/* $Id: platform_stub.c,v 1.2 2019/08/06 06:56:14 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_1RU/platform_stub.c,v $
 *------------------------------------------------------------------
 * Copyright (c) 2019 by Cisco Systems, Inc.
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

uint32_t ovld_poe_psu_pwr_control (uint32_t option)
{
    // no power control so far 
    return 0; 
}


/******** History ******** 
$Log: platform_stub.c,v $
Revision 1.2  2019/08/06 06:56:14  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.1.2.10  2019/04/03 08:40:44  alpeng
add psu poe check back to curie per HW request.

Revision 1.1.2.9  2018/11/08 01:03:35  ptong
Silence all the stub functions

Revision 1.1.2.8  2018/10/31 12:27:37  alpeng
curie psu is the same as Juno, fixed basic util, will fix the remaining psu utils.

Revision 1.1.2.7  2018/10/29 09:18:57  leschen
Move get_gesw_pname to linux_main.c and return gesw port name is not applicable to Curie.

Revision 1.1.2.6  2018/10/26 07:34:09  leschen
Move function is_plat_10gkr_capable to linux_main.c and return true to support 10g-kr.

Revision 1.1.2.5  2018/09/27 09:46:24  alpeng
support tam lib and aikido for curie

Revision 1.1.2.4  2018/07/19 09:27:37  alpeng
1. Moving IR3570 chips to CPU I2C bus. 2. Removed related code of i2c devices which are not support on Curie; max1617, IDT8T49N287I(sys clk), poe psu, 30w poe

Revision 1.1.2.3  2018/07/10 07:27:23  alpeng
add is_radium() and is_polonium(); remove pcie switch files from Makefile

Revision 1.1.2.2  2018/06/28 10:19:19  alpeng
remove bcm gesw files from Makefile and put its functions into platform_stub.c for NGIO reference; will follow GB method on NGIO GE SW portion

Revision 1.1.2.1  2018/06/22 08:05:19  alpeng
move curie diag to neptune/curie_1RU directory

Revision 1.1.2.1  2018/05/30 02:39:37  alpeng
porting neptune x86 to curie

Revision 1.2  2018/05/18 09:25:00  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.1.2.3  2017/07/05 06:31:15  alpeng
update fan info, update PSU and remove pem files

Revision 1.1.2.2  2017/04/05 08:29:45  leschen
Sync with <ng_diag-tag-032917>

Revision 1.1.2.1  2016/06/02 22:04:02  jskow
Move Overlord/x86 specific files to Neptune/x86.

Revision 1.4  2013/07/22 19:37:03  mcharon
move hts to utah dir/add platform_stub


$Endlog$
*/
