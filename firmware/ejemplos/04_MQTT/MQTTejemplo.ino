#include <WiFi.h>
#include <PubSubClient.h>

// ============ CONFIGURACIÓN ============
// Wi-Fi
const char* ssid = "Fibertel WiFi014 2.4GHz";
const char* password = "contrase�a";

// MQTT Broker (IP de tu Linux)
const char* mqtt_server = "192.168.0.39";  // ← CAMBIAR POR TU IP
const int mqtt_port = 1883;

// ============ OBJETOS ============
WiFiClient espClient;
PubSubClient client(espClient);

// ============ VARIABLES ============
unsigned long lastMsg = 0;
int contador = 0;

// ============ SETUP ============
void setup() {
  Serial.begin(115200);
  
  // Conectar Wi-Fi
  Serial.print("Conectando a Wi-Fi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Wi-Fi conectado!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  
  // Configurar MQTT
  client.setServer(mqtt_server, mqtt_port);
}

// ============ LOOP ============
void loop() {
  // Reconectar si se perdió la conexión
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  // Publicar cada 5 segundos
  unsigned long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
    contador++;
    
    String payload = "Hola desde ESP32 #" + String(contador);
    client.publish("test/topic", payload.c_str());
    Serial.println("Enviado: " + payload);
  }
}

// ============ FUNCIÓN DE RECONEXIÓN ============
void reconnect() {
  while (!client.connected()) {
    Serial.print("🔗 Conectando a MQTT...");
    if (client.connect("ESP32_Prueba")) {
      Serial.println("Conectado!");
    } else {
      Serial.print(" Falló, rc=");
      Serial.print(client.state());
      Serial.println(" reintentando en 5s...");
      delay(5000);
    }
  }
}
