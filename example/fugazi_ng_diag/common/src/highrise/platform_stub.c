/* $Id: platform_stub.c,v 1.1 2020/08/19 09:49:36 markzha Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/highrise/platform_stub.c,v $
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


int sgmii_lpbk_util(int port, int pkt_cnt)
{
    printf("FIX THIS: %s %s", __FUNCTION__, __FILE__);
    return (PASSED);
}

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

/***************************************************
 * Function: i2c_quack_reset
 * Description: toggle ACT2 RESET pin
 *
 * Seems this's a legacy function
 * Overlord/Curie1RU/Curie2RU:
 *     MB ACT2    - toggle QUACK_RST_L from DASH
 * TSN:
 *     MB ACT2    - do nothing
 * Highrise:
 *     MB ACT2    - do nothing
 */
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


/*-------------------------------------------------
 * $Log: platform_stub.c,v $
 * Revision 1.1  2020/08/19 09:49:36  markzha
 * *** empty log message ***
 *
 * Revision 1.2  2019/01/10 06:36:25  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
