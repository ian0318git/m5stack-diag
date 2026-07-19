/* $Id: dev_98dxc25x.c,v 1.2 2021/09/24 01:22:18 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_98dxc25x_marvell/dev_98dxc25x.c,v $
 *------------------------------------------------------------------
 *
 * Filename:	dev_98dxc25x.c
 *
 * Description: Marvell 98dxc25x ESW Device Driver.
 * Copyright (c) 2019 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */


#include <sys/time.h>
#include "defs.h"
#include "types.h"
#include "queryflags.h"
#include "common.h"
#include "dev_98dxc25x.h"
#include "dev_print.h"
#include "free.h"
#include "proto.h"
#include "common_utils.h"
#include "types.h"
#include "error.h"
#include "nvmonvars.h"
#ifdef LINUX_APP
#include <assert.h>
#endif


static uint32 dev_98dxc25x_attach(dev_object_t *);
static uint32 dev_98dxc25x_detach(dev_object_t *);
static uint32 dev_98dxc25x_restart(dev_object_t *);
static void dev_98dxc25x_destroy(dev_object_t **);
static int dev_98dxc25x_xcat5_init(dev_object_t *, uint);
static int dev_98dxc25x_reg_pci_rd(dev_object_t *, uint, uint, uint *);
static int dev_98dxc25x_reg_pci_wr(dev_object_t *, uint, uint, uint);
static int dev_98dxc25x_reg_config_rd(dev_object_t *, uint, uint, uint *);
static int dev_98dxc25x_reg_config_wr(dev_object_t *, uint, uint, uint);
static int dev_98dxc25x_clear_all_port_interrupt(dev_object_t *, uint);
static int dev_98dxc25x_sw_gpp_init(dev_object_t *, uint);
static int dev_98dxc25x_config_port_pve(dev_object_t *, uint, uint, uint);
static int dev_98dxc25x_unconfig_port_pve(dev_object_t *, uint, uint, uint);
static int dev_98dxc25x_config_port_pve_single_direction(dev_object_t *, uint, uint, uint);
static int dev_98dxc25x_unconfig_port_pve_single_direction(dev_object_t *, uint, uint, uint);
static int dev_98dxc25x_print_mac_counter(dev_object_t *, uint, uint);
static int dev_98dxc25x_port_force_link_set(dev_object_t *, uint, mrvl_98dxc25x_link_status_t, int, boolean);
static int dev_98dxc25x_xcat5_all_reg_test(dev_object_t *, uint);
static int dev_98dxc25x_pcie_config_read_util(dev_object_t *);
static int dev_98dxc25x_pcie_config_write_util(dev_object_t *);
static int dev_98dxc25x_xcat5_reg_read_util(dev_object_t *, uint);
static int dev_98dxc25x_xcat5_reg_write_util(dev_object_t *, uint);
static int dev_98dxc25x_xcat5_internal_reg_read_util(dev_object_t *, uint);
static int dev_98dxc25x_xcat5_internal_reg_write_util(dev_object_t *, uint);
static int dev_98dxc25x_xcat5_enable_force_interrupt(dev_object_t *, uint, uint);
static int dev_98dxc25x_xcat5_gen_int(dev_object_t *, uint);
static int dev_98dxc25x_xcat5_clear_int(dev_object_t *, uint);
static int dev_98dxc25x_led_test(dev_object_t *,  uint);
static int dev_98dxc25x_config_pcs_loopback(dev_object_t *dev, uint, uint);
static int dev_98dxc25x_unconfig_pcs_loopback(dev_object_t *dev, uint, uint);
static int dev_98dxc25x_xcat5_exit(dev_object_t *, uint, MAD_DEV *);
static int dev_98dxc25x_xcat5_phy_port_errata_init(dev_object_t *, uint, uint);

/*===================================================================*
 *                    Polling function                               *
 *===================================================================*/


/*===================================================================*
 *                    Global variables                               *
 *===================================================================*/

static char mrv_98dxc25x_err_buf[MRV98DXC25X_ERR_BUF_SIZE];


/******************************************************************************
 *
 * Name:	mrv98dxc25x_dev_create()
 *
 * Description:	Create object with various device function
 *        		point to "do nothing"
 *
 * Input:	dev_object_t pointer to the 98dxc25x device.
 *		    error reporting function pointer.
 *
 * Returns:	none
 *
 *****************************************************************************/
void mrv98dxc25x_dev_create (dev_object_t *dev, dev_error_report_t error_report_fn)
{
    dev_object_fvt_t	*dev_fvt;
    dev_98dxc25x_object_t *obj_98dxc25x= (dev_98dxc25x_object_t*)dev;

    /* Allocate memory for the device object */
    if ((dev_fvt = (dev_object_fvt_t *)malloc(sizeof(dev_object_fvt_t))) ==
		            NULL) {
        /* Unable to allocate memory */
        error_report_fn(dev, "malloc failure in 98dxc25x_dev_create()", 0);
        printf("%s: NULL\n", __func__);
	    return;
    }

    /* Init the device object structure to default "do nothing" */
    init_default_dev_object(dev, dev_fvt);

    obj_98dxc25x->base.dev_object_fvt->dev_attach	= dev_98dxc25x_attach;
    obj_98dxc25x->base.dev_object_fvt->dev_detach	= dev_98dxc25x_detach;
    obj_98dxc25x->base.dev_object_fvt->dev_restart	= dev_98dxc25x_restart;
    obj_98dxc25x->base.dev_object_fvt->dev_error_report	= error_report_fn;
    obj_98dxc25x->base.dev_object_fvt->dev_destroy	= dev_98dxc25x_destroy;
    obj_98dxc25x->base.dev_object_fvt->dev_name	= "98dxc25x Marvell ESW AlleyCat5";

    obj_98dxc25x->callin_fvt = (dev_98dxc25x_callin_fvt_t *)
                               malloc(sizeof(dev_98dxc25x_callin_fvt_t));
    obj_98dxc25x->callout_fvt = (dev_98dxc25x_callout_fvt_t *)
                                malloc(sizeof(dev_98dxc25x_callout_fvt_t));

    obj_98dxc25x->base.dev_state = DEV_STATE_CREATE;
}


/******************************************************************************
 *
 * Name:	dev_98dxc25x_attach()
 *
 * Description:	Attach the 98dxc25x device for use. This function will
 *		        initialize and setup all necessary pointers and bring the
 *        		chip to operation.
 *
 * Input:	Pointer to the 98dxc25x device object
 *
 * Returns:	PASSED/FAILED
 *
 *****************************************************************************/
