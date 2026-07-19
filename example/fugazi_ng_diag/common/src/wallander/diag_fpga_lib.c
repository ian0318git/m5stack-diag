/* $Id: diag_fpga_lib.c,v 1.1 2015/02/26 07:18:29 xiaoyizh Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/wallander/diag_fpga_lib.c,v $
 *-----------------------------------------------------------------------------
 * diag_fpga_lib.c - Wallander FPGA Library
 *
 * Apr 2014, Xiaoying Zhang
 * Copyright (c) 2015 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */
#include "error.h"
#include "common.h"
#include "types.h"
#include "menu.h"
#include <stdio.h>
#include "diag_fpga_lib.h"
#include "i2c_api.h"
#include "platform_i2c.h"
#include "dev_tmp421.h"
#include "i2c_dev.h"
#include "ethernet.h"

#include <fcntl.h>
#include <asm/ioctl.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>

uchar *local_bus_mmap_addr = NULL;

extern int32_t cavium_i2c_fd0;
extern int32_t cavium_i2c_fd1;

extern void msleep(unsigned long);

int fpga_upgrade_sector_erase(int);
int is_sfp_present(int, int);
int get_board_id(void);
int enable_sfp_tx_transmit(int, int);
uchar* fpga_get_local_bus_addr(void);
int fpga_toggle_sfp_led(int, int, int);

/******************************************************************************
 *
 * Function    : fpga_reg_i2c_read
 * Description : FPGA Register Read through Cavium I2C iface
 * Input       : addr  - register offset.
 *               buf   - read buffer
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************************/
int fpga_reg_i2c_read (int addr, char *buf)
{
    n2g_i2c_dev_t i2c_dev;
    uint i2c_slave_addr;
    i2c_slave_addr = CAVIUM_I2C_FPGA;

    i2c_dev.rd_hd_size = 1;
    i2c_dev.wr_hd_size = 1;

    /* Open the Cavium I2C bus 0 */
    if (open_i2c(&i2c_dev, i2c_slave_addr, CPU_I2C0) == FAILED) {
        return (FAILED);
    }

    /* Call the I2C common I2C api to read back the register value */
    return (read_i2c_reg(&i2c_dev, (uchar *)buf, addr, sizeof(fpga_p)));
}

/******************************************************************************
 *
 * Function    : fpga_reg_i2c_write
 * Description : FPGA Register Write through Cavium I2C iface
 * Input       : addr  - register offset.
 *               data  - data for write
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************************/
int fpga_reg_i2c_write (int addr, char data)
{
    n2g_i2c_dev_t i2c_dev;
    uint i2c_slave_addr;
    i2c_slave_addr = CAVIUM_I2C_FPGA;

    i2c_dev.rd_hd_size = 1;
    i2c_dev.wr_hd_size = 1;

    /* Open the Cavium I2C bus 0 */
    if (open_i2c(&i2c_dev, i2c_slave_addr, CPU_I2C0) == FAILED) {
        return (FAILED);
    }

    /* Call the I2C common I2C api to write the register value */
    return (write_i2c_reg(&i2c_dev, (uchar *)&data, addr, sizeof(fpga_p)));
}

/******************************************************************************
 *
 * Function    : fpga_reg_read
 * Description : FPGA Register Read through Cavium Local Bus
 * Input       : addr  - register offset.
 *               buf   - read buffer
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************************/
int fpga_reg_read (int addr, char *buf)
{
    uchar *fpga_offset; 

    fpga_offset = fpga_get_local_bus_addr();
    if (fpga_offset == NULL) {
        return (FAILED);
    }

    *buf = *(fpga_offset + addr);

    return (PASSED);
}

/******************************************************************************
 *
 * Function    : fpga_reg_write
 * Description : FPGA Register Write through Cavium Local Bus
 * Input       : addr  - register offset.
 *               data  - data for write
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************************/
int fpga_reg_write (int addr, char data)
{
    uchar *fpga_offset;

    fpga_offset = fpga_get_local_bus_addr();
    if (fpga_offset == NULL) {
        return (FAILED);
    }

    *(fpga_offset + addr) = data;
//     printf("%s: addr = %#x data = %#x\n", __FUNCTION__, addr, data);
    return (PASSED);
}

