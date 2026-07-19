/* $Id: dev_dimm.c,v 1.4 2013/11/26 08:40:31 hroni Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_dimm/dev_dimm.c,v $
 *------------------------------------------------------------------------------
 *
 * Filename:	dev_dimm.c
 *
 * Description:	SPD DIMM driver	functions.
 *
 *		SPD DIMM does not generate interrupt; therefore,
 *		dev_intr_enable, dev_intr_disable, dev_isr are not used.
 *		Since the DIMM SPD interface is attached to the EEPROM,
 *		there is no need for dev_reconfig_needed, dev_restart,
 *		dev_init, dev_oper_enable and dev_oper_disable.
 *		Only dev_attach, dev_detach, dev_show, dev_err_report,
 *		dev_collect_crashinfo, dev_destroy are implemented.
 *
 * Copyright (c) 2007-2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------------------
 *
 */

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "endians.h"
#include "types.h"
#include "defs.h"
#include "common.h"
#include "common_utils.h"
#include "dev_print.h"
#include "dev_object.h"
#include "dev_dimm.h"
#include "free.h"
#include "proto.h"
#include "n2g_api_rc.h"

/*===================================================================*
 *                    Function Prototypes                            *
 *===================================================================*/
static uint32   dimm_dev_attach(dev_object_t *);
static uint32	dimm_dev_detach(dev_object_t *);
static uint32	dimm_dev_show(dev_object_t *, print_fn_t, dev_show_cmd);
static uint32	dimm_crsh(dev_object_t *, print_fn_t, dev_show_cmd);
static void	dimm_destroy(dev_object_t **);
static int	dimm_get_memsize(dev_object_t *);
 
/*****************************************************************
 *
 * Name: spd_dimm_dev_create()
 *
 * Description: Create object with various device function point to "do nothing"
 *
 * Input: dev_object_t pointer to the SPD DIMM device.
 *	  error reporting function pointer.
 *	  dev_fvt pointer to the device function vector table.
 *
 * Returns: none
 *
 *****************************************************************/
void
spd_dimm_dev_create (dev_object_t *dev, dev_error_report_t error_report_fn,
		     dev_object_fvt_t *dev_fvt)
{
    dev_object_fvt_t *fvt;
    dev_dimm_object_t *dimm = (dev_dimm_object_t *)dev;

    if (dev_fvt) {
	/* Device function vector table already allocated */
	fvt = dev_fvt;
    } else {
	/* Allocate memory for the device object */
	if ((fvt = (dev_object_fvt_t *)malloc(sizeof(dev_object_fvt_t))) ==
		NULL) {
	    /* Unable to allocate memory */
	    error_report_fn(dev, "malloc failure in spd_dimm_dev_create()",
			0);
	    return;
	}
    }

    /* Init the device object structure to default "do nothing" */
    dimm->base.dev_state = DEV_STATE_CREATE;
    init_default_dev_object(dev, fvt);

    dimm->base.dev_object_fvt->dev_attach	   = dimm_dev_attach;
    dimm->base.dev_object_fvt->dev_detach	   = dimm_dev_detach;
    dimm->base.dev_object_fvt->dev_error_report = error_report_fn;
    dimm->base.dev_object_fvt->dev_show	   = dimm_dev_show;
    dimm->base.dev_object_fvt->dev_collect_crashinfo = dimm_crsh;
    dimm->base.dev_object_fvt->dev_destroy	   = dimm_destroy;
    dimm->base.dev_object_fvt->dev_name = "SPD DIMM";

    if (dev_fvt) {
	/* Caller will provide callin and callout */
	return;
    }

    dimm->callin_fvt = (dimm_callin_fvt_t *)
				malloc(sizeof(dimm_callin_fvt_t));
    dimm->callout_fvt = (dimm_callout_fvt_t *)
				malloc(sizeof(dimm_callout_fvt_t));
}
/*****************************************************************
 *
 * Name: dimm_dev_attach()
 *
 * Description: Attach the SPD DIMM device for use. This
 *   function will initialize and setup all necessary pointers
 *   and bring the chip to operation.
 *
 * Input: Pointer to the SPD DIMM device object
 *
 * Returns: PASSED/FAILED
 *
 *****************************************************************/
