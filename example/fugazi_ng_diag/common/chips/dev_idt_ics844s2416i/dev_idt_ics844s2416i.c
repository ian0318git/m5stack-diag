/* $Id: dev_idt_ics844s2416i.c,v 1.5 2013/11/26 08:40:32 hroni Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_idt_ics844s2416i/dev_idt_ics844s2416i.c,v $
 *------------------------------------------------------------------------------
 *
 * Filename:	dev_idt_ics844s2416i.c
 *
 * Description:	IDT ICS844S2416I FEMTOCLOCK Crystal-To-Differential
 *		HCSL/LVCMOS Frequency Synthesizer driver functions.
 *
 *		ICS844S2416I does not generate interrupt; therefore,
 *		dev_intr_enable, dev_intr_disable, dev_isr are not used.
 *		dev_attach, dev_detach, dev_reconfig_needed, dev_restart,
 *		dev_init, dev_oper_enable, dev_oper_disable, dev_show,
 *		dev_err_report, dev_collect_crashinfo, dev_destroy are
 *		implemented.
 *
 *		Bits definition of the Qn's of the chip are platform specific.
 *		The caller will pass the struct of the bits and registers
 *		test table.
 *
 * Copyright (c) 2007-2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------------------
 *
 */

#include <stdlib.h>
#include <assert.h>
#include "endians.h"
#include "types.h"
#include "defs.h"
#include "common.h"
#include "common_utils.h"
#include "dev_print.h"
#include "dev_object.h"
#include "dev_idt_ics844s2416i.h"
#include "free.h"
#include "i2c_api.h"
#include "proto.h"

/*===================================================================*
 *                    Function Prototypes                            *
 *===================================================================*/
static uint32   ics2416i_dev_attach(dev_object_t *);
static uint32	ics2416i_dev_detach(dev_object_t *);
static uint32	ics2416i_dev_reconfig(dev_object_t *, void *, boolean *);
static uint32	ics2416i_dev_restart(dev_object_t *);
static uint32	ics2416i_dev_init(dev_object_t *);
static uint32	ics2416i_dev_oper_en(dev_object_t *);
static uint32	ics2416i_dev_oper_dis(dev_object_t *);
static uint32	ics2416i_dev_show(dev_object_t *, print_fn_t, dev_show_cmd);
static uint32	ics2416i_crsh(dev_object_t *, print_fn_t, dev_show_cmd);
static void	ics2416i_destroy(dev_object_t **);
static int	ics2416_i2c_rd(ulong, int, ulong *, void *);
static int	ics2416_i2c_wr(ulong, int, ulong, void *);
static int	ics844s2416i_reg_test(dev_object_t *);
    
/* Register test table */
static reg_info_t_ext reg_ext = {
		sizeof(charint), ics2416_i2c_rd, ics2416_i2c_wr, 0};
		
static reg_info_t ics2416i_default_reg_tbl[] = {
    {"Data Bytes 0 - 3", 0, READ_WRITE | SAVE_RESTORE | REG_ACCESS,
	{(uint)REG_EXT}, 0xffffffc0, 0xf7ffa000},
    {0, 0, 0, {0}, 0, 0},
};

/* Default bits descriptor */
static ics2416_bit_t ics2416i_default_bit_d = {
    "Q16_EN",
    "Q15_EN",
    "Q14_EN",
    "Q13_EN",
    "Q12_EN",
    "Q11_EN",
    "Q10_EN",
    "Q9_EN",
    "Q8_EN",
    "Q7_EN",
    "Q6_EN",
    "Q5_EN",
    "Q4_EN",
    "Q3_EN",
    "Q2_EN",
    "Q1_EN",
    "Q0_EN",
};

/*****************************************************************
 *
 * Name: ics844s2416i_dev_create()
 *
 * Description: Create object with various device function
 * point to "do nothing"
 *
 * Input: dev_object_t pointer to the FEMTOCLOCK device.
 *	  error reporting function pointer.
 *
 * Returns: none
 *
 *****************************************************************/
