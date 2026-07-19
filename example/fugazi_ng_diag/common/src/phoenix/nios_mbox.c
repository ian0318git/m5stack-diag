/*------------------------------------------------------------------
 *
 * nios_mbox.c
 *
 * Copyright (c) 2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */


#include <stdio.h>
#include "types.h"
#include "common.h"
#include "queryflags.h"
#include "diag_fpga_lib.h"
#include "nios_reg.h"
#include "nios_mbox_api.h"


#define NIOS_MBOX_SHOW_MENU_MODE 0x0
#define NIOS_MBOX_SHOW_DUMP_MODE 0x1

#define FAN_CTRL_PRESSURE_THRESHOLDS_NUM 5
#define FAN_CTRL_TEMPERATURE_THRESHOLDS_NUM 8
#define NIOS_MBOX_N2H_PARM_COOKIE_SIZE 256
#define SMARTFAN_DATA_ARRAY_SIZE 4


typedef struct {
    unsigned int offset;
    char strName[40];
    unsigned int type;
    unsigned int ar_size;
} nios_mbox_tbl_t;


static nios_mbox_tbl_t nios_mbox_table[] = {
    { NIOS_MBOX_NIOS_STATUS, "NIOS Status", NIOS_MBOX_DT_INT16, 0 },
    { NIOS_MBOX_NIOS_VERSION, "NIOS Firmware Revision", NIOS_MBOX_DT_INT16, 0 },
    { NIOS_MBOX_NIOS_FW_CRASH_DET, "NIOS Crash Detected", NIOS_MBOX_DT_INT8, 0 },
    { NIOS_MBOX_PAD_DEBUG_INFO_1, "Pad Debug Info 1", NIOS_MBOX_DT_INT8 | NIOS_MBOX_DT_ARRAY, 3 },
    { NIOS_MBOX_MAIN_POLLING_LOOP_CNT, "NIOS Loops Executed", NIOS_MBOX_DT_INT32, 0 },
    { NIOS_MBOX_PAD_DEBUG_INFO_2, "Pad Debug Info 2", NIOS_MBOX_DT_INT8 | NIOS_MBOX_DT_ARRAY, 4 },
    { NIOS_MBOX_NIOS_MODE, "NIOS Mode", NIOS_MBOX_DT_INT16, 0 },
    { NIOS_MBOX_PWR_DOWN_REASON, "Power Down Reason", NIOS_MBOX_DT_INT16, 0 },
    { NIOS_MBOX_PWR_VOLTAGE_FAULT_VECTOR0, "Power Voltage Fault Vector 0", NIOS_MBOX_DT_INT16, 0 },
    { NIOS_MBOX_PWR_VOLTAGE_FAULT_VECTOR1, "Power Voltage Fault Vector 0", NIOS_MBOX_DT_INT16, 0 },
    { NIOS_MBOX_FPGA_PWR_DOWN_REASON, "Logic FPGA Power Down Reason", NIOS_MBOX_DT_INT16, 0 },
    { NIOS_MBOX_PAD_DEBUG_INFO_3, "Pad Debug Info 3", NIOS_MBOX_DT_INT8 | NIOS_MBOX_DT_ARRAY, 6 },
    { NIOS_MBOX_NIOS_IPC_CTRL_PTR, "NIOS IPC CTRL PTR", NIOS_MBOX_DT_INT16, 0 },
    { NIOS_MBOX_HOST_IPC_CTRL_PTR, "HOST IPC CTRL PTR", NIOS_MBOX_DT_INT16, 0 },
    { NIOS_MBOX_PSU0_COOKIE_PTR, "PSU0 Cookie PTR", NIOS_MBOX_DT_INT16, 0 },
    { NIOS_MBOX_PSU1_COOKIE_PTR, "PSU1 Cookie PTR", NIOS_MBOX_DT_INT16, 0 },
    { NIOS_MBOX_POE0_COOKIE_PTR, "POE0 Cookie PTR", NIOS_MBOX_DT_INT16, 0 },
    { NIOS_MBOX_POE1_COOKIE_PTR, "POE1 Cookie PTR", NIOS_MBOX_DT_INT16, 0 },
    { NIOS_MBOX_N2H_PTR, "N2H PTR", NIOS_MBOX_DT_INT16, 0 },
    { NIOS_MBOX_H2N_PTR, "H2N PTR", NIOS_MBOX_DT_INT16, 0 },
    { NIOS_MBOX_PAD_DEBUG_INFO_4, "Pad Debug Info 4", NIOS_MBOX_DT_INT8 | NIOS_MBOX_DT_ARRAY, 16 },
    { NIOS_MBOX_NIOS_CTRL, "NIOS CTRL", NIOS_MBOX_NOT_SUPPORT, 0 },  // Parsing NOT support
    { NIOS_MBOX_NIOS_MSG, "NIOS MSG", NIOS_MBOX_NOT_SUPPORT, 6 },  // Parsing NOT support
    { NIOS_MBOX_HOST_CTRL, "HOST CTRL", NIOS_MBOX_NOT_SUPPORT, 0 },  // Parsing NOT support
    { NIOS_MBOX_HOST_MSG, "HOST MSG", NIOS_MBOX_NOT_SUPPORT, 6 },  // Parsing NOT support
    { NIOS_MBOX_PAD_IPC, "Pad IPC", NIOS_MBOX_DT_INT8 | NIOS_MBOX_DT_ARRAY, 8 },
    { NIOS_MBOX_NIOS_INT_STS_SYS, "NIOS Internal System Status", NIOS_MBOX_DT_INT16, 0 },
    { NIOS_MBOX_NIOS_INT_STS_FAN, "NIOS Internal FAN Status", NIOS_MBOX_DT_INT16, 0 },
    { NIOS_MBOX_NIOS_INT_STS_PSU, "NIOS Internal PSU Status", NIOS_MBOX_DT_INT16, 0 },
    { NIOS_MBOX_NIOS_INT_STS_POE, "NIOS Internal POE Status", NIOS_MBOX_DT_INT16, 0 },
    { NIOS_MBOX_NIOS_INT_STS_TEMP, "NIOS Internal Temperature Status", NIOS_MBOX_DT_INT16, 0 },
    { NIOS_MBOX_NIOS_INT_STS_OVC, "NIOS Internal Over-Current Status", NIOS_MBOX_DT_INT16, 0 },
    { NIOS_MBOX_PENDING_N2H_MSGS, "Pending N2H Msgs", NIOS_MBOX_DT_INT16, 0 },
    { NIOS_MBOX_NIOS_I2C_ERR_DEV, "NIOS I2C Error Devices", NIOS_MBOX_DT_INT16 | NIOS_MBOX_DT_ARRAY, 2 },
    { NIOS_MBOX_PAD, "Pad", NIOS_MBOX_DT_INT8 | NIOS_MBOX_DT_ARRAY, 158 },
    { NIOS_MBOX_N2H_PARM, "N2H Param", NIOS_MBOX_DT_STRUCT, 0 },
    { NIOS_MBOX_H2N_PARM, "H2N Param", NIOS_MBOX_NOT_SUPPORT, 0 },  // Parsing NOT support
    { NIOS_MBOX_PAD_1, "Pad_1", NIOS_MBOX_DT_INT8 | NIOS_MBOX_DT_ARRAY, 512 },
    { NIOS_MBOX_DEBUG_RAW, "Debug RAW", NIOS_MBOX_DT_INT8 | NIOS_MBOX_DT_ARRAY, 4096 },
    { NIOS_MBOX_NOT_DEFINED, "", NIOS_MBOX_NOT_SUPPORT, 0 }
};


