#include <Wire.h>   // Librería para I2C
#include <BH1750.h> // Librería del sensor BH1750
#include <DHT.h>    // Librería para el DHT11

// 🔹 Definimos los pines del multiplexor CD4052B
#define S0 D5   // Selección 0
#define S1 D6   // Selección 1
#define ANALOG A0 // Entrada analógica del ESP8266

// 🔹 Definimos los pines del DHT11
#define DHTPIN D7
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// 🔹 Configuración del sensor BH1750 (Luz) por I2C
BH1750 lightMeter;

// 🔹 Pines I2C en ESP8266 (GPIO4 = SDA, GPIO5 = SCL)
#define SDA_PIN D2
#define SCL_PIN D1

// 🔹 Parámetros del divisor de voltaje para medir el panel solar
#define R1 10000.0  // Resistencia 10kΩ
#define R2 2200.0   // Resistencia 2.2kΩ
#define VREF 3.3    // Voltaje de referencia del ESP8266

// 🔹 Estructura para almacenar los datos de los sensores
struct SensorData {
  float panelCurrent;
  float batteryCurrent;
  float loadCurrent;
  float voltage;
  float power;
  float temperature;
  float humidity;
  float lightIntensity;
};

// Función para seleccionar el canal del multiplexor
void selectMuxChannel(int channel) {
  digitalWrite(S0, channel & 0x01);
  digitalWrite(S1, (channel >> 1) & 0x01);
  delay(10); // Pequeño delay para estabilizar la lectura
}

// Función para leer corriente usando el multiplexor (ACS712)
float readCurrent(int channel) {
  selectMuxChannel(channel);
  int rawValue = analogRead(ANALOG);
  float voltage = rawValue * (VREF / 1024.0);
  float current = (voltage - 2.5) / 0.185; // Conversión para ACS712 de 5A
  return current;
}

// Función para leer voltaje del panel (Divisor de voltaje)
float readVoltage() {
  int rawValue = analogRead(ANALOG);
  float voltage = rawValue * (VREF / 1024.0);
  return voltage * ((R1 + R2) / R2);
}

// Función para leer el sensor de luz BH1750
float readLightIntensity() {
  return lightMeter.readLightLevel(); // Devuelve la intensidad en lux
}

// Función para leer todos los sensores
SensorData readSensors() {
  SensorData data;
  data.panelCurrent = readCurrent(0);  // Canal 0: Corriente del panel
  data.batteryCurrent = readCurrent(1); // Canal 1: Corriente de la batería
  data.loadCurrent = readCurrent(2);    // Canal 2: Corriente de carga
  data.voltage = readVoltage();  // Medición de voltaje
  data.power = data.panelCurrent * data.voltage; // Cálculo de potencia
  data.temperature = dht.readTemperature(); // Temperatura DHT11
  data.humidity = dht.readHumidity(); // Humedad DHT11
  data.lightIntensity = readLightIntensity(); // Intensidad de luz en lux
  return data;
}

// Configuración inicial
void setup() {
  Serial.begin(115200);
  Serial.println("Iniciando sensores...");

  // Configuración de pines del multiplexor
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  digitalWrite(S0, LOW);
  digitalWrite(S1, LOW);

  // Inicializar el sensor DHT11
  dht.begin();

  // Inicializar comunicación I2C
  Wire.begin(SDA_PIN, SCL_PIN);
  
  // Inicializar BH1750
  if (!lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    Serial.println("Error iniciando el BH1750!");
  } else {
    Serial.println("BH1750 iniciado correctamente.");
  }
}

// Loop principal: Leer y mostrar los datos
void loop() {
  // Leer sensores
  SensorData data = readSensors();

  // Mostrar datos en el Monitor Serie
  Serial.println("===== Monitoreo Solar =====");
  Serial.print("Corriente Panel: "); Serial.print(data.panelCurrent); Serial.println(" A");
  Serial.print("Corriente Batería: "); Serial.print(data.batteryCurrent); Serial.println(" A");
  Serial.print("Corriente Carga: "); Serial.print(data.loadCurrent); Serial.println(" A");
  Serial.print("Voltaje Panel: "); Serial.print(data.voltage); Serial.println(" V");
  Serial.print("Potencia Generada: "); Serial.print(data.power); Serial.println(" W");
  Serial.print("Temperatura: "); Serial.print(data.temperature); Serial.println(" °C");
  Serial.print("Humedad: "); Serial.print(data.humidity); Serial.println(" %");
  Serial.print("Intensidad de Luz: "); Serial.print(data.lightIntensity); Serial.println(" lux");
  Serial.println("===========================");
  delay(2000); // Espera de 2 segundos antes de la siguiente lectura
}
