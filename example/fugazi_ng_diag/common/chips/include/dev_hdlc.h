/* $Id: dev_hdlc.h,v 1.2 2012/03/28 00:38:08 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/include/dev_hdlc.h,v $
 *------------------------------------------------------------------
 *
 * FILE NAME: dev_hdlc.h
 *
 * Common Device Driver for HDLC functional block within Goofy ASIC.
 * Ported from HDLC code in FIO ASIC.
 *
 * April 2007 - Christine Wen
 *
 * Copyright (c) 2007-2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DEV_HDLC_H__
#define __DEV_HDLC_H__

#define HH_PAD unsigned int

/*
 * I/O FPGA hdlc register structure
 */
typedef struct hdlc_regs {
    volatile unsigned int h_sr;			/* 0000 */
    volatile unsigned int h_edtbar; 		/* 0004 */
    volatile unsigned int h_idtbar;		/* 0008 */
    volatile unsigned int h_cer;		/* 000c */
    volatile unsigned int h_edavnr;		/* 0010 */
    volatile unsigned int h_ebrr;		/* 0014 */
    volatile unsigned int h_ipqnr;		/* 0018 */
    volatile unsigned int h_eer;		/* 001c */
    volatile unsigned int h_ier;		/* 0020 */
    volatile unsigned int h_ilur;		/* 0024 */
    volatile unsigned int h_cacr;		/* 0028 */
    volatile unsigned int h_ccr;		/* 002c */
    volatile unsigned int h_cr;			/* 0030 */
    volatile unsigned int h_imfs;               /* 0034 */
    volatile unsigned int h_imfse;              /* 0038 */
    volatile unsigned int h_tdmcr;              /* 003c */
    HH_PAD pad1[0x30];                          /* 0040 - 0xff */ 
    volatile unsigned int h_dba;                /* 0100 */
    volatile unsigned int h_dbdl;               /* 0104 */
    volatile unsigned int h_dbdu;               /* 0108 */
    volatile unsigned int h_fcr;                /* 010c */
    volatile unsigned int h_mpes;               /* 0110 */
    volatile unsigned int h_mpec;               /* 0114 */
    volatile unsigned int h_embpea;             /* 0118 */
    volatile unsigned int h_imbpea;             /* 011c */
    volatile unsigned int h_ecampea;            /* 0120 */
    volatile unsigned int h_icampea;            /* 0124 */
    volatile unsigned int h_par_err_inj;        /* 0128 */
    HH_PAD pad3[0x35];                          /* 012c - 01ff */  
    volatile unsigned int h_stibes;             /* 0200 */
    volatile unsigned int h_stibec;             /* 0204 */
    HH_PAD pad4[0x7e];                          /* 0208 - 03ff */
    volatile unsigned int h_ca_dpram_base;	/* 0400 */
} hdlc_regs_t;

#undef HH_PAD

/*
 * Bit definitions for hdlc interrupt status register (h_sr)
 */
#define HDLC_DEBUG_BUF_READ_INT         0x00000080
#define HDLC_ING_FRM_SIZE_ERR_INT       0x00000040
#define HDLC_CNFG_CMD_DONE_INT          0x00000020
#define HDLC_ING_LNK_UPDWN_INT          0x00000010
#define HDLC_ING_PRTCL_Q_INT            0x00000008
#define HDLC_ING_OVERRUN_INT            0x00000004
#define HDLC_EGR_BUF_REL_INT            0x00000002
#define HDLC_EGR_UNDERRUN_INT           0x00000001

/*
 * Bit definitions for hdlc channel assigner control register (h_cacr)
 */
#define HDLC_CA_ACTIVE_HALF		0x00000002
#define HDLC_CA_R2HALF			0x00000001
#define HDLC_CA_R2HALF_UPPER		0x00000001
#define HDLC_CA_R2HALF_LOWER		0x00000000

/*
 * Bit definitions for hdlc interrupt control register (h_cr)
 */
