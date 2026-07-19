/* $Id: dev_88e6390.c,v 1.2 2019/01/10 06:19:23 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_88e6390_marvell/dev_88e6390.c,v $
 *------------------------------------------------------------------
 *
 * dev_88e6390.c
 *
 * Description:	Marvell 88E6390 Device Driver
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
#include "dev_88e6390.h"
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

static uint32 dev_88e6390_attach(dev_object_t *);
static uint32 dev_88e6390_detach(dev_object_t *);
static uint32 dev_88e6390_restart(dev_object_t *);
static void dev_88e6390_destroy(dev_object_t **);
static int dev_88e6390_smi_rd(dev_object_t *, int, ushort *);
static int dev_88e6390_smi_wr(dev_object_t *, int, ushort);
static int dev_88e6390_reg_rd_util(dev_object_t *);
static int dev_88e6390_reg_wr_util(dev_object_t *);
static int dev_88e6390_phy_reg_rd_util(dev_object_t *);
static int dev_88e6390_phy_reg_wr_util(dev_object_t *);
static int dev_88e6390_c45_phy_reg_rd_util(dev_object_t *);
static int dev_88e6390_c45_phy_reg_wr_util(dev_object_t *);
static int dev_88e6390_reg_rd(dev_object_t *, int, int, ushort *);
static int dev_88e6390_reg_wr(dev_object_t *, int, int, ushort);
static int dev_88e6390_phy_reg_rd(dev_object_t *, int, int, int, ushort *);
static int dev_88e6390_phy_reg_wr(dev_object_t *, int, int, int, ushort);
static int dev_88e6390_c45_phy_reg_rd(dev_object_t *, int, int, int, ushort *);
static int dev_88e6390_c45_phy_reg_wr(dev_object_t *, int, int, int, ushort);
static int dev_88e6390_pwr_up_ge_port(dev_object_t *, int);
static int dev_88e6390_pwr_up_serdes_port(dev_object_t *); 
static int dev_88e6390_set_port_forward(dev_object_t *, int); 
static int dev_88e6390_show_port_status(dev_object_t *, int); 
static int dev_88e6390_enable_phy_port_interrupt(dev_object_t *, int, boolean);
static int dev_88e6390_force_phymac_speed(dev_object_t *, int, int);
static int dev_88e6390_force_phy_speed(dev_object_t *, int, int);
static int dev_88e6390_force_mac_speed(dev_object_t *, int, int);
static int dev_88e6390_phy_mac_lpbk_test_if(dev_object_t *, int, int);
static int dev_88e6390_enable_int_mask_and_reg(dev_object_t *);
static int dev_88e6390_config_pvlan(dev_object_t *, int);
static int dev_88e6390_led_on(dev_object_t *dev, int); 
static int dev_88e6390_led_off(dev_object_t *dev, int); 
static int dev_88e6390_set_testmode_util(dev_object_t *);
static int dev_88e6390_reset(dev_object_t *);
static int dev_88e6390_chk_intr_assert(dev_object_t *);
static int dev_88e6390_chk_intr_deassert(dev_object_t *);
static int dev_88e6390_set_vod_util(dev_object_t *);
static int dev_88e6390_set_vod(dev_object_t *, int, int, int);

/*===================================================================*
 *                    Polling function                               *
 *===================================================================*/
static int dev_88e6390_is_smi_bus_free(dev_object_t *);                        
static int dev_88e6390_polling_phy_reg(dev_object_t *, int, int, int, ushort); 
static int dev_88e6390_polling_port_reg(dev_object_t *, int, int, int, int, ushort);

/*===================================================================*
 *                    Test function                                  *
 *===================================================================*/
static int dev_88e6390_test_reg(dev_object_t *);
static int dev_88e6390_phy_mac_lpbk_test(dev_object_t *, int, int);
static int dev_88e6390_ext_lpbk_test(dev_object_t *, int, int);
static int dev_88e6390_intr_test(dev_object_t *);

/*===================================================================*
 *                    Global variables                               *
 *===================================================================*/
static char mrv_88e6390_err_buf[MRV88E6390_ERR_BUF_SIZE];

static dev_88e6390_pvlan_profile_t vlan_profile[] = 
{
    /* Profile 1 (P1/P2, P0/P3/CPU Port(P5) */
    {{  ESW_PBVM_VLAN_TBL(ESW_PORT3) | ESW_PBVM_VLAN_TBL(ESW_PORT5),
        ESW_PBVM_VLAN_TBL(ESW_PORT2),
        ESW_PBVM_VLAN_TBL(ESW_PORT1),
        ESW_PBVM_VLAN_TBL(ESW_PORT0) | ESW_PBVM_VLAN_TBL(ESW_PORT5),
        0,
        ESW_PBVM_VLAN_TBL(ESW_PORT0) | ESW_PBVM_VLAN_TBL(ESW_PORT3),
        0
    }},
    {{0,}}
};

/* ======== 88E6390 Test Mode Setting ========== */

/* Based on Marvell FAE,
 * Steps to enter 6390 ESW to Test Mode 1, 2 or 4 are:
 * 1. Write Page 0, Reg  9 = 0x1F00 (Set PHY to Master mode)
 * 2. Write Page 0, Reg  0 = 0x9140 (Soft-reset)
 * 3. Write Page 4, Reg 27 = 0x3E80 (Disable Clock on the HSDACP/N by set bit8 to 0)
 * 4. Write Page 6, Reg 26 = 0x8000 (Enable TX_TCLK)
 */
static mrvl_88e6390_phy_setup_t esw_testmode124_steps[] = {
    {PHY_PAGE(0), ESWPHY_1000TCR_ADDR,  0x1F00, 0xFFFF},
    {PHY_PAGE(0), ESWPHY_CCR_ADDR,  0x9140, 0x7B40},
    {PHY_PAGE(4), MRV88E6390_PAGE4_REG27, 0x3E80, 0xFFFF},
    {PHY_PAGE(6), GEPHY_MISC_TEST_REG, 0x8000, 0xFFA0}
};


/* Based on Marvell FAE,
 * Steps to enter 6390 PHY to Test Mode 3 are:
 * 1. Write Page 0, Reg  9 = 0x1700 (Set PHY to Slave mode)
 * 2. Write Page 0, Reg  0 = 0x9140 (Soft-reset)
 * 3. Write Page 4, Reg 27 = 0x3E80 (Disable Clock on the HSDACP/N by set bit8 to 0)
 * 4. Write Page 6, Reg 26 = 0x8000 (Enable TX_TCLK)
 */
static mrvl_88e6390_phy_setup_t esw_testmode3_steps[] = {
    {PHY_PAGE(0), ESWPHY_1000TCR_ADDR,  0x1700, 0xFFFF},
    {PHY_PAGE(0), ESWPHY_CCR_ADDR,      0x9140, 0x7B40},
    {PHY_PAGE(4), MRV88E6390_PAGE4_REG27, 0x3E80, 0xFFFF},
    {PHY_PAGE(6), GEPHY_MISC_TEST_REG,  0x8000, 0xFFA0}
};
/******************************************************************************
 *
 * Name:	mrv88e6390_dev_create()
 *
 * Description:	Create object with various device function
 *        		point to "do nothing"
 *
 * Input:	dev_object_t pointer to the 88E6390 device.
 *		    error reporting function pointer.
 *
 * Returns:	none
 *
 *****************************************************************************/
void mrv88e6390_dev_create (dev_object_t *dev, dev_error_report_t error_report_fn)
{
    dev_object_fvt_t	*dev_fvt;
    dev_88e6390_object_t *obj_88e6390= (dev_88e6390_object_t*)dev;

    /* Allocate memory for the device object */
    if ((dev_fvt = (dev_object_fvt_t *)malloc(sizeof(dev_object_fvt_t))) ==
		            NULL) {
        /* Unable to allocate memory */
        error_report_fn(dev, "malloc failure in 88e6390_dev_create()", 0);
        printf("%s: NULL\n", __func__);
	    return;
    }

    /* Init the device object structure to default "do nothing" */
    init_default_dev_object(dev, dev_fvt);

    obj_88e6390->base.dev_object_fvt->dev_attach	= dev_88e6390_attach;
    obj_88e6390->base.dev_object_fvt->dev_detach	= dev_88e6390_detach;
    obj_88e6390->base.dev_object_fvt->dev_restart	= dev_88e6390_restart;
    obj_88e6390->base.dev_object_fvt->dev_error_report	= error_report_fn;
    obj_88e6390->base.dev_object_fvt->dev_destroy	= dev_88e6390_destroy;
    obj_88e6390->base.dev_object_fvt->dev_name	= "88E6390 Marvell ESW";

    obj_88e6390->callin_fvt = (dev_88e6390_callin_fvt_t *)
                               malloc(sizeof(dev_88e6390_callin_fvt_t));
    obj_88e6390->callout_fvt = (dev_88e6390_callout_fvt_t *)
                                malloc(sizeof(dev_88e6390_callout_fvt_t));

    obj_88e6390->base.dev_state = DEV_STATE_CREATE;
}


/******************************************************************************
 *
 * Name:	dev_88e6390_attach()
 *
 * Description:	Attach the 88E6390 device for use. This function will
 *		        initialize and setup all necessary pointers and bring the
 *        		chip to operation.
 *
 * Input:	Pointer to the 88E6390 device object
 *
 * Returns:	PASSED/FAILED
 *
 *****************************************************************************/
static uint32 dev_88e6390_attach (dev_object_t *dev)
{
    dev_88e6390_object_t *obj_88e6390 = (dev_88e6390_object_t *) dev;

    if (obj_88e6390->callin_fvt == NULL) {
        DEV_ERROR_REPORT(dev, "dev_88e6390_attach() callin malloc", DEV_88E6390_ATTACH);
        return (FAILED);
    }

    if (obj_88e6390->callout_fvt == NULL) {
        DEV_ERROR_REPORT(dev, "dev_88e6390_attach() callout malloc", DEV_88E6390_ATTACH);
        return (FAILED);
    }

    /* init the call in function */
    obj_88e6390->callin_fvt->register_test  = dev_88e6390_test_reg;
    obj_88e6390->callin_fvt->esw_phy_mac_lpbk_test = dev_88e6390_phy_mac_lpbk_test;
    obj_88e6390->callin_fvt->ext_lpbk_test  = dev_88e6390_ext_lpbk_test;
    obj_88e6390->callin_fvt->intr_test = dev_88e6390_intr_test;

    obj_88e6390->callin_fvt->esw_reg_read_util   = dev_88e6390_reg_rd_util;
    obj_88e6390->callin_fvt->esw_reg_write_util  = dev_88e6390_reg_wr_util;
    obj_88e6390->callin_fvt->esw_phy_reg_read_util   = dev_88e6390_phy_reg_rd_util;
    obj_88e6390->callin_fvt->esw_phy_reg_write_util  = dev_88e6390_phy_reg_wr_util;
    obj_88e6390->callin_fvt->esw_c45_phy_reg_read_util   = dev_88e6390_c45_phy_reg_rd_util;
    obj_88e6390->callin_fvt->esw_c45_phy_reg_write_util  = dev_88e6390_c45_phy_reg_wr_util;

    obj_88e6390->callin_fvt->esw_reg_read   = dev_88e6390_reg_rd;
    obj_88e6390->callin_fvt->esw_reg_write  = dev_88e6390_reg_wr;
    obj_88e6390->callin_fvt->esw_phy_reg_read   = dev_88e6390_phy_reg_rd;
    obj_88e6390->callin_fvt->esw_phy_reg_write  = dev_88e6390_phy_reg_wr;
    obj_88e6390->callin_fvt->esw_c45_phy_reg_read   = dev_88e6390_c45_phy_reg_rd;
    obj_88e6390->callin_fvt->esw_c45_phy_reg_write  = dev_88e6390_c45_phy_reg_wr;

    obj_88e6390->callin_fvt->esw_pwr_up_ge_port     = dev_88e6390_pwr_up_ge_port;
    obj_88e6390->callin_fvt->esw_pwr_up_serdes_port  = dev_88e6390_pwr_up_serdes_port;
    obj_88e6390->callin_fvt->esw_set_port_forward = dev_88e6390_set_port_forward;
    obj_88e6390->callin_fvt->esw_enable_phy_port_interrupt = 
                             dev_88e6390_enable_phy_port_interrupt ;
    obj_88e6390->callin_fvt->esw_enable_int_mask_and_reg = 
                             dev_88e6390_enable_int_mask_and_reg;
    obj_88e6390->callin_fvt->esw_config_pvlan = dev_88e6390_config_pvlan;
    obj_88e6390->callin_fvt->esw_set_led_on = dev_88e6390_led_on;
    obj_88e6390->callin_fvt->esw_set_led_off = dev_88e6390_led_off;
    obj_88e6390->callin_fvt->esw_set_testmode_util = dev_88e6390_set_testmode_util;
    obj_88e6390->callin_fvt->esw_set_vod_util = dev_88e6390_set_vod_util;
    obj_88e6390->base.dev_state = DEV_STATE_ATTACH;

    return (PASSED);
}

/******************************************************************************
 *
 * Name:	dev_88e6390_detach()
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
 * Input:	Pointer to the 88E6390 device object
 *
 * Returns:	PASSED/FAILED
 *
 *****************************************************************************/
static uint32 dev_88e6390_detach (dev_object_t *dev)
{
    dev_88e6390_object_t *obj_88e6390 = (dev_88e6390_object_t *) dev;

    /* Init the device object structure to default "do nothing" */
    init_default_dev_object(dev, obj_88e6390->base.dev_object_fvt);

    obj_88e6390->base.dev_state = DEV_STATE_DETACH;

    return (PASSED);

}


/******************************************************************************
 * Name:	dev_88e6390_restart
 *
 * Description:	To restart a previously initialized device without resetting
 *		        the device or changing its configuration.
 *		        For example, during a failover event.
 *
 *		        Change the state of the device from its current state
 *		        to an initial state. Also, dev_state must be assigned the
 *		        value of DEV_STATE_INIT.
 *
 * Input:	dev_object_t pointer to the 88E6390 device
 *
 * Returns:	PASSED/FAILED
 *
 * Assumptions:	The dev_attach() and dev_reconfig_needed() functions has been
 *		        called and successfully executed.
 *
 *****************************************************************************/
static uint32 dev_88e6390_restart (dev_object_t *dev)
{
    dev_88e6390_object_t *obj_88e6390= (dev_88e6390_object_t *) dev;

    obj_88e6390->base.dev_state = DEV_STATE_INIT;
    return (PASSED);
}


/******************************************************************************
 * Name:	dev_88e6390_destroy
 *
 * Description:	Destroy the dev_object structure and free all the resources.
 *
 * Input:	dev_object_t pointer to the 88E6390 device
 *
 * Returns:	none
 *
 * Assumptions:	The dev_attch() function has been called and successfully
 *
 *****************************************************************************/
static void dev_88e6390_destroy (dev_object_t **dev)
{
    dev_88e6390_object_t *obj_88e6390;

    if (dev == NULL) {
        return;
    }

    if (*dev == NULL) {
        return;
    }

    obj_88e6390 = (dev_88e6390_object_t *)*dev;

    if (obj_88e6390->callout_fvt) {
        free(obj_88e6390->callout_fvt);	/* Free callout struct */
    }

    if (obj_88e6390->callin_fvt) {
        free(obj_88e6390->callin_fvt);		/* Free callin struct */
    }

    free(obj_88e6390->base.dev_object_fvt);	/* Free dev_object_t */
}

