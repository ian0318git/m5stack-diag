/* $Id: act2_utils.c,v 1.9 2015/01/14 08:52:59 danchung Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/act2_utils.c,v $
 *------------------------------------------------------------------
 * act2_utils.c
 * 
 * This file contains code developed for the ACT2 (Ruby) device.  Some of the printfs are very important.
 * the string output is captured by the script so if you modify the output, you may break manufacturing script.
 * for example, note that getdec_answer defaults answer is 0.  Manufacturing script expects "0" to show up on
 * the screen. if the default value is changed, the script will break.
 * Also, under linux, the max stdin buffer is 4K (we don't understand why buffer is small). so we need to break
 * some of the stdin queries into multiple queries, with each query taking in at most 4K data.
 *
 * Copyright (c) 2014-2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 * Author: Alan O'Sullivan
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
#include "nmc93c46.h"
#include "cross_platform.h"
#include "smart_cookie.h"
#include "cookie_4.h"
#include "proto.h"
#include "act2l_typedef.h"
#include "act2_utils.h"
#include "i2c_api.h"
#include "platform_cookie.h"
#include "mfg_api.h"
#include "object.h"
//#include "mfg.h"
#include "platform_i2c.h"
#include "dash_fpga.h"
#include "error.h"
#include "object.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
//#include <openssl/sha.h>
//#include "/usr/include/openssl/sha.h"

/* from manufac script data size is 958. translate to assic that 958 * 3 each data
   has 2 bytes (ie, 9e), plus space, so bufer should be at least 958 *3. */
#define SUDI_MAX_SIZE ((WDC_SIZE * 3 ) + 100)
extern int print_cookie(int argc,char *argv[]);
extern char atoh(char c);
//#define ACT_BUFFER_SIZE 512
extern int getline(char *buffer, int bufsiz);
int act2_i2c_debug = 0;

/************************************************
Below is the list of error conditions returned from library:
"ACT2_ERR_CKSUM_BAD",   //0x0B
"ACT2_ERR_CMD_UNKNOWN",   //0x0C
"ACT2_ERR_CMD_LENGTH",   //0x0D
"ACT2_ERR_PARAMETER_INVALID", //0x0E
"ACT2_ERR_ALGORITHM_INVALID", //0x0F
"ACT2_ERR_PERMISSION_NOT",  //0x10
"ACT2_ERR_SESSION_GEN_FAIL", //0x11
"ACT2_ERR_SESSION_NOT_AVAIL", //0x12
"ACT2_ERR_SESSION_IS_OPEN",  //0x13
"ACT2_ERR_SESSION_INVALID",  //0x14
"ACT2_ERR_HANDLE_INVALID",  //0x15
"ACT2_ERR_SORW_PENDING",  //0x16
0,        //0x17
"ACT2_ERR_USERID_UNKNOWN",  //0x18
"ACT2_ERR_USERID_INVALID",  //0x19
"ACT2_ERR_USERID_EXISTS",  //0x1A
//serial number C1; product id; : 0x CB
0,        //0x1B
0,        //0x1C
0,        //0x1D
0,        //0x1E
0,        //0x1F
"ACT2_ERR_BOUNDS_CHECK",  //0x20
"ACT2_ERR_EEPROM_SPACE",  //0x21
"ACT2_ERR_EEPROM_WRITE",  //0x22
"ACT2_ERR_ALGORITHM",   //0x23
"ACT2_ERR_KEY_LENGTH",   //0x24
"ACT2_ERR_OBJECT_DIRTY",  //0x25
"ACT2_ERR_OBJECT_GENERAL",  //0x26
"ACT2_ERR_OBJECT_IN_ROM",  //0x27
"ACT2_ERR_OBJECT_LENGTH",  //0x28
"ACT2_ERR_OBJECT_PERMISSION", //0x29
"ACT2_ERR_OBJECT_PRIVATE",  //0x2A
"ACT2_ERR_OBJECT_TYPE",   //0x2B
"ACT2_ERR_PROCESS_FAIL",  //0x2C
"ACT2_ERR_RAM_SPACE",   //0x2D
"ACT2_ERR_VERIFY_FAIL",   //0x2E
"ACT2_ERR_CODE_FAIL",   //0x2F
"ACT2_ERR_LIBRARY",    //0x30
"RC_WRITE_ERROR",    // 0x31
"RC_READ_ERROR",    // 0x32
"RC_READ_CMD_ERROR",   // 0x33 
"RC_READ_CHKSUM_ERROR",   //  0x34
"ACT2_STATUS_ONLY"    //  0x35   

***********/

static uchar cookie[COOKIE_SIZE_512];    
static char prodSN[20];
static char prodName[256];

/* version 0x17 (or 23 dec)  or above is act2; below is act1 */
static int   act2_chip_version = 0x17;  /*hardcode for now until quake_version is fixed */
static sc_context cont;
static dev_if_info_t dev_if;
static int act2_toggle_debug_flag(int dummy);
static int  act2_test(int);
static int  act2_enumerate(int);
static int  act2_auth_exist(void);
static int  act2_authenticate(int);
static ACT2_STATUS act2_platform_object_burn_in_test(int dummy);
static int user_act2l_read_sudi(void);
extern int smart_cookie_read_x(sc_context *con_p, ushort size);
extern int quack_version(sc_context *con);
int  act2_install_wdc(int);
/* This is the session id that is generated during mfg login */
static u4 session_id = 0;
static uint act2_simple_mode[MAX_DEVICE_TYPE];
static int device_type = MOTHER_BOARD;

