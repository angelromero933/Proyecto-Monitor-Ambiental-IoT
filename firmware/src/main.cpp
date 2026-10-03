#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <SD.h>
#include "config.h"
#include "sensores.h"

// Variables globales para Enviar Json
unsigned long lastMsg = 0;
const unsigned long INTERVALO = 30000;

//Variables globales para leer sensoresy controlar leds
const unsigned long INTERVALO_LECTURA = 2000;
unsigned long lastRead = 0;

Sensores *sensores = nullptr;
WiFiClientSecure espClient;
PubSubClient client(espClient);

bool datosGuardar = false;

void conectarWiFi();
void reconectarMQTT();
void callback(char* topic, byte* payload, unsigned int length);
void leerMensajesGuardados();
void guardarEnSD(const char* datos);
void controlarLEDs();

void setup() {    
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n=== SISTEMA DE MONITOREO AMBIENTAL ===\n");
    
  conectarWiFi();

  sensores = new Sensores();
  if(sensores == nullptr){
    Serial.println("Error al crear objeto Sensores");
    ESP.restart();
  }
  sensores->inicializarSensores();
   
  espClient.setInsecure(); 
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
  client.setBufferSize(512);                          //amplio el tamaño para enviar el json + los paquetes mqtt
  client.setKeepAlive(60);                            //Mantengo la conexion por mas tiempo, ya que se desconecta del broker si mo hay act.
  client.setSocketTimeout(30);
     
  reconectarMQTT();
    
  Serial.println("\nSistema listo. Monitoreando ...\n");
  delay(3000);
}

void loop() {
  if (!client.connected()) {
    reconectarMQTT();
  }
  client.loop();

  if (sensores == nullptr) return;
  digitalWrite(LED_AZUL_PIN, HIGH);
  unsigned long ahora = millis();

  // Lectura + semaforo cada 2 seg
  if (ahora - lastRead >= INTERVALO_LECTURA) {
    lastRead = ahora;
    sensores->leerSensores();
    controlarLEDs();
  }

  // Publicación cada 30 seg
  if (ahora - lastMsg >= INTERVALO) {
    lastMsg = ahora;

    StaticJsonDocument<256> doc;
    doc["Temperatura"] = serialized(String(sensores->obtenerTemperatura(), 1));
    doc["Humedad"]     = serialized(String(sensores->obtenerHumedad(), 1));
    doc["Presion"]     = serialized(String(sensores->obtenerPresion(), 2));
    doc["Fecha"]       = sensores->obtenerReloj();
    doc["Gas-Ppm"]     = sensores->obtenerGas();
    doc["creator"]     = creator;

    char buffer[256];
    serializeJson(doc, buffer);

    if (client.publish(mqtt_topic, buffer)) {
      Serial.print("Publicado: ");
      Serial.println(buffer);
      if (datosGuardar) {
        leerMensajesGuardados();
      }
    } else {
      Serial.println("Error al publicar, guardando en SD...");
      guardarEnSD(buffer);
    }
  }
}

void conectarWiFi() {
  Serial.print("Conectando a Wi-Fi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
}

void reconectarMQTT() {
    while (!client.connected()) {
        Serial.print("Conectando a HiveMQ...");
        
        if (client.connect("ESP32_001", mqtt_user, mqtt_password)) {
            Serial.println(" MQTT conectado");
            client.subscribe(mqtt_topic);
            Serial.println("   Suscrito a: " + String(mqtt_topic));
        } else {
            Serial.print(" error rc=");
            Serial.print(client.state());
            Serial.println(" reintentando en 5s...");
            delay(5000);
        }
    }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensaje recibido en: ");
  Serial.print(topic);
  Serial.print(" -> ");
  
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);
}

void guardarEnSD(const char* datos) {
  if (!sensores->obtenerEstadoSD()) return;
  File file = SD.open(filename, FILE_APPEND);
  if (file) {
    file.println(datos);
    file.close();
    datosGuardar = true;
    Serial.println("Datos guardados en SD");
  }else {
    Serial.println("Error al abrir archivo SD");
  }
}

void leerMensajesGuardados() {
  File file = SD.open(filename, FILE_READ);

  if (!file){
    Serial.println("No hay archivo en SD");
    datosGuardar = false;
    return;
  }

  Serial.println("Leyendo datos de SD...");
  int enviados = 0;

  while (file.available()){
    String linea = file.readStringUntil('\n');
    linea.trim();

    if (linea.length() == 0)
      continue;

    StaticJsonDocument<256> doc;
    if (deserializeJson(doc, linea)){
      Serial.println("JSON inválido, saltando...");
      continue;
    }

    if (client.publish(mqtt_topic, linea.c_str())){
      Serial.println(linea);
      enviados++;
      delay(100); 
    }else{
      Serial.println("Error al enviar, deteniendo");
      break;
    }
  }
  file.close();

  if (enviados > 0){
    if (SD.remove(filename)){
      Serial.println(String(enviados) + " datos enviados. SD vaciada.");
      datosGuardar = false;
    }else{
      Serial.println("Error al borrar archivo SD");
    }
  }
}

void controlarLEDs() {
  Nivel n = sensores->nivelGlobal();

  digitalWrite(LED_VERDE_PIN,    n == Nivel::NORMAL);
  digitalWrite(LED_AMARILLO_PIN, n == Nivel::ADVERTENCIA);
  digitalWrite(LED_ROJO_PIN,     n == Nivel::PELIGRO);
  digitalWrite(BUZZER_PIN,       n == Nivel::PELIGRO);
}