#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* mqtt_server = "demo.thingsboard.io";
const char* ID = "2da86770-f6a6-11ef-8e00-e370a74757c3";
const char* token = "eN7Ny6EDpSzARiDrxU9E";
const int port = 1883;

#define R 5
#define G 4
#define B 2

#define DHTPIN 32
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#define TEMP_MIN 17.22  
#define TEMP_MAX 36.11  

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Connected!");
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect(ID, token, "")) {
      Serial.println("Connected to MQTT Broker");
    } else {
      delay(5000);
    }
  }
}

void setColor(int red, int green, int blue) {
  analogWrite(R, red);
  analogWrite(G, green);
  analogWrite(B, blue);
}

void sendTemperature(float temp, String colorHex) {
  String payload = "{\"temperature\": " + String(temp, 1) + ", \"led_color\": \"" + colorHex + "\"}";
  client.publish("v1/devices/me/telemetry", payload.c_str());
  Serial.println("Sent to ThingsBoard: " + payload);
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, port);
  dht.begin();

  pinMode(R, OUTPUT);
  pinMode(G, OUTPUT);
  pinMode(B, OUTPUT);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  float temp = dht.readTemperature();
  String colorHex = "#FFFFFF";
  
  if (temp > TEMP_MAX) {
    setColor(255, 0, 0);
    colorHex = "#FF0000";
  } else if (temp < TEMP_MIN) {
    setColor(0, 0, 255);
    colorHex = "#0000FF";
  } else {
    setColor(0, 255, 0);
    colorHex = "#00FF00";
  }

  sendTemperature(temp, colorHex);

  delay(5000); 
}
