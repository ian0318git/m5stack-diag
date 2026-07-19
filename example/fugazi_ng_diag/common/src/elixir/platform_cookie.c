/* $Id: platform_cookie.c,v 1.2 2021/09/24 01:21:07 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/elixir/platform_cookie.c,v $
 *------------------------------------------------------------------
 * 
 * platform_cookie.c
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
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
#include "diag_i2c_lib.h"
#include "tam_act2_api_drv_support.h"
#include "tam_aikido_mailbox.h"
#include "platform_stub.h"
#include "platform_i2c.h"
#include "/auto/sp-engops/diags/pld/act2lite/ARM/TSN/tam_library.h"
#include "/auto/sp-engops/diags/pld/act2lite/ARM/TSN/tam_lib_manufacturing.h"
#include "diag_moka_fpga_lib.h"
#include "queryflags.h"
#include "i2c_api.h"
#include "n2g_api_rc.h"
#include "nvmonvars.h"
#include <assert.h>
#include "goofy_i2c.h"
#include "diag_enhance_err_msg_lib.h"
#include "diag_poe_psu_util.h"
#include "diag_poe_psu_lib.h"
#include "diag_wifi_lib.h"
#include "plug_slot.h"               //../plug_common
#include "diag_sirius_fpga_lib.h"
#include "plug_host_fpga_lib.h"      //for pluggable code
#include "plug_common_host_impl.h"
#include "diag_aikido_fpga_lib.h"


/***********************************************************************
 *  Macro Processor Definitions
 ************************************************************************/

/***********************************************************************
 *  Functions Declaration
 ************************************************************************/
static int reserved_pass_function(void);
int alter_mb_cookie(void);
int alter_wifi_cookie(void);
int alter_poe_cookie(void);
int alter_plug_cookie(void);
int smartchip(int);
int tam_lib_read_cookie(void);
ushort get_cookie_id(int, int, uchar *, uint16_t *, char *);
int get_pid(uchar *, char *);
int get_pcb_serial(uchar *, char *);
int print_cookie(int, char *argv[]);
int platform_get_pid(char *);
static int alter_cookie(int board_type);

void i2c_act2_reset(sc_context *);
int i2c_act2_write_bytes(sc_context *, char *, int);
int i2c_act2_read_bytes(sc_context *, char *);

/***********************************************************************
 *  Extern Functions Declaration
 ************************************************************************/
extern void msleep(unsigned long);
extern int cookie_4_processor (uchar *, int, int, cli_cookie_cmd *);
extern int cookie_4_processor_x(uchar *, int, int, int, cli_cookie_cmd *);
extern COOKIE_4 *buffer_search_x(boolean, int);
extern COOKIE_4 *get_new_buf(void);
extern void fetch_user_input_data(char *, COOKIE_4 *);
extern void cookie_4_enque(COOKIE_4 *, COOKIE_4 *);
extern void movbyte(unsigned char *, unsigned char *, int);
extern tam_lib_status_t
tam_lib_scc_write_eeprom(void *tam_handle,
                         uint8_t * src_buffer,
                         uint16_t length, uint16_t dest);
extern boolean aikido_act2_flag;
extern boolean pcb_for_sudi;
boolean aikido_mailbox_flag;
boolean aikido_act2_flag;
extern int act2_ic_debug;

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
static int g_plug_slot = 0; 
static uint8_t use_interrupt = 0;

static int is_set_already = NOT_SET;
static dev_if_info_t dev_if;
static char i2c_err[80];
static char smc_buf[80];
static char g_i2c_adapter0[] = I2CBUS0;
static char g_i2c_adapter2[] = I2CBUS2;

static int current_platform_cid;
static struct pid_list *current_platform_pid;
static struct pid_list invalid_pid = {(uchar*)"INVALID-PID", FALSE, FALSE};
static struct pid_list elixir_pid_list[PLATFORM_PID_LIST_LENGTH] = {
    /*
     * Elixir PID list
     * Format: (PID and each functional flag)
     *       PID,            (1)Pluggable (2)Sirius FPGA
     */
    {(uchar*)"C1131X-8PLTEPWA",TRUE,         TRUE},
    {(uchar*)"C1131X-8PLTEPWB",TRUE,         TRUE},
    {(uchar*)"C1131X-8PLTEPWE",TRUE,         TRUE},
    {(uchar*)"C1131X-8PLTEPWQ",TRUE,         TRUE},
    {(uchar*)"C1131X-8PLTEPWZ",TRUE,         TRUE},
    {(uchar*)"C1131X-8PWA"    ,FALSE,        FALSE},
    {(uchar*)"C1131X-8PWB"    ,FALSE,        FALSE},
    {(uchar*)"C1131X-8PWE"    ,FALSE,        FALSE},
    {(uchar*)"C1131X-8PWQ"    ,FALSE,        FALSE},
    {(uchar*)"C1131X-8PWZ"    ,FALSE,        FALSE},
    {(uchar*)"C1131-8PLTEPWA" ,TRUE,         TRUE},
    {(uchar*)"C1131-8PLTEPWB" ,TRUE,         TRUE},
    {(uchar*)"C1131-8PLTEPWE" ,TRUE,         TRUE},
    {(uchar*)"C1131-8PLTEPWQ" ,TRUE,         TRUE},
    {(uchar*)"C1131-8PLTEPWZ" ,TRUE,         TRUE},
    {(uchar*)"C1131-8PWA"     ,FALSE,        FALSE},
    {(uchar*)"C1131-8PWB"     ,FALSE,        FALSE},
    {(uchar*)"C1131-8PWE"     ,FALSE,        FALSE},
    {(uchar*)"C1131-8PWQ"     ,FALSE,        FALSE},
    {(uchar*)"C1131-8PWZ"     ,FALSE,        FALSE},

};

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

