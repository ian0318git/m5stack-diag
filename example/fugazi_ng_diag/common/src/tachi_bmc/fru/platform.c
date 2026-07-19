/* $Id: platform.c,v 1.2 2016/04/20 08:41:35 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/fru/platform.c,v $
 *
 *      File:   platform.c
 *
 *      Description:
 *
 *
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *
 *********************************************************************/

#include <fcntl.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <termio.h>
#include "diag_main.h"
#include "ipmi_sprom.h"
#include "ipmi_sprom_ops.h"
#include "diag_sprom.h"
#include "diag_platform.h"
#include "platform_gpio.h"

int blade_sprom_rd_hw (uint8_t *psprom, int index)
{
        diag_sprom_t	*dsprom = NULL;

    printf("Rd index = %d\n", index);
        dsprom = ipmi_sprom_init(SPROM_PLATFORM_IBMC, 
                    ((index > 1) ? SPROM_UTIL_TYPE_OPLIN : 
                     SPROM_UTIL_TYPE_SUP), 0);
        if (!dsprom) {
                printf("Error: sprom information failed\n");
                return (DIAG_SOFTWARE_ERROR);
        }

    return(diag_sprom_rd (dsprom, 0x00, (index > 1) ?
                sizeof(sprom_ipmi_mezz_t) :
                sizeof(sprom_ipmi_ibmc_t), 
                psprom));
}

int blade_sprom_wr_hw (uint8_t *psprom, int index)
{
        diag_sprom_t	*dsprom = NULL;

        dsprom = ipmi_sprom_init(SPROM_PLATFORM_IBMC, 
                    ((index > 1) ? SPROM_UTIL_TYPE_OPLIN : 
                     SPROM_UTIL_TYPE_SUP), 0);
        if (!dsprom) {
                printf("Error: sprom information failed\n");
                return (DIAG_SOFTWARE_ERROR);
        }

    return(diag_sprom_wr (dsprom, 0x00, (index > 1) ?
                sizeof(sprom_ipmi_mezz_t) :
                sizeof(sprom_ipmi_ibmc_t), 
                psprom));
}

int diag_board_serial_no_get (uint8_t *pserial, uint8_t psize)
{
    return (ipmi_sprom_brd_serial_no_get(SPROM_UTIL_TYPE_IBMC, pserial, psize));
}

