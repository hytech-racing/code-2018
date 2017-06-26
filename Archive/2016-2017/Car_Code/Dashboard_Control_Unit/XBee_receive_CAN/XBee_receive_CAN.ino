#define XB Serial2
#include <FlexCAN.h>
#include "HyTech17.h"

CAN_message_t msg;
unsigned long baud = 115200;
const int led_pin = 13;  // 13 = Teensy 3.X & LC
                         // 11 = Teensy 2.0
                         //  6 = Teensy++ 2.0
int c = 0;

void setup()
{
  pinMode(led_pin, OUTPUT);
  digitalWrite(led_pin, LOW);
  Serial.begin(baud);  // USB, communication to PC or Mac
  XB.begin(baud); // Xbee communication
// communication to hardware serial
}

byte buff[80];

void loop()
{
  int rd, wr; // how much we can read, write, and how much 'good' data was copied
 
  delay(30); // needed so far to not get zeros as data values
  
//  rd = Serial.available(); // check if any data has arrived on the USB virtual serial port
//  if (rd > 0) { 
//    
//    wr = XB.availableForWrite(); // check if we can send data to the Xbee
//    if (wr > 0) 
//    {
//      if (rd > wr) rd = wr;                             // compute how much data to move, the smallest
//      if (rd > 80) rd = 80;                             // of rd, wr and the buffer size, Xbee is MAX 100 bytes, so 80 is safe
//      n = Serial.readBytes((char *)buff, rd);           // read data from the USB port
//
//      // need to decide how MATLAB program will send commands about what information to capture. 
//      
//      // recieve a command from MATLAB
//      // decide how to decode it and so that the teensy on the car will understand
//      // such as send 0, then a msg id to stop sending that kind of msg
//      // or send 1 to add that msg id to the 'filter' aboard the car teensy
//
//      // also it could be a toggle, where if you send a msg id that it is already filtering for, it deletes it
//      // and if the msg id is not on the list it adds it
//      
//      XB.write(buff, n);                        // write it to the hardware serial port
//    }
//  }
    
  
//  int test = 0;
//  test = XB.read();
//  Serial.println(test);
  
  rd = XB.available(); // next, check if the Xbee has any CAN msgs to deliver
  if (rd > 0) {
//    wr = Serial.availableForWrite(); // make sure we can send to the computer
//    if (wr > 0) 
//    {
      if (rd > wr) rd = wr;                   // compute how much data to move, the smallest
      if (rd > 80) rd = 80;                   // of rd, wr and the buffer size
      //Serial.println((rd > 0) && (wr > 0));
      //Serial.print("   "); Serial.println(sizeof(msg));
      //for (int i = 0; i < sizeof(buff); i++) Serial.print(buff[i]);
      //Serial.println();
      
      XB.readBytes(buff,sizeof(msg));         // read msg as bytes from the Xbee
      //for (int i = 0; i < sizeof(buff); i++) Serial.print(buff[i]);
      //Serial.println();
      //Serial.println("----------------------------------------------------------------------------------");
      //memcpy(&msg,&buff[0],rd);             // msg is recreated from bytes
      memcpy(&msg,&buff,sizeof(msg));
      //Serial.print(msg.id, HEX);
      c = c + 1;


      // We are now going to figure out what type of msg we just received,
      // and then decode the message to extract and send the relavent data to the computer
      if (c%2 == 1) {
      msgDecode(msg);      
//      CAN_message_t msg2;
//      msg = msg2; 
      memset(buff,0,sizeof(buff));
      }
  }

  // check if the USB virtual serial wants a new baud rate
  if (Serial.baud() != baud) {
    baud = Serial.baud();
    if (baud == 57600) {
      XB.begin(58824); //Need this for jank reason
    } else {
      XB.begin(baud);
    }
  }
  
}

