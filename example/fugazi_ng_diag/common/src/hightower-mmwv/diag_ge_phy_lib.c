/* $Id: diag_ge_phy_lib.c,v 1.2 2021/01/25 09:21:42 markzha Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/hightower-mmwv/diag_ge_phy_lib.c,v $
 *-----------------------------------------------------------------------------
 *
 * Filename   : platform_ge_phy.c
 * Description: Chrysler GE PHY(Marvell 88E1514) Library.
 *
 * Copyright (c) 2017 - 2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *-----------------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include "error.h"
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

#include "common.h"
#include "common_utils.h"
#include "types.h"
#include "nvmonvars.h"
#include "plat_defs.h"
#include "ethernet.h"
#include "queryflags.h"
#include "diag_ge_phy_lib.h"
#include "diag_ge_phy_test.h"
#include "platform_i2c.h"
#include "proto.h"
#include "byteswap.h"
#include "dev_88e151x.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

/*******************************************************************************
 *                                 Externs                                      
 *******************************************************************************
 */
extern int diag_ge_phy_no;
int chrysler_gephy_reg_wr_util(int);
int chrysler_gephy_reg_rd_util(int);
int dnv_eth_link_is_up(int);
int check_ext_lpbk_flag(void);

int diag_gephy_dev_create(int, dev_88e151x_object_t *);
uint32 diag_gephy_smi_rd(uint32, ushort *);
uint32 diag_gephy_smi_wr(uint32, ushort);

/*******************************************************************************
 *                                  Global                                      
 *******************************************************************************
 */

/*******************************************************************************
 *                                  Function                                      
 *******************************************************************************
 */

