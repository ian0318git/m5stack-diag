 /* $Id: platform_cookie.c,v 1.2 2018/08/06 02:31:52 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/platform_cookie.c,v $
 *------------------------------------------------------------------
 *
 * platform_cookie.c - Specific MB cooke support from Xformers.
 *
 * Sept. 2008, Shih-Nan Huang
 *
 * Copyright (c) 2014-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include "types.h"
#include "error.h"
#include "cookie_4.h"
#include "cross_platform.h"
#include "common.h"
#include "act2_utils.h"
#include "platform_cookie.h"
#include "cli_cmd.h"
#include "act2l_typedef.h"
#include "mfg_api.h"
#include "object.h"
#include "diag_i2c_addr.h"
#include "tam_act2_api_drv_support.h"
#include "tam_aikido_mailbox.h"
#include "platform_stub.h"
#include "platform_i2c.h"
#include "/auto/sp-engops/diags/pld/act2lite/ARM/TSN/tam_library.h"
#include "/auto/sp-engops/diags/pld/act2lite/ARM/TSN/tam_lib_manufacturing.h"
#include "diag_fpga.h"
#include "queryflags.h"
#include "i2c_api.h"
#include "n2g_api_rc.h"
#include "nvmonvars.h"
#include "diag_fpga_i2c_lib.h"

/***********************************************************************
 *  Macro Processor Definitions
 ************************************************************************/

/***********************************************************************
 *  Functions Declaration
 ************************************************************************/
int alter_mb_cookie(void);
int smartchip(int);
int tam_lib_read_cookie(void);
ushort get_cookie_id(int, int, uchar *, uint16_t *, char *);
int get_pid(uchar *, char *);
int get_pcb_serial(uchar *, char *);
int print_cookie(int, char *argv[]);
int platform_get_pid(char *);
static int alter_cookie(int board_type);
int read_eeprom_block(unsigned int, unsigned int, unsigned char *);
int plat_init_smart_eeprom_context(sc_context *, uchar, uchar, uchar *);
int i2c_quack_read_bytes(sc_context *, char *);
int i2c_quack_write_bytes(sc_context *, char *, int);

/***********************************************************************
 *  Extern Functions Declaration
 ************************************************************************/
extern void msleep(unsigned long);
extern int cookie_4_processor (uchar *, int, int, cli_cookie_cmd *);
extern int tam_act2_reset(int);
extern int cookie_4_processor_x(uchar *, int, int, int, cli_cookie_cmd *);
extern COOKIE_4 *buffer_search_x(boolean, int);
extern COOKIE_4 *get_new_buf(void);
extern void fetch_user_input_data(char *, COOKIE_4 *);
extern void cookie_4_enque(COOKIE_4 *, COOKIE_4 *);
extern void movbyte(unsigned char *, unsigned char *, int);
extern tam_lib_status_t tam_lib_scc_write_eeprom(void *tam_handle,
                              uint8_t * src_buffer,
                              uint16_t length, uint16_t dest);

/***********************************************************************
 *  Global Variable
 ************************************************************************/
extern COOKIE_4 *cookie_root;
extern cookie_4_table cookie_4_info[];
static int get_sku_flag = FALSE;
static char get_prodName[PRODUCT_NAME_LEN] = {0};
static void *platform_tam_handle = NULL;
static uchar cookie_contents[COOKIE_SIZE_512];
static int g_cookie_type = 0; 
static char smc_buf[80];
static char i2c_err[80];

static uchar default_mb_cookie[COOKIE_SIZE_512] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
};

static uchar default_921_e2e_cookie[COOKIE_SIZE_512] = {
    0x04, 0xff, 0xc1, 0x8b, 0x46, 0x4f, 0x43, 0x32,
    0x32, 0x32, 0x30, 0x35, 0x57, 0x31, 0x42, 0x40,
    0x10, 0x94, 0x41, 0x01, 0x00, 0xc0, 0x46, 0x00,
    0x4a, 0x01, 0xd9, 0x89, 0x01, 0x42, 0x30, 0x31,
    0x88, 0x00, 0x00, 0x00, 0x00, 0x02, 0x01, 0xcb,
    0x88, 0x43, 0x39, 0x33, 0x31, 0x4a, 0x2d, 0x34,
    0x50, 0x89, 0x56, 0x30, 0x30, 0x20, 0xc6, 0xa8,
    0x54, 0x42, 0x44, 0x54, 0x42, 0x44, 0x54, 0x42,
    0x44, 0x20, 0x09, 0xed, 0xc2, 0x8b, 0x46, 0x43,
    0x57, 0x32, 0x32, 0x32, 0x33, 0x30, 0x30, 0x31,
    0x54, 0xc3, 0x06, 0x78, 0x72, 0x5d, 0xab, 0xfe,
    0x30, 0x43, 0x00, 0x10, 0xc4, 0x08, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
};

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
 *         cookie_size - size of cookie
 *
 * Output: none
 *
 *------------------------------------------------------------------------*/
