/* $Id: diag_ppb.h,v 1.4 2021/04/15 00:52:44 achiu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/ppb/inc/diag_ppb.h,v $
 *------------------------------------------------------------------
 * diag_ppb.h
 *      Oakenshield host-dsp interface
 *
 * Mar 2012, Smita Rane
 *
 * Copyright (c) 2012-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DIAG_PPB_H__
#define __DIAG_PPB_H__

/* Loopback */
#define INTERNAL        1
#define EXTERNAL        2
/* DSP Identification */
#define DSP_SP2603	0x0
#define DSP_SP2601	0x1
#define DSP_SP2602	0x2
#define DSP_500MHZ	0x0
#define DSP_350MHZ	0x1
#define DSP_250MHZ	0x2
#define DSP_550MHZ	0x0
#define DSP_400MHZ	0x1
#define DSP_300MHZ	0x2
#define DSP_750MHZ	0x3
#define DSP_CHIP0	0x0
#define DSP_CHIP1	0x1

#define WAIT_TIME       1000 /* one sec */

/* Taken from host code must match */
/*
 * defines for mother board type device or network module
 */
enum {
    MOTHER_BOARD =  0, /* used for mother board's cookie   */
    NETWK_MODULE =  1,  /* used for network module's cookie */
    WIC_MODULE   =  2,  /* used for WIC module's cookie     */
    VIC_MODULE   =  3,  /* used for VIC module's cookie     */
    SPMM_MODULE  =  4,  /* used for SPMM module's cookie    */
    DAUGHTER_CARD = 5,  /* used for daughter card cookie    */
    VM_MODULE   = 6,  /* used for PVDM module cookie on MB */
    AIM_MODULE    = 7,  /* used for AIM module cookie */
    BACK_PLANE    = 8,  /* for Mid Rise module cookie */
    ISM_MODULE    = 9,  /* for Integrated Service Module cookie */
    NM_ADAPTER    = 10,  /* for Integrated Service Module cookie */
    SM_MODULE     = 11,  /* for Service Module cookie */
    PSU_MODULE    = 12,  /* for Power Supply Uint Module cookie */
    SM_DAUGHTER_CARD  = 13, /* for SM's daughter card cookie */
    WIC_DAUGHTER_CARD,   /* for SM's daughter card cookie */
    HW_CRYPTO_ACC, /* Hardware based crypto accelerator */
    PSU_AC_MODULE, /* for Power Supply AC Module cookie */
    PSU_DC_MODULE,  /* for Power Supply DC Module cookie */
    RISER_CARD,  /* for Riser Card Module cookie */
    MAX_DEVICE_TYPE
};

/* Console input buffer size */
#define CBUF_SIZE 80
#define EXIT_CHAR '\033'

typedef struct {
	uint8_t device_type;
	uint8_t device_freq;
	uint8_t chip_id;
	uint8_t core_id;
} dsp_type_t;

typedef struct {
	uint8_t	major_num; /* major release number */
	uint8_t	minor_num; /* minor release number */
	uint8_t	debug_num; /* debug version */
} ver_type_t;

/* Voltage margining defines */
#define SDA                      6
#define SCL                      7
#define DAC_1DOT5                1
#define DAC_3DOT3                2
#define DAC_DEV_ADDR_FOR_WRITE   0x20
#define DAC_DEV_ADDR_FOR_READ    DAC_DEV_ADDR_FOR_WRITE
#define DAC_1DOT5_MASK           0x0C
#define DAC_3DOT3_MASK           0x30
#define DAC_1DOT5_VOLT_HIGH      0x08
#define DAC_1DOT5_VOLT_LOW       0x04
#define DAC_3DOT3_VOLT_HIGH      0x20
#define DAC_3DOT3_VOLT_LOW       0x10

/* DSP core ID */
#define DSS_CORE0	0x0
#define DSS_CORE1	0x1
#define DSS_CORE2	0x2
#define DSS_CORE3	0x3

/* PFUSE123 values */
#define PFUSE123_SP2702 0xC
#define PFUSE123_SP2704 0x0


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
	int8_t	data[1500];
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

typedef struct fpga_packet_t {
    uint8_t pkt_hdr;
    uint8_t pkt_data[1200];
} fpga_packet_t;

typedef struct {
	uint8_t slot_id;
	uint8_t module_id;
	uint8_t dsp_id;
	uint8_t core_id;
} dspid_t;

#define NUM_BOOT_MSG	10

typedef struct bootup_msg_t {
    char msg[NUM_BOOT_MSG*2048];
} bootup_msg_t;

typedef struct host_comm_status_t {
    uint32_t prev_command_recv;
    uint32_t prev_status;
    char prev_msg[2048];
    uint32_t cur_command_recv;
    uint32_t cur_status;
    char cur_msg[2048];
} host_comm_status_t;

#define MSG_READY	1
#define MSG_TEST	2
#define MSG_TIMEOUT	3
#define MSG_LPBK	4
#define MSG_MEM		5

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
#define RESULT_SUCCESSFUL              0x0000003C
#define RESULT_ABORTED                 0x00C9
#define RESULT_TIMEOUT                 0x00FD


