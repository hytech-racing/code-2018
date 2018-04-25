/*
 * HyTech 2018 BMS Control Unit
 * Init 2017-04-11
 * Configured for HV Board Rev 2
 * Monitors cell voltages and temperatures, sends BMS_OK signal to close Shutdown Circuit
 */

/*
 * Shutdown circuit notes:
 * 1. GLV control system latches shutdown circuit closed.
 * 2. AIR's close.
 * 3. High voltage is available to the motor controller, TSAL is lit.
 * 4. Any faults (IMD OKHS, BMS_OK, BSPD) will open shutdown circuit, opening AIR's.
 */

/*
 * Operation notes:
 * 1. BMS sensors can be powered at all times.
 * 2. Once Teensy gets power from external power lines, give BMS_OK signal.
 * 3. No need to check DC bus voltage, because all batteries read their true voltages at all times. (They are continuous with each other at all times, due to no relay.)
 * 4. Once temperatures go too high, current goes too high, or cell voltages go too high or too low, drive the BMS_OK signal low.
 */

/*
 * LTC6804 state / communication notes:
 * The operation of the LTC6804 is divided into two separate sections: the core circuit and the isoSPI circuit. Both sections have an independent set of operating states, as well as a shutdown timeout. See LTC6804 Datasheet Page 20.
 * When sending an ADC conversion or diagnostic command, wake up the core circuit by calling wakeup_sleep()
 * When sending any other command (such as reading or writing registers), wake up the isoSPI circuit by calling wakeup_idle().
 */

#include <Arduino.h>
#include <FlexCAN.h>
#include "HyTech17.h"
#include "LT_SPI.h"
#include "LTC68041.h"
#include <Metro.h>

/*
 * Pin definitions
 */
#define BMS_OK A8
#define CURRENT_SENSE A3
#define LED_STATUS 7
#define LTC6820_CS 10
#define TEMP_SENSE_1 A9
#define TEMP_SENSE_2 A2
#define WATCHDOG A0

/*
 * Constant definitions
 */
#define TOTAL_IC 4
#define TOTAL_CELLS 9
#define TOTAL_THERMISTORS 3
#define TOTAL_SEGMENTS 2
#define THERMISTOR_RESISTOR_VALUE 10000

/*
 * Timers
 */
Metro timer_can_update = Metro(100);
Metro timer_debug = Metro(500);
Metro timer_process_cells = Metro(1000);
Metro timer_watchdog_timer = Metro(250);

/*
 * Global variables
 */
short voltage_cutoff_low = 2980;
short voltage_cutoff_high = 4200;
short total_voltage_cutoff = 150;
short discharge_current_constant_high = 22000; // 220.00A
short charge_current_constant_high = -10000; // 100.00A // TODO take into account max charge allowed for regen, 100A is NOT NOMINALLY ALLOWED!
short charge_temp_critical_high = 4400; // 44.00C
short discharge_temp_critical_high = 6000; // 60.00C
short temp_critical_low = 0; // 0C
short voltage_difference_threshold = 500; // 5.00V

bool first_fault_overvoltage = false; // Wait for 2 voltage faults in a row before shutting off BMS
bool first_fault_undervoltage = false;
bool first_fault_total_voltage_high = false;

uint16_t cell_voltages[TOTAL_IC][12]; // contains 12 battery cell voltages. Numbers are stored in 0.1 mV units.
uint16_t aux_voltages[TOTAL_IC][6]; // contains auxiliary pin voltages.
     /* Data contained in this array is in this format:
      * Thermistor 1
      * Thermistor 2
      * Thermistor 3
      */
int16_t cell_delta_voltage[TOTAL_IC][TOTAL_CELLS]; // keep track of which cells are being discharged
int16_t ignore_cell[TOTAL_IC][TOTAL_CELLS]; //cells to be ignored for Balance testing

