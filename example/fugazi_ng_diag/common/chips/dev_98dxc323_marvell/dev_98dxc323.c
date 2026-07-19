/* $Id: dev_98dxc323.c,v 1.2 2019/12/11 10:10:22 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_98dxc323_marvell/dev_98dxc323.c,v $
 *------------------------------------------------------------------
 * Filename:	dev_98dxc323.c
 *
 * Description: Marvell 98dxc323 ESW Device Driver
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
#include "dev_98dxc323.h"
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


static uint32 dev_98dxc323_attach(dev_object_t *);
static uint32 dev_98dxc323_detach(dev_object_t *);
static uint32 dev_98dxc323_restart(dev_object_t *);
static void dev_98dxc323_destroy(dev_object_t **);
static int dev_98dxc323_xcat3_init(dev_object_t *, uint);
static int dev_98dxc323_xcat3_init_post(dev_object_t *, uint);
static int dev_98dxc323_reg_pci_rd (dev_object_t *, uint, uint, uint *);
static int dev_98dxc323_reg_pci_wr (dev_object_t *, uint, uint, uint);
static int dev_98dxc323_reg_config_rd (dev_object_t *, uint, uint, uint *);
static int dev_98dxc323_reg_config_wr (dev_object_t *, uint, uint, uint);
static int dev_98dxc323_clear_all_port_interrupt(dev_object_t *, uint);
static int dev_98dxc323_sw_gpp_init(dev_object_t *, uint);
static int dev_98dxc323_config_port_pve (dev_object_t *, uint, uint, uint);
static int dev_98dxc323_unconfig_port_pve (dev_object_t *, uint, uint, uint);
static int dev_98dxc323_config_port_pve_single_direction(dev_object_t *, uint, uint, uint);
static int dev_98dxc323_unconfig_port_pve_single_direction (dev_object_t *, uint, uint, uint);
static int dev_98dxc323_print_sw_counter (dev_object_t *, uint, uint);
static int dev_98dxc323_clear_sw_counter (dev_object_t *, uint, uint);
static int dev_98dxc323_port_force_link_set (dev_object_t *, uint, mrvl_98dxc323_link_status_t, int, boolean);
static int dev_98dxc323_xcat3_all_reg_test (dev_object_t *, uint);
static int dev_98dxc323_pcie_config_read_util (dev_object_t *);
static int dev_98dxc323_pcie_config_write_util (dev_object_t *);
static int dev_98dxc323_xcat3_reg_read_util (dev_object_t *, uint);
static int dev_98dxc323_xcat3_reg_write_util (dev_object_t *, uint);
static int dev_98dxc323_xcat3_internal_reg_read_util (dev_object_t *, uint);
static int dev_98dxc323_xcat3_internal_reg_write_util (dev_object_t *, uint);
static int dev_98dxc323_xcat3_enable_force_interrupt(dev_object_t *, uint, uint);
static int dev_98dxc323_xcat3_gen_int (dev_object_t *, uint);
static int dev_98dxc323_xcat3_clear_int (dev_object_t *, uint);
static int dev_98dxc323_led_test (dev_object_t *,  uint);
static int dev_98dxc323_10g_kr_test_mode (dev_object_t *, uint, uint, uint, uint);
static int dev_98dxc323_serdes_tx_config_read (dev_object_t *);
static int dev_98dxc323_serdes_tx_config_write (dev_object_t *);
static int dev_98dxc323_phy_tx_config_read (dev_object_t *);
static int dev_98dxc323_phy_tx_config_write (dev_object_t *);



/*===================================================================*
 *                    Polling function                               *
 *===================================================================*/


/*===================================================================*
 *                    Global variables                               *
 *===================================================================*/

static char mrv_98dxc323_err_buf[MRV98DXC323_ERR_BUF_SIZE];


/******************************************************************************
 *
 * Name:	mrv98dxc323_dev_create()
 *
 * Description:	Create object with various device function
 *        		point to "do nothing"
 *
 * Input:	dev_object_t pointer to the 98dxc323 device.
 *		    error reporting function pointer.
 *
 * Returns:	none
 *
 *****************************************************************************/
void mrv98dxc323_dev_create (dev_object_t *dev, dev_error_report_t error_report_fn)
{
    dev_object_fvt_t	*dev_fvt;
    dev_98dxc323_object_t *obj_98dxc323= (dev_98dxc323_object_t*)dev;

    /* Allocate memory for the device object */
    if ((dev_fvt = (dev_object_fvt_t *)malloc(sizeof(dev_object_fvt_t))) ==
		            NULL) {
        /* Unable to allocate memory */
        error_report_fn(dev, "malloc failure in 98dxc323_dev_create()", 0);
        printf("%s: NULL\n", __func__);
	    return;
    }

    /* Init the device object structure to default "do nothing" */
    init_default_dev_object(dev, dev_fvt);

    obj_98dxc323->base.dev_object_fvt->dev_attach	= dev_98dxc323_attach;
    obj_98dxc323->base.dev_object_fvt->dev_detach	= dev_98dxc323_detach;
    obj_98dxc323->base.dev_object_fvt->dev_restart	= dev_98dxc323_restart;
    obj_98dxc323->base.dev_object_fvt->dev_error_report	= error_report_fn;
    obj_98dxc323->base.dev_object_fvt->dev_destroy	= dev_98dxc323_destroy;
    obj_98dxc323->base.dev_object_fvt->dev_name	= "98dxc323 Marvell ESW AlleyCat3";

    obj_98dxc323->callin_fvt = (dev_98dxc323_callin_fvt_t *)
                               malloc(sizeof(dev_98dxc323_callin_fvt_t));
    obj_98dxc323->callout_fvt = (dev_98dxc323_callout_fvt_t *)
                                malloc(sizeof(dev_98dxc323_callout_fvt_t));

    obj_98dxc323->base.dev_state = DEV_STATE_CREATE;
}


/******************************************************************************
 *
 * Name:	dev_98dxc323_attach()
 *
 * Description:	Attach the 98dxc323 device for use. This function will
 *		        initialize and setup all necessary pointers and bring the
 *        		chip to operation.
 *
 * Input:	Pointer to the 98dxc323 device object
 *
 * Returns:	PASSED/FAILED
 *
 *****************************************************************************/
