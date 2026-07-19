/* $Id: bcm82752_api.c,v 1.2 2018/05/18 09:24:56 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/cavium/bcm82752_api.c,v $
 *-----------------------------------------------------------------------------
 * bcm82752_api.c - API for BCM 10G PHY bcm82752.
 *          Leverage from KP
 *
 * June 2016, Bo Wang
 *
 * Copyright (c) 2016 - 2018 by Cisco Systems, Inc.
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
#include <sys/ioctl.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <net/if.h>
#include <netinet/in.h>
#include <linux/sockios.h>
#include <linux/mii.h>

#include "defs.h"
#include "types.h"
#include "proto.h"
#include "common.h"
#include "common_utils.h"
#include "monitor.h"
#include "menu.h"
#include "nvmonvars.h"
#include "error.h"
#include "router_if.h"
#include "sgmii_defs.h"
#include "cvmx.h"
#include "ethernet.h"
#include "linux_eth.h"
#include "platform_eth.h"
#include "cvmx-mdio.h"
#include "ethernet.h"  /* for SFPx port definition */
#include "dash_fpga.h" /* for get SFP ctrl reg */  
#include "queryflags.h" /* for query user functions */  
#include "sff_trans.h"
#include "bcm82752_api.h"
#include "bcm82752_reg_def.h"
#include "quadra28_pkg.h"
#include "bcm_pm_if_api.h"

static unsigned int tx_rx = 0, inv = 0, ena_dis = 1, lb = 0, time_val = 0;
static int bcm82752_device_open = 0;

int bcm8275x_hw_init_done = 0;


extern bcm_plp_access_t plp_info;
extern bcm_plp_sec_phy_access_t sec_info;
extern uint16_t bcm82752_ucode[];
extern uint32_t bcm82752_ucode_size;
extern int quadra28_device_open();
extern int bcm_reg_read(void *p_ctxt,int if_side,unsigned int phy_id,unsigned int lane,unsigned int dev_id,unsigned int *reg_addr,unsigned int *val,int n);

#define EDC_FW_SUCCESS                  0x0     /* success */
#define EDC_FW_ERR_PARAM                0x1     /* bad parameters */
#define EDC_FW_ERR_SW_INIT              0x2     /* fw_ops is invalid */
#define EDC_FW_ERR_HW_ACC               0x3     /* edc hw acc error */
#define EDC_FW_ERR_ACC_TIMEOUT          0x4     /* edc hw acc timeout */
#define EDC_FW_ERR_NOT_SUPPORT          0x5     /* edc not support operation */
#define EDC_FW_ERR_LINE_NOT_LOCKED      0x6     /* edc port line side not locked */
#define EDC_FW_ERR_HOST_NOT_LOCKED      0x7     /* edc port line side not locked */
#define EDC_FW_ERR_LPBK_UNSUPPORTED     0x8     /* edc loopback unsupported */
#define EDC_FW_ERR_NO_EDC_CONNECTED     0x9     /* no edc connected for the fp */
#define EDC_FW_ERR_INTF_UNSUPPORTED     0xa     /* edc interface unsupported */
#define EDC_FW_ERR_NOT_INITIALIZED      0xb     /* edc port not initialized */
#define EDC_FW_ERR_LINE_NOT_CONVERGED   0xc     /* edc port line side not converged  */
#define EDC_FW_ERR_HOST_NOT_CONVERGED   0xd     /* edc port host side not converged  */
#define EDC_FW_ERR_NOT_IMPLEMENTED      999     /* functionality not implemented */

/*
 * Function: bcm82757_miura_reg_rd
 *
 * Description:
 * Read Broadcom 82757 PHY direct/indirect register by BCM API MIURA_1_1
 * 
 * Input:
 * lane - lane of the PHY
 * intf - system/line side
 * dev_id - MMD
 * reg - direct/indirect Register
 *
 * Return: read_value/FAILED
 */
int bcm82757_miura_reg_rd(uint lane, uint intf, uint dev_id, uint regnum)
{
    unsigned int rdval = 0, rv = FAILED;
    if (intf == BCM82752_XFI_INTF) {
        //miura definition: SYS_SIDE=1, LINE_SIDE=0
        plp_info.if_side = SYS_SIDE;
    } else {
        plp_info.if_side = LINE_SIDE;
    }
    plp_info.lane_map = 1 << lane;
    rv = bcm_plp_reg_value_get("miura", plp_info, dev_id, regnum, &rdval);
    if (rv != PASSED) {
        return (-1);
    }
#ifdef DEBUG
    printf("phy_id:%#.2x, if_side:%d, lane:%#.4x, reg:%#.8x\n", plp_info.phy_addr, plp_info.if_side, plp_info.lane_map, rdval);
#endif
    return (rdval);
}

/*
 * Function: bcm82757_miura_reg_wr
 *
 * Description:
 * Write Broadcom 82757 PHY direct/indirect register by BCM API MIURA_1_1
 * 
 * Input:
 * lane - lane of the PHY
 * intf - system/line side
 * dev_id - MMD
 * reg - direct/indirect Register
 * val - value to write
 * Return: PASSED/FAILED
 */
int bcm82757_miura_reg_wr(uint lane, uint intf, uint dev_id, uint regnum, uint val)
{
    unsigned int rv = FAILED;
    if (intf == BCM82752_XFI_INTF) {
        //miura definition: SYS_SIDE=1, LINE_SIDE=0
        plp_info.if_side = SYS_SIDE;
    } else {
        plp_info.if_side = LINE_SIDE;
    }
    plp_info.lane_map = 1 << lane;
    rv = bcm_plp_reg_value_set("miura", plp_info, dev_id, regnum, val);
    if (rv != PASSED) {
        return (-1);
    }
    return (rv);
}

/*
 * Function: bcm82757_emphasis_setting
 *
 * Description:
 * 
 * Input:
 * lane - lane of the PHY
 * intf - system/line side
 * dev_id - MMD
 * reg - direct/indirect Register
 * val - value to write
 * Return: PASSED/FAILED
 */
int bcm82757_emphasis_setting (void)
{
    ushort wrval;
    uint rv = FAILED, phy_addr, dev_id = BCM82752_DEV_PMA, regnum;

    plp_info.if_side = LINE_SIDE;
    for (phy_addr = 0; phy_addr < 2; phy_addr++) {
        plp_info.lane_map = 1 << phy_addr;
        regnum = BCM82757_TX_CTRL5_REG;
        wrval = 0x2000;
        rv = bcm_plp_reg_value_set("miura", plp_info, dev_id, regnum, wrval);
        if (rv != PASSED) {
            return (-1);
        }
        regnum = BCM82757_TX_FIR_CTRL1_REG;
        wrval = 0x00E0;
        rv = bcm_plp_reg_value_set("miura", plp_info, dev_id, regnum, wrval);
        if (rv != PASSED) {
            return (-1);
        }
        regnum = BCM82757_TX_FIR_CTRL2_REG;
        wrval = 0x8028;
        rv = bcm_plp_reg_value_set("miura", plp_info, dev_id, regnum, wrval);
        if (rv != PASSED) {
            return (-1);
        }
    }
    return (rv);
}

/*
 * Function: bcm82752_reg_rd
 *
 * Description:
 * Read Broadcom 82752 PHY register. Use Cavium MDIO bus access directly.
 * 
 * Input:
 * port - The MII phy id
 * dev - MMD
 * reg - Register to read
 *
 * Return: read_value/FAILED
 */
int bcm82752_reg_rd(int port, int dev, int reg)
{
    unsigned int mii_value = 0x0;
    int bus_id = SMI_BUS_1;

    mii_value = cvmx_mdio_45_read(bus_id, port, dev, reg);
    if (mii_value == -1) {
        printf("Read error from phy %d dev %u)\n", port, dev);
        return FAILED;
    } else {
#ifdef DEBUG
        printf("phy %d dev %u reg %d = 0x%04%x\n", port, dev, reg, mii_value);
#endif
    }
    return (mii_value);
}

/*
 * Function: bcm82752_reg_wr
 *
 * Description:
 * Write Broadcom 82752 PHY register. Use Cavium MDIO bus access directly.
 * 
 * Input:
 * port - The MII phy id
 * dev - MMD
 * reg - Register to write
 * val - value to write
 *
 * Return: PASSED/FAILED
 */
int bcm82752_reg_wr(int port, int dev, int reg, int val)
{
    int status;
    int bus_id = SMI_BUS_1;

    status = cvmx_mdio_45_write(bus_id, port, dev, reg, val);
    if (status == -1) {
        printf("Write error from phy %d dev %u)\n", port, dev);
        return (FAILED);
    } else {
#ifdef DEBUG
        printf("phy %d dev %u reg %d = 0x%04%x\n", port, dev, reg, mii_value);
#endif
    }
    return (PASSED);
}

/*
 * Set whether to access XFI or SFI registers.
 * Register 1.FFFF.0 = 1 for XFI register access.
 */
int bcm82752_xfi_sfi_access(int port, bcm82752_intf_t intf)
{
    uint32_t rc = FW_SUCCESS;
    uint32_t reg_value;

    switch(intf)
    {   
        case BCM82752_XFI_INTF:
            reg_value = bcm82752_reg_rd(port, BCM82752_DEV_PMA, BCM82752_XFI_SFI_SWITCH_REG);
            /* Set bit 0 to 1 for XFI access */
            reg_value |= BCM82752_XFI_SWITCH_MASK;
            rc = bcm82752_reg_wr(port, BCM82752_DEV_PMA, BCM82752_XFI_SFI_SWITCH_REG, reg_value);
            break;

        case BCM82752_SFI_INTF:
            reg_value = bcm82752_reg_rd(port, BCM82752_DEV_PMA, BCM82752_XFI_SFI_SWITCH_REG);
            /* Set bit 0 to 0 for SFI access */
            reg_value &= (~BCM82752_XFI_SWITCH_MASK);
            rc = bcm82752_reg_wr(port, BCM82752_DEV_PMA, BCM82752_XFI_SFI_SWITCH_REG, reg_value);
            break;

        default:
            rc = FW_ERR_PARAM;
            break;
    }

    /* Add some dealy for the access switching */
#if 0
    edc_fw_usleep(10);
#endif
    return rc;
}

int bcm82752_edc_mode_complete_check(int port, uint16_t data)
{
    int i;
    uint16_t xcvr_reg_data;
    uint16_t temp_reg_data;
    int timeout = 1; 

    bcm82752_reg_wr(port, BCM82752_DEV_PMA, BCM82752_PMD_CONTROL_MODE_0_REG, data);
    /* give edc ucode time to complete and reflect change in duplicate register */
    usleep(200 * 1000);

    temp_reg_data = bcm82752_reg_rd(port, BCM82752_DEV_PMA, BCM82752_PMD_CONTROL_MODE_0_REG);

    for (i=0; i < 1000; i++) {
        /* read 1.C843 until it equals 1.C8D8 (or timeout) */
        xcvr_reg_data = bcm82752_reg_rd(port, BCM82752_DEV_PMA, BCM82752_PMD_CONTROL_MODE_0_COPY_REG);
        if (temp_reg_data == xcvr_reg_data) {
            timeout = 0; 
            break;
        } else {
            continue;
        }
    }

    if (timeout) {
        printf("%s: Port %d mode mismatch: 1.C8D8=0x%x, 1.C843=0x%x\n", __FUNCTION__, port, temp_reg_data, xcvr_reg_data);
        return 0;
    }
    return 1;
}

/* Set the active state for optxenb */
static uint32_t bcm82752_tx_onoff_control(uint32_t port, uint8_t enable)
{
  uint16_t xcvr_reg_data;

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("BCM INFO - %s(port %d, enable %d)\n",__FUNCTION__,
                port, enable);
    }

  /* 0xc8da */
  xcvr_reg_data = bcm82752_reg_rd(port, BCM82752_DEV_PMA, BCM82752_APPS_MODE_2_REG);

  if (enable) {
    /* drive bit 5 low will drive optxenb low */
    xcvr_reg_data &= 0xffDF;
  } else {
    xcvr_reg_data |= 0x0060;
  }

  bcm82752_reg_wr(port, BCM82752_DEV_PMA, BCM82752_APPS_MODE_2_REG,
                  xcvr_reg_data);
  return 0;
}

int bcm82752_soft_reset(int port, int dev_id)
{
    int i;
    uint16_t xcvr_reg_data;
    uint8_t reset_problem = 1; 

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("BCM INFO - %s(port %d)\n",__FUNCTION__, port);
    }

    xcvr_reg_data = bcm82752_reg_rd(port, dev_id, BCM82752_PMD_CONTROL_REG);
    xcvr_reg_data |= 0x8000;
    bcm82752_reg_wr(port, dev_id, BCM82752_PMD_CONTROL_REG, xcvr_reg_data);

    /* Per broadcom, verify bit 15 not stuck high after soft reset */
    for (i = 0; i < 10; i++) {
        xcvr_reg_data = bcm82752_reg_rd(port, dev_id, BCM82752_PMD_CONTROL_REG);
        if ((xcvr_reg_data & 0x8000) != 0x8000) {
            reset_problem = 0; 
            break;
        } else {
            usleep(100 * 1000);
            continue;
        }
    }

    if (reset_problem) {
        printf("%s() reset_problem detected!\n",__FUNCTION__);
    }
    return reset_problem;
}

