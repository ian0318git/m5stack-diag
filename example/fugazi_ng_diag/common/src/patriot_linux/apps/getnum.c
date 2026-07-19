/* $Id: getnum.c,v 1.1 2014/03/25 02:12:33 huanngo Exp $
 * $Source: 
 *------------------------------------------------------------------
 *
 * Copyright (c) 2007-2014 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */


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

/*
** Convert the ascii string pointed to by cptr to binary according to base.
** Result is placed in *longret.
** Return value is the number of characters processed.
** Maxchars defines the maximum number of characters to process.  If
** maxchars == 0, process until an invalid character occurs.
** Getnum exists for historical reasons.
*/
int
getnum(char *cptr, int base, unsigned int *longret)
     /*cptr : character buffer pointer */
     /*longret : for the result */
{
  return(getnnum(cptr, base, longret, 0));
}

int
getnnum(char *cptr, int base, unsigned int *longret, int maxchars)
     /*cptr : character buffer pointer */
     /*longret : for the result */
{
  char cval;
  unsigned int value = 0; /* init */
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
  return(count);
}

/* End of File */
/*------------------------------------------------------------------------------
 * $Log: getnum.c,v $
 * Revision 1.1  2014/03/25 02:12:33  huanngo
 * Adding patriot_linux directory to ng_diag code tree
 *
 * Revision 1.2  2012/05/08 23:52:54  huanngo
 * Support SM Patriot on ngd main code tree
 *
 * Revision 1.1.4.2  2011/08/18 19:43:23  huanngo
 * Update code to patriot2-branch
 *
 *------------------------------------------------------------------------------
 * $Endlog$
 *
 *------------------------------------------------------------------------------
 */

