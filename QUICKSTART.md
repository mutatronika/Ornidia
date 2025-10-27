# Ornidia - Guía Rápida de Inicio

## 🎯 Descripción

Bienvenido a Ornidia, un proyecto de sistemas de monitoreo con microcontroladores dividido en tres áreas:

1. **🔆 Panel Solar** - Monitoreo de energía solar
2. **🌱 Invernadero** - Control ambiental para cultivos
3. **🌤️ Estación Meteorológica** - Medición atmosférica

Esta guía te ayudará a comenzar rápidamente con cualquiera de los sistemas.

---

## 📋 Requisitos Previos

### Hardware Mínimo
- Microcontrolador: ESP8266, ESP32, o Arduino
- Cable USB para programación
- Sensores según el sistema elegido (ver secciones específicas)

### Software
- Arduino IDE (1.8.x o superior) o arduino-cli
- Git (opcional, para clonar el repositorio)

---

## 🚀 Inicio Rápido por Sistema

### 1️⃣ Sistema de Panel Solar

**Hardware necesario:**
- ESP8266
- 3× ACS712 (sensores de corriente)
- CD4052B (multiplexor)
- BH1750 (sensor de luz)
- Divisor de voltaje (R1: 100KΩ, R2: 22KΩ)

**Instalación de librerías:**
```bash
arduino-cli lib install "ACS712"
arduino-cli lib install "BH1750"
```

**Cargar sketch:**
```bash
cd solar_panel/
# Abrir solar_monitor.ino en Arduino IDE
# Seleccionar: Tools → Board → ESP8266 → Generic ESP8266
# Compilar y cargar
```

**[Documentación completa →](solar_panel/README.md)**

---

### 2️⃣ Sistema de Invernadero

**Hardware necesario:**
- ESP8266 o ESP32
- DHT22 (temperatura y humedad)
- BH1750 (sensor de luz)
- Sensor de humedad de suelo
- Relés para actuadores (opcional)

**Instalación de librerías:**
```bash
arduino-cli lib install "DHT sensor library"
arduino-cli lib install "Adafruit Unified Sensor"
arduino-cli lib install "BH1750"
```

**Cargar sketch:**
```bash
cd greenhouse/
# Abrir greenhouse_monitor.ino en Arduino IDE
# Seleccionar placa ESP8266 o ESP32
# Configurar umbrales en el código según tus cultivos
# Compilar y cargar
```

**[Documentación completa →](greenhouse/README.md)**

---

### 3️⃣ Estación Meteorológica

**Hardware necesario:**
- ESP8266, ESP32 o Arduino Mega
- BMP180 (presión y temperatura)
- DHT22 (temperatura y humedad)
- BH1750 (luz ambiente)
- Anemómetro (opcional)
- Pluviómetro (opcional)

**Instalación de librerías:**
```bash
arduino-cli lib install "DHT sensor library"
arduino-cli lib install "Adafruit Unified Sensor"
arduino-cli lib install "BH1750"
arduino-cli lib install "Adafruit BME280 Library"
```

**Cargar sketch:**
```bash
cd weather_station/
# Abrir weather_station.ino en Arduino IDE
# Ajustar ALTITUDE según tu ubicación
# Seleccionar placa apropiada
# Compilar y cargar
```

**[Documentación completa →](weather_station/README.md)**

---

## 🔧 Configuración del Arduino IDE

### 1. Instalar soporte para ESP8266/ESP32

**ESP8266:**
1. File → Preferences → Additional Board Manager URLs
2. Añadir: `http://arduino.esp8266.com/stable/package_esp8266com_index.json`
3. Tools → Board → Boards Manager → buscar "ESP8266" → Install

**ESP32:**
1. File → Preferences → Additional Board Manager URLs
2. Añadir: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
3. Tools → Board → Boards Manager → buscar "ESP32" → Install

### 2. Instalar librerías

**Opción A: Desde Arduino IDE**
1. Tools → Manage Libraries (Ctrl+Shift+I)
2. Buscar e instalar las librerías según tu sistema

**Opción B: Desde arduino-cli**
```bash
# Ver sección de cada sistema arriba
```

