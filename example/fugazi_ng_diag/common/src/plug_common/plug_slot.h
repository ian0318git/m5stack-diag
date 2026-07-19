/* $Id: plug_slot.h,v 1.5 2019/11/25 08:55:51 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/plug_common/plug_slot.h,v $
 *------------------------------------------------------------------
 *
 * plug_slot.h - Header file for Slot related functions
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#ifndef __PLUG_SLOT__
#define __PLUG_SLOT__

#define PLUG_PWR_EN_DELAY                               (30)
#define PLUG_PWR_AFTER_EN_DELAY                         (600)
#define PLUG_PWR_OFF_DELAY                              (1000)
#define PLUG_PWR_OK_TIMEOUT                             (100)

#define PLUG_MOD_STR                                    "PLUGGABLE"

#define PLUG_SLOT_VAC_ID                                (0xFFFE)
#define PLUG_SLOT_INVALID_ID                            (0xFFFF)

#define PLUG_SLOT_READ_ID_RETRY                         (3)
#define PLUG_SLOT_READ_ID_DELAY_IN_SEC                  (2)

#define PLUG_RESET_DELAY                                (200)

/* Slot 1 - Pluggable FPGA I2C Ctrl 0/UART Ctrl 0
 * Slot 2 - Pluggable FPGA I2C Ctrl 1/UART Ctrl 1
 */
#define PLUG_UART_CTRL(slot)                            (slot - 1)

#define PLUG_COOKIE_SIZE_512                                 (512)


typedef struct plug_intf_t {
    char name[48];               /* module name */
    int    slot;                 /* PLUG Slot Number on Platform        */
    unsigned short id;           /* PLUG Cookie id                      */
    int    menu_display;         /* Flag set if submenu is invoked      */
    int    test_type;            /* Full test or HWIC interface test 
				    Initialize with FULL_TEST or IFACE_TEST*/
    boolean cli;                 /* CLI parameter mode */
    /* unsigned int mod_type; */       /* MOTHER_BOARD,WIC_DAUGHTER_CARD, etc in cross_platform.h */
        
    char serial_num[20];
    unsigned short bd_rev;        /* board revision */

    int (*is_present)(void *);
    int (*on)(void *);
    void (*off)(void *);
    int (*i2c_reset)(void *);
    int (*i2c_unreset)(void *);
    int (*reset)(void *);
    int (*unreset)(void *);
    int (*uart_on)(void *);
    int (*uart_off)(void *);
    int (*get_id)(void *, char *);
    uchar cookie[PLUG_COOKIE_SIZE_512];        /* cookie */
    uint8_t       i2c_ctrl;               /*i2c ctrl number */
    uint8_t       uart_ctrl;              /*i2c ctrl number */
    PFT    diag;                          /* main test of this module */
    PFT    intf_diag;                     /* intface test of this module */
    void *priv;                           /* ptr to module private data struct */
} plug_if;

typedef enum {
    PLUG_TEST_CARD,
    PLUG_LTE,
    PLUG_SERIAL
} plug_slot_type;

extern void init_plug_info(void);
extern void plug_module_power_off(int);
extern int plug_test(int);
extern int plug_intf_test(int);
extern struct plug_intf_t *slot_get_plugslot(int);
extern int plug_slot_i2c_poweron_unreset(struct plug_intf_t *, int, char *);
extern int plug_slot_reset(void *);
extern int plug_slot_unreset(void *);
extern int plug_slot_i2c_reset(void *);
extern int plug_slot_i2c_unreset(void *);
extern int plug_is_pwr_ok(void *);
extern unsigned short plug_cookie_get(int);
extern int get_max_pim_slots(void);
#endif

/*-------------------------------------------------
$Log: plug_slot.h,v $
Revision 1.5  2019/11/25 08:55:51  kehuang2
Collapse Tabei-L into main trunk

Revision 1.4  2019/08/06 06:56:16  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.3  2018/11/23 09:02:32  hondwang
Re-instruct pluggable common code with CDETs CSCvn17216

Revision 1.2.62.1  2018/10/15 06:50:18  hondwang
pluggable common code re-instruct modify code

Revision 1.2  2018/01/20 04:53:29  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.1.4.5  2017/11/13 09:05:49  hondwang
Add pluggable slot cookie info with Diag login

Revision 1.1.4.4  2017/09/21 19:30:14  hondwang
Poweroff pluggable module before testing

Revision 1.1.4.3  2017/08/28 07:53:50  shjung
Added pluggable I/O interface test

Revision 1.1.4.2  2017/08/08 07:40:40  hondwang
add pluggable for star-branch-c9xx

Revision 1.1.2.5  2017/08/02 05:40:41  lucywang
add LED utility of Pluggable Serial

Revision 1.1.2.4  2017/07/31 10:49:58  lucywang
add pluggable serial code of host and module

Revision 1.1.2.3  2017/07/25 03:49:47  hondwang
pluggable testcard continuous testing need power off delay

Revision 1.1.2.2  2017/07/20 12:52:38  hondwang
Add module reset pin testing and fix test card not unreset issue

Revision 1.1.2.1  2017/07/13 06:32:19  tirawan
Reorganize Star Pluggable directory structure

Revision 1.1.2.4  2017/06/22 19:27:11  tirawan
Add LTE Test items and add log section at the bottom of the code


*/

