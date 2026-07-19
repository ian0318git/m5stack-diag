/* $Id: platform_pem_eeprom_modify.c,v 1.9 2017/07/10 02:27:51 leschen Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/x86/platform_pem_eeprom_modify.c,v $
 *------------------------------------------------------------------
 *
 * platform_pem_eeprom_modify.c: 
 *           Utility to modify the raw hex data contents
 *           of the PEM EEPROM.
 *
 * May 2013 - porting the code from Nightster
 *
 * Copyright (c) 2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 * Original Author: Jay McCloy
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "endians.h"
#include "common.h"
#include "defs.h"
#include "proto.h"
#include "error.h"
#include "nvmonvars.h"
#include "mon_plat_defs.h"
#include "cross_platform.h"
#include "platform_i2c.h"
#include "menu.h"
#include "plat_defs.h"

#include "types.h"
#include "queryflags.h"
#include "platform_idprom.h"
#include "platform_idprom_utils.h"
#include "platform_psu.h" /* for cookie related */
#include "cli_cmd.h" /* for cookie related */


/* Global variable */
static uchar buf[PEM_EEPROM_SIZE];
static uint32 size = PEM_EEPROM_SIZE;
extern char *promsyntax;
static int max_size;
static int psu_no_now = 1;

/* Function prototype */
static int pem_modify(void);
static int pem_init(int);
static void pem_eeprom_dflt_init_cmd_f(void);
static int read_pem_eeprom_util(void);
static int pem_show_cookie(void);
static void pem_monitor_dump_wr(void);
static void pem_monitor_write_wr(void);
extern void build_pem_fan_menu_wr(void);

/* extern function */
extern int idprom_get_field_data(uchar *, int, int,
                          void *, int, boolean);
extern boolean idprom_update_field_data(uchar *, int,
        boolean, int, int, void *, ushort *, ushort *);
extern void idprom_print_all_fields(uchar *, int);
extern int mcp_print_buffer_data_with_ascii(uchar *, uint32, int, int);
extern int rp1ruve_pem_write_eeprom(uchar *, int, uint32, int);
extern int rp1ruve_pem_read_eeprom(uchar *, int, uint32, int, int);
extern void rp1ruve_pem_display_eeprom (uchar *, int, int);
extern int pem_monitor_write(int);
extern int pem_monitor_dump(int);
extern int16_t pem_force_error_f(void);
extern int16_t pem_alert_enable_f(void);
extern int16_t pem_check(void);
extern int16_t pem_clear(void);
extern void build_pem_fan_menu(int);
extern void build_psu_menu(uint32_t);
extern boolean is_overlord(void);
extern int show_psu_cookie(int);
extern int cookie_4_processor_x (uchar *, int, int, int, cli_cookie_cmd *);

/* 1RU AC Power Supply PCAMAP 95-10391-01-01  EDCS-686133
 * History: rev 0.3  10/08/2008  Dave Valley, 
 *
 * DO NOT CHANGE this buffer without referring to the latest 1RU AC PEM PCAMAP.
 *
 */
static uchar pem1ru_ac_idprom[EEPROM_BYTES] = {
    /*0000*/ 0x04, 0xFF, 0x41, 0x00, 0x03, 0x89, 0x56, 0x30, 0x30, 0x20, 0xCB, 0x94, 0x4E, 0x32, 0x32, 0x30,
    /*0010*/ 0x30, 0x2D, 0x50, 0x41, 0x43, 0x2D, 0x34, 0x30, 0x30, 0x57, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    /*0020*/ 0xC1, 0x8B, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0xDF, 0x45, 0x01,
    /*0030*/ 0x55, 0x01, 0xB4, 0x02, 0x42, 0x30, 0x35, 0x88, 0x00, 0x00, 0x00, 0x00, 0x0B, 0x00, 0x03, 0x00,
    /*0040*/ 0x81, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0xC6, 0x8A, 0x55, 0x4E, 0x41, 0x53, 0x53, 0x49, 0x47,
    /*0050*/ 0x4E, 0x45, 0x44, 0xC4, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC5, 0x08, 0x00,
    /*0060*/ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xDA, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*0070*/ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC9, 0x11, 0x00, 0x01, 0x01, 0xDE, 0x15,
    /*0080*/ 0xF4, 0x07, 0xC6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4A, 0xF3, 0x00, 0x04, 0x00,
    /*0090*/ 0x06, 0x00, 0xFA, 0xC7, 0x16, 0x13, 0x00, 0x32, 0x01, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x88,
    /*00A0*/ 0x00, 0x00, 0x21, 0x00, 0x2E, 0xE0, 0x00, 0x00, 0x78, 0x00, 0x0B, 0xC7, 0x19, 0x12, 0x00, 0x32,
    /*00B0*/ 0x02, 0x20, 0x20, 0x50, 0x45, 0x4D, 0x20, 0x49, 0x6E, 0x02, 0x00, 0x8D, 0x00, 0x64, 0x00, 0x5F,
    /*00C0*/ 0x00, 0x5A, 0x00, 0x50, 0x00, 0xC5, 0xCC, 0xA0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*00D0*/ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*00E0*/ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    /*00F0*/ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
};  /* End of 1RU/AC PCAMAP */

