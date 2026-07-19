/* $Id: shrinkray_host.h,v 1.2 2014/03/03 06:33:43 palin2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/shrinkray_host.h,v $
 *------------------------------------------------------------------
 *
 * shrinkray_host.h: Header file for ShrinkRay host side Diag.
 *
 * May 2013 - Paul Lin(palin2) ported from Lebowski
 *
 * Original Author: Ian Chang
 *
 * Copyright (c) 2013-2014 by Cisco Systems, Inc.
 * All rights reserved.
 */
 
#include "ngio.h"

#ifndef SHRINKRAY_HOST_H
#define SHRINKRAY_HOST_H

#define SHRINKRAY_GE_BP_PACKET_NO    (10)
#define SHRINKRAY_ESC_CR_STRING      "\033\012"
#define SHRINKRAY_TURN_GE_LPBK       "shrinkraynet -l\012"
#define SHRINKRAY_CR_STRING          "\012"
#define SHRINKRAY_RUN_DIAG           "shrinkraynet\012"

#define GMII_DMA
#define ETHER_PACKET_LEN_MAX        1514

/*******************************************************************************
 constants
*******************************************************************************/
#define TDM_CM_PASSWD 0xCAC00000
#define TDM_CM_ODRV   0x00008000

#define ST_FLASH_MANU_ID        0x20
#define ATMEL_FLASH_MANU_ID     0x1F
#define YETI3_CONFIG_RESET_TIME     50  /* seconds */
#define SHRINKRAY_RETRY              3

#define SHRINKRAY_ID   0x0BF4

/*******************************************************************************
 I2C Expander addresses
*******************************************************************************/

#define PCA9555_I2C_ADDRESS         0x20    /* 16-bit I/O expander (bits 7-4) */

/* ShrikRay GPIO Expander Definition */
#define SRINKRAY_DB_PRESENT     0x01   /* IO Port 0.0 */
#define SRINKRAY_BOOT_SEL       0x02   /* IO Port 0.1 */
#define SRINKRAY_SUBMOD_RESET   0x04   /* IO Port 0.2 */
#define SRINKRAY_PRI_INF_RDY    0x08   /* IO Port 0.3 */
#define SRINKRAY_UART_MUX_SEL   0x10   /* IO Port 0.4 */
#define SRINKRAY_RESET_CONF     0x20   /* IO Port 0.5 */
/* IO Port 0.6 & IO Port 0.7 are undefined in ShinkRay */

#define SRINKRAY_E0_10G_CAP     0x01   /* IO Port 1.0 */
#define SRINKRAY_E1_10G_CAP     0x02   /* IO Port 1.1 */
/* IO Port 1.2 to IO Port 1.7 are undefined in ShinkRay */

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

#define SHRINKRAY_CMD        0
#define SHRINKRAY_DATA       1
#define SHRINKRAY_ACK        2
#define SHRINKRAY_RESULT     4


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
 * Shrinkray SM side shrinkray_test.h
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
#define SHRINKRAY_SLOT1    1
#define SHRINKRAY_SLOT2    2

#define VAXORDER(lword)  \
          ((((uint32)lword & 0x000000ff) << 24) | (((uint32)lword & 0x0000ff00) << 8) | \
           (((uint32)lword & 0x00ff0000) >> 8)  | (((uint32)lword & 0xff000000) >> 24))


/*******************************************************************************
 macros
*******************************************************************************/

/*******************************************************************************
 prototypes
*******************************************************************************/
int shrinkray_sm_test(void *);

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

#ifdef PSE2_TESTCARD
extern int  phy_init(dev_object_t *dev);
extern void	phy_1112_remove(void);
extern int  phy_set_mode(uint32_t mode);
extern int  phy_set_loopback(uchar mode);
extern void dev_phy_platform_detach(void);
#endif
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


typedef struct shrinkray_ds {
    ushort  cookie_id;
    uchar   slot;
    uchar   uart;
    ulong   num_asic;    /* max number of Sasquatch switches */
    ulong   num_ports;   /* Number of FE ports on board */
    ulong   num_titanium; /* Number of power controllers onboard */
    ulong   num_sfp;
    ulong   host_pci_base_addr;
    ulong   nm_pci_base_addr;
    ulong   shrinkray_ds_addr;
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
    ether_header_t ether_hdr;
    struct ngio_intf_t *shrinkray_sm_iface;
} shrinkray_ds_t;


/*******************************************************************************
 prototypes
*******************************************************************************/
extern int shrinkray_setup_ge_env(shrinkray_ds_t *);
extern int shrinkray_send_cmd(shrinkray_ds_t *, uchar, int);
extern int shrinkray_cleanup_ge_env(shrinkray_ds_t *);
extern void shrinkray_clear_rx_buf(void);
extern int set_promisc(char *, int);
extern int get_sm_mac_addr(int, uchar *);
extern int get_ctrl_plane_sgmii_port(void);

#endif		/* SHRINKRAY_HOST_H */


/*------------------------------------------------------------------
 * $Log: shrinkray_host.h,v $
 * Revision 1.2  2014/03/03 06:33:43  palin2
 * -Initial check-in ShrinkRay host side Diag.
 *
 * Revision 1.1.4.2  2014/02/28 08:31:54  steja
 * Fix the GE Backplane loopback fail
 *
 * Revision 1.1.4.1  2014/02/26 11:08:18  palin2
 * -To support ShrinkRay host side tests on O2.
 * -This branch is created to pick up O2 main tunk code changes.
 *
 * Revision 1.1.2.7  2014/01/27 08:51:06  steja
 * Code clean up
 *
 * Revision 1.1.2.6  2013/10/09 10:17:22  palin2
 * Correct ShrinkRay ID.
 *
 * Revision 1.1.2.5  2013/09/24 00:35:38  palin2
 * Removed unnecessary definition and Added ShrinkRay ID.
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
 * Initial check-in to add ShrinkRay host side Diag support on O2.
 *
 *------------------------------------------------------------------
 * $Endlog$
 */

