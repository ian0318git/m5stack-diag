/******************************************************************************
 * nios_reg.h
 *
 * Header file for NIOS registers.
 * This file is common between NIOS firmware and IOS software
 *
 * Copyright (c) 2020 by cisco Systems, Inc.
 * All rights reserved.
 *****************************************************************************/
#ifndef __NIOS_REG_H__
#define __NIOS_REG_H__


// Define typedefs used in file definition
typedef signed int                      sint32;
typedef signed short                    sint16;
typedef signed char                     schar;


// Define maximum queue size and message size
#define   MAX_NIOS_MSGS                 6         // Maximum Queue Entries: Nios to Host
#define   MAX_HOST_MSGS                 6         // Maximum Queue Entries: Host to Nios
#define   MAX_NIOS_MSG_LEN              18        // Maximum Message Length: Nios to Host
#define   MAX_HOST_MSG_LEN              18        // Maximum Message Length: Host to Nios


// Nios-to-Host IPC Queue Control
typedef struct {
    // Basic NIOS information
    uint32            signature;                    // 0xDEADBEEF - for NIOS-to-Host
                                                    // 0xCAFECAFE - for Host-to-NIOS
    uint8             head;                         // Index of Next Message from host
                                                    // Master will update the Head
    uint8             tail;                         // Index of Last Message from host
                                                    // Slave will update the Tail
    uint16            pad;                          //
    uint32            message_count;                // Total number of messages
} nios_ipc_t;


// Nios-to-Host IPC Message
typedef struct {
  uint8             cmd;                          // Message Command
  uint8             len;                          // Message Length
  uint8             data[MAX_NIOS_MSG_LEN];       // Message Data
} nios_msg_t;


// Motherboard System Voltages.
typedef struct {
                                                  // NEPTUNE (mV)
  sint16            voltage_0;                    // 12V
  sint16            voltage_1;                    // 5V
  sint16            voltage_2;                    // 3.3V_STBY
  sint16            voltage_3;                    // 3.3V_CPCPU
  sint16            voltage_4;                    // 3.3V
  sint16            voltage_5;                    // 3.0V
  sint16            voltage_6;                    // 2.5V
  sint16            voltage_7;                    // 1.8V_CPCPU
  sint16            voltage_8;                    // 1.8V
  sint16            voltage_9;                    // 1.7V
  sint16            voltage_10;                   // 1.5V
  sint16            voltage_11;                   // 1.3V
  sint16            voltage_12;                   // 1.2V
  sint16            voltage_13;                   // 1.2V_CPCPU
  sint16            voltage_14;                   // 1.1V
  sint16            voltage_15;                   // 1.05V
  sint16            voltage_16;                   // 1.0V
  sint16            voltage_17;                   // 0.9V
  sint16            voltage_18;                   // 0.85V
  sint16            voltage_19;                   // 0.6V_CPCPU
  sint16            voltage_20;                   // 0.6V_DPCPU
  uint16            pad[11];                      // Future Use (up to 32 rails)
} voltages_t;


// Temperature Sensors
typedef struct {
  sint16            inlet_1_temp;                 // Inlet 1 Temperature (C)
  sint16            inlet_2_temp;                 // Inlet 2 Temperature (C)
  sint16            outlet_1_temp;                // Outlet 1 Temperature (C)
  sint16            outlet_2_temp;                // Outlet 2 Temperature (C)
  sint16            working_temp;                 // Working Temperature (C)
  sint16            cpu_core_temp;                // CP CPU Temperature (C)
  sint16            cpu_pch_temp;                 // CP PCH Temperature (C)
  sint16            dpcpu_core_temp;              // DP CP Temperature (C)
  sint16            cpu_ddr0_temp;                // CP DDR0 Temperature (C)
  sint16            cpu_ddr1_temp;                // CP DDR1 Temperature (C)
  sint16            device_temp[6];               // Future use
} temperature_t;


// Module environmental parameters
typedef struct {
  sint16            in_12v;                       // Voltage in mV
  sint16            in_12v_i;                     // Current in mA
  sint16            poe_reading;                  // Future Use
  sint16            poe_i_power_reading;          // Future Use
  uint32            pad;                          // Future use, e.g. temp
} module_env_t;


