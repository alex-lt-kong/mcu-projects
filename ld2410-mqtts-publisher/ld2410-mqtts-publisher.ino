#include "configs.h"
#define sensorSerial Serial1

#define SERIAL_BAUD_RATE 115200

//Change the communication baud rate here, if necessary
//#define LD2410_BAUD_RATE 256000
#include <MyLD2410.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <Wire.h>

#ifdef DEBUG_MODE
MyLD2410 sensor(sensorSerial, true);
#else
MyLD2410 sensor(sensorSerial);
#endif

unsigned long next_print_at = 0, interval_ms = 1000;

void printValue(const byte &val) {
  Serial.print(' ');
  Serial.print(val);
}

void printData() {
  Serial.print(sensor.statusString());
  if (sensor.presenceDetected()) {
    Serial.print(", distance: ");
    Serial.print(sensor.detectedDistance());
    Serial.print("cm");
  }
  Serial.println();
  if (sensor.movingTargetDetected()) {
    Serial.print(" MOVING    = ");
    Serial.print(sensor.movingTargetSignal());
    Serial.print("@");
    Serial.print(sensor.movingTargetDistance());
    Serial.print("cm ");
    if (sensor.inEnhancedMode()) {
      Serial.print("\n signals->[");
      sensor.getMovingSignals().forEach(printValue);
      Serial.print(" ] thresholds:[");
      sensor.getMovingThresholds().forEach(printValue);
      Serial.print(" ]");
    }
    Serial.println();
  }
  if (sensor.stationaryTargetDetected()) {
    Serial.print(" STATIONARY= ");
    Serial.print(sensor.stationaryTargetSignal());
    Serial.print("@");
    Serial.print(sensor.stationaryTargetDistance());
    Serial.print("cm ");
    if (sensor.inEnhancedMode()) {
      Serial.print("\n signals->[");
      sensor.getStationarySignals().forEach(printValue);
      Serial.print(" ] thresholds:[");
      sensor.getStationaryThresholds().forEach(printValue);
      Serial.print(" ]");
    }
    Serial.println();
  }

  if (sensor.inEnhancedMode() && (sensor.getFirmwareMajor() > 1)) {
    Serial.print("Light level: ");
    Serial.println(sensor.getLightLevel());
    Serial.print("Output level: ");
    Serial.println((sensor.getOutLevel()) ? "HIGH" : "LOW");
  }

  Serial.println();
}

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
    Serial.printf("Connecting to MQTT broker " MQTT_SERVER ":%d with MQTT_CLIENT_ID: " MQTT_CLIENT_ID "\n", MQTT_PORT);
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
  Serial.begin(SERIAL_BAUD_RATE);
#if defined(ARDUINO_ARCH_ESP32)
  sensorSerial.setPins(0, 1);  // D0, D1 on Nano ESP32
#elif defined(ARDUINO_ARCH_RP2040)
  sensorSerial.setTX(0);  // GP0
  sensorSerial.setRX(1);  // GP1
#else
#error "Define ARDUINO_ARCH_ESP32 or ARDUINO_ARCH_RP2040"
#endif
  sensorSerial.begin(LD2410_BAUD_RATE);


  delay(2000);
  Serial.println(__FILE__);
  if (!sensor.begin()) {
    Serial.println("Failed to communicate with the sensor.");
    while (true) {}
  }


  //  enhanced (engineering) modes.
  sensor.enhancedMode();


  delay(next_print_at);
  setup_wifi();
  // wifi_client.setInsecure();
  wifi_client.setCACert(root_ca);
  mqtts_client.setServer(MQTT_SERVER, MQTT_PORT);
}

void loop() {
  //delay(5000);
  if ((sensor.check() == MyLD2410::Response::DATA) && (millis() > next_print_at)) {
    next_print_at = millis() + interval_ms;
    const auto movingTargetDetected = sensor.movingTargetDetected();
    const auto stationaryTargetDetected = sensor.stationaryTargetDetected();
    String payload = "{";
    payload += "\"stationary_target_detected\": " + String(stationaryTargetDetected ? '1' : '0') + ", ";
    if (stationaryTargetDetected) {
      payload += "\"stationary_target_distance_cm\": " + String(sensor.stationaryTargetDistance()) + ", ";
    }
    payload += "\"moving_target_detected\": " + String(movingTargetDetected ? '1' : '0');
    if (movingTargetDetected) {
      payload += +" ,";
      payload += "\"moving_target_distance_cm\": " + String(sensor.stationaryTargetDistance());
    }

    payload += "}";


    if (!mqtts_client.connected()) {
      reconnect_mqtts();
    }

    Serial.printf("Publishing to topic %s: ", MQTT_TOPIC);
    Serial.println(payload);
    mqtts_client.publish(MQTT_TOPIC, payload.c_str());
  }
}