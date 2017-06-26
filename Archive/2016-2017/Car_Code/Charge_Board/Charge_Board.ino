#include <FlexCAN.h> // import teensy library
#include <Metro.h>
#include "HyTech17.h"

// define pins
#define RELAY_SWITCH_PIN 5
#define EXT_BTN1_PIN 1
#define EXT_OUTPUT_1 2
#define EXT_SSEG_1 6
#define EXT_SSEG_2 11
#define EXT_SSEG_3 12
#define EXT_SSEG_4 13

FlexCAN CAN(500000);
static CAN_message_t msg;

uint8_t state;
bool btn_charge_pressed = false;
bool btn_charge_debouncing = false;
uint8_t btn_charge_new;
int lastDebounceCHARGE = 0;

Metro timer_btn_charge = Metro(10);

void setup() {
    Serial.begin(115200); // init serial for PC communication

    CAN.begin(); // init CAN system
    Serial.println("CAN system and serial communication initialized");
    pinMode(RELAY_SWITCH_PIN, OUTPUT);
    pinMode(EXT_OUTPUT_1, OUTPUT);
    pinMode(EXT_SSEG_1, OUTPUT);
    pinMode(EXT_SSEG_2, OUTPUT);
    pinMode(EXT_SSEG_3, OUTPUT);
    pinMode(EXT_SSEG_4, OUTPUT);
    pinMode(EXT_BTN1_PIN, INPUT_PULLUP);
    set_state(CHARGE_STATE_NOT_CHARGING);
}

void loop () {
    while (CAN.read(msg)) {
        if (msg.id == ID_BMS_CURRENT) {
            BMS_currents bms_currents(msg.buf);
            if (state == CHARGE_STATE_CHARGE_REQUESTED
                && bms_currents.getChargingState() == 1) {
                set_state(CHARGE_STATE_CHARGING);
            }
            if (bms_currents.getChargingState() == 1){
                Serial.println("BMS entered charge mode.");
            }
            else if (bms_currents.getChargingState() == 0) {
                Serial.println("BMS in discharging state.");
            }
            else if (bms_currents.getChargingState() == 2){
                Serial.println("BMS in unknown state.");
            }
            else {
                Serial.println(bms_currents.getChargingState());
            }
        }
    }
    switch(state) {
        case CHARGE_STATE_NOT_CHARGING:
            if (digitalRead(EXT_BTN1_PIN) == LOW) {
                set_state(CHARGE_STATE_CHARGE_REQUESTED);
            }
            break;
        case CHARGE_STATE_CHARGING:
            if (digitalRead(EXT_BTN1_PIN) == HIGH) {
                set_state(CHARGE_STATE_NOT_CHARGING);
            }
            break;
    }
}

// void pollForButtonPress() {
//     /*
//     * Handle start button press and depress
//     */
//     if (digitalRead(EXT_BTN1_PIN) == btn_charge_pressed && !btn_charge_debouncing) { // Value is different than stored
//         btn_charge_debouncing = true;
//         timer_btn_charge.reset();
//     }
//     if (btn_charge_debouncing && digitalRead(EXT_BTN1_PIN) != btn_charge_pressed) { // Value returns during debounce period
//         btn_charge_debouncing = false;
//     }
//     if (btn_charge_debouncing && timer_btn_charge.check()) { // Debounce period finishes without value returning
//         btn_charge_pressed = !btn_charge_pressed;
//         if (btn_charge_pressed) {
//             lastDebounceCHARGE++;
//             Serial.print("Charge button pressed id ");
//             Serial.println(lastDebounceCHARGE);
//         }
//     }
// }

void set_state(uint8_t new_state) {
    if (state == new_state)
        return;
    if (new_state == CHARGE_STATE_CHARGING) {
        //digitalWrite(EXT_SSEG_3, HIGH);

        //SSEG code if we want
        digitalWrite(RELAY_SWITCH_PIN, HIGH);
    } else if (new_state == CHARGE_STATE_CHARGE_REQUESTED) {
        Charge_status req_msg = Charge_status();
        req_msg.setChargeCommand(1);
        req_msg.write(msg.buf);
        msg.id = ID_CHARGE_STATUS;
        msg.len = 1;
        CAN.write(msg);
    }
}
