#include <SPI.h>
#include <Ethernet.h>
#include <SD.h>

// Pines para los LEDs
#define LED_BAT_VERDE 2
#define LED_BAT_ROJO 3
#define LED_CARGA_VERDE 4
#define LED_CARGA_ROJO 5
#define LED_PANEL_VERDE 6
#define LED_PANEL_ROJO 7

// Pines analógicos para los sensores ACS712
#define PIN_PANEL A0
#define PIN_BATERIA A1
#define PIN_CARGA A2

// Pines analógicos para los divisores de voltaje
#define PIN_VOLTAJE_PANEL A3
#define PIN_VOLTAJE_BATERIA A4
#define PIN_VOLTAJE_CARGA A5

// Constantes para los cálculos
#define MUESTRAS 1000
#define MV_POR_AMP 185.0  // 185mV/A para ACS712-05B
#define OFFSET_VOLTAJE 2500 // 2.5V en mV
#define RELACION_DIVISOR 5.0 // Relación del divisor de voltaje

// Buffer para manejo HTTP sin String
#define BUFFER_SIZE 512
char httpBuffer[BUFFER_SIZE];

// Configuración de red
byte mac[] = { 0xDE, 0xA5, 0x2E, 0xEF, 0xF0, 0xED };
IPAddress ip(192, 168, 1, 140);
EthernetServer server(80);

// Variables para almacenar las mediciones
float corrientePanel = 0;
float corrienteBateria = 0;  // Positiva = carga, Negativa = descarga
float corrienteCarga = 0;
float voltajePanel = 0;
float voltajeBateria = 0;
float voltajeCarga = 0;

// Estados del sistema
bool ethernetOK = false;
bool sdOK = false;

void setup() {
  // Inicializar comunicación serial
  Serial.begin(9600);
  Serial.println(F("Iniciando sistema de monitoreo solar..."));
  
  // Configurar pines de los LEDs
  pinMode(LED_BAT_VERDE, OUTPUT);
  pinMode(LED_BAT_ROJO, OUTPUT);
  pinMode(LED_CARGA_VERDE, OUTPUT);
  pinMode(LED_CARGA_ROJO, OUTPUT);
  pinMode(LED_PANEL_VERDE, OUTPUT);
  pinMode(LED_PANEL_ROJO, OUTPUT);
  
  // Parpadear todos los LEDs como test inicial
  testLEDs();
  
  // Inicializar Ethernet con validación
  Serial.print(F("Configurando Ethernet... "));
  if (Ethernet.begin(mac) == 0) {
    Serial.println(F("DHCP falló, usando IP estática"));
    Ethernet.begin(mac, ip);
    delay(1000);
    
    // Verificar si Ethernet responde
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println(F("ERROR: Shield Ethernet no encontrado"));
      ethernetOK = false;
    } else if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println(F("ERROR: Cable Ethernet desconectado"));
      ethernetOK = false;
    } else {
      ethernetOK = true;
      Serial.print(F("IP: "));
      Serial.println(Ethernet.localIP());
    }
  } else {
    ethernetOK = true;
    Serial.print(F("DHCP OK, IP: "));
    Serial.println(Ethernet.localIP());
  }
  
  // Inicializar tarjeta SD con validación
  Serial.print(F("Inicializando tarjeta SD... "));
  if (!SD.begin(4)) {
    Serial.println(F("ERROR: No se pudo inicializar SD"));
    sdOK = false;
  } else {
    Serial.println(F("SD OK"));
    sdOK = true;
  }
  
  // Iniciar servidor solo si Ethernet funciona
  if (ethernetOK) {
    server.begin();
    Serial.println(F("Servidor web iniciado"));
  }
  
  Serial.println(F("Sistema listo"));
}

void loop() {
  // Verificar estado de conexión Ethernet
  if (ethernetOK && Ethernet.linkStatus() == LinkOFF) {
    Serial.println(F("Advertencia: Cable Ethernet desconectado"));
  }
  
  // Leer sensores
  leerSensores();
  
  // Controlar LEDs
  controlarLEDs();
  
  // Manejar clientes web solo si Ethernet funciona
  if (ethernetOK) {
    manejarClienteWeb();
  }
  
  delay(100); // Pequeña pausa para estabilidad
}

