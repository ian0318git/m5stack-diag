/* $Id: platform_sensor.c,v 1.4 2019/01/18 05:54:47 yungchen Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/platform_sensor.c,v $
 *------------------------------------------------------------------
 * Filename:  platform_sensor.c
 *
 * Description: TSN Maxim 31730 Diode Sensor I2C device.
 *
 * Copyright (c) 2016 ~ 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <string.h>
#include "common.h"
#include "proto.h"
#include "error.h"
#include "platform_i2c.h"
#include "menu.h"
#include "i2c_address.h"
#include "nvmonvars.h"
#include "platform_fpga.h"
#include "platform_fru.h"
#include "platform_cookie.h"
#include "i2c_api.h"
#include "dev_object.h"
#include "platform_sensor.h"
#include "plat_defs.h"

static boolean wifi_temp_mode = FALSE;

/* Function prototypes */
static int max31730_read_current_temp(uint32_t);
static int read_offset(int);
int snsr_read(n2g_i2c_if_t *);

/* Max31730 registers table. This device is command based. Register offset is
 * the command written to the device.
 */
static reg_info_t sensor_reg_table[] = {
    {"Configuration register", MAX31730_CMD_RCL, READ_WRITE,
     {0}, 0xFF, 0x10},

    {"Current local temperature", MAX31730_CMD_RLTS_MSB, READ_ONLY,
     {0}, 0xFF, 0x00},

    {"Current remote 1 temperature", MAX31730_CMD_RRTE_MSB_1, READ_ONLY,
     {0}, 0xFF, 0x00},

    {"Current remote 2 temperature", MAX31730_CMD_RRTE_MSB_2, READ_ONLY,
     {0}, 0xFF, 0x00},

    {"Current remote 3 temperature", MAX31730_CMD_RRTE_MSB_3, READ_ONLY,
     {0}, 0xFF, 0x00},

    {"Local THIGH limit", MAX31730_CMD_RLHI_MSB, READ_WRITE,
     {0}, 0xFF, 0x00},

    {"Remote 1 THIGH limit", MAX31730_CMD_RRHI_MSB_1, READ_WRITE,
     {0}, 0xFF, 0x00},

    {"Remote 2 THIGH limit", MAX31730_CMD_RRHI_MSB_2, READ_WRITE,
     {0}, 0xFF, 0x00},

    {"Remote 3 THIGH limit", MAX31730_CMD_RRHI_MSB_3, READ_WRITE,
     {0}, 0xFF, 0x00},

    {"All channels TLOW limit", MAX31730_CMD_RALI_MSB, READ_WRITE,
     {0}, 0xFF, 0x00},

    {"Manufacturer ID code", MAX31730_CMD_MFGID, READ_ONLY,
     {0}, 0xFF, 0x4D},

    {"Revision code", MAX31730_CMD_REVID, READ_ONLY,
     {0}, 0xFF, 0x01},

    {0, 0, 0, {0}, 0, 0},
};

/*
 * Diode Sensor Menu
 */
static submenu_xtable_t snsr_menu_table[] = {
    {"Register test", (PFT) max31730_register_test, 0,
     MM_3, (type_t(*)())0, 0, (PFT) 0, 0},
    {"Interrupt test", (PFT) mb_int_test, 0,
     MM_3, (type_t(*)())0, 0, (PFT) 0, 0},
    {"Show Max31730 registers", (PFT) show_snsr_reg, 0,
     MM_1, (type_t(*)())0, 0, (PFT) 0, 0},
    {"Alter Max31730 register", (PFT) alter_snsr_reg, 0,
     MM_1, (type_t(*)())0, 0, (PFT) 0, 0},
    {"Fixed Local THIGH limit", (PFT) set_threshold, 0,
     MM_1, (type_t(*)())0, 0, (PFT) 0, 0},
    {"Fixed Remote 1 THIGH limit", (PFT) set_threshold, 1,
     MM_1, (type_t(*)())0, 0, (PFT) 0, 0},
    {"Fixed Remote 2 THIGH limit", (PFT) set_threshold, 2,
     MM_1, (type_t(*)())0, 0, (PFT) 0, 0},
    {"Fixed Remote 3 THIGH limit", (PFT) set_threshold, 3,
     MM_1, (type_t(*)())0, 0, (PFT) 0, 0},
    {"Fixed All Channels TLOW limit", (PFT) set_threshold, 4,
     MM_1, (type_t(*)())0, 0, (PFT) 0, 0},
    {"Read offset", (PFT) read_offset, 0,
     MM_1, (type_t(*)())0, 0, (PFT) 0, 0},

};

#define SNSR_MENU_TABLE_SIZE (sizeof(snsr_menu_table)/sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t snsr_menu_primary_items[SNSR_MENU_TABLE_SIZE +
                                       MAX_BASE_ITEMS];
static mitem_t snsr_menu_secondary_items[SNSR_MENU_TABLE_SIZE +
                                         MAX_BASE_ITEMS];

static struct menuinfo snsrdiag = {
    "Temp. Sensor(MAX31730) Utility Menu",      /* title */
    0,                          /* title string */
    (PFT) menu_show_dflags,     /* shows major flags */
    0,                          /* generic prompt */
    0,                          /* size */
    snsr_menu_primary_items,
};

static struct menuinfo *snsrdiagp = &snsrdiag;

/*
 * Diode Wifi Sensor Menu
 */
static submenu_xtable_t wifi_snsr_menu_table[] = {
    {"Interrupt test", (PFT) wifi_int_test, 0,
     MM_3, (type_t(*)())0, 0, (PFT) 0, 0},
    {"Show Max31730 registers", (PFT) show_snsr_reg, 0,
     MM_1, (type_t(*)())0, 0, (PFT) 0, 0},
    {"Alter Max31730 register", (PFT) alter_snsr_reg, 0,
     MM_1, (type_t(*)())0, 0, (PFT) 0, 0},
    {"Fixed Local THIGH limit", (PFT) set_threshold, 0,
     MM_1, (type_t(*)())0, 0, (PFT) 0, 0},
    {"Fixed Remote 1 THIGH limit", (PFT) set_threshold, 1,
     MM_1, (type_t(*)())0, 0, (PFT) 0, 0},
    {"Fixed Remote 2 THIGH limit", (PFT) set_threshold, 2,
     MM_1, (type_t(*)())0, 0, (PFT) 0, 0},
    {"Fixed All Channels TLOW limit", (PFT) set_threshold, 4,
     MM_1, (type_t(*)())0, 0, (PFT) 0, 0},
    {"Read offset", (PFT) read_offset, 0,
     MM_1, (type_t(*)())0, 0, (PFT) 0, 0},

};

#define WIFI_SNSR_MENU_TABLE_SIZE (sizeof(wifi_snsr_menu_table)/sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t wifi_snsr_menu_primary_items[WIFI_SNSR_MENU_TABLE_SIZE +
                                       MAX_BASE_ITEMS];
static mitem_t wifi_snsr_menu_secondary_items[WIFI_SNSR_MENU_TABLE_SIZE +
                                         MAX_BASE_ITEMS];

static struct menuinfo wifi_snsrdiag = {
    "Wifi Temp. Sensor(MAX31730) Utility Menu",      /* title */
    0,                          /* title string */
    (PFT) menu_show_dflags,     /* shows major flags */
    0,                          /* generic prompt */
    0,                          /* size */
    wifi_snsr_menu_primary_items,
};

static struct menuinfo *wifi_snsrdiagp = &wifi_snsrdiag;
static double temperature;


/*******************************************************************************
 *
 * Function   : build_snsr_menu
 * Description: To build Temperature Sensor (MAX31730) menu.
 * Inputs     : Test/Menu 
 * Outputs    : None
 *
 *******************************************************************************
 */
