/* $Id: dev_pca9545.h,v 1.2 2012/03/28 00:38:08 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_pca9545a_ti/dev_pca9545.h,v $
 *------------------------------------------------------------------
 * Filename:	dev_pca9545.h
 *
 * Description: Structs and defines used by TI PCA9545A common device driver.
 *		Refer to the vendor datasheet for more info.
 *
 * Copyright (c) 2008-2012 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DEV_PCA9545_H__
#define __DEV_PCA9545_H__

#include "types.h"
#include "i2c_api.h"
#include "common_utils.h"
#include "dev_object.h"

/* Common defines */
#define ERR_BUF_SIZE	80

typedef uint8_t pca_t;		/* PCA9545A control byte */

/*
 * Device display struct
 */
typedef struct dev_pca9545_desc_t_ {
    char *name;		/* Text of field */
    char *true;		/* Text if bit(s) mask is non-zero */
    char *false;	/* Text if bit(s) mask is zero */
    pca_t mask;		/* Bit(s) Mask for the field */
} dev_pca_desc_t;

/*
 * dev_error_report message codes
 */
typedef enum {
    PCA_DEV_STATE = 0,
    PCA_ATTACH,
    PCA_DETACH,
    PCA_SHOW,
    PCA_INIT,
    PCA_DESTROY,
    PCA_ALTER,
    PCA_TEST,
} pca9545_report_code_t;

/*
 * device callin function - service provided and defined by the device
 */
typedef struct pca_callin_fvt_t_ {
    int (*peek_n_poke)(dev_object_t *dev, print_fn_t);  /* Peek-n-poke */
    int (*pca_test)(dev_object_t *dev);
} pca_callin_fvt_t;

/*
 * device callout function - service needed by the device and defined by
 *			     platform
 */
typedef struct pca_callout_fvt_t_ {
    /*
     * Vectors set by the upper level (eg., platform).
     */
    uint32_t (*open)(n2g_i2c_if_t *);
    uint32_t (*close)(n2g_i2c_if_t *);
    uint32_t (*rd)(n2g_i2c_if_t *);
    uint32_t (*wr)(n2g_i2c_if_t *);
} pca_callout_fvt_t;

/*
 * Define the PCA9545A device object structure
 */
typedef struct dev_pca_object_t {
    dev_object_t	base;
    pca_callin_fvt_t	*callin_fvt;
    pca_callout_fvt_t	*callout_fvt;
    n2g_i2c_if_t	*i2c_p;		/* I2C API interace pointer */
    dev_pca_desc_t	*desc_p;	/* Descriptor for show */
    uchar		*dev_name;	/* Device function name */
    pca_t		init;		/* Init control register value */
} dev_pca_object_t;

/*
 * PCA9545A Control register Defines
 */
#define PCA9545_INT3	0x80	/* INT3# */
#define PCA9545_INT2	0x40	/* INT2# */
#define PCA9545_INT1	0x20	/* INT1# */
#define PCA9545_INT0	0x10	/* INT0# */

#define PCA9545_B3	0x08	/* B3 */
#define PCA9545_B2	0x04	/* B2 */
#define PCA9545_B1	0x02	/* B1 */
#define PCA9545_B0	0x01	/* B0 */

#define PCA9545_CTRL_MIN	0
#define PCA9545_CTRL_MAX	0xF

/* Functions prototype */
extern void dev_pca9545_create (dev_object_t *dev,
			    dev_error_report_t error_report_fn);

#endif /* __DEV_PCA9545_H__ */

/*------------------------------------------------------------------
$Log: dev_pca9545.h,v $
Revision 1.2  2012/03/28 00:38:08  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:02  ptong
Initial archive of ng_diag module


$Endlog$
*/
