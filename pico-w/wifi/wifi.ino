#include "configs.h"

#include <WiFi.h>

void setup_wifi(int attempts = 0) {
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

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  delay(2000);  // Wait for Serial Monitor to initialize
  Serial.println("Starting Pico W Wi-Fi connection...");

  // Set Wi-Fi to station mode
  setup_wifi();

  // Connect to Wi-Fi
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
}

void loop() {
  // Check if still connected to Wi-Fi
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Still connected to Wi-Fi");
  } else {
    Serial.println("Disconnected from Wi-Fi. Reconnecting...");
    setup_wifi();
  }
  delay(2000);  // Check every 2 seconds
}
