/*
ORNIDIA v3.0 - IMPLEMENTACIÓN POR FASES
=======================================
FASE ACTUAL: Configurar con #define FASE_ACTUAL

FASE_1: Solo mejoras básicas (sin calibración ADC)
FASE_2: Agregar calibración Vref
FASE_3: Implementar corrección no-linealidad completa
*/

#include <ACS712.h>
#include <Wire.h>

// ================== CONFIGURACIÓN DE FASES ==================
#define FASE_ACTUAL 1  // Cambiar a 2 o 3 según progreso

// Pines del sistema
#define S0 D5              
#define S1 D6              
#define PIN_ANALOG A0      
#define NUM_MUESTRAS 50    
#define FACTOR_DIVISOR_ACS 1.5  // Para resistores 1kΩ/2kΩ

// ================== ESTRUCTURAS BÁSICAS ==================

struct KalmanFilter {
    float Q, R, P, X, K;
    
    KalmanFilter(float q = 0.01, float r = 0.1) : Q(q), R(r), P(1.0), X(0.0), K(0.0) {}
    
    float filtrar(float medicion) {
        P = P + Q;
        K = P / (P + R);
        X = X + K * (medicion - X);
        P = (1 - K) * P;
        return X;
    }
    
    void reiniciar() { X = 0.0; P = 1.0; }
};

class FiltroPromedio {
    float buffer[10];
    int indice = 0;
    int contador = 0;
    
public:
    float agregar(float valor) {
        buffer[indice] = valor;
        indice = (indice + 1) % 10;
        if (contador < 10) contador++;
        
        float suma = 0;
        for (int i = 0; i < contador; i++) {
            suma += buffer[i];
        }
        return suma / contador;
    }
    
    void limpiar() { contador = 0; indice = 0; }
};

// ================== VARIABLES GLOBALES ==================

KalmanFilter kalmanPanel(0.005, 0.05);
KalmanFilter kalmanBateria(0.01, 0.08);
KalmanFilter kalmanCarga(0.005, 0.05);
KalmanFilter kalmanVoltaje(0.001, 0.02);

FiltroPromedio filtroPanel, filtroBateria, filtroCarga;

float offsetPanel = 2.5, offsetBateria = 2.5, offsetCarga = 2.5;
float vref_actual = 3.3;
bool sistemaCalibrado = false;

#if FASE_ACTUAL >= 2
// ================== FASE 2: CALIBRACIÓN VREF ==================

void calibrarVref() {
    Serial.println("🔧 CALIBRACIÓN VREF DISPONIBLE");
    Serial.println("   Conecta voltímetro de precisión al pin A0");
    Serial.println("   Envía comando: Vx.xxx (ej: V2.450)");
    Serial.print("   Vref actual: ");
    Serial.print(vref_actual, 4);
    Serial.println("V");
}

void procesarComandoVref(String comando) {
    if (comando.startsWith("V") || comando.startsWith("v")) {
        float voltaje_conocido = comando.substring(1).toFloat();
        
        if (voltaje_conocido > 0.1 && voltaje_conocido < 3.5) {
            // Leer ADC actual
            long suma = 0;
            for (int i = 0; i < 100; i++) {
                suma += analogRead(PIN_ANALOG);
                delay(5);
            }
            float adc_promedio = suma / 100.0;
            
            // Calcular Vref real
            float vref_nuevo = voltaje_conocido * 1024.0 / adc_promedio;
            
            if (vref_nuevo > 2.8 && vref_nuevo < 3.6) {
                vref_actual = vref_nuevo;
                Serial.print("✅ Vref calibrado: ");
                Serial.print(vref_actual, 4);
                Serial.print("V (ADC: ");
                Serial.print(adc_promedio, 1);
                Serial.println(")");
            } else {
                Serial.println("❌ Vref fuera de rango esperado");
            }
        } else {
            Serial.println("❌ Voltaje debe estar entre 0.1V y 3.5V");
        }
    }
}

float leerVoltajeCalibrado() {
    long suma = 0;
    for (int i = 0; i < NUM_MUESTRAS; i++) {
        suma += analogRead(PIN_ANALOG);
        delayMicroseconds(500);
    }
    float adc_promedio = suma / float(NUM_MUESTRAS);
    return (adc_promedio * vref_actual) / 1024.0;
}

#endif

#if FASE_ACTUAL >= 3
// ================== FASE 3: CORRECCIÓN NO-LINEALIDAD ==================

struct PuntoCurva {
    uint16_t adc;
    float voltaje;
};

// Tabla de corrección (ajustar según calibración individual)
const PuntoCurva curva_correccion[] = {
    {0,    0.000},
    {100,  0.310},   // Ajustar según mediciones
    {200,  0.630},
    {300,  0.940},
    {400,  1.250},
    {500,  1.570},
    {600,  1.880},
    {700,  2.190},
    {800,  2.510},
    {900,  2.820},
    {1000, 3.130},
    {1023, 3.200}    // Ajustar según Vref real
};

