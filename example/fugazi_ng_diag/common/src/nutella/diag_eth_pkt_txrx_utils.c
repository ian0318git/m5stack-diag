/* $Id: diag_eth_pkt_txrx_utils.c,v 1.4 2019/07/11 12:31:27 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nutella/diag_eth_pkt_txrx_utils.c,v $
 *-----------------------------------------------------------------------------
 * File: eth_pkt_txrx_utils.c
 * Description: contain functions are related to utility 
 * Aug 2015, Alan Peng
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>  /* va_list */
#include "diag_eth_pkt_txrx.h"

extern int pkt_deb_flag;

/*
 ************************************************************************
 * Function: swap32
 * 32 bit value swap. 
 *
 * Input: i - for data 
 *
 * Return: swapped data 
 ************************************************************************
 */
unsigned int swap32 (unsigned int ia)
{
    ia = (ia << 16) | (ia >> 16);

    return (ia & 0xff00ffff) >> 8 | (ia & 0xffff00ff) << 8;
}

/*
 ************************************************************************
 * Function: crc32
 * Ethernet packet 4 byte CRC calculation
 *
 * Input: crc - a value of crc
 *        data - packet data pointer to calculate crc
 *        len - packet length 
 *
 * Return: crc - return crc
 ************************************************************************
 */
unsigned int crc32 (unsigned int crc, unsigned char *data, int len)
{
    int ib;
    int              crc_table_inited = 0;
    unsigned int     crc_table[256];

    if (!crc_table_inited) {
        int jb;
        unsigned int accum;

        for (ib = 0; ib < 256; ib++) {
            accum = ib;

            for (jb = 0; jb < 8; jb++) {
                if (accum & 1) {
                    accum = accum >> 1 ^ 0xedb88320UL;
                } else {
                    accum = accum >> 1;
                }
            }

            crc_table[ib] = swap32(accum);
        }

        crc_table_inited = 1;
    }

    for (ib = 0; ib < len; ib++) {
        crc = crc << 8 ^ crc_table[crc >> 24 ^ data[ib]];
    }

    return crc;
}

/*
 ************************************************************************
 * Function: display_pkt
 * Packet display for help debugging
 *
 * Input: b_ptr - buffer porinter
 *        pktlen - packet length 
 *
 * Return: 0 when matched
 ************************************************************************
 */
void display_pkt (unsigned char *b_ptr, int pktlen)
{
    int ic, len;
    len = pktlen;

    printf("%s- packet: showing %d bytes\n", __FUNCTION__, len);
    for (ic = 0; ic < len; ic++) {
        if ((ic > 0) && ((ic % 16) == 0)) {
            printf("\n");
        }
        printf("%02x ", *b_ptr++);
    }
    printf("\n");
}

/*
 ************************************************************************
 * Function: chk_macaddr
 * Compare if 2 mac addresses matches
 *
 * Input:
 * macaddr1 and macaddr2 - 2 mac addresses to be compared
 *
 * Return: 0 when matched
 ************************************************************************
 */
int chk_macaddr (unsigned char *macaddr1, unsigned char *macaddr2)
{
    return(memcmp(macaddr1, macaddr2, 6));
}

/************************************************************************
 *
 * Function: eth_pkt_txrx_usage
 *
 * Description : the usage of packet transfer
 *
 * Input: None
 *
 * Output: None
 ************************************************************************
 */
void eth_pkt_txrx_usage (void) {

    printf("usage: \n"
           " \n"
           " -i [interface name] : necessary, e.g. eth1, xaui0 \n"
           " -c [packet count]   : optional, default is 100, e.g. -c 50\n"
           " \n"
           " example for sending 20 packets on eth3 \n"
           "./eth_pkt_txrx -i eth3 -c 20\n");

}

/************************************************************************
 *
 * Function: print_debug_msg
 *
 * Description : based on debug flag to display the msg. 
 *
 * Input:  str - user defined msg 
 *
 * Output: None
 ************************************************************************
 */
void print_debug_msg (const char * format, ...)
{ 
    va_list args;

    if (pkt_deb_flag == TRUE) {
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
    }

#if 0
char *str1, unsigned long base, unsigned long addr,
                  unsigned int line, char *f)
    unsigned int *ptr32;
    if (f)
        printf("file %s", f);

    if (line)
        printf("line %d;  ", line);

    printf("\n");

    printf("\n");

    if (addr < base) {
        printf("line %d; file %s", line, f);
        printf("addresses not valid; base @%#lx; device @%#lx\n",
               base, addr);
        fflush(stdout);
        exit(0);
    }
    ptr32 =  (unsigned int *)addr;
    printf("%s @%#x = %#x;  ", str1, (unsigned int)(addr-base), *ptr32);
#endif 

    fflush(stdout);
}
/*---------------------------------------------------------------
$Log: diag_eth_pkt_txrx_utils.c,v $
Revision 1.4  2019/07/11 12:31:27  alicehua
Collapse Nutella codes into main trunk.

$Endlog$
*/