static uint32
dimm_dev_attach (dev_object_t *dev)
{
    uint32 rc;
    dev_dimm_object_t *dimm = (dev_dimm_object_t *) dev;

    if (dimm->callin_fvt == NULL) {
	DEV_ERROR_REPORT(dev, "dimm_dev_attach() callin malloc",
			 DIMM_ATTACH);
	return(FAILED);
    }

    if (dimm->callout_fvt == NULL) {
	DEV_ERROR_REPORT(dev, "dimm_dev_attach() callout malloc", 
			 DIMM_ATTACH);
	return(FAILED);
    }

    /* init the call in function */
    dimm->callin_fvt->get_memsize = dimm_get_memsize;

    /* Lock the I2C device */
    if ((rc = dimm->callout_fvt->open(dimm->i2c_p)) != PASSED) {
        DEV_ERROR_REPORT(dev, "dimm_dev_attach() I2C open", rc);
        return(FAILED);
    }

    dimm->base.dev_state = DEV_STATE_ATTACH;

    return (PASSED);
}

/*****************************************************************
 *
 * Name: dimm_dev_detach()
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
 * Input: Pointer to the SPD DIMM device object
 *
 * Returns: PASSED/FAILED
 *
 *****************************************************************/
static uint32
dimm_dev_detach (dev_object_t *dev)
{
    uint32 rc;
    dev_dimm_object_t *dimm = (dev_dimm_object_t *) dev;

    /* Unlock the I2C device */
    if ((rc = dimm->callout_fvt->close(dimm->i2c_p)) != PASSED) {
	DEV_ERROR_REPORT(dev, "dimm_dev_detach() I2C close", rc);
	return(FAILED);
    }

    /* Init the device object structure to default "do nothing" */
    init_default_dev_object(dev, dimm->base.dev_object_fvt);

    dimm->base.dev_state = DEV_STATE_DETACH;

    return (PASSED);

}

