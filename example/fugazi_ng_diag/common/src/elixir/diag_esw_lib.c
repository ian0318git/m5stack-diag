/* $Id: diag_esw_lib.c,v 1.2 2021/09/24 01:21:06 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/elixir/diag_esw_lib.c,v $
 *------------------------------------------------------------------
 * 
 * diag_esw_lib.c
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "error.h"
#include "common.h"
#include "types.h"
#include "proto.h"
#include "queryflags.h"
#include "menu.h"
#include "ethernet.h"
#include "common_utils.h"
#include "nvmonvars.h"
#include "diag_smi_lib.h"
#include "diag_moka_fpga_lib.h"
#include "diag_cpu_lib.h"
#include "diag_enhance_err_msg_lib.h"
#include "platform_cookie.h"
#include "diag_esw_lib.h"
#include "diag_esw_util.h"
#include "diag_cpu_lib.h"
#include "linux_pciutils.h"
#include "dev_98dxc25x.h"
#include "dev_88e1680.h"
#include "diag_esw_test.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>


/* Variable */
static int phy_dev_88e1680_group_start_addr = ELIXIR_1680_GROUP_0_START_ADDR; 
static dev_98dxc25x_object_t esw_98dxc25x_obj; /* 98dxc25x object*/
static dev_88e1680_object_t phy_88e1680_obj; /* 88e1680 object*/
static int dev_98dxc25x_init = FALSE;
static int dev_88e1680_init = FALSE;

/* Function */
extern uint32 err_report(dev_object_t *, char *, uint32);
extern uint32_t pci_domain_config_read(uint32_t, uint32_t, uint16_t, uint32_t, int);
extern uint32_t pci_domain_config_write(uint32_t, uint32_t, uint16_t, uint32_t, int, uint32_t);
extern int get_pcie_link_cap_with_domain(uint32_t, uint32_t, uint16_t, int, uint);
extern int get_pcie_link_status_with_domain(uint32_t, uint32_t, uint16_t, int, uint);
extern int get_pcie_cap_struct_ptr_with_domain(uint32_t, uint32_t, uint16_t, int, uint);

static int xcat5_specific_port_init(uint);
static int xcat5_port_init(uint, uint);
static int xcat5_port_force_link_set(uint, uint, int, boolean);
static int xcat5_specific_port_enable(uint);
static int xcat5_port_enable(uint, uint);
static int xcat5_smi_phy_init(uint);
static void xcat5_reg_config_read(uint, uint, uint *);
static int phy_mad_load_driver(MAD_DEV *, int);
static unsigned int phy_smi_read(MAD_DEV *, unsigned int, unsigned int, unsigned int *);
static unsigned int phy_smi_write(MAD_DEV *, unsigned int, unsigned int, unsigned int);
static char * phy_get_dev_name(MAD_DEVICE_ID);
static int phy_mad_disable_int(MAD_DEV *);
static int phy_mad_set_phy_enable(MAD_DEV *, MAD_LPORT, int);
static int xcat5_port_phy_1680_init(void);
static int phy_read_reg(MAD_DEV *, MAD_LPORT, MAD_U16, MAD_U16, MAD_U32 *);
static int phy_write_reg(MAD_DEV *, MAD_LPORT, MAD_U16, MAD_U16, MAD_U16);
static int xcat5_port_disable(uint, uint);
static int phy_mad_display_reg(MAD_DEV *, MAD_U8, MAD_U16);
static void xcat5_reg_config_write(uint, uint, uint);
static int xcat5_pcs_loopback_enable(uint, uint);
static int xcat5_pcs_loopback_disable(uint, uint);
static void xcat5_pcie_config_read(int, uint *);
static void xcat5_pcie_config_write(int, uint);
static int phy_mad_set_test_mode(MAD_DEV *, uint, uint);
static int xcat5_exit(MAD_DEV *);
static int xcat5_cpss_pp_phase1_info_init(CPSS_DXCH_PP_PHASE1_INIT_INFO_STC *);
static int xcat5_cpss_pp_phase2_info_init(uint, CPSS_DXCH_PP_PHASE2_INIT_INFO_STC *);

int diag_ac5_init(void);
int diag_esw_exit(void);
int smi_read_reg(unsigned int, unsigned int, unsigned short *);
int smi_write_reg(unsigned int, unsigned int, unsigned short);
int diag_config_port_speed(uint, uint, uint);
int diag_reset_esw_to_default(void);
int xcat5_reg_pci_read(uint, uint, uint*);
void diag_esw_remove_pcie_device(void);
int diag_esw_all_phy_green_led_off(void);
int diag_esw_all_phy_green_led_on(void);

/*******************************************************************************
 *
 * Function    : diag_ac5_init
 * Description : Function to init AC5 Switch
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int diag_ac5_init(void)
{
   
    dev_98dxc25x_object_t *esw_98dxc25x_obj_p = NULL;
    dev_object_t *esw_dev;

    dev_88e1680_object_t *phy_88e1680_obj_p = NULL;
    dev_object_t *phy_dev;

    int rc = 0;
    int ix = 0;
    MAD_DEV * mad_dev;
    int phy_addr;
    uint32_t bus, domain, reg_val, cap_val, sta_val;
    uint32_t cap_s, sta_s, sta_w; /*cap_w*/


    bus = get_pcie_bus_num(AC5_DEV_VID, AC5_DEV_PID);
    domain = 0;

    reg_val = get_pcie_cap_struct_ptr_with_domain(domain, bus, PCI_DEV_0, PCI_FUN_0, PCI_CAP_PTR_OFFSET);
    if (reg_val == FAILED) {
        cterr('f',0, "Can't get PCI cap pointer");
        return (FAILED);
    }

    cap_val = get_pcie_link_cap_with_domain(domain, bus, PCI_DEV_0, PCI_FUN_0, reg_val);
    sta_val = get_pcie_link_status_with_domain(domain, bus, PCI_DEV_0, PCI_FUN_0, reg_val);

    /* Speed - bit 0~3 */
    cap_s = cap_val & PCI_EXP_LINK_STA_SPD_MASK;
    sta_s = sta_val & PCI_EXP_LINK_STA_SPD_MASK;
    /* Width - bit 4~9 */
    sta_w = (sta_val & PCI_EXP_LINK_STA_WID_MASK) >> PCI_EXP_LINK_WID_SHIFT;

    printf("Capbility Speed: %x, Link Speed: %x, Link Width: %x\n", cap_s, sta_s, sta_w);

    if (sta_s != PCI_EXP_LINK_STA_SPD_8GT) {
        printf("It's not PCIE GEN3 speed between AC5 and CPU\n");
    }

    /* Init pcie address to prepare some data variable for following process.
     * After resize pcie bar and mumap pcie address, need to init pcie address again.
     */

    if (cpss_pcie_extserv_init_ex() != PASSED) {
        return (FAILED);
    }
   
    cpss_pcie_bar2_resize();

    cpss_pcie_extserv_cleanup_ex();
    
    msleep(500);

    /* To rescaen pci device after resize bar2 */
    system(PCIE_REMOVE);
    msleep(500);
    system(PCIE_RESCAN);
    msleep(500);
    system(PCIE_ENABLE00);
    msleep(300);
    system(PCIE_ENABLE01);
    msleep(300);

    if (cpss_pcie_extserv_init_ex() != PASSED) {
        return (FAILED);
    }

    /* Create 98dxc25x device driver */
    esw_98dxc25x_obj_p = (dev_98dxc25x_object_t *)diag_get_esw_98dxc25x_obj(); 
    if (esw_98dxc25x_obj_p == NULL) {
        cterr('f', 0, "%s: Creating object failed", __func__);
        return (FAILED);
    }
    esw_dev = (dev_object_t *)esw_98dxc25x_obj_p;


    /* Call 98dxc25x xcat5 init function */
    if (esw_98dxc25x_obj_p->callin_fvt->esw_xcat5_init(esw_dev, esw_98dxc25x_obj_p->cpss_dev) != PASSED) {
        printf("%s:%d Failed to ESW xcat5 init.",
               __FUNCTION__, __LINE__);
        goto _exit;
    }

    /* Create 88e1680 device driver */
    phy_88e1680_obj_p = (dev_88e1680_object_t *)diag_get_phy_88e1680_obj(); 
    if (phy_88e1680_obj_p == NULL) {
        cterr('f', 0, "%s: Creating object failed", __func__);
        return (FAILED);
    }
    phy_dev = (dev_object_t *)phy_88e1680_obj_p;


    /* Call 88e1680 MAD driver start function */
    phy_addr = phy_dev_88e1680_group_start_addr;
    mad_dev = &phy_mad_88e1680;
    if (phy_88e1680_obj_p->callin_fvt->phy_start_mad_driver(phy_dev, mad_dev, phy_addr) != PASSED) {
        printf("%s:%d Failed to start phy mad driver for 88e1680.",
                __FUNCTION__, __LINE__);
        goto _exit;
    }

    /* Config AC5 & 1680 init port setting */
    rc = xcat5_port_phy_1680_init();
    if (rc == FAILED) {
        cterr('f', 0, "%s: xcat5_port_phy_1680_init failed",__func__);
        goto _exit;
    }

    /* Modify register value to conform HW measurement criterion (QSGMII voltage amplitude) */
    for (ix = 0; ix < ELIXIR_ESW_PORT_NUM; ix++) {

        rc = cpssDxChPhyPortSmiRegisterWrite(esw_98dxc25x_obj_p->cpss_dev, ix, 22, 0xFD);
        if (rc != GT_OK) {
            cterr('f', 0, "%s: Failed to do AC5 & phy port init setting", __func__);
            goto _exit;
        }

        rc = cpssDxChPhyPortSmiRegisterWrite(esw_98dxc25x_obj_p->cpss_dev, ix, 8, 0xB45);
        if (rc != GT_OK) {
            cterr('f', 0, "%s: Failed to do AC5 & phy port init setting", __func__);
            goto _exit;
        }

        rc = cpssDxChPhyPortSmiRegisterWrite(esw_98dxc25x_obj_p->cpss_dev, ix, 7, 0x200D);
        if (rc != GT_OK) {
            cterr('f', 0, "%s: Failed to do AC5 & phy port init setting", __func__);
            goto _exit;
        }

        rc = cpssDxChPhyPortSmiRegisterWrite(esw_98dxc25x_obj_p->cpss_dev, ix, 22, 0x0);
        if (rc != GT_OK) {
            cterr('f', 0, "%s: Failed to do AC5 & phy port init setting", __func__);
            goto _exit;
        }
    }

    return (PASSED);

 _exit:

    return (FAILED);

}

