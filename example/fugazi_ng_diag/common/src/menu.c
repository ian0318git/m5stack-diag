/* $Id: menu.c,v 1.26 2021/06/02 07:43:01 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/menu.c,v $
 *------------------------------------------------------------------
 *
 * Author: Ben chen. 
 * Copyright (c) 2007-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

/*
** Routines for displaying menus and allowing the user to select
** menu items.
*/
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <setjmp.h>
#include <signal.h> /* interrupt */
#include <stdlib.h> /* interrupt */

#include "setjmps.h"
#include "signals.h"
#include "common.h"
#include "types.h"
#include "menu.h"
#include "pcmap.h"
#include "nvsysvars.h"
#include "error.h"
#include "monitor.h"
#include "proto.h"
#include "queryflags.h"
#include "dev_print.h"

#undef  MENU_DEBUG

extern int read_real_time(void);
extern void flush_test_progress_buf(void);

#ifdef UNKNOWN_INTRS_CTERR_IF_Z
ulong  diagflag_zram;   /* ram global for special Z flag */
#endif /* UNKNOWN_INTRS_CTERR_IF_Z */

#ifdef AUTHENTICATION_TEST_Y
ulong  diagflag_yram;   /* ram global for special Y flag */
#endif /* AUTHENTICATION_TEST_Y */

/* often used menu strings */
char  dgmenustr[] = "Diagnostic Menu";
char  adiagfstr[] = "alter diag flags";
char  aparamstr[] = "alter test parameters";
char basutilstr[] = "basic utilities";
char doalldgstr[] = "do all diags in this menu";
char dogrpdgstr[] = "do group of diags in this menu";
char  regtststr[] = "register test";
char  inttststr[] = "interrupt test";
 
static char togglestr[]      = "toggle \"%s\" flag";
static char contstr[]        = "Continuous";
static char stopstr[]        = "Stop on error";
static char ext_loopbk_str[] = "Ext. loopback";
static char ge_int_loopbk_str[] = "ge-Int loopback";
static char abbr_test_str[]  = "Abbr. test";
static char loopstr[]        = "Loop on error";
static char warning_str[]    = "Warning";
static char verbose_str[]    = "Verbose mode";
static char trace_str[]      = "Trace mode";
static char power_on_str[]   =  "ngio Pwr after test";
static char mintest_str[]    = "Min test time";
static char option_str[]     = "Optional output";
static char xec_auth_str[]   = "eXec authentication";
static char permu_str[]      = "permUtation test";
static char debug_str[]      = "Debug option";
static char ext_cust_str[]   = "exterNal customer";

static long contmask = D_CONTINUOUS; 
static long stopmask = D_STOPONERR;
static long ext_loopback_mask = D_EXT_LOOPBACK;
static long ge_int_loopback_mask = D_GE_INT_LOOPBACK;
static long abbr_test_mask = D_ABBR_TEST;
static long loopmask = D_LOOPONERR;
static long warning_mask = D_WARNING; 
static long verbose_mask = D_VERBOSE;
static long trace_mask = D_TRACE; 
static long mintest_mask = D_MIN_TEST_TIME; 
static long option_mask = D_SET_OPTIONS;
static long xecauth_mask = D_XEC_AUTH; 
static long permu_mask = D_PERMU_TEST;
static long debug_mask = D_DEBUG_OPTIONS;
static long show_power_on_mask = D_POWER_ON;
static long ext_cust_mask = D_EXT_CUSTOMER;

static unsigned int finite_cont_num = MF_DEF_FINITE_CONT_NUM; 
static unsigned int finite_cont_cntr;

static void do_permu_diags (menuinfo_t *menup, int *test_idx, int test_count);
static int test_stop;

static void menu_config_finite_cont_number(void);
/*
 * Forward prototypes
 */
void toggle_destructive_xflag(int);
void menu_togglediagflag(int flagmask);
void menu_togglexflag(int xflagmask);
void display_uut_default(print_fn_t print_fn);
static int get_menuindex_token (char *, menu_index_token_t, boolean);
static int get_menu_index (char *inbuf);
static void sig_handler(int); 
static void register_intr(void);

    

//extern int setjmp(jmp_buf);


static struct mitem diagflagitems[] = {
  {togglestr, 0, (char *)contstr, (PFT)menu_togglediagflag, (type_t *)&contmask,  
   0, (type_t(*)())0, 0},
  {togglestr,0, (char *)stopstr, (PFT)menu_togglediagflag,  (type_t *)&stopmask,
   0, (type_t(*)())0, 0},
  {togglestr,0, (char *)ext_loopbk_str, (PFT)menu_togglediagflag, (type_t *)&ext_loopback_mask,
   0, (type_t(*)())0, 0},
  {togglestr,0, (char *)ge_int_loopbk_str, (PFT)menu_togglexflag, (type_t *)&ge_int_loopback_mask,
   0, (type_t(*)())0, 0},
  {togglestr,0, (char *)abbr_test_str,  (PFT)menu_togglediagflag, (type_t *)&abbr_test_mask,
   0, (type_t(*)())0, 0},
  {togglestr,0, (char *)loopstr,      (PFT)menu_togglediagflag, (type_t *)&loopmask,
   0, (type_t(*)())0, 0},
  {togglestr,0, (char *)warning_str,  (PFT)menu_togglexflag, (type_t *)&warning_mask,
   0, (type_t(*)())0, 0},
  {togglestr,0, (char *)trace_str,    (PFT)menu_togglexflag, (type_t *)&trace_mask, 
   0, (type_t(*)())0, 0},
  {togglestr,0, (char *)verbose_str,  (PFT)menu_togglediagflag, (type_t *)&verbose_mask,
   0, (type_t(*)())0, 0},
  {togglestr,0, (char *)power_on_str,(PFT)menu_togglexflag, (type_t *)&show_power_on_mask,
   0, (type_t(*)())does_platform_multitask, 0},
  {togglestr,0, (char *)mintest_str,  (PFT)menu_togglexflag, (type_t *)&mintest_mask,
   0, (type_t(*)())0, 0},
  {togglestr,0, (char *)option_str,   (PFT)menu_togglexflag, (type_t *)&option_mask,
   0, (type_t(*)())0, 0},
  {togglestr, 0, (char *)xec_auth_str,  (PFT)menu_togglexflag, (type_t *)&xecauth_mask,
   0, (type_t(*)())0, 0},
  {togglestr, 0, (char *)permu_str,    (PFT)menu_togglexflag, (type_t *)&permu_mask,
   0, (type_t(*)())0, 0},
  {togglestr, 0, (char *)debug_str,     (PFT)menu_togglexflag, (type_t *)&debug_mask,
   0, (type_t(*)())0, 0},
  {togglestr, 0, (char *)power_on_str,     (PFT)menu_togglediagflag, (type_t *)&show_power_on_mask,
   0, (type_t(*)())0, 0},
  {togglestr, 0, (char *)ext_cust_str,     (PFT)toggle_destructive_xflag, 
  (type_t *)&ext_cust_mask,   0, (type_t(*)())0, 0},
};

