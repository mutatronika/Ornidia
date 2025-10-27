# Sistema de Monitoreo de Panel Solar

## Descripción
Sistema de monitoreo en tiempo real para paneles solares utilizando ESP8266 y sensores de corriente ACS712.

## Hardware Requerido

### Componentes Principales
- **ESP8266** - Microcontrolador WiFi
- **Panel Solar** - Hybrytec 3W, 12V (Vmax), 0.25A (Imax)
- **Controlador de Carga PWM** - 12V/24V, 20A
- **Batería** - MTEK 12V, 2.7Ah
- **Regulador MC34063A** - Conversión 12V a 5V

### Sensores
- **ACS712** (×3) - Sensores de corriente Hall Effect
  - Panel: ACS712ELCTR-05B-T
  - Batería: ACS712ELCTR-05B-T  
  - Carga: ACS712ELCTR-05B-T
- **CD4052B** - Multiplexor analógico 4 canales
- **BH1750FVI** - Sensor de luz ambiente I2C
- **Divisor de voltaje** - Para medición de voltaje del panel (R1: 100KΩ, R2: 22KΩ)

## Arquitectura del Sistema

```
Panel Solar (12V)
    │
    │ (Corriente medida con ACS712)
    │
[ PWM Charge Controller ]
    │            │
Batería        Carga
  🔋            📦
    │            │ (Corriente medida con ACS712)
    │
    │ (Corriente medida con ACS712)
    │
ESP8266 ← Multiplexor CD4052B ← Sensores
    │
BH1750FVI (I2C)
```

## Conexiones

### Multiplexor CD4052B
| Pin CD4052B | Conexión | Descripción |
|------------|----------|-------------|
| VCC (16) | 3.3V ESP8266 | Alimentación |
| GND (8) | GND ESP8266 | Tierra |
| X (14) | A0 ESP8266 | Salida común analógica |
| X0 (15) | ACS712 Panel | Corriente del panel |
| X1 (12) | ACS712 Batería | Corriente de la batería |
| X2 (13) | ACS712 Carga | Corriente de la carga |
| X3 (10) | Divisor de voltaje | Voltaje del panel |
| S0 (9) | D5 ESP8266 | Selector canal (Bit 0) |
| S1 (10) | D6 ESP8266 | Selector canal (Bit 1) |
| INH (6) | GND | Habilitación |

### Selección de Canales
| S1 (D6) | S0 (D5) | Canal | Sensor |
|---------|---------|-------|--------|
| 0 | 0 | X0 | ACS712 Panel |
| 0 | 1 | X1 | ACS712 Batería |
| 1 | 0 | X2 | ACS712 Carga |
| 1 | 1 | X3 | Voltaje del Panel |

## Parámetros Monitoreados

- ✅ Corriente del panel solar
- ✅ Corriente de la batería
- ✅ Corriente de la carga
- ✅ Voltaje del panel
- ✅ Potencia generada
- ✅ Luz ambiente

## Estructura de Archivos

```
solar_panel/
├── README.md              # Este archivo
├── solar_monitor.ino      # Sketch principal con servidor web
├── data/                  # Archivos de interfaz web
│   ├── index.html
│   ├── style.css
│   └── script.js
├── examples/              # Ejemplos y pruebas
│   ├── 01_ASC712_CD4052B.ino
│   └── 01_ASC712_CD4052B_KALMAN.ino
└── sensors/               # Código específico de sensores
    └── ACS712.ino
```

## Librerías Requeridas

```cpp
#include <Wire.h>          // I2C (incluida con Arduino IDE)
#include <ACS712.h>        // Sensor de corriente
#include <BH1750.h>        // Sensor de luz
#include <Ethernet.h>      // Conectividad de red
#include <SPI.h>           // Comunicación SPI
#include <SD.h>            // Tarjeta SD
```

### Instalación
```bash
arduino-cli lib install "ACS712"
arduino-cli lib install "BH1750"
```

## Compilación y Carga

### Arduino IDE
1. Abrir `solar_monitor.ino`
2. Seleccionar placa: Tools → Board → ESP8266 → Generic ESP8266 Module
3. Configurar puerto: Tools → Port
4. Compilar y cargar: Sketch → Upload

### arduino-cli
```bash
arduino-cli compile --fqbn esp8266:esp8266:generic solar_monitor/
arduino-cli upload -p /dev/ttyUSB0 --fqbn esp8266:esp8266:generic solar_monitor/
```

## Características del PWM Solar Charge Controller

- **Voltaje de carga**: 12V ó 24V
- **Capacidad**: 20A
- **Rango de temperatura**: -25~55°C
- **Voltaje protección sobre descarga**: 10.5V ó 21V
- **Voltaje protección sobre carga**: 14.4V ó 27.4V
- **Voltaje de switching**: 11.8V o 23.6V
- **Voltaje protección sobrecarga**: 14V ó 28V

## Especificaciones del Panel Solar

- **Potencia**: 3W
- **Voltaje máximo**: 12V
- **Corriente máxima**: 0.25A
- **Voltaje circuito abierto (Voc)**: 15V
- **Corriente cortocircuito (Isc)**: 0.29A

## Almacenamiento de Datos

El sistema soporta almacenamiento de datos en:
- **Tarjeta SD** - Almacenamiento local
- **Servidor web** - Interfaz HTTP para visualización
- **MQTT** - Para integración con servidores caseros o cloud

## Monitoreo y Visualización

El sistema incluye:
- Servidor web embebido con dashboard HTML/CSS/JS
- LEDs de estado para indicación visual
- Puerto serial para debugging (115200 baud)

## Uso

1. Conectar el hardware según el diagrama
2. Cargar el sketch en el ESP8266
3. Conectar a la red WiFi/Ethernet
4. Acceder a la interfaz web desde el navegador
5. Monitorear los parámetros en tiempo real

## Ejemplos Incluidos

### 01_ASC712_CD4052B.ino
Lectura básica de sensores ACS712 con multiplexor CD4052B.

### 01_ASC712_CD4052B_KALMAN.ino
Implementación con filtro Kalman para mejorar la precisión de las lecturas.

## Soporte para Múltiples Plataformas

- ESP8266 (principal)
- Arduino Mega 2560 (ejemplos en Arduino/Mega/)
- Raspberry Pi (futuro)
- ESP32 (futuro)

## Troubleshooting

### Panel no detecta corriente
- Verificar conexión del ACS712
- Comprobar polaridad
- Revisar calibración del sensor

### Voltaje incorrecto
- Verificar divisor de voltaje (R1: 100KΩ, R2: 22KΩ)
- Comprobar VREF (3.3V para ESP8266)

### Multiplexor no responde
- Verificar conexiones S0, S1
- Asegurar que INH esté en GND
- Comprobar alimentación 3.3V

## Referencias

- [Datasheet ACS712](../datasheet/)
- [Documentación ESP8266](http://arduino.esp8266.com)
- [Guía de librerías](../LIBRARIES.md)

---

**Última actualización**: 2025-10-27
