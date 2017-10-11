/*
 * HyTech 2017 Vehicle Front Control Unit
 * Init 2017-05-13
 * Interface with dashboard lights, buttons, and buzzer.
 * Read pedal sensor values and communicate with motor controller.
 * Configured for Pedal Box Board rev3 
 */
#include <FlexCAN.h>
#include "HyTech17.h"
#include <Metro.h>
#include <MCP3208.h>
/*
 * Pin definitions
 */
#define BSPD_FAULT A7
#define BTN_START A5
#define LED_START 5
#define LED_BMS 6
#define LED_BSPD 13
#define LED_IMD 7
#define PEDAL_BRAKE 3 //adc port of brake sensor
#define PEDAL_THROTTLE_1 0 //first throttle sensor port on adc
#define PEDAL_THROTTLE_2 2 //second throttle sensor port on adc
#define READY_SOUND 2
#define SOFTWARE_SHUTDOWN_RELAY 12
#define CS_PIN A0
#define CLOCK_PIN 13
#define MOSI_PIN 1
#define MISO_PIN 0

/*
 * Constant definitions
 */
// TODO some of these values need to be calibrated once hardware is installed
#define BRAKE_ACTIVE 282
#define MIN_THROTTLE_1 463 // compare pedal travel
#define MAX_THROTTLE_1 246
#define MIN_THROTTLE_2 93
#define MAX_THROTTLE_2 306
#define MIN_BRAKE 242
#define MAX_BRAKE 306
#define MAX_TORQUE 1600 // Torque in Nm * 10
#define MIN_HV_VOLTAGE 500 // Used to check if Accumulator is energized

/*
 * Timers
 */
Metro timer_btn_start = Metro(10);
Metro timer_debug = Metro(500);
Metro timer_debug_rear_state = Metro(500);
Metro timer_debug_rms_current_information = Metro(500);
Metro timer_debug_rms_fault_codes = Metro(500);
Metro timer_debug_rms_internal_states = Metro(500);
Metro timer_debug_rms_motor_position_information = Metro(500);
Metro timer_debug_rms_temperatures_1 = Metro(500);
Metro timer_debug_rms_temperatures_3 = Metro(500);
Metro timer_debug_rms_torque_timer_information = Metro(500);
Metro timer_debug_rms_voltage_information = Metro(500);
Metro timer_debug_torque = Metro(500);
Metro timer_inverter_enable = Metro(2000); // Timeout failed inverter enable
Metro timer_led_start_blink_fast = Metro(150);
Metro timer_led_start_blink_slow = Metro(400);
Metro timer_motor_controller_send = Metro(50);
Metro timer_ready_sound = Metro(2000); // Time to play RTD sound
Metro timer_can_update = Metro(100);

/*
 * Global variables
 */
bool brake_pedal_active = false; // True if brake is considered pressed
bool btn_start_debouncing = false;
uint8_t btn_start_id = 0; // Increments to differentiate separate button presses
uint8_t btn_start_new = 0;
bool btn_start_pressed = false;
bool debug = true;
bool fsae_brake_pedal_implausibility = false; // FSAE EV2.5
bool fsae_throttle_pedal_implausibility = false;
bool led_start_active = false;
uint8_t led_start_type = 0; // 0 for off, 1 for steady, 2 for fast blink, 3 for slow blink
uint8_t state = TCU_STATE_WAITING_SHUTDOWN_CIRCUIT_INITIALIZED;
double temperature; // Temperature of onboard thermistor
uint16_t value_pedal_brake = 0;
uint16_t value_pedal_throttle_1 = 0;
uint16_t value_pedal_throttle_2 = 0;

FlexCAN CAN(500000);
static CAN_message_t msg;

MCP3208 adc(CLOCK_PIN, MOSI_PIN, MISO_PIN, CS_PIN); // for analog to digital converter