void
ics844s2416i_dev_create (dev_object_t *dev, dev_error_report_t error_report_fn)
{
    dev_object_fvt_t	*dev_fvt;
    dev_844s2416i_object_t *ics2416 = (dev_844s2416i_object_t *)dev;

    /* Allocate memory for the device object */
    if ((dev_fvt = (dev_object_fvt_t *)malloc(sizeof(dev_object_fvt_t))) ==
		NULL) {
	/* Unable to allocate memory */
	error_report_fn(dev, "malloc failure in ics844s2416i_dev_create()",
			0);
	return;
    }

    /* Init the device object structure to default "do nothing" */
    ics2416->base.dev_state = DEV_STATE_CREATE;
    init_default_dev_object(dev, dev_fvt);

    ics2416->base.dev_object_fvt->dev_attach	   = ics2416i_dev_attach;
    ics2416->base.dev_object_fvt->dev_detach	   = ics2416i_dev_detach;
    ics2416->base.dev_object_fvt->dev_reconfig_needed = ics2416i_dev_reconfig;
    ics2416->base.dev_object_fvt->dev_restart	   = ics2416i_dev_restart;
    ics2416->base.dev_object_fvt->dev_init	   = ics2416i_dev_init;
    ics2416->base.dev_object_fvt->dev_oper_enable  = ics2416i_dev_oper_en;
    ics2416->base.dev_object_fvt->dev_oper_disable = ics2416i_dev_oper_dis;
    ics2416->base.dev_object_fvt->dev_error_report = error_report_fn;
    ics2416->base.dev_object_fvt->dev_show	   = ics2416i_dev_show;
    ics2416->base.dev_object_fvt->dev_collect_crashinfo = ics2416i_crsh;
    ics2416->base.dev_object_fvt->dev_destroy	   = ics2416i_destroy;
    ics2416->base.dev_object_fvt->dev_name = "ICS844S2416I - FEMTOCLOCK";

    ics2416->callin_fvt = (ics2416i_callin_fvt_t *)
				malloc( sizeof(ics2416i_callin_fvt_t));
    ics2416->callout_fvt = (ics2416i_callout_fvt_t *)
				malloc( sizeof(ics2416i_callout_fvt_t));
}
/*****************************************************************
 *
 * Name: ics2416i_dev_attach()
 *
 * Description: Attach the ICS844S2416I device for use. This
 *   function will initialize and setup all necessary pointers
 *   and bring the chip to operation.
 *
 * Input: Pointer to the FEMTOCLOCK device object
 *
 * Returns: PASSED/FAILED
 *
 *****************************************************************/
static uint32
ics2416i_dev_attach (dev_object_t *dev)
{
    uint32 rc;
    dev_844s2416i_object_t *ics2416 = (dev_844s2416i_object_t *) dev;
    char err_buf[ERR_BUF_SIZE];

    if (ics2416->callin_fvt == NULL) {
	DEV_ERROR_REPORT(dev, "ics844s2416i_dev_attach() callin malloc", 
			 ICS2416_ATTACH);
	return(FAILED);
    }

    if (ics2416->callout_fvt == NULL) {
	DEV_ERROR_REPORT(dev, "ics844s2416i_dev_attach() callout malloc", 
			 ICS2416_ATTACH);
	return(FAILED);
    }

    /* init the call in function */
    ics2416->callin_fvt->register_test = ics844s2416i_reg_test;

    /* Lock the I2C device */
    if ((rc = ics2416->callout_fvt->open(ics2416->i2c_p)) != PASSED) {
	sprintf(err_buf, "ics844s2416i_dev_attach() I2C open failed with rc = "
			 "%#x", rc);
        DEV_ERROR_REPORT(dev, err_buf, ICS2416_ATTACH);
        return(FAILED);
    }

    ics2416->base.dev_state = DEV_STATE_ATTACH;

    return (PASSED);
}

