/* $Id: graffdsp.h,v 1.2 2012/12/24 00:06:49 srane Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/sp27dsp/dss/inc/graffdsp.h,v $
 *------------------------------------------------------------------
 * graffdsp.h
 *      Graffham - header file 
 *
 * Mar 2012, Smita Rane
 *
 * Copyright (c) 2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#ifndef MONTDSP_H_
#define MONTDSP_H_


/******************************************
 * DSP Data Types	  *
 ******************************************/
#define INT16 short
#define UINT16 unsigned short
#define INT32 int
#define UINT32 unsigned int
#define UINT8 unsigned char


typedef	unsigned short	    uint;	/* System V compatibility */
typedef unsigned int	    ulong;	/* System V compatibility */
typedef int Bool;


/******************************************
 * DSP TNETV2685 Diag Defines		  *
 ******************************************/
#define STATUS int
#define OK 1
//#define ERROR 0
#define PASSED 0
#define FAILED 1

#define TRUE    (1)
#define FALSE   (0)

typedef ulong (*PFI)();
typedef struct diag_table diag_table_t;
struct diag_table {
    int test_select;  /* test selection */
    PFI diag;     /* test function */
};
/* test communication strucure used in main */
typedef struct {
    UINT32   command;
    UINT32   ack;                /* ACK   */
    UINT32   result;     /*  */
    UINT32   flags;     /* */
    UINT32	 select;
    UINT32   faults;     /* */
    UINT32   location;     /* */
    UINT32   expected;     /* */
    UINT32   actual;     /* */
    UINT32   extra;     /* */
    UINT32   errorcount;     /* */
    UINT32   testcounter;     /* */
    UINT32   ReadyOnTest;     /* */
    UINT32   TestCtrl;     /* */
    UINT32   WhoAmI;     /* could be dspid_t*/
    UINT32   ver_no;     /* */
    UINT32   wait_states;     /* */
    UINT32   param1;
    UINT32   param2;
    UINT32   param3;
    UINT32   param4;
	UINT32   silab_chip;
    UINT32   mstrstat_b;
    UINT32   mstrstat_a;
    UINT8    bufmsg[128]; /* data buffer for host/dsp intf */
    UINT8    errmsg[128]; /* data buffer for host/dsp intf */
} dspif_info_t;
/* Host Interface Structure - operation type defines */
#define OP_NULL            0
#define OP_TEST_REQUEST    1   /* host->DSP : run test */
#define OP_INFO_REQUEST    2   /* host->DSP : get info/data */
#define OP_DSP_REQUEST     3   /* DSP->host: not likely to be used */
#define OP_RESPONSE        4   /* mutual: send back test result, info  */
#define OP_READY           5   /* DSP->host : DSP is up and ready */
#define OP_TEST_STOP       6   /* host->DSP: to stop current test */

/* Host Interface Structure - command defines */
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


#define SELECT_NULL         0x0000
#define SELECT_DSP_SANITY   0x0001  /* DSP Integrity test */
#define SELECT_DSP_SDRAM        0x0002  /* SDRAM test */
#define SELECT_HOST_INT         0x0004  /* Host Interrupt test */
#define SELECT_DSP_INT          0x0008  /* DSP Interrupt test */
#define SELECT_WTINT            0x0010  /* watchdog timer interrupt test */
#define SELECT_GE_LPBK          0x0020  /* Host send packet to DSP test */
#define SELECT_GE_LPBK_PF       0x0040  /* with pause frame */
#define SELECT_GE_LPBK_PT       0x0080  /* with pass through */
#define SELECT_INTF_SYNC        0x0082  /* NGVM SYNC interface signals test */
#define SELECT_TDM_INTLPBK      0x0100  /* TDM internal loopback test */
#define SELECT_TDM_EXTLPBK      0x0200  /* TDM external loopback test */
#define SELECT_TDM_HOSTLPBK     0x0400  /* TDM external loopback test */

#define SELECT_VIC				0x8000  /* Base for All the tests for VICs */

#define CHECK_LEVEL   0x100 
#define CHECK_LOW     0x100
#define CHECK_HIGH    0x101
#define SET_LEVEL   0x10 
#define SET_LOW     0x10
#define SET_HIGH    0x11

/* Shamu */
#define SELECT_CODEC_DSP_SEL_TDM		SELECT_VIC + 0x0010
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
#define SELECT_CODEC_SI3241_TDMSW_LPBK         SELECT_VIC + 0x008F

/* TEMP HOST ID */
#define HOST_ID         0xfacefeed

#ifndef NULL
#define NULL    0
#endif
#define PATTERN 0
#define INC_PAT 1
#define DEC_PAT 2



#define ERR_PTR    ((char *)&errmsg)
#define BUF_PTR    ((char *)&bufmsg)

#define LOW_IRAM      0x00B10000
#define HIGH_IRAM     0x00B5FFFF

#define LOW_SDRAM      0x10000000
#define HIGH_SDRAM	   0x20000000 /* 256 MB */
#define HIGH_SDRAM_64M 0x14000000 /* 64 MB */

#define HPI_START      0x00A00000
#define HPI_END        0x00A02DFF

#define TOP_STACK      0xEF00
#define BOTTOM_STACK   0xF000

/* Test Packets */

typedef struct
{
    int length;
    int chanNum;
    int msgID;
    int processID;
    unsigned int *data;
} PACKET;

#define DPRAM_EOF       0x55            /* end of frame character */
#define DPRAM_BASE      (0x00a00000 + 0x1000)
#define DPRAM_END       (0x00a00000 + 0x2000) 

