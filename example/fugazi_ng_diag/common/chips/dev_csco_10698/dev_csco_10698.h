/* $Id: dev_csco_10698.h,v 1.2 2012/03/28 00:38:07 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_csco_10698/dev_csco_10698.h,v $
 *------------------------------------------------------------------
 * Filename:	dev_csco_10698.h
 *
 * Description: Cisco part number 15-10698-01 (Renesas R5F21262SNFP) is used
 *		by the Xformers Environmental Control Unit and the Xformers
 *		Power Sequence. This file is derived from EDCS-534569 and
 *		EDCS-618748.
 *
 * Copyright (c) 2007-2012 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DEV_CSCO_10698_H__
#define __DEV_CSCO_10698_H__

#include "types.h"
#include "i2c_api.h"
#include "common_utils.h"
#include "dev_object.h"

/* Common defines */
#define ERR_BUF_SIZE		80
#define REN_I2C_PROC_TIME	3	/* 800 microseconds. round up to 1ms */

typedef uint16_t ren_t;		/* Renesas registers size */
typedef uint8_t ren_o;		/* Renesas registers offset size */

/*
 * dev_error_report message codes
 */
typedef enum {
    REN_DEV_STATE = 0,
    REN_ATTACH,
    REN_DETACH,
    REN_SHOW,
    REN_INIT,
    REN_DESTROY,
    REN_I2C_READ,
    REN_I2C_WRITE,
} csco_10698_report_code_t;

/*
 * device callin function - service provided and defined by the device
 */
typedef struct ren_callin_fvt_t_ {
} ren_callin_fvt_t;

/*
 * device callout function - service needed by the device and defined by
 *			     platform
 */
typedef struct ren_callout_fvt_t_ {
    /*
     * Vectors set by the upper level (eg., platform).
     */
    uint32_t (*open)(n2g_i2c_if_t *);
    uint32_t (*close)(n2g_i2c_if_t *);
    uint32_t (*rd)(n2g_i2c_if_t *);
    uint32_t (*wr)(n2g_i2c_if_t *);
} ren_callout_fvt_t;

/*
 * Registers initialization struct. First register is ReadOnly register.
 * If the offset is 0 (first register), then end of the init table.
 */
typedef struct dev_ren_reg_init_t_ {
    uint8_t	offset;	/* Register offset */
    ren_t	data;	/* Init value */
} dev_ren_reg_init_t;

/*
 * Define the CSCO 10698 device object structure
 */
typedef struct dev_ren_object_t {
    dev_object_t	base;
    ren_callin_fvt_t	*callin_fvt;
    ren_callout_fvt_t	*callout_fvt;
    n2g_i2c_if_t	*i2c_p;		/* I2C API interace pointer */
    reg_info_t		*reg_p;		/* Registers table pointer */
    dev_ren_reg_init_t	*init_p;	/* Register init table pointer */
    uchar		*dev_name;	/* Device function name */
} dev_ren_object_t;

/* Functions prototype */
extern void csco_10698_dev_create(dev_object_t *, dev_error_report_t);


#endif /* __DEV_CSCO_10698_H__ */

/*------------------------------------------------------------------
$Log: dev_csco_10698.h,v $
Revision 1.2  2012/03/28 00:38:07  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:01  ptong
Initial archive of ng_diag module


$Endlog$
*/
