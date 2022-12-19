#ifndef _R51_BRIDGE_PICO_H_
#define _R51_BRIDGE_PICO_H_

#include <Arduino.h>
#include <CRC32.h>
#include <Core.h>
#include <Vehicle.h>

extern "C" {
    #include <hardware/flash.h>
    #include <hardware/sync.h>
};

#define FLASH_TARGET_OFFSET (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE)
#define FLASH_MEMORY_ADDRESS (XIP_BASE + FLASH_TARGET_OFFSET)

namespace R51 {

class PicoFilteredPipe : public PicoPipe {
    public:
        PicoFilteredPipe() : PicoPipe(IO_CORE_BUFFER_SIZE, PROC_CORE_BUFFER_SIZE) {}

        // Filtering for the I/O core. Forwards all frames  to the processing core.
        bool filterLeft(const Message&) override { return true; }

        // Filtering for the processing core. Only forwards CAN bus frames and
        // system events to the I/O core.
        bool filterRight(const Message& msg) override {
            return msg.type() == Message::CAN_FRAME ||
                msg.type() == Message::J1939_MESSAGE ||
                msg.type() == Message::EVENT;
        }

        void onBufferOverrun(const Message& msg) override {
            DEBUG_MSG_OBJ("pipe: dropped frame: ", msg);
        }
};

class PicoConfigStore : public ConfigStore {
    public:
        PicoConfigStore() : valid_(false), flash_((const uint8_t*)(FLASH_MEMORY_ADDRESS)) {
            init();
        }

        Error loadTireMap(uint8_t* map) override {
            if (!valid_) {
                return ConfigStore::UNSET;
            }
            memcpy(map, data_, 4);
            return ConfigStore::SET;
        }

        Error saveTireMap(uint8_t* map) override {
            memcpy(data_, map, 4);
            save();
            return ConfigStore::SET;
        }

    private:
        void init() {
            memset(data_, 0, FLASH_PAGE_SIZE);

            uint32_t checksum = CRC32::calculate(flash_, kDataLen);
            if (checksum != 0xFFFFFFFF && memcmp(&checksum, flash_ + kDataLen, sizeof(checksum)) == 0) {
                memcpy(data_, flash_, kDataLen);
                valid_ = true;
                return;
            }

            rp2040.idleOtherCore();
            uint32_t ints = save_and_disable_interrupts();
            flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
            restore_interrupts(ints);
            rp2040.resumeOtherCore();
        }

        void save() {
            uint32_t checksum = CRC32::calculate(data_, kDataLen);
            memcpy(data_ + kDataLen, &checksum, sizeof(checksum));
            if (memcmp(data_, flash_, kDataLen + sizeof(checksum)) == 0) {
                return;
            }

            rp2040.idleOtherCore();
            uint32_t ints = save_and_disable_interrupts();
            flash_range_program(FLASH_TARGET_OFFSET, data_, FLASH_PAGE_SIZE);
            restore_interrupts(ints);
            rp2040.resumeOtherCore();
        }

        bool valid_;
        const uint8_t* flash_;
        uint8_t data_[FLASH_PAGE_SIZE];
        static const size_t kDataLen = 4;   // The amount of data we're actually storing in flash.
};

}  // namespace R51

#endif  // _R51_BRIDGE_PICO_H_
