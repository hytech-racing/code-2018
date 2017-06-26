#ifndef __HYTECH17_H__
#define __HYTECH17_H__

#include <string.h>
#include <stdint.h>

/*
 * ECU state definitions
 */
#define PCU_STATE_WAITING_BMS_IMD 1
#define PCU_STATE_WAITING_DRIVER 2
#define PCU_STATE_LATCHING 3
#define PCU_STATE_SHUTDOWN_CIRCUIT_INITIALIZED 4
#define PCU_STATE_FATAL_FAULT 5

#define TCU_STATE_WAITING_SHUTDOWN_CIRCUIT_INITIALIZED 1
#define TCU_STATE_WAITING_TRACTIVE_SYSTEM 2
#define TCU_STATE_TRACTIVE_SYSTEM_NOT_ACTIVE 3
#define TCU_STATE_TRACTIVE_SYSTEM_ACTIVE 4
#define TCU_STATE_ENABLING_INVERTER 5
#define TCU_STATE_WAITING_READY_TO_DRIVE_SOUND 6
#define TCU_STATE_READY_TO_DRIVE 7

#define DCU_STATE_INITIAL_STARTUP 1
#define DCU_STATE_WAITING_TRACTIVE_SYSTEM 2
#define DCU_STATE_PRESSED_TRACTIVE_SYSTEM 3
#define DCU_STATE_WAITING_MC_ENABLE 4
#define DCU_STATE_PRESSED_MC_ENABLE 5
#define DCU_STATE_PLAYING_RTD 6
#define DCU_STATE_READY_TO_DRIVE 7
#define DCU_STATE_TS_INACTIVE 8
#define DCU_STATE_FATAL_FAULT 9

#define CHARGE_STATE_NOT_CHARGING 0
#define CHARGE_STATE_CHARGE_REQUESTED 1
#define CHARGE_STATE_CHARGING 2

/*
 * CAN ID definitions
 */
#define ID_PCU_STATUS 0xD0
#define ID_PCU_VOLTAGES 0xD1
#define ID_TCU_STATUS 0xD2
#define ID_TCU_READINGS 0xD3
#define ID_TCU_RESTART 0xD4
#define ID_BMS_VOLTAGE 0xD8
#define ID_BMS_CURRENT 0xD9
#define ID_BMS_TEMPERATURE 0xDA
#define ID_BMS_STATUS 0xDB
#define ID_DCU_STATUS 0xDC
#define ID_CHARGE_STATUS 0xDD
#define ID_MC_TEMPERATURES_1 0xA0
#define ID_MC_TEMPERATURES_2 0xA1
#define ID_MC_TEMPERATURES_3 0xA2
#define ID_MC_ANALOG_INPUTS_VOLTAGES 0xA3
#define ID_MC_DIGITAL_INPUT_STATUS 0xA4
#define ID_MC_MOTOR_POSITION_INFORMATION 0xA5
#define ID_MC_CURRENT_INFORMATION 0xA6
#define ID_MC_VOLTAGE_INFORMATION 0xA7
#define ID_MC_FLUX_INFORMATION 0xA8
#define ID_MC_INTERNAL_VOLTAGES 0xA9
#define ID_MC_INTERNAL_STATES 0xAA
#define ID_MC_FAULT_CODES 0xAB
#define ID_MC_TORQUE_TIMER_INFORMATION 0xAC
#define ID_MC_MODULATION_INDEX_FLUX_WEAKENING_OUTPUT_INFORMATION 0xAD
#define ID_MC_FIRMWARE_INFORMATION 0xAE
#define ID_MC_DIAGNOSTIC_DATA 0xAF
#define ID_MC_COMMAND_MESSAGE 0xC0
#define ID_MC_READ_WRITE_PARAMETER_COMMAND 0xC1
#define ID_MC_READ_WRITE_PARAMETER_RESPONSE 0xC2

/*

 * A GENERAL_NOTE: the load functions in these classes take a byte array containing raw CAN buffer data.
 * The data contained in this byte array is used to populate the struct.
 *
 * A GENERAL_NOTE: The write functions in these classes take a byte array that is meant to be populated
 * with the data contained in the object. The byte array can then be used as the raw CAN buffer.
 */

/*
 * CAN message structs and classes
 */