static struct menuinfo menu_diagflag = {
  "Diagnostic Flag Menu",                      /* title */
  0,                                           /* no title param */
  (PFT)menu_show_all_dflags,                   /* all only in this menu */
  "enter item to toggle > ",                   /* our special prompt */
  sizeof(diagflagitems)/sizeof(struct mitem),  /* size of menu */
  diagflagitems,
};
struct menuinfo *menu_diagflagp = &menu_diagflag;

extern int netflashbooted;
boolean slots_polled_for_display;
int async_select;
#define EXIT_CHAR '\033'
#define DOUBLE_CHAR_INCREMENT 1000
#define MAX_POST_BASE 8

long zero = 0;
long one = 1;
long two   = 2;
long three = 3;
long four = 4;
long five = 5;
long six = 6;
long seven = 7;



/*
 * Following globals can be assigned during the init of a particular
 * platform.  If init_base_submenu() finds num_post_item_base to be
 * nonzero (originally zeroed), the corresponding menu items in
 * post_item_base[] are added to the end of the given menu's standard
 * base items.
 */
int num_post_item_base = 0;
subitem_x_t post_item_base[MAX_POST_BASE];
int base_submenu_item_total = 0; /* final num of base submenu item */
boolean platform_does_multitasking = FALSE;

void (*pre_diag_exec)(void) = NULL;  /* if assigned, called before menu item */
void (*menu_display_real_time)(void) = NULL;  /* ditto, when continuous */


/*------------------------------------------------------------------------
 * sig_handler
 *
 * Description:
 *   interrupt handler, return TRUE to arg test_stop. 
 *    
 */
static void sig_handler(int signo)
{
    printf("catch the signal SIGQUIT %d\n",signo);
    test_stop = TRUE; 
}

/*------------------------------------------------------------------------
 * register_intr
 *
 * Description:
 *   register interrupt SIGQUIT and handler 
 *    
 */
static void register_intr (void)
{

    if (signal(SIGQUIT, sig_handler) == SIG_ERR) { 
        perror("signal error "); 
        exit (EXIT_FAILURE); 
    }
    
    /* clean up the stop */ 
    test_stop = FALSE; 
  
}


/*------------------------------------------------------------------------
 * reset_errmsg_var
 *
 * Description:
 *    This function should not be called. This function should be
 *    replaced with the one inside linux_error.c
 *    This function is declared with weak attribute so it could be 
 *    defined elsewhere 
 *
 */

void reset_errmsg_var (void)
     __attribute__((weak, alias("__reset_errmsg_var")));
void __reset_errmsg_var (void)
{
    printf("WARNING!! '%s' in '%s' should not be invoked\n"
           , __FUNCTION__, __FILE__);
}

void
menu_togglediagflag (int flagmask)
{
    (NVRAM)->diagflag ^= flagmask;

#ifdef MENU_DEBUG
    printf("\ndiagflag %p = %#.2x\n", &(NVRAM)->diagflag, 
	   (NVRAM)->diagflag);
#endif
}

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * Function: does_platform_multitask
 *
 * Called to determine whether the underlying platform has
 * multitasking capability (in which case, global boolean
 * platform_does_multitasking has been set to TRUE during init of the
 * platform).
 *
 * Note that whether a multitasking platform is currently in
 * multitasking mode is a separate matter (determined by the value of
 * the global boolean kern_is_multitasking on such platforms).
 */
boolean
does_platform_multitask (void)
{
    return(platform_does_multitasking);
}

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * Function: does_item_multitask
 *
 * Determine whether the given item multitasks: indicated by either the
 * flag MF_MULTI or the conditional flags MF_IS_SLOT_MULTI and
 * MF_IS_HWIC_MULTI, which result in calls to does_nm_multitask or
 * does_hwic_multitask.
 */
boolean
does_item_multitask (mitem_t *miptr)
{
    return FALSE;
}

void
menu_togglexflag (int xflagmask)
{
    diagflag_xram ^= xflagmask;

#ifdef MENU_DEBUG
    printf("\ndiagflag_xram = %#.8lx\n", diagflag_xram);
#endif
}

void
toggle_destructive_xflag (int xflagmask)
{
    int answer = 'n';

    diagflag_xram ^= xflagmask;

    if (diagflag_xram & xflagmask) {
        answer = getc_answer("Do you want to run destructive tests? ", 
                             "yn", 'n');
        if (answer == 'y') {
            diagflag_xram |= D_RUN_DESTRUCT;
        } else {
            diagflag_xram &= ~D_RUN_DESTRUCT;
        }
    } else {
        diagflag_xram |= D_RUN_DESTRUCT;
    }

#ifdef MENU_DEBUG
    printf("\ndiagflag_xram = %#.8x\n", diagflag_xram);
#endif
}

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * Function: menu_config_finite_cont_number
 *
 * This function lets user to configure finite continuous number.
 * finite_cont_num = 0 -> Infinite continuous run
 * finite_cont_num > 0 -> Finite continuous run
 */
static void menu_config_finite_cont_number (void)
{
    printf("\nFinite Continuous Number = 0 ==> Infinite continuous run\n");
    printf("Finite Continuous Number > 0 ==> Finite continuous run until ");
    printf("finite continuous number is reached\n\n");
    printf("Current Finite Continuous Number: [%d]\n", finite_cont_num); 

    finite_cont_num = getdec_answer("New Finite Continuous Number", 
                                     finite_cont_num, 0, (0x80000000));

    printf("\nNew Finite Continuous Number: [%d]\n", finite_cont_num);
}


#ifdef UNKNOWN_INTRS_CTERR_IF_Z
/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * Function: menu_togglezflag
 *
 * Toggle the Z-flag.  This setting determines whether cterr() is used
 * instead of printf to report unknown interrupts in the platform.
 * Because the Z-flag is hidden from the user (does not appear in the
 * alter-flags menu), print the current value of the flag (OFF by
 * default).
 */
void
menu_togglezflag (int zflagmask)
{
    diagflag_zram ^= zflagmask;
    printf("\nToggled -- Z flag is now %s\n", 
           (diagflag_zram & D_USE_CTERR) ? "ON" : "OFF");
}
#endif /* UNKNOWN_INTRS_CTERR_IF_Z */

#ifdef AUTHENTICATION_TEST_Y
/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * Function: menu_toggleyflag
 *
 * Toggle the Y-flag. This is the flag to turn on the authentication
 * test for manufacturing. This flag is temporary.
 */
void
menu_toggleyflag (int yflagmask)
{
    diagflag_yram ^= yflagmask;
    printf("\nToggled -- Y flag is now %s\n", 
           (diagflag_yram & D_AUTH_Y) ? "ON" : "OFF");
}
#endif /* AUTHENTICATION_TEST_Y */

void
menu_show_dflags (void)
{
    unsigned short flag = (NVRAM)->diagflag;
    unsigned long xflag = diagflag_xram;
    static char onstr[] = "ON", offstr[] = "OFF";

    printf("FLAGS: %s %s  %s %s  %s %s  %s %s\n"
           "       %s %s",
	 contstr,        (flag & D_CONTINUOUS) ? onstr : offstr,
	 stopstr,        (flag & D_STOPONERR)  ? onstr : offstr,
	 ext_loopbk_str, !(flag & D_EXT_LOOPBACK) ? onstr : offstr,
     mintest_str,    !(xflag & D_MIN_TEST_TIME) ? onstr : offstr,
     ge_int_loopbk_str, (xflag & D_GE_INT_LOOPBACK) ? onstr : offstr);
}

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * Function: menu_show_all_dflags
 *
 * Display all the diag flags.  Note that this is called (only when the
 * alter-flags menu is entered) so the user can see the current flag
 * settings before toggling any of them. 
 * Also note that atlantis diag flags are different.
 */
