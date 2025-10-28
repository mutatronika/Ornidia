#include <SPI.h>
#include <Ethernet.h>
#include <SD.h>
// tamaño del búfer utilizado para capturar solicitudes HTTP
#define REQ_BUF_SZ   60

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
#define REQ_BUF_SZ 200
char HTTP_req[REQ_BUF_SZ] = {0};
char req_index = 0;

// Configuración de red
byte mac[] = { 0xDE, 0xA5, 0x2E, 0xEF, 0xF0, 0xED };
IPAddress ip(192, 168, 1, 140);
EthernetServer server(80);
File webFile;
char HTTP_req[REQ_BUF_SZ] = {0}; // buffered HTTP request stored as null terminated string
char req_index = 0;              // index into HTTP_req buffer
boolean LED_state[4] = {0}; // stores the states of the LEDs

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
  
  // ✅ CRÍTICO: Configurar pin 10 para shield W5100
  pinMode(10, OUTPUT);
  digitalWrite(10, HIGH);  // Activar SD primero
  
  // ✅ Inicializar SD ANTES que Ethernet
  Serial.print(F("Inicializando SD... "));
  if (!SD.begin(4)) {
    Serial.println(F("ERROR - SD no inicializada"));
 //   sdOK = false;
  return;    // init failed
  } //else {
    // Verificar archivos necesarios
    if (!SD.exists("index.htm")) {
      Serial.println(F("ERROR - No se encuentra index.htm"));
 //     sdOK = false;
  return;    //can't find index file
    //} else {
      Serial.println(F("SD OK - index.htm encontrado"));
  //    sdOK = true;
    }
  //}
  
  // Configurar pines de los LEDs
  pinMode(LED_BAT_VERDE, OUTPUT);
  pinMode(LED_BAT_ROJO, OUTPUT);
  pinMode(LED_CARGA_VERDE, OUTPUT);
  pinMode(LED_CARGA_ROJO, OUTPUT);
  pinMode(LED_PANEL_VERDE, OUTPUT);
  pinMode(LED_PANEL_ROJO, OUTPUT);
  
  // Test inicial de LEDs
  testLEDs();
  
  // ✅ Cambiar a Ethernet DESPUÉS de SD
  digitalWrite(10, LOW);   // Activar Ethernet
  
  // Inicializar Ethernet con validación
  Serial.print(F("Configurando Ethernet... "));
  if (Ethernet.begin(mac) == 0) {
    Serial.println(F("DHCP falló, usando IP estática"));
    Ethernet.begin(mac, ip);
  }
  
  // Verificar estado de Ethernet
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println(F("ERROR: Shield Ethernet no encontrado"));
    ethernetOK = false;
  } else if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println(F("ERROR: Cable Ethernet desconectado"));
    ethernetOK = false;
  } else {
    ethernetOK = true;
    Serial.print(F("Ethernet OK - IP: "));
    Serial.println(Ethernet.localIP());
  }
  
  // Iniciar servidor solo si Ethernet funciona
  if (ethernetOK) {
    server.begin();
    Serial.println(F("Servidor web iniciado"));
  }
  
  Serial.println(F("Sistema listo"));
}

void loop() {
  // Leer sensores
  leerSensores();
  
  // Controlar LEDs
  controlarLEDs();
  
  // Manejar clientes web solo si Ethernet funciona
  if (ethernetOK) {
    manejarClienteWeb();
  }
  
  delay(100);
}

void testLEDs() {
  // Test de LEDs al inicio
  int leds[] = {LED_BAT_VERDE, LED_BAT_ROJO, LED_CARGA_VERDE, LED_CARGA_ROJO, LED_PANEL_VERDE, LED_PANEL_ROJO};
  
  for (int i = 0; i < 6; i++) {
    digitalWrite(leds[i], HIGH);
    delay(200);
    digitalWrite(leds[i], LOW);
  }
}

