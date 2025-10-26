# Ornidia
 Monitoreo de Panel solar con ESP8266
 
                 Panel Solar (Hybrytec)
                        |
                        | (Corriente medida con ACS712)
                        |
        [ PWM Charge Controller ]
            |            |
        Batería        Carga
        🔋             📦
        |             | (Corriente medida con ACS712)
        |
        | (Corriente medida con ACS712)
        |
       ESP8266 ← Multiplexor CD4052B ← Sensores
            |
        BH1750FVI (I2C)

# Resumen de tu Hardware
Componente	Especificación
Panel Solar (Hybrytec)	3W, 12V (Vmax), 0.25A (Imax), Voc 15V, Isc 0.29A
Controlador de Carga (PWM)	12V/24V, 20A
Batería (MTEK)	12V, 2.7Ah
ESP8266	Microcontrolador principal
Sensores	ACS712 (corriente), CD4052B (multiplexor), Divisor de tensión (voltaje), BH1750 (luz)
📌 Tu sistema monitorea:
✅ Corriente del panel
✅ Corriente de la batería
✅ Corriente de la carga
✅ Voltaje del panel
✅ Luz ambiente

 
Características: PWM Solar Charge Controller

Voltaje de carga:12V ó 24V
Capacidad: 20A
Rango de temperatura: -25~55°C
Voltaje de protección de sobre descarga: 10,5V ó 21V
Voltaje de protección de sobre carga :14,4V ó 27,4V
Voltaje de Swicheo: 11,8V o 23,6V
Voltaje de protección de sobrecarga:14V ó 28V

                   [ Panel Solar 12V ]
                           │
           (Corriente Panel medida con ACS712)
                           │
       +-------------------+----------------+
       |  Controlador PWM Solar (12V)       |
       |  - Carga Batería                    |
       |  - Alimentación de Carga (12V)      |
       +-------------------+----------------+
                           │
                           │
      (Corriente Batería medida con ACS712)
                           │
                 +------------------+
                 |   Batería 12V     |
                 +------------------+
                           │
         (Corriente Carga medida con ACS712)
                           │
         +-----------------------------------+
         |       Regulador 12V → 5V (MC34063A) |
         |        - Alimenta ESP8266           |
         +-----------------------------------+
                           │
         +-----------------------------------+
         |         ESP8266 (WiFi)            |
         |  - Recibe 5V del MC34063A         |
         |  - Controla Multiplexor CD4052B   |
         |  - Lee sensores ACS712            |
         |  - Lee voltaje del panel con Divisor |
         |  - Muestra datos en Web/MQTT      |
         +-----------------------------------+
                           │
    +--------------------------------------------------+
    |    Multiplexor CD4052B (4 Canales Analógicos)   |
    |    CH0 - ACS712 (Panel)                         |
    |    CH1 - ACS712 (Batería)                       |
    |    CH2 - ACS712 (Carga)                         |
    |    CH3 - Divisor de Voltaje (Panel)            |
    +--------------------------------------------------+


Regulacion 12V a 5V

    12V del PWM
       │
       │   +---------+
       ├──►| Vin     |
       │   |         | MC34063A
       ├──►| SW      |────┬──► +5V a ESP8266
       │   |         |    │
       │   |         |   (Inductor 100µH)
       │   +---------+    │
       │                 │
       └─────────────────┘ GND


Divisor de VOltaje 

        +--------------+
        | Panel Solar  |
        |  12V-15V     |
        +--------------+
                |
       (Divisor de Voltaje)
                |
           ESP8266 (A0)
+---------------------------------+

        +12V del Panel Solar
                |
              [ R1 ] (100KΩ)
                |
        -------> (A0 ESP8266)
                |
              [ R2 ] (22KΩ)
                |
               GND




ACS712ELCTR-05B-T: Para mediciones precisas de hasta ±5A
ACS712ELCTR-20A-T: Para rangos más amplios de hasta ±20A
ACS712ELCTR-30A-T: Para las mayores corrientes de hasta ±30A


Conexión del CD4052B con la ESP8266
Pin del CD4052B	Conexión	Descripción
VCC (Pin 16)	3.3V ESP8266	Alimentación del multiplexor
GND (Pin 8)	GND ESP8266	Tierra
X (Pin 14)	A0 ESP8266	Salida común (único canal analógico de ESP8266)
X0 (Pin 15)	Salida ACS712 Panel	Sensor de corriente del panel
X1 (Pin 12)	Salida ACS712 Batería	Sensor de corriente de la batería
X2 (Pin 13)	Salida ACS712 Carga	Sensor de corriente de la carga
X3 (Pin 10)	Salida del divisor de voltaje	Voltaje del panel
S0 (Pin 9)	D5 ESP8266	Selector de canal (Bit 0)
S1 (Pin 10)	D6 ESP8266	Selector de canal (Bit 1)
INH (Pin 6)	GND ESP8266	Habilitación (debe estar en GND para funcionar)
Y (Pin 3)	Sin usar	No lo necesitamos
📌 Cómo se selecciona cada canal usando S0 y S1:

S1 (D6)	S0 (D5)	Canal Activo	Sensor
0	0	X0	ACS712 Panel
0	1	X1	ACS712 Batería
1	0	X2	ACS712 Carga
1	1	X3	Voltaje del Panel
✅ La ESP8266 cambia S0 y S1 para seleccionar qué sensor leer en A0.

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