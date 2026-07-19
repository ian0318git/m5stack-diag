/* $Id: diag_tlk10232_lib.c,v 1.6 2017/09/27 01:56:30 leschen Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn_linux/diag_tlk10232_lib.c,v $
 *-----------------------------------------------------------------------------
 * diag_tlk10232_lib.c - Utility Menu and Functions for Woodlawn TLK10232
 *
 * January 2013, Leslie Chen
 * Copyright (c) 2017 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <linux/types.h>
#include <linux/socket.h>
#include <linux/ioctl.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <net/if.h>
#include <netinet/in.h>
#include <linux/sockios.h>
#include <linux/mii.h>

#include "defs.h"
#include "types.h"
#include "common.h"
#include "monitor.h"
#include "cross_platform.h"
#include "menu.h"
#include "error.h"
#include "router_if.h"
#include "sgmii_defs.h"
#include "cvmx.h"
#include "queryflags.h"
#include "ethernet.h"
#include "linux_eth.h"
#include "platform_eth.h"
#include "cvmx-mdio.h"

#include "platform_smi_lib.h"
#include "diag_tlk10232_lib.h"
#include "diag_fpga_lib.h"

int config_tlk_10232_mode(int);
int read_tlk_10232_reg(ulong, int, ulong *, void *);
int write_tlk_10232_reg(ulong, int, ulong, void *);
int tlk10232_xaui_to_xaui_configuration(void);
int set_tlk10232_lpbk_bit(int);
int tlk10232_mode_select(void);
int tlk10232_global_reset (void);
int tlk10232_path_reset(void);
int tlk10232_kr_configuration(void);
int is_10gkr_capable(void);
void run_tlk10232_script(void);

extern void msleep(unsigned long);

/******************************************************************************
 *
 * Function: config_tlk_10232_mode
 *
 * Description: This function config tlk_10232 to operate in XAUI mode or 10GBASE-KR mode.
 *                    1. Internal loopback test path : cavium <-> CH. B
 *                    2. Backplane loopback test path : cavium <-> CH .A <-> CH. B
 *                    3. TLK10232 Default mode : XAUI CH. A <-> 10G-KR
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int config_tlk_10232_mode (int mode)
{
    int bus_id, phy_id, dev_id, reg_addr, val, mii_value; 
    bus_id = TLK_10232_SMI2_ADDR;
    
    switch (mode) {
        case XAUIB_TO_10GKR: 
            /* Set up CHB data path. 0x1E.0x001A. val 0x4c20 */
            phy_id = TLK_10232_PHY_ADDR_CHANNEL_B;
            dev_id = TLK_10232_CHANNEL_CTRL_1_DEV;
            reg_addr = TLK_10232_DSR_CONTROL_2_REG;

            /* Switch CHB to 10G-KR data path 
             * Woodlawn Cavium <-XAUI-> TLK10232 <-10GKR-> Platform GE-Switch
             * no need to re-execute 10G-KR init script, it has a possibility to casue ethernet port link down */
            mii_value = cvmx_mdio_45_write(bus_id, phy_id, dev_id, reg_addr, DSR_CONTROL_2_REG_DEFAULT_VAL);
            break;
        case XAUIB_TO_XAUIB:  /* Cavium <-> TLK10232 CHB */
            /* Set up CHB data path. 0x1E.0x001A. val 0x4c20->0x2c20 */
            phy_id = TLK_10232_PHY_ADDR_CHANNEL_B;
            dev_id = TLK_10232_CHANNEL_CTRL_1_DEV;
            reg_addr = TLK_10232_DSR_CONTROL_2_REG;
            mii_value = cvmx_mdio_45_read(bus_id, phy_id, dev_id, reg_addr);
            val = (mii_value & (~TLK_10232_DSR_CONTROL_2_MASK_VAL_1)) | TLK_10232_DSR_ANY_DATA;

            /* Switch CHB to TLK10232 internal XAUI data path 
             * Data in and data out on same channel-B XAUI data path. 
             * no need to re-execute XAUI init script, it has a possibility to casue ethernet port link down */
            mii_value = cvmx_mdio_45_write(bus_id, phy_id, dev_id, reg_addr, val);
            if (mii_value < 0) {
                cterr('f', 0, "Write error from device %u(0x%x)", dev_id, phy_id);
                fflush(stdout);
                return (FAILED);
            }

            msleep(10);
            break;
        case XAUIB_TO_XAUIA: /* Cavium <-> TLK10232 CHB <-> TLK10232 CHA <-> BP */
            /* Switch CHB to TLK10232 external XAUI data path 
             * Woodlawn Cavium <-XAUI-> TLK10232-CHB <-> TLK10232-CHA <-XAUI-> Platform GE-Switch
             * no need to re-execute XAUI init script, it has a possibility to casue ethernet port link down */

            /* Set up CHA data path. 0x1E.0x001A. val 0x4c20->0xac20 */            
            phy_id = TLK_10232_PHY_ADDR_CHANNEL_A;
            dev_id = TLK_10232_CHANNEL_CTRL_1_DEV;
            reg_addr = TLK_10232_DSR_CONTROL_2_REG;
            mii_value = cvmx_mdio_45_read(bus_id, phy_id, dev_id, reg_addr);
            val = (mii_value & (~TLK_10232_DSR_CONTROL_2_MASK_VAL_2)) | 
                        (~TLK_10232_DSR_CONTROL_3_MASK_VAL_3_CHA); 
            mii_value = cvmx_mdio_45_write(bus_id, phy_id, dev_id, reg_addr, val);
            if (mii_value < 0) {
                cterr('f', 0, "Write error from device %u(0x%x)", dev_id, phy_id);
                return (FAILED);
            }

            /* Set up CHB data path. 0x1E.0x001A. val 0x4c20->0xac20 */            
            phy_id = TLK_10232_PHY_ADDR_CHANNEL_B;
            mii_value = cvmx_mdio_45_read(bus_id, phy_id, dev_id, reg_addr);
            val = (mii_value & (~TLK_10232_DSR_CONTROL_2_MASK_VAL_2)) | 
                        (~TLK_10232_DSR_CONTROL_3_MASK_VAL_3_CHB); 
            
            mii_value = cvmx_mdio_45_write(bus_id, phy_id, dev_id, reg_addr, val);
            if (mii_value < 0) {
                cterr('f', 0, "Write error from device %u(0x%x)", dev_id, phy_id);
                return (FAILED);
            }
            msleep(10);
            break;
        default :
            cterr('f', 0, "TLK10232 doesn't support this mode");
            return (FAILED);
    }
    return (PASSED);
}

