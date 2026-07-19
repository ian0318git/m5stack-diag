/* $Id: platform_sfp_cookie.c,v 1.2 2013/10/08 08:48:30 tirawan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn_linux/platform_sfp_cookie.c,v $
 *------------------------------------------------------------------
 * platform_sfp_cookie.c
 *
 * Description: Overlord SFP Cookie I2C device.
 *              This file is ported from Informers. 
 *
 * Copyright (c) 2011-2013 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdlib.h>
#include "endians.h"
#include "common.h"
#include "platform_sfp_cookie.h"
#include "defs.h"
#include "proto.h"
#include "error.h"
#include "cross_platform.h"
#include "pca9545a.h"
#include "platform_i2c.h"
#include "menu.h"
#include "n2g_api_rc.h"
#include "i2c_dev.h"
#include "i2c_api.h"
#include "diag_fpga_lib.h"
#include "diag_xaui_88X2222M_lib.h"
#include "platform_eth.h"

/******************************************************************************
 *                                   Externs
 ******************************************************************************/


/****************************************************************************** 
 *                               Function prototypes
 ******************************************************************************/

int dump_sfp_eeprom(int);
int fpga_sfp_i2c_read_test (void);    

/****************************************************************************** 
 *                                 Global variables
 ******************************************************************************/

static int sku_6gesfp_port_map[] = {4,5,0,1,2,3};
static uint port_addr_map_4ge_sku[] = {SFP_0, SFP_1, SFP_2, SFP_3, SFP_PLUS};
static uint port_addr_map_6ge_sku[] = {SFP_4, SFP_5,SFP_0, SFP_1, SFP_2, SFP_3}; 

/******************************************************************************
 *                                   Menus
 ******************************************************************************/


/******************************************************************************
 *
 * function   : dump_sfp_eeprom
 * Description:	Show sfp module content.
 * Inputs     : sfp - SFP I2C device number
 * Outputs    : PASSED/FAILED.
 *
 ******************************************************************************/