void setup() {
  pinMode(BSPD_FAULT, INPUT);
  pinMode(BTN_START, INPUT_PULLUP);
  pinMode(LED_BMS, OUTPUT);
  pinMode(LED_BSPD, OUTPUT);
  pinMode(LED_IMD, OUTPUT);
  pinMode(LED_START, OUTPUT);
  
  pinMode(PEDAL_BRAKE, INPUT);
  pinMode(PEDAL_THROTTLE_1, INPUT);
  pinMode(PEDAL_THROTTLE_2, INPUT);
  pinMode(READY_SOUND, OUTPUT);
  pinMode(SOFTWARE_SHUTDOWN_RELAY, OUTPUT);
  pinMode(13, OUTPUT);

  Serial.begin(115200); // init serial for PC communication
  CAN.begin();
  delay(100);
  Serial.println("CAN system and serial communication initialized");

  digitalWrite(SOFTWARE_SHUTDOWN_RELAY, HIGH);
  digitalWrite(13, HIGH); // Used to indicate power

  // Send restart message, so Rear ECU knows to power cycle the inverter (in case of CAN message timeout from TCU to inverter)
  msg.id = ID_TCU_RESTART;
  msg.len = 1;
  CAN.write(msg);
}

void loop() {
  while (CAN.read(msg)) {
    // Handle PCU (power board) status messages
    if (msg.id == ID_PCU_STATUS) {
      // Load message into PCU_status object
      PCU_status pcu_status(msg.buf);
      if (pcu_status.get_bms_fault()) {
        digitalWrite(LED_BMS, HIGH);
        Serial.println("RCU BMS FAULT: detected");
      }
      if (pcu_status.get_imd_fault()) {
        digitalWrite(LED_IMD, HIGH);
        Serial.println("RCU IMD FAULT: detected");
      }
      if (debug && timer_debug_rear_state.check()) {
        Serial.print("RCU STATE: ");
        Serial.println(pcu_status.get_state());
      }

      // Set internal state based on PCU state
      // If initializing, start light off
      // If waiting for driver press, flash start light slow
      switch (pcu_status.get_state()) {
        case PCU_STATE_WAITING_BMS_IMD:
        set_start_led(0);
        set_state(TCU_STATE_WAITING_SHUTDOWN_CIRCUIT_INITIALIZED);
        break;

        case PCU_STATE_WAITING_DRIVER:
        set_start_led(3); // Slow blink
        set_state(TCU_STATE_WAITING_SHUTDOWN_CIRCUIT_INITIALIZED);
        break;

        case PCU_STATE_LATCHING:
        set_start_led(0);
        set_state(TCU_STATE_WAITING_SHUTDOWN_CIRCUIT_INITIALIZED);
        break;

        case PCU_STATE_SHUTDOWN_CIRCUIT_INITIALIZED:
        if (state < TCU_STATE_TRACTIVE_SYSTEM_NOT_ACTIVE) {
          set_state(TCU_STATE_TRACTIVE_SYSTEM_NOT_ACTIVE);
        }
        break;

        case PCU_STATE_FATAL_FAULT:
        set_start_led(0);
        set_state(TCU_STATE_TRACTIVE_SYSTEM_NOT_ACTIVE);
        break;
      }
    }

    if (msg.id == ID_MC_TEMPERATURES_1 && debug && timer_debug_rms_temperatures_1.check()) {
      MC_temperatures_1 mc_temperatures_1 = MC_temperatures_1(msg.buf);
      Serial.print("MODULE A TEMP: ");
      Serial.println(mc_temperatures_1.get_module_a_temperature());
      Serial.print("MODULE B TEMP: ");
      Serial.println(mc_temperatures_1.get_module_b_temperature());
      Serial.print("MODULE C TEMP: ");
      Serial.println(mc_temperatures_1.get_module_c_temperature());
      Serial.print("GATE DRIVER BOARD TEMP: ");
      Serial.println(mc_temperatures_1.get_gate_driver_board_temperature());
    }

    if (msg.id ==ID_MC_TEMPERATURES_3 && debug && timer_debug_rms_temperatures_3.check()) {
      MC_temperatures_3 mc_temperatures_3 = MC_temperatures_3(msg.buf);
      Serial.print("RTD 4 TEMP: ");
      Serial.println(mc_temperatures_3.get_rtd_4_temperature());
      Serial.print("RTD 5 TEMP: ");
      Serial.println(mc_temperatures_3.get_rtd_5_temperature());
      Serial.print("MOTOR TEMP: ");
      Serial.println(mc_temperatures_3.get_motor_temperature());
      Serial.print("TORQUE SHUDDER: ");
      Serial.println(mc_temperatures_3.get_torque_shudder());
    }

    if (msg.id == ID_MC_MOTOR_POSITION_INFORMATION && debug && timer_debug_rms_motor_position_information.check()) {
      MC_motor_position_information mc_motor_position_information = MC_motor_position_information(msg.buf);
      Serial.print("MOTOR ANGLE: ");
      Serial.println(mc_motor_position_information.get_motor_angle());
      Serial.print("MOTOR SPEED: ");
      Serial.println(mc_motor_position_information.get_motor_speed());
      Serial.print("ELEC OUTPUT FREQ: ");
      Serial.println(mc_motor_position_information.get_electrical_output_frequency());
      Serial.print("DELTA RESOLVER FILT: ");
      Serial.println(mc_motor_position_information.get_delta_resolver_filtered());
    }

    if (msg.id == ID_MC_CURRENT_INFORMATION && debug && timer_debug_rms_current_information.check()) {
      MC_current_information mc_current_information = MC_current_information(msg.buf);
      Serial.print("PHASE A CURRENT: ");
      Serial.println(mc_current_information.get_phase_a_current());
      Serial.print("PHASE B CURRENT: ");
      Serial.println(mc_current_information.get_phase_b_current());
      Serial.print("PHASE C CURRENT: ");
      Serial.println(mc_current_information.get_phase_c_current());
      Serial.print("DC BUS CURRENT: ");
      Serial.println(mc_current_information.get_dc_bus_current());
    }

    if (msg.id == ID_MC_VOLTAGE_INFORMATION) {
      MC_voltage_information mc_voltage_information = MC_voltage_information(msg.buf);
      if (debug && timer_debug_rms_voltage_information.check()) {
        Serial.print("DC BUS VOLTAGE: ");
        Serial.println(mc_voltage_information.get_dc_bus_voltage());
        Serial.print("OUTPUT VOLTAGE: ");
        Serial.println(mc_voltage_information.get_output_voltage());
        Serial.print("PHASE AB VOLTAGE: ");
        Serial.println(mc_voltage_information.get_phase_ab_voltage());
        Serial.print("PHASE BC VOLTAGE: ");
        Serial.println(mc_voltage_information.get_phase_bc_voltage());
      }
      if (mc_voltage_information.get_dc_bus_voltage() >= MIN_HV_VOLTAGE && state == TCU_STATE_TRACTIVE_SYSTEM_NOT_ACTIVE) {
        set_state(TCU_STATE_TRACTIVE_SYSTEM_ACTIVE);
      }
      if (mc_voltage_information.get_dc_bus_voltage() < MIN_HV_VOLTAGE && state > TCU_STATE_TRACTIVE_SYSTEM_NOT_ACTIVE) {
        set_state(TCU_STATE_TRACTIVE_SYSTEM_NOT_ACTIVE);
      }
    }

    if (msg.id == ID_MC_INTERNAL_STATES) {
      MC_internal_states mc_internal_states = MC_internal_states(msg.buf);
      if (debug && timer_debug_rms_internal_states.check()) {
        Serial.print("VSM STATE: ");
        Serial.println(mc_internal_states.get_vsm_state());
        Serial.print("INVERTER STATE: ");
        Serial.println(mc_internal_states.get_inverter_state());
        Serial.print("INVERTER RUN MODE: ");
        Serial.println(mc_internal_states.get_inverter_run_mode());
        Serial.print("INVERTER ACTIVE DISCHARGE STATE: ");
        Serial.println(mc_internal_states.get_inverter_active_discharge_state());
        Serial.print("INVERTER COMMAND MODE: ");
        Serial.println(mc_internal_states.get_inverter_command_mode());
        Serial.print("INVERTER ENABLE: ");
        Serial.println(mc_internal_states.get_inverter_enable_state());
        Serial.print("INVERTER LOCKOUT: ");
        Serial.println(mc_internal_states.get_inverter_enable_lockout());
        Serial.print("DIRECTION COMMAND: ");
        Serial.println(mc_internal_states.get_direction_command());
      }
      if (mc_internal_states.get_inverter_enable_state() && state == TCU_STATE_ENABLING_INVERTER) {
        set_state(TCU_STATE_WAITING_READY_TO_DRIVE_SOUND);
      }
    }

    if (msg.id == ID_MC_FAULT_CODES && debug && timer_debug_rms_fault_codes.check()) {
      MC_fault_codes mc_fault_codes = MC_fault_codes(msg.buf);
      Serial.print("POST FAULT LO: ");
      Serial.println(mc_fault_codes.get_post_fault_lo());
      Serial.print("POST FAULT HI: ");
      Serial.println(mc_fault_codes.get_post_fault_hi());
      Serial.print("RUN FAULT LO: ");
      Serial.println(mc_fault_codes.get_run_fault_lo());
      Serial.print("RUN FAULT HI: ");
      Serial.println(mc_fault_codes.get_run_fault_hi());
    }

    if (msg.id == ID_MC_TORQUE_TIMER_INFORMATION && debug && timer_debug_rms_torque_timer_information.check()) {
      MC_torque_timer_information mc_torque_timer_information = MC_torque_timer_information(msg.buf);
      Serial.print("COMMANDED TORQUE: ");
      Serial.println(mc_torque_timer_information.get_commanded_torque());
      Serial.print("TORQUE FEEDBACK: ");
      Serial.println(mc_torque_timer_information.get_torque_feedback());
      Serial.print("RMS UPTIME: ");
      Serial.println(mc_torque_timer_information.get_power_on_timer());
    }
  }

  /*
   * Send state over CAN
   */
  if (timer_can_update.check()) {
    // Send Throttle Control Unit message
    TCU_status tcu_status = TCU_status(fsae_throttle_pedal_implausibility,
      0, fsae_brake_pedal_implausibility, brake_pedal_active, state);
    tcu_status.write(msg.buf);
    msg.id = ID_TCU_STATUS;
    msg.len = sizeof(CAN_message_tcu_status_t);
    CAN.write(msg);

    // Send second Throttle Control Unit message
    read_values(); // Calculate new values to send
    TCU_readings tcu_readings = TCU_readings(value_pedal_throttle_1, value_pedal_throttle_2, value_pedal_brake, temperature);
    tcu_readings.write(msg.buf);
    msg.id = ID_TCU_READINGS;
    msg.len = sizeof(CAN_message_tcu_readings_t);
    CAN.write(msg);

    // Send Dashboard Control Unit message
    DCU_status dcu_status = DCU_status(btn_start_id, 0, 0, 0); // Nothing currently relies on the other data, so sending 0s for now
    dcu_status.write(msg.buf);
    msg.id = ID_DCU_STATUS;
    msg.len = sizeof(CAN_message_dcu_status_t);
    CAN.write(msg);
  }

  /*
   * State machine
   */
  switch (state) {
    case TCU_STATE_WAITING_SHUTDOWN_CIRCUIT_INITIALIZED:
    break;

    case TCU_STATE_TRACTIVE_SYSTEM_ACTIVE:
    if (btn_start_new == btn_start_id) { // Start button has been pressed
      if (brake_pedal_active) { // Required to hold brake pedal to activate motor controller
        set_state(TCU_STATE_ENABLING_INVERTER);
      } else {
        btn_start_new = btn_start_id + 1;
      }
    }
    break;

    case TCU_STATE_TRACTIVE_SYSTEM_NOT_ACTIVE:
    break;

    case TCU_STATE_ENABLING_INVERTER:
      if (timer_inverter_enable.check()) { // Inverter enable timeout
        set_state(TCU_STATE_TRACTIVE_SYSTEM_NOT_ACTIVE);
      }
    break;

    case TCU_STATE_WAITING_READY_TO_DRIVE_SOUND:
    if (timer_ready_sound.check()) { // RTDS has sounded
      set_state(TCU_STATE_READY_TO_DRIVE);
    }
    break;

    case TCU_STATE_READY_TO_DRIVE:
    if (timer_motor_controller_send.check()) {
      MC_command_message mc_command_message = MC_command_message(0, 0, 0, 1, 0, 0);
      read_values(); // Read new sensor values

      // Check for throttle implausibility FSAE EV2.3.10
      fsae_throttle_pedal_implausibility = false;
      /*if (value_pedal_throttle_1 < MIN_THROTTLE_1 || value_pedal_throttle_1 > MAX_THROTTLE_1) {
        fsae_throttle_pedal_implausibility = true;
      }
      if (value_pedal_throttle_2 < MIN_THROTTLE_2 || value_pedal_throttle_2 > MAX_THROTTLE_2) {
        fsae_throttle_pedal_implausibility = true;
      }*/

      // Calculate torque value
      int calculated_torque = 0;
      if (!fsae_throttle_pedal_implausibility) {
        int torque1 = map(value_pedal_throttle_1, MIN_THROTTLE_1, MAX_THROTTLE_1, 0, MAX_TORQUE);
        int torque2 = map(value_pedal_throttle_2, MIN_THROTTLE_2, MAX_THROTTLE_2, 0, MAX_TORQUE);
        /*if (abs(torque1 - torque2) * 100 / MAX_TORQUE > 10) { // Second throttle implausibility check FSAE EV2.3.6
          fsae_throttle_pedal_implausibility = true;
        } else {*/
          calculated_torque = min(torque1, torque2);
          Serial.print("FCU RAW TORQUE: ");
          Serial.println(calculated_torque);
          if (calculated_torque > MAX_TORQUE) {
            calculated_torque = MAX_TORQUE;
          }
          if (calculated_torque < 0) {
            calculated_torque = 0;
          }
        /*}*/
      }

      // FSAE EV2.5 APPS / Brake Pedal Plausibility Check
      if (fsae_brake_pedal_implausibility && !brake_pedal_active && calculated_torque <= (MAX_TORQUE / 4)) {
        fsae_brake_pedal_implausibility = false; // Clear implausibility
      }
      if (brake_pedal_active && calculated_torque > (MAX_TORQUE / 4)) {
        //fsae_brake_pedal_implausibility = true;
      }

      if (fsae_brake_pedal_implausibility || fsae_throttle_pedal_implausibility) {
        // Implausibility exists, command 0 torque
        calculated_torque = 0;
      }
      
      if (debug && timer_debug_torque.check()) {
        Serial.print("FCU REQUESTED TORQUE: ");
        Serial.println(calculated_torque);
        Serial.print("FCU IMPLAUS THROTTLE: ");
        Serial.println(fsae_throttle_pedal_implausibility);
        Serial.print("FCU IMPLAUS BRAKE: ");
        Serial.println(fsae_brake_pedal_implausibility);
      }
      
      mc_command_message.set_torque_command(calculated_torque);

      mc_command_message.write(msg.buf);
      msg.id = ID_MC_COMMAND_MESSAGE;
      msg.len = 8;
      CAN.write(msg);
    }
    break;
  }

  /*
   * Send a message to the Motor Controller over CAN when vehicle is not ready to drive
   */
  if (state < TCU_STATE_READY_TO_DRIVE && timer_motor_controller_send.check()) {
    MC_command_message mc_command_message = MC_command_message(0, 0, 0, 0, 0, 0);
    if (state >= TCU_STATE_ENABLING_INVERTER) {
      mc_command_message.set_inverter_enable(true);
    }
    mc_command_message.write(msg.buf);
    msg.id = ID_MC_COMMAND_MESSAGE;
    msg.len = 8;
    CAN.write(msg);
  }

  /*
   * Blink start led
   */
  if ((led_start_type == 2 && timer_led_start_blink_fast.check()) || (led_start_type == 3 && timer_led_start_blink_slow.check())) {
    if (led_start_active) {
      digitalWrite(LED_START, LOW);
    } else {
      digitalWrite(LED_START, HIGH);
    }
    led_start_active = !led_start_active;
  }

  /*
   * Handle start button press and depress
   */
  if (digitalRead(BTN_START) == btn_start_pressed && !btn_start_debouncing) { // Value is different than stored
    btn_start_debouncing = true;
    timer_btn_start.reset();
  }
  if (btn_start_debouncing && digitalRead(BTN_START) != btn_start_pressed) { // Value returns during debounce period
    btn_start_debouncing = false;
  }
  if (btn_start_debouncing && timer_btn_start.check()) { // Debounce period finishes without value returning
    btn_start_pressed = !btn_start_pressed;
    if (btn_start_pressed) {
      btn_start_id++;
      Serial.print("FCU START BUTTON ID: ");
      Serial.println(btn_start_id);
    }
  }

  /*
   * Illuminate BSPD fault LED
   */
  digitalWrite(LED_BSPD, digitalRead(BSPD_FAULT));
}

