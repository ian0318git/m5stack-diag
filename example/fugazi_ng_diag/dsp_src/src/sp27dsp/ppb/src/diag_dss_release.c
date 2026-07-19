/* $Id: diag_dss_release.c,v 1.3 2012/07/17 20:34:38 srane Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/sp27dsp/ppb/src/diag_dss_release.c,v $
 *------------------------------------------------------------------
 * diag_dss_release.c
 *      Graffham ARM - DSS related functions 
 *
 * Mar 2012, Smita Rane
 *
 * Copyright (c)2012by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

/*****************************************************************************
 *                             NOTIFICATION
 *
 * Copyright 2009 LSI Inc.
 * All Rights Reserved
 *
 * This is unpublished proprietary information of LSI Inc.  This
 * copyright notice does not evidence publication.
 *
 * The use of the software, documentation, methodologies, and other information
 * contained herein is governed solely by the associated license agreements.
 * Any inconsistent use shall be deemed to be a misappropriation of the
 * intellectual property of LSI Inc. and treated accordingly.
 *----------------------------------------------------------------------------
 *
 * File: lsi_mg_dss_release.c - optionally load and start DSS execution after reset
 *
 * Description: Procedure to release a DSS from reset:
 *    1. load memory sections into SYSMEM & LMEM as required
 *    2. set BOOTADDR to the execution start address (typically 0 or 0x4000000)
 *    	 Note: register holds only upper 28 bits of address.
 *    3. set ICUBASE (VBA) to start of vector table (typically 0.)
 *    	 Note: register holds only upper 20 bits of address.
 *    4. set bit 0 of  the SOFTRESET register to a 1. A soft reset causes
 *       DSS program execution to jump to the address stored in the BOOTADDR register

 * Author: BAS
 * Created: Nov 2009
 * History:
 * 
 *
 *****************************************************************************/

#include <stdint.h>
#include "diag_dss.h"
#include "common.h"
#include "lsi_sp27xx_reg.h"
#include "libgeneric.h"
#include "uart.h"
#include "debug_console.h"

static uint32_t ag_mg_dev_dssWaitForIdle (uint32_t, uint32_t);

/***********************************************************************
 *
 * Function: ag_mg_dev_dssRelease
 *
 * Description:  Release specified DSS core from reset 
 *
 * Input : DSS core 
 *
 * Returns: none
 *
 **********************************************************************
 */
static void ag_mg_dev_dssRelease (uint32_t dssId /* DSS0=0, ... DSS3=3 */)
{
    volatile lsi_sp27xx_dss_reg_s   *pDssReg;

#ifdef BOOT_DEBUG
    bsp_debug_printf("\r\n In ag_mg_dev_dssRelease for dssId = %d", dssId);
#endif
    pDssReg = (lsi_sp27xx_dss_reg_s *) DSS_REG[dssId];
#ifdef BOOT_DEBUG
    bsp_debug_printf("\r\n pDssReg = 0x%x", pDssReg);
#endif

    /* 1. set BOOTADDR to the execution start address
     * (ex: 0, 0x00200000, 0x04000000) */
    pDssReg->bootaddr.fields.addr = 0x00001000 >> 4;

    /* 2. set ICUBASE (VBA) to start of vector table
     * (ex: 0, 0x00200000, 0x40000000) */
    pDssReg->icubase.fields.addr = 0x00001000 >> 12;

    /* 3. set bit 0 of the BOOT register to a 1. */
    pDssReg->boot.fields.btbit = 1;

    /* 4. Execute the soft reset on the requested DSS core */
    pDssReg->softreset.fields.softr = 1;
}

/***********************************************************************
 *
 * Function: sp_ReleaseDSS
 *
 * Description:  Release specified DSS cores from reset 
 *
 * Input : DSS cores 
 *
 * Returns: PASSED
 *
 **********************************************************************
 */
uint32_t sp_ReleaseDSS (uint32_t dssBitmask)
{
   uint32_t        dssId;

#ifdef BOOT_DEBUG
    bsp_debug_printf("\r\n In sp_ReleaseDSS for dssBitmask = %d", dssBitmask);
#endif

    for (dssId = 0; dssId < AG_MG_NUM_DSS; dssId++)
    {
#ifdef BOOT_DEBUG
        bsp_debug_printf("\r\n In sp_ReleaseDSS resetting dssid = 0x%x, bitmask = 0x%x", 
            dssId, dssBitmask);
#endif
        if (dssBitmask & (1<<dssId)) { 
            ag_mg_dev_dssRelease(dssId); 
        }
    }

    return 1;
}

/***********************************************************************
 *
 * Function: ag_mg_dev_dssWaitForIdle
 *
 * Description:  Place specified DSS in debug mode.  
 *               If core is already in WAIT/STOP state, do nothing.
 *
 * Input : DSS core, timeout - waiting period
 *
 * Returns: TRUE/FALSE
 *
 **********************************************************************
 */
