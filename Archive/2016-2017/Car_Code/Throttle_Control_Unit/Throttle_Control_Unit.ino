#include <FlexCAN.h> // import teensy library
#include <Metro.h>
#include "HyTech17.h"

//ports
#define BRAKE_ANALOG_PORT A2 //analog port of brake sensor
#define THROTTLE_PORT_1 A0 //first throttle sensor port
#define THROTTLE_PORT_2 A1 //second throttle sensor port
// TODO: These values need to be determined from testing
//constants
#define MIN_THROTTLE_1 0//compare pedal travel
#define MAX_THROTTLE_1 1024
#define MIN_THROTTLE_2 0
#define MAX_THROTTLE_2 1024
#define MIN_BRAKE 0
#define MAX_BRAKE 1024

//Description of Throttle Control Unit
//Senses the angle of each pedal to determine safely how much torque the motor controller should produce with the motor
//Reads a signal from the Brake System Plausibility Device located on the same board to sense if the BSPD has detected a fault.

FlexCAN CAN(500000);
static CAN_message_t msg;

int voltageThrottlePedal1 = 0; //voltage of 1st throttle
int voltageThrottlePedal2 = 0; //voltage of 2nd throttle
int voltageBrakePedal = 0;//voltage of brakepedal

// additional values to report
bool throttleImplausibility = false; // for throttle, not brake
bool throttleCurve = false; // false -> normal, true -> boost
float thermTemp = 0.0; // temperature of onboard thermistor
bool brakeImplausibility = false; // fault if BSPD signal too low - still to be designed
bool brakePedalActive = false; // true if brake is considered pressed
uint8_t oldBtnId = 0;
uint8_t expBtnId = 0;

// FSAE requires that torque be shut off if an implausibility persists for over 100 msec (EV3.5.4).
bool torqueShutdown = false;

// FUNCTION PROTOTYPES
void readValues();
bool checkDeactivateTractiveSystem();
int sendCANUpdate();

// State - use HyTech library
uint8_t state;

// timer
Metro CANUpdateTimer = Metro(500); // Used for how often to send state
Metro updateTimer = Metro(500); // Read in values from pins
Metro implausibilityTimer = Metro(50); // Used for throttle error check
Metro debugTimer = Metro(500);
Metro throttleTimer = Metro(100); // Used for sending commands to Motor Controller
Metro tractiveTimeOut = Metro(5000); // Used to check timeout of tractive system activation
Metro timer_motor_controller_send = Metro(50);

// setup code
void setup() {
    Serial.begin(115200); // init serial for PC communication

    CAN.begin(); // init CAN system
    Serial.println("CAN system and serial communication initialized");
    //To detect an open circuit
    //enable the pullup resistor on the Teensy input pin >>>
    pinMode(BRAKE_ANALOG_PORT, INPUT);
    pinMode(THROTTLE_PORT_1, INPUT);
    pinMode(THROTTLE_PORT_2, INPUT);
    //open circuit will show a high signal outside of the working range of the sensor.
    set_state(TCU_STATE_WAITING_SHUTDOWN_CIRCUIT_INITIALIZED);
}