typedef struct CAN_message_pcu_status_t {
    uint8_t state;
    bool bms_fault;
    bool imd_fault;
    uint16_t okhs_value;
    uint16_t discharge_ok_value;
} CAN_msg_pcu_status;

class PCU_status {
  public:
    PCU_status();
    PCU_status(uint8_t buf[8]);
    PCU_status(uint8_t state, bool bms_fault, bool imd_fault, uint16_t okhs_value, uint16_t discharge_ok_value);
    void load(uint8_t buf[8]);
    void write(uint8_t buf[8]);
    uint8_t get_state();
    bool get_bms_fault();
    bool get_imd_fault();
    bool get_okhs_value();
    bool get_discharge_ok_value();
    void set_state(uint8_t state);
    void set_bms_fault(bool bms_fault);
    void set_imd_fault(bool imd_fault);
    void set_okhs_value(uint16_t okhs_value);
    void set_discharge_ok_value(uint16_t discharge_ok_value);
  private:
    CAN_message_pcu_status_t message;
};

typedef struct CAN_message_pcu_voltages_t {
    uint16_t GLV_battery_voltage;
    uint16_t shutdown_circuit_voltage;
} CAN_msg_pcu_voltages;

class PCU_voltages {
    public:
        PCU_voltages();
        PCU_voltages(uint8_t buf[8]);
        PCU_voltages(uint16_t GLV_battery_voltage, uint16_t shutdown_circuit_voltage);
        void load(uint8_t buf[8]);
        void write(uint8_t buf[8]);
        uint16_t get_GLV_battery_voltage();
        uint16_t get_shutdown_circuit_voltage();
        void set_GLV_battery_voltage(uint16_t GLV_battery_voltage);
        void set_shutdown_circuit_voltage(uint16_t shutdown_circuit_voltage);
    private:
        CAN_message_pcu_voltages_t message;
};

typedef struct CAN_message_tcu_status_t {
    bool throttle_implausibility;
    bool throttle_curve;
    bool brake_implausibility;
    bool brake_pedal_active;
    uint8_t state;
} CAN_msg_tcu_status;

class TCU_status {
  public:
    TCU_status();
    TCU_status(uint8_t buf[8]);
    TCU_status(bool throttle_implausibility, bool throttle_curve, bool brake_implausibility, bool brake_pedal_active, uint8_t state);
    void load(uint8_t buf[8]);
    void write(uint8_t buf[8]);
    bool get_throttle_implausibility();
    bool get_throttle_curve();
    bool get_brake_implausibility();
    bool get_brake_pedal_active();
    uint8_t get_state();
    void set_throttle_implausibility(bool throttle_implausibility);
    void set_throttle_curve(bool throttle_curve);
    void set_brake_implausibility(bool brake_implausibility);
    void set_brake_pedal_active(bool brake_pedal_active);
    void set_state(uint8_t new_state);
  private:
    CAN_message_tcu_status_t message;
};

typedef struct CAN_message_tcu_readings_t {
  uint16_t throttle_value_1;
  uint16_t throttle_value_2;
  uint16_t brake_value;
  uint16_t temperature;

} CAN_msg_tcu_readings;

class TCU_readings {
  public:
    TCU_readings();
    TCU_readings(uint8_t buf[8]);
    TCU_readings(uint16_t throttle_value_1, uint16_t throttle_value_2, uint16_t brake_value, uint16_t temperature);
    void load(uint8_t buf[8]);
    void write(uint8_t buf[8]);
    uint16_t get_throttle_value_1();
    uint16_t get_throttle_value_2();
    uint16_t get_throttle_value_avg();
    uint16_t get_brake_value();
    uint16_t get_temperature();
    void set_throttle_value_1(uint16_t throttle_value_1);
    void set_throttle_value_2(uint16_t throttle_value_2);
    void set_brake_value(uint16_t brake_value);
    void set_temperature(uint16_t temperature);
  private:
    CAN_message_tcu_readings_t message;
};

typedef struct CAN_message_dcu_status_t {
    uint8_t btn_press_id;
    uint8_t light_active_1;
    uint8_t light_active_2;
    uint8_t rtds_state; // ready to drive sound state
} CAN_message_dcu_status;

