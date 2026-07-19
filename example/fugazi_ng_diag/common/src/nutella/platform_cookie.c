/* $Id: platform_cookie.c,v 1.5 2019/08/16 10:56:20 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nutella/platform_cookie.c,v $
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
#include "tam_library.h"
#include "tam_lib_manufacturing.h"
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
int get_hwv(char *);
int get_vsn(char *);
int get_pca(char *);
int get_part_number(char *);
int get_clei_code(char *, int *);
int get_vid(char *);
int get_pcb_serial(uchar *, char *);
int print_cookie(int, char *argv[]);
int platform_get_pid(char *);
static int alter_cookie(int);
void display_eeprom_string(unsigned int, unsigned int);
void display_eeprom_hex_content(unsigned int, unsigned int);
static int write_eeprom_block(unsigned int, char *, int);
static int write_eeprom_hex(unsigned int, unsigned int);
int plat_init_smart_eeprom_context(sc_context *, uchar, uchar, uchar *);
int i2c_quack_read_bytes(sc_context *, char *);
int i2c_quack_write_bytes(sc_context *, char *, int);
void read_eeprom_util(void); 
int write_eeprom_util(void); 
int sync_cookie_content_into_eeprom(void);
int alter_dmi_eeprom(int);
int get_mac_addr(uchar *, char *);
int get_mac_addr_blk_size(char *);
int get_mfg_test_data(char *);
int get_controller_type(char *);
unsigned char CharToHex(unsigned char);
static int clear_field(unsigned int, int);
void get_input_string(char *, int);

/***********************************************************************
 *  Extern Functions Declaration
 ************************************************************************/
extern int ExecuteCmdbyPopen(char *cmd, char *retBuf, int sizeofBuf);
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
static char get_prodName[PRODUCT_NAME_LEN] = {0};
static void *platform_tam_handle = NULL;
static uchar cookie_contents[COOKIE_SIZE_512];
static int g_cookie_type = 0; 
static char smc_buf[80];
static char i2c_err[80];
static uint8_t use_interrupt = 0;
boolean aikido_act2_flag = TRUE;
boolean aikido_mailbox_flag = TRUE;
static boolean modified_cookie_flag = FALSE;

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

