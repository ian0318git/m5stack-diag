/* $Id: plug_common_host_impl.c,v 1.4 2018/11/23 08:49:52 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/plug_common_host_impl.c,v $
 *------------------------------------------------------------------
 *
 * plug_common_host.c - PLUGGABLE Common Host Function Implementation
 *
 * Copyright (c) 2015 - 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include "nvmonvars.h"
#include "common.h"
#include "types.h"
#include "error.h"
#include "i2c_api.h"
#include "menu.h"
#include "cookie_4.h"
#include "proto.h"
#include "plug_common_host_impl.h"
#include "plug_testcard_host_impl.h"
#include "plug_host_fpga_lib.h"
#include "plug_slot.h"

int plug_common_host_i2c_ctrl(int);
void plug_common_host_usb_hub_reset(int);
ushort plug_common_host_get_cookie_id(int, int, uchar *,uint16_t *, char *);
int plug_common_host_plug_fpga_reg_write(uint, uint);
int plug_common_host_plug_fpga_reg_read(uint, uint *);
int plug_common_host_diag_fpga_reg_bitops(uint, uint, uint);
int plug_common_host_usb_3p0_mode_set(int);
int plug_common_host_usb_2p0_mode_set(int);
int plug_common_host_get_max_plug_slots(void);
int plug_common_host_i2c_rd(uint8_t, uint8_t, uint32, char *);
int plug_common_host_i2c_wr(uint8_t, uint8_t, uint32, char);
int plug_common_host_i2c_rd_2bytes(uint8_t, uint8_t, uint32, ushort *);
int plug_common_host_i2c_wr_2bytes(uint8_t, uint8_t, uint32, ushort);


/*-------------------------------------------------------------------
 * Function : plug_common_host_i2c_ctrl 
 * Description: This function get i2c controller number by slot
 * INPUT: slot - plug slot number. Start from 1 
 * OUTPUT: base on slot 1/2 to return 0/1
 * -------------------------------------------------------------------
*/
int plug_common_host_i2c_ctrl (int slot)
{
    return (PLUG_I2C_CTRL(slot));
}

/*****************************************************************************n
 * Function    : plug_common_host_usb_hub_reset
 * Description : Function to reset Pluggable USB HUB or not
 * Inputs      : ENABLE  reset USB HUB
 *               DISABLE un-reset USB HUB
 * Outputs     : None 
 *
 *******************************************************************************
 */
void plug_common_host_usb_hub_reset (int reset)
{
    if (reset == ENABLE) {
        msleep(PLUG_PWR_OFF_DELAY);
        plug_common_host_diag_fpga_reg_bitops(FPGA_BIT_OPS_ON, FPGA_EXTER_DEV_RST_REG, 
                                              FPGA_USB_HUB_RESET_BIT);
    } else {
        plug_common_host_diag_fpga_reg_bitops(FPGA_BIT_OPS_OFF, FPGA_EXTER_DEV_RST_REG, 
                                              FPGA_USB_HUB_RESET_BIT);
        msleep(PLUG_PWR_OFF_DELAY);

    }
}

/**************************************************************************
 *
 * Name: plug_common_host_get_cookie_id 
 *
 * Description: read plug cookie id
 *
 * Inputs: slot, type, eeprom_data, err - dummy parameters
 *         id - control id
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
ushort plug_common_host_get_cookie_id (int slot, int type, uchar * eeprom_data,
              uint16_t * id, char *err)
{
    return (get_cookie_id(slot, type, eeprom_data, id, err));
}

/*******************************************************************************
 * Function    : plug_common_host_plug_fpga_reg_write
 * Description : Function to write Pluggable FPGA Register
 * Inputs      : offset
 *               data_out - Data to be written to Pluggable FPGA
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int plug_common_host_plug_fpga_reg_write (uint offset, uint data_out)
{
    return (plug_fpga_reg_write(offset, data_out));
}

/*******************************************************************************
 * Function    : plug_common_host_plug_fpga_reg_read
 * Description : Function to read Pluggable FPGA Register
 * Inputs      : offset
 *               *data_in - Pointer to buffer
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int plug_common_host_plug_fpga_reg_read (uint offset, uint *data_in)
{
    return (plug_fpga_reg_read(offset, data_in));
}

/*******************************************************************************
 * Function    : plug_common_host_diag_fpga_reg_bitops
 * Description : Function to turn on/off bit on plug FPGA Register
 * Inputs      : ops - ON or OFF
 *               offset 
 *               bit
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int plug_common_host_diag_fpga_reg_bitops (uint ops, uint offset, uint bit)
{
    return (diag_fpga_reg_bitops(ops, offset, bit));
}

/*******************************************************************************
 *
 * Function   :    plug_common_host_usb_3p0_mode_set
 * Description:    Setup plug USB mode to 3.0
 * Inputs     :    slot - plug slot number 
 * Outputs    :    NONE
 *
 *******************************************************************************
 */