#define HDLC_DEBUG_BUF_READ_INT_EN      0x00000080
#define HDLC_ING_FRM_SIZE_ERR_INT_EN    0x00000040
#define HDLC_CNFG_CMD_DONE_INT_EN       0x00000020
#define HDLC_ING_LNK_UPDWN_INT_EN       0x00000010
#define HDLC_ING_PRTCL_Q_INT_EN         0x00000008
#define HDLC_ING_OVERRUN_INT_EN         0x00000004
#define HDLC_EGR_BUF_REL_INT_EN         0x00000002
#define HDLC_EGR_UNDERRUN_INT_EN        0x00000001
#define HDLC_NET_INTR_EN_ALL            0x0000007f
#define HDLC_NET_EG_INT_EN_ALL          0x00000033
#define HDLC_NET_ING_INT_EN_ALL         0x0000004c

/*
 * Bit definitions for hdlc configuration command register (h_ccr)
 */
#define HDLC_CMD_EXECUTE		0x00040000

#define HDLC_CMD_LINK			0x00000000
#define HDLC_CMD_CFG			0x00020000
#define HDLC_CMD_EGRESS	        	0x00000000
#define HDLC_CMD_INGRESS		0x00010000
#define HDLC_CHAN_CFG_MASK		0x0000f800
#define HDLC_CHAN_SHIFT                 11
#define HDLC_SRC_MEM_QUEUE_SHIFT        11
#define HDLC_CRC_32                     0x00100000
#define HDLC_CRC_SHIFT                  20
#define HDLC_IDLE_FLAG_PAT_FF           0x00080000
#define HDLC_IDLE_FLAG_SHIFT            19
#define HDLC_X_IDLE_FLAG_MASK		0x00000780
#define HDLC_INV			0x00000040
#define HDLC_CRC_ON			0x00000020
#define HDLC_TRANSP			0x00000010
#define HDLC_RATE_64K        	        0x00000008
#define HDLC_RATE_16K            	0x00000002
#define HDLC_RATE_8K            	0x00000001
#define HDLC_RATE_MASK	        	0x0000000f

/* 
 * Bit definition for TDM bus control (0x3c) 
 */  
#define HDLC_TDM_E1                     0x00000010
#define HDLC_TDM_2PORT                  0x00000008
#define HDLC_TDM_1PORT                  0x00000004
#define HDLC_TDM_TE                     0x00000002
#define HDLC_TDM_SRC_SWITCH             0x00000001
#define HDLC_TDM_SRC_WIC                0x00000000
#define HDLC_TDM_VIC_MODE_SHIFT         4

/* 
 * Bit definition for eg/ing mem buf par err address (0x40 and 0x44) 
 */
#define HDLC_MEM_BUF_PAR_ERR_ADDR_MASK  0x000003ff

/* 
 * Bit definition for eg/ing ca mem par err address (0x48 and 0x4c) 
 */
#define HDLC_CA_MEM_PAR_ERR_ADDR_MASK   0x000000ff

/* 
 * Bit definition for Debug Buffer Address (0x0100) 
 */
#define HDLC_DBA_EGRESS                 0x00000000
#define HDLC_DBA_INGRESS                0x80000000
#define HDLC_DBA_ADDR_MASK              0x000003ff

#define HDLC_DBG_CHAN_OFFSET            0x8

/* 
 * Bit definition for Debug Buffer Data Upper (0x0108) 
 */
#define HDLC_DBDU_DATA_READY            0x80000000
#define HDLC_DBDU_DATA_MASK             0x0000ffff

/* 
 * Bit definition for Memory Parity Error Status register (0x0110) 
 */
#define HDLC_ING_CA_MEM_PAR_ERR_INT     0x00000008
#define HDLC_EG_CA_MEM_PAR_ERR_INT      0x00000004
#define HDLC_ING_MEM_BUF_PAR_ERR_INT    0x00000002
#define HDLC_EG_MEM_BUF_PAR_ERR_INT     0x00000001

/* 
 * Bit definition for Memory Parity Error Control register (0x0114) 
 */