int bcm82752_ucode_download(void)
{
    int TOTAL_WR_BYTE = bcm82752_ucode_size;
    int reg_value, port, i;
    int PHY0_ADDRESS = 0x0;

    /* Enable Broadcast mode */
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("BCM INFO - ENABLE BCAST MODE:\n");
    }
    reg_value = 0x0001;
    for (port = 0; port < 2; port++) {
        bcm82752_reg_wr(port, BCM82752_DEV_PMA, BCM82752_BROADCAST_CONTROL_REG, reg_value);
        usleep(2000);
    }

    /* For reading MGT Message register */
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("BCM INFO - SETUP FOR READING MGT MESSAGE REGISTER:\n");
    }
    reg_value = bcm82752_reg_rd(PHY0_ADDRESS, BCM82752_DEV_PMA, 0XFFD1);
    reg_value |= (1<<12);
    bcm82752_reg_wr(PHY0_ADDRESS, BCM82752_DEV_PMA, 0XFFD1, reg_value);
    usleep(2000);

    /* Program SPA control Register to boot from MDIO */
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("BCM INFO - PROGRAM SPA CONTROL REGISTER TO BOOT FROM MDIO:\n");
    }
    reg_value = bcm82752_reg_rd(PHY0_ADDRESS, BCM82752_DEV_PMA, BCM82752_SPI_PORT_CONTROL_STATUS_REG);
    reg_value &= 0x5FFF;
    reg_value |= 0x4000;
    bcm82752_reg_wr(PHY0_ADDRESS, BCM82752_DEV_PMA, BCM82752_SPI_PORT_CONTROL_STATUS_REG, reg_value);
    usleep(2000);

    /* Put micros in reset */
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("BCM INFO - PUT MICROS IN RESET:\n");
    }
    reg_value = 0x0001;
    bcm82752_reg_wr(PHY0_ADDRESS, BCM82752_DEV_PMA, 0xFFD0, reg_value);
    usleep(2000);
    reg_value = bcm82752_reg_rd(PHY0_ADDRESS, BCM82752_DEV_PMA, BCM82752_GENERAL_CONTROL_STATUS_REG);
    reg_value |= (1 << 2);
    bcm82752_reg_wr(PHY0_ADDRESS, BCM82752_DEV_PMA, BCM82752_GENERAL_CONTROL_STATUS_REG, reg_value);
    usleep(1000 * 5);

    /* Release micros from reset */
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("BCM INFO - RELEASE MICROS FROM RESET:\n");
    }
    reg_value = bcm82752_reg_rd(PHY0_ADDRESS, BCM82752_DEV_PMA, BCM82752_GENERAL_CONTROL_STATUS_REG);
    reg_value &= ~(1 << 2);
    bcm82752_reg_wr(PHY0_ADDRESS, BCM82752_DEV_PMA, BCM82752_GENERAL_CONTROL_STATUS_REG, reg_value);
    usleep(2000);
    reg_value = 0;
    bcm82752_reg_wr(PHY0_ADDRESS, BCM82752_DEV_PMA, 0xFFD0, reg_value);

    /* Magic delay, change with care */
    usleep(1000 * 10);

    /* Set starting address to 0x8000 */
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("BCM INFO - SET STARTING ADDRESS TO 0x8000:\n");
    }
    reg_value = 0x8000;
    bcm82752_reg_wr(PHY0_ADDRESS, BCM82752_DEV_PMA, BCM82752_MESSAGE_IN_REG, reg_value);
    usleep(2000);

    /* Write the size of the data transfer */
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("BCM INFO - WRITE THE SIZE OF THE DATA TRANSFER:\n");
    }
    reg_value = TOTAL_WR_BYTE/2;
    bcm82752_reg_wr(PHY0_ADDRESS, BCM82752_DEV_PMA, BCM82752_MESSAGE_IN_REG, reg_value);
    usleep(2000);

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("BCM INFO - DOWNLOADING %d BYTES OF UCODE\n", bcm82752_ucode_size);
        printf("           FIRST 8 BYTES OF FIRMWARE FILE: ");
    }
    for (i = 0; i < 8; i++) {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("0x%x ", ((unsigned char*)bcm82752_ucode)[i]);
        }
    }
    printf("\n");

    /* Write out the data */
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("BCM INFO - WRITING OUT THE DATA USING REGISTER 0x%x:\n", BCM82752_MESSAGE_IN_REG);
    }
    for (i = 0; i < TOTAL_WR_BYTE - 1; i += 2) {
        reg_value = (((unsigned char*)bcm82752_ucode)[i] << 8) | (((unsigned char*)bcm82752_ucode)[i+1]);
        bcm82752_reg_wr(PHY0_ADDRESS, BCM82752_DEV_PMA, BCM82752_MESSAGE_IN_REG, reg_value);
        usleep(20);
        if (i == 16) {
            /* TBDJCB Turn off register debug output */
        }
        if ((i != 0) && (i % 8192 == 0)) {
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("Download word 0x%x hex bytes\n", i);
            }
        }
    }

    /* Make sure the last word is read by micro */
    usleep(2000);

    /* Start Checking results */
    /* Only valid to check every other port */
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("BCM INFO - CHECKING RESULTS:\n");
    }
    for (port = 0; port < 2; port++) {
        reg_value = bcm82752_reg_rd(port, BCM82752_DEV_PMA, BCM82752_MESSAGE_OUT_REG);
        if (reg_value != 0x4321) {
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("BCM ERR - MSG_OUT (0xCA13) of Done from MGT micro port %d: 0x%x. Expected 0x4321.\n", port, reg_value);
            }
        } else {
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("BCM INFO - DONE Message from Micro-%d 0x%x as expected.\n", port, reg_value);
            }
        }

        reg_value = bcm82752_reg_rd(port, BCM82752_DEV_PMA, BCM82752_MESSAGE_OUT_REG);
        if (reg_value != 0x0300) {
            printf("BCM ERR - MSG_OUT (0xCA13) of Chksum from MGT micro port %d: 0x%x. Expected 0x0300.\n", port, reg_value);
        } else {
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("BCM INFO - Checksum Message from Micro-%d 0x%x as expected.\n", port, reg_value);
            }
        }

        reg_value = bcm82752_reg_rd(port, BCM82752_DEV_PMA, 0xFFF3);
        if (reg_value != 0x600d) {
            printf("BCM ERR - Boot Checksum from Central micro port %d: 0x%x. Should be 0x600d\n", port, reg_value);
        } else {
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("BCM INFO - Boot Checksum from Central micro port %d: 0x%x as expected.\n", port, reg_value);
            }
        }
    }

    /* Broadcast mode: to read EDC0/1 Message register flip alternate register bit (0xFFD1.12) */
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("BCM INFO - FLIP REG BIT 0xFFD1.12 TO READ EDC0/1 MSG:\n");
    }
    reg_value = bcm82752_reg_rd(PHY0_ADDRESS, BCM82752_DEV_PMA, 0xFFD1);
    reg_value &= ~(1 << 12);
    bcm82752_reg_wr(PHY0_ADDRESS, BCM82752_DEV_PMA, 0xFFD1, reg_value);
    usleep(2000);

    /* Read Checksum from EDC micro */
    for (port = 0; port < 2; port++) {
        reg_value = bcm82752_reg_rd(port, BCM82752_DEV_PMA, BCM82752_MESSAGE_OUT_REG);
        if (reg_value != 0x4321) {
            printf("BCM ERR - MSG_OUT (0xCA13) of Done from EDC micro port %d: 0x%x. Should be 0x4321.\n", port, reg_value);
        } else {
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("BCM INFO - MSG_OUT (0xCA13) of Done from EDC micro port %d: 0x%x as expected.\n", port, reg_value);
            }
        }

        reg_value = bcm82752_reg_rd(port, BCM82752_DEV_PMA, BCM82752_MESSAGE_OUT_REG);
        if (reg_value != 0x0300) {
            printf("BCM ERR - MSG_OUT (0xCA13) of Chksum from EDC micro port %d: 0x%x. Should be 0x0300.\n", port, reg_value);
        } else {
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("BCM INFO - MSG_OUT (0xCA13) of Chksum from EDC micro port %d: 0x%x as expected.\n", port, reg_value);
            }
        }

        reg_value = bcm82752_reg_rd(port, BCM82752_DEV_PMA, BCM82752_GENERAL_PURPOSE_REG_4);
        if (reg_value != 0x600d) {
            printf("BCM ERR - Chksum (0xCA1C) from EDC micro port %d: 0x%x. Should be 0x600d.\n", port, reg_value);
        } else {
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("BCM INFO - Checksum (0xCA1C) from EDC micro port %d: 0x%x as expected.\n", port, reg_value);
            }
        }
    } /* end for (port... */

    /* Clear broadcast mode */
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("BCM INFO - CLEAR BROADCAST MODE:\n");
    }
    reg_value = 0x0;
    for (port = 0; port < 2; port++) {
        reg_value = bcm82752_reg_rd(port, BCM82752_DEV_PMA, BCM82752_BROADCAST_CONTROL_REG);
        reg_value &= 0xFFFE;
        bcm82752_reg_wr(port, BCM82752_DEV_PMA, BCM82752_BROADCAST_CONTROL_REG, reg_value);
    }
    usleep(2000);

    /* Read MGT microcode version */
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("BCM INFO - READ MGT UCODE VERSION:\n");
    }
    for (port = 0; port < 2; port++) {
        int tmp;
        reg_value = bcm82752_reg_rd(port, BCM82752_DEV_PMA, 0xC017);
        tmp = bcm82752_reg_rd(port, BCM82752_DEV_PMA, 0xC161);
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("BCM INFO - Port %d version info: 0xC017=0x%x, 0xC161=0x%x\n", port, reg_value, tmp);
        }
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("BCM INFO - MICROCODE DOWNLOAD COMPLETE.\n");
    }

    return FW_SUCCESS;
}

int bcm82752_verify_mcode(int port)
{
    int reg_value;
    int rc = FW_SUCCESS;

    // reg 0xCA1C is the checksum verification code and if the checksum is good then the value is 0x600D
    reg_value = bcm82752_reg_rd(port, BCM82752_DEV_PMA, BCM82752_GENERAL_PURPOSE_REG_4);
    if(reg_value != BCM82752_MCODE_CKSUM_VERIFY_VALUE)
    {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("BCM INFO - %s(port %d) bad microcode checksum verification code,\n read 0x%x, expected 0x%x\n",
                    __FUNCTION__, port, reg_value, BCM82752_MCODE_CKSUM_VERIFY_VALUE);
        }
        if(rc == FW_SUCCESS)
            rc = FW_ERR_HW_ACC;
    }
    return rc;
}

int bcm82752_is_sfp_module_present(int port)
{
  uint16_t xcvr_reg_data;

  /* Set bit 12 of the optics digital control register
   * then check module absent fault in RX_ALARM status register
   */
  xcvr_reg_data = bcm82752_reg_rd(port, BCM82752_DEV_PMA, BCM82752_PCS_OPTICS_DIGITAL_CONTROL_REG);
  xcvr_reg_data |= 0x1000;
  bcm82752_reg_wr(port, BCM82752_DEV_PMA, BCM82752_PCS_OPTICS_DIGITAL_CONTROL_REG, xcvr_reg_data);
  xcvr_reg_data = bcm82752_reg_rd(port, BCM82752_DEV_PMA, BCM82752_RX_ALARM_STATUS_REG);
  if ((NVRAM)->diagflag & D_VERBOSE) {
      printf("BCM INFO - %s(port %d) SFP PRESENT = %d\n",__FUNCTION__, port, !(xcvr_reg_data & 0x20));
  }
  return (!(xcvr_reg_data & 0x20));
}

/*
 * I2C access to the SFP registers and EEPROM through the phy
 * i2c_dev_addr = I2C device address (0xA0)
 * reg_addr     = reg address (probably 0 for reading SFP eeprom)
 * buf          = ptr to buffer
 * size         = size of buffer to read
 * write_op     = 1 for write operation, 0 for read operation
 */