static uint32 dev_98dxc323_attach (dev_object_t *dev)
{
    dev_98dxc323_object_t *obj_98dxc323 = (dev_98dxc323_object_t *) dev;

    if (obj_98dxc323->callin_fvt == NULL) {
        DEV_ERROR_REPORT(dev, "dev_98dxc323_attach() callin malloc", DEV_98DXC323_ATTACH);
        return (FAILED);
    }

    if (obj_98dxc323->callout_fvt == NULL) {
        DEV_ERROR_REPORT(dev, "dev_98dxc323_attach() callout malloc", DEV_98DXC323_ATTACH);
        return (FAILED);
    }
	
    obj_98dxc323->callin_fvt->esw_xcat3_init = dev_98dxc323_xcat3_init;
    obj_98dxc323->callin_fvt->esw_xcat3_init_post = dev_98dxc323_xcat3_init_post;
    obj_98dxc323->callin_fvt->esw_clear_all_port_interrupt = dev_98dxc323_clear_all_port_interrupt;
    obj_98dxc323->callin_fvt->esw_sw_gpp_init = dev_98dxc323_sw_gpp_init;
    obj_98dxc323->callin_fvt->esw_config_port_pve = dev_98dxc323_config_port_pve;
    obj_98dxc323->callin_fvt->esw_unconfig_port_pve = dev_98dxc323_unconfig_port_pve;
    obj_98dxc323->callin_fvt->esw_config_port_pve_single_direction = dev_98dxc323_config_port_pve_single_direction;
    obj_98dxc323->callin_fvt->esw_unconfig_port_pve_single_direction = dev_98dxc323_unconfig_port_pve_single_direction;
    obj_98dxc323->callin_fvt->esw_print_sw_counter = dev_98dxc323_print_sw_counter;
    obj_98dxc323->callin_fvt->esw_clear_sw_counter = dev_98dxc323_clear_sw_counter;
    obj_98dxc323->callin_fvt->esw_port_force_link_set = dev_98dxc323_port_force_link_set;
    obj_98dxc323->callin_fvt->esw_xcat3_all_reg_test = dev_98dxc323_xcat3_all_reg_test;
    obj_98dxc323->callin_fvt->esw_pcie_config_read_util = dev_98dxc323_pcie_config_read_util;
    obj_98dxc323->callin_fvt->esw_pcie_config_write_util = dev_98dxc323_pcie_config_write_util;
    obj_98dxc323->callin_fvt->esw_xcat3_reg_read_util = dev_98dxc323_xcat3_reg_read_util;
    obj_98dxc323->callin_fvt->esw_xcat3_reg_write_util = dev_98dxc323_xcat3_reg_write_util;
    obj_98dxc323->callin_fvt->esw_xcat3_internal_reg_read_util = dev_98dxc323_xcat3_internal_reg_read_util;
    obj_98dxc323->callin_fvt->esw_xcat3_internal_reg_write_util = dev_98dxc323_xcat3_internal_reg_write_util;
    obj_98dxc323->callin_fvt->esw_xcat3_gen_int = dev_98dxc323_xcat3_gen_int;
    obj_98dxc323->callin_fvt->esw_xcat3_clear_int = dev_98dxc323_xcat3_clear_int;
    obj_98dxc323->callin_fvt->esw_xcat3_led_test = dev_98dxc323_led_test;
    obj_98dxc323->callin_fvt->esw_xcat3_10g_kr_test_mode = dev_98dxc323_10g_kr_test_mode;
    obj_98dxc323->callin_fvt->esw_xcat3_serdes_tx_config_read = dev_98dxc323_serdes_tx_config_read;
    obj_98dxc323->callin_fvt->esw_xcat3_serdes_tx_config_write = dev_98dxc323_serdes_tx_config_write;
    obj_98dxc323->callin_fvt->esw_xcat3_phy_tx_config_read = dev_98dxc323_phy_tx_config_read;
    obj_98dxc323->callin_fvt->esw_xcat3_phy_tx_config_write = dev_98dxc323_phy_tx_config_write;

	
    obj_98dxc323->base.dev_state = DEV_STATE_ATTACH;

    return (PASSED);
}

/******************************************************************************
 *
 * Name:	dev_98dxc323_detach()
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
 * Input:	Pointer to the 98dxc323 device object
 *
 * Returns:	PASSED/FAILED
 *
 *****************************************************************************/
static uint32 dev_98dxc323_detach (dev_object_t *dev)
{
    dev_98dxc323_object_t *obj_98dxc323 = (dev_98dxc323_object_t *) dev;

    /* Init the device object structure to default "do nothing" */
    init_default_dev_object(dev, obj_98dxc323->base.dev_object_fvt);

    obj_98dxc323->base.dev_state = DEV_STATE_DETACH;

    return (PASSED);

}


/******************************************************************************
 * Name:	dev_98dxc323_restart
 *
 * Description:	To restart a previously initialized device without resetting
 *		        the device or changing its configuration.
 *		        For example, during a failover event.
 *
 *		        Change the state of the device from its current state
 *		        to an initial state. Also, dev_state must be assigned the
 *		        value of DEV_STATE_INIT.
 *
 * Input:	dev_object_t pointer to the 98dxc323 device
 *
 * Returns:	PASSED/FAILED
 *
 * Assumptions:	The dev_attach() and dev_reconfig_needed() functions has been
 *		        called and successfully executed.
 *
 *****************************************************************************/
static uint32 dev_98dxc323_restart (dev_object_t *dev)
{
    dev_98dxc323_object_t *obj_98dxc323= (dev_98dxc323_object_t *) dev;

    obj_98dxc323->base.dev_state = DEV_STATE_INIT;
    return (PASSED);
}


/******************************************************************************
 * Name:	dev_98dxc323_destroy
 *
 * Description:	Destroy the dev_object structure and free all the resources.
 *
 * Input:	dev_object_t pointer to the 98dxc323 device
 *
 * Returns:	none
 *
 * Assumptions:	The dev_attch() function has been called and successfully
 *
 *****************************************************************************/
