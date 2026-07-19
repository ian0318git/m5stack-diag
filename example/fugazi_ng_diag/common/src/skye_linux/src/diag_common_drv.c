/* $Id: diag_common_drv.c,v 1.2 2015/05/25 03:59:15 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/skye_linux/src/diag_common_drv.c,v $
 *------------------------------------------------------------------
 *
 * Filename: diag_common_drv.c
 *
 * Description: Device Driver Library
 * Author: Sofian Teja
 *
 * Copyright (c) 2014-2015 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#include <stdio.h>
#include <stdlib.h>
#include "error.h"
#include "common.h"
#include "common_utils.h"
#include "diag_common_drv.h"
#include "nvmonvars.h"
#include <stdint.h>
#include "defs.h"
#include "types.h"
#include "ethernet.h"
#include "skye_eth.h"
#include <gxio/mpipe.h>
#include "skye_xaui.h"
#include "diag_tlk10232_lib.h"

/***********************************************************************
 *  Macro Definitions
 ************************************************************************/

/***********************************************************************
 *  Static Functions Declaration
 ************************************************************************/

/***********************************************************************
 *  Functions Declaration
 ************************************************************************/

int skye_phy_reg_rd(int, int, int, int *);
int skye_phy_reg_wr(int, int, int, int);
/***********************************************************************
 *  Externs
 ************************************************************************/
extern void msleep(int msecs);

/***********************************************************************
 *  Global Variable
 ************************************************************************/

/***********************************************************************
 *  Functions
 ************************************************************************/


/***********************************************************************
 *  Static Functions
 ************************************************************************/
/*
 * Function: mdio_wr_wrap
 *
 * Description:
 * Wrap function for Tilera MPIPE MDIO WRITE register
 *
 * Input:
 * link_name  - The Tilera port
 * phy_addr   - The MII phy id
 * dev_addr   - Register location to read
 * reg_addr   - The register value
 * write_data - Write value
 * Return: pass/fail
 */
int
mdio_wr_wrap (char *link_name, int phy_addr, int dev_addr, int reg_addr, int write_data)
{
    /*
    Initialize the GXIO library.  We're just going to do
    mpipe_link_attr_set/_get, so tell it we aren't going to
    send/receive packets.
    */
    char* interface = link_name;
    gxio_mpipe_context_t context;
    gxio_mpipe_link_t lnk;
    uint32_t open_flags;
    int status, wr_status;

    status = gxio_mpipe_init(&context, gxio_mpipe_link_instance(interface));
    if (status < 0)
    {
        cterr_db_print("Link '%s' does not exist, can't initialize mPIPE", interface);
        gxio_mpipe_destroy(&context);
        return (status);
    }
    open_flags = GXIO_MPIPE_LINK_NO_DATA | GXIO_MPIPE_LINK_AUTO_NONE | GXIO_MPIPE_LINK_CTL;
    status = gxio_mpipe_link_open(&lnk, &context, interface, open_flags);
    if (status < 0)
    {
        cterr_db_print("mpipe-link: can't open link %s: %s\n",
                interface, gxio_strerror(status));
        gxio_mpipe_link_close(&lnk);
        gxio_mpipe_destroy(&context);
        return (status);
    }
    wr_status = gxio_mpipe_link_mdio_wr_ex(&lnk, phy_addr, dev_addr,
                reg_addr, write_data);
    if (wr_status < 0)
    {
        cterr_db_print("mpipe-mdio: write failed (%s)\n", gxio_strerror(wr_status));
        gxio_mpipe_link_close(&lnk);
        gxio_mpipe_destroy(&context);
        return (wr_status);
    }

    gxio_mpipe_link_close(&lnk);
    gxio_mpipe_destroy(&context);
    return (wr_status);
}


/*
 * Function: mdio_rd_wrap
 *
 * Description:
 * Wrap function for Tilera MPIPE MDIO READ register
 *
 * Input:
 * link_name  - The Tilera port
 * phy_addr   - The MII phy id
 * dev_addr   - Register location to read
 * reg_addr   - The register value
 * Return: pass/fail
 */
