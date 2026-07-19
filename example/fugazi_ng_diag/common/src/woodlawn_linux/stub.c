/* $Id: stub.c,v 1.2 2013/10/08 08:48:32 tirawan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn_linux/stub.c,v $
 *------------------------------------------------------------------
 * File: woodlawn_linux_stub.c
 *
 * Copyright (c) 2011-2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include "types.h"

/*
** Return the value for the ascii hex character or -1 if invalid.
*/
char
atoh(char c)
{
  if(c >= '0' && c <= '9') return(c - '0');
  if(c >= 'A' && c <= 'F') return(c - ('A' - 10));
  if(c >= 'a' && c <= 'f') return(c - ('a' - 10));
  return(-1);
}

int
getnnum(char *cptr, int base, utype_t *longret, int maxchars)
     /*cptr : character buffer pointer */
     /*longret : for the result */
{
  char cval;
  unsigned long value = 0; /* init */
  int count = 0; /* init */

  while(1) {
    cval = atoh(*cptr);
    if(cval < 0 || cval >= base) break;  /* invalid character encountered */
    value = (value * base) + cval;
    cptr++;
    count++;
    if(maxchars && count == maxchars) break;
  }
  *longret = value;  /* place result */
  //  printf("%s %d %p %p\n", __FILE__, __LINE__, value, *longret);    
  return(count);
}

/*
** Convert the ascii string pointed to by cptr to binary according to base.
** Result is placed in *longret.
** Return value is the number of characters processed.
** Maxchars defines the maximum number of characters to process.  If
** maxchars == 0, process until an invalid character occurs.
** Getnum exists for historical reasons.
*/
int
getnum(char *cptr, int base, utype_t *longret)
     /*cptr : character buffer pointer */
     /*longret : for the result */
{
  return(getnnum(cptr, base, longret, 0));
}

/*
 * Function: timer_calibrate 
 *
 * Just return the value back to the caller.
 * NOTE: The functions wastetime and msleep performs their own calibration.
 */
long
timer_calibrate (long t) 
{
    return(t);
}


/*-------------------------------------------------
 * $Log: stub.c,v $
 * Revision 1.2  2013/10/08 08:48:32  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.4.2  2013/08/20 10:59:11  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.1  2013/04/24 10:37:25  tirawan
 * Initial check-in for woodlawn linux code
 *
 * Revision 1.2  2013/03/27 08:45:06  kuangik
 * Code cleanup
 *
 * Revision 1.1  2013/03/13 06:43:02  kuangik
 * Add for the first time
 *
 * Revision 1.4  2012/08/03 10:16:56  evanli
 * Mapping to latest O2 source code on 20120726
 *
 * Revision 1.2  2012/07/23 07:39:43  leslie
 * Remove print_64bit_hex function.
 *
 * Revision 1.1.1.1  2012/02/10 05:59:50  kody
 * Initial imports Woodlawn project code base.
 *
 * Revision 1.1.2.1  2011/04/05 19:59:39  ptong
 * Initial checkin
 *
 * $Endlog$
 *-------------------------------------------------
 */