/******************************************************************************
 *
 * Function: tlk10232_xaui_to_xaui_configuration
 *
 * Description: Set up TLK10232 registers for xaui path loopback test
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int tlk10232_xaui_to_xaui_configuration (void)
{
    int bus_id, phy_id, dev_id, reg_addr, val, mii_value;
    bus_id = TLK_10232_SMI2_ADDR;
    
    /* Do global reset first to clear previous settings */
    if (tlk10232_global_reset() == FAILED) {
        cterr('f', 0, "tlk10232 global reset fail beofre execute XAUI init script");
        return (FAILED);
    }

    /* Disable auto-negotiation. 0x07.0x0000.bit 12, val 0x3000->0x2000 */
    /* Disable CHA auto-negotiation */
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_A;
    dev_id = TLK_10232_AN_CTRL;
    reg_addr = TLK_10232_AN_CTRL_REG;

    mii_value = cvmx_mdio_45_read(bus_id, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr, dev_id, phy_id);
        return (FAILED);
    }
    val = mii_value & (~TLK_10232_AN_DISABLE);
    
    mii_value = cvmx_mdio_45_write(bus_id, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr, dev_id, phy_id);
        return (FAILED);
    }
    /* Disable CHB auto-negotiation */
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_B;
    mii_value = cvmx_mdio_45_read(bus_id, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr, dev_id, phy_id);
        return (FAILED);
    }
    val = mii_value & (~TLK_10232_AN_DISABLE);
    
    mii_value = cvmx_mdio_45_write(bus_id, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr, dev_id, phy_id);
        return (FAILED);
    }
    
    /* Disable link training. 0x01.0x0096 val 0x0000 */
    /* Disable CHA link training */
    dev_id = TLK_10232_LT_TRAIN_CTRL;
    reg_addr = TLK_10232_LT_TRAIN_CTRL_REG;
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_A;

    mii_value = cvmx_mdio_45_read(bus_id, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr, dev_id, phy_id);
        return (FAILED);
    }
    val = mii_value & (~TLK_10232_LINK_TRAIN_DISABLE);
    
    mii_value = cvmx_mdio_45_write(bus_id, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr, dev_id, phy_id);
        return (FAILED);
    }

    /* Disable CHB link training */
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_B;
    mii_value = cvmx_mdio_45_read(bus_id, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr, dev_id, phy_id);
        return (FAILED);
    }
    val = mii_value & (~TLK_10232_LINK_TRAIN_DISABLE);
    
    mii_value = cvmx_mdio_45_write(bus_id, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr, dev_id, phy_id);
        return (FAILED);
    }
    
    /* Disable auto HS status check. 0x1E.0x8021. val 0x000f->0x0003f */
    /* Disable CHA auto HS status check */
    dev_id = TLK_10232_TI_RESERVED_CTRL;
    reg_addr = TLK_10232_TI_RESERVED_CTRL_REG;
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_A;

    mii_value = cvmx_mdio_45_read(bus_id, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr, dev_id, phy_id);
        return (FAILED);
    }
    val = mii_value | TLK_10232_HS_AUTO_STATUS_CHECK_DISABLE;
    
    mii_value = cvmx_mdio_45_write(bus_id, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr, dev_id, phy_id);
        return (FAILED);
    }

    /* Disable CHB auto HS status check */
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_B;
    mii_value = cvmx_mdio_45_read(bus_id, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr, dev_id, phy_id);
        return (FAILED);
    }
    val = mii_value | TLK_10232_HS_AUTO_STATUS_CHECK_DISABLE;
    
    mii_value = cvmx_mdio_45_write(bus_id, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr, dev_id, phy_id);
        return (FAILED);
    }    
    return (PASSED);
}

