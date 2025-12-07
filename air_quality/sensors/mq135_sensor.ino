/*
 * Ornidia - Sensor MQ-135
 * 
 * Ejemplo de uso del sensor de calidad del aire MQ-135
 * Detecta: NH3, NOx, alcohol, benzeno, humo, CO₂
 * 
 * Conexiones:
 * MQ-135 VCC  -> 5V
 * MQ-135 GND  -> GND
 * MQ-135 AOUT -> A0 (salida analógica)
 * MQ-135 DOUT -> D8 (salida digital - opcional)
 * 
 * IMPORTANTE: Requiere pre-calentamiento de 24-48 horas para lecturas estables
 */

const int MQ135_APIN = A0;
const int MQ135_DPIN = 8;

// Constantes de calibración (ajustar según calibración en aire limpio)
const float RL_VALUE = 10.0;        // Resistencia de carga en kOhm
const float RO_CLEAN_AIR = 3.6;     // Resistencia del sensor en aire limpio (kOhm)
const float ATMOCO2 = 400.0;        // Concentración atmosférica de CO₂ (ppm)

// Parámetros de la curva de CO₂ (datasheet MQ-135)
const float PARA = 116.6020682;
const float PARB = 2.769034857;

// Variables para promedio móvil
const int NUM_READINGS = 10;
int readings[NUM_READINGS];
int readIndex = 0;
int total = 0;
int average = 0;

void setup() {
  Serial.begin(115200);
  pinMode(MQ135_DPIN, INPUT);
  
  Serial.println("Sensor MQ-135 - Calidad del Aire");
  Serial.println("=================================");
  Serial.println();
  Serial.println("ADVERTENCIA:");
  Serial.println("- Requiere 24-48h de pre-calentamiento inicial");
  Serial.println("- Las lecturas se estabilizan después del warm-up");
  Serial.println("- Calibrar R0 en aire limpio para mejor precisión");
  Serial.println();
  
  // Inicializar array de lecturas
  for (int i = 0; i < NUM_READINGS; i++) {
    readings[i] = 0;
  }
  
  delay(2000);
}

void loop() {
  // Leer valor analógico
  int sensorValue = analogRead(MQ135_APIN);
  
  // Promedio móvil
  total = total - readings[readIndex];
  readings[readIndex] = sensorValue;
  total = total + readings[readIndex];
  readIndex = (readIndex + 1) % NUM_READINGS;
  average = total / NUM_READINGS;
  
  // Convertir a voltaje
  float voltage = average * (5.0 / 1023.0);
  
  // Calcular resistencia del sensor (Rs)
  float rs = ((5.0 * RL_VALUE) / voltage) - RL_VALUE;
  
  // Calcular ratio Rs/R0
  float ratio = rs / RO_CLEAN_AIR;
  
  // Calcular concentración de CO₂ (ppm)
  float ppm = PARA * pow(ratio, -PARB);
  
  // Mostrar resultados
  Serial.println("=== Lectura MQ-135 ===");
  Serial.print("Valor ADC: ");
  Serial.println(average);
  
  Serial.print("Voltaje: ");
  Serial.print(voltage);
  Serial.println(" V");
  
  Serial.print("Rs: ");
  Serial.print(rs);
  Serial.println(" kΩ");
  
  Serial.print("Rs/R0: ");
  Serial.println(ratio);
  
  Serial.print("CO₂ estimado: ");
  Serial.print(ppm);
  Serial.println(" ppm");
  
  // Leer salida digital (umbral)
  int digitalValue = digitalRead(MQ135_DPIN);
  Serial.print("Umbral digital: ");
  Serial.println(digitalValue == HIGH ? "NO SUPERADO" : "SUPERADO ⚠");
  
  // Interpretación de calidad
  interpretarCalidad(ppm);
  
  Serial.println("======================\n");
  
  delay(2000);
}

void interpretarCalidad(float co2_ppm) {
  Serial.print("Calidad del aire: ");
  
  if (co2_ppm < 600) {
    Serial.println("Excelente ✓✓✓");
  } else if (co2_ppm < 1000) {
    Serial.println("Bueno ✓✓");
  } else if (co2_ppm < 1500) {
    Serial.println("Aceptable ✓");
  } else if (co2_ppm < 2000) {
    Serial.println("Moderado - Ventilar ⚠");
  } else if (co2_ppm < 5000) {
    Serial.println("Pobre - Ventilar inmediatamente ⚠⚠");
  } else {
    Serial.println("Peligroso ⚠⚠⚠");
  }
}

// Función de calibración en aire limpio
float calibrateR0() {
  Serial.println("Iniciando calibración...");
  Serial.println("Asegúrese de estar en aire limpio");
  Serial.println("Leyendo 50 muestras...");
  
  float rs_sum = 0;
  for (int i = 0; i < 50; i++) {
    int sensorValue = analogRead(MQ135_APIN);
    float voltage = sensorValue * (5.0 / 1023.0);
    float rs = ((5.0 * RL_VALUE) / voltage) - RL_VALUE;
    rs_sum += rs;
    delay(100);
  }
  
  float rs_avg = rs_sum / 50.0;
  float r0 = rs_avg / pow(ATMOCO2 / PARA, 1.0 / PARB);
  
  Serial.print("R0 calibrado: ");
  Serial.print(r0);
  Serial.println(" kΩ");
  Serial.println("Actualizar RO_CLEAN_AIR en el código con este valor");
  
  return r0;
}