/*******************************************************************************
 *
 * Function   : nios_mbox_int8_show
 * Description: Show 8-bit integer or 8-bit integer array.
 * Inputs     : offset - NIOS mailbox's offset to be shown.
 *              size - 1 means an 8-bit integer.
 *                     > 1 means an 8-bit integer array.
 * Outputs    : None.
 *
 *******************************************************************************
 */
static void nios_mbox_int8_show(unsigned int offset, unsigned int size)
{
    unsigned int ix;
    unsigned long addr = get_platform_nios_mailbox_msg_base();
    uint8 *var = (uint8 *)(addr + offset);

    for (ix = 0; ix < size; ix++) {
        if (ix > 0 && ix%16 == 0) {
            printf("\n");
        }
        printf("0x%X ", var[ix]);
    }

    return;
}


/*******************************************************************************
 *
 * Function   : nios_mbox_int16_show
 * Description: Show 16-bit integer or 16-bit integer array.
 * Inputs     : offset - NIOS mailbox's offset to be shown.
 *              size - 1 means an 16-bit integer.
 *                     > 1 means an 16-bit integer array.
 * Outputs    : None.
 *
 *******************************************************************************
 */
static void nios_mbox_int16_show(unsigned int offset, unsigned int size)
{
    unsigned int ix;
    unsigned long addr = get_platform_nios_mailbox_msg_base();
    uint16 *var = (uint16 *)(addr + offset);

    for (ix = 0; ix < size; ix++) {
        if (ix > 0 && ix%16 == 0) {
            printf("\n");
        }
        printf("0x%X ", var[ix]);
    }

    return;
}


