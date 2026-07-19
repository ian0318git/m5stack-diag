/* $Id: dev_at24c0n.c,v 1.4 2013/11/26 08:40:31 hroni Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_at24c0n_atmel/dev_at24c0n.c,v $
 *------------------------------------------------------------------------------
 *
 * Filename:	dev_at24c0n.c
 *
 * Description:	Atmel AT24C01/02/04 are used in various applications.
 *		This file provides the I2C Common device driver functions used
 *		by these devices.
 *
 *		These devices does not generate interrupt; therefore,
 *		dev_intr_enable,  dev_intr_disable, and dev_isr are
 *		not implemented.
 *
 *		dev_init is used to clear or set a data pattern to the whole
 *		EEPROM.
 *
 *		Only dev_attach, dev_detach, dev_reconfig_needed, dev_restart,
 *		dev_show, dev_err_report, dev_collect_crashinfo, dev_destroy
 *		are implemented.
 *
 *		Refer to Vendor datasheet for more info.
 *
 * Copyright (c) 2007-2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------------------
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <assert.h>
#include "endians.h"
#include "defs.h"
#include "common.h"
#include "dev_at24c0n.h"
#include "dev_print.h"
#include "free.h"
#include "proto.h"

/*===================================================================*
 *                    Function Prototypes                            *
 *===================================================================*/
static uint32   dev_24c0n_attach(dev_object_t *);
static uint32	dev_24c0n_detach(dev_object_t *);
static uint32	dev_24c0n_reconfig(dev_object_t *, void *, boolean *);
static uint32	dev_24c0n_restart(dev_object_t *);
static uint32	dev_24c0n_init(dev_object_t *);
static uint32	dev_24c0n_show(dev_object_t *, print_fn_t, dev_show_cmd);
static uint32	dev_24c0n_crsh(dev_object_t *, print_fn_t, dev_show_cmd);
static void	dev_24c0n_destroy(dev_object_t **);
static int	at_alter_eeprom(dev_object_t *, print_fn_t);
static int	at_eeprom_test(dev_object_t *, print_fn_t, at_eeprom_test_t *);
static int	at_eeprom_program(dev_object_t *, print_fn_t, unsigned char*,
                                  unsigned int);
static int	at_eeprom_addr_test(dev_object_t *, at_o, at_o);
static int	at_eeprom_data_test(dev_object_t *, at_o, at_o, at_t);
static at_o	get_eeprom_size(dev_object_t *);
static int      at_eeprom_read(dev_object_t *, print_fn_t, unsigned char *,
                               unsigned int);

/*****************************************************************
 *
 * Name: dev_at24c0n_create()
 *
 * Description: Create object with various device function
 * point to "do nothing"
 *
 * Input: dev_object_t pointer to the ATMEL EEPROM device.
 *	  error reporting function pointer.
 *
 * Returns: none
 *
 *****************************************************************/
void
dev_at24c0n_create (dev_object_t *dev, dev_error_report_t error_report_fn)
{
    dev_object_fvt_t	*dev_fvt;
    dev_at_object_t *pat = (dev_at_object_t *)dev;

    /* Allocate memory for the device object */
    if ((dev_fvt = (dev_object_fvt_t *)malloc(sizeof(dev_object_fvt_t))) ==
		NULL) {
	/* Unable to allocate memory */
	error_report_fn(dev, "malloc failure in dev_at24c0n_create()", 0);
	return;
    }

    /* Init the device object structure to default "do nothing" */
    init_default_dev_object(dev, dev_fvt);

    pat->base.dev_object_fvt->dev_attach	= dev_24c0n_attach;
    pat->base.dev_object_fvt->dev_detach	= dev_24c0n_detach;
    pat->base.dev_object_fvt->dev_reconfig_needed = dev_24c0n_reconfig;
    pat->base.dev_object_fvt->dev_restart	= dev_24c0n_restart;
    pat->base.dev_object_fvt->dev_init		= dev_24c0n_init;
    pat->base.dev_object_fvt->dev_show		= dev_24c0n_show;
    pat->base.dev_object_fvt->dev_error_report	= error_report_fn;
    pat->base.dev_object_fvt->dev_collect_crashinfo = dev_24c0n_crsh;
    pat->base.dev_object_fvt->dev_destroy	= dev_24c0n_destroy;
    pat->base.dev_object_fvt->dev_name	= "ATMEL AT24C0n EEPROM";

    pat->callin_fvt = (at_callin_fvt_t *)
				malloc(sizeof(at_callin_fvt_t));
    pat->callout_fvt = (at_callout_fvt_t *)
				malloc(sizeof(at_callout_fvt_t));

    pat->base.dev_state = DEV_STATE_CREATE;

}
/*****************************************************************
 *
 * Name: dev_24c0n_attach()
 *
 * Description: Attach the ATMEL EEPROM device for use. This
 *   function will initialize and setup all necessary pointers
 *   and bring the chip to operation.
 *
 * Input: Pointer to the ATMEL EEPROM device object
 *
 * Returns: PASSED/FAILED
 *
 *****************************************************************/