static uint32 dev_98dxc25x_attach (dev_object_t *dev)
{
    dev_98dxc25x_object_t *obj_98dxc25x = (dev_98dxc25x_object_t *) dev;

    if (obj_98dxc25x->callin_fvt == NULL) {
        DEV_ERROR_REPORT(dev, "dev_98dxc25x_attach() callin malloc", DEV_98DXC25X_ATTACH);
        return (FAILED);
    }

    if (obj_98dxc25x->callout_fvt == NULL) {
        DEV_ERROR_REPORT(dev, "dev_98dxc25x_attach() callout malloc", DEV_98DXC25X_ATTACH);
        return (FAILED);
    }
	
    obj_98dxc25x->callin_fvt->esw_xcat5_init = dev_98dxc25x_xcat5_init;
    obj_98dxc25x->callin_fvt->esw_clear_all_port_interrupt = dev_98dxc25x_clear_all_port_interrupt;
    obj_98dxc25x->callin_fvt->esw_sw_gpp_init = dev_98dxc25x_sw_gpp_init;
    obj_98dxc25x->callin_fvt->esw_config_port_pve = dev_98dxc25x_config_port_pve;
    obj_98dxc25x->callin_fvt->esw_unconfig_port_pve = dev_98dxc25x_unconfig_port_pve;
    obj_98dxc25x->callin_fvt->esw_config_port_pve_single_direction = dev_98dxc25x_config_port_pve_single_direction;
    obj_98dxc25x->callin_fvt->esw_unconfig_port_pve_single_direction = dev_98dxc25x_unconfig_port_pve_single_direction;
    obj_98dxc25x->callin_fvt->esw_print_mac_counter = dev_98dxc25x_print_mac_counter;
    obj_98dxc25x->callin_fvt->esw_port_force_link_set = dev_98dxc25x_port_force_link_set;
    obj_98dxc25x->callin_fvt->esw_xcat5_all_reg_test = dev_98dxc25x_xcat5_all_reg_test;
    obj_98dxc25x->callin_fvt->esw_pcie_config_read_util = dev_98dxc25x_pcie_config_read_util;
    obj_98dxc25x->callin_fvt->esw_pcie_config_write_util = dev_98dxc25x_pcie_config_write_util;
    obj_98dxc25x->callin_fvt->esw_xcat5_reg_read_util = dev_98dxc25x_xcat5_reg_read_util;
    obj_98dxc25x->callin_fvt->esw_xcat5_reg_write_util = dev_98dxc25x_xcat5_reg_write_util;
    obj_98dxc25x->callin_fvt->esw_xcat5_internal_reg_read_util = dev_98dxc25x_xcat5_internal_reg_read_util;
    obj_98dxc25x->callin_fvt->esw_xcat5_internal_reg_write_util = dev_98dxc25x_xcat5_internal_reg_write_util;
    obj_98dxc25x->callin_fvt->esw_xcat5_gen_int = dev_98dxc25x_xcat5_gen_int;
    obj_98dxc25x->callin_fvt->esw_xcat5_clear_int = dev_98dxc25x_xcat5_clear_int;
    obj_98dxc25x->callin_fvt->esw_xcat5_led_test = dev_98dxc25x_led_test;
    obj_98dxc25x->callin_fvt->esw_config_pcs_loopback = dev_98dxc25x_config_pcs_loopback;
    obj_98dxc25x->callin_fvt->esw_unconfig_pcs_loopback = dev_98dxc25x_unconfig_pcs_loopback;
    obj_98dxc25x->callin_fvt->esw_xcat5_exit = dev_98dxc25x_xcat5_exit;
    obj_98dxc25x->callin_fvt->esw_xcat5_phy_port_init = dev_98dxc25x_xcat5_phy_port_errata_init;
	
    obj_98dxc25x->base.dev_state = DEV_STATE_ATTACH;

    return (PASSED);
}

/******************************************************************************
 *
 * Name:	dev_98dxc25x_detach()
 *
 * Description:	detach the device specific functions from the caller.
 *	        	All of the device specific function are connected to the
 *        		dev_do_nothing() function, except for the dev_attach()
 *        		function. Also, the dev_state must be assigned the value
 *        		of DEV_STATE_DETACH.
 *
 *        		Since, some platforms may want to detach the device, but not
 *        		release the memory resources (via a free () in the
 *        		dev_destroy()), this function can be executed to accomplish
 *        		this task. However, before a detached device can be used again,
 *        		it must be re-attached (via the dev_attach()).
 *
 * Input:	Pointer to the 98dxc25x device object
 *
 * Returns:	PASSED/FAILED
 *
 *****************************************************************************/
static uint32 dev_98dxc25x_detach (dev_object_t *dev)
{
    dev_98dxc25x_object_t *obj_98dxc25x = (dev_98dxc25x_object_t *) dev;

    /* Init the device object structure to default "do nothing" */
    init_default_dev_object(dev, obj_98dxc25x->base.dev_object_fvt);

    obj_98dxc25x->base.dev_state = DEV_STATE_DETACH;

    return (PASSED);

}


/******************************************************************************
 * Name:	dev_98dxc25x_restart
 *
 * Description:	To restart a previously initialized device without resetting
 *		        the device or changing its configuration.
 *		        For example, during a failover event.
 *
 *		        Change the state of the device from its current state
 *		        to an initial state. Also, dev_state must be assigned the
 *		        value of DEV_STATE_INIT.
 *
 * Input:	dev_object_t pointer to the 98dxc25x device
 *
 * Returns:	PASSED/FAILED
 *
 * Assumptions:	The dev_attach() and dev_reconfig_needed() functions has been
 *		        called and successfully executed.
 *
 *****************************************************************************/
static uint32 dev_98dxc25x_restart (dev_object_t *dev)
{
    dev_98dxc25x_object_t *obj_98dxc25x= (dev_98dxc25x_object_t *) dev;

    obj_98dxc25x->base.dev_state = DEV_STATE_INIT;
    return (PASSED);
}


/******************************************************************************
 * Name:	dev_98dxc25x_destroy
 *
 * Description:	Destroy the dev_object structure and free all the resources.
 *
 * Input:	dev_object_t pointer to the 98dxc25x device
 *
 * Returns:	none
 *
 * Assumptions:	The dev_attch() function has been called and successfully
 *
 *****************************************************************************/
static void dev_98dxc25x_destroy (dev_object_t **dev)
{
    dev_98dxc25x_object_t *obj_98dxc25x;

    if (dev == NULL) {
        return;
    }

    if (*dev == NULL) {
        return;
    }

    obj_98dxc25x = (dev_98dxc25x_object_t *)*dev;

    if (obj_98dxc25x->callout_fvt) {
        free(obj_98dxc25x->callout_fvt);	/* Free callout struct */
    }

    if (obj_98dxc25x->callin_fvt) {
        free(obj_98dxc25x->callin_fvt);		/* Free callin struct */
    }

    free(obj_98dxc25x->base.dev_object_fvt);	/* Free dev_object_t */
}


