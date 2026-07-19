/* $Id: platform_dimm.c,v 1.4 2013/11/26 08:40:37 hroni Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/cavium/platform_dimm.c,v $
 *------------------------------------------------------------------
 * platform_dimm.c - Overlord DIMM main function/menu.
 *
 * June 2011, Paul Lin
 * 
 * Ported from Xformers, original author is Simon Yen  
 *
 * Copyright (c) 2011-2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <assert.h>
#include <stdio.h>
#include "endians.h"
#include "common.h"
#include "types.h"
#include "defs.h"
#include "menu.h"
#include "fcntl.h"
#include "stdlib.h"
#include "signals.h"
#include "string.h"
#include "error.h"
#include "i2c_dev.h"
#include "i2c_api.h"
#include "n2g_api_rc.h"
#include "platform_i2c.h"
#include "platform_dimm.h"
#include "queryflags.h"

/******************************************************************************
 *                             Function protos
 ******************************************************************************/
static uint32_t dump_dimm_info(n2g_i2c_dev_t *, uint32_t);
static uint32_t dump_dimm_info_wrap(int);
static uint32_t dump_dimm_raw_data_wrap(int);

#ifdef DIMM_DEBUG
static uint32_t alter_dimm_data(n2g_i2c_dev_t *);
static uint32_t alter_dimm_data_wrap(int);
#endif

/******************************************************************************
 *                                Externs
 ******************************************************************************/
extern int get_board_rev (void);
extern uint32_t api_mb_i2c_read(n2g_i2c_dev_t *, uint32_t, uint8_t, char *);
extern uint32_t api_mb_i2c_write(n2g_i2c_dev_t *, uint32_t, uint8_t, char *);

extern int32_t cavium_i2c_fd0;

extern void msleep(int);
/******************************************************************************
 *                             Global Variables
 ******************************************************************************/


/******************************************************************************
 *                                 Menus
 ******************************************************************************/ 
/*
 * DIMM utility menu tables
 */
static submenu_xtable_t dimm_util_table[] = {
    {"Show Dimm info",          (PFT)dump_dimm_info_wrap,      TRUE,
     MF_CONTINUOUS,             (type_t(*)())0,                0,
     (type_t(*)())0,             0},
    {"Show Dimm data in RAW",   (PFT)dump_dimm_raw_data_wrap,  TRUE,
     MF_CONTINUOUS,             (type_t(*)())0,                0,
     (type_t(*)())0,             0},
#if 0  /* fixme */
    {"Alter Dimm RAW data",     (PFT)alter_dimm_data_wrap,     TRUE,
     MF_CONTINUOUS,             (type_t(*)())0,                0,
     (type_t(*)())0,             0},
#endif
};

