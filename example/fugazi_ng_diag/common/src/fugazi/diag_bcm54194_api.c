/* $Id: diag_bcm54194_api.c,v 1.2 2021/06/02 08:22:34 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/diag_bcm54194_api.c,v $
 *-----------------------------------------------------------------------------
 * bcm54194_api.c - API for BCM GE PHY bcm54194.
 *
 *
 * June 2016, Mecca Ho
 * Jan 2019, Letsai modified for Fugazi.
 *
 * Copyright (c) 2016 - 2020 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */
#include <stdlib.h>
#include <stdio.h>
#include <linux/socket.h>
#include <sys/ioctl.h>
#include <string.h>
#include <unistd.h>
#include <linux/if_ether.h>
#include <netinet/in.h>
#include "types.h"
#include "common.h"
#include "queryflags.h"
#include "platform_eth.h"
#include "platform_ext_lpbk.h"
#include "diag_bcm54194_api.h"
#include "dash_fpga.h"
#include "diag_bnxt.h"
#include "diag_bcm_lib.h"
#include "diag_miura_reg.h"
#include "nvsysvars.h"


extern void reset_platform_ext_dev(int);
extern void unreset_platform_ext_dev(int);

extern int is_glc_ge_100fx;

extern void msleep(unsigned long t);
extern struct fugazi *fugazi_struct;

/* Dummy value for 1G phy
 * Only 10G phy need device id */
int devad = FUGAZI_MIURA_DEV_1G_PHY;

/* setup delay time for driver to read the PHY reg. */
#define ETH_DRIVER_DELAY    1

/*
 * Function: bcm54194_suspend_lnx_link_polling
 *
 * Description: To accomplish read the RDB reg.
 *              Need to write the RDB reg addr to 0x1E and read RDB reg val from 0x1F.
 *              It will impact user normal operation when linux driver polling the RDB reg.
 *
 * Input: type - port type
 *        eth_num - eth number
 *        suspend_update - TRUE/FALSE
 *
 * Return: PASSED/FAILED
 */
int bcm54194_suspend_lnx_link_polling (char *type, int eth_num, boolean suspend_update)
{
    int sk;
    struct ifreq ethreq;
    char pname[10];

    sprintf(pname,"%s%d", type, eth_num);

    /* Create socket for ioctl calls
     */
    sk = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP));
    if (-1 == sk) {
        printf("%s() Error Creating RX Socket", __FUNCTION__);
        return(FAILED);
    }

    strncpy(ethreq.ifr_name, pname, IFNAMSIZ);

    if (suspend_update) {
        ioctl(sk, SIOCNEPSUSPENDUPLINK, &ethreq);
    } else {
        ioctl(sk, SIOCNEPRESUMEUPLINK, &ethreq);
    }

    sleep(ETH_DRIVER_DELAY);

    close(sk);

    return (0);
}

/*
 * Function: bcm54194_rdb_access_enable
 *
 * Description: Enable BCM54194 RDB access mode.
 *  According to "BCM54194 register guide", enable BCM54194 RDB access mode by
 *        Write Register 0x17 = 0x0F7E
 *        Write Register 0x15 = 0x0000
 *   Hardware or software resets to the chip will enable RDB Access mode.
 *
 * Input: phy_num - PHY number
 *        phy_addr - PHY mdio address
 *
 * Return: PASSED/FAILED
 */
int bcm54194_rdb_access_enable (int phy_num, int phy_addr)
{
    int rc, regnum;

    /* Enable RDB access mode */

    /* step 1: Write Register 0x17 = 0x0F7E */
    regnum = 0x17;
    rc = fugazi_bnxt_mdio_write(&fugazi_struct->bnxt[phy_num], phy_addr, devad, regnum, 0x0F7E);
    if (rc != PASSED) {
        printf("Failed to write GE PHY, phy_addr:0x%x, 0x%x\n", phy_addr, regnum);
        return (rc);
    }

    /* step 2: Write Register 0x15 = 0x0000 */
    regnum = 0x15;
    rc = fugazi_bnxt_mdio_write(&fugazi_struct->bnxt[phy_num], phy_addr, devad, regnum, 0x0000);
    if (rc != PASSED) {
        printf("Failed to write GE PHY, phy_addr:0x%x, 0x%x\n", phy_addr, regnum);
        return (rc);
    }

    return (rc);
}

/*
 * Function: bcm54194_rdb_access_disable
 *
 * Description: Disable BCM54194 RDB access mode and enable Legacy Access mode.
 *  According to "BCM54194 register guide", Disable BCM54194 RDB access mode by
 *        Write Register 0x1E = 0x0087
 *        Write Register 0x1F = 0x8000
 * Hardware or software resets to the chip will enable RDB Access mode.
 *
 * Input: phy_num - PHY number
 *        phy_addr - PHY mdio address
 *
 * Return: PASSED/FAILED
 */
int bcm54194_rdb_access_disable (int phy_num, int phy_addr)
{
    int rc, regnum;
    
    /* Disable RDB access mode */
    /* step 1: Write Register 0x1E = 0x0087 */
    regnum = 0x1E;
    rc = fugazi_bnxt_mdio_write(&fugazi_struct->bnxt[phy_num], phy_addr, devad, regnum, 0x0087);
    if (rc != PASSED) {
        printf("Failed to write GE PHY, phy_addr:0x%x, 0x%x\n", phy_addr, regnum);
        return (rc);
    }

    /* step 2: Write Register 0x1F = 0x8000 */
    regnum = 0x1F;
    rc = fugazi_bnxt_mdio_write(&fugazi_struct->bnxt[phy_num], phy_addr, devad, regnum, 0x8000);
    if (rc != PASSED) {
        printf("Failed to write GE PHY, phy_addr:0x%x, 0x%x\n", phy_addr, regnum);
        return (rc);
    }
    rc = fugazi_bnxt_mdio_write(&fugazi_struct->bnxt[phy_num], phy_addr, devad, regnum, 0x8000);
    if (rc != PASSED) {
        printf("Failed to write GE PHY, phy_addr:0x%x, 0x%x\n", phy_addr, regnum);
        return (rc);
    }
    return (rc);

}

/*
 * Function: bcm54194_rdb_read
 *
 * Description: BCM54194 RDB register read.
 *   Fugazi MDIO access to 1G PHY is through BCM57412 MAC MDIO master with Clause 22.
 *   According to "BCM54194 register guide", read BCM54194 RDB register by
 *   Write Register 0x1E, bit[15:0] = 0xZZZZ, 0xZZZZ is register offset to be read
 *   Read Register 0x1F, data read back from 0xZZZZ.
 *
 * Input: phy_num - PHY number
 *        phy_addr - PHY mdio address
 *        rdb_offset - RDB register offset
 *        *reg_val - point to storage to store read value
 *         *
 * Return: PASSED/FAILED
 */
int bcm54194_rdb_read (int phy_num, int phy_addr, int rdb_offset, uint16_t *reg_val)
{
    int rc, regnum;
    
    /* Read the RDB register */
    /* step 1: write 'rdb_offset' to 0x1E */
    regnum = 0x1E;
    rc = fugazi_bnxt_mdio_write(&fugazi_struct->bnxt[phy_num], phy_addr, devad, regnum, rdb_offset);

    if (rc != PASSED) {
        printf("Failed to write GE PHY %d, phy_addr:0x%x, 0x%x\n", phy_num-2, phy_addr, regnum);
        return (rc);
    }

    /* step 2: read data from 0x1F, reg_val will contains value from reg 'rdb_offset' */
    regnum = 0x1F;
    rc = fugazi_bnxt_mdio_read(&fugazi_struct->bnxt[phy_num], phy_addr, devad, regnum, reg_val);
    if (rc < 0) {
        printf("Failed to read GE PHY %d, phy_addr:0x%x, 0x%x\n", phy_num-2, phy_addr, regnum);
        return (rc);
    }

    return (rc);
}

/*
 * Function: bcm54194_rdb_write
 *
 * Description: BCM54194 RDB register write.
 *   Fugazi MDIO access to 1G PHY is through BCM57412 MAC MDIO master with Clause 22.
 *   According to "BCM54194 register guide", write BCM54194 RDB register by
 *   Write Register 0x1E, bit[15:0] = 0xZZZZ, 0xZZZZ is register offset to be write
 *   Write Register 0x1F, bit[15:0], Bit[15:0] contains the desired bits to be written to 0xZZZZ.
 *
 * Input: phy_num - PHY number
 *        phy_addr - PHY mdio address
 *        rdb_offset - RDB register offset
 *        reg_val - write value
 *         *
 * Return: PASSED/FAILED
 */
int bcm54194_rdb_write (int phy_num, int phy_addr, int rdb_offset, uint16_t reg_val)
{
    int rc, regnum;

    /* Write the RDB register */
    /* step 1: write 'rdb_offset' to 0x1E */
    regnum = 0x1E;
    rc = fugazi_bnxt_mdio_write(&fugazi_struct->bnxt[phy_num], phy_addr, devad, regnum, rdb_offset);
    if (rc < 0) {
        printf("Failed to write GE PHY %d, phy_addr:0x%x, 0x%x\n", phy_num-2, phy_addr, regnum);
        return (rc);
    }

    /* step 2: write 'reg_val' value to 0x1F which will write 'reg_val' to reg. 'rdb_offset' */
    regnum = 0x1F;
    rc = fugazi_bnxt_mdio_write(&fugazi_struct->bnxt[phy_num], phy_addr, devad, regnum, reg_val);
    if (rc != PASSED) {
        printf("Failed to write GE PHY %d, phy_addr:0x%x, 0x%x\n", phy_num-2, phy_addr, regnum);
        return (rc);
    }

    return (rc);
}

/*
 * Function: bcm54194_switch_intf_access
 *
 * Description:
 * After reset, the Copper and Fiber registers space are accessible by default.
 * To switch from the Copper/Fiber register space to SGMII register space, the
 * following steps are required.
 * (All writes are to Port 0, but all ports are affected)
 *
 * 1. Switch from Copper/Fiber register space to SGMII register space:
 *     Write RDB 0x234, bits[6:5] = 2'b10
 *     Write RDB 0x234, bits[6:5] = 2'b01
 *     Write RDB 0x021, bits[0] = 1'b1 (Access SGMII Registers)
 *
 * 2. Switch from SGMII register space to Copper/Fiber register space:
 *     Write RDB 0x234, bits[6:5] = 2'b10
 *     Write RDB 0x234, bits[6:5] = 2'b00
 *     Write RDB 0x021, bits[0] = 1'b0 (Access Copper Registers)
 *                              = 1'b1 (Access Fiber Registers)
 *
 * Input: phy_num - PHY number
 *        intf - what interface going to switch (0: SMII; 1: Copper; 2: Fiber
 *
 * Return: PASSED/FAILED
 *
 */
