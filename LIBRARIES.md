# Ornidia - Dependencias de Librerías

Este archivo lista todas las librerías Arduino requeridas para compilar los sketches del proyecto Ornidia, organizado por sistema de monitoreo.

---

## 📦 Instalación Rápida

### Método 1: Arduino IDE (Recomendado)

1. Abrir Arduino IDE
2. Menú: Tools → Manage Libraries... (o Ctrl+Shift+I)
3. Buscar e instalar las librerías necesarias según el sistema

### Método 2: arduino-cli

```bash
# Actualizar índice de librerías
arduino-cli lib update-index

# Instalar todas las librerías comunes
arduino-cli lib install "ACS712"
arduino-cli lib install "DHT sensor library"
arduino-cli lib install "Adafruit Unified Sensor"
arduino-cli lib install "BH1750"
arduino-cli lib install "Adafruit BME280 Library"
```

---

## 🔆 Sistema de Monitoreo de Panel Solar

### Librerías Específicas
- **ACS712** - Sensor de corriente Hall Effect
  - Versión recomendada: 0.3.0 o superior
  - Repositorio: https://github.com/rkoptev/ACS712-arduino
  - Uso: Medición de corriente del panel, batería y carga

- **BH1750** - Sensor de luz digital I2C
  - Versión recomendada: 1.3.0 o superior
  - Repositorio: https://github.com/claws/BH1750
  - Uso: Medición de intensidad lumínica

### Librerías de Comunicación
- **Wire** - I2C (incluida con Arduino IDE)
- **SPI** - Comunicación SPI (incluida)
- **Ethernet** - Conectividad de red (incluida)
- **SD** - Tarjeta SD (incluida)

### Instalación
```bash
arduino-cli lib install "ACS712"
arduino-cli lib install "BH1750"
```

### Sketches
- `solar_panel/solar_monitor.ino` - Sketch principal
- `solar_panel/examples/01_ASC712_CD4052B.ino` - Ejemplo con multiplexor
- `solar_panel/examples/01_ASC712_CD4052B_KALMAN.ino` - Con filtro Kalman

---

## 🌱 Sistema de Monitoreo de Invernadero

### Librerías Específicas
- **DHT sensor library** - Sensor de temperatura y humedad
  - Versión recomendada: 1.4.0 o superior
  - Repositorio: https://github.com/adafruit/DHT-sensor-library
  - Uso: DHT11 o DHT22 para ambiente del invernadero
  - **Dependencia**: Adafruit Unified Sensor

- **BH1750** - Sensor de luz digital I2C
  - Versión recomendada: 1.3.0 o superior
  - Uso: Medición de PAR (luz fotosintética)

- **Adafruit Unified Sensor** - Librería base
  - Versión recomendada: 1.1.0 o superior
  - Requerida por: DHT sensor library

### Librerías de Comunicación
- **Wire** - I2C (incluida con Arduino IDE)

### Instalación
```bash
arduino-cli lib install "DHT sensor library"
arduino-cli lib install "Adafruit Unified Sensor"
arduino-cli lib install "BH1750"
```

### Sketches
- `greenhouse/greenhouse_monitor.ino` - Sistema completo de invernadero
- `greenhouse/sensors/02_DHT11ino.ino` - Ejemplo con DHT11
- `greenhouse/sensors/03_BH1750FVI.ino` - Ejemplo con sensor de luz

---

## 🌤️ Estación Meteorológica

### Librerías Específicas
- **BMP180I2C** - Sensor de presión y temperatura
  - Versión recomendada: Última disponible
  - Alternativa: "Adafruit BMP085 Unified" o "Adafruit BMP280 Library"
  - Uso: Presión atmosférica y temperatura

- **DHT sensor library** - Sensor de temperatura y humedad
  - Versión recomendada: 1.4.0 o superior
  - Uso: DHT22 para datos meteorológicos precisos
  - **Dependencia**: Adafruit Unified Sensor