static uint32
dev_24c0n_attach (dev_object_t *dev)
{
    uint32 rc;
    dev_at_object_t *pat = (dev_at_object_t *) dev;
    char err_buf[ERR_BUF_SIZE];

    if (pat->callin_fvt == NULL) {
	DEV_ERROR_REPORT(dev, "dev_24c0n_attach() callin malloc", 
			 AT_ATTACH);
	return(FAILED);
    }

    if (pat->callout_fvt == NULL) {
	DEV_ERROR_REPORT(dev, "dev_24c0n_attach() callout malloc", 
			 AT_ATTACH);
	return(FAILED);
    }

    /* init the call in function */
    pat->callin_fvt->peek_n_poke = at_alter_eeprom;
    pat->callin_fvt->eeprom_test = at_eeprom_test;
    pat->callin_fvt->eeprom_program = at_eeprom_program;
    pat->callin_fvt->eeprom_read   = at_eeprom_read;

    /* Lock the I2C device */
    if ((rc = pat->callout_fvt->open(pat->i2c_p)) != PASSED) {
	sprintf(err_buf, "dev_24c0n_attach() I2C open return rc = %#x", rc);
        DEV_ERROR_REPORT(dev, err_buf, AT_ATTACH);
        return(FAILED);
    }

    pat->base.dev_state = DEV_STATE_ATTACH;

    return (PASSED);
}

/*****************************************************************
 *
 * Name: dev_24c0n_detach()
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
 * Input: Pointer to the ATMEL EEPROM device object
 *
 * Returns: PASSED/FAILED
 *
 *****************************************************************/
static uint32
dev_24c0n_detach (dev_object_t *dev)
{
    uint32 rc;
    dev_at_object_t *pat = (dev_at_object_t *) dev;
    char err_buf[ERR_BUF_SIZE];

    /* Unlock the I2C device */
    if ((rc = pat->callout_fvt->close(pat->i2c_p)) != PASSED) {
	sprintf(err_buf, "dev_24c0n_detach() I2C close rc = %#x", rc);
	DEV_ERROR_REPORT(dev, err_buf, AT_DETACH);
	return(FAILED);
    }

    /* Init the device object structure to default "do nothing" */
    init_default_dev_object(dev, pat->base.dev_object_fvt);

    pat->base.dev_state = DEV_STATE_DETACH;

    return (PASSED);

}

/*****************************************************************
 * Name: dev_24c0n_reconfig_needed
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
 * Input: dev_object_t pointer to the ATMEL EEPROM device
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
dev_24c0n_reconfig(dev_object_t *dev, void *context_handle,
					 boolean *reconfig)
{
    *reconfig = FALSE;		/* No need to reconfig from scratch */
    return(PASSED);
}

/*****************************************************************
 * Name: dev_24c0n_restart
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
dev_24c0n_restart(dev_object_t *dev)
{
    dev_at_object_t *pat = (dev_at_object_t *) dev;

    pat->base.dev_state = DEV_STATE_INIT;
    return(PASSED);
}

/*****************************************************************
 * Name:	dev_24c0n_init
 *
 * Description:	Initilialize EEPROM contents with data pattern.
 *
 * Input:	dev_object_t pointer to the ATMEL EEPROM device
 *		Data pattern to be initialized to is passed in the object
 *		struct
 *
 * Returns:	PASSED/FAILED
 *
 *****************************************************************/
static uint32
dev_24c0n_init(dev_object_t *dev)
{
    uint32 rc;
    n2g_i2c_if_t new_i2c_if;
    dev_at_object_t *pat = (dev_at_object_t *)dev;
    at_callout_fvt_t *callout_p = pat->callout_fvt;
    char err_buf[ERR_BUF_SIZE];
    at_t data;
    at_o i, size;

    memset(&new_i2c_if, 0, sizeof(new_i2c_if));
    
    /* Setup I2C API interface struct */
    new_i2c_if.size = sizeof(data);
    new_i2c_if.buf = (char *)&data;
    new_i2c_if.i2c_bus_type = pat->i2c_p->i2c_bus_type;
    new_i2c_if.i2c_dev = pat->i2c_p->i2c_dev;
    new_i2c_if.rd_hd_size = pat->i2c_p->rd_hd_size;
    new_i2c_if.wr_hd_size = pat->i2c_p->wr_hd_size;
    new_i2c_if.mux = pat->i2c_p->mux;
    //    new_i2c_if.attr = i2c_dev = pat->i2c_p->i2c_dev;
    //    memcpy(&new_i2c_if.attr, &pat->i2c_p->attr, sizeof(n2g_i2c_dev_t));

    /* Get the EEPROM size */
    if ((size = get_eeprom_size(dev)) == 0) {
	return(FAILED);
    }

    data = pat->param;
    for (i = 0; i <= size; i++) {
	new_i2c_if.offset = i;
	rc = callout_p->wr(&new_i2c_if);
	if (rc != PASSED) {
	    sprintf(err_buf, "dev_24c0n_init: Write failed @ %#x. rc = %#x",
							     i, rc);
	    DEV_ERROR_REPORT(dev, err_buf, AT_INIT);
	    return(FAILED);
	}
	msleep(AT24C0X_T_WR);
    } /* endof for */

    return(PASSED);
}


