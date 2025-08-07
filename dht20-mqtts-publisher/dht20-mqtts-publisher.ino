#include "configs.h"

#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <Wire.h>
#include <Adafruit_AHTX0.h>

Adafruit_AHTX0 aht;
WiFiClientSecure wifi_client;
PubSubClient mqtts_client(wifi_client);

void setup_wifi(int attempts = 0) {
  const auto max_attempts = 10;
  if (attempts >= max_attempts) {
    Serial.printf("\nmax_attempts (%d) reached, restarting the device as the last resort\n", max_attempts);
    #if defined(ARDUINO_ARCH_ESP32)
    ESP.restart();
    #elif defined(ARDUINO_ARCH_RP2040)
    rp2040.reboot();
    #endif
  }
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.printf("\nConnecting to WiFi [" WIFI_SSID "] (%d/%d)\n", attempts + 1, max_attempts);

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

void reconnect_mqtts() {
  while (!mqtts_client.connected()) {
    Serial.printf("Connecting to MQTT broker " MQTT_SERVER ":%d\n", MQTT_PORT);
    if (mqtts_client.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD)) {
      Serial.println("connected");
    } else {
      const int retry_delay_ms = 10 * 1000;
      Serial.print("mqtts_client.connect() failed, mqtts_client.state() = ");
      Serial.print(mqtts_client.state());
      Serial.printf(" retrying in %d seconds\n", retry_delay_ms / 1000);
      delay(retry_delay_ms);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  // Initialize I²C with custom pins
  #if defined(ARDUINO_ARCH_ESP32)
  Serial.println("ARDUINO_ARCH_ESP32 defined");
  #define SDA_PIN D4  // D4
  #define SCL_PIN D5  // D5
  Wire.begin(SDA_PIN, SCL_PIN);
  #elif defined(ARDUINO_ARCH_RP2040)
  Serial.println("ARDUINO_ARCH_RP2040 defined");
  Wire.setSDA(0);  // GP0
  Wire.setSCL(1);  // GP1
  Wire.begin();
  #endif

  if (!aht.begin(&Wire)) {
    Serial.println("Failed to find AHT20 sensor. Check wiring!");
    while (1) delay(1000);
  }
  setup_wifi();
  wifi_client.setCACert(root_ca);
  mqtts_client.setServer(MQTT_SERVER, MQTT_PORT);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi lost. Reconnecting...");
    setup_wifi();
  }

  if (!mqtts_client.connected()) {
    reconnect_mqtts();
  }

  mqtts_client.loop();

  static unsigned long lastMsg = INT_MIN;
  unsigned long now = millis();
  if (now - lastMsg < 60000 * 5) return;

  lastMsg = now;

  sensors_event_t humidity, temp;
  if (aht.getEvent(&humidity, &temp)) {
    Serial.printf("Temperature: %.1f °C, Humidity: %.1f %\n", temp.temperature, humidity.relative_humidity);
  } else {
    Serial.println("Failed to read from DHT20");
    return;
  }

  delay(1000);

  String payload = "{";
  payload += "\"temp_celsius\": " + String(temp.temperature, 1) + ",";
  payload += "\"RH\": " + String(humidity.relative_humidity, 0);
  payload += "}";

  Serial.printf("Publishing to topic %s: ", MQTT_TOPIC);
  Serial.println(payload);
  mqtts_client.publish(MQTT_TOPIC, payload.c_str());
}