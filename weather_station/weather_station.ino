/*
 * Estación Meteorológica Completa
 * 
 * Este sketch lee múltiples sensores meteorológicos y calcula
 * parámetros derivados para predicción del tiempo.
 * 
 * Hardware requerido:
 * - ESP8266 / ESP32 / Arduino Mega
 * - BMP180 (presión y temperatura)
 * - DHT22 (temperatura y humedad)
 * - BH1750 (luz ambiente)
 * - Anemómetro (opcional)
 * - Pluviómetro (opcional)
 * 
 * Librerías:
 * - BMP180I2C
 * - DHT sensor library
 * - BH1750
 * - Wire
 */

#include <Wire.h>
#include <BMP180I2C.h>
#include <DHT.h>
#include <BH1750.h>

// ===== CONFIGURACIÓN DE PINES =====
#define DHTPIN D7           // Pin del DHT22
#define DHTTYPE DHT22       // Tipo de sensor DHT
#define WIND_SPEED_PIN D5   // Anemómetro (interrupciones)
#define RAIN_GAUGE_PIN D6   // Pluviómetro (interrupciones)

// ===== CONFIGURACIÓN I2C =====
#define SDA_PIN D2          // GPIO4
#define SCL_PIN D1          // GPIO5
#define BMP180_ADDR 0x77    // Dirección I2C del BMP180

// ===== CONSTANTES =====
#define ALTITUDE 100.0      // Altitud en metros (ajustar según ubicación)
#define RAIN_MM_PER_TIP 0.2794  // mm por pulso del pluviómetro

// ===== OBJETOS DE SENSORES =====
BMP180I2C bmp(BMP180_ADDR);
DHT dht(DHTPIN, DHTTYPE);
BH1750 lightMeter;

// ===== ESTRUCTURA DE DATOS =====
struct WeatherData {
  // Mediciones directas
  float temperature;       // °C
  float humidity;          // %
  float pressure;          // hPa
  float lightIntensity;    // lux
  float windSpeed;         // km/h
  float rainfall;          // mm
  
  // Datos calculados
  float dewPoint;          // Punto de rocío (°C)
  float heatIndex;         // Índice de calor (°C)
  float windChill;         // Sensación térmica (°C)
  float seaLevelPressure;  // Presión a nivel del mar (hPa)
  
  // Tendencias
  float pressureTrend;     // Cambio de presión (hPa/hora)
  String forecast;         // Predicción simple
};

WeatherData currentWeather;
WeatherData previousWeather;

// ===== VARIABLES DE INTERRUPCIONES =====
volatile int windPulseCount = 0;
volatile int rainTipCount = 0;
unsigned long lastWindTime = 0;
unsigned long lastRainTime = 0;

// ===== SETUP =====
void setup() {
  Serial.begin(115200);
  delay(100);
  
  Serial.println("\n=====================================");
  Serial.println("  ESTACIÓN METEOROLÓGICA COMPLETA");
  Serial.println("=====================================\n");
  
  // Inicializar I2C
  Wire.begin(SDA_PIN, SCL_PIN);
  Serial.println("✓ Bus I2C inicializado");
  
  // Inicializar BMP180
  if (bmp.begin()) {
    Serial.println("✓ BMP180 inicializado");
    bmp.resetToDefaults();
    bmp.setSamplingMode(BMP180MI::MODE_UHR);
  } else {
    Serial.println("✗ Error inicializando BMP180");
  }
  
  // Inicializar DHT22
  dht.begin();
  Serial.println("✓ DHT22 inicializado");
  
  // Inicializar BH1750
  if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    Serial.println("✓ BH1750 inicializado");
  } else {
    Serial.println("✗ Error inicializando BH1750");
  }
  
  // Configurar interrupciones para viento y lluvia
  pinMode(WIND_SPEED_PIN, INPUT_PULLUP);
  pinMode(RAIN_GAUGE_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(WIND_SPEED_PIN), windSpeedISR, FALLING);
  attachInterrupt(digitalPinToInterrupt(RAIN_GAUGE_PIN), rainGaugeISR, FALLING);
  
  Serial.println("\n✓ Sistema listo. Iniciando mediciones...\n");
  
  // Primera lectura
  readSensors();
  previousWeather = currentWeather;
}