/**********************************************************************
 * Function: diag_get_esw_98dxc25x_obj
 * Description: (1) Get the 98dxc25x device driver object
 *              (2) Attach configurations to device object
 * Inputs     : None 
 * Outputs    : address of device object
 **********************************************************************
 */
dev_object_t *diag_get_esw_98dxc25x_obj (void)
{

    int rc; 

    dev_98dxc25x_object_t *esw_98dxc25x_obj_p  = &esw_98dxc25x_obj;

    if (dev_98dxc25x_init == FALSE) {

        /* Setup device object base */
        mrv98dxc25x_dev_create((dev_object_t *)esw_98dxc25x_obj_p, (dev_error_report_t)err_report);

        /* Setup call-out function vectors */
        esw_98dxc25x_obj_p->cpss_dev = ELIXIR_AC5_CPSS_DEV;
        esw_98dxc25x_obj_p->port_group = XCAT5_PCI_PORT_GROUP;

        esw_98dxc25x_obj_p->callout_fvt->cpss_pp_phase1_info_init = xcat5_cpss_pp_phase1_info_init;
        esw_98dxc25x_obj_p->callout_fvt->cpss_pp_phase2_info_init = xcat5_cpss_pp_phase2_info_init; 
        esw_98dxc25x_obj_p->callout_fvt->xcat5_specific_port_init = xcat5_specific_port_init;
        esw_98dxc25x_obj_p->callout_fvt->xcat5_specific_port_enable = xcat5_specific_port_enable;
        esw_98dxc25x_obj_p->callout_fvt->smi_phy_init = xcat5_smi_phy_init;
        esw_98dxc25x_obj_p->callout_fvt->reg_config_rd = xcat5_reg_config_read;
        esw_98dxc25x_obj_p->callout_fvt->port_enable = xcat5_port_enable;
        esw_98dxc25x_obj_p->callout_fvt->reg_config_wr = xcat5_reg_config_write;
        esw_98dxc25x_obj_p->callout_fvt->pcs_loopback_enable = xcat5_pcs_loopback_enable;
        esw_98dxc25x_obj_p->callout_fvt->pcs_loopback_disable = xcat5_pcs_loopback_disable;
        esw_98dxc25x_obj_p->callout_fvt->pcie_config_read = xcat5_pcie_config_read;
        esw_98dxc25x_obj_p->callout_fvt->pcie_config_write = xcat5_pcie_config_write;
        esw_98dxc25x_obj_p->callout_fvt->xcat5_exit = xcat5_exit;


        /* Attach the device object */
        rc = esw_98dxc25x_obj_p->base.dev_object_fvt->dev_attach((dev_object_t *)esw_98dxc25x_obj_p);
        if (rc != PASSED) {
            printf("%s:%d:Failed to attach 98dxc25x object\n", __func__, __LINE__);
            return (NULL);
        }
        dev_98dxc25x_init = TRUE;

    }

    return ((dev_object_t *)esw_98dxc25x_obj_p);
}

/***********************************************************************
 *
 *  Function: xcat5_cpss_pp_phase1_info_init
 *
 *  Description: Xcat5 initial phase 1
 *
 *  Input: xcat5_pp_phase1_info_ptr - phase1_info pointer 
 *
 *  Outputs: PASSED / FAILED
 *       
 **********************************************************************
 */
static int xcat5_cpss_pp_phase1_info_init (CPSS_DXCH_PP_PHASE1_INIT_INFO_STC *xcat5_pp_phase1_info_ptr)
{
    CPSS_DXCH_PP_PHASE1_INIT_INFO_STC *info;
    cpss_pcie_bind_func bind_func;
    GT_VOID *int_vect;
    GT_UINTPTR int_mask;
    GT_UINTPTR pci_base, internal_base;
    GT_UINTPTR pci_base_size, internal_base_size;


    /* since we use MSI interrupt, PCI interrupt line number is a dummy here */
    if (extDrvGetPciIntVec(0, (void **)&int_vect) != GT_OK) {
	    cterr('f',0,"Failed to get interrupt vector number.");
	    return (FAILED);
    }

    if (extDrvGetIntMask(0, &int_mask) != GT_OK) {
	    cterr('f',0,"Failed to get interrupt mask.");
	    return (FAILED);
    }
#ifdef DEBUG
    printf("int_vect = %lu, int_mask = %u\n", (unsigned long)int_vect, int_mask);
#endif
    cpss_pcie_get_pciemap_ex(&pci_base, &internal_base);
    cpss_pcie_get_pciemap_size_ex(&pci_base_size, &internal_base_size);

#ifdef DEBUG
    printf("pci_base = %lx, internal_base = %lx, exreg_base = %lx\n", pci_base, internal_base);
#endif

    cpss_pcie_get_extserv(&bind_func.extDrv, &bind_func.os, &bind_func.trace);
    if (cpssExtServicesBind(&bind_func.extDrv, &bind_func.os, &bind_func.trace) != GT_OK) {
	    cterr('f',0,"Failed to do cpssExtServicesBind()");
	    return (FAILED);
    }

    info = xcat5_pp_phase1_info_ptr;

    memset(info, 0, sizeof(*info));

    /* AC5 PHASE 1 CONFIG 
     * Execute Marvel app_demo application separately to print related setting value.
     * And copy those value to here, then we can use our diag application to initialize AC5.
     */

    info->hwInfo[0].busType = CPSS_HW_INFO_BUS_TYPE_PEX_E; /* new structure member */
    info->hwInfo[0].hwAddr.busNo = 1;
    info->hwInfo[0].hwAddr.devSel = 0;
    info->hwInfo[0].hwAddr.funcNo = 0;
    info->devNum = 0;

    info->hwInfo[0].resource.switching.start = pci_base;
    info->hwInfo[0].resource.cnm.start = internal_base;
    info->hwInfo[0].resource.resetAndInitController.start = 0;
    info->hwInfo[0].resource.sram.start = 0;

    info->hwInfo[0].resource.switching.size = pci_base_size;
    info->hwInfo[0].resource.cnm.size = internal_base_size;
    info->hwInfo[0].resource.resetAndInitController.size = 0;
    info->hwInfo[0].resource.sram.size = 0;

    info->hwInfo[0].resource.switching.phys = 0x0;
    info->hwInfo[0].resource.cnm.phys = 0x800000;
    info->hwInfo[0].resource.resetAndInitController.phys = 0;

    info->hwInfo[0].irq.switching = 0xffffffff;
    info->hwInfo[0].intMask.switching = 0xffffffff;

    info->coreClock = CPSS_DXCH_AUTO_DETECT_CORE_CLOCK_CNS;
    info->mngInterfaceType = CPSS_CHANNEL_PEX_EAGLE_E;
    info->isrAddrCompletionRegionsBmp = 0x02;
    info->appAddrCompletionRegionsBmp = 0x3C;
    info->ppHAState = CPSS_SYS_HA_MODE_ACTIVE_E;
    info->serdesRefClock = CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_25_SINGLE_ENDED_E;
    info->isExternalCpuConnected = GT_FALSE;
    
    info->numOfPortGroups = 1;

    return (PASSED);
}