void loop() {
    while (CAN.read(msg)) {
        // TODO: Handle CAN messages from other components (e.g. MC, Dashboard)
        if (msg.id == ID_PCU_STATUS) {
            PCU_status pcu_status(msg.buf);
            if (pcu_status.get_bms_fault()) {
                Serial.println("BMS Fault detected");
            }
            if (pcu_status.get_imd_fault()) {
                Serial.println("IMD Fault detected");
            }
            switch (pcu_status.get_state()) {
                // NOTE: see if this should be happening (depending on current TCU state)
                case PCU_STATE_WAITING_BMS_IMD:
                    break;
                case PCU_STATE_WAITING_DRIVER:
                    break;
                case PCU_STATE_LATCHING:
                    set_state(TCU_STATE_WAITING_SHUTDOWN_CIRCUIT_INITIALIZED);
                    break;
                case PCU_STATE_FATAL_FAULT:
                    // assuming shutdown_circuit has opened
                    set_state(TCU_STATE_TRACTIVE_SYSTEM_NOT_ACTIVE);
                    break;
                case PCU_STATE_SHUTDOWN_CIRCUIT_INITIALIZED:
                    // TCU must wait until PCU in PCU_STATE_SHUTDOWN_CIRCUIT_INITIALIZED to go into TCU_STATE_WAITING_TRACTIVE_SYSTEM
                    if (state == TCU_STATE_WAITING_SHUTDOWN_CIRCUIT_INITIALIZED || state == TCU_STATE_TRACTIVE_SYSTEM_NOT_ACTIVE) {
                        set_state(TCU_STATE_WAITING_TRACTIVE_SYSTEM);

                        // reset tractive system timeout
                        tractiveTimeOut.reset();
                    }
                    break;
            }
        } else if (msg.id == ID_MC_VOLTAGE_INFORMATION) {
            MC_voltage_information mc_voltage_information(msg.buf);
            if (state == TCU_STATE_WAITING_TRACTIVE_SYSTEM) {
                // This code checks (when waiting for tractive system) if the tractive system has turned on
                if (mc_voltage_information.get_dc_bus_voltage() > 100) {
                    // Assume this condition is the way to check if tractive system is on
                    set_state(TCU_STATE_TRACTIVE_SYSTEM_ACTIVE);
                    // NOTE: You must assume that for tractive system to turn on, the AIRs will be closed
                }
            }
        } else if (msg.id == ID_MC_INTERNAL_STATES) {
            MC_internal_states mc_internal_states(msg.buf);
            if (state == TCU_STATE_ENABLING_INVERTER) {
                // This code checks if the inverter has turned on
                if (mc_internal_states.get_inverter_enable_state()) {
                    set_state(TCU_STATE_WAITING_READY_TO_DRIVE_SOUND);
                }
            }
        } else if (msg.id == ID_DCU_STATUS) {
            DCU_status dcu_status(msg.buf);
            if (state == TCU_STATE_TRACTIVE_SYSTEM_ACTIVE) {
                oldBtnId = dcu_status.get_btn_press_id();
            }
        }
    }

    if (updateTimer.check()) {
        readValues();
        updateTimer.reset();
    }

    // implausibility Timer checking
    if (!checkDeactivateTractiveSystem()) {
        // timer reset if you should not deactivate tractive system
        implausibilityTimer.reset();
    }

    if (CANUpdateTimer.check()){
        sendCANUpdate();
    }
    switch(state) {
        //TODO: check if reqs are met to move to each state
        case TCU_STATE_WAITING_SHUTDOWN_CIRCUIT_INITIALIZED:
            // NOTE: Process handled in CAN message handler
            break;
        case TCU_STATE_TRACTIVE_SYSTEM_NOT_ACTIVE:
            // TCU must wait until PCU in SHUTDOWN_CIRCUIT_INITIALIZED state
            // NOTE: Process handled in CAN message handler
            break;
        case TCU_STATE_WAITING_TRACTIVE_SYSTEM:
            // NOTE: Assume tractive system will go off if shut down circuit opens - handled in CAN read

            if (tractiveTimeOut.check()) {
                // time out has occured, tractive system not active state
                set_state(TCU_STATE_TRACTIVE_SYSTEM_NOT_ACTIVE);
            }

            break;
        case TCU_STATE_TRACTIVE_SYSTEM_ACTIVE:
            // TODO - make sure start button and brake pressed
            // REVIEW: TCU will check for brake and start button press immediately (no delay?)
            // NOTE: there is no timeout for the above state change
            if (brakePedalActive) {
                // check Start button on dashboard - set in CAN message processing
                // TODO There is no checking for if brakePedalActive
                if (oldBtnId == expBtnId) {
                    // Sending enable torque message
                    set_state(TCU_STATE_ENABLING_INVERTER);
                    MC_command_message enableInverterCommand;
                    enableInverterCommand.set_inverter_enable(true);
                    uint8_t MC_enable_message[8];
                    enableInverterCommand.write(MC_enable_message);

                    msg.id = ID_MC_COMMAND_MESSAGE;
                    msg.len = 8;
                    memcpy(&msg.buf[0], &MC_enable_message[0], sizeof(uint8_t));
                    memcpy(&msg.buf[1], &MC_enable_message[1], sizeof(uint8_t));
                    memcpy(&msg.buf[2], &MC_enable_message[2], sizeof(uint8_t));
                    memcpy(&msg.buf[3], &MC_enable_message[3], sizeof(uint8_t));
                    memcpy(&msg.buf[4], &MC_enable_message[4], sizeof(uint8_t));
                    memcpy(&msg.buf[5], &MC_enable_message[5], sizeof(uint8_t));
                    memcpy(&msg.buf[6], &MC_enable_message[6], sizeof(uint8_t));
                    memcpy(&msg.buf[7], &MC_enable_message[7], sizeof(uint8_t));

                    CAN.write(msg);
                }
            }
            break;
        case TCU_STATE_ENABLING_INVERTER:
            // NOTE: checking for inverter enable done in CAN message handler
            break;
        case TCU_STATE_WAITING_READY_TO_DRIVE_SOUND:
            // TODO: sound goes off
            // TODO: state change if sound finished
            set_state(TCU_STATE_READY_TO_DRIVE);
            break;
        case TCU_STATE_READY_TO_DRIVE:
            break;
    }

    /*
     * Send a message to the Motor Controller over CAN when vehicle is not ready to drive
     */
    if (state < TCU_STATE_READY_TO_DRIVE && timer_motor_controller_send.check()) {
      msg.id = ID_MC_COMMAND_MESSAGE;
      msg.len = 8;
      if (state < TCU_STATE_ENABLING_INVERTER) {
        generate_MC_message(msg.buf, 0, true, false);
      } else if (state < TCU_STATE_READY_TO_DRIVE) {
        generate_MC_message(msg.buf, 0, true, true);
      }
      CAN.write(msg);
    }
    if (state == TCU_STATE_READY_TO_DRIVE && timer_motor_controller_send.check()) {
        MC_command_message mccm = MC_command_message();
        int16_t cmd_torque = voltageThrottlePedal1 / 4; // THROTTLE CURVE LOL TODO
        if (!throttleImplausibility && !brakeImplausibility)
          mccm.set_torque_command(cmd_torque);
        else
          mccm.set_torque_command(0);
        mccm.set_angular_velocity(0);
        mccm.set_direction(1);
        mccm.set_inverter_enable(1);
        mccm.set_discharge_enable(0);
        mccm.write(msg.buf);
        msg.id = ID_MC_COMMAND_MESSAGE;
        msg.len = 8;
        CAN.write(msg);
    }
}