static uchar pem1ru_ac_idprom_32k[EEPROM_BYTES] = {
    /*0000*/ 0x04, 0xFF, 0x41, 0x00, 0x03, 0x89, 0x56, 0x30, 0x30, 0x20, 0xCB, 0x94, 0x4E, 0x32, 0x32, 0x30,
    /*0010*/ 0x30, 0x2D, 0x50, 0x41, 0x43, 0x2D, 0x34, 0x30, 0x30, 0x57, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    /*0020*/ 0xC1, 0x8B, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0xDF, 0x45, 0x01,
    /*0030*/ 0x55, 0x01, 0xB4, 0x02, 0x42, 0x30, 0x35, 0x88, 0x00, 0x00, 0x00, 0x00, 0x0B, 0x00, 0x03, 0x00,
    /*0040*/ 0x81, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0xC6, 0x8A, 0x55, 0x4E, 0x41, 0x53, 0x53, 0x49, 0x47,
    /*0050*/ 0x4E, 0x45, 0x44, 0xC4, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC5, 0x08, 0x00,
    /*0060*/ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xDA, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*0070*/ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC9, 0x11, 0x00, 0x01, 0x01, 0xDE, 0x15,
    /*0080*/ 0xF4, 0x07, 0xC6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4A, 0xF3, 0x00, 0x04, 0x00,
    /*0090*/ 0x06, 0x00, 0xFA, 0xC7, 0x16, 0x13, 0x00, 0x32, 0x01, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x88,
    /*00A0*/ 0x00, 0x00, 0x21, 0x00, 0x2E, 0xE0, 0x00, 0x00, 0x78, 0x00, 0x0B, 0xC7, 0x19, 0x12, 0x00, 0x32,
    /*00B0*/ 0x02, 0x20, 0x20, 0x50, 0x45, 0x4D, 0x20, 0x49, 0x6E, 0x02, 0x00, 0x8D, 0x00, 0x64, 0x00, 0x5F,
    /*00C0*/ 0x00, 0x5A, 0x00, 0x50, 0x00, 0xC5, 0xC7, 0x19, 0x12, 0x00, 0x32, 0x03, 0x20, 0x50, 0x45, 0x4D,
    /*00D0*/ 0x20, 0x4F, 0x75, 0x74, 0x02, 0x00, 0x8E, 0x00, 0x64, 0x00, 0x5F, 0x00, 0x5A, 0x00, 0x50, 0x00,
    /*00E0*/ 0x62, 0xC7, 0x19, 0x12, 0x00, 0x32, 0x04, 0x20, 0x50, 0x45, 0x4D, 0x20, 0x49, 0x6E, 0x74, 0x02,
    /*00F0*/ 0x00, 0x8F, 0x00, 0x64, 0x00, 0x5F, 0x00, 0x5A, 0x00, 0x50, 0x00, 0x6D, 0xCC, 0xA0, 0x00, 0x00,
    /*0100*/ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*0110*/ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF,
    /*0120*/ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    /*0130*/ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    /*0140*/ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    /*0150*/ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    /*0160*/ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    /*0170*/ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    /*0180*/ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    /*0190*/ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    /*01A0*/ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    /*01B0*/ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    /*01C0*/ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    /*01D0*/ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    /*01E0*/ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    /*01F0*/ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};  /* End of 1RU/AC PCAMAP */


/* 1RU DC Power Supply PCAMAP 95-10392-01 EDCS-686134
 * History: rev 0.3  10/08/2008  Dave Valley, 
 *
 * DO NOT CHANGE this buffer without referring to the latest 1RU DC PEM PCAMAP.
 *
 */
