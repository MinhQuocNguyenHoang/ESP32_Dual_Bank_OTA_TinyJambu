import paho.mqtt.client as mqtt
import json
# local_broker = "192.168.137.1" # IP của hotspot của máy host
local_broker = "192.168.1.144" # IP khi gửi 1 sample mẫu lên thingboard
local_port = 1883
local_topic = "node/sensor_phong_khach/telemetry"

tb_host = "100.103.61.31"
tb_device_token = "hg10whgn9thv4n933dr8"
tb_topic = "v1/devices/me/telemetry"

local_client = mqtt.Client(client_id="Python_Bridge_Receiver")
tb_client = mqtt.Client(client_id="Python_Bridge_Sender")

def on_connect_local(client, userdata, flags, rc):
    if rc == 0:
        print("Successful connected to Mosquitto !!!")
        local_client.subscribe(local_topic)
        print(f"Reading data from sensor base on topic: {local_topic}")
    else:
        print(f"Error !!! Can't connect to Mosquitto, error: {rc}")
def on_message_local(client, userdata, msg):
    try:
        payload = msg.payload.decode('utf-8')
        data = json.loads(payload)
        print(f"Recieve data from Mosquitto Local: {data}")

        tb_payload = json.dumps(data)

        tb_client.publish(tb_topic, tb_payload, 1)
        print(f"Successful transport data to ThingBoard Clouds")
    except Exception as e:
        print(f"Error: {e}")

tb_client.username_pw_set(tb_device_token)
print("Connecting to ThingBoards Cloud.....")
tb_client.connect(tb_host, 1883, 60)
tb_client.loop_start()

local_client.on_connect = on_connect_local
local_client.on_message = on_message_local
local_client.connect(local_broker, local_port, 60)

try:
    print("System is working....")
    local_client.loop_forever()
except KeyboardInterrupt:
    print("Disconnecting.....")
    local_client.disconnect()
    tb_client.loop_stop()
    tb_client.disconnect()
    print("Successful disconnect")
