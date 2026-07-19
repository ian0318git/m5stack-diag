/* $Id: shcmd.c,v 1.2 2012/03/28 00:38:15 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/shcmd.c,v $
 *------------------------------------------------------------------
 *
 * Copyright (c) 2007-2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include "endians.h"
#include "types.h"
#include "sh.h"
#include "pcmap.h"
#include "nvsysvars.h"
#include "proto.h"
#include <stdio.h>
#include <string.h>

static SH_TOKEN shcmd();

/*
** Parse and execute all shell commands on a command line.
*/
void
shcmdline(struct shstuff *shp)
{
  SH_TOKEN token, shcmd();

  if(setjmp(shp->shparserrjmp))
    return;  /* set up trap door for fatal command errors */
  while(1) {  /* do all commands on the line */
    shp->argc = 0;  /* init the argument count */
    shp->aliasflag = 1;  /* always check argv[0] for an alias */
    shp->argvptr = shp->argvbuf;  /* start at the beginning of the buffer */
    switch(token = shcmd(shp)) {  /* process all commands on line */ 
    case W_EOLN:
    case W_ENDCMD:
      shp->argv[shp->argc] = '\0';  /* terminate the argv's */
      if(shp->argc) invoke(shp);
      if(token != W_EOLN) break;  /* more commands on this line */
    case W_EOF:
      return;
    default:
        break;
    }
  }
}

/*
** Parse all of the arguments to a single command.  Handle aliases.
** The command may be terminated by a semi-colon, an end of line,
** redirection characters, etc..  The return value is a SH_TOKEN that
** reflects the terminator.  Error messages are printed for all
** unimplimented tokens.
*/
static SH_TOKEN
shcmd(struct shstuff *shp)
{
  SH_TOKEN token, parsetoken();
  char savealiasflag, *savecmdptr, *cmdtoalias, *alias();
  int saveargc;

  while(1) {  /* parse all of the args to a command */
    shp->argv[shp->argc] = shp->argvptr;
    switch(token = parsetoken(shp,NEUTRAL)) {
    case W_REG:  /* regular word with no special meaning to shell */
      if(shp->argc >= MAXARG) {
	puts("too many args\n");
	goto invalid;
      }
      if(shp->aliasflag) {  /* look for a possible alias */
	savecmdptr = shp->cmdptr;  /* save it for later */
	saveargc = shp->argc;  /* is this necessary */
	cmdtoalias = shp->argv[shp->argc];
	if((shp->cmdptr = getvar( ALIAS_TABLE,cmdtoalias))) {
	  /*
	  ** A space as the last character in the string means to check for
	  ** an alias on the next argv.
	  */
	  if(*shp->cmdptr == '\0' ||
	     *(shp->cmdptr + strlen(shp->cmdptr) - 1) != ' ')
	    savealiasflag = 0;  /* quit aliasing in the rest of this cmd */
	  else savealiasflag = shp->aliasflag;  /* continue aliasing */
	  /*
	  ** Aliases must be reprocessed by parsecommand.  Aliases can be
	  ** many words long and include shell variables which must be
	  ** resolved.  Aliases can also consist of several complete
	  ** commands as well as just a command name alias (I.E.
	  **        alias bigcmd='fill 0 100 55aa; dump 0 100; dump'
	  ** which has 2 complete commands and a command name).
	  */
	  shp->aliasflag = 0;  /* do not nest aliases */
	  while(1) {  /* possible multi cmds */
	    shp->argvptr = cmdtoalias;  /* point back to start of this argv */
	    if((token = shcmd(shp)) == W_EOLN) break;
	    if(token == W_ENDCMD) {  /* end of one command */
	      shp->argv[shp->argc] = '\0';  /* terminate it */
	      if(shp->argc) invoke(shp);  /* invoke it */
	      shp->argc = saveargc;  /* prepare for the rest of the alias */
	    } else {
	      printf("bad alias\n");
	      goto invalid;
	    }
	  }
	  shp->aliasflag = savealiasflag;
	  shp->cmdptr = savecmdptr;  /* restore it */
	  continue;
	} else {
	  shp->aliasflag = 0;  /* no alias found, discontinue aliasing */
	  shp->cmdptr = savecmdptr;  /* restore it */
	}
      }
      shp->argvptr++;  /* bump past the last string's terminator */
      shp->argc++;
      continue;
    case W_VOID:  /* word is void, reuse this argv buffer */
      continue;
    case W_REDIN:  /* redirect input */
      puts("Input redirection is not supported at this level\n");
      goto invalid;
    case W_REDOUT:
    case W_APPEND:
      puts("Output redirection is not supported at this level\n");
      goto invalid;
    case W_PIPE:
      puts("Multiple pipes are not supported at this level\n");
      goto invalid;
    case W_BKGRND:
      puts("Background jobs are not supported at this level\n");
invalid:
      shp->argc = 0;  /* invalidate all arguments */
    default:
      return(token);
    }
  }
}

/******** History ******** 
$Log: shcmd.c,v $
Revision 1.2  2012/03/28 00:38:15  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:05  ptong
Initial archive of ng_diag module


$Endlog$
*/