static void dev_98dxc323_destroy (dev_object_t **dev)
{
    dev_98dxc323_object_t *obj_98dxc323;

    if (dev == NULL) {
        return;
    }

    if (*dev == NULL) {
        return;
    }

    obj_98dxc323 = (dev_98dxc323_object_t *)*dev;

    if (obj_98dxc323->callout_fvt) {
        free(obj_98dxc323->callout_fvt);	/* Free callout struct */
    }

    if (obj_98dxc323->callin_fvt) {
        free(obj_98dxc323->callin_fvt);		/* Free callin struct */
    }

    free(obj_98dxc323->base.dev_object_fvt);	/* Free dev_object_t */
}


/**********************************************************************
 *
 * Function: dev_98dxc323_reg_pci_rd
 *
 * This function: Reads Marvell ESW registers through callout
 *
 * Input : dev - dev_object_t pointer to the 98dxc323 device
 *           cpss_dev - cpss device number
 *	       reg - register
 *	       data*  - read-back data
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int dev_98dxc323_reg_pci_rd (dev_object_t *dev, uint cpss_dev, uint reg, uint *data)
{
    int rc;
    dev_98dxc323_object_t *obj_98dxc323 = (dev_98dxc323_object_t *) dev;
    dev_98dxc323_callout_fvt_t *callout_p = obj_98dxc323->callout_fvt;

    rc = (*callout_p->reg_pci_rd)(cpss_dev, reg, data);

    if (rc != PASSED) {
        sprintf(mrv_98dxc323_err_buf, "%s: Failed (%#x)", __func__, rc);
        return (FAILED);
    }

    return (PASSED);
}


/**********************************************************************
 *
 * Function: dev_98dxc323_reg_pci_wr
 *
 * This function: writes Marvell ESW registers through callout
 *
 * Input : dev - dev_object_t pointer to the 98dxc323 device
 *           cpss_dev - cpss device number
 *	       reg - register
 *	       data  - write-in data
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int dev_98dxc323_reg_pci_wr (dev_object_t *dev, uint cpss_dev, uint reg, uint data)
{
    dev_98dxc323_object_t *obj_98dxc323 = (dev_98dxc323_object_t *) dev;
    dev_98dxc323_callout_fvt_t *callout_p = obj_98dxc323->callout_fvt;
    int rc ;

    rc = (*callout_p->reg_pci_wr)(cpss_dev, reg, data);

    if (rc != PASSED) {
        sprintf(mrv_98dxc323_err_buf, "%s: Failed (%#x)", __func__, rc);
        return (FAILED);
    }

    return (PASSED);
}


/**********************************************************************
 *
 * Function: dev_98dxc323_reg_config_rd
 *
 * This function: Reads Marvell ESW registers through callout
 *
 * Input : dev - dev_object_t pointer to the 98dxc323 device
 *           cpss_dev - cpss device number
 *	       reg - register
 *	       data*  - read-back data
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int dev_98dxc323_reg_config_rd (dev_object_t *dev, uint cpss_dev, uint reg, uint *data)
{
    int rc;
    dev_98dxc323_object_t *obj_98dxc323 = (dev_98dxc323_object_t *) dev;
    dev_98dxc323_callout_fvt_t *callout_p = obj_98dxc323->callout_fvt;

    rc = (*callout_p->reg_config_rd)(cpss_dev, reg, data);

    if (rc != PASSED) {
        sprintf(mrv_98dxc323_err_buf, "%s: Failed (%#x)", __func__, rc);
        return (FAILED);
    }

    return (PASSED);
}


/**********************************************************************
 *
 * Function: dev_98dxc323_reg_config_wr
 *
 * This function: writes Marvell ESW registers through callout
 *
 * Input : dev - dev_object_t pointer to the 98dxc323 device
 *           cpss_dev - cpss device number
 *	       reg - register
 *	       data  - write-in data
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int dev_98dxc323_reg_config_wr (dev_object_t *dev, uint cpss_dev, uint reg, uint data)
{
    dev_98dxc323_object_t *obj_98dxc323 = (dev_98dxc323_object_t *) dev;
    dev_98dxc323_callout_fvt_t *callout_p = obj_98dxc323->callout_fvt;
    int rc ;

    rc = (*callout_p->reg_config_wr)(cpss_dev, reg, data);

    if (rc != PASSED) {
        sprintf(mrv_98dxc323_err_buf, "%s: Failed (%#x)", __func__, rc);
        return (FAILED);
    }

    return (PASSED);
}


/**********************************************************************
 *
 * Function: dev_98dxc323_xcat3_init
 *
 * This function: Marvell 98DXC323 xcat3 Initialization
 *
 * Input : dev - dev_object_t pointer to the 98dxc323 device
 *           cpss_dev - cpss device number
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int dev_98dxc323_xcat3_init (dev_object_t *dev, uint cpss_dev)
{
    int rc;
    dev_98dxc323_object_t *obj_98dxc323 = (dev_98dxc323_object_t *) dev;
    dev_98dxc323_callout_fvt_t *callout_p = obj_98dxc323->callout_fvt;

    rc = (*callout_p->cpss_driver_init)(cpss_dev);
    if (rc != PASSED) {
        sprintf(mrv_98dxc323_err_buf, "%s: Failed (%#x)", __func__, rc);
        return (FAILED);
    }
  
    rc = (*callout_p->cpss_device_init)(cpss_dev);
    if (rc != PASSED) {
        sprintf(mrv_98dxc323_err_buf, "%s: Failed (%#x)", __func__, rc);
        return (FAILED);
    }

    rc = dev_98dxc323_clear_all_port_interrupt(dev, cpss_dev);
    if (rc != PASSED) {
        sprintf(mrv_98dxc323_err_buf, "%s: Failed (%#x)", __func__, rc);
        return (FAILED);
    }
   
    return (PASSED);
}


/**********************************************************************
 *
 * Function: dev_98dxc323_xcat3_init_post
 *
 * This function: Marvell 98DXC323 Initialization Post
 *
 * Input : dev - dev_object_t pointer to the 98dxc323 device
 *            cpss_dev - cpss device number
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int dev_98dxc323_xcat3_init_post(dev_object_t *dev, uint cpss_dev)
{
    int rc;
    dev_98dxc323_object_t *obj_98dxc323 = (dev_98dxc323_object_t *) dev;
    dev_98dxc323_callout_fvt_t *callout_p = obj_98dxc323->callout_fvt;

    rc = (*callout_p->smi_phy_init)(cpss_dev);
    if (rc != PASSED) {
        sprintf(mrv_98dxc323_err_buf, "%s: Failed (%#x)", __func__, rc);
        return (FAILED);
    }

    return (PASSED);
}


/******************************************************************************
 *
 * Function:    dev_98dxc323_clear_port_interrupt
 *
 * Description: 98dxc323 clear all port interrupt
 *
 * Inputs:  dev_object_t pointer to the 98dxc323 device
 *             cpss_dev - cpss device number
 *
 * Outputs: PASSED - No errors encounterd.
 *          FAILED - Errors encounterd.
 *
 * Assumptions: create and dev_attach have to be called first. dev_destroy will
 *              also be called after the exit.
 *
 *****************************************************************************/
