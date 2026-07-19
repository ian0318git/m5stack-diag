/* $Id: nim_f35.h,v 1.5 2018/05/18 09:24:48 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/nim_f35.h,v $
 *******************************************************************************
 *
 * nim_f35.h - F35 Header file
 *
 * Jan 2014, Smita Rane
 *
 * Copyright (c) 2014-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *******************************************************************************
 */

typedef struct __attribute__((__packed__)) f35_test_menu_ {
    char  *desc;
    uint  msg_id;
    uint  cmd_id;
    uint  msg_len;
} f35_test_menu_t;

/* msg_id defines */
#define NO_MSG                         0
#define KW_MSG                         1
#define F35_CMD                        2
#define CON_CMD                        3
/* KW has decoded the command and has ACK it - step 2 */
#define KW_CMD_RESP                    4   
/* KW has decoded the command and has NACK it - step 2 */
#define F35_CMD_ACK                    5   
/* KW received command and is processing it - step 1*/
#define KW_PROC_CMD                    6   
/* KW has executed command and is responding with pass - step 3 */
#define KW_CMD_PASS                    7   
/* KW has executed command and is responding with fail - step 3 */
#define KW_CMD_FAIL                    8   
/* KW has errors in IPC sync get_ipc_cmd() - step 3 */
#define KW_IPC_FAIL                    9   
#define KW_READY_FOR_CMD               10   
#define F35_CMD_NACK                   11   
/* KW received IPC command that is not supported */
#define F35_CMD_NO_SUPPORT             12  
/* KW expected IPC command but receive not specified msg_id */
#define NOT_F35_CMD                    13  
    



/*
 * KW Tests defines 
 * invoked by IPC command (intr/poll), or command line 
 * cmd_id
 */
typedef enum kw_diag_tests_ {
    /*  
       Host : IPC command (intr/poll)
       KW   : -"- , command line
    */
    F35_INT_REG_TEST=1,
    DRMW_FPGA_REG_TEST,
    ALTERA_PCI_REG_TEST,
    
    F35_MSI_INTR_TEST,
    DRMW_MSI_INT_TEST,
    DRMW_GDF_IPC_ERR_INT_TEST,
    
    F35_USB_TEST,
    F35_SPI_FLASH_TEST,

    F35_MODEM_ENUM_TEST,
    AT_COMMAND_TEST,
    F35_PHY_ADDR_INFO,
    EXIT_F35_DIAG,
    START_F35_DIAG,
    ANTENNAE_TEST,
    SIM_CARD_TEST,

    /* 
       Following commands are utilites for debugging 
       They cannot be invoked using IPC commands (intr/poll)
       Use either memory mapping from host or command line
       Host : memory mapping
       KW   : command line
    */
    F35_INT_REG_ALT,
    F35_INT_REG_DISP,
    F35_MEM_ALT,
    F35_MEM_DISP,

    /* 
       Following commands are invoked using IPC command (poll)
       or command line
       Host : IPC command (intr/poll)
       KW   : -"- , command line
    */
    DRMW_FPGA_REG_ALT,
    DRMW_FPGA_REG_DISP,
    ALTERA_PCI_REG_ALT,
    ALTERA_PCI_REG_DISP,
    F35_CPU_REG_DISP,
    USER_GDF_TEST,
    GDF_TEST,
    ALTERA_PCI_LINK_TEST,
    MODEM_ENUM_TEST,
    MODEM_AT_TEST,
    END_F35_COMMAND_LINE,

    /* 
       These tests cannot be invoked on command line 
       Host : IPC command (intr/poll)
    */
    DRMW_GDF_EGR_TEST = END_F35_COMMAND_LINE,
    DRMW_GDF_IGR_TEST,
    
    END_HOST_CMDS,
    
    /* read() operation commands between userspace and kernel */
    READ_DRMW_FPGA_STATE,
    READ_DRMW_FPGA_REGS,
    READ_DRMW_DATA_STRUCT,
    READ_ALTERA_PCI_REGS,
    RD_WR_ALTERA_PCI_REG,

    READ_IPC_SENT_INTR_STAT,
    
    GDF_EGRESS_INTR_CHECK,
    CLR_DIS_GDF_INTR_STAT,
    CLR_EN_GDF_INTR_STAT,
    CLR_EN_IPC_INTR_STAT,
    CLR_GDFIPC_INTR_DATA_STRUCT,
    
    GET_IPC_COMMAD_HOST,
    
    READ_DRMW_FPGA_REGS_RESP,
    READ_DRMW_DATA_STRUCT_RESP,

    ARM_CPU_REG_CON_DISP, /*(user only command)*/
    ALT_PCI_REG_CON_ALT,
    ALT_PCI_REG_CON_DISP,
    DRMW_FPGA_REG_CON_ALT,
    DRMW_FPGA_REG_CON_DISP,
    KW_MAP_CON_DISP,
    KW_INT_REG_CON_ALT,
    KW_INT_REG_CON_DISP,

    READ_ALT_PCI_REG,  /*kernel-user space command*/
    WAIT_GDF_RECV_INTR,
    WAIT_GDF_SENT_INTR,
    READ_KW_PHY_ADDR,
    READ_SPI_PROM,
    WRITE_SPI_PROM,

    USER_GDF_CONGSTN_TEST,
} kw_diag_tests_t;