int bcm54194_switch_intf_access (int phy_num, bcm54194_intf_t intf)
{
    uint32_t rc = FAILED;
    uint16_t reg_val, reg_val1, reg_val2, reg_val3;
    int rdb_offset;
    int phy_addr = ge_port_mapping_phy_addr_down[phy_num];

    rdb_offset = BCM54194_EXTERNAL_SERDES_CTRL_REG;
    rc = bcm54194_rdb_read(phy_num, phy_addr, rdb_offset, &reg_val1);
    if (rc < 0) {
        printf("Failed to read GE PHY %d, phy addr:%d, RDB offset:0x%x\n", phy_num-2, phy_addr, rdb_offset);
        return (rc);
    }
    reg_val1 &= ~(CU_FIBER_SGMII_REG_MASK);
    reg_val1 |= (CU_FIBER_SGMII_REG_SGMII_MODE); /* Set to SGMII mode */
    rc = bcm54194_rdb_write(phy_num, phy_addr, rdb_offset, reg_val1);
    if (rc < 0) {
        printf("Failed to write GE PHY %d, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n", phy_num-2, phy_addr, rdb_offset, reg_val1);
        return (rc);
    }

    switch(intf)
    {
        case BCM54194_SGMII_INTF:
            reg_val2 = 0x20;
            reg_val3 = 0x1;
            break;

        case BCM54194_COPPER_INTF:
            reg_val2 = 0x00;
            reg_val3 = 0x0;
            break;

        case BCM54194_FIBER_INTF:
            reg_val2 = 0x00;
            reg_val3 = 0x1;
            break;
        default:
            break;
    }

    rdb_offset = BCM54194_EXTERNAL_SERDES_CTRL_REG;
    rc = bcm54194_rdb_read(phy_num, phy_addr, rdb_offset, &reg_val);
    if (rc < 0) {
        printf("Failed to read GE PHY %d, phy addr:%d, RDB offset:0x%x\n", phy_num-2, phy_addr, rdb_offset);
        return (rc);
    }
    reg_val &= ~(CU_FIBER_SGMII_REG_MASK);
    reg_val |= reg_val2;
    rc = bcm54194_rdb_write(phy_num, phy_addr, rdb_offset, reg_val);
    if (rc < 0) {
    	printf("Failed to write GE PHY %d, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n", phy_num-2, phy_addr, rdb_offset, reg_val);
        return (rc);
    }

    rdb_offset = BCM54194_MODE_CTRL_REG;
    rc = bcm54194_rdb_read(phy_num, phy_addr, rdb_offset, &reg_val);
    if (rc < 0) {
        printf("Failed to read GE PHY %d, phy addr:%d, RDB offset:0x%x\n", phy_num-2, phy_addr, rdb_offset);
        return (rc);
    }
    reg_val &= ~(BCM54194_REG_1000X_EN_BIT);
    reg_val |= reg_val3;
    rc = bcm54194_rdb_write(phy_num, phy_addr, rdb_offset, reg_val);
    if (rc < 0) {
    	printf("Failed to write GE PHY %d, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n", phy_num-2, phy_addr, rdb_offset, reg_val);
        return (rc);
    }

    /* Add some dealy for the access switching */
    msleep(BCM54194_INTF_ACCESS_SWITCH_DELAY);
    return rc;
}

/*
 * Function: bcm54194_per_port_reset
 *
 * Description: This reset will set the PHY registers listed below to the
 * default values and hardware strap balls that are labeled sample on reset(SOR)
 * are relatched.
 * IEEE Registers (0x00 to 0x0F)
 * Per-Port RDB Registers (RDB_Reg. 0x00 to offset 0x2FF)
 * Input: phy_addr - PHY mdio address
 *        intf - what side of interface to reset (0: SMII; 1: Copper; 2: Fiber
 *         *
 * Return: PASSED/FAILED
 */
static int bcm54194_per_port_reset (int phy_addr, int intf)
{
    ushort reg_val;
    int rc, regnum = BCM54194_CTRL_REG;
    int phy_num = FUGAZI_MAC_1G_PHY_0;

    /* Switch to SGMII/Copper/Fiber register space */
    if (intf == BCM54194_FIBER_INTF) {
    	bcm54194_reg_1000x_en(phy_num, phy_addr, TRUE);
    } else {
        bcm54194_reg_1000x_en(phy_num, phy_addr, FALSE);
    }

    /* Reset PHY port */
    rc = fugazi_bnxt_mdio_read(&fugazi_struct->bnxt[phy_num], phy_addr, devad, regnum, &reg_val);
    if (rc < 0) {
        printf("Failed to read GE PHY %d, phy_addr:0x%x, 0x%x\n", phy_num-2, phy_addr, regnum);
        return (FAILED);
    }
    reg_val |= BCM54194_RESET_BIT;
    rc = fugazi_bnxt_mdio_write(&fugazi_struct->bnxt[phy_num], phy_addr, devad, regnum, reg_val);
    if (rc < 0) {
        printf("Failed to write GE PHY %d, phy_addr:0x%x, 0x%x\n", phy_num-2, phy_addr, regnum);
    }
    return (rc);
}

/*
 * Function: bcm54194_global_rdb_reset
 *
 * Description: Global RDB Register reset will reset the Global RDB Registers
 * (RDB_Reg 0x800 to offset 0xAFF) to their default values.
 * This needs to be done to Port 0's PHY address.
 *
 * Input: none
 *
 * Return: PASSED/FAILED
 */
static int bcm54194_global_rdb_reset (void)
{
    int rc;
    ushort reg_val;
    int phy_addr = PHY_PORT0;
    int phy_num = FUGAZI_MAC_1G_PHY_0;
    int rdb_offset = BCM54194_TOP_MISC_TOP_GBL_RST_REG;

    rc = bcm54194_rdb_read(phy_num, phy_addr, rdb_offset, &reg_val);
    if (rc < 0) {
        printf("Failed to read GE PHY %d, phy addr:%d, RDB offset:0x%x\n", phy_num-2, phy_addr, rdb_offset);
        return (rc);
    }
    reg_val |= BCM54194_TOP_MII_REG_SOFT_RST_BIT;
    rc = bcm54194_rdb_write(phy_num, phy_addr, rdb_offset, reg_val);
    if (rc < 0) {
    	printf("Failed to write GE PHY %d, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n", phy_num-2, phy_addr, rdb_offset, reg_val);
        return (rc);
    }

    return (rc);
}

/*
 * Function: bcm54194_clause45_reset
 *
 * Description: Clause 45 Register reset will reset the EEE block and registers
 * to their default values.
 * To enable the reset, set Clause 45 DEVAD 0x1, Address 0x0, bit[15] = 1'b0
 * This needs to be done to Port 0's PHY address.
 *
 * Input: none
 *
 * Return: none
 */
static int bcm54194_clause45_reset (void)
{
    ushort mii_value; 
    int dev_id = 0x1;
    int rc, regnum;
    int phy_addr = PHY_PORT0;
    int phy_num = FUGAZI_MAC_1G_PHY_0;

    regnum = BCM54194_CTRL_REG;
    rc = fugazi_bnxt_mdio_read(&fugazi_struct->bnxt[phy_num], phy_addr, dev_id, regnum, &mii_value);
    if (rc < 0) {
        printf("Read error from phy %d 0x%x\n", phy_num-2, phy_addr);
        return (FAILED);
    }
    /* perform 'software reset' to IEEE register 0 */
    mii_value |= (BCM54194_RESET_BIT);
    rc = fugazi_bnxt_mdio_write(&fugazi_struct->bnxt[phy_num], phy_addr, dev_id, regnum, mii_value);
    if (rc == -1) {
        printf("Write error from phy %d 0x%x\n", phy_num-2, phy_addr);
        return FAILED;
    }
    return (rc);
}

/*
 * Function: bcm54194_soft_reset
 *
 * Description: This function reset all set of registers
 * The BCM54194 has multiple reset bits as follows:
 * 1. Per-Port Register Reset
 * 2. Global RDB Register Reset
 * 3. Clause 45 Register Reset
 *
 * No one use this function so far.
 *
 * Input: none
 *
 * Return: PASSED/FAILED
 */
int bcm54194_soft_reset (void)
{
    int phy_addr;
    int phy_num, port;
    int intf = BCM54194_SGMII_INTF;

    for (phy_num = FUGAZI_MAC_1G_PHY_0; phy_num < (FUGAZI_MAC_1G_PHY_3+1); phy_num++)
    {
        for (port = phy_num*2; port < (phy_num*2)+2; port++)
        {
            phy_addr = ge_port_mapping_phy_addr_up[port];
            for (intf = BCM54194_SGMII_INTF; intf < BCM54194_FIBER_INTF; intf++) {
                if (bcm54194_per_port_reset(phy_addr, intf)) {
                    printf("BCM54194 Per-Port reset failed.\n");
                    printf("BCM54194 Per-Port reset failed at GE PHY %d, port %d\n", phy_num-2, port);
                    return (FAILED);
                }
            }
	    }
    }

    if (bcm54194_global_rdb_reset()) {
    	printf("BCM54194 Global RDB register reset failed.\n");
    	return (FAILED);
    }

    if (bcm54194_clause45_reset()) {
    	printf("BCM54194 Clause45 register reset failed.\n");
    	return (FAILED);
    }

    return (PASSED);
}

/*
 * Function: bcm54194_init_script
 *
 * Description: Initialize all the six BCM54194 1G PHY on Fugazi.
 *
 * Input: none
 *
 * Return: none
 */
void bcm54194_init_script (void)
{
    int rdb_reg;
    uint16_t reg_val;
    int phy_num, port;
    int phy_addr, phy_addr_up;

    /* Workaround for MDIO address issue on BCM54194 B0 silicon.
     * Avoiding touch RDB_reg 0x234 to switch register space.
     * 
     * SGMII/Copper:
     * GPHY BA+0 to BA+3, SGMII SerDes BA+4 to BA+7, MACsec BA+9
     * 
     * SGMII/Fiber:
     * Fiber SerDes BA+0 to BA+3, SGMII SerDes BA+4 to BA+7, MACsec BA+9
     */
    for (port = FUGAZI_1G_eth_4; port < MAX_FUGAZI_1G_ETH; port ++)
    {
        phy_num  = (int) (port/2);
        phy_addr = ge_port_mapping_phy_addr_down[port];

        bcm54194_rdb_access_disable(phy_num, phy_addr);
        fugazi_bnxt_mdio_write(&fugazi_struct->bnxt[phy_num], phy_addr, devad, 0x17, 0x0D19);
        fugazi_bnxt_mdio_read(&fugazi_struct->bnxt[phy_num], phy_addr, devad, 0x15, &reg_val);
        reg_val |= (0x1 << 3);
        fugazi_bnxt_mdio_write(&fugazi_struct->bnxt[phy_num], phy_addr, devad, 0x17, 0x0D19);
        fugazi_bnxt_mdio_write(&fugazi_struct->bnxt[phy_num], phy_addr, devad, 0x15, reg_val);
        bcm54194_rdb_access_enable(phy_num, phy_addr);

        /* Reset the PHY by writing to the PHY Reset register in the FPGA 
         * The reset is required on A0 silicon, but not on B0 silicon
         * It appears to be harmless on B0, so go ahead and do it for all systems
         */
        fugazi_bnxt_mdio_write(&fugazi_struct->bnxt[phy_num], phy_addr, devad, 0x17, 0x0D19);
        fugazi_bnxt_mdio_write(&fugazi_struct->bnxt[phy_num], phy_addr, devad, 0x15, 0x4189);
        fugazi_bnxt_mdio_write(&fugazi_struct->bnxt[phy_num], phy_addr, devad, 0x17, 0x0D19);
        fugazi_bnxt_mdio_write(&fugazi_struct->bnxt[phy_num], phy_addr, devad, 0x15, 0xC189);

        phy_addr_up = ge_port_mapping_phy_addr_up[port];
        rdb_reg = BCM54194_MODE_CTRL_REG; /* 0x21 */
        bcm54194_rdb_write(phy_num, phy_addr_up, rdb_reg, 0xFC01);

        /* Disable SGMII AN */
        fugazi_bnxt_mdio_write(&fugazi_struct->bnxt[phy_num], phy_addr_up, devad, 0x0, 0x0140);

        /* Set RDB 0x22D.4 to 0 for SGMII auto-negotiation to function property. */
        rdb_reg = 0x22D;
        bcm54194_rdb_read(phy_num, phy_addr_up, rdb_reg, &reg_val);
        reg_val &= ~(0x1 << 4);
        bcm54194_rdb_write(phy_num, phy_addr_up, rdb_reg, reg_val);

        /* Disable SUPER_ISOLATE bit. 
         * To make 1GE PHY copper interface in normal operation. */
        rdb_reg = BCM54194_COPPER_POWER_MII_CTRL_REG;   /* 0x2A */
        bcm54194_rdb_read(phy_num, phy_addr, rdb_reg, &reg_val);
        reg_val &= ~(BCM54194_SUPPER_ISOLATE);
        bcm54194_rdb_write(phy_num, phy_addr, rdb_reg, reg_val);
    }
}

/*
 * Function: bcm54194_reset
 *
 * Description: Reset/unreset BCM54194 1G PHY from FPGA, and
 *              Initialize all the BCM54194 1G PHY on Fugazi.
 *
 * Input: print_msg (1: Print Reset msg)
 *
 * Return: none
 */
void bcm54194_reset (int print_msg)
{
    /* Reset BCM54194 GE PHY by FPGA bit_13 at FPGA version v0.9.1 */
    /* Reset BCM54194 GE PHY by FPGA bit_0 at latest FPGA version */
    if (print_msg) {
        printf("Reset all BCM54194 PHYs...\n");
    }
    reset_platform_ext_dev(FPGA_EXT_GE_RST_1G);
 
    msleep(20);
 
    if (print_msg) {
        printf("Unreset all BCM54194 PHYs...\n");
    }
    unreset_platform_ext_dev(FPGA_EXT_GE_RST_1G);
    msleep(10);

    if (print_msg) {
        printf("Initialize BCM54194 PHYs...\n");
    }

    /* Initialize all the BCM54194 1G PHY */
    bcm54194_init_script();
    msleep(100);
}


