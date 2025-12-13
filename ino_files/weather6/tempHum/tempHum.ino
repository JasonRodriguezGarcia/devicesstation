// VERSION CON LCD 1602 16X2 CON MODULO I2C (OPCIONAL)
// POSIBILIDAD DE CAMBIAR ENTRE SENSOR DHT11 (SENCILLO) POR DHT22 (MEJORADO)
// ENVIA ID DEL DISPOSITIVO PARA PODER IDENTIFICAR PROCEDENCIA DE DATOS DE DISTINTOS DISPOSITIVOS EN BACKEND
// PERTIME EL USO DE BACKEND HTTPS O HTTP LOCAL AL HACER LLAMADA A API, SELECCIONABLE CON pin D6 a masa.
// AÑADIDO CONFIG.H PARA SER USADA COMO VARIABLE DE ENTORNO A LO .ENV EN REACT.

// Para poder trabajar y programar este módulo compatible Arduino, hay que descargar arduino ide de 
// https://www.arduino.cc/en/software/
// Luego hay que configurar varias cosas.
// Tengo una placa (China) detectada como LilyGo T-Display en com6 pero se puede usar como Generic ESP8266
// Para añadir DHT.h librería con (mi versión es en inglés)
// Sketch / include library / manage libraries y buscar dht sensor library e instalar
// Para añadir esp8266 en archivo / preferencias / additional boards manager url añadir
// http://arduino.esp8266.com/stable/package_esp8266com_index.json
// y en tools / board / board manager ya aparecerá ESP8266 instalar
// luego en tools / board / aparecerá ESP8266 / Generic ESP8266 module
// después en tools / Flash Size / poner 4MB(FS: 1MB OTA: 1019kb)
// Instalar la librería LiquidCrystal_I2C desde el Administrador de Bibliotecas (Tools / Manage libraries).
// Instalar la librería Base 64 desde Sketch → Include Library → Manage Libraries


// https://www.youtube.com/watch?v=5NkyvyodCLE&ab_channel=LosProfesparavos

#include "config.h"             // variables de entorno (escondidas en un config.h)

// Esto para ESP8266
#include <ESP8266WiFi.h>        // libreria para el wifi
#include <ESP8266HTTPClient.h>  // libreria httpclient
#include <DHT.h>                // libreria sensor dht11
#define DHTPIN 2                // definiendo pin data del DHT11
// #define DHTTYPE DHT11        // Definiendo el tipo de DHT para ser DHT11, lee cada 1 segundo
#define DHTTYPE DHT22           // Definiendo el tipo de DHT para ser DHT22, lee cada 2 segundos
DHT dht(DHTPIN, DHTTYPE);
// Esto para el permitir https
#include <WiFiClientSecure.h>  // Añade esta librería arriba para https
// Esto es para el el LCD 16x2
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
// Dirección I2C común: 0x27 o 0x3F
// LiquidCrystal_I2C lcd(0x27, 16, 2);  // PARA DIRECCION 0x27
LiquidCrystal_I2C lcd(0x3F, 16, 2);     // PARA DIRECCION 0x3F
#define CONFIG_PIN 12  // GPIO12 osea D6 mostrado en la placa OJO  // Pin que selecciona URL
// Declara apiBaseUrl como puntero a char normal, no const para asignar mas tarde
const char* apiBaseUrl;
// const char* apiKey = "tu_clave_de_api";

// Variable a usar para el cambio de estado del pin 12 (GPIO12 osea D6)
// y poder conectar o desconectar el pin y el cambio se haga al vuelo
static bool lastState = HIGH;

String deviceId; // define variable para poder identificar dispositivo en backend

void setup() {
  Serial.begin(9600); // para mostar los Serial.print() en la consola

  // Para seleccionar apiBaseUrl según D5 si está conectado a masa o no
  pinMode(CONFIG_PIN, INPUT_PULLUP);
  // Guarda el estado ACTUAL del pin como estado inicial
  lastState = digitalRead(CONFIG_PIN);

  if(digitalRead(CONFIG_PIN) == LOW){
      // LOCAL
      apiBaseUrl = APIBASEURLLOCAL; 
  }else{
      // REMOTO
      apiBaseUrl = APIBASEURLREMOTO; 
  }

  Serial.print("API URL seleccionada: ");
  Serial.println(apiBaseUrl);

  // Para el LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0); // Columna 0, línea 0
  lcd.print("Weather Station");
  lcd.setCursor(0, 1); // Columna 0, línea 1
  lcd.print("Booting ...");
  
  dht.begin(); // INICIALIZA SENSOR DHT22

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD); // poner nombre wifi y contraseña

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando a la red WiFi...");
  }

  Serial.println("Conexión WiFi establecida");

  Serial.print("IP del ESP8266: "); // DATO INFORMATIVO SIN MAS
  Serial.println(WiFi.localIP());

  lcd.setCursor(0, 1);            // Columna 0, línea 1
  deviceId = WiFi.macAddress();   // "84:F3:EB:AB:12:34" (por ejemplo)
  deviceId.replace(":", "");      // eliminar los dos puntos ya que si no la MAC no entra en una línea
  lcd.print("MAC:" + deviceId);
  Serial.println("MAC: " + deviceId);

  
  delay(3000);
}

