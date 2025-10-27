#include <ACS712.h>

// Definición de pines
#define PIN_ANALOG A0    // Entrada analógica de ESP8266
#define S0 D5           // Pin selector S0 del CD4052B
#define S1 D6           // Pin selector S1 del CD4052B

// Configuración del sensor ACS712
// Parámetros: pin analógico, voltaje de referencia, resolución ADC, sensibilidad
ACS712 sensorACS(A0, 3.3, 1024, 185); // 185 mV/A para ACS712-05B (5A)

// Variables para filtrado y estabilización
const int NUM_MUESTRAS = 10;
const int DELAY_ESTABILIZACION = 10; // ms

void setup() {
    Serial.begin(115200);
    
    // Configurar pines del multiplexor como salidas
    pinMode(S0, OUTPUT);
    pinMode(S1, OUTPUT);
    
    // Inicializar en canal 0
    seleccionarCanal(0);
    
    Serial.println("Sistema iniciado - Monitor de corriente con ACS712");
    Serial.println("Canal 0: Panel Solar");
    Serial.println("Canal 1: Batería");
    Serial.println("Canal 2: Carga");
    Serial.println("Canal 3: Divisor de voltaje (si está conectado)");
    Serial.println("----------------------------------------");
}

void seleccionarCanal(int canal) {
    // Validar que el canal esté en rango (0-3 para CD4052B)
    if (canal < 0 || canal > 3) {
        Serial.println("Error: Canal fuera de rango (0-3)");
        return;
    }
    
    // Configurar los pines de selección
    digitalWrite(S0, canal & 0x01);        // Bit menos significativo
    digitalWrite(S1, (canal >> 1) & 0x01); // Bit más significativo
}

float leerCorrienteACS(int canal) {
    seleccionarCanal(canal);
    delay(DELAY_ESTABILIZACION); // Pausa para estabilizar el multiplexor
    
    // Tomar múltiples muestras para mayor precisión
    float sumaLecturas = 0;
    for (int i = 0; i < NUM_MUESTRAS; i++) {
        int lecturaRaw = analogRead(PIN_ANALOG);
        float voltaje = lecturaRaw * (3.3 / 1024.0); // Convertir a voltios
        sumaLecturas += voltaje;
        delay(2); // Pequeña pausa entre lecturas
    }
    
    float voltajePromedio = sumaLecturas / NUM_MUESTRAS;
    
    // Convertir voltaje a corriente
    // ACS712-05B: Sensibilidad = 185 mV/A, Vcc/2 = 2.5V (punto cero)
    float corriente = (voltajePromedio - 2.5) / 0.185;
    
    return corriente;
}

float leerVoltaje(int canal) {
    // Función para leer voltaje del divisor de voltaje en el canal especificado
    seleccionarCanal(canal);
    delay(DELAY_ESTABILIZACION);
    
    float sumaLecturas = 0;
    for (int i = 0; i < NUM_MUESTRAS; i++) {
        int lecturaRaw = analogRead(PIN_ANALOG);
        float voltaje = lecturaRaw * (3.3 / 1024.0);
        sumaLecturas += voltaje;
        delay(2);
    }
    
    float voltajePromedio = sumaLecturas / NUM_MUESTRAS;
    
    // Ajustar según tu divisor de voltaje
    // Por ejemplo, si usas un divisor 10:1, multiplica por 10
    // float voltajeReal = voltajePromedio * 10.0; // Ajustar según tu divisor
    
    return voltajePromedio; // Retorna voltaje sin ajustar, modifica según necesites
}

void loop() {
    // Leer corrientes de los tres sensores ACS712
    float corrientePanel = leerCorrienteACS(0);
    float corrienteBateria = leerCorrienteACS(1);
    float corrienteCarga = leerCorrienteACS(2);
    
    // Si tienes un divisor de voltaje en el canal 3
    // float voltajeSistema = leerVoltaje(3);
    
    // Mostrar resultados
    Serial.println("=== LECTURAS ACTUALES ===");
    Serial.print("Corriente Panel Solar: ");
    Serial.print(corrientePanel, 3);
    Serial.println(" A");
    
    Serial.print("Corriente Batería: ");
    Serial.print(corrienteBateria, 3);
    Serial.println(" A");
    
    Serial.print("Corriente Carga: ");
    Serial.print(corrienteCarga, 3);
    Serial.println(" A");
    
    // Mostrar potencia estimada (necesitarías voltaje para cálculo preciso)
    // Serial.print("Potencia Panel: ");
    // Serial.print(corrientePanel * voltajeSistema, 2);
    // Serial.println(" W");
    
    Serial.println("---------------------------");
    
    // Verificar condiciones anómalas
    if (abs(corrientePanel) > 6.0 || abs(corrienteBateria) > 6.0 || abs(corrienteCarga) > 6.0) {
        Serial.println("¡ADVERTENCIA! Corriente fuera de rango normal");
    }
    
    delay(2000);
}
