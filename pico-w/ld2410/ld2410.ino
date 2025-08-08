#define sensorSerial Serial1


// User defines
// #define DEBUG_MODE
#define ENHANCED_MODE
#define SERIAL_BAUD_RATE 115200

//Change the communication baud rate here, if necessary
//#define LD2410_BAUD_RATE 256000
#include "MyLD2410.h"

#ifdef DEBUG_MODE
MyLD2410 sensor(sensorSerial, true);
#else
MyLD2410 sensor(sensorSerial);
#endif

unsigned long nextPrint = 0, printEvery = 1000;  // print every second

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

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  sensorSerial.setTX(0);
  sensorSerial.setRX(1);
  sensorSerial.begin(LD2410_BAUD_RATE);


  delay(2000);
  Serial.println(__FILE__);
  if (!sensor.begin()) {
    Serial.println("Failed to communicate with the sensor.");
    while (true) {}
  }

#ifdef ENHANCED_MODE
  sensor.enhancedMode();
#else
  sensor.enhancedMode(false);
#endif

  delay(nextPrint);
}

void loop() {
  if ((sensor.check() == MyLD2410::Response::DATA) && (millis() > nextPrint)) {
    nextPrint = millis() + printEvery;
    printData();
  }
}