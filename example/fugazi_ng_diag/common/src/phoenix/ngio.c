/* $Id: ngio.c,v 1.2 2021/04/15 00:52:26 achiu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/phoenix/ngio.c,v $
 *------------------------------------------------------------------
 * ngio.c  check EDCS 1108257 section 2.7.5 for NGIO init sequence
 *
 *
 * Copyright (c) 2015-2019 by Cisco Systems, Inc.
 * All rights reserved.
 * 
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "proto.h"
#include "i2c_api.h"
#include "linux_api.h"
#include "platform_i2c.h"
#include "linux_usb_test.h"
#include "common.h"
#include "slot.h"
#include "ngio.h"
#include "pca.h"
#include "nvmonvars.h"
#include "platform_cookie.h"
#include "cross_platform.h"
#include "plat_defs.h"
#include "linux_pciutils.h"
#include "platform_slot.h"  /* requires slot.h */
#include "diag_fpga.h"

#define PCI_HOTPLUG_BITS         ((1 << 3) + (1 << 4) + (1 << 5))
extern uint pcie_config_read(uint32_t, uint32_t, uint16_t, uint, uint);
extern void pcie_config_write(uint32_t, uint32_t, uint16_t, uint, uint, uint32_t);
extern uint32_t cpu_ge_status(int);
extern int netstat_main(char *);

/* int dev" passed into all routines in this files are 1 based index, so
   we must subtract one from the value
   base on 1108257 
*/
static const unsigned char ngio_err[NGIO_FAIL_END][80] = {
    {"failed to enable power"},
    {"failed to unreset i2c"},
};


/*-------------------------------------------------------------------
 *
 * Function : ngio_get_dev
 * Description: get real device number. 0 index based
 * INPUT:  dev - device number; func -- pointer to name of calling function
 * OUTPUT: real device number
 * -------------------------------------------------------------------
*/
int
ngio_get_dev (int dev, unsigned char * func)
{
    if ((dev - 1) < 0 ) {
        printf("invalid slot number %d %s, \n", dev, func);
        assert(!"ngio.c has invalid slot number\n");
        fflush(stdout);
    }
    return (dev - 1);
}


/*-------------------------------------------------------------------
 *
 * Function : ngiosm_present 
 * Description: check if SM is present
 *              Phoenix design on board Virtual SM always TRUE
 * INPUT:  p -- pointer to struct ngio_intf_t 
 * OUTPUT: TRUE if SM is present
 * -------------------------------------------------------------------
*/
int
ngiosm_present (void *p)
{
    int sts;
    struct ngio_intf_t *intf = (struct ngio_intf_t *)p;
    assert(intf->slot);
    
    ngio_t *ngio = (ngio_t *)(dash_fpga +  NGIO_BASE);

    assert(intf->slot);
    
    sts = ngio->sm[intf->slot-FIRST_SLOT].ctrl;

    return (sts & NGIO_PRSNT);
}


/*-------------------------------------------------------------------
 *
 * Function : is_ngiosm_i2c_unreset
 * Description: check ngiosm i2c reset status.
 * INPUT:  p -- pointer to struct ngio_intf_t
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
int
is_ngiosm_i2c_unreset (void *p)
{
    return 0;
}
  
/*-------------------------------------------------------------------
 *
 * Function : ngiosm_reset
 * Description: reset SM
 * INPUT:  p -- pointer to struct ngio_intf_t 
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
int
ngiosm_reset (void *p)
{
    struct ngio_intf_t *intf = (struct ngio_intf_t *)p;
    ngio_t *ngio = (ngio_t *)(dash_fpga +  NGIO_BASE);
    assert(intf->slot);
    
    /* Phoenix Virtual SM1 use Virtual SM0 Module_reset pin */
    if (intf->slot == SECOND_SLOT) {
        ngio->sm[SM_FIRST_SLOT_OIR].ctrl |= NGIO_RESET;
    }
    ngio->sm[intf->slot-FIRST_SLOT].ctrl |= NGIO_RESET;

    return (OK);
}

/*-------------------------------------------------------------------
 *
 * Function : ngiosm_unreset
 * Description: unreset SM
 * INPUT:  p -- pointer to struct ngio_intf_t 
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
int
ngiosm_unreset (void *p)
{
    ngio_t *ngio = (ngio_t *)(dash_fpga +  NGIO_BASE);
    struct ngio_intf_t *intf = (struct ngio_intf_t *)p;

    assert(intf->slot);
    
    /* Phoenix Virtual SM1 use Virtual SM0 Module_reset pin */
    if (intf->slot == SECOND_SLOT) {
        ngio->sm[SM_FIRST_SLOT_OIR].ctrl |= NGIO_PWR_EN;
        ngio->sm[SM_FIRST_SLOT_OIR].ctrl &= ~(NGIO_I2C_RESET | NGIO_RESET);
    }
    
    ngio->sm[intf->slot-FIRST_SLOT].ctrl &= ~(NGIO_RESET | 
					       NGIO_PCI_RDY);

    ngio->sm[intf->slot-FIRST_SLOT].ctrl |= NGIO_SRC_SEL;
    
    /* This code only applicable on Victory platform with
     * PLX PCIe switch.
     * 
    if (is_ngio_use_pcie(p)) {
        ngio_plx_intr_mask(p, 1);
    }*/

    if ((NVRAM)->diagflag & D_VERBOSE) {
        print_offset_val((char *)__FUNCTION__, dash_fpga,
                         (unsigned long)&ngio->sm[intf->slot-FIRST_SLOT].ctrl,
                         0, 0);
    }

    return (OK);
}

