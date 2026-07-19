/* $Id: dev_cy7c64215.h,v 1.2 2012/03/28 00:38:07 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_cy7c64215_cypress/dev_cy7c64215.h,v $
 *------------------------------------------------------------------
 * Filename:	dev_cy7c64215.h
 *
 * Description: Structs and defines used by CY7C64215 common device driver.
 *		Refer to the vendor datasheet for more info.
 *
 * Copyright (c) 2007-2012 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DEV_CY7C64215_H__
#define __DEV_CY7C64215_H__

#include "types.h"
#include "i2c_api.h"
#include "common_utils.h"
#include "dev_object.h"

/* Common defines */
#define ERR_BUF_SIZE	80

typedef uint8_t usb_t;		/* USB Controller data byte */
typedef uint16_t usb_o;		/* USB Controller address */

/*
 * Cypress CY7C64215 download format
 */
typedef struct cy7c64215_cmd_t_ {
    uint8_t size;	/* Number of bytes to xfer in the command */
    uchar *ptr;		/* Points to command data */
} cy7c64215_cmd_t;

#define CY7C64215_REG_SIZE	10	/* Number of bytes in register access */

/*
 * Cypress CY7C64215 registers access struct
 */
typedef struct cy7c64215_reg_t_ {
    usb_t fw_version_major;	/* major.minor */
    usb_t fw_version_minor;
    usb_t host_baud_rate_ms_byte; /* 99 if not set by PC */
    usb_t host_baud_rate_ls_byte; /* 99 if not set by PC */
    usb_t chip_vendor_id;	/* 0x1 = Cypress */
    usb_t chip_component_id;	/* 0x1 = 64215 (part number) */
    usb_t chip_info_die_ms_byte; /* 0x0 */
    usb_t chip_info_die_ls_byte; /* 0x0 */
    usb_t bootloader_version_major; /* major.minor */
    usb_t bootloader_version_minor;
    usb_t sec_counter;
    usb_t min_counter;
} cy7c64215_reg_t;

/*
 * dev_error_report message codes
 */
typedef enum {
    CY7C64215_DEV_STATE = 0,
    CY7C64215_ATTACH,
    CY7C64215_DETACH,
    CY7C64215_SHOW,
    CY7C64215_INIT,
    CY7C64215_DESTROY,
} cy7c64215_report_code_t;

/*
 * device callin function - service provided and defined by the device
 */
typedef struct cy7c64215_callin_fvt_t_ {
} cy7c64215_callin_fvt_t;

/*
 * device callout function - service needed by the device and defined by
 *			     platform
 */
typedef struct cy7c64215_callout_fvt_t_ {
    /*
     * Vectors set by the upper level (eg., platform).
     */
    uint32_t (*open)(n2g_i2c_if_t *);
    uint32_t (*close)(n2g_i2c_if_t *);
    uint32_t (*rd)(n2g_i2c_if_t *);
    uint32_t (*wr)(n2g_i2c_if_t *);
} cy7c64215_callout_fvt_t;

/*
 * Define the CY7C64215 device object structure
 */
typedef struct dev_cy7c64215_object_t {
    dev_object_t	base;
    cy7c64215_callin_fvt_t  *callin_fvt;
    cy7c64215_callout_fvt_t *callout_fvt;
    n2g_i2c_if_t	*i2c_p;		/* I2C API interace pointer */
    cy7c64215_cmd_t	*init_p;	/* Pointer to download struct */
} dev_cy7c64215_object_t;

/*
 * CY7C64215 Defines
 */

/*	Commands */
#define CY7C64215_CMD_ENTER_B	0xFF38	/* Enter Bootloader */
#define CY7C64215_CMD_WR_BLK	0xFF39	/* Write Block */
#define CY7C64215_CMD_EXIT_B	0xFF3B	/* Exit Bootloader */

/*	Status */
#define CY7C64215_STAT_SUCCESS	0x20	/* Bootload mode (Success) */
#define CY7C64215_STAT_VER_ERR	0x02	/* Image verify error */
#define CY7C64215_STAT_CKS_ERR	0x04	/* Flash checksum error */
#define CY7C64215_STAT_PRT_ERR	0x08	/* Flash protection error */
#define CY7C64215_STAT_CCK_ERR	0x10	/* Comm checksum error */
#define CY7C64215_STAT_INV_KEY	0x40	/* Invalid bootloader key */
#define CY7C64215_STAT_INV_CMD	0x80	/* Invalid command error */

#define CY7C64215_I2C_PREFIX_SHIFT	8	/* I2C prefix shift count */
#define CY7C64215_CMD_DELAY	70	/* 80 ms delay after command is sent */
#define CY7C64215_RESET_DELAY	150	/* 100 ms minimum for V1.27 */

/* Functions prototype */
extern void dev_cy7c64215_create(dev_object_t *, dev_error_report_t);


#endif /* __DEV_CY7C64215_H__ */

/*------------------------------------------------------------------
$Log: dev_cy7c64215.h,v $
Revision 1.2  2012/03/28 00:38:07  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:01  ptong
Initial archive of ng_diag module


$Endlog$
*/
