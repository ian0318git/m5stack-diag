/* $Id: platform_cookie.c,v 1.32 2020/02/03 01:35:09 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/platform_cookie.c,v $
 *------------------------------------------------------------------
 *
 * platform_cookie.c - Specific MB cooke support from Xformers.
 *
 * Sept. 2008, Shih-Nan Huang
 *
 * Copyright (c) 2014-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <linux/hdreg.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include "types.h"
#include "error.h"
#include "common.h"
#include "cross_platform.h"
#include "mon_plat_defs.h"
#include "nmc93c46.h"
#include "menu.h"
#include "goofy_i2c.h"
#include "proto.h"
#include "i2c_api.h"
#include "platform_i2c.h"
#include "cli_cmd.h"
#include "smart_cookie.h"
#include "cookie_4.h"
#include "i2c_address.h"
#include "queryflags.h"
#include "platform_30w_poe.h"
#include "platform_cookie.h"
#include "platform_slot.h"
#include "platform_i2c.h"
#include "nvmonvars.h"
#include "plat_defs.h"
#include "dash_fpga.h"
#include "ngio.h"
#include "slot.h"


/* ref. blockdev.c for return device (SATA) size in bytes u64 */

extern unsigned int use_mb_quack;
/* for now support eeprom */
extern int smart_cookie_read_x(sc_context *, ushort);
extern int cookie_4_processor_x(uchar *, int,
                                int, int, cli_cookie_cmd *);
int get_pid(uchar *, char *);
int get_mb_pid(char *);

/* for now support PoE cookie */
int alter_poe_cookie(void);
static int alter_ilp_cookie_x(boolean, cli_cookie_cmd *);
static int32_t init_cli_cmd (cli_cookie_cmd *, struct ngio_intf_t *,
                                    char *);
/*----------------------------------------------------------------------------
 * Defines
 *--------------------------------------------------------------------------*/
#define QUACK_RETRY 8

/*------------------------------------------------------------------
 * Externs
 *-----------------------------------------------------------------*/


/*---------------------------------------------------------------------------
 * Globals
 *--------------------------------------------------------------------------*/
void i2c_quack_reset(sc_context *con_p);
int i2c_quack_write_bytes(sc_context *con_p, char *tx_buffer, int tx_size);
int i2c_quack_read_bytes(sc_context *con_p, char *rx_buffer);
boolean aikido_mailbox_flag;
boolean aikido_act2_flag;


static char smc_buf[80];
static char i2c_err[80];

/* Default cookie contents for Sunridges PoE Powerball */
static uchar default_mb_cookie[COOKIE_SIZE_512] = {
    0x04,0xFF,0x40,0x07,0x6E,0x41,0x01,0x00,
    0x83,0x4A,0x1D,0xC1,0x01,0x42,0x30,0x31,
    0x88,0x00,0x00,0x00,0x00,0x02,0x01,0xC1,
    0x8B,0x00,0x00,0x00,0x00,0x00,0x03,0x00,
    0x81,0x00,0x00,0x00,0x00,0x04,0x00,0xCB,
    0x86,0x50,0x4F,0x57,0x45,0x52,0x42,0x41,
    0x4C,0x4C,0x89,0x56,0x30,0x30,0x20,0xD9,
    0x04,0x40,0xC1,0xCB,0xC6,0x8A,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
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
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
};

static uchar default_cookie_contents[COOKIE_SIZE_512] = {
 0x04 ,0xff ,0x40 ,0x07 ,0x83 ,0x41 ,0x01 ,0x00,
 0x82 ,0x49 ,0x38 ,0xaa ,0x03 ,0x88 ,0x00 ,0x02,
 0x01 ,0xfe ,0x02 ,0x03 ,0x85 ,0x1c ,0x2a ,0x59,
 0x03 ,0xc1 ,0x8b ,0x46 ,0x4f ,0x43 ,0x31 ,0x36,
 0x34, 0x36, 0x34, 0x44, 0x4d, 0x52, 0x03, 0x00,
 0x81, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0xcb,
 0x90, 0x4e, 0x49, 0x4d, 0x2d, 0x34, 0x4d, 0x46,
 0x54, 0x2d, 0x54, 0x31, 0x2f, 0x45, 0x31, 0x20,
 0x20, 0x89, 0x56, 0x30, 0x31, 0x20, 0xd9, 0x03,
 0x40, 0xc1, 0xcb, 0xc6, 0x8a, 0x49, 0x50, 0x39,
 0x49, 0x41, 0x4e, 0x36, 0x43, 0x41, 0x41, 0xc0,
 0x46, 0x03, 0x20, 0x00, 0x96, 0x82, 0x03, 0x42,
 0x30, 0x31, 0xf3, 0x00, 0x06, 0x40, 0x02, 0x61,
 0x43, 0x07, 0x00, 0xcf, 0x06, 0x0c, 0xd9, 0x96,
 0xa8, 0x04, 0xb0, 0x43, 0x00, 0x02, 0xc9, 0x0b,
 0x02, 0x01, 0x01, 0x07, 0x00, 0x00, 0x00, 0x00,
 0x01, 0x01, 0x00, 0xc4, 0x08, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff,
 0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff,
 0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff,
 0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff,
 0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff,
 0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff,
 0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff,
 0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff,
 0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff,
 0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff,
 0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff,
 0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff,
 0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff,
 0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff,
 0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff,
 0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff,
 0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff,
 0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff,
 0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff,
 0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff,
 0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff,
 0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff,
 0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff,
 0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff,
 0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff,
 0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff,
 0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff,
 0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff,
 0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff,
 0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff,
 0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff,
 0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff,
 0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff,
 0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff,
 0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff,
 0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff,
 0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff,
 0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff,
 0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff,
 0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff,
 0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff,
 0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff,
 0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff,
 0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff,
 0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff,
 0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff,
 0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff,
};

/*--------------------------------------------------------------------
 * Function: quack_err_str
 *
 * Decription: return module type and slot number, given cntrl number
 * 
 * Input: val - controller number
 * 
 * Output: error string
 *
 *--------------------------------------------------------------------
 */