class DCU_status {
public:
    DCU_status();
    DCU_status(uint8_t buf[8]);
    DCU_status(uint8_t btn_press_id, uint8_t light_active_1, uint8_t light_active_2, uint8_t rtds_state);
    void load(uint8_t buf[8]);
    void write(uint8_t buf[8]);
    uint8_t get_btn_press_id();
    uint8_t get_light_active_1();
    uint8_t get_light_active_2();
    uint8_t get_rtds_state();
    void set_btn_press_id(uint8_t btn_press_id);
    void set_light_active_1(uint8_t light_active_1);
    void set_light_active_2(uint8_t light_active_2);
    void set_rtds_state(uint8_t rtds_state);
private:
    CAN_message_dcu_status_t message;
};

typedef struct CAN_message_bms_voltage_t {
    // integer from 0 to 65536 mapping from 0 to 5v
    uint16_t avgVoltage;
    uint16_t lowVoltage;
    uint16_t highVoltage;
    uint16_t totalVoltage;
} CAN_message_bms_voltage_t;

class BMS_voltages {
  public:
    BMS_voltages();
    BMS_voltages(uint8_t buf[]);
    BMS_voltages(uint16_t avg, uint16_t low, uint16_t high, uint16_t total);
    void load(uint8_t buf[]);
    void write(uint8_t buf[]);
    uint16_t getAverage();
    uint16_t getLow();
    uint16_t getHigh();
    uint16_t getTotal();
    void setAverage(uint16_t avg);
    void setLow(uint16_t low);
    void setHigh(uint16_t high);
    void setTotal(uint16_t total);
  private:
    CAN_message_bms_voltage_t bmsVoltageMessage;
};

enum CHARGING_STATE {
    DISCHARGING = 0,
    CHARGING = 1,
    UNKNOWN = 2
};

typedef struct CAN_message_bms_current_t {
    float current;
    uint8_t chargeState;
} CAN_message_bms_current_t;

class BMS_currents {
  public:
    BMS_currents();
    BMS_currents(uint8_t buf[]);
    BMS_currents(float _current, uint8_t state);
    void load(uint8_t buf[]);
    void write(uint8_t buf[]);
    float getCurrent();
    uint8_t getChargingState();
    void setCurrent(float _current);
    void setChargingState(uint8_t state);
  private:
    CAN_message_bms_current_t bmsCurrentMessage;
};

typedef struct CAN_message_bms_temperature_t {
    uint16_t avgTemp;
    uint16_t lowTemp;
    uint16_t highTemp;
} CAN_message_bms_temperature_t;

class BMS_temperatures {
  public:
    BMS_temperatures();
    BMS_temperatures(uint8_t buf[]);
    BMS_temperatures(uint16_t avg, uint16_t low, uint16_t high);
    void load(uint8_t buf[]);
    void write(uint8_t buf[]);
    uint16_t getAvgTemp();
    uint16_t getLowTemp();
    uint16_t getHighTemp();
    void setAvgTemp(uint16_t avg);
    void setLowTemp(uint16_t low);
    void setHighTemp(uint16_t high);
  private:
    CAN_message_bms_temperature_t bmsTemperatureMessage;
};

typedef struct CAN_message_bms_error_t {
    // TODO: Implement BMS Error message
    /* Error Flags as bit map:
     * errorFlagsByte1
     * -0- -1- -2- -3- -4- -5- -6- -7-
     * 0. Overvoltage
     * 1. Undervoltage
     * 2. Total Voltage
     * 3.
     * 4. Discharge overcurrent
     * 5. Discharge undercurrent
     * 6. Charge overcurrent
     * 7. Charge undercurrent
     */
    /*
     * errorFlagsByte2
     * -0- -1- -2- -3- -4- -5- -6- -7-
     * 0. Discharge overtemp
     * 1. Discahrge undertemp
     * 2. Charge overtemp
     * 3. Charge undertemp
     */
     uint8_t errorFlagsByte1;
     uint8_t errorFlagsByte2;
     uint8_t BMSStatusOK;
} CAN_message_bms_error_t;

class BMS_status {
  public:
    BMS_status();
    BMS_status(uint8_t buf[]);
    void load(uint8_t buf[]);
    void write(uint8_t buf[]);
    /***************GETTERS*****************/
    bool getOvervoltage();
    bool getUndervoltage();
    bool getTotalVoltageHigh();

    bool getDischargeOvercurrent();
    // bool getDischargeUndercurrent();
    bool getChargeOvercurrent();
    // bool getChargeUndercurrent();

