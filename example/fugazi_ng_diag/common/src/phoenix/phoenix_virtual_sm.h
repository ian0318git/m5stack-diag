/* $Id: phoenix_virtual_sm.h,v 1.2 2021/04/15 00:52:27 achiu2 Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/phoenix/phoenix_virtual_sm.h,v $
 *******************************************************************************
 * phoenix_virtual_sm.h : Phoenix defines
 *
 * Dec 2019 - Honda Wang
 *
 * Copyright (c) 2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *******************************************************************************
 */

 
/*******************************************************************************
 * Extern function prototypes
 *******************************************************************************
 */
extern int do_all_menu_items(struct menuinfo *);
extern int ngio_sync_out_enable (void *p, int mask);
extern unsigned int fru_table_offset;
extern fru_table_t platform_fru_table[];
extern void local_mac_addrs_init(void);

typedef enum {
    MB = 0,
    IO_MB = 0,
    MEM_DIMM0,
    PVDM
} fru_offset_t;

typedef struct {
	uint8_t device_type;
	uint8_t device_freq;
	uint8_t chip_id;
	uint8_t core_id;
} dsp_type_t;

#define DIAGFW_MAJ_REL    0x0  /* official rel; major function change */
#define DIAGFW_MIN_REL    0x0  /* pre-official, minor fix */
#define DIAGFW_DEBUG_VER  0x1  /* debug version used for the same release */

/* SP27xx DSS core ID */
#define DSS_CORE0	0x0
#define DSS_CORE1	0x1
#define DSS_CORE2	0x2
#define DSS_CORE3	0x3

/* if this is to be changed the the .lcf file must be changed */
/* interface header */
/* packet send out */
typedef uint8_t mac_address_t[6];

/* packet header */
typedef struct {
    mac_address_t  dest_addr;
    mac_address_t  src_addr;
    uint16_t      pkt_type;
    uint16_t      pkt_len;
} gepkt_hdr_t;

typedef struct {
    uint32_t src_id;       /* host or dspid_t   */
    uint32_t dest_id;      /* host or dspid_t  */
    uint32_t op_type;      /* type of operation, request or response     */
    uint32_t data_len;     /* ethernet data length; can be 0 */
} dspif_hdr_t;              

typedef struct {
        uint8_t major_num;
        uint8_t minor_num;
        uint8_t debug_num;
} ver_type_t;

#define ETHER_PACKET_LEN_MAX        1514
#define INVALID_NGSM_NUM               0xFF
#define MAX_DSPS_PER_NGSM              1
#define PHOENIX_UART                  0 
#define TIME_GET_STRING             50
#define TIME_GET_CAL_VAL            10

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
} dspif_info_t;

typedef struct {
    dspif_hdr_t dspif_hdr;
    dspif_info_t  dspif_info;
} dspif_ether_t;

typedef struct {
        dspif_hdr_t dspif_hdr;
        dsp_type_t dsp_device;
        ver_type_t fw_ver;
} dspif_ready_t;
    
typedef struct {
	dspif_hdr_t dspif_hdr;
	uint8_t	data[2048];
} dspif_lpbk_t;

typedef struct dspif_mem_t_ {
    dspif_hdr_t dspif_hdr;
    dspif_info_t  dspif_info;
    uint8_t pkt_data[1000-sizeof(dspif_ether_t)];
} dspif_mem_t;

typedef struct ge_packet_t {
    gepkt_hdr_t pkt_hdr;
	uint8_t pkt_data[1500];
} ge_packet_t;

typedef struct {
	uint8_t slot_id;
	uint8_t module_id;
	uint8_t dsp_id;
	uint8_t core_id;
} dspid_t;

/* define this structure to hold the necessary data and functions
 * required for NGSM to operate; This will proivde a easier way 
 * for porting between different platforms.
 */
