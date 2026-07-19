/* $Id: diag_aikido_test.h,v 1.2 2021/06/02 08:22:34 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/diag_aikido_test.h,v $
 *------------------------------------------------------------------
 * Filename: diag_aikido_test.h
 *
 * Description: Diagnostic Aikido Mailbox Register test header file.
 *
 * Copyright (c) 2019-2020 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DIAG_AIKIDO_TEST_H__
#define __DIAG_AIKIDO_TEST_H__


#define AIKIDO_MAILBOX_REG      0xE000
#define AIKIDO_REG_TEST_LEN     0x4
#define DEBUG_INDEX             256 
#define RIPPLE_0_PATTERN        0xFF 
#define FUGAZI_PATTERN          0x5a 
#define REG_TEST_BITS           8 


extern int aikido_mailbox_reg_test(void);
extern unsigned int aikido_spi_read(unsigned, unsigned, unsigned, unsigned, 
                                    unsigned char *); 
extern unsigned int aikido_spi_write(unsigned, unsigned, unsigned, unsigned, 
                                     unsigned char *); 

#endif    /* __DIAG_AIKIDO_TEST_H__ */

/*
 *------------------------------------------------------------------
 * $Log: diag_aikido_test.h,v $
 * Revision 1.2  2021/06/02 08:22:34  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.4.2  2020/08/26 02:37:47  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.2.3  2020/08/05 11:18:06  iachang
 * Code clean up.
 *
 * Revision 1.1.2.2  2020/07/30 05:51:30  iachang
 * code clean up
 *
 * Revision 1.1.2.1  2019/03/26 17:16:54  iachang
 * Add Aikido register test.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