int build_snsr_menu(boolean mb_temp_test_items_executed)
{
    char *tname = "M/B Temperature Sensor";
    wifi_temp_mode = FALSE;
    testname(tname);

    build_primary_submenu(snsr_menu_table, SNSR_MENU_TABLE_SIZE,
                          "Diode Sensor Utility Menu", &snsrdiagp);
    build_secondary_submenu(snsr_menu_table, SNSR_MENU_TABLE_SIZE,
                            snsr_menu_secondary_items);
    if (mb_temp_test_items_executed) {
        menu(&snsrdiag, snsr_menu_secondary_items, 0);
    } else {
        do_all_menu_items(snsrdiagp);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : build_wifi_snsr_menu
 * Description: To build Wifi Temperature Sensor (MAX31730) menu.
 * Inputs     : None
 * Outputs    : None
 *
 *******************************************************************************
 */
int build_wifi_snsr_menu (boolean items_executed)
{
    char *tname = "Wifi Temperature Sensor";
    if (tsn_fpga_check_dev_present(FPGA_CPP_WLAN_PRESENT) != TRUE) {
        printf("\n Wifi not present \n");
        return (PASSED);
    }
    wifi_temp_mode = TRUE;
    testname(tname);
    build_primary_submenu(wifi_snsr_menu_table, WIFI_SNSR_MENU_TABLE_SIZE,
                          "Diode Sensor Utility Menu", &wifi_snsrdiagp);
    build_secondary_submenu(wifi_snsr_menu_table, WIFI_SNSR_MENU_TABLE_SIZE,
                            wifi_snsr_menu_secondary_items);
    if (items_executed) {    
        menu(&wifi_snsrdiag, wifi_snsr_menu_secondary_items, 0);
    } else {
        do_all_menu_items(wifi_snsrdiagp);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : read_offset 
 * Description: Read sensor offset address
 * Inputs     : select
 * Outputs    : None
 *
 *******************************************************************************
 */
static int read_offset (int select)
{

    n2g_i2c_if_t *i2c_if;
    uint32_t rc;
    uint8_t old_data = 0, tmp;

    /*
     * init i2c_if for I2C
     */
    if (wifi_temp_mode == TRUE) { /* Wifi */
        if (this_is_star() || this_is_supernova()) {
            i2c_if = (n2g_i2c_if_t *) get_n2g_i2c_if(I2C_CTRL_TWO, I2C_MUX_ZERO,
                                             WIFI_I2C_STAR_ADDR_TEMP);
        } else {
        i2c_if = (n2g_i2c_if_t *)get_n2g_i2c_if(I2C_CTRL_TWO, I2C_MUX_ZERO,
                                             WIFI_I2C_ADDR_TEMP);
        }                                                                                  
    } else {
        i2c_if = (n2g_i2c_if_t *)get_n2g_i2c_if(I2C_CTRL_ONE, I2C_MUX_ZERO,
                                             MB_I2C_ADDR_MB_TEMP);
    }

    if (i2c_if == NULL) {
        printf("%s: Failed to get I2C interface structure.\n",
               __FUNCTION__);
        return (FAILED);
    }

    tmp = gethex_answer("Enter the data:", old_data, 0, 0xFF);

    i2c_if->buf = (char *) &old_data;

    i2c_if->offset = tmp;
    rc = snsr_read(i2c_if);
    msleep(10);

    printf("@ %#x = 0x%02X \n", i2c_if->offset, old_data);

    if (rc == FAILED) {
        printf("%s: Failed to I2C read on %#x.",
              __FUNCTION__, i2c_if->offset);
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : set_threshold_x 
 * Description: set sensor threshold 
 * Inputs     : select/mode/change_value
 * Outputs    : None
 *
 *******************************************************************************
 */
int set_threshold_x (int select, int mode, int change_value)
{

    n2g_i2c_if_t *i2c_if;
    uint32_t rc = FAILED;
    uint8_t old_data, new_data;
    char *tname = "Fixed limit";
    if (mode == ALTER_M) {
        testname(tname);
        prpass(testpass, "Fixed sensor limit, ");
    }

    /*
     * init i2c_if for I2C
     */
    if (wifi_temp_mode == TRUE) { /* Wifi */
        if (this_is_star() || this_is_supernova()) {
            i2c_if = (n2g_i2c_if_t *) get_n2g_i2c_if(I2C_CTRL_TWO, I2C_MUX_ZERO,
                                             WIFI_I2C_STAR_ADDR_TEMP);
        } else {
        i2c_if = (n2g_i2c_if_t *) get_n2g_i2c_if(I2C_CTRL_TWO, I2C_MUX_ZERO,
                                             WIFI_I2C_ADDR_TEMP);
        }                                                                                  
    } else {
        i2c_if = (n2g_i2c_if_t *) get_n2g_i2c_if(I2C_CTRL_ONE, I2C_MUX_ZERO,
                                             MB_I2C_ADDR_MB_TEMP);
    }

    if (i2c_if == NULL) {
        printf("%s: Failed to get I2C interface structure.\n",
               __FUNCTION__);
        return (FAILED);
    }

    switch (select) {
    case 0:
        i2c_if->offset = MAX31730_CMD_RLHI_MSB;
        break;
    case 1:
        i2c_if->offset = MAX31730_CMD_RRHI_MSB_1;
        break;
    case 2:
        i2c_if->offset = MAX31730_CMD_RRHI_MSB_2;
        break;
    case 3:
        i2c_if->offset = MAX31730_CMD_RRHI_MSB_3;
        break;
    case 4:
        i2c_if->offset = MAX31730_CMD_RALI_MSB;
        break;

    default:
        printf("not support this threhold \n");
        break;
    }

    i2c_if->buf = (char *) &old_data;
    rc = snsr_read(i2c_if);
    if (rc == FAILED) {
        cterr('f', 0, "%s: Failed to I2C read on %#x.",
              __FUNCTION__, i2c_if->offset);
        return (FAILED);
    }
    if (mode == ALTER_M) {
        new_data = gethex_answer("Enter the data (MSB):", old_data, 0x0, 0xFF);
    } else {
        new_data = change_value; 
        printf("new_data %d", change_value); 
    }
    i2c_if->buf = (char *) &new_data;

    /*
     * Setup I2C API interface struct
     */
    i2c_if->size = sizeof(uint8_t);

    rc = n2g_i2c_write(i2c_if);
    if (rc == FAILED) {
        printf("n2g_i2c_write() on %s failed \n", __FUNCTION__);
        return (FAILED);
    }


    switch (select) {
    case 0:
        i2c_if->offset = MAX31730_CMD_RLHI_LSB;
        break;
    case 1:
        i2c_if->offset = MAX31730_CMD_RRHI_LSB_1;
        break;
    case 2:
        i2c_if->offset = MAX31730_CMD_RRHI_LSB_2;
        break;
    case 3:
        i2c_if->offset = MAX31730_CMD_RRHI_LSB_3;
        break;
    case 4:
        i2c_if->offset = MAX31730_CMD_RALI_LSB;
        break;

    default:
        printf("not support this threhold \n");
        break;
    }


    i2c_if->buf = (char *) &old_data;
    rc = snsr_read(i2c_if);
    if (rc == FAILED) {
        cterr('f', 0, "%s: Failed to I2C read on %#x.",
              __FUNCTION__, i2c_if->offset);
        return (FAILED);
    }
    if ( mode == ALTER_M) {
        new_data = gethex_answer("Enter the data (LSB):", old_data, 0, 0xF0);
    } else {
        /* TEST_M no alter LSB */
        return (PASSED); 
    }
    i2c_if->buf = (char *) &new_data;
    rc = n2g_i2c_write(i2c_if);
    if (rc == FAILED) {
        printf("n2g_i2c_write() on %s failed \n", __FUNCTION__);
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : set_threshold 
 * Description: set sensor threshold as ALTER_M
 * Inputs     : select corresponding sensor
 * Outputs    : None
 *
 *******************************************************************************
 */
int set_threshold(int select)
{
    return (set_threshold_x(select, ALTER_M, 0));
}

/*******************************************************************************
 *
 * Function   : show_snsr_reg
 * Description: Display Temperature Sensor(MAX31730) Registers.
 * Inputs     : NONE
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int show_snsr_reg (void)
{
    n2g_i2c_if_t *i2c_if;
    uint32_t rc;
    reg_info_t *reg_p;
    uint16_t data, cfgbyte;
    uint32 cmd;
    int temper;

    /*
     * init i2c_if for I2C
     */
    if (wifi_temp_mode == TRUE) { /* Wifi */
        if (this_is_star() || this_is_supernova()) {
            i2c_if = (n2g_i2c_if_t *) get_n2g_i2c_if(I2C_CTRL_TWO, I2C_MUX_ZERO,
                                             WIFI_I2C_STAR_ADDR_TEMP);
        } else {
        i2c_if = (n2g_i2c_if_t *) get_n2g_i2c_if(I2C_CTRL_TWO, I2C_MUX_ZERO,
                                             WIFI_I2C_ADDR_TEMP);
        }                                                                                  
    } else {
        i2c_if = (n2g_i2c_if_t *) get_n2g_i2c_if(I2C_CTRL_ONE, I2C_MUX_ZERO,
                                             MB_I2C_ADDR_MB_TEMP);
    }
  
    if (i2c_if == NULL) {
        printf("%s: Failed to get I2C interface structure.\n",
              __FUNCTION__);
        return (FAILED);
    }

    /*
     * Setup I2C API interface struct
     */
    i2c_if->buf = (char *) &data;

    printf("\n%s Registers:\n", i2c_if->dev_name);

    /*
     * Points to the beginning of the registers table
     */
    reg_p = &sensor_reg_table[0];

    /*
     * Read registers value
     */
    while (reg_p->name) {
        if (!(reg_p->type & WRITE_ONLY)) {
            /*
             * Write only regiser is not readable,
             * * so skip them.
             */
            i2c_if->offset = reg_p->offset;
            i2c_if->size = sizeof(data);

            rc = snsr_read(i2c_if);
            if (rc != PASSED) {
                printf("%s: I2C read %s @ %#x rc = %#x.\n",
                      __FUNCTION__, reg_p->name, reg_p->offset, rc);
                return (FAILED);
            }

            /*
             * Display All registers info
             */
            cmd = DEV_SHOW_ALL;

            /*
             * Display Sensor info
             */
            switch (cmd) {
            case DEV_SHOW_ALL:
            case DEV_SHOW_CONFIG:
            case DEV_SHOW_REGISTERS:

                switch (reg_p->offset) {
                case MAX31730_CMD_RLTS_MSB:
                case MAX31730_CMD_RRTE_MSB_1:
                case MAX31730_CMD_RRTE_MSB_2:
                case MAX31730_CMD_RRTE_MSB_3:
                     if ((wifi_temp_mode == TRUE) && 
                         (reg_p->offset == MAX31730_CMD_RRTE_MSB_3)) { /* Wifi */
                         break; 
                     }
                case MAX31730_CMD_RLHI_MSB:
                case MAX31730_CMD_RRHI_MSB_1:
                case MAX31730_CMD_RRHI_MSB_2:
                case MAX31730_CMD_RRHI_MSB_3:
                     if ((wifi_temp_mode == TRUE) && 
                         (reg_p->offset == MAX31730_CMD_RRHI_MSB_3)) { /* Wifi */
                         break; 
                     }
                case MAX31730_CMD_RALI_MSB:
                    i2c_if->offset = reg_p->offset;

                    rc = snsr_read(i2c_if);
                    temper = data;

                    i2c_if->offset = reg_p->offset + 1;
                    rc = snsr_read(i2c_if);

                    temper = (temper << 8) | data;
                    if (temper & 0x8000) {      /* two's complement */
                        temper = ((temper ^ 0xffff) + 1);
                        temper = temper * (-1);
                    }
                    printf("%s is %.4f degrees C.\n",
                           reg_p->name,
                           (cfgbyte) ? (temper / 256.0) +
                           64 : (temper / 256.0));
                    /*
                     * extrange(13h) =1 ==> + 64 degrees C.
                     */
                    break;

                case MAX31730_CMD_MFGID:
                case MAX31730_CMD_REVID:
                    printf("%s is 0x%02x\n", reg_p->name, data);
                    break;
                case MAX31730_CMD_RCL:
                    printf("%s is 0x%02x\n", reg_p->name, data);
                    printf("  - ADC %s.\n",
                           (data & MAX31730_RCL_STOP) ?
                           "is shut down (Standby mode) and reduces supply current to 2.5uA"
                           : "in continuous-conversion mode");
                    printf("  - SMBus timeout %s.\n",
                           (data & MAX31730_RCL_TIMEOUT) ? "disabled" :
                           "enabled");
                    printf("  - %s mode.\n",
                           (data & MAX31730_RCL_INTERRUPT) ? "Comparator" :
                           "Interrupt");
                    printf("  - Entended Range %s.\n",
                           (data & MAX31730_RCL_EXT) ? "enabled" :
                           "disabled");
                    cfgbyte = data & MAX31730_RCL_EXT;
                    break;

                default:
                    printf("%s: Invalid command %02x.\n",
                          __FUNCTION__, reg_p->offset);
                    return (FAILED);
                    break;
                }               /* endof switch(offset) */
                break;
            case DEV_SHOW_BRIEF:
                printf("@ %#x = 0x%02X ", reg_p->offset, data);
                break;
            default:
                assert(!"dev_31730_show");
                break;
            }                   /* endof switch(cmd) */
        }
        /*
         * endof if (WRITE_ONLY)
         */
        reg_p++;                /* Get next register */
    }                           /* endof while */
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : alter_snsr_reg
 * Description: To alter Temperature Sensor (MAX31730) Register.
 * Inputs     : NONE 
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int alter_snsr_reg (void)
{
    n2g_i2c_if_t *i2c_if;
    uint32_t rc;
    reg_info_t *reg_p;
    uint8_t cmd;
    uint8_t old_data, new_data;

    /*
     * init i2c_if for I2C
     */
    if (wifi_temp_mode == TRUE) { /* Wifi */
        if (this_is_star() || this_is_supernova()) {
            i2c_if = (n2g_i2c_if_t *) get_n2g_i2c_if(I2C_CTRL_TWO, I2C_MUX_ZERO,
                                             WIFI_I2C_STAR_ADDR_TEMP);
        } else {
        i2c_if = (n2g_i2c_if_t *) get_n2g_i2c_if(I2C_CTRL_TWO, I2C_MUX_ZERO,
                                             WIFI_I2C_ADDR_TEMP);
        }                                                                                  
    } else {
        i2c_if = (n2g_i2c_if_t *) get_n2g_i2c_if(I2C_CTRL_ONE, I2C_MUX_ZERO,
                                             MB_I2C_ADDR_MB_TEMP);
    }

    if (i2c_if == NULL) {
        printf("%s: Failed to get I2C interface structure.\n",
              __FUNCTION__);
        return (FAILED);
    }

    /*
     * Setup I2C API interface struct
     */
    i2c_if->size = sizeof(uint8_t);

    printf("\nRegister number:\n");

    /*
     * Parse through the register table to search for writeable registers
     */
    reg_p = &sensor_reg_table[0];       /* Points to the beginning of the table */

    while (reg_p->name) {
        if (!(reg_p->type & READ_ONLY)) {
            /*
             * Only Write_only or read_write register can be altered
             */
            printf("   0x%02X - %s\n", reg_p->offset, reg_p->name);
        }
        reg_p++;                /* update the register table pointer */
    }                           /* endof while */

    /*
     * Get the register offset to alter
     */
    cmd = gethex_answer("Enter the register number:", 0, 0,
                      MAX31730_CMD_REVID);

    /*
     * Find the register text in the register table
     */
    reg_p = &sensor_reg_table[0];       /* Points to the beginning of the table */

    while (reg_p->name && reg_p->offset != cmd) {
        /*
         * Not requested register
         */
        reg_p++;                /* update the register table pointer */
    }

    /*
     * Got the read/write pair. Some registers cannot be read, but writeable
     */
    /*
     * Readable registers. Read the register first.
     */
    i2c_if->buf = (char *) &old_data;
    i2c_if->offset = reg_p->offset;  /*** offset for sensor register ??***/

    rc = snsr_read(i2c_if);
    if (rc != PASSED) {
        printf("%s: I2C read %s cmd %#x rc = %#x.\n",
              __FUNCTION__, reg_p->name, reg_p->offset, rc);
        return (FAILED);
    }
    /*
     * Get the new data
     */
    new_data = gethex_answer("Enter the data:", old_data, 0, 0xFF);

    /*
     * Write the new data
     */
    i2c_if->buf = (char *) &new_data;
    i2c_if->offset = cmd;

    printf("\nwrite %#x with cmd %#x\n", new_data, i2c_if->offset);

    rc = n2g_i2c_write(i2c_if);
    if (rc != PASSED) {
        printf("%s: I2C write %s cmd %#x rc = %#x.\n",
              __FUNCTION__, reg_p->name, reg_p->offset, rc);
    }

    return (rc);
}

/*********************************************************************
 *
 * Function:    snsr_read
 *
 * Description:    Local Read Max31730 Register.
 *        Sensor IOFPGA read has 2 I2C operations. The I2C write with
 *        the register offset. Then wait for the REN_I2C_PROC_TIME
 *        milliseconds to allow the Sensor firmware to setup
 *        the data of the requested register. Then the I2C read will
 *        return the data.
 *
 * Inputs:    i2c_if - pointer to the I2C API struct.
 *
 * Outputs:    PASSED/FAILED.
 *
 * Assumptions:
 *
 *********************************************************************
 */
int snsr_read (n2g_i2c_if_t * i2c_if)
{
    n2g_i2c_if_t new_i2c_if;
    uint32_t rc = FAILED;
    char *reg_data;
    char reg_tmp[32];

    reg_data = &reg_tmp[0];

    memcpy(&new_i2c_if, i2c_if, sizeof(new_i2c_if));
    new_i2c_if.buf = reg_data;

    rc = n2g_i2c_read(&new_i2c_if);
    if (rc != PASSED) {
        /*
         * Unable to read data
         */
        printf("%s: Failed to I2C read (rc = 0x%08x).", __FUNCTION__, rc);
        return (rc);
    }

    *((sn_d *) i2c_if->buf) = (*(sn_d *) new_i2c_if.buf);

    return (rc);
}

/*******************************************************************************
 *
 * Function   : max31730_read_current_temp
 * Description:    To read current temperature from MAX31730 Register.
 * Inputs     : tmp_type - to determine local or remote temperature to read
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int max31730_read_current_temp (uint32_t tmp_type)
{
    n2g_i2c_if_t *i2c_if;
    uint32_t rc;
    uint16_t cfgbyte;
    uint8_t data;
    int temper;

    /*
     * init i2c_if for I2C
     */
    if (wifi_temp_mode == TRUE) { /* Wifi */
        if (this_is_star() || this_is_supernova()) {
            i2c_if = (n2g_i2c_if_t *) get_n2g_i2c_if(I2C_CTRL_TWO, I2C_MUX_ZERO,
                                             WIFI_I2C_STAR_ADDR_TEMP);
        } else {
        i2c_if = (n2g_i2c_if_t *) get_n2g_i2c_if(I2C_CTRL_TWO, I2C_MUX_ZERO,
                                             WIFI_I2C_ADDR_TEMP);
        }                                                                                  
    } else {
        i2c_if = (n2g_i2c_if_t *) get_n2g_i2c_if(I2C_CTRL_ONE, I2C_MUX_ZERO,
                                             MB_I2C_ADDR_MB_TEMP);
    }
    if (i2c_if == NULL) {
        printf("%s: Failed to get I2C interface structure.\n",
              __FUNCTION__);
        return (FAILED);
    }

    /*
     * Setup I2C API interface struct
     */
    i2c_if->buf = (char *) &data;
    i2c_if->size = sizeof(data);

    /*
     * Read temperature
     */
    i2c_if->offset = tmp_type;

    rc = snsr_read(i2c_if);
    if (rc != PASSED) {
        printf("%s: Failed to read Temperature from Reg %#x (rc = %#x).",
              __FUNCTION__, i2c_if->offset, rc);
        return (FAILED);
    }

    temper = data;

    i2c_if->offset = tmp_type + 1;
    rc = snsr_read(i2c_if);

    temper = (temper << 8) | data;
    if (temper & 0x8000) {      /* two's complement */
        temper = ((temper ^ 0xffff) + 1);
        temper = temper * (-1);
    }

    /*
     * Read extended-range enable bit
     */
    i2c_if->offset = MAX31730_CMD_RCL;
    rc = snsr_read(i2c_if);

    cfgbyte = data & MAX31730_RCL_EXT;

    if (cfgbyte) {
        /* extrange(13h) =1 ==> + 64 degrees C. */
        printf("%.4f degrees Celsius.\n", ((temper / 256.0) + 64));    
        temperature = (((double)temper / 256.0) + 65);
    } else {
        printf("%.4f degrees Celsius.\n", (temper / 256.0));
        temperature = ((double)temper / 256.0);
    }
    return (PASSED);

}


/*******************************************************************************
 *
 * Function   : max31730_read_local_temp
 * Description: To read temperature from HW address U2402(Near SFP).
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int max31730_read_local_temp (void)
{
    return (max31730_read_current_temp(MAX31730_CMD_RLTS_MSB));
}

/*******************************************************************************
 *
 * Function   : max31730_read_remote_temp1
 * Description: To read temperature from HW address Q2402(Near LEDs).
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int max31730_read_remote_temp1 (void)
{
    return (max31730_read_current_temp(MAX31730_CMD_RRTE_MSB_1));
}

/*******************************************************************************
 *
 * Function   : max31730_read_remote_temp2
 * Description: To read temperature from HW address Q2400(Near DDR).
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int max31730_read_remote_temp2 (void)
{
    return (max31730_read_current_temp(MAX31730_CMD_RRTE_MSB_2));
}

/*******************************************************************************
 *
 * Function   : max31730_read_remote_temp3
 * Description: To read temperature from HW address Q2401(Near Reset Button).
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int max31730_read_remote_temp3 (void)
{
    return (max31730_read_current_temp(MAX31730_CMD_RRTE_MSB_3));
}


/*******************************************************************************
 *  
 * Function   : max31730_read_current_temp_errormsg
 * Description: To read current temperature from MAX31730 Register.
 * Inputs     : tmp_type - to determine local or remote temperature to read
 * Outputs    : PASSED/FAILED
 *      
 *******************************************************************************
 */
static int max31730_read_current_temp_errormsg (uint32_t tmp_type)
{
    n2g_i2c_if_t *i2c_if;
    uint32_t rc;
    uint16_t cfgbyte;
    uint8_t data;
    int temper;
        
    /*  
     * init i2c_if for I2C
     */
    if (wifi_temp_mode == TRUE) { /* Wifi */
        if (this_is_star() || this_is_supernova()) {
            i2c_if = (n2g_i2c_if_t *) get_n2g_i2c_if(I2C_CTRL_TWO, I2C_MUX_ZERO,
                                             WIFI_I2C_STAR_ADDR_TEMP);
        } else {
        i2c_if = (n2g_i2c_if_t *) get_n2g_i2c_if(I2C_CTRL_TWO, I2C_MUX_ZERO,
                                             WIFI_I2C_ADDR_TEMP);
        }                                     
    } else {
        i2c_if = (n2g_i2c_if_t *) get_n2g_i2c_if(I2C_CTRL_ONE, I2C_MUX_ZERO,
                                             MB_I2C_ADDR_MB_TEMP);
    }
    if (i2c_if == NULL) {
        cterr('f', 0, "%s: Failed to get I2C interface structure.",
              __FUNCTION__);
        return (FAILED);
    }

    /*
     * Setup I2C API interface struct
     */
    i2c_if->buf = (char *) &data;
    i2c_if->size = sizeof(data);

    /*
     * Read temperature
     */
    i2c_if->offset = tmp_type;

    rc = snsr_read(i2c_if);
    if (rc != PASSED) {
        cterr('f', 0,
              "%s: Failed to read Temperature from Reg %#x (rc = %#x).",
              __FUNCTION__, i2c_if->offset, rc);
        return (FAILED);
    }

    temper = data;

    i2c_if->offset = tmp_type + 1;
    rc = snsr_read(i2c_if);

    temper = (temper << 8) | data;
    if (temper & 0x8000) {      /* two's complement */
        temper = ((temper ^ 0xffff) + 1);
        temper = temper * (-1);
    }

    /*
     * Read extended-range enable bit
     */
    i2c_if->offset = MAX31730_CMD_RCL;
    rc = snsr_read(i2c_if);

    cfgbyte = data & MAX31730_RCL_EXT;

    if (cfgbyte) {
        /* extrange(13h) =1 ==> + 64 degrees C. */
        cterr_db_print("%.4f degrees Celsius.\n", ((temper / 256.0) + 64));
        temperature = (((double)temper / 256.0) + 65);
    } else {
        cterr_db_print("%.4f degrees Celsius.\n", (temper / 256.0));
        temperature = ((double)temper / 256.0);
    }
    return (PASSED);

}


/*******************************************************************************
 *
 * Function   : max31730_read_local_temp_errormsg
 * Description: To read temperature from HW address U2402(Near SFP) 
 * Inputs     : NULL
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int max31730_read_local_temp_errormsg (void)
{
    return (max31730_read_current_temp_errormsg(MAX31730_CMD_RLTS_MSB));
}

/*******************************************************************************
 *
 * Function   : max31730_read_remote_temp1_errormsg
 * Description: To read temperature from HW address Q2402(Near LEDs).
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int max31730_read_remote_temp1_errormsg (void)
{
    return (max31730_read_current_temp_errormsg(MAX31730_CMD_RRTE_MSB_1));
}

/*******************************************************************************
 *
 * Function   : max31730_read_remote_temp2_errormsg
 * Description: To read temperature from HW address Q2400(Near DDR).
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int max31730_read_remote_temp2_errormsg (void)
{
    return (max31730_read_current_temp_errormsg(MAX31730_CMD_RRTE_MSB_2));
}

/*******************************************************************************
 *
 * Function   : max31730_read_remote_temp3_errormsg
 * Description: To read temperature from HW address Q2401(Near Reset Button).
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int max31730_read_remote_temp3_errormsg (void)
{
    return (max31730_read_current_temp_errormsg(MAX31730_CMD_RRTE_MSB_3));
}

/*******************************************************************************
 *
 * Function   : tsn_display_temp_errormsg
 * Description: Function to print temperature on TSN enhanced error message 
 *              by reading sensor chip MAX31730.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_display_temp_errormsg (void)
{
    int result = FAILED;

    /*
     * Get temperature,
     * for TSN, to read it from MAX31730.
     */
    cterr_db_print("Current local (thermal sensor) temperature is ");
    result = max31730_read_local_temp_errormsg();

    if (result != PASSED) {
        cterr_db_print("N/A.\n");
    }

    cterr_db_print("Current remote 1 (CPU) temperature is ");
    result = max31730_read_remote_temp1_errormsg();

    if (result != PASSED) {
        cterr_db_print("N/A.\n");
    }

    cterr_db_print("Current remote 2 (DDR) temperature is ");
    result = max31730_read_remote_temp2_errormsg();

    if (result != PASSED) {
        cterr_db_print("N/A.\n");
    }

    cterr_db_print("Current remote 3 (SFP) temperature is ");
    result = max31730_read_remote_temp3_errormsg();

    if (result != PASSED) {
        cterr_db_print("N/A.\n");
    }

    return (result);
}

/******************************************************************************
 *
 * Function: max31730_register_test
 *
 * Description: This function performs the max31730 register test.
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int max31730_register_test (void)
{
    uchar mb_get_loc[FRU_SIZE] = {0};
    uchar mb_get_pid[FRU_SIZE] = {0};
#ifdef ENHANCE_ERROR_MSG_RDY
    /*
     * 1. Subtests of the test function will reuse all variables
     * 2. All variables will be cleared automatically when
     *    entering and leaving each menu item.
     */
    /* Segment 1: PID | Unique_string : slot_info */
    fru_table_offset = MB;
    /* fru_table_offset should be set, otherwise, it will not */
    /* go to enhanced error message format in cterr() */
    /* set fru_table_offset to get the predefine value */
    /* or change mb_pid & mb_loc below */
    platform_get_pid((char *)mb_get_pid);
    strcpy((char *)mb_get_loc, "MB");
    platform_fru_table[fru_table_offset].pid_string = mb_get_pid;
    platform_fru_table[fru_table_offset].location_string = mb_get_loc;

    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("Marvell Armada 7040", "I2C", "MB Thermal");

    /* Segment 5: register and memory dump */
    //cterr_add_reg_dump((PFV)bcm63168_get_xdsl_profile);

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)tsn_display_temp_errormsg);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Check the interface between the Host "
                    "SoC and the failed I2C devices.",
                    "If there is no problem for these interfaces, "
                    "replace one I2C device and redo the test.");
#endif

    char *tname = "Temperature Sensor Register";

    testname(tname);
    prpass(testpass, "%s, ", tname);

    if (register_tests(0, sensor_reg_table) == FAILED) {
        cterr('f', 0, "Temperature Sensor Register test failed.");
        return (FAILED);
    } else {
        prpass(testpass, "Temperature Sensor Register ");
    }

    return (PASSED);
}


/******************************************************************************
 *
 * Function: mb_int_test
 *
 * Description: This function performs the motherboard interrupt test.
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int mb_int_test (void)
{
    uchar mb_get_loc[FRU_SIZE] = {0};
    uchar mb_get_pid[FRU_SIZE] = {0};
#ifdef ENHANCE_ERROR_MSG_RDY
    /*
     * 1. Subtests of the test function will reuse all variables
     * 2. All variables will be cleared automatically when
     *    entering and leaving each menu item.
     */
    /* Segment 1: PID | Unique_string : slot_info */
    fru_table_offset = MB;
    /* fru_table_offset should be set, otherwise, it will not */
    /* go to enhanced error message format in cterr() */
    /* set fru_table_offset to get the predefine value */
    /* or change mb_pid & mb_loc below */
    platform_get_pid((char *)mb_get_pid);
    strcpy((char *)mb_get_loc, "MB");
    platform_fru_table[fru_table_offset].pid_string = mb_get_pid;
    platform_fru_table[fru_table_offset].location_string = mb_get_loc;

    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("Marvell Armada 7040", "I2C", "MB Thermal");

    /* Segment 5: register and memory dump */
    //cterr_add_reg_dump((PFV)bcm63168_get_xdsl_profile);

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)tsn_display_temp_errormsg);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Check the interface between the Host "
                    "SoC and the failed I2C devices.",
                    "If there is no problem for these interfaces, "
                    "replace one I2C device and redo the test.");
