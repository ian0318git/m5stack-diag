/* $Id: diag_cpld_lib.c,v 1.2 2019/10/17 02:16:20 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/diag_cpld_lib.c,v $
 *-----------------------------------------------------------------------------
 *
 * Filename   : diag_cpld_lib.c
 * Description: CPLD Library.
 *
 * Copyright (c) 2018-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/io.h>
#include <time.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "defs.h"
#include "error.h"
#include "common.h"
#include "types.h"
#include "nvsysvars.h"
#include <unistd.h>
#include <strings.h>
#include <assert.h>
#include "queryflags.h"
#include "common_utils.h"
#include "queryflags.h"
#include "menu.h"
#include "proto.h"
#include "linux_main.h"
#include "diag_cpld_lib.h"
#include "diag_fpga.h"

/*******************************************************************************
 *                          Function Declaration
 *******************************************************************************
 */
int open_cpld(void);
int cpld_enable_pcie_bar1(void);

/*******************************************************************************
 *                          Global Variables
 *******************************************************************************
 */
static int fd_cpld = -1;
static unsigned long dash_cpld = 0;
int cpld_read_reg(uint, uint32_t *);
int cpld_write_reg(uint, uint);
int cpld_register_operation (uint, uint, uint);
/*****************************************************************************
 *
 * Function   : open_cpld
 * Description: open cpld driver
 * Inputs     : NONE
 *
 * Outputs    : return file descript of cpld driver
 *
 *****************************************************************************/
