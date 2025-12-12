# Sistema de Medición de Calidad del Aire

## Descripción
Sistema completo de monitoreo de calidad del aire basado en microcontroladores (Arduino, Raspberry Pi) para medición de partículas, gases contaminantes y parámetros ambientales que afectan la salud respiratoria.

## Hardware Requerido

### Componentes Principales
- **Arduino** (Uno, Mega, Nano) - Microcontrolador
- **Raspberry Pi** (opcional) - Para procesamiento avanzado y servidor
- **Fuente de alimentación** - 5V DC o USB
- **Carcasa ventilada** - Protección con flujo de aire

### Sensores de Calidad del Aire

#### Partículas (Material Particulado)
- **PMS5003 / PMS7003** - Sensor láser PM1.0, PM2.5, PM10
- **SDS011** - Sensor láser PM2.5, PM10
- **GP2Y1010AU0F** - Sensor óptico básico de polvo
- **PPD42NS** - Detector de partículas

#### Gases Contaminantes
- **MQ-135** - Calidad del aire (NH3, NOx, alcohol, benzeno, humo, CO2)
- **MQ-7** - Monóxido de carbono (CO)
- **MQ-2** - Gas LP, propano, metano, humo
- **MQ-9** - CO y gases combustibles
- **MH-Z19B** - CO₂ infrarrojo (NDIR) de alta precisión
- **CCS811** - eCO₂ y eTVOC (compuestos orgánicos volátiles)

#### Sensores Ambientales
- **DHT22** - Temperatura y humedad (±0.5°C, ±2% RH)
- **BME280** - Temp, humedad y presión barométrica
- **BMP180** - Presión y temperatura

#### Otros
- **BH1750** - Luz ambiente (para correlación solar)

## Parámetros Medidos

### Partículas (PM - Particulate Matter)
- 🔬 **PM1.0** - Partículas ≤ 1.0 μm (ultra finas)
- 🔬 **PM2.5** - Partículas ≤ 2.5 μm (respirables)
- 🔬 **PM10** - Partículas ≤ 10 μm (inhalables)

### Gases
- ☁️ **CO₂** - Dióxido de carbono (ppm)
- ⚠️ **CO** - Monóxido de carbono (ppm)
- 🏭 **VOC/TVOC** - Compuestos orgánicos volátiles totales (ppb)
- 💨 **NH₃** - Amoníaco
- 🚗 **NOx** - Óxidos de nitrógeno

### Ambientales
- 🌡️ **Temperatura** - °C
- 💧 **Humedad** - % RH
- 🏔️ **Presión** - hPa

### Índices Calculados
- 📊 **AQI** - Índice de Calidad del Aire (Air Quality Index)
- 🎯 **IAQ** - Índice de Calidad del Aire Interior (Indoor Air Quality)
- 🌡️ **Índice de confort** - Basado en temp/humedad

## Arquitectura del Sistema

```
┌──────────────────────────────────────────┐
│         Microcontrolador                 │
│          (Arduino/Raspberry Pi)          │
│                                          │
│  ┌────────────────────────────────────┐  │
│  │   Sensores de Partículas           │  │
│  │  - PMS5003 (UART) [PM1.0/2.5/10]  │  │
│  │  - SDS011 (UART) [PM2.5/10]       │  │
│  │  - GP2Y1010AU0F (Analógico)       │  │
│  └────────────────────────────────────┘  │
│                                          │
│  ┌────────────────────────────────────┐  │
│  │   Sensores de Gases (I2C)          │  │
│  │  - MH-Z19B (CO₂ NDIR) [UART]      │  │
│  │  - CCS811 (eCO₂/TVOC) [0x5A]      │  │
│  │  - BME280 (Temp/Hum/Pre) [0x76]   │  │
│  └────────────────────────────────────┘  │
│                                          │
│  ┌────────────────────────────────────┐  │
│  │   Sensores MQ (Analógicos)         │  │
│  │  - MQ-135 (Calidad aire)           │  │
│  │  - MQ-7 (CO)                       │  │
│  │  - MQ-2 (Gases combustibles)       │  │
│  └────────────────────────────────────┘  │
│                                          │
│  ┌────────────────────────────────────┐  │
│  │   Procesamiento                    │  │
│  │  - Cálculo de AQI                  │  │
│  │  - Promedios móviles               │  │
│  │  - Alertas por umbrales            │  │
│  └────────────────────────────────────┘  │
└──────────────────────────────────────────┘
              │
              ▼
   ┌──────────────────────┐
   │   Almacenamiento     │
   │  - Tarjeta SD        │
   │  - MQTT Broker       │
   │  - ThingSpeak        │
   │  - AirGradient       │
   │  - PurpleAir         │
   │  - Servidor local    │
   └──────────────────────┘
```