/**********************************************************************
 *
 *  Function: xcat5_cpss_pp_phase2_info_init
 *
 *  Description: Xcat5 initial phase 2
 *
 *  Input: cpss_dev - cpss dev number
 *         xcat5_pp_phase2_info_ptr - phase2_info pointer 
 *
 *  Outputs: PASSED - on success
 *       
 **********************************************************************
 */
static int xcat5_cpss_pp_phase2_info_init (uint cpss_dev, 
                                           CPSS_DXCH_PP_PHASE2_INIT_INFO_STC *xcat5_pp_phase2_info_ptr)
{
    CPSS_DXCH_PP_PHASE2_INIT_INFO_STC *info;
    int tc = 0;

    info = xcat5_pp_phase2_info_ptr;

    memset(info, 0, sizeof(*info));

    info->newDevNum = cpss_dev;	

    info->fuqUseSeparate = FALSE;

    info->auqCfg.auDescBlockSize = 0x8000;
    info->auqCfg.auDescBlock = NULL;
    info->fuqCfg.auDescBlockSize = 0x8000;
    info->fuqCfg.auDescBlock = NULL;

    info->netIfCfg.txDescBlockSize = 0x1000;
    info->netIfCfg.txDescBlock = NULL;

    info->netIfCfg.rxDescBlockSize = 0x1000;
    info->netIfCfg.rxDescBlock = NULL;
    info->netIfCfg.rxBufInfo.allocMethod = CPSS_RX_BUFF_STATIC_ALLOC_E;
    info->netIfCfg.rxBufInfo.buffData.staticAlloc.rxBufBlockSize = 0x3e80;
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
    info->netIfCfg.rxBufInfo.rxBufSize = 0x60c;

    return (PASSED);

}

/*
 **********************************************************************
 *
 *  Function: xcat5_specific_port_init
 *
 *  Description: Xcat5 specific port initial
 *
 *  Input: cpss_dev - cpss dev number
 *
 *  Outputs: PASSED / FAILED
 *       
 **********************************************************************
 */
