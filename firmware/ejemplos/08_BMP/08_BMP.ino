#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>

// Crear objeto del sensor BMP280/BMP180
Adafruit_BMP280 bmp;

void setup() {
  Serial.begin(115200);

  // Inicializar I2C con los pines del ESP32
  Wire.begin(21, 22);

  Serial.println("=== PRUEBA BMP280/BMP180 ===");

  // Inicializar el sensor
  if (!bmp.begin(0x77)) {
    Serial.println("No se encontró el sensor BMP.");
    while (1);
  }

  Serial.println("Sensor BMP inicializado correctamente.");
}

void loop() {
  float temperatura = bmp.readTemperature();
  float presion = bmp.readPressure() / 100.0; // Pascales a hPa
  float altitud = bmp.readAltitude(1013.25);  // Presión al nivel del mar

  Serial.print("Temperatura: ");
  Serial.print(temperatura);
  Serial.println(" °C");

  Serial.print("Presión: ");
  Serial.print(presion);
  Serial.println(" hPa");

  Serial.print("Altitud aproximada: ");
  Serial.print(altitud);
  Serial.println(" m");

  Serial.println("------------------------");

  delay(2000);
}