static uchar default_967_K9_cookie[COOKIE_SIZE_512] = {
    0x04, 0xff, 0xc3, 0x06, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xc1, 0x8b, 0x30, 0x30, 0x30, 0x30,
    0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x40,
    0x0c, 0xb2, 0x41, 0x01, 0x00, 0x09, 0xdd, 0x82,
    0x4a, 0x27, 0xb4, 0x00, 0x42, 0x30, 0x33, 0x88,
    0x00, 0x00, 0x00, 0x00, 0x02, 0x01, 0x81, 0x00,
    0x00, 0x00, 0x00, 0x04, 0x00, 0x43, 0x00, 0x0e,
    0xc2, 0x8b, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
    0x30, 0x30, 0x30, 0x30, 0x30, 0x89, 0x56, 0x30,
    0x30, 0x00, 0x4a, 0xff, 0xff, 0xc0, 0x46, 0x03,
    0x20, 0x01, 0x8e, 0xbf, 0x01, 0xc6, 0x8a, 0x54,
    0x42, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xcc, 0x83, 0x4e, 0x2f, 0x41, 0xcb, 0x89,
    0x49, 0x53, 0x52, 0x39, 0x36, 0x37, 0x2f, 0x4b,
    0x39, 0xc9, 0x03, 0x37, 0x81, 0x04, 0xff, 0xff,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
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

static uchar default_967B_K9_cookie[COOKIE_SIZE_512] = {
    0x04, 0xff, 0xc3, 0x06, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xc1, 0x8b, 0x30, 0x30, 0x30, 0x30,
    0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x40,
    0x0c, 0xbd, 0x41, 0x01, 0x00, 0x09, 0xdd, 0x82,
    0x4a, 0x28, 0x19, 0x00, 0x42, 0x30, 0x33, 0x88,
    0x00, 0x00, 0x00, 0x00, 0x02, 0x01, 0x81, 0x00,
    0x00, 0x00, 0x00, 0x04, 0x00, 0x43, 0x00, 0x0e,
    0xc2, 0x8b, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
    0x30, 0x30, 0x30, 0x30, 0x30, 0x89, 0x56, 0x30,
    0x30, 0x00, 0x4a, 0xff, 0xff, 0xc0, 0x46, 0x03,
    0x20, 0x01, 0x8f, 0xc9, 0x01, 0xc6, 0x8a, 0x54,
    0x42, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xcc, 0x83, 0x4e, 0x2f, 0x41, 0xcb, 0x8a,
    0x49, 0x53, 0x52, 0x39, 0x36, 0x37, 0x42, 0x2f,
    0x4b, 0xc9, 0x03, 0x37, 0x81, 0x04, 0xff, 0xff,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
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

static uchar default_967_LTEA_cookie[COOKIE_SIZE_512] = {
    0x04, 0xff, 0xc3, 0x06, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xc1, 0x8b, 0x30, 0x30, 0x30, 0x30,
    0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x40,
    0x0c, 0xb4, 0x41, 0x01, 0x00, 0x09, 0xdd, 0x82,
    0x4a, 0x27, 0xb4, 0x00, 0x42, 0x30, 0x33, 0x88,
    0x00, 0x00, 0x00, 0x00, 0x02, 0x01, 0x81, 0x00,
    0x00, 0x00, 0x00, 0x04, 0x00, 0x43, 0x00, 0x0e,
    0xc2, 0x8b, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
    0x30, 0x30, 0x30, 0x30, 0x30, 0x89, 0x56, 0x30,
    0x30, 0x00, 0x4a, 0xff, 0xff, 0xc0, 0x46, 0x03,
    0x20, 0x01, 0x8e, 0xc0, 0x01, 0xc6, 0x8a, 0x54,
    0x42, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c,
    0x00, 0xcc, 0x83, 0x4e, 0x2f, 0x41, 0xcb, 0x91,
    0x49, 0x53, 0x52, 0x39, 0x36, 0x37, 0x2d, 0x4c,
    0x54, 0x45, 0x41, 0x2d, 0x45, 0x41, 0x2f, 0x4b,
    0x39, 0xc9, 0x03, 0x37, 0x81, 0x04, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
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
    struct plug_intf_t *plugslot;

    *i2c_err = '\0';
    
    con_p->info_string = smc_buf;
    
    switch (type) {
    case MOTHER_BOARD: /* I2C interface */
        con_p->type = type;
        con_p->slot = slot;        /* slot = 0 */
        con_p->cookie_contents = cookie_p;
        con_p->quack_read_2bytes = (PFT) i2c_quack_read_bytes;
        con_p->quack_write_2bytes = (PFT) i2c_quack_write_bytes;
        con_p->quack_reset = (PFT) i2c_quack_reset;
        con_p->dev_if_p->parm1 = (uint8_t) CPU_I2C0;
        if (aikido_act2_flag) {
            con_p->dev_if_p->parm2 = (uint8_t) MB_I2C_ADDR_AIKIDO_ACT2;
        } else {
            con_p->dev_if_p->parm2 = (uint8_t) MB_I2C_ADDR_ACT2;
        }
        con_p->dev_if_p->parm3 = (uint8_t) MB_I2C_MUX_ACT2;
        con_p->dev_if_p->parm4 = (uint8_t) MB_I2C_CTRL_ACT2;
        con_p->dev_if_p->cookie_size = COOKIE_SIZE_512;
        con_p->dev_if_p->interface = SCC_I2C_IF;
        sprintf((char *)smc_buf, "MB");
        break;
    case DAUGHTER_CARD:
        con_p->type = type;
        con_p->slot = slot ;
        con_p->cookie_contents = cookie_p;
        con_p->quack_read_2bytes = (PFT) i2c_quack_read_bytes;
        con_p->quack_write_2bytes = (PFT) i2c_quack_write_bytes;
        con_p->quack_reset = (PFT) i2c_quack_reset;
        con_p->dev_if_p->parm1 = (uint8_t) CPU_I2C2,
        con_p->dev_if_p->parm2 = (uint8_t) WIFI_I2C_ADDR_ACT2;
        con_p->dev_if_p->parm3 = (uint8_t) WIFI_I2C_MUX_ACT2;
        con_p->dev_if_p->parm4 = (uint8_t) WIFI_I2C_CTRL_ACT2;
        con_p->dev_if_p->cookie_size = COOKIE_SIZE_512;
        con_p->dev_if_p->interface = SCC_I2C_IF;
        sprintf((char *)smc_buf, "WIFI");
        break;
    case PLUGGABLE_CARD:
        plugslot = (struct plug_intf_t *)slot_get_plugslot(slot);
        if (plug_slot_i2c_poweron_unreset(plugslot, slot, "PLUG") == FAILED) {
            printf("%s: PLUG I2C Unreset failed\n", __func__);
            return (FAILED);
        }
        con_p->type = type;
        con_p->slot = slot;        /* slot = 0 */
        con_p->cookie_contents = cookie_p;
        con_p->quack_read_2bytes = (PFT)i2c_act2_read_bytes;
        con_p->quack_write_2bytes = (PFT)i2c_act2_write_bytes;
        con_p->quack_reset = (PFT)plug_i2c_act2_reset;
        con_p->dev_if_p->parm1 = (uint8_t)PLUG_FPGA; 
        con_p->dev_if_p->parm2 = (uint8_t)PLUG_I2C_ADDR_ACT2;   
        con_p->dev_if_p->parm3 = (uint8_t)0;
        con_p->dev_if_p->parm4 = slot - 1; /* I2C Controller */
        con_p->dev_if_p->interface = SCC_I2C_IF;
        sprintf((char *)smc_buf, "PLUG");
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
 *           intializes with plug slot sc_context.
 * Input:  con_p   - pointer to sc_context
 *         slot    - slot
 *         type    - type
 * Output: none
 *
 **********************************************************************
*/
void platform_init_smart_context (sc_context *con_p, int slot, int type) 
{
    con_p->dev_if_p = &dev_if;
    con_p->dev_if_p->cookie_size = COOKIE_SIZE_512;

    plat_init_smart_eeprom_context(con_p, type, (uchar)slot, cookie_contents);

    act2_init_cont(con_p);
    con_p->quack_reset(con_p);
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
    unsigned int reg, reset, slot; 
    if (con_p->type == PLUGGABLE_CARD)  {
        slot = con_p->slot;
        switch (slot) {
        case 1: 
            reg = FPGA_PLUG1_STSCTL_REG; 
            reset = PLUG_I2C_RESET; 
        break; 
        case 2: 
            reg = FPGA_PLUG2_STSCTL_REG; 
            reset = PLUG_I2C_RESET; 
        break; 
        }    
        printf("Resetting ACT2 PLUG%d...", slot);
        fflush(stdout);
        plug_fpga_reg_or(reg, reset); 
        msleep(ACT2_RESET_UNRESET_DELAY);
        plug_fpga_reg_nand(reg, reset); 
        /* ACT2 unreset delay implement in tam_lib_platform_read */
        msleep(ACT2_UNRESET_DELAY);
        printf("Done\n");
        fflush(stdout);
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
    uint sku_id_val;

    printf("%s: FIX ME !! Platform currently skip SKU ID check.\n", __FUNCTION__);
    sku_id_val = DSL_SKU_ISR967_K9;

    switch (board_type) {
    case MOTHER_BOARD:
        printf("\nLoading default cookie format for Motherboard.\n");
        switch (sku_id_val) {
        case DSL_SKU_ISR967_K9:
            movbyte(default_967_K9_cookie, contents, cookie_size);
            break;
        case DSL_SKU_ISR967B_K9:
            movbyte(default_967B_K9_cookie, contents, cookie_size);
            break;
        case DSL_SKU_ISR967_LTE_EA_K9:
            movbyte(default_967_LTEA_cookie, contents, cookie_size);
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

    printf("%s: FIX ME !! Platform currently skip SKU ID check.\n", __FUNCTION__);
    sku_id_val = DSL_SKU_ISR967_K9;

    printf("\n Loading default cookie contents ...\n");
    switch (sku_id_val) {
    case DSL_SKU_ISR967_K9:
        movbyte(default_967_K9_cookie, contents, EEPROM_RD_WR_LENGTH);
        break;
    case DSL_SKU_ISR967B_K9:
        movbyte(default_967B_K9_cookie, contents, EEPROM_RD_WR_LENGTH);
        break;
    case DSL_SKU_ISR967_LTE_EA_K9:
        movbyte(default_967_LTEA_cookie, contents, EEPROM_RD_WR_LENGTH);
        break;
    default:
    movbyte(default_mb_cookie, contents, EEPROM_RD_WR_LENGTH);
        break;
    }
}

/*
 * Function: platform_has_pluggable
 *
 * Description:
 *   This function is used to distinguish the platform whether it has pluggable
 *
 * Returns:
 *   TRUE/FALSE
 */
int platform_has_pluggable (void) {
    return (current_platform_pid->pluggable_flag);
}

/*
 * Function: platform_has_sirius_fpga
 *
 * Description:
 *   This function is used to distinguish the platform whether it has sirius FPGA
 *
 * Returns:
 *   TRUE/FALSE
 */
int platform_has_sirius_fpga (void) {
    return (current_platform_pid->sirius_fpga_flag);
}

/*
 * Function: get_current_pid
 *
 * Description:
 *   This function is used to return current pid of Betelgeuse.
 *
 * Returns:
 *   a string of current pid
 */
uchar *get_current_pid (void) {
     return (current_platform_pid->pid);    
}

/*
 * Function: initial_current_product_id
 *
 * Description:
 *   This function is used to read PID from ccookies then check whether it is involved in elixir_pid_list.
 *
 * Returns:
 *   PASSED/FAILED
 */
int initial_current_product_id (void) {
    uchar mb_pid[FRU_SIZE] = {0};
    int ix;

    /* Betelgeuse only has Aikido ACT2 */
    aikido_act2_flag = TRUE;
    aikido_mailbox_flag = FALSE;

    if (is_set_already == NOT_SET) {   
        is_set_already = SET_ALREADY;
        current_platform_pid = &invalid_pid;
        platform_get_pid((char *)mb_pid);
        for (ix = 0; ix < PLATFORM_PID_LIST_LENGTH; ix++) {
            if (strcmp((char*)mb_pid, (char*)elixir_pid_list[ix].pid) == PASSED) {
                current_platform_pid = &elixir_pid_list[ix];
                break;
            }
        }

        if (strcmp((char*)current_platform_pid->pid,(char*)invalid_pid.pid) == PASSED) {
            printf("\nThe PID is invalid (%s)\n", (char *)mb_pid);
            return (FAILED);
        } else {
            current_platform_cid = get_mb_id(); 
            printf("\nThe PID is valid (%s);  CID is 0x%x\n", (char *)current_platform_pid->pid, current_platform_cid);
            return (PASSED);
        }
    } else {
        if ((NVRAM)->diagflag & D_VERBOSE) { 
            printf("current_platform_pid PID:%s", current_platform_pid->pid);
        }
        return (PASSED);
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
static int alter_cookie(int board_type)
{
    void *tam_handle_ptr;
    void *platform_opaque_handle = NULL;
    uint16_t platform_buffer_size = PLATFORM_BUFF_SIZE;
    uint8_t cookie_contents_buf[COOKIE_SIZE_512];
    tam_lib_status_t status;
    int ret_val;
    sc_context *con, cont;
    con = &cont;

    tam_handle_ptr = NULL;
    if (aikido_mailbox_flag) {
        /* Initialize Mailbox */
        status = tam_lib_device_open_mailbox((void *)con, use_interrupt,
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
        status =
            tam_lib_scc_write_eeprom(tam_handle_ptr,
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
 * Name: ilp_eeprom_read
 *
 * Description: This is a wrapper for the ILP i2c EEPROM read routine
 *
 * Inputs: offset - i2c device offset
 *         dest - points to where the read data to be stored
 *         byte_count - number of data bytes to read 
 * 
 * Output: PASSED/FAILED
 *
 *************************************************************************/
int ilp_eeprom_read (uint offset, uchar *dest, int byte_count)
{
    n2g_i2c_if_t i2c_if;
    int ix;
    uint32_t rc;
    char data;

    /* Fill the interface struct passed to I2C APIs */
    i2c_if.i2c_bus_type     = CPU_I2C2;
    i2c_if.i2c_dev          = MB_I2C_ADDR_POE_EEPROM;
    i2c_if.size             = byte_count;
    i2c_if.buf              = (char *)&data;
    i2c_if.offset           = offset;

    /* Lock the controller for the device */
    for (ix = 0; ix < N2G_I2C_OPEN_TIMEOUT; ix++) {
        if ((rc = n2g_i2c_open(&i2c_if)) == E_I2C_LOCKED) {
            msleep(N2G_I2C_BIT_DELAY);
        } else {
            break;
        }
    }

    if (rc != PASSED) {
        printf("%s:%d:Unable to open I2C device\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    /* Perform I2C Read */
    rc = n2g_i2c_read(&i2c_if);
    if (rc != PASSED) {
        printf("%s:%d:Unable to read I2C, statux = %x\n",
                __FUNCTION__, __LINE__, rc);
        return (FAILED);
    }

    *dest = (uchar)data;

    return (rc);
}

/**************************************************************************
 *
 * Name: ilp_eeprom_write
 *
 * Description: This is a wrapper for the ILP i2c EEPROM write routine
 *
 * Inputs: offset - i2c device offset
 *         dest - points to where the write data to be stored
 *         byte_count - number of data bytes to read
 *
 * Output: PASSED/FAILED
 *
 *************************************************************************/
int ilp_eeprom_write (uchar offset, uchar data, int byte_count)
{
    n2g_i2c_if_t i2c_if;
    int ix;
    uint32_t rc;

    /* Fill the interface struct passed to I2C APIs */
    i2c_if.i2c_bus_type     = CPU_I2C2;
    i2c_if.i2c_dev          = MB_I2C_ADDR_POE_EEPROM;
    i2c_if.size             = byte_count;
    i2c_if.buf              = (char *)&data;
    i2c_if.offset           = offset;

    /* Lock the controller for the device */
    for (ix = 0; ix < N2G_I2C_OPEN_TIMEOUT; ix++) {
        if ((rc = n2g_i2c_open(&i2c_if)) == E_I2C_LOCKED) {
            msleep(N2G_I2C_BIT_DELAY);
        } else {
            break;
        }
    }

    if (rc != PASSED) {
        printf("%s:%d:Unable to open I2C device\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    /* Perform I2C Write */
    rc = n2g_i2c_write(&i2c_if);
    if (rc != PASSED) {
        printf("%s:%d:Unable to write I2C, statux = %x\n",
                __FUNCTION__, __LINE__, rc);
        return (FAILED);
    }

    msleep(10);     /* 5ms write cycle time at least */

    return (rc);
}

/**************************************************************************
 *
 * Name: read_poe_dc_cookie
 *
 * Description: Read the Inception PoE daughtercard cookie from the
 *              standard I2C interface
 *
 * Inputs: poe_dc_cookie - pointer to the buffer to hold the cookie
 *
 * Outputs: None
 *
 *************************************************************************/
static void read_poe_dc_cookie (uchar *poe_dc_cookie)
{
    int addr_offset;
    uchar return_data;

    for (addr_offset = 0; addr_offset < COOKIE_SIZE_128; addr_offset++) {
        if (ilp_eeprom_read(addr_offset, &return_data, 1) == FAILED) {
            return;
        }

        poe_dc_cookie[addr_offset] = return_data;

        if (diagflag_xram & D_TRACE) {
            if (!(addr_offset % 8)) {
                printf("\npoe_dc_cookie[0x%02x]: 0x%02x",
                       addr_offset, poe_dc_cookie[addr_offset]);
            } else {
                printf(" 0x%02x", poe_dc_cookie[addr_offset]);
            }
        }
    }
    return;
}

/**************************************************************************
 *
 * Name: write_poe_dc_cookie
 *
 * Description: Write the PoE daughtercard cookie to the
 *              standard I2C interface
 *
 * Inputs: poe_dc_cookie - pointer to the buffer to hold the cookie
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
static void write_poe_dc_cookie (uchar *poe_dc_cookie)
{
    int addr_offset;

    for (addr_offset = 0; addr_offset < COOKIE_SIZE_128; addr_offset++) {
        if (ilp_eeprom_write(addr_offset, poe_dc_cookie[addr_offset], 1)) {
            return;
        }
    }

    return;

}


/*
 * Function: alter_ilp_eeprom
 *
 * Description:
 *   This function is the alter POE eeprom
 *   cookie contents. It allow a user to edit/diaplay the contents of the
 *   cookie info within the eeprom device.
 *
 * Parameters:
 *    none
 *
 * Returns:
 *   PASSED/FAILED
 */
static int alter_ilp_eeprom (void)
{
    uchar cookie[COOKIE_SIZE_128];
    uchar *poe_cookie = &cookie[0];
    int modified = 1;

    read_poe_dc_cookie(poe_cookie);

    if (poe_cookie[0] != CURRENT_FORMAT_VERSION) {
        printf("\n!!!Invalid eeprom format or missing card!!!\n");
    }

    if (cookie_4_processor_x((uchar *)poe_cookie,0, DAUGHTER_CARD, COOKIE_SIZE_128 ,NULL) == modified) {
        write_poe_dc_cookie(poe_cookie);
    }
    return (PASSED);
}

/*
 * Function: alter_poe_cookie
 *
 * Description:
 *   This function is the entry point for the alter POE eeprom
 *   cookie contents. It allow a user to edit/diaplay the contents of the
 *   cookie info within the eeprom device.
 *
 * Parameters:
 *    none
 *
 * Returns:
 *   PASSED/FAILED
 */
int alter_poe_cookie(void)
{
    if (platform_has_poe(0) == FALSE) {
        printf("POE DC not present\n");
        return (PASSED); 
    }
    
    if (alter_ilp_eeprom() != PASSED ) {
        return (FAILED);
    }

    return (PASSED);
}

/**************************************************************************
 * Function: alter_plug_cookie
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
int alter_plug_cookie(void)
{
    int max_slot, slot;
    struct plug_intf_t *plug;
    sc_context *con, cont;
    dev_if_info_t dev_ifl;
    uchar cookie_contents[COOKIE_SIZE_512];

    max_slot = plug_common_host_get_max_plug_slots();
    slot = getdec_answer("Enter PLUG slot number", 1, FIRST_SLOT,
            max_slot);
  
    plug = (struct plug_intf_t *)slot_get_plugslot(slot);

    /* to check whether pluggable module is present */
    if (diag_plug_module_is_present() != PASSED) {
        return (FAILED);
    }

    if (plug_slot_i2c_poweron_unreset(plug, slot, "PLUGGABLE_CARD") < 0) {
        return (FAILED);
    }
        
    con = &cont;
    con->dev_if_p = &dev_ifl;
    con->dev_if_p->cookie_size = COOKIE_SIZE_512;

    if (plat_init_smart_eeprom_context (con, PLUGGABLE_CARD, 
                                        slot, (uchar *)cookie_contents)) {
        return (FAILED);
    }

    act2_init_cont(con);
    
    if (alter_cookie(PLUGGABLE_CARD) != PASSED ) {
        return (FAILED);
    }
    
    return (PASSED);

}
/*
 * Function: alter_wifi_cookie
 *
 * Description:
 *   This function is the entry point for the alter wifi ACT2
 *   cookie contents. It allow a user to edit/diaplay the contents of the
 *   cookie info within the ACT2 device.
 *
 * Parameters:
 *    none
 *
 * Returns:
 *   PASSED/FAILED
 */
int alter_wifi_cookie(void)
{
    char i2c_adapter[] = I2CBUS2;
    sc_context *con, cont; 

    con = &cont;
    /* Confirm WiFi module is out of RESET */
    if (release_wifi_from_reset() != PASSED) {
        printf("%s(%d): Failed to release WiFi module from RESET.\n",
               __func__, __LINE__);
        return (FAILED);
    }

    /* Wifi card not support ACT2 AIKIDO or TAM Mailbox */
    aikido_act2_flag = FALSE;
    aikido_mailbox_flag = FALSE;

    if (diagact2_lib_initialize(i2c_adapter, WIFI_I2C_ADDR_ACT2) != PASSED) {
        return (FAILED);
    }
    platform_init_smart_context(con,0,DAUGHTER_CARD);

    if (alter_cookie(DAUGHTER_CARD) != PASSED ) {
        return (FAILED);
    }
    /* Restore the flag */
    aikido_act2_flag = TRUE;
    aikido_mailbox_flag = FALSE;

    return (PASSED);
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
    char i2c_adapter[] = I2CBUS0;
    int  act2_chip;
    sc_context *con, cont;

    con = &cont;
    printf("\n\nSelect Act2 Chip:");
    aikido_mailbox_flag = FALSE;
    act2_chip = getdec_answer("\n(1-Aikido-ACT2):", 1, 0, 1);
    if (act2_chip == 1) {
        aikido_act2_flag = TRUE;
        aikido_mailbox_flag = FALSE;
        printf("\nSelect AIKIDO ACT2\n");
    } else {
        printf("\nUnsupported option!!\n");
    }    
    if (aikido_mailbox_flag == FALSE ){
        if (diagact2_lib_initialize(i2c_adapter, MB_I2C_ADDR_ACT2) != PASSED) {
           return (FAILED);
        }
    }	
    platform_init_smart_context(con, 0, MOTHER_BOARD);

    if (alter_cookie(MOTHER_BOARD) != PASSED ) {
        return (FAILED);
    }
    diagact2_close_i2c_adapter();

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
    char choice[5];
    int rc = PASSED;
    sc_context *con, cont;
    dev_if_info_t dev_ifs;
    int  act2_chip;
    char *tname = "Smart Cookie";
    int slot, max_slot;
    struct plug_intf_t *plug;

    testname(tname);

    if (submenu_flag == 0) {
        prpass(testpass, "Main Menu");
    } else {
        prpass(testpass, "Sub Menu");
    }
    if (platform_has_pluggable() == TRUE) {
        printf("\nEnter (m)otherboard, (p)lug, (Q)UIT  >");
    } else {
        printf("\nEnter (m)otherboard, (Q)UIT  >");
    } 

    get_line(choice, sizeof(choice) + 1);

    switch (choice[0]) {
    case 'm':
        printf("\n\nSelect Act2 Chip:");
        aikido_mailbox_flag = FALSE;
        act2_chip = getdec_answer("\n(1-Aikido-I2C-ACT2 , 2-Aikido-SPI-ACT2):", 
                                  1, 0, 2);
        if (act2_chip == 1) {
            aikido_act2_flag = TRUE;
            aikido_mailbox_flag = FALSE;
            printf("\nSelect AIKIDO ACT2\n");
        } else if (act2_chip == 0) {
            aikido_act2_flag = FALSE;
            aikido_mailbox_flag = FALSE;
            printf("\nSelect Discrete ACT2\n");
        }  else if (act2_chip == 2) {
            aikido_act2_flag = TRUE;
            aikido_mailbox_flag = TRUE;
            printf("\nSelect Aikido Mailbox\n");
        }
        /*
         * Dummy initialized for error message "cont.dev_if_p that
         * is not initialized."
         */
            con = &cont;
         con->dev_if_p = &dev_ifs;

         plat_init_smart_eeprom_context(con, MOTHER_BOARD, 0, cookie_contents);

            act2_init_cont((void *) con);

        /*
         * ACT2 library
         */
        return (act2_prog(0));

        break;

    case 'w':
        /* Confirm WiFi module is out of RESET */
        if (release_wifi_from_reset() != PASSED) {
            printf("%s(%d): Failed to release WiFi module from RESET.\n",
                   __func__, __LINE__);
            return (FAILED);
        }

        /* Wifi not support AIKIDO / TAM Library */
        aikido_act2_flag = FALSE;
        aikido_mailbox_flag = FALSE;
        /*
         * Dummy initialized for error message "cont.dev_if_p that
         * is not initialized."
         */
        con = &cont;
        con->dev_if_p = &dev_ifs;

        plat_init_smart_eeprom_context(con, DAUGHTER_CARD, 0, cookie_contents);

        act2_init_cont((void *) con);
        aikido_act2_flag = FALSE;

        /*
         * ACT2 library
         */
        return (act2_prog(0));
        break;

    case 'p':
        max_slot = plug_common_host_get_max_plug_slots();
        slot = getdec_answer("Enter PLUG slot number", 1, FIRST_SLOT,
                max_slot);
  
        if (platform_has_pluggable() != TRUE) {
            printf("\n This SKU doesn't support pluggable \n");
            return (PASSED);
        }

        /* to check whether pluggable module is present */
        if (diag_plug_module_is_present() != PASSED) {
            return (FAILED);
        }

        plug = (struct plug_intf_t *)slot_get_plugslot(slot);

        if (plug_slot_i2c_poweron_unreset(plug, slot, "PLUGGABLE_CARD") < 0) {
            return (FAILED);
        }

        aikido_act2_flag = FALSE;
        aikido_mailbox_flag = FALSE;
            
        con = &cont;
        con->dev_if_p = &dev_ifs;
        con->dev_if_p->cookie_size = COOKIE_SIZE_512;

        if (plat_init_smart_eeprom_context (con, PLUGGABLE_CARD, 
                                            slot, (uchar *)cookie_contents)) {
            return (FAILED);
        }

        act2_init_cont(con);

        /*
         * ACT2 library
         */
        return (act2_prog(0));
        break;

    default:
        printf("Invalid input %s\n", choice);
        break;
    }

    return (rc);
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
                                      platform_buffer_size,
                                      &platform_tam_handle);
        if (ret_val != TAM_RC_OK) {
            printf("\n TAM lib: Cannot open handler: status = 0x%x",
                   ret_val);
            printf("\n tan_handle = %p ", platform_tam_handle);
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

/*************************************************************************
 Function: tam_lib_wifi_read_cookie
 *
 * This function is read the wifi cookie content by tam library
 *
 * Input: none
 *
 * Output: PASSED / FAILED
 **************************************************************************
*/
int tam_lib_wifi_read_cookie(void)
{
    void *platform_opaque_handle = NULL;
    uint16_t platform_buffer_size = PLATFORM_BUFF_SIZE;
    int ret_val;
    char i2c_adapter[] = I2CBUS2;  /* wifi act2 i2c bus 2 */
    
    ret_val = diagact2_lib_initialize(i2c_adapter, WIFI_I2C_ADDR_ACT2);

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

    ret_val =
        tam_lib_scc_read_eeprom(platform_tam_handle,
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

    /* cookie type is MOTHERBOARD */
    g_cookie_type = type;
    g_plug_slot = slot;
    if (g_cookie_type == MOTHER_BOARD) {
        if (!aikido_mailbox_flag) {
            if (diagact2_lib_initialize(g_i2c_adapter0, MB_I2C_ADDR_ACT2) != PASSED) {
        return (FAILED);
    }
        }
    } else if (g_cookie_type == DAUGHTER_CARD) { 
        /* Wifi card not support ACT2 AIKIDO or TAM Mailbox */
        aikido_mailbox_flag = FALSE;
        if (diagact2_lib_initialize(g_i2c_adapter2, WIFI_I2C_ADDR_ACT2) != PASSED) {
            return (FAILED);
        }
    } else if (g_cookie_type == PLUGGABLE_CARD) { 
        /* PLUG should be link by plat_init_smart_eeprom_context */
        aikido_mailbox_flag = FALSE;
        con = &cont;
        platform_init_smart_context(con,slot,g_cookie_type);
        act2_init_cont(con);
    } else {
            return (FAILED);
    }


    if (tam_lib_read_cookie() == FAILED) {
        printf("\ntam_lib_read_cook fail.\n");
        return (FAILED);
    }

    memcpy(eeprom_data, cookie_contents, COOKIE_SIZE_512);

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
    sc_context *con, cont;

    con = &cont;

    if (platform_tam_handle == NULL) {
        /* cookie type is MOTHERBOARD */
        if (g_cookie_type == MOTHER_BOARD) {
            if (!aikido_mailbox_flag) {
                if (diagact2_lib_initialize(g_i2c_adapter0, MB_I2C_ADDR_ACT2) != PASSED) {
            return (FAILED);
        }
            }
            platform_init_smart_context(con,0,g_cookie_type);
        } else if (g_cookie_type == DAUGHTER_CARD) { 
            /* Wifi card not support ACT2 AIKIDO or TAM Mailbox */
            aikido_mailbox_flag = FALSE;
            if (diagact2_lib_initialize(g_i2c_adapter2, WIFI_I2C_ADDR_ACT2) != PASSED) {
            return (FAILED);
        }
            platform_init_smart_context(con,0,g_cookie_type);
        } else if (g_cookie_type == PLUGGABLE_CARD) { 
            /* PLUG should be link by plat_init_smart_eeprom_context */
            aikido_mailbox_flag = FALSE;
            platform_init_smart_context(con,g_plug_slot,g_cookie_type);
        } else {
                return (FAILED);
            }

        if (tam_lib_read_cookie() == FAILED) {
            return (FAILED);
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
    sc_context *con, cont;

    con = &cont;

    if (platform_tam_handle == NULL) {
        /* cookie type is MOTHERBOARD */
        if (g_cookie_type == MOTHER_BOARD) {
            if (!aikido_mailbox_flag) {
                if (diagact2_lib_initialize(g_i2c_adapter0, MB_I2C_ADDR_ACT2) != PASSED) {
            return (FAILED);
        }
            }
            platform_init_smart_context(con,0,g_cookie_type);
        } else if (g_cookie_type == DAUGHTER_CARD) { 
            /* Wifi card not support ACT2 AIKIDO or TAM Mailbox */
            aikido_mailbox_flag = FALSE;
            if (diagact2_lib_initialize(g_i2c_adapter2, WIFI_I2C_ADDR_ACT2) != PASSED) {
            return (FAILED);
        }
            platform_init_smart_context(con,0,g_cookie_type);
        } else if (g_cookie_type == PLUGGABLE_CARD) { 
            /* PLUG should be link by plat_init_smart_eeprom_context */
            aikido_mailbox_flag = FALSE;
            platform_init_smart_context(con,g_plug_slot,g_cookie_type);
        } else {
                return (FAILED);
            }

        if (tam_lib_read_cookie() == FAILED) {
           return (FAILED);
        }

        memcpy(eeprom_data, cookie_contents, COOKIE_SIZE_512);
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
    sc_context *con, cont;

    con = &cont;
    memset(prodName, 0, sizeof(prodName));
    memset(vidName, 0, sizeof(vidName));
    memset(prodSN, 0, sizeof(prodSN));

    if (platform_tam_handle == NULL) {
        /* cookie type is MOTHERBOARD */
        if (g_cookie_type == MOTHER_BOARD) {
            if (!aikido_mailbox_flag) {
                if (diagact2_lib_initialize(g_i2c_adapter0, MB_I2C_ADDR_ACT2) != PASSED) {
            return (FAILED);
        }
            }
            platform_init_smart_context(con,0,g_cookie_type);
        } else if (g_cookie_type == DAUGHTER_CARD) { 
            /* Wifi card not support ACT2 AIKIDO or TAM Mailbox */
            aikido_mailbox_flag = FALSE;
            if (diagact2_lib_initialize(g_i2c_adapter2, WIFI_I2C_ADDR_ACT2) != PASSED) {
            return (FAILED);
        }
            platform_init_smart_context(con,0,g_cookie_type);
        } else if (g_cookie_type == PLUGGABLE_CARD) { 
            /* PLUG should be link by plat_init_smart_eeprom_context */
            aikido_mailbox_flag = FALSE;
            platform_init_smart_context(con,g_plug_slot,g_cookie_type);
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
    sc_context *con, cont;
    uint16_t id = 0;
    uchar cookie[COOKIE_SIZE_512];

    /* Only the first time needs to go to get the PID. */
    if (get_sku_flag == FALSE) {
        con = &cont;
        con->type = MOTHER_BOARD;
        con->slot = 0;
        con->dev_if_p = &dev_if;
        con->dev_if_p->parm1 = (uint8_t) CPU_I2C0;
        if (aikido_act2_flag){
            con->dev_if_p->parm2 = (uint8_t) MB_I2C_ADDR_AIKIDO_ACT2;
        } else {
        con->dev_if_p->parm2 = (uint8_t) MB_I2C_ADDR_ACT2;
        }
        con->dev_if_p->parm3 = (uint8_t) MB_I2C_MUX_ACT2;
        con->dev_if_p->parm4 = (uint8_t) MB_I2C_CTRL_ACT2;
        con->dev_if_p->cookie_size = COOKIE_SIZE_512;
        con->quack_read_2bytes = (PFT) reserved_pass_function;
        con->quack_write_2bytes = (PFT) reserved_pass_function;
        con->quack_reset = (PFT) reserved_pass_function;
        con->dev_if_p->interface = SCC_I2C_IF;

        act2_init_cont((void *) con);

        if (get_cookie_id(cont.slot, 0, cookie, &id, err)==FAILED) {
            printf("%s:%d:Unable to read cookie id\n", __FUNCTION__, __LINE__);
            return (FAILED);
        }
        get_sku_flag = TRUE;
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
    sc_context *con, cont;

    con = &cont;

    if (platform_tam_handle == NULL) {
        /* cookie type is MOTHERBOARD */
        if (g_cookie_type == MOTHER_BOARD) {
            if (!aikido_mailbox_flag) {
                if (diagact2_lib_initialize(g_i2c_adapter0, MB_I2C_ADDR_ACT2) != PASSED) {
            return (FAILED);
        }
            }
            platform_init_smart_context(con,0,g_cookie_type);
        } else if (g_cookie_type == DAUGHTER_CARD) { 
            /* Wifi card not support ACT2 AIKIDO or TAM Mailbox */
            aikido_mailbox_flag = FALSE;
            if (diagact2_lib_initialize(g_i2c_adapter2, WIFI_I2C_ADDR_ACT2) != PASSED) {
                return (FAILED);
            }
            platform_init_smart_context(con,0,g_cookie_type);
        } else if (g_cookie_type == PLUGGABLE_CARD) { 
            /* PLUG should be link by plat_init_smart_eeprom_context */
            aikido_mailbox_flag = FALSE;
            platform_init_smart_context(con,g_plug_slot,g_cookie_type);
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
        printf("%s:%d:Unsupported old cookie format 0x%02x\n", 
               __FUNCTION__, __LINE__, eeprom_data[0]);
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
ushort
get_mb_id(void)
{
    int cookie_size = COOKIE_SIZE_512;
    uchar cookie_contents_buf[cookie_size];
    ushort cntl_type = INVALID_ID;
    sc_context *con, cont;
    dev_if_info_t dev_if;
    char err[80];
    ushort id = 0;

#ifdef AIKIDO_ACT2
    aikido_act2_flag = TRUE;
    aikido_mailbox_flag = FALSE;
    printf("\nSelect AIKIDO ACT2\n");
#else
    aikido_act2_flag = FALSE;
    aikido_mailbox_flag = FALSE;
    printf("\nSelect Discrete ACT2\n"); 
#endif
    /*
     * Dummy initialized for error message "cont.dev_if_p that
     * is not initialized."
     */
    con = &cont;
    con->type = MOTHER_BOARD;
    con->slot = 0;
    con->dev_if_p = &dev_if;
    con->dev_if_p->parm1 = (uint8_t) CPU_I2C0;
    if (aikido_act2_flag) {
        con->dev_if_p->parm2 = (uint8_t) MB_I2C_ADDR_AIKIDO_ACT2;
    } else {
    con->dev_if_p->parm2 = (uint8_t) MB_I2C_ADDR_ACT2;
    }
    con->dev_if_p->parm3 = (uint8_t) MB_I2C_MUX_ACT2;
    con->dev_if_p->parm4 = (uint8_t) MB_I2C_CTRL_ACT2;
    con->dev_if_p->cookie_size = COOKIE_SIZE_512;
    con->quack_read_2bytes = (PFT) reserved_pass_function;
    con->quack_write_2bytes = (PFT) reserved_pass_function;
    con->quack_reset = (PFT) reserved_pass_function;
    con->dev_if_p->interface = SCC_I2C_IF;

    act2_init_cont((void *) con);

    if (get_cookie_id(cont.slot, cont.type, cookie_contents_buf, &id, err)==FAILED) {
        printf("%s:%d:Unable to read cookie id\n", __FUNCTION__, __LINE__);
        return (cntl_type);
    }

    printf("\nControl Type            : [0x%x]\n", id);
    return (id);
}

static int reserved_pass_function(void)
{
    return (PASSED);
}

/*-------------------------------------------------
 * $Log: platform_cookie.c,v $
 * Revision 1.2  2021/09/24 01:21:07  harrchan
 * Collapse Elixir-branch to Main Trunk.
 *
 * Revision 1.1.2.9  2021/07/30 06:22:00  harrchan
 * Closing i2c adapter after leaving the cookie menu
 *
 * Revision 1.1.2.8  2021/07/01 02:39:27  harrchan
 * Add Aikido-SPI-ACT2 option into menu so that users can choose SPI interface to programming ACT2.
 *
 * Revision 1.1.2.7  2021/05/26 04:02:32  harrchan
 * Fixed tam_lib_device_open_mailbox() input NULL pointer issue.
 *
 * Revision 1.1.2.6  2021/03/15 07:14:36  illiu
 * Add PID list for P2A
 *
 * Revision 1.1.2.5  2020/11/06 06:25:30  harrchan
 * Modify PID base on P1B PCAMAP
 *
 * Revision 1.1.2.4  2020/10/26 07:08:28  harrchan
 * 1.Changed PID table in platform_i2c.c
 * 2.Modify menu item to match up Elixir hardware design.
 *
 * Revision 1.1.2.3  2020/09/15 07:51:34  harrchan
 * Check in Betelgeuse PID struct for temporarily use
 *
 * Revision 1.1.2.2  2020/09/14 05:49:45  harrchan
 * Remove DSL and GSHDSL relevant part
 *
 * Revision 1.1.2.1  2020/09/09 09:08:08  illiu
 * First version which has been ported with Dreamliner and Marvel CPSS
 *
 * Revision 1.2  2019/01/10 06:36:25  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
