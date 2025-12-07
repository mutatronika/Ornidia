# Ornidia - Guía de Plataformas

Esta guía detalla qué plataforma usar para cada sistema del proyecto Ornidia y sus ventajas específicas.

## 📋 Resumen de Compatibilidad

| Sistema | ESP8266 | ESP32 | Arduino | Raspberry Pi |
|---------|---------|-------|---------|--------------|
| 🔆 **Panel Solar** | ✅ **Recomendado** | ⚪ No necesario | ⚪ No WiFi | ⚪ Excesivo |
| 🌱 **Invernadero** | ✅ Recomendado | ✅ Avanzado | ✅ Básico | ⚪ Opcional |
| 🌤️ **Meteorológica** | ✅ Básico | ✅ Recomendado | ✅ Con Ethernet | ✅ Servidor |
| 💨 **Calidad Aire** | ⚪ Futuro | ⚪ Futuro | ✅ **Recomendado** | ✅ Avanzado |

---

## 🔆 Sistema de Panel Solar

### Plataforma Recomendada: **ESP8266**

#### ¿Por qué ESP8266?
- ✅ WiFi integrado para envío de datos a cloud
- ✅ Bajo consumo energético (importante para sistema solar)
- ✅ Suficiente memoria para sensores y web server
- ✅ Económico (~$3-5 USD)
- ✅ 1 ADC analógico suficiente (con multiplexor CD4052B)

#### Configuración Típica
```
ESP8266 + ACS712 (×3) + CD4052B + BH1750
```

#### Alternativas
- **Arduino Mega + Ethernet Shield**: Si requieres conexión cableada
- **ESP32**: Si necesitas más pines o procesamiento (no necesario para este sistema)

---

## 🌱 Sistema de Invernadero

### Plataformas Recomendadas: **ESP8266** / **ESP32** / **Arduino**

### Opción 1: ESP8266 (Básico con WiFi)
#### Ideal para:
- Invernadero pequeño a mediano
- Monitoreo remoto vía WiFi
- Control básico de relés
- Presupuesto limitado

#### Configuración
```
ESP8266 + DHT22 + BH1750 + Sensor humedad suelo + 2-4 relés
```

### Opción 2: ESP32 (Avanzado)
#### Ideal para:
- Invernadero grande
- Múltiples zonas de control
- Cámara ESP32-CAM
- Bluetooth para configuración local

#### Ventajas sobre ESP8266
- ✅ Más pines GPIO (control de más actuadores)
- ✅ Dual-core (multitarea real)
- ✅ Bluetooth integrado
- ✅ Mayor velocidad de procesamiento
- ✅ Compatible con cámaras

#### Configuración
```
ESP32 + DHT22 (×2-3) + BH1750 + BME280 + Múltiples sensores suelo + 8+ relés
```

### Opción 3: Arduino Uno/Nano (Sin WiFi)
#### Ideal para:
- Primer proyecto / aprendizaje
- Sin necesidad de conectividad
- Almacenamiento local en SD
- Máxima simplicidad

#### Configuración
```
Arduino Uno + DHT11 + Sensor humedad suelo + 2 relés + SD card
```

---

## 🌤️ Estación Meteorológica

### Plataformas Recomendadas: **ESP32** / **Arduino Mega** / **Raspberry Pi**

### Opción 1: ESP32 (Recomendado para uso general)
#### Ideal para:
- Estación completa con WiFi
- Envío a servicios cloud (Weather Underground, ThingSpeak)
- Múltiples sensores simultáneos
- Dashboard web embebido

#### Ventajas
- ✅ WiFi y Bluetooth
- ✅ Suficientes pines para todos los sensores
- ✅ Procesamiento rápido para cálculos meteorológicos
- ✅ Deep sleep para ahorro con panel solar

#### Configuración
```
ESP32 + BMP280 + DHT22 + BH1750 + Anemómetro + Pluviómetro + UV sensor
```

### Opción 2: ESP8266 (Básico)
#### Ideal para:
- Estación simple (temp, hum, presión, luz)
- Sin sensores de viento/lluvia
- Presupuesto muy limitado

