/* $Id: tam_act2_utils.c,v 1.30 2021/06/09 08:54:23 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tam_act2_utils.c,v $
 *------------------------------------------------------------------
 * FILE NAME : tam_act2_utils.c
 * 
 * This file contains code developed for the ACT2-TAM device.  Some of the printfs are very important.
 * the string output is captured by the script so if you modify the output, you may break manufacturing script.
 * for example, note that getdec_answer defaults answer is 0.  Manufacturing script expects "0" to show up on
 * the screen. if the default value is changed, the script will break.
 * Also, under linux, the max stdin buffer is 4K (we don't understand why buffer is small). so we need to break
 * some of the stdin queries into multiple queries, with each query taking in at most 4K data.
 *
 * Copyright (c) 2014-2021 by Cisco Systems, Inc.
 * All rights reserved.
 *
 * Author: Ian Chang
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "types.h"
#include "common.h"
#include "menu.h"
#include "queryflags.h"
#include "nvsysvars.h"
#include "nmc93c46.h"
#include "cross_platform.h"
#include "smart_cookie.h"
#include "cookie_4.h"
#include "proto.h"
#include "i2c_api.h"
#include "platform_cookie.h"
#include "platform_i2c.h"
#include "dash_fpga.h"
#include "error.h"
#include "ngio.h"
#include "slot.h"
#include "tam_library.h"
#include "tam_lib_manufacturing.h"
#include "tam_aikido_mailbox.h"
#ifdef PLAT_HAS_PLUG
#include "plug_slot.h"
#endif 
#include "tam_aikido_upgrade.h"
#if defined (SHA1_SHA2_SWSUDI2099) || defined (AIKIDO_CT_DEV_KEY) || defined (WDC_OBJECT_NAME)
#include "tam_lib_object_manager.h"
#endif
#include "key_tlv_parser.h"

#define SYS_PROC_PRINTK_FILE                        "/proc/sys/kernel/printk"
#define SYS_CHANGE_PRINTK_LEVEL                     "dmesg -n"
#define SYS_SUPPRESS_PRINTK_LEVEL                   (3)
#define SYS_RESTORE_PRINTK_CMD                      "dmesg -n"
#define WDC_DUMMY_OBJ_NUMBER                        (2)
#define DUMMY_WRITE_SIZE                            (10)
#define DUMMY_OBJ_SIZE                              (4096 - data_size) /* One sector = 4K Bytes */
#define PRODSN_NO                                   (11)
#define WDC_OBJ_NAME                                "WDC_Object"
#define OBJ_NAME_LEN                                (128)
#define RNG_TEST_LOOP                               (10)
#define RNG_SIZE                                    (100)
#define RNG_SIZE_INCREASE                           (10)
#ifdef SUDI_2099_2029
/* SUDI_2099 */
#define PRINT_SUDI_2099_STS                         \
        printf("\nTAM 2099 is '%s'\n", sudi_2099 == TRUE ? "ON" : "OFF");
        
#ifndef HARSA_SUDI
#define HARSA_SUDI                                  (3)
#endif
#endif

/* 
 * ECC SUDI will be supported starting from ACT2 v1.5 chip.
 * However, those codes are not tested since ECC has not been used yet.
 * So, comment out the codes.
 */
//#define SUPPORT_ECC
/* from manufac script data size is 958. translate to assic that 958 * 3 each data
   has 2 bytes (ie, 9e), plus space, so bufer should be at least 958 *3. */
#define SUDI_MAX_SIZE ((WDC_SIZE * 3 ) + 100)
/* FW version v1.5 baseline */
#define TAM_ACT2_V1_5_FW    (0x1a)

#ifdef SUDI_2099_2029
/* SUDI_2099: The 2099 SUDI TAM Library version should be V3.1.1 or later */
#define TAM_LIB_2099_VERSION            (0x030101)
#endif

#define TAM_AIKIDO_AIK_REQUIRED_FW_VERSION    (0x22) /* fw ver 2.2 */

extern int print_cookie(int argc,char *argv[]);
extern char atoh(char c);

int act2_i2c_debug = 0;
boolean pcb_for_sudi = TRUE;

#ifdef SUDI_2099_2029
/* SUDI_2099 */
boolean sudi_2099 = FALSE;
#endif

#ifdef AIKIDO_CT_DEV_KEY
boolean rem_dev_key_found = FALSE;  /* Removal Dev Key Flag */
#endif
static int default_printk_level = -1;

/************************************************
Below is the list of error conditions returned from library:
"TAM_LIB_ERR_CKSUM_BAD",                //0x0B
"TAM_LIB_ERR_CMD_UNKNOWN",              //0x0C
"TAM_LIB_ERR_CMD_LENGTH",               //0x0D
"TAM_LIB_ERR_PARAMETER_INVALID",        //0x0E
"TAM_LIB_ERR_ALGORITHM_INVALID",        //0x0F
"TAM_LIB_ERR_PERMISSION_NOT",           //0x10
"TAM_LIB_ERR_SESSION_GEN_FAIL",         //0x11
"TAM_LIB_ERR_SESSION_NOT_AVAIL",        //0x12
"TAM_LIB_ERR_SESSION_IS_OPEN",          //0x13
"TAM_LIB_ERR_SESSION_INVALID",          //0x14
"TAM_LIB_ERR_HANDLE_INVALID",           //0x15
"TAM_LIB_ERR_SORW_PENDING",             //0x16
"TAM_LIB_ERR_RESEED_REQUIRED",          //0x17
"TAM_LIB_ERR_USERID_UNKNOWN",           //0x18
"TAM_LIB_ERR_USERID_INVALID",           //0x19
"TAM_LIB_ERR_USERID_EXISTS",            //0x1A
//serial number C1; product id; : 0x CB
0,        //0x1B
0,        //0x1C
0,        //0x1D
0,        //0x1E
0,        //0x1F
"TAM_LIB_ERR_BOUNDS_CHECK",             //0x20
"TAM_LIB_ERR_EEPROM_SPACE",             //0x21
"TAM_LIB_ERR_EEPROM_WRITE",             //0x22
"TAM_LIB_ERR_ALGORITHM",                //0x23
"TAM_LIB_ERR_KEY_LENGTH",               //0x24
"TAM_LIB_ERR_OBJECT_DIRTY",             //0x25
"TAM_LIB_ERR_OBJECT_GENERAL",           //0x26
"TAM_LIB_ERR_OBJECT_IN_ROM",            //0x27
"TAM_LIB_ERR_OBJECT_LENGTH",            //0x28
"TAM_LIB_ERR_OBJECT_PERMISSION",        //0x29
"TAM_LIB_ERR_OBJECT_PRIVATE",           //0x2A
"TAM_LIB_ERR_OBJECT_TYPE",              //0x2B
"TAM_LIB_ERR_PROCESS_FAIL",             //0x2C
"TAM_LIB_ERR_RAM_SPACE",                //0x2D
"TAM_LIB_ERR_VERIFY_FAIL",              //0x2E
"TAM_LIB_ERR_DATA_FORMAT",              //0x2F
"TAM_LIB_ERR_LIBRARY",                  //0x30
"TAM_LIB_ERR_CODE_FAIL",                // 0x31
"TAM_LIB_ERR_FIPS_TEST_FAIL",           // 0x32
"TAM_LIB_ERR_ZEROED_IDEVID_KEYS",       // 0x33 

"TAM_LIB_ERR_UDI_INVALID",              // 0xE0
"TAM_LIB_ERR_UDI_MISMATCH",             // 0xE1
                                           
"TAM_LIB_ERR_FW_VERSION",               // 0xF1
"TAM_LIB_ERR_BUS_TOO_SMALL",            // 0xF2
"TAM_LIB_ERR_DUPLICATE",                // 0xF3
"TAM_LIB_ERR_INVALID_STATE",            // 0xF4
"TAM_LIB_ERR_USER_NOT_LOGGED",          // 0xF5
"TAM_LIB_ERR_NULL_HANDLE",              // 0xF6
"TAM_LIB_ERR_DETECTED",                 // 0xF7
"TAM_LIB_ERR_RANGE_INVALID",            // 0xF8
"TAM_LIB_ERR_NULL_POINTER",             // 0xF9
"TAM_LIB_ERR_NO_RESOURCES",             // 0xFA
"TAM_LIB_ERR_READ_FAILURE",             // 0xFB
"TAM_LIB_ERR_READ_CHECKSUM_FAILURE",    // 0xFC
"TAM_LIB_ERR_WRITE_FAILURE",            // 0xFD
"TAM_LIB_ERR_INVALID_LENGTH",           // 0xFE
"TAM_LIB_ERR_NOT_SUPPORTED",            // 0xFF
***********/


static uchar cookie[COOKIE_SIZE_512];    
static char prodSN[20];
static char pcbSN[20];
static char chassisSN[20];
static char prodName[256];
static char prodName_debug[256];
static uint16_t id = 0;
void *tam_handle = NULL;

/* version 0x17 (or 23 dec)  or above is act2; below is act1 */
static int act2_chip_version;
static sc_context cont;
static dev_if_info_t dev_if;
static int act2_toggle_debug_flag(int dummy);
static int tam_act2_switch_simple_mode(int dummy);
static int tam_act2_get_serial_num(void);
static int tam_act2_get_session_id(void);
static int tam_act2_mfg_login(void);
static int tam_act2_install_CLIIP(void);
static int tam_act2_install_SUDI(void);
static int tam_act2_close_mfg_login(void);
static int tam_act2_get_cskmp(void);
static int tam_act2_version(int dummy);
static int tam_act2_test(int);
static int tam_act2_enumerate(int);
static int tam_act2_authenticate_udi(int);
static int tam_act2_platform_object_burn_in_test(int dummy);
static int tam_user_act2l_read_sudi(void);
extern int smart_cookie_read_x(sc_context *con_p, ushort size);
extern int quack_version(sc_context *con);
static int tam_act2_install_wdc(int);
static int tam_act2_display_wdc(int);
static int get_serial_number_for_sudi(void);
static int tam_act2_rng_test(int);

/* This is the session id that is generated during mfg login */
static uint32_t session_id = 0;
static uint act2_simple_mode[MAX_DEVICE_TYPE];
static int device_type = MOTHER_BOARD;

int tam_act2_reset(int);
int tam_mfg_errinfo(int);
int tam_mfg_errinfo_clr(int);
int tam_lib_display_scc_id(int);
boolean menu_display(void);
boolean aikido_hide_toggle_2099_item(void);
int tam_act2_generate_ecskmp(void);
static int act2_i2c_unit_test(void);
#ifdef SUDI_2099_2029
static int is_tam_library_sudi_2099(void);
#endif
#ifdef ENABLE_SUDI_2099_CLI
static int tam_toggle_sudi_2099(void);
#endif
static void tam_show_library_version(void);
static unsigned int get_aikido_chip_info(tam_lib_chip_info_t *); 
static int dump_aikido_chip_info(void);
static int dump_aikido_fpga_version(void); 
unsigned int aikido_espi_read_util(void);
void is_tam_aikido_on_wrapper(void);
unsigned int tam_lib_fw_threshold(unsigned int);
#ifdef AIKIDO_SUPPORT_AIK
boolean aikido_aik_flag = FALSE; 
static void tam_setup_aikido_aik_flag(boolean *); 
static void toggle_aikido_aik_flag(void);
#endif

#ifdef SHA1_SHA2_SWSUDI2099
#define ASCII_CHAR_PER_BYTE 2
#define GET_WORD_BLOCK_LENGTH 200
#define GET_WORD_BYTES_PER_BLOCK ( GET_WORD_BLOCK_LENGTH / ASCII_CHAR_PER_BYTE )
#define CHAR_PER_NEWLINE 1
#define SUDI_MAX_SIZE ((WDC_SIZE * 3 ) + 100)
#define  MAX_ACT2_OBJ_LEN 8192 
static int select_sha1_sha2_swsudi(void);
static int tam_act2_install_sha1_sha2_swsudi(int);
static int tam_act2_display_sha1_sha2_swsudi(int);
static int act2_enter_sha1_sha2_swsudi_obj(uint8_t**,uint16_t);
static int act2_install_sha1_sha2_swsudi_obj(uint8_t**,uint16_t);
static uint16_t  which_act2_obj_param  = 0;
static char act2_obj_name[][32] = { "SudCrtSha1",
                                    "SudKeySha1",
                                    "SudCrtSha2",
                                    "SudKeySha2",
                                    "SudCrt99",
                                    "SudKey99" };
/*used for Display purpose*/
static char act2_obj_disp_name[][32] = { "SHA1 Cert",
                                         "SHA1 Key",
                                         "SHA2 Cert",
                                         "SHA2 Key",
                                         "SWSUDI2099 Cert",
                                         "SWSUDI2099 Key" };
#endif
#ifdef AIKIDO_CT_DEV_KEY
/*used for Consent Token purpose*/
static char ct_obj_name[][32] = { "RM_DEV_KEY",
                                  "CT_RM_BLOB",
                                  "CT_CTX" };
#define CT_MAX_OBJECTS      (sizeof(ct_obj_name) / sizeof(ct_obj_name[0]))
#endif
unsigned int aikido_espi_write_header_util(void);

/* 
 * Sub Menu used for ACT-2 utility.
 */
submenu_xtable_t tam_act2_submenu_table[] = {
    {"Switch simple mode",         (PFT)tam_act2_switch_simple_mode,  0, 0, (type_t(*)())0,0,(type_t(*)())0,0},
    {"Get serial number",          (PFT)tam_act2_get_serial_num,      0, 0, (type_t(*)())0,0,(type_t(*)())0,0},
    {"Display session id",         (PFT)tam_act2_get_session_id,      0, 0, (type_t(*)())0,0,(type_t(*)())0,0},
    {"Manufacturing login",        (PFT)tam_act2_mfg_login,           0, 0, (type_t(*)())0,0,(type_t(*)())0,0},
    {"Install identity",           (PFT)tam_act2_install_CLIIP,       0, 0, (type_t(*)())0,0,(type_t(*)())0,0},
    {"Verify identity",            (PFT)tam_act2_install_SUDI,        0, 0, (type_t(*)())0,0,(type_t(*)())0,0},
    {"Authenticate (n/a)/display cert", (PFT)tam_act2_authenticate_udi, 0, MF_HIDDEN_EXE, (type_t(*)())menu_display,0,(type_t(*)())tam_act2_authenticate_udi,0},
    {"Install WDC",                (PFT)tam_act2_install_wdc,         0, 0, (type_t(*)())0,0,(type_t(*)())0,0},
    {"Close manufacturing login",  (PFT)tam_act2_close_mfg_login,     0, 0, (type_t(*)())0,0,(type_t(*)())0,0},
    {"Display CSKMP",              (PFT)tam_act2_get_cskmp,           0, 0, (type_t(*)())0,0,(type_t(*)())0,0},
    {"Display SCC Version",        (PFT)tam_act2_version,             0, 0, (type_t(*)())0,0,(type_t(*)())0,0},
    {"ACT2 <--> Diags comminucation test",   (PFT)tam_act2_test,      0, 0, (type_t(*)())0,0,(type_t(*)())0,0},
    {"Check for identity",         (PFT)tam_user_act2l_read_sudi,     0, 0, (type_t(*)())0,0,(type_t(*)())0,0},
    {"Enumerate object",           (PFT)tam_act2_enumerate,           0, 0, (type_t(*)())0,0,(type_t(*)())0,0},
    {"burn in test",               (PFT)tam_act2_platform_object_burn_in_test,     0, MF_CONTINUOUS | MF_DOGRP,
     (type_t(*)())0, MF_CONTINUOUS | MF_DOGRP, (type_t(*)())0,0},
    {"Reset MB Quack chip",           (PFT)tam_act2_reset,     0, MF_CONTINUOUS | MF_DOGRP,
     (type_t(*)())0, MF_CONTINUOUS | MF_DOGRP, (type_t(*)())0,0},
    {"toggle act2 debug flag",     (PFT)act2_toggle_debug_flag, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0,0},
    {"Display TAM MFG Error",      (PFT)tam_mfg_errinfo,              0, 0, (type_t(*)())0,0,(type_t(*)())0,0},
    {"Clear TAM MFG Error",        (PFT)tam_mfg_errinfo_clr,          0, 0, (type_t(*)())0,0,(type_t(*)())0,0},
    {"Get TAM Chip Info",          (PFT)tam_lib_display_scc_id,       0, 0, (type_t(*)())0,0,(type_t(*)())0,0},
    {"Select S/N for SUDI/WDC", (PFT)get_serial_number_for_sudi,  0, 0, (type_t(*)())0,0,(type_t(*)())0,0},
    {"Generate ECSKMP",            (PFT)tam_act2_generate_ecskmp,    0, 0, (type_t(*)())0,0,(type_t(*)())0,0},
    {"Discrete ACT2 Offline test", (PFT)act2_i2c_unit_test,  0, MF_HIDDEN_EXE, (type_t(*)())menu_display,0,(type_t(*)())act2_i2c_unit_test,0},
#if defined (AIKIDO_CT_DEV_KEY) || defined (WDC_OBJECT_NAME)
    {"Display WDC & Other Objects",(PFT)tam_act2_display_wdc,         0, 0, (type_t(*)())0,0,(type_t(*)())0,0},
#else
    {"Display WDC",                (PFT)tam_act2_display_wdc,         0, 0, (type_t(*)())0,0,(type_t(*)())0,0},
#endif
#ifdef ENABLE_SUDI_2099_CLI
    {"Toggle SUDI 2099 Flag (Discrete-ACT2)",      (PFT)tam_toggle_sudi_2099, 0, 0,  
                                   (type_t(*)())aikido_hide_toggle_2099_item,0,(type_t(*)())0,0},
#endif
#ifdef SHA1_SHA2_SWSUDI2099
    {"Install SHA1/SHA2/SWSUDI",   (PFT)tam_act2_install_sha1_sha2_swsudi, 0, 0, (type_t(*)())0,0,(type_t(*)())0,0},
    {"Display SHA1/SHA2/SWSUDI",   (PFT)tam_act2_display_sha1_sha2_swsudi, 0, 0, (type_t(*)())0,0,(type_t(*)())0,0},
#endif
    {"Dump AIKIDO chip info",      (PFT)dump_aikido_chip_info,        0, 0, (type_t(*)())is_tam_aikido_on_wrapper,0,(type_t(*)())0,0},
    {"Dump AIKIDO FPGA version",   (PFT)dump_aikido_fpga_version,     0, 0, (type_t(*)())is_tam_aikido_on_wrapper,0,(type_t(*)())0,0},
    {"Dump AIKIDO ESPI register",  (PFT)aikido_espi_read_util,        0, 0, (type_t(*)())is_tam_aikido_on_wrapper,0,(type_t(*)())0,0},
#ifdef AIKIDO_SUPPORT_AIK
    {"Toggle AIKIDO AIK Flag",     (PFT)toggle_aikido_aik_flag,        0, 0, (type_t(*)())is_tam_aikido_on_wrapper,0,(type_t(*)())0,0},
#endif
#ifdef AIKIDO_WRITE_UTIL
    {"Write AIKIDO ESPI header register",  (PFT)aikido_espi_write_header_util,        0, 0, (type_t(*)())0,0,(type_t(*)())0,0},
#endif
    {"Aikido RNG Test",           (PFT)tam_act2_rng_test,           0, MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT, 
     (type_t(*)())is_tam_aikido_on_wrapper, 0, (type_t(*)())0, 0},
};

#define ACT2_SUBMENU_TABLE_SIZE (sizeof(tam_act2_submenu_table) / \
                                 sizeof(submenu_xtable_t))

/* 
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t act2_primary_items[ACT2_SUBMENU_TABLE_SIZE +
                                  MAX_BASE_ITEMS];
static mitem_t act2_secondary_items[ACT2_SUBMENU_TABLE_SIZE +
                                    MAX_BASE_ITEMS];

menuinfo_t tam_act2_subtest_menu = {
    "%s utility Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    act2_primary_items,
};
static menuinfo_t *act2_submenup = &tam_act2_subtest_menu;

/*-------------------------------------------------------------------
 *
 * Function : is_tam_aikido_mbox_on
 * Description: Return TRUE if mailbox flag is turned on for Aikido
 *              This function returns FALSE by default, and if ACT2
 *              device is Aikido, declare this function in platform
 *              code
 * INPUT:  dummy -- not used.
 * OUTPUT: return TRUE or FALSE
 * -------------------------------------------------------------------
 */
int is_tam_aikido_mbox_on (void)
    __attribute__((weak, alias("__is_tam_aikido_mbox_on")));
int __is_tam_aikido_mbox_on (void)
{
    return (FALSE);
}
/*-------------------------------------------------------------------
 *
 * Function : is_tam_aikido_on
 * Description: Return TRUE if ACT2 chip is Aikido
 *              This function returns FALSE by default, and if ACT2
 *              device is Aikido, declare this function in platform
 *              code
 * INPUT:  dummy -- not used.
 * OUTPUT: return TRUE or FALSE
 * -------------------------------------------------------------------
 */
int is_tam_aikido_on (void)
    __attribute__((weak, alias("__is_tam_aikido_on")));
int __is_tam_aikido_on (void)
{
    return (FALSE);
}

/*-------------------------------------------------------------------
 *
 * Function : tam_lib_device_open_mailbox
 * Description: This function is declared here for solely compilation
 *              purpose.
 *              This function implementation shall be declared in 
 *              platform code if Aikido is used
 * INPUT:  dummy -- not used.
 * OUTPUT: return TRUE or FALSE
 * -------------------------------------------------------------------
 */
tam_lib_status_t
tam_lib_device_open_mailbox (void *platform_opaque_handle,
                             uint8_t use_interrupt,
                             uint16_t mbx_msg_size,
                             uint32_t mbx_reg_base_addr,
                             void **tam_handle)
    __attribute__((weak, alias("__tam_lib_device_open_mailbox")));
tam_lib_status_t
__tam_lib_device_open_mailbox (void *platform_opaque_handle,
                             uint8_t use_interrupt,
                             uint16_t mbx_msg_size,
                             uint32_t mbx_reg_base_addr,
                             void **tam_handle)
{
    return (FALSE);
}
/**********************************************************************
 *
 * Function: menu_display
 *
 * Description: Hidden the menu item.
 *
 * Input : NONE
 *                     
 * Output: FALSE
 *
 **********************************************************************
 */
boolean menu_display(void) {
    /* Authenticate menu item should not be displayed,
     * so always return FALSE  
     */
    return(FALSE);        
}

/**********************************************************************
 *
 * Function: aikido_hide_toggle_2099_item
 * Description: aikido already program to good place. 
 *              hide this menu item that aikido does not need to prog
 *              2099. 
 * Input : NONE
 * Output: FALSE
 *
 **********************************************************************
 */
boolean aikido_hide_toggle_2099_item (void) {

    if (is_tam_aikido_on()) { 
        return(FALSE);        
    } else {
        return(TRUE);         
    } 
}

/*-------------------------------------------------------------------
 *
 * Function : getline_act2
 * Description: wrapper function to read a line of string from terminal
 * INPUT:  size -- lenght of string to read
 * OUTPUT: buf -- string read from terminal
 *         return PASSED or FAILED
 * -------------------------------------------------------------------
 */
