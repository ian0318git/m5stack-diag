/* $Id: monitor.c,v 1.7 2014/08/18 22:15:14 yuetwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/monitor.c,v $
 *------------------------------------------------------------------
 *
 * Copyright (c) 2007-2014 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include "endians.h"
#include "types.h"
#include "sh.h"
#include "pcmap.h"
#include "monitor.h"
#include "nvsysvars.h"
#include "error.h"
#include "proto.h"
#include "dnld.h"
#include "menu.h"
#include "flash.h"
#include "testmem.h"
#include "signals.h"
#include "dev_print.h"
#include <string.h>
#include <stdlib.h>
#include "cli_cmd.h"
#include "common.h"

extern int netflashbooted;
extern void return_to_roms(void);
extern int r4k_cachetest();
extern int progauth();
extern void alarm(int);

volatile unsigned char envflag, hkeepflags;

jmp_buf monjmpbuf, *monjmpptr;

int history(), repeat(), help(), gdb_cntrl();

extern int testleds();
extern int dumperrlog();
extern int diag_menu(int argc, char *argv[]);

unsigned short g_monhistcount;  /* monitor valid history count */
struct cmdhist *g_curhistptr;   /* current history slot */
struct cmdhist commandhistory[NUMMONHIST];

static char *repptr;
static uint repcnt;
jmp_buf monjmp;
#ifdef DEBUG
static struct cmdhist *getlohist();
#endif
static char *monrc = "$MONRC";

/*
**  Get a command line from the user and maintain the history mechanism.
**  The monitor prompt is taken from the monitor "shell" variable PS1 if
**  it exists.  If this variable contains a '!' character the history count
**  will replace this in the prompt.
*/
void
monitor(int reset)
{
    register int i;
    register char c, *strptr;
    struct shstuff shstuff;
    register struct shstuff *shp = &shstuff;
    register struct nvram *nvptr = NVRAM;
    struct cmdhist *chp;
    char *ptr = 0;
    
    initsigs();  /* set signal mechanism to default */
    envflag = INMON;
    if(reset) {
	if(!setjmp(monjmp)) {           /* allow for <break> */
	    monjmpptr = &monjmp;
  	    if((shp->cmdptr = getvar(nvptr->evartbl, monrc+1))) {
		shcmdline(shp);         /* execute it */
	    }
	}
	monjmpptr = 0;
    }

    chp = CURHISTPTR;  /* get pointer from last execution of monitor */
    if(chp) {  /* something in history */
	if((i = MONHISTCOUNT) > NUMMONHIST) {
	    /*
	     ** Renumber the items in the history buffer and reset
	     ** monhistcount.  Strange things will happen if monhistcount
	     ** is allowed to roll over to zero.
	     */
	    MONHISTCOUNT = i = (NUMMONHIST - 1);
	    while(i) {
		if(--chp < CMDHIST)
		    chp = &CMDHIST[NUMMONHIST - 1];  /* wraparound */
		chp->histnum = --i;
	    }
	    chp = CURHISTPTR;  /* get pointer again */
	}
    } else MONHISTCOUNT = 1;  /* nothing in history, init this */
    while(1) {  /* prompt for commands forever */
	while(1) {  /* do all history slots */
	    envflag = INMON;
	    if(!chp || chp >= &CMDHIST[NUMMONHIST])
		chp = CMDHIST;  /* init or wraparound */
	    CURHISTPTR = chp;  /* update for next execution of monitor */
	    chp->histnum = MONHISTCOUNT;  /* fill in history number */
	    /* in case user <breaks> from entering command line */
	    if(setjmp(monjmp)) putchar('\n');
	    monjmpptr = &monjmp;
		strptr = "diagmon ! > ";  /* if PS1 is not set, use this */
	    while((c = *strptr++)) {  /* prompt the user */
		if(c == '!') printf("%d",MONHISTCOUNT);  /* print hist num */
		else putchar(c);
	    }
	    switch((type_t)get_line(chp->cmdbuf,MONLINESIZ)) {
	    case -1:  /* buffer overflow, do not execute command */
		puts("command NOT executed\n");
		continue;
	    case 0:   /* empty buffer, reuse this line */
		continue;
	    default:
                ptr = strchr(chp->cmdbuf, '\n');
                if (ptr) {
                    *ptr = '\0';
                }
		repcnt = 0;
		/* set parser pointer to our command */
		shp->cmdptr = chp->cmdbuf;
		shcmdline(shp);  /* parse and execute the command */
		if ( repcnt ) {
		    strcpy(chp->cmdbuf, repptr);
		    printf("%s", repptr);
		    if ( repcnt > 1 ) 
			printf(", repeating %d times\n", repcnt);
		    else
			printf("\n");
		    while ( repcnt > 0 ) {
			/* set parser pointer to our command */
			shp->cmdptr = chp->cmdbuf;
			shcmdline(shp);  /* parse and execute the command */
			repcnt -= 1;
		    }
		}
		break;
	    }
	    MONHISTCOUNT++;
	    chp++;
	}
    }
}

