/* $Id: platform_tam_cookie.c,v 1.9 2021/02/24 03:46:27 xiaolaya Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/platform_tam_cookie.c,v $
 *------------------------------------------------------------------
 *
 * platform_tam_cookie.c - copy from tachi/tsn
 * diag_plat_cookie.c - Platform cookie function
 * 
 * July 2015, Times Huang
 *
 * Copyright (c) 2015-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ioctl.h> 
#include <assert.h>
#include "common.h"
#include "cross_platform.h"
#include "act2l_typedef.h"
#include "mfg_api.h"
#include "types.h"
#include "cookie_4.h"
#include "cli_cmd.h"
#include "error.h"
#include "menu.h"
#include "defs.h"
#include "common_utils.h"
#include "platform_tam_cookie.h"
#include "act2_utils.h"
#include "proto.h"
#include <tam_library.h>
#include <tam_lib_manufacturing.h>
#include "nmc93c46.h"
#include "smart_cookie.h"
#include "platform_i2c.h"
#include "i2c_address.h"
#include "plat_defs.h"
#include "slot.h"
#include "mb_tests.h"
#include "ngio.h"
#include "i2c_api.h"
#include "goofy_i2c.h"
#include "tam_aikido_mailbox.h"
#include "dash_fpga.h" 
#include "platform_slot.h" 
#include "queryflags.h"
#include "plug_slot.h"



#define QUACK_RETRY  8 
#define ACT2_RESET_UNRESET_DELAY                        (500)
#define ACT2_UNRESET_DELAY                              (5000)


boolean aikido_mailbox_flag;
boolean aikido_act2_flag;
static uint8_t use_interrupt = 0;

extern struct plug_intf_t *slot_get_plugslot(int);
extern int tam_lib_get_fw_ver(int); 
extern int is_tam_aikido_mbox_on(void); 
extern int tam_act2_reset(int);
extern tam_lib_status_t tam_lib_scc_write_eeprom(void *, uint8_t *, uint16_t, uint16_t);
extern int cookie_4_processor_x(uchar *, int, int, int, cli_cookie_cmd *);
extern int cookie_is_act2(sc_context *);
extern int slot_get_pcb_serial(uchar *, char *);   
extern int act2_is_simple_mode(void *c);
extern int smart_cookie_write(sc_context *con, uchar *cookie_buf);
extern int smart_cookie_write_x(sc_context *con_p, uchar *cookie_buf, ushort size);
extern int smart_cookie_read_x(sc_context *con_p, ushort size);
int alter_mb_cookie(void);
int alter_nim_cookie(void); 
int alter_nim_dc_cookie(void);
int alter_sm_cookie(void); 
int alter_sm_dc_cookie(void); 
int alter_sm_vm_dc_cookie(void); 
int alter_poe_cookie(void);
int alter_raid_cookie(void);
int alter_pim_cookie(void);
void init_cookie_4_default_x(int, int, uchar *, int);
ushort get_cookie_id(int, int, uchar *, uint16_t *, char *);
int get_pcb_serial(uchar *, char *);
int get_pid(uchar *, char *);
int print_cookie(int, char **);
int smartchip(int);
int get_cookie_field_val(int, char *);

static int alter_cookie(int, int);
static int tam_lib_read_cookie(void);
static void store_mb_pid(void);

static dev_if_info_t dev_if;
static uchar cookie_contents[COOKIE_SIZE_512];
static uchar default_cookie_contents[COOKIE_SIZE_512] = {
    0x04, 0xff, 0xc1, 0x8b, 0x46, 0x44, 0x4f, 0x32, 
    0x32, 0x34, 0x38, 0x31, 0x30, 0x44, 0x38, 0x40,
    0x10, 0xac, 0x41, 0x01, 0x00, 0x82, 0x49, 0x4b, 
    0xdf, 0x03, 0x42, 0x30, 0x34, 0xc0, 0x46, 0x03,
    0x20, 0x01, 0x9d, 0x72, 0x02, 0x88, 0x00, 0x07, 
    0xc7, 0xb1, 0x02, 0x03, 0xcb, 0x88, 0x43, 0x45,
    0x39, 0x33, 0x33, 0x31, 0x53, 0x58, 0x89, 0x56, 
    0x30, 0x30, 0x00, 0xc6, 0x8a, 0x54, 0x42, 0x44,
    0x54, 0x42, 0x44, 0x54, 0x42, 0x44, 0x54, 0x09, 
    0xd0, 0xc2, 0x8b, 0x46, 0x44, 0x4f, 0x32, 0x32,
    0x34, 0x39, 0x41, 0x32, 0x33, 0x4d, 0xc3, 0x06, 
    0x08, 0xec, 0xf5, 0xa7, 0x4a, 0x50, 0x43, 0x00,
    0x90, 0xc4, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
};

void i2c_act2_reset(sc_context *);

static int i2c_act2_write_bytes(sc_context *, char *, int);
static int i2c_act2_read_bytes(sc_context *, char *);

static char smc_buf[80];
static char i2c_err[80];

extern struct ngio_intf_t *slot_get_ngiowic(int);
extern int slot_i2c_unreset(struct ngio_intf_t *, int, char *);

/*--------------------------------------------------------------------------
 *
 * Function get_sm_mac_addr
 *
 * This function will return the MAC addr of the specified SM
 *
 * Inputs : SM slot #
 * board_mac_addr addr - pointer to mac addr
 * Returns :
 */
int
get_sm_mac_addr (int slot, uchar *board_mac_addr)
{
    return (get_ngio_mac_addr (slot, SM_MODULE, board_mac_addr));
}

/**********************************************************************
 *
 * Function: smart_cookie_read_write_eeprom
 *
 * Description: Dummy function
 *
 * Input:  Dummy parameters
 * Output: PASSED
 *
 *********************************************************************
 */
int smart_cookie_read_write_eeprom (sc_context * con, cli_cookie_cmd * cli_cmd)
{
    if (act2_is_simple_mode(con)) {
        cterr('f', 0, "device is in simple mode. can't access Act1 space.");
        return FAILED;
    }

    if (con->dev_if_p->cookie_size != COOKIE_SIZE_128) {
        if(smart_cookie_read_x(con, con->dev_if_p->cookie_size)) {
            printf("failed smart_cookie.c @%d\n", __LINE__);
            return FAILED;
        }
        if (cookie_4_processor_x(con->cookie_contents, con->type,
				con->slot, con->dev_if_p->cookie_size, cli_cmd)) {
            if(smart_cookie_write_x(con, con->cookie_contents,
                                 con->dev_if_p->cookie_size))
                 return FAILED;
        }
     } else {   /* size 128; should not come here for new new platforms (ovd, or newer ) */
        if(smart_cookie_read(con)) {
            return FAILED;
        }
        if (cookie_4_processor_x(con->cookie_contents, con->type,
                                con->slot, con->dev_if_p->cookie_size, cli_cmd)) {
	    if(smart_cookie_write(con, con->cookie_contents))
            return FAILED;
        }
    }

    return (PASSED);
}

/*************************************************************************
 *  Function: cli_change_cookie
 *
 * This function is to search the cookie field and change field value
 *
 * Input: uchar *str
 *        int field 
 * Output: PASSED / FAILED
 ********************************************************************
 */
int cli_change_cookie (int field, char *str, cli_cookie_cmd * cmd)
{
    return (PASSED);
}

/**************************************************************************
 *
 * Name: i2c_quack_reset
 *
 * Description: This function implementes a reset to Quack chip by
 *              reset the line for 50ms then unreset it
 *
 * Inputs: con - pointer to sc_context
 *
 * Outputs: None
 *
 *************************************************************************/
void
i2c_quack_reset (sc_context *con_p)
{
    return;
}

/**********************************************************************
 *
 * Function: init_ngio_context
 *
 * Description:
 *           intializes ngio context.
 *
 * Input:  con_p   - pointer to sc_context
 *         type    - type of module (ie, aim, mb, wic, etc)
 *         slot    - slot
 *         cookie_p- pointer to eeprom data
 *
 * Output: PASS/FAIL
 *
 **********************************************************************
 */
static void
init_ngio_context (sc_context *con_p, uchar type,
                                uchar slot, uchar *cookie_p)
{
    con_p->type = type;
    con_p->slot = slot;
    con_p->cookie_contents = cookie_p;
    con_p->quack_read_2bytes = (PFT)i2c_act2_read_bytes;
    con_p->quack_write_2bytes = (PFT)i2c_act2_write_bytes;
    con_p->quack_reset = (PFT)i2c_act2_reset;

    con_p->dev_if_p->parm1 = (uint8_t)IOFPGA_I2C;
    con_p->dev_if_p->parm3 = (uint8_t)NGIO_I2C_MUX_ACT2;

    con_p->dev_if_p->interface = SCC_I2C_IF;

    /* important, using legacy way */ 
    aikido_act2_flag = FALSE;
    aikido_mailbox_flag = FALSE;
}

/**********************************************************************
 *
 * Function: plat_init_smart_eeprom_context
 *
 * Description:
 *           intializes sc_context.
 * Input:  con_p   - pointer to sc_context
 *         type    - type of module (ie, aim, mb, wic, etc)
 *         slot    - slot
 *         cookie_p- pointer to eeprom data
 *
 * Output: PASS/FAIL
 *
 **********************************************************************
 */
