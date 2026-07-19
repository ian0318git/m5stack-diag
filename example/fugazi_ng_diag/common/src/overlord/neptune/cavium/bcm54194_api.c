/* $Id: bcm54194_api.c,v 1.4 2018/07/23 07:38:47 meho Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/cavium/bcm54194_api.c,v $
 *-----------------------------------------------------------------------------
 * bcm54194_api.c - API for BCM GE PHY bcm54194.
 *
 *
 * June 2016, Mecca Ho
 *
 * Copyright (c) 2016 - 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */
#include <stdio.h>
#include <linux/socket.h>
#include <sys/ioctl.h>
#include <linux/if_ether.h>
#include <netinet/in.h>
#include "types.h"
#include "common.h"
#include "cvmx.h"
#include "platform_eth.h"
#include "platform_ext_lpbk.h"
#include "cvmx-mdio.h"
#include "bcm54194_api.h"
#include "dash_fpga.h"

extern void reset_platform_ext_dev (int);
extern void unreset_platform_ext_dev (int);

extern int is_glc_ge_100fx;

extern void msleep(unsigned long t);
int macsec_mspu_port_mapping_tbl[] = {0x0, 0x3, 0x6, 0x9, 0xC, 0xF, 0x12, 0x15};

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
 * Function: bcm54194_mspu_bcast_write
 *
 * Description: BCM54194 MSPU register broadcast write.
 * The broadcast feature allows writing to all MSPU ports/slices at the the same time
 * and can be used for the write command only.
 * Write access to a register in all eight slices of the MSPU can be achieved by doing
 * a single broadcast write.
 * Input
 *
 * Return: none
 */
int bcm54194_mspu_write(int mspu_sector, int mspu_reg, uint32_t reg_val)
{
    int bus_id = SMI_BUS_0, rc, regnum = 0x0;
    int mspu_phyad = ge_port_mapping_phy_addr[0]+9; /* 0xF + 0x9 */
    unsigned short wrval_lsb, wrval_msb;

    wrval_lsb = (reg_val & 0xFFFF);
    wrval_msb = (reg_val >> 16);
    /* Write the RDB register */
    regnum = 0x1B;
    rc = cvmx_mdio_write(bus_id, mspu_phyad, regnum, mspu_sector);
    if (rc < 0) {
        printf("Failed to write GE PHY, phy_addr:0x%x, 0x%x\n", mspu_phyad, regnum);
        return (rc);
    }

    regnum = 0x18;
    rc = cvmx_mdio_write(bus_id, mspu_phyad, regnum, mspu_reg);
    if (rc != PASSED) {
        printf("Failed to write GE PHY, phy_addr:0x%x, 0x%x\n", mspu_phyad, regnum);
        return (rc);
    }

    regnum = 0x19;
    rc = cvmx_mdio_write(bus_id, mspu_phyad, regnum, wrval_lsb);
    if (rc != PASSED) {
        printf("Failed to write GE PHY, phy_addr:0x%x, 0x%x\n", mspu_phyad, regnum);
        return (rc);
    }

    regnum = 0x1A;
    rc = cvmx_mdio_write(bus_id, mspu_phyad, regnum, wrval_msb);
    if (rc != PASSED) {
        printf("Failed to write GE PHY, phy_addr:0x%x, 0x%x\n", mspu_phyad, regnum);
        return (rc);
    }
    return (rc);
}

/*
 * Function: bcm54194_rdb_access_enable
 *
 * Description: Enable BCM54194 RDB access mode.
 *
 * Hardware or software resets to the chip will enable RDB Access mode.
 * Input: none
 *
 * Return: none
 */
