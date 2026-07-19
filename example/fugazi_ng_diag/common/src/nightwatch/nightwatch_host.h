/* $Id: nightwatch_host.h,v 1.2 2019/08/06 06:56:09 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nightwatch/nightwatch_host.h,v $
 *------------------------------------------------------------------
 *
 * nightwatch_host.h: main header file for Nightwatch host diag.
 *
 * May. 2018 - Mingchun Ding(Ported from EagleEye)
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 * Original Author: Ian Chang
 */

#include "ngio.h"

#ifndef NIGHTWATCH_HOST_H
#define NIGHTWATCH_HOST_H

#define GMII_DMA
#define ETHER_PACKET_LEN_MAX        1514

/*******************************************************************************
 constants
*******************************************************************************/
#define TDM_CM_PASSWD 0xCAC00000
#define TDM_CM_ODRV   0x00008000

#define XAUI_PNUM     2
#define KRXG_PNUM     0
#define GE0_PNUM      nightwatch_info_p->xge_port
#define GE1_PNUM      1

#define ST_FLASH_MANU_ID        0x20
#define ATMEL_FLASH_MANU_ID     0x1F
#define NIGHTWATCH_RETRY        3

#define NIGHTWATCH_TEST_ALL_TEST      MIRAGE_TEST_ALL_TEST
#define NIGHTWATCH_TEST_MAX_WAIT_TIME 5000
#define NIGHTWATCH_WAIT_TIME_PER_TEST 120
#define NIGHTWATCH_WAIT_RETRY         80
#define NIGHTWATCH_POWER_CYCLE_SECOND 10

#define NULL_FUNC   (type_t(*)())0

enum {
    NWK_OVERLORD,
    NWK_NEPTUNE,
    NWK_UTAH_GH,
    NWK_UTAH,
    NWK_CURIE_1RU
};

enum {
    NWK_24_PORT,
    NWK_48_PORT
};

#define NWK_SET_PARAM(F,v) \
{\
    F(nightwatch_submenu_table[5].x_pparam, v);      \
    F(nightwatch_submenu_table[5].x_sparam, v);      \
    F(nightwatch_submenu_table[4].x_pparam, v);      \
    F(nightwatch_submenu_table[4].x_sparam, v);      \
    F(nightwatch_submenu_table[1].x_pparam, v);      \
    F(nightwatch_submenu_table[1].x_sparam, v);      \
    F(nightwatch_submenu_table[7].x_pparam, v);      \
}
/* stardustFlag - the first parameter of module's diag apaplication
 * Refer to https://wiki.cisco.com/display/DIAGS/Nightwatch
 */
#define NWK_DFLAG_AUTO_RUN          0x200
#define NWK_DFLAG_SET_SLOT(_f,_s)   _f = ((_f & ~0xF) | ((_s)&0xF))
#define NWK_DFLAG_SET_HOST(_f,_t)   _f = ((_f & ~0xF000) | (((_t)&0xF)<<12))
#define NWK_DFLAG_SET_MODE(_f,_t)   _f = ((_f & ~0x100) | (((_t)&1)<<8))
#define NWK_DFLAG_SET_BUS(_f,_b)    _f = ((_f & ~0xFFFF0000) | (((_b)&0xFFFF)<<16))

/*******************************************************************************
 I2C Expander addresses
*******************************************************************************/

#define PCA9555_I2C_ADDRESS         0x20    /* 16-bit I/O expander (bits 7-4) */

