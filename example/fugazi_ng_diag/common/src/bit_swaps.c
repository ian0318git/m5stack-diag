/* $Id: bit_swaps.c,v 1.3 2013/05/02 17:27:48 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/bit_swaps.c,v $
 *------------------------------------------------------------------
 * bit_swaps.c -- Bit swapper array for flipping token ring mac addrs
 *                to ether order 
 *
 * Copyright (c) 2013 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */
#include <types.h>
#include <cookie_4.h>

//static ushort swap_num_bits(ushort , int);
/*************************************************************
 * Function: swapbyte
 * Description:  swap bits of a byte. for example, 0xF7 will become EF.
 * Input: c , byte to swap
 * Output: return byte that has been swapped.
 *************************************************************
 */
unsigned char
swapbyte (unsigned char c)
{
    int i ;
    unsigned char result=0;
        
    for(i=0;i<8;++i) {
        result=result<<1;
        result|=(c&1);
        c=c>>1;
    }
    return result;
}

/******** History ******** 
$Log: bit_swaps.c,v $
Revision 1.3  2013/05/02 17:27:48  mcharon
move ttf2array to linux_api.c

Revision 1.2  2012/03/28 00:38:13  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:04  ptong
Initial archive of ng_diag module


$Endlog$
*/
