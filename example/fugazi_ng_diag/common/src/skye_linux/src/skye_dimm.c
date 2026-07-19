/* $Id: skye_dimm.c,v 1.2 2015/05/25 03:59:16 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/skye_linux/src/skye_dimm.c,v $
 *------------------------------------------------------------------------------
 * skye_dimm.c - Main file of Skye DIMM function and utility.
 *
 * July 02 2013, Paul Lin(palin2) ported from Overlord.
 *
 * Jul 2014 - Paul Lin(palin2) ported from Shrinkray.
 *
 * Copyright (c) 2013-2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------------------
 */

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include "common.h"
#include "common_utils.h"
#include "defs.h"
#include "types.h"
#include "error.h"
#include "menu.h"
#include "proto.h"
#include "string.h"
#include "skye_i2c.h"
#include "skye_dimm.h"


/******************************************************************************
 *                             Function protos
 ******************************************************************************/
static int dump_dimm_info(uint32_t);
static int dump_dimm_spd_wrap(int);
static int dump_dimm_spd_in_raw_wrap(int);
static int alter_dimm_spd_by_byte_wrap(int);


/******************************************************************************
 *                                Externs
 ******************************************************************************/


/******************************************************************************
 *                             Global Variables
 ******************************************************************************/
uint32_t dimm_no = UNKNOWN_DIMM_NO;


/******************************************************************************
 *                                 Menus
 ******************************************************************************/ 
/*
 * DIMM utilities SubMenu Table
 */
static submenu_xtable_t dimm_util_table[] = {
    {"Show DIMM SPD info",            (PFT)dump_dimm_spd_wrap,            TRUE,
     MF_CONTINUOUS,                   (type_t(*)())0,                     0,
     (type_t(*)())0,                  0},
    {"Show DIMM SPD in RAW",          (PFT)dump_dimm_spd_in_raw_wrap,     TRUE,
     MF_CONTINUOUS,                   (type_t(*)())0,                     0,
     (type_t(*)())0,                  0},
    {"Alter DIMM SPD by byte",        (PFT)alter_dimm_spd_by_byte_wrap,   TRUE,
     MF_CONTINUOUS,                   (type_t(*)())0,                     0,
     (type_t(*)())0,                  0},
};