void
menu_show_all_dflags (void)
{
    unsigned short flag = (NVRAM)->diagflag;
    unsigned long xflag = diagflag_xram;
    static char onstr[] = "ON", offstr[] = "OFF";

    printf("FLAGS: %s %s  %s %s  %s %s  %s %s\n"
           "       %s %s  %s %s  %s %s  %s %s\n"
           "       %s %s  %s %s  %s %s\n"
           "       %s %s  %s %s  %s %s  %s %s\n"
           "       %s %s",
	 contstr,        (flag & D_CONTINUOUS) ? onstr : offstr,
	 stopstr,        (flag & D_STOPONERR)  ? onstr : offstr,
	 ext_loopbk_str, !(flag & D_EXT_LOOPBACK) ? onstr : offstr,
         mintest_str,    !(xflag & D_MIN_TEST_TIME) ? onstr : offstr,
         abbr_test_str,  (flag & D_ABBR_TEST) ? onstr : offstr,
         loopstr,        (flag & D_LOOPONERR) ? onstr : offstr,
         warning_str,    (xflag & D_WARNING)  ? onstr : offstr,
         trace_str,      (xflag & D_TRACE)  ? onstr : offstr,
         verbose_str,    (flag & D_VERBOSE) ? onstr : offstr,
         option_str,     (xflag & D_SET_OPTIONS) ? onstr : offstr,
         xec_auth_str,   (xflag & D_XEC_AUTH) ? onstr : offstr,
         permu_str,      (xflag & D_PERMU_TEST) ? onstr : offstr,
         debug_str,      (xflag & D_DEBUG_OPTIONS) ? onstr : offstr,
         power_on_str,   (flag & D_POWER_ON) ? onstr : offstr,
         ext_cust_str,   (xflag & D_EXT_CUSTOMER) ? onstr : offstr,
         ge_int_loopbk_str, (xflag & D_GE_INT_LOOPBACK) ? onstr : offstr);

}

void
menu_pr_err_accum (void)
{
    printf("\n%ld total accumulated errors\n", err_accum);
}

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * Function: exec_doall_menu_items()
 *
 * This function does the same thing as the menu_exec_doall_diags()
 * except without the permutation test handling. 
 * Execute all items in this menu that have the MF_DOALL flag.  Use
 * setjmp at the start to recover in place from a BREAK the user might
 * enter during the series.  Return TRUE if all items complete, or
 * FALSE if the user breaks.
 *
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
boolean
exec_doall_menu_items (struct menuinfo *menup)
{
    jmp_buf localjmp;
    jmp_buf *savjmp = monjmpptr;   /* save original jmpbuf */
    int abort;
    char i;
    struct mitem *miptr =  menup->miptr;

    abort = setjmp(localjmp);
    register_intr(); 

    if (!abort) {
        /* Clear the test's cterr info setup */
        reset_errmsg_var();
	monjmpptr = &localjmp; /* redirect breaks */
	for (i = 0; i < menup->msize; i++, miptr++) {

            /* User stop test */
            if (test_stop == TRUE) {
                break; 
            }

	    /* 
	     * Check that if NetBooted, MF_NOTNET not set and
	     * the MF_DOALL flag is set
	     */
	    if ((!netflashbooted || !(miptr->mflag & MF_NOTNET)) &&
	        (miptr->mflag & MF_DOALL)) {

            /*
             * Determine if the current test is destructive and
             * whether it should be executed.
             */
            if ((miptr->mflag & MF_DESTRUCTIVE) &&
                (diagflag_xram & D_EXT_CUSTOMER)) {
                if (!(diagflag_xram & D_RUN_DESTRUCT)) {
                    printf("\nDestructive test skipped\n");
                    continue;
                }
            }                       

		/* Check for existence of the menu item (i.e. interface) */
		if (!(miptr->mixfunc) || (*miptr->mixfunc)(miptr->mixparam)) {
                    /*
                     * Before executing the menu item function, if
                     * pre_diag_exec is non-NULL, call it as a preliminary
                     * (e.g., to restore mempools on this platform).
                     */
                    if (pre_diag_exec) {
                        (*pre_diag_exec)();
                    }
		    (*miptr->mfunc)(*miptr->mfparam);
		    if (miptr->mflag & MF_SHOW_ERRCOUNT) {
			/*
			 * This (submenu) item doesn't give error
			 * summary by itself, as it is also used as a
			 * component of a larger diagnostic.
			 */
			prcomplete(testpass, errcount, 0);
		    }
		    errcount = 0;
		}
	    }
            /* Clear the test's cterr info setup */
            reset_errmsg_var();
	}
        monjmpptr = savjmp;  /* redirect breaks back */
        return(TRUE);
    } else { 
        monjmpptr = savjmp;  /* redirect breaks back */
        return(FALSE);  /* user did BREAK before all items completed */
    }
}

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * Function: do_all_menu_items()
 *
 * This function does the same thing as the  do_menu_all_diags()
 * except without the permutation test handling. 
 * 
 * NOTE: To avoid recursive call when running permutation test (U Flag),
 * except for menu items 'c' and 'd', do all and do group in main
 * and submenu, all other test items that attempt to do all diag
 * should use this function and not the do_menu_all_diags ()
 *
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
int
do_all_menu_items (struct menuinfo *menup)
{
    boolean all_items_done;

    all_items_done = exec_doall_menu_items(menup);
    if (all_items_done) {
        menu_pr_err_accum();
    } else {
        /*
         * User did <BREAK>.  Display accumulated errors here only if
         * not a continuous run because display will occur in menu() as
         * a result of <BREAK>.
         */
        if (!(DIAGFLAG & D_CONTINUOUS)) {
            menu_pr_err_accum();
        }
        if (monjmpptr) {
	    longjmp(*monjmpptr, 1);  /* Back to previous point */
        }
    }

    return(PASSED);
}

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * Function: menu_exec_doall_diags
 *
 * Execute all items in this menu that have the MF_DOALL flag.  Use
 * setjmp at the start to recover in place from a BREAK the user might
 * enter during the series.  Return TRUE if all items complete, or
 * FALSE if the user breaks.
 *
 * Assumption:  Generally called only from do_menu_all_diags, which
 *              then takes action depending on whether all the items
 *              completed.
 */