int plug_common_host_usb_3p0_mode_set (int slot)
{
    uint reg;

    reg = FPGA_PLUG_OFFSET_BY_SLOT(FPGA_PLUG1_STSCTL_REG, slot);

    msleep(DELAY_USBCMD); 
    /* Power off USB */
    diag_fpga_reg_bitops(FPGA_BIT_OPS_OFF, reg, PLUG_PWR_EN_BIT);
    msleep(DELAY_USBHUBCMD);
    /* Reset USB HUB for cpu switch USB mode */
    plug_common_host_usb_hub_reset(ENABLE);
    
    /* Recover to USB Auto(3.0) mode */
    msleep(DELAY_SYSCMD);
    system(USB1_30_CUSTOM_REG1_CMD);
    msleep(DELAY_SYSCMD);
    system(USB1_30_CUSTOM_REG2_CMD);
    msleep(DELAY_SYSCMD);
    system(USB1_MISC_CTRL_1_REG_CMD);
    msleep(DELAY_SYSCMD);
    system(USB1_30_CTRL_REG_CMD);
    msleep(DELAY_SYSCMD);

    /* Un-Reset USB HUB for cpu switch USB mode */
    plug_common_host_usb_hub_reset(DISABLE);
    msleep(DELAY_USBHUBCMD);
    /* Power on USB */
    diag_fpga_reg_bitops(FPGA_BIT_OPS_ON, reg, PLUG_PWR_EN_BIT);
    
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   :    plug_common_host_usb_2p0_mode_set
 * Description:    Setup plug USB mode to 2.0
 * Inputs     :    slot - plug slot number
 * Outputs    :    NONE
 *
 *******************************************************************************
 */
int plug_common_host_usb_2p0_mode_set (int slot)
{
    uint reg;

    reg = FPGA_PLUG_OFFSET_BY_SLOT(FPGA_PLUG1_STSCTL_REG, slot);

    /* Change to USB 2.0 */
    msleep(DELAY_USBCMD);
    
    /* Power off USB */
    diag_fpga_reg_bitops(FPGA_BIT_OPS_OFF, reg, PLUG_PWR_EN_BIT);
    msleep(DELAY_USBHUBCMD);
    /* Reset USB HUB for cpu switch USB mode */
    plug_common_host_usb_hub_reset(ENABLE);

    /* Test USB 2.0 mode */
    msleep(DELAY_SYSCMD);
    system(USB1_20_CUSTOM_REG1_CMD);
    msleep(DELAY_SYSCMD);
    system(USB1_20_CUSTOM_REG2_CMD);
    msleep(DELAY_SYSCMD);
    system(USB1_MISC_CTRL_1_REG_CMD);
    msleep(DELAY_SYSCMD);
    system(USB1_20_CTRL_REG_CMD);
    msleep(DELAY_SYSCMD);

    /* Un-Reset USB HUB for cpu switch USB mode */
    plug_common_host_usb_hub_reset(DISABLE);
    msleep(DELAY_USBHUBCMD);
    /* Power on USB */
    diag_fpga_reg_bitops(FPGA_BIT_OPS_ON, reg, PLUG_PWR_EN_BIT);

    msleep(DELAY_USBCMD); 

    return (PASSED);
}

/*******************************************************************************
 *  Function:    plug_common_host_get_max_plug_slots
 *  Description: return max slot number of plug slots
 *  Inputs:  NONE
 *  Output:  max number of plug slots
 ******************************************************************************
 */
int plug_common_host_get_max_plug_slots (void)
{
    if(this_is_star_c1109_4p()) { 
       return (PLUG_SLOT_2);
    } else {
       return (PLUG_SLOT_1);
    }
}


/*******************************************************************************
 * Function   : plug_common_host_i2c_rd
 * Description: This function performs I2C read operation 
 * Inputs     : i2c_ctrl - I2C Controller
 *              i2c_addr - I2C Device Address (7-byte real address)
 *              offset - Offset
 *              *data - Data pointer
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
int plug_common_host_i2c_rd (uint8_t i2c_ctrl, uint8_t i2c_addr, 
                                    uint32 offset, char *data)
{
    n2g_i2c_if_t i2c_if;

    /* Setup I2C API parameter structure */
    i2c_if.offset = offset;
    i2c_if.sub_addr_len = 1;
    i2c_if.i2c_bus_type = PLUG_FPGA; 
    
    i2c_if.i2c_dev = i2c_addr;
    i2c_if.i2c_speed = N2G_I2C_100KHZ;
    i2c_if.size = sizeof(char);
    i2c_if.buf = (char *)data;
    i2c_if.i2c_ctrl = i2c_ctrl;

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s: I2c read i2c_ctrl= %08X, i2c_addr=%08X, offset=%08X, data=%08X.\n",
                __FUNCTION__, i2c_ctrl, i2c_addr, offset, *data);
    } 
     
    if (n2g_i2c_open(&i2c_if) != PASSED) {
        printf("%s: n2g_i2c_open failed\n", __func__);
        return (FAILED);
    }

    if (n2g_i2c_read(&i2c_if) != PASSED) {
        printf("%s: n2g_i2c_read failed\n", __func__);
        return (FAILED);
    }

    if (n2g_i2c_close(&i2c_if) != PASSED) {
        printf("%s: n2g_i2c_close failed\n", __func__);
        return (FAILED);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s: I2c read i2c_reply data=%08X.\n",__FUNCTION__, *data);
    } 

    return (PASSED);
}