/*
 * Read values of sensors
 */
void read_values() {
    
    value_pedal_throttle_1 = (adc.readADC(PEDAL_THROTTLE_1)*5)/1024;
    value_pedal_throttle_2 = (adc.readADC(PEDAL_THROTTLE_2)*5)/1024;
    value_pedal_brake = (adc.readADC(PEDAL_BRAKE)*5)/1024;
    if (value_pedal_brake >= BRAKE_ACTIVE) {
      brake_pedal_active = true;
    } else {
      brake_pedal_active = false;
    }
    if (debug && timer_debug.check()) {
      Serial.print("FCU PEDAL THROTTLE 1: ");
      Serial.println(value_pedal_throttle_1);
      Serial.print("FCU PEDAL THROTTLE 2: ");
      Serial.println(value_pedal_throttle_2);
      Serial.print("FCU PEDAL BRAKE: ");
      Serial.println(value_pedal_brake);
      Serial.print("FCU BRAKE ACT: ");
      Serial.println(brake_pedal_active);
      Serial.print("FCU STATE: ");
      Serial.println(state);
    }
    // TODO calculate temperature
}

/*
 * Set the Start LED
 */
void set_start_led(uint8_t type) {
  if (led_start_type != type) {
    led_start_type = type;

    if (type == 0) {
      digitalWrite(LED_START, LOW);
      led_start_active = false;
      Serial.println("FCU Setting Start LED off");
      return;
    }

    digitalWrite(LED_START, HIGH);
    led_start_active = true;

    if (type == 1) {
      Serial.println("FCU Setting Start LED solid on");
    } else if (type == 2) {
      timer_led_start_blink_fast.reset();
      Serial.println("FCU Setting Start LED fast blink");
    } else if (type == 3) {
      timer_led_start_blink_slow.reset();
      Serial.println("FCU Setting Start LED slow blink");
    }
  }
}

