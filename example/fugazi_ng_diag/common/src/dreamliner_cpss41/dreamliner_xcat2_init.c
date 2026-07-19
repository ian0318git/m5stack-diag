/* $Id: dreamliner_xcat2_init.c,v 1.2 2019/12/11 10:10:26 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner_cpss41/dreamliner_xcat2_init.c,v $
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
#include <cpss/generic/version/cpssGenStream.h>
#include <cpss/common/init/cpssInit.h>
//#include <cpss/generic/init/cpssInit.h>
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

#include "dreamliner.h"
#include "dreamliner_ge_switch.h"
#include "nim_dm_cpss_extserv.h"

#if defined(TACHI_INTEL) || defined(YWEN)
static int del_all_vlan(int, int);
extern GT_STATUS cpssDxChBrgVlanEgressFilteringEnable
(
    IN GT_U8    dev,
    IN GT_BOOL  enable
);
#endif

extern GT_STATUS cpssDxChPortPcsResetSet
(
    IN  GT_U8                          devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    IN  CPSS_PORT_PCS_RESET_MODE_ENT   mode,
    IN  GT_BOOL                        state
);

extern GT_STATUS hwPpStartInit
(
    IN  GT_U8                       devNum,
    IN  CPSS_REG_VALUE_INFO_STC     *initDataListPtr,
    IN  GT_U32                      initDataListLen
);

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

int marvell_cpssPpInit = FALSE;

static CPSS_PP_DEVICE_TYPE xcat2_pp_dev_type;
static CPSS_DXCH_PP_PHASE1_INIT_INFO_STC xcat2_pp_phase1_info;
static CPSS_DXCH_PP_PHASE2_INIT_INFO_STC xcat2_pp_phase2_info;
static CPSS_REG_VALUE_INFO_STC dummyRegValInfoList[] = GT_DUMMY_REG_VAL_INFO_LIST;
/*
 **********************************************************************
 *
 *  Function: xcat2_cpss_pp_phase1_info_init
 *
 *  Description: Xcat2 initial phase 1
 *
 *  Input: None
 *
 *  Outputs: GT_OK - on success
 *       
 **********************************************************************
 */