uint32_t bcm82752_twsi_mii_reg_rw(uint8_t port, uint32_t i2c_dev_addr,
                                 uint32_t reg_addr, uint8_t *buf, uint32_t size,
                                 uint8_t write_op)
{
  uint16_t xcvr_reg_data;
  uint32_t i, mii_wait = 500;
  //fwdev_log_lvl_t lvl;

  if ((NVRAM)->diagflag & D_VERBOSE) {
      printf("BCM INFO - %s(port, idc_dev_addr 0x%x, reg_addr 0x%x, size %d, wr %d)\n",
              __FUNCTION__, i2c_dev_addr, reg_addr, size, write_op);
  }
  //lvl = edc_fw_set_log_lvl(FWDEV_LOG_LVL_ERR);

  /* read twice */
  xcvr_reg_data = bcm82752_reg_rd(port, BCM82752_DEV_PMA, BCM82752_TWO_WIRE_CONTROL_REG);
  xcvr_reg_data = bcm82752_reg_rd(port, BCM82752_DEV_PMA, BCM82752_TWO_WIRE_CONTROL_REG);

  /* If 2wire master is not enabled */
  if (!(xcvr_reg_data & 0x8000)) {
    xcvr_reg_data = 0x8000;
    bcm82752_reg_wr(port, BCM82752_DEV_PMA, BCM82752_TWO_WIRE_CONTROL_REG,
                    xcvr_reg_data);
    usleep(100*1000);
  }

  /* Recommendations from broadcom */
  /* Check status of master */
  xcvr_reg_data = bcm82752_reg_rd(port, BCM82752_DEV_PMA, BCM82752_TWO_WIRE_CONTROL_REG);
  xcvr_reg_data = bcm82752_reg_rd(port, BCM82752_DEV_PMA, BCM82752_TWO_WIRE_CONTROL_REG);
  if (xcvr_reg_data & 0x0600) {
    /* 0x0600 means the I2c master is not working correctly. */
    /* This should not happen, but if it does, then reset.   */
    /* write 1.8207.10 = 1 then 1.8207.10 = 0                */
    /* Bits 10:09 are Status of I2C Master transfer.         */
    /* 0 - Successful,                                       */
    /* 1 - No Ackn for Slave ID,                             */
    /* 2 - No Ackn for Reg Address,                          */
    /* 3 - No Ackn for Data Byte.                            */
    /* Reset value is 0x0.                                   */
    printf("BCM INFO - I2C Master Status = 0x%x\n",xcvr_reg_data);
    printf("         -- WARNING RESET I2C via 1.8207.10\n");

    xcvr_reg_data = bcm82752_reg_rd(port, BCM82752_DEV_PMA, 0x8207);
    xcvr_reg_data |= (1<<10);
    bcm82752_reg_wr(port, BCM82752_DEV_PMA, 0x8207, xcvr_reg_data);
    xcvr_reg_data &= 0xfbff;
    bcm82752_reg_wr(port, BCM82752_DEV_PMA, 0x8207, xcvr_reg_data);
    usleep(30 * 1000);
  }

  xcvr_reg_data = bcm82752_reg_rd(port, BCM82752_DEV_PMA, BCM82752_TWO_WIRE_CONTROL_REG);
  xcvr_reg_data = bcm82752_reg_rd(port, BCM82752_DEV_PMA, BCM82752_TWO_WIRE_CONTROL_REG);

  if (xcvr_reg_data & 0x0080) {
    usleep(30 * 1000);

    xcvr_reg_data = bcm82752_reg_rd(port, BCM82752_DEV_PMA, BCM82752_TWO_WIRE_CONTROL_REG);
    xcvr_reg_data = bcm82752_reg_rd(port, BCM82752_DEV_PMA, BCM82752_TWO_WIRE_CONTROL_REG);
    if (xcvr_reg_data & 0x0080) {
      printf("BCM INFO - I2c status = 0x%x\n",xcvr_reg_data);
      printf("         -- Reset I2C via 1.8207.10\n");

      xcvr_reg_data = bcm82752_reg_rd(port, BCM82752_DEV_PMA, 0x8207);
      xcvr_reg_data |= (1<<10);
      bcm82752_reg_wr(port, BCM82752_DEV_PMA, 0x8207, xcvr_reg_data);
      xcvr_reg_data &= 0xfbff;
      bcm82752_reg_wr(port, BCM82752_DEV_PMA, 0x8207, xcvr_reg_data);
      usleep(30 * 1000);
    }
  }

  while (mii_wait) {
    usleep(100);
    xcvr_reg_data = bcm82752_reg_rd(port, BCM82752_DEV_PMA, BCM82752_TWO_WIRE_CONTROL_REG);


    if ((xcvr_reg_data & 0x000C) == 0x000C) {
      /* Command Failed */
      printf("BCM ERR - I2C STATUS FAILURE 0x8000 = 0x%x\n",
              xcvr_reg_data);
      mii_wait = 0;
      break;
    } else if ((xcvr_reg_data & 0x0008) == 0x0008) {
      /* Command in progress, keep trying */
    } else {
      /* complete ok, or idle */
      break;
    }
    mii_wait--;
  } /* end while */

  if (mii_wait == 0) {
    printf("BCM ERR - port %d I2C Status Timeout. 0x8000 = 0x%x\n",
            port, xcvr_reg_data);
    //lvl = edc_fw_set_log_lvl(lvl);
    return EDC_FW_ERR_ACC_TIMEOUT;
  }

  /* If 2wire master is in auto-detect mode */
  if (xcvr_reg_data & 0x1000) {
    xcvr_reg_data &= ~0x1000;
  }
  bcm82752_reg_wr(port, BCM82752_DEV_PMA, BCM82752_TWO_WIRE_CONTROL_REG,
                  xcvr_reg_data);

  /* starting address location 0x8007 storage into reg 0x8004 */
  xcvr_reg_data = 0x8007;
  bcm82752_reg_wr(port, BCM82752_DEV_PMA, BCM82752_TWO_WIRE_INTERNAL_ADDRESS_REG,
                  xcvr_reg_data);

  /* set start addr location in 0x8003 of r/w data to be retrieved
   * or stored in sfp+ eeprom
   */
  xcvr_reg_data = reg_addr;
  bcm82752_reg_wr(port, BCM82752_DEV_PMA, BCM82752_TWO_WIRE_NVM_ADDRESS_REG,
                  xcvr_reg_data);

  /* number of bytes. SFP eeprom should be 256bytes */
  xcvr_reg_data = size & 0x3FFF;
  bcm82752_reg_wr(port, BCM82752_DEV_PMA, BCM82752_TWO_WIRE_TRANSFER_SIZE_REG,
                  xcvr_reg_data);

  /* eeprom device address reg 0x8005 */
  xcvr_reg_data = bcm82752_reg_rd(port, BCM82752_DEV_PMA, BCM82752_TWO_WIRE_SLAVE_ID_ADDRESS_REG);
  xcvr_reg_data &= 0x00FF;
  xcvr_reg_data |= ((i2c_dev_addr << 8) & 0xFE00);
  bcm82752_reg_wr(port, BCM82752_DEV_PMA, BCM82752_TWO_WIRE_SLAVE_ID_ADDRESS_REG,
                  xcvr_reg_data);

  /* Check cmd/status */
  xcvr_reg_data = bcm82752_reg_rd(port, BCM82752_DEV_PMA, BCM82752_TWO_WIRE_CONTROL_REG);
  xcvr_reg_data = bcm82752_reg_rd(port, BCM82752_DEV_PMA, BCM82752_TWO_WIRE_CONTROL_REG);
  if (xcvr_reg_data & 0x000C) {
    printf("BCM ERR - port %d, BSC I2C is not idle\n", port);
    //lvl = edc_fw_set_log_lvl(lvl);
    return EDC_FW_ERR_HW_ACC;
  }

  if (!write_op) {
    /* start the read transaction */
    xcvr_reg_data = bcm82752_reg_rd(port, BCM82752_DEV_PMA, BCM82752_TWO_WIRE_CONTROL_REG);
    xcvr_reg_data &= 0xFFDC;
    xcvr_reg_data |= 0x0002;
    bcm82752_reg_wr(port, BCM82752_DEV_PMA, BCM82752_TWO_WIRE_CONTROL_REG,
                    xcvr_reg_data);

    mii_wait = 500;

    while(mii_wait) {
      usleep(1000);
      xcvr_reg_data = bcm82752_reg_rd(port, BCM82752_DEV_PMA, BCM82752_TWO_WIRE_CONTROL_REG);
      if ((xcvr_reg_data & 0x000C)  == 0x000C) {
        /* Command Failed */
        printf("BCM ERR - I2C STATUS FAILURE 0x8000 = 0x%x\n", xcvr_reg_data);
        mii_wait = 0;
        break;
      } else if ((xcvr_reg_data & 0x0008) == 0x0008) {
        /* Command in progress.  Keep trying */
      } else {
        /* Complete ok, or Idle */
        break;
      }
      mii_wait--;
    } /* end while */

    if (mii_wait == 0) {
      printf("BCM ERR - port %d I2C Read can not complete.\n", port);
      printf("        -- Status: 0x8000 = 0x%x, Expected 0.\n",
              xcvr_reg_data);
      //lvl = edc_fw_set_log_lvl(lvl);
      return EDC_FW_ERR_HW_ACC;
    }

    /* Read out the data */
    for (i = 0; i < size; i++) {
      xcvr_reg_data = bcm82752_reg_rd(port, BCM82752_DEV_PMA, (0x8007 + i));
      buf[i] = xcvr_reg_data;
    }

  } else { /* WRITE OPERATION */
    /* load the write data */
    for (i = 0; i < size; i++) {
      xcvr_reg_data = buf[i];
      bcm82752_reg_wr(port, BCM82752_DEV_PMA, (0x8007 + i), xcvr_reg_data);
    }

    /* start the write transaction */
    xcvr_reg_data = bcm82752_reg_rd(port, BCM82752_DEV_PMA, BCM82752_TWO_WIRE_CONTROL_REG);
    xcvr_reg_data &= 0xffDC;
    xcvr_reg_data |= 0x0022;
    bcm82752_reg_wr(port, BCM82752_DEV_PMA, BCM82752_TWO_WIRE_CONTROL_REG,
                    xcvr_reg_data);

    mii_wait = 500;
    while(mii_wait) {
      usleep(1000);
      xcvr_reg_data = bcm82752_reg_rd(port, BCM82752_DEV_PMA, BCM82752_TWO_WIRE_CONTROL_REG);
      if ((NVRAM)->diagflag & D_VERBOSE) {
          printf("BCM INFO - I2C Write operation status: 0x8000 = 0x%x\n",
                  xcvr_reg_data);
      }
      if ((xcvr_reg_data & 0x000C)  == 0x000C) {
        /* Command Failed */
        printf("BCM ERR - I2C STATUS FAILURE 0x8000 = 0x%x\n", xcvr_reg_data);
        mii_wait = 0;
        break;
      } else if ((xcvr_reg_data & 0x0008) == 0x0008) {
        /* Command in progress.  Keep trying */
      } else {
        /* Complete ok, or Idle */
        break;
      }
      mii_wait--;
    } /* end while */

    if (mii_wait == 0) {
      printf("BCM ERR - port %d I2C Write can not complete.\n",
              port);
      printf("        -- Status: 0x8000 = 0x%x, Expected 0.\n",
              xcvr_reg_data);
      //lvl = edc_fw_set_log_lvl(lvl);
      return EDC_FW_ERR_HW_ACC;
    }
  } /* end write operation */

  //lvl = edc_fw_set_log_lvl(lvl);
  return EDC_FW_SUCCESS;
}

static void bcm87252_display_sfp_eeprom_cksum_err(uint8_t port,
                                                  sff_trans_map_t *sfp_map,
                                                  int start, int end)
{
  printf("SFP Module port %d, EEPROM SFP Byte %d-%d checksum (0x%x). "
         "Calculated checksum=0x%x\n", port, start, end, sfp_map->sff_rcksum,
         sfp_map->sff_ccksum);
  sfp_map->sff_rcksum = 0;
  sfp_map->sff_ccksum = 0;
}

static uint32_t bcm87252_sfp_eeprom_checksum_validate(uint8_t port,
                                                      sff_trans_map_t *sfp_map)
{
  int rc = 0;

  if ((NVRAM)->diagflag & D_VERBOSE) {
      printf("BCM INFO - %s() VALIDATING PORT %d EEPROM\n",__FUNCTION__, port);
  }

  /* Check Base ID fields 0-62 */
  if ((rc = sff_trans_eeprom_checksum_base_ids_validate(sfp_map)) < 0) {
    printf("BCM ERR - SFP EEPROM Base ID Fields corrupted on port %d.\n",port);
    bcm87252_display_sfp_eeprom_cksum_err(port, sfp_map, 0, 62);
  }

  /* Check extended ID fields 64-94 */
  if ((rc |= sff_trans_eeprom_checksum_ext_ids_validate(sfp_map)) < 0) {
    printf("BCM ERR - SFP Extended ID Fields corrupted on port %d.\n",port);
    bcm87252_display_sfp_eeprom_cksum_err(port, sfp_map, 64, 94);
  }

  /* Vendor Specific ID Fields */
  if ((rc |= sff_trans_eeprom_checksum_vend_ids_validate(sfp_map)) < 0) {
    printf("BCM ERR - %s() SFP Vendor Specific ID Fields 96-123 corrupted.\n",
            __FUNCTION__);
    bcm87252_display_sfp_eeprom_cksum_err(port, sfp_map, 96, 123);
  }

  /* Check vendor specific checksums only for cisco parts */
  if (sff_trans_check_cisco_pn(sfp_map)) {
      if ((NVRAM)->diagflag & D_VERBOSE) {
          printf("BCM INFO - %s() Performing Cisco SFP checksum validation\n",
                  __FUNCTION__);
      }
    rc |= sff_sfp_eeprom_cisco_vendor_checksum_validate(sfp_map);
  }

  if ((NVRAM)->diagflag & D_VERBOSE) {
      printf("BCM INFO - %s() VALIDATION COMPLETE, RETURNING 0x%x\n",__FUNCTION__,
              rc);
  }

  return rc;
}

/* For sfp_addr, it is normally 0xA0 for first 128B of SFP eeprom or 0xA2 for
 * extended DOM region.
 */
uint32_t bcm82752_sfp_eeprom_rd(uint8_t port, sff_trans_map_t *sfp_map,
                                uint8_t sfp_addr)
{
  uint32_t rc = EDC_FW_SUCCESS;
  if ((NVRAM)->diagflag & D_VERBOSE) {
      printf("BCM INFO - %s(port %d)\n",__FUNCTION__, port);
  }

  if((rc = bcm82752_twsi_mii_reg_rw(port, sfp_addr, 0, sfp_map->sff_eeprom,
                                    SFF_EEPROM_SIZE, 0)) != 0) {
      printf("BCM ERR - EEPROM Read Failure on port %d\n", port);
      return rc;
  }

  if ((NVRAM)->diagflag & D_VERBOSE) {
      printf("BCM INFO - %s() Verifying checksums...\n",__FUNCTION__);
  }

  /* verify checksums in eeprom */
  rc = bcm87252_sfp_eeprom_checksum_validate(port, sfp_map);

  return rc;
}

/*
 * Read the SFP eeprom, get the sfp module type ID from the eeprom data and
 * convert to EDC SFP mode for applying to EDC
 */
uint32_t bcm82752_get_edc_sfp_module_type(uint8_t port,
                                          phy_port_mode_t *sfp_type)
{
  uint32_t rc = EDC_FW_SUCCESS;
  sff_trans_map_t sfp_map;
  sff_sfp_module_id_t sfp_mod_id;

  if ((rc = bcm82752_sfp_eeprom_rd(port, &sfp_map, 0xA0)) != EDC_FW_SUCCESS) {
    printf("BCM ERR - %s(port %d) Error Reading SFP EEPROM 0x%x\n",
            __FUNCTION__, port, rc);
    return rc;
  }

  sfp_mod_id = sff_get_sfp_module_id(&sfp_map);

  switch(sfp_mod_id) {
  case SFF_SFP_10G_ER:
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("BCM INFO - %s(port %d) Detected SFP_10G_ER SFP module.\n",
                __FUNCTION__, port);
    }
    *sfp_type = PORT_MODE_10G_ER;
    break;
  case SFF_SFP_10G_LRM:
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("BCM INFO - %s(port %d) Detected SFP_10G_LRM SFP module.\n",
                __FUNCTION__, port);
    }
    *sfp_type = PORT_MODE_10G_LRM;
    break;
  case SFF_SFP_10G_LR:
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("BCM INFO - %s(port %d) Detected SFP_10G_LR SFP module.\n",
                __FUNCTION__, port);
    }
    *sfp_type = PORT_MODE_10G_LR;
    break;
  case SFF_SFP_10G_SR:
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("BCM INFO - %s(port %d) Detected SFP_10G_SR SFP module.\n",
                __FUNCTION__, port);
    }
    *sfp_type = PORT_MODE_10G_SR;
    break;
  case SFF_SFP_10G_ZR:
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("BCM INFO - %s(port %d) Detected SFP_10G_ZR SFP module.\n",
                __FUNCTION__, port);
    }
    *sfp_type = PORT_MODE_10G_ZR;
    break;
  default:
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("BCM INFO - %s(port %d) Detected Default (Code 0x%x) SFP module.\n",
                __FUNCTION__, port, sfp_mod_id);
    }
    *sfp_type = PORT_MODE_DEFAULT;
    break;
  }

  return rc;
}

