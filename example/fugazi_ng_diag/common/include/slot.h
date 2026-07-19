/* $Id: slot.h,v 1.24 2021/06/28 11:48:06 xiaolaya Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/slot.h,v $
 *------------------------------------------------------------------
 * slot.h - Defines for the port module support routines
 *
 * Copyright (c) 2014-2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __SLOT_H__
#define __SLOT_H__

#define MAX_SUBTEST_ITEMS 75

#define SLOT_VACCODE      0xfe     /* slot is vacant */
#define SLOT_ILLCODE      0xfd     /* illegal module type code */

#define WIC_TYPE          1
#define VPM_VIC_TYPE      2
#define DAUGHTER_CARD_TYPE 2

#ifndef COOKIE_SIZE_512
#define  COOKIE_SIZE_512     512
#endif

/* Flags for providing specific info for the module
 */
#define MOD_INFO_NULL            0x0
#define MOD_INFO_USE_PCIE        0x1
#define MOD_INFO_DC_IS_NIM       0x2
#define MOD_INFO_DC_IS_VM        0x4   /* using bit assigment instead integer */
#define MOD_INFO_SM_IS_CARRIER   0x8   /* used for Switzer-Carrier */

/*
 * Common function to all platform, to be used by those
 * platforms that require pci bus speed to be changed.
 */

/*
** This structure is used to map the module id to the name and
** diagnostic routine.
*/
struct module_info {
    char   *name;                /* module name */
    PFT    diag;          /* the diagnostic for this module */
    ushort id;                   /* the port module id */
    PFT    intf_diag;             /* the pci test for this module */
    ushort  early_unreset;    /* set this flag if host should unreset module immediately
                                after power up per pci spec; otherwise, module code is
                                reponsible for unresetting the module.
                                useful in case some hardware needs to be init before
                                module comes out of reset.*/
    uint mod_info_flags; /* These flags provide more info of the
			    module */

};

//typedef struct ngio_intf ngio_intf_t;

typedef struct ngio_intf_t {

    char name[48];      /* module name */
    int    slot;                 /* HWIC Slot Number on Platform        */
    unsigned short id;           /* HWIC Cookie id                      */
    int    menu_display;         /* Flag set if submenu is invoked      */
    int    test_type;            /* Full test or HWIC interface test 
				    Initialize with FULL_TEST or IFACE_TEST*/
    boolean cli;                 /* CLI parameter mode */
    unsigned int mod_type;       /* MOTHER_BOARD,WIC_DAUGHTER_CARD, etc in cross_platform.h */
        
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
    void (*pci_rdy)(void *, int);
    unsigned short (*get_id)(void *, char *);
    uchar cookie[COOKIE_SIZE_512];/* cookie */
    uint8_t       i2c_ctrl;               /*i2c ctrl number */
    uint8_t       uart_ctrl;              /*i2c ctrl number */
    PFT    diag;                          /* main test of this module */
    PFT    intf_diag;                     /* intface test of this module */
    void *oir;                            /* oir i2c device */
    void *pca;                            /* io expander PCA device */
    struct ngio_intf_t *dc;               /* poitner to daughter card */
    struct ngio_intf_t *pc;               /* parent card */
    void *priv;                           /* ptr to module private data struct */

} ngio_if;

typedef struct daughtercard_info {
     ushort id;                  /* interface id */ 
     char   *name;               /* interface name */
     char   ser_num[12];         /* the daughtercard serial number */
     unsigned short revision;            /* Board revision */
} daughtercard_info_t;