static uint32_t 
xcat2_cpss_pp_phase1_info_init ()
{
    CPSS_DXCH_PP_PHASE1_INIT_INFO_STC *info;
    nim_dm_cpss_bind_func bind_func;
    GT_VOID *int_vect;
    /* Modified for CPSS 4.1 */
    GT_UINTPTR int_mask;
    //GT_U32 int_mask;
    GT_UINTPTR pci_base, internal_base;

    /* since we use MSI interrupt, PCI interrupt line number is a dummy here */
    if (extDrvGetPciIntVec(0, (void **)&int_vect) != GT_OK) {
	cterr('f',0,"Failed to get interrupt vector number.");
	return FAILED;
    }

    if (extDrvGetIntMask(0, &int_mask) != GT_OK) {
	cterr('f',0,"Failed to get interrupt mask.");
	return FAILED;
    }
#ifdef DEBUG
    printf("int_vect = %lu, int_mask = %u\n", (unsigned long)int_vect, int_mask);
#endif
    nim_dm_cpss_get_pciemap(&pci_base, &internal_base);
#ifdef DEBUG
    printf("pci_base = %lx, internal_base = %lx\n", pci_base, internal_base);
#endif
    nim_dm_cpss_get_extserv(&bind_func.extDrv, &bind_func.os, &bind_func.trace);
    if (cpssExtServicesBind(&bind_func.extDrv, &bind_func.os, &bind_func.trace) != GT_OK) {
	cterr('f',0,"Failed to do cpssExtServicesBind()");
	return (FAILED);
    }

    info = &xcat2_pp_phase1_info;

    memset(info, 0, sizeof(*info));

        /* devNum -          Temporary device number to allow basic
                             communication.  number in range 0..31 */
    info->devNum = xcat2_dev_num[get_slot_num()-1];

    /* All PCIe base addresses and internal base addresses should be 
     * enumerated already.
     *
     * PEX Internal Base Addr uses BAR 0
     * PEX          Base Addr uses BAR 1
     *
     * busBaseAddr -     The unique bus Base address (PCI/PEX base
                         address /SMI id select / TWSI id select...)
                         that the device connected to on the management
                         interface bus (PCI/PEX/SMI/TWSI).
                         NOTE: Use CPSS_PARAM_NOT_USED_CNS for
                               'multi port groups' device to
                               indicate to use info from array of
                               multiPortGroupsInfoPtr 
    */
    info->busBaseAddr = pci_base;

        /* internalPciBase - Base address to which the internal PCI
                             registers are mapped to. Relevant only
                             when mngInterfaceType = CPSS_CHANNEL_PCI_E */
    info->internalPciBase = internal_base;

        /* intVecNum - The interrupt vector number this PP is connected to */
    info->intVecNum = (unsigned long)int_vect;

        /* intMask   - The interrupt mask to enable PP interrupts.         */
    info->intMask = int_mask;

    /* set the clock to the recommended value which is 167MHz */
        /* coreClock - The PP core clock in MHz.                         */
    info->coreClock = CPSS_DXCH_AUTO_DETECT_CORE_CLOCK_CNS;

        /* mngInterfaceType - Management interface type (PCI/SMI/TWSI/PEX).*/
    info->mngInterfaceType = CPSS_CHANNEL_PEX_E;
    
        /* ppHAState - CPU High Availability mode(Active or Standby). */
    info->ppHAState = CPSS_SYS_HA_MODE_ACTIVE_E;

       /* powerDownPortsBmp - The bitmap of ports those SERDES power status
                 is not changed during cpssDxChHwPpPhase1Init.
                 If port's bit is 0 then cpssDxChHwPpPhase1Init
                 changes power state of the port's SERDES to be UP.
                 If port's bit is 1 then cpssDxChHwPpPhase1Init
                 does not changes power state of the port's
                 SERDES and it stay to be DOWN.*/
    info->powerDownPortsBmp.ports[0] = 0;

        /* serdesRefClock   - SERDES reference clock type.*/
    //info->serdesRefClock = CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_125_SINGLE_ENDED_E;
    info->serdesRefClock = CPSS_DXCH_PP_SERDES_REF_CLOCK_INTERNAL_125_E;
        /* initSerdesDefaults - GT_TRUE - cpss performs SERDES initialization.
                                GT_FALSE- cpss doesn't perform SERDES
                                          initialization.*/
    info->initSerdesDefaults = GT_TRUE;

        /* isExternalCpuConnected - GT_TRUE - External CPU is connected
                                    to the PP.*/
    info->isExternalCpuConnected = GT_TRUE;

        
        /* numOfPortGroups   - number of port groups that the device support,
                               used to know the number of elements in the
                               array of multiPortGroupsInfoPtr.
                               NOTE: relevant only when busBaseAddr equal to
                                     CPSS_PARAM_NOT_USED_CNS relevant to
                                     'multi-port-groups' device.*/
    info->numOfPortGroups = 0;

    return (GT_OK);
}


/*
 **********************************************************************
 *
 *  Function: xcat2_cpss_pp_phase2_info_init
 *
 *  Description: Xcat2 initial phase 2
 *
 *  Input: None
 *
 *  Outputs: GT_OK - on success
 *       
 **********************************************************************
 */
