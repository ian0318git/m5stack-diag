/* $Id: sp_ppb_types.h,v 1.2 2017/07/28 07:58:38 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/ppb/inc/sp_ppb_types.h,v $
 *------------------------------------------------------------------
 * sp_ppb_types.h
 *
 * Mar 2012, Smita Rane
 *
 * Copyright (c) 2012-2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

/******************************************************************************
 * >>>>>>>>>>>>>>>>>>>>>>>>>>>>    NOTIFICATION    <<<<<<<<<<<<<<<<<<<<<<<<<<<<
 *
 * Copyright (©) 2007 LSI Corporation
 * All Rights Reserved
 *
 * This is unpublished proprietary information of LSI Corporation. This
 * copyright notice does not evidence publication.
 *
 * The use of the software, documentation, methodologies, and other information
 * contained herein is governed solely by the associated license agreements.
 * Any inconsistent use shall be deemed to be a misappropriation of the
 * intellectual property of LSI Corporation and treated accordingly.
 * ----------------------------------------------------------------------------
 *
 * Author:	 TW 9/1/2007
 *
 * Content:  StarPro2700 Packet Processor (PPB) definitions.
 *
 ******************************************************************************/

#ifndef SP_PPB_TYPES_H_
#define SP_PPB_TYPES_H_

//#include "ag_mg_cfg.h"
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if 0
/* Ethernet MAC address */
typedef struct sp_eth_mac_addr {		
	uint8_t	byte[6];
} StarProPPB_EthAddr_t;
#endif

/* Timer IDs */
typedef enum
{
	SP_PPB_TIMER0 = 0,
	SP_PPB_TIMER1,
	SP_PPB_TIMER2,
	SP_PPB_TIMER3,
	SP_PPB_TIMER4,
	SP_PPB_TIMER5,
	SP_PPB_TIMER6,
	SP_PPB_TIMER7
} StarProPPB_TimerId_t;

/* Packet type */
typedef enum
{
    SP_VALID_ETHIPv4UDP_PKT	= 0, /* valid IPv4/UDP packet */
    SP_UNREC_ETHIPv4UDP_PKT	= 1, /* unrecognized IPv4/UDP packet */
    SP_UNREC_ETH_FRM		= 2, /* unrecognized Ethernet frame */
    SP_TDMOP_PKT		= 3, /* TDMoP packet */
    SP_RAW_ETH_PKT		= 4, /* Raw Ethernet packet */
    SP_LINX_FRM			= 5, /* OSE LINX Ethernet frame	*/
    SP_ARP_FRM			= 6, /* ARP Ethernet frame */
    SP_ICMP_PKT			= 7, /* ARP Ethernet frame */
    SP_INVALID_PKT		= 8, /* Invalid packet */
    SP_UNREC_ETHIPv4_PKT	= 9, /* unrecognized IPv4 packet */
    SP_VALID_UDL0_PKT	 	= 10, /* valid UDL PROG0 packet	*/
    SP_VALID_UDL1_PKT	 	= 11, /* valid UDL PROG1 packet	*/
    SP_VALID_UDL2_PKT	 	= 12, /* valid UDL PROG2 packet	*/
    SP_VALID_UDL3_PKT	 	= 13 /* valid UDL PROG3 packet */
} StarPro_PktType_t;

/* PPB packet driver memory */
typedef struct
{
    void *p_mem;   // pointer to memory section
    uint32_t size; // size of the memory section
} StarProPPB_mem_t;

/* PLL configuration */
typedef struct
{       
        uint32_t        f_in;                                                   
/* Input frequency                                      */
        uint32_t        f_out;                                                  
/* Output frequency                             */
        uint8_t         ecko_sel;                                               
/* Clock output select                          */
        uint8_t         odiv;                                                   
/* Output devider                                       */
        uint8_t         mult;                                                   
/* Multiplier                                           */                     
        uint8_t         idiv;                                                   
/* Input devider                                        */
} StarProPPB_PllCfg_t;

/* DSS identifier */
typedef enum{
        SP_DSS_ID_DSS0  = 0,                                    
/* DSP subsystem 0                                              */
        SP_DSS_ID_DSS1  = 1,                                    
/* DSP subsystem 1                                              */
        SP_DSS_ID_DSS2  = 2,                                    
/* DSP subsystem 2                                              */ 
} StarProPPB_DssId_t;

typedef enum
{
        SP_PPB_DTCM             = 0,                                    
/* ARM DataTCM                                                  */
        SP_PPB_SRAM             = 1,                                    
/* Packet Processor SRAM                                */
        SP_PPB_PCEMEM   = 2,                                    
/* Packet Processor PCE memory                  */
        SP_PPB_SYSMEM   = 3,                                    
/* System memory                                                */
        SP_PPB_DSSMEM   = 4,                                    
/* DSS local memory                                             */
        SP_PPB_EXTMEM   = 5                                             
/* External memory                                              */
} StarProPPB_MemLoc_t; 

typedef enum
{ 
        SP_PPB_ANY              = 0,                                    /* no allignment required                               */
        SP_PPB_1BYTE    = 1,                                    /* allignment at 1 byte boundary                */
        SP_PPB_2BYTES   = 2,                                    /* allignment at 2 bytes boundary               */
        SP_PPB_4BYTES   = 3,                                    /* allignment at 4 bytes boundary               */
        SP_PPB_8BYTES   = 4,                                    /* allignment at 8 bytes boundary               */
        SP_PPB_16BYTES  = 5,                                    /* allignment at 16 bytes boundary              */
        SP_PPB_32BYTES  = 6,                                    /* allignment at 32 bytes boundary              */
        SP_PPB_64BYTES  = 7,                                    /* allignment at 64 bytes boundary              */
        SP_PPB_128BYTES = 8                                             /* allignment at 128 bytes boundary             */
} StarProPPB_MemAlgn_t;

typedef struct
{                                                                                       uint32_t                                size;                   
/* memory size                                                  */              
        StarProPPB_MemLoc_t             loc;                    
/* memory location                                              */                                      StarProPPB_MemAlgn_t    algn;                   
/* memory alignment requirement                 */
} StarProPPB_mem_req_t;

#endif /* SP_PPB_TYPES_H_ */

/******** History ********
$Log: sp_ppb_types.h,v $
Revision 1.2  2017/07/28 07:58:38  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.1.2.1  2017/06/29 08:14:33  harrchan
Initial commit code for Oakenshield

Revision 1.2  2012/07/17 20:34:28  srane
cleanup

Revision 1.1  2012/04/18 09:50:19  srane
Initial checkin


$Endlog$
*/