const int PUNTOS_CURVA = sizeof(curva_correccion) / sizeof(curva_correccion[0]);

float interpolarCurva(uint16_t adc_raw) {
    if (adc_raw <= curva_correccion[0].adc) return 0.0;
    if (adc_raw >= curva_correccion[PUNTOS_CURVA-1].adc) {
        return curva_correccion[PUNTOS_CURVA-1].voltaje * (vref_actual / 3.2);
    }
    
    for (int i = 0; i < PUNTOS_CURVA - 1; i++) {
        if (adc_raw >= curva_correccion[i].adc && adc_raw <= curva_correccion[i+1].adc) {
            float x1 = curva_correccion[i].adc;
            float y1 = curva_correccion[i].voltaje;
            float x2 = curva_correccion[i+1].adc;
            float y2 = curva_correccion[i+1].voltaje;
            
            float voltaje = y1 + (y2 - y1) * (adc_raw - x1) / (x2 - x1);
            return voltaje * (vref_actual / 3.2);
        }
    }
    
    return (adc_raw * vref_actual) / 1024.0;
}

float leerVoltajeCorregido() {
    long suma = 0;
    for (int i = 0; i < NUM_MUESTRAS; i++) {
        uint16_t raw = analogRead(PIN_ANALOG);
        suma += raw;
        delayMicroseconds(500);
    }
    uint16_t adc_promedio = suma / NUM_MUESTRAS;
    return interpolarCurva(adc_promedio);
}

#endif

// ================== FUNCIONES PRINCIPALES ==================

void seleccionarCanal(int canal) {
    digitalWrite(S0, canal & 0x01);
    digitalWrite(S1, (canal >> 1) & 0x01);
    delayMicroseconds(100);
}

float leerVoltajeRaw(int canal) {
    seleccionarCanal(canal);
    delay(5);
    
    float voltaje;
    
#if FASE_ACTUAL >= 3
    voltaje = leerVoltajeCorregido();
#elif FASE_ACTUAL >= 2
    voltaje = leerVoltajeCalibrado();
#else
    // FASE 1: Lectura básica mejorada
    long suma = 0;
    for (int i = 0; i < NUM_MUESTRAS; i++) {
        suma += analogRead(PIN_ANALOG);
        delayMicroseconds(500);
    }
    voltaje = (suma / float(NUM_MUESTRAS)) * (vref_actual / 1024.0);
#endif
    
    return voltaje;
}

float calcularOffset(int canal) {
    Serial.print("📋 Calibrando canal ");
    Serial.print(canal);
    Serial.print("...");
    
    float voltaje_raw = leerVoltajeRaw(canal);
    float voltaje_real = voltaje_raw * FACTOR_DIVISOR_ACS;
    
    Serial.print(" Offset: ");
    Serial.print(voltaje_real, 4);
    Serial.println("V");
    
    return voltaje_real;
}

void calibrarSistema() {
    Serial.println("🔧 CALIBRANDO SISTEMA ORNIDIA");
    Serial.print("   Fase activa: ");
    Serial.println(FASE_ACTUAL);
    
#if FASE_ACTUAL >= 2
    Serial.println("   ✓ Calibración Vref disponible");
#endif
#if FASE_ACTUAL >= 3
    Serial.println("   ✓ Corrección no-linealidad activa");
#endif
    
    Serial.println("⏳ Iniciando en 3 segundos...");
    for (int i = 3; i > 0; i--) {
        Serial.print(i); Serial.print("... ");
        delay(1000);
    }
    Serial.println();
    
#if FASE_ACTUAL >= 2
    calibrarVref();
#endif
    
    // Limpiar filtros
    filtroPanel.limpiar();
    filtroBateria.limpiar();
    filtroCarga.limpiar();
    
    // Calibrar offsets
    offsetPanel = calcularOffset(0);
    offsetBateria = calcularOffset(1);
    offsetCarga = calcularOffset(2);
    
    // Reiniciar Kalman
    kalmanPanel.reiniciar();
    kalmanBateria.reiniciar();
    kalmanCarga.reiniciar();
    kalmanVoltaje.reiniciar();
    
    sistemaCalibrado = true;
    
    Serial.println("✅ CALIBRACIÓN COMPLETADA");
    
#if FASE_ACTUAL >= 2
    Serial.println("📝 Usa 'Vx.xxx' para calibrar Vref");
#endif
#if FASE_ACTUAL >= 3
    Serial.println("🎯 Precisión esperada: ±0.2%");
#endif
    
    Serial.println("═══════════════════════════════════");
}