static uint32_t 
xcat2_cpss_pp_phase2_info_init ()
{
    CPSS_DXCH_PP_PHASE2_INIT_INFO_STC *info;
    int tc = 0;

    info = &xcat2_pp_phase2_info;

    memset(info, 0, sizeof(*info));

    info->newDevNum = xcat2_dev_num[get_slot_num()-1];

    info->fuqUseSeparate = FALSE;

    info->auqCfg.auDescBlockSize = 0;
    info->auqCfg.auDescBlock = NULL;
    info->fuqCfg.auDescBlockSize = 0;
    info->fuqCfg.auDescBlock = NULL;

    info->netIfCfg.txDescBlockSize = 0;
    info->netIfCfg.txDescBlock = NULL;

    info->netIfCfg.rxDescBlockSize = 0;
    info->netIfCfg.rxDescBlock = NULL;
    info->netIfCfg.rxBufInfo.allocMethod = CPSS_RX_BUFF_STATIC_ALLOC_E;
    info->netIfCfg.rxBufInfo.buffData.staticAlloc.rxBufBlockSize = 0;
    info->netIfCfg.rxBufInfo.buffData.staticAlloc.rxBufBlockPtr = NULL;

    for (tc = 0; tc < (CPSS_MAX_RX_QUEUE_CNS - 1); tc++) {
        info->netIfCfg.rxBufInfo.bufferPercentage[tc] = 0;
    }

    info->netIfCfg.rxBufInfo.bufferPercentage[tc] = 0;

    /* HeaderOffset- The number of bytes at the beginning of the buffer reserved
     * for the application's use before the start of the received packet */
    info->netIfCfg.rxBufInfo.headerOffset = 0;

    /* RxBufSize- The SDMA Rx data buffer size to be used in the CPU. If the 
     * received packet size is greater than the buffer size, the packet is broken
     * and chained to multiple buffers */
    info->netIfCfg.rxBufInfo.rxBufSize = 0;

    return (GT_OK);

}

/*
 **********************************************************************
 *
 *  Function: xcat2_port_init
 *
 *  Description: Xcat2 port initial
 *
 *  Input: dev_num - device number
 *         port_num - port number
 *  Outputs  : PASSED/FAILED
 *       
 **********************************************************************
 */
static uint32_t 
xcat2_port_init (uint32_t dev_num, uint32_t port_num)
{
    uint32_t rc = GT_OK;
    uint32_t mruSize;
    CPSS_PORT_SPEED_ENT speed;
    uint32_t port_mode;

    /*********************************************************/
    /* Port Configuration                                    */
    /*********************************************************/
    mruSize = 1522; /* default */

    if (port_num >= GE0_XCAT2_PORT)
	port_mode = CPSS_PORT_INTERFACE_MODE_1000BASE_X_E;
    else
	port_mode = CPSS_PORT_INTERFACE_MODE_QSGMII_E;

    speed = CPSS_PORT_SPEED_1000_E;

    rc = cpssDxChPortInterfaceModeSet(dev_num, port_num, port_mode);
    if(rc != GT_OK) {
	cterr('f',0,"Failed to call cpssDxChPortInterfaceModeSet() for port %d,"
	      " err code %d", port_num, rc);
	return rc;
    }

    if (port_force_link_set(LINK_DOWN, port_num, TRUE) != OK) {
        cterr('f',0,"Failed port_force_link_set()");
        return (FAILED);
    }

    rc = cpssDxChPortDuplexAutoNegEnableSet(dev_num, port_num, GT_FALSE);
    if (rc != GT_OK) {
	cterr('f',0,"Failed to call cpssDxChPortDuplexAutoNegEnableSet() "
	      "for port %d, err code %d", port_num, rc);
	return rc;
    }

#ifdef TACHI_INTEL
    int ix;
    rc = cpssDxChPortSpeedAutoNegEnableSet(dev_num, port_num, GT_FALSE);
    /********** Create Vlans: VLAN_1 to VLAN_5 */
    for(ix = 1; ix < 6; ix++) {
        rc = xcat2_vlan_add(dev_num, ix) ;
        if(rc != GT_OK) {
            cterr('f',0,"Failed to add Vlan %d.",ix);
            return (rc);
        }
    }

    /* Delet all ports VLAN setting */
    for (ix = 0; ix < (XCAT2_USED_PORT); ix++) {
        del_all_vlan(dev_num, ix);
    }

    del_all_vlan(dev_num, GE0_XCAT2_PORT);
    del_all_vlan(dev_num, GE1_XCAT2_PORT);

#else 
    rc = cpssDxChPortSpeedAutoNegEnableSet(dev_num, port_num, GT_FALSE);
#endif
    if (rc != GT_OK) {
	cterr('f',0,"Failed to call cpssDxChPortSpeedAutoNegEnableSet() "
	      "for port %d, err code %d", port_num, rc);
	return rc;
    }

    rc = cpssDxChPortSpeedSet(dev_num, port_num, speed);
    if(rc != GT_OK) {
	cterr('f',0,"Failed to call cpssDxChPortSpeedSet() for port %d,"
	      " err code %d", port_num, rc);
	return rc;
    }


    rc = cpssDxChPortDuplexModeSet(dev_num, port_num, CPSS_PORT_FULL_DUPLEX_E);
    if (rc != GT_OK) {
	cterr('f',0,"Failed to call cpssDxChPortDuplexModeSet() for port %d,"
	      " err code %d", port_num, rc);
	return rc;
    }

    rc = cpssDxChPortSerdesPowerStatusSet(dev_num,port_num,
					  CPSS_PORT_DIRECTION_BOTH_E,0x8,GT_TRUE);
    if( rc != GT_OK ) {
	cterr('f',0,"Failed cpssDxChPortSerdesPowerStatusSet, rc = %#x", rc);
	return rc;
    }

    rc = cpssDxChPortMruSet(dev_num, port_num, mruSize);
    if (rc != GT_OK) {
	cterr('f',0,"Failed to call cpssDxChPortMruSet() "
	      "for port %d, err code %d", port_num, rc);
	return rc;
    }

    rc = cpssDxChPortMacCountersEnable(dev_num, port_num, GT_TRUE);
    if (rc != GT_OK) {
        cterr('f',0,"failed to enable Port counters, err code %d\n", rc);
        return (rc);
    }

    rc = cpssDxChPortMacCountersClearOnReadSet(dev_num, port_num, GT_TRUE);
    if (rc != GT_OK) {
        cterr('f',0,"failed to clear counters, err code %d\n", rc);
        return (rc);
    }

#ifdef TACHI_INTEL
    /* remove it */
#else
    rc = cpssDxChBrgVlanPortIngFltEnable(dev_num, port_num, GT_TRUE);
    if (rc) {
	cterr('f',0,"cpssDxChBrgVlanPortIngFltEnable failed\n");
	return (rc);
    }
#endif

    if (port_force_link_set(LINK_DOWN, port_num, FALSE) != OK) {
        cterr('f',0,"Failed port_force_link_set()");
        return (FAILED);
    }

    return (rc);
}

