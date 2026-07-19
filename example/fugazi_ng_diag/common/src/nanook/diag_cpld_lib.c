/* $Id: diag_cpld_lib.c,v 1.2 2019/12/11 10:10:28 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/diag_cpld_lib.c,v $
 *-----------------------------------------------------------------------------
 *
 * Filename   : diag_cpld_lib.c
 * Description: CPLD Library.
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
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

/*******************************************************************************
 *                          Function Declaration
 *******************************************************************************
 */
int open_cpld(void);

/*******************************************************************************
 *                          Global Variables
 *******************************************************************************
 */
static int fd_cpld = -1;
int cpld_read_reg(uint, uint32_t *);
int cpld_write_reg(uint, uint);
extern unsigned long dash_cpld;

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

    if (nanook_open_module(&fd_cpld, NANOOK_CPLD_KLM)==PASSED) {

        ptr = (void *)mmap(NULL, CPLD_SIZE, (PROT_READ | PROT_WRITE),
                           MAP_SHARED, fd_cpld, 0xFED40000);
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
        /* we need this only for cpld uitility; so if we fail it's ok  */
        printf("*****can't open cpld mmap driver....*******\n");
        return -1;
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

/*-------------------------------------------------
 * $Log: diag_cpld_lib.c,v $
 * Revision 1.2  2019/12/11 10:10:28  lucywang
 * Merged Nanook to main trunk
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
