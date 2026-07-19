/* $Id: platform_smi.c,v 1.2 2017/08/02 14:21:49 steja Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/platform_smi.c,v $
 *-----------------------------------------------------------------------------
 *
 * Filename   : platform_smi.c
 * Description: TSN SMI bus library.
 *
 * Copyright (c) 2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *-----------------------------------------------------------------------------
 */

#include <stdio.h> 
#include "common.h" 
#include "types.h" 
#include "nvmonvars.h"
#include "common_utils.h"
#include "tsn_comm.h"
#include "proto.h" 
#include "platform_smi.h" 


/*******************************************************************************
 *                          Function Declaration
 *******************************************************************************
 */
int tsn_smi_read(int, int, ushort *);
int tsn_smi_write(int, int, ushort);
int tsn_smi_read_util(int);
int tsn_smi_write_util(int);

/*******************************************************************************
 *                                  Function
 *******************************************************************************
 */
/*******************************************************************************
 *
 * Function   : tsn_smi_read
 * Description: Function to do TSN SMI read.
 * Inputs     : phy_addr - PHY device address
 *              reg_addr - PHY device register address
 *              *buf     - buffer to put the read back register value
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_smi_read (int phy_addr, int reg_addr, ushort *buf)
{
    int    ctr = 0;
    uint   reg_val = 0;
    uint   smi_regaddr = (uint)M7040_SMI_REG;

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s:%d phy_addr = 0x%08X.\n", __FUNCTION__, __LINE__, phy_addr);
    }

    /* Comfirm SMI bus is ready for access. */
    for (ctr = 0; ctr < TSN_SMI_RETRY_MAX; ctr++) {
        if (tsn_mem_read32(smi_regaddr, &reg_val) != PASSED) {
            printf("%s:%d Failed to read CPU SMI Management Reg.(0x%08X).\n",
                   __FUNCTION__, __LINE__, smi_regaddr);
            return (FAILED);
        }

        if ((reg_val & M7040_SMI_BUSY) == 0) {
            break;
        } else {
            if (ctr == (TSN_SMI_RETRY_MAX - 1)) {
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
    if (tsn_mem_write32(smi_regaddr, reg_val) != PASSED) {
        printf("%s:%d Failed to write CPU SMI Management Reg.(0x%08X).\n",
               __FUNCTION__, __LINE__, smi_regaddr);
        return (FAILED);
    }

    for (ctr = 0; ctr < TSN_SMI_RETRY_MAX; ctr++) {
        msleep(5);

        /* Do SMI read */
        reg_val = 0;
        if (tsn_mem_read32(smi_regaddr, &reg_val) != PASSED) {
            printf("%s:%d Failed to read CPU SMI Management Reg.(0x%08X).\n",
                   __FUNCTION__, __LINE__, smi_regaddr);
            return (FAILED);
        }

        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("%s:%d read back = 0x%08X.\n",
                   __FUNCTION__, __LINE__, reg_val);
        }

        if ((reg_val & M7040_SMI_READ_VALID) == M7040_SMI_READ_VALID) {
            break;
        } else {
            if (ctr == (TSN_SMI_RETRY_MAX - 1)) {
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
 * Function   : tsn_smi_write
 * Description: Function to do TSN SMI read.
 * Inputs     : phy_addr - PHY device address
 *              reg_addr - PHY device register address
 *              w_data   - data that wanted to write in
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_smi_write (int phy_addr, int reg_addr, ushort w_data)
{
    int    ctr = 0;
    uint   reg_val = 0;
    uint   smi_regaddr = (uint)M7040_SMI_REG;

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s(%d): phy_addr = 0x%08X.\n", __func__, __LINE__, phy_addr);
    }

    /* Comfirm SMI bus is ready for access. */
    for (ctr = 0; ctr < TSN_SMI_RETRY_MAX; ctr++) {
        msleep(5);
        if (tsn_mem_read32(smi_regaddr, &reg_val) != PASSED) {
            printf("%s(%d): Failed to read CPU SMI Management Reg.(0x%08X).\n",
                   __func__, __LINE__, smi_regaddr);
            return (FAILED);
        }

        if ((reg_val & M7040_SMI_BUSY) == 0) {
            break;
        } else {
            if (ctr == (TSN_SMI_RETRY_MAX - 1)) {
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
    if (tsn_mem_write32(smi_regaddr, reg_val) != PASSED) {
        printf("%s(%d): Failed to write CPU SMI Management Reg.(0x%08X).\n",
               __func__, __LINE__, smi_regaddr);
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : tsn_smi_read_util
 * Description: Utility to do TSN SMI read.
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_smi_read_util (int opt)
{
    int    phy_addr = 0, reg_addr = 0;
    ushort reg_val = 0;

    phy_addr = gethex_answer("Enter SMI PHY addr.: ", 0, 0, 0x1F);
    reg_addr = gethex_answer("Enter SMI PHY Reg. addr.: ", 0, 0, 0x1F);

    if (tsn_smi_read(phy_addr, reg_addr, &reg_val) != PASSED) {
        return (FAILED);
    } else {
        printf("SMI device(0x%02X) register 0x%02X: 0x%04X.\n",
               phy_addr, reg_addr, reg_val);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : tsn_smi_write_util
 * Description: Utility to do TSN SMI write.
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_smi_write_util (int opt)
{
    int    phy_addr = 0, reg_addr = 0;
    ushort reg_val = 0, w_data = 0;

    phy_addr = gethex_answer("Enter SMI PHY addr.: ", 0, 0, 0x1F);
    reg_addr = gethex_answer("Enter SMI PHY Reg. addr.: ", 0, 0, 0x1F);

    if (tsn_smi_read(phy_addr, reg_addr, &reg_val) != PASSED) {
        return (FAILED);
    }

    w_data = (ushort)gethex_answer("Enter write in data(0x0 ~ 0xffff): ",
                                   reg_val, 0, 0xffff);

    if (tsn_smi_write(phy_addr, reg_addr, w_data) != PASSED) {
        return (FAILED);
    } else {
        printf("Done writing 0x%04X to SMI device(0x%02X) register 0x%02X.\n",
               w_data, phy_addr, reg_addr);
    }
    return (PASSED);
}


/*-------------------------------------------------
 * $Log: platform_smi.c,v $
 * Revision 1.2  2017/08/02 14:21:49  steja
 * Support TSN-H/M platform code
 *
 * Revision 1.1.8.2  2017/07/29 03:41:20  steja
 * tsn-branch5 synced with Maintrunk repositories
 *
 * Revision 1.1.6.3  2017/07/24 14:14:11  palin2
 * 1. To improve code readability.
 * 2. All changes are verified before check-in.
 *
 * Revision 1.1.6.2  2017/07/20 13:38:08  steja
 * tsn-branch4 merge with maintrunk
 *
 * Revision 1.1.4.2  2016/06/30 06:22:51  steja
 * tsn-branch2 sync with main trunk
 *
 * Revision 1.1.2.2  2016/05/26 10:17:38  palin2
 * Optimise SMI read write function and Switch init function.
 *
 * Revision 1.1.2.1  2016/04/29 10:14:57  palin2
 * Updated code and added support ext. loopback test after bring up Switch.
 *
 * $Endlog$
 *-------------------------------------------------
 */