/*
 **********************************************************************
 *
 *  Function: xcat2_specific_port_init
 *
 *  Description: Xcat2 specific port initial
 *
 *  Input: None
 *  Outputs  : PASSED/FAILED
 *       
 **********************************************************************
 */
static uint32_t 
xcat2_specific_port_init (void)
{
    uint32_t rc = GT_OK;
    GT_U32 port;
    GT_U32 dev_num = xcat2_dev_num[get_slot_num()-1];

    /* Port initialization */
    for (port = 0; port < XCAT2_USED_PORT; port++) {
	rc = xcat2_port_init(dev_num, port);
	if (rc != GT_OK) {
	    cterr('f',0," Failed xcat2_port_init err code 0x%0x,"
		   " dev_num %d, port %d\n", rc, dev_num, port);
	    return (rc);
	}
    }

    for (port = GE0_XCAT2_PORT; port <= GE1_XCAT2_PORT; port++) {
	/* for flexlink ports 24 and 25, the mode needs to be set prior 
	   setting interface speed. */
	/* set port24/25 to SGMII mode */
	rc = xcat2_port_init(dev_num, port);
	if (rc != GT_OK) {
	    cterr('f',0," Failed xcat2_port_init err code 0x%0x,"
		   " dev_num %d, port %d\n", rc, dev_num, port);
	    return (rc);
	}
    }

    /* put used ports in reset */
    for (port = 0; port < XCAT2_USED_PORT; port++) {
	rc = cpssDxChPortEnableSet(dev_num, port, GT_FALSE);
	if (rc != GT_OK) {
	    cterr('f', 0," Error failed port disable:cpssDxChPortEnableSet err code 0x%0x,"
		   " dev_num %d, port %d\n", rc, dev_num, port);
	    return (rc);
	}

	rc = cpssDxChPortMacResetStateSet(dev_num, port, GT_TRUE);
	if (rc != GT_OK) {
	    cterr('f',0," Error failed port reset:cpssDxChPortMacResetStateSet err 0x%0x,"
		   " dev_num %d, port %d\n", rc, dev_num, port);
	    return (rc);
	}
    }

    /* unreset the ports */
    for (port = 0; port < XCAT2_USED_PORT; port++) {
	rc = cpssDxChPortMacResetStateSet(dev_num, port, GT_FALSE);
	if (rc != GT_OK) {
	    cterr('f',0," Error failed port unreset:cpssDxChPortMacResetStateSet err 0x%0x,"
		   " dev_num %d, port %d\n", rc, dev_num, port);
	    return (rc);
	}

	rc = cpssDxChPortEnableSet(dev_num, port, GT_TRUE);
	if (rc != GT_OK) {
	    cterr('f',0," Error failed port enable:cpssDxChPortEnableSet err code 0x%0x,"
		  " dev_num %d, port %d\n", rc, dev_num, port);
	    return (rc);
	}
    }

    for (port = GE0_XCAT2_PORT; port <= GE1_XCAT2_PORT; port++) {
	rc = cpssDxChPortMacResetStateSet(dev_num, port, GT_FALSE);
	if (rc != GT_OK) {
	    cterr('f',0," Error failed port unreset:cpssDxChPortMacResetStateSet err 0x%0x,"
		   " dev_num %d, port %d\n", rc, dev_num, port);
	    return (rc);
	}

	rc = cpssDxChPortEnableSet(dev_num, port, GT_TRUE);
	if (rc != GT_OK) {
	    cterr('f',0," Error failed port enable:cpssDxChPortEnableSet err code 0x%0x,"
		  " dev_num %d, port %d\n", rc, dev_num, port);
	    return (rc);
	}
    }
#if defined(TACHI_INTEL) || defined(YWEN)
    rc = cpssDxChBrgVlanEgressFilteringEnable(dev_num, GT_TRUE); 
    if (rc) {
	cterr('f',0,"cpssDxChBrgVlanEgressFilteringEnable failed\n");
	return (rc);
    }
#endif

    return (rc);
}


