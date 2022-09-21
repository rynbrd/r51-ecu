#ifndef _R51_COMMON_EVENT_H_
#define _R51_COMMON_EVENT_H_

// Convenience macro for defining getters and setters on child events.
#define EVENT_PROPERTY(type, name, getter, setter) \
    type name() const { return (getter); } \
    bool name(type value) { \
        if ((getter) == value) { \
            return false; \
        } \
        (setter); \
        return true; \
    }

#include <Arduino.h>
#include <Canny.h>

namespace R51 {

// Sub-system identifier. This allows a physical to host multiple subsystems.
enum class SubSystem : uint8_t {
    ECM = 0x04,
    IPDM = 0x06,
    TIRE = 0x08,
    CLIMATE = 0x0A,
    SETTINGS = 0x0B,
    STEERING_KEYPAD = 0x0C,
    BLUETOOTH = 0x0D,
    HMI = 0x0E,
    AUDIO = 0x0F,
};

struct Event : public Printable {
    // Subsystem identifier. This is the subsystem the event belongs to.
    uint8_t subsystem;
    // ID of the event. This uniquely identifies an event within a subsystem.
    uint8_t id;
    // Event data. Empty bytes are padded with 0xFF.
    uint8_t data[6];

    // Construct an empty system event. SubSystem and ID are set to 0x00.
    Event();

    // Construct a specific event with empty data.
    Event(uint8_t subsystem, uint8_t id);
    Event(SubSystem subsystem, uint8_t id);

    // Construct a specific event containing the provided data. The
    // size is set to the size of the provided data array. This array
    // should not exceed 5 bytes.
    Event(uint8_t subsystem, uint8_t id, const uint8_t* data, size_t size);
    template <size_t N> 
    Event(uint8_t subsystem, uint8_t id, const uint8_t (&data)[N]);
    template <size_t N> 
    Event(SubSystem subsystem, uint8_t id, const uint8_t (&data)[N]);

    // Print the system event.
    size_t printTo(Print& p) const override;
};

// Return true if the two system events are equal
bool operator==(const Event& left, const Event& right);

// Return true if the two system events are not equal
bool operator!=(const Event& left, const Event& right);

template <size_t N>
Event::Event(uint8_t subsystem, uint8_t id, const uint8_t (&data)[N]) :
        Event(subsystem, id, data, N) {}

template <size_t N>
Event::Event(SubSystem subsystem, uint8_t id, const uint8_t (&data)[N]) :
        Event((uint8_t)subsystem, id, data, N) {}

}  // namespace R51

#endif  // _R51_COMMON_EVENT_H_
