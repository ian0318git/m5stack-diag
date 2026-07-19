/* $Id: slot.c,v 1.68 2021/04/12 13:36:29 xiaolaya Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/slot.c,v $
 *------------------------------------------------------------------
 * slot.c - Interface routines to support SM on xformers. 
 *
 * Copyright (c) 2015-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "types.h"
#include "menu.h"
#include "setjmps.h"
#include "nvmonvars.h"
#include "error.h"
#include "slot.h"
#include "strings.h"
#include "cookie_4.h"
#include "nmc93c46.h"
#include "platform_slot.h"  /* requires slot.h */
#include "dev_print.h"
#include "proto.h"
#include "linux_api.h"
#include "sm_slot.h"
#include "cross_platform.h"
#include "cli_cmd.h"
#include "i2c_api.h"
#include "smart_cookie.h"
#include "platform_i2c.h"
#include "platform_cookie.h"
#include "ngio.h"
#include "dash_fpga.h"  /* needd for externs;  common code should reall not have this */
#include "pca.h"
#include "plat_defs.h" /* needed for extern function is_plat_10gkr_capable() */

int slot_get_info(struct ngio_intf_t *ngio, char*);

static struct ngio_intf_t sm[MAX_SM+FIRST_SLOT];
static struct ngio_intf_t wic[MAX_WIC+FIRST_SLOT];
static struct ngio_intf_t vm[MAX_VM+FIRST_SLOT];
//static n2g_i2c_if_t pca_i2c;
//static unsigned char pca_buff[256];



/*-------------------------------------------------------------------
 *
 * Function : set_ngio_now_testing
 * Description: a weak function for set_ngio_now_testing 
 *
 * INPUT:  ngio_ptr - pointer to ngio structure.
 * OUTPUT: None
 * -------------------------------------------------------------------
 */
void set_ngio_now_testing (struct ngio_intf_t *ngio_ptr)
    __attribute__((weak, alias("__set_ngio_now_testing")));
void __set_ngio_now_testing (struct ngio_intf_t *ngio_ptr)
{
    return; 
}

/*-------------------------------------------------------------------
 *
 * Function : print_pim_slots
 * Description: a weak function for print_pim_slot
 *
 * INPUT:  plug_ptr - pointer to pluggable structure.
 * OUTPUT: None
 * -------------------------------------------------------------------
 */
void print_pim_slots (void)
    __attribute__((weak, alias("__print_pim_slots")));
void __print_pim_slots (void)
{
    return; 
}

/*
 * The test not available message to the user.
 */
int 
slot_notavail (void)
{
    cterr('w',0,"Test is not available for now. Sorry!");
    return(1);
}

static unsigned short
slot_get_id (void *io, char *err)
{
    struct ngio_intf_t *ngio = (struct ngio_intf_t *)io;
    unsigned int slot;
    uint16_t id;
    
    /* FIX ME: there's too many return codes */
    if (ngio->id == 0 || ngio->id == INVALID_ID ||
        ngio->id == FAILED ||
        ngio->id == SLOT_VACCODE || ngio->id == SLOT_ILLCODE) {
        if (ngio->pc) {
            if (ngio->pc->pc) {
                slot = ngio->pc->pc->slot;
            } else {
                slot = ngio->pc->slot;
            }
        } else {
            slot = ngio->slot;
        }
        if (get_cookie_id(slot, ngio->mod_type,
                          ngio->cookie, &id, err)==FAILED) {
            return FAILED;
        }
        ngio->id = id;
        return PASSED;
    }

    /* id has already been read so just return pass */
    return PASSED;
}

/* Curie 2RU: disabling SM PCIe before SM Patriot tests. */
int c2ru_patriot_pcie_init(int slot, int id)
{
    if (!is_curie_2ru())
        return PASSED;

    if (id != 0x775)
        return PASSED;

    printf("\nCurie2RU: Patriot: disable slot %d pcie root port\n", slot);

    if (slot == 1) {
        system("setpci -s 00:1d.2 50.b=10");
    } else {
        system("setpci -s 00:1d.0 50.b=10");
    }

    system("echo 1 > /sys/bus/pci/rescan");

    return PASSED;
}

/* Curie 2RU: enable SM PCIe after Patriot tests */
void c2ru_patriot_pcie_exit(int slot, int id)
{
    if (!is_curie_2ru())
        return;

    if (id != 0x775)
        return;

    if (slot == 1) {
        system("setpci -s 00:1d.2 50.b=0");
    } else {
        system("setpci -s 00:1d.0 50.b=0");
    }

    printf("\nCurie2RU: Patriot: restore slot %d pcie root port\n", slot);

    system("echo 1 > /sys/bus/pci/rescan");
}

/* Curie 2RU: Manually remove BCM57412 on SM test card before powering off */
void c2ru_sm_bcm57412_pcie_exit(int slot, int id)
{
    if (!is_curie_2ru())
        return;

    if (id != SM_BCM57412_TESTCARD)
        return;

    /* On Curie2RU only slot 1 has a PCIe interface to connect BCM57412 on SM */
    if (slot == 1) {
        system("echo 1 > /sys/devices/pci0000:5e/0000:5e:02.0/0000:61:00.0/remove");
        system("echo 1 > /sys/devices/pci0000:5e/0000:5e:02.0/0000:61:00.1/remove");
    }
}

void curie_1ru_sm_bcm57412_pcie_exit(int slot, int id)
{
    char buf[128];
    FILE *fp;
    char get_pcie_bus[50] = {0};
    char rm_pcie_bus[50] = {0};
    char *result_file = "/sm_bcm57412_pcie.txt";
    int ix = 0;
    char *ptr;

    if (!is_curie_1ru())
        return;

    if (id != SM_BCM57412_TESTCARD)
        return;

    /* On Curie1RU only slot 1 has a PCIe interface to connect BCM57412 on SM */
    if (slot == 1) {
        for (ix = 10; ix <= 11; ix++) {
            sprintf(get_pcie_bus, "ethtool -i eth%d | grep bus-info | cut -c 11- > /sm_bcm57412_pcie.txt", ix);
            system(get_pcie_bus);

            fp = fopen(result_file, "r");
            if (fp == NULL) {
                goto exit_f;
            } else {
                /* To remove newline */
                fgets(buf, sizeof(buf), fp);
                ptr = strchr(buf, '\n');
                if (ptr) {
                    * ptr = '\0';
                }

                sprintf(rm_pcie_bus, "echo 1 > /sys/bus/pci/devices/%s/remove", buf);
                system(rm_pcie_bus);
                fclose(fp);
                goto exit_s;
            }
exit_f:
            printf("\n***Unable to open /sm_bcm57412_pcie.txt\n");
exit_s:
            printf("\nRemove 10G_SM BCM57412 ETH%d PCIe bus successfully\n", ix);
        }
    }
}