/*
 * Function: bcm54194_reg_1000x_en
 *
 * Description: Access 1000BASE-T/SGMII register or Copper register
 * RDB_Reg 0x021 bit[0] = 1 (1000BASE-T register space selected)
 * RDB_Reg 0x021 bit[0] = 0 (Copper register space selected)
 * Input: phy_addr
 *
 * Need to select Copper register space before bring eth up
 *
 * Input: phy_num - PHY number
 *        phy_addr - PHY mdio address
 *        enable - Select 1: 1000BASE-T register space; 0: Copper register space
 * Return: PASSED/FAILED
 */
int bcm54194_reg_1000x_en (int phy_num, int phy_addr, int enable)
{
    uint16_t reg_val;
    int rc, rdb_offset;

	/* Select 1000BASE-T register space. */
	rdb_offset = BCM54194_MODE_CTRL_REG;
    rc = bcm54194_rdb_read(phy_num, phy_addr, rdb_offset, &reg_val);
    if (rc != PASSED) {
        printf("Failed to read GE PHY %d, phy addr:%d, RDB offset:0x%x\n", phy_num-2, phy_addr, rdb_offset);
        return (rc);
    }

    if (enable) {
        reg_val |= BCM54194_REG_1000X_EN_BIT;
    } else {
        reg_val &= ~(BCM54194_REG_1000X_EN_BIT);
    }

    rc = bcm54194_rdb_write(phy_num, phy_addr, rdb_offset, reg_val);
    if (rc != PASSED) {
    	printf("Failed to write GE PHY %d, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n", phy_num-2, phy_addr, rdb_offset, reg_val);
        return (rc);
    }
    return (rc);
}

/*
 * Function: bcm54194_sgmii_slave_mode
 *
 * Description: Config BCM54194 as SGMII-Slave mode.
 *              The mode is used when connecting to a 
 *              SFP SGMII-to Copper Transceiver
 *              (10/100/1000BASE-T) module.
 *
 * Input: phy_num - PHY number
 *        phy_addr - PHY mdio address
 *        enable - 1: enable SGMII-Slave mode; 0 disable SGMII-Slave mode
 * Return: PASSED/FAILED
 */
int bcm54194_sgmii_slave_mode (int phy_num, int phy_addr, int enable)
{
    uint16_t reg_val;
    int rc, rdb_offset, regnum;

    if (enable) {
        /* Disable Copper/Fiber Auto-detection */
        rdb_offset = BCM54194_AUTO_DETECT_MEDIUM_REG;
        rc = bcm54194_rdb_read(phy_num, phy_addr, rdb_offset, &reg_val);
        if (rc != PASSED) {
            printf("Failed to read GE PHY %d, phy addr:%d, RDB offset:0x%x\n",
                    phy_num-2, phy_addr, rdb_offset);
            return (rc);
        }
        reg_val &= ~(BCM54194_AUTO_DET_MEDIUM_EN_BIT);
        rc = bcm54194_rdb_write(phy_num, phy_addr, rdb_offset, reg_val);
        if (rc != PASSED) {
    	    printf("Failed to write GE PHY %d, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n",
                    phy_num-2, phy_addr, rdb_offset, reg_val);
            return (rc);
        }

        /* Enable Fiber mode and access Copper register */
        rdb_offset = BCM54194_MODE_CTRL_REG;
        rc = bcm54194_rdb_read(phy_num, phy_addr, rdb_offset, &reg_val);
        if (rc != PASSED) {
            printf("Failed to read GE PHY %d, phy addr:%d, RDB offset:0x%x\n", phy_num-2, phy_addr, rdb_offset);
            return (rc);
        }
        reg_val &= ~(BCM54194_MODE_SEL_MASK | BCM54194_REG_1000X_EN_BIT);
        reg_val |= BCM54194_SGMII_TO_FIBER_MODE;
        rc = bcm54194_rdb_write(phy_num, phy_addr, rdb_offset, reg_val);
        if (rc != PASSED) {
    	    printf("Failed to write GE PHY %d, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n", phy_num-2, phy_addr, rdb_offset, reg_val);
            return (rc);
        }

        /* Power down the Copper interface */
        regnum = BCM54194_CTRL_REG;
        rc = fugazi_bnxt_mdio_read(&fugazi_struct->bnxt[phy_num], phy_addr, devad, regnum, &reg_val);
        if (rc < 0) {
            printf("Failed to read GE PHY %d, phy_addr:0x%x, reg:0x%x\n", phy_num-2, phy_addr, regnum);
            return (rc);
        }
        reg_val |= BCM54194_POWER_DOWN_BIT;
        rc = fugazi_bnxt_mdio_write(&fugazi_struct->bnxt[phy_num], phy_addr, devad, regnum, reg_val);
        if (rc < 0) {
            printf("Failed to write GE PHY %d, phy_addr:0x%x, 0x%x\n", phy_num-2, phy_addr, regnum);
            return (rc);
        }

        /* Enable SGMII-Slave mode */
        rdb_offset = BCM54194_SGMII_SLAVE_REG;  /* 0x235 */
        rc = bcm54194_rdb_read(phy_num, phy_addr, rdb_offset, &reg_val);
        if (rc != PASSED) {
            printf("Failed to read GE PHY %d, phy addr:%d, RDB offset:0x%x\n", phy_num-2, phy_addr, rdb_offset);
            return (rc);
        }
        reg_val |= BCM54194_SGMII_SLAVE_MODE_EN_BIT;
        rc = bcm54194_rdb_write(phy_num, phy_addr, rdb_offset, reg_val);
        if (rc != PASSED) {
    	    printf("Failed to write GE PHY %d, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n", phy_num-2, phy_addr, rdb_offset, reg_val);
            return (rc);
        }
    } else {
        /* Enable Copper/Fiber Auto-detection */
        rdb_offset = BCM54194_AUTO_DETECT_MEDIUM_REG;
        rc = bcm54194_rdb_read(phy_num, phy_addr, rdb_offset, &reg_val);
        if (rc != PASSED) {
            printf("Failed to read GE PHY %d, phy addr:%d, RDB offset:0x%x\n",
                    phy_num-2, phy_addr, rdb_offset);
            return (rc);
        }
        reg_val |= BCM54194_AUTO_DET_MEDIUM_EN_BIT;
        rc = bcm54194_rdb_write(phy_num, phy_addr, rdb_offset, reg_val);
        if (rc != PASSED) {
    	    printf("Failed to write GE PHY %d, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n",
                    phy_num-2, phy_addr, rdb_offset, reg_val);
            return (rc);
        }

        /* Enable Fiber mode and access Copper register */
        rdb_offset = BCM54194_MODE_CTRL_REG;
        rc = bcm54194_rdb_read(phy_num, phy_addr, rdb_offset, &reg_val);
        if (rc != PASSED) {
            printf("Failed to read GE PHY %d, phy addr:%d, RDB offset:0x%x\n", phy_num-2, phy_addr, rdb_offset);
            return (rc);
        }
        reg_val &= ~(BCM54194_MODE_SEL_MASK | BCM54194_REG_1000X_EN_BIT);
        reg_val |= BCM54194_SGMII_TO_FIBER_MODE;
        rc = bcm54194_rdb_write(phy_num, phy_addr, rdb_offset, reg_val);
        if (rc != PASSED) {
    	    printf("Failed to write GE PHY %d, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n", phy_num-2, phy_addr, rdb_offset, reg_val);
            return (rc);
        }

        /* Power up the Copper interface */
        regnum = BCM54194_CTRL_REG;
        rc = fugazi_bnxt_mdio_read(&fugazi_struct->bnxt[phy_num], phy_addr, devad, regnum, &reg_val);
        if (rc < 0) {
            printf("Failed to read GE PHY %d, phy_addr:0x%x, reg:0x%x\n", phy_num-2, phy_addr, regnum);
            return (rc);
        }
        reg_val &= ~(BCM54194_POWER_DOWN_BIT);
        rc = fugazi_bnxt_mdio_write(&fugazi_struct->bnxt[phy_num], phy_addr, devad, regnum, reg_val);
        if (rc < 0) {
            printf("Failed to write GE PHY %d, phy_addr:0x%x, 0x%x\n", phy_num-2, phy_addr, regnum);
            return (rc);
        }

        /* Disable SGMII-Slave mode */
        rdb_offset = BCM54194_SGMII_SLAVE_REG;
        rc = bcm54194_rdb_read(phy_num, phy_addr, rdb_offset, &reg_val);
        if (rc != PASSED) {
            printf("Failed to read GE PHY %d, phy addr:%d, RDB offset:0x%x\n", phy_num-2, phy_addr, rdb_offset);
            return (rc);
        }
        reg_val &= ~(BCM54194_SGMII_SLAVE_MODE_EN_BIT);
        rc = bcm54194_rdb_write(phy_num, phy_addr, rdb_offset, reg_val);
        if (rc != PASSED) {
    	    printf("Failed to write GE PHY %d, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n", phy_num-2, phy_addr, rdb_offset, reg_val);
            return (rc);
        }
    }

    return (rc);
}

/*
 * Function: dump_bcm54194_loopback_config
 *
 * Description: dump registers value according to loopback mode.
 *
 * Input: phy_num - PHY number
 *        phy_addr - PHY mdio address
 *        loopback_mode - 1: PHY internal; 2 with ext loopback plug
 * Return: PASSED/FAILED
 */
int dump_bcm54194_loopback_config (int phy_num, int phy_addr, int loopback_mode)
{
    uint16_t reg_val;
    int rc, rdb_offset, regnum;

    switch(loopback_mode)
    {
        case GE_PHY_INT_LPBK:
            /* Enable loopback mode. */
            rdb_offset = BCM54194_COPPER_AUXILIARY_CTRL_REG;
            rc = bcm54194_rdb_read(phy_num, phy_addr, rdb_offset, &reg_val);
            if (rc != PASSED) {
                printf("Failed to write GE PHY %d, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n", phy_num-2, phy_addr, rdb_offset, reg_val);
                return (rc);
            }
            printf("rdb_reg %#.2x = %#.4x\n", rdb_offset, reg_val);

            /* Enable loopback mode without loopback plug. */

            /* Read RDB register 0x2C */
            rdb_offset = BCM54194_COPPER_MISCEL_TEST_REG;
            rc = bcm54194_rdb_read(phy_num, phy_addr, rdb_offset, &reg_val);
            if (rc != PASSED) {
                printf("Failed to write GE PHY %d, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n", phy_num-2, phy_addr, rdb_offset, reg_val);
                return (rc);
            }
            printf("rdb_reg %#.2x = %#.4x\n", rdb_offset, reg_val);
            
            regnum = BCM54195_1000BASE_CTRL_REG;
            fugazi_bnxt_mdio_read(&fugazi_struct->bnxt[phy_num], phy_addr, devad, regnum, &reg_val);
            printf("reg %#.2x = %#.4x\n", regnum, reg_val);

            regnum = BCM54194_CTRL_REG;
            fugazi_bnxt_mdio_read(&fugazi_struct->bnxt[phy_num], phy_addr, devad, regnum, &reg_val);
            printf("reg %#.2x = %#.4x\n", regnum, reg_val);
            
            rdb_offset = BCM54194_AUTO_DETECT_MEDIUM_REG;
            rc = bcm54194_rdb_read(phy_num, phy_addr, rdb_offset, &reg_val);
            if (rc != PASSED) {
                printf("Failed to write GE PHY %d, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n", phy_num-2, phy_addr, rdb_offset, reg_val);
                return (rc);
            }
            printf("rdb_reg %#.2x = %#.4x\n", rdb_offset, reg_val);
            break;

        case GE_PHY_EXT_LPBK:
        	/* Enable loopback mode with loopback plug. */
        	rdb_offset = BCM54194_COPPER_AUXILIARY_CTRL_REG;
            rc = bcm54194_rdb_read(phy_num, phy_addr, rdb_offset, &reg_val);
            if (rc != PASSED) {
                printf("Failed to write GE PHY %d, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n", phy_num-2, phy_addr, rdb_offset, reg_val);
                return (rc);
            }
            printf("rdb_reg %#.2x = %#.4x\n", rdb_offset, reg_val);

            regnum = BCM54195_1000BASE_CTRL_REG;
            fugazi_bnxt_mdio_read(&fugazi_struct->bnxt[phy_num], phy_addr, devad, regnum, &reg_val);
            printf("reg %#.2x = %#.4x\n", regnum, reg_val);

            regnum = BCM54194_CTRL_REG;
            fugazi_bnxt_mdio_read(&fugazi_struct->bnxt[phy_num], phy_addr, devad, regnum, &reg_val);
            printf("reg %#.2x = %#.4x\n", regnum, reg_val);
            
            rdb_offset = BCM54194_AUTO_DETECT_MEDIUM_REG;
             bcm54194_rdb_read(phy_num, phy_addr, rdb_offset, &reg_val);
            if (rc != PASSED) {
                printf("Failed to write GE PHY %d, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n", phy_num-2, phy_addr, rdb_offset, reg_val);
                return (rc);
            }
            printf("rdb_reg %#.2x = %#.4x\n", rdb_offset, reg_val);
            break;
    }

    return (rc);
}