/*****************************************************************
 *
 * Name: ics2416i_dev_detach()
 *
 * Description: detach the device specific functions from the caller.
 *		All of the device specific function are connected to the
 *		dev_do_nothing() function, except for the dev_attach()
 *		function. Also, the dev_state must be assigned the value
 *		of DEV_STATE_DETACH.
 *
 *		Since, some platforms may want to detach the device, but not
 *		release the memory resources (via a free () in the
 *		dev_destroy()), this function can be executed to accomplish
 *		this task. However, before a detached device can be used again,
 *		it must be re-attached (via the dev_attach()).
 *
 * Input: Pointer to the FEMTOCLOCK device object
 *
 * Returns: PASSED/FAILED
 *
 *****************************************************************/
static uint32
ics2416i_dev_detach (dev_object_t *dev)
{
    uint32 rc;
    dev_844s2416i_object_t *ics2416 = (dev_844s2416i_object_t *) dev;
    char err_buf[ERR_BUF_SIZE];

    /* Unlock the I2C device */
    if ((rc = ics2416->callout_fvt->close(ics2416->i2c_p)) != PASSED) {
	sprintf(err_buf, "ics844s2416i_dev_detach() I2C close failed. rc = %#x",
							rc);
	DEV_ERROR_REPORT(dev, err_buf, ICS2416_DETACH);
	return(FAILED);
    }

    /* Init the device object structure to default "do nothing" */
    init_default_dev_object(dev, ics2416->base.dev_object_fvt);

    ics2416->base.dev_state = DEV_STATE_DETACH;

    return (PASSED);

}

/*****************************************************************
 * Name: ics2416i_dev_reconfig_needed
 *
 * Description: To check whether device re-configuration is needed during
 *		(re)initialization. Based on the provided context information,
 *		the boolean return value, and possibly other factors external
 *		to the device object, the caller shall decide whether to invoke
 *		either dev_restart or dev_init, but not both. In general, the
 *		boolean return value alone is not sufficient to decide whether
 *		the device can safely be restarted or whether it must be fully
 *		initialized from scratch.
 *
 * Input: dev_object_t pointer to the FEMTOCLOCK device
 *	  void * - a device/platform specific context handle
 *	  boolean * - a pointer to a boolean
 *
 * Returns: PASSED/FAILED, context information and a boolean value.
 *	    The boolean value shall be set to TRUE if the device must be
 *	    reconfigured from scratch and it shall be set to FALSE otherwise.
 *
 * Assumptions: The dev_attach() function has been called and successfully
 *****************************************************************/
static uint32
ics2416i_dev_reconfig(dev_object_t *dev, void *context_handle,
					 boolean *reconfig)
{
    *reconfig = FALSE;		/* No need to reconfig from scratch */
    return(PASSED);
}

/*****************************************************************
 * Name: ics2416i_dev_restart
 *
 * Description:	To restart a previously initialized device without resetting
 *		the device or changing its configuration.
 *		For example, during a failover event.
 *
 *		Change the state of the device from its current state
 *		to an initial state. Also, dev_state must be assigned the
 *		value of DEV_STATE_INIT.
 *   
 * Input: dev_object_t pointer to the FEMTOCLOCk DEVICE
 *
 * Returns: PASSED/FAILED
 *
 * Assumptions: The dev_attach() and dev_reconfig_needed() functions has been
 *              called and successfully executed.
 *****************************************************************/
static uint32
ics2416i_dev_restart(dev_object_t *dev)
{
    dev_844s2416i_object_t *ics2416 = (dev_844s2416i_object_t *) dev;

    ics2416->base.dev_state = DEV_STATE_INIT;
    return(PASSED);
}

/*****************************************************************
 *
 * Name: ics2416i_dev_init()
 *
 * Description: Initializes the FEMTOCLOCK chip 
 *              
 *
 * Input: dev_object_t pointer to the FEMTOCLOCK device.
 *	  Caller has to setup the i2c_p parameters with init values.
 *
 * Returns: PASSED/FAILED
 *
 * Note: Make sure base.dev_addr has been initialized to chip_base_addr
 *       before calling this function.
 *
 *****************************************************************/