/*******************************************************************************
 *
 * Function    : plat_mem_read32
 * Description : Function to read memory by byte.
 * Inputs      : offset - memory offset
 *               *buf   - buffer to put read back register value
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int plat_mem_read32 (uint offset, uint *buf)
{
    int      fd = -1;
    void     *map_base, *virt_addr;
    off_t    target = 0;
    unsigned map_size, page_size, offset_in_page;

    target = (off_t)offset;

    fd = open("/dev/mem", (O_RDONLY | O_SYNC));
    if (fd < 0) {
        printf("%s: Failed to open MEM device.\n", __FUNCTION__);
        return (FAILED);
    }

    map_size = page_size = getpagesize();
    offset_in_page = (unsigned)target & (page_size - 1);
    if (offset_in_page + (8 * sizeof(int)) > page_size) {
        map_size *= 2;
    }

    map_base = mmap(NULL, map_size, PROT_READ, MAP_SHARED, fd,
                    target & ~(off_t)(page_size -1));

    if (map_base == MAP_FAILED) {
            printf("%s: Failed to map in virtual address space.\n",
                   __FUNCTION__);
            close(fd);
            return (FAILED);
    }

    virt_addr = (char *)map_base + offset_in_page;
    *buf = *(volatile uint32_t*)virt_addr;

    if (munmap(map_base, map_size) == -1) {
        printf("%s: Failed to munmap.\n", __FUNCTION__);
        close(fd);
        return (FAILED);
    }
    close(fd);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : plat_mem_write32
 * Description : Function performs write memory by byte.
 * Inputs      : offset  - offset
 *               wr_data - data for write
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int plat_mem_write32 (uint offset, uint wr_data)
{
    int      fd = -1;
    void     *map_base, *virt_addr;
    off_t    target = 0;
    unsigned map_size, page_size, offset_in_page;

    target = (off_t)offset;

    fd = open("/dev/mem", (O_RDWR | O_SYNC));
    if (fd < 0) {
        printf("%s: Failed to open MEM device.\n", __FUNCTION__);
        return (FAILED);
    }

    map_size = page_size = getpagesize();
    offset_in_page = (unsigned)target & (page_size - 1);
    if (offset_in_page + (8 * sizeof(int)) > page_size) {
        map_size *= 2;
    }

    map_base = mmap(NULL, map_size, (PROT_READ | PROT_WRITE), MAP_SHARED, fd,
                    target & ~(off_t)(page_size -1));

    if (map_base == MAP_FAILED) {
            printf("%s: Failed to map in virtual address space.\n",
                   __FUNCTION__);
            close(fd);
            return (FAILED);
    }

    virt_addr = (char *)map_base + offset_in_page;
    *(volatile uint32_t*)virt_addr = wr_data;

    if (munmap(map_base, map_size) == -1) {
        printf("%s: Failed to munmap.\n", __FUNCTION__);
        close(fd);
        return (FAILED);
    }
    close(fd);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : plat_smi_read
 * Description: Function to do SMI read.
 * Inputs     : phy_addr - PHY device address
 *              reg_addr - PHY device register address
 *              *buf     - buffer to put the read back register value
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int plat_smi_read (int phy_addr, int reg_addr, ushort *buf)
{
    int    ctr = 0;
    uint   reg_val = 0;
    uint   smi_regaddr = (uint)CN9130_SMI_REG;
    uint addr_mask = (uint)(SMIMR_REGAD | SMIMR_PHYAD);
    uint expect_addr = 0;

    expect_addr = (uint)(((reg_addr & SMIMR_REGAD_MSK) << SMIMR_REGAD_OFFSET) |
                         ((phy_addr & SMIMR_PHYAD_MSK) << SMIMR_PHYAD_OFFSET));

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s:%d phy_addr = 0x%08X.\n", __FUNCTION__, __LINE__, phy_addr);
    }

    /* Comfirm SMI bus is ready for access. */
    for (ctr = 0; ctr < PLAT_SMI_RETRY_MAX; ctr++) {
        if (plat_mem_read32(smi_regaddr, &reg_val) != PASSED) {
            printf("%s:%d Failed to read CPU SMI Management Reg.(0x%08X).\n",
                   __FUNCTION__, __LINE__, smi_regaddr);
            return (FAILED);
        }

        if ((reg_val & CN9130_SMI_BUSY) == 0) {
            break;
        } else {
            if (ctr == (PLAT_SMI_RETRY_MAX - 1)) {
                printf("TIME OUT !! SMI Bus is still busy.\n");
                return (FAILED);
            }
        }
    }

    /* Package content */
    reg_val = 0;
    reg_val = (uint)((CN9130_SMI_OPCODE_RD) |
                     ((reg_addr & 0x1f) << 21) |
                     ((phy_addr & 0x1f) << 16));

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s:%d write in 0x%08X.\n", __FUNCTION__, __LINE__, reg_val);
    }

    /* Write SMI command package */
    if (plat_mem_write32(smi_regaddr, reg_val) != PASSED) {
        printf("%s:%d Failed to write CPU SMI Management Reg.(0x%08X).\n",
               __FUNCTION__, __LINE__, smi_regaddr);
        return (FAILED);
    }

    for (ctr = 0; ctr < PLAT_SMI_RETRY_MAX; ctr++) {
        /* Do SMI read */
        reg_val = 0;
        if (plat_mem_read32(smi_regaddr, &reg_val) != PASSED) {
            printf("%s:%d Failed to read CPU SMI Management Reg.(0x%08X).\n",
                   __FUNCTION__, __LINE__, smi_regaddr);
            return (FAILED);
        }

        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("%s:%d read back = 0x%08X.\n",
                   __FUNCTION__, __LINE__, reg_val);
        }

        if ((reg_val & CN9130_SMI_READ_VALID) == CN9130_SMI_READ_VALID) {
            if ((reg_val & addr_mask) == expect_addr) {
                break;
            }
        } else {
            if (ctr == (PLAT_SMI_RETRY_MAX - 1)) {
                printf("Failed !! Read back data is invalid.\n");
                return (FAILED);
            }
        }
    }
    *buf = (short)(reg_val & 0xffff);

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : plat_smi_write
 * Description: Function to do SMI read.
 * Inputs     : phy_addr - PHY device address
 *              reg_addr - PHY device register address
 *              w_data   - data that wanted to write in
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int plat_smi_write (int phy_addr, int reg_addr, ushort w_data)
{
    int    ctr = 0;
    uint   reg_val = 0;
    uint   smi_regaddr = (uint)CN9130_SMI_REG;

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s(%d): phy_addr = 0x%08X.\n", __func__, __LINE__, phy_addr);
    }

    /* Comfirm SMI bus is ready for access. */
    for (ctr = 0; ctr < PLAT_SMI_RETRY_MAX; ctr++) {
        reg_val = (uint)CN9130_SMI_BUSY;
        if (plat_mem_read32(smi_regaddr, &reg_val) != PASSED) {
            printf("%s(%d): Failed to read CPU SMI Management Reg.(0x%08X).\n",
                   __func__, __LINE__, smi_regaddr);
            return (FAILED);
        }

        if ((reg_val & CN9130_SMI_BUSY) == 0) {
            break;
        } else {
            if (ctr == (PLAT_SMI_RETRY_MAX - 1)) {
                printf("TIME OUT !! SMI Bus is still busy.\n");
                return (FAILED);
            }
        }
    }

    /* Package content */
    reg_val = 0;
    reg_val = (uint)(((reg_addr & 0x1f) << 21) |
                     ((phy_addr & 0x1f) << 16) |
                     w_data);
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s(%d): write in 0x%08X.\n", __func__, __LINE__, reg_val);
    }

    /* Write SMI command package */
    if (plat_mem_write32(smi_regaddr, reg_val) != PASSED) {
        printf("%s(%d): Failed to write CPU SMI Management Reg.(0x%08X).\n",
               __func__, __LINE__, smi_regaddr);
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : diag_gephy_smi_rd
 * Description : Function to read PHY register through SMI
 * Inputs      : addr - Register Address
 *               buf - pointer to the buffer
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
uint32 diag_gephy_smi_rd (uint32 addr, ushort *buf)
{
    return (plat_smi_read(CHRYSLER_1514_GE_PHY_ADDR, addr, buf));
}

/*******************************************************************************
 *
 * Function    : diag_gephy_smi_wr
 * Description : Function to read PHY register through SMI
 * Inputs      : addr - Register Address
 *               data - Data to be written to PHY register
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
uint32 diag_gephy_smi_wr (uint32 addr, ushort data)
{
    return (plat_smi_write(CHRYSLER_1514_GE_PHY_ADDR, addr, data));
}

/*******************************************************************************
 *
 * Function    : diag_gephy_dev_create
 * Description : Function to create 88E151X Device Object
 * Inputs      : phy_no    - GE PHY 0 or PHY 1
 *               gephy_obj - Pointer of 88E151X device driver object
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int diag_gephy_dev_create (int phy_no, dev_88e151x_object_t *gephy_obj)
{
    dev_object_t *dev = (dev_object_t *)gephy_obj;

    /* Create common device object */
    mrv88e151x_dev_create(dev, (dev_error_report_t)err_report);

    if (dev == NULL) {
        return (FAILED);
    }

    /* Attach the device */
    gephy_obj->base.dev_object_fvt->dev_attach(dev);

    /* Setup call-out function vectors */
    gephy_obj->callout_fvt->rd = diag_gephy_smi_rd;
    gephy_obj->callout_fvt->wr = diag_gephy_smi_wr;

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : chrysler_gephy_reg_rd_util
 * Description: Utility to read Chrysler GE PHY(Marvell 88E1514) register.
 * Inputs     : eth_num - ethernet number
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int chrysler_gephy_reg_rd_util (int eth_num)
{
    dev_88e151x_object_t gephy_obj;
    dev_88e151x_object_t *gephy_obj_p = &gephy_obj;
    int rc = PASSED;

    rc = diag_gephy_dev_create(diag_ge_phy_no, gephy_obj_p);

    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed", __func__);
        return (rc);
    }

    rc = gephy_obj_p->callin_fvt->display_register((dev_object_t *)gephy_obj_p);

    gephy_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&gephy_obj_p);

    return (rc);
}