/*-------------------------------------------------------------------
 *
 * Function : ngiosm_i2c_reset
 * Description: reset i2c on SM
 * INPUT:  p -- pointer to struct ngio_intf_t 
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
int
ngiosm_i2c_reset (void *p)
{
    struct ngio_intf_t *intf = (struct ngio_intf_t *)p;
    ngio_t *ngio = (ngio_t *)(dash_fpga +  NGIO_BASE);
    assert(intf->slot);

    ngio->sm[intf->slot-FIRST_SLOT].ctrl |= NGIO_I2C_RESET;

    return (OK);
}

/*-------------------------------------------------------------------
 *
 * Function : ngio_i2c_unreset
 * Description: unreset i2c of a module
 * INPUT:  p -- pointer to struct ngio_intf_t 
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
static int
ngio_i2c_unreset (ng_t *io)
{
    int i;

    io->ctrl &= ~NGIO_I2C_RESET;
    
    for (i = 0; i < 60; i++) {
        if (io->ctrl & NGIO_PWR_OK)
            break;
        msleep(10);
    }
    /*
    print_offset_val((uchar *)__FUNCTION__, dash_fpga, (unsigned long)&io->ctrl,
                     0, 0);
    */

    if (io->ctrl & NGIO_PWR_OK) {
        msleep(I2C_UNRESET_DELAY);
        return 0;
    }

    printf("ngio status register reports module power is not up. NGIO_PWR_OK "
           " bit16 is not set.\n");
    fflush(stdout);

    return -1;

}

/*-------------------------------------------------------------------
 *
 * Function : ngiosm_i2c_unreset
 * Description: un reset i2c on SM by calling ngio_i2c_unrest
 * INPUT:  p -- pointer to struct ngio_intf_t 
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
int
ngiosm_i2c_unreset (void *p)
{
    struct ngio_intf_t *intf = (struct ngio_intf_t *)p;
    ngio_t *ngio = (ngio_t *)(dash_fpga +  NGIO_BASE);
    ng_t *io;
    assert(intf->slot);    
    
    io = (ng_t *)&ngio->sm[intf->slot-FIRST_SLOT];
    return (ngio_i2c_unreset(io));
}

/*-------------------------------------------------------------------
 *
 * Function : ngio_enable
 * Description: power on  modual
 * INPUT:  p -- pointer to struct ngio_intf_t 
 * OUTPUT: PASSED if power on successfully 
 * -------------------------------------------------------------------
*/
static int
ngio_enable (ng_t *io, struct ngio_intf_t *intf)
{
    /* make sure ctrl reg is back in its default state before
       taking powering module */

    io->ctrl &= ~(NGIO_UART_TX) | (NGIO_I2C_RESET | NGIO_RESET);
    io->ctrl |= NGIO_PWR_EN;
    msleep(30);
#if NGIO_DBG
        print_offset_val(__FUNCTION__, dash_fpga, 
                     (unsigned long)&io->ctrl, 0, 0);
#endif 

    if (io->ctrl & NGIO_PWR_EN) {
        /* rails need to be powered up within 500m of NGIO_PWR_EN
           being set to 1
         */
        io->intr |= NGIO_FLT_INTR;
        msleep(ENABLE_DELAY);
        /* power fault intr enable */
        /* insert/removal intr already on */
        return 0;
    }
    printf("ngio status register reports module power is not up. NGIO_PWR_EN "
           " bit4 is not set.\n");
    
    return(-1);
}

/*-------------------------------------------------------------------
 *
 * Function : ngiosm_enable
 * Description: power up SM module by calling ngio_enable
 * INPUT:  p -- pointer to struct ngio_intf_t 
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
int
ngiosm_enable (void *p)
{
    struct ngio_intf_t *intf = (struct ngio_intf_t *)p;
    ngio_t *ngio = (ngio_t *)(dash_fpga +  NGIO_BASE);
    ng_t *io;
    assert(intf->slot);    

    /* Phoenix Virtual SM1 use Virtual SM0 Module_reset pin */
    if (intf->slot == SECOND_SLOT) {
        ngio->sm[SM_FIRST_SLOT_OIR].ctrl |= NGIO_PWR_EN;
        ngio->sm[SM_FIRST_SLOT_OIR].ctrl &= ~(NGIO_I2C_RESET | NGIO_RESET);
    }
    io = (ng_t *)&ngio->sm[intf->slot-FIRST_SLOT];
    return (ngio_enable(io, intf));
}

/*-------------------------------------------------------------------
 *
 * Function : ngiosm_disable
 * Description: turn off SM module
 * INPUT:  p -- pointer to struct ngio_intf_t 
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
void
ngiosm_disable (void *p)
{
    struct ngio_intf_t *intf = (struct ngio_intf_t *)p;
    ngio_t *ngio = (ngio_t *)(dash_fpga +  NGIO_BASE);

    assert(intf->slot);

    ngiosm_reset(p);
    ngiosm_i2c_reset(p);
    
    /* Phoenix Virtual SM1 use Virtual SM0 Module_reset pin */
    if (intf->slot == SECOND_SLOT) {
        ngio->sm[SM_FIRST_SLOT_OIR].ctrl &= ~(NGIO_PWR_EN);
    }

    ngio->sm[intf->slot-FIRST_SLOT].ctrl &= ~(NGIO_PWR_EN);

    ngio->sm[intf->slot-FIRST_SLOT].ctrl &=
        ~(NGIO_PCI_RDY | NGIO_UART_TX);

    if ((NVRAM)->diagflag & D_VERBOSE) {
        print_offset_val((char *)__FUNCTION__, dash_fpga,
                         (unsigned long)&ngio->sm[intf->slot-FIRST_SLOT].ctrl,
                         0, 0);
    }

    msleep(ENABLE_DELAY);
}

