/*
 * Ornidia - Sensor MH-Z19B
 * 
 * Ejemplo de uso del sensor de CO₂ NDIR MH-Z19B
 * Mide: CO₂ (400-5000 ppm)
 * 
 * Conexiones:
 * MH-Z19B VIN -> 5V (4.5-5.5V)
 * MH-Z19B GND -> GND
 * MH-Z19B TX  -> Arduino RX (Pin 2 SoftwareSerial)
 * MH-Z19B RX  -> Arduino TX (Pin 3 SoftwareSerial)
 * 
 * Librería requerida: MH-Z19
 * arduino-cli lib install "MH-Z19"
 */

/*
 * Librería requerida: MH-Z19
 * arduino-cli lib install "MH-Z19"
 * 
 * Descomentar las siguientes líneas después de instalar la librería
 */

#include <SoftwareSerial.h>
// #include <MHZ19.h>

SoftwareSerial mhz19Serial(2, 3); // RX, TX
// MHZ19 mhz19;

unsigned long getDataTimer = 0;

void setup() {
  Serial.begin(115200);
  mhz19Serial.begin(9600);
  
  // mhz19.begin(mhz19Serial);
  // mhz19.autoCalibration(true); // Auto-calibración en aire exterior (400ppm)
  
  Serial.println("MH-Z19B CO₂ Sensor");
  Serial.println("==================");
  Serial.println("Esperando 3 minutos para warm-up...");
  
  delay(3000);  // En producción: delay(180000) para 3 minutos
}

void loop() {
  if (millis() - getDataTimer >= 5000) {
    // int co2 = mhz19.getCO2();
    // int temp = mhz19.getTemperature();
    
    // Valores simulados (descomentar arriba para sensor real)
    int co2 = random(400, 1200);
    int temp = random(20, 25);
    
    Serial.print("CO₂: ");
    Serial.print(co2);
    Serial.println(" ppm");
    
    Serial.print("Temperatura: ");
    Serial.print(temp);
    Serial.println(" °C");
    
    // Interpretar nivel de CO₂
    if (co2 < 400) {
      Serial.println("Estado: Exterior/Referencia");
    } else if (co2 < 1000) {
      Serial.println("Estado: Bueno ✓");
    } else if (co2 < 2000) {
      Serial.println("Estado: Ventilación recomendada ⚠");
    } else if (co2 < 5000) {
      Serial.println("Estado: Aire viciado - Ventilar! ⚠⚠");
    } else {
      Serial.println("Estado: PELIGROSO ⚠⚠⚠");
    }
    
    Serial.println("--------------------------------");
    
    getDataTimer = millis();
  }
}

// Calibración manual a 400ppm (ejecutar en aire exterior limpio)
void calibrateTo400ppm() {
  Serial.println("Iniciando calibración a 400ppm...");
  // mhz19.calibrate();
  Serial.println("Calibración completada!");
}

// Obtener rango de medición
void getRange() {
  // int range = mhz19.getRange();
  // Serial.print("Rango: 0-");
  // Serial.print(range);
  // Serial.println(" ppm");
}