/******************************************************************************
 *
 * Function    : fpga_reg_read32
 * Description : FPGA Register Read 4 times to get 32-bit value
 * Input       : addr  - register offset.
 *               buf   - read buffer
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************************/
int fpga_reg_read32 (int addr, ulong *buf)
{
    uchar *fpga_offset;
    uchar tmp;
    int i;
    *buf = 0;

    fpga_offset = fpga_get_local_bus_addr();
    if (fpga_offset == NULL) {
        return (FAILED);
    }

    /* The address must be 4-byte aligned */
    addr &= ~(0x3);
    for (i = 0; i < 4; i++, addr++) {
        tmp = *(fpga_offset + addr);
        *buf |= (tmp << (i*8));
//         printf("%s: addr = %#x value = %#x *buf = %#x\n", __FUNCTION__, addr, tmp, *buf);
    }

    return (PASSED);
}

/******************************************************************************
 *
 * Function    : fpga_reg_write
 * Description : FPGA Register Write 32-bit data
 * Input       : addr  - register offset.
 *               data  - data for write
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************************/
int fpga_reg_write32 (int addr, ulong data)
{
    uchar *fpga_offset;
    int i;

    fpga_offset = fpga_get_local_bus_addr();
    if (fpga_offset == NULL) {
        return (FAILED);
    }

    /* The address must be 4-byte aligned */
    addr &= ~(0x3);

    for (i = 0; i < 4; i++, addr++) {
        *(fpga_offset + addr) = data & 0xff;
//         printf("%s: addr = %#x data = %#x\n", __FUNCTION__, addr, data);
        data = data >> 8;
    }

    return (PASSED);
}

/******************************************************************************
 *
 * Function    : fpga_reg_or
 * Description : Set reg0x70, bit5=1 for starting flash update.
 * Input       : offset  - register offset.
 *                  bit  - data for write
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************************/
int fpga_reg_or (int offset, char bit)
{
    char val;

    fpga_reg_read(offset, &val);
//     printf("%s: offset %#x val %#x set bit %#x\n", __FUNCTION__, offset, val, bit);
    val |= bit;
    fpga_reg_write(offset, val);

    return (PASSED);
}

/******************************************************************************
 *
 * Function    : fpga_reg_nand
 * Description : Disable flash update.
 * Input       : offset  - register offset.
 *                  bit  - data for write
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************************/
int fpga_reg_nand (int offset, char bit)
{
    char val;

    fpga_reg_read(offset, &val);
    val &= ~(bit);
    fpga_reg_write(offset, val);

    return (PASSED);
}

/*****************************************************************************
 *
 * Function   : fpga_set_ready_bit
 * Description: Set primary interface ready
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
int fpga_set_ready_bit (void)
{
    return fpga_reg_or(FPGA_GPIO_EXP_LOW, FPGA_PRI_IF_RDY);
}

/*****************************************************************************
 *
 * Function   : fpga_reset_phy
 * Description: Reset PHY
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
int fpga_reset_phy (void)
{
    if (fpga_reg_nand(FPGA_CPU_DEV_RST, FPGA_PHY_RST_L)) {
        return (FAILED);
    }
    sleep(1);
    return fpga_reg_or(FPGA_CPU_DEV_RST, FPGA_PHY_RST_L);
}

/*****************************************************************************
 *
 * Function   : fpga_enable_phy_coma_mode_output
 * Description: Enable PHY COMA_MODE output
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
int fpga_enable_phy_coma_mode_output (void)
{
    return fpga_reg_or(FPGA_PHY_ZL30254_CTRL_STAT, PHY_COMA_MODE_OUTPUT_EN);
}

/*****************************************************************************
 *
 * Function   : fpga_turn_on_phy_coma_mode
 * Description: Turn on PHY COMA_MODE
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
int fpga_turn_on_phy_coma_mode (void)
{
    return fpga_reg_or(FPGA_PHY_ZL30254_CTRL_STAT, PHY_COMA_MODE_OUTPUT);
}

/*****************************************************************************
 *
 * Function   : fpga_turn_off_phy_coma_mode
 * Description: Turn off PHY COMA_MODE
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
int fpga_turn_off_phy_coma_mode (void)
{
    return fpga_reg_nand(FPGA_PHY_ZL30254_CTRL_STAT, PHY_COMA_MODE_OUTPUT);
}

/*****************************************************************************
 *
 * Function   : fpga_enable_ts
 * Description: Enable PHY Timestamp function
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
int fpga_enable_ts (void)
{
    return fpga_reg_or(FPGA_TS_CTRL, PHY_TS_ENABLE);
}

/*****************************************************************************
 *
 * Function   : fpga_enable_ts_intr
 * Description: Enable PHY Timestamp interrupt
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
int fpga_enable_ts_intr (void)
{
    return fpga_reg_or(FPGA_TS_CTRL, PHY_TS_INTR_ENABLE);
}

/*****************************************************************************
 *
 * Function   : fpga_check_ts_intr
 * Description: Check whether PHY Timestamp interrupt is triggered.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
int fpga_check_ts_intr (void)
{
    char val;

    fpga_reg_read(FPGA_TS_INT_CTRL, &val);
    printf("FPGA_TS_INT_CTRL: 0x%x\n", val);

    if (val & PHY_TS_INT) {
        return (PASSED);
    } else {
        return (FAILED);
    }
}

/*****************************************************************************
 *
 * Function   : fpga_check_ts_ready
 * Description: Check whether PHY Timestamp is ready
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
int fpga_check_ts_ready (void)
{
    char val;

    fpga_reg_read(FPGA_TS_STAT, &val);
    printf("FPGA_TS_STAT: 0x%x\n", val);

    if (val & PHY_TS_READY) {
        return (PASSED);
    } else {
        return (FAILED);
    }
}

/******************************************************************************
 *
 * Function    : is_sfp_present
 * Description : Get sfp status and control register, 0-sfp module is present.
 * Input       : phy - PHY address (0/1)
 *               sfp - SFP I2C device number.
 *
 * Output: TRUE/FALSE
 *
 *****************************************************************************/
