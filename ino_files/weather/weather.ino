// tengo una placa LilyGo T-Display en com6
// para añadir DHT.h librería con 
// Sketch / include library / manage libraries y buscar dht sensor library e instalar
// para añadir esp8266 en archivo / preferencias / additional boards manager url añadir
// http://arduino.esp8266.com/stable/package_esp8266com_index.json
// y en tools / board / board manager buscar esp8266 instalar
// luego en tools / board / aparecerá ESP8266 / Generic ESP8266 module

// Esto para ESP8266
#include <ESP8266WiFi.h> // libreria para el wifi
#include <ESP8266HTTPClient.h> // libreria httpclient
// Esto para ESP32
// #include <WiFi.h>
// #include <HTTPClient.h>
#include <DHT.h> // libreria sensor dht11

// https://www.youtube.com/watch?v=5NkyvyodCLE&ab_channel=LosProfesparavos

// descargar arduino ide
// https://www.arduino.cc/en/software/

// Esto para ESP8266
#define DHTPIN 2
// Esto para ESP32
// #define DHTPIN 21

#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

// const char* apiBaseUrl = "http://api/";
// const char* apiBaseUrl = "http://localhost:5000/api/v1/";
const char* apiBaseUrl = "http://192.168.0.18:5000/api/v1/";
const char* apiKey = "tu_clave_de_api";

void setup() {
  Serial.begin(9600);
  WiFi.begin("xxxxxxxexxxx", "xxxxxxxxxxxxxx"); // poner nombre wifi y contraseña

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
  if (isnan(temperature_data) || isnan(humidity_data)) {
    Serial.println("Error al leer el sensor DHT11");
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
//   String jsonData = "{\"temperature\": " + String(temperature_data) + ", \"humidity\": " + String(humidity_data) + "}";
  String jsonData = "{\"temperature\": " + String(temperature_data) + ", \"humidity\": " + String(humidity_data) + "}";

  int httpCode = http.POST(jsonData);

  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK) {
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