typedef struct ngsm_entity_t_ {
    int cookie_id;           /* Controller/Cookie ID */
    char name[128];
    int ngsm_id;             /* SP27XX id */
    int num_dsp;             /* Number of DSP on NGSM */
    int ngsm_num;            /* slot 1 or slot 2 */
    int pslot;               /* NGWIC parent slot # */
    int plat_ngsm_num;       /* Same as ngsm_num */
    int ge_tgt_dev;          /* TGT_DEV_NGSM */
    char slot_type_str[40]; /* Used for testnem */
    char tty_dev[20];       /* ttyDASH# */
    char pid[128];          /* PID */
    int dsp_downloaded[MAX_DSPS_PER_NGSM];
    int major_rel[MAX_DSPS_PER_NGSM];
    int minor_rel[MAX_DSPS_PER_NGSM];
    int debug_ver[MAX_DSPS_PER_NGSM];
    uchar tx_pak[ETHER_PACKET_LEN_MAX];     /* Ethernet tx packet buffer */
    uchar rx_pak[ETHER_PACKET_LEN_MAX];     /* Ethernet rx packet buffer */
    ether_hdr_t  eth_hdr[MAX_DSPS_PER_NGSM];
    int ge_in_port;
    int ge_out_port;
    int socket_gl;
    uchar *malloc_p[2];
    void *pca;
    fe_packet_t recv_packet;
    dspif_ether_t cmd_packet;
    dspif_ether_t result_packet;
    boolean ge_setup_flag;
    
    /* Can add NGVM specific struct here */
    int (*reset_ngsm)(void);              /* function - reset ngsm */
    int (*reset_dsp) (int dsp_num);  /* function - reset dsp */
    int (*init_ngsm) (void);               /* function - reset ngsm */
    int (*get_pid)   (char *pid);

} ngsm_entity_t;

#define PFUSE123_SP2702 0xC
#define PFUSE123_SP2704 0x0
#define PFUSE123_UNKNOWN 0xFF
    
#define MAX_NUM_NGSM        MAX_SM   /* currently not supported on SM */
#define NUM_DSP_PHOENIX    1

/*************************** WARNING *************************/
/*************************************************************/
/*    All constants defined below must match the CE code     */
/*************************************************************/
/*************************** WARNING *************************/

/* Host Interface Structure - operation type defines */
#define OP_NULL            0
#define OP_TEST_REQUEST    1   /* host->DSP : run test */
#define OP_INFO_REQUEST    2   /* host->DSP : get info/data */
#define OP_DSP_REQUEST     3   /* DSP->host: not likely to be used */
#define OP_RESPONSE        4   /* mutual: send back test result, info  */
#define OP_READY           5   /* DSP->host : DSP is up and ready */
#define OP_TEST_STOP       6   /* host->DSP: to stop current test */

/* Host Interface Structure - command defines shared between host and CE */
#define CMD_ABORT                      0x0000
#define CMD_RESET                      0x0009
#define CMD_RUN                        0x0096
#define CMD_STOP                       0x0099

#define ACK_NULL                       0x0000
#define ACK_ERROR                      0x007E
#define ACK_OK                         0x0089

#define FLAG_NULL                      0x0000
#define FLAG_CONT_RUN                  0x0001
#define FLAG_STOP_ON_ERROR             0x0002
#define FLAG_LOOP_ON_ERROR             0x0004
#define FLAG_NO_FREQ_TESTS             0x8000

#define LOCATION_DSP_INSTRUCTION       0x0001
#define LOCATION_DSP_BRANCH            0x0002
#define LOCATION_DSP_DATA              0x0004
#define LOCATION_DSP_ADDR              0x0008
#define LOCATION_DSP_PATTERN           0x0010
#define LOCATION_DSP_INTERRUPT         0x0020

#define RESULT_RUNNING                 0x0000
#define RESULT_FAILED                  0x0023
#define RESULT_SUCCESSFUL              0x003C
#define RESULT_ABORTED                 0x00C9
#define RESULT_TIMEOUT                 0x00FD

/* FXS/FXO Port Led for PHOENIX */
#define FXS_FXO_LED_REG                0x90E0

#define FXO_P0_LEDGRN_EN               0xFFFFFFEF
#define FXO_P0_LEDRED_EN               0xFFFFFFDF
#define FXO_P1_LEDGRN_EN               0xFFFFFFBF
#define FXO_P1_LEDRED_EN               0xFFFFFF7F
#define FXO_P2_LEDGRN_EN               0xFFFFFEFF
#define FXO_P2_LEDRED_EN               0xFFFFFDFF
#define FXO_P3_LEDGRN_EN               0xFFFFFBFF
#define FXO_P3_LEDRED_EN               0xFFFFF7FF
#define FXO_P4_LEDGRN_EN               0xFFFFEFFF
#define FXO_P4_LEDRED_EN               0xFFFFDFFF
#define FXO_P5_LEDGRN_EN               0xFFFFBFFF
#define FXO_P5_LEDRED_EN               0xFFFF7FFF

