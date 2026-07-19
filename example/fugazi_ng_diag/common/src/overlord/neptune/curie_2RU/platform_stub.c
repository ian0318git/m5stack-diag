/* $Id: platform_stub.c,v 1.1 2020/01/09 01:02:04 jiajliu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_2RU/platform_stub.c,v $
 *------------------------------------------------------------------
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <setjmp.h>
#include <assert.h>
#include <stdio.h>
#include "types.h" /* cli_cookie_cmd */
#include "cli_cmd.h" /*cli_cookie_cmd */


void   (*should_pause_diag)(void) = NULL;

int netflashbooted = 1;

jmp_buf monjmpbuf, *monjmpptr;


type_t
smartchip_authenticate_retest(uchar type, uchar slot)
{
    return 0;
}


int
smartchip_authenticate(uchar type, uchar slot)
{
    return 0;
}

void
get_pci_bus_no (int slot, int i, int j)
{
    assert(!"not supported\n");
}

void
dma_intr (int irq, void *a)
{

}

void
hts_intr (void)
{

}

/**********************************************************************
 *
 * Function: get_mem_overhead_factor
 *
 * Description: Return the overhead factor for memory test
 *
 * Input:  None
 * Output: overhead factor
 * **********************************************************************
 */

float get_mem_overhead_factor(void) {
    return 0.006;
}


/* the following functions and variable are not used for curie */
int bcm_uid = 0;

int
ovld_get_ge_sw_port_num (int slot, int tgt_device, int local_port)
{
    //    printf("%s:Curie not support GESW \n", __FUNCTION__); 
    return 0; 
}

int get_gesw_line_loopback(int port_num)
{
    //    printf("%s:Curie not support GESW \n", __FUNCTION__); 
    return 0; 
}

int set_gesw_line_loopback(int port_num, int onoff)
{
    //    printf("%s:Curie not support GESW \n", __FUNCTION__); 
    return 0; 
}

int ovld_bcm_check_port_init (void)
{
    //    printf("%s:Curie not support GESW \n", __FUNCTION__); 
    return 0; 
}

void port_tx_util(void)
{
    //    printf("%s:Curie not support GESW \n", __FUNCTION__); 
    return;
}


int bcm_gesw_ge_link_status_get(int unit, int port, int *status) 
{
    //    printf("%s:Curie not support GESW \n", __FUNCTION__); 
    return 0; 
}

int
bcm_gesw_config (void)
{
    //    printf("%s:Curie not support GESW \n", __FUNCTION__); 
    return 0; 
}

void set_canis_loopback(int slot, int mode)
{
    //    printf("%s:Curie not support GESW \n", __FUNCTION__); 
    return; 
}

int cfg_10gkr_port(int port, int en_10gkr)
{
    //    printf("%s:Curie not support GESW \n", __FUNCTION__); 
    return 0; 
}

int is_gesw_1g_intf (int slot_type, int slot_num, int ngio_port_num)
{
    //    printf("%s:Curie not support GESW \n", __FUNCTION__); 
    return 0; 
}

int is_bcm_greyhound (void)
{
    //    printf("%s:Curie not support GESW \n", __FUNCTION__); 
    return 0; 
}

int exec_bcm_shell_cmd (int unit, char *cmd, int print_cmd)
{
    //    printf("%s:Curie not support GESW \n", __FUNCTION__); 
    return 0; 
}

int plx_pcie_utp_ext_lpbk_test (uint32_t test_port) 
{
    //    printf("%s:Curie not support PCIE SW \n", __FUNCTION__); 
    return 0; 
}

int ovld_pcie_8prbs_ext_lpbk_test (uint32_t test_port)
{
    //    printf("%s:Curie not support PCIE SW \n", __FUNCTION__); 
    return 0; 
}

int pcie_dump_port_regs_util (uint32_t sw_port)
{
    //    printf("%s:Curie not support PCIE SW \n", __FUNCTION__); 
    return 0; 
}

int is_poe_present (char *buf)
{
    //    printf("%s:Curie not support PoE     \n", __FUNCTION__); 
    return 0; 
}

int clear_snsr_alert (void)
{
    //    printf("%s:Curie not support MAX1617 \n", __FUNCTION__); 
    return 0; 
}

int gen_snsr_alert (void)
{
    //    printf("%s:Curie not support MAX1617 \n", __FUNCTION__); 
    return 0; 
}

int freq_mrgn_x (int type, int level, boolean spread)
{
    //    printf("%s:Curie not support freq mrgn \n", __FUNCTION__); 
    return 0; 
}

/*
 *-----------------------------------------------------------------------------
$Log: platform_stub.c,v $
Revision 1.1  2020/01/09 01:02:04  jiajliu
Merge Curie 2RU to main trunk

$Endlog$
 *-----------------------------------------------------------------------------
 */
