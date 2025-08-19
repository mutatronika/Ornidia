#include <ACS712.h>
#include <Wire.h>

// =================================================================================
//                           CONFIGURACIÓN DEL HARDWARE
// =================================================================================

// Pines del Multiplexor CD4052B
#define S0 D5              // Selector de canal 0 (GPIO14)
#define S1 D6              // Selector de canal 1 (GPIO12) 
#define PIN_ANALOG A0      // Entrada analógica única del ESP8266

// Configuración del sistema
#define NUM_MUESTRAS 50    // Número de muestras para promedio
#define DELAY_CANAL 5      // ms - Tiempo de estabilización del multiplexor
#define DELAY_MUESTRA 2    // ms - Tiempo entre muestras

// Factor de corrección para divisor de voltaje ACS712 según EzloPi (R1=1k, R2=2k)
#define FACTOR_DIVISOR_ACS 1.5  // Para divisor 1kΩ/2kΩ: Factor = (1k+2k)/2k = 1.5

// Objeto para el sensor ACS712 (referencia, no se usa directamente)
ACS712 sensorACS(0, 3.3, 1024, 185);  // Configurado para ACS712-05B (5A)

// =================================================================================
//                              FILTRO KALMAN
// =================================================================================

struct KalmanFilter {
    // Parámetros optimizados para sensores de corriente
    float Q = 0.01;      // Varianza del proceso (qué tan rápido cambia la señal real)
    float R = 0.1;       // Varianza de la medición (ruido del sensor)
    float P = 1.0;       // Error de estimación inicial
    float X = 0.0;       // Estado estimado inicial
    float K = 0.0;       // Ganancia de Kalman
    
    float filtrar(float medicion) {
        // Predicción
        P = P + Q;
        
        // Actualización
        K = P / (P + R);
        X = X + K * (medicion - X);
        P = (1 - K) * P;
        
        return X;
    }
    
    void reiniciar() {
        X = 0.0;
        P = 1.0;
    }
    
    void ajustarParametros(float q, float r) {
        Q = q;
        R = r;
    }
};

// =================================================================================
//                           VARIABLES GLOBALES
// =================================================================================

// Filtros Kalman independientes para cada sensor
KalmanFilter kalmanPanel, kalmanBateria, kalmanCarga, kalmanVoltaje;

// Offsets calibrados para cada sensor
float offsetPanel = 2.5, offsetBateria = 2.5, offsetCarga = 2.5;

// Variables para estadísticas y debug
unsigned long ultimaLectura = 0;
bool sistemaCalibrado = false;

// =================================================================================
//                           FUNCIONES PRINCIPALES
// =================================================================================

void seleccionarCanal(int canal) {
    if (canal < 0 || canal > 3) {
        Serial.println("⚠️ Error: Canal fuera de rango (0-3)");
        return;
    }
    
    digitalWrite(S0, canal & 0x01);        // Bit 0
    digitalWrite(S1, (canal >> 1) & 0x01); // Bit 1
    
    delayMicroseconds(100); // Estabilización rápida del multiplexor
}

float leerVoltajeRaw(int canal, int muestras = NUM_MUESTRAS) {
    seleccionarCanal(canal);
    delay(DELAY_CANAL);
    
    float suma = 0;
    int lecturasValidas = 0;
    
    for (int i = 0; i < muestras; i++) {
        int raw = analogRead(PIN_ANALOG);
        if (raw >= 0 && raw <= 1023) { // Validar lectura
            suma += raw * (3.3 / 1024.0);
            lecturasValidas++;
        }
        delay(DELAY_MUESTRA);
    }
    
    if (lecturasValidas == 0) {
        Serial.println("⚠️ Error: No se pudieron obtener lecturas válidas");
        return 0.0;
    }
    
    return suma / lecturasValidas;
}

