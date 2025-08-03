/*
 * This Arduino Nano ESP32 code was developed by newbiely.com
 *
 * This Arduino Nano ESP32 code is made available for public use without any restriction
 *
 * For comprehensive instructions and wiring diagrams, please visit:
 * https://newbiely.com/tutorials/arduino-nano-esp32/arduino-nano-esp32-motion-sensor
 */

#define PIN_TO_SENSOR D2  // The Arduino Nano ESP32 pin connected to OUTPUT pin of sensor

int motion_state = LOW;       // current state of pin
int prev_motion_state = LOW;  // previous state of pin

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("setup()");
  pinMode(PIN_TO_SENSOR, INPUT);  // set Arduino Nano ESP32 pin to input mode to read value from OUTPUT pin of sensor
}

void loop() {
  prev_motion_state = motion_state;           // store old state
  motion_state = digitalRead(PIN_TO_SENSOR);  // read new state
  Serial.println("Motion detected!");
  if (prev_motion_state == LOW && motion_state == HIGH) {  // pin state change: LOW -> HIGH
    Serial.println("Motion detected!");
    // TODO: turn on alarm, light or activate a device ... here
  } else if (prev_motion_state == HIGH && motion_state == LOW) {  // pin state change: HIGH -> LOW
    Serial.println("Motion stopped!");
    // TODO: turn off alarm, light or deactivate a device ... here
  }
  delay(1000);
}