static uchar default_eeprom_content[NUTELLA_EEPROM_INIT_SIZE] = {
    /* 0x00 */
    0x0f, 0x53, 0x4d, 0x20, 0x3a, 0x20, 0x43, 0x69,
    0x73, 0x63, 0x6f, 0x20, 0x53, 0x79, 0x73, 0x74,
    /* 0x10 */
    0x65, 0x6d, 0x73, 0x2c, 0x20, 0x49, 0x6e, 0x63,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x20 */
    0x53, 0x50, 0x20, 0x3a, 0x20, 0x76, 0x45, 0x64,
    0x67, 0x65, 0x2d, 0x31, 0x30, 0x30, 0x31, 0x00,
    /* 0x30 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x40 */
    0x53, 0x56, 0x20, 0x3a, 0x20, 0x31, 0x2e, 0x30,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x50 */
    0x53, 0x53, 0x20, 0x3a, 0x20, 0x31, 0x39, 0x36,
    0x42, 0x30, 0x31, 0x30, 0x38, 0x31, 0x39, 0x30,
    /* 0x60 */
    0x30, 0x30, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x70 */
    0x53, 0x4b, 0x20, 0x3a, 0x20, 0x76, 0x45, 0x64,
    0x67, 0x65, 0x2d, 0x31, 0x30, 0x30, 0x31, 0x00,
    /* 0x80 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x90 */
    0x53, 0x46, 0x20, 0x3a, 0x20, 0x76, 0x45, 0x64,
    0x67, 0x65, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0xA0 */
    0x42, 0x4d, 0x20, 0x3a, 0x20, 0x43, 0x69, 0x73,
    0x63, 0x6f, 0x20, 0x53, 0x79, 0x73, 0x74, 0x65,
    /* 0xB0 */
    0x6d, 0x73, 0x2c, 0x20, 0x49, 0x6e, 0x63, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0xC0 */ 
    0x42, 0x50, 0x20, 0x3a, 0x20, 0x76, 0x45, 0x64,
    0x67, 0x65, 0x2d, 0x31, 0x30, 0x30, 0x31, 0x00,
    /* 0xD0 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0xE0 */
    0x42, 0x56, 0x20, 0x3a, 0x20, 0x31, 0x2e, 0x30,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0xF0 */
    0x42, 0x53, 0x20, 0x3a, 0x20, 0x46, 0x4f, 0x43,
    0x32, 0x32, 0x32, 0x30, 0x35, 0x57, 0x31, 0x42,
    /* 0x100 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x110 */
    0x43, 0x4d, 0x20, 0x3a, 0x20, 0x43, 0x69, 0x73,
    0x63, 0x6f, 0x20, 0x53, 0x79, 0x73, 0x74, 0x65,
    /* 0x120 */
    0x6d, 0x73, 0x2c, 0x20, 0x49, 0x6e, 0x63, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x130 */
    0x43, 0x56, 0x20, 0x3a, 0x20, 0x31, 0x2e, 0x30,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x140 */
    0x43, 0x53, 0x20, 0x3a, 0x20, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x150 */
    0x43, 0x41, 0x20, 0x3a, 0x20, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x160 */
    0x43, 0x53, 0x4b, 0x3a, 0x20, 0x76, 0x45, 0x64,
    0x67, 0x65, 0x2d, 0x31, 0x30, 0x30, 0x31, 0x00,
    /* 0x170 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

/**************************************************************************
 *
 * Name: CharToHex 
 *
 * Description: ASCII Character text change to 16 hex
 *
 * Inputs: bHex - ASCII Text
 *
 * Outputs: Hex value
 *
 *************************************************************************/
unsigned char CharToHex (unsigned char bHex)
{
    if((bHex >= 0) && (bHex <= 9)) {
        bHex += 0x30;
    } else if ((bHex >= 10) && (bHex <= 15)) {
        /* Capital */
        bHex += 0x37;
    } else {
        bHex = 0xff;
    }

    return (bHex);
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
 *         cookie_size - size of cookie
 *
 * Output: none
 *
 *------------------------------------------------------------------------*/
void init_cookie_4_default_x(int board_type, int cookie_type,
                        uchar * contents, int cookie_size)
{
    uint sku_id_val;

    printf("%s: Nutella currently skip SKU ID check.\n", __FUNCTION__);
    sku_id_val = 0xFF;

    switch (board_type) {
    case MOTHER_BOARD:
        printf("\nLoading default cookie format for Nutella Motherboard.\n");
        switch (sku_id_val) {
        case 0xFF:
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

/*******************************************************************************
 *
 * Function    : alter_dmi_eeprom
 * Description : Alter/read/init DMI EEPROM util.
 * Inputs      : opt - reserved for future use
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int alter_dmi_eeprom (int opt)
{
    int sel_action;

    sel_action = gethex_answer("Select action for EEPROM: 0/Read, 1/Write: "
                               , 0, 0, 1);
    
    switch (sel_action) {
         case READ_DMI_EEPROM:
             read_eeprom_util();
             break;
         case WRITE_DMI_EEPROM:
             if (write_eeprom_util() == FAILED) {
                 return (FAILED);
             }
             break;
         default:
             printf("\nPlease try again\n");
             break;
    }
    
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : read_eeprom_util
 * Description : read EEPROM register util.
 * Inputs      : None 
 * Outputs     : None
 *
 *******************************************************************************
 */

void read_eeprom_util (void)
{
    uint reg_offset = 0, reg_size = 0;

    reg_offset = gethex_answer("Enter register offset (0x0 ~ 0x2000):",
                               0, 0, 0x2000);
    reg_size = gethex_answer("Enter register size (0x1 ~ 0x2000):",
                             0x1, 0x1, 0x2000);
    /* Display EEPROM HEX content */
    display_eeprom_hex_content(reg_offset, reg_size);
    /* Display EEPROM human readable string content. */
    display_eeprom_string(reg_offset, reg_size);
}


/*******************************************************************************
 *
 * Function    : get_eeprom_string
 * Description : Read EEPROM from specific register offset to get string content
 * Inputs      : offset - read form which offset
 *               size   - read how many byte
 *               string_buf    - string buffer
 * Outputs     : None
 *
 *******************************************************************************
 */
void get_eeprom_string (unsigned int offset, unsigned int size, char *string_buf) 
{
    char cmd[MAX_COMMAND_LENGTH] = {0};
    
    sprintf(cmd, GET_READABLE_STRING, offset, size);
    ExecuteCmdbyPopen(cmd, string_buf, MAX_COMMAND_LENGTH);
}

/*******************************************************************************
 *
 * Function    : display_eeprom_hex_content 
 * Description : Show EEPROM content for binary content by eeprog.
 * Inputs      : offset   - read form which offset
 *               size     - read how many byte
 * Outputs     : None
 *
 *******************************************************************************
 */
void display_eeprom_hex_content (unsigned int offset, unsigned int size)
{
    char cmd[MAX_COMMAND_LENGTH] = {0};
    char buf[MAX_STRING_LEN] = {0};
    
    /* Get hex data by eeprog tool */
    sprintf(cmd, GET_HEX_EEPROM_CONTENT, offset, size);
    ExecuteCmdbyPopen(cmd, buf, MAX_COMMAND_LENGTH);
    printf("Hex data:%s\n", buf);
}

/*******************************************************************************
 *
 * Function    : display_eeprom_string
 * Description : Show EEPROM content for both binary and string format by eeprog.
 * Inputs      : offset   - read form which offset
 *               size     - read how many byte
 * Outputs     : None
 *
 *******************************************************************************
 */
void display_eeprom_string (unsigned int offset, unsigned int size)
{
    char buf[MAX_STRING_LEN] = {0};
    char cmd[MAX_COMMAND_LENGTH] = {0};
    char show_string[NUTELLA_EEPROM_INIT_SIZE];
    int ix = 0, jx = 0, show_flag = FALSE;
    char r_string[MAX_STRING_LEN] = {0};
    /* First readable format is Nuber/Count of type-11 strings */
    int show_num = 1;

    /* Get Number/Count of type-11 strings hex data */
    sprintf(cmd, GET_NO_STRING);
    ExecuteCmdbyPopen(cmd, r_string, MAX_COMMAND_LENGTH);
    
    /* Dump current DMI EEPROM content */ 
    printf("\nShow current DMI EEPROM content:\n");
    printf(" %d : Number/Count of type-11 strings : %c%c\n", 
           show_num, r_string[NO_OF_STRING_HEX_VAL_BYTE0], 
           r_string[NO_OF_STRING_HEX_VAL_BYTE1]);

    memset(show_string, 0, sizeof(show_string));
    /* Get string data by eeprog tool 
     * Original string output that reads by eeprog tool will be like: 
     * SM : Cisco Systems, IncSP : ISRABCDESV : 1.0SS : 196A23025YQDSK : 
     * vEdge-1001SF : vEdgeBM : Cisco Systems, IncBP : vEdge-1001BV : 1.0
     * BS : FOC23025YQDCM : Cisco Systems, IncCV : 1.0CS : CA : CSK: vEdge-1001
     */
    get_eeprom_string(offset, size, buf);
    /* Below codes will parsing the string data to make it readable in the 
     * console output as below:
     *  1 : Number/Count of type-11 strings : 0f
     *  2 : SM : Cisco Systems, Inc
     *  3 : SP : ISRABCDE
     *  4 : SV : 1.0
     *  5 : SS : 196A23025YQD
     *  6 : SK : vEdge-1001
     *  7 : SF : vEdge
     *  8 : BM : Cisco Systems, Inc
     *  9 : BP : vEdge-1001
     *  10 : BV : 1.0
     *  11 : BS : FOC23025YQD
     *  12 : CM : Cisco Systems, Inc
     *  13 : CV : 1.0
     *  14 : CS :
     *  15 : CA :
     *  16 : CSK: vEdge-1001 
     */
    for (ix = 0; ix < NUTELLA_EEPROM_INIT_SIZE; ix++) {
        /* Each string will be end of mutiple 0, 
         * so if a character is not greater than 0,
         * then it's the end of string. */
        if(buf[ix] > 0) {
            /* Get meaningful string */
            show_string[jx] = buf[ix];
            jx += 1;
            show_flag = TRUE;
        } else {
            /* Add end of string and show the string */
            show_string[ix] = END_OF_STRING;
            jx = 0;
            /* Get a new type-11 string and show it. */
            if (show_flag == TRUE) {
                /* Number of the type-11 string */
                show_num += 1;
                printf(" %d : %s", show_num, show_string);
                /* Printf a new line when it shows after second string */
                if (ix > SECOND_FIELD) {
                    printf("\n");
                }
            }
            memset(show_string, 0, sizeof(show_string));
            show_flag = FALSE;
        }
    } 
}

/*******************************************************************************
 *
 * Function    : get_input_string 
 * Description : Get user input string content and check if the length is correct.
 * Inputs      : *w_buf - get write string
 *               str_len - string length 
 * Outputs     : None 
 *
 *******************************************************************************
 */

void get_input_string(char *w_buf, int str_len)
{
   int FLAG = FALSE, input_len;

   /* Get input string */
   while (FLAG == FALSE) {
       /* Get the user input string */
       fgets(w_buf, MAX_COMMAND_LENGTH, stdin);
       /* Calculate input string length, -1 means remove enter character. */
       input_len = (strlen(w_buf) - 1);
       if (input_len > str_len) {
           printf("\nYou input %d characters, it should be less than %d characters\n",
                  input_len, str_len);
           printf("Please try to input again:\n");
       } else {
           FLAG = TRUE;
       }
   }
}

/*******************************************************************************
 *
 * Function    : write_eeprom_util
 * Description : write EEPROM register util.
 * Inputs      : None 
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */

int write_eeprom_util (void)
{
    char c;
    int choose = 0;
    uint reg_offset = 0, reg_size = 0;
    char w_buffer[MAX_STRING_LEN] = {0};
  
    /* Display current DMI EEPROM content in human readable string */ 
    display_eeprom_string(READ_STRING_START_ADDR, READ_STRING_END_ADDR);
    printf(" %d : Exit\n", DMI_EEPROM_EXIT);

    choose = getdec_answer("Select decimal option:", 
                           DMI_EEPROM_NUM_OF_STRING, DMI_EEPROM_NUM_OF_STRING, 
                           DMI_EEPROM_ERASE);

    memset(w_buffer, 0, sizeof(w_buffer));
    switch(choose){
         case DMI_EEPROM_NUM_OF_STRING:
             printf("\nInput hex value for Number/Count of type-11 strings "
                    "(Format as:\"0f\")\n");
             reg_offset = EEPROM_NUM_OF_STRING_OFFSET;
             reg_size = EEPROM_NUM_STRINGS_LEN;
             /* Clear this field to NULL value first. */
             if (clear_field(reg_offset, reg_size) == FAILED) {
                 return (FAILED);
             }
             /* Get input hex number */
             w_buffer[0] = gethex_answer("Enter hex number: "
                                          , 0, 0, 0xff);
             printf("Enter %#x\n", w_buffer[0]);
             /* Write string into EEPROM field */
             if (write_eeprom_hex(reg_offset, w_buffer[0]) == FAILED) {
                 return (FAILED);
             }
             break;
         case DMI_EEPROM_SM:
             printf("\nInput SM string, format as:\"SM : Cisco Systems, Inc\"\n");
             reg_offset = EEPROM_SM_OFFSET;
             reg_size = EEPROM_SM_LEN;
             break;
         case DMI_EEPROM_SP:
             printf("\nInput SP string, format as:\"SP : ISR1100-4G\"\n");
             reg_offset = EEPROM_SP_OFFSET;
             reg_size = EEPROM_SP_LEN;
             break;
         case DMI_EEPROM_SV:
             printf("\nSV field can only altered from MB CPU cookie menu\n");
             return (PASSED);
         case DMI_EEPROM_SS:
             printf("\nSS field can only altered from MB CPU cookie menu\n");
             return (PASSED);
         case DMI_EEPROM_SK:
             printf("\nSK field can only altered from MB CPU cookie menu\n");
             return (PASSED);
         case DMI_EEPROM_SF:
             printf("\nInput SF string, format as:\"SF : vEdge\"\n");
             reg_offset = EEPROM_SF_OFFSET;
             reg_size = EEPROM_SF_LEN;
             break;
         case DMI_EEPROM_BM:
             printf("\nInput BM string, format as:\"BM : Cisco Systems, Inc\"\n");
             reg_offset = EEPROM_BM_OFFSET;
             reg_size = EEPROM_BM_LEN;
             break;
         case DMI_EEPROM_BP:
             printf("\nInput BP string, format as:\"BP : ISR1100-4G\"\n");
             reg_offset = EEPROM_BP_OFFSET;
             reg_size = EEPROM_BP_LEN;
             break;
         case DMI_EEPROM_BV:
             printf("\nInput BV string, format as:\"BV : 1.0\"\n");
             reg_offset = EEPROM_BV_OFFSET;
             reg_size = EEPROM_BV_LEN;
             break;
         case DMI_EEPROM_BS:
             printf("\nBS field can only altered from MB CPU cookie menu\n");
             return (PASSED);
         case DMI_EEPROM_CM:
             printf("\nInput CM string, format as:\"CM : Cisco Systems, Inc\"\n");
             reg_offset = EEPROM_CM_OFFSET;
             reg_size = EEPROM_CM_LEN;
             break;
         case DMI_EEPROM_CV:
             printf("\nInput CV string, format as:\"CV : 1.0\"\n");
             reg_offset = EEPROM_CV_OFFSET;
             reg_size = EEPROM_CV_LEN;
             break;
         case DMI_EEPROM_CS:
             printf("\nInput CV string, format as:\"CS : Empty\"\n");
             reg_offset = EEPROM_CS_OFFSET;
             reg_size = EEPROM_CS_LEN;
             break;
         case DMI_EEPROM_CA:
             printf("\nInput CA string, format as:\"CA : Empty\"\n");
             reg_offset = EEPROM_CA_OFFSET;
             reg_size = EEPROM_CA_LEN;
             break;
         case DMI_EEPROM_CSK:
             printf("\nInput CSK string, format as:\"CSK: ISR1100-4G-AC\"\n");
             reg_offset = EEPROM_CSK_OFFSET;
             reg_size = EEPROM_CSK_LEN;
             break;
         case DMI_EEPROM_EXIT:
             printf("\nExit\n");
             return (PASSED);
             break;
         case DMI_EEPROM_ERASE:
             c = getc_answer("Will erease the EEPROM, do you want to continue?", 
                             "yn",'n');
             if (c == 'y') {
                 printf("\nErase EEPROM. Wait a minute...\n");
                 /* Create 8K bytes 0xFF bin file. */
                 system(CREATE_ERASE_FILE);
                 usleep(WAIT_EEPROG);
                 /* Erase EEPROM content */
                 system(ERASE_EEPROM_CMD);
                 usleep(WAIT_EEPROG);
                 /* Remove bin file */
                 system(RM_ERASE_FILE);
                 printf("Erase EEPROM done.\n");
             } else {
                 return (PASSED);
             }
             break;
         default:
             printf("\nPlease try again\n");
             break;
    }
             
    if ((choose != DMI_EEPROM_ERASE) && (choose != DMI_EEPROM_NUM_OF_STRING)) {
        /* Clear this field to NULL value first. */
        if (clear_field(reg_offset, reg_size) == FAILED) {
            return (FAILED);
        }
        /* Get input string */
        get_input_string(w_buffer, reg_size);
        /* Write string into EEPROM field, last character is 'Enter' from user 
         * input string, so string length needs to minus 1. */
        if (write_eeprom_block(reg_offset , w_buffer, strlen(w_buffer) - 1) 
            == FAILED) {
            return (FAILED);
        }
    }

    return (PASSED);

}
/*******************************************************************************
 *
 * Function    : write_eeprom_hex
 * Description : write EEPROM register by eeprog.
 * Inputs      : offset  - write from which offset
 *               wr_data - data i} * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */

static int write_eeprom_hex (unsigned int offset,  unsigned int wr_data)
{
    FILE *fp;
    char cmd[MAX_COMMAND_LENGTH] = {0};
    unsigned int write_buf[1] = {0};

    /* This file is for eeprog writing with option -i */
    fp = fopen(WRITE_FILE_NAME, "wb+");
    if (fp < 0) {
        fclose(fp);
        printf("Can not open diag file");
        return (FAILED);
    }

    write_buf[0] = wr_data; 
    /* Write a word at a time */
    fwrite(write_buf, sizeof(unsigned int)/4, 1, fp);
    fclose(fp);
    sprintf(cmd, WRITE_DIAG_FILE, offset);
    system(cmd);
    /* Wait for write cycle time (option -t) */
    usleep(WAIT_EEPROG);

    /* Remove the file */
    system(RM_WRITE_FILE_NAME);

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : clear_field
 * Description : Clear field value to NULL by eeprog.
 * Inputs      : offset  - write from which offset
 *               field_len - field length
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */

static int clear_field (unsigned int offset, int field_len)
{
    FILE *fp;
    char c_data[MAX_COMMAND_LENGTH] = {0};
    char cmd[MAX_COMMAND_LENGTH] = {0};
    
    /* This file is for eeprog writing with option -i */
    fp = fopen(CLEAR_FILE_NAME, "wb+");
    if (fp < 0) {
        fclose(fp);
        printf("Can not open clear file");
        return (FAILED);
    }

    /* Last character is Enter */
    fwrite(c_data, sizeof(char), field_len, fp);
    fclose(fp);
    sprintf(cmd, WRITE_CLEAR_FILE, offset);
    system(cmd);
    /* Wait for write cycle time (option -t) */
    usleep(WAIT_EEPROG);

    /* Remove the file */
    system(RM_CLEAR_FILE_NAME);

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : write_eeprom_block
 * Description : write EEPROM register by eeprog.
 * Inputs      : offset  - write from which offset
 *               w_data - data in hex
 *               w_length - write data length 
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */

static int write_eeprom_block (unsigned int offset, char *w_data, int w_length)
{
    FILE *fp;
    char cmd[MAX_COMMAND_LENGTH] = {0};
    
    /* This file is for eeprog writing with option -i */
    fp = fopen(WRITE_FILE_NAME, "wb+");
    if (fp < 0) {
        fclose(fp);
        printf("Can not open diag file");
        return (FAILED);
    }

    /* Write a string at a time */
    fwrite(w_data, sizeof(char), w_length, fp);
    fclose(fp);
    sprintf(cmd, WRITE_DIAG_FILE, offset);
    system(cmd);
    /* Wait for write cycle time (option -t) */
    usleep(WAIT_EEPROG);

    /* Remove the file */
    system(RM_WRITE_FILE_NAME);

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : sync_cookie_content_into_eeprom
 * Description : Sync SK/SV/SS/BS four fields from cookie into DMI EEPROM.
 * Inputs      : None
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int sync_cookie_content_into_eeprom (void)
{
    int ix = 0;
    char err[80];
    uint16_t id = 0;
    dev_if_info_t dev_if;
    sc_context *con, cont;
    uchar cookie[COOKIE_SIZE_512];
    char VSN[EEPROM_SS_LEN] = {0};
    char HWdot[HW_DOT_LEN] = {0x2e};
    char prodName[EEPROM_SK_LEN] = {0};
    char HW_version[EEPROM_SV_LEN] = {0};
    char pcaSerial[EEPROM_BS_LEN] = {0};
    char SK_buf[MAX_STRING_LEN] = "SK : ";
    char SV_buf[MAX_STRING_LEN] = "SV : ";
    char SS_buf[MAX_STRING_LEN] = "SS : ";
    char BS_buf[MAX_STRING_LEN] = "BS : ";
    
    /*
     * Dummy initialized for error message "cont.dev_if_p that
     * is not initialized."
     */
    con = &cont;
    con->slot = 0;
    con->dev_if_p = &dev_if;
    con->dev_if_p->cookie_size = COOKIE_SIZE_512;

    plat_init_smart_eeprom_context(con, MOTHER_BOARD, 0, cookie);
    act2_init_cont((void *) con);
    tam_act2_reset(0);
    if (get_cookie_id(cont.slot, 0, cookie, &id, err) == FAILED) {
        cterr('f', 0, "act2 program -- unable to read cookie id.");
        return (FAILED);
    }

    memset(prodName, 0, sizeof(prodName));
    memset(HW_version, 0, sizeof(HW_version));
    memset(VSN, 0, sizeof(VSN));
    memset(pcaSerial, 0, sizeof(pcaSerial));
    
    /* Clear SK field to NULL value first. */
    if (clear_field(EEPROM_SK_OFFSET, EEPROM_SK_LEN) == FAILED) {
        return (FAILED);
    } 
    /* Get PID (SK) */
    if (get_pid(cookie, prodName) == FAILED) {
        return (FAILED);
    }
    for (ix = 0; ix < (EEPROM_SK_LEN - TITLE_LEN); ix++) {
        SK_buf[ix + TITLE_LEN] = prodName[ix];
    }
    printf("\nProdName/Id:                %s\n", prodName);

    /* Write SK string into DMI EEPROM field from Cookie */
    if (write_eeprom_block(EEPROM_SK_OFFSET, SK_buf, strlen(SK_buf)) == FAILED) {
        return (FAILED);
    }

    /* Clear SV field to NULL value first. */
    if (clear_field(EEPROM_SV_OFFSET, EEPROM_SV_LEN) == FAILED) {
        return (FAILED);
    }
    /* Get Hardware version (SV) */
    if (get_hwv(HW_version) == FAILED) {
        return (FAILED);
    }
    /* There is no '.'(0x2e) character read out from cookie, so
     * need to add '.' between two HW revision values.
     * And the value read out from cookie is ASCII text, so need to
     * convert it into Hex value and write into EEPROM. 
     * For example:
     *     Cookie Hardware Revision       : 1.0, this is original value
     *     from cookie.
     *     However, get_hwv() function will get '10' string ASCII text 
     *     instead of '1.0'
     *     So need to add '.' between 10 to make it 1.0
     *     And need to convert ASCII text "1.0" into hex value "31 2e 20"
     *     Then save it into EEPROM. 
     */
    for (ix = 0; ix < (EEPROM_SV_LEN - TITLE_LEN); ix++) {
        /* Convert from ASCII text into Hex value and save into the EEPROM */
        if (ix == 0) {
            SV_buf[ix + TITLE_LEN] = CharToHex(HW_version[0] & 0xF);
        } else if (ix == 1){
            SV_buf[ix + TITLE_LEN] = HWdot[0];
        } else if (ix == 2){
            SV_buf[ix + TITLE_LEN] = CharToHex(HW_version[1] & 0xF);
        } else {
            SV_buf[ix + TITLE_LEN] = 0;
        } 
    }
    printf("Hardware Version:           %x.%x\n", HW_version[0] & 0xFF, 
           HW_version[1] & 0xFF); 
    /* Write SV string into DMI EEPROM field from Cookie */
    if (write_eeprom_block(EEPROM_SV_OFFSET, SV_buf, strlen(SV_buf)) == FAILED) {
        return (FAILED);
    }
    
    /* Clear SS field to NULL value first. */
    if (clear_field(EEPROM_SS_OFFSET, EEPROM_SS_LEN) == FAILED) {
        return (FAILED);
    }
    /* Get Device value-VSN (SS) */
    if (get_vsn(VSN) == FAILED) {
        return (FAILED);
    }
    for (ix = 0; ix < (EEPROM_SS_LEN - TITLE_LEN); ix++) {
        SS_buf[ix + TITLE_LEN] = VSN[ix];
    }
    printf("Device Value:               %s\n", VSN); 
    
    /* Write SS string into DMI EEPROM field from Cookie */
    if (write_eeprom_block(EEPROM_SS_OFFSET, SS_buf, strlen(SS_buf)) == FAILED) {
        return (FAILED);
    }

    /* Clear BS field to NULL value first. */
    if (clear_field(EEPROM_BS_OFFSET, EEPROM_BS_LEN) == FAILED) {
        return (FAILED);
    }
    /* Get PCA Serial (BS) */
    if (get_pca(pcaSerial) == FAILED) {
        return (FAILED);
    }
    for (ix = 0; ix < (EEPROM_BS_LEN - TITLE_LEN); ix++) {
        BS_buf[ix + TITLE_LEN] = pcaSerial[ix];
    }
    printf("PCA Serial:                 %s\n", pcaSerial); 
    
    /* Write BS string into DMI EEPROM field from Cookie */
    if (write_eeprom_block(EEPROM_BS_OFFSET, BS_buf, strlen(BS_buf)) == FAILED) {
        return (FAILED);
    }
    
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : init_eeprom_default
 * Description : write EEPROM default value.
 * Inputs      : None 
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int init_eeprom_default (void)
{
    FILE *fp;
    char cmd[MAX_COMMAND_LENGTH] = {0};
    char r_buf[MAX_STRING_LEN] = {0};

    /* Get hex data by eeprog tool */
    sprintf(cmd, CHECK_EEPROM_STATUS);
    ExecuteCmdbyPopen(cmd, r_buf, MAX_COMMAND_LENGTH);
    
    /* Check if EEPROM default value is EMPTY by eeprog tool, if it's empty, 
     * then the content will be like below dumped by eeprog tool. And need to
     * load default value.
     *     nutella-diag# eeprog -qxf /dev/i2c-0 0x54 -16 -t 10 -r 0x0:0x30
     *     0000|  ff ff ff ff ff ff ff ff   ff ff ff ff ff ff ff ff
     *     0010|  ff ff ff ff ff ff ff ff   ff ff ff ff ff ff ff ff
     *     0020|  ff ff ff ff ff ff ff ff   ff ff ff ff ff ff ff ff
     */
    if ((r_buf[9] == HEX_VAL_OF_F_CHAR) && (r_buf[10] == HEX_VAL_OF_F_CHAR) 
        && (r_buf[12] == HEX_VAL_OF_F_CHAR) && (r_buf[13] == HEX_VAL_OF_F_CHAR)) {
        /* This file is for eeprog writing with option -i */
        fp = fopen(INIT_FILE_NAME, "wb+");
        if (fp < 0) {
            fclose(fp);
            printf("Can not open init file");
            return (FAILED);
        }
        /* Init the DMI EEPROM */
        fwrite(default_eeprom_content, sizeof(char), NUTELLA_EEPROM_INIT_SIZE, fp);
        fclose(fp);
        sprintf(cmd, INIT_EEPROM);
        printf("Wait a minute...\n");
        system(cmd);
        /* Wait for write cycle time (option -t) */
        usleep(WAIT_EEPROG);

        /* Remove the file */
        system(RM_INIT_FILE);
    }

    return (PASSED);
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

    printf("%s: Nutella currently skip SKU ID check.\n", __FUNCTION__);
    sku_id_val = 0xFF;

    printf("\n Loading default cookie contents ...\n");
    switch (sku_id_val) {
    case 0xFF:
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
        printf("\nDBG :Select ACT2: tam_lib_device_open()\n");
        status = tam_lib_device_open(platform_opaque_handle,
                                 platform_buffer_size, &tam_handle_ptr);
        if (status != TAM_RC_OK) {
            printf("\n TAM lib: Cannot open handler: status = 0x%x", status);
            printf("\n Chip might be quack, not act2\n");
            return (FAILED);
        }
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
        /* User modified cookie content */
        modified_cookie_flag = TRUE;
    } else {
        /* User does not modify cookie content */
        modified_cookie_flag = FALSE;
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
    unsigned char buf[3000];
    unsigned int op = 0, is_addr = 1, ix;

    printf("\n\nSelect Act2 Chip:");
    act2_chip = getdec_answer("\n(0-Discrete-I2C-ACT2, 1-Aikido-SPI-ACT2,"
                                "2-Aikido-I2C-ACT):", 0, 0, 2);
    if (act2_chip == DISCRETE_I2C_ACT2) {
        aikido_act2_flag = FALSE;
        aikido_mailbox_flag = FALSE;
        printf("\nSelect Discrete ACT2(I2C)\n");
    } else if (act2_chip == AIKIDO_ACT2) {
        aikido_act2_flag = TRUE;
        aikido_mailbox_flag = TRUE;
        printf("\nSelect AIKIDO MAIL Box(SPI)\n");
        /* Check if Aikido FPGA is ready by checking the Aikido FPGA version Register. */
        memset(buf, 0, sizeof(buf));
        for (ix = 0; ix < AIKID_FPGA_TIME_OUT; ix++) {
            aikido_spi_read(AIKIDO_REG_SIZE, AIKIDO_FPGA_VER_REG, op, is_addr, buf);
            
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("\nAikido FPGA version register = %#x\n", buf[ix]);
            }
            
            if((buf[0] >= 4) && (buf[0] != 0xff)) {
                break;
            }
            msleep(ONE_SEC);
        }
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("\nWait %d seconds for Aikido FPGA ready...\n", ix);
        }
    } else if (act2_chip == AIKIDO_I2C_ACT2) {
        aikido_act2_flag = TRUE;
        aikido_mailbox_flag = FALSE;
        printf("\nSelect AIKIDO ACT2(I2C)\n");
    } else {
        printf("\n %s:%d Never come here \n", __FUNCTION__, __LINE__);
    }

    if (alter_cookie(MOTHER_BOARD) != PASSED ) {
        return (FAILED);
    }

    if (modified_cookie_flag == TRUE) {
        printf("\nCookie changed, sync DMI EEPROM content from corresponding "
               "cookie content.\n");
        /* Check if EEPROM is empty, if yes, init it. */
        if (init_eeprom_default() == FAILED) {
            printf("Init DMI EEPROM fail.\n");
            return (FAILED);
        }
        /* Sync DMI EEPROM content from cookie content. */
        if (sync_cookie_content_into_eeprom() == FAILED) {
            printf("Sync DMI EEPROM from Cookie fail.\n");
            return (FAILED);
        }
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
        act2_chip = getdec_answer("\n(0-Discrete-I2C-ACT2, 1-Aikido-SPI-ACT2,"
                                  "2-Aikido-I2C-ACT):", 0, 0, 2);
        if (act2_chip == DISCRETE_I2C_ACT2) {
            aikido_act2_flag = FALSE;
            aikido_mailbox_flag = FALSE;
            printf("\nSelect Discrete ACT2(I2C)\n");
        } else if (act2_chip == AIKIDO_ACT2) {
            aikido_act2_flag = TRUE;
            aikido_mailbox_flag = TRUE;
            printf("\nSelect AIKIDO MAIL Box(SPI)\n");
        } else if (act2_chip == AIKIDO_I2C_ACT2) {
            aikido_act2_flag = TRUE;
            aikido_mailbox_flag = FALSE;
            printf("\nSelect AIKIDO ACT2(I2C)\n");
        } else {
            printf("\n %s:%d Un-support chip.\n", __FUNCTION__, __LINE__); 
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
        if (aikido_act2_flag == TRUE) {
            con_p->dev_if_p->parm2 = (uint8_t)MB_I2C_ADDR_AIKIDO_ACT2;
        } else {
            con_p->dev_if_p->parm2 = (uint8_t)MB_I2C_ADDR_ACT2;
        }
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
    int ret_val;
 
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
    char prodName[PRODUCT_NAME_LEN] = {0};

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
/**************************************************************************
 *
 * Name: get_hwv
 *
 * Description: read hardware version
 *
 * Inputs: hwv - hardware version
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
int get_hwv(char *hwv)
{
    int ix;
    uint8_t *pdata = NULL;
    uint8_t ret_num_of_bytes = 0;
    char HWversion[HW_VERSION_LEN] = { 0 };

    if (platform_tam_handle == NULL) {
        /* cookie type is MOTHERBOARD */
        if (g_cookie_type == MOTHER_BOARD) {
            if (tam_lib_read_cookie() == FAILED) {
                return (FAILED);
            }
        }   
    }

    pdata = search_type_ret_addr_of_first_data(cookie_contents, HW_VERSION_TYPE,
                                               &ret_num_of_bytes, 0);
    if (pdata == NULL) {
        printf
            ("\n *** ERROR: Hardware Version field not programmed per Product PCAMAP.");
        return (FAILED);
    } else {
        ret_num_of_bytes =
            (ret_num_of_bytes >
             MAX_HWV_LEN) ? MAX_HWV_LEN : ret_num_of_bytes;
        for (ix = 0; ix < ret_num_of_bytes; ix++) {
            HWversion[ix] = *(pdata + ix);
        }
    }

    sprintf(hwv, "%s", HWversion);

    return (PASSED);
}
/**************************************************************************
 *
 * Name: get_vsn
 *
 * Description: read device value
 *
 * Inputs: vsn - device value
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
int get_vsn(char *vsn)
{
    int ix;
    uint8_t *pdata = NULL;
    uint8_t ret_num_of_bytes = 0;
    char VSN[VSN_LEN] = { 0 };

    if (platform_tam_handle == NULL) {
        /* cookie type is MOTHERBOARD */
        if (g_cookie_type == MOTHER_BOARD) {
            if (tam_lib_read_cookie() == FAILED) {
                return (FAILED);
            }
        }   
    }

    pdata = search_type_ret_addr_of_first_data(cookie_contents, VSN_TYPE,
                                               &ret_num_of_bytes, 0);
    if (pdata == NULL) {
        printf("\n *** ERROR: Device Value field not programmed per Product "
                "PCAMAP.");
        return (FAILED);
    } else {
        ret_num_of_bytes =
            (ret_num_of_bytes >
             MAX_VSN_LEN) ? MAX_VSN_LEN : ret_num_of_bytes;
        for (ix = 0; ix < ret_num_of_bytes; ix++) {
            VSN[ix] = *(pdata + ix);
        }
    }

    sprintf(vsn, "%s", VSN);

    return (PASSED);
}
/**************************************************************************
 *
 * Name: get_pca
 *
 * Description: read device value
 *
 * Inputs: pca - PCA Serial number
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
int get_pca(char *pca)
{
    int ix;
    uint8_t *pdata = NULL;
    uint8_t ret_num_of_bytes = 0;
    char pcaSerial[PCA_LEN] = {0};

    if (platform_tam_handle == NULL) {
        /* cookie type is MOTHERBOARD */
        if (g_cookie_type == MOTHER_BOARD) {
            if (tam_lib_read_cookie() == FAILED) {
                return (FAILED);
            }
        }   
    }

    pdata = search_type_ret_addr_of_first_data(cookie_contents, PCA_TYPE,
                                               &ret_num_of_bytes, 0);
    if (pdata == NULL) {
        printf("\n *** ERROR: PCA field not programmed per Product PCAMAP.");
        return (FAILED);
    } else {
        ret_num_of_bytes =
            (ret_num_of_bytes >
             MAX_PCA_LEN) ? MAX_PCA_LEN : ret_num_of_bytes;
        for (ix = 0; ix < ret_num_of_bytes; ix++) {
            pcaSerial[ix] = *(pdata + ix);
        }
    }

    sprintf(pca, "%s", pcaSerial);

    return (PASSED);
}

/**************************************************************************
 *
 * Name: get_part_number
 *
 * Description: read partnumber
 *
 * Inputs: pnu - part number
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
int get_part_number(char *pnu)
{
    int ix;
    uint8_t *pdata = NULL;
    uint8_t ret_num_of_bytes = 0;
    char partNUMBER[PART_NUMBER_LEN] = { 0 };

    if (platform_tam_handle == NULL) {
        /* cookie type is MOTHERBOARD */
        if (g_cookie_type == MOTHER_BOARD) {
            if (tam_lib_read_cookie() == FAILED) {
                return (FAILED);
            }
        }   
    }

    pdata = search_type_ret_addr_of_first_data(cookie_contents, PART_NUMBER_TYPE,
                                               &ret_num_of_bytes, 0);
    if (pdata == NULL) {
        printf
            ("\n *** ERROR: Part Number field not programmed per Product PCAMAP.");
        return (FAILED);
    } else {
        ret_num_of_bytes =
            (ret_num_of_bytes >
             PART_NUMBER_LEN) ? PART_NUMBER_LEN : ret_num_of_bytes;
        for (ix = 0; ix < ret_num_of_bytes; ix++) {
            partNUMBER[ix] = *(pdata + ix);
        }
    }

    sprintf(pnu,"%02d-%06d-%02d", (partNUMBER[0] & 0xFF)*256 + (partNUMBER[1] & 0xFF),
            ((partNUMBER[2] & 0xFF)*65536 + (partNUMBER[3] & 0xFF)*256 + (partNUMBER[4] & 0xFF)),
            partNUMBER[5] & 0xFF);

    return (PASSED);
}

/**************************************************************************
 *
 * Name: get_clei_code
 *
 * Description: Get CLEI code
 *
 * Inputs: clei_code - CLEI code
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
int get_clei_code(char *clei_code, int *get_clei_len)
{
    int ix;
    uint8_t *pdata = NULL;
    uint8_t ret_num_of_bytes = 0;
    char cleiCode[CLEI_CODE_LEN] = { 0 };

    if (platform_tam_handle == NULL) {
        /* cookie type is MOTHERBOARD */
        if (g_cookie_type == MOTHER_BOARD) {
            if (tam_lib_read_cookie() == FAILED) {
                return (FAILED);
            }
        }   
    }

    pdata = search_type_ret_addr_of_first_data(cookie_contents, CLEI_CODE_TYPE,
                                               &ret_num_of_bytes, 0);
    if (pdata == NULL) {
        printf
            ("\n *** ERROR: CLEI code field not programmed per Product PCAMAP.");
        return (FAILED);
    } else {
        ret_num_of_bytes =
            (ret_num_of_bytes >
             CLEI_CODE_LEN) ? CLEI_CODE_LEN : ret_num_of_bytes;
        for (ix = 0; ix < ret_num_of_bytes; ix++) {
            cleiCode[ix] = *(pdata + ix);
        }
    }

    *get_clei_len = ret_num_of_bytes;
    sprintf(clei_code, "%s", cleiCode);

    return (PASSED);
}

/**************************************************************************
 *
 * Name: get_vid
 *
 * Description: Get VID
 *
 * Inputs: vid_content - VID
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
int get_vid(char *vid_content)
{
    int ix;
    uint8_t *pdata = NULL;
    uint8_t ret_num_of_bytes = 0;
    char ver_id[VERID_LEN] = { 0 };

    if (platform_tam_handle == NULL) {
        /* cookie type is MOTHERBOARD */
        if (g_cookie_type == MOTHER_BOARD) {
            if (tam_lib_read_cookie() == FAILED) {
                return (FAILED);
            }
        }   
    }

    pdata = search_type_ret_addr_of_first_data(cookie_contents, VID_TYPE,
                                               &ret_num_of_bytes, 0);
    if (pdata == NULL) {
        printf
            ("\n *** ERROR: VID field not programmed per Product PCAMAP.");
        return (FAILED);
    } else {
        ret_num_of_bytes =
            (ret_num_of_bytes >
             VERID_LEN) ? VERID_LEN : ret_num_of_bytes;
        for (ix = 0; ix < ret_num_of_bytes; ix++) {
             ver_id[ix] = *(pdata + ix);
        }
    }

    sprintf(vid_content, "%s", ver_id);

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