/*****************************************************************
 * Name: dev_24c0n_show
 *
 * Description:	Provide platforms with a mechanism to display some common
 *		device information via the device print function argument.
 *
 * Input: dev_object_t pointer to the ATMEL EEPROM device
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
dev_24c0n_show(dev_object_t *dev, print_fn_t dev_print, dev_show_cmd cmd)
{
    uint32 rc;
    at_t data[4];	/* data bytes from ATMEL EERPOM */
    unsigned int i, j, text_i, size;
    n2g_i2c_if_t new_i2c_if;
    dev_at_object_t *pat = (dev_at_object_t *)dev;
    at_callout_fvt_t *callout_p = pat->callout_fvt;
    dev_at24c0n_desc_t *pdesc;
    char err_buf[ERR_BUF_SIZE], text_buf[18], *buf;

    memcpy(&new_i2c_if, pat->i2c_p, sizeof(new_i2c_if));
    
    switch(cmd) {
    case DEV_SHOW_ALL:
	new_i2c_if.size = sizeof(data);
	new_i2c_if.buf = (char *)&data[0];

	/* Get the EEPROM size */
	if ((size = get_eeprom_size(dev)) == 0) {
	    return(FAILED);
	}

	dev_print("\n	%s EEPROM Contents:\n", pat->dev_name);

	for (i = 0, text_i = 0; i <= size; i += (sizeof(data))) {
	    /* Read the data bytes of ATMEL EEPROM */
	    new_i2c_if.offset = i;

#ifdef I2C_DEBUG
	    printf("dev_24c0n_show(): wrote %#x offset\n", offset);
#endif /* I2C_DEBUG */

	    rc = (*callout_p->rd)(&new_i2c_if);

	    if (rc != PASSED) {
		sprintf(err_buf, "dev_24c0n_show() read %#x rc = %#x",
						i, rc);
		DEV_ERROR_REPORT(dev, err_buf, AT_SHOW);
		return(FAILED);
	    }

	    if ((i % 16) == 0) {
		dev_print("\n 0x%.4X : ", i);
	    }

	    for (j = 0; j < sizeof(data); j++) {
		dev_print(" %02X", data[j]);
		if ((data[j] >= ' ') && (data[j] <= 'z')) {
		    text_buf[text_i + j] = data[j];
		} else {
		    text_buf[text_i + j] = '.';
		}
	    }

	    if (text_i == 12) {
		/* Last 4 bytes of a line. Print the text */
		text_buf[text_i + j] = '\0';
		dev_print("  %s", text_buf);
		text_i = 0;	/* reinitialize the text buffer index */
	    } else {
		dev_print(" ");
		text_i += sizeof(data);
	    }

	} /* endof for */
	break;
    case DEV_SHOW_BRIEF:
	dev_print("\n   %s Contents:\n", pat->dev_name);

	pdesc = pat->init_p;	/* Get the first descriptor */

	while(pdesc->name) {
	    new_i2c_if.size = pdesc->size;
	    new_i2c_if.offset = pdesc->offset;

	    buf = malloc(pdesc->size);
	    if (buf == NULL) {
		sprintf(err_buf, "dev_24c0n_show() malloc %d byte failed",
							  pdesc->size);
		DEV_ERROR_REPORT(dev, err_buf, AT_SHOW);
		return(FAILED);
	    }
	    new_i2c_if.buf = buf;
	    rc = (*callout_p->rd)(&new_i2c_if);
	    if (rc != PASSED) {
		free(buf);
		sprintf(err_buf, "dev_24c0n_show() read %s %d bytes @ %#x "
				 "failed. rc = %#x",
				 pdesc->name, pdesc->size, pdesc->offset, rc);
		DEV_ERROR_REPORT(dev, err_buf, AT_SHOW);
		return(FAILED);
	    }
	    dev_print("%s @ %#x : ", pdesc->name, pdesc->offset);
	    switch(pdesc->type) {
	    case AT_DESC_HEX:
		dev_print("0x");
		for (i = 0; i < pdesc->size; i++) {
		    dev_print("%02x ", (uint8_t)buf[i]);
		} /* endof for */
		break;
	    case AT_DESC_DEC:
		for (i = 0; i < pdesc->size; i++) {
		    dev_print("%d ", buf[i]);
		}
		break;
	    case AT_DESC_TXT:
		for (i = 0; i < pdesc->size; i++) {
		    dev_print("%c", buf[i]);
		}
		break;
	    default:
		assert(!"dev_24c0n_show - type");
		break;
	    } /* endof switch type */
	    dev_print("\n");
	    free(buf);
	    pdesc++;
	} /* endof while */
	break;
    default:
	assert(!"dev_24c0n_show - cmd");
	break;
    } /* endof switch */

    return(PASSED);
}

static int
at_eeprom_read(dev_object_t *dev, print_fn_t dev_print, unsigned char *data,
                  unsigned int len)
{
    uint32 rc;
    //    at_t data[4];	/* data bytes from ATMEL EERPOM */
    unsigned int i;
    n2g_i2c_if_t new_i2c_if;
    dev_at_object_t *pat = (dev_at_object_t *)dev;
    at_callout_fvt_t *callout_p = pat->callout_fvt;
    char err_buf[ERR_BUF_SIZE];
    char eeprom[AT24C02_MAX+1];
    

    memcpy(&new_i2c_if, pat->i2c_p, sizeof(new_i2c_if));
    new_i2c_if.buf = eeprom;
    /* Get the EEPROM size */
    
    for (i = 0; i < len; i+=4) {
        new_i2c_if.buf = &eeprom[i];
        /* Read the data bytes of ATMEL EEPROM */
        new_i2c_if.offset = i;
        rc = (*callout_p->rd)(&new_i2c_if);
        if (rc != PASSED) {
            sprintf(err_buf, "dev_24c0n_read()  %#x rc = %#x",
						i, rc);
            DEV_ERROR_REPORT(dev, err_buf, AT_SHOW);
            return(FAILED);
        }
    }
    memcpy(pat->i2c_p->buf, &new_i2c_if.buf[0], len);
    
    return(PASSED);
}