## Conexiones

### PMS5003 / PMS7003 (UART)
| Pin PMS | Conexión Arduino | Descripción |
|---------|------------------|-------------|
| VCC | 5V | Alimentación |
| GND | GND | Tierra |
| TX | RX (D10 SoftSerial) | Transmisión de datos |
| RX | TX (D11 SoftSerial) | Recepción (opcional) |
| SET | D12 | Sleep/Wake (opcional) |

### MH-Z19B (UART - CO₂)
| Pin MH-Z19B | Conexión Arduino | Descripción |
|-------------|------------------|-------------|
| VIN | 5V | Alimentación (4.5-5.5V) |
| GND | GND | Tierra |
| TX | RX (D2 SoftSerial) | Transmisión |
| RX | TX (D3 SoftSerial) | Recepción |

### CCS811 (I2C - eCO₂/TVOC)
| Pin CCS811 | Conexión Arduino | Descripción |
|------------|------------------|-------------|
| VCC | 3.3V | Alimentación |
| GND | GND | Tierra |
| SDA | A4 (SDA) | Datos I2C |
| SCL | A5 (SCL) | Clock I2C |
| WAK | GND | Wake (activo bajo) |

### BME280 (I2C)
| Pin BME280 | Conexión Arduino | Descripción |
|------------|------------------|-------------|
| VCC | 3.3V | Alimentación |
| GND | GND | Tierra |
| SDA | A4 (SDA) | Datos I2C |
| SCL | A5 (SCL) | Clock I2C |

### MQ-135 (Analógico)
| Pin MQ-135 | Conexión Arduino | Descripción |
|------------|------------------|-------------|
| VCC | 5V | Alimentación |
| GND | GND | Tierra |
| AOUT | A0 | Salida analógica |
| DOUT | D8 | Salida digital (opcional) |

### GP2Y1010AU0F (Sensor óptico de polvo)
| Pin GP2Y | Conexión Arduino | Descripción |
|----------|------------------|-------------|
| VCC | 5V | Alimentación |
| GND | GND | Tierra |
| LED | D7 | Control LED |
| VOUT | A1 | Salida analógica |

## Estructura de Archivos

```
air_quality/
├── README.md                    # Este archivo
├── air_quality_monitor.ino      # Sketch principal
├── sensors/                     # Código de sensores
│   ├── pms5003_sensor.ino       # Sensor de partículas láser
│   ├── mhz19_sensor.ino         # Sensor CO₂ NDIR
│   ├── ccs811_sensor.ino        # Sensor eCO₂/TVOC
│   ├── mq135_sensor.ino         # Sensor calidad aire
│   └── gp2y_sensor.ino          # Sensor óptico polvo
├── examples/                    # Ejemplos
│   ├── basic_monitor.ino        # Monitor básico PM + CO₂
│   ├── complete_monitor.ino     # Monitor completo
│   └── outdoor_monitor.ino      # Monitor exterior
└── docs/                        # Documentación
    ├── aqi_calculation.md       # Cálculo del AQI
    ├── calibration.md           # Calibración de sensores
    └── health_guidelines.md     # Guías de salud
```

## Librerías Requeridas

```cpp
#include <Wire.h>              // I2C (incluida)
#include <SoftwareSerial.h>    // UART para sensores (incluida)
#include <Adafruit_BME280.h>   // Sensor ambiental
#include <Adafruit_CCS811.h>   // Sensor eCO₂/TVOC
#include <PMS.h>               // Sensor PMS5003/7003
#include <MHZ19.h>             // Sensor MH-Z19B CO₂
```

### Instalación
```bash
arduino-cli lib install "Adafruit BME280 Library"
arduino-cli lib install "Adafruit CCS811 Library"
arduino-cli lib install "PMS Library"
arduino-cli lib install "MH-Z19"
```

## Cálculo del AQI (Air Quality Index)