#### Limitaciones
- Pocos pines GPIO (máximo 4-5 sensores)
- Sin hardware interrupts confiables para pluviómetro

### Opción 3: Arduino Mega + Ethernet/WiFi Shield
#### Ideal para:
- Máxima cantidad de sensores
- Conexión Ethernet cableada
- Sin depender de WiFi
- Estación profesional fija

#### Ventajas
- ✅ 54 pines digitales + 16 analógicos
- ✅ Ethernet confiable (cable UTP)
- ✅ Mayor estabilidad para exteriores
- ✅ Sin interferencias WiFi

#### Configuración
```
Arduino Mega + Ethernet Shield + BME280 + BH1750 + DHT22 + 
Anemómetro + Veleta + Pluviómetro + UV + Detector rayos
```

### Opción 4: Raspberry Pi (Servidor + Adquisición)
#### Ideal para:
- Estación meteorológica profesional
- Procesamiento avanzado de datos
- Servidor web local con Grafana
- Machine Learning para predicción
- Múltiples estaciones remotas

#### Ventajas
- ✅ Linux completo
- ✅ Python para análisis avanzado
- ✅ Base de datos local (MySQL/PostgreSQL/InfluxDB)
- ✅ Almacenamiento masivo (SD grande o SSD)
- ✅ Servidor web completo

#### Configuración
```
Raspberry Pi 4 + Sensores I2C/SPI + Arduino como ADC externo + 
Base de datos + Grafana + Node-RED + Servidor web
```

---

## 💨 Sistema de Calidad del Aire

### Plataformas Recomendadas: **Arduino Uno/Mega** / **Raspberry Pi**

### Opción 1: Arduino Uno/Mega (Recomendado)
#### Ideal para:
- Monitor de calidad del aire interior
- Uso en escuelas, oficinas, hogares
- Presupuesto limitado
- Simplicidad y confiabilidad

#### ¿Por qué Arduino y no ESP?
- ✅ Voltaje 5V nativo (sensores como PMS5003 requieren 5V)
- ✅ SoftwareSerial confiable para múltiples UART
- ✅ No sobrecalentamiento (sensores funcionan 24/7)
- ✅ Más pines analógicos para sensores MQ
- ✅ Económico

#### Arduino Uno
Para monitor básico:
```
Arduino Uno + PMS5003 + MH-Z19B + (opcional: BME280)
```

#### Arduino Mega
Para monitor completo:
```
Arduino Mega + PMS5003 + MH-Z19B + CCS811 + BME280 + MQ-135 + MQ-7 + SD card
```

### Opción 2: Raspberry Pi (Avanzado)
#### Ideal para:
- Monitor exterior urbano
- Integración con redes comunitarias (Sensor.Community, PurpleAir)
- Procesamiento de datos en tiempo real
- Múltiples sensores + cámara

#### Ventajas
- ✅ Conectividad WiFi/Ethernet integrada
- ✅ Python para fácil integración con APIs
- ✅ Almacenamiento masivo de datos
- ✅ Dashboard web local
- ✅ Envío automático a múltiples plataformas

#### Configuración
```
Raspberry Pi 3/4 + PMS5003 (USB Serial) + MH-Z19B (USB Serial) + 
BME280 (I2C) + Base de datos + Dashboard + Envío a cloud
```

### ¿Por qué NO ESP8266/ESP32 (por ahora)?
- ⚠️ Voltaje 3.3V requiere conversores de nivel
- ⚠️ PMS5003 requiere 5V (consume ~100mA)
- ⚠️ SoftwareSerial limitado en ESP
- ⚠️ Múltiples UART dificultan conexión simultánea
- ⚠️ ESP puede sobrecalentarse con sensores 24/7

**Nota**: Soporte para ESP en desarrollo con conversores de nivel.

---

## 📊 Comparativa de Características

