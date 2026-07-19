/* $Id: switzer_traf.c,v 1.6 2021/10/20 06:10:18 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/switzer/switzer_traf.c,v $
 *------------------------------------------------------------------
 *
 * switzer_traf.c
 * Copyright (c) 2018-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <features.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/ethtool.h>
#include <linux/sockios.h>
#include <assert.h>
#include <pthread.h>
#include <sys/time.h>
#include <poll.h>

#include "common.h"
#include "switzer_common.h"
#include "switzer_traf.h"

#define POLL_TIMEOUT_RX_TASK 500
#define ETH_HD_LEN 21
#define PKT_MIN_LEN 60
#define PKT_MAX_LEN 1500
#define PKT_TYPE 0xdead
#define PKT_TAIL 0xbeaf

struct switzer_eth_traf_info {
    uint64_t count;
    uint64_t size;
    uint64_t errcnt;
    uint32_t sq[0];
};

struct switzer_eth_task {
    int sock;
    char iface[64];
    int done;
    unsigned int pkt_len;
    unsigned int duration;
    unsigned int burst;
    int tx_mode;
    int tx_chk_bit;
    int rx_mode;
    pthread_t tid;
    size_t sq_len;
    uint32_t *sq;
    struct switzer_eth_traf_info *info;
    struct switzer_eth_task *next;
    struct switzer_eth_task *prev;
};


struct switzer_eth_traf {
    int sock;
    char iface[64];
    int tx_task_count;
    int rx_task_count;
    size_t sq_len;
    struct switzer_eth_traf_info *tx_info;
    struct switzer_eth_traf_info *rx_info;
    struct switzer_eth_task *tx_task;
    struct switzer_eth_task *rx_task;
};

struct packet_setting {
    uint8_t mac_da[6];
    uint8_t mac_sa[6];
    uint16_t pkt_type;
    uint8_t chksum;
    uint16_t pkt_len;
    uint32_t sq_id;
    uint8_t data[0];
}__attribute__((__packed__));


static int switzer_get_mac_sa(const char *if_name, uint8_t *mac_sa)
{
    int raw;
    struct ifreq ifr;

    bzero((void *)&ifr, sizeof(ifr));
    sprintf(ifr.ifr_name, if_name);
    /* Create the raw socket */
    if ((raw = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL)))== -1) {
        log_err("Error creating raw socket\n");
        return(-1);
    }

    /* get hdware addr */
    if((ioctl(raw, SIOCGIFHWADDR, &ifr)) == -1) {
        log_err("Error getting HDADDR index !\n");
        return(-1);
    }
    memcpy(mac_sa, ifr.ifr_hwaddr.sa_data, 6);
    close(raw);

    return 0;
}

static void switzer_gen_socket_pkt (struct packet_setting *pkt,
                                    const char *iface, int pkt_len, int real_mac)
{
    int i;
    int data_len;

    uint8_t mac_sa[] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};
    uint8_t mac_da[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

    if (real_mac) {
        switzer_get_mac_sa(iface, mac_sa);
    }
    memcpy(pkt->mac_da, mac_da, 6);
    memcpy(pkt->mac_sa, mac_sa, 6);

    pkt_len = pkt_len > PKT_MAX_LEN ? PKT_MAX_LEN : pkt_len;
    pkt_len = pkt_len < PKT_MIN_LEN ? PKT_MIN_LEN : pkt_len;
    data_len = pkt_len - ETH_HD_LEN;
    pkt->pkt_len = pkt_len;
    pkt->pkt_type = PKT_TYPE;

    for (i = 0; i < data_len - 2; i++) {
        pkt->data[i] = (uint8_t)rand() % 256;
    }
    pkt->data[i] = (uint8_t)(PKT_TAIL >> 8);
    pkt->data[i+1] = (uint8_t)(PKT_TAIL & 0xff);
}

