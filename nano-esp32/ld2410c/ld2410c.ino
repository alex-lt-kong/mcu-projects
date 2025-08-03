#include <ld2410.h>

#include <Wire.h>

#define MONITOR_SERIAL Serial
#define RADAR_SERIAL Serial2
#define RADAR_RX_PIN D5
#define RADAR_TX_PIN D4

ld2410 radar;

void setup() {
  MONITOR_SERIAL.begin(115200); // Feedback over Serial Monitor
  // radar.debug(MONITOR_SERIAL); Uncomment to show debug information from the library on the Serial Monitor. By default this does not show sensor reads as they are very frequent.
 
  RADAR_SERIAL.begin(256000, SERIAL_8N1, RADAR_RX_PIN, RADAR_TX_PIN); // UART for monitoring the radar
  delay(2000);
  MONITOR_SERIAL.print(F("\nConnect LD2410 radar TX to GPIO:"));
  MONITOR_SERIAL.println(RADAR_RX_PIN);
  MONITOR_SERIAL.print(F("Connect LD2410 radar RX to GPIO:"));
  MONITOR_SERIAL.println(RADAR_TX_PIN);
  MONITOR_SERIAL.print(F("LD2410 radar sensor initialising: "));
 
  if (radar.begin(RADAR_SERIAL))
  {
    MONITOR_SERIAL.println(F("OK"));
    MONITOR_SERIAL.print(F("LD2410 firmware version: "));
    MONITOR_SERIAL.print(radar.firmware_major_version);
    MONITOR_SERIAL.print('.');
    MONITOR_SERIAL.print(radar.firmware_minor_version);
    MONITOR_SERIAL.print('.');
    MONITOR_SERIAL.println(radar.firmware_bugfix_version, HEX);
  }
  else
  {
    MONITOR_SERIAL.println(F("not connected"));
  }
}

void loop() {

}