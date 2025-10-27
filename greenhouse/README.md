# Sistema de Monitoreo de Invernadero

## Descripción
Sistema de monitoreo inteligente para invernaderos utilizando ESP8266/ESP32 y múltiples sensores ambientales para optimizar el crecimiento de plantas.

## Hardware Requerido

### Componentes Principales
- **ESP8266 / ESP32 / Arduino** - Microcontrolador
- **Fuente de alimentación** - 5V DC
- **Relés** - Para control de actuadores (opcional)

### Sensores Ambientales
- **DHT11 / DHT22** - Temperatura y humedad del aire
  - Rango temperatura: 0-50°C
  - Rango humedad: 20-90% RH
- **BH1750FVI** - Sensor de luz ambiente I2C
  - Rango: 1-65535 lux
  - Resolución: 1 lux
- **Sensor de humedad de suelo** - Capacitivo o resistivo
- **Sensor de pH** (opcional) - Para monitoreo de suelo

### Actuadores (opcionales)
- **Bomba de agua** - Sistema de riego automático
- **Ventiladores** - Control de temperatura
- **Iluminación LED** - Suplemento de luz
- **Electroválvulas** - Control de riego por zonas

## Parámetros Monitoreados

### Condiciones Ambientales
- 🌡️ **Temperatura del aire** - Rango óptimo para cada cultivo
- 💧 **Humedad relativa** - Control de ambiente
- ☀️ **Intensidad lumínica** - Fotosíntesis y crecimiento
- 🌱 **Humedad del suelo** - Necesidades de riego

### Datos Calculados
- 📊 **VPD (Déficit de Presión de Vapor)** - Optimización de crecimiento
- 📈 **DLI (Integral de luz diaria)** - Acumulación de luz
- ⏰ **Horas de luz** - Fotoperiodo

## Arquitectura del Sistema

```
┌─────────────────────────────────────┐
│          ESP8266/ESP32              │
│  ┌──────────────────────────────┐   │
│  │   Sensores Ambientales       │   │
│  │  - DHT11 (Temp/Humedad)      │   │
│  │  - BH1750 (Luz)              │   │
│  │  - Humedad de suelo          │   │
│  └──────────────────────────────┘   │
│               │                      │
│               ▼                      │
│  ┌──────────────────────────────┐   │
│  │   Procesamiento de Datos     │   │
│  │  - Promedios                 │   │
│  │  - Cálculo VPD               │   │
│  │  - Alertas                   │   │
│  └──────────────────────────────┘   │
│               │                      │
│               ▼                      │
│  ┌──────────────────────────────┐   │
│  │   Control de Actuadores      │   │
│  │  - Riego automático          │   │
│  │  - Ventilación               │   │
│  │  - Iluminación               │   │
│  └──────────────────────────────┘   │
└─────────────────────────────────────┘
           │
           ▼
  ┌─────────────────┐
  │  Almacenamiento │
  │  - Servidor web │
  │  - Tarjeta SD   │
  │  - MQTT/Cloud   │
  └─────────────────┘
```

## Conexiones

### DHT11/DHT22
| Pin DHT | Conexión ESP8266 | Descripción |
|---------|------------------|-------------|
| VCC | 3.3V | Alimentación |
| DATA | D7 (GPIO13) | Datos |
| GND | GND | Tierra |

### BH1750 (I2C)
| Pin BH1750 | Conexión ESP8266 | Descripción |
|------------|------------------|-------------|
| VCC | 3.3V | Alimentación |
| GND | GND | Tierra |
| SDA | D2 (GPIO4) | Datos I2C |
| SCL | D1 (GPIO5) | Clock I2C |
| ADDR | GND | Dirección 0x23 |

### Sensor Humedad de Suelo (Analógico)
| Pin Sensor | Conexión ESP8266 | Descripción |
|------------|------------------|-------------|
| VCC | 3.3V | Alimentación |
| GND | GND | Tierra |
| A0 | A0 | Lectura analógica |

## Estructura de Archivos

```
greenhouse/
├── README.md              # Este archivo
├── greenhouse_monitor.ino # Sketch principal (a crear)
├── sensors/               # Código de sensores
│   ├── 02_DHT11ino.ino    # Ejemplo DHT11
│   └── 03_BH1750FVI.ino   # Ejemplo BH1750
├── examples/              # Ejemplos adicionales
│   ├── auto_irrigation.ino
│   ├── vpd_calculator.ino
│   └── light_control.ino
└── docs/                  # Documentación adicional
    ├── cultivos.md        # Guía de parámetros por cultivo
    └── automatizacion.md  # Guía de automatización
```

## Librerías Requeridas

```cpp
#include <DHT.h>           // Sensor temperatura/humedad
#include <BH1750.h>        // Sensor de luz
#include <Wire.h>          // I2C (incluida)
```

