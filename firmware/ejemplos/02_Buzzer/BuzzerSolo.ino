int buzzer = 23;  // El pin donde conectaste el buzzer

void setup() {
  pinMode(buzzer, OUTPUT);
}

void loop() {
  digitalWrite(buzzer, HIGH);  // Suena
  delay(2000);                 // Suena 1 segundo
  digitalWrite(buzzer, LOW);   // Silencio
  delay(1000);                 // Silencio 1 segundo
}