/* $Id: diag_fpga_lib.c,v 1.3 2015/02/14 12:48:41 kodko Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn_linux/diag_fpga_lib.c,v $
 *-----------------------------------------------------------------------------
 * diag_fpga_lib.c - Woodlawn FPGA Library
 *
 * February 2012, Times Huang
 * Copyright (c) 2013 - 2015 by Cisco Systems, Inc.
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

#include <fcntl.h>
#include <asm/ioctl.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>

uchar *mmap_addr = NULL;

extern int32_t cavium_i2c_fd0;
extern int32_t cavium_i2c_fd1;

extern void msleep(unsigned long);

int fpga_reg_read(int, char *);
int fpga_reg_write(int, char);

int fpga_reg_or(int, char);
int fpga_reg_nand(int, char);
int fpga_upgrade_sector_erase(int);
int is_sfp_present(int, int);
int get_sku_id(void);
int enable_sfp_tx_transmit(int, int);
uchar* fpga_get_local_bus_addr(void);
int fpga_toggle_sfp_led(int, int, int);
/*------------------------------------------------------------------*/

/******************************************************************************
 *
 * Function    : fpga_upgrade_sector_erase
 * Description : Erase FPGA sectors.
 * Input       : offset  - sector offset.
 *              
 * Output: PASSED/FAILED
 *
 *****************************************************************************/
int fpga_upgrade_sector_erase (int offset)
{
    int ix;
    char val;

    /* 1. Set reg0x7C for flash sector erase select */
    fpga_reg_write(FPGA_REMOTE_UPD_SPI_ADDR3, offset);

    /* 2. Set reg0x71 bit2=1 for sector erase req */
    fpga_reg_or(FPGA_REMOTE_UPD_CTRL_REG, REMOTE_UPDATE_FLASH_SECTOR_ERASE);

    /* 3. Check Reg0x72 bit1 until equal to 0 for each sector erasing */
    for (ix = 0; ix < SECTOR_ERASE_TIMEOUT; ix++) {
        fpga_reg_read(FPGA_REMOTE_UPD_STS_REG, &val);

        /* Check if the sector erase is completed */
        if (!(val & REMOTE_UPDATE_STS_FLASH_BUSY)) {
            return (PASSED);
        }

        msleep(1);
    }

    printf("Erasing sector %d fails (Timeout)\n", offset);

    return (FAILED);
}

/******************************************************************************
 *
 * Function    : fpga_reg_read
 * Description : FPGA Register Read through Cavium I2C iface
 * Input       : addr  - register offset.
 *               buf   - read buffer
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************************/
int fpga_reg_read (int addr, char *buf)
{
    n2g_i2c_dev_t i2c_dev;
    uint i2c_slave_addr;
    i2c_slave_addr = CAVIUM_FPGA;

    /* Open the Cavium I2C bus 1 */
    if (open_i2c(&i2c_dev, i2c_slave_addr, CPU_I2C1) == FAILED) {
        return (FAILED);
    }

    /* Call the I2C common I2C api to read back the register value */
    return (read_i2c_reg(&i2c_dev, (uchar *)buf, addr, sizeof(fpga_p)));
}