/*****************************************************************
 * Name: dev_24c0n_crsh
 *
 * Description:	Allow platforms to collect data from a device during a crash.
 *		Print data to the crash log (via the provide print error) using
 *		the appropriate verbisity level requested by the host
 *
 * Input: dev_object_t pointer to the ATMEL EEPROM device
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
dev_24c0n_crsh(dev_object_t *dev, print_fn_t dev_print, dev_show_cmd verbosity)
{
    dev_at_object_t *pat = (dev_at_object_t *)dev;

    /* more development in this section */
    dev_print("dev_24c0n_crsh(): No Crash info available for %s\n",
						pat->dev_name);
    return(PASSED);
}

/*****************************************************************
 * Name: dev_24c0n_destroy
 *
 * Description:	Destroy the dev_object structure and free all the resources.
 *
 * Input: dev_object_t pointer to the ATMEL EEPROM device
 *
 * Returns: none
 *
 * Assumptions: The dev_attch() function has been called and successfully
 *
 *****************************************************************/
static void
dev_24c0n_destroy(dev_object_t **dev)
{
    uint32_t rc;
    dev_at_object_t *pat;
    char err_buf[ERR_BUF_SIZE];

    if (dev == NULL) {
	return;
    }

    if (*dev == NULL) {
	return;
    }

    pat = (dev_at_object_t *)*dev;

    /* Unlock the I2C device */
    if ((rc = pat->callout_fvt->close(pat->i2c_p)) != PASSED) {
	sprintf(err_buf, "dev_24c0n_destroy() I2C close ret. code %#x", rc);
	DEV_ERROR_REPORT(*dev, err_buf, AT_DESTROY);
	return;
    }

    if (pat->callout_fvt) {
	free(pat->callout_fvt);	/* Free callout struct */
    }

    if (pat->callin_fvt) {
	free(pat->callin_fvt);	/* Free callin struct */
    }

    free(pat->base.dev_object_fvt);	/* Free dev_object_t */
}

/********************************************************************
 *
 * Function:	at_alter_eeprom
 *
 * Description:	Peek-n-poke AT24C0x byte location.
 *
 * Inputs:	dev_object_t pointer to the AT24C0x device
 *		A device print function vector
 *
 * Outputs:	PASSED - No errors encounterd.
 *		FAILED - Errors encounterd.
 *
 * Assumptions:	create and dev_attach have to be called first. dev_destroy will
 *		also be called after the exit.
 *
 *********************************************************************
 */
static int
at_alter_eeprom(dev_object_t *dev, print_fn_t dev_print)
{
    n2g_i2c_if_t new_i2c_if;
    uint32_t rc;
    at_o dev_size;
    dev_at_object_t *pat = (dev_at_object_t *)dev;
    at_callout_fvt_t *callout_p = pat->callout_fvt;
    register char *c_ptr;
    int tmp;
    uint val;
#ifdef LINUX_APP
    char err_buf[ERR_BUF_SIZE], inbuf[4], done = FALSE;
#else
    char err_buf[ERR_BUF_SIZE], inbuf[3], done = FALSE;
#endif
    at_t old_data, new_data;
    at_o addr;
    memset(&new_i2c_if, 0, sizeof(new_i2c_if));
    //    memcpy(&new_i2c_if.attr, &pat->i2c_p->attr, sizeof(n2g_i2c_dev_t));
    /* Setup I2C API interface struct */
    new_i2c_if.size = sizeof(at_t);
    new_i2c_if.i2c_bus_type = pat->i2c_p->i2c_bus_type;
    new_i2c_if.i2c_dev = pat->i2c_p->i2c_dev;
    new_i2c_if.rd_hd_size = pat->i2c_p->rd_hd_size;
    new_i2c_if.wr_hd_size = pat->i2c_p->wr_hd_size;
    new_i2c_if.mux = pat->i2c_p->mux;
    
    if ((dev_size = get_eeprom_size(dev)) == 0) {
	return(FAILED);
    }

    /* Get the location to peek-n-poke */
    addr = gethex_answer("Enter the starting address:", 0, 0, dev_size);

    dev_print("Enter the data bytes. x or q to quit\n");

    while((addr <= dev_size) && (done == FALSE)) {
	/* Read the data first. */
	new_i2c_if.buf = (char *)&old_data;
	new_i2c_if.offset = addr;

	rc = (*callout_p->rd)(&new_i2c_if);
	if (rc != PASSED) {
	    sprintf(err_buf, "at_alter_eeprom() read offset %#x rc = %#x",
						addr, rc);
	    DEV_ERROR_REPORT(dev, err_buf, AT_ALTER);
	    return(FAILED);
	} /* endof if rc */

	dev_print("0x%.2x @ 0x%.2x ==> ", old_data, addr);

        c_ptr = inbuf;
	get_line(c_ptr, sizeof(inbuf));

	switch (*c_ptr) {
	case 'x': /* exit */
	case 'q': /* quit */
	case 'X': /* exit */
	case 'Q': /* quit */
	    done = TRUE;
	    break;
	case 0:	/* next location */
#ifdef LINUX_APP
	case '\n':   /* next location */
	case '\r':   /* next location */
#endif
	    break;
	default:
	    tmp = getnum(c_ptr, 16, &val);
#ifdef LINUX_APP
            if (tmp >= 3) {
		dev_print("Too much input \"%s\"\n", c_ptr);
		continue;	/* Same location again */
            }
#endif
	    if (tmp == 0) {
		dev_print("bad value \"%s\"\n", c_ptr);
		continue;	/* Same location again */
	    } else {
		new_data = (at_t)val;

		/* Write the new data */
		new_i2c_if.buf = (char *)&new_data;

		rc = (*callout_p->wr)(&new_i2c_if);
		if (rc != PASSED) {
		    sprintf(err_buf, "at_alter_eeprom() write failed. rc = %#x",
						 rc);
		    DEV_ERROR_REPORT(dev, err_buf, AT_ALTER);
		    return(FAILED);
		} /* endof if rc */
		msleep(AT24C0X_T_WR + 1);
	    } /* endof if tmp */
	    break; /* next location */
	} /* endof switch */
	addr++;
    } /* endof while */

    return(PASSED);
}