int
mdio_rd_wrap (char *link_name, int phy_addr, int dev_addr, int reg_addr)
{
    /*
    Initialize the GXIO library.  We're just going to do
    mpipe_link_attr_set/_get, so tell it we aren't going to
    send/receive packets.
    */
    char* interface = link_name;
    gxio_mpipe_context_t context;
    gxio_mpipe_link_t lnk;
    uint32_t open_flags;
    int status, read_data;

    status = gxio_mpipe_init(&context, gxio_mpipe_link_instance(interface));
    if (status < 0)
    {
        cterr('f', 0, "Link '%s' does not exist, can't initialize mPIPE", interface);
        gxio_mpipe_destroy(&context);
        return (status);
    }
    open_flags = GXIO_MPIPE_LINK_NO_DATA | GXIO_MPIPE_LINK_AUTO_NONE | GXIO_MPIPE_LINK_STATS;
    status = gxio_mpipe_link_open(&lnk, &context, interface, open_flags);
    if (status < 0)
    {
        cterr('f', 0, "mpipe-link: can't open link %s: %s\n",
                interface, gxio_strerror(status));
        gxio_mpipe_link_close(&lnk);
        gxio_mpipe_destroy(&context);
        return (status);
    }
    read_data = gxio_mpipe_link_mdio_rd_ex(&lnk, phy_addr, dev_addr,
                 reg_addr);
    if (read_data < 0)
    {
        cterr('f', 0, "mpipe-mdio: read failed (%s)\n", gxio_strerror(read_data));
        gxio_mpipe_link_close(&lnk);
        gxio_mpipe_destroy(&context);
        return (read_data);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("%#x\n", read_data);
    }
    gxio_mpipe_link_close(&lnk);
    gxio_mpipe_destroy(&context);
    return (read_data);
}


/*
 * Function: skye_phy_reg_rd
 *
 * Description:
 * Read marvell PHY register
 *
 * Input:
 * port   - The Tilera port
 * phy_id - The MII phy id
 * reg - Register location to read
 * val - The register value
 * Return: pass/fail
 */
int
skye_phy_reg_rd (int port, int phy_id, int reg, int *val)
{
    int mii_value, dev_id;
    char link[1024];
    dev_id = -1;  /* PHY 1514 no need dev_id */
    sprintf(link,"%s%d", SEL_PORT_ETH, port);
    mii_value = mdio_rd_wrap(link, phy_id, dev_id, reg);
    if (mii_value < 0) {
        cterr_db_print("\nRead error from device %u(0x%x), reg=%#x, mii_value=%#x\n",
                phy_id, dev_id, reg, mii_value);
        return (FAILED);
    } else {
        if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
            cterr_db_print("Device %d(0x%x) reg %d(0x%x) = 0x%04x\n",
                phy_id, phy_id, reg, reg, mii_value);
        }
        *val = mii_value;
        return (PASSED);
    }
}

/*
 * Function: skye_phy_reg_wr
 *
 * Description:
 * Write marvell PHY register
 *
 * Input:
 * port   - The Tilera port
 * phy_id - The MII phy id
 * reg - Register location to write
 * val - write value
 *
 * Return: pass/fail
 */
int
skye_phy_reg_wr (int port, int phy_id, int reg, int val)
{
    int status, dev_id;
    char link[1024];
    dev_id = -1;  /* PHY 1514 no need dev_id */
    sprintf(link,"%s%d", SEL_PORT_ETH, port);
    status = mdio_wr_wrap(link, phy_id, dev_id, reg, val);
    if (status < 0) {
        cterr_db_print("Write error to device %d(0x%x)\n", phy_id, phy_id);
        return (FAILED);
    } else {
        if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
            cterr_db_print("Device %d(0x%x) reg %d(0x%x) <- 0x%04x\n",
               phy_id, phy_id, reg, reg, val);
        }
    }

    return(PASSED);
}


/*
 * Function: skye_tlk_reg_rd
 *
 * Description:
 * Read TLK10232 PHY register
 *
 * Input:
 * port   - The Tilera port
 * phy_id - The MII phy id
 * reg - Register location to read
 * Return: pass/fail
 */