#define SELECT_NULL         0x0000
#define SELECT_DSP_SANITY   0x0001  /* DSP Integrity test */
#define SELECT_DSP_SDRAM	0x0002  /* SDRAM test */
#define SELECT_DSS0_SANITY      0x0003  /* DSP Integrity test */
#define SELECT_DSS1_SANITY      0x0004  /* DSP Integrity test */
#define SELECT_DSS2_SANITY      0x0005  /* DSP Integrity test */
#define SELECT_DSS3_SANITY      0x0006  /* DSP Integrity test */
#define SELECT_UART_TEST        0x0007  /* Test uart interface */
#define SELECT_DAC_1DOT5SM_HIGH 0x0008
#define SELECT_DAC_1DOT5SM_LOW  0x0009
#define SELECT_DAC_NO_1DOT5SM   0x000A
#define SELECT_ECC_MEM          0x000B
#define SELECT_DSP_CONSOLE      0x000C
#define SELECT_UART_LPBK        0x000D
#define SELECT_GE1_INT_LPBK     0x000E
#define SELECT_WTINT		    0x0010  /* watchdog timer interrupt test */
#define SELECT_UART_LPBK_RESULT 0x0011
#define SELECT_UART_LPBK_STOP   0x0012
#define SELECT_UART_LPBK_RX     0x0013
#define SELECT_DAC_3DOT3SM_HIGH 0x0014
#define SELECT_DAC_3DOT3SM_LOW  0x0015
#define SELECT_DAC_NO_3DOT3SM   0x0016
#define SELECT_ARM11CPU1_BOOT   0x0017
#define SELECT_DAC_1DOT5_SHOW   0x0018
#define SELECT_DAC_3DOT3_SHOW   0x0019
#define SELECT_READ_FPGA_DIR_REG    0x001A
#define SELECT_WRITE_FPGA_DIR_REG   0x001B
#define SELECT_READ_FPGA_INDIR_REG  0x001C
#define SELECT_WRITE_FPGA_INDIR_REG 0x001D
#define SELECT_GE0_LPBK		0x0020  /* Host send packet to DSP test */
#define SELECT_GE1_LPBK		0x0021  /* Host send packet to DSP test */
#define SELECT_MEM_DISP		0x0022  /* Host send packet to DSP test */
#define SELECT_FW_VER_DISP	0x0023  /* Host send packet to DSP test */
#define SELECT_GET_INFO     0x0024  /* Host send packet to DSP test */
#define SELECT_GE_LPBK_PF	0x0040  /* with pause frame */
#define SELECT_GE0_LPBK_PT	0x0080  /* with pass through (Internal) */
#define SELECT_GE1_LPBK_PT	0x0081  /* with pass through */
#define SELECT_INTF_SYNC        0x0082  /* NGVM SYNC interface signals test */
#define SELECT_TDM_CODEC_RST 0x0084  /* TDM CODEC reset test*/
#define SET_TOGGLE_SEP_MB_DBX_TEST_FLAG 0x0085  /* Toggle separate mb or bx test flag */
#define SELECT_HW_BRD_TYPE_FLAG         0x0086  /*  Send hardward board type flag to DSP */

#define SELECT_TDM_INTLPBK	0x0100  /* TDM internal loopback test */
#define SELECT_TDM_EXTLPBK	0x0200  /* TDM external loopback test */
#define SELECT_READY	        0x0300  /* TDM external loopback test */

#define SELECT_FPGA_REG_TEST                 0x0400  /* FPGA Register test */
#define SELECT_FPGA_MEM_TEST                 0x0401  /* FPGA Memory test */
#define SELECT_FPGA_INT_TEST                 0x0402  /* FPGA Interrupt test */
#define SELECT_FPGA_TDMSW_FORCE_BYTE_TEST    0x0403  /* FPGA TDMSW16 force byte test */

/* NGVM Interface */
#define SYNC_OUT 0x8
#define SYNC_TRIG_OUT 0x20
#define SYNC_IN 0x800
#define SYNC_OUT1 0x2000
#define SYNC_TRIG_IN 0x8000

#define CHECK_LEVEL   0x100 
#define CHECK_LOW     0x100
#define CHECK_HIGH    0x101
#define SET_LEVEL   0x10 
#define SET_LOW     0x10
#define SET_HIGH    0x11
#define DSP_STOP_WHILE_LOOP 0x1111


#define SELECT_VIC			0x8000  /* Base for All the tests for VICs */


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
#define SELECT_CODEC_SI32261_CALIBRATE_RESULT   SELECT_VIC + 0x000A
#define SELECT_CODEC_SI32261_CALIBRATE_SAVE     SELECT_VIC + 0x000B
#define SELECT_CODEC_SET_FAIL_OVER_PORT         SELECT_VIC + 0x000C
#define SELECT_FXS_FXO_LED                      SELECT_VIC + 0x000D
#define SELECT_CODEC_SI32261_RING               SELECT_VIC + 0x000E


