/* $Id: diag_bcm82757_test.h,v 1.2 2021/06/02 08:22:34 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/diag_bcm82757_test.h,v $
 *------------------------------------------------------------------
 *
 * diag_bcm82757_test.h - Fugazi BCM82757 Diag test definitions.
 *
 * Dec. 2018, Nocken Zou <yozou@cisco.com>
 *
 * Copyright (c) 2019-2021 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __FUGAZI_BCM82757_TEST_H__
#define __FUGAZI_BCM82757_TEST_H__

#define PRBS_TEST_DELAY             1
#define BCM82757_TX_CTRL5_REG       0x4800D0A5
#define BCM82757_TX_FIR_CTRL1_REG   0x4800D110
#define BCM82757_TX_FIR_CTRL2_REG   0x4800D111
#define BCM82757_LANE_MAX           4
#define BCM82757_PHY_RESET_TIME     10
#define BCM82757_LASI_WAIT_TIME     1000
#define BCM82757_LINK_DOWN_INT      0x01 /* Link Down Interrupt */
#define BCM82757_INT_CLEAR          0x02 /* Clear Interrupt */
#define BCM82757_INT_ENABLE         0x01 /* Enable Interrupt */
#define BCM82757_INT_DISABLE        0x00 /* Disable Interrupt */
#define BCM82757_PORT_ETH           "eth"
#define BCM82757_TLB_RX_PRBS_CHK_ERR_CNT_MSB    0x4200d0da
#define BCM82757_TLB_RX_PRBS_CHK_ERR_CNT_LSB    0x4200d0db
#define BCM82757_PRBS_CHK_ERR_CNT_MSB           0x4800d0da
#define BCM82757_PRBS_CHK_ERR_CNT_LSB           0x4800d0db
#define SIOCSMIILASI                0x89F1  /* Read LASI event count.   */
#define RX_LOS_STATUS_REG           0x8a5f
#define TX_FLT_STATUS_REG           0x8A67 
#define MOD_ABS_STATUS_REG          0x8A6F 
#define PIN_ACERT_VALUE             0x4
#define LINK_MAX_CHECK              260
#define MAX_LINKUP_CONSISTENCY      10

extern int fugazi_diag_init(int argc, char *argv[]);
extern void fugazi_diag_exit(void);
extern int fugazi_bcm82757_test(int);
extern int bcm82757_show_fw_version( void );
extern int bcm82757_recover_clock( int, int );
extern int bcm82757_PHY_init(int, int);
extern int bcm82757_loopback_set(int, int, unsigned int, unsigned int);
extern long bcm82757_reg_read(void);
extern long bcm82757_reg_write(void); 
extern long bcm82757_link_status(void);
extern long bcm82757_interrupt(void);
extern long bcm82757_init_f(void);
extern int  fugazi_bcm82757_check_link_stable(int, int, unsigned int *, unsigned int *);


#endif /* __FUGAZI_BCM82757_TEST_H__ */

/*-------------------------------------------------
 * $Log: diag_bcm82757_test.h,v $
 * Revision 1.2  2021/06/02 08:22:34  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.8.4  2021/04/29 01:43:17  pdoong
 * Add checking if PHY Network side link is up in 'SyncE Recovered Clock Test'
 *
 * Revision 1.1.8.3  2020/10/07 08:07:42  iachang
 * CSCvo59196-15 Fugazi: Fixed BCM82757 ext loopback test failed get hand up on Apollo
 *
 * Revision 1.1.8.2  2020/08/26 02:37:48  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.6.18  2020/08/03 09:25:40  iachang
 * Code clean up.
 *
 * Revision 1.1.6.17  2020/03/18 06:51:44  iachang
 * Create independent file for LASI test
 *
 * Revision 1.1.6.16  2020/03/06 05:54:43  iachang
 * Implement BCM82757 Side Band Test
 *
 * Revision 1.1.6.15  2020/03/04 08:29:18  iachang
 * Correct BCM82757 Side_band register dump
 *
 * Revision 1.1.6.14  2020/02/25 08:02:49  iachang
 * Modify BCM82757 LASI test utility.
 *
 * Revision 1.1.6.13  2019/09/16 11:23:42  iachang
 * CSCvr24877 : Display PRBS error count when inject error from external
 *
 * Revision 1.1.6.12  2019/07/19 02:29:35  iachang
 * Sync loopback funtion with Curie-2RU
 * Changed Loopback funciton from Curie-2RU to ISR common function tx_rx_diag()
 * Changed BCM82757 print message "lane" to "port"
 *
 * Revision 1.1.6.11  2019/07/18 22:35:26  pdoong
 * Add BCM82757 PHY init utility
 *
 * Revision 1.1.6.10  2019/06/21 06:58:46  iachang
 * Support BCM82757 Eye scan utility.
 * Add BCM82757 interrupt utility.
 *
 * Revision 1.1.6.9  2019/06/14 23:58:05  pdoong
 * Add configure bcm82757 10G PHY to generate recovered clock output
 *
 * Revision 1.1.6.8  2019/06/13 14:21:20  iachang
 * Add BCM82757 interrupt utility
 *
 * Revision 1.1.6.7  2019/06/13 08:26:57  iachang
 * Add print_msg flag with BCM82757 reset function.
 *
 * Revision 1.1.6.6  2019/06/12 08:00:35  iachang
 * Merge BCM82757 signal port loopback test item.
 * Add more information with error message.
 *
 * Revision 1.1.6.5  2019/05/14 02:01:31  pdoong
 * Added to sysyem info to display SyncE/bam82757 firmware version
 *
 * Revision 1.1.6.4  2019/04/15 21:09:42  iachang
 * Add Compliance mask test setting.
 *
 * Revision 1.1.6.3  2019/04/01 22:34:07  iachang
 * Support 2nd BCM82757 utility.
 *
 * Revision 1.1.6.2  2019/03/14 03:48:24  letsai
 * Initial check in.
 *
 *
 *
 *
 * $Endlog$
 * */
