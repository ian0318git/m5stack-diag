/* $Id: platform_cookie.c,v 1.3 2021/04/15 01:55:08 peteteng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/katar/x86/platform_cookie.c,v $
 *------------------------------------------------------------------
 *
 * katar_platform_cookie.c - Specific MB cooke support from Xformers.
 *
 * Sept. 2008, Shih-Nan Huang
 *
 * Copyright (c) 2014-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include "platform_aikido.h"
#include "mb_tests.h" 


/***********************************************************************
 *  External Functions Declaration
 ************************************************************************/
extern int tam_act2_reset(int);
extern int cookie_4_processor_x(uchar *, int, int, int, cli_cookie_cmd *);
extern int act2_prog(int);
extern cookie_4_table cookie_4_info[];
extern COOKIE_4 *buffer_search_x(boolean, int); 
extern COOKIE_4 *get_new_buf(void);
extern int katar_get_plat_sku(void);
extern void msleep(int t); 
extern void tam_lib_platform_smbus(void *,boolean);
extern int file_exist(char *dest, size_t *);
extern int get_i2c_fd(int);
extern void fetch_user_input_data(char *, COOKIE_4 *);
extern void cookie_4_enque(COOKIE_4 *, COOKIE_4 *);
extern int is_fpga_i2c_scanned_aikido_addr(int);

/***********************************************************************
 *  Local Functions Declaration
 ************************************************************************/
int katar_alter_mb_cookie(void);
int katar_alter_mb_cookie_eeprom(void);
static int katar_alter_cookie(int board_type);
static int katar_alter_cookie_eeprom(int board_type);
void katar_platform_init_smart_context(sc_context *, int, int); 
void katar_platform_init_smart_context_eeprom(sc_context *, int, int); 
int katar_plat_init_smart_eeprom_context(sc_context *, uchar, uchar, uchar *);
int katar_plat_init_smart_eeprom_context_eeprom(sc_context *, uchar, uchar, uchar *);
int katar_i2c_quack_read_bytes(void);
int katar_i2c_quack_write_bytes(void);
int katar_i2c_quack_reset(void);
int katar_tam_lib_read_cookie(void);
int katar_load_cookie_eeprom(void);
int katar_read_cookie_eeprom(void);
int katar_read_cookie_eeprom_x(uchar *);
int katar_write_cookie_eeprom_from_file(void);
int write_cookie_eeprom_reg(int, uchar *, int);
int read_cookie_eeprom_reg(int, uchar *, int);

#define COOKIE_EEPROM_SIZE 256
#define COOKIE_EEPROM_ADDR 0x51

static int cookie_eeprom_i2c_fd = -1;

#define VID_LEN                     20 
#define MAX_VID_LEN             5

/***********************************************************************
 *  Global Variable
 ************************************************************************/
extern COOKIE_4 *cookie_root; 
static dev_if_info_t katar_dev_if;
static uchar katar_cookie_contents[COOKIE_SIZE_512];
static char katar_smc_buf[80];
static char katar_i2c_err[80];
static char katar_g_i2c_adapter0[] = I2CBUS0;
static void *katar_platform_tam_handle = NULL;
static int katar_g_cookie_type = 0; 

//static void *platform_tam_handle = NULL;  
static int g_cookie_type = 0; 
boolean aikido_mailbox_flag;
//static char g_i2c_adapter0[] = I2CBUS0;


