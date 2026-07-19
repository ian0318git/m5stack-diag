/* $Id: diag_raid_util.c,v 1.2 2016/04/20 11:25:33 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_raid_util.c,v $
 *------------------------------------------------------------------
 *
 * diag_raid_util.c - Utility Function
 *
 * Oct. 2015, benchen2
 *
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include "common.h"
#include "types.h"
#include "error.h"
#include "menu.h"
#include "defs.h"
#include "common_utils.h"
#include "i2c_api.h"
#include "goofy_i2c.h"
#include "proto.h"
#include "platform_i2c.h"
#include "nvmonvars.h"
#include "diag_raid_util.h"
#include "diag_raid_lib.h"

uint32_t cpld_upgrade_firmware(void);
uint32_t sbr_upgrade_firmware(void);
uint32_t raid_eeprom_ctrl_switch(void);
uint32_t raid_sbr_ctrl_op(void);
uint32_t raid_pca9557_read(void);
uint32_t raid_pca9557_write(void);

/* Sub Menu used for raid utility.
 */
static submenu_xtable_t raid_util_submenu_table[] = {
    {"CPLD upgrade", (type_t(*)())cpld_upgrade_firmware,   0,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
	{"SBR EEPROM upgrade", (type_t(*)())sbr_upgrade_firmware,   0,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
	{"SBR ctrl", (type_t(*)())raid_sbr_ctrl_op,   0,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Control Switch 480/200", (type_t(*)())raid_eeprom_ctrl_switch,   0,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"PCA9557 Read", (type_t(*)())raid_pca9557_read,   0,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"PCA9557 Write", (type_t(*)())raid_pca9557_write,   0,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Dump CPLD Registers", (type_t(*)())platform_cpld_reg_dump,   0,
    0, (type_t(*)())0, 0, (type_t(*)())0,   0},
};

#define RAID_UTIL_SUBMENU_TABLE_SIZE (sizeof(raid_util_submenu_table) / \
				       sizeof(submenu_xtable_t))

static mitem_t raid_util_primary_items[RAID_UTIL_SUBMENU_TABLE_SIZE +
				       MAX_BASE_ITEMS];
static mitem_t raid_util_secondary_items[RAID_UTIL_SUBMENU_TABLE_SIZE +
					 MAX_BASE_ITEMS];

menuinfo_t raid_util_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    raid_util_primary_items,
};

menuinfo_t *raid_util_submenup = &raid_util_subtest_menu;

int diag_raid_util (void)
{
    build_primary_submenu(raid_util_submenu_table,
    		     RAID_UTIL_SUBMENU_TABLE_SIZE,
                 "RAID", &raid_util_submenup);
    build_secondary_submenu(raid_util_submenu_table,
    		     RAID_UTIL_SUBMENU_TABLE_SIZE,
                 raid_util_secondary_items);

    menu(raid_util_submenup, raid_util_secondary_items, '\0');
    return (PASSED);
}


extern uchar raid_sbr_fw[];
extern ulong raid_sbr_fw_size;
uint32_t sbr_upgrade_firmware (void)
{
    int retval = PASSED;
    
    if (raid_sbr_ctrl(SBR_EN_PROGRAM) != PASSED) {
        retval = FAILED;
    }

    if (diag_raid_sbr_fw_upgrade(raid_sbr_fw_size, raid_sbr_fw) != PASSED) {
        retval = FAILED;
    }

    if (raid_sbr_ctrl(SBR_DIS_PROGRAM) != PASSED) {
        retval = FAILED;
    }

    if ( retval == PASSED ) {
        printf("Program Done!!!\n");
        printf("Reboot the system manually\n");
    } else {
        printf("Program Fail!!!\n");
    }
    
    return (retval);
}

uint32_t raid_sbr_ctrl_op(void)
{  
    int retval = PASSED;
    int method = gethex_answer("LSI/HOST(1/0)", 0, 0, 1);
    
    /* method 0 control by BMC*/
    if (method == 0) {
        if (raid_sbr_ctrl(SBR_EN_PROGRAM) != PASSED) {
            retval = FAILED;
        } else {
            printf("Bus 5 Device 0xA0 can acces by HOST\n");
            system("I2cBusScan 5");
        }
    /* method 1 control by LSI*/
    } else {
        if (raid_sbr_ctrl(SBR_DIS_PROGRAM) != PASSED) {
            retval = FAILED;
        } else {
            printf("Bus 5 Device 0xA0 can acces by LSI\n");
        }
    }
    return (retval);
}


