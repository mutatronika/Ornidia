/*
  =======================================================
  ORNIDIA v4.0 - Plataforma ESP32 (6 canales analógicos)
  =======================================================
  FASE_1: Lectura filtrada (promedio + Kalman)
  FASE_2: Calibración Vref automática
  FASE_3: Corrección no-linealidad ADC + Vref
*/

#include <Arduino.h>

// =====================================================
// 🔧 CONFIGURACIÓN GENERAL
// =====================================================
#define FASE_ACTUAL 3
#define NUM_MUESTRAS 50
#define ACS_SENSIBILIDAD 0.185  // 185 mV/A (modelo 5A)
float vref_actual = 3.3;

// Pines analógicos (ESP32)
#define PIN_I_PANEL   34
#define PIN_I_BATERIA 35
#define PIN_I_CARGA   32
#define PIN_V_PANEL   33
#define PIN_V_BATERIA 25
#define PIN_V_CARGA   26

// =====================================================
// 🔹 FILTRO KALMAN
// =====================================================
struct Kalman {
  float Q, R, P, X, K;
  Kalman(float q=0.01, float r=0.1) : Q(q), R(r), P(1.0), X(0.0), K(0.0) {}
  float filtrar(float medida) {
    P = P + Q;
    K = P / (P + R);
    X = X + K * (medida - X);
    P = (1 - K) * P;
    return X;
  }
  void reset() { X = 0.0; P = 1.0; }
};

// =====================================================
// 🔹 FILTRO PROMEDIO MÓVIL
// =====================================================
class FiltroPromedio {
  float buf[10];
  int idx = 0, count = 0;
public:
  float agregar(float valor) {
    buf[idx] = valor;
    idx = (idx + 1) % 10;
    if (count < 10) count++;
    float sum = 0;
    for (int i = 0; i < count; i++) sum += buf[i];
    return sum / count;
  }
  void limpiar() { idx = 0; count = 0; }
};

// =====================================================
// 🔹 CORRECCIÓN NO LINEALIDAD (FASE 3)
// =====================================================
#if FASE_ACTUAL >= 3
struct PuntoCurva { uint16_t adc; float voltaje; };

// ⚠️ Reemplazar estos datos con los obtenidos en tu calibración Excel
const PuntoCurva curva_correccion[] = {
  { 0, 0.000 }, { 380, 0.300 }, { 760, 0.600 },
  { 1140, 0.900 }, { 1520, 1.200 }, { 1900, 1.500 },
  { 2280, 1.800 }, { 2660, 2.100 }, { 3040, 2.400 },
  { 3420, 2.700 }, { 3800, 3.000 }, { 4095, 3.200 }
};
const int PUNTOS_CURVA = sizeof(curva_correccion)/sizeof(curva_correccion[0]);

float interpolarCurva(uint16_t raw) {
  if (raw <= curva_correccion[0].adc) return 0;
  if (raw >= curva_correccion[PUNTOS_CURVA-1].adc)
    return curva_correccion[PUNTOS_CURVA-1].voltaje * (vref_actual/3.2);

  for (int i=0;i<PUNTOS_CURVA-1;i++) {
    if (raw >= curva_correccion[i].adc && raw <= curva_correccion[i+1].adc) {
      float x1 = curva_correccion[i].adc, y1 = curva_correccion[i].voltaje;
      float x2 = curva_correccion[i+1].adc, y2 = curva_correccion[i+1].voltaje;
      float v = y1 + (y2 - y1) * (raw - x1) / (x2 - x1);
      return v * (vref_actual/3.2);
    }
  }
  return (raw * vref_actual) / 4095.0;
}
#endif

// =====================================================
// 🔹 CALIBRACIÓN DE VREF (FASE 2)
// =====================================================
#if FASE_ACTUAL >= 2
void calibrarVref() {
  Serial.println(F("🔧 Iniciando calibración Vref"));
  Serial.println(F("   Envía por serial: Vx.xxx (ej: V2.489)"));
  Serial.print(F("   Vref actual: ")); Serial.println(vref_actual, 4);
}

void procesarComandoVref(String cmd) {
  if (cmd.startsWith("V") || cmd.startsWith("v")) {
    float v_conocido = cmd.substring(1).toFloat();
    if (v_conocido < 0.1 || v_conocido > 3.5) {
      Serial.println(F("❌ Voltaje fuera de rango (0.1–3.5 V)"));
      return;
    }
    long suma = 0;
    for (int i=0;i<100;i++) suma += analogRead(PIN_V_PANEL);
    float adc_prom = suma/100.0;
    float nuevo_vref = v_conocido * 4095.0 / adc_prom;
    if (nuevo_vref > 2.8 && nuevo_vref < 3.6) {
      vref_actual = nuevo_vref;
      Serial.print(F("✅ Nuevo Vref calibrado: "));
      Serial.println(vref_actual,4);
    } else {
      Serial.println(F("❌ Resultado fuera de rango esperado"));
    }
  }
}
#endif

