/* $Id: mvDdr3PrvIf.h,v 1.1 2015/02/13 11:34:24 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/cpss/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/ddr3/private/mvDdr3PrvIf.h,v $
 *------------------------------------------------------------------
 *
 * Ian Chang - Nov. 2014
 *
 * Copyright (c) 2014-2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 */
/*******************************************************************************
*                Copyright 2001, Marvell International Ltd.
* This code contains confidential information of Marvell semiconductor, inc.
* no rights are granted herein under any patent, mask work right or copyright
* of Marvell or any third party.
* Marvell reserves the right at its sole discretion to request that this code
* be immediately returned to Marvell. This code is provided "as is".
* Marvell makes no warranties, express, implied or otherwise, regarding its
* accuracy, completeness or performance.
********************************************************************************
* mvHwsDdr3PrvIf.h
*
* DESCRIPTION:
*       
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*       $Revision: 1.1 $
******************************************************************************/

#ifndef __mvHwDDR3PrvIf_H
#define __mvHwDDR3PrvIf_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ddr3/mvHwsDdr3InitIf.h>

typedef  enum
{
    TRAINING_FREQ_100, /* slow */
    TRAINING_FREQ_400, /* medium */
    TRAINING_FREQ_800  /* high */
}MV_HWS_TRAINING_FREQ;


/********************/
/* General Consts   */
/********************/
#define LEN_KILLER_PATTERN                      128
#define LEN_SPECIAL_PATTERN                     128
#define LEN_PBS_PATTERN                         5


/**********************/
/* EMC related Consts */
/**********************/
#define LEN_EMC_MAX_PATTERN                     40


/************************/
/* SDRAM related Consts */
/************************/
#define SDRAM_DQS_RX_OFFS                       1024
#define SDRAM_DQS_TX_OFFS                       4096

#define SDRAM_PBS_I_OFFS                        0x40
#define SDRAM_PBS_II_OFFS                       0x80
#define SDRAM_PBS_NEXT_OFFS                     (SDRAM_PBS_II_OFFS - SDRAM_PBS_I_OFFS)

/**********/
/* Consts */
/**********/
#define MAX_DELAY                               0x1F
#define MIN_DELAY                               0

/* max values */
#define MAX_LU_MEM_IF                           14
#define MAX_FWD_MEM_IF                          8
#define MAX_PUP_NUM                             2

/* max bit numbers (5 for LU, for FW(pup0 - 5, pup1 - 4 */
#define PUP_DQ_NUM          5

/* DQS */
#define ADLL_ERROR                              0x55
#define ADLL_MAX                                31
#define ADLL_MIN                                0
#define MIN_WIN_SIZE                            4
#define VALID_WIN_THRS                          ADLL_MAX

/* PBS */
#define MAX_PBS                                 31
#define MIN_PBS                                 0
#define COUNT_PBS_PATTERN                       2
#define COUNT_PBS_STARTOVER                     1
#define COUNT_PBS_REPEAT                        1
#define COUNT_PBS_COMP_RETRY_NUM                3
#define PBS_DIFF_LIMIT                          31
#define INIT_WL_DELAY                           15
#define INIT_RL_DELAY                           15
#define REG_PHY_DQS_REF_DLY_OFFS                10

/* phy internal registers */
#define PUP_RL_MODE                             0x2
#define PUP_WL_MODE                             0
#define PUP_PBS_TX                              0x10
#define PUP_PBS_RX                              0x30
#define PUP_DQS_RD_DESKEW                       0x38
#define PUP_DQS_WR                              0x1
#define PUP_DQS_RD                              0x3
#define PUP_DELAY_MASK                          0x1F

/***********/
/* Macros  */
/***********/
#define IS_PUP_ACTIVE(_uiData_, _uiPup_)        (((_uiData_) >> (_uiPup_)) & 0x1)

/******************************/
/* DRAM information structure */
/******************************/
typedef struct 
{
    GT_U32                  memDataWidth;        /* memory data width of the LU IF is 5 bits, FWD IF is 9 bits */
    GT_U32                  numOfIf;             /* 14 for LU IF, 8 for FWD IF */
    GT_U32                  ddrWidth;            /* 32/64 Bit or 16/32 Bit */
    MV_HWS_TARGET_FREQ      targetFreq;
    MV_HWS_MEMORY_CFG_INFO  vendorParam;
    GT_U32  auiWlValues[MAX_PUP_NUM][7];
    GT_U32  auiRlValues[MAX_PUP_NUM][7];

} MV_HWS_DRAM_INFO;

typedef struct
{
    GT_U8 tRas;
    GT_U8 tRcd;
    GT_U8 tRp;
    GT_U8 tWr;
    GT_U8 tWtr;
    GT_U8 tRasHigh;
    GT_U8 tRrd;
    GT_U8 tRtp;

}MV_HWS_DDR3_TIMING_LOW_PARAMS;

typedef struct
{ 
    GT_U8 tRFC; 
    GT_U8 tR2R;
    GT_U8 tR2W_W2R; 
    GT_U8 tW2W;
    GT_U8 tRFC_high;
    GT_U8 tR2R_high; 
    GT_U8 tR2W_W2R_high;
    GT_U8 tMOD;

}MV_HWS_DDR3_TIMING_HIGH_PARAMS;


typedef struct
{ 
    GT_U8 tPD;
    GT_U8 tPDLL;
    GT_U8 tWLMRD;
    GT_U8 tWLDELAY;

}MV_HWS_DDR3_TIMING_PARAMS;