#define DIMM_UTIL_TABLE_SZ \
        (sizeof(dimm_util_table) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t dimm_util_primary_items[DIMM_UTIL_TABLE_SZ + MAX_BASE_ITEMS];
static mitem_t dimm_util_secondary_items[DIMM_UTIL_TABLE_SZ + MAX_BASE_ITEMS];

static menuinfo_t dimm_util_menu = {
    "DIMM Utilities Menu",
    0,                            /* mtparam added by init_empty_menu */
    (PFT)show_endnote,            /* notes missing WICs in combos */
    0,                            /* use generic prompt */
    0,                            /* size (bumped by add_menu_item() */
    dimm_util_primary_items,
};

menuinfo_t *dimm_util_submenup = &dimm_util_menu;


/******************************************************************************
 *
 * Function   : build_plat_dimm_util_menu
 * Description: This the entry to Overlord Dimm utility menu
 * Inputs     : show_menu - PASSED: show utility submenu
 *                          FAILED: run all submenu option (disabled by default)
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************/
int build_plat_dimm_util_menu (int show_menu)
{
    int dummy;

    dummy = show_menu;

    build_primary_submenu(dimm_util_table, DIMM_UTIL_TABLE_SZ,
                          "DIMM Utilities Menu", &dimm_util_submenup);
    build_secondary_submenu(dimm_util_table, DIMM_UTIL_TABLE_SZ,
                            dimm_util_secondary_items);

    /* Display Utility Menu */
    menu(dimm_util_submenup, dimm_util_secondary_items, 0);

    return (PASSED);
}


/******************************************************************************
 *
 * Function   : dump_dimm_info
 * Description:	Provide platforms with a mechanism to dump DIMM information.
 * Inputs     : dev_object_t pointer to the SPD DIMM device
 *	            A command to decide the dump type
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************/
static uint32_t dump_dimm_info (n2g_i2c_dev_t *dev, uint32_t cmd)
{
    uint32_t ctr, rc = FAILED;
    uint32_t *dimm_p;
    uchar *dimm_ch_p;
    ddr3_dimm_t dimm;
    n2g_i2c_if_t i2c_if;


    /* Setup the interface struct for I2C API read */
    i2c_if.i2c_bus_type = dev->bus_no;
    i2c_if.i2c_dev = dev->dev_addr;

    /* Read the bytes from SPD DIMM */
    i2c_if.size = sizeof(uint32_t);	  /* Read 4 bytes at a time */
    for (ctr = 0, dimm_p = (uint32_t *)&dimm; ctr < sizeof(ddr3_dimm_t);
         ctr += sizeof(uint32_t), dimm_p++) {
        i2c_if.offset = ctr;
        i2c_if.buf = (char *)dimm_p;

        rc = api_mb_i2c_read(dev, i2c_if.offset, i2c_if.size, (char *)i2c_if.buf);
        if (rc != PASSED) {
            /* Read failed */
            if (rc == E_I2C_INV_ACK) {
                printf("\nDimm is not installed.\n");
            } else {
                printf("%s: I2C read offset %#.8x failed(return code = %#.8x).",
                       __FUNCTION__, ctr, rc);
            }
            return(FAILED);
        }
        msleep(10);
    }

    switch (cmd) {
    case DIMM_DUMP_ALL:
    /* Display the info */
    printf("\nNumber of SPD bytes used / bytes total and CRC Coverage = %#x\n",
           dimm.size);    
    printf("SPD Revision = %#x\n", dimm.revision);    
    printf("DRAM Device Type = %#x\n", dimm.dram_type);    
    printf("Module Type    = %#x\n", dimm.module_type);    
    printf("SDRAM Density and Banks = %#x\n", dimm.desity_bank);    
    printf("SDRAM Addressing = %0#x\n", dimm.addressing);    
    printf("Module Nominal Voltage, VDD = %d\n", dimm.vdd);    
    printf("Module Organization = %#x\n", dimm.organization);    
    printf("Module Memory Bus Width = %#x\n", dimm.bus_width);              
    printf("Fine Timebase(FTB) Dividend / Divisor = %#x\n", dimm.ftb_dividend);
    printf("Medium Timebase(MTB) Dividend = %#x\n", dimm.mtb_dividend);
    printf("Medium Timebase(MTB) Divisor = %#x\n", dimm.mtb_divisor);    
    printf("SDRAM Minimum Cycle Time(tCKmin) = %#x\n", dimm.tckmin);
    printf("CAS Latencies Supported, LSB = %#x\n", dimm.cas_lsb);
    printf("CAS Latencies Supported, MSB = %#x\n", dimm.cas_msb);
    printf("MIN CAS Latency(tAAmin) = %#x\n", dimm.taamin);
    printf("MIN Write Recovery(tWRmin) = %#x\n", dimm.twrmin);   
    printf("MIN RAS# to CAS# Delay(tRCDmin) = %#x\n", dimm.trcdmin);
    printf("MIN Row Act. to Row Act. Delay(tRRDmin) = %#x\n", dimm.trrdmin);    
    printf("MIN Row Precharge Delay(tRPmin) = %#x\n", dimm.trpmin);
    printf("Upper Nibbles for tRAS and tRC = %#x\n", dimm.u_nib_tras_trc);    
    printf("MIN Act. to Precharge Delay(tRASmin), LSB = %#x\n",
           dimm.trasmin_lsb);         
    printf("MIN Act. to Act./Refresh Delay(tRCmin), LSB = %#x\n",
           dimm.trcmin_lsb);              
    printf("MIN Refresh Recovery Delay(tRFCmin), LSB = %#x\n",
           dimm.trfcmin_lsb);              
    printf("MIN Refresh Recovery Delay(tRFCmin), MSB = %#x\n",
           dimm.trfcmin_msb);           
    printf("MIN Internal WR to RD Cmd Delay(tWTRmin) = %#x\n", dimm.twtrmin);
    printf("MIN Internal RD to Percharge Cmd Delay(tRTPmin) = %#x\n",
           dimm.trtpmin);
    printf("Upper Nibble for tFAW = %#x\n", dimm.u_nib_tfaw);
    printf("MIN Four Activate WIN Delay(tFAWmin) = %#x\n", dimm.tfawmin);
    printf("SDRAM Optional Features = %#x\n", dimm.opt_features);
    printf("SDRAM Thermal and Refresh Options = %#x\n", dimm.therm_ref_opt);
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
    
    for (ctr = 0, dimm_ch_p = (uchar *)&dimm; ctr < sizeof(ddr3_dimm_t);
         ctr++, dimm_ch_p++) {
        if ((ctr % 16) == 0) {
            printf("\n%.2x:", ctr);
        }
        printf(" %.2x", *dimm_ch_p);
    }
    printf("\n");
    break;
    default:
    assert(!"dimm_dev_show");
    break;
    }

    return(PASSED);
}


/******************************************************************************
 *
 * Function   :	dump_dimm_info_wrap
 * Description:	Dump DIMM info.
 * Inputs     :	None
 * Outputs    :	PASSED/FAILED
 *
 ******************************************************************************/
static uint32_t dump_dimm_info_wrap (int opt)
{
    uint32_t rc;
    n2g_i2c_dev_t i2c_dev;
    int dummy = 0;

    dummy = opt;

    i2c_dev.bus_no = CPU_I2C0;
    i2c_dev.dev_addr = MB_I2C_ADDR_DIMM0;
    i2c_dev.rd_hd_size = 1;
    i2c_dev.wr_hd_size = 1;

    /* Set I2C device to SLAVE mode */
    if (cavium_i2c_fd0 <= 0) {
         cterr('f', 0, "/dev/i2c-octeon.0/ is not opened correctly.");
         return (FAILED);
    } else {
        /* Set I2C device to SLAVE */
        if ((rc = ioctl(cavium_i2c_fd0, I2C_SLAVE, i2c_dev.dev_addr)) < 0) {
            cterr('f', 0, "%s at %s: unable to connect to device %#x. "
                          "rc = %#x", __FUNCTION__, __FILE__,
                          i2c_dev.dev_addr, rc);
            return (FAILED);
        } else {
            i2c_dev.fp = cavium_i2c_fd0;
        }
    }

    /* Display the registers */
    rc = dump_dimm_info(&i2c_dev, DIMM_DUMP_ALL);
    if (rc != PASSED) {
        cterr('f', 0, "Dump DIMM info failed(return code = %#x).", rc);
        return (rc);
    }
    return (PASSED);
}


/******************************************************************************
 *
 * Function   :	dump_dimm_raw_data_wrap
 * Description:	Wrap function to dump DIMM RAW data.
 * Inputs     : None.
 * Outputs    : PASSED/FAILED.
 *
 ******************************************************************************/
static uint32_t dump_dimm_raw_data_wrap (int opt)
{
    uint32_t rc = FAILED;
    n2g_i2c_dev_t i2c_dev;
    int dummy = 0;

    dummy = opt;

    i2c_dev.bus_no = CPU_I2C0;
    i2c_dev.dev_addr = MB_I2C_ADDR_DIMM0;
    i2c_dev.rd_hd_size = 1;
    i2c_dev.wr_hd_size = 1;

    /* Set I2C device to SLAVE mode */
    if (cavium_i2c_fd0 <= 0) {
         cterr('f', 0, "/dev/i2c-octeon.0/ is not opened correctly.");
         return (FAILED);
    } else {
        /* Set I2C device to SLAVE */
        if ((rc = ioctl(cavium_i2c_fd0, I2C_SLAVE, i2c_dev.dev_addr)) < 0) {
            cterr('f', 0, "%s at %s: unable to connect to device %#x. "
                          "rc = %#x", __FUNCTION__, __FILE__,
                          i2c_dev.dev_addr, rc);
            return (FAILED);
        } else {
            i2c_dev.fp = cavium_i2c_fd0;
        }
    }

    /* dump DIMM RAW data */
    rc = dump_dimm_info(&i2c_dev, DIMM_DUMP_RAW);

    return(rc);
}

#ifdef DIMM_DEBUG
/******************************************************************************
 *
 * Function   :	alter_dimm_data
 * Description:	Peek-n-poke AT24C0x byte location.
 * Inputs     :	dev_object_t pointer to the AT24C0x device
 * Outputs    :	PASSED/FAILED
 *
 ******************************************************************************/
static uint32_t alter_dimm_data (n2g_i2c_dev_t *dev)
{
    n2g_i2c_if_t i2c_if;
    uint32_t rc = FAILED;
    register char *c_ptr;
    int tmp;
    uint8_t val;
    char inbuf[3], done = FALSE;
    uint8_t old_data, new_data;
    uint16_t addr;

    /* Setup I2C API interface struct */
    i2c_if.size = sizeof(uint8_t);
    i2c_if.i2c_bus_type = dev->bus_no;
    i2c_if.i2c_dev = dev->dev_addr;

    /* Get the location to peek-n-poke */
    addr = gethex_answer("Enter the starting address:", 0, 0, 0xff);

    printf("Enter the data bytes. x or q to quit\n");

    while((addr <= 0xff) && (done == FALSE)) {
        /* Read the data first. */
        i2c_if.buf = &old_data;
        i2c_if.offset = addr;

        rc = api_mb_i2c_read(dev, i2c_if.offset, i2c_if.size, (char *)i2c_if.buf);
        if (rc != PASSED) {
            /* Read failed */
            if (rc == E_I2C_INV_ACK) {
                printf("\nDimm is not installed.\n");
            } else {
                printf("%s: I2C read offset %#.8x failed(return code = %#.8x).",
                       __FUNCTION__, addr, rc);
            }
            return(FAILED);
        }
        printf("%#.2x @ %#x ==> ", old_data, addr);

        c_ptr = inbuf;
	      fgets((char *)inbuf, sizeof(inbuf), stdin);

        switch (*c_ptr) {
        case 'x': /* exit */
        case 'q': /* quit */
        case 'X': /* exit */
        case 'Q': /* quit */
            done = TRUE;
	      break;
        case 0:	/* next location */
	      break;
        default:
            c_ptr = take_0x_addr(c_ptr);
	          tmp = getnum(c_ptr, 16, &val);
	          if (tmp == 0) {
		            printf("bad value \"%s\"\n", c_ptr);
		            continue;	/* Same location again */
	          } else {
		            new_data = (uint8_t)val;

		            /* Write the new data */
		            i2c_if.buf = &new_data;

		            rc = api_mb_i2c_write(dev, i2c_if.offset, 
                                      i2c_if.size, (char *)i2c_if.buf);
                if (rc != PASSED) {
                    /* Write failed */
                    if (rc == E_I2C_INV_ACK) {
                        printf("\nDimm is not installed.\n");
                    } else {
                        printf("%s: I2C read offset %#.8x failed"
                               "(return code = %#.8x).",
                               __FUNCTION__, addr, rc);
                    }
                    return (FAILED);
                }
                msleep(10);
            } /* endof if tmp */
        break; /* next location */
        } /* endof switch */
        addr++;
    } /* endof while */

    return (PASSED);
}
#endif
#ifdef DIMM_DEBUG
/******************************************************************************
 *
 * Function   :	alter_dimm_data_wrap
 * Description:	Alter 256-byte onboard DIMM SPD EEPROM.
 * Inputs     :	None
 * Outputs    :	PASSED/FAILED
 *
 ******************************************************************************/
static uint32_t alter_dimm_data_wrap (int opt)
{
    n2g_i2c_dev_t i2c_dev;
    uint32_t rc = FAILED;
    int dummy = 0;

    dummy = opt;

    i2c_dev.bus_no = CPU_I2C0;
    i2c_dev.dev_addr = MB_I2C_ADDR_DIMM0;
    i2c_dev.rd_hd_size = 1;
    i2c_dev.wr_hd_size = 1;

    /* Set I2C device to SLAVE mode */
    if (cavium_i2c_fd0 <= 0) {
         cterr('f', 0, "/dev/i2c-octeon.0/ is not opened correctly.");
         return (FAILED);
    } else {
        if ((rc = ioctl(cavium_i2c_fd0, I2C_SLAVE, i2c_dev.dev_addr)) < 0) {
            cterr('f', 0, "%s at %s: unable to connect to device %#x. "
                          "rc = %#x", __FUNCTION__, __FILE__,
                          i2c_dev.dev_addr, rc);
            return (FAILED);
        } else {
            i2c_dev.fp = cavium_i2c_fd0;
        }
    }

    /* Alter DIMM RAW data */
    rc = alter_dimm_data(&i2c_dev);

    return(rc);

}
#endif


/******** History ********
*----------------------------------------------------
$Log: platform_dimm.c,v $
Revision 1.4  2013/11/26 08:40:37  hroni
fix compiler warning

Revision 1.3  2012/06/06 09:57:28  iachang
Clean up complier warnings.

Revision 1.2  2012/03/28 00:38:18  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:07  ptong
Initial archive of ng_diag module


$Endlog$
*/