void init_cookie_4_default_x(int board_type, int cookie_type,
                        uchar * contents, int cookie_size)
{
    uint sku_id_val;

    printf("%s: Viper currently skip SKU ID check.\n", __FUNCTION__);
    sku_id_val = C921J_E2E;

    switch (board_type) {
    case MOTHER_BOARD:
        printf("\nLoading default cookie format for Viper Motherboard.\n");
        switch (sku_id_val) {
        case C921J_E2E:
            movbyte(default_921_e2e_cookie, contents, cookie_size);
            break;
        default:
            movbyte(default_mb_cookie, contents, cookie_size);
        break;
        }
        break;
    default:
        printf("\nLoading default cookie format.\n");
        movbyte(default_mb_cookie, contents, cookie_size);
        break;
    }
}

int read_eeprom_block (unsigned int offset, unsigned int size, unsigned char *buf)
{
    printf("%s: To be implemented\n", __FUNCTION__);
    return (FAILED); 
}

/*
 * Function: init_cookie_default
 *
 * This function will initialize the cookie default contents for the
 * new cookie. The contents will be initialized in TLV format.
 *
 * Input:
 *    board_type  - Motherboard or module card.
 *    contents    - contents of the cookie.
 *
 * Output:
 *   none
 */
void init_cookie_default(int board_type, uint8_t * contents)
{
    uint sku_id_val;

    printf("%s: Viper currently skip SKU ID check.\n", __FUNCTION__);
    sku_id_val = C921J_E2E;

    printf("\n Loading default cookie contents ...\n");
    switch (sku_id_val) {
    case C921J_E2E:
        movbyte(default_921_e2e_cookie, contents, EEPROM_RD_WR_LENGTH);
        break;
    default:
        movbyte(default_mb_cookie, contents, EEPROM_RD_WR_LENGTH);
        break;
    }
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
 *    none
 *
 * Returns:
 *   PASSED/FAILED
 */
static int alter_cookie (int board_type)
{
    dev_if_info_t dev_if;
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

    if (plat_init_smart_eeprom_context(con, board_type, 0, 
                                      (uchar *)cookie_contents_buf) == FAILED) {
        printf("\n%s: Init Smart EEPROM context failed\n", __func__);
        return (FAILED);
    }

    act2_init_cont(con);
    con->quack_reset(con);

    tam_handle_ptr = NULL;
    printf("\nDBG :Select ACT2: tam_lib_device_open()\n");

    status = tam_lib_device_open(platform_opaque_handle,
                                 platform_buffer_size, &tam_handle_ptr);
    if (status != TAM_RC_OK) {
        printf("\n TAM lib: Cannot open handler: status = 0x%x", status);
        return (FAILED);
    }
    

    status = tam_lib_scc_read_eeprom(tam_handle_ptr,
                                (uint8_t *) cookie_contents_buf,
                                EEPROM_RD_WR_LENGTH, EEPROM_WRITE_ADDR);
    if (status != TAM_RC_OK) {
        printf("\n *** ERROR: tam_lib_scc_read_cookie. status = 0x%x",
               status);
        printf("\n Cannot read data from the EEPROM (Cookie). \n");
        return (FAILED);
    }

    /*
     * board_type is DAUGHTER_CARD, cookie_type is slot 0, cmd is NULL
     */
    ret_val = cookie_4_processor_x(cookie_contents_buf, board_type, 0,
                                   EEPROM_RD_WR_LENGTH, NULL);

    if (ret_val) {
        status = tam_lib_scc_write_eeprom(tam_handle_ptr,
                                     (uint8_t *) cookie_contents_buf,
                                     EEPROM_RD_WR_LENGTH,
                                     EEPROM_WRITE_ADDR);
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
 * Function: alter_mb_cookie
 *
 * Description:
 *   This function is the entry point for the alter motherboard ACT2
 *   cookie contents. It allow a user to edit/diaplay the contents of the
 *   cookie info within the ACT2 device.
 *
 * Parameters:
 *    none
 *
 * Returns:
 *   PASSED/FAILED
 */
int alter_mb_cookie(void)
{
    int  act2_chip;

    printf("\n\nSelect Act2 Chip:");
    act2_chip = getdec_answer("\n(0-Discrete-ACT2):", 0, 0, 0);
    if (act2_chip == DISCRETE_ACT2) {
        printf("\nSelect Discrete ACT2\n");
    }  
    tam_act2_reset(0);
    if (alter_cookie(MOTHER_BOARD) != PASSED ) {
        return (FAILED);
    }

    return (PASSED);
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
    dev_if_info_t dev_if;
    int  act2_chip;
    char *tname = "Smart Cookie";

    testname(tname);

    if (submenu_flag == 0) {
        prpass(testpass, "Main Menu");
    } else {
        prpass(testpass, "Sub Menu");
    }

    printf("\nEnter (m)otherboard, (Q)UIT  >");
    get_line(choice, sizeof(choice) + 1);

    switch (choice[0]) {
    case 'm':
        printf("\n\nSelect Act2 Chip:");
        act2_chip = getdec_answer("\n(0-Discrete-ACT2):", 0, 0, 0);
        if (act2_chip == DISCRETE_ACT2) {
            printf("\nSelect Discrete ACT2\n");
        }
        /*
         * Dummy initialized for error message "cont.dev_if_p that
         * is not initialized."
         */
        con = &cont;
        con->slot = 0;
        con->dev_if_p = &dev_if;
        con->dev_if_p->cookie_size = COOKIE_SIZE_512;

        plat_init_smart_eeprom_context(con, MOTHER_BOARD, 0, cookie_contents);
        act2_init_cont((void *) con);
        return (act2_prog(0));
        break;

    default:
        printf("Invalid input %s\n", choice);
        break;
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
        con_p->quack_read_2bytes = (PFT)i2c_quack_read_bytes;
        con_p->quack_write_2bytes = (PFT)i2c_quack_write_bytes;
        con_p->quack_reset = (PFT)i2c_quack_reset;
        con_p->dev_if_p->parm1 = (uint8_t)IOFPGA_I2C; 
        con_p->dev_if_p->parm2 = (uint8_t)MB_I2C_ADDR_ACT2;
        con_p->dev_if_p->parm3 = (uint8_t)MB_I2C_MUX_ACT2;  
        con_p->dev_if_p->parm4 = (uint8_t)I2C_CTRL_ZERO;   
        con_p->dev_if_p->interface = SCC_I2C_IF;
        sprintf((char *)smc_buf, "MB");
        break;
    default:
        cterr('f',0,"in plat_init_smart_eeprom_context: Not a supported "
              "Smart EEPROM type %d", type);
        retval = FAILED;
    }

    return (retval);
}

/*************************************************************************
 Function: cli_change_cookie
 *
 * This function is to search the cookie field and change field value
 *
 * Input: uchar *str
 *        int field
 *
 * Output: PASSED / FAILED
 **************************************************************************
*/
int cli_change_cookie(int field, char *str, cli_cookie_cmd * cmd)
{
    COOKIE_4 *buf;
    cookie_4_table *cp = cookie_4_info;

    /*
     * make sure the type been supported 
     */
    if ((cp = search_type_4_table(field)) == (cookie_4_table *) NULL)
        return (FAILED);

    if ((buf = buffer_search_x(TRUE, field)) == (COOKIE_4 *) NULL) {
        /*
         * Add Cookie Field if it doesn't exist in the buf pool
         */
        /*
         * accept new input type afterward 
         */
        if ((buf = get_new_buf()) == (COOKIE_4 *) NULL) {
            printf("Index out of COOKIE_4 buffer pool!!\n");
            return (FAILED);
        }

        buf->p_info = cp;       /* pointer in cookie_4_table */
        buf->type = field;
        cmd->type = CLI_COOKIE_ADD;
        /*
         * Don't need user interaction 
         */
        /*
         * prompt_for_display_format(buf);
         */

    }

    fetch_user_input_data(str, buf);

    if (cmd->type == CLI_COOKIE_ADD) {
        cookie_4_enque(cookie_root, buf);
    }

    return (PASSED);
}

/*************************************************************************
 Function: tam_lib_read_cookie
 *
 * This function is read the MB cookie content by tam library
 *
 * Input: none 
 *
 * Output: PASSED / FAILED
 **************************************************************************
*/
int tam_lib_read_cookie(void)
{
    void *platform_opaque_handle = NULL;
    uint16_t platform_buffer_size = PLATFORM_BUFF_SIZE;
    int ret_val;


    if (platform_tam_handle == NULL) {
        ret_val = tam_lib_device_open(platform_opaque_handle,
                                      platform_buffer_size,
                                      &platform_tam_handle);
        if (ret_val != TAM_RC_OK) {
            printf("\n TAM lib: Cannot open handler: status = 0x%x",
                   ret_val);
            printf("\n tan_handle = %p ", platform_tam_handle);
            return (FAILED);
        }
    }
    

    ret_val = tam_lib_scc_read_eeprom(platform_tam_handle,
                                (unsigned char *) cookie_contents,
                                EEPROM_RD_WR_LENGTH, EEPROM_WRITE_ADDR);
    if (ret_val != TAM_RC_OK) {
        printf("\n *** ERROR: tam_lib_scc_read_cookie. ret_val = 0x%x",
               ret_val);
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
 * Function: i2c_quack_read_bytes 
 * Description:
 * Input: none
 *
 * Output: PASS/FAIL
 *
 **********************************************************************
 */
int i2c_quack_read_bytes (sc_context *con_p, char *rx_buffer)
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
        if ((ret_status = n2g_i2c_read(n2g_i2c_if_p)) != RC_I2C_OP_OK) {
            sprintf(i2c_err,  i2c_err_str(ret_status));
            error_flag = FAILED;
            sleep(1);
        } else {
            return (PASSED);
        }
    }

    /* still need close it after this point */
    return (error_flag);
    return (PASSED);
}

/**********************************************************************
 *
 * Function: i2c_quack_write_bytes
 * Description:
 * Input: none
 *
 * Output: PASS/FAIL
 *
 **********************************************************************
 */
int i2c_quack_write_bytes (sc_context *con_p, char *tx_buffer, int tx_size)
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
ushort get_cookie_id(int slot, int type, uchar * eeprom_data,
              uint16_t * id, char *err)
{
    int ix;
    uint8_t *pdata = NULL;
    uint8_t ret_num_of_bytes = 0;
    uint8_t ctrl_type[CONTROL_TYPE_LEN] = { 0 };

    /* cookie type is MOTHERBOARD */
    g_cookie_type = type;
    if (g_cookie_type == MOTHER_BOARD) {
        if (tam_lib_read_cookie() == FAILED) {
            return (FAILED);
        }
    } 
    pdata =
        search_type_ret_addr_of_first_data(cookie_contents,
                                           CONTROLLER_TYPE,
                                           &ret_num_of_bytes, 0);
    if (pdata == NULL) {
        printf
            ("\n *** ERROR: Control Type field not programmed per Product PCAMAP.");
        return (FAILED);
    } else {
        ret_num_of_bytes =
            (ret_num_of_bytes >
             MAX_PID_LEN) ? MAX_PID_LEN : ret_num_of_bytes;
        for (ix = 0; ix < ret_num_of_bytes; ix++) {
            ctrl_type[ix] = *(pdata + ix);
        }
    }

    *id = (ctrl_type[0] << 8 | ctrl_type[1]);

    return (PASSED);
}

/**************************************************************************
 *
 * Name: get_pid
 *
 * Description: read product id
 *
 * Inputs: dummy - dummy parameters
 *         pid - product id
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
int get_pid(uchar * dummy, char *pid)
{
    int ix;
    uint8_t *pdata = NULL;
    uint8_t ret_num_of_bytes = 0;
    char prodName[PRODUCT_NAME_LEN] = { 0 };

    if (platform_tam_handle == NULL) {
        /* cookie type is MOTHERBOARD */
        if (g_cookie_type == MOTHER_BOARD) {
            if (tam_lib_read_cookie() == FAILED) {
                return (FAILED);
            }
        }   
    }

    pdata = search_type_ret_addr_of_first_data(cookie_contents, PRODUCT_ID,
                                               &ret_num_of_bytes, 0);
    if (pdata == NULL) {
        printf
            ("\n *** ERROR: Product id field not programmed per Product PCAMAP.");
        return (FAILED);
    } else {
        ret_num_of_bytes =
            (ret_num_of_bytes >
             MAX_PID_LEN) ? MAX_PID_LEN : ret_num_of_bytes;
        for (ix = 0; ix < ret_num_of_bytes; ix++) {
            prodName[ix] = *(pdata + ix);
        }
    }

    sprintf(pid, "%s", prodName);

    return (PASSED);
}

/*-----------------------------------------------------------------------------
 *
 * Function get_pcb_serial
 *
 * This function will return the SERIAL number.
 *
 * Inputs: eeprom_data - dummy parameters
 *         serial - control id
 *
 * Returns : PASSED/FAILED
 */
int get_pcb_serial(uchar * eeprom_data, char *serial)
{
    int ix;
    uint8_t *pdata = NULL;
    uint8_t ret_num_of_bytes = 0;
    char prodSN[PRODUCT_SERIAL_LEN] = { 0 };

    if (platform_tam_handle == NULL) {
        /* cookie type is MOTHERBOARD */
        if (g_cookie_type == MOTHER_BOARD) {
            if (tam_lib_read_cookie() == FAILED) {
                return (FAILED);
            }
        } 
    }

    pdata =
        search_type_ret_addr_of_first_data(cookie_contents, PCB_SERIAL_NUM,
                                           &ret_num_of_bytes, 0);
    if (pdata == NULL) {
        printf
            ("\n *** ERROR: Chassis Serial Number field not programmed per Product PCAMAP.\n");
        return (FAILED);
    } else {
        ret_num_of_bytes =
            (ret_num_of_bytes >
             MAX_CSN_LEN) ? MAX_CSN_LEN : ret_num_of_bytes;
        for (ix = 0; ix < ret_num_of_bytes; ix++) {
            prodSN[ix] = *(pdata + ix);
        }
    }

    sprintf(serial, "%s", prodSN);

    return (PASSED);
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
int print_cookie(int argc, char *argv[])
{
    int ix;
    uint8_t *pdata = NULL;
    uint8_t ret_num_of_bytes = 0;
    char prodName[PRODUCT_NAME_LEN];
    char vidName[VID_LEN];
    char prodSN[PRODUCT_SERIAL_LEN];

    memset(prodName, 0, sizeof(prodName));
    memset(vidName, 0, sizeof(vidName));
    memset(prodSN, 0, sizeof(prodSN));

    if (platform_tam_handle == NULL) {
        /* cookie type is MOTHERBOARD */
        if (g_cookie_type == MOTHER_BOARD) {
            if (tam_lib_read_cookie() == FAILED) {
                return (FAILED);
            }
        } 
    }

    printf("\nUDI");
    pdata = search_type_ret_addr_of_first_data(cookie_contents, PRODUCT_ID,
                                               &ret_num_of_bytes, 0);
    if (pdata == NULL) {
        printf
            ("\n *** ERROR: Product id field not programmed per Product PCAMAP.");
        return (FAILED);
    } else {
        printf("\n PID:");
        ret_num_of_bytes =
            (ret_num_of_bytes >
             MAX_PID_LEN) ? MAX_PID_LEN : ret_num_of_bytes;
        for (ix = 0; ix < ret_num_of_bytes; ix++) {
            printf("%c", *(pdata + ix));
            prodName[ix] = *(pdata + ix);
        }
    }

    pdata = search_type_ret_addr_of_first_data(cookie_contents, VERSION_ID,
                                               &ret_num_of_bytes, 0);
    if (pdata == NULL) {
        printf
            ("\n *** ERROR: Version id field not programmed per Product PCAMAP.");
        return (FAILED);
    } else {
        printf("\n VID:");
        ret_num_of_bytes =
            (ret_num_of_bytes >
             MAX_VID_LEN) ? MAX_VID_LEN : ret_num_of_bytes;
        for (ix = 0; ix < ret_num_of_bytes; ix++) {
            printf("%c", *(pdata + ix));
            vidName[ix] = *(pdata + ix);
        }
    }

    if (pcb_for_sudi == TRUE) {
        pdata =
        search_type_ret_addr_of_first_data(cookie_contents, PCB_SERIAL_NUM,
                                           &ret_num_of_bytes, 0);
    } else {
        pdata =
        search_type_ret_addr_of_first_data(cookie_contents, CHASSIS_SERIAL_NUM,
                                           &ret_num_of_bytes, 0);
    }
    if (pdata == NULL) {
        printf
            ("\n *** ERROR: Chassis Serial Number field not programmed per Product PCAMAP.\n");
        return (FAILED);
    } else {
        printf("\n SN:");
        ret_num_of_bytes =
            (ret_num_of_bytes >
             MAX_CSN_LEN) ? MAX_CSN_LEN : ret_num_of_bytes;
        for (ix = 0; ix < ret_num_of_bytes; ix++) {
            printf("%c", *(pdata + ix));
            prodSN[ix] = *(pdata + ix);
        }
    }

    return (PASSED);
}

/*------------------------------------------------------------------------
 * platform_get_pid
 *
 * DESCRIPTION:  Get the platform product id
 *
 * PARAMETERS: *pid - product id
 *
 * RETURNS:
 *     PASSED - if successful
 *     FAILED - if unsuccessful
 *--------------------------------------------------------------------------*/
int platform_get_pid (char *pid)
{
    char err[80];
    sc_context cont;
    uint16_t id = 0;
    uchar cookie[COOKIE_SIZE_512];

    /* Only the first time needs to go to get the PID. */
    if (get_sku_flag == FALSE) {
        get_sku_flag = TRUE;
        tam_act2_reset(0);
        if (get_cookie_id(cont.slot, 0, cookie, &id, err)==FAILED) {
            cterr('f', 0, "act2 program -- unable to read cookie id.");
            return (FAILED);
        }

        get_pid(cookie, get_prodName);
    }

    sprintf(pid, "%s", get_prodName);

    return (PASSED);
}

/**** not support on O2, but called on common code. ****/

int get_tlv_serial (uchar *eeprom_data, char *serial, uchar ser_tlv_field)
{

    uchar *data_ptr;
    uchar num_byte;

    if (platform_tam_handle == NULL) {
        /* cookie type is MOTHERBOARD */
        if (g_cookie_type == MOTHER_BOARD) {
            if (tam_lib_read_cookie() == FAILED) {
                return (FAILED);
            }
        } 
    }
    eeprom_data = cookie_contents;
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
ushort get_mb_id(void)
{
    int cookie_size = COOKIE_SIZE_512;
    uchar cookie_contents_buf[cookie_size];
    ushort cntl_type = INVALID_ID;
    sc_context *con, cont;
    dev_if_info_t dev_if;
    char err[80];
    ushort id = 0;

    /*
     * Dummy initialized for error message "cont.dev_if_p that
     * is not initialized."
     */
    con = &cont;
    con->type = MOTHER_BOARD;
    con->slot = 0;
    con->dev_if_p = &dev_if;
    con->dev_if_p->parm1 = (uint8_t) CPU_I2C0;
    con->dev_if_p->parm2 = (uint8_t) MB_I2C_ADDR_ACT2;
    con->dev_if_p->parm3 = (uint8_t) MB_I2C_MUX_ACT2;
    con->dev_if_p->parm4 = (uint8_t) MB_I2C_CTRL_ACT2;
    con->dev_if_p->cookie_size = COOKIE_SIZE_512;
    con->quack_read_2bytes = (PFT) i2c_quack_read_bytes;
    con->quack_write_2bytes = (PFT) i2c_quack_write_bytes;
    con->quack_reset = (PFT) i2c_quack_reset;
    con->dev_if_p->interface = SCC_I2C_IF;

    act2_init_cont((void *) con);

    if (tam_lib_read_cookie() == FAILED) {
        cterr('f', 0,"Failed to read Mother Board cookie\n");
        return(cntl_type);
    }

    if (get_cookie_id(cont.slot, cont.type, cookie_contents_buf, &id, err)==FAILED) {
        cterr('f', 0, "act2 program -- unable to read cookie id.");
        return(cntl_type);
    }

    return (id);
}

/*-------------------------------------------------
 * $Log: platform_cookie.c,v $
 * Revision 1.2  2018/08/06 02:31:52  harrchan
 * Merge viper E2E to the main trunk (CSCvk28469)
 *
 * Revision 1.1.2.4  2018/06/27 06:27:52  harrchan
 * Follow the coding rule to clean up code
 *
 * Revision 1.1.2.3  2018/06/14 01:28:46  harrchan
 * Remove Aikido option and Aikido keyword in whole source code
 *
 * Revision 1.1.2.2  2018/05/09 07:11:26  olin2
 * 1. Move GE and DSL init to the beginning. 2. Add has GE1. 3. Show cookie info
 *
 * Revision 1.1.2.1  2018/02/27 08:06:52  harrchan
 * Initial viper application code base
 *
 * $Endlog$
 *-------------------------------------------------
 */
