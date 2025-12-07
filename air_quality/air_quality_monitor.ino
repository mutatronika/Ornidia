/*
 * Ornidia - Sistema de Medición de Calidad del Aire
 * 
 * Monitor completo de calidad del aire con soporte para múltiples sensores:
 * - PMS5003/PMS7003: Partículas PM1.0, PM2.5, PM10
 * - MH-Z19B: CO₂ (NDIR)
 * - CCS811: eCO₂ y TVOC
 * - BME280: Temperatura, humedad, presión
 * - MQ-135: Calidad del aire general
 * 
 * Plataformas: Arduino Uno/Mega/Nano, Raspberry Pi (con adaptación)
 * 
 * Autor: Ornidia Project
 * Fecha: 2025-10-27
 * Versión: 1.0
 */

#include <Wire.h>
#include <SoftwareSerial.h>

// Configuración de sensores (comentar los que no se usen)
#define USE_PMS5003      // Sensor de partículas láser
#define USE_MHZ19B       // Sensor CO₂ NDIR
#define USE_CCS811       // Sensor eCO₂/TVOC
#define USE_BME280       // Sensor temp/hum/presión
#define USE_MQ135        // Sensor calidad aire analógico

// Librerías de sensores (instalar según sensores usados)
#ifdef USE_PMS5003
  // #include <PMS.h>
  // PMS pms(Serial1);  // Para Arduino Mega (Hardware Serial)
  // Para Arduino Uno/Nano usar SoftwareSerial
  SoftwareSerial pmsSerial(10, 11); // RX, TX
#endif

#ifdef USE_MHZ19B
  // #include <MHZ19.h>
  SoftwareSerial mhz19Serial(2, 3); // RX, TX
  // MHZ19 mhz19;
#endif

#ifdef USE_CCS811
  // #include <Adafruit_CCS811.h>
  // Adafruit_CCS811 ccs;
#endif

#ifdef USE_BME280
  // #include <Adafruit_BME280.h>
  // Adafruit_BME280 bme;
#endif

#ifdef USE_MQ135
  const int MQ135_PIN = A0;
  const int RL_VALUE = 10;  // kOhm - resistencia de carga
  const float RO_CLEAN_AIR = 3.6; // kOhm - R0 en aire limpio (calibrar)
#endif

// Pines de configuración
const int LED_STATUS = 13;

// Variables de medición
struct AirQualityData {
  // Partículas (μg/m³)
  float pm1_0;
  float pm2_5;
  float pm10;
  
  // Gases
  int co2;          // ppm
  int tvoc;         // ppb
  int eco2;         // ppm (estimado)
  
  // Ambientales
  float temperature;  // °C
  float humidity;     // %
  float pressure;     // hPa
  
  // Índices calculados
  int aqi;            // Air Quality Index
  String aqi_level;   // Nivel de calidad
};

AirQualityData airData;

// Timing
unsigned long lastReadPM = 0;
unsigned long lastReadGas = 0;
unsigned long lastReadEnv = 0;
const unsigned long PM_INTERVAL = 60000;    // 60 segundos
const unsigned long GAS_INTERVAL = 5000;    // 5 segundos
const unsigned long ENV_INTERVAL = 30000;   // 30 segundos

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  
  pinMode(LED_STATUS, OUTPUT);
  
  Serial.println(F("================================="));
  Serial.println(F("  Ornidia Air Quality Monitor"));
  Serial.println(F("================================="));
  Serial.println();
  
  // Inicializar I2C
  Wire.begin();
  
  // Inicializar sensores
  initializeSensors();
  
  Serial.println(F("Sistema iniciado correctamente"));
  Serial.println(F("Esperando lecturas estables..."));
  Serial.println();
  
  delay(2000);
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Leer sensores de partículas (cada 60 segundos)
  if (currentMillis - lastReadPM >= PM_INTERVAL) {
    lastReadPM = currentMillis;
    readPMSensor();
  }
  
  // Leer sensores de gases (cada 5 segundos)
  if (currentMillis - lastReadGas >= GAS_INTERVAL) {
    lastReadGas = currentMillis;
    readGasSensors();
  }
  
  // Leer sensores ambientales (cada 30 segundos)
  if (currentMillis - lastReadEnv >= ENV_INTERVAL) {
    lastReadEnv = currentMillis;
    readEnvironmentalSensors();
  }
  
  // Calcular AQI
  calculateAQI();
  
  // Mostrar datos
  displayData();
  
  // Verificar alertas
  checkAlerts();
  
  delay(1000);
}