int
getline_act2 (char *buf, int size)
{
    int xx;
    FILE *file;
    char cmd[32];

    /* (getpagesize() = 4096 ; the intel x86 memory page size */
    if (size >= (getpagesize()-1)) {
        printf("read size is %d; must be less than %d \n", size, getpagesize()-1);
        assert(!"*******\n\nrquest read size is too big!!!****\n\n");
    }
        
    /* CSCvf15334: Suppress printk message as we don't want fgets gets interrupted
     *             and results in UART buffer overflow
     */
    if (default_printk_level == -1) {
        file = fopen(SYS_PROC_PRINTK_FILE, "rb");
        if (file == NULL) {
            printf("%s; Warning! %s is not found\n", __func__, SYS_PROC_PRINTK_FILE);
            fflush(stdout);
        } else {
            fscanf(file, "%d", &default_printk_level);
            fclose(file);
        }
    }

    if (default_printk_level != -1) {
        sprintf(cmd, "%s %d", SYS_CHANGE_PRINTK_LEVEL, SYS_SUPPRESS_PRINTK_LEVEL);
        system(cmd);
    }
        
    fgets(buf, size, stdin);
    xx = strlen(buf);
    if ((xx && (buf[xx-1] == '\n')) || (xx && (buf[xx-1] == '\r')) ) {
        buf[xx-1] = '\0';
        xx--;
        if (act2_i2c_debug) {
            printf("\nDBG : Get MFG [Enter] \n");
            fflush(stdout);
        }    
    }
    
    /* Restore printk */
    if (default_printk_level != -1) {
        sprintf(cmd, "%s %d", SYS_CHANGE_PRINTK_LEVEL, default_printk_level);
        system(cmd);
    }
    
    return xx;
}

/*-------------------------------------------------------------------
 *
 * Function : get_sm_dc_i2c_quack
 * Description: Return i2c quack pointer if defined
 *              This function returns NULL by default, and if
 *              needed, declare this function in platform
 *              code
 * INPUT:  dummy -- not used.
 * OUTPUT: return NULL
 * -------------------------------------------------------------------
 */
void* get_sm_dc_wic_i2c_quack (int slot)
    __attribute__((weak, alias("__get_sm_dc_wic_i2c_quack")));
void* __get_sm_dc_wic_i2c_quack(int slot)
{
    return NULL;
}

/*-------------------------------------------------------------------
 *
 * Function : act2_get_n2g_i2c_if 
 * Description: get i2c struct
 * INPUT:  
 * OUTPUT: pointer to n2g_i2c_if_t 
 * -------------------------------------------------------------------
 */
void *
tam_act2_get_n2g_i2c_if (void)
{
    n2g_i2c_if_t *i2c;

    assert(cont.dev_if_p);
    if (cont.type == SM_DC_WIC_CARD || cont.type == SM_DC_WIC_DC_VM_CARD)
        i2c = (n2g_i2c_if_t *)get_sm_dc_wic_i2c_quack(cont.slot);
    else
        /*i2c_dev is parm2; i2c_ctrl is i2c controller (parm4) */
        i2c = (n2g_i2c_if_t *)platform_i2c_get_quack(cont.dev_if_p->parm2,
                                                 cont.dev_if_p->parm4);

    return (void*)i2c;
}

/*-------------------------------------------------------------------
 *
 * Function : act2_init_cont
 * Description: initialize sc_context structure
 *             called by smart_cookie.c to initalize context 
 * INPUT:  con - pointer to sc_context structure
 * OUTPUT: NONE
 * -------------------------------------------------------------------
 */
void
act2_init_cont (void *con)
{

    cont.dev_if_p = &dev_if;
    memcpy(&cont, con, sizeof(sc_context));
    /* need to set devicet type */
    device_type = cont.type;
}
/*
 * Function: tam_act2_prog
 *
 * Description : Ask user if they want to program ACT-2 TAM chip on motherboard or internal
 * module.
 * Then build the primary & secondary submenus for the ACT2
 * diags based on the _xtable_ tam_act2_submenu_table.  If the given
 * arg is TRUE, execute all the tests in the menu flagged with
 * MF_DOALL, and return the result.  Otherwise, present the menu to the
 * user for interaction.
 * INPUT :act2_items_executed flag ; not used
 * OUTPUT: PASSED
 */

static int 
tam_act2_prog (boolean act2_items_executed)
{
    char err[80];
    int ret_val;
    uint8_t mode;    
    int dummy = 0;
    
    tam_act2_reset(0);

    if (get_cookie_id(cont.slot, cont.type, cookie, &id, err)==FAILED) {
        cterr('f', 0, "act2 program -- unable to read cookie id.");
        return FAILED;
    }
    
    printf("\nControl Type            : [0x%x]\n", id);
    memset(prodName, '\0', sizeof(prodName));
    get_pid(cookie, prodName);
    get_pid(cookie, prodName_debug);
    printf("ProdName/Id             : [%s]\n", prodName); 
    memset(pcbSN, '\0', sizeof(pcbSN));
    get_pcb_serial(cookie, pcbSN);
    printf("PCB Serial Num          : [%s]\n", pcbSN);
    if (cont.type == MOTHER_BOARD) { /* If this is motherboard */
	memset(chassisSN, '\0', sizeof(chassisSN));
	get_tlv_serial(cookie, chassisSN, CHASSIS_SERIAL_NUM);
    
	printf("Chassis Serial Num      : [%s]\n", chassisSN); 
    }
    /* By default, using PCB serial number */
    memcpy(prodSN, pcbSN, sizeof(pcbSN));
    act2_simple_mode[device_type] = 0;

    build_primary_submenu(tam_act2_submenu_table, ACT2_SUBMENU_TABLE_SIZE,
                          "ACT2", &act2_submenup);
    build_secondary_submenu(tam_act2_submenu_table,
			    ACT2_SUBMENU_TABLE_SIZE,
			    act2_secondary_items);

    if (act2_items_executed) {
        printf("\n Only submenu support at this time");
    } else {
        if (is_tam_aikido_mbox_on() == TRUE) {
            ret_val = tam_lib_device_open_mailbox((void *)&cont, MBX_USE_INTERRUPT,
                                                  MBX_MSG_SIZE, MBX_REG_BASE_ADDR,
    		                                      &tam_handle);
    	    if (ret_val != TAM_RC_OK) {
                printf("%s: ERROR: Can't initialize Mailbox. Status: %#x\n",
                        __func__, ret_val);
	            return (FAILED);
    	    }
        } else {
            ret_val = tam_lib_device_open((void *)&cont, 259, &tam_handle);
            if (ret_val != TAM_RC_OK) {
                printf("%s: ERROR: Can't open handler. Status: %#x\n", 
                        __func__, ret_val);
                return (FAILED);
            }
        }
        mode = tam_lib_check_mode(tam_handle);
        if (mode != BUS_MODE_SIMPLE) {
            act2_simple_mode[device_type] = 0;
            printf("\n TAM Chip Setup In Legacy Mode, device_type = %x\n",device_type);
        } else {
            act2_simple_mode[device_type] = 1;
            printf("\n TAM Chip Setup In Simple Mode, device_type = %x\n",device_type);
        }
        tam_lib_display_scc_id(dummy);
        
#ifdef AIKIDO_SUPPORT_AIK
        /* setup aikido aik flag for enable/disable AIK program */
        tam_setup_aikido_aik_flag(&aikido_aik_flag); 

#endif
#ifdef SUDI_2099_2029
        /* AIKIDO already program to 2099, no need to check */
        if (is_tam_aikido_on()) {
            sudi_2099 = FALSE; 
        }
#endif
        menu(&tam_act2_subtest_menu, act2_secondary_items, '\0');
        tam_lib_device_close(&tam_handle);
    }
    printf("\nResetting ACT2\n");
    tam_act2_reset(0);
    return PASSED;
}

/*
 * Function: act2_prog
 *
 * Description : Ask user if they want to program ACT-2 chip on motherboard or internal
 * module.
 * Then build the primary & secondary submenus for the ACT2
 * diags based on the _xtable_ act2_submenu_table.  If the given
 * arg is TRUE, execute all the tests in the menu flagged with
 * MF_DOALL, and return the result.  Otherwise, present the menu to the
 * user for interaction.
 * INPUT :act2_items_executed flag ; not used
 * OUTPUT: PASSED
 */
int 
act2_prog (boolean act2_items_executed)
{
    return tam_act2_prog(act2_items_executed);
}

/*
 * Function: tam_act2_switch_simple_mode
 *
 * This function switches the chip to simple mode. In this mode the
 * chip will accept raw data formats, unlike the packets formats written
 * for ACT1/QUACK. Don't check return status because if chip is already
 * in simple mode, ACT2 library will return error code, but it really isn't
 * erro in this case. so don't check error code.
 *
 * Inputs: None
 *
 * Output: PASSED/FAILED
 */
int
tam_act2_switch_simple_mode (int dummy)
{
    tam_lib_status_t status;
    uint8_t mode;    

    mode = tam_lib_check_mode(tam_handle);
    if (mode != BUS_MODE_SIMPLE) {
        status = tam_lib_set_simple(tam_handle);
        if (status != TAM_RC_OK) {
            printf("\n ERROR: Cannot set to simple mode. status 0x%x\n", status);
            return (FAILED);
        }
        mode = tam_lib_check_mode(tam_handle);
        if (mode != BUS_MODE_SIMPLE) {
            printf("\n ERROR: Cannot set to simple mode\n");
        } else {
            printf("\nTAM Chip Setup In Simple Mode\n");
        }
    } else {
        printf("\nTAM Chip Already In Simple Mode\n");
    }
    act2_simple_mode[device_type] = 1;

    return(PASSED);
}

/*
 * Function: act2_is_simple_mode
 *
 * Description: software returing mode of the chip.
 * This is software state machine so might not be in sync with actual mode
 * in hardware.
 * Use caution when using the api to check the mode.
 *
 * Inputs: c -- pointer to sc_context
 *
 * Output: TRUE if in simple mode
 */
int
act2_is_simple_mode (void *c)
{
    sc_context *con = (sc_context *)c;
    
    assert(con->type < MAX_DEVICE_TYPE);
    
    return act2_simple_mode[con->type];
}

/*
 * Function: tam_act2_get_serial_num
 *
 * This function will read the serial number burned into the Ruby ACT2 device
 * at the time the device was manufactured. This is NOT the serial number defined
 * in the PCAMAP.
 *
 * Inputs: None
 *
 * Output: PASSED/FAILED
 */
int
tam_act2_get_serial_num (void)
{
    uchar chip_serial_number[32];
    int ix;
    tam_lib_status_t status;


    memset(chip_serial_number, 0xA5, sizeof(chip_serial_number));

    if (act2_chip_version < 0x15) {
        printf("\n *** ERROR: Cannot perform this function on an device.");
        printf("\n SCC Version on chip is %x.\n",act2_chip_version);
        return(FAILED);
    }

    status = tam_lib_get_chip_serial_number(tam_handle, chip_serial_number);
    if (status != TAM_RC_OK) {
        cterr('f',0," Cannot read serial number.");
        printf("\n tam_lib_get_chip_serial_number returned with status 0x%x\n", status);
        return (FAILED);
    }    

    printf("\n ACT-2: serial number =");
    for (ix = 0; ix < 32; ix++) {
        printf("%02x", chip_serial_number[ix]);
    }

    return(PASSED);
}

/*
 * Function: tam_act2_get_session_id
 *
 * This function will display the session id of the ACT2 chip. This value is zero
 * if the mfg login has not been run since last power cycle. The session id is 
 * required for CLIIP installation, SUDI installation and master key generation.
 *
 * Inputs: None
 *
 * Output: PASSED/FAILED
 */
int
tam_act2_get_session_id (void)
{
    printf("\n session id = %#x\n", session_id);
    return(PASSED);
}

/*
 * Function: tam_act2_get_cskmp (DEBUG ONLY)
 *
 * This function is used to get the CSKMP from the ACT2 device.
 * Used during bring-up only, to test I2C read/write functions. Not to 
 * be used for production units.
 *
 * Inputs: None
 *
 * Output: PASSED/FAILED
 */
int
tam_act2_get_cskmp (void)
{
    printf("NOT supported by ACT2 TAM lib\n");
    return(PASSED);
}

/*
 * Function: tam_act2_generate_ecskmp
 *
 * This function is used to get the ECSKMP from the ACT2 device.
 * Used during AIKIDO FPGA bring-up only.
 * Generate ECSKMP, read ECSKMP and save ECSKMP in a file.
 *
 * Inputs: None
 *
 * Output: PASSED/FAILED
 */
int
tam_act2_generate_ecskmp (void)
{
    tam_lib_status_t status;
    uchar repeat;
    uchar *buffer;
    uint16_t buffer_length;
    uint16_t ix, jx;
    char filename[32];
    FILE *fp;
    uchar chip_serial_number[32];

    memset(chip_serial_number, 0xA5, sizeof(chip_serial_number));

    if (act2_chip_version < 0x15) {
        printf("\n *** ERROR: Cannot perform this function on an device.");
        printf("\n SCC Version on chip is %x.\n",act2_chip_version);
        return(FAILED);
    }
    /* Read the serial number */
    status = tam_lib_get_chip_serial_number(tam_handle, chip_serial_number);
    if (status != TAM_RC_OK) {
        cterr('f',0," Cannot read serial number.");
        printf("\n tam_lib_get_chip_serial_number returned with status 0x%x\n", status);
        return (FAILED);
    }
    printf("\nACT-2: serial number =");
    for (ix = 0; ix < 32; ix++) {
        printf("%02x", chip_serial_number[ix]);
    }

   /*generate ECSKMP */
    status = tam_lib_mfg_ecskmp_generate(tam_handle, &repeat);
    if (status == TAM_LIB_ERR_PERMISSION_NOT) {
        printf("\n%s-%u tam_lib_mfg_ecskmp_generate "
               " already cliiped status=0x%0x-%s",
               __FUNCTION__, __LINE__, status, tam_lib_rc2string(status));
        cterr('f',0," Cannot generate ECSKMP");
        tam_mfg_errinfo(0);
        return (FAILED);
   } else if (status != TAM_RC_OK) {
        printf("\n%s-%u ERROR:tam_lib_mfg_ecskmp_generate "
               " status=0x%0x-%s", __FUNCTION__, __LINE__, status,
               tam_lib_rc2string(status));
        cterr('f',0," Cannot generate ecskmp");
        tam_mfg_errinfo(0);
        return (FAILED);
    } else {
        printf("\nECSKMP generated. repeat=%d\n", repeat);
    }
    buffer = malloc(800);
    /*read ECSKMP */
    status = tam_lib_mfg_ecskmp_read(tam_handle, buffer, &buffer_length);
    if (status != TAM_RC_OK) {
        cterr('f',0," Cannot read ECSKMP");
        tam_mfg_errinfo(0);
        return (FAILED);
    }
    /* Add Serial Number In Filename */
    for (ix = 0, jx = 0; jx < 32; jx++) {
        ix += sprintf(&filename[ix], "%02x", chip_serial_number[jx]);
    }
    sprintf(&filename[ix], ".bin");
    fp = fopen(filename, "wb+");
    fwrite((void *) buffer, 1, (32+256+256+256), fp);
    fclose(fp);
    printf("\nECSKMP Key:\n");
    for (ix = 0; ix < (32+256+256+256); ix++) {
        printf("%02x", *buffer++);
    }
    printf("\n\n");
    printf("Notes: Copy ECSKMP *.bin files to USB\n");
    printf("1. mount USB \n");
    printf("2. Copy *.bin files to USB\n");
    printf("3. umount USB\n");

    return(PASSED);

}
/*
 * Function: tam_act2_mfg_login
 *
 * This function is called to perform an manufacturing login to the ACT-2 chip.
 * This will generate an session id that should be used for CLIIP/SUDI and master
 * key generation. The act2l* functions called here are defined in the
 * ../common/chips/libact2/lib/lab_act2.a. 
 * Diags creates an session id and logs into the Ruby chip as a mfg user.
 *
 * Inputs: None
 *
 * Output: PASSED/FAILED
 */
int
tam_act2_mfg_login (void)
{
    int ret_val, cnt; 
    uchar signature[513];
    uchar ruby_signature[256];

    uint ix, xx, bytes;
    uint16_t len_nonce_cert_chain;
    tam_lib_status_t status;
    
    uchar *nonce_cert_chain_ptr;
    uchar *ruby_nonce_cert_chain;
    uchar nonce_num[32];
    
    memset(signature, '0', sizeof(signature));
    memset(ruby_signature, '0', sizeof(ruby_signature));

    printf("\n ACT-2 Manufacturing Login %p.", (void *)&cont);

    if (act2_chip_version < 0x15) {
        printf("\n *** ERROR: Cannot perform this function on a device.");
        printf("\n SCC Version on chip is %x.\n",act2_chip_version);
        return(FAILED);
    }

    printf("\n Switching to simple mode... ");
    ret_val = tam_act2_switch_simple_mode(0);
    
    if (ret_val) {
        printf("Cannot switch device to simple mode. may already be in simple mode.");
    }

    printf("\n Reading serial number... ");
    ret_val = tam_act2_get_serial_num();

    if (ret_val) {
        cterr('f',0," Cannot read serial number.");
        return(FAILED);
    }

    printf("\n STEP1: Initialize manufacturing login. ");
    len_nonce_cert_chain = 0;
    do {
        len_nonce_cert_chain = getdec_answer("\n Enter Nonce + cert chain length >",0,100,65535);
    } while (!len_nonce_cert_chain);


    /* Check to make sure nonce_cert_chain_len is greater than 100 bytes */
    if (len_nonce_cert_chain < 100) {
        cterr('f',0," Error Nonce + cert chain should be greater than 100");
        return(FAILED);
    } 

    status = tam_lib_mfg_login_init(tam_handle, 
					len_nonce_cert_chain, nonce_num);
    if (status != TAM_RC_OK) {
        cterr('f',0," Cannot read Nonce number from   (Ruby) device");
        printf("\n tam_lib_mfg_login_init returned with status 0x%x\n", status);
        tam_mfg_errinfo(0);
        return (FAILED);
    }

    /* Required, do not remove printing */
    printf("\n Nonce Number is:");
    for (ix = 0; ix < sizeof(nonce_num); ix++) {
        printf("%02x", nonce_num[ix]);
    }    

    nonce_cert_chain_ptr = (uchar *)malloc((len_nonce_cert_chain*2)+2);    
    ruby_nonce_cert_chain = (uchar *)malloc((len_nonce_cert_chain*2)+2);

    memset(nonce_cert_chain_ptr, '0', (len_nonce_cert_chain*2) + 2);
    memset(ruby_nonce_cert_chain, '0', ( len_nonce_cert_chain*2) + 2);

    for (cnt=0, xx=0, bytes=0; cnt<2; cnt++) {
        printf("\n Read (nonce + cert chain) > ");
        xx = getline_act2((char *)&nonce_cert_chain_ptr[bytes], getpagesize()-2);
        bytes +=xx;
        printf("..count %d; total bytes entered so far is %d\n",  cnt, bytes);fflush(stdout);
    }

    for (ix = 0; ix < (len_nonce_cert_chain*2+1); ix++) {
        nonce_cert_chain_ptr[ix] = atoh(nonce_cert_chain_ptr[ix]);
    }

    for (ix = 0; ix < (len_nonce_cert_chain); ix++) {
        ruby_nonce_cert_chain[ix] = ((nonce_cert_chain_ptr[2*ix] << 4) & 0xf0) | 
            ((nonce_cert_chain_ptr[2*ix + 1]) & 0x0f);
    }

    fflush(stdout);
    
    printf("\ng STEP 2: Finalize manufacturing login. ");

    status = tam_lib_mfg_login_credentials(tam_handle,
					    len_nonce_cert_chain,
					    ruby_nonce_cert_chain);
    if (status != TAM_RC_OK) {
        cterr('f',0," Cannot write to device");
        printf("\n tam_lib_mfg_login_credentials returned with status 0x%x\n", status);
        tam_mfg_errinfo(0);
        return (FAILED);
    }
    printf("tam_lib_mfg_login_credentials successfully\n");

    /* OBTAIN THE SIGNATURE AND ITS LENGTH HERE */

    printf("\n Read signature > ");fflush(stdout);
    getline_act2((char* )signature, sizeof(signature));    //    fread(signature, sizeof(signature), 1, stdin); 

    for (ix = 0; ix < (256*2); ix++) {
        signature[ix] = atoh(signature[ix]);
    }

    for (ix = 0; ix < (256); ix++) {
        ruby_signature[ix] = ((signature[2*ix] << 4) & 0xf0) | 
                             ((signature[2*ix + 1]) & 0x0f);
    }
    if (act2_i2c_debug) {
        for (ix = 0; ix < (256); ix++) {
            printf("%02x", ruby_signature[ix]);
        }
    }
    status = tam_lib_mfg_login_signature(tam_handle,
					    sizeof(ruby_signature),
					    ruby_signature,
					    &session_id);
    if (status != TAM_RC_OK) {
        cterr('f',0," Cannot write to device");
        printf("\n tam_lib_mfg_login_signature returned with status 0x%x\n", status);
        tam_mfg_errinfo(0);
        return (FAILED);
    }
				
    free(nonce_cert_chain_ptr);
    free(ruby_nonce_cert_chain);
    printf("\n ACT-2 Manufacturing Login Successful. session_id %x\n", session_id);
    return(PASSED);
}

/*
 * Function: tam_act2_install_cliip
 *
 * This function is called by autotest to program in the CLIIP certification
 * info into ACT2. The act2l* functions called here are defined in the
 * ../common/chips/libact2/lib/lab_act2.a. 
 * Diags should use the session id created during manufacturing login.
 *
 * Inputs: None
 *
 * Output: PASSED/FAILED
 */
int
tam_act2_install_CLIIP (void)
{
    uint ix, cnt, xx, bytes ;
    uchar *cliip_data_ptr;
    uchar *ruby_cliip_data;
    uint16_t len_cliip_data, len_cliip_data_swp, cliip_text_size;
    uint16_t max_line_size;
    tam_lib_status_t status;
    printf("\n ACT-2 CLIIP Installation.");

    if (session_id == 0) {
        printf("\n The Session id has not been generated to allow this.");
        printf("\n Please run the manufacturing login option first.");
        return(PASSED);
    }

    if (act2_chip_version < 0x15) {
        printf("\n *** ERROR: Cannot perform this function on device.");
        printf("\n SCC Version on chip is %x.\n", act2_chip_version);
        return(FAILED);
    }
    len_cliip_data = 0;
    do {
        len_cliip_data = getdec_answer("\n Enter CLIIP data length >",0,1,65535);
    } while (!len_cliip_data);
    cliip_text_size = (len_cliip_data * 2) ;
    cliip_data_ptr = (uchar *)malloc(len_cliip_data*2+1);
    ruby_cliip_data = (uchar *)malloc(len_cliip_data);
    max_line_size = getpagesize()-2;    

    for (bytes = cnt = xx = 0; bytes < cliip_text_size ; bytes += xx) {
        printf("\n Read CLIIP data > ");
        fflush(stdout);
        xx = getline_act2((char *)&cliip_data_ptr[bytes], max_line_size);
        if (act2_i2c_debug) {
            printf("\nDBG : getline_act2 loop %d;size %d,total %d\n",cnt + 1, 
                     xx, bytes + xx );
            fflush(stdout);
        }    
    }
    /* Act2 team : The CLIIP is larger on 1.3a than 1.3. There should not be any
                   hardcoding of size checks for variable length data  */
#ifdef NO_NEED
    if (chip_vendor == 0x13) {
        printf("\nACT2 version is 1.3, check CLIIP length");
        if (bytes != (len_cliip_data*2)) {
            cterr('f', 0, "the cliip data entered is %d bytes; it has to be %d bytes long", bytes, len_cliip_data*2);
            return FAILED;
        }
    } else {
        printf("\nACT2 version is not 1.3, skip CLIIP length check");
    }        
#endif
    for (ix = 0; ix < (len_cliip_data*2); ix++) {
        cliip_data_ptr[ix] = atoh(cliip_data_ptr[ix]);
    }
    if (act2_i2c_debug) {
        printf("\nDBG : atoh(cliip_data_ptr[ix] %d\n", len_cliip_data*2);
        for (ix =0; ix < (len_cliip_data*2); ix++) {
            printf("%02X", cliip_data_ptr[ix]);
        }
        printf("\n[END]\n");
        fflush(stdout);
    }    

    for (ix = 0; ix < (len_cliip_data); ix++) {
        ruby_cliip_data[ix] = ((cliip_data_ptr[2*ix] << 4) & 0xf0) | 
                              ((cliip_data_ptr[2*ix + 1]) & 0x0f);
    }

    /*
     * An ret_val of 1 will indicate that the CLIIP has already been installed
     * for this chip, it's not a error and sudi or master key generation can 
     * be done with this ret_val. retry ZZZ.
     */
    len_cliip_data_swp =     len_cliip_data;

    if (act2_i2c_debug) {
        printf("\nDBG : Data before sent to tam_lib_mfg_cliip_install %d\n", len_cliip_data_swp);
        for (ix =0; ix < len_cliip_data_swp; ix++) {
            printf("%02X", ruby_cliip_data[ix]);
        }
        printf("\n[END]\n");
    }    

    status = tam_lib_mfg_cliip_install(tam_handle,
			session_id, len_cliip_data_swp, ruby_cliip_data);
    if (status != TAM_RC_OK) {
        if (status == TAM_LIB_RC_REPEAT) { 
            printf("\n CLIIP already installed.");
        } else {
            printf("\n tam_lib_mfg_cliip_install returned with status %d session_id 0x%x\n", 
            status, session_id);
            tam_mfg_errinfo(0);
            free(cliip_data_ptr);
            free(ruby_cliip_data);
            return (FAILED);
        }
    }
    free(cliip_data_ptr);
    free(ruby_cliip_data);
    printf("\n Identity Installation Successful.");
    return(PASSED);
}

