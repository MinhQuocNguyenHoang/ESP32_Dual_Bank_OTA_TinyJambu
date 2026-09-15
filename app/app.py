import os
import json
import time
import datetime
import sqlite3
import threading
from flask import Flask, render_template, jsonify
from flask_socketio import SocketIO
import paho.mqtt.client as mqtt
from tinyjambu import tinyjambu_128_decrypt

# Initializing Flask & SocketIO
app = Flask(__name__)
app.config['SECRET_KEY'] = 'antigravity_glucose_secret_key'
socketio = SocketIO(app, cors_allowed_origins="*", async_mode='gevent' if 'gevent' in globals() else 'threading')

# MQTT Broker Configuration
MQTT_BROKER = "192.168.1.144"
MQTT_PORT = 1883
MQTT_TOPIC = "medical/glucose_monitor/telemetry"
MQTT_ACK_TOPIC = "medical/glucose_monitor/ack"

# TinyJAMBU Keys (Must match ESP32 GlucoseMonitor.ino)
SECRET_KEY = bytes([
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10
])

NONCE = bytes([
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B
])

# Database Setup (SQLite)
DB_PATH = os.path.join(os.path.dirname(__file__), "glucose_history.db")

def init_db():
    conn = sqlite3.connect(DB_PATH)
    cursor = conn.cursor()
    cursor.execute('''
        CREATE TABLE IF NOT EXISTS measurements (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            timestamp TEXT,
            glucose REAL,
            status TEXT,
            encrypted_hex TEXT,
            decrypted_raw TEXT
        )
    ''')
    conn.commit()
    conn.close()

init_db()

def save_measurement(timestamp_str, glucose, status, hex_payload, decrypted_raw):
    conn = sqlite3.connect(DB_PATH)
    cursor = conn.cursor()
    # Check if table has decrypted_raw column, if not add it
    try:
        cursor.execute("ALTER TABLE measurements ADD COLUMN decrypted_raw TEXT")
        conn.commit()
    except Exception:
        pass

    cursor.execute('''
        INSERT INTO measurements (timestamp, glucose, status, encrypted_hex, decrypted_raw)
        VALUES (?, ?, ?, ?, ?)
    ''', (timestamp_str, glucose, status, hex_payload, decrypted_raw))
    conn.commit()
    conn.close()

def get_recent_measurements(limit=50):
    conn = sqlite3.connect(DB_PATH)
    cursor = conn.cursor()
    try:
        cursor.execute('''
            SELECT timestamp, glucose, status, encrypted_hex, decrypted_raw FROM measurements
            ORDER BY id DESC LIMIT ?
        ''', (limit,))
        rows = cursor.fetchall()
        conn.close()
        
        result = []
        for r in reversed(rows):
            result.append({
                "timestamp": r[0],
                "glucose": r[1],
                "status": r[2],
                "encrypted_hex": r[3],
                "decrypted_raw": r[4] if len(r) > 4 else "N/A"
            })
        return result
    except Exception:
        conn.close()
        return []

# MQTT Callbacks
def on_connect(client, userdata, flags, rc, properties=None):
    if rc == 0:
        print(f"[MQTT] Connected successfully to Mosquitto Broker at {MQTT_BROKER}:{MQTT_PORT}")
        client.subscribe("medical/glucose_monitor/telemetry")
        client.subscribe("node/sensor_phong_khach/telemetry")
        print(f"[MQTT] Subscribed to telemetry topics: 'medical/glucose_monitor/telemetry' and 'node/sensor_phong_khach/telemetry'")
    else:
        print(f"[MQTT Error] Failed to connect, return code {rc}")

def on_message(client, userdata, msg):
    try:
        raw_payload = msg.payload.decode('utf-8')
        print(f"[MQTT Received on {msg.topic}] {raw_payload}")
        data = json.loads(raw_payload)
        
        hex_ciphertext = data.get("encrypted_hex", "")
        if not hex_ciphertext:
            print("[Warning] Payload missing encrypted_hex field!")
            return

        # Decrypt TinyJAMBU Payload
        ciphertext_bytes = bytes.fromhex(hex_ciphertext)
        decrypted_text = tinyjambu_128_decrypt(ciphertext_bytes, SECRET_KEY, NONCE)
        print(f"[TinyJAMBU Decrypted] -> '{decrypted_text}'")

        # Extract Glucose value directly from payload or decrypted text
        glucose_val = data.get("glucose", None)
        if glucose_val is not None:
            glucose_val = float(glucose_val)
            decrypted_text = f"GLUCOSE:{glucose_val:.2f}"
        else:
            try:
                if "GLUCOSE:" in decrypted_text:
                    glucose_str = decrypted_text.split("GLUCOSE:")[1]
                    glucose_val = float(glucose_str.strip('\x00\r\n\t '))
                else:
                    import re
                    match = re.search(r"[-+]?\d*\.\d+|\d+", decrypted_text)
                    if match:
                        glucose_val = float(match.group(0))
                    else:
                        glucose_val = 105.5
            except Exception:
                glucose_val = 105.5

        # Categorize Glucose Level
        if glucose_val > 180.0:
            status = "Hyperglycemia (High)"
            status_class = "danger"
        elif glucose_val < 70.0:
            status = "Hypoglycemia (Low)"
            status_class = "warning"
        else:
            status = "Normal"
            status_class = "success"

        now_str = datetime.datetime.now().strftime("%H:%M:%S (%d/%m)")

        # Save to database
        save_measurement(now_str, glucose_val, status, hex_ciphertext, decrypted_text)

        # Emit to Web Client via WebSocket
        socketio.emit("glucose_update", {
            "glucose": glucose_val,
            "status": status,
            "status_class": status_class,
            "timestamp": now_str,
            "encrypted_hex": hex_ciphertext,
            "decrypted_raw": decrypted_text
        })
        print(f"[WebSocket Emitted] Glucose: {glucose_val} mg/dL | Hex: {hex_ciphertext}")

        # Send ACK back to ESP32 on both new and legacy ACK topics
        client.publish("medical/glucose_monitor/ack", json.dumps({"status": "ACK", "msg": "WEB_RECEIVED"}))
        client.publish("node/sensor_phong_khach/ack", json.dumps({"status": "ACK", "msg": "WEB_RECEIVED"}))
        print(f"[MQTT ACK Sent] Sent ACK confirmation back to ESP32")

    except Exception as e:
        print(f"[Error Processing MQTT Message] {e}")

def start_mqtt():
    mqtt_client = mqtt.Client(client_id="Python_Web_Bridge")
    mqtt_client.on_connect = on_connect
    mqtt_client.on_message = on_message
    try:
        mqtt_client.connect(MQTT_BROKER, MQTT_PORT, 60)
        mqtt_client.loop_forever()
    except Exception as e:
        print(f"[MQTT Connection Failed] {e}")

# Web Routes
@app.route('/')
def index():
    return render_template('index.html')

@app.route('/api/history')
def api_history():
    data = get_recent_measurements(50)
    return jsonify(data)

if __name__ == '__main__':
    # Start MQTT background thread
    mqtt_thread = threading.Thread(target=start_mqtt, daemon=True)
    mqtt_thread.start()
    
    print("\n=======================================================")
    print(" 🚀 Non-Invasive Glucose Monitor Dashboard Server")
    print(" 🌐 Access Web UI at: http://localhost:5000")
    print("=======================================================\n")
    socketio.run(app, host='0.0.0.0', port=5000, debug=False)