/******************************************************************************
 *
 * Function: set_tlk10232_lpbk_bit
 *
 * Description: Set up TLK10232 deep remote lpbk bit for GE BP loopback test
 * Inputs      : Set or clear lpbk bit 
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int set_tlk10232_lpbk_bit (int lpbk_setting)
{
    int bus_id, phy_id, dev_id, reg_addr, val, mii_value;
    bus_id = TLK_10232_SMI2_ADDR;
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_B;
    
    /* Set deep remote lpbk bit */
    dev_id = TLK_10232_LPBK_TP_CTRL;
    reg_addr = TLK_10232_LPBK_TP_REG;

    mii_value = cvmx_mdio_45_read(bus_id, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr, dev_id, phy_id);
        return (FAILED);
    }

    if (lpbk_setting == 1) {
        /* Set up lpbk bit */
        val = mii_value | TLK_10232_LPBK_TP_VAL;
    } else {
        /* Clear lpbk bit */
        val = mii_value & (~ TLK_10232_LPBK_TP_VAL);
    }

    mii_value = cvmx_mdio_45_write(bus_id, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr, dev_id, phy_id);
        return (FAILED);
    }
    
    return (PASSED);
}

int tlk10232_mode_select (void)
{
    int rv;
    char val;
    printf("TLK10232 mode selection\n");
    val = getdec_answer("1-XAUI <-> 10GKR, 2-XAUIB <-> XAUIB, 3-XAUIB <-> XAUIA", 1, 1, 3); 
    rv = config_tlk_10232_mode(val);
    if (rv != PASSED) {
        cterr('f', 0, "TLK10232 configuration failed");
        return (FAILED);
    }
    
    return (PASSED);
}

