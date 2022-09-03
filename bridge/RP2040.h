#ifndef _R51_BRIDGE_RP2040_H_
#define _R51_BRIDGE_RP2040_H_
#ifdef RASPBERRYPI_PICO

#include <Arduino.h>
#include <CRC32.h>
#include <Vehicle.h>

extern "C" {
    #include <hardware/sync.h>
    #include <hardware/flash.h>
};

#define FLASH_TARGET_OFFSET (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE)
#define FLASH_MEMORY_ADDRESS (XIP_BASE + FLASH_TARGET_OFFSET)

class PicoConfigStore : public R51::ConfigStore {
    public:
        PicoConfigStore(bool multicore_lockout) :
                lockout_(multicore_lockout), valid_(false),
                flash_((const uint8_t*)(FLASH_MEMORY_ADDRESS)) {
            init();
        }

        Error loadTireMap(uint8_t* map) override {
            if (!valid_) {
                return R51::ConfigStore::UNSET;
            }
            memcpy(map, data_, 4);
            return R51::ConfigStore::SET;
        }

        Error saveTireMap(uint8_t* map) override {
            memcpy(data_, map, 4);
            save();
            return R51::ConfigStore::SET;
        }

    private:
        void init() {
            memset(data_, 0, FLASH_PAGE_SIZE);

            uint32_t checksum = CRC32::calculate(flash_, kDataLen);
            if (memcmp(&checksum, flash_ + kDataLen, sizeof(checksum)) == 0) {
                memcpy(data_, flash_, kDataLen);
                valid_ = true;
                return;
            }

            if (lockout_) {
                multicore_lockout_start_blocking();
            }
            uint32_t ints = save_and_disable_interrupts();
            flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
            restore_interrupts(ints);
            if (lockout_) {
                multicore_lockout_end_blocking();
            }
        }

        void save() {
            uint32_t checksum = CRC32::calculate(data_, kDataLen);
            memcpy(&checksum, data_ + kDataLen, sizeof(checksum));
            if (memcmp(data_, flash_, kDataLen + sizeof(checksum)) == 0) {
                return;
            }

            if (lockout_) {
                multicore_lockout_start_blocking();
            }
            uint32_t ints = save_and_disable_interrupts();
            flash_range_program(FLASH_TARGET_OFFSET, data_, FLASH_PAGE_SIZE);
            restore_interrupts(ints);
            if (lockout_) {
                multicore_lockout_end_blocking();
            }
        }

        bool lockout_;
        bool valid_;
        const uint8_t* flash_;
        uint8_t data_[FLASH_PAGE_SIZE];
        static const size_t kDataLen = 4;   // The amount of data we're actually storing in flash.
};

#endif  // RASPBERRYPI_PICO
#endif  // _R51_BRIDGE_RP2040_H_
