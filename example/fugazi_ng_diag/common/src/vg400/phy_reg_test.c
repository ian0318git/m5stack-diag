/* $Id: phy_reg_test.c,v 1.3 2018/12/21 00:56:43 haohsu Exp $
*  $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/vg400/phy_reg_test.c,v $
*------------------------------------------------------------------
*
* FILE NAME: phy_reg_test.c
*
* Jan 2018 - Sam Hsu
*
* Copyright (c) 2018 by Cisco Systems, Inc.
* All rights reserved.
*
*------------------------------------------------------------------          
*/


#include <stdint.h>
#include <stdio.h>
#include "endians.h"
#include "types.h"
#include "defs.h"
#include "error.h"
#include "common.h"
#include "common_utils.h"
#include "pcmap.h"
#include "cpu.h"
#include "pci.h"
#include "nvsysvars.h"
#include "testmem.h"
#include "proto.h"
#include "extern.h"
#include "queryflags.h"
#include "pm_utils.h"  
#include "dash_fpga.h"
#include "platform_eth.h"


/**********************************************************************
 *
 * Function: phy_register_write
 *
 * Description: Write 1548 PHY register
 *
 * Input : info for all registers
 *
 * Output: PASS/FAIL
 *
 ***********************************************************************
 */
static int phy_register_write (reg_info_t *reg_ptr, ulong reg_addr, uint value) {

    return (vg400_phy_reg_wr(PORT_NUM,reg_addr,value));
}

/**********************************************************************
 *
 * Function: phy_register_read
 *
 * Description: Read 1548 PHY register
 *
 * Input : info for all registers
 *
 * Output: PASS/FAIL
 *
 ***********************************************************************
 */
static int phy_register_read (reg_info_t *reg_ptr, ulong reg_addr, volatile uint *buf) {
    

    return (vg400_phy_reg_rd(PORT_NUM,reg_addr, (ushort *)buf));
}

/**********************************************************************
 *
 * Function: phy_1548_register_tests
 *
 * For each register from reg_ptr, this function checks for accessibility
 * and does a ripple 1 and a ripple 0 test if applicable (not all registers
 * are W/R register).
 *
 * Input : Address of the first register, info for all registers
 *
 * Output: PASS/FAIL
 *
 ***********************************************************************
 */
