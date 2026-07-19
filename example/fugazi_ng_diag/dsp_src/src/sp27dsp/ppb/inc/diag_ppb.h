/* $Id: diag_ppb.h,v 1.13 2017/07/11 11:42:45 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/sp27dsp/ppb/inc/diag_ppb.h,v $
 *------------------------------------------------------------------
 * diag_ppb.h
 *      Graffham host-dsp interface
 *
 * Mar 2012, Smita Rane
 *
 * Copyright (c) 2012-2017 by Cisco Systems, Inc.
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
#define DAC_DOT93                2
#define DAC_DEV_ADDR_FOR_WRITE   0x20
#define DAC_DEV_ADDR_FOR_READ    DAC_DEV_ADDR_FOR_WRITE
#define DAC_1DOT5_REG            0xF8
#define DAC_DOT93_REG            0xF9
#define DAC_1DOT5_VOLT_HIGH      0x40
#define DAC_1DOT5_VOLT_LOW       0xC0
#define DAC_DOT93_VOLT_HIGH      0x02
#define DAC_DOT93_VOLT_LOW       0x82

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
#define SELECT_DAC_1DOT5VM_HIGH 0x0008
#define SELECT_DAC_1DOT5VM_LOW  0x0009
#define SELECT_DAC_NO_1DOT5VM   0x000A
#define SELECT_ECC_MEM          0x000B
#define SELECT_DSP_CONSOLE      0x000C
#define SELECT_UART_LPBK        0x000D
#define SELECT_GE1_INT_LPBK     0x000E
#define SELECT_WTINT		0x0010  /* watchdog timer interrupt test */
#define SELECT_UART_LPBK_RESULT 0x0011
#define SELECT_UART_LPBK_STOP   0x0012
#define SELECT_UART_LPBK_RX     0x0013
#define SELECT_DAC_DOT93VM_HIGH 0x0014
#define SELECT_DAC_DOT93VM_LOW  0x0015
#define SELECT_DAC_NO_DOT93VM   0x0016
#define SELECT_ARM11CPU1_BOOT   0x0017
#define SELECT_GE0_LPBK		0x0020  /* Host send packet to DSP test */
#define SELECT_GE1_LPBK		0x0021  /* Host send packet to DSP test */
#define SELECT_MEM_DISP		0x0022  /* Host send packet to DSP test */
#define SELECT_FW_VER_DISP	0x0023  /* Host send packet to DSP test */
#define SELECT_GE_LPBK_PF	0x0040  /* with pause frame */
#define SELECT_GE0_LPBK_PT	0x0080  /* with pass through (Internal) */
#define SELECT_GE1_LPBK_PT	0x0081  /* with pass through */
#define SELECT_INTF_SYNC        0x0082  /* NGVM SYNC interface signals test */
#define SELECT_TDM_INTLPBK	0x0100  /* TDM internal loopback test */
#define SELECT_TDM_EXTLPBK	0x0200  /* TDM external loopback test */
#define SELECT_READY	        0x0300  /* TDM external loopback test */

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



#define SELECT_VIC			0x8000  /* Base for All the tests for VICs */
/* FXS CODEC (PEB2465) related tests */

#define SELECT_CODEC_SET_RING           SELECT_VIC + 0x0011
#define SELECT_CODEC_STOP_RING          SELECT_VIC + 0x0012
#define SELECT_CODEC_CHAN_TONE_ON       SELECT_VIC + 0x0013
#define SELECT_CODEC_CHAN_TONE_OFF      SELECT_VIC + 0x0014
#define SELECT_CODEC_CHAN_INIT          SELECT_VIC + 0x0015
#define SELECT_CODEC_DIGITAL_LOOPBACK   SELECT_VIC + 0x0016

/* FXO CODEC (Si3044) related tests */

#define SELECT_CODEC_SI3044_REG                SELECT_VIC + 0x0022
#define SELECT_CODEC_SI3044_DIGITAL_LPBK       SELECT_VIC + 0x0023
#define SELECT_CODEC_SI3044_INIT               SELECT_VIC + 0x0024
#define SELECT_CODEC_SI3044_ANALOG_LPBK        SELECT_VIC + 0x0025
#define SELECT_CODEC_SI3044_REG_WR             SELECT_VIC + 0x0026
#define SELECT_CODEC_SI3044_REG_RD             SELECT_VIC + 0x0027

/* BRI NT-TE (PEB 3086) related tests */

