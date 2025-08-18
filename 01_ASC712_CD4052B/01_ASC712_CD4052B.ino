
#include <ACS712.h>

#define PIN_ANALOG A0    // Entrada analógica de ESP8266
#define S0 D5           // Nuevo pin para selector S0
#define S1 D6           // Nuevo pin para selector S1

ACS712 sensorACS(0, 3.3, 1024, 185); // Objeto para todos los sensores

void setup() {
    Serial.begin(115200);
    
    pinMode(S0, OUTPUT);
    pinMode(S1, OUTPUT);
}

void seleccionarCanal(int canal) {
    digitalWrite(S0, canal & 0x01);
    digitalWrite(S1, (canal >> 1) & 0x01);
}

float leerCorrienteACS(int canal) {
    seleccionarCanal(canal);
    delay(5);  // Pequeña pausa para estabilizar
    float lectura = analogRead(PIN_ANALOG) * (3.3 / 1024.0); // Convertir a voltios
    return (lectura - 2.5) / 0.185;  // Convertir a amperios (5A versión)
}

void loop() {
    float corrientePanel = leerCorrienteACS(0);
    float corrienteBateria = leerCorrienteACS(1);
    float corrienteCarga = leerCorrienteACS(2);

    Serial.print("Corriente Panel: "); Serial.print(corrientePanel); Serial.println(" A");
    Serial.print("Corriente Batería: "); Serial.print(corrienteBateria); Serial.println(" A");
    Serial.print("Corriente Carga: "); Serial.print(corrienteCarga); Serial.println(" A");

    Serial.println("----------------------");
    delay(2000);
}