static void switzer_display_pkt(unsigned char *b_ptr, int pktlen)
{
    int i, len;
    len = pktlen;
    prt("%s- packet: showing %d bytes\n", __FUNCTION__, len);
    for (i=0; i < len; i++) {
        if ((i>0) && ((i % 16) == 0)) {
	        prt("\n");
	    }
	    prt("%02x ", *b_ptr++);
    }
    prt("\n");
}

static uint8_t switzer_chksum(struct packet_setting *pkt)
{
    int i;
    uint8_t sum = 0;
    uint8_t *data = (void *)pkt;
    int len = pkt->pkt_len;

    for (i=0; i< len; i++) {
        sum += data[i];
    }

    sum -= pkt->chksum;
    return sum;
}

static int switzer_chk_pkt(struct packet_setting *pkt)
{
    uint8_t chksum = pkt->chksum;
    if (chksum != switzer_chksum(pkt)) {
        log_err("pkt_data does not match\n");
        return(-1);
    }
    return 0;
}

static void switzer_cleanup_eth_dev (char *if_name, int sock)
{
    struct ifreq ifr;

    bzero(&ifr, sizeof(ifr));

    /* Set the network card in promiscuous mode */
    sprintf(ifr.ifr_name, if_name);
    if (ioctl(sock,SIOCGIFFLAGS,&ifr) == -1) {
        log_err("ioctl: SIOCSIFFLAGS get interface index\n");
    }

    ifr.ifr_flags &= ~IFF_PROMISC;
    if (ioctl(sock,SIOCSIFFLAGS,&ifr) == -1) {
        log_err("ioctl: SIOCSIFFLAGS to clear promiscous mode\n");
    }

    close(sock);
}

static int switzer_set_promisc (const char *device, int sock)
{
    struct ifreq ifr;

    bzero(&ifr, sizeof(ifr));

    /* Set the network card in promiscuos mode */
    sprintf(ifr.ifr_name, device);
    if (ioctl(sock,SIOCGIFFLAGS,&ifr)==-1) {
        log_err("ioctl: SIOCSIFFLAGS get index interface\n");
        close(sock);
        return(-1);
    }

    ifr.ifr_flags |= IFF_PROMISC;
    if (ioctl(sock,SIOCSIFFLAGS,&ifr)==-1) {
        log_err("ioctl: SIOCSIFFLAGS to set promiscous mode\n");
        close(sock);
        return(-1);
    }

    return 0;
}


static int switzer_bind_socket (const char *device, int rawsock, int protocol)
{
    struct sockaddr_ll sll;
    struct ifreq ifr;

    bzero((void *)&sll, sizeof(sll));
    bzero((void *)&ifr, sizeof(ifr));

    /* First Get the Interface Index  */
    sprintf((char *)ifr.ifr_name, device);
    if ((ioctl(rawsock, SIOCGIFINDEX, &ifr)) == -1) {
        log_err("Error getting Interface index !\n");
        return(-1);
    }

    /* Bind our raw socket to this interface */
    sll.sll_family = AF_PACKET;
    sll.sll_ifindex = ifr.ifr_ifindex;
    sll.sll_protocol = htons(protocol);

    if((bind(rawsock, (struct sockaddr *)&sll, sizeof(sll))) == -1) {
        log_err("Error binding raw socket to interface\n");
        return(-1);
    }

    return 0;
}

static int switzer_setup_socket (const char *eth_name, int *sock)
{
    int raw;
    /* Create the raw socket */
    if ((raw = socket(PF_PACKET, SOCK_RAW | SOCK_NONBLOCK, htons(ETH_P_ALL)))== -1) {
        log_err("Error creating raw socket\n");
        return -1;
    }

    if (switzer_bind_socket(eth_name, raw, ETH_P_ALL) == -1) {
        log_err("bind socket failed\n");
        close(raw);
        return -1;
    }

    if (switzer_set_promisc(eth_name, raw) == -1) {
        log_err("set promisc failed\n");
        close(raw);
        return -1;
    }

    *sock = raw;
    return 0;
}

static int switzer_eth_pkt_tx (struct switzer_eth_task *tx_task)
{
    unsigned int burst = tx_task->burst;
    uint64_t sq_id = 0, sen_size = 0;
    uint64_t sen_cnt = 0, sq_cnt = 0;
    uint32_t *tx_sq = tx_task->sq;
    unsigned int dura;
    uint16_t pkt_type, pkt_len = tx_task->pkt_len;
    int i, rc, tx_skt = tx_task->sock;
    uint8_t txpkt_buf[PKT_MAX_LEN];
    struct packet_setting *tx_pkt = (void *)txpkt_buf;

    srand((int)time(NULL));
    memset(txpkt_buf, 0, PKT_MAX_LEN);


    if (tx_task->tx_mode == SWITZER_ETH_TRAF_TX_MODE_FIXED){
        switzer_gen_socket_pkt(tx_pkt, tx_task->iface, pkt_len, 1);
        pkt_type = tx_pkt->pkt_type;
        pkt_len = tx_pkt->pkt_len;
    } else if (tx_task->tx_mode == SWITZER_ETH_TRAF_TX_MODE_FIXED_VIRTUAL_MAC) {
        switzer_gen_socket_pkt(tx_pkt, tx_task->iface, pkt_len, 0);
        pkt_type = tx_pkt->pkt_type;
        pkt_len = tx_pkt->pkt_len;
    }
    dura = tx_task->duration;

    while (!tx_task->done) {
        if (tx_task->tx_mode == SWITZER_ETH_TRAF_TX_MODE_RADOM){
            pkt_len = rand() % (PKT_MAX_LEN - PKT_MIN_LEN + 1) + PKT_MIN_LEN;
            switzer_gen_socket_pkt(tx_pkt, tx_task->iface, pkt_len, 1);
            pkt_type = tx_pkt->pkt_type;
            pkt_len = tx_pkt->pkt_len;
        } else if (tx_task->tx_mode == SWITZER_ETH_TRAF_TX_MODE_RADOM_VIRTUAL_MAC) {
            pkt_len = rand() % (PKT_MAX_LEN - PKT_MIN_LEN + 1) + PKT_MIN_LEN;
            switzer_gen_socket_pkt(tx_pkt, tx_task->iface, pkt_len, 0);
            pkt_type = tx_pkt->pkt_type;
            pkt_len = tx_pkt->pkt_len;
        }

        tx_pkt->sq_id = sq_cnt;
        tx_pkt->pkt_len = pkt_len;
        tx_pkt->pkt_type = pkt_type;
        if (tx_task->tx_chk_bit == SWITZER_ETH_TRAF_TX_CHECK_BIT_ADD_YES) {
            tx_pkt->chksum = switzer_chksum(tx_pkt);
        }

        tx_pkt->sq_id = htonl(tx_pkt->sq_id);
        tx_pkt->pkt_type = htons(tx_pkt->pkt_type);
        tx_pkt->pkt_len = htons(tx_pkt->pkt_len);

        sq_id = sq_cnt;
        sq_cnt++;
        sq_cnt = sq_cnt % tx_task->sq_len;
        rc = write(tx_skt, tx_pkt, pkt_len);
        if (rc <= 0 ) {
            if (errno == EAGAIN || !rc)
                continue;
            else {
                log_err("sock write err\n");
                return -1;
            }
        }
        sen_cnt++;
        sen_size += pkt_len;
        tx_sq[sq_id]++;
        if (burst > 0 && dura > 0) {
            if (sen_cnt % burst == 0) {
                usleep(dura);
            }
        }
    }

    tx_task->info->count += sen_cnt;
    tx_task->info->size += sen_size;
    for (i = 0; i < tx_task->sq_len; i++) {
        tx_task->info->sq[i] += tx_sq[i];
    }
    return 0;
}