int bcm82752_hw_reset_init(int port)
{
    int xcvr_reg_data;

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("BCM INFO - %s port %d\n",__FUNCTION__, port);
    }

    /* Operations to be done only once */
    if (bcm8275x_hw_init_done != 1) {
        /* check if firmware is already there, if yes, no need to download again. */
        if (bcm82752_verify_mcode(port) != FW_SUCCESS) {
            /* load micro code */
            bcm82752_ucode_download();
            if (bcm82752_verify_mcode(port) != FW_SUCCESS) {
                printf("BCM ERR - port %d ucode verification failed.\n", port);
                /* We have a big problem if ucode fails, but lets not keep repeating the
                 * process. Lets try to get past this part at least.
                 */
                bcm8275x_hw_init_done= 1;
                return FW_ERR_HW_ACC;
            }
        } else {
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("BCM INFO - UCODE VERIFIED UP-TO-DATE, SKIP DOWNLOAD\n");
            }
        }
        bcm8275x_hw_init_done = 1;
    } else {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("BCM INFO - UCODE INIT COMPLETED ALREADY\n");
        }
    }

    /* bcm recommends 100ms after load fw before access any registers */
    usleep(100*1000);

    /* Set default mode: 10G, System side to LR mode, copper connection */
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("BCM INFO - SET DEFAULT MODE:\n");
    }
    xcvr_reg_data = 0x8802;
    bcm82752_edc_mode_complete_check(port, xcvr_reg_data);

    /* For ref_clk re-timer mode write 1.c8d9.4 = 0x1 TBDJCB */
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("BCM INFO - SETUP RETIMER MODE:\n");
    }
    xcvr_reg_data = bcm82752_reg_rd(port, BCM82752_DEV_PMA, BCM82752_APPS_MODE_0_REG);
    xcvr_reg_data |= 0x0010;
    bcm82752_reg_wr(port, BCM82752_DEV_PMA, BCM82752_APPS_MODE_0_REG, xcvr_reg_data);

    /* Request fw to perform config update */
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("BCM INFO - REQUEST FW TO PERFORM CONFIG UPDATE\n");
    }
    xcvr_reg_data = 0x8882;
    bcm82752_edc_mode_complete_check(port, xcvr_reg_data);

    /* sw reset */
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("BCM INFO - PERFORM A SOFT RESET:\n");
    }
    bcm82752_soft_reset(port, BCM82752_DEV_PMA);

    /* Power up module TBDJCB - NOT SURE IF WE SHOULD DO THIS NOW. */
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("BCM INFO - POWER UP MODULE:\n");
    }
    bcm82752_tx_onoff_control(port, 1);

    /* Give it time to come up */
    usleep(100*1000);

    /* SFP unreset */
    /* this is for controlling rate select pins
     * not reset as initially thought.
     * bcm82752_gpio_ctrl_init(port, 0);
     */

    /* Get SFP module type and set mode */
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("BCM INFO - GET SFP MODULE TYPE AND SET MODE:\n");
    }
    if (bcm82752_is_sfp_module_present(port)) {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("BCM INFO - SFP MODULE DETECTED FOR PORT %d\n", port);
        }
        /* Get the module internal ID */
        phy_port_mode_t sfp_type = PORT_MODE_10G_SR;
        sff_trans_map_t sfp_map;
        if (bcm82752_sfp_eeprom_rd(port, &sfp_map, 0xA0) != FW_SUCCESS) {
            printf("bcm82752_sfp_eeprom_rd failed\n");
            return (-1);
        }
        if (bcm82752_get_edc_sfp_module_type(port, &sfp_type) != FW_SUCCESS) {
            printf("bcm82752_get_edc_sfp_module_type failed\n");
            return (-1);
        }
        if (bcm82752_set_port_mode(port, sfp_type) != FW_SUCCESS) {
            printf("bcm82752_set_port_mode failed\n");
            return (-1);
        }
    } else {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("BCM INFO - No SFP presence detected for port %d.\n", port);
            printf("BCM INFO - Default to 10G_SR Mode for port %d.\n",port);
        }
        /* Default to 10G-SR */
        if (bcm82752_set_port_mode(port, PORT_MODE_10G_SR) != FW_SUCCESS) {
            printf("bcm82752_set_port_mode failed\n");
            return (-1);
        }
    }

    /* Set tx emphasis, default to 10G now */
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("BCM INFO - SET TX EMPHASIS, DEFAULT TO 10G NOW:\n");
    }
    bcm82752_set_sfi_serdes(port, SPEED_10G);

    /* Final Init step, set optxenb_lvl polarity.
     * TBDJCB - currently, FPGA register 0x5C (SFP+ Control register)
     * is not implemented. It also controls the TXONOFF inputs to the
     * EDC, but until that register is implemented, a pull-up will
     * keep those inputs high. In order to enable TXONFF, we need to set
     * polarity optxenb_lvl to 1 to enable TX output on the EDC.
     */
    xcvr_reg_data = bcm82752_reg_rd(port, BCM82752_DEV_PMA, BCM82752_PHY_IDENTIFIER_REG);
    xcvr_reg_data |= 0x8000;
    bcm82752_reg_wr(port, BCM82752_DEV_PMA, BCM82752_PHY_IDENTIFIER_REG, xcvr_reg_data);
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("BCM INFO - HW RESET INIT FINISHED\n");
    }

    return FW_SUCCESS;
}

int is_bcm82752()
{
	int xcvr_reg_data, port = 0, phy_id;

    /* Sanity check, read chip ID */
    xcvr_reg_data = bcm82752_reg_rd(port, BCM82752_DEV_PMA, BCM82752_PMD_ID_MSB_REG);
    phy_id = (xcvr_reg_data << 16);
    xcvr_reg_data = bcm82752_reg_rd(port, BCM82752_DEV_PMA, BCM82752_PMD_ID_LSB_REG);
    phy_id |= xcvr_reg_data;

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("BCM INFO - %s() chip ID check read 0x%x\n", __FUNCTION__, phy_id);
    }

    if (phy_id != PMD_PHY_ID) {
        return (FALSE);
    } else {
    	return (TRUE);
    }
}

int not_bcm82752()
{
    return (!is_bcm82752());
}

int bcm82752_init(int port)
{
    int rc = FW_SUCCESS;
    int xcvr_reg_data;
    int phy_id;

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("BCM INFO - %s(port %d).\n", __FUNCTION__, port);
    }

    /* Sanity check, read chip ID */
    xcvr_reg_data = bcm82752_reg_rd(port, BCM82752_DEV_PMA, BCM82752_PMD_ID_MSB_REG);
    phy_id = (xcvr_reg_data << 16);
    xcvr_reg_data = bcm82752_reg_rd(port, BCM82752_DEV_PMA, BCM82752_PMD_ID_LSB_REG);
    phy_id |= xcvr_reg_data;
    
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("BCM INFO - %s() chip ID check read 0x%x\n", __FUNCTION__, phy_id);
    }

    if (phy_id != PMD_PHY_ID) {
        printf("BCM ERR - Expected PHY ID 0x%x does not match read PHY ID 0x%x\n", PMD_PHY_ID, phy_id);
        return FW_ERR_SW_INIT;
    }

    /* Hardware reset */
    rc = bcm82752_hw_reset_init(port);
    if (rc != FW_SUCCESS) {
        printf("BCM ERR - HW_RESET_INIT FAILED (0x%x)\n",rc);
        return rc;
    }

    /* previous EDC it would setup tx preemphasis, but this is now done in hw_reset */
    //    bcm82752_min_cfg(port);
    printf("BCM INFO - EDC Initialization Complete (port %d).\n", port);

    return rc;
}

int bcm82752_set_port_mode(int port, phy_port_mode_t mode)
{
    int xcvr_reg_data, retimer_mode, retimer_reg_data;
    int rc = FW_SUCCESS;

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("BCM INFO - %s(port %d, mode %d)\n", __FUNCTION__, port, mode);
    }

    rc = bcm82752_xfi_sfi_access(port, BCM82752_SFI_INTF);

    /* Before setting any mode of operation, deassert 1.C8D8.7 to enable setting a mode of operation.  */    
    xcvr_reg_data = bcm82752_reg_rd(port, BCM82752_DEV_PMA, BCM82752_PMD_CONTROL_MODE_0_REG);
    xcvr_reg_data &= 0xFF7F;
    if (!bcm82752_edc_mode_complete_check(port, xcvr_reg_data)) {
        printf("BCM ERR - %s(): Mode change failed for port %d, mode %d", __FUNCTION__, port, mode);
        return -1;
    }

    xcvr_reg_data = bcm82752_reg_rd(port, BCM82752_DEV_PMA, BCM82752_PMD_CONTROL_MODE_0_REG);

    switch (mode)
    {
        case PORT_MODE_10G_LRM:
        case PORT_MODE_10G_LRM_SM:
            xcvr_reg_data &= ~0x3740;
            xcvr_reg_data |= 0x4830;
            retimer_mode = 1;
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("BCM INFO - SETTING PORT %d MODE TO 10G_LRM:\n", port);
            }
            break;
        case PORT_MODE_10G_CX1:
        case PORT_MODE_10G_COPPER_PASSIVE:
            xcvr_reg_data &= ~0x7570;
            xcvr_reg_data |= 0x0A00;
            retimer_mode = 1;
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("BCM INFO - SETTING PORT %d MODE TO 10G_CX1:\n", port);
            }
            break;

        case PORT_MODE_10G_ZR:
            xcvr_reg_data &= ~0xA400;
            xcvr_reg_data |= 0x5A00;
            retimer_mode = 1;
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("BCM INFO - SETTING PORT %d MODE TO 10G_ZR:\n", port);
            }
            break;

        case PORT_MODE_1G_SR:
        case PORT_MODE_10G_LR:
        case PORT_MODE_10G_SR:
        case PORT_MODE_10G_ER:
        case PORT_MODE_10G_ACX1:
        case PORT_MODE_10G_FET:
        case PORT_MODE_10G_USR:
        default:
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("BCM INFO - SETTING PORT %d MODE TO 10G_SR/LR/ER\n", port);
            }
            xcvr_reg_data &= ~0x7770;
            xcvr_reg_data |= 0x0800;
            retimer_mode = 1;
            break;
    }

    /* set recovered_clk_retimer or ref_clk_retimer modes */
    retimer_reg_data = bcm82752_reg_rd(port, BCM82752_DEV_PMA, BCM82752_APPS_MODE_0_REG);
    if (retimer_mode)
        retimer_reg_data |= 0x10;
    else
        retimer_reg_data &= ~0x10;
    bcm82752_reg_wr(port, BCM82752_DEV_PMA, BCM82752_APPS_MODE_0_REG, retimer_reg_data);

    /* finish_change request to firmware */
    xcvr_reg_data |= 0x0080;
    if (!bcm82752_edc_mode_complete_check(port, xcvr_reg_data)) {
        printf("BCM ERR - %s(): Mode change failure for port %d, mode %d", __FUNCTION__, port, mode);
        return -1;
    }

    /* soft reset is affecting txonoff, also give it time to be off */
#if 0
    /* TBDJCB - NOTE cat3k was doing this to control some opttxenb pin which is
     * reset for them when doing softreset. This register is not documented other than in a "Register Differences between BCM82780 and BCM84780" document provided to Cisco.  */
    rc |= bcm82752_tx_onoff_control(port, 0);
    usleep(100 *1000);
#endif

    /* SW reset required */
    if (bcm82752_soft_reset(port, BCM82752_DEV_PMA)) {
        printf("BCM ERR - Soft reset error detected for port %d\n", port);
        return -1;
    }

#if 0
    /* TBDJCB - soft reset is affecting txonoff, also give it time to be off */
    rc |= bcm82752_tx_onoff_control(port, 1);
    usleep(300 *1000);
#endif
    return rc;
}


int bcm82752_set_port_speed(int port, phy_speed_t speed)
{
    uint16_t reg_val;
    int rc = FW_SUCCESS;

    bcm82752_xfi_sfi_access(port, BCM82752_SFI_INTF);

    switch(speed)
    {    
        default:
        case SPEED_10G:
            /* Forced 10G can be enabled by writing following register in MMF device 7:
             * Writing register 0x8309 = 0x0020 (forcing 1G/10G speed selection)
             * Writing register 0xFFE0 = 0x0000 (clearing Clause 37 AN)
             */
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("BCM INFO - %s(port %d) Set to 10G speed.\n", __FUNCTION__, port);
            }
            reg_val = bcm82752_reg_rd(port, BCM82752_DEV_1GBE, BCM82752_MISCELLANEOUS_2_REG);
            reg_val |= FORCE_SPEED_ENC_EN;
            bcm82752_reg_wr(port, BCM82752_DEV_1GBE, BCM82752_MISCELLANEOUS_2_REG, reg_val);

            reg_val = bcm82752_reg_rd(port, BCM82752_DEV_1GBE, BCM82752_MII_CONTROL_REG);
            reg_val &= ~ AUTO_NEG_EN;
            bcm82752_reg_wr(port, BCM82752_DEV_1GBE, BCM82752_MII_CONTROL_REG, reg_val);

            reg_val = bcm82752_reg_rd(port, BCM82752_DEV_PMA, BCM82752_PMD_CONTROL_REG);
            reg_val |= SPEED_10G_MSK;
            bcm82752_reg_wr(port, BCM82752_DEV_PMA, BCM82752_PMD_CONTROL_REG, reg_val);

            bcm82752_reg_wr(port, BCM82752_DEV_PMA, BCM82752_PMD_CONTROL_2_REG, 0x0008);
            break;

        case SPEED_1G:
            /* forced 1-GbE mode is set by writing register 0x0000 = 0x0040 and 
             * 0x0007 = 0x000D in MMF Device 1
             */
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("BCM INFO - %s(port %d) Set to 1GBE Speed.\n", __FUNCTION__, port);
            }
            /* get the auto negotiation flag */
            reg_val = bcm82752_reg_rd(port, BCM82752_DEV_PMA, BCM82752_PRBS31_TEST_WINDOW_0_REG);
            if (reg_val == 0x1) {
                if ((NVRAM)->diagflag & D_VERBOSE) {
                    printf("BCM INFO - %s(port %d) Set to Auto Negotiate.\n", __FUNCTION__, port);
                }
                reg_val = bcm82752_reg_rd(port, BCM82752_DEV_1GBE, BCM82752_MISCELLANEOUS_2_REG);
                reg_val &= ~ FORCE_SPEED_ENC_EN;
                bcm82752_reg_wr(port, BCM82752_DEV_1GBE, BCM82752_MISCELLANEOUS_2_REG, reg_val);

                reg_val = bcm82752_reg_rd(port, BCM82752_DEV_1GBE, BCM82752_MII_CONTROL_REG);
                reg_val |= AUTO_NEG_EN;
                reg_val &= ~SPEED_SEL_MSB;
                reg_val &= ~SPEED_SEL_LSB;
                bcm82752_reg_wr(port, BCM82752_DEV_1GBE, BCM82752_MII_CONTROL_REG, reg_val);

                reg_val = bcm82752_reg_rd(port, BCM82752_DEV_1GBE, BCM82752_AUTO_NEGOTIATION_ADVERTISEMENT_REG);
                reg_val &= ~ HALF_DUPLEX_AD;
                reg_val |= FULL_DUPLEX_AD;
                bcm82752_reg_wr(port, BCM82752_DEV_1GBE, BCM82752_AUTO_NEGOTIATION_ADVERTISEMENT_REG, reg_val);
            } else {
                if ((NVRAM)->diagflag & D_VERBOSE) {
                    printf("BCM INFO - %s(port %d) Set Fix speed\n", __FUNCTION__, port);
                }
                reg_val = bcm82752_reg_rd(port, BCM82752_DEV_1GBE, BCM82752_MISCELLANEOUS_2_REG);
                reg_val |= FORCE_SPEED_ENC_EN;
                bcm82752_reg_wr(port, BCM82752_DEV_1GBE, BCM82752_MISCELLANEOUS_2_REG, reg_val);

                reg_val = bcm82752_reg_rd(port, BCM82752_DEV_1GBE, BCM82752_MII_CONTROL_REG);
                reg_val &= ~AUTO_NEG_EN;
                reg_val |= SPEED_SEL_MSB;
                reg_val &= ~SPEED_SEL_LSB;
                bcm82752_reg_wr(port, BCM82752_DEV_1GBE, BCM82752_MII_CONTROL_REG, reg_val);
            }

            reg_val = bcm82752_reg_rd(port, BCM82752_DEV_PMA, BCM82752_PMD_CONTROL_REG);
            reg_val &= ~SPEED_10G_MSK;
            bcm82752_reg_wr(port, BCM82752_DEV_PMA, BCM82752_PMD_CONTROL_REG, reg_val);

            bcm82752_reg_wr(port, BCM82752_DEV_PMA, BCM82752_PMD_CONTROL_2_REG, 0x000D);
            break;

        case SPEED_DEFAULT:
            /* 1-GbE mode can be enabled in two distinctive methods, Forced 1-GbE mode or auto-negotiation mode
             *  auto-negotiation can be enabled by writing following registers in MMF Device 7:
             *  Write register 0x8309 = 0x0000 (clearing forced 1G/10G selection)
             *  Write register 0xFFE0 = 0x1000 enable Auto-negotiation
             *  Write register 0xFFE4 = 0x0020 (enable full-duplex advertisement)
             */
            reg_val = bcm82752_reg_rd(port, BCM82752_DEV_1GBE, BCM82752_MISCELLANEOUS_2_REG);
            reg_val &= ~ FORCE_SPEED_ENC_EN;
            bcm82752_reg_wr(port, BCM82752_DEV_1GBE, BCM82752_MISCELLANEOUS_2_REG, reg_val);

            reg_val = bcm82752_reg_rd(port, BCM82752_DEV_1GBE, BCM82752_MII_CONTROL_REG);
            reg_val |= AUTO_NEG_EN;
            reg_val &= ~ SPEED_SEL_MSB;
            bcm82752_reg_wr(port, BCM82752_DEV_1GBE, BCM82752_MII_CONTROL_REG, reg_val);

            reg_val = bcm82752_reg_rd(port, BCM82752_DEV_1GBE, BCM82752_AUTO_NEGOTIATION_ADVERTISEMENT_REG);
            reg_val &= ~ HALF_DUPLEX_AD;
            reg_val |= FULL_DUPLEX_AD;
            bcm82752_reg_wr(port, BCM82752_DEV_1GBE, BCM82752_AUTO_NEGOTIATION_ADVERTISEMENT_REG, reg_val);
            break;
    }

    return rc;
}

/* TX emphasis settings */
int bcm82752_set_sfi_serdes(int port, phy_speed_t speed)
{
    uint16_t regD0A5 = 0;
    uint16_t regD0A3 = 0;
    uint16_t regD111 = 0;
    uint16_t regD110 = 0;
    bcm82752_intf_t intf;
    int rc = FW_SUCCESS;

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("BCM INFO - %s(port %d, speed %d)\n",__FUNCTION__, port, speed);
    }

    for (intf = BCM82752_XFI_INTF; intf < 2; intf++) {
        bcm82752_xfi_sfi_access(port, intf);

        /* perform system side changes then line side changes */
        if (intf == BCM82752_SFI_INTF) {
            if (speed == SPEED_1G) {
                regD0A3 = 0x0a16;   /* TBDCB - need to update these w/ real hw testing. */
                regD0A5 = 0;
                regD111 = 0x3c;
                regD110 = 0;
            } else {
                /* 10G speed */
                regD0A3 = 0xa12;
                regD0A5 = 0x7000;
                regD110 = 0x00a0;
                regD111 = 0x801c;
            }

            bcm82752_reg_wr(port, BCM82752_DEV_PMA, 0xD0A3, regD0A3);
        } else {
            /* XFI System Side */
            regD0A5 = 0xf000;
            regD110 = 0x00C0;
            regD111 = 0x8036;
        }

        bcm82752_reg_wr(port, BCM82752_DEV_PMA, 0xD0A5, regD0A5);
        bcm82752_reg_wr(port, BCM82752_DEV_PMA, 0xD110, regD110);
        bcm82752_reg_wr(port, BCM82752_DEV_PMA, 0xD111, regD111);

        {
            uint16_t xcvr_reg_data;
            xcvr_reg_data = bcm82752_reg_rd(port, BCM82752_DEV_PMA, 0xD0A3);
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("SERDES: port %d, intf %d  0x%x:0x%04x\n",port, intf, 0xD0A3,xcvr_reg_data);
            }
            xcvr_reg_data = bcm82752_reg_rd(port, BCM82752_DEV_PMA, 0xD0A5);
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("SERDES: port %d, intf %d  0x%x:0x%04x\n",port, intf, 0xD0A5,xcvr_reg_data);
            }
            xcvr_reg_data = bcm82752_reg_rd(port, BCM82752_DEV_PMA, 0xD110);
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("SERDES: port %d, intf %d  0x%x:0x%04x\n",port, intf, 0xD110,xcvr_reg_data);
            }
            xcvr_reg_data = bcm82752_reg_rd(port, BCM82752_DEV_PMA, 0xD111);
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("SERDES: port %d, intf %d  0x%x:0x%04x\n",port, intf, 0xD111,xcvr_reg_data);
            }
        }
    } /* end for */

    /* TBDJCB Turn on SFP again */

    /* need to sleep minimum ~300msec */
    usleep(500*1000);
    return rc;
}

/*------------------------------------------------------------------
 *
 * Function: bcm82757_config_loopback
 *
 * Input:
 * side: system - 0, line - 1
 * lb_mode = 1 : Digital(or Global) Loopback (Deeper loopback)
 * lb_mode = 2 : Remote Loopback (Shallow loopback)
 * These two lb_modes available for both line and system side.
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int bcm82757_config_loopback(unsigned int side, unsigned int lb_mode, unsigned int enable)
{
    int rc = FAILED;
    //unsigned int lb_mode = 4;//phymodLoopbackRemotePCS 
    unsigned int enable_flag, phy_id=0;
    memset(&sec_info, 0, sizeof(sec_info));
    memset(&plp_info, 0, sizeof(plp_info));
    
    plp_info.platform_ctxt = (void*)5;
    plp_info.lane_map = 0x3;
    plp_info.if_side = side;
    plp_info.phy_addr = phy_id;
    rc = bcm_plp_loopback_set("miura", plp_info, lb_mode, enable);
    if (rc) {
        printf("bcm_plp_loopback_set failed for PHY-ID[%d], LANE_MAP [0x%x] with return code [%d]\n", phy_id, plp_info.lane_map, rc);
        return (rc);
    }

    if (enable) {
        rc = bcm_plp_loopback_get("miura", plp_info, lb_mode, &enable_flag);
        if (rc) {
            printf("bcm_plp_loopback_get failed for PHY-ID[%d], LANE_MAP [0x%x] with return code [%d]\n", phy_id, plp_info.lane_map, rc);
            return (rc);
        }

        if (enable_flag != TRUE) {
            printf("phymodLoopbackRemotePCS config failed, %d\n", enable_flag);
            rc = (FAILED);
        }
    }
    return (rc);
}

int bcm82752_config_loopback(int port, bcm82752_loopback_t loopback_mode)
{
    int rc = FW_SUCCESS;
    uint16_t reg_val;

    switch(loopback_mode)
    {
        case BCM82752_LOOPBACK_PCS_LINE:             /* PCS Line loopback */
            /* Line loopback connects the SFI PMD CDR/deserializer to the SFI PMD CMU.
             * to enable this operation, bit 0 in PMD & PCS Test Control Register(0xCD0A)
             * must be set to 1 (in SFI side)
             */
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("BCM INFO - PCS Line loopback\n");
            }
            reg_val = bcm82752_reg_rd(port, BCM82752_DEV_PMA, BCM82752_PMD_AND_PCS_TEST_CONTROL_REG);
            reg_val |= 0x1;
            bcm82752_reg_wr(port, BCM82752_DEV_PMA, BCM82752_PMD_AND_PCS_TEST_CONTROL_REG, reg_val);
            break;

        case BCM82752_LOOPBACK_PCS_PRBS:             /* PCS PRBS loopback */
            /* PRBS loopback connects the SFI PMD CDR/deserializer to the SFI PMD CMU.
             * It is async loopback.
             * to enable this operation, bit 7 and 8 in User PRBS Control Register(0xCD14)
             * must be set to 1 (in SFI side)
             */
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("BCM INFO - PCS PRBS loopback\n");
            }
            reg_val = bcm82752_reg_rd(port, BCM82752_DEV_PMA, BCM82752_USER_PRBS_CONTROL_0_REG);
            reg_val |= 0x180;
            bcm82752_reg_wr(port, BCM82752_DEV_PMA, BCM82752_USER_PRBS_CONTROL_0_REG, reg_val);
            break;

        case BCM82752_LOOPBACK_PCS:             /* PCS loopback */
            /* The PCS/PMD loopback enables the transmit data path from 
             * the XFI deserializers through the Tx GearBox back to Rx GearBox
             * the PMD control register(0x0000.0) or PCS Control 1 Register(0x0000.14)
             * enable PCS/PMD diagnostic loopback
             */
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("BCM INFO - PCS loopback\n");
            }
            reg_val = bcm82752_reg_rd(port, BCM82752_DEV_PMA, BCM82752_PMD_CONTROL_REG);
            reg_val |= 0x1;
            bcm82752_reg_wr(port, BCM82752_DEV_PMA, BCM82752_PMD_CONTROL_REG, reg_val);

            reg_val = bcm82752_reg_rd(port, BCM82752_DEV_PCS, BCM82752_PCS_CONTROL_1_REG);
            reg_val |= BIT32(14);
            bcm82752_reg_wr(port, BCM82752_DEV_PCS, BCM82752_PCS_CONTROL_1_REG, reg_val);
            break;

        case BCM82752_LOOPBACK_XFI_LINE:
            /* It connects the XFI deserializer output directly to the XFI serializer input
             * bit 0 in PMD & PCS Test Control Register(0xCD0A) must be set to 1
             */
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("BCM INFO - XFI Line loopback\n");
            }

            /* first switch to XFI mode, default is SFI mode */
            bcm82752_xfi_sfi_access(port, BCM82752_XFI_INTF);

            /* set xfi line loopback */
            reg_val = bcm82752_reg_rd(port, BCM82752_DEV_PMA, BCM82752_PMD_AND_PCS_TEST_CONTROL_REG);
            reg_val |= 0x1;
            bcm82752_reg_wr(port, BCM82752_DEV_PMA, BCM82752_PMD_AND_PCS_TEST_CONTROL_REG, reg_val);

            /* switch back to SFI mode */
            bcm82752_xfi_sfi_access(port, BCM82752_SFI_INTF);
            break;

        case BCM82752_LOOPBACK_XFI_PRBS:
            /* It connects the XFI deserializer output directly to the XFI serializer input
             * bit 0 in PMD & User PRBS Control Register(0xCD14) must be set to 1
             */
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("BCM INFO - XFI PRBS loopback\n");
            }

            /* first switch to XFI mode, default is SFI mode */
            bcm82752_xfi_sfi_access(port, BCM82752_XFI_INTF);

            /* set xfi line loopback */
            reg_val = bcm82752_reg_rd(port, BCM82752_DEV_PMA, BCM82752_USER_PRBS_CONTROL_0_REG);
            reg_val |= 0x180;
            bcm82752_reg_wr(port, BCM82752_DEV_PMA, BCM82752_USER_PRBS_CONTROL_0_REG, reg_val);

            /* switch back to SFI mode */
            bcm82752_xfi_sfi_access(port, BCM82752_SFI_INTF);
            break;

        case BCM82752_LOOPBACK_XFI:
            /* The XFI system loopback enables the receiver data path from the SFI
             * deserializer through the 64B/66B encoder, loopbacked to the transmit
             * path 64B/66B decoder
             * set XFI device 1 register 0x0000 bit 0 to 1 or XFI device 3 register
             * 0x0000 bit 14 to 1
             */
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("BCM INFO - XFI loopback\n");
            }

            /* first switch to XFI mode, default is SFI mode */
            bcm82752_xfi_sfi_access(port, BCM82752_XFI_INTF);

            /* set XFI loopback */
            reg_val = bcm82752_reg_rd(port, BCM82752_DEV_PMA, BCM82752_PMD_CONTROL_REG);
            reg_val |= 0x1;
            bcm82752_reg_wr(port, BCM82752_DEV_PMA, BCM82752_PMD_CONTROL_REG, reg_val);

            reg_val = bcm82752_reg_rd(port, BCM82752_DEV_PCS, BCM82752_PCS_CONTROL_1_REG);
            reg_val |= BIT32(14);
            bcm82752_reg_wr(port, BCM82752_DEV_PCS, BCM82752_PCS_CONTROL_1_REG, reg_val);

            /* switch back to SFI mode */
            bcm82752_xfi_sfi_access(port, BCM82752_SFI_INTF);
            break;

        case BCM82752_LOOPBACK_NONE:
        default:
            /* Clear the loopback */
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("BCM INFO - None loopback\n");
            }
            /* default is SFI mode */
            /* Clear PCS Line loopback */
            reg_val = bcm82752_reg_rd(port, BCM82752_DEV_PMA, BCM82752_PMD_AND_PCS_TEST_CONTROL_REG);
            reg_val &= ~0x1;
            bcm82752_reg_wr(port, BCM82752_DEV_PMA, BCM82752_PMD_AND_PCS_TEST_CONTROL_REG, reg_val);

            /* Clear PCS PRBS loopback */
            reg_val = bcm82752_reg_rd(port, BCM82752_DEV_PMA, BCM82752_USER_PRBS_CONTROL_0_REG);
            reg_val &= ~0x180;
            bcm82752_reg_wr(port, BCM82752_DEV_PMA, BCM82752_USER_PRBS_CONTROL_0_REG, reg_val);

            /* Clear PCS loopback */
            reg_val = bcm82752_reg_rd(port, BCM82752_DEV_PMA, BCM82752_PMD_CONTROL_REG);
            reg_val &= ~0x1;
            bcm82752_reg_wr(port, BCM82752_DEV_PMA, BCM82752_PMD_CONTROL_REG, reg_val);

            reg_val = bcm82752_reg_rd(port, BCM82752_DEV_PCS, BCM82752_PCS_CONTROL_1_REG);
            reg_val &= ~BIT32(14);
            bcm82752_reg_wr(port, BCM82752_DEV_PCS, BCM82752_PCS_CONTROL_1_REG, reg_val);

            /* Switch to XFI mode */
            bcm82752_xfi_sfi_access(port, BCM82752_XFI_INTF);

            /* Clear XFI line loopback */
            reg_val = bcm82752_reg_rd(port, BCM82752_DEV_PMA, BCM82752_PMD_AND_PCS_TEST_CONTROL_REG);
            reg_val &= ~0x1;
            bcm82752_reg_wr(port, BCM82752_DEV_PMA, BCM82752_PMD_AND_PCS_TEST_CONTROL_REG, reg_val);

            /* Clear XFI PRBS loopback */
            reg_val = bcm82752_reg_rd(port, BCM82752_DEV_PMA, BCM82752_USER_PRBS_CONTROL_0_REG);
            reg_val &= ~0x180;
            bcm82752_reg_wr(port, BCM82752_DEV_PMA, BCM82752_USER_PRBS_CONTROL_0_REG, reg_val);

            /* Clear XFI loopback */
            reg_val = bcm82752_reg_rd(port, BCM82752_DEV_PMA, BCM82752_PMD_CONTROL_REG);
            reg_val &= ~0x1;
            bcm82752_reg_wr(port, BCM82752_DEV_PMA, BCM82752_PMD_CONTROL_REG, reg_val);

            reg_val = bcm82752_reg_rd(port, BCM82752_DEV_PCS, BCM82752_PCS_CONTROL_1_REG);
            reg_val &= ~BIT32(14);
            bcm82752_reg_wr(port, BCM82752_DEV_PCS, BCM82752_PCS_CONTROL_1_REG, reg_val);

            /* Switch back to SFI mode */
            bcm82752_xfi_sfi_access(port, BCM82752_SFI_INTF);

            break;
    }
    return rc;
}

