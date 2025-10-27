#include <DHT.h>
#include <ACS712.h>
#include <Wire.h>

// 🔹 Pines del Multiplexor CD4052B
#define S0 D5   // Selección 0
#define S1 D6   // Selección 1
#define ANALOG A0 // Única entrada analógica en ESP8266

// 🔹 Pines del DHT11
#define DHTPIN D7
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// 🔹 Parámetros del divisor de voltaje
#define R1 10000.0  // Resistencia 10kΩ
#define R2 2200.0   // Resistencia 2.2kΩ
#define VREF 3.3    // Voltaje de referencia del ESP8266

// 🔹 Estructura para almacenar datos de sensores
struct SensorData {
  float panelCurrent;
  float batteryCurrent;
  float loadCurrent;
  float voltage;
  float power;
  float temperature;
  float humidity;
};

// Función para seleccionar el canal del multiplexor
void selectMuxChannel(int channel) {
  digitalWrite(S0, channel & 0x01);
  digitalWrite(S1, (channel >> 1) & 0x01);
  delay(10); // Pequeño delay para estabilizar la lectura
}

// Función para leer corriente (usando el multiplexor)
float readCurrent(int channel) {
  selectMuxChannel(channel);
  int rawValue = analogRead(ANALOG);
  float voltage = rawValue * (VREF / 1024.0);
  float current = (voltage - 2.5) / 0.185; // Conversión para ACS712 de 5A
  return current;
}

// Función para leer voltaje del panel
float readVoltage() {
  int rawValue = analogRead(ANALOG);
  float voltage = rawValue * (VREF / 1024.0);
  return voltage * ((R1 + R2) / R2);
}

// Función para leer todos los sensores
SensorData readSensors() {
  SensorData data;
  data.panelCurrent = readCurrent(0);
  data.batteryCurrent = readCurrent(1);
  data.loadCurrent = readCurrent(2);
  data.voltage = readVoltage();
  data.power = data.panelCurrent * data.voltage;
  data.temperature = dht.readTemperature();
  data.humidity = dht.readHumidity();
  return data;
}

void setup() {
  Serial.begin(115200);
  Serial.println("Iniciando...");

  // Configuración de pines
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  digitalWrite(S0, LOW);
  digitalWrite(S1, LOW);
  
  // Inicializar el sensor DHT
  dht.begin();
}

void loop() {
  // Leer sensores y mostrar en Serial
  SensorData data = readSensors();
  Serial.print("Panel: "); Serial.print(data.panelCurrent); Serial.println(" A");
  Serial.print("Batería: "); Serial.print(data.batteryCurrent); Serial.println(" A");
  Serial.print("Carga: "); Serial.print(data.loadCurrent); Serial.println(" A");
  Serial.print("Voltaje: "); Serial.print(data.voltage); Serial.println(" V");
  Serial.print("Potencia: "); Serial.print(data.power); Serial.println(" W");
  Serial.print("Temperatura: "); Serial.print(data.temperature); Serial.println(" °C");
  Serial.print("Humedad: "); Serial.print(data.humidity); Serial.println(" %");
  Serial.println("------------------------");
  delay(2000); // Esperar 2 segundos
}