#define HDLC_ING_CA_MEM_PAR_ERR_INT_EN  0x00000008
#define HDLC_EG_CA_MEM_PAR_ERR_INT_EN   0x00000004
#define HDLC_ING_MEM_BUF_PAR_ERR_INT_EN 0x00000002
#define HDLC_EG_MEM_BUF_PAR_ERR_INT_EN  0x00000001
#define HDLC_MEM_PAR_ERR_INT_EN_ALL     0x0000000f

/* 
 * Bit definition for HDLC STI Bus Error Status (0x0200) 
 */
#define HDLC_INIT_RESP_NXA              0x00000040
#define HDLC_INIT_RESP_ERR              0x00000020
#define HDLC_INIT_RESP_SIZE_ERR         0x00000010
#define HDLC_INIT_RESP_CMD_ERR          0x00000008
#define HDLC_TARG_BE_ERR                0x00000004
#define HDLC_RDWR_SIZE_ERR              0x00000002
#define HDLC_NP_WRITE_ERR               0x00000001

/* 
 * Bit definition for HDLC STI Bus Error Control (0x0204) 
 */
#define HDLC_INIT_RESP_NXA_EN           0x00000040
#define HDLC_INIT_RESP_ERR_EN           0x00000020
#define HDLC_INIT_RESP_SIZE_ERR_EN      0x00000010
#define HDLC_INIT_RESP_CMD_ERR_EN       0x00000008
#define HDLC_TARG_BE_ERR_EN             0x00000004
#define HDLC_RDWR_SIZE_ERR_EN           0x00000002
#define HDLC_NP_WRITE_ERR_EN            0x00000001
#define HDLC_STI_ERR_EN_ALL             0x0000007f

/*
 * Bit definitions for hdlc channel assigner dpram
 */
#define HDLC_DPRAM_EGR_ACTIVE		0x8000
#define HDLC_DPRAM_EGR_CHAN_MASK	0x1f00
#define HDLC_DPRAM_EGR_CHAN_SHIFT	8
#define HDLC_DPRAM_ING_ACTIVE		0x0080
#define HDLC_DPRAM_ING_CHAN_MASK	0x001f
#define HDLC_DPRAM_MASK			0x9f9f
#define HDLC_DPRAM_SIZE			256       /* total entries */
#define HDLC_CA_DPRAM_ENTRIES           128       /* 4 bytes/entry */
#define HDLC_CA_DPRAM_ENTRIES_VALID     32        /* Goofy supports 2Mbps HDLC channel */
#define HDLC_CHAN_ALL		        0xffffffff

/* HDLC Transmmit Buffer Descriptor: */
typedef struct hdlc32_egress_bd_ {
    unsigned int data_ptr; /* has to be type unsigned int for DiagLinux */
    unsigned int description;
} hdlc32_egress_bd_t;

/* HDLC Receive Buffer Descriptor: Ingress*/
typedef struct hdlc32_ingress_bd_ {
    unsigned int data_ptr; /* has to be type unsigned int for DiagLinux */
    unsigned int description;
} hdlc32_ingress_bd_t;

/* Egress Buffer Descriptor */
#define EGR_DESC_OWNERSHIP      	0x80000000
#define EGR_DESC_SOF	        	0x20000000
#define EGR_DESC_EOF	        	0x10000000
#define EGR_DESC_NOTIFY	        	0x08000000
#define EGR_LENGTH_MASK	        	0x01ffc000
#define EGR_LENGTH_SHIFT        	14
#define EGR_UNDERRUN                    0x00002000
#define EGR_NEXT_QUAD_MASK      	0x00000f80
#define EGR_NEXT_QUAD_SHIFT     	7
#define EGR_CHAN_NUM_MASK       	0x0000001f

