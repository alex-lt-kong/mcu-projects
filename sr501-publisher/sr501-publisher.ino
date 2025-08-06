#include "configs.h"

#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>

#if defined(ARDUINO_ARCH_ESP32)
#define PIN_TO_SENSOR D2  // The Arduino Nano ESP32 pin connected to OUTPUT pin of sensor
#elif defined(ARDUINO_ARCH_RP2040)
#define PIN_TO_SENSOR 1  // Pico W GP1
#else
#error "Define ARDUINO_ARCH_ESP32 or ARDUINO_ARCH_RP2040"
#endif

WiFiClient wifi_client;
PubSubClient mqtt_client(wifi_client);

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

void reconnect_mqtt() {
  while (!mqtt_client.connected()) {
    Serial.printf("Connecting to MQTT broker " MQTT_SERVER ":%d\n", MQTT_PORT);
    if (mqtt_client.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD)) {
      Serial.println("connected");
    } else {
      const int retry_delay_ms = 10 * 1000;
      Serial.print("mqtt_client.connect() failed, mqtt_client.state() = ");
      Serial.print(mqtt_client.state());
      Serial.printf(" retrying in %d seconds", retry_delay_ms / 1000);
      delay(retry_delay_ms);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(2000);
#if defined(ARDUINO_ARCH_ESP32)
  Serial.println("ARDUINO_ARCH_ESP32 defined");
#elif defined(ARDUINO_ARCH_RP2040)
  Serial.println("ARDUINO_ARCH_RP2040 defined");
#endif

  setup_wifi();
  mqtt_client.setServer(MQTT_SERVER, MQTT_PORT);

  pinMode(PIN_TO_SENSOR, INPUT);  // set Arduino Nano ESP32 pin to input mode to read value from OUTPUT pin of sensor
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi lost. Reconnecting...");
    setup_wifi();
  }

  if (!mqtt_client.connected()) {
    reconnect_mqtt();
  }

  mqtt_client.loop();

  const auto motion_state = digitalRead(PIN_TO_SENSOR);

  String payload = "{";
  payload += "\"motion_state\": " + String(motion_state);
  payload += "}";

  Serial.printf("Publishing to topic %s: ", MQTT_TOPIC);
  Serial.println(payload);
  mqtt_client.publish(MQTT_TOPIC, payload.c_str());

  delay(30000);
}