static uint32_t ag_mg_dev_dssWaitForIdle (uint32_t dssId, /* DSS0=0, ... DSS3=3 */
                                          uint32_t timeout)
{
    uint32_t status;
    uint32_t oceBase;

    /* Read the ALARM register from DSS ctrl block.  Is core in WAIT/STOP state? */
    if ((HW_REG_ACCESS(LSI_SP27XX_DBM_DSSCTRLREGS_DSS_ALARM_RA(dssId))
                       & (LSI_SP27XX_DSS_ALARM_WAITALARM_BM
                        | LSI_SP27XX_DSS_ALARM_STOPALARM_BM)) == 0) {
#ifdef BOOT_DEBUG
        uart_puts("\r\n In ag_mg_dev_dssWaitForIdle core not in wait/stop ");
#endif
        /* Core is not in WAIT/STOP state.  Try to place into Debug mode */

        switch (dssId) {        /* No register interface for OCE. Access directly */
        case 0: oceBase = LSI_SP27XX_DBM_DSSCTRLREGS_DSS0_BASE|0x400; break;
        case 1: oceBase = LSI_SP27XX_DBM_DSSCTRLREGS_DSS1_BASE|0x400; break;
        case 2: oceBase = LSI_SP27XX_DBM_DSSCTRLREGS_DSS2_BASE|0x400; break;
        case 3: oceBase = LSI_SP27XX_DBM_DSSCTRLREGS_DSS3_BASE|0x400; break;
       default: return(FALSE);
    }

    /* If the core is initially in debug mode, it might have been put into
     * that state via a different source.  If this is the case, the debug
     * request from this program might not get satisfied.
     */
    if ((*(volatile unsigned int*)(oceBase | 0x1F0) & 0x6) == 0x6)
    { 
        uart_puts("\r\n In ag_mg_dev_dssWaitForIdle core in debug mode ");
        /* Core is in Debug mode */
    } else {
        /* APB Debug Request for this core via EMCR[APBDBG]=1 */
        uart_puts("\r\n In ag_mg_dev_dssWaitForIdle core in request for this core mode ");
        *(volatile unsigned int*)(oceBase | 0x4) |= 0x08000000;
    }

    timeout += 1;
    while(timeout--)
    {
        /* Poll RD_STATUS[2:0]==6 until core is in debug mode and idle */
        status = *(volatile unsigned int*)(oceBase | 0x1F0) & 0x7;
        if (status == 0x6) { 
            uart_puts("\r\n status = 6");
            break;
        }
    }
    if (timeout == 0) 
        return(FALSE); /* Cannot verify debug mode, failure */
        /* Core is in DEBUG mode.  Nothing more to be done. */
#ifdef BOOT_DEBUG
        uart_puts("\r\n in debug mode");
#endif
    } else {
#ifdef BOOT_DEBUG
        uart_puts("\r\n in wait/stop mode");
#endif
        /* Core is in WAIT/STOP state.  Nothing to be done. Return */
    }

    return(TRUE);
}

/*******************************************************************************
 * Low Level function resets the specified DSS cores.
 ******************************************************************************/
static void ag_mg_dev_dssReset (uint32_t dssId)
{
    volatile lsi_sp27xx_dss_reg_s   *pDssReg;

#ifdef BOOT_DEBUG
    bsp_debug_printf("\r\n In ag_mg_dev_dssReset for DSS = %d", dssId);
#endif

    /* Avoid resetting DSS until AXI bus activity is halted.  DSS needs to be
     * in wait/stop/debug state */
    if (ag_mg_dev_dssWaitForIdle(dssId, 2) == FALSE)
    {
        if (ag_mg_dev_dssWaitForIdle(dssId, 0xFFFFFF) == FALSE) /* try again */
        {
#ifdef BOOT_DEBUG
            bsp_debug_printf("\r\n ERROR return without reset In ag_mg_dev_dssReset for DSS = %d" , dssId);
#endif
            return;         /* Can't put DSS into debug mode */
        }
    }

    pDssReg = (lsi_sp27xx_dss_reg_s *)DSS_REG[dssId];
#ifdef BOOT_DEBUG
    bsp_debug_printf("\r\n Reset the DSS%d pDssReg = 0x%x", dssId, pDssReg);
#endif
    pDssReg->hardreset.reg = 1;
}

/*******************************************************************************
 * This function resets the specified DSS cores.
 ******************************************************************************/
uint32_t sp_ResetDSS (uint32_t dssBitmask)
{
    uint32_t dssId;
         
#ifdef BOOT_DEBUG
    bsp_debug_printf("\r\n In sp_ResetDSS for dssBitmask = %d", dssBitmask);
#endif
                      
    for (dssId = 0; dssId < AG_MG_NUM_DSS; dssId++)
    {                           
        if( dssBitmask & (1<<dssId))    
        {
            ag_mg_dev_dssReset( dssId );
        }                       
    }

    return 1;
}       

/******** History ********
$Log: diag_dss_release.c,v $
Revision 1.3  2012/07/17 20:34:38  srane
cleanup

Revision 1.2  2012/05/10 22:57:58  srane
Add TDM support. Adjust the linker sections.

Revision 1.1  2012/04/18 09:44:02  srane
Initial checkin


$Endlog$
*/

