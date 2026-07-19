/* $Id: cetus_gesw_api.c,v 1.3 2017/03/30 08:30:53 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/cetus_gesw_api.c,v $
 *------------------------------------------------------------------
 *
 * bcm_gesw_api.c - User API for CETUS switch setup. This API is
 *                  based on the Tachi's GE switch API.
 *
 * Aug 2015, Mecca Ho
 *
 * Copyright (c) 2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include "types.h"
#include "common.h"
#include "cetus_gesw_defs.h"
#include "plat_defs.h"

/*******************************************************************/
/*****     Code for GESW API     ***********************************/
/*******************************************************************/


/*------------------------------------------------------------------
 *
 * Function: port_tx_util
 * This utility use the tx command provided in the BCM shell to
 * transmit a packet out of a port. This is a very useful utility to
 * help NGIO module diag development.
 *
 * Input: void
 *
 * Return: void
 *
 *------------------------------------------------------------------
 */
void port_tx_util(void)
{
    printf("No implement port tx utility. Add for compiler...\n");
}

/*------------------------------------------------------------------
 *
 * Function: get_gesw_line_loopback
 * Return the state of the line loopback setting of the port
 *
 * Input: port_num - the GE port number
 *
 * Return: 1 for set, 0 for clear, -1 error
 *
 *------------------------------------------------------------------
 */
int get_gesw_line_loopback(int port_num)
{
    printf("No implement gesw line loopback. Add for compiler...\n");
    return (ENABLE);
}

/*------------------------------------------------------------------
 *  
 * Function: tachi_get_ge_sw_port_num
 *
 *
 * Input: 
 *	slot - target device slot number (SM and WIC starts with 1,
 *             others starts with 0)
 *      tgt_device - target devices are:
 *	    TGT_DEV_CPU, TGT_DEV_NGWIC, TGT_DEV_NGVM, TGT_DEV_NGSM
 *      local_port - CPU 2 sgmii ports, SM and WIC has 2 ports (E0, E1)
 *
 * Output: ge switch port number or -1 if error
 *
 *------------------------------------------------------------------
 */
int tachi_get_ge_sw_port_num (int slot, int tgt_device, int local_port)
{
	printf("To be developed...\n");
    return (PASSED);
}


/******** History ******** 
$Log: cetus_gesw_api.c,v $
Revision 1.3  2017/03/30 08:30:53  hondwang
Tachi-L brach merge

Revision 1.2.14.1  2017/01/09 12:17:29  hondwang
Add Wallander support

Revision 1.2  2016/04/20 11:25:24  benchen2
add tachi fru portion

Revision 1.1.2.1  2015/08/11 07:44:28  meho
Added f35 nim tests.


$Endlog$
*/
