/* $Id: skye_host.h,v 1.2 2015/05/25 00:41:20 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/skye_host.h,v $
 *------------------------------------------------------------------
 *
 * skye_host.h: Header file for Skye host side Diag.
 *
 * Jul 2014 - Paul Lin(palin2) ported from Shrinkray
 *
 * Copyright (c) 2015 by Cisco Systems, Inc.
 * All rights reserved.
 */
 
#include "ngio.h"

#ifndef SKYE_HOST_H
#define SKYE_HOST_H

#define SKYE_GE_BP_PACKET_NO    (10)
#define SKYE_ESC_CR_STRING      "\033\012"
#define SKYE_TURN_GE_LPBK       "skyenet -l\012"
#define SKYE_CR_STRING          "\012"
#define SKYE_RUN_DIAG           "skyenet\012"

/*
 * Based on HW info., Skye FPGA takes around 2 seconds to get ready
 * when both Golden and Upgraded image appearance.
 */
#define SKYE_FPGA_READY_TIME    2000   /* 2 seconds(=2000 ms) */

#define GMII_DMA
#define ETHER_PACKET_LEN_MAX        1514

#define SKYE_CPU0_IP_ADDR_SUBNET       "192.123.123"
#define SKYE_CPU1_IP_ADDR_SUBNET       "192.168.1"

/*******************************************************************************
 constants
*******************************************************************************/
#define TDM_CM_PASSWD 0xCAC00000
#define TDM_CM_ODRV   0x00008000

#define ST_FLASH_MANU_ID        0x20
#define ATMEL_FLASH_MANU_ID     0x1F
#define YETI3_CONFIG_RESET_TIME     50  /* seconds */
#define SKYE_RETRY              3

#define SKYE_1CPU_ID   0x0BF4
#define SKYE_2CPU_ID   0x0CAA

#define SKYE_LINUX_MENU              "/diag/skye_lnx\012"
#define SKYE_LINUX_NC                "/diag/skye_lnx -e\012"
#define SKYE_LINUX_TEST_CPU0_XAUI    "/diag/skye_lnx -j\012"
#define SKYE_LINUX_TEST_CPU1_XAUI    "/diag/skye_lnx -k\012"
#define SKYE_SET_TLK_1GKX_LBPK       "/diag/skye_lnx -l\012"
#define SKYE_DISABLE_TLK_1GKX_LBPK   "/diag/skye_lnx -m\012"
#define SKYE_CPU1_XAUI_PING_TEST     "/diag/skye_lnx -n\012"
#define SKYE_CPU0_XG_PING_TEST       "/diag/skye_lnx -o\012"
#define SKYE_LINUX_PROMPT       "#"
//#define SKYE_DIAG_MENU_PROMPT   "Main Diagnostic Menu item >"
#define SKYE_DIAG_STATUS        "skyelnx"
#define SKYE_DIAG_IP_ADDR_SUBNET "192.123.123"
#define SKYE_DIAG_IP_ADDR_BASE  (100)
#define SKYE_SET_FTP            "bootparam -dev tftp\012"
#define SKYE_SET_IMG            "bootparam -img /firmware/skye_img\012"
#define SKYE_SET_TFTPSERVER     "bootparam -host 192.123.123.1\012"
#define SKYE_SET_SOURCE         "bootparam root=/dev/gbe4\012"
#define SKYE_BOOT_CMD           "boot -tftp -host 192.123.123.1 -img /firmware/skye_img\012"
#define SKYE_REMOVE_MEM_ENV     "clearcfg\012"
#define SKYE_YES_STRING         "y\012"
#define SKYE_PING_SERVERIP      "ping 192.123.123.1\012"
#define SKYE_PING_TOUT          (120) /* 120 secs */
#define SKYE_BL_PROMPT_TOUT     (120) /* 120 secs */
#define SKYE_LINUX_PROMPT_TOUT  (120) /* 120 secs */
#define SKYE_DIAG_PROMPT_TOUT   (120)  /* 120 secs */
#define SKYE_DUAL_CPU_XAUI_TOUT (60)   /* 60 secs */
#define SKYE_TLK_LPBKBIT_TOUT   (60)   /* 60 secs */
#define SKYE_CR_STRING          "\012"
#define SKYE_DEST_DIAG_IMG  "/firmware/skye_img"
#define SKYE_SRC_DIAG_IMG   "skye_img"
#define SKYE_BL_PROMPT          "mboot"
#define SKYE_PING_ALIVE         "is alive!"
#define SKYE_UART_READ_TIMEOUT          (1) /* secs */
#define SKYE_POWER_UP_DELAY     (1000)
#define SKYE_CPU0_SET_IPADDR_MBOOT   "ifconfig gbe4 192.123.123.101 -mask 255.255.255.0 -up\012"
#define SKYE_CPU0_SET_IPADDR         "ifconfig gbe4 192.123.123.101 netmask 255.255.255.0\012"
#define SKYE_CPU1_SET_IPADDR_MBOOT   "ifconfig xgbe1 192.168.1.102 -mask 255.255.255.0 -up\012"
#define SKYE_CPU1_SET_IPADDR         "ifconfig xgbe1 192.168.1.102 netmask 255.255.255.0\012"
#define SKYE_CPU1_SET_ROUTE_MBOOT    "route add default -gw 192.168.1.101 -dev xgbe1\012"
#define SKYE_CPU1_SET_ROUTE          "route add default gw 192.168.1.101\012"
#define PASS_RESULTS            "pass"
#define DONE_RESULTS            "DONE"
#define CMD_LENGTH              256
#define PING10GKR               TRUE
#define SKYE_DIAG_IP_ADDR_BASE_XG    (200)
#define CONFIG_XGBE2_DOWN       "ifconfig xgbe2 down\012"
#define CONFIG_GBE4_UP          "ifconfig gbe4 up\012"
#define CONFIG_GBE4_DOWN        "ifconfig gbe4 down\012"
#define CHECK_XG_OUTPUT         "cat /diag/skye_check_xg_config_output.txt\012"
#define CHECK_PING_OUTPUT       "cat /diag/skye_check_ping_output.txt\012"