boolean
menu_exec_doall_diags (struct menuinfo *menup)
{
    jmp_buf localjmp;
    jmp_buf *savjmp = monjmpptr;   /* save original jmpbuf */
    int abort;
    int  test_list[MAX_TEST_ITEM], test_num = 0, idx = 0;
    register char i;
    struct mitem *miptr =  menup->miptr;

    abort = setjmp(localjmp);
    register_intr(); 

    if (!abort) {
        /* Clear the test's cterr info setup */
        reset_errmsg_var();
	monjmpptr = &localjmp; /* redirect breaks */
	for (i = 0; i < menup->msize; i++, miptr++) {

            /* User stop test */
            if (test_stop == TRUE) {
                break; 
            }
	    /* 
	     * Check that if NetBooted, MF_NOTNET not set and
	     * the MF_DOALL flag is set
	     */
	    if ((!netflashbooted || !(miptr->mflag & MF_NOTNET)) &&
	        (miptr->mflag & MF_DOALL)) {
	        
            /*
             * Determine if the current test is destructive and
             * whether it should be executed.
             */
            if ((miptr->mflag & MF_DESTRUCTIVE) &&
                (diagflag_xram & D_EXT_CUSTOMER)) {
                if (!(diagflag_xram & D_RUN_DESTRUCT)) {
                    printf("\nDestructive test skipped\n");
                    continue;
                }
            }

		/* Check for existence of the menu item (i.e. interface) */
		if (!(miptr->mixfunc) || (*miptr->mixfunc)(miptr->mixparam)) {
                    /*
                     * Prepare test list for permutation test of user chooses
                     * to run permutation test
                     */
                    if (diagflag_xram & D_PERMU_TEST) {
                        test_list[idx++] = i;
                        test_num++;
                    }
                    else {
                        /*
                         * Before executing the menu item function, if
                         * pre_diag_exec is non-NULL, call it as a preliminary
                         * (e.g., to restore mempools on this platform).
                         */
                        if (pre_diag_exec) {
                            (*pre_diag_exec)();
                        }
                        (*miptr->mfunc)(*miptr->mfparam);
                        if (miptr->mflag & MF_SHOW_ERRCOUNT) {
                            /*
                             * This (submenu) item doesn't give error
                             * summary by itself, as it is also used as a
                             * component of a larger diagnostic.
                             */
                            prcomplete(testpass, errcount, 0);
                        }
                        errcount = 0;
		    }
		}
	    }
            /* Clear the test's cterr info setup */
            reset_errmsg_var();
	}

        /*
         * Start permutation test if flag is ON
         */
	if (diagflag_xram & D_PERMU_TEST) {
            errcount = 0;
            do_permu_diags (menup, test_list, test_num);
        }

        monjmpptr = savjmp;  /* redirect breaks back */
        return(TRUE);
    } else { 
        monjmpptr = savjmp;  /* redirect breaks back */
        return(FALSE);  /* user did BREAK before all items completed */
    }
}

int
do_menu_all_diags (struct menuinfo *menup)
{
    boolean all_items_done;

    all_items_done = menu_exec_doall_diags(menup);
    if (all_items_done) {
        menu_pr_err_accum();
    } else {
        /*
         * User did <BREAK>.  Display accumulated errors here only if
         * not a continuous run because display will occur in menu() as
         * a result of <BREAK>.
         */
        if (!(DIAGFLAG & D_CONTINUOUS)) {
            menu_pr_err_accum();
        }
        if (monjmpptr) {
            longjmp(*monjmpptr, 1);  /* Back to previous point */
        }
    }

    return(PASSED);
}
/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * Function: is_diaglist_ok
 *
 */
boolean 
is_diaglist_ok (char *list, menuinfo_t *menup)
{
    int numchar, ix;
    menu_index_token_t ix_token;
    mitem_t *miptr;

    while ((numchar = get_menuindex_token(list, ix_token, FALSE)) > 0) {
	ix = get_menu_index(ix_token);
	miptr = menup->miptr + ix;  /* point to selected item */
	/*
	 * Boundary check the selection, and ensure that this item is
	 * valid when running netbooted.
	 */
	if ((ix < 0) || (ix >= menup->msize) ||
	   (netflashbooted && (miptr->mflag & MF_NOTNET)) ||
	   (miptr->mixfunc && !(*miptr->mixfunc)(miptr->mixparam))) {
	    return(FALSE);
	}
	list += numchar;
    }
    return((numchar < 0) ? FALSE : TRUE);
}    

int
do_menu_grp_diags (menuinfo_t *menup)
{
    register struct mitem *miptr;
    register struct nvram *nvptr = NVRAM;
    jmp_buf *savjmp = monjmpptr;  /* save original jmpbuf */
    jmp_buf localjmp;
    char buffer[sizeof(nvptr->diaglist)];
    int numchar, ix, abort;
    int size;

    int  test_list[MAX_TEST_ITEM], test_num = 0, i = 0;
    char *pc;
    menu_index_token_t ix_token;
    boolean list_ok = TRUE;

    /*
    ** Prompt the user for the items to include in the group test.
    ** If the tests are to be run continuously, prompt only on the
    ** first pass and check entries whether list is new or old.
    */
    if(!(nvptr->diagflag & D_CONTINUOUS) || testpass == 1) {
	/* 
	 * after stopping the group continuous test, needs to
	 * clear the error accumulated count
	 */
	err_accum = 0;
	displaymenu(menup, MF_DOALL | MF_DOGRP);
	do {
	    printf("\nenter the menu items to execute\n(in test order) [%s]:",
                   nvptr->diaglist);
	    get_line(buffer, sizeof(buffer));
            size = strlen(buffer);

            if (size) {
                if (buffer[size-1] ==  '\r' || buffer[size-1] == '\n')
                    buffer[size-1] = '\0';
            }

	    if(*buffer) {
		/* user entered a list */
		list_ok = is_diaglist_ok(buffer, menup);
		if (!list_ok) {
		    puts("illegal menu item..., please try again\n"
			 "(make sure one space between each menu item)\n");
		} else {
		    strcpy(nvptr->diaglist, buffer);
		}
	    } else {
		/* check prior diaglist as it may not apply to this menu */
		list_ok = is_diaglist_ok(nvptr->diaglist, menup);
		if (!list_ok) {
		    puts("\nsaved list in error for this menu...try again");
		}
	    }
	}
	while (!list_ok);
    }

    pc = nvptr->diaglist;
    abort = setjmp(localjmp);
    register_intr(); 

    if (!abort) {
        /* Clear the test's cterr info setup */
        reset_errmsg_var();
	monjmpptr = &localjmp; /* redirect breaks */

	while ((numchar = get_menuindex_token(pc, ix_token, FALSE)) > 0) {

            /* User stop test */
            if (test_stop == TRUE) {
                break; 
            }

	    ix = get_menu_index(ix_token); 

            /* flush the error buffer in between test items */
            flush_test_progress_buf();

            miptr = menup->miptr + ix;
	    /*
	     * Check that item is in the list and valid.
	     */
	    if(ix < 0 || ix >= menup->msize) {
		break;
	    }

            /* 
             * NM-Volant has tests that last more than an hour
             * we don't want to use MF_DOALL, but we do want to
             * group it if running continuous flag
             */

            /* check to make sure we can */
            if (miptr->mflag & (MF_DOALL | MF_DOGRP)) {
                
                /*
                 * Determine if the current test is destructive and
                 * whether it should be executed.
                 */
                if ((miptr->mflag & MF_DESTRUCTIVE) &&
                    (diagflag_xram & D_EXT_CUSTOMER)) {
                    if (!(diagflag_xram & D_RUN_DESTRUCT)) {
                        printf("\nDestructive test skipped\n");
                        continue;
                    }
                }
		/*
		 * Prepare test list for permutation test of user chooses
		 * to run permutation test and it hasn't been run before.
		 */
		if (diagflag_xram & D_PERMU_TEST) {
		    test_list[i++] = ix;
		    test_num++;
		}
		else {
		    /*
		     * Before executing the menu item function, if
		     * pre_diag_exec is non-NULL, call it as a preliminary 
		     * (e.g., to restore mempools on this platform).
		     */
		    if (pre_diag_exec) {
			(*pre_diag_exec)();
		    }
		    (*miptr->mfunc)(*miptr->mfparam);
		    if (miptr->mflag & MF_SHOW_ERRCOUNT) {
			/*
			 * This (submenu) item doesn't give error
			 * summary by itself, as it is also used as a
			 * component of a larger diagnostic.
			 */
			prcomplete(testpass, errcount, 0);
		    }
		    errcount = 0;
		}
                /* Clear the test's cterr info setup */
                reset_errmsg_var();
	    }
	    pc += numchar;  /* move past this token in diaglist */
	}
	/*
	 * Start permutation test if flag is ON
	 */
	if (diagflag_xram & D_PERMU_TEST) {
	    errcount = 0;
	    do_permu_diags (menup, test_list, test_num);
	}
        menu_pr_err_accum();
    } else { 
        /*
         * User did <BREAK>.  Display accumulated errors here only if
         * not a continuous run because display will occur in menu() as
         * a result of the <BREAK>.
         */
        if (!(DIAGFLAG & D_CONTINUOUS)) {
            menu_pr_err_accum();
        }
    }
    monjmpptr = savjmp;  /* redirect breaks back */

    if(abort && savjmp) longjmp(*savjmp,1);

    return(PASSED);
}