void initializeSensors() {
  Serial.println(F("Inicializando sensores..."));
  
  #ifdef USE_PMS5003
    pmsSerial.begin(9600);
    Serial.println(F("  [OK] PMS5003 inicializado"));
  #endif
  
  #ifdef USE_MHZ19B
    mhz19Serial.begin(9600);
    // mhz19.begin(mhz19Serial);
    // mhz19.autoCalibration(true);
    Serial.println(F("  [OK] MH-Z19B inicializado"));
  #endif
  
  #ifdef USE_CCS811
    // if (!ccs.begin()) {
    //   Serial.println(F("  [ERROR] CCS811 no detectado"));
    // } else {
    //   Serial.println(F("  [OK] CCS811 inicializado"));
    //   while (!ccs.available());
    // }
    Serial.println(F("  [OK] CCS811 inicializado (simulado)"));
  #endif
  
  #ifdef USE_BME280
    // if (!bme.begin(0x76)) {
    //   Serial.println(F("  [ERROR] BME280 no detectado"));
    // } else {
    //   Serial.println(F("  [OK] BME280 inicializado"));
    // }
    Serial.println(F("  [OK] BME280 inicializado (simulado)"));
  #endif
  
  #ifdef USE_MQ135
    pinMode(MQ135_PIN, INPUT);
    Serial.println(F("  [OK] MQ-135 configurado"));
    Serial.println(F("  [INFO] MQ-135 requiere 24-48h de pre-calentamiento"));
  #endif
  
  Serial.println();
}

void readPMSensor() {
  #ifdef USE_PMS5003
    // PMS::DATA data;
    // if (pms.readUntil(data)) {
    //   airData.pm1_0 = data.PM_AE_UG_1_0;
    //   airData.pm2_5 = data.PM_AE_UG_2_5;
    //   airData.pm10 = data.PM_AE_UG_10_0;
    // }
    
    // Valores simulados para ejemplo
    airData.pm1_0 = random(0, 15) + random(0, 100) / 100.0;
    airData.pm2_5 = random(5, 25) + random(0, 100) / 100.0;
    airData.pm10 = random(10, 40) + random(0, 100) / 100.0;
  #else
    airData.pm1_0 = 0;
    airData.pm2_5 = 0;
    airData.pm10 = 0;
  #endif
}

void readGasSensors() {
  #ifdef USE_MHZ19B
    // int co2 = mhz19.getCO2();
    // if (co2 > 0) {
    //   airData.co2 = co2;
    // }
    
    // Valor simulado
    airData.co2 = random(400, 800);
  #else
    airData.co2 = 0;
  #endif
  
  #ifdef USE_CCS811
    // if (ccs.available()) {
    //   if (!ccs.readData()) {
    //     airData.eco2 = ccs.geteCO2();
    //     airData.tvoc = ccs.getTVOC();
    //   }
    // }
    
    // Valores simulados
    airData.eco2 = random(400, 1000);
    airData.tvoc = random(0, 300);
  #else
    airData.eco2 = 0;
    airData.tvoc = 0;
  #endif
  
  #ifdef USE_MQ135
    // Leer sensor analógico MQ-135
    int sensorValue = analogRead(MQ135_PIN);
    float voltage = sensorValue * (5.0 / 1023.0);
    // Cálculo simplificado (requiere calibración)
    float rs = ((5.0 * RL_VALUE) / voltage) - RL_VALUE;
    float ratio = rs / RO_CLEAN_AIR;
    // Conversión a ppm (fórmula aproximada)
    // float ppm = pow(10, ((log10(ratio) - 0.76) / -0.33));
  #endif
}

void readEnvironmentalSensors() {
  #ifdef USE_BME280
    // airData.temperature = bme.readTemperature();
    // airData.humidity = bme.readHumidity();
    // airData.pressure = bme.readPressure() / 100.0;
    
    // Valores simulados
    airData.temperature = 22.0 + random(-20, 50) / 10.0;
    airData.humidity = 50.0 + random(-100, 200) / 10.0;
    airData.pressure = 1013.25 + random(-50, 50) / 10.0;
  #else
    airData.temperature = 0;
    airData.humidity = 0;
    airData.pressure = 0;
  #endif
}