/*-----------------------------------------------------------------------------
 *
 * Function get_mac_addr
 *
 * This function will return the MAC address.
 *
 * Inputs: eeprom_data - dummy parameters
 *         mac_addr - MAC address
 *
 * Returns : PASSED/FAILED
 */
int get_mac_addr(uchar *eeprom_data, char *mac_addr)
{
    int ix;
    uint8_t *pdata = NULL;
    uint8_t ret_num_of_bytes = 0;
    char mac_address[MAC_ADDRESS_LEN] = { 0 };

    if (platform_tam_handle == NULL) {
        /* cookie type is MOTHERBOARD */
        if (g_cookie_type == MOTHER_BOARD) {
            if (tam_lib_read_cookie() == FAILED) {
                return (FAILED);
            }
        } 
    }

    pdata =
        search_type_ret_addr_of_first_data(cookie_contents, CHASSIS_MAC_TYPE,
                                           &ret_num_of_bytes, 0);
    if (pdata == NULL) {
        printf
            ("\n *** ERROR: Chassis MAC Address field not programmed per Product PCAMAP.\n");
        return (FAILED);
    } else {
        ret_num_of_bytes =
            (ret_num_of_bytes >
             MAX_MAC_LEN) ? MAX_MAC_LEN : ret_num_of_bytes;
        for (ix = 0; ix < ret_num_of_bytes; ix++) {
            mac_address[ix] = *(pdata + ix);
        }
    }

    sprintf(mac_addr, "%s", mac_address);

    return (PASSED);
}