float calcularOffset(int canal) {
    Serial.print("📋 Calibrando sensor canal ");
    Serial.print(canal);
    Serial.print("...");
    
    // Tomar más muestras para calibración más precisa
    float voltajeRaw = leerVoltajeRaw(canal, 100);
    
    // Compensar divisor de voltaje si existe
    float voltajeReal = voltajeRaw * FACTOR_DIVISOR_ACS;
    
    Serial.print(" Offset: ");
    Serial.print(voltajeReal, 4);
    Serial.println("V");
    
    return voltajeReal;
}

void calibrarSistema() {
    Serial.println("🔧 INICIANDO CALIBRACIÓN DEL SISTEMA");
    Serial.println("⚠️  IMPORTANTE: Desconecte todas las cargas durante la calibración");
    Serial.println("⏳ Calibrando en 3 segundos...");
    
    for (int i = 3; i > 0; i--) {
        Serial.print(i);
        Serial.print("... ");
        delay(1000);
    }
    Serial.println("¡INICIANDO!");
    
    // Calibrar cada sensor
    offsetPanel = calcularOffset(0);
    offsetBateria = calcularOffset(1);
    offsetCarga = calcularOffset(2);
    
    // Reiniciar filtros Kalman
    kalmanPanel.reiniciar();
    kalmanBateria.reiniciar();
    kalmanCarga.reiniciar();
    kalmanVoltaje.reiniciar();
    
    sistemaCalibrado = true;
    
    Serial.println("✅ CALIBRACIÓN COMPLETADA");
    Serial.println("═══════════════════════════════════");
}

float leerCorrienteACS(int canal, float offset, KalmanFilter &kalman) {
    // Leer voltaje raw del sensor
    float voltajeRaw = leerVoltajeRaw(canal, NUM_MUESTRAS);
    
    // Compensar divisor de voltaje
    float voltajeReal = voltajeRaw * FACTOR_DIVISOR_ACS;
    
    // Convertir a corriente usando el offset calibrado
    // ACS712-05B: 185mV/A, punto cero en offset personalizado
    float corrienteBruta = (voltajeReal - offset) / 0.185;
    
    // Aplicar filtro Kalman
    float corrienteFiltrada = kalman.filtrar(corrienteBruta);
    
    return corrienteFiltrada;
}

float leerVoltajePanel() {
    seleccionarCanal(3);
    delay(DELAY_CANAL);
    
    // Leer voltaje del divisor de tensión del panel
    float voltajeRaw = leerVoltajeRaw(3, 20); // Menos muestras para voltaje (más estable)
    
    // Aplicar fórmula del divisor R1=100kΩ, R2=22kΩ
    float voltajePanel = voltajeRaw * ((100.0 + 22.0) / 22.0);
    
    // Aplicar filtro Kalman para suavizar
    return kalmanVoltaje.filtrar(voltajePanel);
}