/* Default cookie contents for RJ45 Sku */
static uchar default_rj45_cookie_512[COOKIE_SIZE_512] = {
    0x04,0xFF,0x40,0x10,0xB4,0x41,0x00,0x01,
    0xE2,0x46,0x00,0x4A,0x01,0xDA,0x8D,0x01,
    0x42,0x50,0x32,0x80,0x00,0x00,0x00,0x00,
    0x02,0x03,0xC1,0x8B,0x20,0x20,0x20,0x20,
    0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x03,
    0x00,0x81,0x00,0x00,0x00,0x00,0x04,0x00,
    0xC0,0x46,0x00,0x4A,0x01,0xDA,0x8D,0x01,
    0xC6,0x8A,0x55,0x4E,0x41,0x53,0x53,0x49,
    0x47,0x4E,0x45,0x44,0xCB,0x94,0x43,0x39,
    0x38,0x30,0x30,0x2D,0x4C,0x2D,0x43,0x2D,
    0x4B,0x39,0x20,0x20,0x20,0x20,0x20,0x20,
    0x20,0x20,0x89,0x50,0x50,0x20,0x20,0xC4,
    0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0xC5,0x08,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0xC3,0x06,0x0C,0x75,0xBD,
    0x00,0x00,0x00,0x43,0x00,0x20,0xC2,0x8B,
    0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
    0x20,0x20,0x20,0xF3,0x00,0x04,0x00,0x06,
    0x00,0xFA,0xC7,0x17,0x91,0x00,0x48,0x01,
    0x42,0x52,0x44,0x54,0x45,0x4D,0x50,0x31,
    0x02,0x00,0x3D,0x00,0x3C,0x00,0x39,0x00,
    0x36,0x00,0xFD,0xC7,0x17,0x91,0x00,0x4F,
    0x02,0x42,0x52,0x44,0x54,0x45,0x4D,0x50,
    0x32,0x02,0x00,0x41,0x00,0x40,0x00,0x3D,
    0x00,0x3A,0x00,0xE4,0xC7,0x17,0x80,0x00,
    0x00,0x03,0x43,0x50,0x55,0x20,0x44,0x69,
    0x65,0x20,0x02,0x00,0x69,0x00,0x68,0x00,
    0x63,0x00,0x5E,0x00,0xAF,0xCC,0xA0,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,
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

/* Default cookie contents for SFP Sku */
static uchar default_sfp_cookie_512[COOKIE_SIZE_512] = {
    0x04,0xFF,0x40,0x10,0xB4,0x41,0x00,0x01,
    0xE2,0x46,0x00,0x4A,0x01,0xDA,0x9B,0x01,
    0x42,0x50,0x32,0x80,0x00,0x00,0x00,0x00,
    0x02,0x03,0xC1,0x8B,0x20,0x20,0x20,0x20,
    0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x03,
    0x00,0x81,0x00,0x00,0x00,0x00,0x04,0x00,
    0xC0,0x46,0x00,0x4A,0x01,0xDA,0x9B,0x01,
    0xC6,0x8A,0x55,0x4E,0x41,0x53,0x53,0x49,
    0x47,0x4E,0x45,0x44,0xCB,0x94,0x43,0x39,
    0x38,0x30,0x30,0x2D,0x4C,0x2D,0x46,0x2D,
    0x4B,0x39,0x20,0x20,0x20,0x20,0x20,0x20,
    0x20,0x20,0x89,0x50,0x50,0x20,0x20,0xC4,
    0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0xC5,0x08,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0xC3,0x06,0x0C,0x75,0xBD,
    0x00,0x00,0x00,0x43,0x00,0x20,0xC2,0x8B,
    0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
    0x20,0x20,0x20,0xF3,0x00,0x04,0x00,0x06,
    0x00,0xFA,0xC7,0x17,0x91,0x00,0x48,0x01,
    0x42,0x52,0x44,0x54,0x45,0x4D,0x50,0x31,
    0x02,0x00,0x3D,0x00,0x3C,0x00,0x39,0x00,
    0x36,0x00,0xFD,0xC7,0x17,0x91,0x00,0x4F,
    0x02,0x42,0x52,0x44,0x54,0x45,0x4D,0x50,
    0x32,0x02,0x00,0x41,0x00,0x40,0x00,0x3D,
    0x00,0x3A,0x00,0xE4,0xC7,0x17,0x80,0x00,
    0x00,0x03,0x43,0x50,0x55,0x20,0x44,0x69,
    0x65,0x20,0x02,0x00,0x69,0x00,0x68,0x00,
    0x63,0x00,0x5E,0x00,0xAF,0xCC,0xA0,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,
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

/* Default cookie contents for RJ45 Sku - 256 bytes */
static uchar default_rj45_cookie_256[COOKIE_SIZE_256] = {
    0x04,0xFF,0x40,0x10,0xB4,0x41,0x00,0x01,
    0x82,0x00,0x00,0x00,0x00,0x42,0xFF,0xFF,
    0x80,0x00,0x00,0x00,0x00,0x02,0x03,0xC1,
    0x8B,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
    0x20,0x20,0x20,0x20,0x03,0x00,0x81,0x00,
    0x00,0x00,0x00,0x04,0x00,0x87,0x4A,0x00,
    0x00,0x01,0xC6,0x8A,0x58,0x58,0x58,0x58,
    0x58,0x58,0x58,0x58,0x58,0x58,0xCB,0x94,
    0x43,0x39,0x38,0x30,0x30,0x2D,0x31,0x30,
    0x2D,0x43,0x2D,0x4B,0x39,0x20,0x20,0x20,
    0x20,0x20,0x20,0x20,0x89,0x50,0x31,0x41,
    0x20,0xC4,0x08,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0xC5,0x08,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0xC3,0x06,0x0C,
    0x75,0xBD,0x00,0x00,0x00,0x43,0x00,0x20,
    0xC2,0x8B,0x20,0x20,0x20,0x20,0x20,0x20,
    0x20,0x20,0x20,0x20,0x20,0xF3,0x00,0x04,
    0x00,0x06,0x00,0xFA,0xC7,0x2E,0x03,0x0B,
    0x35,0x01,0x2D,0x4E,0x41,0x2D,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x1C,0xC7,0x2E,0x03,0x0B,
    0x36,0x02,0x2D,0x4E,0x41,0x2D,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x36,0xC7,0x2E,0x03,0x0B,
    0x37,0x03,0x2D,0x4E,0x41,0x2D,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};

/* Default cookie contents for SFP Sku - 256 bytes */
static uchar default_sfp_cookie_256[COOKIE_SIZE_256] = {
    0x04,0xFF,0x40,0x10,0xB5,0x41,0x00,0x01,
    0x82,0x00,0x00,0x00,0x00,0x42,0xFF,0xFF,
    0x80,0x00,0x00,0x00,0x00,0x02,0x03,0xC1,
    0x8B,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
    0x20,0x20,0x20,0x20,0x03,0x00,0x81,0x00,
    0x00,0x00,0x00,0x04,0x00,0x87,0x4A,0x00,
    0x00,0x01,0xC6,0x8A,0x58,0x58,0x58,0x58,
    0x58,0x58,0x58,0x58,0x58,0x58,0xCB,0x94,
    0x43,0x39,0x38,0x30,0x30,0x2D,0x31,0x30,
    0x2D,0x46,0x2D,0x4B,0x39,0x20,0x20,0x20,
    0x20,0x20,0x20,0x20,0x89,0x50,0x31,0x41,
    0x20,0xC4,0x08,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0xC5,0x08,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0xC3,0x06,0x0C,
    0x75,0xBD,0x00,0x00,0x00,0x43,0x00,0x20,
    0xC2,0x8B,0x20,0x20,0x20,0x20,0x20,0x20,
    0x20,0x20,0x20,0x20,0x20,0xF3,0x00,0x04,
    0x00,0x06,0x00,0xFA,0xC7,0x2E,0x03,0x0B,
    0x35,0x01,0x2D,0x4E,0x41,0x2D,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x1C,0xC7,0x2E,0x03,0x0B,
    0x36,0x02,0x2D,0x4E,0x41,0x2D,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x36,0xC7,0x2E,0x03,0x0B,
    0x37,0x03,0x2D,0x4E,0x41,0x2D,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};


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
    retval = katar_plat_init_smart_eeprom_context (con_p, type, slot, cookie_p);

    return (retval); 
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
init_cookie_4_default_x (int board_type, int cookie_type, 
                         uchar *contents, int cookie_size)
{
    switch (board_type) {
    case MOTHER_BOARD:
        printf("\nLoading default cookie format for RJ45 Sku.\n");
        movbyte(default_rj45_cookie_512, contents, cookie_size);
        break;
    case NETWK_MODULE:
        printf("\nLoading default cookie format for SFP Sku.\n");
        movbyte(default_sfp_cookie_512, contents, cookie_size);
        break;
    case WIC_MODULE:
        printf("\nLoading default cookie format for RJ45 Sku - 256 bytes.\n");
        movbyte(default_rj45_cookie_256, contents, cookie_size);
        break;
    case VIC_MODULE:
        printf("\nLoading default cookie format for SFP Sku - 256 bytes.\n");
        movbyte(default_sfp_cookie_256, contents, cookie_size);
        break;
    default:
        printf("\nLoading default cookie format for RJ45 Sku.\n");
        movbyte(default_rj45_cookie_512, contents, cookie_size);
        break;
    }
}


/*
 * Function: katar_alter_mb_cookie
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
int katar_alter_mb_cookie(void)
{

    switch(katar_get_plat_sku()) 
    {    
        case KATAR_RJ45_SKU:
            printf("\nRJ45 sku detected!\n");
            if (katar_alter_cookie(MOTHER_BOARD) != PASSED ) {
                return (FAILED);
            }
            break;
        case KATAR_SFP_SKU:
        case KATAR_SFP1_SKU:
            printf("\nSFP sku detected!\n");
            if (katar_alter_cookie(NETWK_MODULE) != PASSED ) {
                return (FAILED);
            }
            break;
        default:
            printf("\nUnknown sku!!!\n");
            break;
    } 
        
    return (PASSED);
}



/*
 * Function: katar_alter_mb_cookie_eeprom
 *
 * Description:
 *   This function is the entry point for the alter motherboard cookie
 *   contents. It allows a user to edit/diaplay the contents of the
 *   cookie info within the EEPROM.
 *
 * Parameters:
 *    none
 *
 * Returns:
 *   PASSED/FAILED
 */
int katar_alter_mb_cookie_eeprom(void)
{
    char i2c_adapter[] = I2CBUS0;
    sc_context *con, cont;

    con = &cont;
    if (katar_diagact2_lib_initialize(i2c_adapter, COOKIE_EEPROM_ADDR) != PASSED) {
        printf("\n *** ERROR: katar_diagact2_lib_initialize");
        return (FAILED);
    }

    if (system("i2cdetect -y 0 | grep 51 > /dev/null")) {
        printf("EERPROM not found!\n");
        return (FAILED);
    }

    switch(katar_get_plat_sku())
    {    
        case KATAR_RJ45_SKU:
            printf("\nRJ45 sku detected!\n");
            katar_platform_init_smart_context_eeprom(con, 0, WIC_MODULE);
            if (katar_alter_cookie_eeprom(WIC_MODULE) != PASSED ) {
                printf("\n *** ERROR: katar_alter_cookie_eeprom");
                return (FAILED);
            } 
            break;
        case KATAR_SFP_SKU:
        case KATAR_SFP1_SKU:
            printf("\nSFP sku detected!\n");
            katar_platform_init_smart_context_eeprom(con, 0, VIC_MODULE);
            if (katar_alter_cookie_eeprom(VIC_MODULE) != PASSED ) {
                printf("\n *** ERROR: katar_alter_cookie_eeprom");
                return (FAILED);
            }
            break;
        default:
            printf("\nUnknown sku!!!\n");
            break;
    }    
           
    return (PASSED);
}



/**********************************************************************
 *
 * Function: katar_platform_init_smart_context
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
void katar_platform_init_smart_context (sc_context *con_p, int slot, int type) 
{
    con_p->dev_if_p = &katar_dev_if;
    con_p->dev_if_p->cookie_size = COOKIE_SIZE_512;

    katar_plat_init_smart_eeprom_context(con_p, type, (uchar)slot, katar_cookie_contents);

    act2_init_cont(con_p);
    con_p->quack_reset(con_p);
}



/**********************************************************************
 *
 * Function: katar_platform_init_smart_context_eeprom
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
void katar_platform_init_smart_context_eeprom (sc_context *con_p, int slot, int type) 
{
    con_p->dev_if_p = &katar_dev_if;
    con_p->dev_if_p->cookie_size = COOKIE_SIZE_512;

    katar_plat_init_smart_eeprom_context_eeprom(con_p, type, (uchar)slot, katar_cookie_contents);

    act2_init_cont(con_p);
    con_p->quack_reset(con_p);
}




/**********************************************************************
 *
 * Function: katar_plat_init_smart_eeprom_context
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
int katar_plat_init_smart_eeprom_context (sc_context *con_p, uchar type,
                                    uchar slot, uchar *cookie_p)
{
    int retval = PASSED;
    *katar_i2c_err = '\0';
    
    con_p->info_string = katar_smc_buf;
    
    switch (type) {
    case MOTHER_BOARD: // RJ45 sku
    case NETWK_MODULE: // SFP sku
        con_p->type = type;
        con_p->slot = slot;        /* slot = 0 */
        con_p->cookie_contents = cookie_p;
        con_p->quack_read_2bytes = (PFT) katar_i2c_quack_read_bytes;
        con_p->quack_write_2bytes = (PFT) katar_i2c_quack_write_bytes;
        con_p->quack_reset = (PFT) katar_i2c_quack_reset;
        con_p->dev_if_p->parm1 = (uint8_t) IOFPGA_I2C;
        con_p->dev_if_p->parm2 = (uint8_t) MB_I2C_ADDR_AIKIDO_ACT2;
        con_p->dev_if_p->parm3 = (uint8_t) MB_I2C_MUX_ACT2;
        con_p->dev_if_p->parm4 = (uint8_t) MB_I2C_CTRL_ACT2;
        con_p->dev_if_p->cookie_size = COOKIE_SIZE_512;
        con_p->dev_if_p->interface = SCC_I2C_IF;
        sprintf((char *)katar_smc_buf, "MB");  // both RJ45 & SFP use the same context
        break;
    default:
        cterr('f',0,"in plat_init_smart_eeprom_context: Not a supported "
              "Smart EEPROM type %d", type);
        assert(!"plat_init_eeprom: invalid argument");
        retval = FAILED;
    }

    return (retval);
}