int plat_init_smart_eeprom_context (sc_context *con_p, uchar type,
                                    uchar slot, uchar *cookie_p)
{
    int retval = PASSED, max_sm_slot;
    struct ngio_intf_t *ngiowic;
    struct plug_intf_t *plugslot;

    *i2c_err = '\0';
    
    con_p->info_string = smc_buf;
    
    switch (type) {
    case MOTHER_BOARD: /* I2C interface */
        con_p->type = type;
        con_p->slot = slot;        /* slot = 0 */
        con_p->cookie_contents = cookie_p;
        con_p->quack_read_2bytes = (PFT)i2c_act2_read_bytes;
        con_p->quack_write_2bytes = (PFT)i2c_act2_write_bytes;
        con_p->quack_reset = (PFT)i2c_act2_reset;
        con_p->dev_if_p->parm1 = (uint8_t)IOFPGA_I2C; 
        if (aikido_act2_flag) {
            printf("MB cookie Read from AIKIDO \n"); 
            con_p->dev_if_p->parm2 = (uint8_t)MB_I2C_ADDR_AIKIDO_ACT2;
        } else {
            printf("MB cookie Read from ACT2   \n"); 
            con_p->dev_if_p->parm2 = (uint8_t)MB_I2C_ADDR_ACT2;
        }
        con_p->dev_if_p->parm3 = (uint8_t)0;   
        con_p->dev_if_p->parm4 = (uint8_t)I2C_CTRL_ZERO;   
        con_p->dev_if_p->interface = SCC_I2C_IF;
        sprintf((char *)smc_buf, "MB");
        break;
    case WIC_MODULE:
    case WIC_DAUGHTER_CARD:
        con_p->dev_if_p->parm4 = (uint8_t)get_wic_i2c_ctrl(slot);
        if (type == WIC_MODULE) {
            sprintf((char *)smc_buf, "WIC MODULE %d", slot);
            init_ngio_context(con_p, type, slot, cookie_p);
            con_p->dev_if_p->parm2 = (uint8_t)NGIOWIC_I2C_ADDR_ACT2;
        } else {
            /* check if slot is exceed MAX SM slot num
             * if yes, we don't need to consider double dc card
             */
            max_sm_slot = get_max_sm_slots();
            if (slot > max_sm_slot) {
                sprintf((char *)smc_buf, "DC (WIC%d)", slot);
            } else {
                /* check if dc is nim or sm */
                ngiowic = (struct ngio_intf_t *)slot_get_ngiowic(slot);
                if (is_ngiowic_i2c_unreset((void *)ngiowic)) {
                    sprintf((char *)smc_buf, "DC (WIC%d)", slot);
                } else {
                    /* nim is not available, assign SM i2c ctrl bus */
                    con_p->dev_if_p->parm4 = (uint8_t)get_sm_i2c_ctrl(slot);
                    sprintf((char *)smc_buf, "DC of DC (SM%d)", slot);
                }
            }

            init_ngio_context(con_p, type, FIRST_SLOT, cookie_p);
            con_p->dev_if_p->parm2 = (uint8_t)NGIOVM_I2C_ADDR_ACT2;
        }
        break;
    case SM_MODULE:
    case SM_DAUGHTER_CARD:
    case SM_VM_DAUGHTER_CARD:  /* This is VM card */
        con_p->dev_if_p->parm4 = (uint8_t)get_sm_i2c_ctrl(slot);
        if (type == SM_MODULE) {
            sprintf((char *)smc_buf, "SM MODULE %d", slot);
            init_ngio_context(con_p, type, slot, cookie_p);
            con_p->dev_if_p->parm2 = (uint8_t)NGIOSM_I2C_ADDR_ACT2;
        } else if (type == SM_VM_DAUGHTER_CARD) {
            sprintf((char *)smc_buf, "DC VM (SM %d)", slot);
            init_ngio_context(con_p, type, slot, cookie_p);
            con_p->dev_if_p->parm2 = (uint8_t)NGIOSM_VM_DC_I2C_ADDR_ACT2;
        } else {
            sprintf((char *)smc_buf, "DC (SM%d)", slot);
            init_ngio_context(con_p, type, FIRST_SLOT, cookie_p);
            con_p->dev_if_p->parm2 = (uint8_t)NGIOWIC_I2C_ADDR_ACT2;
        }
        break;
    case VM_MODULE:
        sprintf((char *)smc_buf, "VM MODULE ");
        con_p->dev_if_p->parm4 = (uint8_t)VM_I2C_CTRL;
        init_ngio_context(con_p, type, slot, cookie_p);
        con_p->dev_if_p->parm2 = (uint8_t)NGIOVM_I2C_ADDR_ACT2;
        break;
    case DAUGHTER_CARD:
        printf("30WPoE daughter card \n");
        con_p->type = type;
        con_p->slot = 1;
        con_p->cookie_contents = cookie_p;
        con_p->quack_read_2bytes = (PFT)i2c_act2_read_bytes;
        con_p->quack_write_2bytes = (PFT)i2c_act2_write_bytes;
        con_p->quack_reset = (PFT)i2c_act2_reset;
        con_p->dev_if_p->parm1 = (uint8_t)IOFPGA_I2C;
        if (is_sword()) {
            /* sword's poe uses act2 chip so address is different */
            con_p->dev_if_p->parm2 = (uint8_t)MB_I2C_ADDR_ACT2;
        } else
            con_p->dev_if_p->parm2 = (uint8_t)MB_I2C_ADDR_POE_30W_QUACK;
        con_p->dev_if_p->parm3 = (uint8_t)I2C_MUX_POE_30W_QUACK;
        con_p->dev_if_p->parm4 = (uint8_t)I2C_CTRL_POE_30W_QUACK;
        con_p->dev_if_p->interface = SCC_I2C_IF;
        sprintf((char *)smc_buf, "MB");

        break;
    case PLUGGABLE_CARD:
        plugslot = (struct plug_intf_t *)slot_get_plugslot(slot);
        if (plug_slot_i2c_unreset(plugslot) == FAILED) {
            printf("%s: PLUG I2C Unreset failed\n", __func__);
            return (FAILED);
        }
        con_p->type = type;
        con_p->slot = slot;        /* slot = 0 */
        con_p->cookie_contents = cookie_p;
        con_p->quack_read_2bytes = (PFT)i2c_act2_read_bytes;
        con_p->quack_write_2bytes = (PFT)i2c_act2_write_bytes;
        con_p->quack_reset = (PFT)i2c_act2_reset;
        con_p->dev_if_p->parm1 = (uint8_t)PLUG_FPGA;
        con_p->dev_if_p->parm2 = (uint8_t)PLUG_I2C_ADDR_ACT2;
        con_p->dev_if_p->parm3 = (uint8_t)0;
        con_p->dev_if_p->parm4 = I2C_CTRL_TWENTY; /* I2C Controller */
        con_p->dev_if_p->interface = SCC_I2C_IF;
        sprintf((char *)smc_buf, "PLUG");
        
        /* important, using legacy way */ 
        aikido_act2_flag = FALSE;
        aikido_mailbox_flag = FALSE;
        break;
    case SM_DC_WIC_CARD:
    case SM_DC_WIC_DC_VM_CARD:
        init_ngio_context(con_p, type, slot, cookie_p);
        con_p->dev_if_p->parm1 = (uint8_t)MOD_IOFPGA_I2C;
        con_p->dev_if_p->parm4 = (uint8_t)get_sm_dc_wic_i2c_ctrl(slot);
        if (type == SM_DC_WIC_CARD) {
            sprintf((char *)smc_buf, "SM DC WIC %d", slot);
            con_p->dev_if_p->parm2 = (uint8_t)NGIOWIC_I2C_ADDR_ACT2;
        } else {
            sprintf((char *)smc_buf, "SM DC WIC DC VM %d", slot);
            con_p->dev_if_p->parm2 = (uint8_t)NGIOSM_VM_DC_I2C_ADDR_ACT2;
        }
        break;
    default:
        cterr('f',0,"in plat_init_smart_eeprom_context: Not a supported "
              "Smart EEPROM type %d", type);
        assert(!"plat_init_eeprom: invalid argument");
        retval = FAILED;
    }

    return (retval);
}


/**************************************************************************
 *
 * Name: i2c_act2_reset
 *
 * Description: This function implementes a reset to Quack chip by
 *              reset the line for 50ms then unreset it
 *
 * Inputs: con - pointer to sc_context
 *
 * Outputs: None
 *
 *************************************************************************/