/*****************************************************************
 * Name: dimm_dev_show
 *
 * Description:	Provide platforms with a mechanism to display some common
 *		device information via the device print function argument.
 *
 * Input: dev_object_t pointer to the SPD DIMM device
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
dimm_dev_show(dev_object_t *dev, print_fn_t dev_print, dev_show_cmd cmd)
{
    uint32 i, rc;
    uint32_t *dimm_p;
    uchar *dimm_ch_p;
    dimm_i2c_t dimm;
    n2g_i2c_if_t i2c_if;
    dev_dimm_object_t *dimm_obj = (dev_dimm_object_t *)dev;
    dimm_callout_fvt_t *callout_p = dimm_obj->callout_fvt;
    uint32_t ser_no = 0;
    char buf[20], err_buf[80];

    /* Setup the interface struct for I2C API read */
    i2c_if.i2c_bus_type = dimm_obj->i2c_p->i2c_bus_type;
    i2c_if.i2c_dev = dimm_obj->i2c_p->i2c_dev;

    /* Read the bytes from SPD DIMM */
    i2c_if.size = sizeof(uint32_t);	/* Read 4 bytes at a time */
    for (i = 0, dimm_p = (uint32_t *)&dimm; i < sizeof(dimm_i2c_t);
					   i += sizeof(uint32_t), dimm_p++) {
	i2c_if.offset = i;
	i2c_if.buf = (char *)dimm_p;

	rc = (*callout_p->rd)(&i2c_if);
	if (rc != PASSED) {
	    break;	/* Read failed */
	}
    }

    if (rc != PASSED) {
	if (rc == E_I2C_INV_ACK) {
	    dev_print("\n%s is not installed.\n", dimm_obj->dev_name);
	    return(PASSED);
	} else {
	    sprintf(err_buf, "dimm_dev_show() read rc = %#.8x offset %#.8x",
						rc, i);
	    DEV_ERROR_REPORT(dev, err_buf, DIMM_SHOW);
	    return(FAILED);
	}
    }

    switch (cmd) {
    case DEV_SHOW_ALL:
    case DEV_SHOW_CONFIG:
    case DEV_SHOW_REGISTERS:
	/* Display the info */
	dev_print("\nNumber of SPD bytes used = %d\n", dimm.size);
	dev_print("Total number of bytes in SPD device = %02x\n",
							dimm.dev_size);
	dev_print("Fundamental memory type = %02x\n", dimm.type);
	dev_print("Number of row address on SDRAM    = %d\n", dimm.row_addr);
	dev_print("Number of column address on SDRAM = %d\n", dimm.column_addr);
	dev_print("DIMM height and module ranks = %02x\n", dimm.height);
	dev_print("Module data width = %d\n", dimm.data_width);
	dev_print("Module voltage interface levels = %#x\n", dimm.v_if);
	dev_print("SDRAM cycle time tCK CAS latency of 4.0  = %#x\n",
							dimm.tck_l4);
	dev_print("SDRAM access time tAC CAS latency of 4.0 = %#x\n",
							dimm.tac_l4);
	dev_print("Module configuration type = %#x\n", dimm.conf_type);
	dev_print("Refresh rate/type = %#x\n", dimm.ref_rate);
	dev_print("SDRAM device width = %d\n", dimm.dev_width);
	dev_print("Error-checking SDRAM data width = %#x\n", dimm.err_data_w);
	dev_print("Burst lengths supported = %#x\n", dimm.burst_len);
	dev_print("Number of banks on SDRAM device = %d\n", dimm.banks);
	dev_print("CAS latencies supported = %#x\n", dimm.cas_lat);
	dev_print("Module thickness = %#x\n", dimm.mod_thick);
	dev_print("DDR2 DIMM type = %#x\n", dimm.ddr2_type);
	dev_print("SDRAM module attributes = %#x\n", dimm.mod_att);
	dev_print("SDRAM device attributes = %#x\n", dimm.dev_att);
	dev_print("SDRAM cycle time tCK CAS latency of 3.0  = %#x\n",
							dimm.tck_l3);
	dev_print("SDRAM access time tAC CAS latency of 3.0 = %#x\n",
							dimm.tac_l3);
	dev_print("SDRAM cycle time tCK CAS latency of 2.0  = %#x\n",
							dimm.tck_l2);
	dev_print("SDRAM access time tAC CAS latency of 2.0 = %#x\n",
							dimm.tac_l2);
	dev_print("MIN row precharge time, tRP = %#x\n", dimm.trp);
	dev_print("MIN row active to row active, tRRD = %#x\n", dimm.trrd);
	dev_print("MIN RAS# to CAS# delay, tRCD = %#x\n", dimm.trcd);
	dev_print("MIN RAS# pulse width, tRAS = %#x\n", dimm.tras);
	dev_print("Module rank density = %#x\n", dimm.rank_den);
	dev_print("Address and command setup time, tISb = %#x\n", dimm.tisb);
	dev_print("Address and command hold time, tIHb  = %#x\n", dimm.tihb);
	dev_print("Data/data mask input setup time, tDSb = %#x\n", dimm.tdsb);
	dev_print("Data/data mask input hold time, tDHb  = %#x\n", dimm.tdhb);
	dev_print("Write recovery time, tWR = %#x\n", dimm.twr);
	dev_print("WRITE-to-READ command delay, tWTR = %#x\n", dimm.twtr);
	dev_print("READ-to_PRECHARGE command delay, tRTP = %x\n", dimm.trtp);
	dev_print("Memory analysis probe = %#x\n", dimm.an_probe);
	dev_print("Extension for tRC and tRFC = %x\n", dimm.x_trc_trfc);
	dev_print("MIN active auto refresh time, tRC = %#x\n", dimm.trc);
	dev_print("MIN AUTO-REFRESH to ACTIVE command period, tRFC = %#x\n",
							dimm.trfc);
	dev_print("SDRAM device MAX cycle time, tCKMAX = %#x\n", dimm.tckmax);
	dev_print("SDRAM device MAX DQS-DQ skew time, tDQSA = %#x\n",
							dimm.tdqsq);
	dev_print("SDRAM device MAX read data hold skew factor, tQHS = %#x\n",
							dimm.tqhs);
	dev_print("PLL relock time = %#x\n", dimm.pll_relock);
	dev_print("SPD revision = %#x\n", dimm.spd_rev);
	dev_print("Checksum for bytes 0 - 62 = %#x\n", dimm.chksum);

	dev_print("Manufacturer's JEDEC ID code = ");
	for (i = 0; i < sizeof(dimm.jedec_id); i++) {
	    dev_print("%#x ", dimm.jedec_id[i]);
	}
	dev_print("\n");

	dev_print("Manufacturing location = %#x\n", dimm.mfg_loc);

	/* copy the Part number to string buffer */
	strncpy((char *)&buf, (char *)&dimm.part_no, sizeof(dimm.part_no));
	buf[sizeof(dimm.part_no)] = '\0';	/* terminate the string */
	dev_print("Module part number : %s\n", buf);

	dev_print("PCB identification code = %#x %#x\n",
					dimm.pcb_id[0], dimm.pcb_id[1]);
	dev_print("Year of manufacture: %#x\n", dimm.mfg_yr);
	dev_print("Week of manufacture: %#x\n", dimm.mfg_wk);

	/* Get the serial number in ASCII */
	strncpy((char *)&buf, (char *)&dimm.serial_no, sizeof(dimm.serial_no));
	buf[sizeof(dimm.serial_no)] = '\0';	/* terminate the string */

	/* Get the serial number in Hex */
	for (i = 0; i < sizeof(ser_no); i++) {
	    ser_no <<= 8;
	    ser_no |= (uint32_t)(dimm.serial_no[i]);
	}

	dev_print("Module serial number: %#x\n", ser_no);
	dev_print("Manufacturer-specific data = ");
	for (i = 0; i < sizeof(dimm.mfg_sp); i++) {
	    if ((i & 0xF) == 0) {
		/* Newline for every 16 bytes */
		dev_print("\n");
	    }
	    dev_print("%#x ", dimm.mfg_sp[i]);
	}

	dev_print("\n");


	break;
    case DEV_SHOW_BRIEF:
	for (i = 0, dimm_ch_p = (uchar *)&dimm; i < sizeof(dimm_i2c_t);
						i++, dimm_ch_p++) {
	    if ((i & 0xf) == 0) {
		dev_print("\n 0x%08x    -  ", i);
	    }
	    dev_print("%02x ", *dimm_ch_p);
	}

	break;
    default:
	assert(!"dimm_dev_show");
	break;
    }

    return(PASSED);
}