static int dev_98dxc323_clear_port_interrupt(dev_object_t *dev, uint cpss_dev, uint32_t port_num)
{
    uint32_t port_irupt_cause_reg;
    uint32_t value;
   
    port_irupt_cause_reg = PORT_0_IRUPT_CAUSE_REGISTER +
	                  PORT_IRUPT_OFFSET * port_num;
    return(dev_98dxc323_reg_pci_rd(dev, cpss_dev, port_irupt_cause_reg, &value));
	
}


/******************************************************************************
 *
 * Function:    dev_98dxc323_clear_all_port_interrupt
 *
 * Description: 98dxc323 clear all port interrupt
 *
 * Inputs:  dev_object_t pointer to the 98dxc323 device
 *             cpss_dev - cpss device number
 *
 * Outputs: PASSED - No errors encounterd.
 *          FAILED - Errors encounterd.
 *
 * Assumptions: create and dev_attach have to be called first. dev_destroy will
 *              also be called after the exit.
 *
 *****************************************************************************/
static int dev_98dxc323_clear_all_port_interrupt(dev_object_t *dev, uint cpss_dev)
{
    uint32_t port_num;
    
    for (port_num = 0; port_num < XCAT3_PORT_NUM; port_num ++) {
	if (dev_98dxc323_clear_port_interrupt(dev, cpss_dev, port_num) != PASSED) {
	    cterr('f',0,"Failed to clear interrupt for port %d", port_num);
	    return (FAILED);
	}
    }
    
    return PASSED;
}


/******************************************************************************
 *
 * Function:    dev_98dxc323_sw_gpp_init
 *
 * Description: 98dxc323 GPP gpio init
 *
 * Inputs:  dev_object_t pointer to the 98dxc323 device
 *             cpss_dev - cpss device number
 *
 * Outputs: PASSED - No errors encounterd.
 *          FAILED - Errors encounterd.
 *
 * Assumptions: create and dev_attach have to be called first. dev_destroy will
 *              also be called after the exit.
 *
 *****************************************************************************/
static int dev_98dxc323_sw_gpp_init(dev_object_t *dev, uint cpss_dev)
{
    uint32_t value = 0;

    //To-do

    return(dev_98dxc323_reg_pci_wr(dev, cpss_dev, GPP_IO_CTRL_REG_OFFSET, value));
}


/**********************************************************************
 *
 * Function: dev_98dxc323_config_port_pve
 *
 * This function: configure port pve
 *
 * Input : dev_object_t - pointer to the 98dxc323 device. 
 *            cpss_dev - cpss device number
 *            src_port - source port
 *            dst_port - destination port
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int 
dev_98dxc323_config_port_pve (dev_object_t *dev, uint cpss_dev, uint src_port, uint dst_port)
{
    uint32_t rc = 0;

    dev_98dxc323_object_t *obj_98dxc323 = (dev_98dxc323_object_t *) dev;
    dev_98dxc323_callout_fvt_t *callout_p = obj_98dxc323->callout_fvt;

    rc = (*callout_p->global_enable_pve)(cpss_dev);
    if (rc) {
        cterr('f',0,"Failed to enable global PVE, rc = %#x\n", rc);
        return rc;
    }

    rc = (*callout_p->port_enable)(cpss_dev, src_port);
    if (rc) {
        cterr('f',0,"Failed to enable src_port &d\n", src_port);
        return rc;
    }

    rc = (*callout_p->port_enable)(cpss_dev, dst_port);
    if (rc) {
        cterr('f',0,"Failed to enable dst_port &d\n", dst_port);
        return rc;
    }

    rc = (*callout_p->set_port_pve)(cpss_dev, src_port, dst_port);
    if (rc) {
        cterr('f',0,"Failed to set PVE for ports %d and %d.\n", src_port, dst_port);
        return rc;
    }
    return PASSED;
}


/**********************************************************************
 *
 * Function: dev_98dxc323_unconfig_port_pve
 *
 * This function: unconfigure port pve
 *
 * Input : dev_object_t - pointer to the 98dxc323 device. 
 *            cpss_dev - cpss device number
 *            src_port - source port
 *            dst_port - destination port
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int 
dev_98dxc323_unconfig_port_pve (dev_object_t *dev, uint cpss_dev, uint src_port, uint dst_port)
{
    uint32_t rc = 0;

    dev_98dxc323_object_t *obj_98dxc323 = (dev_98dxc323_object_t *) dev;
    dev_98dxc323_callout_fvt_t *callout_p = obj_98dxc323->callout_fvt;

    rc = (*callout_p->global_disable_pve)(cpss_dev);
    if (rc) {
        cterr('f',0,"Failed to disable global PVE, rc = %#x\n", rc);
        return rc;
    }

    rc = (*callout_p->clear_port_pve)(cpss_dev, src_port, dst_port);
    if (rc) {
        cterr('f',0,"Failed to clear PVE for ports %d and %d.\n", src_port, dst_port);
        return rc;
    }
    return PASSED;
}


/**********************************************************************
 *
 * Function: dev_98dxc323_config_port_pve_single_direction
 *
 * This function: configure port pve for single direction
 *
 * Input : dev_object_t - pointer to the 98dxc323 device. 
 *            cpss_dev - cpss device number
 *            src_port - source port
 *            dst_port - destination port
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int 
dev_98dxc323_config_port_pve_single_direction (dev_object_t *dev, uint cpss_dev, uint src_port, uint dst_port)
{
    uint32_t rc = 0;

    dev_98dxc323_object_t *obj_98dxc323 = (dev_98dxc323_object_t *) dev;
    dev_98dxc323_callout_fvt_t *callout_p = obj_98dxc323->callout_fvt;

    rc = (*callout_p->global_enable_pve)(cpss_dev);
    if (rc) {
        cterr('f',0,"Failed to enable global PVE, rc = %#x\n", rc);
        return rc;
    }

    rc = (*callout_p->port_enable)(cpss_dev, src_port);
    if (rc) {
        cterr('f',0,"Failed to enable src_port &d\n", src_port);
        return rc;
    }

    rc = (*callout_p->port_enable)(cpss_dev, dst_port);
    if (rc) {
        cterr('f',0,"Failed to enable dst_port &d\n", dst_port);
        return rc;
    }

    rc = (*callout_p->set_port_pve_singel_direction)(cpss_dev, src_port, dst_port);
    if (rc) {
        cterr('f',0,"Failed to set PVE for ports %d and %d.\n", src_port, dst_port);
        return rc;
    }
    return PASSED;
}


/**********************************************************************
 *
 * Function: dev_98dxc323_unconfig_port_pve_single_direction
 *
 * This function: unconfigure port pve fore single direction
 *
 * Input : dev_object_t - pointer to the 98dxc323 device. 
 *            cpss_dev - cpss device number
 *            src_port - source port
 *            dst_port - destination port
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int 
dev_98dxc323_unconfig_port_pve_single_direction (dev_object_t *dev, uint cpss_dev, uint src_port, uint dst_port)
{
    uint32_t rc = 0;

    dev_98dxc323_object_t *obj_98dxc323 = (dev_98dxc323_object_t *) dev;
    dev_98dxc323_callout_fvt_t *callout_p = obj_98dxc323->callout_fvt;

    rc = (*callout_p->global_disable_pve)(cpss_dev);
    if (rc) {
        cterr('f',0,"Failed to disable global PVE, rc = %#x\n", rc);
        return rc;
    }

    rc = (*callout_p->clear_port_pve_singel_direction)(cpss_dev, src_port, dst_port);
    if (rc) {
        cterr('f',0,"Failed to clear PVE for ports %d and %d.\n", src_port, dst_port);
        return rc;
    }
    return PASSED;
}


/**********************************************************************
 *
 * Function: dev_98dxc323_print_sw_counter
 *
 * This function: print software counter
 *
 * Input : dev_object_t - pointer to the 98dxc323 device. 
 *            cpss_dev - cpss device number
 *            port_num - number of ports
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int 
dev_98dxc323_print_sw_counter (dev_object_t *dev, uint cpss_dev, uint port_num)
{

    uint32_t data1, data2;
    int ix;

    dev_98dxc323_reg_pci_rd(dev, cpss_dev, 0x9300018, &data1);
    dev_98dxc323_reg_pci_rd(dev, cpss_dev, 0x930004c, &data2);
    printf("\ncounter for GE port 24: RX = 0x%x, TX = 0x%x\n", data1, data2);
    dev_98dxc323_reg_pci_rd(dev, cpss_dev, 0x9320018, &data1);
    dev_98dxc323_reg_pci_rd(dev, cpss_dev, 0x932004c, &data2);
    printf("counter for GE port 25: RX = 0x%x, TX = 0x%x\n", data1, data2);

    for (ix = 0; ix < port_num; ix++) {
	if (ix < 6) {
	    dev_98dxc323_reg_pci_rd(dev, cpss_dev, 0x4010018+ix*0x80, &data1);
           dev_98dxc323_reg_pci_rd(dev, cpss_dev, 0x401004c+ix*0x80, &data2);
	} else if ((ix >= 6) && (ix < 12)) {
           dev_98dxc323_reg_pci_rd(dev, cpss_dev, 0x4810018+(ix-6)*0x80, &data1);
           dev_98dxc323_reg_pci_rd(dev, cpss_dev, 0x481004c+(ix-6)*0x80, &data2);
	} else if ((ix >= 12) && (ix < 18)) {
           dev_98dxc323_reg_pci_rd(dev, cpss_dev, 0x5010018+(ix-12)*0x80, &data1);
           dev_98dxc323_reg_pci_rd(dev, cpss_dev, 0x501004c+(ix-12)*0x80, &data2);
	} else{
           dev_98dxc323_reg_pci_rd(dev, cpss_dev, 0x5810018+(ix-18)*0x80, &data1);
           dev_98dxc323_reg_pci_rd(dev, cpss_dev, 0x581004c+(ix-18)*0x80, &data2);
	}
	printf("counter for GE port %d: RX = 0x%x, TX = 0x%x\n", ix + 1, data1, data2);
    }

    return PASSED;
	
}


/**********************************************************************
 *
 * Function: dev_98dxc323_clear_sw_counter
 *
 * This function: clear software counter
 *
 * Input : dev_object_t - pointer to the 98dxc323 device. 
 *            cpss_dev - cpss device number
 *            port_num - number of ports
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int 
dev_98dxc323_clear_sw_counter (dev_object_t *dev, uint cpss_dev, uint port_num)
{

    uint32_t data1, data2;
    int ix;

    dev_98dxc323_reg_pci_rd(dev, cpss_dev, 0x9300018, &data1);
    dev_98dxc323_reg_pci_rd(dev, cpss_dev, 0x930004c, &data2);
    dev_98dxc323_reg_pci_rd(dev, cpss_dev, 0x9320018, &data1);
    dev_98dxc323_reg_pci_rd(dev, cpss_dev, 0x932004c, &data2);
    
    for (ix = 0; ix < port_num; ix++) {
	if (ix < 6) {
	    dev_98dxc323_reg_pci_rd(dev, cpss_dev, 0x4010018+ix*0x80, &data1);
           dev_98dxc323_reg_pci_rd(dev, cpss_dev, 0x401004c+ix*0x80, &data2);
	} else if ((ix >= 6) && (ix < 12)) {
           dev_98dxc323_reg_pci_rd(dev, cpss_dev, 0x4810018+(ix-6)*0x80, &data1);
           dev_98dxc323_reg_pci_rd(dev, cpss_dev, 0x481004c+(ix-6)*0x80, &data2);
	} else if ((ix >= 12) && (ix < 18)) {
           dev_98dxc323_reg_pci_rd(dev, cpss_dev, 0x5010018+(ix-12)*0x80, &data1);
           dev_98dxc323_reg_pci_rd(dev, cpss_dev, 0x501004c+(ix-12)*0x80, &data2);
	} else{
           dev_98dxc323_reg_pci_rd(dev, cpss_dev, 0x5810018+(ix-18)*0x80, &data1);
           dev_98dxc323_reg_pci_rd(dev, cpss_dev, 0x581004c+(ix-18)*0x80, &data2);
	}
    }

    return PASSED;
	
}


/**********************************************************************
 *
 * Function: dev_98dxc323_unconfig_port_pve
 *
 * This function: port force link set
 *
 * Input : dev - dev_object_t pointer to the 98dxc323 device
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
static int
dev_98dxc323_port_force_link_set (dev_object_t *dev, uint cpss_dev, mrvl_98dxc323_link_status_t link, int port, boolean set)
{

    dev_98dxc323_object_t *obj_98dxc323 = (dev_98dxc323_object_t *) dev;
    dev_98dxc323_callout_fvt_t *callout_p = obj_98dxc323->callout_fvt;
	
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
 * Function   :	dev_98dxc323_all_reg_test
 *
 * Description:	Test xCat3 all internal registers by calling CPSS API.
 *              Original value is restored after test completes.
 *
 * Inputs     :	dev_object_t - pointer to the 98dxc323 device. 
 *                  cpss_dev - cpss device number
 *
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int 
dev_98dxc323_xcat3_all_reg_test (dev_object_t *dev, uint cpss_dev)
{
    int rc = 0;
    uint badReg = 0;
    uint readVal = 0;
    uint writeVal = 0;
    uint testStatus = 0;

    dev_98dxc323_object_t *obj_98dxc323 = (dev_98dxc323_object_t *) dev;
    dev_98dxc323_callout_fvt_t *callout_p = obj_98dxc323->callout_fvt;

    rc =  (*callout_p->xcat3_all_reg_test)(cpss_dev,
                                &testStatus,
                                &badReg,
                                &readVal,
                                &writeVal);

    if (rc == PASSED) {
        if (testStatus == FALSE) {
            cterr('f', 0, "xCat3 switch all register test failed.\n"
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
 * Function   :	dev_98dxc323__pcie_config_read_util
 *
 * Description:	Utility to read PCIe configuration space register.
 *
 * Inputs     :	dev_object_t - pointer to the 98dxc323 device. 
 *
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int 
dev_98dxc323_pcie_config_read_util (dev_object_t *dev)
{
    uint reg_val = 0, reg_addr = 0;

    dev_98dxc323_object_t *obj_98dxc323 = (dev_98dxc323_object_t *) dev;
    dev_98dxc323_callout_fvt_t *callout_p = obj_98dxc323->callout_fvt;

    reg_addr = gethex_answer("Enter the address:", 0, 0, 0xFFFFF);

    (*callout_p->pcie_config_read)(reg_addr, (uint *)&reg_val);

    printf("\n\n Value of 0x%05X is 0x%08X.\n\n", reg_addr, reg_val);

    return (PASSED);
}


/******************************************************************************
 *
 * Function   :	dev_98dxc323_pcie_config_write_util
 *
 * Description:	Utility to write PCIe configuration space register.
 *
 * Inputs     :	dev_object_t - pointer to the 98dxc323 device. 
 *
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int 
dev_98dxc323_pcie_config_write_util (dev_object_t *dev)
{
    uint32_t reg_addr = 0, write_in = 0;

    dev_98dxc323_object_t *obj_98dxc323 = (dev_98dxc323_object_t *) dev;
    dev_98dxc323_callout_fvt_t *callout_p = obj_98dxc323->callout_fvt;

    reg_addr = gethex_answer("Enter the address:", 0, 0, 0xFFFFF);
    write_in = gethex_answer("Enter the data:", 0, 0, 0xFFFFFFFF);

    (*callout_p->pcie_config_write)(reg_addr, write_in);
    printf("\n\nDone.\n");

    return (PASSED);
}


/******************************************************************************
 *
 * Function   :	dev_98dxc323_xcat3_reg_read_util
 *
 * Description:	Utility to read xCat3 memory mapped registers.
 *
 * Inputs     :	dev_object_t - pointer to the 98dxc323 device.
 *                  cpss_dev - cpss device number
 *
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int 
dev_98dxc323_xcat3_reg_read_util (dev_object_t *dev, uint cpss_dev)
{
    uint32_t reg_val, reg_addr;

    reg_addr = gethex_answer("Enter the register offset:", 0, 0, 0xFFFFFFFF);
#if 0
    /* This API can read pp registers without CPSS initialization */
    cpssDxChDiagRegRead(dl_get_bus_base_addr(), 3, 0, reg_addr, (uint32_t *)&reg_val, 0);