static uint32
ics2416i_dev_init (dev_object_t *dev)
{
    uint32 rc;
    dev_844s2416i_object_t *ics2416 = (dev_844s2416i_object_t *)dev;
    ics2416i_callout_fvt_t *callout_p = (ics2416i_callout_fvt_t *)ics2416->callout_fvt;
    char err_buf[ERR_BUF_SIZE];

    ics2416->base.dev_state = DEV_STATE_INIT;

    /* Write to the FEMTOCLOCK with platform default value */
    rc = callout_p->wr(ics2416->i2c_p);

    if (rc != PASSED) {
	sprintf(err_buf, "ics844s2416i_dev_init() write failed. rc = %#x", rc);
	DEV_ERROR_REPORT(dev, err_buf, ICS2416_INIT);
	return(FAILED);
    }

    return(PASSED);

}

/*****************************************************************
 * Name: ics2416i_dev_oper_en
 *
 * Description:	Enable device operation.
 *
 *		Change the state of the device from its current state to an
 *		enabled state (which implies that the device is in an
 *		operational state at the end of this function execution). Also,
 *		the dev_state must be assigned the value of DEV_STATE_ENABLE_OP
 *
 *		For devices such as port asic's and framers, this function
 *		be used to enable all or only part of the total device port's
 *		or channel's.
 *
 *		For FEMTOCLOCK, one clock or multiple of clocks can be enabled
 *		at the same time, and it is passed in the parameter struct.
 *
 * Input: dev_object_t pointer to the FEMTOCLOCK device
 *
 * Returns: PASSED/FAILED
 *
 * Assumptions: The dev_init() function has been called and successfully
 *
 *****************************************************************/
static uint32
ics2416i_dev_oper_en(dev_object_t *dev)
{
    uint32 rc;
    ics844s2416_reg_t reg;
    dev_844s2416i_object_t *ics2416 = (dev_844s2416i_object_t *)dev;
    ics2416i_callout_fvt_t *callout_p = ics2416->callout_fvt;
    char err_buf[ERR_BUF_SIZE];

    /* Read the data bytes of FEMTOCLOCK */
    rc = (*callout_p->rd)(ics2416->i2c_p);

    if (rc != PASSED) {
	sprintf(err_buf, "ics2416i_dev_oper_en() read failed. rc = %#x", rc);
	DEV_ERROR_REPORT(dev, err_buf, ICS2416_OPER_EN);
	return(FAILED);
    }

    reg = *(ics844s2416_reg_t *)ics2416->i2c_p->buf;

    /* Enable the device */
    reg.data.d.lword |= ics2416->enable.data.d.lword;

    /* Write the enable */
    rc = callout_p->wr(ics2416->i2c_p);

    if (rc != PASSED) {
	sprintf(err_buf, "ics2416i_dev_oper_en() write failed. rc = %#x", rc);
	DEV_ERROR_REPORT(dev, err_buf, ICS2416_OPER_EN);
	return(FAILED);
    }

    ics2416->base.dev_state = DEV_STATE_ENABLE_OP;

    return (PASSED);

}

/*****************************************************************
 * Name: ics2416i_dev_oper_dis
 *
 * Description:	Disable device operation.
 *
 *		Change the state of the device from its current state to an
 *		disabled state (which implies that the device is in a
 *		non-operational state at the end of this function execution).
 *		Also, the dev_state must be assigned the value of
 *		DEV_STATE_DISABLE_OP
 *
 *		For devices such as port asic's and framers, this function
 *		be used to disable all the ports or channels of a specific
 *		device
 * 
 *		For FEMTOCLOCK, one clock or multiple of clocks can be disabled
 *		at the same time, and it is passed in the parameter struct.
 *
 * Input: dev_object_t pointer to the FEMTOCLOCK device
 *
 * Returns: PASSED/FAILED
 *
 * Assumptions: The dev_init() function has been called and successfully
 *
 *****************************************************************/