void i2c_act2_reset (sc_context *con_p)
{
    unsigned int slot; 
    struct ngio_intf_t *ngio; 

    slot = con_p->slot;

    if (con_p->type == MOTHER_BOARD) {
        if (is_tam_aikido_mbox_on() == FALSE) { 
            printf("Resetting ACT2 Motherboard...");
            fflush(stdout);
            reset_plat_dev(FPGA_RST_ACT2);
            msleep(ACT2_RESET_UNRESET_DELAY);
            unreset_plat_dev(FPGA_RST_ACT2);
            msleep(ACT2_UNRESET_DELAY);
            printf("Done\n");
            fflush(stdout);
        } /* only ACT2 needs reset, AIKIDO doesn't need it */

    } else if (con_p->type == SM_MODULE) { 
        printf("Resetting SM ACT2...");
        fflush(stdout);
        ngio = (struct ngio_intf_t *)slot_get_ngiosm(slot);
        ngio->i2c_reset(ngio); 
        msleep(ACT2_RESET_UNRESET_DELAY);
        ngio->i2c_unreset(ngio); 
        msleep(ACT2_UNRESET_DELAY);
        printf("Done\n");
        fflush(stdout);

    } else if (con_p->type == WIC_MODULE)  {
        printf("Resetting NIM ACT2...");
        fflush(stdout);
        ngio = (struct ngio_intf_t *)slot_get_ngiowic(slot);
        ngio->i2c_reset(ngio); 
        msleep(ACT2_RESET_UNRESET_DELAY);
        ngio->i2c_unreset(ngio); 
        msleep(ACT2_UNRESET_DELAY);
        printf("Done\n");
        fflush(stdout);

    } else if (con_p->type == PLUGGABLE_CARD) {
        printf("Fixme %s: pluggable i2c reset \n", __FUNCTION__);

    } else if (con_p->type == SM_DC_WIC_CARD || SM_DC_WIC_DC_VM_CARD) {
        printf("Resetting NIM ACT2...");
        fflush(stdout);
        ngio = (struct ngio_intf_t *)get_sm_dc_wic_ngio(slot);
        ngio->i2c_reset(ngio);
        msleep(ACT2_RESET_UNRESET_DELAY);
        ngio->i2c_unreset(ngio);
        msleep(ACT2_UNRESET_DELAY);
        printf("Done\n");
        fflush(stdout);

    } else {
        printf("error %s: con_p->type unknown \n", __FUNCTION__); 
    }
    return;
}


/**************************************************************************
 *
 * Name: i2c_act2_read_bytes
 *
 * Description: Read bytes from the I2C interface
 *
 * Inputs: con_p   - Pointer to sc_context
 *         read_buffer - buffer to hold the data
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
int i2c_act2_read_bytes (sc_context *con_p, char *rx_buffer)
{
    uint32_t ret_status, error_flag = PASSED;
    n2g_i2c_if_t *n2g_i2c_if_p, n2g_i2c_if_ds;
    int ix;

    memset(&n2g_i2c_if_ds, 0, sizeof(n2g_i2c_if_t));
    n2g_i2c_if_p = &n2g_i2c_if_ds;
    n2g_i2c_if_p->i2c_bus_type = con_p->dev_if_p->parm1;
    n2g_i2c_if_p->i2c_dev = con_p->dev_if_p->parm2;
    n2g_i2c_if_p->mux = con_p->dev_if_p->parm3;
    n2g_i2c_if_p->i2c_ctrl = con_p->dev_if_p->parm4;
    
    n2g_i2c_if_p->size = 4; /* default of read 2 byte routine */
    n2g_i2c_if_p->buf = rx_buffer;
    n2g_i2c_if_p->offset = -1;
    if (n2g_i2c_if_p->i2c_bus_type == MOD_IOFPGA_I2C)
        n2g_i2c_if_p->i2c_base = get_sm_dc_wic_i2c_base(n2g_i2c_if_p->i2c_ctrl);

    for (ix = 0; ix < QUACK_RETRY; ix++) {
        if ((ret_status = n2g_i2c_read (n2g_i2c_if_p)) != RC_I2C_OP_OK) {
            sprintf(i2c_err,  i2c_err_str(ret_status));
            error_flag = FAILED;
            sleep(1);
        } else {
            return (PASSED);
        }
    }

    /* still need close it after this point */
    return (error_flag);
}

/**************************************************************************
 *
 * Name: i2c_act2_write_bytes
 *
 * Description: Write bytes to the I2C interface
 *
 * Inputs: con_p   - Pointer to sc_context
 *         tx_buffer - pointer to the command to be sent
 *         tx_size - size of the command
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
int i2c_act2_write_bytes (sc_context *con_p, char *tx_buffer, int tx_size)
{
    uint32_t ret_status, error_flag = PASSED;
    n2g_i2c_if_t *n2g_i2c_if_p, n2g_i2c_if_ds;
    int ix;

    memset(&n2g_i2c_if_ds, 0, sizeof(n2g_i2c_if_t));
    n2g_i2c_if_p = &n2g_i2c_if_ds;
    n2g_i2c_if_p->i2c_bus_type = con_p->dev_if_p->parm1;
    n2g_i2c_if_p->i2c_dev = con_p->dev_if_p->parm2;
    n2g_i2c_if_p->mux = con_p->dev_if_p->parm3;
    n2g_i2c_if_p->i2c_ctrl = con_p->dev_if_p->parm4;
    
    n2g_i2c_if_p->size = tx_size;
    n2g_i2c_if_p->buf = tx_buffer;
    n2g_i2c_if_p->offset = -1;
    if (n2g_i2c_if_p->i2c_bus_type == MOD_IOFPGA_I2C)
        n2g_i2c_if_p->i2c_base = get_sm_dc_wic_i2c_base(n2g_i2c_if_p->i2c_ctrl);

    for (ix = 0; ix < QUACK_RETRY; ix++) {
        if ((ret_status = n2g_i2c_write(n2g_i2c_if_p)) != RC_I2C_OP_OK) {
            sprintf(i2c_err,  i2c_err_str(ret_status));
            error_flag = FAILED;
            sleep(1);
        } else {
            return (PASSED);
        }
    }

    /* still need close it after this point */
    return (error_flag);
}

void clean_smart_eeprom_context (sc_context *con_p)
{
	printf("To be developed...\n");
}

/*------------------------------------------------------------------------
 * smartchip
 *
 * DESCRIPTION:
 *  menu to allow user to select various test for smart chip.
 *
 * PARAMETERS:
 *     op - type of operation to be performed.
 *
 * RETURNS:
 *     PASSED - if successful
 *     FAILED - if unsuccessful
 *--------------------------------------------------------------------------*/
int smartchip (int submenu_flag)
{
    char choice[5], type;
    sc_context *con, cont;
    int slot, max_slot, act2_chip;
    struct ngio_intf_t *ngio;
    struct plug_intf_t *plugslot;
    

    con = &cont;
    con->dev_if_p = &dev_if;
    con->dev_if_p->cookie_size = COOKIE_SIZE_512;
    
    
    testname("Smart cookie");
    if (submenu_flag == 0) {
        prpass(testpass, "Main Menu");
    } else {
        prpass(testpass, "Sub Menu");
    }

    printf("\nEnter (m)otherboard, (s)m, (n)im, (i)sp, (p)im (Q)UIT  >");
    get_line(choice, sizeof(choice) + 1);
    
    switch (choice[0]) {
    case 'm':
        act2_chip = getdec_answer("\n(1-Aikido-ACT2):", 1, 0, 1);
        if (act2_chip == 0) {
            aikido_act2_flag = FALSE;
            aikido_mailbox_flag = FALSE;
            printf("\nSelect Discrete ACT2\n");
        } else if (act2_chip == 1) {
            aikido_act2_flag = TRUE;
            aikido_mailbox_flag = TRUE;
            printf("\nSelect AIKIDO MAIL Box\n");
        } else {
             printf("\n %s:%d Never come here \n", __FUNCTION__, __LINE__); 
        }

        plat_init_smart_eeprom_context(con, MOTHER_BOARD, 0, cookie_contents);

        act2_init_cont((void *)con);

        return (act2_prog(0));
        break;
    case 's':
        slot = FIRST_SLOT;

        ngio = (struct ngio_intf_t *)slot_get_ngiosm(slot);
        
        if (!ngiosm_present(ngio)) {
	    printf("\nNo SM in slot %d.\n", con->slot);
            return FAILED;
        }
        if ((ngiosm_enable(ngio)) < 0) {
            printf("Unable to power SM module slot %d\n", slot);
            return FAILED;
        }
        if ((ngiosm_i2c_unreset(ngio)) < 0) {
            printf("Unable to unreset SM  module slot %d\n", slot);
            return FAILED;
        }

        type = SM_MODULE;
        
        if (plat_init_smart_eeprom_context(con, type, slot, cookie_contents) == FAILED) {
            return FAILED;
        }
        act2_init_cont((void*)con);  /* must be called after plat_init */

        return (act2_prog(0));

    case 'n':  
        slot = FIRST_SLOT;

        ngio = (struct ngio_intf_t *)slot_get_ngiowic(slot);
        
        if (!ngiowic_present(ngio)) {
	    printf("\nNo NIM in slot %d.\n", con->slot);
            return FAILED;
        }
        if ((ngiowic_enable(ngio)) < 0) {
            printf("Unable to power NIM module slot %d\n", slot);
            return FAILED;
        }
        if ((ngiowic_i2c_unreset(ngio)) < 0) {
            printf("Unable to unreset NIM  module slot %d\n", slot);
            return FAILED;
        }

        type = WIC_MODULE;
        
        if (plat_init_smart_eeprom_context(con, type, slot, cookie_contents) == FAILED) {
            return FAILED;
        }
        act2_init_cont((void*)con);  /* must be called after plat_init */

        return (act2_prog(0));
        break;

    case 'I':
    case 'i':
        plat_init_smart_eeprom_context(con, ISP_CARD, 2, cookie_contents);

        act2_init_cont((void *)con);
        
        return (act2_prog(0));
        break;
    case 'p':
        max_slot = get_max_pim_slots();
        slot = getdec_answer("Enter PLUG slot number", 1, FIRST_SLOT,
                max_slot);
  
        plugslot = (struct plug_intf_t *)slot_get_plugslot(slot);

        if (plug_slot_i2c_poweron_unreset(plugslot, slot, "PLUGGABLE_CARD") != PASSED) {
            printf("Unable to unreset PIM module slot %d\n", slot);
            return (FAILED);
        }

        if (plat_init_smart_eeprom_context (con, PLUGGABLE_CARD,
                                            slot, (uchar *)cookie_contents)) {
            return (FAILED);
        }

        act2_init_cont(con);

        /*
         * ACT2 library
         */
        return (act2_prog(0));
        break;

    default:
        printf("Invalid input %s\n", choice);
        break;
    }
    
    return (PASSED);
}