/* Ingress Payload Descriptor */
#define ING_DESC_OWNERSHIP      	0x80000000
#define ING_DESC_ABORT	        	0x40000000
#define ING_DESC_SOF	        	0x20000000
#define ING_DESC_EOF     		0x10000000
#define ING_NOTIFY	        	0x08000000
#define ING_CRC_ERROR	        	0x04000000
#define ING_RNT_ERROR        		0x02000000
#define ING_LENGTH_MASK		        0x01ffc000
#define ING_LENGTH_SHIFT	        14
/* this ingress length definition is different from FIO */
#define ING_MAX_BUF_1024	        0x01000000
#define ING_MAX_BUF_2044	        0x01ffc000
#define ING_OVERRUN                     0x00002000
#define ING_FRM_SIZE_ERROR              0x00001000
#define ING_ERR_MASK		        ING_DESC_ABORT | ING_CRC_ERROR | ING_RNT_ERROR \
                                        | ING_OVERRUN | ING_FRM_SIZE_ERROR
#define ING_NEXT_QUAD_MASK	        0x00000f80
#define ING_NEXT_QUAD_SHIFT	        7
#define ING_CHAN_NUM_MASK	        0x0000001f

#define MASK_1K                         0x000003FF
#define PAK_TEST_SIZE                   128
#define HDLC_DATA_BUF_LENGTH            1024
#define HDLC32_TXBD_PER_QUAD	        16
/* YWEN: For Goofy, this should be changed to 16 ingress BD per queue */
#define HDLC32_RXBD_PER_QUAD	        16
#define HDLC32_MAX_CHAN                 32

#define HDLC_LOG_BUF_SIZE               380
#define HDLC_INTR_WAIT_TIME             4000   /* 2 sec */
#define HDLC_ERR_WAIT_TIME              200    /* 200ms */

typedef enum hdlc_trans_dir_ {
    HDLC_EGRESS,
    HDLC_INGRESS,
} hdlc_trans_dir;

typedef enum tdm_bus_t_ {
    WIC_TDM,
    TDM_SWITCH,
} tdm_bus_t;

typedef enum crc_type_ {
    CRC16,
    CRC32,
} crc_type;

typedef enum idle_flag_pat_ {
    IDLE_FLAG_7E,
    IDLE_FLAG_FF,
} idle_flag_pat;

typedef enum oper_mode_ {
    TRANSPARENT = 1,
    INVERT,
    RATE_8K,
    RATE_16K,
    RATE_64K,
} oper_mode;

typedef enum hdlc_err_t_ {
    UNDERRUN = 1,
    OVERRUN,
    FRM_SIZE_ERR,
} hdlc_err_t;

typedef struct hdlc32_egress_bd_q_t_ {
    hdlc32_egress_bd_t bd_array[HDLC32_MAX_CHAN][HDLC32_TXBD_PER_QUAD];
} hdlc32_egress_bd_q_t;

typedef struct hdlc32_ingress_bd_q_t_ {
    hdlc32_ingress_bd_t bd_array[HDLC32_MAX_CHAN][HDLC32_RXBD_PER_QUAD];
} hdlc32_ingress_bd_q_t;

typedef struct hdlc32_egress_buf_q_t_ {
    unsigned char buf_array[HDLC32_MAX_CHAN][HDLC32_TXBD_PER_QUAD][HDLC_DATA_BUF_LENGTH];
} hdlc32_egress_buf_q_t;

typedef struct hdlc32_ingress_buf_q_t_ {
    unsigned char buf_array[HDLC32_MAX_CHAN][HDLC32_RXBD_PER_QUAD][HDLC_DATA_BUF_LENGTH];
} hdlc32_ingress_buf_q_t;

/*
 * device callin function - service provided and defined by the device
 */
#ifndef LINUX_KLM
typedef struct dev_hdlc_callin_fvt_ {
    int  (*register_test)(dev_object_t *);
    int  (*dpram_test)(dev_object_t *);
    int  (*hdlc_lpbk)(dev_object_t *, int, int, int, int, crc_type, 
		      idle_flag_pat, oper_mode, int, int, int, int, int);
    int  (*hdlc_mlpbk)(dev_object_t *, int, int, int, crc_type, 
		       idle_flag_pat, oper_mode);
    void (*config_tdm_bus)(dev_object_t *, tdm_bus_t, int, int, int); 
    int  (*hdlc_error_test)(dev_object_t *, hdlc_err_t);
    void (*hdlc_show_debug_info)(dev_object_t *, uint);
} dev_hdlc_callin_fvt_t;

