/* $Id: hts_menu.h,v 1.2 2013/10/14 11:17:14 hroni Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/utah/hts_menu.h,v $
 *------------------------------------------------------------------
 * hts_menu.h - header file for Hts main routine/menu.
 *
 *
 * Copyright (c) 2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 * Original Author : mcharon
 *
 *
 *------------------------------------------------------------------
 *
 */

#define HT_IF   "ht"
#define ETH4_IF "eth4"
#define ETH3_IF "eth3"

#define PACKET_COUNT        1000    /* number of transmit packet */
#define ETH_PKT_MAX_LEN     1024    /* maximum packet length */
#define PACKET_LENGTH       1024    /* length of transmit packet */
#define IF_NAME_MAX_LEN     32
#define MAX_RX_WAIT_TIME    8       /* maximum waiting time during receiving */

typedef struct {
    char name[10];                  /* name of eth*/
    unsigned char *pkt_buf;         /* pointer to rx buffer */
    int pkt_len;                    /* packet length */
    int socket;
} packet_info_t;


typedef enum {
    HTS_MEM_EXEC =  0,
    HTS_RESET_EXEC,
    HTS_DESC_EXEC,
    HTS_FBUF_EXEC,
    HTS_RW_EXEC,
    HTS_XAUI_EXEC,
    HTS_TX_EXEC,
    HTS_RX_EXEC,
    HTS_SET_INTR_LVL,
    HTS_SET_FCPU_DEST,
    HTS_SET_FCPU_PRIO,
    HTS_DUMP_REG,
    HTS_DUMP_PTR_BLK,
} hts_menu;
/*
#define MAX_FCPU_DESC  24

typedef struct test_desc_t_ {
    unsigned int fcpu_idx[MAX_FCPU_DESC];
    unsigned int sz[MAX_FCPU_DESC];
    unsigned char *data[MAX_FCPU_DESC];
    unsigned int ntest;
} test_desc_t;
*/

extern struct menuinfo *maindiagp_hts;
extern int hts_dma_tests(int);

/*---------------------------------------------------------------
$Log: hts_menu.h,v $
Revision 1.2  2013/10/14 11:17:14  hroni
implement hts_menu_tx_rx()

Revision 1.1  2013/07/22 19:37:02  mcharon
move hts to utah dir/add platform_stub


$Endlog$
*/
