/* $Id: mgmt_port.h,v 1.2 2017/08/02 14:21:47 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/mgmt_port.h,v $
 *------------------------------------------------------------------
 * Filename:    mgmtphy.h
 *
 * Description:
 *
 * Copyright (c) 2017 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __MGMTPHY_HEADER__
#define __MGMTPHY_HEADER__

#define _STR(s) #s
#define STR(s) _STR(s)



#define MAX_COMMAND_LENGTH 2048
#define MAX_PATH_LENGTH 1024
#define MAX_FILENAME_LENGTH 255


#define ENABLE_CONSOLE_MSG "dmesg -E >/dev/null 2>&1"
#define DISABLE_CONSOLE_MSG "dmesg -D >/dev/null 2>&1"
#define DISABLE_IPV6_CMD "echo 1 > /proc/sys/net/ipv6/conf/all/disable_ipv6"

/*
'xgene_enet' proc control usage:

echo <eth_dev_name> <command> <block> [<reg> <value> | <value>] > /proc/xgene_enet

 where eth_dev_name: eth0, eth1, etc...
    command:
         0 for read
         1 for write
   block ID:
         12 MAC LOOPBACK SET
*/

#define MGMTPHY_PROC_FILE_NAME "xgene_enet"
#define MGMTPHY_PROC_COMMAND_READ  0
#define MGMTPHY_PROC_COMMAND_WRITE 1
#define MGMTPHY_PROC_MACLOOPBACK 12
#define MGMTPHY_PROC_MODE_DISABLE 0
#define MGMTPHY_PROC_MODE_ENABLE 1

/*
'mv88e1510' sys control usage:

DIAG_MGMT_PHY_EXT_LOOPBACK: Format:1 {0(Disable)|1(Enable)}
DIAG_MGMT_PHY_INT_LOOPBACK: Format:2 {0(Disable)|1(Enable)}
DIAG_MGMT_PHY_TRAFFIC     : Format:3
DIAG_MGMT_PHY_READ         : Format:4 <Reg> <PhyPage> 
DIAG_MGMT_PHY_WRITE        : Format:5 <Reg> <Value(hex)> <PhyPage> 

*/

#define MGMTPHY_SYS_FILE_NAME "mv88e1510"

#define MGMTPHY_SYS_EXTLOOPBACK 1
#define MGMTPHY_SYS_INTLOOPBACK 2
#define MGMTPHY_SYS_TRAFFIC_TEST 3
#define MGMTPHY_SYS_REGISTER_READ 4
#define MGMTPHY_SYS_REGISTER_WRITE 5
#define MGMTPHY_SYS_REGISTER_TEST 6
#define MGMTPHY_SYS_MODE_DISABLE 0
#define MGMTPHY_SYS_MODE_ENABLE 1

/* APM's RGMII-0 default interface enumerated number in Linux */
#define DEFAULT_MGMTPHY_RGMII_ID 0


#define TSN_LOOPBACK_MGMTPHY_PACKET_NO 1


/* Type definition for MGMT PHY register */
typedef struct mgmtphy_reg {
    int    page;
    int    s_offset;
    int    e_offset;
} mgmtphy_reg_t;


/*-----------------------------------------------------------------------
 *  Externs                                                             *
 *----------------------------------------------------------------------*/

int mgmtphy_is_present (void);
int mgmtphy_mac_lpbk_test (void);
int mgmtphy_internal_lpbk_test(int);
int mgmtphy_external_lpbk_test(int);
int mgmtphy_traffic_test (void);
int mgmtphy_register_access_test(void);
int mgmtphy_subsystem_test(int);
int mgmtphy_read_register(void);
int mgmtphy_write_register(void);
extern int mgmtphy_dump_all_reg(void);

#endif  /* __MGMTPHY_HEADER__ */

/*------------------------------------------------------------------
$Log: mgmt_port.h,v $
Revision 1.2  2017/08/02 14:21:47  steja
Support TSN-H/M platform code

Revision 1.1.8.2  2017/07/29 03:41:11  steja
tsn-branch5 synced with Maintrunk repositories

Revision 1.1.6.2  2017/07/20 13:38:06  steja
tsn-branch4 merge with maintrunk

Revision 1.1.4.2  2016/06/30 06:22:49  steja
tsn-branch2 sync with main trunk

Revision 1.1.2.1  2016/03/21 02:56:06  steja
Add debug card test items


$Endlog$
*/