/*
 * testslot - invoke the diagnostic for the slot
 * A given slot number >= than the max number of slots indicates
 * that the user has opted to see a subtest menu for selection of a
 * particular subtest (e.g., test just one of the interfaces on the PM).
 * Pass such a coded slot number on to the invoked diagnostic.
 */  
static int
slot_test (struct ngio_intf_t *ngio, int real_slot, int slot, int test_type,
              char *mod_str)
{
    int  test_err;

    assert(ngio);
    
    if (real_slot != slot) /* User opted for submenus */
	ngio->menu_display = TRUE;
    else
        ngio->menu_display = FALSE;

    /* Clear the test's cterr info setup before each slot test */
    reset_errmsg_var();
    testname("%s Slot %1d", mod_str,  real_slot);
    prpass(testpass, " "); /* Zero out the teatpass buffer */

    if (slot_get_info(ngio, mod_str) == FAILED) {
        prcomplete(testpass, errcount, 0);
        return (FAILED);
    }

    /* Curie 2RU: HW suggested diabling SM PCIe before Patriot tests.
     * Their PCIe clocks are not compatible between Skylake-D and Patriot */
    c2ru_patriot_pcie_init(real_slot, ngio->id);

    /* invoke the diagnostics */

    ngio->test_type = test_type;

    if (test_type == FULL_TEST) {
        test_err = ngio->diag((void *)ngio);
    }  else {
        if (ngio->intf_diag)
            test_err = ngio->intf_diag((void *)ngio);
        else {
            test_err = PASSED;
            cterr('w', 0, "No interface test available.");
        }
    }

    if (slot != real_slot) {
        printf("\n%s Subtest Menu accumulated errors = %ld",
               ngio->name, err_accum);
    }

    if (slot == real_slot) {
	if (test_err == PASSED) { /* only run Authenticate if the test PASSED */
	    if (diagflag_xram & D_XEC_AUTH) { /* For MFG */
		smartchip_authenticate_retest(ngio->mod_type, real_slot);
            }
#ifdef AUTHENTICATION_TEST_Y    
	    if (diagflag_yram & D_AUTH_Y) { /* For EDVT with retry */
		smartchip_authenticate(ngio->mod_type, real_slot);
            }
#endif

	}
	prcomplete(testpass, errcount, 0);
    }

    if (test_err == FAILED) {
        return test_err;
    } 
    
    if (!((NVRAM)->diagflag & D_POWER_ON)) {
        if (ngio->off) {
            c2ru_sm_bcm57412_pcie_exit(real_slot, ngio->id);
            curie_1ru_sm_bcm57412_pcie_exit(real_slot, ngio->id);
            ngio->off(ngio);
            c2ru_patriot_pcie_exit(real_slot, ngio->id);
        }

        /* workaround from curie2ru ROMMON v3.0
         * NIM2 test card PCI info still exists at first read after power down,
         * add pci rescan to avoid subsequent SM PCI test failure
         */
        if (ngio->mod_type == WIC_MODULE && real_slot == 2) {
            if (is_curie_2ru() && ngio->off) {
                if (ngio->id == NIM_10GKR_TESTCARD)
                    system("echo 1 > /sys/bus/pci/rescan");
            }
        }
    } 

    return test_err;
}

/*-----------------------------------------------------------------------------
 *
 * Function get_board_revision
 *
 * This function will return the REVISION number of the HWIC module.
 *
 * Inputs : hwic_num - HWIC Slot Number.
 *          eeprom_data - pointer to eeprom data.
 *
 * Returns : revision number of board.
 */
static int
slot_get_bd_revision (uchar *eeprom_data, unsigned short *board_rev)
{
    uchar  *data_ptr;
    uchar  num_byte;
    
    /* for polling slots, do not print warning. simply print the content */
    if ((uchar)eeprom_data[0] == CURRENT_FORMAT_VERSION) {
	if ((data_ptr = (uchar *)search_type_ret_addr_of_first_data
	     ((uchar *)eeprom_data, BOARD_REV, &num_byte, FALSE)) == NULL) {
	    *board_rev =  0xffff;
	} else {
	    *board_rev = *data_ptr++;
	    *board_rev = *board_rev << 8 | *data_ptr;
	}
    } else { 
	/* Get Board Revision Number from WIC EEPROM located at 0x10. */
	*board_rev =  (ushort)(*(eeprom_data + 0x10));
    }

    return(PASSED);
}


/*-----------------------------------------------------------------------------
 *
 * Function get_pcb_serial
 *
 * This function will return the SERIAL number of the HWIC module.
 *
 * Inputs : hwic_num - HWIC Slot Number.
 *          eeprom_data - pointer to eeprom data.
 *
 * Returns : serial number of board.
 */
static int
slot_get_pcb_serial (uchar *eeprom_data, char *serial)
{
    uchar *data_ptr;
    uchar num_byte;

    if ((uchar)eeprom_data[0] == CURRENT_FORMAT_VERSION) {
	/* for polling slots, do not print warning. simply print the content */
	if ((data_ptr = (uchar *)search_type_ret_addr_of_first_data
	     ((uchar *)eeprom_data, PCB_SERIAL_NUM, &num_byte, FALSE)) == NULL) {
            sprintf(serial, "NO PCB NUM");
	} else {
            memcpy(serial, data_ptr, num_byte);
	}
	return(0);
    } else {
	/* Get PCB Serial Number from WIC EEPROM located at 0x4 to 0x7 */
	return(*(int *)(eeprom_data + 0x4));
    }
}

int
slot_i2c_unreset (struct ngio_intf_t *ngio, int slot, char *type)
{
    int err;

    /* need to check for slot presense because this routine is also called by cookie
       utility */
    if (!ngio->is_present(ngio)) {
        printf("Vacant slot");
        return ERROR;
    }

    if ((err=ngio->on(ngio)) < 0) {
        cterr('f', 0, "Unable to power module %s%d\n", type, slot);
        return FAILED;
    }

    if ((err=ngio->i2c_unreset(ngio)) < 0) {
        cterr('f', 0, "Unable to unreset i2c on %s%d\n", type, slot);
        return FAILED;
    }
   
    return PASSED;
}

/*-----------------------------------------------------------------------------
 *
 * Function check if NGIO is 10G-KR capable and uses 16-bit GPIO expander 
 *
 * This function will return the value of kR support and 
 * 16-bit GPIO fileds in module cookie 
 *
 * Inputs : kr_support - pointer to the byte to store the KR support field.
 *            if kr_support = 0x1, the module eth0 support 10G-KR
 *            if kr_support = 0x3, the module eth0 and eth1 support 10G-KR 
 *          eeprom_data - pointer to eeprom data.
 *          use_16bitgpio - pointer to the 16-bit GPIO byte
 *             	0x00 : Module uses a GPIO-8 device
 *              0x01 : Module uses a GPIO-16 device
 * Returns : PASSED 
 */
