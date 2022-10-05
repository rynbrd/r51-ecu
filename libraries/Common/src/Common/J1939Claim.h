#ifndef _R51_COMMON_J1939_CLAIM_H_
#define _R51_COMMON_J1939_CLAIM_H_

#include <Arduino.h>
#include <Canny.h>

namespace R51 {

// An address claim. Emitted to the bus when this device claims an address.
class J1939Claim : public Printable {
    public:
        J1939Claim() : address_(Canny::NullAddress), name_(0) {}
        J1939Claim(uint8_t address, uint64_t name) : address_(address), name_(name) {}

        uint8_t address() const { return address_; }
        void address(uint8_t address) { address_ = address; }

        uint64_t name() const { return name_; }
        void name(uint64_t name) { name_ = name; }

        size_t printTo(Print& p) const override;
    private:
        uint8_t address_;
        uint64_t name_;
};

bool operator==(const J1939Claim& left, const J1939Claim& right);
bool operator!=(const J1939Claim& left, const J1939Claim& right);

}  // namespace R51

#endif  // _R51_COMMON_J1939_CLAIM_H_
