//circuito con resistencia + led rojo, controlado por suscripcion mqtt en celular, IoT MQTT Panel (android)


#include <WiFi.h>
#include <PubSubClient.h>

// Configuraci贸n de Red
const char* ssid = "Fibertel WiFi014 2.4GHz";
const char* password = "contrase馻";

// Configuraci贸n del Broker MQTT
const char* mqtt_server = "192.168.0.39"; // Broker p煤blico de prueba

// Configuraci贸n de Pines
const int ledPin = 12; // Pin del LED

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

// Funci贸n que se ejecuta al recibir un mensaje MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  String mensaje = "";
  for (int i = 0; i < length; i++) {
    mensaje += (char)payload[i];
  }

  // Si el mensaje es "encender"
  if (mensaje == "encender") {
    digitalWrite(ledPin, HIGH);
  }
  // Si el mensaje es "apagar"
  if (mensaje == "apagar") {
    digitalWrite(ledPin, LOW);
  }
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect("ESP32Client")) {
      // Nos suscribimos al t贸pico para escuchar comandos
      client.subscribe("casa/led");
    } else {
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
