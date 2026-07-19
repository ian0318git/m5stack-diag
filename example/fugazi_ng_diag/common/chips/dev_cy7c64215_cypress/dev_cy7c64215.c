/* $Id: dev_cy7c64215.c,v 1.5 2013/11/26 08:40:31 hroni Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_cy7c64215_cypress/dev_cy7c64215.c,v $
 *------------------------------------------------------------------------------
 *
 * Filename:	dev_cy7c64215.c
 *
 * Description:	Cypress CY7C64215 enCoRe II Full Speed USB Controller.
 *		This file provides the I2C Common device driver functions used
 *		for this device.
 *
 *		This device does not generate interrupt; therefore,
 *		dev_intr_enable,  dev_intr_disable, and dev_isr are
 *		not implemented.
 *
 *		There is no register to dump for this device; therefore,
 *		dev_show is not implemented.
 *
 *		dev_init is used to download firmware to the device.
 *
 *		dev_attach, dev_detach, dev_reconfig_needed, dev_restart,
 *		dev_err_report, dev_collect_crashinfo, dev_destroy
 *		are also implemented.
 *
 *		Refer to Vendor datasheet and EDCS-640159 for more info.
 *
 * Copyright (c) 2008-2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------------------
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "endians.h"
#include "defs.h"
#include "common.h"
#include "dev_cy7c64215.h"
#include "dev_print.h"
#include "free.h"
#include "proto.h"
#include "goofy_i2c.h"
#include "byteswap.h"


/* #define USB_CONSOLE_DEGUG  * */

/*===================================================================*
 *                    Function Prototypes                            *
 *===================================================================*/
static uint32   dev_usb_attach(dev_object_t *);
static uint32	dev_usb_detach(dev_object_t *);
static uint32	dev_usb_reconfig(dev_object_t *, void *, boolean *);
static uint32	dev_usb_restart(dev_object_t *);
static uint32	dev_usb_init(dev_object_t *);
static uint32	dev_usb_crsh(dev_object_t *, print_fn_t, dev_show_cmd);
static void	dev_usb_destroy(dev_object_t **);

/*****************************************************************
 *
 * Name: dev_cy7c64215_create()
 *
 * Description: Create object with various device function
 * point to "do nothing"
 *
 * Input: dev_object_t pointer to the CY7C64215 device.
 *	  error reporting function pointer.
 *
 * Returns: none
 *
 *****************************************************************/
void
dev_cy7c64215_create (dev_object_t *dev, dev_error_report_t error_report_fn)
{
    dev_object_fvt_t	*dev_fvt;
    dev_cy7c64215_object_t *pusb = (dev_cy7c64215_object_t *)dev;

    /* Allocate memory for the device object */
    if ((dev_fvt = (dev_object_fvt_t *)malloc(sizeof(dev_object_fvt_t))) ==
		NULL) {
	/* Unable to allocate memory */
	error_report_fn(dev, "malloc failure in dev_cy7c64215_create()", 0);
	return;
    }

    /* Init the device object structure to default "do nothing" */
    init_default_dev_object(dev, dev_fvt);

    pusb->base.dev_object_fvt->dev_attach	= dev_usb_attach;
    pusb->base.dev_object_fvt->dev_detach	= dev_usb_detach;
    pusb->base.dev_object_fvt->dev_reconfig_needed = dev_usb_reconfig;
    pusb->base.dev_object_fvt->dev_restart	= dev_usb_restart;
    pusb->base.dev_object_fvt->dev_init		= dev_usb_init;
    pusb->base.dev_object_fvt->dev_error_report	= error_report_fn;
    pusb->base.dev_object_fvt->dev_collect_crashinfo = dev_usb_crsh;
    pusb->base.dev_object_fvt->dev_destroy	= dev_usb_destroy;
    pusb->base.dev_object_fvt->dev_name	= "USB Controller (CY7C64215)";

    /* Current callin_fvt is empty. bypass the malloc. If not empty,
     * un-comment the following -
     */
    /* pusb->callin_fvt = (cy7c64215_callin_fvt_t *)
				malloc(sizeof(cy7c64215_callin_fvt_t));
     */

    pusb->callout_fvt = (cy7c64215_callout_fvt_t *)
				malloc(sizeof(cy7c64215_callout_fvt_t));

    pusb->base.dev_state = DEV_STATE_CREATE;

}
/*****************************************************************
 *
 * Name: dev_usb_attach()
 *
 * Description: Attach the CY7C64215 device for use. This
 *   function will initialize and setup all necessary pointers
 *   and bring the chip to operation.
 *
 * Input: Pointer to the CY7C64215 device object
 *
 * Returns: PASSED/FAILED
 *
 *****************************************************************/