int act2_reset(int);

/* 
 * Sub Menu used for ACT-2 utility.
 */
submenu_xtable_t act2_submenu_table[] = {
    {"Switch simple mode",         (PFT)act2_switch_simple_mode,  0, 0, (type_t(*)())0,0,(type_t(*)())0,0},
    {"Get serial number",          (PFT)act2_get_serial_num,      0, 0, (type_t(*)())0,0,(type_t(*)())0,0},
    {"Display session id",         (PFT)act2_get_session_id,      0, 0, (type_t(*)())0,0,(type_t(*)())0,0},
    {"Manufacturing login",        (PFT)act2_mfg_login,           0, 0, (type_t(*)())0,0,(type_t(*)())0,0},
    {"Install identity",           (PFT)act2_install_CLIIP,       0, 0, (type_t(*)())0,0,(type_t(*)())0,0},
    {"Verify identity",            (PFT)act2_install_SUDI,        0, 0, (type_t(*)())0,0,(type_t(*)())0,0},
    {"Authenticate (n/a)/display cert",               (PFT)act2_authenticate,        0, MF_HIDDEN_EXE, (type_t(*)())act2_auth_exist,0,(type_t(*)())0,0},
    {"Install WDC",                (PFT)act2_install_wdc,        0, 0, (type_t(*)())0,0,(type_t(*)())0,0},
    {"Close manufacturing login",  (PFT)act2_close_mfg_login,     0, 0, (type_t(*)())0,0,(type_t(*)())0,0},
    {"Display CSKMP",              (PFT)act2_get_cskmp,           0, 0, (type_t(*)())0,0,(type_t(*)())0,0},
    {"Display SCC Version",        (PFT)act2_version,             0, 0, (type_t(*)())0,0,(type_t(*)())0,0},
    {"ACT2 <--> Diags comminucation test",   (PFT)act2_test,      0, 0, (type_t(*)())0,0,(type_t(*)())0,0},
    {"Check for identity",         (PFT)user_act2l_read_sudi,     0, 0, (type_t(*)())0,0,(type_t(*)())0,0},
    {"Enumerate object",         (PFT)act2_enumerate,     0, 0, (type_t(*)())0,0,(type_t(*)())0,0},
    {"burn in test",         (PFT)act2_platform_object_burn_in_test,     0, MF_CONTINUOUS | MF_DOGRP,
     (type_t(*)())0, MF_CONTINUOUS | MF_DOGRP, (type_t(*)())0,0},
    {"Reset MB Quack chip",         (PFT)act2_reset,     0, MF_CONTINUOUS | MF_DOGRP,
     (type_t(*)())0, MF_CONTINUOUS | MF_DOGRP, (type_t(*)())0,0},
    {"toggle act2 debug flag",   (PFT)act2_toggle_debug_flag, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0,0},
};

#define ACT2_SUBMENU_TABLE_SIZE (sizeof(act2_submenu_table) / \
                                 sizeof(submenu_xtable_t))

/* 
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t act2_primary_items[ACT2_SUBMENU_TABLE_SIZE +
                                  MAX_BASE_ITEMS];
static mitem_t act2_secondary_items[ACT2_SUBMENU_TABLE_SIZE +
                                    MAX_BASE_ITEMS];

menuinfo_t act2_subtest_menu = {
    "%s utility Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    act2_primary_items,
};
static menuinfo_t *act2_submenup = &act2_subtest_menu;

/*-------------------------------------------------------------------
 *
 * Function : getline_act2
 * Description: wrapper function to read a line of string from terminal
 * INPUT:  size -- lenght of string to read
 * OUTPUT: buf -- string read from terminal
 *         return PASSED or FAILED
 * -------------------------------------------------------------------
 */
