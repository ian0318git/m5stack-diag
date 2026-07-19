/* $Id: dev_pca9557.h,v 1.2 2018/01/20 04:21:32 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_pca9557/dev_pca9557.h,v $
 * Filename:	dev_pca9557.h
 *
 * Description:	8-bit GPIO Expander PCA9557 Header
 *
 * Copyright (c) 2018 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DEV_PCA9557_H__
#define __DEV_PCA9557_H__

#include "types.h"
#include "common_utils.h"
#include "dev_object.h"

/* Common defines */
#define PCA9557_ERR_BUF_SIZE            (80)

typedef enum {
    PCA9557_DEV_STATE = 0,
    PCA9557_ATTACH,
    PCA9557_DETACH,
    PCA9557_INIT,
    PCA9557_SHOW,
    PCA9557_DESTROY,
    PCA9557_ALTER,
    PCA9557_ALERT,
    PCA9557_DISPLAY,
    PCA9557_REG_TEST,
    PCA9557_READ,
    PCA9557_WRITE,
} pca9557_report_code_t;

typedef enum {
    INPUT_PORT,
    OUTPUT_PORT,
    POLARITY_INV_PORT,
    CONFIG_PORT,
} pca9557_register_offset_t;

typedef enum {
    PORT_0,
    PORT_1
} pca9557_port_num;

typedef enum {
    PORT_DIR_INPUT,
    PORT_DIR_OUTPUT
} pca9557_port_direction;

typedef enum {
    PORT_VAL_LOW,
    PORT_VAL_HIGH
} pca9557_port_value;

typedef struct pca9557_callin_fvt_t_ {
    int (*register_test)(dev_object_t *dev);            /* Register Test */
    int (*alter_register)(dev_object_t *dev);    /* Alter register */
    int (*dump_register)(dev_object_t *dev);    /* Dump register */
    int (*config_port)(dev_object_t *dev, int, int, int); /* Configure port direction */
    int (*drive_port)(dev_object_t *dev, int, int, int); /* Drive high/low on port */
    int (*read_port)(dev_object_t *dev, int, int, int *); /* Read the current port value */
} pca9557_callin_fvt_t;

/*
 * device callout function - service needed by the device and defined by
 *			     platform
 */
typedef struct pca9557_callout_fvt_t_ {
    /*
     * Vectors set by the upper level (eg., platform).
     */
    uint32_t (*rd)(uint32, char*);
    uint32_t (*wr)(uint32, char*);
} pca9557_callout_fvt_t;

/*
 * Define the PCA9557 device object structure
 */
typedef struct pca9557_object_t {
    dev_object_t	    base;
    pca9557_callin_fvt_t	    *callin_fvt;
    pca9557_callout_fvt_t	*callout_fvt;
    reg_info_t		    *init_p;	/* Initialization table pointer */
    int i2c_addr;
} dev_pca9557_object_t;

extern  void pca9557_dev_create(dev_object_t *, dev_error_report_t);

#endif
