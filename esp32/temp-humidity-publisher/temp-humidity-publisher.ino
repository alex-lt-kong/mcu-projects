#include "configs.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_AHTX0.h>

#define SDA_PIN 4  // D4
#define SCL_PIN 5  // D5

Adafruit_AHTX0 aht;
WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect_mqtt() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    if (client.connect("NanoESP32Client", MQTT_USERNAME, MQTT_PASSWORD)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  // Initialize I²C with custom pins
  Wire.begin(SDA_PIN, SCL_PIN);

  if (!aht.begin(&Wire)) {
    Serial.println("Failed to find AHT20 sensor. Check wiring!");
    while (1) delay(10);
  }
  setup_wifi();
  client.setServer(MQTT_SERVER, MQTT_PORT);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi lost. Reconnecting...");
    setup_wifi();
  }

  if (!client.connected()) {
    reconnect_mqtt();
  }

  client.loop();

  static unsigned long lastMsg = 0;
  unsigned long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;

    sensors_event_t humidity, temp;
    if (aht.getEvent(&humidity, &temp)) {
      Serial.printf("Temperature: %.2f °C, Humidity: %.2f %\n", temp.temperature, humidity.relative_humidity);
    } else {
      Serial.println("Failed to read from DHT20");
      return;
    }

    delay(60000);

    String payload = "{";
    payload += "\"temp_celsius\": " + String(temp.temperature, 2) + ",";
    payload += "\"RH\": " + String(humidity.relative_humidity, 2);
    payload += "}";

    Serial.print("Publishing: ");
    Serial.println(payload);
    client.publish(MQTT_TOPIC, payload.c_str());
  }
}