void msgDecode(CAN_message_t msg)
{
// ids that we do have being decoded currently:
      // ID_MC_TEMPERATURES_1
      // ID_MC_TEMPERATURES_3
      // ID_MC_MOTOR_POSITION_INFORMATION
      // ID_MC_CURRENT_INFORMATION
      // ID_MC_VOLTAGE_INFORMATION
      // ID_MC_INTERNAL_STATES
      // ID_MC_FAULT_CODES
      // ID_MC_TORQUE_TIMER_INFORMATION
      // ID_PCU_STATUS

      // Still need to add:
      // ID_MC_TEMPERATURES_2
      // ID_MC_ANALOG_INPUTS_VOLTAGES
      // ID_MC_DIGITAL_INPUT_STATUS 
      // ID_MC_FLUX_INFORMATION
      // ID_MC_INTERNAL_VOLTAGES
      // ID_MC_MODULATION_INDEX_FLUX_WEAKENING_OUTPUT_INFORMATION 
      // ID_MC_FIRMWARE_INFORMATION 
      // ID_MC_DIAGNOSTIC_DATA 
      // ID_MC_COMMAND_MESSAGE 
      // ID_MC_READ_WRITE_PARAMETER_COMMAND 
      // ID_MC_READ_WRITE_PARAMETER_RESPONSE 
      if (msg.id == ID_MC_TEMPERATURES_1) {
        Serial.print("ID_MC_TEMPERATURES_1 ");
        MC_temperatures_1 mc_temperatures_1 = MC_temperatures_1(msg.buf);
        //Serial.print("MODULE A TEMP: ");
        Serial.print(mc_temperatures_1.get_module_a_temperature());
        Serial.print(" ");
        //Serial.print("MODULE B TEMP: ");
        Serial.print(mc_temperatures_1.get_module_b_temperature());
        Serial.print(" ");
        //Serial.print("MODULE C TEMP: ");
        Serial.print(mc_temperatures_1.get_module_c_temperature());
        Serial.print(" ");
        //Serial.print("GATE DRIVER BOARD TEMP: ");
        Serial.print(mc_temperatures_1.get_gate_driver_board_temperature());
        Serial.println();
      }

      if (msg.id == ID_MC_TEMPERATURES_3) {
        Serial.print("ID_MC_TEMPERATURES_3 ");
        MC_temperatures_3 mc_temperatures_3 = MC_temperatures_3(msg.buf);
        //Serial.print("RTD 4 TEMP: ");
        Serial.print(mc_temperatures_3.get_rtd_4_temperature());
        Serial.print(" ");
        //Serial.print("RTD 5 TEMP: ");
        Serial.print(mc_temperatures_3.get_rtd_5_temperature());
        Serial.print(" ");
        //Serial.print("MOTOR TEMP: ");
        Serial.print(mc_temperatures_3.get_motor_temperature());
        Serial.print(" ");
        //Serial.print("TORQUE SHUDDER: ");
        //Serial.println(mc_temperatures_3.get_torque_shudder());
        Serial.println();
      }

      if (msg.id == ID_MC_MOTOR_POSITION_INFORMATION) {
        Serial.print("ID_MC_MOTOR_POSITION_INFORMATION ");
        MC_motor_position_information mc_motor_position_information = MC_motor_position_information(msg.buf);
        //Serial.print("MOTOR ANGLE: ");
        Serial.print(mc_motor_position_information.get_motor_angle());
        Serial.print(" ");
        //Serial.print("MOTOR SPEED: ");
        Serial.print(mc_motor_position_information.get_motor_speed());
        //Serial.print("ELEC OUTPUT FREQ: ");
        //Serial.println(mc_motor_position_information.get_electrical_output_frequency());
        //Serial.print("DELTA RESOLVER FILT: ");
        //Serial.println(mc_motor_position_information.get_delta_resolver_filtered());
        Serial.println();
      }

      if (msg.id == ID_MC_CURRENT_INFORMATION) {
        Serial.print("ID_MC_CURRENT_INFORMATION ");
        MC_current_information mc_current_information = MC_current_information(msg.buf);
        //Serial.print("PHASE A CURRENT: ");
        Serial.print(mc_current_information.get_phase_a_current());
        Serial.print(" ");
        //Serial.print("PHASE B CURRENT: ");
        Serial.print(mc_current_information.get_phase_b_current());
        Serial.print(" ");
        //Serial.print("PHASE C CURRENT: ");
        Serial.print(mc_current_information.get_phase_c_current());
        Serial.print(" ");
        //Serial.print("DC BUS CURRENT: ");
        Serial.print(mc_current_information.get_dc_bus_current());
        Serial.println();
      }

      if (msg.id == ID_MC_VOLTAGE_INFORMATION) {
        Serial.print("ID_MC_VOLTAGE_INFORMATION ");
        MC_voltage_information mc_voltage_information = MC_voltage_information(msg.buf);
        //Serial.print("DC BUS VOLTAGE: ");
        Serial.print(mc_voltage_information.get_dc_bus_voltage());
        Serial.print(" ");
        //Serial.print("OUTPUT VOLTAGE: ");
        Serial.print(mc_voltage_information.get_output_voltage());
        Serial.print(" ");
        //Serial.print("PHASE AB VOLTAGE: ");
        Serial.print(mc_voltage_information.get_phase_ab_voltage());
        Serial.print(" ");
        //Serial.print("PHASE BC VOLTAGE: ");
        Serial.print(mc_voltage_information.get_phase_bc_voltage());
        Serial.println();
      }

      if (msg.id == ID_MC_INTERNAL_STATES) {
        Serial.print("ID_MC_INTERNAL_STATES ");
        MC_internal_states mc_internal_states = MC_internal_states(msg.buf);
        //Serial.print("VSM STATE: ");
        Serial.print(mc_internal_states.get_vsm_state());
        Serial.print(" ");
        //Serial.print("INVERTER STATE: ");
        Serial.print(mc_internal_states.get_inverter_state());
        Serial.print(" ");

        // TODO relay states
        
        //Serial.print("INVERTER RUN MODE: ");
        Serial.print(mc_internal_states.get_inverter_run_mode());
        Serial.print(" ");
        //Serial.print("INVERTER ACTIVE DISCHARGE STATE: ");
        Serial.print(mc_internal_states.get_inverter_active_discharge_state());
        Serial.print(" ");
        //Serial.print("INVERTER COMMAND MODE: ");
        Serial.print(mc_internal_states.get_inverter_command_mode());
        Serial.print(" ");
        //Serial.print("INVERTER ENABLE: ");
        if (mc_internal_states.get_inverter_enable_state()) {
          Serial.print("1");
        } else {
          Serial.print("0");
        }
        Serial.print(" ");
        //Serial.print("INVERTER LOCKOUT: ");
        Serial.print(mc_internal_states.get_inverter_enable_lockout());
        Serial.print(" ");
        //Serial.print("DIRECTION COMMAND: ");
        Serial.print(mc_internal_states.get_direction_command());
        Serial.println();
      }

//      if (msg.id == ID_MC_FAULT_CODES) {
//        Serial.print("ID_MC_FAULT_CODES ");
//        MC_fault_codes mc_fault_codes = MC_fault_codes(msg.buf);
//        //Serial.print("POST FAULT LO: 0x");
//        Serial.print(mc_fault_codes.get_post_fault_lo(), HEX);
//        Serial.print(" ");
//        //Serial.print("POST FAULT HI: 0x");
//        Serial.print(mc_fault_codes.get_post_fault_hi(), HEX);
//        Serial.print(" ");
//        //Serial.print("RUN FAULT LO: 0x");
//        Serial.print(mc_fault_codes.get_run_fault_lo(), HEX);
//        Serial.print(" ");
//        //Serial.print("RUN FAULT HI: 0x");
//        Serial.print(mc_fault_codes.get_run_fault_hi(), HEX);
//        Serial.println();
//      }

      if (msg.id == ID_MC_TORQUE_TIMER_INFORMATION) {
        Serial.print("ID_MC_TORQUE_TIMER_INFORMATION ");
        MC_torque_timer_information mc_torque_timer_information = MC_torque_timer_information(msg.buf);
        //Serial.print("COMMANDED TORQUE: ");
        Serial.print(mc_torque_timer_information.get_commanded_torque());
        Serial.print(" ");
        //Serial.print("TORQUE FEEDBACK: ");
        Serial.print(mc_torque_timer_information.get_torque_feedback());
        Serial.print(" ");
        //Serial.print("RMS UPTIME: ");
        Serial.print(mc_torque_timer_information.get_power_on_timer());
        Serial.println();
      }

      
//      if (msg.id == ID_PCU_STATUS) {
//        Serial.print("ID_MC_PCU_STATUS ");
//        PCU_status pcu_status = PCU_status(msg.buf);
//        //Serial.print("PCU STATE: ");
//        Serial.print(pcu_status.get_state());
//        Serial.print(" ");
//        //Serial.print("BMS FAULT: ");
//        Serial.print(pcu_status.get_bms_fault());
//        Serial.print(" ");
//        //Serial.print("IMD FAULT: ");
//        Serial.print(pcu_status.get_imd_fault());
//        Serial.println();
//      }
}
