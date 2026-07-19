/* $Id: dev_cpss42.h,v 1.2 2021/09/24 01:22:18 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_98dxc25x_marvell/dev_cpss42.h,v $
 *------------------------------------------------------------------
 * 
 * Filename   : dev_cpss42.h
 *
 * Description: Marvell 98dxc25x ESW device driver.
 *
 *------------------------------------------------------------------
 */

#ifndef __DEV_CPSS42_H__
#define __DEV_CPSS42_H__

/* workaround to avoid to redefine then cause build error. */
#undef BIT_0
#undef BIT_1
#undef BIT_2
#undef BIT_3
#undef BIT_4
#undef BIT_5
#undef BIT_6
#undef BIT_7
#undef BIT_8
#undef BIT_9
#undef BIT_10
#undef BIT_11
#undef BIT_12
#undef BIT_13
#undef BIT_14
#undef BIT_15
#undef BIT_16
#undef BIT_17
#undef BIT_18
#undef BIT_19
#undef BIT_20
#undef BIT_21
#undef BIT_22
#undef BIT_23
#undef BIT_24
#undef BIT_25
#undef BIT_26
#undef BIT_27
#undef BIT_28
#undef BIT_29
#undef BIT_30
#undef BIT_31

#include <generic/cpssTypes.h>
#include <generic/bridge/cpssGenBrgFdb.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgFdbHash.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgFdb.h>
#include <cpss/dxCh/dxChxGen/diag/cpssDxChDiag.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgStp.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgVlan.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>
#include <cpss/common/init/cpssInit.h>
#include <cpss/extServices/cpssExtServices.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/phy/cpssDxChPhySmi.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortBufMg.h>
#include <cpss/extServices/cpssExtServices.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortTx.h>
#include <cpss/dxCh/dxChxGen/config/cpssDxChCfgInit.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortStat.h>
#include <cpss/dxCh/dxChxGen/mirror/cpssDxChMirror.h>
#include <cpssDriver/pp/hardware/cpssDriverPpHw.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
#include <cpss/dxCh/dxChxGen/version/cpssDxChVersion.h>
#include <cpss/generic/version/cpssGenVersion.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/cpssDxChPortPizzaArbiterProfile.h>
#include <cpss/dxCh/dxChxGen/cscd/cpssDxChCscd.h>
#include <cpss/generic/cpssHwInit/cpssLedCtrl.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInitLedCtrl.h>
#include "cpss_extserv.h"
#include "cpss_common.h"
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgPrvEdgeVlan.h>
#include <cpss/generic/log/cpssLog.h>

#include "madApi.h"
#include "madHwCntl.h"

/* Dummy for competability, for init process. */
#define GT_DUMMY_REG_VAL_INFO_LIST          \
{                                           \
    {0x00000000, 0x00000000, 0x00000000, 0},\
    {0x00000001, 0x00000000, 0x00000000, 0},\
    {0x00000002, 0x00000000, 0x00000000, 0},\
    {0x00000003, 0x00000000, 0x00000000, 0},\
    {0x00000004, 0x00000000, 0x00000000, 0},\
    {0x00000005, 0x00000000, 0x00000000, 0},\
    {0x00000006, 0x00000000, 0x00000000, 0},\
    {0xFFFFFFFF, 0x00000000, 0x00000000, 0},    /* Delimiter */      \
    {0xFFFFFFFF, 0x00000000, 0x00000000, 0},    /* Delimiter */      \
    {0xFFFFFFFF, 0x00000000, 0x00000000, 0}     /* Delimiter */      \
}


void DxChVersionGet(CPSS_VERSION_INFO_STC *);
void LogEnableSet(void);
int PpInit(void);
int DxChHwPpPhase1Init(CPSS_DXCH_PP_PHASE1_INIT_INFO_STC *, CPSS_PP_DEVICE_TYPE *);
int DxChHwPpImplementWaInit(GT_U32, CPSS_DXCH_IMPLEMENT_WA_ENT *);
int DxChHwPpStartInit(GT_U32, CPSS_REG_VALUE_INFO_STC *, GT_U32);
int DxChHwPpPhase2Init(GT_U32, CPSS_DXCH_PP_PHASE2_INIT_INFO_STC *);
int DxChCfgPpLogicalInit(GT_U32, CPSS_DXCH_PP_CONFIG_INIT_STC *);
int DxChBrgFdbInit(GT_U32);
int DxChBrgFdbHashModeSet(GT_U32);
int DxChBrgFdbMacVlanLookupModeSet(GT_U32);
int DxChBrgVlanBridgingModeSet(GT_U32);
int DxChPortStatInit(GT_U32);
int DxChCfgDevEnable(GT_U32);
int DxChDiagAllRegTest(uint, uint *, uint *, uint *, uint *);
int DrvPpHwRegisterWrite(uint, int, uint, uint);
int DrvPpHwRegisterRead(uint, int, uint, uint *);
int DxChBrgPrvEdgeVlanEnable(uint);
int DxChBrgPrvEdgeVlanDisable(uint);
int DxChBrgPrvEdgeVlanPortEnable(uint, uint, uint);
int DxChBrgPrvEdgeVlanPortDisable(uint, uint, uint);
int DxChPortMacCountersOnPortGet(uint, uint, CPSS_PORT_MAC_COUNTER_SET_STC *);
void DxChCfgDevRemove(uint);
int DxChPhyPortSmiRegisterWrite(uint, uint, int, int);


#endif

/*------------------------------------------------------------------
 *$Log: dev_cpss42.h,v $
 *Revision 1.2  2021/09/24 01:22:18  harrchan
 *Collapse Elixir-branch to Main Trunk.
 *
 *Revision 1.1.2.2  2021/04/23 02:48:17  illiu
 *Add function: DrvPpHwRegisterRead
 *
 *Revision 1.1.2.1  2021/04/12 08:57:41  illiu
 *Add file: Marvell library function
 *
 *
 *$Endlog$
*/

