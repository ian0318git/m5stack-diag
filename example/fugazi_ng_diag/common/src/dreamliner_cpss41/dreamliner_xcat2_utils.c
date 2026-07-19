/* $Id: dreamliner_xcat2_utils.c,v 1.2 2019/12/11 10:10:26 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner_cpss41/dreamliner_xcat2_utils.c,v $
 *------------------------------------------------------------------
 *
 * dreamliner_xcat2_init.c - This file contains functions to init CPSS driver
 *                          for Marvell GE switch.
 *
 * Christine Wen -- Jan. 2014
 *
 * Copyright (c) 2014-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
/* List of include files */
#include "common.h"
#include "types.h"

/* workaround to avoid build error. BIT_0 to BIT_31 are defined in common.h.
   And Marvell code defines them again.
*/
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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <generic/cpssTypes.h>
#include <generic/bridge/cpssGenBrgFdb.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgFdbHash.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgFdb.h>
#include <cpss/dxCh/dxChxGen/diag/cpssDxChDiag.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgStp.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgVlan.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>
/* Modified for CPSS 4.1 */
//#include <cpss/generic/init/cpssInit.h>
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
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgPrvEdgeVlan.h>

#include "dreamliner_ge_switch.h"


static uint32_t 
xcat2_global_enable_pve (uint8_t dev_num)
{
    uint32_t rc = 0;

    rc  = cpssDxChBrgPrvEdgeVlanEnable(dev_num, GT_TRUE);
    if (rc) {
        cterr('f',0,"Error from cpssDxChBrgPrvEdgeVlanEnable(), rc = 0x%x\n", rc);
    }
    
    return rc;
} 

static uint32_t 
xcat2_global_disable_pve (uint8_t dev_num)
{
    uint32_t rc = 0;

    rc  = cpssDxChBrgPrvEdgeVlanEnable(dev_num, GT_FALSE);
    if (rc) {
        cterr('f',0,"Error from cpssDxChBrgPrvEdgeVlanEnable(), rc = 0x%x\n", rc);
    }
    
    return rc;
} 

static uint32_t 
xcat2_set_port_pve (uint8_t dev_num, uint8_t src_port, uint8_t dst_port)
{
    uint32_t rc = 0;

    rc = cpssDxChBrgPrvEdgeVlanPortEnable(dev_num,
                                          src_port,
                                          GT_TRUE,
                                          dst_port,
                                          dev_num,
                                          GT_FALSE);
    if (rc) {
        cterr('f',0,"Error from cpssDxChBrgPrvEdgeVlanPortEnable(), rc = 0x%x\n", rc);
        return rc;
    }
    rc = cpssDxChBrgPrvEdgeVlanPortEnable(dev_num,
                                          dst_port,
                                          GT_TRUE,
                                          src_port,
                                          dev_num,
                                          GT_FALSE);
    if (rc) {
        cterr('f',0,"Error from cpssDxChBrgPrvEdgeVlanPortEnable(), rc = 0x%x\n", rc);
        return rc;
    }
    return rc; 
}

static uint32_t 
xcat2_clear_port_pve (uint8_t dev_num, uint8_t src_port, uint8_t dst_port)
{
    uint32_t rc = 0;
     
    rc = cpssDxChBrgPrvEdgeVlanPortEnable(dev_num,
                                          src_port,
                                          GT_FALSE,
                                          dst_port,
                                          dev_num,
                                          GT_FALSE);
    if (rc) {
        cterr('f',0,"Error from cpssDxChBrgPrvEdgeVlanPortEnable(), rc = 0x%x\n", rc);
        return rc;
    }
    rc = cpssDxChBrgPrvEdgeVlanPortEnable(dev_num,
                                          dst_port,
                                          GT_FALSE,
                                          src_port,
                                          dev_num,
                                          GT_FALSE);
    if (rc) {
	cterr('f',0,"Error from cpssDxChBrgPrvEdgeVlanPortEnable(), rc = 0x%x\n", rc);
        return rc;
    }

    return rc; 
}

uint32_t 
xcat2_port_enable (uint32_t dev_num, uint32_t port_num)
{
    uint32_t rc = GT_OK;

    rc = cpssDxChPortEnableSet(dev_num, port_num, 1);
    if (rc != GT_OK) {
        cterr('f',0,"Error failed cpssDxChPortEnableSet \n");
    }
    return (rc);
}

uint32_t 
xcat2_port_disable (uint32_t dev_num, uint32_t port_num)
{
    uint32_t rc = GT_OK;

    rc = cpssDxChPortEnableSet(dev_num, port_num, 0);
    if (rc != GT_OK) {
        cterr('f',0,"Error failed cpssDxChPortEnableSet");
    }
    return (rc);
}

