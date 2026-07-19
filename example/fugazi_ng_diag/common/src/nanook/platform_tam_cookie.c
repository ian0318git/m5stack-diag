/* $Id: platform_tam_cookie.c,v 1.2 2019/12/11 10:10:35 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/platform_tam_cookie.c,v $
 *------------------------------------------------------------------
 *
 * platform_tam_cookie.c - platform tam library codes 
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
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
#include "types.h"
#include "cookie_4.h"
#include "cli_cmd.h"
#include "error.h"
#include "menu.h"
#include "defs.h"
#include "common_utils.h"
#include "legacy_smart_cookie.h"
#include "platform_tam_cookie.h"
#include "act2_utils.h"
#include "proto.h"
#include <tam_library.h>
#include "platform_i2c.h"
#include "plat_defs.h"
#include "slot.h"
#include "mb_tests.h"
#include "ngio.h"
#include "i2c_api.h"
#include "platform_slot.h"
#include "queryflags.h"
#include "diag_fpga_i2c_lib.h"
#include "tam_aikido_mailbox.h"
#include "diag_i2c_addr.h"


#define PLATFORM_BUFF_SIZE                      (259)
#define PRODUCT_NAME_LEN                        (256)
/*******************************************************************************
 *                              Global Variables                               *
 *******************************************************************************
 */
static char i2c_err[80];
boolean aikido_mailbox_flag;
boolean aikido_act2_flag;
//static dev_if_info_t dev_if;
//static uchar cookie_contents[COOKIE_SIZE_512];
//static uint8_t use_interrupt = 0;
//static char smc_buf[80];
//static char i2c_err[80];

/*******************************************************************************
 *                             Function Prototypes                             *
 *******************************************************************************
 */
int i2c_act2_write_bytes(sc_context *, char *, int);
int i2c_act2_read_bytes(sc_context *, char *);
void i2c_act2_reset(sc_context *);
//int plat_init_smart_eeprom_context (sc_context *, uchar, uchar, uchar *);

//static int tam_lib_read_cookie (void);
//static void init_ngio_context (sc_context *, uchar, uchar, uchar *);

extern int tam_act2_reset(int);
extern int is_tam_aikido_mbox_on(void);

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
            tam_act2_reset(0);
            printf("Done\n");
            fflush(stdout);
        } /* only ACT2 needs reset, AIKIDO doesn't need it */
    } else if (con_p->type == WIC_MODULE)  {
        printf("Resetting NIM ACT2...");
        fflush(stdout);
        ngio = (struct ngio_intf_t *)slot_get_ngiosm(slot);
        ngio->i2c_reset(ngio);
        msleep(ACT2_RESET_UNRESET_DELAY);
        ngio->i2c_unreset(ngio);
        msleep(ACT2_UNRESET_DELAY);
        printf("Done\n");
        fflush(stdout);
    } else if (con_p->type == PLUGGABLE_CARD) {
        printf("Fixme %s: pluggable i2c reset \n", __FUNCTION__);

    } else {
        printf("error %s: con_p->type unknown \n", __FUNCTION__);
    }
    return;
}

#if 0
/*
 * Function: get_mb_pid
 *
 * Description:
 *   This function read MB cookie for product id field*
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

/*************************************************************************
 * Function: tam_lib_read_cookie
 *
 * This function is read the MB cookie content by tam library
 *
 * Input: none
 *
 * Output: PASSED / FAILED
 **************************************************************************
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
 ***********************************************************************
 */
int plat_init_smart_eeprom_context (sc_context *con_p, uchar type,
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
        #if 0 
	if (is_sword()) {
            /* sword's poe uses act2 chip so address is different */
            con_p->dev_if_p->parm2 = (uint8_t)MB_I2C_ADDR_ACT2;
        } else
	#endif
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
 ***********************************************************************
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
#endif

/*---------------------------------------------------------------
$Log: platform_tam_cookie.c,v $
Revision 1.2  2019/12/11 10:10:35  lucywang
Merged Nanook to main trunk


$Endlog$
*/
