/* $Id: diag_common_drv.c,v 1.2 2013/10/08 08:48:27 tirawan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn_linux/diag_common_drv.c,v $
 *------------------------------------------------------------------
 * Filename: diag_common_drv.c
 *
 * Description: Device Driver Library
 * Author: Times Huang
 *
 * Copyright (c) 2013 by cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include "error.h"
#include "common.h"
#include "common_utils.h"
#include "diag_ge_phy_88E1340_lib.h"
#include "dev_phy_88e1340.h"
#include "diag_common_drv.h"
#include "diag_ge_phy_88E1112C_lib.h"

#include <stdint.h>
#include "defs.h"
#include "types.h"
#include "cvmx.h"
#include "ethernet.h"
#include "platform_eth.h"
#include "cvmx-mdio.h"
/***********************************************************************
 *  Macro Definitions
 ************************************************************************/

/***********************************************************************
 *  Static Functions Declaration
 ************************************************************************/

static int mrv88e1340_phy_read_smi(uint, uint, uint, uint *);
static int mrv88e1340_phy_write_smi(uint, uint, uint, uint);
static inline int user_cvmx_mdio_write(int, int, int, int);
static inline int user_cvmx_mdio_read(int, int, int);
/***********************************************************************
 *  Functions Declaration
 ************************************************************************/

dev_object_t *diag_get_88e1340_obj(int);
int woodlawn_phy_reg_rd(int, int, int, int *);
int woodlawn_phy_reg_wr(int, int, int, int);
int get_smi_bus_id(int);
/***********************************************************************
 *  Externs
 ************************************************************************/

extern uint32 err_report(dev_object_t *, char *, uint32);
/***********************************************************************
 *  Global Variable
 ************************************************************************/
/*
 *  88E1340 Device Driver Object (PHY 0)
 */
static int mrv_88e1340_dev_0_init = FALSE;
static dev_88e1340_object_t mrv_88e1340_obj_0;

/*
 *  88E1548 Device Driver Object (PHY 1)
 */
static int mrv_88e1340_dev_1_init = FALSE;
static dev_88e1340_object_t mrv_88e1340_obj_1;

/***********************************************************************
 *  Functions
 ************************************************************************/

/**********************************************************************
 *
 * Function: im_get_88e1340_obj
 *
 * Description: Get 88E1340 device driver object for IM
 *
 **********************************************************************
 */
dev_object_t *diag_get_88e1340_obj (int phy_no)
{
    dev_88e1340_object_t *p_obj = &mrv_88e1340_obj_0;
    int start_phy_addr;

    if (phy_no == MRVL_1340_PHY0) {
        p_obj = &mrv_88e1340_obj_0;
    } else if (phy_no == MRVL_1340_PHY1) {
        p_obj = &mrv_88e1340_obj_1;
    } else {
        return (NULL);
    }

    if ((phy_no == MRVL_1340_PHY0 && mrv_88e1340_dev_0_init == FALSE) ||
        (phy_no == MRVL_1340_PHY1 && mrv_88e1340_dev_1_init == FALSE)) {
        /* Create the device driver object */
        dev_88e1340_create((dev_object_t *)p_obj, (dev_error_report_t)err_report);

        /* Attach the callin function */
        p_obj->base.dev_object_fvt->dev_attach((dev_object_t *)p_obj);

        /* Assign the callout function */
        p_obj->callout_fvt->smi_read  = mrv88e1340_phy_read_smi;
        p_obj->callout_fvt->smi_write = mrv88e1340_phy_write_smi;

        if (phy_no == MRVL_1340_PHY0) {
            mrv_88e1340_dev_0_init = TRUE;
            start_phy_addr = MRVL_88E1340_PHY0_SMI_ADDR;
        } else if (phy_no == MRVL_1340_PHY1) {
            mrv_88e1340_dev_1_init = TRUE;
            start_phy_addr = MRVL_88E1340_PHY1_SMI_ADDR;
        }

        p_obj->base_phyaddr  = start_phy_addr;
        p_obj->addr_seq = MRV88E1340_PHY_ADDR_DECR;
    }

    return ((dev_object_t *)p_obj);
}

/***********************************************************************
 *  Static Functions
 ************************************************************************/

