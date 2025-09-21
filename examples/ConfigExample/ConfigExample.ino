/*
 * Config Example
 * Simple configuration storage
 */

#include "EepromShim.h"

struct MyConfig
{
    uint8_t version = 1;
    bool loaded = false;
    int value = 42;
};

void setup()
{
    Serial.begin(115200);
    while (!Serial)
        delay(10);

    Serial.println("Config Example");

    EepromShim::init();

    MyConfig defaults;
    MyConfig config = EepromShim::init(defaults, EE_INIT);

    Serial.print("Config loaded: ");
    Serial.println(config.loaded ? "YES" : "NO");
    Serial.print("Value: ");
    Serial.println(config.value);
}

void loop()
{
    // Nothing
}