static int
getline_act2 (char *buf, int size)
{
    int xx;

    if (size >= (getpagesize()-1)) {
        printf("read size is %d; must be less than %d \n", size, getpagesize()-1);
        assert(!"*******\n\nrquest read size is too big!!!****\n\n");
    }
        
    fgets(buf, size, stdin);
    xx = strlen(buf);
    if (xx && (buf[xx-1] == '\n')) {
        buf[xx-1] = '\0';
        xx--;
    }
    
    return xx;
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
act2_get_n2g_i2c_if (void)
{
    n2g_i2c_if_t *i2c;

    assert(cont.dev_if_p);
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
    /* need to set devicet type */
    device_type = cont.type;

    memcpy(&cont, con, sizeof(sc_context));
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
    uint16_t id = 0;
    char err[80];
    
    act2_reset(0);

    if (get_cookie_id(cont.slot, cont.type, cookie, &id, err)==FAILED) {
        cterr('f', 0, "act2 program -- unable to read cookie id.");
        return FAILED;
    }

    memset(prodName, '\0', sizeof(prodName));
    get_pid(cookie, prodName);
    
    memset(prodSN, '\0', sizeof(prodSN));
    get_pcb_serial(cookie, prodSN);
    
    //    slot = atoh(choice[1]);
    act2_simple_mode[device_type] = 0;

    //      get_cms_data_for_sudi(INTERNAL_MODULE_SLOT, act2_device_slot_num);
    build_primary_submenu(act2_submenu_table, ACT2_SUBMENU_TABLE_SIZE,
                          "ACT2", &act2_submenup);
    build_secondary_submenu(act2_submenu_table,
			    ACT2_SUBMENU_TABLE_SIZE,
			    act2_secondary_items);

    if (act2_items_executed) {
        printf("\n Only submenu support at this time");
    } else {
        menu(&act2_subtest_menu, act2_secondary_items, '\0');
    }

    return PASSED;
}

/*
 * Function: act2_switch_simple_mode
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
act2_switch_simple_mode (int dummy)
{
    int ret;
    
    if ((ret=act2l_switch_2_simple_mode((void *)&cont))) {
        return (FAILED);
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
 * Function: act2_get_serial_num
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
act2_get_serial_num (void)
{
    //    sc_context cont;
    //    dev_if_info_t dev_if;
    uchar chip_serial_number[32];
    int i;

    memset(chip_serial_number, '\0', sizeof(chip_serial_number));

    if (act2_chip_version < 0x15) {
        printf("\n *** ERROR: Cannot perform this function on an device.");
        printf("\n SCC Version on chip is %x.\n",act2_chip_version);
        return(FAILED);
    }
    
    act2l_get_chip_serial_number((void *)&cont, chip_serial_number);

    printf("\n ACT-2: serial number =");
    if (strlen((char *)chip_serial_number)) {
        for (i = 0; i < 32; i++) {
            printf("%02x", chip_serial_number[i]);
        }
    }

    return(PASSED);
}

/*
 * Function: act2_get_session_id
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
act2_get_session_id (void)
{
    printf("\n session id = %#x\n", session_id);
    return(PASSED);
}

/*
 * Function: act2_get_cskmp (DEBUG ONLY)
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
act2_get_cskmp (void)
{
    uchar cskmp[1000];
    uchar flag;

    act2l_get_cskm_package((void *)&cont, &flag, cskmp);
    return(PASSED);
}

/*
 * Function: act2_mfg_login
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
act2_mfg_login (void)
{
    int ret_val, cnt;
    uchar signature[513];
    uchar ruby_signature[513];
    uint i, xx, bytes;
    u2 len_nonce_cert_chain;
    
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
    ret_val = act2_switch_simple_mode(0);
    
    if (ret_val) {
        printf("Cannot switch device to simple mode. may already be in simple mode.");
    }

    printf("\n Reading serial number... ");
    ret_val = act2_get_serial_num();

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

    ret_val = act2l_init_manufacturing_login((void *)&cont,
                                             len_nonce_cert_chain, nonce_num);
        
    if (ret_val) {
        cterr('f',0," Cannot read Nonce number from   (Ruby) device");
        printf("\n act2l_init_manufacturing_login returned with status %d", ret_val);
        return(FAILED);
    }

    printf("\n Nonce Number is:");
    for (i = 0; i < sizeof(nonce_num); i++) {
        printf("%02x", nonce_num[i]);
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

    for (i = 0; i < (len_nonce_cert_chain*2+1); i++) {
        nonce_cert_chain_ptr[i] = atoh(nonce_cert_chain_ptr[i]);
    }

    /* ZZZ_ARRAY */
    for (i = 0; i < (len_nonce_cert_chain); i++) {
        ruby_nonce_cert_chain[i] = ((nonce_cert_chain_ptr[2*i] << 4) & 0xf0) | 
            ((nonce_cert_chain_ptr[2*i + 1]) & 0x0f);
    }

    fflush(stdout);
    
    printf("\n Read signature > ");fflush(stdout);
    getline_act2((char *)signature, sizeof(signature));    //    fread(signature, sizeof(signature), 1, stdin);

    for (i = 0; i < (256*2); i++) {
        signature[i] = atoh(signature[i]);
    }

    /* ZZZ_ARRAY */
    for (i = 0; i < (256*2); i++) {
        ruby_signature[i] = ((signature[2*i] << 4) & 0xf0) | 
            ((signature[2*i + 1]) & 0x0f);
    }

    for (i = 0; i < (256); i++) {
        printf("%02x", ruby_signature[i]);
    }

    printf("\ng STEP 2: Finalize manufacturing login. ");

    ret_val = act2l_finalize_manufacturing_login(/*NULL */ (void *)NULL,
                                                 len_nonce_cert_chain,
                                                 ruby_nonce_cert_chain, 
                                                 ruby_signature, 
                                                 &session_id);
    if (ret_val) {
        cterr('f',0," Cannot write to device");
        printf("\n act2l_finalize_manufacturing_login returned with status %d", ret_val);
        return(FAILED);
    }

    printf("\n ACT-2 Manufacturing Login Successful.");
    return(PASSED);
}

