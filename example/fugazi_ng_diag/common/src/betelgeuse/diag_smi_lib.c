/* $Id: diag_smi_lib.c,v 1.2 2019/01/10 06:36:24 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/diag_smi_lib.c,v $
 *------------------------------------------------------------------
 * 
 * diag_smi_lib.c
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h> 
#include "common.h" 
#include "types.h" 
#include "nvmonvars.h"
#include "common_utils.h"
#include "proto.h" 
#include "diag_smi_lib.h"
#include "diag_cpu_lib.h"
#include "diag_cpu_lib.h"


/*******************************************************************************
 *                          Function Declaration
 *******************************************************************************
 */
int plat_smi_read(int, int, ushort *);
int plat_smi_write(int, int, ushort);
int plat_smi_read_util(int);
int plat_smi_write_util(int);

/*******************************************************************************
 *                                  Function
 *******************************************************************************
 */
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
    uint   smi_regaddr = (uint)M7040_SMI_REG;
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

        if ((reg_val & M7040_SMI_BUSY) == 0) {
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
    reg_val = (uint)((M7040_SMI_OPCODE_RD) |
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

        if ((reg_val & M7040_SMI_READ_VALID) == M7040_SMI_READ_VALID) {
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
    uint   smi_regaddr = (uint)M7040_SMI_REG;

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s(%d): phy_addr = 0x%08X.\n", __func__, __LINE__, phy_addr);
    }

    /* Comfirm SMI bus is ready for access. */
    for (ctr = 0; ctr < PLAT_SMI_RETRY_MAX; ctr++) {
        reg_val = (uint)M7040_SMI_BUSY;
        if (plat_mem_read32(smi_regaddr, &reg_val) != PASSED) {
            printf("%s(%d): Failed to read CPU SMI Management Reg.(0x%08X).\n",
                   __func__, __LINE__, smi_regaddr);
            return (FAILED);
        }

        if ((reg_val & M7040_SMI_BUSY) == 0) {
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
 * Function   : plat_smi_read_util
 * Description: Utility to do SMI read.
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int plat_smi_read_util (int opt)
{
    int    phy_addr = 0, reg_addr = 0;
    ushort reg_val = 0;

    phy_addr = gethex_answer("Enter SMI PHY addr.: ", 0, 0, 0x1F);
    reg_addr = gethex_answer("Enter SMI PHY Reg. addr.: ", 0, 0, 0x1F);

    if (plat_smi_read(phy_addr, reg_addr, &reg_val) != PASSED) {
        return (FAILED);
    } else {
        printf("SMI device(0x%02X) register 0x%02X: 0x%04X.\n",
               phy_addr, reg_addr, reg_val);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : plat_smi_write_util
 * Description: Utility to do SMI write.
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int plat_smi_write_util (int opt)
{
    int    phy_addr = 0, reg_addr = 0;
    ushort reg_val = 0, w_data = 0;

    phy_addr = gethex_answer("Enter SMI PHY addr.: ", 0, 0, 0x1F);
    reg_addr = gethex_answer("Enter SMI PHY Reg. addr.: ", 0, 0, 0x1F);

    if (plat_smi_read(phy_addr, reg_addr, &reg_val) != PASSED) {
        return (FAILED);
    }

    w_data = (ushort)gethex_answer("Enter write in data(0x0 ~ 0xffff): ",
                                   reg_val, 0, 0xffff);

    if (plat_smi_write(phy_addr, reg_addr, w_data) != PASSED) {
        return (FAILED);
    } else {
        printf("Done writing 0x%04X to SMI device(0x%02X) register 0x%02X.\n",
               w_data, phy_addr, reg_addr);
    }
    return (PASSED);
}

/*-------------------------------------------------
 * $Log: diag_smi_lib.c,v $
 * Revision 1.2  2019/01/10 06:36:24  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