#endif 
    dev_98dxc323_reg_pci_rd(dev, cpss_dev, reg_addr, (uint32_t *)&reg_val);    

    printf("\n register at offset %#x is %#x.\n", reg_addr, reg_val);

    return (PASSED);
}


/******************************************************************************
 *
 * Function   :	dev_98dxc323_xcat3_reg_write_util
 *
 * Description: Utility to write xCat3 memory mapped registers.
 *
 * Inputs     :	dev_object_t - pointer to the 98dxc323 device.
 *                  cpss_dev - cpss device number
 *
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int 
dev_98dxc323_xcat3_reg_write_util (dev_object_t *dev, uint cpss_dev)
{
    uint32_t reg_addr, write_in, reg_val;

    reg_addr = gethex_answer("Enter the register offset:", 0, 0, 0xFFFFFFFF);
    write_in = gethex_answer("Enter the register data:", 0, 0, 0xFFFFFFFF);

    dev_98dxc323_reg_pci_wr(dev, cpss_dev, reg_addr, write_in);

    dev_98dxc323_reg_pci_rd(dev, cpss_dev, reg_addr, (uint32_t *)&reg_val);    
    printf("\nregister at offset %#x is %#x.\n", reg_addr, reg_val);

    return (PASSED);
}


/******************************************************************************
 *
 * Function   :	dev_98dxc323_xcat3_internal_reg_read_util
 *
 * Description:	Utility to read xCat3 memory mapped registers.
 *
 * Inputs     :	dev_object_t - pointer to the 98dxc323 device.
 *                  cpss_dev - cpss device number
 *
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int 
dev_98dxc323_xcat3_internal_reg_read_util (dev_object_t *dev, uint cpss_dev)
{
    uint32_t reg_val, reg_addr;

    reg_addr = gethex_answer("Enter the register offset:", 0, 0, 0xFFFFFFFF);
#if 0
    /* This API can read pp registers without CPSS initialization */
    cpssDxChDiagRegRead(dl_get_bus_base_addr(), 3, 0, reg_addr, (uint32_t *)&reg_val, 0);