int open_cpld (void)
{
    void *ptr;

    if (tabei_open_module(&fd_cpld, TABEI_L_CPLD_KLM)==PASSED) {
        if (is_promethium() == TRUE) {
            /* The CPLD address for Promethium  */
            ptr = (void *)mmap(NULL, CPLD_SIZE_BAR1, (PROT_READ | PROT_WRITE),
                               MAP_SHARED, fd_cpld, CPLD_PROMETHIUM_ADDRESS);
        } else {
            /* The CPLD address for TABEI-L, Because there is TPM on it */
            ptr = (void *)mmap(NULL, CPLD_SIZE_BAR1, (PROT_READ | PROT_WRITE),
                               MAP_SHARED, fd_cpld, CPLD_PCIE_BAR1_ADDRESS);
        }
        if (ptr == MAP_FAILED) {
            close(fd_cpld);
            perror("Error mmapping the file for CPLD (TPM space)");
            return (FAILED);
        }
        dash_cpld = (unsigned long)ptr;
#ifdef DEBUG
        printf("CPLD version  %#x %#x\n", *((unsigned int *)((long)dash_cpld + 0x80)),
               *((unsigned int *)((long)dash_cpld + 0x84))  );
#endif
    } else {
        /* we need this only for cpld utility; so if we fail it's ok  */
        printf("*****can't open cpld mmap driver....*******\n");
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : cpld_read_reg
 * Description : Function to read CPLD register.
 * Inputs      : reg_offset - register offset
 *               *buf       - buffer to put read back register value
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int cpld_read_reg (uint reg_offset, uint32_t *buf)
{
    assert(dash_cpld);
    *buf = *(unsigned int *)(dash_cpld + reg_offset);

    return (PASSED);
}


/*******************************************************************************
 *
 * Function    : cpld_write_reg
 * Description : Function performs CPLD register write.
 * Inputs      : reg_offset - register offset
 *               wr_data    - data for write
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int cpld_write_reg (uint reg_offset, uint wr_data)
{
    assert(dash_cpld);
    *(unsigned int *)(dash_cpld + reg_offset) = wr_data;

    return (PASSED);
}
/*******************************************************************************
 *
 * Function   : cpld_register_operation
 * Description: Function to control CPLD register
 * Inputs     : reg_offset - CPLD register address
 *              reg_mask   - CPLD mask
 *              set_val    - set the value to CPLD
 * Outputs    : PASSED /FAILED
 *
 *******************************************************************************
 */
int cpld_register_operation (uint reg_offset, uint reg_mask, uint set_val)
{
    uint32_t reg_val = 0;
    
    /* Access CPLD Register */
    if (cpld_read_reg(reg_offset, &reg_val) != PASSED) {
        printf("Failed to read CPLD register 0x%04X.\n", reg_offset);
        return (FAILED);
    }

    /* Logic operation */
    reg_val &= reg_mask;
    reg_val |= set_val; 
    if (cpld_write_reg(reg_offset, reg_val) != PASSED) {
        printf("Failed to write CPLD register 0x%04X.\n", reg_offset);
        return (FAILED);
    }
    msleep(DELAY_FOR_OPERATION);

    return(PASSED);

}

/*******************************************************************************
 *
 * Function    : cpld_enable_pcie_bar1
 * Description : Function to enable PCIe Bar 1 to access CPLD.
 * Inputs      : void
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int cpld_enable_pcie_bar1 (void)
{
    uint32_t reg_offset = 0;
    unsigned long dash_cpld_bar0 = 0;
    void *ptr;

    /*  Accees PCIe Bar0 to open PCIe Bar1 */
    if (tabei_open_module(&fd_cpld, TABEI_L_CPLD_KLM) == PASSED) {

        ptr = (void *)mmap(NULL, CPLD_SIZE_BAR0, (PROT_READ | PROT_WRITE),
                           MAP_SHARED, fd_cpld, CPLD_PCIE_BAR0_ADDRESS);
        if (ptr == MAP_FAILED) {
            close(fd_cpld);
            perror("Error mmapping the file for CPLD (TPM space)");
            return (FAILED);
        }
        dash_cpld_bar0 = (unsigned long)ptr;
    } else {
        /* we need this only for cpld utility; so if we fail it's ok  */
        printf("*****can't open cpld mmap driver....*******\n");
        return (FAILED);
    }

    reg_offset = CPLD_PCIE_ENABLE_BAR1_REG;
    *(unsigned int *)(dash_cpld_bar0 + reg_offset) = CPLD_PCIE_ENABLE;

    close(fd_cpld);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : cpld_reset_api
 * Description : Function of CPLD to reset/unreset interface.
 * Inputs      : r_offset  - register offset
 *               r_bit     - reset bit of register
 *               r_opt     - reset(TRUE)/un-reset(FALSE)
 *               r_time_ms - the reset time interval(millisecond)
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int cpld_reset_api (uint r_offset, uint r_bit, uint r_opt, uint r_time_ms)
{
    uint reg_val = 0;

    /* Read CPLD interface reset register. */
    if (cpld_read_reg(r_offset, &reg_val) != PASSED) {
        printf("%s: Failed to read FPGA reg.(0x%04X).\n",
               __FUNCTION__, r_offset);
        return (FAILED);
    }

    if (r_opt == TRUE) {
        /* Set the Reset bit. */
        reg_val |= r_bit;
    } else if (r_opt == FALSE) {
        /* Clear the reset bit. */
        reg_val &= (uint)(~r_bit);
    } else {
        printf("%s: Invalid Reset option(%#x).\n", __FUNCTION__, r_opt);
        return (FAILED);
    }

    /* Write the reset/un-reset into the corresponding register bit. */
    if (cpld_write_reg(r_offset, reg_val) != PASSED) {
        printf("%s: Failed to write CPLD reg.(0x%04X).\n",
               __FUNCTION__, r_offset);
        return (FAILED);
    }

    /* Delay milliseconds after reset/un-reset */
    msleep(r_time_ms);

    /* Confirm the change to FPGA interface reset register. */
    reg_val = 0;
    if (cpld_read_reg(r_offset, &reg_val) != PASSED) {
        printf("%s: Failed to read CPLD reg.(0x%04X).\n",
               __FUNCTION__, r_offset);
        return (FAILED);
    }

    if (((r_opt == TRUE) && ((reg_val & r_bit) != r_bit)) ||
        ((r_opt == FALSE) && ((reg_val & r_bit) != 0))) {
        printf("%s: Failed to %s reset bit in CPLD reg.(0x%04X).\n",
               __FUNCTION__, (r_opt == TRUE) ? "set" : "clear", r_offset);
        return (FAILED);
    }

     return (PASSED);
}

/*-------------------------------------------------------------------
 *
 * Function: cpld_set_irq
 * 
 * test irq intr from cpld
 * make sure our KLM intr handler deassert the bit 
 *
 * Input: which_irq
 * Output: NONE
 *
 *-------------------------------------------------------------------
 */
void cpld_set_irq (int which_irq)
{
    unsigned int tmp32;
    assert(dash_cpld);

    rst_cpld_t *cpld = (rst_cpld_t*)dash_cpld;
    /* write 0xCA to enable register */
    tmp32 = ((CPLD_MAGIC_NUM_CA << 24) | which_irq);
    cpld->tst = tmp32;

}


/*-------------------------------------------------------------------
 *
 * Function: cpld_check_irq
 * 
 * check irq intr from cpld
 * make sure our KLM intr handler deassert the bit 
 *
 * Input: which_irq
 * Output: TRUE/FALSE
 *
 *-------------------------------------------------------------------
 */
int cpld_check_irq (int which_irq)
{
    int counter = 0;

    assert(dash_cpld);

    rst_cpld_t *cpld = (rst_cpld_t*)dash_cpld;

    while (cpld->tst & which_irq) {

        msleep(WAITTIME_1000_MS);

        if (counter >= CPLD_TIMEOUT) {
            printf("cpld->tst: %x\n", cpld->tst);
            return (FALSE);
        }

        counter++;

    }
    return (TRUE);

}

/*-------------------------------------------------
 * $Log: diag_cpld_lib.c,v $
 * Revision 1.2  2019/10/17 02:16:20  kehuang2
 * Collapse Tabei-L into main trunk
 *
 * Revision 1.1.2.11  2019/09/27 07:57:23  kehuang2
 * Clean up code
 *
 * Revision 1.1.2.10  2019/09/05 09:30:39  kehuang2
 * Support Promethium Init CPLD
 *
 * Revision 1.1.2.9  2019/09/05 08:50:35  olin2
 * Support FPGA serial IRQ interrupt util
 *
 * Revision 1.1.2.8  2019/06/17 03:23:45  olin2
 * Support CPLD reset api
 *
 * Revision 1.1.2.7  2019/04/29 08:14:26  kehuang2
 * Clean up code
 *
 * Revision 1.1.2.6  2019/04/24 07:59:20  kehuang2
 * Update CPLD access
 *
 * Revision 1.1.2.5  2019/04/19 03:43:04  kehuang2
 * Clean up code
 *
 * Revision 1.1.2.4  2019/04/19 03:33:32  kehuang2
 * Support new FPGA
 *
 * Revision 1.1.2.3  2019/04/12 06:06:44  olin2
 * Support read/write CPLD
 *
 * Revision 1.1.2.2  2018/10/19 01:44:19  harrchan
 * I2C scan test
 *
 * Revision 1.1.2.1  2018/10/15 12:30:12  kodko
 * Add CPLD register read/write function.
 *
 * $Endlog$
 *-------------------------------------------------
 */