static int
slot_get_10gkr_capable (uchar *eeprom_data, uchar *kr_support, 
                        uchar *use_16bitgpio)
{
    uchar  *data_ptr;
    uchar  num_byte;
    
    if ((uchar)eeprom_data[0] == CURRENT_FORMAT_VERSION) {
	if ((data_ptr = (uchar *)search_type_ret_addr_of_first_data
	     ((uchar *)eeprom_data, DEVICE_VALUES, &num_byte, FALSE)) == NULL) {
	    *kr_support =  0x0;
	} else {
            /* KR support byte is the 6th byte of device values in the cookie */
	    *kr_support = *(data_ptr+5);
            /* 16-bit GPIO byte is the 7th byte of device values in the cookie */
	    *use_16bitgpio = *(data_ptr+6);
	}
    } else { 
	*kr_support =  0x0;
    }

    return(PASSED);
}
  
/**********************************************************************
 * Function: set_gpio_kr_capable_pins.
 *
 * Description: This function will set the KR capable pins of the
 *              PCA9557 I2C device (GPIO expander) if host is  
 *              KR capable
 * Input:  ngio interface data structure 
 *
 * Output: PASSED/FAILED.
 **********************************************************************
 */
static int
set_gpio_10gkr_capable_pins (struct ngio_intf_t *ngio, uchar bit_mask)
{
    uchar data;
    n2g_i2c_if_t *pca;
    n2g_i2c_if_t pca_i2c;

    /* init pca for 9555 */
    pca_init_i2c((void *)&pca_i2c);
    pca_i2c.dev_name = "PCA9555";
    pca_i2c.i2c_ctrl = ngio->i2c_ctrl;

    /* assign i2c address of 16-bit gpio expander */
    if (ngio->mod_type == SM_MODULE) {
        pca_i2c.i2c_dev = SM_I2C_ADDR_IO_PORT;
    } else {
        pca_i2c.i2c_dev = NGWIC_I2C_ADDR_IO_PORT_16b;
    }

    if(ngio->mod_type == DAUGHTER_CARD) {
        pca_i2c.i2c_bus_type = ((n2g_i2c_if_t *)(ngio->pca))->i2c_bus_type ;
        pca_i2c.i2c_base= ((n2g_i2c_if_t *)(ngio->pca))->i2c_base;
    }

    /* adjust the dev addr for the dc on thule card 
     * the i2c address will use the wic */ 
    if (ngio->id == NGSM_THULE) {
        pca_i2c.i2c_dev = NGWIC_I2C_ADDR_IO_PORT_16b;
    }

    pca = &pca_i2c; /* 16 bit */
    
    /* 16bit GPIO */
    /* Set IO bit 8, 9 output, ie. IO bit 1, 2 of Port 1 */
    if (io_port_8bit_i2c_read(pca, CONFIGURATION_P1_REG, &data, TRUE) == FAILED) {
        cterr('f', 0, "Unable to read PCA9555 register @ 0x07\n");
        return (FAILED);
    }
    
    data &= ~(BIT0 | BIT1 );

    if (io_port_8bit_i2c_write(pca, CONFIGURATION_P1_REG, &data) == FAILED) {
        cterr('f', 0, "Unable to write PCA9555 register @ 0x07\n");
        return (FAILED);
    }

    /* write IO bit 8, 9 to indicate whether host E0, E1 are 10G-KR capable*/
    if (io_port_8bit_i2c_read(pca, OUTPUT_PORT1_REG, &data, TRUE) == FAILED) {
        cterr('f', 0, "Unable to read PCA9555 register @ 0x03\n");
        return (FAILED);
    }
    
    /* clear IO bit 8, 9(bit 0, 1 of port1) first */
    data &= 0xfc; 
    data |= bit_mask;

    if (io_port_8bit_i2c_write(pca, OUTPUT_PORT1_REG, &data) == FAILED) {
        cterr('f', 0, "Unable to write PCA9555 register @ 0x03\n");
        return (FAILED);
    }

    return PASSED;
}

/*-----------------------------------------------------------------------
 *
 * Function: get_slot_info
 *  
 * This functions prints cookie information on SM module.
 *
 * Input : none
 * 
 * Output: none.
 *
 *------------------------------------------------------------------------
 */
int
slot_get_info (struct ngio_intf_t *ngio, char *type)
{
    int slot, index, rv;
    int retry = 0;
    int status = PASSED;
    char err[80], pid[80] = {0};
    struct module_info *p;

    if (!ngio)
        return PASSED;
        
    /* 
     * to leave a blank line between network modules
     * polling print and this polling print
     */
    printf("\n");

    slot = ngio->slot;

    if (slot < FIRST_SLOT) {
        assert(!" invalid slot number in ngio_intf_t structure");
    }

    /* used for platform to distinguish which ngio is testing */
    set_ngio_now_testing(ngio); 

    if (!ngio->is_present((void *)ngio)) {
        printf("%s Slot%1d: (slot vacant)\n", type, slot);
        ngio->id = SLOT_VACCODE;
        sprintf((char *)ngio->name, "(slot vacant)");
        return FAILED;
    } else {
        sprintf((char *)ngio->name, "%s Slot%1d: id=?\n", type, slot);
    }

    for (retry = 0, *err = '\0'; retry < 3; retry++ ) {
        if (slot_i2c_unreset(ngio, slot, type)==FAILED) {
            return FAILED;
        }
        /* call slot_get_xx_id */
        if ((status = ngio->get_id((void *)ngio, err))==FAILED) {
            if (ngio->off)
                ngio->off(ngio);
            sleep(2);
            continue;
        }
        break;
    }

    if (status == FAILED) {
        cterr('f', 0, "unable to read cookie: %s", err);
        return FAILED;
    }
    
    if (ngio->id == INVALID_ID) {
        cterr('f', 0, "NGIO %s%d: invalid cookie %#x. Is id programmed ?",
                  type, slot, ngio->id);
        return (FAILED);
    }
    slot_get_bd_revision(ngio->cookie, &ngio->bd_rev);
    slot_get_pcb_serial(ngio->cookie, ngio->serial_num);

    p = (struct module_info *)get_platform_slot_table(&index, ngio->id);
    if (!p) {
        cterr('f', 0, "slot %d. Card not supported. cookie id = %#x.", slot, ngio->id);
        return FAILED;
    } 

    if (p->early_unreset == TRUE) {
        if ((rv=ngio->unreset(ngio)) < 0) {
            cterr('f', 0, "Unable to unreset on %s%d\n", type, slot);
            return FAILED;
        }
        /* story: without this line, on pull slot, 
         * overdrive will show Link training error on thule case; 
         * new testcard also has trouble. 
         * msleep(100) is not enought to resolve this issue; 
         * msleep(150) is good.
         */
         msleep(150);
    }
    ngio->diag = p->diag;
    ngio->intf_diag= p->intf_diag;

    /* When using share controller ID, read product ID from cookie 
     * instead of use default poduct ID (CSCvg14997) */
    if (strcmp(p->name, BOARD_SHARE_ID) == 0) {
        /* get pid for name*/
        get_slot_bd_pid(ngio->cookie, pid); 
        sprintf((char *)ngio->name, pid);
    } else if (p->name == NULL) {
        cterr('f', 0, "Product name is NULL\n");
        return (FAILED);
    } else {
        sprintf((char *)ngio->name, (char *)p->name);
    }

    return PASSED;
}