### Instalación
```bash
arduino-cli lib install "DHT sensor library"
arduino-cli lib install "Adafruit Unified Sensor"
arduino-cli lib install "BH1750"
```

## Valores de Referencia por Cultivo

### Tomates
- Temperatura día: 21-27°C
- Temperatura noche: 16-18°C
- Humedad relativa: 60-80%
- DLI: 20-30 mol/m²/día

### Lechugas
- Temperatura día: 18-22°C
- Temperatura noche: 12-16°C
- Humedad relativa: 50-70%
- DLI: 12-17 mol/m²/día

### Pimientos
- Temperatura día: 24-28°C
- Temperatura noche: 18-20°C
- Humedad relativa: 60-70%
- DLI: 20-30 mol/m²/día

### Fresas
- Temperatura día: 18-25°C
- Temperatura noche: 10-15°C
- Humedad relativa: 60-75%
- DLI: 15-25 mol/m²/día

## Automatización

### Sistema de Riego Automático
```cpp
if (soilMoisture < THRESHOLD_LOW) {
    activateWaterPump();
} else if (soilMoisture > THRESHOLD_HIGH) {
    deactivateWaterPump();
}
```

### Control de Ventilación
```cpp
if (temperature > TEMP_MAX || humidity > HUMIDITY_MAX) {
    activateFans();
} else if (temperature < TEMP_MIN) {
    deactivateFans();
}
```

### Control de Iluminación Suplementaria
```cpp
if (lightIntensity < LUX_THRESHOLD && isDayTime()) {
    activateGrowLights();
} else {
    deactivateGrowLights();
}
```

## Almacenamiento de Datos

### Servidor Local
- Datos guardados en tarjeta SD
- Servidor web con dashboard
- API REST para consultas

### Cloud (opcional)
- MQTT a broker IoT
- ThingSpeak / Blynk
- Grafana para visualización

## Cálculo de VPD (Déficit de Presión de Vapor)

El VPD es un indicador crucial para el crecimiento óptimo:

```cpp
float calculateVPD(float temp, float humidity) {
    float svp = 610.7 * pow(10, (7.5 * temp) / (237.3 + temp)) / 1000;
    float vpd = svp * (1 - humidity / 100.0);
    return vpd;
}
```

**Rangos óptimos de VPD:**
- Plántulas: 0.4-0.8 kPa
- Crecimiento vegetativo: 0.8-1.2 kPa
- Floración/Fructificación: 1.0-1.5 kPa

## Alertas y Notificaciones

El sistema puede enviar alertas por:
- **WiFi/MQTT** - Mensajes a servidor
- **LED/Buzzer** - Indicadores locales
- **Email/SMS** - Vía servicios cloud (opcional)

### Condiciones de Alerta
- ⚠️ Temperatura fuera de rango
- ⚠️ Humedad crítica
- ⚠️ Luz insuficiente
- ⚠️ Suelo muy seco

## Compilación y Carga

### Arduino IDE
1. Abrir sketch principal
2. Seleccionar placa: ESP8266/ESP32/Arduino
3. Instalar librerías requeridas
4. Compilar y cargar

### arduino-cli
```bash
arduino-cli compile --fqbn esp8266:esp8266:generic greenhouse/
arduino-cli upload -p /dev/ttyUSB0 --fqbn esp8266:esp8266:generic greenhouse/
```

## Ejemplos de Uso

### Monitoreo Básico
Monitor continuo de temperatura, humedad y luz con registro en tarjeta SD.

### Riego Automático
Sistema que riega automáticamente según humedad del suelo y hora del día.

### Control de Clima
Sistema completo con control de temperatura, humedad y luz.

## Expansiones Futuras

- 📷 Cámara para time-lapse de crecimiento
- 🌡️ Sensores de temperatura de suelo
- 💨 Sensor de CO₂
- 🔋 Sistema solar para alimentación
- 📱 App móvil para control remoto

## Soporte para Múltiples Plataformas

- ESP8266 ✅
- ESP32 ✅
- Arduino Uno/Mega ✅
- Raspberry Pi 🔄

## Troubleshooting

### DHT11 no lee datos
- Verificar conexiones VCC, GND, DATA
- Añadir resistor pull-up de 10KΩ entre DATA y VCC
- Comprobar pin correcto en código

### BH1750 no detectado
- Verificar dirección I2C (0x23 o 0x5C)
- Comprobar conexiones SDA, SCL
- Usar `i2cdetect` para escanear bus

### Lecturas erráticas
- Mejorar fuente de alimentación
- Añadir capacitores de desacople
- Alejar sensores de fuentes de ruido

## Referencias

- [Guía de cultivos](docs/cultivos.md)
- [Documentación DHT](https://www.adafruit.com/product/385)
- [Datasheet BH1750](../datasheet/)
- [VPD y crecimiento de plantas](https://en.wikipedia.org/wiki/Vapour-pressure_deficit)

---

**Última actualización**: 2025-10-27