static int xcat5_specific_port_init (uint cpss_dev)
{
    uint32_t rc = GT_OK;
    GT_U32 port;
    GT_U32 dev_num = cpss_dev;

    /* Port initialization */
    /* port 0-7 for Phy*/
    for (port = 0; port < ELIXIR_XCAT5_USED_PORT; port++) {
	    rc = xcat5_port_init(dev_num, port);
	    if (rc != GT_OK) {
	        cterr('f',0," Failed xcat5_port_init err code 0x%0x,"
		       " dev_num %d, port %d\n", rc, dev_num, port);
	        return (rc);
	    }
    }

    /* Port 26 for CPU */
    port = XCAT5_TO_CPU_PORT;
    rc = xcat5_port_init(dev_num, port);
	if (rc != GT_OK) {
	    cterr('f',0," Failed xcat5_port_init err code 0x%0x,"
		   " dev_num %d, port %d\n", rc, dev_num, port);
	    return (rc);
    }

    /* Port 24 for wifi module */
    port = XCAT5_TO_WIFI_PORT;
    rc = xcat5_port_init(dev_num, port);
	if (rc != GT_OK) {
	    cterr('f',0," Failed xcat5_port_init err code 0x%0x,"
		   " dev_num %d, port %d\n", rc, dev_num, port);
	    return (rc);
    }

    /* put used ports in reset */
    /* Port 0-7 for Phy */
    for (port = 0; port < ELIXIR_XCAT5_USED_PORT; port++) {
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
    /* Port 0-7 for phy */
    for (port = 0; port < ELIXIR_XCAT5_USED_PORT; port++) {
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

    /* Port 26 for CPU */
    port = XCAT5_TO_CPU_PORT;
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

    /* Port 24 for wifi module */
    port = XCAT5_TO_WIFI_PORT;
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
    return (rc);
}

/*
 **********************************************************************
 *
 *  Function: xcat5_port_init
 *
 *  Description: Xcat5 port initial
 *
 *  Input: dev_num - device number
 *         port_num - port number
 *
 *  Outputs: PASSED/FAILED
 *       
 **********************************************************************
 */
static int xcat5_port_init (uint dev_num, uint port_num)
{
    uint32_t rc = GT_OK;
    CPSS_PORT_SPEED_ENT speed;
    uint32_t port_mode;

    CPSS_PORTS_BMP_STC      initPortsBmp;       /* bitmap of ports to init */
    CPSS_PORTS_BMP_STC      *initPortsBmpPtr; /* because argument 2 of new cpssDxChPortModeSpeedSet function change from normal type to pointer type */ 



    if (xcat5_port_force_link_set(dev_num, LINKDOWN, port_num, TRUE) != OK) {
        cterr('f',0,"Failed xcat5_port_force_link_set()");
        return (FAILED);
    }

    if (port_num >= XCAT5_TO_WIFI_PORT) {
	
	    port_mode = CPSS_PORT_INTERFACE_MODE_SGMII_E;
	    speed = CPSS_PORT_SPEED_2500_E;

    } else {
	    port_mode = CPSS_PORT_INTERFACE_MODE_QSGMII_E;
	    speed = CPSS_PORT_SPEED_1000_E;
    }

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&initPortsBmp);
    CPSS_PORTS_BMP_PORT_SET_MAC(&initPortsBmp, port_num);

    initPortsBmpPtr = &initPortsBmp;
    rc = cpssDxChPortModeSpeedSet(dev_num, initPortsBmpPtr, GT_TRUE, port_mode, speed);
    if (rc != GT_OK) {
	    cterr('f',0,"Failed to call cpssDxChPortInterfaceModeSet() for port %d,"
	          " err code %d", port_num, rc);
	    return (rc);
    }

    if (port_num < XCAT5_TO_WIFI_PORT) {
        if ((rc = cpssDxChPortInbandAutoNegEnableSet(dev_num,port_num,GT_TRUE)) != GT_OK) {
            return (rc);
        }
    }

    rc = cpssDxChPortSerdesPowerStatusSet(dev_num,port_num,
					  CPSS_PORT_DIRECTION_BOTH_E,0x8,GT_TRUE);
    if (rc != GT_OK) {
	    cterr('f',0,"Failed cpssDxChPortSerdesPowerStatusSet, rc = %#x", rc);
	    return (rc);
    }
    rc = cpssDxChPortMacCountersEnable(dev_num, port_num, GT_TRUE);
    if (rc != GT_OK) {
        cterr('f',0,"failed to enable Port counters, err code %d\n", rc);
        return (rc);
    }

    rc = cpssDxChPortMacCountersClearOnReadSet(dev_num, port_num, GT_FALSE);
    if (rc != GT_OK) {
        cterr('f',0,"failed to clear counters, err code %d\n", rc);
        return (rc);
    }

    rc = cpssDxChBrgVlanPortIngFltEnable(dev_num, port_num, GT_TRUE);
    if (rc) {
	    cterr('f',0,"cpssDxChBrgVlanPortIngFltEnable failed\n");
	    return (rc);
    }

    if (xcat5_port_force_link_set(dev_num, LINKDOWN, port_num, FALSE) != OK) {
        cterr('f',0,"Failed xcat5_port_force_link_set()");
        return (FAILED);
    }

    return (rc);
	
}

/******************************************************************************
 *
 * Function   :	xcat5_port_force_link_set
 *
 * Description:	set force link status for a specific port
 *              
 * Inputs     :	link: LINK_UP or LINK_DOWN
 *              port:
 *              set: TRUE or FALSE
 *
 * Outputs    : PASSED / FAILED
 *
 ******************************************************************************
 */
static int xcat5_port_force_link_set (uint cpss_dev, uint link, int port, boolean set)
{
    uint32_t dev_num = cpss_dev;
    int rc = 0;

    if (link == LINKDOWN) {
	    rc = cpssDxChPortForceLinkDownEnableSet(dev_num, port, set);
	    if(rc != GT_OK) {
	        printf("Failed cpssDxChPortForceLinkDownEnableSet call, rc = %x", rc);
	        return (FAILED);
	    }
    } else {
	    rc = cpssDxChPortForceLinkPassEnableSet(dev_num, port, set);
	    if(rc != GT_OK) {
	        printf("Failed cpssDxChPortForceLinkDownEnableSet call, rc = %x", rc);
	        return (FAILED);
	    }
    }
    return (PASSED);
}

/*
 **********************************************************************
 *
 *  Function: xcat5_specific_port_enable
 *
 *  Description: Xcat5 specific port enable
 *
 *  Input: None
 *
 *  Outputs  : PASSED / FAILED
 *       
 **********************************************************************
 */
static int xcat5_specific_port_enable (uint cpss_dev)
{
    uint32_t rc = GT_OK;
    GT_U32 port;
    GT_U32 dev_num = cpss_dev;
    GT_BOOL is_link_up;

    /* Port Enable */
    /* Port 0-7 for Phy */
    for (port = 0; port < ELIXIR_XCAT5_USED_PORT; port++) {
    
	    rc = xcat5_port_enable(dev_num, port);
	    if (rc != GT_OK) {
	        cterr('f',0,"Error failed xcat5_port_enable 0x%0x, "
		       "dev_num %d, port %d\n", rc, dev_num, port);
	        return (rc);
	    }

	    if (xcat5_port_force_link_set(dev_num, LINKUP, port, TRUE) != OK) {
	        cterr('f',0,"Failed xcat5_port_force_link_set()");
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

    /* Port 26 for CPU */
    port = XCAT5_TO_CPU_PORT;
    rc = xcat5_port_enable(dev_num, port);
    if (rc != GT_OK) {
        cterr('f',0,"Error failed xcat5_port_enable 0x%0x, "
            "dev_num %d, port %d\n", rc, dev_num, port);
	    return (rc);
    }

    /* Port 24 for wifi module */
    port = XCAT5_TO_WIFI_PORT;
    rc = xcat5_port_enable(dev_num, port);
    if (rc != GT_OK) {
        cterr('f',0,"Error failed xcat5_port_enable 0x%0x, "
            "dev_num %d, port %d\n", rc, dev_num, port);
	    return (rc);
    }

    return (GT_OK);
}
/**********************************************************************
 *
 * Function: xcat5_port_enable
 *
 * Description: Set xCat5 port enable
 *
 * Input: dev_num - cpss dev number
 *           port_num - port number
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int xcat5_port_enable (uint dev_num, uint port_num)
{
    uint32_t rc = GT_OK;

    rc = cpssDxChPortEnableSet(dev_num, port_num, 1);
    if (rc != GT_OK) {
        cterr('f',0,"Error failed cpssDxChPortEnableSet \n");
    }
    return (rc);
}

/******************************************************************************
 *
 * Function   :	xcat5_smi_phy_init
 * Description:	init SMI master1 for PHY access.
 *              
 * Inputs     :	cpss_dev - cpss dev number 
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int xcat5_smi_phy_init (uint cpss_dev)
{
    int rc, ix;
    uint32_t sum_err = 0;

    GT_U32 dev_num = cpss_dev;

    /* SMI1, 1680*1, PORT 0-7, PHYSICAL PHY ADDR SMI0 0-7 */
    for (ix = 0; ix < ELIXIR_ESW_SMI1_PORT_START_NUM; ix++) {
	    /* set PHY SMI address */
        rc = cpssDxChPhyPortAddrSet(dev_num, ix, ix);
        if (rc == PASSED) {
            /* set SMI interface 1 for PHY ports */
            rc = cpssDxChPhyPortSmiInterfaceSet(dev_num, ix, CPSS_PHY_SMI_INTERFACE_1_E);
        }
        if (rc != PASSED) {
            sum_err++;
        }
    }

    if (sum_err == 0) {
	    rc = cpssDxChPhyPortSmiInit(dev_num);
	    if (rc != PASSED) {
	        return (GT_FAIL);
        }    
    } else {
	    return (GT_FAIL);  
    }

    return (PASSED);
}

/******************************************************************************
 *
 * Function   :	xcat5_reg_config_read
 *
 * Description: Read memory mapped xCat5 internal registers through 
 *              PCI interface.
 *
 * Inputs     :	cpss_dev - cpss dev number 
 *              offset   - register offset
 *              *data    - pointer to hold the read data
 *
 * Outputs    : None
 *
 ******************************************************************************
 */
static void xcat5_reg_config_read (uint cpss_dev, uint offset, uint *data)
{
    GT_UINTPTR pci_base, internal_base;
    cpss_pcie_get_pciemap_ex(&pci_base, &internal_base);

    *data = *(unsigned int *)(internal_base + offset);

}


/*******************************************************************************
 *
 * Function   : diag_esw_exit
 *
 * Description: Function to exit Elixir switch(Marvell 98DXC25X & PHY Marvell 88E1680)
 *
 * Inputs     : None
 *
 * Outputs    : PASSED / FAILED
 *
 *******************************************************************************
 */
int diag_esw_exit (void)
{

    int rc = 0;
    char dirname[128];
    MAD_DEV * mad_dev;

    dev_98dxc25x_object_t *esw_98dxc25x_obj_p = NULL;
    dev_object_t *dev;


    /* Create 98dxc25x device driver */
    esw_98dxc25x_obj_p = (dev_98dxc25x_object_t *)diag_get_esw_98dxc25x_obj(); 
    if (esw_98dxc25x_obj_p == NULL) {
        cterr('f', 0, "%s: Creating object failed", __func__);
        return (FAILED);
    }
    dev = (dev_object_t *)esw_98dxc25x_obj_p;
 

    mad_dev = &phy_mad_88e1680;

    /* Call 98dxc25x xcat5 finish function */
    rc = esw_98dxc25x_obj_p->callin_fvt->esw_xcat5_exit(dev, 
                                                        esw_98dxc25x_obj_p->cpss_dev, mad_dev);

    if (rc == FAILED) {
        cterr('f',0,"Failed to finish AC5 switch", __func__);
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("1. cpssDxChCfgDevRemove and DmdUnloadDriver Done\n");
    }


    cpss_pcie_extserv_cleanup_ex();

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("2. extserve_cleanup_ex Done\n");
    }

    /* Check if PCIE driver is working, then remove it */
    memset(dirname, '\0', sizeof(dirname));
    snprintf(dirname, sizeof(dirname), "%s", PCIE_DRV_PATH);
    if (access(dirname, F_OK) != -1) {
        system(ETH_RM_AC5_NIM_DM_MODULE);
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("3. rmmod AC5 driver Done\n");
        }
    }

    diag_esw_remove_pcie_device();

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("4. remove pcie device Done\n");
    }

    /* Put switch/phy in Reset. */
    if (fpga_reset_32_api(FPGA_EXTER_DEV_RST_REG, EXT_ESW_RESET, TRUE,
                          ESW_WAIT_200MS) != PASSED) {

        printf("%s: Failed to put switch in Reset.\n", __FUNCTION__);
        return (FAILED);
    }

    msleep(ESW_WAIT_500MS);

    /* Release switch/phy from Reset. */
    if (fpga_reset_32_api(FPGA_EXTER_DEV_RST_REG, EXT_ESW_RESET, FALSE,
                          ESW_WAIT_200MS) != PASSED) {

        printf("%s: Failed to release switch from Reset.\n", __FUNCTION__);
        return (FAILED);
    }
    msleep(ESW_WAIT_500MS);
    system(PCI_RESCAN);
    msleep(ESW_WAIT_500MS);

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("5. Reset AC5/88E1680 Done\n");
    }


    return (PASSED);

}

/**********************************************************************
 * Function: diag_get_phy_88e1680_obj
 * Description: (1) Get the 88e1680 device driver object
 *              (2) Attach configurations to device object
 * Inputs     : None 
 * Outputs    : address of device object
 **********************************************************************
 */
dev_object_t *diag_get_phy_88e1680_obj (void)
{

    int rc;

    dev_88e1680_object_t *phy_88e1680_obj_p = &phy_88e1680_obj;

    if (dev_88e1680_init == FALSE) {
    
        /* Setup device object base */
        mrv88e1680_dev_create((dev_object_t *)phy_88e1680_obj_p, (dev_error_report_t)err_report);

        /* Setup call-out function vectors */
        phy_88e1680_obj_p->callout_fvt->phy_mad_load_driver = phy_mad_load_driver;
        phy_88e1680_obj_p->callout_fvt->phy_mad_disable_int = phy_mad_disable_int;
        phy_88e1680_obj_p->callout_fvt->phy_mad_set_phy_enable = phy_mad_set_phy_enable;
        phy_88e1680_obj_p->callout_fvt->phy_read_reg = phy_read_reg;
        phy_88e1680_obj_p->callout_fvt->phy_write_reg = phy_write_reg;
        phy_88e1680_obj_p->callout_fvt->phy_mad_display_reg = phy_mad_display_reg;
        phy_88e1680_obj_p->callout_fvt->phy_mad_set_test_mode = phy_mad_set_test_mode;

        /* Attach the device pbject */
        rc = phy_88e1680_obj_p->base.dev_object_fvt->dev_attach((dev_object_t *)phy_88e1680_obj_p);
        if (rc != PASSED) {
            printf("%s:%d:Failed to attach 88e1680 object\n", __func__, __LINE__);
            return (NULL);
        }

        dev_88e1680_init = TRUE;
    }

    return ((dev_object_t *)phy_88e1680_obj_p);

}