void leerSensores() {
  // Leer corriente en los tres sensores
  corrientePanel = leerCorriente(PIN_PANEL);
  corrienteBateria = leerCorrienteBidireccional(PIN_BATERIA);
  corrienteCarga = leerCorriente(PIN_CARGA);
  
  // Leer voltajes
  voltajePanel = leerVoltaje(PIN_VOLTAJE_PANEL) * RELACION_DIVISOR;
  voltajeBateria = leerVoltaje(PIN_VOLTAJE_BATERIA) * RELACION_DIVISOR;
  voltajeCarga = leerVoltaje(PIN_VOLTAJE_CARGA) * RELACION_DIVISOR;
  
  // Mostrar valores por serial
  Serial.print(F("Panel V:"));
  Serial.print(voltajePanel, 2);
  Serial.print(F("V I:"));
  Serial.print(corrientePanel, 3);
  Serial.print(F("A | Bat V:"));
  Serial.print(voltajeBateria, 2);
  Serial.print(F("V I:"));
  Serial.print(corrienteBateria, 3);
  Serial.println(corrienteBateria >= 0 ? F("A(+)") : F("A(-)"));
}

float leerCorriente(int pin) {
  float suma = 0;
  for (int i = 0; i < MUESTRAS; i++) {
    int valor = analogRead(pin);
    float voltaje = (valor / 1023.0) * 5000; // en mV
    float corriente = ((voltaje - OFFSET_VOLTAJE) / MV_POR_AMP);
    suma += corriente * corriente;
    delayMicroseconds(100);
  }
  float corriente_rms = sqrt(suma / MUESTRAS);
  return fabs(corriente_rms); // ✅ Corregido: fabs() para flotantes
}

float leerCorrienteBidireccional(int pin) {
  float suma = 0;
  float sumaInstantanea = 0;
  int muestrasValidas = 0;
  
  for (int i = 0; i < MUESTRAS; i++) {
    int valor = analogRead(pin);
    
    if (valor >= 0 && valor <= 1023) {
      float voltaje = (valor / 1023.0) * 5000; // en mV
      float corriente = ((voltaje - OFFSET_VOLTAJE) / MV_POR_AMP);
      
      suma += corriente * corriente;
      sumaInstantanea += corriente;
      muestrasValidas++;
    }
    delayMicroseconds(100);
  }
  
  if (muestrasValidas == 0) return 0;
  
  float corriente_rms = sqrt(suma / muestrasValidas);
  float promedio = sumaInstantanea / muestrasValidas;
  
  if (corriente_rms < 0.05) return 0; // Filtrar ruido
  
  return (promedio >= 0) ? corriente_rms : -corriente_rms;
}

float leerVoltaje(int pin) {
  float suma = 0;
  for (int i = 0; i < 100; i++) {
    suma += analogRead(pin);
    delayMicroseconds(100);
  }
  float promedio = suma / 100.0;
  return (promedio * 5.0) / 1023.0;
}

void controlarLEDs() {
  // LEDs de batería con umbrales mejorados
  if (voltajeBateria > 12.5) {
    digitalWrite(LED_BAT_VERDE, HIGH);
    digitalWrite(LED_BAT_ROJO, LOW);
  } else if (voltajeBateria < 11.8) {
    digitalWrite(LED_BAT_VERDE, LOW);
    digitalWrite(LED_BAT_ROJO, HIGH);
  } else {
    // Estado intermedio - parpadeo
    static unsigned long ultimoParpadeo = 0;
    static bool estadoLED = false;
    if (millis() - ultimoParpadeo > 500) {
      estadoLED = !estadoLED;
      digitalWrite(LED_BAT_VERDE, LOW);
      digitalWrite(LED_BAT_ROJO, estadoLED);
      ultimoParpadeo = millis();
    }
  }
  
  // LEDs de carga
  digitalWrite(LED_CARGA_VERDE, corrienteCarga < 1.0);
  digitalWrite(LED_CARGA_ROJO, corrienteCarga >= 1.0);
  
  // LEDs de panel
  digitalWrite(LED_PANEL_VERDE, voltajePanel > 14.0);
  digitalWrite(LED_PANEL_ROJO, voltajePanel <= 14.0);
}

