/*
 * BasicEeprom Example
 *
 * This example demonstrates basic usage of the EepromShim library
 * for reading and writing simple data types to EEPROM/Flash memory.
 *
 * Compatible with:
 * - Arduino AVR boards (uses built-in EEPROM)
 * - Adafruit SAMD51 boards (uses QSPI Flash)
 */

#include "EepromShim.h"

// EEPROM addresses for our data
const int INT_ADDR = 0;
const int FLOAT_ADDR = 4;
const int BYTE_ADDR = 8;
const int STRING_ADDR = 16;

void setup()
{
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  Serial.println("EepromShim Basic Example");
  Serial.println("========================");

  // Initialize EEPROM
  EepromShim::init();

// Check if flash is working (SAMD51 only)
#ifdef ARDUINO_SAMD_ADAFRUIT
  if (!EepromShim::checkFlash())
  {
    Serial.println("ERROR: Flash memory not working!");
    while (1)
      delay(100);
  }
  Serial.println("Flash memory OK");
#endif

  Serial.println("\n1. Writing test data...");

  // Write different data types
  int testInt = 12345;
  float testFloat = 3.14159;
  byte testByte = 200;
  String testString = "Hello EEPROM!";

  EepromShim::put(INT_ADDR, testInt);
  EepromShim::put(FLOAT_ADDR, testFloat);
  EepromShim::put(BYTE_ADDR, testByte);

  // Write string character by character
  for (int i = 0; i < testString.length(); i++)
  {
    EepromShim::write(STRING_ADDR + i, testString[i]);
  }
  EepromShim::write(STRING_ADDR + testString.length(), 0); // null terminator

  Serial.println("Data written successfully!");

  Serial.println("\n2. Reading back data...");

  // Read back the data
  int readInt;
  float readFloat;
  byte readByte;
  char readString[32];

  EepromShim::get(INT_ADDR, readInt);
  EepromShim::get(FLOAT_ADDR, readFloat);
  EepromShim::get(BYTE_ADDR, readByte);

  // Read string character by character
  for (int i = 0; i < 31; i++)
  {
    readString[i] = EepromShim::read(STRING_ADDR + i);
    if (readString[i] == 0)
      break;
  }
  readString[31] = 0; // ensure null termination

  Serial.println("\n3. Results:");
  Serial.printf("Integer: %d (expected: %d)\n", readInt, testInt);
  Serial.printf("Float: %.5f (expected: %.5f)\n", readFloat, testFloat);
  Serial.printf("Byte: %d (expected: %d)\n", readByte, testByte);
  Serial.printf("String: '%s' (expected: '%s')\n", readString, testString.c_str());

  // Verify data integrity
  bool success = (readInt == testInt) &&
                 (abs(readFloat - testFloat) < 0.0001) &&
                 (readByte == testByte) &&
                 (strcmp(readString, testString.c_str()) == 0);

  if (success)
  {
    Serial.println("\n✓ All data verified successfully!");
  }
  else
  {
    Serial.println("\n✗ Data verification failed!");
  }

  Serial.println("\nExample complete. Memory operations working correctly.");
}

void loop()
{
  // Nothing to do in loop
  delay(1000);
}