/*******************************************************************************
 * Function   : plug_common_host_i2c_wr
 * Description: This function performs I2C write operation
 * Inputs     : i2c_ctrl - I2C Controller
 *              i2c_addr - I2C Device Address (7-byte real address)
 *              offset - Offset
 *              data - Data
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
int plug_common_host_i2c_wr (uint8_t i2c_ctrl, uint8_t i2c_addr, 
                                    uint32 offset, char data)
{
    n2g_i2c_if_t i2c_if;

    /* Setup I2C API parameter structure */
    i2c_if.offset = offset;
    i2c_if.sub_addr_len = 1;
    i2c_if.i2c_bus_type = PLUG_FPGA;     

    i2c_if.i2c_dev = i2c_addr;
    i2c_if.i2c_speed = N2G_I2C_100KHZ;
    i2c_if.size = sizeof(char);
    i2c_if.buf = (char *)&data;
    i2c_if.i2c_ctrl = i2c_ctrl;

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s: I2c write i2c_ctrl= %08X, i2c_addr=%08X, offset=%08X, data=%08X.\n",
                __FUNCTION__, i2c_ctrl, i2c_addr, offset, data);
    }

    if (n2g_i2c_open(&i2c_if) != PASSED) {
        printf("%s: n2g_i2c_open failed\n", __func__);
        return (FAILED);
    }

    if (n2g_i2c_write(&i2c_if) != PASSED) {
        printf("%s: n2g_i2c_read failed\n", __func__);
        return (FAILED);
    }

    if (n2g_i2c_close(&i2c_if) != PASSED) {
        printf("%s: n2g_i2c_close failed\n", __func__);
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 * Function   : plug_common_host_i2c_rd_2bytes
 * Description: This function performs I2C read operation 
 * Inputs     : i2c_ctrl - I2C Controller
 *              i2c_addr - I2C Device Address (7-byte real address)
 *              offset - Offset
 *              *data - Data pointer
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
int plug_common_host_i2c_rd_2bytes (uint8_t i2c_ctrl, uint8_t i2c_addr, 
                                    uint32 offset, ushort *data)
{
    n2g_i2c_if_t i2c_if;

    /* Setup I2C API parameter structure */
    i2c_if.offset = offset;
    i2c_if.sub_addr_len = 1;
    i2c_if.i2c_bus_type = PLUG_FPGA; 
    i2c_if.mux = 0;
    
    i2c_if.i2c_dev = i2c_addr;
    i2c_if.i2c_speed = N2G_I2C_100KHZ;
    i2c_if.size = sizeof(ushort);
    i2c_if.buf = (char *)data;
    i2c_if.i2c_ctrl = i2c_ctrl;

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s: I2c read i2c_ctrl= %08X, i2c_addr=%08X, offset=%08X, data=%08X.\n",
                __FUNCTION__, i2c_ctrl, i2c_addr, offset, *data);
    } 
     
    if (n2g_i2c_open(&i2c_if) != PASSED) {
        printf("%s: n2g_i2c_open failed\n", __func__);
        return (FAILED);
    }

    if (n2g_i2c_read(&i2c_if) != PASSED) {
        printf("%s: n2g_i2c_read failed\n", __func__);
        return (FAILED);
    }

    if (n2g_i2c_close(&i2c_if) != PASSED) {
        printf("%s: n2g_i2c_close failed\n", __func__);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 * Function   : plug_common_host_i2c_wr_2bytes
 * Description: This function performs I2C write operation
 * Inputs     : i2c_ctrl - I2C Controller
 *              i2c_addr - I2C Device Address (7-byte real address)
 *              offset - Offset
 *              data - Data
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
int plug_common_host_i2c_wr_2bytes (uint8_t i2c_ctrl, uint8_t i2c_addr, 
                                    uint32 offset, ushort data)
{
    n2g_i2c_if_t i2c_if;

    /* Setup I2C API parameter structure */
    i2c_if.offset = offset;
    i2c_if.sub_addr_len = 1;
    i2c_if.i2c_bus_type = PLUG_FPGA;     
    i2c_if.mux = 0;

    i2c_if.i2c_dev = i2c_addr;
    i2c_if.i2c_speed = N2G_I2C_100KHZ;
    i2c_if.size = sizeof(ushort);
    i2c_if.buf = (char *)&data;
    i2c_if.i2c_ctrl = i2c_ctrl;

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s: I2c write i2c_ctrl= %08X, i2c_addr=%08X, offset=%08X, data=%08X.\n",
                __FUNCTION__, i2c_ctrl, i2c_addr, offset, data);
    }

    if (n2g_i2c_open(&i2c_if) != PASSED) {
        printf("%s: n2g_i2c_open failed\n", __func__);
        return (FAILED);
    }

    if (n2g_i2c_write(&i2c_if) != PASSED) {
        printf("%s: n2g_i2c_read failed\n", __func__);
        return (FAILED);
    }

    if (n2g_i2c_close(&i2c_if) != PASSED) {
        printf("%s: n2g_i2c_close failed\n", __func__);
        return (FAILED);
    }

    return (PASSED);
}

    

/*-------------------------------------------------
$Log: plug_common_host_impl.c,v $
Revision 1.4  2018/11/23 08:49:52  hondwang
Re-instruct pluggable common code with CDETs CSCvn17216

Revision 1.3.46.1  2018/10/15 06:53:07  hondwang
pluggable common code re-instruct modify code

Revision 1.3  2018/03/27 12:46:38  hondwang
Code modify for Star_C1101_4PLTEP_4PLTEPWX and Pluggable LTE EM7455, WP7601, WP7603 ER

Revision 1.2.2.1  2018/03/09 05:55:36  shjung
Supported WP7608/7609

Revision 1.2  2018/02/09 09:56:56  hondwang
Merge Star branch star-branch-c9xx to main trunk

Revision 1.1.6.2  2018/01/20 06:11:17  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.1.4.4  2017/12/12 13:02:57  shjung
Supported LTE-WP7607 module

Revision 1.1.4.3  2017/11/23 03:20:44  hondwang
Show plug module name with system info function

Revision 1.1.4.2  2017/08/15 14:18:39  hondwang
star branch c9xx initial check in

Revision 1.1.2.1  2017/07/20 17:22:15  tirawan
Add Pluggable Host implementation codes


*/