/*******************************************************************************
 I2C Expander addresses
*******************************************************************************/

#define PCA9555_I2C_ADDRESS         0x20    /* 16-bit I/O expander (bits 7-4) */

/* Skye GPIO Expander Definition */
#define SKYE_DB_PRESENT     0x01   /* IO Port 0.0 */
#define SKYE_BOOT_SEL       0x02   /* IO Port 0.1 */
#define SKYE_SUBMOD_RESET   0x04   /* IO Port 0.2 */
#define SKYE_PRI_INF_RDY    0x08   /* IO Port 0.3 */
#define SKYE_UART_MUX_SEL   0x10   /* IO Port 0.4 */
#define SKYE_RESET_CONF     0x20   /* IO Port 0.5 */
/* IO Port 0.6 & IO Port 0.7 are undefined in Skye */

#define SKYE_E0_10G_CAP     0x01   /* IO Port 1.0 */
#define SKYE_E1_10G_CAP     0x02   /* IO Port 1.1 */
/* IO Port 1.2 to IO Port 1.7 are undefined in Skye */

#define BIT0      0x01
#define BIT1      0x02
#define BIT2      0x04
#define BIT3      0x08
#define BIT4      0x10
#define BIT5      0x20
#define BIT6      0x40
#define BIT7      0x80

/* PCA9555 Definition */
#define PCA9555_IN_PORT0_REG            0x00
#define PCA9555_IN_PORT1_REG            0x01
#define PCA9555_OUT_PORT0_REG           0x02
#define PCA9555_OUT_PORT1_REG           0x03
#define PCA9555_POLAR_INV_P0_REG        0x04
#define PCA9555_POLAR_INV_P1_REG        0x05
#define PCA9555_CFG_PORT0_REG           0x06
#define PCA9555_CFG_PORT1_REG           0x07

#define PCA9555_PORT0_MASK              0xFF
#define PCA9555_PORT1_MASK              0xFF
#define PCA9555_PORT0_INIT              0x00
#define PCA9555_PORT1_INIT              0x00

