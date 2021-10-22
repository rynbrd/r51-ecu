#ifndef __ECU_MCP2515_H__
#define __ECU_MCP2515_H__

#include "mcp_can.h"
#include "can.h"

namespace ECU {

class Mcp2515 : public CanTranceiver {
    public:
        // Initialize a new MCP2515 tranceiver connected to the provided CS pin.
        Mcp2515(uint8_t cs_pin, CanSpeed can_speed);

        // Destroy the tranceiver.
        ~Mcp2515();

        // Start the tranceiver. Must be called before any other method. Return
        // OK on success or ERROR on failure. 
        Status begin() override;

        // Return true if a message is available to read.
        bool available() const override;

        // Recieve a frame. Return OK on success, ERROR on failure, or NOENT if
        // there is no frame ready to be read.
        Status read(Frame* frame) const override;

        // Send a frame. Return OK on success or ERROR on failure.
        Status write(const Frame& frame) override;
    private:
        CanSpeed speed_;
        MCP_CAN* client_;
};

}

#endif