/**********************************************************************
 *
 * Function: katar_plat_init_smart_eeprom_context_eeprom
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
int katar_plat_init_smart_eeprom_context_eeprom (sc_context *con_p, uchar type,
                                    uchar slot, uchar *cookie_p)
{
    int retval = PASSED;
    *katar_i2c_err = '\0';
    
    con_p->info_string = katar_smc_buf;
    
    switch (type) {
    case WIC_MODULE: // RJ45 sku
    case VIC_MODULE: // SFP sku
        con_p->type = type;
        con_p->slot = slot;        /* slot = 0 */
        con_p->cookie_contents = cookie_p;
        con_p->quack_read_2bytes = (PFT) katar_i2c_quack_read_bytes;
        con_p->quack_write_2bytes = (PFT) katar_i2c_quack_write_bytes;
        con_p->quack_reset = (PFT) katar_i2c_quack_reset;
        con_p->dev_if_p->parm1 = (uint8_t) CPU_I2C0;
        con_p->dev_if_p->parm2 = (uint8_t) COOKIE_EEPROM_ADDR;
        con_p->dev_if_p->parm3 = (uint8_t) MB_I2C_MUX_ACT2;
        con_p->dev_if_p->parm4 = (uint8_t) MB_I2C_CTRL_ACT2;
        con_p->dev_if_p->cookie_size = COOKIE_SIZE_256;
        con_p->dev_if_p->interface = SCC_I2C_IF;
        sprintf((char *)katar_smc_buf, "MB");  // both RJ45 & SFP use the same context
        break;
    default:
        cterr('f',0,"in plat_init_smart_eeprom_context: Not a supported "
              "Smart EEPROM type %d", type);
        assert(!"plat_init_eeprom: invalid argument");
        retval = FAILED;
    }

    return (retval);
}