/******************************************************************************
 *
 * Function:    dev_88e6390_test_reg
 *
 * Description: Tests the 88E6390 registers.
 *              Restores original register values after test.
 *
 * Inputs:  dev_object_t pointer to the 88E6390 device
 *          A device print function vector
 *
 * Outputs: PASSED - No errors encounterd.
 *          FAILED - Errors encounterd.
 *
 * Assumptions: create and dev_attach have to be called first. dev_destroy will
 *              also be called after the exit.
 *
 *****************************************************************************/
static int dev_88e6390_test_reg (dev_object_t *dev)
{
    int    rc = FAILED;
    int    ctr = 0;
    ushort orig_val = 0, test_pattern = REG_PAGE(3), read_back = 0;
    int    start_port = 0, end_port = 0;

    start_port = ESW_PORT1;
    end_port = ESW_PORT8;

    for (ctr = start_port; ctr <= end_port; ctr++) {

        printf("Marvell 88E6390 Register test at Port%d\n",ctr);

        if (dev_88e6390_phy_reg_rd(dev, ctr,
                                  (int)REG_PAGE(0),
                                  (int)ESW_GEPHY_PAGE_ADDR,
                                  &orig_val) != PASSED) {
            printf("%s:%d: Fail at Port:%d\n", __FUNCTION__, __LINE__, ctr);
            printf("%s:%d: Reason: Failed to read original value of page register(%d)\n", 
                   __FUNCTION__, __LINE__, ESW_GEPHY_PAGE_ADDR);
            return (FAILED);
        }

        if (dev_88e6390_phy_reg_wr(dev, ctr,
                                  (int)REG_PAGE(0),
                                  (int)ESW_GEPHY_PAGE_ADDR,
                                  test_pattern) != PASSED) {
            printf("%s:%d: Fail at Port:%d\n", __FUNCTION__, __LINE__, ctr);
            printf("%s:%d: Reason: Failed to write pattern value to page register(%d)\n", 
                   __FUNCTION__, __LINE__, ESW_GEPHY_PAGE_ADDR);
            return (FAILED);
        }

        if (dev_88e6390_phy_reg_rd(dev, ctr,
                                  (int)REG_PAGE(0),
                                  (int)ESW_GEPHY_PAGE_ADDR,
                                  &read_back) != PASSED) {
            printf("%s:%d: Fail at Port:%d\n", __FUNCTION__, __LINE__, ctr);
            printf("%s:%d: Reason: Failed to read value of page register(%d)\n", 
                   __FUNCTION__, __LINE__, ESW_GEPHY_PAGE_ADDR);
            return (FAILED);
        }
        if (read_back == test_pattern) {
            rc = PASSED;
        } else {
            printf("%s:%d: Fail at Port:%d\n", __FUNCTION__, __LINE__, ctr);
            printf("%s:%d: Reason: Data mismatched, test_pattern %#x; read_back %#x.\n\n", 
                   __FUNCTION__, __LINE__, test_pattern, read_back);
            rc = FAILED;
        }

        if (dev_88e6390_phy_reg_wr(dev, ctr,
                               (int)REG_PAGE(0),
                               (int)ESW_GEPHY_PAGE_ADDR,
                               orig_val) != PASSED) {
            printf("%s:%d: Fail at Port:%d\n", __FUNCTION__, __LINE__, ctr);
            printf("%s:%d: Reason: Fail to restore page register(%d)\n", 
                   __FUNCTION__, __LINE__, ESW_GEPHY_PAGE_ADDR);
            return (FAILED);
        }

        if (rc != PASSED) {
            return (FAILED);
        }
    }
    return (PASSED);
}

/******************************************************************************
 *
 * Function:    dev_88e6390_ext_lpbk_test
 *
 * Description: Tests the 88E6390 external loopback.
 *
 * Inputs:  dev_object_t pointer to the 88E6390 device
 *          A device print function vector
 *          start_port -  The external lpbk test start port
 *          end_port   -  End port 
 *
 *
 * Outputs: PASSED - No errors encounterd.
 *          FAILED - Errors encounterd.
 *
 * Assumptions: create and dev_attach have to be called first. dev_destroy will
 *              also be called after the exit.
 *
 *****************************************************************************/
static int dev_88e6390_ext_lpbk_test (dev_object_t *dev, int start_port, int end_port)
{
    dev_88e6390_object_t *obj_88e6390 = (dev_88e6390_object_t *) dev;
    dev_88e6390_callout_fvt_t *callout_p = obj_88e6390->callout_fvt;
    int ctr = 0, speed_ctr = 0;
    int test_speed[] = {SPD_10MBPS, SPD_100MBPS, SPD_1000MBPS};
    int rc = PASSED;
    int esw_port = 0;

    /* Prepare for ESW external loopback test:
     * 1. Disable all ports.
     * 2. Enable PHY Stub.
     */
    for (ctr = start_port; ctr <= end_port; ctr++) {
        /* Disable Switch port */
        esw_port = ctr;
        if (dev_88e6390_reg_wr(dev, esw_port, (int)ESW_PORT_CTL_REG_OFFSET, 
                                  ESW_DISABLE_PORT) != PASSED) {
            printf("%s:%d: Fail at Port:%d\n", __FUNCTION__, __LINE__, ctr);
            printf("%s:%d: Reason: Fail to write port%d SMI reg:0x%02X data:0x%04x\n", 
                   __FUNCTION__, __LINE__, esw_port, ESW_PORT_CTL_REG_OFFSET, ESW_DISABLE_PORT);
            return (FAILED);
        }

    }

    /* Start to test in different speed port by port */
    for (ctr = start_port; ctr <= end_port; ctr++) {
        /* Enable Switch port */
        esw_port = ctr;
        if (dev_88e6390_reg_wr(dev, (int)esw_port, (int)ESW_PORT_CTL_REG_OFFSET,
                               ESW_ENABLE_PORT) != PASSED) {
            printf("%s:%d: Fail at Port:%d\n", __FUNCTION__, __LINE__, ctr);
            printf("%s:%d: Reason: Fail to write port%d SMI reg:0x%02X data:0x%04x\n", 
                   __FUNCTION__, __LINE__, esw_port, ESW_PORT_CTL_REG_OFFSET, ESW_ENABLE_PORT);
            return (FAILED);
        }

        for (speed_ctr = 0; speed_ctr < (sizeof(test_speed)/sizeof(int)); speed_ctr++) {
            /* Enable PHY Stub */
            if (test_speed[speed_ctr] == SPD_1000MBPS) {
                if (dev_88e6390_phy_reg_wr(dev, ctr, (int)PHY_PAGE(6), 
                                   (int)CHECKER_CONTROL_OFFSET, ESW_ENABLE_PHY_STUB) 
                                    != PASSED) {
                    printf("%s:%d: Fail at Port:%d, Speed:%dMbps\n", 
                           __FUNCTION__, __LINE__, ctr, test_speed[speed_ctr]);
                    printf("%s:%d: Reason: Fail to write PHY reg: Page:%d Reg:%d Data:0x%04x\n", 
                           __FUNCTION__, __LINE__, PHY_PAGE(6), CHECKER_CONTROL_OFFSET, ESW_ENABLE_PHY_STUB);
                    return (FAILED);
                }
            }
            
            /* Configure MAC and PHY speed based on testing request */
            if (dev_88e6390_force_phymac_speed(dev, ctr, test_speed[speed_ctr]) 
                != PASSED) {
                printf("%s:%d: Fail at Port:%d, Speed:%dMbps\n", 
                       __FUNCTION__, __LINE__, ctr, speed_ctr);
                printf("%s:%d: Reason: Fail to set PHY port MAC speed\n", 
                       __FUNCTION__, __LINE__);
                return (FAILED);
            }

            printf("Marvell 88E6390 External loopback test at Port%d in Speed:%4dmbps\n", 
                   ctr, test_speed[speed_ctr]);

            rc = (*callout_p->esw_phy_tx_rx_test)();
            if (rc != PASSED) {
                printf("%s:%d: Fail at Port:%d, Speed:%dMbps\n", 
                       __FUNCTION__, __LINE__, ctr, test_speed[speed_ctr]);
                printf("%s:%d: Reason: Fail to run loopback test for Tx/Rx\n", 
                       __FUNCTION__, __LINE__);
                dev_88e6390_show_port_status(dev, ctr);
                return (FAILED);
            }
            msleep(50);
        }

        esw_port = ctr;

        if (dev_88e6390_reg_wr(dev, (int)esw_port, (int)ESW_PORT_CTL_REG_OFFSET,
                               ESW_DISABLE_PORT) != PASSED) {
            printf("%s:%d: Fail at Port:%d\n", __FUNCTION__, __LINE__, ctr);
            printf("%s:%d: Reason: Fail to write port%d SMI reg:0x%02X data:0x%04x\n", 
                   __FUNCTION__, __LINE__, esw_port, ESW_PORT_CTL_REG_OFFSET, ESW_DISABLE_PORT);
            return (FAILED);
        }
    }

    for (ctr = start_port; ctr <= end_port; ctr++) {
        /* Disable PHY Stub */
        if (dev_88e6390_phy_reg_wr(dev, ctr, (int)PHY_PAGE(6), 
                                  (int)CHECKER_CONTROL_OFFSET, ESW_DISABLE_PHY_STUB) 
                                   != PASSED) {
            printf("%s:%d: Fail at Port:%d\n", __FUNCTION__, __LINE__, ctr);
            printf("%s:%d: Reason: Fail to write PHY reg: Page:%d Reg:%d Data:0x%04x\n", 
                   __FUNCTION__, __LINE__, PHY_PAGE(6), CHECKER_CONTROL_OFFSET, ESW_DISABLE_PHY_STUB);
            return (FAILED);
        }

        /* Enable Switch port */
        esw_port = ctr;
        if (dev_88e6390_reg_wr(dev, (int)esw_port, (int)ESW_PORT_CTL_REG_OFFSET,
                               ESW_ENABLE_PORT) != PASSED) {
            printf("%s:%d: Fail at Port:%d\n", __FUNCTION__, __LINE__, ctr);
            printf("%s:%d: Reason: Fail to write port%d SMI reg:0x%02X data:0x%04x\n", 
                   __FUNCTION__, __LINE__, esw_port, ESW_PORT_CTL_REG_OFFSET, ESW_ENABLE_PORT);
            return (FAILED);
        }
    }

    if (rc != PASSED) {
        return (FAILED);
    }
    return (PASSED);
}

/******************************************************************************
 *
 * Function:    dev_88e6390_intr_test
 *
 * Description: Tests the 88E6390 interrupt function.
 *
 * Inputs:  dev_object_t pointer to the 88E6390 device
 *          A device print function vector
 *
 * Outputs: PASSED - No errors encounterd.
 *          FAILED - Errors encounterd.
 *
 * Assumptions: create and dev_attach have to be called first. dev_destroy will
 *              also be called after the exit.
 * Note: For more details of interrupt procedure, please check
 *       the "88E6390X/88E6390/88E6290/88E6190X/88E6190 
 *            Switch Functional Specification"
 *       Doc. No.: MV-S110642-00
 *       Please refer to the following page:
 *       P.278, Switch Global Status Register, EEInt field
 *       P.350, EEPROM Command, EEBusy field
 *
 *****************************************************************************/
static int dev_88e6390_intr_test (dev_object_t *dev)
{
    ushort reg_val;
    int smi_addr, reg_addr;

    /* read Global 2(0x1c) offset 0x13, interrupt is asserted */
    smi_addr = ESW_SMIDEV_GLOB2; /* dev:0x1c */
    reg_addr = IMP_COMM_DBG_REG;
    if (dev_88e6390_reg_rd(dev, smi_addr, reg_addr, &reg_val) == FAILED) {
        printf("%s:%d: Failed to read Device Reg with smi_addr:0x%x reg_addr:0x%x\n", 
               __func__, __LINE__, smi_addr, reg_addr);
        return (FAILED);
    }

    /* check the interrupt pin is asserted */
    if (dev_88e6390_chk_intr_assert(dev) != PASSED) {
        printf("%s:%d: The interrupt pin is not asserted!!\n", __func__, __LINE__);
        return (FAILED);
    }

    /* write Global 2(0x1c) offset 0x14 with data 0x8000, interrupt is de-asserted */
    smi_addr = ESW_SMIDEV_GLOB2; /* dev:0x1c */
    reg_addr = EEPROM_CMD_REG;
    reg_val = EEBUSY;
    if (dev_88e6390_reg_wr(dev, smi_addr, reg_addr, reg_val) == FAILED) {
        printf("%s:%d: Failed to write Device Reg with smi_addr:0x%x reg_addr:0x%x data:0x%x\n", 
               __func__, __LINE__, smi_addr, reg_addr, reg_val);
        return (FAILED);
    }

    /* check the interrupt pin is de-asserted */
    if (dev_88e6390_chk_intr_deassert(dev) != PASSED) {
        printf("%s:%d: The interrupt pin is still asserted!!\n", __func__, __LINE__);
        return (FAILED);
    }
    
    return (PASSED);
}

/**********************************************************************
 *
 * Function: dev_88e6390_smi_rd
 *
 * This function: Reads Marvell ESW registers through callout
 *
 * Input : dev    - dev_object_t pointer to the 88E6390 device
 *	       cmd    - Command register in 88e6390
 *	                if it is cmd_reg the value will be 0x0 
 *	                if it is data_reg the value will be 0x1 
 *	       buf    - points to the data buffer to be read.Refrence 
 *	                it's format from 88e6390 datasheet.
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int dev_88e6390_smi_rd (dev_object_t *dev, int cmd, ushort *buf)
{
    int rc;
    dev_88e6390_object_t *obj_88e6390 = (dev_88e6390_object_t *) dev;
    dev_88e6390_callout_fvt_t *callout_p = obj_88e6390->callout_fvt;

    rc = (*callout_p->rd)(cmd, (ushort *)buf);

    if (rc != PASSED) {
        sprintf(mrv_88e6390_err_buf, "%s: Failed (%#x)", __func__, rc);
        return (FAILED);
    }

    return (PASSED);
}


/**********************************************************************
 *
 * Function: dev_88e6390_reset
 *
 * This function: reset  Marvell ESW registers through callout
 * Input : dev - dev_object_t pointer to the 88E6390 device
 * Output: PASSED/FAILED
 **********************************************************************
 */