static uint32
dev_usb_attach (dev_object_t *dev)
{
    uint32 rc;
    dev_cy7c64215_object_t *pusb = (dev_cy7c64215_object_t *) dev;
    char err_buf[ERR_BUF_SIZE];

    /* Current callin_fvt is empty. bypass the malloc. If not empty,
     * un-comment the following -
     */
    /*   if (pusb->callin_fvt == NULL) {
	DEV_ERROR_REPORT(dev, "dev_usb_attach() callin malloc", 
			 CY7C64215_ATTACH);
	return(FAILED);
    }
     */

    if (pusb->callout_fvt == NULL) {
	DEV_ERROR_REPORT(dev, "dev_usb_attach() callout malloc", 
			 CY7C64215_ATTACH);
	return(FAILED);
    }

    /* init the call in function */
    /*
    pusb->callin_fvt->peek_n_poke = usb_alter_eeprom;
     */

    /* Lock the I2C device */
    if ((rc = pusb->callout_fvt->open(pusb->i2c_p)) != PASSED) {
	sprintf(err_buf, "dev_usb_attach() I2C open return rc = %#x", rc);
        DEV_ERROR_REPORT(dev, err_buf, CY7C64215_ATTACH);
        return(FAILED);
    }

    pusb->base.dev_state = DEV_STATE_ATTACH;

    return (PASSED);
}

/*****************************************************************
 *
 * Name: dev_usb_detach()
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
 * Input: Pointer to the CY7C64215 device object
 *
 * Returns: PASSED/FAILED
 *
 *****************************************************************/
static uint32
dev_usb_detach (dev_object_t *dev)
{
    uint32 rc;
    dev_cy7c64215_object_t *pusb = (dev_cy7c64215_object_t *) dev;
    char err_buf[ERR_BUF_SIZE];

    /* Unlock the I2C device */
    if ((rc = pusb->callout_fvt->close(pusb->i2c_p)) != PASSED) {
	sprintf(err_buf, "dev_usb_detach() I2C close rc = %#x", rc);
	DEV_ERROR_REPORT(dev, err_buf, CY7C64215_DETACH);
	return(FAILED);
    }

    /* Init the device object structure to default "do nothing" */
    init_default_dev_object(dev, pusb->base.dev_object_fvt);

    pusb->base.dev_state = DEV_STATE_DETACH;

    return (PASSED);

}

/*****************************************************************
 * Name: dev_usb_reconfig_needed
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
 * Input: dev_object_t pointer to the CY7C64215 device
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
dev_usb_reconfig(dev_object_t *dev, void *context_handle,
					 boolean *reconfig)
{
    *reconfig = FALSE;		/* No need to reconfig from scratch */
    return(PASSED);
}

/*****************************************************************
 * Name: dev_usb_restart
 *
 * Description:	To restart a previously initialized device without resetting
 *		the device or changing its configuration.
 *		For example, during a failover event.
 *
 *		Change the state of the device from its current state
 *		to an initial state. Also, dev_state must be assigned the
 *		value of DEV_STATE_INIT.
 *   
 * Input: dev_object_t pointer to the CY7C64215 device
 *
 * Returns: PASSED/FAILED
 *
 * Assumptions: The dev_attach() and dev_reconfig_needed() functions has been
 *              called and successfully executed.
 *****************************************************************/
static uint32
dev_usb_restart(dev_object_t *dev)
{
    dev_cy7c64215_object_t *pusb = (dev_cy7c64215_object_t *) dev;

    pusb->base.dev_state = DEV_STATE_INIT;
    return(PASSED);
}

/*****************************************************************
 * Name:	dev_usb_init
 *
 * Description:	Initilialize CY7C64215 contents with data pattern.
 *
 * Input:	dev_object_t pointer to the CY7C64215 device
 *		Data pattern to be initialized to is passed in the object
 *		struct
 *
 * Returns:	PASSED/FAILED
 *
 *****************************************************************/