static int
at_eeprom_program(dev_object_t *dev, print_fn_t dev_print, unsigned char *data,
                  unsigned int len)
{
    n2g_i2c_if_t new_i2c_if;
    uint32_t rc;
    at_o dev_size;
    dev_at_object_t *pat = (dev_at_object_t *)dev;
    at_callout_fvt_t *callout_p = pat->callout_fvt;
    char err_buf[ERR_BUF_SIZE];
    at_o addr;
    memset(&new_i2c_if, 0, sizeof(new_i2c_if));
    //    memcpy(&new_i2c_if.attr, &pat->i2c_p->attr, sizeof(n2g_i2c_dev_t));
    /* Setup I2C API interface struct */
    new_i2c_if.size = sizeof(at_t);
    new_i2c_if.i2c_bus_type = pat->i2c_p->i2c_bus_type;
    new_i2c_if.i2c_dev = pat->i2c_p->i2c_dev;
    new_i2c_if.rd_hd_size = pat->i2c_p->rd_hd_size;
    new_i2c_if.wr_hd_size = pat->i2c_p->wr_hd_size;
    new_i2c_if.mux = pat->i2c_p->mux;
    
    if ((dev_size = get_eeprom_size(dev)) == 0) {
	return(FAILED);
    }

    /* Get the location to peek-n-poke */
    addr = 0; 
    while((addr <= dev_size)) {

	new_i2c_if.offset = addr;
        /* Write the new data */
        new_i2c_if.buf = (char *)data;

        rc = (*callout_p->wr)(&new_i2c_if);
        if (rc != PASSED) {
            sprintf(err_buf, "at_alter_program() write failed. rc = %#x",
                    rc);
            DEV_ERROR_REPORT(dev, err_buf, AT_ALTER);
            return(FAILED);
        } /* endof if rc */
        msleep(AT24C0X_T_WR + 1);
        addr++;
        data++;
        len--;
        if (len==0)
            break;
    } /* endof if tmp */


    return(PASSED);

}

/********************************************************************
 *
 * Function:	at_eeprom_test
 *
 * Description:	Memory test of AT24C0x device.
 *
 * Warnings:	According to ATMEL datasheet, the "Endurance" (under AC
 *		Characteristics) is 1 million write cycles. Use of this test
 *		over this limit during the life cycle of the box can cause
 *		damage to the chip. Exercise caution in use of this test.
 *
 * Inputs:	dev_object_t pointer to the AT24C0x device
 *		A device print function vector
 *		Pointer to a struct with parameters. NULL pointer for all bytes.
 *
 * Outputs:	PASSED - No errors encounterd.
 *		FAILED - Errors encounterd.
 *
 * Assumptions:	create and dev_attach have to be called first. dev_destroy will
 *		also be called after the exit.
 *
 *********************************************************************
 */
