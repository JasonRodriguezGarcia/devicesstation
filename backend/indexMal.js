// meassure, penúltima versión antes del cambio a rutas avanzadas con la creación de db.js, meassure.js
import express from "express";
import path from "path";
import cors from 'cors';
import { fileURLToPath } from "url";
import PouchDB from 'pouchdb';
import pouchdbFind from 'pouchdb-find'  // Plugin de PouchDB que permite hacer consultas tipo MongoDB (por campos).


const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

const app = express();
const PORT = process.env.PORT || 5000;

// Creating a database (if it doesn't already exist)
const dataDirectory = path.join(__dirname, 'data');  //directorio data, ojo __dirname está arriba

// This allows advances queries (like .find()) in the database.
PouchDB.plugin(pouchdbFind)
// It doesnt seem working¿?
PouchDB.defaults({
  prefix: path.join(dataDirectory, path.sep), 
});
const db = new PouchDB(path.join(dataDirectory, 'devices')); // bbdd wheater
// creating indexes
(async () => {
    try { 
        // En pouch.db cuando se crean los índices al principio en backend y al hacer consultas , se 
        // selecciona automaticamente el índice más adecuado, no hace falta seleccionar uno
        // Si no encuentra uno adecuado entonces se lanza el aviso
        // "No matching index found, create an index to optimize query time."
        // await db.createIndex({ index: { fields: ['_id'] } });  // No necesario Pouchdb lo hace solo
        await db.createIndex({ index: { fields: ['_id', 'type'] } });
        await db.createIndex({ index: { fields: ['type'] } });
        await db.createIndex({ index: { fields: ['type', 'deviceMac'] } });
        console.log('Índice creado para campos: "_id", "id+type", "type", "type+deviceMac".');
    } catch (err) {
        console.error('Error al crear índices:', err);
    }
  })();



// Middleware
// cors() permite el acceso entre dominios (Cross-Origin Resource Sharing).
// Esto es necesario cuando tienes React y Express en distintos orígenes (por ejemplo, React en localhost:3000 y Express en localhost:5000).
// Sin esto, el navegador bloquearía las peticiones por razones de seguridad.
// Para configurar cors
const corsOptions = { 
    // poner más dentro de los "[]" separados por "," cuando tenga la web del deployment
    origin: ["http://localhost:3000", "https://weatherstation-1-zu3u.onrender.com", "*"], 
    // methods:["POST"] // sin esta línea se deja todos los methods
}
app.use(cors()); // to be able access from frontend, because they are not on same directory?¿server¿?
// Habilita el parsing de JSON en los requests.
// Es decir, cuando un cliente (como React) envía datos en formato JSON (por ejemplo, en un POST o PUT), Express puede leerlos desde req.body.
app.use(express.json());
// app.use('/api/v1/meassures', usersRouters)

// ********************************************
// ********************************************
// At the beginning, remove all devices if any
const removeDevices = async () => {
    try {
        const selector = {
            docType: "devices"
        }
        // Paso 1: Buscar todos los "devices"
        const result = await db.find({
            selector,
            fields: ['_id', '_rev'], // OJO añadir SIEMPRE _id y _rev si no, no actualiza
            // limit: 10
        })
        // const result = await db.allDocs({include: true})

        // Paso 2: Eliminar cada documento encontrado
        for (let doc of result.docs) {
            await db.remove(doc); // Borramos el documento usando _id y _rev
            // await db.remove(doc._id, doc._rev); // Borramos el documento usando _id y _rev
        }
        const resultCuenta = await db.allDocs({ include_docs: true}); // trae todos los documentos
        const docsNormales = resultCuenta.rows.filter(row => !row.id.startsWith('_design/'));
        console.log("cuenta docsNormales: ", docsNormales.length)
        console.log("All devices Deleted OK")
        
    } catch (error) {
        console.log(error.message)
    }
}

removeDevices()
// ********************************************
// ********************************************

// setting initial state with devices
let devicesPrevious = []
setInterval(async () => {
    // retrieving unique values from devicesPrevious
    const unicos = [...new Set(devicesPrevious)];
    const devicesTMP = []
    // getting devices
    const selector = {
        // type: "devices"
    }
    // Searching for all available "devices"
    const result = await db.find({
        selector,
        fields: ['_id', '_rev', 'deviceMAC'], // OJO añadir SIEMPRE _id y _rev si no, no actualiza
        // limit: 10
    })
    // If no data, no devices
    if (result.docs.length == 0) {
        devicesPrevious = []
        return
    }

    for (let doc of result.docs) {
        let founded = unicos.includes(doc.deviceMAC)
        if (founded) {
            console.log("Encontrado!!")
            devicesTMP.push(doc)
        }
        else {
            console.log("NO encontrado")
            // se borra el elemento no encontrado
            await db.remove(doc._id, doc._rev); // Borramos el documento usando _id y _rev
        }
    }

    // devicePrevious value setted to devicesTMP value
    devicesPrevious = [...devicesTMP]

}, 15000);

// *************************
// *************************
// *** devices start    ****
// *************************
// *************************
app.get('/api/v1/devices', async (req, res) => {
    try {
        // Fetch all documents from the 'wheather_db'
        const result = await db.allDocs({ include_docs: true });
        // console.log(result);
  
        // Filter the meassure if 'type' is used in the document
        const devices = result.rows
            // .filter(row => row.doc.type === 'devices')  // Ensure the document type is 'meassure'
            .map(row => row.doc);  // Map to get the document content

        console.log("Esto se manda a frontend: ", devices)
 
        res.status(200).json(devices);
    } catch (error) {
        res.status(500).json({ error: 'Failed to retrieve meassure' });
    }
});

