 /* $Id: platform_cookie.c,v 1.3 2020/04/20 02:28:24 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/platform_cookie.c,v $
 *------------------------------------------------------------------
 *
 * platform_cookie.c - Specific MB cooke support from Xformers.
 *
 * Sept. 2008, Shih-Nan Huang
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <assert.h>
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
#include "platform_i2c.h"
#include "tam_library.h"
#include "tam_lib_manufacturing.h"
#include "dash_fpga.h"
#include "diag_fpga_i2c_lib.h"
#include "queryflags.h"
#include "i2c_api.h"
#include "n2g_api_rc.h"
#include "nvmonvars.h"
#include "slot.h"
#include "ngio.h"
#include "legacy_smart_cookie.h"

/***********************************************************************
 *  Macro Processor Definitions
 ************************************************************************/

/***********************************************************************
 *  Functions Declaration
 ************************************************************************/
int alter_nim_cookie(void);
int alter_mb_cookie(void);
int smartchip(int);
int tam_lib_read_cookie(void);
ushort get_cookie_id(int, int, uchar *, uint16_t *, char *);
int get_pid(uchar *, char *);
int get_part_number(char *);
int get_clei_code(char *, int *);
int get_vid(char *);
int get_pcb_serial(uchar *, char *);
int print_cookie(int, char *argv[]);
int platform_get_pid(char *);
static int alter_cookie(int, int);
static int32_t alter_ngio_cookie(cli_cookie_cmd *, ngio_if *, int);
int read_eeprom_block(unsigned int, unsigned int, unsigned char *);
int write_eeprom_block(unsigned int, char *);
int write_eeprom_hex(unsigned int, unsigned int);
int plat_init_smart_eeprom_context(sc_context *, uchar, uchar, uchar *);
int i2c_quack_read_bytes(sc_context *, char *);
int i2c_quack_write_bytes(sc_context *, char *, int);
int read_eeprom_util(int); 
int write_eeprom_util(int); 
int init_eeprom_default(int);
int alter_bios_eeprom(int);
int get_mac_addr(uchar *, char *);
int get_mac_addr_blk_size(char *);
int get_mfg_test_data(char *);
int get_controller_type(char *);
unsigned char CharToHex(unsigned char);
int clear_field(unsigned int, int);

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
extern struct ngio_intf_t *slot_get_ngiowic(int);
extern boolean aikido_act2_flag;
extern boolean aikido_mailbox_flag;
extern int slot_i2c_unreset(struct ngio_intf_t *, int, char *);

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
static uint8_t use_interrupt = 0;

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