### Escala AQI (EPA - USA)
| AQI | Nivel | Color | PM2.5 (μg/m³) | Descripción |
|-----|-------|-------|---------------|-------------|
| 0-50 | Bueno | Verde | 0-12.0 | Calidad del aire satisfactoria |
| 51-100 | Moderado | Amarillo | 12.1-35.4 | Aceptable, grupos sensibles con precaución |
| 101-150 | Dañino (GS) | Naranja | 35.5-55.4 | Grupos sensibles afectados |
| 151-200 | Dañino | Rojo | 55.5-150.4 | Todos pueden experimentar efectos |
| 201-300 | Muy dañino | Púrpura | 150.5-250.4 | Alerta de salud |
| 301+ | Peligroso | Granate | 250.5+ | Emergencia de salud |

### Fórmula de Cálculo
```cpp
int calculateAQI(float pm25) {
    // Breakpoints para PM2.5
    float cLow, cHigh;
    int iLow, iHigh;
    
    if (pm25 <= 12.0) {
        cLow = 0.0; cHigh = 12.0;
        iLow = 0; iHigh = 50;
    } else if (pm25 <= 35.4) {
        cLow = 12.1; cHigh = 35.4;
        iLow = 51; iHigh = 100;
    } 
    // ... más rangos
    
    // Fórmula AQI
    float aqi = ((iHigh - iLow) / (cHigh - cLow)) * (pm25 - cLow) + iLow;
    return round(aqi);
}
```

## Umbrales y Alertas

### PM2.5
- ✅ **0-12 μg/m³** - Bueno
- ⚠️ **12-35 μg/m³** - Moderado
- 🔶 **35-55 μg/m³** - Dañino para grupos sensibles
- 🔴 **>55 μg/m³** - Dañino para todos

### CO₂ (Interior)
- ✅ **<400 ppm** - Exterior (referencia)
- ✅ **400-1000 ppm** - Bueno
- ⚠️ **1000-2000 ppm** - Ventilación recomendada
- 🔶 **2000-5000 ppm** - Aire viciado, somnolencia
- 🔴 **>5000 ppm** - Peligroso, problemas de salud

### TVOC
- ✅ **0-220 ppb** - Excelente
- ⚠️ **220-660 ppb** - Bueno
- 🔶 **660-2200 ppb** - Moderado
- 🔴 **>2200 ppb** - Pobre

## Almacenamiento de Datos

### Formato de Datos
```json
{
    "timestamp": "2025-10-27T12:00:00Z",
    "pm1_0": 5.2,
    "pm2_5": 12.3,
    "pm10": 18.5,
    "co2": 450,
    "tvoc": 125,
    "temperature": 22.5,
    "humidity": 55.0,
    "pressure": 1013.25,
    "aqi": 48,
    "aqi_level": "Good"
}
```

### Destinos de Datos
1. **Tarjeta SD** - CSV o JSON
2. **MQTT** - Broker local o cloud
3. **ThingSpeak** - Plataforma IoT
4. **AirGradient** - Red de sensores de calidad del aire
5. **PurpleAir** - Red comunitaria de monitoreo
6. **Sensor.Community** - Proyecto open data
7. **Servidor propio** - Base de datos

## Intervalos de Lectura Recomendados

- **PM2.5/PM10**: 60 segundos (alta energía del sensor)
- **CO₂**: 5 segundos
- **TVOC/eCO₂**: 1-10 segundos
- **Temperatura/Humedad**: 30-60 segundos
- **Promedio móvil**: 5-15 minutos para reportes

## Instalación

### Interior
- ✅ Altura 1.0-1.5 metros (zona de respiración)
- ✅ Alejado de ventanas y puertas
- ✅ No cerca de cocinas o fuentes de contaminación
- ✅ Circulación de aire natural
- ❌ No en luz solar directa
- ❌ No cerca de ventiladores o AC

### Exterior
- ✅ Carcasa protegida con ventilación
- ✅ Protección contra lluvia
- ✅ Altura 2-3 metros
- ✅ Alejado de tráfico directo (>3m de vía)
- ✅ No obstruido por edificios

## Calibración de Sensores

### Sensores de Partículas (PMS5003)
- No requieren calibración de fábrica
- Limpieza periódica (cada 6-12 meses)
- Comparar con estación oficial cercana

### MH-Z19B (CO₂)
- Auto-calibración ABC habilitada (400 ppm baseline)
- Calibración manual: exponer a aire exterior limpio
- Comando: `sensor.calibrateZero()`

