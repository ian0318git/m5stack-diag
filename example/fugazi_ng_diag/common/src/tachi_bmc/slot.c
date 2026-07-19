/* $Id: slot.c,v 1.8 2017/05/22 06:44:01 haohsu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/slot.c,v $
 *------------------------------------------------------------------
 * slot.c - Interface routines to support SM on xformers. 
 *
 * Copyright (c) 2015-2017 by Cisco Systems, Inc.
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
#include "nvmonvars.h"
#include "error.h"
#include "slot.h"
#include "cookie_4.h"
#include "nmc93c46.h"
#include "platform_slot.h"  /* requires slot.h */
#include "proto.h"
#include "linux_api.h"
#include "cross_platform.h"
#include "i2c_api.h"
#include "smart_cookie.h"
#include "platform_i2c.h"
//#include "platform_cookie.h"
#include "ngio.h"
#include "pca.h"
#include "plat_defs.h" /* needed for extern function is_plat_10gkr_capable() */
#include "diag_fpga_lib.h"
#include "diag_plat_cookie.h"

int slot_get_info(struct ngio_intf_t *ngio, char*);

static struct ngio_intf_t wic[MAX_WIC+FIRST_SLOT];
//static n2g_i2c_if_t pca_i2c;
//static unsigned char pca_buff[256];


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

    /* ngio->off already includes disable NIM PCIe */
    if (!((NVRAM)->diagflag & D_POWER_ON)) {
        if (ngio->off) {
            ngio->off(ngio);
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
 int
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
            memcpy(serial, data_ptr, 12);
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
    char err[80];
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

    if (!ngio->is_present((void *)ngio)) {
        printf("%s Slot%1d: (slot vacant)\n", type, slot);
        ngio->id = SLOT_VACCODE;
        sprintf((char *)ngio->name, "(slot vacant)");
        return FAILED;
    } else {
        sprintf((char *)ngio->name, "%s Slot%1d: id=?\n", type, slot);
    }

if (skip_init_seq == TRUE) {

    if ((status = ngio->get_id((void *)ngio, err))==FAILED) {
         ngio->off(ngio);
    }
    goto __SKIP_INIT; 
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

__SKIP_INIT:
    slot_get_bd_revision(ngio->cookie, &ngio->bd_rev);
    slot_get_pcb_serial(ngio->cookie, ngio->serial_num);

    p = (struct module_info *)get_platform_slot_table(&index, ngio->id);
    if (!p) {
        cterr('f', 0, "slot %d. Card not supported. cookie id = %#x.", slot, ngio->id);
        return FAILED;
    } 
if (skip_init_seq == TRUE) {

    goto __SKIP_INITX;
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

    /* Enable NIM PCIe */
    if (p->mod_info_flags & MOD_INFO_USE_PCIE) {
        /* enable pci rdy pins */
        ngio->pci_rdy(ngio, 1);
    }

__SKIP_INITX:

    ngio->diag = p->diag;
    ngio->intf_diag= p->intf_diag;
    sprintf((char *)ngio->name,(char *)p->name);

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
    struct module_info *p;   

    p = (struct module_info *)get_platform_slot_table(&index, ngio->pc->id);
    if (!p) {
        printf("Card not supported with cookie id = %#x.", ngio->pc->id);
        return FAILED;
    }

    if ((p->mod_info_flags) & MOD_INFO_DC_IS_VM){
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
        /*
         *  Turn off dreamliner power to avoid Power on dreamliner twice
         * */
        if ((wic[slotnum].id == NIM_ES2_8P) || (wic[slotnum].id == NIM_ES2_8) 
                || (wic[slotnum].id == NIM_ES2_4)) {
            if (wic[slotnum].off) {
                wic[slotnum].off(&wic[slotnum]);
            }
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
    
    sprintf(ngio_str, "WIC%d Daughtercard", parent->slot);

    val = print_dc_slots(parent->dc, ngio_str);
    if (val == FAILED) {
        result += val; 
    } else if (parent->dc->id != SLOT_VACCODE && parent->dc->id ) {
        sprintf(ngio_str, "DC");
        result += slot_test(parent->dc, FIRST_SLOT, FIRST_SLOT, IFACE_TEST, 
                            ngio_str);
    
        err_count = err_accum - save_err_accum;
        printf("\nWIC%d DC Interface test, total errors in this "
               "pass, ... %d errors", parent->slot, err_count);
    }
    return(result);
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

static int
slot_wic_enable_uart (void *p)
{
    struct ngio_intf_t *ngio = (struct ngio_intf_t *)p;
    ngiowic_enable_uart(ngio);

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
    pca->i2c_dev = NGWIC_I2C_ADDR_IO_PORT;
    
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

    return PASSED;
}
/* must be called first */
void
init_slot_info (void)
{
    int i;
    /* wic ctrl bus = 10 on tachi entry */
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
        wic[i].uart_ctrl = WIC1_UART_CTRL + (i-FIRST_SLOT);
        wic[i].get_id = slot_get_id;
        wic[i].i2c_ctrl = get_wic_i2c_ctrl(i);
        wic[i].oir = platform_get_wic_oir(i);
        wic[i].mod_type = WIC_MODULE;
        wic[i].pci_rdy = ngiowic_pci_rdy;
        wic[i].pc = NULL;
        wic[i].dc = NULL;
        slot_init_dc(&wic[i], WIC_DAUGHTER_CARD);
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

/* clear cookie id field of ngio structure */
void
slot_clear_cookie_id (int type, int slot_num)
{
    switch (type) {
    case WIC_MODULE:
        wic[slot_num].id = SLOT_VACCODE;
        break;
    default:
        break;
    }
}

/******** History ******** 
$Log: slot.c,v $
Revision 1.8  2017/05/22 06:44:01  haohsu
Modify Quack showing PID and SN number

Revision 1.7  2017/03/30 08:34:08  hondwang
Tachi-L brach merge

Revision 1.6.4.1  2017/03/07 03:28:18  hondwang
Fix dreamliner and wallander IO interface test

Revision 1.6  2016/09/14 07:27:27  jimmyya
change the serdes setting during F35 up

Revision 1.5  2016/07/12 01:53:19  hondwang
Fix F2W bug and add PCAMAP ID

Revision 1.4  2016/06/21 03:02:56  jimmyya
Add setting Lewis serder function

Revision 1.3  2016/06/04 09:22:20  alpeng
initial check in for f2w

Revision 1.2  2016/04/20 11:25:31  benchen2
add tachi fru portion

Revision 1.1.2.5  2015/10/01 09:19:51  alpeng
set pci ready when USE_PCIE flag turn on

Revision 1.1.2.4  2015/09/26 08:00:41  alpeng
add pci rdy during init ngio

Revision 1.1.2.3  2015/09/18 06:58:55  alpeng
using function return nim i2c bus num; set loopback for testcard GE test, send pkt from Lewis

Revision 1.1.2.2  2015/09/17 13:05:10  tirawan
Add Bios Boot Flash Test, fixes I2C controller number for NIM

Revision 1.1.2.1  2015/07/31 10:40:00  alpeng
first check in for testcard


$Endlog$
*/