/*
 * Function: katar_alter_cookie
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
static int katar_alter_cookie(int board_type)
{
    int act2_chip; 
    boolean aikido_mailbox_flag;
    tam_lib_status_t status = TAM_RC_OK;
    void *platform_opaque_handle = NULL;
    uint8_t use_interrupt = 0;
    uint16_t mbx_msg_size = 0x700;
    uint32_t mbx_reg_base_addr = MBX_REG_BASE_ADDR;
    void *tam_handle = NULL;
    uint8_t cookie_contents_buf[COOKIE_SIZE_512];
    int ret_val;
    sc_context *con, cont;
    dev_if_info_t dev_if;

    if (diagflag_xram & D_DEBUG_OPTIONS) {
        tam_lib_platform_debug(platform_opaque_handle, TRUE);
    } else {
        tam_lib_platform_debug(platform_opaque_handle, FALSE);
    }

    printf("\nSelect Aikido Interface:");
    act2_chip = getdec_answer("\n(1-I2C ; 2-Device Bus):", 1, 1, 2);  
    if (act2_chip == 1) { 
        aikido_mailbox_flag = FALSE;
        printf("\nSelect AIKIDO I2C\n");
        if (system("i2cdetect -y 0 | grep 77 > /dev/null")) {
            if (is_fpga_i2c_scanned_aikido_addr(0) == 0) {
                printf("AIKIDO not found on SMBus or FPGA-I2C!\n");
                return (FAILED);
            }
            printf("Use FPGA-I2C interface\n");  // Use FPGA-I2C
            tam_lib_platform_smbus(platform_opaque_handle, FALSE);
            con = &cont;
            con->dev_if_p = &dev_if;
            con->dev_if_p->cookie_size = COOKIE_SIZE_512;
            if (katar_plat_init_smart_eeprom_context(con, board_type, 0, (uchar *)cookie_contents_buf) == FAILED) {
                printf("\n%s: Init Smart EEPROM context failed\n", __func__);
                return (FAILED);
            }
            act2_init_cont(con);
            con->quack_reset(con);
        }
        else {
            printf("Use SMBus interface\n");  // Use SMBus
            tam_lib_platform_smbus(platform_opaque_handle, TRUE);
        }
    } else {
        aikido_mailbox_flag = TRUE;
        printf("\nSelect AIKIDO Device Bus (Mailbox)\n");  // Use LPC
    }    

    if (aikido_mailbox_flag == TRUE) {  // device open - LPC
        /* Initialize Mailbox */
        status = tam_lib_device_open_mailbox((void *)con, 
                                             use_interrupt, mbx_msg_size, 
                                             mbx_reg_base_addr, &tam_handle);
        if (status != TAM_RC_OK) {
            printf("\n ERROR: Cannot Initialize Mailbox. Status %#x\n", status);
            return (FAILED);
        }    
    } else {  // device open - I2C
        int ret_val;                 
        /* I2C Platform Initialize */
        if (katar_diagact2_lib_initialize(I2CBUS0, MB_I2C_ADDR_AIKIDO_ACT2) != PASSED) {
            return (FAILED);
        }

        if (tam_handle == NULL) {
            ret_val = tam_lib_device_open(platform_opaque_handle,
                                          PLATFORM_BUFF_SIZE,
                                          &tam_handle);
            if (ret_val != TAM_RC_OK) {
                printf("\n%s: TAM lib: Cannot open handler: status = 0x%x", __func__, ret_val);
                printf("\n%s: tam_handle = %p ", __func__, tam_handle);
                return (FAILED);
            }    
        }    
    }    
    fflush(stdout);

    status = tam_lib_scc_read_eeprom(tam_handle,
                                (uint8_t *) cookie_contents_buf,
                                EEPROM_RD_WR_LENGTH, EEPROM_WRITE_ADDR);
    if (status != TAM_RC_OK) {
        printf("\n *** ERROR: tam_lib_scc_read_cookie. status = 0x%x",
               status);
        printf("\n Cannot read data from the EEPROM (Cookie). \n");
        return (FAILED);
    }

    /*
     * board_type is MOTHER_BOARD, cookie_type is slot 0, cmd is NULL
     */
    ret_val = cookie_4_processor_x(cookie_contents_buf, board_type, 0,
                                   EEPROM_RD_WR_LENGTH, NULL);

    if (ret_val) {
        if (aikido_mailbox_flag == TRUE) {
            status =
                tam_lib_scc_write_eeprom(tam_handle,
                                         (uint8_t *) cookie_contents_buf,
                                         EEPROM_RD_WR_LENGTH,
                                         EEPROM_WRITE_ADDR);
        }
        else {
            int i25 = 0;
            while (i25 <= EEPROM_RD_WR_LENGTH) {
                status =
                    tam_lib_scc_write_eeprom(tam_handle,
                                         &cookie_contents_buf[i25],
                                         25,
                                         EEPROM_WRITE_ADDR+i25);
                i25 += 25;
            }
            i25 -= 25;
            status =
                tam_lib_scc_write_eeprom(tam_handle,
                                     &cookie_contents_buf[i25],
                                     EEPROM_RD_WR_LENGTH-i25,
                                     EEPROM_WRITE_ADDR+i25);  // write the last few bytes
        }
        if (status != TAM_RC_OK) {
            printf("\n *** ERROR: tam_lib_scc_write_eeprom. status = 0x%x",
                   status);
            printf("\n Cannot write data to the EEPROM (Cookie). \n");
            return (FAILED);
        }
    }

    status = tam_lib_device_close(&tam_handle);
    if (status != TAM_RC_OK) {
        printf("\n TAM lib: Cannot close handler: status = 0x%x", status);
        return (FAILED);
    }

    return (status);
}



/*
 * Function: katar_alter_cookie_eeprom
 *
 * Description:
 *   This function is the entry point for the alter EEPROM
 *   cookie contents. It allow a user to edit/diaplay the contents of the
 *   cookie info within the EEPROM device.
 *
 * Parameters:
 *    none
 *
 * Returns:
 *   PASSED/FAILED
 */
