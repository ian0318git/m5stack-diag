/* $Id: platform_slot.c,v 1.2 2021/04/15 00:52:27 achiu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/phoenix/platform_slot.c,v $
 *------------------------------------------------------------------
 *
 * platform_slot.c - Platform specific slot support functions.
 *
 * Sept 2008, Shih-Nan Huang ported from Xformers, updated for O2.
 *
 * Copyright (c) 2018-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include "endians.h"
#include "common.h"
#include "types.h"
#include "slot.h"
#include "dev_print.h"
#include "proto.h"
#include "error.h"
#include "ngio.h"
#include "menu.h"
#include "platform_cookie.h"
#include <assert.h>
#include "cookie_4.h"
#include "cross_platform.h"
#include "dash_fpga.h" 
#include "diag_fpga_lib.h"
#include "i2c_api.h"
#include "n2g_api_rc.h"
#include "platform_i2c.h"
#include "ngio.h"
#include "pca.h"
#include "platform_slot.h"
#include <string.h>
#include "plat_defs.h"     

int get_dsp_or_dynamo(void *);

/* all slots start at FIRST_SLOT */
extern int phoenix_vir_test(void *wic);
extern uint32_t ngwic_testcard(void *);
extern int dynamo_test(void *);
extern uint32_t fortitude_test(void *);
static uint8_t sm_uart_ctrl[] = {0,  SM1_UART_CTRL, SM2_UART_CTRL, SM3_UART_CTRL, SM4_UART_CTRL};
static uint8_t wic_uart_ctrl[] = {0,  WIC1_UART_CTRL, WIC2_UART_CTRL, WIC3_UART_CTRL};
unsigned int ngio_testing_now = NOW_TESTING_NONE; 

#define MAX_MOD_IDS (sizeof(port_module_tbl) / sizeof(struct module_info))

/* put your NIM module here */
static struct module_info port_module_tbl[] = {
    /* example */
    //    {.name = "END", .id = SLOT_ILLCODE, .diag = NULL, .intf_diag= NULL, early_unreset, mod_info_flags},
    {.name = BOARD_SHARE_ID,            .id = 0x0BEB,
     .diag = (PFT)get_dsp_or_dynamo,    .intf_diag = (PFT)get_dsp_or_dynamo,
     .early_unreset = 0,            .mod_info_flags = MOD_INFO_NULL},
    {.name = "NGWIC TestCard",          .id = NGWIC_TESTCARD,
     .diag = (PFT)ngwic_testcard,       .intf_diag = (PFT)ngwic_testcard, 
     .early_unreset = 1,                .mod_info_flags = MOD_INFO_NULL},
    {.name = "NIM-10GKR TestCard",      .id = NIM_10GKR_TESTCARD,
     .diag = (PFT)ngwic_testcard,       .intf_diag = (PFT)ngwic_testcard, 
     .early_unreset = 1,                .mod_info_flags = MOD_INFO_USE_PCIE},
    {.name = "NIM Dynamo",              .id = DYNAMO_NIM2,
     .diag = (PFT)dynamo_test,          .intf_diag = (PFT)dynamo_test,
     .early_unreset = 0,                .mod_info_flags = MOD_INFO_NULL},
    {.name = "NGWIC Fortitude",           .id = NGWIC_FORTITUDE,
     .diag = (PFT)fortitude_test, .intf_diag = (PFT)fortitude_test,
     .early_unreset = 0,                .mod_info_flags = MOD_INFO_NULL},
};

static struct module_info ngio_ngnim_dsp_dynamo[] = {
    {.name = "Phoenix Virtual SM",      .id = PHOENIX_VIRTUAL_SM,
     .diag = (PFT)phoenix_vir_test,     .intf_diag = (PFT)phoenix_vir_test,
     .early_unreset = 0,                .mod_info_flags = MOD_INFO_NULL},
    {.name = "NIM Dynamo",              .id = DYNAMO_NIM1,
     .diag = (PFT)dynamo_test,          .intf_diag = (PFT)dynamo_test,
     .early_unreset = 0,                .mod_info_flags = MOD_INFO_NULL},
};

/*
 * Function:	slot_start_with
 * Description:	This function return the number of the first Slot.
 * Inputs:	none.
 * Output:	NIM_SLOT1 (1).
 */

int 
slot_start_with (void)
{
    return(FIRST_SLOT);
}
/*
 * Function:	get_real_slot
 * Description:	Determine the "real" slot number of the module 
 *		from the given slot arg, which may be incremented 
 *		by a constant to distinguish a submenu entry from
 * 		a main menu entry.
 * Inputs:	slot  - specifies slot number
 * Output:	real slot number.
 */