static uchar pem1ru_dc_idprom[EEPROM_BYTES] = {
    /*              0    1    2    3    4    5    6    7    
		    8    9    A    B    C    D    E    F */
    /* 0x00: */  0x01,0xFF,0x41,0x01,0x00,0x89,0x56,0x30,
    /* 0x08: */  0x30,0x20,0xCB,0x92,0x41,0x53,0x52,0x31,
    /* 0x10: */  0x30,0x30,0x31,0x2D,0x50,0x57,0x52,0x2D,
    /* 0x18: */  0x44,0x43,0x20,0x20,0x20,0x20,0xC1,0x8B,
    /* 0x20: */  0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
    /* 0x28: */  0x20,0x20,0x20,0xC0,0x46,0x01,0x55,0x00,
    /* 0x30: */  0x01,0x53,0x02,0x42,0x30,0x31,0x80,0x00,
    /* 0x38: */  0x00,0x00,0x00,0x0B,0x01,0x03,0x00,0x81,
    /* 0x40: */  0x00,0x00,0x00,0x00,0x04,0x00,0xC6,0x8A,
    /* 0x48: */  0x55,0x4E,0x41,0x53,0x53,0x49,0x47,0x4E,
    /* 0x50: */  0x45,0x44,0xC4,0x08,0x00,0x00,0x00,0x00,
    /* 0x58: */  0x00,0x00,0x00,0x00,0xC5,0x08,0x00,0x00,
    /* 0x60: */  0x00,0x00,0x00,0x00,0x00,0x00,0xDA,0x10,
    /* 0x68: */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    /* 0x70: */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    /* 0x78: */  0xC9,0x11,0x00,0x01,0x01,0xD7,0x03,0x79,
    /* 0x80: */  0x02,0x8F,0x00,0x00,0x00,0x00,0x00,0x00,
/* #warning "addr 0x8e not defined in spec" */
    /* 0x88: */  0x00,0x00,0x1A,0xC7,0x12,0x05,/*0x??*/0x58,0x01,
    /* 0x90: */  0x20,0x20,0x20,0x20,0x00,0x25,0x33,0x00,
/* #warning "addr 0x8e not defined in spec" */
    /* 0x98: */  0x0C,0x28,0x00,0x39,0x32,0x00,0x2B,/*0x??*/0xC7,
    /* 0xA0: */  0x17,0x09,0x58,0x02,0x20,0x20,0x20,0x49,
    /* 0xA8: */  0x6E,0x6C,0x65,0x74,0x01,0x00,0x00,0x64,
    /* 0xB0: */  0x00,0x5A,0x00,0x50,0x00,0x46,0x00,0xEC,
    /* 0xB8: */  0xC7,0x17,0x09,0x58,0x03,0x49,0x6E,0x74,
    /* 0xC0: */  0x65,0x72,0x6E,0x61,0x6C,0x01,0x01,0x00,
    /* 0xC8: */  0xB4,0x00,0xA0,0x00,0x8C,0x00,0x78,0x00,
    /* 0xD0: */  0x05,0xCC,0xA0,0x00,0x00,0x00,0x00,0x00,
    /* 0xD8: */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    /* 0xE0: */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    /* 0xE8: */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    /* 0xF0: */  0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,
    /* 0xF8: */  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF
};  /* End of 1RU/DC PCAMAP */

static uchar pem1ru_dc_idprom_32k[EEPROM_BYTES*2] = {
    /*              0    1    2    3    4    5    6    7    
		    8    9    A    B    C    D    E    F */
    /* 0x00: */  0x01,0xFF,0x41,0x01,0x00,0x89,0x56,0x30,
    /* 0x08: */  0x30,0x20,0xCB,0x92,0x41,0x53,0x52,0x31,
    /* 0x10: */  0x30,0x30,0x31,0x2D,0x50,0x57,0x52,0x2D,
    /* 0x18: */  0x44,0x43,0x20,0x20,0x20,0x20,0xC1,0x8B,
    /* 0x20: */  0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
    /* 0x28: */  0x20,0x20,0x20,0xC0,0x46,0x01,0x55,0x00,
    /* 0x30: */  0x01,0x53,0x02,0x42,0x30,0x31,0x80,0x00,
    /* 0x38: */  0x00,0x00,0x00,0x0B,0x01,0x03,0x00,0x81,
    /* 0x40: */  0x00,0x00,0x00,0x00,0x04,0x00,0xC6,0x8A,
    /* 0x48: */  0x55,0x4E,0x41,0x53,0x53,0x49,0x47,0x4E,
    /* 0x50: */  0x45,0x44,0xC4,0x08,0x00,0x00,0x00,0x00,
    /* 0x58: */  0x00,0x00,0x00,0x00,0xC5,0x08,0x00,0x00,
    /* 0x60: */  0x00,0x00,0x00,0x00,0x00,0x00,0xDA,0x10,
    /* 0x68: */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    /* 0x70: */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    /* 0x78: */  0xC9,0x11,0x00,0x01,0x01,0xD7,0x03,0x79,
    /* 0x80: */  0x02,0x8F,0x00,0x00,0x00,0x00,0x00,0x00,
/* #warning "addr 0x8e not defined in spec" */
    /* 0x88: */  0x00,0x00,0x1A,0xC7,0x12,0x05,/*0x??*/0x58,0x01,
    /* 0x90: */  0x20,0x20,0x20,0x20,0x00,0x25,0x33,0x00,
/* #warning "addr 0x8e not defined in spec" */
    /* 0x98: */  0x0C,0x28,0x00,0x39,0x32,0x00,0x2B,/*0x??*/0xC7,
    /* 0xA0: */  0x17,0x09,0x58,0x02,0x20,0x20,0x20,0x49,
    /* 0xA8: */  0x6E,0x6C,0x65,0x74,0x01,0x00,0x00,0x64,
    /* 0xB0: */  0x00,0x5A,0x00,0x50,0x00,0x46,0x00,0xEC,
    /* 0xB8: */  0xC7,0x17,0x09,0x58,0x03,0x49,0x6E,0x74,
    /* 0xC0: */  0x65,0x72,0x6E,0x61,0x6C,0x01,0x01,0x00,
    /* 0xC8: */  0xB4,0x00,0xA0,0x00,0x8C,0x00,0x78,0x00,
    /* 0xD0: */  0x05,0xCC,0xA0,0x00,0x00,0x00,0x00,0x00,
    /* 0xD8: */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    /* 0xE0: */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    /* 0xE8: */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    /* 0xF0: */  0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,
    /* 0xF8: */  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xaa,

    /* 0x100: */  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    /* 0x108: */  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    /* 0x110: */  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    /* 0x118: */  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    /* 0x120: */  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    /* 0x128: */  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    /* 0x130: */  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    /* 0x138: */  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    /* 0x140: */  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    /* 0x148: */  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    /* 0x150: */  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    /* 0x158: */  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    /* 0x160: */  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    /* 0x168: */  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    /* 0x170: */  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    /* 0x178: */  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    /* 0x180: */  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    /* 0x188: */  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    /* 0x190: */  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    /* 0x198: */  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    /* 0x1a0: */  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    /* 0x1a8: */  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    /* 0x1b0: */  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    /* 0x1b8: */  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    /* 0x1c0: */  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    /* 0x1c8: */  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    /* 0x1d0: */  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    /* 0x1d8: */  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    /* 0x1e0: */  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    /* 0x1e8: */  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    /* 0x1f0: */  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    /* 0x1f8: */  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF
};  /* End of 1RU/DC PCAMAP */