/******************************************************************************
 *
 * Function    : fpga_i2c_write
 * Description : FPGA Register Write through Cavium I2C iface
 * Input       : addr  - register offset.
 *               data  - data for write
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************************/
int fpga_reg_write (int addr, char data)
{
    n2g_i2c_dev_t i2c_dev;
    uint i2c_slave_addr;
    i2c_slave_addr = CAVIUM_FPGA;
    
    /* Open the Cavium I2C bus 1 */
    if (open_i2c(&i2c_dev, i2c_slave_addr, CPU_I2C1) == FAILED) {
        return (FAILED);
    }

    /* Call the I2C common I2C api to write the register value */
    return (write_i2c_reg(&i2c_dev, (uchar *)&data, addr, sizeof(fpga_p)));
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

    if (phy == 0) {
        fpga_addr = FPGA_GEP0_G0_SFP_STS + sfp;
    } else {
        fpga_addr = FPGA_GEP1_G0_SFP_STS + sfp;
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

    if (phy == 0) {
        fpga_addr = FPGA_GEP0_G0_SFP_CTL + sfp;
    } else {
        fpga_addr = FPGA_GEP1_G0_SFP_CTL + sfp;
    }

    if (fpga_reg_read(fpga_addr, &buf) == FAILED) {
        return (FAILED);
    }

    buf &= ~(GEP_SFP_CTL_TX_DISABLE);

    if (fpga_reg_write(fpga_addr, buf) == FAILED) {
        return (FAILED);
    }

    return (PASSED);
}


/******************************************************************************
 *
 * Function    : fpga_toggle_sfp_led
 * Description : This function toggles SFP LED
 * Input       : port - port number
 *               led - which LED, Enable(Upper)/Speed(Lower)
 *               on - TRUE to turn on, FALSE to turn off
 *
 * Output: TRUE/FALSE
 *
 *****************************************************************************/
int fpga_toggle_sfp_led (int port, int led, int on)
{
    char buf;
    int fpga_addr;

    switch (port) {
    case 0:
    case 1:
        fpga_addr = PHY_STS_LED_REG_2;
        break;
    case 2:
    case 3:
        fpga_addr = PHY_STS_LED_REG_0;
        break;
    case 4:
    case 5:
        fpga_addr = PHY_STS_LED_REG_1;
        break;
    case 6: /* SFP Plus */
        fpga_addr = PHY_STS_LED_REG_3;
        break;
    default:
        printf("\nInvalid port number (%d)\n", port);
        return (FAILED);
    }

    if (fpga_reg_read(fpga_addr, &buf) == FAILED) {
        return (FAILED);
    }

    if (led == FPGA_SFP_LED_EN) {
        if (on == TRUE) {
            buf |= ((0x8) << ((port % 2) * 4));
        } else {
            buf &= ~((0x8) << ((port % 2) * 4));
        }
    } else {
        if (on == TRUE) {
            buf |= ((0x3) << ((port % 2) * 4));
        } else {
            buf &= ~((0x3) << ((port % 2) * 4));
        }
    }

    if (fpga_reg_write(fpga_addr, buf) == FAILED) {
        return (FAILED);
    }

    return (PASSED);
}

/******************************************************************************
 *
 * Function: get_sku_id
 *
 * Description: This function read fpga id to distinguish SKU type.
 *
 * Inputs      : None
 * Outputs     : WOODLAWN_4GE_1XAUI/WOODLAWN_6GE/WOODLAWN_6GE_1XAUI
 *
 *****************************************************************************/
int get_sku_id (void)
{
    char reg_val, id;
    
    /* Read fpga id, bit4 - fpga id1, bit5 - fpga id2 */
    if (fpga_reg_read(FPGA_HIGH_VER_REG, &reg_val) == FAILED) {
        cterr('f', 0, "Read FPGA ID failed");
        return (FAILED);
    }
    id = (reg_val & (~FPGA_ID_MASK)) >> 4;
   
    /* SKU1 ID = 01(4 port sfp, 1 port sfp+), SKU2 ID = 00(6 port sfp). */
    if (id == FPGA_ID_SKU1) {
        return (WOODLAWN_4GE_1XAUI);
    } else if (id == FPGA_ID_SKU2) {
        return (WOODLAWN_6GE);
    } else {
        return (WOODLAWN_6GE_1XAUI);
    }
}

/******************************************************************************
 *
 * Function    : fpga_get_local_bus_addr
 * Description : Create a new mapping in the virtual address space.
 * Input       : None.
 *              
 * Output: mmap_addr/NULL
 *
 *****************************************************************************/
uchar* fpga_get_local_bus_addr (void)
{
    int fd;

    if (mmap_addr == NULL) {
        fd = open("/dev/mem", O_RDWR);
        if (fd < 0) {
            printf("Open MEM device failed");
            return (NULL);
        }

        mmap_addr = mmap(0, FPGA_LOCAL_BUS_LENGTH, PROT_READ | PROT_WRITE, MAP_SHARED, 
                           fd, FPGA_LOCAL_BUS_START_ADDR);
        if (mmap_addr == MAP_FAILED) {
            printf("Unable to create a new mapping in the virtual address space\n");
            mmap_addr = NULL;
        }
    }
    
    return (mmap_addr);
}

/***********************************************************************
 *  
 * Function: verify_fpga_sync_clk_out
 *    
 * Description: Verify SYNC_OUT from BP to FPGA
 *      
 * Inputs: None
 *        
 * Outputs: PASSED/FAILED
 *          
 *************************************************************************/
int verify_fpga_sync_clk_out (void)
{
    int ix;
    char fpga_val, sync_out_clk, sync_out_valid;

    for (ix = 0; ix < FPGA_WAIT_PHY_READY; ix++) {
        if (fpga_reg_read(FPGA_RESET_SIGNAL_REG, &fpga_val) == FAILED) {
            printf("Read FPGA register %#.8x failed\n", FPGA_RESET_SIGNAL_REG);
            fflush(stdout);
            return (FAILED);
        }

        /* If bit 0 is "1" - Finish reset and PHY is ready */
        if ((fpga_val & GE_PHY_READY_MASK) == GE_PHY_READY) {
            break;
        }
        msleep(100);
    }

    if (ix == FPGA_WAIT_PHY_READY) {
        printf("PHY is not ready\n");
        fflush(stdout);
        return (FAILED);
    }

    /* Read FPGA reg 0x0e to verify that the sync_out clock is valid and the frequency is right */
    if (fpga_reg_read(DEV_STATUS_REG, &fpga_val) == FAILED) {
        printf("Read FPGA register %#.8x failed\n", DEV_STATUS_REG);
        fflush(stdout);
        return (FAILED);
    }

    sync_out_valid = (fpga_val & FPGA_SYNC_OUT_VALID_MASK) >> FPGA_SYNC_OUT_VALID_SHIFT;
    if (sync_out_valid != SYNC_OUT_CLK_VALID) {
        printf("Invalid sync_out_clock\n");
        fflush(stdout);
        return (FAILED);
    }

    sync_out_clk = fpga_val >> FPGA_SYNC_OUT_SHIFT;
    if (sync_out_clk == SYNC_OUT_CLK_25M) {
        printf("25MHz SYNC_CLK_OUT is fed into FPGA successfully\n");
        fflush(stdout);
    } else {
        printf("8KHz SYNC_CLK_OUT is fed into FPGA successfully\n");
        fflush(stdout);
    } 

    return (PASSED);
}


/***********************************************************************
 *  
 * Function: verify_fpga_sync_trig_out
 *    
 * Description: Verify SYNC_TRIG_OUT from BP to FPGA
 *      
 * Inputs: None
 *        
 * Outputs: PASSED/FAILED
 *          
 *************************************************************************/
int verify_fpga_sync_trig_out (void)
{
    int sku_id;
    int ix, switch_cnt = 0;
    char fpga_val, fpga_trig_mask;
    char fpga_val_pre, fpga_val_curr;

    /* Get the SKU id */
    sku_id = get_sku_id();

    for (ix = 0; ix < FPGA_WAIT_PHY_READY; ix++) {
        if (fpga_reg_read(FPGA_RESET_SIGNAL_REG, &fpga_val) == FAILED) {
            printf("Read FPGA register %#.8x failed\n", FPGA_RESET_SIGNAL_REG);
            fflush(stdout);
            return (FAILED);
        }

        /* If bit 0 is "1" - Finish reset and PHY is ready */
        if ((fpga_val & GE_PHY_READY_MASK) == GE_PHY_READY) {
            break;
        }
        msleep(100);
    }

    if (ix == FPGA_WAIT_PHY_READY) {
        printf("PHY is not ready\n");
        fflush(stdout);
        return (FAILED);
    }

    /* Read FPGA reg 0xA1 to verify PTP 1PPS trigger out */
    if (fpga_reg_read(FPGA_PTP_TRIG_OUT_SEL_REG, &fpga_val) == FAILED) {
        printf("Read FPGA register %#.8x failed\n", FPGA_PTP_TRIG_IN_SEL_REG);
        fflush(stdout);
        return (FAILED);
    }

    /* bit2 represent trigger from BP */
    fpga_trig_mask = FPGA_SYCN_TRIG_OUT;

    for (ix = 0, switch_cnt = 0; ix < FPGA_TRIG_VERIFY_TIME; ix++) {
        fpga_val_pre = (fpga_val & fpga_trig_mask);

        if (fpga_reg_read(FPGA_PTP_TRIG_OUT_SEL_REG, &fpga_val) == FAILED) {
            printf("Read FPGA register %#.8x failed\n", FPGA_PTP_TRIG_IN_SEL_REG);
            fflush(stdout);
            return (FAILED);
        }
            
        fpga_val_curr = (fpga_val & fpga_trig_mask);
        /* Check whether the bit change from low to high */
        if (fpga_val_pre != fpga_val_curr) {
            if (fpga_val_curr) {
                switch_cnt++;
            }

            if (switch_cnt == FPGA_TRIG_VERIFY_NUM) {
                break;
            }
        }
        msleep(1);
    }

    if (switch_cnt < FPGA_TRIG_VERIFY_NUM) {
        printf("SYNC_TRIG_OUT is not fed into FPGA successfully\n");
        fflush(stdout);
        return (FAILED);
    }

    return (PASSED);
}

/*-------------------------------------------------
 * $Log: diag_fpga_lib.c,v $
 * Revision 1.3  2015/02/14 12:48:41  kodko
 * Collapse timing card branch code into main trunk.
 *
 * Revision 1.2.8.1  2014/04/30 13:47:22  kodko
 * Support 1548P/2222P clock/trigger in/out verification.
 *
 * Revision 1.2  2013/10/08 08:48:27  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.4.2  2013/08/20 10:58:51  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.2  2013/06/13 11:42:44  tirawan
 * Implement LED nc dispatch command for host side to be able to control SM LED
 *
 * Revision 1.1.2.1  2013/04/24 10:37:15  tirawan
 * Initial check-in for woodlawn linux code
 *
 * Revision 1.5  2013/03/27 07:58:21  leslie
 * Add mmap to get local bus address
 *
 * Revision 1.4  2013/03/27 04:49:35  kuangik
 * Code cleanup after adding -Wall
 *
 * Revision 1.3  2013/03/20 03:09:38  kuangik
 * Enable tx transmit on SFP module when running sfp ext loopback
 *
 * Revision 1.1  2013/03/13 06:42:50  kuangik
 * Add for the first time
 *
 * Revision 1.19  2013/03/01 13:51:55  kuangik
 * Update Loopback Test, SFP Present, and SFP EEPROM display
 *
 * Revision 1.18  2013/02/26 01:35:54  leslie
 * Remove pre-define woodlawn old sku.
 *
 * Revision 1.17  2013/01/18 06:24:17  leslie
 * Fix and clean up code.
 *
 * Revision 1.16  2013/01/15 23:32:01  leslie
 * Fix the return value of different SKUs.
 *
 * Revision 1.15  2012/11/20 01:24:57  leslie
 * Fix detect sfp present function.
 *
 * Revision 1.14  2012/11/19 02:13:50  leslie
 * Fix detect sfp module present function
 *
 * Revision 1.13  2012/10/24 10:26:42  leslie
 * Fix argument type.
 *
 * Revision 1.12  2012/10/04 03:17:14  leslie
 * Update for FPGA program utility.
 *
 * Revision 1.11  2012/08/28 08:31:59  leslie
 * Fix function fpga_reg_write.
 *
 * Revision 1.10  2012/08/18 02:35:50  leslie
 * Open I2C bus 1
 *
 * Revision 1.9  2012/08/03 10:16:55  leslie
 * Mapping to latest O2 source code on 20120726
 *
 * Revision 1.7  2012/07/19 06:39:17  leslie
 * Remove library functoin open_i2c
 *
 * Revision 1.6  2012/05/30 01:31:17  leslie
 * Add function open_i2c_1, fpga_reg_read, fpga_reg_write
 *
 * Revision 1.5  2012/04/16 12:33:39  kuangik
 * Add FPGA Firmware upgrade function
 *
 * Revision 1.4  2012/04/06 06:03:29  kuangik
 * Update for FPGA Test Item
 *
 * Revision 1.3  2012/03/26 07:17:59  kody
 * Add stdio.h
 *
 * Revision 1.2  2012/02/10 06:44:20  leslie
 * Add symbol for cvs history comment
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