void readValues() {
    voltageThrottlePedal1 = analogRead(THROTTLE_PORT_1);
    voltageThrottlePedal2 = analogRead(THROTTLE_PORT_2);
    voltageBrakePedal = analogRead(BRAKE_ANALOG_PORT);
    if (voltageBrakePedal > 300)    // TODO: Fix this once baseline values are determined
        brakePedalActive = true;
    else
        brakePedalActive = false;
    Serial.print("Throttle 1: ");
    Serial.print(voltageThrottlePedal1);
    Serial.print("     Throttle 2: ");
    Serial.print(voltageThrottlePedal2);
    Serial.print("     Brake: ");
    Serial.print(voltageBrakePedal);
    Serial.print(" ");
    Serial.println(brakePedalActive);
    //TODO: decide/set torque values for input values
}

bool checkDeactivateTractiveSystem() {
    // Check for errors - AKA implausibility checking
    // Throttle 10% check
    float deviationCheck = ((float) voltageThrottlePedal1) / ((float) voltageThrottlePedal2);
    int difference = abs(voltageThrottlePedal1 - voltageThrottlePedal2);
    if (difference > (MAX_THROTTLE_1 - MIN_THROTTLE_1) / 10.0) {
        throttleImplausibility = true;
    } else if (voltageThrottlePedal1 < MIN_THROTTLE_1 || voltageThrottlePedal2 < MIN_THROTTLE_2) {
        // Checks for failure of position sensor wiring
        // Check for open circuit or short to ground
        throttleImplausibility = true;
    } else if (voltageThrottlePedal1 > MAX_THROTTLE_1 || voltageThrottlePedal2 > MAX_THROTTLE_2) {
        // Check for short to power
        throttleImplausibility = true;
    } else {
        throttleImplausibility = false;
    }

    // Check brake pedal sensor
    if (voltageBrakePedal > MAX_BRAKE || voltageBrakePedal < MIN_BRAKE) {
        brakeImplausibility = true;
    } else {
        // No brake implausibility detected
        brakeImplausibility = false;
    }
    

    // return true if any implausibility present
    if (throttleImplausibility || brakeImplausibility) {
        if (debugTimer.check()) {
          Serial.print("IMPLAUSIBILITY -- Throttle: ");
          Serial.print(throttleImplausibility);
          Serial.print(" -- Brake: ");
          Serial.println(brakeImplausibility);
        }
        return true;
    } else {
        return false;
    }
}