/*******************************************************************************
 *
 * Function   : nios_mbox_int32_show
 * Description: Show 32-bit integer or 32-bit integer array.
 * Inputs     : offset - NIOS mailbox's offset to be shown.
 *              size - 1 means an 32-bit integer.
 *                     > 1 means an 32-bit integer array.
 * Outputs    : None.
 *
 *******************************************************************************
 */
static void nios_mbox_int32_show(unsigned int offset, unsigned int size)
{
    unsigned int ix;
    unsigned long addr = get_platform_nios_mailbox_msg_base();
    uint32 *var = (uint32 *)(addr + offset);

    for (ix = 0; ix < size; ix++) {
        if (ix > 0 && ix%16 == 0) {
            printf("\n");
        }
        printf("0x%X ", var[ix]);
    }

    return;
}


/*******************************************************************************
 *
 * Function   : nios_mbox_n2h_parm_show_volt
 * Description: Show the voltage information in NIOS mailbox n2h_parm.
 * Inputs     : volt - n2h_parm volt
 * Outputs    : None.
 *
 *******************************************************************************
 */
static void nios_mbox_n2h_parm_show_volt(voltages_t volt)
{
    printf("\nVoltages:\n");
    printf("12V        = %d mV\n", volt.voltage_0);
    printf("5V         = %d mV\n", volt.voltage_1);
    printf("3.3V_STBY  = %d mV\n", volt.voltage_2);
    printf("3.3V_CPCPU = %d mV\n", volt.voltage_3);
    printf("3.3V       = %d mV\n", volt.voltage_4);
    printf("3.0V       = %d mV\n", volt.voltage_5);
    printf("2.5V       = %d mV\n", volt.voltage_6);
    printf("1.8V_CPCPU = %d mV\n", volt.voltage_7);
    printf("1.8V       = %d mV\n", volt.voltage_8);
    printf("1.7V       = %d mV\n", volt.voltage_9);
    printf("1.5V       = %d mV\n", volt.voltage_10);
    printf("1.3V       = %d mV\n", volt.voltage_11);
    printf("1.2V       = %d mV\n", volt.voltage_12);
    printf("1.2V_CPCPU = %d mV\n", volt.voltage_13);
    printf("1.1V       = %d mV\n", volt.voltage_14);
    printf("1.05V      = %d mV\n", volt.voltage_15);
    printf("1.0V       = %d mV\n", volt.voltage_16);
    printf("0.9V       = %d mV\n", volt.voltage_17);
    printf("0.85V      = %d mV\n", volt.voltage_18);
    printf("0.6V_CPCPU = %d mV\n", volt.voltage_19);
    printf("0.6V_DPCPU = %d mV\n", volt.voltage_20);

    return;
}


/*******************************************************************************
 *
 * Function   : nios_mbox_n2h_parm_show_temp
 * Description: Show the temperature information in NIOS mailbox n2h_parm.
 * Inputs     : temp - n2h_parm temp
 * Outputs    : None.
 *
 *******************************************************************************
 */
static void nios_mbox_n2h_parm_show_temp(temperature_t temp)
{
    printf("\nTemperatures:\n");
    printf("Inlet 1 Temperature = %d Celcius\n", temp.inlet_1_temp);
    printf("Inlet 2 Temperature = %d Celcius\n", temp.inlet_2_temp);
    printf("Outlet 1 Temperature = %d Celcius\n", temp.outlet_1_temp);
    printf("Outlet 2 Temperature = %d Celcius\n", temp.outlet_2_temp);
    printf("Working Temperature = %d Celcius\n", temp.working_temp);
    printf("CP CPU Temperature = %d Celcius\n", temp.cpu_core_temp);
    printf("CP PCH Temperature = %d Celcius\n", temp.cpu_pch_temp);
    printf("DP CP Temperature = %d Celcius\n", temp.dpcpu_core_temp);
    printf("CP DDR0 Temperature = %d Celcius\n", temp.cpu_ddr0_temp);
    printf("CP DDR1 Temperature = %d Celcius\n", temp.cpu_ddr1_temp);

    return;
}