/**********************************************************************
 *
 * Function: dev_98dxc25x_reg_pci_rd
 *
 * This function: Reads Marvell ESW registers through callout
 *
 * Input : dev - dev_object_t pointer to the 98dxc25x device
 *           cpss_dev - cpss device number
 *	       reg - register
 *	       data*  - read-back data
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int dev_98dxc25x_reg_pci_rd (dev_object_t *dev, uint cpss_dev, uint reg, uint *data)
{
     
    int rc;
    dev_98dxc25x_object_t *obj_98dxc25x = (dev_98dxc25x_object_t *) dev;

    rc = DrvPpHwRegisterRead(cpss_dev, obj_98dxc25x->port_group, reg, data); 

    if (rc != PASSED) {
        sprintf(mrv_98dxc25x_err_buf, "%s: Failed (%#x)", __func__, rc);
        return (FAILED);
    }

    return (PASSED);

}


/**********************************************************************
 *
 * Function: dev_98dxc25x_reg_pci_wr
 *
 * This function: writes Marvell ESW registers through callout
 *
 * Input : dev - dev_object_t pointer to the 98dxc25x device
 *           cpss_dev - cpss device number
 *	       reg - register
 *	       data  - write-in data
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int dev_98dxc25x_reg_pci_wr (dev_object_t *dev, uint cpss_dev, uint reg, uint data)
{

    int rc = 0;
    dev_98dxc25x_object_t *obj_98dxc25x = (dev_98dxc25x_object_t *) dev;

    rc = DrvPpHwRegisterWrite(cpss_dev, obj_98dxc25x->port_group, reg, data);

    if (rc != PASSED) {
        sprintf(mrv_98dxc25x_err_buf, "%s: Failed (%#x)", __func__, rc);
        return (FAILED);
    }

    return (PASSED);
    
}


/**********************************************************************
 *
 * Function: dev_98dxc25x_reg_config_rd
 *
 * This function: Reads Marvell ESW registers through callout
 *
 * Input : dev - dev_object_t pointer to the 98dxc25x device
 *         cpss_dev - cpss device number
 *	       reg - register
 *	       data*  - read-back data
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int dev_98dxc25x_reg_config_rd (dev_object_t *dev, uint cpss_dev, uint reg, uint *data)
{
    dev_98dxc25x_object_t *obj_98dxc25x = (dev_98dxc25x_object_t *) dev;
    dev_98dxc25x_callout_fvt_t *callout_p = obj_98dxc25x->callout_fvt;

    (*callout_p->reg_config_rd)(cpss_dev, reg, data);

    return (PASSED);
}


/**********************************************************************
 *
 * Function: dev_98dxc25x_reg_config_wr
 *
 * This function: writes Marvell ESW registers through callout
 *
 * Input : dev - dev_object_t pointer to the 98dxc25x device
 *           cpss_dev - cpss device number
 *	       reg - register
 *	       data  - write-in data
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int dev_98dxc25x_reg_config_wr (dev_object_t *dev, uint cpss_dev, uint reg, uint data)
{
    dev_98dxc25x_object_t *obj_98dxc25x = (dev_98dxc25x_object_t *) dev;
    dev_98dxc25x_callout_fvt_t *callout_p = obj_98dxc25x->callout_fvt;

    (*callout_p->reg_config_wr)(cpss_dev, reg, data);

    return (PASSED);
}


/**********************************************************************
 *
 * Function: dev_98dxc25x_xcat5_init
 *
 * This function: Marvell 98DXC25X xcat5 Initialization
 *
 * Input : dev - dev_object_t pointer to the 98dxc25x device
 *           cpss_dev - cpss device number
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int dev_98dxc25x_xcat5_init (dev_object_t *dev, uint cpss_dev)
{

    int rc;
    dev_98dxc25x_object_t *obj_98dxc25x = (dev_98dxc25x_object_t *) dev;
    dev_98dxc25x_callout_fvt_t *callout_p = obj_98dxc25x->callout_fvt;

    GT_U32 dev_num = cpss_dev;
    CPSS_VERSION_INFO_STC cpssVersion;    
    CPSS_REG_VALUE_INFO_STC *regCfgList; 
    GT_U32 regCfgListSize;       
    CPSS_DXCH_IMPLEMENT_WA_ENT waArr = CPSS_DXCH_IMPLEMENT_WA_SERDES_INTERNAL_REG_ACCESS_E;

    CPSS_DXCH_PP_PHASE1_INIT_INFO_STC xcat5_pp_phase1_info;
    CPSS_DXCH_PP_PHASE1_INIT_INFO_STC *xcat5_pp_phase1_info_ptr = &xcat5_pp_phase1_info;


    CPSS_PP_DEVICE_TYPE xcat5_pp_dev_type;
    CPSS_REG_VALUE_INFO_STC dummyRegValInfoList[] = GT_DUMMY_REG_VAL_INFO_LIST;

    CPSS_DXCH_PP_PHASE2_INIT_INFO_STC xcat5_pp_phase2_info;
    CPSS_DXCH_PP_PHASE2_INIT_INFO_STC *xcat5_pp_phase2_info_ptr = &xcat5_pp_phase2_info;

    CPSS_DXCH_PP_CONFIG_INIT_STC    ppConfig;

    /* Get CPSS version */
    DxChVersionGet(&cpssVersion);

    LogEnableSet();

    rc = (*callout_p->cpss_pp_phase1_info_init)(xcat5_pp_phase1_info_ptr);
    if (rc != PASSED) {
        sprintf(mrv_98dxc25x_err_buf, "%s: Failed (%#x)", __func__, rc);
        return (FAILED);
    }

    /* Initialize the internal DB of CPSS regarding PPs */
    rc = PpInit();
    if (rc != GT_OK) {
        cterr('f',0,"Error failed cpssPpInit Error code 0x%x\n",rc);
        return (FAILED);
    }

    /*********************************************************************/
    /*   HW Phase 1 initialization                                       */
    /*********************************************************************/
    dev_num = xcat5_pp_phase1_info.devNum;
    rc = DxChHwPpPhase1Init(&xcat5_pp_phase1_info, &xcat5_pp_dev_type);
    if ((rc != GT_OK) && (rc != GT_ALREADY_EXIST)) {
        cterr('f',0,"Failed cpssDxChHwPpPhase1Init(), error code 0x%x\n", rc);
        return (FAILED);
    }

    rc = DxChHwPpImplementWaInit(dev_num, &waArr);
    if (rc) {
	    cterr('f',0,"Failed cpssDxChHwPpImplementWaInit(), error code 0x%x\n", rc);
        return (FAILED);
    }

    /* Config functions for this board */
    regCfgList     = dummyRegValInfoList;
    regCfgListSize = (sizeof(dummyRegValInfoList) / sizeof(CPSS_REG_VALUE_INFO_STC));

    /* Set PP's registers */
    rc = DxChHwPpStartInit(dev_num, regCfgList, regCfgListSize);

    if (rc != GT_OK) {
	    cterr('f',0,"Failed hwPpStartInit(), error code 0x%x\n", rc);
	    return (FAILED);
    }

    rc = (*callout_p->cpss_pp_phase2_info_init)(dev_num, xcat5_pp_phase2_info_ptr);
    if (rc != PASSED) {
        sprintf(mrv_98dxc25x_err_buf, "%s: Failed (%#x)", __func__, rc);
        return (FAILED);
    }

    rc = DxChHwPpPhase2Init(dev_num, &xcat5_pp_phase2_info);
    if (rc != GT_OK) {
        cterr('f',0,"Failed cpssDxChHwPpPhase2Init(), error code 0x%x\n", rc);  
        return (FAILED);
    }

    /*********************************************************************/
    /*   Logical phase initialization                                      */
    /*********************************************************************/
    dev_num = cpss_dev;
    ppConfig.routingMode = CPSS_DXCH_POLICY_BASED_ROUTING_ONLY_E;
    rc = DxChCfgPpLogicalInit(dev_num, &ppConfig);
    if (rc != GT_OK) {
        cterr('f',0,"Failed cpssDxChCfgPpLogicalInit(),  rc %d\n", rc);  
        return (rc);
    }

    /*********************************************************************/
    /*   FDB initialization                                              */
    /*********************************************************************/
    rc = DxChBrgFdbInit(dev_num);
    if (rc != GT_OK) {
        cterr('f',0,"Failed cpssDxChBrgFdbInit() rc %d\n", rc);  
        return (rc);
    }

    /********** Set FDB hash function mode */
    rc = DxChBrgFdbHashModeSet(dev_num);
    if (rc != GT_OK) {
        cterr('f', 0,"Failed cpssDxChBrgFdbHashModeSet(),  rc %d\n", rc);
        return (rc);
    }

    /********** Set the VLAN lookup mode */
    rc = DxChBrgFdbMacVlanLookupModeSet(dev_num);
    if (rc != GT_OK) {
        printf("Failed cpssDxChBrgFdbMacVlanLookupModeSet()\n");
        return (rc);
    }

    /********** VLAN Initialization-- set VLAN-aware mode */
    rc = DxChBrgVlanBridgingModeSet(dev_num);
    if (rc != GT_OK) {
        cterr('f',0,"Failed cpssDxChBrgVlanBridgingModeSet()\n");  
        return (rc);
    } 
    /********** Port Initialization */
    rc = DxChPortStatInit(dev_num);
    if (rc != GT_OK) {
        cterr('f',0,"Failed cpssDxChPortStatInit()\n");  
        return (rc);
    }

    /********** Port Configuration */
    rc = (*callout_p->xcat5_specific_port_init)(dev_num);
    if (rc != GT_OK) {
        sprintf(mrv_98dxc25x_err_buf, "%s: Failed (%#x)", __func__, rc);
        return (FAILED);
    }

    /********** Enable ports */
    rc = (*callout_p->xcat5_specific_port_enable)(dev_num);
    if (rc != GT_OK) {
        sprintf(mrv_98dxc25x_err_buf, "%s: Failed (%#x)", __func__, rc);
        return (FAILED);
    }

    /********** Enable device */
    rc = DxChCfgDevEnable(dev_num);
    if(rc != GT_OK) {
        cterr('f',0,"cpssDxChCfgDevEnable() failed, rc 0x%x\n",rc);
        return (rc);
    }

    rc = (*callout_p->smi_phy_init)(cpss_dev);
    if (rc != PASSED) {
        sprintf(mrv_98dxc25x_err_buf, "%s: Failed (%#x)", __func__, rc);
        return (FAILED);
    }

    return (PASSED);

}