int dump_sfp_eeprom (int sfp)
{
    uint32 rv;
    uchar burst_buf[I2C_BURST_SIZE];
    uint sfp_i2c_addr, reg_addr; 
    int fpga_addr;
    char write_data;
    n2g_i2c_dev_t i2c_dev;
    int ix, sku_id, sfp_port;

    /* Turn off 88X2222M I2C */
    if (turn_off_mrvl_88X2222M_i2c() == FAILED) {
        printf("Close 88X2222M I2C failed\n");
        return (FAILED);
    }
    
    /* Get the SKU id */
    sku_id = get_sku_id();

    /* Check if SFP+ is available at this SKU */
    if (sfp ==6 && sku_id == WOODLAWN_6GE) {
        printf("This SKU doesn't have SFP+\n");
        return (PASSED);
    } 

     /* Check if SFP is available at this SKU */
    if ((sfp == 4 || sfp ==5) && sku_id == WOODLAWN_4GE_1XAUI) {
        printf("This SKU doesn't have SFP %d!\n", sfp);
        return (PASSED);
    } 

    /* Need to adjust port number for 6GE SKU
     * Port 0 -> GEP1_G0 SFP
     * Port 1 -> GEP1_G1 SFP
     * Port 2 -> GEP0_G0 SFP
     * Port 3 -> GEP0_G1 SFP
     * Port 4 -> GEP0_G2 SFP
     * Port 5 -> GEP0_G3 SFP
     */
    if (sku_id == WOODLAWN_6GE) {
        sfp_port = sku_6gesfp_port_map[sfp];
    } else {
        sfp_port = sfp;
    }

    if (sfp_port == 6) {
        /* Check if SFP+ is present */
        if (mrvl_88X2222M_is_sfp_plus_present() == FALSE) {
            printf("SFP+ is not detected\n");
            return (PASSED);
        }
    } else {
        if (is_sfp_present(0, sfp_port) == FALSE) {
            printf("SFP-%d is not detected\n", sfp);
            return (PASSED);
        }
    }
        
    /* Claim I2C ownership */
    fpga_addr = 0x20; 
    write_data = 0x02;

    rv = fpga_reg_write(fpga_addr, write_data); 
    if (rv != PASSED) {
        printf("%s:%d Failed to write FPGA offset 0x%.8x.",
                __FUNCTION__, __LINE__, fpga_addr);
        return (FAILED);
    } 

    fpga_reg_read(fpga_addr, &write_data);
    if (write_data != 0x02) {
        printf("Failed to claim I2C ownership (%02x)\n", write_data);
        return (FAILED);
    }
        
    switch (sfp_port) {
        case 0:
            sfp_i2c_addr = SFP_0;
            break;
        case 1:
            sfp_i2c_addr = SFP_1;
            break;
        case 2:
            sfp_i2c_addr = SFP_2;
            break;
        case 3:
            sfp_i2c_addr = SFP_3;
            break;
        case 4:
            sfp_i2c_addr = SFP_4;
            break;
        case 5:
            sfp_i2c_addr = SFP_5;
            break;
        case 6:
            sfp_i2c_addr = SFP_PLUS;
            break;
        default :
            cterr('f', 0, "not support this SFP port num,", sfp);
            return (FAILED);
    }

    /* Open the Cavium I2C bus 1 */
    if (open_i2c(&i2c_dev, sfp_i2c_addr, CPU_I2C1) == FAILED) {
        cterr('f', 0, "Fail to open the i2c interface.");
        return (FAILED);
    }

    if (sfp == 6 && sku_id == WOODLAWN_4GE_1XAUI) {
        printf("SFP+ EEPROM contents:\n\n");
    } else {
        printf("SFP-%d EEPROM contents:\n\n", sfp);
    }

    printf("0x00: ");
    for (reg_addr = 0; reg_addr < SFP_EEPROM_SIZE; reg_addr += I2C_BURST_SIZE) {
        if (reg_addr && ((reg_addr % 16) == 0)) {
            printf("\n");
            printf("0x%02x: ", reg_addr);
        }

        rv = read_i2c_reg(&i2c_dev, (uchar *)&burst_buf, reg_addr,
                          I2C_BURST_SIZE);
        if (rv != PASSED) {
            printf("Read reg fail at offset 0x%.8x\n", reg_addr);
            return (FAILED);
        } else {
            for (ix = 0; ix < I2C_BURST_SIZE; ix++) {
                printf("%02x ", burst_buf[ix]);
            }
        }
    }
    printf("\n");

    /* CPU release the ownership */
    write_data = 0x0;
    rv = fpga_reg_write(fpga_addr, write_data); 
    if (rv != PASSED) {
        cterr('f', 0, "%s:%d Failed to write FPGA offset 0x%.8x.",
                      __FUNCTION__, __LINE__, fpga_addr);
        return (FAILED);
    }
    
    return (PASSED);
}

/******************************************************************************
 *
 * function   : fpga_sfp_i2c_read_test
 * Description: Test the path between fpga and sfp/sfp+ module.
 * Inputs     : none
 * Outputs    : PASSED/FAILED.
 *
 ******************************************************************************/