void
print_ngio_slots (struct ngio_intf_t *ngio, char *mod_type)
{

    if (!ngio)
        return;
    /* 
     * to leave a blank line between network modules
     * polling print and this polling print
     */
    printf("\n");

    printf("%s %s", mod_type,  ngio->name);
    if (ngio->id == INVALID_ID) {
        printf("\n");
    } else { 
        printf(", ID=%#.4x, ", ngio->id );
        printf("SN=%s, ", ngio->serial_num);
        if (ngio->bd_rev == 0xffff)
            printf("Rev=(no revision found)\n");
        else
            printf("Rev=%c%c", ngio->bd_rev>>8,
                   ngio->bd_rev&0xff);
        
    }
    printf("\n");
}


int
print_slots (struct ngio_intf_t *ngio, char *ngio_str)
{
    clrtestname();
    if (slot_get_info(ngio, (char *)ngio_str)==FAILED) {
        return FAILED;
    }
    if (ngio->id != SLOT_VACCODE) {
        print_ngio_slots(ngio, ngio_str);
    }

    return(PASSED);
}

int
print_dc_slots (struct ngio_intf_t *ngio, char *ngio_str)
{
    int status, index;
    n2g_i2c_if_t *pca_tmp;
    struct module_info *p;   

    p = (struct module_info *)get_platform_slot_table(&index, ngio->pc->id);
    if (!p) {
        printf("Card not supported with cookie id = %#x.", ngio->pc->id);
        return FAILED;
    }

    /* a special case when SM is Switzer-Carrier */
    if ((p->mod_info_flags) & MOD_INFO_SM_IS_CARRIER) {
         goto NOT_SUPPORT_DC;
    }
    /* a special case for thule, its pca addr is nim instead of sm */
    else if ((p->mod_info_flags) & MOD_INFO_DC_IS_NIM){
        ngio->is_present = ngiosm_present;
        ngio->unreset = ngiosm_unreset;
        pca_tmp = (n2g_i2c_if_t *)ngio->pca;
        pca_tmp->i2c_dev = NGWIC_I2C_ADDR_IO_PORT;

    } else if ((p->mod_info_flags) & MOD_INFO_DC_IS_VM){
        /* WIC_DC instead of SM_DC */
        ngio->mod_type = WIC_DAUGHTER_CARD;
    } else {
         goto NOT_SUPPORT_DC; 
    }

    status = ngio->is_present(ngio);

    if (status < 0) {
        printf("%s: problem with expander. Unable to probe daughter card.\n",
               ngio_str);
        return(status);
    }

    if (status) {
        if (slot_get_info(ngio, ngio_str)==FAILED) {
            return(FAILED);
        }
        print_ngio_slots(ngio, ngio_str);
    } else {
NOT_SUPPORT_DC:
        printf("%s (slot vacant)\n", ngio_str);
        ngio->id = SLOT_VACCODE;
        sprintf(ngio->name, "(slot vacant)");
        return(FAILED);
    }            

    return(PASSED);
}

int
wic_test (int slot)
{
    int real_slot = get_wic_real_slot(slot);

    assert(real_slot < MAX_WIC + FIRST_SLOT);
    
    return (slot_test(&wic[real_slot], real_slot, slot, FULL_TEST,
                          "WIC"));
}

int
sm_test (int slot)
{
    int real_slot = get_sm_real_slot(slot);

    assert(real_slot < MAX_SM + FIRST_SLOT);
    
    return (slot_test(&sm[real_slot], real_slot, slot, FULL_TEST,
                          "SM"));
}

int
vm_test (int slot)
{
    int real_slot = get_vm_real_slot(slot);

    assert(real_slot < MAX_VM + FIRST_SLOT);

    return (slot_test(&vm[real_slot], real_slot, slot, FULL_TEST,
                          "VM"));

}
/* *dc_test: entry point for daughter card test:
 *  input: void *p: pionter to module where daughter card is inserted
 *         slot:  daughter card slot number. 
 *  output: passes if test is successfaul; failed otherwise
 *
 **
 */
int
dc_test (void *p, unsigned int slot)
{
    struct ngio_intf_t *parent = (struct ngio_intf_t *)p;

    int real_slot = get_dc_real_slot(slot);

    assert(real_slot < MAX_DC + FIRST_SLOT);
    assert(parent->dc);

    
    return (slot_test(parent->dc, real_slot, slot, FULL_TEST,
                          "DC"));

}

int
carrier_wic_test (void *p, int real_slot, int slot, int test_type)
{
    struct ngio_intf_t *ngio = (struct ngio_intf_t *)p;
    return(slot_test(ngio, real_slot, slot, test_type, "Thule WIC"));
}

/**********************************************************************
 *
 * Function: sm_iface_test (void)
 *
 * Input : none.
 *
 * Output: none.
 *
 **********************************************************************
 */
type_t
sm_iface_test (void)
{
    int slotnum, save_err_accum, err_count;
    int result, val;
    char ngio_str[32];

    /* flush error message buffer */
    flush_test_progress_buf();

    save_err_accum = err_accum;
    result = PASSED;

    for (slotnum = FIRST_SLOT; slotnum <= get_max_sm_slots(); slotnum++) {
        sprintf(ngio_str, "(slot %d) SM", slotnum);
        val = print_slots(&sm[slotnum], ngio_str);
        if (val == FAILED) {
            result += val;
            continue;
        }

        if (sm[slotnum].id != SLOT_VACCODE && sm[slotnum].id ) {
            sprintf(ngio_str, "SM");
            //            result += test_intf(&sm[slotnum], slotnum, ngio_str);
            result += slot_test(&sm[slotnum], slotnum, slotnum, IFACE_TEST, ngio_str);

        }
    }
    err_count = err_accum - save_err_accum;
    printf("\nAll SM slots Interface test, total errors in this pass, ... "
	   "%d errors", err_count);

    return(result);
}

/**********************************************************************
 *
 * Function: wic_iface_test (void)
 *
 * Input : none.
 *
 * Output: none.
 *
 **********************************************************************
 */