static int katar_alter_cookie_eeprom(int board_type)
{
    uchar cookie_contents_buf[COOKIE_SIZE_256];  // katar_cookie_contents
    int ret_val;

    ret_val = katar_read_cookie_eeprom_x(cookie_contents_buf);
    if (ret_val == FAILED) {
        printf("\n *** ERROR: katar_read_cookie_eeprom");
        return (FAILED);
    }

    ret_val = cookie_4_processor_x(cookie_contents_buf, board_type, 0, COOKIE_EEPROM_SIZE, NULL);
    if (ret_val) {
        ret_val = write_cookie_eeprom_reg(0, (uchar *)cookie_contents_buf, COOKIE_EEPROM_SIZE);
        if (ret_val == FAILED) {
            printf("\n *** ERROR: write_cookie_eeprom_reg");
            return (FAILED);
        }
    }

    return (PASSED);
}

int read_eeprom_block (unsigned int offset,
                  unsigned int size, unsigned char *buf)
{
	return (PASSED);
}

int katar_i2c_quack_read_bytes(void)
{
    return (PASSED);
}


int katar_i2c_quack_write_bytes(void)
{
    return (PASSED);
}


int katar_i2c_quack_reset(void)
{
    return (PASSED);
}


/*------------------------------------------------------------------------
 * katar_smartchip
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
int katar_smartchip(int submenu_flag)
{
    char choice[5];
    int rc = PASSED;
    sc_context *con, cont;
    dev_if_info_t dev_ifs;
    char *tname = "Smart Cookie";
    int  act2_chip;
    void *platform_opaque_handle = NULL;

    printf("\nSelect Aikido Interface:");
    act2_chip = getdec_answer("\n(1-I2C ; 2-Device Bus):", 1, 1, 2);  
    if (act2_chip == 1) { 
        aikido_mailbox_flag = FALSE;
        printf("\nSelect AIKIDO I2C\n");
        if (system("i2cdetect -y 0 | grep 77 > /dev/null")) {
            if (is_fpga_i2c_scanned_aikido_addr(0) == 0) {
                printf("AIKIDO not found on SMBus or FPGA-I2C!\n");
                return (FAILED);
            }
            printf("Use FPGA-I2C interface\n");  // Use FPGA-I2C
            tam_lib_platform_smbus(platform_opaque_handle, FALSE);
        }
        else {
            printf("Use SMBus interface\n");  // Use SMBus
            tam_lib_platform_smbus(platform_opaque_handle, TRUE);
        }
    } else {
        aikido_mailbox_flag = TRUE;
        printf("\nSelect AIKIDO Device Bus (Mailbox)\n");  // Use LPC
    } 

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
        /*
         * Dummy initialized for error message "cont.dev_if_p that
         * is not initialized."
         */
        con = &cont;
        con->dev_if_p = &dev_ifs;

        if (katar_plat_init_smart_eeprom_context(con, MOTHER_BOARD, 0, katar_cookie_contents) == FAILED) {
            printf("\n%s: Init Smart EEPROM context failed\n", __func__);
            return (FAILED);
        }

        act2_init_cont((void *) con);

        /*
         * ACT2 library
         */
        return (act2_prog(0));

        break;

    case 'q':
    case 'Q':
        break;

    default:
        printf("Invalid input %s\n", choice);
        break;
    }

    return (rc);
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
    uchar ctrl_type[CONTROL_TYPE_LEN] = { 0 };

    /* cookie type is MOTHERBOARD */
    if (!aikido_mailbox_flag) {
        if (katar_diagact2_lib_initialize(katar_g_i2c_adapter0, MB_I2C_ADDR_AIKIDO_ACT2) != PASSED) {
            return (FAILED);
        }
    }

    if (katar_tam_lib_read_cookie() == FAILED) {
        printf("\ntam_lib_read_cook fail.\n");
        return (FAILED);
    }

    memcpy(eeprom_data, katar_cookie_contents, COOKIE_SIZE_512);

    pdata =
        search_type_ret_addr_of_first_data(katar_cookie_contents,
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



/*************************************************************************
 Function: katar_tam_lib_read_cookie
 *
 * This function is read the MB cookie content by tam library
 *
 * Input: none 
 *
 * Output: PASSED / FAILED
 **************************************************************************
*/
int katar_tam_lib_read_cookie(void)
{
    void *platform_opaque_handle = NULL;
    uint16_t platform_buffer_size = PLATFORM_BUFF_SIZE;
    int ret_val;
    uint8_t use_interrupt = 0;                                                                                                                                                                          
    uint16_t mbx_msg_size = 0x700;
    uint32_t mbx_reg_base_addr = MBX_REG_BASE_ADDR;
    sc_context *con, cont;
    con = &cont;
    
    if (aikido_mailbox_flag) {
        /* Initialize Mailbox */
        ret_val = tam_lib_device_open_mailbox((void *)con, use_interrupt,
                                              mbx_msg_size, mbx_reg_base_addr,
                                              &katar_platform_tam_handle);
        if (ret_val != TAM_RC_OK) {
            /* handle error */
            printf("\n ERROR: Cannot Initialize Mailboxe. status 0x%x\n", ret_val);
            return (FAILED);
        }
    } else {
        if (katar_platform_tam_handle == NULL) {
            ret_val = tam_lib_device_open(platform_opaque_handle,
                                          platform_buffer_size,
                                          &katar_platform_tam_handle);
            if (ret_val != TAM_RC_OK) {
                printf("\n TAM lib: Cannot open handler: status = 0x%x",
                       ret_val);
                printf("\n tam_handle = %p ", katar_platform_tam_handle);
                return (FAILED);
            }
        }
    }

    ret_val = tam_lib_scc_read_eeprom(katar_platform_tam_handle,
                                (unsigned char *) katar_cookie_contents,
                                EEPROM_RD_WR_LENGTH, EEPROM_WRITE_ADDR);
    if (ret_val != TAM_RC_OK) {
        printf("\n *** ERROR: tam_lib_scc_read_cookie. ret_val = 0x%x",
               ret_val);
        printf("\n Cannot read data from the EEPROM (Cookie). \n");
        return (FAILED);
    }
    ret_val = tam_lib_device_close(&katar_platform_tam_handle);
    if (ret_val != TAM_RC_OK) {
        printf("\n TAM lib: Cannot close handler: status = 0x%x", ret_val);
        return (FAILED);
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

    if (katar_platform_tam_handle == NULL)
    {
        /* cookie type is MOTHERBOARD */
        if (!aikido_mailbox_flag) {
            if (katar_diagact2_lib_initialize(katar_g_i2c_adapter0, MB_I2C_ADDR_AIKIDO_ACT2) != PASSED) {
                return (FAILED);
            }    
        } 

        katar_platform_init_smart_context(con,0,katar_g_cookie_type);

        if (katar_tam_lib_read_cookie() == FAILED) {
            printf("\ntam_lib_read_cook fail.\n");
            return (FAILED);
        }
    }

    pdata = search_type_ret_addr_of_first_data(katar_cookie_contents, PRODUCT_ID,
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

    if (katar_platform_tam_handle == NULL)
    {
        /* cookie type is MOTHERBOARD */
        if (!aikido_mailbox_flag) {
            if (katar_diagact2_lib_initialize(katar_g_i2c_adapter0, MB_I2C_ADDR_AIKIDO_ACT2) != PASSED) {
                return (FAILED);
            }   
        }

        katar_platform_init_smart_context(con,0,katar_g_cookie_type);

        if (katar_tam_lib_read_cookie() == FAILED) {
            printf("\ntam_lib_read_cook fail.\n");
            return (FAILED);
        }

        memcpy(eeprom_data, katar_cookie_contents, COOKIE_SIZE_512);
    }

    pdata =
        search_type_ret_addr_of_first_data(katar_cookie_contents, PCB_SERIAL_NUM,
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



int get_tlv_serial (uchar *eeprom_data, char *serial, uchar ser_tlv_field)
{

    uchar *data_ptr;
    uchar num_byte;
    sc_context *con, cont;

    con = &cont;

    if (katar_platform_tam_handle == NULL)
    {
        /* cookie type is MOTHERBOARD */
        if (!aikido_mailbox_flag) {
            if (katar_diagact2_lib_initialize(katar_g_i2c_adapter0, MB_I2C_ADDR_AIKIDO_ACT2) != PASSED) {
                return (FAILED);
            }   
        }

        katar_platform_init_smart_context(con,0,katar_g_cookie_type);

        if (katar_tam_lib_read_cookie() == FAILED) {
            return (FAILED);
        }
    }
    memcpy(eeprom_data, katar_cookie_contents, COOKIE_SIZE_512);

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


/*
 * Function: katar_load_cookie_eeprom
 *
 * Description:
 *   This function is to load cookie on EEPROM.
 *
 * Parameters:
 *    none
 *
 * Returns:
 *   PASSED/FAILED
 */
int katar_load_cookie_eeprom(void)
{
    char fname[32];
    size_t size = 0;
    uint32_t rc = FAILED;

    if (system("i2cdetect -y 0 | grep 51 > /dev/null")) {
        printf("EERPROM not found!\n");
        return (FAILED);
    } 

    sprintf(fname, "/cookie_256B.txt");
    if (file_exist(fname, &size)) {
        printf("/cookie_256B.txt existed. \n");
        rc = katar_write_cookie_eeprom_from_file();
        if (rc != PASSED) {
            printf("%s: Write cookie into EEPROM Failed !!!\n", __FUNCTION__);
        }
    } else {
        printf("/cookie_256B.txt does not existed. \n");
        return (FAILED);
    }

    return (PASSED);
}


/*
 * Function: katar_write_cookie_eeprom_from_file
 *
 * Description:
 *   This function is to write cookie from file.
 *
 * Parameters:
 *    none
 *
 * Returns:
 *   PASSED/FAILED
 */
int katar_write_cookie_eeprom_from_file(void)
{
    uchar out_prom[COOKIE_EEPROM_SIZE*2], *out_p;
    char buf_tmp[COOKIE_EEPROM_SIZE*2], *buf_p;
    int  i;
    uint32_t rc = FAILED;
    FILE *fp;

    memset(buf_tmp, 0, (COOKIE_EEPROM_SIZE*2));

    printf("Write EEPROM contents from /cookie_256B.txt \n");
    fp = fopen("/cookie_256B.txt", "r");
    if (!fp) {
        printf("file /cookie_256B.txt is not existed\n");
        return (FAILED);
    }

    buf_p = buf_tmp;
    for (i = 0; i < COOKIE_EEPROM_SIZE; i++) {
        fscanf(fp, "%s", buf_p);
        out_prom[i] = strtol(buf_p, NULL, 16);
    }
    fclose(fp);

    out_p = out_prom;
    for (i = 0; i < COOKIE_EEPROM_SIZE; i++) {
        if (!(i % 16)) {
            printf("\n  0x%02x:", i); }
        printf(" %02x", *out_p++);
    }

    if (getc_answer("\n\nWriting above cookie?", "yn", 'n') == 'y') {
        /* save back to original cookie */
        rc = write_cookie_eeprom_reg(0, out_prom, COOKIE_EEPROM_SIZE);
    } else {
        printf("Abort..\n ");
    }

    return(rc);
}




/******************************************************************************
 *
 * function   : write_cookie_eeprom_reg
 * Description: Wrapper to write cookie EEPROM's register.
 * Inputs     : addr_ptr_id - pointer register ID.
 *              data_buf_p - pointer to data buffer
 *              size - number of bytes to be read.
 * Outputs    : PASSED/FAILED.
 *
 ******************************************************************************/
int write_cookie_eeprom_reg (int addr_ptr_id, uchar * data_buf_p, int size)
{
    n2g_i2c_dev_t i2c_dev;
    n2g_i2c_if_t  i2c_if;
    unsigned int rc, i;
    //    char *out_p;
    uint8_t retry = 0;

    i2c_if.i2c_dev = COOKIE_EEPROM_ADDR;
    i2c_if.offset = 0;  // write from address 0
    i2c_if.size = 1;  // write 1 byte at a time
    i2c_if.buf = (char *)data_buf_p;

    /* Init device structure */
    i2c_dev.bus_no = CPU_I2C0;
    i2c_dev.rd_hd_size = 0; 
    i2c_dev.wr_hd_size = 1;
    i2c_dev.dev_addr = COOKIE_EEPROM_ADDR;

    cookie_eeprom_i2c_fd = get_i2c_fd(0);

    /* Set I2C device to SLAVE mode */
    if (cookie_eeprom_i2c_fd <= 0) {
         cterr('f', 0, "/dev/i2c-0 is not opened correctly.");
         return (FAILED);
    } else {
        if ((rc = ioctl(cookie_eeprom_i2c_fd, I2C_SLAVE, i2c_dev.dev_addr)) < 0) {
            cterr('f', 0, "%s at %s: unable to connect to device %#x. "
                          "rc = %#x", __FUNCTION__, __FILE__,
                          i2c_dev.dev_addr, rc);
            return (FAILED);
        } else {
            i2c_dev.fp = cookie_eeprom_i2c_fd;
        }
    }

    /* Get Registers value */
    /* Setup the interface struct for I2C API read */
    i2c_if.i2c_bus_type = i2c_dev.bus_no;

    //out_p = data_buf_p;

    for (i=0; i<size; i++) {
        if (diagflag_xram & D_DEBUG_OPTIONS) {
            printf("===== writing byte: %d \n", i);
        }
        msleep(10);  // avoid fail and retry
        rc = api_mb_i2c_write(&i2c_dev, i2c_if.offset, i2c_if.size, (char *)i2c_if.buf);
        retry = 0;
        while((rc != PASS) && (retry < 6)) {
            msleep(500);
            rc = api_mb_i2c_write(&i2c_dev, i2c_if.offset, i2c_if.size, (char *)i2c_if.buf);
            retry++;
        }    
        if (rc != PASSED) {
            if (rc == E_I2C_INV_ACK) {
                printf("%s: I2C device is not installed.\n", __FUNCTION__);
            } else {
                printf("%s: I2C Write Failed !!!n", __FUNCTION__);
                printf("(Bus%d, Dev 0x%01X, offset 0x%01X)\n", i2c_dev.bus_no, i2c_if.i2c_dev, i2c_if.offset);
            }
            return (FAILED);
        } 
        i2c_if.offset++;
        i2c_if.buf++;
    }

    printf("%s: Cookie Writing Done.\n", __FUNCTION__);

    return (PASSED);
}



/******************************************************************************
 *
 * Function   : init_cookie_pla_i2c_struct
 * Description: To init i2c_dev structure.
 * Inputs     : dev_object_t *i2c_dev
 * Outputs    : PASSED/FAILED
 *  
 ******************************************************************************/
int init_cookie_eeprom_i2c_struct (n2g_i2c_dev_t *i2c_dev) {
    uint32_t rc = FAILED;
        
    i2c_dev->bus_no = CPU_I2C0;
    i2c_dev->rd_hd_size = 1;
    i2c_dev->wr_hd_size = 1;
    i2c_dev->dev_addr = COOKIE_EEPROM_ADDR;
        
    cookie_eeprom_i2c_fd = get_i2c_fd(0);
                           
    /* Set I2C device to SLAVE mode */
    if (cookie_eeprom_i2c_fd <= 0) {
         cterr('f', 0, "/dev/i2c-0 is not opened correctly.");
         return (FAILED);
    } else {
        if ((rc = ioctl(cookie_eeprom_i2c_fd, I2C_SLAVE, i2c_dev->dev_addr)) < 0) {
            cterr('f', 0, "%s at %s: unable to connect to device %#x. "
                          "rc = %#x", __FUNCTION__, __FILE__,
                          i2c_dev->dev_addr, rc);
            return (FAILED);
        } else {
            i2c_dev->fp = cookie_eeprom_i2c_fd;
        }
    }
    return (PASSED);
}



/*
 * Function: katar_read_cookie_eeprom
 *
 * Description:
 *   This function is to read cookie on EEPROM boards.
 *
 * Parameters:
 *    none
 *
 * Returns:
 *   PASSED/FAILED
 */
int katar_read_cookie_eeprom(void)
{
    uchar out_prom[COOKIE_EEPROM_SIZE*2];

    if (system("i2cdetect -y 0 | grep 51 > /dev/null")) {
        printf("EERPROM not found!\n");
        return (FAILED);
    } 

    if (katar_read_cookie_eeprom_x(out_prom)) {
        return (FAILED);
    }    
    return (PASSED);
}



 
/*
 * Function   : katar_read_cookie_eeprom_x
 * Description: This function is to read cookie on EEPROM boards.
 * Inputs     : uchar *out_prom
 * Outputs    : PASSED/FAILED
 */
int katar_read_cookie_eeprom_x(uchar *out_prom)
{
    uint32_t rc;
    uchar *out_prom_start;
    int  j, offset=0;

    printf("Read cookie contents from EEPROM\n");

    out_prom_start = out_prom;  // save start address
    while(offset < COOKIE_EEPROM_SIZE) {
        rc = read_cookie_eeprom_reg(offset, out_prom, 16);  // read 16 bytes at a time
        if (rc == FAILED) {
            return (rc);
        }
        printf("\n  0x%02x:", offset);
        for (j=0; j<16; j++) {
            printf(" %02x", *out_prom++);
        }
        offset+=16;
    }
    printf("\n\n%s: Cookie Reading Done.\n", __FUNCTION__);

    out_prom = out_prom_start;
    return (PASSED);
}



/******************************************************************************
 *
 * function   : read_cookie_eeprom_reg
 * Description: Wrapper to read cookie EEPROM's register.
 * Inputs     : addr_ptr_id - pointer register ID.
 *              data_buf_p - pointer to data buffer
 *              size - number of bytes to be read.
 * Outputs    : PASSED/FAILED.
 *
 ******************************************************************************/
int read_cookie_eeprom_reg (int addr_ptr_id, uchar * data_buf_p, int size)
{
    n2g_i2c_dev_t i2c_dev;
    n2g_i2c_if_t  i2c_if;
    uint32_t      rc = FAILED;
    uint8_t retry = 0;
        
    /* Init device structure */
    if (init_cookie_eeprom_i2c_struct(&i2c_dev) != PASSED) {
        printf("Init Cookie EEPROM i2c_dev struct failed.");
        return (FAILED);
    }
 
    /* Get Registers value */
    /* Setup the interface struct for I2C API read */
    i2c_if.i2c_bus_type = i2c_dev.bus_no;
    i2c_if.i2c_dev = i2c_dev.dev_addr;
    i2c_if.size = size; 
    i2c_if.offset = addr_ptr_id;
    i2c_if.buf = (char *)data_buf_p;

    rc = api_mb_i2c_read(&i2c_dev, i2c_if.offset, i2c_if.size, (char *)i2c_if.buf);
    retry = 0;
    while((rc != PASS) && (retry < 3)) {
        msleep(1000);
        rc = api_mb_i2c_read(&i2c_dev, i2c_if.offset, i2c_if.size, (char *)i2c_if.buf);
        retry++;
    } 
    if (rc != PASSED) {
        printf("%s: EEPROM Read Failed !!!n", __FUNCTION__);
        printf("(Bus%d, Dev 0x%01X, offset 0x%01X)\n",
                           i2c_dev.bus_no, i2c_if.i2c_dev, i2c_if.offset);
        return (FAILED);
    }

    //printf("%s: Cookie Reading Done.\n", __FUNCTION__);
    return (PASSED);
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

    if (katar_platform_tam_handle == NULL) {
        /* cookie type is MOTHERBOARD */
        if (g_cookie_type == MOTHER_BOARD) {
            if (!aikido_mailbox_flag) {
                if (katar_diagact2_lib_initialize(katar_g_i2c_adapter0, MB_I2C_ADDR_AIKIDO_ACT2) != PASSED) {
                    return (FAILED);
                }
            }
            katar_platform_init_smart_context(con,0,g_cookie_type);
        } else if (g_cookie_type == DAUGHTER_CARD) { 
            /* Wifi card not support ACT2 AIKIDO or TAM Mailbox */
            aikido_mailbox_flag = FALSE;
            //if (diagact2_lib_initialize(g_i2c_adapter2, WIFI_I2C_ADDR_ACT2) != PASSED) {
            //return (FAILED);
            //}
            katar_platform_init_smart_context(con,0,g_cookie_type);
        } else if (g_cookie_type == PLUGGABLE_CARD) { 
            /* PLUG should be link by plat_init_smart_eeprom_context */
            aikido_mailbox_flag = FALSE;
            //platform_init_smart_context(con,g_plug_slot,g_cookie_type);
        } else {
                return (FAILED);
        }

        if (katar_tam_lib_read_cookie() == FAILED) {
            return (FAILED);
        }
    }

    printf("\nUDI");
    pdata = search_type_ret_addr_of_first_data(katar_cookie_contents, PRODUCT_ID,
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

    pdata = search_type_ret_addr_of_first_data(katar_cookie_contents, VERSION_ID,
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
        search_type_ret_addr_of_first_data(katar_cookie_contents, PCB_SERIAL_NUM,
                                           &ret_num_of_bytes, 0);
    } else {
        pdata =
        search_type_ret_addr_of_first_data(katar_cookie_contents, CHASSIS_SERIAL_NUM,
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



/*
 *------------------------------------------------------------------
 * $Log: platform_cookie.c,v $
 * Revision 1.3  2021/04/15 01:55:08  peteteng
 * Upgrade to TAM Lib-v3.4.24 based on PRRQ#5091945
 *
 * Revision 1.2  2019/06/14 05:24:49  mikech2
 * Collapse katar-branch00 to Main Trunk
 *
 * Revision 1.1.2.9  2019/05/08 02:37:02  peteteng
 * Update PCAMAP-v15
 *
 * Revision 1.1.2.8  2019/05/06 05:47:24  peteteng
 * Update PCAMAP-v14
 *
 * Revision 1.1.2.7  2019/05/02 03:26:23  peteteng
 * Update PCAMAP-v13
 *
 * Revision 1.1.2.6  2019/04/26 01:17:34  mikech2
 * clean up Makefile
 *
 * Revision 1.1.2.5  2019/04/16 08:33:16  peteteng
 * Fix compiler warning
 *
 * Revision 1.1.2.4  2019/04/12 01:35:55  peteteng
 * Code cleanup
 *
 * Revision 1.1.2.3  2019/03/13 03:34:15  peteteng
 * Code cleanup
 *
 * Revision 1.1.2.2  2019/02/12 08:06:29  mikech2
 * rename katar_*.h files
 *
 * Revision 1.1.2.1  2019/01/29 01:54:20  mikech2
 * rename katar_* files
 *
 * Revision 1.1.2.16  2018/12/28 09:46:27  peteteng
 * Support Aikido FW upgrade thru UserLogic FPGA I2C
 *
 * Revision 1.1.2.15  2018/12/27 00:42:24  peteteng
 * Support Aikido thru UserLogic FPGA I2C
 *
 * Revision 1.1.2.14  2018/12/21 07:28:45  peteteng
 * Add ACT2 programming thru LPC
 *
 * Revision 1.1.2.13  2018/12/20 09:10:57  peteteng
 * Add FPGA I2C read/write/scan/reset util
 *
 * Revision 1.1.2.12  2018/12/14 02:40:46  peteteng
 * Add cookie util through LPC mailbox
 *
 * Revision 1.1.2.10  2018/12/13 15:41:06  peteteng
 * Fix cookie util in Aikido FW-v10015
 *
 * Revision 1.1.2.9  2018/12/06 08:32:25  mikech2
 * Fine-tune Aikido I2C r/w and fix Aikido update FW utility
 *
 * Revision 1.1.2.8  2018/12/01 10:39:00  peteteng
 * Speed up Aikido cookie
 *
 * Revision 1.1.2.7  2018/11/29 03:19:58  peteteng
 * Fix Aikido cookie - read one byte
 *
 * Revision 1.1.2.6  2018/11/17 11:09:49  peteteng
 * Fix Aikido cookie issue
 *
 * Revision 1.1.2.5  2018/11/08 02:21:52  peteteng
 * Remove names in comment
 *
 * Revision 1.1.2.4  2018/10/30 08:24:20  peteteng
 * Add EEPROM and Aikido ACT2 verification
 *
 * Revision 1.1.2.3  2018/10/30 06:32:01  peteteng
 * Change cookie util order; remove i2c-1 inspection; add ACT2 programming case Qq
 *
 * Revision 1.1.2.2  2018/10/23 08:14:02  peteteng
 * Update PCAMAP-v.4 and auto detect sku
 *
 * Revision 1.1.2.1  2018/10/22 08:02:26  mikech2
 * Move project folder to common/src/katar/x86
 *
 * Revision 1.1.2.7  2018/10/22 03:04:32  peteteng
 * use common code tam_act2_utils.c without smart_cookie.c
 *
 * Revision 1.1.2.6  2018/10/08 09:09:21  peteteng
 * Fix EEPROM write retry issue
 *
 * Revision 1.1.2.5  2018/10/04 09:40:27  peteteng
 * Add alter MB CPU cookie on EEPROM Utility
 *
 * Revision 1.1.2.4  2018/09/20 06:44:59  peteteng
 * Add load/read 256-byte cookie on EEPROM in cookie utility
 *
 * Revision 1.1.2.3  2018/07/12 08:02:08  peteteng
 * add tam_lib_platform_write/read
 *
 * Revision 1.1.2.2  2018/06/29 03:40:01  peteteng
 * Add ACT2 utility Menu
 *
 * Revision 1.1.2.1  2018/06/26 06:30:09  peteteng
 * Add Aikido Cookie menu
 *
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */


