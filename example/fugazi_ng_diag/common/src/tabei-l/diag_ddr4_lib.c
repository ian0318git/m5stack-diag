 /* $Id: diag_ddr4_lib.c,v 1.2 2019/10/17 02:16:20 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/diag_ddr4_lib.c,v $
 *------------------------------------------------------------------
 *
 * diag_ddr4_lib.c - This file is for ddr4 library
 *
 *
 * Copyright (c) 2008-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "endians.h"
#include "defs.h"
#include "fcntl.h"
#include "proto.h"
#include "types.h"
#include "nvsysvars.h"
#include "error.h"
#include "pcmap.h"
#include "strings.h"
#include "menu.h"
#include "common.h"
#include "cross_platform.h"
#include "i2c_api.h"
#include "n2g_api_rc.h"
#include "diag_ddr4_lib.h"
#include "diag_i2c_lib.h"
#include "plat_defs.h"
#include "queryflags.h"
#include "diag_common.h"


/******************************************************************************
 *                             Function protos
 ******************************************************************************/
static uint32_t dump_dimm_info(n2g_i2c_dev_t *, uint32_t);
static uint32_t dump_dimm_info_wrap(int);
static uint32_t dump_dimm_raw_data_wrap(int);


/******************************************************************************
 *                             Global Variables
 ******************************************************************************/
uint32_t dimm_no = UNKNOWN_DIMM_NO;

/******************************************************************************
 *                                 Menus
 ******************************************************************************/
/*
 * DIMM utility menu tables
 */
static submenu_xtable_t dimm_util_table[] = {
    {"Show Dimm info", (PFT) dump_dimm_info_wrap, TRUE,
     MM_3, (type_t(*)())0, 0,
     (type_t(*)())0, 0},
    {"Show Dimm data in RAW", (PFT) dump_dimm_raw_data_wrap, TRUE,
     MM_3, (type_t(*)())0, 0,
     (type_t(*)())0, 0},
};

