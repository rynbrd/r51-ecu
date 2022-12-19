#ifndef _R51_PICO_CONFIG_H_
#define _R51_PICO_CONFIG_H_

#include <Arduino.h>
#include <Vehicle.h>

extern "C" {
    #include <hardware/flash.h>
};

namespace R51 {

class PicoConfigStore : public ConfigStore {
    public:
        PicoConfigStore();

        // Load the tire mapping from flash.
        ConfigStore::Error loadTireMap(uint8_t* map) override;

        // Save the tire mapping to flash.
        ConfigStore::Error saveTireMap(uint8_t* map) override;

    private:
        void load();
        void save();

        bool valid_;
        const uint8_t* flash_;
        uint8_t data_[FLASH_PAGE_SIZE];
};

}  // namespace R51

#endif  // _R51_BRIDGE_PICO_H_
