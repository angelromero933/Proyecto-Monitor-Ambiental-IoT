#ifndef CONFIG_H
#define CONFIG_H

//wifi
const char* ssid = "Fibertel WiFi014 2.4GHz";
const char* password = "01420822666";

//mqtt hivemq
const char* mqttServer = "6a27f69ee3d5461f8991096bdfcaaf3c.s1.eu.hivemq.cloud";
const int mqttPort = 8883;
const char* mqtt_user = "adminhive";
const char* mqtt_password = "adminhive";

#define DHT_PIN 15            // DHT22 GPIO15

#define GAS_PIN 34            // ADC MQ-135(solo entrada) GPIO34
#define MQ135_DOUT_PIN 4      // DOUT a GPIO4 para alarma digital(HIGH o LOW) es digital, solo indica si hay gas o no, no da valor de ppm 

#define SDA_PIN 21            // SDA GPIO21 (i2c)
#define SCL_PIN 22            // SCL GPIO22 (i2c)

#define SD_CS_PIN 5           // CS GPIO5 (SD card) Chip select
#define SD_MOSI_PIN 23        // MOSI GPIO23 (SD card) SPI data output     (DI en wokwi)      
#define SD_MISO_PIN 19        // MISO GPIO19 (SD card) SPI data input      (DO en wokwi)
#define SD_SCK_PIN 18         // SCK GPIO18 (SD card) SPI clock

#define BUZZER_PIN 13       
#define LED_ROJO_PIN_ 12
#define LED_VERDE_PIN_ 27
#define LED_AZUL_PIN_ 26
#define LED_AMARILLO_PIN_ 14


#endif