/*
 * Function: get_pid
 *
 * Description:
 *   This function read cookie for product id field .
 *
 * Parameters:
 *   eeprom_data - cookie in eeprom 
 *   pid - a pid read from cookie. 
 *
 * Returns:
 *   PASSED/FAILED
 */
int get_pid (uchar *eeprom_data, char *pid)
{
    int32_t i;
    uchar num_byte, *data_ptr;

    /* using NGIO cookie structure ngio_intf_t->cookie as eeprom_data
     * to bypass accessing cookie again and again
     */
    if ((data_ptr = (uchar *) search_type_ret_addr_of_first_data
         (eeprom_data, (uchar) PRODUCT_ID,
          &num_byte, FALSE)) == (uchar *) NULL) {
        /*Search CONTROLLER_TYPE failed. */
        pid[0] = 0;                /* illegal code */
        return(FAILED);
    } else {
        for (i = 0; i < num_byte; i++) {
            pid[i] = *data_ptr++;
        }
    }

    return (PASSED);
}

/*
 * Function: get_mb_pid
 *
 * Description:
 *   This function read MB cookie for product id field .
 *
 * Parameters:
 *   pid - a pid read from cookie.
 *
 * Returns:
 *   none; 
 */
void get_mb_pid (char *pid) 
{
    sc_context *con, cont;
    int ix;
    uint8_t *pdata = NULL;
    uint8_t ret_num_of_bytes = 0;
    char prodName[PRODUCT_NAME_LEN] = { 0 };

    con = &cont;
    con->dev_if_p = &dev_if;
    con->dev_if_p->cookie_size = COOKIE_SIZE_512;

    if (plat_init_smart_eeprom_context(con, MOTHER_BOARD, 0, 
                                      (uchar *)cookie_contents) == FAILED) {
        printf("\n%s: Init Smart EEPROM context failed\n", __func__);
        return;
    }
    act2_init_cont(con);

    if (tam_lib_read_cookie() == FAILED) {
        printf("%s: Read cookie failed\n", __FUNCTION__);
        return;
    }

    pdata = search_type_ret_addr_of_first_data(cookie_contents, PRODUCT_ID,
                                               &ret_num_of_bytes, 0);
   
    if (pdata == NULL) {
        printf("\n *** ERROR: Product id field not programmed per Product PCAMAP.");
        return;
    } else {
        ret_num_of_bytes = (ret_num_of_bytes > MAX_PID_LEN) ?
                            MAX_PID_LEN : ret_num_of_bytes;
        for (ix = 0; ix < ret_num_of_bytes; ix++) {
            prodName[ix] = *(pdata + ix);
        }
    }

    sprintf(pid, "%s", prodName);

    return; 
}

/*
 * Function: store_mb_pid
 *
 * Description:
 *   This function read MB cookie for product id field .
 *   And store it into /tmp/mb_pid file for future use. 
 *
 * Parameters:
 *   none
 *
 * Returns:
 *   none;
 */
static void store_mb_pid (void) 
{
    FILE *fp; 
    char sku_name[32]; 

    get_mb_pid(sku_name);

    fp = fopen(MB_PID_FILE, "w"); 
    if (fp != NULL) {
        fprintf(fp, "%s", sku_name);
        fclose(fp);
    } else {
        printf("Failed to open %s \n", MB_PID_FILE);
    }

    return; 
}

/*-----------------------------------------------------------------------
 *
 * Function: get_plat_sku_cookie
 *
 * This functions based on PID cookie to give the sku number
 *
 * Input : none
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------
 */
int get_plat_sku_cookie (void)
{
    char sku_name[50];

    memset(sku_name, '\0', sizeof(sku_name));
    get_mb_pid(sku_name);
    printf("Cookie: Platform PID = %s\n\n", sku_name);
    fflush(stdout);

    if (strncmp(sku_name, SKU_ISR4451_STR, strlen(SKU_ISR4451_STR)) == 0) {
        /* Overlord */
        return(SKU_ISR4451);
    }
    else if (strncmp(sku_name, SKU_ISR4431_STR, strlen(SKU_ISR4431_STR)) == 0) {
        /* Juno */
        return(SKU_ISR4431);
    }
    else if (strncmp(sku_name, SKU_ISR4351_STR, strlen(SKU_ISR4351_STR)) == 0) {
        /* Utah */
        return(SKU_ISR4351);
    }
    else if (strncmp(sku_name, SKU_ISR4331_STR, strlen(SKU_ISR4331_STR)) == 0) {
        /* Sword */
        return(SKU_ISR4331);
    }
    else if (strncmp(sku_name, SKU_ISR4321_STR, strlen(SKU_ISR4321_STR)) == 0) {
        /* Dagger */
        return(SKU_ISR4321);
    }
    else if (strncmp(sku_name, SKU_ISR4461_STR, strlen(SKU_ISR4461_STR)) == 0) {
        /* Neptune */
        return (SKU_ISR4461);
    }
    else if (strncmp(sku_name, SKU_ISR4452E_STR, strlen(SKU_ISR4452E_STR)) == 0) {
        /* Triton */
        return (SKU_ISR4452E);
    }
    else if (strncmp(sku_name, SKU_ISR4452_STR, strlen(SKU_ISR4452_STR)) == 0) {
        /* Proteus */
        return (SKU_ISR4452);
    }
    else if (strncmp(sku_name, SKU_ISR4432_STR, strlen(SKU_ISR4432_STR)) == 0) {
        /* Neso */
        return (SKU_ISR4432);
    }
    else if (strncmp(sku_name, SKU_ISR4221_STR, strlen(SKU_ISR4221_STR)) == 0) {
        /* Goldbeach */
        return(SKU_ISR4221);
    }
    else if (strncmp(sku_name, SKU_VG450_STR, strlen(SKU_VG450_STR)) == 0) {
        /* VG450 */
        return(SKU_VG450);
    }
    else if (strncmp(sku_name, SKU_C8300_2N2S_4G2X_STR, strlen(SKU_C8300_2N2S_4G2X_STR)) == 0 ||
             strncmp(sku_name, SKU_C8300_2N2S_4T2X_STR, strlen(SKU_C8300_2N2S_4T2X_STR)) == 0) {
        /* uranium */
        return(SKU_C8300_2N2S_4T2X);
    }
    else if (strncmp(sku_name, SKU_C8300_2N2S_6G_STR, strlen(SKU_C8300_2N2S_6G_STR)) == 0 ||
             strncmp(sku_name, SKU_C8300_2N2S_6T_STR, strlen(SKU_C8300_2N2S_6T_STR)) == 0) {
        /* thorium */
        return(SKU_C8300_2N2S_6T);
    }
    else if (strncmp(sku_name, SKU_C8300_1N1S_4T2X_STR, strlen(SKU_C8300_1N1S_4T2X_STR)) == 0) {
        /* radium */
        return(SKU_C8300_1N1S_4T2X);
    }
    else if (strncmp(sku_name, SKU_C8300_1N1S_6T_STR, strlen(SKU_C8300_1N1S_6T_STR)) == 0) {
        /* thallium */
        return(SKU_C8300_1N1S_6T);
    }
    else {
        printf("Cookie: Platform SKU unknown\n");
        return(SKU_INVALID);
    }
}



/*
 * Function: is_poe_sku
 *
 * Description:
 *   check mb is POE sku to distinguish POE related item is skipped or not. 
 *
 * Parameters:
 *   none
 *
 * Returns:
 *   TRUE/FALSE
 */
int is_poe_sku_test (void) 
{

    /* not support yet */
    return (FALSE); 
    store_mb_pid();  // will use this function
    return (FALSE); 

}

/*------------------------------------------------------------------------
 * print_cookie
 *
 * DESCRIPTION:  print cookie info to be captured by MFG script. this is
 *               used for WDC.
 *
 *
 * PARAMETERS: argc, argv - dummy parameters
 *
 * RETURNS:
 *     PASSED - if successful
 *     FAILED - if unsuccessful
 *--------------------------------------------------------------------------*/