/******************************************************************************
 *
 * Function   :	phy_mad_load_driver
 * Description:	load PHY MAD driver
 * Inputs     :	dev - point to MAD_DEV
 *              smi_addr - PHY smi address
 * Outputs    : PASSED(0)/FAILED(1)
 *
 ******************************************************************************
 */
static int phy_mad_load_driver (MAD_DEV *dev, int smi_addr)
{
    MAD_SYS_CONFIG cfg;
    MAD_STATUS status;

    /* clear structures */
    memset((char*)&cfg,0,sizeof(MAD_SYS_CONFIG));
    memset((char*)dev,0,sizeof(MAD_DEV));

    /*
     *  Register all the required functions to MAD driver.
     */
    cfg.BSPFunctions.readMii   = phy_smi_read;
    cfg.BSPFunctions.writeMii  = phy_smi_write;
    
    cfg.BSPFunctions.semCreate = NULL;
    cfg.BSPFunctions.semDelete = NULL;
    cfg.BSPFunctions.semTake   = NULL;
    cfg.BSPFunctions.semGive   = NULL;


    cfg.smiBaseAddr = smi_addr;  /* Set SMI Address */

    if ((status = mdLoadDriver(&cfg, dev)) != PASSED) {

        cterr('f',0,"madLoadDriver return Failed, status = %#x", status);
	    return (status);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Device Name   : %s\n", phy_get_dev_name(dev->deviceId));
        printf("Device ID     : 0x%x\n",dev->deviceId);
        printf("Revision      : 0x%x\n",dev->revision);
        printf("Base Reg Addr : 0x%x\n",dev->baseRegAddr);
        printf("No of Ports   : %d\n",dev->numOfPorts);
        printf("MAD has been started.\n");
    }
    return (PASSED);
}

/******************************************************************************
 *
 * Function   :	phy_smi_read
 * Description:	read PHY registers through SMI interface.
 * Inputs     :	dev - point to MAD_DEV
 *              smi_addr - PHY smi address
 *              reg_addr - PHY register address
 *              rd_data - point to unsigned int which holds regiter value
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static unsigned int phy_smi_read (MAD_DEV *dev, unsigned int smi_addr, unsigned int reg_addr, unsigned int *reg_data)
{
    unsigned int port_num;
    port_num = smi_addr;
	
    if (smi_read_reg(port_num, reg_addr, (unsigned short *)reg_data) == PASSED) {
	    return (MAD_TRUE);
    } else {
	    return (MAD_FALSE);
    }
}

/******************************************************************************
 *
 * Function   :	phy_smi_write
 * Description:	write PHY registers through SMI interface.
 * Inputs     :	dev - point to MAD_DEV
 *              smi_addr - PHY smi address
 *              reg_addr - PHY register address
 *              data - data to write to PHY regiter
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static unsigned int phy_smi_write (MAD_DEV *dev, unsigned int smi_addr, unsigned int reg_addr, unsigned int data)
{
    unsigned int port_num;
    port_num = smi_addr;

    if (smi_write_reg(port_num, reg_addr, (unsigned short)data) == PASSED) {
	    return (MAD_TRUE);
    } else {
	    return (MAD_FALSE);
    }
}

/******************************************************************************
 *
 * Function   :	phy_get_dev_name
 * Description:	get PHY device name from device ID.
 * Inputs     :	device_id
 * Outputs    : device name
 *
 ******************************************************************************
 */
static char * phy_get_dev_name ( MAD_DEVICE_ID device_id)
{
    switch (device_id) {
        case MAD_88E10X0: return ("MAD_88E10X0 ");   	
        case MAD_88E10X0S: return ("MAD_88E10X0S ");   
        case MAD_88E1011: return ("MAD_88E1011 ");   
        case MAD_88E104X: return ("MAD_88E104X ");
        case MAD_88E1111: return ("MAD_88E1111/MAD_88E1115 ");
        case MAD_88E1112: return ("MAD_88E1112 ");
        case MAD_88E1116: return ("MAD_88E1116/MAD_88E1116R ");
        case MAD_88E114X: return ("MAD_88E114X ");
        case MAD_88E1149: return ("MAD_88E1149 ");
        case MAD_88E1149R: return ("MAD_88E1149R ");
        case MAD_SWG65G : return ("MAD_SWG65G ");
        case MAD_88E1181: return ("MAD_88E1181 ");
        case MAD_88E3016: return ("MAD_88E3015/MAD_88E3016/MAD_88E3018/MAD_88E3019");	
        case MAD_88E1121: return ("MAD_88E1121/MAD_88E1121R ");
        case MAD_88E3082: return ("MAD_88E3082/MAD_88E3083 ");
        case MAD_88E1240: return ("MAD_88E1240 ");
        case MAD_88E1340S: return ("MAD_88E1340S ");
        case MAD_88E1340: return ("MAD_88E1340 ");
        case MAD_88E1340M: return ("MAD_88E1340M ");
        case MAD_88E1119R: return ("MAD_88E1119R ");
        case MAD_88E1310: return ("MAD_88E1310 ");
        case MAD_88E1540: return ("MAD_88E1540 ");
        case MAD_88E1548: return ("MAD_88E1548 ");
        case MAD_88E1680: return ("MAD_88E1680 ");	
        case MAD_88E1680M: return ("MAD_88E1680M ");
        case MAD_SW1680: return ("MAD_SW1680 ");
        default : return (" No-name ");
    }
}

