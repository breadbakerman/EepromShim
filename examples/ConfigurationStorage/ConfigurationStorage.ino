/*
 * ConfigurationStorage Example
 *
 * This example demonstrates advanced usage of the EepromShim library
 * for storing and retrieving configuration structures, including
 * validation and default settings.
 *
 * Features demonstrated:
 * - Storing complex data structures
 * - Configuration validation with magic numbers
 * - Default configuration loading
 * - Configuration versioning
 *
 * Compatible with:
 * - Arduino AVR boards (uses built-in EEPROM)
 * - Adafruit SAMD51 boards (uses QSPI Flash)
 */

#include "EepromShim.h"

// Configuration structure
struct AppConfig
{
    uint32_t magic;      // Magic number for validation
    uint16_t version;    // Configuration version
    char deviceName[32]; // Device name
    uint8_t brightness;  // LED brightness (0-255)
    float sensorOffset;  // Sensor calibration offset
    bool enableLogging;  // Enable data logging
    uint16_t sampleRate; // Sample rate in Hz
    uint8_t checksum;    // Simple checksum
};

const uint32_t CONFIG_MAGIC = 0xC0FF1234;
const uint16_t CONFIG_VERSION = 1;
const int CONFIG_ADDR = 0;

// Default configuration
AppConfig defaultConfig = {
    .magic = CONFIG_MAGIC,
    .version = CONFIG_VERSION,
    .deviceName = "MyDevice",
    .brightness = 128,
    .sensorOffset = 0.0,
    .enableLogging = true,
    .sampleRate = 100,
    .checksum = 0};

// Calculate simple checksum for configuration
uint8_t calculateChecksum(const AppConfig &config)
{
    uint8_t checksum = 0;
    const uint8_t *data = (const uint8_t *)&config;

    // Checksum all bytes except the checksum field itself
    for (size_t i = 0; i < sizeof(AppConfig) - 1; i++)
    {
        checksum ^= data[i];
    }

    return checksum;
}

// Load configuration from EEPROM
bool loadConfig(AppConfig &config)
{
    EepromShim::get(CONFIG_ADDR, config);

    // Validate magic number and version
    if (config.magic != CONFIG_MAGIC)
    {
        Serial.println("Invalid magic number - loading defaults");
        return false;
    }

    if (config.version != CONFIG_VERSION)
    {
        Serial.printf("Version mismatch (got %d, expected %d) - loading defaults\n",
                      config.version, CONFIG_VERSION);
        return false;
    }

    // Validate checksum
    uint8_t expectedChecksum = calculateChecksum(config);
    if (config.checksum != expectedChecksum)
    {
        Serial.printf("Checksum mismatch (got 0x%02X, expected 0x%02X) - loading defaults\n",
                      config.checksum, expectedChecksum);
        return false;
    }

    Serial.println("Configuration loaded successfully from EEPROM");
    return true;
}

// Save configuration to EEPROM
void saveConfig(AppConfig &config)
{
    // Update checksum before saving
    config.checksum = calculateChecksum(config);

    EepromShim::put(CONFIG_ADDR, config);
    Serial.println("Configuration saved to EEPROM");
}

// Print current configuration
void printConfig(const AppConfig &config)
{
    Serial.println("\nCurrent Configuration:");
    Serial.println("=====================");
    Serial.printf("Magic: 0x%08X\n", config.magic);
    Serial.printf("Version: %d\n", config.version);
    Serial.printf("Device Name: %s\n", config.deviceName);
    Serial.printf("Brightness: %d\n", config.brightness);
    Serial.printf("Sensor Offset: %.3f\n", config.sensorOffset);
    Serial.printf("Logging: %s\n", config.enableLogging ? "Enabled" : "Disabled");
    Serial.printf("Sample Rate: %d Hz\n", config.sampleRate);
    Serial.printf("Checksum: 0x%02X\n", config.checksum);
}

void setup()
{
    Serial.begin(115200);
    while (!Serial)
        delay(10);

    Serial.println("EepromShim Configuration Storage Example");
    Serial.println("=======================================");

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

    AppConfig currentConfig;

    // Try to load existing configuration
    if (!loadConfig(currentConfig))
    {
        // Load defaults if configuration is invalid
        Serial.println("Loading default configuration...");
        currentConfig = defaultConfig;
        saveConfig(currentConfig);
    }

    printConfig(currentConfig);

    // Demonstrate configuration modification
    Serial.println("\nModifying configuration...");

    // Change some settings
    strcpy(currentConfig.deviceName, "UpdatedDevice");
    currentConfig.brightness = 200;
    currentConfig.sensorOffset = -0.5;
    currentConfig.enableLogging = false;
    currentConfig.sampleRate = 50;

    // Save modified configuration
    saveConfig(currentConfig);
    printConfig(currentConfig);

    // Verify by reloading
    Serial.println("\nVerifying saved configuration...");
    AppConfig verifyConfig;

    if (loadConfig(verifyConfig))
    {
        Serial.println("✓ Configuration verified successfully!");

        // Compare configurations
        if (memcmp(&currentConfig, &verifyConfig, sizeof(AppConfig)) == 0)
        {
            Serial.println("✓ All data matches perfectly!");
        }
        else
        {
            Serial.println("✗ Data mismatch detected!");
        }
    }
    else
    {
        Serial.println("✗ Configuration verification failed!");
    }

    Serial.println("\nExample complete. Configuration system working correctly.");
}

void loop()
{
    // In a real application, you might periodically save configuration
    // or respond to user input to modify settings
    delay(1000);
}