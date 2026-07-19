/* $Id: menu.h,v 1.2 2017/07/28 07:58:37 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/ppb/inc/menu.h,v $
 *------------------------------------------------------------------
 * menu.h
 *
 * Copyright (c) 2011-2017 by Cisco Systems, Inc.
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
#endif //UNKNOWN_INTRS_CTERR_IF_Z

#ifdef AUTHENTICATION_TEST_Y
/* Define bit in ulong diagflag_yram to turn on the authentication test for
 * MFG (OFF by default).
 */
#define D_AUTH_Y       0x00000001 
#endif //AUTHENTICATION_TEST_Y

typedef struct menu_disp_t {
    char name[120];
} menu_disp;

typedef struct menuinfo menuinfo_t;

struct menuinfo {
  char *mtitle;          /* the menu title */
  type_t mtparam;           /* optional title parameter */
  type_t (*mtfunc)(int, ...);       /* optional menu title function (may be NULL) */
  char *mprompt;         /* optional menu prompt (may be NULL for default) */
  int msize;             /* the number of items in the menu */
  struct mitem *miptr;   /* pointer to array of menu item structures */
};

typedef struct mitem mitem_t;

struct mitem {
  char *mline;     /* the menu item display line OR */
  type_t (*mlfunc)(int, ...); /* the menu item display line function */
  char *mlparam;   /* optional menu line parameter */
  type_t (*mfunc)(int, ...); /* the function invoked for this menu item */
  type_t *mfparam;   /* optional function parameter !!! ZZZ: has to be long !!!*/ 
  int mflag;             /* function flag (see below) */
  type_t (*mixfunc)(int, ...);      /* the menu item exists function */
  type_t mixparam;          /* function parameter */
};

typedef struct subitem_t_ {
    char *itemstr;
    type_t (*itemfcn)(int, ...);
    type_t *itemparam;
    int itemflag;
} subitem_t;

/*
 * subitem_x_t extends subitem_t with the menu item existence function and
 * its parameter.
 */
typedef struct subitem_x_t_ {
    char *itemstr;
    type_t (*itemfcn)(int, ...);
    type_t *itemparam;
    int itemflag;
    type_t (*xitemfunc)(int, ...);      /* the menu item exists function */
    type_t xitemparam;          /* function parameter */
} subitem_x_t;

typedef struct menu_item_ {
    char  *name;                 /* name of item in sub menu */
    PFT   diag;                  /* the diagnostic function for the submenu */
    int   flag;
} menu_item_t;

/*
 * The xtable struct is designed for table-driven generation of
 * both the primary and secondary (sub)menus by a pair of
 * complementary functions that operate on such a table.
 */
typedef struct submenu_xtable_t_ {
    char *x_title;        /* title of the item */
    type_t  (*x_pfunc)(int, ...);    /* primary diag function */
    type_t  x_pparam;        /* primary function parameter (N.B., int, not int*) ZZZ has to be long!! */
    int  x_flags;         /* menu item flags */
    type_t (*x_xfunc)(int, ...);    /* item boolean existence function */
    type_t  x_xparam;        /* existence function parameter */
    type_t  (*x_sfunc)(int, ...);    /* secondary diag function (if any) */
    type_t  x_sparam;        /* secondary function parameter (not int*) ZZZ has to be long */
} submenu_xtable_t;

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

#define MAX_BASE_ITEMS     10    /* alt flags, do all, ..., select multi, ... */
#define MAX_LIST_CHAR     160    /* approx. two lines' worth of user input */
#define MAX_TEST_ITEM     100

typedef char menu_index_token_t[3]; 

#ifdef UNKNOWN_INTRS_CTERR_IF_Z
extern ulong diagflag_zram;   /* ram global for special Z flag */
#endif //UNKNOWN_INTRS_CTERR_IF_Z
#ifdef AUTHENTICATION_TEST_Y
extern ulong diagflag_yram;   /* ram global for special Y flag */
#endif //AUTHENTICATION_TEST_Y
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
extern void (*pre_diag_exec)(void);
extern void (*menu_display_real_time)(void);
extern struct menuinfo *utilmenup;

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
			  type_t (*mfunc)(void), type_t *mfparam, int mflag);
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

#ifndef LINUX_APP
//extern struct menuinfo *maindiagp;
#endif

#endif /* __MENU_H__ */
/* end of module */

/******** History ******** 
$Log: menu.h,v $
Revision 1.2  2017/07/28 07:58:37  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.1.2.1  2017/06/29 08:14:32  harrchan
Initial commit code for Oakenshield

Revision 1.3  2012/07/17 20:34:28  srane
cleanup

Revision 1.2  2012/05/24 23:22:38  srane
Add UART test (for ethernet mode).

Revision 1.1  2012/04/18 09:50:18  srane
Initial checkin


$Endlog$
*/