#define SELECT_BRI_LOOPBACK                    SELECT_VIC + 0x0040

/* FXO CODEC (Si3050) related tests */

#define SELECT_CODEC_SI3050_REG                SELECT_VIC + 0x0052
#define SELECT_CODEC_SI3050_DIGITAL_LPBK       SELECT_VIC + 0x0053
#define SELECT_CODEC_SI3050_INIT               SELECT_VIC + 0x0054
#define SELECT_CODEC_SI3050_ANALOG_LPBK        SELECT_VIC + 0x0055
#define SELECT_CODEC_SI3050_REG_WR             SELECT_VIC + 0x0056
#define SELECT_CODEC_SI3050_REG_RD             SELECT_VIC + 0x0057

/* FXS CODEC (SI3220) related tests */

#define SELECT_CODEC_SI3220_SET_RING           SELECT_VIC + 0x0061
#define SELECT_CODEC_SI3220_STOP_RING          SELECT_VIC + 0x0062
#define SELECT_CODEC_SI3220_CHAN_TONE_ON       SELECT_VIC + 0x0063
#define SELECT_CODEC_SI3220_CHAN_TONE_OFF      SELECT_VIC + 0x0064
#define SELECT_CODEC_SI3220_CHAN_INIT          SELECT_VIC + 0x0065
#define SELECT_CODEC_SI3220_DIGITAL_LOOPBACK   SELECT_VIC + 0x0066
#define SELECT_CODEC_SI3220_REG_READ           SELECT_VIC + 0x0067
#define SELECT_CODEC_SI3220_REG_WRITE          SELECT_VIC + 0x0068
#define SELECT_CODEC_SI3220_CALIBRATE          SELECT_VIC + 0x0069

/* VIC3_2E/M (Legerity Codec QL061) related tests */
#define SELECT_CODEC_QL061_SEND_CMD           SELECT_VIC + 0x0070
#define SELECT_CODEC_QL061_READ_REV           SELECT_VIC + 0x0071
#define SELECT_CODEC_QL061_REGS_TEST          SELECT_VIC + 0x0072
#define SELECT_CODEC_QL061_ILB_LPBK           SELECT_VIC + 0x0073
#define SELECT_SINGLE                         0x0

/* FXS CODEC (SI3241) related tests */
 
#define SELECT_CODEC_SI3241_SET_RING           SELECT_VIC + 0x0081
#define SELECT_CODEC_SI3241_STOP_RING          SELECT_VIC + 0x0082
#define SELECT_CODEC_SI3241_CHAN_TONE_ON       SELECT_VIC + 0x0083
#define SELECT_CODEC_SI3241_CHAN_TONE_OFF      SELECT_VIC + 0x0084
#define SELECT_CODEC_SI3241_CHAN_INIT          SELECT_VIC + 0x0085
#define SELECT_CODEC_SI3241_DIGITAL_LOOPBACK   SELECT_VIC + 0x0086
#define SELECT_CODEC_SI3241_REG_READ           SELECT_VIC + 0x0087
#define SELECT_CODEC_SI3241_REG_WRITE          SELECT_VIC + 0x0088
#define SELECT_CODEC_SI3241_CALIBRATE          SELECT_VIC + 0x0089
#define SELECT_CODEC_SI3241_RAM_READ           SELECT_VIC + 0x008A
#define SELECT_CODEC_SI3241_RAM_WRITE          SELECT_VIC + 0x008B
#define SELECT_CODEC_SI3241_PROTECTED          SELECT_VIC + 0x008C
#define SELECT_CODEC_SI3241_LD_PTCH            SELECT_VIC + 0x008D
#define SELECT_CODEC_SI3241_PWR_ALM            SELECT_VIC + 0x008E

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
extern void graff_submenu_prcomplete(void);
extern void eprom_disp(char *cmdargs, uint8_t *arr, int size);
extern void do_mem_md(char* cmdargs);
extern void do_mem_md_off(char* cmdargs);
extern int do_mem_mw(char* cmdargs);
extern int do_mem_mm(char* cmdargs);
extern int do_mem_nm(char* cmdargs); 

#endif /* __DIAG_PPB_H__ */
/*
 * $Log: diag_ppb.h,v $
 * Revision 1.13  2017/07/11 11:42:45  alpeng
 * update sticky tag from HEAD to neptune-branch0
 *
 * Revision 1.12  2017/02/07 03:05:24  alpeng
 * add packet type on packet format for graffham
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


