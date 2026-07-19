/* $Id: sm_woodlawn_fpga_lib.c,v 1.6 2018/05/18 09:25:02 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn/sm_woodlawn_fpga_lib.c,v $
 *******************************************************************************
 * File Name: sm_woodlawn.c
 *
 * Description: Woodlawn SM main source file
 *
 * Author: Kody Ko
 *
 * Copyright (c) 2013-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *******************************************************************************
 */


#include "endians.h"
#include "types.h"
#include "defs.h"
#include "signals.h"
#include "error.h"
#include "nvmonvars.h"
#include "nvsysvars.h"
#include "pm_utils.h"
#include "pci.h"
#include "slot.h"
#include "mon_plat_defs.h"
#include "common.h"
#include "common_utils.h"
#include "cookie_4.h"
#include "proto.h"
#include "string.h"
#include "sm_slot.h"
#include "menu.h"
#include "router_if.h"
#include "sgmii_defs.h"
#include "dev_ltc4215.h"
#include "oir_ltc4215_api.h"
#include "platform_slot.h"
#include "cross_platform.h"
#include "platform_i2c.h"
#include "sm_woodlawn.h"
#include "ngio.h"
#include "plat_defs.h"

#include "sm_woodlawn_fpga_lib.h"


#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

long woodlawn_fpga_reg_test(void);

long util_oir_fpga_reg_read(void *);
long util_oir_fpga_reg_write(void *);
void fpga_init_i2c(int, void *);
int woodlawn_fpga_check_ver(void);

static int woodlawn_fpga_read_fn(unsigned long, int, unsigned long *, void *);
static int woodlawn_fpga_write_fn(unsigned long, int, unsigned long, void *);
extern n2g_i2c_if_t woodlawn_fpga_i2c; 

#define READ_WRITE 0
#define REG_ACCESS 8
#define FPGA_RW (READ_WRITE | REG_ACCESS)

static reg_info_t_ext fpga_reg_ext = {1, woodlawn_fpga_read_fn,
                              woodlawn_fpga_write_fn, 0};

static n2g_i2c_if_t fpga = 
{
    .dev_name = "Woodlawn_FPGA",
    .offset = 0,
    .i2c_bus_type = IOFPGA_I2C,
    .size = 0x1,
    .mux = I2C_MUX_ZERO,
    .buf = NULL,
    .i2c_ctrl = I2C_CTRL_TEN,
    .i2c_dev = 0x20,
};

static reg_info_t fpga_test_regs[] = {
    {"GPIO Expander 0 Output Register", FPGA_GPIO_EXP0_OUT_REG, FPGA_RW,
        {(unsigned long)&fpga_reg_ext}, 0x2, 0x0},
    {"END", 0x0 ,0 ,{0}, 0x0, 0x0},
};

/**********************************************************************
 *
 * Function: fpga_init_i2c
 *
 * Init FPGA I2C.
 *
 * Input : slot - slot number
 *           dev - FPGA I2C structure
 *
 * Output: None
 *
 **********************************************************************
 */
void fpga_init_i2c (int slot, void *dev)
{
    assert(dev);
    memcpy(dev, &fpga, sizeof(n2g_i2c_if_t));
}

/**********************************************************************
 *
 * Function: woodlawn_fpga_reg_read
 *
 * Wrapper for FPGA Register Read utility.
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long util_oir_fpga_reg_read (void *p)
{
    n2g_i2c_if_t *i2c_p = (n2g_i2c_if_t *)p;
    uchar fpga_buf[1];
    unsigned int reg_addr;
    int rc = PASSED;
    
    printf("\n\nFPGA Register Read\n\n");
    reg_addr = gethex_answer("Reg offset to read", 0, 0, 0xff);
    i2c_p->offset = reg_addr;
    i2c_p->buf = (char *)fpga_buf;

    rc = n2g_i2c_read(i2c_p);

    if (rc != PASSED) {
        cterr('f', 0, "Unable to read i2c, ret_code = %#x\n", rc);
        rc = FAILED;
    } else {
        printf("\nRegister @ %#x = %#x\n", reg_addr, fpga_buf[0]);
    }

    return (rc);
}

/**********************************************************************
 *
 * Function: woodlawn_fpga_reg_write
 *
 * Wrapper for FPGA Register write utility.
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long util_oir_fpga_reg_write (void *p)
{
    n2g_i2c_if_t *i2c_p = (n2g_i2c_if_t *)p;
    uint8_t fpga_write_buf[1];
    unsigned int write_addr;
    int rc = PASSED;
    printf("\n\nFPGA Register Write\n\n");

    write_addr = gethex_answer("Reg offset to write", 0, 0, 0xff);
    i2c_p->offset = write_addr;
    
    fpga_write_buf[0] = gethex_answer("Enter new reg value", 0x0, 0, 0xff);
    
    i2c_p->buf = (char *)&fpga_write_buf[0];

    /* alter reg with new value */
    rc = n2g_i2c_write(i2c_p);
    if (rc != PASSED) {
        printf("unable to write i2c.\n");
        rc = FAILED;
        return (rc);
    }

    return (rc);
}

