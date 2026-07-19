/* $Id: diag_aikido_test.c,v 1.2 2021/06/02 08:22:34 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/diag_aikido_test.c,v $
 *------------------------------------------------------------------
 *
 * diag_aikido_test.c - Aikido Mailbox Register test
 *
 *
 * Copyright (c) 2019-2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "endians.h"
#include "defs.h"
#include "types.h"
#include "nvsysvars.h"
#include "error.h"
#include "pcmap.h"
#include "strings.h"
#include "menu.h"
#include "common.h"
#include "proto.h"
#include "diag_aikido_test.h"
#include "diag_common.h"



/*
 * Global variables
 */


/* Local functions */
int aikido_mailbox_reg_test();

/******************************************************************************
 *
 * Function: aikido_mailbox_reg_test
 *
 * Description: Aikido Register Test with Mailbox
 * This function checks for MailBox DPRAM register, 
 * and does a ripple 1 and a ripple 0 test if applicable.
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int aikido_mailbox_reg_test (void)
{
    unsigned int size = 0, address, op, is_addr, ix, jx, data;
    unsigned char buf_bk[AIKIDO_REG_TEST_LEN], buf_wr[AIKIDO_REG_TEST_LEN];
    unsigned char buf_rd[AIKIDO_REG_TEST_LEN]; 

    address = AIKIDO_MAILBOX_REG;
    size = AIKIDO_REG_TEST_LEN;
	testname("Aikido Register");
    
    /*
     * fugazi Aikido Register test access the Mail Box DPRAM
     */

    memset(buf_bk, 0, sizeof(buf_bk));
    memset(buf_wr, 0, sizeof(buf_wr));
    memset(buf_rd, 0, sizeof(buf_rd));

    op = 0; 
    is_addr = 1; 
    size = 0; /* if user want to read 1 byte, size must be equal to 0 */

    for (jx = 0; jx  < AIKIDO_REG_TEST_LEN; jx++, address++) {
        /*
         * back up data
         */
        prpass(testpass, "Backup data , ");
        aikido_spi_read(size, address, op, is_addr, buf_bk); 
    
        /* ripple 1 test */
        prpass(testpass, "ripple 1 test, ");
        for (ix = 0; ix < REG_TEST_BITS; ix++) {
            buf_wr[0] = (1 << ix) ;
            /* Write to register under test */
            aikido_spi_write(size, address, op, is_addr, buf_wr); 
            aikido_spi_read(size, address, op, is_addr, buf_rd); 
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("\n read back data %#x = %#x ",address, buf_rd[0]);
            }
            if (buf_wr[0] != buf_rd[0]) {
                cterr('f', 0, "Ripple one test failed Expect %#x, Read %#x",
                      buf_wr[0], buf_rd[0]);
                return (FAILED);
            }
         }
        buf_wr[0] = RIPPLE_0_PATTERN;
        /* ripple 0 test */
        prpass(testpass, "ripple 0 test, ");
        for (ix = 0; ix < REG_TEST_BITS; ix++) {
            buf_wr[0] = (~(1 << ix)) ;
            /* Write to register under test */
            aikido_spi_write(size, address, op, is_addr, buf_wr); 
            aikido_spi_read(size, address, op, is_addr, buf_rd); 
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("\n read back data %#x = %#x ",address, buf_rd[0]);
            }
            if (buf_wr[0] != buf_rd[0]) {
                cterr('f', 0, "Ripple zero test failed Expect %#x, Read %#x",
                      buf_wr[0], buf_rd[0]);
                return (FAILED);
            }
        }
    
        /* pattern test */
        prpass(testpass, "pattern test, ");
        data = FUGAZI_PATTERN;
        for (ix = 0; ix < REG_TEST_BITS; ix++) {
            buf_wr[0] = data;
            /* Write to register under test */
            aikido_spi_write(size, address, op, is_addr, buf_wr); 
            aikido_spi_read(size, address, op, is_addr, buf_rd); 
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("\n read back data %#x = %#x ",address, buf_rd[0]);
            }
            if (buf_wr[0] != buf_rd[0]) {
                cterr('f', 0, "Pattern test failed Expect %#x, Read %#x",
                      buf_wr[0], buf_rd[0]);
                return (FAILED);
            }
            data = ~data;   /* complement data pattern */
        }
    }
    prpass(testpass, "Restore data , ");
    aikido_spi_write(size, address, op, is_addr, buf_bk); 
    return (PASSED);
}


/*
 *------------------------------------------------------------------
 * $Log: diag_aikido_test.c,v $
 * Revision 1.2  2021/06/02 08:22:34  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.4.2  2020/08/26 02:37:47  iachang
 * Merge Fugazi code into main trunk
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