/*-------------------------------------------------------------------
 *
 * Function : ngiosm_enable_intr
 * Description: enable SM intr
 * INPUT:  dev -- slot number starting with 1
 *         intr type -- type of interupt
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
void
ngiosm_enable_intr (int dev, int intr_type)
{
}

/*-------------------------------------------------------------------
 *
 * Function : ngiosm_disable_intr
 * Description: disable SM intr
 * INPUT:  dev -- slot number starting with 1
 *         intr type -- type of interupt
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
void
ngiosm_disable_intr (int dev, int intr_type)
{
}

/*-------------------------------------------------------------------
 *
 * Function : ngiosm_enable_uart
 * Description: enable sm uart
 * INPUT:  p -- pointer to struct ngio_intf_t 
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
int
ngiosm_enable_uart (void *p)
{
    struct ngio_intf_t *intf = (struct ngio_intf_t *)p;
    ngio_t *ngio = (ngio_t *)(dash_fpga +  NGIO_BASE);
    assert(intf->slot);

    ngio->sm[intf->slot-FIRST_SLOT].ctrl |= NGIO_UART_TX;

    return (OK);
}

/*-------------------------------------------------------------------
 *
 * Function : ngiosm_disable_uart
 * Description: disale uart
 * INPUT:  p -- pointer to struct ngio_intf_t 
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
int
ngiosm_disable_uart (void *p)
{
    struct ngio_intf_t *intf = (struct ngio_intf_t *)p;
    ngio_t *ngio = (ngio_t *)(dash_fpga +  NGIO_BASE);
    assert(intf->slot);

    ngio->sm[intf->slot-FIRST_SLOT].ctrl &= ~NGIO_UART_TX;

    return (OK);
}

/*-------------------------------------------------------------------
 *
 * Function : ngiowic_present 
 * Description: check if WIC is present
 * INPUT:  p -- pointer to struct ngio_intf_t 
 * OUTPUT: TRUE if wic is present; FALSE otherwise
 * -------------------------------------------------------------------
*/
int
ngiowic_present (void *p)
{
    int sts;
    struct ngio_intf_t *intf = (struct ngio_intf_t *)p;
    assert(intf->slot);
    
    ngio_t *ngio = (ngio_t *)(dash_fpga +  NGIO_BASE);

    assert(intf->slot);
    
    sts = ngio->wic[intf->slot-FIRST_SLOT].ctrl;

    return (sts & NGIO_PRSNT);
}

/*-------------------------------------------------------------------
 *
 * Function : is_ngiowic_i2c_unreset
 * Description: check ngiowic i2c reset status.
 * INPUT:  p -- pointer to struct ngio_intf_t
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
int
is_ngiowic_i2c_unreset (void *p)
{
    ngio_t *ngio = (ngio_t *)(dash_fpga +  NGIO_BASE);
    struct ngio_intf_t *intf = (struct ngio_intf_t *)p;

    assert(intf->slot);

    if (ngio->wic[intf->slot-FIRST_SLOT].ctrl & NGIO_I2C_RESET)
    {
        return (FALSE);
    } else {
        return (TRUE);
    }
}


/*-------------------------------------------------------------------
 *
 * Function : ngiowic_reset
 * Description: reset wic
 * INPUT:  p -- pointer to struct ngio_intf_t 
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
int
ngiowic_reset (void *p)
{
    struct ngio_intf_t *intf = (struct ngio_intf_t *)p;
    ngio_t *ngio = (ngio_t *)(dash_fpga +  NGIO_BASE);
    assert(intf->slot);
    
    //    dev = ngio_get_dev(dev, (unsigned char *)__FUNCTION__);
    ngio->wic[intf->slot-FIRST_SLOT].ctrl |= NGIO_RESET;

    return (OK);
}

/*-------------------------------------------------------------------
 *
 * Function : ngiowic_unreset
 * Description: unreset wic
 * INPUT:  p -- pointer to struct ngio_intf_t 
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
int
ngiowic_unreset (void *p)
{
    ngio_t *ngio = (ngio_t *)(dash_fpga +  NGIO_BASE);
    struct ngio_intf_t *intf = (struct ngio_intf_t *)p;

    assert(intf->slot);
    
    ngio->wic[intf->slot-FIRST_SLOT].ctrl &= ~(NGIO_RESET | 
					       NGIO_PCI_RDY);

    ngio->wic[intf->slot-FIRST_SLOT].ctrl |= NGIO_SRC_SEL;
    
    if ((NVRAM)->diagflag & D_VERBOSE) {
        print_offset_val((char *)__FUNCTION__, dash_fpga,
                         (unsigned long)&ngio->wic[intf->slot-FIRST_SLOT].ctrl,
                         0, 0);
    }

    return (OK);
}

/*-------------------------------------------------------------------
 *
 * Function : ngiowic_i2c_reset
 * Description: reset i2c on wic
 * INPUT:  p -- pointer to struct ngio_intf_t 
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
int
ngiowic_i2c_reset (void *p)
{
    struct ngio_intf_t *intf = (struct ngio_intf_t *)p;
    ngio_t *ngio = (ngio_t *)(dash_fpga +  NGIO_BASE);
    assert(intf->slot);

    ngio->wic[intf->slot-FIRST_SLOT].ctrl |= NGIO_I2C_RESET;

    return (OK);
}

/*-------------------------------------------------------------------
 *
 * Function : ngiowic_i2c_unrset
 * Description: unreset i2c on wic
 * INPUT:  p -- pointer to struct ngio_intf_t 
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
int
ngiowic_i2c_unreset (void *p)
{
    struct ngio_intf_t *intf = (struct ngio_intf_t *)p;
    ngio_t *ngio = (ngio_t *)(dash_fpga +  NGIO_BASE);
    ng_t *io;
    assert(intf->slot);    
    
    io = (ng_t *)&ngio->wic[intf->slot-FIRST_SLOT];
    return (ngio_i2c_unreset(io));

}

/*-------------------------------------------------------------------
 *
 * Function : ngiowic enable
 * Description: power up wic
 * INPUT:  p -- pointer to struct ngio_intf_t 
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
int
ngiowic_enable (void *p)
{
    struct ngio_intf_t *intf = (struct ngio_intf_t *)p;
    ngio_t *ngio = (ngio_t *)(dash_fpga +  NGIO_BASE);
    ng_t *io;
    assert(intf->slot);    

    io = (ng_t *)&ngio->wic[intf->slot-FIRST_SLOT];
    return (ngio_enable(io, intf));

}

/*-------------------------------------------------------------------
 *
 * Function : ngiowic_disable
 * Description: power down wic
 * INPUT:  p -- pointer to struct ngio_intf_t 
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
void
ngiowic_disable (void *p)
{
    struct ngio_intf_t *intf = (struct ngio_intf_t *)p;
    ngio_t *ngio = (ngio_t *)(dash_fpga +  NGIO_BASE);
    char buf[128], file_rmv[128];
    unsigned char bus, dev, func, offset, val, reg; 
    size_t size = 0;

    printf("\nngiowic_disable\n");

    assert(intf->slot);
    if (intf->mod_type == WIC_MODULE) {
        switch (intf->slot) {
            case NGWIC1_SLOT:
                bus = PHOENIX_NIM1_PCIE_UPPORT_BUS;
                dev = PHOENIX_NIM1_PCIE_UPPORT_DEV;
                func = PHOENIX_NIM1_PCIE_UPPORT_FUNC;
                offset = PHOENIX_NIM1_PCIE_UPPORT_2ND_BUS_OFF;
                reg = pci_config_read(bus, dev, func, offset);

                sprintf(file_rmv, "%s0000:%02x:00.0/remove",
                        PHOENIX_NIM1_PCIE_UPPORT_PATH, reg);
                offset = PHOENIX_NIM1_PCIE_UPPORT_2ND_RESETOFF;
                break;

            case NGWIC2_SLOT:
                bus = PHOENIX_NIM2_PCIE_UPPORT_BUS;
                dev = PHOENIX_NIM2_PCIE_UPPORT_DEV;
                func = PHOENIX_NIM2_PCIE_UPPORT_FUNC;
                offset = PHOENIX_NIM2_PCIE_UPPORT_2ND_BUS_OFF;
                reg = pci_config_read(bus, dev, func, offset);

                sprintf(file_rmv, "%s0000:%02x:00.0/remove",
                        PHOENIX_NIM2_PCIE_UPPORT_PATH, reg);
                offset = PHOENIX_NIM2_PCIE_UPPORT_2ND_RESETOFF;
                break;

            default:
                printf("%s(): Invalid slot, %d\n", __func__, intf->slot);
                return;
        }

        /* step 1 - remove device */
        /* NIM1: echo 1 > /sys/devices/pci0000:00/0000:00:0f.0/0000:1b:00.0/remove */
        /* NIM2: echo 1 > /sys/devices/pci0000:00/0000:00:0e.0/0000:13:00.0/remove */
        if (file_exist(file_rmv, &size)) {
            sprintf(buf, "echo 1 > %s", file_rmv);
            system(buf);
        }
    
        msleep(1000);
    
        /* step 2 - secoundary bus reset to disable link */
        val = 0x40; /* disable link */
        reg = pci_config_write(bus, dev, func, offset, val);
    
        ngio->wic[intf->slot-FIRST_SLOT].ctrl &=
            ~(NGIO_PCI_RDY | NGIO_UART_TX);
    
        /* delay 3 sec, wait for os to handle remove intr 
         * then reset and power off module
         */
        msleep(3000);
        ngiowic_reset(p);
        ngiowic_i2c_reset(p);
    
        msleep(3000); /* using 3 sec delay is not stable enough */
        ngio->wic[intf->slot-FIRST_SLOT].ctrl &= ~(NGIO_PWR_EN);
    
        /* step 3 - unreset secondary bus */
        val = 0x0; /* unreset, default value is 0*/
        reg = pci_config_write(bus, dev, func, offset, val);
    
        msleep(3000); /* using 3 sec delay is not stable enough */
    } 

    if ((NVRAM)->diagflag & D_VERBOSE) {
        print_offset_val((char *)__FUNCTION__, dash_fpga,
                         (unsigned long)&ngio->wic[intf->slot-FIRST_SLOT].ctrl,
                         0, 0);
    }
    msleep(ENABLE_DELAY);
}