/******************************************************************************
 *
 * Function:    dev_98dxc25x_clear_port_interrupt
 *
 * Description: 98dxc25x clear all port interrupt
 *
 * Inputs:  dev_object_t pointer to the 98dxc25x device
 *             cpss_dev - cpss device number
 *
 * Outputs: PASSED - No errors encounterd.
 *          FAILED - Errors encounterd.
 *
 * Assumptions: create and dev_attach have to be called first. dev_destroy will
 *              also be called after the exit.
 *
 *****************************************************************************/
static int dev_98dxc25x_clear_port_interrupt(dev_object_t *dev, uint cpss_dev, uint32_t port_num)
{
    uint32_t port_irupt_cause_reg;
    uint32_t value;
   
    port_irupt_cause_reg = PORT_0_IRUPT_CAUSE_REGISTER +
	                  PORT_IRUPT_OFFSET * port_num;
    return (dev_98dxc25x_reg_pci_rd(dev, cpss_dev, port_irupt_cause_reg, &value));
	
}


/******************************************************************************
 *
 * Function:    dev_98dxc25x_clear_all_port_interrupt
 *
 * Description: 98dxc25x clear all port interrupt
 *
 * Inputs:  dev_object_t pointer to the 98dxc25x device
 *             cpss_dev - cpss device number
 *
 * Outputs: PASSED - No errors encounterd.
 *          FAILED - Errors encounterd.
 *
 * Assumptions: create and dev_attach have to be called first. dev_destroy will
 *              also be called after the exit.
 *
 *****************************************************************************/
static int dev_98dxc25x_clear_all_port_interrupt(dev_object_t *dev, uint cpss_dev)
{
    uint32_t port_num;
    
    for (port_num = 0; port_num < XCAT5_PORT_NUM; port_num ++) {
	    if (dev_98dxc25x_clear_port_interrupt(dev, cpss_dev, port_num) != PASSED) {
	        cterr('f',0,"Failed to clear interrupt for port %d", port_num);
	        return (FAILED);
	    }
    }
    
    return (PASSED);
}


/******************************************************************************
 *
 * Function:    dev_98dxc25x_sw_gpp_init
 *
 * Description: 98dxc25x GPP gpio init
 *
 * Inputs:  dev_object_t pointer to the 98dxc25x device
 *             cpss_dev - cpss device number
 *
 * Outputs: PASSED - No errors encounterd.
 *          FAILED - Errors encounterd.
 *
 * Assumptions: create and dev_attach have to be called first. dev_destroy will
 *              also be called after the exit.
 *
 *****************************************************************************/
static int dev_98dxc25x_sw_gpp_init(dev_object_t *dev, uint cpss_dev)
{
    uint32_t value = 0;

    return (dev_98dxc25x_reg_pci_wr(dev, cpss_dev, GPP_IO_CTRL_REG_OFFSET, value));
}