// Fan rotational speed and status.
typedef struct {
  uint16            fan1;                         // Fan 1 (RPM)
  uint16            fan2;                         // Fan 2 (RPM)
  uint16            fan3;                         // Fan 3 (RPM)
  uint16            fan4;                         // Fan 4 (RPM)
  uint16            fan5;                         // Fan 5 (RPM), Future Use
  uint16            pad[3];                       // Future use
} fan_t;


// Power readings.
typedef struct {
  sint32            motherboard;                  // Power to Motherboard (mW)
  sint32            nim;                          // Power to NIMs (mW)
  sint32            fan;                          // Power to Fans (mW)
  sint32            cpu;                          // Power to CP CPU (mW), Future Use
  sint32            cpu_ddr4;                     // Power to CP CPU DDR4 (mW)
  sint32            dp_cpu;                       // Power to DP CPU (mW)
  sint32            dp_ddr4;                      // Power to DP CPU DDR4 (mW)
  sint32            pad;                          // Future use
} power_t;


// Miscellaneous - use for other or spare.
typedef struct {
  sint16            pressure;                     // Pressure (kPA)
  uint16            pad[15];                      // Future use
} misc_env_t;


// Fan Control.
typedef struct {
  uint16            status;                       // Future Use
  uchar             fan_table;                    // See FAN_TABLE_xxx defines
  uchar             fan_temp_level;               // Temperature: 1-5
  uchar             fan_barom_level;              // Altitude: 1-5
  uchar             fan_tray_status;              // See FAN_TRAY_xxx defines
  uchar             fan_table_data[3][5][5];      // 3=fan tables, 5=barometer leveles, 5=temperature levels
  uchar             pad_1[4];                     //
  uchar             override;                     // See FAN_OVERRIDE_xxx defines
  uchar             barometer_level[5];           // presure thresholds
  uchar             pad_2[3];                     //
  uchar             temperature_level[8];         // temperature thresholds
  uchar             pad_3[10];                    // Future use
} fan_control_t;


// Cisco SmartFan Data
typedef struct {
  unsigned char     cisco_cpn[16];                // SmartFan CPN
  unsigned char     cisco_serial_number[12];      // SmartFan Cisco Serial Number
  unsigned char     cisco_rev[2];                 // SmartFan CPN Revision
  unsigned char     mfg_name[10];                 // SmartFan Manufacturer
  unsigned char     mfg_pn[20];                   // SmartFan Manufacturer Part Number
  unsigned char     mfg_assy_location[10];        // SmartFan Manufacturer Assembly Location
  unsigned char     mfg_datecode[12];             // SmartFan Manufacturer Date Code
  unsigned char     mfg_sn[16];                   // SmartFan Manufacturer Serial Number
  unsigned char     pad[30];                      //
} csfan_data_inv_t;


// Cookie Information
typedef struct {
  uint8             data[256];                    // Raw Cookie Data
} dev_cookie_t;


// AC or DC Power supply environmental parameters
typedef struct {
  uint32            in_v;                         // Input voltage (mV)
  uint32            in_i;                         // Input Current (mA)
  uint32            in_pwr;                       // Input Power (mW)
  uint32            out_12v;                      // 12V output voltage (mV)
  uint32            out_12v_i;                    // 12V output current (mA)
  uint32            out_12v_pwr;                  // 12V output power (mW)
  schar             temp1;                        // Temperature 1 (C)
  schar             temp2;                        // Temperature 2 (C)
  schar             temp3;                        // Temperature 3 (C)
  uchar             pad_1;                        //
  uint16            fan1;                         // Fan 1 in RPM
  uint16            pad_2;                        //
  uint32            pad_3[2];                     //
} psu12v_env_t;


// POE PSU converter supply environmental parameters
typedef struct {
  uint32            in_12v;                       // POE PSU Input Voltage (mV)
  uint32            in_12v_i;                     // POE PSU Input Current (mA)
  uint32            in_pwr;                       // POE PSU Input Power (mW)
  uint32            out_pwr;                      // POE PSU Output Power (mW)
  schar             temp1;                        // POE PSU Temperature ( C)
  schar             pad_1[3];                     //
  uint16            pad_2[6];                     //
} poe_env_t;