/*-------------------------------------------------------------------
 *
 * Function : ngiowic_enable_intr
 * Description: enable wic intr
 * INPUT:  dev -- slot number starting with 1; intr_type -- type of intr
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
void
ngiowic_enable_intr (int dev, int intr_type)
     
{
    ngio_t *ngio = (ngio_t *)(dash_fpga +  NGIO_BASE);
    dev = ngio_get_dev(dev, (unsigned char *)__FUNCTION__);

    ngio->wic[dev].intr |= intr_type;

}

/*-------------------------------------------------------------------
 *
 * Function : ngiowic_disable intr
 * Description: disable wic intr
 * INPUT:  dev -- slot number starting with 1; intr_type -- type of intr
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
void
ngiowic_disable_intr (int dev, int intr_type)
{
    ngio_t *ngio = (ngio_t *)(dash_fpga +  NGIO_BASE);
    dev = ngio_get_dev(dev, (unsigned char *)__FUNCTION__);
    ngio->wic[dev].intr &= ~intr_type;

}

/*-------------------------------------------------------------------
 *
 * Function : ngiowic_enable_uart
 * Description: enable wic uart
 * INPUT:  p -- pointer to struct ngio_intf_t 
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
int
ngiowic_enable_uart (void *p)
{
    struct ngio_intf_t *intf = (struct ngio_intf_t *)p;
    ngio_t *ngio = (ngio_t *)(dash_fpga +  NGIO_BASE);
    assert(intf->slot);

    ngio->wic[intf->slot-FIRST_SLOT].ctrl |= NGIO_UART_TX;

    return (OK);
}

/*-------------------------------------------------------------------
 *
 * Function : ngiowic_disable_uart
 * Description: disale wic uart
 * INPUT:  p -- pointer to struct ngio_intf_t 
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
int
ngiowic_disable_uart (void *p)
{
    struct ngio_intf_t *intf = (struct ngio_intf_t *)p;
    ngio_t *ngio = (ngio_t *)(dash_fpga +  NGIO_BASE);
    assert(intf->slot);

    ngio->wic[intf->slot-FIRST_SLOT].ctrl &= ~NGIO_UART_TX;

    return (OK);
}

/*-------------------------------------------------------------------
 *
 * Function : ngiovm_i2c_reset
 * Description: reset i2c on vm
 * INPUT:  p -- pointer to struct ngio_intf_t 
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
int
ngiovm_i2c_reset (void *p)
{
    return 0;
}

/*-------------------------------------------------------------------
 *
 * Function : ngiovm_i2c_unrset
 * Description: unreset i2c on vm
 * INPUT:  p -- pointer to struct ngio_intf_t 
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
int
ngiovm_i2c_unreset (void *p)
{
    return 0;
}

/*-------------------------------------------------------------------
 *
 * Function : ngiovm_reset
 * Description: rset vm
 * INPUT:  p -- pointer to struct ngio_intf_t 
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
int
ngiovm_reset (void *p)
{
    return 0;
}
/*-------------------------------------------------------------------
 *
 * Function : ngiovm_unreset 
 * Description: unreset vm
 * INPUT:  p -- pointer to struct ngio_intf_t 
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
int
ngiovm_unreset (void *p)
{
    return 0;
}

/*-------------------------------------------------------------------
 *
 * Function : ngiovm_present 
 * Description: check if VM is present
 * INPUT:  p -- pointer to struct ngio_intf_t 
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
int
ngiovm_present (void *p)
{
    return 0;
}

/*-------------------------------------------------------------------
 *
 * Function : ngiostat_enable_intr
 * Description: enable sata hard drive intr
 * INPUT:  dev -- slot number starting with 1
 *         intr type -- type of interupt
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
void
ngiosata_enable_intr (int dev, int intr_type)
{
    ngio_t *ngio = (ngio_t *)(dash_fpga +  NGIO_BASE);
    
    ngio->sata_intr |= intr_type;

}

/*-------------------------------------------------------------------
 *
 * Function : ngiosata_disable_intrn
 * Description: disable sata inter
 * INPUT:  dev -- slot number starting with 1
 *         intr type -- type of interupt
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
void
ngiosata_disable_intr (int dev, int intr_type)
{
    ngio_t *ngio = (ngio_t *)(dash_fpga +  NGIO_BASE);
    
    ngio->sata_intr &= ~intr_type;

}

/*-------------------------------------------------------------------
 *
 * Function : ngiovm_present 
 * Description: do nothing. vm module is always on.
 * INPUT:  p -- pointer to struct ngio_intf_t 
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
int
ngiovm_enable (void *p)
{
    return (0);
}

/*-------------------------------------------------------------------
 *
 * Function : ngiovm_disable
 * Description: do nothing. vm module is always on.
 * INPUT:  p -- pointer to struct ngio_intf_t 
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
void
ngiovm_disable (void *p)
{

}

/*-------------------------------------------------------------------
 *
 * Function : ngiovm_enable_uart
 * Description: do nothing uart is always on.
 * INPUT:  p -- pointer to struct ngio_intf_t 
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
int
ngiovm_enable_uart (void *p)
{
    return 0;
}
/*-------------------------------------------------------------------
 *
 * Function : ngiodisable_uart
 * Description: do nothing. vm uart is always on.
 * INPUT:  p -- pointer to struct ngio_intf_t 
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
int
ngiovm_disable_uart (void *p)
{
    return 0;
}

/*-------------------------------------------------------------------
 *
 * Function : ngiodc_present
 * Description: check if daughter  is present
 * INPUT:  p -- pointer to struct ngio_intf_t 
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
int
ngiodc_present (void *intf)
{
    struct ngio_intf_t *dc = (struct ngio_intf_t *)intf;
    struct ngio_intf_t *parent;
    unsigned char data;

    assert(dc);
    assert(dc->pc);
    assert(dc->pc->pca);
    
    parent = dc->pc;
    assert(parent->pca);
    
    if (io_port_8bit_i2c_read(parent->pca, 0, &data, 0)) {
        return ERROR;
    }

    if (data & 0x1) {
        return 0;
    }
    return 1;
}

/*-------------------------------------------------------------------
 *
 * Function : ngiodc_reset
 * Description: reset daughter card
 * INPUT:  p -- pointer to struct ngio_intf_t 
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
int
ngiodc_reset (void *p)
{
    struct ngio_intf_t *dc = (struct ngio_intf_t *)p;
    struct ngio_intf_t *parent;
    unsigned char data;

    assert(dc);
    assert(dc->pc);
    assert(dc->pc->pca);
    
    parent = dc->pc;

    if (io_port_8bit_i2c_read(parent->pca, CONFIGURATION_REG, &data, 0)) {
        printf("unable to read from config reg of daughter card expander\n");
        return ERROR;
    }

    /* configure as output pin */
    data &= ~DB_RESET_L;
    if (io_port_8bit_i2c_write(parent->pca, CONFIGURATION_REG, &data)) {
        printf("unable to write to config reg of daughter card expander\n");
        return ERROR;
    }

    if (io_port_8bit_i2c_read(parent->pca, OUTPUT_PORT_REG, &data, 0)) {
        printf("unable to read from output port reg of daughter card expnder\n");
        return ERROR;
    }
    
    data &= ~(DB_RESET_L); 

    if (io_port_8bit_i2c_write(parent->pca, OUTPUT_PORT_REG, &data)) {
        printf("unable to write to output port reg of daughter card expnder\n");
        return ERROR;
    }
    return PASSED;

}

