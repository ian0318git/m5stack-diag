/* $Id: platform_stub.c,v 1.2 2021/09/24 01:21:07 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/elixir/platform_stub.c,v $
 *------------------------------------------------------------------
 * 
 * platform_stub.c
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include "types.h"
#include "common.h"
#include "error.h"
#include "cli_cmd.h"
#include "platform_stub.h"



int interface_up( char* ethname )
{
    printf("FIX THIS: %s %s", __FUNCTION__, __FILE__);
    return (PASSED);
}

int ngiosm_i2c_reset(void *p)
{
    return (PASSED);
}

int ngiosm_i2c_unreset(void *p)
{
    return (PASSED);
}

int ngiovm_i2c_reset(void *p)
{
    return (OK);
}

int ngiovm_i2c_unreset(void *p)
{
    return (OK);
}

int ngiowic_i2c_reset(void *p)
{
    return (OK);
}

int ngiowic_i2c_unreset(void *p)
{
    return (PASSED);
}

int i2c_quack_read_bytes(void)
{
    return (PASSED);
}

int i2c_quack_write_bytes(void)
{
    return (PASSED);
}

int i2c_quack_reset(void)
{
    return (PASSED);
}

int quack_version(sc_context * con)
{
    return (PASSED);
}

struct ngio_intf_t *slot_get_ngiosm(int slot)
{
    return (PASSED);
}

struct ngio_intf_t *slot_get_ngiovm(int slot)
{
    return (PASSED);
}

struct ngio_intf_t *slot_get_ngiowic(int slot)
{
    return (PASSED);
}

int
smart_cookie_read_write_eeprom(sc_context * con, cli_cookie_cmd * cli_cmd)
{
    return (PASSED);
}

int slot_start_with(void)
{
    return (PASSED);
}

int get_max_sm_slots(void)
{
    return (PASSED);
}

/* Below stub functions for AC3/AC5 switch */
int is_goldbeach(void)
{
    return 0;
}

boolean is_nanook(void)
{
    return 0;
}

boolean is_nanook_plus(void)
{
    return 1;
}

int dynamic_get_inface (int ia, char *ib)
{
    return 0;
}

int is_juno_plx(void)
{
    return 0;
}
int is_utah_plx(void)
{
    return 0;
}
int is_sword(void)
{
    return 0;
}
int is_dagger(void)
{
    return 0;
}
int is_ntpn_machines(void)
{
    return 0;
}
int is_vg450(void)
{
    return 0;
}
int get_ngio_pcie_dev_bus_num(uint ia, uint ib)
{
    return 0;
}
int get_sm_device_no(int ia)
{
    return 0;
}
int get_wic_device_no(int ib)
{
    return 0;
}
boolean is_juno(void)
{
    return 0;
}
int is_neptune (void)
{
    return 0;
}
int get_sgmii_port_num(uint port, uint type)
{
    return 0;
}

/**********************************************************************
 *
 * Function: get_mem_overhead_factor
 *
 * Description: Return the overhead factor for memory test
 *
 * Input:  None
 * Output: overhead factor
 *
 **********************************************************************
 */

float get_mem_overhead_factor(void) {
   /* Overhead is needed, or oom-killer will delete pid.*/ 
   return 0.1;
}

/*-------------------------------------------------
 * $Log: platform_stub.c,v $
 * Revision 1.2  2021/09/24 01:21:07  harrchan
 * Collapse Elixir-branch to Main Trunk.
 *
 * Revision 1.1.2.3  2021/05/31 10:50:44  illiu
 * Remove redundant function
 *
 * Revision 1.1.2.2  2020/11/05 06:34:55  harrchan
 * 1.Base on P1A bring up result to Modify the AC5 MAC/internal/external loopback test
 * 2.Remove some debug message on AC5 init process
 *
 * Revision 1.1.2.1  2020/09/09 09:08:08  illiu
 * First version which has been ported with Dreamliner and Marvel CPSS
 *
 * Revision 1.2  2019/01/10 06:36:25  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
