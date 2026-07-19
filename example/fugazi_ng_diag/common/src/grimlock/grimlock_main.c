/* $Id: grimlock_main.c,v 1.2 2020/03/13 12:06:53 letsai Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/grimlock/grimlock_main.c,v $
 *------------------------------------------------------------------
 * grimlock_main.c
 *
 * Wilbur Huang -- Jan. 2020
 *
 * Copyright (c) 2020 by Cisco Systems, Inc.
 * All rights reserved.
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
#include "grimlock_fpga.h"
#include "grimlock.h"

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

static uint open_module(int *, uint);

extern void diag_menu (int argc, const char *argv[]);
extern int grimlock_fpga_download();

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
static int parse_module_pid ()
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

static void set_gpio_ready_bit ()
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
 
int main(int argc, const char *argv[])
{    
    void *base_ptr;
    uint rc = FAILED;

    //atexit(cleanup_before_exit);

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
	/* only download Grimlock FPGA image for P1A or P1B hw */
	rc = grimlock_fpga_download();
	if (rc != PASSED) {
	    return (rc);
	}
    }

    /* attach framer device */
    /*
    rc = framer_attach();
    if (rc != PASSED) {
	return (rc);
    }
    */

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
$Log: grimlock_main.c,v $
Revision 1.2  2020/03/13 12:06:53  letsai
Merge Grimlock NIM to maintrunk

Revision 1.1.4.3  2020/01/15 09:28:49  wilbhuan
Removed T1/E1 framer function.

Revision 1.1.4.2  2020/01/15 03:30:11  wilbhuan
1. Initial code of Grimlock NIM application.
2. Leveraged from Fortitude Grimlock NIM.
3. Only replace all Fortitude related word as Grimlock.
4. Fortitude's T1/E1 function doesn't remove.

$Endlog$
*/