/*!<
  The tx_cfg[][6] store the LTC6804 configuration data that is going to be written
  to the LTC6804 ICs on the daisy chain. The LTC6804 configuration data that will be
  written should be stored in blocks of 6 bytes. The array should have the following format:

 |  tx_cfg[0][0]| tx_cfg[0][1] |  tx_cfg[0][2]|  tx_cfg[0][3]|  tx_cfg[0][4]|  tx_cfg[0][5]| tx_cfg[1][0] |  tx_cfg[1][1]|  tx_cfg[1][2]|  .....    |
 |--------------|--------------|--------------|--------------|--------------|--------------|--------------|--------------|--------------|-----------|
 |IC1 CFGR0     |IC1 CFGR1     |IC1 CFGR2     |IC1 CFGR3     |IC1 CFGR4     |IC1 CFGR5     |IC2 CFGR0     |IC2 CFGR1     | IC2 CFGR2    |  .....    |

*/
uint8_t tx_cfg[TOTAL_IC][6]; // data defining how data will be written to daisy chain ICs.

/**
 * CAN Variables
 */
FlexCAN CAN(500000);
static CAN_message_t msg;

/**
 * BMS State Variables
 */
//BMS_detailed_temperatures bms_detailed_temperatures[TOTAL_SEGMENTS]; // TODO write individual temperatures here for CAN bus
BMS_detailed_voltages bms_detailed_voltages[TOTAL_SEGMENTS][3];
BMS_status bms_status;
BMS_temperatures bms_temperatures;
BMS_voltages bms_voltages;
bool watchdog_high = true;

int minVoltageICIndex;
int minVoltageCellIndex;
int voltage_difference;

bool cell_discharging[TOTAL_IC][12];

void setup() {
    pinMode(BMS_OK, OUTPUT);
    pinMode(CURRENT_SENSE, INPUT);
    pinMode(LED_STATUS, OUTPUT);
    pinMode(LTC6820_CS,OUTPUT);
    pinMode(TEMP_SENSE_1,INPUT);
    pinMode(TEMP_SENSE_2,INPUT);
    pinMode(WATCHDOG, OUTPUT);

    Serial.begin(115200); // Init serial for PC communication
    CAN.begin(); // Init CAN for vehicle communication
    delay(100);
    Serial.println("CAN system and serial communication initialized");

    digitalWrite(BMS_OK, HIGH);
    digitalWrite(WATCHDOG, watchdog_high);

    initialize(); // Call our modified initialize function instead of the default Linear function
    init_cfg(); // Initialize and write configuration registers to LTC6804 chips
    poll_cell_voltage();
    memcpy(cell_delta_voltage, cell_voltages, 2 * TOTAL_IC * TOTAL_CELLS);
    bms_status.set_state(BMS_STATE_CHARGE_ENABLED);
    Serial.println("Setup Complete!");

    // DEBUG Code for testing cell packs
    /*ignore_cell[0][3] = true;
    ignore_cell[0][4] = true;
    ignore_cell[0][5] = true;
    ignore_cell[0][6] = true;*/
}

// TODO Implement Coulomb counting to track state of charge of battery.

/*
 * Main BMS Control Loop
 */
