/*
 * BasicEeprom Example
 * Simple read/write to EEPROM
 */

#include "EepromShim.h"

void setup()
{
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  Serial.println("Basic EEPROM Example");

  // Initialize EEPROM
  EepromShim::init();

  // Write a byte
  EepromShim::write(0, 42);
  Serial.println("Wrote 42 to address 0");

  // Read it back
  int value = EepromShim::read(0);
  Serial.print("Read back: ");
  Serial.println(value);
}

void loop()
{
  // Nothing
}