/* 
 * Keep the enum in snc with the regs_tbl[]
 */
typedef enum ehwic_reg_id_ {
    KW_INT_REG_ID=0,
    FPGA_UNIT_ID,
    FPGA_PCIEUNIT_ID,
    ARM_CPU_UNIT_ID,
    END_GEN_UNIT_ID,
    USB_UNIT_ID=END_GEN_UNIT_ID,
    PCIE_UNIT_ID,
    MPP_UNIT_ID,
    MBUS_UNIT_ID,
    SPI_UNIT_ID,
    UART_UNIT_ID,
    DDR_UNIT_ID,
    RTC_UNIT_ID,
    BROM_UNIT_ID,
    GPIO_UNIT_ID,
    MISC_UNIT_ID,
    END_KW_INT_UNIT_ID,
} ehwic_reg_id;

#define MAX_NUM_MODEM        2

typedef struct f35_ds_t_ {
    uint diag_kernel_flag;           /* Check its usage */
    uint diag_uboot_flag;            /* Check its usage */
    uint image_flag;                 /* State of the KW CPU */
    uint flash_id;                   /* ID of flash used on Tesla */
    uint mmc_id;                     /* ID of flash used on Tesla */
    uint cpld_rev;                      /* ID of flash used on Tesla */
    uint kernel_version;                /* ID of flash used on Tesla */
    uint uboot_version;                 /* ID of flash used on Tesla */
    uint modem_id[MAX_NUM_MODEM];    /* ID of modem used on Tesla */
    uint num_modem;
    int  hwic_type[4];
    int uboot_info;
    int modem_info[MAX_NUM_MODEM];
    uchar modem_ati[MAX_NUM_MODEM][FOUR_K];
    uchar uboot_banner[FOUR_K];
    ether_hdr_t  eth_hdr;
} f35_ds_t;

typedef struct enum_test_ {
    uint32_t pid;
    uint32_t vid;
} enum_test_t;

typedef struct flash_test_t_ {
    uchar   err_msg[200];
    uint   size;                   /* total bank size in bytes             */
    uint   flash_id;
    uint  vendor;
    uint  device_id;
    uint   manufacturer_id;
} flash_test_t;

typedef struct reg_alt_test_ {
    uint target_cap_id;   /* For KW internal register and PCI CAP id */
    uint offset;      /* Register offset */
    uint op;          /* READ_REG/WRITE_REG */
    uint data;        /* data to write or read */
} reg_alt_test_t;

/* may need to combine ipc_msg_t and dspif_hdr_T */
#define IPC_MSG_DATA_LEN               512 - (5*4)