/******************************************************************************
 *
 * Function   :	smi_read_reg
 * Description:	read from device register through SMIinterface.
 * Inputs     :	port_num - phy port number
 *              reg_addr - device register address
 *              rd_data - point to unsigned short which holds regiter value
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
int smi_read_reg (unsigned int port_num, unsigned int reg_addr, unsigned short *reg_data)
{
    return (cpssDxChPhyPortSmiRegisterRead(ELIXIR_AC5_CPSS_DEV, port_num, reg_addr, reg_data));
}


/******************************************************************************
 *
 * Function   :	smi_write_reg
 * Description:	write to device register through SMI interface.
 * Inputs     :	port_num - phy port number
 *              reg_addr - device register address
 *              wr_data - write data
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
int smi_write_reg (unsigned int port_num, unsigned int reg_addr, unsigned short wr_data)
{
    return (cpssDxChPhyPortSmiRegisterWrite(ELIXIR_AC5_CPSS_DEV, port_num, reg_addr, wr_data));
}

/******************************************************************************
 *
 * Function   :	phy_mad_disable_int
 * Description:	disable all the interrupt
 * Inputs     :	dev - point to MAD_DEV
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int phy_mad_disable_int (MAD_DEV *dev)
{
    MAD_STATUS status;
    MAD_LPORT port;
    MAD_INT_TYPE int_type;

    /* clear out all int causes */
    memset(&int_type, 0, sizeof(MAD_INT_TYPE));

    for (port = 0; port < dev->numOfPorts; port++) {
        if ((status = mdIntSetEnable(dev,port,&int_type)) != MAD_OK) {
            printf("mdIntSetEnable returned fail.\n");
            return (FAILED);
        }
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: phy_mad_set_phy_enable
 *
 * Description: set phy enable 
 *
 * Input: dev - point to MAD_DEV
 *           port_num - PHY port number 
 *           status - enable/disable
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int phy_mad_set_phy_enable (MAD_DEV *dev, MAD_LPORT port_num, int status)
{
    return (mdSysSetPhyEnable(dev, port_num, status));
}

/*
 **********************************************************************
 *
 *  Function: xcat5_port_phy_1680_init
 *
 *  Description: Config AC5 & 1680 init port setting
 *
 *  Input: None
 *  Outputs  : PASSED/FAILED
 *       
 **********************************************************************
 */
static int xcat5_port_phy_1680_init (void)
{

    int rc = 0;

    dev_98dxc25x_object_t *esw_98dxc25x_obj_p = NULL;
    dev_object_t *dev;

    /* Create 98dxc25x device driver */
    esw_98dxc25x_obj_p = (dev_98dxc25x_object_t *)diag_get_esw_98dxc25x_obj(); 
    if (esw_98dxc25x_obj_p == NULL) {
        cterr('f', 0, "%s: Creating object failed", __func__);
        return (FAILED);
    }
    dev = (dev_object_t *)esw_98dxc25x_obj_p;


    /* Call 98dxc25x xcat5 & phy port init setting function */
    rc = esw_98dxc25x_obj_p->callin_fvt->esw_xcat5_phy_port_init(dev, esw_98dxc25x_obj_p->cpss_dev,
                                                                 ELIXIR_ESW_PORT_NUM);


    if (rc == FAILED) {
        cterr('f',0,"Failed to do AC5 & phy port init setting");
        goto _exit;
    }

    return (PASSED);


 _exit:

    return (FAILED);
    
}

/*******************************************************************************
 *
 * Function   : diag_esw_remove_pcie_device
 * Description: remove the pcie device
 * Inputs     : None
 * Outputs    : None
 *
 *******************************************************************************
 */
void diag_esw_remove_pcie_device (void)
{

    printf("Remove pcie device\n");
    system("echo 1 > /sys/bus/pci/devices/0000:01:00.0/remove");

    msleep(ESW_WAIT_1000MS);

}

/******************************************************************************
 *
 * Function   :	xcat5_reg_pci_read
 * Description: Read memory mapped xCat5 internal registers through 
 *              PCI interface.
 * Inputs     :	cpss_dev - cpss dev number 
 *                  offset - register offset
 *                  *data - pointer to hold the read data 
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
int xcat5_reg_pci_read (uint cpss_dev, uint offset, uint *data)
{
    uint32_t rc;

    rc = cpssDrvPpHwRegisterRead(cpss_dev, XCAT5_PCI_PORT_GROUP, offset, (GT_U32 *)data);

    return (rc);
}

/******************************************************************************
 *
 * Function   :	phy_read_reg
 * Description:	read a PHY register in paged mode.
 * Inputs     :	dev - point to MAD_DEV
 *              port_num - PHY port number to read the register for.
 *              page_num - Page number to be accessed.
 *              reg_addr - The register's address.
 *              data - point to unsigned int which holds regiter value
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int phy_read_reg (MAD_DEV *dev, MAD_LPORT port_num, MAD_U16 page_num, MAD_U16 reg_addr, MAD_U32 *data)
{
    MAD_STATUS status;

    status = mdSysGetPagedPhyReg(dev, port_num, page_num, reg_addr, data);

    if (status == MAD_OK) {
        *data &= 0xffff; 
        return (PASSED);
    } else {
	    return (FAILED);
    }
}

/******************************************************************************
 *
 * Function   :	phy_write_reg
 * Description:	write to a PHY register in paged mode.
 * Inputs     :	dev - point to MAD_DEV
 *              port_num - PHY port number to read the register for.
 *              page_num - Page number to be accessed.
 *              reg_addr - The register's address.
 *              data - write data
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int phy_write_reg (MAD_DEV *dev, MAD_LPORT port_num, MAD_U16 page_num, MAD_U16 reg_addr, MAD_U16 data)
{
    MAD_STATUS status;

    status = mdSysSetPagedPhyReg(dev, port_num, page_num, reg_addr, data);

    if (status == MAD_OK) {
	    return (PASSED);
    } else {
	    return (FAILED);
    }
}

/******************************************************************************
 *
 * Function   :	diag_config_port_speed
 * Description:	Config port speed.
 * Inputs     :	dev_num - device number
 *              port_num - pHY port number
 *              target_speed - config speed
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
int diag_config_port_speed (uint dev_num, uint port_num, uint target_speed) 
{

    uint32_t rc = 0;
    CPSS_PORT_SPEED_ENT speed;
    CPSS_PORTS_BMP_STC initPortsBmp; /* bitmap of ports to init */
    GT_BOOL is_link_up;

    rc = xcat5_port_disable(dev_num, port_num);
    if (rc != PASSED) {
        cterr('f',0,"Error failed xcat5_port_disable 0x%0x, "
		     "dev_num %d, port %d\n", rc, dev_num, port_num);
        return (FAILED);
    }

    if (xcat5_port_force_link_set(dev_num, LINKUP, port_num, FALSE) != PASSED) {
	    cterr('f',0,"Failed xcat5_port_force_link_set()");
	    return (FAILED);
    }

    if (xcat5_port_force_link_set(dev_num, LINKDOWN, port_num, TRUE) != PASSED) {
        cterr('f',0,"Failed xcat5_port_force_link_set()");
        return (FAILED);
    }

    if (target_speed == SPD_1000MBPS) {
        speed = CPSS_PORT_SPEED_1000_E;
    } else if (target_speed == SPD_100MBPS) {
        speed = CPSS_PORT_SPEED_100_E;
    } else if (target_speed == SPD_10MBPS) {
        speed = CPSS_PORT_SPEED_10_E;
    } else {
        cterr('f',0,"Unsupported speed.\n");
        return (FAILED);
    }

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&initPortsBmp);
    CPSS_PORTS_BMP_PORT_SET_MAC(&initPortsBmp, port_num);


    rc = cpssDxChPortDuplexAutoNegEnableSet(dev_num, port_num, GT_FALSE);
    if (rc != GT_OK) {
	    cterr('f',0,"Failed to call cpssDxChPortDuplexAutoNegEnableSet() "
	          "for port %d, err code %d", port_num, rc);
	    return (FAILED);
    }

    rc = cpssDxChPortSpeedAutoNegEnableSet(dev_num, port_num, GT_FALSE);
    if (rc != GT_OK) {
	    cterr('f',0,"Failed to call cpssDxChPortSpeedAutoNegEnableSet() "
	          "for port %d, err code %d", port_num, rc);
	    return (FAILED);
    }

    rc = cpssDxChPortSpeedSet(dev_num, port_num, speed);
    if(rc != GT_OK) {
	    cterr('f',0,"Failed to call cpssDxChPortSpeedSet() for port %d,"
	          " err code %d", port_num, rc);
	    return (FAILED);
    }

    rc = cpssDxChPortDuplexModeSet(dev_num, port_num, CPSS_PORT_FULL_DUPLEX_E);
    if (rc != GT_OK) {
	    cterr('f',0,"Failed to call cpssDxChPortDuplexModeSet() for port %d,"
	          " err code %d", port_num, rc);
        return (FAILED);
    }

    if (xcat5_port_force_link_set(dev_num, LINKDOWN, port_num, FALSE) != PASSED) {
        cterr('f',0,"Failed xcat5_port_force_link_set()");
        return (FAILED);
    }

    rc = cpssDxChPortEnableSet(dev_num, port_num, GT_FALSE);
    if (rc != GT_OK) {
	    cterr('f', 0," Error failed port disable:cpssDxChPortEnableSet err code 0x%0x,"
		      " dev_num %d, port %d\n", rc, dev_num, port_num);
	    return (FAILED);
    }

    rc = cpssDxChPortMacResetStateSet(dev_num, port_num, GT_TRUE);
    if (rc != GT_OK) {
	    cterr('f',0," Error failed port reset:cpssDxChPortMacResetStateSet err 0x%0x,"
		      " dev_num %d, port %d\n", rc, dev_num, port_num);
	    return (FAILED);
    }

    rc = cpssDxChPortMacResetStateSet(dev_num, port_num, GT_FALSE);
    if (rc != GT_OK) {
	    cterr('f',0," Error failed port unreset:cpssDxChPortMacResetStateSet err 0x%0x,"
		      " dev_num %d, port %d\n", rc, dev_num, port_num);
	    return (FAILED);
    }

    rc = cpssDxChPortEnableSet(dev_num, port_num, GT_TRUE);
    if (rc != GT_OK) {
	    cterr('f',0," Error failed port enable:cpssDxChPortEnableSet err code 0x%0x,"
		      " dev_num %d, port %d\n", rc, dev_num, port_num);
	    return (FAILED);
    }

    rc = xcat5_port_enable(dev_num, port_num);
    if (rc != GT_OK) {
	    cterr('f',0,"Error failed xcat5_port_enable 0x%0x, "
		      "dev_num %d, port %d\n", rc, dev_num, port_num);
	    return (FAILED);
    }

    if (xcat5_port_force_link_set(dev_num, LINKUP, port_num, TRUE) != PASSED) {
	    cterr('f',0,"Failed xcat5_port_force_link_set()");
	    return (FAILED);
    }

    if (cpssDxChPortLinkStatusGet(dev_num, port_num, &is_link_up) != OK) {
	    cterr('f',0,"Failed cpssDxChPortLinkStatusGet()");
	    return (FAILED);
    }
    if (is_link_up == GT_FALSE) {
	    cterr('f',0,"Link is not up for port %d", port_num);
	    return (FAILED);
    }

    return (PASSED);

}

