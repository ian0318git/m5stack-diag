 /* $Id: diag_esw_test.h,v 1.4 2018/12/10 09:57:05 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/diag_esw_test.h,v $
 *------------------------------------------------------------------
 * 
 * Filename   : diag_esw_test.h
 * Description: header file of Viper ethernet switch,
 *              Marvell 88E6176.
 *
 * Copyright (c) 2016 ~ 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DIAG_ESW_TEST_H__
#define __DIAG_ESW_TEST_H__

#define GE_SWITCH_IFACE_UP "ifconfig enp5s0f0 up > /dev/null"
#define GE_SWITCH_IFACE_DOWN "ifconfig enp5s0f0 down > /dev/null"
#define VIPERJ_GE_SWITCH_IFACE_UP "ifconfig enp5s0f0 up > /dev/null"
#define VIPERJ_GE_SWITCH_IFACE_DOWN "ifconfig enp5s0f0 down > /dev/null"
#define MAX_RETRY 10
#define POLL_DELAY 100

extern int diag_smi_reg_test(void);
extern int diag_eth_ext_lpbk_test(void);
extern int esw_set_allports_forward(void);
extern int diag_esw_int_test(void);
#endif   /* __DIAG_ESW_TEST_H__ */

/*------------------------------------------------------------------
$Log: diag_esw_test.h,v $
Revision 1.4  2018/12/10 09:57:05  harrchan
Add workaround to PHY and Switch loopback test (CSCvn43011)

Revision 1.3  2018/10/11 06:02:59  harrchan
Add FPGA function test (CSCvm72986)

Revision 1.2  2018/08/06 02:31:50  harrchan
Merge viper E2E to the main trunk (CSCvk28469)

Revision 1.1.2.7  2018/07/03 05:38:55  harrchan
Follow the coding rule to clean up code

Revision 1.1.2.6  2018/05/11 02:22:11  harrchan
Changed interface name by using Cisco BIOS

Revision 1.1.2.5  2018/05/04 03:44:44  lucywang
Changed interface name by using Cisco BIOS c900-rommon.05022018.bin

Revision 1.1.2.4  2018/03/29 10:25:52  lucywang
Changed interface name by using Cisco BIOS

Revision 1.1.2.3  2018/03/28 07:03:51  lucywang
Added API to check SKU ViperJ and changed interface name for ViperJ

Revision 1.1.2.2  2018/03/23 06:36:17  olin2
Hide ifconfig message

Revision 1.1.2.1  2018/02/27 08:06:39  harrchan
Initial viper application code base


$Endlog$
*/

