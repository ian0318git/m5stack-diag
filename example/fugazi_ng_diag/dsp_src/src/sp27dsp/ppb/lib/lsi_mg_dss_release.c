/* $Id: lsi_mg_dss_release.c,v 1.2 2012/05/10 22:48:11 srane Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/sp27dsp/ppb/lib/lsi_mg_dss_release.c,v $
 *------------------------------------------------------------------
 * lsi_mg_dss_release.c
 *
 * Mar 2012, Smita Rane
 *
 * Copyright (c) 2012 by Cisco Systems, Inc.
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
#include "regs.h"
#include "lsi_sp27xx_reg.h"
#include "libgeneric.h"

/* swap bytes and words: use for loading records from big-endian processors over PCI port */ 
#define SWAP_BIGENDIAN(_val_) (((_val_ & 0xFF) << 24) | ((_val_ & 0xFF00) << 8) | ((_val_ & 0xFF0000) >> 8) | ((_val_ & 0xFF000000) >> 24)) 


void ag_mg_dev_dssRelease(
        uint32_t dssId          /* DSS0=0, ... DSS3=3 */
        )
{
        volatile lsi_27xx_dss_reg_s   *pDssReg;


        pDssReg = (lsi_27xx_dss_reg_s *) DSS_REG[dssId];

        /* 1. set BOOTADDR to the execution start address
         * (ex: 0, 0x00200000, 0x04000000) */
        pDssReg->bootaddr.fields.addr = 0x00001000 >> 4;
                //ag_mg_dev_carState.dssStartAddress[dssId] >> 4;

        /* 2. set ICUBASE (VBA) to start of vector table
         * (ex: 0, 0x00200000, 0x40000000) */
        pDssReg->icubase.fields.addr = 0x00001000 >> 12;
                //ag_mg_dev_carState.dssVectorAddress[dssId] >> 12;

        /* 3. set bit 0 of the BOOT register to a 1. */
        pDssReg->boot.fields.btbit = 1;

        #ifdef CONFIGURE_WITH_DSS_RESET_RELEASE
        /* 4. Execute the soft reset on the requested DSS core */
        pDssReg->softreset.fields.softr = 1;
        #endif          /* CONFIGURE_WITH_DSS_RESET_RELEASE */
}

uint32_t sp_ReleaseDSS(
        uint32_t        dssBitmask
        )
{
        uint32_t        dssId;

        //SPPPBPD_RELEASE_DSS_ENTRY( dssBitmask );

        for (dssId = 0; dssId < AG_MG_NUM_DSS; dssId++)
        {
#ifdef DEV_DEBUG
bsp_debug_printf("\n In StarProPPB_ReleaseDSS resetting dssid = 0x%x, bitmask = 0x%x", dssId, dssBitmask);
#endif
                if( dssBitmask & (1<<dssId)) { ag_mg_dev_dssRelease( dssId ); }
        }

        //SPPPBPD_RELEASE_DSS_COMPLETE( dssBitmask, SP_PPB_SUCCESS );
        return 1;
}


#if 0
void sp_ReleaseDSS(
	int32_t		numrecs,		/* in: number for records to load */
	load_rec_t	records[],		/* in: array of load records */
	uint32_t startAddress,		/* in: VBA register value (location of reset vector) */
	uint32_t DSSreleaseMask)	/* in: DSS subsystems to start */
{
volatile uint32_t *p_mem;		/* for loading memories */
uint32_t ii, jj;
int waitcnt;

int	numDSS = NUMBEROFDSS;

	/* The SP2702 has a 0xC in PFUSE123 indicating that DSS2 and DSS3 are disabled */
	if (HW_REG_ACCESS(LSI_SP27XX_CAR_PFUSE_RA(123)) != 0) {
		numDSS = 2;
	}

	if (DSSreleaseMask & DSSRESETALL) {
		for (ii = 0; ii < numDSS; ii++) {
 /*    0. Apply a hardreset to all cores if requested. */
			REG32_WRITE(LSI_SP27XX_DBM_DSSCTRLREGS_DSS_HARDRESET_RA(ii), 1);
	 	}
	}	

    if (numDSS == 2) {					/* SP2702 */
   	 /*    1. load memory sections */
   		for (ii = 0; ii < numrecs; ii++) {
   			p_mem = (uint32_t *) records[ii].load_address;
   		   	/* image may be generic with data for DSSLMEM in DSS2/DSS3 we shouldn't load */
   			if (((uint32_t) p_mem >= 0x84000000) && ((uint32_t)p_mem < 0x86040000)) {
   				continue;
   			}
   			for (jj = 0; jj < records[ii].long_count; jj++) {
   				// *p_mem++ = SWAP_BIGENDIAN(records[ii].data[jj]);
   				*p_mem++ = records[ii].data[jj];
   			}
   		}
    } else {							/* SP2704 */
	 /*    1. load memory sections */
		for (ii = 0; ii < numrecs; ii++) {
			p_mem = (uint32_t *) records[ii].load_address;
			for (jj = 0; jj < records[ii].long_count; jj++) {
				// *p_mem++ = SWAP_BIGENDIAN(records[ii].data[jj]);
				*p_mem++ = records[ii].data[jj];
			}
		}
    }

	/* Set up all requested DSS cores */
	for (ii = 0; ii < numDSS; ii++) {
		if (DSSreleaseMask & (1<<ii)) {

 /*    2. set BOOTADDR to the execution start address (typically 0 or 0x4000000) */
			REG32_WRITE(LSI_SP27XX_DBM_DSSCTRLREGS_DSS_BOOTADDR_RA(ii), startAddress >> 4);

 /*    3. set ICUBASE (VBA) to start of vector table (typically 0 or 0x40000000) */
			REG32_WRITE(LSI_SP27XX_DBM_DSSCTRLREGS_DSS_ICU_BASEADDR_RA(ii), startAddress >> 12);
		}
	}


	for (ii = 0; ii < numDSS; ii++) {
		if (DSSreleaseMask & (1<<ii)) {
 /*    4. Apply a softreset to the core. This also releases the core but may set CLKALARM. This should be cleared by DSS */
			REG32_WRITE(LSI_SP27XX_DBM_DSSCTRLREGS_DSS_SOFTRESET_RA(ii), 1);
			waitcnt = 1000;
			while (waitcnt-- > 0) {
				/* DSS should signal initialization complete, see: init_clrClkAlrm() in libsp27dss.elb */
				if (HW_REG_ACCESS(LSI_SP27XX_DBM_DSSCTRLREGS_DSS_GPR0_RA(ii)) != 0) {
					lsi_mg_delay(10);		/* allow SP2704 power to stabilize */
					break;
				}
			}
			if (waitcnt <= 0) {
				/* try a second softreset */
				REG32_WRITE(LSI_SP27XX_DBM_DSSCTRLREGS_DSS_SOFTRESET_RA(ii), 0);
				lsi_mg_delay(100);
			}
		}
	}
}