/*******************************************************************************
 *
 * Function   : nios_mbox_n2h_parm_show_fan
 * Description: Show the FAN information in NIOS mailbox n2h_parm.
 * Inputs     : fan - n2h_parm fan
 * Outputs    : None.
 *
 *******************************************************************************
 */
static void nios_mbox_n2h_parm_show_fan(fan_t fan)
{
    printf("\nFans:\n");
	printf("Fan 1 (RPM) = %d\n", fan.fan1);
    printf("Fan 2 (RPM) = %d\n", fan.fan2);
    printf("Fan 3 (RPM) = %d\n", fan.fan3);
    printf("Fan 4 (RPM) = %d\n", fan.fan4);

    return;
}


/*******************************************************************************
 *
 * Function   : nios_mbox_n2h_parm_show_power
 * Description: Show the power information in NIOS mailbox n2h_parm.
 * Inputs     : power - n2h_parm power
 * Outputs    : None.
 *
 *******************************************************************************
 */
static void nios_mbox_n2h_parm_show_power(power_t power)
{
    printf("\nPower:\n");
    printf("Power to Motherboard (mW) = %d\n", power.motherboard);
    printf("Power to NIMs (mW) = %d\n", power.nim);
    printf("Power to Fans (mW) = %d\n", power.fan);
    printf("Power to CP CPU (mW) = %d\n", power.cpu); /* For Future Use */
    printf("Power to CP CPU DDR4 (mW) = %d\n", power.cpu_ddr4);
    printf("Power to DP CPU (mW) = %d\n", power.dp_cpu);
    printf("Power to DP CPU DDR4 (mW) = %d\n", power.dp_ddr4);

    return;
}


/*******************************************************************************
 *
 * Function   : nios_mbox_n2h_parm_show_misc_env
 * Description: Show the misc environment information in NIOS mailbox n2h_parm.
 * Inputs     : misc_env - n2h_parm misc_env
 * Outputs    : None.
 *
 *******************************************************************************
 */
static void nios_mbox_n2h_parm_show_misc_env(misc_env_t misc_env)
{
    printf("\nMiscellaneous Sensor Data:\n");
    printf("Pressure (kPA) = %d\n", misc_env.pressure);

    return;
}


/*******************************************************************************
 *
 * Function   : nios_mbox_n2h_parm_show_fan_ctrl
 * Description: Show the fan control information in NIOS mailbox n2h_parm.
 * Inputs     : fan_ctrl - n2h_parm fan_ctrl
 * Outputs    : None.
 *
 *******************************************************************************
 */
static void nios_mbox_n2h_parm_show_fan_ctrl(fan_control_t fan_ctrl)
{
    uchar fan_tbl = fan_ctrl.fan_table;
    uchar temp_lvl = fan_ctrl.fan_temp_level;
    uchar barom_lvl = fan_ctrl.fan_barom_level;
    uchar fan_tbl_data;
    fan_tbl_data = fan_ctrl.fan_table_data[fan_tbl][barom_lvl][temp_lvl];
    int ix;

    printf("\nFan Controller Info:\n");
    printf("Status = 0x%X\n", fan_ctrl.status); // Future Use
    printf("Fan table = 0x%X\n", fan_tbl);
    printf("Temperature Level = 0x%X\n", temp_lvl);
    printf("Altitude Level = 0x%X\n", barom_lvl);
    printf("Fan Tray Status = 0x%X\n", fan_ctrl.fan_tray_status);
    printf("Fan Table Data = 0x%X\n", fan_tbl_data);
    printf("override = 0x%X\n", fan_ctrl.override);

    printf("Pressure Thresholds =\n");
    for (ix = 0; ix < FAN_CTRL_PRESSURE_THRESHOLDS_NUM; ix++) {
        printf(" 0x%02X", fan_ctrl.barometer_level[ix]);
    }
    printf("\n");

    printf("Temperature Thresholds =\n");
    for (ix = 0; ix < FAN_CTRL_TEMPERATURE_THRESHOLDS_NUM; ix++) {
        printf(" 0x%02X", fan_ctrl.temperature_level[ix]);
    }
    printf("\n");

    return;
}


