/* $Id: diag_common_drv.c,v 1.1 2015/02/26 07:18:29 xiaoyizh Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/wallander/diag_common_drv.c,v $
 *------------------------------------------------------------------
 * diag_common_drv.c - Device Driver Library
 * Mar 2014, Xiaoying Zhang
 *
 * Copyright (c) 2015 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "defs.h"
#include "types.h"
#include "error.h"
#include "common.h"
#include "common_utils.h"
#include "ethernet.h"
#include "platform_eth.h"
#include "cvmx.h"
#include "cvmx-mdio.h"
#include "cvmx-twsi.h"
#include "cvmx-csr-db.h"
#include "diag_common_drv.h"
#include "diag_vtss_phy.h"

/***********************************************************************
 *  Macro Definitions
 ************************************************************************/

/***********************************************************************
 *  Static Functions Declaration
 ************************************************************************/
uchar *smi_mmap_addr = NULL;

//static int mrv88e1340_phy_read_smi(uint, uint, uint, uint *);
//static int mrv88e1340_phy_write_smi(uint, uint, uint, uint);
static inline int user_cvmx_mdio_write(int, int, int, int);
static inline int user_cvmx_mdio_read(int, int, int);

/***********************************************************************
 *  Externs
 ************************************************************************/
extern int memops_mmap(ulong, ulong, ulong *, ulong *);
extern uint32 err_report(dev_object_t *, char *, uint32);

/*
 * Function: disable_pcs_interrupt
 *
 * Description:
 * Disable PCS interrupt
 *
 * Input:
 * block_id - block ID (0/1)
 * mask - mask to disable the interrupt
 *
 * Return: None
 */
void disable_pcs_interrupt(int block_id, uint64_t mask)
{
//     uint64_t addr64;
    uint64_t data;

    int i = 0;
    for (i = 0; i < 4; i++) {
        data = cvmx_read_csr(CVMX_PCSX_INTX_EN_REG(i, block_id));
//         printf("cvmx_read_csr(%#llx) = 0x%x\n", CVMX_PCSX_INTX_EN_REG(i, block_id), data);
        data &= ~(mask);
        cvmx_write_csr(CVMX_PCSX_INTX_EN_REG(i, block_id), data);
    }
}


/*
 * Function: wallander_phy_reg_page_rd
 *
 * Description:
 * Read Vitesse PHY register
 *
 * Input:
 * bus_id - SMI bus is
 * phy_id - The MII phy id
 * page - The page index
 * reg - Register location to read
 * val - The register value
 * Return: pass/fail
 */
int wallander_phy_reg_page_rd(int bus_id, int phy_id, int page, int reg, ushort *val)
{
    int status;
//     int cur_page;
    int regval = 0xFF;

//     printf("Set to bus %d, phy %d, page %d\n", bus_id, phy_id, page);
    if (page != 0) {
        status = wallander_phy_reg_wr(bus_id, phy_id, VSC85XX_PHY_EXT_REG_PAGE, page);
        if (status) {
            return (status);
        }
    }

    status = wallander_phy_reg_rd(bus_id, phy_id, reg, &regval);
//     printf("PHY-%d @page%d reg%x value = %#x\n", phy_id, page, reg, regval);
    *val = regval;
    if (status) {
        return (status);
    }

    if (page != 0) {
        status = wallander_phy_reg_wr(bus_id, phy_id, VSC85XX_PHY_EXT_REG_PAGE, 0);
    }
    return (status);
}

/*
 * Function: wallander_phy_reg_wr
 *
 * Description:
 * Write Vitesse PHY register
 *
 * Input:
 * bus_id - SMI bus id
 * phy_id - The MII phy id
 * page - The page index
 * reg - Register location to write
 * val - write value
 *
 * Return: pass/fail
 */