/*
 * device callout function - service needed by the device
 *                           and defined by platform
 */
typedef struct dev_hdlc_callout_fvt_ {
    int (*set_hdlc_intr)(dev_object_t *dev, goofy_intr_class_t class,
			 unsigned int intr_num, unsigned int type, unsigned int cpu,
			 boolean enable);
    PFI (*install_isr_vect)(dev_object_t *dev, goofy_intr_class_t class,
			    unsigned int intr_num, unsigned int type, PFI vect);
    void * (*get_plat_goofy_inst)(goofy_dev_t mod_type, int mod_num);
    ulong (*make_phy_addr)(ulong address);
    ulong (*make_vir_addr)(ulong address);
} dev_hdlc_callout_fvt_t;

/* define the structure for HDLC device specific information */
typedef struct dev_hdlc_dep_t_ {
    /* pointers for egress/ingress buffer descriptor queues and buffer queues */
    void                    *hdlc32_egress_bdq_ptr;
    void                    *hdlc32_ingress_bdq_ptr;
    hdlc32_egress_bd_q_t    *hdlc32_egress_bd_q;
    hdlc32_ingress_bd_q_t   *hdlc32_ingress_bd_q;
    hdlc32_egress_buf_q_t   *hdlc32_egress_buf_q;
    hdlc32_ingress_buf_q_t  *hdlc32_ingress_buf_q;
    /* Buffer pointer for error or information log */
    unsigned char                   *log_buffer;
    /* pointers for HDLC device function vector table */
    dev_object_fvt_t        *hdlc_fvt;
    dev_hdlc_callin_fvt_t   *hdlc_callin;
    dev_hdlc_callout_fvt_t  *hdlc_callout;
    /* this flag is for underrun, overrun and ing frame size error tests to
       not call cterr() */
    int                     test_flag; 
    /* this variable is used to save which HDLC controller it is. 
     set_hdlc_intr() need this info */
    int                     hdlc_controller;
} dev_hdlc_dep_t;
#endif  /* linux_klm */

/* define the structure for HDLC device interrupt specific information */
typedef struct dev_hdlc_intr_dep_t_ {
    int hdlc_ingr_prtcl_q_int;
    int hdlc_eg_buf_rel_int;
    volatile unsigned int hdlc_sr;
    unsigned int hdlc_dbdu;
    unsigned int hdlc_imfse;
    unsigned int hdlc_ilur;
    unsigned int hdlc_ipqnr;
    unsigned int hdlc_ier;
    unsigned int hdlc_ebrr;
    unsigned int hdlc_eer;
    unsigned int hdlc_mpes;
    unsigned int hdlc_icampea;
    unsigned int hdlc_ecampea;
    unsigned int hdlc_imbpea;
    unsigned int hdlc_embpea;
    unsigned int hdlc_stibes;
}dev_hdlc_intr_dep_t;

#ifndef LINUX_KLM
/*
 * Define the HDLC device object structure.
 */
typedef struct dev_hdlc_object_t_ {
    dev_object_t           base;
    dev_hdlc_callout_fvt_t *callout_fvt;
    dev_hdlc_callin_fvt_t  *callin_fvt;
    dev_hdlc_dep_t         hdlc_dep; 
    dev_hdlc_intr_dep_t    hdlc_intr_dep; 
}dev_hdlc_object_t;

/* extern definitions */
extern int dev_hdlc_create(dev_object_t *dev, dev_error_report_t error_report_fn);
extern int sw128tdm_display_reg_all(void); /* Need for debugging */

#endif  /* LINUX_KLM */
#endif  /* __DEV_HDLC_H__ */

/******** History ******** 
$Log: dev_hdlc.h,v $
Revision 1.2  2012/03/28 00:38:08  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:02  ptong
Initial archive of ng_diag module


$Endlog$
*/
