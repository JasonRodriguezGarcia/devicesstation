// VERSION CON LCD 1602 16X2 CON MODULO I2C (OPCIONAL)
// POSIBILIDAD DE CAMBIAR ENTRE SENSOR DHT11 (SENCILLO) POR DHT22 (MEJORADO)
// ENVIA ID DEL DISPOSITIVO PARA PODER IDENTIFICAR PROCEDENCIA DE DATOS DE DISTINTOS DISPOSITIVOS EN BACKEND
// PERTIME EL USO DE HTTPS AL HACER LLAMADA A API

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
// --- CA LET’S ENCRYPT ISRG ROOT X1 (VÁLIDA HASTA 2035) ---
static const char LE_ROOT_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgISA5cVwP0xR2lHr9CLAGRg+jQaMA0GCSqGSIb3DQEBCwUA
MEoxCzAJBgNVBAYTAlVTMQswCQYDVQQIDAJDQTEWMBQGA1UEBwwNTW91bnRhaW4g
VmlldzEUMBIGA1UECgwLRGVjaUNlcnQgSW5jMB4XDTIwMDkxNjAwMDAwMFoXDTMz
MDkxNjAwMDAwMFowSjELMAkGA1UEBhMCVVMxCzAJBgNVBAgMAkNBMRYwFAYDVQQH
DA1Nb3VudGFpbiBWaWV3MRQwEgYDVQQKDAtEZWdpQ2VydCBJbmMwggIiMA0GCSqG
SIb3DQEBAQUAA4ICDwAwggIKAoICAQDNOv4QIULaXfPa2vL1P2SMy1iKq+VqKjcd
M8jTe3uBbl9Pz3dCavgrPUW4O7r05P1+WNZwxKMoYnKpCnGcNVp9/nhFlBp1C1A+
pykB0x/M33V+lpXFKnf0Z1J2H2TNv3yoaxplxXy0eYGti+nkZxku9kpFWn1qmu/j
iJ6TmcnZ0Vp0XAjk8xyA7mV3IMguQmyENqwR7XmmNM8mUv6YT59vAeU3P6wakfxU
4GoCdZJqoZaMM5sE8SzvdnyTRuT7pCn5kFzT2/8qkQaaMo+yhV6ZRlGRtmpikHpp
lP2jewerDnQnvznbjb0oEJj2QXqzUZ81E+Vnw1KhTUICBCOBYZp2LKjcg1sKkp+2
g4P84F8pMuvLFQ5BtIg32rX8wkeyaMV+rZ5ilYLMZpfeMZ/+ydPf3YXRw2xPEMsr
3MrI/gKXa+v5PNF+vZxeTGW9pyhp+Yp0T8fjM7ARuwS4GgKxecFlD/DFtlpZoZXw
L6Ij7/JPUv2lSqyD4U2Jppl6xr1+LvoIPxj8aWEkKC4vCh9wbwtKnEZ0G93SaOfp
6RLlA8POiG0B0E7U3vE0UN5LXBo/qbPorJzTQtMZspN4402ZVNca3CoGvU8VGS7j
5QT1v49tG6PFEMrBKSe4gGk45hbcWqy5KumbYrudkCVzlzvSnluxPSSMpk9z5pKz
x0/mPDDhugIDAQABo0IwQDAdBgNVHQ4EFgQUNxeSwA2kLTeksoJGELp65z3xwOYw
HwYDVR0jBBgwFoAUNxeSwA2kLTeksoJGELp65z3xwOYwCgYIKoZIzj0EAwIDSAAw
RQIhAPa4GmFkvGr8bYEiwDr1+eoo1+w7SaiQpsqvsmdu4dfKAiA2EEmnmqMxhIeb
8R8vD55eyXHD+gk1LXeMmfxknn62BQ==
-----END CERTIFICATE-----
)EOF";

const char* apiBaseUrl = "https://weatherstation-hyck.onrender.com/api/v1/";
// const char* apiBaseUrl = "http://192.168.0.20:5000/api/v1/"; // ip local
// const char* apiKey = "tu_clave_de_api";

String deviceId; // define variable para poder identificar dispositivo en backend

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

    WiFi.begin("xxxxxxxxxxxxx", "xxxxxxxxxxxxxxxxxx"); // poner nombre wifi y contraseña

    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Conectando a la red WiFi...");
    }

    Serial.println("Conexión WiFi establecida");
    Serial.print("IP del ESP8266: ");
    Serial.println(WiFi.localIP());
    lcd.setCursor(0, 1);            // Columna 0, línea 1
    deviceId = WiFi.macAddress();   // "84:F3:EB:AB:12:34" (por ejemplo)
    deviceId.replace(":", "");      // eliminar los dos puntos ya que si no la MAC no entra en una línea
    lcd.print("MAC:" + deviceId);
    Serial.println("MAC: " + deviceId);

    
    delay(3000);
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

  HTTPClient http;
  String url = String(apiBaseUrl) + "meassures";
  Serial.println("URL de solicitud: " + url);

  bool isHttps = url.startsWith("https");
  Serial.println("isHttps: " + isHttps);

  if (isHttps) {
      Serial.println("🔒 Modo HTTPS con CA Let’s Encrypt");

      WiFiClientSecure client;
      client.setTimeout(5000);
      client.setTrustAnchors(new X509List(LE_ROOT_CA));

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

  http.addHeader("Content-Type", "application/json");

  String jsonData = 
    "{\"device_id\": \"" + String(deviceId) + 
    "\", \"temperature\": " + String(temperature_data) +
    ", \"humidity\": " + String(humidity_data) + "}";

  int httpCode = http.POST(jsonData);

  if (httpCode > 0) {
      Serial.println("HTTP CODE: " + String(httpCode));
      Serial.println("Payload: " + http.getString());
  } else {
      Serial.println("❌ Error en POST: " + String(http.errorToString(httpCode).c_str()));
  }

  http.end();
}