/*
 **********************************************************************
 *
 *  Function: xcat2_specific_port_enable
 *
 *  Description: Xcat2 specific port enable
 *
 *  Input: None
 *  Outputs  : PASSED/FAILED
 *       
 **********************************************************************
 */
static uint32_t 
xcat2_specific_port_enable (void)
{
    uint32_t rc = GT_OK;
    GT_U32 port;
    GT_U32 dev_num = xcat2_dev_num[get_slot_num()-1];
    GT_BOOL is_link_up;

    /* Port Enable */
    for (port = 0; port < XCAT2_USED_PORT; port++) {
	rc = xcat2_port_enable(dev_num, port);
	if (rc != GT_OK) {
	    cterr('f',0,"Error failed xcat2_port_enable 0x%0x, "
		   "dev_num %d, port %d\n", rc, dev_num, port);
	    return (rc);
	}
	
	if (port_force_link_set(LINK_UP, port, TRUE) != OK) {
	    cterr('f',0,"Failed port_force_link_set()");
	    return (FAILED);
	}
	if (cpssDxChPortLinkStatusGet(dev_num, port, &is_link_up) != OK) {
	    cterr('f',0,"Failed cpssDxChPortLinkStatusGet()");
	    return (FAILED);
	}
	if (is_link_up == GT_FALSE) {
	    cterr('f',0,"Link is not up for port %d", port);
	    return (FAILED);
	}
    }

    for (port = GE0_XCAT2_PORT; port <= GE1_XCAT2_PORT; port++) {
	rc = xcat2_port_enable(dev_num, port);
	if (rc != GT_OK) {
	    cterr('f',0,"Error failed xcat2_port_enable 0x%0x, "
		   "dev_num %d, port %d\n", rc, dev_num, port);
	    return (rc);
	}
    }

    return (GT_OK);
}


