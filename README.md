# Ornidia
Proyecto de Sistemas de Monitoreo con Microcontroladores

## 🎯 Descripción

Ornidia es un proyecto integral de monitoreo y control basado en microcontroladores (PIC, Arduino, ESP32, ESP8266, Raspberry Pi, etc.) dividido en tres sistemas principales:

1. **🔆 Monitoreo de Panel Solar** - Sistema de monitoreo de energía solar
2. **🌱 Monitoreo de Invernadero** - Sistema de control ambiental para cultivos
3. **🌤️ Estación Meteorológica** - Sistema de medición de parámetros atmosféricos

Cada sistema utiliza diversos sensores y actuadores para obtener datos y almacenarlos en servidores caseros y/o cloud.

---

## 📂 Estructura del Proyecto

```
Ornidia/
├── solar_panel/          # Sistema de monitoreo de panel solar
│   ├── README.md         # Documentación del sistema solar
│   ├── solar_monitor.ino # Sketch principal
│   ├── data/             # Interfaz web
│   ├── examples/         # Ejemplos y pruebas
│   └── sensors/          # Código de sensores específicos
│
├── greenhouse/           # Sistema de monitoreo de invernadero
│   ├── README.md         # Documentación del invernadero
│   ├── sensors/          # Sensores (DHT, BH1750, humedad suelo)
│   ├── examples/         # Ejemplos de automatización
│   └── docs/             # Guías de cultivos y automatización
│
├── weather_station/      # Estación meteorológica
│   ├── README.md         # Documentación de la estación
│   ├── sensors/          # Sensores (BMP180, anemómetro, etc.)
│   ├── examples/         # Ejemplos de configuración
│   └── docs/             # Guías de instalación y calibración
│
├── LIBRARIES.md          # Dependencias de librerías
├── TESTING.md            # Guía de pruebas
├── QUICKSTART.md         # Inicio rápido
└── verify_integrity.sh   # Script de verificación
```

---

## 🔆 Sistema de Monitoreo de Panel Solar

Sistema completo de monitoreo para instalaciones de energía solar con ESP8266.

### Hardware
- Panel Solar Hybrytec (3W, 12V)
- Controlador PWM (12V/24V, 20A)
- Batería MTEK (12V, 2.7Ah)
- ESP8266 con WiFi
- Sensores: ACS712 (×3), CD4052B, BH1750FVI

### Parámetros Monitoreados
- ✅ Corriente del panel, batería y carga
- ✅ Voltaje del panel
- ✅ Potencia generada
- ✅ Luz ambiente

**[Ver documentación completa →](solar_panel/README.md)**

---

## 🌱 Sistema de Monitoreo de Invernadero

Sistema de control ambiental para optimizar el crecimiento de plantas.

### Hardware
- ESP8266 / ESP32 / Arduino
- Sensores: DHT11/DHT22, BH1750, humedad de suelo
- Actuadores: relés, bombas, ventiladores, luces LED

### Parámetros Monitoreados
- 🌡️ Temperatura y humedad del aire
- ☀️ Intensidad lumínica
- 🌱 Humedad del suelo
- 📊 VPD (Déficit de Presión de Vapor)

### Automatización
- Riego automático
- Control de ventilación
- Iluminación suplementaria
- Alertas por condiciones críticas

**[Ver documentación completa →](greenhouse/README.md)**

---

## 🌤️ Estación Meteorológica

Sistema completo de medición de parámetros atmosféricos.

### Hardware
- ESP8266 / ESP32 / Arduino Mega / Raspberry Pi
- Sensores: BMP180, DHT22, BH1750, anemómetro, pluviómetro
- Carcasa impermeable para exteriores

### Parámetros Medidos
- 🌡️ Temperatura y humedad
- 🏔️ Presión atmosférica
- 💨 Velocidad y dirección del viento
- 🌧️ Precipitación
- ☀️ Intensidad lumínica e índice UV

