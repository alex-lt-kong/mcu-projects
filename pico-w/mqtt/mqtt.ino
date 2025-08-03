#include "configs.h"

#include <WiFi.h>            
#include <PubSubClient.h>   

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

unsigned long lastMqttReconnectAttempt = 0;

// Forward declarations
void setup_wifi(int attempts = 0);
void reconnect_mqtt();

void setup_wifi(int attempts) {
  const auto max_attempts = 3;
  if (attempts >= max_attempts) {
    Serial.printf("\nmax_attempts (%d) reached, restarting the device as the last resort\n", max_attempts);
    rp2040.reboot();
  }
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.printf("\nConnecting to WiFi " WIFI_SSID " (%d/%d)\n", attempts + 1, max_attempts);

  for (int i = 0; i < 60; ++i) {
    delay(1000);
    Serial.print(".");
    if (WiFi.status() != WL_CONNECTED) continue;

    Serial.println("\nWiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    return;
  }
  setup_wifi(attempts + 1);
}

// MQTT reconnect function
void reconnect_mqtt() {
  // Loop until reconnected
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect (add username/password if needed)
    if (mqttClient.connect("PicoWClient", MQTT_USERNAME, MQTT_PASSWORD)) {
      Serial.println("connected!");
      // Publish a sample payload
      mqttClient.publish(MQTT_TOPIC, "Hello from Pico W!");
      // (You can also subscribe to topics if you want)
      // mqttClient.subscribe("your/subscribe/topic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("Starting Pico W Wi-Fi connection...");

  setup_wifi();

  // Set MQTT server and port
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  // Optional: set callback function if you want to receive messages
  // mqttClient.setCallback(your_callback_function);
}

void loop() {
  // Ensure WiFi is connected
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Disconnected from Wi-Fi. Reconnecting...");
    setup_wifi();
  }

  // Ensure MQTT is connected
  if (!mqttClient.connected()) {
    reconnect_mqtt();
  }

  // Let the MQTT client process incoming messages and maintain connection
  mqttClient.loop();

  // Publish a sample payload every 10 seconds
  static unsigned long lastPublish = 0;
  if (millis() - lastPublish > 10000) {
    String payload = String("Pico W uptime: ") + millis()/1000 + "s";
    mqttClient.publish(MQTT_TOPIC, payload.c_str());
    
    Serial.print("Published: ");
    Serial.println(payload);
    lastPublish = millis();
  }

  delay(100);
}