/*
 * Function: bcm54194_config_loopback
 *
 * Description: Configurating BCM54194 in loopback mode.
 * Input: phy_addr, speed, interface, loopback mode, enable
 *
 * Input: phy_num - PHY number
 *        phy_addr - PHY mdio address
 *        speed - Interface speed (100MBPS Only for Fugazi)
 *        intf - loop back at what interface (0: SMII; 1: Copper; 2: Fiber
 *        loopback_mode - 1: PHY internal; 3: with ext loopback plug
 *        enable - 1: enable; 0: disable
 *
 * Return: PASSED/FAILED
 */
int bcm54194_config_loopback (int phy_num, int phy_addr, int speed, bcm54194_intf_t intf, int loopback_mode, int enable)
{
    uint16_t reg_val;
    int rc, rdb_offset, regnum, duplex = FULL_DUPLEX;

    switch(loopback_mode)
    {
        case GE_PHY_INT_LPBK:
                    /* configure for data loopback at PHY internal */
                    rdb_offset = BCM54194_MODE_CTRL_REG;
                    rc = bcm54194_rdb_read(phy_num, phy_addr, rdb_offset, &reg_val);
                    if (rc != PASSED) {
                        printf("Failed to read GE PHY %d, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n",                                                                       phy_num-2, phy_addr, rdb_offset, reg_val);
                        return (rc);
                    }
                    reg_val |= BCM54194_REG_1000X_EN_BIT;
                    rc = bcm54194_rdb_write(phy_num, phy_addr, rdb_offset, reg_val);
                    if (rc != PASSED) {
                    	printf("Failed to write GE PHY %d, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n",                                                                       phy_num-2, phy_addr, rdb_offset, reg_val);
                        return (rc);
                    }

                    if (speed == SPD_1000MBPS) {
                        regnum = BCM54194_CTRL_REG;
                        reg_val = (BCM54194_INTERNAL_LOOPBACK | BCM54194_DUPLEX_BIT |
                                   BCM54194_MSB_SPEED_SEL | BCM54194_CTRL_REG_RESERVED_BITS); /* 0x4140 */
                        rc = fugazi_bnxt_mdio_write(&fugazi_struct->bnxt[phy_num], phy_addr, devad, regnum, reg_val);
                        if (rc != PASSED) {
                            printf("Failed to write GE PHY %d, phy_addr:0x%x, 0x%x\n", phy_num-2, phy_addr, regnum);
                        }
                    }
            break;

        case GE_PHY_SFP_EXT_LPBK:
                /* configure for data loopback at PHY external loopback */
                if (enable) {
                    /* Select 1000BASE-T register space and set speed
                     * to make SFP link up.
                     */
                    rc = bcm54194_cfg_setting(phy_num, phy_addr, speed, AUTONEG_OFF, duplex, intf);
                    if (rc != PASSED) {
                        printf("Failed to config PHY %d setting. phy addr:0x%x\n", phy_num-2, phy_addr);
                        return (rc);
                    }
                } else {
                    /* Restore to Copper register space. */
                    bcm54194_reg_1000x_en(phy_num, phy_addr, enable);
                }
            break;

        default:
            /* Clear the loopback */
            printf("BCM INFO - None loopback\n");

            break;
    }

    return (rc);
}

/*
 * Function: bcm54194_cfg_setting
 *
 * Description: BCM54194 control register setting.
 * Input: phy_addr, speed, duplex, interface
 *
 * Input: phy_num - PHY number
 *        phy_addr - PHY mdio address
 *        speed - Interface speed (100MBPS Only for Fugazi)
 *        auto_neg - 1: enable auto-nego; 0: disable auto-nego
 *        duplex- 1: full duplex; 0: half duplex
 *        intf - what interface (0: SMII; 1: Copper; 2: Fiber
 *
 * Return: PASSED/FAILED
 */
int bcm54194_cfg_setting (int phy_num, int phy_addr, int speed, int auto_neg, int duplex, bcm54194_intf_t intf)
{
    int rc, regnum = BCM54194_CTRL_REG;
    ushort reg_val;

    /* Switch to SGMII/Copper/Fiber register space */
    if (intf == BCM54194_FIBER_INTF) {
        bcm54194_reg_1000x_en(phy_num, phy_addr, TRUE);
    } else {
        bcm54194_reg_1000x_en(phy_num, phy_addr, FALSE);
    }

    rc = fugazi_bnxt_mdio_read(&fugazi_struct->bnxt[phy_num], phy_addr, devad, regnum, &reg_val);
    if (rc < 0) {
        printf("Failed to read GE PHY %d, phy_addr:0x%x, 0x%x\n", phy_num-2, phy_addr, regnum);
        return (FAILED);
    }

    reg_val &= ~(BCM54194_SPEED_MASK);
    switch(speed)
    {
        case SPD_1000MBPS:
            reg_val |= BCM54194_SPEED_1000MBPS;
            break;
        case SPD_100MBPS:
            reg_val |= BCM54194_SPEED_100MBPS;
            break;
        case SPD_10MBPS:
            reg_val |= BCM54194_SPEED_10MBPS;
            break;
        default:
            reg_val |= BCM54194_SPEED_1000MBPS;
            break;
    }

    if (duplex) {
        reg_val |= BCM54194_DUPLEX_BIT;
    } else {
        reg_val &= ~(BCM54194_DUPLEX_BIT);
    }

    if (auto_neg) {
        reg_val |= BCM54194_AN_ENABLE;
    } else {
        reg_val &= ~(BCM54194_AN_ENABLE);
    }

    rc = fugazi_bnxt_mdio_write(&fugazi_struct->bnxt[phy_num], phy_addr, devad, regnum, reg_val);
    if (rc < 0) {
        printf("Failed to write GE PHY %d, phy_addr:0x%x, 0x%x\n", phy_num-2, phy_addr, regnum);
    }

    return rc;
}

/**********************************************************************
 *
 * Function: bcm54194_is_linkup
 *
 * Description:
 * Check if the BCM54194 SGMII/Copper/Fiber interface link status is up
 *
 * Input: phy_num - PHY number
 *        phy_addr - PHY mdio address
 *        intf - what interface (0: SMII; 1: Copper; 2: Fiber
 *
 * Return: PASSED/FAILED
 */
boolean bcm54194_is_linkup (int phy_num, int phy_addr, bcm54194_intf_t intf)
{
    int regnum = BCM54194_STAT_REG; /* IEEE reg. 0x01 */
    int repeat = 1000;
    ushort reg_val;
    int rc;

    /* Switch to SGMII/Copper/Fiber register space */
    if (intf == BCM54194_FIBER_INTF) {
        bcm54194_reg_1000x_en(phy_num, phy_addr, TRUE);
    } else {
        bcm54194_reg_1000x_en(phy_num, phy_addr, FALSE);
    }

    /* max timeout waiting for Link up 100 sec */
    do {
        msleep(10);
        rc = fugazi_bnxt_mdio_read(&fugazi_struct->bnxt[phy_num], phy_addr, devad, regnum, &reg_val);
        if (reg_val & BCM54194_LINK_STAT_BIT) {
            return (TRUE);
        }
        printf(".");
        fflush(stdout);
    } while (repeat-- > 0);
    printf("\n");
    if (!(reg_val & BCM54194_LINK_STAT_BIT)) {  /* check if 0x04 */
        printf("No link at system side!\n");
    }

    return (FALSE);
}

/*
 * Function: bcm54194_sig_pwr_ctrl
 *
 * Description: Configurate BCM54194 in Power Down mode.
 * Input: phy_addr, speed, interface, loopback mode, enable
 *
 * Input: phy_num - PHY number
 *        phy_addr - PHY mdio address
 *        enable - 1: in Normal operation; 0: in low-power standby mode
 *        intf - at what interface (0: SMII; 1: Copper; 2: Fiber
 *
 * Return: PASSED/FAILED
 */
int bcm54194_sig_pwr_ctrl(int phy_num, int phy_addr, boolean enable, bcm54194_intf_t intf)
{
    int rc, regnum = BCM54194_CTRL_REG;
    ushort reg_val;

    /* Switch to SGMII/Copper/Fiber register space */
    if (intf == BCM54194_FIBER_INTF) {
        bcm54194_reg_1000x_en(phy_num, phy_addr, TRUE);
    } else {
        bcm54194_reg_1000x_en(phy_num, phy_addr, FALSE);
    }

    rc = fugazi_bnxt_mdio_read(&fugazi_struct->bnxt[phy_num], phy_addr, devad, regnum, &reg_val);
    if (rc < 0) {
        printf("Failed to read GE PHY %d, phy_addr:0x%x, 0x%x\n", phy_num-2, phy_addr, regnum);
        return (FAILED);
    }

    if (enable) {
        /* configure PHY in normal operation */
        reg_val &= ~(BCM54194_POWER_DOWN_BIT);
    } else {
        /* configure PHY in lowe-power standby mode */
        reg_val |= BCM54194_POWER_DOWN_BIT;
    }

    rc = fugazi_bnxt_mdio_write(&fugazi_struct->bnxt[phy_num], phy_addr, devad, regnum, reg_val);
    if (rc < 0) {
        printf("Failed to write GE PHY %d, phy_addr:0x%x, 0x%x\n", phy_num-2, phy_addr, regnum);
    }

    return (rc);
}

/*
 * Function: bcm54194_mdio45_reg_rd
 *
 * Description:
 * Read Broadcom 54194 PHY register. Use Cavium MDIO bus access directly.
 * Note: not used on Fugazi. Fugazi MDIO access to PHY is through BCM57412 MAC.
 * 
 * Input:
 * port - The MII phy id
 * dev - MMD
 * reg - Register to read
 *
 * Return: read_value/FAILED
 */
int bcm54194_mdio45_reg_rd (int bus_id, int phy_addr, int dev, int reg)
{
    uint16_t mii_value = 0x0;
    int rc;
    int phy_num = FUGAZI_MAC_1G_PHY_0;

    rc = fugazi_bnxt_mdio_read(&fugazi_struct->bnxt[phy_num], phy_addr, dev, reg, &mii_value);
    if (rc == -1) {
        printf("Read error from phy %d addr %d reg %d.%x)\n", phy_num-2, phy_addr, dev, reg);
        return FAILED;
    } else {
#ifdef DEBUG
        printf("phy %d addr %#.2x reg %d.%#.4x = %#.4x\n", phy_num-2, phy_addr, dev, reg, mii_value);
#endif
    }
    return (mii_value);
}

/*
 * Function: bcm54194_mdio45_reg_wr
 *
 * Description:
 * Write Broadcom 54194 PHY register. Use Cavium MDIO bus access directly.
 * Note: not used on Fugazi. Fugazi MDIO access to PHY is through BCM57412 MAC.
 * 
 * Input:
 * port - The MII phy id
 * dev - MMD
 * reg - Register to write
 * val - value to write
 *
 * Return: PASSED/FAILED
 */
