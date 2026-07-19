/* $Id: error.h,v 1.7 2013/12/12 01:25:51 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/error.h,v $
 *------------------------------------------------------------------
 *
 * Copyright (c) 2007-2012, 2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 * Author: clev
 *------------------------------------------------------------------
 */
#ifndef _ERROR_H_
#define _ERROR_H_

#define TESTNAMEBUFSIZ 160

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
extern int dumperrlog();
extern void dump_n_flush();
extern void clrtestname();
extern void cterr(char, int, char*, ...);
extern void testname(char *, ...);
extern void prpass(int, char *, ...);
extern void prcomplete(int, int, char *, ...);

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

extern unsigned int cterr_db_print(char *fmtptr, ...);
extern void set_enhance_err_flag(int flag);
extern int get_enhance_err_flag(void);
#endif /* _ERROR_H_ */

/******** History ******** 
$Log: error.h,v $
Revision 1.7  2013/12/12 01:25:51  mcharon
default enhance flag to 0 for now

Revision 1.6  2013/12/12 01:23:23  mcharon
add set/get method to turn on/off advance err flag used for debugging

Revision 1.5  2013/12/04 18:53:04  mcharon
externs  functions so compile will work

Revision 1.4  2013/10/08 11:03:47  erwu2
enhanced err msg first check-in

Revision 1.3  2012/09/18 19:18:06  mcharon
 add clrtestname

Revision 1.2  2012/03/28 00:38:10  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:03  ptong
Initial archive of ng_diag module


$Endlog$
*/