// ===== LOOP PRINCIPAL =====
void loop() {
  static unsigned long lastReading = 0;
  static unsigned long lastDisplay = 0;
  
  unsigned long now = millis();
  
  // Leer sensores cada 30 segundos
  if (now - lastReading >= 30000) {
    readSensors();
    calculateDerivedData();
    lastReading = now;
  }
  
  // Mostrar datos cada 60 segundos
  if (now - lastDisplay >= 60000) {
    displayWeatherData();
    lastDisplay = now;
    
    // Guardar para calcular tendencias
    previousWeather = currentWeather;
  }
  
  delay(100);
}

// ===== INTERRUPCIONES =====
void ICACHE_RAM_ATTR windSpeedISR() {
  windPulseCount++;
}

void ICACHE_RAM_ATTR rainGaugeISR() {
  unsigned long now = millis();
  if (now - lastRainTime > 200) {  // Debounce 200ms
    rainTipCount++;
    lastRainTime = now;
  }
}

// ===== LECTURA DE SENSORES =====
void readSensors() {
  // Leer temperatura y humedad del DHT22
  currentWeather.temperature = dht.readTemperature();
  currentWeather.humidity = dht.readHumidity();
  
  if (isnan(currentWeather.temperature) || isnan(currentWeather.humidity)) {
    Serial.println("⚠ Error leyendo DHT22");
    currentWeather.temperature = 0;
    currentWeather.humidity = 0;
  }
  
  // Leer presión del BMP180
  if (bmp.measurePressure()) {
    do {
      delay(10);
    } while (!bmp.hasValue());
    
    currentWeather.pressure = bmp.getPressure() / 100.0;  // Convertir a hPa
  }
  
  // Leer luz ambiente
  currentWeather.lightIntensity = lightMeter.readLightLevel();
  
  // Calcular velocidad del viento (promedio de 10 segundos)
  currentWeather.windSpeed = calculateWindSpeed();
  
  // Calcular lluvia acumulada
  currentWeather.rainfall = rainTipCount * RAIN_MM_PER_TIP;
}

// ===== CÁLCULOS =====
float calculateWindSpeed() {
  // Asumir 1 pulso por revolución, 2.4 km/h por Hz
  // Ajustar según especificaciones de tu anemómetro
  unsigned long elapsed = millis() - lastWindTime;
  if (elapsed > 0) {
    float speed = (windPulseCount * 2.4 * 1000.0) / elapsed;
    windPulseCount = 0;
    lastWindTime = millis();
    return speed;
  }
  return 0;
}

void calculateDerivedData() {
  // Punto de rocío
  currentWeather.dewPoint = calculateDewPoint(
    currentWeather.temperature, 
    currentWeather.humidity
  );
  
  // Índice de calor
  currentWeather.heatIndex = calculateHeatIndex(
    currentWeather.temperature,
    currentWeather.humidity
  );
  
  // Sensación térmica
  currentWeather.windChill = calculateWindChill(
    currentWeather.temperature,
    currentWeather.windSpeed
  );
  
  // Presión a nivel del mar
  currentWeather.seaLevelPressure = calculateSeaLevelPressure(
    currentWeather.pressure,
    ALTITUDE
  );
  
  // Tendencia de presión
  currentWeather.pressureTrend = 
    currentWeather.pressure - previousWeather.pressure;
  
  // Predicción simple
  currentWeather.forecast = forecastFromPressure(
    currentWeather.seaLevelPressure,
    currentWeather.pressureTrend
  );
}

float calculateDewPoint(float temp, float humidity) {
  float a = 17.27;
  float b = 237.7;
  float alpha = ((a * temp) / (b + temp)) + log(humidity / 100.0);
  return (b * alpha) / (a - alpha);
}

float calculateHeatIndex(float temp, float humidity) {
  // Convertir a Fahrenheit
  float T = temp * 9.0 / 5.0 + 32.0;
  float RH = humidity;
  
  // Fórmula simplificada
  float HI = 0.5 * (T + 61.0 + ((T - 68.0) * 1.2) + (RH * 0.094));
  
  if (HI > 80) {
    // Fórmula completa de Rothfusz
    HI = -42.379 + 2.04901523 * T + 10.14333127 * RH
         - 0.22475541 * T * RH - 6.83783e-3 * T * T
         - 5.481717e-2 * RH * RH + 1.22874e-3 * T * T * RH
         + 8.5282e-4 * T * RH * RH - 1.99e-6 * T * T * RH * RH;
  }
  
  // Convertir de vuelta a Celsius
  return (HI - 32.0) * 5.0 / 9.0;
}

