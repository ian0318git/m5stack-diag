/* $Id: linux_stub.c,v 1.4 2013/05/09 19:25:17 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/linux_stub.c,v $
 *------------------------------------------------------------------
 *
 * Copyright (c) 2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <assert.h>
#include "common.h"
#include "types.h"
//#include "hwic_slot.h"

/*********************************************************************** 
*  Extern Functions Declaration 
************************************************************************/
extern void msleep(int);

void cache_flush(void)
{
    printf("!!!!\n\ncache_flush not supported\n\n");
}

void print_64bit_hex(void)
{
    printf("!!!!\n\nprint_64bit_hex: not supported\n\n");
}

int
disable_msi_intr(int slot, int check_pending_intr)
{
    return (PASSED);
}

int
enable_msi_intr(int slot, int check_pending_intr)
{
    return (PASSED);
}

void 
print_exception_info(void)
{
}

boolean
check_for_pending_msi_intr (int slot, int dummy)
{
    return (FALSE);
}


/*
int
show_margins_x (int dummy, boolean mode)
{
    printf("%s not supported \n", __FUNCTION__);
}
*/

void initsigs (void)
{

}

void setmore (void)
{
    printf("%s not supported \n", __FUNCTION__);
}
/*
** Return the value for the1;,.e ascii hex character or -1 if invalid.
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
 * Declare this function here temporarily to get past Diaglinux linking error.
 * We can remove this function when Guido is supported in Diaglinux since it's 
 * defined in guido_vic_tests.c
 */
#define VIC_FXO_CODEC_RESET_REG_OFFSET 0x3
#define SI3241_ENABLE 0x1
#define SI3241_RESET  0x0

int
toggle_vic3_fxs_reset (uchar *vwic_base_p)
{
    uchar *reset_control_bit;

    reset_control_bit = vwic_base_p + VIC_FXO_CODEC_RESET_REG_OFFSET;
    
    *reset_control_bit = SI3241_ENABLE;
    msleep(400);
    *reset_control_bit = SI3241_RESET;
    msleep(400);
    *reset_control_bit = SI3241_ENABLE;
    msleep(400);
    if (*reset_control_bit != SI3241_ENABLE) {
	return(1);
    } else {
	return(0);
    }
}

/* Montalvo is not supported in Diaglinux yet. Stub for now and remove them
 * when Montalvo is supported.
 */

/* These functions are stubbed for now until ISM is supported */
void
print_ism_slot_x(boolean mode)
{
    printf("print_ism_slot_x() are stubbed in linux_main.c for now\n");
}