// =====================================================
// 🔹 LECTURAS ANALÓGICAS
// =====================================================
float leerVoltajeADC(int pin) {
  long suma = 0;
  for (int i=0;i<NUM_MUESTRAS;i++) {
    suma += analogRead(pin);
    delayMicroseconds(300);
  }
  uint16_t raw = suma/NUM_MUESTRAS;

#if FASE_ACTUAL >= 3
  return interpolarCurva(raw);
#elif FASE_ACTUAL >= 2
  return (raw * vref_actual) / 4095.0;
#else
  return (raw * 3.3) / 4095.0;
#endif
}

// =====================================================
// 🔹 VARIABLES GLOBALES DE FILTROS
// =====================================================
Kalman kIpanel, kIbateria, kIcarga, kVpanel, kVbateria, kVcarga;
FiltroPromedio fIpanel, fIbateria, fIcarga;
float offsetIpanel=2.5, offsetIbateria=2.5, offsetIcarga=2.5;

// =====================================================
// 🔹 LECTURAS Y CÁLCULOS
// =====================================================
float leerCorriente(int pin, float offset, Kalman &kal, FiltroPromedio &f) {
  float v = leerVoltajeADC(pin);
  float i = (v - offset) / ACS_SENSIBILIDAD;
  return kal.filtrar(f.agregar(i));
}

float leerVoltajeDivisor(int pin, float r1, float r2, Kalman &kal) {
  float v = leerVoltajeADC(pin);
  return kal.filtrar(v * ((r1 + r2) / r2));
}

// =====================================================
// 🔹 CALIBRACIÓN INICIAL
// =====================================================
void calibrarSistema() {
  Serial.println(F("🔧 Calibrando sistema ORNIDIA..."));
  delay(1000);

  offsetIpanel  = leerVoltajeADC(PIN_I_PANEL);
  offsetIbateria= leerVoltajeADC(PIN_I_BATERIA);
  offsetIcarga  = leerVoltajeADC(PIN_I_CARGA);

  fIpanel.limpiar(); fIbateria.limpiar(); fIcarga.limpiar();
  kIpanel.reset(); kIbateria.reset(); kIcarga.reset();
  kVpanel.reset(); kVbateria.reset(); kVcarga.reset();

  Serial.println(F("✅ Calibración completada."));
}

// =====================================================
// 🔹 CONFIGURACIÓN INICIAL
// =====================================================
void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
  Serial.println();
  Serial.println(F("🚀 ORNIDIA v4.0 - ESP32 Edition"));
  Serial.print(F("   Fase actual: ")); Serial.println(FASE_ACTUAL);
  calibrarSistema();

#if FASE_ACTUAL >= 2
  calibrarVref();
#endif
}

// =====================================================
// 🔹 LOOP PRINCIPAL
// =====================================================
void loop() {
  float iPanel   = leerCorriente(PIN_I_PANEL, offsetIpanel, kIpanel, fIpanel);
  float iBateria = leerCorriente(PIN_I_BATERIA, offsetIbateria, kIbateria, fIbateria);
  float iCarga   = leerCorriente(PIN_I_CARGA, offsetIcarga, kIcarga, fIcarga);

  float vPanel   = leerVoltajeDivisor(PIN_V_PANEL, 100.0, 22.0, kVpanel);   // R1=100k, R2=22k
  float vBateria = leerVoltajeDivisor(PIN_V_BATERIA, 100.0, 22.0, kVbateria);
  float vCarga   = leerVoltajeDivisor(PIN_V_CARGA, 100.0, 22.0, kVcarga);

  float potencia = vPanel * iPanel;

  Serial.println(F("═══════════════════════════════════"));
  Serial.printf("☀ Panel: %.3f V  %.3f A  (%.3f W)\n", vPanel, iPanel, potencia);
  Serial.printf("🔋 Batería: %.3f V  %.3f A\n", vBateria, iBateria);
  Serial.printf("📦 Carga: %.3f V  %.3f A\n", vCarga, iCarga);
  Serial.println(F("═══════════════════════════════════\n"));

  // Comandos por serial
  if (Serial.available()) {
    String cmd = Serial.readString();
    cmd.trim();
    if (cmd.equalsIgnoreCase("C")) calibrarSistema();
#if FASE_ACTUAL >= 2
    else if (cmd.startsWith("V")) procesarComandoVref(cmd);
#endif
  }

  delay(2000);
}