/***********************************************************************
 * 
 *   Function: do_permu_diags()
 *
 * Description: 
 *     Performs test on all arrangements of 2 out of n test items to be 
 * executed. This test is used to increase the chance of catching test 
 * interference problem. This function is only useful if the permutation 
 * flag (U) is turned on. Let n be the total number of items to be executed 
 * and k be the number of items to be run at one time (2 in this case), then 
 * the number of passes need to be run to exhaust all arrangements is: 
 * (n P k) = n!/(n - k)!. For example, if the do group list has 6 items,
 * and we choose to run arrangement of 2, then the number of pass is:
 * 6!/(4!) = 30 passes (non-duplicated ordered list).
 *
 * Input: Pointer to the menu items
 *        Pointer to list of test items to be executed
 *        The number of items contains in the above list
 *
 * Output: None
 *
 ***********************************************************************/
void
do_permu_diags (menuinfo_t *menup, int *test_idx, int test_count)
{
    register struct mitem *ptr1, *ptr2;
    int i, j, total_pass, count = 0;

    /*
     * There are a total of n!/(n - 2)! = n * (n-1) non-duplicated ordered
     * arrangement of 2. For duplicated ordered arrangement of 2 such as 
     * a a, b b, ... there are * n * (n-1) + n = n^2 arrangements.
     */
    if (test_count < 2)
	total_pass = test_count;
    else
	total_pass = test_count * test_count;

    /* Start test */
    for (i = 0; i < test_count; i++) {
	ptr1 = (struct mitem *)(menup->miptr + (int)test_idx[i]);
	printf("Performing test at ");
	printf(menup->mtitle, menup->mtparam);
	for (j = 0; j < test_count; j++) {
	    ptr2 = (struct mitem *)(menup->miptr + (int)test_idx[j]);
	    count++;
	    prpass(testpass, "Arrangement #%d of %d, (%s and %s),", count, 
		   total_pass, ptr1->mline, ptr2->mline);
	    /* restore mempool if pre_diag_exec is non-NULL */
	    if (pre_diag_exec) {
		(*pre_diag_exec)();
	    }
	    (*ptr1->mfunc)(*ptr1->mfparam);
	    /* restore mempool if pre_diag_exec is non-NULL */
	    if (pre_diag_exec) {
		(*pre_diag_exec)();
	    }
	    (*ptr2->mfunc)(*ptr2->mfparam);
	}
    }
    prcomplete(testpass, errcount, 0);
}

int
menu_flags (int c)
{
  int mask = 0;
  int xmask = 0;

  switch(c) {
  case 'A': mask = D_ABBR_TEST; break;
  case 'C': mask = D_CONTINUOUS; break;
  case 'D': xmask = D_DEBUG_OPTIONS; break;
  case 'E': mask = D_EXT_LOOPBACK;  break;
  case 'I': xmask = D_GE_INT_LOOPBACK;  break;
  case 'L': mask = D_LOOPONERR; break;
  case 'M': xmask = D_MIN_TEST_TIME; break;
  case 'N': xmask = D_EXT_CUSTOMER; break;
  case 'O': xmask = D_SET_OPTIONS; break;
  case 'P': mask = D_POWER_ON;break;
  case 'Q': mask = D_QUIETMODE; break;
  case 'S': mask = D_STOPONERR; break;
  case 'T': xmask = D_TRACE;   break;
  case 'V': mask = D_VERBOSE;   break;
  case 'W': xmask = D_WARNING;   break;
  case 'X': xmask = D_XEC_AUTH;   break;
  case 'U': xmask = D_PERMU_TEST; break;
#ifdef UNKNOWN_INTRS_CTERR_IF_Z
  case 'Z': 
      menu_togglezflag(D_USE_CTERR);
      break;
#endif /* UNKNOWN_INTRS_CTERR_IF_Z */
#ifdef AUTHENTICATION_TEST_Y
  case 'Y': 
      menu_toggleyflag(D_AUTH_Y);
      break;
#endif /* AUTHENTICATION_TEST_Y */
  default: puts("unknown flag\n"); return(-1);
  }
  menu_togglediagflag(mask);
    if (xmask == D_EXT_CUSTOMER) {
        toggle_destructive_xflag(xmask);
    } else {
        menu_togglexflag(xmask);
    }  
  return(0);
}


boolean check_menu_flag (uint flag) {
    if((NVRAM)->diagflag & flag)
        return TRUE;
    else
        return FALSE;
}


/*
** Fix buffer for secondary menu note, which is written by mtfunc
** show_endnote().
 */
char endnote_buf[160]; 

/* 
 * Default the UUT for display registers
 */
void
display_uut_default (print_fn_t print_fn)
{
#ifndef LINUX_APP
    print_fn("\nIn display_uut_default: ");
    print_fn("\nPlease initialize display_uut_registers function.");
#else
    printf("FIX ME menu.c display_uut_default\n"); 
#endif
}

