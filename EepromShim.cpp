/* EepromShim.cpp - EEPROM wrapper with QSPI flash fallback on SAMD/SAM
Copyright (c) 2025 by breadbaker
MIT License */
#include <EepromShim.h>
#if __has_include(<SDCard.h>)
#include <SDCard.h>
#endif

#ifndef EEPROM_h
#define EE_LOG_PREFIX ANSI_GRAY "[EEPROM/QSPI] " ANSI_DEFAULT
#else
#define EE_LOG_PREFIX ANSI_GRAY "[EEPROM] " ANSI_DEFAULT
#endif

namespace EepromShim
{
#ifndef EEPROM_h
    static Adafruit_FlashTransport_QSPI flashTransport;
    static Adafruit_SPIFlash flash(&flashTransport);
#endif

    Configuration init(uint8_t flags)
    {
#ifndef EEPROM_h
        if (!flash.begin())
        {
#ifndef EEPROM_SERIAL_DISABLE
            if (!(flags & EE_SILENT))
                SERIAL.println(F(EE_LOG_PREFIX ANSI_ERROR "Failed to initialize QSPI flash!" ANSI_DEFAULT));
#endif
        }
#endif
        if (flags & EE_DUMP)
            serialDumpSample();
        return getConfig(flags);
    }

    template <typename T>
    T &get(int idx, T &t)
    {
#ifdef EEPROM_h
        return EEPROM.get(idx, t);
#else
        if (idx < 0 || idx + sizeof(T) > EEPROM_SIZE)
            return t;
        uint8_t *ptr = (uint8_t *)&t;
        flash.readBuffer(EEPROM_FLASH_BASE_ADDR + idx, ptr, sizeof(T));
        return t;
#endif
    }

    template <typename T>
    const T &put(int idx, const T &t)
    {
#ifdef EEPROM_h
        return EEPROM.put(idx, t);
#else
        if (idx < 0 || idx + sizeof(T) > EEPROM_SIZE)
            return t;
        const uint8_t *ptr = (const uint8_t *)&t;
        flash.writeBuffer(EEPROM_FLASH_BASE_ADDR + idx, ptr, sizeof(T));
        flash.syncBlocks();
        return t;
#endif
    }

    uint8_t read(int idx)
    {
#ifdef EEPROM_h
        return EEPROM.read(idx);
#else
        if (idx < 0 || idx >= EEPROM_SIZE)
            return 0xFF;
        return flash.read8(EEPROM_FLASH_BASE_ADDR + idx);
#endif
    }

    void write(int idx, uint8_t val)
    {
#ifdef EEPROM_h
        EEPROM.write(idx, val);
#else
        if (idx < 0 || idx >= EEPROM_SIZE)
            return;
        flash.writeBuffer(EEPROM_FLASH_BASE_ADDR + idx, &val, 1);
        flash.syncBlocks();
#endif
    }

    void update(int idx, uint8_t val)
    {
#ifdef EEPROM_h
        EEPROM.update(idx, val);
#else
        if (idx < 0 || idx >= EEPROM_SIZE)
            return;
        if (flash.read8(EEPROM_FLASH_BASE_ADDR + idx) != val)
        {
            flash.writeBuffer(EEPROM_FLASH_BASE_ADDR + idx, &val, 1);
            flash.syncBlocks();
        }
#endif
    }

    Configuration getConfig(uint8_t flags)
    {
        Configuration config = {};
        get(EEPROM_CONFIG_ADDRESS, config);
        if (config.version == EEPROM_CONFIG_VERSION)
        {
            if (!(flags & EE_SILENT))
                status(true);
            config.loaded = true;
            return config;
        }
        else
        {
            Configuration defaults;
            defaults.version = EEPROM_CONFIG_VERSION;
            if (flags & EE_INIT)
            {
                checkFlash(flags | EE_INIT);
                setConfig(defaults);
            }
            if (!(flags & EE_SILENT))
                status(false);
            return defaults;
        }
    }

    bool status(bool ok)
    {
#ifndef EEPROM_SERIAL_DISABLE
        if (ok)
            SERIAL.println(F(EE_LOG_PREFIX ANSI_SUCCESS "Valid EEPROM config found, using existing values." ANSI_DEFAULT));
        else
            SERIAL.println(F(EE_LOG_PREFIX ANSI_ERROR "No valid EEPROM config found, using defaults!" ANSI_DEFAULT));
#endif
        return ok;
    }

