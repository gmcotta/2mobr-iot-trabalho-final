import paho.mqtt.client as mqtt
import csv
from dotenv import load_dotenv
import datetime
import json
import os

load_dotenv()

MQTT_ADDRESS = os.getenv("MQTT_ADDRESS")
MQTT_PORT = os.getenv("MQTT_PORT")
MQTT_USER = os.getenv("MQTT_USER")
MQTT_PASSWORD = os.getenv("MQTT_PASSWORD")
MQTT_TOPIC = "estacao/+/+"

def on_connect(client, userdata, flags, rc):
    print("Conectado com o resultado " + str(rc))
    client.subscribe(MQTT_TOPIC)

def on_message(client, userdata, msg):
    json_payload = json.loads(msg.payload)
    
    now = datetime.datetime.now().strftime("%d-%m-%Y %H:%M:%S")
    for obj in json_payload:
        obj["datetime"] = now

    print(msg.topic)
    print(json.dumps(json_payload))
    print("")

    device_id = msg.topic.split("/")[1]
    today = datetime.datetime.today().strftime("%d-%m-%Y")
    csv_file_path = f"{device_id}_{today}.csv"

    add_to_csv(csv_file_path, json_payload)


def add_to_csv(file_path, payload):
    with open(file_path, mode='a') as file:
        writer = csv.DictWriter(file, fieldnames=payload[0].keys())

        # Check if the file is empty
        file.seek(0, 2)  # Move the cursor to the end of the file
        file_empty = file.tell() == 0

        # Write header only if the file is empty
        if file_empty:
            writer.writeheader()

        # Write data to the CSV file
        for row in payload:
            writer.writerow(row)

def main():
    mqtt_client = mqtt.Client()
    mqtt_client.username_pw_set(MQTT_USER, MQTT_PASSWORD)
    mqtt_client.on_connect = on_connect
    mqtt_client.on_message = on_message

    mqtt_client.connect(MQTT_ADDRESS, int(MQTT_PORT))
    mqtt_client.loop_forever()

if __name__ == "__main__":
    main()