void loop() {
    while (CAN.read(msg)) {
        if (msg.id == ID_BMS_TEMPERATURES) { // Used temporarily while we have an external temperature monitor ECU
            bms_temperatures.load(msg.buf);
            //bms_status.set_discharge_overtemp(false);  // RESET these values, then check below if they should be set again
            //bms_status.set_charge_overtemp(false);
            //bms_status.set_undertemp(false);
            if (bms_status.get_state() == BMS_STATE_DISCHARGING && bms_temperatures.get_high_temperature() > discharge_temp_critical_high) {
                bms_status.set_discharge_overtemp(true);
                Serial.println("Discharge overtemperature fault!");
            } else if (bms_status.get_state() >= BMS_STATE_CHARGE_ENABLED && bms_temperatures.get_high_temperature() > charge_temp_critical_high) {
                bms_status.set_charge_overtemp(true);
                Serial.println("Charge overtemperature fault!");
            } else if (bms_temperatures.get_low_temperature() < temp_critical_low) {
                bms_status.set_undertemp(true);
                Serial.println("Undertemperature fault!");
            }
        }
        if (msg.id == ID_CCU_STATUS) { // TODO - currently the BMS doesn't actually need to know the CCU status, could be a future feature
            CCU_status ccu_status = CCU_status(msg.buf);
            if (ccu_status.get_charger_enabled()) {
                Serial.println("Charger enabled");
            } else {
                Serial.println("Charger NOT enabled");
            }
        }
    }

    if (timer_process_cells.check()) {
        // poll_cell_voltage(); No need to print this twice
        process_voltages(); // polls controller, and store data in bms_voltages object.
        //bms_voltages.set_low(37408); // DEBUG Remove before final code
        balance_cells();
        //process_temps(); // store data in bms_temperatures object.
        process_current(); // store data in bms_status object.
        print_uptime();
        print_temperatures();

        if (bms_status.get_error_flags()) { // BMS error - drive BMS_OK signal low
            Serial.println("STATUS NOT GOOD!!!!!!!!!!!!!!!");
            Serial.print("Error code: 0x");
            Serial.println(bms_status.get_error_flags(), HEX);
            digitalWrite(BMS_OK, LOW);
        }
    }

    /*
    uint8_t data[1][8];
    uint8_t cmd[2] = {0x00,0x12};
    int res = LTC6804_rdcfg(0,data);
    for (int i = 0;i<8;i++){
      Serial.print("register: ");
      Serial.print(i);
      Serial.print(" value: ");
      Serial.println(data[0][i]);

       Serial.print("PEC:");
       Serial.println(res);
    }
    delay(2000);
    */

    if (timer_can_update.check()) {
        bms_status.write(msg.buf);
        msg.id = ID_BMS_STATUS;
        msg.len = sizeof(CAN_message_bms_status_t);
        CAN.write(msg);

        bms_voltages.write(msg.buf);
        msg.id = ID_BMS_VOLTAGES;
        msg.len = sizeof(CAN_message_bms_voltages_t);
        CAN.write(msg);

        msg.id = ID_BMS_DETAILED_VOLTAGES;
        msg.len = sizeof(CAN_message_bms_detailed_voltages_t);
        for (int i = 0; i < TOTAL_SEGMENTS; i++) {
            for (int j = 0; j < 3; j++) {
                bms_detailed_voltages[i][j].write(msg.buf);
                CAN.write(msg);
            }
        }

        /*bms_temperatures.write(msg.buf);
        msg.id = ID_BMS_TEMPERATURES;
        msg.len = sizeof(CAN_message_bms_temperatures_t);
        CAN.write(msg);*/
    }

    if (timer_watchdog_timer.check()) { // Send alternating keepalive signal to watchdog timer
        watchdog_high = !watchdog_high;
        digitalWrite(WATCHDOG, watchdog_high);
    }
}

/*
 * Initialize communication with LTC6804 chips. Based off of LTC6804_initialize()
 * Changes: Doesn't call quikeval_SPI_connect(), Sets ADC mode to MD_FILTERED
 */
void initialize() {
    spi_enable(SPI_CLOCK_DIV16); // Sets 1MHz Clock
    set_adc(MD_FILTERED,DCP_DISABLED,CELL_CH_ALL,AUX_CH_ALL); // Sets global variables used in ADC commands // TODO Change CELL_CH_ALL and AUX_CH_ALL so we don't read all GPIOs and cells
}

/*
 * Initialize the configuration array and write configuration to ICs
 * See LTC6804 Datasheet Page 51 for tables of register definitions
 */
void init_cfg() {
    for (int i = 0; i < TOTAL_IC; i++) {
        tx_cfg[i][0] = 0xFE; // 11111110 - All GPIOs enabled, Reference Remains Powered Up Until Watchdog Timeout, ADCOPT 0 allows us to use filtered ADC mode // TODO maybe we can speed things up by disabling some GPIOs
        tx_cfg[i][1] = 0x00;
        tx_cfg[i][2] = 0x00;
        tx_cfg[i][3] = 0x00;
        tx_cfg[i][4] = 0x00;
        tx_cfg[i][5] = 0x00;
    }
    cfg_set_overvoltage_comparison_voltage(voltage_cutoff_high * 10); // Calculate overvoltage comparison register values
    cfg_set_undervoltage_comparison_voltage(voltage_cutoff_low * 10); // Calculate undervoltage comparison register values
    wakeup_idle(); // Wake up isoSPI
    delayMicroseconds(1200); // Wait 4*t_wake for wakeup command to propogate and all 4 chips to wake up - See LTC6804 Datasheet page 54
    LTC6804_wrcfg(TOTAL_IC, tx_cfg); // Write configuration to ICs
}