app.delete('/api/v1/device/:id', async (req, res) => {

    const {id} = req.params
    console.log(req.params)
    try {
        const device = await db.get(id)   // cogemos documento (meassure)
        await db.remove(device)   // borra documento
        res.status(200).json({message: "Deleted OK"})

    } catch (error) {
        res.status(500).json({ error: 'Failed to delete meassure' });
    }
});

// *************************
// *************************
// *** meassures start   ***
// *************************
// *************************
// TAMBIÉN PODRÍAMOS PROBAR LAS APIS CON POSTMAN
// poniendo POST y la url http://localhost:5000/api/v1/meassures
// como Body - Raw - json
// data structure:
// {
    // device_id: '8CAAB57456A8', // each device will have a different MAC address from factory
    // "temperature": 15.5,
    // "humidity": 68,
// }

app.get('/api/v1/measures', async (req, res) => {
    try {
        // Fetch all documents from the 'wheather_db'
        const result = await db.allDocs({ include_docs: true });
        // console.log(result);
  
        // Extract measure data from the documents
        // const measure = result.rows.map(row => row.doc);

        // Filter the measure if 'type' is used in the document
        const measures = result.rows
            // .filter(row => row.doc.type === 'tempHum')  // Ensure the document type is 'measure'
            .map(row => row.doc);  // Map to get the document content
 
        // Send the meassure as JSON response
        res.status(200).json(measures);
    } catch (error) {
        res.status(500).json({ error: 'Failed to retrieve measure' });
    }
});
  
// ✅ Endpoint de salud básico (para Render / UptimeRobot-pending)
// Usaremos este Endpoint para que el backend en Render que es gratuíto no entre en stand-by
app.get("/api/v1/meassures/health", (req, res) => {
    console.log("Llamando a Health para no entrar en stand-by...")
    res.status(200).send("OK")
})

app.post('/api/v1/meassures', async (req, res) => {
    try {
        const measure = req.body; // measure data from request body
        // const {device_id, temperature, humidity} = req.body;
        const {device_id, type, data} = req.body;
        // measure.type = "tempHum" // crear un usuario, tipo usuario measure (como una tabla measure)
        measure.date = new Date().toLocaleString("es-ES", { timeZone: "Europe/Madrid" });

        
        // Insert new meassure into the database
        const response = await db.post(measure);

        // Check if device is in devices table
        const selector = {
            $and: [
                // {type: "devices"},
                {deviceMAC: device_id}
            ]
        }
        const deviceExist = await db.find({
            selector,
            fields: ['_id', '_rev', 'type', 'deviceMAC', 'data'], // add ALLWAYS _id AND _rev otherwise, doen't update
            // limit: 10
        })
        // Either if exist or not one device, it will be updated/created with last temp. and humid. registry
        if (deviceExist.docs.length !== 0) {
            // if device exist, update temp and humid with last registry
            console.log("EXISTE")
            const existingDevice = deviceExist.docs[0];
            const updatedDevice = {
                _id: existingDevice._id,
                _rev: existingDevice._rev, // compulsory to update in PouchDB
                // type: "devices",
                type,
                deviceMAC: device_id,
                data
                // temperature,
                // humidity
            };
            await db.put(updatedDevice);
            
        } else {
            // if not exist, a new deviced is created with temp. and humid.
            console.log("NO EXISTE, dispositivo nuevo !!")  // deviceExist.docs will be [] (empty)
            let device = {
                // type: "devices", // crear un dispositivo, tipo dispositivo "devices" (como una tabla devices)
                type, // crear un dispositivo, tipo dispositivo "devices" (como una tabla devices)
                deviceMAC: device_id,
                data
                // temperature: temperature,
                // humidity: humidity
            }
            // Insert new device into the database
            const response = await db.post(device);
        }
        // MAC is stored anyway for device existing purposes
        devicesPrevious.push(device_id)
        
        // Respond with success
        res.status(201).json({ id: response.id, ...meassure });
    } catch (error) {
        res.status(500).json({ error: 'Failed to add measure' });
    }
});

app.delete('/api/v1/meassures/:id', async (req, res) => {

    const {id} = req.params
    try {
        const meassure = await db.get(id)   // getting document (meassure)
        await db.remove(meassure)   // deleting document
        res.status(200).json({message: "Deleted OK"})

    } catch (error) {
        res.status(500).json({ error: 'Failed to delete meassure' });
    }
});

app.get('/api/v1/meassures/:id', async (req, res) => {

    const {id} = req.params
    try {
        const meassure = await db.get(id)   // getting document (meassure)
        res.status(200).json({ ...meassure})

    } catch (error) {
        res.status(500).json({ error: "Failed to get, Id not found" });
    }
});

app.put('/api/v1/meassures/:id', async (req, res) => {
    try {
        const { id } = req.params;  // Get the meassure ID from the URL
        const updatedmeassure = req.body;  // The updated meassure data from the request body

        // Fetch the current meassure data using the ID
        const existingmeassure = await db.get(id);
        
        // Update the existing meassure's data with the new data
        const updatedDoc = {
            ...existingmeassure,
            ...updatedmeassure, // This will overwrite any matching fields
        };

        // Save the updated document back to the database
        const response = await db.put(updatedDoc);

        // Send back the updated meassure data
        res.status(200).json({
            id: response.id,
            rev: response.rev,
            ...updatedmeassure,  // The updated fields from the request
        });
    } catch (error) {
        res.status(500).json({ error: 'Failed to update meassure' });
    }
});

// ********** meassure end ***************
//*****************************************


// Start Server
app.listen(PORT, () => {
  console.log(`Server running on http://localhost:${PORT}`);
});