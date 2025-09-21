/* EepromShim.h - EEPROM wrapper with QSPI flash fallback on SAMD/SAM
Copyright (c) 2025 by breadbaker
MIT License */
#pragma once
#include <Arduino.h>
#if __has_include(<config.h>)
#include <config.h> // Include a global config if available
#endif
#if !defined(ARDUINO_ARCH_SAMD) && !defined(ARDUINO_ARCH_SAM)
#include <EEPROM.h>
#undef EEPROM_SIZE
#define EEPROM_SIZE EEPROM.length() // EEPROM size for AVR boards
#undef EEPROM_SD_FILE
#define EEPROM_SD_FILE F("") // Not used for AVR boards
#else
#include <Adafruit_SPIFlash.h>
#ifndef EEPROM_SIZE
#define EEPROM_SIZE 4096 // Default EEPROM size for SAMD/SAM boards
#endif
#ifndef EEPROM_FLASH_BASE_ADDR
#define EEPROM_FLASH_BASE_ADDR 0x100000 // Base address in flash for EEPROM emulation (1MB offset)
#endif
#endif

#ifdef SERIAL_OUT_DISABLE
#define EEPROM_SERIAL_DISABLE // Disable Serial output
#endif

#if !defined(EEPROM_SERIAL_DISABLE)
#undef SERIAL // Serial port mapping
#ifdef SERIAL_PORT_USBVIRTUAL
#define SERIAL SERIAL_PORT_USBVIRTUAL
#else
#define SERIAL Serial // Use default Serial port
#endif
#endif

#ifndef EEPROM_CONFIG_ADDRESS
#define EEPROM_CONFIG_ADDRESS 0x00 // Address for configuration data in EEPROM
#endif

#ifndef EEPROM_CONFIG_VERSION
#define EEPROM_CONFIG_VERSION 1 // Version of the EEPROM configuration structure
#endif

// Flags:
#define EE_NONE 0x00
#define EE_INIT 0x01
#define EE_DUMP 0x02
#define EE_FORCE 0x08
#define EE_SILENT 0x80

// Define ANSI color codes if not already defined:
#ifndef ANSI_GRAY
#define ANSI_GRAY ""
#endif
#ifndef ANSI_DEFAULT
#define ANSI_DEFAULT ""
#endif
#ifndef ANSI_YELLOW
#define ANSI_YELLOW ""
#endif
#ifndef ANSI_YELLOW2
#define ANSI_YELLOW2 ""
#endif
#ifndef ANSI_ERROR
#define ANSI_ERROR ""
#endif
#ifndef ANSI_SUCCESS
#define ANSI_SUCCESS ""
#endif

namespace EepromShim
{
    Configuration init(uint8_t flags = EE_NONE);
    bool status(bool ok = false);
    void serialDumpSample(uint16_t maxSamples = 256);
    void fill(uint8_t value = 0xFF, uint16_t start = 0, uint16_t end = EEPROM_SIZE - 1, uint8_t flags = EE_NONE);
    Configuration getConfig(uint8_t flags = EE_NONE);
    void printAddress(const uint16_t address);
    void list(uint16_t start = 0, uint16_t end = EEPROM_SIZE - 1, uint8_t flags = EE_NONE);
    bool load(const String &path, int16_t start = -1, uint8_t flags = EE_NONE);
    bool save(const String &path, uint16_t start = 0, uint16_t end = EEPROM_SIZE - 1, uint8_t flags = EE_NONE);
    void setConfig(const Configuration &config, uint8_t flags = EE_NONE);
    void wipeConfig(uint8_t flags = EE_NONE);
    bool checkFlash(uint8_t flags = EE_NONE);
    bool eraseFlash(uint8_t flags = EE_NONE);
    template <typename T>
    T &get(int idx, T &t);
    template <typename T>
    const T &put(int idx, const T &t);
    uint8_t read(int idx);
    void update(int idx, uint8_t val);
    void write(int idx, uint8_t val);
}