/******************************************************************** 
* Function:  xcat2_cpss_driver_init
*
* Input: None
*
* Output:
*       GT_OK       - on success
*       GT_FAIL     - on error
*       GT_HW_ERROR - on hardware error
* Description  : CPSS driver init
*  
* Returns:  Unix style pass(zero) / fail (non-zero)
*
********************************************************************/
uint32_t 
xcat2_cpss_driver_init ()
{
    uint32_t rc = 0;
    GT_U32 dev_num = xcat2_dev_num[get_slot_num()-1];
    CPSS_VERSION_INFO_STC cpssVersion;    
    CPSS_REG_VALUE_INFO_STC *regCfgList; 
    GT_U32 regCfgListSize;       
    CPSS_DXCH_IMPLEMENT_WA_ENT waArr[1]={CPSS_DXCH_IMPLEMENT_WA_SERDES_INTERNAL_REG_ACCESS_E} ;

    /* Get CPSS version */
    cpssDxChVersionGet(&cpssVersion);

    /* Modified for CPSS 4.1 */
    printf("CPSS Version:%s\n", CPSS_STREAM_NAME_CNS);

    rc = xcat2_cpss_pp_phase1_info_init();
    if (rc != GT_OK) {
        cterr('f',0," Failed xcat2_pp_phase1_info_init(), error code 0x%x\n", rc);  
        return (rc);
    } 

    /* Initialize the internal DB of CPSS regarding PPs */
    if (marvell_cpssPpInit == FALSE) {
        rc = cpssPpInit();
        if (rc != GT_OK) {
            cterr('f',0,"Error failed cpssPpInit Error code 0x%x\n",rc);
            return (rc);
        } else {
            marvell_cpssPpInit = TRUE;
        }
    }

    /*********************************************************************/
    /*   HW Phase 1 initialization                                       */
    /*********************************************************************/
    dev_num = xcat2_pp_phase1_info.devNum;
    rc = cpssDxChHwPpPhase1Init(&xcat2_pp_phase1_info, &xcat2_pp_dev_type);
    if ((rc != GT_OK) && (rc != GT_ALREADY_EXIST)) {
        cterr('f',0,"Failed cpssDxChHwPpPhase1Init(), error code 0x%x\n", rc);
        return (rc);
    }  

    rc = cpssDxChHwPpImplementWaInit(dev_num, 1, waArr, NULL);
    if (rc) {
	cterr('f',0,"Failed cpssDxChHwPpImplementWaInit(), error code 0x%x\n", rc);
        return (rc);
    }

    /* Config functions for this board */          
    regCfgList     = dummyRegValInfoList;
    regCfgListSize = (sizeof(dummyRegValInfoList) / sizeof(CPSS_REG_VALUE_INFO_STC));

    /* Set PP's registers */
    /* Modified for CPSS 4.1 */
    //rc = hwPpStartInit(dev_num, regCfgList, regCfgListSize);
    rc = cpssDxChHwPpStartInit(dev_num, 0, regCfgList, regCfgListSize);
    if (rc != GT_OK) {
	cterr('f',0,"Failed hwPpStartInit(), error code 0x%x\n", rc);
	return rc;
    }

    /*********************************************************************/
    /*   HW Phase 2 initialization                                       */
    /*********************************************************************/
    rc= xcat2_cpss_pp_phase2_info_init();
    if (rc != GT_OK) {
        cterr('f',0,"Failed xcat2_cpss_pp_phase2_info_init(), "
	      "error code 0x%x\n", rc);
        return (rc);
    } 

    rc = cpssDxChHwPpPhase2Init(dev_num, &xcat2_pp_phase2_info);
    if (rc != GT_OK) {
        cterr('f',0,"Failed cpssDxChHwPpPhase2Init(), error code 0x%x\n", rc);  
        return (rc);
    } 

    return rc;
}



