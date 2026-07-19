/* $Id: diag_dimm_lib.c,v 1.2 2019/01/10 06:36:22 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/diag_dimm_lib.c,v $
 *------------------------------------------------------------------
 * 
 * diag_dimm_lib.c
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <assert.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include "endians.h"
#include "common.h"
#include "types.h"
#include "defs.h"
#include "menu.h"
#include "fcntl.h"
#include "proto.h"
#include "stdlib.h"
#include "signals.h"
#include "string.h"
#include "error.h"
#include "cross_platform.h"
#include "i2c_api.h"
#include "n2g_api_rc.h"
#include "platform_i2c.h"
#include "queryflags.h"
#include "tam_act2_api_drv_support.h"
#include "diag_i2c_lib.h"
#include "diag_dimm_lib.h"
#include "linux_main.h"


/******************************************************************************
 *                             Function protos
 ******************************************************************************/
int dump_dimm_data(n2g_i2c_dev_t *, uint32_t);
int init_dimm_i2c_struct(n2g_i2c_dev_t *);
int read_eeprom_block(unsigned int, unsigned int, uchar *);


/******************************************************************************
 *                             Global Variables
 ******************************************************************************/
static int i2c_fd0 = -1;

static n2g_i2c_if_t i2c_eeprom[] = {
    {
     .offset = 0,
     .i2c_bus_type = CPU_I2C0,
     .i2c_dev = MB_I2C_ADDR_EEPROM,
     .mux = MB_I2C_MUX_EEPROM,
     .i2c_ctrl = MB_I2C_CTRL_EEPROM,
     .sub_addr_len = 0,
     .size = sizeof(int16_t),   /* Read 2 bytes (16 bits) at a time */
     .rd_hd_size = 2,           /* not used */
     .wr_hd_size = 2,           /* not used */

     .buf = NULL,
     }
    ,
};


/******************************************************************************
 *
 * Function   : init_dimm_i2c_struct
 * Description: To init i2c_dev structure.
 * Inputs     : dev_object_t *i2c_dev
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************/
int init_dimm_i2c_struct (n2g_i2c_dev_t *i2c_dev)
{
    int rc = FAILED;

    i2c_dev->bus_no = CPU_I2C0;
    i2c_dev->rd_hd_size = HD_SIZE_2;
    i2c_dev->wr_hd_size = HD_SIZE_2;
    i2c_dev->dev_addr = MB_I2C_ADDR_EEPROM;
    i2c_fd0 = get_i2c_fd(0);

    /*
     * Set I2C device to SLAVE mode 
     */
    if (i2c_fd0 <= 0) {
        printf("%s:%d:/dev/i2c-0 is not opened correctly.\n", __FUNCTION__, __LINE__);
        return (FAILED);
    } else {
        if ((rc = ioctl(i2c_fd0, I2C_SLAVE, i2c_dev->dev_addr)) < 0) {
            printf("%s:%d:Unable to connect to device %#x. rc = %#x\n", 
                   __FUNCTION__, __LINE__, i2c_dev->dev_addr, rc);
            return (FAILED);
        } else {
            i2c_dev->fp = i2c_fd0;
        }
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : dump_dimm_data
 * Description: Provide platforms with a mechanism to dump DIMM information.
 * Inputs     : dev_object_t pointer to the SPD DIMM device
 *              A command to decide the dump type
 * Outputs    : PASSED/FAILED or MEMsize
 *
 *******************************************************************************
 */
int dump_dimm_data (n2g_i2c_dev_t * dev, uint32_t cmd)
{
    uint32_t ctr, rc = FAILED;
    uchar *dimm_p;
    uchar *dimm_ch_p;
    ddr3_dimm_t dimm;
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
    for (ctr = 0, dimm_p = (uchar *) & dimm; ctr < sizeof(ddr3_dimm_t);
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
             ctr < sizeof(ddr3_dimm_t); ctr++, dimm_ch_p++) {
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

/*********************************************************************
 *
 * Function:    read_eeprom_block
 *
 * Description: Display 256-byte EEPROM Registers.
 *
 * Inputs:
 *              offset -- reg offset
 *              size   -- data len
 *              buf    -- eeprom content
 *
 * Outputs: PASSED - No errors encounterd.
 *          FAILED - Errors encounterd.
 *
 * Assumptions:
 *
 *********************************************************************
 */
int read_eeprom_block (unsigned int offset,
                       unsigned int size, unsigned char *buf)
{
    unsigned int d32;
    n2g_i2c_if_t i2c_if;
    uint32_t rc, ix;

    memcpy(&i2c_if, &i2c_eeprom[0], sizeof(i2c_if));
    memset(buf, 0, size);
    i2c_if.buf = (char *) &d32;

    for (ix = offset; ix < size; ix++) {
        d32 = 0;
        i2c_if.offset = ix;
        rc = n2g_i2c_read(&i2c_if);
        if (rc != PASSED) {
            printf("%s:%d:unable to read from eeprom\n", 
                   __FUNCTION__, __LINE__);
            return FAILED;
        }
        buf[ix] = i2c_if.buf[0];
    }

    return (PASSED);

}

/*-------------------------------------------------
 * $Log: diag_dimm_lib.c,v $
 * Revision 1.2  2019/01/10 06:36:22  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
