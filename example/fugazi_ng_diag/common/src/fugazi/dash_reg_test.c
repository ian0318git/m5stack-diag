/* $Id: dash_reg_test.c,v 1.2 2021/06/02 08:22:34 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/dash_reg_test.c,v $
 *------------------------------------------------------------------
 *
 * Filename: dash_reg_test.c
 * mcharon
 * Description: register test for dash
 * Copyright (c) 2013-2020 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "dash_fpga.h"
#include "common_utils.h"
#include "error.h"
#include "defs.h"

#define FAILED 1
#define PASSED 0
#define MASK_32 0xFFFFFFFF

static reg_info_t cpld_regs[] = {
    {"LPC reserve",       0x0,  READ_ONLY,  {4}, MASK_32,  MASK_32},
    {"LPC reset reason",  0x4,  READ_ONLY,  {4}, 0x1ff,  1},
    {"LPC scratchpad",    0x8,  READ_ONLY,  {4}, MASK_32,  0},
    {"LPC status",        0xC,  READ_ONLY,  {4}, 0x30,  0},
    {"LPC reset ctrl",    0x10, READ_ONLY,  {4}, 0x2,  0},
    {"LPC led",           0x14, READ_WRITE, {4}, 0x3,  2},
    {"LPC debug",         0x18, READ_ONLY,  {4}, 0x3,  2},
    {"LPC reset",         0x1C, READ_ONLY,  {4}, 0x47F,0},
    {"LPC flash led",     0x20, READ_WRITE, {4}, 0x30, 0},
    {"LPC irq0 status",   0x24, READ_ONLY,  {4}, 0x2A00, 0},
    {"LPC irq0 mask",     0x28, READ_WRITE, {4}, 0x2A00, 0x2A00},
    {"LPC chasis  N/A",   0x44, READ_ONLY,  {4}, 0x0, 0},
    {"LPC alrm mgmt N/A", 0x48, READ_ONLY,  {4}, 0x0, 0},
    {"LPC  chasis test",  0x54, READ_WRITE, {4}, 0x00040000, 0},
    {"LPC  chasis test",  0x54, READ_WRITE, {4}, 0xFF000000, 0},
    {"LPC contrl0",       0x58, READ_ONLY,  {4}, 0xFFFF, 0},
    {"LPC contrl1",       0x5C, READ_WRITE, {4}, 0xFFFF3FFF, 0},
    {"LPC bd type",       0x80, READ_ONLY,  {4}, 0xFF, 0x18},
    {"LPC version",       0x84, READ_ONLY,  {4}, 0x0, 0},
    {"END",               0x000,       0,   {0}, 0x0,     0x0},
};

static reg_info_t lpc_pwcy_regs[] = {
    {"LPC Power Cycle",   0x60, READ_WRITE, {4}, 0xFFFF3FFF, 0x0},
    {"END",               0x000,         0, {0}, 0x0,        0x0},
};

static reg_info_t system_regs[] = {
    {"ext device ext rst",       0x4,  READ_ONLY,  {4}, 0xFF,  0},
    {"ext device in rst",        0x8,  READ_WRITE, {4}, 0x1FFFF,  0x1FFFF},
    {"ext device pin ctrl",      0x10, READ_ONLY,  {4}, 3,  0},
    {"ext device bd rev",        0x80, READ_ONLY,  {4}, 0xFF,  0x18},
    {"ext device fpga rev",      0x84, READ_ONLY,  {4}, 0x0,  0},
    {"END",                      0x00, 0,          {0}, 0x0,     0x0},
};

static reg_info_t intr_sts_regs[] = {
    {"intr sts ",       0x0,  READ_ONLY,  {4}, 0x3FFF,  0},
    {"intr enable",     0x4,  READ_WRITE, {4}, 0x3FFF,  0},
    {"intr sfp sts",    0x10, READ_ONLY,  {4}, 0xF,  0},
    {"intr sfp enable", 0x14, READ_WRITE, {4}, 0xF, 0},
    {"intr c2w sts",    0x20, READ_ONLY,  {4}, 0x1FFFF,  0},
    {"intr c2w enable", 0x24, READ_WRITE, {4}, 0x1FFFF,  0},
    {"intr c2w overide",0x28, READ_WRITE, {4}, 0x1FFFF,  0},
    {"intr uart sts",   0x30, READ_ONLY,  {4}, 0x1FF,  0x0},
    {"intr uart enable",0x34, READ_WRITE, {4}, 0x1FF,  0x0},
    {"intr uart overide",0x38, READ_WRITE,{4}, 0x1FF,  0x0},
    {"intr mod OIR sts",0x40, READ_ONLY, {4}, 0x73,  0x0},
    {"intr mod OIR enable",0x44, READ_WRITE,  {4}, 0x73,  0},
    {"intr mod OIR overide",0x48, READ_WRITE,  {4}, 0x73,  0},
    {"intr misc sts",    0x50, READ_WRITE,  {4}, 0xF,  0},
    {"intr misc enable", 0x54, READ_WRITE,  {4}, 0xF,  0},
    {"intr misc overide", 0x58, READ_WRITE,  {4},0xF, 0},
    {"END",               0x000,       0,  {0}, 0x0,     0x0},
};

static reg_info_t spi_prom_regs[] = {
    {"spi prom status", 0x04, READ_ONLY, {BW_32BITS}, 0xA, 0},
    {"END",               0x000,       0,  {0}, 0x0,     0x0},
};

static reg_info_t scratchpad_regs[] = {
    {"Scratchpad Register",        0x00,  READ_WRITE, {BW_32BITS}, 0xffffffff,  0},
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
static int test_scratchpad_ctrl (void)
{
    unsigned long scratchpad;
    
    prpass(testpass, "Scratchpad control registers, ");
    scratchpad = get_sys_low_level_base();

    if (register_tests(scratchpad, scratchpad_regs)) {
        return FAILED;
    }

    return PASSED;
}

/*-------------------------------------------------------------------
 *
 * Function : dash_rd_wr_test
 * Description: main entry to fpga register test
 * do not test mbx. which can contain information placd their by
 * NIOS.
 * INPUT:  flga not used
 * OUTPUT: PASSED or FAILED
 * -------------------------------------------------------------------
*/
int dash_rd_wr_test (int flag)
{
    unsigned long addr;
    unsigned int val;
    char *tname = "DASH FPGA register";

    testname("%s", tname);

    assert(dash_cpld);
    assert(dash_fpga);

    /*CSCvc48599 :Diag enhance the LPC register test.*/
    prpass(testpass, "LPC Power Cycle Register, ");
    addr = get_cpld_addr();
    if (register_tests(addr, lpc_pwcy_regs)) {
        return FAILED;
    }

    /* When flag > 0, the test can only run by Intel cpu
     */
    if (flag > 0) {
        addr = get_platform_prom_addr();
        val = register_read(addr + spi_prom_regs[0].offset, spi_prom_regs[0].size.size);
        if (!(val & 0xa)) {
	        cterr('f', 0, "failed reading spi prom status register");
	    }
    }

    if (test_scratchpad_ctrl()) {
        return FAILED;
    } else {
        printf("passed\n");
    }
    
    return PASSED;

}

