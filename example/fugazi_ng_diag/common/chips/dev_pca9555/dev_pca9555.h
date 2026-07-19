/* $Id: dev_pca9555.h,v 1.3 2018/03/27 12:46:36 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_pca9555/dev_pca9555.h,v $
 *------------------------------------------------------------------
 * Filename:	dev_pca9555.h
 *
 * Description:	16-bit GPIO Expander PCA9555 Header
 *
 * Copyright (c) 2018 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DEV_PCA9555_H__
#define __DEV_PCA9555_H__

#include "defs.h"
#include "types.h"
#include "common_utils.h"
#include "dev_object.h"

/* Common defines */
#define PCA9555_ERR_BUF_SIZE            (80)

#define PCA9555_REG_RO_FLAG              (READ_ONLY | REG_ACCESS)
/* No REG_ACCESS here accroding to different reg size */
#define PCA9555_REG_RW_FLAG              (READ_WRITE| SAVE_RESTORE | REG_ACCESS)
typedef enum {
    PCA9555_DEV_STATE = 0,
    PCA9555_ATTACH,
    PCA9555_DETACH,
    PCA9555_INIT,
    PCA9555_SHOW,
    PCA9555_DESTROY,
    PCA9555_ALTER,
    PCA9555_ALERT,
    PCA9555_DISPLAY,
    PCA9555_REG_TEST,
    PCA9555_READ,
    PCA9555_WRITE,
} pca9555_report_code_t;

typedef enum {
    INPUT_PORT_0,
    INPUT_PORT_1,
    OUTPUT_PORT_0,
    OUTPUT_PORT_1,
    POLARITY_INV_PORT_0,
    POLARITY_INV_PORT_1,
    CONFIG_PORT_0,
    CONFIG_PORT_1
} pca9555_register_offset_t;

typedef enum {
    PORT_0,
    PORT_1
} pca9555_port_num;

typedef enum {
    PORT_DIR_INPUT,
    PORT_DIR_OUTPUT
} pca9555_port_direction;

typedef enum {
    PORT_VAL_LOW,
    PORT_VAL_HIGH
} pca9555_port_value;

typedef struct pca9555_callin_fvt_t_ {
    int (*register_test)(dev_object_t *dev);            /* Register Test */
    int (*alter_register)(dev_object_t *dev);    /* Alter register */
    int (*dump_register)(dev_object_t *dev);    /* Dump register */
    int (*config_port)(dev_object_t *dev, int, int, int); /* Configure port direction */
    int (*drive_port)(dev_object_t *dev, int, int, int); /* Drive high/low on port */
    int (*read_port)(dev_object_t *dev, int, int, int *); /* Read the current port value */
} pca9555_callin_fvt_t;

/*
 * device callout function - service needed by the device and defined by
 *			     platform
 */
typedef struct pca9555_callout_fvt_t_ {
    /*
     * Vectors set by the upper level (eg., platform).
     */
    uint32_t (*rd)(uint32, ushort *);
    uint32_t (*wr)(uint32, ushort *);
} pca9555_callout_fvt_t;

/*
 * Define the PCA9555 device object structure
 */
typedef struct pca9555_object_t {
    dev_object_t	    base;
    pca9555_callin_fvt_t	    *callin_fvt;
    pca9555_callout_fvt_t	*callout_fvt;
    reg_info_t		    *init_p;	/* Initialization table pointer */
    int i2c_addr;
} dev_pca9555_object_t;

extern void pca9555_dev_create(dev_object_t *, dev_error_report_t);
extern reg_info_t_ext pca9555_reg_ext;
extern reg_info_t* pca9555_reg_map;

#endif