/*******************************************************************************
 *
 * Function   : nios_mbox_n2h_parm_show_cookie
 * Description: Show the cookie raw data in NIOS mailbox n2h_parm.
 * Inputs     : cookie - n2h_parm cookie
 * Outputs    : None.
 *
 *******************************************************************************
 */
static void nios_mbox_n2h_parm_show_cookie(dev_cookie_t cookie)
{
    int ix;

    for (ix = 0; ix < NIOS_MBOX_N2H_PARM_COOKIE_SIZE; ix++) {
        printf(" 0x%02X", cookie.data[ix]);
        if (ix%16 == 15) {
            printf("\n");
        }
    }

    return;
}


/*******************************************************************************
 *
 * Function   : nios_mbox_n2h_parm_show_psu_env
 * Description: Show the PSU environment information in NIOS mailbox n2h_parm.
 * Inputs     : psu_env - n2h_parm psu_env
 * Outputs    : None.
 *
 *******************************************************************************
 */
static void nios_mbox_n2h_parm_show_psu_env(psu12v_env_t psu_env)
{
    printf("Input Voltage: %u mV\n", psu_env.in_v);
    printf("Input Current: %u mA\n", psu_env.in_i);
    printf("Input Power: %u mW\n", psu_env.in_pwr);
    printf("12V Output Voltage: %u mV\n", psu_env.out_12v);
    printf("12V Output Current: %u mA\n", psu_env.out_12v_i);
    printf("12V Output Power: %u mW\n", psu_env.out_12v_pwr);
    printf("Temperature 1: %d Celcius\n", psu_env.temp1);
    printf("Temperature 2: %d Celcius\n", psu_env.temp2);
    printf("Temperature 3: %d Celcius\n", psu_env.temp3);
    printf("Fan in RPM: %u\n", psu_env.fan1);

    return;
}


/*******************************************************************************
 *
 * Function   : nios_mbox_n2h_parm_show_poe_env
 * Description: Show the POE PSU environment information in NIOS mailbox
 *              n2h_parm.
 * Inputs     : poe_env - n2h_parm poe_env
 * Outputs    : None.
 *
 *******************************************************************************
 */
static void nios_mbox_n2h_parm_show_poe_env(poe_env_t poe_env)
{
#ifdef PHOENIX
    /* Do nothing */
    printf("Phoenix doesn't have POE\n");
#else
    printf("POE PSU Input Voltage = %u mV\n", poe_env.in_12v);
    printf("POE PSU Input Current = %u mA\n", poe_env.in_12v_i);
    printf("POE PSU Input Power = %u mW\n", poe_env.in_pwr);
    printf("POE PSU Output Power = %u mW\n", poe_env.out_pwr);
    printf("POE PSU Temperature = %d Celcius\n", poe_env.temp1);
#endif

    return;
}


/*******************************************************************************
 *
 * Function   : nios_mbox_n2h_parm_show_psu_status
 * Description: Show the PSU/POE PSU status in NIOS mailbox n2h_parm.
 * Inputs     : *n2h_parm - n2h_parm pointer
 * Outputs    : None.
 *
 *******************************************************************************
 */
static void nios_mbox_n2h_parm_show_psu_status(n2h_parm_t *n2h_parm)
{
    printf("\nPSU/POE PSU Status: 1 - Installed, 0 - Not intstalled\n");
    printf("PSU 0 Status: 0x%X\n", n2h_parm->psu0_status);
    printf("PSU 1 Status: 0x%X\n", n2h_parm->psu1_status);
    printf("POE PSU 0 Status: 0x%X\n", n2h_parm->poe0_status);
    printf("POE PSU 1 Status: 0x%X\n", n2h_parm->poe1_status);

    return;
}


/*******************************************************************************
 *
 * Function   : nios_mbox_n2h_parm_show_pwr_dbg_data
 * Description: Show the power debug data in NIOS mailbox n2h_parm.
 * Inputs     : *n2h_parm - n2h_parm pointer
 * Outputs    : None.
 *
 *******************************************************************************
 */