static int switzer_drop_pkt(struct packet_setting *pkt, int size)
{
    if (pkt->pkt_type != PKT_TYPE)
        return -1;
    if (size < PKT_MIN_LEN || size > PKT_MAX_LEN)
        return -1;
    if (pkt->data[size - ETH_HD_LEN - 2] != PKT_TAIL >> 8)
        return -1;
    if (pkt->data[size - ETH_HD_LEN - 1] != (PKT_TAIL & 0xff))
        return -1;
    return 0;
}

static int switzer_eth_pkt_rx(struct switzer_eth_task *rx_task)
{
    int i, rc, err_flag;
    uint64_t rv_cnt = 0, err_cnt = 0, rv_size = 0;
    uint32_t *rx_sq = rx_task->sq;
    int rx_skt = rx_task->sock;
    uint8_t rxpkt_buf[PKT_MAX_LEN];
    struct packet_setting *rx_pkt = (void *)rxpkt_buf;
    int rvbuf_size = 208 * 1024;

    setsockopt(rx_skt, SOL_SOCKET, SO_RCVBUF, (const char *)&rvbuf_size, sizeof(int));
    memset(rxpkt_buf, 0, PKT_MAX_LEN);


    while (!rx_task->done) {
        err_flag = 0;
        rc = read(rx_skt, rxpkt_buf, sizeof(rxpkt_buf));
        if (rc <= 0) {
            if (errno == EAGAIN || !rc){
                struct pollfd fd = {
                    .fd = rx_skt,
                    .events = POLLIN,
                };
                if (poll(&fd, 1, POLL_TIMEOUT_RX_TASK) < 0)
                    break;
                if (!(fd.revents & POLLIN))
                    continue;
            } else {
                log_err("sock read err\n");
                return -1;
            }
            continue;
        }

        rx_pkt->pkt_type = ntohs(rx_pkt->pkt_type);
        rx_pkt->pkt_len = ntohs(rx_pkt->pkt_len);
        rx_pkt->sq_id = ntohl(rx_pkt->sq_id);

        if (switzer_drop_pkt(rx_pkt, rc))
            continue;

        rv_cnt++;
        rv_size += rc;
        if (rx_task->rx_mode == SWITZER_ETH_TRAF_RX_MODE_CHECK_BIT) {
            if (switzer_chk_pkt(rx_pkt)) {
                switzer_display_pkt(rxpkt_buf,rc);
                err_flag = 1;
            }
        }

        if (rx_pkt->sq_id >=0 && rx_pkt->sq_id <= rx_task->sq_len)
            rx_sq[rx_pkt->sq_id]++;
        else
            err_flag = 1;

        if (rx_task->rx_mode == SWITZER_ETH_TRAF_RX_MODE_CHECK_LEN) {
            if (rx_pkt->pkt_len != rc) {
                err_flag = 1;
            }
        }

        if (err_flag)
            err_cnt++;
    }

    rx_task->info->count += rv_cnt;
    rx_task->info->errcnt += err_cnt;
    rx_task->info->size += rv_size;
    for (i = 0; i < rx_task->sq_len; i++) {
        rx_task->info->sq[i] += rx_sq[i];
    }
    return 0;
}

struct switzer_eth_traf *switzer_eth_traf_init(const char *iface, size_t sq_len)
{
    int raw, len;
    struct switzer_eth_traf *traf;

    if (sq_len <= 0) {
        log_err("sq_len err\n");
        return NULL;
    }

    traf = (struct switzer_eth_traf *)malloc(sizeof(struct switzer_eth_traf));
    if (traf == NULL) {
        log_err("eth_traf init err\n");
        return NULL;
    }
    traf->tx_task_count = 0;
    traf->rx_task_count = 0;
    traf->sq_len = sq_len;

    traf->tx_task = (struct switzer_eth_task *)malloc(sizeof(struct switzer_eth_task));
    if (traf->tx_task == NULL) {
        log_err("eth_traf init err\n");
        goto err1;
    }