/*
 * Function: tam_act2_install_SUDI
 *
 * This function is called by autotest to program in the SUDI certification 
 * info into ACT2. 
 * Diags must first run the manufacturing login before SUDI certification 
 * can be done.
 * Add support for both RSA/ECC !!!!!
 *
 * Inputs: None
 *
 * Output: PASSED/FAILED
 */
int
tam_act2_install_SUDI (void)
{
    uint ix;
    uchar *leaf_sudi_data_ptr;
    uchar *ruby_leaf_sudi_data;
    uchar *ca_root_sudi_data_ptr;
    uchar *ruby_ca_root_sudi_data;
    uint16_t len_leaf_sudi, len_ca_root_sudi;
    uint16_t len_leaf_sudi_swp, len_ca_root_sudi_swp;
    uint16_t cms_sudi_request_len;
    uint8_t *cms_sudi_request_ptr = NULL;
    uint8_t *sudi_ptr;
    sudi_info_t sgbu_specs;
    char sudi_key_type_ptr[10];
    tam_lib_status_t status;

    assert(cont.dev_if_p);

    /* default SN and Name */

    printf("type %d; slot%d\n", cont.type, cont.slot);
    tam_act2_switch_simple_mode(0);

    printf("prod SN is %s. \nLength of ProdSN = %d.\n",
           prodSN, (uint)strlen(prodSN));
    printf("prodName is %s. \nLength of prodName = %d.\n",
           prodName, (uint)strlen(prodName));

    printf("\n Serial Number = ");
    for (ix =0; ix < 11; ix++) {
        printf("%c", prodSN[ix]);
    }
    
    printf("\n PID = ");    
    for (ix =0; ix < sizeof(prodName); ix++) {
        printf("%c", prodName[ix]);
    }

    /* Display the info required for SUDI */
    printf("\n Common Name = ");
    for (ix = 0; ix < sizeof(prodName); ix++) { 
        printf("%c", prodName[ix]);
    }

    sgbu_specs.serial_number_ptr = prodSN;
    sgbu_specs.serial_number_length= strlen(prodSN);

    /* Set the common name. It's needed for mfg back */
    sgbu_specs.product_name_ptr = prodName;
    sgbu_specs.product_name_length = strlen(prodName);

    sgbu_specs.pid_ptr = prodName;  
    sgbu_specs.pid_length = strlen(prodName);

#ifdef SUDI_2099_2029
    /* Display the status of SUDI 2099 */
    PRINT_SUDI_2099_STS

    /* Display TAM Library Version */
    tam_show_library_version();

    /* Check if TAM library supports 2099 or not */
    if ((sudi_2099 == TRUE) && (is_tam_library_sudi_2099() == FALSE)) {
        printf("\n *** ERROR: THIS TAM LIBRARY DOESN'T SUPPORT SUDI 2099\n");
        printf("\n     PLEASE UPDATE YOUR TAM LIBRARY FOR SUDI 2099\n");
        return (FAILED);
    }

    /* SUDI_2099 */
    if (sudi_2099 == TRUE) {
        sprintf(sudi_key_type_ptr, "HARSA");
    } else {
        sprintf(sudi_key_type_ptr, "RSA");
    }
#else
    sprintf(sudi_key_type_ptr, "RSA");
#endif

    sgbu_specs.sudi_key_type_ptr = sudi_key_type_ptr;
    sgbu_specs.sudi_key_type_length = strlen(sudi_key_type_ptr);
    printf("\n Identity verifcation. ");

    if (session_id == 0) {
        printf("\n The Session id has not been generated to allow this.");
        printf("\n Please run the manufacturing login option first.");
        return(PASSED);
    }

    if (act2_chip_version < 0x15) {
        printf("\n *** ERROR: Cannot perform this function on device.");
        printf("\n SCC Version on chip is %x.\n",act2_chip_version);
        return(FAILED);
    }

    /* Generate CMS SUDI request and print it out */
    printf("\ngenerating identity verification CMS request..\n");
    status = tam_lib_mfg_create_sudi_request(tam_handle,
				      session_id,
				      &sgbu_specs,
				      &cms_sudi_request_ptr,
				      &cms_sudi_request_len);
    cms_sudi_request_len =  (cms_sudi_request_len);
    if (status != TAM_RC_OK) {
        cterr('f',0," Cannot create CMS sudi request");
        printf("\n tam_lib_mfg_create_sudi_request returned with status 0x%x\n", status);
        tam_mfg_errinfo(0);
        return (FAILED);
    }

    printf("\n ACT-2: SUDI cms request (length is %d) = ",cms_sudi_request_len);
    sudi_ptr = cms_sudi_request_ptr;
    /* Required, do not remove printing */
    for (ix = 0; ix < cms_sudi_request_len; ix++) {
        printf("%02x", (*sudi_ptr++));
    }
    /* SUDI Cert Chain installation */
    len_leaf_sudi = 0;
    do {
        len_leaf_sudi = getdec_answer("\n Enter SUDI cert length >",0,1,65535);
    } while (!len_leaf_sudi);

    leaf_sudi_data_ptr = (uchar *)malloc(len_leaf_sudi*2+1);
    ruby_leaf_sudi_data = (uchar *)malloc(len_leaf_sudi*2+1);
    printf("\n Reading Leaf SUDI certificate data > ");
    getline_act2((char *)leaf_sudi_data_ptr, (len_leaf_sudi*2+1));

    for (ix = 0; ix < (len_leaf_sudi*2); ix++) {
        leaf_sudi_data_ptr[ix] = atoh(leaf_sudi_data_ptr[ix]);
    }

    for (ix = 0; ix < (len_leaf_sudi); ix++) {
        ruby_leaf_sudi_data[ix] = ((leaf_sudi_data_ptr[2*ix] << 4) & 0xf0) | 
            ((leaf_sudi_data_ptr[2*ix + 1]) & 0x0f);
    }

    len_ca_root_sudi = 0;
    do {
        len_ca_root_sudi = getdec_answer("\n Enter (CA and ROOT) length >",0,1,65535);
    } while (!len_ca_root_sudi);
    
    ca_root_sudi_data_ptr = (uchar *)malloc((len_ca_root_sudi*2+1));
    ruby_ca_root_sudi_data = (uchar *)malloc((len_ca_root_sudi+1));
    printf("\n Reading CA and ROOT SUDI data > ");
    getline_act2((char *)ca_root_sudi_data_ptr, (len_ca_root_sudi*2+1));

    for (ix = 0; ix < (len_ca_root_sudi*2); ix++) {
        ca_root_sudi_data_ptr[ix] = atoh(ca_root_sudi_data_ptr[ix]);
    }

    for (ix = 0; ix < (len_ca_root_sudi); ix++) {
        ruby_ca_root_sudi_data[ix] = ((ca_root_sudi_data_ptr[2*ix] << 4) & 0xf0) | 
            ((ca_root_sudi_data_ptr[2*ix + 1]) & 0x0f);
    }
    len_leaf_sudi_swp =  (len_leaf_sudi); 
    len_ca_root_sudi_swp =  (len_ca_root_sudi);

#ifdef SUDI_2099_2029
    if (sudi_2099 == TRUE) {
        /* SUDI_2099: Install 2099 HARSA SUDI leaf object */
        status = tam_lib_mfg_install_cert_and_chain(tam_handle, session_id,
                                                    len_ca_root_sudi_swp,
                                                    ruby_ca_root_sudi_data,
                                                    len_leaf_sudi_swp,
                                                    ruby_leaf_sudi_data,
                                                    HARSA_SUDI);

        if (status != TAM_RC_OK) {
            cterr('f', 0, "HARSA SUDI installation Failed (tam_lib_mfg_install_cert_and_chain)");
            return (FAILED);
        }
    } else { /* SUDI 2029 */
#endif
        /* install SUDI leaf object */
        status = tam_lib_object_write(tam_handle, 
                                      session_id,
                                      TAM_LIB_IDEVID_RSA_CERT,
                                      ruby_leaf_sudi_data,
                                      len_leaf_sudi_swp);
        if (is_tam_aikido_on()) {
            if (status != TAM_RC_OK) {
                printf("\nHA-root RESUDI first time TAM_LIB_IDEVID_RSA_CERT return status "
                       "0x%x\n", status);
                if (status != TAM_LIB_ERR_VERIFY_FAIL) {
                    cterr('f',0," Cannot write SUDI leaf data to ACT-2 device");
                    printf("\n tam_lib_object_write returned with status 0x%x\n", status);
                    return (FAILED);
                }
            }
        } else {
            if (status != TAM_RC_OK) {
                cterr('f',0," Cannot write SUDI leaf data to ACT-2 device");
                printf("\n tam_lib_object_write returned with status 0x%x\n", status);
                return (FAILED);
            }
        }
        /* install SUDI CA root object */
        status = tam_lib_object_write(tam_handle, 
	    			    session_id,
	    			    TAM_LIB_IDEVID_RSA_CERT_CHAIN,
	    			    ruby_ca_root_sudi_data,
	    			    len_ca_root_sudi_swp);
        if (status != TAM_RC_OK) {
            cterr('f',0," Cannot write SUDI CA root data to ACT-2 device");
            printf("\n tam_lib_object_write returned with status 0x%x\n", status);
            return (FAILED);
        }
#ifdef SUDI_2099_2029
    }
#endif

    if (is_tam_aikido_on()) {
        /* HA-root RESUDI install SUDI leaf object */
        status = tam_lib_object_write(tam_handle, 
    				    session_id,
    				    TAM_LIB_IDEVID_RSA_CERT,
    				    ruby_leaf_sudi_data,
    				    len_leaf_sudi_swp);
        if (status != TAM_RC_OK) {
            cterr('f',0," Cannot write SUDI leaf data to ACT-2 device");
            printf("\n tam_lib_object_write returned with status 0x%x\n", status);
            return (FAILED);
        }
    }

#ifdef SUPPORT_ECC
    /* V1.5 should install RSA and ECC */
    if (act2_chip_version >= TAM_ACT2_V1_5_FW) {
        /* install SUDI leaf object */
        status = tam_lib_object_write(tam_handle, 
					    session_id,
					    TAM_LIB_IDEVID_ECC_CERT,
					    ruby_leaf_sudi_data,
					    len_leaf_sudi_swp);
        if (status != TAM_RC_OK) {
            cterr('f',0," Cannot write ECC SUDI leaf data to ACT-2 device");
            printf("\n tam_lib_object_write returned with status 0x%x\n", status);
            return (FAILED);
        }
        /* install SUDI ECC root object */
        status = tam_lib_object_write(tam_handle, 
					    session_id,
					    TAM_LIB_IDEVID_ECC_CERT_CHAIN,
					    ruby_ca_root_sudi_data,
					    len_ca_root_sudi_swp);
        if (status != TAM_RC_OK) {
            cterr('f',0," Cannot write ECC SUDI CA root data to ACT-2 device");
            printf("\n tam_lib_object_write returned with status 0x%x\n", status);
            return (FAILED);
        }
    }
#endif

#ifdef AIKIDO_SUPPORT_AIK
    if (aikido_aik_flag) {

        printf("\n AIK installation. ");
        sprintf(sudi_key_type_ptr, "AIK");
        sgbu_specs.sudi_key_type_ptr = sudi_key_type_ptr;
        sgbu_specs.sudi_key_type_length = strlen(sudi_key_type_ptr);
        cms_sudi_request_ptr = NULL;
        /* Generate AIK request and print it out */
        printf("\ngenerating AIK identity verification CMS request..\n");
        status = tam_lib_mfg_create_sudi_request(tam_handle,
                          session_id,
                          &sgbu_specs,
                          &cms_sudi_request_ptr,
                          &cms_sudi_request_len);
        cms_sudi_request_len =  (cms_sudi_request_len);
        if (status != TAM_RC_OK) {
            cterr('f',0," Cannot create CMS AIK request");
            printf("\n tam_lib_mfg_create_sudi_request returned with status 0x%x\n", status);
            tam_mfg_errinfo(0);
            return (FAILED);
        }

        printf("\n ACT-2: AIK cms request (length is %d) = ",cms_sudi_request_len);
        sudi_ptr = cms_sudi_request_ptr;
        /* Required, do not remove printing */
        for (ix = 0; ix < cms_sudi_request_len; ix++) {
            printf("%02x", (*sudi_ptr++));
        }
        /* SUDI Cert Chain installation */
        len_leaf_sudi = 0;
        do {
            len_leaf_sudi = getdec_answer("\n Enter AIK cert length >",0,1,65535);
        } while (!len_leaf_sudi);

        leaf_sudi_data_ptr = (uchar *)malloc(len_leaf_sudi*2+1);
        ruby_leaf_sudi_data = (uchar *)malloc(len_leaf_sudi*2+1);
        printf("\n Reading Leaf AIK certificate data > ");
        getline_act2((char *)leaf_sudi_data_ptr, (len_leaf_sudi*2+1));

        for (ix = 0; ix < (len_leaf_sudi*2); ix++) {
            leaf_sudi_data_ptr[ix] = atoh(leaf_sudi_data_ptr[ix]);
        }

        for (ix = 0; ix < (len_leaf_sudi); ix++) {
            ruby_leaf_sudi_data[ix] = ((leaf_sudi_data_ptr[2*ix] << 4) & 0xf0) |
                ((leaf_sudi_data_ptr[2*ix + 1]) & 0x0f);
        }

        len_ca_root_sudi = 0;
        do {
            len_ca_root_sudi = getdec_answer("\n Enter (CA and ROOT) length >",0,1,65535);
        } while (!len_ca_root_sudi);

        ca_root_sudi_data_ptr = (uchar *)malloc((len_ca_root_sudi*2+1));
        ruby_ca_root_sudi_data = (uchar *)malloc((len_ca_root_sudi+1));
        printf("\n Reading CA and ROOT AIK data > ");
        getline_act2((char *)ca_root_sudi_data_ptr, (len_ca_root_sudi*2+1));

        for (ix = 0; ix < (len_ca_root_sudi*2); ix++) {
            ca_root_sudi_data_ptr[ix] = atoh(ca_root_sudi_data_ptr[ix]);
        }

        for (ix = 0; ix < (len_ca_root_sudi); ix++) {
            ruby_ca_root_sudi_data[ix] = ((ca_root_sudi_data_ptr[2*ix] << 4) & 0xf0) |
                ((ca_root_sudi_data_ptr[2*ix + 1]) & 0x0f);
        }
        len_leaf_sudi_swp =  (len_leaf_sudi);
        len_ca_root_sudi_swp =  (len_ca_root_sudi);

        /* install SUDI leaf object */
        status = tam_lib_object_write(tam_handle,
                                      session_id,
                                      TAM_LIB_AIK_RSA_CERT,
                                      ruby_leaf_sudi_data,
                                      len_leaf_sudi_swp);
        if (status != TAM_RC_OK) {
            cterr('f',0," Cannot write AIK leaf data to ACT-2 device");
            printf("\n tam_lib_object_write returned with status 0x%x\n", status);
            free(ca_root_sudi_data_ptr);
            free(ruby_ca_root_sudi_data);
            return (FAILED);
        }
        /* install AIK CA root object */
        status = tam_lib_object_write(tam_handle,
                                      session_id,
                                      TAM_LIB_AIK_RSA_CERT_CHAIN,
                                      ruby_ca_root_sudi_data,
                                      len_ca_root_sudi_swp);
        if (status != TAM_RC_OK) {
            cterr('f',0," Cannot write AIK CA root data to ACT-2 device");
            printf("\n tam_lib_object_write returned with status 0x%x\n", status);
            free(ca_root_sudi_data_ptr);
            free(ruby_ca_root_sudi_data);
            return (FAILED);
        }
 
        printf("\n ACT-2 AIK Installation Successful.");
 
    }
#endif

    free(ca_root_sudi_data_ptr);
    free(ruby_ca_root_sudi_data);

#ifdef SUDI_2099_2029
    /* SUDI_2099 */
    if (sudi_2099 == TRUE) {
        printf("\n ACT-2 2099_SUDI Installation Successful.");
    } else {
        printf("\n ACT-2 SUDI Installation Successful.");
    }
#else
    printf("\n ACT-2 SUDI Installation Successful.");
#endif
    return (PASSED);
}

/*
 * Function: tam_act2_close_mfg_login
 *
 * This function will close disconnect the manufacturing login to the ACT-2 chip.
 *
 * Inputs: None
 *
 * Output: PASSED/FAILED
 */
int
tam_act2_close_mfg_login (void)
{
    tam_lib_status_t status;
    char func_name[32];
    
    printf("\n ACT-2 Close Manufacturing login.");

    if (session_id == 0) {
        printf("\n The Session id has not been generated to allow this.");
        printf("\n Please run the manufacturing login option first.");
        return(PASSED);
    }

#ifdef SUDI_2099_2029
    /* According to Chandana (chandapr), we need to invoke tam_lib_mfg_session_end()
     * for TAM library version newer than 3.1.1
     * tam_lib_session_end() function will be invoked if the tam library version is
     * older than 3.1.1, for backward compatibility
     */
    if (is_tam_library_sudi_2099() == TRUE) {
        strcpy(func_name, "tam_lib_mfg_session_end");
        status = tam_lib_mfg_session_end(tam_handle, session_id);
    } else {
        strcpy(func_name, "tam_lib_session_end");
        status = tam_lib_session_end(tam_handle, session_id);
    }
#else
    strcpy(func_name, "tam_lib_session_end");
    status = tam_lib_session_end(tam_handle, session_id);
#endif
    if (status != TAM_RC_OK) {
        cterr('f',0," Cannot end session 0x%x the ACT-2 device", session_id);
        printf("\n %s returned with status 0x%x\n", func_name, status);
        return (FAILED);
    }
    printf("\n ACT-2 Manufacturing Login Closed Successful.");
    return(PASSED);
}

/*-------------------------------------------------------------------
 *
 * Function : tam_act2_version
 * Description: send query to act2 chip to get version number
 * INPUT:  dummy -- not used
 * OUTPUT: version number of the chip
 * -------------------------------------------------------------------
 */
static int
tam_act2_version (int dummy)
{
    int version;
    
    /* willl send these bytes to quack
       01 01 00 00
       02 fe 00 00
       03 00 00 00
    */

    version = quack_version(&cont);
    printf("version is %#x\n", version);
    return version;
}

/*-------------------------------------------------------------------
 *
 * Function : act2_version
 * Description: send query to act2 chip to get version number
 * INPUT:  dummy -- not used
 * OUTPUT: version number of the chip
 * -------------------------------------------------------------------
 */
int
act2_version (int dummy)
{
    return tam_act2_version(dummy);
}

/*-------------------------------------------------------------------
 *
 * Function : tam_act2_test
 * Description: internal chip test
 * INPUT:  dummy -- not used
 * OUTPUT: PASSED or FAILED
 * -------------------------------------------------------------------
 */
static int
tam_act2_test (int dummy)
{
    printf("NOT supported by ACT2 TAM lib\n");
    return(PASSED);
}

/*
 * Function: tam_user_act2l_read_sudi
 *
 * This function will check to ensure that the CLIIP/SUDI has been installed.
 *
 * Inputs: None
 *
 * Output: PASSED/FAILED
 */
static int
tam_user_act2l_read_sudi (void)
{
    int ret_val;
    tam_lib_status_t status;
    uint8_t buffer[SUDI_MAX_SIZE];
    uint16_t length = sizeof(buffer);
    uint32_t cert_obj_id, cert_chain_obj_id, err_count = 0; 
    char sudi_name[32];

    if (session_id == 0) {
        printf("\n The Session id has not been generated to allow this.");
        printf("\n Please run the manufacturing login option first.");
        return(PASSED);
    }

    /* Ensure that the ACT2 device is in simple mode */
    printf("\n Switching ACT-2 device to simple mode... ");
    ret_val = tam_act2_switch_simple_mode(0);
    if (ret_val) {
        printf("Cannot switch device to simple mode. may already be in simple mode.");
    }

#ifdef SUDI_2099_2029
    /* Display the status of SUDI 2099 */
    PRINT_SUDI_2099_STS

    if (sudi_2099 == TRUE) {
        cert_obj_id = TAM_LIB_IDEVID_HARSA_CERT;
        cert_chain_obj_id = TAM_LIB_IDEVID_HARSA_CERT_CHAIN;
        sprintf(sudi_name, "2099_SUDI");
    } else {
        cert_obj_id = TAM_LIB_IDEVID_RSA_CERT;
        cert_chain_obj_id = TAM_LIB_IDEVID_RSA_CERT_CHAIN;
        sprintf(sudi_name, "SUDI");
    }
#else
    cert_obj_id = TAM_LIB_IDEVID_RSA_CERT;
    cert_chain_obj_id = TAM_LIB_IDEVID_RSA_CERT_CHAIN;
    sprintf(sudi_name, "SUDI");
#endif

    status = tam_lib_object_read(tam_handle,
                                 session_id,
                                 cert_obj_id,
                                 buffer, 
                                 &length);
    if (status != TAM_RC_OK) {
        cterr('f', 0, "Cannot get RSA leaf %s cert in session 0x%x the ACT-2 device", sudi_name, session_id);
        err_count++; 
    } else {
        printf("Successfully get RSA leaf %s cert with size of %d\n", sudi_name, length);
    }
    status = tam_lib_object_read(tam_handle,
                                 session_id,
                                 cert_chain_obj_id,
                                 buffer, 
                                 &length);
    if (status != TAM_RC_OK) {
        cterr('f', 0, "Cannot get RSA CA %s cert in session %#x the ACT-2 device", sudi_name, session_id);
        err_count++; 
    } else {
        printf("Successfully get RSA CA %s cert with size of %d\n", sudi_name, length);
    }
#ifdef SUPPORT_ECC
    /* V1.5 should install RSA and ECC */
    if (act2_chip_version >= TAM_ACT2_V1_5_FW) {
        status = tam_lib_object_read(tam_handle,
                                     session_id,
                                     TAM_LIB_IDEVID_ECC_CERT,
                                     buffer, 
                                     &length);
        if (status != TAM_RC_OK) {
            printf("Cannot get ECC leaf SUDI cert in session 0x%x the ACT-2 device", session_id);
            err_count++; 
        } else  {
            printf("Successfully get ECC leaf SUDI cert with size of %d\n", length);
        }
        status = tam_lib_object_read(tam_handle,
					 session_id,
					 TAM_LIB_IDEVID_ECC_CERT_CHAIN,
					 buffer, 
					 &length);
        if (status != TAM_RC_OK) {
            printf("Cannot get ECC CA SUDI cert in session 0x%x the ACT-2 device", session_id);
            err_count++; 
        } else {
            printf("Successfully get ECC CA SUDI cert with size of %d\n", length);
        }
    }
#endif

#ifdef AIKIDO_SUPPORT_AIK
    if (aikido_aik_flag) {

        /* check AIK */
        status = tam_lib_object_read(tam_handle,
                                     session_id,
                                     TAM_LIB_AIK_RSA_CERT,
                                     buffer, 
                                     &length); 
        if (status != TAM_RC_OK) {
            cterr('f', 0, "Cannot get AIK leaf SUDI cert in session 0x%x the ACT-2 device", session_id);
            err_count++; 
        } else  {
            printf("Successfully get AIK leaf SUDI cert with size of %d\n", length);
        }
        
        status = tam_lib_object_read(tam_handle,
                                     session_id,
                                     TAM_LIB_AIK_RSA_CERT_CHAIN,
                                     buffer,
                                     &length); 
        if (status != TAM_RC_OK) {
            cterr('f', 0, "Cannot get AIK CA SUDI cert in session 0x%x the ACT-2 device", session_id);
            err_count++; 
        } else {
            printf("Successfully get AIK CA SUDI cert with size of %d\n", length);
        }
    }

    /* once AIK is popular on following platforms, 
     * we refine the error mesage here.
     * Otherwise it is confusing people.  */
    if (err_count != 0) {
        cterr('f', 0, "CLIIP/SUDI install Failed");
        return (FAILED);
    }
#else
    if (status != TAM_RC_OK) {
        cterr('f', 0, "CLIIP/SUDI install Failed");
        return (FAILED);
    }
#endif

    printf("\n -------------------------");
    printf("\n CLIIP/SUDI install OK.");
    printf("\n -------------------------");
    return (PASSED);
}