/*-------------------------------------------------------------------
 *
 * Function : ngiodc_unreset
 * Description: unreset daughter card
 * INPUT:  p -- pointer to struct ngio_intf_t 
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
int
ngiodc_unreset (void *p)
{
    struct ngio_intf_t *dc = (struct ngio_intf_t *)p;
    struct ngio_intf_t *parent;
    unsigned char data;

    assert(dc);
    assert(dc->pc);
    assert(dc->pc->pca);

    parent = dc->pc;

    if (io_port_8bit_i2c_read(parent->pca, CONFIGURATION_REG, &data, 0)) {
        printf("fail reading from config reg of daughter card expander\n");
        return ERROR;
    }

    /* configure as output pin */
    data &= ~DB_RESET_L;
    if (io_port_8bit_i2c_write(parent->pca, CONFIGURATION_REG, &data)) {
        printf("fail writing to config reg of daughter card expander\n");
        return ERROR;
    }
    if (io_port_8bit_i2c_read(parent->pca, OUTPUT_PORT_REG, &data, 0)) {
        printf("fail reading from output port reg of daughter card expander\n");
        return ERROR;
    }

    data |= DB_RESET_L;

    if (io_port_8bit_i2c_write(parent->pca, OUTPUT_PORT_REG, &data)) {
        printf("fail writing to output port reg of daughter card expander\n");
        return ERROR;
    }
    return PASSED;

}

