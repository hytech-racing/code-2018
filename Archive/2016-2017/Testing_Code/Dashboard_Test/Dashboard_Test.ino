#include <FlexCAN.h>
#include <HyTech17.h>
#include <Metro.h>

/******* PIN definitions ***********/
/**
 * THIS IS MODIFIED FOR TESTING
 * SWAP COMMENTS FOR REAL CAR
 */
#define BTN_CYCLE A1
//#define BTN_BOOST A16
// #define BTN_START A17
#define BTN_START A0
//#define LED_START 7
#define LED_START 7
//#define LED_BMS 6
#define LED_BMS 6
#define LED_IMD 5
#define READY_SOUND 8

/*****Dashboard States and Fault Flags********/
bool imd_fault;
bool bms_fault;
bool bspd_fault;
bool cool_fault;
bool comm_fault;
bool motor_fault;
bool pedal_fault;
bool general_fault;

Metro timer_btn_start = Metro(10);
Metro timer_led_start_blink_fast = Metro(250);
Metro timer_led_start_blink_slow = Metro(500);
Metro timer_inverter_enable = Metro(2000);  // Timeout failed inverter enable
Metro timer_ready_sound = Metro(2000);      // Time to play RTD sound
Metro timer_can_update = Metro(500);

unsigned long lastDebounceTOGGLE = 0;   // the last time the output pin was toggled
unsigned long lastDebounceBOOST = 0;  // the last time the output pin was toggled
unsigned long lastDebounceSTART = 0;  // the last time the output pin was toggled
uint8_t btn_start_new = 0;
bool btn_start_pressed = false;
bool btn_start_debouncing = false;
bool led_start_active = false;
unsigned long debounceDelay = 50;     // the debounce time; increase if the output flickers
uint8_t led_start_type = 0;
uint8_t state;

/*************** BUTTON TYPES ****************
 *  Start Button
 *  Toggle Button
 *  Select Button
 *  Boost Button
*/
int count;

/**
 * CAN Variables
 */
FlexCAN CAN(500000);
static CAN_message_t msg;

void setup() {
    // put your setup code here, to run once:
    imd_fault = false;
    bms_fault = false;
    bspd_fault = false;
    cool_fault = false;
    comm_fault = false;
    motor_fault = false;
    pedal_fault = false;
    general_fault = false;
    state = DCU_STATE_INITIAL_STARTUP;
    pinMode(LED_BMS, OUTPUT);
    pinMode(LED_IMD, OUTPUT);
    pinMode(LED_START, OUTPUT);
    pinMode(READY_SOUND, OUTPUT);
    pinMode(BTN_CYCLE, INPUT_PULLUP);
    pinMode(BTN_START, INPUT_PULLUP);
    Serial.begin(115200);
    CAN.begin();
    timer_can_update.reset();
}