    bool getDischargeOvertemp();
    // bool getDischargeUndertemp();
    bool getChargeOvertemp();
    // bool getChargeUndertemp();

    bool getBMSStatusOK();

    /***************SETTERS*****************/
    void setOvervoltage(bool flag);
    void setUndervoltage(bool flag);
    void setTotalVoltageHigh(bool flag);

    void setDischargeOvercurrent(bool flag);
    // void setDischargeUndercurrent(bool flag);
    void setChargeOvercurrent(bool flag);
    // void setChargeUndercurrent(bool flag);

    void setDischargeOvertemp(bool flag);
    // void setDischargeUndertemp(bool flag);
    void setChargeOvertemp(bool flag);
    // void setChargeUndertemp(bool flag);

    void clearAllFlags();

    void setBMSStatusOK(bool flag);
  private:
    CAN_message_bms_error_t bmsErrorMessage;
};

typedef struct CAN_message_charge_status_t {
    uint8_t charge_command;
} CAN_message_charge_status_t;

class Charge_status {
    public:
        Charge_status();
        Charge_status(uint8_t buf[]);
        void load(uint8_t buf[]);
        void write(uint8_t buf[]);
        uint8_t getChargeCommand();
        void setChargeCommand(uint8_t cmd);
    private:
        CAN_message_charge_status_t message;
};

typedef struct CAN_message_mc_temperatures_1_t {
  int16_t module_a_temperature;
  int16_t module_b_temperature;
  int16_t module_c_temperature;
  int16_t gate_driver_board_temperature;
} CAN_message_mc_temperatures_1_t;

class MC_temperatures_1 {
  public:
    MC_temperatures_1();
    MC_temperatures_1(uint8_t buf[8]);
    void load(uint8_t buf[8]);
    int16_t get_module_a_temperature();
    int16_t get_module_b_temperature();
    int16_t get_module_c_temperature();
    int16_t get_gate_driver_board_temperature();
  private:
    CAN_message_mc_temperatures_1_t message;
};

typedef struct CAN_message_mc_temperatures_2_t {
  int16_t control_board_temperature;
  int16_t rtd_1_temperature;
  int16_t rtd_2_temperature;
  int16_t rtd_3_temperature;
} CAN_message_mc_temperatures_2_t;

class MC_temperatures_2 {
  public:
    MC_temperatures_2();
    MC_temperatures_2(uint8_t buf[8]);
    void load(uint8_t buf[8]);
    int16_t get_control_board_temperature();
    int16_t get_rtd_1_temperature();
    int16_t get_rtd_2_temperature();
    int16_t get_rtd_3_temperature();
  private:
    CAN_message_mc_temperatures_2_t message;
};

typedef struct CAN_message_mc_temperatures_3_t {
  int16_t rtd_4_temperature;
  int16_t rtd_5_temperature;
  int16_t motor_temperature;
  int16_t torque_shudder;
} CAN_message_mc_temperatures_3_t;

class MC_temperatures_3 {
  public:
    MC_temperatures_3();
    MC_temperatures_3(uint8_t buf[8]);
    void load(uint8_t buf[8]);
    int16_t get_rtd_4_temperature();
    int16_t get_rtd_5_temperature();
    int16_t get_motor_temperature();
    int16_t get_torque_shudder();
  private:
    CAN_message_mc_temperatures_3_t message;
};

typedef struct CAN_message_mc_analog_input_voltages_t {
  int16_t analog_input_1;
  int16_t analog_input_2;
  int16_t analog_input_3;
  int16_t analog_input_4;
} CAN_message_mc_analog_input_voltages_t;

class MC_analog_input_voltages {
  public:
    MC_analog_input_voltages();
    MC_analog_input_voltages(uint8_t buf[8]);
    void load(uint8_t buf[8]);
    int16_t get_analog_input_1();
    int16_t get_analog_input_2();
    int16_t get_analog_input_3();
    int16_t get_analog_input_4();
  private:
    CAN_message_mc_analog_input_voltages_t message;
};

typedef struct CAN_message_mc_digital_input_status_t {
  bool digital_input_1;
  bool digital_input_2;
  bool digital_input_3;
  bool digital_input_4;
  bool digital_input_5;
  bool digital_input_6;
  bool digital_input_7;
  bool digital_input_8;
} CAN_message_mc_digital_input_status_t;

