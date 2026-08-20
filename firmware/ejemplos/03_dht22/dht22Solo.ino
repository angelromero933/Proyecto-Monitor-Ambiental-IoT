#include <DHT.h>
#define DHT22_PIN  4 // ESP32 pin GPIO21 connected to DHT22 sensor

DHT dht22(DHT22_PIN, DHT22);

void setup() {
  Serial.begin(9600);
  dht22.begin(); // initialize the DHT22 sensor
}

void loop() {
  // read humidity
  float humi  = dht22.readHumidity();
  // read temperature in Celsius
  float tempC = dht22.readTemperature();
  // read temperature in Fahrenheit
  float tempF = dht22.readTemperature(true);

  // check whether the reading is successful or not
  if ( isnan(tempC) || isnan(tempF) || isnan(humi)) {
    Serial.println("Failed to read from DHT22 sensor!");
  } else {
    Serial.print("Humidity: ");
    Serial.print(humi);
    Serial.print("%");

    Serial.print("  |  ");

    Serial.print("Temperature: ");
    Serial.print(tempC);
    Serial.print("°C  ~  ");
    Serial.print(tempF);
    Serial.println("°F");
  }

  // wait a 2 seconds between readings
  delay(2000);
}
