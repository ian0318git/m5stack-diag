/* $Id: stub.c,v 1.2 2018/05/18 09:24:58 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/cavium/stub.c,v $
 *------------------------------------------------------------------
 * File: ovld_linux_stub.c
 *
 * Copyright (c) 2011-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
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

void msleep(unsigned long t)
{
    usleep(t*1000);
}

void mdelay(unsigned long t)
{
    usleep(t*1000);
}

void udelay(unsigned long t)
{
    usleep(t);
}

int get_line(char *ptr, unsigned int size)
{
    int i;
    unsigned int len = 0;
    fgets(ptr, size, stdin);
    len = strlen(ptr);
    for (i=0;i<2;i++) {
        if (len) {
            if (ptr[len - 1] == '\r' || ptr[len - 1] == '\n') {
                ptr[len - 1] = '\0';
                len--;
            }
        }
    }

    return len;
}

/*-------------------------------------------------
$Log: stub.c,v $
Revision 1.2  2018/05/18 09:24:58  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.1.2.1  2016/06/06 05:58:51  xiaoyizh
Initial Check-in for Neptune Data Plane diags.

Revision 1.3  2013/02/15 01:09:50  ptong
Stop using common/src/linux_api.c in Cavium Linux

Revision 1.2  2012/03/28 00:38:19  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:07  ptong
Initial archive of ng_diag module


$Endlog$
*/
