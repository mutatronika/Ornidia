/*
 * Sistema de Monitoreo de Invernadero
 * 
 * Este sketch monitorea temperatura, humedad, luz y humedad del suelo
 * para optimizar las condiciones de crecimiento de plantas.
 * 
 * Hardware requerido:
 * - ESP8266 / ESP32
 * - DHT11 o DHT22 (temperatura y humedad)
 * - BH1750 (sensor de luz I2C)
 * - Sensor de humedad de suelo (analógico)
 * - Relés para control de actuadores (opcional)
 * 
 * Librerías:
 * - DHT sensor library
 * - BH1750
 * - Wire
 */

#include <Wire.h>
#include <DHT.h>
#include <BH1750.h>

// ===== CONFIGURACIÓN DE PINES =====
#define DHTPIN D7           // Pin del sensor DHT
#define DHTTYPE DHT22       // DHT11 o DHT22
#define SOIL_MOISTURE_PIN A0  // Sensor de humedad de suelo
#define RELAY_WATER D5      // Relé de bomba de agua
#define RELAY_FAN D6        // Relé de ventilador
#define RELAY_LIGHT D8      // Relé de luz suplementaria

// ===== CONFIGURACIÓN I2C =====
#define SDA_PIN D2          // GPIO4
#define SCL_PIN D1          // GPIO5

// ===== UMBRALES DE CONTROL =====
#define TEMP_MAX 28.0       // Temperatura máxima (°C)
#define TEMP_MIN 18.0       // Temperatura mínima (°C)
#define HUMIDITY_MAX 80.0   // Humedad máxima (%)
#define HUMIDITY_MIN 50.0   // Humedad mínima (%)
#define SOIL_DRY 30         // Humedad suelo baja (%)
#define SOIL_WET 70         // Humedad suelo alta (%)
#define LIGHT_MIN 10000     // Luz mínima (lux)

// ===== OBJETOS DE SENSORES =====
DHT dht(DHTPIN, DHTTYPE);
BH1750 lightMeter;

// ===== VARIABLES GLOBALES =====
struct GreenhouseData {
  float temperature;
  float humidity;
  float lightIntensity;
  int soilMoisture;
  float vpd;              // Déficit de Presión de Vapor
  bool waterPumpActive;
  bool fanActive;
  bool lightActive;
};

GreenhouseData currentData;

// ===== SETUP =====
void setup() {
  Serial.begin(115200);
  delay(100);
  
  Serial.println("\n=================================");
  Serial.println("Sistema de Monitoreo de Invernadero");
  Serial.println("=================================\n");
  
  // Configurar pines de relés
  pinMode(RELAY_WATER, OUTPUT);
  pinMode(RELAY_FAN, OUTPUT);
  pinMode(RELAY_LIGHT, OUTPUT);
  
  // Apagar todos los relés al inicio
  digitalWrite(RELAY_WATER, LOW);
  digitalWrite(RELAY_FAN, LOW);
  digitalWrite(RELAY_LIGHT, LOW);
  
  // Inicializar DHT
  dht.begin();
  Serial.println("✓ DHT22 inicializado");
  
  // Inicializar I2C
  Wire.begin(SDA_PIN, SCL_PIN);
  
  // Inicializar BH1750
  if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    Serial.println("✓ BH1750 inicializado");
  } else {
    Serial.println("✗ Error inicializando BH1750");
  }
  
  Serial.println("\nSistema listo. Iniciando monitoreo...\n");
}

// ===== LOOP PRINCIPAL =====
void loop() {
  // Leer sensores
  readSensors();
  
  // Calcular VPD
  calculateVPD();
  
  // Mostrar datos
  displayData();
  
  // Controlar actuadores
  controlActuators();
  
  // Enviar datos (descomentar según necesidad)
  // sendDataToServer();
  // sendDataToMQTT();
  
  delay(10000);  // Leer cada 10 segundos
}

// ===== FUNCIONES DE LECTURA =====
void readSensors() {
  // Leer temperatura y humedad
  currentData.temperature = dht.readTemperature();
  currentData.humidity = dht.readHumidity();
  
  // Validar lecturas DHT
  if (isnan(currentData.temperature) || isnan(currentData.humidity)) {
    Serial.println("⚠ Error leyendo DHT22");
    currentData.temperature = 0;
    currentData.humidity = 0;
  }
  
  // Leer luz
  currentData.lightIntensity = lightMeter.readLightLevel();
  
  // Leer humedad del suelo (convertir a porcentaje)
  int soilRaw = analogRead(SOIL_MOISTURE_PIN);
  currentData.soilMoisture = map(soilRaw, 0, 1023, 0, 100);
}