#define PCA9555_IO_INPUT                0x1
#define PCA9555_IO_OUTPUT               0x0
#define PCA9555_IO_HIGH                 0x1
#define PCA9555_IO_LOW                  0x0

#define SKYE_CMD        0
#define SKYE_DATA       1
#define SKYE_ACK        2
#define SKYE_RESULT     4


#define TEST_OK                                  0x40
#define TEST_ACK                                 0x80
#define TEST_FAILED                              0xC0


#define FROM_HOST_SWITCH_CONSOLE                 0x01
#define FROM_HOST_CPU_ALIVE_TEST                 0x02
#define FROM_HOST_WRITE_MAC_ADDR                 0x03

/*ACK*/
#define TO_HOST_SWITCH_CONSOLE_ACK \
        (FROM_HOST_SWITCH_CONSOLE + TEST_ACK)
#define TO_HOST_CPU_ALIVE_TEST_ACK \
        (FROM_HOST_CPU_ALIVE_TEST + TEST_ACK)

/*OK*/
#define TO_HOST_SWITCH_CONSOLE_OK \
        (FROM_HOST_SWITCH_CONSOLE + TEST_OK)
#define TO_HOST_CPU_ALIVE_TEST_OK \
        (FROM_HOST_CPU_ALIVE_TEST + TEST_OK)

/*FAIL*/
#define TO_HOST_SWITCH_CONSOLE_FAIL \
        (FROM_HOST_SWITCH_CONSOLE + TEST_FAILED)
#define TO_HOST_CPU_ALIVE_TEST_FAIL \
        (FROM_HOST_CPU_ALIVE_TEST + TEST_FAILED)

/*******************************************************************************
 typedefs
*******************************************************************************/
/*
 * this command enum must matches with the testlist table defined in
 * Skye SM side skye_test.h
 */
enum {
    NO_CMD = 0,
    MIC_REG_TEST,
    MIC_RING_TEST,
    MIC_MEM_TEST,
    ELMER_REG_TEST,
    SYS_SDRAM_TEST,
    STR_REG_TEST,
    STR_MABIST_TEST,
    STR_INT_MEM_TEST,
    STR_PSRO_TEST,
    STR_LBIST_TEST,
    STR_PRBS_TEST,
    STR_EXT_SRAM_TEST,
    MM_REG_TEST,
    MM_MEM_TEST,
    MM_BIST_TEST,
    CAM_BIST_TEST,               /* 0x10 */
    CAM_MEM_TEST,
    MIC_FRAME_LPBK_TEST,
    SASQ_SNEAK_PORT_TEST,
    SASQ_MAC_LPBK_TEST,
    ETH_PHY_LPBK_TEST,
    ETH_EXT_LPBK_TEST,
    REDB_SERDES_INF_TEST,
    SASQ_YUKON_LPBK_TEST,
    SFP_SERDES_LPBK_TEST,
    INSIDE_RING_LPBK_TEST,
    ILP_REG_TEST,
    ILP_TEST_SUBMENU,
    MIRAGE_TEST_ALL_TEST,
    MIRAGE_LED_TEST,
    MIRAGE_MISC_TEST,
    EXIT,  /* Always the last? */
};

enum {
    NO_ACK  = 0,
    CMD_ACK,
    ACK_ACK,
    TEST_DONE,
    TEST_FAIL,
};


#define RC_DATA_SIZE    (2 * UART_RD_SIZE + 1)
#define CON_BUF_SIZE    400
#define MAX_BUF_SIZE    1470   /* Maximum buffer size to have a total ethernet
                                  packet of 1518 bytes */
#define SKYE_SLOT1    1
#define SKYE_SLOT2    2

#define VAXORDER(lword)  \
          ((((uint32)lword & 0x000000ff) << 24) | (((uint32)lword & 0x0000ff00) << 8) | \
           (((uint32)lword & 0x00ff0000) >> 8)  | (((uint32)lword & 0xff000000) >> 24))


/*******************************************************************************
 macros
*******************************************************************************/

/*******************************************************************************
 prototypes
*******************************************************************************/
int skye_sm_test(void *);