type_t
wic_iface_test (void)
{
    int slotnum, save_err_accum, err_count;
    int result, val;
    char ngio_str[32];

    /* flush error message buffer */
    flush_test_progress_buf();

    save_err_accum = err_accum;
    result = PASSED;

    for (slotnum = FIRST_SLOT; slotnum <= get_max_wic_slots(); slotnum++) {
        sprintf(ngio_str, "(slot %d) WIC", slotnum);
        val = print_slots(&wic[slotnum], ngio_str);
        if (val == FAILED) {
            result += val;
            continue;
        }
        if (wic[slotnum].id != SLOT_VACCODE && wic[slotnum].id ) {
            sprintf(ngio_str, "WIC");
            result += slot_test(&wic[slotnum], slotnum, slotnum, IFACE_TEST, ngio_str);
            //            result += test_intf(&wic[slotnum], slotnum, ngio_str);
        }
    }

    err_count = err_accum - save_err_accum;
    printf("\nAll WIC Interface test, total errors in this pass, ... "
	   "%d errors", err_count);

    return(result);
}

/**********************************************************************
 *
 * Function: dc_iface_test ()
 *
 * Input : p - pointer to the parent ngio struct.
 *         slot - DC slot
 *
 * Output: none.
 *
 **********************************************************************
 */
type_t
dc_iface_test (void *p, unsigned int slot)
{
    struct ngio_intf_t *parent = (struct ngio_intf_t *)p;
    int save_err_accum, err_count;
    int result, val;
    char ngio_str[32];
    
    assert(parent->dc);
    /* flush error message buffer */
    flush_test_progress_buf();
    
    save_err_accum = err_accum;
    result = PASSED; 
    
    if (parent->mod_type == WIC_MODULE)
        sprintf(ngio_str, "WIC%d Daughtercard", parent->slot);
    else
        sprintf(ngio_str, "SM%d Daughtercard", parent->slot);
    val = print_dc_slots(parent->dc, ngio_str);
    if (val == FAILED) {
        result += val; 
    } else if (parent->dc->id != SLOT_VACCODE && parent->dc->id ) {
        sprintf(ngio_str, "DC");
        result += slot_test(parent->dc, FIRST_SLOT, FIRST_SLOT, IFACE_TEST, 
                            ngio_str);
    
        err_count = err_accum - save_err_accum;
        if (parent->mod_type == WIC_MODULE) {
            printf("\nWIC%d DC Interface test, total errors in this "
                   "pass, ... %d errors", parent->slot, err_count);
        } else {
            printf("\nSM%d DC Interface test, total errors in this "
                   "pass, ... %d errors", parent->slot, err_count);
        }
    }
    return(result);
}


/**********************************************************************
 *
 * Function: vm_iface_test (void)
 *
 * Input : none.
 *
 * Output: none.
 *
 **********************************************************************
 */
type_t
vm_iface_test (void)
{
    int slotnum, save_err_accum, err_count;
    int result, val;
    char ngio_str[32];
    //    print_sm_slots(ngio);
    //    return PASSED;
    /* flush error message buffer */
    flush_test_progress_buf();

    save_err_accum = err_accum;
    result = PASSED;

    for (slotnum = FIRST_SLOT; slotnum <= get_max_vm_slots(); slotnum++) {
        sprintf(ngio_str, "VM");
        val = print_slots(&vm[slotnum], ngio_str);
        if (val == FAILED) {
            result += val;
            continue; 
        }
        if (vm[slotnum].id != SLOT_VACCODE) {
            sprintf(ngio_str, "VM");
            //            result += test_intf(&vm[slotnum], slotnum, ngio_str);
            result += slot_test(&vm[slotnum], slotnum, slotnum, IFACE_TEST, ngio_str);
        }
    }

    err_count = err_accum - save_err_accum;
    printf("\nAll VM Interface test, total errors in this pass, ... "
	   "%d errors", err_count);

    return(result);
}

struct ngio_intf_t *
slot_get_ngiosm (int slot)
{
    if (slot < FIRST_SLOT || slot > MAX_SM) {
        assert(!" invlid slot in slot_get_ngiosm\n");
    }
    return &sm[slot];
}

struct ngio_intf_t *
slot_get_ngiowic (int slot)
{
    if (slot < FIRST_SLOT || slot > MAX_WIC) {
        assert(!" invlid slot in slot_get_ngiowic\n");
    }
    if (slot < FIRST_SLOT) {

    }
    return &wic[slot];
}

struct ngio_intf_t *
slot_get_ngiovm (int slot)
{
    if (slot < FIRST_SLOT || slot > MAX_VM) {
        assert(!" invlid slot in slot_get_ngiovm\n");
    }

    return &vm[slot];
}

static int
slot_wic_enable_uart (void *p)
{
    struct ngio_intf_t *ngio = (struct ngio_intf_t *)p;
    ngiowic_enable_uart(ngio);

    return (PASSED);
}

static int
slot_sm_enable_uart (void *p)
{
    struct ngio_intf_t *ngio = (struct ngio_intf_t *)p;
    ngiosm_enable_uart(ngio);

    return (PASSED);
}

static int
slot_vm_enable_uart (void *p)
{
    struct ngio_intf_t *ngio = (struct ngio_intf_t *)p;
    ngiovm_enable_uart(ngio);

    return (PASSED);
}

static int
slot_dc_dummy (void *p)
{

    return PASSED;
}

static int
slot_wic_disable_uart (void *p)
{
    struct ngio_intf_t *ngio = (struct ngio_intf_t *)p;

    assert(p);
    assert(ngio->slot);
    ngiowic_disable_uart(ngio);

    return (PASSED);
}

static int
slot_sm_disable_uart (void *p)
{
    struct ngio_intf_t *ngio = (struct ngio_intf_t *)p;
    assert(p);
    assert(ngio->slot);
    
    ngiosm_disable_uart(ngio);

    return (PASSED);
}

static int
slot_vm_disable_uart (void *p)
{
    struct ngio_intf_t *ngio = (struct ngio_intf_t *)p;

    assert(p);
    assert(ngio->slot);

    /* do nothing */
    ngiovm_disable_uart(ngio);

    return (PASSED);
}

void
slot_init_dc (struct ngio_intf_t * p, unsigned int mod_type)
{
    n2g_i2c_if_t *pca;
    
    if (p->dc != NULL)
        return;
    
    p->dc = (struct ngio_intf_t *)malloc(sizeof(struct ngio_intf_t));
    memset(p->dc, 0, sizeof(struct ngio_intf_t));
    
    pca = (n2g_i2c_if_t *)malloc(sizeof(n2g_i2c_if_t));
    memset(pca, 0, sizeof(n2g_i2c_if_t));

    pca_init_i2c((void *)pca);
    pca->i2c_ctrl = p->i2c_ctrl;
    switch (mod_type) {
    case SM_DAUGHTER_CARD:
        pca->i2c_dev = SM_I2C_ADDR_IO_PORT1;
        break;
    case WIC_DAUGHTER_CARD:
        pca->i2c_dev = NGWIC_I2C_ADDR_IO_PORT;
        break;
    default:
        break;
    }
    pca->buf = (void *)malloc(COOKIE_SIZE_512);
    memset(pca->buf, 0, COOKIE_SIZE_512);

    p->pca = (void *)pca;
    
    assert(p->dc);

    p->dc->pc = p;
    p->dc->dc = NULL;
    
    p->dc->i2c_ctrl = p->i2c_ctrl;
    p->dc->uart_ctrl = p->uart_ctrl;
    p->dc->mod_type = mod_type;
    p->dc->slot = FIRST_SLOT;
    p->dc->get_id = slot_get_id;
    p->dc->on  = slot_dc_dummy;
    p->dc->off = ngiodc_disable;
    p->dc->i2c_unreset  = slot_dc_dummy;

    /* module code need to override these */
    p->dc->is_present = ngiodc_present;
    p->dc->reset = ngiodc_reset;
    p->dc->unreset = ngiodc_unreset;
    p->dc->uart_on = ngiodc_enable_uart;
    p->dc->uart_off = ngiodc_disable_uart;
    p->dc->pci_rdy = NULL;

}