static uint32
ics2416i_dev_oper_dis(dev_object_t *dev)
{
    uint32 rc;
    ics844s2416_reg_t reg;
    dev_844s2416i_object_t *ics2416 = (dev_844s2416i_object_t *)dev;
    ics2416i_callout_fvt_t *callout_p = ics2416->callout_fvt;
    char err_buf[ERR_BUF_SIZE];

    /* Read the data bytes of FEMTOCLOCK */
    rc = (*callout_p->rd)(ics2416->i2c_p);

    if (rc != PASSED) {
	sprintf(err_buf, "ics2416i_dev_oper_dis() read failed. rc = #%#x", rc);
	DEV_ERROR_REPORT(dev, err_buf, ICS2416_OPER_DIS);
	return(FAILED);
    }

    reg = *(ics844s2416_reg_t *)ics2416->i2c_p->buf;

    /* Disable the device */
    reg.data.d.lword &= ~(ics2416->enable.data.d.lword);

    /* Write the enable */
    rc = (*callout_p->wr)(ics2416->i2c_p);

    if (rc != PASSED) {
	sprintf(err_buf, "ics2416i_dev_oper_dis() write failed. rc = %#x", rc);
	DEV_ERROR_REPORT(dev, err_buf, ICS2416_OPER_DIS);
	return(FAILED);
    }

    ics2416->base.dev_state = DEV_STATE_DISABLE_OP;

    return (PASSED);

}

/*****************************************************************
 * Name: ics2416i_dev_show
 *
 * Description:	Provide platforms with a mechanism to display some common
 *		device information via the device print function argument.
 *
 * Input: dev_object_t pointer to the FEMTOCLOCK device
 *	  A device print function vector
 *	  A dev_show_cmd_e command
 *
 * Returns: PASSED/FAILED
 *
 * Assumptions: The device printf function vector has been provided by the host
 *              platform which implements the print logging functionality. The
 *              dev_attach() function has been called and successfully executed
 *
 *****************************************************************/