float calculateWindChill(float temp, float windSpeed) {
  if (temp <= 10 && windSpeed > 4.8) {
    return 13.12 + 0.6215 * temp - 11.37 * pow(windSpeed, 0.16)
           + 0.3965 * temp * pow(windSpeed, 0.16);
  }
  return temp;
}

float calculateSeaLevelPressure(float pressure, float altitude) {
  return pressure / pow(1.0 - (altitude / 44330.0), 5.255);
}

String forecastFromPressure(float pressure, float trend) {
  if (pressure > 1022) {
    if (trend > 0.5) return "Despejado, mejorando";
    else if (trend < -0.5) return "Despejado, empeorando";
    else return "Despejado, estable";
  } else if (pressure > 1009) {
    if (trend > 0.5) return "Parcial, mejorando";
    else if (trend < -0.5) return "Parcial, empeorando";
    else return "Parcial, estable";
  } else {
    if (trend > 0.5) return "Nublado, mejorando";
    else if (trend < -0.5) return "Lluvia probable";
    else return "Nublado, estable";
  }
}

// ===== MOSTRAR DATOS =====
void displayWeatherData() {
  Serial.println("\n╔════════════════════════════════════════╗");
  Serial.println("║     DATOS METEOROLÓGICOS ACTUALES     ║");
  Serial.println("╚════════════════════════════════════════╝");
  
  Serial.println("\n📊 MEDICIONES DIRECTAS");
  Serial.println("─────────────────────────────────────────");
  Serial.print("🌡️  Temperatura:     ");
  Serial.print(currentWeather.temperature, 1);
  Serial.println(" °C");
  
  Serial.print("💧 Humedad:          ");
  Serial.print(currentWeather.humidity, 1);
  Serial.println(" %");
  
  Serial.print("🏔️  Presión:         ");
  Serial.print(currentWeather.pressure, 1);
  Serial.println(" hPa");
  
  Serial.print("📊 Presión (nivel):  ");
  Serial.print(currentWeather.seaLevelPressure, 1);
  Serial.println(" hPa");
  
  Serial.print("☀️  Luz:             ");
  Serial.print(currentWeather.lightIntensity, 0);
  Serial.println(" lux");
  
  Serial.print("💨 Viento:           ");
  Serial.print(currentWeather.windSpeed, 1);
  Serial.println(" km/h");
  
  Serial.print("🌧️  Lluvia (acum.):  ");
  Serial.print(currentWeather.rainfall, 2);
  Serial.println(" mm");
  
  Serial.println("\n📈 DATOS CALCULADOS");
  Serial.println("─────────────────────────────────────────");
  Serial.print("💧 Punto de rocío:   ");
  Serial.print(currentWeather.dewPoint, 1);
  Serial.println(" °C");
  
  Serial.print("🔥 Índice de calor:  ");
  Serial.print(currentWeather.heatIndex, 1);
  Serial.println(" °C");
  
  Serial.print("❄️  Sens. térmica:   ");
  Serial.print(currentWeather.windChill, 1);
  Serial.println(" °C");
  
  Serial.print("📊 Tendencia (1h):   ");
  Serial.print(currentWeather.pressureTrend, 2);
  Serial.println(" hPa");
  
  Serial.println("\n🌤️  PREDICCIÓN");
  Serial.println("─────────────────────────────────────────");
  Serial.print("   ");
  Serial.println(currentWeather.forecast);
  
  Serial.println("═════════════════════════════════════════\n");
}

// ===== FUNCIONES DE ENVÍO (OPCIONAL) =====
/*
void sendToThingSpeak() {
  // Implementar envío a ThingSpeak
  ThingSpeak.setField(1, currentWeather.temperature);
  ThingSpeak.setField(2, currentWeather.humidity);
  ThingSpeak.setField(3, currentWeather.pressure);
  ThingSpeak.setField(4, currentWeather.lightIntensity);
  ThingSpeak.setField(5, currentWeather.windSpeed);
  ThingSpeak.setField(6, currentWeather.rainfall);
  ThingSpeak.writeFields(channelID, apiKey);
}

void sendToMQTT() {
  // Publicar datos via MQTT
  mqttClient.publish("weather/temperature", String(currentWeather.temperature));
  mqttClient.publish("weather/humidity", String(currentWeather.humidity));
  mqttClient.publish("weather/pressure", String(currentWeather.pressure));
  // ... etc
}
*/
