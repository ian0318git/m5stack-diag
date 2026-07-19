/* $Id: dev_cpss42.c,v 1.2 2021/09/24 01:22:18 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_98dxc25x_marvell/dev_cpss42.c,v $
 *------------------------------------------------------------------
 *
 * Filename:	dev_cpss42.c
 *
 * Description: Marvell 98dxc25x ESW Device Driver.
 *
 *------------------------------------------------------------------
 */

#include "dev_cpss42.h"

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



void DxChVersionGet (CPSS_VERSION_INFO_STC *cpssVersion)
{
    cpssDxChVersionGet(cpssVersion);
}

void LogEnableSet (void)
{
    cpssLogEnableSet(GT_TRUE);
}

int PpInit (void)
{
    return (cpssPpInit());
}

int DxChHwPpPhase1Init (CPSS_DXCH_PP_PHASE1_INIT_INFO_STC *xcat5_pp_phase1_info, 
                        CPSS_PP_DEVICE_TYPE *xcat5_pp_dev_type)
{
    return (cpssDxChHwPpPhase1Init(xcat5_pp_phase1_info, xcat5_pp_dev_type));
}

int DxChHwPpImplementWaInit (GT_U32 dev_num, CPSS_DXCH_IMPLEMENT_WA_ENT *waArr)
{
    return (cpssDxChHwPpImplementWaInit(dev_num, 1, waArr, NULL));
}

int DxChHwPpStartInit (GT_U32 dev_num, CPSS_REG_VALUE_INFO_STC *regCfgList, GT_U32 regCfgListSize)
{
    return (cpssDxChHwPpStartInit(dev_num, regCfgList, regCfgListSize));
}

int DxChHwPpPhase2Init (GT_U32 dev_num, CPSS_DXCH_PP_PHASE2_INIT_INFO_STC *xcat5_pp_phase2_info)
{
    return (cpssDxChHwPpPhase2Init(dev_num, xcat5_pp_phase2_info));
}

int DxChCfgPpLogicalInit (GT_U32 dev_num, CPSS_DXCH_PP_CONFIG_INIT_STC *ppConfig)
{
    return (cpssDxChCfgPpLogicalInit(dev_num, ppConfig));
}

int DxChBrgFdbInit (GT_U32 dev_num)
{
    return (cpssDxChBrgFdbInit(dev_num));
}

int DxChBrgFdbHashModeSet (GT_U32 dev_num)
{
    return (cpssDxChBrgFdbHashModeSet(dev_num, CPSS_MAC_HASH_FUNC_XOR_E));
}

int DxChBrgFdbMacVlanLookupModeSet (GT_U32 dev_num)
{
    return (cpssDxChBrgFdbMacVlanLookupModeSet(dev_num, CPSS_IVL_E));
}

int DxChBrgVlanBridgingModeSet (GT_U32 dev_num)
{
    return (cpssDxChBrgVlanBridgingModeSet(dev_num, CPSS_BRG_MODE_802_1Q_E));
}

int DxChPortStatInit (GT_U32 dev_num)
{
    return (cpssDxChPortStatInit(dev_num));
}

int DxChCfgDevEnable (GT_U32 dev_num)
{
    return (cpssDxChCfgDevEnable(dev_num, GT_TRUE));
}

int DxChDiagAllRegTest (uint cpss_dev, uint *testStatus, uint *badReg, uint *readVal, uint *writeVal)
{
    return (cpssDxChDiagAllRegTest(cpss_dev,
                                   testStatus,
                                   badReg,
                                   readVal,
                                   writeVal));
}

int DrvPpHwRegisterWrite (uint cpss_dev, int port_group, uint reg, uint data)
{
    return (cpssDrvPpHwRegisterWrite(cpss_dev, port_group, reg, data));
}

int DrvPpHwRegisterRead (uint cpss_dev, int port_group, uint reg, uint *data)
{
    return (cpssDrvPpHwRegisterRead(cpss_dev, port_group, reg, (GT_U32 *)data));
}


int DxChBrgPrvEdgeVlanEnable (uint cpss_dev)
{
    return (cpssDxChBrgPrvEdgeVlanEnable(cpss_dev, GT_TRUE));
}

int DxChBrgPrvEdgeVlanDisable (uint cpss_dev)
{
    return (cpssDxChBrgPrvEdgeVlanEnable(cpss_dev, GT_FALSE));
}

int DxChBrgPrvEdgeVlanPortEnable (uint cpss_dev, uint src_port, uint dst_port)
{
    return (cpssDxChBrgPrvEdgeVlanPortEnable(cpss_dev,
                                             src_port,
                                             GT_TRUE,
                                             dst_port,
                                             cpss_dev,
                                             GT_FALSE)); 
}

int DxChBrgPrvEdgeVlanPortDisable (uint cpss_dev, uint src_port, uint dst_port)
{
    return (cpssDxChBrgPrvEdgeVlanPortEnable(cpss_dev,
                                             src_port,
                                             GT_FALSE,
                                             dst_port,
                                             cpss_dev,
                                             GT_FALSE)); 
}

int DxChPortMacCountersOnPortGet (uint cpss_dev, uint port_num,
                                 CPSS_PORT_MAC_COUNTER_SET_STC *portMacCounterSetArray)
{
     return (cpssDxChPortMacCountersOnPortGet(cpss_dev, port_num, portMacCounterSetArray));
}

void DxChCfgDevRemove (uint cpss_dev)
{
    cpssDxChCfgDevRemove(cpss_dev);
}

int DxChPhyPortSmiRegisterWrite (uint cpss_dev, uint port, int reg, int value)
{
    return (cpssDxChPhyPortSmiRegisterWrite(cpss_dev, port, reg, value));
}


/*------------------------------------------------------------------
 *$Log: dev_cpss42.c,v $
 *Revision 1.2  2021/09/24 01:22:18  harrchan
 *Collapse Elixir-branch to Main Trunk.
 *
 *Revision 1.1.2.2  2021/04/23 02:48:13  illiu
 *Add function: DrvPpHwRegisterRead
 *
 *Revision 1.1.2.1  2021/04/12 08:57:38  illiu
 *Add file: Marvell library function
 *
 *
 *$Endlog$
*/