static int dev_88e6390_reset (dev_object_t *dev)
{
    dev_88e6390_object_t *obj_88e6390 = (dev_88e6390_object_t *) dev;
    dev_88e6390_callout_fvt_t *callout_p = obj_88e6390->callout_fvt;
    int rc ;

    rc = (*callout_p->reset)();

    if (rc != PASSED) {
        sprintf(mrv_88e6390_err_buf, "%s: Failed (%#x)", __func__, rc);
        return (FAILED);
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: dev_88e6390_smi_wr
 *
 * This function: writes Marvell ESW registers through callout
 *
 * Input : dev     - dev_object_t pointer to the 88E6390 device
 *	       cmd     - Command data register in 88e6390   
 *	                if it is cmd_reg the value will be 0x0 
 *	                if it is data_reg the value will be 0x1 
 *	       regval  - points to the data buffer to be write.Refrence 
 *	                 it's format from 88e6390 datasheet.
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int dev_88e6390_smi_wr (dev_object_t *dev, int cmd, ushort regval)
{
    dev_88e6390_object_t *obj_88e6390 = (dev_88e6390_object_t *) dev;
    dev_88e6390_callout_fvt_t *callout_p = obj_88e6390->callout_fvt;
    int rc ;

    rc = (*callout_p->wr)(cmd, (ushort)regval);

    if (rc != PASSED) {
        sprintf(mrv_88e6390_err_buf, "%s: Failed (%#x)", __func__, rc);
        return (FAILED);
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: dev_88e6390_reg_rd_util
 *
 * This function: Read Marvell ethernet switch registers
 *
 * Input : dev      - dev_object_t pointer to the 88E6390 device
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int dev_88e6390_reg_rd_util (dev_object_t *dev)
{
    int    dev_addr = 0, reg_addr = 0;
    ushort reg_val = 0;

    dev_addr = gethex_answer("Enter SMI device addr.: ", 0, 0, 0x1F);
    reg_addr = gethex_answer("Enter SMI register addr.: ", 0, 0, 0x1F);

    if (dev_88e6390_reg_rd(dev, dev_addr, reg_addr, &reg_val) != PASSED) {
        return (FAILED);
    } else {
        printf("Switch SMI device %#x, register %#x: 0x%04X.\n",
               dev_addr, reg_addr, reg_val);
        return (PASSED);
    }
}

/**********************************************************************
 *
 * Function: dev_88e6390_reg_wr_util
 *
 * This function: writes Marvell Ethernet Switch registers
 *
 * Input : dev      - dev_object_t pointer to the 88E6390 device
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int dev_88e6390_reg_wr_util (dev_object_t *dev)
{
    int    dev_addr = 0, reg_addr = 0;
    ushort reg_val = 0, wr_data = 0;
    
    dev_addr = gethex_answer("Enter SMI device addr.: ", 0, 0, 0x1F);
    reg_addr = gethex_answer("Enter SMI register addr.: ", 0, 0, 0x1F);

    if (dev_88e6390_reg_rd(dev, dev_addr, reg_addr, &reg_val) != PASSED) {
        return (FAILED);
    }
    wr_data = (ushort)gethex_answer("Enter wanted SMI register value: ",
                                    reg_val, 0, 0xFFFF);

    if (dev_88e6390_reg_wr(dev, dev_addr, reg_addr, wr_data) != PASSED) {
        return (FAILED);
    } else {
        printf("Writed 0x%04X to Switch SMI device %#x, register %#x.\n",
               wr_data, dev_addr, reg_addr);
        return (PASSED);
    }
}

/**********************************************************************
 *
 * Function: dev_88e6390_phy_reg_rd
 *
 * This function: read Marvell Ethernet Switch PHY registers
 *
 * Input : dev      - dev_object_t pointer to the 88E6390 device
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int dev_88e6390_phy_reg_rd_util (dev_object_t *dev)
{
    int    phy_port = 0, reg_page, reg_addr = 0;
    ushort reg_val = 0;
    
    phy_port = gethex_answer("Enter port number(0 ~ 0x1A): ", 0x1, 0, 0x1A);
    reg_page = gethex_answer("Enter page number of PHY register(0 ~ 0xff): ",
                             0, 0, 0xff);
    reg_addr = getdec_answer("Enter PHY register addr.(0 ~ 31): ", 0, 0, 31);

    if (dev_88e6390_phy_reg_rd(dev, phy_port, reg_page, reg_addr, &reg_val) 
                               != PASSED) {
        return (FAILED);
    } else {
        printf("ESW port%d PHY: page %d, register %d = 0x%04X.\n",
               phy_port, reg_page, reg_addr, reg_val);
        return (PASSED);
    }
}
/**********************************************************************
 *
 * Function: dev_88e6390_phy_reg_wr_util
 *
 * This function: writes Marvell Ethernet Switch PHY registers
 *
 * Input : dev      - dev_object_t pointer to the 88E6390 device
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int dev_88e6390_phy_reg_wr_util (dev_object_t *dev)
{
    int    phy_port = 0, reg_page, reg_addr = 0;
    ushort reg_val = 0, wr_data = 0;

    phy_port = gethex_answer("Enter port number(0 ~ 0xf): ", 0x1, 0, 0xf);
    reg_page = gethex_answer("Enter page number of PHY register(0 ~ 0xff): ",
                             0, 0, 0xff);
    reg_addr = getdec_answer("Enter PHY register addr.(0 ~ 31): ", 0, 0, 31);

    if (dev_88e6390_phy_reg_rd(dev, phy_port, reg_page, reg_addr, &reg_val) 
                               != PASSED) {
        return (FAILED);
    }

    wr_data = (ushort)gethex_answer("Enter wanted ESW PHY register value: ",
                                    reg_val, 0, 0xFFFF);

    if (dev_88e6390_phy_reg_wr(dev, phy_port, reg_page, reg_addr, wr_data) 
                               != PASSED) {
        return (FAILED);
    } else {
        printf("Writed 0x%04X to ESW port%d PHY: page %d, register %d.\n",
                                    wr_data, phy_port, reg_page, reg_addr);
        return (PASSED);
    }
}

/**********************************************************************
 *
 * Function: dev_88e6390_c45_phy_reg_rd_util
 *
 * This function: read Marvell ESW PHY  registers (Clause 45)
 *
 * Input : dev      - dev_object_t pointer to the 88E6390 device
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int dev_88e6390_c45_phy_reg_rd_util (dev_object_t *dev)
{
    int    phy_port = 0, dev_num, reg_addr = 0;
    ushort rd_buf = 0;

    phy_port = gethex_answer("Enter port number(0 ~ 0xf): ", 0x1, 0, 0xf);
    dev_num = getdec_answer("Enter device number(0 ~ 255): ", 4, 0, 255);
    reg_addr = gethex_answer("Enter register addr.(0 ~ 0xFFFF): ",
                             0x2002, 0, 0xFFFF);

    if (dev_88e6390_c45_phy_reg_rd(dev, phy_port, dev_num, reg_addr, &rd_buf) 
                                   != PASSED) {
        return (FAILED);
    } else {
        printf("SMI C45: port %d, Device%d, register%#x = 0x%04X.\n",
               phy_port, dev_num, reg_addr, rd_buf);
        return (PASSED);
    }
}
/**********************************************************************
 *
 * Function: dev_88e6390_c45_phy_reg_wr_util
 *
 * This function: write Marvell ESW PHY  registers (Clause 45)
 *
 * Input : dev      - dev_object_t pointer to the 88E6390 device
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int dev_88e6390_c45_phy_reg_wr_util (dev_object_t *dev)
{
    int    phy_port = 0, dev_num, reg_addr = 0;
    ushort rd_buf = 0, wr_data = 0;

    phy_port = gethex_answer("Enter port number(0 ~ 0xf): ", 0x1, 0, 0xf);
    dev_num = getdec_answer("Enter device number(0 ~ 255): ", 4, 0, 255);
    reg_addr = gethex_answer("Enter register addr.(0 ~ 0xFFFF): ",
                             0x2002, 0, 0xFFFF);

    if (dev_88e6390_c45_phy_reg_rd(dev, phy_port, dev_num, reg_addr, &rd_buf) 
                                   != PASSED) {
        return (FAILED);
    }
    wr_data = (ushort)gethex_answer("Enter wanted ESW PHY register value: ",
                                    rd_buf, 0, 0xFFFF);

    if (dev_88e6390_c45_phy_reg_wr(dev, phy_port, dev_num, reg_addr, wr_data) 
                                   != PASSED) {
        return (FAILED);
    } else {
        printf("SMI C45: Writed 0x%04X to port%d, Device%d, register%#x.\n",
               wr_data, phy_port, dev_num, reg_addr);
        return (PASSED);
    }
}

/**********************************************************************
 * Function: dev_88e6390_reg_rd
 *
 * This function: Read Marvell ethernet switch registers
 * Input : dev      - dev_object_t pointer to the 88E6390 device
 *         dev_addr - device address
 *         reg_addr - register address
 *         buf      - buffer to store data
 * Output: PASSED/FAILED
 **********************************************************************/
static int dev_88e6390_reg_rd (dev_object_t *dev, int dev_addr, 
                               int reg_addr, ushort *buf)
{
    int    smi_cmd_reg = (int)ESW_SMI_CMD_REG;
    int    smi_data_reg = (int)ESW_SMI_DATA_REG;
    ushort reg_val = 0, wr_data = 0;

    /* [Polling] Checking the SMI bus is available to access */
    if (dev_88e6390_is_smi_bus_free(dev) != PASSED) {
        printf("DBG[%s:%d] SMI bus is busy\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    /* [Set Command] Set command as "Read Data Register" operation */
    wr_data = ((ushort)SMI_CMD_SMIBUSY |
               (ushort)SMI_CMD_SMIMODE_C22 |
               (ushort)SMI_CMD_SMIOP_RD | 
               (ushort)((dev_addr & 0x1f) << 5) |
               (ushort)(reg_addr & 0x1f));

    /* [Write Command] Write command to the "SMI Command Register(0x0)" */
    if (dev_88e6390_smi_wr(dev, smi_cmd_reg, wr_data) != PASSED) {
        printf("DBG[%s:%d] Failed to SMI write command reg.(%#X)\n",
                   __FUNCTION__, __LINE__, smi_cmd_reg);
        return (FAILED);
    } 

    /* [Polling] Checking the SMI bus is available to access */
    if (dev_88e6390_is_smi_bus_free(dev) != PASSED) {
        printf("DBG[%s:%d] SMI bus is busy\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }
 
    /* [Read Data] Read data from "SMI Data Register(0x1)" */
    if (dev_88e6390_smi_rd(dev, smi_data_reg, &reg_val) != PASSED) {
        printf("DBG[%s:%d] Failed to read SMI register 0x%02X.\n",
               __FUNCTION__, __LINE__, smi_data_reg);
        return (FAILED);
    }
    *buf = reg_val;

    return (PASSED);
}

/**********************************************************************
 * Function: dev_88e6390_reg_wr
 *
 * This function: writes Marvell Ethernet Switch registers
 * Input : dev      - dev_object_t pointer to the 88E6390 device
 *         dev_addr - device address
 *         reg_addr - register address
 *         wr_data  - buffer to write data
 * Output: PASSED/FAILED
 **********************************************************************/
static int dev_88e6390_reg_wr (dev_object_t *dev, int dev_addr, int reg_addr,
                                ushort wr_data)
{
    int    smi_cmd_reg = (int)ESW_SMI_CMD_REG;
    int    smi_data_reg = (int)ESW_SMI_DATA_REG;
    ushort cmd_data = 0;

    /* [Polling] Checking the SMI bus is available to access */
    if (dev_88e6390_is_smi_bus_free(dev) != PASSED) {
        printf("DBG[%s:%d] SMI bus is busy\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    /* [Write Data] Write expected data into "SMI Data Register(0x1)" */
    /*              Note: This action just put the data into register,
     *                    doesn't send out to SMI bus. 
     *                    Don't be misunderstood                      */
    if (dev_88e6390_smi_wr(dev, smi_data_reg, wr_data) != PASSED) {
        printf("DBG[%s:%d] Failed to SMI write command reg.(%#X)\n",
                   __FUNCTION__, __LINE__, smi_data_reg);
        return (FAILED);
    } 

    /* [Set Command] Set command as "Write Data Register" operation */
    cmd_data = ((ushort)SMI_CMD_SMIBUSY |
                (ushort)SMI_CMD_SMIMODE_C22 |
                (ushort)SMI_CMD_SMIOP_WR |
                (ushort)((dev_addr & 0x1f) << 5) |
                (ushort)(reg_addr & 0x1f));

    /* [Write Command] Write command to the "SMI Command Register(0x0)" */
    if (dev_88e6390_smi_wr(dev, smi_cmd_reg, cmd_data) != PASSED) {
        printf("DBE[%s:%d] Failed to SMI write command reg.(%#X)\n",
                   __FUNCTION__, __LINE__, smi_cmd_reg);
        return (FAILED);
    } 

    /* [Polling] Checking the SMI bus is available to access */
    if (dev_88e6390_is_smi_bus_free(dev) != PASSED) {
        printf("DBG[%s:%d] SMI bus is busy\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: dev_88e6390_phy_reg_rd
 *
 * This function: read Marvell Ethernet Switch PHY registers
 *
 * Input : dev      - dev_object_t pointer to the 88E6390 device
 *	       phy_port - phy port
 *	       reg_page - register page
 *	       reg_addr - register address
 *	       buf      - the space for read data
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int dev_88e6390_phy_reg_rd (dev_object_t *dev, int phy_port, int reg_page, int reg_addr, ushort *buf)
{
    int    smi_dev = 0, smi_reg = 0, phy_page_reg = (int)PHY_REG(22);
    ushort smi_cmd = 0, wr_data = 0, pattern = 0;

    smi_dev = (int)ESW_SMIDEV_GLOB2; /* dev:0x1c */
    smi_reg = (int)ESW_GLOB2_PC;     /* reg:0x18 */

    pattern = SMI_CMD_SMIBUSY;
    /* Checking the inner device SMI bus is available to access */
    if (dev_88e6390_polling_phy_reg(dev, COMPARE_AND, smi_dev, smi_reg, pattern) != PASSED) {
        printf("DBG[%s:%d] Inner device SMI bus is busy\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    /* If user wants to read value of Page addr. register(reg22),
     * then no need to change page. */
    if (reg_addr == ESW_GEPHY_PAGE_ADDR) {
        smi_cmd = ((ushort)SMI_CMD_SMIBUSY |
                   (ushort)SMI_CMD_SMIMODE_C22 |
                   (ushort)SMI_CMD_SMIOP_RD |
                   (ushort)((phy_port & 0x1f) << 5) |
                   (ushort)(phy_page_reg & 0x1f));

        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("DBG[%s:%d] data for SMI command reg. = 0x%04X.\n",
                   __FUNCTION__, __LINE__, smi_cmd);
        }

        smi_dev = (int)ESW_SMIDEV_GLOB2; /* dev:0x1c */
        smi_reg = (int)ESW_GLOB2_PC;     /* reg:0x18 */

        if (dev_88e6390_reg_wr(dev, smi_dev, smi_reg, smi_cmd) != PASSED) {
            printf("DBG[%s:%d] Failed to write inner SMI device\n"
                   "smi_dev:0x%x smi_reg:0x%x, data:0x%x\n",
                   __FUNCTION__, __LINE__, smi_dev, smi_reg, smi_cmd);
            return (FAILED);
        }

        pattern = SMI_CMD_SMIBUSY;
        /* Checking the inner device SMI bus is available to access */
        if (dev_88e6390_polling_phy_reg(dev, COMPARE_AND, smi_dev, smi_reg, pattern) != PASSED) {
            printf("DBG[%s:%d] Inner device SMI bus is busy\n", __FUNCTION__, __LINE__);
            return (FAILED);
        }

        smi_dev = (int)ESW_SMIDEV_GLOB2; /* dev:0x1c */
        smi_reg = (int)ESW_GLOB2_PD;     /* reg:0x19 */
        if (dev_88e6390_reg_rd(dev, smi_dev, smi_reg, buf) != PASSED) {
            printf("DBG[%s:%d] Failed to write inner SMI device\n"
                   "smi_dev:0x%x smi_reg:0x%x\n",
                   __FUNCTION__, __LINE__, smi_dev, smi_reg);
            return (FAILED);
        }

        return (PASSED);
    }

    /* Change page */
    wr_data = (ushort)(reg_page & 0xFF);
    smi_dev = (int)ESW_SMIDEV_GLOB2; /* dev:0x1c */
    smi_reg = (int)ESW_GLOB2_PD;     /* reg:0x19 */
    if (dev_88e6390_reg_wr(dev, smi_dev, smi_reg, wr_data) != PASSED) {
        printf("DBG[%s:%d] Failed to write inner SMI device\n"
               "smi_dev:0x%x smi_reg:0x%x, data:0x%x\n",
               __FUNCTION__, __LINE__, smi_dev, smi_reg, wr_data);
        return (FAILED);
    }

    smi_cmd = ((ushort)SMI_CMD_SMIBUSY |
               (ushort)SMI_CMD_SMIMODE_C22 |
               (ushort)SMI_CMD_SMIOP_WR |
               (ushort)((phy_port & 0x1f) << 5) |
               (ushort)(phy_page_reg & 0x1f));

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("DBG[%s:%d] data for SMI command reg. = 0x%04X.\n",
               __FUNCTION__, __LINE__, smi_cmd);
    }

    smi_dev = (int)ESW_SMIDEV_GLOB2; /* dev:0x1c */
    smi_reg = (int)ESW_GLOB2_PC;     /* reg:0x18 */
    if (dev_88e6390_reg_wr(dev, smi_dev, smi_reg, smi_cmd) != PASSED) {
        printf("DBG[%s:%d] Failed to write inner SMI device\n"
               "smi_dev:0x%x smi_reg:0x%x, data:0x%x\n",
               __FUNCTION__, __LINE__, smi_dev, smi_reg, smi_cmd);
        return (FAILED);
    }

    /* Read register value */
    smi_cmd = ((ushort)SMI_CMD_SMIBUSY |
               (ushort)SMI_CMD_SMIMODE_C22 |
               (ushort)SMI_CMD_SMIOP_RD |
               (ushort)((phy_port & 0x1f) << 5) |
               (ushort)(reg_addr & 0x1f));

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("DBG[%s:%d] data for SMI command reg. = 0x%04X.\n",
               __FUNCTION__, __LINE__, smi_cmd);
    }

    pattern = SMI_CMD_SMIBUSY;
    /* Checking the inner device SMI bus is available to access */
    if (dev_88e6390_polling_phy_reg(dev, COMPARE_AND, smi_dev, smi_reg, pattern) != PASSED) {
        printf("DBG[%s:%d] Inner device SMI bus is busy\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    smi_dev = (int)ESW_SMIDEV_GLOB2; /* dev:0x1c */
    smi_reg = (int)ESW_GLOB2_PC;     /* reg:0x18 */
    if (dev_88e6390_reg_wr(dev, smi_dev, smi_reg, smi_cmd) != PASSED) {
        printf("DBG[%s:%d] Failed to write inner SMI device\n"
               "smi_dev:0x%x smi_reg:0x%x, data:0x%x\n",
               __FUNCTION__, __LINE__, smi_dev, smi_reg, smi_cmd);
        return (FAILED);
    }

    pattern = SMI_CMD_SMIBUSY;
    /* Checking the inner device SMI bus is available to access */
    if (dev_88e6390_polling_phy_reg(dev, COMPARE_AND, smi_dev, smi_reg, pattern) != PASSED) {
        printf("DBG[%s:%d] Inner device SMI bus is busy\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    smi_dev = (int)ESW_SMIDEV_GLOB2; /* dev:0x1c */
    smi_reg = (int)ESW_GLOB2_PD;     /* reg:0x19 */
    if (dev_88e6390_reg_rd(dev, smi_dev, smi_reg, buf) != PASSED) {
        printf("DBG[%s:%d] Failed to write inner SMI device\n"
               "smi_dev:0x%x smi_reg:0x%x\n",
               __FUNCTION__, __LINE__, smi_dev, smi_reg);
        return (FAILED);
    }
    return (PASSED);
}


/**********************************************************************
 *
 * Function: dev_88e6390_phy_reg_wr
 *
 * This function: writes Marvell Ethernet Switch PHY registers
 *
 * Input : dev      - dev_object_t pointer to the 88E6390 device
 *	       phy_port - phy port
 *	       reg_page - register page
 *	       reg_addr - register address
 *	       wr_in    - data to be write in
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int dev_88e6390_phy_reg_wr (dev_object_t *dev, int phy_port, int reg_page, int reg_addr, ushort wr_in)
{
    int    smi_dev = 0, smi_reg = 0, phy_page_reg = (int)PHY_REG(22);
    ushort smi_cmd = 0, wr_data = 0, pattern = 0;

    /* Check if SMI bus is available */
    smi_dev = (int)ESW_SMIDEV_GLOB2; /* dev:0x1c */
    smi_reg = (int)ESW_GLOB2_PC;     /* reg:0x18 */

    pattern = SMI_CMD_SMIBUSY;
    /* Checking the inner device SMI bus is available to access */
    if (dev_88e6390_polling_phy_reg(dev, COMPARE_AND, smi_dev, smi_reg, pattern) != PASSED) {
        printf("DBG[%s:%d] Inner device SMI bus is busy\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    /* Change page */
    /* If user wants to read value of Page addr. register(reg22),
     * then no need to change page.
     */
    if (reg_addr == ESW_GEPHY_PAGE_ADDR) {
        wr_data = (ushort)(wr_in & 0xFF);
    } else {
        wr_data = (ushort)(reg_page & 0xFF);
    }
    smi_dev = (int)ESW_SMIDEV_GLOB2; /* dev:0x1c */
    smi_reg = (int)ESW_GLOB2_PD;     /* reg:0x19 */
    if (dev_88e6390_reg_wr(dev, smi_dev, smi_reg, wr_data) != PASSED) {
        printf("DBG[%s:%d] Failed to write inner SMI device\n"
               "smi_dev:0x%x smi_reg:0x%x, data:0x%x\n",
               __FUNCTION__, __LINE__, smi_dev, smi_reg, smi_cmd);
        return (FAILED);
    }

    smi_cmd = ((ushort)SMI_CMD_SMIBUSY |
               (ushort)SMI_CMD_SMIMODE_C22 |
               (ushort)SMI_CMD_SMIOP_WR |
               (ushort)((phy_port & 0x1f) << 5) |
               (ushort)(phy_page_reg & 0x1f));

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("DBG[%s:%d] data for SMI command reg. = 0x%04X.\n",
               __FUNCTION__, __LINE__, smi_cmd);
    }

    smi_dev = (int)ESW_SMIDEV_GLOB2; /* dev:0x1c */
    smi_reg = (int)ESW_GLOB2_PC;     /* reg:0x18 */
    if (dev_88e6390_reg_wr(dev, smi_dev, smi_reg, smi_cmd) != PASSED) {
        printf("DBG[%s:%d] Failed to write inner SMI device\n"
               "smi_dev:0x%x smi_reg:0x%x, data:0x%x\n",
               __FUNCTION__, __LINE__, smi_dev, smi_reg, smi_cmd);
        return (FAILED);
    }

    pattern = SMI_CMD_SMIBUSY;
    /* Checking the inner device SMI bus is available to access */
    if (dev_88e6390_polling_phy_reg(dev, COMPARE_AND, smi_dev, smi_reg, pattern) != PASSED) {
        printf("DBG[%s:%d] Inner device SMI bus is busy\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    /* If user wants to read value of Page addr. register(reg22),
     * then no need to change page.
     */
    if (reg_addr == ESW_GEPHY_PAGE_ADDR) {
        return (PASSED);
    }

    /* Write register value */
    smi_dev = (int)ESW_SMIDEV_GLOB2; /* dev:0x1c */
    smi_reg = (int)ESW_GLOB2_PD;     /* reg:0x19 */
    if (dev_88e6390_reg_wr(dev, smi_dev, smi_reg, wr_in) != PASSED) {
        printf("DBG[%s:%d] Failed to write inner SMI device\n"
               "smi_dev:0x%x smi_reg:0x%x, data:0x%x\n",
               __FUNCTION__, __LINE__, smi_dev, smi_reg, wr_in);
        return (FAILED);
    }

    smi_cmd = ((ushort)SMI_CMD_SMIBUSY |
               (ushort)SMI_CMD_SMIMODE_C22 |
               (ushort)SMI_CMD_SMIOP_WR |
               (ushort)((phy_port & 0x1f) << 5) |
               (ushort)(reg_addr & 0x1f));

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("DBG[%s:%d] data for SMI command reg. = 0x%04X.\n",
               __FUNCTION__, __LINE__, smi_cmd);
    }

    smi_dev = (int)ESW_SMIDEV_GLOB2; /* dev:0x1c */
    smi_reg = (int)ESW_GLOB2_PC;     /* reg:0x18 */
    if (dev_88e6390_reg_wr(dev, smi_dev, smi_reg, smi_cmd) != PASSED) {
        printf("DBG[%s:%d] Failed to write inner SMI device\n"
               "smi_dev:0x%x smi_reg:0x%x, data:0x%x\n",
               __FUNCTION__, __LINE__, smi_dev, smi_reg, smi_cmd);
        return (FAILED);
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: dev_88e6390_c45_phy_reg_rd
 *
 * This function: read Marvell ESW PHY  registers (Clause 45)
 *
 * Input : dev      - dev_object_t pointer to the 88E6390 device
 *	       phy_port - which port want to read
 *         dev_num - the device index reference 88E6390 datasheet
 *         reg_addr - register address
 *         buf - buffer to load read back data
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int dev_88e6390_c45_phy_reg_rd (dev_object_t *dev, int phy_port, 
                                       int dev_num, int reg_addr, ushort *buf)
{
    int    smi_dev = 0, smi_reg = 0;
    ushort reg_val = 0, smi_cmd = 0, wr_data = 0, pattern = 0;

    /* Set register address to SMI PHY Data Reg(0x19) */
    wr_data = (ushort)(reg_addr & 0xFFFF);
    smi_dev = (int)ESW_SMIDEV_GLOB2; /* dev:0x1c */
    smi_reg = (int)ESW_GLOB2_PD;     /* reg:0x19 */
    if (dev_88e6390_reg_wr(dev, smi_dev, smi_reg, wr_data) != PASSED) {
        printf("%s:%d Failed to write ESW SMI dev %#x reg. %#x\n",
               __FUNCTION__, __LINE__, smi_dev, smi_reg);
        return (FAILED);
    }

    /* Set access command to write address to SMI PHY Command Reg(0x18) */
    smi_cmd = ((ushort)SMI_CMD_SMIBUSY |
               (ushort)SMI_CMD_SMIMODE_C45 |
               (ushort)SMIOP_C45_WR_ADDR |
               (ushort)((phy_port & 0x1f) << 5) |
               (ushort)(dev_num & 0x1f));

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("DBG[%s:%d] data for SMI command reg. = 0x%04X.\n",
               __FUNCTION__, __LINE__, smi_cmd);
    }

    smi_dev = (int)ESW_SMIDEV_GLOB2; /* dev:0x1c */
    smi_reg = (int)ESW_GLOB2_PC;     /* reg:0x18 */
    if (dev_88e6390_reg_wr(dev, smi_dev, smi_reg, smi_cmd) != PASSED) {
        printf("%s:%d Failed to write ESW SMI dev %#x reg. %#x\n",
               __FUNCTION__, __LINE__, smi_dev, smi_reg);
        return (FAILED);
    }

    smi_dev = (int)ESW_SMIDEV_GLOB2; /* dev:0x1c */
    smi_reg = (int)ESW_GLOB2_PC;     /* reg:0x18 */
    pattern = SMI_CMD_SMIBUSY;
    /* Checking the inner device SMI bus is available to access */
    if (dev_88e6390_polling_phy_reg(dev, COMPARE_AND, smi_dev, smi_reg, pattern) != PASSED) {
        printf("DBG[%s:%d] Inner device SMI bus is busy\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    /* Set access command to read data back to SMI PHY Command Reg(0x18) */
    smi_cmd = ((ushort)SMI_CMD_SMIBUSY |
               (ushort)SMI_CMD_SMIMODE_C45 |
               (ushort)SMIOP_C45_RD_DATA |
               (ushort)((phy_port & 0x1f) << 5) |
               (ushort)(dev_num & 0x1f));

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("DBG[%s:%d] data for SMI command reg. = 0x%04X.\n",
               __FUNCTION__, __LINE__, smi_cmd);
    }

    smi_dev = (int)ESW_SMIDEV_GLOB2; /* dev:0x1c */
    smi_reg = (int)ESW_GLOB2_PC;     /* reg:0x18 */
    if (dev_88e6390_reg_wr(dev, smi_dev, smi_reg, smi_cmd) != PASSED) {
        printf("%s:%d Failed to write ESW SMI dev %#x reg. %#x\n",
               __FUNCTION__, __LINE__, smi_dev, smi_reg);
        return (FAILED);
    }

    smi_dev = (int)ESW_SMIDEV_GLOB2; /* dev:0x1c */
    smi_reg = (int)ESW_GLOB2_PC;     /* reg:0x18 */
    pattern = SMI_CMD_SMIBUSY;
    /* Checking the inner device SMI bus is available to access */
    if (dev_88e6390_polling_phy_reg(dev, COMPARE_AND, smi_dev, smi_reg, pattern) != PASSED) {
        printf("DBG[%s:%d] Inner device SMI bus is busy\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    /* Read back the register value from SMI PHY Data Reg(0x19) */
    reg_val = 0;
    smi_dev = (int)ESW_SMIDEV_GLOB2; /* dev:0x1c */
    smi_reg = (int)ESW_GLOB2_PC;     /* reg:0x18 */
    if (dev_88e6390_reg_rd(dev, smi_dev, smi_reg, &reg_val) != PASSED) {
        printf("%s:%d Failed to SMI read ESW SMI dev %#x reg. %#x\n",
               __FUNCTION__, __LINE__, smi_dev, smi_reg);
        return (FAILED);
    }
    *buf = reg_val;    

    return (PASSED);
}
/**********************************************************************
 *
 * Function: dev_88e6390_c45_phy_reg_wr
 *
 * This function: write Marvell ESW PHY  registers (Clause 45)
 *
 * Input : dev      - dev_object_t pointer to the 88E6390 device
 *	       phy_port - which port want to write
 *         dev_num - the device index reference 88E6390 datasheet
 *         reg_addr - register address
 *         wr_in - data to write in register
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int dev_88e6390_c45_phy_reg_wr (dev_object_t *dev, int phy_port, 
                                       int dev_num, int reg_addr, ushort wr_in)
{
    int    smi_dev = 0, smi_reg = 0;
    ushort smi_cmd = 0, wr_data = 0, pattern = 0;

    /* Set register address to SMI PHY Data Reg(0x19) */
    wr_data = (ushort)(reg_addr & 0xFFFF);
    smi_dev = (int)ESW_SMIDEV_GLOB2; /* dev:0x1c */
    smi_reg = (int)ESW_GLOB2_PD;     /* reg:0x19 */
    if (dev_88e6390_reg_wr(dev, smi_dev, smi_reg, wr_data) != PASSED) {
        printf("%s:%d Failed to write ESW SMI dev %#x reg. %#x\n",
               __FUNCTION__, __LINE__, smi_dev, smi_reg);
        return (FAILED);
    }

    /* Set access command to write address to SMI PHY Command Reg(0x18) */
    smi_cmd = ((ushort)SMI_CMD_SMIBUSY |
               (ushort)SMI_CMD_SMIMODE_C45 |
               (ushort)SMIOP_C45_WR_ADDR |
               (ushort)((phy_port & 0x1f) << 5) |
               (ushort)(dev_num & 0x1f));

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("DBG[%s:%d] data for SMI command reg. = 0x%04X.\n",
               __FUNCTION__, __LINE__, smi_cmd);
    }

    smi_dev = (int)ESW_SMIDEV_GLOB2; /* dev:0x1c */
    smi_reg = (int)ESW_GLOB2_PC;     /* reg:0x18 */
    if (dev_88e6390_reg_wr(dev, smi_dev, smi_reg, smi_cmd) != PASSED) {
        printf("%s:%d Failed to write ESW SMI dev %#x reg. %#x\n",
               __FUNCTION__, __LINE__, smi_dev, smi_reg);
        return (FAILED);
    }

    pattern = SMI_CMD_SMIBUSY;
    /* Checking the inner device SMI bus is available to access */
    if (dev_88e6390_polling_phy_reg(dev, COMPARE_AND, smi_dev, smi_reg, pattern) != PASSED) {
        printf("DBG[%s:%d] Inner device SMI bus is busy\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    /* Set write in data to SMI PHY Data Reg(0x19) */
    wr_data = wr_in;
    smi_dev = (int)ESW_SMIDEV_GLOB2; /* dev:0x1c */
    smi_reg = (int)ESW_GLOB2_PD;     /* reg:0x19 */
    if (dev_88e6390_reg_wr(dev, smi_dev, smi_reg, wr_data) != PASSED) {
        printf("%s:%d Failed to write ESW SMI dev %#x reg. %#x\n",
               __FUNCTION__, __LINE__, smi_dev, smi_reg);
        return (FAILED);
    }

    /* Set access command to start write data to SMI PHY Command Reg(0x18) */
    smi_cmd = ((ushort)SMI_CMD_SMIBUSY |
               (ushort)SMI_CMD_SMIMODE_C45 |
               (ushort)SMIOP_C45_WR_DATA |
               (ushort)((phy_port & 0x1f) << 5) |
               (ushort)(dev_num & 0x1f));

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("DBG[%s:%d] data for SMI command reg. = 0x%04X.\n",
               __FUNCTION__, __LINE__, smi_cmd);
    }

    smi_dev = (int)ESW_SMIDEV_GLOB2; /* dev:0x1c */
    smi_reg = (int)ESW_GLOB2_PC;     /* reg:0x18 */
    if (dev_88e6390_reg_wr(dev, smi_dev, smi_reg, smi_cmd) != PASSED) {
        printf("%s:%d Failed to write ESW SMI dev %#x reg. %#x\n",
               __FUNCTION__, __LINE__, smi_dev, smi_reg);
        return (FAILED);
    }

    pattern = SMI_CMD_SMIBUSY;
    /* Checking the inner device SMI bus is available to access */
    if (dev_88e6390_polling_phy_reg(dev, COMPARE_AND, smi_dev, smi_reg, pattern) != PASSED) {
        printf("DBG[%s:%d] Inner device SMI bus is busy\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: dev_88e6390_show_port_status
 *
 * This function: show the port status when some test fail
 *
 * Input : dev - dev_object_t pointer to the 88E6390 device
 *         port - port number
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int dev_88e6390_show_port_status (dev_object_t *dev, int port) 
{
    ushort result = 0;
    uint   speed = 0;
    int    rc = FAILED;
    int    reg_page = (int)PHY_PAGE(0);
    int    reg_addr = 0;
    ushort reg_val = 0;

    if (rc == FAILED) {
        printf("%s;%d: Creating object failed!!\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    /* Get value of Copper Auto-nego Adv. register(4_0) */
    reg_addr = (int)COP_AUTONEG_ADV_REG4;
    if (dev_88e6390_phy_reg_rd(dev, port, reg_page, reg_addr, 
                                &reg_val) != PASSED) {
        printf("%s(%d): Failed to read Page%d, Reg%d.\n",
               __FUNCTION__, __LINE__, reg_page, reg_addr);
        return (FAILED);
    }
    printf("Port%d PHY Copper Auto-Nego Adv Reg(%d_%d) = 0x%04X.\n",
           port, reg_addr, reg_page, reg_val);

    /* Get value of Copper Specific Status register 1(17_0) */
    reg_addr = (int)COP_STATUS_REG17;
    reg_val = 0;
    if (dev_88e6390_phy_reg_rd(dev, port, reg_page, reg_addr, 
                                &reg_val) != PASSED) {
        printf("%s(%d): Failed to read Page%d, Reg%d.\n",
               __FUNCTION__, __LINE__, reg_page, reg_addr);
        return (FAILED);
    }

    result = ((reg_val & (ushort)COP_P0R17_SPEED) >> COP_P0R17_SPEED_OFFSET);

    switch (result) {
    case COP_P0R17_SPEED_1000:
        speed = SPD_1000MBPS;
    break;
    case COP_P0R17_SPEED_100:
        speed = SPD_100MBPS;
    break;
    case COP_P0R17_SPEED_10:
        speed = SPD_10MBPS;
    break;
    default:
        printf("Unknown speed value: %d.\n", result);
    break;
    }   
    printf("PHY Speed is %d Mbps\n", speed);

    if (reg_val & (ushort)COP_P0R17_DUPLEX_FULL) {
        printf("%s:%d: PHY is Full Duplex\n", __FUNCTION__, __LINE__);
    } else {
        printf("%s:%d: PHY is Half Duplex\n", __FUNCTION__, __LINE__);
    }
   
    if (reg_val & (ushort)COP_P0R17_COP_LINK_UP) {
        printf("%s:%d: Copper Link Up\n", __FUNCTION__, __LINE__);
    } else { 
        printf("%s:%d: Copper Link Down\n", __FUNCTION__, __LINE__);
    }
   
    if (reg_val & (ushort)COP_P0R17_GLOBAL_LINK_UP) {
        printf("%s:%d: Global Link Status is Up\n", __FUNCTION__, __LINE__);
    } else { 
        printf("%s:%d: Global Link Status is Down\n", __FUNCTION__, __LINE__);
    }
    return (PASSED);
}

/**********************************************************************
 *
 * Function: dev_88e6390_pwr_up_ge_port
 *
 * This function: Power up GE port
 *
 * Input : dev  - dev_object_t pointer to the 88E6390 device
 *         port - port number 
 *
 * Output: PASSED/FAILED
 * Note  : According to "Info of reference", it indicates that
 *         the description of Copper Control Register(Page:0 Reg:0).
 *         The purpose of this function is to set the 
 *         "Power Down(bit[11])" as 0 for normal operation.
 *
 *         If Power Down is...
 *           ... 1: Power down
 *           ... 0: Normal operation
 *         Due to the default value of this bit is 0 after reset the 
 *         device, hence, run this function before loopbacl test is
 *         necessary!!.
 *         
 *         [Info of reference]:
 *         Document: Link Street 88E6390X/88E6390/88E6190X/88E6390 
 *                   PHY and SERDES Functional Specification
 *         Doc. No.: MV-S110662-00
 *         Section : 3.2 (Gigabit PHY Registers)
 *         Page    : P.52
 **********************************************************************
 */
static int dev_88e6390_pwr_up_ge_port (dev_object_t *dev, int port) 
{
    ushort reg_val = 0;

    /* Page:0 Reg:0 */
    int page_addr = PHY_PAGE(0);
    int reg_addr  = ESWPHY_CCR_ADDR;
    
    if (dev_88e6390_phy_reg_rd(dev, port, page_addr, reg_addr, &reg_val) != PASSED) {
        printf("%s:%d Failed to read ESW port%d, page%d reg%d.",
                   __FUNCTION__, __LINE__, port, 0, 0);
        return (FAILED);
    }
    reg_val &= (short)(~ESW_CCR_PWRDWN);

    if (dev_88e6390_phy_reg_wr(dev, port, page_addr, reg_addr, reg_val) != PASSED) {
        printf("%s:%d Failed to write phy reg  port%d, page%d, reg%d.",
                   __FUNCTION__, __LINE__, port, 0, 0);
        return (FAILED);
    }
    return (PASSED);
}

/**********************************************************************
 *
 * Function: dev_88e6390_pwr_up_serdes_port
 *
 * This function: Power up ESW to CPU SERDES port
 * Input : dev  - dev_object_t pointer to the 88E6390 device
 * Output: PASSED/FAILED
 * Note  : According to "Info of reference", it indicates that
 *         the description of 1000BASE-X/SMIII Control Register
 *         (Device 4, Reg 0x2000).
 *         The purpose of this function is to set the 
 *         "Power Down(bit[11])" as 0 for normal operation.
 *
 *         If Power Down is...
 *           ... 1: Power down
 *           ... 0: Normal
 *         Due to the default value of this bit is 0 after reset the 
 *         device, hence, run this function before loopbacl test is
 *         necessary!!.
 *         
 *         [Info of reference]:
 *         Document: Link Street 88E6390X/88E6390/88E6190X/88E6390 
 *                   PHY and SERDES Functional Specification
 *         Doc. No.: MV-S110662-00
 *         Section : 2.5 (Power Management)
 *                   3.3.4 (SGMII Register Description)
 *         Page    : P.42 / P.128
 **********************************************************************
 */
static int dev_88e6390_pwr_up_serdes_port (dev_object_t *dev) 
{
    /* SERDES Port:9, Dev:4 Reg:0x2000 */
    int phy_port = ESW_SERDES_PORT9; 
    int dev_num  = ESW_SGMII_DEVNUM;
    int reg_addr = ESW_SGMII_CONTR_REG;
    ushort reg_val = 0, rd_buf = 0;

    if (dev_88e6390_c45_phy_reg_rd(dev, phy_port, dev_num, reg_addr, &rd_buf) !=PASSED) {
        printf("DBG[%s:%d] Failed to read SERDES port via Clause 45 at "
               "Port:%d Dev:%d Reg:0x%x\n", __FUNCTION__, __LINE__,
               phy_port, dev_num, reg_addr); 
    }

    /* clear "Power Down field(bit[11])" as 0, to power up SERDES port */
    reg_val &= (short)(~ESW_SGMII_PWRDWN);

    if (dev_88e6390_c45_phy_reg_wr(dev, phy_port, dev_num, reg_addr, reg_val) !=PASSED) {
        printf("DBG[%s:%d] Failed to write SERDES port via Clause 45 at "
               "Port:%d Dev:%d Reg:0x%x Data:0x%x\n", __FUNCTION__, __LINE__,
               phy_port, dev_num, reg_addr, reg_val); 
    }
    return (PASSED);
}

/**********************************************************************
 *
 * Function: dev_88e6390_set_port_forward
 *
 * This function: Set port forward
 *
 * Input : dev  - dev_object_t pointer to the 88E6390 device 
 *         port - port number 
 *        
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int dev_88e6390_set_port_forward (dev_object_t *dev, int port) 
{
    ushort reg_val = 0; 
    int reg_addr = (int)ESW_PORTCTR_REG;

    if (dev_88e6390_reg_rd(dev, port, reg_addr, &reg_val) != PASSED) {
            printf("%s:%d Failed to read ESW port%d, reg%d.",
                   __FUNCTION__, __LINE__, port, reg_addr);
            return (FAILED);
    }
    reg_val |= (ushort)(ESW_PCR_FORWARD);

    if (dev_88e6390_reg_wr(dev, port, reg_addr, reg_val) != PASSED) {
            printf("%s:%d Failed to set ESW port%d forwarding.",
                   __FUNCTION__, __LINE__, port);
            return (FAILED);
    }

    msleep(20);

    /* confirm port in forwarding mode */
    reg_val = 0;
    if (dev_88e6390_reg_rd(dev, port, reg_addr, &reg_val) != PASSED) {
        printf("%s:%d Failed to read ESW port%d, reg%d.",
                   __FUNCTION__, __LINE__, port, reg_addr);
        return (FAILED);
    }

    if ((reg_val & (ushort)ESW_PCR_FORWARD) != (ushort)ESW_PCR_FORWARD) {
        printf("%s: Failed to set ESW port%d forwarding mode.",
                   __FUNCTION__, port);
        return (FAILED);
    }
    return (PASSED);
}

/**********************************************************************
 *
 * Function: dev_88e6390_enable_phy_port_interrupt
 *
 * This function: Set phy port interrupt enable
 *
 * Input : dev  - dev_object_t pointer to the 88E6390 device 
 *         port - port number 
 *        
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int dev_88e6390_enable_phy_port_interrupt (dev_object_t *dev, int port, boolean enable) 
{
    if (enable == TRUE) {
        if (dev_88e6390_phy_reg_wr(dev, port, PHY_PAGE(0), COP_SPEC_INT_EN_REG, 
                                   ENABLE_ALL_INT_PHY_REG) != PASSED) {
            printf("%s:%d: \n Can not write phy page: %#x, phy reg: %#x\n",
                   __FUNCTION__, __LINE__, PHY_PAGE(0), COP_SPEC_INT_EN_REG); 
            return (FAILED);
        }
    } else { 
        if (dev_88e6390_phy_reg_wr(dev, port, PHY_PAGE(0), COP_SPEC_INT_EN_REG,
                                   DISABLE_ALL_INT_PHY_REG) != PASSED) {
            printf("%s:%d: \n Can not write phy page: %#x, phy reg: %#x\n",
                   __FUNCTION__, __LINE__, PHY_PAGE(0), COP_SPEC_INT_EN_REG); 
            return (FAILED);
        }
    }
    return (PASSED);
}

/**********************************************************************
 *
 * Function: dev_esw_force_phymac_speed
 *
 * Description: Function to force switch MAC and PHY in specific speed.
 *              This is for testing purpose.
 *
 * Inputs     : port_num - port number that want to config.
 *              speed_opt - speed that want to set(10/100/1000mbps)
 * Outputs    : PASSED/FAILED
 **********************************************************************
 */
static int dev_88e6390_force_phymac_speed (dev_object_t *dev, int port_num, int speed_opt)
{
    int    reg_page = (int)REG_PAGE(0);
    int    reg_addr = (int)REG_ADDR(17);
    ushort pattern = 0;

    /* Configure PHY side */
    if (dev_88e6390_force_phy_speed(dev, port_num, speed_opt) != PASSED) {
        printf("%s(%d) Failed to force Port%d PHY at %dmbps.\n",
               __func__, __LINE__, port_num, speed_opt);
        return (FAILED);
    }

    /* Configure MAC side */
    if (dev_88e6390_force_mac_speed(dev, port_num, speed_opt) != PASSED) {
        printf("%s(%d) Failed to force Port%d MAC at %dmbps.\n",
               __func__, __LINE__, port_num, speed_opt);
        return (FAILED);
    }

    /* Confirm port status */
    /* Based on datasheet, we can confirm PHY link status,
     * linked speed and duplex from Copper Specific Status Register(17_0)
     */
    pattern = (ushort)(ESWPHY_CSSR1_RESOLVED |
                       ESWPHY_CSSR1_RT_LINK_UP |
                       ESWPHY_CSSR1_COP_LINK_UP |
                       ESWPHY_CSSR1_FULLDUP);

    switch(speed_opt) {
        case SPD_10MBPS:
            pattern |= (ushort)ESWPHY_CSSR1_SPD_10MBPS;
            break;
        case SPD_100MBPS:
            pattern |= (ushort)ESWPHY_CSSR1_SPD_100MBPS;
            break;
        case SPD_1000MBPS:
            pattern |= (ushort)ESWPHY_CSSR1_SPD_1000MBPS;
            break;
        default:  
            printf("%s(%d): Unsupported speed(%d).\n",
                   __func__, __LINE__, speed_opt);
            return (FAILED);
            break;
    }

    if (dev_88e6390_polling_port_reg(dev, COMPARE_AND_EQL, 
                                     port_num, reg_page, reg_addr, pattern) != PASSED) {
        printf("DBG[%s:%d] PHY port MAC is not linkup at "
               "Port:%d Page:%d Reg:%d\n", __FUNCTION__, __LINE__,
               port_num, reg_page, reg_addr);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : dev_esw_force_phy_speed
 * Description: Function to force switch PHY in specific speed.
 *              This is for testing purpose.
 * Inputs     : port_num - port number that want to config.
 *              speed_opt - speed that want to set(10/100/1000mbps)
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int dev_88e6390_force_phy_speed (dev_object_t *dev, int port_num, int speed_opt)
{
    int    reg_page = (int)REG_PAGE(0);
    int    reg_addr = (int)REG_ADDR(0);
    ushort reg_val = 0, chk_val = 0, pattern = 0;

    /* Based on datasheet, to force PHY at specific speed needs to
     * configure PHY Copper Control Register(page0, reg0; 0_0).
     */
    /* Config. speed */
    switch(speed_opt) {
        case SPD_10MBPS:
            chk_val = (ushort)COP_SPD_10Mbps;
            break;
        case SPD_100MBPS:
            chk_val = (ushort)COP_SPD_100Mbps;
            break;
        case SPD_1000MBPS:
            chk_val = (ushort)COP_SPD_1000Mbps;
            break;
        default:  
            printf("%s(%d): Unsupported speed(%d).\n",
                   __func__, __LINE__, speed_opt);
            return (FAILED);
            break;
    }

    /* Disable Auto-negotiation and force Duplex mode to Full-duplex */
    chk_val |= (ushort)ESWPHY_CCR_DUPLEX_FULL;

    /* Based on datasheet, software reset is needed for speed change. */
    reg_val = (ushort)(chk_val | ESWPHY_CCR_COP_RST);

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\n\nDBG[%s(%d)] Value that wanted write to ESW port%d "
               "PHY reg. %d_%d = 0x%04X.\n\n",
               __func__, __LINE__, port_num, reg_addr, reg_page, reg_val);
    }

    if (dev_88e6390_phy_reg_wr(dev, port_num, reg_page, reg_addr, reg_val) != PASSED) {
        printf("%s(%d) Failed to write ESW port%d PHY register %d_%d.\n",
               __func__, __LINE__, port_num, reg_addr, reg_page);
        return (FAILED);
    }

    pattern = ESWPHY_CCR_COP_RST;
    /* checking the soft-reset is done */
    if (dev_88e6390_polling_port_reg(dev, COMPARE_AND, 
                                     port_num, reg_page, reg_addr, pattern) != PASSED) {
        printf("DBG[%s:%d] The soft-reset is not finished at "
               "Port:%d Page:%d Reg:%d\n", __FUNCTION__, __LINE__,
               port_num, reg_page, reg_addr);
        return (FAILED);
    }

    pattern = chk_val;
    /* checking the config data is matched */
    if (dev_88e6390_polling_port_reg(dev, COMPARE_EQL, 
                                     port_num, reg_page, reg_addr, pattern) != PASSED) {
        printf("DBG[%s:%d] The config data is not matched at "
               "Port:%d Page:%d Reg:%d\n", __FUNCTION__, __LINE__,
               port_num, reg_page, reg_addr);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : dev_88e6390_force_mac_speed
 * Description: Function to force switch MAC in specific speed.
 *              This is for testing purpose.
 * Inputs     : port_num - port number that want to config.
 *              speed_opt - speed that want to set(10/100/1000mbps)
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int dev_88e6390_force_mac_speed (dev_object_t *dev, int port_num, int speed_opt)
{
    int    esw_port = 0;
    int    reg_addr = (int)ESW_PCR_ADDR;
    ushort reg_val = 0, chk_val = 0, spd_val = 0, pattern = 0;

    esw_port = port_num;

    /* Config. speed */
    switch(speed_opt) {
        case SPD_10MBPS:
            spd_val = (ushort)ESW_PCR_10MBPS;
            break;
        case SPD_100MBPS:
            spd_val = (ushort)ESW_PCR_100MBPS;
            break;
        case SPD_1000MBPS:
            spd_val = (ushort)ESW_PCR_1000MBPS;
            break;
        default:  
            printf("%s(%d): Unsupported speed(%d).\n",
                   __func__, __LINE__, speed_opt);
            return (FAILED);
            break;
    }

    /* Check if MAC speed is expected */
    chk_val = (ushort)(ESW_PCR_F_LINKUP |
                       ESW_PCR_FORCE_LINK |
                       ESW_PCR_F_FULLDPX |
                       ESW_PCR_FORCE_DPX |
                       spd_val);

    chk_val |= (ushort)ESW_PCR_FORCE_SPEED;

    if (dev_88e6390_reg_rd(dev, esw_port, reg_addr, &reg_val) != PASSED) {
        printf("%s(%d) Failed to read ESW port%d Reg.0x%02X\n",
               __func__, __LINE__, port_num, reg_addr);
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\n\nDBG[%s(%d)] Now ESW port%d Reg.0x%02X = 0x%04X.\n",
               __func__, __LINE__, port_num, reg_addr, reg_val);
        printf("DBG[%s(%d)] check value = 0x%04X.\n\n",
               __func__, __LINE__, chk_val);
    }

    /* If yes, return PASS directly; if no, change MAC speed.*/
    if (reg_val == chk_val) {
        return (PASSED);
    }

    /* Force link down MAC before change its speed based on datasheet */
    reg_val &= (ushort)(~(ESW_PCR_F_LINKUP | ESW_PCR_FORCE_LINK));
    reg_val |= (ushort)ESW_PCR_FORCE_LINK;
    if (dev_88e6390_reg_wr(dev, esw_port, reg_addr, reg_val) != PASSED) {
        printf("%s(%d) Failed to write ESW port%d Reg.0x%02X\n",
               __func__, __LINE__, port_num, reg_addr);
        return (FAILED);
    }

    pattern = ESW_PSR_LINK;
    /* Confirm ESW port MAC is linked down.
     * By checking Link status bit(bit 11) of Port Status Reg.(0x0) */
    if (dev_88e6390_polling_phy_reg(dev, COMPARE_AND, esw_port, reg_addr, pattern) != PASSED) {
        printf("DBG[%s:%d] Inner device SMI bus is busy\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    /* Config. ESW testing port MAC as expected */
    reg_addr = (int)ESW_PCR_ADDR;

    if (dev_88e6390_reg_wr(dev, esw_port, reg_addr, chk_val) != PASSED) {
        printf("%s(%d) Failed to write ESW port%d Reg.0x%02X\n",
               __func__, __LINE__, port_num, reg_addr);
        return (FAILED);
    }

    pattern = chk_val;
    /* Confirm ESW testing port MAC is configured correctly */
    if (dev_88e6390_polling_phy_reg(dev, COMPARE_EQL, esw_port, reg_addr, chk_val) != PASSED) {
        printf("DBG[%s:%d] Failed to config MAC speed\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : dev_88e6390_enable_int_mask_and_reg
 * Description: Function to force switch MAC in specific speed.
 *              This is for testing purpose.
 * Inputs     : port_num - port number that want to config.
 *              speed_opt - speed that want to set(10/100/1000mbps)
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int dev_88e6390_enable_int_mask_and_reg (dev_object_t *dev)
{
    ushort dev_addr, reg_addr, reg_val;
    /* Enable switch global reg(1B and 1C) for interrupt mask */
    /* detail in 88E6390  datasheet*/
    dev_addr = ESW_GLOBAL1_REG;
    reg_addr = ESW_GLOBAL_CONTROL_REG;
    reg_val = ENABLE_ESW_GLOBAL_CONTROL_REG;

    if (dev_88e6390_reg_wr(dev, dev_addr, reg_addr, reg_val) != PASSED) {
        printf("%s(%d): Failed to write ESW port%d SMI register 0x%02X.\n",
                __FUNCTION__, __LINE__, dev_addr, reg_addr);
        return (FAILED);
    }

    dev_addr = ESW_GLOBAL2_REG;
    reg_addr = ESW_INT_MASK_REG;
    reg_val = ENABLE_ESW_INT_MASK_REG;

    if (dev_88e6390_reg_wr(dev, dev_addr, reg_addr, reg_val) != PASSED) {
        printf("%s(%d): Failed to write ESW port%d SMI register 0x%02X.\n",
                __FUNCTION__, __LINE__, dev_addr, reg_addr);
        return (FAILED);
    }
    return (PASSED);
}
/*******************************************************************************
 *
 * Function   : dev_88e6390_phy_mac_lpbk_test_if
 * Description: Function to do CPU to switch PHY MAC loopback set.
 * Inputs     : port_num - Number of ESW port.
 *              speed_opt - test speed(10/100/1000mbps)
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int dev_88e6390_phy_mac_lpbk_test_if (dev_object_t *dev, int port_num, int speed_opt) {
    dev_88e6390_object_t *obj_88e6390 = (dev_88e6390_object_t *) dev;
    dev_88e6390_callout_fvt_t *callout_p = obj_88e6390->callout_fvt;
    int    reg_page = 0, reg_addr = 0;
    ushort phy_reg = 0, pattern = 0;
    int    smi_dev_addr = 0;
    ushort smi_reg = 0, smi_spd_chk = 0;
    int    ctr = 0, start_port = 0, end_port = 0;

    if ((port_num < (int)ESW_PORT1) || (port_num > (int)ESW_PORT8)) {
        printf("%s:%d: Fail at Port:%d, Speed:%dMbps\n", 
               __FUNCTION__, __LINE__, port_num, speed_opt);
        printf("%s:%d: Reason: Unsupported ESW port\n", 
               __FUNCTION__, __LINE__);
        return (FAILED);
    }

    printf("Marvell 88E6390 MAC loopback test at Port%d in Speed:%4dmbps\n", port_num, speed_opt);

    if ((speed_opt != SPD_10MBPS) && (speed_opt != SPD_100MBPS) &&
        (speed_opt != SPD_1000MBPS)) {
        printf("%s:%d: Fail at Port:%d, Speed:%dMbps\n", 
               __FUNCTION__, __LINE__, port_num, speed_opt);
        printf("%s:%d: Reason: Unsupported test speed\n", 
               __FUNCTION__, __LINE__);
        return (FAILED);
    }

    /* 2. Configure ESW PHY */
    /* 2-1. Set ESW PHY MAC speed */
    reg_page = REG_PAGE(2);
    reg_addr = REG_ADDR(21);
    phy_reg = 0;
    if (dev_88e6390_phy_reg_rd(dev, port_num, reg_page, reg_addr, &phy_reg) != PASSED) {
        printf("%s:%d: Fail at Port:%d, Speed:%dMbps\n", 
               __FUNCTION__, __LINE__, port_num, speed_opt);
        printf("%s:%d: Reason: Fail to read PHY reg: Page:%d Reg:%d\n", 
               __FUNCTION__, __LINE__, reg_page, reg_addr);
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s(%d): ESW port%d PHY reg %d_%d = 0x%04X.\n",
               __FUNCTION__, __LINE__, port_num, reg_addr, reg_page, phy_reg);
    }

    phy_reg &= (ushort)(~ESWPHY_MSCR2_MACSPD_MSK);
    switch(speed_opt) {
        case SPD_10MBPS:
            phy_reg |= (ushort)ESWPHY_MSCR2_MACSPD_10MBPS;
            break;
        case SPD_100MBPS:
            phy_reg |= (ushort)ESWPHY_MSCR2_MACSPD_100MBPS;
            break;
        case SPD_1000MBPS:
            phy_reg |= (ushort)ESWPHY_MSCR2_MACSPD_1000MBPS;
            break;
        default:  
            printf("%s:%d: Fail at Port:%d, Speed:%dMbps\n", 
                   __FUNCTION__, __LINE__, port_num, speed_opt);
            printf("%s:%d: Reason: Unsupported test speed\n", 
                   __FUNCTION__, __LINE__);
            return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s(%d): ESW port%d PHY reg %d_%d set data = 0x%04X.\n",
               __FUNCTION__, __LINE__, port_num, reg_addr, reg_page, phy_reg);
    }

    if (dev_88e6390_phy_reg_wr(dev, port_num, reg_page, reg_addr, 
                                  phy_reg) != PASSED) {
        printf("%s:%d: Fail at Port:%d, Speed:%dMbps\n", 
               __FUNCTION__, __LINE__, port_num, speed_opt);
        printf("%s:%d: Reason: Fail to write PHY reg: Page:%d Reg:%d Data:0x%04x\n", 
               __FUNCTION__, __LINE__, reg_page, reg_addr, phy_reg);
        return (FAILED);
    }

    /* 2-2. Set PHY Loopback mode */
    reg_page = REG_PAGE(0);
    reg_addr = REG_ADDR(0);
    phy_reg = 0;
    if (dev_88e6390_phy_reg_rd(dev, port_num, reg_page, reg_addr, &phy_reg) != PASSED) {
        printf("%s:%d: Fail at Port:%d, Speed:%dMbps\n", 
               __FUNCTION__, __LINE__, port_num, speed_opt);
        printf("%s:%d: Reason: Fail to read PHY reg: Page:%d Reg:%d\n", 
               __FUNCTION__, __LINE__, reg_page, reg_addr);
        return (FAILED);
    }

    phy_reg &= (ushort)(~ESWPHY_CCR_AN_EN);
    phy_reg |= (ushort)ESWPHY_CCR_COP_RST;

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s(%d): ESW port%d PHY reg %d_%d set data = 0x%04X.\n",
               __FUNCTION__, __LINE__, port_num, reg_addr, reg_page, phy_reg);
    }

    if (dev_88e6390_phy_reg_wr(dev, port_num, reg_page, reg_addr,
                                  phy_reg) != PASSED) {
        printf("%s:%d: Fail at Port:%d, Speed:%dMbps\n", 
               __FUNCTION__, __LINE__, port_num, speed_opt);
        printf("%s:%d: Reason: Fail to write PHY reg: Page:%d Reg:%d Data:0x%04x\n", 
               __FUNCTION__, __LINE__, reg_page, reg_addr, phy_reg);
        return (FAILED);
    }

    pattern = ESWPHY_CCR_COP_RST;
    /* checking the soft-reset is done */
    if (dev_88e6390_polling_port_reg(dev, COMPARE_AND, 
                                     port_num, reg_page, reg_addr, pattern) != PASSED) {
        printf("DBG[%s:%d] The soft-reset is not finished at "
               "Port:%d Page:%d Reg:%d\n", __FUNCTION__, __LINE__,
               port_num, reg_page, reg_addr);
        return (FAILED);
    }

    if (dev_88e6390_phy_reg_rd(dev, port_num, reg_page, reg_addr,
                               &phy_reg) != PASSED) {
        printf("%s:%d: Fail at Port:%d, Speed:%dMbps\n", 
               __FUNCTION__, __LINE__, port_num, speed_opt);
        printf("%s:%d: Reason: Fail to read PHY reg: Page:%d Reg:%d\n", 
               __FUNCTION__, __LINE__, reg_page, reg_addr);
        return (FAILED);
    }

    phy_reg |= (ushort)ESWPHY_CCR_LPBK;
    if (dev_88e6390_phy_reg_wr(dev, port_num, reg_page, reg_addr,
                                  phy_reg) != PASSED) {
        printf("%s:%d: Fail at Port:%d, Speed:%dMbps\n", 
               __FUNCTION__, __LINE__, port_num, speed_opt);
        printf("%s:%d: Reason: Fail to write PHY reg: Page:%d Reg:%d Data:0x%04x\n", 
               __FUNCTION__, __LINE__, reg_page, reg_addr, phy_reg);
        return (FAILED);
    }

    /* 3. Config. ESW port */
    smi_dev_addr = port_num;
    start_port = (int)ESW_PORT0;
    end_port = (int)ESW_PORT3;

    /* 3-1. Config. ESW port SMI register 0x1 */
    reg_addr = (int)REG_ADDR(1);
    smi_reg = 0;
    if (dev_88e6390_reg_rd(dev, smi_dev_addr, reg_addr, &smi_reg) != PASSED) {
        printf("%s:%d: Fail at Port:%d, Speed:%dMbps\n", 
               __FUNCTION__, __LINE__, port_num, speed_opt);
        printf("%s:%d: Reason: Fail to read port%d SMI reg:0x%02X\n", 
               __FUNCTION__, __LINE__, port_num, reg_addr);
        return (FAILED);
    }

    if ((smi_reg & (ushort)(ESW_PCR_FORCE_LINK | ESW_PCR_F_LINKUP)) !=
        (ushort)(ESW_PCR_FORCE_LINK)) {
        smi_reg |= (ushort)ESW_PCR_FORCE_LINK;
        smi_reg &= (ushort)(~ESW_PCR_F_LINKUP);

        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("%s(%d): ESW port%d SMI reg. 0x%02X set data = 0x%04X.\n",
                   __FUNCTION__, __LINE__, port_num, reg_addr, smi_reg);
        }

        if (dev_88e6390_reg_wr(dev, smi_dev_addr, reg_addr, smi_reg) != PASSED) {
            printf("%s:%d: Fail at Port:%d, Speed:%dMbps\n", 
                   __FUNCTION__, __LINE__, port_num, speed_opt);
            printf("%s:%d: Reason: Fail to write port%d SMI reg:0x%02X data:0x%04x\n", 
                   __FUNCTION__, __LINE__, port_num, reg_addr, smi_reg);
            return (FAILED);
        }
    }

    smi_reg = (ushort)(ESW_PCR_FORCE_SPEED | ESW_PCR_FORCE_LINK |
                       ESW_PCR_FORCE_DPX | ESW_PCR_F_FULLDPX);

    switch(speed_opt) {
        case SPD_10MBPS:
            break;
        case SPD_100MBPS:
            smi_reg |= (ushort)ESW_PCR_100MBPS;
            smi_spd_chk = (ushort)ESW_PSR_100MBPS;
            break;
        case SPD_1000MBPS:
            smi_reg |= (ushort)ESW_PCR_1000MBPS;
            smi_spd_chk = (ushort)ESW_PSR_1000MBPS;
            break;
        default:  
            printf("%s:%d: Fail at Port:%d, Speed:%dMbps\n", 
                   __FUNCTION__, __LINE__, port_num, speed_opt);
            printf("%s:%d: Reason: Unsupported test speed\n", 
                   __FUNCTION__, __LINE__);
            return (FAILED);
    }

    smi_reg |= (ushort)ESW_PCR_F_LINKUP;

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s(%d): ESW port%d SMI reg. 0x%02X set data = 0x%04X.\n",
               __FUNCTION__, __LINE__, port_num, reg_addr, smi_reg);
    }

    if (dev_88e6390_reg_wr(dev, smi_dev_addr, reg_addr, smi_reg) != PASSED) {
        printf("%s:%d: Fail at Port:%d, Speed:%dMbps\n", 
               __FUNCTION__, __LINE__, port_num, speed_opt);
        printf("%s:%d: Reason: Fail to write port%d SMI reg:0x%02X data:0x%04x\n", 
               __FUNCTION__, __LINE__, port_num, reg_addr, smi_reg);
        return (FAILED);
    }

    msleep(ESW_ACCESS_WAITTIME);

    reg_addr = (int)REG_ADDR(0);
    smi_reg = 0;
    if (dev_88e6390_reg_rd(dev, smi_dev_addr, reg_addr, &smi_reg) != PASSED) {
        printf("%s:%d: Fail at Port:%d, Speed:%dMbps\n", 
               __FUNCTION__, __LINE__, port_num, speed_opt);
        printf("%s:%d: Reason: Fail to read port%d SMI reg:0x%02X\n", 
               __FUNCTION__, __LINE__, port_num, reg_addr);
        return (FAILED);
    }

    if ((smi_reg & (ushort)ESW_PSR_LINK) != (ushort)ESW_PSR_LINKUP) {
        printf("%s:%d: Fail at Port:%d, Speed:%dMbps\n", 
               __FUNCTION__, __LINE__, port_num, speed_opt);
        printf("%s:%d: Reason: Fail to force link up at ESW PHY port:%d MAC\n", 
               __FUNCTION__, __LINE__, port_num);
        return (FAILED);
    }

    if ((smi_reg & (ushort)ESW_PSR_DPX) != (ushort)ESW_PSR_FULLDPX) {
        printf("%s:%d: Fail at Port:%d, Speed:%dMbps\n", 
               __FUNCTION__, __LINE__, port_num, speed_opt);
        printf("%s:%d: Reason: Fail to set full duplex at ESW PHY port:%d MAC\n", 
               __FUNCTION__, __LINE__, port_num);
        return (FAILED);
    }

    if ((smi_reg & (ushort)ESW_PSR_SPD_MSK) != smi_spd_chk) {
        printf("%s:%d: Fail at Port:%d, Speed:%dMbps\n", 
               __FUNCTION__, __LINE__, port_num, speed_opt);
        printf("%s:%d: Reason: Fail to set speed at ESW PHY port:%d MAC\n", 
               __FUNCTION__, __LINE__, port_num);
        return (FAILED);
    }

    /* 3-2. Disable all other ESW ports */
    reg_addr = ESW_PORT_CTL_REG_OFFSET;
    for (ctr = start_port; ctr <= end_port; ctr++) {
        if (ctr == port_num) {
            continue;
        }

        smi_dev_addr = ctr;

        smi_reg = 0;
        if (dev_88e6390_reg_rd(dev, smi_dev_addr, reg_addr, &smi_reg) != PASSED) {
            printf("%s:%d: Fail at Port:%d, Speed:%dMbps\n", 
                   __FUNCTION__, __LINE__, port_num, speed_opt);
            printf("%s:%d: Reason: Fail to read port%d SMI reg:0x%02X\n", 
                   __FUNCTION__, __LINE__, port_num, reg_addr);
            return (FAILED);
        }

        if ((smi_reg & (ushort)ESW_PCR_PS_MSK) != (ushort)ESW_PCR_PORT_DIS) {
            smi_reg &= (ushort)(~ESW_PCR_PS_MSK);

            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("%s(%d): ESW port%d SMI reg. 0x%02X set data = 0x%04X.\n",
                       __FUNCTION__, __LINE__, port_num, reg_addr, smi_reg);
            }

            if (dev_88e6390_reg_wr(dev, smi_dev_addr, reg_addr, smi_reg) != PASSED) {
                printf("%s:%d: Fail at Port:%d, Speed:%dMbps\n", 
                       __FUNCTION__, __LINE__, port_num, speed_opt);
                printf("%s:%d: Reason: Fail to write port%d SMI reg:0x%02X data:0x%04x\n", 
                       __FUNCTION__, __LINE__, port_num, reg_addr, smi_reg);
                return (FAILED);
            }
        }
    }

    /* 4. Run SGMII loopback test */
    if ((*callout_p->esw_phy_tx_rx_test)() != PASSED) {
        printf("%s:%d: Fail at Port:%d, Speed:%dMbps\n", 
               __FUNCTION__, __LINE__, port_num, speed_opt);
        printf("%s:%d: Reason: Fail to run loopback test for Tx/Rx\n", 
               __FUNCTION__, __LINE__);
        dev_88e6390_show_port_status(dev, ctr);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : dev_88e6390_phy_mac_lpbk_test
 * Description: Function to do CPU to switch PHY MAC loopback set.
 * Inputs     : port_num - Number of ESW port.
 *              speed_opt - test speed(10/100/1000mbps)
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int dev_88e6390_phy_mac_lpbk_test (dev_object_t *dev, int start_port, int end_port) {
    int port_ctr;
    for (port_ctr = start_port; port_ctr <= end_port; port_ctr++) {
        if(dev_88e6390_phy_mac_lpbk_test_if(dev, port_ctr, SPD_10MBPS) != PASSED) {
            printf("%s:%d: Fail at Port:%d\n", __FUNCTION__, __LINE__, port_ctr);
            printf("%s:%d: Reason: Failed at 10Mbps MAC loopback test\n", 
                   __FUNCTION__, __LINE__);
            return (FAILED);
        }
        if(dev_88e6390_phy_mac_lpbk_test_if(dev, port_ctr, SPD_100MBPS) != PASSED) {
            printf("%s:%d: Fail at Port:%d\n", __FUNCTION__, __LINE__, port_ctr);
            printf("%s:%d: Reason: Failed at 100Mbps MAC loopback test\n", 
                   __FUNCTION__, __LINE__);
            return (FAILED);
        }
        if(dev_88e6390_phy_mac_lpbk_test_if(dev, port_ctr, SPD_1000MBPS) != PASSED) {
            printf("%s:%d: Fail at Port:%d\n", __FUNCTION__, __LINE__, port_ctr);
            printf("%s:%d: Reason: Failed at 1000Mbps MAC loopback test\n", 
                   __FUNCTION__, __LINE__);
            return (FAILED);
        }
        if(dev_88e6390_reset(dev) != PASSED) {
            printf("%s:%d: Fail at Port:%d\n", __FUNCTION__, __LINE__, port_ctr);
            printf("%s:%d: Reason: Fail to reset ESW after MAC loopback test\n", 
                   __FUNCTION__, __LINE__);
        }
   } 
   return (PASSED);
}
/*******************************************************************************
 *
 * Function   : dev_88e6390_config_pvlan
 * Description: Configures Port-based VLAN
 * Inputs     : which_profile - Profile
 *              Profile-1:
 *                - P0/P2, P1/P3/CPU Port(P5)
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int dev_88e6390_config_pvlan (dev_object_t *dev, int which_profile)
{
    int ix, smi_addr;
    int reg_addr = (int)ESW_PORT_VLAN_REG; 
    ushort reg_val = 0;
    dev_88e6390_pvlan_profile_t *applied_profile;

    if (which_profile >= DEV_88E6390_VLAN_PROFILE_END) {
        printf("%s: VLAN Profile (%d) is invalid\n", __func__, which_profile);
        return (FAILED);
    }

    applied_profile = &vlan_profile[which_profile];

    /* Configures VLAN Table on each port */
    for (ix = 0; ix < ESW_PORT6; ix++) {
        smi_addr = ix; 

        if (dev_88e6390_reg_rd(dev, smi_addr, reg_addr, &reg_val) == FAILED) {
            printf("%s: Failed to read Port %d VLAN Table\n", __func__, ix);
            return (FAILED);
        }

        reg_val &= (ushort)(~ESW_PBVM_VLAN_TBL_MSK);
        reg_val |= applied_profile->port_vtable[ix];

        if (dev_88e6390_reg_wr(dev, smi_addr, reg_addr, reg_val) == FAILED) {
            printf("%s: Failed to write to Port %d VLAN Table\n", __func__, ix);
            return (FAILED);
        }
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: dev_88e6390_led_on
 *
 * This function: Turn on led with specified port
 *
 * Input : dev  - dev_object_t pointer to the 88E6390 device 
 *         port - port number 
 *        
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int dev_88e6390_led_on (dev_object_t *dev, int port) 
{
    ushort reg_val = 0; 
    int reg_addr = (int)ESW_LED_CONTR_REG;

    /* Write 88e6390 LED Register to turn on light */
    reg_val = ((ESW_LCR_UPDATE) | (ESW_LCR_LED1_F_ON) | (ESW_LCR_LED0_F_ON));
    if (dev_88e6390_reg_wr(dev, port, reg_addr, reg_val) != PASSED) {
            printf("%s:%d Failed to set ESW LED ON port%d.",
                   __FUNCTION__, __LINE__, port);
            return (FAILED);
    }
    return (PASSED);
}

/**********************************************************************
 *
 * Function: dev_88e6390_led_off
 *
 * This function: Turn off led with specified port
 *
 * Input : dev  - dev_object_t pointer to the 88E6390 device 
 *         port - port number 
 *        
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int dev_88e6390_led_off (dev_object_t *dev, int port) 
{
    ushort reg_val = 0; 
    int reg_addr = (int)ESW_LED_CONTR_REG;

    /* Write 88e6390 LED Register to turn on light */
    reg_val = ((ESW_LCR_UPDATE) | (ESW_LCR_LED1_F_OFF) | (ESW_LCR_LED0_F_OFF));
    if (dev_88e6390_reg_wr(dev, port, reg_addr, reg_val) != PASSED) {
            printf("%s:%d Failed to set ESW LED OFF  port%d.",
                   __FUNCTION__, __LINE__, port);
            return (FAILED);
    }
    return (PASSED);
}

/******************************************************************************
 *
 * Function:	dev_88e6390_set_testmode_util
 *
 * Description:	Set test mode
 *
 * Inputs:	dev_object_t pointer to the 88E6390 device
 *
 * Outputs:	PASSED - No errors encounterd.
 *              FAILED - Errors encounterd.
 *
 * Assumptions:	create and dev_attach have to be called first. dev_destroy will
 *		        also be called after the exit.
 *
 *****************************************************************************/
static int dev_88e6390_set_testmode_util (dev_object_t *dev)
{
    int test_mode = 0;;
    int ctr = 0, total_steps = 0, port_num = 0;
    uint16_t testmode_val = 0;
    mrvl_88e6390_phy_setup_t *step_ptr;

    printf("ESW(Marvell 6390) Port numner:\n");
    port_num = gethex_answer("Enter Port number (1-8)", 1, 1, 8);

    printf("ESW(Marvell 6390) Supported TestMode:\n");
    printf("[0] Normal Mode.\n");
    printf("[1] Transmit Waveform Test.\n");
    printf("[2] Transmit Jitter Test (Master).\n");
    printf("[3] Transmit Jitter Test (Slave).\n");
    printf("[4] Transmit Distortion Test.\n");
    test_mode = gethex_answer("Enter Test mode (0-4)", 0, 0, 7);


   if ((test_mode == ESW_TESTMODE1) || (test_mode == ESW_TESTMODE2) ||
        (test_mode == ESW_TESTMODE4)) {
        step_ptr = &esw_testmode124_steps[0];
        total_steps = sizeof(esw_testmode124_steps) / sizeof(mrvl_88e6390_phy_setup_t);

        /* 1. Enable Test mode 1: 0x3F00
         * 2. Enable Test mode 2: 0x5F00
         * 3. Enable Test mode 4: 0x9F00
         */
        if (test_mode == ESW_TESTMODE1) {
            testmode_val = ESW_TESTMODE1_REG_VAL;
        } else if (test_mode == ESW_TESTMODE2) {
            testmode_val = ESW_TESTMODE2_REG_VAL;
        } else {
            testmode_val = ESW_TESTMODE4_REG_VAL;
        }
    } else if (test_mode == ESW_TESTMODE3) {
        step_ptr = &esw_testmode3_steps[0];
        total_steps = sizeof(esw_testmode3_steps) / sizeof(mrvl_88e6390_phy_setup_t);

        /* Enable Test mode 3: 0x7700 */
        testmode_val = ESW_TESTMODE3_REG_VAL;
    } else if (test_mode == ESW_TESTMODE_NORMAL) {
        dev_88e6390_phy_reg_wr(dev, port_num, PHY_PAGE(0), ESWPHY_CCR_ADDR,
                                  0x9140);
        return (PASSED);
    } else {
        printf("%s: Not support TestMode%d.\n", __FUNCTION__, test_mode);
        return (FAILED);
    }

    for (ctr = 0; ctr < total_steps; ctr++, step_ptr++) {
        /* Set register */
        printf("%s:%d: Set Port %d Set TestMode%d: Set page%d Reg%.2d to 0x%04X\n", 
               __FUNCTION__, __LINE__, port_num, test_mode, 
               step_ptr->reg_page, step_ptr->reg_off,step_ptr->val);
        printf("\n");
        if (dev_88e6390_phy_reg_wr(dev, port_num, step_ptr->reg_page,
                    step_ptr->reg_off, step_ptr->val) != PASSED) {
            printf("\n%s: Failed to set ESW(6390) page%d Reg%.2d to 0x%04X.\n",
                   __FUNCTION__, step_ptr->reg_page,
                   step_ptr->reg_off, step_ptr->val);
            return (FAILED);
        }

    }

    /* Set Test mode by write page0 Reg 9 */
    /* Set register */
    printf("%s:%d: Set TestMode%d: Set page%d Reg%.2d to 0x%04X\n", 
           __FUNCTION__, __LINE__, test_mode, PHY_PAGE(0), ESWPHY_1000TCR_ADDR, testmode_val);
    printf("\n");
    if (dev_88e6390_phy_reg_wr(dev, port_num,  PHY_PAGE(0), ESWPHY_1000TCR_ADDR, 
                               testmode_val) != PASSED) {
        printf("\n%s: Failed to set ESW(6390) page%d Reg%.2d to 0x%04X.\n",
                   __FUNCTION__, PHY_PAGE(0), ESWPHY_1000TCR_ADDR, 
                   testmode_val);
        return (FAILED);
    }
    printf("\nNow ESW(6390) enter TestMode%d, and press \'q\' to exit: ",
            test_mode);

    while (1) {
        if(getchar() == 'q') {
            dev_88e6390_phy_reg_wr(dev, port_num, PHY_PAGE(0),
                                   ESWPHY_CCR_ADDR, 0x9140);
            break;
        }
    }
    return (PASSED);

}

/**********************************************************************
 *
 * Function: dev_88e6390_chk_intr_assert
 *
 * This function: Checking the interrupt pin is asserted
 * Input : dev - dev_object_t pointer to the 88E6390 device
 * Output: PASSED/FAILED
 **********************************************************************
 */
static int dev_88e6390_chk_intr_assert (dev_object_t *dev)
{
    dev_88e6390_object_t *obj_88e6390 = (dev_88e6390_object_t *) dev;
    dev_88e6390_callout_fvt_t *callout_p = obj_88e6390->callout_fvt;
    int rc, ix;

    /* As Hardware Team's suggestion, while platform running in high/low temp.(EDVT), 
     * the electric reaction of Interrupt Pin might not instantaneous,
     * hence, checking the status of Interrupt Pin by polling. */
    for (ix = 0; ix < INTR_POLLING_ROUND; ix++) 
    {
        rc = (*callout_p->chk_intr_assert)();
        if (rc == PASSED) {
            break;
        }
        msleep(INTR_POLLING_PERIOD);
    }

    return (rc);
}

/**********************************************************************
 *
 * Function: dev_88e6390_chk_intr_deassert
 *
 * This function: Checking the interrupt pin is de-asserted
 * Input : dev - dev_object_t pointer to the 88E6390 device
 * Output: PASSED/FAILED
 **********************************************************************
 */
static int dev_88e6390_chk_intr_deassert (dev_object_t *dev)
{
    dev_88e6390_object_t *obj_88e6390 = (dev_88e6390_object_t *) dev;
    dev_88e6390_callout_fvt_t *callout_p = obj_88e6390->callout_fvt;
    int rc, ix;

    /* As Hardware Team's suggestion, while platform running in high/low temp.(EDVT), 
     * the electric reaction of Interrupt Pin might not instantaneous,
     * hence, checking the status of Interrupt Pin by polling. */
    for (ix = 0; ix < INTR_POLLING_ROUND; ix++) 
    {
        rc = (*callout_p->chk_intr_deassert)();
        if (rc == PASSED) {
            break;
        }
        msleep(INTR_POLLING_PERIOD);
    }

    return (rc);
}

/**********************************************************************
 * Function: dev_88e6390_is_smi_bus_free
 *
 * This function: Checking whether the SMI bus is available.
 * Input : dev - dev_object_t pointer to the 88E6390 device
 * Output: PASSED/FAILED
 * Note  : Checking SMI bus is free to access by checking 
 *         "SMI Command Register(0x0) bit[15]".
 *         This is used as device is in "multi chip address mode"
 **********************************************************************/
static int dev_88e6390_is_smi_bus_free (dev_object_t *dev)
{
    ushort reg_val = 0;
    int polling_result = FAILED, ix;

    for (ix = 0; ix < MAX_POLLING_ROUND; ix++)
    {
        /* read SMI bus with command register */
        if (dev_88e6390_smi_rd(dev, ESW_SMI_CMD_REG, &reg_val) != PASSED) {
            printf("%s:%d Failed to SMI read command reg.(%#X)\n",
                   __FUNCTION__, __LINE__, ESW_SMI_CMD_REG);
            return (FAILED);
        }
        
        /* compare data */
        if ((reg_val & (ushort)SMI_CMD_SMIBUSY) == 0) {
            polling_result = PASSED;
            break;
        }

        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("DBG[%s:%d] SMI bus is busy\n", __FUNCTION__, __LINE__);
        }

        /* put a delay for hardware preparation */
        msleep(SMI_BUS_PREPARE);
    }

    return (polling_result);
}

/**********************************************************************
 * Function: dev_88e6390_polling_phy_reg
 *
 * This function: Checking whether the read data is equal to pattern
 * Input : dev        - dev_object_t pointer to the 88E6390 device
 *         compare_op - COMPARE_AND/COMPARE_EQL
 *         smi_dev    - inner SMI device address 
 *         smi_reg    - inner SMI register address
 *         pattern    - a pattern for data comparing
 * Output: PASSED/FAILED
 **********************************************************************/
static int dev_88e6390_polling_phy_reg(dev_object_t *dev, int compare_op, int smi_dev, int smi_reg, ushort pattern)
{
    ushort reg_val = 0;
    int polling_result = FAILED, ix;

    for (ix = 0; ix < MAX_POLLING_ROUND; ix++)
    {
        /* read inner device with given register */
        if (dev_88e6390_reg_rd(dev, smi_dev, smi_reg, &reg_val) != PASSED) {
            printf("%s:%d Failed to SMI read ESW SMI dev %#x reg. %#x\n",
        	   __FUNCTION__, __LINE__, smi_dev, smi_reg);
            return (FAILED);
        }
     
        /* compare data */
        if (compare_op == COMPARE_AND) {
            if ((reg_val & pattern) == 0) {
                polling_result = PASSED;
                break;
            }
        } else if(compare_op == COMPARE_EQL) {
            if (reg_val == pattern) {
                polling_result = PASSED;
                break;
            }
        } else {
            if ((reg_val & pattern) == pattern) {
                polling_result = PASSED;
                break;
            }
        }

        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("DBG[%s:%d] The inner PHY SMI bus is busy\n", __FUNCTION__, __LINE__);
        }

        /* put a delay for hardware preparation */
        msleep(SMI_BUS_PREPARE);
    }

    return (polling_result);
}

/**********************************************************************
 * Function: dev_88e6390_polling_port_reg
 *
 * This function: Checking whether the read data is equal to pattern
 * Input : dev        - dev_object_t pointer to the 88E6390 device
 *         compare_op - COMPARE_AND/COMPARE_EQL
 *         phy_por    - PHY port number
 *         reg_pag    - page number of PHY port
 *         reg_add    - register address of PHY port
 *         pattern    - a pattern for data comparing 
 * Output: PASSED/FAILED
 **********************************************************************/
static int dev_88e6390_polling_port_reg(dev_object_t *dev, int compare_op, 
                                 int phy_port, int reg_page, int reg_addr, ushort pattern)
{
    ushort reg_val = 0;
    int polling_result = FAILED, ix;

    for (ix = 0; ix < MAX_POLLING_ROUND; ix++)
    {
        /* read inner PHY port with given page/register */
        if (dev_88e6390_phy_reg_rd(dev, phy_port, reg_page, reg_addr, &reg_val) != PASSED) {
            printf("DBG[%s:%d] Failed to read PHY port at"
                   "port:%d page:%d reg:0x%x\n",
        	   __FUNCTION__, __LINE__, phy_port, reg_page, reg_addr);
            return (FAILED);
        }
     
        /* compare data */
        if (compare_op == COMPARE_AND) {
            if ((reg_val & pattern) == 0) {
                polling_result = PASSED;
                break;
            }
        } else if(compare_op == COMPARE_EQL) {
            if (reg_val == pattern) {
                polling_result = PASSED;
                break;
            }
        } else {
            if ((reg_val & pattern) == pattern) {
                polling_result = PASSED;
                break;
            }
        }

        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("DBG[%s:%d] The inner PHY SMI bus is busy\n", __FUNCTION__, __LINE__);
        }

        /* put a delay for hardware preparation */
        msleep(SMI_BUS_PREPARE);
    }

    return (polling_result);
}

/**********************************************************************
 *
 * Function: dev_88e6390_set_vod_util
 *
 * This function: Checking the ste the output voltage
 * Input : dev - dev_object_t pointer to the 88E6390 device
 * Output: PASSED/FAILED
 **********************************************************************
 */
static int dev_88e6390_set_vod_util (dev_object_t *dev)
{
    int def_port = 0, s_port = 0, e_port = 0;
    int esw_port = 7, eth_mode = 2, vod_val = 0;
    int ctr = 0;

    def_port = ESW_PORT1;
    s_port = ESW_PORT1;
    e_port = (ESW_PORT8 + 1);

    printf("\n8 Port Switch port mapping - \n");
    printf("-------------------------\n");
    printf("|  2  |  4  |  6  |  8  |\n");
    printf("|-----+-----+-----+-----|\n");
    printf("|  1  |  3  |  5  |  7  |\n");
    printf("-------------------------\n");
    esw_port = (int)gethex_answer("Enter port num(1 ~ 8, 9 for all): ",
                                  def_port, s_port, e_port);
    if (esw_port != e_port) {
        s_port = esw_port;
        e_port = (s_port + 1);
    }
    printf("\nEthernet modes -\n");
    printf("    1 - 10 Mbps\n");
    printf("    2 - 100 Mbps\n");
    printf("    3 - 1000 Mbps\n");
    eth_mode = (int)gethex_answer("Enter mode: ", 0x2, 0x1, 0x3);

    printf("\nVOD modes -\n");
    printf(" 0:  0%%,   1:  -2%%,   2:  -4%%,   3:  -6%%\n");
    printf(" 4: -8%%,   5: -10%%,   6: -12%%,   7: -14%%\n");
    printf(" 8:  0%%,   9:   2%%,   a:   4%%,   b:   6%%\n");
    printf(" c:  8%%,   d:  10%%,   e:  12%%,   f:  14%%\n");
    vod_val = (int)gethex_answer("Enter VOD value: ", 0x0, 0x0, 0xf);

    for (ctr = s_port; ctr < e_port; ctr++) {
        if (dev_88e6390_set_vod(dev, esw_port, eth_mode, vod_val) != PASSED) {
            printf("Failed to adjust port%d VOD.\n", ctr);
            return (FAILED);
        }
        printf("Done adjust LAN Switch port%d VOD.\n", ctr);
    }
    return (PASSED);
}

/**********************************************************************
 *
 * Function: dev_88e6390_set_vod
 *
 * This function: Checking the ste the output voltage
 * Input : dev - dev_object_t pointer to the 88E6390 device
 * Output: PASSED/FAILED
 **********************************************************************
 */
static int dev_88e6390_set_vod(dev_object_t *dev, int esw_port, int eth_mode, int vod_val)
{
    int    reg_page = (int)REG_PAGE(252);
    int    reg_addr = (int)REG_ADDR(17);
    ushort reg_val = 0;

    if (eth_mode == 3) {
        reg_addr = (int)REG_ADDR(18);
    }

    /* Adjust VOD */
    if (dev_88e6390_phy_reg_rd(dev, esw_port, reg_page, reg_addr, &reg_val) != PASSED) {
        printf("%s: Failed to read Reg. %d_%d.\n",
               __FUNCTION__, reg_addr, reg_page);
        return (FAILED);
    }

    if (eth_mode == 3) {
        reg_val = (ushort)((vod_val << 12) |
                           (vod_val << 8) |
                           (vod_val << 4) |
                           (vod_val));
    } else if (eth_mode == 1) {
        reg_val &= (ushort)(~0x00ff);
        reg_val |= (ushort)((vod_val << 4) | vod_val);
    } else {
        reg_val &= (ushort)(~0xff00);
        reg_val |= (ushort)((vod_val << 12) | (vod_val << 8));
    }

    msleep(10);

    if (dev_88e6390_phy_reg_rd(dev, esw_port, reg_page, reg_addr, &reg_val) != PASSED) {
        printf("%s: Failed to set Reg. %d_%d to 0x%04X.\n",
               __FUNCTION__, reg_addr, reg_page, reg_val);
        return (FAILED);
    }

    msleep(10);

    /* Recover Page Addr to 0x0 to trigger adjust process */
    reg_page = (int)REG_PAGE(0);
    reg_addr = (int)REG_ADDR(22);
    reg_val = 0;
    
    if (dev_88e6390_phy_reg_wr(dev, esw_port, reg_page, reg_addr, reg_val) != PASSED) {
        printf("%s: Failed to trigger adjust process by set page addr to %d.\n",
	       __FUNCTION__, reg_val);
        return (FAILED);
    }
    return (PASSED);
}

/*-------------------------------------------------
 * $Log: dev_88e6390.c,v $
 * Revision 1.2  2019/01/10 06:19:23  wilbhuan
 * The beginning of Marvell 88E6390 Ethernet Switch PHY device driver.
 *
 *-------------------------------------------------
 */
