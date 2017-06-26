#include <FlexCAN.h> // import teensy library
#include <HyTech17.h>
#include <Metro.h>

#define DRIVE_STATE 8 //PLACEHOLDER, need to decide right value for drive state from TCU
#define FAN_IDLE 40 //PLACEHOLDER
#define FAN_PIN_1 A7 
#define FAN_PIN_2 A8 
#define FAN_PIN_3 A9
#define TEMP_THRESHOLD 40
#define COUNTER_RESET A0
#define FAN_SPEED_MAX 255
#define MC1_ID 0xA0 
#define MC2_ID 0xA1 
#define MC3_ID 0xA2 
#define BMS_ID 0xD4
#define ID_CCU_STATUS 0x0 //PLACEHOLDER

FlexCAN CAN(500000);
static CAN_message_t msg;

//flow sensor variables
int pulses = 0;
float flowRate = 0.0;
int milli = 300; //time in milliseconds of each cycle to poll for
Metro pulseTimer = Metro(milli);

// State - use HyTech library
uint8_t state;

Metro timer = Metro(500); //adjust fan speeds every 500 millis
bool started =  false;
int fanSpeed_1 = FAN_IDLE; 
int fanSpeed_2 = FAN_IDLE;
int fanSpeed_3 = FAN_IDLE;

//objects 
MC_temperatures_1 mc1;
MC_temperatures_2 mc2;
MC_temperatures_3 mc3;
BMS_temperatures bms;

static CAN_message_t ccu_status_msg;
CCU_status curCCU_status;

// setup code
void setup() {
    Serial.begin(115200); // init serial

    //Pins for flow sensor
    pinMode(5, INPUT);
    pinMode(6, INPUT);
    pinMode(7, INPUT);
    pinMode(8, INPUT);
    pinMode(9, INPUT);
    pinMode(10, INPUT);
    pinMode(11, INPUT);
    pinMode(12, INPUT);
    pinMode(13, INPUT);
    pinMode(COUNTER_RESET, OUTPUT);
    pinMode(A1, INPUT);

    //fan output pins
    pinMode(FAN_PIN_1, OUTPUT);
    pinMode(FAN_PIN_2, OUTPUT);
    pinMode(FAN_PIN_3, OUTPUT);

    CAN.begin();
    Serial.println("CAN system and serial communication initialized");
}

// TODO use library functions to get temp values, etc.
void loop() {
    //check the flow sensor if timer is up
    if(pulseTimer.check()) { 
        int q1 = digitalRead(5);
        int q2 = digitalRead(6);
        int q3 = digitalRead(7);
        int q4 = digitalRead(8);
        int q5 = digitalRead(9);
        int q6 = digitalRead(10);
        int q7 = digitalRead(11);
        int q8 = digitalRead(12);
        int q9 = digitalRead(13);
        
        // if a plastic sensor use the following calculation
        // Sensor Frequency (Hz) = 7.5 * Q (Liters/min)
        // Liters = Q * time elapsed (fseconds) / 60 (seconds/minute)
        // Liters = (Frequency (Pulses/second) / 7.5) * time elapsed (seconds) / 60
        // Liters = Pulses / (7.5 * 60)
        pulses = q1 * (1 << 0) + q2 * (1 << 1) + q3 * (1 << 2) + q4 * (1 << 3) + q5 * (1<<4) 
            + q6 * (1<<5) + q7 * (1<<6) + q8 * (1<<7) + q9 * (1<<8);
        Serial.print("pulses: ");
        Serial.println(pulses);
        float freq = pulses/60.0;
        flowRate = freq/7.5;
        flowRate = flowRate/((float)milli/1000);
        Serial.print("flow rate: ");
        Serial.println(flowRate);
        //TODO send CAN message with the flow rate
        digitalWrite(COUNTER_RESET, HIGH);
        digitalWrite(COUNTER_RESET, LOW);
        pulseTimer.reset();
    }
    if (timer.check()) {
        while (CAN.read(msg)) {
            if (!started && msg.id == 0xD1 && msg.buf[1] == DRIVE_STATE) {
                started = true;
                //initial start up of car, so start up all fans and pump to some initial speed
                analogWrite(FAN_PIN_1, fanSpeed_1);
                analogWrite(FAN_PIN_2, fanSpeed_2);
                analogWrite(FAN_PIN_3, fanSpeed_3);
            }
            if (started) {
                //TODO figure out which fans are pointing at what aka what fans(3) are paired with what parts(4)
                if (msg.id == MC1_ID) {
                    mc1.load(msg.buf);
                    if (mc1.get_gate_driver_board_temperature() >= TEMP_THRESHOLD && fanSpeed_1 < FAN_SPEED_MAX) {
                        fanSpeed_1 += 1;
                    } else if (mc1.get_gate_driver_board_temperature() < TEMP_THRESHOLD && fanSpeed_1 > FAN_IDLE) {
                        fanSpeed_1 -= 1;
                    }
                    analogWrite(FAN_PIN_1, fanSpeed_1);

                } else if (msg.id == MC2_ID) {
                    mc2.load(msg.buf);
                    if (mc2.get_control_board_temperature() >= TEMP_THRESHOLD && fanSpeed_2 < FAN_SPEED_MAX) {
                        fanSpeed_2 += 1;
                    } else if (mc2.get_control_board_temperature() < TEMP_THRESHOLD && fanSpeed_2 > FAN_IDLE) {
                        fanSpeed_2 -= 1;
                    }
                    analogWrite(FAN_PIN_2, fanSpeed_2);
                } else if (msg.id == BMS_ID) {
                    bms.load(msg.buf);
                    if (bms.getAvgTemp() >= TEMP_THRESHOLD && fanSpeed_3 < FAN_SPEED_MAX) {
                        fanSpeed_3 += 1;
                    } else if (bms.getAvgTemp() < TEMP_THRESHOLD && fanSpeed_3 > FAN_IDLE) {
                        fanSpeed_3 -= 1;
                    }
                    analogWrite(FAN_PIN_3, fanSpeed_3);
                }
                sendCanUpdate();
            }
        }
        timer.reset();
    }
}

void sendCanUpdate() {
    //TODO write CAN update code
    ccu_status_msg.id = ID_CCU_STATUS;
    ccu_status_msg.len = 1;

    
    short shortFlowRate = (short) (flowRate * 10);
    curCCU_status.set_flow_rate(shortFlowRate);
    curCCU_status.write(ccu_status_msg.buf);

    CAN.write(ccu_status_msg);
    
}