static int
at_eeprom_test(dev_object_t *dev, print_fn_t dev_print, at_eeprom_test_t *t_if)
{
    n2g_i2c_if_t new_i2c_if;
    uint32_t rc;
    dev_at_object_t *pat = (dev_at_object_t *)dev;
    at_callout_fvt_t *callout_p = pat->callout_fvt;
    uint i, j, text_i;
    at_t *p_buf_p, *buf_p;
    at_o dev_size, start_addr;
    char err_buf[ERR_BUF_SIZE], text_buf[18];

    memset(&new_i2c_if, 0, sizeof(new_i2c_if));
    //    memcpy(&new_i2c_if.attr, &pat->i2c_p->attr, sizeof(n2g_i2c_dev_t));
    
    dev_print("\nWARNING: This test is destructive. "
	      "You may lose the original data\n");
    if (getc_answer("Continue ?", "yn", 'n') == 'n') {
	return(PASSED);
    }

    /* Setup I2C API interface struct */
    new_i2c_if.i2c_bus_type = pat->i2c_p->i2c_bus_type;
    new_i2c_if.i2c_dev = pat->i2c_p->i2c_dev;
    new_i2c_if.rd_hd_size = pat->i2c_p->rd_hd_size;
    new_i2c_if.wr_hd_size = pat->i2c_p->wr_hd_size;
    new_i2c_if.mux = pat->i2c_p->mux;
    
    if (t_if) {
	/* User provided parameters */
	start_addr = t_if->start;
	dev_size = t_if->size;
    } else {
	/* Use the whole device */
	dev_size = get_eeprom_size(dev) + 1;
	if (dev_size == 0) {
	    /* Unable to find the EEPROM size */
	    return(FAILED);
	}
	start_addr = 0;		/* start from beginning */
    }

    /* Allocate memory for preserving existing content */
    p_buf_p = (at_t *)malloc(sizeof(at_t) * dev_size);

    if (p_buf_p == NULL) {
	/* Unable to allocate memory */
	DEV_ERROR_REPORT(dev, "at_eeprom_test() malloc failed", AT_MEM_TEST);
	return(FAILED);
    }

    /* Read the data */
    new_i2c_if.offset = start_addr;
    new_i2c_if.buf = (char *)p_buf_p;

    for (i = 0; i < dev_size; ) {
	/* Note that the size field of the I2C API struct is 255 bytes max */
	if ((dev_size - i) > I2C_MAX_XFER_SIZE) {
	    new_i2c_if.size = I2C_MAX_XFER_SIZE;
	} else {
	    new_i2c_if.size = (dev_size - i);
	}

	rc = (*callout_p->rd)(&new_i2c_if);
	if (rc != PASSED) {
	    free(p_buf_p);
	    sprintf(err_buf, "at_eeprom_test() read current contents failed @ "
			     "%#x with rc = %#x", new_i2c_if.offset, rc);
	    DEV_ERROR_REPORT(dev, err_buf, AT_MEM_TEST);
	    return(FAILED);
	} /* endof if rc */

	i += new_i2c_if.size;
	new_i2c_if.offset += new_i2c_if.size;
	new_i2c_if.buf += new_i2c_if.size;
    }

    /* Print out existing contents first */
    dev_print("\n        The original data - \n");
    /* Clear the character buffers */
    for (j = 0; j < 16; j++) {
	text_buf[j] = ' ';
    }

    for (i = start_addr, text_i = start_addr % 16, buf_p = p_buf_p;
			i < dev_size; ) {
	if ((i % 16) == 0) {
	    dev_print("\n %#.4X : ", i);
	}

	for (j = 0; (j < 16) && (i < dev_size); i++, j++, buf_p++) {
	    dev_print(" %02X", *buf_p);
	    if ((*buf_p >= ' ') && (*buf_p <= 'z')) {
		text_buf[text_i + j] = *buf_p;
	    } else {
		text_buf[text_i + j] = '.';
	    }
	} /* endof for j */

	/* Last 4 bytes of a line. Print the text */
	text_buf[17] = '\0';
	dev_print("  %s", text_buf);
	text_i = 0;	/* reinitialize the text buffer index */
    } /* endof for i */

    /* Address pattern test */
    rc = at_eeprom_addr_test(dev, start_addr, dev_size);
    if (rc == PASSED) {
	/* Data pattern ones tests */
	rc = at_eeprom_data_test(dev, start_addr, dev_size, 0xFF);
	if (rc == PASSED) {
	    /* Data pattern zeroes tests */
	    rc = at_eeprom_data_test(dev, start_addr, dev_size, 0xFF);
	    if (rc != PASSED) {
		sprintf(err_buf, "at_eeprom_test() all zeroes test failed.");
	    }
	} else {
	    sprintf(err_buf, "at_eeprom_test() all ones test failed.");
	}
    } else {
	sprintf(err_buf, "at_eeprom_test() Address pattern test failed.");
    }

    if (rc != PASSED) {
	DEV_ERROR_REPORT(dev, err_buf, AT_MEM_TEST);
    }

    /* Restore the original contends */
    new_i2c_if.offset = start_addr;
    new_i2c_if.buf = (char *)p_buf_p;

    for (i = 0; i < dev_size; ) {
	/* Use Page write */
	if ((dev_size - i) > AT_PAGE_WRITE_MAX) {
	    new_i2c_if.size = AT_PAGE_WRITE_MAX;
	} else {
	    new_i2c_if.size = dev_size - i;
	}

	rc = (*callout_p->wr)(&new_i2c_if);

	if (rc != PASSED) {
	    break;
	}

	i += new_i2c_if.size;
	new_i2c_if.offset += new_i2c_if.size;
	new_i2c_if.buf += new_i2c_if.size;
	msleep(AT24C0X_T_WR + 1);
    } /* endof for */

    free(p_buf_p);

    if (rc != PASSED) {
	sprintf(err_buf, "at_eeprom_test() Unable to restore. rc = %#x", rc);
	DEV_ERROR_REPORT(dev, err_buf, AT_MEM_TEST);
    }

    return(rc);

}

/********************************************************************
 *
 * Function:	at_eeprom_addr_test
 *
 * Description:	AT24C0x address pattern test.
 *
 * Inputs:	dev_object_t pointer to the AT24C0x device
 *		Starting address to be tested.
 *		Number of bytes to be tested.
 *
 * Outputs:	PASSED - No errors encounterd.
 *		FAILED - Errors encounterd.
 *
 * Assumptions:
 *
 *********************************************************************
 */