/*
 * Function: act2_install_cliip
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
act2_install_CLIIP (void)
{
    int ret_val;
    uint i, cnt, xx, bytes ;
    uchar *cliip_data_ptr;
    uchar *ruby_cliip_data;
    u2 len_cliip_data, len_cliip_data_swp;

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
    
    cliip_data_ptr = (uchar *)malloc(len_cliip_data*2+1);
    ruby_cliip_data = (uchar *)malloc(len_cliip_data*2+1);

    for (bytes = cnt = xx=0; cnt<3; cnt++) {
        printf("\n Read CLIIP data > ");
        //getline_act2(cliip_data_ptr, (len_cliip_data*2+1));
        xx = getline_act2((char *)&cliip_data_ptr[bytes], getpagesize()-2);
        bytes +=xx;
        //   printf("total clip bytes entered so far is: %d\n", bytes);fflush(stdout);
    }

    if (bytes != (len_cliip_data*2)) {
        cterr('f', 0, "the cliip data entered is %d bytes; it has to be %d bytes long", bytes, len_cliip_data*2);
        return FAILED;
    }
    
    for (i = 0; i < (len_cliip_data*2); i++) {
        cliip_data_ptr[i] = atoh(cliip_data_ptr[i]);
    }

    /*ZZZ_ARRAY*/
    for (i = 0; i < (len_cliip_data*2); i++) {
        ruby_cliip_data[i] = ((cliip_data_ptr[2*i] << 4) & 0xf0) | 
            ((cliip_data_ptr[2*i + 1]) & 0x0f);
    }

    /*
     * An ret_val of 1 will indicate that the CLIIP has already been installed
     * for this chip, it's not a error and sudi or master key generation can 
     * be done with this ret_val. retry ZZZ.
     */
    len_cliip_data_swp =     len_cliip_data;
    for (i=0;i<1;i++) {
        ret_val = act2l_install_CLIIP((void *)&cont,
                                      session_id, len_cliip_data_swp, ruby_cliip_data);
        if (ret_val == 1) {
            printf("\n Identity Already Installed.");
            break;
        } else if (ret_val == 0) {
            printf("\n Identity Installation Successful.");
            break;
        } else {

        }
    }
    if ((ret_val != 1) && (ret_val != 0)) {
        cterr('f',0," Cannot write CLIIP to device");
        return(FAILED);
    }

    return(PASSED);
}

/*
 * Function: act2_install_SUDI
 *
 * This function is called by autotest to program in the SUDI certification 
 * info into ACT2. 
 * Diags must first run the manufacturing login before SUDI certification 
 * can be done.
 *
 * Inputs: None
 *
 * Output: PASSED/FAILED
 */