### Datos Calculados
- Punto de rocío
- Heat index y wind chill
- Tendencia de presión
- Predicción meteorológica básica

**[Ver documentación completa →](weather_station/README.md)**

---

## 🔧 Plataformas Soportadas

El proyecto es compatible con múltiples microcontroladores:

- ✅ **ESP8266** - WiFi integrado, ideal para IoT
- ✅ **ESP32** - Mayor potencia y conectividad
- ✅ **Arduino** (Uno, Mega, Nano) - Plataforma estándar
- 🔄 **Raspberry Pi** - Para procesamiento avanzado
- 🔄 **PIC** - Microcontroladores Microchip (en desarrollo)

---

## 💾 Almacenamiento de Datos

Todos los sistemas soportan múltiples opciones de almacenamiento:

### Local
- 📂 **Tarjeta SD** - Almacenamiento persistente
- 🌐 **Servidor web local** - Dashboard en tiempo real
- 💻 **Servidor casero** - MySQL, PostgreSQL, InfluxDB

### Cloud
- ☁️ **MQTT Broker** - Mosquitto, HiveMQ
- 📊 **ThingSpeak** - Plataforma IoT de MathWorks
- 🌐 **Blynk** - App móvil y dashboard
- 📈 **Grafana** - Visualización avanzada
- 🏠 **Home Assistant** - Integración domótica

---

## 📚 Librerías Requeridas

### Sensores Principales
```bash
arduino-cli lib install "ACS712"
arduino-cli lib install "DHT sensor library"
arduino-cli lib install "Adafruit Unified Sensor"
arduino-cli lib install "BH1750"
arduino-cli lib install "Adafruit BME280 Library"
```

### Comunicación
- Wire (I2C) - Incluida
- SPI - Incluida
- Ethernet - Incluida
- WiFi (ESP8266/ESP32) - Incluida

**[Ver lista completa →](LIBRARIES.md)**

---

## 🚀 Inicio Rápido

### 1. Clonar el repositorio
```bash
git clone https://github.com/mutatronika/Ornidia.git
cd Ornidia
```

### 2. Seleccionar el sistema
Navega al directorio del sistema que deseas usar:
- `cd solar_panel/` para monitoreo solar
- `cd greenhouse/` para invernadero
- `cd weather_station/` para estación meteorológica

### 3. Instalar librerías
```bash
# Ver LIBRARIES.md para lista completa
arduino-cli lib install "ACS712"
arduino-cli lib install "DHT sensor library"
# ... etc
```

### 4. Compilar y cargar
```bash
arduino-cli compile --fqbn esp8266:esp8266:generic
arduino-cli upload -p /dev/ttyUSB0 --fqbn esp8266:esp8266:generic
```

**[Guía completa de inicio →](QUICKSTART.md)**

---

## 🧪 Verificación de Integridad del Proyecto

Este proyecto incluye un sistema automatizado de verificación de integridad que valida:
- ✅ Estructura del proyecto
- ✅ Sintaxis de todos los sketches Arduino
- ✅ Dependencias de librerías
- ✅ Archivos de interfaz web
- ✅ Documentación
- ✅ Integridad del repositorio Git

### Ejecutar Verificación

```bash
# Hacer el script ejecutable (solo primera vez)
chmod +x verify_integrity.sh

# Ejecutar verificación
./verify_integrity.sh
```

### Resultado Esperado
```
✓ ALL TESTS PASSED! ✓
Total Tests: 76
Passed: 76
Failed: 0
```

### Documentación de Pruebas

- 📘 **[TESTING.md](TESTING.md)** - Guía completa de pruebas y verificación
- 📗 **[LIBRARIES.md](LIBRARIES.md)** - Dependencias y librerías requeridas
- 📙 **[QUICKSTART.md](QUICKSTART.md)** - Guía rápida de inicio

### CI/CD

El proyecto incluye GitHub Actions que ejecutan automáticamente la verificación de integridad en cada push o pull request.

Ver: `.github/workflows/integrity-check.yml`