/**********************************************************************
 *
 * Function: dev_98dxc25x_config_port_pve
 *
 * This function: configure port pve
 *
 * Input : dev_object_t - pointer to the 98dxc25x device. 
 *            cpss_dev - cpss device number
 *            src_port - source port
 *            dst_port - destination port
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int dev_98dxc25x_config_port_pve (dev_object_t *dev, uint cpss_dev, uint src_port, uint dst_port)
{

    uint32_t rc = 0;
    dev_98dxc25x_object_t *obj_98dxc25x = (dev_98dxc25x_object_t *) dev;
    dev_98dxc25x_callout_fvt_t *callout_p = obj_98dxc25x->callout_fvt;


    rc  = DxChBrgPrvEdgeVlanEnable(cpss_dev);
    if (rc) {
        cterr('f',0,"Error from cpssDxChBrgPrvEdgeVlanEnable(), rc = 0x%x, ", rc);
        return (rc);
    }

    rc = (*callout_p->port_enable)(cpss_dev, src_port);
    if (rc) {
        cterr('f',0,"Failed to enable src_port &d\n", src_port);
        return (rc);
    }

    rc = (*callout_p->port_enable)(cpss_dev, dst_port);
    if (rc) {
        cterr('f',0,"Failed to enable dst_port &d\n", dst_port);
        return (rc);
    }

    rc = DxChBrgPrvEdgeVlanPortEnable(cpss_dev,
                                      src_port,
                                      dst_port);
    if (rc) {
        cterr('f',0,"Error from cpssDxChBrgPrvEdgeVlanPortEnable(), rc = 0x%x\n", rc);
        return (rc);
    }


    rc = DxChBrgPrvEdgeVlanPortEnable(cpss_dev,
                                      dst_port,
                                      src_port);
    if (rc) {
        cterr('f',0,"Error from cpssDxChBrgPrvEdgeVlanPortEnable(), rc = 0x%x\n", rc);
        return (rc);
    }

    return (PASSED);

}

/**********************************************************************
 *
 * Function: dev_98dxc25x_unconfig_port_pve
 *
 * This function: unconfigure port pve
 *
 * Input : dev_object_t - pointer to the 98dxc25x device. 
 *            cpss_dev - cpss device number
 *            src_port - source port
 *            dst_port - destination port
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int dev_98dxc25x_unconfig_port_pve (dev_object_t *dev, uint cpss_dev, uint src_port, uint dst_port)
{

    uint32_t rc = 0;

    rc = DxChBrgPrvEdgeVlanDisable(cpss_dev);
    if (rc) {
        cterr('f',0,"Error from cpssDxChBrgPrvEdgeVlanEnable(), rc = 0x%x\n", rc);
        return (rc);
    }
    
    rc = DxChBrgPrvEdgeVlanPortDisable(cpss_dev,
                                       src_port,
                                       dst_port);

    if (rc) {
        cterr('f',0,"Error from cpssDxChBrgPrvEdgeVlanPortEnable(), rc = 0x%x\n", rc);
        return (rc);
    }

    rc = DxChBrgPrvEdgeVlanPortDisable(cpss_dev,
                                       dst_port,
                                       src_port);

    if (rc) {
	    cterr('f',0,"Error from cpssDxChBrgPrvEdgeVlanPortEnable(), rc = 0x%x\n", rc);
        return (rc);
    }

    return (PASSED);
    
}


/**********************************************************************
 *
 * Function: dev_98dxc25x_config_port_pve_single_direction
 *
 * This function: configure port pve for single direction
 *
 * Input : dev_object_t - pointer to the 98dxc25x device. 
 *            cpss_dev - cpss device number
 *            src_port - source port
 *            dst_port - destination port
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int dev_98dxc25x_config_port_pve_single_direction (dev_object_t *dev, uint cpss_dev, uint src_port, uint dst_port)
{
    uint32_t rc = 0;

    dev_98dxc25x_object_t *obj_98dxc25x = (dev_98dxc25x_object_t *) dev;
    dev_98dxc25x_callout_fvt_t *callout_p = obj_98dxc25x->callout_fvt;

    rc = (*callout_p->global_enable_pve)(cpss_dev);
    if (rc) {
        cterr('f',0,"Failed to enable global PVE, rc = %#x\n", rc);
        return (rc);
    }

    rc = (*callout_p->port_enable)(cpss_dev, src_port);
    if (rc) {
        cterr('f',0,"Failed to enable src_port &d\n", src_port);
        return (rc);
    }

    rc = (*callout_p->port_enable)(cpss_dev, dst_port);
    if (rc) {
        cterr('f',0,"Failed to enable dst_port &d\n", dst_port);
        return (rc);
    }

    rc = (*callout_p->set_port_pve_singel_direction)(cpss_dev, src_port, dst_port);
    if (rc) {
        cterr('f',0,"Failed to set PVE for ports %d and %d.\n", src_port, dst_port);
        return (rc);
    }
    return (PASSED);
}


/**********************************************************************
 *
 * Function: dev_98dxc25x_unconfig_port_pve_single_direction
 *
 * This function: unconfigure port pve fore single direction
 *
 * Input : dev_object_t - pointer to the 98dxc25x device. 
 *            cpss_dev - cpss device number
 *            src_port - source port
 *            dst_port - destination port
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int dev_98dxc25x_unconfig_port_pve_single_direction (dev_object_t *dev, uint cpss_dev, uint src_port, uint dst_port)
{
    uint32_t rc = 0;

    dev_98dxc25x_object_t *obj_98dxc25x = (dev_98dxc25x_object_t *) dev;
    dev_98dxc25x_callout_fvt_t *callout_p = obj_98dxc25x->callout_fvt;

    rc = (*callout_p->global_disable_pve)(cpss_dev);
    if (rc) {
        cterr('f',0,"Failed to disable global PVE, rc = %#x\n", rc);
        return (rc);
    }

    rc = (*callout_p->clear_port_pve_singel_direction)(cpss_dev, src_port, dst_port);
    if (rc) {
        cterr('f',0,"Failed to clear PVE for ports %d and %d.\n", src_port, dst_port);
        return (rc);
    }
    return (PASSED);
}


/**********************************************************************
 *
 * Function: dev_98dxc25x_print_mac_counter
 *
 * This function: print ac5 mac counter
 *
 * Input : dev_object_t - pointer to the 98dxc25x device. 
 *            cpss_dev - cpss device number
 *            port_num - number of ports
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int dev_98dxc25x_print_mac_counter (dev_object_t *dev, uint cpss_dev, uint port_num)
{

    GT_STATUS rc;
    CPSS_PORT_MAC_COUNTER_SET_STC portMacCounterSetArray;


    rc = DxChPortMacCountersOnPortGet(cpss_dev, port_num, &portMacCounterSetArray);
    if (rc != GT_OK) {
        cterr('f',0,"Error failed cpssDxChPortMacCountersOnPortGet, Failed print port%d mac counter.\n", port_num);
        return (FAILED);
    }

	printf("counter for GE port %d\n", port_num);
	printf("Good Octets Received: 0x%x\n", portMacCounterSetArray.goodOctetsRcv.l[0]);
	printf("badOctetsRcv: 0x%x\n", portMacCounterSetArray.badOctetsRcv.l[0]);
	printf("macTransmitErr: 0x%x\n", portMacCounterSetArray.macTransmitErr.l[0]);
	printf("goodPktsRcv: 0x%x\n", portMacCounterSetArray.goodPktsRcv.l[0]);
	printf("badPktsRcv: 0x%x\n", portMacCounterSetArray.badPktsRcv.l[0]);
	printf("brdcPktsRcv: 0x%x\n", portMacCounterSetArray.brdcPktsRcv.l[0]);
	printf("mcPktsRcv: 0x%x\n", portMacCounterSetArray.mcPktsRcv.l[0]);
	printf("goodOctetsSent: 0x%x\n", portMacCounterSetArray.goodOctetsSent.l[0]);
	printf("excessiveCollisions: 0x%x\n", portMacCounterSetArray.excessiveCollisions.l[0]);
	printf("brdcPktsSent: 0x%x\n", portMacCounterSetArray.brdcPktsSent.l[0]);
	printf("\n");

    return (PASSED);
}

/**********************************************************************
 *
 * Function: dev_98dxc25x_unconfig_port_pve
 *
 * This function: port force link set
 *
 * Input : dev - dev_object_t pointer to the 98dxc25x device
 *            cpss_dev - cpss device number
 *            link - link status
 *            port - port number
 *            set - enable/disable
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int dev_98dxc25x_port_force_link_set (dev_object_t *dev, uint cpss_dev, mrvl_98dxc25x_link_status_t link, int port, boolean set)
{

    dev_98dxc25x_object_t *obj_98dxc25x = (dev_98dxc25x_object_t *) dev;
    dev_98dxc25x_callout_fvt_t *callout_p = obj_98dxc25x->callout_fvt;
	
    int rc;

    if (link == LINK_DOWN) {

	    rc = (*callout_p->force_link_down_en)(cpss_dev, port, set);
        if (rc) {
            printf("Failed callout force_link_down_en call, rc = %x", rc);
	        return (rc);
        }
	   
    } else {

        rc = (*callout_p->force_link_pass_en)(cpss_dev, port, set);
        if (rc) {
            printf("Failed callout force_link_pass_en call, rc = %x", rc);
	        return (rc);
        }
    }	
	
    return (PASSED);
}


/******************************************************************************
 *
 * Function   :	dev_98dxc25x_all_reg_test
 *
 * Description:	Test xCat5 all internal registers by calling CPSS API.
 *              Original value is restored after test completes.
 *
 * Inputs     :	dev_object_t - pointer to the 98dxc25x device. 
 *                  cpss_dev - cpss device number
 *
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int dev_98dxc25x_xcat5_all_reg_test (dev_object_t *dev, uint cpss_dev)
{

    int rc = 0;
    uint badReg = 0;
    uint readVal = 0;
    uint writeVal = 0;
    uint testStatus = 0;


    rc  = DxChDiagAllRegTest(cpss_dev,
                             &testStatus,
                             &badReg,
                             &readVal,
                             &writeVal);
    if (rc) {
        cterr('f', 0, "Error from cpssDxChDiagAllRegTest(), rc = 0x%x\n", rc);
    }

    if (rc == PASSED) {
        if (testStatus == FALSE) {
            cterr('f', 0, "xCat5 switch all register test failed.\n"
		          "Reg 0x%08x, readVal 0x%08x, writeVal 0x%08x\n",
		          badReg, readVal, writeVal);
            rc = FAILED;
        } else {
            rc = PASSED;
        }
    } else {
        cterr('f', 0, "Error returned by cpssDxChDiagAllRegTest(), rc 0x%x\n", rc);        
    }

    return (rc);

}


/******************************************************************************
 *
 * Function   :	dev_98dxc25x__pcie_config_read_util
 *
 * Description:	Utility to read PCIe configuration space register.
 *
 * Inputs     :	dev_object_t - pointer to the 98dxc25x device. 
 *
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int dev_98dxc25x_pcie_config_read_util (dev_object_t *dev)
{
    uint reg_val = 0, reg_addr = 0;

    dev_98dxc25x_object_t *obj_98dxc25x = (dev_98dxc25x_object_t *) dev;
    dev_98dxc25x_callout_fvt_t *callout_p = obj_98dxc25x->callout_fvt;

    reg_addr = gethex_answer("Enter the address:", 0, 0, 0xFFFFF);

    (*callout_p->pcie_config_read)(reg_addr, (uint *)&reg_val);

    printf("\n\n Value of 0x%05X is 0x%08X.\n\n", reg_addr, reg_val);

    return (PASSED);
}


/******************************************************************************
 *
 * Function   :	dev_98dxc25x_pcie_config_write_util
 *
 * Description:	Utility to write PCIe configuration space register.
 *
 * Inputs     :	dev_object_t - pointer to the 98dxc25x device. 
 *
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int dev_98dxc25x_pcie_config_write_util (dev_object_t *dev)
{
    uint32_t reg_addr = 0, write_in = 0;

    dev_98dxc25x_object_t *obj_98dxc25x = (dev_98dxc25x_object_t *) dev;
    dev_98dxc25x_callout_fvt_t *callout_p = obj_98dxc25x->callout_fvt;

    reg_addr = gethex_answer("Enter the address:", 0, 0, 0xFFFFF);
    write_in = gethex_answer("Enter the data:", 0, 0, 0xFFFFFFFF);

    (*callout_p->pcie_config_write)(reg_addr, write_in);
    printf("\n\nDone.\n");

    return (PASSED);
}


/******************************************************************************
 *
 * Function   :	dev_98dxc25x_xcat5_reg_read_util
 *
 * Description:	Utility to read xCat3 memory mapped registers.
 *
 * Inputs     :	dev_object_t - pointer to the 98dxc25x device.
 *              cpss_dev - cpss device number
 *
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int dev_98dxc25x_xcat5_reg_read_util (dev_object_t *dev, uint cpss_dev)
{
    uint32_t reg_val, reg_addr;

    reg_addr = gethex_answer("Enter the register offset:", 0, 0, 0xFFFFFFFF);
 
    dev_98dxc25x_reg_pci_rd(dev, cpss_dev, reg_addr, (uint32_t *)&reg_val);    

    printf("\n register at offset %#x is %#x.\n", reg_addr, reg_val);

    return (PASSED);
}


/******************************************************************************
 *
 * Function   :	dev_98dxc25x_xcat5_reg_write_util
 *
 * Description: Utility to write xCat5 memory mapped registers.
 *
 * Inputs     :	dev_object_t - pointer to the 98dxc25x device.
 *                  cpss_dev - cpss device number
 *
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int dev_98dxc25x_xcat5_reg_write_util (dev_object_t *dev, uint cpss_dev)
{
    uint32_t reg_addr, write_in;

    reg_addr = gethex_answer("Enter the register offset:", 0, 0, 0xFFFFFFFF);
    write_in = gethex_answer("Enter the register data:", 0, 0, 0xFFFFFFFF);

    dev_98dxc25x_reg_pci_wr(dev, cpss_dev, reg_addr, write_in);
    printf("\n\nDone.\n");

    return (PASSED);
}


/******************************************************************************
 *
 * Function   :	dev_98dxc25x_xcat5_internal_reg_read_util
 *
 * Description:	Utility to read xCat5 memory mapped registers.
 *
 * Inputs     :	dev_object_t - pointer to the 98dxc25x device.
 *              cpss_dev     - cpss device number
 *
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int dev_98dxc25x_xcat5_internal_reg_read_util (dev_object_t *dev, uint cpss_dev)
{
    uint32_t reg_val, reg_addr;

    reg_addr = gethex_answer("Enter the register offset:", 0, 0, 0xFFFFFFFF);
 
    dev_98dxc25x_reg_config_rd(dev, cpss_dev, reg_addr, (uint32_t *)&reg_val);    

    printf("\n register at offset %#x is %#x.\n", reg_addr, reg_val);

    return (PASSED);
}


/******************************************************************************
 *
 * Function   :	dev_98dxc25x_xcat5_internal_reg_write_util
 *
 * Description: Utility to write xCat5 memory mapped registers.
 *
 * Inputs     :	dev_object_t - pointer to the 98dxc25x device.
 *              cpss_dev     - cpss device number
 *
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int dev_98dxc25x_xcat5_internal_reg_write_util (dev_object_t *dev, uint cpss_dev)
{
    uint32_t reg_addr, write_in;

    reg_addr = gethex_answer("Enter the register offset:", 0, 0, 0xFFFFFFFF);
    write_in = gethex_answer("Enter the register data:", 0, 0, 0xFFFFFFFF);

    dev_98dxc25x_reg_config_wr(dev, cpss_dev, reg_addr, write_in);
    printf("\n\nDone.\n");

    return (PASSED);
}


/******************************************************************************
 *
 * Function   :	dev_98dxc25x_xcat5_enable_force_interrupt
 *
 * Description: xCat5 interrupt test.
 *
 * Inputs     :	dev_object_t - pointer to the 98dxc25x device.
 *                  cpss_dev - cpss device number
 *            enable - enable / disable
 *
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int dev_98dxc25x_xcat5_enable_force_interrupt(dev_object_t *dev, uint cpss_dev, uint enable)
{

    uint32_t reg_val, reg_addr;

    if (enable == DEV_98DXC25X_ENABLE) {

        /* Set DATAOUT_ENABLE_CONTROL_REG to enable MPP25 can output signal */ 
        reg_addr = DATAOUT_ENABLE_CONTROL_REG;
        reg_val = DATAOUT_ENABLE;
        dev_98dxc25x_reg_pci_wr(dev, cpss_dev, reg_addr, reg_val);

        /* MPP25 output high signal*/
        reg_addr = DATAOUT_REG;
        reg_val = MPP25_HIGH;
        dev_98dxc25x_reg_pci_wr(dev, cpss_dev, reg_addr, reg_val);

    } 
    else {

        /* MPP25 output low signal*/
        reg_addr = DATAOUT_REG;
        reg_val = MPP25_LOW;
        dev_98dxc25x_reg_pci_wr(dev, cpss_dev, reg_addr, reg_val);

        /* Set DATAOUT_ENABLE_CONTROL_REG to disable the output-data ability of MPP25 */ 
        reg_addr = DATAOUT_ENABLE_CONTROL_REG;
        reg_val = DATAOUT_DISABLE;
        dev_98dxc25x_reg_pci_wr(dev, cpss_dev, reg_addr, reg_val);    

    }
	
    return (PASSED);
}