static uint32
ics2416i_dev_show(dev_object_t *dev, print_fn_t dev_print, dev_show_cmd cmd)
{
    uint32 rc;
    ics844s2416_reg_t *reg;
    dev_844s2416i_object_t *ics2416 = (dev_844s2416i_object_t *)dev;
    ics2416i_callout_fvt_t *callout_p = ics2416->callout_fvt;
    char err_buf[ERR_BUF_SIZE];

    /* Read the data bytes of FEMTOCLOCK */
    rc = (*callout_p->rd)(ics2416->i2c_p);

    if (rc != PASSED) {
	sprintf(err_buf, "ics2416i_dev_show() read failed. rc = %#x", rc);
        DEV_ERROR_REPORT(dev, err_buf, ICS2416_SHOW);
        return(FAILED);
    }

    reg = (ics844s2416_reg_t *)ics2416->i2c_p->buf;	/* Get the registers */

    switch (cmd) {
    case DEV_SHOW_ALL:
    case DEV_SHOW_CONFIG:
    case DEV_SHOW_REGISTERS:
	if (ics2416->bit_p == NULL) {
	    /* Use the default table */
	    ics2416->bit_p = &ics2416i_default_bit_d;
	}

	if (ics2416->bit_p->q16) {
	    dev_print("\n%s %s\n", ics2416->bit_p->q16,
			       (reg->data.d.byte[0] & ICS844S2416_DB0_Q16_EN) ?
				"Enabled" : "Disabled");
	}

	if (ics2416->bit_p->q15) {
	    dev_print("%s %s\n", ics2416->bit_p->q15,
			       (reg->data.d.byte[0] & ICS844S2416_DB0_Q15_EN) ?
				"Enabled" : "Disabled");
	}

	if (ics2416->bit_p->q14) {
	    dev_print("%s %s\n", ics2416->bit_p->q14,
			       (reg->data.d.byte[0] & ICS844S2416_DB0_Q14_EN) ?
				"Enabled" : "Disabled");
	}

	if (ics2416->bit_p->q13) {
	    dev_print("%s %s\n", ics2416->bit_p->q13,
			       (reg->data.d.byte[0] & ICS844S2416_DB0_Q13_EN) ?
				"Enabled" : "Disabled");
	}

	if (ics2416->bit_p->q12) {
	    dev_print("%s %s", ics2416->bit_p->q12,
			       (reg->data.d.byte[0] & ICS844S2416_DB0_Q12_EN) ?
				"Enabled" : "Disabled");
	    dev_print(" with %s\n", (reg->data.d.byte[3] &
				   ICS844S2416_DB3_Q12_100) ?
				   "100MHz" : "200MHz");
	}

	if (ics2416->bit_p->q11) {
	    dev_print("%s %s\n", ics2416->bit_p->q11,
			       (reg->data.d.byte[0] & ICS844S2416_DB0_Q11_EN) ?
				"Enabled" : "Disabled");
	}

	if (ics2416->bit_p->q10) {
	    dev_print("%s %s\n", ics2416->bit_p->q10,
			       (reg->data.d.byte[0] & ICS844S2416_DB0_Q10_EN) ?
				"Enabled" : "Disabled");
	}

	if (ics2416->bit_p->q9) {
	    dev_print("%s %s\n", ics2416->bit_p->q9,
			       (reg->data.d.byte[0] & ICS844S2416_DB0_Q9_EN) ?
				"Enabled" : "Disabled");
	}

	if (ics2416->bit_p->q8) {
	    dev_print("%s %s\n", ics2416->bit_p->q8,
			       (reg->data.d.byte[1] & ICS844S2416_DB1_Q8_EN) ?
				"Enabled" : "Disabled");
	}

	if (ics2416->bit_p->q7) {
	    dev_print("%s %s\n", ics2416->bit_p->q7,
			       (reg->data.d.byte[1] & ICS844S2416_DB1_Q7_EN) ?
				"Enabled" : "Disabled");
	}

	if (ics2416->bit_p->q6) {
	    dev_print("%s %s\n", ics2416->bit_p->q6,
			       (reg->data.d.byte[1] & ICS844S2416_DB1_Q6_EN) ?
				"Enabled" : "Disabled");
	}

	if (ics2416->bit_p->q5) {
	    dev_print("%s %s\n", ics2416->bit_p->q5,
			       (reg->data.d.byte[1] & ICS844S2416_DB1_Q5_EN) ?
				"Enabled" : "Disabled");
	}

	if (ics2416->bit_p->q4) {
	    dev_print("%s %s\n", ics2416->bit_p->q4,
			       (reg->data.d.byte[1] & ICS844S2416_DB1_Q4_EN) ?
				"Enabled" : "Disabled");
	}

	if (ics2416->bit_p->q3) {
	    dev_print("%s %s\n", ics2416->bit_p->q3,
			       (reg->data.d.byte[1] & ICS844S2416_DB1_Q3_EN) ?
				"Enabled" : "Disabled");
	}

	if (ics2416->bit_p->q2) {
	    dev_print("%s %s\n", ics2416->bit_p->q2,
			       (reg->data.d.byte[1] & ICS844S2416_DB1_Q2_EN) ?
				"Enabled" : "Disabled");
	}

	if (ics2416->bit_p->q1) {
	    dev_print("%s %s\n", ics2416->bit_p->q1,
			       (reg->data.d.byte[1] & ICS844S2416_DB1_Q1_EN) ?
				"Enabled" : "Disabled");
	}

	if (ics2416->bit_p->q0) {
	    dev_print("%s %s\n", ics2416->bit_p->q0,
			       (reg->data.d.byte[2] & ICS844S2416_DB2_Q0_EN) ?
				"Enabled" : "Disabled");
	}

	dev_print("PLL2 Feedback Divider - %#.2x\n", (reg->data.d.byte[2] &
			ICS844S2416_DB2_M_MASK) >> DB2_M_SHIFT);
	dev_print("PLL2 SSC Mode - %s\n", (reg->data.d.byte[2] &
			ICS844S2416_DB2_SSC) ? "Down-Spread" : "Off");
	dev_print("Bypass Clock - %s\n", (reg->data.d.byte[2] &
			ICS844S2416_DB2_BYPASS) ? "Reference Clock" : "PLL");

	break;
    case DEV_SHOW_BRIEF:
	dev_print("Data Bytes 0 - 3 = 0x%.8X\n", reg->data.d.lword);
	break;
    default:
	assert(!"ics2416i_dev_show");
	break;
    }

    return(PASSED);
}