class MC_digital_input_status {
  public:
    MC_digital_input_status();
    MC_digital_input_status(uint8_t buf[]);
    void load(uint8_t buf[]);
    void loadByteIntoBooleanStruct(int* index, uint8_t buf[], bool* structVariable);
    bool digital_input_1();
    bool digital_input_2();
    bool digital_input_3();
    bool digital_input_4();
    bool digital_input_5();
    bool digital_input_6();
    bool digital_input_7();
    bool digital_input_8();
  private:
    CAN_message_mc_digital_input_status_t message;
};

typedef struct CAN_message_mc_motor_position_information_t {
  int16_t motor_angle;
  int16_t motor_speed;
  int16_t electrical_output_frequency;
  int16_t delta_resolver_filtered;
} CAN_message_mc_motor_position_information_t;

class MC_motor_position_information {
  public:
    MC_motor_position_information();
    MC_motor_position_information(uint8_t buf[8]);
    void load(uint8_t buf[8]);
    int16_t get_motor_angle();
    int16_t get_motor_speed();
    int16_t get_electrical_output_frequency();
    int16_t get_delta_resolver_filtered();
  private:
    CAN_message_mc_motor_position_information_t message;
};

typedef struct CAN_message_mc_current_information_t {
  int16_t phase_a_current;
  int16_t phase_b_current;
  int16_t phase_c_current;
  int16_t dc_bus_current;
} CAN_message_mc_current_information_t;

class MC_current_information {
  public:
    MC_current_information();
    MC_current_information(uint8_t buf[8]);
    void load(uint8_t buf[8]);
    int16_t get_phase_a_current();
    int16_t get_phase_b_current();
    int16_t get_phase_c_current();
    int16_t get_dc_bus_current();
  private:
    CAN_message_mc_current_information_t message;
};

typedef struct CAN_message_mc_voltage_information_t {
  int16_t dc_bus_voltage;
  int16_t output_voltage;
  int16_t phase_ab_voltage;
  int16_t phase_bc_voltage;
} CAN_message_mc_voltage_information_t;

class MC_voltage_information {
  public:
    MC_voltage_information();
    MC_voltage_information(uint8_t buf[8]);
    void load(uint8_t buf[8]);
    int16_t get_dc_bus_voltage();
    int16_t get_output_voltage();
    int16_t get_phase_ab_voltage();
    int16_t get_phase_bc_voltage();
  private:
    CAN_message_mc_voltage_information_t message;
};

typedef struct CAN_message_mc_internal_states_t {
  uint16_t vsm_state;
  uint8_t inverter_state;
  uint8_t relay_state;
  uint8_t inverter_run_mode_discharge_state;
  uint8_t inverter_command_mode;
  uint8_t inverter_enable;
  uint8_t direction_command;
} CAN_message_mc_internal_states_t;

class MC_internal_states {
  public:
    MC_internal_states();
    MC_internal_states(uint8_t buf[8]);
    void load(uint8_t buf[8]);
    uint8_t get_vsm_state();
    uint8_t get_inverter_state();
    bool get_relay_active_1();
    bool get_relay_active_2();
    bool get_relay_active_3();
    bool get_relay_active_4();
    bool get_relay_active_5();
    bool get_relay_active_6();
    bool get_inverter_run_mode();
    uint8_t get_inverter_active_discharge_state();
    bool get_inverter_command_mode();
    bool get_inverter_enable_state();
    bool get_inverter_enable_lockout();
    bool get_direction_command();
  private:
    CAN_message_mc_internal_states_t message;
};

typedef struct CAN_message_mc_fault_codes_t {
  uint16_t post_fault_lo;
  uint16_t post_fault_hi;
  uint16_t run_fault_lo;
  uint16_t run_fault_hi;
} CAN_message_mc_fault_codes_t;

