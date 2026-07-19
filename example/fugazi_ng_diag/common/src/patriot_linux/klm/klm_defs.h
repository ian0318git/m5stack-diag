/* $Id: klm_defs.h,v 1.1 2014/03/25 02:12:43 huanngo Exp $
 * $Source: 
 *******************************************************************************
 * File Name: klm_defs.h
 *
 * Description: definition needed for klms
 *
 *      
 * Original author mcharon
 * Copyright (c)2012 ~ 2014 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *******************************************************************************
 */

#ifndef __KLM_DEFS__
#define __KLM_DEFS__
#ifdef LINUX_APP
#include <linux/ioctl.h>
#endif

#ifdef LINUX_KLM
#include <linux/ioctl.h>
#endif

#define CHECK_ISR_PISR             0x1
#define CHECK_ISR_FMSR             0x2
#define CHECK_OOFIE_SEFIE_OOMFIE   0x4
#define CHECK_OOFL_SEFL_OOMFL      0x8

typedef struct sm_cmd_iface_t_ {
    int slot;
    int cmd;
    int param1;
} sm_cmd_iface_t;

typedef struct asic_cmd_t_ {
    unsigned int id;         /* what type of asic test we will run */
    unsigned int subtest;
    unsigned int slot;
    unsigned int port;
    unsigned int channel;
    unsigned int param1;
} asic_cmd_t;

typedef struct patriot_intr_dev_t_ {
    int fpga_intr_cnt;
    int (*fpga_get_intr_cnt)(void);
    void (*fpga_clear_intr_cnt)(void);
    int framer_intr_cnt;
    int framer_intr_bit;
    int (*framer_get_intr_cnt)(void);
    void (*framer_clear_intr_cnt)(void);
} patriot_intr_dev_t;

#endif /* __KLM_DEFS__*/

/******** History *********/
/*------------------------------------------------------------------------------
 * $Log: klm_defs.h,v $
 * Revision 1.1  2014/03/25 02:12:43  huanngo
 * Adding patriot_linux directory to ng_diag code tree
 *
 * Revision 1.3  2014/03/06 01:56:52  steja
 * 1. added cli command for margining patriot voltage
 * 2. enhance framer interrupt and ecc memory test timing
 *
 * Revision 1.2  2012/05/08 23:52:56  huanngo
 * Support SM Patriot on ngd main code tree
 *
 * Revision 1.1.4.6  2012/04/12 18:37:03  huanngo
 * Clean up and cosmetic changes
 *
 * Revision 1.1.4.5  2012/03/27 07:45:06  steja
 * Fix Warning compilation
 *
 * Revision 1.1.4.4  2012/03/13 13:26:55  steja
 * Support Framer Interrupt
 *
 * Revision 1.1.4.3  2011/12/21 23:46:32  huanngo
 * Adding tests for FPGA interrupt and fix bug in FPGA i2c intermittent access failure
 *
 * Revision 1.1.4.2  2011/08/18 19:43:27  huanngo
 * Update code to patriot2-branch
 *
 * Revision 1.1.2.1  2011/07/21 20:06:27  huanngo
 * Add support for mem_mgr.ko
 *
 *------------------------------------------------------------------------------
 * $Endlog$
 *
 *------------------------------------------------------------------------------
 */