void discharge_cell(int ic, int cell) {
    cell_discharging[ic][cell] = true;
    discharge_cell(ic, cell, true);
}

void discharge_cell(int ic, int cell, bool setDischarge) {
    if (ic < TOTAL_IC && cell < TOTAL_CELLS) {
        if (cell < 8) {
            if(setDischarge) {
                tx_cfg[ic][4] = tx_cfg[ic][4] | (0b1 << cell); 
            } else {
                tx_cfg[ic][4] = tx_cfg[ic][4] & ~(0b1 << cell ); 
            }
        } else {
            if (setDischarge) {
                tx_cfg[ic][5] = tx_cfg[ic][5] | (0b1 << (cell - 8)); 
            } else {
                tx_cfg[ic][5] = tx_cfg[ic][5] & ~(0b1 << (cell - 8)); 
            }
        }
    }
    wakeup_idle();
    //delayMicroseconds(1200); // Wait 4*t_wake for wakeup command to propogate and all 4 chips to wake up - See LTC6804 Datasheet page 54
    LTC6804_wrcfg(TOTAL_IC, tx_cfg);
}

void discharge_all() {
    for (int i = 0; i < TOTAL_IC; i++) {
        tx_cfg[i][4] = 0b11111111;
        tx_cfg[i][5] = tx_cfg[i][5] | 0b00001111;
    }
    wakeup_idle();
    //delayMicroseconds(1200); // Wait 4*t_wake for wakeup command to propogate and all 4 chips to wake up - See LTC6804 Datasheet page 54
    LTC6804_wrcfg(TOTAL_IC, tx_cfg);
}

void stop_discharge_cell(int ic, int cell) {
    cell_discharging[ic][cell] = false;
    discharge_cell(ic, cell, false);
}

void stop_discharge_all() {
    for (int i = 0; i < TOTAL_IC; i++) {
        tx_cfg[i][4] = 0b0;
        tx_cfg[i][5] = 0b0;
    }
    wakeup_idle();
    //delayMicroseconds(1200); // Wait 4*t_wake for wakeup command to propogate and all 4 chips to wake up - See LTC6804 Datasheet page 54
    LTC6804_wrcfg(TOTAL_IC, tx_cfg);
}

void balance_cells() {
  if (bms_voltages.get_low() > voltage_cutoff_low) {
      for (int ic = 0; ic < TOTAL_IC; ic++) { // Loop through ICs
          for (int cell = 0; cell < TOTAL_CELLS; cell++) { // Loop through cells
              if (!ignore_cell[ic][cell]) { // Ignore any cells specified in ignore_cell
                  uint16_t cell_voltage = cell_voltages[ic][cell]; // current cell voltage in mV
                  if (cell_discharging[ic][cell]) {
                      if (cell_voltage < bms_voltages.get_low() + voltage_difference_threshold - 6) {
                          stop_discharge_cell(ic, cell);
                      }
                  }
                  else if (cell_voltage > bms_voltages.get_low() + voltage_difference_threshold) {
                          discharge_cell(ic, cell);
                  }
              }
          }
      }
  }
  else {
      Serial.println("Not Balancing!");
      stop_discharge_all(); // Make sure none of the cells are discharging
  }
}

void poll_cell_voltage() {
    //Serial.println("Polling Voltages...");
    wakeup_sleep(); // Wake up LTC6804 ADC core
    LTC6804_adcv(); // Start cell ADC conversion
    delay(205); // Need to wait at least 201.317ms for conversion to finish, due to filtered sampling mode (26Hz) - See LTC6804 Datasheet Table 5
    //wakeup_sleep(); // TODO testing since every once in a while we still get 6.5535 issue with below 2 lines
    wakeup_idle(); // Wake up isoSPI
    delayMicroseconds(1200); // Wait 4*t_wake for wakeup command to propogate and all 4 chips to wake up - See LTC6804 Datasheet page 54
    uint8_t error = LTC6804_rdcv(0, TOTAL_IC, cell_voltages); // Reads voltages from ADC registers and stores in cell_voltages.
    if (error == -1) {
        Serial.println("A PEC error was detected in cell voltage data");
    }
    print_cells(); // Print the cell voltages to Serial.
}

