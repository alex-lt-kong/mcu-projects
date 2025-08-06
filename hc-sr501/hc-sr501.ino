#if defined(ARDUINO_ARCH_ESP32)
#define PIN_TO_SENSOR D2  // The Arduino Nano ESP32 pin connected to OUTPUT pin of sensor
#elif defined(ARDUINO_ARCH_RP2040)
#define PIN_TO_SENSOR 1  // Pico W GP1
#else
#error "Define ARDUINO_ARCH_ESP32 or ARDUINO_ARCH_RP2040"
#endif

int motion_state = LOW;       // current state of pin
int prev_motion_state = LOW;  // previous state of pin

void setup() {
  Serial.begin(115200);
  delay(2000);
  #if defined(ARDUINO_ARCH_ESP32)
  Serial.println("ARDUINO_ARCH_ESP32 defined");
  #elif defined(ARDUINO_ARCH_RP2040)
  Serial.println("ARDUINO_ARCH_RP2040 defined");
  #endif


  pinMode(PIN_TO_SENSOR, INPUT);  // set Arduino Nano ESP32 pin to input mode to read value from OUTPUT pin of sensor
}

void loop() {
  motion_state = digitalRead(PIN_TO_SENSOR);
  Serial.printf("motion_state: %d\n", motion_state);
  delay(2000);
}
