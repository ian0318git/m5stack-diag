/* $Id: error.h,v 1.2 2015/05/25 03:59:10 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/skye_linux/include/error.h,v $
 *------------------------------------------------------------------
 *
 * error.h: Headfile for common definitions of error display.
 *
 * April 17, 2013 - palin2 ported from Overlord.
 *
 * Copyright (c) 2013-2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 * Author: clev
 *------------------------------------------------------------------
 */

#define TESTNAMEBUFSIZ 80

typedef char testnamebuf_t[TESTNAMEBUFSIZ];

extern unsigned long multi_testpass[];
extern unsigned long multi_errcount[];
extern unsigned long multi_err_accum[];
extern unsigned long multi_warncount[];
extern testnamebuf_t multi_testname[];

extern char *errlog_start;
extern char testnamebuf[];
extern unsigned long testpass, errcount, err_accum, warncount;
extern char *banner_string;

extern void errleds(), clrline();
extern void logprintf(), bell(), clrerrlog(), scanerrlog();
extern int  dumperrlog();
extern void dump_n_flush();
extern void clrtestname();
extern void cterr(char, int, char*, ...);
extern void prpass(int, char *, ...);
extern void prcomplete(int, int, char *, ...);
extern void testname(char *, ...);

/* For Skye enhanced error message */
extern int cterr_clear_debug(void);
extern int cterr_add_debug(char *fmtptr,...);
extern int cterr_clear_component(void);
extern int cterr_add_component(char *fmtptr,...);
extern int cterr_clear_reg_dump (void);
extern int cterr_add_reg_dump (void(*func_ptr)(),...);
extern int cterr_clear_env_dump (void);
extern int cterr_add_env_dump (void (*func_ptr)(),...);

extern int cterr_add_component_exec(char *fmtptr,...);
extern int cterr_add_reg_dump_exec (void(*func_ptr)(),...);
extern int cterr_add_env_dump_exec (void(*func_ptr)(),...);
extern int cterr_add_debug_exec    (char *fmtptr,...);

extern unsigned int cterr_db_print(char *fmtptr, ...);
/* add an specific str "END_LIST" for wrapper to know */
/* where is end point of variable function*/
#define END_LIST "END_LIST"

/* define wrapper for cterr_add_xxx to know */
/* where is end point of variable function*/
#define cterr_add_component(args...) cterr_add_component_exec(args, END_LIST)
#define cterr_add_reg_dump(args...)  cterr_add_reg_dump_exec(args,END_LIST)
#define cterr_add_env_dump(args...)  cterr_add_env_dump_exec(args,END_LIST)
#define cterr_add_debug(args...)     cterr_add_debug_exec(args, END_LIST)

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

#define FRU_INFO_INVALID     0xFF


/******** History ******** 
$Log: error.h,v $
Revision 1.2  2015/05/25 03:59:10  steja
Add Support Skye SM

Revision 1.1.4.2  2015/04/29 11:36:25  steja
Code check-in to skye-branch2 for ER code review


------------------------------------------------------------------
Revision 1.1.2.2  2014/08/22 04:58:47  palin2
First check-in to enhance Skye error message.

Revision 1.1.2.1  2014/07/21 01:56:37  palin2
Initial check-in Skye module side Diag code.

$Endlog$
*/