#define FXS_P0_LEDGRN_EN               0xFFFEFFFF             
#define FXS_P0_LEDRED_EN               0xFFFDFFFF
#define FXS_P1_LEDGRN_EN               0xFFFBFFFF 
#define FXS_P1_LEDRED_EN               0xFFF7FFFF
#define FXS_P2_LEDGRN_EN               0xFFEFFFFF
#define FXS_P2_LEDRED_EN               0xFFDFFFFF
#define FXS_P3_LEDGRN_EN               0xFFBFFFFF
#define FXS_P3_LEDRED_EN               0xFF7FFFFF
#define FXS_P4_LEDGRN_EN               0xFEFFFFFF
#define FXS_P4_LEDRED_EN               0xFDFFFFFF
#define FXS_P5_LEDGRN_EN               0xFBFFFFFF
#define FXS_P5_LEDRED_EN               0xF7FFFFFF
#define FXS_P6_LEDGRN_EN               0xEFFFFFFF
#define FXS_P6_LEDRED_EN               0xDFFFFFFF
#define FXS_P7_LEDGRN_EN               0xBFFFFFFF
#define FXS_P7_LEDRED_EN               0x7FFFFFFF

#define ALL_FXSP_LEDGRN                0xAAAAFFFF 
#define ALL_FXSP_LEDRED                0x5555FFFF
#define ALL_FXOP_LEDGRN                0xFFFFAAAF 
#define ALL_FXOP_LEDRED                0xFFFF555F

#define ALL_LEDGRN_EN                  0xAAAAAAAF      
#define ALL_REDLED_EN                  0x5555555F
#define ALL_LED_EN                     0x0000000F
#define ALL_LED_DIS                    0xFFFFFFFF

/* Test commands sent from host to Oakenshield */
#define SELECT_NULL                 0x0000
#define SELECT_DSP_SANITY           0x0001  /* DSP Integrity test */
#define SELECT_DSP_SDRAM            0x0002  /* SDRAM test */
#define SELECT_DSS0_SANITY          0x0003  /* DSP Integrity test */
#define SELECT_DSS1_SANITY          0x0004  /* DSP Integrity test */
#define SELECT_DSS2_SANITY          0x0005  /* DSP Integrity test */
#define SELECT_DSS3_SANITY          0x0006  /* DSP Integrity test */
#define SELECT_UART_TEST            0x0007  /* Test uart interface */
#define SELECT_DAC_1DOT5SM_HIGH     0x0008
#define SELECT_DAC_1DOT5SM_LOW      0x0009
#define SELECT_DAC_NO_1DOT5SM       0x000A
#define SELECT_ECC_MEM              0x000B
#define SELECT_DSP_CONSOLE          0x000C
#define SELECT_UART_LPBK            0x000D
#define SELECT_UART_LPBK_RESULT     0x0011
#define SELECT_UART_LPBK_STOP       0x0012
#define SELECT_UART_LPBK_RX         0x0013
#define SELECT_DAC_3DOT3SM_HIGH     0x0014
#define SELECT_DAC_3DOT3SM_LOW      0x0015
#define SELECT_DAC_NO_3DOT3SM       0x0016
#define SELECT_ARM11CPU1_BOOT       0x0017
#define SELECT_DAC_1DOT5_SHOW       0x0018
#define SELECT_DAC_3DOT3_SHOW       0x0019
#define SELECT_READ_FPGA_DIR_REG    0x001A
#define SELECT_WRITE_FPGA_DIR_REG   0x001B
#define SELECT_READ_FPGA_INDIR_REG  0x001C
#define SELECT_WRITE_FPGA_INDIR_REG 0x001D
#define SELECT_GE0_LPBK             0x0020  /* Host send lpbk packet to DSP over Ge0 Link */
#define SELECT_GE1_LPBK             0x0021  /* Host send lpbk packet to DSP over GE1 link */
#define SELECT_MEM_DISP             0x0022  /* Host send packet to DSP */
#define SELECT_FW_VER_DISP          0x0023  /* Host send packet to DSP test */
#define SELECT_GE_LPBK_PF	        0x0040  /* with pause frame */
#define SELECT_GE0_LPBK_PT    	    0x0080  /* with pass through */
#define SELECT_GE1_LPBK_PT	        0x0081  /* with pass through */
#define SELECT_INTF_SYNC            0x0082  /* NGVM SYNC interface signals test */
#define SELECT_FAIL_OVERPORT        0x0083  /* Failed over port */
#define SELECT_TDM_CODEC_RST        0x0084  /* TDM codec reset */
#define SET_TOGGLE_SEP_MB_DBX_TEST_FLAG 0x0085  /* Toggle separate mb or bx test flag */
#define SELECT_HW_BRD_TYPE_FLAG     0x0086  /* Send hardward board type flag to DSP */

