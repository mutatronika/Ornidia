# Ornidia - Guía de Pruebas y Verificación de Integridad

## Descripción

Este documento describe cómo verificar la integridad del proyecto Ornidia, un sistema de monitoreo de panel solar basado en ESP8266.

## Pruebas Automáticas

### Script de Verificación de Integridad

El proyecto incluye un script automatizado que verifica la integridad del sistema:

```bash
./verify_integrity.sh
```

### Qué Verifica el Script

El script realiza las siguientes pruebas:

#### 1. Estructura del Proyecto
- ✓ Verifica que existan todos los directorios principales
- ✓ Comprueba la existencia de archivos clave (README, .gitignore)

#### 2. Validación de Sketches Arduino
- ✓ Verifica la sintaxis básica de todos los archivos .ino
- ✓ Comprueba la presencia de funciones setup() y loop()
- ✓ Valida el balanceo de llaves y paréntesis
- ✓ Verifica que los archivos tengan contenido
- ✓ Comprueba la inicialización de comunicación Serial

#### 3. Dependencias de Librerías
- ✓ Lista todas las librerías requeridas por el proyecto
- ✓ Identifica dependencias únicas

#### 4. Validación de Sketches Principales
- ✓ Verifica funciones críticas en solar_monitor.ino
- ✓ Comprueba implementaciones específicas (Filtro Kalman, calibración)

#### 5. Archivos de Interfaz Web
- ✓ Valida la estructura HTML de index.html
- ✓ Verifica la existencia de CSS y JavaScript
- ✓ Comprueba la sintaxis básica de archivos web

#### 6. Documentación
- ✓ Verifica la existencia y contenido de README.md
- ✓ Comprueba documentación de hardware
- ✓ Valida información de conexiones

#### 7. Integridad de Git
- ✓ Verifica que el repositorio Git esté inicializado
- ✓ Comprueba la existencia de .gitignore
- ✓ Valida que los artefactos de compilación estén ignorados

## Resultados de las Pruebas

### Última Ejecución

**Fecha**: Verificar archivo `/tmp/ornidia_test_results.txt`

**Resultado**: ✓ TODAS LAS PRUEBAS PASARON

**Estadísticas**:
- Total de pruebas: 76
- Pruebas exitosas: 76
- Pruebas fallidas: 0

### Archivo de Resultados

Los resultados detallados se guardan en `/tmp/ornidia_test_results.txt` después de cada ejecución.

## Sketches del Proyecto

### Sketches Principales

1. **Arduino/solar_monitor/solar_monitor.ino**
   - Sistema completo de monitoreo con Ethernet y servidor web
   - Incluye lectura de sensores ACS712
   - Control de LEDs de estado
   - Interfaz web para visualización de datos

2. **test/fase1/fase1.ino**
   - Implementación por fases (FASE_1, FASE_2, FASE_3)
   - Filtro Kalman optimizado
   - Sistema de calibración automática
   - Soporte para corrección de no-linealidad del ADC

3. **test/CD4058/CD4058.ino**
   - Lectura con multiplexor CD4052B
   - Filtrado Kalman para estabilidad
   - Calibración de offsets

### Sketches de Prueba

- **01_ASC712_CD4052B/01_ASC712_CD4052B.ino**: Prueba básica de ACS712 con multiplexor
- **02_DHT11ino/02_DHT11ino.ino**: Integración de sensor de temperatura/humedad
- **03_BH1750FVI/03_BH1750FVI.ino**: Sensor de luz ambiente
- **04_BMP180/04_BMP180.ino**: Sensor de presión atmosférica y temperatura

## Librerías Requeridas

Para compilar los sketches, necesitas instalar las siguientes librerías en el Arduino IDE:

### Librerías de Sensores
- **ACS712**: Sensor de corriente
- **DHT**: Sensor de temperatura y humedad (DHT11/DHT22)
- **BH1750**: Sensor de luz ambiente I2C
- **BMP180I2C**: Sensor de presión y temperatura I2C