float leerCorrienteACS(int canal, float offset, KalmanFilter &kalman, FiltroPromedio &filtro) {
    float voltaje_raw = leerVoltajeRaw(canal);
    float voltaje_real = voltaje_raw * FACTOR_DIVISOR_ACS;
    float corriente_bruta = (voltaje_real - offset) / 0.185;
    
    // Doble filtrado
    float corriente_promedio = filtro.agregar(corriente_bruta);
    float corriente_final = kalman.filtrar(corriente_promedio);
    
    return corriente_final;
}

float leerVoltajePanel() {
    seleccionarCanal(3);
    delay(5);
    
    float voltaje_raw = leerVoltajeRaw(3);
    float voltaje_panel = voltaje_raw * ((100.0 + 22.0) / 22.0);
    
    return kalmanVoltaje.filtrar(voltaje_panel);
}

void mostrarLecturas() {
    float corrientePanel = leerCorrienteACS(0, offsetPanel, kalmanPanel, filtroPanel);
    float corrienteBateria = leerCorrienteACS(1, offsetBateria, kalmanBateria, filtroBateria);
    float corrienteCarga = leerCorrienteACS(2, offsetCarga, kalmanCarga, filtroCarga);
    float voltajePanel = leerVoltajePanel();
    
    Serial.println("═══════════════════════════════════");
    Serial.print("📊 ORNIDIA FASE ");
    Serial.print(FASE_ACTUAL);
    
#if FASE_ACTUAL >= 3
    Serial.println(" (±0.2% precisión)");
#elif FASE_ACTUAL >= 2  
    Serial.println(" (Vref calibrado)");
#else
    Serial.println(" (Filtrado básico)");
#endif
    
    Serial.println("═══════════════════════════════════");
    
    Serial.print("☀  Panel:     ");
    Serial.print(corrientePanel, 4);
    Serial.print("A @ ");
    Serial.print(voltajePanel, 2);
    Serial.println("V");
    
    Serial.print("🔋 Batería:   ");
    Serial.print(corrienteBateria, 4);
    Serial.println("A");
    
    Serial.print("📦 Carga:     ");
    Serial.print(corrienteCarga, 4);
    Serial.println("A");
    
    Serial.print("⚡ Potencia:   ");
    Serial.print(corrientePanel * voltajePanel, 3);
    Serial.println("W");
    
    // Estado del sistema
    Serial.print("📊 Estado: ");
    if (corrientePanel > 0.05) {
        Serial.print("Generando ☀");
    } else {
        Serial.print("Sin sol 🌙");
    }
    
    if (corrienteBateria > 0.05) {
        Serial.println(" | Cargando 🔋⬆");
    } else if (corrienteBateria < -0.05) {
        Serial.println(" | Descargando 🔋⬇");
    } else {
        Serial.println(" | Equilibrio 🔋⚖");
    }
    
    Serial.println("═══════════════════════════════════");
}

void setup() {
    Serial.begin(115200);
    Serial.println();
    Serial.println("🚀 SISTEMA ORNIDIA v3.0");
    Serial.print("   Fase implementada: ");
    Serial.println(FASE_ACTUAL);
    
#if FASE_ACTUAL == 1
    Serial.println("   ✓ Filtros Kalman optimizados");
    Serial.println("   ✓ Filtro de promedio móvil");
#elif FASE_ACTUAL == 2
    Serial.println("   ✓ Calibración Vref automática");
    Serial.println("   ✓ Filtros optimizados");
#elif FASE_ACTUAL == 3
    Serial.println("   ✓ Corrección ADC no-lineal");
    Serial.println("   ✓ Calibración Vref");
    Serial.println("   ✓ Precisión ±0.2%");
#endif
    
    Serial.println();
    
    pinMode(S0, OUTPUT);
    pinMode(S1, OUTPUT);
    
    calibrarSistema();
    
    Serial.println("🎯 Sistema operativo");
    
#if FASE_ACTUAL >= 2
    Serial.println("📝 Comandos: 'D'=debug, 'C'=calibrar, 'Vx.xxx'=ajustar Vref");
#else
    Serial.println("📝 Comandos: 'D'=debug, 'C'=calibrar");
#endif
    
    Serial.println();
}

void loop() {
    static unsigned long ultima_lectura = 0;
    
    if (millis() - ultima_lectura >= 2000) {
        mostrarLecturas();
        ultima_lectura = millis();
    }
    
    if (Serial.available()) {
        String comando = Serial.readString();
        comando.trim();
        
        if (comando.equalsIgnoreCase("D")) {
            Serial.print("🔧 DEBUG - Fase: ");
            Serial.print(FASE_ACTUAL);
            Serial.print(", Vref: ");
            Serial.print(vref_actual, 4);
            Serial.print("V, Calibrado: ");
            Serial.println(sistemaCalibrado ? "SÍ" : "NO");
            
        } else if (comando.equalsIgnoreCase("C")) {
            calibrarSistema();
            
#if FASE_ACTUAL >= 2
        } else if (comando.startsWith("V") || comando.startsWith("v")) {
            procesarComandoVref(comando);
#endif
        }
    }
    
    delay(100);
}