int
print_all_slots (int dummy)
{
    int slotnum;
    char ngio_str[32];
    
    for (slotnum = FIRST_SLOT; slotnum <= get_max_sm_slots(); slotnum++) {
        sprintf(ngio_str, "SM%d:", slotnum);
        print_slots(&sm[slotnum], ngio_str);
        if (sm[slotnum].id != SLOT_VACCODE) {
            sprintf(ngio_str, "SM%d Daughtercard:", slotnum);
            print_dc_slots(sm[slotnum].dc,  ngio_str);
            if (sm[slotnum].dc->id != SLOT_VACCODE) { 
                 sprintf(ngio_str, "SM%d DC of DC:", slotnum);
                 print_dc_slots(sm[slotnum].dc->dc,  ngio_str);
            }
        }
        /* there is a flag to keep ngio power on */
        if (!((NVRAM)->diagflag & D_POWER_ON)) {
            /* some of ngio want to point ngio->off to NULL;
             * they don't want to power off ngio module to get trouble
             * e.g. the SATA cannot be link up once overdrive is powered off */
            if (sm[slotnum].off) {
                ngiosm_disable(&sm[slotnum]);
            }
        }
    }

    for (slotnum = FIRST_SLOT; slotnum <= get_max_wic_slots(); slotnum++) {
        sprintf(ngio_str, "WIC%d:", slotnum);
        print_slots(&wic[slotnum], ngio_str);
        if (wic[slotnum].id != SLOT_VACCODE) {
            sprintf(ngio_str, "WIC%d Daughtercard:", slotnum);
            print_dc_slots(wic[slotnum].dc,  ngio_str);
        }
        if (!((NVRAM)->diagflag & D_POWER_ON)) {
            if (wic[slotnum].off) {
                ngiowic_disable(&wic[slotnum]);
            }
        }
    }

    for (slotnum = FIRST_SLOT; slotnum <= get_max_vm_slots(); slotnum++) {
        sprintf(ngio_str, "VM%d", slotnum);
        print_slots(&vm[slotnum], ngio_str);
        if (!((NVRAM)->diagflag & D_POWER_ON)) {
            if (vm[slotnum].off) {
                ngiovm_disable(&vm[slotnum]);
            }
        }
    }
    
    print_pim_slots();

    return PASSED;
}
/* must be called first */
void
init_slot_info (void)
{
    int i;
    for (i=FIRST_SLOT;i<MAX_WIC+FIRST_SLOT;i++) {
        memset(&wic[i], 0, sizeof(struct ngio_intf_t));
        wic[i].slot = i;
        wic[i].is_present = ngiowic_present;
        wic[i].on = ngiowic_enable;
        wic[i].off = ngiowic_disable;
        wic[i].i2c_reset = ngiowic_i2c_reset;
        wic[i].i2c_unreset = ngiowic_i2c_unreset;
        wic[i].reset = ngiowic_reset;
        wic[i].unreset = ngiowic_unreset;
        wic[i].uart_on = slot_wic_enable_uart;
        wic[i].uart_off = slot_wic_disable_uart;
        wic[i].uart_ctrl = get_wic_uart_ctrl(i);
        wic[i].get_id = slot_get_id;
        wic[i].i2c_ctrl = get_wic_i2c_ctrl(i); 
        wic[i].oir = platform_get_wic_oir(i);
        wic[i].mod_type = WIC_MODULE;
        wic[i].pci_rdy = ngiowic_pci_rdy;
        wic[i].pc = NULL;
        wic[i].dc = NULL;
        slot_init_dc(&wic[i], WIC_DAUGHTER_CARD);
    }

    for (i=FIRST_SLOT;i<MAX_SM+FIRST_SLOT;i++) {
        memset(&sm[i], 0, sizeof(struct ngio_intf_t));
        sm[i].slot = i;
        sm[i].is_present = ngiosm_present;
        sm[i].on = ngiosm_enable;
        sm[i].off = ngiosm_disable;
        sm[i].i2c_reset = ngiosm_i2c_reset;
        sm[i].i2c_unreset = ngiosm_i2c_unreset;
        sm[i].reset = ngiosm_reset;
        sm[i].unreset = ngiosm_unreset;
        sm[i].pci_rdy = ngiosm_pci_rdy;
        sm[i].uart_on = slot_sm_enable_uart;
        sm[i].uart_off = slot_sm_disable_uart;
        sm[i].uart_ctrl = get_sm_uart_ctrl(i);
        sm[i].get_id = slot_get_id;
        sm[i].i2c_ctrl = get_sm_i2c_ctrl(i); 
        sm[i].oir = platform_get_sm_oir(i);
        sm[i].mod_type = SM_MODULE;
        sm[i].pc = NULL;
        sm[i].dc = NULL;
        slot_init_dc(&sm[i], SM_DAUGHTER_CARD);
        slot_init_dc(sm[i].dc, SM_DAUGHTER_CARD);
    }
    for (i=FIRST_SLOT;i<MAX_VM+FIRST_SLOT;i++) {
        memset(&vm[i], 0, sizeof(struct ngio_intf_t));
        vm[i].is_present = ngiovm_present;
        vm[i].slot = i;
        vm[i].on = ngiovm_enable;
        vm[i].off = ngiovm_disable;
        vm[i].i2c_reset = ngiovm_i2c_reset;
        vm[i].i2c_unreset = ngiovm_i2c_unreset;
        vm[i].reset = ngiovm_reset;
        vm[i].unreset = ngiovm_unreset;
        vm[i].uart_on = slot_vm_enable_uart;
        vm[i].uart_off = slot_vm_disable_uart;
        vm[i].uart_ctrl = VM1_UART_CTRL+(i-FIRST_SLOT);
        vm[i].get_id = slot_get_id;
        vm[i].i2c_ctrl = VM_I2C_CTRL+(i-FIRST_SLOT);
        vm[i].oir = platform_get_vm_oir(i);
        vm[i].mod_type = VM_MODULE;
        vm[i].pc = NULL;
        vm[i].dc = NULL;
    }

}

