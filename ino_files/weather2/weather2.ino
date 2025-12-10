// VERSION CON LCD 1602 16X2 CON MODULO I2C (OPCIONAL)
// POSIBILIDAD DE CAMBIAR ENTRE SENSOR DHT11 (SENCILLO) POR DHT22 (MEJORADO)

// Para poder trabajar y programar este módulo compatible Arduino, hay que descargar arduino ide de 
// https://www.arduino.cc/en/software/
// Luego hay que configurar varias cosas.
// Tengo una placa (China) detectada como LilyGo T-Display en com6 pero se puede usar como Generic ESP8266
// para añadir DHT.h librería con (mi versión es en inglés)
// Sketch / include library / manage libraries y buscar dht sensor library e instalar
// Para añadir esp8266 en archivo / preferencias / additional boards manager url añadir
// http://arduino.esp8266.com/stable/package_esp8266com_index.json
// y en tools / board / board manager ya aparecerá ESP8266 instalar
// luego en tools / board / aparecerá ESP8266 / Generic ESP8266 module
// después en tools / Flash Size / poner 4MB(FS: 1MB OTA: 1019kb)
// Instalar la librería LiquidCrystal_I2C desde el Administrador de Bibliotecas (Tools / Manage libraries).



// https://www.youtube.com/watch?v=5NkyvyodCLE&ab_channel=LosProfesparavos

// Esto para ESP8266
#include <ESP8266WiFi.h> // libreria para el wifi
#include <ESP8266HTTPClient.h> // libreria httpclient
#include <DHT.h> // libreria sensor dht11
#define DHTPIN 2 // definiendo pin data del DHT11
// #define DHTTYPE DHT11 // Definiendo el tipo de DHT para ser DHT11, lee cada 1 segundo
#define DHTTYPE DHT22 // Definiendo el tipo de DHT para ser DHT22, lee cada 2 segundos
DHT dht(DHTPIN, DHTTYPE);

// Esto es para el el LCD 16x2
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
// Dirección I2C común: 0x27 o 0x3F
// LiquidCrystal_I2C lcd(0x27, 16, 2); 
LiquidCrystal_I2C lcd(0x3F, 16, 2); 



// const char* apiBaseUrl = "http://api/";
// const char* apiBaseUrl = "http://localhost:5000/api/v1/";
const char* apiBaseUrl = "http://192.168.0.18:5000/api/v1/";
const char* apiKey = "tu_clave_de_api";

void setup() {
    Serial.begin(9600);
    // Para el LCD
    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0, 0); // Columna 0, línea 0
    lcd.print("Weather Station");
    lcd.setCursor(0, 1); // Columna 0, línea 1
    lcd.print("Booting ...");
    
    dht.begin(); // INICIALIZA SENSOR DHT22

    // Iniciando la consola serie
    WiFi.begin("xxxxxxxxx", "xxxxxxxxxxx"); // poner nombre wifi y contraseña

    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Conectando a la red WiFi...");
    }

    Serial.println("Conexión WiFi establecida");
}

void loop() {
  float temperature_data = dht.readTemperature();
  float humidity_data = dht.readHumidity();
  Serial.print("Temperature: ");
  Serial.println(temperature_data);
  Serial.print("Humidity: ");
  Serial.println(humidity_data);
//   delay(500); EVITAR ESTE DELAY YA QUE PUEDE CAUSAR LECTURAS INESTABLES, IMPORTANTE!!
  // Salida a LCD
  lcd.clear();
  lcd.setCursor(0, 0); // Columna 0, línea 0
  lcd.print("Temp: ");
  lcd.print(temperature_data);
  lcd.print(" C");
  lcd.setCursor(0, 1); // Columna 0. Línea 1
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
  WiFiClient client;
  HTTPClient http;

  String url = String(apiBaseUrl) + "meassures";
  Serial.println("URL de solicitud: " + url);

  http.begin(client, url);

//   http.addHeader("Authorization", apiKey);
  http.addHeader("Content-Type", "application/json");

  // Crear un objeto JSON con los datos a enviar en el body
  String jsonData = "{\"temperature\": " + String(temperature_data) + ", \"humidity\": " + String(humidity_data) + "}";

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
    Serial.println("Error en la conexión");
  }

  http.end();
}