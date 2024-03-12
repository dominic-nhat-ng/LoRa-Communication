import paho.mqtt.client as mqtt
import json
import random
import time

class MQTTClient:
    def __init__(self, username, password, server, port):
        self.client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
        self.client.on_connect = self.on_connect
        self.client.on_message = self.on_message
        self.client.username_pw_set(username=username, password=password)
        self.client.connect(server, port, 60)
        self.client.loop_start()

    def on_connect(self, client, userdata, flags, rc, properties=None):
        print("Connected with result code "+str(rc))

    def on_message(self, client, userdata, msg):
        print(msg.topic+" "+str(msg.payload))

    def publish_data(self):
        while True:
            # Generate a random temperature value
            data = {"temperature": random.randint(0, 100)}
            self.client.publish('v1/devices/me/telemetry', json.dumps(data), 1)
            time.sleep(5)

# Usage
            
if __name__ == "__main__":
    mqtt_client = MQTTClient("Nhat", "Nhat", "iot.tdlogy.com", 1883)
    mqtt_client.publish_data()