void mostrarEstadisticas() {
    float corrientePanel = leerCorrienteACS(0, offsetPanel, kalmanPanel);
    float corrienteBateria = leerCorrienteACS(1, offsetBateria, kalmanBateria);
    float corrienteCarga = leerCorrienteACS(2, offsetCarga, kalmanCarga);
    float voltajePanel = leerVoltajePanel();
    
    Serial.println("═══════════════════════════════════");
    Serial.println("       📊 ORNIDIA - LECTURAS ACTUALES");
    Serial.println("═══════════════════════════════════");
    
    // Mostrar lecturas principales
    Serial.print("☀️  Panel Solar:    ");
    Serial.print(corrientePanel, 3);
    Serial.print(" A  (");
    Serial.print(corrientePanel * 1000, 0);
    Serial.print(" mA) @ ");
    Serial.print(voltajePanel, 2);
    Serial.println(" V");
    
    Serial.print("🔋 Batería:        ");
    Serial.print(corrienteBateria, 3);
    Serial.print(" A  (");
    Serial.print(corrienteBateria * 1000, 0);
    Serial.println(" mA)");
    
    Serial.print("📦 Carga:          ");
    Serial.print(corrienteCarga, 3);
    Serial.print(" A  (");
    Serial.print(corrienteCarga * 1000, 0);
    Serial.println(" mA)");
    
    // Calcular potencia
    float potenciaPanel = corrientePanel * voltajePanel;
    Serial.print("⚡ Potencia Panel:  ");
    Serial.print(potenciaPanel, 2);
    Serial.println(" W");
    
    // Estado del sistema
    Serial.println("───────────────────────────────────");
    Serial.print("📈 Estado: ");
    if (corrientePanel > 0.05) {
        Serial.println("Panel generando energía ☀️");
    } else {
        Serial.println("Sin generación solar 🌙");
    }
    
    Serial.print("🔄 Batería: ");
    if (corrienteBateria < -0.05) {
        Serial.println("Cargándose ⬆️");
    } else if (corrienteBateria > 0.05) {
        Serial.println("Descargándose ⬇️");
    } else {
        Serial.println("En equilibrio ⚖️");
    }
    
    // Advertencias
    if (abs(corrientePanel) > 0.4 || abs(corrienteBateria) > 3.0 || abs(corrienteCarga) > 2.0) {
        Serial.println("⚠️  ADVERTENCIA: Corriente fuera de rango esperado");
    }
    
    if (voltajePanel > 16.0 || voltajePanel < 8.0) {
        Serial.println("⚠️  ADVERTENCIA: Voltaje de panel anómalo");
    }
    
    Serial.println("═══════════════════════════════════");
}

void mostrarDebug() {
    Serial.println("🔧 INFORMACIÓN DE DEBUG:");
    Serial.print("Offset Panel: "); Serial.print(offsetPanel, 4); Serial.println(" V");
    Serial.print("Offset Batería: "); Serial.print(offsetBateria, 4); Serial.println(" V");
    Serial.print("Offset Carga: "); Serial.print(offsetCarga, 4); Serial.println(" V");
    Serial.print("Factor Divisor ACS: "); Serial.println(FACTOR_DIVISOR_ACS);
    Serial.print("Tiempo funcionando: "); Serial.print(millis() / 1000); Serial.println(" s");
}

// =================================================================================
//                              SETUP Y LOOP
// =================================================================================

void setup() {
    Serial.begin(115200);
    Serial.println();
    Serial.println("🚀 SISTEMA ORNIDIA INICIANDO...");
    Serial.println("   Monitoreo de Panel Solar v2.0");
    Serial.println("   Con Filtro Kalman Optimizado");
    Serial.println();
    
    // Configurar pines
    pinMode(S0, OUTPUT);
    pinMode(S1, OUTPUT);
    
    // Configurar filtros Kalman con parámetros optimizados
    kalmanPanel.ajustarParametros(0.01, 0.1);    // Panel: cambios moderados
    kalmanBateria.ajustarParametros(0.02, 0.15); // Batería: más dinámica
    kalmanCarga.ajustarParametros(0.01, 0.1);    // Carga: estable
    kalmanVoltaje.ajustarParametros(0.001, 0.05); // Voltaje: muy estable
    
    // Calibración automática
    calibrarSistema();
    
    Serial.println("🎯 Sistema listo para monitoreo");
    Serial.println("📊 Enviando 'D' para debug avanzado");
    Serial.println();
    
    ultimaLectura = millis();
}

void loop() {
    // Mostrar lecturas cada 2 segundos
    if (millis() - ultimaLectura >= 2000) {
        mostrarEstadisticas();
        ultimaLectura = millis();
    }
    
    // Comando de debug por serial
    if (Serial.available()) {
        char comando = Serial.read();
        if (comando == 'D' || comando == 'd') {
            mostrarDebug();
        } else if (comando == 'C' || comando == 'c') {
            calibrarSistema();
        }
    }
    
    delay(100); // Pequeña pausa para estabilidad
}