uint32_t 
xcat2_config_port_pve (uint8_t dev_num, uint8_t src_port, uint8_t dst_port)
{
    uint32_t rc = 0;

    rc = xcat2_global_enable_pve(dev_num);
    if (rc) {
        cterr('f',0,"Failed to enable global PVE, rc = %#x\n", rc);
        return rc;
    }

    rc = xcat2_port_enable(dev_num, src_port);
    if (rc) {
        cterr('f',0,"Failed to enable src_port &d\n", src_port);
        return rc;
    }

    rc = xcat2_port_enable(dev_num, dst_port);
    if (rc) {
        cterr('f',0,"Failed to enable dst_port &d\n", dst_port);
        return rc;
    }

    rc = xcat2_set_port_pve(dev_num, src_port, dst_port);
    if (rc) {
        cterr('f',0,"Failed to set PVE for ports %d and %d.\n", src_port, dst_port);
        return rc;
    }
    return PASSED;
}

uint32_t 
xcat2_unconfig_port_pve (uint8_t dev_num, uint8_t src_port, uint8_t dst_port)
{
    uint32_t rc = 0;

    rc = xcat2_global_disable_pve(dev_num);
    if (rc) {
        cterr('f',0,"Failed to disable global PVE, rc = %#x\n", rc);
        return rc;
    }

    rc = xcat2_clear_port_pve(dev_num, src_port, dst_port);
    if (rc) {
        cterr('f',0,"Failed to clear PVE for ports %d and %d.\n", src_port, dst_port);
        return rc;
    }
    return PASSED;
}


uint32_t 
xcat2_port_mac_loopback_enable (uint32_t dev_num, uint32_t port_num)
{
    uint32_t rc = GT_OK;

    rc = cpssDxChPortInternalLoopbackEnableSet(dev_num, port_num, GT_TRUE);
    if (rc != GT_OK) {
        cterr('f',0,"Enable port MAC loopback failed, rc 0x%08x\n", rc);
    }
#ifdef DEBUG
    uint32_t data;
    rc = xcat2_reg_pci_read(PORT_MAC_CTRL_REG+PORT_MAC_OFFSET*port_num, &data);
    if (rc != GT_OK) {
        cterr('f',0,"Read port MAC control register failed, rc 0x%08x\n", rc);
	return (FAILED);
    }
    data |= PORT_PCS_LPBK_EN;
    rc = xcat2_reg_pci_write(PORT_MAC_CTRL_REG+PORT_MAC_OFFSET*port_num, data);
    if (rc != GT_OK) {
        cterr('f',0,"Write port MAC control register failed, rc 0x%08x\n", rc);
    }
#endif
    return (rc);
}


uint32_t 
xcat2_port_mac_loopback_disable (uint32_t dev_num, uint32_t port_num)
{
    uint32_t rc = GT_OK;

    rc = cpssDxChPortInternalLoopbackEnableSet(dev_num, port_num, GT_FALSE);
    if (rc != GT_OK) {
        cterr('f',0,"Disable port MAC loopback failed, rc 0x%08x\n", rc);
    }
#ifdef DEBUG
    uint32_t data;
    rc = xcat2_reg_pci_read(PORT_MAC_CTRL_REG+PORT_MAC_OFFSET*port_num, &data);
    if (rc != GT_OK) {
        cterr('f',0,"Read port MAC control register failed, rc 0x%08x\n", rc);
	return (FAILED);
    }
    data &= ~PORT_PCS_LPBK_EN;
    rc = xcat2_reg_pci_write(PORT_MAC_CTRL_REG+PORT_MAC_OFFSET*port_num, data);
    if (rc != GT_OK) {
        cterr('f',0,"Write port MAC control register failed, rc 0x%08x\n", rc);
    }
#endif
    return (rc);
}

/******************************************************************** 
* Function:  xcat2_soft_reset
*
* Input:     dev_num  - device number. Number in range 0..31
*
* Output:
*       GT_OK   - on success
*       GT_FAIL - on error
* Description  : Soft reset Marvell switch
*  
* Returns:  PASSED/FAILED
*
********************************************************************/
uint32_t 
xcat2_soft_reset (uint32_t dev_num)
{
    uint32_t rc = GT_OK;

    printf("configure skip parameters related to soft reset\n");
    rc = cpssDxChHwPpSoftResetSkipParamSet(dev_num, 
                                           CPSS_HW_PP_RESET_SKIP_TYPE_REGISTER_E,
                                           GT_FALSE);
    rc = cpssDxChHwPpSoftResetSkipParamSet(dev_num, 
                                           CPSS_HW_PP_RESET_SKIP_TYPE_TABLE_E,
                                           GT_FALSE);
    rc = cpssDxChHwPpSoftResetSkipParamSet(dev_num, 
                                           CPSS_HW_PP_RESET_SKIP_TYPE_EEPROM_E,
                                           GT_TRUE);
    rc = cpssDxChHwPpSoftResetSkipParamSet(dev_num, 
                                           CPSS_HW_PP_RESET_SKIP_TYPE_PEX_E,
                                           GT_TRUE);

    printf("Calling cpssDxChHwPpSoftResetTrigger\n");
    rc = cpssDxChHwPpSoftResetTrigger(dev_num);
    if (rc != GT_OK) {
        cterr('f',0,"Error failed cpssDxChHwPpSoftResetTrigger Error code "
               "0x%x\n", rc);
    }
    printf("cpssDxChHwPpSoftResetTrigger done!\n");

    return (rc);
}