typedef struct  __attribute__((__packed__)) ipc_msg_ {
    volatile uint  own;          /* owner -- host , kw, kw_proc */
    volatile uint  msg_id;       /* same as op_type Type of IPC Message - 
                           NO_MSG, KW_MSG, IPC_CMD, KW_ACK, KW_CMD_RESP,
                           KW_READY_FOR_CMD */
    volatile uint  cmd_id;       /* which command */
    volatile uint  msg_len;
    volatile uint  cmd_rslt;
    volatile uint  msg_data[IPC_MSG_DATA_LEN];   /* IPC Message content.   */
} ipc_msg_t;

typedef struct {
    uint32_t src_id;       /* host or dspid_t   */
    uint32_t dest_id;      /* host or dspid_t  */
    uint32_t op_type;      /* type of operation, request or response     */
    uint32_t data_len;     /* ethernet data length; can be 0 */
} cmd_hdr_t;

typedef struct {
    uint32_t   command;
    uint32_t   ack;                /* ACK   */
    uint32_t   result;     /*  */
    uint32_t   flags;     /* */
    uint32_t   select;    
    uint32_t   faults;     /* */
    uint32_t   location;     /* */
    uint32_t   expected;     /* */
    uint32_t   actual;     /* */
    uint32_t   extra;     /* */
    uint32_t   errorcount;     /* */
    uint32_t   testcounter;     /* */
    uint32_t   ReadyOnTest;     /* */
    uint32_t   TestCtrl;     /* */
    uint32_t   WhoAmI;     /* could be dspid_t*/
    uint32_t   ver_no;     /* */
    uint32_t   wait_states;     /* */
    uint32_t   param1;
    uint32_t   param2;
    uint32_t   param3;
    uint32_t   param4;
    uint8_t    bufmsg[128]; /* data buffer for host/dsp intf */
    uint8_t    errmsg[128]; /* data buffer for host/dsp intf */
} cmd_info_t;

typedef struct {
    cmd_hdr_t cmd_hdr;
    cmd_info_t cmd_info;
} f35_cmd_pkt_t;

typedef struct f35_priv_t_ {
    f35_cmd_pkt_t *cmd_packet;
} f35_priv_t;
    
typedef struct ngio_priv_t_ {
    struct ngio_intf_t *ngiop;
    char pid[80];
    int fw_dnlded;
    int major_rel;
    int minor_rel;
    int debug_ver;

    int kernel_flag;
    int uboot_flag;
    int boot_dev_id;

    void *dev_priv;  /* ngio module specific ds */

    int ge_setup_flag;
    int socket_gl;
    ether_hdr_t eth_hdr;

    /* 1600 matched with f35 src code */
    fe_packet_t recv_packet; /* recv_rx_pkt() copied pkt received in here */
    uchar tx_pak[1600];      /* send_tx_pkt() sends out his packet */  
    uchar rx_pak[1600];      /* recv_rx_pkt() copied pkt received in here */

    int (*build_tx_pkt)(void *);
    int (*send_tx_pkt)(void *);
} ngio_priv_t;

typedef struct {
        uint8_t device_type;
        uint8_t device_freq;
        uint8_t chip_id;
        uint8_t core_id;
} dev_type_t;

typedef struct {
        uint8_t major_num;
        uint8_t minor_num;
        uint8_t debug_num;
} ver_type_t;

typedef struct {
        cmd_hdr_t dev_hdr;
        dev_type_t device;
        ver_type_t fw_ver;
} ready_t;
    
#define HOST_ID 0xf35  /* common */
#define ARMADA_38X 0x1

/* From ngvm_graffham.h */
#define RESULT_RUNNING                 0x0000
#define RESULT_FAILED                  0x0023
#define RESULT_SUCCESSFUL              0x003C
#define RESULT_ABORTED                 0x00C9
#define RESULT_TIMEOUT                 0x00FD

#define FLAG_NULL                      0x0000
#define FLAG_CONT_RUN                  0x0001
#define FLAG_STOP_ON_ERROR             0x0002
#define FLAG_LOOP_ON_ERROR             0x0004
#define FLAG_NO_FREQ_TESTS             0x8000

#define SELECT_READY            0x0300  /* Check if DSP is ready */

#define DIAGFW_MAJ_REL    0x0  /* official rel; major function change */
#define DIAGFW_MIN_REL    0x0  /* pre-official, minor fix */
#define DIAGFW_DEBUG_VER  0x2  /* debug version used for the same release */

