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

WiFiClient espClient;
PubSubClient client(espClient);

#define R 5
#define G 4
#define B 2

#define DHTPIN 32
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

float TEMP_MIN = 0;
float TEMP_MAX = 0;

void reconnect() {
  while (!client.connected()) {
    if (client.connect(ID, token, "")) {
      client.subscribe("v1/devices/me/rpc/request/+"); // Subscribe RPC
    } else {
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println("Received: " + message);

  if (message.indexOf("\"method\":\"setTempMin\"") != -1) {
    int paramsMinIndex = message.indexOf("\"params\":");
    if (paramsMinIndex != -1) {
      int valueStart = message.indexOf(":", paramsMinIndex) + 1;
      TEMP_MIN = message.substring(valueStart, message.indexOf("}", paramsMinIndex)).toFloat();
      Serial.printf("Updated Temp Min: %.2f\n", TEMP_MIN);
    }
  } else if (message.indexOf("\"method\":\"setTempMax\"") != -1) {
    int paramsMaxIndex = message.indexOf("\"params\":");
    if (paramsMaxIndex != -1) {
      int valueStart = message.indexOf(":", paramsMaxIndex) + 1;
      TEMP_MAX = message.substring(valueStart, message.indexOf("}", paramsMaxIndex)).toFloat();
      Serial.printf("Updated Temp Max: %.2f\n", TEMP_MAX);
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
  delay(5000);
}


void setup() {
  Serial.begin(115200);
  
  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Connected!");

  client.setServer(mqtt_server, port);
  client.setCallback(callback);

  dht.begin();

  reconnect();

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
  Serial.printf("TEMP_MIN: %.2f, TEMP_MAX: %.2f\n", TEMP_MIN, TEMP_MAX);

  delay(500);
}
