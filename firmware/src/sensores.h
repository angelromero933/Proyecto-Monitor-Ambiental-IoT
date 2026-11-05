#include <Arduino.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <Adafruit_BMP280.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <RTClib.h>
#include <time.h>
#include "config.h"

enum class Nivel { NORMAL = 0, ADVERTENCIA = 1, PELIGRO = 2 };
class Sensores {
public:
    Sensores() : dht22(DHT_PIN, DHT22), bmp(), rtc() {
        temperatura = 0.0;
        humedad = 0.0;
        presion = 0.0;
        valorAnalogico = 0.0;

        encendido = true;
        estadoDigital = false;
        estadoRtc = false;
        estadoSd = false;
    }

    void inicializarSensores(){
        Wire.begin(SDA_PIN, SCL_PIN);

        pinMode(LED_ROJO_PIN, OUTPUT);
        pinMode(LED_VERDE_PIN, OUTPUT);
        pinMode(LED_AMARILLO_PIN, OUTPUT);
        pinMode(LED_AZUL_PIN, OUTPUT);
        pinMode(BUZZER_PIN, OUTPUT);
            
        pinMode(GAS_PIN, INPUT);
        pinMode(MQ135_DOUT_PIN, INPUT);

        dht22.begin(); 
        
        if (!bmp.begin(0x76)) {
        Serial.println("bmp error");
        } else {
            Serial.println("bmp incializado correctamente");
        }
        
        if (SD.begin(SD_CS_PIN)) {
            estadoSd = true;
            Serial.println("sd card incializado correctamente");
        } else {
            estadoSd = false;
            Serial.println("sd card error, sigue sin backup)");
        }

        if (!rtc.begin()){
            Serial.println("Error al inicializar el modulo RTC");
            estadoRtc = false;
        }else{
            estadoRtc = true;
            if (rtc.lostPower()){
                Serial.println("El RTC perdio energia, ajustar hora");
                configTime((-3 * 3600), 0, "pool.ntp.org");
                struct tm t;
                if (getLocalTime(&t, 10000)){
                    rtc.adjust(DateTime(t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec));
                    Serial.println("RTC sincronizado");
                }else{
                    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
                    Serial.println("Sin servidor NTP, usando hora de compilacion como respaldo.");
                }
            }
        }
    }
    void leerSensores(){
        float t_raw = dht22.readTemperature();
        float h_raw = dht22.readHumidity();

        if (isnan(t_raw) || isnan(h_raw)) {
            temperatura = NAN;
            humedad = NAN;
        } else {
            temperatura = round(t_raw * 10.0) / 10.0;  // Guarda con 1 decimal
            humedad = int(h_raw);
        }
        valorAnalogico = round(analogRead(GAS_PIN));
        estadoDigital = digitalRead(MQ135_DOUT_PIN);
        
        float p_raw = bmp.readPressure();
        if (p_raw <= 0 || isnan(p_raw)) {
            presion = NAN;
        } else {
            presion = round(p_raw) / 100.0; // Convertir a hPa
        }
    }
    
    float obtenerTemperatura(){     return temperatura; }   
    float obtenerHumedad(){         return humedad;  }
    float obtenerPresion(){         return presion;     }
    float obtenerGas(){             return valorAnalogico;     }
    bool obtenerEstadoDigital(){    return estadoDigital;   }
    bool obtenerEstadoSD(){ return estadoSd; }
    String obtenerReloj(){
        if (!estadoRtc) return "1970-01-01T00:00:00";

        DateTime ahora = rtc.now();
        if (ahora.year() < 2020) return "1970-01-01T00:00:00";
        return ahora.timestamp(DateTime::TIMESTAMP_FULL);   
    }

    Nivel nivelGas() {
        if (valorAnalogico >= UMBRAL_ALERTA) return Nivel::PELIGRO;

        if (valorAnalogico >= UMBRAL_ADVERTENCIA && valorAnalogico < UMBRAL_ALERTA) return Nivel::ADVERTENCIA;

        return Nivel::NORMAL;
    }

    Nivel nivelDHT() {
        if (temperatura > 35 || temperatura < 5)  return Nivel::PELIGRO;

        if (temperatura > 30 || temperatura < 10 ||
            humedad > 70 || humedad < 30)         return Nivel::ADVERTENCIA;

        return Nivel::NORMAL;
    }

    Nivel nivelBMP() { 
        if (presion < 980 || presion > 1040) {
            return Nivel::PELIGRO; 
        }
        if (presion < 1000 || presion > 1020) {
            return Nivel::ADVERTENCIA; 
        }

        return Nivel::NORMAL; 
    }

    Nivel nivelGlobal() {
        int max_alerta = max((int)nivelGas(), max((int)nivelDHT(), (int)nivelBMP()));
        return (Nivel)max_alerta;
    }

private:
    float temperatura;
    float humedad;
    float presion;
    float valorAnalogico;
    bool encendido, estadoDigital, estadoSd, estadoRtc;

    DHT dht22;
    Adafruit_BMP280 bmp; // I2C
    RTC_DS3231 rtc; // I2C
};