#define DIMM_UTIL_TABLE_SZ \
        (sizeof(dimm_util_table) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t dimm_util_primary_items[DIMM_UTIL_TABLE_SZ +
                                       MAX_BASE_ITEMS];
static mitem_t dimm_util_secondary_items[DIMM_UTIL_TABLE_SZ +
                                         MAX_BASE_ITEMS];

static menuinfo_t dimm_util_menu = {
    "DIMM Utilities Menu",
    0,                          /* mtparam added by init_empty_menu */
    (PFT) show_endnote,         /* notes missing WICs in combos */
    0,                          /* use generic prompt */
    0,                          /* size (bumped by add_menu_item() */
    dimm_util_primary_items,
};

menuinfo_t *dimm_util_submenup = &dimm_util_menu;


/******************************************************************************
 *
 * Function   : build_plat_dimm_util_menu
 * Description: This the entry to Dimm utility menu
 * Inputs     : dimm - DIMM number
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************/
int build_plat_dimm_util_menu (int dimm)
{
    char *tname = "DIMM";

    testname("%s", tname);

    build_primary_submenu(dimm_util_table, DIMM_UTIL_TABLE_SZ,
                          "DIMM Utilities Menu", &dimm_util_submenup);
    build_secondary_submenu(dimm_util_table, DIMM_UTIL_TABLE_SZ,
                            dimm_util_secondary_items);

    /*
     * Setup DIMM number 
     */
    dimm_no = dimm;

    /*
     * Display Utility Menu 
     */
    menu(dimm_util_submenup, dimm_util_secondary_items, 0);

    return (PASSED);
}


/******************************************************************************
 *
 * Function   : dump_dimm_info
 * Description: Provide platforms with a mechanism to dump DIMM information.
 * Inputs     : dev_object_t pointer to the SPD DIMM device
 *              A command to decide the dump type
 * Outputs    : PASSED/FAILED or MEMsize
 *
 ******************************************************************************/
static uint32_t dump_dimm_info (n2g_i2c_dev_t * dev, uint32_t cmd)
{
    uint32_t ctr, rc = FAILED;
    uchar *dimm_p;
    uchar *dimm_ch_p;
    ddr4_dimm_t dimm;
    n2g_i2c_if_t i2c_if;

    /*
     * Setup the interface struct for I2C API read 
     */
    i2c_if.i2c_bus_type = dev->bus_no;
    i2c_if.i2c_dev = dev->dev_addr;

    /*
     * Read the bytes from SPD DIMM 
     */
    i2c_if.size = sizeof(uchar);        /* Read 1 byte at a time */
    for (ctr = 0, dimm_p = (uchar *) & dimm; ctr < sizeof(ddr4_dimm_t);
         ctr += sizeof(uchar), dimm_p++) {
        i2c_if.offset = ctr;
        i2c_if.buf = (char *) dimm_p;

        rc = api_mb_i2c_read(dev, i2c_if.offset, i2c_if.size, i2c_if.buf);
        if (rc != PASSED) {
            /*
             * Read failed 
             */
            if (rc == E_I2C_INV_ACK) {
                printf("\nDimm is not installed.\n");
            } else {
                printf("%s: I2C read offset %#.8x failed(return code = %#.8x).",
                     __FUNCTION__, ctr, rc);
            }
            return (FAILED);
        }
        msleep(10);
    }

    switch (cmd) {
    case DIMM_DUMP_ALL:
        /*
         * Display the info 
         */
        printf
            ("\nNumber of SPD bytes used / bytes total and CRC Coverage = %#x\n",
             dimm.size);
        printf("SPD Revision = %#x\n", dimm.revision);
        printf("DRAM Device Type = %#x\n", dimm.dram_type);
        printf("Module Type    = %#x\n", dimm.module_type);
        printf("SDRAM Density and Banks = %#x\n", dimm.desity_bank);
        printf("SDRAM Addressing = %0#x\n", dimm.addressing);
        printf("Module Nominal Voltage, VDD = %d\n", dimm.vdd);
        printf("Module Organization = %#x\n", dimm.organization);
        printf("Module Memory Bus Width = %#x\n", dimm.bus_width);
        printf("Fine Timebase(FTB) Dividend / Divisor = %#x\n",
               dimm.ftb_dividend);
        printf("Medium Timebase(MTB) Dividend = %#x\n", dimm.mtb_dividend);
        printf("Medium Timebase(MTB) Divisor = %#x\n", dimm.mtb_divisor);
        printf("SDRAM Minimum Cycle Time(tCKmin) = %#x\n", dimm.tckmin);
        printf("CAS Latencies Supported, LSB = %#x\n", dimm.cas_lsb);
        printf("CAS Latencies Supported, MSB = %#x\n", dimm.cas_msb);
        printf("MIN CAS Latency(tAAmin) = %#x\n", dimm.taamin);
        printf("MIN Write Recovery(tWRmin) = %#x\n", dimm.twrmin);
        printf("MIN RAS# to CAS# Delay(tRCDmin) = %#x\n", dimm.trcdmin);
        printf("MIN Row Act. to Row Act. Delay(tRRDmin) = %#x\n",
               dimm.trrdmin);
        printf("MIN Row Precharge Delay(tRPmin) = %#x\n", dimm.trpmin);
        printf("Upper Nibbles for tRAS and tRC = %#x\n",
               dimm.u_nib_tras_trc);
        printf("MIN Act. to Precharge Delay(tRASmin), LSB = %#x\n",
               dimm.trasmin_lsb);
        printf("MIN Act. to Act./Refresh Delay(tRCmin), LSB = %#x\n",
               dimm.trcmin_lsb);
        printf("MIN Refresh Recovery Delay(tRFCmin), LSB = %#x\n",
               dimm.trfcmin_lsb);
        printf("MIN Refresh Recovery Delay(tRFCmin), MSB = %#x\n",
               dimm.trfcmin_msb);
        printf("MIN Internal WR to RD Cmd Delay(tWTRmin) = %#x\n",
               dimm.twtrmin);
        printf("MIN Internal RD to Percharge Cmd Delay(tRTPmin) = %#x\n",
               dimm.trtpmin);
        printf("Upper Nibble for tFAW = %#x\n", dimm.u_nib_tfaw);
        printf("MIN Four Activate WIN Delay(tFAWmin) = %#x\n",
               dimm.tfawmin);
        printf("SDRAM Optional Features = %#x\n", dimm.opt_features);
        printf("SDRAM Thermal and Refresh Options = %#x\n",
               dimm.therm_ref_opt);
        printf("Module Thermal Sensor = %#x\n", dimm.therm_sensor);
        printf("SDRAM Device Type = %#x\n", dimm.sdram_type);
        printf("Type Spec Sec., Index. by Key Byte 3 = ");
        for (ctr = 0; ctr < sizeof(dimm.type_spec); ctr++) {
            printf("%#x ", dimm.type_spec[ctr]);
        }
        printf("\n");
        printf("Module Manufacturer's JEDEC ID Code = ");
        for (ctr = 0; ctr < sizeof(dimm.mod_jedec_id); ctr++) {
            printf("%#x ", dimm.mod_jedec_id[ctr]);
        }
        printf("\n");
        printf("Manufacturer's Location = %#x\n", dimm.manu_location);
        printf("Manufacturer's Date = ");
        for (ctr = 0; ctr < sizeof(dimm.manu_date); ctr++) {
            printf("%#x ", dimm.manu_date[ctr]);
        }
        printf("\n");
        printf("Module Serial Number = ");
        for (ctr = 0; ctr < sizeof(dimm.serial_no); ctr++) {
            printf("%#x ", dimm.serial_no[ctr]);
        }
        printf("\n");
        printf("Cyclical Redundancy Code = ");
        for (ctr = 0; ctr < sizeof(dimm.crc); ctr++) {
            printf("%#x ", dimm.crc[ctr]);
        }
        printf("\n");
        printf("Module Part Number = ");
        for (ctr = 0; ctr < sizeof(dimm.part_no); ctr++) {
            printf("%#x ", dimm.part_no[ctr]);
        }
        printf("\n");
        printf("Module Revision Code = ");
        for (ctr = 0; ctr < sizeof(dimm.rev_code); ctr++) {
            printf("%#x ", dimm.rev_code[ctr]);
        }
        printf("\n");
        printf("DRAM Manufacturer's JEDEC ID Code = ");
        for (ctr = 0; ctr < sizeof(dimm.dram_jedec_id); ctr++) {
            printf("%#x ", dimm.dram_jedec_id[ctr]);
        }
        printf("\n");
        printf("Manufacturer's Specific Data = ");
        for (ctr = 0; ctr < sizeof(dimm.manu_spec); ctr++) {
            printf("%#x ", dimm.manu_spec[ctr]);
        }
        printf("\n");
        printf("Open for customer use = ");
        for (ctr = 0; ctr < sizeof(dimm.customer_use); ctr++) {
            printf("%#x ", dimm.customer_use[ctr]);
        }
        printf("\n\n");
        break;

    case DIMM_DUMP_RAW:

        printf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f");

        for (ctr = 0, dimm_ch_p = (uchar *) & dimm;
             ctr < sizeof(ddr4_dimm_t); ctr++, dimm_ch_p++) {
            if ((ctr % 16) == 0) {
                printf("\n%.2x:", ctr);
            }
            printf(" %.2x", *dimm_ch_p);
        }
        printf("\n");
        break;

    case DIMM_DUMP_MEMSIZE:
        /*
         * return memsize direcctly 
         */
        return ((uint32_t) dimm.desity_bank);
        break;

    default:
        assert(!"dimm_dev_show");
        break;
    }
    return (PASSED);
}


/******************************************************************************
 *
 * Function   :    diag_ddr4_dimm_util
 * Description:    Dump DIMM info.
 * Inputs     :    opt - future use
 * Outputs    :    PASSED/FAILED
 *
 ******************************************************************************/
uint32_t dump_dimm_info_wrap (int opt)
{
    uint32_t rc = FAILED;
    n2g_i2c_dev_t i2c_dev;

    /*
     * Init I2C device structure 
     */
    rc = init_dimm_i2c_struct(&i2c_dev);
    if (rc != PASSED) {
        cterr('f', 0, "Init DIMM i2c_dev struct failed"
              "(return code = %#x).", rc);
        return (rc);
    }

    /*
     * Display the registers 
     */
    rc = dump_dimm_info(&i2c_dev, DIMM_DUMP_ALL);
    if (rc != PASSED) {
        cterr('f', 0, "Dump DIMM info failed(return code = %#x).", rc);
    }
    return (rc);
}

/******************************************************************************
 *
 * Function   :    dump_dimm_raw_data_wrap
 * Description:    Wrap function to dump DIMM RAW data.
 * Inputs     :    opt - future use
 * Outputs    :    PASSED/FAILED.
 *
 ******************************************************************************/
static uint32_t dump_dimm_raw_data_wrap(int opt)
{
    uint32_t rc = FAILED;
    n2g_i2c_dev_t i2c_dev;

    /*
     * Init I2C device structure 
     */
    rc = init_dimm_i2c_struct(&i2c_dev);
    if (rc != PASSED) {
        cterr('f', 0, "Init DIMM i2c_dev struct failed"
              "(return code = %#x).", rc);
        return (rc);
    }

    /*
     * dump DIMM RAW data 
     */
    rc = dump_dimm_info(&i2c_dev, DIMM_DUMP_RAW);
    if (rc != PASSED) {
        cterr('f', 0, "Dump DIMM RAW info failed(return code = %#x).", rc);
    }
    return (rc);
}

/******** History ********
$Log: diag_ddr4_lib.c,v $
Revision 1.2  2019/10/17 02:16:20  kehuang2
Collapse Tabei-L into main trunk

Revision 1.1.2.3  2019/03/07 05:53:15  olin2
Clean up code

Revision 1.1.2.2  2018/11/16 05:42:09  olin2
Clean up code

Revision 1.1.2.1  2018/10/02 01:49:58  harrchan
Initial commit for Tabei-L P1A bring up.

$Endlog$
*/
