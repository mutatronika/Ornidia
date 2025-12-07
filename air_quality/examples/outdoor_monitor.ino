/*
 * Ornidia - Monitor Exterior de Calidad del Aire
 * 
 * Sistema para monitoreo ambiental urbano con múltiples sensores de partículas
 * Compatible con redes comunitarias como Sensor.Community
 * 
 * Hardware:
 * - Arduino Mega (más pines y memoria)
 * - PMS5003 (partículas PM)
 * - BME280 (temp/humedad/presión)
 * - Carcasa ventilada IP65
 * 
 * Conexiones:
 * PMS5003:
 *   VCC -> 5V
 *   GND -> GND
 *   TX  -> Serial1 RX (Pin 19)
 * 
 * BME280 (I2C):
 *   VCC -> 3.3V
 *   GND -> GND
 *   SDA -> Pin 20 (SDA)
 *   SCL -> Pin 21 (SCL)
 */

#include <Wire.h>
// #include <Adafruit_BME280.h>

// Adafruit_BME280 bme;

// Estructura de datos
struct AirQualityData {
  // Partículas
  float pm1_0;
  float pm2_5;
  float pm10;
  
  // Ambiente
  float temperature;
  float humidity;
  float pressure;
  
  // Calculados
  int aqi;
  String aqi_level;
  
  // Timestamp
  unsigned long timestamp;
} currentData;

// Promedios móviles (para envío cada 2.5 minutos)
const int AVG_SAMPLES = 5;
float pm25_avg[AVG_SAMPLES];
float pm10_avg[AVG_SAMPLES];
int avgIndex = 0;

unsigned long lastRead = 0;
unsigned long lastSend = 0;
const unsigned long READ_INTERVAL = 30000;  // 30 segundos
const unsigned long SEND_INTERVAL = 150000; // 2.5 minutos

void setup() {
  Serial.begin(115200);
  Serial1.begin(9600); // PMS5003 en Serial1
  Wire.begin();
  
  Serial.println(F("=========================================="));
  Serial.println(F("  Monitor Exterior de Calidad del Aire"));
  Serial.println(F("=========================================="));
  Serial.println();
  
  // Inicializar BME280
  // if (!bme.begin(0x76)) {
  //   Serial.println(F("ERROR: BME280 no detectado!"));
  //   while (1) delay(10);
  // }
  Serial.println(F("BME280 inicializado (simulado)"));
  
  // Inicializar arrays
  for (int i = 0; i < AVG_SAMPLES; i++) {
    pm25_avg[i] = 0;
    pm10_avg[i] = 0;
  }
  
  Serial.println(F("Sistema listo para monitoreo exterior"));
  Serial.println();
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Leer sensores cada 30 segundos
  if (currentMillis - lastRead >= READ_INTERVAL) {
    lastRead = currentMillis;
    
    readPMS();
    readBME();
    calculateAQI();
    
    // Guardar en promedio móvil
    pm25_avg[avgIndex] = currentData.pm2_5;
    pm10_avg[avgIndex] = currentData.pm10;
    avgIndex = (avgIndex + 1) % AVG_SAMPLES;
    
    displayData();
  }
  
  // Enviar datos cada 2.5 minutos
  if (currentMillis - lastSend >= SEND_INTERVAL) {
    lastSend = currentMillis;
    sendToSensorCommunity();
  }
  
  delay(1000);
}

void readPMS() {
  // Valores simulados
  currentData.pm1_0 = random(0, 20) + random(0, 100) / 100.0;
  currentData.pm2_5 = random(5, 40) + random(0, 100) / 100.0;
  currentData.pm10 = random(10, 60) + random(0, 100) / 100.0;
  
  // Implementación real con Serial1
  // byte buffer[32];
  // if (Serial1.available() >= 32) {
  //   Serial1.readBytes(buffer, 32);
  //   if (buffer[0] == 0x42 && buffer[1] == 0x4D) {
  //     currentData.pm1_0 = ((buffer[10] << 8) | buffer[11]) / 1.0;
  //     currentData.pm2_5 = ((buffer[12] << 8) | buffer[13]) / 1.0;
  //     currentData.pm10 = ((buffer[14] << 8) | buffer[15]) / 1.0;
  //   }
  // }
}

