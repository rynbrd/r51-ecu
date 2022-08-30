#ifndef _R51_BRIDGE_REALDASH_
#define _R51_BRIDGE_REALDASH_

#include <Canny.h>
#include <Caster.h>
#include <Common.h>

// Caster node for communicating with RealDash. This converts Event messages to
// a format that's compatible with RealDash.
class RealDashAdapter : public Caster::Node<R51::Message> {
    public:
        // Construct a new RealDash node that communicates over the provided
        // CAN connection. Events are sent and received using the provided
        // can_id.
        RealDashAdapter(Canny::Connection* connection, uint32_t frame_id) :
            connection_(connection), frame_id_(frame_id), frame_(8) {}

        // Encode and send an Event message to RealDash.
        void handle(const R51::Message& msg) override;

        // Yield received Events from RealDash.
        void emit(const Caster::Yield<R51::Message>& yield) override;

    private:
        Canny::Connection* connection_;
        uint32_t frame_id_;
        Canny::Frame frame_;
        R51::Event event_;
};

#endif  // _R51_BRIDGE_REALDASH_