extern ulong get_pci_device_base_offset(uint slot, uint dev_num);
extern ulong get_pci_device_base(uint slot, uint pm_device);
extern int  do_all_menu_items(struct menuinfo *);
extern PFV  put_sm_isr_vect (int sm_slot, uchar intr_level, PFV isr_vect);
extern int  get_ge_sw_port_num(int, int);
extern int  setup_sw_dev(uint, uint, uint);
extern int  mvl_sw_cleanup(int, int);
extern void reset_sm_module(int, boolean);
extern int  get_real_slot(int);
extern ushort get_nm_id(int);
extern int api_platform_feature(void);
extern int set_gesw_line_loopback(int , int);
extern int get_gesw_line_loopback(int);
extern uint32_t check_poe_psu_present(uint32_t , uint32_t);
extern void ngiosm_disable_intr(int, int );

/*
 * Motherboard features defines. Port from cross_platform.h
 */
#define INT_48V_SUPPORT		0x00000001	/* Internal -48 volts support */

/* IO Expender Port Number */
typedef enum {
    IO0_0 = 1,              /* IO Port 0.0 */
    IO0_1,                  /* IO Port 0.1 */
    IO0_2,                  /* IO Port 0.2 */
    IO0_3,                  /* IO Port 0.3 */
    IO0_4,                  /* IO Port 0.4 */
    IO0_5,                  /* IO Port 0.5 */
    IO0_6,                  /* IO Port 0.6 */
    IO0_7,                  /* IO Port 0.7 */
    IO1_0,                  /* IO Port 1.0 */
    IO1_1,                  /* IO Port 1.1 */
    IO1_2,                  /* IO Port 1.2 */
    IO1_3,                  /* IO Port 1.3 */
    IO1_4,                  /* IO Port 1.4 */
    IO1_5,                  /* IO Port 1.5 */
    IO1_6,                  /* IO Port 1.6 */
    IO1_7,                  /* IO Port 1.7 */   
    IO_PORT_MAX,            /* MAX IO Port */
} PCA9539_PIO_PORTS;  


/* CPU 0 or CPU 1 */
enum {
    CPU0 = 0,
    CPU1,
};
#ifdef NOT_USED
/*
 * Ehternet packet 
 */
 
typedef uchar           mac_addr_t[6];
typedef struct {
    mac_addr_t  dest_addr;
    mac_addr_t  src_addr;
    ushort      pkt_len;
} ether_hdr_t;    

typedef struct fe_packet_t {
    ether_hdr_t eth_hdr;
    uchar       pad_data[2];
//    uchar       data[1500];
    uint       data[250];
} fe_packet_t;

struct  ether_addr {
    uchar  ether_addr_octet[6];
};

struct  ether_header {
    struct ether_addr   ether_dhost;
    struct ether_addr   ether_shost;
    ushort             ether_type;
};
typedef struct  ether_header  ether_header_t;

typedef struct {
    ulong   op_type;      /* type of operation, request or response     */
    ulong   cmd;          /* command                                    */
    ulong   ret_code;     /* return code, ACK, PASS, FAIL, etc          */
    ulong   platform;     /* router platform type                       */
    ulong   param1;
    ulong   param2;
    ulong   param3;
    ulong   nvram_val;
} if_info_t;              /* header above                               */

#define MAX_BUF_SIZE    1470   /* Maximum buffer size to have a total ethernet
                                  packet of 1518 bytes */
typedef struct {
    ether_header_t ether_hdr;  /* 14 bytes */
    ushort      pad;
    if_info_t  if_info;
    uchar   if_buf[MAX_BUF_SIZE]; /* buffer contains data for host/ce intf */
} if_ether_t;
#endif