uint32_t 
xcat2_enable_port_interrupt (uint32_t port_num)
{
    uint32_t port_irupt_mask_reg;
   
    port_irupt_mask_reg = PORT_0_IRUPT_MASK_REGISTER +
	                  PORT_IRUPT_OFFSET * port_num;
    return (xcat2_reg_pci_write(port_irupt_mask_reg, 0xFFFFFFFF));
}

uint32_t 
xcat2_disable_port_interrupt (uint32_t port_num)
{
    uint32_t port_irupt_mask_reg;
   
    port_irupt_mask_reg = PORT_0_IRUPT_MASK_REGISTER +
	                  PORT_IRUPT_OFFSET * port_num;
    return (xcat2_reg_pci_write(port_irupt_mask_reg, 0x0));
}


uint32_t 
xcat2_clear_port_interrupt (uint32_t port_num)
{
    uint32_t port_irupt_cause_reg;
    uint32_t value;
   
    port_irupt_cause_reg = PORT_0_IRUPT_CAUSE_REGISTER +
	                  PORT_IRUPT_OFFSET * port_num;
    return (xcat2_reg_pci_read(port_irupt_cause_reg, &value));
}


uint32_t 
xcat2_clear_all_port_interrupt (void)
{
    uint32_t port_num;
    
    for (port_num = 0; port_num < XCAT2_PORT_NUM; port_num ++) {
	if (xcat2_clear_port_interrupt(port_num) != GT_OK) {
	    cterr('f',0,"Failed to clear interrupt for port %d", port_num);
	    return (FAILED);
	}
    }
    
    return PASSED;
}