void generate_MC_message(unsigned char* message, int torque, boolean backwards, boolean enable) {
  message[0] = torque & 0xFF;
  message[1] = torque >> 8;
  message[2] = 0;
  message[3] = 0;
  message[4] = char(backwards);
  message[5] = char(enable);
  message[6] = 0;
  message[7] = 0;
}

int sendCANUpdate(){
    int bufferAvailable = 0;

    /* --------- Sending TCU_status --------- */
    TCU_status curTCU_status = TCU_status();
    msg.id = ID_TCU_STATUS;
    msg.len = 8;

    curTCU_status.set_throttle_implausibility(throttleImplausibility);
    curTCU_status.set_throttle_curve(throttleCurve);
    curTCU_status.set_brake_implausibility(brakeImplausibility);
    curTCU_status.set_brake_pedal_active(brakePedalActive);
    curTCU_status.set_state(state);

    curTCU_status.write(msg.buf);
    bufferAvailable += CAN.write(msg);

    /* --------- Sending TCU_readings ------- */
    TCU_readings curTCU_readings = TCU_readings();
    msg.id = ID_TCU_READINGS;
    msg.len = 8;

    short shortThrottle1 = (short) voltageThrottlePedal1 * 100;
    short shortThrottle2 = (short) voltageThrottlePedal2 * 100;
    short shortBrake = (short) voltageBrakePedal * 100;
    short shortTemp = (short) thermTemp * 100;

    curTCU_readings.set_throttle_value_1(shortThrottle1);
    curTCU_readings.set_throttle_value_2(shortThrottle2);
    curTCU_readings.set_brake_value(shortBrake);
    curTCU_readings.set_temperature(shortTemp);

    curTCU_readings.write(msg.buf);
    bufferAvailable += CAN.write(msg);

    return bufferAvailable; // used for error checking
}

void set_state(uint8_t new_state) {
    // Use if there are special state change cases
    if (state == new_state) {
        return; // don't do anything if same state
    } else if (new_state == TCU_STATE_WAITING_SHUTDOWN_CIRCUIT_INITIALIZED) {
        // setup() call
        expBtnId = oldBtnId + 1;
        state = TCU_STATE_WAITING_SHUTDOWN_CIRCUIT_INITIALIZED;
    } else {
        switch (state) {
            // NOTE: handles state transitions in special cases
            case TCU_STATE_TRACTIVE_SYSTEM_ACTIVE:
                if (new_state == TCU_STATE_ENABLING_INVERTER) {
                    state = TCU_STATE_ENABLING_INVERTER;
                    break;
                }
                break;
            default:
                uint8_t old_state = state;
                state = new_state;
                break;
        }
    }
    sendCANUpdate(); // New message sent since change of state
}