#define DIMM_UTIL_TABLE_SZ \
        (sizeof(dimm_util_table) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t dimm_util_primary_items[DIMM_UTIL_TABLE_SZ + MAX_BASE_ITEMS];
static mitem_t dimm_util_secondary_items[DIMM_UTIL_TABLE_SZ + MAX_BASE_ITEMS];

static menuinfo_t dimm_util_menu = {
    "%s Utilities SubMenu",
    0,                            /* mtparam added by init_empty_menu */
    (PFT)show_endnote,            /* notes missing WICs in combos */
    0,                            /* use generic prompt */
    0,                            /* size (bumped by add_menu_item() */
    dimm_util_primary_items,
};

menuinfo_t *dimm_util_submenup = &dimm_util_menu;


/*******************************************************************************
 *
 * Function   : build_dimm_util_menu
 * Description: Function to build Skye DIMM utility submenu.
 * Inputs     : num - number of DIMM
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int
build_dimm_util_menu (int num)
{
    char menu_title[32];

    memset(menu_title, 0 , sizeof(menu_title));
    snprintf(menu_title, sizeof(menu_title), "CPU CH%d DIMM", num);

    build_primary_submenu(dimm_util_table, DIMM_UTIL_TABLE_SZ,
                          menu_title, &dimm_util_submenup);
    build_secondary_submenu(dimm_util_table, DIMM_UTIL_TABLE_SZ,
                            dimm_util_secondary_items);

    /* Setup DIMM number */
    dimm_no = num;

    /* Display Utility Menu */
    menu(dimm_util_submenup, dimm_util_secondary_items, 0);

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : dump_dimm_info
 * Description:	Function to dump DDR DIMM SPD info.
 * Inputs     : cmd - Command to decide the dump type
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int
dump_dimm_info (uint32_t cmd)
{
    int        ctr = 0;
    uchar      buf[DDR3_SPD_SIZE];
    ddr3_spd_t *dimm;

    memset(buf, 0, sizeof(buf));

    if (skye_dimm_spd_read(dimm_no, 0, DDR3_SPD_SIZE, buf) != PASSED) {
        printf("%s: Failed to read DIMM%d SPD.\n", __FUNCTION__, dimm_no);
        return (FAILED);
    }

    dimm = (ddr3_spd_t *)buf;

    switch (cmd) {
    case DIMM_DUMP_ALL:
        /* Display the info */
        printf("\nNumber of SPD bytes used / bytes total and CRC Coverage"
               " = %#x\n", dimm->size);    
        printf("SPD Revision = %#x\n", dimm->revision);    
        printf("DRAM Device Type = %#x\n", dimm->dram_type);    
        printf("Module Type    = %#x\n", dimm->module_type);    
        printf("SDRAM Density and Banks = %#x\n", dimm->desity_bank);    
        printf("SDRAM Addressing = %0#x\n", dimm->addressing);    
        printf("Module Nominal Voltage, VDD = %d\n", dimm->vdd);    
        printf("Module Organization = %#x\n", dimm->organization);    
        printf("Module Memory Bus Width = %#x\n", dimm->bus_width);              
        printf("Fine Timebase(FTB) Dividend / Divisor = %#x\n",
               dimm->ftb_dividend);
        printf("Medium Timebase(MTB) Dividend = %#x\n", dimm->mtb_dividend);
        printf("Medium Timebase(MTB) Divisor = %#x\n", dimm->mtb_divisor);    
        printf("SDRAM Minimum Cycle Time(tCKmin) = %#x\n", dimm->tckmin);
        printf("CAS Latencies Supported, LSB = %#x\n", dimm->cas_lsb);
        printf("CAS Latencies Supported, MSB = %#x\n", dimm->cas_msb);
        printf("MIN CAS Latency(tAAmin) = %#x\n", dimm->taamin);
        printf("MIN Write Recovery(tWRmin) = %#x\n", dimm->twrmin);   
        printf("MIN RAS# to CAS# Delay(tRCDmin) = %#x\n", dimm->trcdmin);
        printf("MIN Row Act. to Row Act. Delay(tRRDmin) = %#x\n", dimm->trrdmin);    
        printf("MIN Row Precharge Delay(tRPmin) = %#x\n", dimm->trpmin);
        printf("Upper Nibbles for tRAS and tRC = %#x\n", dimm->u_nib_tras_trc);    
        printf("MIN Act. to Precharge Delay(tRASmin), LSB = %#x\n",
               dimm->trasmin_lsb);         
        printf("MIN Act. to Act./Refresh Delay(tRCmin), LSB = %#x\n",
               dimm->trcmin_lsb);              
        printf("MIN Refresh Recovery Delay(tRFCmin), LSB = %#x\n",
               dimm->trfcmin_lsb);              
        printf("MIN Refresh Recovery Delay(tRFCmin), MSB = %#x\n",
               dimm->trfcmin_msb);           
        printf("MIN Internal WR to RD Cmd Delay(tWTRmin) = %#x\n",
               dimm->twtrmin);
        printf("MIN Internal RD to Percharge Cmd Delay(tRTPmin) = %#x\n",
               dimm->trtpmin);
        printf("Upper Nibble for tFAW = %#x\n", dimm->u_nib_tfaw);
        printf("MIN Four Activate WIN Delay(tFAWmin) = %#x\n", dimm->tfawmin);
        printf("SDRAM Optional Features = %#x\n", dimm->opt_features);
        printf("SDRAM Thermal and Refresh Options = %#x\n", dimm->therm_ref_opt);
        printf("Module Thermal Sensor = %#x\n", dimm->therm_sensor);    
        printf("SDRAM Device Type = %#x\n", dimm->sdram_type);
        printf("Type Spec Sec., Index. by Key Byte 3 = ");
        for (ctr = 0; ctr < sizeof(dimm->type_spec); ctr++) {
             printf("%#x ", dimm->type_spec[ctr]);
        }
        printf("\n");
        printf("Module Manufacturer's JEDEC ID Code = ");
        for (ctr = 0; ctr < sizeof(dimm->mod_jedec_id); ctr++) {
            printf("%#x ", dimm->mod_jedec_id[ctr]);
        }
        printf("\n");
        printf("Manufacturer's Location = %#x\n", dimm->manu_location);
        printf("Manufacturer's Date = ");
        for (ctr = 0; ctr < sizeof(dimm->manu_date); ctr++) {
            printf("%#x ", dimm->manu_date[ctr]);
        }
        printf("\n");
        printf("Module Serial Number = ");
        for (ctr = 0; ctr < sizeof(dimm->serial_no); ctr++) {
            printf("%#x ", dimm->serial_no[ctr]);
        }
        printf("\n");
        printf("Cyclical Redundancy Code = ");
        for (ctr = 0; ctr < sizeof(dimm->crc); ctr++) {
            printf("%#x ", dimm->crc[ctr]);
        }
        printf("\n");
        printf("Module Part Number = ");
        for (ctr = 0; ctr < sizeof(dimm->part_no); ctr++) {
            printf("%#x ", dimm->part_no[ctr]);
        }
        printf("\n");
        printf("Module Revision Code = ");
        for (ctr = 0; ctr < sizeof(dimm->rev_code); ctr++) {
             printf("%#x ", dimm->rev_code[ctr]);
        }
        printf("\n");
        printf("DRAM Manufacturer's JEDEC ID Code = ");
        for (ctr = 0; ctr < sizeof(dimm->dram_jedec_id); ctr++) {
            printf("%#x ", dimm->dram_jedec_id[ctr]);
        }
        printf("\n");
        printf("Manufacturer's Specific Data = ");
        for (ctr = 0; ctr < sizeof(dimm->manu_spec); ctr++) {
            printf("%#x ", dimm->manu_spec[ctr]);
        }
        printf("\n");
        printf("Open for customer use = ");
        for (ctr = 0; ctr < sizeof(dimm->customer_use); ctr++) {
            printf("%#x ", dimm->customer_use[ctr]);
        }
        printf("\n\n");
    break;
    case DIMM_DUMP_RAW:
        printf("DDR DIMM%d SPD in RAW:\n", dimm_no);
        printf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f");
    
        for (ctr = 0; ctr < sizeof(buf); ctr++) {
            if ((ctr % 16) == 0) {
                printf("\n%.2x:", ctr);
            }
            printf(" %.2x", buf[ctr]);
        }
        printf("\n");
    break;
    case DIMM_DUMP_MEMSIZE:
        /* return memsize direcctly */
        return((uint32_t)dimm->desity_bank);
    break;
    default:
        assert(!"dimm_dev_show");
    break;
    }

    return(PASSED);
}

/*******************************************************************************
 *
 * Function   :	dump_dimm_spd_wrap
 * Description:	Wrapper function to dump DDR DIMM SPD info.
 * Inputs     :	opt - reserved for future use
 * Outputs    :	PASSED/FAILED
 *
 *******************************************************************************
 */
static int
dump_dimm_spd_wrap (int opt)
{
    int dummy = 0;

    dummy = opt;

    /* Display the registers */
    if (dump_dimm_info(DIMM_DUMP_ALL) != PASSED) {
        cterr('f', 0, "Failed to dump DIMM%d SPD info.", dimm_no);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   :	dump_dimm_spd_in_raw_wrap
 * Description:	Wrapper function to dump DDR DIMM SPD data in RAW.
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int
dump_dimm_spd_in_raw_wrap (int opt)
{
    int dummy = 0;

    dummy = opt;

    /* dump DIMM RAW data */
    if (dump_dimm_info(DIMM_DUMP_RAW) != PASSED) {
        cterr('f', 0, "Failed to dump DIMM%d SPD data in RAW.", dimm_no);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   :	alter_dimm_spd_by_byte_wrap
 * Description:	Wrapper function to dump DDR DIMM SPD data in RAW.
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int
alter_dimm_spd_by_byte_wrap (int opt)
{
    int      dummy = 0;
    dummy = opt;

#ifdef DIAG_DEBUG
    uint16_t offset = 0;
    uchar    wdata = 0;

    offset = (uint16_t)gethex_answer("Enter offset you want to write ",
                                     0xFF, 0x0, 0xFF);
    wdata = (uchar)gethex_answer("Enter Data you want to write-in ",
                                 0x0, 0x0, 0xFF);

    /* Start to alter DIMM SPD data */
    if (skye_dimm_spd_write(dimm_no, offset, 1, &wdata) != PASSED) {
        printf("%s: Failed to read DIMM%d SPD.\n", __FUNCTION__, dimm_no);
        return (FAILED);
    }
#else
    /* SPD write utility is for debug purpose only because
     * write incorrect data to some SPD fields may damage DDR memory.
     * So take it out intentionally.
     */
    printf("\nSPD write utility is intentionally taken out.\n\n");
#endif

    return (PASSED);
}



/*-------------------------------------------------
$Log: skye_dimm.c,v $
Revision 1.2  2015/05/25 03:59:16  steja
Add Support Skye SM

Revision 1.1.4.2  2015/04/29 11:36:34  steja
Code check-in to skye-branch2 for ER code review


------------------------------------------------------------
Revision 1.1.2.1  2014/07/21 01:56:55  palin2
Initial check-in Skye module side Diag code.

---------------------------------------------------
skye_dimm.c:
Revision 1.2.8.1  2014/05/14 14:23:28  palin2
Intentionally take out SPD write utility to protect SPD data.

Revision 1.2  2014/02/27 15:01:44  palin2
Initial check-in ShrinkRay SM side Diag code.

Revision 1.1.4.7  2014/02/07 03:57:44  steja
code clean up

Revision 1.1.4.6  2013/12/06 09:39:44  iachang
Move DIMM Thermal sensor to skye_thermal.c
Support on-board Thermal sensor
Convert the measure to actual temperature

Revision 1.1.4.5  2013/11/13 08:20:09  palin2
1. Add write DIMM Thermal sensor register utility.
2. Update DIMM Thermal sensor register table.

Revision 1.1.4.4  2013/11/13 01:38:04  palin2
1. Add utilities to read & dump ShrinkRay DIMM thermal sensor register.
2. Also add specific I2C read/write function for ShrinkRay Thermal sensor.

Revision 1.1.4.3  2013/09/18 03:02:06  palin2
Update code based on code review comments.

Revision 1.1.4.2  2013/09/13 07:00:09  palin2
Initial check-in ShrinkRay SM side Diag code.

Revision 1.1.2.5  2013/07/14 22:03:12  palin2
Added ShrinkRay I2C write support and DDR DIMM SPD write utility.

Revision 1.1.2.4  2013/07/09 07:24:25  palin2
Create "skye_i2c_api.c" for ShrinkRay I2C APIs,
and move related I2C read/write function to it.

Revision 1.1.2.3  2013/07/02 08:44:15  palin2
1. Fixed complie issue caused by misused passing argument for function "close".
2. Clean up include files.

Revision 1.1.2.2  2013/07/02 08:16:16  palin2
Add "close" function to close the opened file descriptor before leave.

Revision 1.1.2.1  2013/07/02 03:14:24  palin2
Add support to dump DDR DIMM SPD info.

---------------------------------------------------
$Endlog$
*/

