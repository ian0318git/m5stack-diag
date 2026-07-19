/* $Id: fortitude_main.c,v 1.16 2013/03/08 17:47:41 ywen Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fortitude/fortitude_main.c,v $
 *------------------------------------------------------------------
 *
 * fortitude_main.c - Fortitude main entry.
 *
 * Christine Wen -- Oct. 2011
 *
 * Copyright (c) 2011-2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include "common.h"
#include "types.h" 
#include "pcmap.h"
#include "fortitude_fpga.h"
#include "fortitude.h"

#define WP3_KLM            0
#define OVLD_VTOP          1
#define MODULE_FILE  "/proc/cmdline"

extern char *banner_string;

static int fd_vtop = -1;
static int fd_wp3 = -1;
static char inf_name[32];
static unsigned long rif_base = 0;
static unsigned long fpga_base = 0;
static unsigned long framer_base = 0;
static int module_port_num = 0;
static boolean module_hw_rev_new = FALSE;

static void cleanup_before_exit(void);
static uint open_module(int *, uint);

extern void diag_menu (int argc, const char *argv[]);
extern int framer_attach();
extern void framer_detach();
extern int fortitude_fpga_download();

static void 
cleanup_before_exit (void)
{
    framer_detach();
}

unsigned long
get_npu_rif_base()
{
    return rif_base;
}

unsigned long
get_fpga_base()
{
    return fpga_base;
}

unsigned long
get_framer_base()
{
    return framer_base;
}

int 
get_vtopf_fd ()
{
    return fd_vtop;
}

int 
get_num_ports ()
{
    return module_port_num;
}

boolean
is_hw_rev_new ()
{
    return module_hw_rev_new;
}

/**********************************************************************
 *
 * Function: parse_module_pid()
 *
 * This function will read the PID from /proc/cmdline file to 
 * determine the number of ports supported on the board.
 *
 * Input : None
 *
 * Output: number of ports or -1 if failed
 *
 **********************************************************************
 */
static int
parse_module_pid ()
{
    FILE *fp;
    unsigned char buff[120];
    int port_num = 0;   
  
    fp = fopen(MODULE_FILE, "r");
    if (fp == NULL) {
	printf("%s: can't find file %s\n", __FUNCTION__, MODULE_FILE);
	return (-1);
    }

    fgets(buff, 120, fp);
    printf("read /proc/cmdline buff = %s\n", buff);
    if (strstr(buff, "1MFT") || strstr(buff, "1CE1T1")) {
	port_num = 1;
    } else if (strstr(buff, "2MFT") || strstr(buff, "2CE1T1")) {
	port_num = 2;     
    } else if (strstr(buff, "4MFT")) {
	port_num = 4;   
    } else if (strstr(buff, "8MFT") || strstr(buff, "8CE1T1")) {
	port_num = 8;
    } else {
	printf("\nERROR! Can not find board PID.\n");
	fclose(fp);
        return (-1);
    }

    if (strstr(buff, "rev=1A") || strstr(buff, "rev=1B")) {
	module_hw_rev_new = FALSE;
    } else {
	module_hw_rev_new = TRUE;
    } 

    fclose(fp);
    return (port_num);
}

static void
set_gpio_ready_bit ()
{
    volatile unsigned int *gpdr_reg, *gpvr_reg;
 
    gpdr_reg = (int *)(get_npu_rif_base() + NPU_GPDR_OFFSET);
    gpvr_reg = (int *)(get_npu_rif_base() + NPU_GPVR_OFFSET);
 
    /* Set direction, 0 for input, 1 for output */
    *gpdr_reg |= READY_BIT;
    msleep(100);
 
    /* Set ready bit low */
    *gpvr_reg &= ~READY_BIT;
    msleep(100);
 
    /* Set ready bit high */
    *gpvr_reg |= READY_BIT;
    msleep(100);
 
    printf("GPIO Ready Bit set!\n");
}
 