/*-----------------------------------------------------------------------------
 *
 * Function get_mac_addr_blk_size
 *
 * This function will return the MAC address block size
 *
 * Inputs: *mac_addr_blk_size - MAC address block size
 *
 * Returns : PASSED/FAILED
 */
int get_mac_addr_blk_size(char *mac_addr_blk_size)
{
    int ix;
    uint8_t *pdata = NULL;
    uint8_t ret_num_of_bytes = 0;
    char mac_blk_size[MAC_ADDR_BLK_LEN] = { 0 };

    if (platform_tam_handle == NULL) {
        /* cookie type is MOTHERBOARD */
        if (g_cookie_type == MOTHER_BOARD) {
            if (tam_lib_read_cookie() == FAILED) {
                return (FAILED);
            }
        } 
    }

    pdata =
        search_type_ret_addr_of_first_data(cookie_contents, MAC_ADDRESS_BLOCK_SIZE_TYPE,
                                           &ret_num_of_bytes, 0);
    if (pdata == NULL) {
        printf
            ("\n *** ERROR: MAC address block size field not programmed per Product PCAMAP.\n");
        return (FAILED);
    } else {
        ret_num_of_bytes =
            (ret_num_of_bytes >
             MAX_MAC_ADDR_BLK_LEN) ? MAX_MAC_ADDR_BLK_LEN : ret_num_of_bytes;
        for (ix = 0; ix < ret_num_of_bytes; ix++) {
            mac_blk_size[ix] = *(pdata + ix);
        }
    }

    sprintf(mac_addr_blk_size, "%02x%02x", mac_blk_size[0] & 0xFF, mac_blk_size[1] & 0xFF);

    return (PASSED);
}