void calculateAQI() {
  // Cálculo AQI basado en PM2.5 (EPA standard)
  float pm25 = airData.pm2_5;
  
  float cLow, cHigh;
  int iLow, iHigh;
  
  if (pm25 <= 12.0) {
    cLow = 0.0; cHigh = 12.0;
    iLow = 0; iHigh = 50;
    airData.aqi_level = "Good";
  } else if (pm25 <= 35.4) {
    cLow = 12.1; cHigh = 35.4;
    iLow = 51; iHigh = 100;
    airData.aqi_level = "Moderate";
  } else if (pm25 <= 55.4) {
    cLow = 35.5; cHigh = 55.4;
    iLow = 101; iHigh = 150;
    airData.aqi_level = "Unhealthy (SG)";
  } else if (pm25 <= 150.4) {
    cLow = 55.5; cHigh = 150.4;
    iLow = 151; iHigh = 200;
    airData.aqi_level = "Unhealthy";
  } else if (pm25 <= 250.4) {
    cLow = 150.5; cHigh = 250.4;
    iLow = 201; iHigh = 300;
    airData.aqi_level = "Very Unhealthy";
  } else {
    cLow = 250.5; cHigh = 500.4;
    iLow = 301; iHigh = 500;
    airData.aqi_level = "Hazardous";
  }
  
  // Fórmula AQI
  float aqi = ((iHigh - iLow) / (cHigh - cLow)) * (pm25 - cLow) + iLow;
  airData.aqi = round(aqi);
}

void displayData() {
  static unsigned long lastDisplay = 0;
  if (millis() - lastDisplay < 10000) return;  // Mostrar cada 10 segundos
  lastDisplay = millis();
  
  Serial.println(F("========== Air Quality Data =========="));
  
  #ifdef USE_PMS5003
    Serial.println(F("--- Partículas ---"));
    Serial.print(F("PM1.0:  ")); Serial.print(airData.pm1_0); Serial.println(F(" μg/m³"));
    Serial.print(F("PM2.5:  ")); Serial.print(airData.pm2_5); Serial.println(F(" μg/m³"));
    Serial.print(F("PM10:   ")); Serial.print(airData.pm10); Serial.println(F(" μg/m³"));
    Serial.println();
  #endif
  
  #ifdef USE_MHZ19B
    Serial.println(F("--- Gases ---"));
    Serial.print(F("CO₂:    ")); Serial.print(airData.co2); Serial.println(F(" ppm"));
  #endif
  
  #ifdef USE_CCS811
    Serial.print(F("eCO₂:   ")); Serial.print(airData.eco2); Serial.println(F(" ppm"));
    Serial.print(F("TVOC:   ")); Serial.print(airData.tvoc); Serial.println(F(" ppb"));
    Serial.println();
  #endif
  
  #ifdef USE_BME280
    Serial.println(F("--- Ambiente ---"));
    Serial.print(F("Temp:   ")); Serial.print(airData.temperature); Serial.println(F(" °C"));
    Serial.print(F("Hum:    ")); Serial.print(airData.humidity); Serial.println(F(" %"));
    Serial.print(F("Pres:   ")); Serial.print(airData.pressure); Serial.println(F(" hPa"));
    Serial.println();
  #endif
  
  Serial.println(F("--- Índice de Calidad ---"));
  Serial.print(F("AQI:    ")); Serial.print(airData.aqi);
  Serial.print(F(" (")); Serial.print(airData.aqi_level); Serial.println(F(")"));
  
  Serial.println(F("======================================"));
  Serial.println();
}

void checkAlerts() {
  static bool alertActive = false;
  
  // Alertas por PM2.5
  if (airData.pm2_5 > 55.4) {
    if (!alertActive) {
      Serial.println(F("⚠️  ALERTA: PM2.5 en nivel dañino!"));
      alertActive = true;
      digitalWrite(LED_STATUS, HIGH);
    }
  } else if (airData.pm2_5 > 35.4) {
    if (!alertActive) {
      Serial.println(F("⚠️  PRECAUCIÓN: PM2.5 en nivel dañino para grupos sensibles"));
      alertActive = true;
      digitalWrite(LED_STATUS, HIGH);
    }
  } else {
    alertActive = false;
    digitalWrite(LED_STATUS, LOW);
  }
  
  // Alertas por CO₂ (interior)
  if (airData.co2 > 2000) {
    Serial.println(F("⚠️  ALERTA: CO₂ alto - Ventilar inmediatamente!"));
  } else if (airData.co2 > 1000) {
    Serial.println(F("⚠️  ADVERTENCIA: CO₂ elevado - Recomienda ventilación"));
  }
  
  // Alertas por TVOC
  if (airData.tvoc > 2200) {
    Serial.println(F("⚠️  ALERTA: TVOC alto - Calidad del aire pobre"));
  }
}

/*
 * Funciones adicionales para implementar:
 * 
 * - sendToCloud(): Enviar datos a ThingSpeak, Sensor.Community, etc.
 * - saveToSD(): Almacenar datos en tarjeta SD
 * - displayOLED(): Mostrar en pantalla OLED local
 * - webServer(): Servidor web para dashboard local
 * - mqttPublish(): Publicar a broker MQTT
 * - calculateIAQ(): Calcular Indoor Air Quality Index
 * - historicalAverage(): Promedios móviles de 1h, 24h
 */