/**********************************************************************
 *
 * Function: dev_98dxc25x_xcat5_gen_int
 *
 * This function: Marvell 98dxc25x generate interrupt
 *
 * Inputs     :	dev_object_t - pointer to the 98dxc25x device.
 *                  cpss_dev - cpss device number
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int dev_98dxc25x_xcat5_gen_int (dev_object_t *dev, uint cpss_dev)
{

    uint rc; 

    rc = dev_98dxc25x_xcat5_enable_force_interrupt(dev, cpss_dev, DEV_98DXC25X_ENABLE);
    return (rc);
    
}


/**********************************************************************
 *
 * Function: dev_98dxc25x_xcat5_clear_int
 *
 * This function: Marvell 98dxc25x clear interrupt
 *
 * Inputs     :	dev_object_t - pointer to the 98dxc25x device.
 *                  cpss_dev - cpss device number
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int dev_98dxc25x_xcat5_clear_int (dev_object_t *dev, uint cpss_dev)
{

    uint rc; 

    rc = dev_98dxc25x_xcat5_enable_force_interrupt(dev, cpss_dev, DEV_98DXC25X_DISABLE);
    return (rc);
    
}


/******************************************************************************
 *
 * Function   :	dev_98dxc25x_led_test
 *
 * Description: xCat5 LED test
 *
 * Inputs     :	dev_object_t - pointer to the 98dxc25x device. 
 *                  cpss_dev - cpss device number
 *
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int dev_98dxc25x_led_test (dev_object_t *dev,  uint cpss_dev)
{

    uint force_data;

    dev_98dxc25x_object_t *obj_98dxc25x = (dev_98dxc25x_object_t *) dev;
    dev_98dxc25x_callout_fvt_t *callout_p = obj_98dxc25x->callout_fvt;

    force_data = 0xff;

    /* turn all LEDs off */
    if ((*callout_p->led_class_config)(cpss_dev, 0, FALSE, FALSE, 0, TRUE, 0)) {
	    cterr('f',0,"Failed to configure LED class 0");
	    return (FAILED);
    }

    if ((*callout_p->led_class_config)(cpss_dev, 1, FALSE, FALSE, 0, TRUE, 0)) {
	    cterr('f',0,"Failed to configure LED class 1");
	    return (FAILED);
    }

    if ((*callout_p->led_class_config)(cpss_dev, 3, FALSE, FALSE, 0, TRUE, 0)) {
	    cterr('f',0,"Failed to configure LED class 3");
	    return (FAILED);
    }

    if ((*callout_p->led_class_config)(cpss_dev, 4, FALSE, FALSE, 0, TRUE, 0)) {
	    cterr('f',0,"Failed to configure LED class 4");
	    return (FAILED);
    }

    msleep(XCAT5_200MS);

    /* turn Link LED on */
    if ((*callout_p->led_class_config)(cpss_dev, 3, FALSE, FALSE, 0, TRUE, 0)) {
	    cterr('f',0,"Failed to configure LED class 3");
	    return (FAILED);
    }

    if ((*callout_p->led_class_config)(cpss_dev, 4, FALSE, FALSE, 0, TRUE, force_data)) {
	    cterr('f',0,"Failed to configure LED class 4");
	    return (FAILED);
    }

    msleep(XCAT5_1000MS);
	
    /* check activity blink LED */
    if ((*callout_p->led_class_config)(cpss_dev, 3, FALSE, FALSE, 0, TRUE, 0)) {
	    cterr('f',0,"Failed to configure LED class 3");
	    return (FAILED);
    }

    if ((*callout_p->led_class_config)(cpss_dev, 4, FALSE, FALSE, 0, TRUE, force_data)) {
	    cterr('f',0,"Failed to configure LED class 4");
	    return (FAILED);
    }
    msleep(XCAT5_1000MS);


    /* turn all LEDs off */
    if ((*callout_p->led_class_config)(cpss_dev, 0, FALSE, FALSE, 0, TRUE, 0)) {
	    cterr('f',0,"Failed to configure LED class 0");
	    return (FAILED);
    }

    if ((*callout_p->led_class_config)(cpss_dev, 1, FALSE, FALSE, 0, TRUE, 0)) {
	    cterr('f',0,"Failed to configure LED class 1");
	    return (FAILED);
    }

    if ((*callout_p->led_class_config)(cpss_dev, 3, FALSE, FALSE, 0, TRUE, 0)) {
	    cterr('f',0,"Failed to configure LED class 3");
	    return (FAILED);
    }

    if ((*callout_p->led_class_config)(cpss_dev, 4, FALSE, FALSE, 0, TRUE, 0)) {
	    cterr('f',0,"Failed to configure LED class 4");
	    return (FAILED);
    }

    msleep(XCAT5_200MS);

    return (PASSED);
	
}