/*
** See if the command is valid and execute it.
*/
int
invoke(struct shstuff *shp)
{
    register int i;
    register struct monitem *monptr;
    extern int optind;
    char buffer[32];
    int retval = -1;  /* bad return value (pessimistic?) */
    char *reason_str = (char *)0;  /* init */

    /* see if this is a shell variable assignment command */
    while(strchr(shp->argv[0],'=')) {  /* do possible multiple assignments */
	retval = setvar((NVRAM)->evartbl,EVTSIZ,shp->argv[0]);
	shift(1,&shp->argc,shp->argv); /* shift this word out of the picture */
	if(!shp->argc) return(retval); /* nothing left in command buffer */
    }
    initsigs();  /* initialize all signals to default */
    hkeepflags &= H_MORE;  /* reset all bits but H_MORE */
    optind = 0;  /* init for getopt() */
    strcpy(testnamebuf,"???");  /* initialize */
    switch(setjmp(monjmp)) { /* set up a trap door */
    case 0:
	monjmpptr = &monjmp;
	/* see if the command is a built-in (in the moncmd list) */
	for(i=0, monptr=moncmd; i<moncmdsiz; i++, monptr++) {
	    if(strcmp(shp->argv[0],monptr->command) == 0) {  /* match */
		shp->retval = (*monptr->procedure)(shp->argc,shp->argv);
		sprintf(buffer,"?=%d",shp->retval);
		setvar((NVRAM)->evartbl,EVTSIZ,buffer);
		if(getvar((NVRAM)->evartbl,"XCODE")) {
		    sprintf(buffer,"exited with code 0x%x",shp->retval);
		    reason_str = buffer;
		}
		retval = 0;  /* a good return value */
		goto exitinvoke;
	    }
	}
	reason_str = "not found";
	break;
    case 1:
	reason_str = "aborted due to user interrupt";
	break;
    case 2:
	reason_str = "aborted due to exception";
	break;
    case 3:
	break;  /* quiet termination */
    case 4:
	reason_str = "aborted due to alarm signal";
	break;
    default:
	reason_str = "terminated - reason unknown";
	break;
    }
    shp->retval = 2;  /* command not allow to finish, bad return value */
    /* should this be recorded in evartbl??? */
exitinvoke:
    hkeepflags &= ~H_MORE;  /* H_MORE good for one command only */
    alarm(0);  /* kill alarm if set */
    if(reason_str) {
	printf("\nmonitor: command \"%s\" %s\n",shp->argv[0],reason_str);
	repcnt = 0;
    }
    return(retval);
}

/*
** Print out the history list using bash command.
** Prints 1 less than NUMMONHIST including the current item.
*/
int
history(int argc, char *argv[])
{
    /* using bash to support history command directly. */ 
    system("history");
    return(0); 
}

/*
** Repeat a specified command or last command using bash command.
** The argument can either be a history number or a string to match.
*/
int
repeat(int argc, char *argv[])
{
    /*using bash command to support repeat command directly. */
    system("!!");
    return(0);
}

/*
** Shift the arguments in an argv[] array to replace 0 with 1, 1 with 2,
** etc.
*/
void
shift(int count, int *argcp, char *argv[])
{
    register int i;
    
    if(count >= *argcp) {  /* count is >= number of arguments */
	*argcp = 0;
	argv[0] = (char *)0;
    } else {  /* count is less than the number of arguments */
	*argcp -= count;
	for(i=0; i <= *argcp; argv[i++] = argv[count++]);
    }
}

int
help()
{
    register int i, numchars;
    register struct monitem *monptr;
    
    for(i=0, monptr=moncmd; i<moncmdsiz; i++, monptr++) {
	numchars = printf("%s  ",monptr->command);
	while(numchars++ < 20) putchar(' ');
	printf("%s\n",monptr->description);
    }
    return(0);
}

int
gdb_cntrl(int argc, char *argv[])
{
    printf("%s not supported on diaglinux\n", __FUNCTION__);
    return(0);

    if ( ! netflashbooted ) {
	printf("gdb is not supported in the ROM based image\n");
	return(1);
    }

    return(1);
}

int
rom_reload()
{
    printf("%s not supported\n", __FUNCTION__);
    return 0;
}

/*
 ********************************************************
 * Function: show_diag_ver
 *
 * This function show the version info of a diagnostics
 * image, eg. time created, diags version ... etc
 *
 * Input: None
 *
 * Output: Return an integer type to compliant with
 *       monitem structure, to avoid compilation
 *       warning.
 ********************************************************
 */
int
show_diag_ver (void)
{
    printf("%s not supported\n", __FUNCTION__);
    return 0;
}


/******** History ******** 
$Log: monitor.c,v $
Revision 1.7  2014/08/18 22:15:14  yuetwang
add engineering banner

Revision 1.6  2013/11/26 08:40:34  hroni
fix compiler warning

Revision 1.5  2012/06/05 09:33:45  aarwang
- Clean up compiler warnings.

Revision 1.4  2012/04/18 07:20:41  alpeng
remove unsupported CLI commands: sync, unset, alias, gdb, and unalias

Revision 1.3  2012/04/10 09:41:17  alpeng
support CLI cmd:disflag, setflag, repeat and history

Revision 1.2  2012/03/28 00:38:14  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:05  ptong
Initial archive of ng_diag module


$Endlog$
*/