#endif

    char *tname = "M/B Temperature Interrupt";
    int rc = PASSED;
    double THIGH_local = 0;
    double THIGH_remote1 = 0;
    double THIGH_remote2 = 0;
    double THIGH_remote3 = 0;
    double TLOW = 0;
    uint buf=0;
    long int ix, delay=COUNT200000;   
    uint     reg_offset = 0, reg_val = 0;
    int      test_thr = 0; 

    testname(tname);
    prpass(testpass, "%s, ", tname);

    printf("\nNotes:\n+/- 1 Degree Celsius Remote Temperature-"
              "Measurement Accuracy (0 Deg C to +100 Deg C)\n");

    /* 1. Test Local sensor */
    /* Read current sensor */
    if (max31730_read_local_temp() != PASSED) {
        cterr('f', 0, "Failed to read Local Temp.");
        return (FAILED);
    }

    /* Set temp threshold */
    test_thr = (int)(temperature - DEG10);
    if ((NVRAM)->diagflag & D_VERBOSE) { 
        printf("Local Hight Temp. test thread is %d degreeC.\n", test_thr);
    }
    if (set_threshold_x(0, TEST_M, test_thr) != PASSED) {
        cterr('f', 0, "Failed to set Local High threshold to %d.", test_thr);
        return (FAILED);
    }
    msleep(LENGTH100);

    /* 1st Verify (0x1128) External Interrupt Pending Register */
    for (ix = 1; ix <= delay; ix++) {
        fpga_read_32_reg (FPGA_EXTER_INT_PENDING_REG, &buf);
        if ((buf & MB_THERM_INTERRUPT_PENDING) == MB_THERM_INTERRUPT_PENDING) {
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf(", delay %ld\n", ix);
                printf("\nLocal temp 1st verify - THIGH=%g i=%ld," 
                     "No Local interrupt triggered!\n", THIGH_local, ix);
            } 
            prpass(testpass, "Local interrupt triggered ");
            break;
        }

        if (ix == delay) {  /* return FAILED if consecutive n=10 times failures */
            if ((NVRAM)->diagflag & D_VERBOSE) { 
                printf("\nFAIL@Local temp 1st verify - THIGH=%g i=%ld,"
                     "No Local interrupt triggered!\n", THIGH_local, ix);
            }
            cterr('f', 0, "No local interrupt triggered!");
            return (FAILED);
        }
    }

    /* Restore default temp threshold */
    test_thr = DEFAULT127;
    if (set_threshold_x(0, TEST_M, test_thr) != PASSED) {
        cterr('f', 0, "Failed to set Local High threshold to %d.", test_thr);
        return (FAILED);
    }
    msleep(LENGTH1000);

    /* Clear pending interrupt on FPGA external interrupt pending Reg.(0x1128) */
    reg_offset = (uint)FPGA_EXTER_INT_PENDING_REG;
    reg_val = (uint)MB_THERM_INTERRUPT_PENDING;

    if (fpga_write_32_reg(reg_offset, reg_val) != PASSED) {
        printf("%s: Failed to clear FPGA pending intrrupt(0x%04X: 0x%04X).\n",
               __FUNCTION__, reg_offset, reg_val);
        return (FAILED);
    }
    msleep(LENGTH100);

    /* 2nd Verify (0x1128) External Interrupt Pending Register */
    for (ix = 1; ix <= delay; ix++) {
        fpga_read_32_reg (FPGA_EXTER_INT_PENDING_REG, &buf);
        if ((buf & MB_THERM_INTERRUPT_PENDING) != MB_THERM_INTERRUPT_PENDING) {
            if ((NVRAM)->diagflag & D_VERBOSE) {  
                printf(", delay %ld\n", ix);
                printf("\nLocal temp 2nd verify - THIGH=%g i=%ld," 
                        "No Local interrupt triggered\n", THIGH_local, ix);
            } 
            prpass(testpass, "No Local interrupt triggered ");
            break;
       }
       if (ix == delay) {  /* return FAILED if consecutive n=10 times failures */
           if ((NVRAM)->diagflag & D_VERBOSE) {  
               printf("\nFAIL@Local temp 2nd verify - THIGH=%g i=%ld,"
                      "Local interrupt triggered!\n", THIGH_local, ix);
           }
           cterr('f', 0, "Local interrupt triggered!");
           return (FAILED);
       }
    }

    /* Test remote1 sensor */
    /* Read current remote1 sensor */
    if (max31730_read_remote_temp1() != PASSED) {
        cterr('f', 0, "Failed to read Remote1 Temp.");
        return (FAILED);
    }

    /* Set temp threshold */
    test_thr = (int)(temperature - DEG10);
    if ((NVRAM)->diagflag & D_VERBOSE) { 
        printf("Remote1 Temp. High test thread is %d degreeC.\n", test_thr);
    }
    if (set_threshold_x(1, TEST_M, test_thr) != PASSED) {
        cterr('f', 0, "Failed to set Remote1 High threshold to %d.", test_thr);
        return (FAILED);
    }
    msleep(LENGTH100);

    /* 1st Verify (0x1128) External Interrupt Pending Register */
    for (ix = 1; ix <= delay; ix++) {
        fpga_read_32_reg (FPGA_EXTER_INT_PENDING_REG, &buf);
        if ((buf & MB_THERM_INTERRUPT_PENDING) == MB_THERM_INTERRUPT_PENDING) {
            if ((NVRAM)->diagflag & D_VERBOSE) { 
               printf(", delay %ld\n", ix);
               printf("\nRemote 1 temp 1st verify - THIGH=%g i=%ld," 
                      "Remote 1 interrupt triggered\n", THIGH_remote1, ix);
            }
            prpass(testpass, "Remote 1 interrupt triggered ");
            break;
        }
        if (ix == delay) {  /* return FAILED if consecutive n=10 times failures */
            if ((NVRAM)->diagflag & D_VERBOSE) { 
                printf("\nFAIL@Remote 1 temp 1st verify - THIGH=%g i=%ld," 
                      "No remote 1 interrupt triggered!\n", THIGH_remote1, ix);
            }
            cterr('f', 0, "No remote 1 interrupt triggered!");
            return (FAILED);
        }
    }
    /* Restore default temp threshold */
    test_thr = DEFAULT127;
    if (set_threshold_x(1, TEST_M, test_thr) != PASSED) {
        cterr('f', 0, "Failed to set Remote1 High threshold to %d.", test_thr);
        return (FAILED);
    }
    msleep(LENGTH1000);

    /* Clear pending interrupt on FPGA external interrupt pending Reg.(0x1128) */
    reg_offset = (uint)FPGA_EXTER_INT_PENDING_REG;
    reg_val = (uint)MB_THERM_INTERRUPT_PENDING;
    if (fpga_write_32_reg(reg_offset, reg_val) != PASSED) {
        printf("%s: Failed to clear FPGA pending intrrupt(0x%04X: 0x%04X).\n",
               __FUNCTION__, reg_offset, reg_val);
        return (FAILED);
    }
    msleep(LENGTH100);

    /* 2nd Verify (0x1128) External Interrupt Pending Register */
    for (ix = 1; ix <= delay; ix++) {
        fpga_read_32_reg (FPGA_EXTER_INT_PENDING_REG, &buf);
        if ((buf & MB_THERM_INTERRUPT_PENDING) != MB_THERM_INTERRUPT_PENDING) {
            if ((NVRAM)->diagflag & D_VERBOSE) {  
                printf(", delay %ld\n", ix);
                printf("\nRemote 1 temp 2nd verify - THIGH=%g i=%ld," 
                "No Remote 1 interrupt triggered\n", THIGH_remote1, ix);
            } 
            prpass(testpass, "No Remote 1 interrupt triggered ");
            break;
        }
        if (ix == delay) {  /* return FAILED if consecutive n=10 times failures */
               if ((NVRAM)->diagflag & D_VERBOSE) {  
                   printf("\nFAIL@Remote 1 temp 2nd verify - THIGH=%g i=%ld," 
                      "Remote 1 interrupt triggered!\n", THIGH_remote1, ix);
               } 
               cterr('f', 0, "Remote 1 interrupt triggered!");
               return (FAILED);
        }
    }
    /* Test remote2 sensor */
    /* Read current remote2 sensor */
    if (max31730_read_remote_temp2() != PASSED) {
        cterr('f', 0, "Failed to read Remote2 Temp.");
        return (FAILED);
    }

    /* Set temp threshold */
    test_thr = (int)(temperature - DEG10);
    if ((NVRAM)->diagflag & D_VERBOSE) { 
        printf("Remote2 Temp. High test thread is %d degreeC.\n", test_thr);
    }
    if (set_threshold_x(2, TEST_M, test_thr) != PASSED) {
        cterr('f', 0, "Failed to set Remote2 High threshold to %d.", test_thr);
        return (FAILED);
    }
    msleep(LENGTH100);

    /* 1st Verify (0x1128) External Interrupt Pending Register */
    for (ix = 1; ix <= delay; ix++) {
         fpga_read_32_reg (FPGA_EXTER_INT_PENDING_REG, &buf);
         if ((buf & MB_THERM_INTERRUPT_PENDING) == MB_THERM_INTERRUPT_PENDING) {
             if ((NVRAM)->diagflag & D_VERBOSE) { 
                 printf(", delay %ld\n", ix);
                 printf("\nRemote 2 temp 1st verify - THIGH=%g i=%ld," 
                       "Remote 2 interrupt triggered\n", THIGH_remote2, ix);
             }   
             prpass(testpass, "Remote 2 interrupt triggered ");
             break;
         }
         if (ix == delay) {  /* return FAILED if consecutive n=10 times failures */
             if ((NVRAM)->diagflag & D_VERBOSE) {  
                 printf("\nFAIL@Remote 2 temp 1st verify - THIGH=%g i=%ld," 
                     "No remote 2 interrupt triggered!\n", THIGH_remote2, ix);
             } 
             cterr('f', 0, "No remote 2 interrupt triggered!");
             return (FAILED);
        }
    }

    /* Restore default temp threshold */
    test_thr = DEFAULT127;
    if (set_threshold_x(2, TEST_M, test_thr) != PASSED) {
        cterr('f', 0, "Failed to set Remote2 High threshold to %d.", test_thr);
        return (FAILED);
    }
    msleep(LENGTH1000);

    /* Clear pending interrupt on FPGA external interrupt pending Reg.(0x1128) */
    reg_offset = (uint)FPGA_EXTER_INT_PENDING_REG;
    reg_val = (uint)MB_THERM_INTERRUPT_PENDING;
    if (fpga_write_32_reg(reg_offset, reg_val) != PASSED) {
        printf("%s: Failed to clear FPGA pending intrrupt(0x%04X: 0x%04X).\n",
               __FUNCTION__, reg_offset, reg_val);
        return (FAILED);
    }
    msleep(LENGTH100);

    /* 2nd Verify (0x1128) External Interrupt Pending Register */
    for (ix = 1; ix <= delay; ix++) {
        fpga_read_32_reg (FPGA_EXTER_INT_PENDING_REG, &buf);
        if ((buf & MB_THERM_INTERRUPT_PENDING) != MB_THERM_INTERRUPT_PENDING) {
            if ((NVRAM)->diagflag & D_VERBOSE) { 
                printf(", delay %ld\n", ix);
                printf("\nRemote 2 temp 2nd verify - THIGH=%g i=%ld,"
                       "No Remote 2 interrupt triggered\n", THIGH_remote2, ix);
            }          
            prpass(testpass, "No Remote 2 interrupt triggered ");
            break;
        } else if (ix == delay) {  /* return FAILED if consecutive n=10 times failures */
            if ((NVRAM)->diagflag & D_VERBOSE) {     
                printf("\nFAIL@Remote 2 temp 2nd verify - THIGH=%g i=%ld," 
                      "Remote 2 interrupt triggered!\n", THIGH_remote2, ix);
            } 
            cterr('f', 0, "Remote 2 interrupt triggered!");
            return (FAILED);
        }
    }

    /* Test remote3 sensor */
    /* Read current remote3 sensor */
    if (max31730_read_remote_temp3() != PASSED) {
        cterr('f', 0, "Failed to read Remote3 Temp.");
        return (FAILED);
    }

    /* Set temp threshold */
    test_thr = (int)(temperature - DEG10);
    if ((NVRAM)->diagflag & D_VERBOSE) { 
        printf("Remote3 Temp. High test thread is %d degreeC.\n", test_thr);
    }
    if (set_threshold_x(3, TEST_M, test_thr) != PASSED) {
        cterr('f', 0, "Failed to set Remote3 High threshold to %d.", test_thr);
        return (FAILED);
    }
    msleep(LENGTH100);

    /* 1st Verify (0x1128) External Interrupt Pending Register */
    for (ix = 1; ix <= delay; ix++) {
        fpga_read_32_reg (FPGA_EXTER_INT_PENDING_REG, &buf);
        if ((buf & MB_THERM_INTERRUPT_PENDING) == MB_THERM_INTERRUPT_PENDING) {
            if ((NVRAM)->diagflag & D_VERBOSE) {   
                printf(", delay %ld\n", ix);
                printf("\nRemote 3 temp 1st verify - THIGH=%g i=%ld," 
                      "Remote 3 interrupt triggered\n", THIGH_remote3, ix);
            }
            prpass(testpass, "Remote 3 interrupt triggered\n");
            break;
        }
        if (ix == delay) {  /* return FAILED if consecutive n=10 times failures */
            if ((NVRAM)->diagflag & D_VERBOSE) {                
                printf("\nFAIL@Remote 3 temp 1st verify - THIGH=%g i=%ld," 
                     "No remote 3 interrupt triggered!\n", THIGH_remote3, ix);
            }
            cterr('f', 0, "No remote 3 interrupt triggered!");
            return (FAILED);
        }
    }
    /* Restore default temp threshold */
    test_thr = DEFAULT127;
    if (set_threshold_x(3, TEST_M, test_thr) != PASSED) {
        cterr('f', 0, "Failed to set Remote3 High threshold to %d.", test_thr);
        return (FAILED);
    }
    msleep(LENGTH1000);

    /* Clear pending interrupt on FPGA external interrupt pending Reg.(0x1128) */
    reg_offset = (uint)FPGA_EXTER_INT_PENDING_REG;
    reg_val = (uint)MB_THERM_INTERRUPT_PENDING;
    if (fpga_write_32_reg(reg_offset, reg_val) != PASSED) {
        printf("%s: Failed to clear FPGA pending intrrupt(0x%04X: 0x%04X).\n",
               __FUNCTION__, reg_offset, reg_val);
        return (FAILED);
    }
    msleep(LENGTH100);

    /* 2nd Verify (0x1128) External Interrupt Pending Register */
    for (ix = 1; ix <= delay; ix++) {
        fpga_read_32_reg (FPGA_EXTER_INT_PENDING_REG, &buf);
        if ((buf & MB_THERM_INTERRUPT_PENDING) != MB_THERM_INTERRUPT_PENDING) {
            if ((NVRAM)->diagflag & D_VERBOSE) {                
                printf(", delay %ld\n", ix);
                printf("\nRemote 3 temp 2nd verify - THIGH=%g i=%ld," 
                        "No Remote 3 interrupt triggered\n", THIGH_remote3, ix);
            } 
            prpass(testpass, "No Remote 3 interrupt triggered ");
            break;
        }
        if (ix == delay) {  /* return FAILED if consecutive n=10 times failures */
            if ((NVRAM)->diagflag & D_VERBOSE) {  
                 printf("\nFAIL@Remote 3 temp 2nd verify - THIGH=%g i=%ld," 
                        "Remote 3 interrupt triggered!\n", THIGH_remote3, ix);
            }
            cterr('f', 0, "Remote 3 interrupt triggered!");
            return (FAILED);
        }
    }

    /* Test all channels TLOW */
    test_thr = DEFAULT127;
    if (set_threshold_x(4, TEST_M, test_thr) != PASSED) {
        cterr('f', 0, "Failed to set Low threshold to %d.", test_thr);
        return (FAILED);
    }
    msleep(LENGTH100);

    /* 1st Verify (0x1128) External Interrupt Pending Register */
    for (ix = 1; ix <= delay; ix++) {
        fpga_read_32_reg (FPGA_EXTER_INT_PENDING_REG, &buf);
        if ((buf & MB_THERM_INTERRUPT_PENDING) == MB_THERM_INTERRUPT_PENDING) {
            if ((NVRAM)->diagflag & D_VERBOSE) {  
                printf(", delay %ld\n", ix);
                printf("\nAll channels TLOW temp 1st verify - TLOW=%g i=%ld," 
                       "Local interrupt triggered\n", TLOW, ix);
            }
            prpass(testpass, "TLOW Local interrupt triggered ");
            break;
        }
        if (ix == delay) {  /* return FAILED if consecutive n=10 times failures */
             if ((NVRAM)->diagflag & D_VERBOSE) { 
                 printf("\nFAIL@All channels TLOW temp 1st verify - TLOW=%g i=%ld," 
                           "No Local interrupt triggered!\n", TLOW, ix);
             }
             cterr('f', 0, "TLOW No local interrupt triggered!");
             return (FAILED);
        }
    }
    /* Restore default temp threshold */
    test_thr = DEFAULT0xC9;
    if (set_threshold_x(4, TEST_M, test_thr) != PASSED) {
        cterr('f', 0, "Failed to set Low threshold back to default.");
        return (FAILED);
    }
    msleep(LENGTH1000);

    /* Clear pending interrupt on FPGA external interrupt pending Reg.(0x1128) */
    reg_offset = (uint)FPGA_EXTER_INT_PENDING_REG;
    reg_val = (uint)MB_THERM_INTERRUPT_PENDING;
    if (fpga_write_32_reg(reg_offset, reg_val) != PASSED) {
        printf("%s: Failed to clear FPGA pending intrrupt(0x%04X: 0x%04X).\n",
               __FUNCTION__, reg_offset, reg_val);
        return (FAILED);
    }
    msleep(LENGTH100);

    /* 2nd Verify (0x1128) External Interrupt Pending Register */
    for (ix = 1; ix <= delay; ix++) {
        fpga_read_32_reg (FPGA_EXTER_INT_PENDING_REG, &buf);
        if ((buf & MB_THERM_INTERRUPT_PENDING) != MB_THERM_INTERRUPT_PENDING) {
            if ((NVRAM)->diagflag & D_VERBOSE) {    
                printf(", delay %ld\n", ix);
                printf("\nAll channels TLOW temp 2nd verify - TLOW=%g i=%ld," 
                      "No Local interrupt triggered\n", TLOW, ix);
            } 
            prpass(testpass, "No TLOW Local interrupt triggered ");
            break;
        }
        if (ix == delay) {  /* return FAILED if consecutive n=10 times failures */
            if ((NVRAM)->diagflag & D_VERBOSE) { 
                 printf("\nFAIL@All channels TLOW temp 2nd verify - TLOW=%g i=%ld," 
                        "Local interrupt triggered!\n", TLOW, ix);
            }
            cterr('f', 0, "Local interrupt triggered!");
            return (FAILED);
        }
    }

    return (rc);
}


