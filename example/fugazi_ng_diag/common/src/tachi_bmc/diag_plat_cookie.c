/* $Id: diag_plat_cookie.c,v 1.7 2018/05/09 03:53:36 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_plat_cookie.c,v $
 *------------------------------------------------------------------
 *
 * diag_plat_cookie.c - Platform cookie function
 * 
 * July 2015, Times Huang
 *
 * Copyright (c) 2015-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ioctl.h> 
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
#include "diag_plat_cookie.h"
#include "act2_utils.h"
#include "proto.h"
#include <tam_library.h>
#include <tam_lib_manufacturing.h>
#include "nmc93c46.h"
#include "smart_cookie.h"
#include "platform_i2c.h"
#include "plat_defs.h"
#include "slot.h"
#include "diag_pem_lib.h"
#include "mb_tests.h"
#include "ngio.h"

extern int tam_act2_reset(int);
extern tam_lib_status_t tam_lib_scc_write_eeprom(void *, uint8_t *, uint16_t, uint16_t);
extern int cookie_4_processor_x(uchar *, int, int, int, cli_cookie_cmd *);
extern int cookie_is_act2(sc_context *);
extern int slot_get_pcb_serial(uchar *, char *);   
int alter_mb_cookie(void);
int alter_nim_cookie(int);
int alter_poe_cookie(void);
int alter_raid_cookie(void);
int cli_change_cookie(int, char *, cli_cookie_cmd *);
void init_cookie_4_default_x(int, int, uchar *, int);
int read_eeprom_block(unsigned int, unsigned int, unsigned char *);
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
static uchar default_tachi_l_contents[COOKIE_SIZE_512] = {
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

int smartchip (int submenu_flag)
{
    char choice[5], type;
    sc_context *con, cont;
    int slot;
    struct ngio_intf_t *ngio;
    

    con = &cont;
    con->dev_if_p = &dev_if;
    con->dev_if_p->cookie_size = COOKIE_SIZE_512;
    
    
    testname("Smart cookie");
    if (submenu_flag == 0) {
        prpass(testpass, "Main Menu");
    } else {
        prpass(testpass, "Sub Menu");
    }

    printf("\nEnter (m)otherboard, (n)im, (i)sp(Q)UIT  >");
    get_line(choice, sizeof(choice) + 1);
    
    switch (choice[0]) {
    case 'm':

        plat_init_smart_eeprom_context(con, MOTHER_BOARD, 0, cookie_contents);

        act2_init_cont((void *)con);

        return (act2_prog(0));
        break;
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
        
        if (plat_init_smart_eeprom_context(con,type, slot, cookie_contents) == FAILED) {
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
int is_poe_sku (void) 
{
    char sku_name[32], poe;
    uchar pid[32]; 
    FILE *fp; 

    if (access(MB_PID_FILE, F_OK) == -1 ) {
        /* file doesn't exist, create it */
        store_mb_pid();
    }

    fp = fopen(MB_PID_FILE, "r");
    if (fp != NULL) {
        fscanf(fp, "%s", sku_name);
        fclose(fp);
    } else {
        printf("Failed to open %s \n", MB_PID_FILE);
    }

    /*
     * For enhanced error message(EEM), EEM will show the sku_id when there 
     * any failure happen
     */
    strcpy(sku_id, sku_name);

    /* 031616 - an example for new PID is ENCS5408/K9 
     * '8' means cores number, then we use minus 4 
     * for catch it. 
     */
    poe = sku_name[strlen(sku_name)-4];
    printf("Platform SKU: %s\n", sku_name);

    /* 6 cores has no POE connector 
     * 8 and 12 cores have POE connector 
     */
    if (poe != '6') {
        if (pem_rd_cookie_pid(pid) != PASSED) {
            printf("Failed to read PSU cookie\n");
            return (FALSE);
        }
        if(strncmp((char *)pid, PSU_POE_SKU_STR, sizeof(PSU_POE_SKU_STR) -1)) {
            printf("Platform is POE SKU with PSU NOT support POE\n");
            return (FALSE);

        } else {
            sprintf(sku_name, "touch %s", MB_POE_SKU); 
            system(sku_name); 
            printf("Platform is POE SKU with PSU support POE\n");
            return (TRUE); 
        }
    } else {
        printf("Platform is NOT POE SKU\n");
        return (FALSE); 
    }
}