/*
 * PSU Menu
 */
static submenu_xtable_t psu_menu_table[] = {
    {"PSU cookie utility",                  (PFT)pem_show_cookie,    0,
        0,                  (PFT)0, 0, (PFT)0, 0},
    {"PSU EEPROM Write",                    (PFT)pem_modify,    0,
        0,                  (PFT)0, 0, (PFT)0, 0},
    {"PSU EEPEOM Read",                     (PFT)read_pem_eeprom_util,    0,
        0,                  (PFT)0, 0, (PFT)0, 0},
    {"PSU EEPROM Default init setting",     (PFT)pem_eeprom_dflt_init_cmd_f,    0,
        0,                  (PFT)0, 0, (PFT)0, 0},
    {"PEM Reg Write",                       (PFT)pem_monitor_write_wr,    0,
        0,                  (PFT)0, 0, (PFT)0, 0},
    {"PEM Reg Read",                        (PFT)pem_monitor_dump_wr,    0,
        0,                  (PFT)0, 0, (PFT)0, 0},
    {"PEM Force Error",                     (PFT)pem_force_error_f,    0,
        0,                  (PFT)0, 0, (PFT)0, 0},
    {"PEM Alert Enable",                    (PFT)pem_alert_enable_f,    0,
        0,                  (PFT)0, 0, (PFT)0, 0},
    {"PEM Check",                           (PFT)pem_check,    0,
        0,                  (PFT)0, 0, (PFT)0, 0},
    {"PEM Clear",                           (PFT)pem_clear,    0,
        0,                  (PFT)0, 0, (PFT)0, 0},
    {"PEM FAN Utilities",                   (PFT)build_pem_fan_menu_wr,    0,
        0,                  (PFT)0, 0, (PFT)0, 0},
};

#define PSU_MENU_TABLE_SIZE (sizeof(psu_menu_table) / \
                sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t psu_menu_primary_items[PSU_MENU_TABLE_SIZE +
                MAX_BASE_ITEMS];
static mitem_t psu_menu_secondary_items[PSU_MENU_TABLE_SIZE +
                MAX_BASE_ITEMS];

static struct menuinfo psudiag = {
    "PSU Utility Menu",         /* title */
    0,                          /* title string added by init_empty_menu */
    0,                          /* do not show major flags */
    0,                          /* generic prompt */
    0,                          /* size -- bumped by add_menu_item() */
    psu_menu_primary_items,
};

static struct menuinfo *psudiagp = &psudiag;


/*******************************************************************************
 *
 * function   : build_pem_menu
 * Description: Build menu for PSU.
 * Inputs     : option
 * Outputs    : None
 *
 *******************************************************************************
 */
void build_pem_menu (uint32_t option)
{
    char    t_name[ERR_BUF_SIZE];

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("option = %#x.\n", option);
    }

    /* Get the PSU number */
    psu_no_now = (option >> OVLD_PSU_OFF);
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s:%d psu_no_now = %d.\n", __FUNCTION__, __LINE__, psu_no_now);
    }

    if ((psu_no_now == 0) || (psu_no_now > MAX_NUM_PSU)) {
        cterr('f', 0, "%s:%d Invalid PSU No.(%d)",
              __FUNCTION__, __LINE__, psu_no_now);
        return;
    }

    sprintf(t_name, "PSU %d", psu_no_now);
    testname(t_name);

    printf("PSU %d", psu_no_now);
    /* Check PSU present */
    if (check_psu_present(psu_no_now) != TRUE) {
        return;
    }

    build_primary_submenu(psu_menu_table, PSU_MENU_TABLE_SIZE,
                          "PSU Utility Menu", &psudiagp);
    build_secondary_submenu(psu_menu_table, PSU_MENU_TABLE_SIZE,
                            psu_menu_secondary_items);

    /* Entered with submenu */
    menu(&psudiag, psu_menu_secondary_items, 0);

    return;
}


/*******************************************************************************
 *
 * function   : build_pem_psu_menu
 * Description: Build menu for PSU for Overlord or Juno addording to board type.
 * Inputs     : option
 * Outputs    : None
 *
 *******************************************************************************
 */