void testLEDs() {
  // Test de LEDs al inicio
  int leds[] = {LED_BAT_VERDE, LED_BAT_ROJO, LED_CARGA_VERDE, LED_CARGA_ROJO, LED_PANEL_VERDE, LED_PANEL_ROJO};
  
  for (int i = 0; i < 6; i++) {
    digitalWrite(leds[i], HIGH);
    delay(100);
    digitalWrite(leds[i], LOW);
  }
}

void leerSensores() {
  // Leer corriente en los tres sensores
  corrientePanel = leerCorriente(PIN_PANEL);
  corrienteBateria = leerCorrienteBidireccional(PIN_BATERIA); // Corregido para batería
  corrienteCarga = leerCorriente(PIN_CARGA);
  
  // Leer voltajes
  voltajePanel = leerVoltaje(PIN_VOLTAJE_PANEL) * RELACION_DIVISOR;
  voltajeBateria = leerVoltaje(PIN_VOLTAJE_BATERIA) * RELACION_DIVISOR;
  voltajeCarga = leerVoltaje(PIN_VOLTAJE_CARGA) * RELACION_DIVISOR;
  
  // Mostrar valores por serial
  Serial.print(F("Panel - V: "));
  Serial.print(voltajePanel, 2);
  Serial.print(F("V, I: "));
  Serial.print(corrientePanel, 3);
  Serial.print(F("A | Batería - V: "));
  Serial.print(voltajeBateria, 2);
  Serial.print(F("V, I: "));
  Serial.print(corrienteBateria, 3);
  Serial.print(F("A "));
  Serial.println(corrienteBateria >= 0 ? F("(Cargando)") : F("(Descargando)"));
}

float leerCorriente(int pin) {
  float suma = 0;
  for (int i = 0; i < MUESTRAS; i++) {
    int valor = analogRead(pin);
    // Convertir a voltaje (0-5V)
    float voltaje = (valor / 1023.0) * 5000; // en mV
    // Convertir a corriente (restar offset y dividir por sensibilidad)
    float corriente = ((voltaje - OFFSET_VOLTAJE) / MV_POR_AMP);
    suma += corriente * corriente;
    delayMicroseconds(100); // Más preciso que delay(1)
  }
  // Calcular valor RMS
  float corriente_rms = sqrt(suma / MUESTRAS);
  return fabs(corriente_rms); // ✅ Corregido: usar fabs() para flotantes
}

float leerCorrienteBidireccional(int pin) {
  float suma = 0;
  float sumaInstantanea = 0;
  
  for (int i = 0; i < MUESTRAS; i++) {
    int valor = analogRead(pin);
    // Convertir a voltaje (0-5V)
    float voltaje = (valor / 1023.0) * 5000; // en mV
    // Convertir a corriente (restar offset y dividir por sensibilidad)
    float corriente = ((voltaje - OFFSET_VOLTAJE) / MV_POR_AMP);
    
    suma += corriente * corriente; // Para RMS
    sumaInstantanea += corriente;  // Para dirección
    delayMicroseconds(100);
  }
  
  // Calcular RMS
  float corriente_rms = sqrt(suma / MUESTRAS);
  // Determinar dirección basada en promedio
  float promedio = sumaInstantanea / MUESTRAS;
  
  // Retornar valor con signo (+ = carga, - = descarga)
  return (promedio >= 0) ? corriente_rms : -corriente_rms;
}

float leerVoltaje(int pin) {
  float suma = 0;
  for (int i = 0; i < 100; i++) {
    suma += analogRead(pin);
    delayMicroseconds(100);
  }
  float promedio = suma / 100.0;
  // Convertir a voltaje (0-5V)
  return (promedio * 5.0) / 1023.0;
}

