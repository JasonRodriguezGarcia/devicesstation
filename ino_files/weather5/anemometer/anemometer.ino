// Código para anemómetro HWFS-1 

#include <ESP8266WiFi.h>        // libreria para el wifi
#include <ESP8266HTTPClient.h>  // libreria httpclient
#include <WiFiClientSecure.h>  // Esto para el permitir https
// Esto es para el el LCD 16x2
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
// Dirección I2C común: 0x27 o 0x3F
// LiquidCrystal_I2C lcd(0x27, 16, 2);  // PARA DIRECCION 0x27
LiquidCrystal_I2C lcd(0x3F, 16, 2);     // PARA DIRECCION 0x3F

#define CONFIG_PIN 12  // GPIO12 osea D6 mostrado en la placa OJO  // Pin que selecciona URL

// Único pin analógico del ESP8266
#define WIND_SENSOR_PIN A0   

// Declara apiBaseUrl como puntero a char normal, no const para asignar mas tarde
const char* apiBaseUrl;

// Evitamos la posibilidad de que suba a 4V, máx 3.3v en A0
const float DIVIDER_FACTOR = 0.825; 

// Contador de tiempo para la llamada API
int contadorAPI = 0;

// Variable a usar para el cambio de estado del pin 12 (GPIO12 osea D6)
// y poder conectar o desconectar el pin y el cambio se haga al vuelo
static bool lastState = HIGH;

String deviceId; // define variable para poder identificar dispositivo en backend

// Offset medido con el sensor quieto y conectado al ESP
// (al estar quieto sin moverse hay veces que se muestra un valor residual)
const float OFFSET_VOLTAGE = 0.039;

void setup() {
  Serial.begin(9600);
    // Para seleccionar apiBaseUrl según D5 si está conectado a masa o no
  pinMode(CONFIG_PIN, INPUT_PULLUP);
  // Guarda el estado ACTUAL del pin como estado inicial
  lastState = digitalRead(CONFIG_PIN);

  if(digitalRead(CONFIG_PIN) == LOW){
      apiBaseUrl = "http://192.168.0.20:5000/api/v1/"; // LOCAL
  } else {
      apiBaseUrl = "https://devicesstation-hyck.onrender.com/api/v1/"; // REMOTO
  }

  Serial.print("API URL seleccionada: ");
  Serial.println(apiBaseUrl);

  // Para el LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0); // Columna 0, línea 0
  lcd.print("Anemomother");
  lcd.setCursor(0, 1); // Columna 0, línea 1
  lcd.print("Booting ...");
  
  // poner nombre wifi y contraseña
  WiFi.begin("vodafone3CAA", "NZWR3LH9ACXPR4"); 

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando a la red WiFi...");
  }

  Serial.println("Conexión WiFi establecida");

  // DATO INFORMATIVO SIN MAS
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
  checkConfigPin(); // Comprobar si se ha conectado el D6 para cambiar de local a https

  // -------- PROMEDIO EN LAS LECTURAS ----------
  int analogValue = 0;
  for (int i = 0; i < 10; i++) {
    analogValue += analogRead(WIND_SENSOR_PIN);
    delay(2);
  }
  analogValue /= 10;

  // -------- VOLTAJE ----------
  float voltage = analogValue * 3.3 / 1023.0;
  voltage = voltage / DIVIDER_FACTOR;

  // -------- RESTAR OFFSET ----------
  voltage -= OFFSET_VOLTAGE;
  if (voltage < 0.0) voltage = 0.0;

  // -------- VELOCIDAD ----------
  float windSpeedMS = voltage * 14.0;
  float windSpeedKMH = voltage * 50.0;

  // -------- ELIMINAR VELOCIDAD RESIDUAL (A VECES MUESTRA 0.01KM/H) POR ----------
  //      redondeos internos del float
  //      multiplicación por 50.0
  //      ruido mínimo de ±1 en la lectura ADC
  if (windSpeedMS < 0.02) windSpeedMS = 0.0;
  if (windSpeedKMH < 0.05) windSpeedKMH = 0.0;

  // -------- SALIDA ----------
  Serial.print("Voltaje ajustado: ");
  Serial.print(voltage, 3);
  Serial.print(" V | Velocidad viento: ");
  Serial.print(windSpeedMS, 2);
  Serial.print(" m/s | ");
  Serial.print(windSpeedKMH, 2);
  Serial.println(" km/h");

    if (contadorAPI >= 5000) { // SI HAN PASADO 5 SEGUNDOS
        // Salida a consola del IDE
        Serial.print("m/s: ");
        Serial.println(windSpeedMS);
        Serial.print("Km/h: ");
        Serial.println(windSpeedKMH);

        // Salida a LCD
        lcd.clear();
        lcd.setCursor(0, 0);  // Columna 0, línea 0
        lcd.print("M/s: ");
        lcd.print(windSpeedMS);
        lcd.setCursor(0, 1);  // Columna 0. Línea 1
        lcd.print("Km/h:  ");
        lcd.print(windSpeedKMH);
        
        if (isnan(windSpeedMS) || isnan(windSpeedKMH)) {
            Serial.println("Error sensor Anemómetro");
            // Salida a LCD
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Error :_(");
            
            delay(2000);
            return;
        }
            
        sendDataToApi(windSpeedMS, windSpeedKMH);
        contadorAPI = 0;
    } else {
        contadorAPI += 500;
    }
  
  delay(500);
}

void sendDataToApi(float windSpeedMS, float windSpeedKMH) {
  WiFiClientSecure client;
  //   Versión que permite usar https, pero tenemos que conseguir el fingerprint de render.com (donde está alojado el
  //   backend). Para ello lo conseguimos desde el Bash de Git en VSC poniendo:
  //     echo | openssl s_client -connect weatherstation-hyck.onrender.com:443 | openssl x509 -noout -fingerprint -sha1
  // OJO PONER EL RESULTADO SIN LOS ":"
  // EN RENDER SE CAMBIA EL FINGERPRINT CADA "X" MESES, POR LO QUE IGUAL NO FUNCIONA MÁS TARDE
  client.setFingerprint("A8EE4611100C0E7D4E9D25EB6350683045916B28"); 
  
  HTTPClient http;
  
  String url = String(apiBaseUrl) + "meassures";
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
    "\"description\": \"Anemomether\","+
    "\"data\": {"+
        "\"m/s\": " + String(windSpeedMS) + ","+
        "\"km/h\": " + String(windSpeedKMH) + "}" +
    "}";

  int httpCode = http.POST(jsonData);

  if (httpCode > 0) {
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