#endif

/* Place specified DSS in debug mode.  If core is already in WAIT/STOP
 * state, do nothing.
 */
boolean_t ag_mg_dev_dssWaitForIdle(
        uint32_t dssId,         /* DSS0=0, ... DSS3=3 */
        uint32_t timeout
        )
{
    uint32_t status;
    uint32_t oceBase;

    /* Read the ALARM register from DSS ctrl block.  Is core in WAIT/STOP state? */
    if ((HW_REG_ACCESS(LSI_SP27XX_DBM_DSSCTRLREGS_DSS_ALARM_RA(dssId))
                       & (LSI_SP27XX_DSS_ALARM_WAITALARM_BM
                       | LSI_SP27XX_DSS_ALARM_STOPALARM_BM)) == 0)
    {
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
        /* Core is in Debug mode */
    } else
    {
        /* APB Debug Request for this core via EMCR[APBDBG]=1 */
        *(volatile unsigned int*)(oceBase | 0x4) |= 0x08000000;
    }

    timeout += 1;
    while(timeout--)
    {
        /* Poll RD_STATUS[2:0]==6 until core is in debug mode and idle */
        status = *(volatile unsigned int*)(oceBase | 0x1F0) & 0x7;
        if (status == 0x6) break;
    }
    if (timeout == 0) return(FALSE); /* Cannot verify debug mode, failure */
        /* Core is in DEBUG mode.  Nothing more to be done. */
    } else
    {
        /* Core is in WAIT/STOP state.  Nothing to be done. Return */
    }

    return(TRUE);
}

void ag_mg_dev_dssReset ( uint32_t dssId)
{
    volatile ag_mg_regs_dss_reg_s   *pDssReg;

    /* Avoid resetting DSS until AXI bus activity is halted.  DSS needs to be
     * in wait/stop/debug state */
    if (ag_mg_dev_dssWaitForIdle(dssId, 2) == FALSE)
    {
        if (ag_mg_dev_dssWaitForIdle(dssId, 0xFFFFFF) == FALSE) /* try again */
        {
                return;         /* Can't put DSS into debug mode */
        }
    }

    pDssReg = (lsi_sp27xx_dss_reg_s *)DSS_REG[dssId];
    pDssReg->hardreset.reg = 1;
}


/*******************************************************************************
 * This function resets the specified DSS cores.
 ******************************************************************************/
uint32_t sp_ResetDSS (uint32_t dssBitmask)
{
    uint32_t dssId;
         
    //SPPPBPD_RESET_DSS_ENTRY( dssBitmask ); 
                      
    for (dssId = 0; dssId < AG_MG_NUM_DSS; dssId++)
    {                           
        if( dssBitmask & (1<<dssId))    
        {
            ag_mg_dev_dssReset( dssId );
                                
            /* Make sure we start in the inactive state */
        }                       
    }

    //SPPPBPD_RESET_DSS_COMPLETE( dssBitmask, SP_PPB_SUCCESS );
    //return SP_PPB_SUCCESS;
    return 1;
}       

/******** History ********
$Log: lsi_mg_dss_release.c,v $
Revision 1.2  2012/05/10 22:48:11  srane
clean up and modify files for exception handling and TDM tests.

Revision 1.1  2012/04/18 09:47:31  srane
Initial checkin


$Endlog$
*/