const char *
quack_err_str (int val)
{
    switch (val) {
    case SM1_I2C_CTRL:
        return "SM1";
    case SM2_I2C_CTRL:
        return "SM2";
    case WIC1_I2C_CTRL:
        return "WIC1";
    case WIC2_I2C_CTRL:
        return "WIC2";
    case WIC3_I2C_CTRL:
        return "WIC3";
    case VM_I2C_CTRL:
        return "VM";
    default:
        break;
    }
    return("invalid controller number");
}
                                  
/*--------------------------------------------------------------------
 * alter_mb_cookie()
 *
 * This functions displays/modifies the cookie on the motherboard.
 * 
 * Input: none.
 * 
 * Output: PASSED/FAILED
 *
 *--------------------------------------------------------------------
 */
int 
alter_mb_cookie (void)
{ 
    return (alter_mb_cookie_x(MENU_MODE, NULL));
}
/*--------------------------------------------------------------------
 * alter_mb_cookie_x()
 *
 * This functions displays/modifies the cookie on the motherboard.
 * 
 * Input: mode, TRUE is CLI ; FALSE is MENU
 *        cli cmd structure. 
 * 
 * Output: PASSED/FAILED
 *
 *--------------------------------------------------------------------
 */
int 
alter_mb_cookie_x (boolean mode, cli_cookie_cmd *cmd)
{
    uint16_t cookie_size = COOKIE_SIZE_256;
    uchar cookie_contents[COOKIE_SIZE_512];
    sc_context *con, cont;
    dev_if_info_t dev_if;
    ushort cookie_id;
     
    /* bool TRUE in CLI mode*/
    if(mode == CLI_MODE) {
        /* Get Controller type */
        cookie_id = get_mb_id();
        if(cookie_id == INVALID_ID){
            if (cmd->cli_mode == CLI_DISCOVERY) {
                prpass(testpass, "MB:get_mb_id");
                cterr('f',0,"MB:Controller type Invalid");
                return (FAILED);
            }else
            if (cmd->cli_mode == CLI_COOKIE){
                printf("\nMB:Controller type Invalid");
            }else {
                cterr('f',0,"\nUnknown cli mode");
                return (FAILED);
            }
        }
    }

    con = &cont;
    con->dev_if_p = &dev_if;
    con->dev_if_p->cookie_size = cookie_size;

    if (plat_init_smart_eeprom_context(con, MOTHER_BOARD,
                                    0, (uchar *)cookie_contents)) {
	return(FAILED);
    }


    if (smart_cookie_read_write_eeprom(con, cmd)) {
        return FAILED;
    }
    return PASSED;


}

/*--------------------------------------------------------------------
 * alter_ilp_cookie_x()
 *
 * This functions modifies the cookie for Powerball
 *
 * Input: mode, TRUE is CLI ; FALSE is MENU
 *        cli cmd structure.
 *
 * Output: PASSED/FAILED
 *
 *--------------------------------------------------------------------
 */
int alter_ilp_cookie_x (boolean mode, cli_cookie_cmd *cmd)
{
    int ret_val = PASSED;
    uint16_t cookie_size = 512;
    uchar cookie_contents[cookie_size];
    sc_context *con, cont;
    dev_if_info_t dev_if;
    char errbuf[80];
    

    /* unreset the i2c bit 8, for accessing ilp. */
    unreset_platform_in_dev(FPGA_IN_I2C_8_RST);

    /* Check the presence of the module if it is discovery command */
    if (cmd != NULL) {
        if (mode == CLI_MODE && cmd->cli_mode == CLI_DISCOVERY) {
            if (is_poe_present(errbuf) == FALSE) {
                /* Report the module is not present in discovery */
                cterr('f', 0, "unable to access 30w POE %s. does device exist?",
                      errbuf);
                printf("\n%sNone\n", cmd->buf);
                return (FAILED);
            }
        }
    }
    con = &cont;
    con->dev_if_p = &dev_if;
    con->dev_if_p->cookie_size = cookie_size;
    if (plat_init_smart_eeprom_context (con, DAUGHTER_CARD,
                                        POE_DAUGHTER_CARD, (uchar *)cookie_contents)) {
        return (FAILED);
    }

    if (smart_cookie_read_write_eeprom(con, cmd)) {
        ret_val = FAILED ;
    }
    clean_smart_eeprom_context(con);    /* Unlock I2C Bus */
    return (ret_val);
}


/*--------------------------------------------------------------------
 * alter_poe_cookie_x()
 *
 * This functions displays/modifies the cookie on the motherboard
 * Powerball
 *
 * Input: mode, TRUE is CLI ; FALSE is MENU
 *        cli cmd structure.
 *
 * Output: PASSED/FAILED
 *
 *--------------------------------------------------------------------
 */
int alter_poe_cookie_x (boolean mode, cli_cookie_cmd *cmd)
{
    alter_ilp_cookie_x(CLI_MODE, cmd);
    return (PASSED);
}

/*************************************************************************
 *
 * Name: alter_poe_cookie
 *
 * Description: This functions displays/modifies the cookie on the 
 *              Powerball on Sunridges platform (MENU entry)
 * 
 * Inputs: none.
 * 
 * Outputs: none.
 *
 ************************************************************************/
int alter_poe_cookie (void)
{
    /* 
    * Menu driven entry point
    */
    alter_ilp_cookie_x(MENU_MODE, NULL);
    return (PASSED);
}

/*--------------------------------------------------------------------
 * alter_bp_cookie()
 *
 * This functions displays/modifies the cookie on the mid raise
 *                  back plane..
 * 
 * Input: none.
 * 
 * Output: PASSED/FAILED
 *
 *--------------------------------------------------------------------
 */
int
alter_bp_cookie (void)
{
    cterr('f', 0, "alter backplane cookie not supported");
    return FAILED;
}

/*-----------------------------------------------------------------------------
 *
 * Function  get_hwic_cookie_id
 *
 * This function will get the cookie id value of the HWIC that is installed
 * in the HWIC slot.
 *
 * Input - hwic_num: the hwic slot number
 *       - spi_cookie_p: pointer to spi cookie id structure
 *       - wic_type: the type of WIC in the slot ( older type or voice type )
 *
 * Output - cookie id
 */
