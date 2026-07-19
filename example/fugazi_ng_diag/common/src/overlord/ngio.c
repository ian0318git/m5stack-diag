/* $Id: ngio.c,v 1.72 2021/08/04 04:48:24 xiaolaya Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/ngio.c,v $
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
#include "dash_fpga.h"
#include "i2c_api.h"
#include "linux_api.h"
#include "platform_i2c.h"
#include "linux_usb_test.h"
#include "common.h"
#include "slot.h"
#include "ngio.h"
#include "pca.h"
#include "nvmonvars.h"
#include "cookie_4.h"
#include "platform_cookie.h"
#include "cross_platform.h"
#include "plat_defs.h"
#include "linux_pciutils.h"
#include "platform_slot.h"  /* requires slot.h */
#include "bcm_gesw_defs.h"
#include "platform_pcie_clk.h"

#define WAIT_NWK_DOP_UP 8000
#define ENABLE_DELAY 600
#define I2C_UNRESET_DELAY 50
#define PCI_HOTPLUG_BITS         ((1 << 3) + (1 << 4) + (1 << 5))
extern uint pcie_config_read(uint32_t, uint32_t, uint16_t, uint, uint);
extern void pcie_config_write(uint32_t, uint32_t, uint16_t, uint, uint, uint32_t);
extern uint32_t cpu_ge_status(int);
extern int netstat_main(char *);

uint8_t __attribute__((weak)) get_adapter_pcie_sub_bus_num(int slot)
{
    return (FALSE);
}

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


int
ngio_ref_clk (struct ngio_intf_t *intf, int enable)
{
    int smclk_offset[] = {-1,2,2};
    int smclk_bit[] = {-1,3,2};
    int wicclk_offset[] = {-1,2,2,1};
    int wicclk_bit[] = {-1,1,0,7};
    int offset = -1;
    int bit = -1;
    unsigned char tmp[PCIE_CLK_BUF_SIZE];
    unsigned char val;
    int ret = FAILED;

    /* Per CRDC HW mentioned, no need to turn on 
     * since it is default on 
     */ 
    if (is_ntpn_machines() || is_vg450()) {
        return (PASSED);
    }

    switch (intf->mod_type) {
    case SM_DAUGHTER_CARD:
    case SM_MODULE:
        offset = smclk_offset[intf->slot];
        bit = smclk_bit[intf->slot];
        break;
    case WIC_MODULE:
        offset = wicclk_offset[intf->slot];
        bit = wicclk_bit[intf->slot];
        break;
       
    }

    if ((bit < 0) || (offset < 0)) {
        printf("module type %d, slot %d\n",
               intf->mod_type, intf->slot);
        assert(!"invalid slot or module type");
        return(FAILED);
    }

    if (read_pcie_clk_reg(tmp) == FAILED) {
        printf("ngio_ref_clk : read_pcie_clk_reg failed\n");
        return(FAILED);
    }

    val = tmp[offset];
    
    ret = FAILED;
    
    if (enable) {
        val |= (1<<bit);
    } else {
        val &= ~(1<<bit);
    }

    ret = write_pcie_clk_reg(offset, val);
    if (ret == FAILED)
        printf("ngio_ref_clk : write_pcie_clk_reg failed\n");

    return(ret);
     
}