/* ******************************************************
 *
 * Function: mrv88e1340_phy_read_smi
 *
 * Description: Read 2 bytes from device on SMI bus.
 *
 * Input:    iface  - Pointer to interface data structure.
 *           phy_addr - SMI Device address.
 *           offset - Location of data to be read in the SMI device.
 *                    offset is the register address * sizeof(ushort).
 *                    Device address is 5 bits. Register address is 5 bits.
 *           data   - Points to where the read data to be stored.
 *
 * Outputs:  PASSED - No errors encountered.
 *           FAILED - Errors encountered.
 *
 * Assumptions:
 *
 * ******************************************************
 */
static int mrv88e1340_phy_read_smi (uint phy_addr, uint page,
                                    uint offset, uint *data)
{
    int ret;

    ret = diag_88e1340_smi_phy_rd(phy_addr, offset, (int *)data);

    return (ret);
}


/* ******************************************************
 *
 * Function: mrv88e1340_phy_write_smi
 *
 * Description: Write 2 bytes to a device on SMI bus.
 *
 * Input:    iface - Pointer to interface data structure.
 *       phy_addr - Device address on SMI.
 *           offset - Location of data to be written in the SMI device.
 *           data - Data to be written.
 *
 * Outputs:  PASSED - No errors encountered.
 *           FAILED - Errors encountered.
 *
 * Assumptions:
 *
 * ******************************************************
 */
static int mrv88e1340_phy_write_smi (uint phy_addr, uint page,
                                     uint offset, uint data)
{
    return (diag_88e1340_smi_phy_wr(phy_addr, offset, data));
}


/*
 * Function: woodlawn_phy_reg_rd
 *
 * Description:
 * Read marvell PHY register
 *
 * Input:
 * bus_id - SMI bus is
 * phy_id - The MII phy id
 * reg - Register location to read
 * val - The register value
 * Return: pass/fail
 */
int woodlawn_phy_reg_rd(int bus_id, int phy_id, int reg, int *val)
{
    int mii_value;

    mii_value = user_cvmx_mdio_read(bus_id, phy_id, reg);

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

        return (PASSED);
    }
}

/*
 * Function: woodlawn_phy_reg_wr
 *
 * Description:
 * Write marvell PHY register
 *
 * Input:
 * bus_id - SMI bus id
 * phy_id - The MII phy id
 * reg - Register location to write
 * val - write value
 *
 * Return: pass/fail
 */
int woodlawn_phy_reg_wr (int bus_id, int phy_id, int reg, int val)
{
    int status;

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

int get_smi_bus_id (int phy_addr)
{
    int bus_id;

    if (phy_addr & 0x4) {
        bus_id = SMI_BUS_1;
    } else {
        bus_id = SMI_BUS_0;
    }
    
    return (bus_id);
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

    if (octeon_has_feature(OCTEON_FEATURE_MDIO_CLAUSE_45))
        __cvmx_mdio_set_clause22_mode(bus_id);

    do {
        smi_cmd.u64 = 0;
        smi_cmd.s.phy_op = MDIO_CLAUSE_22_READ;
        smi_cmd.s.phy_adr = phy_id;
        smi_cmd.s.reg_adr = location;
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
 * Revision 1.2  2013/10/08 08:48:27  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.4.2  2013/08/20 10:58:50  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.1  2013/04/24 10:37:14  tirawan
 * Initial check-in for woodlawn linux code
 *
 * Revision 1.3  2013/03/27 04:49:35  kuangik
 * Code cleanup after adding -Wall
 *
 * Revision 1.10  2013/03/09 02:04:05  kuangik
 * Add workaround for SMI read failed
 *
 * Revision 1.9  2013/03/08 09:34:07  kuangik
 * Clear all warning
 *
 * Revision 1.8  2013/02/18 07:47:42  leslie
 * Add get smi bus id function.
 *
 * Revision 1.7  2013/01/18 06:23:18  leslie
 * Fix and clean up code.
 *
 * Revision 1.6  2012/08/27 06:42:31  evanli
 * Modify address is decreasing and make to select correct bus to do R/W
 *
 * Revision 1.5  2012/08/03 10:16:55  evanli
 * Mapping to latest O2 source code on 20120726
 *
 * Revision 1.3  2012/07/03 04:43:26  leslie
 * Add Woodlawn Phy Register R/W Function
 *
 * Revision 1.2  2012/05/18 10:18:44  kody
 * Fix the type warning during compile.
 *
 * Revision 1.1  2012/04/06 06:10:15  kuangik
 * Add Common Drive File for the first time
 *
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 *
 *------------------------------------------------------------------
 */

