/* $Id: ngvm_graffham.h,v 1.12 2013/11/26 08:40:33 hroni Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/ngvm_graffham.h,v $
 *******************************************************************************
 * ngvm_graffham.h : Graffham defines
 *
 * Apr 2012, Smita Rane
 *
 * Copyright (c) 2012-2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *******************************************************************************
 */

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
#define INVALID_NGVM_NUM               0xFF
#define MAX_DSPS_PER_NGVM              1

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
 * required for NGVM to operate; This will proivde a easier way 
 * for porting between different platforms.
 */
typedef struct ngvm_entity_t_ {
    int cookie_id;           /* Controller/Cookie ID */
    char name[128];
    int ngvm_id;             /* SP27XX id */
    int num_dsp;             /* Number of DSP on NGVM */
    int ngvm_num;            /* 0 - onboard, 1 - NGWIC1 DC, ... */
    int pslot;               /* NGWIC parent slot # */
    int plat_ngvm_num;       /* Same as ngvm_num */
    int ge_tgt_dev;          /* TGT_DEV_NGVM or TGT_DEV_NGWIC */
    char slot_type_str[40]; /* Used for testnem */
    char tty_dev[20];       /* ttyDASH# */
    char pid[128];          /* PID */
    int dsp_downloaded[MAX_DSPS_PER_NGVM];
    int major_rel[MAX_DSPS_PER_NGVM];
    int minor_rel[MAX_DSPS_PER_NGVM];
    int debug_ver[MAX_DSPS_PER_NGVM];
    uchar tx_pak[ETHER_PACKET_LEN_MAX];     /* Ethernet tx packet buffer */
    uchar rx_pak[ETHER_PACKET_LEN_MAX];     /* Ethernet rx packet buffer */
    ether_hdr_t  eth_hdr[MAX_DSPS_PER_NGVM];
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
    int (*reset_ngvm)(void);              /* function - reset ngvm */
    int (*reset_dsp) (int dsp_num);  /* function - reset dsp */
    int (*init_ngvm) (void);               /* function - reset ngvm */
    int (*get_pid)   (char *pid);

} ngvm_entity_t;

#define PFUSE123_SP2702 0xC
#define PFUSE123_SP2704 0x0
#define PFUSE123_UNKNOWN 0xFF
    
#define MAX_NUM_NGVM        MAX_VM + MAX_WIC   /* currently not supported on SM */
#define NUM_DSP_GRAFFHAM    1

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

/* Test commands sent from host to Graffham */
#define SELECT_NULL             0x0000
#define SELECT_DSP_SANITY       0x0001  /* DSP Integrity test */
#define SELECT_DSP_SDRAM	0x0002  /* SDRAM test */
#define SELECT_DSS0_SANITY      0x0003  /* DSP Integrity test */
#define SELECT_DSS1_SANITY      0x0004  /* DSP Integrity test */
#define SELECT_DSS2_SANITY      0x0005  /* DSP Integrity test */
#define SELECT_DSS3_SANITY      0x0006  /* DSP Integrity test */
#define SELECT_UART_TEST        0x0007  /* Test uart interface */
#define SELECT_DAC_1DOT5VM_HIGH 0x0008
#define SELECT_DAC_1DOT5VM_LOW  0x0009
#define SELECT_DAC_NO_1DOT5VM   0x000A
#define SELECT_ECC_MEM          0x000B
#define SELECT_DSP_CONSOLE      0x000C
#define SELECT_UART_LPBK        0x000D
#define SELECT_UART_LPBK_RESULT 0x0011
#define SELECT_UART_LPBK_STOP   0x0012
#define SELECT_UART_LPBK_RX     0x0013
#define SELECT_DAC_DOT93VM_HIGH 0x0014
#define SELECT_DAC_DOT93VM_LOW  0x0015
#define SELECT_DAC_NO_DOT93VM   0x0016
#define SELECT_ARM11CPU1_BOOT   0x0017
#define SELECT_GE0_LPBK         0x0020  /* Host send lpbk packet to DSP over Ge0 Link */
#define SELECT_GE1_LPBK         0x0021  /* Host send lpbk packet to DSP over GE1 link */
#define SELECT_MEM_DISP         0x0022  /* Host send packet to DSP */
#define SELECT_FW_VER_DISP      0x0023  /* Host send packet to DSP test */
#define SELECT_GE_LPBK_PF	0x0040  /* with pause frame */
#define SELECT_GE0_LPBK_PT	0x0080  /* with pass through */
#define SELECT_GE1_LPBK_PT	0x0081  /* with pass through */
#define SELECT_INTF_SYNC        0x0082  /* NGVM SYNC interface signals test */
#define SELECT_TDM_INTLPBK	0x0100  /* TDM internal loopback test */
#define SELECT_TDM_EXTLPBK	0x0200  /* TDM external loopback test */
#define SELECT_READY	        0x0300  /* Check if DSP is ready */

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

/* TEMP HOST ID */
#define HOST_ID		0xfacefeed

#define MM_1    (MF_CONTINUOUS | MF_SHOW_ERRCOUNT)
#define MM_2    (MM_1 | MF_DOALL)

/******* ********
$Log: ngvm_graffham.h,v $
Revision 1.12  2013/11/26 08:40:33  hroni
fix compiler warning

Revision 1.11  2013/03/13 08:43:30  srane
Add test name structure for debug printfs.

Revision 1.10  2013/02/28 00:35:57  srane
Add support for NGVM testcard.

Revision 1.9  2013/01/29 02:06:22  srane
Add define for unknown LSI chipset.

Revision 1.8  2012/12/24 09:59:06  srane
Add NGVM interface SYNC signals test, show firmware ver command defines.

Revision 1.7  2012/10/04 19:15:34  srane
Add support for SP2702 (CSCuc51339).

Revision 1.6  2012/09/10 05:57:11  srane
Add ARM11 CPU1 test case, dsp memory display pkt structure.

Revision 1.5  2012/08/28 18:47:49  srane
Add defines for voltage margin tests.

Revision 1.4  2012/08/15 16:12:50  srane
Add define for ETH1 Loopback.

Revision 1.3  2012/07/17 20:13:52  srane
Add DAC voltage margin and DSP firmware version check.

Revision 1.2  2012/05/24 23:27:29  srane
Add support for both ethernet and uart moder, GPIO expander, uart tests.
General cleanup

Revision 1.1  2012/05/16 07:27:06  srane
Initial commit for Graffham NGVM.



$Endlog$
*/