int bcm82752_is_link_up(int port, int *link_up)
{
    int rc = 0;
    uint16_t xcvr_reg_data;
    int i, j;

    rc = bcm82752_xfi_sfi_access(0, BCM82752_SFI_INTF);

    /* Read status register twice to get status */
    xcvr_reg_data = bcm82752_reg_rd(port, BCM82752_DEV_PMA, BCM82752_USER_PMD_STATUS_REG);
    xcvr_reg_data = bcm82752_reg_rd(port, BCM82752_DEV_PMA, BCM82752_USER_PMD_STATUS_REG);

    /* It might take some time for link state to be presented in the register */
    for (i = 0; i < 10; i++) {
        /* look for cdr lock in reg 0xc804. Normally 0xdF when SFP present & link up.
         * It is 0x88 when no SFP is plugged in.
         * [0] ln_rx_sigdet - signal detected at line-side input
         * [1] oplosb       - TBDJCB 0 = light is ok, but cat3k checks for 1
         * [2] ln_lkdtcdr   - 1 = line CDR lock detected
         * [3] ln_lkdtcmu   - 1 = line pll has lock
         * [4] sys_rx_sigdet- 1 = signal detected at system-side input
         * [6] sys_lkdtcdr  - 1 = system CDR lock detected
         * [7] sys_lkdtcmu  - 1 = system pll has lock
         */
        if ((xcvr_reg_data & 0xdf) != 0xdf) {
            xcvr_reg_data = bcm82752_reg_rd(port, BCM82752_DEV_PMA, BCM82752_USER_PMD_STATUS_REG);
            xcvr_reg_data = bcm82752_reg_rd(port, BCM82752_DEV_PMA, BCM82752_USER_PMD_STATUS_REG);
            usleep(10*1000);
            *link_up = 0;
            continue;
        } else {
            *link_up = 1;
        }
    }

    if (*link_up == 0) {
        return 0;
    }

    /* Since we will never operate as a repeater mode continue */
    for (j = BCM82752_XFI_INTF; j < BCM82752_MAX_INTF; j++) {
        rc = bcm82752_xfi_sfi_access(0, BCM82752_XFI_INTF);
        xcvr_reg_data = bcm82752_reg_rd(port, BCM82752_DEV_PMA, BCM82752_PMD_DIGITAL_STATUS_REG);
        xcvr_reg_data = bcm82752_reg_rd(port, BCM82752_DEV_PMA, BCM82752_PMD_DIGITAL_STATUS_REG);
        for (i = 0; i < 10; i++) {
            /* PMD Digital Status Register 0xCD09
             * under normal operation this register should read 0x200
             */
            if (!(xcvr_reg_data & 0x200)) {
                printf("BCM INFO - %s(p %d) %s side PMD Digital Status Down, Reg 0x%x = 0x%x\n", __FUNCTION__, port, j ? 
                        "XFI system":"SFI line", BCM82752_PMD_DIGITAL_STATUS_REG, xcvr_reg_data);
                xcvr_reg_data = bcm82752_reg_rd(port, BCM82752_DEV_PMA, BCM82752_PMD_DIGITAL_STATUS_REG);
                xcvr_reg_data = bcm82752_reg_rd(port, BCM82752_DEV_PMA, BCM82752_PMD_DIGITAL_STATUS_REG);
                usleep(10*1000);
                *link_up = 0;
                continue;
            } else {
                *link_up = 1;
                break;
            }
        }
    }

    if (*link_up == 0) {
        return 0;
    }

    for (j = BCM82752_XFI_INTF; j < BCM82752_MAX_INTF; j++) {
        rc = bcm82752_xfi_sfi_access(0, BCM82752_XFI_INTF);
        xcvr_reg_data = bcm82752_reg_rd(port, BCM82752_DEV_PCS, BCM82752_PCS_STATUS_1_REG);
        xcvr_reg_data = bcm82752_reg_rd(port, BCM82752_DEV_PCS, BCM82752_PCS_STATUS_1_REG);
        for (i = 0; i < 10; i++) {
            /* In XFI PCS (device address 00011) Status 1 Register 0x0001
             * [2] PCS receive link status - 1 = PCS receive link up.
             */
            if (!(xcvr_reg_data & 0x4)) {
                printf("BCM INFO - %s(p %d) %s side Link Down, Dev3 PCS Status Reg 0x%x = 0x%x\n", __FUNCTION__, port, j ? 
                        "XFI system":"SFI line", BCM82752_PCS_STATUS_1_REG, xcvr_reg_data);
                xcvr_reg_data = bcm82752_reg_rd(port, BCM82752_DEV_PCS, BCM82752_PCS_STATUS_1_REG);
                xcvr_reg_data = bcm82752_reg_rd(port, BCM82752_DEV_PCS, BCM82752_PCS_STATUS_1_REG);
                usleep(10 * 1000);
                *link_up = 0;
                continue;
            } else {
                *link_up = 1;
                break;
            }
        }
    }

    if (*link_up) {
        printf("BCM INFO - %s(port %d) Link is up!\n", __FUNCTION__, port);
    } else {
        printf("BCM INFO - %s(port %d) Link is down!\n", __FUNCTION__, port);
    }

    return rc;
}

int bcm82752_cfg_setting(int port, int speed, int duplex, int auto_neg, bcm82752_intf_t intf)
{
    int rc = 0;


    return rc;
}