int is_sfp_present (int phy, int sfp)
{
    char buf;
    int is_present, fpga_addr;

    fpga_addr = FPGA_GEP0_G0_SFP_STS + sfp;

    switch(sfp){
        case SFP0:
            fpga_addr = FPGA_PORT0_INT_STS;
            break;
        case SFP1:
            fpga_addr = FPGA_PORT2_INT_STS;
            break;
        default:
            printf("error: not support this SFP port num\n");
            return (FALSE);
    }

    if (fpga_reg_read(fpga_addr, &buf) == FAILED) {
        return (FALSE);
    }

    is_present = buf & GEP_SFP_STS_PRESENT;

    if (is_present == 0) {
        return (TRUE);
    } else {
        return (FALSE);
    }
}

/******************************************************************************
 *
 * Function    : enable_sfp_tx
 * Description : Enable SFP Transmit.
 * Input       : sfp - SFP port number.
 *
 * Output: TRUE/FALSE
 *
 *****************************************************************************/
int enable_sfp_tx (int sfp)
{
    char buf;
    int fpga_addr;

    switch(sfp){
        case SFP0:
            fpga_addr = FPGA_PORT0_INT_CTRL;
            break;
        case SFP1:
            fpga_addr = FPGA_PORT2_INT_CTRL;
            break;
        default:
            printf("error: not support this SFP port num\n");
            return (FAILED);
    }

    if (is_sfp_present(0, sfp) == FALSE) {
        printf("SFP-%d is not detected\n", sfp);
        return (FAILED);
    }

    if (fpga_reg_read(fpga_addr, &buf) == FAILED) {
        return (FAILED);
    }

    buf &= ~(GEP_SFP_CTL_TX_DISABLE);

    if (fpga_reg_write(fpga_addr, buf) == FAILED) {
        return (FAILED);
    } else {
        return (PASSED);
    }
}


/******************************************************************************
 *
 * Function    : show_sfp_status
 * Description : Show sfp status
 * Input       : phy - PHY address (0/1)
 *               sfp - SFP I2C device number.
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************************/
int show_sfp_status (int phy, int sfp)
{
    char buf;
    int fpga_addr;
    int is_present;
    int tx_fault;
    int rx_loss;

//     fpga_addr = FPGA_GEP0_G0_SFP_STS + sfp;

    switch(sfp){
        case SFP0:
            fpga_addr = FPGA_PORT0_INT_STS;
            break;
        case SFP1:
            fpga_addr = FPGA_PORT2_INT_STS;
            break;
        default:
            printf("error: not support this SFP port num\n");
            return (FAILED);
    }

    if (fpga_reg_read(fpga_addr, &buf) == FAILED) {
        return (FALSE);
    }

    printf("FPGA reg @%#x = %#x\n", fpga_addr, buf);

    is_present = buf & GEP_SFP_STS_PRESENT;
    tx_fault = buf & GEP_SFP_STS_TX_FAULT;
    rx_loss = buf & GEP_SFP_STS_RX_LOSS;

    printf("\n    ==== SFP-%d Status ====\n", sfp);
    printf("        Present:  %s\n", is_present ? "No" : "Yes");
    printf("        TX_FAULT: %s\n", tx_fault ? "Yes" : "No");
    printf("        RX_LOSS:  %s\n\n", rx_loss ? "Yes" : "No");

    return (PASSED);
}