static int
at_eeprom_addr_test(dev_object_t *dev, at_o start, at_o size)
{
    n2g_i2c_if_t new_i2c_if;
    uint32_t rc = PASSED;
    uint i;
    at_t *wr_buf_p, *buf_p;
    dev_at_object_t *pat = (dev_at_object_t *)dev;
    at_callout_fvt_t *callout_p = pat->callout_fvt;
    char err_buf[ERR_BUF_SIZE];
    at_t pattern;

#ifdef EEPROM_DEBUG
    printf("\nat_eeprom_addr_test() start %#x, %#x bytes\n", start, size);
#endif /* EEPROM_DEBUG */
    memset(&new_i2c_if, 0, sizeof(new_i2c_if));
    //    memcpy(&new_i2c_if.attr, &pat->i2c_p->attr, sizeof(n2g_i2c_dev_t));
    
    wr_buf_p = (at_t *)malloc(sizeof(at_t) * size);

    if (wr_buf_p == NULL) {
	/* Unable to allocate memory */
	DEV_ERROR_REPORT(dev, "at_eeprom_addr_test() malloc failed",
			 AT_MEM_TEST);
	return(FAILED);
    }

    /* Setup I2C API interface struct */
    new_i2c_if.i2c_bus_type = pat->i2c_p->i2c_bus_type;
    new_i2c_if.i2c_dev = pat->i2c_p->i2c_dev;
    new_i2c_if.rd_hd_size = pat->i2c_p->rd_hd_size;
    new_i2c_if.wr_hd_size = pat->i2c_p->wr_hd_size;
    new_i2c_if.mux = pat->i2c_p->mux;
    
    for (i = 0, pattern = (at_t)start, buf_p = wr_buf_p; i < size;
		i++, pattern++, buf_p++) {
	*buf_p = pattern;
    }

    /* Page write the data. Up to 8 bytes per transaction */
    new_i2c_if.offset = start;
    new_i2c_if.buf = (char *)wr_buf_p;

    for (i = 0; i < size; ) {
	/* Page write can only handle up to 8 bytes */
	if ((size - i) > AT_PAGE_WRITE_MAX) {
	    new_i2c_if.size = AT_PAGE_WRITE_MAX;
	} else {
	    new_i2c_if.size = size - i;
	}

	rc = (*callout_p->wr)(&new_i2c_if);

	if (rc != PASSED) {
	    break;
	}

	i += new_i2c_if.size;
	new_i2c_if.offset += new_i2c_if.size;
	new_i2c_if.buf += new_i2c_if.size;
	msleep(AT24C0X_T_WR + 1);
    } /* endof for */

    if (rc != PASSED) {
	sprintf(err_buf, "at_eeprom_addr_test() write failed. rc = %#x", rc);
	free(wr_buf_p);
	DEV_ERROR_REPORT(dev, err_buf, AT_MEM_TEST);
	return(FAILED);
    } /* endof if rc */

    /* Clear the buffer for the read */
    for (i = 0, buf_p = wr_buf_p; i < size; i++, buf_p++) {
	*buf_p = 0;
    }

    /* Page read the data */
    new_i2c_if.offset = start;
    new_i2c_if.buf = (char *)wr_buf_p;

    for (i = 0; i < size; ) {
	/* Note that the size field of the I2C API struct is 255 bytes max */
	if ((size - i) > I2C_MAX_XFER_SIZE) {
	    new_i2c_if.size = I2C_MAX_XFER_SIZE;
	} else {
	    new_i2c_if.size = size - i;
	}

	rc = (*callout_p->rd)(&new_i2c_if);
	if (rc != PASSED) {
	    free(wr_buf_p);
	    sprintf(err_buf, "at_eeprom_addr_test() read failed @ "
			     "%#x with rc = %#x", new_i2c_if.offset, rc);
	    DEV_ERROR_REPORT(dev, err_buf, AT_MEM_TEST);
	    return(FAILED);
	} /* endof if rc */

	i += new_i2c_if.size;
	new_i2c_if.offset += new_i2c_if.size;
	new_i2c_if.buf += new_i2c_if.size;
    }

    /* Check the data read */
    for (i = 0, pattern = (at_t)start, buf_p = wr_buf_p; i < size;
		i++, pattern++, buf_p++) {
	if (*buf_p != pattern) {
	    /* Data miss compare */
	    sprintf(err_buf, "at_eeprom_addr_test() Expect %#x. Read %#x",
			      pattern, *buf_p);
	    free(wr_buf_p);
	    DEV_ERROR_REPORT(dev, err_buf, AT_MEM_TEST);
	    return(FAILED);
	}
    }

    free(wr_buf_p); /* Free the buffer */

    return(PASSED);
}

/********************************************************************
 *
 * Function:	at_eeprom_data_test
 *
 * Description:	AT24C0x data pattern test.
 *
 * Inputs:	dev_object_t pointer to the AT24C0x device
 *		Starting address to be tested.
 *		Number of bytes to be tested.
 *		Data pattern to be tested.
 *
 * Outputs:	PASSED - No errors encounterd.
 *		FAILED - Errors encounterd.
 *
 * Assumptions:
 *
 *********************************************************************
 */
