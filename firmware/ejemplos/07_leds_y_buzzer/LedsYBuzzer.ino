#include <Arduino.h>

// Pines del proyecto IoT
#define BUZZER_PIN 13
#define LED_ROJO_PIN 12
#define LED_AMARILLO_PIN 14
#define LED_VERDE_PIN 27
#define LED_AZUL_PIN 26

void setup() {
  pinMode(LED_ROJO_PIN, OUTPUT);
  pinMode(LED_AMARILLO_PIN, OUTPUT);
  pinMode(LED_VERDE_PIN, OUTPUT);
  pinMode(LED_AZUL_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
}

void loop() {

  // LED rojo
  digitalWrite(LED_ROJO_PIN, HIGH);
  delay(1000);
  digitalWrite(LED_ROJO_PIN, LOW);

  // LED amarillo
  digitalWrite(LED_AMARILLO_PIN, HIGH);
  delay(1000);
  digitalWrite(LED_AMARILLO_PIN, LOW);

  // LED verde
  digitalWrite(LED_VERDE_PIN, HIGH);
  delay(1000);
  digitalWrite(LED_VERDE_PIN, LOW);

  // LED azul
  digitalWrite(LED_AZUL_PIN, HIGH);
  delay(1000);
  digitalWrite(LED_AZUL_PIN, LOW);

  // Buzzer
  digitalWrite(BUZZER_PIN, HIGH);
  delay(1000);
  digitalWrite(BUZZER_PIN, LOW);

  delay(1000);
}
