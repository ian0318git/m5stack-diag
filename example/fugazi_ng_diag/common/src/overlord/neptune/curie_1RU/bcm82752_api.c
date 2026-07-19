/* $Id: bcm82752_api.c,v 1.2 2019/08/06 06:56:11 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_1RU/bcm82752_api.c,v $
 *-----------------------------------------------------------------------------
 * bcm82752_api.c - API for BCM 10G PHY bcm82752.
 *          Leverage from KP
 *
 * Feb 2019, Leschen
 *
 * Copyright (c) 2016 - 2019 by Cisco Systems, Inc.
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
#include "ethernet.h"
#include "linux_eth.h"
#include "platform_eth.h"
#include "bcm57412_lib.h"
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

extern struct curie_bcm82752 *curie;
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
    int rc;
    uint16_t data;

    rc = curie_bnxt_mdio_read(&curie->bnxt[0], port, dev, reg, &data);

    if (rc < 0) {
        printf("BCM82752 read error\n");
        return (FAILED);
    }
    return (data);
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
    int rc;
	
    rc = curie_bnxt_mdio_write(&curie->bnxt[0], port, dev, reg, val);

    if (rc < 0) {
        printf("BCM82752 write error\n");
        return (FAILED);
    }
    return (PASSED);
}

int curie_bcm82752_set_cl37_an(struct curie_bcm82752 *curie, int port, int enable)
{
    uint32_t data;
    struct curie_quadra28 *q28 = &curie->quadra28[port];
    bcm_plp_access_t *info = &q28->info;

    bcm_plp_reg_value_get(q28->type, *info, 1, 0x0009, &data);
    data |= 1;
    bcm_plp_reg_value_set(q28->type, *info, 1, 0x0009, data);

    bcm_plp_reg_value_get(q28->type, *info, 7, 0xffe0, &data);

    if (enable) {
        data |= (1 << 12);
    } else {
        data &= ~(1 << 12);
    }

    bcm_plp_reg_value_set(q28->type, *info, 7, 0xffe0, data);

    bcm_plp_reg_value_get(q28->type, *info, 1, 0x0009, &data);
    data &= ~1;
    bcm_plp_reg_value_set(q28->type, *info, 1, 0x0009, data);

    return 0;
}

int curie_bcm82752_check_cl37_an(struct curie_bcm82752 *curie, int port, int *an, int *link, int *done)
{
    uint32_t data;
    struct curie_quadra28 *q28 = &curie->quadra28[port];
    bcm_plp_access_t *info = &q28->info;

    bcm_plp_reg_value_get(q28->type, *info, 7, 0xffe0, &data);
    *an = (data & 0x1000) ? 1 : 0;

    bcm_plp_reg_value_get(q28->type, *info, 7, 0xffe1, &data);
    bcm_plp_reg_value_get(q28->type, *info, 7, 0xffe1, &data);

    *link = (data & 0x0004) ? 1 : 0;
    *done = (data & 0x0020) ? 1 : 0;

    return 0;
}

int bcm82752_emphasis_setting (void)
{
    ushort wrval;
    uint rv = FAILED, phy_addr, dev_id = BCM82752_DEV_PMA, regnum;
    struct curie_quadra28 *q28;

    for (phy_addr = 0; phy_addr < 2; phy_addr++) {
        q28 = &curie->quadra28[phy_addr];
        bcm_plp_access_t *info = &q28->info;

        regnum = BCM82757_TX_CTRL5_REG;
        wrval = 0x2000;
        rv = bcm_plp_reg_value_set(q28->type, *info, dev_id, regnum, wrval);
        if (rv != PASSED) {
            return (-1);
        }

        regnum = BCM82757_TX_FIR_CTRL1_REG;
        wrval = 0x00E0;
        rv = bcm_plp_reg_value_set(q28->type, *info, dev_id, regnum, wrval);
        if (rv != PASSED) {
            return (-1);
        }

        regnum = BCM82757_TX_FIR_CTRL2_REG;
        wrval = 0x8028;
        rv = bcm_plp_reg_value_set(q28->type, *info, dev_id, regnum, wrval);
        if (rv != PASSED) {
            return (-1);
        }
    }
    return (rv);
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

int is_bcm82752()
{
    int xcvr_reg_data, port = 0x10, phy_id;

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

int curie_bcm82752_link_status(struct curie_bcm82752 *curie,
                                  int port, curie_if_side_t if_side,
                                  unsigned int *link_status)
{
    struct curie_quadra28 *q28 = &curie->quadra28[port];
    bcm_plp_access_t *info = &q28->info;

    info->if_side = CURIE_IF_SIDE_TO_QUADRA28(if_side);
    return bcm_plp_link_status_get(q28->type, *info, link_status);
}

int bcm82752_is_link_up(int port, unsigned int *link_up)
{
    int rc = FAILED;
    int ix;
    curie_if_side_t if_side;

    for (ix = 0; ix < 6; ix++) {
        if_side = CURIE_IF_SIDE_SYS;
        rc = curie_bcm82752_link_status(curie, port, if_side, link_up);
        if (rc < 0) {
            if (ix == 0x5) {
                cterr('f', 0, "bcm82752 get sys side link status failed");
                return (rc);
            }
        }
        if (!(*link_up)) {
            msleep(2000);
            continue;
        }
        break;
    }

    for (ix = 0; ix < 6; ix++) {
        if_side = CURIE_IF_SIDE_LINE;
        rc = curie_bcm82752_link_status(curie, port, if_side, link_up);
        if (rc < 0) {
            if (ix == 0x5) {
                cterr('f', 0, "bcm82752 get line side link status failed");
                return (rc);
            }
        }
        if (!(*link_up)) {
            msleep(2000);
            continue;
        }
        break;
    }

    return (rc);
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

/*-------------------------------------------------
$Log: bcm82752_api.c,v $
Revision 1.2  2019/08/06 06:56:11  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.1.2.2  2019/06/27 06:30:17  leschen
Support CL73 utilities.

Revision 1.1.2.1  2019/03/12 07:41:51  leschen
Initial check in to support BCM82752


$Endlog$
*/
