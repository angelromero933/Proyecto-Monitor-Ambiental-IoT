Microsd card adapter 
#include <SPI.h>
#include <SD.h>

const int chipSelect = 4; // Pin CS conectado al pin 4 del Arduino

void setup() {
  Serial.begin(9600);
  
  Serial.print("Inicializando tarjeta SD...");
  if (!SD.begin(chipSelect)) {
    Serial.println("Error al inicializar o tarjeta no presente.");
    return;
  }
  Serial.println("Tarjeta lista.");
}

void loop() {
  // Abre el archivo. Nota: solo se puede abrir un archivo a la vez.
  File dataFile = SD.open("registro.txt", FILE_WRITE);

  if (dataFile) {
    // Ejemplo de registro: simulación de temperatura y estado
    dataFile.println("Aire Encendido - Temperatura: 24C");
    dataFile.close();
    Serial.println("Datos guardados correctamente.");
  } else {
    Serial.println("Error al abrir el archivo registro.txt");
  }

  delay(5000); // Espera 5 segundos antes del siguiente registro
}