### CCS811 (TVOC/eCO₂)
- Requiere 48 horas de burn-in
- Calibración automática con lecturas BME280
- Mejorar precisión con temperatura/humedad real

### Sensores MQ
- Pre-calentamiento: 24-48 horas primera vez
- Calibración en aire limpio
- Factor de corrección por temperatura/humedad

## Mantenimiento

### Mensual
- Verificar lecturas vs. datos oficiales
- Limpiar carcasa externa

### Trimestral
- Revisar conexiones
- Actualizar firmware si aplica

### Semestral
- Limpiar sensor de partículas (aire comprimido)
- Verificar calibración de CO₂

### Anual
- Reemplazo de sensores MQ si degradación
- Limpieza profunda de todos los sensores

## Ejemplos de Uso

### Monitor Básico
PM2.5 + CO₂ + Temperatura/Humedad. Ideal para interior.

### Monitor Completo
Todos los sensores con cálculo de AQI y envío a cloud.

### Monitor Exterior
PM2.5 + PM10 para monitoreo ambiental urbano.

## Integración con Servicios

### Sensor.Community (ex-Luftdaten)
```cpp
// Enviar datos a Sensor.Community
String url = "https://api.sensor.community/v1/push-sensor-data/";
// JSON con mediciones
```

### ThingSpeak
```cpp
ThingSpeak.setField(1, pm25);
ThingSpeak.setField(2, pm10);
ThingSpeak.setField(3, co2);
ThingSpeak.writeFields(channelID, apiKey);
```

### AirGradient
Compatible con protocolo AirGradient para visualización en dashboard.

## Visualización de Datos

- **Dashboard web local** - HTML/CSS/JavaScript
- **Grafana** - Visualización avanzada
- **Node-RED** - Flujos y automatización
- **Home Assistant** - Integración domótica
- **Display OLED local** - SSD1306 para lecturas en tiempo real

## Expansiones Futuras

- 📱 App móvil con alertas push
- 🌐 Red de sensores comunitarios
- 🔋 Versión con batería y solar
- 📡 Transmisión LoRa/LoRaWAN
- 🤖 Machine Learning para predicciones
- 🌡️ Integración con HVAC para control automático

## Soporte para Plataformas

- Arduino (Uno, Mega, Nano) ✅
- Raspberry Pi ✅
- ESP32 (futuro) 🔄
- ESP8266 (futuro) 🔄

## Consideraciones de Salud

### Efectos de PM2.5
- Irritación respiratoria
- Agravamiento de asma
- Enfermedades cardiovasculares (exposición prolongada)
- Reducción de la función pulmonar

### Grupos Sensibles
- Niños
- Adultos mayores
- Personas con asma o EPOC
- Enfermedades cardiovasculares

### Recomendaciones por AQI
- **0-50**: Sin restricciones
- **51-100**: Sensibles: reducir actividad exterior prolongada
- **101-150**: Sensibles: evitar actividad exterior prolongada
- **151-200**: Todos: reducir actividad exterior
- **201+**: Todos: permanecer en interior

## Troubleshooting

### PMS5003 no responde
- Verificar conexión UART (TX/RX cruzados)
- Alimentación 5V estable (>1A)
- Comprobar baudrate (9600)

### MH-Z19B lecturas incorrectas
- Esperar warm-up (3 minutos)
- Verificar calibración ABC
- No obstruir entrada/salida de aire

### CCS811 error de lectura
- Completar burn-in de 48 horas
- Verificar dirección I2C (0x5A o 0x5B)
- Proporcionar datos de temp/humedad

### Sensores MQ lecturas erráticas
- Pre-calentamiento insuficiente (24-48h)
- Calibración en aire limpio necesaria
- Verificar alimentación estable 5V

## Referencias

- [EPA AQI](https://www.airnow.gov/aqi/)
- [WHO Air Quality Guidelines](https://www.who.int/news-room/fact-sheets/detail/ambient-(outdoor)-air-quality-and-health)
- [Sensor.Community](https://sensor.community/)
- [AirGradient](https://www.airgradient.com/)
- [PurpleAir](https://www2.purpleair.com/)

## Normativas

- **EPA (USA)**: National Ambient Air Quality Standards
- **WHO**: Air Quality Guidelines
- **EU**: Air Quality Directive 2008/50/EC
- **China**: GB 3095-2012

---

**Última actualización**: 2025-10-27
**Plataformas**: Arduino, Raspberry Pi