int
act2_install_SUDI (void)
{
    int ret_val;
    uint i;
    uchar *leaf_sudi_data_ptr;
    uchar *ruby_leaf_sudi_data;
    uchar *ca_root_sudi_data_ptr;
    uchar *ruby_ca_root_sudi_data;
    u2 len_leaf_sudi, len_ca_root_sudi;
    u2 len_leaf_sudi_swp, len_ca_root_sudi_swp;
    u2 cms_sudi_request_len;
    p_u1 cms_sudi_request_ptr, sudi_ptr;
    board_specs_t sgbu_specs;

    assert(cont.dev_if_p);

    //    ASR-4501K9  no hdd  ovld
    //    ASR-4201K9  no hdd   omah
    /* default SN and Name */

    printf("type %d; slot%d\n", cont.type, cont.slot);
    act2_switch_simple_mode(0);

    printf("prod SN is %s. \nLength of ProdSN = %d.\n",
           prodSN, (uint)strlen(prodSN));
    printf("prodName is %s. \nLength of prodName = %d.\n",
           prodName, (uint)strlen(prodName));

    printf("\n Serial Number = ");
    for (i =0; i < 11; i++) {
        printf("%c", prodSN[i]);
    }
    
    printf("\n PID = ");    
    for (i =0; i < sizeof(prodName); i++) {
        printf("%c", prodName[i]);
    }

    /* Display the info required for SUDI */
    printf("\n Common Name = ");
    for (i = 0; i < sizeof(prodName); i++) { 
        printf("%c", prodName[i]);
    }

    sgbu_specs.prodSnPtr = (uchar *)prodSN;
    sgbu_specs.prodSnLen = strlen(prodSN);

    /* Set the common name. It's needed for mfg back */
    sgbu_specs.prodNamePtr = (uchar *)prodName;//pid_altamont;
    sgbu_specs.prodNameLen = strlen(prodName);//sizeof(pid_altamont);

    sgbu_specs.pidPtr = (uchar *)prodName;  
    sgbu_specs.pidLen = strlen(prodName);

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
    ret_val = create_CMS_SUDI_request((void *)&cont,
                                      session_id, 
                                      &cms_sudi_request_ptr,
                                      &cms_sudi_request_len,
                                      &sgbu_specs);
    cms_sudi_request_len =  (cms_sudi_request_len); /*ZZZ*/

    if (ret_val) {
        cterr('f',0," Cannot create CMS sudi request");
        printf("\n create_CMS_SUDI_request returned with status %d", ret_val);
        return(FAILED);
    }

    printf("\n ACT-2: SUDI cms request (length is %d) = ",cms_sudi_request_len);
    sudi_ptr = cms_sudi_request_ptr;
    for (i = 0; i < cms_sudi_request_len; i++) {
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

    for (i = 0; i < (len_leaf_sudi*2); i++) {
        leaf_sudi_data_ptr[i] = atoh(leaf_sudi_data_ptr[i]);
    }

    /*ZZZZ_ARRAY*/
    for (i = 0; i < (len_leaf_sudi*2); i++) {
        ruby_leaf_sudi_data[i] = ((leaf_sudi_data_ptr[2*i] << 4) & 0xf0) | 
            ((leaf_sudi_data_ptr[2*i + 1]) & 0x0f);
    }

    len_ca_root_sudi = 0;
    do {
        len_ca_root_sudi = getdec_answer("\n Enter (CA and ROOT) length >",0,1,65535);
    } while (!len_ca_root_sudi);
    
    ca_root_sudi_data_ptr = (uchar *)malloc((len_ca_root_sudi*2+1));
    ruby_ca_root_sudi_data = (uchar *)malloc((len_ca_root_sudi*2+1));
    printf("\n Reading CA and ROOT SUDI data > ");
    getline_act2((char *)ca_root_sudi_data_ptr, (len_ca_root_sudi*2+1));

    for (i = 0; i < (len_ca_root_sudi*2); i++) {
        ca_root_sudi_data_ptr[i] = atoh(ca_root_sudi_data_ptr[i]);
    }

    /*ZZZ_ARRAY*/
    for (i = 0; i < (len_ca_root_sudi*2); i++) {
        ruby_ca_root_sudi_data[i] = ((ca_root_sudi_data_ptr[2*i] << 4) & 0xf0) | 
            ((ca_root_sudi_data_ptr[2*i + 1]) & 0x0f);
    }
    /* 893+839*/
    len_leaf_sudi_swp =  (len_leaf_sudi);  /*ZZZ*/
    len_ca_root_sudi_swp =  (len_ca_root_sudi); /*ZZZ*/

    /* retry ZZZ */
    for (i=0;i<1;i++) {
        ret_val = act2l_install_SUDI((void *)&cont,
                                     session_id, 
                                     len_leaf_sudi_swp, 
                                     ruby_leaf_sudi_data,
                                     len_ca_root_sudi_swp, 
                                     ruby_ca_root_sudi_data);

        if (ret_val) {

        } else {
            break;
        }
    }
    if (ret_val) {
        cterr('f',0," Cannot write SUDI data to ACT-2 device");
        printf("\n act2l_install_sudi returned with status %d", ret_val);
        return(FAILED);
    }

    printf("\n ACT-2 SUDI Installation Successful.");
    return(PASSED);
}

/*
 * Function: act2_gen_master_key
 *
 * This function will generate the master key within the ACT-2 chip.
 *
 * Inputs: None
 *
 * Output: PASSED/FAILED
 */
int
act2_gen_master_key (void)
{
    printf("\n ACT-2 Master Key generation not supported.");
    return (PASSED);
}

/*
 * Function: act2_close_mfg_login
 *
 * This function will close disconnect the manufacturing login to the ACT-2 chip.
 *
 * Inputs: None
 *
 * Output: PASSED/FAILED
 */
int
act2_close_mfg_login (void)
{
    int ret_val;
    
    printf("\n ACT-2 Close Manufacturing login.");

    if (session_id == 0) {
        printf("\n The Session id has not been generated to allow this.");
        printf("\n Please run the manufacturing login option first.");
        return(PASSED);
    }

    ret_val = act2l_close_manufacturing_login((void *)&cont, session_id);
    if (ret_val) {
        cterr('f',0," Cannot close manufacturing login to the ACT-2 device");
        printf("\n act2l_close_manufacturing_login returned with status %d", ret_val);
        return(FAILED);
    }

    printf("\n ACT-2 Manufacturing Login Closed Successful.");
    return(PASSED);
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
 * Function : act2_test
 * Description: internal chip test
 * INPUT:  dummy -- not used
 * OUTPUT: PASSED or FAILED
 * -------------------------------------------------------------------
 */
static int
act2_test (int dummy)
{
    if (act2l_test_communication((void *)&cont))
        return(FAILED);
    
    return(PASSED);
}

/*
 * Function: user_act2l_read_sudi_
 *
 * This function will check to ensure that the CLIIP/SUDI has been installed.
 *
 * Inputs: None
 *
 * Output: PASSED/FAILED
 */
static int
user_act2l_read_sudi (void)
{
    int ret_val;

    /* Ensure that the ACT2 device is in simple mode */
    printf("\n Switching ACT-2 device to simple mode... ");
    ret_val = act2_switch_simple_mode(0);
    if (ret_val) {
        printf("Cannot switch device to simple mode. may already be in simple mode.");
    }

    ret_val = act2l_read_sudi((void *)&cont);
    if (ret_val) {
        printf("\n CLIIP/SUDI not installed.");
        printf("\n act2l_read_sudi returned with status %d", ret_val);
        return(FAILED);
    }

    printf("\n -------------------------");
    printf("\n CLIIP/SUDI install OK.");
    printf("\n -------------------------");
    return (PASSED);
}

/*
 * Function: act2_auth_exist
 *
 * Description : existence funtion for act2 authenticate
 *
 * Inputs: none
 *
 * Output: TRUE/FALSE
 */
static int
act2_auth_exist (void)
{
    /* act2 authenticate menu item should not be displayed,
     * so always return FALSE  
     */
    return FALSE;
}

/*
 * Function: act2_authenticate
 *
 * Description : Send command to authenticate
 *
 * Inputs: a  - notused
 *
 * Output: PASSED/FAILED
 */
static int
act2_authenticate (int a)
{
    /* warning message to remind user to use this item carefully */
    printf("\nFor correlation purpose only.");
    printf("\nAuthentication must use IOS which uses 3rd party ACT2 library type.");
    printf("\nDiags uses standalone type just for quick checking.");
    if (getc_answer("\nContinue? [y/n]", "yn", 'n') == 'y') {
        int ret_val;

        printf("session id is %d\n", session_id);

        ret_val = act2l_test_manufacturing_installation(NULL, session_id);
        if (ret_val) {
            cterr('f', 0, "authentication failed ; status %d", ret_val);
            return FAILED;
        } 
        printf("Authentication passed\n");

        return PASSED;
    }
        
    return PASSED;
}


/*
 * Function: get_wdc
 *
 * Description : helper function to getwdc from terminal and convert to binary
 *
 * Inputs: NONE
 *
 * Output: data_size - size of data read from terminal
 *         credential - string containing credential
 */
static int
get_wdc(u2 *data_size, unsigned char *credential)
{
    unsigned int size, i, byte;
    uchar cre_buf[SUDI_MAX_SIZE];
    uchar cre[SUDI_MAX_SIZE];
    uchar *tmp;
    char *argv[] = { "progwdc", "M"};

    assert(credential);
    memset(cre_buf, 0xA5, sizeof(cre_buf));
    memset(credential, 0xA5, sizeof(credential));

    /* reset to make sure chip is not in simple mode. */
    act2_reset(0);
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

/*
 * Function:  act2_platform_initial_config
 *
 * Description : create wdc object and save it into act2 chip
 *
 * Inputs: module -- pointer to object that we want to pass to act2 driver
 *         src_buffer -- containing credential
 *         data_size - size of data read from terminal
 * Output: return status from library
 *         
 */
static ACT2_STATUS
act2_platform_initial_config (void *module,
                              uchar *src_buffer,
                              u2 data_size)
{
    uchar admin_password[64];
    ACT2_STATUS status = 0;
    ACT2_STATUS cleanup_status = 0;
    OBJECT_ID object_id = 0;
    u1 i, num_objects;
    ACT2_OBJECT_ENUM object_list[50];
    SESSION_ID admin_session_id = 0;
    p_u1 dest_buffer = NULL;
    int num_retries = 3;

    act2_switch_simple_mode(0);
    
    memset(object_list, 0xA5, sizeof(object_list));
    
    memset(admin_password, 0xA5, sizeof(admin_password));

    status = act2l_gen_admin_credential(NULL, admin_password);

    if (status) {
        cterr('f', 0, "%s act2l_gen_admin_credential status error %02x\n", __FUNCTION__, status);
        return FAILED;
    }

    //create administrator session
    status = act2_session_init(module, 0x01 /* ACT2_ADMIN_USER */, admin_password, &admin_session_id);
    if (status){
        cterr('f', 0, "Create Session Failed with status %02x\n", status);
        goto config_exit;
    }
    //#ifdef ENUM

    status = secure_object_enumerate(module, admin_session_id, &num_objects, object_list);
    if (status) {
        cterr('f', 0, "secure object enumerate Failed with status %02x\n", status);
        goto config_exit;
    }

    printf("num obj=%d\n", num_objects);

    if (num_objects > 50)
        num_objects = 50;

    for (i=0; i < num_objects; i++) {
        //    for (i=0; i < 1; i++) {
        printf("deleting [%d]: id: %04x\n", i, object_list[i].object_id);
        status = secure_object_delete(module, admin_session_id, object_list[i].object_id);
        if (status) {
            cterr('f', 0, "delete object Failed with status %02x\n", status);
            goto config_exit;
        }   
    }

    
    dest_buffer = (p_u1)malloc(data_size);  /* freed by the library */
    assert(dest_buffer);
    memset(dest_buffer, 0, data_size);
    
    printf("secure_object_created %d\n", __LINE__);fflush(stdout);
    
    //create a RAW unencrypted, non-CSP object in EEPROM of data_size bytes
    status = secure_object_create(module, admin_session_id, RAW_OBJECT, data_size, NOT_CSP,
                                  ROM, SOURCE_COPY, &object_id);
    if (status){
        cterr('f', 0, "Create Object Failed with status %02x\n", status);
        goto config_exit;
    }

    printf("w/rite_object_created %d\n", __LINE__);fflush(stdout);
    //Write buffer to object
    act2_write_object(module, admin_session_id, object_id, src_buffer, data_size);
    if (status){
        cterr('f', 0, "Write Object Failed with status %02x\n", status);
        goto config_exit;
    }

    printf("read_object_created  data_size %d; line %d\n", data_size,  __LINE__);fflush(stdout);
    //Read the object in that buffer.  
    act2_read_object(module, admin_session_id, object_id, dest_buffer, data_size);
    if (status){
        cterr('f', 0, "Read Object Failed with status %02x\n", status);
        goto config_exit;
    }
	
    //Compare two buffers, if retries fail delete object, 
    do {

        printf("writing object %d\n", num_retries);
        
        act2_write_object(module, admin_session_id, object_id, src_buffer, data_size);
        if (status) {
            cterr('f', 0, "Retrying Write Object Failed with status %02x\n", status);
            goto config_exit;
                
        }
        printf("read_object_created %d\n", __LINE__);fflush(stdout);	
        act2_read_object(module, admin_session_id, object_id, dest_buffer, data_size);
        if (status){
            cterr('f', 0, "Read Object Failed with status %02x\n", status);
            goto config_exit;
        }

        if (!memcmp(dest_buffer, src_buffer, data_size)) {
            printf("ok passed\n");
            status = act2_terminate_session(module, admin_session_id);
            return status;
        }

        //If failed, retry writing original buffer, reread and compare again.
        printf("failed reading object trying again %d\n", num_retries);

    } while (--num_retries);

    
 config_exit:
    cleanup_status = act2_terminate_session(module, admin_session_id);
    if (cleanup_status) {
        cterr('f', 0, "Termiate session error");
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
    int ret_val;
    uchar credential[SUDI_MAX_SIZE];
    u2 data_size = 0;

    if (get_wdc(&data_size, credential) == FAILED) {
        return FAILED;
    }
    
    ret_val =  act2_platform_initial_config(NULL, credential, data_size);
    if (ret_val) {
        cterr('f', 0, "%s failed status = %d\n", __FUNCTION__, ret_val);
        return FAILED;
    }

    printf("WCD passed\n");

    return PASSED;
}


/*-------------------------------------------------------------------
 *
 * Function : act2_enumerate
 * Description: enumerate wdc object ids stored in act2 chip. in theory, we
 * shoudl have exatcly one object id. no more. no less. this utility can help
 * check if there's too many objects or none at all.
 * INPUT:  notused -- not used.
 * OUTPUT: return PASSED or FAILED
 * -------------------------------------------------------------------
 */
static int
act2_enumerate (int notused)
{
    uchar admin_password[64];
    char err[80];
    ACT2_STATUS status = 0;
    ACT2_STATUS cleanup_status = 0;
    u1 i, num_objects;
    void *module = NULL;
    ACT2_OBJECT_ENUM object_list[50];
    SESSION_ID admin_session_id = 0;

    memset(object_list, 0xA5, sizeof(object_list));
    
    memset(admin_password, 0xA5, sizeof(admin_password));

    status = act2l_gen_admin_credential(NULL, admin_password);

    if (status) {
        cterr('f', 0, "%s act2l_gen_admin_credential status error %02x\n", __FUNCTION__, status);
        return FAILED;
    }

    //create administrator session
    status = act2_session_init(module, 0x01 /* ACT2_ADMIN_USER */, admin_password, &admin_session_id);
    if (status) {
        sprintf(err, "Create Session Failed with status %02x", status);
        goto config_exit;
    }

    num_objects = 0;
    status = secure_object_enumerate(module, admin_session_id, &num_objects, object_list);
    if (status) {
        sprintf(err, "secure object enumerate Failed with status %02x", status);
        goto config_exit;
    }

    printf("num obj =%d \n", num_objects);

    if (num_objects > 50)
        num_objects = 50;

    for (i = 0; i < num_objects; i++) {
        printf("Object ID [%d]: %04x\n", i, object_list[i].object_id);
    }

 config_exit:
    
    cleanup_status = act2_terminate_session(module, admin_session_id);

    if (status) {
        cterr('f', 0, err);
    }
    
    if (cleanup_status) {
        cterr('f', 0, "Termiate session error");
    }

    /* if we fail, return status of failure before cleanup */
    return FAILED;

}

/*-------------------------------------------------------------------
 *
 * Function : act2_platform_object_burn_in_test
 * Description: burn in test for act2 chip.  inside the for loop, we
 * can bump up the number of loops to run. higher the number the longer
 * the burn in test.
 * INPUT:  dummy -- not used.
 * OUTPUT: return PASSED or FAILED
 * -------------------------------------------------------------------
 */
static ACT2_STATUS
act2_platform_object_burn_in_test (int dummy)
{
    uchar admin_password[64];
    ACT2_STATUS status = 0;
    OBJECT_ID object_id = 0;
    u2 i;
    SESSION_ID admin_session_id = 0;
    p_u1 dest_buffer = NULL;
    u2 data_size;
    p_u1 ptr;
    void *module = NULL;

    /* !!THIS TEST NEED TO BE IN SIMPLE MODE. WDC and SUDI NEED TO BE INSTALLED !! */
    testname("act2 burnin");
    memset(admin_password, 0xA5, sizeof(admin_password));

    status = act2l_gen_admin_credential(NULL, admin_password);

    if (status) {
        cterr('f', 0, "%s act2l_gen_admin_credential status error %02x\n", __FUNCTION__, status);
        return FAILED;
    }

    //create administrator session
    status = act2_session_init(NULL, 0x01 /* ACT2_ADMIN_USER */, admin_password, &admin_session_id);
    if (status){
        cterr('f', 0, "Create Session Failed with status %02x\n", status);
        return FAILED;
    }

    /* is this the default data size for wdc ?? */
    data_size = 0x200;
    
    assert(data_size);
    dest_buffer = (p_u1)malloc(data_size);  /* freed by the library */
    assert(dest_buffer);
    memset(dest_buffer, 0, data_size);

    ptr = dest_buffer;
    for (i = 0; i < 4; i++, ptr+=0x80) {
	status = act2_get_random_number(module, 0xFFFFFFFF, ptr, 0x80);
	if (status) {
            free(dest_buffer);
	    cterr('f', 0, "%s  act2_get_random_number status error %02x\n", __FUNCTION__, status);
	    return FAILED;
	}
    }
    //create a RAW unencrypted, non-CSP object in RAM of data_size bytes
    status = secure_object_create(module, admin_session_id, RAW_OBJECT, data_size, NOT_CSP,
                                  RAM, SOURCE_COPY, &object_id);
    if (status){
        free(dest_buffer);
        cterr('f', 0, "Create Object Failed with status %02x\n", status);
        return FAILED;
    }

    /* for burn in test, we call library routine which will stress the device.
       put inside the loop so we can keep runing. the for loop limit can be any value.
       alternatel, we can use continous flag but test time is longer that way.
       act2_write_object is library api.
    */
    for (i = 1; i < 0x3; i++) {	
	//Write buffer to object
        
	status = act2_write_object(module, admin_session_id, object_id, dest_buffer, i);
	if (status){
            free(dest_buffer);
	    cterr('f', 0, "Write Object Failed with status %02x\n", status);
	    return FAILED;
	}	
	//Read the object in that buffer.
	status = act2_read_object(module, admin_session_id, object_id, dest_buffer, i);
	if (status){
            free(dest_buffer);
	    cterr('f', 0, "Read Object Failed with status %02x\n", status);
	    return FAILED;
	}
    }	

    free(dest_buffer);

    status = secure_object_delete(module, admin_session_id, object_id);

    if (!status) {
        printf("secure_object_deleted %08x\n", object_id);fflush(stdout);
    }
    else {
        cterr('f', 0, "Deleting Object Failed with status %02x\n", status);
        return FAILED;
    }
 
    
    status = act2_terminate_session(module, admin_session_id);

    if (status) {
        cterr('f', 0, "Terminating Session Failed with status %02x\n", status);
        return FAILED;
    }
    
    prcomplete(testpass, errcount, (char *)0);
    return PASSED;

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
    reset_plat_dev(FPGA_RST_ACT2);
    unreset_plat_dev(FPGA_RST_ACT2);
    act2_simple_mode[device_type] = 0;
    msleep(200);
    return PASSED;

}

int
act2_toggle_debug_flag (int dummy)
{
    act2_i2c_debug ^= 1;
    return(PASSED);
    
}
/******** History ******** 
$Log: act2_utils.c,v $
Revision 1.9  2015/01/14 08:52:59  danchung
CSCus28903: Implement the new menu item flag MF_HIDDEN_EXE on act2 submenu
item "Authenticate (n/a)/display cert"

Revision 1.8  2014/06/06 21:03:10  mcharon
change the menu name for authenticat

Revision 1.7  2014/02/13 19:54:41  mcharon
add debug flag for act2

Revision 1.6  2014/02/13 19:03:11  mcharon
support act2 authentication on sword

Revision 1.5  2014/02/12 18:23:49  mcharon
show err status in dec, to be consistent

Revision 1.4  2013/12/18 06:32:58  hroni
use toolchain in router/bin to do make with TOOLS_VER=c4.5.3-p1, TOOLS_ARCH=x86_64

Revision 1.3  2013/11/26 08:40:35  hroni
fix compiler warning

Revision 1.2  2013/05/09 07:36:50  alpeng
updating files

Revision 1.1  2013/05/09 05:42:35  alpeng
moving overlord common code from x86

Revision 1.22  2013/05/01 20:40:56  mcharon
 call memset inside act2_utils.c

Revision 1.21  2013/04/23 21:40:54  mcharon
remove prodSN[11]='\0'

Revision 1.20  2013/03/08 19:06:12  mcharon
in wdc call get_pid instead of get_mb_pid, so modules are supported

Revision 1.19  2013/02/06 06:10:19  mcharon
better error msg for verification CMS request message

Revision 1.18  2013/01/24 01:02:15  mcharon
dont' print out cms_sudi_req_ptr before calling create_CMS_SUDI_requeste

Revision 1.17  2013/01/15 02:25:13  palin2
Reset Quack chip(ACT2) by FPGA right after Diag is up to
ensure Quack chip in a valid state.

Revision 1.16  2012/11/17 01:15:18  mcharon
reset i2c device; don't cterr in driver code..propogate err message to slot.c

Revision 1.15  2012/11/07 18:21:17  mcharon
cleanup


Revision 1.13  2012/08/30 19:46:29  mcharon
during wdc install reset quack before reading cookie to get out of simple mode

Revision 1.12  2012/08/29 20:39:19  mcharon
reset quack before reading cookie to get it out of simple mode

Revision 1.11  2012/08/11 00:38:02  mcharon
take out 1 sec delay between set/reset quack chip

Revision 1.10  2012/08/11 00:18:48  mcharon
support simpe mode and add menu to reset Act chip.

Revision 1.9  2012/07/25 20:11:23  mcharon
do not stop test if switch_to_simple_mode failes. chip maybe in simple mode already

Revision 1.8  2012/07/19 18:58:48  mcharon
remove references to dc3_cookie

Revision 1.7  2012/06/26 16:59:05  mcharon
add burn in test act2_platform_object_burn_in_test() for ACT2 chip

Revision 1.6  2012/06/20 20:29:58  mcharon
support wdc

Revision 1.5  2012/06/07 02:11:21  palin2
Clean up compiler warnings.

Revision 1.4  2012/06/06 07:34:05  palin2
Clean up compiler warnings.

Revision 1.3  2012/06/04 10:35:15  palin2
Clean up compiler warnings.

Revision 1.2  2012/03/28 00:38:19  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:07  ptong
Initial archive of ng_diag module


$Endlog$
*/