/******************************************************************************
 *
 * Function: wifi_int_test
 *
 * Description: This function performs the wifi interrupt test.
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int wifi_int_test (void)
{
    uchar mb_get_loc[FRU_SIZE] = {0};
    uchar mb_get_pid[FRU_SIZE] = {0};
#ifdef ENHANCE_ERROR_MSG_RDY
    /*
     * 1. Subtests of the test function will reuse all variables
     * 2. All variables will be cleared automatically when
     *    entering and leaving each menu item.
     */
    /* Segment 1: PID | Unique_string : slot_info */
    fru_table_offset = MB;
    /* fru_table_offset should be set, otherwise, it will not */
    /* go to enhanced error message format in cterr() */
    /* set fru_table_offset to get the predefine value */
    /* or change mb_pid & mb_loc below */
    platform_get_pid((char *)mb_get_pid);
    strcpy((char *)mb_get_loc, "MB");
    platform_fru_table[fru_table_offset].pid_string = mb_get_pid;
    platform_fru_table[fru_table_offset].location_string = mb_get_loc;

    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("Marvell Armada 7040", "Local Bus", "System FPGA");

    /* Segment 5: register and memory dump */
    //cterr_add_reg_dump((PFV)bcm63168_get_xdsl_profile);

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)tsn_display_temp_errormsg);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Check the interface between the Host "
                    "SoC and the failed I2C devices.",
                    "If there is no problem for these interfaces, "
                    "replace one I2C device and redo the test.");