/**********************************************************************
 *
 * Function: woodlawn_fpga_read_fn
 *
 * Read Woodlawn FPGA Register.
 *
 * Input : addr - Register offset
 *           size - Register size
 *           buf - Read buffer 
 *           param -param
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int woodlawn_fpga_read_fn (unsigned long addr, int size, unsigned long *buf, void *param)
{
    n2g_i2c_if_t *i2c_if = &woodlawn_fpga_i2c; 
    int rc = PASSED;

    i2c_if->offset = (unsigned int)addr;
    i2c_if->buf = (char *)buf;

    rc = n2g_i2c_read(i2c_if);
    if (rc != PASSED) {
        cterr('f', 0, "Unable to read i2c, ret_code = %#x\n", rc);
        rc = FAILED;
        return (rc);
    }

    return (rc);
}

/**********************************************************************
 *
 * Function: woodlawn_fpga_write_fn
 *
 * Write Woodlawn FPGA Register.
 *
 * Input : addr - Register offset
 *           size - Register size
 *           data - data for write 
 *           param -param
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int woodlawn_fpga_write_fn (unsigned long addr, int size, unsigned long data, void *param)
{
    n2g_i2c_if_t *i2c_if = &woodlawn_fpga_i2c;
    int rc = PASSED;
    i2c_if->buf = (char *)&data;
    i2c_if->offset = (unsigned int)addr;
   
    rc = n2g_i2c_write(i2c_if);
    if (rc != PASSED) {
        printf("unable to write i2c.\n");
        rc = FAILED;
        return (rc);
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: woodlawn_fpga_reg_test
 *
 * This function perform the FPGA register test.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long woodlawn_fpga_reg_test (void)
{
    testname("Woodlawn FPGA Register");
    prpass(testpass, "FPGA Register Test");
  
    if (register_tests(0, fpga_test_regs) == FAILED) {
        cterr('f', 0, "FPGA Register Test Failed");
        return (FAILED);
    }
    
    prcomplete(testpass, errcount, 0);
    return (PASSED);
}

/**********************************************************************
 *
 * Function: woodlawn_fpga_check_ver
 *
 * This function check FPGA version.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int woodlawn_fpga_check_ver (void)
{
    n2g_i2c_if_t *i2c_p = &woodlawn_fpga_i2c; 
    uchar fpga_buf[80];
    int rc = PASSED;
    memset(fpga_buf, 0, sizeof(fpga_buf));
    i2c_p->offset = FPGA_HIGH_VER_REG;
    i2c_p->buf = (char *)fpga_buf;
    rc = n2g_i2c_read(i2c_p);
    if (rc != PASSED) {
        cterr('f', 0, "Unable to read fpga mode\n");
        return (FAILED);
    } 

    *fpga_buf &= FPGA_MODE_MASK;
    if (*fpga_buf != FPGA_MODE_MASK) { 
        printf("Indicate FPGA is not in upgrade mode\n");
        return (FAILED); 
    } 
    return (PASSED);
}
/******** History ********/
/*------------------------------------------------------------------------------
 * $Log: sm_woodlawn_fpga_lib.c,v $
 * Revision 1.6  2018/05/18 09:25:02  alpeng
 *  Neptune merge to trunk with tag <neptune-branch-0518>
 *
 * Revision 1.5.30.1  2016/11/07 02:30:57  leschen
 * Modify Woodlawn uart and i2c controller for Neptune.
 *
 * Revision 1.5  2014/10/17 07:43:08  leschen
 * Remove FPGA capable 10G serdes bit to do host side FPGA test
 *
 * Revision 1.4  2014/02/18 09:11:12  alpeng
 * CSCul88171-3: remove useless files: mon_boot.h, c82576_ethmap.h, eth_frames.c, stack.h, sys_regs.h
 *
 * Revision 1.3  2013/11/26 08:40:38  hroni
 * fix compiler warning
 *
 * Revision 1.2  2013/10/08 08:48:26  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.4.2  2013/08/20 10:58:49  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.4  2013/04/25 07:12:39  kodko
 * Add lib to check FPGA version
 *
 * Revision 1.1.2.3  2013/04/10 10:24:03  tirawan
 * Change FPGA register test to test GPIO Expander Output bit
 *
 * Revision 1.1.2.2  2013/03/07 12:55:02  tirawan
 * Update FPGA I2C Address, and add power on sequence for Woodlawn SM
 *
 * Revision 1.1.2.1  2013/02/06 03:04:55  tirawan
 * Woodlawn Support on O2
 *
 *------------------------------------------------------------------------------
 * $Endlog$
 *------------------------------------------------------------------------------
 */