int wallander_phy_reg_page_wr (int bus_id, int phy_id, int page, int reg, ushort val)
{
    int status;
//     int cur_page;

    /* Set page first */
//     if (wallander_phy_reg_rd(bus_id, phy_id, VSC85XX_PHY_EXT_REG_PAGE, &cur_page )) {
//         return (FAILED);
//     }
// 
//     if ( page != cur_page ) {
//         status = wallander_phy_reg_wr(bus_id, phy_id, VSC85XX_PHY_EXT_REG_PAGE, page);
//         if (status) {
//             return (status);
//         }
//     }
//
//     status = wallander_phy_reg_wr(bus_id, phy_id, reg, val);
//     return (status);

    if (page != 0) {
        status = wallander_phy_reg_wr(bus_id, phy_id, VSC85XX_PHY_EXT_REG_PAGE, page);
        if (status) {
            return (status);
        }
    }

    status = wallander_phy_reg_wr(bus_id, phy_id, reg, val);
    if (status) {
        return (status);
    }

    if (page != 0) {
        status = wallander_phy_reg_wr(bus_id, phy_id, VSC85XX_PHY_EXT_REG_PAGE, 0);
    }

    return (status);
}

/*
 * Function: wallander_phy_reg_rd
 *
 * Description:
 * Read Vitesse PHY register
 *
 * Input:
 * bus_id - SMI bus is
 * phy_id - The MII phy id
 * reg - Register location to read
 * val - The register value
 * Return: pass/fail
 */
int wallander_phy_reg_rd(int bus_id, int phy_id, int reg, int *val)
{
    int mii_value = 0;

//     printf("Calling user_cvmx_mdio_read(0, %d, %d)\n", phy_id, reg);
    mii_value = user_cvmx_mdio_read(bus_id, phy_id, reg);
//     printf ("mii_value = %#lx\n", mii_value);
    if (mii_value < 0) {
        printf("\nRead error from device %u(0x%x), reg=%#x, mii_value=%#x\n",
                phy_id, bus_id, reg, mii_value);
        return (FAILED);

    } else {
    #if DEBUG
        printf("Device %d(0x%x) reg %d(0x%x) = 0x%04x\n", 
            phy_id, phy_id, reg, reg, mii_value);
    #endif

        *val = mii_value;
//          printf ("*val = %#lx\n", *val);

        return (PASSED);
    }
}

/*
 * Function: wallander_phy_reg_wr
 *
 * Description:
 * Write Vitesse PHY register
 *
 * Input:
 * bus_id - SMI bus id
 * phy_id - The MII phy id
 * reg - Register location to write
 * val - write value
 *
 * Return: pass/fail
 */
int wallander_phy_reg_wr (int bus_id, int phy_id, int reg, int val)
{
    int status;

//     printf("Calling user_cvmx_mdio_write(0, %d, %d)\n", phy_id, reg);
    status = user_cvmx_mdio_write(bus_id, phy_id, reg, val);

    if (status < 0) {
        printf("Write error to device %d(0x%x)\n", phy_id, phy_id);
        return (FAILED);
    } else {
    #if DEBUG
        printf("Device %d(0x%x) reg %d(0x%x) <- 0x%04x\n", 
           phy_id, phy_id, reg, reg, val);
    #endif
    }

    return(PASSED);
}


/**
 * Perform an MII read. This function is used to read PHY
 * registers controlling auto negotiation.
 * (The reason why we propagate this function from Cavium executive is because
 *  there might be race condition occurs due to linux phy state machine also
 *  fires smi read command to cpu.
 *  So, workaround is implemented to make sure that the phy address and register
 *  address remains the same as original, if not then retry)
 *
 * @param bus_id   MDIO bus number. Zero on most chips, but some chips (ex CN56XX)
 *                 support multiple busses.
 * @param phy_id   The MII phy id
 * @param location Register location to read
 *
 * @return Result from the read or -1 on failure
 */
