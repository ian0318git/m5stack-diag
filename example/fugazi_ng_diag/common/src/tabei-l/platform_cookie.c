 /* $Id: platform_cookie.c,v 1.7 2020/12/17 07:19:05 kodko Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/platform_cookie.c,v $
 *------------------------------------------------------------------
 *
 * platform_cookie.c - Specific MB cooke support from Xformers.
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
#include "platform_i2c.h"
#include "tam_library.h"
#include "tam_lib_manufacturing.h"
#include "diag_fpga.h"
#include "queryflags.h"
#include "i2c_api.h"
#include "n2g_api_rc.h"
#include "nvmonvars.h"
#include "diag_fpga_i2c_lib.h"
#include "ngio.h"
#include "plug_slot.h"
#include "legacy_smart_cookie.h"
#include "slot.h"
#include "assert.h"

/***********************************************************************
 *  Macro Processor Definitions
 ************************************************************************/

/***********************************************************************
 *  Functions Declaration
 ************************************************************************/
int alter_mb_cookie(void);
int alter_pim_cookie(void);
int alter_nim_cookie(void);
int alter_nim_dc_cookie(void);
int smartchip(int);
int tam_lib_read_cookie(void);
ushort get_cookie_id(int, int, uchar *, uint16_t *, char *);
int get_pid(uchar *, char *);
int get_part_number(char *);
int get_clei_code(char *, int *);
int get_vid(char *);
int get_pcb_serial(uchar *, char *);
int get_chassis_serial(uchar *, char *);
int print_cookie(int, char *argv[]);
int platform_get_pid(char *);
static int alter_cookie(int, int);
int read_eeprom_block(unsigned int, unsigned int, unsigned char *);
int write_eeprom_block(unsigned int, char *, int);
int write_eeprom_hex(unsigned int, unsigned int);
int plat_init_smart_eeprom_context(sc_context *, uchar, uchar, uchar *);
int i2c_quack_read_bytes(sc_context *, char *);
int i2c_quack_write_bytes(sc_context *, char *, int);
void read_eeprom_util(void); 
void write_eeprom_util(void); 
int init_eeprom_default(void);
int alter_bios_eeprom(int);
int get_mac_addr(uchar *, char *);
int get_mac_addr_blk_size(char *);
int get_mfg_test_data(char *);
int get_controller_type(char *);
unsigned char CharToHex(unsigned char);
int clear_field(unsigned int, int);
void get_input_string(char *, int);
static int32_t alter_ngio_cookie(cli_cookie_cmd *, ngio_if *, int);
void display_eeprom_hex_content(unsigned int, unsigned int);
void display_eeprom_string(void);
void clear_entire_eeprom_to_zero(void);
int i2c_act2_write_bytes(sc_context *, char *, int);
int i2c_act2_read_bytes(sc_context *, char *);
void i2c_act2_reset(sc_context *);
boolean aikido_act2_flag;
boolean aikido_mailbox_flag;


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
static boolean modified_cookie_flag = FALSE;
static dev_if_info_t dev_if;

