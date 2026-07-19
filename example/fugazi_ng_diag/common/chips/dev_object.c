/* $Id: dev_object.c,v 1.2 2012/03/28 00:38:06 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_object.c,v $
 *------------------------------------------------------------------
 *
 * FILE NAME: Common Device Driver
 *
 * Nov 2005 - Anh Dang 
 *
 * Copyright (c) 2007-2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include "endians.h"
#include "proto.h"
#include "dev_object.h"
#include "common.h"

/*****************************************************************
 * Name:
 *   dev_destroy_default 
 * Description:
 *    Common "do nothing" routines.
 * Returns:
 *    None
 * Notes:
 *    The routines do not access the device nor do they 
 *    change the state of the device object.
 *****************************************************************/
void
dev_destroy_default (dev_object_t **dev)
{
    assert(!"dev_destroy_default");
}

/*****************************************************************
 * Name:
 *   dev_do_nothing 
 * Description:
 *    Common "do nothing" routines.
 * Returns:
 *    FAILED
 * Notes:
 *    The routines do not access the device nor do they 
 *    change the state of the device object.
 *****************************************************************/
uint32
dev_do_nothing (dev_object_t *dev)
{
    assert(!"dev_do_nothing");
    return (FAILED);
}

/*****************************************************************
 * Name:
 *   dev_reconfig_needed_do_nothing 
 * Description:
 *    Common "do nothing" routines.
 * Returns:
 *    FAILED
 * Notes:
 *    The routines do not access the device nor do they 
 *    change the state of the device object.
 *****************************************************************/
uint32
dev_reconfig_needed_do_nothing (dev_object_t *dev,
                                void *dev_mode,
                                boolean *reconfig_needed)
{
    assert(!"dev_reconfig_needed_do_nothing");
    return (FAILED);
}

/*****************************************************************
 * Name:
 *   dev_do_nothing 
 * Description:
 *    Common "do nothing" routines.
 * Returns:
 *    FAILED
 * Notes:
 *    The routines do not access the device nor do they 
 *    change the state of the device object.
 *****************************************************************/
uint32
dev_show_do_nothing (dev_object_t *dev, print_fn_t print_fn, dev_show_cmd cmd)
{
    assert(!"dev_show_do_nothing");
    return (FAILED);
}

/*****************************************************************
 * Name:
 *   dev_error_report_do_nothing 
 * Description:
 *    Common "do nothing" routines.
 * Returns:
 *    FAILED
 * Notes:
 *    The routines do not access the device nor do they 
 *    change the state of the device object.
 *****************************************************************/
void
dev_error_report_do_nothing (dev_object_t *dev, char *err_str, uint32 err_id)
{
    assert(!"dev_error_report_do_nothing");
}

/*****************************************************************
 * Name:
 *   dev_collect_crashinfo_do_nothing 
 * Description:
 *    Common "do nothing" routines.
 * Returns:
 *    FAILED
 * Notes:
 *    The routines do not access the device nor do they 
 *    change the state of the device object.
 *****************************************************************/
uint32
dev_collect_crashinfo_do_nothing (dev_object_t *dev, 
                                  print_fn_t print_fn,
                                  dev_show_cmd verbosity)
{
    assert(!"dev_collect_crashinfo_do_nothing");
    return (FAILED);
}

/*****************************************************************
 * Name:
 *    init_default_dev_object
 * Description:
 *    Set fvt to the common "do nothing" function vector table.
 * Input: dev_object_t *dev -  pointer to the xx device 
 *        dev_object_fvt_t *fvt - pointer to object function vector table.
 * Returns:
 *    None.
 * Notes:
 *    It is up to the caller to override the do nothing
 *    functions on a case by case basis as appropriate.
 *****************************************************************/
void
init_default_dev_object (dev_object_t *dev, dev_object_fvt_t *fvt)
{
    dev_xx_object_t *dev_xx = (dev_xx_object_t *)dev;
    
    dev_xx->base.dev_object_fvt = fvt;
    dev_xx->base.dev_object_fvt->dev_detach = dev_do_nothing;
    dev_xx->base.dev_object_fvt->dev_reconfig_needed = 
	dev_reconfig_needed_do_nothing;
    dev_xx->base.dev_object_fvt->dev_restart = dev_do_nothing;
    dev_xx->base.dev_object_fvt->dev_init = dev_do_nothing;
    dev_xx->base.dev_object_fvt->dev_oper_enable = dev_do_nothing;
    dev_xx->base.dev_object_fvt->dev_oper_disable = dev_do_nothing;
    dev_xx->base.dev_object_fvt->dev_intr_enable = dev_do_nothing;
    dev_xx->base.dev_object_fvt->dev_intr_disable = dev_do_nothing;
    dev_xx->base.dev_object_fvt->dev_isr = dev_do_nothing;
    dev_xx->base.dev_object_fvt->dev_show = dev_show_do_nothing;
    dev_xx->base.dev_object_fvt->dev_error_report = 
	dev_error_report_do_nothing;
    dev_xx->base.dev_object_fvt->dev_collect_crashinfo = 
	dev_collect_crashinfo_do_nothing;
    dev_xx->base.dev_object_fvt->dev_destroy = dev_destroy_default;
    dev_xx->base.dev_object_fvt->dev_name = "dev_set_do_nothing";
}

/*********************************************************************
 *
 * Function: err_report()
 *
 * Description: This function reports error or warning 
 *              depends on the error ID flag
 *
 * Inputs:  dev      - Pointer to the PMC common device object
 *          err_msg  - Error message to be reported
 *          err_type - Type of error (Fatal, warning or just info)
 *
 * Outputs: always PASS but return type uint32 for compatibility
 *
 *********************************************************************
 */
uint32
err_report (dev_object_t *dev, char *err_msg, uint32 err_type)
{

    switch(err_type) {
	case WARNING:
	    cterr('w', 0, "%s", err_msg);
	    break;
	case RETRY:
	    printf("\nRetry: %s\n", err_msg);
	    break;
	case FATAL:
	    printf("\nFatal Error: %s\n", err_msg);
	    break;
	default:
	    cterr('f', 0, "%s", err_msg);
	    break;
    }
    return(PASSED);
}

/******** History ******** 
$Log: dev_object.c,v $
Revision 1.2  2012/03/28 00:38:06  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:01  ptong
Initial archive of ng_diag module


$Endlog$
*/