static inline int user_cvmx_mdio_read (int bus_id, int phy_id, int location)
{
    cvmx_smix_cmd_t smi_cmd;
    cvmx_smix_rd_dat_t smi_rd;
    int retries = SMI_ACCESS_RETRY_TIME;
    uint64_t smi_base = 0;
//     int timeout = 1000;

    if (octeon_has_feature(OCTEON_FEATURE_MDIO_CLAUSE_45))
        __cvmx_mdio_set_clause22_mode(bus_id);

    do {
//         uint64_t addr_64 = 0;
        smi_cmd.u64 = 0;
        smi_cmd.s.phy_op = MDIO_CLAUSE_22_READ;
        smi_cmd.s.phy_adr = phy_id;
        smi_cmd.s.reg_adr = location;
//         addr_64 = smi_base + SMI_CMD(bus_id);
        cvmx_write_csr(CVMX_SMIX_CMD(bus_id), smi_cmd.u64);
        smi_rd = __cvmx_mdio_read_rd_dat(bus_id);


        /* Now check if command executed doesn't get modified in the middle.
         * If it is modified then perform it again)
         */
        smi_cmd.u64 = cvmx_read_csr(CVMX_SMIX_CMD(bus_id));
        if (smi_cmd.s.phy_adr != phy_id || smi_cmd.s.reg_adr != location) {
            continue;
        }

        if (smi_rd.s.val) {
            return (smi_rd.s.dat);
        } else {
            return (-1);
        }
    } while (retries--);

    return (-1);
}


/**
 * Perform an MII write. This function is used to write PHY
 * registers controlling auto negotiation.
 * (The reason why we propagate this function from Cavium executive is because
 *  there might be race condition occurs due to linux phy state machine also
 *  fires smi read command to cpu.
 *  So, workaround is implemented to make sure that the phy address and register
 *  address remains the same as original, if not then retry)
 *
 * @param bus_id   MDIO bus number. Zero on most chips, but some chips (ex CN56XX)
 *                 support multiple busses.
 * @param phy_id   The MII phy id
 * @param location Register location to write
 * @param val      Value to write
 *
 * @return -1 on error
 *         0 on success
 */
static inline int user_cvmx_mdio_write (int bus_id, int phy_id, int location, int val)
{
    cvmx_smix_cmd_t smi_cmd;
    cvmx_smix_wr_dat_t smi_wr;
    int retries = SMI_ACCESS_RETRY_TIME;
//     uint64_t smi_base = 0;
//     int timeout = 1000;

    if (octeon_has_feature(OCTEON_FEATURE_MDIO_CLAUSE_45))
        __cvmx_mdio_set_clause22_mode(bus_id);

    do {
        smi_wr.u64 = 0;
        smi_wr.s.dat = val;
        cvmx_write_csr(CVMX_SMIX_WR_DAT(bus_id), smi_wr.u64);

        smi_cmd.u64 = 0;
        smi_cmd.s.phy_op = MDIO_CLAUSE_22_WRITE;
        smi_cmd.s.phy_adr = phy_id;
        smi_cmd.s.reg_adr = location;
        cvmx_write_csr(CVMX_SMIX_CMD(bus_id), smi_cmd.u64);

        if (CVMX_WAIT_FOR_FIELD64(CVMX_SMIX_WR_DAT(bus_id),
            cvmx_smix_wr_dat_t, pending, ==, 0, CVMX_MDIO_TIMEOUT)) {
            continue;
        }

        /* Now check if command executed doesn't get modified in the middle.
         * If it is modified then perform it again)
         */
        smi_cmd.u64 = cvmx_read_csr(CVMX_SMIX_CMD(bus_id));
        if (smi_cmd.s.phy_adr != phy_id || smi_cmd.s.reg_adr != location) {
            continue;
        } else {
            return (0);
        }
    } while (retries--);

    return (-1);

}


/*------------------------------------------------------------------
 * $Log: diag_common_drv.c,v $
 * Revision 1.1  2015/02/26 07:18:29  xiaoyizh
 * Initial check in for Wallander.
 *
 *
 * $Endlog$
 *------------------------------------------------------------------
 */