/**********************************************************************
 *
 * Function: dev_98dxc25x_config_pcs_loopback
 *
 * This function: configure pcs loopback
 *
 * Input : dev_object_t - pointer to the 98dxc25x device. 
 *            cpss_dev - cpss device number
 *            port - MAC port
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int dev_98dxc25x_config_pcs_loopback (dev_object_t *dev, uint cpss_dev, uint port)
{
    uint32_t rc = 0;

    dev_98dxc25x_object_t *obj_98dxc25x = (dev_98dxc25x_object_t *) dev;
    dev_98dxc25x_callout_fvt_t *callout_p = obj_98dxc25x->callout_fvt;

    rc = (*callout_p->pcs_loopback_enable)(cpss_dev, port);
    if (rc) {
        cterr('f',0,"Failed to enable global PVE, rc = %#x\n", rc);
        return (rc);
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: dev_98dxc25x_config_pcs_loopback
 *
 * This function: configure pcs loopback
 *
 * Input : dev_object_t - pointer to the 98dxc25x device. 
 *            cpss_dev - cpss device number
 *            port - MAC port
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int dev_98dxc25x_unconfig_pcs_loopback (dev_object_t *dev, uint cpss_dev, uint port)
{
    uint32_t rc = 0;

    dev_98dxc25x_object_t *obj_98dxc25x = (dev_98dxc25x_object_t *) dev;
    dev_98dxc25x_callout_fvt_t *callout_p = obj_98dxc25x->callout_fvt;

    rc = (*callout_p->pcs_loopback_disable)(cpss_dev, port);
    if (rc) {
        cterr('f',0,"Failed to enable global PVE, rc = %#x\n", rc);
        return (rc);
    }

    return (PASSED);
}

/******************************************************************************
 *
 * Function   :	dev_98dxc25x_exit
 *
 * Description:	Finish AC5 switch
 *
 * Inputs     :	dev_object_t - pointer to the 98dxc25x device. 
 *                  cpss_dev - cpss device number
 *                phy_number - 88E1680 phy number
 *
 *
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int dev_98dxc25x_xcat5_exit (dev_object_t *dev, uint cpss_dev, MAD_DEV *mad_dev)
{

    int rc = 0;
    dev_98dxc25x_object_t *obj_98dxc25x = (dev_98dxc25x_object_t *) dev;
    dev_98dxc25x_callout_fvt_t *callout_p = obj_98dxc25x->callout_fvt;

    DxChCfgDevRemove(cpss_dev);

    rc = (*callout_p->xcat5_exit)(mad_dev);

    if (rc != PASSED) {
        return (FAILED);
    }

    return (PASSED);
    
}

/******************************************************************************
 *
 * Function   :	dev_98dxc25x_xcat5_phy_port_errata_init
 *
 * Description:	AC5 & PHY port init setting
 *
 * Inputs     :	dev_object_t - pointer to the 98dxc25x device. 
 *                  cpss_dev - cpss device number
 *                phy_number - 88E1680 port number
 *
 *
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int dev_98dxc25x_xcat5_phy_port_errata_init (dev_object_t *dev, uint cpss_dev, uint port_number)
{

    int rc = 0;
    uint ix = 0;
    uint port = 0;

    for (ix = 0; ix < port_number; ix++) {
        port = ix;

        /* PHY Errata */
        rc = DxChPhyPortSmiRegisterWrite(cpss_dev, port, 22, 0x4);
        if (rc != GT_OK) {
            return (FAILED);
        }

        rc = DxChPhyPortSmiRegisterWrite(cpss_dev, port, 27, 0x3FA0);
        if (rc != GT_OK) {
            return (FAILED);
        }

        /* MACSec and PTP disable */
        rc = DxChPhyPortSmiRegisterWrite(cpss_dev, port, 22, 0);
        if (rc != GT_OK) {
            return (FAILED);
        }

        rc = DxChPhyPortSmiRegisterWrite(cpss_dev, port, 22, 0x12);
        if (rc != GT_OK) {
            return (FAILED);
        }

        rc = DxChPhyPortSmiRegisterWrite(cpss_dev, port, 27, 0);
        if (rc != GT_OK) {
            return (FAILED);
        }

        rc = DxChPhyPortSmiRegisterWrite(cpss_dev, port, 22, 0);
        if (rc != GT_OK) {
            return (FAILED);
        }

        rc = DxChPhyPortSmiRegisterWrite(cpss_dev, port, 22, 0xFD);
        if (rc != GT_OK) {
            return (FAILED);
        }

        rc = DxChPhyPortSmiRegisterWrite(cpss_dev, port, 7, 0x200D);
        if (rc != GT_OK) {
            return (FAILED);
        }

        rc = DxChPhyPortSmiRegisterWrite(cpss_dev, port, 22, 0);
        if (rc != GT_OK) {
            return (FAILED);
        }

        /* EEE init */
        rc = DxChPhyPortSmiRegisterWrite(cpss_dev, port, 22, 0xFF);
        if (rc != GT_OK) {
            return (FAILED);
        }

        rc = DxChPhyPortSmiRegisterWrite(cpss_dev, port, 17, 0xB030);
        if (rc != GT_OK) {
            return (FAILED);
        }

        rc = DxChPhyPortSmiRegisterWrite(cpss_dev, port, 16, 0x215C);
        if (rc != GT_OK) {
            return (FAILED);
        }

        rc = DxChPhyPortSmiRegisterWrite(cpss_dev, port, 22, 0x3);
        if (rc != GT_OK) {
            return (FAILED);
        }

        rc = DxChPhyPortSmiRegisterWrite(cpss_dev, port, 16, 0x1117);
        if (rc != GT_OK) {
            return (FAILED);
        }
		
        rc = DxChPhyPortSmiRegisterWrite(cpss_dev, port, 22, 0x0);
        if (rc != GT_OK) {
            return (FAILED);
        }

        rc = DxChPhyPortSmiRegisterWrite(cpss_dev, port, 16, 0x3360);
        if (rc != GT_OK) {
            return (FAILED);
        }

        rc = DxChPhyPortSmiRegisterWrite(cpss_dev, port, 0, 0x9140);
        if (rc != GT_OK) {
            return (FAILED);
        }

    }

    return (PASSED);
    
}




/*------------------------------------------------------------------
 *$Log: dev_98dxc25x.c,v $
 *Revision 1.2  2021/09/24 01:22:18  harrchan
 *Collapse Elixir-branch to Main Trunk.
 *
 *Revision 1.1.2.8  2021/05/31 10:37:29  illiu
 *Remove function dev_98dxc25x_clear_sw_counter
 *Remove function dev_98dxc25x_phy_tx_config_read/write
 *Rename function dev_98dxc25x_print_sw_counter to dev_98dxc25x_print_mac_counter
 *
 *Revision 1.1.2.7  2021/04/23 02:49:51  illiu
 *1. Rename function name: dev_98dxc25x_xcat5_phy_port_init -> dev_98dxc25x_xcat5_phy_port_errata_init
 *2. Remove call-out function: reg_pci_rd and use dev_cpss42.c to implement it
 *
 *Revision 1.1.2.6  2021/04/12 08:56:20  illiu
 *Move some Marvell library's functions from platform code to device driver
 *
 *Revision 1.1.2.5  2021/03/18 08:19:51  illiu
 *1. Add call-in function to do AC5 switch exit process
 *2. Add call-out function to do AC5 switch exit process
 *3. Add call-in function to do AC5 & PHY init port setting
 *4. Add call-out function to do AC5 & PHY init port setting
 *
 *Revision 1.1.2.4  2021/02/04 03:21:55  illiu
 *Clean up code
 *
 *Revision 1.1.2.3  2020/11/05 07:11:11  harrchan
 *1. Add print_port_mac_counter function to print MAC counters
 *
 *Revision 1.1.2.2  2020/11/05 04:19:14  illiu
 *Add test item: xCat5 Interrupt Test
 *
 *$Endlog$
*/