| Característica | ESP8266 | ESP32 | Arduino | Raspberry Pi |
|---------------|---------|-------|---------|--------------|
| **Precio** | $3-5 | $6-10 | $5-15 | $35-75 |
| **WiFi** | ✅ | ✅ | ❌* | ✅ |
| **Bluetooth** | ❌ | ✅ | ❌ | ✅ |
| **Voltaje I/O** | 3.3V | 3.3V | 5V | 3.3V |
| **Pines GPIO** | ~11 | ~34 | 14-54 | 40 |
| **Pines ADC** | 1 | 18 | 6-16 | ❌** |
| **RAM** | 80KB | 520KB | 2-8KB | 1-8GB |
| **Procesamiento** | 80MHz | 240MHz | 16MHz | 1.5GHz |
| **Consumo** | Bajo | Medio | Muy bajo | Alto |
| **SO/Linux** | ❌ | ❌ | ❌ | ✅ |

*Con módulo externo
**Requiere ADC externo (MCP3008)

---

## 🔋 Consideraciones de Energía

### Para Sistemas con Panel Solar

#### Mejor opción: ESP8266
- Consumo activo: ~70mA
- Deep sleep: ~20μA
- WiFi: ~170mA (transmisión corta)

#### ESP32 Deep Sleep
- Consumo activo: ~160mA
- Deep sleep: ~10μA
- Ideal para envío cada 5-15 minutos

#### Arduino + Shield Ethernet
- Consumo constante: ~200mA
- No tiene deep sleep eficiente
- Requiere panel solar más grande

---

## 🌐 Conectividad Requerida

### WiFi (ESP8266/ESP32)
- ✅ Panel Solar: Envío a cloud
- ✅ Invernadero: Control remoto
- ✅ Meteorológica: Weather Underground
- ❌ Calidad Aire: No prioritario (uso local)

### Ethernet (Arduino Mega)
- ✅ Meteorológica: Instalación fija
- ⚪ Panel Solar: Alternativa sin WiFi
- ❌ Invernadero: Cable limitante
- ❌ Calidad Aire: No necesario

### Sin Conectividad (Arduino Uno + SD)
- ⚪ Cualquier sistema puede funcionar sin conexión
- Almacenamiento local en tarjeta SD
- Lectura por puerto serial

---

## 💡 Recomendaciones por Caso de Uso

### 1. "Quiero empezar con lo más simple"
→ **Arduino Uno** + Sensores básicos + Monitor Serial

### 2. "Necesito monitoreo remoto WiFi económico"
→ **ESP8266** (Panel Solar, Invernadero básico)

### 3. "Proyecto profesional con múltiples sensores"
→ **ESP32** o **Arduino Mega** (Meteorológica, Invernadero grande)

### 4. "Servidor local con procesamiento avanzado"
→ **Raspberry Pi** (Meteorológica central, Calidad aire urbano)

### 5. "Máxima autonomía con panel solar"
→ **ESP8266 con deep sleep** (Panel Solar, Meteorológica simple)

### 6. "Calidad del aire en casa/escuela"
→ **Arduino Uno/Mega** (Calidad Aire interior)

### 7. "Red de sensores comunitarios"
→ **Raspberry Pi** (Múltiples nodos de Calidad Aire)

---

## 🔄 Migración entre Plataformas

El código de Ornidia está diseñado para ser portable:

1. **Librerías estándar**: Wire, SoftwareSerial, etc.
2. **Abstracción de hardware**: Fácil cambio de pines
3. **Código modular**: Sensores independientes

### Ejemplo de migración:
```cpp
// ESP8266
#define DHT_PIN D7

// Arduino
#define DHT_PIN 7

// Mismo código de lectura
dht.begin(DHT_PIN);
```

---

## 📚 Recursos Adicionales

- [Pinout ESP8266](https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/)
- [Pinout ESP32](https://randomnerdtutorials.com/esp32-pinout-reference-gpios/)
- [Arduino Mega Pinout](https://www.arduino.cc/en/Hacking/PinMapping2560)
- [Raspberry Pi GPIO](https://pinout.xyz/)

---

**Última actualización**: 2025-10-27