#endif

    wifi_temp_mode = TRUE;

    char *tname = "WiFi Temperature Interrupt";
    int rc = PASSED;
    double THIGH_local = 0;
    double THIGH_remote1 = 0;
    double THIGH_remote2 = 0;
    double TLOW = 0;
    uint buf=0;
    long int ix, delay=COUNT200000;   
    uint     reg_offset = 0, reg_val = 0;
 
    testname(tname);
    prpass(testpass, "%s, ", tname);

    printf("\nNotes:\n+/- 1 Degree Celsius Remote Temperature-"
              "Measurement Accuracy (0 Deg C to +100 Deg C)\n");
    /* Test local sensor */
    /* Read current sensor */
    rc = max31730_read_local_temp();
    if (rc != PASSED) {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("FAIL@Local temp reading - in max31730_read_local_temp!\n");
        }
        cterr('f', 0, "Local temp reading failed!");
        wifi_temp_mode = FALSE;
        return (FAILED);
    }
    /* Set temp threshold */
    THIGH_local = temperature - DEG10;
    rc = set_threshold_x(0, TEST_M, THIGH_local);
    /* 1st Verify (0x1128) External Interrupt Pending Register */
    for (ix = 1; ix <= delay; ix++) {
        fpga_read_32_reg (FPGA_EXTER_INT_PENDING_REG, &buf);
        if ((buf & WIFI_THERM_INTERRUPT_PENDING) == WIFI_THERM_INTERRUPT_PENDING) {
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf(", delay %ld\n", ix);
                printf("\nLocal temp 1st verify - THIGH=%g i=%ld," 
                      "Local interrupt triggered\n", THIGH_local, ix);
            }
            prpass(testpass, "Local interrupt triggered ");
            break;
        }
        else if (ix == delay) {  /* return FAILED if consecutive n=10 times failures */
            if ((NVRAM)->diagflag & D_VERBOSE) { 
                printf("\nFAIL@Local temp 1st verify - THIGH=%g i=%ld,"
                     "No Local interrupt triggered!\n", THIGH_local, ix);
            }
            cterr('f', 0, "No local interrupt triggered!");
            wifi_temp_mode = FALSE;
            return (FAILED);
           }
    }
    /* Restore default temp threshold */
    THIGH_local = DEFAULT127;
    set_threshold_x(0, TEST_M, THIGH_local);
    msleep(LENGTH1000);

    /* Clear pending interrupt on FPGA external interrupt pending Reg.(0x1128) */
    reg_offset = (uint)FPGA_EXTER_INT_PENDING_REG;
    reg_val = (uint)WIFI_THERM_INTERRUPT_PENDING;
    if (fpga_write_32_reg(reg_offset, reg_val) != PASSED) {
        printf("%s: Failed to clear FPGA pending intrrupt(0x%04X: 0x%04X).\n",
               __FUNCTION__, reg_offset, reg_val);
        return (FAILED);
    }
    msleep(LENGTH100);

    /* 2nd Verify (0x1128) External Interrupt Pending Register */
    for (ix = 1; ix <= delay; ix++) {
        fpga_read_32_reg (FPGA_EXTER_INT_PENDING_REG, &buf);
        if ((buf & WIFI_THERM_INTERRUPT_PENDING) != WIFI_THERM_INTERRUPT_PENDING) {
            if ((NVRAM)->diagflag & D_VERBOSE) {  
                printf(", delay %ld\n", ix);
                printf("\nLocal temp 2nd verify - THIGH=%g i=%ld," 
                        "No Local interrupt triggered\n", THIGH_local, ix);
            } 
            prpass(testpass, "No Local interrupt triggered ");
            break;
       } else if (ix == delay) {  /* return FAILED if consecutive n=10 times failures */
           if ((NVRAM)->diagflag & D_VERBOSE) {  
               printf("\nFAIL@Local temp 2nd verify - THIGH=%g i=%ld,"
                      "Local interrupt triggered!\n", THIGH_local, ix);
           }
           cterr('f', 0, "Local interrupt triggered!");
           wifi_temp_mode = FALSE;
           return (FAILED);
       }
    }
    /* Test remote1 sensor */
    /* Read current remote1 sensor */
    rc = max31730_read_remote_temp1();
    if (rc != PASSED) {
        if ((NVRAM)->diagflag & D_VERBOSE) { 
            printf("FAIL@Remote 1 temp reading - in max31730_read_remote_temp1!\n");
        } 
        cterr('f', 0, "Remote 1 temp reading failed!");
        wifi_temp_mode = FALSE;
        return (FAILED);
    }
    /* Set temp threshold */
    THIGH_remote1 = temperature - DEG10;
    rc = set_threshold_x(1, TEST_M, THIGH_remote1);
    /* 1st Verify (0x1128) External Interrupt Pending Register */
    for (ix = 1; ix <= delay; ix++) {
        fpga_read_32_reg (FPGA_EXTER_INT_PENDING_REG, &buf);
        if ((buf & WIFI_THERM_INTERRUPT_PENDING) == WIFI_THERM_INTERRUPT_PENDING) {
            if ((NVRAM)->diagflag & D_VERBOSE) { 
               printf(", delay %ld\n", ix);
               printf("\nRemote 1 temp 1st verify - THIGH=%g i=%ld," 
                      "Remote 1 interrupt triggered\n", THIGH_remote1, ix);
            }
            prpass(testpass, "Remote 1 interrupt triggered ");
            break;
        } else if (ix == delay) {  /* return FAILED if consecutive n=10 times failures */
            if ((NVRAM)->diagflag & D_VERBOSE) { 
                printf("\nFAIL@Remote 1 temp 1st verify - THIGH=%g i=%ld," 
                      "No remote 1 interrupt triggered!\n", THIGH_remote1, ix);
            }
            cterr('f', 0, "No remote 1 interrupt triggered!");
            wifi_temp_mode = FALSE;
            return (FAILED);
        }
    }
    /* Restore default temp threshold */
    THIGH_remote1 = DEFAULT127;
    set_threshold_x(1, TEST_M, THIGH_remote1);
    msleep(LENGTH1000);

    /* Clear pending interrupt on FPGA external interrupt pending Reg.(0x1128) */
    reg_offset = (uint)FPGA_EXTER_INT_PENDING_REG;
    reg_val = (uint)WIFI_THERM_INTERRUPT_PENDING;
    if (fpga_write_32_reg(reg_offset, reg_val) != PASSED) {
        printf("%s: Failed to clear FPGA pending intrrupt(0x%04X: 0x%04X).\n",
               __FUNCTION__, reg_offset, reg_val);
        return (FAILED);
    }
    msleep(LENGTH100);

    /* 2nd Verify (0x1128) External Interrupt Pending Register */
    for (ix = 1; ix <= delay; ix++) {
        fpga_read_32_reg (FPGA_EXTER_INT_PENDING_REG, &buf);
        if ((buf & WIFI_THERM_INTERRUPT_PENDING) != WIFI_THERM_INTERRUPT_PENDING) {
            if ((NVRAM)->diagflag & D_VERBOSE) {  
                printf(", delay %ld\n", ix);
                printf("\nRemote 1 temp 2nd verify - THIGH=%g i=%ld," 
                "No Remote 1 interrupt triggered\n", THIGH_remote1, ix);
            } 
            prpass(testpass, "No Remote 1 interrupt triggered ");
            break;
        } else if (ix == delay) {  /* return FAILED if consecutive n=10 times failures */
               if ((NVRAM)->diagflag & D_VERBOSE) {  
                   printf("\nFAIL@Remote 1 temp 2nd verify - THIGH=%g i=%ld," 
                      "Remote 1 interrupt triggered!\n", THIGH_remote1, ix);
               } 
               cterr('f', 0, "Remote 1 interrupt triggered!");
               wifi_temp_mode = FALSE;
               return (FAILED);
        }
    }
    /* Test remote2 sensor */
    /* Read current remote1 sensor */
    rc = max31730_read_remote_temp2();
    if (rc != PASSED) {
        if ((NVRAM)->diagflag & D_VERBOSE) {  
            printf("FAIL@Remote 2 temp reading - in max31730_read_remote_temp2!\n");
        } 
        cterr('f', 0, "Remote 2 temp reading failed!");
        wifi_temp_mode = FALSE;
        return (FAILED);
    }
    /* Set temp threshold */
    THIGH_remote2 = temperature - DEG10;
    rc = set_threshold_x(2, TEST_M, THIGH_remote2);
    /* 1st Verify (0x1128) External Interrupt Pending Register */
    for (ix = 1; ix <= delay; ix++) {
         fpga_read_32_reg (FPGA_EXTER_INT_PENDING_REG, &buf);
         if ((buf & WIFI_THERM_INTERRUPT_PENDING) == WIFI_THERM_INTERRUPT_PENDING) {
             if ((NVRAM)->diagflag & D_VERBOSE) { 
                 printf(", delay %ld\n", ix);
                 printf("\nRemote 2 temp 1st verify - THIGH=%g i=%ld," 
                       "Remote 2 interrupt triggered\n", THIGH_remote2, ix);
             }   
             prpass(testpass, "Remote 2 interrupt triggered ");
             break;
         } else if (ix == delay) {  /* return FAILED if consecutive n=10 times failures */
             if ((NVRAM)->diagflag & D_VERBOSE) {  
                 printf("\nFAIL@Remote 2 temp 1st verify - THIGH=%g i=%ld," 
                     "No remote 2 interrupt triggered!\n", THIGH_remote2, ix);
             } 
             cterr('f', 0, "No remote 2 interrupt triggered!");
             wifi_temp_mode = FALSE;
             return (FAILED);
        }
    }
    /* Restore default temp threshold */
    THIGH_remote2 = DEFAULT127;
    set_threshold_x(2, TEST_M, THIGH_remote2);
    msleep(LENGTH1000);

    /* Clear pending interrupt on FPGA external interrupt pending Reg.(0x1128) */
    reg_offset = (uint)FPGA_EXTER_INT_PENDING_REG;
    reg_val = (uint)WIFI_THERM_INTERRUPT_PENDING;
    if (fpga_write_32_reg(reg_offset, reg_val) != PASSED) {
        printf("%s: Failed to clear FPGA pending intrrupt(0x%04X: 0x%04X).\n",
               __FUNCTION__, reg_offset, reg_val);
        return (FAILED);
    }
    msleep(LENGTH100);

    /* 2nd Verify (0x1128) External Interrupt Pending Register */
    for (ix = 1; ix <= delay; ix++) {
        fpga_read_32_reg (FPGA_EXTER_INT_PENDING_REG, &buf);
        if ((buf & WIFI_THERM_INTERRUPT_PENDING) != WIFI_THERM_INTERRUPT_PENDING) {
            if ((NVRAM)->diagflag & D_VERBOSE) { 
                printf(", delay %ld\n", ix);
                printf("\nRemote 2 temp 2nd verify - THIGH=%g i=%ld,"
                       "No Remote 2 interrupt triggered\n", THIGH_remote2, ix);
            }          
            prpass(testpass, "No Remote 2 interrupt triggered ");
            break;
        } else if (ix == delay) {  /* return FAILED if consecutive n=10 times failures */
            if ((NVRAM)->diagflag & D_VERBOSE) {     
                printf("\nFAIL@Remote 2 temp 2nd verify - THIGH=%g i=%ld," 
                      "Remote 2 interrupt triggered!\n", THIGH_remote2, ix);
            } 
            cterr('f', 0, "Remote 2 interrupt triggered!");
            wifi_temp_mode = FALSE;
            return (FAILED);
        }
    }
    /* Test all channels TLOW */
    /* Read current local sensor */
    rc = max31730_read_local_temp();
    if (rc != PASSED) {
        if ((NVRAM)->diagflag & D_VERBOSE) {   
            printf("FAIL@Local temp reading - in max31730_read_local_temp!\n");
        } 
        cterr('f', 0, "Local temp reading failed!");
        wifi_temp_mode = FALSE;
        return (FAILED);
    }
    /* Set temp threshold */
    TLOW = temperature + DEG10;
    rc = set_threshold_x(4, TEST_M, TLOW);
    /* 1st Verify (0x1128) External Interrupt Pending Register */
    for (ix = 1; ix <= delay; ix++) {
        fpga_read_32_reg (FPGA_EXTER_INT_PENDING_REG, &buf);
        if ((buf & WIFI_THERM_INTERRUPT_PENDING) == WIFI_THERM_INTERRUPT_PENDING) {
            if ((NVRAM)->diagflag & D_VERBOSE) {  
                printf(", delay %ld\n", ix);
                printf("\nAll channels TLOW temp 1st verify - TLOW=%g i=%ld," 
                       "Local interrupt triggered\n", TLOW, ix);
            }
            prpass(testpass, "TLOW Local interrupt triggered ");
            break;
        } else if (ix == delay) {  /* return FAILED if consecutive n=10 times failures */
             if ((NVRAM)->diagflag & D_VERBOSE) { 
                 printf("\nFAIL@All channels TLOW temp 1st verify - TLOW=%g i=%ld," 
                           "No Local interrupt triggered!\n", TLOW, ix);
             }
             cterr('f', 0, "TLOW No local interrupt triggered!");
             wifi_temp_mode = FALSE;
             return (FAILED);
        }
    }
    /* Restore default temp threshold */
    TLOW = DEFAULTMIN55;
    set_threshold_x(4, TEST_M, TLOW);
    msleep(LENGTH1000);

    /* Clear pending interrupt on FPGA external interrupt pending Reg.(0x1128) */
    reg_offset = (uint)FPGA_EXTER_INT_PENDING_REG;
    reg_val = (uint)WIFI_THERM_INTERRUPT_PENDING;
    if (fpga_write_32_reg(reg_offset, reg_val) != PASSED) {
        printf("%s: Failed to clear FPGA pending intrrupt(0x%04X: 0x%04X).\n",
               __FUNCTION__, reg_offset, reg_val);
        return (FAILED);
    }
    msleep(LENGTH100);

    /* 2nd Verify (0x1128) External Interrupt Pending Register */
    for (ix = 1; ix <= delay; ix++) {
        fpga_read_32_reg (FPGA_EXTER_INT_PENDING_REG, &buf);
        if ((buf & WIFI_THERM_INTERRUPT_PENDING) != WIFI_THERM_INTERRUPT_PENDING) {
            if ((NVRAM)->diagflag & D_VERBOSE) {    
                printf(", delay %ld\n", ix);
                printf("\nAll channels TLOW temp 2nd verify - TLOW=%g i=%ld," 
                      "No Local interrupt triggered\n", TLOW, ix);
            } 
            prpass(testpass, "No TLOW Local interrupt triggered ");
            break;
        } else if (ix == delay) {  /* return FAILED if consecutive n=10 times failures */
            if ((NVRAM)->diagflag & D_VERBOSE) { 
                 printf("\nFAIL@All channels TLOW temp 2nd verify - TLOW=%g i=%ld," 
                        "Local interrupt triggered!\n", TLOW, ix);
            }
            cterr('f', 0, "Local interrupt triggered!");
            wifi_temp_mode = FALSE;
            return (FAILED);
        }
    }

    wifi_temp_mode = FALSE;
    return (rc);
}

