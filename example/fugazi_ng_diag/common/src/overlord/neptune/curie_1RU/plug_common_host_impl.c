/* $Id: plug_common_host_impl.c,v 1.3 2019/12/18 09:18:37 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_1RU/plug_common_host_impl.c,v $
 *------------------------------------------------------------------
 *
 * plug_common_host.c - PLUGGABLE Common Host Function Implementation
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
#include "platform_tam_cookie.h"
#include "platform_i2c.h"
#include "mb_tests.h"

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
    return (PLUG_I2C_CTRL);
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
    return (dash_fpga_reg_write(offset, data_out));
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
    return (dash_fpga_reg_read(offset, data_in));
}

/*******************************************************************************
 * Function    : diag_fpga_reg_bitops
 * Description : Function to turn on/off bit on FPGA Register
 * Inputs      : ops - ON or OFF
 *               offset 
 *               bit
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int diag_fpga_reg_bitops (uint ops, uint offset, uint bit)
{
    uint data;
    
    if (dash_fpga_reg_read(offset, &data) != PASSED) {
        return (FAILED);
    }
    
    switch (ops) {
    case FPGA_BIT_OPS_ON:
        data |= (0x1 << bit);
        break;
    case FPGA_BIT_OPS_OFF:
        data &= ~(0x1 << bit);
        break;
    default:
        printf("Not recognized bit ops (%d)\n", ops);
        return (FAILED);
    }

    return (dash_fpga_reg_write(offset, data));
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
    /* Enable super speed and route USB port from EHCI controller to XHCI controller */
    system(ENABLE_USB3_SS);
    msleep(1000);
    system(ROUTE_USB2_TO_XHCI);
    msleep(1000);
    system(UDEVTRIGGER);
    msleep(1000);

    /* Unbind and bind XHCI controller to XHCI driver */
    system(UNBIND_EHCI_CONTROLLER);
    msleep(500);
    system(BIND_EHCI_CONTROLLER);
    msleep(500);

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
    /* Disable USB 3.0 super speed */
    system(DISABLE_USB3_SS);
    msleep(1000);

    /* Unbind and bind XHCI controller to XHCI driver */
    system(UNBIND_XHCI_CONTROLLER);
    msleep(500);
    system(BIND_XHCI_CONTROLLER);
    msleep(500);

    /* Route USB ports from XHCI controller to EHCI controller */
    system(ROUTE_USB2_TO_EHCI);
    msleep(1000);
    system(UDEVTRIGGER);
    msleep(1000);

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
       return (PLUG_SLOT_1);
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
    
/*-------------------------------------------------------------------
 * Function : plug_common_host_pcie_dev_disable 
 * Description: This function disable host PCIe root port
 * INPUT: slot - plug slot number. Start from 1 
 * OUTPUT: None
 * -------------------------------------------------------------------
*/
void plug_common_host_pcie_dev_disable (int slot)
{
#define PLUGGABLE_PCIE_ROOT "/sys/bus/pci/devices/0000:00:1c.3/remove"
    if (access(PLUGGABLE_PCIE_ROOT, F_OK ) != -1 ) {
        // file exists
        system("echo 1 > /sys/bus/pci/devices/0000\\:00\\:1c.3/remove");
    }
}

/*-------------------------------------------------------------------
 * Function : plug_pci_dev_remove
 * Description: This function disable plug PCIe device
 * INPUT: slot - plug slot number. Start from 1 
 * OUTPUT: None
 * -------------------------------------------------------------------
*/
void plug_pci_dev_remove (int slot)
{
    /* if file exists, remove it */
    if (access(PLUGGABLE_PCIE_PATH, F_OK ) != -1 ) {
        /* "echo 1 > /sys/bus/pci/devices/0000:40:00.0/remove" */
        system(PLUGGABLE_PCIE_REMOVE); 
    } 
}


/*-------------------------------------------------
$Log: plug_common_host_impl.c,v $
Revision 1.3  2019/12/18 09:18:37  alpeng
1. support quack cookie rd/wr; 2. fixed new rommon break nightwatch issue; 3. a workaround for new pim testcard crashed system issue; 4. bump to v2.0.1 for curie

Revision 1.2  2019/08/06 06:56:14  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.1.2.7  2019/02/11 07:33:37  meho
Support new PIM test-card (PCIe)

Revision 1.1.2.6  2018/10/30 07:25:07  meho
Removed disable super speed in USB3.0 controller

Revision 1.1.2.5  2018/10/18 08:24:51  meho
Added USB3.0 480M speed test.

Revision 1.1.2.4  2018/10/17 08:15:36  meho
Corrected switch USB3.0/2.0 steps

Revision 1.1.2.3  2018/10/16 09:05:39  meho
Pluggable re-structured



*/

