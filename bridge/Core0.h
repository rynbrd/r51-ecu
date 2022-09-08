#ifndef _R51_BRIDGE_CORE0_H_
#define _R51_BRIDGE_CORE0_H_

#include "Config.h"

#include <Arduino.h>
#include <Bluetooth.h>
#include <Canny.h>
#include <Canny/RealDash.h>
#include <Caster.h>
#include <Common.h>
#include <Console.h>
#include <Vehicle.h>

#include "CAN.h"
#include "Debug.h"
#include "J1939.h"
#include "Pico.h"
#include "RealDash.h"

namespace R51 {

void onBluetoothConnectProxy(void* arg);
void onBluetoothDisconnectProxy(void* arg);

class Core0 {
    public:
        Core0(Canny::Connection* can, Canny::Connection* j1939 = nullptr, uint8_t j1939_address = 1) :
            can_connection_(can),
            can_node_(&can_connection_),
            j1939_connection_(j1939 == nullptr ? nullptr : new FilteredJ1939(j1939, j1939_address)),
            j1939_node_(j1939 == nullptr ? nullptr : new J1939Adapter(j1939_connection_, j1939_address)),
            j1939_address_(j1939_address),
            #if defined(DEFOG_HEATER_ENABLE)
            defog_node_(DEFOG_HEATER_PIN, DEFOG_HEATER_MS),
            #endif
            #if defined(STEERING_KEYPAD_ENABLE)
            steering_keypad_node_(STEERING_PIN_A, STEERING_PIN_B),
            #endif
            #if defined(RASPBERRYPI_PICO)
            tires_node_(&config_),
            #endif
            #if defined(CONSOLE_ENABLE)
            console_node_(&SERIAL_DEVICE),
            #endif
            #if defined(BLUETOOTH_ENABLE)
            ble_(BLUETOOTH_SPI_CS_PIN, BLUETOOTH_SPI_IRQ_PIN),
            ble_node_(&ble_),
            #endif
            #if defined(REALDASH_ENABLE) && defined(BLUETOOTH_ENABLE)
            realdash_connection_(&ble_),
            realdash_node_(&realdash_connection_, REALDASH_FRAME_ID,
                REALDASH_HB_ID, REALDASH_HB_MS),
            #endif
            bus_(nullptr), node_count_(0) {}

        ~Core0() {
            if (j1939_node_ != nullptr) {
                delete j1939_node_;
            }
            if (j1939_connection_ != nullptr) {
                delete j1939_connection_;
            }
            if (bus_ != nullptr) {
                delete bus_;
            }
        }

        void setup() {
            setup_can();
            setup_j1939();
            setup_console();
            setup_bluetooth();
            setup_realdash();
            setup_vehicle();
            setup_bus();
        }

        void loop() {
            bus_->loop();
            #if defined(BLUETOOTH_ENABLE)
            ble_.update(BLUETOOTH_UPDATE_MS);
            #endif
        }

        void onBluetoothConnect() {
            #if defined(BLUETOOTH_ENABLE)
            ble_node_.onConnect();
            #endif
        }

        void onBluetoothDisconnect() {
            #if defined(BLUETOOTH_ENABLE)
            ble_node_.onDisconnect();
            #endif
        }

    private:
        void setup_can() {
            nodes_[node_count_++] = &can_node_;
        }

        void setup_j1939() {
            if (j1939_node_ != nullptr) {
                nodes_[node_count_++] = j1939_node_;
            }
        }

        void setup_console() {
            #if defined(CONSOLE_ENABLE)
            nodes_[node_count_++] = &console_node_;
            #endif
        }

        void setup_bluetooth() {
            #if defined(BLUETOOTH_ENABLE)
            DEBUG_MSG("setup: initializing bluetooth");
            while (!ble_.begin()) {
                DEBUG_MSG("setup: failed to init bluetooth");
                delay(500);
            }
            ble_.setOnConnect(onBluetoothConnectProxy, (void*)this);
            ble_.setOnDisconnect(onBluetoothDisconnectProxy, (void*)this);
            nodes_[node_count_++] = &ble_node_;
            #endif
        }

        void setup_realdash() {
            #if defined(REALDASH_ENABLE)
            nodes_[node_count_++] = &realdash_node_;
            #endif
        }

        void setup_vehicle() {
            nodes_[node_count_++] = &climate_node_;
            nodes_[node_count_++] = &settings_node_;
            nodes_[node_count_++] = &ipdm_node_;
            nodes_[node_count_++] = &tires_node_;
            #if defined(DEFOG_HEATER_ENABLE)
            nodes_[node_count_++] = &defog_node_;
            #endif
            #if defined(STEERING_KEYPAD_ENABLE)
            nodes_[node_count_++] = &steering_keypad_node_;
            #endif
        }

        void setup_bus() {
            DEBUG_MSG("setup: configuring bus");
            bus_ = new Caster::Bus<Message>(nodes_, node_count_);
        }

        FilteredCAN can_connection_;
        CANNode<Canny::Frame> can_node_;

        FilteredJ1939* j1939_connection_;
        J1939Adapter* j1939_node_;
        uint8_t j1939_address_;

        Climate climate_node_;
        Settings settings_node_;
        IPDM ipdm_node_;
        #if defined(DEFOG_HEATER_ENABLE)
        Defog defog_node_;
        #endif
        #if defined(STEERING_KEYPAD_ENABLE)
        SteeringKeypad steering_keypad_node_;
        #endif
        #if defined(RASPBERRYPI_PICO)
        PicoConfigStore config_;
        #endif
        TirePressureState tires_node_;
        #if defined(CONSOLE_ENABLE)
        ConsoleNode console_node_;
        #endif
        #if defined(BLUETOOTH_ENABLE)
        BLE ble_;
        BLENode ble_node_;
        #endif
        #if defined(REALDASH_ENABLE) && defined(BLUETOOTH_ENABLE)
        Canny::RealDash realdash_connection_;
        RealDashAdapter realdash_node_;
        #endif

        Caster::Bus<Message>* bus_;
        Caster::Node<Message>* nodes_[11];
        uint8_t node_count_;
};

void onBluetoothConnectProxy(void* arg) {
    ((Core0*)arg)->onBluetoothConnect();
}

void onBluetoothDisconnectProxy(void* arg) {
    ((Core0*)arg)->onBluetoothDisconnect();
}

}  // namespace R51

#endif  // _R51_BRIDGE_CORE0_H_
