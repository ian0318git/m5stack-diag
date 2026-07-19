/* $Id: menu.h,v 1.16 2021/06/02 07:42:56 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/menu.h,v $
 *------------------------------------------------------------------
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __MENU_H__
#define __MENU_H__

#define UNKNOWN_INTRS_CTERR_IF_Z
#define AUTHENTICATION_TEST_Y
/*
** Structure defines for the handling of menus.
*/

#ifdef UNKNOWN_INTRS_CTERR_IF_Z
/* Define bit in ulong diagflag_zram that determines whether to use cterr() or
 * printf() for handling unknown interrupts (OFF by default).
 */
#define D_USE_CTERR       0x00000001 
#endif /*UNKNOWN_INTRS_CTERR_IF_Z*/

#ifdef AUTHENTICATION_TEST_Y
/* Define bit in ulong diagflag_yram to turn on the authentication test for
 * MFG (OFF by default).
 */
#define D_AUTH_Y       0x00000001 
#endif /*AUTHENTICATION_TEST_Y*/
typedef struct title_buf_t_ {
   char title[80];
} title_buf_t;

typedef struct menuinfo menuinfo_t;

struct menuinfo {
  char *mtitle;          /* the menu title */
  type_t mtparam;           /* optional title parameter */
  type_t (*mtfunc)();       /* optional menu title function (may be NULL) */
  char *mprompt;         /* optional menu prompt (may be NULL for default) */
  int msize;             /* the number of items in the menu */
  struct mitem *miptr;   /* pointer to array of menu item structures */
};

typedef struct mitem mitem_t;

struct mitem {
  char *mline;     /* the menu item display line OR */
  type_t (*mlfunc)();/* the menu item display line function */
  char *mlparam;   /* optional menu line parameter */
  type_t (*mfunc)(); /* the function invoked for this menu item */
  type_t *mfparam;   /* optional function parameter !!! ZZZ: has to be long !!!*/ 
  int mflag;             /* function flag (see below) */
  type_t (*mixfunc)();      /* the menu item exists function */
  type_t mixparam;          /* function parameter */
};

typedef struct subitem_t_ {
    char *itemstr;
    type_t (*itemfcn)();
    type_t *itemparam;
    int itemflag;
} subitem_t;

/*
 * subitem_x_t extends subitem_t with the menu item existence function and
 * its parameter.
 */
typedef struct subitem_x_t_ {
    char *itemstr;
    type_t (*itemfcn)();
    type_t *itemparam;
    int itemflag;
    type_t (*xitemfunc)();      /* the menu item exists function */
    type_t xitemparam;          /* function parameter */
} subitem_x_t;

typedef struct menu_item_ {
    char  *name;                 /* name of item in sub menu */
    PFT   diag;                  /* the diagnostic function for the submenu */
    long   flag;
} menu_item_t;

/*
 * The xtable struct is designed for table-driven generation of
 * both the primary and secondary (sub)menus by a pair of
 * complementary functions that operate on such a table.
 */
typedef struct submenu_xtable_t_ {
    char *x_title;        /* title of the item */
    type_t  (*x_pfunc)();    /* primary diag function */
    type_t  x_pparam;        /* primary function parameter (N.B., int, not int*) ZZZ has to be long!! */
    int  x_flags;         /* menu item flags */
    type_t (*x_xfunc)();    /* item boolean existence function */
    type_t  x_xparam;        /* existence function parameter */
    type_t  (*x_sfunc)();    /* secondary diag function (if any) */
    type_t  x_sparam;        /* secondary function parameter (not int*) ZZZ has to be long */
} submenu_xtable_t;

typedef struct tests_submenu_t_ {
  char  *name;             /* sub menu test name          */ 
  PFT    diag;             /* sub menu test to be called  */
  type_t    parm;             /* parameter to be passed      */
  int    flag;             /* flags that can be used      */
} test_submenu_t;

/* defines for mflag above */
#define MF_CONTINUOUS    0x01    /* menu item may be run continuously */
#define MF_DOALL         0x02    /* included in doall and dogrp */
#define MF_NOTNET        0x04    /* menu item not allowed when net boot */
#define MF_SHOW_ERRCOUNT 0x08    /* (submenu) item, also part of larger diag */
#define MF_MULTI         0x10    /* menu item capable of multitasking */
#define MF_IS_SLOT_MULTI 0x20    /* does corresponding slot multitask? */
#define MF_IS_HWIC_MULTI 0x40    /* does this HWIC multitask? */
#define MF_DOGRP         0x80    /* included in dogrp, but not doall */
#define MF_DESTRUCTIVE   0x100   /* Destructive Test */
#define MF_HIDDEN_EXE    0x200   /* menu item should be hidden but still can be execuated */

#define MF_DEF_FINITE_CONT_NUM  (0)

#define MAX_BASE_ITEMS     10    /* alt flags, do all, ..., select multi, ... */
#define MAX_LIST_CHAR     160    /* approx. two lines' worth of user input */
#define MAX_TEST_ITEM     100

/* define for set_diagflag() */
#define WRITE_DIAGFLAG    0
#define READ_DIAGFLAG     1
#define DIAGFLAG_LOG_STR    "diagflag.log"
#define XDIAGFLAG_LOG_STR   "xdiagflag.log"
#define YDIAGFLAG_LOG_STR   "ydiagflag.log"
#define ZDIAGFLAG_LOG_STR   "zdiagflag.log"


typedef char menu_index_token_t[3]; 