int bcm54194_rdb_access_enable()
{
    int rc, regnum = 0x0, bus_id = SMI_BUS_0;
    int phy_addr = ge_port_mapping_phy_addr[0];

    /* Enable RDB access mode */
    regnum = 0x17;
    rc = cvmx_mdio_write(bus_id, phy_addr, regnum, 0x0F7E);
    if (rc != PASSED) {
        printf("Failed to write GE PHY, phy_addr:0x%x, 0x%x\n", phy_addr, regnum);
        return (rc);
    }
    regnum = 0x15;
    rc = cvmx_mdio_write(bus_id, phy_addr, regnum, 0x0000);
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
 *
 * Hardware or software resets to the chip will enable RDB Access mode.
 * Input: none
 *
 * Return: none
 */
int bcm54194_rdb_access_disable()
{
    int rc, regnum = 0x0, bus_id = SMI_BUS_0;
    int phy_addr = ge_port_mapping_phy_addr[0];
    
    /* Disable RDB access mode */
    regnum = 0x1E;
    rc = cvmx_mdio_write(bus_id, phy_addr, regnum, 0x0087);
    if (rc != PASSED) {
        printf("Failed to write GE PHY, phy_addr:0x%x, 0x%x\n", phy_addr, regnum);
        return (rc);
    }
    regnum = 0x1F;
    rc = cvmx_mdio_write(bus_id, phy_addr, regnum, 0x8000);
    if (rc != PASSED) {
        printf("Failed to write GE PHY, phy_addr:0x%x, 0x%x\n", phy_addr, regnum);
        return (rc);
    }
    rc = cvmx_mdio_write(bus_id, phy_addr, regnum, 0x8000);
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
 *
 * Input: none
 *
 * Return: none
 */
int bcm54194_rdb_read(int bus_id, int phy_addr, int rdb_offset, uint16_t *reg_val)
{
    int rc, regnum = 0x0;

    /* Read the RDB register */
    regnum = 0x1E;
    rc = cvmx_mdio_write(bus_id, phy_addr, regnum, rdb_offset);
    if (rc != PASSED) {
        printf("Failed to write GE PHY, phy_addr:0x%x, 0x%x\n", phy_addr, regnum);
        return (rc);
    }
    regnum = 0x1F;
    *reg_val = cvmx_mdio_read(bus_id, phy_addr, regnum);
    if (reg_val < 0) {
        printf("Failed to read GE PHY, phy_addr:0x%x, 0x%x\n", phy_addr, regnum);
        return (rc);
    }

    return (rc);
}

/*
 * Function: bcm54194_rdb_write
 *
 * Description: BCM54194 RDB register write.
 *
 * Input: none
 *
 * Return: none
 */
int bcm54194_rdb_write(int bus_id, int phy_addr, int rdb_offset, uint16_t reg_val)
{
    int rc, regnum = 0x0;

    /* Write the RDB register */
    regnum = 0x1E;
    rc = cvmx_mdio_write(bus_id, phy_addr, regnum, rdb_offset);
    if (rc < 0) {
        printf("Failed to write GE PHY, phy_addr:0x%x, 0x%x\n", phy_addr, regnum);
        return (rc);
    }
    regnum = 0x1F;
    rc = cvmx_mdio_write(bus_id, phy_addr, regnum, reg_val);
    if (rc != PASSED) {
        printf("Failed to write GE PHY, phy_addr:0x%x, 0x%x\n", phy_addr, regnum);
        return (rc);
    }

    return (rc);
}

/*
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
 */
int bcm54194_switch_intf_access(bcm54194_intf_t intf)
{
    uint32_t rc = FAILED;
    uint16_t reg_val, reg_val1, reg_val2, reg_val3;
    int bus_id = SMI_BUS_0, rdb_offset;
    int phy_addr = ge_port_mapping_phy_addr[0];

    rdb_offset = BCM54194_EXTERNAL_SERDES_CTRL_REG;
    rc = bcm54194_rdb_read(bus_id, phy_addr, rdb_offset, &reg_val1);
    if (rc < 0) {
        printf("Failed to read GE PHY, phy addr:%d, RDB offset:0x%x\n", phy_addr, rdb_offset);
        return (rc);
    }
    reg_val1 &= ~(0x60);
    reg_val1 |= (0x40);
    rc = bcm54194_rdb_write(bus_id, phy_addr, rdb_offset, reg_val1);
    if (rc < 0) {
    	printf("Failed to write GE PHY, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n", phy_addr, rdb_offset, reg_val1);
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

    rdb_offset = 0x234;
    rc = bcm54194_rdb_read(bus_id, phy_addr, rdb_offset, &reg_val);
    if (rc < 0) {
        printf("Failed to read GE PHY, phy addr:%d, RDB offset:0x%x\n", phy_addr, rdb_offset);
        return (rc);
    }
    reg_val &= ~(0x60);
    reg_val |= reg_val2;
    rc = bcm54194_rdb_write(bus_id, phy_addr, rdb_offset, reg_val);
    if (rc < 0) {
    	printf("Failed to write GE PHY, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n", phy_addr, rdb_offset, reg_val);
        return (rc);
    }

    rdb_offset = 0x021;
    rc = bcm54194_rdb_read(bus_id, phy_addr, rdb_offset, &reg_val);
    if (rc < 0) {
        printf("Failed to read GE PHY, phy addr:%d, RDB offset:0x%x\n", phy_addr, rdb_offset);
        return (rc);
    }
    reg_val &= ~(0x1);
    reg_val |= reg_val3;
    rc = bcm54194_rdb_write(bus_id, phy_addr, rdb_offset, reg_val);
    if (rc < 0) {
    	printf("Failed to write GE PHY, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n", phy_addr, rdb_offset, reg_val);
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
 * Input: none
 *
 * Return: none
 */
static int bcm54194_per_port_reset(int phy_addr, int intf)
{
    int bus_id = SMI_BUS_0;
    ushort reg_val;
    int rc, regnum = BCM54194_CTRL_REG;

    /* Switch to SGMII/Copper/Fiber register space */
    if (intf == BCM54194_FIBER_INTF) {
    	bcm54194_reg_1000x_en(phy_addr, TRUE);
    } else {
        bcm54194_reg_1000x_en(phy_addr, FALSE);
    }

    reg_val = cvmx_mdio_read(bus_id, phy_addr, regnum);
    if (reg_val < 0) {
        printf("Failed to read GE PHY, phy_addr:0x%x, 0x%x\n", phy_addr, regnum);
        return (FAILED);
    }
    reg_val |= BCM54194_RESET_BIT;
    rc = cvmx_mdio_write(bus_id, phy_addr, regnum, reg_val);
    if (rc < 0) {
        printf("Failed to write GE PHY, phy_addr:0x%x, 0x%x\n", phy_addr, regnum);
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
 * Return: none
 */
static int bcm54194_global_rdb_reset(void)
{
    int rc, bus_id = SMI_BUS_0;
    ushort reg_val;
    int rdb_offset = BCM54194_TOP_MISC_TOP_GBL_RST_REG, phy_addr = PHY_PORT0;

    rc = bcm54194_rdb_read(bus_id, phy_addr, rdb_offset, &reg_val);
    if (rc < 0) {
        printf("Failed to read GE PHY, phy addr:%d, RDB offset:0x%x\n", phy_addr, rdb_offset);
        return (rc);
    }
    reg_val |= BCM54194_TOP_MII_REG_SOFT_RST_BIT;
    rc = bcm54194_rdb_write(bus_id, phy_addr, rdb_offset, reg_val);
    if (rc < 0) {
    	printf("Failed to write GE PHY, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n", phy_addr, rdb_offset, reg_val);
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
static int bcm54194_clause45_reset(void)
{
    int bus_id = SMI_BUS_0;
    int mii_value, dev_id = 0x1;
    int rc, regnum = 0x0, phy_addr = PHY_PORT0;

    mii_value = cvmx_mdio_45_read(bus_id, phy_addr, dev_id, regnum);
    if (mii_value < 0) {
        printf("Read error from phy 0x%x\n", phy_addr);
        return (FAILED);
    }
    mii_value |= (0x80);
    rc = cvmx_mdio_45_write(bus_id, phy_addr, dev_id, regnum, mii_value);
    if (rc == -1) {
        printf("Write error from phy 0x%x\n", phy_addr);
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
 * need to fix this function
 * No one use this function so far.
 *
 * Input: none
 *
 * Return: none
 */
int bcm54194_soft_reset(void)
{
    int phy_curr = 0, phy_max = 0x4, intf = BCM54194_SGMII_INTF;

    for (phy_curr = 0; phy_curr < phy_max; phy_curr++) {
        for (intf = BCM54194_SGMII_INTF; intf < BCM54194_FIBER_INTF; intf++) {
            if (bcm54194_per_port_reset(phy_curr, intf)) {
                printf("BCM54194 Per-Port reset failed.\n");
    	        return (FAILED);
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

void bcm54194_init_script(void)
{
    int bus_id = SMI_BUS_0, rdb_reg, ix;
    ushort reg_val;
    int port_cnt = 4, phy_addr = ge_port_mapping_phy_addr[0];

    /* Workaround for MDIO address issue on BCM54194 B0 silicon.
     * Avoiding touch RDB_reg 0x234 to switch register space.
     * 
     * SGMII/Copper:
     * GPHY BA+0 to BA+3, SGMII SerDes BA+4 to BA+7, MACsec BA+9
     * 
     * SGMII/Fiber:
     * Fiber SerDes BA+0 to BA+3, SGMII SerDes BA+4 to BA+7, MACsec BA+9
     */
    bcm54194_rdb_access_disable();
    cvmx_mdio_write(bus_id, phy_addr, 0x17, 0x0D19);
    reg_val = cvmx_mdio_read(bus_id, phy_addr, 0x15);
    reg_val |= (0x1 << 3);
    cvmx_mdio_write(bus_id, phy_addr, 0x17, 0x0D19);
    cvmx_mdio_write(bus_id, phy_addr, 0x15, reg_val);
    bcm54194_rdb_access_enable();

    /* Reset the PHY by writing to the PHY Reset register in the FPGA 
     * The reset is required on A0 silicon, but not on B0 silicon
     * It appears to be harmless on B0, so go ahead and do it for all systems
     */
    cvmx_mdio_write(bus_id, phy_addr, 0x17, 0x0D19);
    cvmx_mdio_write(bus_id, phy_addr, 0x15, 0x4189);
    cvmx_mdio_write(bus_id, phy_addr, 0x17, 0x0D19);
    cvmx_mdio_write(bus_id, phy_addr, 0x15, 0xC189);

    bcm54194_rdb_write(bus_id, 0x13, 0x21, 0xFC01);
    bcm54194_rdb_write(bus_id, 0x14, 0x21, 0xFC01);
    bcm54194_rdb_write(bus_id, 0x15, 0x21, 0xFC01);
    bcm54194_rdb_write(bus_id, 0x16, 0x21, 0xFC01);

    for (ix = 0; ix < port_cnt; ix++) {
        phy_addr = ge_port_mapping_phy_addr[ix];
        /* Set RDB 0x22D.4 to 0 for SGMII auto-negotiation to function property. */
        //bcm54194_switch_intf_access(BCM54194_SGMII_INTF);
        rdb_reg = 0x22D;
        bcm54194_rdb_read(bus_id, phy_addr+4, rdb_reg, &reg_val);
        reg_val &= ~(0x1 << 4);
        bcm54194_rdb_write(bus_id, phy_addr+4, rdb_reg, reg_val);


        /* Disable SUPER_ISOLATE bit. 
         * To make 1GE PHY copper interface in normal operation. */
        //bcm54194_switch_intf_access(BCM54194_COPPER_INTF);
        rdb_reg = 0x2A;
        bcm54194_rdb_read(bus_id, phy_addr, rdb_reg, &reg_val);
        reg_val &= ~(0x1 << 5);
        bcm54194_rdb_write(bus_id, phy_addr, rdb_reg, reg_val);
    }
}

void bcm54194_reset(void)
{
    /* Reset GE PHY by FPGA */
    reset_platform_ext_dev(FPGA_EXT_GE_QUAD_RST);
    msleep(10);
    unreset_platform_ext_dev(FPGA_EXT_GE_QUAD_RST);
    msleep(10);

    bcm54194_init_script();
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
 * Return: PASSED/FAILED
 */
int bcm54194_reg_1000x_en(int phy_addr, int enable)
{
    uint16_t reg_val;
    int rc, bus_id = SMI_BUS_0, rdb_offset;

	/* Select 1000BASE-T register space. */
	rdb_offset = 0x21;
    rc = bcm54194_rdb_read(bus_id, phy_addr, rdb_offset, &reg_val);
    if (rc != PASSED) {
        printf("Failed to read GE PHY, phy addr:%d, RDB offset:0x%x\n", phy_addr, rdb_offset);
        return (rc);
    }

    if (enable) {
        reg_val |= BCM54194_REG_1000X_EN_BIT;
    } else {
        reg_val &= ~(BCM54194_REG_1000X_EN_BIT);
    }

    rc = bcm54194_rdb_write(bus_id, phy_addr, rdb_offset, reg_val);
    if (rc != PASSED) {
    	printf("Failed to write GE PHY, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n", phy_addr, rdb_offset, reg_val);
        return (rc);
    }
    return (rc);
}

/*
 * Function: bcm54194_100base_fx_config
 *
 * Description: Config BCM54194 as 100BASE-FX mode.
 * Input: phy_addr
 *
 * Return: PASSED/FAILED
 */
int bcm54194_100base_fx_config(int phy_addr, int enable)
{
    uint16_t reg_val;
    int rc, bus_id = SMI_BUS_0, rdb_offset, regnum;

    if (enable) {
        /* Disable Copper/Fiber Auto-detection */
        rdb_offset = BCM54194_AUTO_DETECT_MEDIUM_REG;
        rc = bcm54194_rdb_read(bus_id, phy_addr, rdb_offset, &reg_val);
        if (rc != PASSED) {
            printf("Failed to read GE PHY, phy addr:%d, RDB offset:0x%x\n", phy_addr, rdb_offset);
            return (rc);
        }
        reg_val &= ~(BCM54194_AUTO_DET_MEDIUM_EN_BIT);
        rc = bcm54194_rdb_write(bus_id, phy_addr, rdb_offset, reg_val);
        if (rc != PASSED) {
    	    printf("Failed to write GE PHY, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n", phy_addr, rdb_offset, reg_val);
            return (rc);
        }

        /* Enable Fiber mode and access Copper register */
        rdb_offset = BCM54194_MODE_CTRL_REG;
        rc = bcm54194_rdb_read(bus_id, phy_addr, rdb_offset, &reg_val);
        if (rc != PASSED) {
            printf("Failed to read GE PHY, phy addr:%d, RDB offset:0x%x\n", phy_addr, rdb_offset);
            return (rc);
        }
        reg_val &= ~(BCM54194_MODE_SEL_MASK | BCM54194_REG_1000X_EN_BIT);
        reg_val |= BCM54194_SGMII_TO_FIBER_MODE;
        rc = bcm54194_rdb_write(bus_id, phy_addr, rdb_offset, reg_val);
        if (rc != PASSED) {
    	    printf("Failed to write GE PHY, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n", phy_addr, rdb_offset, reg_val);
            return (rc);
        }

        /* Power down the Copper interface */
        regnum = BCM54194_CTRL_REG;
        reg_val = cvmx_mdio_read(bus_id, phy_addr, regnum);
        if (reg_val < 0) {
            printf("Failed to read GE PHY, phy_addr:0x%x, reg:0x%x\n", phy_addr, regnum);
            return (rc);
        }
        reg_val |= BCM54194_POWER_DOWN_BIT;
        rc = cvmx_mdio_write(bus_id, phy_addr, regnum, rdb_offset);
        if (rc < 0) {
            printf("Failed to write GE PHY, phy_addr:0x%x, 0x%x\n", phy_addr, regnum);
            return (rc);
        }

        /* Enable 100BASE-FX mode */
        rdb_offset = BCM54194_SERDES_100FX_CTRL_REG;
        rc = bcm54194_rdb_read(bus_id, phy_addr, rdb_offset, &reg_val);
        if (rc != PASSED) {
            printf("Failed to read GE PHY, phy addr:%d, RDB offset:0x%x\n", phy_addr, rdb_offset);
            return (rc);
        }
        reg_val |= BCM54194_100BASE_FX_MODE_EN;
        rc = bcm54194_rdb_write(bus_id, phy_addr, rdb_offset, reg_val);
        if (rc != PASSED) {
    	    printf("Failed to write GE PHY, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n", phy_addr, rdb_offset, reg_val);
            return (rc);
        }

        /* Select port */
        rdb_offset = BCM54194_SGMII_LN_CTRL_1G_REG;
        rc = bcm54194_rdb_read(bus_id, phy_addr, rdb_offset, &reg_val);
        if (rc != PASSED) {
            printf("Failed to read GE PHY, phy addr:%d, RDB offset:0x%x\n", phy_addr, rdb_offset);
            return (rc);
        }
        reg_val &= ~(BCM54194_SERFES_PORT_SEL_MASK);
        if (phy_addr == ge_port_mapping_phy_addr[0]) {
            reg_val |= BCM54194_SERFES_PORT0_FIBER;
        } else {
            reg_val |= BCM54194_SERFES_PORT1_FIBER;
        }
        rc = bcm54194_rdb_write(bus_id, phy_addr, rdb_offset, reg_val);
        if (rc != PASSED) {
    	    printf("Failed to write GE PHY, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n", phy_addr, rdb_offset, reg_val);
            return (rc);
        }

        /* Set rise/fall time to 142 ns */
        phy_addr = ge_port_mapping_phy_addr[0];
        rdb_offset = BCM54194_SGMII_TX_ACTRL_2_REG;
        rc = bcm54194_rdb_read(bus_id, phy_addr, rdb_offset, &reg_val);
        if (rc != PASSED) {
            printf("Failed to read GE PHY, phy addr:%d, RDB offset:0x%x\n", phy_addr, rdb_offset);
            return (rc);
        }
        reg_val &= ~(BCM54194_SERDES_RISE_FALL_MASK);
        reg_val |= BCM54194_SERDES_RISE_FALL_142NS;
        rc = bcm54194_rdb_write(bus_id, phy_addr, rdb_offset, reg_val);
        if (rc != PASSED) {
    	    printf("Failed to write GE PHY, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n", phy_addr, rdb_offset, reg_val);
            return (rc);
        }

        /* Set signal detect threshold = 100 mV */
        rdb_offset = BCM54194_SGMII_RX_ACTRL_5_REG;
        rc = bcm54194_rdb_read(bus_id, phy_addr, rdb_offset, &reg_val);
        if (rc != PASSED) {
            printf("Failed to read GE PHY, phy addr:%d, RDB offset:0x%x\n", phy_addr, rdb_offset);
            return (rc);
        }
        reg_val &= ~(BCM54194_SD_THRESHOLD_MASK);
        reg_val |= BCM54194_SD_THRESHOLD_100mV;
        rc = bcm54194_rdb_write(bus_id, phy_addr, rdb_offset, reg_val);
        if (rc != PASSED) {
    	    printf("Failed to write GE PHY, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n", phy_addr, rdb_offset, reg_val);
            return (rc);
        }
    } else {
        bcm54194_reset();
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
 * Input: phy_addr
 *
 * Return: PASSED/FAILED
 */
int bcm54194_sgmii_slave_mode(int phy_addr, int enable)
{
    uint16_t reg_val;
    int rc, bus_id = SMI_BUS_0, rdb_offset, regnum;

    if (enable) {
        /* Disable Copper/Fiber Auto-detection */
        rdb_offset = BCM54194_AUTO_DETECT_MEDIUM_REG;
        rc = bcm54194_rdb_read(bus_id, phy_addr, rdb_offset, &reg_val);
        if (rc != PASSED) {
            printf("Failed to read GE PHY, phy addr:%d, RDB offset:0x%x\n",
                    phy_addr, rdb_offset);
            return (rc);
        }
        reg_val &= ~(BCM54194_AUTO_DET_MEDIUM_EN_BIT);
        rc = bcm54194_rdb_write(bus_id, phy_addr, rdb_offset, reg_val);
        if (rc != PASSED) {
    	    printf("Failed to write GE PHY, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n",
                    phy_addr, rdb_offset, reg_val);
            return (rc);
        }

        /* Enable Fiber mode and access Copper register */
        rdb_offset = BCM54194_MODE_CTRL_REG;
        rc = bcm54194_rdb_read(bus_id, phy_addr, rdb_offset, &reg_val);
        if (rc != PASSED) {
            printf("Failed to read GE PHY, phy addr:%d, RDB offset:0x%x\n", phy_addr, rdb_offset);
            return (rc);
        }
        reg_val &= ~(BCM54194_MODE_SEL_MASK | BCM54194_REG_1000X_EN_BIT);
        reg_val |= BCM54194_SGMII_TO_FIBER_MODE;
        rc = bcm54194_rdb_write(bus_id, phy_addr, rdb_offset, reg_val);
        if (rc != PASSED) {
    	    printf("Failed to write GE PHY, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n", phy_addr, rdb_offset, reg_val);
            return (rc);
        }

        /* Power down the Copper interface */
        regnum = BCM54194_CTRL_REG;
        reg_val = cvmx_mdio_read(bus_id, phy_addr, regnum);
        if (reg_val < 0) {
            printf("Failed to read GE PHY, phy_addr:0x%x, reg:0x%x\n", phy_addr, regnum);
            return (rc);
        }
        reg_val |= BCM54194_POWER_DOWN_BIT;
        rc = cvmx_mdio_write(bus_id, phy_addr, regnum, reg_val);
        if (rc < 0) {
            printf("Failed to write GE PHY, phy_addr:0x%x, 0x%x\n", phy_addr, regnum);
            return (rc);
        }

        /* Enable SGMII-Slave mode */
        rdb_offset = BCM54194_SGMII_SLAVE_REG;
        rc = bcm54194_rdb_read(bus_id, phy_addr, rdb_offset, &reg_val);
        if (rc != PASSED) {
            printf("Failed to read GE PHY, phy addr:%d, RDB offset:0x%x\n", phy_addr, rdb_offset);
            return (rc);
        }
        reg_val |= BCM54194_SGMII_SLAVE_MODE_EN_BIT;
        rc = bcm54194_rdb_write(bus_id, phy_addr, rdb_offset, reg_val);
        if (rc != PASSED) {
    	    printf("Failed to write GE PHY, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n", phy_addr, rdb_offset, reg_val);
            return (rc);
        }
    } else {
        /* Enable Copper/Fiber Auto-detection */
        rdb_offset = BCM54194_AUTO_DETECT_MEDIUM_REG;
        rc = bcm54194_rdb_read(bus_id, phy_addr, rdb_offset, &reg_val);
        if (rc != PASSED) {
            printf("Failed to read GE PHY, phy addr:%d, RDB offset:0x%x\n",
                    phy_addr, rdb_offset);
            return (rc);
        }
        reg_val |= BCM54194_AUTO_DET_MEDIUM_EN_BIT;
        rc = bcm54194_rdb_write(bus_id, phy_addr, rdb_offset, reg_val);
        if (rc != PASSED) {
    	    printf("Failed to write GE PHY, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n",
                    phy_addr, rdb_offset, reg_val);
            return (rc);
        }

        /* Enable Fiber mode and access Copper register */
        rdb_offset = BCM54194_MODE_CTRL_REG;
        rc = bcm54194_rdb_read(bus_id, phy_addr, rdb_offset, &reg_val);
        if (rc != PASSED) {
            printf("Failed to read GE PHY, phy addr:%d, RDB offset:0x%x\n", phy_addr, rdb_offset);
            return (rc);
        }
        reg_val &= ~(BCM54194_MODE_SEL_MASK | BCM54194_REG_1000X_EN_BIT);
        reg_val |= BCM54194_SGMII_TO_FIBER_MODE;
        rc = bcm54194_rdb_write(bus_id, phy_addr, rdb_offset, reg_val);
        if (rc != PASSED) {
    	    printf("Failed to write GE PHY, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n", phy_addr, rdb_offset, reg_val);
            return (rc);
        }

        /* Power up the Copper interface */
        regnum = BCM54194_CTRL_REG;
        reg_val = cvmx_mdio_read(bus_id, phy_addr, regnum);
        if (reg_val < 0) {
            printf("Failed to read GE PHY, phy_addr:0x%x, reg:0x%x\n", phy_addr, regnum);
            return (rc);
        }
        reg_val &= ~(BCM54194_POWER_DOWN_BIT);
        rc = cvmx_mdio_write(bus_id, phy_addr, regnum, reg_val);
        if (rc < 0) {
            printf("Failed to write GE PHY, phy_addr:0x%x, 0x%x\n", phy_addr, regnum);
            return (rc);
        }

        /* Disable SGMII-Slave mode */
        rdb_offset = BCM54194_SGMII_SLAVE_REG;
        rc = bcm54194_rdb_read(bus_id, phy_addr, rdb_offset, &reg_val);
        if (rc != PASSED) {
            printf("Failed to read GE PHY, phy addr:%d, RDB offset:0x%x\n", phy_addr, rdb_offset);
            return (rc);
        }
        reg_val &= ~(BCM54194_SGMII_SLAVE_MODE_EN_BIT);
        rc = bcm54194_rdb_write(bus_id, phy_addr, rdb_offset, reg_val);
        if (rc != PASSED) {
    	    printf("Failed to write GE PHY, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n", phy_addr, rdb_offset, reg_val);
            return (rc);
        }
    }

    return (rc);
}

int dump_bcm54194_loopback_config(int phy_addr, int loopback_mode)
{
    uint16_t reg_val;
    int rc, bus_id = SMI_BUS_0, rdb_offset, regnum;

    switch(loopback_mode)
    {
        case GE_PHY_INT_LPBK:
            /* Enable loopback mode. */
            rdb_offset = BCM54194_COPPER_AUXILIARY_CTRL_REG;
            rc = bcm54194_rdb_read(bus_id, phy_addr, rdb_offset, &reg_val);
            if (rc != PASSED) {
            	printf("Failed to write GE PHY, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n", phy_addr, rdb_offset, reg_val);
                return (rc);
            }
            printf("rdb_reg %#.2x = %#.4x\n", rdb_offset, reg_val);

            /* Enable loopback mode without loopback plug. */
            rdb_offset = 0x2C;
            rc = bcm54194_rdb_read(bus_id, phy_addr, rdb_offset, &reg_val);
            if (rc != PASSED) {
            	printf("Failed to write GE PHY, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n", phy_addr, rdb_offset, reg_val);
                return (rc);
            }
            printf("rdb_reg %#.2x = %#.4x\n", rdb_offset, reg_val);
            
            regnum = BCM54195_1000BASE_CTRL_REG;
            reg_val = cvmx_mdio_read(bus_id, phy_addr, regnum);
            printf("reg %#.2x = %#.4x\n", regnum, reg_val);

            regnum = BCM54194_CTRL_REG;
            reg_val = cvmx_mdio_read(bus_id, phy_addr, regnum);
            printf("reg %#.2x = %#.4x\n", regnum, reg_val);
            
            rdb_offset = BCM54194_AUTO_DETECT_MEDIUM_REG;
            rc = bcm54194_rdb_read(bus_id, phy_addr, rdb_offset, &reg_val);
            if (rc != PASSED) {
            	printf("Failed to write GE PHY, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n", phy_addr, rdb_offset, reg_val);
                return (rc);
            }
            printf("rdb_reg %#.2x = %#.4x\n", rdb_offset, reg_val);
            break;

        case GE_PHY_EXT_LPBK:
        case PTP_SGMII_EXT_LPBK:
        	/* Enable loopback mode with loopback plug. */
        	rdb_offset = BCM54194_COPPER_AUXILIARY_CTRL_REG;
            rc = bcm54194_rdb_read(bus_id, phy_addr, rdb_offset, &reg_val);
            if (rc != PASSED) {
            	printf("Failed to write GE PHY, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n", phy_addr, rdb_offset, reg_val);
                return (rc);
            }
            printf("rdb_reg %#.2x = %#.4x\n", rdb_offset, reg_val);

            regnum = BCM54195_1000BASE_CTRL_REG;
            reg_val = cvmx_mdio_read(bus_id, phy_addr, regnum);
            printf("reg %#.2x = %#.4x\n", regnum, reg_val);

            regnum = BCM54194_CTRL_REG;
            reg_val = cvmx_mdio_read(bus_id, phy_addr, regnum);
            printf("reg %#.2x = %#.4x\n", regnum, reg_val);
            
            rdb_offset = BCM54194_AUTO_DETECT_MEDIUM_REG;
            rc = bcm54194_rdb_read(bus_id, phy_addr, rdb_offset, &reg_val);
            if (rc != PASSED) {
            	printf("Failed to write GE PHY, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n", phy_addr, rdb_offset, reg_val);
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
 *
 * Return: PASSED/FAILED
 */
int bcm54194_config_loopback(int phy_addr, int speed, bcm54194_intf_t intf, int loopback_mode, int enable)
{
    uint16_t reg_val;
    int rc, bus_id = SMI_BUS_0, rdb_offset, regnum, duplex = FULL_DUPLEX;

    /* Switch to SGMII/Copper/Fiber register space */
    //bcm54194_switch_intf_access(intf);

    switch(loopback_mode)
    {
        case GE_PHY_SGMII_LPBK:// need to check link status of Fiber register space

            if (enable) {
                bcm54194_sig_pwr_ctrl(phy_addr, TRUE, BCM54194_FIBER_INTF);
            }

            /* Select 1000BASE-T register space. */
            bcm54194_reg_1000x_en(phy_addr, enable);

        	/* Force link when in 10Mbps or 100Mbps mode. Not needed for 1000Mbps mode. */
            if (speed != SPD_1000MBPS) {
        	    rdb_offset = BCM54194_TEST_1_REG;
                rc = bcm54194_rdb_read(bus_id, phy_addr, rdb_offset, &reg_val);
                if (rc != PASSED) {
                    printf("Failed to read GE PHY, phy addr:%d, RDB offset:0x%x\n", phy_addr, rdb_offset);
                    return (rc);
                }
                if (enable) {
                    reg_val |= BCM54194_FORCE_LINK_BIT;
                } else {
                    reg_val &= ~BCM54194_FORCE_LINK_BIT;
                }
                rc = bcm54194_rdb_write(bus_id, phy_addr, rdb_offset, reg_val);
                if (rc != PASSED) {
                	printf("Failed to write GE PHY, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n", phy_addr, rdb_offset, reg_val);
                    return (rc);
                }
            }

            /* Enable SGMII internal loopback */
            regnum = BCM54194_CTRL_REG;
            reg_val = cvmx_mdio_read(bus_id, phy_addr, regnum);
            if (reg_val < 0) {
                printf("Failed to read GE PHY, phy_addr:0x%x, reg:0x%x\n", phy_addr, regnum);
                return (rc);
            }
            if (enable) {
                reg_val |= BCM54194_INTERNAL_LOOPBACK;
            } else {
                reg_val &= ~(BCM54194_INTERNAL_LOOPBACK);
            }
            rc = cvmx_mdio_write(bus_id, phy_addr, regnum, reg_val);
            if (rc != PASSED) {
                printf("Failed to write GE PHY, phy_addr:0x%x, reg:0x%x\n", phy_addr, regnum);
                return (rc);
            }

            rc = bcm54194_cfg_setting(phy_addr, speed, AUTONEG_ON, duplex, intf);
            if (rc != PASSED) {
                printf("Failed to config PHY setting. phy addr:0x%x\n", phy_addr);
            	return (rc);
            }

            break;

        case GE_PHY_INT_LPBK:
            /* Enable loopback mode. */
            rdb_offset = BCM54194_COPPER_AUXILIARY_CTRL_REG;
            if (enable) {
                reg_val = (0x8400);
            } else {
                reg_val = (0x430);
            }
            rc = bcm54194_rdb_write(bus_id, phy_addr, rdb_offset, reg_val);
            if (rc != PASSED) {
            	printf("Failed to write GE PHY, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n", phy_addr, rdb_offset, reg_val);
                return (rc);
            }

            /* Enable loopback mode without loopback plug. */
            rdb_offset = 0x2C;
            if (enable) {
                reg_val = (0x4014);
            } else {
                reg_val = (0x4004);
            }
            rc = bcm54194_rdb_write(bus_id, phy_addr, rdb_offset, reg_val);
            if (rc != PASSED) {
            	printf("Failed to write GE PHY, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n", phy_addr, rdb_offset, reg_val);
                return (rc);
            }

            /* Enable 1000BASE-T Master mode */
            if (speed == SPD_1000MBPS) {
                regnum = BCM54195_1000BASE_CTRL_REG;
                if (enable) {
                	reg_val = (0x1800);
                } else {
                	reg_val = (0xF00);
                }
                rc = cvmx_mdio_write(bus_id, phy_addr, regnum, reg_val);
                if (rc != PASSED) {
                    printf("Failed to write GE PHY, phy_addr:0x%x, 0x%x\n", phy_addr, regnum);
                }
            }

            /* Power Down Copper Interface.
             * When power up copper interface,
             * the COPPER_AN_ENABLE bit[12] will reset to 1
             */
            if (enable) {
            	bcm54194_sig_pwr_ctrl(phy_addr, FALSE, intf);
            	msleep(10);
            	bcm54194_sig_pwr_ctrl(phy_addr, TRUE, intf);
            }

            rc = bcm54194_cfg_setting(phy_addr, speed, AUTONEG_OFF, duplex, intf);
            if (rc != PASSED) {
            	printf("Failed to config PHY setting. phy addr:0x%x\n", phy_addr);
            	return (rc);
            }
            
            /* To exit the Copper Loopback without Loopback Plug,
             * Broadcom recommends a software or hardware reset. */
            if (!enable) {
                bcm54194_reset();
            }

            break;

        case GE_PHY_EXT_LPBK:
        case PTP_SGMII_EXT_LPBK:
        	/* Enable loopback mode with loopback plug. */
        	rdb_offset = BCM54194_COPPER_AUXILIARY_CTRL_REG;
            if (enable) {
                reg_val = (0x8400);
            } else {
                reg_val = (0x430);
            }
            rc = bcm54194_rdb_write(bus_id, phy_addr, rdb_offset, reg_val);
            if (rc != PASSED) {
            	printf("Failed to write GE PHY, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n", phy_addr, rdb_offset, reg_val);
                return (rc);
            }

            /* Enable 1000BASE-T Master mode */
            if (speed == SPD_1000MBPS) {
                regnum = BCM54195_1000BASE_CTRL_REG;
            if (enable) {
                	reg_val = (0x1800);
            } else {
                	reg_val = (0xF00);
            }
            rc = cvmx_mdio_write(bus_id, phy_addr, regnum, reg_val);
            if (rc != PASSED) {
                    printf("Failed to write GE PHY, phy_addr:0x%x, 0x%x\n", phy_addr, regnum);
                }
            }

            /* Power Down Copper Interface.
             * When power up copper interface,
             * the COPPER_AN_ENABLE bit[12] will reset to 1
             */
            if (enable) {
            	bcm54194_sig_pwr_ctrl(phy_addr, FALSE, intf);
            	msleep(10);
            	bcm54194_sig_pwr_ctrl(phy_addr, TRUE, intf);
            }

            rc = bcm54194_cfg_setting(phy_addr, speed, AUTONEG_OFF, duplex, intf);
            if (rc != PASSED) {
            	printf("Failed to config PHY setting. phy addr:0x%x\n", phy_addr);
            	return (rc);
            }

            /* To exit the Copper Loopback with Loopback Plug,
             * Broadcom recommends a software or hardware reset. */
            if (!enable) {
                bcm54194_reset();
            }

            break;

        case GE_PHY_SFP_EXT_LPBK:
            if (speed == SPD_100MBPS) {
                if (is_glc_ge_100fx) {
                    bcm54194_sgmii_slave_mode(phy_addr, enable);
                } else {
                    bcm54194_100base_fx_config(phy_addr, enable);
                }
            } else {
                /* From BCM FAE:
                 * No phy configuration is needed for fiber SFP loopback with a plug.
                 * Just plug fiber tx => rx loopback plug in SFP.
                 */
                if (enable) {
            	    /* Select 1000BASE-T register space and set speed
            	     * to make SFP link up.
            	     */
                    rc = bcm54194_cfg_setting(phy_addr, speed, AUTONEG_ON, duplex, intf);
                    if (rc != PASSED) {
                	    printf("Failed to config PHY setting. phy addr:0x%x\n", phy_addr);
                	    return (rc);
                    }
                } else {
                    /* Restore to Copper register space. */
                    bcm54194_reg_1000x_en(phy_addr, enable);
                }
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
 *
 * Return: PASSED/FAILED
 */
int bcm54194_cfg_setting(int phy_addr, int speed, int auto_neg, int duplex, bcm54194_intf_t intf)
{
    int rc, regnum = BCM54194_CTRL_REG, bus_id = SMI_BUS_0;
    ushort reg_val;

    /* Switch to SGMII/Copper/Fiber register space */
    if (intf == BCM54194_FIBER_INTF) {
    	bcm54194_reg_1000x_en(phy_addr, TRUE);
    } else {
        bcm54194_reg_1000x_en(phy_addr, FALSE);
    }

    reg_val = cvmx_mdio_read(bus_id, phy_addr, regnum);
    if (reg_val < 0) {
        printf("Failed to read GE PHY, phy_addr:0x%x, 0x%x\n", phy_addr, regnum);
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

    rc = cvmx_mdio_write(bus_id, phy_addr, regnum, reg_val);
    if (rc < 0) {
        printf("Failed to write GE PHY, phy_addr:0x%x, 0x%x\n", phy_addr, regnum);
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
 * Input: phy_addr -
 *        intf - SGMII/Copper/Fiber
 *
 * Return: true/false
 */
boolean bcm54194_is_linkup(int phy_addr, bcm54194_intf_t intf)
{
    int bus_id = SMI_BUS_0, regnum = BCM54194_STAT_REG;
    int repeat = 10000;
    ushort reg_val;

    /* Switch to SGMII/Copper/Fiber register space */
    if (intf == BCM54194_FIBER_INTF) {
    	bcm54194_reg_1000x_en(phy_addr, TRUE);
    } else {
        bcm54194_reg_1000x_en(phy_addr, FALSE);
    }

    do {
        msleep(10);
        reg_val = cvmx_mdio_read(bus_id, phy_addr, regnum);
        if (reg_val & BCM54194_LINK_STAT_BIT) {
            return (TRUE);
        }
    } while (repeat-- > 0);
    return (FALSE);
}

int bcm54194_sig_pwr_ctrl(int phy_addr, boolean enable, bcm54194_intf_t intf)
{
    int rc, regnum = BCM54194_CTRL_REG, bus_id = SMI_BUS_0;
    ushort reg_val;

    /* Switch to SGMII/Copper/Fiber register space */
    if (intf == BCM54194_FIBER_INTF) {
    	bcm54194_reg_1000x_en(phy_addr, TRUE);
    } else {
        bcm54194_reg_1000x_en(phy_addr, FALSE);
    }

    reg_val = cvmx_mdio_read(bus_id, phy_addr, regnum);
    if (reg_val < 0) {
        printf("Failed to read GE PHY, phy_addr:0x%x, 0x%x\n", phy_addr, regnum);
        return (FAILED);
    }

    if (enable) {
    	reg_val &= ~(BCM54194_POWER_DOWN_BIT);
    } else {
        reg_val |= BCM54194_POWER_DOWN_BIT;
    }

    rc = cvmx_mdio_write(bus_id, phy_addr, regnum, reg_val);
    if (rc < 0) {
        printf("Failed to write GE PHY, phy_addr:0x%x, 0x%x\n", phy_addr, regnum);
    }

    return (rc);
}

/*
 * Function: bcm54194_mdio45_reg_rd
 *
 * Description:
 * Read Broadcom 54194 PHY register. Use Cavium MDIO bus access directly.
 * 
 * Input:
 * port - The MII phy id
 * dev - MMD
 * reg - Register to read
 *
 * Return: read_value/FAILED
 */
int bcm54194_mdio45_reg_rd(int bus_id, int phy_addr, int dev, int reg)
{
    int mii_value = 0x0;

    mii_value = cvmx_mdio_45_read(bus_id, phy_addr, dev, reg);
    if (mii_value == -1) {
        printf("Read error from phy %d reg %d.%x)\n", phy_addr, dev, reg);
        return FAILED;
    } else {
#ifdef DEBUG
        printf("phy %#.2x reg %d.%#.4x = %#.4x\n", phy_addr, dev, reg, mii_value);
#endif
    }
    return (mii_value);
}

/*
 * Function: bcm54194_mdio45_reg_wr
 *
 * Description:
 * Write Broadcom 54194 PHY register. Use Cavium MDIO bus access directly.
 * 
 * Input:
 * port - The MII phy id
 * dev - MMD
 * reg - Register to write
 * val - value to write
 *
 * Return: PASSED/FAILED
 */
int bcm54194_mdio45_reg_wr(int bus_id, int phy_addr, int dev, int reg, int val)
{
    int status;

    status = cvmx_mdio_45_write(bus_id, phy_addr, dev, reg, val);
    if (status == -1) {
        printf("Write error from phy %d reg %d.%x)\n", phy_addr, dev, reg);
        return (FAILED);
    } else {
#ifdef DEBUG
        printf("phy %#.2x reg %d.%#.4x = 0x%04%x\n", phy_addr, dev, reg, mii_value);
#endif
    }
    return (PASSED);
}

/*------------------------------------------------------------------
 *
 * Function: init_bcm54194_macsec
 *
 * Description:
 * This is a sample script for Encryption and decryption on packets.
 * The TCAM Secure Association table is setup ony to look at MAC SA =
 * BBBBBBBBBBBBh for decision making;
 * For encryption send plaintext packets with enough IPG to support
 * 32byte MACsec overhead addition
 * Input:  phy_addr - phy address
 *
 * Output: NONE
 *
 *------------------------------------------------------------------
 */
void init_bcm54194_macsec(uint phy_addr)
{
    int bus_id = SMI_BUS_0;
    int mspu_sector = 0x00;

    /******************************
     * BCM54194 top configuration 
     ******************************/

    /* Configure datapath select to bring 1588 close to Line 
     * and enabling MSPU (MacSec) path */
    bcm54194_mdio45_reg_wr(bus_id, phy_addr, 7, 0x984D, 0x40FF);
    bcm54194_rdb_write(bus_id, phy_addr, 0x084A, 0x0);

    /* Power up all MSPU's by enabling clock */
    bcm54194_rdb_write(bus_id, phy_addr, 0x084B, 0x8000);
    
    /* Enable MSPU Switch MAC to take SW side speed setting */
    bcm54194_mdio45_reg_wr(bus_id, phy_addr, 7, 0x9870, 0x00FF);

    /* Set MSPU SW side speed setting to 1G */
    bcm54194_mdio45_reg_wr(bus_id, phy_addr, 7, 0x9871, 0xAAAA);

    /******************************
     * MSPU configuration: UniMAC
     ******************************/
    /* Setting Line side and Switch side UniMAC 
     * Enable RX and TX, ETH_SPEED = 1G, 
     * enable RX and TX SW programmed pause capability*/
    bcm54194_mspu_write(mspu_sector, 0x7808, 0x0146009B);
    bcm54194_mspu_write(mspu_sector, 0x7C08, 0x0146009B);   

    /* Setting TX IPG length to be 8 on Line side */
    bcm54194_mspu_write(mspu_sector, 0x785C, 0x00000008);
    bcm54194_mspu_write(mspu_sector, 0x7C5C, 0x00000008);   

    /* Enable TX CRC corruption when system signals
     * corrupt CRC on Line and Switch side */
    bcm54194_mspu_write(mspu_sector, 0x7B14, 0x00000002);
    bcm54194_mspu_write(mspu_sector, 0x7F14, 0x00000002);   

    /* Setting Frame length to support Jumbo packets */
    bcm54194_mspu_write(mspu_sector, 0x7814, 0x00003FFF);
    bcm54194_mspu_write(mspu_sector, 0x7C14, 0x00003FFF);   

    /* Disable MAC statistics clear on read */ 
    //printf("mfix: script typo? %d\n", __LINE__);
    bcm54194_mspu_write(mspu_sector, 0x0000, 0x00000002);

    /**************************************
     * MSPU configuration: EIP165 generic
     **************************************/
    /* SAF/cut-through mode egress 
     * enabling SAF mode */
    /* 0xAAAA ---> sector 3 (egress, ===> 0x2FE00) */
    mspu_sector = 0xAAAA;
    //printf("mfix: script typo? %d\n", __LINE__);
    bcm54194_mspu_write(mspu_sector, 0xFE00, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0xFE04, 0x06000400);   

    /* SAF/cut-through mode ingress 
     * enabling SAF mode */
    /* 0x5555 ---> sector 2 (ingress, ===> 0x1FE00) */
    mspu_sector = 0x5555;
    bcm54194_mspu_write(mspu_sector, 0xFE00, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0xFE04, 0x06000400);   

    /**********************************************
     * MSPU configuration: 
     * Encryption and Decryption configuration
     **********************************************/
    /* Encryption configuration */
    // EIP165.IgEIP160.OutPostProc: CC_Configure
    //printf("mfix: script typo(mspu_sector is 0x08 or 0x00)? %d\n", __LINE__);
    mspu_sector = 0x5555;
    bcm54194_mspu_write(mspu_sector, 0xE844, 0x05FF0000);
    bcm54194_mspu_write(mspu_sector, 0xE840, 0x0000C000);   

    // EIP165.IgEIP160.StatTCAM: DefaultMode
    bcm54194_mspu_write(mspu_sector, 0x4410, 0x0000000C);

    // EIP165.IgEIP160.StatRXCAM: DefaultMode
    bcm54194_mspu_write(mspu_sector, 0x5410, 0x0000000C);

    // EIP165.IgEIP160.StatSA: DefaultMode
    bcm54194_mspu_write(mspu_sector, 0xE210, 0x0000000C);

    // EIP165.IgEIP160.StatSECY: DefaultMode
    bcm54194_mspu_write(mspu_sector, 0xE410, 0x0000000C);

    // EIP165.IgEIP160.StatIFC: DefaultMode
    bcm54194_mspu_write(mspu_sector, 0xE610, 0x0000000C);

    // Program PE transform err detection
    bcm54194_mspu_write(mspu_sector, 0xF124, 0x80FE0000);

    // EIP165.IgEIP160.Flow: Configure
    // 'context_size': 2(0x2)
    bcm54194_mspu_write(mspu_sector, 0x797C, 0x02000000);

    // Program packet engine to fetch only 
    // 5 x 128-bit words of the context
    bcm54194_mspu_write(mspu_sector, 0xF408, 0xE5880214);

    // Program PE to disabled context update for externally bad packets
    // Program PE to perform a strict comparison for packet number
    // EIP165.IgEIP160.EIP62: Configure
    // 'ctx_upd_ctrl': 3(0x3)
    // 'pn_thr_mode': 1(0x1)
    bcm54194_mspu_write(mspu_sector, 0xF408, 0xE5880614);
    bcm54194_mspu_write(mspu_sector, 0xF430, 0x00000003);

    // EIP165.EgEIP160: DefaultMode
    // EIP165.EgEIP160.StatTCAM: DefaultMode
    mspu_sector = 0xAAAA;
    bcm54194_mspu_write(mspu_sector, 0x4410, 0x0000000C);

    // EIP165.EgEIP160.StatSA: DefaultMode
    bcm54194_mspu_write(mspu_sector, 0xE210, 0x0000000C);

    // EIP165.EgEIP160.StatSECY: DefaultMode
    bcm54194_mspu_write(mspu_sector, 0xE410, 0x0000000C);

    // EIP165.EgEIP160.StatIFC: DefaultMode
    bcm54194_mspu_write(mspu_sector, 0xE610, 0x0000000C);

    // Program PE transform err detection
    bcm54194_mspu_write(mspu_sector, 0xF124, 0x80FE0000);

    // EIP165.EgEIP160.Flow: Configure
    // 'context_size': 2(0x2)
    bcm54194_mspu_write(mspu_sector, 0x797C, 0x02000000);

    // Program packet engine to fetch only
    // 6 x 128-bit words of the context
    bcm54194_mspu_write(mspu_sector, 0xF408, 0xE5880218);

    // Program PE to disabled context update for externally bad packets
    // Program PE to perform a strict comparison for packet number
    // EIP165.EgEIP160.EIP62: Configure
    // 'ctx_upd_ctrl': 3(0x3)
    // 'pn_thr_mode': 1(0x1)
    bcm54194_mspu_write(mspu_sector, 0xF408, 0xE5880618);
    bcm54194_mspu_write(mspu_sector, 0xF430, 0x00000003);

    // Program EIP-160 latency compensation FIFO
    // EIP165.EgEIP160.Port: Set
    // 'non_vlan_mtu_check': 32760(0x7ff8)
    // 'non_vlan_mtu_check_drop': 1(0x1)
    bcm54194_mspu_write(mspu_sector, 0xF200, 0x0000FFF8);
    bcm54194_mspu_write(mspu_sector, 0xF204, 0x0000FFF8);
    bcm54194_mspu_write(mspu_sector, 0xF208, 0x0000FFF8);
    bcm54194_mspu_write(mspu_sector, 0xF20C, 0x0000FFF8);
    bcm54194_mspu_write(mspu_sector, 0xF210, 0x0000FFF8);
    bcm54194_mspu_write(mspu_sector, 0xF214, 0x0000FFF8);
    bcm54194_mspu_write(mspu_sector, 0xF218, 0x0000FFF8);
    bcm54194_mspu_write(mspu_sector, 0xF21C, 0x0000FFF8);
    bcm54194_mspu_write(mspu_sector, 0xF220, 0x0000FFF8);
    bcm54194_mspu_write(mspu_sector, 0xF224, 0x0000FFF8);
    bcm54194_mspu_write(mspu_sector, 0xF228, 0x0000FFF8);
    bcm54194_mspu_write(mspu_sector, 0xF22C, 0x0000FFF8);
    bcm54194_mspu_write(mspu_sector, 0xF230, 0x0000FFF8);
    bcm54194_mspu_write(mspu_sector, 0xF234, 0x0000FFF8);
    bcm54194_mspu_write(mspu_sector, 0xF238, 0x0000FFF8);
    bcm54194_mspu_write(mspu_sector, 0xF23C, 0x0000FFF8);

    // PICS-CSA-2/3/4: Cipher suite
    // Set Header bypass length to 0 bytes
    // EIP165.EgEIP160.Flow: Configure
    // 'hb_size': 0(0x0)
    // Installing flows to exercise confidentialityOffset
    // EIP165.EgEIP160.Flow: FlowCreate(Descr='MACsec out, confOffset=0 bytes')
    // Configure array of size 96 to address starting &H10000
    bcm54194_mspu_write(mspu_sector, 0x0000, 0x924bc066);
    bcm54194_mspu_write(mspu_sector, 0x0004, 0x00c8a707);
    bcm54194_mspu_write(mspu_sector, 0x0008, 0x651ead73);
    bcm54194_mspu_write(mspu_sector, 0x000C, 0x37f20a35);
    bcm54194_mspu_write(mspu_sector, 0x0010, 0x8b603225);
    bcm54194_mspu_write(mspu_sector, 0x0014, 0x25fbfd26);
    bcm54194_mspu_write(mspu_sector, 0x0018, 0x1e63dc20);
    bcm54194_mspu_write(mspu_sector, 0x001C, 0x23d96efc);
    bcm54194_mspu_write(mspu_sector, 0x0020, 0x9bb538a3);
    bcm54194_mspu_write(mspu_sector, 0x0024, 0x72445d05);
    bcm54194_mspu_write(mspu_sector, 0x0028, 0x00000007);
    bcm54194_mspu_write(mspu_sector, 0x002C, 0x5b6ec642);
    bcm54194_mspu_write(mspu_sector, 0x0030, 0xe7b3dd10);
    bcm54194_mspu_write(mspu_sector, 0x0034, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x0038, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x003C, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x0040, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x0044, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x0048, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x004C, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x0050, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x0054, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x0058, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x005C, 0x00000000);

    // WriteBlock to 65632 completed
    // Configure array of size 8 to address starting &H17000
    bcm54194_mspu_write(mspu_sector, 0x7000, 0x00270003);
    bcm54194_mspu_write(mspu_sector, 0x7004, 0x000c0c00);

    // WriteBlock to 94216 completed
    // Configure array of size 4 to address starting &H13800
    bcm54194_mspu_write(mspu_sector, 0x3800, 0x0000c000);

    // WriteBlock to 79876 completed
    // Writing the match rules for RULE0
    // Packet type is not specified for the rules (untagged&, tagged etc.). 
    // Enabling all of them
    // Configure array of size 64 to address starting &H12000
    bcm54194_mspu_write(mspu_sector, 0x2000, 0x00002003);
    bcm54194_mspu_write(mspu_sector, 0x2004, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x2008, 0xc2d80000);
    bcm54194_mspu_write(mspu_sector, 0x200C, 0x6782426b);
    bcm54194_mspu_write(mspu_sector, 0x2008, 0xbbbb0000);
    bcm54194_mspu_write(mspu_sector, 0x200C, 0xbbbbbbbb);
    bcm54194_mspu_write(mspu_sector, 0x2010, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x2014, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x2018, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x201C, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x2020, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x2024, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x2028, 0xffff0000);
    bcm54194_mspu_write(mspu_sector, 0x202C, 0xffffffff);
    bcm54194_mspu_write(mspu_sector, 0x2030, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x2034, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x2038, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x203C, 0x00000000);

    // WriteBlock to 73792 completed
    bcm54194_mspu_write(mspu_sector, 0x3000, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x6100, 0x00004000);

    // EIP165.EgEIP160.Flow:
    // FlowCreate(Descr='MACsec out, confOffset=4 bytes')
    // Configure array of size 96 to address starting &H10080
    bcm54194_mspu_write(mspu_sector, 0x0080, 0x924bc066);
    bcm54194_mspu_write(mspu_sector, 0x0084, 0x00de6fed);
    bcm54194_mspu_write(mspu_sector, 0x0088, 0x630c96f0);
    bcm54194_mspu_write(mspu_sector, 0x008C, 0x0e2f153b);
    bcm54194_mspu_write(mspu_sector, 0x0090, 0x9c9c2ca3);
    bcm54194_mspu_write(mspu_sector, 0x0094, 0xe04883b4);
    bcm54194_mspu_write(mspu_sector, 0x0098, 0x07a4a51a);
    bcm54194_mspu_write(mspu_sector, 0x009C, 0xa1e8bc48);
    bcm54194_mspu_write(mspu_sector, 0x00A0, 0x4f1e5a44);
    bcm54194_mspu_write(mspu_sector, 0x00A4, 0xaa7af0b3);
    bcm54194_mspu_write(mspu_sector, 0x00A8, 0x00000007);
    bcm54194_mspu_write(mspu_sector, 0x00AC, 0x63ca71e9);
    bcm54194_mspu_write(mspu_sector, 0x00B0, 0x323863ce);
    bcm54194_mspu_write(mspu_sector, 0x00B4, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x00B8, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x00BC, 0x00010000);
    bcm54194_mspu_write(mspu_sector, 0x00C0, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x00C4, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x00C8, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x00CC, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x00D0, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x00D4, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x00D8, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x00DC, 0x00000000);

    // WriteBlock to 65760 completed
    // Configure array of size 8 to address starting &H17008
    bcm54194_mspu_write(mspu_sector, 0x7008, 0x00270003);
    bcm54194_mspu_write(mspu_sector, 0x700C, 0x040c0c00);

    // WriteBlock to 94224 completed
    // Configure array of size 4 to address starting &H13808
    bcm54194_mspu_write(mspu_sector, 0x3808, 0x0000c001);

    // WriteBlock to 79884 completed
    // Writing the match rules for RULE1
    // Packet type is not specified for the rules (untagged&, tagged etc.).
    // Enabling all of them
    // Configure array of size 64 to address starting &H12040
    bcm54194_mspu_write(mspu_sector, 0x2040, 0x00002003);
    bcm54194_mspu_write(mspu_sector, 0x2044, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x2048, 0x95d60000);
    bcm54194_mspu_write(mspu_sector, 0x204C, 0x1510ceb7);
    bcm54194_mspu_write(mspu_sector, 0x2050, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x2054, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x2058, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x205C, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x2060, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x2064, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x2068, 0xffff0000);
    bcm54194_mspu_write(mspu_sector, 0x206C, 0xffffffff);
    bcm54194_mspu_write(mspu_sector, 0x2070, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x2074, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x2078, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x207C, 0x00000000);

    // WriteBlock to 73856 completed
    bcm54194_mspu_write(mspu_sector, 0x3004, 0x00000001);
    bcm54194_mspu_write(mspu_sector, 0x6100, 0x00004001);

    /* Decryption configuration */
    // EIP165.IgEIP160.OutPostProc: CC_Configure
    mspu_sector = 0x5555;
    bcm54194_mspu_write(mspu_sector, 0xe844, 0x05ff0000);
    bcm54194_mspu_write(mspu_sector, 0xe840, 0x0000c000);

    // EIP165.IgEIP160.StatTCAM: DefaultMode
    bcm54194_mspu_write(mspu_sector, 0x4410, 0x0000000c);

    // EIP165.IgEIP160.StatRXCAM: DefaultMode
    bcm54194_mspu_write(mspu_sector, 0x5410, 0x0000000c);

    // EIP165.IgEIP160.StatSA: DefaultMode
    bcm54194_mspu_write(mspu_sector, 0xe210, 0x0000000c);

    // EIP165.IgEIP160.StatSECY: DefaultMode
    bcm54194_mspu_write(mspu_sector, 0xe410, 0x0000000c);

    // EIP165.IgEIP160.StatIFC: DefaultMode
    bcm54194_mspu_write(mspu_sector, 0xe610, 0x0000000c);

    // Program PE transform err detection
    bcm54194_mspu_write(mspu_sector, 0xF124, 0x80fe0000);

    // EIP165.IgEIP160.Flow: Configure
    // 'context_size': 2(0x2)
    bcm54194_mspu_write(mspu_sector, 0x797C, 0x02080000);

    // Program packet engine to fetch only 
    // 5 x 128-bit words of the context
    bcm54194_mspu_write(mspu_sector, 0xF408, 0xe5880218);

    // Program PE to disabled context update for externally bad packets
    // Program PE to perform a strict comparison for packet number
    // EIP165.IgEIP160.EIP62: Configure
    // 'ctx_upd_ctrl': 3(0x3)
    // 'pn_thr_mode': 1(0x1)
    bcm54194_mspu_write(mspu_sector, 0xF408, 0xe5880618);
    bcm54194_mspu_write(mspu_sector, 0xF430, 0x00000003);

    // EIP165.EgEIP160: DefaultMode
    // EIP165.EgEIP160.StatTCAM: DefaultMode
    mspu_sector = 0xAAAA;
    bcm54194_mspu_write(mspu_sector, 0x4410, 0x0000000c);

    // EIP165.EgEIP160.StatSA: DefaultMode
    bcm54194_mspu_write(mspu_sector, 0xe210, 0x0000000c);
    
    // EIP165.EgEIP160.StatSECY: DefaultMode
    bcm54194_mspu_write(mspu_sector, 0xe410, 0x0000000c);
    
    // EIP165.EgEIP160.StatIFC: DefaultMode
    bcm54194_mspu_write(mspu_sector, 0xe610, 0x0000000c);
    
    // Program PE transform err detection
    bcm54194_mspu_write(mspu_sector, 0xF124, 0x80fe0000);

    // EIP165.EgEIP160.Flow: Configure
    // 'context_size': 2(0x2)
    bcm54194_mspu_write(mspu_sector, 0x797C, 0x02080000);

    // Program packet engine to fetch only 
    // 6 x 128-bit words of the context
    bcm54194_mspu_write(mspu_sector, 0xF408, 0xe5880218);

    // Program PE to disabled context update for externally bad packets
    // Program PE to perform a strict comparison for packet number
    // EIP165.EgEIP160.EIP62: Configure
    // 'ctx_upd_ctrl': 3(0x3)
    // 'pn_thr_mode': 1(0x1)
    bcm54194_mspu_write(mspu_sector, 0xF408, 0xe5880618);
    bcm54194_mspu_write(mspu_sector, 0xF430, 0x00000003);

    // Program EIP-160 latency compensation FIFO
    // EIP165.IgEIP160.Port: Set
    // 'non_vlan_mtu_check': 32760(0x7ff8)
    // 'non_vlan_mtu_check_drop': 1(0x1)
    // EIP165.IgEIP160.Flow: Configure
    // 'hb_size': 0(0x0)
    // PICS-CSA-2/3/4: Cipher suite
    // Installing flows to exercise confidentialityOffset
    // EIP165.IgEIP160.Flow: FlowCreate(Descr='MACsec in, confOffset=0 bytes, key_len=16')
    // Configure array of size 96 to address starting &H00
    mspu_sector = 0x5555;
    //printf("mfix Swaraj edited, %d\n", __LINE__);
    bcm54194_mspu_write(mspu_sector, 0x0000, 0xd24bc06f);
    bcm54194_mspu_write(mspu_sector, 0x0004, 0x00c8a707);
    bcm54194_mspu_write(mspu_sector, 0x0008, 0x651ead73);
    bcm54194_mspu_write(mspu_sector, 0x000C, 0x37f20a35);
    bcm54194_mspu_write(mspu_sector, 0x0010, 0x8b603225);
    bcm54194_mspu_write(mspu_sector, 0x0014, 0x25fbfd26);
    bcm54194_mspu_write(mspu_sector, 0x0018, 0x1e63dc20);
    bcm54194_mspu_write(mspu_sector, 0x001C, 0x23d96efc);
    bcm54194_mspu_write(mspu_sector, 0x0020, 0x9bb538a3);
    bcm54194_mspu_write(mspu_sector, 0x0024, 0x72445d05);
    bcm54194_mspu_write(mspu_sector, 0x0028, 0x00000001);
    bcm54194_mspu_write(mspu_sector, 0x002C, 0x00000080);
    bcm54194_mspu_write(mspu_sector, 0x0030, 0x5b6ec642);
    bcm54194_mspu_write(mspu_sector, 0x0034, 0xe7b3dd10);
    bcm54194_mspu_write(mspu_sector, 0x0038, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x003C, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x0040, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x0044, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x0048, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x004C, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x0050, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x0054, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x0058, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x005C, 0x00000000);

    // WriteBlock to 96 completed
    // Configure array of size 8 to address starting &H7000
    bcm54194_mspu_write(mspu_sector, 0x7000, 0x00170002);
    bcm54194_mspu_write(mspu_sector, 0x7004, 0x00000c00);

    // WriteBlock to 28680 completed
    // Configure array of size 8 to address starting &H3800
    bcm54194_mspu_write(mspu_sector, 0x3800, 0xc000c000);
    bcm54194_mspu_write(mspu_sector, 0x3804, 0xc000c000);

    // WriteBlock to 14344 completed
    // Configure array of size 12 to address starting &H3400
    bcm54194_mspu_write(mspu_sector, 0x3400, 0x5b6ec642);
    bcm54194_mspu_write(mspu_sector, 0x3404, 0xe7b3dd10);
    bcm54194_mspu_write(mspu_sector, 0x3408, 0x00000000);

    // WriteBlock to 13324 completed
    bcm54194_mspu_write(mspu_sector, 0x3700, 0x00004000);

    // Writing the match rules for RULE0
    // Packet type is not specified for the rules (untagged&, tagged etc.).
    // Enabling all of them
    // Configure array of size 64 to address starting &H2000
    bcm54194_mspu_write(mspu_sector, 0x2000, 0x00002003);
    bcm54194_mspu_write(mspu_sector, 0x2004, 0x00000000);
    //printf("mfix: commented out by Swaraj for screening. %d\n", __LINE__);
    bcm54194_mspu_write(mspu_sector, 0x2008, 0xbbbb0000);
    bcm54194_mspu_write(mspu_sector, 0x200C, 0xbbbbbbbb);
    bcm54194_mspu_write(mspu_sector, 0x2010, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x2014, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x2018, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x201C, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x2020, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x2024, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x2028, 0xffff0000);
    bcm54194_mspu_write(mspu_sector, 0x202C, 0xffffffff);
    bcm54194_mspu_write(mspu_sector, 0x2030, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x2034, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x2038, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x203C, 0x00000000);

    // WriteBlock to 8256 completed
    bcm54194_mspu_write(mspu_sector, 0x3000, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x6100, 0x00004000);
    
    // EIP165.IgEIP160.Flow: 
    // FlowCreate(Descr='MACsec in, confOffset=4 bytes, key_len=16')
    // Configure array of size 96 to address starting &H80
    //printf("mfix: Swaraj edited. %d\n", __LINE__);
    bcm54194_mspu_write(mspu_sector, 0x0080, 0xd24bc06f);
    bcm54194_mspu_write(mspu_sector, 0x0084, 0x00de6fed);
    bcm54194_mspu_write(mspu_sector, 0x0088, 0x630c96f0);
    bcm54194_mspu_write(mspu_sector, 0x008C, 0x0e2f153b);
    bcm54194_mspu_write(mspu_sector, 0x0090, 0x9c9c2ca3);
    bcm54194_mspu_write(mspu_sector, 0x0094, 0xe04883b4);
    bcm54194_mspu_write(mspu_sector, 0x0098, 0x07a4a51a);
    bcm54194_mspu_write(mspu_sector, 0x009C, 0xa1e8bc48);
    bcm54194_mspu_write(mspu_sector, 0x00A0, 0x4f1e5a44);
    bcm54194_mspu_write(mspu_sector, 0x00A4, 0xaa7af0b3);
    bcm54194_mspu_write(mspu_sector, 0x00A8, 0x00000001);
    bcm54194_mspu_write(mspu_sector, 0x00AC, 0x00000080);
    bcm54194_mspu_write(mspu_sector, 0x00B0, 0x63ca71e9);
    bcm54194_mspu_write(mspu_sector, 0x00B4, 0x323863ce);
    bcm54194_mspu_write(mspu_sector, 0x00B8, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x00BC, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x00C0, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x00C4, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x00C8, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x00CC, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x00D0, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x00D4, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x00D8, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x00DC, 0x00000000);

    // WriteBlock to 224 completed
    // Configure array of size 8 to address starting &H7008
    bcm54194_mspu_write(mspu_sector, 0x7008, 0x00170002);
    bcm54194_mspu_write(mspu_sector, 0x700C, 0x04000C00);

    // WriteBlock to 28688 completed
    // Configure array of size 8 to address starting &H3808 
    bcm54194_mspu_write(mspu_sector, 0x3808, 0xc001c001);
    bcm54194_mspu_write(mspu_sector, 0x380C, 0xc001c001);

    // WriteBlock to 14352 completed
    // Configure array of size 12 to address starting &H3410
    bcm54194_mspu_write(mspu_sector, 0x3410, 0x63ca71e9);
    bcm54194_mspu_write(mspu_sector, 0x3414, 0x323863ce);
    bcm54194_mspu_write(mspu_sector, 0x3418, 0x00000001);

    // WriteBlock to 13340 completed
    bcm54194_mspu_write(mspu_sector, 0x3700, 0x00004001);

    // Writing the match rules for RULE1
    // Packet type is not specified for the rules (untagged&, tagged etc.).
    // Enabling all of them
    // Configure array of size 64 to address starting &H2040
    bcm54194_mspu_write(mspu_sector, 0x2040, 0x00002003);
    bcm54194_mspu_write(mspu_sector, 0x2044, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x2048, 0x95d60000);
    bcm54194_mspu_write(mspu_sector, 0x204C, 0x1510ceb7);
    bcm54194_mspu_write(mspu_sector, 0x2050, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x2054, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x2058, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x205C, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x2060, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x2064, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x2068, 0xffff0000);
    bcm54194_mspu_write(mspu_sector, 0x206C, 0xffffffff);
    bcm54194_mspu_write(mspu_sector, 0x2070, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x2074, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x2078, 0x00000000);
    bcm54194_mspu_write(mspu_sector, 0x207C, 0x00000000);
    
    // WriteBlock to 8320 completed
    bcm54194_mspu_write(mspu_sector, 0x3004, 0x00000001);
    bcm54194_mspu_write(mspu_sector, 0x6100, 0x00004001);

    return;
}

/*------------------------------------------------------------------
 *
 * Function: disable_bcm54194_macsec
 *
 * Description:
 * Input:  phy_addr - phy address
 *
 * Output: NONE
 *
 *------------------------------------------------------------------
 */
void disable_bcm54194_macsec(uint phy_addr)
{
    int bus_id = SMI_BUS_0;

    /* Power down all MSPU's */
    bcm54194_rdb_write(bus_id, phy_addr, 0x084B, 0x03FF);
}

/***********************************************************************
 *
 * Function: enable_bcm54194_i2c_access
 *
 * Description: Enable BCM54194 I2C master mode, enable SDA/SCL line de-glitch.
 *              Need to enable it before I2C access.
 * Inputs: TRUE/FALSE
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
int enable_bcm54194_i2c_access (boolean enable)
{
    int rc, bus_id = SMI_BUS_0, phy_addr = PHY_PORT0;
    ushort reg_val;
    int offset = BCM54194_I2C_MASTER_CTRL_REG;

    rc = bcm54194_rdb_read(bus_id, phy_addr, offset, &reg_val);
    if (rc < 0) {
        printf("Failed to read GE PHY, phy addr:%d, RDB offset:0x%x\n", phy_addr, offset);
        return (rc);
    }

    if (enable) {
        reg_val |= (BCM54194_I2C_MASTER_EN_BIT | BCM54194_I2C_SDA_DEGL_EN_BIT | 
                    BCM54194_I2C_SCL_DEGL_EN_BIT | BCM54194_I2C_SPD_400KBPS |
                   (BCM54194_I2C_ENABLE_ALL_PORT << 5));
    } else {
        reg_val &= ~(BCM54194_I2C_MASTER_EN_BIT);
        reg_val |= (BCM54194_I2C_DISABLE_ALL_PORT << 5);
    }
    reg_val |= BCM54194_I2C_SOFT_RST_BIT;

    rc = bcm54194_rdb_write(bus_id, phy_addr, offset, reg_val);
    if (rc < 0) {
    	printf("Failed to write GE PHY, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n", phy_addr, offset, reg_val);
        return (rc);
    }

    return (PASSED);
}

/***********************************************************************
 *
 * Function: bcm54194_issue_i2c_cmd
 *
 * Description: Issuing the I2C command, then check the command execution status.
 *
 * Inputs: i2c_cmd : BCM54194_I2C_READ_CURR_CMD
 *                   BCM54194_I2C_WRITE_CMD
 *                   BCM54194_I2C_READ_CURR_CMD
 *                   BCM54194_I2C_FLUSH_CURR_CMD
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
static int bcm54194_issue_i2c_cmd (int i2c_cmd)
{
    int rc, bus_id = SMI_BUS_0, phy_addr = PHY_PORT0;
    ushort reg_val;
    int repeat = 100;

    rc = bcm54194_rdb_read(bus_id, phy_addr, BCM54194_I2C_MASTER_CTRL_REG, &reg_val);
    if (rc < 0) {
        printf("Failed to read GE PHY, phy addr:%d, RDB offset:0x%x\n",
                phy_addr, BCM54194_I2C_MASTER_CTRL_REG);
        return (rc);
    }
    reg_val &= ~(BCM54194_I2C_CMD_MASK);
    reg_val |= i2c_cmd;

    rc = bcm54194_rdb_write(bus_id, phy_addr, BCM54194_I2C_MASTER_CTRL_REG, reg_val);
    if (rc < 0) {
    	printf("Failed to write GE PHY, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n",
                phy_addr, BCM54194_I2C_MASTER_CTRL_REG, reg_val);
        return (rc);
    }

    while (repeat--) {
        rc = bcm54194_rdb_read(bus_id, phy_addr, BCM54194_I2C_MASTER_STS_REG, &reg_val);
        if (rc < 0) {
            printf("Failed to read GE PHY, phy addr:%d, RDB offset:0x%x\n",
                    phy_addr, BCM54194_I2C_MASTER_STS_REG);
            return (rc);
        }

        if (reg_val & BCM54194_I2C_CMD_DONE_BIT) {
            break;
        }
        msleep(10);
    }

    if (repeat == 0 ) {
        printf("I2C command execution failed. I2CM_CMD = 0x#.03%x, I2CM_STS = 0x#.04%x\n",
                i2c_cmd, reg_val);
        return (FAILED);
    } else {
        return (PASSED);
    }
}

/***********************************************************************
 *
 * Function: bcm54194_i2c_slave_read
 *
 * Description: Read 1 byte of data from a given address at selected I2C slave.
 *              Enable I2C master mode before I2C access.
 * Inputs: 
 * slave_addr : external I2C slave devices address
 * offset : external I2C slave register offset
 * rd_val : read data
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
int bcm54194_i2c_slave_read (int slave_addr, int offset, ushort *rdval)
{
    int rc, bus_id = SMI_BUS_0, phy_addr = PHY_PORT0;

    /* Not sure if need to enable i2c access or not, not yet to verify. */
    //enable_bcm54194_i2c_access(TRUE);

    rc = bcm54194_rdb_write(bus_id, phy_addr, BCM54194_I2C_MASTER_DEV_ADDR_REG, slave_addr);
    if (rc < 0) {
    	printf("Failed to write GE PHY, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n",
                phy_addr, BCM54194_I2C_MASTER_REG_ADDR_REG, offset);
        return (rc);
    }

    rc = bcm54194_rdb_write(bus_id, phy_addr, BCM54194_I2C_MASTER_REG_ADDR_REG, offset);
    if (rc < 0) {
    	printf("Failed to write GE PHY, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n",
                phy_addr, BCM54194_I2C_MASTER_REG_ADDR_REG, offset);
        return (rc);
    }

    if (bcm54194_issue_i2c_cmd(BCM54194_I2C_READ_CMD) != PASSED) {
        return (FAILED);
    }

    rc = bcm54194_rdb_read(bus_id, phy_addr, BCM54194_I2C_MASTER_RDAT_REG, rdval);
    if (rc < 0) {
        printf("Failed to read GE PHY, phy addr:%d, RDB offset:0x%x\n",
                phy_addr, BCM54194_I2C_MASTER_RDAT_REG);
        return (rc);
    }

    /* Not sure if need to enable i2c access or not, not yet to verify. */
    //enable_bcm54194_i2c_access(FALSE);

    return (PASSED);
}

/***********************************************************************
 *
 * Function: bcm54194_i2c_slave_write
 *
 * Description: Write 1 byte of data to a given address at selected I2C slave.
 *              Enable I2C master mode before I2C access.
 * Inputs: 
 * slave_addr : external I2C slave devices address
 * offset : external I2C slave register offset
 * wr_val : write data
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
int bcm54194_i2c_slave_write (int slave_addr, int offset, ushort wrval)
{
    int rc, bus_id = SMI_BUS_0, phy_addr = PHY_PORT0;

    /* Not sure if need to enable i2c access or not, not yet to verify. */
    //enable_bcm54194_i2c_access(TRUE);

    rc = bcm54194_rdb_write(bus_id, phy_addr, BCM54194_I2C_MASTER_DEV_ADDR_REG, slave_addr);
    if (rc < 0) {
    	printf("Failed to write GE PHY, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n",
                phy_addr, BCM54194_I2C_MASTER_REG_ADDR_REG, offset);
        return (rc);
    }

    rc = bcm54194_rdb_write(bus_id, phy_addr, BCM54194_I2C_MASTER_REG_ADDR_REG, offset);
    if (rc < 0) {
    	printf("Failed to write GE PHY, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n",
                phy_addr, BCM54194_I2C_MASTER_REG_ADDR_REG, offset);
        return (rc);
    }

    rc = bcm54194_rdb_write(bus_id, phy_addr, BCM54194_I2C_MASTER_WDAT_REG, wrval);
    if (rc < 0) {
         printf("Failed to read GE PHY, phy addr:%d, RDB offset:0x%x\n",
                 phy_addr, BCM54194_I2C_MASTER_RDAT_REG);
        return (rc);
    }

    if (bcm54194_issue_i2c_cmd(BCM54194_I2C_WRITE_CMD) != PASSED) {
        return (FAILED);
    }

    /* Not sure if need to enable i2c access or not, not yet to verify. */
    //enable_bcm54194_i2c_access(FALSE);

    return (PASSED);
}

/***********************************************************************
 *
 * Function: bcm54194_i2c_slave_flush
 *
 * Description: Reset a selected or all slaves.
 *
 * Inputs: 
 * slave_addr : external I2C slave devices address.
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
int bcm54194_i2c_slave_flush (int slave_addr)
{
    //enable_bcm54194_i2c_access(TRUE);

    if (bcm54194_issue_i2c_cmd(BCM54194_I2C_FLUSH_CMD) != PASSED) {
        return (FAILED);
    }

    if (bcm54194_issue_i2c_cmd(BCM54194_I2C_NO_OP_CMD) != PASSED) {
        return (FAILED);
    }

    //enable_bcm54194_i2c_access(FALSE);

    return (PASSED);
}

/*
 * Function: bcm54194_transmit_test_pattern
 *
 * Description: Enable BCM54194 RDB access mode.
 *
 * Hardware or software resets to the chip will enable RDB Access mode.
 * Input: none
 *
 * Return: none
 */
int bcm54194_transmit_test_pattern(int bus_id, int phy_addr, int mode)
{
    int rc, regnum = 0x0;
    ushort reg_val;

    if ((mode == BCM54194_TEST_MODE_1) || (mode == BCM54194_TEST_MODE_2) ||
        (mode == BCM54194_TEST_MODE_4)) {
    	/* Disable auto-negotiation and force to 1000BASE-T mode */
    	regnum = BCM54194_CTRL_REG;
        rc = cvmx_mdio_write(bus_id, phy_addr, regnum, 0x0040);
        if (rc != PASSED) {
            printf("Failed to write GE PHY, phy_addr:0x%x, 0x%x\n", phy_addr, regnum);
            return (rc);
        }

        /* Disable Auto-MDIX */
        regnum = BCM54194_COPPER_MISCEL_CTRL_REG;
        rc = bcm54194_rdb_read(bus_id, phy_addr, regnum, &reg_val);
        if (rc < 0) {
            printf("Failed to read GE PHY, phy addr:%d, RDB offset:0x%x\n",
                    phy_addr, regnum);
            return (rc);
        }
        reg_val &= ~BCM54194_FORCE_AUTO_MDIX_BIT;
        rc = bcm54194_rdb_write(bus_id, phy_addr, regnum, reg_val);
        if (rc < 0) {
        	printf("Failed to write GE PHY, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n",
                    phy_addr, regnum, reg_val);
            return (rc);
        }
    } else {
    	/* Enable Auto-Negotiation */
        regnum = BCM54194_CTRL_REG;
        rc = cvmx_mdio_write(bus_id, phy_addr, regnum, 0x1140);
        if (rc != PASSED) {
            printf("Failed to write GE PHY, phy_addr:0x%x, %#.2x\n", phy_addr, regnum);
            return (rc);
        }
    }

    regnum = BCM54195_1000BASE_CTRL_REG;
    reg_val = cvmx_mdio_read(bus_id, phy_addr, regnum);
    if (reg_val < 0) {
        printf("Failed to read GE PHY, phy_addr:0x%x, 0x%x\n", phy_addr, regnum);
        return (rc);
    }
    reg_val &= ~BCM54194_TEST_MODE_MASK;

    switch(mode)
    {
        case BCM54194_TEST_MODE_1:
            reg_val |= BCM54194_TRANSMIT_WAVE_TEST;
            break;
        case BCM54194_TEST_MODE_2:
        	reg_val |= BCM54194_MS_TRANSMIT_JITTER_TEST;
            break;
        case BCM54194_TEST_MODE_3:
        	reg_val |= BCM54194_SL_TRANSMIT_JITTER_TEST;
            break;
        case BCM54194_TEST_MODE_4:
        	reg_val |= BCM54194_TRANSMIT_DIST_TEST;
            break;
        default:
        	reg_val |= BCM54194_NORMAL_MODE;
            break;
    }
    rc = cvmx_mdio_write(bus_id, phy_addr, BCM54195_1000BASE_CTRL_REG, reg_val);
    if (rc < 0) {
        printf("Failed to write GE PHY, phy_addr:0x%x, %#.2x\n", phy_addr, regnum);
        return (rc);
    }
    return (rc);
}

/*-------------------------------------------------
$Log: bcm54194_api.c,v $
Revision 1.4  2018/07/23 07:38:47  meho
Added dump bcm54194 internal loopback setting when failure occur.

Revision 1.3  2018/06/07 01:30:55  meho
BRCM recommends need a reset after exiting Copper Loopback on BCM54194

Revision 1.2  2018/05/18 09:24:53  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.1.2.26  2018/05/07 08:47:58  meho
Fixed BCM54194 test mode utility.

Revision 1.1.2.25  2018/04/16 08:42:19  meho
Added GLC-GE-100FX SFP loopback utility.

Revision 1.1.2.24  2018/01/10 09:23:17  meho
Added 100M SFP external loopback utility.

Revision 1.1.2.23  2017/12/29 06:28:11  meho
Workaround for BCM54194 B0 silicon MDIO address issue.

Revision 1.1.2.22  2017/10/31 07:07:14  meho
Separated BCM54194 PTP1588 LIB from bcm54194_api.c.

Revision 1.1.2.21  2017/10/31 02:47:51  meho
Added comment on BCM54194 PTP scripts.

Revision 1.1.2.20  2017/10/30 08:52:57  meho
Added 1588 config script for BCM54194.

Revision 1.1.2.19  2017/10/18 09:18:20  meho
Added BCM54194 reset by FPGA.

Revision 1.1.2.18  2017/10/17 09:58:46  meho
Added bcm54194 MACsec test.

Revision 1.1.2.17  2017/07/11 06:45:57  meho
Fixed PRRQ commnet.

Revision 1.1.2.16  2017/04/10 05:27:24  meho
Integrated BCM82752/82757 API.

Revision 1.1.2.15  2017/01/11 03:40:08  meho
Added GE PHY Test Mode Util.

Revision 1.1.2.14  2016/11/28 03:43:55  meho
1. Fixed GE phy Mac/Int/Ext loopback test bugs.
2. Added 10G FW download.

Revision 1.1.2.13  2016/09/14 02:44:27  meho
Added BCM54194 I2C r/w utilities.

Revision 1.1.2.12  2016/08/18 06:57:49  meho
Code clean up.

Revision 1.1.2.11  2016/08/12 10:12:18  meho
Clean up code.

Revision 1.1.2.10  2016/08/04 03:39:38  meho
Added the enable BCM54194 PTP function in loopback test.

Revision 1.1.2.9  2016/08/03 06:25:19  meho
Added enable PTP1588 sequence for BCM54195.

Revision 1.1.2.8  2016/07/26 07:54:26  meho
Added GE PHY PTP1588 loopback test skeleton.

Revision 1.1.2.7  2016/07/21 09:43:12  meho
Added GE PHY MACsec skeleton.

Revision 1.1.2.6  2016/07/20 01:44:59  meho
Added GE PHY loopback debug utilities.

Revision 1.1.2.5  2016/07/13 08:28:09  meho
1. Added Cavium PCS internal loopback.
2. Added check link up function for bcm54194.

Revision 1.1.2.4  2016/07/12 08:40:58  meho
1. Added BCM54194/BCM82752 register tests.
2. Added BCM54194 internal/external-copper loopback configuration.

Revision 1.1.2.3  2016/07/07 09:04:29  meho
1. Added BCM54194 RDB register r/w utility.
2. Added GE PHY internal/external loopback skeleton.
3. Added 10GE PHY internal/external loopback skeleton.

Revision 1.1.2.2  2016/06/23 12:54:17  meho
Added some delay for the access switching.

Revision 1.1.2.1  2016/06/23 12:44:54  meho
Added bcm54194 soft-reset function.



$Endlog$
*/
