#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <SD.h>
#include "config.h"
#include "sensores.h"

unsigned long lastMsg = 0;
const unsigned long INTERVALO = 30000;
const unsigned long TIEMPO_AVISO  = 3000;  //leds tiempo de encendido cual alerta

unsigned long ledRojoHasta = 0;    // Timestamp de apagado
unsigned long ledAmarHasta = 0;
unsigned long ledAzulHasta = 0;
unsigned long buzzerHasta  = 0;

bool ledRojoOn = false;
bool ledAmarOn = false;
bool ledAzulOn = false;
bool buzzerOn  = false;

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
  
  pinMode(LED_ROJO_PIN, OUTPUT);
  pinMode(LED_VERDE_PIN, OUTPUT);
  pinMode(LED_AMARILLO_PIN, OUTPUT);
  pinMode(LED_AZUL_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
    
  pinMode(GAS_PIN, INPUT);
  pinMode(MQ135_DOUT_PIN, INPUT);
  
  Serial.println("\n=== SISTEMA DE MONITOREO AMBIENTAL ===\n");
    
  sensores = new Sensores();
  if(sensores == nullptr){
    Serial.println("Error al crear objeto Sensores");
    ESP.restart();
  }
  sensores->inicializarSensores();
    
  conectarWiFi();
   
  espClient.setInsecure(); 
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
  client.setBufferSize(512);
  client.setKeepAlive(60);
  client.setSocketTimeout(30);
     
  reconectarMQTT();
    
  if (!SD.begin(SD_CS_PIN)) {
      Serial.println("Error al inicializar SD (seguirá sin backup)");
  } else {
    Serial.println("SD inicializada");
  }
    
  Serial.println("\nSistema listo. Monitoreando...\n");
  delay(3000);
}

void loop() {
  if (!client.connected()) {
    reconectarMQTT();
  }
  client.loop();
  digitalWrite(LED_VERDE_PIN, HIGH);
  unsigned long ahora = millis();
  if (ahora - lastMsg >= INTERVALO) {
    lastMsg = ahora;
    if (sensores == nullptr)
      return;

    sensores->leerSensores();

    controlarLEDs();
    

    String creator = "86d8ce35-705a-4413-9791-9192e5470a76";

    StaticJsonDocument<256> doc;
    doc["Temperatura"] = sensores->obtenerTemperatura();
    doc["Humedad"] = sensores->obtenerHumedad();
    doc["Presion"] = sensores->obtenerPresion();
    doc["Fecha"] = sensores->obtenerReloj();
    doc["Gas-Ppm"] = sensores->obtenerGas();
    doc["creator"] = creator;

    char buffer[256];
    serializeJson(doc, buffer);

    if (client.publish(mqtt_topic, buffer)) {
      Serial.print("Publicado: ");
      Serial.println(buffer);

      if (datosGuardar){
        leerMensajesGuardados();
      }
    }else {
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
    Serial.println("Datos guardados en SD");
  }else {
    Serial.println("Error al abrir archivo SD");
  }
}

void leerMensajesGuardados() {
  File file = SD.open(filename, FILE_READ);

  if (!file){
    Serial.println("No hay archivo en SD");
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
    }else{
      Serial.println("Error al borrar archivo SD");
    }
  }
}

void controlarLEDs() {
  unsigned long ahora = millis();

  if (sensores->obtenerEstadoMQ()){
    ledRojoHasta = ahora + TIEMPO_AVISO;
    if (!ledRojoOn){
      digitalWrite(LED_ROJO_PIN, HIGH);
      ledRojoOn = true;
      Serial.println("LED Rojo encendido (gas)");
    }
  }
  
  if (ledRojoOn && ahora >= ledRojoHasta){
    digitalWrite(LED_ROJO_PIN, LOW);
    ledRojoOn = false;
    Serial.println("LED Rojo apagado");
  }

  if (sensores->obtenerEstadoDHT()){
    ledAmarHasta = ahora + TIEMPO_AVISO;
    if (!ledAmarOn){
      digitalWrite(LED_AMARILLO_PIN, HIGH);
      ledAmarOn = true;
      Serial.println("LED Amarillo encendido (DHT)");
    }
  }
  if (ledAmarOn && ahora >= ledAmarHasta){
    digitalWrite(LED_AMARILLO_PIN, LOW);
    ledAmarOn = false;
    Serial.println("LED Amarillo apagado");
  }

  if (sensores->obtenerEstadoBMP()){
    ledAzulHasta = ahora + TIEMPO_AVISO;
    if (!ledAzulOn){
      digitalWrite(LED_AZUL_PIN, HIGH);
      ledAzulOn = true;
      Serial.println("LED Azul encendido (BMP)");
    }
  }
  if (ledAzulOn && ahora >= ledAzulHasta){
    digitalWrite(LED_AZUL_PIN, LOW);
    ledAzulOn = false;
    Serial.println("LED Azul apagado");
  }

  bool alarma = (sensores->obtenerEstadoMQ() or sensores->obtenerEstadoDHT() or sensores->obtenerEstadoBMP());

  if (alarma){
    buzzerHasta = ahora + TIEMPO_AVISO;
    if (!buzzerOn){
      digitalWrite(BUZZER_PIN, HIGH);
      buzzerOn = true;
      Serial.println("Buzzer encendido");
    }
  }
  if (buzzerOn && ahora >= buzzerHasta){
    digitalWrite(BUZZER_PIN, LOW);
    buzzerOn = false;
    Serial.println("Buzzer apagao");
  }
}