void readBME() {
  // Valores simulados
  currentData.temperature = 20.0 + random(-50, 100) / 10.0;
  currentData.humidity = 50.0 + random(-200, 200) / 10.0;
  currentData.pressure = 1013.25 + random(-100, 100) / 10.0;
  
  // Implementación real
  // currentData.temperature = bme.readTemperature();
  // currentData.humidity = bme.readHumidity();
  // currentData.pressure = bme.readPressure() / 100.0;
}

void calculateAQI() {
  float pm25 = currentData.pm2_5;
  
  if (pm25 <= 12.0) {
    currentData.aqi = ((50 - 0) / (12.0 - 0)) * (pm25 - 0) + 0;
    currentData.aqi_level = "Good";
  } else if (pm25 <= 35.4) {
    currentData.aqi = ((100 - 51) / (35.4 - 12.1)) * (pm25 - 12.1) + 51;
    currentData.aqi_level = "Moderate";
  } else if (pm25 <= 55.4) {
    currentData.aqi = ((150 - 101) / (55.4 - 35.5)) * (pm25 - 35.5) + 101;
    currentData.aqi_level = "Unhealthy (SG)";
  } else {
    currentData.aqi = 151;
    currentData.aqi_level = "Unhealthy";
  }
}

void displayData() {
  Serial.println(F("========== Lectura de Sensores =========="));
  Serial.print(F("PM1.0:  ")); Serial.print(currentData.pm1_0, 1); Serial.println(F(" μg/m³"));
  Serial.print(F("PM2.5:  ")); Serial.print(currentData.pm2_5, 1); Serial.println(F(" μg/m³"));
  Serial.print(F("PM10:   ")); Serial.print(currentData.pm10, 1); Serial.println(F(" μg/m³"));
  Serial.print(F("Temp:   ")); Serial.print(currentData.temperature, 1); Serial.println(F(" °C"));
  Serial.print(F("Hum:    ")); Serial.print(currentData.humidity, 1); Serial.println(F(" %"));
  Serial.print(F("Pres:   ")); Serial.print(currentData.pressure, 1); Serial.println(F(" hPa"));
  Serial.print(F("AQI:    ")); Serial.print(currentData.aqi); 
  Serial.print(F(" (")); Serial.print(currentData.aqi_level); Serial.println(F(")"));
  Serial.println(F("========================================="));
  Serial.println();
}

void sendToSensorCommunity() {
  // Calcular promedios
  float pm25_mean = 0;
  float pm10_mean = 0;
  
  for (int i = 0; i < AVG_SAMPLES; i++) {
    pm25_mean += pm25_avg[i];
    pm10_mean += pm10_avg[i];
  }
  pm25_mean /= AVG_SAMPLES;
  pm10_mean /= AVG_SAMPLES;
  
  Serial.println(F("========== Enviando a Sensor.Community =========="));
  Serial.print(F("PM2.5 (promedio 2.5min): ")); Serial.println(pm25_mean, 1);
  Serial.print(F("PM10 (promedio 2.5min):  ")); Serial.println(pm10_mean, 1);
  
  // Formato JSON para Sensor.Community
  Serial.println(F("JSON:"));
  Serial.println(F("{"));
  Serial.print(F("  \"software_version\": \"Ornidia-1.0\",\n"));
  Serial.print(F("  \"sensordatavalues\": [\n"));
  Serial.print(F("    {\"value_type\": \"P1\", \"value\": \"")); 
  Serial.print(pm10_mean, 2); Serial.println(F("\"},"));
  Serial.print(F("    {\"value_type\": \"P2\", \"value\": \"")); 
  Serial.print(pm25_mean, 2); Serial.println(F("\"},"));
  Serial.print(F("    {\"value_type\": \"temperature\", \"value\": \"")); 
  Serial.print(currentData.temperature, 2); Serial.println(F("\"},"));
  Serial.print(F("    {\"value_type\": \"humidity\", \"value\": \"")); 
  Serial.print(currentData.humidity, 2); Serial.println(F("\"},"));
  Serial.print(F("    {\"value_type\": \"pressure\", \"value\": \"")); 
  Serial.print(currentData.pressure * 100, 2); Serial.println(F("\"}"));
  Serial.println(F("  ]"));
  Serial.println(F("}"));
  Serial.println(F("================================================="));
  Serial.println();
  
  // Aquí se enviaría via WiFi/Ethernet a:
  // POST https://api.sensor.community/v1/push-sensor-data/
}
