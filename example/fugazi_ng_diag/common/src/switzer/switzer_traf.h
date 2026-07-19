/* $Id: switzer_traf.h,v 1.3 2020/05/22 02:28:48 qingcwan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/switzer/switzer_traf.h,v $
 *------------------------------------------------------------------
 *
 * switzer_traf.h
 *
 *
 * Copyright (c) 2018-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __SWITZER_ETH_TXRX__
#define __SWITZER_ETH_TXRX__

struct switzer_eth_traf;
struct switzer_eth_task;

struct switzer_eth_traf_tx_task_settings {
    struct switzer_eth_traf *traf;
    enum {
        SWITZER_ETH_TRAF_TX_MODE_FIXED,
        SWITZER_ETH_TRAF_TX_MODE_RADOM,
        SWITZER_ETH_TRAF_TX_MODE_FIXED_VIRTUAL_MAC,
        SWITZER_ETH_TRAF_TX_MODE_RADOM_VIRTUAL_MAC,
    } mode;
    enum {
        SWITZER_ETH_TRAF_TX_CHECK_BIT_ADD_YES,
        SWITZER_ETH_TRAF_TX_CHECK_BIT_ADD_NO,
    } check;
    unsigned int len;
    unsigned int burst;
    unsigned int duration;
};

struct switzer_eth_traf_rx_task_settings {
    struct switzer_eth_traf *traf;
    enum {
        SWITZER_ETH_TRAF_RX_MODE_CHECK_BIT,
        SWITZER_ETH_TRAF_RX_MODE_CHECK_LEN,
    } chk_mode;
};

struct switzer_eth_traf_statistic_info {
    uint64_t tx_cnt;
    uint64_t tx_size;
    uint64_t rx_cnt;
    uint64_t rx_size;
    uint64_t rx_errcnt;
};

struct switzer_eth_traf *switzer_eth_traf_init(const char *iface, size_t sq_len);
void switzer_eth_traf_exit(struct switzer_eth_traf *traf);

int switzer_eth_traf_tx_poll(struct switzer_eth_traf *traf, time_t timeout);
int switzer_eth_traf_tx_pkt(struct switzer_eth_traf *traf, const char *pkt, size_t len);
int switzer_eth_traf_rx_poll(struct switzer_eth_traf *traf, time_t timeout);
int switzer_eth_traf_rx_pkt(struct switzer_eth_traf *traf, char *pkt, size_t len);

struct switzer_eth_task *switzer_eth_traf_tx_task_start(struct switzer_eth_traf_tx_task_settings *settings);
void switzer_eth_traf_tx_stop(struct switzer_eth_task *task);

struct switzer_eth_task *switzer_eth_traf_rx_task_start(struct switzer_eth_traf_rx_task_settings *settings);
void switzer_eth_traf_rx_stop(struct switzer_eth_task *task);

void switzer_eth_traf_statistic(struct switzer_eth_traf *traf, struct switzer_eth_traf_statistic_info *info, int verbose);

int switzer_eth_traf_util_test(const char *tx_iface, const char *rx_iface,
                    struct switzer_eth_traf_tx_task_settings *tx_settings,
                    struct switzer_eth_traf_rx_task_settings *rx_settings,
                    time_t timeout);

#endif