static void nios_mbox_n2h_parm_show_pwr_dbg_data(n2h_parm_t *n2h_parm)
{
    printf("\nPower Debug Data:\n");
    printf("Number of times PSU power exceeded: %u\n", n2h_parm->power_exceed_psu);
    printf("Number of times Motherboard power exceeded: %u\n", n2h_parm->power_exceed_mb);
    printf("Number of times Fan power exceeded: %u\n", n2h_parm->power_exceed_fan);
    printf("Number of times NIM power exceeded: %u\n", n2h_parm->power_exceed_nim);

    return;
}


/*******************************************************************************
 *
 * Function   : nios_mbox_n2h_parm_show_smartfan_data
 * Description: Show the Cisco smartfan data in NIOS mailbox n2h_parm.
 * Inputs     : *smartfan_data - n2h_parm smartfan_data pointer
 * Outputs    : None.
 *
 *******************************************************************************
 */
static void nios_mbox_n2h_parm_show_smartfan_data(csfan_data_inv_t *smartfan_data)
{
    csfan_data_inv_t *smartfan = smartfan_data;
    int ix;

    for (ix = 0; ix < SMARTFAN_DATA_ARRAY_SIZE; ix++) {
        printf("\nSmartFan %d\n", ix+1);
        printf("SmartFan CPN: %s\n", smartfan[ix].cisco_cpn);
        printf("SmartFan Cisco Serial Number: %s\n", smartfan[ix].cisco_serial_number);
        printf("SmartFan CPN Revision: %s\n", smartfan[ix].cisco_rev);
        printf("SmartFan Manufacturer: %s\n", smartfan[ix].mfg_name);
        printf("SmartFan Manufacturer Part Number: %s\n", smartfan[ix].mfg_pn);
        printf("SmartFan Manufacturer Assembly Location: %s\n", smartfan[ix].mfg_assy_location);
        printf("SmartFan Manufacturer Date Code: %s\n", smartfan[ix].mfg_datecode);
        printf("SmartFan Manufacturer Serial Number: %s\n", smartfan[ix].mfg_sn);
    }

    return;
}


/*******************************************************************************
 *
 * Function   : nios_mbox_n2h_parm_show_menu
 * Description: Show the menu of n2h_parm
 * Inputs     : *n2h_parm - n2h_parm pointer
 * Outputs    : None.
 *
 *******************************************************************************
 */
static void nios_mbox_n2h_parm_show_menu(n2h_parm_t *n2h_parm)
{
    int opt;
    int isQuit = 0;

    while (!isQuit) {
        opt = getdec_answer("\n 0: Exit"
                            "\n 1: Voltage\n 2: Temperature\n 3: Fan\n 4: Power"
                            "\n 5: Miscellaneous Sensor Data\n 6: Fan Controller"
                            "\n 7: PSU Cookie raw data\n 8: PSU Environmental Data"
                            "\n 9: POE PSU Environmental Data"
                            "\n10: PSU Status\n11: Power Debug Data"
                            "\n12: Cisco Smartfan"
                            "\nEnter (0 ~ 12): ", 1, 0, 12);

        switch (opt) {
            case 0:
                return;

            case 1:
                nios_mbox_n2h_parm_show_volt(n2h_parm->volt);
                break;

            case 2:
                nios_mbox_n2h_parm_show_temp(n2h_parm->temp);
                break;

            case 3:
                nios_mbox_n2h_parm_show_fan(n2h_parm->fan);
                break;

            case 4:
                nios_mbox_n2h_parm_show_power(n2h_parm->power);
                break;

            case 5:
                nios_mbox_n2h_parm_show_misc_env(n2h_parm->misc_env);
                break;

            case 6:
                nios_mbox_n2h_parm_show_fan_ctrl(n2h_parm->fan_ctrl);
                break;

            case 7:
                printf("\nPSU 0 cookie raw data:\n");
                nios_mbox_n2h_parm_show_cookie(n2h_parm->psu0_cookie);
                printf("\nPSU 1 cookie raw data:\n");
                nios_mbox_n2h_parm_show_cookie(n2h_parm->psu1_cookie);
                break;

            case 8:
                printf("\nPSU 0 environmental data:\n");
                nios_mbox_n2h_parm_show_psu_env(n2h_parm->psu0_env);
                printf("\nPSU 1 environmental data:\n");
                nios_mbox_n2h_parm_show_psu_env(n2h_parm->psu1_env);
                break;

            case 9:
                printf("\nPOE 0 environmental data:\n");
                nios_mbox_n2h_parm_show_poe_env(n2h_parm->poe0_env);
                printf("\nPOE 1 environmental data:\n");
                nios_mbox_n2h_parm_show_poe_env(n2h_parm->poe1_env);
                break;

            case 10:
                nios_mbox_n2h_parm_show_psu_status(n2h_parm);
                break;

            case 11:
                nios_mbox_n2h_parm_show_pwr_dbg_data(n2h_parm);
                break;

            case 12:
                nios_mbox_n2h_parm_show_smartfan_data(n2h_parm->smartfan_data);
                break;

            default:
                printf("Error: Invalid option !!\n");
        }
    }

    return;
}


