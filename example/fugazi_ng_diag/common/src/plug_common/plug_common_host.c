/* $Id: plug_common_host.c,v 1.4 2019/08/06 06:56:16 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/plug_common/plug_common_host.c,v $
 *------------------------------------------------------------------
 *
 * plug_common_host.c - PLUGGABLE Common Host Function
 *                   (Needs to be implemented by host side)
 *
 * Copyright (c) 2015 - 2019 by Cisco Systems, Inc.
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
#include "common.h"
#include "types.h"
#include "error.h"
#include "plug_common_host.h"

#define PLUG_WARNING_MSG(func)         printf("'%s' is not implemented !!!\n", func);



/*-------------------------------------------------------------------
 * Function : plug_common_host_i2c_ctrl 
 * Description: This function get i2c controller number by slot
 * INPUT: slot - plug slot number. Start from 1 
 * OUTPUT: base on slot 1/2 to return 0/1
 * -------------------------------------------------------------------
*/
int plug_common_host_i2c_ctrl (int slot)
__attribute__((weak, alias("__plug_common_host_i2c_ctrl")));
int __plug_common_host_i2c_ctrl (int slot)
{
    PLUG_WARNING_MSG(__func__)
    return (FAILED);
}

/*******************************************************************************
 * Function    : plug_common_host_usb_hub_reset
 * Description : Function to reset Pluggable USB HUB or not
 * Inputs      : ENABLE  reset USB HUB
 *               DISABLE un-reset USB HUB
 * Outputs     : None 
 *
 *******************************************************************************
 */