/*****************************************************************
 * Name: ics2416i_crsh
 *
 * Description:	Allow platforms to collect data from a device during a crash.
 *		Print data to the crash log (via the provide print error) using
 *		the appropriate verbisity level requested by the host
 *
 * Input: dev_object_t pointer to the FEMTOCLOCK device
 *        A crash print function vector.
 *        A verbosity level.
 *
 * Returns: PASSED/FAILED
 *
 * Assumptions: A device print function vector has been provided by the host
 *		platform which implements the crash logging functionality. It
 *		could be the mechanism to log info to the Compact Flash before
 *		the device crash and now retrieve them. The dev_attch()
 *		function has been called and successfully executed.
 *
 *****************************************************************/
static uint32
ics2416i_crsh(dev_object_t *dev, print_fn_t dev_print, dev_show_cmd verbosity)
{

    /* more development in this section */
    dev_print("ics2416i_crsh(): No Crash info available for ICS844S2416I\n");
    return(PASSED);
}

/*****************************************************************
 * Name: ics2416i_destroy
 *
 * Description:	Destroy the dev_object structure and free all the resources.
 *
 * Input: dev_object_t pointer to the FEMTOCLOCK device
 *
 * Returns: none
 *
 * Assumptions: The dev_attch() function has been called and successfully
 *
 *****************************************************************/
static void
ics2416i_destroy(dev_object_t **dev)
{
    uint32_t rc;
    dev_844s2416i_object_t *ics2416;
    char err_buf[ERR_BUF_SIZE];

    if (dev == NULL) {
	return;
    }

    if (*dev == NULL) {
	return;
    }

    ics2416 = (dev_844s2416i_object_t *)*dev;

    /* Unlock the I2C device */
    if ((rc = ics2416->callout_fvt->close(ics2416->i2c_p)) != PASSED) {
	sprintf(err_buf, "ics844s2416i_destroy() I2C close return code %#x",
						rc);
	DEV_ERROR_REPORT(*dev, err_buf, ICS2416_DESTROY);
	return;
    }

    if (ics2416->callout_fvt) {
	free(ics2416->callout_fvt);	/* Free callout struct */
    }

    if (ics2416->callin_fvt) {
	free(ics2416->callin_fvt);	/* Free callin struct */
    }

    free(ics2416->base.dev_object_fvt);	/* Free dev_object_t */
}

/**********************************************************************
 *
 * Function: ics844s2416i_reg_test
 *
 * This function: tests ICS844S2416I FEMTOCLOCK registers
 *
 * Input : dev - Pointer to the FEMTOCLOCK device object
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
ics844s2416i_reg_test(dev_object_t *dev)
{
    reg_info_t *reg_ptr;
    dev_844s2416i_object_t *ics2416 = (dev_844s2416i_object_t *)dev;

    if (ics2416->reg_table_p) {
	/* User provided register table */
	reg_ptr = ics2416->reg_table_p;
    } else {
	/* Use default register table */
	reg_ptr = &ics2416i_default_reg_tbl[0];
    }

    /* Setup ext struct for register_tests() */
    reg_ptr->size.ext = &reg_ext;
    reg_ptr->size.ext->param = (void *)dev;

    /* registers_test() will call ics2416_i2c_rd/wr() through reg_info_t_ext
     * struct's rd_ptr and wr_ptr.
     */
    return (register_tests(0, reg_ptr));

}