void process_voltages() {
    poll_cell_voltage(); // cell_voltages[] array populated with cell voltages now.
    double totalVolts = 0; // stored as double volts
    uint16_t maxVolt = cell_voltages[0][0]; // stored in 0.1 mV units
    uint16_t minVolt = cell_voltages[0][0]; // stored in 0.1 mV units
    double avgVolt = 0; // stored as double volts
    int maxIC = 0;
    int maxCell = 0;
    int minIC = 0;
    int minCell = 0;
    for (int ic = 0; ic < TOTAL_IC; ic++) {
        for (int cell = 0; cell < TOTAL_CELLS; cell++) {
            bms_detailed_voltages[ic][cell / 3].set_voltage(cell % 3, cell_voltages[ic][cell]); // Populate CAN message struct
            if (!ignore_cell[ic][cell]) {
                uint16_t currentCell = cell_voltages[ic][cell];
                cell_delta_voltage[ic][cell] = currentCell - cell_delta_voltage[ic][cell];
                if (currentCell > maxVolt) {
                    maxVolt = currentCell;
                    maxIC = ic;
                    maxCell = cell;
                }
                if (currentCell < minVolt) {
                    minVolt = currentCell;
                    minIC = ic;
                    minCell = cell;
                }
                totalVolts += currentCell * 0.0001;
            }
        }
    }
    avgVolt = totalVolts / (TOTAL_IC * TOTAL_CELLS); // stored as double volts
    bms_voltages.set_average(static_cast<uint16_t>(avgVolt * 1000 + 0.5)); // stored in millivolts
    bms_voltages.set_total(static_cast<uint16_t>(totalVolts + 0.5)); // number is in units volts
    bms_voltages.set_low(minVolt);
    bms_voltages.set_high(maxVolt);

    // TODO: Low and High voltage error checking.

    //bms_status.set_overvoltage(false); // RESET these values, then check below if they should be set again
    //bms_status.set_undervoltage(false);
    //bms_status.set_total_voltage_high(false);

    if (bms_voltages.get_high() > voltage_cutoff_high*10) {
        if (first_fault_overvoltage) {
            bms_status.set_overvoltage(true); // TODO may need to comment this out for now when driving due to 65535 errors
        }
        first_fault_overvoltage = true;
        Serial.println("VOLTAGE FAULT!!!!!!!!!!!!!!!!!!!");
        Serial.print("max IC: "); Serial.println(maxIC);
        Serial.print("max Cell: "); Serial.println(maxCell); Serial.println();
    } else {
        first_fault_overvoltage = false;
    }

    if (bms_voltages.get_low() < voltage_cutoff_low) {
        if (first_fault_undervoltage) {
            bms_status.set_undervoltage(true);
        }
        first_fault_undervoltage = true;
        Serial.println("VOLTAGE FAULT!!!!!!!!!!!!!!!!!!!");
        Serial.print("min IC: "); Serial.println(minIC);
        Serial.print("min Cell: "); Serial.println(minCell); Serial.println();
    } else {
        first_fault_undervoltage = false;
    }

    if (bms_voltages.get_total() > total_voltage_cutoff) {
        if (first_fault_total_voltage_high) {
            bms_status.set_total_voltage_high(true); // TODO may need to comment this out for now when driving due to 65535 errors
        }
        first_fault_total_voltage_high = true;
        Serial.println("VOLTAGE FAULT!!!!!!!!!!!!!!!!!!!");
    }

    Serial.print("Average: "); Serial.println(avgVolt, 4);
    Serial.print("Total: "); Serial.println(totalVolts, 4);
    Serial.print("Min: "); Serial.println(minVolt / (double) 1e4, 4);
    Serial.print("Max: "); Serial.println(maxVolt / (double) 1e4, 4);
}