/*
** Continuously display the menu prompt and call the corresponding
** menu function (mfunc).  The menu selections are boundary checked.
** The user may display the menu by entering a <CR>,or may return
** to the calling procedure by typing <ESC><RET>.
**
** A double letter (lower case only) entered legally indicates that
** the user wants to see a secondary menu corresponding to the diag
** selected.  In this case, second_miptr must be non-NULL (pointing to
** an array of the same size as menuptr->miptr), and the second_miptr->
** mfunc corresponding to the (doubly) selected item must also be non-
** null.  Thus, second_miptr->mfunc would be called (instead of the
** primary mfunc), and would either display a secondary menu, or output
** a message saying that it is not implemented for this diagnostic.
**
** The printed index to menu items goes from 'a' to 'z', then to '1a', 
** '1b',...,'1z','2a','2b', ..., and so on.  
*/
void
menu (menuinfo_t *menuptr, mitem_t *second_miptr, char menucmd)
{
    jmp_buf *savjmp = monjmpptr;  /* save original jmpbuf */
    jmp_buf menujmp;
    unsigned char dismen = 1;
    char linebuf[80];
    int i, ix;
    boolean dbl_char_entered;
    register char c;
    register struct mitem *miptr;
    unsigned char continuous;

    err_accum = 0;  /* initialize */
    i = 0;

    while(1) {  /* do until user says to quit */

        /* clean out test_progress_buf */
        flush_test_progress_buf();
        /* Clear the test's cterr info setup */
        reset_errmsg_var();

        finite_cont_cntr = 0;

	if(menucmd) {
	    c = menucmd;
	    menucmd = EXIT_CHAR;  /* exit after this one command */
	} else {
	    if(dismen) {
		displaymenu(menuptr, 0);
		dismen = 0;
	    }

            printf("\n");
	    if(menuptr->mprompt) puts(menuptr->mprompt);
	    else {

#ifdef DISPLAY_ENGINEERING_BANNER
	engineering_banner();
#endif
		printf("enter ");
		printf(menuptr->mtitle, menuptr->mtparam);
		printf(" item > ");  /* waiting for input */
	    }

            get_line(linebuf, sizeof(linebuf));
            i = strlen(linebuf);
	    if (i == (sizeof(linebuf) - 1)) {
		continue;
            }
	    if (!strlen(linebuf)) {
		dismen = 1;
		continue;
	    }
            if (linebuf[0] == '\n') {
		dismen = 1;
		continue;
	    }

	    for (i=0; ; i++) {
		c = linebuf[i];
		if (!c) {
		    dismen = 1;
		    break;
		} else if (c != ' ' && c != '\t')
		    break;
	    }
	    if (dismen)
		continue;
	}
	if(c == EXIT_CHAR) {  /* EXIT_CHAR entered, return to caller */
	    return;
	}
	if(c >= 'A' && c <= 'Z') {
	    menu_flags(c);
	    dismen = 1;
	    continue;
	}
	
	/*
	 * Parse the user's menu selector
	 */
	ix = get_menu_index(&linebuf[i]);
	dbl_char_entered = (ix >= DOUBLE_CHAR_INCREMENT);
	if (dbl_char_entered) {
	    ix -= DOUBLE_CHAR_INCREMENT;
	}
	
	miptr = menuptr->miptr + ix;  /* point to selected item */
	
	/*
	 * Boundary check the selection, and ensure that this item is
	 * valid when running netbooted.
	 */
	if ((ix < 0) || (ix >= menuptr->msize) ||
	   (netflashbooted && (miptr->mflag & MF_NOTNET)) ||
           (miptr->mixfunc && !(*miptr->mixfunc)(miptr->mixparam) &&
            !(miptr->mflag & MF_HIDDEN_EXE))) {
	    printf("illegal menu item..., please try again\n"
		 "(make sure one space between each menu item)\n");
	    continue;
	}
	dismen = 0;
	errcount = 0;  /* initialize */
	testpass = 0;
	hkeepflags &= ~H_USRINT;

	if(miptr->mflag & MF_CONTINUOUS) {  /* test can be run continuously */
	    continuous = (!dbl_char_entered && (DIAGFLAG & D_CONTINUOUS));
	    if(continuous) {
		testpass = 1;
	    }
	} else {
	    continuous = 0;
	}

        register_intr(); 
	switch(setjmp(menujmp)) {
	case 0:
	    monjmpptr = &menujmp; /* redirect breaks */
            async_select = 0; /* LINGL: for 24m_diag.c get_async_clk() */
	    while(1) {
		if (dbl_char_entered) {
		    /*
		     * User entered double character.  If secondary menu was
		     * given, and if diagnostic corresponding to the entered
		     * character exists, execute it.
		     */
		    if (!do_second_item(second_miptr, ix)) {
			printf("subtest menu not available for this "
			     "double character\n");
		    }
		} else if(miptr->mfunc) {
            /*
             * Determine if the current test is destructive and
             * whether it should be executed.
             */
            if ((miptr->mflag & MF_DESTRUCTIVE) &&
                (diagflag_xram & D_EXT_CUSTOMER)) {
                if (!(diagflag_xram & D_RUN_DESTRUCT)) {
                    printf("\nDestructive test skipped\n");
                    break;
                }
            }

                    /*
		     * Before executing the menu item function, if
		     * pre_diag_exec is non-NULL, call it as a preliminary.
                     */
		    if (pre_diag_exec) {
			(*pre_diag_exec)();
		    }
                    /*
                     * If in continuous mode AND the platform has a
                     * real time clock, display the time before execution
                     * of the item in each pass.
                     */
                    if (((NVRAM)->diagflag & D_CONTINUOUS) && 
                        (menu_display_real_time != NULL)) {
                        (*menu_display_real_time)();
                    }
                    /*
                     * In case item function called is menu() itself, push
                     * dummy args for the second and third parameters also.
                     */
                    (*miptr->mfunc)(*miptr->mfparam,0,0);
		    if (miptr->mflag & MF_SHOW_ERRCOUNT) {
			/*
			 * This (submenu) item doesn't give error
			 * summary by itself, as it is also used as a
			 * component of a larger diagnostic.
			 */
			prcomplete(testpass, errcount, 0);
		    }
		}
                /* Clear the test's cterr info setup */
                reset_errmsg_var();

        finite_cont_cntr++;
		if (!continuous || (finite_cont_num && finite_cont_cntr >= finite_cont_num)) {
            break;
        }
                if(test_stop) break;
		testpass++;  /* increment our pass count */
	    }
	    monjmpptr = savjmp;  /* redirect breaks back */
            break;
        case 1:   /* console break (out of run) */
            if (continuous) {
                menu_pr_err_accum();
            }
	    /* fall through */
	default:  /* longjmp(menujmp, X > 1) */
	    monjmpptr = savjmp;  /* redirect breaks back */
	}
    }
}

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * Function: item_is_called_for
 *
 * Determine whether the given item should be displayed, given its pointer
 * and the called-for mflag.  When the argument mflag is nonzero, display
 * only those items having that flag.  However, when a nonzero arg mflag
 * is unequal to the item mflag, call does_item_multitask, which evaluates
 * the cases of item mflags MF_MULTI and the two conditional multitasking 
 * mflags MF_IS_SLOT_MULTI and MF_IS_HWIC_MULTI.
 */
boolean
item_is_called_for (mitem_t *miptr, int mflag)
{
    if (netflashbooted && (miptr->mflag & MF_NOTNET)) {
        return(FALSE);
    }
    /*
     * If an existence function is assigned for this item, make its
     * return value a condition for display.
     */
    if (!(miptr->mixfunc) || (*miptr->mixfunc)(miptr->mixparam)) {
	return(((mflag) ? (miptr->mflag & mflag) : TRUE) ||
               does_item_multitask(miptr));
    } else {
        return(FALSE);
    }
}

