#include "DHT.h"
#include "PubSubClient.h"
#include "ESP8266WiFi.h"
#include <Wire.h> 
#include <Adafruit_BMP280.h>
#include <ArduinoJson.h>

#include "credentials.h"

#define DHTPIN D5
#define DHTTYPE DHT22 
DHT dht(DHTPIN, DHTTYPE);

#define BMP280_I2C_ADDRESS  0x76
Adafruit_BMP280 bmp280; // I2C

const char* ssid = SSID;               
const char* wifi_password = WIFI_PASSWORD;

const char* mqtt_rpi_server = MQTT_RPI_SERVER;
const char* mqtt_rpi_username = MQTT_RPI_USERNAME;
const char* mqtt_rpi_password = MQTT_RPI_PASSWORD;
const char* rpi_topic = "estacao/7429d76e-730f-4446-8bbb-fde933c511af/sensores";
const char* clientID = "client_estacaometeorologica";

int interval_seconds = 10; 

WiFiClient wifiClient;
PubSubClient rpi_client(mqtt_rpi_server, MQTT_RPI_PORT, wifiClient); 

void connect_wifi() {
  Serial.print("Conectando ao ");
  Serial.println(ssid);

  WiFi.begin(ssid, wifi_password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("WiFi conectado!");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensagem recebida [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect_rpi_mqtt() {
  while (!rpi_client.connected()) {
    Serial.print("Tentando conexão MQTT do Raspberry Pi...");
    if (rpi_client.connect(clientID, mqtt_rpi_username, mqtt_rpi_password)) {
      Serial.println("Conectado");
    } else {
      Serial.print("failed, rc=");
      Serial.print(rpi_client.state());
      Serial.println(" tente novamente em 5 segundos");
      delay(5000);
    }
  }
}

void setup() {
  Wire.begin();
  Serial.begin(9600); 
  dht.begin();
  bmp280.begin(BMP280_I2C_ADDRESS);

  connect_wifi();
  delay(3000);
  rpi_client.setServer(mqtt_rpi_server, MQTT_RPI_PORT);
  rpi_client.setCallback(callback);
}

void loop() {
  delay(1000);
  if (!rpi_client.connected()) {
    reconnect_rpi_mqtt();
  }

  float h = dht.readHumidity(); // %
  Serial.print("DHT22 Umidade: "); 
  Serial.print(h);
  Serial.println(" %");
  float t = dht.readTemperature(); // °C
  Serial.print("DHT22 Temperatura: "); 
  Serial.print(t);
  Serial.println(" *C");

  float temp = bmp280.readTemperature(); // °C
  Serial.print("BMP280 Temperatura: "); 
  Serial.print(temp);
  Serial.println(" *C");
  float pressure = bmp280.readPressure(); // Pa   
  Serial.print("BMP280 Pressão: "); 
  Serial.print(pressure);
  Serial.println(" Pa");

  const size_t capacity = JSON_ARRAY_SIZE(3) + 3*JSON_OBJECT_SIZE(3);
  StaticJsonDocument<capacity> doc;

  JsonArray data = doc.to<JsonArray>();

  JsonObject obj1 = data.createNestedObject();
  obj1["variable"] = "umidade";
  obj1["unit"] = "%";
  obj1["value"] = String(h);

  JsonObject obj2 = data.createNestedObject();
  obj2["variable"] = "temperatura";
  obj2["unit"] = "C";
  obj2["value"] = String((t + temp) / 2);

  JsonObject obj3 = data.createNestedObject();
  obj3["variable"] = "pressao_atmosferica";
  obj3["unit"] = "Pa";
  obj3["value"] = String(pressure);

  String payload;
  serializeJson(doc, payload);

  if (rpi_client.publish(rpi_topic, payload.c_str())) {
    Serial.println("Dados enviados para o Raspberry Pi!");
  }

  delay(1000 * interval_seconds);  
}