#include <EEPROM.h>

void setup() {

  //Start serial
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.print("EEPROM length: ");
  Serial.println(EEPROM.length());

  for (int index = 0; index < EEPROM.length(); ++index) {
    if (index % 16 == 0) {
      Serial.print(decToHex(index, 4));
    }
    Serial.print(" ");
    Serial.print(decToHex(EEPROM[index], 2));
    if (index % 16 == 15) {
      Serial.println();
    }
  }
}

void loop() {
  // empty loop
}

String decToHex(long decValue, byte desiredStringLength) {

  String hexString = String(decValue, HEX);
  while (hexString.length() < desiredStringLength) hexString = "0" + hexString;

  return hexString;
}