#define SELECT_TDM_INTLPBK	0x0100  /* TDM internal loopback test */
#define SELECT_TDM_EXTLPBK	0x0200  /* TDM external loopback test */
#define SELECT_READY	        0x0300  /* Check if DSP is ready */

#define SELECT_FPGA_REG_TEST                 0x0400  /* FPGA Register test */
#define SELECT_FPGA_MEM_TEST                 0x0401  /* FPGA Memory test */
#define SELECT_FPGA_INT_TEST                 0x0402  /* FPGA Interrupt test */
#define SELECT_FPGA_TDMSW_FORCE_BYTE_TEST    0x0403  /* FPGA TDMSW16 force byte test */

#define SELECT_VIC				0x8000  /* Base for All the tests for VICs */



/* FXS CODEC (SI32261) related tests */
#define SELECT_CODEC_SI32261_DIGITAL_LOOPBACK   SELECT_VIC + 0x0001
#define SELECT_CODEC_SI32261_SET_RING           SELECT_VIC + 0x0002
#define SELECT_CODEC_SI32261_STOP_RING          SELECT_VIC + 0x0003
#define SELECT_CODEC_SI32261_REG_READ           SELECT_VIC + 0x0004
#define SELECT_CODEC_SI32261_REG_WRITE          SELECT_VIC + 0x0005
#define SELECT_CODEC_SI32261_CALIBRATION        SELECT_VIC + 0x0006
#define SELECT_CODEC_SI32261_RAM_READ           SELECT_VIC + 0x0007
#define SELECT_CODEC_SI32261_RAM_WRITE          SELECT_VIC + 0x0008
#define SELECT_CODEC_SI32261_PROTECTED          SELECT_VIC + 0x0009
#define SELECT_CODEC_SI32261_CALIBRATE_RESULT   SELECT_VIC + 0x000a
#define SELECT_CODEC_SI32261_CALIBRATE_SAVE     SELECT_VIC + 0x000b
#define SELECT_CODEC_SET_FAIL_OVER_PORT         SELECT_VIC + 0x000C
#define SELECT_FXS_FXO_LED                      SELECT_VIC + 0x000D
#define SELECT_CODEC_SI32261_RING               SELECT_VIC + 0x000E


/* FXO CODEC (SI3050) related tests */
#define SELECT_CODEC_SI3050_DIGITAL_LOOPBACK    SELECT_VIC + 0x0101
#define SELECT_CODEC_SI3050_INIT                SELECT_VIC + 0x0102
#define SELECT_CODEC_SI3050_REG_WR              SELECT_VIC + 0x0103
#define SELECT_CODEC_SI3050_REG_RD              SELECT_VIC + 0x0104


typedef struct test_commands {
    uint test_id;
    char test_name[50];
} test_commands_t;

/* NGVM Interface */
#define SYNC_OUT      0x8
#define SYNC_TRIG_OUT 0x20
#define SYNC_IN       0x800
#define SYNC_OUT1     0x2000
#define SYNC_TRIG_IN  0x8000

#define CHECK_LEVEL   0x100 
#define CHECK_LOW     0x100
#define CHECK_HIGH    0x101
#define SET_LEVEL   0x10 
#define SET_LOW     0x10
#define SET_HIGH    0x11

