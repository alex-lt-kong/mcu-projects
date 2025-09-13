#include "configs.h"

#include <ArduinoJson.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <microDS18B20.h>

// GPIO 22 on PicoW
MicroDS18B20<22> sensor;

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
  sensor.requestTemp();

  JsonDocument jsonDoc;
  jsonDoc["temp_celsius"] = round(sensor.getTemp() * 10) / 10.0;;

  String output;
  serializeJson(jsonDoc, output);
  Serial.printf("Publishing payload [%s] to topic [%s]... ", output.c_str(), MQTT_TOPIC);
  
  std::string msgPackStr;
  size_t size = serializeMsgPack(jsonDoc, msgPackStr);

  if (mqtts_client.publish(MQTT_TOPIC, msgPackStr.c_str())) {
    Serial.println("done");
  } else {
    Serial.println("failed!");
  }
  delay(60000);
}