void manejarClienteWeb() {
  EthernetClient client = server.available();
  if (client) {
    boolean currentLineIsBlank = true;
    
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        
        // ✅ Buffer fijo en lugar de String
        if (req_index < (REQ_BUF_SZ - 1)) {
          HTTP_req[req_index] = c;
          req_index++;
        }
        
        if (c == '\n' && currentLineIsBlank) {
          // Verificar tipo de petición usando strstr
          if (StrContains(HTTP_req, "ajax_inputs") || StrContains(HTTP_req, "GET /data")) {
            enviarDatosJSON(client);
          }
          else if (StrContains(HTTP_req, "GET /") && !StrContains(HTTP_req, "GET /data")) {
            servirArchivo(client, "index.htm"); // ✅ Cambiado a .htm
          }
          else if (StrContains(HTTP_req, "style.css")) {
            servirArchivo(client, "style.css");
          }
          else if (StrContains(HTTP_req, "script.js")) {
            servirArchivo(client, "script.js");
          }
          else {
            enviar404(client);
          }
          
          // ✅ Limpiar buffer como en el código que funciona
          req_index = 0;
          StrClear(HTTP_req, REQ_BUF_SZ);
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
  client.println(F("HTTP/1.1 200 OK"));
  client.println(F("Content-Type: application/json"));
  client.println(F("Connection: close"));
  client.println();
  
  // JSON optimizado
  client.print(F("{\"panel\":{\"voltaje\":"));
  client.print(voltajePanel, 2);
  client.print(F(",\"corriente\":"));
  client.print(corrientePanel, 3);
  client.print(F(",\"potencia\":"));
  client.print(voltajePanel * corrientePanel, 2);
  client.print(F(",\"led\":\""));
  client.print((voltajePanel > 14.0) ? F("verde") : F("rojo"));
  
  client.print(F("\"},\"bateria\":{\"voltaje\":"));
  client.print(voltajeBateria, 2);
  client.print(F(",\"corriente\":"));
  client.print(corrienteBateria, 3);
  client.print(F(",\"potencia\":"));
  client.print(fabs(voltajeBateria * corrienteBateria), 2);
  client.print(F(",\"estado\":\""));
  client.print((corrienteBateria >= 0) ? F("cargando") : F("descargando"));
  client.print(F("\",\"led\":\""));
  client.print((voltajeBateria > 12.5) ? F("verde") : (voltajeBateria < 11.8) ? F("rojo") : F("amarillo"));
  
  client.print(F("\"},\"carga\":{\"voltaje\":"));
  client.print(voltajeCarga, 2);
  client.print(F(",\"corriente\":"));
  client.print(corrienteCarga, 3);
  client.print(F(",\"potencia\":"));
  client.print(voltajeCarga * corrienteCarga, 2);
  client.print(F(",\"led\":\""));
  client.print((corrienteCarga < 1.0) ? F("verde") : F("rojo"));
  
  client.print(F("\"},\"sistema\":{\"ethernet\":"));
  client.print(ethernetOK ? F("true") : F("false"));
  client.print(F(",\"sd\":"));
  client.print(sdOK ? F("true") : F("false"));
  client.println(F("}}"));
}

void servirArchivo(EthernetClient client, const char* filename) {
  if (!sdOK) {
    enviarHTMLBasico(client);
    return;
  }
  
  // ✅ Activar SD para leer archivo
  digitalWrite(10, HIGH);
  webFile = SD.open(filename);
  
  if (webFile) {
    client.println(F("HTTP/1.1 200 OK"));
    
    // Determinar tipo de contenido
    if (strstr(filename, ".htm") != NULL) {
      client.println(F("Content-Type: text/html"));
    } else if (strstr(filename, ".css") != NULL) {
      client.println(F("Content-Type: text/css"));
    } else if (strstr(filename, ".js") != NULL) {
      client.println(F("Content-Type: application/javascript"));
    }
    
    client.println(F("Connection: close"));
    client.println();
    
    // Enviar archivo
    while (webFile.available()) {
      client.write(webFile.read());
    }
    webFile.close();
  } else {
    enviar404(client);
  }
  
  // ✅ Reactivar Ethernet
  digitalWrite(10, LOW);
}

void enviar404(EthernetClient client) {
  client.println(F("HTTP/1.1 404 Not Found"));
  client.println(F("Content-Type: text/html"));
  client.println(F("Connection: close"));
  client.println();
  client.println(F("<h1>404 - Archivo no encontrado</h1>"));
  client.println(F("<p>Verifica que index.htm existe en la SD</p>"));
}

void enviarHTMLBasico(EthernetClient client) {
  client.println(F("HTTP/1.1 200 OK"));
  client.println(F("Content-Type: text/html"));
  client.println(F("Connection: close"));
  client.println();
  
  client.println(F("<!DOCTYPE html><html><head>"));
  client.println(F("<title>Monitor Solar</title>"));
  client.println(F("<meta http-equiv='refresh' content='5'>"));
  client.println(F("<style>"));
  client.println(F("body{font-family:Arial;margin:20px;background:#f0f0f0;}"));
  client.println(F(".sensor{border:2px solid #333;margin:10px;padding:15px;border-radius:8px;background:white;}"));
  client.println(F(".verde{border-color:#4CAF50;}.rojo{border-color:#F44336;}"));
  client.println(F("h1{color:#333;text-align:center;}"));
  client.println(F("h3{color:#666;margin-top:0;}"));
  client.println(F("</style></head><body>"));
  
  client.println(F("<h1>🌞 Sistema de Monitoreo Solar</h1>"));
  
  // Panel Solar
  client.print(F("<div class='sensor "));
  client.print((voltajePanel > 14.0) ? F("verde'>") : F("rojo'>"));
  client.println(F("<h3>📱 Panel Solar</h3>"));
  client.print(F("<p><strong>Voltaje:</strong> "));
  client.print(voltajePanel, 2);
  client.println(F(" V</p>"));
  client.print(F("<p><strong>Corriente:</strong> "));
  client.print(corrientePanel, 3);
  client.println(F(" A</p>"));
  client.print(F("<p><strong>Potencia:</strong> "));
  client.print(voltajePanel * corrientePanel, 2);
  client.println(F(" W</p></div>"));
  
  // Batería
  client.print(F("<div class='sensor "));
  client.print((voltajeBateria > 12.5) ? F("verde'>") : F("rojo'>"));
  client.println(F("<h3>🔋 Batería</h3>"));
  client.print(F("<p><strong>Voltaje:</strong> "));
  client.print(voltajeBateria, 2);
  client.println(F(" V</p>"));
  client.print(F("<p><strong>Corriente:</strong> "));
  client.print(corrienteBateria, 3);
  client.print(F(" A ("));
  client.print((corrienteBateria >= 0) ? F("Cargando") : F("Descargando"));
  client.println(F(")</p>"));
  client.print(F("<p><strong>Potencia:</strong> "));
  client.print(fabs(voltajeBateria * corrienteBateria), 2);
  client.println(F(" W</p></div>"));
  
  // Carga
  client.print(F("<div class='sensor "));
  client.print((corrienteCarga < 1.0) ? F("verde'>") : F("rojo'>"));
  client.println(F("<h3>⚡ Carga</h3>"));
  client.print(F("<p><strong>Voltaje:</strong> "));
  client.print(voltajeCarga, 2);
  client.println(F(" V</p>"));
  client.print(F("<p><strong>Corriente:</strong> "));
  client.print(corrienteCarga, 3);
  client.println(F(" A</p>"));
  client.print(F("<p><strong>Potencia:</strong> "));
  client.print(voltajeCarga * corrienteCarga, 2);
  client.println(F(" W</p></div>"));
  
  client.println(F("<p style='text-align:center;color:#666;'>"));
  client.println(F("⚠️ Modo básico - SD no disponible</p>"));
  client.println(F("</body></html>"));
}

// ✅ Funciones auxiliares del código que funciona
void StrClear(char *str, char length) {
  for (int i = 0; i < length; i++) {
    str[i] = 0;
  }
}

char StrContains(char *str, char *sfind) {
  char found = 0;
  char index = 0;
  char len = strlen(str);
  
  if (strlen(sfind) > len) {
    return 0;
  }
  
  while (index < len) {
    if (str[index] == sfind[found]) {
      found++;
      if (strlen(sfind) == found) {
        return 1;
      }
    } else {
      found = 0;
    }
    index++;
  }
  return 0;
}