/***********************************************************************
 *
 * Function: enable_bcm82752_ptp_engine
 *
 * Description: Enable PHY 82752 PTP engine
 *
 * Inputs: None
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
int enable_bcm82752_ptp_engine (int eth_port)
{
    return (PASSED);
}

int en_bcm82752_ptp_per_port (int eth_port, int speed)
{
    return(PASSED);
}

int quadra28_eye_diagram(void)
{
    int no_phy_ids = 1;
    unsigned int phy_id = 0,link_sts=0;
    int rv = 0, p_ctxt=5;
    unsigned int if_side;
    int sp = 0, ref = 0, mode = 0;
    int intf = 0;
    unsigned int lane=0x1;
    bcm_plp_device_aux_modes_t s_aux_mode;
    bcm_plp_device_aux_modes_t aux_mode;
    bcm_plp_pm_phy_diagnostics_t diag;
    memset(&s_aux_mode, 0, sizeof(bcm_plp_device_aux_modes_t));
    memset(&aux_mode, 0, sizeof(bcm_plp_device_aux_modes_t));
    memset(&diag,0,sizeof(bcm_plp_pm_phy_diagnostics_t));
    s_aux_mode.pass_thru = 1;
    int n;
    bcm_plp_access_t phy_info;
    unsigned int prbs_lock=0, prbs_lock_loss=0, error_count=0;
    /*config settings used to configure the speed and interface on system side and line side*/
    int speed = 10000, if_type = bcm_pm_InterfaceSR, ref_clk = 0, if_mode = 0; /* Line side default config values for 10G SR*/
    int s_speed = 10000, s_if_type = bcm_pm_InterfaceXFI; /* system side default config values for 10G  XFI*/
    unsigned int poly_t = 0, lb_t = 0, inv_t = 0, ena_dis_t = 0;

    if (!bcm82752_device_open) {
        if (quadra28_device_open() < 0) {
            printf("Init Failed\n");
            return rv;
        } else {
    	    bcm82752_device_open = 1;
            printf("Init SUccess\n");
        }
    }

    /** ******************************************************************************************
    * Setting the line side and system side configuration in 10G mode
    * In 10G mode each lane is addressed by individual MDIO address. Each lane is configured individually in respective mode and
    * speed.
    ********************************************************************************************/
    printf("---------------------------------------------------------\n");
    printf("------------- SYSTEM IF_TYPE = %d -----------------------\n", s_if_type);
    printf("---------------------------------------------------------\n");
    if_side = SYSTEM_SIDE_INTERFACE;
    printf("=========================if_side %d================================================\n", if_side);

    for (phy_id = 0; phy_id <= no_phy_ids; phy_id++) {
        phy_info.platform_ctxt = (void*)&p_ctxt;
        phy_info.phy_addr = phy_id;
        phy_info.if_side = if_side;
        phy_info.lane_map = lane;
        rv = bcm_plp_quadra28_mode_config_set((*(bcm_plp_quadra28_access_t*) (&phy_info)), s_speed, s_if_type,
                                              ref_clk, if_mode, (void*)&s_aux_mode);
        if (rv != 0) {
            printf("bcm_plp_mode_config_set failed with rv:%d lane: 0x%x\n", rv, lane);
            return rv;
        }
        printf("Mode config set success for phy_id = %d,Interface = %d, speed = %d if_mode:%d ref_clk:%d\n",
                    phy_id, s_if_type, speed, if_mode, ref_clk);
        rv = bcm_plp_quadra28_mode_config_get((*(bcm_plp_quadra28_access_t*) (&phy_info)), &sp, &intf, &ref,
                                              &mode, (void*)&aux_mode);
        if (rv != 0) {
            printf("bcm_plp_mode_config_get failed with rv:%d lane: 0x%x\n", rv, lane);
            return rv;
        }
        printf("Mode config get success for phy_id = %d,Interface = %d, speed = %d if_mode:%d ref_clk:%d\n",
                    phy_id, intf, sp, mode, ref);

        if((speed == sp)&& (s_if_type == intf) && (if_mode == mode)&& (ref_clk == ref)){
            printf("PASSED : Mode config set successfully\n");
        } else {
            printf("FAIL : Mode config set fail\n");
            return rv;
        }
    }
    printf("--------------------------------------------------\n");
    printf("------------- LINE IF_TYPE = %d -----------------------\n", if_type);
    printf("--------------------------------------------------\n");
    if_side = LINE_SIDE_INTERFACE;
    printf("=========================if_side %d================================================\n", if_side);
    for (phy_id = 0; phy_id <= no_phy_ids; phy_id++) {
        phy_info.platform_ctxt = (void*)&p_ctxt;
        phy_info.phy_addr = phy_id;
        phy_info.if_side = if_side;
        phy_info.lane_map = lane;
        rv = bcm_plp_quadra28_mode_config_set((*(bcm_plp_quadra28_access_t*) (&phy_info)), speed, if_type,
                                              ref_clk, if_mode, (void*)&s_aux_mode);
        if (rv != 0) {
            printf("bcm_plp_quadra28_mode_config_set failed with rv:%d lane: 0x%x\n", rv, lane);
            return rv;
        }
        printf("Mode config set success for phy_id = %d,Interface = %d, speed = %d if_mode:%d ref_clk:%d\n",
                phy_id, if_type, speed, if_mode, ref_clk);
        rv = bcm_plp_quadra28_mode_config_get((*(bcm_plp_quadra28_access_t*) (&phy_info)), &sp, &intf, &ref,
                                              &mode, (void*)&aux_mode);
        if (rv != 0) {
            printf("bcm_plp_quadra28_mode_config_get failed with rv:%d lane: 0x%x\n", rv, lane);
            return rv;
        }
        printf("Mode config get success for phy_id = %d,Interface = %d, speed = %d if_mode:%d ref_clk:%d\n",
                phy_id, intf, sp, mode, ref);

        if((speed == sp)&& (if_type == intf) && (if_mode == mode)&& (ref_clk == ref)){
            printf("PASSED : Mode config set successfully\n");
        } else {
            printf("FAIL : Mode config set fail\n");
            return rv;
        }
    }

    /**********************************************************************
     * In standalone setup TX line is disabled because it is not controlled by tx_disable_pin
     * So in standalone we need to disable the tx_disable_pin control with TX squelch off
     * This is required for any traffic or communication to start on lane
     *********************************************************************/
    for(if_side=0;if_side<=1;if_side++){
        for (phy_id = 0; phy_id <= no_phy_ids; phy_id++) {
            phy_info.platform_ctxt = (void*)&p_ctxt;
            phy_info.phy_addr = phy_id;
            phy_info.if_side = if_side;
            phy_info.lane_map = lane;
            rv = bcm_plp_quadra28_tx_lane_control_set((*(bcm_plp_quadra28_access_t*) (&phy_info)),
                                                      bcmpmTxSquelchOff);
            if(rv != 0){
                printf("tx lane control set failed rv=%d phy_id=%d\n",rv,phy_id);
            }
            else{
                printf("tx lane control set passed phy_id=%d\n",phy_id);
            }
        }
    }

   /* Reading the register specified in reg_array */
    if_side=LINE_SIDE_INTERFACE;
    printf("if_side=%d\n",if_side);
    n=sizeof(reg_array)/sizeof(reg_array[0]);
    for(phy_id=0;phy_id<= no_phy_ids;phy_id++){

         bcm_reg_read(&p_ctxt,if_side, phy_id,lane,1,reg_array,val_array,n);
    }

    /*************************************************************************************************************************
      *  prbs checker and generator are enabled in different polynomial 7,9,11,15,23,31,58 as specified by poly_array.
      *  tx_rx parameter is to enable checker or generator or both.
      *  tx_rx = 0 for both checker and generator
      *  tx_rx=1 to enable checker
      *  tx_rx=2 to enable prbs generator
      *************************************************************************************************************************/
    if_side=LINE_SIDE_INTERFACE;
    printf("---------------------PRBS TX RX SET---------if_side = %d---------\n",if_side);

    for(phy_id=0;phy_id<=no_phy_ids;phy_id++){
        phy_info.platform_ctxt = (void*)&p_ctxt;
        phy_info.phy_addr = phy_id;
        phy_info.if_side = if_side;
        phy_info.lane_map = lane;
        rv = bcm_plp_quadra28_prbs_set((*(bcm_plp_quadra28_access_t*) (&phy_info)),
                                              tx_rx, poly_array[0], inv, lb, ena_dis);
        if (rv != 0) {
            printf("prbs set failed for lane = 0x%x on phy_id = %d ,rv=%d\n",lane,phy_id,rv);
            return rv;
        } else {
                    rv = bcm_plp_quadra28_prbs_get((*(bcm_plp_quadra28_access_t*) (&phy_info)),
                                                   tx_rx, &poly_t, &inv_t, &lb_t, &ena_dis_t);
                    if (rv != 0) {
                        printf("prbs config get failed for lane = 0x%x on phy_id = %d\n",lane,phy_id);
                        return rv;
                    }
                    if ((poly_t == poly_array[0])&&(inv_t == inv)&&(ena_dis_t == ena_dis))
                    {
                        printf("PASSED: prbs get poly = %d at for lane = 0x%x tx_rx=%d inv=%d on phy_id = %d\n",poly_t,lane,tx_rx,inv_t,phy_id);
                    }  else {
                        printf("FAILED: prbs get invalid with rv = %d poly = %d inv : %d inv = %d lb %d en_dis %d at for lane = 0x%x on phy_id = %d\n",
                                rv,poly_t,inv_t,inv,lb_t,ena_dis_t,lane,phy_id);
                    }
         }
    }
    if_side=SYSTEM_SIDE_INTERFACE;
    printf("---------------------PRBS TX RX SET---------if_side = %d---------\n",if_side);

    for(phy_id=0;phy_id<= no_phy_ids;phy_id++){
        phy_info.platform_ctxt = (void*)&p_ctxt;
        phy_info.phy_addr = phy_id;
        phy_info.if_side = if_side;
        phy_info.lane_map = lane;
        rv = bcm_plp_quadra28_prbs_set((*(bcm_plp_quadra28_access_t*) (&phy_info)),
                                       tx_rx, poly_array[0], inv, lb, ena_dis);
        if (rv != 0) {
            printf("prbs set failed for lane = 0x%x on phy_id = %d ,rv=%d\n",lane,phy_id,rv);
            return rv;
        } else {
            rv = bcm_plp_quadra28_prbs_get((*(bcm_plp_quadra28_access_t*) (&phy_info)),
                                           tx_rx, &poly_t, &inv_t, &lb_t, &ena_dis_t);
            if (rv != 0) {
                printf("prbs config get failed for lane = 0x%x on phy_id = %d\n",lane,phy_id);
                return rv;
            }
            if ((poly_t == poly_array[0])&&(inv_t == inv)&&(ena_dis_t == ena_dis))
            {
                printf("PASSED: prbs get poly = %d at for lane = 0x%x tx_rx=%d inv=%d on phy_id = %d\n",poly_t,lane,tx_rx,inv_t,phy_id);
            }  else {
                printf("FAILED: prbs get invalid with rv = %d poly = %d inv : %d inv = %d lb %d en_dis %d at for lane = 0x%x on phy_id = %d\n",
                        rv,poly_t,inv_t,inv,lb_t,ena_dis_t,lane,phy_id);
            }
         }
    }
    /* To avoid prbs check failure after enabling prbs added sleep */
    sleep(2);
    if_side=LINE_SIDE_INTERFACE;
    printf("---------------------PRBS STAT-------if_side = %d----------\n",if_side);

    for(phy_id=0;phy_id<= no_phy_ids;phy_id++){
        phy_info.platform_ctxt = (void*)&p_ctxt;
        phy_info.phy_addr = phy_id;
        phy_info.if_side = if_side;
        phy_info.lane_map = lane;
        rv = bcm_plp_quadra28_prbs_rx_stat((*(bcm_plp_quadra28_access_t*) (&phy_info)), time_val);
        if (rv != 0) {
            printf("prbs stat failed for lane = 0x%x on phy_id = %d\n",lane,phy_id);

        }
        rv = bcm_plp_quadra28_prbs_status_get((*(bcm_plp_quadra28_access_t*) (&phy_info)), &prbs_lock, &prbs_lock_loss, &error_count);
        if (rv != 0) {
            printf("prbs stat get failed for lane = 0x%x on phy_id = %d\n",lane,phy_id);

        } else if(prbs_lock && (!prbs_lock_loss) && (error_count == 0))  {
            printf("PASSED: prbs stat get prbs_lock = %d, prbs_lock_loss = %d, error_count = %d phy_id = %d lane=%d\n", prbs_lock, prbs_lock_loss, error_count,phy_id,lane);
        } else {
            printf("FAILED: prbs stat get prbs_lock = %d, prbs_lock_loss = %d, error_count = %d phy_id = %d lane=%d\n", prbs_lock, prbs_lock_loss, error_count,phy_id,lane);
        }
    }
    if_side=SYSTEM_SIDE_INTERFACE;
    printf("---------------------PRBS STAT-------if_side = %d----------\n",if_side);

    for(phy_id=0;phy_id<= no_phy_ids;phy_id++){
        phy_info.platform_ctxt = (void*)&p_ctxt;
        phy_info.phy_addr = phy_id;
        phy_info.if_side = if_side;
        phy_info.lane_map = lane;
        rv = bcm_plp_quadra28_prbs_rx_stat((*(bcm_plp_quadra28_access_t*) (&phy_info)), time_val);
        if (rv != 0) {
            printf("prbs stat failed for lane = 0x%x on phy_id = %d\n",lane,phy_id);

        }
        rv = bcm_plp_quadra28_prbs_status_get((*(bcm_plp_quadra28_access_t*) (&phy_info)), &prbs_lock, &prbs_lock_loss, &error_count);
        if (rv != 0) {
            printf("prbs stat get failed for lane = 0x%x on phy_id = %d\n",lane,phy_id);

        } else if(prbs_lock && (!prbs_lock_loss) && (error_count == 0))  {
            printf("PASSED: prbs stat get prbs_lock = %d, prbs_lock_loss = %d, error_count = %d phy_id = %d lane=%d\n", prbs_lock, prbs_lock_loss, error_count,phy_id,lane);
        } else {
            printf("FAILED: prbs stat get prbs_lock = %d, prbs_lock_loss = %d, error_count = %d phy_id = %d lane=%d\n", prbs_lock, prbs_lock_loss, error_count,phy_id,lane);
        }
    }

   phy_info.if_side = SYSTEM_SIDE_INTERFACE;
   printf("=========================Link Status if_side if_side %d================================================\n", phy_info.if_side);
   for (phy_id =0;  phy_id <= no_phy_ids ; phy_id++) {
       phy_info.phy_addr=phy_id;
       rv = bcm_plp_quadra28_link_status_get((*(bcm_plp_quadra28_access_t*) (&phy_info)), &link_sts);
       if (rv != 0) {
           printf("bcm_plp_quadra28_link_status_get failed with rv:%d phy_info.lane_map: 0x%x\n", rv, phy_info.lane_map);

       } else {
           printf("Link status = %d on phy_info.lane_map map = 0x%x\n", link_sts, phy_info.lane_map);
       }
       if (link_sts == 1){
           printf("PASSED: bcm_plp_link_status_get phy_id %d and phy_info.lane_map 0x%x\n", phy_id, phy_info.lane_map);
       } else {
           printf("FAILED: bcm_plp_link_status_get phy_id %d and phy_info.lane_map 0x%x\n", phy_id, phy_info.lane_map);
       }
   }
   phy_info.if_side = LINE_SIDE_INTERFACE;
   printf("=========================Link Status if_side if_side %d================================================\n", phy_info.if_side);
   for (phy_id =0;  phy_id <= no_phy_ids ; phy_id++) {
       phy_info.phy_addr=phy_id;
       rv = bcm_plp_quadra28_link_status_get((*(bcm_plp_quadra28_access_t*) (&phy_info)), &link_sts);
       if (rv != 0) {
           printf("bcm_plp_quadra28_link_status_get failed with rv:%d phy_info.lane_map: 0x%x\n", rv, phy_info.lane_map);

       } else {
           printf("Link status = %d on phy_info.lane_map map = 0x%x\n", link_sts, phy_info.lane_map);
       }
       if (link_sts == 1){
           printf("PASSED: bcm_plp_link_status_get phy_id %d and phy_info.lane_map 0x%x\n", phy_id, phy_info.lane_map);
       } else {
           printf("FAILED: bcm_plp_link_status_get phy_id %d and phy_info.lane_map 0x%x\n", phy_id, phy_info.lane_map);
       }
   }

    phy_info.if_side = SYSTEM_SIDE_INTERFACE;
    printf("=========================Eys Scan if_side if_side %d================================================\n", phy_info.if_side);
    for (phy_id =0;  phy_id <= no_phy_ids ; phy_id++) {
 	    phy_info.phy_addr = phy_id;
        phy_info.lane_map = lane;
        rv = bcm_plp_quadra28_display_eye_scan((*(bcm_plp_quadra28_access_t*) (&phy_info)));
        if (rv != 0) {
            printf("bcm_plp_quadra28_display_eye_scan failed with rv:%d lane: 0x%x\n", rv, lane);
            return rv;
        }
    }
    phy_info.if_side = LINE_SIDE_INTERFACE;
    printf("=========================Eye Scan if_side if_side %d================================================\n", phy_info.if_side);
    for (phy_id =0;  phy_id <= no_phy_ids ; phy_id++) {
 	    phy_info.phy_addr = phy_id;
        phy_info.lane_map = lane;
        rv = bcm_plp_quadra28_display_eye_scan((*(bcm_plp_quadra28_access_t*) (&phy_info)));
        if (rv != 0) {
            printf("bcm_plp_quadra28_display_eye_scan failed with rv:%d lane: 0x%x\n", rv, lane);
            return rv;
        }
    }

    phy_info.if_side = LINE_SIDE_INTERFACE;
    for (phy_id =0;  phy_id <= no_phy_ids ; phy_id++) {
        phy_info.phy_addr=phy_id;
        printf("=====================DSC phy_info.if_side %d phy_info.lane_map 0x%x===================================\n", phy_info.if_side, lane);
        rv = bcm_plp_quadra28_phy_diagnostics_get((*(bcm_plp_quadra28_access_t*) (&phy_info)),
                                                  (bcm_plp_quadra28_pm_phy_diagnostics_t*)&diag);
        if (rv != 0) {
            printf("bcm_plp_quadra28_phy_diagnostics_get failed with rv:%d phy_info.lane_map: 0x%x\n", rv, lane);
            return rv;
        }
        printf("signal_detect             = \t0x%x\n", diag.signal_detect);
        printf("vga_bias_reduced          = \t0x%x\n", diag.vga_bias_reduced);
        printf("postc_metric              = \t0x%x\n", diag.postc_metric);
        printf("osr_mode                  = \t0x%x\n", diag.osr_mode);
        printf("pmd_mode                  = \t0x%x\n", diag.rx_lock);
        printf("rx_ppm                    = \t0x%x\n", diag.rx_ppm);
        printf("tx_ppm                    = \t0x%x\n", diag.tx_ppm);
        printf("clk90_offset              = \t0x%x\n", diag.clk90_offset);
        printf("clkp1_offset              = \t0x%x\n", diag.clkp1_offset);
        printf("p1_lvl                    = \t0x%x\n", diag.p1_lvl);
        printf("m1_lvl                    = \t0x%x\n", diag.m1_lvl);
        printf("dfe1_dcd                  = \t0x%x\n", diag.dfe1_dcd);
        printf("dfe2_dcd                  = \t0x%x\n", diag.dfe2_dcd);
        printf("slicer_target             = \t0x%x\n", diag.slicer_target);
        printf("slicer_offset:offset_pe   = \t0x%x\n", diag.slicer_offset.offset_pe);
        printf("slicer_offset:offset_ze   = \t0x%x\n", diag.slicer_offset.offset_ze);
        printf("slicer_offset:offset_me   = \t0x%x\n", diag.slicer_offset.offset_me);
        printf("slicer_offset:offset_po   = \t0x%x\n", diag.slicer_offset.offset_po);
        printf("slicer_offset:offset_zo   = \t0x%x\n", diag.slicer_offset.offset_zo);
        printf("slicer_offset:offset_mo   = \t0x%x\n", diag.slicer_offset.offset_mo);
        printf("eyescan:heye_left         = \t0x%x\n", diag.eyescan.heye_left);
        printf("eyescan:heye_right        = \t0x%x\n", diag.eyescan.heye_right);
        printf("eyescan:veye_upper        = \t0x%x\n", diag.eyescan.veye_upper);
        printf("eyescan:veye_lower        = \t0x%x\n", diag.eyescan.veye_lower);
        printf("state_machine_status      = \t0x%x\n", diag.state_machine_status);
        printf("link_time                 = \t0x%x\n", diag.link_time);
        printf("pf_main                   = \t0x%x\n", diag.pf_main);
        printf("pf_hiz                    = \t0x%x\n", diag.pf_hiz);
        printf("pf_bst                    = \t0x%x\n", diag.pf_bst);
        printf("pf_low                    = \t0x%x\n", diag.pf_low);
        printf("pf2_ctrl                  = \t0x%x\n", diag.pf2_ctrl);
        printf("vga                       = \t0x%x\n", diag.vga);
        printf("dc_offset                 = \t0x%x\n", diag.dc_offset);
        printf("p1_lvl_ctrl               = \t0x%x\n", diag.p1_lvl_ctrl);
        printf("dfe1                      = \t0x%x\n", diag.dfe1);
        printf("dfe2                      = \t0x%x\n", diag.dfe2);
        printf("dfe3                      = \t0x%x\n", diag.dfe3);
        printf("dfe4                      = \t0x%x\n", diag.dfe4);
        printf("dfe5                      = \t0x%x\n", diag.dfe5);
        printf("dfe6                      = \t0x%x\n", diag.dfe6);
        printf("txfir_pre                 = \t0x%x\n", diag.txfir_pre);
        printf("txfir_main                = \t0x%x\n", diag.txfir_main);
        printf("txfir_post1               = \t0x%x\n", diag.txfir_post1);
        printf("txfir_post2               = \t0x%x\n", diag.txfir_post2);
        printf("txfir_post3               = \t0x%x\n", diag.txfir_post3);
        printf("tx_amp_ctrl               = \t0x%x\n", diag.tx_amp_ctrl);
        printf("br_pd_en                  = \t0x%x\n", diag.br_pd_en);
    }

    phy_info.if_side = SYSTEM_SIDE_INTERFACE;
    for (phy_id =0;  phy_id <= no_phy_ids; phy_id++) {
        phy_info.phy_addr=phy_id;
        printf("=====================DSC phy_info.if_side %d phy_info.lane_map 0x%x===================================\n", phy_info.if_side, lane);
        rv = bcm_plp_quadra28_phy_diagnostics_get((*(bcm_plp_quadra28_access_t*) (&phy_info)),
                                                  (bcm_plp_quadra28_pm_phy_diagnostics_t*)&diag);
        if (rv != 0) {
            printf("bcm_plp_quadra28_phy_diagnostics_get failed with rv:%d phy_info.lane_map: 0x%x\n", rv, lane);
            return rv;
        }
        printf("signal_detect             = \t0x%x\n", diag.signal_detect);
        printf("vga_bias_reduced          = \t0x%x\n", diag.vga_bias_reduced);
        printf("postc_metric              = \t0x%x\n", diag.postc_metric);
        printf("osr_mode                  = \t0x%x\n", diag.osr_mode);
        printf("pmd_mode                  = \t0x%x\n", diag.rx_lock);
        printf("rx_ppm                    = \t0x%x\n", diag.rx_ppm);
        printf("tx_ppm                    = \t0x%x\n", diag.tx_ppm);
        printf("clk90_offset              = \t0x%x\n", diag.clk90_offset);
        printf("clkp1_offset              = \t0x%x\n", diag.clkp1_offset);
        printf("p1_lvl                    = \t0x%x\n", diag.p1_lvl);
        printf("m1_lvl                    = \t0x%x\n", diag.m1_lvl);
        printf("dfe1_dcd                  = \t0x%x\n", diag.dfe1_dcd);
        printf("dfe2_dcd                  = \t0x%x\n", diag.dfe2_dcd);
        printf("slicer_target             = \t0x%x\n", diag.slicer_target);
        printf("slicer_offset:offset_pe   = \t0x%x\n", diag.slicer_offset.offset_pe);
        printf("slicer_offset:offset_ze   = \t0x%x\n", diag.slicer_offset.offset_ze);
        printf("slicer_offset:offset_me   = \t0x%x\n", diag.slicer_offset.offset_me);
        printf("slicer_offset:offset_po   = \t0x%x\n", diag.slicer_offset.offset_po);
        printf("slicer_offset:offset_zo   = \t0x%x\n", diag.slicer_offset.offset_zo);
        printf("slicer_offset:offset_mo   = \t0x%x\n", diag.slicer_offset.offset_mo);
        printf("eyescan:heye_left         = \t0x%x\n", diag.eyescan.heye_left);
        printf("eyescan:heye_right        = \t0x%x\n", diag.eyescan.heye_right);
        printf("eyescan:veye_upper        = \t0x%x\n", diag.eyescan.veye_upper);
        printf("eyescan:veye_lower        = \t0x%x\n", diag.eyescan.veye_lower);
        printf("state_machine_status      = \t0x%x\n", diag.state_machine_status);
        printf("link_time                 = \t0x%x\n", diag.link_time);
        printf("pf_main                   = \t0x%x\n", diag.pf_main);
        printf("pf_hiz                    = \t0x%x\n", diag.pf_hiz);
        printf("pf_bst                    = \t0x%x\n", diag.pf_bst);
        printf("pf_low                    = \t0x%x\n", diag.pf_low);
        printf("pf2_ctrl                  = \t0x%x\n", diag.pf2_ctrl);
        printf("vga                       = \t0x%x\n", diag.vga);
        printf("dc_offset                 = \t0x%x\n", diag.dc_offset);
        printf("p1_lvl_ctrl               = \t0x%x\n", diag.p1_lvl_ctrl);
        printf("dfe1                      = \t0x%x\n", diag.dfe1);
        printf("dfe2                      = \t0x%x\n", diag.dfe2);
        printf("dfe3                      = \t0x%x\n", diag.dfe3);
        printf("dfe4                      = \t0x%x\n", diag.dfe4);
        printf("dfe5                      = \t0x%x\n", diag.dfe5);
        printf("dfe6                      = \t0x%x\n", diag.dfe6);
        printf("txfir_pre                 = \t0x%x\n", diag.txfir_pre);
        printf("txfir_main                = \t0x%x\n", diag.txfir_main);
        printf("txfir_post1               = \t0x%x\n", diag.txfir_post1);
        printf("txfir_post2               = \t0x%x\n", diag.txfir_post2);
        printf("txfir_post3               = \t0x%x\n", diag.txfir_post3);
        printf("tx_amp_ctrl               = \t0x%x\n", diag.tx_amp_ctrl);
        printf("br_pd_en                  = \t0x%x\n", diag.br_pd_en);
    }

    phy_info.if_side = LINE_SIDE_INTERFACE;
    printf("=========================phy_info.if_side %d================================================\n", phy_info.if_side);
    printf("----------------------PRBS CLEAR---------------------------------------------------\n");
    for (phy_id =0;  phy_id <= no_phy_ids ; phy_id++) {
        phy_info.phy_addr=phy_id;
        rv = bcm_plp_quadra28_prbs_clear((*(bcm_plp_quadra28_access_t*) (&phy_info)), tx_rx);
        if (rv != 0) {
            printf("bcm_plp_quadra28_prbs_clear failed with rv:%d phy_info.lane_map: 0x%x\n", rv, phy_info.lane_map);
            return rv;
        }
        printf ("PASSED: prbs clear pass on phy_id:%d phy_info.lane_map:0x%x\n", phy_id, phy_info.lane_map);
    }
    phy_info.if_side = SYSTEM_SIDE_INTERFACE;
    printf("=========================phy_info.if_side %d================================================\n", phy_info.if_side);
    printf("----------------------PRBS CLEAR---------------------------------------------------\n");
    for (phy_id =0;  phy_id <= no_phy_ids ; phy_id++) {
        phy_info.phy_addr=phy_id;
        rv = bcm_plp_quadra28_prbs_clear((*(bcm_plp_quadra28_access_t*) (&phy_info)), tx_rx);
        if (rv != 0) {
            printf("bcm_plp_quadra28_prbs_clear failed with rv:%d phy_info.lane_map: 0x%x\n", rv, phy_info.lane_map);
            return rv;
        }
        printf ("PASSED: prbs clear pass on phy_id:%d phy_info.lane_map:0x%x\n", phy_id, phy_info.lane_map);
    }

    return rv;
}