/*
 * Function: rtrim
 *
 * Description : Strip the whitespace end of string
 *
 * Inputs: string
 *
 * Output: None
 */
boolean rtrim(char *s)
{
    int ix;

    ix = strlen(s) - 1;
    while((s[ix] == ' ') && ix >= 0){ix--;};
    if (diagflag_xram & D_DEBUG_OPTIONS) {
        s[ix+1] = '\0';
    }
    if (ix != (strlen(s) - 1)) {
        return FALSE;
    }
    return TRUE;
}
/*
 * Function: tam_act2_authenticate_udi
 *
 * Description : Send command to authenticate
 *
 * Inputs: a  - notused
 *
 * Output: PASSED/FAILED
 */
static int
tam_act2_authenticate_udi (int a)
{
    tam_lib_status_t status;
    uint8_t cert_type; 

    printf("&&& For correlation purpose only. Still need to be\n");    
    printf("&&& authenticated by IOS,\n");    
    printf("&&& since different ACT2's library type is used -\n");    
    printf("&&& IOS: 3rd party, Diags: Standalone\n");    
    if (getc_answer("&&& Continue [y/n]:", "yn", 'y') != 'y') {
        return (PASSED);
    }

    tam_act2_switch_simple_mode(0);

    printf("\nConrol Type : [0x%x]\n", id);
    printf("ProdName/Id : [%s]\n", prodName);    
    printf("Serial Num  : [%s]\n", prodSN);    

    /* DESCRIPTION :
      This function must be called by the platform to authenticate 
      the ACT2 chip and to validate the PID and Serial Number. The
      Serial Number can be skipped by passing NULL for this parameter.
      Cert_type is specified as RSA_SUDI, ECC_SUDI or DEFAULT_SUDI
      as defined in tam_library.h
      This function requires OpenSSL/CiscoSSL or RSA crypto tool kit
      to facilitate validation of the certificate chain, the signature
      verification and PID/SN retrieval for SUDI certificate. However
      standalone TAM Library doesn't need OpenSSL/CiscoSSL or RSA 
      crypto tool kit. A successful return provides confidence that
      the chip is authentic.
    */

#ifdef SUDI_2099_2029
    /* Display the status of SUDI 2099 */
    PRINT_SUDI_2099_STS

    /* SUDI_2099 */
    if (sudi_2099 == TRUE) {
        cert_type = HARSA_SUDI;
    } else {
        cert_type = RSA_SUDI;
    }
#else
    cert_type = RSA_SUDI;
#endif
    printf("&&& Calling tam_lib_authentication_udi\n");
    if (diagflag_xram & D_DEBUG_OPTIONS) {
        printf("&&& It's the debug mode, will strip off the whitespace \n");    
        if (getc_answer("&&& Continue [y/n]:", "yn", 'y') != 'y') {
            return (PASSED);
        }
        rtrim(prodName_debug);
        printf("Strip whitespace : ProdName/Id : [%s]\n", prodName_debug);    
        status = tam_lib_authentication_udi(tam_handle, prodName_debug, prodSN, cert_type);
    } else {   
        status = tam_lib_authentication_udi(tam_handle, prodName, prodSN, cert_type);
    }
    if (status != TAM_RC_OK) {
        printf("\nAuthentication failed with return status 0x%x ", status);
        if (status == TAM_LIB_ERR_UDI_MISMATCH) {
            printf(": The UDI MISMATCH");
        }
        if (status == TAM_LIB_ERR_UDI_INVALID) {
            printf(": The UDI INVALID");
        }
        if (rtrim(prodName) == FALSE) {
            printf("\n!!! Whitespace is not allowed after PID.");
        }
        printf("\n");
        return(FAILED);
    } else {
#ifdef SUDI_2099_2029
        if (sudi_2099 == TRUE) {
            printf("\n Authentication 2099-HARSA passed\n");
        } else {
#endif
            if (act2_chip_version >= TAM_ACT2_V1_5_FW) {
                printf("\n Authentication RSA passed\n");
            } else {
                printf("\n Authentication passed\n");
            }
#ifdef SUDI_2099_2029
        }
#endif
    }
#ifdef SUPPORT_ECC
    /* V1.5 should install RSA and ECC */
    if (act2_chip_version >= TAM_ACT2_V1_5_FW) {
        status = tam_lib_authentication_udi(tam_handle, prodName, prodSN, ECC_SUDI);
        if (status != TAM_RC_OK) {
            printf("\nAuthentication ECC failed with return status 0x%x ", status);
            if (status == TAM_LIB_ERR_UDI_MISMATCH) {
                printf(": The ECC UDI MISMATCH");
            }
            if (status == TAM_LIB_ERR_UDI_INVALID) {
                printf(": The ECC UDI INVALID");
            }
            if (rtrim(prodName) == FALSE) {
                printf("\n!!! Whitespace is not allowed after PID.");
            }
            printf("\n");
            return(FAILED);
        } else {
		    printf("\n Authentication ECC Passed\n");
        }
    }
#endif

#ifdef AIKIDO_SUPPORT_AIK
    if (aikido_aik_flag) {
        uint admin_session_id = 0;
        uint cleanup_status = 0;

        status = tam_lib_admin_login(tam_handle, &admin_session_id);
        if (status != TAM_RC_OK) {
            printf("%s act2l_gen_admin_credential status error %02x\n", 
                  __FUNCTION__, status);
            return(FAILED);
        }

        /* check AIK */
        status = tam_lib_aik_validate_certchain_udi(tam_handle, 
                                                    admin_session_id, 
                                                    prodName,
                                                    prodSN); 
        if (status != TAM_RC_OK) {
            printf("\nAuthentication AIK failed with return status 0x%x ", status);
            if (status == TAM_LIB_ERR_UDI_MISMATCH) {
                printf(": The AIK UDI MISMATCH");
            }
            if (status == TAM_LIB_ERR_UDI_INVALID) {
                printf(": The AIK UDI INVALID");
            }
            if (rtrim(prodName) == FALSE) {
                printf("\n!!! Whitespace is not allowed after PID.");
            }
            printf("\n");
            goto config_exit;
        } else {
            printf("\n Authentication AIK Passed\n");
        }

config_exit:
        cleanup_status = tam_lib_admin_logout(tam_handle, admin_session_id);
        if (cleanup_status) {
            printf("Terminate session error on %s \n", __FUNCTION__);
        }
    }
#endif

    return PASSED;

}
/*-------------------------------------------------------------------
 * Function: get_wdc
 *
 * Description : helper function to getwdc from terminal and convert to binary
 *
 * Inputs: NONE
 *
 * Output: data_size - size of data read from terminal
 *         credential - string containing credential
 * -------------------------------------------------------------------
 */
static int
get_wdc(uint16_t *data_size, unsigned char *credential)
{
    unsigned int size, i, byte;
    uchar cre_buf[SUDI_MAX_SIZE];
    uchar cre[SUDI_MAX_SIZE];
    uchar *tmp;
    char *argv[] = { "progwdc", "M"};

    assert(credential);
    memset(cre_buf, 0xA5, sizeof(cre_buf));
    memset(credential, 0xA5, SUDI_MAX_SIZE);

    /* reset to make sure chip is not in simple mode. */
    tam_act2_reset(0);
    if (print_cookie(2, argv) == FAILED) {
        cterr('f', 0, "WDC: unable to read out cookie info.");
        return FAILED;
    }

    size = getdec_answer("\nEnter WDC Size", 0, 0, WDC_SIZE);
    assert(size);

    printf("\nEnter %d bytes: ", CRE_MAX_SIZE);
    getline_act2((char *)cre_buf, sizeof(cre_buf));
    
    /* format of data from script is "xx xx xx xx xx".
       there is a space between each set of data.
       in the above example, data size would be 5.
    */
    for (i = byte = 0, tmp = cre_buf; i < size; i++) {
        cre[byte++] = atoh(*tmp);
        tmp++;
        cre[byte++] = atoh(*tmp);
        tmp++;
        tmp++; /* ignore space */
    }

    printf("converting to binary...\n\n");fflush(stdout);
    
    for (i = 0; i < (size); i++) {
        credential[i] = ((cre[2*i] << 4) & 0xf0) | 
            ((cre[2*i + 1]) & 0x0f);
        printf("%02x ", credential[i]);
        if ((i%42) == 41)
            printf("\n");
    }
    *data_size = size;
    if (*data_size < 1) {
        cterr('f', 0, "unable to get wdc; wdc size is %d", data_size);                        
        return FAILED;
    }

    return PASSED;
}
/*-------------------------------------------------------------------
 * Function:  tam_act2_platform_initial_config
 *
 * Description : create wdc object and save it into act2 chip
 *
 * Inputs: module -- pointer to object that we want to pass to act2 driver
 *         src_buffer -- containing credential
 *         data_size - size of data read from terminal
 * Output: return status from library
 * -------------------------------------------------------------------
 */
static int
tam_act2_platform_initial_config (void *module,
                                  uchar *src_buffer,
                                  uint16_t data_size)
{
    uchar admin_password[64];
    uint status = 0;
    uint cleanup_status = 0;
    uint object_id = 0;
    uchar num_objects;
    uint ix;
    tam_lib_object_enum_couplet_t object_list[50];
    uint admin_session_id = 0;
    uint8_t *dest_buffer = NULL;
    int num_retries = 3;
    uint dummy_obj_id = 0;
    uint ctr = 0;
    uint16_t dummy_data_size = (uint16_t)DUMMY_OBJ_SIZE;
    uint8_t src_dummy_buffer[DUMMY_OBJ_SIZE];
    uint8_t dest_dummy_buffer[DUMMY_OBJ_SIZE];
    uint16_t read_obj_data_len = 0;

    tam_act2_switch_simple_mode(0);
    
    memset(object_list, 0xA5, sizeof(object_list));
    
    memset(admin_password, 0xA5, sizeof(admin_password));

    status = tam_lib_generate_admin_pin(tam_handle, (uint8_t *)admin_password);
    if (status != TAM_RC_OK) {
        cterr('f', 0, "%s act2l_gen_admin_credential status error %02x\n", 
              __FUNCTION__, status);
        return FAILED;
    }

    /* create administrator session */
    status = tam_lib_session_init(tam_handle, TAM_ADMIN_USER, admin_password, 
                                  &admin_session_id);
    if (status != TAM_RC_OK) {
        cterr('f', 0, "Create Session Failed with status %02x\n", status);
        goto config_exit;
    }
#ifdef AIKIDO_CT_DEV_KEY
    /* CSCvu36133 : Fixed WDC install cause consent token install and remove DEV key more complicated issue */
    uint32_t ct_key_obj_id, jx;
    tam_lib_object_enum_couplet_t ct_object_list[CT_MAX_OBJECTS];
    boolean skip_del_ct_key = FALSE;
    memset(&ct_object_list[0], 0x00, sizeof(ct_object_list));
    /* DEV Key detect */
    status = tam_lib_omgr_enable_object_naming(tam_handle, admin_session_id);
    if (status != TAM_RC_OK) {
        cterr('f', 0, "Create Session Failed with status 0x%02x\n", status);
        goto config_exit;
    }
    rem_dev_key_found = FALSE;
    for (jx = 0; jx < CT_MAX_OBJECTS; jx++) {
        status = tam_lib_omgr_get_oid_from_name(tam_handle, admin_session_id,
                                                ct_obj_name[jx], 
                                                &ct_key_obj_id);
        if (status == TAM_RC_OK) {
            ct_object_list[jx].object_id = ct_key_obj_id;
            printf("\nFind %s id : 0x%x\n", ct_obj_name[jx], ct_object_list[jx].object_id);
            rem_dev_key_found = TRUE;
        } else {
            printf("\nNot find %s , status 0x%02x\n", ct_obj_name[jx], status);
        }
    }
    /* DEV Key detect end */
#endif    
    status = tam_lib_object_enumerate(tam_handle, admin_session_id, &num_objects, 
                                      object_list);
    if (status != TAM_RC_OK) {
        cterr('f', 0, "secure object enumerate Failed with status %02x\n", status);
        goto config_exit;
    }

    printf("num obj=%d\n", num_objects);

    if (num_objects > 50)
        num_objects = 50;

    for (ix = 0; ix < num_objects; ix++) {
        printf("deleting [%d]: id: 0x%04x\n", ix, object_list[ix].object_id);
#ifdef AIKIDO_CT_DEV_KEY
        /* CSCvu36133 : Fixed WDC install cause consent token install and remove DEV key more complicated issue */       
        skip_del_ct_key = FALSE;
        for (jx = 0; jx < CT_MAX_OBJECTS; jx++) {
            if (object_list[ix].object_id == ct_object_list[jx].object_id) {
                /*  skip deleting the RM_DEV_KEY object  */
                printf("TAM lib. doesn't delete %s id : 0x%x\n", 
                        ct_obj_name[jx], object_list[ix].object_id);
                skip_del_ct_key = TRUE;
                continue;
            }
        }
        if (skip_del_ct_key == TRUE) {
            continue;
        }
#endif
        status = tam_lib_object_delete(tam_handle, admin_session_id, 
                                       object_list[ix].object_id);
        if (status != TAM_RC_OK) {
            cterr('f', 0, "delete object Failed with status %02x\n", status);
            goto config_exit;
        }   
    }
#ifdef AIKIDO_CT_DEV_KEY
    /* CSCvu36133 : Fixed WDC install cause consent token install and remove DEV key more complicated issue */
    /* Per STO Rename the object to RM_DEV_KEY, if we don't do it can't find RM_DEV_KEY next time */
    if (rem_dev_key_found == TRUE) {
        status = tam_lib_omgr_enable_object_naming(tam_handle, admin_session_id);
        if (status != TAM_RC_OK) {
            cterr('f',0,"%s-%u enable object naming failed, status=0x%x-%s ",
                  __FUNCTION__, __LINE__, status, tam_lib_rc2string(status));
            goto config_exit;
        }
        for (jx = 0; jx < CT_MAX_OBJECTS; jx++) {
            if (ct_object_list[jx].object_id == 0x0) {
                continue;
            }
            
            /* Rename the object to RM_DEV_KEY  */
            printf("\nTAM lib. rename the object ID(0x%x) to %s\n", 
                   ct_object_list[jx].object_id, ct_obj_name[jx]);
            status = tam_lib_omgr_set_oid_name(tam_handle, admin_session_id, 
                                               ct_object_list[jx].object_id,
                                               ct_obj_name[jx]);
            if (status != TAM_RC_OK) {
                cterr('f',0,"%s-%u failed, status=0x%x-%s ", __FUNCTION__, __LINE__,
                       status, tam_lib_rc2string(status));
                goto config_exit;
            }
        }
    }
#endif
    if (is_tam_aikido_on()) {
        /* delete all users and their associated stored objects - cleanup */     
        for (ctr = 0; ctr < TAM_LIB_NUM_RESTRICTED_USERS; ctr++) {
            status = tam_lib_user_delete(tam_handle,
                                         admin_session_id,
                                         ctr+2);            
            if (status != TAM_RC_OK) {
                /* CSCvm79684: The new TAM lib.(newer than v2.10.2) restricted 
                   users reduced from 14 to 12. For backward compatibility, if 
                   get TAM_LIB_ERR_USERID_INVALID error, ignore this error and 
                   continue. */
                if (status == TAM_LIB_ERR_USERID_INVALID) {
                    printf("\nTAM Library doesn't allow to delete USER %d, "
                           "ignore this.\n", ctr);
                    continue;
                }
                cterr('f', 0, " user delete status %02x\n", status);
                goto config_exit;
            } else {
                if ((NVRAM)->diagflag & D_VERBOSE) {
                    printf("User %x deleted ", ctr);
                }
            }
        }
    }    
    dest_buffer = (uchar *)malloc(data_size);  /* freed by the library */
    assert(dest_buffer);
    memset(dest_buffer, 0, data_size);
    
    printf("secure_object_created %d\n", __LINE__);fflush(stdout);
    
    /* create a RAW unencrypted, non-CSP object in EEPROM of data_size bytes */
    status = tam_lib_object_create(tam_handle, admin_session_id, TAM_RAW_OBJECT, 
                                  data_size, TAM_LIB_NO_ZEROIZE,
                                  TAM_LIB_MEM_EEPROM, TAM_LIB_CLEAR_TEXT, &object_id);
    if (status != TAM_RC_OK) {
        cterr('f', 0, "Create Object Failed with status %02x\n", status);
        goto config_exit;
    }

    printf("write_object_created %d\n", __LINE__);fflush(stdout);
    /* Write buffer to object */
    tam_lib_object_write(tam_handle, admin_session_id, object_id, src_buffer, data_size);
    if (status != TAM_RC_OK) {
        cterr('f', 0, "Write Object Failed with status %02x\n", status);
        goto config_exit;
    }

    printf("read_object_created  data_size %d; line %d\n", data_size,  __LINE__);
    fflush(stdout);
    /* Read the object in that buffer */ 
    tam_lib_object_read(tam_handle, admin_session_id, object_id, dest_buffer, 
                        &read_obj_data_len);
    if (status != TAM_RC_OK) {
        cterr('f', 0, "Read Object Failed with status %02x\n", status);
        goto config_exit;
    }
#if defined (AIKIDO_CT_DEV_KEY) || defined (WDC_OBJECT_NAME)
    /* CSCvw97109 - Promethium IOS complains WDC length invalid */
    /* After investigation, IOS code likely use object_name=WDC_Object to find the WDC */
    /* In old codes, WDC is un-named, team asked to name the WDC object as WDC_Object */
    /* Code review with STO and team in the meeting */
    printf("\nTAM lib. add object ID(0x%x) with WDC_Object name\n", object_id);
    status = tam_lib_omgr_enable_object_naming(tam_handle, admin_session_id);
    if (status != TAM_RC_OK) {
        cterr('f',0,"%s-%u enable object naming failed, status=0x%x-%s ",
              __FUNCTION__, __LINE__, status, tam_lib_rc2string(status));
        goto config_exit;
    }
    status = tam_lib_omgr_set_oid_name(tam_handle, admin_session_id, 
                                       object_id, WDC_OBJ_NAME);
    if (status != TAM_RC_OK) {
        cterr('f',0,"%s-%u failed, status=0x%x-%s ", __FUNCTION__, __LINE__,
               status, tam_lib_rc2string(status));
        goto config_exit;
    } 
#endif 
    if (is_tam_aikido_on()) {
        /* (CSCvg25685)TSN: TAM FW wiping out the WDC object during this
         *                  power-cycle-with-software-running testing.
         *
         * Aikido TAM FW writes to the same sector of the WDC's, that will create
         * a corner case issue that during the erase/write, when it powers off,
         * WDC will be gone.
         * 
         * Before the ACT2 team has a permanent fix and TSN FCS is approaching,
         * Team suggest diags to reserve the remaining space in the same WDC sector
         * with a dummy object, so that the TAM sees no available space and
         * will not write to this sector.
         *
         * In order not to "potentially" affect current projects, put in a #if here
         * for the exception, for the Aikido-based projects.
         * Discrete-ACT2 seems not to be impacted by this, only Aikido-ACT2.
         */
    
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("\n[DBG]WDC data size = %d bytes; WDC dummy data size = %d.\n",
                   data_size, dummy_data_size);
        }
    
        /* create a RAW unencrypted, non-CSP object in EEPROM of DUMMY_OBJ_SIZE bytes */
        status = tam_lib_object_create(tam_handle, admin_session_id, TAM_RAW_OBJECT,
                                       DUMMY_OBJ_SIZE, TAM_LIB_NO_ZEROIZE,
                                       TAM_LIB_MEM_EEPROM, TAM_LIB_CLEAR_TEXT,
                                       &dummy_obj_id);
        if (status != TAM_RC_OK) {
            cterr('f', 0, "Create dummy Object Failed with status %02x\n", status);
            goto config_exit;
        }
        printf("dummy object_created %d\n", __LINE__);
        fflush(stdout);
    
        /* Write buffer to object */
        memset(src_dummy_buffer, 0, sizeof(src_dummy_buffer)); /* Normalize buffer */
        memset(src_dummy_buffer, 0xCC, DUMMY_WRITE_SIZE);
        if ((NVRAM)->diagflag & D_VERBOSE) {
            int ctr = 0;
    
            printf("\n[DBG]WDC dummy source data:\n");
            for (ctr = 0; ctr < sizeof(src_dummy_buffer); ctr++) {
                /* Change line every 32 dump value */
                if ((ctr % 32) == 0) {
                    printf("\n");
                }
                printf("%02x ", src_dummy_buffer[ctr]);
            }
        }
    
        tam_lib_object_write(tam_handle, admin_session_id, dummy_obj_id,
                             src_dummy_buffer, DUMMY_WRITE_SIZE);
        if (status != TAM_RC_OK) {
            cterr('f', 0, "Write Object Failed with status %02x\n", status);
            goto config_exit;
        }
    
        /* Read the object in that buffer */
        memset(dest_dummy_buffer, 0, sizeof(dest_dummy_buffer)); /* Normalize buffer */
        tam_lib_object_read(tam_handle, admin_session_id, dummy_obj_id,
                            dest_dummy_buffer, &dummy_data_size);
        if (status != TAM_RC_OK) {
            cterr('f', 0, "Read Object Failed with status %02x\n", status);
            goto config_exit;
        }
    
        if ((NVRAM)->diagflag & D_VERBOSE) {
            int ctr = 0;
    
            printf("\n[DBG]WDC dummy read back data:\n");
            for (ctr = 0; ctr < sizeof(dest_dummy_buffer); ctr++) {
                /* Change line every 32 dump value */
                if ((ctr % 32) == 0) {
                    printf("\n");
                }
                printf("%02x ", dest_dummy_buffer[ctr]);
            }
        }
    }	
    /* Compare two buffers, if retries fail delete object */
    do {

        printf("writing object %d\n", num_retries);
        tam_lib_object_write(module, admin_session_id, object_id, src_buffer, 
                             data_size);
        if (status != TAM_RC_OK) {
            cterr('f', 0, "Retrying Write Object Failed with status %02x\n", status);
            goto config_exit;
                
        }
        printf("read_object_created %d\n", __LINE__);fflush(stdout);	
        tam_lib_object_read(tam_handle, admin_session_id, object_id, dest_buffer, 
                            &read_obj_data_len);
        if (status != TAM_RC_OK) {
            cterr('f', 0, "Read Object Failed with status %02x\n", status);
            goto config_exit;
        }

        if (!memcmp(dest_buffer, src_buffer, data_size)) {
            printf("ok passed\n");
            status = tam_lib_admin_logout(tam_handle, admin_session_id);
            return status;
        }

        /* If failed, retry writing original buffer, reread and compare again.*/
        printf("failed reading object trying again %d\n", num_retries);

    } while (--num_retries);

    
 config_exit:
    cleanup_status = tam_lib_admin_logout(tam_handle, admin_session_id);
    if (cleanup_status) {
        cterr('f', 0, "Termiate session error");
    }

    /* if we fail, return status of failure before cleanup */
    return status;
}
/*-------------------------------------------------------------------
 *
 * Function : tam_act2_install_wdc
 * Description: main entry point called to install wdc to act2 chip
 * INPUT:  a -- not used.
 * OUTPUT: return PASSED or FAILED
 * -------------------------------------------------------------------
 */