int print_cookie (int argc, char *argv[])
{
    int ix;
    uint8_t *pdata = NULL;
    uint8_t ret_num_of_bytes = 0;
    char prodName[PRODUCT_NAME_LEN];
    char vidName[VID_LEN];
    char prodSN[PRODUCT_SERIAL_LEN] = { 0 };

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
            prodName[ix] = *(pdata + ix);
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
            vidName[ix] = *(pdata + ix);
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
            prodSN[ix] = *(pdata + ix);
        }
    }

    return (PASSED);
}

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
    con->quack_reset(con);

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
    /* SM cookie_size ??? */
    con->dev_if_p->cookie_size = COOKIE_SIZE_512;

    plat_init_smart_eeprom_context(con, type, slot, cookie_contents);
    act2_init_cont(con);
    con->quack_reset(con);

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
        printf("\nLoading default cookie format for Tachi Motherboard.\n");
        movbyte(default_tachi_l_contents, contents, cookie_size);
        break;
    default:
        movbyte(default_tachi_l_contents, contents, cookie_size);
        break;
    }
}

int read_eeprom_block (unsigned int offset, unsigned int size, unsigned char *buf)
{
    printf("%s: To be implemented\n", __FUNCTION__);
    return (FAILED); 
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
int cli_change_cookie (int field, char *str, cli_cookie_cmd * cmd)
{
    return (PASSED);
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
int alter_nim_cookie (int slot) 
{
    return (alter_cookie(WIC_MODULE, slot));
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
    return (alter_cookie(PSU_MODULE, POE_CARD));
}

int alter_raid_cookie (void)
{
    return (alter_cookie(ISP_CARD, RAID_CARD));
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
    return (alter_cookie(MOTHER_BOARD, 0));
}


static int tam_lib_read_cookie (void)
{
    int ret_val;
    void *platform_opaque_handle = NULL;
    void *platform_tam_handle = NULL;

    ret_val = tam_lib_device_open(platform_opaque_handle, PLATFORM_BUFF_SIZE,
                                  &platform_tam_handle);

    if (ret_val != TAM_RC_OK) {
        printf("\n TAM lib: Cannot open handler: status = 0x%x", ret_val);
        return (FAILED);
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
    char pid[128];
    char serial[48];

    if (plat_init_smart_eeprom_context(con, board_type, slot, 
                                      (uchar *)cookie_contents_buf) == FAILED) {
        printf("\n%s: Init Smart EEPROM context failed\n", __func__);
        return (FAILED);
    }

    act2_init_cont(con);
    con->quack_reset(con);

   if (board_type == WIC_MODULE) {
        if (!cookie_is_act2(con)) {
        printf("Cookie utility doesn't support Quack chip!\n");   
        smart_cookie_read(con);
            if (get_pid (cookie_contents_buf, pid)== FAILED) { 
                printf("%s:read PID failed\n",__func__);
            } else {
                printf("PID = %s\n",pid);
            } 
            slot_get_pcb_serial (cookie_contents_buf, serial);
            printf("Serial Number = %s",serial);
            return (FAILED);
        }
    }

    tam_handle_ptr = NULL;
    status = tam_lib_device_open(platform_opaque_handle,
                                 platform_buffer_size, &tam_handle_ptr);
                                 
    if (status != TAM_RC_OK) {
        printf("\n TAM lib: Cannot open handler: status = 0x%x", status);
        return (FAILED);
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
$Log: diag_plat_cookie.c,v $
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

