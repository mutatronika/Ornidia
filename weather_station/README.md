# Sistema de Estación Meteorológica

## Descripción
Estación meteorológica completa basada en microcontroladores (ESP8266, ESP32, Arduino, Raspberry Pi) para medición y registro de parámetros atmosféricos.

## Hardware Requerido

### Componentes Principales
- **ESP8266 / ESP32 / Arduino Mega** - Microcontrolador
- **Raspberry Pi** (opcional) - Para procesamiento avanzado y servidor
- **Fuente de alimentación** - 5V DC o panel solar
- **Carcasa impermeable** - Protección para exteriores

### Sensores Meteorológicos

#### Temperatura y Humedad
- **DHT11** - Básico (±2°C, ±5% RH)
- **DHT22** - Mejorado (±0.5°C, ±2% RH)
- **BME280** - Profesional con presión (I2C)

#### Presión Atmosférica
- **BMP180** - Barómetro I2C (300-1100 hPa)
- **BMP280** - Mejorado
- **BME280** - Con temp/humedad integrado

#### Luz y Radiación Solar
- **BH1750FVI** - Luxómetro digital I2C
- **Sensor UV** - Radiación ultravioleta
- **Piranómetro** (opcional) - Radiación solar total

#### Viento
- **Anemómetro** - Velocidad del viento
- **Veleta** - Dirección del viento

#### Precipitación
- **Pluviómetro** - Balancín (rain gauge)

#### Otros (opcionales)
- **Sensor de calidad del aire** - PM2.5, PM10
- **Sensor de CO₂** - MH-Z19
- **Detector de rayos** - AS3935

## Parámetros Medidos

### Básicos
- 🌡️ **Temperatura** - Aire ambiente (°C)
- 💧 **Humedad relativa** - % RH
- 🏔️ **Presión atmosférica** - hPa o mbar
- ☀️ **Intensidad lumínica** - Lux

### Avanzados
- 💨 **Velocidad del viento** - km/h o m/s
- 🧭 **Dirección del viento** - Grados (0-360°)
- 🌧️ **Precipitación** - mm/hora
- 🌤️ **Índice UV** - 0-11+
- 📊 **Altitud** - metros (calculada)
- 🌡️ **Punto de rocío** - °C (calculado)
- 💨 **Sensación térmica** - °C (calculada)

### Datos Calculados
- **Tendencia de presión** - Predicción meteorológica
- **Heat Index** - Índice de calor
- **Wind Chill** - Sensación térmica por viento
- **Evapotranspiración** - ET₀

## Arquitectura del Sistema

```
┌──────────────────────────────────────────┐
│         Microcontrolador                 │
│        (ESP8266/ESP32/Arduino)           │
│                                          │
│  ┌────────────────────────────────────┐  │
│  │   Sensores I2C (Bus compartido)    │  │
│  │  - BMP180 (Presión/Temp) [0x77]   │  │
│  │  - BH1750 (Luz) [0x23]            │  │
│  │  - BME280 (opcional) [0x76]       │  │
│  └────────────────────────────────────┘  │
│                                          │
│  ┌────────────────────────────────────┐  │
│  │   Sensores Digitales               │  │
│  │  - DHT11/DHT22 (Temp/Humedad)     │  │
│  └────────────────────────────────────┘  │
│                                          │
│  ┌────────────────────────────────────┐  │
│  │   Sensores Analógicos              │  │
│  │  - Anemómetro (velocidad viento)  │  │
│  │  - Veleta (dirección viento)      │  │
│  │  - Pluviómetro (interrupciones)   │  │
│  │  - Sensor UV                       │  │
│  └────────────────────────────────────┘  │
│                                          │
│  ┌────────────────────────────────────┐  │
│  │   Procesamiento                    │  │
│  │  - Promedios                       │  │
│  │  - Cálculos derivados              │  │
│  │  - Predicción                      │  │
│  └────────────────────────────────────┘  │
└──────────────────────────────────────────┘
              │
              ▼
   ┌──────────────────────┐
   │   Almacenamiento     │
   │  - Tarjeta SD        │
   │  - MQTT Broker       │
   │  - ThingSpeak        │
   │  - WeatherUnderground│
   │  - Servidor local    │
   └──────────────────────┘
```