/**********************************************************************
 *
 * Function: xcat5_port_disable
 *
 * Description: Set xCat5 port disable
 *
 * Input: dev_num - cpss dev number
 *           port_num - port number
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int xcat5_port_disable (uint dev_num, uint port_num)
{
    uint32_t rc = 0;

    rc = cpssDxChPortEnableSet(dev_num, port_num, 0);
    if (rc != GT_OK) {
        cterr('f',0,"Error failed cpssDxChPortEnableSet");
        return (FAILED);
    }
    return (PASSED);
}

/******************************************************************************
 *
 * Function   :	phy_display_reg
 * Description:	diaplay a page of registers for a specific port.
 * Inputs     :	dev - point to MAD_DEV
 *              port_num - PHY port number to read the register for.
 *              page_num - Page number to be accessed.
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int phy_mad_display_reg (MAD_DEV *dev, MAD_U8 port_num, MAD_U16 page_num)
{
    MAD_STATUS status;
    int ix;
    MAD_U16 data;

    if (dev == 0) {
        printf("MAD driver is not initialized.\n");
        return (FAILED);
    }

    printf("Read PHY port %d page %d : \n", (int)port_num, (int)page_num);

    for (ix = 0; ix < 32; ix++) {
	    if ((status = madHwReadPagedPhyReg(dev, port_num, page_num, ix, &data)) != MAD_OK) {
	        cterr('f',0,"Reading page %d  port %d register %d failed.", 
		      (int)page_num, (int)port_num, ix);
	        return (FAILED);
	    }

	    if ((ix + 1) % 4) {
	        printf("reg %02d: 0x%04x    ", ix, (int)data);
        }
	    else {
	        printf("reg %02d: 0x%04x\n", ix, (int)data);
        }
    }

    printf("\n");
    
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : diag_reset_esw_to_default
 * Description: Function to reset Nanook switch and re-init it.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int diag_reset_esw_to_default (void)
{

    uint rc;

    diag_esw_exit();

    /* Re-Insert PCIE driver */
    system(ETH_INSMOD_AC5_NIM_DM_MODULE);

    rc = diag_ac5_init();

    return (rc);

}

/******************************************************************************
 *
 * Function   :	xcat5_reg_config_write
 * Description: write to memory mapped xcat5 internal registers through 
 *              PCI interface.
 * Inputs     :	cpss_dev - cpss dev number 
 *              offset - register offset
 *              data - data to write 
 * Outputs    : None
 *
 ******************************************************************************
 */
static void xcat5_reg_config_write (uint cpss_dev, uint offset, uint data)
{

    GT_UINTPTR pci_base, internal_base;
    cpss_pcie_get_pciemap_ex(&pci_base, &internal_base);

    *(unsigned int *)(internal_base + offset) = data;

}

/**********************************************************************
 *
 * Function: xcat5_pcs_loopback_enable
 *
 * Description: Set xCat5 pcs loopback enable
 *
 * Input: dev_num - cpss dev number
 *        port_num - port number
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int xcat5_pcs_loopback_enable (uint dev_num, uint port_num)
{
    uint32_t rc = GT_OK;

    rc = cpssDxChPortInternalLoopbackEnableSet(dev_num, port_num, GT_TRUE);
    if (rc != GT_OK) {
        cterr('f',0,"Error failed cpssDxChPortInternalLoopbackEnableSet\n");
    }
    return (rc);
}

/**********************************************************************
 *
 * Function: xcat5_pcs_loopback_disable
 *
 * Description: Set xCat5 pcs loopback disable
 *
 * Input: dev_num - cpss dev number
 *        port_num - port number
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int xcat5_pcs_loopback_disable (uint dev_num, uint port_num)
{
    uint32_t rc = GT_OK;

    rc = cpssDxChPortInternalLoopbackEnableSet(dev_num, port_num, GT_FALSE);
    if (rc != GT_OK) {
        cterr('f',0,"Error failed cpssDxChPortInternalLoopbackEnableSet\n");
    }
    return (rc);
}

/******************************************************************************
 *
 * Function   :	xcat5_pcie_config_read
 * Description: wrapper to read PCIe configuration space register.
 * Inputs     :	offset - register offset
 *              reg_ptr - pointer to hold register data
 * Outputs    : None
 *
 *******************************************************************************
 */
static void xcat5_pcie_config_read (int offset, uint *reg_ptr)
{
    uint32_t bus;
    int device;

    bus = ELIXIR_XCAT_PCIE_BUS;
    device = 0;

    *reg_ptr = pci_domain_config_read(0, ELIXIR_XCAT_PCIE_BUS, device, 0, offset);

}

/******************************************************************************
 *
 * Function   :	xcat5_pcie_config_write
 * Description: wrapper to write PCIe configuration space register.
 * Inputs     :	offset - register offset
 *              reg_data - register data to write
 * Outputs    : None
 *
 ******************************************************************************
 */
static void xcat5_pcie_config_write (int offset, uint reg_data)
{
    uint32_t bus;
    int device;

    bus = ELIXIR_XCAT_PCIE_BUS;
    device = 0;

    pci_domain_config_write(0, ELIXIR_XCAT_PCIE_BUS, device, 0, offset, reg_data);

}

/******************************************************************************
 *
 * Function   :	diag_esw_all_phy_green_led_off
 * Description: perform esw all phy green led off.
 *              
 * Inputs     :	None
 *              
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
int diag_esw_all_phy_green_led_off (void)
{
    int ix;
    MAD_DEV * mad_dev;

    dev_88e1680_object_t *phy_88e1680_obj_p = NULL;
    dev_object_t *dev;


    /* Create 88e1680 device driver */
    phy_88e1680_obj_p = (dev_88e1680_object_t *)diag_get_phy_88e1680_obj(); 
    if (phy_88e1680_obj_p == NULL) {
        cterr('f', 0, "%s: Creating object failed", __func__);
        return (FAILED);
    }
    dev = (dev_object_t *)phy_88e1680_obj_p;

    /* Force 88E1680 phy led off */
    for (ix = 0; ix < ELIXIR_ESW_PORT_NUM; ix++) {
        mad_dev = &phy_mad_88e1680;
        /* Call 88e1680 PHY led off function */
        if (phy_88e1680_obj_p->callin_fvt->led_off(dev, mad_dev, ix) != PASSED) {
            cterr('f',0,"Failed to force led on for phy port %d", ix);
            goto _exit;
        }
    }

    return (PASSED);
	
 _exit:

    return (FAILED);

}

/******************************************************************************
 *
 * Function   :	diag_esw_all_phy_green_led_on
 * Description: perform esw all phy green led on.
 *              
 * Inputs     :	None
 *              
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
int diag_esw_all_phy_green_led_on (void)
{
    int ix;
    MAD_DEV * mad_dev;

    dev_88e1680_object_t *phy_88e1680_obj_p = NULL;
    dev_object_t *dev;


    /* Create 88e1680 device driver */
    phy_88e1680_obj_p = (dev_88e1680_object_t *)diag_get_phy_88e1680_obj(); 
    if (phy_88e1680_obj_p == NULL) {
        cterr('f', 0, "%s: Creating object failed", __func__);
        return (FAILED);
    }
    dev = (dev_object_t *)phy_88e1680_obj_p;

    /* Force 88E1680 phy led on */
    for (ix = 0; ix < ELIXIR_ESW_PORT_NUM; ix++) {
        mad_dev = &phy_mad_88e1680;
        /* Call 88e1680 PHY led on function */
        if (phy_88e1680_obj_p->callin_fvt->led_on(dev, mad_dev, ix) != PASSED) {
            cterr('f',0,"Failed to force led on for phy port %d", ix);
            goto _exit;
        }
    }

    return (PASSED);
	
 _exit:

    return (FAILED);

}

/**********************************************************************
 *
 * Function: phy_mad_set_test_mode
 *
 * Description: This function provides PHY test mode for Marvell GE PHY.
 *
 * Input: dev - point to MAD_DEV
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int phy_mad_set_test_mode (MAD_DEV *dev, uint port, uint test_mode)
{

    MAD_STATUS status;

    assert(dev);

    if (test_mode > 0) {
	    if ((status = mdDiagSetIEEETest(dev, port, ENABLE, test_mode)) != MAD_OK) {
	        cterr('f',0,"mdDiagSetIEEETest returned fail.");
	        return (FAILED);
	    }
    } else {
	/* Go back to normal mode */
	    if ((status = mdDiagSetIEEETest(dev, port, DISABLE, 1)) != MAD_OK) {
	        cterr('f',0,"mdDiagSetIEEETest returned fail.");
	        return (FAILED);
	    }
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: xcat5_exit
 *
 * Description: xCat5 exit
 *
 * Input: cpss_dev - cpss dev number
 *         mad_dev - point to MAD_DEV
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int xcat5_exit (MAD_DEV *mad_dev)
{
    mdUnloadDriver(mad_dev);

    return (PASSED);
} 