void poll_aux_voltage() {
    wakeup_sleep();
    //delayMicroseconds(200) // TODO try this if we are still having intermittent 6.5535 issues, maybe the last ADC isn't being given enough time to wake up
    LTC6804_adax(); // Start GPIO ADC conversion
    delay(205); // Need to wait at least 201.317ms for conversion to finish, due to filtered sampling mode (26Hz) - See LTC6804 Datasheet Table 5
    wakeup_idle(); // Wake up isoSPI
    delayMicroseconds(1200); // Wait 4*t_wake for wakeup command to propogate and all 4 chips to wake up - See LTC6804 Datasheet page 54
    uint8_t error = LTC6804_rdaux(0, TOTAL_IC, aux_voltages);
    if (error == -1) {
        Serial.println("A PEC error was detected in auxiliary voltage data");
    }
    // print_aux();
    delay(200);
}

void process_temps() { // TODO make work with signed int8_t CAN message (yes temperatures can be negative)
    double avgTemp, lowTemp, highTemp, totalTemp;
    poll_aux_voltage();
    totalTemp = 0;
    lowTemp = calculate_degrees_celsius(thermistor_resistance_gpio12(aux_voltages[0][0]));
    highTemp = lowTemp;
    for (int ic = 0; ic < TOTAL_IC; ic++) {
        Serial.println("Thermistor 1");
        uint16_t resistance = thermistor_resistance_gpio12(aux_voltages[ic][0]);
        uint16_t thermTemp = calculate_degrees_celsius(resistance);
        if (thermTemp < lowTemp) {
            lowTemp = thermTemp;
        }
        if (thermTemp > highTemp) {
            highTemp = thermTemp;
        }
        totalTemp += thermTemp;
        Serial.println("Thermistor 2");
        resistance = thermistor_resistance_gpio12(aux_voltages[ic][1]);
        thermTemp = calculate_degrees_celsius(resistance);
        if (thermTemp < lowTemp) {
            lowTemp = thermTemp;
        }
        if (thermTemp > highTemp) {
            highTemp = thermTemp;
        }
        totalTemp += thermTemp;
        Serial.println("Thermistor 3");
        resistance = thermistor_resistance_gpio3(aux_voltages[ic][2]);
        thermTemp = calculate_degrees_celsius(resistance);
        if (thermTemp < lowTemp) {
            lowTemp = thermTemp;
        }
        if (thermTemp > highTemp) {
            highTemp = thermTemp;
        }
        totalTemp += thermTemp;
        Serial.println("----------------------\n");
    }
    avgTemp = (uint16_t) (totalTemp / ((3) * TOTAL_THERMISTORS));
    bms_temperatures.set_low_temperature((uint16_t) lowTemp);
    bms_temperatures.set_high_temperature((uint16_t) highTemp);
    bms_temperatures.set_average_temperature((uint16_t) avgTemp);

    //bms_status.set_discharge_overtemp(false); // RESET these values, then check below if they should be set again
    //bms_status.set_charge_overtemp(false);

    if (bms_status.get_state() == BMS_STATE_DISCHARGING) { // Discharging
        if (bms_temperatures.get_high_temperature() > discharge_temp_critical_high) {
            bms_status.set_discharge_overtemp(true);
            Serial.println("TEMPERATURE FAULT!!!!!!!!!!!!!!!!!!!");
        }
    } else if (bms_status.get_state() == BMS_STATE_CHARGING) { // Charging
        if (bms_temperatures.get_high_temperature() > charge_temp_critical_high) {
            bms_status.set_charge_overtemp(true);
            Serial.println("TEMPERATURE FAULT!!!!!!!!!!!!!!!!!!!");
        }
    }

    Serial.print("Low Temp: ");
    Serial.println(lowTemp / 100);
    Serial.print("High Temp: ");
    Serial.println(highTemp / 100);
    Serial.print("Average Temp: ");
    Serial.println(avgTemp / 100);
}

/*
 * Calculate thermistor resistance if connected to GPIO1 or GPIO2, given temp_voltage in 0.1mV units
 */
uint16_t thermistor_resistance_gpio12(double temp_voltage) { // TODO check this math
    /* voltage measured across thermistor is dependent on the resistor in the voltage divider
     * all voltage measurements stored in arrays are in 0.1 mV, or 1/10,000 of a volt
     */
     temp_voltage = temp_voltage / 1e4;
     double resistance = 1e6 * (5 - temp_voltage) / (temp_voltage + 100 * temp_voltage - 5);
     Serial.println(resistance, 2);
     return (uint16_t) resistance;
    // resistances stored as 1 ohm units.
}

