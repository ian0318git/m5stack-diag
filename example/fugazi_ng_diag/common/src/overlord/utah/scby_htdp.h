/* $Id: scby_htdp.h,v 1.1 2013/07/22 19:50:39 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/utah/scby_htdp.h,v $
 *------------------------------------------------------------------
 *
 * scby_htdp.h --header for espresso xaui
 *
 * 2013
 *
 * Copyright (c) 2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __SCBY
#define __SCBY

#define HT_OFFSET 0x100000
#define RING_BASE_ADDR1    0x1B0
#define RING_BASE_ADDR2    0x1B8

#define SCBY_PAD(from_adr, to_adr, name) \
unsigned int name[(to_adr - from_adr)/sizeof(unsigned int)]

#define HTD_INT_FBCTL		0x80000000	/* free buff error (or) */
#define HTD_INT_FRCPU		0x40000000	/* from cpu  error (or) */
#define HTD_INT_TOCPU		0x20000000	/* to   cpu  error (or) */
#define HTD_INT_NPWR_HTFC	0x08000000	/* ht err -non posted FC */
#define HTD_INT_NPWR_FBCT	0x04000000	/* ht err -non posted FB */
#define HTD_INT_NPWR_FCPU	0x02000000	/* ht err -non posted Fcpu */
#define HTD_INT_NPWR_TCPU	0x01000000	/* ht err -non posted Tcpu */
#define HTD_INT_FCPU1A_Q3  	0x00800000	/* level from cpu1a Q3 fifo*/
#define HTD_INT_FCPU1A_Q2  	0x00400000	/* level from cpu1a Q2 fifo*/
#define HTD_INT_FCPU1A_Q1  	0x00200000	/* level from cpu1a Q1 fifo*/
#define HTD_INT_FCPU1A_Q0  	0x00100000	/* level from cpu1a Q0 fifo*/
#define HTD_INT_FCPU0A_Q3  	0x00080000	/* level from cpu0a Q3 fifo*/
#define HTD_INT_FCPU0A_Q2  	0x00040000	/* level from cpu0a Q2 fifo*/
#define HTD_INT_FCPU0A_Q1  	0x00020000	/* level from cpu0a Q1 fifo*/
#define HTD_INT_FCPU0A_Q0  	0x00010000	/* level from cpu0a Q0 fifo*/
#define HTD_INT_FCPU1_Q3  	0x00008000	/* level from cpu1  Q3 fifo*/
#define HTD_INT_FCPU1_Q2  	0x00004000	/* level from cpu1  Q2 fifo*/
#define HTD_INT_FCPU1_Q1  	0x00002000	/* level from cpu1  Q1 fifo*/
#define HTD_INT_FCPU1_Q0  	0x00001000	/* level from cpu1  Q0 fifo*/
#define HTD_INT_FCPU0_Q3  	0x00000800	/* level from cpu0  Q3 fifo*/
#define HTD_INT_FCPU0_Q2  	0x00000400	/* level from cpu0  Q2 fifo*/
#define HTD_INT_FCPU0_Q1  	0x00000200	/* level from cpu0  Q1 fifo*/
#define HTD_INT_FCPU0_Q0  	0x00000100	/* level from cpu0  Q0 fifo*/
#define HTD_INT_FB_LOW	  	0x00000020	/* Free Buffer is low      */
#define HTD_INT_FB_TCPU		0x00000010	/* >=1 Tcpu FB desc added  */
#define HTD_INT_TCPU1		0x00000004	/* >=1 Tcpu1 desc added  */
#define HTD_INT_TCPU0		0x00000001	/* >=1 Tcpu0 desc added  */

#define FCPU_RP_SZ		64


/* FB_EXTADDR_SZ  */
#define HTD_FBENT_DEF		0x00000020	/* Tony default ? */
//	volatile unsigned int fb_count;		/* 0xb60 fb count */
#define HTD_FB_RBCNT_M		0x1f000000	/* non-recy buf count */
#define HTD_FB_FBCNT_M		0x0001ffff	/* fb_cnt */
                                                   
