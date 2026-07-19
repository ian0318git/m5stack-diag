/* $Id: skye_main.h,v 1.2 2015/05/25 03:59:11 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/skye_linux/include/skye_main.h,v $
 *------------------------------------------------------------------
 * Header file for platform ethernet code
 *
 * Jul 2014 - Paul Lin(palin2) ported from Shrinkray,
 *            original author is Sofian(steja).
 *
 * Copyright (c) 2013-2015 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

#ifndef SHRINKRAY_MAIN_H_
#define SHRINKRAY_MAIN_H_

/* CPU 0 or CPU 1 */
enum {
    CPU0 = 0,
    CPU1,
};

#define SHRINKRAY_CMD        0
#define SHRINKRAY_DATA       1
#define SHRINKRAY_ACK        2
#define SHRINKRAY_RESULT     4
#define SHRINKRAY_MENU       5

#define TEST_OK                                  0x40
#define TEST_ACK                                 0x80
#define TEST_FAIL                                0xC0


#define FROM_HOST_SWITCH_CONSOLE                 0x01
#define FROM_HOST_CPU_ALIVE_TEST                 0x02
#define FROM_HOST_WRITE_MAC_ADDR                 0x03

/*ACK*/
#define TO_HOST_SWITCH_CONSOLE_ACK \
        (FROM_HOST_SWITCH_CONSOLE + TEST_ACK)
#define TO_HOST_CPU_ALIVE_TEST_ACK \
        (FROM_HOST_CPU_ALIVE_TEST + TEST_ACK)
#define TO_HOST_WRITE_MAC_ADDR_ACK \
        (FROM_HOST_WRITE_MAC_ADDR + TEST_ACK)

/*OK*/
#define TO_HOST_SWITCH_CONSOLE_OK \
        (FROM_HOST_SWITCH_CONSOLE + TEST_OK)
#define TO_HOST_CPU_ALIVE_TEST_OK \
        (FROM_HOST_CPU_ALIVE_TEST + TEST_OK)
#define TO_HOST_WRITE_MAC_ADDR_OK \
        (FROM_HOST_WRITE_MAC_ADDR + TEST_OK)

/*FAIL*/
#define TO_HOST_SWITCH_CONSOLE_FAIL \
        (FROM_HOST_SWITCH_CONSOLE + TEST_FAIL)
#define TO_HOST_CPU_ALIVE_TEST_FAIL \
        (FROM_HOST_CPU_ALIVE_TEST + TEST_FAIL)
#define TO_HOST_WRITE_MAC_ADDR_FAIL \
        (FROM_HOST_WRITE_MAC_ADDR + TEST_FAIL)

#define CMD_NOT_FOUND                        0x11       /* command not found */
#define TO_HOST_REINIT_TX_RX_FAIL            0xFE

int etsec_recv_nframes[3];
int etsec_tx_nframes[3];

typedef struct sm_skye_eth_intr_iface_t_ {
    int eth_tx_intr_cnt;
    int eth_rx_intr_cnt;

} sm_skye_eth_intr_iface_t;

#define SKYE_OUTPUT_PING_FILES          "/diag/skye_check_ping_output.txt"
#define SKYE_PING_XGBE2_RESULT_FILES    "/diag/skye_ping_xgbe2_result.txt"
#define SKYE_SET_TLK_BIT_FILES          "/diag/skye_check_set_tlk_bit.txt"
#define SKYE_CLR_TLK_BIT_FILES          "/diag/skye_check_clr_tlk_bit.txt"
#define SKYE_SET_XG_CFG_FILES           "/diag/skye_check_xg_config_output.txt"

extern void skye_menu(void);
extern int skye_cpu_alive_test(void);
extern int skye_switch_console(void);
extern int skye_write_mac_addr(void);
extern int set_cpu_speed(void);


#endif /* SHRINKRAY_MAIN_H_ */

/*-------------------------------------------------
 * $Log: skye_main.h,v $
 * Revision 1.2  2015/05/25 03:59:11  steja
 * Add Support Skye SM
 *
 * Revision 1.1.4.4  2015/05/05 11:53:07  steja
 * CDETS[CSCuu01237] Solving TLK intermittent loopback issue on GH platform.
 *
 * Revision 1.1.4.3  2015/04/29 13:30:32  steja
 * Update TLK 10G-KR test path
 *
 * Revision 1.1.4.2  2015/04/29 11:36:28  steja
 * Code check-in to skye-branch2 for ER code review
 *
 *
 * ------------------------------------------------------------------
 * Revision 1.1.2.3  2015/03/26 08:33:37  steja
 * Debug edvt found issue on 2CPU skye Dual CPU Xaui Test
 *
 * Revision 1.1.2.2  2014/08/28 02:54:11  steja
 * Support Do all test for NC command
 *
 * Revision 1.1.2.1  2014/07/21 01:56:40  palin2
 * Initial check-in Skye module side Diag code.
 *
 *-------------------------------------------------
 * shrinkray_main.h:
 * Revision 1.2  2014/02/27 15:01:09  palin2
 * Initial check-in ShrinkRay SM side Diag code.
 *
 * Revision 1.1.4.2  2013/09/13 07:00:00  palin2
 * Initial check-in ShrinkRay SM side Diag code.
 *
 * Revision 1.1.2.2  2013/09/05 08:07:44  steja
 * Support Set CPU Frequency margin utilities
 *
 * Revision 1.1.2.1  2013/08/15 11:30:32  steja
 * Add code command and respond ( Host <->GE <-> TILE CPU#0) for G2 (PPC & MIPS) platform
 *
 *-------------------------------------------------
 * $Endlog$
 */