#define MM_1    (MF_CONTINUOUS | MF_SHOW_ERRCOUNT)
#define MM_2    (MM_1 | MF_DOALL)

extern int tftp_get(char *dir, char *file, char *server_ip, char *dest,
                    unsigned int check);
#ifdef TACHI
extern int tachi_get_ge_sw_port_num(int slot, int tgt_device, int local_port);
#else
extern int ovld_get_ge_sw_port_num(int slot, int tgt_device, int local_port);
#endif
extern int set_gesw_line_loopback(int port_num, int onoff);

static int ltc4215_register_test(void);
static int ltc4215_reg_write(void);
static int ltc4215_reg_read(void);
static int f35_power_stat(void);
static int f35_power_off (void);
static int f35_pwr_off(void);
static int f35_pwr_on(void);
static int f35_pwr_cycle(void);
static int f35_utils(void);
static int gpio_exp_test(void);
static int f35_ready_test(int slot);
static int f35_rslt_test(int slot);
static int intf_ready(struct ngio_intf_t *iface);
static int f35_test(int);
static int f35_console_switch(void);
static int enable_bp_ge_lpbk(void);
static int disable_bp_ge_lpbk(void);
static int pca9557_reg_write(void);
static int pca9557_reg_read(void);
static int f35_o2_shell(void);
static int f35_o2_command(void);
static int f35_uart_test(void);

#if 0
static int f35_led_test (void);
static int f35_mem_short_test (void);
static int f35_cpld_test (void);
static int f35_soc_test (void);
static int f35_emmcflash_test (void);
static int f35_bootflash_test (void);
static int f35_modem_test (void);
#endif

#define CR_STRING           "\015\012"
#define CR_C_STRING         "\003"
#define UBOOT_PROMPT        ">"
#define BOOTD_COMMAND       "bootd\012"
#define HOST_IP             "192.123.123"
#define LOAD_STR            "#####"
#define BOOTD_CMD_COUNT     10
#define BOOT_TIMEOUT        20
#define UDP_UP_DOWN_TIMEOUT 100
#define DHCP_TIMEOUT        300
#define UART_TIMEOUT        100
#define BOOTD_COMMAND_TIMEOUT  50
#define FTP_SERVER          "192.123.123.1:69"
#define BOOT_DELAY          500
#define DELAY_1_SEC         1000
#define DELAY_HELF_SEC      0.5
#define AUTOBOOT_STOP_TIME  6000
#define GB_READY_TIME_OUT   150
#define READY_TIME_OUT      60
#define RETRY_TIME          5

extern uint32_t cpu_ge_status(int);
extern int dash_tx_uart(char *, char *);
extern int dash_rx_polling_uart(char *, char *, int);
extern int dash_uart_setup(char *);
extern int netstat_main(char *);
extern int utah_port_is_linkup(int);

/******** History ********
$Log: nim_f35.h,v $
Revision 1.5  2018/05/18 09:24:48  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.4  2017/08/10 10:10:30  iachang
CSCvf44161: Merge Goldbeach into USD platform as one image

Revision 1.3  2016/10/16 12:28:09  iachang
Supported Goldbeach Platform.

Revision 1.2.2.2  2018/05/17 10:50:19  alpeng
 sync with trunk <trunk-051618>

Revision 1.2.2.1  2017/04/05 09:10:30  leschen
Sync with <ng_diag-tag-032917>

Revision 1.4  2017/08/10 10:10:30  iachang
CSCvf44161: Merge Goldbeach into USD platform as one image

Revision 1.3  2016/10/16 12:28:09  iachang
Supported Goldbeach Platform.

Revision 1.2  2016/04/20 07:03:33  benchen2
merge tachi_branch to maintrunk

Revision 1.1.16.2  2016/01/19 03:44:53  alpeng
update f35 code for packet parsing

Revision 1.1.16.1  2015/08/11 07:52:00  meho
Added get sw port number for Tachi.

Revision 1.1  2014/11/01 05:04:32  srane
Initial commit for F35 4G NIM



$Endlog$
*/