/*-------------------------------------------------------------------
 *
 * Function : ngiodc_i2c_rset
 * Description: do nothing. can't reset i2c on daughter card
 * INPUT:  p -- pointer to struct ngio_intf_t 
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
int
ngiodc_i2c_reset (void *p)
{
    assert(!"not supported");
    return ERROR;
}

/*-------------------------------------------------------------------
 *
 * Function : ngiodc_i2c_unrset
 * Description: do nothing. can't unreset i2c on daughter card
 * INPUT:  p -- pointer to struct ngio_intf_t 
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
int
ngiodc_i2c_unreset (void *p)
{
    printf("FIX ME: %s %d\n", __FILE__, __LINE__);
    assert(!"not supported");
    return ERROR;
}

/*-------------------------------------------------------------------
 *
 * Function : ngiodc_enable
 * Description: do nothing. daughter card is alwasy on
 * INPUT:  p -- pointer to struct ngio_intf_t 
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
int
ngiodc_enable (void *p)
{
    assert(!"not supported");
    return ERROR;
}

/*-------------------------------------------------------------------
 *
 * Function : ngiodc_disable
 * Description: do nothing. daughter card is alwasy on
 * INPUT:  p -- pointer to struct ngio_intf_t 
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
void
ngiodc_disable (void *p)
{
    ngiodc_reset(p);
}

/*-------------------------------------------------------------------
 *
 * Function : ngiodc_enable_intr
 * Description: do nothing
 * INPUT:  dev -- slot number starting with 1
 *         intr type -- type of interupt
 * INPUT:  p -- pointer to struct ngio_intf_t 
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
void
ngiodc_enable_intr (int dev, int intr_type)
{
    assert(!"not supported");

}

/*-------------------------------------------------------------------
 *
 * Function : ngiodc_disable_intr
 * Description: disable daughter card intr
 * INPUT:  dev -- slot number starting with 1
 *         intr type -- type of interupt
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
void
ngiodc_disable_intr (int dev, int intr_type)
{
    assert(!"not supported");
}

/*-------------------------------------------------------------------
 *
 * Function : ngiodc_enalbe_uart
 * Description: enale duaghter card uart
 * INPUT:  dev -- slot number starting with 1
 *         intr type -- type of interupt
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
int
ngiodc_enable_uart (void *p)
{
    struct ngio_intf_t *dc = (struct ngio_intf_t *)p;
    struct ngio_intf_t *parent;
    unsigned char data;
    
    assert(dc);
    assert(dc->pc);
    assert(dc->pc->pca);
    
    parent = dc->pc;
    assert(parent->pca);
    if (io_port_8bit_i2c_read(parent->pca, OUTPUT_PORT_REG, &data, 0)) {
        printf("fail to read from output port reg of daughter card expander\n");
        return ERROR;
    }
    data &= ~0x10;
    if (io_port_8bit_i2c_write(parent->pca, OUTPUT_PORT_REG, &data)) {
        printf("fail to write to outputport reg of daughter card expander\n");
        return ERROR;
    }

    return (OK);
}

/*-------------------------------------------------------------------
 *
 * Function : ngiodisable_uart
 * Description: dsiable daughet card uart
 * INPUT:  p -- pointer to struct ngio_intf_t 
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
int
ngiodc_disable_uart (void *p)
{
    struct ngio_intf_t *dc = (struct ngio_intf_t *)p;
    struct ngio_intf_t *parent;
    unsigned char data;
    
    assert(dc);
    assert(dc->pc);
    assert(dc->pc->pca);
    
    parent = dc->pc;
    assert(parent->pca);

    if (io_port_8bit_i2c_read(parent->pca, OUTPUT_PORT_REG, &data, 0)) {
        printf("fail to read from outputport reg of daughter card expander\n");
        return ERROR;
    }
    data |= 0x10;
    if (io_port_8bit_i2c_write(parent->pca, OUTPUT_PORT_REG, &data)) {
        printf("fail to write to outputport reg of daughter card expander\n");
        return ERROR;
    }

    return (OK);
}

/*-------------------------------------------------------------------
 *
 * Function : clr_oir_intr
 * Description: clear OIR inter
 * INPUT:  io -- pointer to struct ng_t ;
 *         buf -- pointer to message
 *         type -- type of module
 *         slot_num -- slot number
 * OUTPUT: NONE
 *         
 * -------------------------------------------------------------------
*/
static void
clr_oir_intr (ng_t *io, char *buf, int type, int slot_num)
{
    *buf = '\0';
    int sts;
    char str[32];

    if (io->ctrl & NGIO_FLT_INTR) {
        sts = io->ctrl ;
        io->ctrl |= NGIO_FLT_INTR;
        sprintf(buf, "pwr fault ");
        switch (type) {
            case SM_MODULE:
                sprintf(str, "SM%d", slot_num);
                break;
            case WIC_MODULE:
                sprintf(str, "WIC%d", slot_num);
                break;
            default:
                *str = '\0';
                break;
        }
        printf("***power fault %s; ngio status @%#x=%#x***\n", str,
               (int)((unsigned long)&io->ctrl - (unsigned long)dash_fpga), 
               sts);
    }
    
    if (io->ctrl & NGIO_RMV_INTR) {
        io->ctrl |= NGIO_RMV_INTR;
        slot_clear_cookie_id(type, slot_num);
        sprintf(buf, "removal");
    }
    
    if (io->ctrl & NGIO_INS_INTR) {
        io->ctrl |= NGIO_INS_INTR;
        slot_clear_cookie_id(type, slot_num);
        sprintf(buf, "insertion");
    }
    
    return;
}

/*-------------------------------------------------------------------
 *
 * Function : oir_sm1_intr_hndlr
 * Description: intr hndlr for sm1
 * INPUT:  irq - irq number; p -- pointer to struct ngio_intf_t 
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
void
oir_sm1_intr_hndlr (int irq, void *p)
{
}

/*-------------------------------------------------------------------
 *
 * Function : oir_sm2_intr_hndlr
 * Description: intr hndlr for sm2
 * INPUT:  irq - irq number; p -- pointer to struct ngio_intf_t 
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
void
oir_sm2_intr_hndlr (int irq, void *p)
{
}

/*-------------------------------------------------------------------
 *
 * Function : oir_sm3_intr_hndlr
 * Description: intr hndlr for sm3
 * INPUT:  irq - irq number; p -- pointer to struct ngio_intf_t
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
void
oir_sm3_intr_hndlr (int irq, void *p)
{
}

/*-------------------------------------------------------------------
 *
 * Function : oir_sm4_intr_hndlr
 * Description: intr hndlr for sm2
 * INPUT:  irq - irq number; p -- pointer to struct ngio_intf_t
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
void
oir_sm4_intr_hndlr (int irq, void *p)
{
}


/*-------------------------------------------------------------------
 *
 * Function : oir_wic1_intr_hndlr
 * Description: intr hndlr for wic1
 * INPUT:  irq - irq number; p -- pointer to struct ngio_intf_t 
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
void
oir_wic1_intr_hndlr (int irq, void *p)
{
    char buf[48];
    ngio_t *ngio = (ngio_t *)(dash_fpga +  NGIO_BASE);
    ng_t *io;

    io = (ng_t *)&ngio->wic[0];
    clr_oir_intr(io, buf, WIC_MODULE, 1);
    if (*buf && ((NVRAM)->diagflag & D_VERBOSE)) {
        printf("\n\n****NGIOWIC1 %s OIR\n\n", buf);
    }
}

/*-------------------------------------------------------------------
 *
 * Function : oir_wic2_intr_hndlr
 * Description: intr hndlr for wic2
 * INPUT:  irq - irq number; p -- pointer to struct ngio_intf_t 
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
void
oir_wic2_intr_hndlr (int irq, void *p)
{
    char buf[48];
    ngio_t *ngio = (ngio_t *)(dash_fpga +  NGIO_BASE);
    ng_t *io;

    io = (ng_t *)&ngio->wic[1];
    clr_oir_intr(io, buf, WIC_MODULE, 2);
    if (*buf && ((NVRAM)->diagflag & D_VERBOSE)) {
        printf("\n\n****NGIOWIC1 %s OIR\n\n", buf);
    }
}

/*-------------------------------------------------------------------
 *
 * Function : oir_wic3_intr_hndlr
 * Description: intr hndlr for wic3
 * INPUT:  irq - irq number; p -- pointer to struct ngio_intf_t 
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
void
oir_wic3_intr_hndlr (int irq, void *p)
{
}

/*-------------------------------------------------------------------
 *
 * Function : oir_sata_intr_hndlr
 * Description: intr hndlr for sata
 * INPUT:  irq - irq number; p -- pointer to struct ngio_intf_t 
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
void
oir_sata_intr_hndlr (int irq, void *p)
{

    ngio_t *ngio = (ngio_t *)(dash_fpga +  NGIO_BASE);

    if (ngio->sata_ctrl & NGIO_SATA1_RMV) {
        ngio->sata_ctrl |= NGIO_SATA1_RMV;
        printf("\n\n****SATA1 Removal OIR detected.\n\n");
    }

    if (ngio->sata_ctrl & NGIO_SATA1_INS) {
        ngio->sata_ctrl |= NGIO_SATA1_INS;
        printf("\n\n****SATA1 Insertino OIR detected.\n\n");

    }

    if (ngio->sata_ctrl & NGIO_SATA2_RMV) {
        ngio->sata_ctrl |= NGIO_SATA2_RMV;
        printf("\n\n****SATA2 Removal OIR detected.\n\n");
    }

    if (ngio->sata_ctrl & NGIO_SATA2_INS) {
        ngio->sata_ctrl |= NGIO_SATA2_INS;
        printf("\n\n****SATA2 Insertion OIR detected.\n\n");
    }

}

/*-------------------------------------------------------------------
 *
 * Function : clr_all_oir_intr
 * Description: clear all possible OIR intr
 * INPUT:  irq - irq number; p -- pointer to struct ngio_intf_t 
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
void
clr_all_oir_intr (void)
{
    char buf[48];
    ngio_t *ngio = (ngio_t *)(dash_fpga +  NGIO_BASE);
    ng_t *io;

    io = (ng_t *)&ngio->wic[0];
    clr_oir_intr(io, buf, WIC_MODULE, 1);
    io = (ng_t *)&ngio->wic[1];
    clr_oir_intr(io, buf, WIC_MODULE, 2);

    ngio->sata_ctrl |= NGIO_SATA1_INS | NGIO_SATA2_INS |
        NGIO_SATA1_RMV | NGIO_SATA1_RMV;
}

/*-------------------------------------------------------------------
 *
 * Function: get_platform_netclk_ptpconfig_base
 * Description: Network Clock and PTP Config address
 * 0x10100 base for above registers
 * Input: None
 *
 * Output: address
 *
 *-------------------------------------------------------------------
 */