class MC_fault_codes {
  public:
    MC_fault_codes();
    MC_fault_codes(uint8_t buf[8]);
    void load(uint8_t buf[8]);
    uint16_t get_post_fault_lo();
    uint16_t get_post_fault_hi();
    uint16_t get_run_fault_lo();
    uint16_t get_run_fault_hi();
    bool get_post_lo_hw_gate_desaturation_fault();
    bool get_post_lo_hw_overcurrent_fault();
    bool get_post_lo_accelerator_shorted();
    bool get_post_lo_accelerator_open();
    bool get_post_lo_current_sensor_low();
    bool get_post_lo_current_sensor_high();
    bool get_post_lo_module_temperature_low();
    bool get_post_lo_module_temperature_high();
    bool get_post_lo_ctrl_pcb_temperature_low();
    bool get_post_lo_ctrl_pcb_temperature_high();
    bool get_post_lo_gate_drive_pcb_temperature_low();
    bool get_post_lo_gate_drive_pcb_temperature_high();
    bool get_post_lo_5v_sense_voltage_low();
    bool get_post_lo_5v_sense_voltage_high();
    bool get_post_lo_12v_sense_voltage_low();
    bool get_post_lo_12v_sense_voltage_high();
    bool get_post_hi_25v_sense_voltage_low();
    bool get_post_hi_25v_sense_voltage_high();
    bool get_post_hi_15v_sense_voltage_low();
    bool get_post_hi_15v_sense_voltage_high();
    bool get_post_hi_dc_bus_voltage_high();
    bool get_post_hi_dc_bus_voltage_low();
    bool get_post_hi_precharge_timeout();
    bool get_post_hi_precharge_voltage_failure();
    bool get_post_hi_eeprom_checksum_invalid();
    bool get_post_hi_eeprom_data_out_of_range();
    bool get_post_hi_eeprom_update_required();
    bool get_post_hi_reserved1(); // TODO delete these?
    bool get_post_hi_reserved2();
    bool get_post_hi_reserved3();
    bool get_post_hi_brake_shorted();
    bool get_post_hi_brake_open();
    bool get_run_lo_motor_overspeed_fault();
    bool get_run_lo_overcurrent_fault();
    bool get_run_lo_overvoltage_fault();
    bool get_run_lo_inverter_overtemperature_fault();
    bool get_run_lo_accelerator_input_shorted_fault();
    bool get_run_lo_accelerator_input_open_fault();
    bool get_run_lo_direction_command_fault();
    bool get_run_lo_inverter_response_timeout_fault();
    bool get_run_lo_hardware_gatedesaturation_fault();
    bool get_run_lo_hardware_overcurrent_fault();
    bool get_run_lo_undervoltage_fault();
    bool get_run_lo_can_command_message_lost_fault();
    bool get_run_lo_motor_overtemperature_fault();
    bool get_run_lo_reserved1(); // TODO delete these?
    bool get_run_lo_reserved2();
    bool get_run_lo_reserved3();
    bool get_run_hi_brake_input_shorted_fault();
    bool get_run_hi_brake_input_open_fault();
    bool get_run_hi_module_a_overtemperature_fault();
    bool get_run_hi_module_b_overtemperature_fault();
    bool get_run_hi_module_c_overtemperature_fault();
    bool get_run_hi_pcb_overtemperature_fault();
    bool get_run_hi_gate_drive_board_1_overtemperature_fault();
    bool get_run_hi_gate_drive_board_2_overtemperature_fault();
    bool get_run_hi_gate_drive_board_3_overtemperature_fault();
    bool get_run_hi_current_sensor_fault();
    bool get_run_hi_reserved1(); // TODO delete these?
    bool get_run_hi_reserved2();
    bool get_run_hi_reserved3();
    bool get_run_hi_reserved4();
    bool get_run_hi_resolver_not_connected();
    bool get_run_hi_inverter_discharge_active();
  private:
    CAN_message_mc_fault_codes_t message;
};

typedef struct CAN_message_mc_torque_timer_information_t {
  int16_t commanded_torque;
  int16_t torque_feedback;
  uint32_t power_on_timer;
} CAN_message_mc_torque_timer_information_t;

class MC_torque_timer_information {
  public:
    MC_torque_timer_information();
    MC_torque_timer_information(uint8_t buf[8]);
    void load(uint8_t buf[8]);
    int16_t get_commanded_torque();
    int16_t get_torque_feedback();
    uint32_t get_power_on_timer();
  private:
    CAN_message_mc_torque_timer_information_t message;
};

typedef struct CAN_message_mc_modulation_index_flux_weakening_output_information_t {
  uint16_t modulation_index; // TODO Signed or Unsigned?
  int16_t flux_weakening_output;
  int16_t id_command;
  int16_t iq_command;
} CAN_message_mc_modulation_index_flux_weakening_output_information_t;