#endif 
    dev_98dxc323_reg_config_rd(dev, cpss_dev, reg_addr, (uint32_t *)&reg_val);    

    printf("\n register at offset %#x is %#x.\n", reg_addr, reg_val);

    return (PASSED);
}


/******************************************************************************
 *
 * Function   :	dev_98dxc323_xcat3_internal_reg_write_util
 *
 * Description: Utility to write xCat3 memory mapped registers.
 *
 * Inputs     :	dev_object_t - pointer to the 98dxc323 device.
 *                  cpss_dev - cpss device number
 *
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int 
dev_98dxc323_xcat3_internal_reg_write_util (dev_object_t *dev, uint cpss_dev)
{
    uint32_t reg_addr, write_in, reg_val;

    reg_addr = gethex_answer("Enter the register offset:", 0, 0, 0xFFFFFFFF);
    write_in = gethex_answer("Enter the register data:", 0, 0, 0xFFFFFFFF);

    dev_98dxc323_reg_config_wr(dev, cpss_dev, reg_addr, write_in);

    dev_98dxc323_reg_config_rd(dev, cpss_dev, reg_addr, (uint32_t *)&reg_val);    
    printf("\nregister at offset %#x is %#x.\n", reg_addr, reg_val);

    return (PASSED);
}


/******************************************************************************
 *
 * Function   :	dev_98dxc323_xcat3_enable_force_interrupt
 *
 * Description: xCat3 interrupt test.
 *
 * Inputs     :	dev_object_t - pointer to the 98dxc323 device.
 *                  cpss_dev - cpss device number
 *            enable - enable / disable
 *
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int 
dev_98dxc323_xcat3_enable_force_interrupt(dev_object_t *dev, uint cpss_dev, uint enable)
{

    uint32_t reg_val, reg_addr;
	
    reg_addr = MPP_CONTROL_8_15_REG;
    dev_98dxc323_reg_config_rd(dev, cpss_dev, reg_addr, (uint32_t *)&reg_val);  
    reg_val &= MPP_CONTROL_MASK_MPP13;
    dev_98dxc323_reg_config_wr(dev, cpss_dev, reg_addr, reg_val);


    if (enable == DEV_98DXC323_ENABLE) {

	 reg_addr = GPIO_DATA_OUT_EN_REG;
        dev_98dxc323_reg_config_rd(dev, cpss_dev, reg_addr, (uint32_t *)&reg_val); 
	 reg_val &= ~ GPIO_DATA_OUT_MPP13_BIT;
	 dev_98dxc323_reg_config_wr(dev, cpss_dev, reg_addr, reg_val);

	 reg_addr = GPIO_DATA_OUT_CLEAR_REG;
	 reg_val = GPIO_DATA_OUT_MPP13_BIT;
	 dev_98dxc323_reg_config_wr(dev, cpss_dev, reg_addr, reg_val);
	 
    } else {

	 reg_addr = GPIO_DATA_OUT_EN_REG;
        dev_98dxc323_reg_config_rd(dev, cpss_dev, reg_addr, (uint32_t *)&reg_val); 
        reg_val |= GPIO_DATA_OUT_MPP13_BIT;
	 dev_98dxc323_reg_config_wr(dev, cpss_dev, reg_addr, reg_val);
    }
	
    return (PASSED);
}


/**********************************************************************
 *
 * Function: dev_98dxc323_xcat3_gen_int
 *
 * This function: Marvell 98dxc323 generate interrupt
 *
 * Inputs     :	dev_object_t - pointer to the 98dxc323 device.
 *                  cpss_dev - cpss device number
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int 
dev_98dxc323_xcat3_gen_int (dev_object_t *dev, uint cpss_dev)
{

    uint rc; 

    rc = dev_98dxc323_xcat3_enable_force_interrupt(dev, cpss_dev, DEV_98DXC323_ENABLE);
    return rc;
    
}


/**********************************************************************
 *
 * Function: dev_98dxc323_xcat3_clear_int
 *
 * This function: Marvell 98dxc323 clear interrupt
 *
 * Inputs     :	dev_object_t - pointer to the 98dxc323 device.
 *                  cpss_dev - cpss device number
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int 
dev_98dxc323_xcat3_clear_int (dev_object_t *dev, uint cpss_dev)
{

    uint rc; 

    rc = dev_98dxc323_xcat3_enable_force_interrupt(dev, cpss_dev, DEV_98DXC323_DISABLE);
    return rc;
    
}


/******************************************************************************
 *
 * Function   :	dev_98dxc323_led_test
 *
 * Description: xCat3 LED test
 *
 * Inputs     :	dev_object_t - pointer to the 98dxc323 device. 
 *                  cpss_dev - cpss device number
 *
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int 
dev_98dxc323_led_test (dev_object_t *dev,  uint cpss_dev)
{

    uint force_data;

    dev_98dxc323_object_t *obj_98dxc323 = (dev_98dxc323_object_t *) dev;
    dev_98dxc323_callout_fvt_t *callout_p = obj_98dxc323->callout_fvt;

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

    msleep(XCAT3_200MS);

    /* turn Link LED on */
    if ((*callout_p->led_class_config)(cpss_dev, 3, FALSE, FALSE, 0, TRUE, 0)) {
	cterr('f',0,"Failed to configure LED class 3");
	return (FAILED);
    }

    if ((*callout_p->led_class_config)(cpss_dev, 4, FALSE, FALSE, 0, TRUE, force_data)) {
	cterr('f',0,"Failed to configure LED class 4");
	return (FAILED);
    }

    msleep(XCAT3_1000MS);
	
    /* check activity blink LED */
    if ((*callout_p->led_class_config)(cpss_dev, 3, FALSE, FALSE, 0, TRUE, 0)) {
	cterr('f',0,"Failed to configure LED class 3");
	return (FAILED);
    }

    if ((*callout_p->led_class_config)(cpss_dev, 4, FALSE, FALSE, 0, TRUE, force_data)) {
	cterr('f',0,"Failed to configure LED class 4");
	return (FAILED);
    }
    msleep(XCAT3_1000MS);


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

    msleep(XCAT3_200MS);

    return (PASSED);
	
}


