/* $Id: sh.h,v 1.3 2014/02/18 09:11:12 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/sh.h,v $
 *------------------------------------------------------------------
 *
 * Copyright (c) 2007-2014 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

/*
** Defines for the monitor shell.
*/
#include "setjmps.h"
#define MAXARG 16
#define ARGVBUFSIZ 1600

#define MOREKLUDGE 1

struct shstuff {
  char *cmdptr;  /* command buffer pointer */
  char *argvptr;  /* argv buffer pointer */
  int argc;  /* argument count */
  char *argv[MAXARG+1];  /* array of argv buffer pointers */
  jmp_buf shparserrjmp;  /* jump buffer for shell parse errors */
  int retval;  /* the return value from a program */
  char argvbuf[ARGVBUFSIZ];  /* the actual argument buffer */
  char aliasflag, loadonly;
};

typedef enum {
  W_REG,     /* a regular word with no special meaning */
  W_PIPE,    /* the shell pipe character */
  W_BKGRND,  /* the shell background character */
  W_ENDCMD,  /* the shell command terminator character */
  W_REDOUT,  /* the shell redirect output character */
  W_APPEND,  /* the shell redirect output append condition */
  W_REDIN,   /* the shell redirect input character */
  W_EOLN,    /* end of a command line */
  W_EOF,     /* end of file condition */
  W_VOID,    /* void word */
} SH_TOKEN;  /* shell token */

typedef enum {
    NEUTRAL,     /* start of word */
    GRTRTHAN,    /* one '>' char encountered */
    INSQUOTE,    /* in single quoted string */
    INDQUOTE,    /* in double quoted string */
    INSHVAR,     /* in a shell variable word */
    INBRACES,    /* in braces '{}' */
    INWORD,      /* within a word */
    INCOMMENT,   /* in a comment string */
    INPIPELINE,  /* in a pipeline */
} SH_STATE;

/* monitor.c */
extern int invoke(struct shstuff *shp);

/* shcmd.c */
extern void shcmdline(struct shstuff *shp);

/* parsetoken.c */
extern SH_TOKEN parsetoken(struct shstuff *shp, SH_STATE state);

/******** History ******** 
$Log: sh.h,v $
Revision 1.3  2014/02/18 09:11:12  alpeng
CSCul88171-3: remove useless files: mon_boot.h, c82576_ethmap.h, eth_frames.c, stack.h, sys_regs.h

Revision 1.2  2012/03/28 00:38:12  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:04  ptong
Initial archive of ng_diag module


$Endlog$
*/