void controlarLEDs() {
  // Controlar LEDs de batería (mejorado)
  if (voltajeBateria > 12.5) { // Umbral más realista
    digitalWrite(LED_BAT_VERDE, HIGH);
    digitalWrite(LED_BAT_ROJO, LOW);
  } else if (voltajeBateria < 11.8) { // Batería baja
    digitalWrite(LED_BAT_VERDE, LOW);
    digitalWrite(LED_BAT_ROJO, HIGH);
  } else { // Estado intermedio - parpadeo
    static unsigned long ultimoParpadeo = 0;
    static bool estadoLED = false;
    if (millis() - ultimoParpadeo > 500) {
      estadoLED = !estadoLED;
      digitalWrite(LED_BAT_VERDE, LOW);
      digitalWrite(LED_BAT_ROJO, estadoLED);
      ultimoParpadeo = millis();
    }
  }
  
  // Controlar LEDs de carga
  digitalWrite(LED_CARGA_VERDE, corrienteCarga < 1.0);
  digitalWrite(LED_CARGA_ROJO, corrienteCarga >= 1.0);
  
  // Controlar LEDs de panel
  digitalWrite(LED_PANEL_VERDE, voltajePanel > 14.0);
  digitalWrite(LED_PANEL_ROJO, voltajePanel <= 14.0);
}

void manejarClienteWeb() {
  EthernetClient client = server.available();
  if (client) {
    boolean currentLineIsBlank = true;
    char request[200] = {0}; // ✅ Buffer fijo en lugar de String
    int requestIndex = 0;
    
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        
        // Almacenar request en buffer fijo
        if (requestIndex < sizeof(request) - 1) {
          request[requestIndex++] = c;
        }
        
        if (c == '\n' && currentLineIsBlank) {
          // Responder según la ruta solicitada
          if (strstr(request, "GET /data") != NULL) {
            enviarDatosJSON(client);
          } 
          else if (strstr(request, "GET /") != NULL && strstr(request, "GET / ") != NULL) {
            servirArchivo(client, "/index.html");
          } 
          else if (strstr(request, "GET /style.css") != NULL) {
            servirArchivo(client, "/style.css");
          } 
          else if (strstr(request, "GET /script.js") != NULL) {
            servirArchivo(client, "/script.js");
          }
          else {
            enviar404(client);
          }
          break;
        }
        
        if (c == '\n') {
          currentLineIsBlank = true;
        } else if (c != '\r') {
          currentLineIsBlank = false;
        }
      }
    }
    
    delay(1);
    client.stop();
  }
}

void enviarDatosJSON(EthernetClient client) {
  // Enviar headers HTTP
  client.println(F("HTTP/1.1 200 OK"));
  client.println(F("Content-Type: application/json"));
  client.println(F("Access-Control-Allow-Origin: *"));
  client.println(F("Connection: close"));
  client.println();
  
  // Construir JSON usando sprintf para evitar String ✅
  snprintf(httpBuffer, BUFFER_SIZE,
    "{"
    "\"panel\":{"
      "\"voltaje\":%.2f,"
      "\"corriente\":%.3f,"
      "\"potencia\":%.2f,"
      "\"led\":\"%s\""
    "},"
    "\"bateria\":{"
      "\"voltaje\":%.2f,"
      "\"corriente\":%.3f,"
      "\"potencia\":%.2f,"
      "\"estado\":\"%s\","
      "\"led\":\"%s\""
    "},"
    "\"carga\":{"
      "\"voltaje\":%.2f,"
      "\"corriente\":%.3f,"
      "\"potencia\":%.2f,"
      "\"led\":\"%s\""
    "},"
    "\"sistema\":{"
      "\"ethernet\":%s,"
      "\"sd\":%s"
    "}"
    "}",
    voltajePanel, corrientePanel, voltajePanel * corrientePanel,
    (voltajePanel > 14.0) ? "verde" : "rojo",
    
    voltajeBateria, corrienteBateria, fabs(voltajeBateria * corrienteBateria),
    (corrienteBateria >= 0) ? "cargando" : "descargando",
    (voltajeBateria > 12.5) ? "verde" : (voltajeBateria < 11.8) ? "rojo" : "amarillo",
    
    voltajeCarga, corrienteCarga, voltajeCarga * corrienteCarga,
    (corrienteCarga < 1.0) ? "verde" : "rojo",
    
    ethernetOK ? "true" : "false",
    sdOK ? "true" : "false"
  );
  
  client.print(httpBuffer);
}