void plug_common_host_usb_hub_reset (int reset)
__attribute__((weak, alias("__plug_common_host_usb_hub_reset")));
void __plug_common_host_usb_hub_reset (int reset)
{
    PLUG_WARNING_MSG(__func__)
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
__attribute__((weak, alias("__plug_common_host_get_cookie_id")));
ushort __plug_common_host_get_cookie_id (int slot, int type, uchar * eeprom_data, 
                                      uint16_t * id, char *err)
{
    PLUG_WARNING_MSG(__func__)
    return (FAILED);
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
__attribute__((weak, alias("__plug_common_host_plug_fpga_reg_write")));
int __plug_common_host_plug_fpga_reg_write (uint offset, uint data_out)
{
    PLUG_WARNING_MSG(__func__)
    return (FAILED);
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
__attribute__((weak, alias("__plug_common_host_plug_fpga_reg_read")));
int __plug_common_host_plug_fpga_reg_read (uint offset, uint *data_in)
{
    PLUG_WARNING_MSG(__func__)
    return (FAILED);
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
__attribute__((weak, alias("__plug_common_host_diag_fpga_reg_bitops")));
int __plug_common_host_diag_fpga_reg_bitops (uint ops, uint offset, uint bit)
{
    PLUG_WARNING_MSG(__func__)
    return (FAILED);
}


/*******************************************************************************
 *
 * Function   :    plug_common_host_usb_3p0_mode_set
 * Description:    Setup USB mode to 3.0
 * Inputs     :    slot - plug slot number 
 * Outputs    :    NONE
 *
 *******************************************************************************
 */
int plug_common_host_usb_3p0_mode_set (int slot)
__attribute__((weak, alias("__plug_common_host_usb_3p0_mode_set")));
int __plug_common_host_usb_3p0_mode_set (int slot)
{
    PLUG_WARNING_MSG(__func__)
    return (FAILED);
}

/*******************************************************************************
 *
 * Function   :    plug_common_host_usb_2p0_mode_set
 * Description:    Setup USB mode to 2.0
 * Inputs     :    slot - plug slot number 
 * Outputs    :    NONE
 *
 *******************************************************************************
 */
int plug_common_host_usb_2p0_mode_set (int slot)
__attribute__((weak, alias("__plug_common_host_usb_2p0_mode_set")));
int __plug_common_host_usb_2p0_mode_set (int slot)
{
    PLUG_WARNING_MSG(__func__)
    return (FAILED);
}

/*******************************************************************************
 *  Function:    plug_common_host_get_max_plug_slots
 *  Description: return max slot number of plug slots
 *  Inputs:  NONE
 *  Output:  max number of plug slots
 ******************************************************************************
 */
int plug_common_host_get_max_plug_slots (void)
__attribute__((weak, alias("__plug_common_host_get_max_plug_slots")));
int __plug_common_host_get_max_plug_slots (uint slot_num)
{
    PLUG_WARNING_MSG(__func__)
    return (FAILED);
}

/*******************************************************************************
 * Function   : plug_common_host_i2c_rd
 * Description: This function performs I2C read operation 
 * Inputs     : i2c_ctrl - I2C controller
 *              i2c_addr - I2C Device Address (7-byte real address)
 *              offset - Offset
 *              *data - Data pointer
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
int plug_common_host_i2c_rd (uint8_t i2c_ctrl, uint8_t i2c_addr, 
                                    uint32 offset, char *data)
__attribute__((weak, alias("__plug_common_host_i2c_rd")));
int __plug_common_host_i2c_rd (uint8_t i2c_ctrl, uint8_t i2c_addr,
                                      uint32 offset, char *data)
{
    PLUG_WARNING_MSG(__func__)
    return (FAILED);
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
__attribute__((weak, alias("__plug_common_host_i2c_wr")));
int __plug_common_host_i2c_wr (uint8_t i2c_ctrl, uint8_t i2c_addr, 
                                      uint32 offset, char data)
{
    PLUG_WARNING_MSG(__func__)
    return (FAILED);
}


/*******************************************************************************
 * Function   : plug_common_host_i2c_rd_2bytes
 * Description: This function performs I2C read operation 
 * Inputs     : i2c_ctrl - I2C controller
 *              i2c_addr - I2C Device Address (7-byte real address)
 *              offset - Offset
 *              *data - Data pointer
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
int plug_common_host_i2c_rd_2bytes_2bytes (uint8_t i2c_ctrl, uint8_t i2c_addr, 
                                    uint32 offset, ushort *data)
__attribute__((weak, alias("__plug_common_host_i2c_rd_2bytes")));
int __plug_common_host_i2c_rd_2bytes (uint8_t i2c_ctrl, uint8_t i2c_addr,
                                      uint32 offset, ushort *data)
{
    PLUG_WARNING_MSG(__func__)
    return (FAILED);
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
__attribute__((weak, alias("__plug_common_host_i2c_wr_2bytes")));
int __plug_common_host_i2c_wr_2bytes (uint8_t i2c_ctrl, uint8_t i2c_addr, 
                                      uint32 offset, ushort data)
{
    PLUG_WARNING_MSG(__func__)
    return (FAILED);
}

/*******************************************************************************
 * Function    : plug_common_host_pcie_dev_disable
 * Description : Function to reset Pluggable USB HUB or not
 * Inputs      : ENABLE  reset USB HUB
 *               DISABLE un-reset USB HUB
 * Outputs     : None 
 *
 *******************************************************************************
 */
void plug_common_host_pcie_dev_disable (int slot)
__attribute__((weak, alias("__plug_common_host_pcie_dev_disable")));
void __plug_common_host_pcie_dev_disable (int slot)
{
    PLUG_WARNING_MSG(__func__)
}

/*-------------------------------------------------
$Log: plug_common_host.c,v $
Revision 1.4  2019/08/06 06:56:16  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.3  2018/11/23 09:02:31  hondwang
Re-instruct pluggable common code with CDETs CSCvn17216

Revision 1.2.62.1  2018/10/15 06:50:18  hondwang
pluggable common code re-instruct modify code

Revision 1.2  2018/01/20 04:53:28  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.1.4.2  2017/08/08 07:39:41  hondwang
add pluggable for star-branch-c9xx

Revision 1.1.2.1  2017/07/20 17:23:10  tirawan
Add Pluggable host implementation codes


*/