int tlk10232_global_reset (void)
{
    int bus_id, phy_id, dev_id, reg_addr, val, mii_value;
    bus_id = TLK_10232_SMI2_ADDR;
    
    /* Do TLK10232 global reset(just need to set up one channel)
    0x1E.0x0000.bit 15, val 0x0610->0x8610 */
    dev_id = TLK_10232_CHANNEL_CTRL_1_DEV;
    reg_addr = TLK_10232_GLOBAL_CTRL_REG;
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_B;
    
    mii_value = cvmx_mdio_45_read(bus_id, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr, dev_id, phy_id);
        return (FAILED);
    }
    val = mii_value | TLK_10232_GLOBAL_RESET;
    
    mii_value = cvmx_mdio_45_write(bus_id, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr, dev_id, phy_id);
        return (FAILED);
    }

    msleep(100);
    return (PASSED);
}

int tlk10232_path_reset (void)
{
    int bus_id, phy_id, dev_id, reg_addr, val, mii_value;
    bus_id = TLK_10232_SMI2_ADDR;
    
    /* Clear data path. 0x1E.0x000e. val 0x0000->0x0008 */
    /* CHA data path reset */
    dev_id = TLK_10232_RESET_CTRL;
    reg_addr = TLK_10232_RESET_CTRL_REG;
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_A;

    mii_value = cvmx_mdio_45_read(bus_id, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr, dev_id, phy_id);
        return (FAILED);
    }
    val = mii_value | TLK_10232_PATH_RESET;
    
    mii_value = cvmx_mdio_45_write(bus_id, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr, dev_id, phy_id);
        return (FAILED);
    }
    
    /* CHB data path reset */
    phy_id = TLK_10232_PHY_ADDR_CHANNEL_B;
    mii_value = cvmx_mdio_45_read(bus_id, phy_id, dev_id, reg_addr);
    if (mii_value < 0) {
        cterr('f', 0, "Read reg %x error from device %x, phy %x", reg_addr, dev_id, phy_id);
        return (FAILED);
    }
    val = mii_value | TLK_10232_PATH_RESET;
    
    mii_value = cvmx_mdio_45_write(bus_id, phy_id, dev_id, reg_addr, val);
    if (mii_value < 0) {
        cterr('f', 0, "Write reg %x error from device %x, phy %x", reg_addr, dev_id, phy_id);
        return (FAILED);
    }

    /* Wait for 1000ms after path reset */
    msleep(PATH_RESET_TIME);

    return (PASSED);
}
/*******************************************************************
 *
 * Function    : read_tlk_10232_reg
 * Description : SMI read funtion for tlk_10232 reg test.
 * Input       : addr  - register offset.
 *               size  - read data size
 *               buf   - read buffer
 *               param - parameter
 *               
 * Output: PASSED/FAILED
 *
 *******************************************************************
 */
int read_tlk_10232_reg (ulong addr, int size, ulong *buff, void *addr_info)
{
    ten_g_phy_t *phy_addr_info = (ten_g_phy_t *)addr_info;
    uint mii_value;
    int phy_id, bus_id, dev_id;
    uint8_t *data_buf = (uint8_t *)buff;

    *buff = 0;

    bus_id = (phy_addr_info->port_id) >> 4;
    phy_id = (phy_addr_info->port_id) & 0xF;
    dev_id = phy_addr_info->device_id;

    mii_value = cvmx_mdio_45_read(bus_id, phy_id, dev_id, addr);

    if (mii_value < 0) {
        cterr('f', 0, "Read error from device %u(0x%x)", dev_id, phy_id);
        return (FAILED);
    } else {
        #if DEBUG
        printf("%s() bus_id is %d phy_id is %d dev_id is %d mii_value is %x\n",
                __FUNCTION__, bus_id, phy_id, dev_id, mii_value);
        #endif
        data_buf[2] = mii_value >> 8 & 0xFF;
        data_buf[3] = mii_value & 0xFF;
        return (PASSED);
    }
}

/*******************************************************************
 *
 * Function    : write_tlk_10232_reg
 * Description : SMI write funtion for tlk_10232 reg test.
 * Input       : addr  - register offset.
 *               size  - read data size
 *               value - data to be written.
 *               param - parameter
 *               
 * Output: PASSED/FAILED
 *
 *******************************************************************
 */
