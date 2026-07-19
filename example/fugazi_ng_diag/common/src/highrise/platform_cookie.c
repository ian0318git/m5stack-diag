/* $Id: 
 * $Source: 
 *------------------------------------------------------------------
 * 
 * platform_cookie.c
 *
 * Copyright (c) 2019 - 2022 by Cisco Systems, Inc.
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
#include "object.h"
#include "tam_act2_api_drv_support.h"
#include "tam_aikido_mailbox.h"
#include "platform_stub.h"
#include "platform_i2c.h"
#include "tam_library.h"
#include "tam_lib_manufacturing.h"
#include "queryflags.h"
#include "i2c_api.h"
#include "n2g_api_rc.h"
#include "nvmonvars.h"
#include <assert.h>



/***********************************************************************
 *  Functions Declaration
 ************************************************************************/
int alter_mb_cookie(void);
int smartchip(int);
int tam_lib_read_cookie(void);
ushort get_cookie_id(int, int, uchar *, uint16_t *, char *);
int get_pid(uchar *, char *);
int get_vid(char *);
int get_pcb_serial(uchar *, char *);
int print_cookie(int, char *argv[]);
static int alter_cookie(int board_type);
int read_eeprom_block(unsigned, unsigned, unsigned char *);
int i2c_act2_write_bytes(sc_context *, char *, int);
int i2c_act2_read_bytes(sc_context *, char *);

/***********************************************************************
 *  Extern Functions Declaration
 ************************************************************************/
extern int cookie_4_processor_x(uchar *, int, int, int, cli_cookie_cmd *);
extern COOKIE_4 *buffer_search_x(boolean, int);
extern COOKIE_4 *get_new_buf(void);
extern void fetch_user_input_data(char *, COOKIE_4 *);
extern void cookie_4_enque(COOKIE_4 *, COOKIE_4 *);
extern void movbyte(unsigned char *, unsigned char *, int);
extern tam_lib_status_t tam_lib_scc_write_eeprom(void *tam_handle,
                         uint8_t * src_buffer,
                         uint16_t length, uint16_t dest);
extern int i2c_quack_reset(void);


extern boolean pcb_for_sudi;
extern COOKIE_4 *cookie_root;
extern cookie_4_table cookie_4_info[];
boolean aikido_mailbox_flag = FALSE;
boolean aikido_act2_flag = FALSE;


/***********************************************************************
 *  Static Variables
 ************************************************************************/
static void *platform_tam_handle = NULL;
static uchar cookie_contents[COOKIE_SIZE_512];
static int g_cookie_type = 0;
static uint8_t use_interrupt = 0;
static dev_if_info_t dev_if;
static char smc_buf[80];

static uchar default_mb_cookie[COOKIE_SIZE_512] = {
    0x04, 0xFF, 0xc3, 0x06, 0x74, 0x26, 0xac, 0xf2,
    0xb2, 0x8A, 0xc1, 0x8b, 0x44, 0x4e, 0x49, 0x50,
    0x31, 0x41, 0x31, 0x32, 0x33, 0x34, 0x35, 0x40,
    0x0c, 0xb0, 0x41, 0x01, 0x00, 0x09, 0xdd, 0x82,
    0x4a, 0x27, 0xb4, 0x00, 0x42, 0x30, 0x34, 0x88,
    0x00, 0x00, 0x00, 0x00, 0x02, 0x01, 0x81, 0x00,
    0x00, 0x00, 0x00, 0x04, 0x00, 0x43, 0x00, 0x0e,
    0xc2, 0x8b, 0x46, 0x47, 0x4c, 0x50, 0x31, 0x41,
    0x31, 0x32, 0x33, 0x34, 0x35, 0xcb, 0x89, 0x49,
    0x53, 0x52, 0x39, 0x36, 0x31, 0x2f, 0x4b, 0x39,
    0x89, 0x56, 0x30, 0x31, 0x20, 0x4a, 0xFF, 0xFF,
    0xc0, 0x46, 0x03, 0x20, 0x01, 0x8e, 0xbc, 0x01,
    0xc6, 0x8a, 0x54, 0x44, 0x42, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xcc, 0x86, 0x52, 0x45,
    0x56, 0x50, 0x31, 0x41, 0xFF, 0xFF, 0xFF, 0xFF,
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

/**********************************************************************
 *
 * Function: plat_init_smart_eeprom_context
 *
 * Description:
 *           intializes sc_context.
 * Input:  con_p   - pointer to sc_context
 *         type    - type of module (ie, mb, dc, plug, etc)
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

    con_p->info_string = smc_buf;
    con_p->dev_if_p = &dev_if;
    con_p->dev_if_p->cookie_size = COOKIE_SIZE_512;
    
    switch (type) {
    case MOTHER_BOARD:
        con_p->type = type;
        con_p->slot = slot;        /* slot = 0 */
        con_p->cookie_contents = cookie_p;
        // Obsoleted, True handler is tam_lib_platform_read in tam_act2_api_drv_support.c
        con_p->quack_read_2bytes = (PFT)i2c_act2_read_bytes;
        // Obsoleted, True handler is tam_lib_platform_write in tam_act2_api_drv_support.c
        con_p->quack_write_2bytes = (PFT)i2c_act2_write_bytes;
        con_p->quack_reset = (PFT)i2c_quack_reset;
        con_p->dev_if_p->parm1 = (uint8_t)MB_I2C_BUS_ACT2;
        con_p->dev_if_p->parm2 = (uint8_t)MB_I2C_ADDR_ACT2;
        con_p->dev_if_p->parm3 = (uint8_t)MB_I2C_MUX_ACT2;
        con_p->dev_if_p->parm4 = (uint8_t)MB_I2C_CTRL_ACT2;
        con_p->dev_if_p->interface = SCC_I2C_IF;
        sprintf((char *)smc_buf, "MB");
        break;
    default:
        printf("%s:%d:Not a supported Smart EEPROM type %d\n",
                __FUNCTION__, __LINE__, type);
        assert(!"plat_init_eeprom: invalid argument");
        retval = FAILED;
    }

    return (retval);
}