int miura_eye_diagram(void)
{
    int rv = 0;

    printf("=========================Eys Scan (system side)================================================\n");
    for (lane = 0; lane < 2; lane ++) 
    {
        /* Filling plp_info */  
        plp_info.phy_addr = 0;
        plp_info.if_side  = SYS_SIDE;
        plp_info.lane_map = 1 << lane;
        
        rv = bcm_plp_display_eye_scan("miura", plp_info);   
        if (rv)
        {
            printf("bcm_plp_display_eye_scan failed for PHY-ID [%d], lane: 0x%x, return code [%d] \n", plp_info.phy_addr, lane, rv);
            return rv;
        }
    }

    printf("=========================Eys Scan (line side)================================================\n");
    for (lane = 0; lane < 2; lane ++) 
    {
        /* Filling plp_info */  
        plp_info.phy_addr = 0;
        plp_info.if_side  = LINE_SIDE;
        plp_info.lane_map = 1 << lane;
        
        rv = bcm_plp_display_eye_scan("miura", plp_info);   
        if (rv)
        {
            printf("bcm_plp_display_eye_scan failed for PHY-ID [%d], lane: 0x%x, return code [%d] \n", plp_info.phy_addr, lane, rv);
            return rv;
        }
    }
    return rv;
}
/*-------------------------------------------------
$Log: bcm82752_api.c,v $
Revision 1.2  2018/05/18 09:24:56  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.1.2.22  2018/02/06 07:31:26  meho
Changed bcm82757 loopback util from global to remote lpbk

Revision 1.1.2.21  2018/01/24 09:34:59  meho
Added BCM82757 emphasis setting in FW download.

Revision 1.1.2.20  2018/01/24 01:38:53  meho
Added BCM82757 line side loopback configuration utility.

Revision 1.1.2.19  2017/09/07 06:46:46  meho
1. Fixed dump BCM82752 register bug.
2. Added dump BCM82757 register utility.
3. Added BCM82757 indirect register r/w utility.

Revision 1.1.2.18  2017/07/11 06:45:57  meho
Fixed PRRQ commnet.

Revision 1.1.2.17  2017/04/10 05:27:24  meho
Integrated BCM82752/82757 API.

Revision 1.1.2.16  2017/01/25 12:32:07  meho
Changed lane number of specified PHY-ID in eye diagram util.

Revision 1.1.2.15  2017/01/25 11:43:54  meho
Renaming the BCM API to 10G Eye Scan.

Revision 1.1.2.13  2017/01/11 02:16:51  meho
Return fail when 10G PHY FW download fail.

Revision 1.1.2.12  2016/12/28 09:07:23  meho
Changed the test name of loopback test.

Revision 1.1.2.11  2016/12/27 08:22:42  meho
Corrected the print Pass location.

Revision 1.1.2.10  2016/12/27 02:01:42  meho
Added ge-Int loopback flag to control Cavium GE int/ext loopback test.

Revision 1.1.2.9  2016/11/29 06:27:52  meho
Changed submenu name and code clean up.

Revision 1.1.2.8  2016/08/11 12:01:35  meho
Added BCM LIB: libphymodepil.a

Revision 1.1.2.7  2016/07/26 10:09:43  meho
Added 10G PHY PTP1588 loopback test skeleton.

Revision 1.1.2.6  2016/07/18 06:55:45  meho
Added read SFP module type from eeprom api.

Revision 1.1.2.5  2016/07/12 08:40:58  meho
1. Added BCM54194/BCM82752 register tests.
2. Added BCM54194 internal/external-copper loopback configuration.

Revision 1.1.2.4  2016/07/07 09:04:29  meho
1. Added BCM54194 RDB register r/w utility.
2. Added GE PHY internal/external loopback skeleton.
3. Added 10GE PHY internal/external loopback skeleton.

Revision 1.1.2.3  2016/06/22 10:40:26  meho
Added GE/10GE PHY r/w utilities.

Revision 1.1.2.2  2016/06/17 10:15:33  bowang3
Add a few more APIs for BCM 10G PHY

Revision 1.1.2.1  2016/06/12 10:31:06  bowang3
Add bcm82752 10G PHY code framework

$Endlog$
*/
