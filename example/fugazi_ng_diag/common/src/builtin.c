/* $Id: builtin.c,v 1.4 2012/06/05 09:33:43 aarwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/builtin.c,v $
 *------------------------------------------------------------------
 *
 * Copyright (c) 2007-2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

/*
** Some monitor built-in commands.
*/

//#include "endians.h"
#include "types.h"
#include "sh.h"
#include "pcmap.h"
#include "nvsysvars.h"
#include "proto.h"
#include "strings.h"
#include <stdio.h>
#include <string.h>

#ifdef NATIVE
#include "fcntl.h"  /* for O_WRONLY and O_RDONLY only */
#endif
extern void do_reset (void);
/*
** Display the command line arguments.
*/
int
echo(int argc,char *argv[])
{
  register int i;
  register int c;
  register char *cptr;
  long temp;
  enum {NEUTRAL, BACKSLASH, OCTNUM} state = NEUTRAL;

  for(i=1; i<argc; i++) {
    cptr = argv[i];
    while((c = *cptr++)) {
      switch(state) {
      case NEUTRAL:
	if(c == '\\') {
	  state = BACKSLASH;
	  continue;
	}
	break;
      case BACKSLASH:
	if(c >= '0' && c <= '7') {
	  --cptr;  /* unget the char */
	  state = OCTNUM;
	  continue;
	}
	switch(c) {
	case 'n': c = '\n'; break;  /* newline */
	case 'b': c = '\b'; break;  /* backspace */
	case 'f': c = '\f'; break;  /* formfeed */
	case 'r': c = '\r'; break;  /* return */
	case '\\': break;           /* backslash */
	case 'c': return(0);           /* end of line, no newline */
	}
	break;
      case OCTNUM:
	cptr += getnnum(cptr, 8, (utype_t *)&temp, 3);
	c = temp;
	break;
      }
      putchar(c);
      state = NEUTRAL;
    }
    putchar(' ');  /* put a space between argv's */
  }
  putchar('\n');
  return(0);
}

/*
** These routines provide the MAN monitor shell with an environment
** variable mechanism that closely resembles that of the Bourne shell
** and an alias mechanism.  The major difference is that our environment
** variables have no attributes.  These variables are stored in a table
** in non-volatile RAM as a series of NULL terminated strings.  The first
** string is a variable name, the second is that variables value, the third
** is the next variable name, and so on.  A null variable name terminates
** the table.  A separate table is used for environment variables and
** aliases.
*/
int
unset(int argc, char *argv[])
{
  register int i;
  char *tblptr;

  printf("%s not supported on diaglinux\n", __FUNCTION__);
  return(0);

  if(argc < 2) {
    printf("usage: %s name1 [name2 ...]\n",argv[0]);
    return(1);
  }
  if(strcmp(argv[0],"unalias") == 0) tblptr = ALIAS_TABLE;
  else tblptr = (NVRAM)->evartbl;
  for(i=1; i<argc; i++) {
    if(unsetvar(tblptr,argv[i]) < 0)
      printf("%s: \"%s\" does not exist\n",argv[0],argv[i]);
  }
  return(0);
}

/*
** This routine is called by both "set" and "alias" commands.
** Display the appropriate table for the user or, if invoked as "alias"
** with arguments, setvar them.
*/
int
setalias(int argc, char *argv[])
{
  printf("%s not supported on diaglinux\n", __FUNCTION__);
  return(0);
 
  register char *tblptr;

  if(strcmp(argv[0],"alias") == 0) {  /* invoked as alias? */
   if(argc > 2) {
alsusage:
      printf("usage: %s [name=value]\n",argv[0]);
      return(1);
    }
    if(argc == 2) {
      if(setvar(ALIAS_TABLE, ALSSIZ,argv[1]) == 0) return(0);
      else goto alsusage;
    }
    tblptr = ALIAS_TABLE;
  } else { 
   if(argc > 1) {
      printf("usage: %s\n",argv[0]);
      return(1);
    }
    tblptr = (NVRAM)->evartbl;
  }
  printtbl(tblptr);
  return(0);
}

int
msleep_cmd(int argc, char *argv[])
{
    uint msecs;

    if(argc != 2 || !getnum(argv[1],10, &msecs)) {
	printf("usage: %s milliseconds\n", argv[0]);
	return(1);
    }
    msleep(msecs);
    return(0);
}

#ifdef NATIVE
/*
** Concatenate all files specified on the command line or dump them as
** ascii hex.  The action depends on argv[0] being cat or fdump.
*/
int
cat(int argc, char *argv[])
{
  register i, num, fd;
  char dump = 0;  /* dump flag */
  int daddr, retval = 0;
  char buffer[32];

  if(strcmp(argv[0],"fdump") == 0) dump = 1;
  if(argc < 2) {
usage:
    printf("usage: %s filename1 [filename2...]\n",argv[0]);
    return(1);
  }
  for(i=1; i<argc; i++) {
    daddr = 0;  /* reset for new file */
    if((fd = open(argv[i],O_RDONLY)) < 0) {
      printf("cannot open \"%s\"\n",argv[i]);
      return(1);
    }
    while(1) {
      filbyte(buffer,sizeof(buffer),0);
      if((num = read(fd,buffer,sizeof(buffer))) == 0) break;
      if(num < 0) {
	printf("%s: read error...aborting\n", argv[0]);
	retval = 1;
	break;
      }
      if(dump) {
	dismem(buffer,num,daddr,1);  /* display as bytes */
	daddr += num;
      } else puts(buffer);
      if(num != sizeof(buffer)) break;
    }
    close(fd);
  }
  return(retval);
}
#endif

int
reset(int argc, char *argv[])
{
#ifdef LINUX_APP
    printf("do reset from linux prompt\n");
#else
    
#ifdef NATIVE
    register fd;
  
#include <fcntl.h>

    if((fd = open(".nvram", O_CREAT | O_WRONLY | O_TRUNC, 0700)) < 0) {
	printf("can't open \".nvram\" file\n");
	exit(1);
    }
    if(write(fd,NVRAM,sizeof(struct nvram)) < sizeof(struct nvram)) {
	printf("write on file \".nvram\" failed\n");
    }
    close(fd);
    exit(0);
#else
    if(argc > 1) {
	if(strcmp(argv[1], "-s") == 0) savenv();  /* save our environment */
	else {
	    printf("usage: %s [-s]\n", argv[0]);
	    return(1);
	}
    }
    do_reset();  /* call asm routine */
#endif NATIVE
#endif /* linux_app */
    return 0;
}

/******** History ******** 
$Log: builtin.c,v $
Revision 1.4  2012/06/05 09:33:43  aarwang
- Clean up compiler warnings.

Revision 1.3  2012/04/18 07:20:41  alpeng
remove unsupported CLI commands: sync, unset, alias, gdb, and unalias

Revision 1.2  2012/03/28 00:38:13  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:05  ptong
Initial archive of ng_diag module


$Endlog$
*/