/*------------------------------------------------------------------
$Log: platform_sensor.c,v $
Revision 1.4  2019/01/18 05:54:47  yungchen
Merge Supernova branch to the main trunk (CSCvn79871)

Revision 1.3  2018/02/09 09:56:55  hondwang
Merge Star branch star-branch-c9xx to main trunk

Revision 1.2.22.1  2018/01/20 06:27:24  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.2  2017/08/02 14:21:49  steja
Support TSN-H/M platform code

Revision 1.1.8.2  2017/07/29 03:41:20  steja
tsn-branch5 synced with Maintrunk repositories

Revision 1.1.6.4  2017/07/25 08:31:56  steja
1. Remove unused code.
2. Verified before check-in

Revision 1.1.6.3  2017/07/24 14:14:11  palin2
1. To improve code readability.
2. All changes are verified before check-in.

Revision 1.1.6.2  2017/07/20 13:38:07  steja
tsn-branch4 merge with maintrunk

Revision 1.1.4.12.2.4  2017/07/18 14:23:37  steja
Code Cleanup

Revision 1.1.4.12.2.3  2017/07/18 03:53:01  steja
Code cleanup

Revision 1.1.4.12.2.2  2017/07/17 14:41:00  steja
code cleanup

Revision 1.1.4.12.2.1  2017/02/23 11:03:16  palin2
Updated code based on FPGA changes. These updates are verified on P2A TSN.

Revision 1.1.4.12  2016/11/21 02:09:43  petteng
Add enhanced error message

Revision 1.1.4.11  2016/11/09 06:49:44  petteng
Add enhanced error message

Revision 1.1.4.10  2016/11/01 07:29:22  petteng
Add enhanced error message

Revision 1.1.4.9  2016/10/06 07:41:40  petteng
Fix temperature reading issue

Revision 1.1.4.7  2016/09/07 15:12:52  steja
Add wifi temperature interrupt test

Revision 1.1.4.6  2016/08/30 12:58:56  steja
Update Wifi temp sensor utility

Revision 1.1.4.5  2016/08/23 08:14:17  steja
Add MB Temperature interrupt test

Revision 1.1.4.4  2016/07/25 09:32:06  steja
Add Wlan DC present or not present

Revision 1.1.4.3  2016/07/21 14:26:32  steja
1. Update Temperature sensor function
2. Add Wifi Temperature sensor to basic utilities
3. Add LTE Extended Feature Test
4. Display Chasis Temperature When Boot up
5. Update I2C scan function
6. Update POE cookie check card present

Revision 1.1.4.2  2016/06/30 06:22:51  steja
tsn-branch2 sync with main trunk

Revision 1.1.2.3  2016/05/24 01:18:11  palin2
Updated Thermal sensor and ACT2 chip I2C bus number based on P1A HW changes

Revision 1.1.2.2  2016/04/11 14:12:27  steja
Update code i2c utility for bringup

Revision 1.1.2.1  2016/03/20 05:32:40  steja
1. Add i2c scan and i2c read write utility
2. Add sensor temperature and eeprom utility


$Endlog$
*/