int bcm54194_mdio45_reg_wr (int bus_id, int phy_addr, int dev, int reg, int val)
{
    int status; 
    int phy_num = FUGAZI_MAC_1G_PHY_0;

    status = fugazi_bnxt_mdio_write(&fugazi_struct->bnxt[phy_num], phy_addr, dev, reg, val);
    if (status == -1) {
        printf("Write error from phy %d addr %d reg %d.%x)\n", phy_num-2, phy_addr, dev, reg);
        return (FAILED);
    } else {
#ifdef DEBUG
        printf("phy %d addr %#.2x reg %d.%#.4x = 0x%04%x\n", phy_num-2, phy_addr, dev, reg, mii_value);
#endif
    }
    return (PASSED);
}

/*
 * Function: check_link
 *
 * Description:
 * Dump the status of system and network side of each port.
 * 
 * Input: None
 *
 * Return: None
 */
void check_link (void)
{
    ushort rdb_rdval;
    ushort ieee_rdval ;
    int phy_addr;
    int phy_num, port;
    int rdb_offset;

    printf("\n- Link Status -");
    printf("\nNetwork - downstream port (Check the Fiber link)");
    printf("\nSystem - upstream port(Check the SGMII Link)");
    printf("\n------------------------------------------------------");

    for (phy_num = FUGAZI_MAC_1G_PHY_0; phy_num < (FUGAZI_MAC_1G_PHY_3+1); phy_num++)
    {
        for (port = phy_num*2; port < (phy_num*2)+2; port++)
        {
            printf("\nETH %d : ", port);
            phy_addr = ge_port_mapping_phy_addr_down[port];
            rdb_offset = BCM54194_MODE_CTRL_REG;
            bcm54194_rdb_read(phy_num, phy_addr, rdb_offset, &rdb_rdval);
            bcm54194_rdb_read(phy_num, phy_addr, rdb_offset, &rdb_rdval);
            /* Checking if Fiber link up */
            if (rdb_rdval & BCM54194_SERDES_LINK_UP)
            {
                printf("SFP%d - Network : UP (RDB reg addr:0x%X = %x) //", port-2, rdb_offset, rdb_rdval);
            } else
            {
                printf("SFP%d - Network : DOWN (RDB reg addr:0x%X = %x) //",  port-2, rdb_offset, rdb_rdval);
            }
    
            phy_addr = ge_port_mapping_phy_addr_up[port];
            fugazi_bnxt_mdio_read(&fugazi_struct->bnxt[phy_num], phy_addr, devad, 1, &ieee_rdval);
            fugazi_bnxt_mdio_read(&fugazi_struct->bnxt[phy_num], phy_addr, devad, 1, &ieee_rdval);
            if (ieee_rdval & 0x4)
            {
                printf(" System - UP (reg addr:0x1 = %x)", ieee_rdval);
            } else
            {
                printf(" System - DOWN (reg addr:0x1 = %x)", ieee_rdval);
            }
        }
    }   
}


/*
 * Function: set_line_side_config
 *
 * Description: Enable 1000BASE-x Line-Side Loopback
 * RDB_Reg 0x023E = 0x78E0 (Disable Copper/Fiber Auto Switching)
 * RDB_Reg 0x021  = 0x7C30 (Enable 1000BASE-X mode and 1000BASE-X Register space)
 * RDB_Reg 0x02C bit[15] = 1'b1 (1000BASE-X Line-Side Loopback Enable)
 * IEEE_ Reg 0x0 bit[9]  = 1'b1 (Restart 1000BASE-X autonegotiation)
 *
 * Input: phy_num - PHY number
 *        port - eth port number, start from 4 (eth4) for 1G PHY
 * Return: PASSED/FAILED
 */
int set_line_side_config (int phy_num, int port)
{
    uint16_t reg_val=0, ieee_rdval;
    int rc, rdb_offset;
    int phy_addr;

    phy_addr = ge_port_mapping_phy_addr_down[port];
    /* Disable Copper/Fiber Auto Switching */
    rdb_offset = BCM54194_AUTO_DETECT_MEDIUM_REG;
    reg_val = (BCM54194_AUXILIARY_100X_SEL | BCM54194_FIBER_IN_USE_LED |
               BCM54194_FIBER_LED | BCM54194_FIBER_SD_SYNC_STATUS);        /* 0x78E0 */
    rc = bcm54194_rdb_write(phy_num, phy_addr, rdb_offset, reg_val);
    if (rc != PASSED) {
        printf("Failed to write GE PHY %d, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n",                                                                                                              phy_num-2, phy_addr, rdb_offset, reg_val);
        return (rc);
    }
    
    /* Enable 1000BASE-X mode and 1000BASE-X Register spac */
    rdb_offset = BCM54194_MODE_CTRL_REG;
    reg_val = 0x7C30;
    rc = bcm54194_rdb_write(phy_num, phy_addr, rdb_offset, reg_val);
    if (rc != PASSED) {
        printf("Failed to write GE PHY %d, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n",                                                                                                              phy_num-2, phy_addr, rdb_offset, reg_val);
        return (rc);
    }
    
    /* 1000BASE-X Line-Side Loopback Enable */
    rdb_offset = BCM54194_COPPER_MISCEL_TEST_REG;
    rc = bcm54194_rdb_read(phy_num, phy_addr, rdb_offset, &reg_val);
    if (rc != PASSED) {
        printf("Failed to read GE PHY %d, phy addr:%d, RDB offset:0x%x\n", phy_num-2, phy_addr, rdb_offset);
        return (rc);
    }
    
    reg_val |= BCM54194_RMT_LPBK_EN;  /* enable Remote line-side loopback */
    
    rc = bcm54194_rdb_write(phy_num, phy_addr, rdb_offset, reg_val);
    if (rc != PASSED) {
        printf("Failed to write GE PHY %d, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n",                                                                                                              phy_num-2, phy_addr, rdb_offset, reg_val);
        return (rc);
    }
    
    /* Restart 1000BASE-X autonegotiation */
    rdb_offset = BCM54194_CTRL_REG;
    rc = fugazi_bnxt_mdio_read(&fugazi_struct->bnxt[phy_num], phy_addr, devad, rdb_offset, &ieee_rdval);
    if (rc != PASSED) {
        printf("Failed to read GE PHY %d, phy addr:%d, RDB offset:0x%x\n", phy_num-2, phy_addr, rdb_offset);
        return (rc);
    }
    
    ieee_rdval |= BCM54194_RESTART_AN;
    
    rc = fugazi_bnxt_mdio_write(&fugazi_struct->bnxt[phy_num], phy_addr, devad, rdb_offset, ieee_rdval);
    if (rc != PASSED) {
        printf("Failed to write GE PHY %d, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n",                                                                                                           phy_num-2, phy_addr, rdb_offset, ieee_rdval);
        return (rc);
    }
    return (rc);
}



/*
 * Function: config_lpbk_mode
 *
 * Description: Enable SGMII/Line Side Loopback mode
 *              from Configure loopabck utility.
 *
 * Input: None
 * Return: None
 */
void config_lpbk_mode (void)
{
    int phy_addr, phy_num, portnum, test_mode;
    
    printf("\nPort number 0 - 7 = ETH4 - ETH11");
    portnum = gethex_answer("\nEnter eth num (0x4 - 0xB)", 4, 4, 11);
    printf("Clear all loopback : 0\n");
    printf("Host loopback : 1\n");
    printf("Line loopback : 2\n");
    test_mode = gethex_answer("\nEnter BCM54194PHY loopback mode", 0, 0, 2);
    phy_addr = ge_port_mapping_phy_addr_down[portnum];
    phy_num  = (int) (portnum/2);

    switch(test_mode)
    {
        case 0:
            bcm54194_reset(1);
            break;
        case 1:
            bcm54194_config_loopback(phy_num, phy_addr, SPD_1000MBPS, BCM54194_FIBER_INTF, GE_PHY_INT_LPBK, TRUE);
            break;
        case 2:
            set_line_side_config(phy_num, portnum);
            break;
        default:
            printf("None loopback\n");
            break;
    }
}


/*
 * Function: bcm54194_recover_clock
 *
 * Description:
 * Enable/disable PHY's recovered clock output REC_CLK1 to idt8a335004 for
 * "recover clock" test item by configure RDB register 0x83C.
 * Refer to datasheet 54194-DS105, section 2.7.1.
 *
 * Input: phy - 0 ~ 3 (1st .. last bcm54194 PHY on Fugazi board)
 *        enable - 1/0: enable/disable output REC_CLK1
 *
 * Return: None
 */
int bcm54194_recover_clock (int phy, int enable)
{
    int rc = PASSED;
    int phy_num, phy_addr;
    int rdb_offset = 0x83C;
    ushort rdb_val;

    if ( phy > GE_PORT3 ) {
        printf("ERROR:  %s:%d phy number %d exceed max phy number %d on Fugazi\n",
                      __FUNCTION__, __LINE__, phy, GE_PORT3);
        return FAILED;
    }

    phy_num  = ge_phy_mapping_phy_num[phy];
    phy_addr = ge_port_mapping_phy_addr_down[phy_num*2];

    rc |= bcm54194_rdb_read(phy_num, phy_addr, rdb_offset, &rdb_val);
    rc |= bcm54194_rdb_read(phy_num, phy_addr, rdb_offset, &rdb_val);
    if (diagflag_xram & D_DEBUG_OPTIONS) {
        printf("%s(): READ: phy_num %d, phy addr:0x%x, RDB offset:0x%x, value=%#.4x\n",
                      __FUNCTION__, phy_num, phy_addr, rdb_offset, rdb_val);
    }
    if (rc < 0) {
        printf("%s(): ERROR: Failed to read phy_num %d, phy addr:0x%x, RDB offset:0x%x\n",
                      __FUNCTION__, phy_num, phy_addr, rdb_offset);
        return (rc);
    }

    /* Disable recovered clock REC_CLK2 output */
    rdb_val |= 0x80;

    /* Enable Auto Clock mode. when link is lost the outputs on
     * REC_CLK[2]/REC__CLK[1] will be driven low. */
    rdb_val |= 0x100;

    /* Select recovered clock REC_CLK1 from PHY port 0 */
    rdb_val &= ~0x07;

    if (enable) {
        /* enable recovered clock REC_CLK1 output */
        rdb_val &= ~0x08;
    }
    else {
        /* disable recovered clock REC_CLK1 output */
        rdb_val |= 0x08;
    }
    if (diagflag_xram & D_DEBUG_OPTIONS) {
        printf("%s(): WRITE: phy_num %d, phy addr:0x%x, RDB offset:0x%x, value=%#.4x\n",
                        __FUNCTION__, phy_num, phy_addr, rdb_offset, rdb_val);
    }

    rc |= bcm54194_rdb_write(phy_num, phy_addr, rdb_offset, rdb_val);
    if (rc < 0) {
        printf("%s(): Failed to write phy_num %d, phy addr:0x%x, RDB offset:0x%x, value=%#.4x\n",
                        __FUNCTION__, phy_num, phy_addr, rdb_offset, rdb_val);
    }

    msleep(10); /* wait 10ms for device take action */

    return (rc);
}


/*
 * Function: bcm54194_config_prbs
 *
 * Description: Config BCM54194 PRBS from utility.
 *
 * Inputs      : NONE
 * Outputs     : PASSED / FAILED
 */