int print_cookie (int argc, char *argv[])
{
    int ix;
    uint8_t *pdata = NULL;
    uint8_t ret_num_of_bytes = 0;

    /* we add delay here for wdc program, which is using act2 reset but 
     * without polling chip status and read cookie directly. 
     * In this case, BST will encounter i2c is not ack error. 
     */
    msleep(ACT2_UNRESET_DELAY); 

    if (tam_lib_read_cookie() == FAILED) {
        printf("%s: Read cookie failed\n", __FUNCTION__);
        return (FAILED);
    }

    printf("\nUDI");
    pdata = search_type_ret_addr_of_first_data(cookie_contents, PRODUCT_ID,
                                               &ret_num_of_bytes, 0);
    
    if (pdata == NULL) {
        printf("\n *** ERROR: Product id field not programmed per Product PCAMAP.");
        return (FAILED);
    } else {
        printf("\n PID:");
        ret_num_of_bytes = (ret_num_of_bytes > MAX_PID_LEN) ? 
                            MAX_PID_LEN : ret_num_of_bytes;    
        for (ix = 0; ix < ret_num_of_bytes; ix++) {
            printf("%c", *(pdata + ix));
        }
    }

    pdata = search_type_ret_addr_of_first_data(cookie_contents, VERSION_ID,
                                               &ret_num_of_bytes, 0);
    
    if (pdata == NULL) {
        printf("\n *** ERROR: Version id field not programmed per Product PCAMAP.");
        return (FAILED);
    } else {
        printf("\n VID:");
        ret_num_of_bytes = (ret_num_of_bytes > MAX_VID_LEN) ? 
                            MAX_VID_LEN : ret_num_of_bytes;    
        for (ix = 0; ix < ret_num_of_bytes; ix++) {
            printf("%c", *(pdata + ix));
        }
    }

    /* To make sure the WDC is in sync with SUDI */
    if (pcb_for_sudi == FALSE) {
	pdata = search_type_ret_addr_of_first_data(cookie_contents, CHASSIS_SERIAL_NUM,
						   &ret_num_of_bytes, 0);
    } else {
	pdata = search_type_ret_addr_of_first_data(cookie_contents, PCB_SERIAL_NUM,
						   &ret_num_of_bytes, 0);
    }
    
    if (pdata == NULL) {
        printf("\n *** ERROR: Chassis Serial Number field not programmed per Product PCAMAP.");
        return (FAILED);
    } else {
        printf("\n SN:");
        ret_num_of_bytes = (ret_num_of_bytes > MAX_CSN_LEN) ? 
                            MAX_CSN_LEN: ret_num_of_bytes;    
        for (ix = 0; ix < ret_num_of_bytes; ix++) {
            printf("%c", *(pdata + ix));
        }
    }

    return (PASSED);
}

/*--------------------------------------------------------------------------
 *
 * Function get_pcb_serial
 *
 * This function will return the SERIAL number.
 *
 * Inputs: eeprom_data - dummy parameters
 *         serial - control id
 *
 * Returns : PASSED/FAILED
 *--------------------------------------------------------------------------
 */
int get_pcb_serial (uchar *eeprom_data, char *serial)
{
    int ix;
    uint8_t *pdata = NULL;
    uint8_t ret_num_of_bytes = 0;
    char prodSN[PRODUCT_SERIAL_LEN] = { 0 };
    
    if (tam_lib_read_cookie() == FAILED) {
        printf("%s: Read cookie failed\n", __FUNCTION__);
        return (FAILED);
    }

    pdata = search_type_ret_addr_of_first_data(cookie_contents, PCB_SERIAL_NUM,
                                               &ret_num_of_bytes, 0);
    
    if (pdata == NULL) {
        printf("\n *** ERROR: Chassis Serial Number field not programmed per Product PCAMAP.");
        return (FAILED);
    } else {
        ret_num_of_bytes = (ret_num_of_bytes > MAX_PID_LEN) ? 
                            MAX_PID_LEN : ret_num_of_bytes;    
        for (ix = 0; ix < ret_num_of_bytes; ix++) {
            prodSN[ix] = *(pdata + ix);
        }
    }

    sprintf(serial, "%s", prodSN);

    return (PASSED);
}

/**************************************************************************
 *
 * Name: get_cookie_id
 *
 * Description: read cookie id
 *
 * Inputs: slot, type, eeprom_data, err - dummy parameters
 *         id - control id
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
ushort get_cookie_id (int slot, int type, uchar *eeprom_data, 
                      uint16_t *id, char *err)
{
    int ix;
    uint8_t *pdata = NULL;
    uint8_t ret_num_of_bytes = 0;
    unsigned char ctrl_type[CONTROL_TYPE_LEN] = { 0 };
    sc_context *con, cont;

    con = &cont;
    con->dev_if_p = &dev_if;
    con->dev_if_p->cookie_size = COOKIE_SIZE_512;

    plat_init_smart_eeprom_context(con, type, slot, cookie_contents);
    act2_init_cont(con);

    /* mb is not using tam_act2_reset(), it has no delay between 
     * reset and unreset, non-aikido act2 needs delay */
    if (type == MOTHER_BOARD) {
        con->quack_reset(con); 
    } else { 
        tam_act2_reset(0); 
    }

    if (cookie_is_act2(con)) {
        printf("Detecting ACT2 chip...\n");

        if (tam_lib_read_cookie() == FAILED) {
            printf("%s: Read cookie failed\n", __FUNCTION__);
            return (FAILED);
        }
    } else {
	printf("\nDetect Smart Cookie chip ...\n");
        if (smart_cookie_read(con) == FAILED) {
            printf("%s: Read cookie failed\n", __FUNCTION__);
            return (FAILED);
        }

    }

    memcpy(eeprom_data, cookie_contents, COOKIE_SIZE_512);

    pdata = search_type_ret_addr_of_first_data(cookie_contents, CONTROLLER_TYPE,
                                               &ret_num_of_bytes, 0);

    if (pdata == NULL) {
        printf("\n *** ERROR: Control Type Number field not programmed per Product PCAMAP.");
        return (FAILED);
    } else {
        ret_num_of_bytes = (ret_num_of_bytes > MAX_PID_LEN) ? 
                            MAX_PID_LEN : ret_num_of_bytes;    
        for (ix = 0; ix < ret_num_of_bytes; ix++) {
            ctrl_type[ix] = *(pdata + ix);
        }
    }

    *id = (ctrl_type[0] << 8 | ctrl_type[1]);

    return (PASSED);
}

/*-----------------------------------------------------------------------
 *
 * Function: get_ngio_mac_addr
 *
 * This functions read MAC addr information on Service Module cookie.
 *
 * Input : slot
 *
 * Output: MAC address.
 *
 *------------------------------------------------------------------------
 */
int
get_ngio_mac_addr (int slot, int type, uchar *board_mac_addr)
{
    int32_t i, retval = PASSED;
    uchar num_byte, *data_ptr;
    sc_context *con, cont;
    dev_if_info_t dev_if;

    con = &cont;
    con->dev_if_p = &dev_if;
    con->dev_if_p->cookie_size = COOKIE_SIZE_512;

    plat_init_smart_eeprom_context(con, type, slot, cookie_contents);
    act2_init_cont(con);

    /* mb is not using tam_act2_reset(), it has no delay between 
     * reset and unreset, non-aikido act2 needs delay */
    if (type == MOTHER_BOARD) {
        con->quack_reset(con);
    } else { 
        tam_act2_reset(0); 
    }

    if (cookie_is_act2(con)) {
        printf("Detecting ACT2 chip...\n");

        if (tam_lib_read_cookie() == FAILED) {
            printf("%s: Read cookie failed\n", __FUNCTION__);
            return (FAILED);
        }
    } else {
        printf("\nDetect Smart Cookie chip ...\n");
        if (smart_cookie_read(con) == FAILED) {
            printf("%s: Read cookie failed\n", __FUNCTION__);
            return (FAILED);
        }
    }

    if ((data_ptr = (uchar *) search_type_ret_addr_of_first_data
         (cookie_contents, (uchar) BOARD_MAC_ADDR,
          &num_byte, FALSE)) == (uchar *) NULL) {
        /*Search BOARD_MAC_ADDR failed. */
        for (i = 0; i < 6; i++) {         /* illegal code */
            board_mac_addr[i] = 0xff;
        }
        retval = FAILED;
    } else {
        for (i = 0; i < num_byte; i++) {
            board_mac_addr[i] = *data_ptr;
            data_ptr++;
        }
        retval = PASSED;
    }

    return retval;
}


/**************************************************************************
 *
 * Name: get_cookie_pid
 *
 * Description: read cookie id
 *
 * Inputs: slot - ngio slot
 *         type - ngio type
 *         eeprom_data - ngio cookie.
 *         pid - to restore pid as the func. read
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
int get_cookie_pid (int slot, int type, unsigned char *eeprom_data, char *pid)
{
    int32_t i;
    uchar num_byte, *data_ptr;

    /* using NGIO cookie structure ngio_intf_t->cookie as eeprom_data
     * to bypass accessing cookie again and again 
     */
    if ((data_ptr = (uchar *) search_type_ret_addr_of_first_data
         (eeprom_data, (uchar) PRODUCT_ID,
          &num_byte, FALSE)) == (uchar *) NULL) {
        /*Search CONTROLLER_TYPE failed. */
        pid[0] = 0;                /* illegal code */
        return(FAILED);
    } else {
        for (i = 0; i < num_byte; i++) {
            pid[i] = *data_ptr++;
        }
    }

    return(PASSED);

}