static int
at_eeprom_data_test(dev_object_t *dev, at_o start, at_o size, at_t pattern)
{
    n2g_i2c_if_t new_i2c_if;
    uint32_t rc = PASSED;
    uint i;
    at_t *wr_buf_p, *buf_p;
    dev_at_object_t *pat = (dev_at_object_t *)dev;
    at_callout_fvt_t *callout_p = pat->callout_fvt;
    char err_buf[ERR_BUF_SIZE];

    wr_buf_p = (at_t *)malloc(sizeof(at_t) * size);

    if (wr_buf_p == NULL) {
	/* Unable to allocate memory */
	DEV_ERROR_REPORT(dev, "at_eeprom_data_test() malloc failed",
			 AT_MEM_TEST);
	return(FAILED);
    }
    memset(&new_i2c_if, 0, sizeof(new_i2c_if));
    //    memcpy(&new_i2c_if.attr, &pat->i2c_p->attr, sizeof(n2g_i2c_dev_t));
    
    /* Setup I2C API interface struct */
    new_i2c_if.i2c_bus_type = pat->i2c_p->i2c_bus_type;
    new_i2c_if.i2c_dev = pat->i2c_p->i2c_dev;
    new_i2c_if.rd_hd_size = pat->i2c_p->rd_hd_size;
    new_i2c_if.wr_hd_size = pat->i2c_p->wr_hd_size;
    new_i2c_if.mux = pat->i2c_p->mux;
    
    for (i = 0, buf_p = wr_buf_p; i < size; i++, buf_p++) {
	*buf_p = pattern;
    }

    /* Page write the data. Up to 8 bytes per transaction */
    new_i2c_if.offset = start;
    new_i2c_if.buf = (char *)wr_buf_p;

    for (i = 0; i < size; ) {
	/* Page write can only handle up to 8 bytes */
	if ((size - i) > AT_PAGE_WRITE_MAX) {
	    new_i2c_if.size = AT_PAGE_WRITE_MAX;
	} else {
	    new_i2c_if.size = size - i;
	}

	rc = (*callout_p->wr)(&new_i2c_if);
	if (rc != PASSED) {
	    break;
	}

	i += new_i2c_if.size;
	new_i2c_if.offset += new_i2c_if.size;
	new_i2c_if.buf += new_i2c_if.size;
	msleep(AT24C0X_T_WR + 1);
    } /* endof for */

    if (rc != PASSED) {
	sprintf(err_buf, "at_eeprom_data_test() write failed. rc = %#x", rc);
	free(wr_buf_p);
	DEV_ERROR_REPORT(dev, err_buf, AT_MEM_TEST);
	return(FAILED);
    } /* endof if rc */

    /* Clear the buffer for the read */
    for (i = 0, buf_p = wr_buf_p; i < size; i++, buf_p++) {
	*buf_p = 0;
    }

    /* Page read the data */
    new_i2c_if.offset = start;
    new_i2c_if.buf = (char *)wr_buf_p;

    for (i = 0; i < size; ) {
	/* Note that the size field of the I2C API struct is 255 bytes max */
	if ((size - i) > I2C_MAX_XFER_SIZE) {
	    new_i2c_if.size = I2C_MAX_XFER_SIZE;
	} else {
	    new_i2c_if.size = size - i;
	}

	rc = (*callout_p->rd)(&new_i2c_if);
	if (rc != PASSED) {
	    sprintf(err_buf, "at_eeprom_data_test() read failed. rc = %#x", rc);
	    free(wr_buf_p);
	    DEV_ERROR_REPORT(dev, err_buf, AT_MEM_TEST);
	    return(FAILED);
	} /* endof if rc */

	i += new_i2c_if.size;
	new_i2c_if.offset += new_i2c_if.size;
	new_i2c_if.buf += new_i2c_if.size;
    }

    /* Check the data read */
    for (i = 0, buf_p = wr_buf_p; i < size; i++, buf_p++) {
	if (*buf_p != pattern) {
	    /* Data miss compare */
	    sprintf(err_buf, "at_eeprom_data_test() Expect %#x. Read %#x @ %#x",
			      pattern, *buf_p, i + start);
	    free(wr_buf_p);
	    DEV_ERROR_REPORT(dev, err_buf, AT_MEM_TEST);
	    return(FAILED);
	}
    }

    free(wr_buf_p); /* Free the buffer */

    return(PASSED);
}

/*******************************************************************************
 * Function:	get_eeprom_size
 *
 * Description:	Get the EEPROM size - 1
 *
 * Inputs:	dev - pointer to the ATMEL EEPROM device object.
 *
 * Output:	device size. 0, if invalid device type
 *******************************************************************************
 */
static at_o
get_eeprom_size(dev_object_t *dev)
{
    dev_at_object_t *pat = (dev_at_object_t *)dev;
    char err_buf[ERR_BUF_SIZE];

    /* Get the EEPROM size */
    switch(pat->dev_type) {
    case AT24C_01:
	return(AT24C01_MAX);
	break;
    case AT24C_02:
	return(AT24C02_MAX);
	break;
    case AT24C_04:
	return(AT24C04_MAX);
	break;
    default:
	sprintf(err_buf, "get_eeprom_size() Unknown EEPROM type %#x",
						pat->dev_type);
	DEV_ERROR_REPORT(dev, err_buf, AT_GET_SIZE);
	return(0);
    }

}

/******** History ******** 
$Log: dev_at24c0n.c,v $
Revision 1.4  2013/11/26 08:40:31  hroni
fix compiler warning

Revision 1.3  2012/06/04 10:05:56  aarwang
- Clean up compiler warnings.

Revision 1.2  2012/03/28 00:38:06  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:01  ptong
Initial archive of ng_diag module


$Endlog$
*/