/*
 **********************************************************************
 *
 *  Function: xcat2_device_init
 *
 *  Description: Xcat2 device initial
 *
 *  Input: None
 *  Outputs  : PASSED/FAILED
 *       
 **********************************************************************
 */
uint32_t xcat2_device_init ()
{
    uint32_t rc = 0;
    GT_U32 dev_num = xcat2_dev_num[get_slot_num()-1];
#ifdef TACHI_INTEL
    int ia; 
#endif 
    
    CPSS_DXCH_PP_CONFIG_INIT_STC    ppConfig;

    /*********************************************************************/
    /*   Logical phase initialization                                      */
    /*********************************************************************/
    ppConfig.routingMode = CPSS_DXCH_POLICY_BASED_ROUTING_ONLY_E;
    rc = cpssDxChCfgPpLogicalInit(dev_num, &ppConfig);
    if (rc != GT_OK) {
        cterr('f',0,"Failed cpssDxChCfgPpLogicalInit(),  rc %d\n", rc);  
        return (rc);
    } 
   
    /*********************************************************************/
    /*   FDB initialization                                              */
    /*********************************************************************/
    rc = cpssDxChBrgFdbInit(dev_num);
    if (rc != GT_OK) {
        cterr('f',0,"Failed cpssDxChBrgFdbInit() rc %d\n", rc);  
        return (rc);
    }

    /********** Set FDB hash function mode */
    rc = cpssDxChBrgFdbHashModeSet(dev_num, CPSS_MAC_HASH_FUNC_XOR_E);
    if (rc != GT_OK) {
        cterr('f', 0,"Failed cpssDxChBrgFdbHashModeSet(),  rc %d\n", rc);
        return (rc);
    } 

    /********** Set the VLAN lookup mode */
    rc = cpssDxChBrgFdbMacVlanLookupModeSet(dev_num, CPSS_IVL_E);
    if (rc != GT_OK) {
        printf("Failed cpssDxChBrgFdbMacVlanLookupModeSet()\n");
        return (rc);
    } 

    /********** VLAN Initialization-- set VLAN-aware mode */
    rc = cpssDxChBrgVlanBridgingModeSet(dev_num, CPSS_BRG_MODE_802_1Q_E);
    if (rc != GT_OK) {
        cterr('f',0,"Failed cpssDxChBrgVlanBridgingModeSet()\n");  
        return (rc);
    } 

    /********** Port Initialization */
    rc = cpssDxChPortStatInit(dev_num);
    if (rc != GT_OK) {
        cterr('f',0,"Failed cpssDxChPortStatInit()\n");  
        return (rc);
    }

    /********** Port Configuration */
    rc = xcat2_specific_port_init();
    if (rc != GT_OK) {
        cterr('f', 0, "xcat2_specific_port_init() failed, rc 0x%x\n", rc);
        return (rc);
    } 

    /********** Enable ports */
    rc = xcat2_specific_port_enable();
    if (rc != GT_OK) {
        cterr('f',0,"xcat2_specific_port_enable() failed, rc 0x%x\n", rc);  
        return (rc);
    } 

    /********** Enable device */
    rc = cpssDxChCfgDevEnable(dev_num, GT_TRUE);
    if(rc != GT_OK) {
        cterr('f',0,"cpssDxChCfgDevEnable() failed, rc 0x%x\n",rc);
        return (rc);
    }

#ifdef TACHI_INTEL
    printf("Init vlan 1-5 for Cross Functional Team \n");
    printf("Using PHY utility to setup vlan for ports \n");
    /********** Create Vlans: VLAN_1 and VLAN_2 */
    for (ia = VLAN_1; ia < (VLAN_5 + 1); ia++) {
        rc = xcat2_vlan_add(dev_num, ia) ;
        if(rc != GT_OK) {
            cterr('f',0,"Failed to add Vlan %d.", ia);
            return (rc);
        }
    }
#endif 

#ifdef YWEN
    /********** Create Vlans: VLAN_1 and VLAN_2 */
    rc = xcat2_vlan_add(dev_num, VLAN_1) ;
    if(rc != GT_OK) {
        cterr('f',0,"Failed to add Vlan 1.");
        return (rc);
    }    

    rc = xcat2_vlan_add(dev_num, VLAN_2) ;
    if(rc != GT_OK) {
        cterr('f',0,"Failed to add Vlan 2.");
        return (rc);
    } 

    /* add ports to vlan 1 and 2 */
    rc = xcat2_vlan_port_add(dev_num, VLAN_1, GE1_XCAT2_PORT);
    if(rc != GT_OK) {
        cterr('f',0,"Failed to add port: %d to Vlan 1.", GE1_XCAT2_PORT);
        return (rc);
    }    
    rc = xcat2_vlan_port_del(dev_num, VLAN_1, GE0_XCAT2_PORT);
    if(rc != GT_OK) {
        cterr('f',0,"Failed to delete port: %d from Vlan 1.", GE0_XCAT2_PORT);
        return (rc);
    }
    rc = xcat2_vlan_port_add(dev_num, VLAN_2, GE0_XCAT2_PORT);
    if(rc != GT_OK) {
        cterr('f',0,"Failed to add port: %d to Vlan 2.", GE0_XCAT2_PORT);
        return (rc);
    }

    rc = xcat2_vlan_port_del(dev_num, VLAN_1, 7);
    if(rc != GT_OK) {
        cterr('f',0,"Failed to delete port: %d from Vlan 1.", GE1_XCAT2_PORT);
        return (rc);
    }
    rc = xcat2_vlan_port_add(dev_num, VLAN_2, 7);
    if(rc != GT_OK) {
        cterr('f',0,"Failed to add port: %d to Vlan 2.", GE1_XCAT2_PORT);
        return (rc);
    }

    for (i = 0; i < (XCAT2_USED_PORT-1); i++) {
	rc = xcat2_vlan_port_add(dev_num, VLAN_1, i) ;
	if(rc != GT_OK) {
	    cterr('f',0,"Failed to add port: %d to Vlan 1.", i);
	    return (rc);
	}    
    }
#endif

    return (rc);
}
#if defined(TACHI_INTEL) || defined(YWEN)  
static int del_all_vlan (int dev_num, int port) {

    int rc = 0;

    rc |= xcat2_vlan_port_del(dev_num, VLAN_1, port);
    rc |= xcat2_vlan_port_del(dev_num, VLAN_2, port);
    rc |= xcat2_vlan_port_del(dev_num, VLAN_3, port);
    rc |= xcat2_vlan_port_del(dev_num, VLAN_4, port);
    rc |= xcat2_vlan_port_del(dev_num, VLAN_5, port);
    if (rc != GT_OK) {
        cterr('f',0,"Failed to delete port%d from Vlan 1-5.", port);
    }

    return (rc);
}
#endif