static int
tam_act2_install_wdc (int a)
{
    int ret_val;
    uchar credential[SUDI_MAX_SIZE];
    uint16_t data_size = 0;

    if (get_wdc(&data_size, credential) == FAILED) {
        return FAILED;
    }
    
    ret_val =  tam_act2_platform_initial_config(NULL, credential, data_size);
    if (ret_val) {
        cterr('f', 0, "%s failed status = %x\n", __FUNCTION__, ret_val);
        return FAILED;
    }

    printf("WCD passed\n");

    return PASSED;
}

/*-------------------------------------------------------------------
 *
 * Function : tam_act2_display_wdc
 * Description: main entry point called to display wdc from act2 chip
 * CSCvh32961 : Display WDC and Dummy data with Act2 chip
 * INPUT:  notused -- not used.
 * OUTPUT: return PASSED or FAILED
 * -------------------------------------------------------------------
 */
static int tam_act2_display_wdc (int notused)
{
    tam_lib_status_t status;
    tam_lib_object_enum_couplet_t object_list[TAM_LIB_MAX_ENUM_OBJECTS];
    uint8_t admin_pin[TAM_PIN_LENGTH];                
    uint8_t couplet_count = 0;
    uint8_t dest_buffer[SUDI_MAX_SIZE * 3]; /* SHA1/SHA2 are the large size object */
    uint8_t object_type;
    uint16_t object_size = 0, eeprom_obj_count = 0, ix = 0, jx = 0;
    uint16_t length = sizeof(dest_buffer);
    uint32_t admin_session_id = 0;
    tam_lib_object_attributes_t attributes;
#if defined (AIKIDO_CT_DEV_KEY) || defined (WDC_OBJECT_NAME)
    char obj_name[OBJ_NAME_LEN];
#endif
    memset(admin_pin, 0x00, TAM_PIN_LENGTH);
	
    status = tam_lib_generate_admin_pin(tam_handle, admin_pin);
    if (status != TAM_RC_OK) {
        printf("\ntam_lib_generate_admin_pin returned with status 0x%x\n", status);
    }

    /* Create the admin session to get the session_id */
    status = tam_lib_session_init(tam_handle, TAM_ADMIN_USER,
                                  admin_pin, &admin_session_id);
    if (status != TAM_RC_OK) {
        printf("\nCreate Session Failed with status 0x%x\n", status);
        goto config_exit;
    }
    status = tam_lib_object_list(tam_handle, admin_session_id, 0, 
                                 TAM_LIB_MAX_ENUM_OBJECTS, &eeprom_obj_count, 
                                 &couplet_count, object_list );
    if (status != TAM_RC_OK) {
        printf("\ntam_lib_object_list returned with status 0x%x\n", status);
        goto config_exit;
    }
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\neeprom_obj_count = [%d]", eeprom_obj_count);
    }
    if (eeprom_obj_count < WDC_DUMMY_OBJ_NUMBER) {
        printf("\nNo dummy data found\n");
    }
    /* get attributes of each object */
    for (ix = 0; ix < eeprom_obj_count ; ix++) {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("\nobj_count = [%d]", ix);
            printf("\nobject_list[%d].object_id   = 0x%x", ix, object_list[ix].object_id);
            printf("\nobject_list[%d].object_type = 0x%x", ix, object_list[ix].object_type);
        }
        status = tam_lib_object_readinfo(tam_handle, admin_session_id, 
                                         object_list[ix].object_id, 
                                         &object_type, &object_size );
        if (status != TAM_RC_OK) {
            printf("\ntam_lib_object_readinfo returned with status 0x%x\n", status);
            goto config_exit;
        }
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("\nobject_size : %d\n", object_size);
        }
        /* Check WDC object */
        if ((object_type == TAM_RAW_OBJECT) && (object_size == WDC_SIZE)) {

            status = tam_lib_object_read(tam_handle, admin_session_id, 
                                         object_list[ix].object_id, dest_buffer, 
                                         &length);
            if (status != TAM_RC_OK) {
                printf("\nRead Object Failed with status 0x%x\n", status);
                goto config_exit;
            }
            printf("\nWDC length : %d", length);
            printf("\nWDC data:\n");
            for (jx = 0; jx < length; jx++) {
                printf("%02x ", dest_buffer[jx]);
            }
            continue;
        }
        /* Check Dummy data object */
        if ((object_type == TAM_RAW_OBJECT) && (object_size == DUMMY_WRITE_SIZE)) { 
            /* We wrote only 10 bytes to dummy object */
            status = tam_lib_object_attributes(tam_handle, admin_session_id,
                                               object_list[ix].object_id,
                                               &attributes);
            if (status != TAM_RC_OK) {
                printf("\nRead Object Failed with status 0x%x\n", status);
                goto config_exit;
            }
            printf("\nFind dummy data");
            printf("\nDummy object size : %d", attributes.size);
            status = tam_lib_object_read(tam_handle, admin_session_id, 
                                         object_list[ix].object_id, dest_buffer, 
                                         &length);
            if (status != TAM_RC_OK) {
                printf("\nRead Object Failed with status 0x%x\n", status);
                goto config_exit;
            }
            printf("\nDummy data length : %d", length);
            printf("\nDummy data info :\n");
            for (jx = 0; jx < length; jx++) {
                printf("%02x ", dest_buffer[jx]);
            }
        } else {
            printf("\nNo dummy data found\n");
        }
    }
#if defined (AIKIDO_CT_DEV_KEY) || defined (WDC_OBJECT_NAME)
    /* CSCvw97109 - Promethium IOS complains WDC length invalid */
    /* After investigation, IOS code likely use object_name=WDC_Object to find the WDC */
    /* In old codes, WDC is un-named, team asked to name the WDC object as WDC_Object */
    /* This code section is to dump all objects in Aikido */
    /* Code review with STO and team in the meeting */
    printf("\n@@@@@@@@@@@@@ Debug @@@@@@@@@@@@@ \n");
    printf("\nTotal # of object = %d", eeprom_obj_count);

    status = tam_lib_omgr_enable_object_naming(tam_handle, admin_session_id);
    if (status != TAM_RC_OK) {
        cterr('f', 0, "Create Session Failed with status 0x%02x\n", status);
        goto config_exit;
    }

    for (ix = 0; ix < eeprom_obj_count ; ix++) {
        printf("\n\n========= Debug ========= \n");
        memset(obj_name, 0x00, sizeof(obj_name));
        memset(dest_buffer, 0x00, sizeof(dest_buffer));
        
        status = tam_lib_object_readinfo(tam_handle, admin_session_id, 
                                         object_list[ix].object_id, 
                                         &object_type, &object_size);
        if (status != TAM_RC_OK) {
            printf("\ntam_lib_object_readinfo returned with status 0x%x\n", status);
            goto config_exit;
        }
        status = tam_lib_omgr_get_name_from_oid(tam_handle,
                                                admin_session_id,
                                                object_list[ix].object_id,
                                                obj_name);
        if (status != TAM_RC_OK) {
            printf("\n%s-%u unable to get name oid status=0x%x-%s\n\r", 
                    __FUNCTION__, __LINE__, status, tam_lib_rc2string(status));
        }
        /* Display obj infomation */
        printf("\nDEBUG: object # [%d]", ix + 1);
        printf("\nobject_id   = 0x%x", object_list[ix].object_id);
        printf("\nobject_type = 0x%x", object_list[ix].object_type);
        printf("\nobject_size = %d", object_size);
        printf("\nobject_name = %s",  obj_name);
        status = tam_lib_object_read(tam_handle, admin_session_id, 
                                     object_list[ix].object_id, dest_buffer, 
                                     &length);
        if (status != TAM_RC_OK) {
            printf("\nRead Object Failed with status 0x%x\n", status);
            continue; /* buffer & length updated with object data on success, if not success skip to print data .*/
        }
        printf("\nobject length : %d", length);
        printf("\nobject data:\n");
        for (jx = 0; jx < length; jx++) {
            printf("%02x ", dest_buffer[jx]);
        }
    } 
#endif
config_exit:
    status = tam_lib_admin_logout(tam_handle, admin_session_id);
    if (status) {
        printf("\nTerminate session error\n");
    }
    /* if we fail, return status of failure before cleanup */
    return status;
}
/*-------------------------------------------------------------------
 *
 * Function : act2_install_wdc
 * Description: main entry point called to install wdc to act2 chip
 * INPUT:  a -- not used.
 * OUTPUT: return PASSED or FAILED
 * -------------------------------------------------------------------
 */
int
act2_install_wdc (int a)
{
    return tam_act2_install_wdc(a);
}

/*-------------------------------------------------------------------
 *
 * Function : tam_act2_enumerate
 * Description: enumerate wdc object ids stored in act2 chip. in theory, we
 * shoudl have exatcly one object id. no more. no less. this utility can help
 * check if there's too many objects or none at all.
 * INPUT:  notused -- not used.
 * OUTPUT: return PASSED or FAILED
 * -------------------------------------------------------------------
 */
static int
tam_act2_enumerate (int notused)
{
    printf("NOT supported by ACT2 TAM lib\n");
    return PASSED;
}

/*-------------------------------------------------------------------
 *
 * Function : tam_act2_platform_object_burn_in_test
 * Description: burn in test for act2 chip.  inside the for loop, we
 * can bump up the number of loops to run. higher the number the longer
 * the burn in test.
 * INPUT:  dummy -- not used.
 * OUTPUT: return PASSED or FAILED
 * -------------------------------------------------------------------
 */
static int
tam_act2_platform_object_burn_in_test (int dummy)
{
    printf("NOT supported by ACT2 TAM lib\n");
    return PASSED;
}


/*-------------------------------------------------------------------
 *
 * Function : tam_act2_reset
 * Description: hard reset act2 chip. 
 * INPUT:  dummy -- not used.
 * OUTPUT: return PASSED or FAILED
 * -------------------------------------------------------------------
 */
int
tam_act2_reset (int dummy)
{
    struct ngio_intf_t *ngio;
#ifdef PLAT_HAS_PLUG
    struct plug_intf_t *plug;
#endif

    switch(cont.type) {
    case MOTHER_BOARD:
        printf("\nMOTHER_BOARD I2C Reset ");
        reset_plat_dev(FPGA_RST_ACT2);
        printf("\nMOTHER_BOARD I2C Un-reset ");
        unreset_plat_dev(FPGA_RST_ACT2);
        /* user may need to add delay or polling chip 
         * status for act2 chip ready. 
         */
        break;
    case SM_MODULE:
    case SM_DAUGHTER_CARD:
        ngio = (struct ngio_intf_t *)slot_get_ngiosm(cont.slot);
        printf("\nSM_MODULE I2C Reset ");
        if ((ngiosm_i2c_reset((void *)ngio)) < 0) {
            cterr('f', 0, "Unable to reset SM module slot %d", cont.slot);
            return FAILED;
        }
        printf("\nSM_MODULE I2C Un-reset ");
        if ((ngiosm_i2c_unreset((void *)ngio)) < 0) {
            cterr('f', 0, "Unable to unreset SM module slot %d", cont.slot);
            return FAILED;
        }
        break;
    case VM_MODULE:
        ngio = (struct ngio_intf_t *)slot_get_ngiovm(cont.slot);
        printf("\nVM_MODULE I2C Reset ");
        if ((ngiovm_i2c_reset(ngio)) < 0) {
            cterr('f', 0, "Unable to reset VM module.\n");
            return FAILED;
        }
        printf("\nVM_MODULE I2C Un-reset ");
        if ((ngiovm_i2c_unreset(ngio)) < 0) {
            cterr('f', 0, "Unable to unreset VM module.\n");
            return FAILED;
        }
        break;
    case WIC_MODULE:
    case WIC_DAUGHTER_CARD:
        printf("\nWIC_MODULE I2C Reset ");
        ngio = (struct ngio_intf_t *)slot_get_ngiowic(cont.slot);
        if ((ngiowic_i2c_reset(ngio)) < 0) {
            printf("Unable to reset WIC  module slot %d\n", cont.slot);
            return FAILED;
        }
        printf("\nWIC_MODULE I2C Un-reset ");
        if ((ngiowic_i2c_unreset(ngio)) < 0) {
            printf("Unable to unreset WIC  module slot %d\n", cont.slot);
            return FAILED;
        }
        break;
    case ISP_CARD:
#ifdef TACHI
        printf("\nISP I2C Un-reset ");
        ngio = (struct ngio_intf_t *)slot_get_ngioisp(cont.slot);
        if ((ngioisp_i2c_unreset(ngio)) < 0) {
            printf("Unable to ISP I2C Reset\n");
            return (FAILED);
        }
        printf("\nISP I2C Reset ");
        if ((ngioisp_i2c_reset(ngio)) < 0) {
            printf("Unable to ISP I2C Un-reset \n");
            return (FAILED);
        }
        break;
#endif
#ifdef PLAT_HAS_PLUG
    case PLUGGABLE_CARD:
        plug = (struct plug_intf_t *)slot_get_plugslot(cont.slot);
        printf("\nPIM_MODULE I2C Reset ");
        if ((plug_slot_i2c_reset(plug)) < 0) {
            cterr('f', 0, "Unable to reset PIM module.\n");
            return FAILED;
        }
        printf("\nPIM_MODULE I2C Un-reset ");
        if ((plug_slot_i2c_unreset(plug)) < 0) {
            cterr('f', 0, "Unable to unreset PIM module.\n");
            return FAILED;
        }
        break;
#endif
    case DAUGHTER_CARD:
    default:
        break;
    }

    act2_simple_mode[device_type] = 0;
    msleep(200);

    return PASSED;
}

/*-------------------------------------------------------------------
 *
 * Function : tam_mfg_errinfo
 * Description: API to print the TAM Manufacturing library error infomation. 
 * INPUT:  dummy -- not used.
 * OUTPUT: return PASSED or FAILED
 * -------------------------------------------------------------------
 */
int
tam_mfg_errinfo (int dummy)
{
    tam_lib_mfg_errinfo_print();
    return PASSED;

}
/*-------------------------------------------------------------------
 *
 * Function : tam_mfg_errinfo_clr
 * Description: API to clear the TAM Manufacturing library error infomation. 
 * INPUT:  dummy -- not used.
 * OUTPUT: return PASSED or FAILED
 * -------------------------------------------------------------------
 */
int
tam_mfg_errinfo_clr (int dummy)
{
    tam_lib_mfg_errinfo_clear();
    return PASSED;

}
/*-------------------------------------------------------------------
 *
 * Function : tam_lib_display_scc_id
 * Description: Display the TAM chip information. 
 * INPUT:  dummy -- not used.
 * OUTPUT: return PASSED or FAILED
 * -------------------------------------------------------------------
 */
int
tam_lib_display_scc_id (int dummy)
{
    tam_lib_status_t status;
    tam_lib_scc_id_t scc_id;

    /* get the SCC_FW_ID data */
    memset(&scc_id, 0x00, sizeof(scc_id));
    status = tam_lib_scc_read_id(tam_handle, &scc_id);
    if (status == TAM_RC_OK) {
        printf("Chip Type  : %02x\n", scc_id.chip_type);
        printf("Chip Vendor: %02x\n", scc_id.chip_vendor);
        printf("Post Result: %02x\n", scc_id.post_result);
        printf("Chip Mode  : %s\n",scc_id.bus_mode ? "SIMPLE" : "LEGACY");
        printf("Firmware Version: %02x\n", scc_id.firmware_version);
        act2_chip_version = scc_id.firmware_version;
    } else {
        printf("\n%s-%u scc read failed, status=0x%x-%s ",
        __FUNCTION__, __LINE__,
        status, tam_lib_rc2string(status));
    }

    tam_show_library_version();
    return (PASSED);
}

/*-------------------------------------------------------------------
 *
 * Function : act2_reset
 * Description: hard reset act2 chip. 
 * INPUT:  dummy -- not used.
 * OUTPUT: return PASSED or FAILED
 * -------------------------------------------------------------------
 */
int
act2_reset (int dummy)
{
    return tam_act2_reset(dummy);
}
/*-------------------------------------------------------------------
 *
 * Function : act2_toggle_debug_flag
 * Description: Toggle the debug flag 
 * INPUT:  dummy -- not used.
 * OUTPUT: return PASSED or FAILED
 * -------------------------------------------------------------------
 */
int
act2_toggle_debug_flag (int dummy)
{
    act2_i2c_debug ^= 1;
    return(PASSED);
}

int get_serial_number_for_sudi(void) {

    uchar ser_field = PCB_SERIAL_NUM;
    ser_field = (uchar)gethex_answer("Enter data field for S/N (PCB S/N: 0xc1, Chasiss S/N: 0xc2):",
				     0x0, 0x0, 0xFF);

    if ((ser_field != PCB_SERIAL_NUM) && (ser_field != CHASSIS_SERIAL_NUM)) {
	cterr('f', 0, "Invalid data field for S/N 0x%02x", ser_field);
	return FAILED;
    }
    
    if (ser_field == PCB_SERIAL_NUM) {
	pcb_for_sudi = TRUE;
    } else {
	pcb_for_sudi = FALSE;
    }

    printf("\nControl Type            : [0x%x]\n", id);
    printf("ProdName/Id             : [%s]\n", prodName);    
    printf("PCB Serial Num          : [%s]\n", pcbSN);
    printf("Chassis Serial Num      : [%s]\n", chassisSN);
    memset(prodSN, '\0', sizeof(prodSN));
    if (get_tlv_serial(cookie, prodSN, ser_field)) {
	cterr('f', 0, "Data field 0x%02x is not programmed", ser_field);
	return (FAILED);
    }
    printf("Serial Num for SUDI/WDC : [%s]\n", prodSN);    
    
    return(PASSED);
}

    
/*
 * Function: act2_i2c_unit_test
 *
 * Description : Do ACT2 off line test.
 */
