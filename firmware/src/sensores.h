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
    }

    void inicializarSensores(){
        pinMode(LED_ROJO_PIN, OUTPUT);
        pinMode(LED_VERDE_PIN, OUTPUT);
        pinMode(LED_AMARILLO_PIN, OUTPUT);
        pinMode(LED_AZUL_PIN, OUTPUT);
        pinMode(BUZZER_PIN, OUTPUT);
            
        pinMode(GAS_PIN, INPUT);
        pinMode(MQ135_DOUT_PIN, INPUT);

        dht22.begin(); 
        bmp.begin(0x76);
        if (!SD.begin(SD_CS_PIN)) {
            Serial.println("Error al inicializar la tarjeta SD.");
            return;
        }
        Serial.println("Tarjeta SD inicializada correctamente.");

        if (!rtc.begin()) {
            Serial.println("Error al inicializar el módulo RTC");
        }
        if (rtc.lostPower()) {
            Serial.println("El RTC perdió energía, ¡vamos a ajustar la hora!");
            rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
        }
    }
    void leerSensores(){
        humedad = dht22.readHumidity();
        temperatura = dht22.readTemperature();

        valorAnalogico = analogRead(GAS_PIN);
        estadoDigital = digitalRead(MQ135_DOUT_PIN);

        presion = float(bmp.readPressure()) / 100.0F; // Convertir a hPa


        alertaGas();
        alertaDHT();
        alertaBMP();
    }
    float obtenerTemperatura(){     return temperatura; }
    float obtenerHumedad(){         return humedad;   }
    float obtenerPresion(){         return presion;  }
    float obtenerGas(){             return valorAnalogico;  }
    bool obtenerEstadoDigital(){    return estadoDigital;   }
    String obtenerReloj(){          return rtc.now().timestamp(DateTime::TIMESTAMP_FULL);  }

    bool obtenerEstadoMQ(){         return mq; }
    bool obtenerEstadoDHT(){        return dht;    }
    bool obtenerEstadoBMP(){        return bmp_estado; }

    void alertaGas(){
        if (valorAnalogico >= UMBRAL_ALERTA || estadoDigital == LOW) {
            Serial.println("ESTADO: PELIGRO - Gas o Contaminación Alta");
            mq = true;
            digitalWrite(LED_ROJO_PIN, HIGH); // Encender LED rojo
            delay(4000);
            mq = false;
            digitalWrite(LED_ROJO_PIN, LOW); // Apagar LED rojo
        } else if (valorAnalogico >= UMBRAL_ADVERTENCIA && valorAnalogico < UMBRAL_ALERTA) {
            Serial.println("ESTADO: ADVERTENCIA - Calidad de Aire Moderada / Ventilar");
            mq = true;
            digitalWrite(LED_AMARILLO_PIN, HIGH); // Encender LED amarillo
            delay(4000);
            mq = false;
            digitalWrite(LED_AMARILLO_PIN, LOW); // Apagar LED amarillo
        } else {
            Serial.println("ESTADO: OK - Aire Limpio");
        }
    }

    void alertaDHT(){
        if(temperatura > 30 or temperatura < 10){
            Serial.println("Temperatura en Rango de Alerta");
            dht = true;
            digitalWrite(LED_AZUL_PIN, HIGH); // Encender LED azul
            delay(4000);
            dht = false;
            digitalWrite(LED_AZUL_PIN, LOW); // Apagar LED azul
        }
        if(humedad > 70 or humedad < 30){
            Serial.println("Humedad en Rango de Alerta");
            dht = true;
            digitalWrite(LED_AMARILLO_PIN, HIGH); // Encender LED amarillo
            delay(4000);
            dht = false;
            digitalWrite(LED_AMARILLO_PIN, LOW); // Apagar LED amarillo
        }
    }

    void alertaBMP(){
        if(presion < 1005){
            Serial.println("Presion en Rango de Alerta, clima inestable");
            bmp_estado = true;
            digitalWrite(LED_AZUL_PIN, HIGH); // Encender LED azul
            delay(4000);
            bmp_estado = false;
            digitalWrite(LED_AZUL_PIN, LOW); // Apagar LED azul
        }
    }

private:
    float temperatura;
    float humedad;
    float presion;
    float valorAnalogico;
    bool mq, dht, bmp_estado, encendido, estadoDigital;

    DHT dht22;
    Adafruit_BMP280 bmp; // I2C
    RTC_DS3231 rtc; // I2C
    DateTime ahora;
};