/*
 **********************************************************************
 *
 *  Function: xcat2_init
 *
 *  Description: Xcat2 initial
 *
 *  Input: None
 *  Outputs  : 
 *       GT_OK       - on success
 *       GT_FAIL     - on error
 **********************************************************************
 */
uint32_t 
xcat2_init ()
{
    uint32_t rc = OK;
    
    /* Xcat2 CPSS Driver initialization. */ 
    rc = xcat2_cpss_driver_init();
    if (rc) {
	cterr('f',0,"Failed CPSS Driver Init \n");
	return (GT_FAIL);
    } 

    rc = xcat2_device_init();
    if (rc) {
	cterr('f',0,"Failed Xcat2 Device Init \n");
	return (GT_FAIL);
    }

    /* Clear port interrupt */
    rc = xcat2_clear_all_port_interrupt();
    if (rc != GT_OK) {
        cterr('f',0,"Failed to clear all port interrupt \n");
        return (GT_FAIL);
    } 

    return (rc);
}



/*
 *------------------------------------------------------------------
 * $Log: dreamliner_xcat2_init.c,v $
 * Revision 1.2  2019/12/11 10:10:26  lucywang
 * Merged Nanook to main trunk
 *
 * 
 *------------------------------------------------------------------
 * $Endlog$
 */