/******************************************************************************
 *
 * Function    : enable_sfp_tx_transmit
 * Description : Turn on Tx transmit
 * Input       : phy - PHY address (0/1)
 *               sfp - SFP I2C device number.
 *
 * Output: TRUE/FALSE
 *
 *****************************************************************************/
int enable_sfp_tx_transmit (int phy, int sfp)
{
    char buf;
    int fpga_addr;

    switch(sfp){
        case SFP0:
            fpga_addr = FPGA_PORT0_INT_STS;
            break;
        case SFP1:
            fpga_addr = FPGA_PORT2_INT_STS;
            break;
        default:
            printf("error: not support this SFP port num\n");
            return (FAILED);
    }

    if (fpga_reg_i2c_read(fpga_addr, &buf) == FAILED) {
        return (FAILED);
    }

    buf &= ~(GEP_SFP_CTL_TX_DISABLE);

    if (fpga_reg_i2c_write(fpga_addr, buf) == FAILED) {
        return (FAILED);
    }

    return (PASSED);
}


/******************************************************************************
 *
 * Function    : fpga_ctrl_sfp_led
 * Description : This function controls SFP LED
 * Input       : port - port number
 *               led - which LED, Yellow/Green/Speed
 *               mode - turn on/off or speed blink
 *
 * Output: TRUE/FALSE
 *
 *****************************************************************************/
int fpga_ctrl_sfp_led (int port, int led, int mode)
{
    char buf = 0;
    int fpga_addr = FPGA_SFP_LED_CTRL;
    char shift = 0;

    if (port == 1) {
        shift = 4;
    } else {
        shift = 0;
    }

    if (fpga_reg_read(fpga_addr, &buf) == FAILED) {
        return (FAILED);
    }

    switch (led) {
    /* Turn on/off LED Yellow */ 
    case FPGA_SFP_LED_Y:
        if (mode) {
            buf |= (GEP_SFP_LED_Y) << shift;
        } else {
            buf &= ~((GEP_SFP_LED_Y) << shift);
        }
        break;

    /* Turn on/off LED Green */ 
    case FPGA_SFP_LED_G:
        if (mode) {
            buf |= (GEP_SFP_LED_G) << shift;
        } else {
            buf &= ~((GEP_SFP_LED_G) << shift);
        }
        break;

    /* Set mode for LED Speed */
    /*  11: 3 blinks 
        10: 2 blinks 
        01: 1 blinks 
        00: 0 blinks
    */
    case FPGA_SFP_LED_SPD:
        buf &= ~((GEP_SFP_LED_SPD) << shift);
        buf |= (mode << shift);
        break;

    default:
        printf("\nInvalid port number (%d)\n", port);
        return (FAILED);
    }

    if (fpga_reg_write(fpga_addr, buf) == FAILED) {
        return (FAILED);
    }

    return (PASSED);
}

/******************************************************************************
 *
 * Function: get_board_id
 *
 * Description: This function read fpga id to distinguish SKU type.
 *
 * Inputs      : None
 * Outputs     : 0/1
 *
 *****************************************************************************/
int get_board_id (void)
{
    char reg_val, id;

    /* Read fpga id, bit0 */
    if (fpga_reg_read(FPGA_BOARD_ID, &reg_val) == FAILED) {
        cterr('f', 0, "Read FPGA Board ID failed");
        return (FAILED);
    }
    id = reg_val & FPGA_ID_MASK;

//     printf("Board ID is: %d\n", id);
    return id;
}

/******************************************************************************
 *
 * Function    : fpga_get_local_bus_addr
 * Description : Create a new mapping in the virtual address space.
 * Input       : None.
 *
 * Output: local_bus_mmap_addr/NULL
 *
 *****************************************************************************/
uchar* fpga_get_local_bus_addr (void)
{
    int fd;

    if (local_bus_mmap_addr == NULL) {
        fd = open("/dev/mem", O_RDWR);
        if (fd < 0) {
            printf("Open MEM device failed");
            return (NULL);
        }

        local_bus_mmap_addr = mmap(0, FPGA_LOCAL_BUS_LENGTH, PROT_READ | PROT_WRITE, MAP_SHARED, 
                           fd, FPGA_LOCAL_BUS_START_ADDR);
        if (local_bus_mmap_addr == MAP_FAILED) {
            printf("Unable to create a new mapping in the virtual address space\n");
            local_bus_mmap_addr = NULL;
        }
    }

    return (local_bus_mmap_addr);
}
/*-------------------------------------------------
 * $Log: diag_fpga_lib.c,v $
 * Revision 1.1  2015/02/26 07:18:29  xiaoyizh
 * Initial check in for Wallander.
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