/******************************************************************************
 *
 * Function   :	dev_98dxc323_10g_kr_test_mode
 *
 * Description:	Set 10G-KR test mode
 *
 * Inputs     :	dev_object_t - pointer to the 98dxc323 device. 
 *
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int 
dev_98dxc323_10g_kr_test_mode (dev_object_t *dev, uint cpss_dev, uint port, uint mode, uint pattern)
{

    dev_98dxc323_object_t *obj_98dxc323 = (dev_98dxc323_object_t *) dev;
    dev_98dxc323_callout_fvt_t *callout_p = obj_98dxc323->callout_fvt;

    if ((*callout_p->set_10g_kr_test_mode)(cpss_dev, port, mode, pattern)) {
	cterr('f',0,"Failed to set_10g_kr_test_mode()");
	return (FAILED);
    }
	
    return (PASSED);
}


/******************************************************************************
 *
 * Function   :	dev_98dxc323_serdes_tx_config_read
 *
 * Description:	Serdes Tx Config Read
 *
 * Inputs     :	dev_object_t - pointer to the 98dxc323 device. 
 *
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int 
dev_98dxc323_serdes_tx_config_read (dev_object_t *dev)
{

    dev_98dxc323_object_t *obj_98dxc323 = (dev_98dxc323_object_t *) dev;
    dev_98dxc323_callout_fvt_t *callout_p = obj_98dxc323->callout_fvt;

    if ((*callout_p->serdes_tx_config_read)()) {
	cterr('f',0,"Failed to serdes_tx_config_read()");
	return (FAILED);
    }
	
    return (PASSED);
}


/******************************************************************************
 *
 * Function   :	dev_98dxc323_serdes_tx_config_write
 *
 * Description:	Serdes Tx Config Write
 *
 * Inputs     :	dev_object_t - pointer to the 98dxc323 device. 
 *
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int 
dev_98dxc323_serdes_tx_config_write (dev_object_t *dev)
{

    dev_98dxc323_object_t *obj_98dxc323 = (dev_98dxc323_object_t *) dev;
    dev_98dxc323_callout_fvt_t *callout_p = obj_98dxc323->callout_fvt;

    if ((*callout_p->serdes_tx_config_write)()) {
	cterr('f',0,"Failed to serdes_tx_config_write()");
	return (FAILED);
    }
	
    return (PASSED);
}


/******************************************************************************
 *
 * Function   :	dev_98dxc323_phy_tx_config_read
 *
 * Description:	Phy Tx Config Read
 *
 * Inputs     :	dev_object_t - pointer to the 98dxc323 device. 
 *
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int 
dev_98dxc323_phy_tx_config_read (dev_object_t *dev)
{

    dev_98dxc323_object_t *obj_98dxc323 = (dev_98dxc323_object_t *) dev;
    dev_98dxc323_callout_fvt_t *callout_p = obj_98dxc323->callout_fvt;

    if ((*callout_p->phy_tx_config_read)()) {
	cterr('f',0,"Failed to phy_tx_config_read()");
	return (FAILED);
    }
	
    return (PASSED);
}


/******************************************************************************
 *
 * Function   :	dev_98dxc323_phy_tx_config_write
 *
 * Description:	Phy Tx Config Write
 *
 * Inputs     :	dev_object_t - pointer to the 98dxc323 device. 
 *
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int 
dev_98dxc323_phy_tx_config_write (dev_object_t *dev)
{

    dev_98dxc323_object_t *obj_98dxc323 = (dev_98dxc323_object_t *) dev;
    dev_98dxc323_callout_fvt_t *callout_p = obj_98dxc323->callout_fvt;

    if ((*callout_p->phy_tx_config_write)()) {
	cterr('f',0,"Failed to phy_tx_config_write()");
	return (FAILED);
    }
	
    return (PASSED);
}


/*------------------------------------------------------------------
 *$Log: dev_98dxc323.c,v $
 *Revision 1.2  2019/12/11 10:10:22  lucywang
 *Merged Nanook to main trunk
 *
 *Revision 1.1.2.7  2019/05/22 08:45:00  chieyang
 *1. Add AC3/88E1680 TX Amp utility 2. Add show Glory FPGA information.
 *
 *Revision 1.1.2.6  2019/05/13 10:37:08  chieyang
 *1. Add 10-KR test mode in ESW utility. 2. Move ESW LED test and utility to LED menu. 3. Modify ESW internal/external test with sub-menu for 24 ports. 4. Modify dreamliner_cpss41 to disable GE1 test for Nanook platform.
 *
 *Revision 1.1.2.5  2019/04/26 07:46:13  chieyang
 *1. Add LED test/utility. 2. Modify 88E1543/88E1680/98DXC323 interrupt test.
 *
 *Revision 1.1.2.4  2019/04/11 09:41:09  chieyang
 *Fixed 98DXC323 all register test error and ESW test message.
 *
 *Revision 1.1.2.3  2019/04/10 08:34:31  chieyang
 *Modify for ESW related test/utility functions.
 *
 *Revision 1.1.2.2  2019/03/21 06:19:20  chieyang
 *Add draft functions for Nanook ESW test.
 *
 *Revision 1.1.2.1  2019/03/18 07:38:20  chieyang
 *Initial Nanook ESW test code.
 *
 *
 *
 *
 *$Endlog$
*/
