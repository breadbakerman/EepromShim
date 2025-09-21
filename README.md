<div align="right">
<img src="https://gist.githubusercontent.com/breadbakerman/70b7ee4430defddaf4f9323ffb73634c/raw/platformio-logo.svg" alt="" />
<img src="https://gist.githubusercontent.com/breadbakerman/8ca75dd6123b4d2882f26837436ef647/raw/arduino-logo.svg" alt="" />
</div>

# EepromShim Library
[![Library Compile Test](https://github.com/breadbakerman/EepromShim/actions/workflows/compile.yml/badge.svg)](https://github.com/breadbakerman/EepromShim/actions/workflows/compile.yml)

EEPROM wrapper with QSPI fallback for Arduino boards. Provides a unified EEPROM interface for AVR (built-in EEPROM) and SAMD51 (QSPI Flash) boards with full Arduino EEPROM API compatibility.

## Features

- **Unified EEPROM Interface**: Same API works across AVR and SAMD51 platforms automatically
- **Smart Backend Selection**: Uses built-in EEPROM on AVR boards, QSPI flash on SAMD51 boards
- **Full API Compatibility**: Drop-in replacement for standard Arduino EEPROM library
- **Write Cycle Optimization**: Minimizes flash wear with intelligent write-before-read logic
- **Flash Validation**: Built-in flash memory testing and integrity checking for SAMD51 boards
- **Template Functions**: Type-safe `get<T>()` and `put<T>()` functions for structured data
- **Configuration Management**: Advanced configuration storage with versioning and validation
- **Error Handling**: Comprehensive error detection and reporting with optional serial output

## Installation

1. Copy the `EepromShim` folder to your Arduino libraries directory
2. Include the header in your sketch: `#include <EepromShim.h>`
3. For SAMD51 boards, ensure the Adafruit SPIFlash library is installed

## Examples

The library includes example sketches to demonstrate usage:

- **BasicEeprom**: Simple EEPROM operations with different data types
- **ConfigurationStorage**: Advanced configuration management with validation and checksums

Access examples in the Arduino IDE via File → Examples → EepromShim.

## Configuration

The library can be configured through compile-time definitions in an optional `config.h` or before including the header:

```cpp
#define EEPROM_SIZE 4096                    // EEPROM size in bytes (default: 4096 for SAMD51)
#define EEPROM_FLASH_BASE_ADDR 0x100000     // Base address in flash (default: 1MB offset)
#define EEPROM_CONFIG_ADDRESS 0x00          // Address for configuration data
#define EEPROM_CONFIG_VERSION 1             // Configuration structure version
#define EEPROM_SERIAL_DISABLE               // Disable serial output (set if SERIAL_OUT_DISABLE is defined)
```

### ANSI Color Configuration

The library supports optional ANSI color codes for serial output. If not defined in your `config.h`, they default to empty strings (no colors). To enable colored output, define these in your `config.h`:

```cpp
#define ANSI_GRAY "\e[38;5;8m"      // Gray color for log prefixes
#define ANSI_DEFAULT "\e[1;37m"     // Default white color
#define ANSI_ERROR "\e[38;5;9m"     // Red color for error messages
#define ANSI_SUCCESS "\e[38;5;10m"  // Green color for success messages
#define ANSI_YELLOW "\e[38;5;11m"   // Yellow color for values
```

The library automatically provides fallback empty definitions if these are not defined, ensuring compilation compatibility regardless of whether color support is configured.

## Template Function Requirements

**Important**: The library uses template functions for type-safe operations with structured data. Due to C++ compilation requirements, template functions need explicit instantiation for custom types.

### Pre-instantiated Types
The library automatically provides template instantiation for:
- `int` - for example usage
- `Configuration` struct (if `config.h` is available)

### Using Custom Structs
To use template functions (`get<T>`, `put<T>`, `init<T>`, etc.) with your own structs:

1. **Option 1**: Use only basic functions
   ```cpp
   // Use read()/write() for custom data
   struct MyData { int value; float ratio; };
   MyData data = {42, 3.14};

   // Write manually
   uint8_t* ptr = (uint8_t*)&data;
   for(size_t i = 0; i < sizeof(MyData); i++) {
       EepromShim::write(addr + i, ptr[i]);
   }
   ```

2. **Option 2**: Add explicit template instantiation
   ```cpp
   // In your .cpp file, after including EepromShim.h
   template MyData& EepromShim::get<MyData>(int, MyData&);
   template const MyData& EepromShim::put<MyData>(int, const MyData&);
   ```

## API Reference

The library provides a complete EEPROM interface with advanced features for configuration management and flash operations.

### Initialization Functions

```cpp
// Initialize EEPROM/Flash subsystem with optional flags
Configuration init(uint8_t flags = EE_NONE);

// Check system status
bool status(bool ok = false);
```

**Flags:**
- `EE_NONE`: No special behavior
- `EE_INIT`: Force initialization
- `EE_DUMP`: Enable memory dumping
- `EE_FORCE`: Force operations
- `EE_SILENT`: Suppress console output

### Core EEPROM Functions

```cpp
// Template functions for structured data (type-safe)
template <typename T>
T &get(int idx, T &t);                    // Read structured data from address

template <typename T>
const T &put(int idx, const T &t);        // Write structured data to address

// Basic byte-level operations
uint8_t read(int idx);                    // Read single byte from address
void write(int idx, uint8_t val);         // Write single byte to address (always writes)
void update(int idx, uint8_t val);        // Write single byte only if different (optimized)
```

**Note**: Template functions (`get<T>`, `put<T>`) require explicit template instantiation for custom types. Built-in types like `int`, `float`, etc. are pre-instantiated. For custom structs, you must either:
- Add explicit template instantiation in your project
- Use only basic `read()`/`write()` functions for custom data structures

### Configuration Management

```cpp
// Template functions for advanced configuration management
template <typename T>
T init(const T &defaults, uint8_t flags = EE_NONE);  // Initialize with configuration struct

template <typename T>
T getConfig(const T &defaults, uint8_t flags = EE_NONE);  // Get configuration from EEPROM

template <typename T>
void setConfig(const T &config, uint8_t flags = EE_NONE);  // Save configuration to EEPROM

template <typename T>
void wipeConfig(uint8_t flags = EE_NONE);  // Clear configuration data
```

**Important**: These template functions work automatically if you have a `config.h` file defining a `Configuration` struct. For other custom structs, you need to add explicit template instantiation in your project.

### Flash-Specific Functions (SAMD51 only)

```cpp
// Test flash memory integrity
bool checkFlash(uint8_t flags = EE_NONE);

// Erase entire flash memory (destructive)
bool eraseFlash(uint8_t flags = EE_NONE);
```

### Utility Functions

```cpp
// Fill memory range with specified value
void fill(uint8_t value = 0xFF, uint16_t start = 0, uint16_t end = EEPROM_SIZE - 1, uint8_t flags = EE_NONE);

// Display memory contents for debugging
void serialDumpSample(uint16_t maxSamples = 256);
void list(uint16_t start = 0, uint16_t end = EEPROM_SIZE - 1, uint8_t flags = EE_NONE);

// File operations (if SD card support available)
bool load(const String &path, int16_t start = -1, uint8_t flags = EE_NONE);
bool save(const String &path, uint16_t start = 0, uint16_t end = EEPROM_SIZE - 1, uint8_t flags = EE_NONE);

// Address printing helper
void printAddress(const uint16_t address);
```

## Platform-Specific Implementation

### AVR Boards (Arduino Uno, Mega, etc.)
- Uses built-in EEPROM hardware
- Full 1024 bytes available on ATmega328P
- No additional setup required
- Direct hardware access for maximum performance

### SAMD51 Boards (Adafruit Grand Central M4, etc.)
- Uses external QSPI flash memory
- 4KB EEPROM emulation by default
- Requires Adafruit SPIFlash library
- Automatic flash initialization and sector management
- Base address offset prevents conflicts with other flash usage

## Usage Examples

### Basic Setup and Usage

```cpp
#include <EepromShim.h>

void setup() {
    Serial.begin(115200);

    // Initialize EEPROM/Flash
    EepromShim::init();

    // For SAMD51 boards, verify flash is working
    #ifdef ARDUINO_SAMD_ADAFRUIT
    if (!EepromShim::checkFlash()) {
        Serial.println("ERROR: Flash memory not working!");
        while (1) delay(100);
    }
    #endif

    // Write some data
    int myValue = 42;
    EepromShim::put(0, myValue);

    // Read it back
    int readValue;
    EepromShim::get(0, readValue);

    Serial.print("Stored value: ");
    Serial.println(readValue);
}

void loop() {
    // Nothing to do in main loop
}
```

### Working with Different Data Types

```cpp
#include <EepromShim.h>

struct SensorData {
    float temperature;
    float humidity;
    uint32_t timestamp;
};

void setup() {
    Serial.begin(115200);
    EepromShim::init();

    // Store structured data
    SensorData data = {23.5, 65.2, 1634567890};
    EepromShim::put(0, data);

    // Store individual values
    EepromShim::put(20, 3.14159f);        // float at address 20
    EepromShim::put(24, (uint16_t)1000);  // uint16_t at address 24

    // Store string data byte by byte
    String message = "Hello World";
    for (int i = 0; i < message.length(); i++) {
        EepromShim::write(30 + i, message[i]);
    }
    EepromShim::write(30 + message.length(), 0); // null terminator

    // Read everything back
    SensorData readData;
    EepromShim::get(0, readData);

    float readFloat;
    EepromShim::get(20, readFloat);

    uint16_t readInt;
    EepromShim::get(24, readInt);

    char readString[32];
    for (int i = 0; i < 31; i++) {
        readString[i] = EepromShim::read(30 + i);
        if (readString[i] == 0) break;
    }

    // Display results
    Serial.printf("Temperature: %.1f°C\n", readData.temperature);
    Serial.printf("Humidity: %.1f%%\n", readData.humidity);
    Serial.printf("Pi: %.5f\n", readFloat);
    Serial.printf("Count: %d\n", readInt);
    Serial.printf("Message: %s\n", readString);
}

void loop() {
    delay(1000);
}
```

### Configuration Storage with Validation

```cpp
#include <EepromShim.h>

struct AppConfig {
    uint32_t magic;           // Magic number for validation
    uint16_t version;         // Configuration version
    char deviceName[32];      // Device name
    uint8_t brightness;       // LED brightness (0-255)
    float sensorOffset;       // Sensor calibration offset
    bool enableLogging;       // Enable data logging
    uint16_t sampleRate;      // Sample rate in Hz
    uint8_t checksum;         // Simple checksum
};

const uint32_t CONFIG_MAGIC = 0xC0FF1234;
const uint16_t CONFIG_VERSION = 1;
const int CONFIG_ADDR = 0;

// Calculate checksum for configuration
uint8_t calculateChecksum(const AppConfig& config) {
    uint8_t checksum = 0;
    const uint8_t* data = (const uint8_t*)&config;

    // Checksum all bytes except the checksum field itself
    for (size_t i = 0; i < sizeof(AppConfig) - 1; i++) {
        checksum ^= data[i];
    }
    return checksum;
}

// Load configuration with validation
bool loadConfig(AppConfig& config) {
    EepromShim::get(CONFIG_ADDR, config);

    // Validate magic number and version
    if (config.magic != CONFIG_MAGIC || config.version != CONFIG_VERSION) {
        return false;
    }

    // Validate checksum
    uint8_t expectedChecksum = calculateChecksum(config);
    if (config.checksum != expectedChecksum) {
        return false;
    }

    return true;
}

// Save configuration with checksum
void saveConfig(AppConfig& config) {
    config.checksum = calculateChecksum(config);
    EepromShim::put(CONFIG_ADDR, config);
}

void setup() {
    Serial.begin(115200);
    EepromShim::init();

    AppConfig config;

    // Try to load existing configuration
    if (!loadConfig(config)) {
        // Load defaults if invalid
        config = {
            .magic = CONFIG_MAGIC,
            .version = CONFIG_VERSION,
            .deviceName = "MyDevice",
            .brightness = 128,
            .sensorOffset = 0.0,
            .enableLogging = true,
            .sampleRate = 100,
            .checksum = 0
        };
        saveConfig(config);
        Serial.println("Loaded default configuration");
    } else {
        Serial.println("Loaded saved configuration");
    }

    // Display current configuration
    Serial.printf("Device: %s\n", config.deviceName);
    Serial.printf("Brightness: %d\n", config.brightness);
    Serial.printf("Sample Rate: %d Hz\n", config.sampleRate);
}

void loop() {
    delay(1000);
}
```

### Write Cycle Optimization

```cpp
#include <EepromShim.h>

void setup() {
    Serial.begin(115200);
    EepromShim::init();

    uint8_t address = 10;
    uint8_t newValue = 42;

    // Always writes (wear on flash/EEPROM)
    EepromShim::write(address, newValue);

    // Only writes if value is different (optimized for flash longevity)
    EepromShim::update(address, newValue);  // Won't write since value is already 42
    EepromShim::update(address, 50);        // Will write since value is different

    // For structured data, put() behaves like update() - only writes if different
    struct Data { int value; float ratio; };
    Data myData = {100, 3.14};

    EepromShim::put(20, myData);  // Writes data
    EepromShim::put(20, myData);  // Won't write - data unchanged

    myData.value = 200;
    EepromShim::put(20, myData);  // Writes only the changed portion
}

void loop() {
    delay(1000);
}
```

### Memory Management and Debugging

```cpp
#include <EepromShim.h>

void setup() {
    Serial.begin(115200);
    EepromShim::init();

    // Fill memory with pattern for testing
    EepromShim::fill(0xAA, 0, 100);      // Fill first 100 bytes with 0xAA
    EepromShim::fill(0x55, 100, 200);    // Fill next 100 bytes with 0x55

    // Display memory contents
    Serial.println("Memory dump (first 50 bytes):");
    EepromShim::list(0, 49);

    // Or use sample dump for overview
    Serial.println("Memory sample:");
    EepromShim::serialDumpSample(32);

    // Clear specific section
    EepromShim::fill(0x00, 50, 150);     // Clear bytes 50-150

    // For SAMD51, can check flash health
    #ifdef ARDUINO_SAMD_ADAFRUIT
    if (EepromShim::checkFlash()) {
        Serial.println("Flash memory is healthy");
    } else {
        Serial.println("Flash memory issues detected");
    }
    #endif
}

void loop() {
    delay(1000);
}
```

## Memory Layout and Addressing

### AVR Boards
- Direct EEPROM hardware access
- Address range: 0 to `EEPROM.length() - 1`
- Typical sizes: 1024 bytes (ATmega328P), 4096 bytes (ATmega2560)

### SAMD51 Boards
- QSPI flash emulation with base address offset
- Default base address: `0x100000` (1MB offset)
- Default EEPROM size: 4096 bytes
- Address range: 0 to `EEPROM_SIZE - 1` (maps to flash addresses `BASE + 0` to `BASE + EEPROM_SIZE - 1`)

## Flash Memory Considerations (SAMD51)

### Write Cycle Optimization
- Flash memory has limited write/erase cycles (~10,000-100,000)
- Use `update()` instead of `write()` to minimize unnecessary writes
- Template `put()` function automatically optimizes writes
- 4KB sector erase requirement handled automatically

### Sector Management
- Flash memory requires sector-level erasing (4KB minimum)
- Library manages sector allocation transparently
- Base address offset prevents conflicts with bootloader and application code
- Automatic sector erase when crossing boundaries

### Error Handling
- Built-in flash initialization checking
- Memory integrity validation
- Graceful fallback when flash operations fail
- Optional error reporting via serial output

## Dependencies

- Arduino core libraries
- **For SAMD51 boards**: Adafruit SPIFlash library
- `config.h` (optional, auto-detected for project-specific settings)

## Notes

- Library automatically detects board type and selects appropriate backend
- No code changes required when switching between AVR and SAMD51 platforms
- All EEPROM operations are blocking (complete before function returns)
- Template functions provide type safety and automatic size calculation
- Serial output can be disabled with `EEPROM_SERIAL_DISABLE` define
- Flash operations on SAMD51 include wear leveling and optimization
- Use `update()` over `write()` for better flash memory longevity
- Configuration management includes built-in versioning and validation

## License

MIT License - Copyright (c) 2025 by breadbaker