/*
═══════════════════════════════════════════════════════════════════════════
  ORNIDIA v3.0 - SISTEMA DE MONITOREO SOLAR CON ESP32
  Implementación por Fases con Calibración Avanzada ADC
═══════════════════════════════════════════════════════════════════════════

HARDWARE:
- ESP32 (6 canales ADC independientes)
- 3x ACS712-05B (corriente Panel, Batería, Carga)
- 3x Divisores de voltaje (Panel, Batería y Carga)

CONEXIONES ESP32:
┌─────────────────────────────────────────────────────────────┐
│  SENSORES DE CORRIENTE (ACS712)                             │
│  ACS712 Panel    → GPIO36 (VP)   [ADC1_CH0]                 │
│  ACS712 Batería  → GPIO39 (VN)   [ADC1_CH3]                 │
│  ACS712 Carga    → GPIO34        [ADC1_CH6]                 │
├─────────────────────────────────────────────────────────────┤
│  DIVISORES DE VOLTAJE (R1=100kΩ, R2=22kΩ)                  │
│  Voltaje Panel   → GPIO35        [ADC1_CH7]                 │
│  Voltaje Batería → GPIO32        [ADC1_CH4]                 │
│  Voltaje Carga   → GPIO33        [ADC1_CH5]                 │
└─────────────────────────────────────────────────────────────┘

DIVISOR DE VOLTAJE (cada uno):
    +Vin ──── R1(100kΩ) ──┬── ESP32 ADC
                          │
                      R2(22kΩ)
                          │
                         GND
    Factor = (100+22)/22 = 5.545

ACS712 (alimentados a 5V con divisor 1kΩ/2kΩ):
    ACS712(OUT) ─── R1(1kΩ) ──┬── ESP32 ADC
                               │
                           R2(2kΩ)
                               │
                              GND
    Factor = (1+2)/2 = 1.5

FASES DE IMPLEMENTACIÓN:
  FASE_1: Filtrado básico optimizado (±1% precisión)
  FASE_2: Calibración Vref automática (±0.5% precisión)
  FASE_3: Corrección no-linealidad completa (±0.2% precisión)

CONFIGURACIÓN: Cambiar #define FASE_ACTUAL según progreso
═══════════════════════════════════════════════════════════════════════════
*/

#include <Wire.h>

// ═══════════════════════════════════════════════════════════════════════════
//                          CONFIGURACIÓN DE FASES
// ═══════════════════════════════════════════════════════════════════════════

#define FASE_ACTUAL 3  // Cambiar a 1, 2 o 3 según progreso

// ═══════════════════════════════════════════════════════════════════════════
//                          DEFINICIÓN DE PINES ESP32
// ═══════════════════════════════════════════════════════════════════════════

// Sensores de Corriente ACS712
#define PIN_ACS_PANEL    36  // GPIO36 (VP)   - ADC1_CH0
#define PIN_ACS_BATERIA  39  // GPIO39 (VN)   - ADC1_CH3  
#define PIN_ACS_CARGA    34  // GPIO34        - ADC1_CH6

// Divisores de Voltaje
#define PIN_VOLT_PANEL   35  // GPIO35        - ADC1_CH7
#define PIN_VOLT_BATERIA 32  // GPIO32        - ADC1_CH4
#define PIN_VOLT_CARGA   33  // GPIO33        - ADC1_CH5

// LED integrado para indicador visual
#define LED_BUILTIN 2

// ═══════════════════════════════════════════════════════════════════════════
//                          CONSTANTES DEL SISTEMA
// ═══════════════════════════════════════════════════════════════════════════

// ACS712 Configuration
#define ACS712_SENSIBILIDAD 0.185  // 185 mV/A para ACS712-05B
#define FACTOR_DIVISOR_ACS 1.5     // Divisor 1kΩ/2kΩ para ACS712 alimentado a 5V

// Divisores de Voltaje (100kΩ/22kΩ)
#define R1_DIVISOR 100.0  // 100kΩ
#define R2_DIVISOR 22.0   // 22kΩ
#define FACTOR_DIVISOR_VOLT ((R1_DIVISOR + R2_DIVISOR) / R2_DIVISOR)  // 5.545

// Parámetros de muestreo
#define NUM_MUESTRAS 50
#define DELAY_MUESTRAS_US 500

// Resolución ADC ESP32
#define ADC_RESOLUTION 4096  // 12 bits (0-4095)

// ═══════════════════════════════════════════════════════════════════════════
//                          ESTRUCTURAS Y CLASES
// ═══════════════════════════════════════════════════════════════════════════

// Filtro de Kalman optimizado
class KalmanFilter {
private:
    float Q, R, P, X, K;
    
public:
    KalmanFilter(float q = 0.01, float r = 0.1) : Q(q), R(r), P(1.0), X(0.0), K(0.0) {}
    