typedef struct skye_ds {
    ushort  cookie_id;
    uchar   slot;
    uchar   uart;
    ulong   num_asic;    /* max number of Sasquatch switches */
    ulong   num_ports;   /* Number of FE ports on board */
    ulong   num_titanium; /* Number of power controllers onboard */
    ulong   num_sfp;
    ulong   host_pci_base_addr;
    ulong   nm_pci_base_addr;
    ulong   skye_ds_addr;
    uchar   fpga_downloaded[MAX_SM+1];
    uchar   fw_downloaded[MAX_SM+1];
    uchar   b_name[30];
    uchar   testname[50];
    int     ge_in_port;
    int     ge_out_port;
    ulong   cmd_tx_buf;
    ulong   cmd_rx_buf;
    uchar tx_pak[ETHER_PACKET_LEN_MAX];     /* Ethernet tx packet buffer */
    uchar rx_pak[ETHER_PACKET_LEN_MAX];     /* Ethernet rx packet buffer */
    //ether_header_t ether_hdr;
    struct ngio_intf_t *skye_sm_iface;
} skye_ds_t;

/*
 * NC Command
 */
/* NC Command Data Structure */
struct nc_command {
    char *cmd_str;
    long (*func)(char *);
};

/* Common Definitions */
#define SKYE_NC_CONN_ITVL                 1
#define DIAG_RTN_PASS_STR                 "PASS"

/* Skye NC file data path definition */
#define DIAG_KILL_NC_TMP_FILE             "/tmp/skye_rm.pid"
#define DIAG_CMD_DISPATCH_FILE            "/tmp/skye_comm_dispatch"
#define SKYE_NC_RET_VAL_FILE              "/tmp/skye_nc_ret_val"
#define SKYE_NC_DONE_FP                   "/tmp/skye_nc_done"
#define SKYE_NC_DBLOG_HOST_FILE           "/tmp/skye_nc_dblog.txt"
#define SKYE_PING_TEST                    "/tmp/skye_ping_result.txt"
#define SKYE_TMP_FM_FILE                  "/tmp/skye_fm"

/* Skye NC port base definition */
#define DIAG_RUN_ALL_PORT_BASE            (2390)
#define DIAG_RTN_STS_OUT_PORT_BASE        (2391)
#define DIAG_RTN_DBLOG_PORT_BASE          (2392)
#define DIAG_EXEC_CMD_TRANS_PORT_BASE     (2398)
#define DIAG_EXEC_CMD_PORT_BASE           (2399)
#define SKYE_NC_DONE_PORT                 (2400)

/* NC Command Dispatch */
#define DIAG_CMD_ALIVE_CHECK              "alive_check"
#define DIAG_DO_ALL_TEST                  "do_all_test"
#define DIAG_DO_MEM_TEST                  "do_mem_test"
#define DIAG_DO_FPGA_TEST                 "do_fpga_test"
#define DIAG_DO_SPIROM_TEST               "do_spirom_test"
#define DIAG_DO_I2CDEV_TEST               "do_i2cdev_test"
#define DIAG_DO_TLK_TEST                  "do_tlk_test"
#define DIAG_DO_PCIE_TEST                 "do_pcie_test"


/*******************************************************************************
 *                                   Externs
 *******************************************************************************
 */
extern int  skye_setup_ge_env(skye_ds_t *);
extern int  skye_send_cmd(skye_ds_t *, uchar, int);
extern int  skye_cleanup_ge_env(skye_ds_t *);
extern void skye_clear_rx_buf(void);
extern int  set_promisc(char *, int);
extern int  get_sm_mac_addr(int, uchar *);
extern int  get_ctrl_plane_sgmii_port(void);
extern void skye_get_ip_addr(char *, int);
extern void skye_nc_dispatch_comm(char *, int);

#endif		/* SKYE_HOST_H */