/*
** Display the menu for the user.  Menu items are numbered (lettered)
** automatically starting with the letter 'a', continuing through 'z',
** and, if necessary, going on to '1a', '1b', ..., '1z', '2a', '2b', ...,
** etc.  If the given mflag is MF_MULTI, suppress printout of the title.
** For each item in the menu, call the function item_is_called_for to
** determine whether to show the item.
*/
void
displaymenu (struct menuinfo *menuptr, int mflag)
{
    register int i, mletter = 'a';  /* menu item letters start with 'a' */
    char mdigit = ' ';          /* <SP>,1,2,... */
    register struct mitem *miptr = menuptr->miptr;  /* pointer to first mitem */

    if (mflag != MF_MULTI) {
	/* display the title for the user */
	printf("\n\n      ");  /* indent it */
	printf(menuptr->mtitle, menuptr->mtparam);
	printf("\n");
    } else {
	printf("\nFollowing items in ");
	printf(menuptr->mtitle, menuptr->mtparam);
	printf(" capable of multitasking\n");
        slots_polled_for_display = FALSE;
    }
    for(i=0; i < menuptr->msize; i++, miptr++, mletter++) {
	if (item_is_called_for(miptr, mflag)) {
            printf("%c%c: ", mdigit, mletter);
            if(miptr->mline) printf(miptr->mline, miptr->mlparam);
            if(miptr->mlfunc) (*miptr->mlfunc)(miptr->mlparam);
            printf("\n");
        }
	if (mletter == 'z') {
	    mletter = '`';   /* mletter++ == 'a' */
	    mdigit = (mdigit == ' ') ? '1' : (mdigit + 1);
	}
    }
    if ((mflag != MF_MULTI) && menuptr->mtfunc) {
	(*menuptr->mtfunc)();
    }
}

/*
** Convert the user's menu selection into an integer index, which is
** the return value.  A valid input must be one of the following tokens:
**    a single lower-case letter;
**    two matching l.c. letters;
**    a single digit from one to nine, followed by a single l.c. letter;
**    a single digit from one to nine, followed by two matching l.c. letters;
**
** This token must be terminated by a char c such that isspace(c) is TRUE,
** or c is '`0`, the string terminator.
**
** If the input is invalid, the return value is -1.  For the case of a
** single letter entered, the return value is (c - 'a') + d*26, where
** d is the digit (0 if no digit entered).  When a matching letter is
** added, the return value is equal to the single-letter value incremented
** by the constant DOUBLE_CHAR_INCREMENT.
*/
int
get_menu_index (char *inbuf)
{
    char c, c2;
    int d, i, retval;

    c = inbuf[0];
    if ((c > '0') && (c <= '9')) {
	d = c - '0';
	i = 1;
    } else if ((c < 'a') || (c > 'z')) {
	return(-1);
    } else {
	d = 0;
	i = 0;
    }
    
    c = inbuf[i];
    if (islower(c)) {
	retval = c - 'a' + d*26;
    } else {
	return(-1);
    }
    i++;
    c2 = inbuf[i];
    if (islower(c2)) {
	if (c == c2) {
	    retval += DOUBLE_CHAR_INCREMENT;
	    i++;
	    c2 = inbuf[i];
	}
    }
    return((isspace(c2) || (c2 == '\0')) ? retval : -1);
}


/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * Function: get_menindex_token
 *
 * Given the pointer to an input buffer, scan the line for the next
 * menu index token.  Return 0 if End Of Line is reached, or -1 if the
 * menu index token found is defective.  Otherwise, copy the found
 * menu index token to the given token pointer, and return the
 * increment to *inbuf that would point it at the next character
 * following the token.
 *
 * N.B.  In this context, where there may be multiple menu index
 * tokens in the input buffer, if the boolean numbers_ok is false, the
 * token must contain only a single lower-case letter, but may be
 * prefixed by a digit from one to nine.  Otherwise, the token may
 * additionally contain a one- or two-digit number in the range
 * [1..99].
 */
int
get_menuindex_token (char *inbuf,
		     menu_index_token_t ix_token,
		     boolean numbers_ok)
{
    char c;
    char *cp = inbuf;
    int numchar = 0;
    boolean numeric = FALSE;

    /*
     * Skip blanks (and tabs, etc.)
     */
    while (isspace(*cp)) {
	cp++;
    }
    
    c = *cp++;
    if (c == '\0') {
	return(0);
    }

    if ((c > '0') && (c <= '9')) {
	numchar++;
	c = *cp++;
	if (numbers_ok) {
	    if (isspace(c) || c == '\0') {
		numeric = TRUE;  /* single digit */
                --cp;  /* Back up to this terminator */
	    } else if (isdigit(c)) {
		numeric = TRUE;  /* double digits */
		numchar++;
		c = *cp;
	    }
	}
    }

    if (!numeric) {
	if (islower(c)) {
	    numchar++;
	    c = *cp;
	} else {
	    return(-1);
	}
    }

    /*
     * Legitimate token must be terminated properly.  If it is,
     * copy the token and its terminating character.
     */
    if (isspace(c) || c == '\0') {
        movbyte((unsigned char *)(cp - numchar), (unsigned char *)ix_token, 
		numchar + 1);
	return(cp - inbuf);
    } else {
	return(-1);
    }
}

/*
** User entered double character.  Check conditions for execution of
** corresponding secondary menu item.  If true, execute its mfunc, and
** return TRUE.  Otherwise, return FALSE.  If pre_diag_exec is non-null,
** call it as a preliminary to the item function.
*/
boolean
do_second_item (mitem_t *secptr, int index)
{
    if (!secptr) {
	return(FALSE);
    } else {
	secptr += index;   /* at corresponding mfunc */
	if (secptr->mfunc) {
	    if (pre_diag_exec) {
		(*pre_diag_exec)();
	    }
	    (*secptr->mfunc)(*secptr->mfparam);
	    return(TRUE);
	} else {
	    return(FALSE);
	}
    }
}

/*
** Function: show_endnote
** Display the global message buffer.  Also display the diag flags.
*/
int
show_endnote (void)
{
    printf("\n%s", endnote_buf);
    menu_show_dflags();
    return(0);
}

/*
** Function: init_empty_menu
** Initialize a subtest menu, which will be built dynamically by
** calls to add_menu_item().
*/
void
init_empty_menu (menuinfo_t *subtest_menu, type_t mtparam)
{
    subtest_menu->mtparam = mtparam;
    subtest_menu->msize = 0;
    sprintf(endnote_buf, " ");  /* Flush buffer */
}