void loop() {
  // put your main code here, to run repeatedly:
  while (CAN.read(msg)) {
    // Handle PCU (power board) status messages
    if (msg.id == ID_PCU_STATUS) {
      // Load message into PCU_status object
      PCU_status pcu_status(msg.buf);

      Serial.print("PCU State: ");
      Serial.println(pcu_status.get_state());

      // Handle PCU fault states
      if (pcu_status.get_bms_fault()) {
        bms_fault = true;
        digitalWrite(LED_BMS, HIGH);
      } else {
        bms_fault = false;
        digitalWrite(LED_BMS, LOW);
      }
      if (pcu_status.get_imd_fault()) {
        imd_fault = true;
        digitalWrite(LED_IMD, HIGH);
      } else {
        imd_fault = false;
        digitalWrite(LED_IMD, LOW);
      }
      // Set Dashboard internal state based on PCU state
      // If not ready to power up, or ready to drive, start light off
      // If ready and waiting for 1st press, flash start light slow
      // If waiting for 2nd press, flash start light fast
      switch (pcu_status.get_state()) {
        case PCU_STATE_WAITING_BMS_IMD:
          set_start_led(0);
          set_state(DCU_STATE_WAITING_TRACTIVE_SYSTEM);
          break;
        case PCU_STATE_WAITING_DRIVER:
          set_start_led(3); // slow blink
          set_state(DCU_STATE_WAITING_TRACTIVE_SYSTEM);
          break;
        case PCU_STATE_LATCHING:
          set_state(DCU_STATE_PRESSED_TRACTIVE_SYSTEM);
          break;
        case PCU_STATE_SHUTDOWN_CIRCUIT_INITIALIZED:
          if (state != DCU_STATE_WAITING_MC_ENABLE)
            set_start_led(0);
          break;
        case PCU_STATE_FATAL_FAULT:
          set_state(DCU_STATE_FATAL_FAULT);
          break;
      }
    }

    // Handle TCU broadcast state messages
    if (msg.id == ID_TCU_STATUS) {
        TCU_status tcu_status(msg.buf);
        Serial.print("TCU State: ");
        Serial.println(tcu_status.get_state());
        switch (tcu_status.get_state()) {
            case TCU_STATE_TRACTIVE_SYSTEM_ACTIVE:
                set_start_led(2); // fast blink
                set_state(DCU_STATE_WAITING_MC_ENABLE);
                break;
            case TCU_STATE_WAITING_READY_TO_DRIVE_SOUND:
                set_state(DCU_STATE_PLAYING_RTD);
                break;
            case TCU_STATE_READY_TO_DRIVE:
                set_state(DCU_STATE_READY_TO_DRIVE);
                break;
        }
    }
  }

  // TODO: more state machine stuff possibly?
  /*
   * State machine
   */
  switch (state) {
      case DCU_STATE_WAITING_TRACTIVE_SYSTEM:
        if (btn_start_new == lastDebounceSTART) {
            set_state(DCU_STATE_PRESSED_TRACTIVE_SYSTEM);
        }
        break;
      case DCU_STATE_WAITING_MC_ENABLE:
        if (btn_start_new == lastDebounceSTART) {
            set_state(DCU_STATE_PRESSED_MC_ENABLE);
        }
        break;
      case DCU_STATE_PRESSED_MC_ENABLE:
        if (timer_inverter_enable.check()) {    // inverter did not enable
            set_state(DCU_STATE_TS_INACTIVE);
        }
        break;
      case DCU_STATE_PLAYING_RTD:
        if (timer_ready_sound.check()) {        // RTD sound is finished
            set_state(DCU_STATE_READY_TO_DRIVE);
        }
        break;
  }


  if (timer_can_update.check()) {
      sendCANUpdate();
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

  // TODO: other buttons
  pollForButtonPress(); // fix this
}

void pollForButtonPress() {
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
      lastDebounceSTART++;
      Serial.print("Start button pressed id ");
      Serial.println(lastDebounceSTART);
      sendCANUpdate();
    }
  }
  if (digitalRead(BTN_CYCLE) == LOW) {
    digitalWrite(READY_SOUND, HIGH);
  } else {
    digitalWrite(READY_SOUND, LOW);
  }
}

void sendCANUpdate() {
    msg.id = ID_DCU_STATUS;
    msg.len = 8;
    DCU_status dcu_status = DCU_status();
    dcu_status.set_btn_press_id(lastDebounceSTART);
    dcu_status.set_light_active_1(0);
    dcu_status.set_light_active_2(0);
    dcu_status.set_rtds_state(state == DCU_STATE_PLAYING_RTD ? 1 : 0);
    dcu_status.write(msg.buf);
    CAN.write(msg);
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
      Serial.println("Setting Start LED off");
      return;
    }

    digitalWrite(LED_START, HIGH);
    led_start_active = true;

    if (type == 1) {
      Serial.println("Setting Start LED solid on");
    } else if (type == 2) {
      timer_led_start_blink_fast.reset();
      Serial.println("Setting Start LED fast blink");
    } else if (type == 3) {
      timer_led_start_blink_slow.reset();
      Serial.println("Setting Start LED slow blink");
    }
  }
}

void test_flash() {
  digitalWrite(LED_BMS, HIGH);
  digitalWrite(LED_IMD, HIGH);
  digitalWrite(LED_START, HIGH);
  digitalWrite(READY_SOUND, HIGH);
  Serial.print("Button 1: ");
  Serial.print(digitalRead(BTN_START));
  Serial.print("  Button 3: ");
  Serial.println(digitalRead(BTN_CYCLE));
}

void set_state(uint8_t new_state) {
    if (state == new_state)
        return;
    state = new_state;
    if (new_state == DCU_STATE_WAITING_MC_ENABLE || new_state == DCU_STATE_WAITING_TRACTIVE_SYSTEM) {
        btn_start_new = lastDebounceSTART + 1;
    }
    if (new_state == DCU_STATE_PRESSED_MC_ENABLE || new_state == DCU_STATE_PRESSED_TRACTIVE_SYSTEM) {
        set_start_led(0);
        sendCANUpdate();
    }
    if (new_state == DCU_STATE_PLAYING_RTD) {
        timer_ready_sound.reset();
        digitalWrite(READY_SOUND, HIGH);
        Serial.println("Playing RTD sound");
    }
    if (new_state == DCU_STATE_READY_TO_DRIVE) {
        digitalWrite(READY_SOUND, LOW);
        Serial.println("RTD sound finished");
    }
    if (new_state == DCU_STATE_FATAL_FAULT) {
      set_start_led(0);
    }
}
