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

// Configuración de red
byte mac[] = { 0xDE, 0xA5, 0x2E, 0xEF, 0xF0, 0xED };
IPAddress ip(192, 168, 1, 140);
EthernetServer server(80);

// Variables para almacenar las mediciones
float corrientePanel = 0;
float corrienteBateria = 0;
float corrienteCarga = 0;
float voltajePanel = 0;
float voltajeBateria = 0;
float voltajeCarga = 0;

void setup() {
  // Inicializar comunicación serial
  Serial.begin(9600);
  
  // Configurar pines de los LEDs
  pinMode(LED_BAT_VERDE, OUTPUT);
  pinMode(LED_BAT_ROJO, OUTPUT);
  pinMode(LED_CARGA_VERDE, OUTPUT);
  pinMode(LED_CARGA_ROJO, OUTPUT);
  pinMode(LED_PANEL_VERDE, OUTPUT);
  pinMode(LED_PANEL_ROJO, OUTPUT);
  
  // Inicializar Ethernet
  Ethernet.begin(mac, ip);
  
  // Inicializar tarjeta SD
  if (!SD.begin(4)) {
    Serial.println("Error al inicializar la tarjeta SD");
    return;
  }
  
  // Iniciar servidor
  server.begin();
  Serial.print("Servidor en: ");
  Serial.println(Ethernet.localIP());
}

void loop() {
  // Leer sensores
  leerSensores();
  
  // Controlar LEDs
  controlarLEDs();
  
  // Manejar clientes web
  manejarClienteWeb();
  
  delay(100); // Pequeña pausa para estabilidad
}

void leerSensores() {
  // Leer corriente en los tres sensores
  corrientePanel = leerCorriente(PIN_PANEL);
  corrienteBateria = leerCorriente(PIN_BATERIA);
  corrienteCarga = leerCorriente(PIN_CARGA);
  
  // Leer voltajes
  voltajePanel = leerVoltaje(PIN_VOLTAJE_PANEL) * RELACION_DIVISOR;
  voltajeBateria = leerVoltaje(PIN_VOLTAJE_BATERIA) * RELACION_DIVISOR;
  voltajeCarga = leerVoltaje(PIN_VOLTAJE_CARGA) * RELACION_DIVISOR;
  
  // Mostrar valores por serial (opcional)
  Serial.print("Panel - V: ");
  Serial.print(voltajePanel);
  Serial.print("V, I: ");
  Serial.print(corrientePanel);
  Serial.println("A");
}

float leerCorriente(int pin) {
  float suma = 0;
  for (int i = 0; i < MUESTRAS; i++) {
    int valor = analogRead(pin);
    // Convertir a voltaje (0-5V)
    float voltaje = (valor / 1023.0) * 5000; // en mV
    // Convertir a corriente (restar offset y dividir por sensibilidad)
    float corriente = ((voltaje - OFFSET_VOLTAJE) / MV_POR_AMP);
    suma += sq(corriente);
    delay(1);
  }
  // Calcular valor RMS
  float corriente_rms = sqrt(suma / MUESTRAS);
  return abs(corriente_rms); // Valor absoluto para corriente
}

float leerVoltaje(int pin) {
  float suma = 0;
  for (int i = 0; i < 100; i++) {
    suma += analogRead(pin);
    delay(1);
  }
  float promedio = suma / 100.0;
  // Convertir a voltaje (0-5V)
  return (promedio * 5.0) / 1023.0;
}

void controlarLEDs() {
  // Controlar LEDs de batería
  digitalWrite(LED_BAT_VERDE, voltajeBateria > 12.0);
  digitalWrite(LED_BAT_ROJO, voltajeBateria <= 12.0);
  
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
    String request = "";
    
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        request += c;
        
        if (c == '\n' && currentLineIsBlank) {
          // Responder a la ruta /data
          if (request.indexOf("GET /data") != -1) {
            enviarDatosJSON(client);
          } 
          // Servir archivos estáticos
          else if (request.indexOf("GET /") != -1) {
            servirArchivo(client, "/index.html");
          } 
          else if (request.indexOf("GET /style.css") != -1) {
            servirArchivo(client, "/style.css");
          } 
          else if (request.indexOf("GET /script.js") != -1) {
            servirArchivo(client, "/script.js");
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
  // Crear objeto JSON con los datos
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json");
  client.println("Connection: close");
  client.println();
  
  client.print("{\"panel\":{\"voltaje\":");
  client.print(voltajePanel, 2);
  client.print(",\"corriente\":");
  client.print(corrientePanel, 3);
  client.print(",\"potencia\":");
  client.print(voltajePanel * corrientePanel, 2);
  client.print(",\"led\":\"");
  client.print((voltajePanel > 14.0) ? "verde" : "rojo");
  client.print("\"},\"bateria\":{\"voltaje\":");
  client.print(voltajeBateria, 2);
  client.print(",\"corriente\":");
  client.print(corrienteBateria, 3);
  client.print(",\"potencia\":");
  client.print(voltajeBateria * corrienteBateria, 2);
  client.print(",\"led\":\"");
  client.print((voltajeBateria > 12.0) ? "verde" : "rojo");
  client.print("\"},\"carga\":{\"voltaje\":");
  client.print(voltajeCarga, 2);
  client.print(",\"corriente\":");
  client.print(corrienteCarga, 3);
  client.print(",\"potencia\":");
  client.print(voltajeCarga * corrienteCarga, 2);
  client.print(",\"led\":\"");
  client.print((corrienteCarga < 1.0) ? "verde" : "rojo");
  client.println("\"}}");
}

void servirArchivo(EthernetClient client, String filename) {
  File file = SD.open(filename);
  
  if (file) {
    // Determinar el tipo de contenido
    if (filename.endsWith(".html")) {
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: text/html");
    } else if (filename.endsWith(".css")) {
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: text/css");
    } else if (filename.endsWith(".js")) {
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: application/javascript");
    } else {
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: application/octet-stream");
    }
    
    client.println("Connection: close");
    client.println();
    
    // Enviar el archivo
    while (file.available()) {
      client.write(file.read());
    }
    
    file.close();
  } else {
    // Archivo no encontrado
      client.println("HTTP/1.1 404 Not Found");
    client.println("Content-Type: text/plain");
    client.println("Connection: close");
    client.println();
    client.println("404 - Archivo no encontrado");
  }
}
