# Guía de Uso del Backend y Firmware

## Índice
1. [Requisitos Previos](#requisitos)
2. [Configuración del Backend](#backend)
3. [Ejecutar el Backend](#ejecutar)
4. [Configuración del Firmware (ESP32)](#firmware)
5. [Prueba con ESP32 Físico](#esp32-fisico)
6. [Prueba con Wokwi (Simulación)](#wokwi)
7. [Verificación de Datos en Supabase](#supabase)

---

## 1️ Requisitos Previos {#requisitos}

### Hardware
- ESP32 (físico o simulación en Wokwi)
- Sensores: DHT22, BMP280, MQ-135 (para pruebas físicas), ...

### Software
- **Python 3.8+**
- **Git** (para clonar el repositorio)
- **VS Code** con PlatformIO (para firmware)
- **Cuenta en HiveMQ Cloud** (gratuita)
- **Cuenta en Supabase** (gratuita)

---

## 2️ Configuración del Backend {#backend}

### Estructura del backend

```
Monitor-Ambiental-IoT/
├── backend/
│   ├── src/
│   │   ├── main.py
│   │   ├── config.py
│   │   ├── database.py
│   │   └── mqtt_client.py
│   ├── .env               # NO subir a GitHub
│   ├── .env.example       # Esta en GitHub a modo ejemplo
│   ├── requirements.txt
│   └── venv/              # NO subir a GitHub
```

### Crear el archivo `.env`, por teminal o el explorador de archivos

```bash
# 1. Navegar al backend
cd backend

# 2. Copiar el ejemplo
cp .env.example .env

# 3. Editar el archivo
nano .env  # Linux/macOS
# o
notepad .env  # Windows
```

### Contenido del `.env`

```env
# ============ SUPABASE ============
SUPABASE_URL=https://tu-proyecto.supabase.co
SUPABASE_KEY=_secretKey_...  # Service Key

# ============ HIVEMQ CLOUD ============
MQTT_BROKER=tu-cluster.s1.eu.hivemq.cloud
MQTT_PORT=8883
MQTT_USER=tu_usuario
MQTT_PASS=tu_contraseña
MQTT_TOPIC=sensores/esp32/datos

# ============ USUARIO ============
USER_ID=tu_uuidDeSupabase
```

### **IMPORTANTE: El `.env` NO se sube a GitHub**

```gitignore
# .gitignore
.env
backend/.env
*.env
```

---

## 3️ Ejecutar el Backend {#ejecutar}

### Activar el entorno virtual (venv)

**Windows (PowerShell):**
```powershell
cd backend
python -m venv venv
venv\Scripts\activate
```

**Windows (CMD):**
```cmd
cd backend
python -m venv venv
venv\Scripts\activate.bat
```

**Linux/macOS:**
```bash
cd backend
python3 -m venv venv
source venv/bin/activate
```

### Instalar dependencias

```bash
pip install -r requirements.txt
```

### Ejecutar uvicorn

```bash
#Estando en el directorio "backend": .../backend>
uvicorn src.main:app --host 0.0.0.0 --port 8000 --reload
```

### Salida esperada

```
Conectando a Supabase: https://tu-proyecto.supabase.co ...
Conectado a HiveMQ: tu-cluster.s1.eu.hivemq.cloud
Suscrito a: sensores/esp32/datos
INFO:     Uvicorn running on http://0.0.0.0:8000
```

### Detener el backend

`Ctrl+C` en la terminal.

### Desactivar el venv

```bash
deactivate
```

---

## 4️ Configuración del Firmware {#firmware}

### Estructura del firmware

```
Monitor-Ambiental-IoT/
└── firmware/
    ├── src/
    │   └── ...
    │   └── main.cpp
    ├── platformio.ini
    ├── diagram.json
    └── wokwi.toml
```

### Configurar `src/config.h`

```cpp
// ============ WIFI ============
// 🔹 Para ESP32 físico
const char* WIFI_SSID = "TU_WIFI";
const char* WIFI_PASSWORD = "TU_CONTRASEÑA";

// 🔹 Para Wokwi (simulación)
// const char* WIFI_SSID = "Wokwi-GUEST";
// const char* WIFI_PASSWORD = "";

// ============ HIVEMQ ============
const char* MQTT_BROKER = "tu-cluster.s1.eu.hivemq.cloud";
const int MQTT_PORT = 8883;
const char* MQTT_USER = "tu_usuario";
const char* MQTT_PASS = "tu_contraseña";
const char* MQTT_TOPIC = "sensores/esp32/datos";

// ============ PINES ============
#define DHT_PIN 15
#define MQ135_PIN 34
#define BMP_SDA 21
#define BMP_SCL 22
#define ...

```

---

## 5️ Prueba con ESP32 Físico {#esp32-fisico}

### Pasos

1. **Configurar Wi-Fi real en `config.h`**
   ```cpp
   const char* WIFI_SSID = "MiWiFi";
   const char* WIFI_PASSWORD = "MiContraseña";
   ```

2. **Compilar y/o subir el firmware a placa**
  

3. **Verificar funcionamiento en el monitor serie**

4. **Activar el backend (venv + uvicorn)**
   ```bash
   cd backend
   source venv/bin/activate
   uvicorn src.main:app --host 0.0.0.0 --port 8000 --reload
   ```

5. **Ver los datos en el monitor serie del ESP32**
   ```
   Datos publicados: {"temperatura":25.5,"humedad":55.2, ...}
   Wi-Fi conectado
   Conectando a HiveMQ... Conectado!
   ```

6. **Ver los datos en la terminal del backend**
   ```
   Mensaje recibido de HiveMQ: {'temperatura': 25.5, 'humedad': 55.2, ...}
   Datos guardados en Supabase (ID: 1)
   ```

---

## 6️ Prueba con Wokwi (Simulación) {#wokwi}

### Pasos

1. **Configurar Wi-Fi para Wokwi en `config.h`**
   ```cpp
   const char* WIFI_SSID = "Wokwi-GUEST";
   const char* WIFI_PASSWORD = "";          # Se deja vacio
   ```

2. **Activar el backend (venv + uvicorn)**
   ```bash
   cd backend
   source venv/bin/activate
   uvicorn src.main:app --host 0.0.0.0 --port 8000 --reload
   ```

3. **Compilar el firmware**

4. **Iniciar Wokwi Simulator en VS Code**
   - `F1` → "Wokwi: Start Simulator"
   - O usar el botón "▶️" en la barra de Wokwi

5. **Ver los datos en la terminal de Wokwi**
   ```
   Conectando a Wi-Fi...
   Wi-Fi conectado
   IP: 10.13.37.2
   Conectando a HiveMQ... Conectado!
   Datos publicados: {"temperatura":25.5,"humedad":55.2, ...}
   ```

6. **Ver los datos en la terminal del backend**
   ```
   Mensaje recibido de HiveMQ: {'temperatura': 25.5, 'humedad': 55.2, ...}
   Datos guardados en Supabase (ID: 1)
   ```

---

## 7️ Verificación en Supabase para ambos metodos {#supabase}

### En el SQL Editor de Tablas de Supabase

### Desde la API del backend

```bash
curl http://localhost:8000/api/datos
```

### Salida esperada

```json
[
  {
    "id": 1,
    "Temperatura": 25.5,
    "Humedad": 55,
    "Presion": 1013.25,
    "Fecha": "2026-08-20T14:34:17Z",
    "Gas-Ppm": 150.0,
    "creator": "uuid",
    "Fecha-Subida": "2026-08-20T14:34:17Z"
  }
]
```

## Resumen de comandos

### Backend
```bash
# Activar venv (Linux/macOS)
cd backend
source venv/bin/activate

# Activar venv (Windows)
cd backend
venv\Scripts\activate

# Instalar dependencias
pip install -r requirements.txt

# Ejecutar servidor
uvicorn src.main:app --host 0.0.0.0 --port 8000 --reload

# Salir del venv
deactivate
```