/*
 * Calculate thermistor resistance if connected to GPIO3, given temp_voltage in 0.1mV units
 */
uint16_t thermistor_resistance_gpio3(double temp_voltage) { // TODO check this math
    /* voltage measured across thermistor is dependent on the resistor in the voltage divider
     * all voltage measurements stored in arrays are in 0.1 mV, or 1/10,000 of a volt
     */
    temp_voltage = temp_voltage / 1e4;
    Serial.println(temp_voltage);
    double res = 5000.0 * temp_voltage;
    Serial.print("Step 1: "); Serial.println(res, 2);
    res = 25000.0 - res;
    Serial.print("Step 2: "); Serial.println(res, 2);
    res = res + 5000.0;
    Serial.print("Step 3: "); Serial.println(res, 2);
    res = res / (temp_voltage - 1.0);
    Serial.print("Final Step 4: "); Serial.println(res, 2);
//    double res = (25000.0 - 5000.0 * temp_voltage + 5000.0) / (temp_voltage - 1.0);
    Serial.print("resistance 3: "); Serial.println(res, 2);
    uint16_t small_res = (uint16_t) res;
    Serial.print("integer resistance 3: "); Serial.println(small_res);
    return small_res;
    // resistances stored as 1 ohm units.
}

/*
 * Calculate thermistor temperature in 0.1C units, given thermistor resistance in ohms
 */
int16_t calculate_degrees_celsius(double thermistor_resistance) {
    /*
     * Temperature equation (in Kelvin) based on resistance is the following:
     * 1/T = 1/T0 + (1/B) * ln(R/R0)
     * T = 1/(1/T0 + (1/B) * ln(R/R0))
     * 
     * T0 = 298.15 (25C in Kelvin)
     * B = 3984 (Specific to our thermistors)
     * R = thermistor_resistance (calculated resistance of thermistor)
     * R0 = 10000 (Resistance of thermistor at 25C)
     */
    double temperature = 1 / ((1 / (double) 298.15) + (1 / (double) 3984) * (double)log(thermistor_resistance / 10000)) - (double) 273.15;
    return (int16_t) (temperature * 100); // temps stored in 0.1 C units
}

void process_current() {
    /*
     * Current sensor: ISB-300-A-604
     * Maximum positive current (300A) corresponds to 4.5V signal
     * Maximum negative current (-300A) corresponds to 0.5V signal
     * 0A current corresponds to 2.5V signal
     * 
     * Resistor divider configuration (R1 = 1e3, R2=2e3):
     * 0V signal corresponds to analogRead == 0
     * 5V signal corresponds to analogRead == 1023
     * 
     * voltage = analogRead() * 5 / 1023
     * current = (voltage - 2.5) * 300 / 2
     */
    double voltage = analogRead(CURRENT_SENSE) / (double) 204.6;
    /*
     * Resistor divider compensation (TODO fix this in hardware by not using resistor dividers)
     * Sensor output @0A: 2.53V/5V=.506
     * Teensy input @0A: 1.698V/3.3V=.51454
     * Ratio: .506/.51454=.9834
     */
    voltage *= .9834;
    double current = (voltage - 2.5) * (double) 150;
    Serial.print("\nCurrent Sensor: ");
    Serial.print(current, 2);
    Serial.println("A");
    bms_status.set_current((int16_t) (current * 100));
    //bms_status.set_charge_overcurrent(false); // RESET these values, then check below if they should be set again
    //bms_status.set_discharge_overcurrent(false);
    if (current < 0) {
        bms_status.set_state(BMS_STATE_CHARGING);
        if (bms_status.get_current() < charge_current_constant_high) {
            bms_status.set_charge_overcurrent(true);
            Serial.println("CHARGE CURRENT HIGH FAULT!!!!!!!!!!!!!!!!!!!");
        }
    } else if (current > 0) {
        bms_status.set_state(BMS_STATE_DISCHARGING);
        if (bms_status.get_current() > discharge_current_constant_high) {
            bms_status.set_discharge_overcurrent(true);
            Serial.println("DISCHARGE CURRENT HIGH FAULT!!!!!!!!!!!!!!!!!!!");
        }
    }
}

/*
 * Update maximum and minimum allowed voltage, current, temperature, etc.
 */
