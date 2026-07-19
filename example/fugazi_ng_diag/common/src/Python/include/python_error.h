/* $Id: python_error.h,v 1.2 2014/06/03 10:53:27 erwu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/Python/include/python_error.h,v $
 *------------------------------------------------------------------
 * Description: error handle header files.
 * Oct 2013 - erwu2
 *
 * Copyright (c) 2013-2014 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __PYTHON_ERROR_H__
#define __PYTHON_ERROR_H__

#include <stdbool.h>
#include <signal.h>

/*-----------------------------------------------------------------------------
 * python-specific prototypes
 *---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 * SRG environment prototypes
 *---------------------------------------------------------------------------*/
extern int ran_num_one_ten();
extern void testname(char *, ...);
extern void prpass(int , char *, ...);
extern unsigned long testpass;
extern unsigned long errcount;

extern char testnamebuf[];
extern void cterr(char, int, char *, ...);
extern void inthdlr(int);

extern int dumperrlog();
extern void clrtestname();
extern void prcomplete(int, int, char *, ...);

int cterr_clear_debug(void);
int cterr_add_debug(char *fmtptr,...);
int cterr_clear_component(void);
int cterr_add_component(char *fmtptr,...);
int cterr_clear_reg_dump (void);
int cterr_add_reg_dump (void(*func_ptr)(),...);
int cterr_clear_env_dump (void);
int cterr_add_env_dump (void (*func_ptr)(),...);

int cterr_add_component_exec(char *fmtptr,...);
int cterr_add_reg_dump_exec (void(*func_ptr)(),...);
int cterr_add_env_dump_exec (void(*func_ptr)(),...);
int cterr_add_debug_exec    (char *fmtptr,...);

/* Buffer that will hold the name of the test been performed. */
#define TESTNAMEBUFSIZ 80
#define FRU_INFO_INVALID 0xFF

/* add an specific str "END_LIST" for wrapper to know */
/* where is end point of variable function */
#define END_LIST "END_LIST"

/* define wrapper for cterr_add_xxx to know */
/* where is end point of variable function */
#define cterr_add_component(args...) cterr_add_component_exec(args, END_LIST)
#define cterr_add_reg_dump(args...)  cterr_add_reg_dump_exec(args,END_LIST)
#define cterr_add_env_dump(args...)  cterr_add_env_dump_exec(args,END_LIST)
#define cterr_add_debug(args...)     cterr_add_debug_exec(args, END_LIST)

/* define error/debug log file */
#define ERRLOG "/errlog.txt" 
#define DBLOG "/dblog.txt"

/* Structure used to hold component list for cterr */
typedef struct cterr_component_ {
    unsigned char *comp_name;
    struct cterr_component_ *next;
} cterr_component_t;

/* Structure used to hold debug info list for cterr */
typedef struct cterr_debug_ {
    unsigned char *debug_comment;
    struct cterr_debug_ *next;
} cterr_debug_t;

/* Structure used to hold reg info list for cterr */
typedef struct cterr_reg_dump_ {
    void (*reg_dump_func)();
    struct cterr_reg_dump_ *next;
} cterr_reg_dump_t;

/* Structure used to hold env info list for cterr */
typedef struct cterr_env_dump_ {
    void (*env_dump_func)();
    struct cterr_env_dump_ *next;
} cterr_env_dump_t;

typedef struct fru_table_ {
    unsigned char *pid_string;
    unsigned char *location_string;
} fru_table_t;

#endif /*__PYTHON_ERROR_H__*/

/*
 *------------------------------------------------------------------
 * $Log: python_error.h,v $
 * Revision 1.2  2014/06/03 10:53:27  erwu2
 * python menu collapsed to main trunk
 *
 * Revision 1.1.2.6  2014/04/29 11:40:36  erwu2
 * update python file structure
 *
 * Revision 1.1.2.5  2014/04/24 08:53:52  erwu2
 * merge makefile and add flag example to test
 *
 * Revision 1.1.2.4  2014/01/21 10:45:19  erwu2
 * improve executable and submenu column definition
 *
 * Revision 1.1.2.3  2014/01/16 11:15:54  erwu2
 * update python files
 *
 * Revision 1.1.2.2  2013/12/19 10:25:19  erwu2
 * improve tftp dnld process
 *
 * Revision 1.1.2.1  2013/12/09 06:20:31  erwu2
 * python menu for o2 example
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