/*------------------------------------------------------------------
 * $Log: skye_host.h,v $
 * Revision 1.2  2015/05/25 00:41:20  steja
 * Add support Skye SM
 *
 * Revision 1.1.4.4  2015/05/05 11:52:57  steja
 * CDETS[CSCuu01237] Solving TLK intermittent loopback issue on GH platform.
 *
 * Revision 1.1.4.3  2015/04/29 13:28:00  steja
 * Update define
 *
 * Revision 1.1.4.2  2015/04/29 11:47:57  steja
 * Code check-in to skye-branch2 for ER code review
 *
 * Revision 1.1.2.18  2015/03/26 08:33:24  steja
 * Debug edvt found issue on 2CPU skye Dual CPU Xaui Test
 *
 * Revision 1.1.2.17  2015/03/09 09:02:30  steja
 * Fix boot up process failed on skye 2 CPU, 2nd CPU ping failed.(CSCut26710)
 *
 * Revision 1.1.2.16  2015/02/13 05:29:04  palin2
 * Added "SKYE_FPGA_READY_TIME" definition for Skye common usage.
 *
 * Revision 1.1.2.15  2015/01/26 01:11:40  steja
 * Add function for frequency margin to host side menu utilities through NC
 *
 * Revision 1.1.2.14  2014/11/27 09:46:32  steja
 * Update UART timeout and skyelnx string
 *
 * Revision 1.1.2.13  2014/11/27 07:24:10  palin2
 * Added PCIe lanes Scan test to 2-CPUs Skye default tests.
 *
 * Revision 1.1.2.12  2014/11/27 02:31:00  steja
 * Fix the intermittent failure to run do all test(CSCur27613)
 *
 * Revision 1.1.2.11  2014/10/07 13:52:52  steja
 * Modify the skye is up function
 *
 * Revision 1.1.2.10  2014/10/01 08:12:22  palin2
 * Merged NC command related definition(skye_comm_lib.h) to here.
 *
 * Revision 1.1.2.9  2014/09/26 09:05:42  steja
 * (CSCuq98591)Fix GBE4 link issue
 *
 * Revision 1.1.2.8  2014/09/18 07:13:24  steja
 * Update for 2 CPU cookie
 *
 * Revision 1.1.2.7  2014/09/12 14:36:56  steja
 * Update code for uart and nc configuration
 *
 * Revision 1.1.2.6  2014/09/09 09:02:07  steja
 * Add skye rx uart to print the test progress.
 *
 * Revision 1.1.2.5  2014/09/04 12:51:53  steja
 * Enhanced 10GKR support capability
 *
 * Revision 1.1.2.4  2014/09/01 15:41:32  steja
 * Fix the Firmware download, need to config IP on mboot
 *
 * Revision 1.1.2.3  2014/08/28 02:53:48  steja
 * Support Do all test for NC command
 *
 * Revision 1.1.2.2  2014/08/15 03:26:50  palin2
 * Initial check-in to support NC command on Skye.
 *
 * Revision 1.1.2.1  2014/07/17 06:32:00  palin2
 * Added Skye ID.
 *
 *------------------------------------------------------------------
 * shrinkray_host.h:
 * Revision 1.2.8.3  2014/06/27 15:22:03  steja
 * Coding for config iptable
 *
 * Revision 1.2.8.2  2014/06/25 13:11:43  steja
 * Add Tftpdownload Skye Firmware
 *
 * Revision 1.2.8.1  2014/05/11 07:53:24  steja
 * Update code for future use
 *
 * Revision 1.2  2014/03/03 06:33:43  palin2
 * -Initial check-in Skye host side Diag.
 *
 * Revision 1.1.4.2  2014/02/28 08:31:54  steja
 * Fix the GE Backplane loopback fail
 *
 * Revision 1.1.4.1  2014/02/26 11:08:18  palin2
 * -To support Skye host side tests on O2.
 * -This branch is created to pick up O2 main tunk code changes.
 *
 * Revision 1.1.2.7  2014/01/27 08:51:06  steja
 * Code clean up
 *
 * Revision 1.1.2.6  2013/10/09 10:17:22  palin2
 * Correct Skye ID.
 *
 * Revision 1.1.2.5  2013/09/24 00:35:38  palin2
 * Removed unnecessary definition and Added Skye ID.
 *
 * Revision 1.1.2.4  2013/08/17 04:32:50  steja
 * Add code command and respond ( Host <->GE <-> TILE CPU#0) for O2 platform
 *
 * Revision 1.1.2.3  2013/07/24 02:26:36  iachang
 * Support Console Switch with CPU0 & CPU1
 *
 * Revision 1.1.2.2  2013/07/08 08:49:16  steja
 * Add GE backplane loopback test (Host <->TLK10232)
 *
 * Revision 1.1.2.1  2013/05/22 02:43:51  palin2
 * Initial check-in to add Skye host side Diag support on O2.
 *
 *------------------------------------------------------------------
 * $Endlog$
 */