void build_pem_psu_menu (uint32_t option)
{
    if (is_overlord()){
        build_psu_menu(option);
    } else {
        build_pem_menu(option);
    }

    return;
}


/*******************************************************************************
 *
 * function   : build_pem_fan_menu_wr
 * Description: Wrapper for build PEM fan menu 
 * Inputs     : None
 * Outputs    : None
 *
 *******************************************************************************
 */
void build_pem_fan_menu_wr (void)
{
    build_pem_fan_menu(psu_no_now);
    return;
}

/*******************************************************************************
 *
 * function   : pem_show_cookie
 * Description: a wrapper for pem cookie utility
 * Inputs     : none
 * Outputs    : pass/fail
 *
 *******************************************************************************
 */
int pem_show_cookie (void)
{
    return(show_psu_cookie(psu_no_now));
}


/*******************************************************************************
 *
 * function   : pem_show_cookie_x
 * Description: a wrapper for pem cookie utility
 * Inputs     : option
 * Outputs    : PASS/FAIL
 *
 *******************************************************************************
 */
int pem_show_cookie_x (boolean mode, cli_cookie_cmd *cmd) 
{
    int rd_size = PEM_EEPROM_SIZE;
    int ps_num;

    if(mode == CLI_MODE) 
       ps_num = cmd->slot;
    else
       ps_num = psu_no_now;
    
    /* get the eeprom data and store into local buffer */
    if (pem_init(ps_num)) 
        return (FAILED);
 
    if (cookie_4_processor_x(buf, PSU_MODULE, 0, rd_size, cmd) == 1) {
        /* write back to eeprom */
        if (rp1ruve_pem_write_eeprom(&buf[0], rd_size, 0, ps_num) != 0) {
            printf("Contents of EEPROM may not be reliable!");
            cterr('f', 0, "PEM:%d EEPROM Write failure!!!", ps_num);
        return (FAILED);
        }
    }

    return PASSED;
}

/*******************************************************************************
 * function   : get_wrdata
 * Description: Build menu for PSU.
 * Inputs     : PSU number
 *              start address location
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int get_wrdata (int ps_num, uint addr)
{
    int tmp;
    uint readval;
    char buffer[48], *c_ptr;

    while (1) {
        /*
         * display current value on local buffer
         */

        printf("0x%04x = %02x > ", addr, buf[addr]);

        readval = buf[addr];
        
        c_ptr = buffer;
        get_line(c_ptr,sizeof(buffer));
        switch(*c_ptr) {
        case 'x':
        case 'q':
            return(0);  /* quit */
        case ',':
        case 'p':       /* prev location */
            if (addr != 0) {
                addr -= 1;
            } else {
                printf("Start of PEM:%d EEPROM at 0x0000\n", ps_num);
            }
            break;
        case 'r':
        case '.':       /* same location */
            continue;
        case 0:
          	if (addr < (size-1)) {
                addr += 1;
            } else {
                printf("End of PEM:%d EEPROM at 0x%04x\n", ps_num, addr);
            }
            break;      /* next location */
        default:
            tmp = getnum(c_ptr,16,&readval);
            if(tmp == 0) {
                printf("bad value \"%s\"\n",c_ptr);
                continue;       /* same location again */
            } else {
                buf[addr] = readval;
                c_ptr += tmp;
                if (*c_ptr == '.')
                    continue;       /* same location */

                if (addr < (size-1)) {
                    addr += 1;
                } else {
                    printf("End of PEM:%d EEPROM at 0x%04x\n", ps_num, addr);
                }
            break;  /* next location */
            }

        }

    }
    return(PASSED);
}


/*******************************************************************************
 *
 * Function   : pem_init
 * Description: check PSU present and read contents of EEPROM into local buffer.
 * Inputs     : PSU number
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int pem_init (int ps_num)
{
    int unused = 0;
  
    max_size = PEM_EEPROM_SIZE;
    memset((uchar *)buf, 0, PEM_EEPROM_SIZE);

    printf("Reading PEM:%d EEPROM ... \n", ps_num);
    /* Read current contents of EEPROM into local buffer */
    if (rp1ruve_pem_read_eeprom(&buf[0], max_size, 0, unused, ps_num) != 0) {
        cterr('f', 0, "PEM:%d EEPROM Read failure!!!\n", ps_num);
        return (FAILED);
    }

    return (PASSED);
  
} /* init() */


/*******************************************************************************
 *
 * Function   : pem_modify
 * Description: modify the EEPROM contents of PSU.
 * Inputs     : PSU number
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int pem_modify (void)
{
    int unused = 0;
    uint addr, ps_num = psu_no_now;

    /* get the eeprom data and store into local buffer */
    if (pem_init(ps_num))
        return (FAILED);
    
    addr = gethex_answer("Start Address Location(hex): ", 0, 0, max_size);

    if (!get_wrdata(ps_num, addr)) {
        printf("Writing modified copy to PEM:%d EEPROM ... \n", ps_num);
        /* Write modified contents of local buffer to EEPROM */
        if (rp1ruve_pem_write_eeprom(&buf[0], max_size, unused, ps_num) != 0) {
            printf("Contents of EEPROM may not be reliable!");
            cterr('f', 0, "PEM:%d EEPROM Write failure!!!", ps_num);
        return (FAILED);
        } 
        printf("Done!\n");
    }
    
    return (PASSED);
  
} /* testFunc() */