/*-------------------------------------------------------------------------
 * init_cookie_4_default_x()
 *
 * cookie default content for the new cookie TLV format
 * if brand new cookie or corrupt cookie content, user will
 * restore cookie content to this default
 *
 * Input : board_type - MB or I/O module index
 *         cookie_type - Red Baron MB or backplane cookie type
 *         contents - ptr returned that contains cookie data
 *         cookie_size -
 *
 * Output: none
 *
 *------------------------------------------------------------------------*/
void init_cookie_4_default_x (int board_type, int cookie_type,
                              uchar * contents, int cookie_size)
{
    switch (board_type) {
    case MOTHER_BOARD:
        printf("\nLoading default cookie format for Motherboard.\n");
        movbyte(default_cookie_contents, contents, cookie_size);
        break;
    default:

        movbyte(default_cookie_contents, contents, cookie_size);
        break;
    }
}

/**************************************************************************
 *
 * Name: alter_ngio_cookie
 *
 * Description: entry to alter ngio cookie.
 *
 * Inputs: cmd - pointer to cli_cookie_cmd
 *         ngio - pointer to struct ngio_intf_t
 *         type - ngio type.
 *
 * Outputs: None
 *
 *************************************************************************/
static int32_t
alter_ngio_cookie (cli_cookie_cmd *cmd, ngio_if *ngio, int type)
{
    int slot = ngio->slot;

    assert(ngio->slot);
    if (type == WIC_DAUGHTER_CARD || type == SM_DAUGHTER_CARD ||
        type == SM_VM_DAUGHTER_CARD) {
        assert(ngio->pc);
        slot = ngio->pc->slot;
    }

    /* using legacy to access */
    aikido_act2_flag = FALSE;
    aikido_mailbox_flag = FALSE;


    return (alter_cookie(type, slot)); 
}

/*
 * Function: alter_nim_cookie
 *
 * Description:
 *   This function is the entry point for the alter NIM ACT2
 *   cookie contents. It allow a user to edit/diaplay the contents of the
 *   cookie info within the ACT2 device.
 *
 * Parameters:
 *   slot - slot number 
 *
 * Returns:
 *   PASSED/FAILED
 */
int alter_nim_cookie (void)
{
    int max_slot, slot;
    struct ngio_intf_t *ngio;

    max_slot = get_max_wic_slots();
    slot = getdec_answer("Enter WIC slot number", 1, FIRST_SLOT,
                            max_slot);

    ngio = (struct ngio_intf_t *)slot_get_ngiowic(slot);

    if (slot_i2c_unreset(ngio, slot, "WIC") < 0) {
        return FAILED;
    }

    return (alter_ngio_cookie(NULL, ngio, WIC_MODULE));
}

/**************************************************************************
 *
 * Name: alter_nim_dc_cookie
 *
 * Description:  alter nim dc cookie via menu.
 *
 * Inputs: None
 *
 * Outputs: return value of alter_ngio_cookie()
 *
 *************************************************************************/
int alter_nim_dc_cookie (void)
{
    int max_slot, slot;
    struct ngio_intf_t *ngio, *dc;

    max_slot = get_max_wic_slots();
    slot = getdec_answer("Enter NIM slot containing daughter board", 1,
                          FIRST_SLOT, max_slot);

    ngio = (struct ngio_intf_t *)slot_get_ngiowic(slot);

    assert(ngio);
    assert(ngio->dc);

    dc = ngio->dc;

    if (slot_i2c_unreset(ngio, slot, "WICDC") < 0) {
        return FAILED;
    }
    return (alter_ngio_cookie(NULL, dc, WIC_DAUGHTER_CARD));
}

/**************************************************************************
 *
 * Name: alter_sm_cookie
 *
 * Description:  alter sm cookie via menu.
 *
 * Inputs: None
 *
 * Outputs: return value of alter_ngio_cookie()
 *
 *************************************************************************/
int alter_sm_cookie (void)
{
    int max_sm_slot, slot;
    struct ngio_intf_t *ngio;

    max_sm_slot = get_max_sm_slots();
    slot = getdec_answer("Enter SM slot number", 1, FIRST_SLOT,
                            max_sm_slot);

    if ((ngio = (struct ngio_intf_t *)slot_get_ngiosm(slot)) < 0 ) {
        return FAILED;
    }


    if (slot_i2c_unreset(ngio, slot, "SM") < 0) {
        return FAILED;
    }

    return alter_ngio_cookie(NULL, ngio, SM_MODULE);
}


/**************************************************************************
 *
 * Name: alter_sm_dc_cookie
 *
 * Description:  alter sm dc cookie via menu.
 *
 * Inputs: None
 *
 * Outputs: return value of alter_ngio_cookie()
 *
 *************************************************************************/
int alter_sm_dc_cookie (void)
{
    int max_sm_slot, slot;
    struct ngio_intf_t *ngio, *dc;

    max_sm_slot = get_max_sm_slots();
    slot = getdec_answer("Enter SM slot containing daughter board ", 1, FIRST_SLOT,
                            max_sm_slot);

    ngio = (struct ngio_intf_t *)slot_get_ngiosm(slot);

    assert(ngio);
    assert(ngio->dc);

    dc = ngio->dc;

    if (slot_i2c_unreset(ngio, slot, "SM") < 0) {
        return FAILED;
    }

    return alter_ngio_cookie(NULL, dc, SM_DAUGHTER_CARD);
}

/**************************************************************************
 *
 * Name: alter_sm_vm_dc_cookie
 *
 * Description:  alter sm vm dc cookie via menu.
 *
 * Inputs: None
 *
 * Outputs: return value of alter_ngio_cookie()
 *
 *************************************************************************/
int alter_sm_vm_dc_cookie (void)
{
    int max_sm_slot, slot;
    struct ngio_intf_t *ngio, *dc;

    max_sm_slot = get_max_sm_slots();
    slot = getdec_answer("Enter SM slot containing vm daughter board ", 1, FIRST_SLOT,
                            max_sm_slot);

    ngio = (struct ngio_intf_t *)slot_get_ngiosm(slot);

    assert(ngio);
    assert(ngio->dc);

    dc = ngio->dc;

    if (slot_i2c_unreset(ngio, slot, "SM") < 0) {
        return FAILED;
    }

    return alter_ngio_cookie(NULL, dc, SM_VM_DAUGHTER_CARD);
}

/**************************************************************************
 *
 * Name: alter_sm_dc_wic_cookie
 *
 * Description:  alter sm_dc_wic cookie via menu.
 *
 * Inputs: None
 *
 * Outputs: return value of alter_ngio_cookie()
 *
 *************************************************************************/
int alter_sm_dc_wic_cookie (void)
{
    int slot;
    struct ngio_intf_t *ngio;

    slot = getdec_answer("Enter SM Daughter WIC slot ", 1, FIRST_SLOT, 2);

    ngio = (struct ngio_intf_t *)get_sm_dc_wic_ngio(slot);

    assert(ngio);

    return (alter_cookie(SM_DC_WIC_CARD, ngio->slot));
}

/**************************************************************************
 *
 * Name: alter_sm_dc_wic_dc_vm_cookie
 *
 * Description:  alter sm_dc_wic_dc_vm cookie via menu.
 *
 * Inputs: None
 *
 * Outputs: return value of alter_ngio_cookie()
 *
 *************************************************************************/
int alter_sm_dc_wic_dc_vm_cookie (void)
{
    int slot;
    struct ngio_intf_t *ngio;

    slot = getdec_answer("Enter SM Daughter WIC slot ", 1, FIRST_SLOT, 2);

    ngio = (struct ngio_intf_t *)get_sm_dc_wic_ngio(slot);

    assert(ngio);

    return (alter_cookie(SM_DC_WIC_DC_VM_CARD, ngio->slot));
}

/*
 * Function: alter_poe_cookie
 *
 * Description:
 *   This function is the entry point for the alter PoE ACT2
 *   cookie contents. It allow a user to edit/diaplay the contents of the
 *   cookie info within the ACT2 device.
 *
 * Parameters:
 *   NONE
 *
 * Returns:
 *   PASSED/FAILED
 */
int alter_poe_cookie (void)
{
    /* not support yet */
    return (PASSED); 
}

/*
 * Function: alter_raid_cookie
 *
 * Description:
 *   This function is the entry point for the alter RAID ACT2
 *
 * Parameters:
 *   NONE
 *
 * Returns:
 *   PASSED/FAILED
 */
int alter_raid_cookie (void)
{
    /* not support yet */
    return (PASSED); 
}

/*
 * Function: alter_mb_cookie
 *
 * Description:
 *   This function is the entry point for the alter motherboard ACT2
 *   cookie contents. It allow a user to edit/diaplay the contents of the
 *   cookie info within the ACT2 device.
 *
 * Parameters:
 *   NONE
 *
 * Returns:
 *   PASSED/FAILED
 */