/*******************************************************************************
 *
 * Function   : nios_mbox_n2h_parm_dump_all
 * Description: Dump all of n2h_parm.
 * Inputs     : *n2h_parm - n2h_parm pointer
 * Outputs    : None.
 *
 *******************************************************************************
 */
static void nios_mbox_n2h_parm_dump_all(n2h_parm_t *n2h_parm)
{
    nios_mbox_n2h_parm_show_volt(n2h_parm->volt);

    nios_mbox_n2h_parm_show_temp(n2h_parm->temp);

    nios_mbox_n2h_parm_show_fan(n2h_parm->fan);

    nios_mbox_n2h_parm_show_power(n2h_parm->power);

    nios_mbox_n2h_parm_show_misc_env(n2h_parm->misc_env);

    nios_mbox_n2h_parm_show_fan_ctrl(n2h_parm->fan_ctrl);

    printf("\nPSU 0 cookie raw data:\n");
    nios_mbox_n2h_parm_show_cookie(n2h_parm->psu0_cookie);
    printf("\nPSU 1 cookie raw data:\n");
    nios_mbox_n2h_parm_show_cookie(n2h_parm->psu1_cookie);

    printf("\nPSU 0 environmental data:\n");
    nios_mbox_n2h_parm_show_psu_env(n2h_parm->psu0_env);
    printf("\nPSU 1 environmental data:\n");
    nios_mbox_n2h_parm_show_psu_env(n2h_parm->psu1_env);

    printf("\nPOE 0 environmental data:\n");
    nios_mbox_n2h_parm_show_poe_env(n2h_parm->poe0_env);
    printf("\nPOE 1 environmental data:\n");
    nios_mbox_n2h_parm_show_poe_env(n2h_parm->poe1_env);

    nios_mbox_n2h_parm_show_psu_status(n2h_parm);

    nios_mbox_n2h_parm_show_pwr_dbg_data(n2h_parm);

    nios_mbox_n2h_parm_show_smartfan_data(n2h_parm->smartfan_data);

    return;
}


/*******************************************************************************
 *
 * Function   : nios_mbox_n2h_parm_show
 * Description:
 * Inputs     : None.
 * Outputs    : None.
 *
 *******************************************************************************
 */
static void nios_mbox_n2h_parm_show(int mode)
{
    unsigned long addr = get_platform_nios_mailbox_msg_base();
    n2h_parm_t *n2h_parm = (n2h_parm_t *)(addr + NIOS_MBOX_N2H_PARM);

    switch (mode) {
        case NIOS_MBOX_SHOW_MENU_MODE:
            nios_mbox_n2h_parm_show_menu(n2h_parm);
            break;

        case NIOS_MBOX_SHOW_DUMP_MODE:
            nios_mbox_n2h_parm_dump_all(n2h_parm);
            break;

        default:
            printf("%s(): Invalid mode, 0x%X\n", __func__, mode);
    }

    return;
}