unsigned long
get_platform_net_clk_ptp_conf_base (void)
{
    unsigned long addr = 0;
    assert(dash_fpga);
    addr = ((unsigned long)dash_fpga) + NET_CLK_PTP_CONF_REG_OFF;
    return addr;

}

void
ngiowic_pci_rdy (void *p, int on)
{
    struct ngio_intf_t *intf = (struct ngio_intf_t *)p;
    
    ngio_t *ngio = (ngio_t *)(dash_fpga +  NGIO_BASE);
    
    if (on) {
        ngio->wic[intf->slot - FIRST_SLOT].ctrl |= NGIO_SRC_SEL;
        ngio->wic[intf->slot - FIRST_SLOT].ctrl |= NGIO_PCI_RDY;
    } else {
         ngio->wic[intf->slot - FIRST_SLOT].ctrl &= ~NGIO_PCI_RDY;
    }
}

void
ngiosm_pci_rdy (void *p, int on)
{
    struct ngio_intf_t *intf = (struct ngio_intf_t *)p;
    
    ngio_t *ngio = (ngio_t *)(dash_fpga +  NGIO_BASE);
    
    if (on) {
        ngio->sm[intf->slot - FIRST_SLOT].ctrl |= NGIO_SRC_SEL;
        ngio->sm[intf->slot - FIRST_SLOT].ctrl |= NGIO_PCI_RDY;
    } else {
         ngio->sm[intf->slot - FIRST_SLOT].ctrl &= ~NGIO_PCI_RDY;
    }
}

int
ngio_sync_out_enable (void *p, int mask)
{
    struct ngio_intf_t *intf = (struct ngio_intf_t *)p;
    ntclk_t *ntclk;
    volatile unsigned int *reg;
    int slot = intf->slot-FIRST_SLOT;
    
    ntclk = (ntclk_t *)get_platform_net_clk_ptp_conf_base();

    switch (intf->mod_type) {                                                                                 
    case WIC_MODULE:
        reg = (volatile unsigned int *)&(ntclk->sync_wic[slot]);
        break;
    case SM_MODULE:
    case SM_DAUGHTER_CARD:
        reg = (volatile unsigned int *)&(ntclk->sync_sm[slot]);
        break;
    case VM_MODULE:
        reg = (volatile unsigned int *)&(ntclk->sync_vm);
        break;
    }     
    *reg |= mask;

    print_offset_val((char *)__FUNCTION__, dash_fpga,
                     (unsigned long)&ntclk->sync_wic[2],
                     0, 0);
    
    return(PASSED);
}

int
ngio_sync_out_disable (void *p, int mask)
{
    struct ngio_intf_t *intf = (struct ngio_intf_t *)p;
    ntclk_t *ntclk;
    volatile unsigned int *reg;
    int slot = intf->slot-FIRST_SLOT;
    
    ntclk = (ntclk_t *)get_platform_net_clk_ptp_conf_base();

    reg = (volatile unsigned int *)&(ntclk->sync_wic[slot]);
    *reg &= ~mask;

    print_offset_val((char *)__FUNCTION__, dash_fpga,
                     (unsigned long)&ntclk->sync_wic[1],
                     0, 0);

    
    return(PASSED);
}

/* 
 * Function: ngio_plx_intr_mask
 * 
 * This function turn on and off the interrupt mask bit in the PCIe
 * bridge on the bus connected to the NGIO slot. It is only applicable
 * on Victory platform with PLX PCIe switch.
 */
void
ngio_plx_intr_mask(void *p, int enable)
{
}

/*
 * Function: get_ngio_pcie_dev_bus_num
 * description: based on PCIe bus to return ngio device bus number. 
 * input : ngio_if->slot
 *         ngio_if->mod_type 
 * output: ngio_pcie_bus_no - NGIO PCIe bus number.
 * Note  : dev_no is defined on platforms HFS.
 */
int
get_ngio_pcie_dev_bus_num (uint mod_type, uint slot)
{
    uint dev_no = 0;

    if (mod_type == WIC_MODULE) {
        switch (slot) {
            case NGWIC1_SLOT:
                dev_no = 10;
                break;

            case NGWIC2_SLOT:
                dev_no = 8;
                break;

            default:
                printf("%s() - WIC_MODULE - Invalid slot\n", __func__);
                return (FAILED);
        }
    } else {
        printf("%s() - Invalid module type 0x%x\n", __func__, mod_type);
        return (FAILED);
    }

    return (dev_no);
}

/*
 * Function: host_ngio_10gkr_capability
 * Find out the platform's capability to support 10G-KR on the
 * NGIO interface. Retune a bit mask value by using
 * NGIO_GE0_BITMASK, NGIO_GE1_BITMASK, etc.
 *
 * input : ngio_if->mod_type 
 *         ngio_if->slot
 *         
 * return: Bit mask to indicate which NGIO GE port is 10G-KR
 *         capable. For example:
 *         0x1 mean GE0 is supported
 *         0x3 mean GE1 and GE0 are supported
 */
uint
host_ngio_10gkr_capability (uint mod_type, uint slot)
{
    uint result = 0;

    return(result);
}

/*
 * Function: cfg_host_10gkr_port
 * Configure the host 10GKR port to be 1G or 10GKR
 *
 * input : mod_type - NGIO module type 
 *         slot - NGIO slot number
 *         ngio_port - NGIO module GE0 or GE1
 *         en_10gkr - 1: set 10G, 0: set 1G
 *         
 * return: 
 */
int
cfg_host_10gkr_port(uint mod_type, uint slot, int ngio_port, int en_10gkr)
{
    return 0;
}

int ngio_cfg_eth_port_speed(uint mod_type, uint slot,
                            const ngio_eth_speed_t *new_speed, ngio_eth_speed_t *old_speed)
    __attribute__((weak, alias("__ngio_cfg_eth_port_speed")));
int __ngio_cfg_eth_port_speed(uint mod_type, uint slot,
                              const ngio_eth_speed_t *new_speed, ngio_eth_speed_t *old_speed)
{
    return (FALSE);
}