/*******************************************************************************
 *
 * Function   : pem_eeprom_display
 * Description: display the EEPROM contents of PSU.
 * Inputs     : buffer - buffer of EEPROM contents
                display size - size of the buffer to display
 * Outputs    : None
 *
 *******************************************************************************
 */
void pem_eeprom_display(uchar *buf, int dis_size) 
{
/*
    int i;
*/
    idprom_print_all_fields(buf+IDPROM_TLV_OFFSET, 
			    dis_size-IDPROM_TLV_OFFSET);
    printf("\nID PROM format version %d", buf[0]);
    printf("\nID PROM contents (hex):");
    mcp_print_buffer_data_with_ascii( &buf[0], dis_size, 16, 1 );


/*
    for (i = 0; i < size; i++) {
        if (i%16 == 0)
        printf("\n\t  0x%04x:", i);
        printf(" %02x", buf[i]);
    }
    printf ("\n");
*/
    return;
}


/*******************************************************************************
 *
 * Function   : pem_eeprom_dflt_init
 * Description: Initialize Power Supply 0 or 1  EEPROM with default values from
 *              PCAMAP
 * Inputs     : power supply number
 *              EEPROM size
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int pem_eeprom_dflt_init (int psNum, int eeprom_size)
{
    int field_length, field_type, ia;
    ushort f_start,f_length;
    uchar *idprom = NULL;
    uchar rdbuf[PEM_EEPROM_SIZE];
    uint32 numBytes = 0;
    uchar dflt_sernum[S_IDPROM_PCB_SERIAL+1]; 
    char sernum[S_IDPROM_PCB_SERIAL+1]; 
    char prompt[80];
    uchar hw_version[2] = {0,0};
    uint32 hw_bld_num = 0x01;  /* HW Version Hi Byte = Build # */
    uint32 hw_rev_num = 0x00;  /* HW Version Lo Byte = Rev #   */
    uint32 ps_type = 0;
    uint32 write_flag = 0;
    int unused = 0;

    uchar *pem_idprom_ptr[4] = { pem1ru_ac_idprom_32k, pem1ru_dc_idprom_32k, pem1ru_ac_idprom, pem1ru_dc_idprom };
    char *pemIdpromStr[4]   = {"pem1ruve_ac_32k",    "pem1ruve_dc_32k",    "pem1ruve_ac",    "pem1ruve_dc" };

    /* This is P3 */
    if ( eeprom_size == PEM_EEPROM_SIZE ) {
        /* getIntValue("PS Type (0=1RUVE-AC, 1=1RUVE-DC)", 0, 1, 0, &ps_type); */
        ps_type = getdec_answer("PS Type (0=1RUVE-AC, 1=1RUVE-DC)", 0, 0, 1);
        idprom   = pem_idprom_ptr[ps_type];
        numBytes = PEM_EEPROM_SIZE;
    }
    /* This is P2/P1 */
    else {
        /* getIntValue("PS Type (0=1RUVE-AC_32k, 1=1RUVE-DC-32k, 2=1RUVE-AC_2k, 3=1RUVE-DC_2k)", 0, 3, 0, &ps_type); */
        ps_type = getdec_answer("PS Type (0=1RUVE-AC_32k, 1=1RUVE-DC-32k, 2=1RUVE-AC_2k, 3=1RUVE-DC_2k)", 0, 0, 3); 
        idprom   = pem_idprom_ptr[ps_type];
        numBytes = ( ps_type < 2 ) ? PEM_EEPROM_SIZE : PEM_EEPROM_SIZE_OLD ;
    }


    /* Read current contents of EEPROM into local buffer */
    printf("==== Reading current contents of PEM:%d EEPROM ====\n", psNum);
    memset(rdbuf, 0x00, sizeof(rdbuf));
    if (rp1ruve_pem_read_eeprom(rdbuf,numBytes,0,unused,psNum) != 0) {
        cterr('f', 0, "PEM:%d EEPROM Read failure!!!", psNum);
        return (FAILED);
    }

    /*
     * Get current Chassis S/N  
     */
    memset(dflt_sernum, 0, sizeof(dflt_sernum));
    memset(sernum, 0, sizeof(sernum));
    memset(prompt, 0, sizeof(prompt));
    idprom_get_field_data(&rdbuf[IDPROM_TLV_OFFSET],
			       (numBytes - IDPROM_TLV_OFFSET),
			       T_IDPROM_PCB_SERIAL, dflt_sernum, 
			       S_IDPROM_PCB_SERIAL, TRUE);
    
    /* 
    sprintf(prompt, "Serial Number [%s] ", dflt_sernum);
    getWord(prompt, &sernum[0], sizeof(sernum));
    */
    sprintf(prompt, "Serial Number [%s] ", dflt_sernum);
    printf("%s", prompt);
    get_line(&sernum[0],sizeof(sernum));

    if (strlen(sernum) == 0) { 
        /* Use default S/N */
        memcpy(sernum, dflt_sernum, S_IDPROM_CHASSIS_SERIAL);
    }

    /*
     * Get current hw version 
     */
    idprom_get_field_data(&rdbuf[IDPROM_TLV_OFFSET],
			       (numBytes - IDPROM_TLV_OFFSET),
			       T_IDPROM_HW_VERSION, hw_version, 2, TRUE);

    /* getIntValue("HW Version High Byte (Build #)", 
		0, 0xFF, hw_version[0], &hw_bld_num);   */
    hw_bld_num = getdec_answer("HW Version High Byte (Build #)", hw_version[0], 0, 0xFF); 

    /* getIntValue("HW Version Low Byte (Rev #)", 
		0, 0xFF, hw_version[1], &hw_rev_num);  */
	hw_rev_num = getdec_answer("HW Version Low Byte (Rev #)", hw_version[1], 0, 0xFF); 
    hw_version[0] = (uchar)hw_bld_num;
    hw_version[1] = (uchar)hw_rev_num;


    /* Display current contents just read from the EEPROM */
    printf("\n==== Current contents of PEM:%d EEPROM ====\n", psNum);
    pem_eeprom_display(rdbuf, numBytes);

    /* Update the contents with user entered values for S/N and HW Version.
     *
     * pcb serial number
     */
    field_length = 11;
    field_type = T_IDPROM_PCB_SERIAL;
    if (!idprom_update_field_data(&idprom[IDPROM_TLV_OFFSET],
				  (numBytes - IDPROM_TLV_OFFSET),
				  FALSE, field_type, field_length,
				  sernum, &f_start, &f_length)) {
        cterr('f', 0, "Unable to update idprom's serial num field data, "
		   "EEPROM has not been written!!!\n");
        return (FAILED);
    }
    /*
     * hw version 
     */
    field_length = 2;
    field_type = T_IDPROM_HW_VERSION;
    if (!idprom_update_field_data(&idprom[IDPROM_TLV_OFFSET],
				  (numBytes - IDPROM_TLV_OFFSET),
				  FALSE, field_type, field_length,
				  hw_version, &f_start, &f_length)) {
	cterr('f', 0, "Unable to update idprom's HW Version field data, "
		   "EEPROM has not been written!!!\n");
	return (FAILED);   
    }

    /* Display updated contents that will be written. */
    printf("\n==== Updated contents to be written to PEM:%d EEPROM ====\n",psNum);
    pem_eeprom_display(idprom, numBytes);
    
    /* getIntValue("Write updated contents to EEPROM? (0:NO, 1:YES)",
		0, 1, 0, &write_flag);  */
    write_flag = getdec_answer("Write updated contents to EEPROM? (0:NO, 1:YES)", 0, 0, 1); 

    if (write_flag == 0) {
        printf("\n==== PEM:%d EEPROM was NOT updated!!!\n\n", psNum);
        return (PASSED);
    }

    /*
     * Update the eeprom
     */
