#ifndef _R51_VEHICLE_CONFIG_H
#define _R51_VEHICLE_CONFIG_H

namespace R51 {

// Interface for configuration storage. Needed to store persistent config
// values across reboots. 
class ConfigStore {
    public:
        enum Error {
            SET,
            UNSET,
        };

        ConfigStore() {}
        virtual ~ConfigStore() = default;

        // Load the tire mapping from storage. Map must be a pointer to an
        // array of length 4. Return SET if map was loaded from storage. Return
        // UNSET otherwise. The data in map is unchanged in this case.
        virtual Error loadTireMap(uint8_t* map) = 0;

        // Save the tire mapping to storage. Map must be a pointer to an array
        // of length 4. Return SET if map is saved to storage.
        virtual Error saveTireMap(uint8_t* map) = 0;
};

}

#endif  // _R51_VEHICLE_CONFIG_H
