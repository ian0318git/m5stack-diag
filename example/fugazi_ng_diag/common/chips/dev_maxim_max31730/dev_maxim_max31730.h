/* $Id: dev_maxim_max31730.h,v 1.2 2019/01/10 06:23:18 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_maxim_max31730/dev_maxim_max31730.h,v $
 *------------------------------------------------------------------
 * Filename   : dev_maxim_max31730.h
 * Description: Header file of Maxim Max31730, a 3-Channel Remote 
 *              Temperature Sensor.
 *
 * Copyright (c) 2018 - 2019 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------------------
 */

#ifndef __DEV_MAX31730_MAXIM_H__
#define __DEV_MAX31730_MAXIM_H__

#include "types.h"
#include "i2c_api.h"
#include "common_utils.h"
#include "dev_object.h"

/* Common defines */
#define SEC_TO_MICROSEC             (1000000)
#define MAX31730_MAX_POLLING_USEC   (1000000)   /* 1sec = 1000000 usec */
#define MAX31730_ERR_BUF_SIZE       (80)

typedef enum {
    MAX31730_DEV_STATE = 0,
    MAX31730_ATTACH,
    MAX31730_DETACH,
    MAX31730_INIT,
    MAX31730_GET_REG,
    MAX31730_DESTROY,
    MAX31730_ALTER,
    MAX31730_ALERT,
    MAX31730_DISPLAY,
    MAX31730_REG_TEST,
    MAX31730_SHOW_TEMP,
    MAX31730_I2C_RD,
    MAX31730_I2C_WR,
    MAX31730_INTR_CONFIRM,
} max31730_report_code_t;

/* MAXIM MAX31730 temperature sensors type definition.
 * MAX31730 totally has 4 temperature sensors:
 * Its own(Local) and 3 external(Remote1 ~ Remote3).
 */
typedef enum {
    MAX31730_LOCAL = 0,
    MAX31730_REMOTE1,
    MAX31730_REMOTE2,
    MAX31730_REMOTE3,
} max31730_temp_sensor_t;

typedef enum {
    MAX31730_REG_ONE_BYTE_ACCESS = 1,
    MAX31730_REG_TWO_BYTE_ACCESS,
} max31730_reg_access_size_t;

typedef enum {
    MAX31730_INTR_NOT_TRIG = 0,
    MAX31730_INTR_IS_TRIG,
} max31730_intr_status_t;

/* MAXIM MAX31730 register Read/Write flag */
#define MAX31730_REG_RO_FLAG   (READ_ONLY | REG_ACCESS)
#define MAX31730_REG_RW_FLAG   (READ_WRITE| SAVE_RESTORE | REG_ACCESS)

typedef enum {
    MAX31730_LOC_TEMP_MSB = 0x0,      /* 00h */
    MAX31730_LOC_TEMP_LSB,            /* 01h */
    MAX31730_R1_TEMP_MSB,             /* 02h */
    MAX31730_R1_TEMP_LSB,             /* 03h */
    MAX31730_R2_TEMP_MSB,             /* 04h */
    MAX31730_R2_TEMP_LSB,             /* 05h */
    MAX31730_R3_TEMP_MSB,             /* 06h */
    MAX31730_R3_TEMP_LSB,             /* 07h */
    MAX31730_HST_TEMP_MSB = 0x10,     /* 10h */
    MAX31730_HST_TEMP_LSB,            /* 11h */
    MAX31730_HST_TEMP_EN,             /* 12h */
    MAX31730_CONFIG,                  /* 13h */
    MAX31730_CUST_IDEALITY_FACTOR,    /* 14h */
    MAX31730_CUST_IDEALITY_EN,        /* 15h */
    MAX31730_CUST_OFFST,              /* 16h */
    MAX31730_CUST_OFFST_EN,           /* 17h */
    MAX31730_FILTER_EN,               /* 18h */
    MAX31730_BETA_COMP_EN,            /* 19h */
    MAX31730_BETA_VAL_CH1,            /* 1Ah */
    MAX31730_BETA_VAL_CH2,            /* 1Bh */
    MAX31730_BETA_VAL_CH3,            /* 1Ch */
    MAX31730_LOC_HLIM_MSB = 0x20,     /* 20h */
    MAX31730_LOC_HLIM_LSB,            /* 21h */
    MAX31730_R1_HLIM_MSB,             /* 22h */
    MAX31730_R1_HLIM_LSB,             /* 23h */
    MAX31730_R2_HLIM_MSB,             /* 24h */
    MAX31730_R2_HLIM_LSB,             /* 25h */
    MAX31730_R3_HLIM_MSB,             /* 26h */
    MAX31730_R3_HLIM_LSB,             /* 27h */
    MAX31730_ALL_LLIM_MSB = 0x30,     /* 30h */
    MAX31730_ALL_LLIM_LSB,            /* 31h */
    MAX31730_THERM_STATUS_H,          /* 32h */ 
    MAX31730_THERM_STATUS_L,          /* 33h */
    MAX31730_THERM_MASK,              /* 34h */
    MAX31730_TEMP_CH_EN,              /* 35h */
    MAX31730_DIO_FAULT_STATUS,        /* 36h */
    MAX31730_LOC_REF_TEMP_MSB = 0x40, /* 40h */
    MAX31730_LOC_REF_TEMP_LSB,        /* 41h */
    MAX31730_R1_REF_TEMP_MSB,         /* 42h */
    MAX31730_R1_REF_TEMP_LSB,         /* 43h */
    MAX31730_R2_REF_TEMP_MSB,         /* 44h */
    MAX31730_R2_REF_TEMP_LSB,         /* 45h */
    MAX31730_R3_REF_TEMP_MSB,         /* 46h */
    MAX31730_R3_REF_TEMP_LSB,         /* 47h */
    MAX31730_MFR_ID = 0x50,           /* 50h */
    MAX31730_REV,                     /* 51h */
} max31730_reg_t;