/*
 * Handle changes in state
 */
void set_state(uint8_t new_state) {
  if (state == new_state) {
    return;
  }
  state = new_state;
  if (new_state == TCU_STATE_TRACTIVE_SYSTEM_NOT_ACTIVE) {
    set_start_led(0);
  }
  if (new_state == TCU_STATE_TRACTIVE_SYSTEM_ACTIVE) {
    set_start_led(2);
    btn_start_new = btn_start_id + 1;
  }
  if (new_state == TCU_STATE_ENABLING_INVERTER) {
    set_start_led(1);
    Serial.println("FCU Enabling inverter");
    MC_command_message mc_command_message = MC_command_message(0, 0, 0, 1, 0, 0);
    msg.id = 0xC0;
    msg.len = 8;
    for(int i = 0; i < 10; i++) {
      mc_command_message.write(msg.buf); // many enable commands
      CAN.write(msg);
    }
    mc_command_message.set_inverter_enable(false);
    mc_command_message.write(msg.buf); // disable command
    CAN.write(msg);
    for(int i = 0; i < 10; i++) {
      mc_command_message.set_inverter_enable(true);
      mc_command_message.write(msg.buf); // many more enable commands
      CAN.write(msg);
    }
    Serial.println("FCU Sent enable command");
    timer_inverter_enable.reset();
  }
  if (new_state == TCU_STATE_WAITING_READY_TO_DRIVE_SOUND) {
    timer_ready_sound.reset();
    digitalWrite(READY_SOUND, HIGH);
    Serial.println("Inverter enabled");
    Serial.println("RTDS enabled");
  }
  if (new_state == TCU_STATE_READY_TO_DRIVE) {
    digitalWrite(READY_SOUND, LOW);
    Serial.println("RTDS deactivated");
    Serial.println("Ready to drive");
  }
}