static int act2_i2c_unit_test() 
{
    int ix, ret_val, buf_size, status;
    uint16_t bytes_actually_read[1] = {0x00};
    void *platform_opaque_handle = NULL;


    /* This is the simple mode command sequence */
    uint8_t simple_mode_cmd_0[4] = {0x01, 0x80, 0x00, 0x02};
    uint8_t simple_mode_cmd_1[4] = {0x02, 0x31, 0x41, 0x0B};
    uint8_t simple_mode_cmd_2[4] = {0x03, 0x00, 0x00, 0x00};

    uint8_t wr_48_bytes_cmd[48] = {0x51, 0x00, 0x2C, 0x00, 0x01, 0x02, 0x03, 
                                   0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 
                                   0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 
                                   0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 
                                   0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 
                                   0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 
                                   0x27, 0x28, 0x29, 0x2A, 0x2B, 0xD0};

    uint8_t wr_64_bytes_cmd[64] = {0x51, 0x00, 0x3C, 0x00, 0x01, 0x02, 0x03, 
                                   0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 
                                   0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 
                                   0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 
                                   0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 
                                   0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 
                                   0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 
                                   0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, 
                                   0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 
                                   0x88};

    uint8_t wr_128_bytes_cmd[128] = {0x51, 0x00, 0x7C, 0x00, 0x01, 0x02, 0x03, 
                                     0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 
                                     0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 
                                     0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 
                                     0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 
                                     0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 
                                     0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 
                                     0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, 
                                     0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 
                                     0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x41, 0x42, 
                                     0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 
                                     0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50, 
                                     0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 
                                     0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 
                                     0x5F, 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 
                                     0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 
                                     0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72, 0x73, 
                                     0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 
                                     0x7B, 0x68};

    uint8_t wr_192_bytes_cmd[192] = {0x51, 0x00, 0xBC, 0x00, 0x01, 0x02, 0x03, 
                                     0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 
                                     0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 
                                     0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 
                                     0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 
                                     0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 
                                     0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 
                                     0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, 
                                     0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 
                                     0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x41, 0x42, 
                                     0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 
                                     0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50, 
                                     0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 
                                     0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 
                                     0x5F, 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 
                                     0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 
                                     0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72, 0x73, 
                                     0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 
                                     0x7B, 0x7C, 0x7D, 0x7E, 0x7F, 0x80, 0x81,
                                     0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 
                                     0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F, 
                                     0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 
                                     0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 
                                     0x9E, 0x9F, 0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 
                                     0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 
                                     0xAC, 0xAD, 0xAE, 0xAF, 0xB0, 0xB1, 0xB2, 
                                     0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 
                                     0xBA, 0xBB, 0x8C};
    uint8_t wr_256_bytes_cmd[256] = {0xA1, 0x00, 0xFC, 0x00, 0x01, 0x02, 0x03, 
                                     0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 
                                     0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 
                                     0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 
                                     0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 
                                     0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 
                                     0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 
                                     0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, 
                                     0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 
                                     0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x41, 0x42, 
                                     0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 
                                     0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50, 
                                     0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 
                                     0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 
                                     0x5F, 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 
                                     0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 
                                     0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72, 0x73, 
                                     0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 
                                     0x7B, 0x7C, 0x7D, 0x7E, 0x7F, 0x80, 0x81,
                                     0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 
                                     0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F, 
                                     0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 
                                     0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 
                                     0x9E, 0x9F, 0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 
                                     0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 
                                     0xAC, 0xAD, 0xAE, 0xAF, 0xB0, 0xB1, 0xB2, 
                                     0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 
                                     0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF, 0xC0, 
                                     0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 
                                     0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 
                                     0xCF, 0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 
                                     0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 
                                     0xDD, 0xDE, 0xDF, 0xE0, 0xE1, 0xE2, 0xE3, 
                                     0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 
                                     0xEB, 0xEC, 0xED, 0xEE, 0xF0, 0xF1, 0xF2,
                                     0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9,
                                     0xFA, 0xFB, 0xD8};
    uint8_t wr_48_bytes_rsp[48] = {0x52, 0x00, 0x2C, 0x00, 0x01, 0x02, 0x03, 
                                   0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 
                                   0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 
                                   0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 
                                   0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 
                                   0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 
                                   0x27, 0x28, 0x29, 0x2A, 0x2B, 0xCF};

    uint8_t wr_64_bytes_rsp[64] = {0x52, 0x00, 0x3C, 0x00, 0x01, 0x02, 0x03, 
                                   0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 
                                   0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 
                                   0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 
                                   0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 
                                   0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 
                                   0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 
                                   0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, 
                                   0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 
                                   0x87};


    uint8_t wr_128_bytes_rsp[128] = {0x52, 0x00, 0x7C, 0x00, 0x01, 0x02, 0x03, 
                                     0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 
                                     0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 
                                     0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 
                                     0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 
                                     0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 
                                     0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 
                                     0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, 
                                     0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 
                                     0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x41, 0x42, 
                                     0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 
                                     0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50, 
                                     0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 
                                     0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 
                                     0x5F, 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 
                                     0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 
                                     0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72, 0x73, 
                                     0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 
                                     0x7B, 0x67};

    uint8_t rand_cmd[14] = {0x86, 0x00, 0x0A, 0x01, 0x02, 0x03, 0x04, 0x00, 0x00,
                            0x00, 0x02, 0x00, 0xFE, 0x65};

    /* The Chip Serial Number */
    uint8_t csn_cmd[4] = {0x8f, 0x00, 0x00, 0x70};

    /* This response buffer will be common for each unit test as we will 
       implement the typical sequence used by the TAm Library */
    uint8_t five_byte_response[5];
    uint8_t second_read_buffer[256];
    uint8_t rd_48_bytes_buffer[48];
    uint8_t rd_64_bytes_buffer[64];
    uint8_t rd_128_bytes_buffer[128];
    uint8_t rd_192_bytes_buffer[192];
    uint8_t rd_240_bytes_buffer[240];
    uint8_t rd_256_bytes_buffer[256];
    /* simple mode response */
    const uint8_t simple_mode_response[5] = {0x80, 0x00, 0x01, 0x00, 0x7E};
    /* already in simple mode response*/
    const uint8_t already_simple_mode_response[5] = {0x32, 0x00, 0x01, 0x03, 0xc9};
    const uint8_t csn_response[4] = {0x8F, 0x00, 0x21, 0x00};

    printf("This utility is only for discrete ACT2, and it currently(Sep-2017) doesn't support Aikido.\n");
    /* TRUE - Aikido act2  FALSE - Discrete act2 */
    status = is_tam_aikido_on();     
    if (status == TRUE) {
        printf("\n This test doesn't support Aikido.\n");
        return (FAILED);
    } else {
        printf("\n The chip is discrete ACT2. \n");
    }

    /* Write simple mode command to enter simple mode */
    buf_size = 4;
    ret_val = tam_lib_platform_write(platform_opaque_handle, simple_mode_cmd_0, buf_size);

    if (ret_val == -1) {
        printf("%s: I2C Write error with %#x", __FUNCTION__, ret_val);
        return (FAILED);
    } 
    buf_size = 4;
    ret_val = tam_lib_platform_write(platform_opaque_handle, simple_mode_cmd_1, buf_size);

    if (ret_val == -1) {
        printf("%s: I2C Write error with %#x", __FUNCTION__, ret_val);
        return (FAILED);
    } 
    buf_size = 4;
    ret_val = tam_lib_platform_write(platform_opaque_handle, simple_mode_cmd_2, buf_size);

    if (ret_val == -1) {
        printf("%s: I2C Write error with %#x", __FUNCTION__, ret_val);
        return (FAILED);
    }

    memset(five_byte_response, 0, sizeof(five_byte_response));
    buf_size = 5;
    ret_val = tam_lib_platform_read(platform_opaque_handle, 0, 0, five_byte_response, buf_size, bytes_actually_read);
    
    if (ret_val == -1) {
        printf("%s: I2C Read error with %#x", __FUNCTION__, ret_val);
        return (FAILED);
    }
    if (memcmp(five_byte_response, simple_mode_response, 5)) {
        if (memcmp(five_byte_response, already_simple_mode_response, 5)) {
            printf("Unable to get simple mode response.");
            return (FAILED);
        }
    }
    printf("Write CNS command\n");
    buf_size = 4;
    ret_val = tam_lib_platform_write(platform_opaque_handle, csn_cmd, buf_size);

    if (ret_val == -1) {
        printf("%s: I2C Write error with %#x", __FUNCTION__, ret_val);
        return (FAILED);
    }
    
    memset(five_byte_response, 0, sizeof(five_byte_response));
    /* First read back 5 bytes for csn command */
    buf_size = 5;
    ret_val = tam_lib_platform_read(platform_opaque_handle, 0, 0, five_byte_response,
                    buf_size, bytes_actually_read);
    
    if (ret_val == -1) {
        printf("%s: I2C Read error with %#x", __FUNCTION__, ret_val);
        return (FAILED);
    }

    /* Only the first 4 bytes are deterministic. */
    if (memcmp(csn_response, five_byte_response, 4)) {
        printf("CSN first response is not correct..");
        return (FAILED);
    }
    
    memset(second_read_buffer, 0, sizeof(second_read_buffer));
    /* Second read back 32 bytes for csn command */
    buf_size = 32;
    ret_val = tam_lib_platform_read(platform_opaque_handle, 0, 0, second_read_buffer,
                    buf_size, bytes_actually_read);
    
    if (ret_val == -1) {
        printf("%s: I2C Read error with %#x", __FUNCTION__, ret_val);
        return (FAILED);
    }

    printf(" CSN command response:");
    for (ix = 0; ix < 5; ix++) {
        printf("%02x ", five_byte_response[ix]);
    } 
    
    for (ix = 0; ix < 32; ix++) {
        printf("%02x ", second_read_buffer[ix]);
    } 


    buf_size = 48;
    ret_val = tam_lib_platform_write(platform_opaque_handle, wr_48_bytes_cmd, buf_size);

    if (ret_val == -1) {
        printf("%s: I2C Write error with %#x", __FUNCTION__, ret_val);
        return (FAILED);
    }
    
    memset(rd_48_bytes_buffer, 0, sizeof(rd_48_bytes_buffer));
    /* Read back 48 bytes for 48 write command */
    buf_size = 48;
    ret_val = tam_lib_platform_read(platform_opaque_handle, 0, 0, rd_48_bytes_buffer,
                            buf_size, bytes_actually_read);
    
    if (ret_val == -1) {
        printf("%s: I2C Read error with %#x", __FUNCTION__, ret_val);
        return (FAILED);
    }
    
    printf("\n\n\nStep 1.");
    printf("\n48 bytes command response:\n");
    for (ix = 0; ix < 48; ix++) {
        printf("%02x ", rd_48_bytes_buffer[ix]);
    } 
    
    /* Compare 48 bytes response. */
    if (memcmp(wr_48_bytes_rsp, rd_48_bytes_buffer, 48)) {
        printf("Write 48 bytes response is not correct..");
              return (FAILED);
    }

    
    buf_size = 64;
    ret_val = tam_lib_platform_write(platform_opaque_handle, wr_64_bytes_cmd, buf_size);

    if (ret_val == -1) {
        printf("%s: I2C Write error with %#x", __FUNCTION__, ret_val);
        return (FAILED);
    }
    
    memset(rd_64_bytes_buffer, 0, sizeof(rd_64_bytes_buffer));
    /* Read back 64 bytes for 64 write command */
    buf_size = 64;
    ret_val = tam_lib_platform_read(platform_opaque_handle, 0, 0, rd_64_bytes_buffer, 
                             buf_size, bytes_actually_read);
    
    if (ret_val == -1) {
        printf("%s: I2C Read error with %#x", __FUNCTION__, ret_val);
        return (FAILED);
    }

    printf("\n\n\nStep 2.");
    printf("\n64 bytes command response:\n");
    for (ix = 0; ix < 64; ix++) {
        printf("%02x ", rd_64_bytes_buffer[ix]);
    } 
    
    /* Compare 64 bytes response. */
    if (memcmp(wr_64_bytes_rsp, rd_64_bytes_buffer, 64)) {
        printf("Write 64 bytes response is not correct..");
        return (FAILED);
    }

    
    /* 128 writes command */
    buf_size = 128;
    ret_val = tam_lib_platform_write(platform_opaque_handle, wr_128_bytes_cmd, buf_size);

    if (ret_val == -1) {
        printf("%s: I2C Write error with %#x", __FUNCTION__, ret_val);
        return (FAILED);
    }
    
    memset(rd_128_bytes_buffer, 0, sizeof(rd_128_bytes_buffer));
    /* Read back 128 bytes for 128 write command */
    buf_size = 128;
    ret_val = tam_lib_platform_read(platform_opaque_handle, 0, 0, rd_128_bytes_buffer, 
                          buf_size, bytes_actually_read);
    
    if (ret_val == -1) {
        printf("%s: I2C Read error with %#x", __FUNCTION__, ret_val);
        return (FAILED);
    }

    printf("\n\n\nStep 3.");
    printf("\n128 bytes command response:\n");
    for (ix = 0; ix < 128; ix++) {
        printf("%02x ", rd_128_bytes_buffer[ix]);
    } 
    
    /* Compare 128 bytes response. */
    if (memcmp(wr_128_bytes_rsp, rd_128_bytes_buffer, 128)) {
        printf("Write 128 bytes response is not correct..");
              return (FAILED);
    }

    
    /* 192 writes command */
    buf_size = 192;
    ret_val = tam_lib_platform_write(platform_opaque_handle, wr_192_bytes_cmd, buf_size);

    if (ret_val == -1) {
        printf("%s: I2C Write error with %#x", __FUNCTION__, ret_val);
        return (FAILED);
    }
    
    memset(rd_192_bytes_buffer, 0, sizeof(rd_192_bytes_buffer));
    /* Read back 192 bytes for rand command */
    buf_size = 192;
    ret_val = tam_lib_platform_read(platform_opaque_handle, 0, 0, rd_192_bytes_buffer,
                              buf_size, bytes_actually_read);
    
    if (ret_val == -1) {
        printf("%s: I2C Read error with %#x", __FUNCTION__, ret_val);
        return (FAILED);
    }

    printf("\n\n\nStep 4.");
    printf("\nRead 192 bytes for rand command response:\n");
    printf("The first 8 bytes should be 0x32, 0x00, 0x01, 0x51, 0x7B, 0x00, 0x00, 0x00.\n");
    printf("If you get 192 bytes and that 8 byte header is right, it PASSES.\n");
    for (ix = 0; ix < 192; ix++) {
        printf("%02x ", rd_192_bytes_buffer[ix]);
    } 
    
    
    /* 256 writes command */

    buf_size = 256;
    ret_val = tam_lib_platform_write(platform_opaque_handle, wr_256_bytes_cmd, buf_size);

    if (ret_val == -1) {
        printf("%s: I2C Write error with %#x", __FUNCTION__, ret_val);
        return (FAILED);
    }
    

    memset(rd_256_bytes_buffer, 0, sizeof(rd_256_bytes_buffer));
    /* Read back 256 bytes for rand command */
    buf_size = 256;
    ret_val = tam_lib_platform_read(platform_opaque_handle, 0, 0, rd_256_bytes_buffer, 
                                 buf_size, bytes_actually_read);
    
    if (ret_val == -1) {
        printf("%s: I2C Read error with %#x", __FUNCTION__, ret_val);
        return (FAILED);
    }

    printf("\n\n\nStep 5.");
    printf("\nRead 256 bytes for rand command response:\n");
    printf("The first 4 bytes should be 0xA1, 0x00, 0x01, 0x0B\n");
    printf("If you get 256 bytes and that 4 byte header is right, it PASSES.\n");
    for (ix = 0; ix < 256; ix++) {
        printf("%02x ", rd_256_bytes_buffer[ix]);
    } 

    
    /* 259 writes rand command */
    buf_size = 14;
    ret_val = tam_lib_platform_write(platform_opaque_handle, rand_cmd, buf_size);

    if (ret_val == -1) {
        printf("%s: I2C Write error with %#x", __FUNCTION__, ret_val);
        return (FAILED);
    } 
    
    memset(rd_64_bytes_buffer, 0, sizeof(rd_64_bytes_buffer));
    memset(rd_240_bytes_buffer, 0, sizeof(rd_240_bytes_buffer));
    /* Read back 240 bytes for rand command */
    buf_size = 240;
    ret_val = tam_lib_platform_read(platform_opaque_handle, 0, 0, rd_240_bytes_buffer, 
                             buf_size, bytes_actually_read);
    
    if (ret_val == -1) {
        printf("%s: I2C Read error with length 240 bytes %#x from rand command", 
              __FUNCTION__, ret_val);
        return (FAILED);
    }
    
    /* Read back 19 bytes for rand command */
    buf_size = 19;
    ret_val = tam_lib_platform_read(platform_opaque_handle, 0, 0, rd_64_bytes_buffer, 
                             buf_size, bytes_actually_read);
    
    if (ret_val == -1) {
        printf("%s: I2C Read error with length 19 bytes %#x from rand command", 
              __FUNCTION__, ret_val);
        return (FAILED);
    }

    printf("\n\n\nStep 6.\n");
    printf("Read 259 bytes for rand command response by read 240 bytes first, then read 19 bytes:\n");
    printf("The first 4 bytes should be 0x86, 0x00, 0xFF, 0x00, following data is random.\n");
    printf("If you get 259 bytes and that 4 byte header is right, it PASSES.\n");
    printf(" Read 240 bytes for rand command response:\n");
    for (ix = 0; ix < 240; ix++) {
        printf("%02x ", rd_240_bytes_buffer[ix]);
    } 
    printf("\nRead rest 19 bytes for rand command response:\n");
    for (ix = 0; ix < 19; ix++) {
        printf("%02x ", rd_64_bytes_buffer[ix]);
    } 

    printf("\n\nACT2 off line test passed\n");
    return (PASSED);
}

/*-------------------------------------------------------------------
 * Function : tam_show_library_version
 * Description: This function displays TAM library version
 * INPUT:  None
 * OUTPUT: None
 * -------------------------------------------------------------------
 */
static void tam_show_library_version (void)
{
    tam_library_version_t tam_library_version; 
    
    tam_lib_get_library_version(&tam_library_version);

    printf("TAM Library Version: %u.%u.%u \n\n", tam_library_version.major,
                                                 tam_library_version.minor,
                                                 tam_library_version.patch);
}


#ifdef SUDI_2099_2029
/*-------------------------------------------------------------------
 * Function : is_tam_library_sudi_2099
 * Description: This function checks whether TAM library version
 *              supports SUDI 2099
 *              (Version >= 3.1.1 supports SUDI 2099 otherwise
 *               it doesn't support SUDI 2099)
 * INPUT:  None
 * OUTPUT: return TRUE or FALSE
 * -------------------------------------------------------------------
 */
static int is_tam_library_sudi_2099 (void)
{
    tam_library_version_t tam_library_version; 
    
    tam_lib_get_library_version(&tam_library_version);

    if ((((tam_library_version.major & 0xFF) << 16) |
        ((tam_library_version.minor & 0xFF) << 8) |
        (tam_library_version.patch & 0xFF)) >= TAM_LIB_2099_VERSION) {
        return (TRUE);
    } else {
        return (FALSE);
    }
}
#endif
#ifdef ENABLE_SUDI_2099_CLI
/*-------------------------------------------------------------------
 * Function : tam_toggle_sudi_2099
 * Description: Utility to toggle SUDI 2099 flag
 * INPUT:  
 * OUTPUT: return PASSED or FAILED
 * -------------------------------------------------------------------
 */
static int tam_toggle_sudi_2099 (void)
{
    char answer;

    /* This utility is only for TAM library newer than V3.1.1 which supports
     * SUDI 2099 and it is intended for Discrete ACT-2 only
     */
    if (is_tam_library_sudi_2099() == FALSE) {
        printf("*** ERROR: TAM Library doesn't support SUDI 2099\n");
        tam_show_library_version();
        return (FAILED);
    }

    /* Display the current status of SUDI 2099 flag */
    PRINT_SUDI_2099_STS

    answer = sudi_2099 == TRUE ? 'y' : 'n';

    /* Prompt user */
    answer = getc_answer("\nEnable [y/n]:", "yn", answer);

    if (answer == 'y') {
        sudi_2099 = TRUE;
    } else {
        sudi_2099 = FALSE;
    }

    printf("TAM 2099 is now '%s'\n", sudi_2099 == TRUE ? "ON": "OFF");

    return (PASSED);
}
#endif

#ifdef SHA1_SHA2_SWSUDI2099

/*-------------------------------------------------------------------
 *  Function : act2_enter_sha1_sha2_swsudi_obj() 
 *  Description: Gets the object(SHA1/SHA2/SWSUDI key/cert in 100bytes chunks and converts them to HEX
 *               for storing in ACT2 
 *  INPUT:
 *      act2_obj_len : Length of Object
 *      act2_raw_obj_buffer - Buffer pointer containing the Object
 *  OUTPUT: TEST_FAILED or TEST_PASSED
 *  -------------------------------------------------------------------
 */
static int act2_enter_sha1_sha2_swsudi_obj(uint8_t* *act2_raw_obj_buffer,uint16_t act2_obj_len)
{
    uint16_t key_cert_length = 0;
    uint16_t ix;
    uint8_t *ascii_act2_obj_buffer = NULL;
    uint len = 0;

    if (!act2_obj_len) {
        cterr('f',0,"Object length must be greater than 0");
        return (FAILED);
    }
    if (act2_obj_len > MAX_ACT2_OBJ_LEN) {
        /*We will face buffer overrun issue at read */
        cterr('f',0,"Object len must be less than %d",MAX_ACT2_OBJ_LEN);
        return (FAILED);
    }

    /* If existing, free it, the size might have changed */
    if (*act2_raw_obj_buffer) {
        printf("act2_raw_obj_buffer not NULL. Freeing\n\r");
        free(*act2_raw_obj_buffer);
        (*act2_raw_obj_buffer) = NULL;
    }

    /* allocate memory for the buffers */
    (*act2_raw_obj_buffer) = (uint8_t *)malloc((act2_obj_len * ASCII_CHAR_PER_BYTE) + 2);
    if (!(*act2_raw_obj_buffer)) {
        cterr('f',0,"Could not malloc a signature buffer of size %d", 
               act2_obj_len);
        return (FAILED);
    }

    ascii_act2_obj_buffer = (uint8_t *)malloc((act2_obj_len * ASCII_CHAR_PER_BYTE) + 2);
    if (!ascii_act2_obj_buffer) {
        cterr('f',0,"Could not malloc an ascii buffer of size %d", 
             ((act2_obj_len*2) + 100));
        free(*act2_raw_obj_buffer);
        (*act2_raw_obj_buffer) = NULL;
        return (FAILED);
    }

    memset((*act2_raw_obj_buffer), 0x00, (act2_obj_len * ASCII_CHAR_PER_BYTE) + 2);
    memset(ascii_act2_obj_buffer, 0x00, (act2_obj_len * ASCII_CHAR_PER_BYTE) + 2);


    key_cert_length = (act2_obj_len * 2) ;
    printf("Entering %s data, expecting a total of %d bytes",
            act2_obj_disp_name[which_act2_obj_param],  key_cert_length);
    for (ix = 0; ix < key_cert_length; ix += len) {
        printf("\nRead %s data > ", act2_obj_disp_name[which_act2_obj_param]);
        fflush(stdout);
        len = getline_act2((char *)&ascii_act2_obj_buffer[ix], 
                            getpagesize() - 2);
        printf("%s count %d; total bytes entered %d\n", 
                act2_obj_disp_name[which_act2_obj_param], len, ix + len );
        fflush(stdout);
    }
    printf("\nExpected %d Received %d bytes\n",  key_cert_length, ix);
    fflush(stdout);
    if (ix != key_cert_length ) {
        cterr('f',0,"Expected %d Received %d bytes\n",  key_cert_length, ix);
        fflush(stdout);
        free(ascii_act2_obj_buffer);
        free(*act2_raw_obj_buffer);
        (*act2_raw_obj_buffer) = NULL;
        return (FAILED);
    }

    /* convert ascii to hex */
    for (ix = 0; ix < key_cert_length; ix++) {
        ascii_act2_obj_buffer[ix] = atoh(ascii_act2_obj_buffer[ix]);
    }
    for (ix = 0; ix < act2_obj_len; ix++) {
        (*act2_raw_obj_buffer)[ix] = ((ascii_act2_obj_buffer[2 * ix] << 4) & 0xf0) | 
            ((ascii_act2_obj_buffer[2 * ix + 1]) & 0x0f);
    }

    /* return the ascii buffer to the pool */
    free(ascii_act2_obj_buffer);

    return (PASSED);
}
/*-------------------------------------------------------------------
 *  Function : act2_install_sha1_sha2_swsudi_obj() 
 *  Description: Write the SHA1/SHA2/SWSUDI
 *               Object(KeyPair/Cert)  on to ACT2. 
 *  INPUT: 
 *      act2_obj_len : Length of Object
 *      act2_raw_obj_buffer - Buffer pointer containing the Object
 *      Global - which_act2_obj_param   Which Object. Index of the name list 
 *  OUTPUT: return SUCCESS or FAILURE
 *  -------------------------------------------------------------------
 */