typedef struct  
{
    GT_U8 tODT_ON_CTL_RD;
    GT_U8 tODT_OFF_CTL_RD;
    GT_U8 tODT_ON_CTL_WR;
    GT_U8 tODT_OFF_CTL_WR;
    
}MV_HWS_ODT_TIMING_PARAMS;

typedef enum
{
    MV_DDR3_RX,
    MV_DDR3_TX

}MV_DDR3_DIRECTION;

/* state machine for centralization - find low & high limit */
typedef enum {
    pupADllLimitsState_FAIL,
    pupADllLimitsState_PASS,
    pupADllLimitsState_FAIL_AFTER_PASS
} pupADllLimitsState;

/*******************************************************************************
* hwsDunitPhyRegWrite
*
* DESCRIPTION:
*       Indirect write to DDR3 PHY register.
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       emcBase   - EMC base address
*       pupNum    - PUP number
*       regNum    - register number
*       bcEnable  - enable / disable broadcast access tp PHY registers (to all PUPs)
*       isCntrlPup - is it control PUP access or data PUP
*       data      - value to write to register
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS hwsDunitPhyRegWrite
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_U32      emcBase,
    GT_U32      pupNum,
    GT_U32      regNum,
    GT_BOOL     bcEnable,
    GT_BOOL     isCntrlPup,
    GT_U32      data
);

/*******************************************************************************
* hwsDunitPhyRegUcWrite
*
* DESCRIPTION:
*       Indirect write to DDR3 PHY register.
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       emcBase   - EMC base address
*       memNum    - memory number
*       pupNum    - PUP number
*       regNum    - register number
*       bcEnable  - enable / disable broadcast access tp PHY registers (to all PUPs)
*       isCntrlPup - is it control PUP access or data PUP
*       data      - value to write to register
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS hwsDunitPhyRegUcWrite
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_U32      emcBase,
    GT_U32      memNum,
    GT_U32      pupNum,
    GT_U32      regNum,
    GT_BOOL     bcEnable,
    GT_BOOL     isCntrlPup,
    GT_U32      data
);

/*******************************************************************************
* hwsDunitPhyRegRead
*
* DESCRIPTION:
*       Indirect read from DDR3 PHY register.
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       emcBase   - EMC base address
*       pupNum    - PUP number
*       regNum    - register number
*       bcEnable  - enable / disable broadcast access tp PHY registers (to all PUPs)
*       isCntrlPup - is it control PUP access or data PUP
*
* OUTPUTS:
*       data      - value read from register
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS hwsDunitPhyRegRead
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_U32      emcBase,
    GT_U32      memNum,
    GT_U32      pupNum,
    GT_U32      regNum,
    GT_BOOL     isCntrlPup,
    GT_U32      *data
);

/*******************************************************************************
* mvHwsDdr3AdllControl
*
* DESCRIPTION:
*       Allow access (R/W) to the parallel interfaces PUP ADLL control
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       memoryType - memory type to init (FWR, LU)
*       memNum    - memory interface number
*       allMemIf  - enable / disable broadcast access to all interfaces
*       pupNum    - PUP number
*       allDataPups  - enable / disable broadcast access to PUPs
*       wrDqDelayAdll - write DQ delay
*       rdDqDelayAdll - read DQ delay
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS mvHwsDdr3AdllControlSet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    MV_HWS_EMC_TYPE         memType,
    GT_U32                  memNum,
    GT_BOOL                 allMemIf,
    GT_U32                  pupNum,
    GT_BOOL                 allDataPups,
    GT_U32                  wrDqDelayAdll,
    GT_U32                  rdDqDelayAdll
);

/*******************************************************************************
* mvHwsDdr3AdllControl
*
* DESCRIPTION:
*       Read parameters of parallel interfaces PUP ADLL control
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       memoryType - memory type to init (FWR, LU)
*       memNum    - memory interface number
*       pupNum    - PUP number
*
* OUTPUTS:
*       wrDqDelayAdll - write DQ delay
*       rdDqDelayAdll - read DQ delay
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS mvHwsDdr3AdllControlGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    MV_HWS_EMC_TYPE         memType,
    GT_U32                  memNum,
    GT_U32                  pupNum,
    GT_U32                  *wrDqDelayAdll,
    GT_U32                  *rdDqDelayAdll
);

/*******************************************************************************
* mvDunitAccessCfg
*
* DESCRIPTION:
*       Configure access to an appropriate DUNIT.
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       emcBase   - EMC base address
*       allMemIf  - enable / disable broadcast access to all interfaces
*       memNum    - memory interface number
*       memMask   - memory access mask
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS mvDunitAccessCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32      emcBase,
    GT_BOOL     allMemIf,
    GT_U32      memMask,
    GT_U32      memNum
);

/*******************************************************************************
* mvHwsDdr3GeneralConfig
*
* DESCRIPTION:
*       Init DDR3 interface with global configuration.
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       memType   - DDR3 interface type
*       freq      - memory target frequency
*       casWL     - CAS write latency
*       casL      - CAS read latency
*       size      - global memory size
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS mvHwsDdr3GeneralConfig
(
    GT_U8       devNum,
    GT_U32      portGroup,
    MV_HWS_EMC_TYPE         memType,
    MV_HWS_TARGET_FREQ      freq,
    MV_HWS_CWL              casWL,
    MV_HWS_CAS_L            casL,
    MV_HWS_DDR3_MEM_SIZE    size
);

#ifdef __cplusplus
}
#endif

#endif /* __mvHwDDR3PrvIf_H */
/*
 *------------------------------------------------------------------
 * $Log: mvDdr3PrvIf.h,v $
 * Revision 1.1  2015/02/13 11:34:24  iachang
 * Check in Marvell GE switch driver header file.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