/*
** Function: init_base_submenu
** Form a base subtest menu that initially contains the following items:
**     alter diag flags
**     enter basic utilities
**     execute all diags in the submenu
**     execute a subset of diags in the submenu
**
**
** If the global array post_item_base has been initialized by a
** platform, add its items at the end of the preceding items.
** Copy the existence function and its parameter separately out of
** post_item_base inasmuch as add_menu_item ignores these fields.  This
** processing has one special effect: if an entry in post_item_base[]
** has an *itemparam equal to -1, the parameter pointer actually
** used in the resulting add_menu_item call is the given
** addr_submenup.
**
** The remaining items in this submenu will be built dynamically
** elsewhere by calls to add_menu_item().
**
** N.B.  Because the parameter passed to the menu item function (mfunc)
**       is dereferenced as int *, a param referring to the submenu
**       itself must be the address of a pointer to the submenu.
*/
void
init_base_submenu (menuinfo_t **addr_submenup, type_t mtparam)
{
    int i;
    type_t *param;
    subitem_x_t *s;
    mitem_t *pi;
    menuinfo_t *pm = *addr_submenup;
    
    init_empty_menu(*addr_submenup, mtparam);
    add_menu_item(*addr_submenup, adiagfstr, (PFT)menu,
		  (type_t *)(((type_t)&menu_diagflagp)), 0);
    add_menu_item(*addr_submenup, basutilstr, (PFT)menu,
                  (type_t *)(((type_t)&utilmenup)), 0);
    add_menu_item(*addr_submenup, doalldgstr, (PFT)do_menu_all_diags,
		  (type_t *)addr_submenup, MF_CONTINUOUS);
    add_menu_item(*addr_submenup, dogrpdgstr, (PFT)do_menu_grp_diags,
		  (type_t *)addr_submenup, MF_CONTINUOUS);

    pi = pm->miptr + pm->msize;  /* next after base items */
    if (num_post_item_base > 0) {
	s = post_item_base;
	for (i = 0; i < num_post_item_base; i++, s++, pi++) {
	    param = s->itemparam;
	    add_menu_item(*addr_submenup,
			  s->itemstr,
			  (PFT)s->itemfcn,
			  (param == (type_t *)-1) ? (type_t *)addr_submenup : param,
			  s->itemflag);
            pi->mixfunc = s->xitemfunc;
            pi->mixparam = s->xitemparam;
	}
    }
}

/*
** Function: add_menu_item
** Add an item to the mitem array of the given menu.
*/
void
add_menu_item (menuinfo_t *menu,
	       char *item_descrip,
	       type_t (*mfunc)(),
	       type_t *mfparam,
	       int mflag)
{
    mitem_t *item = menu->miptr;

    item += menu->msize;  /* position this next item in the array */
    item->mline = item_descrip;
    item->mfunc = mfunc;
    item->mfparam = mfparam;
    item->mflag = mflag;
    menu->msize++;
}


/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * Function: build_primary_submenu
 *
 * Generate the primary submenu with the given title and defined by the
 * args _xtable_ and size.  The address of the pointer to the primary
 * submenu is the arg addr_submenup (see note below).  Beyond the base
 * items, add each item defined in the _xtable_ by calling add_menu_item.
 * But add_menu_item() ignores the menu existence function and parameter,
 * so then copy those out of the corresponding _xtable_ element.
 * 
 *
 * N.B.  Because the parameter passed to the menu item function (mfunc)
 *       is dereferenced as int *, a param referring to the submenu
 *       itself must be the address of a pointer to the submenu.
 */
void
build_primary_submenu (submenu_xtable_t *px, int size, char *title,
                       menuinfo_t **addr_submenup)
{
    int i;
    mitem_t *pi;
    menuinfo_t *pm = *addr_submenup;

    init_base_submenu(addr_submenup, (long)title);
    base_submenu_item_total = pm->msize;
    pi = pm->miptr + pm->msize;  /* initially at next item after base items */
    for (i = 0; i < size; i++, px++, pi++) {
        add_menu_item(*addr_submenup, px->x_title, px->x_pfunc,
                      &px->x_pparam, px->x_flags);
        pi->mixfunc = px->x_xfunc;
        pi->mixparam = px->x_xparam;
    }
} 


/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * Function: build_secondary_submenu
 *
 * Generate the secondary submenu defined by the args _xtable_ and
 * size into the mitem array pointed to by arg pm.  This is a two-step
 * process: first insert the base items, followed by the secondary
 * function & parameter from each element of the xtable.
 *
 * Note that the item string copied into each element of the submenu
 * is for appearance only; just the diag function and param are 
 * required in a secondary menu.
 *
 */
void
build_secondary_submenu (submenu_xtable_t *px, int size, mitem_t *pm)
{
    int i;
    subitem_x_t *s;

    /* setup the secondary function to config the finite continuous number */
    pm->mline = adiagfstr;
    pm->mfunc = (PFT)menu_config_finite_cont_number;
    pm++->mfparam = (type_t *)&ext_cust_mask;

    pm++->mline = basutilstr;
    pm++->mline = doalldgstr;
    pm++->mline = dogrpdgstr;
    for (i = 0, s = post_item_base; i < num_post_item_base; i++, s++, pm++) {
        pm->mline = s->itemstr;
    }
    for (i = 0; i < size; i++, px++, pm++) {
        pm->mline = px->x_title;
        pm->mfunc = px->x_sfunc;
        pm->mfparam = &px->x_sparam;
    }
}

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * Function: menu_item_reserved
 *
 * Serves as a common (non-)existence function that can be used in menus
 * to reserve an item index.  Returns FALSE so that the item does not
 * appear in the display but serves to get the item index incremented.
 * Used, for example, to force a certain following item to always appear
 * at a fixed menu letter or index.
 */
boolean
menu_item_reserved (void)
{
    return(FALSE);
}

/* End of module */

/******** History ******** 
$Log: menu.c,v $
Revision 1.26  2021/06/02 07:43:01  iachang
CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk

Revision 1.25  2019/07/11 12:34:40  alicehua
Collapse Nutella codes into main trunk

Revision 1.24.68.1  2019/04/01 09:38:07  harrchan
Add finite continuous feature

Revision 1.24  2018/05/18 09:24:48  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.23.2.3  2017/04/28 02:28:40  alpeng
add ctrl-\ for break continuous test

Revision 1.23.2.2  2017/01/09 00:49:49  ptong
Print the skipped plugin list after mb_tests menu title to warn user on Neptune diag

Revision 1.23.2.1  2016/12/27 02:06:29  meho
Added ge-Int loopback flag to control Cavium GE int/ext loopback test.

Revision 1.23  2016/04/20 07:03:33  benchen2
merge tachi_branch to maintrunk

Revision 1.22.8.1  2016/01/21 03:54:15  alpeng
hide eXec authenication flag on genernal menu

Revision 1.22  2015/01/14 08:45:32  danchung
add a new menu item flag to be an attribute to make the menu item hidden
and still executable

Revision 1.21  2014/08/18 22:15:14  yuetwang
add engineering banner

Revision 1.20  2013/11/26 08:40:34  hroni
fix compiler warning

Revision 1.19  2013/11/13 11:46:45  hroni
add four for extern use

Revision 1.18  2013/11/11 21:18:39  mcharon
pass string instead of number in first argum of host_send_packet ; add xaui support

Revision 1.17  2013/10/08 11:03:47  erwu2
enhanced err msg first check-in

Revision 1.16  2013/06/24 08:39:24  alpeng
support Ext. loopback flag checking before USB/CF diag

Revision 1.15  2013/02/06 01:43:21  mcharon
use capital P instead of lowe case p in power_on_str

Revision 1.14  2012/10/25 06:16:55  mcharon
turn on module when exit test

Revision 1.13  2012/09/18 23:06:19  mcharon
remove netbootflash variable causing do_grop to fail

Revision 1.12  2012/07/18 22:59:29  ptong
Fix a problem so that (NVRAM)->diagflag is used correctly on Cavium data plane menu

Revision 1.11  2012/06/25 07:02:14  alpeng
revert method for storing diag flag

$Endlog$
*/