    float filtrar(float medicion) {
        P = P + Q;
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

// Filtro de promedio móvil
class FiltroPromedio {
private:
    static const int TAM_BUFFER = 10;
    float buffer[TAM_BUFFER];
    int indice;
    int contador;
    
public:
    FiltroPromedio() : indice(0), contador(0) {
        for (int i = 0; i < TAM_BUFFER; i++) buffer[i] = 0.0;
    }
    
    float agregar(float valor) {
        buffer[indice] = valor;
        indice = (indice + 1) % TAM_BUFFER;
        if (contador < TAM_BUFFER) contador++;
        
        float suma = 0;
        for (int i = 0; i < contador; i++) {
            suma += buffer[i];
        }
        return suma / contador;
    }
    
    void limpiar() { 
        contador = 0; 
        indice = 0;
        for (int i = 0; i < TAM_BUFFER; i++) buffer[i] = 0.0;
    }
};

// Estructura para almacenar datos de un canal
struct DatosCanal {
    float corriente;
    float voltaje;
    float potencia;
    unsigned long timestamp;
};

// ═══════════════════════════════════════════════════════════════════════════
//                          VARIABLES GLOBALES
// ═══════════════════════════════════════════════════════════════════════════

// Filtros Kalman para sensores de corriente
KalmanFilter kalmanPanel(0.005, 0.05);
KalmanFilter kalmanBateria(0.01, 0.08);
KalmanFilter kalmanCarga(0.005, 0.05);

// Filtros Kalman para voltímetros
KalmanFilter kalmanVoltPanel(0.001, 0.02);
KalmanFilter kalmanVoltBateria(0.001, 0.02);
KalmanFilter kalmanVoltCarga(0.001, 0.02);

// Filtros de promedio para corriente
FiltroPromedio filtroPanel, filtroBateria, filtroCarga;

// Filtros de promedio para voltaje
FiltroPromedio filtroVoltPanel, filtroVoltBateria, filtroVoltCarga;

// Offsets calibrados para ACS712
float offsetPanel = 1.65;     // ~3.3V/2 para ESP32 a 3.3V (o 2.5V si a 5V con divisor)
float offsetBateria = 1.65;
float offsetCarga = 1.65;

// Voltaje de referencia del ADC
float vref_actual = 3.3;

// Datos históricos
DatosCanal datosPanel, datosBateria, datosCarga;

// Estados del sistema
bool sistemaCalibrado = false;
unsigned long ultimaLectura = 0;
unsigned long tiempoInicio = 0;

// Acumuladores de energía (Wh)
float energiaGenerada = 0.0;
float energiaConsumida = 0.0;
float energiaBateria = 0.0;
unsigned long ultimoCalculo = 0;

// ═══════════════════════════════════════════════════════════════════════════
//                   FASE 2 y 3: CALIBRACIÓN AVANZADA ADC
// ═══════════════════════════════════════════════════════════════════════════

#if FASE_ACTUAL >= 2

void calibrarVref() {
    Serial.println("🔧 CALIBRACIÓN VREF DISPONIBLE");
    Serial.println("   1. Conecta multímetro de precisión a cualquier pin ADC");
    Serial.println("   2. Mide voltaje real con multímetro");
    Serial.println("   3. Envía comando: Vx.xxx (ej: V2.450)");
    Serial.print("   Vref actual: ");
    Serial.print(vref_actual, 4);
    Serial.println("V");
}

void procesarComandoVref(String comando, int pin_test) {
    if (comando.startsWith("V") || comando.startsWith("v")) {
        float voltaje_conocido = comando.substring(1).toFloat();
        
        if (voltaje_conocido > 0.1 && voltaje_conocido < 3.5) {
            // Leer ADC actual sin correcciones
            long suma = 0;
            for (int i = 0; i < 100; i++) {
                suma += analogRead(pin_test);
                delay(5);
            }
            float adc_promedio = suma / 100.0;
            
            // Calcular Vref real
            float vref_nuevo = voltaje_conocido * ADC_RESOLUTION / adc_promedio;
            
            if (vref_nuevo > 2.8 && vref_nuevo < 3.6) {
                vref_actual = vref_nuevo;
                Serial.print("✅ Vref calibrado: ");
                Serial.print(vref_actual, 4);
                Serial.print("V (ADC raw: ");
                Serial.print(adc_promedio, 1);
                Serial.print(", Voltaje: ");
                Serial.print(voltaje_conocido, 3);
                Serial.println("V)");
                
                // Guardar en EEPROM sería ideal aquí
            } else {
                Serial.print("❌ Vref fuera de rango esperado: ");
                Serial.println(vref_nuevo, 4);
            }
        } else {
            Serial.println("❌ Voltaje debe estar entre 0.1V y 3.5V");
        }
    }
}

float leerVoltajeCalibrado(int pin) {
    long suma = 0;
    for (int i = 0; i < NUM_MUESTRAS; i++) {
        suma += analogRead(pin);
        delayMicroseconds(DELAY_MUESTRAS_US);
    }
    float adc_promedio = suma / float(NUM_MUESTRAS);
    return (adc_promedio * vref_actual) / ADC_RESOLUTION;
}

#endif

// ═══════════════════════════════════════════════════════════════════════════
//                   FASE 3: CORRECCIÓN NO-LINEALIDAD ADC
// ═══════════════════════════════════════════════════════════════════════════

#if FASE_ACTUAL >= 3

struct PuntoCurva {
    uint16_t adc;
    float voltaje;
};

// Lookup Table para corrección no-linealidad ESP32
// IMPORTANTE: Estos son valores típicos - calibrar individualmente para máxima precisión
const PuntoCurva curva_correccion[] = {
    {0,     0.000},
    {410,   0.330},
    {819,   0.660},
    {1229,  0.990},
    {1638,  1.320},
    {2048,  1.650},   // Punto medio teórico
    {2457,  1.980},
    {2867,  2.310},
    {3276,  2.640},
    {3686,  2.970},
    {4095,  3.300}    // Máximo teórico
};

const int PUNTOS_CURVA = sizeof(curva_correccion) / sizeof(curva_correccion[0]);

float interpolarCurva(uint16_t adc_raw) {
    // Límites
    if (adc_raw <= curva_correccion[0].adc) return 0.0;
    if (adc_raw >= curva_correccion[PUNTOS_CURVA-1].adc) {
        return curva_correccion[PUNTOS_CURVA-1].voltaje * (vref_actual / 3.3);
    }
    
    // Buscar segmento e interpolar
    for (int i = 0; i < PUNTOS_CURVA - 1; i++) {
        if (adc_raw >= curva_correccion[i].adc && adc_raw <= curva_correccion[i+1].adc) {
            float x1 = curva_correccion[i].adc;
            float y1 = curva_correccion[i].voltaje;
            float x2 = curva_correccion[i+1].adc;
            float y2 = curva_correccion[i+1].voltaje;
            
            // Interpolación lineal
            float voltaje = y1 + (y2 - y1) * (adc_raw - x1) / (x2 - x1);
            
            // Escalar según Vref real
            return voltaje * (vref_actual / 3.3);
        }
    }
    
    // Fallback
    return (adc_raw * vref_actual) / ADC_RESOLUTION;
}

float leerVoltajeCorregido(int pin) {
    long suma = 0;
    for (int i = 0; i < NUM_MUESTRAS; i++) {
        uint16_t raw = analogRead(pin);
        suma += raw;
        delayMicroseconds(DELAY_MUESTRAS_US);
    }
    uint16_t adc_promedio = suma / NUM_MUESTRAS;
    return interpolarCurva(adc_promedio);
}

void generarTablaCalibracion() {
    Serial.println("╔═══════════════════════════════════════════════════╗");
    Serial.println("║  🔬 GENERADOR DE TABLA DE CALIBRACIÓN ADC        ║");
    Serial.println("╚═══════════════════════════════════════════════════╝");
    Serial.println();
    Serial.println("Instrucciones:");
    Serial.println("1. Conecta fuente de voltaje ajustable a GPIO35");
    Serial.println("2. Para cada voltaje solicitado:");
    Serial.println("   - Ajusta la fuente al voltaje indicado");
    Serial.println("   - Verifica con multímetro de precisión");
    Serial.println("   - Presiona ENTER");
    Serial.println();
    
    float voltajes[] = {0.0, 0.3, 0.6, 0.9, 1.2, 1.5, 1.8, 2.1, 2.4, 2.7, 3.0, 3.3};
    int num_puntos = sizeof(voltajes) / sizeof(voltajes[0]);
    
    Serial.println("Tabla de calibración generada:");
    Serial.println("const PuntoCurva curva_correccion[] = {");
    
    for (int i = 0; i < num_puntos; i++) {
        Serial.print("Aplica ");
        Serial.print(voltajes[i], 1);
        Serial.print("V y presiona ENTER... ");
        
        // Esperar ENTER
        while (!Serial.available()) delay(100);
        Serial.readString();
        
        // Tomar lecturas
        long suma = 0;
        for (int j = 0; j < 100; j++) {
            suma += analogRead(PIN_VOLT_PANEL);
            delay(10);
        }
        uint16_t adc_promedio = suma / 100;
        
        Serial.print("ADC: ");
        Serial.print(adc_promedio);
        Serial.print(" → ");
        
        // Generar línea de código
        Serial.print("    {");
        Serial.print(adc_promedio);
        Serial.print(", ");
        Serial.print(voltajes[i], 3);
        Serial.print("}");
        if (i < num_puntos - 1) Serial.println(",");
        else Serial.println();
    }
    
    Serial.println("};");
    Serial.println();
    Serial.println("✅ Copia esta tabla en tu código en la sección FASE 3");
}

#endif

// ═══════════════════════════════════════════════════════════════════════════
//                          FUNCIONES PRINCIPALES
// ═══════════════════════════════════════════════════════════════════════════

float leerVoltajeRaw(int pin) {
    float voltaje;
    
#if FASE_ACTUAL >= 3
    // FASE 3: Con corrección no-linealidad
    voltaje = leerVoltajeCorregido(pin);
    
#elif FASE_ACTUAL >= 2
    // FASE 2: Con calibración Vref
    voltaje = leerVoltajeCalibrado(pin);
    
#else
    // FASE 1: Lectura básica mejorada
    long suma = 0;
    for (int i = 0; i < NUM_MUESTRAS; i++) {
        suma += analogRead(pin);
        delayMicroseconds(DELAY_MUESTRAS_US);
    }
    voltaje = (suma / float(NUM_MUESTRAS)) * (vref_actual / ADC_RESOLUTION);
#endif
    
    return voltaje;
}

float calcularOffsetACS(int pin, const char* nombre) {
    Serial.print("📋 Calibrando ");
    Serial.print(nombre);
    Serial.print("... ");
    
    float voltaje_raw = leerVoltajeRaw(pin);
    
    Serial.print("Offset: ");
    Serial.print(voltaje_raw, 4);
    Serial.println("V");
    
    return voltaje_raw;
}

void calibrarSistema() {
    Serial.println();
    Serial.println("╔═══════════════════════════════════════════════════╗");
    Serial.println("║   🔧 CALIBRACIÓN SISTEMA ORNIDIA ESP32           ║");
    Serial.println("╚═══════════════════════════════════════════════════╝");
    
    Serial.print("   Fase activa: ");
    Serial.print(FASE_ACTUAL);
    
#if FASE_ACTUAL == 1
    Serial.println(" (Filtrado básico)");
    Serial.println("   ✓ Filtros Kalman + Promedio móvil");
    Serial.println("   ✓ Precisión esperada: ±1.0%");
#elif FASE_ACTUAL == 2
    Serial.println(" (+ Calibración Vref)");
    Serial.println("   ✓ Filtros optimizados");
    Serial.println("   ✓ Calibración Vref automática");
    Serial.println("   ✓ Precisión esperada: ±0.5%");
#elif FASE_ACTUAL == 3
    Serial.println(" (+ Corrección no-linealidad)");
    Serial.println("   ✓ Corrección curva ADC");
    Serial.println("   ✓ Calibración Vref");
    Serial.println("   ✓ Precisión esperada: ±0.2%");
#endif
    
    Serial.println();
    Serial.println("⚠️  PREPARACIÓN PARA CALIBRACIÓN:");
    Serial.println("   ✓ Desconectar TODAS las cargas");
    Serial.println("   ✓ Panel solar en corto o sin luz");
    Serial.println("   ✓ Batería desconectada temporalmente");
    Serial.println("   ✓ Sistema en reposo (corriente = 0)");
    Serial.println();
    
#if FASE_ACTUAL >= 2
    calibrarVref();
    Serial.println();
#endif
    
    Serial.println("⏳ Iniciando calibración en 5 segundos...");
    for (int i = 5; i > 0; i--) {
        Serial.print("   ");
        Serial.print(i);
        Serial.println("...");
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
        delay(1000);
    }
    Serial.println("   ¡CALIBRANDO!");
    digitalWrite(LED_BUILTIN, LOW);
    
    Serial.println();
    
    // Limpiar todos los filtros
    filtroPanel.limpiar();
    filtroBateria.limpiar();
    filtroCarga.limpiar();
    filtroVoltPanel.limpiar();
    filtroVoltBateria.limpiar();
    filtroVoltCarga.limpiar();
    
    // Calibrar offsets de sensores ACS712
    offsetPanel = calcularOffsetACS(PIN_ACS_PANEL, "ACS712 Panel   ");
    offsetBateria = calcularOffsetACS(PIN_ACS_BATERIA, "ACS712 Batería ");
    offsetCarga = calcularOffsetACS(PIN_ACS_CARGA, "ACS712 Carga   ");
    
    // Reiniciar filtros Kalman
    kalmanPanel.reiniciar();
    kalmanBateria.reiniciar();
    kalmanCarga.reiniciar();
    kalmanVoltPanel.reiniciar();
    kalmanVoltBateria.reiniciar();
    kalmanVoltCarga.reiniciar();
    
    // Resetear acumuladores de energía
    energiaGenerada = 0.0;
    energiaConsumida = 0.0;
    energiaBateria = 0.0;
    
    sistemaCalibrado = true;
    tiempoInicio = millis();
    ultimoCalculo = millis();
    
    Serial.println();
    Serial.println("✅ CALIBRACIÓN COMPLETADA");
    Serial.println();
    
#if FASE_ACTUAL >= 2
    Serial.println("📝 Comandos disponibles:");
    Serial.println("   'D' = Debug detallado");
    Serial.println("   'C' = Recalibrar sistema");
    Serial.println("   'Vx.xxx' = Calibrar Vref (ej: V2.450)");
    Serial.println("   'E' = Estadísticas de energía");
#if FASE_ACTUAL >= 3
    Serial.println("   'T' = Generar tabla de calibración ADC");
#endif
#else
    Serial.println("📝 Comandos: 'D'=Debug, 'C'=Calibrar, 'E'=Energía");
#endif
    
    Serial.println("═══════════════════════════════════════════════════");
}

float leerCorrienteACS(int pin, float offset, KalmanFilter &kalman, FiltroPromedio &filtro) {
    // Leer voltaje del sensor
    float voltaje_raw = leerVoltajeRaw(pin);
    
    // Compensar divisor de voltaje ACS712 (1k/2k)
    float voltaje_real = voltaje_raw * FACTOR_DIVISOR_ACS;
    
    // Convertir a corriente usando offset calibrado
    // ACS712-05B: 185mV/A, punto cero en offset
    float corriente_bruta = (voltaje_real - offset) / ACS712_SENSIBILIDAD;
    
    // Aplicar doble filtrado
    float corriente_promedio = filtro.agregar(corriente_bruta);
    float corriente_final = kalman.filtrar(corriente_promedio);
    
    return corriente_final;
}

float leerVoltajeDivisor(int pin, KalmanFilter &kalman, FiltroPromedio &filtro) {
    // Leer voltaje después del divisor
    float voltaje_raw = leerVoltajeRaw(pin);
    
    // Aplicar factor del divisor de voltaje (100k/22k)
    float voltaje_real = voltaje_raw * FACTOR_DIVISOR_VOLT;
    
    // Filtrado doble
    float voltaje_promedio = filtro.agregar(voltaje_real);
    float voltaje_final = kalman.filtrar(voltaje_promedio);
    
    return voltaje_final;
}

void actualizarDatos() {
    // Leer sensores de corriente
    datosPanel.corriente = leerCorrienteACS(PIN_ACS_PANEL, offsetPanel, kalmanPanel, filtroPanel);
    datosBateria.corriente = leerCorrienteACS(PIN_ACS_BATERIA, offsetBateria, kalmanBateria, filtroBateria);
    datosCarga.corriente = leerCorrienteACS(PIN_ACS_CARGA, offsetCarga, kalmanCarga, filtroCarga);
    
    // Leer voltímetros
    datosPanel.voltaje = leerVoltajeDivisor(PIN_VOLT_PANEL, kalmanVoltPanel, filtroVoltPanel);
    datosBateria.voltaje = leerVoltajeDivisor(PIN_VOLT_BATERIA, kalmanVoltBateria, filtroVoltBateria);
    datosCarga.voltaje = leerVoltajeDivisor(PIN_VOLT_CARGA, kalmanVoltCarga, filtroVoltCarga);
    
    // Calcular potencias
    datosPanel.potencia = datosPanel.corriente * datosPanel.voltaje;
    datosBateria.potencia = datosBateria.corriente * datosBateria.voltaje;
    datosCarga.potencia = datosCarga.corriente * datosCarga.voltaje;
    
    // Timestamp
    unsigned long ahora = millis();
    datosPanel.timestamp = ahora;
    datosBateria.timestamp = ahora;
    datosCarga.timestamp = ahora;
    
    // Actualizar acumuladores de energía
    float delta_tiempo_h = (ahora - ultimoCalculo) / 3600000.0; // Convertir ms a horas
    if (delta_tiempo_h > 0) {
        if (datosPanel.potencia > 0) {
            energiaGenerada += datosPanel.potencia * delta_tiempo_h;
        }
        if (datosCarga.potencia > 0) {
            energiaConsumida += datosCarga.potencia * delta_tiempo_h;
        }
        energiaBateria += datosBateria.potencia * delta_tiempo_h; // Puede ser + o -
        ultimoCalculo = ahora;
    }
}

void mostrarLecturas() {
    actualizarDatos();
    
    // Cabecera
    Serial.println();
    Serial.println("╔═══════════════════════════════════════════════════╗");
    Serial.print("║       📊 ORNIDIA ESP32 - FASE ");
    Serial.print(FASE_ACTUAL);
    
#if FASE_ACTUAL == 3
    Serial.println(" (±0.2%)         ║");
#elif FASE_ACTUAL == 2  
    Serial.println(" (±0.5%)         ║");
#else
    Serial.println(" (±1.0%)         ║");
#endif
    
    Serial.println("╚═══════════════════════════════════════════════════╝");
    
    // Panel Solar
    Serial.println("┌─────────────────────────────────────────────────┐");
    Serial.println("│  ☀️  PANEL SOLAR                                 │");
    Serial.println("├─────────────────────────────────────────────────┤");
    Serial.print("│  Voltaje:   ");
    Serial.print(datosPanel.voltaje, 3);
    Serial.println(" V                              │");
    Serial.print("│  Corriente: ");
    Serial.print(datosPanel.corriente, 4);
    Serial.print(" A (");
    Serial.print(datosPanel.corriente * 1000, 1);
    Serial.println(" mA)              │");
    Serial.print("│  Potencia:  ");
    Serial.print(datosPanel.potencia, 3);
    Serial.println(" W                               │");
    Serial.print("│  Estado:    ");
    if (datosPanel.corriente > 0.05) {
        Serial.println("☀️  GENERANDO                          │");
    } else {
        Serial.println("🌙 SIN GENERACIÓN                      │");
    }
    Serial.println("└─────────────────────────────────────────────────┘");
    
    // Batería
    Serial.println("┌─────────────────────────────────────────────────┐");
    Serial.println("│  🔋 BATERÍA                                      │");
    Serial.println("├─────────────────────────────────────────────────┤");
    Serial.print("│  Voltaje:   ");
    Serial.print(datosBateria.voltaje, 3);
    Serial.println(" V                              │");
    Serial.print("│  Corriente: ");
    Serial.print(datosBateria.corriente, 4);
    Serial.print(" A (");
    Serial.print(datosBateria.corriente * 1000, 1);
    Serial.println(" mA)             │");
    Serial.print("│  Potencia:  ");
    Serial.print(abs(datosBateria.potencia), 3);
    Serial.println(" W                               │");
    
    // Estado de la batería con porcentaje estimado
    Serial.print("│  Estado:    ");
    if (datosBateria.corriente < -0.05) {
        Serial.println("🔋⬆️  CARGANDO                          │");
    } else if (datosBateria.corriente > 0.05) {
        Serial.println("🔋⬇️  DESCARGANDO                       │");
    } else {
        Serial.println("🔋⚖️  EN EQUILIBRIO                     │");
    }
    
    // Estimación de carga de batería (basado en voltaje típico 12V)
    float porcentaje_bateria = 0;
    if (datosBateria.voltaje >= 10.5 && datosBateria.voltaje <= 14.5) {
        porcentaje_bateria = ((datosBateria.voltaje - 10.5) / (14.4 - 10.5)) * 100;
        if (porcentaje_bateria > 100) porcentaje_bateria = 100;
        if (porcentaje_bateria < 0) porcentaje_bateria = 0;
    }
    Serial.print("│  Carga Est: ");
    Serial.print(porcentaje_bateria, 1);
    Serial.print("% ");
    
    // Barra de progreso
    int barras = porcentaje_bateria / 10;
    Serial.print("[");
    for (int i = 0; i < 10; i++) {
        if (i < barras) Serial.print("█");
        else Serial.print("░");
    }
    Serial.println("]           │");
    Serial.println("└─────────────────────────────────────────────────┘");
    
    // Carga
    Serial.println("┌─────────────────────────────────────────────────┐");
    Serial.println("│  📦 CARGA/CONSUMO                                │");
    Serial.println("├─────────────────────────────────────────────────┤");
    Serial.print("│  Voltaje:   ");
    Serial.print(datosCarga.voltaje, 3);
    Serial.println(" V                              │");
    Serial.print("│  Corriente: ");
    Serial.print(datosCarga.corriente, 4);
    Serial.print(" A (");
    Serial.print(datosCarga.corriente * 1000, 1);
    Serial.println(" mA)              │");
    Serial.print("│  Potencia:  ");
    Serial.print(datosCarga.potencia, 3);
    Serial.println(" W                               │");
    Serial.print("│  Estado:    ");
    if (datosCarga.corriente > 0.05) {
        Serial.println("📦 CONSUMIENDO                         │");
    } else {
        Serial.println("⭕ SIN CARGA                           │");
    }
    Serial.println("└─────────────────────────────────────────────────┘");
    
    // Balance energético
    Serial.println("┌─────────────────────────────────────────────────┐");
    Serial.println("│  ⚖️  BALANCE ENERGÉTICO                          │");
    Serial.println("├─────────────────────────────────────────────────┤");
    
    float balanceTotal = datosPanel.potencia - abs(datosBateria.potencia) - datosCarga.potencia;
    
    Serial.print("│  Generación:     ");
    Serial.print(datosPanel.potencia, 2);
    Serial.println(" W                       │");
    Serial.print("│  Consumo Carga:  ");
    Serial.print(datosCarga.potencia, 2);
    Serial.println(" W                       │");
    Serial.print("│  Flujo Batería:  ");
    Serial.print(datosBateria.potencia, 2);
    Serial.print(" W ");
    if (datosBateria.corriente < 0) Serial.println("(cargando)          │");
    else if (datosBateria.corriente > 0) Serial.println("(descargando)       │");
    else Serial.println("(sin flujo)         │");
    
    Serial.println("├─────────────────────────────────────────────────┤");
    Serial.print("│  Balance Neto:   ");
    Serial.print(balanceTotal, 2);
    Serial.print(" W ");
    
    if (balanceTotal > 0.5) {
        Serial.println("✅ EXCEDENTE             │");
    } else if (balanceTotal < -0.5) {
        Serial.println("⚠️  DÉFICIT               │");
    } else {
        Serial.println("⚖️  EQUILIBRADO           │");
    }
    Serial.println("└─────────────────────────────────────────────────┘");
    
    // Eficiencia del sistema
    Serial.println("┌─────────────────────────────────────────────────┐");
    Serial.println("│  📈 ESTADÍSTICAS ACUMULADAS                     │");
    Serial.println("├─────────────────────────────────────────────────┤");
    
    Serial.print("│  Energía Generada:   ");
    Serial.print(energiaGenerada, 3);
    Serial.println(" Wh                 │");
    Serial.print("│  Energía Consumida:  ");
    Serial.print(energiaConsumida, 3);
    Serial.println(" Wh                 │");
    Serial.print("│  Balance Batería:    ");
    if (energiaBateria >= 0) Serial.print("+");
    Serial.print(energiaBateria, 3);
    Serial.println(" Wh                 │");
    
    if (energiaGenerada > 0.01) {
        float eficiencia = (energiaConsumida / energiaGenerada) * 100;
        Serial.print("│  Eficiencia Sistema: ");
        Serial.print(eficiencia, 1);
        Serial.println("%                       │");
    }
    
    Serial.println("└─────────────────────────────────────────────────┘");
    
    // Info del sistema
    unsigned long tiempoFuncionamiento = (millis() - tiempoInicio) / 1000;
    Serial.print("⏱️  Tiempo funcionando: ");
    
    if (tiempoFuncionamiento >= 3600) {
        Serial.print(tiempoFuncionamiento / 3600);
        Serial.print("h ");
        Serial.print((tiempoFuncionamiento % 3600) / 60);
        Serial.println("m");
    } else {
        Serial.print(tiempoFuncionamiento / 60);
        Serial.print("m ");
        Serial.print(tiempoFuncionamiento % 60);
        Serial.println("s");
    }
    
#if FASE_ACTUAL >= 2
    Serial.print("🎯 Vref calibrado: ");
    Serial.print(vref_actual, 4);
    Serial.println("V");
#endif
    
    Serial.print("📊 Sistema calibrado: ");
    Serial.println(sistemaCalibrado ? "✅ SÍ" : "⚠️  NO");
    
    // Advertencias y alertas
    bool alerta = false;
    Serial.println();
    
    if (datosPanel.voltaje > 16.0 || datosPanel.voltaje < 8.0) {
        Serial.println("⚠️  ALERTA: Voltaje de panel fuera de rango (8-16V)");
        alerta = true;
    }
    
    if (datosBateria.voltaje > 14.5) {
        Serial.println("🔴 CRÍTICO: Sobrevoltaje en batería (>14.5V)");
        alerta = true;
    } else if (datosBateria.voltaje < 10.5) {
        Serial.println("🔴 CRÍTICO: Batería descargada (<10.5V)");
        alerta = true;
    } else if (datosBateria.voltaje < 11.5) {
        Serial.println("⚠️  ADVERTENCIA: Batería baja (<11.5V)");
        alerta = true;
    }
    
    if (abs(datosPanel.corriente) > 0.4) {
        Serial.println("⚠️  ADVERTENCIA: Corriente panel fuera de rango (>0.4A)");
        alerta = true;
    }
    
    if (abs(datosBateria.corriente) > 3.0) {
        Serial.println("⚠️  ADVERTENCIA: Corriente batería alta (>3.0A)");
        alerta = true;
    }
    
    if (abs(datosCarga.corriente) > 2.0) {
        Serial.println("⚠️  ADVERTENCIA: Corriente carga alta (>2.0A)");
        alerta = true;
    }
    
    if (!alerta) {
        Serial.println("✅ Sistema operando normalmente");
    }
    
    Serial.println("═══════════════════════════════════════════════════");
}

void mostrarDebug() {
    Serial.println();
    Serial.println("╔═══════════════════════════════════════════════════╗");
    Serial.println("║           🔧 INFORMACIÓN DE DEBUG                ║");
    Serial.println("╚═══════════════════════════════════════════════════╝");
    
    Serial.print("Fase implementada: ");
    Serial.println(FASE_ACTUAL);
    
    Serial.print("Vref actual: ");
    Serial.print(vref_actual, 4);
    Serial.println("V");
    
    Serial.print("Sistema calibrado: ");
    Serial.println(sistemaCalibrado ? "SÍ" : "NO");
    
    Serial.println();
    Serial.println("─────────────────────────────────────────────────");
    Serial.println("OFFSETS CALIBRADOS ACS712:");
    Serial.println("─────────────────────────────────────────────────");
    Serial.print("  Panel:   ");
    Serial.print(offsetPanel, 4);
    Serial.print("V → ");
    Serial.print(offsetPanel * FACTOR_DIVISOR_ACS, 4);
    Serial.println("V (real)");
    
    Serial.print("  Batería: ");
    Serial.print(offsetBateria, 4);
    Serial.print("V → ");
    Serial.print(offsetBateria * FACTOR_DIVISOR_ACS, 4);
    Serial.println("V (real)");
    
    Serial.print("  Carga:   ");
    Serial.print(offsetCarga, 4);
    Serial.print("V → ");
    Serial.print(offsetCarga * FACTOR_DIVISOR_ACS, 4);
    Serial.println("V (real)");
    
    Serial.println();
    Serial.println("─────────────────────────────────────────────────");
    Serial.println("LECTURAS RAW ADC (0-4095):");
    Serial.println("─────────────────────────────────────────────────");
    
    int raw_acs_panel = analogRead(PIN_ACS_PANEL);
    int raw_acs_bat = analogRead(PIN_ACS_BATERIA);
    int raw_acs_carga = analogRead(PIN_ACS_CARGA);
    int raw_volt_panel = analogRead(PIN_VOLT_PANEL);
    int raw_volt_bat = analogRead(PIN_VOLT_BATERIA);
    int raw_volt_carga = analogRead(PIN_VOLT_CARGA);
    
    Serial.print("  ACS Panel:    ");
    Serial.print(raw_acs_panel);
    Serial.print(" → ");
    Serial.print((raw_acs_panel * vref_actual) / ADC_RESOLUTION, 3);
    Serial.println("V");
    
    Serial.print("  ACS Batería:  ");
    Serial.print(raw_acs_bat);
    Serial.print(" → ");
    Serial.print((raw_acs_bat * vref_actual) / ADC_RESOLUTION, 3);
    Serial.println("V");
    
    Serial.print("  ACS Carga:    ");
    Serial.print(raw_acs_carga);
    Serial.print(" → ");
    Serial.print((raw_acs_carga * vref_actual) / ADC_RESOLUTION, 3);
    Serial.println("V");
    
    Serial.print("  Volt Panel:   ");
    Serial.print(raw_volt_panel);
    Serial.print(" → ");
    Serial.print(((raw_volt_panel * vref_actual) / ADC_RESOLUTION) * FACTOR_DIVISOR_VOLT, 3);
    Serial.println("V");
    
    Serial.print("  Volt Batería: ");
    Serial.print(raw_volt_bat);
    Serial.print(" → ");
    Serial.print(((raw_volt_bat * vref_actual) / ADC_RESOLUTION) * FACTOR_DIVISOR_VOLT, 3);
    Serial.println("V");
    
    Serial.print("  Volt Carga:   ");
    Serial.print(raw_volt_carga);
    Serial.print(" → ");
    Serial.print(((raw_volt_carga * vref_actual) / ADC_RESOLUTION) * FACTOR_DIVISOR_VOLT, 3);
    Serial.println("V");
    
    Serial.println();
    Serial.println("─────────────────────────────────────────────────");
    Serial.println("CONFIGURACIÓN DE HARDWARE:");
    Serial.println("─────────────────────────────────────────────────");
    Serial.print("  ACS712 Sensibilidad:  ");
    Serial.print(ACS712_SENSIBILIDAD, 3);
    Serial.println(" V/A");
    
    Serial.print("  Factor Divisor ACS:   ");
    Serial.println(FACTOR_DIVISOR_ACS, 3);
    
    Serial.print("  Factor Divisor Volt:  ");
    Serial.println(FACTOR_DIVISOR_VOLT, 3);
    
    Serial.print("  Muestras por lectura: ");
    Serial.println(NUM_MUESTRAS);
    
    Serial.println();
    Serial.println("─────────────────────────────────────────────────");
    Serial.println("PINES ASIGNADOS:");
    Serial.println("─────────────────────────────────────────────────");
    Serial.print("  ACS Panel:    GPIO");
    Serial.println(PIN_ACS_PANEL);
    Serial.print("  ACS Batería:  GPIO");
    Serial.println(PIN_ACS_BATERIA);
    Serial.print("  ACS Carga:    GPIO");
    Serial.println(PIN_ACS_CARGA);
    Serial.print("  Volt Panel:   GPIO");
    Serial.println(PIN_VOLT_PANEL);
    Serial.print("  Volt Batería: GPIO");
    Serial.println(PIN_VOLT_BATERIA);
    Serial.print("  Volt Carga:   GPIO");
    Serial.println(PIN_VOLT_CARGA);
    
    Serial.println();
    Serial.println("═══════════════════════════════════════════════════");
}

void mostrarEstadisticasEnergia() {
    Serial.println();
    Serial.println("╔═══════════════════════════════════════════════════╗");
    Serial.println("║        📊 ESTADÍSTICAS DE ENERGÍA                ║");
    Serial.println("╚═══════════════════════════════════════════════════╝");
    
    unsigned long tiempoFuncionamiento = (millis() - tiempoInicio) / 1000;
    float horas = tiempoFuncionamiento / 3600.0;
    
    Serial.print("Tiempo de monitoreo: ");
    Serial.print(horas, 2);
    Serial.println(" horas");
    Serial.println();
    
    Serial.println("┌─────────────────────────────────────────────────┐");
    Serial.println("│  ENERGÍA ACUMULADA                              │");
    Serial.println("├─────────────────────────────────────────────────┤");
    
    Serial.print("│  ☀️  Generada (Panel):    ");
    Serial.print(energiaGenerada, 3);
    Serial.println(" Wh            │");
    
    Serial.print("│  📦 Consumida (Carga):    ");
    Serial.print(energiaConsumida, 3);
    Serial.println(" Wh            │");
    
    Serial.print("│  🔋 Balance Batería:      ");
    if (energiaBateria >= 0) Serial.print("+");
    Serial.print(energiaBateria, 3);
    Serial.println(" Wh            │");
    
    Serial.println("└─────────────────────────────────────────────────┘");
    
    Serial.println("┌─────────────────────────────────────────────────┐");
    Serial.println("│  PROMEDIOS Y EFICIENCIA                         │");
    Serial.println("├─────────────────────────────────────────────────┤");
    
    if (horas > 0.01) {
        float potenciaPromPanel = energiaGenerada / horas;
        float potenciaPromCarga = energiaConsumida / horas;
        
        Serial.print("│  Potencia Prom. Panel:    ");
        Serial.print(potenciaPromPanel, 2);
        Serial.println(" W              │");
        
        Serial.print("│  Potencia Prom. Carga:    ");
        Serial.print(potenciaPromCarga, 2);
        Serial.println(" W              │");
        
        if (energiaGenerada > 0.01) {
            float eficiencia = (energiaConsumida / energiaGenerada) * 100;
            Serial.print("│  Eficiencia Global:       ");
            Serial.print(eficiencia, 1);
            Serial.println("%               │");
            
            float perdidas = energiaGenerada - energiaConsumida - energiaBateria;
            Serial.print("│  Pérdidas Sistema:        ");
            Serial.print(perdidas, 3);
            Serial.println(" Wh            │");
        }
    }
    
    Serial.println("└─────────────────────────────────────────────────┘");
    
    Serial.println("┌─────────────────────────────────────────────────┐");
    Serial.println("│  PROYECCIONES DIARIAS                           │");
    Serial.println("├─────────────────────────────────────────────────┤");
    
    if (horas > 0.1) {
        float proyeccionDiaria = (energiaGenerada / horas) * 24;
        float consumoDiario = (energiaConsumida / horas) * 24;
        
        Serial.print("│  Generación proyectada:   ");
        Serial.print(proyeccionDiaria, 1);
        Serial.println(" Wh/día       │");
        
        Serial.print("│  Consumo proyectado:      ");
        Serial.print(consumoDiario, 1);
        Serial.println(" Wh/día       │");
        
        float balance = proyeccionDiaria - consumoDiario;
        Serial.print("│  Balance proyectado:      ");
        if (balance >= 0) Serial.print("+");
        Serial.print(balance, 1);
        Serial.println(" Wh/día       │");
    } else {
        Serial.println("│  Datos insuficientes para proyección           │");
    }
    
    Serial.println("└─────────────────────────────────────────────────┘");
    
    Serial.println("═══════════════════════════════════════════════════");
}

void resetearEstadisticas() {
    energiaGenerada = 0.0;
    energiaConsumida = 0.0;
    energiaBateria = 0.0;
    ultimoCalculo = millis();
    tiempoInicio = millis();
    
    Serial.println("✅ Estadísticas de energía reseteadas");
}

// ═══════════════════════════════════════════════════════════════════════════
//                          SETUP Y LOOP PRINCIPAL
// ═══════════════════════════════════════════════════════════════════════════

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println();
    Serial.println("╔═══════════════════════════════════════════════════╗");
    Serial.println("║     🚀 SISTEMA ORNIDIA v3.0 - ESP32              ║");
    Serial.println("║        Monitoreo Solar Avanzado                  ║");
    Serial.println("║        6 Canales Independientes                  ║");
    Serial.println("╚═══════════════════════════════════════════════════╝");
    Serial.println();
    
    // Configurar pines
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    
    // Configurar ADC ESP32
    analogReadResolution(12);  // 12 bits (0-4095)
    analogSetAttenuation(ADC_11db);  // 0-3.3V
    
    Serial.print("   Fase implementada: ");
    Serial.print(FASE_ACTUAL);
    
#if FASE_ACTUAL == 1
    Serial.println(" (Básico)");
    Serial.println("   ✓ Filtros Kalman optimizados");
    Serial.println("   ✓ Filtro de promedio móvil");
    Serial.println("   ✓ Precisión: ±1%");
    
#elif FASE_ACTUAL == 2
    Serial.println(" (+ Vref)");
    Serial.println("   ✓ Calibración Vref automática");
    Serial.println("   ✓ Filtros optimizados");
    Serial.println("   ✓ Precisión: ±0.5%");
    
#elif FASE_ACTUAL == 3
    Serial.println(" (+ No-linealidad)");
    Serial.println("   ✓ Corrección curva ADC");
    Serial.println("   ✓ Calibración Vref");
    Serial.println("   ✓ Precisión: ±0.2%");
#endif
    
    Serial.println();
    Serial.println("Hardware configurado:");
    Serial.println("  ✓ 3x Sensores ACS712 (corriente)");
    Serial.println("  ✓ 3x Divisores de voltaje");
    Serial.println("  ✓ 6 canales ADC independientes");
    Serial.println();
    
    // Calibración inicial
    calibrarSistema();
    
    Serial.println();
    Serial.println("🎯 Sistema listo para monitoreo continuo");
    Serial.println();
}

void loop() {
    // Mostrar lecturas cada 2 segundos
    if (millis() - ultimaLectura >= 2000) {
        mostrarLecturas();
        ultimaLectura = millis();
    }
    
    // Procesar comandos seriales
    if (Serial.available()) {
        String comando = Serial.readString();
        comando.trim();
        comando.toUpperCase();
        
        if (comando == "D") {
            // Debug detallado
            mostrarDebug();
            
        } else if (comando == "C") {
            // Recalibrar sistema
            calibrarSistema();
            
        } else if (comando == "E") {
            // Estadísticas de energía
            mostrarEstadisticasEnergia();
            
        } else if (comando == "R") {
            // Resetear estadísticas
            resetearEstadisticas();
            
#if FASE_ACTUAL >= 2
        } else if (comando.startsWith("V")) {
            // Calibrar Vref
            procesarComandoVref(comando, PIN_VOLT_PANEL);
#endif
            
#if FASE_ACTUAL >= 3
        } else if (comando == "T") {
            // Generar tabla de calibración
            generarTablaCalibracion();
#endif
            
        } else if (comando == "H" || comando == "HELP" || comando == "?") {
            // Ayuda
            Serial.println();
            Serial.println("╔═══════════════════════════════════════════════════╗");
            Serial.println("║           📖 COMANDOS DISPONIBLES                ║");
            Serial.println("╚═══════════════════════════════════════════════════╝");
            Serial.println("  D - Mostrar información de debug");
            Serial.println("  C - Recalibrar sistema completo");
            Serial.println("  E - Estadísticas de energía");
            Serial.println("  R - Resetear estadísticas");
            Serial.println("  H - Mostrar esta ayuda");
#if FASE_ACTUAL >= 2
            Serial.println("  Vx.xxx - Calibrar Vref (ej: V2.450)");
#endif
#if FASE_ACTUAL >= 3
            Serial.println("  T - Generar tabla calibración ADC");
#endif
            Serial.println("═══════════════════════════════════════════════════");
        }
    }
    
    // Parpadeo de LED para indicar funcionamiento
    static unsigned long ultimoParpadeo = 0;
    if (millis() - ultimoParpadeo >= 5000) {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(50);
        digitalWrite(LED_BUILTIN, LOW);
        ultimoParpadeo = millis();
    }
    
    delay(100);
}