int phy_1548_register_tests (ulong base_addr, reg_info_t *reg_ptr) {

    unsigned int  ix, rc;
    ulong reg_addr;
    uint temp, readval, data, original_data, size;

    /* calling routines should put in the prpass() */
    while (reg_ptr->size.size != 0) {
        if (reg_ptr->type & REG_ACCESS) {
            /* Caller provided access functions */
            reg_addr = base_addr + reg_ptr->offset;  
        } else {
            /* Direct memory access register */
            reg_addr = base_addr + reg_ptr->offset;
        }

        /*
         * Test a register if it's a R/W register
         */
        if ((reg_ptr->type & (READ_ONLY | WRITE_ONLY)) == READ_WRITE) {
            if (reg_ptr->type & SAVE_RESTORE) {
                /* Save and restore the original data */
                rc = phy_register_read(reg_ptr, reg_addr, &original_data);
                if (rc != PASSED) {
                    cterr('f', 0, "%s Register first read failed at offset %#x",
                        reg_ptr->name, reg_ptr->offset);
                    return (FAIL);
                }
            }
            if (reg_ptr->type & REG_ACCESS) {
            /* Caller provided read/write */
                size = reg_ptr->size.ext->size;
            } else {
            /* Direct memory access */
                if (reg_ptr->size.size > BW_56BITS) {
                    size = (reg_ptr->size.size >> 8);
                } else {
                    size = reg_ptr->size.size;
                }
            }

            /* 
             * ripple 1 test
             */
    
            for (ix = 0; ix < (size * 8); ix++) {
                temp = (1 << ix) & reg_ptr->mask;
                if (!temp) {
                    continue;
                }
                rc = phy_register_write(reg_ptr, reg_addr, temp);
                if (rc != PASSED) {
                    cterr('f', 0, "Ripple one test failed when writing %s "
                        "register at offset %#x with %#x",
                         reg_ptr->name, reg_ptr->offset, temp);
                    return (FAIL);
                }

                rc = phy_register_read(reg_ptr, reg_addr, &readval);
                if (rc != PASSED) {
                    cterr('f', 0, "Ripple one test failed when reading %s "
                        "register at offset %#x with %#x",  
                         reg_ptr->name, reg_ptr->offset, temp);
                    return (FAIL);
                }

                if ((readval&reg_ptr->mask) != temp) {
                    cterr ('f',0,"Ripple one test failed when accessing %s "
                         "Register at %#x. Expect: %#x, Read: %#x.",
                          reg_ptr->name, reg_addr, temp, readval);
                    return (FAIL);   

                }
            }

            /*
             * ripple 0 test
             */

            for (ix = 0; ix < (size * 8); ix++) {
                temp = (1 << ix) & reg_ptr->mask;
                if (!temp) {
                    continue;
                }
                temp = (~(1 << ix)) & reg_ptr->mask;

                rc = phy_register_write(reg_ptr, reg_addr, temp);
                if (rc != PASSED) {
                    cterr ('f',0,"Ripple zero test failed when writing %s"
                        "Register at %#x.",  reg_ptr->name, reg_addr);
                    return (FAIL);
                }

                rc = phy_register_read(reg_ptr, reg_addr, &readval);
                if (rc != PASSED) {    
                    cterr ('f',0,"Ripple zero test failed when reading %s"
                         "Register at %#x.",  reg_ptr->name, reg_addr);
                    return (FAIL);
                }

                if ((readval&reg_ptr->mask) != temp) {
                cterr ('f',0,"Ripple zero test failed when accessing "
                     "%s Register at %#x. Expect: %#x, Read: "
                     "%#x.", reg_ptr->name, reg_addr, temp,
                      readval);
                    return (FAIL);
                }
            }

            /*
             * pattern test
             */
    
            data = PATTERN; 
            for (ix = 0;ix < 2;ix++){
                temp = data &reg_ptr->mask;

                if (!temp) {
                    continue;   
                }

                rc = phy_register_write(reg_ptr, reg_addr, temp);
                if (rc != PASSED) {
                    cterr ('f',0,"Pattern test failed when writing %s "
                        "Register at %#x.", reg_ptr->name, reg_addr);
                    return (FAIL);
                }

                rc = phy_register_read(reg_ptr, reg_addr, &readval);
                if (rc != PASSED) {
                    cterr ('f',0,"Pattern test failed when accessing %s "
                         "Register at %#x. Expect: %#x, Read: %#x.",
                         reg_ptr->name, reg_addr, temp, readval);
                    return (FAIL);
                }

                data = ~PATTERN; /* complement data pattern */
            }

            /*
             * restore reset value
             */
           if (reg_ptr->type & SAVE_RESTORE) {
            /* Restore the save value */
                rc = phy_register_write(reg_ptr, reg_addr, original_data);
            } else {
                rc = phy_register_write(reg_ptr, reg_addr, reg_ptr->reset_val);
            }
            if (rc != PASSED) {
                cterr('f', 0, "Write failed when %s %s Register at %#x",
                    (reg_ptr->type & SAVE_RESTORE) ? "restoring" :
                    "resetting", reg_ptr->name, reg_addr);
                return (FAIL);
            }
        }
        reg_ptr++;  
    }
    return (PASS);
}


/*
 * ------------------------------------------------------------------
 *  $Log: phy_reg_test.c,v $
 *  Revision 1.3  2018/12/21 00:56:43  haohsu
 *  CSCvn39819-Fix 1548 PHY Regsiter test fail
 *
 *  Revision 1.2  2018/08/30 06:47:15  haohsu
 *  Collapse Vg400-branch to Main Trunk
 *
 *  Revision 1.1.2.4  2018/08/01 02:17:32  haohsu
 *  Vg400 code change for branch
 *
 *  Revision 1.1.2.3  2018/03/15 00:30:52  haohsu
 *  Code change for VG400
 *
 *  Revision 1.1.2.2  2018/01/26 09:41:54  haohsu
 *  *** empty log message ***
 *
 *  Revision 1.1.2.1  2018/01/26 08:20:36  haohsu
 *  *** empty log message ***
 *
 *
 *  ------------------------------------------------------------------
 *  $Endlog$
 */