int fpga_sfp_i2c_read_test (void)
{
    uint32 rv;
    uchar burst_buf[I2C_BURST_SIZE];
    uint *sfp_i2c_addr, reg_addr; 
    int fpga_addr;
    char write_data;
    n2g_i2c_dev_t i2c_dev;
    int ix, sku_id, sfp_port, module_num, err_num;
    int ret_val = PASSED;

    /* Turn off 88X2222M I2C */
    if (turn_off_mrvl_88X2222M_i2c() == FAILED) {
        printf("Close 88X2222M I2C failed\n");
        return (FAILED);
    }
    
    /* Get the SKU id */
    sku_id = get_sku_id();

    /* Need to adjust port number for 6GE SKU
     * Port 0 -> GEP1_G0 SFP
     * Port 1 -> GEP1_G1 SFP
     * Port 2 -> GEP0_G0 SFP
     * Port 3 -> GEP0_G1 SFP
     * Port 4 -> GEP0_G2 SFP
     * Port 5 -> GEP0_G3 SFP
     */
    if ((check_ext_lpbk_flag()) && (sku_id == WOODLAWN_4GE_1XAUI)) { /* check 4ge sku */
        module_num = 5;
        sfp_i2c_addr = port_addr_map_4ge_sku;
        
        /* Check if SFP+ is present */
        if (mrvl_88X2222M_is_sfp_plus_present() == FALSE) {
            cterr('f', 0, "SFP+ is not detected");
            return (FAILED);
        } else {
            for (sfp_port = 0; sfp_port < (module_num - 1); sfp_port++) {
                if (is_sfp_present(0, sfp_port) == FALSE) {
                    cterr('f', 0, "SFP-%d is not detected", sfp_port);
                    return (FAILED);
                }
            }
        }
    } else if ((check_ext_lpbk_flag()) && (sku_id == WOODLAWN_6GE)) { /* check 6ge sku */
        module_num = 6;
        sfp_i2c_addr = port_addr_map_6ge_sku;
        
        for (sfp_port = 0; sfp_port < module_num; sfp_port++) {
            if (is_sfp_present(0, sku_6gesfp_port_map[sfp_port]) == FALSE) {
                cterr('f', 0, "SFP-%d is not detected", sfp_port);
                return (PASSED);
            }
        }
    } else {
        printf("EXT lpbk flag not turn on or sku id is not correct\n");
        return (PASSED);
    }

    /* Claim I2C ownership */
    fpga_addr = 0x20; 
    write_data = 0x02;

    rv = fpga_reg_write(fpga_addr, write_data); 
    if (rv != PASSED) {
        cterr('f', 0, "%s:%d Failed to write FPGA offset 0x%.8x.",
                __FUNCTION__, __LINE__, fpga_addr);
        return (PASSED);
    } 

    fpga_reg_read(fpga_addr, &write_data);
    if (write_data != 0x02) {
        cterr('f', 0, "Failed to claim I2C ownership (%02x)\n", write_data);
        return (FAILED);
    }

    for (sfp_port = 0; sfp_port < module_num; sfp_port++) {
        err_num = 0;
        /* Open the Cavium I2C bus 1 */
        if (open_i2c(&i2c_dev, sfp_i2c_addr[sfp_port], CPU_I2C1) == FAILED) {
            cterr('f', 0, "Fail to open the i2c interface.");
            return (FAILED);
        }

        for (reg_addr = 0; reg_addr < FPGA_SFP_PATH_TEST_SIZE; reg_addr += I2C_BURST_SIZE) {
            rv = read_i2c_reg(&i2c_dev, (uchar *)&burst_buf, reg_addr, I2C_BURST_SIZE);
            if (rv != PASSED) {
                cterr('f', 0, "Read reg fail at offset 0x%.8x\n", reg_addr);
                return (FAILED);
            } else {
                for (ix = 0; ix < I2C_BURST_SIZE; ix++) {
                    if (burst_buf[ix] == 0xff) {
                        err_num ++;
                    } 
                }
            }
        }

        if (err_num == FPGA_SFP_PATH_TEST_SIZE) {
            cterr('f', 0, "Read %d byte with val 0xff at slave addr %#.2x", err_num, 
                       sfp_i2c_addr[sfp_port]);
            ret_val = FAILED;
        }
    }

    /* CPU release the ownership */
    write_data = 0x0;
    rv = fpga_reg_write(fpga_addr, write_data); 
    if (rv != PASSED) {
        cterr('f', 0, "%s:%d Failed to write FPGA offset 0x%.8x.",
                      __FUNCTION__, __LINE__, fpga_addr);
        return (FAILED);
    }
    
    return (ret_val);
}
/*------------------------------------------------------------------
 * $Log: platform_sfp_cookie.c,v $
 * Revision 1.2  2013/10/08 08:48:30  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.4.2  2013/08/20 10:59:10  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.2  2013/05/20 06:26:02  leschen
 * Add test item to test the path between fpga and sfp/sfp+
 *
 * Revision 1.1.2.1  2013/04/24 10:37:24  tirawan
 * Initial check-in for woodlawn linux code
 *
 * Revision 1.7  2013/04/10 11:16:24  leslie
 * Use FPGA to dump sfp+ contents
 *
 * Revision 1.6  2013/03/27 08:45:06  kuangik
 * Code cleanup
 *
 * Revision 1.4  2013/03/20 10:34:23  kuangik
 * Remove SFP+ eeprom
 *
 * Revision 1.2  2013/03/19 03:23:28  kuangik
 * Correct is_sfp_present for 6GE SKU
 *
 * Revision 1.7  2012/10/24 10:44:05  leslie
 * Fix and clean up code.
 *
 * Revision 1.6  2012/08/30 06:39:10  leslie
 * Fix the issue of dump sfp eeprom.
 *
 * Revision 1.5  2012/08/18 02:51:09  leslie
 * Modify dump sfp eeprom library to open i2c bus 1
 *
 * Revision 1.4  2012/08/03 10:16:56  evanli
 * Mapping to latest O2 source code on 20120726
 *
 * Revision 1.2  2012/07/19 06:57:30  leslie
 * Add dump sfp eeprom code library.
 *
 * Revision 1.1.1.1  2012/02/10 05:59:50  kody
 * Initial imports Woodlawn project code base.
 *
 * Revision 1.1.2.4  2011/11/04 01:25:46  ptong
 * Add setup_i2c_dev_structure in show_cookie_wrap
 *
 * Revision 1.1.2.3  2011/11/03 15:01:58  palin2
 * Updated Cavium I2C MUX and SFP related test and utilities.
 *
 * Revision 1.1.2.2  2011/10/14 15:00:41  palin2
 * Fixed Overlord cavium part compile error caused by the change of "n2g_i2c_if_t" member name.
 *
 * Revision 1.1.2.1  2011/08/26 08:34:54  palin2
 * Added SFP tests support to Overlord Cavium.
 *
 * Revision 1.1.2.1  2011/03/24 06:59:50  mcharon
 * create overlord
 *
 * Revision 1.1.2.1  2011/03/11 22:33:57  mcharon
 * iniitail support dyno
 *
 * Revision 1.3  2010/06/28 16:47:09  siyen
 * Changes sequence of GLC-GE100FX to FX then Edge rate (CSCte67711. PRRQ 1215362)
 *
 * Revision 1.2  2010/05/28 01:20:59  siyen
 * Added Rate Control registers setup for GLC-GE-100FX SFP (CSCte67711)
 *
 * Revision 1.1.1.1  2009/10/17 02:05:55  huyhoang
 * Initial archive of diaglinux module
 *
 * Revision 1.1.8.4.2.1  2009/10/14 01:14:30  huyhoang
 * + Add DiagLinux support to ngd-diag-rep repository
 *
 * Revision 1.1.8.4  2009/07/06 17:34:37  mcharon
 * change sleep to msleep....getline to get_line
 *
 * Revision 1.1.8.3  2009/06/05 03:01:21  sctsai
 * Sync with informers2-tag-060509 repository.
 *
 * Revision 1.1.8.2  2009/06/04 09:37:55  sctsai
 * Sync with informers2-tag-060209 repository.
 *
 * Revision 1.1.6.5  2009/06/03 17:13:38  siyen
 * Added GLC-GE-100FX SFP registers read and Timbercon electrical SFP cookie change.
 *
 * Revision 1.1.6.4  2009/06/01 17:33:26  siyen
 * Added I2C bit-bang for GLC-GE-100FX supports.
 *
 * Revision 1.1.6.3  2009/04/09 17:23:17  sctsai
 * The changes:
 * 1.Create a new header file "endians.h" to support both gcc & icc.
 *   .c file should include it at the beginning, if need.
 * 2.Change INFORMERS_ICC to INTEL_ICC
 * 3.Move all x86 related files to le directory.
 * 4.Temporarily, change FPGA path to /auto/sp-engops/diags/pld_icc.
 *
 * Revision 1.1.6.2  2009/04/07 21:53:34  sctsai
 * Sync with informers-tag-040609 repository.
 *
 * Revision 1.1.6.1  2009/02/18 02:57:19  sctsai
 * Sync informers-tag-021609 to informers2-branch.
 *
 * Revision 1.1.2.3  2009/03/19 02:43:27  siyen
 * Added SFP present detect.
 *
 * Revision 1.1.2.2  2008/11/18 00:38:52  siyen
 * GE PHY/MAC NIC Supports.
 *
 * Revision 1.1.2.1  2008/10/22 21:02:01  siyen
 * Initial check-in.
 *
 *------------------------------------------------------------------
 * $Endlog$
 *------------------------------------------------------------------
 */
