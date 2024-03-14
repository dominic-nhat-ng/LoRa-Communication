import paho.mqtt.client as mqtt
from time import sleep
import random
import json

def on_connect(client, userdata, flags, rc, properties=None):
    print("Connected with result code "+str(rc))

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    print(msg.topic+" "+str(msg.payload))

client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
client.on_connect = on_connect
client.on_message = on_message

client.username_pw_set("Nhat", "Nhat")
client.connect("iot.tdlogy.com", 1883, 60)

client.loop_start()

import json
import random
while True:
    # sleep(5)
    data = {"temperature": random.randint(20, 30), "humidity": random.randint(50, 100)}
    client.publish("v1/devices/me/telemetry", json.dumps(data), 1)
    sleep(5)


# client.loop_stop()
# client.disconnect()