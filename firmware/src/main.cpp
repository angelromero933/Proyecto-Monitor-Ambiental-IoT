#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <config.h>

WiFiClientSecure espClient;
PubSubClient client(espClient);

void reconnect();
void callback(char* topic, byte* payload, unsigned int length);

void setup() {
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
  if (!client.connected()) {
    reconnect();
  }
  
  client.loop();
  
  float temp = 25.5;
  int hum = 55;
  float presion = 1013.25;
  float gasPpm = 150.0;
  String fecha = "2026-08-09T14:34:17Z";
  String creator = "86d8ce35-705a-4413-9791-9192e5470a76";//uuid de supabase(id de user admin)
  
  StaticJsonDocument<256> doc;
  doc["Temperatura"] = temp;
  doc["Humedad"] = hum;
  doc["Presion"] = presion;
  doc["Fecha"] = fecha;
  doc["Gas-Ppm"] = gasPpm;
  doc["creator"] = creator;
  
  char buffer[256];
  serializeJson(doc, buffer);
  
  // Publicar en HiveMQ
  if (client.publish("sensores/esp32/datos", buffer)) {
    Serial.print("Datos publicados: ");
    Serial.println(buffer);
  } else {
    Serial.println("Error al publicar");
  }
  
  delay(5000);
}