ushort
get_hwic_cookie_id (int hwic_num, void *spi_cookie_p, uchar wic_type)
{
    unsigned cookie_id = 0;

    /* fixme */
    printf("fixme: this function is depreciated, please consider to use get_cookie_id()\n");

    printf("get_hwic_cookie_id: %d\n", cookie_id);
    
    return(cookie_id);
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
    con_p->quack_read_2bytes = (PFT)i2c_quack_read_bytes;
    con_p->quack_write_2bytes = (PFT)i2c_quack_write_bytes;
    con_p->quack_reset = (PFT)i2c_quack_reset;
    
    con_p->dev_if_p->parm1 = (uint8_t)IOFPGA_I2C;
    con_p->dev_if_p->parm3 = (uint8_t)NGIO_I2C_MUX_ACT2;

    con_p->dev_if_p->interface = SCC_I2C_IF;        

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
int
plat_init_smart_eeprom_context (sc_context *con_p, uchar type,
                                uchar slot, uchar *cookie_p)
{
    int retval = PASSED, max_sm_slot;
    struct ngio_intf_t *ngiowic;

    *i2c_err = '\0';
    
    con_p->info_string = smc_buf;

    switch (type) {
    case MOTHER_BOARD: /* I2C interface */
        con_p->type = type;
        con_p->slot = slot;        /* slot = 0 */
        con_p->cookie_contents = cookie_p;
        con_p->quack_read_2bytes = (PFT)i2c_quack_read_bytes;
        con_p->quack_write_2bytes = (PFT)i2c_quack_write_bytes;
        con_p->quack_reset = (PFT)i2c_quack_reset;
        con_p->dev_if_p->parm1 = (uint8_t)IOFPGA_I2C;
        con_p->dev_if_p->parm2 = (uint8_t)MB_I2C_ADDR_ACT2;
        con_p->dev_if_p->parm3 = (uint8_t)MB_I2C_MUX_ACT2;
        con_p->dev_if_p->parm4 = (uint8_t)MB_I2C_CTRL_ACT2;
        con_p->dev_if_p->interface = SCC_I2C_IF;
        sprintf((char *)smc_buf, "MB");
        break;
    case BACK_PLANE:
        assert(!"back plane not suporeted in platform_cookie.c\n");
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
            /*  printf("wic daughter: i2c ctrl no %d; addr %#x\n",
                con_p->dev_if_p->parm4, con_p->dev_if_p->parm2);
            */
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
            /* printf("sm daughter card: i2c ctrl # %d;  addr %#x\n",
               con_p->dev_if_p->parm4, con_p->dev_if_p->parm2);
            */
        }

        break;
    case VM_MODULE:
        sprintf((char *)smc_buf, "VM MODULE ");
        con_p->dev_if_p->parm4 = (uint8_t)VM_I2C_CTRL;
        init_ngio_context(con_p, type, slot, cookie_p);
        con_p->dev_if_p->parm2 = (uint8_t)NGIOVM_I2C_ADDR_ACT2;
        /* printf("vm module: i2c ctrl: %d ;addr %#x\n",
           con_p->dev_if_p->parm4, con_p->dev_if_p->parm2);
        */
        break;
    case DAUGHTER_CARD:
        printf("30WPoE daughter card \n");
        con_p->type = type;
        con_p->slot = 1;
        con_p->cookie_contents = cookie_p;
        con_p->quack_read_2bytes = (PFT)i2c_quack_read_bytes;
        con_p->quack_write_2bytes = (PFT)i2c_quack_write_bytes;
        con_p->quack_reset = (PFT)i2c_quack_reset;   
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
    default:
        cterr('f',0,"in plat_init_smart_eeprom_context: Not a supported "
              "Smart EEPROM type %d", type);
        assert(!"plat_init_eeprom: invalid argument");
        retval = FAILED;
    }

    return (retval);
}


/*-----------------------------------------------------------------------
 *
 * Function: print_bp_slot
 *  
 * This functions prints information on modules installed on Back Plane.
 *
 * Input : none
 * 
 * Output: none.
 *
 *------------------------------------------------------------------------
 */
void
print_bp_slot (void)
{
    cterr('f', 0, "Mid plane cookie not supported");
    return;
}

/*-----------------------------------------------------------------------
 *
 * Function: get_mb_id
 *  
 * This functions read cookie information on Mother Board cookie.
 *
 * Input : none
 * 
 * Output: none.
 *
 *------------------------------------------------------------------------
 */
