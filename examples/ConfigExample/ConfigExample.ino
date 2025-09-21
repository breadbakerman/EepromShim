/*
 * Config Example
 * Simple configuration storage using basic functions
 */

#include "EepromShim.h"

void setup()
{
    Serial.begin(115200);
    while (!Serial)
        delay(10);

    Serial.println("Config Example");

    EepromShim::init();

    // Write a simple config value
    int myValue = 123;
    EepromShim::put(0, myValue);
    Serial.println("Config saved");

    // Read it back
    int readValue;
    EepromShim::get(0, readValue);
    Serial.print("Config value: ");
    Serial.println(readValue);
}

void loop()
{
    // Nothing
}