    traf->rx_task = (struct switzer_eth_task *)malloc(sizeof(struct switzer_eth_task));
    if (traf->rx_task == NULL) {
        log_err("eth_ traf init err\n");
        goto err2;
    }

    traf->tx_task->next = NULL;
    traf->tx_task->prev = NULL;
    traf->rx_task->next = NULL;
    traf->rx_task->prev = NULL;

    len = sizeof(uint32_t) * (sq_len + 6);

    traf->tx_info = (struct switzer_eth_traf_info *)malloc(len);
    if (traf->tx_info == NULL) {
        log_err("tx_info create err\n");
        goto err3;
    }

    memset(traf->tx_info, 0, len);

    traf->rx_info = (struct switzer_eth_traf_info *)malloc(len);
    if (traf->rx_info == NULL) {
        log_err("rx_info create err\n");
        goto err4;
    }

    memset(traf->rx_info, 0, len);

    if (switzer_setup_socket (iface, &raw) == -1){
        log_err("socket setup err\n");
        goto err5;
    }
    traf->sock = raw;
    strcpy(traf->iface, iface);
    return traf;

err5:
    free(traf->rx_info);
err4:
    free(traf->tx_info);
err3:
    free(traf->rx_task);
err2:
    free(traf->tx_task);
err1:
    free(traf);
    return NULL;
}

struct switzer_eth_task * switzer_eth_traf_tx_task_start(struct switzer_eth_traf_tx_task_settings *settings)
{
    int rc, len;
    struct switzer_eth_task *tx_task = NULL;

    tx_task = (struct switzer_eth_task *)malloc(sizeof(struct switzer_eth_task));
    if (tx_task == NULL) {
        log_err("tx_task malloc err\n");
        return NULL;
    }

    tx_task->sock = settings->traf->sock;
    tx_task->pkt_len = settings->len;
    tx_task->info = settings->traf->tx_info;
    tx_task->sq_len = settings->traf->sq_len;
    tx_task->duration = settings->duration;
    tx_task->burst = settings->burst;
    tx_task->tx_mode = settings->mode;
    tx_task->tx_chk_bit = settings->check;
    tx_task->done = 0;

    len = sizeof(uint32_t) * tx_task->sq_len;
    tx_task->sq = (uint32_t *)malloc(len);
    if (tx_task == NULL) {
        log_err("tx_task sq create err\n");
        goto err1;
    }
    memset(tx_task->sq, 0, len);

    strcpy(tx_task->iface, settings->traf->iface);
    tx_task->next = NULL;
    tx_task->prev = NULL;

    rc = pthread_create(&tx_task->tid, NULL, (void *)switzer_eth_pkt_tx, tx_task);
    if (rc) {
        log_err("pthread create err\n");
        goto err2;
    }

    settings->traf->tx_task_count++;
    tx_task->next = settings->traf->tx_task->next;
    tx_task->prev = settings->traf->tx_task;
    if (settings->traf->tx_task->next != NULL)
        settings->traf->tx_task->next->prev = tx_task;
    settings->traf->tx_task->next = tx_task;
    return tx_task;

err2:
    free(tx_task->sq);
err1:
    free(tx_task);
    return NULL;
}

void switzer_eth_traf_tx_stop(struct switzer_eth_task *task)
{
    task->done = 1;
    if (pthread_join(task->tid, NULL)) {
        log_err("tx pthread join err\n");
    }
    task->prev->next = task->next;
    if (task->next != NULL) {
        task->next->prev = task->prev;
    }
    free(task->sq);
    free(task);
}

struct switzer_eth_task *switzer_eth_traf_rx_task_start(struct switzer_eth_traf_rx_task_settings *settings)
{
    int rc, len;
    struct switzer_eth_task *rx_task = NULL;