#define HTD_TCPUCFG_EN		0x80000000	/* enable descriptor ring */
#define HTD_TCPUCFG_ERRDIS	0x40000000	/* on err auto disab descring*/
#define HTD_TCPUCFG_FBLOW	0x20000000	/* assert FC when fb is low*/
// changing HDREXP affects debug flags and htp_flag2
#define HTD_TCPUCFG_HDREXP_0	0x00000000	/* hdr exp sel 00= 0bytes*/
#define HTD_TCPUCFG_HDREXP_64	0x01000000	/* hdr exp sel 01=64bytes*/
#define HTD_TCPUCFG_HDREXP_128	0x02000000	/* hdr exp sel 10=128bytes*/
#define HTD_TCPUCFG_HDREXP_256	0x03000000	/* hdr exp sel 11=256bytes*/
#define HTD_TCPUCFG_LOWMRK_M	0x00ff0000	/* low water thrsh - descring*/
#define HTD_TCPUCFG_DESCRING_SZ	0x0000ffff	/* length desc ring (size)*/

#define HTD_TCPU_FBCFG_EN	0x80000000	/* enable */
#define HTD_TCPU_FBCFG_DSZ_MSB	0x0000fff8	/* len of desc ring msb */
#define HTD_TCPU_FBCFG_DSZ_LSB	0x00000007	/* len of desc ring lsb */


#define HTD_FCPUCFG_EN		0x80000000	/* enable descriptor ring */
#define HTD_FCPUCFG_DESCRING_SZ	0x0000ffff	/* length desc ring (size)*/


#define FCPU_NDEST       6
#define FCPU_NPRIORITY	4
#define FCPU_DESC_FIFO  FCPU_NPRIORITY * FCPU_NDEST 
#define NPRIORITY	4			/* high and low */
#define TCPU_NPRIORITY	4		/* high and low */
#define NQUEUES		4			/* 4 queues     */
#define NCPUS		4			/* 4 cpu 0,1,0a,1a */
#define NCPUS_NS	2			/* 2 cpu 0,1  (no shadow)*/
#define NNOTRINGS	2			/* 2 notification rings */

typedef enum {
    LP = 0,  
    HP1,
    HP2,
    FC,
} tcpu_priority;


typedef enum {
    FCPU_Q3_RPTR = 0,
    FCPU_Q2_RPTR,
    FCPU_Q1_RPTR,
    FCPU_Q0_RPTR,
    FCPU_Q7_RPTR,
    FCPU_Q6_RPTR,
    FCPU_Q5_RPTR,
    FCPU_Q4_RPTR,
    FCPU_Q11_RPTR,
    FCPU_Q10_RPTR,
    FCPU_Q9_RPTR,
    FCPU_Q8_RPTR,
    FCPU_Q15_RPTR,
    FCPU_Q14_RPTR,
    FCPU_Q13_RPTR,
    FCPU_Q12_RPTR,
    FCPU_Q19_RPTR,
    FCPU_Q18_RPTR,
    FCPU_Q17_RPTR,
    FCPU_Q16_RPTR,
    FCPU_Q23_RPTR,
    FCPU_Q22_RPTR,
    FCPU_Q21_RPTR,
    FCPU_Q20_RPTR,
    TCPU_FC_WPTR,
    TCPU_HP2_WPTR,
    TCPU_HP1_WPTR,
    TCPU_LP_WPTR,
    FREE_BUF_FILL,
    RESERVE2,
    RESERVE1,
    RESERVE0,
} tcpu_rdwr_ptr;


typedef struct fcpu_desc_ {
    volatile unsigned int	lenflags;	/* FCpu desc len + flags */
    volatile unsigned int	bptr;	/* where pkt is */
    volatile unsigned int	swdata;	/* sw data field trigger*/
    volatile unsigned int	spare;	/* not used*/
} fcpu_desc_t;