- **BH1750** - Sensor de luz digital I2C
  - Uso: Medición de radiación solar (lux)

- **Adafruit BME280 Library** (Opcional)
  - Sensor todo-en-uno: temperatura, humedad y presión
  - Más preciso que BMP180 + DHT22 por separado

### Librerías de Comunicación
- **Wire** - I2C (incluida con Arduino IDE)

### Instalación
```bash
arduino-cli lib install "DHT sensor library"
arduino-cli lib install "Adafruit Unified Sensor"
arduino-cli lib install "BH1750"
arduino-cli lib install "Adafruit BME280 Library"
```

### Sketches
- `weather_station/weather_station.ino` - Estación completa
- `weather_station/sensors/04_BMP180.ino` - Ejemplo con BMP180

---

## 💨 Sistema de Medición de Calidad del Aire

### Librerías Específicas
- **PMS Library** - Sensores de partículas Plantower PMS5003/PMS7003
  - Versión recomendada: 1.1.0 o superior
  - Repositorio: https://github.com/fu-hsi/pms
  - Uso: Medición de PM1.0, PM2.5, PM10

- **MH-Z19** - Sensor de CO₂ NDIR MH-Z19B
  - Versión recomendada: 1.5.4 o superior
  - Repositorio: https://github.com/WifWaf/MH-Z19
  - Uso: Medición precisa de CO₂ (400-5000 ppm)

- **Adafruit CCS811 Library** - Sensor eCO₂ y TVOC
  - Versión recomendada: 1.1.0 o superior
  - Repositorio: https://github.com/adafruit/Adafruit_CCS811
  - Uso: eCO₂ y compuestos orgánicos volátiles
  - **Dependencia**: Adafruit Unified Sensor

- **Adafruit BME280 Library** - Sensor ambiental
  - Compartido con Estación Meteorológica
  - Uso: Temperatura, humedad, presión para calibración

### Librerías de Comunicación
- **Wire** - I2C (incluida con Arduino IDE)
- **SoftwareSerial** - UART para sensores (incluida)

### Instalación
```bash
arduino-cli lib install "PMS Library"
arduino-cli lib install "MH-Z19"
arduino-cli lib install "Adafruit CCS811 Library"
arduino-cli lib install "Adafruit BME280 Library"
arduino-cli lib install "Adafruit Unified Sensor"
```

### Sketches
- `air_quality/air_quality_monitor.ino` - Monitor completo
- `air_quality/sensors/pms5003_sensor.ino` - Ejemplo PMS5003
- `air_quality/sensors/mhz19_sensor.ino` - Ejemplo MH-Z19B
- `air_quality/sensors/ccs811_sensor.ino` - Ejemplo CCS811
- `air_quality/sensors/mq135_sensor.ino` - Ejemplo MQ-135 (analógico)

---

## 📊 Tabla de Compatibilidad de Librerías

| Librería | Panel Solar | Invernadero | Meteorológica | Calidad Aire | Versión Mínima |
|----------|-------------|-------------|---------------|--------------|----------------|
| ACS712 | ✅ | ❌ | ❌ | ❌ | 0.2.0 |
| DHT sensor library | ❌ | ✅ | ✅ | ❌ | 1.3.0 |
| Adafruit Unified Sensor | ❌ | ✅ | ✅ | ✅ | 1.1.0 |
| BH1750 | ✅ | ✅ | ✅ | ❌ | 1.1.0 |
| BMP180I2C | ❌ | ❌ | ✅ | ❌ | - |
| PMS Library | ❌ | ❌ | ❌ | ✅ | 1.1.0 |
| MH-Z19 | ❌ | ❌ | ❌ | ✅ | 1.5.4 |
| Adafruit CCS811 | ❌ | ❌ | ❌ | ✅ | 1.1.0 |
| Adafruit BME280 | ❌ | ❌ | ✅ | ✅* | 2.0.0 |
| Wire (I2C) | ✅ | ✅ | ✅ | ✅ | - |
| SoftwareSerial | ❌ | ❌ | ❌ | ✅ | - |
| SPI | ✅ | ❌ | ❌ | ❌ | - |
| Ethernet | ✅ | ❌** | ❌** | ❌** | 2.0.0 |
| SD | ✅ | ❌** | ❌** | ❌** | 1.2.0 |