/**********************************************************************
 *
 * Function: ics2416i_i2c_rd
 *
 * This function: reads ICS844S2416I FEMTOCLOCK registers
 *
 * Input : addr - offset of register to be read.
 *	   size - number of bytes to be read.
 *	   buf  - points to the data buffer to be read.
 *	   param - Pointer to the FEMTOCLOCK device object
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
ics2416_i2c_rd(ulong addr, int size, ulong *buf, void *param)
{
    uint32 rc;
    n2g_i2c_if_t i2c_if;
    dev_844s2416i_object_t *ics2416 = (dev_844s2416i_object_t *)param;
    ics2416i_callout_fvt_t *callout_p = ics2416->callout_fvt;
    char err_buf[ERR_BUF_SIZE];

#ifdef DEBUG_I2C
    printf("ics2416_i2c_rd() size = %#x\n", size);
#endif /* DEBUG_I2C */

    /* Setup the interface struct for I2C API read */
    i2c_if.offset = addr;
    i2c_if.buf = (char *)buf;
    i2c_if.size = size;
    i2c_if.i2c_bus_type = ics2416->i2c_p->i2c_bus_type;
    i2c_if.i2c_dev = ics2416->i2c_p->i2c_dev;

#ifdef DEBUG_REG_TEST
    printf("reading bus %#x, dev %#x, @ %#x for %d bytes\n",
	i2c_if.i2c_bus_type,  i2c_if.i2c_dev, addr, size);
#endif /* DEBUG_REG_TEST */

    /* Call the I2C Read API */
    rc = callout_p->rd(&i2c_if);
    if (rc != PASSED) {
	/* Read failed */
	sprintf(err_buf, "ics2416_i2c_rd() read failed. rc = %#x", rc);
	DEV_ERROR_REPORT((dev_object_t *)param, err_buf, ICS2416_I2C_READ);
	return(FAILED);
    } else {
	/* Got the data */
	return(PASSED);
    }
}

/**********************************************************************
 *
 * Function: ics2416_i2c_wr
 *
 * This function: writes ICS844S2416I FEMTOCLOCK registers
 *
 * Input : addr - offset of register to be written.
 *         size - number of bytes to write.
 *         data  - write data.
 *         param - Pointer to the FEMTOCLOCK device object
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
ics2416_i2c_wr(ulong addr, int size, ulong data, void *param)
{
    uint32 rc;
    n2g_i2c_if_t i2c_if;
    dev_844s2416i_object_t *ics2416 = (dev_844s2416i_object_t *)param;
    ics2416i_callout_fvt_t *callout_p = ics2416->callout_fvt;
    char err_buf[ERR_BUF_SIZE];

#ifdef DEBUG_I2C
    printf("ics2416_i2c_wr() size = %#x, data = %#x\n", size, data);
#endif /* DEBUG_I2C */

    /* Setup the interface struct for I2C API write */
    i2c_if.offset = addr;
    i2c_if.buf = (char *)&data;
    i2c_if.size = size;
    i2c_if.i2c_bus_type = ics2416->i2c_p->i2c_bus_type;
    i2c_if.i2c_dev = ics2416->i2c_p->i2c_dev;

#ifdef DEBUG_REG_TEST
    printf("writing %#x to bus %#x, dev %#x, @ %#x for %d bytes\n",
	data, i2c_if.i2c_bus_type, i2c_if.i2c_dev, addr, size);
#endif /* DEBUG_REG_TEST */

    /* Call the I2C Write API */
    rc = callout_p->wr(&i2c_if);
    if (rc != PASSED) {
	/* Read failed */
	sprintf(err_buf, "ics2416_i2c_wr() write %#x failed. rc = %#x",
						 (unsigned int)data, rc);
	DEV_ERROR_REPORT((dev_object_t *)param, err_buf, ICS2416_I2C_WRITE);
	return(FAILED);
    } else {
	/* Data written */
	return(PASSED);
    }
}


/******** History ******** 
$Log: dev_idt_ics844s2416i.c,v $
Revision 1.5  2013/11/26 08:40:32  hroni
fix compiler warning

Revision 1.4  2012/06/04 10:06:04  aarwang
- Clean up compiler warnings.

Revision 1.3  2012/03/28 00:38:07  mcharon
remove forward slash from second line

Revision 1.2  2012/03/25 03:58:04  palin2
Clean up Overlord Intel side (x86) compile warnings.

Revision 1.1.1.1  2012/03/23 23:02:01  ptong
Initial archive of ng_diag module


$Endlog$
*/
