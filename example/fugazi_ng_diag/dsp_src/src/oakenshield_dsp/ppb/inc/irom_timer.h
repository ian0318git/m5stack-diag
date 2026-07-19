/* $Id: irom_timer.h,v 1.2 2017/07/28 07:58:37 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/ppb/inc/irom_timer.h,v $
 *------------------------------------------------------------------
 * irom_timer.h
 *      Oakenshield ARM diagnostic 
 *		LSI code to set timer so we can release CPU in loop.
 *
 * Mar 2012, Smita Rane
 *
 * Copyright (c) 2012-2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
/*****************************************************************************
 *                             NOTIFICATION
 *
 * Copyright ¨ 2000-2005 Agere Systems Inc.
 * All Rights Reserved
 *
 * This is unpublished proprietary information of Agere Systems Inc.  This
 * copyright notice does not evidence publication.
 *
 * The use of the software, documentation, methodologies, and other information
 * contained herein is governed solely by the associated license agreements.
 * Any inconsistent use shall be deemed to be a misappropriation of the
 * intellectual property of Agere Systems Inc. and treated accordingly.
 *----------------------------------------------------------------------------
 *
 * $RCSfile: irom_timer.h,v $
 *
 * Description:	PPB timer Interface Support
 *
 * $Log: 
 *
 *****************************************************************************/

/* !MANFILE:	 irom_timer.h */

#ifndef __AG_MG_IROM_TIMER_H
#define __AG_MG_IROM_TIMER_H

/* Timer base addresse */
#define AG_MG_IROM_TIMERBASE	0x30001000  

/* Timer register offsets */
#define AG_MG_IROM_TIMERLOAD		(0x000)		/* load register 					*/
#define AG_MG_IROM_TIMERVAL			(0x004)		/* value register 					*/
#define AG_MG_IROM_TIMERCONTROL		(0x008)		/* control register					*/
#define AG_MG_IROM_TIMERINTCLR		(0x00c)		/* interrupt clear register			*/
#define AG_MG_IROM_TIMERRIS			(0x010)		/* raw interrupt status register	*/
#define AG_MG_IROM_TIMERMIS			(0x014)		/* mask interrupt status register	*/
#define AG_MG_IROM_TIMERBGLOAD		(0x018)		/* background load timer register	*/

/* Timer IDs */
#define	AG_MG_IROM_TIMER0	0
#define	AG_MG_IROM_TIMER1	1
#define	AG_MG_IROM_TIMER2	2
#define	AG_MG_IROM_TIMER3	3
#define	AG_MG_IROM_TIMER4	4
#define	AG_MG_IROM_TIMER5	5
#define	AG_MG_IROM_TIMER6	6
#define	AG_MG_IROM_TIMER7	7

/* Timer register addr generation */
#define AG_MG_IROM_TIMER_REG(timerID, reg_offset) *((volatile uint32_t *)((AG_MG_IROM_TIMERBASE) + (0x20 * timerID) + (reg_offset)))

#define AG_MG_IROM_TIMER_REG_A(timerID, reg_offset) ((volatile uint32_t *)((AG_MG_IROM_TIMERBASE) + (0x20 * timerID) + (reg_offset)))


/* Timer control register fields*/
#define AG_MG_IROM_TIMERCONTROL_DFLT			(0x00000003) 
/* one shot; 32-bit; disabled */
#define AG_MG_IROM_TIMERCONTROL_ENABLE			(0x00000001 << 7)
#define AG_MG_IROM_TIMERCONTROL_MODE			(0x00000001 << 6)
#define AG_MG_IROM_TIMERCONTROL_INTEN			(0x00000001 << 5)
#define AG_MG_IROM_TIMERCONTROL_PRE_MASK		(0x00000003 << 2)
#define AG_MG_IROM_TIMERCONTROL_PRE_OFST		(2)
#define AG_MG_IROM_TIMERCONTROL_SIZE			(0x00000001 << 1)
#define AG_MG_IROM_TIMERCONTROL_OSCNT			(0x00000001 << 0)

/* Timer raw interrupt status register bit fields */
#define AG_MG_IROM_TIMERRIS_RAWINT_MASK			(0x000000001 << 0)
#define AG_MG_IROM_TIMERRIS_RAWINT_OFST			(0)
	

/* Timer mask interrupt status register bit fields */
#define AG_MG_IROM_TIMERMIS_INT_MASK			(0x000000001 << 0)
#define AG_MG_IROM_TIMERMIS_INT_OFST			(0)

#endif /* __AG_MG_IROM_TIMER_H */

/*
 * $Log: irom_timer.h,v $
 * Revision 1.2  2017/07/28 07:58:37  harrchan
 * Collapse Oakenshield-branch to Main Trunk.
 *
 * Revision 1.1.2.1  2017/06/29 08:14:32  harrchan
 * Initial commit code for Oakenshield
 *
 * Revision 1.1  2012/04/18 09:50:18  srane
 * Initial checkin
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
