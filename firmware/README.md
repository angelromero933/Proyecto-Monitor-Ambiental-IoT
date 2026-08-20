# Firmware para ESP32 - Monitoreo Ambiental

## 📋 Descripción
Código del ESP32 que lee sensores (DHT22 y sensor de gas) y publica los datos por MQTT.

## 🛠️ Tecnologías
- PlatformIO
- Wokwi (simulación)
- Librerías: DHT, PubSubClient, ArduinoJson

## 📁 Estructura
- `src/main.cpp`: Código principal
- `diagram.json`: Circuito para Wokwi
- `platformio.ini`: Configuración de PlatformIO

## 🚀 Cómo probar
1. Abrí `diagram.json` en Wokwi (extensión de VS Code)
2. Hacé clic en "Start Simulation"
3. Abrí el Serial Monitor para ver los datos