int 
main(int argc, const char *argv[])
{    
    void *base_ptr;
    uint rc = FAILED;

    atexit(cleanup_before_exit);

    fflush(stdin);

    /* Open modules */
    rc = open_module(&fd_wp3, WP3_KLM);
    if (rc != PASSED) {
	printf("***/dev/wp3 not found***/n");
	return (rc);
    }

    rc = open_module(&fd_vtop, OVLD_VTOP);
    if (rc != PASSED) {
	printf("***/dev/addr_vtop not found***/n");
	return (rc);
    }

    /* the address we want to map to needs to be 4k aligned. */
    /* Currently we are mmaping the memory chunk from 0x41000000 - 0x4202ffff */
    base_ptr = (void *)mmap(NULL, WP3_MMAP_LEN, (PROT_READ | PROT_WRITE), 
			    MAP_SHARED, fd_wp3, NPU_RIF_BASE);
    if (base_ptr == MAP_FAILED) {
	close(fd_wp3);
	perror("Error mmapping the file");
	return (FAILED);
    }
#ifdef DEBUG
    printf("\nRIF_BASE = %#x\n", (ulong)base_ptr);
    printf("intr_reg = %#x\n", *(int *)((ulong)(base_ptr)+0xC2DC));
#endif

    rif_base = (unsigned long)base_ptr;
    fpga_base = rif_base + FPGA_OFFSET;
    framer_base = rif_base + FRAMER_OFFSET;

    rc = parse_module_pid();
    if (rc == -1) {
	printf("***Failed to get module port number***");
	return (FAILED);
    } else if (rc == -2) {
	printf("***Failed to get module hw_rev***");
	return (FAILED);
    } else {
	module_port_num = rc;
    }

    if (is_hw_rev_new() == FALSE) {
	/* only download Fortitude FPGA image for P1A or P1B hw */
	rc = fortitude_fpga_download();
	if (rc != PASSED) {
	    return (rc);
	}
    }

    /* attach framer device */
    rc = framer_attach();
    if (rc != PASSED) {
	return (rc);
    }

    /* take TDMSW out of reset */
    fpga_unreset_tdmsw();

    /* take TDM PLL out of reset */
    fpga_unreset_tdm_pll();

    /* set CTCLK_SRC to be 2MHz */
    fpga_set_ctclk_src(CTCLK_SRC_2M);

    /* turn off over commit to avoid oom during memory test. */
    system("echo 2 > /proc/sys/vm/overcommit_memory");
 
    // is_flag_available(); /* check the file to store the diag flag */

    set_gpio_ready_bit();

    printf("%s", banner_string);

    /* go to menu directly; don't call monitor(); */
    diag_menu(1, argv); 
     
    return(PASSED);
}

/*****************************************************************************
 *
 * Function   : open_module
 * Description: Wrap function to open module
 * Inputs     : The file descriptor pointer of module and module_type
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
uint open_module (int *fd, uint module_type)
{
    uint rc = FAILED;

    /* Clear inf_name buffer */
    memset(&inf_name[0], 0, sizeof(inf_name));

    switch (module_type) {
    case WP3_KLM:
        strcpy(inf_name, "/dev/wp3");
        break;

    case OVLD_VTOP:
         strcpy(inf_name, "/dev/addr_vtop");
         break;

    default:
        printf("unknown device name");
        return (rc);
    }
    *fd = open(inf_name, O_RDWR);
    if (*fd <= 0) {
        return (rc);
    }
    return (PASSED);
}


/******** History ********
$Log: fortitude_main.c,v $
Revision 1.16  2013/03/08 17:47:41  ywen
Update code to support pilot and production build.

Revision 1.15  2012/11/13 17:22:00  ywen
Add version control to Fortitude FW.

Revision 1.14  2012/11/01 21:37:17  ywen
Add code to set primary interface ready bit before bringing up the main menu.

Revision 1.13  2012/10/31 21:28:01  ywen
Add utility to upgrade golden FPGA image.

Revision 1.12  2012/10/15 18:13:40  ywen
Support Rev.2 Fortitude HW

Revision 1.11  2012/10/04 21:39:40  ywen
- Add code to get board HW revision information.
- Add support for different HW revision.

Revision 1.10  2012/08/29 20:07:25  ywen
- Add utility to upgrade FPGA image for P1C and later builds.
- Add peek/poke utilities for FPGA multiboot registers.

Revision 1.9  2012/08/21 23:13:21  ywen
bump up image version.

Revision 1.8  2012/06/25 21:24:09  ywen
revert method for storing diag flag

Revision 1.7  2012/06/13 17:54:34  ywen
Add support for TDMSW16 and 2 port SKU.

Revision 1.6  2012/06/05 22:20:07  ywen
Turn off over commit for memory test.

Revision 1.5  2012/05/23 23:15:00  ywen
Improve code to parse board PID.

Revision 1.4  2012/05/21 22:26:17  ywen
- Add code to check the diag flag file before starting the menu.
- Add version string.

Revision 1.3  2012/05/15 23:30:56  ywen
Add code to get PID from kernel file and parse the port number from that.

Revision 1.2  2012/03/28 00:38:17  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:06  ptong
Initial archive of ng_diag module


$Endlog$
*/