/* Temperature Reg.(0x0) */
#define MAX31730_TEMP_SIGN_BIT     (1 << 15)   /* D15(bit7 of MSByte) */
#define MAX31730_VALID_TEMP_MASK   (0xFFF0)    /* Valid data: D15 ~ D4 */
#define MAX31730_VALID_TEMP_SHIFT  (4)
#define MAX31730_TEMP_UNIT         (0.0625)    /* One LSB: 0.0625 degree Celcius */

/* THERM Mask Reg.(0x34) */
#define MAX31730_MASK_ALL          (0xF)

typedef struct max31730_callin_fvt_t_ {
    int (*register_test)(dev_object_t *);        /* Register Test */
    int (*interrupt_test)(dev_object_t *, int);  /* Interrupt Test */
    int (*show_register)(dev_object_t *);        /* Get register */
    int (*alter_register)(dev_object_t *);       /* Alter register */
    int (*dump_register)(dev_object_t *);        /* Dump register */
    int (*show_temp)(dev_object_t *);            /* Show Temperature */
} max31730_callin_fvt_t;

/*
 * MAXIM MAX31730 device callout function:
 * Service needed by the device, need to be defined by platform.
 */
typedef struct max31730_callout_fvt_t_ {
    /*
     * Vectors set by the upper level (eg., platform).
     */
    uint32_t (*open)(n2g_i2c_if_t *);
    uint32_t (*close)(n2g_i2c_if_t *);
    uint32_t (*rd)(n2g_i2c_if_t *);
    uint32_t (*wr)(n2g_i2c_if_t *);
    uint32_t (*intr_confirm)(boolean);
} max31730_callout_fvt_t;

/*
 * MAXIM MAX31730 device object structure definition
 */
typedef struct max31730_object_t {
    dev_object_t             base;
    max31730_callin_fvt_t    *callin_fvt;
    max31730_callout_fvt_t   *callout_fvt;
    n2g_i2c_if_t             *i2c_p; /* I2C API interace pointer */
    reg_info_t               *reg_p; /* Register table pointer */
} dev_max31730_object_t;

/*
 * MAXIM MAX31730 interrupt test object structure definition
 */
typedef struct intr_test_obj {
    char           *intr_name;
    reg_info_t_ext *ext;                 /* For register access */
    char           *therm_thr_name;
    unsigned int   therm_thr_offset;     /* Thermal threshold reg. offset */
    unsigned int   test_thr_val;         /* Threshold value for test */
    unsigned int   reset_thr_val;        /* Reset threshold value */
    char           *therm_status_name;
    unsigned int   therm_status_offset;  /* Thermal status reg. offset */
    unsigned int   test_status_val;      /* Thermal status value for test */
    unsigned int   reset_status_val;     /* Reset thermal status reg. value */
} intr_test_obj_t;

/* Externs */
extern void max31730_dev_create(dev_object_t *, dev_error_report_t);

#endif   /* __DEV_MAXIM_MAX31730_H__ */

/*-------------------------------------------------
 * $Log: dev_maxim_max31730.h,v $
 * Revision 1.2  2019/01/10 06:23:18  wilbhuan
 * The beginning of Maxim Integrated MAX31730 Temperature Sensor device driver.
 *
 *-------------------------------------------------
 */
