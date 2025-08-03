#include <Wire.h>
#include <Adafruit_AHTX0.h>

Adafruit_AHTX0 aht;

void setup() {
  Serial.begin(115200);
  delay(1000);  // Wait for Serial to initialize

  Wire.setSDA(0);  // GP0
  Wire.setSCL(1);  // GP1
  Wire.begin();

  if (!aht.begin()) {
    Serial.println("Could not find AHT20 sensor. Check wiring!");
    while (1) delay(1000);
  }

  Serial.println("AHT20 sensor initialized.");
}

void loop() {
  sensors_event_t humidity, temperature;
  aht.getEvent(&humidity, &temperature);  // Read sensor values

  Serial.print("Temperature: ");
  Serial.print(temperature.temperature);
  Serial.println(" °C");

  Serial.print("Humidity: ");
  Serial.print(humidity.relative_humidity);
  Serial.println(" %");

  delay(2000);  // Wait before next reading
}