class MC_modulation_index_flux_weakening_output_information {
  public:
    MC_modulation_index_flux_weakening_output_information();
    MC_modulation_index_flux_weakening_output_information(uint8_t buf[8]);
    void load(uint8_t buf[8]);
    uint16_t get_modulation_index();
    int16_t get_flux_weakening_output();
    int16_t get_id_command();
    int16_t get_iq_command();
  private:
    CAN_message_mc_modulation_index_flux_weakening_output_information_t message;
};

typedef struct CAN_message_mc_firmware_information_t {
  uint16_t eeprom_version_project_code;
  uint16_t software_version;
  uint16_t date_code_mmdd;
  uint16_t date_code_yyyy;
} CAN_message_mc_firmware_information_t;

class MC_firmware_information {
  public:
    MC_firmware_information();
    MC_firmware_information(uint8_t buf[8]);
    void load(uint8_t buf[8]);
    uint16_t get_eeprom_version_project_code();
    uint16_t get_software_version();
    uint16_t get_date_code_mmdd();
    uint16_t get_date_code_yyyy();
  private:
    CAN_message_mc_firmware_information_t message;
};

typedef struct CAN_message_mc_command_message_t {
  int16_t torque_command;
  int16_t angular_velocity;
  bool direction;
  uint8_t inverter_enable_discharge_enable;
  int16_t commanded_torque_limit;
} CAN_message_mc_command_message_t;

class MC_command_message {
  public:
    MC_command_message();
    MC_command_message(uint8_t buf[8]);
    MC_command_message(int16_t torque_command, int16_t angular_velocity, bool direction, bool inverter_enable, bool discharge_enable, int16_t commanded_torque_limit);
    void load(uint8_t buf[8]);
    void write(uint8_t buf[8]);
    int16_t get_torque_command();
    int16_t get_angular_velocity();
    bool get_direction();
    bool get_inverter_enable();
    bool get_discharge_enable();
    int16_t get_commanded_torque_limit();
    void set_torque_command(int16_t torque_command);
    void set_angular_velocity(int16_t angular_velocity);
    void set_direction(bool direction);
    void set_inverter_enable(bool inverter_enable);
    void set_discharge_enable(bool discharge_enable);
    void set_commanded_torque_limit(int16_t commanded_torque_limit);
  private:
    CAN_message_mc_command_message_t message;
};

typedef struct CAN_message_mc_read_write_parameter_command_t {
  uint16_t parameter_address;
  bool rw_command;
  uint8_t reserved1;
  uint8_t data[4];
} CAN_message_mc_read_write_parameter_command_t;

class MC_read_write_parameter_command {
  public:
    MC_read_write_parameter_command();
    MC_read_write_parameter_command(uint8_t buf[8]);
    MC_read_write_parameter_command(uint16_t parameter_address, bool rw_command, uint8_t data[]);
    void load(uint8_t buf[8]);
    void write(uint8_t buf[8]);
    uint16_t get_parameter_address();
    bool get_rw_command();
    uint8_t* get_data();
    void set_parameter_address(uint16_t parameter_address);
    void set_rw_command(bool rw_command);
    void set_data(uint8_t data[]);
  private:
    CAN_message_mc_read_write_parameter_command_t message;
};

typedef struct CAN_message_mc_read_write_parameter_response_t {
  uint16_t parameter_address;
  bool write_success;
  uint8_t reserved1;
  uint8_t data[4];
} CAN_message_mc_read_write_parameter_response_t;

class MC_read_write_parameter_response {
  public:
    MC_read_write_parameter_response();
    MC_read_write_parameter_response(uint8_t buf[8]);
    void load(uint8_t buf[8]);
    uint16_t get_parameter_address();
    bool get_write_success();
    uint8_t* get_data();
  private:
    CAN_message_mc_read_write_parameter_response_t message;
};

typedef struct CAN_message_ccu_status_t {
    uint16_t flow_rate;
} CAN_message_ccu_status_t;

class CCU_status {
  public:
    CCU_status();
    CCU_status(uint8_t buf[8]);
    void load(uint8_t buf[8]);
    uint16_t get_flow_rate();
    void set_flow_rate(uint16_t flow);
    void write(uint8_t buf[]);
  private:
    CAN_message_ccu_status_t message;
};


#endif
