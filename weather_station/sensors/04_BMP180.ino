#include <Wire.h>            // Librería para I2C
#include <BMP180I2C.h>       // Librería corregida para BMP180
#include <BH1750.h>          // Sensor de luz
#include <DHT.h>             // Sensor DHT11

// 🔹 Pines del multiplexor CD4052B
#define S0 D5
#define S1 D6
#define SENSOR_ANALOGICO A0  // Única entrada analógica del ESP8266

// 🔹 Pines del DHT11
#define DHTPIN D7
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// 🔹 Sensores I2C
BMP180I2C bmp(0x77);    // Dirección I2C del BMP180
BH1750 lightMeter;    // Objeto para BH1750

// 🔹 Factor de calibración del ACS712
#define SENSITIVITY 0.185  // Para la versión de 5A

// 🔹 Configuración del multiplexor
void seleccionarCanal(int canal) {
    digitalWrite(S0, canal & 0x01);
    digitalWrite(S1, (canal >> 1) & 0x01);
}

// 🔹 Leer corriente con ACS712
float leerCorriente(int canal) {
    seleccionarCanal(canal);
    delay(5);
    float suma = 0;
    int muestras = 50;
    for (int i = 0; i < muestras; i++) {
        suma += analogRead(SENSOR_ANALOGICO);
    }
    float promedio = suma / muestras;
    float voltaje = promedio * (3.3 / 1024.0);
    float corriente = (voltaje - 2.5) / SENSITIVITY;
    return corriente;
}

// 🔹 Leer voltaje del panel solar
float leerVoltaje() {
    seleccionarCanal(3);
    delay(5);
    float suma = 0;
    int muestras = 50;
    for (int i = 0; i < muestras; i++) {
        suma += analogRead(SENSOR_ANALOGICO);
    }
    float promedio = suma / muestras;
    float voltaje = (promedio * 3.3 / 1024.0) * 5.7; // Factor de divisor resistivo
    return voltaje;
}

void setup() {
    Serial.begin(115200);
    Serial.println("Iniciando sensores...");

    // 🔹 Configurar pines del multiplexor
    pinMode(S0, OUTPUT);
    pinMode(S1, OUTPUT);

    // 🔹 Iniciar DHT11
    dht.begin();

    // 🔹 Iniciar I2C y sensores
    Wire.begin(D2, D1);

    // 🔹 Iniciar BMP180
    if (!bmp.begin()) {
        Serial.println("Error: No se detectó el BMP180");
        while (1);
    }

    // 🔹 Iniciar BH1750
    lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE);
}

void loop() {
    // 🔹 Leer sensores de corriente
    float corrientePanel = leerCorriente(0);
    float corrienteBateria = leerCorriente(1);
    float corrienteCarga = leerCorriente(2);

    // 🔹 Leer voltaje del panel solar
    float voltajePanel = leerVoltaje();
    
    // 🔹 Calcular potencia del panel
    float potenciaPanel = corrientePanel * voltajePanel;

    // 🔹 Leer temperatura y humedad del DHT11
    float temperaturaDHT = dht.readTemperature();
    float humedad = dht.readHumidity();

    // 🔹 Leer presión y temperatura del BMP180
    float temperaturaBMP = 0;
    float presion = 0;

    if (bmp.measureTemperature()) {  
        temperaturaBMP = bmp.getTemperature();
    } else {
        Serial.println("Error al leer temperatura BMP180");
    }

    if (bmp.measurePressure()) {  
        presion = bmp.getPressure();
    } else {
        Serial.println("Error al leer presión BMP180");
    }

    // 🔹 Leer luz en lux del BH1750
    float lux = lightMeter.readLightLevel();

    // 🔹 Mostrar datos en Serial Monitor
    Serial.println("===== MONITOREO SOLAR =====");
    Serial.print("Corriente Panel: "); Serial.print(corrientePanel); Serial.println(" A");
    Serial.print("Corriente Batería: "); Serial.print(corrienteBateria); Serial.println(" A");
    Serial.print("Corriente Carga: "); Serial.print(corrienteCarga); Serial.println(" A");
    
    Serial.print("Voltaje Panel: "); Serial.print(voltajePanel); Serial.println(" V");
    Serial.print("Potencia Panel: "); Serial.print(potenciaPanel); Serial.println(" W");
    
    Serial.print("Temperatura (DHT11): "); Serial.print(temperaturaDHT); Serial.println(" °C");
    Serial.print("Humedad (DHT11): "); Serial.print(humedad); Serial.println(" %");
    
    Serial.print("Temperatura (BMP180): "); Serial.print(temperaturaBMP); Serial.println(" °C");
    Serial.print("Presión: "); Serial.print(presion / 100.0); Serial.println(" hPa");
    
    Serial.print("Luz: "); Serial.print(lux); Serial.println(" lx");

    Serial.println("===========================");
    
    delay(2000);  // Esperar 2 segundos antes de la siguiente lectura
}