/*******************************************************************************
 *
 * Function   : nios_mbox_offset_lookup
 * Description: Function to lookup NIOS mailbox message via offset.
 * Inputs     : offset
 *              *tbl
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int nios_mbox_offset_lookup(unsigned int offset, nios_mbox_tbl_t* tbl)
{
    nios_mbox_tbl_t *tmp = &nios_mbox_table[0];

    while (tmp->offset != NIOS_MBOX_NOT_DEFINED) {
        if (offset == tmp->offset) {
            *tbl = *tmp;
            return (PASSED);
        }
        tmp++;
    }

    return (FAILED);
}


/*******************************************************************************
 *
 * Function   : nios_mbox_show_all_offset
 * Description: Function to show all available offsets of NIOS mailbox messages.
 * Inputs     : None
 * Outputs    : None
 *
 *******************************************************************************
 */
void nios_mbox_show_all_offset(void)
{
    nios_mbox_tbl_t *tmp = &nios_mbox_table[0];

    printf("\nOffset\t Name\n");
    printf("-----------------------------------------\n");
    while (tmp->offset != NIOS_MBOX_NOT_DEFINED) {
        if (tmp->type == NIOS_MBOX_NOT_SUPPORT) {
            printf("0x%X\t %s (Not support parsing)\n",
                   tmp->offset, tmp->strName);
            tmp++;
            continue;
        }
        printf("0x%X\t %s\n", tmp->offset, tmp->strName);
        tmp++;
    }

    return;
}


/*******************************************************************************
 *
 * Function   : nios_mbox_show_parse_msg
 * Description: Function to parse the NIOS mailbox message.
 * Inputs     : tbl - the message we want to show in nios_mbox_table.
 *              mode - 0x0: NIOS_MBOX_SHOW_MENU_MODE
 *                     0x1: NIOS_MBOX_SHOW_DUMP_MODE
 * Outputs    : None.
 *
 *******************************************************************************
 */
static void nios_mbox_show_parse_msg(nios_mbox_tbl_t tbl, int mode)
{
    unsigned int size = 1;
    unsigned int type = tbl.type;
    unsigned int offset = tbl.offset;

    if (type == NIOS_MBOX_NOT_SUPPORT) {
        goto err_not_support;
    }

    if (type & NIOS_MBOX_DT_ARRAY) {
        size = tbl.ar_size;
        type &= (~NIOS_MBOX_DT_ARRAY);
    }

    printf("\n%s (0x%X)\n", tbl.strName, offset);

    switch (type) {
        case NIOS_MBOX_DT_INT8:
            nios_mbox_int8_show(offset, size);
            break;

        case NIOS_MBOX_DT_INT16:
            nios_mbox_int16_show(offset, size);
            break;

        case NIOS_MBOX_DT_INT32:
            nios_mbox_int32_show(offset, size);
            break;

        case NIOS_MBOX_DT_STRUCT:
            if (offset == NIOS_MBOX_N2H_PARM) {
                nios_mbox_n2h_parm_show(mode);
            } else {
                goto err_not_support;
            }
            break;

        default:
            goto err_not_support;
    }

    printf("\n\n");
    return;

err_not_support:
    printf("This offset parsing is NOT supported.\n");
    return;
}


/*******************************************************************************
 *
 * Function   : nios_mbox_show_message
 * Description: Function to show NIOS mailbox message via offset.
 * Inputs     : offset
 * Outputs    : None
 *
 *******************************************************************************
 */
void nios_mbox_show_message(unsigned int offset)
{
    nios_mbox_tbl_t tbl;

    if (nios_mbox_offset_lookup(offset, &tbl) != PASSED) {
        printf("Error: Invalid offset !!\n");
        goto error_exit;
    }

    nios_mbox_show_parse_msg(tbl, NIOS_MBOX_SHOW_MENU_MODE);

error_exit:
    return;
}


/*******************************************************************************
 *
 * Function   : nios_mbox_show_message_all
 * Description: Function to show NIOS mailbox message of all offsets.
 * Inputs     : None
 * Outputs    : None
 *
 *******************************************************************************
 */
void nios_mbox_show_message_all(void)
{
    nios_mbox_tbl_t *tmp = &nios_mbox_table[0];

    while (tmp->offset != NIOS_MBOX_NOT_DEFINED) {
        if (tmp->type == NIOS_MBOX_NOT_SUPPORT) {
            printf("0x%X\t %s: Not support parsing\n",
                   tmp->offset, tmp->strName);
            tmp++;
            continue;
        }
        nios_mbox_show_parse_msg(*tmp, NIOS_MBOX_SHOW_DUMP_MODE);
        tmp++;
    }

    return;
}