typedef enum {
    PCA9555_GPIO0_REGISTER = 0,             /* IO Port 0.0 */
    PCA9555_BOOT_SE_REGISTER,               /* IO Port 0.1 */
    PCA9555_SYS_RET_REGISTER,               /* IO Port 0.2 */
    PCA9555_IN_RDY_REGISTER,                /* IO Port 0.3 */
    PCA9555_GPIO4_REGISTER,                 /* IO Port 0.4 */
    PCA9555_MODE_REGISTER,                  /* IO Port 0.5 */
    PCA9555_CGPIO6_REGISTER,                /* IO Port 0.6 */
    PCA9555_DETECT_REGISTER,                /* IO Port 0.7 */
    PCA9555_GPIO8_REGISTER,                 /* IO Port 1.0 */
    PCA9555_GPIO9_REGISTER,                 /* IO Port 1.1 */
    PCA9555_SYS_RET2_REGISTER,              /* IO Port 1.2 */
    PCA9555_GPIO11_REGISTER,                /* IO Port 1.3 */
    PCA9555_I2C_RET_REGISTER,               /* IO Port 1.4 */
    PCA9555_GPIO13_REGISTER,                /* IO Port 1.5 */
    PCA9555_PWR_OFF_REGISTER,               /* IO Port 1.6 */
    PCA9555_GPIO15_REGISTER,                /* IO Port 1.7 */
    PCA9555_IO_PORT_MAX,                    /* MAX IO Port */
} PCA9555_PIO_PORTS;

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

/*******************************************************************************
 typedefs
*******************************************************************************/
/*
 * this command enum must matches with the testlist table defined in
 * Nightwatch SM side nightwatch_test.h
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
#define NIGHTWATCH_SLOT1    1
#define NIGHTWATCH_SLOT2    2

#define NIGHTWATCH_B115200    0 /* Baudrate (0-115200, 1-9600)*/
#define NIGHTWATCH_B9600      1 /* Baudrate (0-115200, 1-9600)*/

#define VAXORDER(lword)  \
          ((((uint32)lword & 0x000000ff) << 24) | (((uint32)lword & 0x0000ff00) << 8) | \
           (((uint32)lword & 0x00ff0000) >> 8)  | (((uint32)lword & 0xff000000) >> 24))

enum {
    NIGHTWATCH_24P_ID = NGSM_NWK24,
    NIGHTWATCH_48P_ID = NGSM_NWK48,
};

/*******************************************************************************
 macros
*******************************************************************************/

/*******************************************************************************
 prototypes
*******************************************************************************/
int nightwatch_sm_test(void *);

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
extern int sgmii_lpbk_util(int, int);
extern int tftp_get(unsigned char *, unsigned char *, unsigned char *,
        unsigned char *, unsigned int);

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


typedef struct nightwatch_ds {
    ushort  cookie_id;
    uchar   slot;
    uchar   uart;
    ulong   num_asic;    /* max number of Sasquatch switches */
    ulong   num_ports;   /* Number of FE ports on board */
    ulong   num_titanium; /* Number of power controllers onboard */
    ulong   num_sfp;
    ulong   host_pci_base_addr;
    ulong   nm_pci_base_addr;
    ulong   nwk_ds_addr;
    uchar   fpga_downloaded[MAX_SM+1];
    uchar   fw_downloaded[MAX_SM+1];
    uchar   b_name[30];
    char   testname[50];
    int     ge_in_port;
    int     ge_out_port;
    ulong   cmd_tx_buf;
    ulong   cmd_rx_buf;
    uchar tx_pak[ETHER_PACKET_LEN_MAX];     /* Ethernet tx packet buffer */
    uchar rx_pak[ETHER_PACKET_LEN_MAX];     /* Ethernet rx packet buffer */
    ether_header_t ether_hdr;
    struct ngio_intf_t *ngio_iface;
    int     xge_port;                    /* 0 for Host GE0, 2 for Host XAUI */
} nightwatch_ds_t;
#endif		/* NIGHTWATCH_HOST_H */

/*-------------------------------------------------
$Log: nightwatch_host.h,v $
Revision 1.2  2019/08/06 06:56:09  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.1.2.2  2019/07/25 02:46:34  mingding
CSCvk64124-34: Add sub-menu item to lock golden image on modules' boot-flash

Revision 1.1.2.1  2019/05/30 05:33:34  mingding
CSCvk64124-29: Support PCIe-based Nightwatch Server Module

*/
