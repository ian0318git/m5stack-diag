/* $Id: eth_pkt_utils.c,v 1.6 2012/09/06 21:29:37 ptong Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/eth_pkt_utils.c,v $
 *------------------------------------------------------------------
 *
 * eth_pkt_utils.c - Common ethernet packet utilities
 *
 * Jan 2012, Paul Tong
 *
 * Copyright (c) 2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  /* getpid */
#include <strings.h>  /* for bzero*/
#include <string.h>
#include <errno.h>
#include <sys/types.h> /* getpid */
#include <sys/socket.h>
#include <features.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/ethtool.h> /*struct ethtool */
#include <linux/sockios.h> /* SIOCETHTOOL */
#include "types.h"
#include "common.h"
#include "error.h"
#include "router_if.h"
#include "sgmii_defs.h"
#include "plat_defs.h"

#undef DEBUG

static int		crc_table_inited;
static unsigned int	crc_table[256];

unsigned int
swap32(unsigned int i)
{
    i = (i << 16) | (i >> 16);

    return (i & 0xff00ffff) >> 8 | (i & 0xffff00ff) << 8;
}

/* Ethernet packet 4 byte CRC calculationg
 */
unsigned int
crc32(unsigned int crc, unsigned char *data, int len)
{
    int			i;

    if (!crc_table_inited) {
	int		j;
	unsigned int		accum;

	for (i = 0; i < 256; i++) {
	    accum = i;

	    for (j = 0; j < 8; j++) {
		if (accum & 1) {
		    accum = accum >> 1 ^ 0xedb88320UL;
		} else {
		    accum = accum >> 1;
		}
	    }

	    crc_table[i] = swap32(accum);
	}

	crc_table_inited = 1;
    }

    for (i = 0; i < len; i++) {
	crc = crc << 8 ^ crc_table[crc >> 24 ^ data[i]];
    }

    return crc;
}

/* Packet display to help debugging
 */
void display_pkt(unsigned char *b_ptr, int pktlen)
{
    int i, len;
    len = pktlen;
    printf("%s- packet: showing %d bytes\n", __FUNCTION__, len);
    for (i=0; i < len; i++) {
        if ((i>0) && ((i % 16) == 0)) {
	    printf("\n");
	}
	printf("%02x ", *b_ptr++);
    }
    printf("\n");
}

/* Convert a string of MAC address "xx:xx:xx:xx:xx:xx" to
 * 6 byte uchar numbers
 */
int macstr2macaddr(char *macstr, mac_addr_t *mac_buf)
{
    char tmp_mac[6];
    char *cptr, tmpstr[4];
    int ii, tmp_hex, count;

    ii = 0;
    count = 0;
    cptr = macstr;
    do {
      memset(tmpstr, 0, sizeof(tmpstr));
      memcpy(tmpstr, cptr, 2);
      count += sscanf((char *)tmpstr, "%x", &tmp_hex);
      tmp_mac[ii] = (uchar)tmp_hex;
      ii++;
      cptr += 3; /* point to next mac byte */
    } while(ii < 6);

    if (count == 6) {
        memcpy(mac_buf, tmp_mac, sizeof(mac_addr_t));
	return(0);
    }
    else {
        return(-1);
    }
}

/* Query function to let user to specify the components
 * of a ethernet packet
 */
void mac_addr_query(char *query_str, mac_addr_t *mac_addr)
{
    char buf[20];
    uchar *cptr;
    mac_addr_t new_mac;
    int len, rv;

    printf("%s", query_str);
    do {
        do {
	    printf("=> Enter MAC addr (enter nn:nn:nn:nn:nn:nn or hit return): ");
	    get_line(buf, sizeof(buf));
	    len = strlen((char *)buf);
	} while((len !=0) && (len != 17));

	if (len == 0) {
	    rv = 0;
	}
	else {
	    rv = macstr2macaddr(buf, &new_mac);
	    if (rv == 0) {
	        memcpy(mac_addr, &new_mac, sizeof(mac_addr_t));
	    }
	}
    } while (rv != 0);

    cptr = (uchar *)*mac_addr;
    printf("MAC address used is ("
	   "%02x:%02x:%02x:%02x:%02x:%02x)\n",
	   cptr[0], cptr[1], cptr[2], 
	   cptr[3], cptr[4], cptr[5]);
}

/******** History ******** 
$Log: eth_pkt_utils.c,v $
Revision 1.6  2012/09/06 21:29:37  ptong
Minor fix on a printf

Revision 1.5  2012/06/06 15:00:27  palin2
Clean up compiler warnings.

Revision 1.4  2012/06/06 09:57:24  iachang
Clean up complier warnings.

Revision 1.3  2012/05/17 22:48:15  ptong
Fixed macst2macaddr

Revision 1.2  2012/03/28 00:38:17  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:06  ptong
Initial archive of ng_diag module


$Endlog$
*/