int bcm54194_config_prbs (void)
{
    ushort wr_val, reg_val;
    int phy_addr, phy_num, rdb_offset, portnum;
    int phy, action, poly, invert, rc;


    phy = getdec_answer("\nEnter phy num 0~3", 0, 0, 3);
    phy_num = ge_phy_mapping_phy_num[phy];
    
    printf("\nGE PHY port num are 0 - 1)\n");
    portnum = getdec_answer("\nEnter port num ", 0, 0, 1);
    phy_addr = ge_port_mapping_phy_addr_down[(phy_num*2) + portnum];

    action = getdec_answer("Enter Action(Check:0, Enable:1, Disable:2, Inject errors:3, Clear)",
                           0, 0, 3);

    switch (action) {
    default:
    case 0:
        rdb_offset = BCM54194_PRBS_STATUS_REG; /* 0x201 */
        rc = bcm54194_rdb_read(phy_num, phy_addr, rdb_offset, &reg_val);
        if (rc != PASSED) {
            printf("Failed to read GE PHY %d, phy addr:%d, RDB offset:0x%x\n", phy_num-2, phy_addr, rdb_offset);
            return (rc);
        }

        if (reg_val & PRBS_LOCKED)
        {
            printf("\nPRBS locked");
        } else
        {
            printf("\nPRBS not locked");
        }

        if (reg_val & PRBS_LOST_LOCK)
        {
            printf("\nPRBS lost lock since last read");
        } else
        {
            printf("\nPRBS has not lost lock since last read");
        }

        reg_val &= (PRBS_ERR_CNTR_MASK);  /* 0x07FF */
        printf("\nPRBS errors: %d", reg_val);
        break;

    case 1:
	    rdb_offset = BCM54194_PRBS_CTRL_REG; /* 0x200 */
        /* Enable PRBS */
        wr_val = PRBS_ENABLE;
        wr_val &= ~(CLR_PRBS_ERR_CNTR);
        
        /* PRBS Config */
        invert = getdec_answer("Invert: 1, Not invert: 0 ", 0, 0, 1);
        if (invert) {
            wr_val |= BCM54194_PRBS_INVERT;
        } else {
            wr_val &= ~(BCM54194_PRBS_INVERT);
        }

        poly = getdec_answer("PRBS Polynomial(7, 15, 23)", 23, 0, 23);
        switch (poly) {
        case 7:
            wr_val |= BCM54194_PRBS_7;
            break;
        case 15:
            wr_val |= BCM54194_PRBS_15;
            break;
        default:
        case 23:
            wr_val |= BCM54194_PRBS_23;
            break;
        }


        /* Clear PRBS error counter first */
        wr_val |= CLR_PRBS_ERR_CNTR;
        wr_val |= PRBS_ENABLE;
        rc = bcm54194_rdb_write(phy_num, phy_addr, rdb_offset, wr_val);
        if (rc != PASSED) {
            printf("Failed to write GE PHY %d, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n", phy_num-2, phy_addr, rdb_offset, reg_val);
            return (rc);
        }

        wr_val &= ~(CLR_PRBS_ERR_CNTR);
        rc = bcm54194_rdb_write(phy_num, phy_addr, rdb_offset, wr_val);
        if (rc != PASSED) {
            printf("Failed to write GE PHY %d, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n", phy_num-2, phy_addr, rdb_offset, reg_val);
            return (rc);
        }

        rc = bcm54194_rdb_read(phy_num, phy_addr, rdb_offset, &reg_val);
        if (rc != PASSED) {
            printf("Failed to read GE PHY %d, phy addr:%d, RDB offset:0x%x\n", phy_num-2, phy_addr, rdb_offset);
            return (rc);
        }
        break;
   
    case 2:
        rdb_offset = BCM54194_PRBS_CTRL_REG; /* 0x200 */
        rc = bcm54194_rdb_read(phy_num, phy_addr, rdb_offset, &reg_val);
        if (rc != PASSED) {
            printf("Failed to read GE PHY %d, phy addr:%d, RDB offset:0x%x\n", phy_num-2, phy_addr, rdb_offset);
            return (rc);
        }
        reg_val &= ~(PRBS_ENABLE);
        rc = bcm54194_rdb_write(phy_num, phy_addr, rdb_offset, reg_val);
        if (rc != PASSED) {
            printf("Failed to write GE PHY %d, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n", phy_num-2, phy_addr, rdb_offset, reg_val);
            return (rc);
        }
        break;
    
    case 3:
        /* to inject error, user must do Enable PRBS first */
	    rdb_offset = BCM54194_PRBS_CTRL_REG; /* 0x200 */
        rc = bcm54194_rdb_read(phy_num, phy_addr, rdb_offset, &reg_val);
        if (rc != PASSED) {
            printf("Failed to read GE PHY %d, phy addr:%d, RDB offset:0x%x\n", phy_num-2, phy_addr, rdb_offset);
            return (rc);
        }

        /* Enable PRBS21 and Star Inject error */
        reg_val |= TEST_PRBS_ERR_CNTR; 
        reg_val |= PRBS_ENABLE;

        rc = bcm54194_rdb_write(phy_num, phy_addr, rdb_offset, reg_val);
        if (rc != PASSED) {
            printf("Failed to write GE PHY %d, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n", phy_num-2, phy_addr, rdb_offset, reg_val);
            return (rc);
        }

        /* Enable PRBS21 and Stop Inject error */
        reg_val &= ~(TEST_PRBS_ERR_CNTR);

        rc = bcm54194_rdb_write(phy_num, phy_addr, rdb_offset, reg_val);
        if (rc != PASSED) {
            printf("Failed to write GE PHY %d, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n", phy_num-2, phy_addr, rdb_offset, reg_val);
            return (rc);
        }

        /* read error counter */
        rdb_offset = 0x201;
        rc = bcm54194_rdb_read(phy_num, phy_addr, rdb_offset, &reg_val);
        if (rc != PASSED) {
            printf("Failed to read GE PHY %d, phy addr:%d, RDB offset:0x%x\n", phy_num-2, phy_addr, rdb_offset);
            return (rc);
        }
        if (reg_val & PRBS_LOCKED)
        {
            printf("\nPRBS locked");
        } else
        {
            printf("\nPRBS not locked");
        }

        if (reg_val & PRBS_LOST_LOCK)
        {
            printf("\nPRBS lost lock since last read");
        } else
        {
            printf("\nPRBS has not lost lock since last read");
        }
        reg_val &= ~(0xf800);
        printf("\nPRBS errors: %d", reg_val);
        break;
    case 4:
    	/* clear error counter, user must do Enable PRBS first */
	    rdb_offset = BCM54194_PRBS_CTRL_REG; /* 0x200 */
        rc = bcm54194_rdb_read(phy_num, phy_addr, rdb_offset, &reg_val);
        if (rc != PASSED) {
            printf("Failed to read GE PHY %d, phy addr:%d, RDB offset:0x%x\n", phy_num-2, phy_addr, rdb_offset);
            return (rc);
        }

        /* Clear PRBS error counter */
        reg_val |= CLR_PRBS_ERR_CNTR;
        rc = bcm54194_rdb_write(phy_num, phy_addr, rdb_offset, reg_val);
        if (rc != PASSED) {
            printf("Failed to write GE PHY %d, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n", phy_num-2, phy_addr, rdb_offset, reg_val);
            return (rc);
        }

        /* back to Normal operation */
        reg_val &= ~CLR_PRBS_ERR_CNTR;
        rc = bcm54194_rdb_write(phy_num, phy_addr, rdb_offset, reg_val);
        if (rc != PASSED) {
            printf("Failed to write GE PHY %d, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n", phy_num-2, phy_addr, rdb_offset, reg_val);
            return (rc);
        }

        break;

    }

    return (PASSED);
}

 /*
  * Function: packet_counter_util
  *
  * Description:
  * Utility to show or modify packet counter of RDB register.
  *
  * Input: None
  *
  * Return: None
  */
void packet_counter_util (void)
{
    ushort rdb_up_rdval, rdb_down_rdval;
    int rdb_offset;
    int port;
    int action = getdec_answer("Enter Action(Enable RX packet counter:0, Enable TX packet counter:1, Show packet counter:2)",
                                0, 0, 2);

    for (port = FUGAZI_1G_eth_4; port < MAX_FUGAZI_1G_ETH; port++)
    {

        /* Set up phy address*/
        int phy_num = (int) (port/2);
        int phy_addr_up = ge_port_mapping_phy_addr_up[port];
        int phy_addr_down = ge_port_mapping_phy_addr_down[port];
        bcm54194_rdb_access_enable(phy_num, phy_addr_up);
        bcm54194_rdb_access_enable(phy_num, phy_addr_down);


        rdb_offset = BCM54194_COPPER_MISCEL_CTRL_REG; /* 0x2F */
        bcm54194_rdb_read(phy_num, phy_addr_down, rdb_offset, &rdb_down_rdval);
        bcm54194_rdb_read(phy_num, phy_addr_up, rdb_offset, &rdb_up_rdval);

        if (action == 0)
        {
            rdb_up_rdval |= BCM54194_RX_PKT_COUNTER_EN; /* 0x800 */
            bcm54194_rdb_write(phy_num, phy_addr_up, rdb_offset, rdb_up_rdval);
            rdb_down_rdval |= BCM54194_RX_PKT_COUNTER_EN ;/* 0x800 */
            bcm54194_rdb_write(phy_num, phy_addr_down, rdb_offset, rdb_down_rdval);
            printf("\nRX packet Counter Enable at ETH %d", port);
        } else if (action == 1)
        {
            rdb_up_rdval &= ~BCM54194_RX_PKT_COUNTER_EN;
            bcm54194_rdb_write(phy_num, phy_addr_up, rdb_offset, rdb_up_rdval);
            rdb_down_rdval &= ~BCM54194_RX_PKT_COUNTER_EN;
            bcm54194_rdb_write(phy_num, phy_addr_down, rdb_offset, rdb_down_rdval);
            printf("\nTX packet Counter Enable at ETH %d", port);
        } else
        {
            printf("\nETH %d : ", port);
            printf("\n------------------------------------------------------");
            printf("\nNetwork - downstream port");
            bcm54194_rdb_read(phy_num, phy_addr_down, 0x1, &rdb_down_rdval);
            printf("\nRDB reg 0x1 = %x", rdb_down_rdval);
            bcm54194_rdb_read(phy_num, phy_addr_down, 0x4, &rdb_down_rdval);
            printf("\nRDB reg 0x4 = %x", rdb_down_rdval);
            bcm54194_rdb_read(phy_num, phy_addr_down, 0x30, &rdb_down_rdval);
            printf("\nRDB reg 0x30 = %x",rdb_down_rdval);
            bcm54194_rdb_read(phy_num, phy_addr_down, 0x2F, &rdb_down_rdval);
            printf("\nRDB reg 0x2F = %x",rdb_down_rdval);
            printf("\nSystem - upstream port");
            bcm54194_rdb_read(phy_num, phy_addr_up, 0x1, &rdb_up_rdval);
            printf("\nRDB reg 0x1 = %x", rdb_up_rdval);
            bcm54194_rdb_read(phy_num, phy_addr_up, 0x4, &rdb_up_rdval);
            printf("\nRDB reg 0x4 = %x", rdb_up_rdval);
            bcm54194_rdb_read(phy_num, phy_addr_up, 0x30, &rdb_up_rdval);
            printf("\nRDB reg 0x30 = %x",rdb_up_rdval);
            bcm54194_rdb_read(phy_num, phy_addr_up, 0x2F, &rdb_up_rdval);
            printf("\nRDB reg 0x2F = %x",rdb_up_rdval);
            printf("\n\n");
        }
    }
}

/*
 * Function: bcm54194_config_interrupt
 *
 * Description:
 *     configure the phy Code interrupts on the INTRP.
 * Input:
 *   phy_num  - PHY number in Fugazi (2,3,4,5)
 *   phy_port - PHY port number, two ports per PHY in Fugazi (0, 1)
 *   enable_flag - 1: enable; 0: disable
 * Note:
 *   Since RDB reg 0x82D is in Global Register space and needs to be accessed
 *   using first port PHY address.
 *
 * Return: PASSED/FAILED
 */
int bcm54194_config_interrupt (int phy_num, int phy_port, int enable_flag)
{
    int rc = PASSED;
    int phy_addr;
    int rdb_offset;
    uint16_t reg_val, port_int_mask=0x00;


    /* configure the phy Code interrupts on the INTRP base on PHY port */
    port_int_mask |= (BCM54194_PORT0_INT_DIS << phy_port);
    phy_addr = ge_port_mapping_phy_addr_down[phy_num * 2];
    rdb_offset = BCM54194_TOP_INTERRUPT_MASK_REG;
    rc = bcm54194_rdb_read(phy_num, phy_addr, rdb_offset, &reg_val);
    if (rc != PASSED) {
        printf("Failed to read GE PHY %d, phy addr:0x%x, RDB offset:0x%x, value=%#.4x\n",
                phy_num-2, phy_addr, rdb_offset, reg_val);
        return (rc);
    }
    if (diagflag_xram & D_DEBUG_OPTIONS) {
        printf("R: PHY %d, phy addr:0x%x, RDB offset:0x%x, value=%#.4x\n",
                phy_num, phy_addr, rdb_offset, reg_val);
    }
    /* disable all the Ports interrupt first */
    reg_val |= BCM54194_PORT_ALL_INT_MASK;
    if (enable_flag) {
        /* interrupt output enabled on INTRP ball */
        reg_val &= ~port_int_mask;
    }
    else {
        /* interrupt output disable on INTRP ball */
        reg_val |= port_int_mask;
    }
    if (diagflag_xram & D_DEBUG_OPTIONS) {
        printf("W: PHY %d, phy addr:0x%x, RDB offset:0x%x, value=%#.4x\n",
                  phy_num, phy_addr, rdb_offset, reg_val);
    }
    rc = bcm54194_rdb_write(phy_num, phy_addr, rdb_offset, reg_val);
    if (rc != PASSED) {
        printf("Failed to write GE PHY %d, phy addr:0x%x, RDB offset:0x%x, value=%#.4x\n",
                 phy_num-2, phy_addr, rdb_offset, reg_val);
        return (rc);
    }

    return (rc);
}