**[Ver lista completa de librerías →](LIBRARIES.md)**

---

## ✅ Verificación de Integridad del Proyecto

Antes de comenzar, verifica que el proyecto esté íntegro:

```bash
# Hacer el script ejecutable (solo la primera vez)
chmod +x verify_integrity.sh

# Ejecutar las pruebas
./verify_integrity.sh
```

**Salida esperada:**
```
✓ ALL TESTS PASSED! ✓
Total Tests: 115
Passed: 115
Failed: 0
```

**Ver resultados detallados:**
```bash
cat /tmp/ornidia_test_results.txt
```

---

## 📊 Almacenamiento de Datos

Todos los sistemas incluyen soporte para:

### Local
- **Puerto Serial** - Monitoreo en tiempo real (115200 baud)
- **Tarjeta SD** - Almacenamiento persistente (sketch de ejemplo)
- **Servidor Web** - Dashboard HTML embebido

### Cloud/Remoto
- **MQTT** - Publicación a broker (configuración requerida)
- **HTTP/HTTPS** - API REST a servidor propio
- **ThingSpeak** - Plataforma IoT (API key requerida)
- **Blynk** - App móvil (configuración requerida)

---

## 🔍 Monitoreo Serial

Para ver los datos en tiempo real:

1. Conectar el microcontrolador por USB
2. Abrir Serial Monitor en Arduino IDE: Tools → Serial Monitor
3. Configurar velocidad: 115200 baud
4. Los datos se mostrarán automáticamente

---

## 📱 Próximos Pasos

### Para Sistema Solar
1. Calibrar sensores ACS712 con carga conocida
2. Ajustar divisor de voltaje según panel
3. Configurar servidor web o MQTT
4. Instalar en carcasa protegida

### Para Invernadero
1. Calibrar sensor de humedad de suelo
2. Configurar umbrales según cultivo
3. Conectar relés para automatización
4. Probar ciclos de riego y ventilación

### Para Estación Meteorológica
1. Configurar altitud de tu ubicación
2. Calibrar anemómetro y pluviómetro
3. Instalar sensores en carcasa exterior
4. Configurar envío a servicios meteorológicos

---

## 🐛 Solución Rápida de Problemas

### Arduino IDE no detecta la placa
- Verificar cable USB (usar cable con datos, no solo carga)
- Instalar drivers CH340 o CP2102 según el chip USB
- Seleccionar puerto correcto: Tools → Port

### Error al compilar por falta de librerías
- Instalar librerías requeridas (ver sección por sistema)
- Verificar instalación: Tools → Manage Libraries

### Sensor no responde
- Verificar conexiones VCC, GND, SDA/SCL
- Usar sketch i2c_scanner para detectar dispositivos I2C
- Comprobar dirección I2C correcta

### Script de verificación no ejecuta
```bash
chmod +x verify_integrity.sh
./verify_integrity.sh
```

### Tests fallan
1. Ver el reporte: `cat /tmp/ornidia_test_results.txt`
2. Revisar qué test falló
3. Consultar `TESTING.md` para soluciones

---

## 📚 Documentación Completa

- **[README.md](README.md)** - Descripción general del proyecto
- **[LIBRARIES.md](LIBRARIES.md)** - Dependencias de librerías
- **[TESTING.md](TESTING.md)** - Guía completa de pruebas
- **[solar_panel/README.md](solar_panel/README.md)** - Sistema solar
- **[greenhouse/README.md](greenhouse/README.md)** - Sistema de invernadero
- **[weather_station/README.md](weather_station/README.md)** - Estación meteorológica

---

## 🤝 Contribuir

1. Fork el repositorio
2. Crear una rama para tu feature
3. Ejecutar `./verify_integrity.sh` antes de commit
4. Enviar Pull Request

---

## 📞 Soporte

- **Issues**: [GitHub Issues](https://github.com/mutatronika/Ornidia/issues)
- **Documentación**: Ver archivos README.md de cada sistema
- **Tests**: Ejecutar `./verify_integrity.sh` para diagnóstico

---

**Última actualización**: 2025-10-27
**Versión**: 2.0