    String bitsToString(uint32_t value, uint8_t bits = 32)
    {
        String str = "";
        for (int i = bits - 1; i >= 0; --i)
            str += (value & (1UL << i)) ? '1' : '0';
        return str;
    }

    void list(uint16_t start, uint16_t end, uint8_t flags)
    {
#ifndef EEPROM_SERIAL_DISABLE
        if (start >= EEPROM_SIZE)
        {
            if (!(flags & EE_SILENT))
                SERIAL.println(F(ANSI_ERROR "Address out of range!" ANSI_DEFAULT));
            return;
        }
        char buf[10];
        if (!(flags & EE_SILENT))
            SERIAL.println(F(EE_LOG_PREFIX "EEPROM adresses with data: "));
        for (uint16_t i = start; i <= end; ++i)
        {
            int value = read(i);
            if (value != 0xFF)
            {
                SERIAL.print(F(ANSI_GRAY));
                sprintf(buf, EEPROM_SIZE > 255 ? "0x%04X" : "0x%02X", i);
                SERIAL.print(buf);
                SERIAL.print(F(": " ANSI_YELLOW2 "b"));
                SERIAL.print(bitsToString(value, 8));
                SERIAL.print(F(" " ANSI_DEFAULT));
                sprintf(buf, "0x%02X", value);
                SERIAL.print(buf);
                SERIAL.print(F(ANSI_YELLOW " "));
                SERIAL.print(value, DEC);
                SERIAL.println(F(ANSI_DEFAULT));
            }
        }
#endif
    }

    bool load(const String &path, int16_t start, uint8_t flags)
    {
        if (SDCard::begin(SD_SILENT))
        {
            File file = SDCard::sd.open(path.c_str(), O_READ);
            if (!file)
            {
#ifndef EEPROM_SERIAL_DISABLE
                if (!(flags & SD_SILENT))
                    SERIAL.println(F(ANSI_ERROR "Failed to open file!" ANSI_DEFAULT));
#endif
                return false;
            }
            String line = SDCard::readLineFromFile(file);
            if (line != F("# eeprom"))
            {
#ifndef EEPROM_SERIAL_DISABLE
                if (!(flags & EE_SILENT))
                    SERIAL.println(F(ANSI_ERROR "Not an EEPROM file!" ANSI_DEFAULT));
#endif
                file.close();
                return false;
            }
            int16_t offset = 0;
            int16_t total = 0;
            bool initial = true;
            while (file.available())
            {
                line = SDCard::readLineFromFile(file);
                line.trim();
                if (line.length() > 0 && line.charAt(0) != '#')
                {
                    int sep = line.indexOf(':');
                    if (sep > 0)
                    {
                        String addrStr = line.substring(0, sep);
                        addrStr.trim();
                        String dataStr = line.substring(sep + 1);
                        dataStr.trim();
                        uint16_t address = strtol(addrStr.c_str(), nullptr, 0);
                        if (start >= 0 && offset == 0)
                            offset = start - address;
#ifndef EEPROM_SERIAL_DISABLE
                        if (initial && !(flags & SD_SILENT))
                        {
                            SERIAL.print(F(EE_LOG_PREFIX "Loading EEPROM data to: " ANSI_YELLOW));
                            SERIAL.print(addrStr);
                            if (offset != 0)
                            {
                                SERIAL.print(F(ANSI_SUCCESS " -> "));
                                char buf[8];
                                sprintf(buf, "0x%04X", address + offset);
                                SERIAL.print(buf);
                            }
                            initial = false;
                        }
#endif
                        uint16_t count = 0;
                        while (dataStr.length())
                        {
                            if (address + offset + count >= EEPROM_SIZE)
                            {
#ifndef EEPROM_SERIAL_DISABLE
                                if (!(flags & EE_SILENT))
                                    SERIAL.println(F(ANSI_ERROR "Address out of range!" ANSI_DEFAULT));
#endif
                                file.close();
                                return false;
                            }
                            update(address + offset + count, strtol(dataStr.substring(0, 2).c_str(), nullptr, 16));
                            dataStr = dataStr.substring(2);
                            dataStr.trim();
                            count++;
                            total++;
                        }
                    }
                }
            }
#ifndef EEPROM_SERIAL_DISABLE
            if (total && !(flags & EE_SILENT))
            {
                SERIAL.print(F(ANSI_DEFAULT " size: " ANSI_YELLOW));
                SERIAL.print(total);
                SERIAL.println(F(ANSI_DEFAULT "b"));
            }
#endif
            file.close();
            return true;
        }
#ifndef EEPROM_SERIAL_DISABLE
        if (!(flags & EE_SILENT))
            SERIAL.println(F(ANSI_ERROR "Error loading EEPROM!" ANSI_DEFAULT));
#endif
        return false;
    }