int write_tlk_10232_reg (ulong addr, int size, ulong val, void *addr_info)
{
    ten_g_phy_t *phy_addr_info = (ten_g_phy_t *)addr_info;
    int status;
    int phy_id, bus_id, dev_id;

    bus_id = (phy_addr_info->port_id) >> 4;
    phy_id = (phy_addr_info->port_id) & 0xF;
    dev_id = phy_addr_info->device_id;

    status = cvmx_mdio_45_write(bus_id, phy_id, dev_id, (int)addr, val);

    if (status < 0) {
        cterr('f', 0, "Write error to device %d(0x%x)", dev_id, phy_id);
        return (FAILED);
    } else {
        #if DEBUG
        printf("%s() bus_id is %d phy_id is %d dev_id is %d val is %x\n",
                __FUNCTION__, bus_id, phy_id, dev_id, val);
        #endif
        return(PASSED);
    }

}

int tlk10232_kr_configuration (void)
{
    /* Final script - enable FEC, LT and AN */
    int bus_id, phy_id_cha, phy_id_chb;
    int data, cnt;
    
    bus_id = TLK_10232_SMI2_ADDR;
    phy_id_cha = TLK_10232_PHY_ADDR_CHANNEL_A;
    phy_id_chb = TLK_10232_PHY_ADDR_CHANNEL_B;

    cvmx_mdio_45_write(bus_id, phy_id_chb, 0x1e, 0x0, 0x8610);

    cvmx_mdio_45_write(bus_id, phy_id_cha, 0x1e, 0x1, 0x8b24);

    cvmx_mdio_45_write(bus_id, phy_id_cha, 0x1e, 0xd, 0x3f80);

    cvmx_mdio_45_write(bus_id, phy_id_chb, 0x1e, 0xd, 0x3f80);

    cvmx_mdio_45_write(bus_id, phy_id_cha, 0x1, 0x0, 0x800);

    cvmx_mdio_45_write(bus_id, phy_id_cha, 0x3, 0x0, 0x800);

    cvmx_mdio_45_write(bus_id, phy_id_chb, 0x7, 0x0, 0x2000);

    cvmx_mdio_45_write(bus_id, phy_id_chb, 0x1, 0x96, 0x0);

    /* Enable FEC */
    cvmx_mdio_45_write(bus_id, phy_id_chb, 0x1, 0xab, 0x1);

    /* Data path reset */
    cvmx_mdio_45_write(bus_id, phy_id_chb, 0x1e, 0xe, 0xe);

    cvmx_mdio_45_write(bus_id, phy_id_chb, 0x1e, 0x9000, 0x24d);

    cvmx_mdio_45_write(bus_id, phy_id_chb, 0x1e, 0x8101, 0x4);

    cvmx_mdio_45_write(bus_id, phy_id_chb, 0x1e, 0x8100, 0x4);

    cvmx_mdio_45_write(bus_id, phy_id_chb, 0x1e, 0x8100, 0x0);

    cvmx_mdio_45_write(bus_id, phy_id_chb, 0x1e, 0x9001, 0x201);

    cvmx_mdio_45_write(bus_id, phy_id_chb, 0x7, 0x0, 0x3000);

    cvmx_mdio_45_write(bus_id, phy_id_chb, 0x1, 0x96, 0x2);

    cvmx_mdio_45_write(bus_id, phy_id_chb, 0x1e, 0x9005, 0x1c00);

    msleep(1000);

    /* for greyhound switch */ 
    cvmx_mdio_45_write(bus_id, phy_id_chb, 0x1e, 0x8100, 0x1);
    cvmx_mdio_45_write(bus_id, phy_id_chb, 0x1e, 0x4, 0x5540);
    cvmx_mdio_45_write(bus_id, phy_id_chb, 0x1, 0x96, 0x3);

    cvmx_mdio_45_read(bus_id, phy_id_chb, 0x7, 0x0);
    for (cnt = 0; cnt < WAIT_AN_COMPLETE; cnt++) {
        cvmx_mdio_45_write(bus_id, phy_id_chb, 0x7, 0x0, 0x3200);
        msleep(10);
        /* If set, read is required to clear AN_RESTART bit */
        data = cvmx_mdio_45_read(bus_id, phy_id_chb, 0x7, 0x0);
        /* sleep 1 second, wait for AN complete */
        msleep(1000);
        data = cvmx_mdio_45_read(bus_id, phy_id_chb, 0x7, 0x1);
        
        tlk10232_path_reset();

        if (data & 0x20) {
            printf("KR-AN-link-complete\n");
            fflush(stdout);
            break;
        } else {
            tlk10232_path_reset();
            printf("KR-AN link not finished - %d, data = %x\n", cnt, data);
            fflush(stdout);
        }
    }

    return (PASSED);
}