/*******************************************************************************
 *
 * Function   : chrysler_gephy_reg_wr_util
 * Description: Utility to write Chrysler GE PHY(Marvell 88E1514) register.
 * Inputs     : eth_num - ethernet number
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int chrysler_gephy_reg_wr_util (int eth_num)
{
    dev_88e151x_object_t gephy_obj;
    dev_88e151x_object_t *gephy_obj_p = &gephy_obj;
    int rc = PASSED;

    rc = diag_gephy_dev_create(diag_ge_phy_no, gephy_obj_p);

    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed", __func__);
        return (FAILED);
    }

    rc = gephy_obj_p->callin_fvt->alter_register((dev_object_t *)gephy_obj_p);

    gephy_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&gephy_obj_p);

    return (rc);
}

/******************************************************************************
 *
 * Function: dnv_eth_link_is_up
 *
 * Description: Return True if link is up, otherwise link is down
 *
 * Inputs      : phy_no - PHY Number
 * Outputs     : TRUE/FALSE
 *
 *****************************************************************************/
int dnv_eth_link_is_up (int phy_no)
{
    /* PHY 1514 is ETH1 */
    char iface_name[32] = "eth1";
    struct ifreq ifr;
    struct ethtool_value edata;
    int fd, ret;

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        printf("%s: Open Socket failed\n", __func__);
        return (FALSE);
    }

    memset(&ifr, 0, sizeof(ifr));
    strcpy(ifr.ifr_name, iface_name);

    edata.cmd = ETHTOOL_GLINK;
    ifr.ifr_data = (caddr_t)&edata;

    ret = ioctl(fd, SIOCETHTOOL, &ifr);
    if (ret != 0) {
        printf("%s: Can't get device settings\n", __func__);
        close(fd);
        return (FALSE);
    }

    close(fd);

    if (edata.data) {
        return (TRUE);
    } else {
        return (FALSE);
    }
}

/*******************************************************************************
 *
 * Function   : check_ext_lpbk_flag
 * Description: Function to check if Ext. Loopback Flag is ON or not.
 * Inputs     : None
 * Outputs    : TRUE(ON) / FALSE(OFF)
 *
 *******************************************************************************
 */
int check_ext_lpbk_flag (void)
{
    /* according to menu_show_dflags(), D_EXT_LPBK is inverse flag */
    if ((NVRAM)->diagflag & D_EXT_LOOPBACK) {
        return (FALSE);
    } else {
        return (TRUE);
    }
}

/*-------------------------------------------------
* $Log: diag_ge_phy_lib.c,v $
* Revision 1.2  2021/01/25 09:21:42  markzha
* Sync RDT issues fixing and optimize compiling for Highrise
*
* Revision 1.1  2020/08/19 09:50:04  markzha
* *** empty log message ***
*
* $Endlog$
*-------------------------------------------------
*/

