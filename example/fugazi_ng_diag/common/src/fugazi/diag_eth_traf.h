/* $Id: diag_eth_traf.h,v 1.2 2021/06/02 08:22:34 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/diag_eth_traf.h,v $
 *------------------------------------------------------------------
 *
 * diag_eth_traf.h
 *
 * Copyright (c) 2019-2020 by Cisco Systems, Inc.
 * All rights reserved.
 * < A copy of switzer_traf.h >
 *
 *------------------------------------------------------------------
 */

#ifndef __DIAG_ETH_TRAF__
#define __DIAG_ETH_TRAF__

#define TX_RX_STABLE_DELAY          (5000)
struct eth_traf;
struct eth_task;

struct eth_traf_tx_task_settings {
    struct eth_traf *traf;
    enum {
        ETH_TRAF_TX_MODE_FIXED,
        ETH_TRAF_TX_MODE_RADOM,
    } mode;
    enum {
        ETH_TRAF_TX_CHECK_BIT_ADD_YES,
        ETH_TRAF_TX_CHECK_BIT_ADD_NO,
    } check;
    unsigned int len;
    unsigned int burst;
    unsigned int interval;
};

struct eth_traf_rx_task_settings {
    struct eth_traf *traf;
    enum {
        ETH_TRAF_RX_MODE_CHECK_BIT,
        ETH_TRAF_RX_MODE_CHECK_LEN,
    } chk_mode;
};

struct eth_traf_statistic_info {
    uint64_t tx_cnt;
    uint64_t tx_size;
    uint64_t rx_cnt;
    uint64_t rx_size;
    uint64_t rx_errcnt;
};

struct eth_traf *eth_traf_init(const char *iface, size_t sq_len);
void eth_traf_exit(struct eth_traf *traf);

int eth_traf_tx_poll(struct eth_traf *traf, time_t timeout);
int eth_traf_tx_pkt(struct eth_traf *traf, const char *pkt, size_t len);
int eth_traf_rx_poll(struct eth_traf *traf, time_t timeout);
int eth_traf_rx_pkt(struct eth_traf *traf, char *pkt, size_t len);

struct eth_task *eth_traf_tx_task_start(struct eth_traf_tx_task_settings *settings);
void eth_traf_tx_stop(struct eth_task *task);

struct eth_task *eth_traf_rx_task_start(struct eth_traf_rx_task_settings *settings);
void eth_traf_rx_stop(struct eth_task *task);

void eth_traf_statistic(struct eth_traf *traf, struct eth_traf_statistic_info *info, int verbose);

int eth_traf_util_test(const char *tx_iface, const char *rx_iface,
                    struct eth_traf_tx_task_settings *tx_settings,
                    struct eth_traf_rx_task_settings *rx_settings,
                    time_t timeout);

#endif


/*-------------------------------------------------
 * $Log: diag_eth_traf.h,v $
 * Revision 1.2  2021/06/02 08:22:34  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.4.2  2020/08/26 02:37:48  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.2.3  2020/07/31 09:52:08  iachang
 * Code clean up.
 *
 * Revision 1.1.2.2  2019/07/19 02:29:36  iachang
 * Sync loopback funtion with Curie-2RU
 * Changed Loopback funciton from Curie-2RU to ISR common function tx_rx_diag()
 * Changed BCM82757 print message "lane" to "port"
 *
 * Revision 1.1.2.1  2019/06/17 06:02:20  iachang
 * Rename eth_traf.c/eth_traf.h to diag_eth_traf.c/diag_eth_traf.h
 * Add ethernet packet check message into cterr.
 *
 * Revision 1.1.6.2  2019/03/14 03:48:26  letsai
 * Initial check in.
 *
 *
 *
 * $Endlog$
 * */