// ===== CÁLCULO DE VPD =====
void calculateVPD() {
  if (currentData.temperature > 0 && currentData.humidity > 0) {
    // Calcular presión de vapor saturada (kPa)
    float svp = 0.6107 * exp((17.27 * currentData.temperature) / 
                             (currentData.temperature + 237.3));
    
    // Calcular VPD
    currentData.vpd = svp * (1 - currentData.humidity / 100.0);
  } else {
    currentData.vpd = 0;
  }
}

// ===== MOSTRAR DATOS =====
void displayData() {
  Serial.println("─────────────────────────────────");
  Serial.println("📊 DATOS DEL INVERNADERO");
  Serial.println("─────────────────────────────────");
  
  Serial.print("🌡️  Temperatura: ");
  Serial.print(currentData.temperature, 1);
  Serial.println(" °C");
  
  Serial.print("💧 Humedad aire: ");
  Serial.print(currentData.humidity, 1);
  Serial.println(" %");
  
  Serial.print("☀️  Luz:         ");
  Serial.print(currentData.lightIntensity, 0);
  Serial.println(" lux");
  
  Serial.print("🌱 Hum. suelo:   ");
  Serial.print(currentData.soilMoisture);
  Serial.println(" %");
  
  Serial.print("📊 VPD:          ");
  Serial.print(currentData.vpd, 2);
  Serial.print(" kPa ");
  printVPDStatus();
  
  Serial.println("\n💡 ESTADO DE ACTUADORES");
  Serial.println("─────────────────────────────────");
  Serial.print("💦 Riego:        ");
  Serial.println(currentData.waterPumpActive ? "ACTIVO" : "inactivo");
  Serial.print("💨 Ventilador:   ");
  Serial.println(currentData.fanActive ? "ACTIVO" : "inactivo");
  Serial.print("💡 Luz:          ");
  Serial.println(currentData.lightActive ? "ACTIVA" : "inactiva");
  Serial.println();
}

void printVPDStatus() {
  if (currentData.vpd < 0.4) {
    Serial.println("(bajo - aumentar ventilación)");
  } else if (currentData.vpd < 0.8) {
    Serial.println("(óptimo - plántulas)");
  } else if (currentData.vpd < 1.2) {
    Serial.println("(óptimo - vegetativo)");
  } else if (currentData.vpd < 1.5) {
    Serial.println("(óptimo - floración)");
  } else {
    Serial.println("(alto - aumentar humedad)");
  }
}

// ===== CONTROL DE ACTUADORES =====
void controlActuators() {
  // Control de riego basado en humedad del suelo
  if (currentData.soilMoisture < SOIL_DRY) {
    activateWaterPump(true);
  } else if (currentData.soilMoisture > SOIL_WET) {
    activateWaterPump(false);
  }
  
  // Control de ventilación
  if (currentData.temperature > TEMP_MAX || currentData.humidity > HUMIDITY_MAX) {
    activateFan(true);
  } else if (currentData.temperature < TEMP_MAX - 2) {
    activateFan(false);
  }
  
  // Control de luz suplementaria
  if (currentData.lightIntensity < LIGHT_MIN && isDayTime()) {
    activateLight(true);
  } else {
    activateLight(false);
  }
}

void activateWaterPump(bool state) {
  currentData.waterPumpActive = state;
  digitalWrite(RELAY_WATER, state ? HIGH : LOW);
}

void activateFan(bool state) {
  currentData.fanActive = state;
  digitalWrite(RELAY_FAN, state ? HIGH : LOW);
}

void activateLight(bool state) {
  currentData.lightActive = state;
  digitalWrite(RELAY_LIGHT, state ? HIGH : LOW);
}

// ===== FUNCIONES AUXILIARES =====
bool isDayTime() {
  // Implementar lógica de horario
  // Por ahora, simple check de hora si tienes RTC
  // O usar sensor de luz como referencia
  return (currentData.lightIntensity > 100);
}

// ===== ENVÍO DE DATOS (OPCIONAL) =====
/*
void sendDataToServer() {
  // Implementar envío HTTP/HTTPS
  // WiFiClient client;
  // http.begin(client, "http://tu-servidor.com/api/greenhouse");
  // http.addHeader("Content-Type", "application/json");
  // String payload = createJSONPayload();
  // http.POST(payload);
}

void sendDataToMQTT() {
  // Implementar publicación MQTT
  // mqttClient.publish("greenhouse/temperature", String(currentData.temperature));
  // mqttClient.publish("greenhouse/humidity", String(currentData.humidity));
  // mqttClient.publish("greenhouse/light", String(currentData.lightIntensity));
}

String createJSONPayload() {
  String json = "{";
  json += "\"temperature\":" + String(currentData.temperature) + ",";
  json += "\"humidity\":" + String(currentData.humidity) + ",";
  json += "\"light\":" + String(currentData.lightIntensity) + ",";
  json += "\"soilMoisture\":" + String(currentData.soilMoisture) + ",";
  json += "\"vpd\":" + String(currentData.vpd);
  json += "}";
  return json;
}
*/