/* ////zzzzzz
    printf("==== Writing PEM:%d EEPROM with %d bytes from %s_idprom[] @0x%llX\n",
	      psNum, numBytes, pemIdpromStr[ps_type], idprom);
*/
    printf("==== Writing PEM:%d EEPROM with %d bytes from %s_idprom[] \n",
	      psNum, numBytes, pemIdpromStr[ps_type]);

    if (rp1ruve_pem_write_eeprom(idprom, numBytes, 
			      SCBY_HT_ACCESS, psNum) != 0) {
        printf("Contents of EEPROM may not be reliable!");
        cterr('f', 0, "PS:%d EEPROM Write failure!!!", psNum);
        return (FAILED);
    }
    /*
     * verify idprom
     */
    memset(rdbuf, 0x00, sizeof(rdbuf));
    printf("==== Reading & verifying new contents of PEM:%d EEPROM ====\n",psNum);
    if (rp1ruve_pem_read_eeprom(rdbuf,numBytes,0,unused,psNum) != 0) {
        printf("Data displayed may not be reliable!!");
        cterr('f', 0, "PEM:%d EEPROM Read failure!!!", psNum);
    }
    for (ia = 0; ia < numBytes; ia++) {
	if (rdbuf[ia] != idprom[ia]) {
	    cterr('f', 0, "Value read from EEPROM does not match data written\n"
		       "ERROR:  eeprom[%d] wr:0x%02x, rd:0x%08x", 
		       ia, idprom[ia], rdbuf[ia]);

	    printf(" write: \n\r" );
	    mcp_print_buffer_data_with_ascii( &idprom[0], numBytes, 16, 1 );
	    printf(" read: \n\r" );
	    mcp_print_buffer_data_with_ascii( &rdbuf[0], numBytes, 16, 1 );
	    
	    return(FAILED);
	}
    }

    return (PASSED);

} /* pem_eeprom_dflt_init() */



/*******************************************************************************
 *
 * Function   : pem_eeprom_dflt_init_cmd_f
 * Description: function to initialize Power Supply 0 or 1  EEPROM with default
 *              values
 * Inputs     : PSU number
 * Outputs    : None
 *
 *******************************************************************************
 */