// NIOS to Host Data Structure
typedef struct {

  // Environmental Data
  voltages_t        volt;                         // Voltage structure (0x0)
  temperature_t     temp;                         // Temperature structure (0x40)
  fan_t             fan;                          // Fan Structure (0x60)
  power_t           power;                        // Power Structure (0x70)
  misc_env_t        misc_env;                     // Miscellaneous Sensor Data (0x90)

  // Fan Control
  fan_control_t     fan_ctrl;                     // Fan Control Information (0xB0)

  // Power Supplies
  dev_cookie_t      psu0_cookie;                  // PSU0 Cookie (0x120)
  dev_cookie_t      psu1_cookie;                  // PSU1 Cookie (0x220)
  dev_cookie_t      poe0_cookie;                  // POE PSU 0 Cookie (0x320)
  dev_cookie_t      poe1_cookie;                  // POE PSU 1 Cookie (0x420)
  psu12v_env_t      psu0_env;                     // PSU 0 Environmental Data Structure (0x520)
  psu12v_env_t      psu1_env;                     // PSU 1 Environmental Data Structure (0x548)
  poe_env_t         poe0_env;                     // POE PSU 0 Environmental Data Structure (0x570)
  poe_env_t         poe1_env;                     // POE PSU 1 Environmental Data Structure (0x590)
  unsigned char     psu0_status;                  // PSU 0 Status. 1-installed (0x5B0)
  unsigned char     psu1_status;                  // PSU 1 Status. 1-installed (0x5B1)
  unsigned char     poe0_status;                  // POE PSU 0 Status. 1-installed (0x5B2)
  unsigned char     poe1_status;                  // POE PSU 1 Status. 1-installed (0x5B3)

  // Padding
  unsigned char     pad_1[4];                     // (0x5B4)

  // Power Debug Data
  uint16            power_exceed_psu;             // Number of times PSU power exceeded (0x5B8)
  uint16            power_exceed_mb;              // Number of times Motherboard power exceeded (0x5BA)
  uint16            power_exceed_fan;             // Number of times Fan power exceeded (0x5BC)
  uint16            power_exceed_nim;             // Number of times NIM power exceeded (0x5BE)

  // Cisco Smartfan Data
  csfan_data_inv_t  smartfan_data[4];             // Cisco Smartfan Data, Inventory (0x5C0)

  // Padding
  unsigned char     pad[64];                      // (0x7C0)
} n2h_parm_t;


// Host to NIOS Data Structure
typedef struct {

  // Sensor Offset
  temperature_t     temp;                         // Temperature Sensor Offset (0x0)
  power_t           power;                        // Power Sensor Offset (0x20)
  misc_env_t        misc_env;                     // Miscellaneous Sensor Offset (0x40)

  // Fan Control Override
  fan_control_t     fan_ctrl;                     // Fan Control Override Information (0x60)

  // NGIO Module Data
  module_env_t      sm1;                          // Future use (0xD0)
  module_env_t      sm2;                          // Future use (0xDC)
  module_env_t      sm3;                          // Future use (0xE8)
  module_env_t      sm4;                          // Future use (0xF4)
  module_env_t      ngwic1;                       // Reserved for future use (0x100)
  module_env_t      ngwic2;                       // Reserved for future use (0x10C)
  module_env_t      ngwic3;                       // Reserved for future use (0x118)
  module_env_t      ngwic4;                       // Reserved for future use (0x124)

  // Threshold, High
  voltages_t        volt_alarm_hi;                // (0x130)
  temperature_t     temp_alarm_hi;                // (0x170)
  fan_t             fan_alarm_hi;                 // (0x190)
  power_t           power_alarm_hi;               // (0x1A0)
  misc_env_t        misc_env_alarm_hi;            // (0x1C0)
  psu12v_env_t      psu0_env_alarm_hi;            // Reserved for future use (0x1E0)
  psu12v_env_t      psu1_env_alarm_hi;            // Reserved for future use (0x208)
  poe_env_t         poe0_env_alarm_hi;            // Reserved for future use (0x230)
  poe_env_t         poe1_env_alarm_hi;            // Reserved for future use (0x250)

  // Threshold, High
  voltages_t        volt_alarm_lo;                // (0x270)
  temperature_t     temp_alarm_lo;                // (0x2B0)
  fan_t             fan_alarm_lo;                 // (0x2D0)
  power_t           power_alarm_lo;               // (0x2E0)
  misc_env_t        misc_env_alarm_lo;            // (0x300)
  psu12v_env_t      psu0_env_alarm_lo;            // Reserved for future use (0x320)
  psu12v_env_t      psu1_env_alarm_lo;            // Reserved for future use (0x348)
  poe_env_t         poe0_env_alarm_lo;            // Reserved for future use (0x370)
  poe_env_t         poe1_env_alarm_lo;            // Reserved for future use (0x390)

  // System Power
  uint32            power_psu_max_diff;           // Margin with which the PSU output (0x3B0)
                                                  // can exceed total calculated system
                                                  // power and cause system shutdown
  // Padding
  uint8             pad_1[12];                    // (0x3B4)
                                                  //
  uchar             temp_force_alert;             // Force high temperature condition (0x3C0)
  uchar             pad_2[3];                     // (0x3C1)

  uchar             pad_3[60];                    // (0x3C4)

} h2n_parm_t;


// Main data structure.  Resides at 0x34000 in the FPGA
typedef struct {
  // Basic NIOS information
  uint16            nios_status;                  // Status (0x0)
                                                  // 0x0001 - Gracefully stopped (set by IPC message)
                                                  // 0x0003 - Diags mode (written by diags)
                                                  // 0x494E - NIOS is initializing (set by NIOS)
                                                  // 0x4E49 - NIOS is running  (set by NIOS)

  uint16            nios_version;                 // NIOS firmware revision (0x2)

  uint8             nios_fw_crash_detected;       // 1 - Watchdog Event occurred (0x4)
  uint8             pad_debug_info_1[3];          // (0x5)
  uint32            main_polling_loop_cnt;        // NIOS loops executed (0x8)
  uint8             pad_debug_info_2[4];          // (0xC)
  uint16            nios_mode;                    // NIOS mode. For Diags only (0x10)
  uint16            pwr_down_reason;              // Power Sequencer Status register (0x12)
  uint16            pwr_voltage_fault_vector0;    // Power Sequencer Voltage Fault register 0 (0x14)
  uint16            pwr_voltage_fault_vector1;    // Power Sequencer Voltage Fault register 1 (0x16)
  uint16            fpga_pwr_down_reason;         // Power Sequencer Scratchpad 0 register (0x18)
                                                  // 1-NIOS System Power Safety
                                                  // 2-NIOS Fan Power Safety
                                                  // 3-NIOS NIM Power Safety
                                                  // 4-NIOS Code Corruption
  uint8             pad_debug_info_3[6];          // (0x1A)

                                                  //
  uint16            nios_ipc_ctrl_ptr;            // Debug Only (0x20)
  uint16            host_ipc_ctrl_ptr;            // Debug Only (0x22)
  uint16            psu0_cookie_ptr;              // Debug Only (0x24)
  uint16            psu1_cookie_ptr;              // Debug Only (0x26)
  uint16            poe0_cookie_ptr;              // Debug Only (0x28)
  uint16            poe1_cookie_ptr;              // Debug Only (0x2A)
  uint16            n2h_ptr;                      // Debug Only (0x2C)
  uint16            h2n_ptr;                      // Debug Only (0x2E)
  uchar             pad_debug_info_4[16];         // (0x30)

                                                  //
  nios_ipc_t        nios_ctrl;                    // (0x40)
  nios_msg_t        nios_msg[MAX_NIOS_MSGS];      // (0x4C)
  nios_ipc_t        host_ctrl;                    // (0xC4)
  nios_msg_t        host_msg[MAX_HOST_MSGS];      // (0xD0)
  char              pad_ipc[8];                   // (0x148)

                                                  //
  uint16            nios_int_sts_sys;             // NIOS internal status, System (0x150)
  uint16            nios_int_sts_fan;             // NIOS internal status, Fan (0x152)
  uint16            nios_int_sts_psu;             // NIOS internal status, PSU (0x154)
  uint16            nios_int_sts_poe;             // NIOS internal status, POE (0x156)
  uint16            nios_int_sts_temp;            // NIOS internal status, Temperature (0x158)
  uint16            nios_int_sts_ovc;             // NIOS internal status, Over-current (0x15A)

  uint16            pending_n2h_msgs;             // Debug Only (0x15C)
  uint16            nios_i2c_err_device[2];       // Status of I2C devices (0x15E)

  // Padding
  uchar             pad[158];                     // (0x162)

                                                  //
  n2h_parm_t        n2h_parm;                     // NIOS-to-Host structure (0x200)

                                                  //
  h2n_parm_t        h2n_parm;                     // Host-to-NIOS structure (0xA00)


  // Padding
  uchar             pad_1[512];                   // (0xE00)

  // Raw device data. For debug only
  uchar             debug_raw[4096];              // Populated by NIOS. (0x1000)

} nios_mailbox_type_t;

#define NIOS_MBOX_NIOS_STATUS 0x0
#define NIOS_MBOX_NIOS_VERSION 0x2
#define NIOS_MBOX_NIOS_FW_CRASH_DET 0x4
#define NIOS_MBOX_PAD_DEBUG_INFO_1 0x5
#define NIOS_MBOX_MAIN_POLLING_LOOP_CNT 0x8
#define NIOS_MBOX_PAD_DEBUG_INFO_2 0xC
#define NIOS_MBOX_NIOS_MODE 0x10
#define NIOS_MBOX_PWR_DOWN_REASON 0x12
#define NIOS_MBOX_PWR_VOLTAGE_FAULT_VECTOR0 0x14
#define NIOS_MBOX_PWR_VOLTAGE_FAULT_VECTOR1 0x16
#define NIOS_MBOX_FPGA_PWR_DOWN_REASON 0x18
#define NIOS_MBOX_PAD_DEBUG_INFO_3 0x1A
#define NIOS_MBOX_NIOS_IPC_CTRL_PTR 0x20
#define NIOS_MBOX_HOST_IPC_CTRL_PTR 0x22
#define NIOS_MBOX_PSU0_COOKIE_PTR 0x24
#define NIOS_MBOX_PSU1_COOKIE_PTR 0x26
#define NIOS_MBOX_POE0_COOKIE_PTR 0x28
#define NIOS_MBOX_POE1_COOKIE_PTR 0x2A
#define NIOS_MBOX_N2H_PTR 0x2C
#define NIOS_MBOX_H2N_PTR 0x2E
#define NIOS_MBOX_PAD_DEBUG_INFO_4 0x30
#define NIOS_MBOX_NIOS_CTRL 0x40
#define NIOS_MBOX_NIOS_MSG 0x4C
#define NIOS_MBOX_HOST_CTRL 0xC4
#define NIOS_MBOX_HOST_MSG 0xD0
#define NIOS_MBOX_PAD_IPC 0x148
#define NIOS_MBOX_NIOS_INT_STS_SYS 0x150
#define NIOS_MBOX_NIOS_INT_STS_FAN 0x152
#define NIOS_MBOX_NIOS_INT_STS_PSU 0x154
#define NIOS_MBOX_NIOS_INT_STS_POE 0x156
#define NIOS_MBOX_NIOS_INT_STS_TEMP 0x158
#define NIOS_MBOX_NIOS_INT_STS_OVC 0x15A
#define NIOS_MBOX_PENDING_N2H_MSGS 0x15C
#define NIOS_MBOX_NIOS_I2C_ERR_DEV 0x15E
#define NIOS_MBOX_PAD 0x162
#define NIOS_MBOX_N2H_PARM 0x200
#define NIOS_MBOX_H2N_PARM 0xA00
#define NIOS_MBOX_PAD_1 0xE00
#define NIOS_MBOX_DEBUG_RAW 0x1000
#define NIOS_MBOX_NOT_DEFINED 0x2000

/* NIOS Mailbox structure data type */
#define NIOS_MBOX_NOT_SUPPORT 0x0
#define NIOS_MBOX_DT_INT8 0x1
#define NIOS_MBOX_DT_INT16 0x2
#define NIOS_MBOX_DT_INT32 0x4
#define NIOS_MBOX_DT_STRUCT 0x8
#define NIOS_MBOX_DT_ARRAY 0x10

#endif