/*-----------------------------------------------------------------------------
 *
 * Function get_mfg_test_data
 *
 * This function will return the Manufacturing Test Data
 *
 * Inputs: *mfg_test_data - Manufacturing Test Data Test Data
 *
 * Returns : PASSED/FAILED
 */
int get_mfg_test_data(char *mfg_test_data)
{
    int ix;
    uint8_t *pdata = NULL;
    uint8_t ret_num_of_bytes = 0;
    char mfg_test_d[MFG_TEST_DATA_LEN] = { 0 };

    if (platform_tam_handle == NULL) {
        /* cookie type is MOTHERBOARD */
        if (g_cookie_type == MOTHER_BOARD) {
            if (tam_lib_read_cookie() == FAILED) {
                return (FAILED);
            }
        } 
    }

    pdata =
        search_type_ret_addr_of_first_data(cookie_contents, MFG_TEST_DATA_TYPE,
                                           &ret_num_of_bytes, 0);
    if (pdata == NULL) {
        printf
            ("\n *** ERROR: MFG test data field not programmed per Product PCAMAP.\n");
        return (FAILED);
    } else {
        ret_num_of_bytes =
            (ret_num_of_bytes >
             MFG_TEST_DATA_LEN) ? MFG_TEST_DATA_LEN : ret_num_of_bytes;
        for (ix = 0; ix < ret_num_of_bytes; ix++) {
            mfg_test_d[ix] = *(pdata + ix);
        }
    }

    sprintf(mfg_test_data, "%s", mfg_test_d);

    return (PASSED);
}

