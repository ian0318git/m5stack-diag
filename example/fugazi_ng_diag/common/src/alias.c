/* $Id: alias.c,v 1.2 2012/03/28 00:38:13 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/alias.c,v $
 *------------------------------------------------------------------
 *
 * Copyright (c) 2007-2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

/*
** String table routines.
** Some of the monitor parameters are stored as strings in tables (i.e.
** monitor environment variables and aliases).  The following routines
** provide table access.
*/

#include "endians.h"
#include "types.h"

#include "pcmap.h"
#include "nvsysvars.h"
#include "proto.h"
#ifdef LINUX_APP
#include <string.h>
#include <setjmp.h>
#include <stdio.h>
#else
#include "strings.h"
#include "setjmps.h"
#endif
static char *endtbl(), *eliminate();

/*
** Search the variable table for the name pointed to by vname.  Return a
** pointer to the value string for that name.
*/
char *
getvar(char *tblptr, char *vname)
{
  register int found = 0;

  while(1) {
    if(!(*tblptr)) break;  /* NULL name means end of our table */
    if(strcmp(vname,tblptr) == 0) found = 1;  /* name matches */
#ifdef DEBUG
    printf("getvar: comparing '%s' with '%s'\n",vname,tblptr);
#endif
    tblptr = strchr(tblptr, '\0') + 1;  /* bump pointer past name */
#ifdef DEBUG
    printf("getvar: var value is '%s'\n",tblptr);
#endif
    if(found) return(tblptr);
    tblptr = strchr(tblptr, '\0') + 1;  /* bump pointer past value */
  }
  return((char *)0);
}

/*
** Set a variable to a value.  The variable name may be new or may already
** exist in the table in which case the old value will be clobbered.
** Strptr must point to a string of the form "varname=varval" with no
** whitespace.
*/
int
setvar(char *tblptr, int tblsiz, char *strptr)
{
  register char *tptr;  /* our temporary pointer */
  register char *vptr;  /* our value pointer */

  if(!(vptr = strchr(strptr,'='))) return(-1);  /* there must be a '=' */
  /* null out the '=' and point to the second string */
  *vptr++ = '\0';
  /* if this item is already in our table remove it (eliminate()) */
  if((tptr = getvar(tblptr,strptr))) tptr = eliminate(tblptr,tptr);
  else tptr = endtbl(tblptr);  /* tptr now points to the end of the table */
  /* add the item to the end of the table if there is room */
  if((unsigned int)(tblsiz - (tptr - tblptr)) <
     (unsigned int)(strlen(strptr) + strlen(vptr) + 3)) {
    printf("out of room in table, cannot set \"%s\"\n",strptr);
    return(-1);
  }
  strcpy(tptr,strptr);  /* copy the variable name */
  tptr = strchr(tptr,'\0') + 1;  /* move pointer past variable name */
  strcpy(tptr,vptr);  /* copy the variable value */
  tptr = strchr(tptr,'\0') + 1;  /* move pointer past variable value */
  *tptr = '\0';  /* terminate the table */
  return(0);
}

/*
** Unset a variable.  This is different from setting it to a null string.
** This deletes the variable name completely.
*/
int
unsetvar(char *tblptr, char *var)
{
  char *vptr;

  if((vptr = getvar(tblptr,var))) {
    (void)eliminate(tblptr,vptr);
    return(0);
  }
  return(-1);
}

/*
** Print out for the user the entire table.
*/
int
printtbl(char *tblptr)
{

    while(*tblptr) {  /* NULL name means end of our table */
    printf("%s=",tblptr);
	tblptr = strchr(tblptr, '\0') + 1;  /* bump pointer past name */
	printf("%s\n",tblptr);
	tblptr = strchr(tblptr, '\0') + 1;  /* bump pointer past value */
    }
    return(0);
}

/*
** Return a pointer to the terminating NULL variable name.
*/
static char *
endtbl(char *tblptr)
{
  while(1) {
    if(!(*tblptr)) return(tblptr);  /* NULL variable name is terminator */
    tblptr = strchr(tblptr,'\0') + 1;  /* move pointer past variable name */
    tblptr = strchr(tblptr,'\0') + 1;  /* move pointer past variable value */
  }
}

/*
** Eliminate the evar, that is, move everything past the evar at ptr
** up to clobber that item.  Ptr must point to the beginning of the value
** portion (as returned from getvar).
*/
static char *
eliminate(char *tblptr, char *vptr)
{
  register char *nextptr;  /* pointer to next entry */

  /* vptr must be pointing at the value portion of an var */
  nextptr = strchr(vptr,'\0') + 1;  /* point nextptr past value portion */
  --vptr; while(*(--vptr));  vptr++;  /* back up vptr to the name portion */
  movbyte((unsigned char *)nextptr,(unsigned char *)vptr,
	  endtbl(tblptr)-nextptr+1);  /* copy terminator too */
  return(endtbl(tblptr));
}

/******** History ******** 
$Log: alias.c,v $
Revision 1.2  2012/03/28 00:38:13  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:04  ptong
Initial archive of ng_diag module


$Endlog$
*/
