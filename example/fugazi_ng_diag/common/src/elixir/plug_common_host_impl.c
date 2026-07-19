/* $Id: plug_common_host_impl.c,v 1.2 2021/09/24 01:21:08 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/elixir/plug_common_host_impl.c,v $
 *------------------------------------------------------------------
 * 
 * plug_common_host_impl.c
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
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
#include "plug_host_fpga_lib.h"
#include "plug_slot.h"
#include "diag_cpu_lib.h"
#include "plug_testcard_test.h"

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

    /* Remove PCIE device first otherwise system will hang */
    system(REMOVE_PIM_TESTCARD_PCIE_DEVICE);

    /* reset the module */
    diag_fpga_reg_bitops(FPGA_BIT_OPS_ON, reg, PLUG_RESET_BIT);

    /* Power off USB */
    diag_fpga_reg_bitops(FPGA_BIT_OPS_OFF, reg, PLUG_PWR_EN_BIT);
    msleep(DELAY_USBHUBCMD);

    /* Marvell A7K CPU implement workaround on pcie hot plug (CSCvw61650) */
    if (plug_testcard_pcie_post_pwr_down() == FAILED) {
        return (FAILED); 
    }

    /* Recover to USB Auto(3.0) mode */
    msleep(DELAY_USBCMD);
    system(USB_POWER_OFF_CMD);
    msleep(DELAY_SYSCMD);
    system(USB1_30_CUSTOM_REG1_CMD);
    msleep(DELAY_SYSCMD);
    system(USB1_30_CUSTOM_REG2_CMD);
    msleep(DELAY_SYSCMD);
    system(USB1_MISC_CTRL_1_REG_CMD);
    msleep(DELAY_SYSCMD);
    system(USB1_30_CTRL_REG_CMD);
    msleep(DELAY_SYSCMD);
    system(USB_POWER_ON_CMD);
    msleep(DELAY_USBCMD); 

    /* Power on USB */
    diag_fpga_reg_bitops(FPGA_BIT_OPS_ON, reg, PLUG_PWR_EN_BIT);
    msleep(DELAY_USBHUBCMD);

    /* unreset the module */
    diag_fpga_reg_bitops(FPGA_BIT_OPS_OFF, reg, PLUG_RESET_BIT);
    msleep(DELAY_USBHUBCMD);

    /* Marvell A7K CPU implement workaround on pcie hot plug (CSCvw61650) */
    if (plug_testcard_pcie_post_pwr_up() == FAILED) {
        return (FAILED); 
    }

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
    /* Remove PCIE device first otherwise system will hang */
    system(REMOVE_PIM_TESTCARD_PCIE_DEVICE);

    /* reset the module */
    diag_fpga_reg_bitops(FPGA_BIT_OPS_ON, reg, PLUG_RESET_BIT);

    /* Power off USB */
    diag_fpga_reg_bitops(FPGA_BIT_OPS_OFF, reg, PLUG_PWR_EN_BIT);
    msleep(DELAY_USBHUBCMD);

    /* Marvell A7K CPU implement workaround on pcie hot plug (CSCvw61650) */
    if (plug_testcard_pcie_post_pwr_down() == FAILED) {
        return (FAILED); 
    }

    /* Test USB 2.0 mode */
    msleep(DELAY_USBCMD);
    system(USB_POWER_OFF_CMD);
    msleep(DELAY_SYSCMD);
    system(USB1_20_CUSTOM_REG1_CMD);
    msleep(DELAY_SYSCMD);
    system(USB1_20_CUSTOM_REG2_CMD);
    msleep(DELAY_SYSCMD);
    system(USB1_MISC_CTRL_1_REG_CMD);
    msleep(DELAY_SYSCMD);
    system(USB1_20_CTRL_REG_CMD);
    msleep(DELAY_SYSCMD);
    system(USB_POWER_ON_CMD);
    msleep(DELAY_USBCMD); 

    /* Power on USB */
    diag_fpga_reg_bitops(FPGA_BIT_OPS_ON, reg, PLUG_PWR_EN_BIT);
    msleep(DELAY_USBHUBCMD);

    /* unreset the module */
    diag_fpga_reg_bitops(FPGA_BIT_OPS_OFF, reg, PLUG_RESET_BIT);
    msleep(DELAY_USBHUBCMD);

    /* Marvell A7K CPU implement workaround on pcie hot plug (CSCvw61650) */
    if (plug_testcard_pcie_post_pwr_up() == FAILED) {
        return (FAILED); 
    }
 
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

    return (PASSED);
}