/* PCA9557 Definition */
#define PCA9557_IN_PORT_REG             0x00
#define PCA9557_OUT_PORT_REG            0x01
#define PCA9557_POLAR_INV_P_REG         0x02
#define PCA9557_CFG_PORT_REG            0x03

#define PCA9557_PORT_MASK               0xFF
#define PCA9557_PORT_INIT               0x00

#define PCA9557_IO_INPUT                0x1
#define PCA9557_IO_OUTPUT               0x0
#define PCA9557_IO_HIGH                 0x1
#define PCA9557_IO_LOW                  0x0

/* TEMP HOST ID */
#define HOST_ID		0xfacefeed

#define MM_1    (MF_CONTINUOUS | MF_SHOW_ERRCOUNT)
#define MM_2    (MM_1 | MF_DOALL)

static char src_mac[] = { 0x00, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };

#define PACKET_TO_DSP   0xDD
#define PACKET_FROM_DSP 0xCC
#define FRU_SIZE 80
#define FIRMWARE_PATH /firmware/dsp_sp2700_fw.img
#define PHOENIX_DSP_LB_GAR_PACKET_ALLOW   10

/* Oakenshield PID */
#define PID_8FXS_12FXO "SM-X-8FXS/12FXO"
#define PID_16FXS_2FXO "SM-X-16FXS/2FXO"
#define PID_24FXS_4FXO "SM-X-24FXS/4FXO"
#define PID_72FXS      "SM-X-72FXS"
#define PID_2FXS_2FXO  "VG400-2FXS/2FXO"
#define PID_4FXS_4FXO  "VG400-4FXS/4FXO"
#define PID_6FXS_6FXO  "VG400-6FXS/6FXO"
#define PID_8FXS       "VG400-8FXS"
#define PID_144FXS       "VG420-144FXS"
#define PID_132FXS_6FXO  "VG420-132FXS/6FXO"
#define PID_84FXS_6FXO   "VG420-84FXS/6FXO"

typedef struct pid_info {
    char pid[32];
    int fxs_port;
} pid_info_t;

#define UART_WAIT_TIME        3000
#define UART_READY_TIME       1000
#define BUF_WAIT_TIME         3000
#define ETH_PORT0             0
#define ETH_PORT1             1
#define SYS_getcpu            309
#define TFTP_IMG_NAME         "oakenshield_dsp_diag.img"
#define IMG_PATH              "/firmware/vg4x0_dsp_sp2700_fw.img"
#define PWR_WAIT_TIME         50
#define PHOENIX_OFF_TIME        1000
#define DSP_SI32261_CAL_TIME  7200
#define DSP_WAIT_TIME         1000
#define NGIO_WAIT_TIME        5000
#define DSP_READY_TIME        2000
#define PKG_WAIT_TIME         1000
#define DSP_SETUP_TIME        10
#define DSP_FPGA_SIZE         1000
#define DSP_BOOT_TIME         300
#define DSP_TIME_OUT          299

/* Phoenix Separate MB or DBx test flag */
#define PHOENIX_TEST_ALL_BOARD  0xF
#define PHOENIX_TEST_SPE_BOARD_MIN  0x0
#define PHOENIX_TEST_SPE_BOARD_MAX  0xF
#define PHOENIX_TEST_ONLY_MB    0x8
#define PHOENIX_TEST_ONLY_DB1   0x4
#define PHOENIX_TEST_ONLY_DB2   0x2
#define PHOENIX_TEST_ONLY_DB3   0x1

/* Phoenix CPLD Magic number */

#define PHOENIX_CPLD_IMAGE_SIZE   0x10000
#define PHOENIX_CPLD_RECV_BUFF    1600
#define PHOENIX_CPLD_TX_BUFF      1001
#define PHOENIX_CPLD_RECV_FOR_DSP    14


/****************
$Log: phoenix_virtual_sm.h,v $
Revision 1.2  2021/04/15 00:52:27  achiu2
[PRRQ:CSCvx56970-2]Phoenix code review for ER

Revision 1.1.2.5  2020/08/05 10:23:16  achiu2
porting back "Replace is_fxo function with db1_has_fxo." modification



$Endlog$
*/

