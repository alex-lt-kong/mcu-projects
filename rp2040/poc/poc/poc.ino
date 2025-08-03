#include <WiFi.h>

// Replace with your Wi-Fi credentials
const char* ssid = "Mamsds-Hotspot-2.4G";     // Your Wi-Fi network name
const char* password = "(83721363!)"; // Your Wi-Fi password

void setup() {
    // Initialize Serial Monitor
    Serial.begin(115200);
    delay(2000); // Wait for Serial Monitor to initialize
    Serial.println("Starting Pico W Wi-Fi connection...");

    // Initialize the built-in LED
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW); // LED off initially

    // Set Wi-Fi to station mode
    WiFi.mode(WIFI_STA);

    // Connect to Wi-Fi
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);

    // Wait for connection
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    // Connection successful
    Serial.println("");
    Serial.println("Wi-Fi connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    // Blink LED to indicate connection
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);
    digitalWrite(LED_BUILTIN, LOW);
}

void loop() {
    // Check if still connected to Wi-Fi
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Still connected to Wi-Fi");
        digitalWrite(LED_BUILTIN, HIGH); // LED on
        delay(500);
        digitalWrite(LED_BUILTIN, LOW);  // LED off
        delay(500);
    } else {
        Serial.println("Disconnected from Wi-Fi. Reconnecting...");
        //WiFi.reconnect();
        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            Serial.print(".");
        }
        Serial.println("Reconnected!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
    }
    delay(2000); // Check every 2 seconds
}