/*******************************************************************************
 * Function   : plug_testcard_pcie_post_pwr_up
 * Description: The function to implement workarund after PCIE device power up
 * Inputs     : none
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
int plug_testcard_pcie_post_pwr_up (void) 
{
    int global_status_timeout = PCIE_TIMEOUT_COUNT;
    uint reg_offset = 0, reg_val = 0;
    
    /* Marvell A7K CPU implement workaround on pcie hot plug (CSCvw61650) */
    /* Polling link control link status bit4=1 and bit5=0 */
    reg_offset = PCIE_LINK_CONTROL_LINK_STATUS; 
    if (plat_mem_read32(reg_offset, &reg_val) != PASSED) {
        printf("Failed to read CPU register 0x%08X (link control link status).\n", reg_offset);
        return (FAILED);
    }
    
    /* Check register link control link status
    * bit4(PCIE_CAP_LINK_DISABLE) value equal 1 
    * and bit5(PCIE_CAP_RETRAIN_LINK) value equal 0 */
    if (((reg_val & PCIE_CAP_LINK_DISABLE) == PCIE_CAP_LINK_DISABLE) && 
        ((reg_val & PCIE_CAP_RETRAIN_LINK) == 0)) {
        while (global_status_timeout > 0) {
    
            /* Write bit4 value to 0 and bit5 value to 1 */
            reg_val &= ~(PCIE_CAP_LINK_DISABLE | PCIE_CAP_RETRAIN_LINK);
            reg_val |= PCIE_CAP_RETRAIN_LINK;
            reg_offset = PCIE_LINK_CONTROL_LINK_STATUS;
            if (plat_mem_write32(reg_offset, reg_val) != PASSED) {
                printf("Failed to write CPU register 0x%08X (link control link status).\n"
                       , reg_offset);
                return (FAILED);
            }
    
            reg_offset = PCIE_GLOBAL_STATUS;
            if (plat_mem_read32(reg_offset, &reg_val) != PASSED) {
                printf("Failed to read CPU register 0x%08X (Global Status Register).\n", reg_offset);
                return (FAILED);
            }
                    
            /* Check bit1 RDLH Link up and bit9 PHYLinkUp */
            if (((reg_val & PHYLINKUP) == PHYLINKUP) && 
                ((reg_val & RDLH_LINKUP) == RDLH_LINKUP)) {
                break; //break global status while loop
            }
    
            global_status_timeout--;
            msleep(10);
        }
    
        if (global_status_timeout == 0) {
            printf("%s:%d:Fail to polling PCIE global status reg bit 1 and bit 9\n",
                   __FUNCTION__, __LINE__);
            return (FAILED);
        }
    }

    system(PCIE_RESCAN);
    return (PASSED);
}

/*******************************************************************************
 * Function   : plug_testcard_pcie_post_pwr_down
 * Description: The function to implement process after pcie device power down
 * Inputs     : none
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
int plug_testcard_pcie_post_pwr_down (void) 
{
    uint reg_offset = 0, reg_val = 0;
    int link_control_link_status_timeout = PCIE_TIMEOUT_COUNT;

    /* Marvell A7K CPU implement workaround on pcie hot plug (CSCvw61650) */
    /* Polling link control link status bit4=0 and bit5=0 */ 
    while (link_control_link_status_timeout > 0) {
        reg_offset = PCIE_LINK_CONTROL_LINK_STATUS; 
        if (plat_mem_read32(reg_offset, &reg_val) != PASSED) {
            printf("Failed to read CPU register 0x%08X (link control link status).\n", reg_offset);
            return (FAILED);
        }

        /* Check register link control link status
         * bit4(PCIE_CAP_LINK_DISABLE) value equal 0 
         * and bit5(PCIE_CAP_RETRAIN_LINK) value equal 0 */
        if (((reg_val & PCIE_CAP_LINK_DISABLE) == 0) && 
            ((reg_val & PCIE_CAP_RETRAIN_LINK) == 0)) {
            /* Write link ontrol link status 
             * bit4 value to 1 and bit5 value to 0 */
            reg_val &= ~(PCIE_CAP_LINK_DISABLE | PCIE_CAP_RETRAIN_LINK);
            reg_val |= PCIE_CAP_LINK_DISABLE;
            reg_offset = PCIE_LINK_CONTROL_LINK_STATUS;
            if (plat_mem_write32(reg_offset, reg_val) != PASSED) {
                printf("Failed to write CPU register 0x%08X (link control link status).\n"
                       , reg_offset);
                return (FAILED);
            }


            /* Read PCIE Global Control register */
            reg_offset = PCIE_GLOBAL_CONTROLL;
            if (plat_mem_read32(reg_offset, &reg_val) != PASSED) {
                printf("Failed to read CPU register 0x%08X (link control link status).\n", reg_offset);
                return (FAILED);
            }

            /* Write CFGInitRst to 1 */
            reg_val |= CFGINITRST; 
            reg_offset = PCIE_GLOBAL_CONTROLL;
            if (plat_mem_write32(reg_offset, reg_val) != PASSED) {
                printf("Failed to write CPU register 0x%08X (link control link status).\n"
                       , reg_offset);
                return (FAILED);
            }

            /* Write CFGInitRst to 0 */
            reg_val &= ~(CFGINITRST); 
            reg_offset = PCIE_GLOBAL_CONTROLL;
            if (plat_mem_write32(reg_offset, reg_val) != PASSED) {
                printf("Failed to write CPU register 0x%08X (link control link status).\n"
                       , reg_offset);
                return (FAILED);
            }
            break; //break link_control_link_status while loop
        }
        link_control_link_status_timeout--;
        msleep(10);
    }

    if (link_control_link_status_timeout == 0) {
        printf("%s:%d:Fail to polling PCIE link control link status status reg bit4=0 and bit5=0 \n",
               __FUNCTION__, __LINE__);
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 * Function   : plug_testcard_pcie_device_remove
 * Description: The function was to remove pcie device 
 * Inputs     : none
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
void plug_testcard_pcie_device_remove (void)
{
    system(REMOVE_PIM_TESTCARD_PCIE_DEVICE);
}




/*-------------------------------------------------
 * $Log: plug_common_host_impl.c,v $
 * Revision 1.2  2021/09/24 01:21:08  harrchan
 * Collapse Elixir-branch to Main Trunk.
 *
 * Revision 1.1.2.4  2021/05/26 04:20:39  harrchan
 * Base on code review comments to clean up
 *
 * Revision 1.1.2.3  2021/04/12 02:53:58  harrchan
 * Add USB power down and up into USB test process
 *
 * Revision 1.1.2.2  2020/11/12 09:01:55  harrchan
 * Remove power off PIM module on PIM usb test. Because currently we had pcie hang issue on elixir
 *
 * Revision 1.1.2.1  2020/09/09 09:08:08  illiu
 * First version which has been ported with Dreamliner and Marvel CPSS
 *
 * Revision 1.2  2019/01/10 06:36:25  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
