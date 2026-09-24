#pragma once
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

class Sensores {
public:
    Sensores() : dht22(DHT_PIN, DHT22), bmp(), rtc() {
        temperatura = 0.0;
        humedad = 0.0;
        presion = 0.0;
        valorAnalogico = 0.0;
        mq = false;
        dht = false;
        bmp_estado = false;
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
            Serial.println(" sd card incializado correctamente");
        } else {
            estadoSd = false;
            Serial.println("sd card error, sigue sin backup)");
        }

        if (!rtc.begin()) {
            Serial.println("Error al inicializar el modulo RTC");
            estadoRtc = false;
        }else{
            estadoRtc = true;
            if (rtc.lostPower()) {
                Serial.println("El RTC perdio energia, ajustar hora");
                configTime((-3 * 3600), 0, "pool.ntp.org");
                rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
                struct tm t;
                if (getLocalTime(&t, 10000)) {
                    rtc.adjust(DateTime(t.tm_year+1900, t.tm_mon+1, t.tm_mday,
                                        t.tm_hour, t.tm_min, t.tm_sec));
                    Serial.println("RTC sincronizado");
                }
            }
        }
    }

    void leerSensores(){
        humedad = round(dht22.readHumidity() * 10.0) / 10.0;  // Guarda con 1 decimal
        temperatura = (int)round(dht22.readTemperature() * 10.0) / 10.0;  // Guarda con 1 decimal

        valorAnalogico = (int)round(analogRead(GAS_PIN));
        estadoDigital = digitalRead(MQ135_DOUT_PIN);

        presion = round(bmp.readPressure() ) / 100.0; // Convertir a hPa

        alertaGas();
        alertaDHT();
        alertaBMP();
    }
    
    int obtenerTemperatura(){     return temperatura; }
    int obtenerHumedad(){         return humedad;   }
    int obtenerPresion(){         
        if (isnan(presion)) return 0;  // ⭐ Validar

        return presion;   
    }
    int obtenerGas(){             return valorAnalogico;  }
    bool obtenerEstadoDigital(){    return estadoDigital;   }
    String obtenerReloj(){
        if (!estadoRtc) return "1970-01-01T00:00:00";
        DateTime ahora = rtc.now();
        if (ahora.year() < 2020) return "1970-01-01T00:00:00";
        return ahora.timestamp(DateTime::TIMESTAMP_FULL);
    }

    bool obtenerEstadoMQ(){         return mq; }
    bool obtenerEstadoDHT(){        return dht;    }
    bool obtenerEstadoBMP(){        return bmp_estado; }
    bool obtenerEstadoSD(){         return estadoSd; }

    void alertaGas(){
        if (valorAnalogico >= UMBRAL_ALERTA || estadoDigital == LOW) {
            Serial.println("ESTADO: PELIGRO - Gas o Contaminación Alta");
            mq = true;
        } else if (valorAnalogico >= UMBRAL_ADVERTENCIA && valorAnalogico < UMBRAL_ALERTA) {
            Serial.println("ESTADO: ADVERTENCIA - Calidad de Aire Moderada / Ventilar");
            mq = true;
        } else {
            Serial.println("ESTADO: OK - Aire Limpio");
            mq = false;
        }
    }

    void alertaDHT(){
        if(temperatura > 30 or temperatura < 10){
            Serial.println("Temperatura en Rango de Alerta");
            dht = true;
        }
        if(humedad > 70 or humedad < 30){
            Serial.println("Humedad en Rango de Alerta");
            dht = true;
        }
    }

    void alertaBMP(){
        if(presion < 1000 or presion > 1020){
            Serial.println("Presion en Rango de Alerta, clima inestable");
            bmp_estado = true;
        }
    }
    void sincronizarNTP() {
        configTime(-3 * 3600, 0, "pool.ntp.org", "time.nist.gov");  // GMT-3 Argentina
        Serial.print("Esperando NTP");
        time_t now = time(nullptr);
        while (now < 24 * 3600) {
            delay(500);
            Serial.print(".");
            now = time(nullptr);
        }
        Serial.println(" NTP (reloj) sincronizado");
    }

private:
    float temperatura;
    int humedad;
    int presion;
    float valorAnalogico;
    bool mq, dht, bmp_estado, encendido, estadoDigital, estadoSd, estadoRtc;

    DHT dht22;
    Adafruit_BMP280 bmp; // I2C
    RTC_DS3231 rtc; // I2C
    DateTime ahora;
};