static int act2_install_sha1_sha2_swsudi_obj (uint8_t* *act2_raw_obj_buffer,uint16_t act2_obj_len)
{
    tam_lib_status_t status = 0;
    uint cleanup_status = 0;
    char obj_name[128];
    uint8_t admin_pin[TAM_PIN_LENGTH];
    uint32_t admin_session_id = 0;
    uint8_t num_admin_objs = 0;
    tam_lib_object_enum_couplet_t admin_obj_list[TAM_LIB_MAX_ENUM_OBJECTS];
    uint32_t admin_act2_obj_id = 0;
    uint8_t *read_acts_obj_buffer = NULL;
    uint16_t read_act2_obj_len = 0;
    uint32_t del_object = 0;
    uint16_t ix;

    /* Make sure the buffer pointer is not NULL */
    if ((*act2_raw_obj_buffer)==NULL) {
        cterr('f',0,"act2_raw_obj_buffer is NULL");
        return (FAILED);
    }

    /* Make sure the buffer size is > 0 */
    if (!act2_obj_len) {
        cterr('f',0,"SWSUDI length must be greater than 0");
        free(*act2_raw_obj_buffer);
        (*act2_raw_obj_buffer) = NULL;
        return (FAILED);
    }

    status = tam_act2_switch_simple_mode(0);
    if (status) {
        printf("Cannot switch device to simple mode. may already be in simple mode.");
    }

    /* allocate memory for the buffers */
    read_acts_obj_buffer = (uint8_t *)malloc(act2_obj_len);
    if (!read_acts_obj_buffer) {
        cterr('f',0,"Could not malloc a read buffer of size %d", 
               act2_obj_len);
        free(*act2_raw_obj_buffer);
        (*act2_raw_obj_buffer) = NULL;
        return (FAILED);
    }

    memset(&admin_pin[0], 0x00, TAM_PIN_LENGTH);
    memset(&admin_obj_list[0], 0x00, sizeof(admin_obj_list));

    /*** Sequence of operations: *
      *  1. generate admin pin   *
      *  2. open admin session   *
      *  3. enable desired endianess   *
      *  4. enable object naming   *
      *  5. enumerate objects    *
      *  6. check if the object is present already    *
      *  7. if exsits ,delete existing object *
      *  8. create the  object     *
      *  9. write the  objet data and name it  *
      *  10. read the object data    *
      *  11. compare read vs write *
      *  12. close admin session  *
      */

    /* generate the admin pin */
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Generating Admin Pin\n");
    }

    status = tam_lib_generate_admin_pin(tam_handle, &admin_pin[0]);

    if (status != TAM_RC_OK) {
        cterr('f',0,"%s-%u failed, status=0x%x-%s ", __FUNCTION__, __LINE__,
               status, tam_lib_rc2string(status));
        free(read_acts_obj_buffer);
        free(*act2_raw_obj_buffer);
        (*act2_raw_obj_buffer) = NULL;
        return (FAILED);
    }

    /* open the admin session */
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Opening Admin session\n");
    }

    status = tam_lib_session_init(tam_handle, 
                                  TAM_ADMIN_USER,
                                  &admin_pin[0],
                                  &admin_session_id);

    if (status != TAM_RC_OK) {
        cterr('f',0,"%s-%u failed, status=0x%x-%s ", __FUNCTION__, __LINE__,
              status, tam_lib_rc2string(status));
        free(read_acts_obj_buffer);
        free(*act2_raw_obj_buffer);
        (*act2_raw_obj_buffer) = NULL;
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Admin session opened, session id 0x%x\n", admin_session_id);
    }
    status = tam_lib_omgr_set_endianness(tam_handle, TAM_LITTLE_ENDIAN);
    if (status != TAM_RC_OK) {
        cterr('f',0,"%s-%u enable little endian failed, status=0x%x-%s ",
               __FUNCTION__, __LINE__, status, tam_lib_rc2string(status));
        goto config_exit;
    }
    printf("enabled LE session id 0x%x\n", admin_session_id);
    status = tam_lib_omgr_enable_object_naming(tam_handle, admin_session_id);

    if (status != TAM_RC_OK) {
        cterr('f',0,"%s-%u enable object naming failed, status=0x%x-%s ",
               __FUNCTION__, __LINE__, status, tam_lib_rc2string(status));
        goto config_exit;
    }
    /* enumerate existing admin objects */
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Enumerating Admin objects\n");
    }

    status = tam_lib_object_enumerate(tam_handle, 
                                      admin_session_id,
                                      &num_admin_objs,
                                      &admin_obj_list[0]);

    if (status != TAM_RC_OK) {
        cterr('f',0,"%s-%u failed, status=0x%x-%s ", __FUNCTION__, __LINE__,
               status, tam_lib_rc2string(status));
        goto config_exit;
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Number of admin objects 0x%x\n", num_admin_objs);
    }

    if ( num_admin_objs > TAM_LIB_MAX_ENUM_OBJECTS ) {
        printf("WARNING: Number of admin objects 0x%x is greater than expected "
               "max value.\n", num_admin_objs);
        printf("WARNING: Setting value to max value.\n");

        num_admin_objs = TAM_LIB_MAX_ENUM_OBJECTS;
        printf("Number of admin objects 0x%x\n", num_admin_objs);
    }

    /* delete  existing objects */
    if (num_admin_objs) {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("Looking for  %s\n" ,act2_obj_name[which_act2_obj_param]);
        }

        for (del_object = 0; del_object < num_admin_objs; del_object++) {
            memset(obj_name,0,sizeof(obj_name));
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("found object  0x%x of 0x%x  Id %x\n", del_object,
                        num_admin_objs, admin_obj_list[del_object].object_id);
            }
            status = tam_lib_omgr_get_name_from_oid(tam_handle,
                                                    admin_session_id,
                                                    admin_obj_list[del_object].object_id,
                                                    obj_name);
            if (status != TAM_RC_OK) {
                if ((NVRAM)->diagflag & D_VERBOSE) {
                    printf(" Id 0x%x is not mapeed to a name\n\r", 
                            admin_obj_list[del_object].object_id);
                    continue;
                }
            }
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf(" object id %x named  as %s\n",
                        admin_obj_list[del_object].object_id, obj_name);
            }
            if (strcmp(obj_name, act2_obj_name[which_act2_obj_param])) {
                continue;
            }
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("Deleting object 0x%x of 0x%x\n", del_object, num_admin_objs);
                printf("Deleting object %s of 0x%x\n",obj_name, num_admin_objs);
            }
            status = tam_lib_omgr_delete_by_oid(tam_handle, 
                                                admin_session_id,
                                                admin_obj_list[del_object].object_id);
            if (status != TAM_RC_OK) {
                cterr('f',0,"%s-%u failed, status=0x%x-%s ", __FUNCTION__, 
                      __LINE__, status, tam_lib_rc2string(status));
                goto config_exit;
            }
            break;
        }
    }

    /* create object */
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Creating Admin object\n");
    }

    status = tam_lib_object_create(tam_handle, 
                                   admin_session_id,
                                   TAM_RAW_OBJECT,
                                   act2_obj_len,
                                   TAM_LIB_NO_ZEROIZE,
                                   TAM_LIB_MEM_EEPROM,
                                   TAM_LIB_MEM_EEPROM,
                                   &admin_act2_obj_id);

    if (status != TAM_RC_OK) {
        cterr('f',0,"%s-%u failed, status=0x%x-%s ", __FUNCTION__, __LINE__,
               status, tam_lib_rc2string(status));
        goto config_exit;
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Admin object created, object id 0x%x\n", admin_act2_obj_id);
    }

    /* write the data into the new object */
    printf("Writing the %s  into the Act2\n\r", act2_obj_name[which_act2_obj_param]);
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Writing the SHA1/SHA2/SWSUDI  data into the object\n");
    }

    status = tam_lib_object_write(tam_handle, 
                                  admin_session_id,
                                  admin_act2_obj_id,
                                  (*act2_raw_obj_buffer),
                                  act2_obj_len);

    if (status != TAM_RC_OK) {
        cterr('f',0,"%s-%u failed, status=0x%x-%s ", __FUNCTION__, __LINE__,
               status, tam_lib_rc2string(status));
        goto config_exit;
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Admin SHA1/SHA2/SWSUDI object written\n");
    }

    status  = tam_lib_omgr_set_oid_name(tam_handle,
                                        admin_session_id,
                                        admin_act2_obj_id,
                                        act2_obj_name[which_act2_obj_param]);

    if (status != TAM_RC_OK) {
        cterr('f',0,"%s-%u failed, status=0x%x-%s ", __FUNCTION__, __LINE__,
               status, tam_lib_rc2string(status));
        goto config_exit;
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Admin object named, name  %s\n", act2_obj_name[which_act2_obj_param]);
    }

    /* allocate memory for the read buffer */
    read_acts_obj_buffer = (uint8_t *)malloc(act2_obj_len);
    if (!read_acts_obj_buffer) {
        cterr('f',0,"Could not malloc a read buffer of size %d", act2_obj_len);
        goto config_exit;
    }

    memset(&read_acts_obj_buffer[0], 0x00, act2_obj_len);

    /* read the object */
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Verifying SWSUDI object\n");
    }

    read_act2_obj_len = act2_obj_len;

    status = tam_lib_object_read(tam_handle, 
                                 admin_session_id,
                                 admin_act2_obj_id,
                                 &read_acts_obj_buffer[0],
                                 &read_act2_obj_len);

    if (status != TAM_RC_OK) {
        cterr('f',0,"%s-%u failed, status=0x%x-%s ", __FUNCTION__, __LINE__,
               status, tam_lib_rc2string(status));
        goto config_exit;
    }

    if (read_act2_obj_len != act2_obj_len) {
        cterr('f',0,"%s: Read length 0x%x not equal to write length 0x%x", 
               __FUNCTION__, read_act2_obj_len, act2_obj_len);
        goto config_exit;
    }

    for (ix = 0; ix < act2_obj_len; ix++)  {
        if (read_acts_obj_buffer[ix] != (*act2_raw_obj_buffer)[ix]) {
            cterr('f',0,"%s: Read compare failed: wrote 0x%x  read 0x%x", 
                   __FUNCTION__, (*act2_raw_obj_buffer)[ix], read_acts_obj_buffer[ix]);
            goto config_exit;
        }
    }
    printf("Write Read Compare %s  Passed\n\r", act2_obj_disp_name[which_act2_obj_param]);

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Admin SHA1/SHA2/SWSUDI object verified\n");
    }	

    /* close the admin session */
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Closing Admin session\n");
    }	
    printf("%s install completed\n", act2_obj_disp_name[which_act2_obj_param]);

config_exit:
    cleanup_status = tam_lib_admin_logout(tam_handle, admin_session_id);
    if (cleanup_status) {
        cterr('f', 0, "Termiate session error");
    }
    if ((NVRAM)->diagflag & D_VERBOSE) {
         printf("Admin session closed\n");
    }	
    /* free the oject buffer and read buffer */
    free(read_acts_obj_buffer);
    free(*act2_raw_obj_buffer);
    (*act2_raw_obj_buffer) = NULL;

    /* if we fail, return status of failure before cleanup */
    return status;
}
/*-------------------------------------------------------------------
 *
 * Function : select_sha1_sha2_swsudi
 * Description: main entry point called to install wdc to act2 chip
 * INPUT:  a -- not used.
 * OUTPUT: return PASSED or FAILED
 * -------------------------------------------------------------------
 */
static int select_sha1_sha2_swsudi (void)
{
    int ix;
    tam_act2_switch_simple_mode(0);

    /* Display SN */
    printf("\nSerial Number = ");
    for (ix =0; ix < PRODSN_NO; ix++) {
        printf("%c", prodSN[ix]);
    }
    printf("\nLength of Serial Number = %d.\n", (uint)strlen(prodSN));
    
    /* Display PID */
    printf("\nPID = ");    
    for (ix =0; ix < sizeof(prodName); ix++) {
        printf("%c", prodName[ix]);
    }
    printf("\nLength of PID = %d.\n", (uint)strlen(prodName));

    /* Menu mode */
    printf("\nOPTION - SHA1/SHA2/SWSUDI :\n");
    printf("0. SHA1 Cert\n");
    printf("1. SHA1 Key\n");
    printf("2. SHA2 Cert\n");
    printf("3. SHA2 Key\n");
    printf("4. SWSUDI2099 Cert\n");
    printf("5. SWSUDI2099 Key\n");
    which_act2_obj_param = getdec_answer("\nSelect an option >",0,0,5);
    return (PASSED);
}

/*-------------------------------------------------------------------
 *
 * Function : tam_act2_install_sha1_sha2_swsudi
 * Description: main entry point called to install wdc to act2 chip
 * INPUT:  a -- not used.
 * OUTPUT: return PASSED or FAILED
 * -------------------------------------------------------------------
 */
static int tam_act2_install_sha1_sha2_swsudi (int dummy)
{
    static uint8_t *act2_raw_obj_buffer = NULL;
    static uint16_t  act2_obj_len  = 0;

    if (select_sha1_sha2_swsudi() == FAILED) {
        return (FAILED);
    }

    act2_obj_len = getdec_answer("\nEnter SW SUDI Length", 0, 0, 0xffff);
    assert(act2_obj_len);

    if (act2_enter_sha1_sha2_swsudi_obj(&act2_raw_obj_buffer,act2_obj_len) == FAILED) {
        return (FAILED);
    }

    if (act2_install_sha1_sha2_swsudi_obj(&act2_raw_obj_buffer,act2_obj_len) == FAILED) {
        return (FAILED);
    }

    return (PASSED);
}
/*-------------------------------------------------------------------
 *  Function : tam_act2_display_sha1_sha2_swsudi 
 *  Description: Display  input  NamedObject.
 *  INPUT:  None
 *  OUTPUT: return PASSED or FAILED
 *  -------------------------------------------------------------------
 */
static int tam_act2_display_sha1_sha2_swsudi (int dummy)
{
    tam_lib_status_t status = 0;
    uint cleanup_status = 0;
    char obj_name[128];
    uint8_t admin_pin[TAM_PIN_LENGTH];
    uint32_t admin_session_id = 0;
    uint8_t num_admin_objs = 0;
    tam_lib_object_enum_couplet_t admin_obj_list[TAM_LIB_MAX_ENUM_OBJECTS];
    uint32_t admin_act2_obj_id = 0;
    uint8_t *read_acts_obj_buffer = NULL;
    uint16_t read_act2_obj_len = 0;
    uint32_t tmpObject = 0;
    uint16_t ix;

    if (select_sha1_sha2_swsudi() == FAILED) {
        return (FAILED);
    }

    memset(&admin_pin[0], 0x00, TAM_PIN_LENGTH);
    memset(&admin_obj_list[0], 0x00, sizeof(admin_obj_list));

    status = tam_act2_switch_simple_mode(0);
   
    if (status) {
        printf("Cannot switch device to simple mode. may already be in simple mode.");
    }

    /*** Sequence of operations: *
      *  1. generate admin pin   *
      *  2. open admin session   *
      *  3. Set Desired Endianess *
      *  4. Enable object naming *
      *  5. enumerate objects    *
      *  6. Search for the specified  object  *
      *  7. Display the data if present
      *  8. close admin session  *
      */


    /* generate the admin pin */
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Generating Admin Pin\n");
    }	

    status = tam_lib_generate_admin_pin(tam_handle, &admin_pin[0]);

    if (status != TAM_RC_OK) {
        cterr('f',0,"%s-%u failed, status=0x%x-%s ", __FUNCTION__, __LINE__,
               status, tam_lib_rc2string(status));
        return (FAILED);
    }

    /* open the admin session */
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Opening Admin session\n");
    }	

    status = tam_lib_session_init(tam_handle, 
                                  TAM_ADMIN_USER,
                                  &admin_pin[0],
                                  &admin_session_id);

    if (status != TAM_RC_OK) {
        cterr('f',0,"%s-%u failed, status=0x%x-%s ", __FUNCTION__, __LINE__,
               status, tam_lib_rc2string(status));
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Admin session opened, session id 0x%x\n", admin_session_id);
    }	

    status = tam_lib_omgr_set_endianness(tam_handle, TAM_LITTLE_ENDIAN);
    if (status != TAM_RC_OK) {
        cterr('f',0,"%s-%u enable little endian failed, status=0x%x-%s ",
               __FUNCTION__, __LINE__, status, tam_lib_rc2string(status));
        goto config_exit;
    }
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("enabled LE session id 0x%x\n", admin_session_id);
    }
    status = tam_lib_omgr_enable_object_naming(tam_handle, admin_session_id);
    if (status != TAM_RC_OK) {
        cterr('f',0,"%s-%u enable object naming failed, status=0x%x-%s ",
               __FUNCTION__, __LINE__, status, tam_lib_rc2string(status));
        goto config_exit;
    }

    /* enumerate existing admin objects */
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Enumerating Admin objects\n");
    }	

    status = tam_lib_object_enumerate(tam_handle, 
                                      admin_session_id,
                                      &num_admin_objs,
                                      &admin_obj_list[0]);

    if (status != TAM_RC_OK) {
        cterr('f',0,"%s-%u failed, status=0x%x-%s ", __FUNCTION__, __LINE__,
              status, tam_lib_rc2string(status));
        goto config_exit;
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Number of admin objects 0x%x\n", num_admin_objs);
    }	

    if (num_admin_objs > TAM_LIB_MAX_ENUM_OBJECTS) {
        printf("WARNING: Number of admin objects 0x%x is greater than expected "
               "max value.\n", num_admin_objs);
        printf("WARNING: Setting value to max value.\n");
    
        num_admin_objs = TAM_LIB_MAX_ENUM_OBJECTS;
        printf("Number of admin objects 0x%x\n", num_admin_objs);
    }

    /* search  existing objects */
    if (num_admin_objs == 0) {
        printf("No objects found  num_objects 0x%x\n", num_admin_objs);
        goto config_exit;
    }
    if (num_admin_objs) {
        for (tmpObject = 0; tmpObject < num_admin_objs; tmpObject++) {
            memset(obj_name, 0, sizeof(obj_name));
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("idx  0x%x id   0x%x \n\r", tmpObject,
                        admin_obj_list[tmpObject].object_id );
            }
            status = tam_lib_omgr_get_name_from_oid(tam_handle,
                                                    admin_session_id,
                                                    admin_obj_list[tmpObject].object_id,
                                                    obj_name);
            if (status != TAM_RC_OK) {
                if ((NVRAM)->diagflag & D_VERBOSE) {
                    printf("%s-%u unable to get name oid status=0x%x-%s\n\r", 
                           __FUNCTION__, __LINE__, status, tam_lib_rc2string(status));
                }
                continue;
            }
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("idx  0x%x id   0x%x name %s\n\r", tmpObject,
                        admin_obj_list[tmpObject].object_id, obj_name);
            }
            if (strcmp(obj_name, act2_obj_name[which_act2_obj_param])) {
                continue;
            }
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("found object 0x%x of 0x%x\n\r", tmpObject, num_admin_objs);
                printf("found  object %s of 0x%x\n\r",obj_name, num_admin_objs);
            }
            admin_act2_obj_id = admin_obj_list[tmpObject].object_id;
            break;
        }
        if (tmpObject == num_admin_objs) {
            printf(" %s Not found   \n\r", act2_obj_name[which_act2_obj_param]);
            goto config_exit;
        }
    }

    /* allocate memory for the read buffer */
    read_acts_obj_buffer = (uint8_t *)malloc(MAX_ACT2_OBJ_LEN);
    if (!read_acts_obj_buffer) {
        cterr('f',0,"Could not malloc a read buffer of size %d", 
               MAX_ACT2_OBJ_LEN);
        goto config_exit;
    }
    memset(&read_acts_obj_buffer[0], 0x00, MAX_ACT2_OBJ_LEN);

    /* read the act2 object */
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("read SUDI object\n");
    }	

    read_act2_obj_len = MAX_ACT2_OBJ_LEN;

    status = tam_lib_object_read(tam_handle, 
                                 admin_session_id,
                                 admin_act2_obj_id,
                                 &read_acts_obj_buffer[0],
                                 &read_act2_obj_len);

    if (status != TAM_RC_OK) {
        cterr('f',0,"s-%u failed, status=0x%x-%s ", __FUNCTION__, __LINE__,
               status, tam_lib_rc2string(status));
        free(read_acts_obj_buffer);
        goto config_exit;
    }
    printf("%s Length : %d Bytes\n\r",act2_obj_disp_name[which_act2_obj_param], 
            read_act2_obj_len);
    printf(" %s Start=>", act2_obj_disp_name[which_act2_obj_param]);

    for (ix = 0; ix < read_act2_obj_len; ix++) {
        printf("%02x", read_acts_obj_buffer[ix]);
    }
    printf("\n\r<=End %s \n\r", act2_obj_disp_name[which_act2_obj_param]);

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s Object\n", act2_obj_disp_name[which_act2_obj_param]);
    }	


    /* close the admin session */
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Closing Admin session\n");
    }	

    status = tam_lib_session_end(tam_handle, admin_session_id);

    if (status != TAM_RC_OK) {
        cterr('f',0,"s-%u failed, status=0x%x-%s ", __FUNCTION__, __LINE__,
               status, tam_lib_rc2string(status));
        free(read_acts_obj_buffer);
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Admin session closed\n");
    }	

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("SHA1/SHA2/SWSUDI Display completed\n");
    }

    /* free the read buffer */
    free(read_acts_obj_buffer);
    return (PASSED);
config_exit:
    cleanup_status = tam_lib_admin_logout(tam_handle, admin_session_id);
    if (cleanup_status) {
        cterr('f', 0, "Termiate session error");
    }
    /* if we fail, return status of failure before cleanup */
    return status;
}
#endif

/*-------------------------------------------------------------------
 *
 * Function : tam_lib_get_aikido_fpga_version
 * Description: This function is declared here for solely compilation
 *              purpose.
 *              This function implementation shall be declared in 
 *              platform code if get aikido fpga version is used
 * INPUT:  dummy -- not used.
 * OUTPUT: return TRUE or FALSE
 * -------------------------------------------------------------------
 */
tam_lib_status_t tam_lib_get_aikido_fpga_version(void *tmp, uint16_t *tmp2) 
    __attribute__((weak, alias("__tam_lib_get_aikido_fpga_version")));
tam_lib_status_t __tam_lib_get_aikido_fpga_version(void *tmp, uint16_t *tmp2) 
{
    printf("This function (%s) shouldn't be called", __func__);
    return (TRUE);
}
/*-------------------------------------------------------------------
 * Function : dump_aikido_fpga_version
 * Description: using tam_lib_get_aikido_fpga_version() to dump 
 *              AIKIDO fpga version
 * INPUT: None
 * OUTPUT: PASSED/FAILED
 * -------------------------------------------------------------------
 */
static int dump_aikido_fpga_version (void)  
{
    uint16_t tmp;
    tam_lib_status_t status;

    status = tam_lib_get_aikido_fpga_version(tam_handle, &tmp); 
    if (status != TAM_RC_OK) {
        printf("\n tam_lib_get_chip_info failed with status 0x%x\n", status);
        return (FAILED); 
    } else { 
        printf(" 0x%x\n", tmp); 
        return (PASSED); 
    }
}

/*-------------------------------------------------------------------
 * Function : get_aikido_chip_info
 * Description: using tam_lib_get_ckip_info() to get chip version. 
 * INPUT: tam library object
 * OUTPUT: PASSED/FAILED
 * -------------------------------------------------------------------
 */
unsigned int get_aikido_chip_info (tam_lib_chip_info_t *tmp) 
{
    tam_lib_status_t status;

    status = tam_lib_get_chip_info(tam_handle, tmp); 
    if (status != TAM_RC_OK) {
        printf("\n tam_lib_get_chip_info failed with status 0x%x\n", status);
        return (FAILED);
    } else { 
        return (PASSED); 
    }
}

/*-------------------------------------------------------------------
 * Function : dump_aikido_chip_info
 * Description: using tam_lib_get_ckip_info() to get chip version. 
 * INPUT: None
 * OUTPUT: PASSED/FAILED
 * -------------------------------------------------------------------
 */
static int dump_aikido_chip_info (void)
{
    tam_lib_chip_info_t chip_info; 
    int ix; 

    if (get_aikido_chip_info(&chip_info) == PASSED) {
        printf("chip_info.fw_version (hex)="); 
        for (ix = 0; ix < TAM_FW_VERSION_LENGTH; ix++) {
            printf(" %02x", chip_info.fw_version[ix]);
        }
        printf("\n"); 
        printf("version v%x.%x.%02x%02x\n", 
                (chip_info.fw_version[0] & 0xF0) >> 4, 
                chip_info.fw_version[0] & 0xF, 
                chip_info.fw_version[1], 
                chip_info.fw_version[2]); 
        return (PASSED);
    } else {
        return (FAILED);
    }

}

/*-------------------------------------------------------------------
 * Function : tam_lib_fw_threshold
 * Description: check chip fw version, return True if equal and later; 
 *              return False if fw version less. 
 * INPUT: fw_version 
 * OUTPUT: return True (need AIK) or False (doesn't need it)
 * -------------------------------------------------------------------
 */
unsigned int tam_lib_fw_threshold (unsigned int fw_version) 
{
    tam_lib_chip_info_t chip_info; 

    if (get_aikido_chip_info(&chip_info) == PASSED) {
        if (chip_info.fw_version[0] >= fw_version) {
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("chip fw 0x%x is larger and equal than threshold 0x%x\n",
                chip_info.fw_version[0], fw_version);
            }
            return (TRUE); 
        } else {
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("chip fw 0x%x is less than threshold 0x%x\n",
                chip_info.fw_version[0], fw_version);
            }
            return (FALSE); 
        }

    } else {
        printf("Failed to get FW version, return TRUE as default\n"); 
        return (TRUE); 
    }
}   

#ifdef AIKIDO_SUPPORT_AIK
/*-------------------------------------------------------------------
 * Function : tam_setup_aikido_aik_flag
 * Description: aikido version 2.2.0 or later need to program AIK
 * INPUT: *ptr_aikido_aik_flag 
 * OUTPUT: NONE 
 * -------------------------------------------------------------------
 */
static void tam_setup_aikido_aik_flag (boolean *ptr_aikido_aik_flag)
{
    if (is_tam_aikido_on()) { 
        if (tam_lib_fw_threshold(TAM_AIKIDO_AIK_REQUIRED_FW_VERSION) == TRUE) { 
            /* AIK program only for AIKIDO */
            *ptr_aikido_aik_flag = TRUE; 
        } else {
            /* if is_tam_aikido_on == FALSE, then it is act2 */
            *ptr_aikido_aik_flag = FALSE; 
        }
    } else {
        *ptr_aikido_aik_flag = FALSE; 
    }
}

/*-------------------------------------------------------------------
 *
 * Function : toggle_aikido_aik_flag 
 * Description: Toggle the aikido aik flag 
 * INPUT:  dummy -- not used.
 * OUTPUT: None
 * -------------------------------------------------------------------
 */
static void toggle_aikido_aik_flag (void)
{
    aikido_aik_flag ^= 1; 

    if (aikido_aik_flag == TRUE) { 
        printf("Enable AIKIDO AIK Flag \n"); 
    } else { 
        printf("Disable AIKIDO AIK Flag \n"); 
    }
}
#endif

/*-------------------------------------------------------------------
 *
 * Function : tam_lib_espi_read
 * Description: This function is declared here for solely compilation
 *              purpose.
 *              This function implementation shall be declared in 
 *              platform code if espi read is used
 * INPUT:  dummy -- not used.
 * OUTPUT: return TRUE or FALSE
 * -------------------------------------------------------------------
 */
tam_lib_status_t tam_lib_espi_read(void *tmp, uint32_t tmp2,
                                   uint16_t tmp3, uint8_t *tmp4)
    __attribute__((weak, alias("__tam_lib_espi_read")));