ushort
get_mb_id(void)
{
    int rc, cookie_size = COOKIE_SIZE_512;
    uchar cookie_contents[cookie_size];
    ushort cntl_type = INVALID_ID;
    uchar num_byte, *data_ptr;
    sc_context *con, cont;
    dev_if_info_t dev_if;

    con = &cont;
    con->dev_if_p = &dev_if;
    con->dev_if_p->parm1 = (uint8_t)IOFPGA_I2C;
    con->dev_if_p->parm2 = (uint8_t)MB_I2C_ADDR_ACT2;
    con->dev_if_p->parm3 = (uint8_t)MB_I2C_MUX_ACT2;
    con->dev_if_p->parm4 = (uint8_t)MB_I2C_CTRL_ACT2;
    con->dev_if_p->cookie_size = cookie_size;
    if ( plat_init_smart_eeprom_context (con, MOTHER_BOARD,
                                    0, (uchar *)cookie_contents)) {
        return(FAILED);
    }

    if (con->dev_if_p->cookie_size != COOKIE_SIZE_512) { 
        rc = smart_cookie_read_x(con, con->dev_if_p->cookie_size);
    } else {
        rc = smart_cookie_read(con);
    }

    if (rc) {
        printf("Failed to read Mother Board cookie\n");
        return(cntl_type);
    }

    if ((data_ptr = (uchar *) search_type_ret_addr_of_first_data
         (cookie_contents, (uchar) CONTROLLER_TYPE,
          &num_byte, FALSE)) == (uchar *) NULL) {
        /*Search CONTROLLER_TYPE failed. */
        cntl_type = INVALID_ID;                /* illegal code */
    } else {
         cntl_type = *data_ptr++;
         cntl_type = cntl_type << 8 | *data_ptr;
    }

    return(cntl_type);
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

/*-----------------------------------------------------------------------
 *
 * Function: get_mb_pid
 *  
 * This functions read PID cookie information on Mother Board cookie.
 *
 * Input : pid : pointer to the pid char array
 * 
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------
 */
int
get_mb_pid(char *pid)
{
    int rc, cookie_size = 512;
    uchar cookie_contents[cookie_size];
    sc_context *con, cont;
    dev_if_info_t dev_if;

    con = &cont;
    con->dev_if_p = &dev_if;
    con->dev_if_p->cookie_size = cookie_size;
    if ( plat_init_smart_eeprom_context (con, MOTHER_BOARD,
                                    0, cookie_contents)) {
        return(FAILED);
    }

    rc = smart_cookie_read(con);

    if (rc) {
        printf("Failed to read Mother Board cookie\n");
        return(FAILED);
    }


    return(get_pid(cookie_contents, pid));
}

/**************************************************************************
 *
 * Name: i2c_quack_read_bytes
 *
 * Description: Read bytes from the I2C interface
 *
 * Inputs: con_p   - Pointer to sc_context
 *         read_buffer - buffer to hold the data
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
int
i2c_quack_read_bytes(sc_context *con_p, char *rx_buffer)
{
    uint32_t ret_status, error_flag = PASSED;
    n2g_i2c_if_t *n2g_i2c_if_p, n2g_i2c_if_ds;
    int i;
    memset(&n2g_i2c_if_ds, 0, sizeof(n2g_i2c_if_t));
    n2g_i2c_if_p = &n2g_i2c_if_ds;
    n2g_i2c_if_p->i2c_bus_type = con_p->dev_if_p->parm1;
    n2g_i2c_if_p->i2c_dev = con_p->dev_if_p->parm2;
    n2g_i2c_if_p->mux = con_p->dev_if_p->parm3;
    n2g_i2c_if_p->i2c_ctrl = con_p->dev_if_p->parm4;
    
    n2g_i2c_if_p->size = 4; /* default of read 2 byte routine */
    n2g_i2c_if_p->buf = rx_buffer;
    n2g_i2c_if_p->offset = -1;

    for (i=0; i<QUACK_RETRY; i++) {
        if ((ret_status = n2g_i2c_read (n2g_i2c_if_p)) != RC_I2C_OP_OK) {
            sprintf(i2c_err,  i2c_err_str(ret_status));
            error_flag = FAILED;
            sleep(1);
        } else {
            return(PASSED);
        }
    }

    /* still need close it after this point */
    return(error_flag);
}
/**************************************************************************
 *
 * Name: i2c_quack_write_bytes
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
int
i2c_quack_write_bytes (sc_context *con_p, char *tx_buffer, int tx_size)
{
    uint32_t  ret_status, error_flag = PASSED;
    n2g_i2c_if_t *n2g_i2c_if_p, n2g_i2c_if_ds;
    int i;
    memset(&n2g_i2c_if_ds, 0, sizeof(n2g_i2c_if_t));
    
    n2g_i2c_if_p = &n2g_i2c_if_ds;
    n2g_i2c_if_p->i2c_bus_type = con_p->dev_if_p->parm1;
    n2g_i2c_if_p->i2c_dev = con_p->dev_if_p->parm2;
    n2g_i2c_if_p->mux = con_p->dev_if_p->parm3;
    n2g_i2c_if_p->i2c_ctrl = con_p->dev_if_p->parm4;

    n2g_i2c_if_p->size = tx_size;
    n2g_i2c_if_p->buf = tx_buffer;
    n2g_i2c_if_p->offset = -1;
    for (i=0; i<QUACK_RETRY ; i++) {
        if ((ret_status = n2g_i2c_write (n2g_i2c_if_p)) != RC_I2C_OP_OK) {
            sprintf(i2c_err,  i2c_err_str(ret_status));
            error_flag = FAILED;
            sleep(1);
        } else {
            return(PASSED);
        }
    }

    return(error_flag);
}

/**************************************************************************
 *
 * Name: poe_i2c_quack_reset
 *
 * Description: This function implementes a reset to PoE Quack chip by
 *              reset the line for 50ms then unreset it
 *
 * Inputs: con - pointer to sc_context
 *
 * Outputs: None
 *
 *************************************************************************/
