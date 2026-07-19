/* $Id: ngsm_thule.c,v 1.3 2014/11/26 07:00:42 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/ngsm_thule.c,v $
 *------------------------------------------------------------------
 *
 * ngsm_thule.c - This file contains functions for Thule NGSM carrier card.
 *
 * bowang3 -- Jul. 2014
 *
 * Copyright (c) 2013-2014 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include "common.h"
#include "types.h"
#include "defs.h"
#include "signals.h"
#include "menu.h"
#include "error.h"
#include "proto.h"
#include "strings.h"
#include "nvmonvars.h"
#include "dev_ltc4215.h"
#include "oir_ltc4215_api.h"
#include "platform_i2c.h"
#include "ngio.h"
#include "slot.h"
#include "i2c_api.h"
#include "plat_defs.h"
#include "common_utils.h"
#include "linux_api.h"
#include "cross_platform.h"

#include <string.h>
#include <termios.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

static struct ngio_intf_t *thule_iface;

extern void init_carrier_wic(struct ngio_intf_t *, int);
extern int carrier_wic_test(void *, int, int, int);
extern void pca_init_i2c(void *);

/*------------------------------------------------------------------------------
 *
 * Function: thule_test().
 *
 * Description: This function is the entry point for Thule NGSM test .
 *
 * Input:  sm - pointer to ngio_intf_t struct
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */

int
thule_test (void *sm)
{
    int slot, ret_val=FAILED;
    ushort board_id = 0;
    n2g_i2c_if_t *pca;

    assert(sm);

    thule_iface = (struct ngio_intf_t *)sm;

    slot = thule_iface->slot;
    board_id = thule_iface->id;

    thule_iface->uart_on(sm);

    printf("Thule_id %#x, SM slot %d test\n", board_id, slot);

    testname("Slot%d Thule NGSM carrier card", slot);

    pca = (n2g_i2c_if_t *)malloc(sizeof(n2g_i2c_if_t));
    memset(pca, 0, sizeof(n2g_i2c_if_t));

    pca_init_i2c((void *)pca);
    pca->i2c_ctrl = thule_iface->i2c_ctrl;
    pca->i2c_dev = NGWIC_I2C_ADDR_IO_PORT;

    pca->buf = (void *)malloc(COOKIE_SIZE_512);
    memset(pca->buf, 0, COOKIE_SIZE_512);

    thule_iface->dc->pca = (void *)pca;

    printf("thule menu display is %d\n",thule_iface->menu_display);

    init_carrier_wic(thule_iface->dc, slot);

    /* To make slot <> real_slot and submenu will be displayed */
    if (thule_iface->test_type == IFACE_TEST) {
        carrier_wic_test(thule_iface->dc, slot, slot, IFACE_TEST);
    } else {
        if (thule_iface->menu_display == TRUE) {
            carrier_wic_test(thule_iface->dc, slot, slot+1, FULL_TEST);
        } else {
            carrier_wic_test(thule_iface->dc, slot, slot, FULL_TEST);
        }
    }

    thule_iface->off = NULL;

    return ret_val;
}

/******** History ********
$Log: ngsm_thule.c,v $
Revision 1.3  2014/11/26 07:00:42  alpeng
Support NGSM+NGWIC+NGVM case

Revision 1.2  2014/10/13 06:20:22  bowang3
Add support to I/O interface test

Revision 1.1  2014/07/01 09:14:57  bowang3
Initial check in support file of NGSM carrier card Thule


$Endlog$
*/


