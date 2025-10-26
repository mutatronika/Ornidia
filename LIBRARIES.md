# Ornidia - Dependencias de Librerías

Este archivo lista todas las librerías Arduino requeridas para compilar los sketches del proyecto Ornidia.

## Librerías Requeridas

### Sensores
- **ACS712** - Sensor de corriente Hall Effect
  - Versión recomendada: 0.3.0 o superior
  - Repositorio: https://github.com/rkoptev/ACS712-arduino
  
- **DHT sensor library** - Sensor de temperatura y humedad
  - Versión recomendada: 1.4.0 o superior
  - Repositorio: https://github.com/adafruit/DHT-sensor-library
  - Dependencia: Adafruit Unified Sensor
  
- **BH1750** - Sensor de luz digital I2C
  - Versión recomendada: 1.3.0 o superior
  - Repositorio: https://github.com/claws/BH1750
  
- **BMP180I2C** - Sensor de presión y temperatura
  - Versión recomendada: Última disponible
  - Nota: Verificar compatibilidad con tu placa

### Comunicación (Incluidas con Arduino IDE)
- **Wire** - Librería I2C estándar
- **SPI** - Librería SPI estándar
- **Ethernet** - Conectividad Ethernet para Arduino
- **SD** - Lectura/escritura de tarjeta SD

## Instalación

### Método 1: Arduino IDE (Recomendado)

1. Abrir Arduino IDE
2. Menú: Tools → Manage Libraries... (o Ctrl+Shift+I)
3. Buscar e instalar cada librería:

```
ACS712
DHT sensor library
Adafruit Unified Sensor (dependencia de DHT)
BH1750
```

4. Para BMP180I2C, puede que necesites instalarla manualmente o buscar una alternativa como "Adafruit BMP085 Unified"

### Método 2: arduino-cli

```bash
# Actualizar índice de librerías
arduino-cli lib update-index

# Instalar librerías
arduino-cli lib install "ACS712"
arduino-cli lib install "DHT sensor library"
arduino-cli lib install "Adafruit Unified Sensor"
arduino-cli lib install "BH1750"
```

### Método 3: Instalación Manual

1. Descargar las librerías desde sus repositorios
2. Extraer en la carpeta de librerías de Arduino:
   - Windows: `Documents/Arduino/libraries/`
   - macOS: `~/Documents/Arduino/libraries/`
   - Linux: `~/Arduino/libraries/`
3. Reiniciar Arduino IDE

## Verificación de Instalación

### Opción 1: Arduino IDE
1. Menú: Sketch → Include Library
2. Verificar que todas las librerías aparezcan en la lista

### Opción 2: arduino-cli
```bash
arduino-cli lib list
```

Debe mostrar:
```
ACS712               0.3.x
DHT sensor library   1.4.x
BH1750               1.3.x
Adafruit Unified Sensor  1.1.x
```

## Librerías por Sketch

### solar_monitor.ino
- Ethernet
- SPI
- SD

### fase1.ino
- ACS712
- Wire

### CD4058.ino
- ACS712
- Wire

### 02_DHT11ino.ino
- ACS712
- DHT
- Wire

### 03_BH1750FVI.ino
- BH1750
- DHT
- Wire

### 04_BMP180.ino
- BH1750
- BMP180I2C
- DHT
- Wire

## Plataformas Soportadas

### ESP8266
El proyecto está diseñado principalmente para ESP8266. Asegúrate de tener instalado el soporte de placa:

```bash
# Añadir URL de paquetes ESP8266 en Arduino IDE:
# File → Preferences → Additional Board Manager URLs:
http://arduino.esp8266.com/stable/package_esp8266com_index.json
```

Luego instalar:
- Tools → Board → Boards Manager → buscar "ESP8266" e instalar

### Arduino Mega
Algunos sketches (carpeta Arduino/Mega) están diseñados para Arduino Mega 2560:
- Ethernet
- SPI
- SD

## Problemas Comunes

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
| SD | 1.2.0 | 1.2.4 | Para Arduino Mega |

## Actualización de Librerías

Para actualizar todas las librerías:

### Arduino IDE
1. Tools → Manage Libraries
2. Filtrar por "UPDATABLE"
3. Actualizar individualmente o todas a la vez

### arduino-cli
```bash
arduino-cli lib upgrade
```

## Licencias

Cada librería tiene su propia licencia. Consultar los repositorios individuales para detalles:
- ACS712: MIT License
- DHT sensor library: MIT License
- BH1750: MIT License
- Ethernet, SPI, SD, Wire: LGPL

## Recursos Adicionales

- [Arduino Library Guide](https://www.arduino.cc/en/Guide/Libraries)
- [ESP8266 Arduino Core](https://github.com/esp8266/Arduino)
- [arduino-cli Documentation](https://arduino.github.io/arduino-cli/)

---

**Última actualización**: 2025-10-26