/**********************************************************************
 *
 * Function: platform_init_smart_context
 *
 * Description:
 *           intializes with sc_context.
 * Input:  con_p   - pointer to sc_context
 *         slot    - slot
 *         type    - type
 * Output: none
 *
 **********************************************************************
*/
void platform_init_smart_context (sc_context *con_p, int slot, int type) 
{

    plat_init_smart_eeprom_context(con_p, type, (uchar)slot, cookie_contents);

    act2_init_cont(con_p);
    con_p->quack_reset(con_p);
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
    printf("Warning: unsupport %s, %d \n", __FUNCTION__, __LINE__);
    return (PASSED);
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
    printf("Warning: unsupport %s, %d \n", __FUNCTION__, __LINE__);
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
void
init_cookie_4_default_x(int board_type, int cookie_type,
                        uchar * contents, int cookie_size)
{

    switch (board_type) {
    case MOTHER_BOARD:
        printf("\nLoading default cookie format for Motherboard.\n");
        movbyte(default_mb_cookie, contents, cookie_size);
        break;
    default:
        printf("\nLoading default cookie format.\n");
        movbyte(default_mb_cookie, contents, cookie_size);
        break;
    }
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

    printf("\n Loading default cookie contents ...\n");
    movbyte(default_mb_cookie, contents, EEPROM_RD_WR_LENGTH);
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
static int alter_cookie(int board_type)
{
    void *tam_handle_ptr = NULL;
    void *platform_opaque_handle = NULL;
    uint16_t platform_buffer_size = PLATFORM_BUFF_SIZE;
    uint8_t cookie_contents_buf[COOKIE_SIZE_512];
    tam_lib_status_t status;
    int ret_val;

    if (aikido_mailbox_flag) {
        /* Initialize Mailbox */
        status = tam_lib_device_open_mailbox(platform_opaque_handle, use_interrupt,
                                             MBX_MSG_SIZE, MBX_REG_BASE_ADDR,
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
            return (FAILED);
        }
    }

    status = tam_lib_scc_read_eeprom(tam_handle_ptr,
                                     (uint8_t *) cookie_contents_buf,
                                     EEPROM_RD_WR_LENGTH, EEPROM_WRITE_ADDR);
    if (status != TAM_RC_OK) {
        printf("\n *** ERROR: tam_lib_scc_read_cookie. status = 0x%x\n", status);
        printf("\n Cannot read data from the EEPROM (Cookie). \n");
        return (FAILED);
    }

    /* int cookie_4_processor_x (uchar *contents, int board_type,
		    int cookie_type, int cookie_size, cli_cookie_cmd *cli_cmd)
     * board_type : MOTHER_BOARD
     * cookie_type: not used in highrise, 0
     * cookie_size: 512
     * cmd        : NULL for menu; cli_cmd for cli
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

    /* tam_lib_device_close() will dis-associate the platform_opaque_handle
     * and nullify the tam_handle. */
    status = tam_lib_device_close(&tam_handle_ptr);
    if (status != TAM_RC_OK) {
        printf("\n TAM lib: Cannot close handler: status = 0x%x", status);
        return (FAILED);
    }

    return (PASSED);
}

/**************************************************************************
 * Function: alter_mb_cookie
 *
 * Description:
 *   This function is the entry point for the alter MB ACT2
 *   cookie contents. It allow a user to edit/diaplay the contents of the
 *   cookie info within the ACT2 device.
 *
 * Parameters:
 *    none
 *
 * Returns:
 *   PASSED/FAILED
 *************************************************************************/
int alter_mb_cookie(void)
{
    sc_context *con, cont;
    con = &cont;

    printf("\n\nAlter MB Act2 Chip:\n");
	aikido_act2_flag = FALSE;
    aikido_mailbox_flag = FALSE;

    platform_init_smart_context(con, 0, MOTHER_BOARD);

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
int smartchip(int submenu_flag)
{
    sc_context *con, cont;
    con = &cont;
    char *tname = "Smart Cookie";

    aikido_act2_flag = FALSE;
    aikido_mailbox_flag = FALSE;

    testname(tname);
    if (submenu_flag == 0) {
        prpass(testpass, "Main Menu");
    } else {
        prpass(testpass, "Sub Menu");
    }

    platform_init_smart_context(con, 0, MOTHER_BOARD);

    /* ACT2 library */
    return (act2_prog(0));

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

    if (aikido_mailbox_flag) {
	    /* Initialize Mailbox */
	    ret_val = tam_lib_device_open_mailbox(
                platform_opaque_handle,
                use_interrupt,
                MBX_MSG_SIZE,
                MBX_REG_BASE_ADDR,
                &platform_tam_handle);
	    if (ret_val != TAM_RC_OK) {
	            printf("\n [%s]:%d, Error: Can't Init Mailbox, return:%d \n",
                        __FUNCTION__, __LINE__, ret_val);
	            return (FAILED);
	    }
    } else {
        if (platform_tam_handle == NULL) {
            ret_val = tam_lib_device_open(
                    platform_opaque_handle,
                    platform_buffer_size,
                    &platform_tam_handle);
            if (ret_val != TAM_RC_OK) {
                printf("\n [%s]:%d, Error: Can't open tam handle, return:%d, tam_handle:%p\n",
                        __FUNCTION__, __LINE__, ret_val, platform_tam_handle);
                return (FAILED);
            }
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

    /* tam_lib_device_close() will dis-associate the platform_opaque_handle
     * and nullify the tam_handle. */
    ret_val = tam_lib_device_close(&platform_tam_handle);
    if (ret_val != TAM_RC_OK) {
        printf("\n TAM lib: Cannot close handler: status = 0x%x", ret_val);
        return (FAILED);
    }

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
ushort
get_cookie_id(int slot, int type, uchar * eeprom_data,
              uint16_t * id, char *err)
{
    int ix;
    uint8_t *pdata = NULL;
    uint8_t ret_num_of_bytes = 0;
    uchar ctrl_type[CONTROL_TYPE_LEN] = { 0 };
    sc_context *con, cont;
    con = &cont;

    /* cookie type is MOTHERBOARD */
    g_cookie_type = type;
    if (g_cookie_type == MOTHER_BOARD) {
        aikido_mailbox_flag = FALSE;
        platform_init_smart_context(con, slot, g_cookie_type);
    } else {
        return (FAILED);
    }


    if (tam_lib_read_cookie() == FAILED) {
        printf("\ntam_lib_read_cook fail.\n");
        return (FAILED);
    }

    memcpy(eeprom_data, cookie_contents, COOKIE_SIZE_512);

    pdata = search_type_ret_addr_of_first_data(cookie_contents,
                                           CONTROLLER_TYPE,
                                           &ret_num_of_bytes, 0);
    if (pdata == NULL) {
        printf("\n *** ERROR: Control Type field not programmed per Product PCAMAP.");
        return (FAILED);
    } else {
        // ret_num_of_bytes should be 2 bytes
        ret_num_of_bytes =
            (ret_num_of_bytes > MAX_PID_LEN) ? MAX_PID_LEN : ret_num_of_bytes;
        for (ix = 0; ix < ret_num_of_bytes; ix++) {
            ctrl_type[ix] = *(pdata + ix);
        }
    }

    *id = (ctrl_type[0] << 8 | ctrl_type[1]);

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("[%s]:%d, Control Type: %#x\n", __FUNCTION__, __LINE__, *id);
    }

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
    sc_context *con, cont;
    con = &cont;

    if (platform_tam_handle == NULL) {
        /* cookie type is MOTHERBOARD */
        if (g_cookie_type == MOTHER_BOARD) { 
            aikido_mailbox_flag = FALSE;
            platform_init_smart_context(con, 0, g_cookie_type);
        } else {
            return (FAILED);
        }

        if (tam_lib_read_cookie() == FAILED) {
            return (FAILED);
        }
    }

    pdata = search_type_ret_addr_of_first_data(
            cookie_contents,
            PRODUCT_ID,
            &ret_num_of_bytes, 0);
    if (pdata == NULL) {
        printf("\n *** ERROR: Product id field not programmed per Product PCAMAP.");
        return (FAILED);
    }
    // ret_num_of_bytes should be 13 bytes
    ret_num_of_bytes =
        (ret_num_of_bytes > MAX_PID_LEN) ? MAX_PID_LEN : ret_num_of_bytes;

    for (ix = 0; ix < ret_num_of_bytes; ix++) {
        prodName[ix] = *(pdata + ix);
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
    sc_context *con, cont;
    con = &cont;

    if (platform_tam_handle == NULL) {
        /* cookie type is MOTHERBOARD */
        if (g_cookie_type == MOTHER_BOARD) { 
            aikido_mailbox_flag = FALSE;
            platform_init_smart_context(con, 0, g_cookie_type);
        } else {
            return (FAILED);
        }

        if (tam_lib_read_cookie() == FAILED) {
            return (FAILED);
        }

        memcpy(eeprom_data, cookie_contents, COOKIE_SIZE_512);
    }

    pdata = search_type_ret_addr_of_first_data(
            cookie_contents,
            PCB_SERIAL_NUM,
            &ret_num_of_bytes, 0);

    if (pdata == NULL) {
        printf("\n *** ERROR: Serial Number field not programmed per Product PCAMAP.\n");
        return (FAILED);
    }

    // ret_num_of_bytes should be 11 bytes
    ret_num_of_bytes =
        (ret_num_of_bytes > MAX_CSN_LEN) ? MAX_CSN_LEN : ret_num_of_bytes;

    for (ix = 0; ix < ret_num_of_bytes; ix++) {
        prodSN[ix] = *(pdata + ix);
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
    sc_context *con, cont;
    con = &cont;

    memset(prodName, 0, sizeof(prodName));
    memset(vidName, 0, sizeof(vidName));
    memset(prodSN, 0, sizeof(prodSN));

    if (platform_tam_handle == NULL) {
        /* cookie type is MOTHERBOARD */
        if (g_cookie_type == MOTHER_BOARD) {
            aikido_mailbox_flag = FALSE;
            platform_init_smart_context(con, 0, g_cookie_type);
        } else {
            return (FAILED);
        }

        if (tam_lib_read_cookie() == FAILED) {
            return (FAILED);
        }
    }

    printf("\nUDI");
    pdata = search_type_ret_addr_of_first_data(cookie_contents, PRODUCT_ID,
                                               &ret_num_of_bytes, 0);
    if (pdata == NULL) {
        printf("\n *** ERROR: Product id field not programmed per Product PCAMAP.");
        return (FAILED);
    }

    printf("\n PID:");
    ret_num_of_bytes =
        (ret_num_of_bytes > MAX_PID_LEN) ? MAX_PID_LEN : ret_num_of_bytes;

    for (ix = 0; ix < ret_num_of_bytes; ix++) {
        printf("%c", *(pdata + ix));
        prodName[ix] = *(pdata + ix);
    }


    pdata = search_type_ret_addr_of_first_data(cookie_contents, VERSION_ID,
                                               &ret_num_of_bytes, 0);
    if (pdata == NULL) {
        printf ("\n *** ERROR: Version id field not programmed per Product PCAMAP.");
        return (FAILED);
    }
    printf("\n VID:");
    ret_num_of_bytes =
        (ret_num_of_bytes > MAX_VID_LEN) ? MAX_VID_LEN : ret_num_of_bytes;

    for (ix = 0; ix < ret_num_of_bytes; ix++) {
        printf("%c", *(pdata + ix));
        vidName[ix] = *(pdata + ix);
    }

    if (pcb_for_sudi == TRUE) {
        pdata = search_type_ret_addr_of_first_data(
                cookie_contents,
                PCB_SERIAL_NUM,
                &ret_num_of_bytes, 0);
    } else {
        pdata = search_type_ret_addr_of_first_data(
                cookie_contents,
                CHASSIS_SERIAL_NUM,
                &ret_num_of_bytes, 0);
    }
    if (pdata == NULL) {
        printf
            ("\n *** ERROR: Chassis Serial Number field not programmed per Product PCAMAP.\n");
        return (FAILED);
    }
    printf("\n SN:");
    ret_num_of_bytes =
        (ret_num_of_bytes > MAX_CSN_LEN) ? MAX_CSN_LEN : ret_num_of_bytes;

    for (ix = 0; ix < ret_num_of_bytes; ix++) {
        printf("%c", *(pdata + ix));
        prodSN[ix] = *(pdata + ix);
    }


    return (PASSED);
}


/**** not support on O2, but called on common code. ****/
int get_tlv_serial (uchar *eeprom_data, char *serial, uchar ser_tlv_field)
{

    uchar *data_ptr;
    uchar num_byte;
    sc_context *con, cont;
    con = &cont;

    if (platform_tam_handle == NULL) {
        /* cookie type is MOTHERBOARD */
        if (g_cookie_type == MOTHER_BOARD) {
            aikido_mailbox_flag = FALSE;
            platform_init_smart_context(con, 0, g_cookie_type);
        } else {
            return (FAILED);
        }

        if (tam_lib_read_cookie() == FAILED) {
            return (FAILED);
        }
    }
    memcpy(eeprom_data, cookie_contents, COOKIE_SIZE_512);
    if (eeprom_data[0] == CURRENT_FORMAT_VERSION) {
        /* for polling slots, do not print warning. simply print the content */
        if ((data_ptr = (uchar *)search_type_ret_addr_of_first_data(
                        eeprom_data, ser_tlv_field, &num_byte, FALSE)) == NULL) {
            sprintf(serial, "NO S/N NUM");
            printf("\n *** ERROR: 0x%02x Serial Number field not programmed per Product PCAMAP.",
                    ser_tlv_field);
            return (FAILED);
        } else {
            memcpy(serial, data_ptr, num_byte);
            return (PASSED);
        }
    } else {
        printf("%s:%d:Unsupported old cookie format 0x%02x\n",
                __FUNCTION__, __LINE__, eeprom_data[0]);
        return (FAILED);
    }
}

int read_eeprom_block (unsigned int offset, unsigned int size, unsigned char *buf)
{
    printf("Error: [%s] highrise should never hit this function!\n", __FUNCTION__);
    return (FAILED);
}

/**************************************************************************
 *
 * Name: get_vid 
 *
 * Description: read version id
 *
 * Inputs: dummy - dummy parameters
 *         pid - product id
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
int get_vid (char *vid)
{
    int ix;
    uint8_t *pdata = NULL;
    uint8_t ret_num_of_bytes = 0;
    char vidName[VID_LEN];
    sc_context *con, cont;
    con = &cont;

    memset(vidName, 0, sizeof(vidName));

    if (platform_tam_handle == NULL) {
        /* cookie type is MOTHERBOARD */
        if (g_cookie_type == MOTHER_BOARD) { 
            aikido_mailbox_flag = FALSE;
            platform_init_smart_context(con, 0, g_cookie_type);
        } else {
            return (FAILED);
        }

        if (tam_lib_read_cookie() == FAILED) {
            return (FAILED);
        }
    }

    pdata = search_type_ret_addr_of_first_data(cookie_contents, VERSION_ID,
                                               &ret_num_of_bytes, 0);
    if (pdata == NULL) {
        printf ("\n *** ERROR: Version id field not programmed per Product PCAMAP.");
        return (FAILED);
    }
    ret_num_of_bytes =
        (ret_num_of_bytes > MAX_VID_LEN) ? MAX_VID_LEN : ret_num_of_bytes;

    for (ix = 0; ix < ret_num_of_bytes; ix++) {
        vidName[ix] = *(pdata + ix);
    }

    sprintf(vid, "%s", vidName);

    return (PASSED);
}

/* for plat p2 and later */
int is_plat_p2 (void)
{
    char vidName[VID_LEN];

    get_vid(vidName);

    /* 3 for V01, 3 chars only */
    if ((strncmp(vidName, HR_V01, 3) != 0) &&
            (strncmp(vidName, HR_v01, 3) != 0)) {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("platform is p2 and later \n"); 
        } 
        /* p2 and later */
        return (TRUE);
    } else {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("platform is p1 \n"); 
        } 
        /* p1 */ 
        return (FALSE);
    }
}
 