/* FXS CODEC (SI3050) related tests */
#define SELECT_CODEC_SI3050_DIGITAL_LOOPBACK    SELECT_VIC + 0x0101
#define SELECT_CODEC_SI3050_INIT                SELECT_VIC + 0x0102
#define SELECT_CODEC_SI3050_REG_WR              SELECT_VIC + 0x0103
#define SELECT_CODEC_SI3050_REG_RD              SELECT_VIC + 0x0104


typedef enum {
    OAKENSHIELD_SKU_16FXS_2FXO,       /* 2FXO on MB */
    OAKENSHIELD_SKU_24FXS_4FXO,       /* 2FXO on MB */
    OAKENSHIELD_SKU_8FXS_12FXO,       /* 2FXO on MB */
    OAKENSHIELD_SKU_72FXS,
    VG400_SKU_2FXS_2FXO,
    VG400_SKU_4FXS_4FXO,
    VG400_SKU_6FXS_6FXO,
    VG400_SKU_8FXS,
    PHOENIX_SKU_144FXS,
    PHOENIX_SKU_132FXS_6FXO,
    PHOENIX_SKU_84FXS_6FXO,
} oakenshield_sku_e;

/* TEMP HOST ID */
#define HOST_ID		0xfacefeed

/* MAGIC number to tell PPB that DSS cores are ready */
#define MAGIC	43	/* 0x2B */

#define SWAP16(X) \
        ( \
        (((X) & 0x00ff) << 8) + \
        (((X) & 0xff00) >> 8) \
        )

#define SWAP32(X) \
        ( \
        (((X) & 0x000000ff) << 24) + \
        (((X) & 0x0000ff00) << 8)  + \
        (((X) & 0x00ff0000) >> 8)  + \
        (((X) & 0xff000000) >> 24) \
        )

#define AG_MG_PPB_MAC0_DEVICE  0

extern void StarProPPB_UTILS_memset8(uint8_t *p_mem, uint8_t pattern, uint32_t n);
extern void StarProPPB_UTILS_memcpy8(uint8_t *pdest, const uint8_t *psrc, uint32_t size);
extern void eprom_disp(char *cmdargs, uint8_t *arr, int size);
extern void do_mem_md(char* cmdargs);
extern void do_mem_md_off(char* cmdargs);
extern int do_mem_mw(char* cmdargs);
extern int do_mem_mm(char* cmdargs);
extern int do_mem_nm(char* cmdargs);
extern void read_ppb_gpio_utility (void);

#endif /* __DIAG_PPB_H__ */

/*Debug flag*/
int oak_diag_flag;

/*Transmit FPGA image data header*/
#define PACKET_TO_HOST 0xCC 
#define PACKET_TO_DSP  0xDD

/*
 * $Log: diag_ppb.h,v $
 * Revision 1.4  2021/04/15 00:52:44  achiu2
 * [PRRQ:CSCvx56970-2]Phoenix code review for ER
 *
 * Revision 1.3  2018/08/30 06:40:21  haohsu
 * Collapse Vg400-branch to Main Trunk
 *
 * Revision 1.2.28.1  2018/02/06 09:34:06  haohsu
 * Code change for VG400
 *
 * Revision 1.2  2017/07/28 07:58:37  harrchan
 * Collapse Oakenshield-branch to Main Trunk.
 *
 * Revision 1.1.2.1  2017/06/29 08:14:32  harrchan
 * Initial commit code for Oakenshield
 *
 * Revision 1.11.80.6  2017/03/30 10:25:49  harrchan
 * Add fpga upgrade utility
 *
 * Revision 1.11.80.5  2017/03/09 07:48:11  harrchan
 * Support oakenshield double wide case
 *
 * Revision 1.11.80.3  2017/02/09 06:41:05  olin2
 * Support voltage margin and fail over port utility
 *
 * Revision 1.11.80.2  2017/01/17 05:07:05  olin2
 * Clean up debug code
 *
 * Revision 1.11.80.1  2016/12/14 05:03:49  olin2
 * Initial commit code for Oakenshield
 *
 * Revision 1.11  2012/12/24 00:07:46  srane
 * Support NGVM interface SYNC signals and firmware version defines.
 *
 * Revision 1.10  2012/10/04 23:36:15  srane
 * Add support for SP2702. Version control.
 *
 * Revision 1.9  2012/09/10 06:31:42  srane
 * Add defines for dsp memory display pkt and ARM11 CPU1 test.
 *
 * Revision 1.8  2012/08/28 18:18:43  srane
 * Add defines for .93V marginging.
 *
 * Revision 1.7  2012/08/15 15:02:37  srane
 * Add define for EMAC1 loopback tests.
 *
 * Revision 1.6  2012/07/17 20:56:33  srane
 * Add defines for DAC voltage marging tests.
 *
 * Revision 1.5  2012/06/28 21:24:38  srane
 * add variables needed for Host-DSP READY message exchange.
 *
 * Revision 1.4  2012/06/07 22:50:11  srane
 * cleanup
 *
 * Revision 1.3  2012/05/24 23:22:38  srane
 * Add UART test (for ethernet mode).
 *
 * Revision 1.2  2012/05/10 22:47:25  srane
 * Add defines for new test.
 *
 * Revision 1.1  2012/04/18 09:50:18  srane
 * Initial checkin
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */


