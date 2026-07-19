/* $Id: patriot_intr.h,v 1.1 2014/03/25 02:12:34 huanngo Exp $
 * $Source: 
 *******************************************************************************
 * File Name: patriot_intr.h
 *
 * Description: Header file for interrupt structures
 *
 *      
 * Author: Huan Ngo
 * Copyright (c)2011-2014 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *******************************************************************************
 */


#define CHECK_ALL_INT              0xF
#define CHECK_ISR_PISR             0x1
#define CHECK_ISR_FMSR             0x2
#define CHECK_OOFIE_SEFIE_OOMFIE   0x4
#define CHECK_OOFL_SEFL_OOMFL      0x8

#define eos(s) ((s)+strlen(s))

typedef struct sm_patriot_eth_intr_iface_t_ {
    int eth_tx_intr_cnt;
    int eth_rx_intr_cnt;

} sm_patriot_eth_intr_iface_t;

typedef struct sm_patriot_fpga_intr_iface_t_ {
    int fpga_intr_cnt;

} sm_patriot_fpga_intr_iface_t;

typedef struct sm_patriot_framer_intr_iface_t_ {
    int framer_intr_cnt;

} sm_patriot_framer_intr_iface_t;

/******** History ********/ 
/*------------------------------------------------------------------------------
 * $Log: patriot_intr.h,v $
 * Revision 1.1  2014/03/25 02:12:34  huanngo
 * Adding patriot_linux directory to ng_diag code tree
 *
 * Revision 1.3  2014/03/06 01:56:51  steja
 * 1. added cli command for margining patriot voltage
 * 2. enhance framer interrupt and ecc memory test timing
 *
 * Revision 1.2  2012/05/08 23:52:55  huanngo
 * Support SM Patriot on ngd main code tree
 *
 * Revision 1.1.2.2  2012/03/13 13:37:39  steja
 * Support Framer Interrupt
 *
 * Revision 1.1.2.1  2011/12/21 23:48:36  huanngo
 * Adding structures for interrupts
 *
 *------------------------------------------------------------------------------
 * $Endlog$
 *
 *------------------------------------------------------------------------------
 */
