/*
 * Ornidia - Monitor Básico de Calidad del Aire
 * 
 * Monitor simple con sensor de partículas PMS5003 y sensor CO₂ MH-Z19B
 * Ideal para principiantes y monitoreo básico de interior
 * 
 * Hardware:
 * - Arduino Uno/Nano
 * - PMS5003 (PM2.5 y PM10)
 * - MH-Z19B (CO₂)
 * 
 * Conexiones:
 * PMS5003:
 *   VCC -> 5V
 *   GND -> GND
 *   TX  -> D10 (SoftwareSerial RX)
 * 
 * MH-Z19B:
 *   VIN -> 5V
 *   GND -> GND
 *   TX  -> D2 (SoftwareSerial RX)
 *   RX  -> D3 (SoftwareSerial TX)
 */

#include <SoftwareSerial.h>

// Configuración PMS5003
SoftwareSerial pmsSerial(10, 11); // RX, TX

// Configuración MH-Z19B
SoftwareSerial co2Serial(2, 3);   // RX, TX

// Variables de medición
struct {
  float pm25;
  float pm10;
  int co2;
  int aqi;
  String status;
} airData;

unsigned long lastRead = 0;
const unsigned long READ_INTERVAL = 60000; // 60 segundos

void setup() {
  Serial.begin(115200);
  pmsSerial.begin(9600);
  co2Serial.begin(9600);
  
  Serial.println(F("======================================"));
  Serial.println(F("  Monitor Básico de Calidad del Aire"));
  Serial.println(F("======================================"));
  Serial.println();
  Serial.println(F("Sensores:"));
  Serial.println(F("  - PMS5003 (PM2.5, PM10)"));
  Serial.println(F("  - MH-Z19B (CO₂)"));
  Serial.println();
  Serial.println(F("Inicializando..."));
  
  delay(3000); // Esperar estabilización
  Serial.println(F("Listo!"));
  Serial.println();
}

void loop() {
  unsigned long currentMillis = millis();
  
  if (currentMillis - lastRead >= READ_INTERVAL) {
    lastRead = currentMillis;
    
    // Leer sensores
    readPM();
    readCO2();
    
    // Calcular AQI
    calculateAQI();
    
    // Mostrar datos
    displayData();
    
    // Verificar alertas
    checkAlerts();
  }
  
  delay(1000);
}

void readPM() {
  // Simulación de lectura (reemplazar con lectura real)
  airData.pm25 = random(5, 35) + random(0, 100) / 100.0;
  airData.pm10 = random(10, 50) + random(0, 100) / 100.0;
  
  // Para implementación real, descomentar:
  // byte buffer[32];
  // if (pmsSerial.available() >= 32) {
  //   pmsSerial.readBytes(buffer, 32);
  //   if (buffer[0] == 0x42 && buffer[1] == 0x4D) {
  //     airData.pm25 = (buffer[12] << 8) | buffer[13];
  //     airData.pm10 = (buffer[14] << 8) | buffer[15];
  //   }
  // }
}

void readCO2() {
  // Simulación de lectura
  airData.co2 = random(400, 1200);
  
  // Para implementación real con librería MH-Z19:
  // airData.co2 = mhz19.getCO2();
}

void calculateAQI() {
  float pm25 = airData.pm25;
  
  if (pm25 <= 12.0) {
    airData.aqi = map(pm25 * 10, 0, 120, 0, 50);
    airData.status = "Good";
  } else if (pm25 <= 35.4) {
    airData.aqi = map(pm25 * 10, 121, 354, 51, 100);
    airData.status = "Moderate";
  } else if (pm25 <= 55.4) {
    airData.aqi = map(pm25 * 10, 355, 554, 101, 150);
    airData.status = "Unhealthy (SG)";
  } else {
    airData.aqi = 151;
    airData.status = "Unhealthy";
  }
}

void displayData() {
  Serial.println(F("========== Calidad del Aire =========="));
  Serial.print(F("PM2.5:  "));
  Serial.print(airData.pm25);
  Serial.println(F(" μg/m³"));
  
  Serial.print(F("PM10:   "));
  Serial.print(airData.pm10);
  Serial.println(F(" μg/m³"));
  
  Serial.print(F("CO₂:    "));
  Serial.print(airData.co2);
  Serial.println(F(" ppm"));
  
  Serial.println();
  Serial.print(F("AQI:    "));
  Serial.print(airData.aqi);
  Serial.print(F(" ("));
  Serial.print(airData.status);
  Serial.println(F(")"));
  Serial.println(F("======================================"));
  Serial.println();
}

void checkAlerts() {
  // Alerta PM2.5
  if (airData.pm25 > 35.4) {
    Serial.println(F("⚠️  ALERTA: PM2.5 alto - Reducir actividad exterior"));
  }
  
  // Alerta CO₂
  if (airData.co2 > 1000) {
    Serial.println(F("⚠️  ALERTA: CO₂ elevado - Ventilar ambiente"));
  }
}