void loop() {
  checkConfigPin(); // Comprobar si se ha conectado el D6 para cambiar de local a https

  float temperature_data = dht.readTemperature();
  float humidity_data = dht.readHumidity();

  // Salida a consola del IDE
  Serial.print("Temperature: ");
  Serial.println(temperature_data);
  Serial.print("Humidity: ");
  Serial.println(humidity_data);
  //   delay(500); EVITAR ESTE DELAY YA QUE PUEDE CAUSAR LECTURAS INESTABLES, IMPORTANTE!!

  // Salida a LCD
  lcd.clear();
  lcd.setCursor(0, 0);  // Columna 0, línea 0
  lcd.print("Temp: ");
  lcd.print(temperature_data);
  lcd.print(" C");
  lcd.setCursor(0, 1);  // Columna 0. Línea 1
  lcd.print("Hum:  ");
  lcd.print(humidity_data);
  lcd.print(" %");
  
  if (isnan(temperature_data) || isnan(humidity_data)) {
    // Serial.println("Error al leer el sensor DHT11");
    Serial.println("Error al leer el sensor DHT22");
    // Salida a LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Error sensor :_(");
  
    delay(2000);
    return;
  }
  
  sendDataToApi(temperature_data, humidity_data);
  
  delay(5000);
}

void sendDataToApi(float temperature_data, float humidity_data) {
// WiFiClient client;
  WiFiClientSecure client;
  //   Versión que permite usar https, pero tenemos que conseguir el fingerprint de render.com (donde está alojado el
  //   backend). Para ello lo conseguimos desde el Bash de Git en VSC poniendo:
  //     echo | openssl s_client -connect devicesstation-backend.onrender.com:443 | openssl x509 -noout -fingerprint -sha1
  // OJO PONER EL RESULTADO SIN LOS ":"
  // EN RENDER SE CAMBIA EL FINGERPRINT CADA "X" MESES, POR LO QUE IGUAL NO FUNCIONA MÁS TARDE
  client.setFingerprint("A8EE4611100C0E7D4E9D25EB6350683045916B28"); 
  
  HTTPClient http;
  
  String url = String(apiBaseUrl) + "measures";
  Serial.println("URL de solicitud: " + url);
  
  bool isHttps = url.startsWith("https");

  if (isHttps) {
      Serial.println("🔒 Modo HTTPS");

      if (!http.begin(client, url)) {
        Serial.println("❌ Error iniciando conexión HTTPS");
        return;
      }

  } else {
      Serial.println("🔓 Modo HTTP local");

      WiFiClient client;
      if (!http.begin(client, url)) {
        Serial.println("❌ Error iniciando conexión HTTP");
        return;
      }
  }

//   http.addHeader("Authorization", apiKey);
  http.addHeader("Content-Type", "application/json");
  // http.addHeader("Authorization", "Bearer "+ deviceId);

  // Crear un objeto JSON con los datos a enviar en el body
  String jsonData = 
    "{\"device_id\": \"" + String(deviceId) + "\"," +
    "\"type\": \"devices\","+
    "\"description\": \"Weather sensor\","+
    "\"data\": {"+
        "\"temperature\": " + String(temperature_data) + ","+
        "\"humidity\": " + String(humidity_data) + "}" +
    "}";

  int httpCode = http.POST(jsonData);

  if (httpCode > 0) {
    // if (httpCode == HTTP_CODE_OK) {
    if (httpCode >= 200 && httpCode < 300) {
      String payload = http.getString();
      Serial.println("Respuesta recibida: " + payload);
    } else {
      Serial.println("Error en la solicitud. Código de error: " + String(httpCode));
    }
  } else {
    Serial.println("Error en la conexión - " + String(http.errorToString(httpCode).c_str()));
  }

  http.end();
}

// Detecta si se ha hecho la conexión/desconexion del pin D6 a masa
// Para el cambio de local a https
void checkConfigPin() {
    bool current = digitalRead(CONFIG_PIN);
    delay(50); // anti rebotes
    current = digitalRead(CONFIG_PIN);

    if (current != lastState) {
        Serial.println("⚠️ Cambio detectado, reiniciando...");
        delay(500);
        ESP.restart();
    }

    lastState = current;
}