/* external prototypes */
extern int  slot_notavail(void);
extern int  get_real_slot(int);
extern int  testslot(int);
extern void poll_slots(void);
extern int  slot_notavail(void);
extern int get_max_sm_slots(void);
extern void slot_init_info(void);
extern int sm_test(int slot);
extern int wic_test(int slot);
extern type_t sm_iface_test(void);
extern type_t wic_iface_test(void);
extern type_t vm_iface_test(void);
extern int enable_sm(int);
extern int enable_wic(int);
extern void disable_sm(int);
extern void disable_wic(int);
extern struct ngio_intf_t *slot_get_ngiowic(int slot);
extern struct ngio_intf_t *slot_get_ngiosm(int slot);
extern struct ngio_intf_t *slot_get_ngiovm(int slot);
extern struct ngio_intf_t *slot_get_ngioisp(int slot);
extern int slot_i2c_unreset (struct ngio_intf_t *ngio, int, char *);
extern void init_slot_info(void);
extern void slot_clear_cookie_id(int type, int slot_num);
extern void slot_init_dc(struct ngio_intf_t *, unsigned int);
extern void ngio_ge_cfg(struct ngio_intf_t *);
extern int carrier_wic_test(void *, int, int, int);
extern void init_carrier_wic(struct ngio_intf_t *, int);
extern void reset_errmsg_var(void);
extern int get_slot_bd_pid (uchar *, char *);

#define BOARD_SHARE_ID  "board_share_id"

//http://www.kroah.com/log/linux/container_of.html
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

#define container_of(ptr, type, member) ({            \
 const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
 (type *)( (char *)__mptr - offsetof(type,member) );})

#define CONT(prt, type, mem) container_of((prt), type, mem)

#endif  /* __SLOT_H__ */

/******** History ******** 
$Log: slot.h,v $
Revision 1.24  2021/06/28 11:48:06  xiaolaya
fix bug for daughter card slot info polling

Revision 1.23  2021/04/12 13:36:05  xiaolaya
*** empty log message ***

Revision 1.22  2017/10/18 01:51:19  olin2
Improve common code for Poll slot (CSCvg14997)

Revision 1.21  2016/04/20 07:03:34  benchen2
merge tachi_branch to maintrunk

Revision 1.20.2.1  2016/04/18 07:00:47  benchen2
according to prrq fix isp define

Revision 1.20  2015/06/05 06:13:11  alpeng
fix poll slot issue

Revision 1.19  2014/07/03 07:18:34  erwu2
Reset Enhanced Error Message Between Slot Tests

Revision 1.18  2014/07/02 08:09:42  alpeng
add new testcard id for en/disable menu item and select smi addr

Revision 1.17  2014/06/24 22:04:50  ptong
Move ngio_ge_cfg from slot_get_info to slot_test to config 10KR ports

Revision 1.16  2014/05/06 08:10:21  danchung
Remove useless function

Revision 1.15  2014/04/28 11:33:56  danchung
Add related functions for Greyhound 10G-KR bring-up

Revision 1.14  2014/03/25 01:07:00  ptong
Added mod_info_flags to struc module_info

Revision 1.13  2014/03/03 22:27:45  mcharon
move daughter card init code to fortitude

Revision 1.12  2014/01/29 01:27:55  mcharon
clear cookie id when insertion/removel event is dectected

Revision 1.11  2013/11/26 08:40:33  hroni
fix compiler warning

Revision 1.10  2013/10/07 03:40:13  alpeng
introducing a flag early_unreset for ngio to put reset state on early stage

Revision 1.9  2013/03/29 05:49:32  mcharon
add function ptr pci_rdy to turn on pci ready by

Revision 1.8  2012/11/17 01:15:16  mcharon
reset i2c device; don't cterr in driver code..propogate err message to slot.c

Revision 1.7  2012/11/12 20:35:22  mcharon
add third arg to slot_i2c_unrest to report slot num when fails..improve err reporting

Revision 1.6  2012/09/13 17:15:17  mcharon
add vm_interface_test

Revision 1.5  2012/06/04 10:35:02  palin2
Clean up compiler warnings.

Revision 1.4  2012/05/05 04:02:20  mcharon
support alter daughter board cookie for wic

Revision 1.3  2012/05/04 20:01:45  mcharon
use void* instead of int as argument to to func ptrs in ngio_intf

Revision 1.2  2012/03/28 00:38:12  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:04  ptong
Initial archive of ng_diag module


$Endlog$
*/
