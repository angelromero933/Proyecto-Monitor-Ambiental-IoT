#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>

#define DHT22_PIN  15 // ESP32 pin GPIO21 connected to DHT22 sensor

DHT dht22(DHT22_PIN, DHT22);

//wifi
const char* ssid = "Wokwi-GUEST";
const char* password = "";

const char* mqttServer = "6a27f69ee3d5461f8991096bdfcaaf3c.s1.eu.hivemq.cloud";
const int mqttPort = 8883;
const char* mqtt_user = "adminhive";
const char* mqtt_password = "adminhive";

WiFiClientSecure espClient;
PubSubClient client(espClient);

void reconnect();
void callback(char* topic, byte* payload, unsigned int length);

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n=== DHT22 + MQTT ===");

  Serial.print("Conectando a Wi-Fi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi conectado");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  espClient.setInsecure();
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
  
  reconnect();
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

void loop() {
 
  float hum  = dht22.readHumidity();
  float temp = dht22.readTemperature();

  StaticJsonDocument<256> doc;
  doc["Temperatura"] = temp;
  doc["Humedad"] = hum;
  
  char buffer[256];
  serializeJson(doc, buffer);
  
  // Publicar en HiveMQ
  if (client.publish("sensores/esp32/datos", buffer)) {
    Serial.print("Datos publicados: ");
    Serial.println(buffer);
  } else {
    Serial.println("Error al publicar");
  }
  // wait a 2 seconds between readings
  delay(2000);
}