*Opcional para calibración ambiental
**Opcional según implementación

---

## 🔧 Compatibilidad por Plataforma

### ESP8266
- Compatible con: Panel Solar, Invernadero, Estación Meteorológica
- Librerías soportadas: Todas excepto las específicas de calidad del aire
- Board Manager URL: `http://arduino.esp8266.com/stable/package_esp8266com_index.json`
- Instalación: Tools → Board → Boards Manager → buscar "ESP8266"
- Ventaja: WiFi integrado, ideal para IoT

### ESP32
- Compatible con: Invernadero, Estación Meteorológica
- Librerías soportadas: Todas las de ESP8266 + más memoria/procesamiento
- Board Manager URL: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
- Ventaja: Mayor potencia, Bluetooth, dual-core

### Arduino (Uno, Mega, Nano)
- Compatible con: Invernadero, Estación Meteorológica, Calidad del Aire
- Librerías soportadas: Todas
- Arduino Mega: Recomendado para proyectos con muchos sensores (más pines y memoria)
- Limitación: Sin WiFi integrado (usar módulo ESP8266/ESP32 externo o Ethernet)
- Ventaja: Estándar, económico, amplia compatibilidad

### Raspberry Pi
- Compatible con: Estación Meteorológica, Calidad del Aire
- Librerías: Usar equivalentes en Python (RPi.GPIO, smbus, pyserial)
- Ventaja: Procesamiento avanzado, Linux completo, múltiples interfaces
- Recomendado para: Servidor de datos, procesamiento ML, dashboard local

---

## ⚠️ Problemas Comunes y Soluciones

### Error: "ACS712.h: No such file or directory"
**Solución**: Instalar librería ACS712 desde el Library Manager

### Error: "DHT.h: No such file or directory"
**Solución**: Instalar "DHT sensor library" y "Adafruit Unified Sensor"

### Error: "Wire.h: No such file or directory"
**Solución**: Wire está incluida con Arduino IDE. Reinstalar Arduino IDE si falta.

### Conflictos de Versión
**Solución**: Usar las versiones recomendadas o actualizar todas las librerías a sus últimas versiones estables.

## Compatibilidad de Versiones

| Librería | Versión Mínima | Versión Probada | Notas |
|----------|---------------|-----------------|-------|
| ACS712 | 0.2.0 | 0.3.0 | Funciona con versiones antiguas |
| DHT sensor library | 1.3.0 | 1.4.4 | Requiere Unified Sensor |
| BH1750 | 1.1.0 | 1.3.0 | Estable |
| Wire | - | - | Incluida con IDE |
| SPI | - | - | Incluida con IDE |
| Ethernet | 2.0.0 | 2.0.2 | Para Arduino Mega |
**Solución**: Wire está incluida con Arduino IDE. Reinstalar Arduino IDE si falta.

### Error: BMP180 no responde
**Solución**: 
- Verificar dirección I2C (0x77 o 0x76)
- Comprobar conexiones SDA/SCL
- Probar con sketch i2c_scanner

### Conflictos de Versión
**Solución**: Usar las versiones recomendadas o actualizar todas las librerías a sus últimas versiones estables.

---

## 📥 Instalación por Sistema

### Para Sistema de Panel Solar
```bash
arduino-cli lib update-index
arduino-cli lib install "ACS712"
arduino-cli lib install "BH1750"
```