void poe_i2c_quack_reset (sc_context *con_p)
{
    /* Bring the PoE into reset state */
    reset_platform_ext_dev(FPGA_EXT_POE_RST);
    /* Sleep for awhile */
    msleep(100);
    /* Release it from reset state */
    unreset_platform_ext_dev(FPGA_EXT_POE_RST);
    msleep(100);
    
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
void
init_cookie_4_default_x (int board_type, int cookie_type, 
                         uchar *contents, int cookie_size)
{
    switch (board_type) {
    case MOTHER_BOARD:
        printf("\nLoading default cookie format for Overlord Motherboard.\n");
        movbyte(default_mb_cookie, contents, cookie_size);
        break;
    case BACK_PLANE:
    case SM_MODULE:
    case NM_ADAPTER:
    case NETWK_MODULE:
    case AIM_MODULE:
    case DAUGHTER_CARD:
        printf("\nLoading default cookie format for POEl\n");
        movbyte(default_cookie_contents, contents, cookie_size);
        break;
    case PSU_MODULE:
    default:
        printf("\nLoading default cookie format.\n");
        movbyte(default_cookie_contents, contents, cookie_size);
        break;
    }
}


/**************************************************************************
 *
 * Name: init_cli_cmd
 *
 * Description: Initialize cookie related info before using CLI cmd. 
 *
 * Inputs: cmd - pointer to cli_cookie_cmd 
 *         ngio - pointer to struct ngio_intf_t
 *         type - ngio type. 
 *
 * Outputs: None
 *
 *************************************************************************/
static int32_t
init_cli_cmd (cli_cookie_cmd *cmd, struct ngio_intf_t *ngio,
              char *type)
{
    int slot;
    int id = 0;
    
    cmd->sm_cookie_id = 0;
    slot = cmd->slot;

    assert(slot);

    if (ngio->is_present(ngio)) {
        if (slot_i2c_unreset(ngio, slot, type) < 0) {
            return FAILED;
        }

        cmd->sm_cookie_id = id;
        cmd->present = TRUE; /* flag the SM present */
            /* Get Controller type */   
        if(id == INVALID_ID){
            if(cmd->cli_mode == CLI_DISCOVERY){
                prpass(testpass, "%s get_ngio_id", cmd->buf);
                cterr('f',0,"%s Controller type Invalid", cmd->buf);
                return (FAILED);
            }else
                if (cmd->cli_mode == CLI_COOKIE) {
                    printf("NG%s%d:Controller type Invalid", cmd->buf, slot);
                }else{
                    cterr('f',0,"\nUnknown cli mode");
                    return (FAILED);
                }
        }
    } else {
        /* no ngio present */
        if(cmd->cli_mode == CLI_DISCOVERY){
            /* return CLI_DEVICE_IS_VACANT as warning case */
            cterr('w',0,"%sDevice is vacant.", cmd->buf);
            return (CLI_DEVICE_IS_VACANT);
        } else
            if (cmd->cli_mode == CLI_COOKIE) {
                printf("NGIO%s%d:Not Present", cmd->buf, slot);
            } else {
                cterr('f',0,"\nUnknown cli mode");
                return (FAILED);
            }
        cmd->present = FALSE;  /* flag SM not present for SM dc, NM ... */
        return (FAILED);
    }
    return (PASSED);
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
    int32_t ret_val = PASSED;
    uchar cookie_contents[COOKIE_SIZE_512];
    sc_context *con, cont;
    dev_if_info_t dev_if;
    int slot = ngio->slot;

    assert(ngio->slot);
    if (type == WIC_DAUGHTER_CARD || type == SM_DAUGHTER_CARD ||
        type == SM_VM_DAUGHTER_CARD) {
        assert(ngio->pc);
        slot = ngio->pc->slot;
    }

    con = &cont;
    con->dev_if_p = &dev_if;
    con->dev_if_p->cookie_size = COOKIE_SIZE_512;

    if (plat_init_smart_eeprom_context (con, type, 
                                    slot, (uchar *)cookie_contents)) {
        return(FAILED);
    }
    if (smart_cookie_read_write_eeprom(con, cmd)) {
         ret_val = FAILED;
    }

    return(ret_val);
}

/**************************************************************************
 *
 * Name: alter_sm_cookie_cli
 *
 * Description:  alter sm cookie via CLI cmd. 
 *
 * Inputs: mode - CLI mode 
 *         cmd - pointer to cli_cookie_cmd
 *
 * Outputs: return value of alter_ngio_cookie()
 *
 *************************************************************************/
int32_t
alter_sm_cookie_cli (boolean mode, cli_cookie_cmd *cmd)
{
    int slot, rc;
    struct ngio_intf_t *ngio;
    slot = cmd->slot;

    ngio = (struct ngio_intf_t *)slot_get_ngiosm(slot);
    
    rc = init_cli_cmd(cmd, ngio, "SM");
    if (rc != PASSED) {
        return rc;
    }
 
    return alter_ngio_cookie(cmd, ngio, SM_MODULE);
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
int32_t
alter_sm_cookie (void)
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
 * Name: alter_sm_dc_cookie_cli
 *
 * Description:  alter sm dc cookie via menu.
 *
 * Inputs: None
 *
 * Outputs: return value of alter_ngio_cookie()
 *
 *************************************************************************/
int32_t
alter_sm_dc_cookie (void)
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
 * Name: alter_sm_dc_cookie_cli
 *
 * Description:  alter sm dc cookie via CLI cmd.
 *
 * Inputs: mode - CLI mode
 *         cmd - pointer to cli_cookie_cmd
 *
 * Outputs: return value of alter_ngio_cookie()
 *
 *************************************************************************/
int32_t
alter_sm_dc_cookie_cli (boolean mode, cli_cookie_cmd *cmd)
{
    int slot;
    struct ngio_intf_t *ngio, *dc;
    
    slot = cmd->slot;

    ngio = (struct ngio_intf_t *)slot_get_ngiosm(slot);

    assert(ngio);
    assert(ngio->dc);
    
    dc = ngio->dc;

    if (slot_i2c_unreset(ngio, slot, "SMDC") < 0) {
        return FAILED;
    }

    if (dc->is_present(dc) <= 0) {
        return(CLI_DEVICE_IS_VACANT);
    }
    
    return alter_ngio_cookie(cmd, ngio->dc, SM_DAUGHTER_CARD);
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
int32_t
alter_sm_vm_dc_cookie (void)
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
 * Name: alter_wic_cookie_cli
 *
 * Description:  alter wic cookie via CLI cmd.
 *
 * Inputs: mode - CLI mode
 *         cmd - pointer to cli_cookie_cmd
 *
 * Outputs: return value of alter_ngio_cookie()
 *
 *************************************************************************/
int32_t
alter_wic_cookie_cli (boolean mode, cli_cookie_cmd *cmd)
{
    struct ngio_intf_t *ngio;
    int slot, rc;

    slot = cmd->slot;

    ngio = slot_get_ngiowic(slot);
    rc = init_cli_cmd(cmd, ngio, "WIC");
    if (rc != PASSED) {
        return rc;
    }

    return alter_ngio_cookie(cmd, ngio, WIC_MODULE);
}

/**************************************************************************
 *
 * Name: alter_hwic_cookie
 *
 * Description:  alter wic cookie via menu
 *
 * Inputs: None.
 *
 * Outputs: return value of alter_ngio_cookie()
 *
 *************************************************************************/
int
alter_hwic_cookie (void)
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
 * Name: alter_hwic_dc_cookie
 *
 * Description:  alter wic dc cookie via menu
 *
 * Inputs: None.
 *
 * Outputs: return value of alter_ngio_cookie()
 *
 *************************************************************************/
int
alter_wic_dc_cookie (void)
{
    int max_slot, slot;
    struct ngio_intf_t *ngio, *dc;
    
    max_slot = get_max_wic_slots();
    slot = getdec_answer("Enter WIC slot containing daughter board", 1, FIRST_SLOT,
                            max_slot);

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
 * Name: alter_wic_dc_cookie_cli
 *
 * Description:  alter wic dc cookie via CLI cmd.
 *
 * Inputs: mode - CLI mode
 *         cmd - pointer to cli_cookie_cmd
 *
 * Outputs: return value of alter_ngio_cookie()
 *
 *************************************************************************/
int32_t
alter_wic_dc_cookie_cli (boolean mode, cli_cookie_cmd *cmd)
{
    int slot;
    struct ngio_intf_t *ngio, *dc;

    slot = cmd->slot;

    ngio = (struct ngio_intf_t *)slot_get_ngiowic(slot);

    assert(ngio);
    assert(ngio->dc);
    
    dc = ngio->dc;

    if (slot_i2c_unreset(ngio, slot, "WICDC") < 0) {
        return FAILED;
    }

    if (dc->is_present(dc) <= 0) {
        return(CLI_DEVICE_IS_VACANT);
    }

    return alter_ngio_cookie(cmd, ngio->dc, WIC_DAUGHTER_CARD);
}

/**************************************************************************
 *
 * Name: alter_vm_cookie_cli
 *
 * Description:  alter vm cookie via CLI cmd.
 *
 * Inputs: mode - CLI mode
 *         cmd - pointer to cli_cookie_cmd
 *
 * Outputs: return value of alter_ngio_cookie()
 *
 *************************************************************************/
int32_t
alter_vm_cookie_cli (boolean mode, cli_cookie_cmd *cmd)
{
    struct ngio_intf_t *ngio;
    int slot, rc;

    slot = cmd->slot = FIRST_SLOT;

    ngio = (struct ngio_intf_t *)slot_get_ngiovm(slot);
    
    rc = init_cli_cmd(cmd, ngio, "VM");
    if (rc != PASSED) {
        return rc;
    }
    
    return alter_ngio_cookie(cmd, ngio, VM_MODULE);
}

/**************************************************************************
 *
 * Name: alter_vm_cookie
 *
 * Description:  alter vm cookie via menu
 *
 * Inputs: None. 
 *
 * Outputs: return value of alter_ngio_cookie()
 *
 *************************************************************************/
int
alter_vm_cookie (void)
{
    int    slot;
    struct ngio_intf_t *ngio;
        
    slot = FIRST_SLOT;

    ngio = (struct ngio_intf_t *)slot_get_ngiovm(slot);
    
    if (slot_i2c_unreset(ngio, slot, "VM") < 0) {
        return FAILED;
    }
    return (alter_ngio_cookie(NULL, ngio, VM_MODULE));
}

/**********************************************************************
 * Function: get_max_num_vm
 *
 * This function returns with the number of vm's which are
 * supported on this platform, on ovld is 1. 
 *
 * Input:
 *      none
 *
 * Output: PASS/FAIL
 *      FAIL if incorrect revision
 *
 **********************************************************************
 */
int get_max_num_vm (void)
{    /* will be call by cli_cmd.c */
    return(MAX_VM);  /* #define MAX_VM 1 on ngio.h  */
}


/**************************************************************************
 *
 * Name: get_cookie_id
 *
 * Description: read cookie id  
 *
 * Inputs: slot - ngio slot
 *         type - ngio type
 *         eeprom_data - ngio cookie. 
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
ushort
get_cookie_id (int slot, int type, uchar *eeprom_data,
               uint16_t *id, char *err)
{
    uchar num_byte, *data_ptr;
    sc_context *con, cont;
    dev_if_info_t dev_if;

    *id = INVALID_ID;
    con = &cont;
    con->dev_if_p = &dev_if;
    con->dev_if_p->cookie_size = COOKIE_SIZE_512;

    plat_init_smart_eeprom_context(con, type, slot, (uchar *)eeprom_data);
    if(smart_cookie_read(con)==PASSED) {
        if ((data_ptr = (uchar *) search_type_ret_addr_of_first_data
             ((uchar *)eeprom_data, (uchar) CONTROLLER_TYPE,
              &num_byte, FALSE)) == (uchar *) NULL) {
            /*Search CONTROLLER_TYPE failed. */
            *id = INVALID_ID;
        } else {
            *id = *data_ptr++;             /* get board ID */
            *id = *id << 8 | *data_ptr;
        }

        return PASSED;
    }

    /* can return failed assuming no cookie id has same value as "FAILED" */
    sprintf(err, i2c_err);
    return FAILED;
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
    sc_context *con, cont;
    dev_if_info_t dev_if;
 
    con = &cont; 
    con->dev_if_p = &dev_if;
    con->dev_if_p->cookie_size = COOKIE_SIZE_512;

    plat_init_smart_eeprom_context(con, type, slot, eeprom_data);

    if(smart_cookie_read(con)==PASSED) {
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

    }
    return(PASSED);

}

void
clean_smart_eeprom_context (sc_context *con_p)
{
    
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
    uchar eeprom_data[COOKIE_SIZE_512];
    uchar num_byte, *data_ptr;
    sc_context *con, cont;
    dev_if_info_t dev_if;

    con = &cont;
    con->dev_if_p = &dev_if;
    /* SM cookie_size ??? */
    con->dev_if_p->cookie_size = COOKIE_SIZE_512;

    plat_init_smart_eeprom_context(con, type, slot, eeprom_data);

    /* read SM cookie */
    if(!smart_cookie_read(con)) {

        if ((data_ptr = (uchar *) search_type_ret_addr_of_first_data
             (eeprom_data, (uchar) BOARD_MAC_ADDR,
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
        }
	retval = PASSED;
    }
    clean_smart_eeprom_context(con);	/* Unlock SM I2C bus */
    return retval;
}

/*-----------------------------------------------------------------------------
 *  
 * Function get_sm_mac_addr
 *  
 * This function will return the MAC addr of the specified SM
 *
 * Inputs : SM slot #
 *          *board_mac_addr addr - pointer to mac addr 
 *  
 * Returns : 
 */ 
int
get_sm_mac_addr (int slot, uchar *board_mac_addr)
{
    return (get_ngio_mac_addr (slot, SM_MODULE, board_mac_addr));
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
get_pcb_serial (uchar *eeprom_data, char *serial)
{
    uchar *data_ptr;
    uchar num_byte;

    if (eeprom_data[0] == CURRENT_FORMAT_VERSION) {
	/* for polling slots, do not print warning. simply print the content */
	if ((data_ptr = (uchar *)search_type_ret_addr_of_first_data
	     (eeprom_data, PCB_SERIAL_NUM, &num_byte, FALSE)) == NULL) {
            sprintf(serial, "NO PCB NUM");
            printf("NO PCB NUM\n");
	} else {
            memcpy(serial, data_ptr, num_byte);
	}
	return(0);
    } else {
	/* Get PCB Serial Number from WIC EEPROM located at 0x4 to 0x7 */
	return(*(int *)(eeprom_data + 0x4));
    }
}


/**** not support on O2, but called on common code. ****/
int alter_pvdm_cookie_x (boolean x, cli_cookie_cmd *ptr)
{
   printf("%s: NOT Support so far !!\n", __FUNCTION__);
   return (FAILED);
}

int pvdm_present (int x)
{
    printf("%s NOt Support so far  !!\n", __FUNCTION__);
    exit(0);
    return 0;
}

int 
alter_ism_cookie_x (boolean x, cli_cookie_cmd *ptr)
{ 
    printf("%s NOt Support so far  !!\n", __FUNCTION__);
    exit(0);
    return 0;
}

int mp_show_cookie_x(boolean x, cli_cookie_cmd *ptr)
{
    printf("%s: NOT Support so far !!\n", __FUNCTION__);
    return (FAILED);
}

/**** not support on O2, but called on common code. ****/

int get_tlv_serial (uchar *eeprom_data, char *serial, uchar ser_tlv_field)
{

    uchar *data_ptr;
    uchar num_byte;

    if (eeprom_data[0] == CURRENT_FORMAT_VERSION) {
	/* for polling slots, do not print warning. simply print the content */
	if ((data_ptr = (uchar *)search_type_ret_addr_of_first_data
	     (eeprom_data, ser_tlv_field, &num_byte, FALSE)) == NULL) {
            sprintf(serial, "NO S/N NUM");
            printf("\n *** ERROR: 0x%02x Serial Number field not programmed per Product PCAMAP.",
	       ser_tlv_field);
	    return (FAILED);
	} else {
            memcpy(serial, data_ptr, num_byte);
	    return (PASSED);
	}
    } else {
	cterr('f', 0, "Unsupported old cookie format 0x%02x", eeprom_data[0]);
	return (FAILED);
    }
}



/******** History ********
$Log: platform_cookie.c,v $
Revision 1.32  2020/02/03 01:35:09  alpeng
missing one line for update pid

Revision 1.31  2020/01/31 07:17:39  leschen
Support Curie new cookie.

Revision 1.30  2020/01/09 01:02:19  jiajliu
Merge Curie 2RU to main trunk

Revision 1.29  2019/12/31 07:48:06  alpeng
revert cookie to old one for RDT

Revision 1.28  2019/12/21 00:52:37  ptong
Curie PID change to C8300-1N1S-4T2X and C8300-1N1S-6G

Revision 1.27  2019/08/06 06:56:10  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.26.2.7  2019/04/26 03:05:43  alpeng
Change Curie 1RU PIDs from CE8300 to C8300

Revision 1.26.2.6  2019/03/18 22:16:26  ptong
Change Curie 1RU PIDs from CE9331 to CE8300

Revision 1.26.2.5  2018/11/01 23:18:45  ptong
Change Curie 1RU PID to new CE9331 value. Remove Neptunium and Polonium SKU

Revision 1.26.2.4  2018/10/15 10:40:37  alpeng
fixed neptune compile error and using ifdef AIKIDO_ACT2 since neptune act2 driver is already define mbx_read/write

Revision 1.26.2.3  2018/09/07 01:43:58  alpeng
add spi read/write util for aikido; change tam lib on Makefile

Revision 1.26.2.2  2018/08/24 19:22:28  meho
Fixed read pluggable slot cookie.

Revision 1.26.2.1  2018/08/08 02:49:20  alpeng
update string cmp for curie mb check SKU during diag launch

Revision 1.26  2018/05/18 09:24:51  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.25  2017/07/28 07:49:43  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.24  2017/07/10 02:51:58  leschen
Remove unused variable

Revision 1.23  2016/10/16 13:28:59  iachang
Supported Goldbeach Platform.

Revision 1.22  2016/06/06 18:30:39  huanngo
Add code to support programming SUDI/WDC with Chassis S/N

Revision 1.21.6.13  2018/05/17 10:50:22  alpeng
 sync with trunk <trunk-051618>

Revision 1.21.6.12  2017/09/28 08:16:25  leschen
Support VG450

Revision 1.21.6.11  2017/09/19 10:18:51  alpeng
support oakenshield; fix oakenshield andf2w uart issue

Revision 1.21.6.10  2017/08/02 09:59:41  leschen
Update Neptune PID from ISR4462 to ISR4461.

Revision 1.21.6.9  2017/04/05 06:45:03  leschen
Sync with <ng_diag-tag-032917>

Revision 1.21.6.8  2017/03/29 03:16:35  leschen
Remove unnecessary print messages.

Revision 1.21.6.7  2017/03/24 22:25:23  ptong
Merge new ACT2 code from main trunk that allow chassis SN tobe used in SUDI/WDC. Increase version to V1.1.4

Revision 1.21.6.6  2017/03/13 08:27:30  leschen
Support latest Neptune/Triton/Proteus/Neso PID.

Revision 1.21.6.5  2017/03/13 07:43:31  leschen
Support Triton system.

Revision 1.21.6.4  2017/03/07 02:07:36  alpeng
update ISR num to 4462 for neptune

Revision 1.21.6.3  2017/01/04 08:32:38  alpeng
update sku num for neptune

Revision 1.21.6.2  2016/12/27 09:50:21  leschen
Clean up codes.

Revision 1.21.6.1  2016/11/18 07:06:53  leschen
Modify chk_plat_sku function to support Neptune.

Revision 1.25  2017/07/28 07:49:43  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.24  2017/07/10 02:51:58  leschen
Remove unused variable

Revision 1.23  2016/10/16 13:28:59  iachang
Supported Goldbeach Platform.

Revision 1.22  2016/06/06 18:30:39  huanngo
Add code to support programming SUDI/WDC with Chassis S/N

Revision 1.21  2016/03/04 19:19:15  ptong
Clean up obsolete ISR platfrom PID

Revision 1.20  2015/06/05 06:13:11  alpeng
fix poll slot issue

Revision 1.19  2014/11/27 07:06:40  alpeng
do not get ngiosm struct once the slot is exceed the max sm

Revision 1.18  2014/11/26 07:00:42  alpeng
Support NGSM+NGWIC+NGVM case

Revision 1.17  2014/11/26 04:14:30  alpeng
reverting to version 1.15

Revision 1.15  2014/06/27 00:27:40  mcharon
fix default cookie content

Revision 1.14  2014/03/06 20:32:16  mcharon
sm daughter card uses ngiowic_i2c_addr_act2 not ngiovm_i2c_addr_act2

Revision 1.13  2014/02/13 19:03:12  mcharon
support act2 authentication on sword

Revision 1.12  2014/02/05 23:11:57  mcharon
sword using act2 so 30wpoe quack address is different from other platforms

Revision 1.11  2013/12/18 06:32:58  hroni
use toolchain in router/bin to do make with TOOLS_VER=c4.5.3-p1, TOOLS_ARCH=x86_64

Revision 1.10  2013/11/26 08:40:36  hroni
fix compiler warning

Revision 1.9  2013/11/11 21:18:40  mcharon
pass string instead of number in first argum of host_send_packet ; add xaui support

Revision 1.8  2013/08/27 20:51:52  mcharon
remove printf inside i2c_quack_reset

Revision 1.7  2013/08/19 01:53:19  alpeng
using both FPGA and MB cookie to get/check board type

Revision 1.6  2013/07/23 04:17:33  alpeng
update SKU num for Juno, Utah and Sword

Revision 1.5  2013/07/18 02:51:57  hroni
Fix Alter PoE cookie: remove the hard coded mux and ctrl ids, define the appropriate mux and ctrl ids in platform_i2c.h

Revision 1.4  2013/07/10 01:34:53  alpeng
moving get_plat_sku() to platform_cookie.c. since the sku number coming from cookie.

Revision 1.3  2013/05/09 23:49:56  mcharon
support cli discovery of daughter card

Revision 1.2  2013/05/09 07:36:50  alpeng
updating files

Revision 1.1  2013/05/09 05:42:36  alpeng
moving overlord common code from x86

Revision 1.32  2013/05/01 20:40:56  mcharon
 call memset inside act2_utils.c

Revision 1.31  2013/03/17 02:04:14  mcharon
support command line for testing 30w poe

Revision 1.30  2013/03/08 19:06:12  mcharon
in wdc call get_pid instead of get_mb_pid, so modules are supported

Revision 1.29  2013/03/01 22:38:14  mcharon
change FIRST_SLOT to slot for wic_daughter_card

Revision 1.28  2013/02/28 00:38:50  srane
Add smart cookie tests for NGWIC Daughter card.

Revision 1.27  2012/11/17 01:15:18  mcharon
reset i2c device; don't cterr in driver code..propogate err message to slot.c

Revision 1.26  2012/11/12 22:18:25  mcharon
print module type and slot when i2c failes

Revision 1.25  2012/11/12 20:35:23  mcharon
add third arg to slot_i2c_unrest to report slot num when fails..improve err reporting

Revision 1.24  2012/11/07 10:58:16  alpeng
remove useless file and clean up code

Revision 1.23  2012/11/06 20:39:50  mcharon
add headers/cleanup/remove unneeded functions/files

Revision 1.22  2012/10/25 18:56:18  mcharon
improve error reporting

Revision 1.21  2012/10/11 08:24:42  alpeng
unsupported SATA on wic slot 3

Revision 1.20  2012/09/28 06:34:20  alpeng
make a note for pid_2check_sata()

Revision 1.19  2012/09/26 03:23:06  alpeng
check sata available via PID

Revision 1.18  2012/09/18 19:19:55  mcharon
support poll slot-fix io intr test; support fpga upgrade; support serial irq intr tst

Revision 1.17  2012/09/17 20:54:48  mcharon
remove plat_init_x: sring

Revision 1.16  2012/08/30 07:44:57  alpeng
infrom user with warning when device is vacant on CLI discovery

Revision 1.15  2012/08/28 10:12:57  alpeng
fixed warning

Revision 1.14  2012/08/28 09:30:59  alpeng
support CLI discovery for SATA, fix typo on alter_sm_dc_cookie_cli()

Revision 1.13  2012/08/18 00:01:13  ptong
Use official SKU for PID checking

Revision 1.12  2012/06/28 21:40:54  srane
Fix DC support in all NGWIC slots and cookie alter for DC.

Revision 1.11  2012/06/06 07:34:05  palin2
Clean up compiler warnings.

Revision 1.10  2012/05/16 07:29:24  srane
Daughter card support.

Revision 1.9  2012/05/11 08:13:27  alpeng
support HDD present to MB test menu

Revision 1.8  2012/05/08 06:12:57  alpeng
change the CLI cmd from PVDM to VM

Revision 1.7  2012/05/05 04:27:43  mcharon
add cli for daughter card

Revision 1.6  2012/05/05 04:02:21  mcharon
support alter daughter board cookie for wic

Revision 1.5  2012/05/04 20:01:46  mcharon
use void* instead of int as argument to to func ptrs in ngio_intf

Revision 1.4  2012/04/26 00:12:09  mcharon
make sure pcb serial is only 11 characters

Revision 1.3  2012/04/18 18:39:26  mcharon
put nul at the end of serial string

Revision 1.2  2012/03/28 00:38:22  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:09  ptong
Initial archive of ng_diag module

$Endlog$
*/