/*-------------------------------------------------------------------
 *
 * Function : display_fpga_regs (int dummy)
 * Description: display fpga resiter
 * INPUT:  dummy not used.
 * OUTPUT: PASSED or FAILED
 * -------------------------------------------------------------------
*/
int display_fpga_regs (int dummy)
{

    FILE *fp;
    unsigned int i, tmp;
    unsigned long cpld;
    unsigned long fpga, offset;
    reg_info_t *ptr;
    
    cpld = dash_cpld;
    fpga = dash_fpga;
    printf("cpld @%#lx: fpga @%#lx; \n", dash_cpld, dash_fpga);

    assert(cpld);
    assert(fpga);

    if ((fp = fopen("fpga_regs.txt", "w"))==NULL) {
        printf("unable to open file");
        exit(0);
    }

    /* print cpld regs */
    fprintf(fp, "*****RST_CPLD Registers:*******\n");
    ptr = (reg_info_t *)&cpld_regs;
    
    for (i = 0; ptr->size.size != 0; i+=4, ptr++) {
        offset = ptr->offset;
        tmp = *((unsigned int *)(cpld +  offset));
        fprintf(fp, "%s: @%#lx=%#x\n", ptr->name, offset , tmp);
        printf("%s: @%#lx=%#x\n", ptr->name, offset, tmp);
    }


    printf("hit return to continue\n");getchar();
    
    fprintf(fp, "****System Low Level Registers:********\n");
    ptr = (reg_info_t *)&system_regs;
    
    for (i = 0; ptr->size.size != 0; i+=4, ptr++) {
        offset = ptr->offset;
        tmp = *((unsigned int *)(fpga +  offset));
        fprintf(fp, "%s: @%#lx=%#x\n", ptr->name, offset , tmp);
        printf("%s: @%#lx=%#x\n", ptr->name, offset, tmp);
    }

    fprintf(fp, "\n****Interrupt status and Control Registers:********\n");
    ptr = (reg_info_t *)&intr_sts_regs;
    
    for (i = 0; ptr->size.size != 0; i+=4, ptr++) {
        offset = ptr->offset;
        tmp = *((unsigned int *)(fpga +  offset));
        fprintf(fp, "%s: @%#lx=%#x\n", ptr->name, offset , tmp);
        printf("%s: @%#lx=%#x\n", ptr->name, offset, tmp);
    }

    printf("output saved to file fpga_regs.txt\n");

    return(PASSED);
}


/*-------------------------------------------------
 * $Log: dash_reg_test.c,v $
 * Revision 1.2  2021/06/02 08:22:34  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.8.2  2020/08/26 02:37:47  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.6.4  2020/08/04 08:37:06  iachang
 * Update Copyright to 2020
 *
 * Revision 1.1.6.3  2020/07/29 08:57:34  iachang
 * Code clean up.
 *
 * Revision 1.1.6.2  2019/03/14 03:48:35  letsai
 * Initial check in.
 *
 *
 *
 * $Endlog$
 * */
