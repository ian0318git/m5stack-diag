/* $Id: platform_tam_cookie.c,v 1.2 2021/06/02 08:22:36 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/platform_tam_cookie.c,v $
 *------------------------------------------------------------------
 *
 * platform_tam_cookie.c - copy from tachi/tsn
 * diag_plat_cookie.c - Platform cookie function
 * 
 * July 2015, Times Huang
 *
 * Copyright (c) 2015-2020 by Cisco Systems, Inc.
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
#include "mb_tests.h"
#include "i2c_api.h"
#include "goofy_i2c.h"
#include "tam_aikido_mailbox.h"
#include "dash_fpga.h" 
#include "queryflags.h"



#define QUACK_RETRY  8 
#define ACT2_RESET_UNRESET_DELAY                        (500)
#define ACT2_UNRESET_DELAY                              (5000)

#define SKU_INVALID 0xff


boolean aikido_mailbox_flag;
boolean aikido_act2_flag;
static uint8_t use_interrupt = 0;

extern int tam_lib_get_fw_ver(int); 
extern int is_tam_aikido_mbox_on(void); 
extern int tam_act2_reset(int);
extern tam_lib_status_t tam_lib_scc_write_eeprom(void *, uint8_t *, uint16_t, uint16_t);
extern int cookie_4_processor_x(uchar *, int, int, int, cli_cookie_cmd *);
extern int cookie_is_act2(sc_context *);
extern int slot_get_pcb_serial(uchar *, char *);   
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

int tam_lib_device_open_mailbox(void *,uint32_t,uint32_t,uint32_t,void *);
static int alter_cookie(int, int);
static int tam_lib_read_cookie(void);

static dev_if_info_t dev_if;
static uchar cookie_contents[COOKIE_SIZE_512];
static uchar default_cookie_contents[COOKIE_SIZE_512] = {
    0x04,0xFF,0xC1,0x8B,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x40,
    0x10,0xe0,0x41,0x01,0x00,0x82,0x49,0x4D,
    0x24,0x05,0x42,0x30,0x31,0x87,0x44,0x1B,
    0x59,0x05,0x88,0x00,0x00,0x00,0x00,0x02,
    0x05,0xCB,0x8B,0x43,0x38,0x35,0x30,0x30,
    0x4C,0x2D,0x38,0x53,0x34,0x58,0x89,0x56,
    0x30,0x30,0x20,0xC6,0x8a,0x54,0x42,0x44,
    0x54,0x42,0x44,0x54,0x42,0x44,0x54,0x09,
    0xC6,0xC2,0x8B,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0xC3,0x06,
    0x00,0x00,0x00,0x00,0x00,0x00,0x43,0x00,
    0x80,0xC4,0x08,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,  
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
int tam_lib_device_open_mailbox (void *handle, uint32_t a, uint32_t b, uint32_t c, void *d)
{
    return (PASSED);
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
    int retval = PASSED;

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
            con_p->dev_if_p->parm2 = (uint8_t)MB_I2C_ADDR_AIKIDO_ACT2;
        } else {
            con_p->dev_if_p->parm2 = (uint8_t)MB_I2C_ADDR_ACT2;
        }
        con_p->dev_if_p->parm3 = (uint8_t)0;   
        con_p->dev_if_p->parm4 = (uint8_t)I2C_CTRL_ZERO;   
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
    char choice[5];
    sc_context *con, cont;
    int act2_chip;

    con = &cont;
    con->dev_if_p = &dev_if;
    con->dev_if_p->cookie_size = COOKIE_SIZE_512;
    
    testname("Smart cookie");
    if (submenu_flag == 0) {
        prpass(testpass, "Main Menu");
    } else {
        prpass(testpass, "Sub Menu");
    }

    printf("\nEnter (m)otherboard (Q)UIT  >");
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
    return(SKU_INVALID);
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
    sc_context *con, cont;
    con = &cont;

    if (aikido_mailbox_flag) {
        /* Initialize Mailbox */
        ret_val = tam_lib_device_open_mailbox((void *)con, use_interrupt,
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
    if (board_type == MOTHER_BOARD) {  
        con->quack_reset(con); 
    } else { 
        tam_act2_reset(0); 
    }

    if (board_type != MOTHER_BOARD) { /* NGIO has no aikido so far */
        if (!cookie_is_act2(con)) { /* quack, return failed,
                                     * tam lib does not support quack */
            printf("Cookie utility doesn't support Quack chip!\n");
            return (FAILED); 
        }
    }

    tam_handle_ptr = NULL;
    if (is_tam_aikido_mbox_on()) {
        /* Initialize Mailbox */
        status = tam_lib_device_open_mailbox((void *)con,  
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
  

/*-------------------------------------------------
 * $Log: platform_tam_cookie.c,v $
 * Revision 1.2  2021/06/02 08:22:36  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.8.4  2021/04/14 06:59:45  iachang
 * CSCvo59196-28 : Add Aikido RNG test
 *                 Fixed tam_lib_device_open_mailbox() input NULL pointer issue.
 *
 * Revision 1.1.8.3  2020/11/06 03:27:01  iachang
 * CSCvo59196-21:Support BCM57412 LASI/No-LASI config program.
 *
 * Revision 1.1.8.2  2020/08/26 02:37:52  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.6.6  2020/07/29 08:57:35  iachang
 * Code clean up.
 *
 * Revision 1.1.6.5  2020/07/21 09:15:35  iachang
 * Remove un-used header file
 *
 * Revision 1.1.6.4  2019/12/17 06:54:43  iachang
 * Removed Discrete-ACT2 select item in cookie utility
 *
 * Revision 1.1.6.3  2019/04/06 01:36:14  letsai
 * 1. Remove unused functions and files.
 * 2. Fix BCM54194 SFP External loopback test.
 * 3. Fix BCM54194 Register test.
 * 4. Fix Voltage Margin Utility.
 * 5. Add function to show system information.
 *
 * Revision 1.1.6.2  2019/03/14 03:48:37  letsai
 * Initial check in.
 *
 *
 *
 *
 * $Endlog$
 */