## Conexiones

### BMP180 / BMP280 (I2C)
| Pin BMP180 | Conexión ESP8266 | Descripción |
|------------|------------------|-------------|
| VCC | 3.3V | Alimentación |
| GND | GND | Tierra |
| SDA | D2 (GPIO4) | Datos I2C |
| SCL | D1 (GPIO5) | Clock I2C |

### DHT11 / DHT22
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

### Anemómetro (Analógico/Digital)
| Pin | Conexión | Descripción |
|-----|----------|-------------|
| VCC | 5V | Alimentación |
| GND | GND | Tierra |
| OUT | A0 o D3 | Señal (analógica o pulsos) |

### Pluviómetro (Interrupciones)
| Pin | Conexión | Descripción |
|-----|----------|-------------|
| VCC | 5V | Alimentación |
| GND | GND | Tierra |
| OUT | D5 | Interrupción (0.2794mm/tip típico) |

## Estructura de Archivos

```
weather_station/
├── README.md                  # Este archivo
├── weather_station.ino        # Sketch principal (a crear)
├── sensors/                   # Código de sensores
│   ├── 04_BMP180.ino          # Ejemplo BMP180
│   ├── dht_sensor.ino         # Lectura DHT
│   ├── wind_sensor.ino        # Anemómetro
│   └── rain_gauge.ino         # Pluviómetro
├── examples/                  # Ejemplos
│   ├── basic_station.ino      # Estación básica
│   ├── advanced_station.ino   # Con cálculos avanzados
│   └── solar_powered.ino      # Alimentación solar
└── docs/                      # Documentación
    ├── calibracion.md         # Calibración de sensores
    ├── instalacion.md         # Instalación exterior
    └── prediccion.md          # Algoritmos de predicción
```

## Librerías Requeridas

```cpp
#include <Wire.h>              // I2C (incluida)
#include <DHT.h>               // Sensor DHT11/DHT22
#include <BMP180I2C.h>         // Barómetro BMP180
#include <BH1750.h>            // Luxómetro
#include <Adafruit_BME280.h>   // BME280 (opcional)
```

### Instalación
```bash
arduino-cli lib install "DHT sensor library"
arduino-cli lib install "Adafruit Unified Sensor"
arduino-cli lib install "BH1750"
arduino-cli lib install "Adafruit BME280 Library"
```

## Mediciones y Cálculos

### Presión al Nivel del Mar
```cpp
float seaLevelPressure = pressure / pow(1.0 - (altitude / 44330.0), 5.255);
```

### Punto de Rocío
```cpp
float dewPoint(float temp, float humidity) {
    float a = 17.27;
    float b = 237.7;
    float alpha = ((a * temp) / (b + temp)) + log(humidity / 100.0);
    return (b * alpha) / (a - alpha);
}
```

### Heat Index (Índice de Calor)
```cpp
float heatIndex(float temp, float humidity) {
    float hi = 0.5 * (temp + 61.0 + ((temp - 68.0) * 1.2) + (humidity * 0.094));
    if (hi > 80) {
        // Fórmula completa de Rothfusz
        float c1 = -42.379;
        float c2 = 2.04901523;
        // ... (implementación completa)
    }
    return hi;
}
```

### Wind Chill (Sensación Térmica)
```cpp
float windChill(float temp, float windSpeed) {
    if (temp <= 10 && windSpeed > 4.8) {
        return 13.12 + 0.6215 * temp - 11.37 * pow(windSpeed, 0.16) + 
               0.3965 * temp * pow(windSpeed, 0.16);
    }
    return temp;
}
```

## Predicción Meteorológica Simple

Basada en tendencia de presión barométrica:

```cpp
String forecastFromPressureTrend(float trend) {
    if (trend > 0.5) return "Mejorando";
    if (trend < -0.5) return "Empeorando";
    return "Estable";
}
```

## Almacenamiento de Datos

### Formato de Datos
```json
{
    "timestamp": "2025-10-27T12:00:00Z",
    "temperature": 22.5,
    "humidity": 65.0,
    "pressure": 1013.25,
    "light": 45000,
    "windSpeed": 15.3,
    "windDirection": 270,
    "rainfall": 0.5,
    "dewPoint": 15.8,
    "heatIndex": 23.1
}
```