int slot_get_73_part_num (uchar *eeprom_data, uchar *part_num)
{
    uchar *data_ptr;
    uchar num_byte;

    if ((data_ptr = (uchar *)search_type_ret_addr_of_first_data
	(eeprom_data, PART_NUM_73, &num_byte, FALSE)) == NULL) {
        return (FAILED);
    } else {
        memcpy(part_num, data_ptr, 4);
    }

    return (PASSED);
}

int get_cookie_pid_wrap (int slot, int type, uchar *eeprom_data, char *pid)
{
    if (get_cookie_pid (slot, type, eeprom_data, pid) == FAILED) {
        return (FAILED);
    }

    return (PASSED);
}

void
init_carrier_wic(struct ngio_intf_t *p, int slot)
{
    p->slot = slot;
    p->is_present = ngiosm_present;
    p->on = ngiosm_enable;
    p->off = ngiosm_disable;
    p->i2c_reset = ngiosm_i2c_reset;
    p->i2c_unreset = ngiosm_i2c_unreset;
    p->reset = ngiosm_reset;
    p->unreset = ngiosm_unreset;
    p->pci_rdy = ngiosm_pci_rdy;
    p->uart_on = slot_sm_enable_uart;
    p->uart_off = slot_sm_disable_uart;
    p->uart_ctrl = get_sm_uart_ctrl(slot); 
    p->get_id = slot_get_id;
    /* using get_sm_i2c_ctrl instead of assign value directly */
    p->i2c_ctrl = get_sm_i2c_ctrl(slot);
    p->oir = platform_get_carrier_wic_oir(slot);
    p->mod_type = SM_DAUGHTER_CARD;
    p->pc = p->pc; 
    p->dc = p->dc;/* init dc inside the carrier wic */
}

/* clear cookie id field of ngio structure */
void
slot_clear_cookie_id (int type, int slot_num)
{
    switch (type) {
    case SM_MODULE:
        sm[slot_num].id = SLOT_VACCODE;
        break;
    case WIC_MODULE:
        wic[slot_num].id = SLOT_VACCODE;
        break;
    case VM_MODULE:
        vm[slot_num].id = SLOT_VACCODE;
        break;
    default:
        break;
    }
}

/*
 * Check and set 10G-KR mode between host and NGIO
 */  
void ngio_ge_cfg(struct ngio_intf_t *ngio)
{
    uint host_10gkr_cap;
    uchar module_10gkr_capable, module_16b_gpio;
    int ngio_ge_port;
    uchar port_bitmask, gpio_exp_bitmask;

    // printf("pfix-0 %s: NGIO type=%d slot= %d name= %s cookie %#x.\n", __FUNCTION__, ngio->mod_type, ngio->slot, ngio->name, ngio->id);
    /* Check if platform is 10g-kr capable on this IO slot.
     * host_10gkr_cap is bit mask such as NGIO_GE0_BITMASK */
    host_10gkr_cap = host_ngio_10gkr_capability(ngio->mod_type, ngio->slot);
    if (host_10gkr_cap > 0) {
        /* Get the module 10gkr capability from cookie
	 */
        slot_get_10gkr_capable(ngio->cookie, &module_10gkr_capable, &module_16b_gpio);
 	
	for (ngio_ge_port= NGIO_GE0; ngio_ge_port <= NGIO_GE1; ngio_ge_port++) {
	    switch (ngio_ge_port) {
	    case NGIO_GE0:
	        port_bitmask = NGIO_GE0_BITMASK;
		gpio_exp_bitmask = GPIO_EXP_GE0_10GKR_BIT;
		break;
	    case NGIO_GE1:
	        port_bitmask = NGIO_GE1_BITMASK;
		gpio_exp_bitmask = GPIO_EXP_GE1_10GKR_BIT;
		break;
	    }
	    
	    // printf("pfix-1 %s: ngio_ge_port= %d host_10gkr_cap= %#.2x module_10gkr_capable= %#.2x, port_bitmask= %#.2x setup-10g=%s\n", __FUNCTION__, ngio_ge_port, host_10gkr_cap, module_10gkr_capable, port_bitmask,  (host_10gkr_cap & module_10gkr_capable & port_bitmask) ? "yes" : "no");
	    /* If module is 10g-kr capable and uses 16-bit GPIO expander,
	     * config host port and set the 10g-kr field of of GPIO
	     * expander on module to 10G or 1G.
	     */
	    if (host_10gkr_cap & port_bitmask) {
	        if ((module_10gkr_capable & port_bitmask) && (module_16b_gpio == 0x1)) {
		    if (cfg_host_10gkr_port(ngio->mod_type, ngio->slot, ngio_ge_port, 1) == FAIL) {
		        cterr('f', 0, "Host GESW set up 10G-KR port failed.\n");
			return;
		    }
		    set_gpio_10gkr_capable_pins (ngio, gpio_exp_bitmask);
		}
		else {
		    if (cfg_host_10gkr_port(ngio->mod_type, ngio->slot, ngio_ge_port, 0) == FAIL) {
		        cterr('f', 0, "Host GESEW set up 1G port failed.\n");
			return;
		    }
		}
	    }
	}
    }
}

/* end of module */