/*****************************************************************
 * Name: dimm_crsh
 *
 * Description:	Allow platforms to collect data from a device during a crash.
 *		Print data to the crash log (via the provide print error) using
 *		the appropriate verbisity level requested by the host
 *
 * Input: dev_object_t pointer to the SPD DIMM device
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
dimm_crsh(dev_object_t *dev, print_fn_t dev_print, dev_show_cmd verbosity)
{

    /* more development in this section */
    dev_print("dimm_crsh(): No Crash info available for SPD DIMM\n");
    return(PASSED);
}

/*****************************************************************
 * Name: dimm_destroy
 *
 * Description:	Destroy the dev_object structure and free all the resources.
 *
 * Input: dev_object_t pointer to the SPD DIMM device
 *
 * Returns: none
 *
 * Assumptions: The dev_attch() function has been called and successfully
 *
 *****************************************************************/
static void
dimm_destroy(dev_object_t **dev)
{
    uint32_t rc;
    dev_dimm_object_t *dimm;

    if (dev == NULL) {
	return;
    }

    if (*dev == NULL) {
	return;
    }

    dimm = (dev_dimm_object_t *)*dev;

    /* Unlock the I2C device */
    if ((rc = dimm->callout_fvt->close(dimm->i2c_p)) != PASSED) {
	DEV_ERROR_REPORT(*dev, "dimm_destroy() I2C close", rc);
	return;
    }

    if (dimm->callin_fvt) {
	free(dimm->callin_fvt);		/* Free callin struct */
    }

    if (dimm->callout_fvt) {
	free(dimm->callout_fvt);	/* Free callout struct */
    }

    free(dimm->base.dev_object_fvt);	/* Free dev_object_t */
}