uint32_t cpld_upgrade_firmware (void)
{
    uchar cpld_buf;
    int method;

    unsigned char *hex;
    /*parse file uploaded by user*/
    hex = parse_cpld_data();

    printf("Programming CPLD 5M570...\n");

    method = gethex_answer("IO_expander/cpld_upgrade(0/1)", 0, 0, 1);

    if (method == 0) {
        cpld_buf = 0;

        if (platform_5m570_i2c_w(CPLD_JTAG_CTL, cpld_buf) == FAILED) {
            printf("Current CPLD firmware is not responding.\n");
        }
       /* Upgrade the CPLD firmware from simpler source code. */
        if (platform_pca9557_init() == FAILED) {
            return (FAILED);
        }
        
        if (platform_pca9557_program_cpld(hex) == FAILED) {
            return (FAILED);
        }
    } else {
        /* Do program the POF firmware file. */
        /* Before upgrade the CPLD firmware, needs to turn on the
         * JTAG_ON bit. */
        if (platform_cpld_jtag_ctl(TRUE) == FAILED) {
            return (FAILED);
        }

        if (platform_simply_program_cpld(hex) == FAILED) {
            return (FAILED);
        }

        if (platform_cpld_jtag_ctl(FALSE) == FAILED) {
                    return (FAILED);
        }
    }

    printf("Please power cycle Tachi system...\n");
    
    return (PASSED);
}

uint32_t raid_eeprom_ctrl_switch (void)
{
    int retval =PASSED;
    int eeprom_ctrl_switch;

    eeprom_ctrl_switch = gethex_answer("CH480/CH200(0/1)", 1, 0, 1);
    
    if (raid_switch_ctrl(eeprom_ctrl_switch) == FAILED) {
        retval = FAILED;
    }
    return (retval);
}

uint32_t raid_pca9557_read (void)
{
    int retval =PASSED;
    int pca9557_offset;
    uint8_t pca9557_val;
    pca9557_offset = gethex_answer("offset", 0, 0, 3);
    if (platform_pca9557_i2c_r(pca9557_offset, &pca9557_val) == FAILED) {
        return (FAILED);  
    }
    printf("value is 0x%x\n", pca9557_val);
    return (retval);
}

uint32_t raid_pca9557_write (void)
{
    int retval =PASSED;
    int pca9557_offset;
    uint8_t pca9557_val;
    
    pca9557_offset = gethex_answer("offset", 0, 0, 3);
    pca9557_val= gethex_answer("value", 0x00, 0x00, 0xff);
    if (platform_pca9557_i2c_w(pca9557_offset, pca9557_val) == FAILED) {
        return (FAILED);
    }
    return (retval);
}

/*---------------------------------------------------------------
$Log: diag_raid_util.c,v $
Revision 1.2  2016/04/20 11:25:33  benchen2
add tachi fru portion

Revision 1.1.2.9  2016/03/02 08:35:42  benchen2
add sbr vdd eeprom ping test

Revision 1.1.2.8  2016/02/05 01:41:24  benchen2
raid card support SBR eeprom program

Revision 1.1.2.7  2016/01/13 01:47:40  benchen2
add raid cpld 9557 upgrade method

Revision 1.1.2.6  2016/01/08 06:22:12  benchen2
add epm570 remote upgrade speedup method

Revision 1.1.2.5  2015/11/16 08:13:10  benchen2
rm cpld power cycle

Revision 1.1.2.4  2015/11/13 07:34:31  benchen2
add raid card test and utility

Revision 1.1.2.3  2015/10/23 07:42:25  benchen2
fix raid card utility

Revision 1.1.2.2  2015/10/20 08:21:10  benchen2
add raid util(PCA9557, control switch)

Revision 1.1.2.1  2015/10/12 08:22:26  benchen2
add raid card cpld upgrade


$Endlog$
*/