    rx_task = (struct switzer_eth_task *)malloc(sizeof(struct switzer_eth_task));
    if (rx_task == NULL) {
        log_err("rx_task malloc err\n");
        return NULL;
    }
    rx_task->sock = settings->traf->sock;
    rx_task->info = settings->traf->rx_info;
    rx_task->sq_len = settings->traf->sq_len;
    rx_task->rx_mode = settings->chk_mode;
    rx_task->done = 0;

    len = sizeof(uint32_t) * rx_task->sq_len;

    rx_task->sq = (uint32_t *)malloc(len);
    if (rx_task->sq == NULL) {
        log_err("rx_task sq create err\n");
        goto err1;
    }
    memset(rx_task->sq, 0, len);
    strcpy(rx_task->iface, settings->traf->iface);
    rx_task->next = NULL;

    rc = pthread_create(&rx_task->tid, NULL, (void *)switzer_eth_pkt_rx, rx_task);
    if (rc) {
        log_err("pthread create err\n");
        goto err2;
    }

    settings->traf->rx_task_count++;

    rx_task->next = settings->traf->rx_task->next;
    rx_task->prev = settings->traf->rx_task;
    if (settings->traf->rx_task->next != NULL)
        settings->traf->rx_task->next->prev = rx_task;
    settings->traf->rx_task->next = rx_task;

    return rx_task;

err2:
    free(rx_task->sq);
err1:
    free(rx_task);
    return NULL;
}

void switzer_eth_traf_rx_stop(struct switzer_eth_task *task)
{
    task->done = 1;
    if (pthread_join(task->tid, NULL)) {
        log_err("rx pthread join err\n");
    }
    task->prev->next = task->next;
    if (task->next != NULL)
        task->next->prev = task->prev;
    free(task->sq);
    free(task);
}

void switzer_eth_traf_statistic(struct switzer_eth_traf *traf, struct switzer_eth_traf_statistic_info *info, int verbose)
{
    int i;
    info->tx_cnt = traf->tx_info->count;
    info->tx_size = traf->tx_info->size;
    info->rx_cnt = traf->rx_info->count;
    info->rx_size = traf->rx_info->size;
    info->rx_errcnt = traf->rx_info->errcnt;

    if (verbose > 0) {
        prt("\n   %-10s%-10s%-10s%-10s%-10s%-10s\n",
                "IF","Transmit","TxBytes","Receive","RxBytes","RxErr");
        prt("---------------------------------------------------------------\n");
        prt("   %-10s%-10lu%-10lu%-10lu%-10lu%-10lu\n",
            traf->iface, info->tx_cnt, info->tx_size,
            info->rx_cnt, info->rx_size, info->rx_errcnt);
        prt("---------------------------------------------------------------\n\n");
        if (verbose > 1) {
            for (i = 0; i < traf->sq_len; i++) {
                prt("%s tx_sq[%d] %d, rx_sq[%d] %d\n",traf->iface, i, traf->tx_info->sq[i], i, traf->rx_info->sq[i]);
            }
        }
    }
}

void switzer_eth_traf_exit(struct switzer_eth_traf *traf)
{
    struct switzer_eth_task *tx_task = NULL;
    struct switzer_eth_task *rx_task = NULL;

    tx_task = traf->tx_task;
    rx_task = traf->rx_task;
    while (tx_task->next != NULL) {
        switzer_eth_traf_tx_stop(tx_task->next);
    }
    free(tx_task);
    while (rx_task->next != NULL) {
        switzer_eth_traf_rx_stop(rx_task->next);
    }
    free(rx_task);
    free(traf->tx_info);
    free(traf->rx_info);
    switzer_cleanup_eth_dev (traf->iface, traf->sock);

    free(traf);
}

