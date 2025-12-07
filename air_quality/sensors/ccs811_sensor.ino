/*
 * Ornidia - Sensor CCS811
 * 
 * Ejemplo de uso del sensor CCS811
 * Mide: eCO₂ (equivalente CO₂) y TVOC (compuestos orgánicos volátiles)
 * 
 * Conexiones:
 * CCS811 VCC -> 3.3V
 * CCS811 GND -> GND
 * CCS811 SDA -> A4 (SDA)
 * CCS811 SCL -> A5 (SCL)
 * CCS811 WAK -> GND (wake - activo bajo)
 * 
 * Librería requerida: Adafruit CCS811 Library
 * arduino-cli lib install "Adafruit CCS811 Library"
 */

#include <Wire.h>
// #include <Adafruit_CCS811.h>

// Adafruit_CCS811 ccs;

void setup() {
  Serial.begin(115200);
  
  Serial.println("CCS811 eCO₂/TVOC Sensor");
  Serial.println("=======================");
  
  // if (!ccs.begin()) {
  //   Serial.println("ERROR: Sensor CCS811 no detectado!");
  //   Serial.println("Verificar conexiones I2C");
  //   while (1) delay(10);
  // }
  
  Serial.println("Sensor CCS811 inicializado");
  Serial.println("IMPORTANTE: Requiere 48 horas de burn-in para lecturas precisas");
  Serial.println("Las lecturas mejorarán con el tiempo de uso");
  
  // Esperar a que el sensor esté listo
  // while (!ccs.available()) delay(10);
  
  delay(1000);
}

void loop() {
  // if (ccs.available()) {
  //   if (!ccs.readData()) {
  //     int eco2 = ccs.geteCO2();
  //     int tvoc = ccs.getTVOC();
  //     
  //     Serial.print("eCO₂: ");
  //     Serial.print(eco2);
  //     Serial.println(" ppm");
  //     
  //     Serial.print("TVOC: ");
  //     Serial.print(tvoc);
  //     Serial.println(" ppb");
  //     
  //     interpretarNiveles(eco2, tvoc);
  //   } else {
  //     Serial.println("ERROR: No se pudo leer del sensor");
  //   }
  // }
  
  // Valores simulados (descomentar arriba para sensor real)
  int eco2 = random(400, 1500);
  int tvoc = random(0, 500);
  
  Serial.print("eCO₂: ");
  Serial.print(eco2);
  Serial.println(" ppm");
  
  Serial.print("TVOC: ");
  Serial.print(tvoc);
  Serial.println(" ppb");
  
  interpretarNiveles(eco2, tvoc);
  
  delay(2000);
}

void interpretarNiveles(int eco2, int tvoc) {
  // Interpretar eCO₂
  Serial.print("CO₂: ");
  if (eco2 < 600) {
    Serial.println("Excelente ✓✓✓");
  } else if (eco2 < 1000) {
    Serial.println("Bueno ✓✓");
  } else if (eco2 < 1500) {
    Serial.println("Moderado ✓");
  } else if (eco2 < 2500) {
    Serial.println("Pobre ⚠");
  } else {
    Serial.println("Muy pobre ⚠⚠");
  }
  
  // Interpretar TVOC
  Serial.print("TVOC: ");
  if (tvoc < 220) {
    Serial.println("Excelente ✓✓✓");
  } else if (tvoc < 660) {
    Serial.println("Bueno ✓✓");
  } else if (tvoc < 2200) {
    Serial.println("Moderado ✓");
  } else {
    Serial.println("Pobre ⚠");
  }
  
  Serial.println("--------------------------------");
}

// Función para calibración con datos ambientales reales
// void calibrateWithEnvironment(float temp, float humidity) {
//   ccs.setEnvironmentalData(humidity, temp);
// }