static uchar default_eeprom_content[EEPROM_SIZE] = {
    /* 0x00 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x10 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x20 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x50, 0x61,
    /* 0x30 */
    0x72, 0x74, 0x20, 0x4E, 0x75, 0x6D, 0x62, 0x65,
    0x72, 0x3A, 0x20, 0x37, 0x34, 0x2D, 0x31, 0x32,
    /* 0x40 */
    0x32, 0x33, 0x33, 0x38, 0x2D, 0x30, 0x31, 0x00,
    0x43, 0x4C, 0x45, 0x49, 0x20, 0x43, 0x6F, 0x64,
    /* 0x50 */
    0x65, 0x3A, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x60 */
    0x00, 0x00, 0x50, 0x72, 0x6F, 0x63, 0x65, 0x73,
    0x73, 0x6f, 0x72, 0x20, 0x54, 0x79, 0x70, 0x65,
    /* 0x70 */
    0x3a, 0x20, 0x30, 0x78, 0x31, 0x30, 0x62, 0x37, 
    0x00, 0x4d, 0x41, 0x43, 0x20, 0x41, 0x64, 0x64, 
    /* 0x80 */
    0x72, 0x65, 0x73, 0x73, 0x20, 0x62, 0x6c, 0x6f, 
    0x63, 0x6b, 0x20, 0x73, 0x69, 0x7a, 0x65, 0x3a, 
    /* 0x90 */
    0x20, 0x30, 0x78, 0x30, 0x30, 0x00, 0x00, 0x00, 
    0x4d, 0x61, 0x6e, 0x75, 0x66, 0x61, 0x63, 0x74, 
    /* 0xA0 */
    0x75, 0x72, 0x69, 0x6e, 0x67, 0x20, 0x54, 0x65, 
    0x73, 0x74, 0x20, 0x44, 0x61, 0x74, 0x61, 0x3a, 
    /* 0xB0 */
    0x20, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 
    0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 
    /* 0xC0 */
    0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 
    0x00, 0x50, 0x43, 0x42, 0x20, 0x53, 0x65, 0x72, 
    /* 0xD0 */
    0x69, 0x61, 0x6c, 0x20, 0x4e, 0x75, 0x6d, 0x62, 
    0x65, 0x72, 0x3a, 0x20, 0x00, 0x00, 0x00, 0x00, 
    /* 0xE0 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x43, 0x68, 0x61, 0x73, 0x73, 0x69, 0x73, 0x20, 
    /* 0xF0 */
    0x4d, 0x41, 0x43, 0x20, 0x41, 0x64, 0x64, 0x72, 
    0x65, 0x73, 0x73, 0x3a, 0x20, 0x00, 0x00, 0x00, 
    /* 0x100 */
    0x00, 0x2e, 0x00, 0x00, 0x00, 0x00, 0x2e, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x56, 0x49, 0x44, 0x3a, 
    /* 0x110 */
    0x20, 0x56, 0x30, 0x30, 0x00, 0x4e, 0x61, 0x6d, 
    0x65, 0x3A, 0x20, 0x43, 0x68, 0x61, 0x73, 0x73, 
    /* 0x120 */
    0x69, 0x73, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    /* 0x130 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x44, 0x45, 0x53, 0x52, 
    /* 0x140 */
    0x3a, 0x20, 0x45, 0x6e, 0x74, 0x65, 0x72, 0x70, 
    0x72, 0x69, 0x73, 0x65, 0x20, 0x4e, 0x65, 0x74, 
    /* 0x150 */
    0x77, 0x6f, 0x72, 0x6b, 0x20, 0x43, 0x6f, 0x6d, 
    0x70, 0x75, 0x74, 0x65, 0x20, 0x53, 0x79, 0x73, 
    /* 0x160 */
    0x74, 0x65, 0x6d, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
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

/**************************************************************************
 *
 * Name: CharToHex 
 *
 * Description: ACSII change to 16 hex
 *
 * Inputs: bHex - ASCII code
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
 *         cookie_size - size of cookie
 *
 * Output: none
 *
 *------------------------------------------------------------------------*/
void init_cookie_4_default_x(int board_type, int cookie_type,
                        uchar * contents, int cookie_size)
{
    uint sku_id_val;

    printf("%s: Nanook currently skip SKU ID check.\n", __FUNCTION__);
    printf("FIXME, update default cookie\n");
    sku_id_val = 0xFF;

    switch (board_type) {
    case MOTHER_BOARD:
        printf("\nLoading default cookie format for Nanook Motherboard.\n");
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
        movbyte(default_cookie_contents, contents, cookie_size);
        break;
    }
}

/*******************************************************************************
 *
 * Function    : alter_bios_eeprom
 * Description : Alter/read/init BIOS EEPROM util.
 * Inputs      : opt - reserved for future use
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int alter_bios_eeprom (int opt)
{
    int sel_action, dummy = 0;

    sel_action = gethex_answer("Select action for EEPROM: 0/Read, 1/Write, "
                             "2/Init (0 ~ 2)", 0, 0, 2);
    
    switch(sel_action){
         case READ_BIOS_EEPROM:
             read_eeprom_util(dummy);
             break;
         case WRITE_BIOS_EEPROM:
             write_eeprom_util(dummy);
             break;
         case INIT_BIOS_EEPROM:
             init_eeprom_default(dummy);
             break;
    }
    
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : read_eeprom_util
 * Description : read EEPROM register util.
 * Inputs      : opt - reserved for future use
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */

int read_eeprom_util (int opt)
{
    uint reg_offset = 0, reg_size = 0;
    char reg_val[MAX_COMMAND_LENGTH] = {0};

    reg_offset = gethex_answer("Enter register offset (0x0 ~ 0x2000):",
            0, 0, 0x2000);
    reg_size = gethex_answer("Enter register size (0x1 ~ 0x2000):",
            0x1, 0x1, 0x2000);
    read_eeprom_block(reg_offset, reg_size, (unsigned char*)reg_val);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : read_eeprom_block
 * Description : read EEPROM register by eeprog.
 * Inputs      : offset - read form which offset
 *               size   - read how many byte
 *               buf    - return buffer
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */

int read_eeprom_block (unsigned int offset, unsigned int size, unsigned char* buf)
{
    char cmd[MAX_COMMAND_LENGTH] = {0};

    sprintf(cmd,"eeprog -qxf /dev/i2c-0 0x54 -16 -t 10 -r 0x%x:0x%x", offset, size);
    ExecuteCmdbyPopen(cmd, (char*)buf, MAX_COMMAND_LENGTH);
    printf("Data:%s\n",buf);
    return (PASSED); 
}

/*******************************************************************************
 *
 * Function    : write_eeprom_util
 * Description : write EEPROM register util.
 * Inputs      : opt - reserved for future use 
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */

int write_eeprom_util (int opt)
{
    uint reg_offset = 0, reg_size = 0;
    uint ix = 0, jx = 0;
    int choose = 0;
    char wr_buffer[64] = {0}, parse_mac[14] = {0}, hex_val[2] = {0};
    uint uuid_buf[14] = {0};
    char reg_val[MAX_COMMAND_LENGTH] = {0};
#ifdef MANUAL_INPUT_UUID
    uint reg_once = 1, ix = 0;
    uint orig_val = 0, reg_val_hex = 0 ;
    char orig_val_buf[MAX_COMMAND_LENGTH] = {0};
#endif

    choose = getdec_answer("Which part you want to change:\n1.PID:\n2.Part Number:\n3.CLEI Code:\n"
                           "4.Processor Type:\n5.MAC Block Size:\n6.MFG Test Data:\n7.PCB Serial Number:\n"
                           "8.Chassis MAC Address:\n9.VID:\n10.Name:\n11.DESR:\n",
                           1, 1, 11);
    
    memset(wr_buffer, 0, sizeof(wr_buffer));
    switch(choose){
         case BIOS_EEPROM_PID:
             printf("\nInput PID data (Format as: vCE9208-2)\n");
             reg_offset = PID_BASE_OFFSET;
             reg_size = EEPROM_PRODUCT_NAME_LEN;
             /* Clear this field to NULL value first. */
             clear_field(reg_offset, reg_size);
             /* Get input string */
             fgets(wr_buffer, sizeof(wr_buffer), stdin);
             printf("sizeof(wr_buffer): %#lx\n", sizeof(wr_buffer));
             /* Write string into EEPROM field */
             write_eeprom_block(reg_offset , wr_buffer);
             /* Show string field content in EEPROM */
             read_eeprom_block(reg_offset, reg_size, (unsigned char*)reg_val);
             break;
         case BIOS_EEPROM_PART_NUM:
             printf("\nInput Part Number (Format as: 74-122340-01)\n");
             reg_offset = PART_NUMBER_BASE_OFFSET;
             reg_size = EEPROM_PART_NUMBER_LEN;
             /* Clear this field to NULL value first. */
             clear_field(reg_offset, reg_size);
             /* Get input string */
             fgets(wr_buffer, sizeof(wr_buffer), stdin);
             printf("sizeof(wr_buffer): %#lx\n", sizeof(wr_buffer));
             /* Write string into EEPROM field */
             write_eeprom_block(reg_offset , wr_buffer);
             /* Show string field content in EEPROM */
             read_eeprom_block(reg_offset, reg_size, (unsigned char*)reg_val);
             break;
         case BIOS_EEPROM_CLEI_CODE:
             printf("\nInput CLEI Code (Format as: TBDTBDTBDT)\n");
             reg_offset = CLEI_CODE_OFFSET;
             reg_size = CLEI_CODE_LEN;
             /* Clear this field to NULL value first. */
             clear_field(reg_offset, reg_size);
             /* Get input string */
             fgets(wr_buffer, sizeof(wr_buffer), stdin);
             /* Write string into EEPROM field */
             write_eeprom_block(reg_offset , wr_buffer);
             /* Show string field content in EEPROM */
             read_eeprom_block(reg_offset, reg_size, (unsigned char*)reg_val);
             break;
         case BIOS_EEPROM_CTRL_TYPE:
             printf("\nInput Processor Type (Format as: 0x10B7)\n");
             reg_offset = PROCESSOR_TYPE_N0X_OFFSET;
             reg_size = EEPROM_PROCESSOR_TYPE_LEN;
             /* Clear this field to NULL value first. */
             clear_field(reg_offset, reg_size);
             /* Get input string */
             fgets(wr_buffer, sizeof(wr_buffer), stdin);
             /* Write string into EEPROM field */
             write_eeprom_block(reg_offset , wr_buffer);
             /* Show string field content in EEPROM */
             read_eeprom_block(reg_offset, reg_size, (unsigned char*)reg_val);
             break;
         case BIOS_EEPROM_MAC_BLK_SIZE:
             printf("\nInput MAC Block Size (Format as: 0x0090)\n");
             reg_offset = MAC_ADDR_BLK_N0X_SIZE_OFFSET;
             reg_size = EEPROM_MACADDR_LEN;
             /* Clear this field to NULL value first. */
             clear_field(reg_offset, reg_size);
             /* Get input string */
             fgets(wr_buffer, sizeof(wr_buffer), stdin);
             /* Write string into EEPROM field */
             write_eeprom_block(reg_offset , wr_buffer);
             /* Show string field content in EEPROM */
             read_eeprom_block(reg_offset, reg_size, (unsigned char*)reg_val);
             break;
         case BIOS_EEPROM_MFG_TEST_DATA:
             printf("\nInput Manufacturing Test Data (Format as: 00000000000000000000000)\n");
             reg_offset = MFG_TEST_DATA_OFFSET;
             reg_size = MFG_TEST_DATA_LEN;
             /* Clear this field to NULL value first. */
             clear_field(reg_offset, reg_size);
             /* Get input string */
             fgets(wr_buffer, sizeof(wr_buffer), stdin);
             /* Write string into EEPROM field */
             write_eeprom_block(reg_offset , wr_buffer);
             /* Show string field content in EEPROM */
             read_eeprom_block(reg_offset, reg_size, (unsigned char*)reg_val);
             break;
         case BIOS_EEPROM_PCB_SN:
             printf("\nInput SN data (Format as: FOCxxxxxxxx)\n");
             /* SN number starts from offset 0x12 */
             reg_offset = SN_BASE_OFFSET;
             reg_size = SN_LEN;
             /* Clear this field to NULL value first. */
             clear_field(reg_offset, reg_size);
             /* Get input string */
             fgets(wr_buffer, sizeof(wr_buffer), stdin);
             /* Write string into EEPROM field */
             write_eeprom_block(reg_offset , wr_buffer);
             /* Show string field content in EEPROM */
             read_eeprom_block(reg_offset, reg_size, (unsigned char*)reg_val);
             
             /* PCB Serial Number starts from offset 0xDC */
             reg_offset = PCB_SN_BASE_OFFSET;
             /* Clear this field to NULL value first. */
             clear_field(reg_offset, reg_size);
             /* Write string into EEPROM field */
             write_eeprom_block(reg_offset , wr_buffer);
             /* Show string field content in EEPROM */
             read_eeprom_block(reg_offset, reg_size, (unsigned char*)reg_val);
             break;
         case BIOS_EEPROM_CHASSIS_MAC:
             printf("\nInput MAC data (Format as: xxxx.xxxx.xxxx)\n");
             reg_offset = MAC_ADDRESS_BASE_OFFSET;
             reg_size = EEPROM_MACADDR_AND_COMMON_LEN;
             /* Clear this field to NULL value first. */
             clear_field(reg_offset, reg_size);
             /* Get input string */
             fgets(wr_buffer, sizeof(wr_buffer), stdin);
             /* Write string into EEPROM field */
             write_eeprom_block(reg_offset , wr_buffer);
             /* Show string field content in EEPROM */
             read_eeprom_block(reg_offset, reg_size, (unsigned char*)reg_val);
             
             /* Program UUID base on MAC address */
             memset(uuid_buf, 0, sizeof(uuid_buf));
             memset(parse_mac, 0, sizeof(parse_mac));
             
             /* Extract ACSII MAC address base on user Input data. */
             jx = 0;
             for (ix = 0; ix < EEPROM_MACADDR_AND_COMMON_LEN; ix++) {
                 if (wr_buffer[ix] != COMMON_ACSII_VAL) {
                     parse_mac[jx] = wr_buffer[ix];
                     jx += 1;
                 }
             }
             
             /* Convert ASCII MAC address into HEX number */
             for (ix = 0; ix < EEPROM_ASCII_MACADDR_LEN; ix++) {
                 /* Use only one byte char array to convert the HEX nunmber,
                  * Or the strtoul() function will convert start from the array pointer. */
                 hex_val[0] = parse_mac[ix];
                 if ((ix % 2) == 0) {
                     uuid_buf[ix/2] = (strtoul(&hex_val[0], NULL, 16) * 0x10);
                 } else {
                     uuid_buf[ix/2] = (uuid_buf[ix/2] + (strtoul(&hex_val[0], NULL, 16)));
                 }
             }
             
             /* Fill in the magic number and rand number after MAC address  */
             for (ix = 0; ix < UUID_LEN; ix++) {
                 if ((ix == UUID_MAGIC_NO_6) || (ix == UUID_MAGIC_NO_7)) {
                     uuid_buf[ix] = 0;
                 } else if (ix == UUID_MAGIC_NO_8) {
                     uuid_buf[ix] = UUID_MAGIC_NO;
                 } else if (ix > UUID_MAGIC_NO_8){
                     srand(ix);
                     uuid_buf[ix] = (rand() & 0xF);
                 }
             }

             /* Program UUID into EEPROM */
             reg_offset = UUID_BASE_OFFSET;
             reg_size = UUID_LEN;
             for (ix = 0; ix < reg_size ; ix++) {
                 write_eeprom_hex(reg_offset + ix, uuid_buf[ix]);
             }
             printf("\nProgram UUID done with below content:\n");
             for (ix = 0; ix < 0x10; ix++) {
                 printf("%#x ", uuid_buf[ix]);
             } 
             printf("\n");
             break;
         case BIOS_EEPROM_VID:
             printf("\nInput VID (Format as: V00)\n");
             reg_offset = VID_OFFSET;
             reg_size = EEPROM_VID_LEN;
             /* Clear this field to NULL value first. */
             clear_field(reg_offset, reg_size);
             /* Get input string */
             fgets(wr_buffer, sizeof(wr_buffer), stdin);
             /* Write string into EEPROM field */
             write_eeprom_block(reg_offset , wr_buffer);
             /* Show string field content in EEPROM */
             read_eeprom_block(reg_offset, reg_size, (unsigned char*)reg_val);
             break;
         case BIOS_EEPROM_NAME:
             printf("\nInput NAME data (Format as: Chassis)\n");
             reg_offset = NAME_OFFSET;
             reg_size = EEPROM_NAME_LEN;
             /* Clear this field to NULL value first. */
             clear_field(reg_offset, reg_size);
             /* Get input string */
             fgets(wr_buffer, sizeof(wr_buffer), stdin);
             /* Write string into EEPROM field */
             write_eeprom_block(reg_offset , wr_buffer);
             /* Show string field content in EEPROM */
             read_eeprom_block(reg_offset, reg_size, (unsigned char*)reg_val);
             break;
         case BIOS_EEPROM_DESR:
             printf("\nInput DESR data (Format as: Enterprise Network Compute System)\n");
             reg_offset = DESR_OFFSET;
             reg_size = EEPROM_DESR_LEN;
             /* Clear this field to NULL value first. */
             clear_field(reg_offset, reg_size);
             /* Get input string */
             fgets(wr_buffer, sizeof(wr_buffer), stdin);
             /* Write string into EEPROM field */
             write_eeprom_block(reg_offset , wr_buffer);
             /* Show string field content in EEPROM */
             read_eeprom_block(reg_offset, reg_size, (unsigned char*)reg_val);
             break;
#ifdef MANUAL_INPUT_UUID
         case 4:
             printf("\nInput UUID data (16 bytes of hex)(Format as: aa)(Type in a hex at a time, you need to type in 16 times)\n");
             reg_offset = UUID_BASE_OFFSET;
             reg_size = UUID_LEN;
             for (ix = 0; ix < reg_size ; ix++)
             {
                 read_eeprom_block(reg_offset + ix, reg_once, (unsigned char*)orig_val_buf);
                 orig_val = strtoul(&orig_val_buf[EEPORG_DATA_START], NULL, 16);
                 reg_val_hex = gethex_answer("Enter write-in data(hex): "
                                            , orig_val, 0, 0xff);
                 
                 write_eeprom_hex(reg_offset + ix, reg_val_hex);
             }

             break;
#endif
         default:
             printf("\nPlease try again\n");
             break;
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

int write_eeprom_hex (unsigned int offset,  unsigned int wr_data)
{
    FILE *fp;
    char cmd[MAX_COMMAND_LENGTH] = {0};
    unsigned int write_buf[1] = {0};

    /* This file is for eeprog writing with option -i */
    fp = fopen("./diag.bin","wb+");
    if (fp < 0) {
        fclose(fp);
        printf("Can not open diag file");
        return (FAILED);
    }

    write_buf[0] = wr_data; 
    /* Write a word at a time */
    fwrite(write_buf, sizeof(unsigned int)/4, 1, fp);
    fclose(fp);
    sprintf(cmd,"eeprog -q -f -16 -i diag.bin -w 0x%x -t 10 /dev/i2c-0 0x54", offset);
    system(cmd);
    /* Wait for write cycle time (option -t) */
    usleep(WAIT_EEPROG);

    /* Remove the file */
    system("rm diag.bin");
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

int clear_field (unsigned int offset, int field_len)
{
    FILE *fp;
    char c_data[MAX_COMMAND_LENGTH] = {0};
    char cmd[MAX_COMMAND_LENGTH] = {0};
    
    /* This file is for eeprog writing with option -i */
    fp = fopen("./clear.bin","wb+");
    if (fp < 0) {
        fclose(fp);
        printf("Can not open diag file");
        return (FAILED);
    }

    /* Last character is Enter */
    fwrite(c_data, sizeof(char), field_len, fp);
    fclose(fp);
    sprintf(cmd,"eeprog -q -f -16 -i clear.bin -w 0x%x -t 10 /dev/i2c-0 0x54", offset);
    system(cmd);
    /* Wait for write cycle time (option -t) */
    usleep(WAIT_EEPROG);

    /* Remove the file */
    system("rm clear.bin");

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : write_eeprom_block
 * Description : write EEPROM register by eeprog.
 * Inputs      : offset  - write from which offset
 *               wr_data - data in hex
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */

int write_eeprom_block (unsigned int offset,  char* wr_data)
{
    FILE *fp;
    char cmd[MAX_COMMAND_LENGTH] = {0};
    int count=0;
    
    /* This file is for eeprog writing with option -i */
    fp = fopen("./diag.bin","wb+");
    if (fp < 0) {
        fclose(fp);
        printf("Can not open diag file");
        return (FAILED);
    }

    /* Write a string at a time */
    count = strlen(wr_data);
    /* Last character is Enter */
    fwrite(wr_data, sizeof(char),count-1, fp);
    fclose(fp);
    sprintf(cmd,"eeprog -q -f -16 -i diag.bin -w 0x%x -t 10 /dev/i2c-0 0x54", offset);
    system(cmd);
    /* Wait for write cycle time (option -t) */
    usleep(WAIT_EEPROG);

    /* Remove the file */
    system("rm diag.bin");

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : init_eeprom_default
 * Description : write EEPROM default value.
 * Inputs      : opt - option for future use.
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int init_eeprom_default (int opt)
{
    FILE *fp;
    char cmd[MAX_COMMAND_LENGTH] = {0};
    uint16_t id = 0;
    char err[80];
    char prodName[EEPROM_PRODUCT_NAME_LEN] = {0};
    char partNumber[EEPROM_PART_NUMBER_LEN] = {0};
    char pcbSN[EEPROM_PCBSN_LEN] = {0}; 
    char MACAddrBlkSize[EEPROM_MACADDR_BLK_LEN] = {0}; 
    char MFGTestData[MFG_TEST_DATA_LEN] = {0}; 
    char ctrl_type[PROCESSOR_TYPE_LEN] = {0}; 
    char clei_code_data[CLEI_CODE_LEN] = {0}; 
    char vid_data[VERID_LEN] = {0}; 
    char macADDR[EEPROM_MACADDR_LEN] = {0}, temp_macADDR[EEPROM_ASCII_MACADDR_LEN] = {0};
    uchar cookie[COOKIE_SIZE_512];
    sc_context *con, cont;
    dev_if_info_t dev_if;
    int ix = 0, jx = 0, clei_len = 0;

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
    memset(partNumber, 0, sizeof(partNumber));
    memset(pcbSN, 0, sizeof(pcbSN));
    memset(macADDR, 0, sizeof(macADDR));
    memset(MACAddrBlkSize, 0, sizeof(MACAddrBlkSize));
    memset(MFGTestData, 0, sizeof(MFGTestData));
    memset(ctrl_type, 0, sizeof(ctrl_type));
    memset(clei_code_data, 0, sizeof(clei_code_data));
    memset(vid_data, 0, sizeof(vid_data));

    /* Get Controller Type */
    get_controller_type(ctrl_type);
    printf("\n");
    printf("Processor Type:             0x%x%x", ctrl_type[0] & 0xFF, ctrl_type[1] & 0xFF);
    for (ix = 0; ix < PROCESSOR_TYPE_LEN; ix++) {
        /* Convert Hex value into ASCII code and save into the EEPROM */
        if (ix == 0) {
            default_eeprom_content[ix + PROCESSOR_TYPE_OFFSET] = 
                CharToHex(((ctrl_type[0] & 0xF0) >> 4) & 0xF);
        } else if (ix == 1){
            default_eeprom_content[ix + PROCESSOR_TYPE_OFFSET] = 
                CharToHex(ctrl_type[0] & 0xF);
        } else if (ix == 2){
            default_eeprom_content[ix + PROCESSOR_TYPE_OFFSET] = 
                CharToHex(((ctrl_type[1] & 0xF0) >> 4) & 0xF);
        } else {
            default_eeprom_content[ix + PROCESSOR_TYPE_OFFSET] = 
                CharToHex(ctrl_type[1] & 0xF);
        }
    }
    printf("\n");
    
    /* Get PID */
    get_pid(cookie, prodName);
    printf("ProdName/Id:                %s\n", prodName); 
    for (ix = 0; ix < EEPROM_PRODUCT_NAME_LEN; ix++) {
        default_eeprom_content[ix] = prodName[ix];
    }
    
    /* Get Part Number */
    get_part_number(partNumber);
    printf("Part Number:                %s\n", partNumber); 
    for (ix = 0; ix < EEPROM_PART_NUMBER_LEN; ix++) {
        default_eeprom_content[ix + PART_NUMBER_BASE_OFFSET] = 
            partNumber[ix];
    }
    
    /* Get CLEI Code */
    get_clei_code(clei_code_data, &clei_len);
    printf("CLEI Code:                  %s\n", clei_code_data); 
    for (ix = 0; ix < clei_len; ix++) {
        default_eeprom_content[ix + CLEI_CODE_OFFSET] 
            = clei_code_data[ix];
    }

    /* Get PCB SN */
    get_pcb_serial(cookie, pcbSN);
    printf("PCB Serial Num:             %s\n", pcbSN);
    for (ix = 0; ix < EEPROM_PCBSN_LEN; ix++) {
        default_eeprom_content[ix + SN_BASE_OFFSET] = pcbSN[ix];
        default_eeprom_content[ix + PCB_SN_BASE_OFFSET] = pcbSN[ix];
    }
    
    /* Get VID */
    get_vid(vid_data);
    printf("VID:                        ");
    for (ix = 0; ix < (VERID_LEN - 1); ix++) {
        default_eeprom_content[ix + VID_OFFSET] = vid_data[ix];
        printf("%c", default_eeprom_content[ix + VID_OFFSET]); 
    }
    printf("\n");
    
    /* Get MAC address block size */
    get_mac_addr_blk_size(MACAddrBlkSize);
    printf("MAC Address block size:     0x%s", MACAddrBlkSize); 
    for (ix = 0; ix < MAC_BLK_LEN; ix++) {
        default_eeprom_content[ix + MAC_ADDR_BLK_SIZE_OFFSET] = 
            MACAddrBlkSize[ix];
    }
    printf("\n");
    
    /* Get Manufacturing Test Data */
    get_mfg_test_data(MFGTestData);
    printf("Manufacturing Test Data:    "); 
    for (ix = 0; ix < MFG_TEST_DATA_LEN; ix++) {
        printf("%x ", MFGTestData[ix]); 
        default_eeprom_content[ix + MFG_TEST_DATA_OFFSET] = CharToHex(MFGTestData[ix] & 0xFF);
    }
    printf("\n");

    /* Get MAC address */
    get_mac_addr(cookie, macADDR);
    printf("Program UUID:               ");
     /* Fill in the magic number and rand number after MAC address  */
    for (ix = 0; ix < UUID_LEN; ix++) {
        if (ix < EEPROM_MACADDR_LEN) {
            default_eeprom_content[ix + UUID_BASE_OFFSET] = (macADDR[ix] & 0xFF);
        } else if ((ix == UUID_MAGIC_NO_6) || (ix == UUID_MAGIC_NO_7)) {
            default_eeprom_content[ix + UUID_BASE_OFFSET] = 0;
        } else if (ix == UUID_MAGIC_NO_8) {
            default_eeprom_content[ix + UUID_BASE_OFFSET] = UUID_MAGIC_NO;
        } else if (ix > UUID_MAGIC_NO_8){
            srand(ix);
            default_eeprom_content[ix + UUID_BASE_OFFSET] = (rand() & 0xF);
        }
        printf("%x ", default_eeprom_content[ix + UUID_BASE_OFFSET]);
    }
    printf("\n");

    printf("Program ASCII MAC address:  ");
    /* Convert MAC address from one hex value in a byte into two hex values in separate bytes.
     * EX. 0x12 in a byte -> 0x1 in first byte, 0x2 in second byte.  */
    for (ix = 0; ix < EEPROM_ASCII_MACADDR_LEN; ix++) {
        if ((ix % 2) == 0) {
            temp_macADDR[ix] = ((macADDR[ix/2] >> 4) & 0xFF);
        } else {
            temp_macADDR[ix] = (macADDR[ix/2] & 0xF);
        }
    }

    /* Program ACSII MAC address */
    jx = 0;
    for (ix = 0; ix < EEPROM_UUID_MACADDR_LEN; ix++) {
        if ((ix != UUID_COMMON_POS_4) && (ix != UUID_COMMON_POS_9)) {
            /* Assign ASCII MAC address into default bufgfer */
            default_eeprom_content[ix + MAC_ADDRESS_BASE_OFFSET] = CharToHex(temp_macADDR[jx]);
            jx = jx + 1;
        } else {
            /* Assign ASCII 0x2e -> '.' into default buffer  */
            default_eeprom_content[ix + MAC_ADDRESS_BASE_OFFSET] = COMMON_ACSII_VAL;
        }
        printf("%x ", default_eeprom_content[ix + MAC_ADDRESS_BASE_OFFSET]);
    }  
    printf("\n");
 
    /* This file is for eeprog writing with option -i */
    fp = fopen("./init.bin","wb+");
    if (fp < 0) {
        fclose(fp);
        printf("Can not open init file");
        return (FAILED);
    }
    fwrite(default_eeprom_content, sizeof(char), EEPROM_SIZE, fp);
    fclose(fp);
    sprintf(cmd,"eeprog -q -f -16 -i init.bin -w 0x0 -t 10 /dev/i2c-0 0x54");
    system(cmd);
    /* Wait for write cycle time (option -t) */
    usleep(WAIT_EEPROG);

    /* Remove the file */
    system("rm init.bin");

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

    printf("%s: Nanook currently skip SKU ID check.\n", __FUNCTION__);
    printf("FIXME\n");
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
 *   board_type - board type
 *   slot - Slot number
 *
 * Returns:
 *   PASSED/FAILED
 */
static int alter_cookie (int board_type, int slot)
{
    dev_if_info_t dev_if;
    void *tam_handle_ptr;
    void *platform_opaque_handle = NULL;
    uint16_t platform_buffer_size = PLATFORM_BUFF_SIZE;
    uint8_t cookie_contents_buf[COOKIE_SIZE_512];
    tam_lib_status_t status;
    int ret_val;
    sc_context *con, cont;

    /* COOKIE Init. */
    if ((slot == 0) || (board_type == WIC_MODULE)) {
        con = &cont;
        con->dev_if_p = &dev_if;
        con->dev_if_p->cookie_size = COOKIE_SIZE_512;
        if (plat_init_smart_eeprom_context(con, board_type, slot, 
                                          (uchar *)cookie_contents_buf) == FAILED) {
            printf("\n%s: Init Smart EEPROM context failed\n", __func__);
            return (FAILED);
        }

        act2_init_cont(con);
        con->quack_reset(con);
    }

    if ((board_type != MOTHER_BOARD)) {
        tam_act2_reset(0);
        if (!cookie_is_act2(con)) { /* quack, return failed,
                                     * tam lib does not support quack */
            printf("Cookie utility doesn't support Quack chip!\n");
            return (FAILED);
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
static int32_t alter_ngio_cookie (cli_cookie_cmd *cmd, ngio_if *ngio, int type)
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

/**************************************************************************
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
 *************************************************************************/
int alter_nim_cookie (void)
{
    int slot = FIRST_SLOT;
    struct ngio_intf_t *ngio;

    if (is_nanook_plus()) {
        slot = FIRST_SLOT + 2;
    } else {
        slot = FIRST_SLOT + 1;
    }

    ngio = (struct ngio_intf_t *)slot_get_ngiowic(slot);
    
    if (slot_i2c_unreset(ngio, slot, "WIC") < 0) {
        return (FAILED);
    }
    
    return (alter_ngio_cookie(NULL, ngio, WIC_MODULE));
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

    act2_chip = getdec_answer("\n(1-Aikido-ACT2):", 1, 1, 1);
    if (act2_chip == 1) {
        aikido_act2_flag = TRUE;
        aikido_mailbox_flag = TRUE;
        printf("\nSelect AIKIDO MAIL Box\n");
    } else {
        printf("\n %s:%d Never come here \n", __FUNCTION__, __LINE__);
    }

    if (alter_cookie(MOTHER_BOARD, 0) != PASSED ) {
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
        act2_chip = getdec_answer("\n(1-Aikido-ACT2):", 1, 1, 1);
        if (act2_chip == 1) {
            aikido_act2_flag = TRUE;
            aikido_mailbox_flag = TRUE;
            printf("\nSelect AIKIDO MAIL Box\n");
        } else {
             printf("\n %s:%d Never come here \n", __FUNCTION__, __LINE__);
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
    int retval = PASSED;
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
            sprintf((char *)smc_buf, "DC (WIC%d)", slot);
            /* check if dc is nim or sm */
            ngiowic = (struct ngio_intf_t *)slot_get_ngiowic(slot);
            sprintf((char *)smc_buf, "DC (WIC%d)", slot);
            
            init_ngio_context(con_p, type, FIRST_SLOT, cookie_p);
            con_p->dev_if_p->parm2 = (uint8_t)NGIOVM_I2C_ADDR_ACT2;
            /*  printf("wic daughter: i2c ctrl no %d; addr %#x\n",
                con_p->dev_if_p->parm4, con_p->dev_if_p->parm2);
            */
        }
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
    sc_context *con, cont;
    dev_if_info_t dev_if;

    /* Init cookie related settings*/
    g_cookie_type = type;
    
    if (g_cookie_type == MOTHER_BOARD) {
        /* cookie type is MOTHERBOARD */
        if (tam_lib_read_cookie() == FAILED) {
            return (FAILED);
        } 
    } else {
        con = &cont;
        con->dev_if_p = &dev_if;
        con->dev_if_p->cookie_size = COOKIE_SIZE_512;
       
        plat_init_smart_eeprom_context(con, type, slot, cookie_contents);
        act2_init_cont(con);
       
        if (cookie_is_act2(con)) {
            printf("Detecting ACT2 chip...\n");
            if (tam_lib_read_cookie() == FAILED) {
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
    if (get_sku_flag == FALSE) {
        get_sku_flag = TRUE;
        tam_act2_reset(0);
        if (get_cookie_id(cont.slot, 0, cookie, &id, err)==FAILED) {
            cterr('f', 0, "BIOS EEPROM program -- unable to read cookie id.");
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

    printf("TBD get_cookie_pid\n");

#ifdef OWEN_TBD
    if(smart_cookie_read(con)==PASSED) {
#endif
    if (1) {
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

void clean_smart_eeprom_context (sc_context *con_p)
{
        printf("To be developed...\n");
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

/*-------------------------------------------------
 * $Log: platform_cookie.c,v $
 * Revision 1.3  2020/04/20 02:28:24  lucywang
 *
 * 1. Fixed unplug/plug NIM module dynamically issue and added NIM cookie
 * 2. Added to support NIM Prince
 * 3. (CSCvn43011) add retry workaround for Deverton issue
 * 4. add debug message and set default value to seneors
 * 5. Reverted Register value of temp/press snsr after test
 * 6. Bumped up version to 1.0.2
 *
 * Revision 1.2  2019/12/11 10:10:33  lucywang
 * Merged Nanook to main trunk
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