static uint32
dev_usb_init(dev_object_t *dev)
{
    uint32 rc;
    int block;
    n2g_i2c_if_t new_i2c_if;
    dev_cy7c64215_object_t *pusb = (dev_cy7c64215_object_t *)dev;
    cy7c64215_callout_fvt_t *callout_p = pusb->callout_fvt;
    cy7c64215_cmd_t *init_p;
    uint16_t cmd, temp;
    char err_buf[ERR_BUF_SIZE];
    unsigned char *cmd_ptr;
    char buffer[64];
    usb_t status;


    /* Setup I2C API interface struct */
    memcpy(&new_i2c_if, pusb->i2c_p, sizeof(new_i2c_if));


    /* Points to the first command struct cy7c42145_cmd_t in platform_usb_fw.h */
    init_p = pusb->init_p;
    block = 1;

    if (init_p->size > (sizeof(buffer)-2)) {
        assert(!"firmware buffer is too small. Please increase buffer size.");
    }
    
    while(init_p->ptr) {

        /* we should generate our own offset, embeded into data stream */
        new_i2c_if.size = init_p->size;
        buffer[0] = init_p->size;  /* first  byte will be ignored by bootloader */
        new_i2c_if.size++;
        buffer[1] = new_i2c_if.i2c_dev;  /* second byte will be ignored by bootloader */
        new_i2c_if.size++;
        memcpy(&buffer[2], (char *)init_p->ptr, init_p->size); /* here's the real data */
        new_i2c_if.buf = buffer;

	rc = callout_p->wr(&new_i2c_if);
	if (rc != RC_I2C_OP_OK) {
	    sprintf(err_buf, "dev_usb_init: Write failed block %d. rc = %#x",
							 block, rc);
	    cterr('f', 0, "%s", err_buf);
	    return(FAILED);
	}

	msleep(CY7C64215_CMD_DELAY);

	/* Check for Exit Bootloader command */
	cmd_ptr = init_p->ptr;
	cmd = ((uint16_t)*cmd_ptr) << 8;
	cmd_ptr++;
	temp = ((int16_t)*cmd_ptr) & 0xFF;
	cmd |= temp;
        
	if (cmd != CY7C64215_CMD_EXIT_B) {
            //            printf("block. %d; cmd %#x\n", block, cmd);
	    /* If not exit bootloader command, check the status */
            status = 0;
	    new_i2c_if.size = sizeof(status);
	    new_i2c_if.buf = (char *)&status;
	    rc = callout_p->rd(&new_i2c_if);

	    if (rc != RC_I2C_OP_OK){
		sprintf(err_buf, "dev_usb_init: Read failed block %d. rc = %#x",
								  block, rc);
		cterr('f', 0, "%s", err_buf);
		return(FAILED);
	    } /* endof if rc */

	    if (status != CY7C64215_STAT_SUCCESS) {
		sprintf(err_buf, "dev_usb_init: Block %d err status = %#x",
						      block, status);
#ifdef USB_CONSOLE_DEGUG
		cterr('f', 0, "%s", err_buf);
#else
		DEV_ERROR_REPORT(dev, err_buf, CY7C64215_INIT);
#endif /* USB_CONSOLE_DEGUG */
		return(FAILED);
	    } /* endof if status */
	} else {/* endof if cmd */
            printf("\nFirmware download done.\n");
        }

	block++;
	init_p++;	/* Points to next block */
        if ((block % 5) == 0) {
            printf(".");fflush(stdout);
        }

    } /* endof for */

    return(PASSED);
}

/*****************************************************************
 * Name: dev_usb_crsh
 *
 * Description:	Allow platforms to collect data from a device during a crash.
 *		Print data to the crash log (via the provide print error) using
 *		the appropriate verbisity level requested by the host
 *
 * Input: dev_object_t pointer to the CY7C64215 device
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
dev_usb_crsh(dev_object_t *dev, print_fn_t dev_print, dev_show_cmd verbosity)
{

    /* more development in this section */
    dev_print("dev_usb_crsh(): No Crash info available for CY7C64215\n");
    return(PASSED);
}

/*****************************************************************
 * Name: dev_usb_destroy
 *
 * Description:	Destroy the dev_object structure and free all the resources.
 *
 * Input: dev_object_t pointer to the CY7C64215 device
 *
 * Returns: none
 *
 * Assumptions: The dev_attch() function has been called and successfully
 *
 *****************************************************************/
static void
dev_usb_destroy(dev_object_t **dev)
{
    uint32_t rc;
    dev_cy7c64215_object_t *pusb;
    char err_buf[ERR_BUF_SIZE];

    if (dev == NULL) {
	return;
    }

    if (*dev == NULL) {
	return;
    }

    pusb = (dev_cy7c64215_object_t *)*dev;

    /* Unlock the I2C device */
    if ((rc = pusb->callout_fvt->close(pusb->i2c_p)) != PASSED) {
	sprintf(err_buf, "dev_usb_destroy() I2C close ret. code %#x", rc);
	DEV_ERROR_REPORT(*dev, err_buf, CY7C64215_DESTROY);
	return;
    }

    if (pusb->callout_fvt) {
	free(pusb->callout_fvt);	/* Free callout struct */
    }

    /* Current callin_fvt is empty. bypass the malloc. If not empty,
     * un-comment the following -
     */
    /*
    if (pusb->callin_fvt) {
	free(pusb->callin_fvt);	* Free callin struct *
    }
     */

    free(pusb->base.dev_object_fvt);	/* Free dev_object_t */
}

/******** History ******** 
$Log: dev_cy7c64215.c,v $
Revision 1.5  2013/11/26 08:40:31  hroni
fix compiler warning

Revision 1.4  2012/06/04 10:05:58  aarwang
- Clean up compiler warnings.

Revision 1.3  2012/03/28 00:38:07  mcharon
remove forward slash from second line

Revision 1.2  2012/03/25 03:57:48  palin2
Clean up Overlord Intel side (x86) compile warnings.

Revision 1.1.1.1  2012/03/23 23:02:01  ptong
Initial archive of ng_diag module


$Endlog$
*/