int 
get_wic_real_slot (int slot)
{
    int real_slot;
    real_slot = (slot <= MAX_WIC ) ? slot : slot - MAX_WIC;

    return(real_slot);
}

/*
 * Function:	get_dc_real_slot
 * Description:	Determine the "real" slot number of the daughter card module 
 *		from the given slot arg, which may be incremented 
 *		by a constant to distinguish a submenu entry from
 * 		a main menu entry.
 * Inputs:	slot  - specifies slot number
 * Output:	real slot number.
 */
int 
get_dc_real_slot(int slot)
{
    int real_slot;
    real_slot = (slot <= MAX_DC ) ? slot : slot - MAX_DC;

    return(real_slot);
}

/*
 * Function:	get_platform_slot_table
 * Description:	serach port_module_tbl for the entry that contains
 *               a certain cookie id
 * 		a main menu entry.
 * Inputs:	id - controller type
 * Output:	index; index value into port_module_tbl
 */
struct module_info *
get_platform_slot_table (int *index, unsigned short id)
{
    int i;
    for (i = 0; i < MAX_MOD_IDS; i++) {
        if ((unsigned short)port_module_tbl[i].id == id) {
            *index = i;
            return (struct module_info *)(&port_module_tbl[i]);      
        }
    }
    return (struct module_info *)NULL;
}

/*
 * Function:	get_max_wic_slots
 * Description:	return max number of wic slots
 * Inputs:	NONE
 * Output:	max number of wic slots
 */
int
get_max_wic_slots (void)
{
    
    return 2;

}

/*
 * Function:    get_pci_dev_num
 *
 * Description: get dev number of SM.
 *
 * Inputs:      slot, dev
 *
 * Output:      SM slot dev no
 */

int
get_pci_dev_num (int slot, int dev)
{
    return(dev);
}

/*
 * Function: get_pci_device_base_offset
 * Descriptoin: dummy fucntion
 * Input: slot    -  not used
 *        dev_num - not used
 *
 * Output: NONE
 */
ulong 
get_pci_device_base_offset (uint slot, uint dev_num)
{
    return PASSED;
}

/*
 * Function: get_pci_device_base
 *
 * Given slot number and device number, this function will return the
 * PCI device base address.
 *
 * Input: slot    -  slot number
 *        dev_num -  PCI device number (IDSEL value).
 *
 * Output: pointer to IO base address for the device
 */
void *
get_pci_device_base (uint slot, uint dev_num)
{
    printf("\n NEED TO FIX %s\n", __FUNCTION__);
    return PASSED;
    
}

/*
 * Function: hwic_slot_start_with
 *
 * This function will return the start number of NGWIC slot.
 *
 * Input: none.
 *
 * Output: 0.
 */
int
hwic_slot_start_with (void)
{
    return(FIRST_SLOT);      /* start with NGWIC slot 1 */
}
/*
 * Function: get_max_hwic_slots
 *
 * This function will return the maximum number of HWIC slots.
 *
 * Input: none.
 *
 * Output: number of hwic slots
 */
int
get_max_hwic_slots (void)
{
    return(MAX_WIC);
}

int
get_wic_serdes_no (int slot)
{
    return (0);
}

int
get_wic_device_no (int slot)
{
    return (0);
}

/*******************************************************************************
 *
 * Function   : get_wic_uart_ctrl (int slot)
 * Description: returns dash fpag uart num of wic 
 *
 * Inputs     : slot number
 *
 * Outputs    : uart num
 *
 *******************************************************************************
 */
uint8_t
get_wic_uart_ctrl (int slot)
{
    if (slot == 0) {
        assert(!"get_wic_uart_ctrl");
    }
    return (wic_uart_ctrl[slot]);

}

/*-----------------------------------------------------------------------------
 *
 * Function slot_get_bd_pid
 *
 * This function will return the board PID
 *
 * Inputs : 
 *          eeprom_data - pointer to eeprom data.
 *          board_pid - borad Product ID
 *
 * Returns : serial number of board.
 */
