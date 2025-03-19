# Ornidia
 Monitoreo de Panel solar con ESP8266


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
