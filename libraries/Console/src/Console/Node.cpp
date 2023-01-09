#include "Node.h"

#include <Canny.h>
#include <Caster.h>
#include <Core.h>
#include <Foundation.h>
#include <Vehicle.h>
#include "Console.h"

#define isEvent(e, s, i) (e->subsystem == (uint8_t)s && e->id == (uint8_t)i)

namespace R51 {
namespace {

void prettyPrintEvent(Stream* stream, const Event* event) {
    if (isEvent(event, SubSystem::KEYPAD, KeypadEvent::KEY_STATE)) {
        const auto* s = (KeyState*)event;
        stream->print(" (keypad ");
        stream->print(s->keypad());
        if (s->pressed()) {
            stream->print(" press ");
        } else {
            stream->print(" release ");
        }
        stream->print(s->key());
        stream->print(")");
    } else if (isEvent(event, SubSystem::KEYPAD, KeypadEvent::ENCODER_STATE)) {
        const auto* s = (EncoderState*)event;
        stream->print(" (keypad ");
        stream->print(s->keypad());
        stream->print(" encoder ");
        stream->print(s->encoder());
        stream->print(" delta ");
        stream->print(s->delta());
        stream->print(")");
    } else if (isEvent(event, SubSystem::CLIMATE, ClimateEvent::SYSTEM_STATE)) {
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
    } else if (isEvent(event, SubSystem::BCM, BCMEvent::ILLUM_STATE)) {
        const auto* s = (IllumState*)event;
        stream->print(" (dash illumination ");
        stream->print(s->illum()  ? "on" : "off");
        stream->print(")");
    } else if (isEvent(event, SubSystem::BCM, BCMEvent::TIRE_PRESSURE_STATE)) {
        stream->print(" (tire pressure ");
        stream->print(event->data[0]);
        stream->print(" ");
        stream->print(event->data[1]);
        stream->print(" ");
        stream->print(event->data[2]);
        stream->print(" ");
        stream->print(event->data[3]);
        stream->print(")");
    } else if (isEvent(event, SubSystem::IPDM, IPDMEvent::POWER_STATE)) {
        stream->print(" (ipdm high beams ");
        stream->print(getBit(event->data, 0, 0) ? "on" : "off");
        stream->print(", low beams ");
        stream->print(getBit(event->data, 0, 1) ? "on" : "off");
        stream->print(", running lights ");
        stream->print(getBit(event->data, 0, 2) ? "on" : "off");
        stream->print(", fog lights ");
        stream->print(getBit(event->data, 0, 3) ? "on" : "off");
        stream->print(", defrost ");
        stream->print(getBit(event->data, 0, 6) ? "on" : "off");
        stream->print(", a/c comp ");
        stream->print(getBit(event->data, 0, 7) ? "on" : "off");
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