    bool save(const String &path, uint16_t start, uint16_t end, uint8_t flags)
    {
        if (start >= EEPROM_SIZE || end >= EEPROM_SIZE || start > end)
        {
#ifndef EEPROM_SERIAL_DISABLE
            if (!(flags & EE_SILENT))
                SERIAL.println(F(ANSI_ERROR "Address out of range!" ANSI_DEFAULT));
#endif
            return false;
        }
        char buf[8];
        if (SDCard::begin(SD_SILENT))
        {
            File file = SDCard::sd.open(path.c_str(), O_WRITE | O_CREAT | ((flags & EE_FORCE) ? O_TRUNC : O_EXCL));
            if (file)
            {
#ifndef EEPROM_SERIAL_DISABLE
                if (!(flags & SD_SILENT))
                {
                    SERIAL.print(F(EE_LOG_PREFIX "Saving EEPROM data  start: " ANSI_YELLOW));
                    char buf[8];
                    sprintf(buf, EEPROM_SIZE > 255 ? "0x%04X" : "0x%02X", start);
                    SERIAL.print(buf);
                    SERIAL.print(F(ANSI_DEFAULT " to: " ANSI_YELLOW));
                    sprintf(buf, EEPROM_SIZE > 255 ? "0x%04X" : "0x%02X", end);
                    SERIAL.print(buf);
                    SERIAL.print(F(ANSI_DEFAULT " length: " ANSI_YELLOW));
                    SERIAL.print(end - start + 1);
                    SERIAL.println(F(ANSI_DEFAULT "b"));
                }
#endif
                uint16_t counter = 0;
                file.println(F("# eeprom"));
                for (uint16_t i = start; i <= end; ++i)
                {
                    sprintf(buf, EEPROM_SIZE > 255 ? "0x%04X" : "0x%02X", i);
                    if (counter % 16 == 0)
                    {
                        file.print(buf);
                        file.print(F(":"));
                    }
                    counter++;
                    char vbuf[5];
                    sprintf(vbuf, " %02X", read(i));
                    file.print(vbuf);
                    if (counter % 16 == 0)
                        file.println();
                }
                file.close();
                return true;
            }
        }
#ifndef EEPROM_SERIAL_DISABLE
        if (!(flags & EE_SILENT))
            SERIAL.println(F(ANSI_ERROR "Error saving EEPROM!" ANSI_DEFAULT));
#endif
        return false;
    }

    void serialDumpSample(uint16_t maxSamples)
    {
#ifndef EEPROM_SERIAL_DISABLE
        const uint16_t bytesPerSample = (EEPROM_SIZE + maxSamples - 1) / maxSamples; // round up
        SERIAL.print(F(EE_LOG_PREFIX "EEPROM Map (" ANSI_YELLOW));
        SERIAL.print(EEPROM_SIZE);
        SERIAL.print(F(ANSI_DEFAULT "b/" ANSI_YELLOW));
        SERIAL.print(bytesPerSample);
        SERIAL.println(F(ANSI_DEFAULT "b):" ANSI_GRAY));
        uint16_t sampleIdx = 0;
        for (uint16_t i = 0; i < EEPROM_SIZE; i += bytesPerSample, ++sampleIdx)
        {
            bool allFF = true;
            for (uint16_t j = i; j < i + bytesPerSample && j < EEPROM_SIZE; ++j)
                if (read(j) != 0xFF)
                {
                    allFF = false;
                    break;
                }
            SERIAL.print(allFF ? F(".") : F(ANSI_YELLOW2 "#" ANSI_GRAY));
            if ((sampleIdx + 1) % 64 == 0 && (sampleIdx + 1) < maxSamples)
                SERIAL.println();
        }
        SERIAL.println(F(ANSI_DEFAULT));
#endif
    }

    void setConfig(const Configuration &config, uint8_t flags)
    {
        put(EEPROM_CONFIG_ADDRESS, config);
#ifndef EEPROM_SERIAL_DISABLE
        if (!(flags & EE_SILENT))
            SERIAL.println(F(EE_LOG_PREFIX "Config saved to EEPROM."));
#endif
    }

