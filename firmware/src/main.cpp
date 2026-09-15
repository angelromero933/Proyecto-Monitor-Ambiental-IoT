#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <DHT.h>
#include <Adafruit_BMP280.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <RTClib.h>
#include "config.h"

DHT dht22(DHT_PIN, DHT22);
Adafruit_BMP280 bmp; // I2C
RTC_DS3231 rtc; // I2C

WiFiClientSecure espClient;
PubSubClient client(espClient);

void reconnect();
void callback(char* topic, byte* payload, unsigned int length);

bool datosGuardar = false;

void setup() {
  pinMode(LED_ROJO_PIN, OUTPUT); 
  pinMode(LED_VERDE_PIN, OUTPUT); 
  pinMode(LED_AMARILLO_PIN, OUTPUT); 
  pinMode(LED_AZUL_PIN, OUTPUT); 
  pinMode(BUZZER_PIN, OUTPUT); 

  Serial.begin(115200);
  delay(1000);
  Serial.println("\n=== SISTEMA DE MONITOREO AMBIENTAL ===");

  Serial.print("Conectando a Wi-Fi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(String("\nWi-Fi conectado, IP: ") + WiFi.localIP().toString());

  espClient.setInsecure();
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
  
  reconnect();

  dht22.begin(); 
  bmp.begin(0x76);
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("Error al inicializar la tarjeta SD.");
    return;
  }
  Serial.println("Tarjeta SD inicializada correctamente.");

  if (!rtc.begin()) {
    Serial.println("Error al inicializar el módulo RTC");
  }
  if (rtc.lostPower()) {
    Serial.println("El RTC perdió energía, ¡vamos a ajustar la hora!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
 
  pinMode(GAS_PIN, INPUT);
  pinMode(MQ135_DOUT_PIN, INPUT);

  Serial.println("Calentando el sensor por favor espere...");  
  delay(30000); // Espera inicial de 30 segundos de precalentamiento
  Serial.println("Sensor calibrado y listo. Monitoreando...");
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

void reconnect() {
  while (!client.connected()) {
    Serial.print("Conectando a HiveMQ...");
    
    if (client.connect("ESP32_001", mqtt_user, mqtt_password)) {
      Serial.println("Conectado!");
      client.subscribe("comandos/esp32");
      Serial.println("Suscrito a: comandos/esp32");
      
    } else {
      Serial.print("Falló, rc=");
      Serial.print(client.state());
      Serial.println(" reintentando en 5s...");
      delay(5000);
    }
  }
}

void leerMensajesGuardados() {
  File file = SD.open(filename, FILE_READ);
  
  if (!file) {
    Serial.println("Error al abrir el archivo de la SD");
    return;
  }

  while (file.available()) {
    String linea = file.readStringUntil('\n'); 
    linea.trim();

    if (linea.length() == 0) continue; 

    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, linea);

    if (error) {
      Serial.print("Error al interpretar el JSON: ");
      Serial.println(error.c_str());
      continue; 
    }
   
    client.publish("sensores/esp32/datos", linea.c_str());
    Serial.println("Datos publicados desde SD: " + linea);
  }
  file.close();
  if (SD.remove(filename)) {
    Serial.println("¡Éxito! Todos los datos enviados. Archivo SD vaciado.");
  } else {
    Serial.println("Error al intentar borrar el archivo.");
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  bool mq = false; 
  bool dht = false; 
  bool bmp_estado = false; 
  bool encendido = true;
  digitalWrite(LED_VERDE_PIN, HIGH); 

  float humi  = dht22.readHumidity();
  float tempC = dht22.readTemperature();

  int valorAnalogico = analogRead(GAS_PIN);
  int estadoDigital = digitalRead(MQ135_DOUT_PIN);

  float presion = bmp.readPressure();

  if( tempC > 30 or tempC < 10){
    Serial.println("Temperatura en Rango de Alerta");
    dht = true;
    delay(4000);
    dht = false;
  }
  if( humi > 70 or humi < 30){
    Serial.println("Humedad en Rango de Alerta");
    dht = true;
    delay(4000);
    dht = false;
  }
  if( presion < 1005){
    Serial.println("Presion en Rango de Alerta, clima inestable");
    bmp_estado = true;
    delay(4000);
    bmp_estado = false;
  }
  
  if (valorAnalogico >= UMBRAL_ALERTA || estadoDigital == LOW) {
    Serial.println("ESTADO: PELIGRO - Gas o Contaminación Alta");
    mq = true;
    delay(4000);
    mq = false;
  }else if (valorAnalogico >= UMBRAL_ADVERTENCIA && valorAnalogico < UMBRAL_ALERTA) {
    Serial.println("ESTADO: ADVERTENCIA - Calidad de Aire Moderada / Ventilar");
      mq = true;
      delay(4000);
      mq = false;
  }else {
    Serial.println("ESTADO: OK - Aire Limpio");
  }

  digitalWrite(LED_ROJO_PIN, mq ? HIGH : LOW);
  digitalWrite(LED_AZUL_PIN, dht ? HIGH : LOW);
  digitalWrite(LED_AMARILLO_PIN, bmp_estado ? HIGH : LOW);
  digitalWrite(BUZZER_PIN, (dht or bmp_estado or mq) ? HIGH : LOW);

  String creator = "86d8ce35-705a-4413-9791-9192e5470a76";//uuid de supabase(id de user admin)

  DateTime ahora = rtc.now();

  StaticJsonDocument<256> doc;
  doc["Temperatura"] = tempC;
  doc["Humedad"] = humi;
  doc["Presion"] = presion;
  doc["Fecha"]   = rtc.now().timestamp(DateTime::TIMESTAMP_FULL);
  doc["Gas-Ppm"] = valorAnalogico;
  doc["creator"] = creator;
  
  char buffer[256];
  serializeJson(doc, buffer);
  
  // Publicar en HiveMQ segun topico
  if (client.publish("sensores/esp32/datos", buffer)) {
    Serial.print("Datos publicados: ");
    Serial.println(buffer);
    if(datosGuardar) {
      leerMensajesGuardados();
    }
  }else {
    Serial.println("Error al publicar");

    File file = SD.open(filename, FILE_WRITE);
    if(file) {
      file.println(buffer);
      file.close();
      Serial.println("Datos escritos en la tarjeta SD.");
    }else {
      Serial.println("Error al abrir el archivo.");
    }
  }
  delay(10000);
}