int switzer_eth_traf_tx_poll(struct switzer_eth_traf *traf, time_t timeout)
{
    struct pollfd fd = {
        .fd = traf->sock,
        .events = POLLOUT,
    };
    if (poll(&fd, 1, timeout) < 0) {
        log_err("poll err\n");
        return -1;
    }
    if (!(fd.revents & POLLOUT)) {
        log_err("write timeout err\n");
        return -1;
    }
    return 0;
}
int switzer_eth_traf_tx_pkt(struct switzer_eth_traf *traf, const char *pkt, size_t len)
{
    int rc;
    int sock = traf->sock;
    uint8_t txpkt_buf[PKT_MAX_LEN];

    if (len > PKT_MAX_LEN) {
        log_err("the len of input pkt err\n");
        return -1;
    }

    memcpy(txpkt_buf, pkt, len);
    rc = write(sock, txpkt_buf, len);
    if (rc < 0 ) {
        log_err("sock write err\n");
        return -1;
    }
    return rc;
}

int switzer_eth_traf_rx_poll(struct switzer_eth_traf *traf, time_t timeout)
{
    struct pollfd fd = {
        .fd = traf->sock,
        .events = POLLIN,
    };
    if (poll(&fd, 1, timeout) < 0) {
        log_err("poll err\n");
        return -1;
    }
    if (!(fd.revents & POLLIN)) {
        log_err("receive timeout\n");
        return -1;
    }
    return 0;
}

int switzer_eth_traf_rx_pkt(struct switzer_eth_traf *traf, char *pkt, size_t len)
{
    int rc, sock = traf->sock;
    uint8_t rxpkt_buf[PKT_MAX_LEN];

    if (len > PKT_MAX_LEN) {
        log_err("the len of input pkt err\n");
        return -1;
    }

    rc = read(sock, rxpkt_buf, sizeof(rxpkt_buf));
    if (rc < 0) {
        log_err("sock read err\n");
        return -1;
    }
    memcpy(pkt, rxpkt_buf, rc);
    return rc;
}

int switzer_eth_traf_util_test(const char *tx_iface, const char *rx_iface,
                    struct switzer_eth_traf_tx_task_settings *tx_settings,
                    struct switzer_eth_traf_rx_task_settings *rx_settings,
                    time_t timeout)
{
    int flag;
    struct switzer_eth_traf *tx_traf = NULL;
    struct switzer_eth_traf *rx_traf = NULL;
    struct switzer_eth_task *tx_task = NULL;
    struct switzer_eth_task *rx_task = NULL;
    struct switzer_eth_traf_statistic_info tx_info;
    struct switzer_eth_traf_statistic_info rx_info;

    flag = strcmp(tx_iface, rx_iface);
    if (flag) {
        tx_traf = switzer_eth_traf_init(tx_iface, 256);
        rx_traf = switzer_eth_traf_init(rx_iface, 256);
    } else {
        tx_traf = switzer_eth_traf_init(tx_iface, 256);
        rx_traf = tx_traf;
    }

    tx_settings->traf = tx_traf;
    rx_settings->traf = rx_traf;
    rx_task = switzer_eth_traf_rx_task_start(rx_settings);
    usleep(5);
    tx_task = switzer_eth_traf_tx_task_start(tx_settings);

    PROMPT_DELAY_MS(50, timeout * 1000);

    switzer_eth_traf_tx_stop(tx_task);
    usleep(5);
    switzer_eth_traf_rx_stop(rx_task);

    if (flag) {
        switzer_eth_traf_statistic(tx_traf, &tx_info, 1);
        switzer_eth_traf_statistic(rx_traf, &rx_info, 1);
        switzer_eth_traf_exit(tx_traf);
        switzer_eth_traf_exit(rx_traf);
    } else {
        switzer_eth_traf_statistic(tx_traf, &tx_info, 1);
        rx_info = tx_info;
        switzer_eth_traf_exit(tx_traf);
    }

    if (tx_info.tx_cnt != rx_info.rx_cnt || rx_info.rx_errcnt || !rx_info.rx_cnt)
        return -1;
    return 0;
}
