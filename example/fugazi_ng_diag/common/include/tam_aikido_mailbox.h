/* $Id: tam_aikido_mailbox.h,v 1.3 2019/07/11 12:34:40 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/tam_aikido_mailbox.h,v $
 *------------------------------------------------------------------
 *
 * FILE NAME: tam_aikido_mailbox.h
 *
 * Aug 2016 - TAM Aikido Mailbox Header File
 *
 * Copyright (c) 2017-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 */

#ifndef __TAM_AIKIDO_MAILBOX_H__
#define __TAM_AIKIDO_MAILBOX_H__

#define WR_DATA_LEN             (0x400)
#define MAX_ADDR_OFFSET         0xFFFFFF

#define MBX_REG_BASE_ADDR       (0x0000C000)
#define MBX_MSG_SIZE            (0x700)
#define MBX_USE_INTERRUPT       (0)

#define TAM_AIKIDO_RESET_TOUT     (1000)
#define TAM_AIKIDO_IRQ_TIMEOUT    (0x4000000)
#define TAM_AIKIDO_WDG_TIMEOUT    (0x2000000)
#define TAM_AIKIDO_POWER_ON_RESET (0x1000000)
#define TAM_AIKIDO_RESET_DELAY    (1000)

#define ACT_RW_DELAY            8000
#define AIKIDO_TAM_RESET        0x37
#define AIKIDO_ADDR_BASE        0xfb000000
#define AIKIDO_TAM_INTERRUPT    0x1000
#define AIKIDO_TAM_RESOURCE     0x2000
#define AIKIDO_MAILBOX          0xC000
#define AIKIDO_DUAL_PORT_SRAM   0x2000
#define AIKIDO_TAM_STATUS       0x200C
/*  bit16 & bit19 = soft reset event and fw ready */
#define AIKIDO_TAM_READY        0x90000 

/* smartfusion2 type and start address for tam and fgpa
 * upgrade field. */
#define AIKIDO_UP_TAM_FW_OFFSET           (0x280000) 
#define AIKIDO_UNKNOWN_FLASH_TYPE         (0xFF)
#define AIKIDO_M2S005S    (1)
#define AIKIDO_M2S005S_UP_FGPA_BIT_OFFSET (0x36D000) 

#define AIKIDO_M2S010S    (2)
#define AIKIDO_M2S010S_UP_FGPA_BIT_OFFSET (0x3CF000) 

#define AIKIDO_M2S010TS   (2)
#define AIKIDO_M2S010TS_UP_FGPA_BIT_OFFSET (0x3CF000) 

#define AIKIDO_M2S025TS   (3)
#define AIKIDO_M2S025TS_UP_FGPA_BIT_OFFSET (0x46F000) 

#define AIKIDO_M2S050TS   (4)
#define AIKIDO_M2S050TS_UP_FGPA_BIT_OFFSET (0x593000) 

#define AIKIDO_M2S060TS   (5)
#define AIKIDO_M2S060TS_UP_FGPA_BIT_OFFSET (0x591000) 

#define AIKIDO_M2S090TS   (6)
#define AIKIDO_M2S090TS_UP_FGPA_BIT_OFFSET (0x700000) 

#define AIKIDO_M2S150TS   (7)
#define AIKIDO_M2S150TS_UP_FGPA_BIT_OFFSET (0x961000) 


#endif /* __TAM_AIKIDO_MAILBOX_H__ */
/*
 *------------------------------------------------------------------
 * $Log: tam_aikido_mailbox.h,v $
 * Revision 1.3  2019/07/11 12:34:40  alicehua
 * Collapse Nutella codes into main trunk
 *
 * Revision 1.2.112.2  2019/07/08 04:52:15  alicehua
 * Added -DAIKIDO_SUPPORT_AIK flag.
 *
 * Revision 1.2.112.1  2019/03/08 05:51:24  harrchan
 * 1.Add utility for Aikido FPGA upgrade
 *
 * Revision 1.2  2017/08/02 14:21:28  steja
 * Support TSN-H/M platform code
 *
 * Revision 1.1.4.2  2017/07/29 03:40:43  steja
 * tsn-branch5 synced with Maintrunk repositories
 *
 * Revision 1.1.2.1  2017/07/21 09:17:31  iachang
 * clean up code
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
