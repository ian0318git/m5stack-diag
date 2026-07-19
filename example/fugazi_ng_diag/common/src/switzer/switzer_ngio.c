/* $Id: switzer_ngio.c,v 1.1 2020/05/22 02:28:47 qingcwan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/switzer/switzer_ngio.c,v $
 *------------------------------------------------------------------
 *
 * switzer_ngio.c - Switzer WIC nigo interfaces.
 *
 * Mar. 2019, Shiyu Wu <shiywu@cisco.com>
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <string.h>
#include <errno.h>

#include "switzer_priv.h"
#include "switzer_common.h"
#include "switzer_ngio.h"

extern void
mdelay (unsigned long t);

int switzer_ngiowic_present(struct switzer_ng_t *wic)
{
    return (wic->ctrl & SWITZER_NGIO_PRSNT);
}

static int switzer_ngio_enable(struct switzer_ng_t *wic)
{
    wic->ctrl &= ~(SWITZER_NGIO_UART_TX) |
        (SWITZER_NGIO_I2C_RESET | SWITZER_NGIO_RESET);

    mdelay(20); /* for enable pcie ref clock */

    wic->ctrl |= SWITZER_NGIO_PWR_EN;

    mdelay(30);
    if (wic->ctrl & SWITZER_NGIO_PWR_EN) {
        /* rails need to be powered up within 500m of NGIO_PWR_EN
           being set to 1
         */
        wic->intr |= SWITZER_NGIO_FLT_INTR;
        switzer_mdelay(600);
        /* power fault intr enable */
        /* insert/removal intr already on */
        return 0;
    }
    prt("ngio status register reports module power is not up. NGIO_PWR_EN "
           " bit4 is not set.\n");

    return -1;
}

/*-------------------------------------------------------------------
 *
 * Function : switzer_ngiowic_disable
 * Description: power down wic
 * INPUT:  wic -- pointer to struct switzer_ng_t
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
void switzer_ngiowic_disable(struct switzer_ng_t *wic)
{
    /* Per HW suggestion,
     * pci ready shoud be pulled down to generate
     * interrupt to kernel via pcie switch.
     * we don't want to change o2/usd code,
     * so we using a if statement to isolate on NTPN machines
     */
    wic->ctrl &= ~(SWITZER_NGIO_PCI_RDY | SWITZER_NGIO_UART_TX);

    /* delay 3 sec, wait for os to handle remove intr
     * then reset and power off module
     */
    switzer_mdelay(3000);
    switzer_ngiowic_reset(wic);
    switzer_ngiowic_i2c_reset(wic);

    switzer_mdelay(3000); /* using 1 sec delay is not stable enough */
    wic->ctrl &= ~(SWITZER_NGIO_PWR_EN);

    switzer_mdelay(600);
}

/*-------------------------------------------------------------------
 *
 * Function : switzer_ngiowic_i2c_unrset
 * Description: unreset i2c on wic
 * INPUT:  wic -- pointer to struct switzer_ng_t
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
int switzer_ngiowic_i2c_unreset(struct switzer_ng_t *wic)
{
    int i;

    wic->ctrl &= ~SWITZER_NGIO_I2C_RESET;
    for (i = 0; i < 60; i++) {
        if (wic->ctrl & SWITZER_NGIO_PWR_OK)
            break;
        switzer_mdelay(10);
    }

    if (wic->ctrl & SWITZER_NGIO_PWR_OK) {
        switzer_mdelay(50);
        return 0;
    }

    prt("ngio status register reports module power is not up. NGIO_PWR_OK "
           " bit16 is not set.\n");

    return -1;
}

/*-------------------------------------------------------------------
 *
 * Function : switzer_ngiowic_unreset
 * Description: unreset wic
 * INPUT:  wic -- pointer to struct switzer_ng_t
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
int switzer_ngiowic_unreset(struct switzer_ng_t *wic)
{
    wic->ctrl &= ~(SWITZER_NGIO_RESET | SWITZER_NGIO_PCI_RDY);
    wic->ctrl |= SWITZER_NGIO_SRC_SEL;
    /* This code only applicable on Victory platform with
     * PLX PCIe switch.
     */
    /*if (is_ngio_use_pcie(p)) {
	    ngio_plx_intr_mask(p, 1);
    }*/
    return 0;
}

/*-------------------------------------------------------------------
 *
 * Function : switzer_ngiowic_i2c_reset
 * Description: reset i2c on wic
 * INPUT:  wic -- pointer to struct switzer_ng_t
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
int switzer_ngiowic_i2c_reset(struct switzer_ng_t *wic)
{
    wic->ctrl |= SWITZER_NGIO_I2C_RESET;
    return 0;
}

/*-------------------------------------------------------------------
 *
 * Function : switzer_ngiowic_reset
 * Description: reset wic
 * INPUT:  wic -- pointer to struct switzer_ng_t
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
int switzer_ngiowic_reset(struct switzer_ng_t *wic)
{
    wic->ctrl |= SWITZER_NGIO_RESET;
    return 0;
}

/*-------------------------------------------------------------------
 *
 * Function : switzer_ngiowic_enable_uart
 * Description: enable wic uart
 * INPUT:  wic -- pointer to struct switzer_ng_t
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
int switzer_ngiowic_enable_uart(struct switzer_ng_t *wic)
{
    wic->ctrl |= SWITZER_NGIO_UART_TX;
    return 0;
}

/*-------------------------------------------------------------------
 *
 * Function : switzer_ngiowic_disable_uart
 * Description: disale wic uart
 * INPUT:  wic -- pointer to struct switzer_ng_t
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
int switzer_ngiowic_disable_uart(struct switzer_ng_t *wic)
{
    wic->ctrl &= ~SWITZER_NGIO_UART_TX;
    return 0;
}

int switzer_ngiowic_enable(struct switzer_ng_t *wic)
{
    return switzer_ngio_enable(wic);
}

static inline void switzer_ngiowic_pci_on(struct switzer_ng_t *wic)
{
    wic->ctrl |= SWITZER_NGIO_SRC_SEL;
    wic->ctrl |= SWITZER_NGIO_PCI_RDY;
    switzer_mdelay(2000);
}

static inline void switzer_ngiowic_pci_off(struct switzer_ng_t *wic)
{
    wic->ctrl &= ~SWITZER_NGIO_PCI_RDY;
}

void switzer_ngiowic_pci_rdy(struct switzer_ng_t *wic, int on)
{
    if (on) {
        switzer_ngiowic_pci_on(wic);
    } else {
        switzer_ngiowic_pci_off(wic);
    }
}