typedef struct tcpu_desc_ {
    volatile unsigned int	lenflaengs;	/* FCpu desc len + flags */
    volatile unsigned int	bptr;	/* where pkt is */
    volatile unsigned int	swdata;	/* sw data field trigger*/
    volatile unsigned int	spare;	/* not used*/
} tcpu_desc_t;

struct fb_desc {	/* free buffer pool entry */
    unsigned int bptr;  
    unsigned int swdata;	/* put queue number or ptr here */
};

#define  FCPU_R_SZ              32
#define  FCPU_FLAG_EOP		0x00000002
#define  FCPU_FLAG_SOP		0x00000004
#define  FCPU_FLAG_REUSE	0x00000008	/* recycle/reuse buffer */
#define  FCPU_FLAG_RPU		0x00000080	/* read ptr update (defined 40) */
#define  FCPU_FLAG_M		0x000000ff
#define  FCPU_TXOFFS_M		0x000fff00
#define  FCPU_TXOFFS_S		8
#define  FCPU_BUFLEN_M		0xfff00000
#define  FCPU_BUFLEN_S		20
#define  FCPU_RP_SZ		64
#define  HTS_BLEN               1024
typedef struct _ring_base_ {
    volatile int lp;
    volatile int hp1;
    volatile int hp2;
    volatile int fc;
} ring_base_reg_t;

struct fpga_ht {
    //    union {
        //        volatile unsigned long fcpu_desc_n[2];
    fcpu_desc_t fcpu_desc[FCPU_NDEST*FCPU_NPRIORITY]; 
    //    } fcpu_d;
    volatile unsigned int ctrl; /* 0x180 - 0x183 */

#define HTS_SERDES_RESET_L  1
#define HTS_HIGIG_RESET_L  2
#define HTS_SERDES_LPBK  4
#define HTS_FCPU_EN      8
#define HTS_TCPU_EN      0x10
#define HTS_FBUF_EN      0x20
#define HTS_HIG2_TX_EN  0x40
#define HTS_HIG2_RX_EN  0x80
#define HTS_PTR_UPD_RT_MSK   0xF00
#define HTS_PTR_UPD_RT_SHFT   8
#define HTS_TCPU_HDR_EXP_MSK  0x3000
#define HTS_TCPU_HDR_EXP_SHFT 12
    

    volatile unsigned int cmd_prm; /* 0x180 */
#define HTS_INTR_LVL_CMD     0x20000000
#define HTS_UPDATE_CMD       0x40000000
#define HTS_CMD_PRM_MSK      0x00FFFFFF
#define HTS_CMD_CH_SHFT      24
    
    volatile unsigned int sts_l; /* 0x188 */
    volatile unsigned int sts_h; /* 0x188 */
    volatile unsigned int ecc_ctrl_l; /* 0x190 */
    volatile unsigned int ecc_ctrl_h; /* 0x194 */
    volatile unsigned int ecc_sts_l; /* 0x198 */
    volatile unsigned int ecc_sts_h; /* 0x198 */
    volatile unsigned int hig_ctrl_l; /* 0x1A0 */
    volatile unsigned int hig_ctrl_h; /* 0x1A0 */
    volatile unsigned int hig_sts_l; /* 0x1A8 */
    volatile unsigned int hig_sts_h; /* 0x1A8 */
    volatile unsigned int rb[TCPU_NPRIORITY];  /* ring base 0x1B0 */

    volatile unsigned int ptr_blk;
    volatile unsigned int ptr_blk_rsv;

    volatile unsigned int fbptr;
    volatile unsigned int fb_sw;
    
    volatile unsigned int mac_addr_l; /* 0x1D8 */
    volatile unsigned int mac_addr_h; /* 0x1D8 */

    volatile unsigned int intr_l;
    volatile unsigned int intr_h;

};

/*---------------------------------------------------------------
$Log: scby_htdp.h,v $
Revision 1.1  2013/07/22 19:50:39  mcharon
move hts headers to utah dir


$Endlog$
*/


#endif