/*-------------------------------------------------------------------
 *
 * Function : ngiosm_present 
 * Description: check if SM is present
 * INPUT:  p -- pointer to struct ngio_intf_t 
 * OUTPUT: TRUE if SM is present
 * -------------------------------------------------------------------
*/
int
ngiosm_present (void *p)
{
    int sts;
    
    struct ngio_intf_t *intf = (struct ngio_intf_t *)p;
    
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
    ngio_t *ngio = (ngio_t *)(dash_fpga +  NGIO_BASE);
    struct ngio_intf_t *intf = (struct ngio_intf_t *)p;

    assert(intf->slot);

    if (ngio->sm[intf->slot-FIRST_SLOT].ctrl & NGIO_I2C_RESET) 
    { 
        return (FALSE);
    } else {
        return (TRUE);
    }
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
    ngio_t *ngio = (ngio_t *)(dash_fpga +  NGIO_BASE);
    struct ngio_intf_t *intf = (struct ngio_intf_t *)p;

    assert(intf->slot);

    ngio->sm[intf->slot-FIRST_SLOT].ctrl |= NGIO_RESET;

    return (PASSED);
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
    struct ngio_intf_t *intf = (struct ngio_intf_t *)p;
    
    assert(intf->slot);
    ngio_t *ngio = (ngio_t *)(dash_fpga +  NGIO_BASE);

#ifdef LEGACY
    unsigned int slot, mode, mask, gen; 

    slot = intf->slot;
    switch (slot) {
    case NGSM1_SLOT:
        mode = NGSM1_LEGACY_MODE; 
        mask = ((NGSM1_LEGACY_MODE >> 1) | (NGSM1_LEGACY_MODE));
    break; 
    case NGSM2_SLOT:
        mode = NGSM2_LEGACY_MODE; 
        mask = ((NGSM2_LEGACY_MODE >> 1) | (NGSM2_LEGACY_MODE));
    break; 
    case NGSM3_SLOT:
        mode = NGSM3_LEGACY_MODE; 
        mask = ((NGSM3_LEGACY_MODE >> 1) | (NGSM3_LEGACY_MODE));
    break; 
    case NGSM4_SLOT:
        mode = NGSM4_LEGACY_MODE; 
        mask = ((NGSM4_LEGACY_MODE >> 1) | (NGSM4_LEGACY_MODE));
    break; 
    }

    ngio->general |= mode; 
#endif
    ngio->sm[intf->slot-FIRST_SLOT].ctrl &= ~(NGIO_RESET |
                                              NGIO_PCI_RDY);

    ngio->sm[intf->slot-FIRST_SLOT].ctrl |= NGIO_SRC_SEL;

    /* This code only applicable on Victory platform with
     * PLX PCIe switch.
     */
    if (is_ngio_use_pcie(p)) {
        if ((NGSM_NWK48 == intf->id ||
             NGSM_NWK24 == intf->id) &&
            is_plx_wrapper()) {
            /* This code only applicable on Nightwatch
             * Make sure DopplerG is ready before trigger hot plug
             */
            msleep(WAIT_NWK_DOP_UP);
        }
	ngio_plx_intr_mask(p, 1);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        print_offset_val((char *)__FUNCTION__, dash_fpga,
                     (unsigned long)&ngio->sm[intf->slot-FIRST_SLOT].ctrl,
                     0, 0);
    }

#ifdef LEGACY
    gen = ngio->general;
    if (gen != mask) {
        print_offset_val((char *)"general ctrl", dash_fpga,
                     (unsigned long)&ngio->general,
                     0, 0);
        cterr('f', 0, "Did not enter legacy mode.");
        return(FAILED);
    }
#endif
    return (PASSED);
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

    assert(intf->slot);
    ngio_t *ngio = (ngio_t *)(dash_fpga +  NGIO_BASE);

    ngio->sm[intf->slot-FIRST_SLOT].ctrl |= NGIO_I2C_RESET;

    return (PASSED);
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
    uint16_t device_no = 0;

    /* Workaround for Switzer-Manhattan RDT eeprom check failed and pcie init failure:
     * V710 need to add more delay between ngio_pwr_en and ngio_unreset,
     * this necessary delay will be added in switzer_manhattan_test.c,
     * But if ngio_enable be repetitively called,
     * io->ctrl &= ~(NGIO_RESET) will also effect ngio un-reset.
     * To make sure this delay be actually used, skip repetitively do ngio_enable for Manhattan.
     */
    if (io->ctrl & NGIO_PWR_EN) {
        if (intf->id == 0x113f || intf->id == 0x1140)
            return 0;
    }

    /* make sure ctrl reg is back in its default state before
       taking powering module */

    io->ctrl &= ~(NGIO_UART_TX | NGIO_I2C_RESET | NGIO_RESET);

    if (!is_goldbeach() && !is_curie_1ru() && !is_curie_2ru() && !is_vg400()) {
        if (ngio_ref_clk(intf, 1) == FAILED) {
            printf("unable to turn on pcie ref clock for slot %d\n", intf->slot);
            return(-1);
        }
        msleep(20); /* for enable pcie ref clock */
     
        /* Disable PLX downstream Hot Plug port*/
        if (is_plx_wrapper()) {
            uint32_t port0_bus_num, reg_val;
            /* Assign bus number of PLX port 0 for different platform  */
            if (is_utah_plx() || is_juno_plx()) {
                port0_bus_num = get_pcie_bus_num (PLX_PCIE_SW_VID, PLX_PCIE_SW_DID_8618);
            } else if (is_sword()) {
                port0_bus_num = get_pcie_bus_num (PLX_PCIE_SW_VID, PLX_PCIE_SW_DID_8617);
            } else if (is_dagger()) {
                port0_bus_num = get_pcie_bus_num (PLX_PCIE_SW_VID, PLX_PCIE_SW_DID_8604);
            }
            /* access ngio pcie swtich bus instead pcie swtich itself */
            /* so the bus number should be plus 1 */
            port0_bus_num += 1; 
            fflush(stdout);
            /* PLX */
            fflush(stdout);
            if ((intf->mod_type == SM_MODULE) || (intf->mod_type == SM_DAUGHTER_CARD)) {
                device_no = get_sm_device_no(intf->slot);
            } else if (intf->mod_type == WIC_MODULE) { 
                device_no = get_wic_device_no(intf->slot);
            }
            reg_val = pcie_config_read(0, port0_bus_num, device_no, 0, 0x80);
            reg_val &= PCI_HOTPLUG_BITS;
            pcie_config_write(0, port0_bus_num, device_no, 0, 0x80, reg_val);
        }
    } /* is_goldbeach */  /* is_curie_1ru */
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
    char cmd[64];

    assert(intf->slot);

    /* Disable the electrical idle detect*/
    if (is_overlord()) {
        printf("\nDisable the electrical idle detect\n");
        if ((intf->mod_type == SM_MODULE) || (intf->mod_type == SM_DAUGHTER_CARD)) {
	        switch (intf->slot) {
            case NGSM1_SLOT:
                // disable NGSM1 PCIe port
                sprintf(cmd, "setpci -s 3:c.0 510.l=200");
                system(cmd);
            break;
            case NGSM2_SLOT:
                // disable NGSM2 PCIe port
                sprintf(cmd, "setpci -s 3:e.0 510.l=200");
                system(cmd);
            break;
            default: 
            break;
            }
        }
    }
    /* This code only applicable on Victory platform with
     * PLX PCIe switch.
     */
    ngio_plx_intr_mask(p, 0);
    if (NGSM_NWK48 == intf->id ||
        NGSM_NWK24 == intf->id ) {
        /* Disable PCIE hotplug before DopplerG is read */
        ngiosm_pci_rdy(intf, FALSE);
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
    char cmd[64], file_rmv[128], buf[128];
    unsigned char bus, dev, func, offset, val, reg; 
    size_t size = 0;
    const char *path;
    
    assert(intf->slot);
    /* Diable PCIe downstream port */
    if (is_plx_wrapper()) {
        /* PLX */
        if (is_utah()) {
            if ((intf->mod_type == SM_MODULE) || (intf->mod_type == SM_DAUGHTER_CARD)) {
	            switch (intf->slot) {
                case NGSM1_SLOT:
                    // disable NGSM1 PCIe port
                    sprintf(cmd, "setpci -s 4:0.0 234.l=1");
                    system(cmd);
                break;
                case NGSM2_SLOT:
                    // disable NGSM2 PCIe port
                    sprintf(cmd, "setpci -s 4:0.0 230.l=2");
                    system(cmd);
                break;
                default: 
                break;
                }
            }
        } else if (is_sword()) {
            if ((intf->mod_type == SM_MODULE) || (intf->mod_type == SM_DAUGHTER_CARD)) {
                switch (intf->slot) {
                /* only one SM */
                case NGIOSM1:
                    // disable Sword NIM1 PCIe port
                    sprintf(cmd, "setpci -s 4:0.0 234.l=2");
                    system(cmd);
                break;
                }
            }
        }
    }
    /* Per HW suggestion,
     * pci ready shoud be pulled down to generate
     * interrupt to kernel via pcie switch.
     * we don't want to change o2/usd code,
     * so we using a if statement to isolate on NTPN machines
     */
    if (is_ntpn_machines() || is_vg450()) {
        if (intf->slot == MAX_SM_NEPTUNE) { /* SM4 */
            /* special ops for SM4 which has no interrupt pin for supporting 
             * hot remove. */
            bus = NEPTUNE_SM4_PCIE_UPPORT_BUS; 
            dev = NEPTUNE_SM4_PCIE_UPPORT_DEV;
            func = NEPTUNE_SM4_PCIE_UPPORT_FUNC; 
            offset = NEPTUNE_SM4_PCIE_UPPORT_2ND_BUS_OFF; 
            reg = pci_config_read(bus, dev, func, offset); 
            /* step 1 - remove device */
            /* echo 1 > /sys/devices/pci0000:00/0000:00:03.1/0000:0f:00.0/remove */
            sprintf(buf, "echo 1 > %s0000:%02x:00.0/remove", 
                         NEPTUNE_SM4_PCIE_UPPORT_PATH, reg); 
            system(buf); 

            /* step 2 - secoundary bus reset to disable link */
            offset = NEPTUNE_SM4_PCIE_UPPORT_2ND_RESETOFF; 
            val = 0x40; /* disable link */
            reg = pci_config_write(bus, dev, func, offset, val); 
        }

        ngio->sm[intf->slot-FIRST_SLOT].ctrl &=
            ~(NGIO_PCI_RDY | NGIO_UART_TX);

       /* delay 3 sec, wait for os to handle remove intr 
        * then reset and power off module
        */
        msleep(3000);
        ngiosm_reset(p);
        ngiosm_i2c_reset(p);

        msleep(3000); /* using 1 sec delay is not stable enough */
        ngio->sm[intf->slot-FIRST_SLOT].ctrl &= ~(NGIO_PWR_EN);

        /* step 3 - unreset secondary bus */
        if (intf->slot == MAX_SM_NEPTUNE) { /* SM4 */
            val = 0x0; /* unreset, default value is 0*/
            reg = pci_config_write(bus, dev, func, offset, val); 
        }
    } else if (is_curie_1ru()) {
        /* Curie 1RU has SM1 only */
        /* special ops for SM4 which has no interrupt pin for supporting 
         * hot remove. */
        bus = CURIE_RADIUM_SM1_PCIE_UPPORT_BUS; 
        dev = CURIE_RADIUM_SM1_PCIE_UPPORT_DEV; 
        func = CURIE_RADIUM_SM1_PCIE_UPPORT_FUNC; 
        offset = CURIE_RADIUM_SM1_PCIE_UPPORT_2ND_BUS_OFF; 
        reg = pci_config_read(bus, dev, func, offset); 
        /* step 1 - remove device */
        /* echo 1 > /sys/devices/pci0000:00/0000:00:03.3/0000:10:00.0/remove */
        
        sprintf(file_rmv, "%s0000:%02x:00.0/remove", 
                          CURIE_RADIUM_SM1_PCIE_UPPORT_PATH, reg); 

        if (file_exist(file_rmv, &size)) {
            sprintf(buf, "echo 1 > %s", file_rmv); 
            system(buf); 
        } 

        /* step 2 - secoundary bus reset to disable link */
        offset = CURIE_RADIUM_SM1_PCIE_UPPORT_2ND_RESETOFF; 
        val = 0x40; /* disable link */
        reg = pci_config_write(bus, dev, func, offset, val); 

        ngio->sm[intf->slot-FIRST_SLOT].ctrl &=
            ~(NGIO_PCI_RDY | NGIO_UART_TX);

        /* delay 3 sec, wait for os to handle remove intr 
         * then reset and power off module
         */
        msleep(3000);
        ngiosm_reset(p);
        ngiosm_i2c_reset(p);

        msleep(3000); /* using 1 sec delay is not stable enough */
        ngio->sm[intf->slot-FIRST_SLOT].ctrl &= ~(NGIO_PWR_EN);

        /* step 3 - unreset secondary bus */
        val = 0x0; /* unreset, default value is 0*/
        reg = pci_config_write(bus, dev, func, offset, val); 

        msleep(2000); /* using 1 sec delay is not stable enough */
    } else if (is_curie_2ru()) {
        if (intf->slot == NGIOSM1) { /* SM1 */
            bus = CURIE_URANIUM_SM1_PCIE_UPPORT_BUS;
            dev = CURIE_URANIUM_SM1_PCIE_UPPORT_DEV;
            func = CURIE_URANIUM_SM1_PCIE_UPPORT_FUNC;
            offset = CURIE_URANIUM_SM1_PCIE_UPPORT_2ND_BUS_OFF;
            path = CURIE_URANIUM_SM1_PCIE_UPPORT_PATH;
        } else {                /* SM2 */
            bus = CURIE_URANIUM_SM2_PCIE_UPPORT_BUS;
            dev = CURIE_URANIUM_SM2_PCIE_UPPORT_DEV;
            func = CURIE_URANIUM_SM2_PCIE_UPPORT_FUNC;
            offset = CURIE_URANIUM_SM2_PCIE_UPPORT_2ND_BUS_OFF;
            path = CURIE_URANIUM_SM2_PCIE_UPPORT_PATH;
        }
        reg = pci_config_read(bus, dev, func, offset);
        /* step 1 - remove device */
        /* echo 1 > /sys/devices/pci0000:00/0000:00:03.3/0000:10:00.0/remove */

        sprintf(file_rmv, "%s0000:%02x:00.0/remove", path, reg);

        if (file_exist(file_rmv, &size)) {
            sprintf(buf, "echo 1 > %s", file_rmv);
            system(buf);
        }

        /* step 2 - secoundary bus reset to disable link */
        if (intf->slot == NGIOSM1) { /* SM1 */
            offset = CURIE_URANIUM_SM1_PCIE_UPPORT_2ND_RESETOFF;
        } else {                /* SM2 */
            offset = CURIE_URANIUM_SM2_PCIE_UPPORT_2ND_RESETOFF;
        }

        /* Per debugging and HW suggestion, it is not reasonable to
         * disable link here if SM module is under reset state
         *
         * Otherwise when module is unreset later, unexpected interrupt
         * of 'Link Down' will be rised and cause unreasonable 'Card not present'
         * event, and the current 'Card present' event is postponed,
         * which may result in unexpected behavior.
         *
         * On Nightwatch, the current 'Card present' event is postponed
         * a few seconds by the unexpected 'Card not present' event,
         * and cause possible conflicting of kernel assigning PCIe BAR address
         * and module's diag app assigning address, then system rebooting may
         * happens once wrong address is used during later running tests.
         *
         */
        if (!(ngio->sm[intf->slot-FIRST_SLOT].ctrl & NGIO_RESET)) {
            val = 0x40; /* disable link */
            reg = pci_config_write(bus, dev, func, offset, val);
        } else {
            val = 0;
        }

        ngio->sm[intf->slot-FIRST_SLOT].ctrl &=
            ~(NGIO_PCI_RDY | NGIO_UART_TX);

        /* delay 3 sec, wait for os to handle remove intr
         * then reset and power off module
         */
        msleep(3000);
        ngiosm_reset(p);
        ngiosm_i2c_reset(p);

        msleep(3000); /* using 1 sec delay is not stable enough */
        ngio->sm[intf->slot-FIRST_SLOT].ctrl &= ~(NGIO_PWR_EN);

        if (0 != val) {
            /* step 3 - unreset secondary bus */
            val = 0x0; /* unreset, default value is 0*/
            reg = pci_config_write(bus, dev, func, offset, val);
        }

        msleep(2000); /* using 1 sec delay is not stable enough */
    } else {
        ngiosm_reset(p);
        ngiosm_i2c_reset(p);

        ngio->sm[intf->slot-FIRST_SLOT].ctrl &= ~NGIO_PWR_EN;

        ngio->sm[intf->slot-FIRST_SLOT].ctrl &=
              ~(NGIO_PCI_RDY | NGIO_UART_TX);

    }
    
    if ((NVRAM)->diagflag & D_VERBOSE) {
        print_offset_val((char *)__FUNCTION__, dash_fpga,
                         (unsigned long)&ngio->sm[intf->slot-FIRST_SLOT].ctrl,
                         0, 0);
    }
    msleep(ENABLE_DELAY);    

    /* Recovery PCIe downstream port */
    if (is_overlord()) {
        printf("\nRecovery PCIe downstream port\n");
        if ((intf->mod_type == SM_MODULE) || (intf->mod_type == SM_DAUGHTER_CARD)) {
	        switch (intf->slot) {
            case NGSM1_SLOT:
                // disable NGSM1 PCIe port
                sprintf(cmd, "setpci -s 3:c.0 540.l=80000000");
                system(cmd);
            break;
            case NGSM2_SLOT:
                // disable NGSM2 PCIe port
                sprintf(cmd, "setpci -s 3:e.0 540.l=80000000");
                system(cmd);
            break;
            default: 
            break;
            }
        } 
    }
    if (is_plx_wrapper()) {
        /* PLX */
        if (is_utah()) {
            if ((intf->mod_type == SM_MODULE) || (intf->mod_type == SM_DAUGHTER_CARD)) {
	            switch (intf->slot) {
                case NGSM1_SLOT:
                    // disable NGSM1 PCIe port
                    sprintf(cmd, "setpci -s 4:0.0 234.l=0");
                    system(cmd);
                break;
                case NGSM2_SLOT:
                    // disable NGSM2 PCIe port
                    sprintf(cmd, "setpci -s 4:0.0 230.l=0");
                    system(cmd);
                break;
                default: 
                break;
                }
            }
        } else if (is_sword()) {
            if ((intf->mod_type == SM_MODULE) || (intf->mod_type == SM_DAUGHTER_CARD)) {
                switch (intf->slot) {
                /* only one SM */
                case NGIOSM1:
                    // disable Sword NIM1 PCIe port
                    sprintf(cmd, "setpci -s 4:0.0 234.l=0");
                    system(cmd);
                break;
                }
            }
        }
    }

    if (NGSM_NWK48 == intf->id ||
        NGSM_NWK24 == intf->id) {
        /* For Nightwatch:
         * Remove PCIE device
         */
        if (is_ngio_use_pcie(p)) {
            ngio_plx_intr_mask(p, 1);
        }
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
    ngio_t *ngio = (ngio_t *)(dash_fpga +  NGIO_BASE);
    
    dev = ngio_get_dev(dev, (unsigned char *)__FUNCTION__);

    ngio->sm[dev].intr |= intr_type;

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
    ngio_t *ngio = (ngio_t *)(dash_fpga +  NGIO_BASE);
    dev = ngio_get_dev(dev, (unsigned char *)__FUNCTION__);
    ngio->sm[dev].intr &= ~intr_type;

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
    //    dev = ngio_get_dev(dev, (unsigned char *)__FUNCTION__);
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
    //    dev = ngio_get_dev(dev, (unsigned char *)__FUNCTION__);
    ngio->sm[intf->slot-FIRST_SLOT].ctrl &= ~NGIO_UART_TX;

    return(PASSED);
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
    
    //    dev = ngio_get_dev(dev, (unsigned char *)__FUNCTION__);
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
    
    /* This code only applicable on Victory platform with
     * PLX PCIe switch.
     */
    if (is_ngio_use_pcie(p)) {
	ngio_plx_intr_mask(p, 1);
    }

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
    char cmd[64];
    assert(intf->slot);    

    /* Disable the electrical idle detect */
    if (is_overlord()) {
        printf("\nDisable the electrical idle detect\n");
        if ((intf->mod_type == SM_MODULE) || (intf->mod_type == SM_DAUGHTER_CARD)) {
            switch (intf->slot) {
            case NGSM1_SLOT:
                // disable NGSM1 PCIe port
                sprintf(cmd, "setpci -s 3:c.0 510.l=200");
                system(cmd);
            break;
            case NGSM2_SLOT:
                // disable NGSM2 PCIe port
                sprintf(cmd, "setpci -s 3:e.0 510.l=200");
                system(cmd);
            break;
            default: 
            break;
            }
        } else if (intf->mod_type == WIC_MODULE) { 
            switch(intf->slot) {
            case NGWIC1_SLOT:
                // disable NIM1 PCIe port
                sprintf(cmd, "setpci -s 3:8.0 510.l=200");
                system(cmd);
            break;
            case NGWIC2_SLOT:
                // disable NIM2 PCIe port
                sprintf(cmd, "setpci -s 3:a.0 510.l=200");
                system(cmd);
            break;
            case NGWIC3_SLOT:
                // disable NIM3 PCIe port
                sprintf(cmd, "setpci -s 3:2.0 510.l=200");
                system(cmd);
            break;
            default: 
            break;
            }
        }
    }
    /* This code only applicable on Victory platform with
     * PLX PCIe switch.
     */
    if ((!is_goldbeach()) && (!is_vg400())) {
        ngio_plx_intr_mask(p, 0);
    }
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
    char cmd[64], file_rmv[128], buf[128];
    unsigned char bus, dev, func, offset, val, reg; 
    size_t size = 0;
    const char *path;

    assert(intf->slot);

    /* Diable PCIe downstream port */
    if (is_plx_wrapper()) {
        /* PLX */
        if (is_utah()) {
            if ((intf->mod_type == SM_MODULE) || (intf->mod_type == SM_DAUGHTER_CARD)) {
	            switch (intf->slot) {
                case NGSM1_SLOT:
                    // disable NGSM1 PCIe port
                    sprintf(cmd, "setpci -s 4:0.0 234.l=1");
                    system(cmd);
                break;
                case NGSM2_SLOT:
                    // disable NGSM2 PCIe port
                    sprintf(cmd, "setpci -s 4:0.0 230.l=2");
                    system(cmd);
                break;
                default: 
                break;
                }
            } else if (intf->mod_type == WIC_MODULE) { 
                switch(intf->slot) {
                case NGWIC1_SLOT:
                    // disable NIM1 PCIe port
                    sprintf(cmd, "setpci -s 4:0.0 234.l=2");
                    system(cmd);
                break;
                case NGWIC2_SLOT:
                    // disable NIM2 PCIe port
                    sprintf(cmd, "setpci -s 4:0.0 234.l=20");
                    system(cmd);
                break;
                case NGWIC3_SLOT:
                    // disable NIM3 PCIe port
                    sprintf(cmd, "setpci -s 4:0.0 234.l=40");
                    system(cmd);
                break;
                default: 
                break;
                }
            }
        } else if (is_sword()) {
            if ((intf->mod_type == SM_MODULE) || (intf->mod_type == SM_DAUGHTER_CARD)) {
                switch (intf->slot) {
                /* only one SM */
                case NGIOSM1:
                    // disable Sword NIM1 PCIe port
                    sprintf(cmd, "setpci -s 4:0.0 234.l=2");
                    system(cmd);
                break;
                }
            } else if (intf->mod_type == WIC_MODULE) {
                switch (intf->slot) {
                case NGWIC1_SLOT:
                    // disable Sword NIM1 PCIe port
                    sprintf(cmd, "setpci -s 4:0.0 230.l=2");
                    system(cmd);
                break;
                case NGWIC2_SLOT:
                   // disable Sword NIM2 PCIe port
                    sprintf(cmd, "setpci -s 4:0.0 234.l=2");
                    system(cmd);
                break;
                default: 
                break;
                }
            }
        } else if (is_dagger()) {
            switch (intf->slot) {
            case NGWIC1_SLOT:
                // disable NIM1 PCIe port
                sprintf(cmd, "setpci -s 4:0.0 234.l=1");
                system(cmd);
            break;
            case NGWIC2_SLOT:
                // disable NIM12 PCIe port
                sprintf(cmd, "setpci -s 4:0.0 234.l=4");
                system(cmd);
            break;
            default: 
            break;
            }
        }
    }

    /* Per HW suggestion, 
     * pci ready shoud be pulled down to generate
     * interrupt to kernel via pcie switch.
     * we don't want to change o2/usd code,
     * so we using a if statement to isolate on NTPN machines
     */
    if (is_ntpn_machines() || is_vg450()) {
        ngio->wic[intf->slot-FIRST_SLOT].ctrl &=
            ~(NGIO_PCI_RDY | NGIO_UART_TX);

       /* delay 3 sec, wait for os to handle remove intr 
        * then reset and power off module
        */
        msleep(3000); 
        ngiowic_reset(p);
        ngiowic_i2c_reset(p);
    
        msleep(3000); /* using 1 sec delay is not stable enough */ 
        ngio->wic[intf->slot-FIRST_SLOT].ctrl &= ~(NGIO_PWR_EN);

    } else if (is_curie_1ru()) {
        /* Curie 1RU has NIM1 only */
        /* special ops for SM4 which has no interrupt pin for supporting 
         * hot remove. */
        bus = CURIE_RADIUM_NIM1_PCIE_UPPORT_BUS; 
        dev = CURIE_RADIUM_NIM1_PCIE_UPPORT_DEV; 
        func = CURIE_RADIUM_NIM1_PCIE_UPPORT_FUNC; 
        offset = CURIE_RADIUM_NIM1_PCIE_UPPORT_2ND_BUS_OFF; 
        reg = pci_config_read(bus, dev, func, offset); 
        /* step 1 - remove device */
        /* echo 1 > /sys/devices/pci0000:00/0000:00:03.3/0000:11:00.0/remove */
        sprintf(file_rmv, "%s0000:%02x:00.0/remove",
                     CURIE_RADIUM_NIM1_PCIE_UPPORT_PATH, reg);

        if (file_exist(file_rmv, &size)) {
            sprintf(buf, "echo 1 > %s", file_rmv);
            system(buf);
        }

        /* step 2 - secoundary bus reset to disable link */
        offset = CURIE_RADIUM_NIM1_PCIE_UPPORT_2ND_RESETOFF; 
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

        msleep(3000); /* using 1 sec delay is not stable enough */
        ngio->wic[intf->slot-FIRST_SLOT].ctrl &= ~(NGIO_PWR_EN);

        /* step 3 - unreset secondary bus */
        val = 0x0; /* unreset, default value is 0*/
        reg = pci_config_write(bus, dev, func, offset, val); 

        msleep(2000); /* using 1 sec delay is not stable enough */
    } else if (is_curie_2ru()) {
        if (intf->slot == NGIOWIC1) { /* WIC1 */
            bus = CURIE_URANIUM_NIM1_PCIE_UPPORT_BUS;
            dev = CURIE_URANIUM_NIM1_PCIE_UPPORT_DEV;
            func = CURIE_URANIUM_NIM1_PCIE_UPPORT_FUNC;
            offset = CURIE_URANIUM_NIM1_PCIE_UPPORT_2ND_BUS_OFF;
            path = CURIE_URANIUM_NIM1_PCIE_UPPORT_PATH;
        } else {                /* WIC2 */
            bus = CURIE_URANIUM_NIM1_PCIE_UPPORT_BUS;
            dev = CURIE_URANIUM_NIM1_PCIE_UPPORT_DEV;
            func = CURIE_URANIUM_NIM1_PCIE_UPPORT_FUNC;
            offset = CURIE_URANIUM_NIM1_PCIE_UPPORT_2ND_BUS_OFF;
            path = CURIE_URANIUM_NIM2_PCIE_UPPORT_PATH;
        }
        reg = pci_config_read(bus, dev, func, offset);
        /* step 1 - remove device */
        /* echo 1 > /sys/devices/pci0000:00/0000:00:03.3/0000:11:00.0/remove */
        sprintf(file_rmv, "%s0000:%02x:00.0/remove", path, reg);

        if (file_exist(file_rmv, &size)) {
            sprintf(buf, "echo 1 > %s", file_rmv);
            system(buf);
        }

        /* step 2 - secoundary bus reset to disable link */
        if (intf->slot == NGIOWIC1) { /* WIC1 */
            offset = CURIE_URANIUM_NIM1_PCIE_UPPORT_2ND_RESETOFF;
        } else {
            offset = CURIE_URANIUM_NIM2_PCIE_UPPORT_2ND_RESETOFF;
        }
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

        msleep(3000); /* using 1 sec delay is not stable enough */
        ngio->wic[intf->slot-FIRST_SLOT].ctrl &= ~(NGIO_PWR_EN);

        /* step 3 - unreset secondary bus */
        val = 0x0; /* unreset, default value is 0*/
        reg = pci_config_write(bus, dev, func, offset, val);

        msleep(2000); /* using 1 sec delay is not stable enough */
    } else {
        ngiowic_reset(p);
        ngiowic_i2c_reset(p);

        ngio->wic[intf->slot-FIRST_SLOT].ctrl &= ~(NGIO_PWR_EN);

        ngio->wic[intf->slot-FIRST_SLOT].ctrl &=
            ~(NGIO_PCI_RDY | NGIO_UART_TX);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        print_offset_val((char *)__FUNCTION__, dash_fpga,
                         (unsigned long)&ngio->wic[intf->slot-FIRST_SLOT].ctrl,
                         0, 0);
    }
    msleep(ENABLE_DELAY);
    /* Recovery PCIe downstream port */
    if (is_overlord()) {
        printf("\nRecovery PCIe downstream port\n");
        if ((intf->mod_type == SM_MODULE) || (intf->mod_type == SM_DAUGHTER_CARD)) {
	        switch (intf->slot) {
            case NGSM1_SLOT:
                // disable NGSM1 PCIe port
                sprintf(cmd, "setpci -s 3:c.0 540.l=80000000");
                system(cmd);
            break;
            case NGSM2_SLOT:
                // disable NGSM2 PCIe port
                sprintf(cmd, "setpci -s 3:e.0 540.l=80000000");
                system(cmd);
            break;
            default: 
            break;
            }
        } else if (intf->mod_type == WIC_MODULE) { 
            switch(intf->slot) {
            case NGWIC1_SLOT:
                // disable NIM1 PCIe port
                sprintf(cmd, "setpci -s 3:8.0 540.l=80000000");
                system(cmd);
            break;
            case NGWIC2_SLOT:
                // disable NIM2 PCIe port
                sprintf(cmd, "setpci -s 3:a.0 540.l=80000000");
                system(cmd);
            break;
            case NGWIC3_SLOT:
                // disable NIM3 PCIe port
                sprintf(cmd, "setpci -s 3:2.0 540.l=80000000");
                system(cmd);
            break;
            default: 
            break;
            }
        }
    }
    /* Enable PCIe downstream port */
    if (is_plx_wrapper()) {
        /* PLX */
        if (is_juno()) {
        } else if (is_utah()) {
            if ((intf->mod_type == SM_MODULE) || (intf->mod_type == SM_DAUGHTER_CARD)) {
	            switch (intf->slot) {
                case NGSM1_SLOT:
                    // disable Sword NGSM1 PCIe port
                    sprintf(cmd, "setpci -s 4:0.0 234.l=0");
                    system(cmd);
                break;
                case NGSM2_SLOT:
                    // disable Sword NGSM2 PCIe port
                    sprintf(cmd, "setpci -s 4:0.0 230.l=0");
                    system(cmd);
                break;
                default: 
                break;
                }
            } else if (intf->mod_type == WIC_MODULE) { 
                switch(intf->slot) {
                case NGWIC1_SLOT:
                    // disable Sword NIM1 PCIe port
                    sprintf(cmd, "setpci -s 4:0.0 234.l=0");
                    system(cmd);
                break;
                case NGWIC2_SLOT:
                    // disable Sword NIM2 PCIe port
                    sprintf(cmd, "setpci -s 4:0.0 234.l=0");
                    system(cmd);
                break;
                case NGWIC3_SLOT:
                    // disable Sword NIM3 PCIe port
                    sprintf(cmd, "setpci -s 4:0.0 234.l=0");
                    system(cmd);
                break;
                default: 
                break;
                }
            }
        } else if (is_sword()) {
            if ((intf->mod_type == SM_MODULE) || (intf->mod_type == SM_DAUGHTER_CARD)) {
	            switch (intf->slot) {
                /* only one SM */
		        case NGIOSM1:
                    // disable Sword NIM1 PCIe port
                    sprintf(cmd, "setpci -s 4:0.0 234.l=0");
                    system(cmd);
		        }
            } else if (intf->mod_type == WIC_MODULE) {
                switch (intf->slot) {
                case NGWIC1_SLOT:
                    // disable Sword NIM1 PCIe port
                    sprintf(cmd, "setpci -s 4:0.0 230.l=0");
                    system(cmd);
                break;
                case NGWIC2_SLOT:
                   // disable Sword NIM2 PCIe port
                    sprintf(cmd, "setpci -s 4:0.0 234.l=0");
                    system(cmd);
                break;
                default: 
                break;
                }
            }
        } else if (is_dagger()) {
            switch (intf->slot) {
            case NGWIC1_SLOT:
                // disable Sword NIM1 PCIe port
                sprintf(cmd, "setpci -s 4:0.0 234.l=0");
                system(cmd);
            break;
            case NGWIC2_SLOT:
                // disable Sword NIM12 PCIe port
                sprintf(cmd, "setpci -s 4:0.0 234.l=0");
                system(cmd);
            break;
            default: 
            break;
            }
        }
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
    ngio_t *ngio = (ngio_t *)(dash_fpga +  NGIO_BASE);
    struct ngio_intf_t *intf = (struct ngio_intf_t *)p;
    assert(intf->slot);    

    ngio->ngvm |= NGIO_I2C_RESET;

    if ((NVRAM)->diagflag & D_VERBOSE) {
        print_offset_val((char *)__FUNCTION__, dash_fpga, (unsigned long)&ngio->ngvm,
                     0, 0);
    }

    return (OK);
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
    ngio_t *ngio = (ngio_t *)(dash_fpga +  NGIO_BASE);
    struct ngio_intf_t *intf = (struct ngio_intf_t *)p;
    assert(intf->slot);    

    ngio->ngvm &= ~NGIO_I2C_RESET;
    if ((NVRAM)->diagflag & D_VERBOSE) {
        print_offset_val((char *)__FUNCTION__, dash_fpga,
                         (unsigned long)&ngio->ngvm, 0, 0);
    }
    msleep(30);

    return (OK);
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
    ngio_t *ngio = (ngio_t *)(dash_fpga +  NGIO_BASE);
    struct ngio_intf_t *intf = (struct ngio_intf_t *)p;
    
    assert(intf->slot);    

    ngio->ngvm |= NGIO_RESET;
    if ((NVRAM)->diagflag & D_VERBOSE) {
        print_offset_val((char *)__FUNCTION__, dash_fpga, (unsigned long)&ngio->ngvm,
                         0, 0);
    }
    return (PASSED);
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
    ngio_t *ngio = (ngio_t *)(dash_fpga +  NGIO_BASE);
    struct ngio_intf_t *intf = (struct ngio_intf_t *)p;
    assert(intf->slot);    

    ngio->ngvm &= ~NGIO_RESET;
    if ((NVRAM)->diagflag & D_VERBOSE) {
        print_offset_val((char *)__FUNCTION__, dash_fpga, 
                         (unsigned long)&ngio->ngvm, 0, 0);
    }
    return (OK);
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
    int sts;
    struct ngio_intf_t *intf = (struct ngio_intf_t *)p;
    
    ngio_t *ngio = (ngio_t *)(dash_fpga +  NGIO_BASE);

    intf->slot = FIRST_SLOT;
    
    sts = ngio->ngvm;

    return (sts & NGIO_PRSNT);

}

/*-------------------------------------------------------------------
 *
 * Function : ngiopim_enable_intr
 * Description: enable pim intr
 * INPUT:  dev -- slot number starting with 1
 *         intr type -- type of interupt
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
void
ngiopim_enable_intr (int dev, int intr_type)
{
    ng_t *ngio = (ng_t *)(dash_fpga + PLUG_MODULE_OFFSET); 
    
    ngio->intr |= intr_type;

}

/*-------------------------------------------------------------------
 *
 * Function : ngiopim_disable_intrn
 * Description: disable pim interrupt
 * INPUT:  dev -- slot number starting with 1
 *         intr type -- type of interupt
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
void
ngiopim_disable_intr (int dev, int intr_type)
{
    ng_t *ngio = (ng_t *)(dash_fpga + PLUG_MODULE_OFFSET); 
    
    ngio->intr &= ~intr_type;

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
    ngiovm_reset(p);
    ngiovm_i2c_reset(p);
    //    assert(!"ngiovm_disable function not supported\n");
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
    ngio_t *ngio = (ngio_t *)(dash_fpga +  NGIO_BASE);
    char buf[48];
    ng_t *io;

    io = (ng_t *)&ngio->sm[0];
    clr_oir_intr(io, buf, SM_MODULE, 1);
    if (*buf && ((NVRAM)->diagflag & D_VERBOSE)) {
        printf("\n\n****NGIOSM1 %s OIR\n\n", buf);
    }
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
    char buf[48];
    ngio_t *ngio = (ngio_t *)(dash_fpga +  NGIO_BASE);
    ng_t *io;

    io = (ng_t *)&ngio->sm[1];
    clr_oir_intr(io, buf, SM_MODULE, 2);
    if (*buf  && ((NVRAM)->diagflag & D_VERBOSE)) {
        printf("\n\n****NGIOSM2 %s OIR\n\n", buf);
    }
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
    char buf[48];
    ngio_t *ngio = (ngio_t *)(dash_fpga +  NGIO_BASE);
    ng_t *io;

    io = (ng_t *)&ngio->sm[2];
    clr_oir_intr(io, buf, SM_MODULE, 3);
    if (*buf  && ((NVRAM)->diagflag & D_VERBOSE)) {
        printf("\n\n****NGIOSM3 %s OIR\n\n", buf);
    }
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
    char buf[48];
    ngio_t *ngio = (ngio_t *)(dash_fpga +  NGIO_BASE);
    ng_t *io;

    io = (ng_t *)&ngio->sm[3];
    clr_oir_intr(io, buf, SM_MODULE, 4);
    if (*buf  && ((NVRAM)->diagflag & D_VERBOSE)) {
        printf("\n\n****NGIOSM3 %s OIR\n\n", buf);
    }
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
        printf("\n\n****NGIOWIC2 %s OIR\n\n", buf);
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
    char buf[48];
    ngio_t *ngio = (ngio_t *)(dash_fpga +  NGIO_BASE);
    ng_t *io;

    io = (ng_t *)&ngio->wic[2];
    clr_oir_intr(io, buf, WIC_MODULE, 3);
    if (*buf && ((NVRAM)->diagflag & D_VERBOSE)) {
        printf("\n\n****NGIOWIC3 %s OIR\n\n", buf);
    }

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

    io = (ng_t *)&ngio->sm[0];
    clr_oir_intr(io, buf, SM_MODULE, 1);
    io = (ng_t *)&ngio->sm[1];
    clr_oir_intr(io, buf, SM_MODULE, 2);

    io = (ng_t *)&ngio->wic[0];
    clr_oir_intr(io, buf, WIC_MODULE, 1);
    io = (ng_t *)&ngio->wic[1];
    clr_oir_intr(io, buf, WIC_MODULE, 2);
    io = (ng_t *)&ngio->wic[2];
    clr_oir_intr(io, buf, WIC_MODULE, 3);

    ngio->sata_ctrl |= NGIO_SATA1_INS | NGIO_SATA2_INS |
        NGIO_SATA1_RMV | NGIO_SATA1_RMV;

    if (is_neptune() || is_vg450()) {
        io = (ng_t *)&ngio->sm[2];
        clr_oir_intr(io, buf, SM_MODULE, 3);
        io = (ng_t *)&ngio->sm[3];
        clr_oir_intr(io, buf, SM_MODULE, 4);
    }
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
    struct ngio_intf_t *intf = (struct ngio_intf_t *)p;
    char juno_nim_devnum[3] = {0x3, 0xb, 0xd};
    char utah_sm_devnum[2] = {0x1, 0x2};
    char utah_nim_devnum[3] = {0x3, 0xb, 0xd};
    char sword_sm_devnum[1] = {0x1};
    char sword_nim_devnum[2] = {0x2, 0x3};
    char dagger_nim_devnum[2] = {0x1, 0x5};
    char bus_dev[8];
    char cmd[64];
    uint pcie_switch_bus_no = 0;

    bus_dev[0] = 0; /* Init to NULL string */
    if (is_plx_wrapper()) {
        if(is_juno()) {
	    /* The PCIe bus number for the NGIO slot is 1 higher than
	     * the bus for the PCIe switch itself
	     */
	    pcie_switch_bus_no = 1 + get_pcie_bus_num(PLX_PCIE_SW_VID, PLX_PCIE_SW_DID_8618);

	    if (intf->mod_type == WIC_MODULE) {
	        switch (intf->slot) {
		case NGIOWIC1:
		case NGIOWIC2:
		case NGIOWIC3:
		    sprintf(bus_dev, "%x:%x", pcie_switch_bus_no, juno_nim_devnum[intf->slot - 1]);
		    break;
		}
	    }
	}
	else if (is_utah()) {
	    /* The PCIe bus number for the NGIO slot is 1 higher than
	     * the bus for the PCIe switch itself
	     */
	    pcie_switch_bus_no = 1 + get_pcie_bus_num(PLX_PCIE_SW_VID, PLX_PCIE_SW_DID_8618);

	    if (intf->mod_type == SM_MODULE ||
                intf->mod_type == SM_DAUGHTER_CARD) {
	        switch (intf->slot) {
		case NGIOSM1:
		case NGIOSM2:
		    sprintf(bus_dev, "%x:%x", pcie_switch_bus_no, utah_sm_devnum[intf->slot - 1]);
		    break;
		}
	    }
	    else if (intf->mod_type == WIC_MODULE) {
	        switch (intf->slot) {
		case NGIOWIC1:
		case NGIOWIC2:
		case NGIOWIC3:
		    sprintf(bus_dev, "%x:%x", pcie_switch_bus_no, utah_nim_devnum[intf->slot - 1]);
		    break;
		}
	    }
	}
	else if (is_sword()) {
	    /* The PCIe bus number for the NGIO slot is 1 higher than
	     * the bus for the PCIe switch itself
	     */
	    pcie_switch_bus_no = 1 + get_pcie_bus_num(PLX_PCIE_SW_VID, PLX_PCIE_SW_DID_8617);

	    if (intf->mod_type == SM_MODULE ||
                intf->mod_type == SM_DAUGHTER_CARD) {
	        switch (intf->slot) {
		case NGIOSM1:
		    sprintf(bus_dev, "%x:%x", pcie_switch_bus_no, sword_sm_devnum[intf->slot - 1]);
		    break;
		}
	    }
	    else if (intf->mod_type == WIC_MODULE) {
	        switch (intf->slot) {
		case NGIOWIC1:
		case NGIOWIC2:
		    sprintf(bus_dev, "%x:%x", pcie_switch_bus_no, sword_nim_devnum[intf->slot - 1]);
		    break;
		}
	    }
	}
	else if (is_dagger()) {
	    /* The PCIe bus number for the NGIO slot is 1 higher than
	     * the bus for the PCIe switch itself
	     */
	    pcie_switch_bus_no = 1 + get_pcie_bus_num(PLX_PCIE_SW_VID, PLX_PCIE_SW_DID_8604);

	    if (intf->mod_type == WIC_MODULE) {
	        switch (intf->slot) {
		case NGIOWIC1:
		case NGIOWIC2:
		    sprintf(bus_dev, "%x:%x", pcie_switch_bus_no, dagger_nim_devnum[intf->slot - 1]);
		    break;
		}
	    }
	}

	if (bus_dev[0] != 0) {
	    if (enable) {
	        sprintf(cmd, "setpci -s %s 80.w=0x8:8", bus_dev);
	    }
	    else {
	        sprintf(cmd, "setpci -s %s 80.w=0x0:8", bus_dev);
	    }

	    system(cmd);
	}
    }
}

/*
 * Function: is_ngio_use_pcie
 *
 * This code checks if the NGIO card is using the PCIe bus. This check
 * is needed so that the interrupt mask bit can be turn on with 
 * ngio_plx_intr_mask() to get around an issue associated with 
 * the PLX switch on the Victory platforms.
 */
int
is_ngio_use_pcie(void *p)
{
    struct ngio_intf_t *intf = (struct ngio_intf_t *)p;
    unsigned short ngio_id;
    int index = 0;
    struct module_info *modinfo_p;
    char err[80]; 

    
    if (intf->get_id((void *)intf, err)) {
        assert(!"is_ngio_use_pcie: Failed to get slot id.");
    } else {
        ngio_id = intf->id; 
    }

    modinfo_p = (struct module_info *)get_platform_slot_table(&index, ngio_id);

    if (modinfo_p == NULL) {
        printf("index is %d; ngio_id is %#x\n", index, ngio_id);
        assert(!"is_ngio_use_pcie: can not retrieve index.");
    }
    if ((modinfo_p->mod_info_flags) & MOD_INFO_USE_PCIE) {
        return(TRUE);
    }
    else {
        return(FALSE);
    }
}

static uint8_t
get_pcie_secondary(uint8_t bus, uint8_t dev, uint8_t func)
{
    return pci_config_read(bus, dev, func, 0x19);
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

    uint pcie_switch_bus_no = 0; 
    uint device_id, vendor_id; 
    uint dev_no = 0, data, ngio_pcie_bus_no; 
    int nep_p1c_pcie_down_port_bus[6] = {1, 2, 7, 3, 9, 8}; /* sm1-3 and nim1-3 */
    int nep_p1b_pcie_down_port_bus[6] = {7, 8, 9, 1, 2, 3}; /* sm1-3 and nim1-3 */
    int *nep_pcie_bus_ptr;
    unsigned int plat_bd_rev = 999;

    if (is_goldbeach() || is_vg400()) { 
        /* Goldbeach without PCIe switch, NIM's PCIe connect to CPU */
        switch(slot) {
        case NGWIC1_SLOT:
            dev_no = NGWIC1_SLOT + 1;
            break;
        case NGWIC2_SLOT:
            dev_no = NGWIC2_SLOT + 1;
            break;
        default: 
            dev_no = 0;
            break;
        } 
        return(dev_no); 
    }

    /* afix - these value be updated with ROMMON
     * version Curie2RU-20181130
     */
    if (is_uranium() || is_thorium()) {
        if ((mod_type == SM_MODULE) || (mod_type == SM_DAUGHTER_CARD)) {
            switch(slot) {
            case NGSM1_SLOT:
                dev_no = get_pcie_secondary(CURIE_URANIUM_SM1_PCIE_UPPORT_BUS,
                                            CURIE_URANIUM_SM1_PCIE_UPPORT_DEV,
                                            CURIE_URANIUM_SM1_PCIE_UPPORT_FUNC);
                break;
            case NGSM2_SLOT:
                dev_no = get_pcie_secondary(CURIE_URANIUM_SM2_PCIE_UPPORT_BUS,
                                            CURIE_URANIUM_SM2_PCIE_UPPORT_DEV,
                                            CURIE_URANIUM_SM2_PCIE_UPPORT_FUNC);
                break;
            default:
                dev_no = 0;
                break;
            }
        } else if (mod_type == WIC_MODULE) {
            switch(slot) {
            case NGWIC1_SLOT:
                dev_no = get_pcie_secondary(CURIE_URANIUM_NIM1_PCIE_UPPORT_BUS,
                                            CURIE_URANIUM_NIM1_PCIE_UPPORT_DEV,
                                            CURIE_URANIUM_NIM1_PCIE_UPPORT_FUNC);
                break;
            case NGWIC2_SLOT:
                dev_no = get_pcie_secondary(CURIE_URANIUM_NIM2_PCIE_UPPORT_BUS,
                                            CURIE_URANIUM_NIM2_PCIE_UPPORT_DEV,
                                            CURIE_URANIUM_NIM2_PCIE_UPPORT_FUNC);
                break;
            default:
                dev_no = 0;
                break;
            }
        } else if (mod_type == DAUGHTER_CARD) {
            dev_no = get_adapter_pcie_sub_bus_num(slot);
        } else {
            dev_no = 0;
        }
        return(dev_no);
    }

    /* afix - these value be updated with ROMMON
     * version Curie1RU-20180912
     */
    if (is_radium() || is_thallium()) { 
        if ((mod_type == SM_MODULE) || (mod_type == SM_DAUGHTER_CARD)) {
           /* only one SM */
           /* 20191212 - due to rommon change pcie down port again, 
            * using dynamic method to get down port bus number.
            */
           /* upper port to SM is 0:3.3, register 0x19 */
           dev_no = pci_config_read(0, 3, 3, 0x19);
        } else if (mod_type == WIC_MODULE) {
           /* only one NIM */
           /* upper port to NIM is 0:1c.0, register 0x19 */
           dev_no = pci_config_read(0, 0x1c, 0, 0x19);
        } else if (mod_type == DAUGHTER_CARD) {
            dev_no = get_adapter_pcie_sub_bus_num(slot);
        } else {
           dev_no = 0; 
        }
        return(dev_no & 0xFF); /* we just need to have bit[7:0] */
    }

    /* afix - these value needs updated. 180712
     */
    if (is_polonium()) {
        if ((mod_type == SM_MODULE) || (mod_type == SM_DAUGHTER_CARD)) {
           /* only one SM */
           dev_no = 0x1;
        } else if (mod_type == WIC_MODULE) {
           switch(slot) {
           case NGWIC1_SLOT:
               dev_no = 0x2;
           break; 
           case NGWIC2_SLOT:
               dev_no = 0x3;
           break; 
           }
        } else {
           dev_no = 0;
        }
        return(dev_no); 
    }


    if (is_plx_wrapper()) {
        /* PLX */
        vendor_id = PLX_PCIE_SW_VID;

        if (is_juno()) {
            device_id = PLX_PCIE_SW_DID_8618;
            /* Juno has NIM only */
            switch(slot) {
            case NGWIC1_SLOT:
                dev_no = 3;
            break;
            case NGWIC2_SLOT:
                dev_no = 0xb;
            break;
            case NGWIC3_SLOT:
                dev_no = 0xd;
            break;
            default: 
                dev_no = 0;
            break;
            }        

            if (mod_type != WIC_MODULE) {
                dev_no = 0;
            }
        } else if (is_utah()) {
            device_id = PLX_PCIE_SW_DID_8618;

            if ((mod_type == SM_MODULE) || (mod_type == SM_DAUGHTER_CARD)) {
                switch(slot) {
                case NGSM1_SLOT:
                    dev_no = 0x1;
                break;
                case NGSM2_SLOT:
                    dev_no = 0x2;
                break;
                default: 
                    dev_no = 0;
                break;
                }
            } else if (mod_type == WIC_MODULE) { 
                switch(slot) {
                case NGWIC1_SLOT:
                    dev_no = 3;
                break;
                case NGWIC2_SLOT:
                    dev_no = 0xb;
                break;
                case NGWIC3_SLOT:
                    dev_no = 0xd;
                break;
                default: 
                    dev_no = 0;
                break;
                }
            } else {
                dev_no = 0;
            }
        } else if (is_sword()) {
            device_id = PLX_PCIE_SW_DID_8617;

            if ((mod_type == SM_MODULE) || (mod_type == SM_DAUGHTER_CARD)) {
                /* only one SM */
                dev_no = 0x1;
            } else if (mod_type == WIC_MODULE) {
                switch(slot) {
                case NGWIC1_SLOT:
                    dev_no = 2;
                break;
                case NGWIC2_SLOT:
                    dev_no = 3;
                break;
                default: 
                    dev_no = 0;
                break;
                }
            } else {
                dev_no = 0;
            }
        } else if (is_dagger()) {

            device_id = PLX_PCIE_SW_DID_8604;
            switch(slot) {
            case NGWIC1_SLOT:
                dev_no = 1;
            break;
            case NGWIC2_SLOT:
                dev_no = 5;
            break;
            default: 
                dev_no = 0;
            break;
            }
        } else {
            dev_no = 0;
        }

    } else if (is_ntpn_machines() || is_vg450()) {
        /* Pericom */ 
        vendor_id = PERICOM_PCIE_SW_VID;
        device_id = PERICOM_PCIE_SW_DID;

        nep_pcie_bus_ptr = nep_p1c_pcie_down_port_bus;

        /* Neptune P1C changed the PCIe switch down port mapping.
         * Neptune P1B and Triton P1A use an older map.
         */
        if (is_neptune() || is_triton() || is_vg450()) {
            get_platform_bd_rev(&plat_bd_rev);
            if (plat_bd_rev < 2) {
                nep_pcie_bus_ptr = nep_p1b_pcie_down_port_bus;
            }
        }

        if ((mod_type == SM_MODULE) || (mod_type == SM_DAUGHTER_CARD)) {
            switch(slot) {
            case NGSM1_SLOT:
                dev_no = nep_pcie_bus_ptr[0]; 
            break;
            case NGSM2_SLOT:
                dev_no = nep_pcie_bus_ptr[1]; 
            break;
            case NGSM3_SLOT:
                dev_no = nep_pcie_bus_ptr[2]; 
            break;
            default: 
                dev_no = 0; 
            break; 
            }
        } else if (mod_type == WIC_MODULE) {  /* NIM */
            switch(slot) {
            case NGWIC1_SLOT:
                dev_no = nep_pcie_bus_ptr[3]; 
            break;
            case NGWIC2_SLOT:
                dev_no = nep_pcie_bus_ptr[4]; 
            break;
            case NGWIC3_SLOT:
                dev_no = nep_pcie_bus_ptr[5]; 
            break;
            default: 
                dev_no = 0; 
            break; 
            }
        } else {
            dev_no = 0; 
        }
    } else {
        /* IDT */
        vendor_id = IDT_PCIE_SW_VID;
        device_id = IDT_PCIE_SW_DID;

        /* For NIM, O2, Juno and Utah have the same dev_no for SM/NIM */
        if ((mod_type == SM_MODULE) || (mod_type == SM_DAUGHTER_CARD)) {
            switch(slot) {
            case NGSM1_SLOT:
                dev_no = 0xc;
            break;
            case NGSM2_SLOT:
                dev_no = 0xe;
            break;
            default: 
                dev_no = 0;
            break;
            }
        } else if (mod_type == WIC_MODULE) {  /* NIM */
            switch(slot) {
            case NGWIC1_SLOT:
                dev_no = 8;
            break;
            case NGWIC2_SLOT:
                dev_no = 0xa;
            break;
            case NGWIC3_SLOT:
                dev_no = 0x2;
            break;
            default: 
                dev_no = 0;
            break;
            }
        } else {
            dev_no = 0;
        }

    } /* is_plx_wrapp */

    if (dev_no == 0) {
        printf("slot num or mod_type is not match to platform, return -1 \n");
        return -1;
    }

    /* The PCIe bus number for the NGIO slot is 1 higher than
     * the bus for the PCIe switch itself
     */
    pcie_switch_bus_no = 1 + get_pcie_bus_num(vendor_id, device_id);

    /* 0x18 is contain the bus info */
    data = pci_config_read(pcie_switch_bus_no, dev_no, 0, 0x18);

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("bus = 0x%x, dev_no = 0x%x\n", pcie_switch_bus_no, dev_no);
        printf("Primary bus, 2nd bus, sub bus = %#.2x, %#.2x, %#.2x\n",
                data & 0xff, (data & 0xff00) >> 8, (data & 0xff0000) >> 16);
    }

    ngio_pcie_bus_no = ((data & 0xff00) >> 8);

    return (ngio_pcie_bus_no); 
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

    /* Overlord and Juno are not 10gkr capable.
     * Utah (usd machines) and Neptune (ntpn machines) are 10gkr
     * capable.
     */
    if (is_plat_10gkr_capable()) {
        if ((mod_type == SM_MODULE) || (mod_type == WIC_MODULE) ||
	    (mod_type == SM_DAUGHTER_CARD) || (mod_type == DAUGHTER_CARD)) {

	    if (is_usd_machines()) {
	        /* Utah (usd machines) support 10gkr on GE0 of all
		 * NGIO modules.
		 */
		result = NGIO_GE0_BITMASK;
	    } else if (is_ntpn_machines() || is_vg450()) {
	        /* Neptune (ntpn machines) support GE0 of all SM
		 * modules, and GE0 of NIM-1 only. NIM-2 and 3 slots
		 * are not 10gkr capable.
		 */
		result = NGIO_GE0_BITMASK;
		if ((mod_type == WIC_MODULE) && (slot > 1)) {
		    result = 0;
		}
	    } else {
                /* Curie supports 10G-KR on both ge0 and ge1 */
                if (is_curie_1ru() || is_curie_2ru()) {
	            result = NGIO_GE0_BITMASK | NGIO_GE1_BITMASK;
                }
            }
	}
    }
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
    int tgt_dev, gesw_port_num;
    int rv;
    char *gesw_port_name="GESW port name unknown";

    switch (mod_type) {
    case SM_DAUGHTER_CARD:
    case SM_MODULE:
        tgt_dev = TGT_DEV_NGSM;
	break;
    case WIC_MODULE:
    case DAUGHTER_CARD:
        tgt_dev = TGT_DEV_NGWIC;
	break;
    default:
        tgt_dev = -1;
        return(FAIL);
    }

    if ((gesw_port_num = ovld_get_ge_sw_port_num(slot, tgt_dev, ngio_port)) >= 0) {
        gesw_port_name = get_gesw_pname(gesw_port_num);
    }

    rv = cfg_10gkr_port(gesw_port_num, en_10gkr);

    if ((en_10gkr) || (rv)) {
        printf("Configure host %s-%d NGIO GE port %d (GESW port %d (%s)) to %s\n",
	       ((mod_type == SM_MODULE) ? "SM" : "NIM"), slot, ngio_port,
	       gesw_port_num, gesw_port_name, (en_10gkr ? "10G" : "1G"));
    }
    if (rv) {
        printf("%s: GESW port %d (%s) configuration failed.\n", __FUNCTION__, gesw_port_num,  gesw_port_name);
	return(FAIL);
    }
    return(PASS);
}

/*-------------------------------------------------------------------
 *
 * Function : ngio_host_get_ctrl_port
 * Description: get platform's network interface index
 * INPUT:  mode_type -- type of Module, SM or NIM
           slot - slot number
 * OUTPUT: interface index
 * -------------------------------------------------------------------
*/
static int ngio_host_get_ctrl_port(uint mod_type, uint slot)
{
    int ctrl_plane_port = 0;

    if (is_overlord() || is_juno() ||
        is_ntpn_machines() || is_goldbeach() || is_vg450() || is_vg400()) {
       /* o2 juno */
       ctrl_plane_port = CPU_SGMII_PORT1;
    } else if (is_curie_1ru()) {
       /* curie 1ru NIM1 GE0=eth7, no GE1; SM1 GE0=eth6, GE1=eth8 */
       switch (mod_type) {
       case WIC_MODULE:
       case WIC_DAUGHTER_CARD:
           ctrl_plane_port = CPU_SGMII_PORT7;
	   break;
       case SM_MODULE:
           ctrl_plane_port = CPU_SGMII_PORT6;
	   break;
       }
    } else if (is_curie_2ru()) {
        /* curie 2ru NIM1 GE0=eth7, no GE1; NIM2 GE0=eth6, no GE1
         * SM1 GE0=eth11, GE1=eth9, SM2 GE0=eth10, GE1=eth8 */
        switch (mod_type) {
        case WIC_MODULE:
        case WIC_DAUGHTER_CARD:
            switch (slot) {
            case NGWIC1_SLOT:
                ctrl_plane_port = CPU_SGMII_PORT7;
                break;
            case NGWIC2_SLOT:
                ctrl_plane_port = CPU_SGMII_PORT6;
                break;
            }
            break;
        case SM_MODULE:
            switch (slot) {
            case NGSM1_SLOT:
                ctrl_plane_port = CPU_SGMII_PORT11;
                break;
            case NGSM2_SLOT:
                ctrl_plane_port = CPU_SGMII_PORT10;
                break;
            }
            break;
        }
    } else {
        /* USD */
       ctrl_plane_port = CPU_SGMII_PORT3;
    }

    return ctrl_plane_port;
}

#define SPEED_STRING "Speed: "

/*-------------------------------------------------------------------
 *
 * Function : ngio_get_eth_port_speed
 * Description: get current speed of specific interface using ethtool
 * INPUT:  eth_num - network interface index
           speed - pointer to the returned speed
 * OUTPUT: On success PASS is returned. On error, FAILED is returned.
 * -------------------------------------------------------------------
*/
static int ngio_get_eth_port_speed(uint eth_num, ngio_eth_speed_t *speed)
{
    char cmd[30], buf[2048];
    FILE *fp;
    char *substr;
    uint cur_speed = 0;

    sprintf(cmd, "ethtool eth%d", eth_num);
    if ((fp = popen(cmd, "r")) == NULL) {
        printf("%s(%d): Failed to popen %s\n", __func__, __LINE__, cmd);
        return FAIL;
    }

    while (fgets(buf, sizeof(buf), fp) != NULL) {
        if (!strcmp(buf, "Auto-negotiation: on")) {
            *speed = 0;
            break;;
        }

        if ((substr = strstr(buf, SPEED_STRING)) != NULL) {
            substr += strlen(SPEED_STRING);
            if (!strcmp(buf, "Unknown!")) {
                *speed = NGIO_ETH_SPEED_UNKNOWN;
                break;;
            } else {
                cur_speed = atoi(substr);
                switch (cur_speed) {
                case 10:
                    *speed = NGIO_ETH_SPEED_10M;
                    break;
                case 100:
                    *speed = NGIO_ETH_SPEED_100M;
                    break;
                case 1000:
                    *speed = NGIO_ETH_SPEED_1G;
                    break;
                case 10000:
                    *speed = NGIO_ETH_SPEED_10G;
                    break;
                default:
                    *speed = NGIO_ETH_SPEED_UNKNOWN;
                    break;
                }
            }
        }
    }

    if (pclose(fp) == -1) {
        printf("%s(%d): Failed to pclose %s\n", __func__, __LINE__, cmd);
        return FAIL;
    }

    return PASS;
}

/*-------------------------------------------------------------------
 *
 * Function : ngio_get_eth_port_speed
 * Description: set the speed of a specific interface using ethtool
 * INPUT:  eth_port - network interface index
           speed - pointer to the speed
 * OUTPUT: PASS
 * -------------------------------------------------------------------
*/
static int _ngio_cfg_eth_port_speed(int eth_port, const ngio_eth_speed_t *speed)
{
    char cmd[256];

    if (is_curie_2ru()) {
        // Only Curie2RU need to be setted to force 1G
        switch (*speed) {
        case NGIO_ETH_SPEED_10M:
            sprintf(cmd, "ethtool -s eth%d autoneg off speed 10", eth_port);
            break;
        case NGIO_ETH_SPEED_100M:
            sprintf(cmd, "ethtool -s eth%d autoneg off speed 100", eth_port);
            break;
        case NGIO_ETH_SPEED_1G:
            sprintf(cmd, "ethtool -s eth%d autoneg off speed 1000", eth_port);
            break;
        case NGIO_ETH_SPEED_10G:
            sprintf(cmd, "ethtool -s eth%d autoneg off speed 10000", eth_port);
            break;
        default:
            sprintf(cmd, "ethtool -s eth%d autoneg on advertise 10000", eth_port);
            break;
        }
        system(cmd);
    }

    return PASS;
}

int ngio_cfg_eth_port_speed(uint mod_type, uint slot,
                            const ngio_eth_speed_t *new_speed, ngio_eth_speed_t *old_speed)
    __attribute__((weak, alias("__ngio_cfg_eth_port_speed")));

/*
 * Function: __ngio_cfg_host_port_forced_speed
 * Configure the host ethx port to be forced speed or Auto-negotiation.
 *
 * Curie 2RU added it. NIM's MAC BCM57412 has not the capability of
 * auto-negotiation to 1G from 10G. We need to set 1G force mode.
 *
 * input : mod_type - NGIO module type
 *         slot - NGIO slot number
 *         speed - 0: Auto-negotiation, other: X Mb/s
 *
 * output: speed - previous speed configuration
 *
 * return: PASS or FAIL
 */
int __ngio_cfg_eth_port_speed(uint mod_type, uint slot,
                              const ngio_eth_speed_t *new_speed, ngio_eth_speed_t *old_speed)
{
    int eth_port;

    if (!is_curie_2ru()) {
        return PASS;
    }

    eth_port = ngio_host_get_ctrl_port(mod_type, slot);

    if ((old_speed != NULL) && (ngio_get_eth_port_speed(eth_port, old_speed) == FAIL)) {
        return FAIL;
    }

    if ((new_speed != NULL) && (_ngio_cfg_eth_port_speed(eth_port, new_speed) == FAIL)) {
        return FAIL;
    }

    return PASS;
}
/******** History ******** 
$Log: ngio.c,v $
Revision 1.72  2021/08/04 04:48:24  xiaolaya
Add 70s delay as a WA for Switzer-Manhattan RDT eeprom check and pci failure

Revision 1.71  2020/07/03 03:19:48  jiajliu
CSCvn24010-63: curie2ru ngsm_unreset: Check if NGIO module is unreset to disable PCIe link

Revision 1.70  2020/05/22 02:28:34  qingcwan
Merge switzer-carrier code into main chunk.

Revision 1.69  2020/01/09 01:02:19  jiajliu
Merge Curie 2RU to main trunk

Revision 1.68  2019/12/18 09:18:37  alpeng
1. support quack cookie rd/wr; 2. fixed new rommon break nightwatch issue; 3. a workaround for new pim testcard crashed system issue; 4. bump to v2.0.1 for curie

Revision 1.67  2019/08/06 06:56:10  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.66  2018/08/30 06:59:55  haohsu
Collapse Vg400-branch to Main Trunk

Revision 1.65  2018/05/18 09:24:51  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.64  2016/10/16 15:42:52  iachang
Supported Goldbeach Platform.

Revision 1.63  2016/10/16 14:59:33  iachang
Supported Goldbeach Platform.

Revision 1.62.18.15  2018/05/17 10:50:22  alpeng
 sync with trunk <trunk-051618>

Revision 1.62.18.14  2018/05/11 06:55:00  alpeng
fixed typo on disable link

Revision 1.62.18.13  2018/04/18 09:10:23  alpeng
add workaround to remove testcard on neptune sm4 gracefully

Revision 1.62.18.12  2017/11/27 06:08:41  leschen
Initial check in to support VG450.

Revision 1.62.18.11  2017/10/11 06:40:57  alpeng
support both p1c and p1b neptune on dreamliner

Revision 1.62.18.10  2017/08/08 09:34:34  alpeng
Per HW suggestion, neptune does not need to access 234h and 230h

Revision 1.62.18.9  2017/07/28 08:53:38  alpeng
add disable downstream port for hotplug

Revision 1.62.18.8  2017/07/21 10:04:06  alpeng
move pci_rdy before reset and power down for ntpn machines

Revision 1.62.18.7  2017/04/05 06:41:58  leschen
Sync with <ng_diag-tag-032917>

Revision 1.62.18.6  2017/03/13 07:43:31  leschen
Support Triton system.

Revision 1.62.18.5  2016/10/28 08:31:23  alpeng
fixed enhance error msg bug, add more info on pcird/wr, update testcard plx scan test

Revision 1.62.18.4  2016/10/21 18:19:39  alpeng
update testcard for sm4

Revision 1.62.18.3  2016/10/18 18:58:55  alpeng
support sm3 and sm4, update intr table

Revision 1.62.18.2  2016/10/17 00:19:33  ptong
10G-KR modules now is supported on Neptune

Revision 1.62.18.1  2016/10/14 23:26:34  alpeng
update old testcard eth num and nim pci on ngio

Revision 1.64  2016/10/16 15:42:52  iachang
Supported Goldbeach Platform.

Revision 1.63  2016/10/16 14:59:33  iachang
Supported Goldbeach Platform.

Revision 1.62  2015/06/05 06:13:11  alpeng
fix poll slot issue

Revision 1.61  2015/02/27 10:02:24  iachang

Add support dreamliner NIM


Revision 1.60.4.5  2015/02/05 15:14:35  iachang
Port Jeff's code to disable downstream Hot Plug port

Revision 1.60.4.4  2015/01/31 00:51:05  iachang
Port Jeff's method to Disable PCIe electrical idle detec on O2.

Revision 1.60  2014/12/11 05:59:51  alpeng
correct the bit definition msg

Revision 1.59  2014/09/09 21:51:08  mcharon
change string 'off' to on in printf when gnio_ref_clk call fails

Revision 1.58  2014/09/06 00:43:10  ptong
r1.57 has funny pcilib error. roll back to 1.52 with latest change in ngio_ref_clk changes. good for both new and old test cards

Revision 1.52  2014/08/15 15:08:02  mcharon
clear pci rdy bit when take wic out of reset

Revision 1.51  2014/06/19 22:00:51  ptong
Incorporated BCM sdk-6.4.1-EA2 for 10G-KR interface. New code works on O2, June, and non Greyhound USD regression

Revision 1.50  2014/04/18 00:18:50  ptong
Prepare to support Greyhound 10G-KR bring-up

Revision 1.49  2014/04/14 06:49:55  alpeng
get_ngio_pcie_dev_bus_num() returns NGIO PCIe bus num

Revision 1.48  2014/04/08 10:43:12  alpeng
support general_get_ngio_pcie_bus_num() to return ngio bus num

Revision 1.47  2014/03/25 01:08:24  ptong
Use get_pcie_bus_num() to get the PCIe bus number for the NGIO slot

Revision 1.46  2014/03/20 22:24:49  ptong
Juno use different bus number

Revision 1.45  2014/03/18 21:23:40  ptong
Add ngio_plx_intr_mask() to control the PCIe interrupt on PLX switch

Revision 1.44  2014/01/29 01:27:55  mcharon
clear cookie id when insertion/removel event is dectected

Revision 1.43  2013/11/26 08:40:35  hroni
fix compiler warning

Revision 1.42  2013/09/12 23:26:40  mcharon
add function to support sync_out signal needed by tdm

Revision 1.41  2013/06/27 17:15:51  mcharon
reset module before turning off

Revision 1.40  2013/04/23 17:20:28  mcharon
add #define LEGACY in case we need to suport module in legacy mode

Revision 1.39  2013/04/03 02:12:39  mcharon
improve power fault message

Revision 1.38  2013/04/02 20:28:59  mcharon
print out power fault message when software detect power fault event

Revision 1.37  2013/04/02 19:18:18  mcharon
don't enable pci ready bit when taking wic out of rset

Revision 1.36  2013/03/29 05:50:18  mcharon
add function for setting pci rdy bit

Revision 1.35  2013/03/27 20:26:30  mcharon
force idt switch to notify hotplug

Revision 1.34  2013/02/16 06:51:57  mcharon
add error message

Revision 1.33  2013/01/14 18:35:46  mcharon
print power fault only if verbose flag is on

Revision 1.32  2012/12/24 10:06:41  srane
Print only if verbose flag enabled.

Revision 1.31  2012/12/21 18:06:59  mcharon
CSCud84025 fix daughter card segfault when powr off

Revision 1.30  2012/12/05 01:26:14  mcharon
when turning power off, reset i2c/uart/and module as well

Revision 1.29  2012/12/05 01:02:29  mcharon
remove print in ngiovm_reset

Revision 1.28  2012/11/21 01:06:34  mcharon
before turning on module, make sure ctrl reg is at default state

Revision 1.27  2012/11/21 00:59:34  mcharon
when power off ngio, need to put ngio back to reset as well

Revision 1.26  2012/11/17 01:15:17  mcharon
reset i2c device; don't cterr in driver code..propogate err message to slot.c

Revision 1.25  2012/11/16 18:22:35  mcharon
in ngiowic_reset, register should be wic and not sm

Revision 1.24  2012/11/12 20:35:23  mcharon
add third arg to slot_i2c_unrest to report slot num when fails..improve err reporting

Revision 1.23  2012/11/08 18:16:54  mcharon
based on ngio spec, ngio can take 500msecs to stablize, so provide delay after pwr up

Revision 1.21  2012/11/06 20:39:50  mcharon
add headers/cleanup/remove unneeded functions/files

Revision 1.18  2012/10/25 06:16:55  mcharon
turn on module when exit test

Revision 1.17  2012/10/11 08:24:42  alpeng
unsupported SATA on wic slot 3

Revision 1.16  2012/09/26 03:23:06  alpeng
check sata available via PID

Revision 1.14  2012/09/20 00:13:01  mcharon
support oir

Revision 1.13  2012/09/18 19:19:55  mcharon
support poll slot-fix io intr test; support fpga upgrade; support serial irq intr tst

Revision 1.12  2012/09/12 09:21:13  alpeng
remove SATA test from mbtest and integrate SATA test into ngwic3 test

Revision 1.11  2012/08/28 18:49:37  srane
Correct the bug in ngiodc reset/unreset routines.

Revision 1.10  2012/07/23 06:57:17  srane
Need to set the config register to output for the DC reset GPIO pins.

Revision 1.8  2012/05/30 16:44:53  palin2
Clean up compile warnings.

Revision 1.7  2012/05/16 07:27:26  srane
Add reset/unreset for daughter cards.

Revision 1.6  2012/05/16 02:48:53  alpeng
modified HDD present function for displaying MB menu

$Endlog$
*/