static uchar default_mb_cookie[COOKIE_SIZE_512] = {
    0x04, 0xFF, 0x40, 0x12, 0x35, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
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

static uchar default_eeprom_content[TABEI_INIT_EEPROM_SIZE] = {
    /* 0x00 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    /* 0x10 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    /* 0x20 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    /* 0x30 */
    0x0A, 0x50, 0x61, 0x72, 0x74, 0x20, 0x4E, 0x75, 
    0x6D, 0x62, 0x65, 0x72, 0x3A, 0x20, 0x00, 0x00, 
    /* 0x40 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x43, 0x4C, 0x45, 0x49, 
    /* 0x50 */
    0x20, 0x43, 0x6F, 0x64, 0x65, 0x3A, 0x20, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    /* 0x60 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x50, 
    0x72, 0x6F, 0x63, 0x65, 0x73, 0x73, 0x6f, 0x72, 
    /* 0x70 */
    0x20, 0x54, 0x79, 0x70, 0x65, 0x3a, 0x20, 0x30, 
    0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4d, 
    /* 0x80 */
    0x41, 0x43, 0x20, 0x41, 0x64, 0x64, 0x72, 0x65,
    0x73, 0x73, 0x20, 0x62, 0x6c, 0x6f, 0x63, 0x6b, 
    /* 0x90 */
    0x20, 0x73, 0x69, 0x7a, 0x65, 0x3a, 0x20, 0x30, 
    0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4d, 
    /* 0xA0 */
    0x61, 0x6e, 0x75, 0x66, 0x61, 0x63, 0x74, 0x75, 
    0x72, 0x69, 0x6e, 0x67, 0x20, 0x54, 0x65, 0x73,
    /* 0xB0 */
    0x74, 0x20, 0x44, 0x61, 0x74, 0x61, 0x3a, 0x20, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    /* 0xC0 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    /* 0xD0 */
    0x00, 0x50, 0x43, 0x42, 0x20, 0x53, 0x65, 0x72, 
    0x69, 0x61, 0x6c, 0x20, 0x4e, 0x75, 0x6d, 0x62, 
    /* 0xE0 */
    0x65, 0x72, 0x3a, 0x20, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    /* 0xF0 */
    0x00, 0x43, 0x68, 0x61, 0x73, 0x73, 0x69, 0x73, 
    0x20, 0x4d, 0x41, 0x43, 0x20, 0x41, 0x64, 0x64, 
    /* 0x100 */
    0x72, 0x65, 0x73, 0x73, 0x3a, 0x20, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    /* 0x110 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x56, 0x49, 
    0x44, 0x3a, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 
    /* 0x120 */
    0x4e, 0x61, 0x6d, 0x65, 0x3a, 0x20, 0x43, 0x68, 
    0x61, 0x73, 0x73, 0x69, 0x73, 0x00, 0x00, 0x00, 
    /* 0x130 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    /* 0x140 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x44, 0x45, 0x53, 0x52, 0x3a, 0x20, 0x45, 0x6e, 
    /* 0x150 */
    0x74, 0x65, 0x72, 0x70, 0x72, 0x69, 0x73, 0x65, 
    0x20, 0x4e, 0x65, 0x74, 0x77, 0x6f, 0x72, 0x6b, 
    /* 0x160 */
    0x20, 0x43, 0x6f, 0x6d, 0x70, 0x75, 0x74, 0x65, 
    0x20, 0x53, 0x79, 0x73, 0x74, 0x65, 0x6d, 0x00, 
    /* 0x170 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
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

    printf("%s: Tabei currently skip SKU ID check.\n", __FUNCTION__);
    sku_id_val = 0xFF;

    switch (board_type) {
    case MOTHER_BOARD:
        printf("\nLoading default cookie format for Tabei Motherboard.\n");
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


/**********************************************************************
 *
 * Function: smart_cookie_read_write_eeprom
 *
 * Description: Smart cookie read/write eeprom
 *
 * Input:  con   - pointer to sc_context
 *         cli_cmd - pointer to cli_cookie_cmd
 *
 * Output: PASSED/FAILED
 *
 *********************************************************************
 */
int smart_cookie_read_write_eeprom (sc_context * con, cli_cookie_cmd * cli_cmd)
{
    if (act2_is_simple_mode(con)) {
        cterr('f', 0, "device is in simple mode. can't access Act1 space.");
        return (FAILED);
    }

    if (con->dev_if_p->cookie_size != COOKIE_SIZE_128) {
        if(smart_cookie_read_x(con, con->dev_if_p->cookie_size)) {
            printf("failed platform_cookie.c @%d\n", __LINE__);
            return (FAILED);
        }
        if (cookie_4_processor_x(con->cookie_contents, con->type,
                                con->slot, con->dev_if_p->cookie_size, cli_cmd)) {
            if(smart_cookie_write_x(con, con->cookie_contents,
                                 con->dev_if_p->cookie_size))
                 return (FAILED);
        }
     } else {   /* size 128; should not come here for new new platforms (ovd, or newer ) */
        if(smart_cookie_read(con)) {
            return (FAILED);
        }
        if (cookie_4_processor_x(con->cookie_contents, con->type,
                                con->slot, con->dev_if_p->cookie_size, cli_cmd)) {
            if(smart_cookie_write(con, con->cookie_contents))
            return (FAILED);
        }
    }

    return (PASSED);
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
    int sel_action;

    sel_action = gethex_answer("Select action for EEPROM: 0/Read, 1/Write: "
                               , 0, 0, 1);
    
    switch (sel_action) {
         case READ_BIOS_EEPROM:
             read_eeprom_util();
             break;
         case WRITE_BIOS_EEPROM:
             write_eeprom_util();
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
    display_eeprom_string();
}

/*******************************************************************************
 *
 * Function    : display_eeprom_hex_content 
 * Description : Show EEPROM content for binary content by eeprog.
 * Inputs      : offset - read form which offset
 *               size   - read how many byte
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
 * Description : Show EEPROM human readable content from BIOS by dmidecode.
 * Inputs      : None 
 * Outputs     : None
 *
 *******************************************************************************
 */
void display_eeprom_string (void)
{
    printf("\n");
    /* Show readable string from BIOS by dmidecode tool due to enable TSOD 
     * feature will make eeprog read incorrect character. And TSDO can 
     * only be disabled in BIOS in Denvertion CPU (CSCvp97974). */
    system(SHOW_DMIDECODE_STRING);
    printf("\n");
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
 * Outputs     : None 
 *
 *******************************************************************************
 */

void write_eeprom_util (void)
{
    int choose = 0;
    char wr_buffer[64] = {0};
    uint reg_offset = 0, reg_size = 0;

    choose = getdec_answer("Which part you want to change:\n1.Name:\n2.DESR:\n"
                           "3.Number of Type-11 strings:\n4.Exit:\n",
                           BIOS_EEPROM_NAME, BIOS_EEPROM_NAME, BIOS_EEPROM_CLEAR0);

    memset(wr_buffer, 0, sizeof(wr_buffer));
    switch(choose) {
         case BIOS_EEPROM_NAME:
             printf("\nInput NAME data (Format as: Chassis)\n");
             reg_offset = NAME_OFFSET;
             reg_size = EEPROM_NAME_LEN;
             /* Clear this field to NULL value first. */
             clear_field(reg_offset, reg_size);
             /* Get input string */
             memset(wr_buffer, 0, sizeof(wr_buffer));
             get_input_string(wr_buffer, reg_size);
             /* Write string into EEPROM field, last character is 'Enter' from user
              * input string, so string length needs to minus 1. */
             write_eeprom_block(reg_offset, wr_buffer, strlen(wr_buffer) - 1);
             break;
         case BIOS_EEPROM_DESR:
             printf("\nInput DESR data (Format as: Enterprise Network Compute System)\n");
             reg_offset = DESR_OFFSET;
             reg_size = EEPROM_DESR_LEN;
             /* Clear this field to NULL value first. */
             clear_field(reg_offset, reg_size);
             /* Get input string */
             memset(wr_buffer, 0, sizeof(wr_buffer));
             get_input_string(wr_buffer, reg_size);
             /* Write string into EEPROM field, last character is 'Enter' from user
              * input string, so string length needs to minus 1. */
             write_eeprom_block(reg_offset, wr_buffer, strlen(wr_buffer) - 1);
             break;
         case BIOS_EEPROM_CLEAR0:
             /* Clear entire EEPROM to 0 value */  
             clear_entire_eeprom_to_zero();
             break;
         case BIOS_EEPROM_NUM_OF_STRING:
             printf("\nInput hex value for Number/Count of type-11 strings (Format as: a)\n");
             reg_offset = NUM_OF_STRING_OFFSET;
             reg_size = EEPROM_NUM_STRINGS;
             /* Clear this field to NULL value first. */
             clear_field(reg_offset, reg_size);
             /* Get input hex number */
             wr_buffer[0] = gethex_answer("Enter hex number: "
                                          , 0, 0, 0xff);
             printf("Enter %#x\n", wr_buffer[0]);
             /* Write string into EEPROM field */
             write_eeprom_hex(reg_offset, wr_buffer[0]);
             break;
         case BIOS_EEPROM_EXIT:
             printf("\nExit\n");
             break;
         default:
             printf("\nPlease try again\n");
             break;
    }
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

int clear_field (unsigned int offset, int field_len)
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

int write_eeprom_block (unsigned int offset,  char *w_data, int w_length)
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
    char r_buf[MAX_STRING_LEN] = {0};

    /* Get hex data by eeprog tool */
    ExecuteCmdbyPopen(CHECK_EEPROM_STATUS, r_buf, MAX_COMMAND_LENGTH);

    /* Check if EEPROM default value is EMPTY by eeprog tool, if it's empty,
     * then the content will be like below dumped by eeprog tool. And need to
     * load default value.
     *     tabei-diag# eeprog -qxf /dev/i2c-0 0x54 -16 -t 10 -r 0x0:0x30
     *     0000|  ff ff ff ff ff ff ff ff   ff ff ff ff ff ff ff ff
     *     0010|  ff ff ff ff ff ff ff ff   ff ff ff ff ff ff ff ff
     *     0020|  ff ff ff ff ff ff ff ff   ff ff ff ff ff ff ff ff
     */
    if ((r_buf[9] == HEX_VAL_OF_F_CHAR) && (r_buf[10] == HEX_VAL_OF_F_CHAR)
        && (r_buf[12] == HEX_VAL_OF_F_CHAR) && (r_buf[13] == HEX_VAL_OF_F_CHAR)) {
        
        /* Cisco BIOS request the EEPROM unused field needs to be zero. */
        clear_entire_eeprom_to_zero();

        /* This file is for eeprog writing with option -i */
        fp = fopen(INIT_FILE_NAME, "wb+");
        if (fp < 0) {
            fclose(fp);
            printf("Can not open init file");
            return (FAILED);
        }
        /* Init the DMI EEPROM */
        fwrite(default_eeprom_content, sizeof(char), TABEI_EEPROM_INIT_SIZE, fp);
        fclose(fp);
        printf("Wait a minute...\n");
        system(INIT_EEPROM);
        /* Wait for write cycle time (option -t) */
        usleep(WAIT_EEPROG);

        /* Remove the file */
        system(RM_INIT_FILE);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : sync_cookie_content_into_eeprom 
 * Description : Sync cookie fields: Part Number, CLEI Code, Processor Type, 
 *               MAC Address block size,  Manufacturing Test Data, 
 *               PCB Serial Number, Chassis MAC Address, VID eight fields from 
 *               cookie into BIOS EEPROM. 
 * Inputs      : None
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int sync_cookie_content_into_eeprom (void)
{
    FILE *fp;
    uint16_t id = 0;
    char err[80];
    char prodName[EEPROM_PRODUCT_NAME_LEN] = {0};
    char partNumber[EEPROM_PART_NUMBER_LEN] = {0};
    char chassisSN[EEPROM_CHASSISSN_LEN] = {0}; 
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
    memset(chassisSN, 0, sizeof(chassisSN));
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
        default_eeprom_content[ix + PID_BASE_OFFSET] = prodName[ix];
    }
    
    /* Get TAN Part Number */
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

    /* Get Chassis SN */
    get_chassis_serial(cookie, chassisSN);
    printf("Chassis Serial Num:         %s\n", chassisSN);
    for (ix = 0; ix < EEPROM_PCBSN_LEN; ix++) {
        default_eeprom_content[ix + SN_BASE_OFFSET] = chassisSN[ix];
        default_eeprom_content[ix + CHASSIS_SN_BASE_OFFSET] = chassisSN[ix];
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
    printf("Manufacturing Test Data:    %s", MFGTestData); 
    for (ix = 0; ix < MFG_TEST_DATA_LEN; ix++) {
        default_eeprom_content[ix + MFG_TEST_DATA_OFFSET] = MFGTestData[ix];
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
            default_eeprom_content[ix + MAC_ADDRESS_BASE_OFFSET] = CharToHex(temp_macADDR[jx] & 0xF);
            jx = jx + 1;
        } else {
            /* Assign ASCII 0x2e -> '.' into default buffer  */
            default_eeprom_content[ix + MAC_ADDRESS_BASE_OFFSET] = COMMON_ACSII_VAL;
        }
        printf("%x ", default_eeprom_content[ix + MAC_ADDRESS_BASE_OFFSET]);
    }  
    printf("\n");
 
    /* This file is for eeprog writing with option -i */
    fp = fopen(INIT_FILE_NAME, "wb+");
    if (fp < 0) {
        fclose(fp);
        printf("Can not open init file");
        return (FAILED);
    }
    fwrite(default_eeprom_content, sizeof(char), TABEI_INIT_EEPROM_SIZE, fp);
    fclose(fp);
    system(INIT_EEPROM);
    /* Wait for write cycle time (option -t) */
    usleep(WAIT_EEPROG);

    /* Remove the file */
    system(RM_INIT_FILE);

    return (PASSED);
}


/**************************************************************************
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
 *************************************************************************/
void init_cookie_default(int board_type, uint8_t * contents)
{
    uint sku_id_val;

    printf("%s: Tabei currently skip SKU ID check.\n", __FUNCTION__);
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

/**************************************************************************
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
 *************************************************************************/
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
    if ((slot == 0) || (board_type == WIC_MODULE) || 
        (board_type == PLUGGABLE_CARD || board_type == WIC_DAUGHTER_CARD)) {
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

    if ((board_type != MOTHER_BOARD) && (board_type != PLUGGABLE_CARD)) {
        tam_act2_reset(0);
        if (!cookie_is_act2(con)) {
            return smart_cookie_read_write_eeprom(con, NULL);
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

    ngio = (struct ngio_intf_t *)slot_get_ngiowic(slot);
    
    if (slot_i2c_unreset(ngio, slot, "WIC") < 0) {
        return (FAILED);
    }
    
    return (alter_ngio_cookie(NULL, ngio, WIC_MODULE));
}

/**************************************************************************
 *
 * Name: alter_nim_dc_cookie
 *
 * Description:  alter nim dc cookie via menu.
 *
 * Inputs: None
 *
 * Outputs: return value of alter_ngio_cookie()
 *
 *************************************************************************/
int alter_nim_dc_cookie (void)
{
    int slot;
    struct ngio_intf_t *ngio, *dc;

    slot = FIRST_SLOT;

    ngio = (struct ngio_intf_t *)slot_get_ngiowic(slot);

    assert(ngio);
    assert(ngio->dc);

    dc = ngio->dc;

    if (slot_i2c_unreset(ngio, slot, "WICDC") < 0) {
        return (FAILED);
    }
    return (alter_ngio_cookie(NULL, dc, WIC_DAUGHTER_CARD));
}


/**************************************************************************
 * Function: alter_pim_cookie
 *
 * Description:
 *   This function is the entry point for the alter plug ACT2
 *   cookie contents. It allow a user to edit/diaplay the contents of the
 *   cookie info within the ACT2 device.
 *
 * Parameters:
 *    none
 *
 * Returns:
 *   PASSED/FAILED
 *************************************************************************/
int alter_pim_cookie(void)
{
    int slot = FIRST_SLOT;
    struct plug_intf_t *plug;

    plug = (struct plug_intf_t *)slot_get_plugslot(slot);

    if (plug_slot_i2c_poweron_unreset(plug, slot, "PLUGGABLE_CARD") != PASSED) {
        printf("Unable to unreset PIM module slot %d\n", slot);
        return (FAILED);
    }
    
    if (alter_cookie(PLUGGABLE_CARD, slot) != PASSED ) {
        return (FAILED);
    }

    return (PASSED);

}

/**************************************************************************
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
 *************************************************************************/
int alter_mb_cookie(void)
{
    int  act2_chip;

#ifdef SUPPORT_DISCRETE_ACT2
    act2_chip = getdec_answer("\n(0-Discrete-ACT2, 1-Aikido-ACT2):", 0, 0, 1);
#else
    act2_chip = getdec_answer("\n(1-Aikido-ACT2):", 1, 1, 1);
#endif
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

    if (alter_cookie(MOTHER_BOARD, 0) != PASSED ) {
        return (FAILED);
    }

    if(has_bios_eeprom() == TRUE) {
        if (modified_cookie_flag == TRUE) {
            printf("\nCookie changed, sync DMI EEPROM content from "
                    "corresponding cookie content.\n");
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
    }

    return (PASSED);
}

/**************************************************************************
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
 *************************************************************************/
int smartchip (int submenu_flag)
{
    char choice[5], type;
    sc_context *con, cont;
    int  act2_chip, slot, max_slot;
    struct ngio_intf_t *ngio;
    char *tname = "Smart Cookie";
    struct plug_intf_t *plugslot;

    con = &cont;
    con->slot = 0;
    con->dev_if_p = &dev_if;
    con->dev_if_p->cookie_size = COOKIE_SIZE_512;

    testname(tname);

    if (submenu_flag == 0) {
        prpass(testpass, "Main Menu");
    } else {
        prpass(testpass, "Sub Menu");
    }

    printf("\nEnter (m)otherboard, (n)im, (p)lug, (Q)UIT  >");
    get_line(choice, sizeof(choice) + 1);

    switch (choice[0]) {
    case 'm':
        printf("\n\nSelect Act2 Chip:");
#ifdef SUPPORT_DISCRETE_ACT2
        act2_chip = getdec_answer("\n(0-Discrete-ACT2, 1-Aikido-ACT2):", 0, 0, 1);
#else
        act2_chip = getdec_answer("\n(1-Aikido-ACT2):", 1, 1, 1);
#endif
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
        act2_init_cont((void *) con);
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

        if (plat_init_smart_eeprom_context(con, type, slot, cookie_contents) == FAILED) {
            return FAILED;
        }
        act2_init_cont((void*)con);  /* must be called after plat_init */

        return (act2_prog(0));
        break;

    case 'p':
        max_slot = get_max_pim_slots();
        slot = getdec_answer("Enter PLUG slot number", 1, FIRST_SLOT,
                max_slot);
  
        plugslot = (struct plug_intf_t *)slot_get_plugslot(slot);

        if (plug_slot_i2c_poweron_unreset(plugslot, slot, "PLUGGABLE_CARD") 
           != PASSED) {
            printf("Unable to unreset PIM module slot %d\n", slot);
            return (FAILED);
        }

        if (plat_init_smart_eeprom_context (con, PLUGGABLE_CARD,
                                            slot, (uchar *)cookie_contents)) {
            return (FAILED);
        }

        act2_init_cont(con);
        printf("finish act2_init_cont\n");//test

        /*
         * ACT2 library
         */
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
    struct plug_intf_t *plugslot;

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
    case PLUGGABLE_CARD:
        plugslot = (struct plug_intf_t *)slot_get_plugslot(slot);
        if (plug_slot_i2c_unreset(plugslot) == FAILED) {
            printf("%s: PLUG I2C Unreset failed\n", __func__);
            return (FAILED);
        }
        con_p->type = type;
        con_p->slot = slot;        /* slot = 0 */
        con_p->cookie_contents = cookie_p;
        con_p->quack_read_2bytes = (PFT)i2c_act2_read_bytes;
        con_p->quack_write_2bytes = (PFT)i2c_act2_write_bytes;
        con_p->quack_reset = (PFT)i2c_act2_reset;
        con_p->dev_if_p->parm1 = (uint8_t)PLUG_FPGA;
        con_p->dev_if_p->parm2 = (uint8_t)PLUG_I2C_ADDR_ACT2;
        con_p->dev_if_p->parm3 = (uint8_t)0;
        con_p->dev_if_p->parm4 = 20; /* I2C Controller */
        con_p->dev_if_p->interface = SCC_I2C_IF;
        sprintf((char *)smc_buf, "PLUG");

        /* important, using legacy way */
        aikido_act2_flag = FALSE;
        aikido_mailbox_flag = FALSE;

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

    pdata = search_type_ret_addr_of_first_data(cookie_contents, PART_NUMBER_TAN,
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

    /* MFG changes the bom part number format from 2-6-2 to 3-5-2 due to 
       74 start part number is belong to Foxconn and 800 start part number is belong
       to Cisco. */
    sprintf(pnu,"%03d-%05d-%02d", (partNUMBER[0] & 0xFF)*256 + (partNUMBER[1] & 0xFF),
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
 * Function get_chassis_serial
 *
 * This function will return the chassis serial number.
 *
 * Inputs: eeprom_data - dummy parameters
 *         serial - control id
 *
 * Returns : PASSED/FAILED
 */
int get_chassis_serial(uchar * eeprom_data, char *serial)
{
    int ix;
    uint8_t *pdata = NULL;
    uint8_t ret_num_of_bytes = 0;
    char chassisSN[CHASSIS_SERIAL_LEN] = { 0 };

    if (platform_tam_handle == NULL) {
        /* cookie type is MOTHERBOARD */
        if (g_cookie_type == MOTHER_BOARD) {
            if (tam_lib_read_cookie() == FAILED) {
                return (FAILED);
            }
        } 
    }

    pdata =
        search_type_ret_addr_of_first_data(cookie_contents, CHASSIS_SERIAL_NUM,
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
            chassisSN[ix] = *(pdata + ix);
        }
    }

    sprintf(serial, "%s", chassisSN);

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
            mac_addr[ix] = *(pdata + ix);
        }
    }

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
    unsigned char mfg_test_d[MFG_TEST_DATA_LEN] = { 0 };

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
            mfg_test_d[ix] = (*(pdata + ix) & 0xFF);
        }
    }

    /* There should be a space when convert the hex value into the string. */
    sprintf(mfg_test_data, "%02X %02X %02X %02X %02X %02X %02X %02X", 
            (mfg_test_d[0] & 0xFF), (mfg_test_d[1] & 0xFF), (mfg_test_d[2] & 0xFF), 
            (mfg_test_d[3] & 0xFF), (mfg_test_d[4] & 0xFF), (mfg_test_d[5] & 0xFF), 
            (mfg_test_d[6] & 0xFF), (mfg_test_d[7] & 0xFF));

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

/*******************************************************************************
 *
 * Function    : clear_entire_eeprom_to_zero
 * Description : Clear entire BIOS EEPROM value to 0 per Cisco BIOS requirement
 * Inputs      : None 
 * Outputs     : None
 *
 *******************************************************************************
 */
void clear_entire_eeprom_to_zero (void)
{
    printf("\nClear EEPROM to 0. Wait a minute...\n");
    /* Create 8K bytes 0x00 bin file. */
    system(CREATE_ERASE_FILE);
    usleep(WAIT_EEPROG);
    /* Erase EEPROM content */
    system(ERASE_EEPROM_CMD);
    usleep(WAIT_EEPROG);
    /* Remove bin file */
    system(RM_ERASE_FILE);
    printf("Clear EEPROM done.\n");
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

/*-------------------------------------------------
 * $Log: platform_cookie.c,v $
 * Revision 1.7  2020/12/17 07:19:05  kodko
 * Change part number field format from 2-6-2 to 3-5-2.
 *
 * Revision 1.6  2020/08/21 03:10:15  kehuang2
 * CSCvv45718: Solve unable to get cookie with specific combiantion of module and test order
 *
 * Revision 1.5  2020/05/05 05:58:52  kodko
 * [CSCvu09466]: Tabei-L EEPROM format needs to add 3 leading zero-bytes so that BIOS can parse the strings quickly, hence it will reduce the BIOS boot time.
 *
 * Revision 1.4  2019/12/30 06:03:45  kehuang2
 * CSCvs55860: Support Alter Quack cookie
 *
 * Revision 1.3  2019/11/25 08:55:52  kehuang2
 * Collapse Tabei-L into main trunk
 *
 * Revision 1.2  2019/10/17 02:16:26  kehuang2
 * Collapse Tabei-L into main trunk
 *
 * Revision 1.1.4.50  2019/08/06 07:20:28  kehuang2
 * Update present function base on the comment of code review
 *
 * Revision 1.1.4.49  2019/08/01 03:38:01  kodko
 * Clean up Alter PIM cookie and BIOS EEPROM utility codes.
 *
 * Revision 1.1.4.48  2019/07/31 03:45:08  kehuang2
 * Merge platform_tam_cookie into platform_cookie
 *
 * Revision 1.1.4.47  2019/07/30 08:44:31  kodko
 * Clean up code based on code review
 *
 * Revision 1.1.4.46  2019/07/30 07:19:55  olin2
 * Code clean up
 *
 * Revision 1.1.4.45  2019/07/30 02:44:12  kehuang2
 * Remove Discrete ACT2 utility
 *
 * Revision 1.1.4.44  2019/07/24 09:30:08  kodko
 * Clean up BIOS EEPROM utility codes.
 *
 * Revision 1.1.4.43  2019/07/17 09:53:44  olin2
 * Fix cannot write specific MAC address to BIOS EEPROM bug
 *
 * Revision 1.1.4.42  2019/07/17 08:13:04  kehuang2
 * Remove sync with BIOS EEPROM when using Promethium
 *
 * Revision 1.1.4.41  2019/07/16 09:12:35  sherliu2
 * Support program PIM ACT2 util
 *
 * Revision 1.1.4.40  2019/07/16 07:27:11  olin2
 * Support program NIM ACT2 util
 *
 * Revision 1.1.4.39  2019/07/11 05:45:04  olin2
 * Correct read PIM cookie procedure
 *
 * Revision 1.1.4.38  2019/06/06 08:32:47  kodko
 * Show BIOS EEPROM readable string by dmidecode command instead of eeprog tool.
 *
 * Revision 1.1.4.37  2019/04/25 03:17:46  kodko
 * Do not sync BIOS EEPROM if the cookie content is not changed.
 *
 * Revision 1.1.4.36  2019/04/10 03:34:17  kodko
 * Support altering NIM module cookie content.
 *
 * Revision 1.1.4.35  2019/04/08 00:41:24  kodko
 * Add space between Manufacturing Test Data string field while alter the content.
 *
 * Revision 1.1.4.34  2019/04/01 06:49:30  kodko
 * Fix alter empty PIM module cookie crash issue.
 *
 * Revision 1.1.4.33  2019/03/27 08:47:59  kodko
 * Modify the BIOS EEPROM utility base on V0.8 format and only can sync the BIOS EEPROM content from cookie utility.
 *
 * Revision 1.1.4.32  2019/03/07 06:38:09  olin2
 * Support Arkentone on Tabei-L
 *
 * Revision 1.1.4.31  2019/02/23 08:06:41  kodko
 * Initialize the type-11 header while manually alter the type-11 string content.
 *
 * Revision 1.1.4.30  2019/02/15 00:35:20  kodko
 * Support enhance features to erase EEPROM with 0x0 and able to alter each individual string.
 *
 * Revision 1.1.4.29  2019/01/21 09:43:28  kodko
 * Clear PCB Serial Number to NULL characters when altering this string.
 *
 * Revision 1.1.4.28  2019/01/19 08:58:00  kodko
 * Fix code to support altering PIM cookie
 *
 * Revision 1.1.4.27  2019/01/19 03:49:03  kodko
 * Support alter PIM cookie.
 *
 * Revision 1.1.4.26  2019/01/18 06:26:36  kodko
 * Fill the rest of a field automatically with null characters when the user writes a string.
 *
 * Revision 1.1.4.25  2019/01/15 02:03:02  kodko
 * Support to manually alter each string field content.
 *
 * Revision 1.1.4.24  2019/01/11 12:04:36  kodko
 * Aligh with Cisco BIOS EEPROM request.
 *
 * Revision 1.1.4.23  2019/01/09 12:54:40  kodko
 * Modify type-11 string location to meet Cisco's requirement.
 *
 * Revision 1.1.4.22  2018/12/20 07:57:40  kodko
 * Shit the BIOS EEPROM string offset for 2 bytes after Processor Type.
 *
 * Revision 1.1.4.21  2018/12/20 02:55:59  kodko
 * Extend BIOS EEPROM Processor type length from 4 bytes to 6 bytes
 *
 * Revision 1.1.4.20  2018/12/07 08:19:59  kodko
 * Support to write BIOS EEPROM String 1~8 fields from fetching cookie content.
 *
 * Revision 1.1.4.19  2018/12/05 06:50:35  olin2
 * initial commit for Aikido
 *
 * Revision 1.1.4.18  2018/12/03 07:53:30  kodko
 * Update BIOS cookie format to version 4.
 *
 * Revision 1.1.4.17  2018/11/16 05:42:12  olin2
 * Clean up code
 *
 * Revision 1.1.4.16  2018/11/14 10:09:20  kodko
 * Re-organize the BIOS EEPROM format.
 *
 * Revision 1.1.4.15  2018/11/13 09:00:53  kodko
 * Support to program BIOS eeprom from getting information of cookie PID/SN/MAC ADDRESS content.
 *
 * Revision 1.1.4.14  2018/11/06 08:00:19  kodko
 * Move get_cookie_id() con structure init to modules only.
 *
 * Revision 1.1.4.13  2018/11/06 06:55:17  kodko
 * Init cookie type before reading the cookie content.
 *
 * Revision 1.1.4.12  2018/11/06 05:26:42  kodko
 * Move con structure init to the beginning of get_cookie_id() function.
 *
 * Revision 1.1.4.11  2018/11/05 12:17:31  kodko
 * Support 64Kbits BIOS EEPROM program.
 *
 * Revision 1.1.4.10  2018/11/02 10:44:33  harrchan
 * EEPROM
 *
 * Revision 1.1.4.8  2018/11/02 07:42:40  harrchan
 * EEPROM init utility
 *
 * Revision 1.1.4.7  2018/11/02 02:39:03  kodko
 * Support cookie read for NIM and PIM modules.
 *
 * Revision 1.1.4.6  2018/11/01 01:07:40  harrchan
 * EEPROM read/write utility
 *
 * Revision 1.1.4.5  2018/10/17 06:14:28  olin2
 * Support FPGA I2C scan
 *
 * Revision 1.1.4.4  2018/10/16 11:33:14  olin2
 * Update NIM test
 *
 * Revision 1.1.4.3  2018/10/15 11:48:29  olin2
 * Update for using common slot.c
 *
 * Revision 1.1.4.2  2018/10/02 01:50:03  harrchan
 * Initial commit for Tabei-L P1A bring up.
 *
 * $Endlog$
 *-------------------------------------------------
 */