#ifdef UNKNOWN_INTRS_CTERR_IF_Z
extern ulong diagflag_zram;   /* ram global for special Z flag */
#endif /*UNKNOWN_INTRS_CTERR_IF_Z*/
#ifdef AUTHENTICATION_TEST_Y
extern ulong diagflag_yram;   /* ram global for special Y flag */
#endif /*AUTHENTICATION_TEST_Y*/
extern char dgmenustr[];
extern char adiagfstr[];
extern char aparamstr[];
extern char basutilstr[];
extern char doalldgstr[];
extern char dogrpdgstr[];
extern char doselmulstr[];
extern char dostamulstr[];
extern char dosetcontostr[];
extern char regtststr[];
extern char inttststr[];
extern char endnote_buf[];   /* Notes for secondary menu */
extern int num_post_item_base;
extern subitem_x_t post_item_base[];
extern int base_submenu_item_total; /* final num of base submenu item */
extern void (*pre_diag_exec)(void);
extern void (*menu_display_real_time)(void);
extern struct menuinfo *utilmenup;

extern void set_diagflag(unsigned long*, char *, boolean);
extern boolean check_menu_flag(uint);
extern boolean slots_polled_for_display;
extern boolean platform_does_multitasking;   /* assigned by platform */
extern boolean does_platform_multitask(void);
extern boolean does_nm_multitask(int slotnum);
extern boolean does_hwic_multitask(int slotnum);
extern boolean does_item_multitask(mitem_t *miptr);
extern void menu(menuinfo_t  *menuptr, mitem_t *second_miptr, char menucmd);
extern void displaymenu(struct menuinfo *menuptr, int mflag);
extern int do_second_item(mitem_t *secptr, int index);
extern int show_endnote(void);
extern void init_empty_menu(menuinfo_t *, type_t mtparam);
extern void init_base_submenu(menuinfo_t **addr_submenup, type_t mtparam);
extern void add_menu_item(menuinfo_t *, char *item_descrip,
			  type_t (*mfunc)(), type_t *mfparam, int mflag);
extern void menu_show_dflags(void);
extern void menu_show_all_dflags(void);  /* for alter flags menu only */
extern void menu_pr_err_accum(void);
extern boolean menu_exec_doall_diags(struct menuinfo *menup);
extern boolean exec_doall_menu_items(struct menuinfo *menup);
extern int  do_menu_all_diags(struct menuinfo *menup);
extern int  do_menu_grp_diags(struct menuinfo *menup);
extern int  do_menu_set_up_multi(struct menuinfo *menup);
extern int  do_menu_select_multi(struct menuinfo *menup);
extern int  do_menu_start_multi(int *null_param);
extern int  do_menu_set_multi_timeout(int *null_param);
extern int  exec_submenu_items(menuinfo_t *menu_p);
extern void build_primary_submenu(submenu_xtable_t *px, int size, char *title,
                             menuinfo_t **addr_submenup);
extern void build_secondary_submenu(submenu_xtable_t *px, int size,
                                    mitem_t *pm);
extern boolean menu_item_reserved(void);

/*
 * The following items are defined in the diag.c file of some
 * platforms.
 */
extern int doalldiags(register struct menuinfo *menup);
extern int dogrpdiags(register struct menuinfo *menup);
extern int showdflags(void);

extern struct menuinfo *menu_diagflagp;
extern menuinfo_t pm_subtest_menu;
extern menuinfo_t *pm_submenup;
extern long zero;
extern long one;
extern long two;
extern long three;
extern long four;
extern long five;
extern long six;
extern long seven;

/* Extern */
extern int do_all_menu_items(struct menuinfo *);

#endif /* __MENU_H__ */
/* end of module */

/******** History ******** 
$Log: menu.h,v $
Revision 1.16  2021/06/02 07:42:56  iachang
CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk

Revision 1.15  2019/07/11 12:34:40  alicehua
Collapse Nutella codes into main trunk

Revision 1.14.2.1  2019/04/01 09:38:07  harrchan
Add finite continuous feature

Revision 1.14  2019/01/10 06:32:37  wilbhuan
Made function "do_all_menu_items" as an external function.

Revision 1.13  2018/05/18 09:24:48  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.12.24.1  2017/01/09 00:49:49  ptong
Print the skipped plugin list after mb_tests menu title to warn user on Neptune diag

Revision 1.12  2015/01/14 08:45:38  danchung
add a new menu item flag to be an attribute to make the menu item hidden
and still executable

Revision 1.11  2013/11/26 08:40:33  hroni
fix compiler warning

Revision 1.10  2013/11/13 11:46:23  hroni
add four for extern use

Revision 1.9  2013/11/11 21:18:38  mcharon
pass string instead of number in first argum of host_send_packet ; add xaui support

Revision 1.8  2013/10/08 11:03:47  erwu2
enhanced err msg first check-in

Revision 1.7  2013/06/24 08:39:24  alpeng
support Ext. loopback flag checking before USB/CF diag

Revision 1.6  2012/05/11 23:28:28  ptong
Add macros for diagflag.log

Revision 1.5  2012/04/27 10:42:42  alpeng
fixed minor bugs and support set external loopback flag for controlling test flow

Revision 1.4  2012/04/11 01:01:05  alpeng
create file for storing diagflag, right after entering diag

Revision 1.3  2012/04/10 09:41:16  alpeng
support CLI cmd:disflag, setflag, repeat and history

Revision 1.2  2012/03/28 00:38:11  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:04  ptong
Initial archive of ng_diag module


$Endlog$
*/