uint32_t 
xcat2_vlan_add (uint32_t dev_num, uint32_t vlan_id)
{
    uint32_t rc = GT_OK;
    uint32_t i;
    CPSS_PORTS_BMP_STC ports_members = {{0}};
    CPSS_PORTS_BMP_STC ports_tagging = {{0}};
    CPSS_DXCH_BRG_VLAN_INFO_STC  vlan_info = {0};
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC ports_tagging_cmd = {{0}};

    for (i = 0; i < 8; i++) {
	ports_members.ports[i] = 0;
	ports_tagging.ports[i] = 0;
    }

    for (i = 0; i < 256; i++) {
	ports_tagging_cmd.portsCmd[i] = CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E;
    }

    vlan_info.stgId = 0x0;
    vlan_info.vrfId = 0x0;
    vlan_info.ucastLocalSwitchingEn = GT_FALSE;
    vlan_info.ipv4IpmBrgEn = GT_FALSE;
    vlan_info.unregIpv4BcastCmd = CPSS_PACKET_CMD_FORWARD_E;
    vlan_info.ipv6UcastRouteEn = GT_FALSE;
    vlan_info.unregNonIpv4BcastCmd = CPSS_PACKET_CMD_FORWARD_E;
    vlan_info.ipv6McastRouteEn = GT_FALSE;
    vlan_info.unregNonIpMcastCmd = CPSS_PACKET_CMD_FORWARD_E;
    vlan_info.mruIdx = 0x0;
    vlan_info.bcastUdpTrapMirrEn = GT_FALSE;
    vlan_info.mirrToRxAnalyzerEn = GT_FALSE;
    vlan_info.ipv6IcmpToCpuEn = GT_FALSE;
    vlan_info.ipv4UcastRouteEn = GT_FALSE;
    vlan_info.ipv4McBcMirrToAnalyzerIndex = 0x0;
    vlan_info.floodVidxMode = CPSS_DXCH_BRG_VLAN_FLOOD_VIDX_MODE_ALL_FLOODED_TRAFFIC_E;
    vlan_info.portIsolationMode = CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_DISABLE_CMD_E;
    vlan_info.ipv4McastRouteEn = GT_FALSE;
    vlan_info.ipv6McMirrToAnalyzerIndex = 0x0;
    vlan_info.floodVidx = 0xFFF;
    vlan_info.ipv4IpmBrgMode = CPSS_BRG_IPM_SGV_E;
    vlan_info.ipv6IpmBrgEn = GT_FALSE;
    vlan_info.unkUcastCmd = CPSS_PACKET_CMD_FORWARD_E;
    vlan_info.mirrToTxAnalyzerEn = GT_FALSE;
    vlan_info.ipv6McMirrToAnalyzerEn = GT_FALSE;
    vlan_info.ipv4McBcMirrToAnalyzerEn = GT_FALSE;
    vlan_info.unregIpv6McastCmd = CPSS_PACKET_CMD_FORWARD_E;
    vlan_info.ipv6SiteIdMode = CPSS_IP_SITE_ID_INTERNAL_E;
    vlan_info.unknownMacSaCmd = CPSS_PACKET_CMD_FORWARD_E;
    vlan_info.unkSrcAddrSecBreach = GT_FALSE;
    vlan_info.ipCtrlToCpuEn = CPSS_DXCH_BRG_IP_CTRL_NONE_E;
    vlan_info.fidValue = 0x5;
    vlan_info.mirrToTxAnalyzerIndex = 0x0;
    vlan_info.mirrToRxAnalyzerIndex = 0x0;
    vlan_info.ipv6IpmBrgMode = CPSS_BRG_IPM_SGV_E;
    vlan_info.naMsgToCpuEn = GT_FALSE;
    vlan_info.mcastLocalSwitchingEn = GT_FALSE;
    vlan_info.ipv4IgmpToCpuEn = GT_FALSE;
    vlan_info.unregIpv4McastCmd = CPSS_PACKET_CMD_FORWARD_E;
    /* Disable auto learn on VLAN */
    vlan_info.autoLearnDisable = GT_TRUE;

    rc = cpssDxChBrgVlanEntryWrite(dev_num,
                                   vlan_id,
                                   &ports_members,
                                   &ports_tagging,
                                   &vlan_info,
                                   &ports_tagging_cmd);
    if (rc != GT_OK) {
        printf("Error failed cpssDxChBrgVlanEntryWrite call, rc = %#x\n", rc);
    }
    return (rc);
}

uint32_t 
xcat2_vlan_port_add (uint32_t dev_num, uint32_t vlan_id, uint32_t port_num)
{
    uint32_t rc = GT_OK;

    rc = cpssDxChBrgVlanMemberAdd(dev_num, vlan_id, port_num, 
                      GT_FALSE, CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E);
    if (rc != GT_OK) {
        printf("Error failed cpssDxChBrgVlanMemberAdd call, rc = %#x\n", rc);
	return (FAILED);
    }

    /* Modified for CPSS 4.1 */
    //rc = cpssDxChBrgVlanPortVidSet(dev_num, port_num, vlan_id);
    rc = cpssDxChBrgVlanPortVidSet(dev_num, port_num, CPSS_DIRECTION_INGRESS_E, vlan_id);
    if (rc != GT_OK) {
        printf("Error failed cpssDxChBrgVlanPortVidSet call, rc = %#x\n", rc);
    }

    return (rc);
}


uint32_t 
xcat2_vlan_port_del (uint32_t dev_num, uint32_t vlan_id, uint32_t port_num)
{
    uint32_t rc = GT_OK;

    rc = cpssDxChBrgVlanPortDelete(dev_num, vlan_id, port_num);
    if (rc != GT_OK) {
        cterr('f',0,"Error failed cpssDxChBrgVlanPortDelete call, rc = %#x\n", rc);
    }
    return (rc);
}

uint32_t
xcat2_vlan_port_show (uint32_t dev_num, uint32_t port_num)
{
    uint32_t rc = GT_OK;
    GT_U16 vlan_id; 

    /* Modified for CPSS 4.1 */
    //rc = cpssDxChBrgVlanPortVidGet(dev_num, port_num, &vlan_id);
    rc = cpssDxChBrgVlanPortVidGet(dev_num, port_num, CPSS_DIRECTION_INGRESS_E,  &vlan_id);
    if (rc != GT_OK) {
        cterr('f',0,"Error failed cpssDxChBrgVlanPortVidGet call, rc = %#x\n", rc);
        rc = 0xFF; 
        return (rc);
    }
    rc = vlan_id; 
    return (rc);
}


/*
 *------------------------------------------------------------------
 * $Log: dreamliner_xcat2_utils.c,v $
 * Revision 1.2  2019/12/11 10:10:26  lucywang
 * Merged Nanook to main trunk
 *
 * 
 *------------------------------------------------------------------
 * $Endlog$
 */

