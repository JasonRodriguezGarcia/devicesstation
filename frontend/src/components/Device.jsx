import React, {useState, useEffect} from 'react';
import { useNavigate } from 'react-router-dom';
import { Button, Box, TextField, Typography } from '@mui/material';
import "./font.css"

const BACKEND_URL_RENDER = process.env.REACT_APP_BACKEND_URL_RENDER
console.log ("backend_url_render: ", BACKEND_URL_RENDER)

function Device() {

    const [devices, setDevices] = useState([]);
    const [contador, setContador] = useState(0);
    const [error, setError] = useState("");

    const navigate = useNavigate();

    const goToCars = () => {
        navigate("/");
    }

    useEffect(()=> {
        console.log("llamando ...: ", BACKEND_URL_RENDER)
        const fetchDevices = async () => {
            try {
                const response = await fetch(`${BACKEND_URL_RENDER}/api/v1/devices`);
                if (!response.ok) {
                throw new Error('Network response was not ok');
                }
                const data = await response.json();
                console.log("imprimo data: ", data);
                setDevices(data);
                console.log("imprimo devices: ", devices);

            } catch (error) {
                setError(error.message); // Handle errors
            } finally {
                // setLoading(false); // Set loading to false once data is fetched or error occurs
            }
        }
        fetchDevices();

    }, [contador])

    useEffect(() => {
        const intervalo = setInterval(() => {
          setContador(prev => prev + 1);
          
          // Limpieza al desmontar
        }, 10000);
        return () => clearInterval(intervalo);
    }, []);

    return (
    <div>
    <Box component="div" sx={{backgroundColor: "#035bffff", height: "100vh"}}>

        <Box component="div" sx={{display: "flex", flexDirection: "column", alignItems: "center"}}
        >
            <Typography variant="h3" sx={{ backgroundColor: "#2137ffff", color: "white", padding: "20px", 
                border: "3px solid", borderRadius: "10px", marginY: "10px"
            }}>
                Devices station
            </Typography>
            {devices.length > 0 ?
                devices.map((device, index) => (
                    <Box key={index} component="div" sx={{display: "flex", margin: "10px", border: "1px solid",
                            borderRadius: "10px", backgroundColor: "black", padding: "10px"}}>
                        <Box component="div"
                            sx={{display: "flex", width: "100%", flexDirection: "column", border: "1px solid",
                                backgroundColor: "blue", color: "white", padding: "10px",
                            }}>
                            {Object.entries(device).map((dev, index) => {
                                const notNeededFields = ['_id', '_rev', 'type']
                                let resultTypo = null
                                if (!notNeededFields.includes(dev[0])) // usamos .some por ser array, si objeto .includes
                                    if (typeof(dev[1]) === 'object') {
                                        resultTypo = Object.entries(dev[1]).map((value, index) => {
                                            return (
                                                <Typography key={index} sx={{fontFamily: 'LcdDot', fontSize: "8px", lineHeight: "30px"}}>
                                                    {value[0]}: {value[1]}
                                                </Typography>
                                            )
                                        })
                                    }
                                    else
                                        resultTypo = `${dev[0]} : ${dev[1]}`
                                    const neededField = () => {
                                        return resultTypo
                                    }
                                        return (
                                            <Typography key={index} sx={{fontFamily: 'LcdDot', fontSize: "8px", lineHeight: "30px"}}>
                                                {neededField()} 
                                            </Typography>
                                        )
                                })
                            }
                        </Box>
                    </Box>      
                ))
            : <Typography>NO DEVICES AVAILABLE !!</Typography> }
        </Box>
    </Box>
    </div>
    );
}
export default Device;