/******************************************************************************
 *
 * Function: is_10gkr_capable
 *
 * Description: Platform is Greyhound or Helix switch   
 *
 * Inputs      : None
 * Outputs     : TRUE/FALSE 
 *
 *****************************************************************************/
int is_10gkr_capable (void)
{
    char fpga_reg_val; 

    /*                                                                                                   * According to FPGA reg 0x3 bit0 to run tlk10232 10g-kr or xaui script.                             * bit0 is BP_GE0_10GKR_CAPABLE bit                                                                  */
    fpga_reg_read(FPGA_10GKR_CAPABLE, &fpga_reg_val);

    if ((fpga_reg_val & 0x1) == GE0_10GKR_CAPABLE) {
        /* support 10g-kr */
        return (TRUE);
    } else {
        /* support xaui */
        /* Init TLK10232 for XAUI Backplane Loopback */
        return (FALSE);
    }
}

/******************************************************************************
 *
 * Function: run_tlk10232_script 
 *
 * Description: According to platform type to execute tlk10232 10GKR   
 *              XAUI init script
 * Inputs      : None
 * Outputs     : None 
 *
 *****************************************************************************/
void run_tlk10232_script (void)
{
    char fpga_reg_val; 

    /*                                                                                                   * According to FPGA reg 0x3 bit0 to run tlk10232 10g-kr or xaui script.                             * bit0 is BP_GE0_10GKR_CAPABLE bit                                                                  */
    fpga_reg_read(FPGA_10GKR_CAPABLE, &fpga_reg_val);

    if ((fpga_reg_val & 0x1) == GE0_10GKR_CAPABLE) {
        /* support 10g-kr */
        printf("----Switch to 10G-KR data path\n");
        fflush(stdout);
        config_tlk_10232_mode(XAUIB_TO_10GKR);
    } else {
        /* support xaui */
        /* Init TLK10232 for XAUI Backplane Loopback */
        printf("----Switch to XAUI data path\n");
        fflush(stdout);
        config_tlk_10232_mode(XAUIB_TO_XAUIA);
    }
}

/*-------------------------------------------------
 * $Log: diag_tlk10232_lib.c,v $
 * Revision 1.6  2017/09/27 01:56:30  leschen
 * CSCvd81389 - No need to execute TLK10232 init script.
 *
 * Revision 1.5  2015/03/31 07:33:31  leschen
 * Fix for KR.
 *
 * Revision 1.4  2014/11/12 06:29:14  leschen
 * Support Greyhound tlk10232 10gkr
 *
 * Revision 1.3  2013/12/12 09:15:27  leschen
 * CSCul71044:Fix TLK10232 internal loopback fail issue
 *
 * Revision 1.2  2013/10/08 08:48:29  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.4.2  2013/08/20 10:58:54  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.1  2013/04/24 10:37:19  tirawan
 * Initial check-in for woodlawn linux code
 *
 * Revision 1.4  2013/04/12 04:48:01  leslie
 * Fix and clean up code
 *
 * Revision 1.3  2013/04/09 11:02:17  leslie
 * Set TLK10232 deep remote lpbk bit
 *
 * Revision 1.2  2013/03/27 04:49:36  kuangik
 * Code cleanup after adding -Wall
 *
 * Revision 1.5  2013/03/07 12:39:48  leslie
 * Fix and add TLK10232 libs.
 *
 * Revision 1.4  2013/01/18 06:30:18  leslie
 * Fix and clean up code.
 *
 * Revision 1.3  2013/01/16 01:21:06  leslie
 * Add config TLK10232 function.
 *
 * Revision 1.1  2013/01/13 23:15:15  leslie
 * Initial check in TLK10232 code.
 *
 * $Endlog$
 *-------------------------------------------------
 */