#define HPIRAM(offset)  ((volatile unsigned short *)((offset)+DPRAM_BASE))

#define HID_MAGIC                       0
#define HID_RX_START_ADDR               1
#define HID_RX_BUF_SIZE                 2
#define HID_TX_START_ADDR               3
#define HID_TX_BUF_SIZE                 4
#define HID_MAX_MSG_SIZE                5
#define HID_RX_END                      6
#define HID_TX_END                      7
#define HID_RX_BUF_INSERT               8
#define HID_RX_BUF_REMOVE               9
#define HID_TX_BUF_REMOVE               10
#define HID_TX_BUF_INSERT               11
#define HID_ERROR_CODE                  12

#define TRIVIAL_BUF     0xffff
#define MAX_MSG_SIZE    1500/2          /* 1500/2 words, 1500 bytes */
#define MAGIC           43              /* for CHPI interface */
#define STATUS_OK       0xbb

#define RX_BUF_SIZE     0x700
#define TX_BUF_SIZE     0x700

#define RXBUF_START     0x1100
#define RXBUF_END       RXBUF_START + RX_BUF_SIZE -1

#define TXBUF_START     RXBUF_END + 1
#define TXBUF_END       TXBUF_START + TX_BUF_SIZE -1

#define PCK_HDR_SIZE 4	
#define SYSMEM_END 0xC02FFFFF     // DSS MAP
#define MEM_STRIDE  2048/4       // 2KB jumps, note code using 32-bit pointers so divide by 4

#define VAL1 0xAAAA5555

#define BLOCKS_per_BANK 128





#define AG_MG_DDR2_CTL_LOCK		(0x00000001 << 20)

/* Note: SP2500_PP.TXT defaults to using DTCM for ZI memory and Stack */
#define DTCM_BASE		0x08000000
#define DTCM_SIZE		0x00010000

#define PPBSRAM_BASE	0x10000000
#define PPBSRAM_SIZE	0x00040000

#define PCEMEM_BASE		0x18000000
#define PCEMEM_SIZE		0x0000C000

#define DSS0LMEM_BASE	0x80000000
#define DSS0LMEM_SIZE	0x00040000

#define DSS1LMEM_BASE	0x88000000
#define DSS1LMEM_SIZE	0x00040000

#define DSS2LMEM_BASE	0x90000000
#define DSS2LMEM_SIZE	0x00040000

#define SMEM_BASE		0xC0000000
#define SMEM_SIZE		0x00300000
#define SMEM_BANKS      12

#define ARM_I2C_BASE	0x30000000
#define ARM_TIMER_BASE	0x30001000
#define ARM_VIC_BASE	0x30010000

#define DDR2_BASE		 0x10000000
#define DDR2MEM_512SIZE  0x20000000 /* Target */
#define DDR2MEM_256SIZE  0x10000000	/* EVM */
#define DDR2MEM_128SIZE  0x08000000	/* Some Union Board Configurations */
#define DDR2MEM_64SIZE  0x04000000	/* Some Union Board Configurations */
#define AGR_SP26XX_DDR2_BASE 0xC2008000

#define ON 1
#define OFF 0

/* options for DDR2 SDRAM drive strength  */
#define FULL 0
#define REDUCED 1

typedef struct {
        UINT8 device_type;
        UINT8 device_freq;
        UINT8 chip_id;
        UINT8 core_id;
        UINT8 num_chan;
} dsp_type_t;

#define DSP_SP2603_ID     0x0
#define DSP_SP2601_ID     0x1
#define DSP_SP2602_ID     0x2
#define DSP_500MHZ        0x0
#define DSP_350MHZ        0x1
#define DSP_250MHZ        0x2
#define DSP_550MHZ        0x0
#define DSP_400MHZ        0x1
#define DSP_300MHZ        0x2
/* DSP core ID */
#define DSS_CORE0       0x0
#define DSS_CORE1       0x1
#define DSS_CORE2       0x2
#define DSS_CORE3       0x3
//volatile  agr_sp26xx_ddr2_regs_s DDR2_REG[1] = AGR_SP26XX_DDR2_BASE;

//#define DDR2_REG ((volatile agr_sp26xx_ddr2_regs_s *)AGR_SP26XX_DDR2_BASE)

#define TDM_INT_LPBK	0x0
#define TDM_EXT_LPBK	0x1

/* PLL Parameters */
#define F_CKI 25                        /* in MHz */
#define MULTIPLIER_500 40               /* required multiplier for VCO = 2 x fDSP */
#define MULTIPLIER_400 32               /* required multiplier for VCO = 2 x fDSP */
#define MULTIPLIER_450 36               /* required multiplier for VCO = 2 x fDSP */
#define MULTIPLIER_250 20               
#define MULTIPLIER_350 28               
#define INPUT_DIVIDER 1         /* desired PLL input divider  */
#define OUTPUT_DIVIDER 2        /* desired PLL input divider  */
#define RUN_PLL 1
#define PLL_BYP         0
#define PLL_DISABLE 0
#define OUTPUT_DIV_2 2          /* used in pll2 setup for ddr2 port - defined desired divider,   */
#define OUTPUT_DIV_4 4
#define OUTPUT_DIV_8 8          /* note converted to proper PLL2SEL reg. value in enable_ddr2pll()  */


#endif /*MONTDSP_H_*/

/* 
 * $Log: graffdsp.h,v $
 * Revision 1.2  2012/12/24 00:06:49  srane
 * Support NGVM interface SYNC signal defines.
 *
 * Revision 1.1  2012/04/18 22:08:17  srane
 * Initial checkin
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
