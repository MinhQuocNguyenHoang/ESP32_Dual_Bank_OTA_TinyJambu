import paho.mqtt.client as mqtt
import json
import random
import time
# Đây là code test kết nối và gửi giá trị mẫu lên server. 

random.seed(42);
sensor_client = mqtt.Client()
sensor_client.connect("localhost", 1883, 60)
status = ["Good", "Bad"]

while(1):
    data_gia = {"temperature": random.randint(1, 30), "humidity": random.randint(1, 70), "status": random.choice(status)}
    time.sleep(3)
    sensor_client.publish("node/sensor_phong_khach/telemetry", json.dumps(data_gia))
    print("Sending to Local Mosquitto!")