### Para Sistema de Invernadero
```bash
arduino-cli lib update-index
arduino-cli lib install "DHT sensor library"
arduino-cli lib install "Adafruit Unified Sensor"
arduino-cli lib install "BH1750"
```

### Para Estación Meteorológica
```bash
arduino-cli lib update-index
arduino-cli lib install "DHT sensor library"
arduino-cli lib install "Adafruit Unified Sensor"
arduino-cli lib install "BH1750"
arduino-cli lib install "Adafruit BME280 Library"
```

### Para Sistema de Calidad del Aire
```bash
arduino-cli lib update-index
arduino-cli lib install "PMS Library"
arduino-cli lib install "MH-Z19"
arduino-cli lib install "Adafruit CCS811 Library"
arduino-cli lib install "Adafruit BME280 Library"
arduino-cli lib install "Adafruit Unified Sensor"
```


### Para Todos los Sistemas (Completo)
```bash
arduino-cli lib update-index
arduino-cli lib install "ACS712"
arduino-cli lib install "DHT sensor library"
arduino-cli lib install "Adafruit Unified Sensor"
arduino-cli lib install "BH1750"
arduino-cli lib install "Adafruit BME280 Library"
arduino-cli lib install "PMS Library"
arduino-cli lib install "MH-Z19"
arduino-cli lib install "Adafruit CCS811 Library"
```

---

## 📊 Versiones de Librerías Recomendadas

| Librería | Versión Mínima | Versión Probada | Notas |
|----------|---------------|-----------------|-------|
| ACS712 | 0.2.0 | 0.3.0 | Funciona con versiones antiguas |
| DHT sensor library | 1.3.0 | 1.4.4 | Requiere Unified Sensor |
| BH1750 | 1.1.0 | 1.3.0 | Estable |
| Wire | - | - | Incluida con IDE |
| SPI | - | - | Incluida con IDE |
| Ethernet | 2.0.0 | 2.0.2 | Para Arduino Mega |
| SD | 1.2.0 | 1.2.4 | Para Arduino Mega |
| Adafruit BME280 | 2.0.0 | 2.2.2 | Opcional |

---

## 🔄 Actualización de Librerías

### Arduino IDE
1. Tools → Manage Libraries
2. Filtrar por "UPDATABLE"
3. Actualizar individualmente o todas a la vez

### arduino-cli
```bash
arduino-cli lib upgrade
```

---

## 📝 Notas de Compatibilidad

### Sensor DHT11 vs DHT22
- **DHT11**: ±2°C, ±5% RH, más económico
- **DHT22**: ±0.5°C, ±2% RH, más preciso
- Ambos usan la misma librería

### BMP180 vs BME280
- **BMP180**: Solo presión y temperatura
- **BME280**: Presión, temperatura y humedad (todo-en-uno)
- El BME280 es más moderno y preciso

### Librerías Alternativas
| Sensor | Librería Principal | Alternativa |
|--------|-------------------|-------------|
| BMP180 | BMP180I2C | Adafruit BMP085 Unified |
| BME280 | Adafruit BME280 | Adafruit BME280 I2C |
| DHT | DHT sensor library | SimpleDHT |

---

## 🔐 Licencias

Cada librería tiene su propia licencia. Consultar los repositorios individuales:
- **ACS712**: MIT License
- **DHT sensor library**: MIT License
- **BH1750**: MIT License
- **Adafruit libraries**: BSD License
- **Wire, SPI, SD, Ethernet**: LGPL

---

## 📚 Recursos Adicionales

- [Arduino Library Guide](https://www.arduino.cc/en/Guide/Libraries)
- [ESP8266 Arduino Core](https://github.com/esp8266/Arduino)
- [ESP32 Arduino Core](https://github.com/espressif/arduino-esp32)
- [arduino-cli Documentation](https://arduino.github.io/arduino-cli/)

---

**Última actualización**: 2025-10-27
**Versión**: 2.0 (reorganización por sistemas)