int update_constraints(uint8_t address, short value) {
    switch(address) {
        case 0: // voltage_cutoff_low
            voltage_cutoff_low = value;
            break;
        case 1: // voltage_cutoff_high
            voltage_cutoff_high = value;
            break;
        case 2: // total_voltage_cutoff
            total_voltage_cutoff = value;
            break;
        case 3: // discharge_current_constant_high
            discharge_current_constant_high = value;
            break;
        case 4: // charge_current_constant_high
            charge_current_constant_high = value;
            break;
        case 5: // charge_temp_critical_high
            charge_temp_critical_high = value;
            break;
        case 6: // discharge_temp_critical_high
            discharge_temp_critical_high = value;
            break;
        case 7: // voltage_difference_threshold
            voltage_difference_threshold = value;
            break;
        default:
            return -1;
    }
    return 0;
}

void print_cells() {
    for (int current_ic = 0; current_ic < TOTAL_IC; current_ic++) {
        Serial.print("IC: ");
        Serial.println(current_ic+1);
        for (int i = 0; i < TOTAL_CELLS; i++) {
            Serial.print("C"); Serial.print(i);
            if (ignore_cell[current_ic][i]) {
                Serial.print(" IGNORED CELL ");
            }
            Serial.print(": ");
            float voltage = cell_voltages[current_ic][i] * 0.0001;
            Serial.print(voltage, 4);
            Serial.print(" Discharging: ");
            Serial.print(cell_discharging[current_ic][i]); 
            Serial.print(" Voltage difference: ");
            Serial.print(cell_voltages[current_ic][i]-bms_voltages.get_low());
            Serial.print(" Delta To Threshold: ");
            Serial.println((cell_voltages[current_ic][i]-bms_voltages.get_low())-voltage_difference_threshold);
        }
        Serial.println();
    }
}

void print_aux() {
    for (int current_ic = 0; current_ic < TOTAL_IC; current_ic++) {
        Serial.print("IC: ");
        Serial.println(current_ic + 1);
        for (int i = 0; i < 6; i++) {
            Serial.print("Aux-"); Serial.print(i+1); Serial.print(": ");
            float voltage = aux_voltages[current_ic][i] * 0.0001;
            Serial.println(voltage, 4);
        }
        Serial.println();
    }
}

void print_temperatures() {
    Serial.print("\nAverage temperature: ");
    Serial.print(bms_temperatures.get_average_temperature() / (double) 100, 2);
    Serial.println(" C");
    Serial.print("Low temperature: ");
    Serial.print(bms_temperatures.get_low_temperature() / (double) 100, 2);
    Serial.println(" C");
    Serial.print("High temperature: ");
    Serial.print(bms_temperatures.get_high_temperature() / (double) 100, 2);
    Serial.println(" C\n");
}

/*
 * Print ECU uptime
 */
void print_uptime() {
    Serial.print("\nECU uptime: ");
    Serial.print(millis() / 1000);
    Serial.print(" seconds (");
    Serial.print(millis() / 1000 / 60);
    Serial.print(" minutes, ");
    Serial.print(millis() / 1000 % 60);
    Serial.println(" seconds)\n");
}

/*
 * Set VOV in configuration registers
 * Voltage is in 100 uV increments
 * See LTC6804 datasheet pages 25 and 53
 */
void cfg_set_overvoltage_comparison_voltage(uint16_t voltage) {
    voltage /= 16;
    for (int i = 0; i < TOTAL_IC; i++) {
        tx_cfg[i][2] = (tx_cfg[i][2] & 0x0F) | ((voltage && 0x00F) << 8);
        tx_cfg[i][3] = (voltage && 0xFF0) >> 4;
    }
}

/*
 * Set VUV in configuration registers
 * Voltage is in 100 uV increments
 * See LTC6804 datasheet pages 25 and 53
 */
void cfg_set_undervoltage_comparison_voltage(uint16_t voltage) {
    voltage /= 16;
    voltage -= 10000;
    for (int i = 0; i < TOTAL_IC; i++) {
        tx_cfg[i][1] = voltage && 0x0FF;
        tx_cfg[i][2] = (tx_cfg[i][2] & 0xF0) | ((voltage && 0xF00) >> 8);
    }
}