/******** History ******** 
$Log: slot.c,v $
Revision 1.68  2021/04/12 13:36:29  xiaolaya
*** empty log message ***

Revision 1.67  2020/12/29 03:10:51  leschen
Remove bnxt_en operations.

Revision 1.66  2020/05/22 02:28:23  qingcwan
Merge switzer-carrier code into main chunk.

Revision 1.65  2020/01/09 01:01:52  jiajliu
Merge Curie 2RU to main trunk

Revision 1.64  2019/12/30 08:44:36  alpeng
CSCvs56482 - fix bug for get serial number

Revision 1.63  2019/08/06 06:56:06  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.62.2.2  2018/10/23 08:17:15  meho
Added PIM module in Poll Slot util

Revision 1.62.2.1  2018/10/15 10:47:04  alpeng
add new func set_ngio_now_test() for platform to assign correct host eth port number for ngio

Revision 1.62  2018/05/18 09:24:48  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.61  2017/10/18 01:51:19  olin2
Improve common code for Poll slot (CSCvg14997)

Revision 1.60.18.8  2018/05/17 10:50:20  alpeng
 sync with trunk <trunk-051618>

Revision 1.60.18.7  2017/10/18 07:16:43  alpeng
apply code diff for oakenshield naming fix

Revision 1.60.18.6  2017/01/18 23:31:01  ptong
Fail diag when GESW failed to set 10GKR on NGIO

Revision 1.60.18.5  2016/11/08 04:41:48  alpeng
consider thule case to update uart ctrl num

Revision 1.60.18.4  2016/11/04 05:13:03  alpeng
update uart info to return uart ctrl number on slot.c

Revision 1.60.18.3  2016/11/03 10:17:41  alpeng
update ngio get i2c method flexibly

Revision 1.60.18.2  2016/10/20 22:15:46  alpeng
update thule get i2c bus num for carrier card; update tc get i2c bus mechanism; dash_fpga for bypass plx conflict

Revision 1.60.18.1  2016/10/17 00:19:33  ptong
10G-KR modules now is supported on Neptune

Revision 1.61  2017/10/18 01:51:19  olin2
Improve common code for Poll slot (CSCvg14997)

Revision 1.60  2015/06/05 06:13:11  alpeng
fix poll slot issue

Revision 1.59  2015/03/12 06:05:33  alpeng
imporve poll_slot(), power off ngio after polled

Revision 1.58  2015/02/27 10:02:16  iachang

Add support dreamliner NIM

Revision 1.57  2014/11/26 07:00:42  alpeng
Support NGSM+NGWIC+NGVM case

Revision 1.56  2014/11/26 04:13:12  alpeng
reverting to version 1.54

Revision 1.54  2014/09/06 00:53:21  ptong
Remove ngio_ge_cfg from slot.c and add to ngio_testcard.c and sm_woodlawn.c

Revision 1.53  2014/08/29 10:28:18  danchung
add get_cookie_pid function wrapper

Revision 1.52  2014/08/05 12:05:21  danchung
Add the function to get Product ID from cookie

Revision 1.51  2014/07/03 07:18:34  erwu2
Reset Enhanced Error Message Between Slot Tests

Revision 1.50  2014/07/02 02:52:42  alpeng
support thule card on setting 10gkr pins

Revision 1.49  2014/07/01 09:07:29  bowang3
Add support to NGSM carrier card Thule

Revision 1.48  2014/06/24 22:04:50  ptong
Move ngio_ge_cfg from slot_get_info to slot_test to config 10KR ports

Revision 1.47  2014/06/19 22:00:50  ptong
Incorporated BCM sdk-6.4.1-EA2 for 10G-KR interface. New code works on O2, June, and non Greyhound USD regression

Revision 1.46  2014/05/27 12:19:26  danchung
Improve readability

Revision 1.45  2014/05/23 10:24:02  danchung
check if module is 10gkr capable only when platform also support 10gkr

Revision 1.44  2014/05/06 08:09:39  danchung
Remove useless function and fix typo

Revision 1.43  2014/04/28 11:33:31  danchung
Add related functions for Greyhound 10G-KR bring-up

Revision 1.42  2014/03/06 04:59:45  mcharon
revert back to version 1.39

Revision 1.41  2014/03/03 22:49:56  mcharon
initialize daughter card interface to NULL

Revision 1.40  2014/03/03 22:27:45  mcharon
move daughter card init code to fortitude

Revision 1.39  2014/01/29 01:27:55  mcharon
clear cookie id when insertion/removel event is dectected

Revision 1.38  2013/12/18 06:32:46  hroni
use toolchain in router/bin to do make with TOOLS_VER=c4.5.3-p1, TOOLS_ARCH=x86_64

Revision 1.37  2013/11/26 08:40:34  hroni
fix compiler warning

Revision 1.36  2013/10/07 03:40:13  alpeng
introducing a flag early_unreset for ngio to put reset state on early stage

Revision 1.35  2013/04/23 17:16:42  mcharon
cleanup compile

Revision 1.34  2013/04/16 20:26:31  mcharon
check to make sure ngio->off is not null before invoking it

Revision 1.33  2013/03/29 05:49:32  mcharon
add function ptr pci_rdy to turn on pci ready by

Revision 1.32  2013/02/27 22:14:28  mcharon
no need to call ->reset, since this is called when module is turned off

Revision 1.31  2013/02/07 22:52:17  srane
Support dc_iface_test

Revision 1.30  2012/12/21 18:06:59  mcharon
CSCud84025 fix daughter card segfault when powr off

Revision 1.29  2012/12/20 05:57:29  mcharon
don't turn off module if module fails

Revision 1.28  2012/12/05 01:07:14  mcharon
reset the module after turning off

Revision 1.27  2012/11/21 19:49:49  palin2
Add function to get cookie part number (73).

Revision 1.26  2012/11/17 01:15:17  mcharon
reset i2c device; don't cterr in driver code..propogate err message to slot.c

Revision 1.25  2012/11/12 20:55:28  mcharon
add warning message for VM interface test when slot is vacant

Revision 1.24  2012/11/12 20:35:23  mcharon
add third arg to slot_i2c_unrest to report slot num when fails..improve err reporting

Revision 1.23  2012/11/06 20:39:49  mcharon
add headers/cleanup/remove unneeded functions/files

Revision 1.22  2012/11/02 17:30:14  mcharon
don not call ngio->off when taking i2c out of reset

Revision 1.21  2012/11/02 16:11:10  alpeng
recover the mask ngio->off on function slot_i2c_unreset()

Revision 1.20  2012/11/02 16:06:29  alpeng
support HDD test on overdirve, add some diag item for manufacturing using

Revision 1.19  2012/10/25 18:56:18  mcharon
improve error reporting

Revision 1.18  2012/10/25 06:39:27  mcharon
slot_i2c_unreset should return FAILED fo failure

Revision 1.17  2012/10/25 06:16:37  mcharon
turn on module when exit test

Revision 1.16  2012/10/04 17:27:16  mcharon
set nio->test_type = test_type

Revision 1.15  2012/09/25 01:02:20  mcharon
don't use *** for warning slot vacancy

Revision 1.14  2012/09/19 20:02:38  mcharon
in intf_test, pass in ngio struct instad of slot number to intf_diag

Revision 1.13  2012/09/18 19:19:54  mcharon
support poll slot-fix io intr test; support fpga upgrade; support serial irq intr tst

Revision 1.12  2012/09/13 17:15:17  mcharon
add vm_interface_test

Revision 1.11  2012/09/11 18:41:12  ptong
Replace printf with cterr warning when a slot is vacant

Revision 1.10  2012/08/30 07:44:57  alpeng
infrom user with warning when device is vacant on CLI discovery

Revision 1.9  2012/08/28 18:50:37  srane
Remove printf from vm_test.

Revision 1.8  2012/06/28 21:40:39  srane
Fix DC support in all NGWIC slots and cookie alter for DC.

Revision 1.7  2012/06/20 21:07:49  mcharon
duaghter card should use parent slot numbr for reading ID

Revision 1.6  2012/05/31 14:24:24  palin2
Clean up compile warnings.

Revision 1.5  2012/05/30 19:42:06  mcharon
don't disable uart intr here. this is done in the driver

Revision 1.4  2012/05/05 04:02:20  mcharon
support alter daughter board cookie for wic

Revision 1.3  2012/05/04 20:01:45  mcharon
use void* instead of int as argument to to func ptrs in ngio_intf


$Endlog$
*/

