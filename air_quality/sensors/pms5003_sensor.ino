/*
 * Ornidia - Sensor PMS5003/PMS7003
 * 
 * Ejemplo de uso del sensor láser de partículas Plantower PMS5003/PMS7003
 * Mide: PM1.0, PM2.5, PM10
 * 
 * Conexiones:
 * PMS5003 VCC -> 5V
 * PMS5003 GND -> GND
 * PMS5003 TX  -> Arduino RX (Pin 10 SoftwareSerial)
 * PMS5003 RX  -> Arduino TX (Pin 11 SoftwareSerial - opcional)
 * PMS5003 SET -> Pin 12 (Sleep/Wake - opcional)
 * 
 * Librería requerida: PMS Library
 * arduino-cli lib install "PMS Library"
 */

#include <SoftwareSerial.h>
// #include <PMS.h>

// Configuración SoftwareSerial para PMS5003
SoftwareSerial pmsSerial(10, 11); // RX, TX
// PMS pms(pmsSerial);
// PMS::DATA data;

const int PMS_SET_PIN = 12;  // Pin para sleep/wake (opcional)

void setup() {
  Serial.begin(115200);
  pmsSerial.begin(9600);
  
  pinMode(PMS_SET_PIN, OUTPUT);
  digitalWrite(PMS_SET_PIN, HIGH); // Wake up sensor
  
  Serial.println("PMS5003 Particle Sensor");
  Serial.println("=======================");
  
  // Esperar estabilización del sensor
  Serial.println("Esperando 30 segundos para estabilización...");
  delay(30000);
}

void loop() {
  // Leer datos del sensor
  // if (pms.readUntil(data)) {
  //   Serial.println("=== Concentración Estándar (CF=1) ===");
  //   Serial.print("PM1.0: "); Serial.print(data.PM_SP_UG_1_0); Serial.println(" μg/m³");
  //   Serial.print("PM2.5: "); Serial.print(data.PM_SP_UG_2_5); Serial.println(" μg/m³");
  //   Serial.print("PM10:  "); Serial.print(data.PM_SP_UG_10_0); Serial.println(" μg/m³");
  //   
  //   Serial.println("\n=== Concentración Atmosférica ===");
  //   Serial.print("PM1.0: "); Serial.print(data.PM_AE_UG_1_0); Serial.println(" μg/m³");
  //   Serial.print("PM2.5: "); Serial.print(data.PM_AE_UG_2_5); Serial.println(" μg/m³");
  //   Serial.print("PM10:  "); Serial.print(data.PM_AE_UG_10_0); Serial.println(" μg/m³");
  //   Serial.println("=====================================\n");
  // }
  
  // Ejemplo con valores simulados (descomentar líneas arriba para usar sensor real)
  Serial.println("=== PMS5003 Reading (Simulated) ===");
  Serial.print("PM1.0: "); Serial.print(random(0, 15)); Serial.println(" μg/m³");
  Serial.print("PM2.5: "); Serial.print(random(5, 25)); Serial.println(" μg/m³");
  Serial.print("PM10:  "); Serial.print(random(10, 40)); Serial.println(" μg/m³");
  Serial.println("====================================\n");
  
  delay(2000);  // Leer cada 2 segundos (recomendado: 60s para ahorro de energía)
}

// Función para activar modo sleep (ahorro de energía)
void pmsSleep() {
  digitalWrite(PMS_SET_PIN, LOW);
  delay(100);
}

// Función para despertar sensor
void pmsWake() {
  digitalWrite(PMS_SET_PIN, HIGH);
  delay(30000);  // Esperar 30s para estabilización
}
