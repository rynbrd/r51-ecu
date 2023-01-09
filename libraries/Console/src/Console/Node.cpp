#include "Node.h"

#include <Canny.h>
#include <Caster.h>
#include <Core.h>
#include <Vehicle.h>
#include "Console.h"

#define isEvent(e, s, i) (e->subsystem == (uint8_t)s && e->id == (uint8_t)i)

namespace R51 {
namespace {

void prettyPrintEvent(Stream* stream, const Event* event) {
    if (isEvent(event, SubSystem::CLIMATE, ClimateEvent::SYSTEM_STATE)) {
        const auto* s = (ClimateSystemState*)event;
        stream->print(" (climate ");
        switch (s->mode()) {
            case CLIMATE_SYSTEM_OFF:
                stream->print("off");
                break;
            case CLIMATE_SYSTEM_AUTO:
                stream->print("auto");
                break;
            case CLIMATE_SYSTEM_MANUAL:
                stream->print("manual");
                break;
            case CLIMATE_SYSTEM_DEFOG:
                stream->print("defog");
                break;
            default:
                break;
        }
        stream->print(", ac ");
        stream->print(s->ac() ? "on" : "off");
        stream->print(", dual ");
        stream->print(s->dual() ? "on" : "off");
        stream->print(")");
    } else if (isEvent(event, SubSystem::CLIMATE, ClimateEvent::AIRFLOW_STATE)) {
        const auto* s = (ClimateAirflowState*)event;
        stream->print(" (climate fan ");
        stream->print(s->fan_speed());
        if (s->face()) {
            stream->print(" face");
        }
        if (s->feet()) {
            stream->print(" feet");
        }
        if (s->windshield()) { 
            stream->print(" windshield");
        }
        if (s->recirculate()) {
            stream->print(" recirculate");
        }
        stream->print(")");
    } else if (isEvent(event, SubSystem::CLIMATE, ClimateEvent::TEMP_STATE)) {
        const auto* s = (ClimateTempState*)event;
        char unit = 'F';
        if (s->units() == UNITS_METRIC) {
            unit = 'C';
        }
        stream->print(" (climate driver temp ");
        stream->print(s->driver_temp());
        stream->print(unit);
        stream->print(", passenger temp ");
        stream->print(s->passenger_temp());
        stream->print(unit);
        stream->print(", outside temp ");
        stream->print(s->outside_temp());
        stream->print(unit);
        stream->print(")");
    }
}

}  // namespace

void ConsoleNode::emit(const Caster::Yield<Message>& yield) {
    Reader::Error err;
    while (console_.stream()->available())  {
        if (command_->line()) {
            err = reader_.line();
        } else {
            err = reader_.word();
        }
        switch (err) {
            case Reader::EOW:
                command_ = command_->next(buffer_);
                break;
            case Reader::EOL:
                command_ = command_->next(buffer_);
                command_->run(&console_, buffer_, yield);
                command_ = &root_;
                break;
            case Reader::OVERRUN:
                command_ = &root_;
                break;
            default:
                break;
        }
    }
}

void ConsoleNode::handle(const Message& msg, const Caster::Yield<R51::Message>&) {
    switch (msg.type()) {
        case Message::EVENT:
            if (!console_.event_mute()) {
                console_.stream()->print("console: event recv ");
                msg.event()->printTo(*console_.stream());
                prettyPrintEvent(console_.stream(), msg.event());
                console_.stream()->println();
            }
            break;
        case Message::CAN_FRAME:
            if (console_.can_filter()->match(*msg.can_frame())) {
                console_.stream()->print("console: can recv ");
                msg.can_frame()->printTo(*console_.stream());
                console_.stream()->println();
            }
            break;
        case Message::J1939_CLAIM:
            if (!console_.j1939_mute()) {
                console_.stream()->print("console: j1939 claim ");
                if (msg.j1939_claim()->address() <= 0x0F) {
                    console_.stream()->print("0x0");
                } else {
                    console_.stream()->print("0x");
                }
                console_.stream()->println(msg.j1939_claim()->address(), HEX);
            }
            break;
        case Message::J1939_MESSAGE:
            if (!console_.j1939_mute()) {
                console_.stream()->print("console: j1939 recv ");
                msg.j1939_message()->printTo(*console_.stream());
                console_.stream()->println();
            }
            break;
        case Message::EMPTY:
            break;
    }
}

}  // namespace R51
