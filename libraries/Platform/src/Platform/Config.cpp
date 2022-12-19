#include "Config.h"

#include <Arduino.h>
#include <CRC32.h>

extern "C" {
    #include <hardware/flash.h>
};

#define PICO_FLASH_TARGET_OFFSET (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE)
#define PICO_FLASH_MEMORY_ADDRESS (XIP_BASE + PICO_FLASH_TARGET_OFFSET)

namespace R51 {

// The amount of data we're actually storing in flash.
static const size_t kPicoConfigDataLen = 4;

PicoConfigStore::PicoConfigStore() :
        valid_(false), flash_((const uint8_t*)(PICO_FLASH_MEMORY_ADDRESS)) {
    load();
}

ConfigStore::Error PicoConfigStore::loadTireMap(uint8_t* map) {
    if (!valid_) {
        return ConfigStore::UNSET;
    }
    memcpy(map, data_, 4);
    return ConfigStore::SET;
}

ConfigStore::Error PicoConfigStore::saveTireMap(uint8_t* map) {
    memcpy(data_, map, 4);
    save();
    return ConfigStore::SET;
}

void PicoConfigStore::load() {
    uint32_t checksum = CRC32::calculate(flash_, kPicoConfigDataLen);
    if (checksum != 0xFFFFFFFF && memcmp(&checksum, flash_ + kPicoConfigDataLen, sizeof(checksum)) == 0) {
        memcpy(data_, flash_, kPicoConfigDataLen);
        valid_ = true;
        return;
    }

    memset(data_, 0, FLASH_PAGE_SIZE);
    rp2040.idleOtherCore();
    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(PICO_FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
    restore_interrupts(ints);
    rp2040.resumeOtherCore();
}

void PicoConfigStore::save() {
    uint32_t checksum = CRC32::calculate(data_, kPicoConfigDataLen);
    memcpy(data_ + kPicoConfigDataLen, &checksum, sizeof(checksum));
    if (memcmp(data_, flash_, kPicoConfigDataLen + sizeof(checksum)) == 0) {
        return;
    }

    rp2040.idleOtherCore();
    uint32_t ints = save_and_disable_interrupts();
    flash_range_program(PICO_FLASH_TARGET_OFFSET, data_, FLASH_PAGE_SIZE);
    restore_interrupts(ints);
    rp2040.resumeOtherCore();
}

}  // namespace R51