### Librerías de Comunicación
- **Wire**: Comunicación I2C (incluida con Arduino IDE)
- **SPI**: Comunicación SPI (incluida con Arduino IDE)
- **Ethernet**: Conectividad Ethernet (incluida con Arduino IDE)
- **SD**: Lectura/escritura de tarjeta SD (incluida con Arduino IDE)

### Instalación de Librerías

#### Opción 1: Mediante el Arduino IDE
1. Abrir Arduino IDE
2. Ir a Sketch → Include Library → Manage Libraries
3. Buscar cada librería por nombre
4. Hacer clic en "Install"

#### Opción 2: Mediante arduino-cli
```bash
arduino-cli lib install ACS712
arduino-cli lib install "DHT sensor library"
arduino-cli lib install BH1750
arduino-cli lib install BMP180I2C
```

## Verificación Manual

### 1. Verificar Estructura de Archivos

```bash
# Listar todos los sketches
find . -name "*.ino" -type f

# Verificar archivos web
ls Arduino/solar_monitor/data/
```

### 2. Revisar Configuración de Hardware

Consulta el README.md para:
- Diagrama de conexiones
- Especificaciones de componentes
- Configuración de pines

### 3. Pruebas de Compilación

Si tienes Arduino IDE o arduino-cli instalado:

```bash
# Verificar compilación de sketch principal
arduino-cli compile --fqbn esp8266:esp8266:generic Arduino/solar_monitor/

# Verificar sketch de prueba
arduino-cli compile --fqbn esp8266:esp8266:generic test/fase1/
```

## Interpretación de Resultados

### Todos los Tests Pasan ✓
El proyecto está íntegro y listo para uso.

### Algunos Tests Fallan ✗
Revisa el archivo de resultados en `/tmp/ornidia_test_results.txt` para identificar:
- Archivos faltantes
- Errores de sintaxis
- Problemas de estructura

## Solución de Problemas

### Error: "Sketch file is empty"
- El archivo .ino está vacío o corrupto
- Restaurar desde el repositorio Git

### Error: "Unbalanced braces"
- Hay un error de sintaxis en el código
- Revisar el archivo indicado y corregir llaves faltantes

### Error: "Missing setup() or loop() function"
- El sketch no tiene la estructura básica de Arduino
- Agregar funciones setup() y loop()

### Error: "Library not found" (compilación)
- Instalar las librerías requeridas según la sección anterior

## Contribuir al Proyecto

### Antes de Hacer Commit

1. Ejecutar el script de verificación:
   ```bash
   ./verify_integrity.sh
   ```

2. Asegurarse de que todas las pruebas pasen

3. Actualizar documentación si es necesario

### Añadir Nuevos Tests

Para añadir nuevas validaciones, edita `verify_integrity.sh` y agrega tu test en la sección apropiada.

## Hardware del Sistema

### Componentes Principales
- **ESP8266**: Microcontrolador principal
- **CD4052B**: Multiplexor analógico 4 canales
- **ACS712**: Sensores de corriente (×3)
- **BH1750**: Sensor de luz ambiente
- **Divisor de voltaje**: Para medición de voltaje del panel

### Conexiones Clave
- S0 → D5 (GPIO14)
- S1 → D6 (GPIO12)
- Entrada analógica → A0

Ver README.md para diagrama completo.

## Mantenimiento

### Frecuencia Recomendada
- Ejecutar verificación de integridad: **Antes de cada deployment**
- Revisar librerías actualizadas: **Mensualmente**
- Validar documentación: **Con cada cambio mayor**

## Contacto y Soporte

Para reportar problemas o sugerir mejoras:
1. Abrir un issue en el repositorio
2. Ejecutar `./verify_integrity.sh` y adjuntar resultados
3. Incluir información del hardware/software usado

---

**Última actualización**: 2025-10-26
**Versión del script**: 1.0
