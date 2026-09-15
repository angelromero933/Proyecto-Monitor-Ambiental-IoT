import os
import json
from dotenv import load_dotenv
from fastapi import FastAPI
import paho.mqtt.client as mqtt
from supabase import create_client
from datetime import datetime

# cargar .env
load_dotenv()

SUPABASE_URL = os.getenv("SUPABASE_URL")
SUPABASE_KEY = os.getenv("SUPABASE_KEY")

if not SUPABASE_URL or not SUPABASE_KEY:
    raise ValueError("SUPABASE_URL o SUPABASE_KEY no están configuradas")

print(f"Conectando a Supabase: {SUPABASE_URL}")
supabase = create_client(SUPABASE_URL, SUPABASE_KEY)

MQTT_BROKER = os.getenv("MQTT_BROKER", "6a27f69ee3d5461f8991096bdfcaaf3c.s1.eu.hivemq.cloud")
MQTT_PORT = int(os.getenv("MQTT_PORT", 8883))
MQTT_USER = os.getenv("MQTT_USER", "adminhive")
MQTT_PASS = os.getenv("MQTT_PASS", "adminhive")
MQTT_TOPIC = os.getenv("MQTT_TOPIC", "sensores/esp32/datos")

app = FastAPI(title="IoT Backend - HiveMQ + Supabase")

#decorador raiz, y endpoint x msj por cada ingreso a la direccion(host)
@app.get("/")
def root():
    return {"message": "IoT Backend funcionando"}

@app.get("/api/datos")
def get_datos(limit: int = 10):
    try:
        result = supabase.table("Lecturas") \
            .select("*") \
            .order("id", desc=True) \
            .limit(limit) \
            .execute()
        return result.data
    except Exception as e:
        return {"error": str(e)}


#por cada msj recibido lo sube a supabase
def on_message(client, userdata, msg):
    try:
        payload = json.loads(msg.payload.decode())
        print(f"Mensaje recibido de HiveMQ: {payload}")
        
        temp = payload.get("Temperatura", 0.0)
        hum = payload.get("Humedad", 0)
        presion = payload.get("Presion", 0.0)
        gasPpm = payload.get("Gas-Ppm", 0.0)
        fecha = payload.get("Fecha", datetime.now().isoformat())
        creator = payload.get("creator", "86d8ce35-705a-4413-9791-9192e5470a76")
       
        
        data = {
            "Temperatura": temp,
            "Humedad": hum,
            "Presion": presion,
            "Fecha": fecha,
            "Gas-Ppm": gasPpm,
            "creator": creator,
        }
        
        # Inserta en Supabase -> Lecturas
        result = supabase.table("Lecturas").insert(data).execute()
        print(f"Datos guardados en Supabase (ID: {result.data[0]['id']})")
        print(f"Datos guardados: {data}")
        
    except Exception as e:
        print(f"Error procesando mensaje: {e}")

#incio servicio mqtt y escucho al broker hivemq
def init_mqtt():
    client = mqtt.Client()
    client.username_pw_set(MQTT_USER, MQTT_PASS)
    client.tls_set()
    client.on_message = on_message
    client.connect(MQTT_BROKER, MQTT_PORT, 60)
    client.subscribe(MQTT_TOPIC)
    client.loop_start()
    print(f"Conectado a HiveMQ: {MQTT_BROKER}")
    print(f"Suscrito a: {MQTT_TOPIC}")
    return client


mqtt_client = init_mqtt()

if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8000)