/*
 * Function: bcm54194_interrupt_generate
 *
 * Description:
 *     Configure the phy loopback to make link change condition to
 *     generate link change interrupt.
 * Input:
 *   phy_addr - PHY MDIO address
 *   phy_num  - 1G PHY number in Fugazi (2,3,4,5)
 *   enable_flag - 1: enable; 0: disable
 * Note:
 *   Since RDB reg 0x82D is in Global Register space and needs to be accessed
 *   using first port PHY address.
 *
 * Return: PASSED/FAILED
 */
int bcm54194_interrupt_generate (int phy_addr, int phy_num, int enable_flag)
{
    int rc = PASSED;
    int rdb_offset, regnum;
    uint16_t reg_val;

    if (!check_ext_lpbk_flag()) {
        /* Ext. loopback diag flag is off. config host local loopback */

        rdb_offset = BCM54194_MODE_CTRL_REG;
        rc = bcm54194_rdb_read(phy_num, phy_addr, rdb_offset, &reg_val);
        printf("R: PHY %d, phy addr:0x%x, rdb_offset:0x%x, value=%#.4x\n",
                phy_num, phy_addr, rdb_offset, reg_val);
        if (rc != PASSED) {
            printf("Failed to read GE PHY %d, phy addr:0x%x, RDB offset:0x%x, value=%#.4x\n",
                   phy_num-2, phy_addr, rdb_offset, reg_val);
           return (rc);
        }
        if (enable_flag) {
            /* Select SGMII register 0x0 to 0x0F */
            reg_val |= BCM54194_REG_1000X_EN_BIT;
        }
        else {
            /* Select copper register 0x0 to 0x0F */
            reg_val &= ~BCM54194_RESTART_AN;
        }
        if (diagflag_xram & D_DEBUG_OPTIONS) {
            printf("W: PHY %d, phy addr:0x%x, rdb_offset:0x%x, value=%#.4x\n",
                     phy_num, phy_addr, rdb_offset, reg_val);
        }
        rc = bcm54194_rdb_write(phy_num, phy_addr, rdb_offset, reg_val);
        if (rc != PASSED) {
            printf("Failed to write GE PHY %d, phy addr:0x%x, RDB offset:0x%x, value=%#.4x\n",                                                                       phy_num-2, phy_addr, rdb_offset, reg_val);
            return (rc);
        }

        regnum = BCM54194_CTRL_REG;
        rc = fugazi_bnxt_mdio_read(&fugazi_struct->bnxt[phy_num], phy_addr, devad, regnum, &reg_val);
        if (diagflag_xram & D_DEBUG_OPTIONS) {
            printf("R: PHY %d, phy addr:0x%x, regnum:0x%x, value=%#.4x\n",
                       phy_num, phy_addr, regnum, reg_val);
        }
        reg_val = (BCM54194_INTERNAL_LOOPBACK | BCM54194_DUPLEX_BIT |
                   BCM54194_MSB_SPEED_SEL | BCM54194_CTRL_REG_RESERVED_BITS); /* 0x4140 */
        rc = fugazi_bnxt_mdio_write(&fugazi_struct->bnxt[phy_num], phy_addr, devad, regnum, reg_val);
        printf("W: PHY %d, phy addr:0x%x, regnum:0x%x, value=0x%#.4x\n",
                   phy_num, phy_addr, regnum, reg_val);
        if (rc != PASSED) {
            printf("Failed to write GE PHY %d, phy_addr:0x%x, 0x%x\n", phy_num-2, phy_addr, regnum);
        }
    } /* Ext. loopback diag flag is off. config host local loopback */
    else {
        /* Ext. loopback diag flag is on. config line loopback */
        /* Disable Copper/Fiber Auto Switching */
        rdb_offset =  BCM54194_AUTO_DETECT_MEDIUM_REG; /* 0x23E */
        reg_val = (BCM54194_AUXILIARY_100X_SEL | BCM54194_FIBER_IN_USE_LED |
                   BCM54194_FIBER_LED | BCM54194_FIBER_SD_SYNC_STATUS);        /* 0x78E0 */
        rc = bcm54194_rdb_write(phy_num, phy_addr, rdb_offset, reg_val);
        if (rc != PASSED) {
            printf("Failed to write GE PHY %d, phy addr:0x%x, RDB offset:0x%x, value=%#.4x\n",                                                                                                              phy_num-2, phy_addr, rdb_offset, reg_val);
            return (rc);
        }

        /* Enable 1000BASE-X mode and 1000BASE-X Register spac */
        rdb_offset = BCM54194_MODE_CTRL_REG;
        reg_val = 0x7C30;
        rc = bcm54194_rdb_write(phy_num, phy_addr, rdb_offset, reg_val);
        if (rc != PASSED) {
            printf("Failed to write GE PHY %d, phy addr:0x%x, RDB offset:0x%x, value=%#.4x\n",                                                                                                              phy_num-2, phy_addr, rdb_offset, reg_val);
            return (rc);
        }

        /* 1000BASE-X Line-Side Loopback Enable */
        rdb_offset = BCM54194_COPPER_MISCEL_TEST_REG;
        rc = bcm54194_rdb_read(phy_num, phy_addr, rdb_offset, &reg_val);
        if (rc != PASSED) {
            printf("Failed to read GE PHY %d, phy addr:%d, RDB offset:0x%x\n", phy_num-2, phy_addr, rdb_offset);
            return (rc);
        }
        if (enable_flag) {
            /* enable line loopback */
            reg_val |= BCM54194_RMT_LPBK_EN;
        }
        else {
            /* disable line loopback */
            reg_val &= ~BCM54194_RMT_LPBK_EN;
        }
        if (diagflag_xram & D_DEBUG_OPTIONS) {
            printf("W: PHY %d, phy addr:0x%x, rdb_offset:0x%x, value=%#.4x\n",
                       phy_num, phy_addr, rdb_offset, reg_val);
        }
        rc = bcm54194_rdb_write(phy_num, phy_addr, rdb_offset, reg_val);
        if (rc != PASSED) {
            printf("Failed to write GE PHY %d, phy addr:0x%x, RDB offset:0x%x, value=%#.4x\n",                                                                                                              phy_num-2, phy_addr, rdb_offset, reg_val);
            return (rc);
        }
    }  /* Ext. loopback diag flag is on. config line loopback */

    return (rc);
}


/*
 * Function: bcm54194_interrupt_set
 *
 * Description:
 *   to Disable/Enable/clear BCM54194 Link change Interrupt .
 *
 * Input:
 *   phy_addr - PHY address
 *   phy_num  - PHY number in Fugazi (2,3,4,5)
 *   int_mode - 0: disable, 1: enable, 2: clear link change interrupt
 *
 * Return: PASSED/FAILED
 */
int bcm54194_interrupt_set (int phy_addr, int phy_num, int int_mode)
{
    int rc = PASSED;
    int rdb_offset;
    uint16_t reg_val;


    rdb_offset = BCM54194_EXPANSION_INTERRUPT_MASK_REG;  /* 0x32 */
    rc = bcm54194_rdb_read(phy_num, phy_addr, rdb_offset, &reg_val);
    if (rc != PASSED) {
        printf("Failed to read GE PHY %d, phy addr:0x%x, RDB offset:0x%x, value=%#.4x\n",
                phy_num-2, phy_addr, rdb_offset, reg_val);
        return (rc);
    }
    if (diagflag_xram & D_DEBUG_OPTIONS) {
        printf("R: PHY %d, phy addr:0x%x, RDB offset:0x%x, value=%#.4x\n",
             phy_num, phy_addr, rdb_offset, reg_val);
    }
    if (int_mode == 1) {
        /* Enable BCM54194 Link change Interrupt */
        reg_val &= ~BCM54194_SERDES_LINK_STATUS_CHANGE_INT_DIS;
    }
    else {
        /* Disable BCM54194 Link change Interrupt */
        reg_val |= BCM54194_SERDES_LINK_STATUS_CHANGE_INT_DIS;
    }
    if (diagflag_xram & D_DEBUG_OPTIONS) {
        printf("W: PHY %d, phy addr:0x%x, RDB offset:0x%x, value=%#.4x\n",
            phy_num, phy_addr, rdb_offset, reg_val);
    }
    rc = bcm54194_rdb_write(phy_num, phy_addr, rdb_offset, reg_val);
    if (rc != PASSED) {
        printf("Failed to write GE PHY %d, phy addr:0x%x, RDB offset:0x%x, value=%#.4x\n",
                 phy_num-2, phy_addr, rdb_offset, reg_val);
        return (rc);
    }

    return (rc);
}

/*
 * Function: bcm54194_interrupt_clear
 *
 * Description:
 *   to clear BCM54194 interrupt status. Read to clear.
 *
 * Input:
 *   phy_addr - PHY address
 *   phy_num  - PHY number in Fugazi (2,3,4,5)
 *
 * Return: PASSED/FAILED
 */
int bcm54194_interrupt_clear (int phy_addr, int phy_num)
{
    int rc = PASSED;
    int rdb_offset;
    uint16_t reg_val;


    rdb_offset = BCM54194_EXPANSION_INTERRUPT_STATUS_REG;  /* 0x31 */
    rc = bcm54194_rdb_read(phy_num, phy_addr, rdb_offset, &reg_val);
    if (rc != PASSED) {
        printf("Failed to read GE PHY %d, phy addr:0x%x, RDB offset:0x%x, value=%#.4x\n",
                    phy_num-2, phy_addr, rdb_offset, reg_val);
        return (rc);
    }
    if (diagflag_xram & D_DEBUG_OPTIONS) {
        printf("R: PHY %d, phy addr:0x%x, RDB offset:0x%x, value=%#.4x\n",
                    phy_num, phy_addr, rdb_offset, reg_val);
    }
    return (rc);
}


/*
 * Function: bcm54194_interrupt_get
 *
 * Description:
 *   to get which port that generated the interrupt.
 *
 * Input:
 *   phy_addr - PHY address
 *   phy_num  - PHY number in Fugazi (2,3,4,5)
 *
 *  Output:
 *   int_status - interrupt status info
 *
 * Return: PASSED/FAILED
  */
int bcm54194_interrupt_get (int phy_addr, int phy_num, uint16_t *int_status)
{
    int rc = PASSED;
    int rdb_offset;
    uint16_t reg_val;

    rdb_offset = BCM54194_PORT_INTERRUPT_STATUS_REG;  /* 0x3B */
    rc = bcm54194_rdb_read(phy_num, phy_addr, rdb_offset, &reg_val);
    if (rc != PASSED) {
        printf("Failed to read GE PHY %d, phy addr:0x%x, RDB offset:0x%x, value=%#.4x\n",
                phy_num-2, phy_addr, rdb_offset, reg_val);
        return (rc);
    }
    if (diagflag_xram & D_DEBUG_OPTIONS) {
        printf("R: PHY %d, phy addr:0x%x, RDB offset:0x%x, value=%#.4x\n",
                phy_num, phy_addr, rdb_offset, reg_val);
    }
    *int_status = reg_val;

    return (rc);
 }


/*
 * Function: bcm54194_interrupt_util
 *
 * Description:
 * Utility to Enable/Disable BCM54194 LASI Interrupt .
 *
 * Input: none
 *
 * Return: None
 */