/**********************************************************************
 *
 * Function: dimm_get_memsize
 *
 * This function: Get DIMM size from its SPD interface.
 *
 * Input : dev_object_t pointer to the SPD DIMM device
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
dimm_get_memsize(dev_object_t *dev)
{
    uint32 i, rc;
    uint32_t mem_size;
    uchar rows, cols, ranks, banks, sdram_width;
    dimm_i2c_t dimm;
    uint32_t *dimm_p;
    n2g_i2c_if_t i2c_if;
    dev_dimm_object_t *dimm_obj = (dev_dimm_object_t *)dev;
    dimm_callout_fvt_t *callout_p = dimm_obj->callout_fvt;
    uchar *dimm_ch_p;
    uint8_t checksum;
    char err_buf[80];

    /* Setup the interface struct for I2C API read */
    i2c_if.i2c_bus_type = dimm_obj->i2c_p->i2c_bus_type;
    i2c_if.i2c_dev = dimm_obj->i2c_p->i2c_dev;

    /* Read the bytes from SPD DIMM */
    i2c_if.size = sizeof(uint32_t);	/* Read 4 bytes at a time */
    for (i = 0, dimm_p = (uint32_t *)&dimm; i < sizeof(dimm_i2c_t);
					    i += sizeof(uint32_t), dimm_p++) {
	i2c_if.offset = i;
	i2c_if.buf = (char *)dimm_p;

	rc = (*callout_p->rd)(&i2c_if);
	if (rc != PASSED) {
	    break;      /* Read failed */
	}
    }

    if (rc != PASSED) {
	*dimm_obj->memsize = 0;
	if (rc == E_I2C_INV_ACK) {
	    /* DIMM not installed */
	    return(PASSED);
	}
#ifdef DEBUG_I2C
	sprintf(err_buf, "dimm_get_memsize() read rc = %#.8x offset %#.8x",
						rc, i);
	DEV_ERROR_REPORT(dev, err_buf, DIMM_GET_MEMSIZE);
#endif /* DEBUG_I2C */
	return(FAILED);
    }

    /* Check checksum */
    for (i = 0, checksum = 0, dimm_ch_p = (uchar *)&dimm; i <= 62;
							  i++, dimm_ch_p++) {
	checksum += *dimm_ch_p;
    }

    if (checksum != dimm.chksum) {
	sprintf(err_buf, "DIMM SPD Checksum failed. Expect %#x, Got %#x\n",
					checksum, dimm.chksum);
	DEV_ERROR_REPORT(dev, err_buf, DIMM_GET_MEMSIZE);
	return(FAILED);
    }

    rows = dimm.row_addr;
    cols = dimm.column_addr;
    ranks = dimm.height;
    banks = dimm.banks;
    sdram_width = dimm.dev_width;

    /* Check if rows + columns are more than 1MB */
    if ((rows + cols) <= DIMM_MIN) {
	DEV_ERROR_REPORT(dev, "dimm_get_memsize() Not enough memory.",
					DIMM_GET_MEMSIZE);
	return(FAILED);
    }

    /* 2**(rows + columns) * banks * sdram_width * ((ranks & 0x7) + 1) */

    mem_size = 0x1 << ((rows + cols) - DIMM_MIN);
    mem_size *= (uint32_t)banks;
    mem_size *= (uint32_t)sdram_width;
    mem_size *= (uint32_t)((ranks & DIMM_RANKS_MASK) + 1);

    *dimm_obj->memsize = mem_size;

    return(PASSED);
}

/******** History ******** 
$Log: dev_dimm.c,v $
Revision 1.4  2013/11/26 08:40:31  hroni
fix compiler warning

Revision 1.3  2012/03/28 00:38:07  mcharon
remove forward slash from second line

Revision 1.2  2012/03/25 03:57:53  palin2
Clean up Overlord Intel side (x86) compile warnings.

Revision 1.1.1.1  2012/03/23 23:02:01  ptong
Initial archive of ng_diag module


$Endlog$
*/