tam_lib_status_t __tam_lib_espi_read(void *tmp, uint32_t tmp2,
                                     uint16_t tmp3, uint8_t *tmp4)
{
    printf("This function (%s) shouldn't be called", __func__);
    return (TRUE);
}

/*---------------------------------------------------------------
 * Function : aikido_espi_read_util
 * Description: using tam_lib_espi_read() to read from flash. 
 * INPUT: None
 * OUTPUT: PASSED/FAILED
 * --------------------------------------------------------------
 */
unsigned int aikido_espi_read_util (void) 
{
    tam_lib_status_t status;
    uint32_t addr_offset = 0x0, ix;
    uint16_t data_len;
    uint8_t tmp_buf[WR_DATA_LEN] = {0};


    addr_offset = gethex_answer("Enter Offset : ", 0x0, 0x0, MAX_ADDR_OFFSET); 
    data_len =  gethex_answer("Enter len : ", 0x0, 0x0, WR_DATA_LEN); 
   
    status = tam_lib_espi_read(tam_handle, addr_offset, data_len, tmp_buf);
    if (status != TAM_RC_OK) {
        printf("\n tam_lib_get_chip_info failed with status 0x%x\n", status);
        return (FAILED);
    }

    for (ix = 0; ix < data_len; ix ++) {
        if (ix % 16 == 0) { 
            printf("\n");
        }
        printf("%02x ", tmp_buf[ix]);
    }
    printf("\n");

    return (PASSED); 
}

#ifdef AIKIDO_WRITE_UTIL
/*---------------------------------------------------------------
 * Function : aikido_espi_write_header_util
 * Description: using tam_lib_espi_write() to write to flash. 
 * INPUT: None
 * OUTPUT: None
 * --------------------------------------------------------------
 */
unsigned int aikido_espi_write_header_util (void) 
{
    tam_lib_status_t status;
    uint32_t addr_offset = 0x0, ix;
    uint16_t data_len;
    uint8_t tmp_buf[WR_DATA_LEN] = {0};


    addr_offset = gethex_answer("Enter Offset : ", 0x0, 0x0, MAX_ADDR_OFFSET); 
    data_len =  gethex_answer("Enter len : ", 0x0, 0x0, WR_DATA_LEN); 

    status = tam_lib_espi_read(tam_handle, 0, WR_DATA_LEN, tmp_buf);
    if (status != TAM_RC_OK) {
        printf("\n tam_lib_espi_read failed with status 0x%x\n", status);
        return (FAILED);
    }

    for (ix = addr_offset; ix < (addr_offset + data_len); ix++) {
        tmp_buf[ix] = gethex_answer("Enter data", 0, 0, 0xffff);
    }
	
    printf("Please check if write data correct:\n");
    for (ix = addr_offset; ix < (addr_offset + data_len); ix++) {
        if (ix % 16 == 0) 
            printf("\n");

        printf("%02x ", tmp_buf[ix]);
    }
    printf("\n");
    if (getc_answer("\nAre you sure you want to continue? (y/n)", "yn", 'y') == 'n') {
        printf("\nNo action taken.");
        return (PASSED);
    }
	
    status = tam_lib_espi_erase_4k(tam_handle, (addr_offset & ~0xfff));
    if (status != TAM_RC_OK) {
        printf("\n tam_lib_espi_erase_4k failed with status 0x%x\n", status);
        return (FAILED);
    }
	
    status = tam_lib_espi_write(tam_handle, 0, WR_DATA_LEN, tmp_buf);
    if (status != TAM_RC_OK) {
        printf("\n tam_lib_espi_write failed with status 0x%x\n", status);
        return (FAILED);
    }

    return (PASSED); 
}
#endif

/*-------------------------------------------------------------------
 *
 * Function : is_tam_aikido_on_wrapper
 * Description: wrapper function for is_tam_aikido_on
 * INPUT:  NONE
 * OUTPUT: NONE
 * -------------------------------------------------------------------
 */
void is_tam_aikido_on_wrapper (void)
{
    is_tam_aikido_on();
}
/*
 * Function: tam_act2_rng_test 
 *
 * Description : This function is used to verify if “Aikido RNG” generate random number. 
 *               Diag verify it by call TAM api tam_lib_trand_read() to generate true random bits twice, 
 *               and compared two data buffers with random data return from the API. 
 *               If data contents are same from these two data buffer, "Aikio RNG" has issue and print failure message. 
 *                
 * INPUT:  dummy -- not used
 *
 * Output: PASSED/FAILED
 */
static int tam_act2_rng_test (int dummy)
{
    tam_lib_status_t status;
    int ix, jx, test_time = RNG_TEST_LOOP;
    uint16_t obj_size = RNG_SIZE;   /* anything between 1 and TAM_MAX_RANDOM_LENGTH (224) */ 
    uint8_t rng_buffer1[TAM_MAX_RANDOM_LENGTH];
    uint8_t rng_buffer2[TAM_MAX_RANDOM_LENGTH];
    uint admin_session_id = 0;
    uint cleanup_status = 0;
    
    if (!is_tam_aikido_on()) {
        printf("Discrete ACT2 didn't support RNG");
        return(FAILED);
    }

    testname("RNG Health");
    prpass(testpass, " ");
    status = tam_lib_admin_login(tam_handle, &admin_session_id);
    if (status != TAM_RC_OK) {
        cterr('f', 0, "\nact2l_gen_admin_credential status error %02x\n",
               __FUNCTION__, status);
        return(FAILED);
    }
    /* RNG health checks many times (test_time) , 
     * Each time generate 2 random number, and compare result should different
     */
    for (ix = 0; ix < test_time; ix++) {
        memset(rng_buffer1, 0, sizeof(rng_buffer1));
        memset(rng_buffer2, 0, sizeof(rng_buffer2));
        obj_size += RNG_SIZE_INCREASE; /* Each test increases size by RNG_SIZE_INCREASE  */       
        /* TAm Library API User Guide : The obj_size length <= TAM_MAX_RANDOM_LENGTH */
        if (obj_size > TAM_MAX_RANDOM_LENGTH) {
            cterr('f', 0, "obj_size : %d over the limit size : %d", obj_size, TAM_MAX_RANDOM_LENGTH);
            goto config_exit;
        } 
        /* Initiates True random bit generation function health checks */
        status = tam_lib_trand_check(tam_handle, admin_session_id);
        if (status != TAM_RC_OK) {
            cterr('f', 0, "#%d %s tam_lib_trand_check status error %02x\n", ix,
                   __FUNCTION__, status);
            goto config_exit;
        }
        /* Generate 1'st random number */
        status = tam_lib_trand_read(tam_handle, obj_size, rng_buffer1);
        if (status != TAM_RC_OK) {
            /* handle error */
            cterr('f', 0, "#%d %s Generate random number status error %02x\n", ix,
                   __FUNCTION__, status);
            goto config_exit;
        }
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("\n\n#%d First Random Number size=%d\n", ix, obj_size);
            for (jx = 0; jx < obj_size; jx++) {
                printf("%02x", rng_buffer1[jx]);
            }
        }
        /* Generate 2'nd random number */
        status = tam_lib_trand_read(tam_handle, obj_size, rng_buffer2);
        if (status != TAM_RC_OK) {
            /* handle error */
            cterr('f', 0, "#%d %s Generate random number status error %02x\n", ix,
                   __FUNCTION__, status);
            goto config_exit;
        }
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("\n#%d Second Random Number size=%d\n", ix, obj_size);
            for (jx = 0; jx < obj_size; jx++) {
                printf("%02x", rng_buffer2[jx]);
            }
        }
        /* Compare two RNG buffers, they should different. */
        if (!memcmp(rng_buffer1, rng_buffer2, obj_size)) {
            printf("\n\n#%d First Random Number size=%d\n", ix, obj_size);
            for (jx = 0; jx < obj_size; jx++) {
                printf("%02x", rng_buffer1[jx]);
            }
            printf("\n#%d Second Random Number size=%d\n", ix, obj_size);
            for (jx = 0; jx < obj_size; jx++) {
                printf("%02x", rng_buffer2[jx]);
            }
            cterr('f', 0, "\nTest %d times, Generated the same random number, "
                  "RNG test error \n", ix);
            status = FAILED;
            goto config_exit;
        }
    }
    printf("\nTrue Random Number Generation Health Check Passed\n");
config_exit:
    cleanup_status = tam_lib_admin_logout(tam_handle, admin_session_id);
    if (cleanup_status) {
        cterr('f', 0, "Terminate session logout error on %s \n", __FUNCTION__);
    }
    if (status != TAM_RC_OK) {
        cterr('f',0,"%s-%u failed, status=0x%x", __FUNCTION__, __LINE__, status);
    }
    return (status); 

}
/*
 *------------------------------------------------------------------
 * $Log: tam_act2_utils.c,v $
 * Revision 1.30  2021/06/09 08:54:23  iachang
 * CSCvo33419-361 : Closed the admin login session if obj_size more than TAM_MAX_RANDOM_LENGT.
 *                  Due to the boundary check, assign the rng_buffer size with TAM_MAX_RANDOM_LENGT.
 *
 * Revision 1.29  2021/04/14 03:23:13  iachang
 * CRRaa88551 : Add Aikido RNG test
 *
 * Revision 1.28  2021/04/13 07:08:04  iachang
 * CRRaa88553 : Added compilation flag to support WDC OBJECT NAME
 *              Supported multi Consent Token Dev Key object.
 *
 * Revision 1.27  2021/02/24 03:46:12  xiaolaya
 * Fix bug for Switzer-Carrier SM Daughter NIM Daughter VM cookie
 *
 * Revision 1.26  2021/01/12 06:47:17  xiaolaya
 * swizter-carrier daughter card eeprom access bug fix2
 *
 * Revision 1.25  2021/01/12 06:31:58  iachang
 * CSCvw97109 - Fixed Promethium IOS complains WDC length invalid issue
 *
 * Revision 1.24  2021/01/12 04:04:57  xiaolaya
 * switzer-carrier daughter card eeprom access bug fix
 *
 * Revision 1.23  2020/08/12 05:38:20  iachang
 * CSCvu36133 : Fixed WDC install cause consent token install and remove DEV key more complicated issue
 *
 * Revision 1.22  2020/01/09 01:01:52  jiajliu
 * Merge Curie 2RU to main trunk
 *
 * Revision 1.21  2019/12/12 07:57:25  yungchen
 * Add define flag to fix diffenence tam library build failed issue
 *
 * Revision 1.20  2019/12/11 10:10:23  lucywang
 * Merged Nanook to main trunk
 *
 * Revision 1.19  2019/08/06 06:56:06  alpeng
 * merge curie, switzer and nightwatch to trunk
 *
 * Revision 1.18  2019/07/11 12:34:40  alicehua
 * Collapse Nutella codes into main trunk
 *
 * Revision 1.17  2019/06/17 01:28:14  mikech2
 * Complete PRRQ#4758652 review, merge to main trunk
 *
 * Revision 1.16  2019/06/14 03:58:52  mikech2
 * Collapse katar-branch00 to Main Trunk
 
 * Revision 1.15.14.3  2019/04/26 09:00:55  harrchan
 * Base on review comments to clean up code
 *
 * Revision 1.15.14.2  2019/04/09 06:56:57  harrchan
 * Add program/verify aikido dev key
 *
 * Revision 1.15.14.1  2019/03/08 05:45:33  harrchan
 * 1.Support diag to access Aikido FPGA with SPI and I2C interface 2.Add utility for Aikido FPGA upgrade
 *
 * Revision 1.15  2018/11/12 06:36:32  iachang
 * CSCvm79684:Fixed Aikido WDC USER ID delete issue with TAM lib. newer than V2.10.2.
 *
 * Revision 1.13.8.4  2019/06/14 03:12:12  mikech2
 * Update tam_act2_utils.c base on PRRQ#4758652
 *
 * Revision 1.13.8.3  2019/06/13 06:56:58  mikech2
 * Merge tam_act2_utils.c revision 1.15 from main trunk
 *
 * Revision 1.13.8.2  2019/04/17 17:42:35  iachang
 * Fixed compile warning.
 *
 * Revision 1.13.8.1  2018/10/25 11:16:39  iachang
 * Supported SHA1/SHA2/SWSUDI program.
 *
 * Revision 1.14  2018/08/14 22:47:10  iachang
 * CSCvk51378: Supportted Act2 SUDI 2099, please refer to Viper Makefile.
 *
 * Revision 1.13  2018/05/18 09:24:49  alpeng
 *  Neptune merge to trunk with tag <neptune-branch-0518>
 *
 * Revision 1.12  2018/02/09 09:11:18  hondwang
 * Merge Star branch star-branch-c9xx to main trunk
 *
 * Revision 1.11.10.2  2018/01/23 09:03:17  iachang
 * CSCvh32961 : Display WDC and Dummy data with Act2 chip
 *
 * Revision 1.11.10.1  2018/01/20 06:29:55  hondwang
 * prepare merge star-branch-c9xx to main trunk
 *
 * Revision 1.11  2017/10/19 13:53:00  palin2
 * Fixed CSCvg25685, TSN: TAM FW wiping out the WDC object during
 * power-cycle-with-software-running testing.
 *
 * Revision 1.10.4.5  2017/12/13 09:31:58  hondwang
 * Check WDC dummmy workaround function for HW reuqest
 *
 * Revision 1.10.4.4  2017/09/08 01:36:19  harrchan
 * Support discrete ACT2 offline test
 *
 * Revision 1.10.4.3  2017/09/06 06:48:19  iachang
 * Support generate eCSKMP files
 *
 * Revision 1.10.4.2  2017/08/17 12:19:43  hondwang
 * Set AIKIDO ACT2 by default
 *
 * Revision 1.10.4.1  2017/08/15 14:04:55  hondwang
 * star branch c9xx initial check in
 *
 * Revision 1.10  2017/08/02 14:21:32  steja
 * Support TSN-H/M platform code
 *
 * Revision 1.9.22.1  2017/07/29 03:40:50  steja
 * tsn-branch5 synced with Maintrunk repositories
 *
 * Revision 1.9.20.3  2017/07/21 09:17:34  iachang
 * clean up code
 *
 * Revision 1.9.20.2  2017/07/21 07:33:08  steja
 * move wifi_reset_init to platform_cookie.c
 *
 * Revision 1.9.20.1  2017/07/20 13:37:58  steja
 * tsn-branch4 merge with maintrunk
 *
 * Revision 1.9.2.7.2.1.4.1  2017/07/20 22:38:56  tirawan
 * CSCvf15334: Suppress printk message as we don't want fgets gets interrupted and results in UART buffer overflow
 *
 * Revision 1.9.2.7.2.1  2017/03/08 14:31:47  steja
 * Update ACT2 programming for P2A
 *
 * Revision 1.9.2.7  2016/11/25 07:54:43  steja
 * Add Debug flag compiler for ACT2 Aikido Reset
 *
 * Revision 1.9.2.6  2016/10/05 09:33:04  steja
 * Support generate eCSKMP files
 *
 * Revision 1.9.2.5  2016/09/19 09:05:50  steja
 * Fix Compile error O2 and Tachi-bmc
 *
 * Revision 1.9.2.4  2016/09/13 14:35:39  steja
 * Commit Aikido / TAM Mailbox code
 *
 * Revision 1.9.2.3  2016/09/01 06:36:10  iachang
 * Supported Aikido cookie access
 * Supported Aikido ACT-2 utilities and programming
 *
 * Revision 1.9.2.2  2016/08/09 09:47:47  iachang
 * Supported FPGA/Aikido firmware upgrade.
 *
 * Revision 1.9.2.1  2016/06/30 08:31:47  steja
 * Fix compiler issue
 *
 * Revision 1.9  2016/06/21 19:40:28  huanngo
 * Add code to allow only 0xc1 and 0xc2 for programming SUDI/WDC
 *
 * Revision 1.8  2016/06/06 18:30:39  huanngo
 * Add code to support programming SUDI/WDC with Chassis S/N
 *
 * Revision 1.7.2.6  2018/05/17 10:50:20  alpeng
 *  sync with trunk <trunk-051618>
 *
 * Revision 1.7.2.5  2018/05/08 23:43:23  ptong
 * Use tam_lib_mfg_install_cert_and_chain() to install HARSA_SUDI (leverage tam_aikido code).
 *
 * Revision 1.7.2.4  2018/05/04 23:42:35  ptong
 * Call tam_lib_authentication_udi with HARSA_SUDI to support 2099 SUDI
 *
 * Revision 1.7.2.3  2018/05/01 22:16:41  ptong
 * Add -DTAM_2099_SUDI compilation flag
 *
 * Revision 1.7.2.2  2017/04/05 06:45:44  leschen
 * Sync with <ng_diag-tag-032917>
 *
 * Revision 1.7.2.1  2017/03/24 22:25:21  ptong
 * Merge new ACT2 code from main trunk that allow chassis SN tobe used in SUDI/WDC. Increase version to V1.1.4
 *
 * Revision 1.12  2018/02/09 09:11:18  hondwang
 * Merge Star branch star-branch-c9xx to main trunk
 *
 * Revision 1.11.10.2  2018/01/23 09:03:17  iachang
 * CSCvh32961 : Display WDC and Dummy data with Act2 chip
 *
 * Revision 1.11.10.1  2018/01/20 06:29:55  hondwang
 * prepare merge star-branch-c9xx to main trunk
 *
 * Revision 1.11  2017/10/19 13:53:00  palin2
 * Fixed CSCvg25685, TSN: TAM FW wiping out the WDC object during
 * power-cycle-with-software-running testing.
 *
 * Revision 1.10.4.5  2017/12/13 09:31:58  hondwang
 * Check WDC dummmy workaround function for HW reuqest
 *
 * Revision 1.10.4.4  2017/09/08 01:36:19  harrchan
 * Support discrete ACT2 offline test
 *
 * Revision 1.10.4.3  2017/09/06 06:48:19  iachang
 * Support generate eCSKMP files
 *
 * Revision 1.10.4.2  2017/08/17 12:19:43  hondwang
 * Set AIKIDO ACT2 by default
 *
 * Revision 1.10.4.1  2017/08/15 14:04:55  hondwang
 * star branch c9xx initial check in
 *
 * Revision 1.10  2017/08/02 14:21:32  steja
 * Support TSN-H/M platform code
 *
 * Revision 1.9.22.1  2017/07/29 03:40:50  steja
 * tsn-branch5 synced with Maintrunk repositories
 *
 * Revision 1.9.20.3  2017/07/21 09:17:34  iachang
 * clean up code
 *
 * Revision 1.9.20.2  2017/07/21 07:33:08  steja
 * move wifi_reset_init to platform_cookie.c
 *
 * Revision 1.9.20.1  2017/07/20 13:37:58  steja
 * tsn-branch4 merge with maintrunk
 *
 * Revision 1.9.2.7.2.1.4.1  2017/07/20 22:38:56  tirawan
 * CSCvf15334: Suppress printk message as we don't want fgets gets interrupted and results in UART buffer overflow
 *
 * Revision 1.9.2.7.2.1  2017/03/08 14:31:47  steja
 * Update ACT2 programming for P2A
 *
 * Revision 1.9.2.7  2016/11/25 07:54:43  steja
 * Add Debug flag compiler for ACT2 Aikido Reset
 *
 * Revision 1.9.2.6  2016/10/05 09:33:04  steja
 * Support generate eCSKMP files
 *
 * Revision 1.9.2.5  2016/09/19 09:05:50  steja
 * Fix Compile error O2 and Tachi-bmc
 *
 * Revision 1.9.2.4  2016/09/13 14:35:39  steja
 * Commit Aikido / TAM Mailbox code
 *
 * Revision 1.9.2.3  2016/09/01 06:36:10  iachang
 * Supported Aikido cookie access
 * Supported Aikido ACT-2 utilities and programming
 *
 * Revision 1.9.2.2  2016/08/09 09:47:47  iachang
 * Supported FPGA/Aikido firmware upgrade.
 *
 * Revision 1.9.2.1  2016/06/30 08:31:47  steja
 * Fix compiler issue
 *
 * Revision 1.9  2016/06/21 19:40:28  huanngo
 * Add code to allow only 0xc1 and 0xc2 for programming SUDI/WDC
 *
 * Revision 1.8  2016/06/06 18:30:39  huanngo
 * Add code to support programming SUDI/WDC with Chassis S/N
 *
 * Revision 1.7  2016/04/20 07:30:52  benchen2
 * fix o2/uath without isp card
 *
 * Revision 1.6  2016/04/20 07:03:33  benchen2
 * merge tachi_branch to maintrunk
 *
 * Revision 1.5.2.2  2016/04/18 07:00:47  benchen2
 * according to prrq fix isp define
 *
 * Revision 1.5.2.1  2016/01/26 06:27:55  benchen2
 * add daughter card ACT2 programming
 *
 * Revision 1.5  2015/05/13 02:34:51  iachang
 * Supported common TAM library include.
 *
 * Revision 1.4  2015/02/13 23:28:57  kwochan
 * Added comment why ECC SUDI is commented out
 *
 * Revision 1.3  2015/02/13 06:29:14  iachang
 * Supported and commented out the ECC SUDI.
 *
 * Revision 1.2  2015/02/13 02:33:35  iachang
 * Supported Act2 TAM library
 *
 * Revision 1.1.4.14  2015/02/10 06:22:44  iachang
 * Avoided impact production MFG script. Changed back WDC passed->WCD passed,Reset Quack chip->Reset MB Quack chip,
 *
 * Revision 1.1.4.13  2015/02/02 03:01:18  iachang
 * Correct WDC promtp string
 *
 * Revision 1.1.4.12  2015/01/29 22:56:25  iachang
 * Supported WDC install
 *
 * Revision 1.1.4.11  2015/01/29 08:40:22  iachang
 * Dynamic adjustment the CLIIP length to replace hard code.
 *
 * Revision 1.1.4.10  2015/01/17 02:39:38  iachang
 * Upgrade TAM library. Built-Jan 9 2015
 *
 * Revision 1.1.4.9  2015/01/15 03:23:04  iachang
 * CSCus28903: Hidden "Authenticate (n/a)/display cert" menu, but still can execute.
 *
 * Revision 1.1.4.8  2015/01/15 03:08:09  iachang
 * Install SUDI with RSA and display TAM MFG error information.
 *
 * Revision 1.1.4.7  2015/01/15 03:04:40  iachang
 * Skip the CLIIP length check
 *
 * Revision 1.1.4.6  2015/01/07 11:35:16  iachang
 * Fixed whitespace debug mode issue.
 *
 * Revision 1.1.4.5  2015/01/07 10:41:28  iachang
 * Add debug mode to strip off the PID whitespace
 *
 * Revision 1.1.4.4  2015/01/06 07:24:44  iachang
 * The CLIIP is larger on 1.3a than 1.3. There should not be any hardcoding of size checks for variable length data
 *
 * Revision 1.1.4.3  2014/12/29 06:38:27  iachang
 * Check PID Whitespace within tam_act2_authenticate_udi() function
 * Display !!! Whitespace is not allowed after PID.
 *
 * Revision 1.1.4.2  2014/12/17 08:36:24  hondwang
 * sync with maintrunk tag ovld-juno-tag-121714
 *
 * Revision 1.1.2.2  2014/12/15 03:16:14  iachang
 * Refer TAM lib. headfile from /auto/sp-engops/diags/pld/act2lite/x86/victory
 *
 * Revision 1.1.2.1  2014/12/12 08:39:04  iachang
 *
 * Move tam_act2_utils.c from common/src/overlord/ to common/src/
 * Move tam_lib_manufacturing.h/tam_library.h from common/src/overlord/ to common/include/
 *
 * Revision 1.1.2.2  2014/12/12 02:25:24  iachang
 * Modify alert message of Authenticate item
 *
 * Revision 1.1.2.1  2014/12/11 09:52:12  iachang
 * Supported Longer PID With TAM lib.
 *------------------------------------------------------------------
 * $Endlog$
 */