/*-----------------------------------------------------------------------------
 *
 * Function get_controller_type
 *
 * This function will return the controller type
 *
 * Inputs: *controller_type - controller type
 *
 * Returns : PASSED/FAILED
 */
int get_controller_type(char *controller_type)
{
    int ix;
    uint8_t *pdata = NULL;
    uint8_t ret_num_of_bytes = 0;
    char cntr_type[PROCESSOR_TYPE_LEN] = { 0 };

    if (platform_tam_handle == NULL) {
        /* cookie type is MOTHERBOARD */
        if (g_cookie_type == MOTHER_BOARD) {
            if (tam_lib_read_cookie() == FAILED) {
                return (FAILED);
            }
        } 
    }

    pdata =
        search_type_ret_addr_of_first_data(cookie_contents, PROCESSOR_TYPE,
                                           &ret_num_of_bytes, 0);
    if (pdata == NULL) {
        printf
            ("\n *** ERROR: Controller type field not programmed per Product PCAMAP.\n");
        return (FAILED);
    } else {
        ret_num_of_bytes =
            (ret_num_of_bytes >
             PROCESSOR_TYPE_LEN) ? PROCESSOR_TYPE_LEN : ret_num_of_bytes;
        for (ix = 0; ix < ret_num_of_bytes; ix++) {
            cntr_type[ix] = *(pdata + ix);
        }
    }

    memcpy(controller_type, cntr_type, ret_num_of_bytes);

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
    tam_act2_reset(0);
    if (get_cookie_id(cont.slot, 0, cookie, &id, err)==FAILED) {
        cterr('f', 0, "act2 program -- unable to read cookie id.");
        return (FAILED);
    }

    get_pid(cookie, get_prodName);

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
    if (aikido_act2_flag == TRUE) {
        con->dev_if_p->parm2 = (uint8_t) MB_I2C_ADDR_AIKIDO_ACT2;
    } else {
        con->dev_if_p->parm2 = (uint8_t) MB_I2C_ADDR_ACT2;
    }
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

/*******************************************************************************
 *
 * Function    : read_eeprom_block
 * Description : Dummy function due to compile need for common source code
                 cookie_4_core.c:2520: undefined reference to `read_eeprom_block'
 * Inputs      : offset   - read form which offset
 *               size     - read how many byte
 *               buf      - return buffer
 * Outputs     : None
 *
 *******************************************************************************
 */
int read_eeprom_block (unsigned int offset, unsigned int size, unsigned char *buf)
{
    printf("%s: To be implemented\n", __FUNCTION__);
    return (FAILED);
}

/*-------------------------------------------------
$Log: platform_cookie.c,v $
Revision 1.5  2019/08/16 10:56:20  alicehua
Remove Discrete ACT2 related codes.(CDETS number:CSCvq77543)

Revision 1.4  2019/07/11 12:31:32  alicehua
Collapse Nutella codes into main trunk.

$Endlog$
*/