void pem_eeprom_dflt_init_cmd_f (void)
{
    int cur_size = PEM_EEPROM_SIZE;
    int ps_num = psu_no_now;

    if (pem_eeprom_dflt_init(ps_num, cur_size) == FAILED) {
        cterr('f', 0, "PEM:$d eeprom init failed!\n", ps_num);
        return;
    }
    return;
}


/*******************************************************************************
 *
 * Function   : read_pem_eeprom
 * Description: read and display power supply eeprom contents
 * Inputs     : PSU number
 *              read size
 *              the address start to read
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int read_pem_eeprom (int ps_num, int rd_size, uint32 addr)
{
    uchar rdBuf[PEM_EEPROM_SIZE]={0};

    memset((uchar *)rdBuf, 0, PEM_EEPROM_SIZE);


    printf("\nReading PS%d EEPROM (%d bytes starting at 0x%04x ...\n",
              ps_num, rd_size, addr);

    if ( rp1ruve_pem_read_eeprom( &rdBuf[0], rd_size, addr, 0, ps_num) == PASSED ) {
        rp1ruve_pem_display_eeprom(&rdBuf[0], rd_size, ps_num);
        return PASSED;
    }
    else
        return FAILED;
}


/*******************************************************************************
 *
 * Function   : read_pem_eeprom_util
 * Description: utility to read PSU eeprom
 * Inputs     : PSU number
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int read_pem_eeprom_util (void)
{
    int rd_size = PEM_EEPROM_SIZE;
    int ps_num = psu_no_now; 

    return (read_pem_eeprom(ps_num, rd_size, 0));
}

/*******************************************************************************
 *
 * Function   : pem_get_hw_ver
 * Description: Get the version num from EEPROM
 *              Upper function have to pass in a 6 byte buffer and check the
 *              PS is present.
 * Inputs     : PSU number
 *              bufer to save the hw version information
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int pem_get_hw_ver( int ps_num, uint32 *buf1, uint32 *buf2, uint32 *buf3 ) {

    uchar rdBuf[PEM_EEPROM_SIZE]={0};
    uchar buf[6]={0};
    int rom_size = PEM_EEPROM_SIZE;

    *buf1 = 0;
    *buf2 = 0;
    *buf3 = 0;

    if ( rp1ruve_pem_read_eeprom( &rdBuf[0], rom_size, 0, 0, ps_num ))
        return FAILED;

    if (idprom_get_field_data( &rdBuf[IDPROM_TLV_OFFSET], (PEM_EEPROM_SIZE - IDPROM_TLV_OFFSET),
                               T_IDPROM_PCB_PARTNBR_6, &buf[0], 6, TRUE) == 0) {
        cterr('f', 0, "idprom_get_field_data returned error!!!");
        return FAILED;
    }

    *buf1 = (uint32)(( buf[0] << 8  ) | buf[1] );
    *buf2 = (uint32)(( buf[2] << 16 ) | ( buf[3] << 8 ) | buf[4] );
    *buf3 = (uint32)( buf[5] );

    return PASSED;
}


/*******************************************************************************
 *
 * Function   : pem_monitor_dump_wr
 * Description: wrapper for ucontroller dump
 * Inputs     : NONE
 * Outputs    : NONE
 *
 *******************************************************************************
 */
void pem_monitor_dump_wr (void) {

    pem_monitor_dump(psu_no_now);
    return;
}

/*******************************************************************************
 *
 * Function   : pem_monitor_dump_wr
 * Description: wrapper for ucontroller write.
 * Inputs     : NONE
 * Outputs    : NONE
 *
 *******************************************************************************
 */
void pem_monitor_write_wr (void) {

    pem_monitor_write(psu_no_now);
    return;
}

/*
 *------------------------------------------------------------------
 * $Log: platform_pem_eeprom_modify.c,v $
 * Revision 1.9  2017/07/10 02:27:51  leschen
 * Remove unused variable
 *
 * Revision 1.8  2013/11/26 08:40:38  hroni
 * fix compiler warning
 *
 * Revision 1.7  2013/09/11 02:25:08  alpeng
 * 1. support Juno fan info and display on initialize stage.
 * 2. support fedora rootfs
 *
 * Revision 1.6  2013/08/22 07:43:30  alpeng
 * support both MENU and CLI PSU cookie on Juno
 *
 * Revision 1.5  2013/08/14 06:01:35  alpeng
 * support PSU ucontroller write
 *
 * Revision 1.4  2013/08/13 07:19:30  alpeng
 * support i2c scan on PEM ucontroller, update the code for new PSU eeprom w/r and ucontroller read
 *
 * REvision 1.3  2013/07/16 08:02:08  alpeng
 * support i2c read and read cookie for juno psu
 *
 * Revision 1.2  2013/07/04 08:02:19  alpeng
 * fixed is_overlord() for latest FPGA rev.
 *
 * Revision 1.1  2013/05/31 12:43:15  danchung
 * Porting PSU source code from Nightster for Juno.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */


