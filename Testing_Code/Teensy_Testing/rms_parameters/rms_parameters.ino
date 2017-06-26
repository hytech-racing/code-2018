/*
 * Nathan Cheek
 * 2016-11-22
 * Read and write RMS parameters
 */

#include <FlexCAN.h>
#include <HyTech17.h>

FlexCAN CAN(500000);
static CAN_message_t msg_tx;
static CAN_message_t msg_rx;
String command_incoming;
bool command_finished = false;
bool wait_confirmation = false;

void setup() {
  Serial.begin(115200);
  CAN.begin();
  delay(100);
  Serial.println("CAN transceiver initialized");
  Serial.println("Welcome to RMS Parameters Interface. Please use either:");
  Serial.println("read <parameter address>");
  Serial.println("write <parameter address> <parameter value>");
}

void loop() {
  /*
   * Handle incoming messages
   */
  while (CAN.read(msg_rx)) {
    if (msg_rx.id == ID_MC_READ_WRITE_PARAMETER_RESPONSE) {
      MC_read_write_parameter_response message = MC_read_write_parameter_response(msg_rx.buf);
      Serial.print("Address: ");
      Serial.print(message.get_parameter_address());
      if (message.get_write_success()) {
        Serial.println(" - Success");
      } else {
        Serial.println(" - Not written");
      }
      Serial.print("Data: ");
      Serial.println(message.get_data(), DEC);
    }
  }

  /*
   * Get user input
   */
  if (Serial.available() > 0) {
    char newChar = (char) Serial.read();
    if (newChar != '\n') {
      command_incoming += newChar;
    } else {
      command_finished = true;      
    }
  }

  /*
   * Parse user input
   */
  if (command_finished) {
    String command = command_incoming;
    command_incoming = "";
    command_finished = false;
    
    if (wait_confirmation) { // Next command needs to be 'y' to otherwise message is discarded
      wait_confirmation = false;
      if (command == 'y' || command == 'Y') {
        CAN.write(msg_tx);
        Serial.println("Sent");
      } else {
        Serial.println("Not sent");
      }
      return;
    }
    
    if (command.substring(0,4) == "read") {
      command = command.substring(5);
      int address = command.toInt();
      if (address == 0) { // Failed to parse address
        print_help();
        return;
      }
      MC_read_write_parameter_command message = MC_read_write_parameter_command(address, 0, 0);
      message.write(msg_tx.buf);
      msg_tx.id = ID_MC_READ_WRITE_PARAMETER_COMMAND;
      msg_tx.len = sizeof(CAN_message_mc_read_write_parameter_command_t);
      Serial.print("Message to send: READ ");
      Serial.print(address);
      Serial.println(" [y/n]");
      wait_confirmation = true;
    } else if (command.substring(0,5) == "write") {
      command = command.substring(6);
      if (command.indexOf(' ') == -1) { // If data argument is not present
        print_help();
        return;
      }
      String addressStr = command.substring(0, command.indexOf(' '));
      int address = addressStr.toInt();
      if (address == 0) { // Failed to parse address
        print_help();
        return;
      }
      command = command.substring(command.indexOf(' ') + 1);
      int data = command.toInt();
      MC_read_write_parameter_command message = MC_read_write_parameter_command(address, 1, data);
      message.write(msg_tx.buf);
      msg_tx.id = ID_MC_READ_WRITE_PARAMETER_COMMAND;
      msg_tx.len = sizeof(CAN_message_mc_read_write_parameter_command_t);
      Serial.print("Message to send: WRITE ");
      Serial.print(address);
      Serial.print(" ");
      Serial.print(data);
      Serial.println(" [y/n]");
      wait_confirmation = true;
    } else {
      print_help();
      return;
    }
  }
}

void print_help() {
  Serial.println("Command not recognized. Please use either:");
  Serial.println("read <parameter address>");
  Serial.println("write <parameter address> <parameter value>");
}