int alter_mb_cookie (void)
{
    int  act2_chip;


    act2_chip = getdec_answer("\n(1-Aikido-ACT2):", 1, 0, 1);
    if (act2_chip == 0) {
        aikido_act2_flag = FALSE;
        aikido_mailbox_flag = FALSE;
        printf("\nSelect Discrete ACT2\n");
    } else if (act2_chip == 1) {
        aikido_act2_flag = TRUE;
        aikido_mailbox_flag = TRUE;
        printf("\nSelect AIKIDO MAIL Box\n");
    } else {
        printf("\n %s:%d Never come here \n", __FUNCTION__, __LINE__); 

    }
    return (alter_cookie(MOTHER_BOARD, 0));
}

/**************************************************************************
 * Function: alter_pim_cookie
 *
 * Description:
 *   This function is the entry point for the alter plug ACT2
 *   cookie contents. It allow a user to edit/diaplay the contents of the
 *   cookie info within the ACT2 device.
 *
 * Parameters:
 *    none
 *
 * Returns:
 *   PASSED/FAILED
 *************************************************************************/
int alter_pim_cookie(void)
{
    int max_slot, slot;
    struct plug_intf_t *plug;
    sc_context *con, cont;
    dev_if_info_t dev_ifl;
    uchar cookie_contents[COOKIE_SIZE_512];

    max_slot = get_max_pim_slots();
    slot = getdec_answer("Enter PLUG slot number", 1, FIRST_SLOT,
            max_slot);
  
    plug = (struct plug_intf_t *)slot_get_plugslot(slot);

    if (plug_slot_i2c_poweron_unreset(plug, slot, "PLUGGABLE_CARD") != PASSED) {
        printf("Unable to unreset PIM module slot %d\n", slot);
        return (FAILED);
    }

    con = &cont;
    con->dev_if_p = &dev_ifl;
    con->dev_if_p->cookie_size = COOKIE_SIZE_512;

    aikido_act2_flag = FALSE;
    aikido_mailbox_flag = FALSE;

    if (plat_init_smart_eeprom_context (con, PLUGGABLE_CARD, 
                                        slot, (uchar *)cookie_contents)) {
        return (FAILED);
    }

    act2_init_cont(con);
    
    if (alter_cookie(PLUGGABLE_CARD, slot) != PASSED ) {
        return (FAILED);
    }
    
    return (PASSED);

}

/*************************************************************************
 * Function: tam_lib_read_cookie
 *
 * This function is read the MB cookie content by tam library
 *
 * Input: none
 *
 * Output: PASSED / FAILED
 *************************************************************************
 */
static int tam_lib_read_cookie (void)
{
    int ret_val;
    void *platform_opaque_handle = NULL;
    void *platform_tam_handle = NULL;

    if (aikido_mailbox_flag) {
        /* Initialize Mailbox */
        ret_val = tam_lib_device_open_mailbox(platform_opaque_handle, use_interrupt,
                                              MBX_MSG_SIZE, MBX_REG_BASE_ADDR,
                                              &platform_tam_handle);
        if (ret_val != TAM_RC_OK) {
            /* handle error */
            printf("\n ERROR: Cannot Initialize Mailboxe. status 0x%x\n", ret_val);
            return (FAILED);
        }
    } else {
        if (platform_tam_handle == NULL) {

            ret_val = tam_lib_device_open(platform_opaque_handle,
                                          PLATFORM_BUFF_SIZE, /*=259 */
                                          &platform_tam_handle);

            if (ret_val != TAM_RC_OK) {
                printf("\n TAM lib: Cannot open handler: status = 0x%x", ret_val);
                return (FAILED);
            }
        }
    }

    ret_val = tam_lib_scc_read_eeprom(platform_tam_handle, 
                                      (unsigned char *) cookie_contents,
                                      EEPROM_RD_WR_LENGTH, EEPROM_WRITE_ADDR);

    if (ret_val != TAM_RC_OK) {
        printf("\n *** ERROR: tam_lib_scc_read_cookie. ret_val = 0x%x", ret_val);
        printf("\n Cannot read data from the EEPROM (Cookie). \n");
        return (FAILED);
    }

    ret_val = tam_lib_device_close(&platform_tam_handle);
    if (ret_val != TAM_RC_OK) {
        printf("\n TAM lib: Cannot close handler: status = 0x%x", ret_val);
        return (FAILED);
    }
    
    return (PASSED);
}


/*
 * Function: alter_cookie
 *
 * Description:
 *   This function is the entry point for the alter ACT2
 *   cookie contents. It allow a user to edit/diaplay the contents of the
 *   cookie info within the ACT2 device.
 *
 * Parameters:
 *   board_type - M/B, NIM
 *   slot - Slot number
 *
 * Returns:
 *   PASSED/FAILED
 */
static int alter_cookie (int board_type, int slot)
{
    void *tam_handle_ptr;
    void *platform_opaque_handle = NULL;
    uint16_t platform_buffer_size = PLATFORM_BUFF_SIZE;
    uint8_t cookie_contents_buf[COOKIE_SIZE_512];
    tam_lib_status_t status;
    int ret_val;
    sc_context *con, cont;
   
    con = &cont;
    con->dev_if_p = &dev_if;
    con->dev_if_p->cookie_size = COOKIE_SIZE_512;

    if (plat_init_smart_eeprom_context(con, board_type, slot, 
                                      (uchar *)cookie_contents_buf) == FAILED) {
        printf("\n%s: Init Smart EEPROM context failed\n", __func__);
        return (FAILED);
    }

    act2_init_cont(con);

    /* mb is not using tam_act2_reset(), it has no delay between 
     * reset and unreset, non-aikido act2 needs delay */
    if (board_type == MOTHER_BOARD   ||
        board_type == SM_DC_WIC_CARD || board_type == SM_DC_WIC_DC_VM_CARD) {
        con->quack_reset(con); 
    } else { 
        tam_act2_reset(0); 
    }

    if (board_type != MOTHER_BOARD) { /* NGIO has no aikido so far */
        if (!cookie_is_act2(con)) { 
            return (smart_cookie_read_write_eeprom(con, NULL));
        }
    }

    tam_handle_ptr = NULL;
    if (is_tam_aikido_mbox_on()) {
        /* Initialize Mailbox */
        status = tam_lib_device_open_mailbox(platform_opaque_handle, 
                                            use_interrupt,
                                            MBX_MSG_SIZE,
                                            MBX_REG_BASE_ADDR,
                                            &tam_handle_ptr);
        if (status != TAM_RC_OK) {
            /* handle error */
            printf("\n ERROR: Cannot Initialize Mailboxe. status 0x%x\n", status);
            return (FAILED);
        }

    } else {
        status = tam_lib_device_open(platform_opaque_handle,
                                     platform_buffer_size, &tam_handle_ptr);
                                 
        if (status != TAM_RC_OK) {
            printf("\n TAM lib: Cannot open handler: status = 0x%x", status);
            printf("\n Chip might be quack, not act2\n"); 
            return (FAILED);
        }                   
    }
    
    status = tam_lib_scc_read_eeprom(tam_handle_ptr, (uint8_t *)cookie_contents_buf,
                                     EEPROM_RD_WR_LENGTH, EEPROM_WRITE_ADDR);          
    
    if (status != TAM_RC_OK) {
        printf("\n *** ERROR: tam_lib_scc_read_cookie. status = 0x%x",
               status);
        printf("\n Cannot read data from the EEPROM (Cookie). \n");
        return (FAILED);
    }
    
    ret_val = cookie_4_processor_x(cookie_contents_buf, board_type, 0,
                                   EEPROM_RD_WR_LENGTH, NULL);
                                   
    if (ret_val) {
        status = tam_lib_scc_write_eeprom(tam_handle_ptr,
                                         (uint8_t *) cookie_contents_buf,
                                          EEPROM_RD_WR_LENGTH, EEPROM_WRITE_ADDR);
                                          
        if (status != TAM_RC_OK) {
            printf("\n *** ERROR: tam_lib_scc_write_eeprom. status = 0x%x",
                   status);
            printf("\n Cannot write data to the EEPROM (Cookie). \n");
            return (FAILED);
        }
    }    
    
    status = tam_lib_device_close(&tam_handle_ptr);
    if (status != TAM_RC_OK) {
        printf("\n TAM lib: Cannot close handler: status = 0x%x", status);
        return (FAILED);
    }

    return (status);
}


/*
 * Function: alter_cookie
 * Description:
 *   not support on O2, but called on common code. ****
 *
 * Parameters:
 *   eeprom_data 
 *   serial 
 *   ser_tlv_field
 *
 * Returns:
 *   PASSED/FAILED
 */

int get_tlv_serial(uchar *eeprom_data, char *serial, uchar ser_tlv_field)
{
    int ix;
    uint8_t *pdata = NULL;
    uint8_t ret_num_of_bytes = 0;
    char prodSN[PRODUCT_SERIAL_LEN] = { 0 };
    
    if (tam_lib_read_cookie() == FAILED) {
        printf("%s: Read cookie failed\n", __FUNCTION__);
        return (FAILED);
    }

    pdata = search_type_ret_addr_of_first_data(cookie_contents, ser_tlv_field,
                                               &ret_num_of_bytes, 0);
    
    if (pdata == NULL) {
        printf("\n *** ERROR: 0x%02x Serial Number field not programmed per Product PCAMAP.",
	       ser_tlv_field);
        return (FAILED);
    } else {
        ret_num_of_bytes = (ret_num_of_bytes > MAX_PID_LEN) ? 
                            MAX_PID_LEN : ret_num_of_bytes;    
        for (ix = 0; ix < ret_num_of_bytes; ix++) {
            prodSN[ix] = *(pdata + ix);
        }
    }

    sprintf(serial, "%s", prodSN);

    return (PASSED);
}

/**************************************************************************
 *
 * Name: get_cookie_field_val
 *
 * Description: This function will return the cookie field value.
 *
 * Inputs: field_type - cookie field type
 *         *field_val - pointer to field value
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
int get_cookie_field_val (int field_type, char *field_val)
{
    int ix;
    uint8_t *pdata = NULL;
    uint8_t ret_num_of_bytes = 0;
    void *tam_handle_ptr;
    void *platform_opaque_handle = NULL;
    uint16_t platform_buffer_size = PLATFORM_BUFF_SIZE;
    uint8_t cookie_contents_buffer[COOKIE_SIZE_512];
    tam_lib_status_t status;
    sc_context *con, cont;

    con = &cont;
    con->dev_if_p = &dev_if;
    con->dev_if_p->cookie_size = COOKIE_SIZE_512;

    if (plat_init_smart_eeprom_context(con, MOTHER_BOARD, 0,
                                      (uchar *)cookie_contents_buffer) == FAILED) {
        printf("\n%s: Init Smart EEPROM context failed\n", __func__);
        return (FAILED);
    }

    act2_init_cont(con);
    /* mb is not using tam_act2_reset(), it has no delay between 
     * reset and unreset, non-aikido act2 needs delay */
    /* for mb using quack_reset() instead of tam_act2_reset() */
    con->quack_reset(con); 

    tam_handle_ptr = NULL;
    status = tam_lib_device_open(platform_opaque_handle,
                                 platform_buffer_size, &tam_handle_ptr);

    if (status != TAM_RC_OK) {
        printf("\n TAM lib: Cannot open handler: status = 0x%x", status);
        return (FAILED);
    }

    status = tam_lib_scc_read_eeprom(tam_handle_ptr, (uint8_t *)cookie_contents_buffer,
                                     EEPROM_RD_WR_LENGTH, EEPROM_WRITE_ADDR);

    if (status != TAM_RC_OK) {
        printf("\n *** ERROR: tam_lib_scc_read_cookie. status = 0x%x",
               status);
        printf("\n Cannot read data from the EEPROM (Cookie). \n");
        return (FAILED);
    }

    status = tam_lib_device_close(&tam_handle_ptr);
    if (status != TAM_RC_OK) {
        printf("\n TAM lib: Cannot close handler: status = 0x%x", status);
        return (FAILED);
    }
   
    pdata =
        search_type_ret_addr_of_first_data(cookie_contents_buffer, field_type,
                                           &ret_num_of_bytes, 0);
  
    /* Get the corresponded cookie field value. */
    if (pdata == NULL) {
        printf("\n *** ERROR: Fab Version field not programmed per Product PCAMAP.\n");
        return (FAILED);
    } else {
        for (ix = 0; ix < ret_num_of_bytes; ix++) {
            field_val[ix] = *(pdata + ix);
        }
    }

    return (PASSED);
}
  
/*---------------------------------------------------------------
$Log: platform_tam_cookie.c,v $
Revision 1.9  2021/02/24 03:46:27  xiaolaya
Fix bug for Switzer-Carrier SM Daughter NIM Daughter VM cookie

Revision 1.8  2021/01/12 04:04:58  xiaolaya
switzer-carrier daughter card eeprom access bug fix

Revision 1.7  2020/01/31 07:17:40  leschen
Support Curie new cookie.

Revision 1.6  2020/01/09 01:02:20  jiajliu
Merge Curie 2RU to main trunk

Revision 1.5  2019/12/31 07:48:06  alpeng
revert cookie to old one for RDT

Revision 1.4  2019/12/21 00:52:38  ptong
Curie PID change to C8300-1N1S-4T2X and C8300-1N1S-6G

Revision 1.3  2019/12/18 09:18:37  alpeng
1. support quack cookie rd/wr; 2. fixed new rommon break nightwatch issue; 3. a workaround for new pim testcard crashed system issue; 4. bump to v2.0.1 for curie

Revision 1.2  2019/08/06 06:56:10  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.1.2.13  2019/04/26 03:05:43  alpeng
Change Curie 1RU PIDs from CE8300 to C8300

Revision 1.1.2.12  2019/04/19 09:46:05  alpeng
mb act2 cookie i2c read/write is disable by hw; diag v1.3.3 is cover act2 i2c support

Revision 1.1.2.11  2019/03/18 22:16:26  ptong
Change Curie 1RU PIDs from CE9331 to CE8300

Revision 1.1.2.10  2019/02/19 06:32:51  alpeng
add a delay to resolve wdc polling cookie problem

Revision 1.1.2.9  2019/02/12 06:24:27  meho
Support new PIM test-card (PCIe)

Revision 1.1.2.8  2019/01/17 06:36:57  alpeng
read cookie from aikido for p1c and later

Revision 1.1.2.7  2019/01/14 10:16:59  alpeng
update default cookie contents; fixed size overflow on spi rd/wr

Revision 1.1.2.6  2018/12/12 03:47:37  meho
Re-named PIM prompt in ACT2 programming util

Revision 1.1.2.5  2018/12/12 03:43:18  meho
Added alter PIM cookie utility

Revision 1.1.2.4  2018/11/01 23:18:45  ptong
Change Curie 1RU PID to new CE9331 value. Remove Neptunium and Polonium SKU

Revision 1.1.2.3  2018/10/16 09:03:01  meho
Pluggable re-structured.

Revision 1.1.2.2  2018/10/05 10:09:10  alpeng
support PIM cookie

Revision 1.1.2.1  2018/09/27 09:46:23  alpeng
support tam lib and aikido for curie

Revision 1.7  2018/05/09 03:53:36  hondwang
Fix F2W P2 board issue

Revision 1.6  2017/05/22 06:44:19  haohsu
Modify Quack showing PID and SN number

Revision 1.5  2017/03/30 08:30:53  hondwang
Tachi-L brach merge

Revision 1.4  2017/01/25 01:13:13  kodko
Get the Fab Version and PCB Revision cookie field values to distinguish the board type and do the USB3.0/USB2.0 or USB3.0 only test.

Revision 1.3.12.1  2016/11/04 19:08:54  benchen2
Modify Enhanced error message

Revision 1.3  2016/06/06 18:30:40  huanngo
Add code to support programming SUDI/WDC with Chassis S/N

Revision 1.2  2016/04/20 11:25:31  benchen2
add tachi fru portion

Revision 1.1.2.22  2016/04/18 07:00:47  benchen2
according to prrq fix isp define

Revision 1.1.2.21  2016/03/30 06:35:00  alpeng
fix get_mb_pid

Revision 1.1.2.20  2016/03/29 02:36:33  alpeng
update get_pid, get cookie from ngio struct instead using act2 again

Revision 1.1.2.19  2016/03/16 09:00:34  alpeng
update sku for check poe and non-poe

Revision 1.1.2.18  2016/03/08 06:46:48  alpeng
update get_cookie_pid, leverage ngio structure to speed up

Revision 1.1.2.17  2016/03/04 09:40:53  alpeng
update testcard enhance err msg

Revision 1.1.2.16  2016/01/28 19:10:11  huanngo
Adding the display of platform SKU when starting BMC diag app

Revision 1.1.2.15  2016/01/26 06:33:34  benchen2
add daughter card ACT2 programming

Revision 1.1.2.14  2016/01/18 12:07:27  alpeng
act2 reset before access it for nim, following MB act2

Revision 1.1.2.13  2016/01/18 07:02:28  alpeng
update cookie info for read mac

Revision 1.1.2.12  2015/12/29 12:31:06  alpeng
support get_mb_pid for check MB sku

Revision 1.1.2.11  2015/12/22 17:26:03  huanngo
Support Tachi-High tests and menus CSCux15587

Revision 1.1.2.10  2015/11/02 10:22:55  tirawan
Add PoE Cookie Utility

Revision 1.1.2.9  2015/10/28 07:55:04  benchen2
add raid act2 cookies utility

Revision 1.1.2.8  2015/09/30 09:15:29  benchen2
add poe act2

Revision 1.1.2.7  2015/09/17 13:05:10  tirawan
Add Bios Boot Flash Test, fixes I2C controller number for NIM

Revision 1.1.2.6  2015/08/31 06:42:08  tirawan
Ported legacy smart cookie to support Quack chip read as TAM library cookie read function doesn't work on Quack chip

Revision 1.1.2.5  2015/08/30 05:57:35  tirawan
To support NIM ACT2 R/W access using TAM library

Revision 1.1.2.4  2015/08/28 02:33:52  tirawan
To support ACT2 M/B cookie programming using Foxconn FPGA

Revision 1.1.2.3  2015/07/31 10:39:59  alpeng
first check in for testcard

Revision 1.1.2.2  2015/07/26 06:02:21  tirawan
Add FPGA I2C read/write function and ACT2 cookie/smartchip programming

Revision 1.1.2.1  2015/07/24 03:39:36  tirawan
Add FPGA I2C read/write function, ACT2 cookie read/write function


$Endlog$
*/