void servirArchivo(EthernetClient client, const char* filename) {
  if (!sdOK) {
    enviarHTMLBasico(client);
    return;
  }
  
  File file = SD.open(filename);
  
  if (file) {
    // Determinar el tipo de contenido
    if (strstr(filename, ".html") != NULL) {
      client.println(F("HTTP/1.1 200 OK"));
      client.println(F("Content-Type: text/html"));
    } else if (strstr(filename, ".css") != NULL) {
      client.println(F("HTTP/1.1 200 OK"));
      client.println(F("Content-Type: text/css"));
    } else if (strstr(filename, ".js") != NULL) {
      client.println(F("HTTP/1.1 200 OK"));
      client.println(F("Content-Type: application/javascript"));
    } else {
      client.println(F("HTTP/1.1 200 OK"));
      client.println(F("Content-Type: application/octet-stream"));
    }
    
    client.println(F("Connection: close"));
    client.println();
    
    // Enviar el archivo en chunks para optimizar memoria
    while (file.available()) {
      int bytesRead = file.read(httpBuffer, min(BUFFER_SIZE - 1, file.available()));
      client.write((uint8_t*)httpBuffer, bytesRead);
    }
    
    file.close();
  } else {
    enviar404(client);
  }
}

void enviar404(EthernetClient client) {
  client.println(F("HTTP/1.1 404 Not Found"));
  client.println(F("Content-Type: text/plain"));
  client.println(F("Connection: close"));
  client.println();
  client.println(F("404 - Archivo no encontrado"));
}

void enviarHTMLBasico(EthernetClient client) {
  // HTML básico cuando SD no está disponible
  client.println(F("HTTP/1.1 200 OK"));
  client.println(F("Content-Type: text/html"));
  client.println(F("Connection: close"));
  client.println();
  
  client.println(F("<!DOCTYPE html>"));
  client.println(F("<html><head><title>Monitor Solar</title>"));
  client.println(F("<meta http-equiv='refresh' content='5'>"));
  client.println(F("<style>body{font-family:Arial;margin:20px;}"));
  client.println(F(".sensor{border:1px solid #ccc;margin:10px;padding:15px;border-radius:5px;}"));
  client.println(F(".verde{background-color:#90EE90;}.rojo{background-color:#FFB6C1;}"));
  client.println(F("</style></head><body>"));
  client.println(F("<h1>Sistema de Monitoreo Solar</h1>"));
  
  // Panel Solar
  snprintf(httpBuffer, BUFFER_SIZE,
    "<div class='sensor %s'><h3>Panel Solar</h3>"
    "<p>Voltaje: %.2fV</p><p>Corriente: %.3fA</p><p>Potencia: %.2fW</p></div>",
    (voltajePanel > 14.0) ? "verde" : "rojo",
    voltajePanel, corrientePanel, voltajePanel * corrientePanel);
  client.print(httpBuffer);
  
  // Batería
  const char* estadoBat = (corrienteBateria >= 0) ? "Cargando" : "Descargando";
  const char* colorBat = (voltajeBateria > 12.5) ? "verde" : "rojo";
  snprintf(httpBuffer, BUFFER_SIZE,
    "<div class='sensor %s'><h3>Batería (%s)</h3>"
    "<p>Voltaje: %.2fV</p><p>Corriente: %.3fA</p><p>Potencia: %.2fW</p></div>",
    colorBat, estadoBat,
    voltajeBateria, corrienteBateria, fabs(voltajeBateria * corrienteBateria));
  client.print(httpBuffer);
  
  // Carga
  snprintf(httpBuffer, BUFFER_SIZE,
    "<div class='sensor %s'><h3>Carga</h3>"
    "<p>Voltaje: %.2fV</p><p>Corriente: %.3fA</p><p>Potencia: %.2fW</p></div>",
    (corrienteCarga < 1.0) ? "verde" : "rojo",
    voltajeCarga, corrienteCarga, voltajeCarga * corrienteCarga);
  client.print(httpBuffer);
  
  client.println(F("<p><small>Tarjeta SD no disponible - Modo básico</small></p>"));
  client.println(F("</body></html>"));
}