void bcm54194_interrupt_util (void)
{
    int rc = PASSED;
    int ethnum, ethnum_index, eth_start, eth_end;
    int phy_addr, phy_num, phy_port;
    unsigned int test_mode;
    uint16_t int_status=0;


    printf("\nPort number 0 - 7 = ETH4 - ETH11");
    ethnum = gethex_answer("\nEnter eth num (0x4 - 0xB; 0xff-all ports)", 0xff, 4, 0xff);
    test_mode = getdec_answer("Enter Enable(Disable:0, Enable:1, Clear:2, Status:3, en lpbk:4, dis_lpbk:5, init:6)", 1, 0, 6);

    if ( ethnum == 0xff) {
        eth_start = FUGAZI_1G_eth_4;
        eth_end = MAX_FUGAZI_1G_ETH;
    }
    else {
        eth_start = ethnum;
        eth_end = ethnum + 1;
    }


    for (ethnum_index=eth_start; ethnum_index<eth_end; ethnum_index++) {
        phy_addr = ge_port_mapping_phy_addr_down[ethnum_index];
        phy_num  = (int) (ethnum_index/2);
        phy_port = (int) (ethnum_index%phy_num);
        printf("\nethnum_index=%d, phy_num=%d, phy_port=%d, phy_addr=0x%02x\n",
                ethnum_index, phy_num, phy_port, phy_addr);
        switch (test_mode) {
        case 6:
            /* Configure PHY core interrupts on the INTRP */
            rc = bcm54194_config_interrupt(phy_num, phy_port, ENABLE);
            if (rc != PASSED) {
                printf("Failed to bcm54194_config_interrupt() GE PHY %d, phy_port %d, phy addr:0x%x\n",
                        phy_num-2, phy_port, phy_addr);
            }
            break;
        case 0:
        case 1:
            /* Disable/Enable BCM54194 Link change Interrupt */
            rc = bcm54194_interrupt_set(phy_addr, phy_num, test_mode);
            if (rc != PASSED) {
                printf("Failed to bcm54194_interrupt_set() GE PHY %d, phy addr:0x%x, test_mode=%d\n",
                        phy_num-2, phy_addr, test_mode);
            }
            break;
        case 2:
            /* Clear BCM54194 Link change Interrupt */
            rc = bcm54194_interrupt_clear(phy_addr, phy_num);
            if (rc != PASSED) {
                printf("Failed to bcm54194_interrupt_clear() GE PHY %d, phy addr:0x%x\n",
                        phy_num-2, phy_addr);
            }
            break;
        case 3:
            /* Read BCM54194 interrupt status */
            rc = bcm54194_interrupt_get(phy_addr, phy_num, &int_status);
            if (rc != PASSED) {
                printf("Failed to bcm54194_interrupt_get() GE PHY %d, phy addr:0x%x, test_mode=%d\n",
                        phy_num-2, phy_addr, test_mode);
            }
            printf("PHY %d, phy addr:0x%x, int_status=0x%#.4x\n",
                        phy_num, phy_addr, int_status);
            break;
        case 4:
            /* Generate interrupts by enable System loopback */
            rc = bcm54194_interrupt_generate(phy_addr, phy_num, ENABLE);
            if (rc != PASSED) {
                printf("Failed to bcm54194_interrupt_generate() GE PHY %d, phy addr:0x%x, test_mode=%d\n",
                        phy_num-2, phy_addr, test_mode);
            }
            break;
        case 5:
            /* Disable System loopback */
            rc = bcm54194_interrupt_generate(phy_addr, phy_num, DISABLE);
            if (rc != PASSED) {
                printf("Failed to bcm54194_interrupt_generate() GE PHY %d, phy addr:0x%x, test_mode=%d\n",
                        phy_num-2, phy_addr, test_mode);
            }
            break;
        default:
            printf("ERROR: invalid test_mode %d\n", test_mode);
        } /* switch (test_mode) { */
    } /* for (ethnum_index=eth_start; ethnum_index<eth_end; ethnum_index++) { */
}

/*
 * Function: bcm54194_interrupt_test
 *
 * Description:
 *   BCM54194 LASI Interrupt test from utility
 *
 * Input: ethnum_index - eth port number, start from 4 (eth4) for 1G PHY
 *        int_status - pointer to a storage to store if interrupt is generate status
 * Return: PASSED/FAILED
 */
int bcm54194_interrupt_test(int ethnum_index, uint16_t *int_status)
{
    int rc = PASSED;
    int phy_addr, phy_num, phy_port;
    uint16_t status = 0;


    phy_addr = ge_port_mapping_phy_addr_down[ethnum_index];
    phy_num  = (int) (ethnum_index/2);
    phy_port = (int) (ethnum_index%phy_num);
    printf("\nethnum_index=%d, phy_num=%d, phy_port=%d, phy_addr=0x%02x\n",
            ethnum_index, phy_num, phy_port, phy_addr);

    /* Configure PHY core interrupts on the INTRP */
    if ( bcm54194_config_interrupt(phy_num, phy_port, ENABLE) ) {
        printf("Failed to bcm54194_config_interrupt() GE PHY %d, phy_port %d, phy addr:0x%x\n",
                phy_num-2, phy_port, phy_addr);
        rc |= FAILED;
    }

    /* Clear BCM54194 Link change Interrupt */
    if ( bcm54194_interrupt_clear(phy_addr, phy_num) ) {
        printf("Failed to bcm54194_interrupt_clear() GE PHY %d, phy addr:0x%x\n",
                phy_num-2, phy_addr);
        rc |= FAILED;
    }

    /* Enable BCM54194 Link change Interrupt */
    if ( bcm54194_interrupt_set(phy_addr, phy_num, ENABLE) ) {
        printf("Failed to bcm54194_interrupt_set(ENABLE) GE PHY %d, phy addr:0x%x\n",
                phy_num-2, phy_addr);
        rc |= FAILED;
    }

    /* Generate interrupts by enable System loopback */
    if ( bcm54194_interrupt_generate(phy_addr, phy_num, ENABLE) ) {
        printf("Failed to bcm54194_interrupt_generate(ENABLE) GE PHY %d, phy addr:0x%x\n",
                phy_num-2, phy_addr);
        rc |= FAILED;
    }

    /* Disable System loopback */
    if ( bcm54194_interrupt_generate(phy_addr, phy_num, DISABLE) ) {
        printf("Failed to bcm54194_interrupt_generate(DISABLE) GE PHY %d, phy addr:0x%x\n",
                phy_num-2, phy_addr);
        rc |= FAILED;
    }

    /* Read BCM54194 interrupt status */
    if ( bcm54194_interrupt_get(phy_addr, phy_num, &status) ) {
        printf("Failed to bcm54194_interrupt_get() GE PHY %d, phy addr:0x%x\n",
                phy_num-2, phy_addr);
        rc |= FAILED;
    }
    else {
        *int_status = status;
    }

    /* Disable BCM54194 Link change Interrupt */
    if ( bcm54194_interrupt_set(phy_addr, phy_num, DISABLE) ) {
        printf("Failed to bcm54194_interrupt_set() GE PHY %d, phy addr:0x%x\n",
                phy_num-2, phy_addr);
        rc |= FAILED;
    }

    if ( !(status & (1 << phy_port)) ) {
        rc |= FAILED;
     }

    return (rc);
}

/*-------------------------------------------------
$Log: diag_bcm54194_api.c,v $
Revision 1.2  2021/06/02 08:22:34  iachang
CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk

Revision 1.1.4.3  2021/05/21 18:44:03  pdoong
Reduce BCM54194 internal/external loopback test checking link up timeout from 100sec to 10sec.

Revision 1.1.4.2  2020/08/26 02:37:47  iachang
Merge Fugazi code into main trunk

Revision 1.1.2.8  2020/08/25 01:10:31  pdoong
Updated code from PRRQ comments.

Revision 1.1.2.7  2020/08/24 00:01:07  pdoong
Change to use BCM54194 register defintion macro for ER.

Revision 1.1.2.6  2020/08/06 04:23:24  pdoong
clean code for BCM54194 1G PHY

Revision 1.1.2.5  2020/07/03 07:34:20  iachang
Support bcm57412_mdio_bus_release() and bcm57412_mdio_bus_acquire()
Move those funcitons from diag_bcm57412_test.c to diag_bcm57412_utils.c

Revision 1.1.2.4  2020/06/26 20:16:09  pdoong
Enable 1G PHY Auto Clock mode for SyncE Recovered Clock test.

Revision 1.1.2.3  2020/04/16 01:56:44  iachang
Add BCM54194 1G PHY LASI test.

Revision 1.1.2.2  2020/02/25 02:48:20  pdoong
add utility to enable/generate 1G PHY interrupt to BCM57412 MAC LASI

Revision 1.1.2.1  2019/10/16 06:12:31  letsai
Modify file name

Revision 1.1.6.28  2019/09/23 07:38:25  letsai
Add packet counter utility of BCM54194 phy

Revision 1.1.6.27  2019/08/30 06:44:56  letsai
1. Use HW reset to replace SW reset before BCM 54194 phy internal/external test. 2. Add delay time for Fiber link up.

Revision 1.1.6.26  2019/08/29 20:32:55  pdoong
Added Clear option in PRBS utility.

Revision 1.1.6.25  2019/08/21 22:27:44  pdoong
Enhanced PRBS inject error feature.

Revision 1.1.6.24  2019/08/21 06:38:56  letsai
Add BCM54194 1G PHY PRBS utility

Revision 1.1.6.23  2019/08/02 07:16:50  letsai
1.Add debug messgage. 2.Fix initial process for BCM 54194 phy

Revision 1.1.6.22  2019/07/19 07:35:28  letsai
1. Support LED control.
2. Support smart fan.
3. Change BCM 54194 phy reset bit.

Revision 1.1.6.21  2019/06/18 06:28:24  letsai
Increase reset time (According to datasheet)

Revision 1.1.6.20  2019/06/15 03:48:42  letsai
1.Fix Rx mismatch error messgage showed in loopback test. 2.Removed Copper registers in BCM54194 phy register test. 3.Add print messgge when reset 1G phy.

Revision 1.1.6.19  2019/06/05 02:08:35  letsai
Turn autonegotiation off when doing external loopback for BCM54194 PHY

Revision 1.1.6.18  2019/05/21 23:22:29  pdoong
Added SyncE recovered clock test from bcm54194 1G PHY output clock

Revision 1.1.6.17  2019/04/25 23:25:27  letsai
1. Remove eUSB test.
2. Fixed bnxt_mdio r/w function to support both 1G and 10G phy.

Revision 1.1.6.16  2019/04/18 23:11:58  letsai
Add loopback mode config uyility and clean up code.

Revision 1.1.6.15  2019/04/18 01:21:30  letsai
1. Clean up code
2. Modify 1G phy address mapping
3. Modify print message of MCU FW opgrade

Revision 1.1.6.14  2019/04/12 23:03:25  letsai
Add utility to enable 1000BASE-X Line-Side Loopback

Revision 1.1.6.13  2019/04/11 23:56:12  letsai
Add dump info.

Revision 1.1.6.12  2019/04/11 23:50:38  letsai
Modify the dump info.

Revision 1.1.6.11  2019/04/11 22:32:28  letsai
1. Replace the sign "*" to "-" when doing FPGA interrupt test
2. Fix M.2 combo test when slot is empty.
3. Make "check link utility" easy to use.
4. When USB console detected, check the corresponding FPGA register bit.

Revision 1.1.6.10  2019/04/10 21:26:58  letsai
1. Support BCM54194 PHY SGMII Internal Loopback test.
2. Return FAILED when M.2 module not present.
3. Clean up code.

Revision 1.1.6.9  2019/04/10 16:29:30  letsai
1. Fix ethernet mapping.
2. Support all BCM54194 phy in utilities.
3. Remove unused functions.

Revision 1.1.6.8  2019/04/09 16:10:39  letsai
1. Support all BCM54194 PHY (0~3) Register Test.
2. Let utilities can dump each phy registers.
3. Check link status for each phy and each port(upstream and downstream).

Revision 1.1.6.7  2019/04/06 01:36:14  letsai
1. Remove unused functions and files.
2. Fix BCM54194 SFP External loopback test.
3. Fix BCM54194 Register test.
4. Fix Voltage Margin Utility.
5. Add function to show system information.

Revision 1.1.6.6  2019/04/03 18:30:36  letsai
Add utility to check link status

Revision 1.1.6.5  2019/04/02 22:15:11  letsai
Modify init scrip

Revision 1.1.6.4  2019/03/30 00:56:02  letsai
1. Add USB console detect utility.
2. Modify FAN utility.
3. Remove unused items.
4. Fix BCM54194 phy register test.

Revision 1.1.6.3  2019/03/25 18:37:36  letsai
Modified eth and port number

Revision 1.1.6.2  2019/03/14 03:48:34  letsai
Initial check in.



$Endlog$
*/