/*-------------------------------------------------
 * $Log: diag_esw_lib.c,v $
 * Revision 1.2  2021/09/24 01:21:06  harrchan
 * Collapse Elixir-branch to Main Trunk.
 *
 * Revision 1.1.2.38  2021/05/31 10:45:06  illiu
 * Change return type of some function to void type
 * Rename function diag_esw_all_phy_led_off/on to diag_esw_all_phy_green_led_off/on
 * Fix ESW Reset Default Utility
 *
 * Revision 1.1.2.37  2021/05/05 06:14:58  illiu
 * Add function pci_domain_config_read() and pci_domain_config_write() to fix ESW PCI Config Read/Write Utility
 *
 * Revision 1.1.2.36  2021/04/23 02:41:13  illiu
 * 1. Clean up code
 * 2. Add variable cpss_dev as member of 98dxc25x object
 * 3. Add variable port_group as member of 98dxc25x object
 * 4. Do AC5/1680 register adjustmanet for QSGMII voltage amplitude at the AC5 init process
 *
 * Revision 1.1.2.35  2021/04/12 08:51:42  illiu
 * 1. Move some Marvell library's functions to device driver
 * 2. Replace object-create method as object-get method (Device driver object)
 * 3. Rename nim_dm prefix function to cpss_pcie prefix function
 * 4. Replace sprintf as snprintf
 *
 * Revision 1.1.2.34  2021/03/22 03:24:16  harrchan
 * Add PCIE speed check test in ESW menu
 *
 * Revision 1.1.2.33  2021/03/18 09:16:21  harrchan
 * To check AC5 PCIE was on GEN3 speed
 *
 * Revision 1.1.2.32  2021/03/18 07:52:09  illiu
 * 1. Add call-in function to do AC5 switch exit process
 * 2. Add call-out function to do AC5 switch exit process
 * 3. Add call-in function to do AC5 & PHY init port setting
 * 4. Add call-out function to do AC5 & PHY init port setting
 *
 * Revision 1.1.2.31  2021/03/15 09:51:22  illiu
 * Use macro string to replace magic number
 *
 * Revision 1.1.2.30  2021/03/05 07:15:46  illiu
 * Modify register value to conform HW measurement criterion (QSGMII voltage amplitude)
 *
 * Revision 1.1.2.29  2021/03/03 06:35:21  illiu
 * Modify register value to conform HW measurement criterion (QSGMII voltage amplitude)
 *
 * Revision 1.1.2.28  2021/02/22 02:38:51  illiu
 * Clean up code
 *
 * Revision 1.1.2.27  2021/02/04 09:33:45  illiu
 * Modify Elixir 1680 phy led callin function
 *
 * Revision 1.1.2.26  2021/02/03 02:50:15  illiu
 * Clean up code
 *
 * Revision 1.1.2.25  2021/01/29 09:14:03  illiu
 * Modify wrong port number(Elixir port is only 0-7) in ESW 88E1680 Tx Config Read/Write Utility
 *
 * Revision 1.1.2.24  2021/01/26 03:23:59  illiu
 * Modify include file because of rename nim_dm prefix file
 *
 * Revision 1.1.2.23  2021/01/07 06:19:23  illiu
 * Clean code
 *
 * Revision 1.1.2.22  2020/12/04 08:36:46  illiu
 * Add 1680 PHY Test Mode Utility
 *
 * Revision 1.1.2.21  2020/11/18 08:59:53  harrchan
 * Modify exit process in diag_esw_exit
 *
 * Revision 1.1.2.20  2020/11/12 06:38:43  illiu
 * 1. Add Elixir 1680 phy led features to MB LED test/utility item
 * 2. Add ESW PHY LED Utility
 *
 * Revision 1.1.2.19  2020/11/05 06:34:55  harrchan
 * 1.Base on P1A bring up result to Modify the AC5 MAC/internal/external loopback test
 * 2.Remove some debug message on AC5 init process
 *
 * Revision 1.1.2.18  2020/11/05 03:01:35  illiu
 * Add test item: xCat5 Interrupt Test, PHY Interrupt Test
 *
 * Revision 1.1.2.17  2020/10/26 07:23:04  harrchan
 * *** empty log message ***
 *
 * Revision 1.1.2.16  2020/10/15 12:04:38  illiu
 * 1. Move AC5 switch init and exit process to linux_main.c(It means do init once diag application is actived and do exit once diag application is exit)
 * 2. Add port configuration process for wifi6 module(XCAT5_TO_WIFI_PORT=26) which is connected to AC5 switch
 * 3. Add nim_dm driver polling, to check if driver is ready
 * 4. Add nim_dm driver polling, to check if driver exist before doing insmod or rmmod commend
 * 5. Modify the accessed path of pcie device in diag_esw_remove_pcie_device function
 * 6. Modify marvell_cpssPpInit_xcat5 and phy_dev_88e1680_group_start_addr to be static type variable
 * 7. Move array: phy_dev_88e1680 to header file
 * 8. Remove marvell_ac5_cpss_dev_num_elixir variable, and use ELIXIR_AC5_CPSS_DEV macro directly
 * 9. Modify AC5 switch test item name: External Loopback Test ==> PHY External Loopback Test
 * 10.Remove unneeded variable: port_group, port_group_phy_num
 * 11.Modify code alignment
 *
 * Revision 1.1.2.15  2020/10/07 11:20:35  illiu
 * Clean up code
 *
 * Revision 1.1.2.14  2020/10/07 09:19:25  illiu
 * Clean up code
 *
 * Revision 1.1.2.13  2020/10/06 02:05:56  illiu
 * Transform calling objects from AC3 file/function to AC5 file/finction (dev_98dxc323.c -> dev_98dxc25x.c)
 *
 * Revision 1.1.2.12  2020/09/28 10:31:07  illiu
 * Add below utility items:
 * 1. ESW PHY Register Read Utility
 * 2. ESW PHY Register Write Utility
 * 3. ESW 88E1680 Tx Config Read Utility
 * 4. ESW 88E1680 Tx Config Write Utility
 *
 * Revision 1.1.2.11  2020/09/26 03:32:23  illiu
 * Add below Utilities items:
 *     ESW PCI Config Read Utility
 *     ESW PCI Config Write Utility
 *     ESW xCat3 Internal Register Write Utility
 *     ESW xCat3 PP Register Read Utility
 *     ESW xCat3 PP Register Write Utility
 *     Print All PHY Counter Utility
 *     Clear All PHY Counter Utility
 *     Print xCat3 Counter Utility
 *     Clear xCat3 Counter Utility
 *     ESW Reset Default Utility
 *
 * Revision 1.1.2.10  2020/09/25 10:05:42  harrchan
 * Add test item Mac internal loopback test
 *
 * Revision 1.1.2.9  2020/09/24 09:50:50  illiu
 * 1. Add test item(xCat3 Interrupt Test, PHY Interrupt Test) and its relative function
 * 2. Modify AC3/AC5 Switch init function: diag_ac3_init()
 *
 * Revision 1.1.2.8  2020/09/23 09:41:25  illiu
 * Replace diag_esw_ext_lpbk_test() function with a stub function because Elixir does not use it to do txrx
 *
 * Revision 1.1.2.7  2020/09/21 09:37:42  illiu
 * Add test item(Internal Loopback Test) and its relative function
 *
 * Revision 1.1.2.6  2020/09/17 10:31:34  illiu
 * Move some macro to diag_esw_lib.h
 *
 * Revision 1.1.2.5  2020/09/17 10:07:25  illiu
 * Add 88E1680 PHY init and test item(PHY Register Test)
 *
 * Revision 1.1.2.4  2020/09/15 09:34:47  illiu
 * Fix AC3 switch init and add test item(diag_esw_xcat3_all_register_test)
 *
 * Revision 1.1.2.3  2020/09/14 05:49:44  harrchan
 * Remove DSL and GSHDSL relevant part
 *
 * Revision 1.1.2.2  2020/09/10 09:52:02  illiu
 * Delete 88E6390/88E6176 Switch related code
 *
 * Revision 1.1.2.1  2020/09/09 09:08:06  illiu
 * First version which has been ported with Dreamliner and Marvel CPSS
 *
 * Revision 1.2  2019/01/10 06:36:22  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