### Destinos de Datos
1. **Tarjeta SD** - CSV o JSON
2. **MQTT** - Broker local o cloud
3. **ThingSpeak** - Plataforma IoT
4. **Weather Underground** - Red global
5. **Servidor propio** - Base de datos MySQL/PostgreSQL

## Intervalos de Lectura Recomendados

- **Temperatura/Humedad**: 10-60 segundos
- **Presión**: 1-5 minutos
- **Viento**: 1-10 segundos (promedio 2 minutos)
- **Lluvia**: Contador continuo (acumulado horario/diario)
- **Luz**: 1-5 minutos

## Instalación Exterior

### Ubicación
- ✅ Área abierta sin obstrucciones
- ✅ 1.5-2 metros sobre el suelo
- ✅ Alejado de fuentes de calor
- ✅ Protección contra lluvia directa (sensores no impermeables)

### Carcasa
- Radiación protegida (blanca)
- Ventilación adecuada
- Protección IP65 mínimo
- Montaje estable

### Alimentación
- Panel solar + batería (recomendado)
- Cable de alimentación (si es cercano)
- Batería de respaldo

## Calibración de Sensores

### Temperatura
Comparar con termómetro calibrado, ajustar offset.

### Presión
Comparar con estación meteorológica oficial cercana.

### Anemómetro
Usar túnel de viento o comparar con estación oficial.

### Pluviómetro
Calibrar con volumen conocido de agua.

## Ejemplos de Uso

### Estación Básica
Temperatura, humedad, presión y luz. Ideal para principiantes.

### Estación Completa
Todos los sensores con envío a cloud y predicción local.

### Estación Solar
Sistema autónomo con panel solar y batería, ideal para ubicaciones remotas.

## Integración con Servicios

### Weather Underground
```cpp
// Enviar datos a WU
String url = "http://weatherstation.wunderground.com/weatherstation/updateweatherstation.php";
url += "?ID=" + STATION_ID;
url += "&PASSWORD=" + STATION_KEY;
url += "&dateutc=now";
url += "&tempf=" + String(tempF);
// ... más parámetros
```

### ThingSpeak
```cpp
ThingSpeak.setField(1, temperature);
ThingSpeak.setField(2, humidity);
ThingSpeak.setField(3, pressure);
ThingSpeak.writeFields(channelID, apiKey);
```

## Visualización de Datos

- **Dashboard web local** - HTML/CSS/JavaScript
- **Grafana** - Visualización avanzada
- **Node-RED** - Flujos y automatización
- **Home Assistant** - Integración domótica

## Expansiones Futuras

- 🌐 Integración con múltiples estaciones
- 📡 Transmisión LoRa/LoRaWAN
- 🔋 Sistema de energía híbrido
- 📷 Cámara para sky conditions
- ⚡ Detector de tormentas eléctricas
- 🌈 Sensor espectral completo

## Soporte para Múltiples Plataformas

- ESP8266 ✅
- ESP32 ✅
- Arduino Mega ✅
- Raspberry Pi ✅
- PIC (futuro)

## Troubleshooting

### BMP180 no responde
- Verificar dirección I2C (0x77)
- Comprobar conexiones SDA/SCL
- Probar con i2c_scanner

### Lecturas de viento erráticas
- Verificar calibración
- Mejorar filtrado de señal
- Comprobar instalación mecánica

### Datos de lluvia incorrectos
- Verificar factor de calibración (mm/tip)
- Limpiar mecanismo de balancín
- Revisar debounce en código

### Presión inexacta
- Calibrar con estación oficial
- Verificar altitud configurada
- Comprobar temperatura del sensor

## Referencias

- [WMO Guidelines](https://public.wmo.int/)
- [Weather Underground](https://www.wunderground.com/)
- [Datasheet BMP180](../datasheet/)
- [Datasheet DHT22](https://www.sparkfun.com/datasheets/Sensors/Temperature/DHT22.pdf)

---

**Última actualización**: 2025-10-27
