/* $Id: dash_reg_test.c,v 1.2 2019/06/14 05:24:48 mikech2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/katar/x86/dash_reg_test.c,v $
 *------------------------------------------------------------------
 *
 * Copyright (c) 2014-2019 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "platform_fpga.h"
#include "common_utils.h"
#include "common.h"
#include "error.h"
#include "defs.h"


static reg_info_t scratchpad_regs[] = {
    {"Scratchpad Register",        0x00,  READ_WRITE, {BW_32BITS}, 0x7FFFFFFF,  0},
    {"END",                        0x000,       0,  {0}, 0x0,     0x0},
};


/*-------------------------------------------------------------------
 *
 * Function : test_scratchpad_ctrl
 * Description: test scratchpad control
 * INPUT:  NONE
 * OUTPUT: PASSED or FAILED
 * -------------------------------------------------------------------
*/
static int
katar_test_scratchpad_ctrl (void)
{
    unsigned long scratchpad;
    
    prpass(testpass, "Scratchpad control registers, ");
    scratchpad = get_scratchpad_reg_addr();

    if (register_tests(scratchpad, scratchpad_regs)) {
        return FAILED;
    }

    return PASSED;
}

extern unsigned long get_platform_reg_base (void);
/*-------------------------------------------------------------------
 *
 * Function : dash_rd_wr_test
 * Description: main entry to fpga register test
 * 
 * INPUT:  none
 * OUTPUT: PASSED or FAILED
 * -------------------------------------------------------------------
*/
int
katar_dash_rd_wr_test (int dummy)
{
    unsigned long addr;
    unsigned int val;
    char *tname = "DASH FPGA register";

    testname("%s", tname);

    addr = get_platform_reg_base();
    val = *((unsigned int *)(addr +  0x80));
    if (val != 0x05000012) {
      cterr('f', 0, "failed reading LPC Board Type Register");
    }
	
    if (katar_test_scratchpad_ctrl()) {
        return FAILED;
    } else {
        printf("passed\n");
    }
    
    return PASSED;
}


/*
 *------------------------------------------------------------------
 * $Log: dash_reg_test.c,v $
 * Revision 1.2  2019/06/14 05:24:48  mikech2
 * Collapse katar-branch00 to Main Trunk
 *
 * Revision 1.1.2.2  2019/02/12 08:06:28  mikech2
 * rename katar_*.h files
 *
 * Revision 1.1.2.1  2019/01/29 01:54:19  mikech2
 * rename katar_* files
 *
 * Revision 1.1.2.1  2018/10/22 08:02:24  mikech2
 * Move project folder to common/src/katar/x86
 *
 * Revision 1.1.2.3  2018/09/07 02:16:52  mikech2
 * Fix FPGA util issue
 *
 * Revision 1.1.2.2  2018/06/29 07:17:31  mikech2
 * Remove compile warning and unused files
 *
 * Revision 1.1.2.1  2018/06/21 08:24:09  mikech2
 * remove unused menu, add scratchpad reg test
 *
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