    void wipeConfig(uint8_t flags)
    {
        for (uint16_t i = EEPROM_CONFIG_ADDRESS; i < EEPROM_CONFIG_ADDRESS + sizeof(Configuration); ++i)
            update(i, 0xFF);
#ifndef EEPROM_SERIAL_DISABLE
        if (!(flags & EE_SILENT))
            SERIAL.println(F(EE_LOG_PREFIX "Config wiped from EEPROM." ANSI_DEFAULT));
#endif
    }

    void fill(uint8_t value, uint16_t start, uint16_t end, uint8_t flags)
    {
        if (start >= EEPROM_SIZE || end >= EEPROM_SIZE || start > end)
        {
#ifndef EEPROM_SERIAL_DISABLE
            if (!(flags & EE_SILENT))
                SERIAL.println(F(ANSI_ERROR "Address out of range!" ANSI_DEFAULT));
#endif
            return;
        }
        char buf[8];
        for (uint16_t i = start; i <= end; ++i)
        {
#ifndef EEPROM_SERIAL_DISABLE
            if (!(flags & EE_SILENT))
                SERIAL.print(F(ANSI_GRAY " Filling EEPROM at: " ANSI_YELLOW));
#endif
            update(i, value);
#ifndef EEPROM_SERIAL_DISABLE
            if (!(flags & EE_SILENT))
            {
                sprintf(buf, EEPROM_SIZE > 255 ? "0x%04X" : "0x%02X", i);
                SERIAL.println(buf);
                SERIAL.print(F(ANSI_DEFAULT "\033[A"));
            }
#endif
        }
#ifndef EEPROM_SERIAL_DISABLE
        if (!(flags & EE_SILENT))
            SERIAL.println(F("\n" ANSI_DEFAULT " Fill complete." ANSI_DEFAULT));
#endif
    }

    void printAddress(const uint16_t address)
    {
#ifndef EEPROM_SERIAL_DISABLE
        if (address >= EEPROM_SIZE)
        {
            SERIAL.println(F(ANSI_ERROR "Address out of range!" ANSI_DEFAULT));
            return;
        }
        SERIAL.print(F(ANSI_GRAY "EEPROM ["));
        char buf[7];
        sprintf(buf, EEPROM_SIZE > 255 ? "0x%04X" : "0x%02X", address);
        SERIAL.print(buf);
        SERIAL.print(F("]: " ANSI_YELLOW));
        sprintf(buf, "0x%02X", read(address));
        SERIAL.print(buf);
        SERIAL.println(F(ANSI_DEFAULT));
#endif
    }

    bool checkFlash(uint8_t flags)
    {
#ifndef EEPROM_h
        uint16_t testAddr = EEPROM_SIZE - 1;
        uint8_t initial = read(testAddr);
        write(testAddr, 0xAA);
        if (read(testAddr) != 0xAA)
        {
            if (flags & EE_INIT)
                eraseFlash(flags);
            return false;
        }
        else
        {
            write(testAddr, initial);
            return true;
        }
#else
        return true; // Real EEPROM always works
#endif
    }

    bool eraseFlash(uint8_t flags)
    {
#ifndef EEPROM_h
#ifndef EEPROM_SERIAL_DISABLE
        if (!(flags & EE_SILENT))
            SERIAL.println(F(EE_LOG_PREFIX "Erasing QSPI flash area for EEPROM emulation..."));
#endif
        uint32_t sectorsNeeded = (EEPROM_SIZE + 4095) / 4096;
        for (uint32_t i = 0; i < sectorsNeeded; i++)
        {
            uint32_t sectorAddr = EEPROM_FLASH_BASE_ADDR + (i * 4096);
            if (!flash.eraseSector(sectorAddr))
            {
#ifndef EEPROM_SERIAL_DISABLE
                if (!(flags & EE_SILENT))
                    SERIAL.println(F(EE_LOG_PREFIX ANSI_ERROR "Failed to erase flash sector!" ANSI_DEFAULT));
#endif
                return false;
            }
        }
#ifndef EEPROM_SERIAL_DISABLE
        if (!(flags & EE_SILENT))
            SERIAL.println(F(EE_LOG_PREFIX ANSI_SUCCESS "Flash area erased successfully." ANSI_DEFAULT));
#endif
#endif
        return true;
    }
}