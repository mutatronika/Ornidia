#include <ACS712.h>
#include <Wire.h>

// Pines del Multiplexor CD4052B
#define S0 D5  // Selector de canal 0
#define S1 D6  // Selector de canal 1
#define PIN_ANALOG A0  // Entrada analógica única del ESP8266

// Objeto para el sensor ACS712
ACS712 sensorACS(0, 3.3, 1024, 185);  // Configurado para 5A

// Variables globales para offsets
float panelOffset = 0, bateriaOffset = 0, cargaOffset = 0;

//  Filtro Kalman para suavizar lecturas
struct Kalman {
    float q = 0.125;  // Variación estimada del proceso
    float r = 4;      // Varianza del ruido de medición
    float p = 1;      // Estimación inicial del error
    float x = 0;      // Estimación inicial del estado

    float filtrar(float medida) {
        p = p + q;
        float k = p / (p + r);
        x = x + k * (medida - x);
        p = (1 - k) * p;
        return x;
    }
};

// Creamos un filtro Kalman para cada sensor
Kalman kalmanPanel, kalmanBateria, kalmanCarga;

//  Función para seleccionar canal en el CD4052B
void seleccionarCanal(int canal) {
    digitalWrite(S0, canal & 0x01);
    digitalWrite(S1, (canal >> 1) & 0x01);
}

// Calcular el offset del ACS712 al inicio
float calcularOffset(int canal) {
    seleccionarCanal(canal);
    delay(5);
    float suma = 0;
    int numMuestras = 100;

    for (int i = 0; i < numMuestras; i++) {
        suma += analogRead(PIN_ANALOG);
        delay(2);
    }
    return (suma / numMuestras) * (3.3 / 1024.0);
}

// Leer corriente con filtro de promedio y Kalman
#define NUM_MUESTRAS 50

float leerCorrienteACS(int canal, float offset, Kalman &kalman) {
    seleccionarCanal(canal);
    delay(5);  // Pequeña pausa
    float suma = 0;

    for (int i = 0; i < NUM_MUESTRAS; i++) {
        suma += analogRead(PIN_ANALOG) * (3.3 / 1024.0);
        delay(2);
    }

    float corrienteProm = (suma / NUM_MUESTRAS - offset) / 0.185;  // Conversión a Amperios
    return kalman.filtrar(corrienteProm);  // Aplicar filtro Kalman
}

//  Función para leer el voltaje del panel
float leerVoltajePanel() {
    seleccionarCanal(3);
    delay(5);
    float vMedido = analogRead(PIN_ANALOG) * (3.3 / 1024.0);  // Convertir ADC a voltios
    return vMedido * ((100.0 + 22.0) / 22.0);  // Aplicar la fórmula del divisor de tensión
}

void setup() {
    Serial.begin(115200);
    
    pinMode(S0, OUTPUT);
    pinMode(S1, OUTPUT);

    Serial.println("Calculando offset del ACS712...");
    panelOffset = calcularOffset(0);
    bateriaOffset = calcularOffset(1);
    cargaOffset = calcularOffset(2);
    Serial.println("Offset calculado.");
}

void loop() {
    float corrientePanel = leerCorrienteACS(0, panelOffset, kalmanPanel);
    float corrienteBateria = leerCorrienteACS(1, bateriaOffset, kalmanBateria);
    float corrienteCarga = leerCorrienteACS(2, cargaOffset, kalmanCarga);
    float voltajePanel = leerVoltajePanel();

    Serial.print("Corriente Panel: "); Serial.print(corrientePanel); Serial.println(" A");
    Serial.print("Corriente Batería: "); Serial.print(corrienteBateria); Serial.println(" A");
    Serial.print("Corriente Carga: "); Serial.print(corrienteCarga); Serial.println(" A");
    Serial.print("Voltaje del Panel: "); Serial.print(voltajePanel); Serial.println(" V");

    Serial.println("----------------------");
    delay(2000);
}
