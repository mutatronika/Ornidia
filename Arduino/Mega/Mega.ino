#include <SPI.h>
#include <Ethernet.h>
#include <SD.h>
#include <math.h>  // Para usar fabs()

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

// Variables para control de estado
bool estadoEthernet = false;
bool estadoCarga = false;
unsigned long ultimoTiempoMedicion = 0;
const unsigned long INTERVALO_MEDICION = 1000;

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
  
  // Inicializar Ethernet con validación
  if (!Ethernet.begin(mac)) {
    Serial.println("Error al inicializar Ethernet");
    while(1) {
      digitalWrite(LED_PANEL_ROJO, HIGH);
      delay(500);
      digitalWrite(LED_PANEL_ROJO, LOW);
      delay(500);
    }
  }
  
  // Verificar conexión
  delay(1000);
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Error: No hay conexión Ethernet");
    while(1) {
      digitalWrite(LED_PANEL_ROJO, HIGH);
      delay(1000);
      digitalWrite(LED_PANEL_ROJO, LOW);
      delay(1000);
    }
  }
  
  estadoEthernet = true;
  Serial.println("Ethernet conectado");
  Serial.print("Dirección IP: ");
  Serial.println(Ethernet.localIP());
  
  // Inicializar tarjeta SD
  if (!SD.begin(4)) {
    Serial.println("Error al inicializar la tarjeta SD");
    return;
  }
  
  // Iniciar servidor
  server.begin();
}

void loop() {
  // Verificar conexión Ethernet
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Error: Conexión Ethernet perdida");
    estadoEthernet = false;
    while(1) {
      digitalWrite(LED_PANEL_ROJO, HIGH);
      delay(1000);
      digitalWrite(LED_PANEL_ROJO, LOW);
      delay(1000);
    }
  }
  
  // Leer sensores cada INTERVALO_MEDICION ms
  if (millis() - ultimoTiempoMedicion >= INTERVALO_MEDICION) {
    leerSensores();
    controlarLEDs();
    ultimoTiempoMedicion = millis();
  }
  
  // Manejar clientes web
  manejarClienteWeb();
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

  // Determinar estado de carga/descarga
  if (corrienteBateria > 0.1) {
    estadoCarga = true;
  } else if (corrienteBateria < -0.1) {
    estadoCarga = false;
  }
  
  // Mostrar valores por serial
  Serial.print("Panel - V: ");
  Serial.print(voltajePanel);
  Serial.print("V, I: ");
  Serial.print(corrientePanel);
  Serial.println("A");
  
  Serial.print("Batería - V: ");
  Serial.print(voltajeBateria);
  Serial.print("V, I: ");
  Serial.print(corrienteBateria);
  Serial.print("A, ");
  Serial.println(estadoCarga ? "Cargando" : "Descargando");
}

float leerCorriente(int pin) {
  float suma = 0;
  int muestrasValidas = 0;
  
  for (int i = 0; i < MUESTRAS && muestrasValidas < MUESTRAS/2; i++) {
    int valor = analogRead(pin);
    float voltaje = (valor / 1023.0) * 5000; // en mV
    
    // Validación de datos
    if (voltaje > 0 && voltaje < 5000) {
      float corriente = ((voltaje - OFFSET_VOLTAJE) / MV_POR_AMP);
      suma += sq(corriente);
      muestrasValidas++;
    }
  }
  
  return (muestrasValidas > 0) ? sqrt(suma / muestrasValidas) : 0;
}

void controlarLEDs() {
  // Controlar LEDs de batería
  digitalWrite(LED_BAT_VERDE, voltajeBateria > 12.0);
  digitalWrite(LED_BAT_ROJO, voltajeBateria <= 12.0);
  
  // Controlar LEDs de carga
  digitalWrite(LED_CARGA_VERDE, estadoCarga);
  digitalWrite(LED_CARGA_ROJO, !estadoCarga);
  
  // Controlar LEDs de panel
  digitalWrite(LED_PANEL_VERDE, voltajePanel > 14.0);
  digitalWrite(LED_PANEL_ROJO, voltajePanel <= 14.0);
}

void manejarClienteWeb() {
  EthernetClient client = server.available();
  if (client) {
    char buffer[100];
    bool currentLineIsBlank = true;
    
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        
        // Usar buffer en lugar de String
        static int pos = 0;
        if (pos < sizeof(buffer) - 1) {
          buffer[pos++] = c;
        }
        
        if (c == '\n' && currentLineIsBlank) {
          // Responder a la ruta /data
          if (strstr(buffer, "GET /data") != NULL) {
            enviarDatosJSON(client);
          } 
          // Servir archivos estáticos
          else if (strstr(buffer, "GET /") != NULL) {
            servirArchivo(client, "/index.html");
          } 
          else if (strstr(buffer, "GET /style.css") != NULL) {
            servirArchivo(client, "/style.css");
          } 
          else if (strstr(buffer, "GET /script.js") != NULL) {
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
  
  char json[200];
  snprintf(json, sizeof(json), 
    "{\"panel\":{\"voltaje\":%.2f,\"corriente\":%.3f,\"potencia\":%.2f,\"led\":\"%s\"},"
    "\"bateria\":{\"voltaje\":%.2f,\"corriente\":%.3f,\"potencia\":%.2f,\"led\":\"%s\"},"
    "\"carga\":{\"voltaje\":%.2f,\"corriente\":%.3f,\"potencia\":%.2f,\"led\":\"%s\"}}",
    voltajePanel, corrientePanel, voltajePanel * corrientePanel,
    (voltajePanel > 14.0) ? "verde" : "rojo",
    voltajeBateria, corrienteBateria, voltajeBateria * corrienteBateria,
    (voltajeBateria > 12.0) ? "verde" : "rojo",
    voltajeCarga, corrienteCarga, voltajeCarga * corrienteCarga,
    estadoCarga ? "verde" : "rojo"
  );
  
  client.println(json);
}

void servirArchivo(EthernetClient client, const char* filename) {
  File file = SD.open(filename);
  
  if (file) {
    // Determinar el tipo de contenido
    const char* contentType = NULL;
    if (strstr(filename, ".html") != NULL) {
      contentType = "text/html";
    } else if (strstr(filename, ".css") != NULL) {
      contentType = "text/css";
    } else if (strstr(filename, ".js") != NULL) {
      contentType = "application/javascript";
    }
    
    // Enviar headers
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/plain");
    if (contentType) {
      client.println(contentType);
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
