/* $Id: prince_ecc.h,v 1.1 2013/08/02 10:05:02 xiaoyizh Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/prince/prince_ecc.h,v $
 *------------------------------------------------------------------
 *
 * prince_ecc.h - Prince Zynq ECC Definitions.
 *
 * Xiaoying Zhang -- Jul. 2013.
 *
 * Copyright (c) 2013 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef _PRINCE_ECC_H_
#define _PRINCE_ECC_H_

/* Zynq DDR memory controller registers that are relevant to ECC */
#define ZYNQ_DDRC_CONTROL_REG_OFFSET            0x0
#define ZYNQ_DDRC_T_ZQ_REG_OFFSET               0xA4

/* ECC control register */
#define ZYNQ_DDRC_ECC_CONTROL_REG_OFFSET        0xC4
/* ECC log register */
#define ZYNQ_DDRC_ECC_CE_LOG_REG_OFFSET         0xC8
/* ECC address register */
#define ZYNQ_DDRC_ECC_CE_ADDR_REG_OFFSET        0xCC
/* ECC data[31:0] register */
#define ZYNQ_DDRC_ECC_CE_DATA_31_0_REG_OFFSET   0xD0

/* Uncorrectable error info regsisters */
#define ZYNQ_DDRC_ECC_UE_LOG_REG_OFFSET         0xDC
#define ZYNQ_DDRC_ECC_UE_ADDR_REG_OFFSET        0xE0
#define ZYNQ_DDRC_ECC_UE_DATA_31_0_REG_OFFSET   0xE4

#define ZYNQ_DDRC_ECC_STAT_REG_OFFSET           0xF0
#define ZYNQ_DDRC_ECC_SCRUB_REG_OFFSET          0xF4

/* Control regsiter bitfield definitions */
#define ZYNQ_DDRC_CTRLREG_BUSWIDTH_MASK         0xC
#define ZYNQ_DDRC_CTRLREG_BUSWIDTH_SHIFT        2

#define ZYNQ_DDRCTL_WDTH_16                     1
#define ZYNQ_DDRCTL_WDTH_32                     0

/* ZQ register bitfield definitions */
#define ZYNQ_DDRC_T_ZQ_REG_DDRMODE_MASK         0x2

/* ECC control register bitfield definitions */
#define ZYNQ_DDRC_ECCCTRL_CLR_CE_ERR            0x2
#define ZYNQ_DDRC_ECCCTRL_CLR_UE_ERR            0x1

/* ECC correctable/uncorrectable error log register definitions */
#define ZYNQ_DDRC_ECC_CE_LOGREG_VALID           0x1
#define ZYNQ_DDRC_ECC_CE_LOGREG_BITPOS_MASK     0xFE
#define ZYNQ_DDRC_ECC_CE_LOGREG_BITPOS_SHIFT    1

/* ECC correctable/uncorrectable error address register definitions */
#define ZYNQ_DDRC_ECC_ADDRREG_COL_MASK          0xFFF
#define ZYNQ_DDRC_ECC_ADDRREG_ROW_MASK          0xFFFF000
#define ZYNQ_DDRC_ECC_ADDRREG_ROW_SHIFT         12
#define ZYNQ_DDRC_ECC_ADDRREG_BANK_MASK         0x70000000
#define ZYNQ_DDRC_ECC_ADDRREG_BANK_SHIFT        28

/* ECC statistic regsiter definitions */
#define ZYNQ_DDRC_ECC_STATREG_UECOUNT_MASK      0xFF
#define ZYNQ_DDRC_ECC_STATREG_CECOUNT_MASK      0xFF00
#define ZYNQ_DDRC_ECC_STATREG_CECOUNT_SHIFT     8

/* ECC scrub regsiter definitions */
#define ZYNQ_DDRC_ECC_SCRUBREG_ECC_MODE_MASK    0x7
#define ZYNQ_DDRC_ECC_SCRUBREG_ECCMODE_SECDED   0x4

/*
 * struct ecc_error_info - ECC error log information
 * @row:   Row number
 * @col:   Column number
 * @bank:  Bank number
 * @bitpos:    Bit position
 * @data:  Data causing the error
 */
struct ecc_error_info {
   ulong row;
   ulong col;
   ulong bank;
   ulong bitpos;
   ulong data;
};

/*
 * struct zynq_ecc_status - ECC status information to report
 * @ce_count:  Correctable error count
 * @ue_count:  Uncorrectable error count
 * @ceinfo:    Correctable error log information
 * @ueinfo:    Uncorrectable error log information
 */
struct zynq_ecc_status {
   ulong ce_count;
   ulong ue_count;
   struct ecc_error_info ceinfo;
   struct ecc_error_info ueinfo;
};

/*
 * struct zynq_ecc_status - ECC status information to report
 * @ce_count:  Correctable error count
 * @ue_count:  Uncorrectable error count
 * @ceinfo:    Correctable error log information
 * @ueinfo:    Uncorrectable error log information
 */
struct zynq_ecc_status_reg_info {
   ulong ecc_ce_log;
   ulong ecc_ce_addr;
   ulong ecc_ue_log;
   ulong ecc_ue_addr;
   ulong ecc_stat;
};

#endif /* _PRINCE_ECC_H_ */

/******** History ******** 
$Log: prince_ecc.h,v $
Revision 1.1  2013/08/02 10:05:02  xiaoyizh
Initial check in for Zynq ECC definitions.


$Endlog$
*/