int
skye_tlk_reg_rd (int port, int phy_id, int dev_id, int reg)
{
    int mii_value1, mii_value2;
    char link[1024];

    sprintf(link,"%s%d", SEL_PORT_XAUI, port);
    mii_value1 = mdio_rd_wrap(link, phy_id, dev_id, reg);
    mii_value2 = mdio_rd_wrap(link, phy_id, dev_id, reg);
    if (mii_value2 < 0) {
        printf("\nRead error from device %x(0x%x), reg=%#x, mii_value2=%#x\n",
               dev_id, phy_id, reg, mii_value2);
        return (mii_value2);
    } else {
        if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
            printf("Device 0x%x(0x%x) reg (0x%x) = (1st)0x%04x, (2nd)0x%04x\n",
               dev_id, phy_id,  reg, mii_value1, mii_value2);
        }
        return (mii_value2);
    }
}

/*
 * Function: skye_tlk_reg_wr
 *
 * Description:
 * Write TLK10232 PHY register
 *
 * Input:
 * port   - The Tilera port
 * phy_id - The MII phy id
 * reg - Register location to write
 * val - write value
 *
 * Return: pass/fail
 */
int
skye_tlk_reg_wr (int port, int phy_id, int dev_id, int reg, int val)
{
    int status;
    char link[1024];

    sprintf(link,"%s%d", SEL_PORT_XAUI, port);
    status = mdio_wr_wrap(link, phy_id, dev_id, reg, val);
    if (status < 0) {
        printf("Write error to device %x(0x%x)\n", dev_id, phy_id);
        return (status);
    } else {
        if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
            printf("Device %d(0x%x) reg %d(0x%x) <- 0x%02x\n",
               dev_id, phy_id, reg, reg, val);
        }
    }

    return(PASSED);
}


/*------------------------------------------------------------------
 * $Log: diag_common_drv.c,v $
 * Revision 1.2  2015/05/25 03:59:15  steja
 * Add Support Skye SM
 *
 * Revision 1.1.4.2  2015/04/29 11:36:32  steja
 * Code check-in to skye-branch2 for ER code review
 *
 *------------------------------------------------------------------
 * Revision 1.1.2.2  2014/09/18 07:18:43  steja
 * 1.Update NC command codei
 * 2.Update enhanced error message
 *
 * Revision 1.1.2.1  2014/07/21 01:56:52  palin2
 * Initial check-in Skye module side Diag code.
 *
 *------------------------------------------------------------------
 * Revision 1.2  2014/02/27 15:01:45  palin2
 * Initial check-in ShrinkRay SM side Diag code.
 *
 * Revision 1.1.4.10  2014/01/28 07:39:40  steja
 * Code clean up
 *
 * Revision 1.1.4.9  2014/01/06 13:00:33  steja
 * 1. clean up code
 * 2. Add header TLK code
 *
 * Revision 1.1.4.8  2013/11/19 14:36:46  steja
 * Provide TLK utility for debugging
 * Update the BTK TLK into coded
 *
 * Revision 1.1.4.7  2013/11/05 09:17:54  steja
 * 1. Fix the MDIO not stable issue
 * 2. debug tlk log
 *
 * Revision 1.1.4.6  2013/10/05 06:20:24  steja
 * Update for debug
 *
 * Revision 1.1.4.5  2013/09/27 09:45:50  steja
 * Fix the 88E1514 Register Read and Write
 *
 * Revision 1.1.4.4  2013/09/27 07:25:13  steja
 * update code for bringup
 *
 * Revision 1.1.4.3  2013/09/16 09:50:15  iachang
 * Code review and update
 *
 * Revision 1.1.4.2  2013/09/13 07:00:07  palin2
 * Initial check-in ShrinkRay SM side Diag code.
 *
 * Revision 1.1.2.1  2013/06/24 09:03:34  steja
 * Checkin :
 * - Support TLK10323 Loopback test & Utility
 * - Support MV1514 Loopback test
 *
 *------------------------------------------------------------------
 * $Endlog$
 */