int
get_slot_bd_pid (uchar *eeprom_data, char *board_pid)
{
    uchar *data_ptr;
    uchar num_byte;

    if (eeprom_data[0] == CURRENT_FORMAT_VERSION) {
        if ((data_ptr = (uchar *)search_type_ret_addr_of_first_data
            (eeprom_data, PRODUCT_ID, &num_byte, FALSE)) == NULL) {
            sprintf((char *)board_pid, (char *)"NO PID");
            return (FAILED);

        } else {
            memcpy(board_pid, data_ptr, num_byte);
        }
        return (PASSED);
    } else {
        sprintf((char *)board_pid, (char *)"NO PID");
        return (FAILED);
    }
}

/*******************************************************************************
 *
 * Function   : get_sm_uart_ctrl (int slot)
 * Description: returns dash fpag uart num of sm
 *
 * Inputs     : slot number
 *
 * Outputs    : uart num
 *
 *******************************************************************************
 */
uint8_t get_sm_uart_ctrl (int slot)
{   
    if (slot == 0) {
        assert(!"get_wic_uart_ctrl");
    }
    return (sm_uart_ctrl[slot]);
}

int get_sm_real_slot(int slot)
{   
    int real_slot;
    real_slot = (slot <= MAX_SM ) ? slot : slot - MAX_SM;

    return(real_slot);
}

int get_vm_real_slot(int slot)
{
    return 0;
}

int get_max_vm_slots (void)
{
    return 0;
}

int get_sm_device_no (int slot)
{   
    return 0;
}


/*-------------------------------------------------------------------
 *
 * Function : set_ngio_now_testing 
 * Description: set flag ngio_testing_now to distinguish 
 *              which ngio/slot is testing
 *
 * INPUT:  ngio_ptr - pointer to ngio structure. 
 * OUTPUT: None
 * -------------------------------------------------------------------
 */
void set_ngio_now_testing (struct ngio_intf_t* ngio_ptr) 
{
    if ((ngio_ptr->mod_type == WIC_MODULE) || 
        (ngio_ptr->mod_type == WIC_DAUGHTER_CARD)) {
        switch (ngio_ptr->slot) {
        case NGWIC1_SLOT: 
            ngio_testing_now = NOW_TESTING_NIM1; 
        break; 
        case NGWIC2_SLOT: 
            ngio_testing_now = NOW_TESTING_NIM2; 
        break; 
        case NGWIC3_SLOT: 
            ngio_testing_now = NOW_TESTING_NONE; 
        break; 
    
        default: 
            printf("%s: slot%d is not matched \n", __FUNCTION__, ngio_ptr->slot); 
        break; 
        }
    } else if ((ngio_ptr->mod_type == SM_MODULE) ||
               (ngio_ptr->mod_type == SM_DAUGHTER_CARD)) {
        switch (ngio_ptr->slot) {
        /* for curie 1ru , it is an entry for return eth port number of ngio
         * on platform_eth_pkt_txrx.c */
        case NGSM1_SLOT: 
            ngio_testing_now = NOW_TESTING_SM1; 
            /* GE1 might not be download port, we don't assign ip here */
            break; 
        case NGSM2_SLOT: 
            ngio_testing_now = NOW_TESTING_SM2; 
            /* GE1 might not be download port, we don't assign ip here */
            break; 
        case NGSM3_SLOT: 
            ngio_testing_now = NOW_TESTING_NONE; 
        break; 
        }
    } else { 
        /* 20190920 -  except NIM/SM and their DC, 
         * the other mod_type has no eth intf, 
         * we don't want to print extra message to confuse people.
         */

    }

    return; 
}

/*******************************************************************************
 *
 * Function get_dsp_or_dynamo
 *
 * This function will select test function with phoenix vitrual SM or Dynamo
 *
 * Inputs : *ngio : ngio interface point
 *
 * Returns : ngio interface
 *******************************************************************************
 */
int get_dsp_or_dynamo (void *ngio)
{
    struct module_info *p;
    struct ngio_intf_t *ngio_iface = (struct ngio_intf_t *)ngio;
    char board_pid[20] = {0};
    int test_err;

    get_slot_bd_pid(ngio_iface->cookie, board_pid);

    printf("PID: %s\n", board_pid);

    if (strncmp(board_pid, DYNAMO_NIM1_PID, DYNAMO_PID_LEN_S) == 0 ) {
        printf("Dynamo\n");
        p = &ngio_ngnim_dsp_dynamo[1];

    } else {
        printf("Virtual SM\n");
        p = &ngio_ngnim_dsp_dynamo[0];
    }

    ngio_iface->diag = p->diag;

    sprintf((char *)ngio_iface->name,(char *)p->name);

    test_err = ngio_iface->diag((void *)ngio_iface);
    return (test_err);

}
