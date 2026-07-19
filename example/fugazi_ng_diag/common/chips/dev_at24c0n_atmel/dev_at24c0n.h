/* $Id: dev_at24c0n.h,v 1.3 2013/11/26 08:40:31 hroni Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_at24c0n_atmel/dev_at24c0n.h,v $
 *------------------------------------------------------------------
 * Filename:	dev_at24c0n.h
 *
 * Description: Structs and defines used by AT24C01/02/04 common device driver.
 *		Refer to the vendor datasheet for more info.
 *
 * Copyright (c) 2007-2013 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DEV_AT24C0N_H__
#define __DEV_AT24C0N_H__

#include "types.h"
#include "i2c_api.h"
#include "common_utils.h"
#include "dev_object.h"

/* Common defines */
#define ERR_BUF_SIZE		80
#define AT_PAGE_WRITE_MAX	8	/* I2C EEPROM Page Write limit.
					 * Larger EEPROM has larger limit. We
					 * use the smaller one.
					 */

typedef uint8_t at_t;		/* EEPROM data byte */
typedef uint16_t at_o;		/* EEPROM address */
/* Note - AT24C04 is 4K bit (512 x 8) device. A byte of address is not
 * sufficient. For AT24C01 and AT24C02, only the lower byte is used.
 * For AT24C04, the higher 256 bytes address has another slave
 * device address. Therefore, the upper 256 bytes are considered as another
 * device. The high byte of the address of none zero will be part of the
 * slave address.
 */

/*
 * Device display struct
 */
typedef struct at24c0n_desc_t_ {
    char *name;		/* Text of field */
    at_o offset;	/* Filed starting offset */
    at_o size;		/* Number of bytes in this field */
    uint8_t type;	/* Display type. Refer to at_desc_type enum */
} dev_at24c0n_desc_t;

/*
 * Display descriptor type
 */
typedef enum{
    AT_DESC_HEX = 0,	/* Hex byte */
    AT_DESC_DEC,	/* Decimal */
    AT_DESC_TXT,	/* String */
} at_desc_type;

/*
 * dev_error_report message codes
 */
typedef enum {
    AT_DEV_STATE = 0,
    AT_ATTACH,
    AT_DETACH,
    AT_SHOW,
    AT_INIT,
    AT_DESTROY,
    AT_ALTER,
    AT_MEM_TEST,
    AT_GET_SIZE,
} at24c0n_report_code_t;

/*
 * EEPROM memory test struct
 */
typedef struct at_eeprom_test_t_ {
    at_o	start;		/* Starting offset of test */
    at_o	size;		/* Number of bytes tested */
} at_eeprom_test_t;

/*
 * device callin function - service provided and defined by the device
 */
typedef struct at_callin_fvt_t_ {
    int (*peek_n_poke)(dev_object_t *dev, print_fn_t);  /* Peek-n-poke */
    int (*eeprom_test)(dev_object_t *, print_fn_t, at_eeprom_test_t *);
    int (*eeprom_program)(dev_object_t *, print_fn_t, unsigned char*,
                          unsigned int);
    int (*eeprom_read)(dev_object_t *, print_fn_t, unsigned char*,
                          unsigned int);
							/* EEPROM test */
} at_callin_fvt_t;

/*
 * device callout function - service needed by the device and defined by
 *			     platform
 */
typedef struct at_callout_fvt_t_ {
    /*
     * Vectors set by the upper level (eg., platform).
     */
    uint32_t (*open)(n2g_i2c_if_t *);
    uint32_t (*close)(n2g_i2c_if_t *);
    uint32_t (*rd)(n2g_i2c_if_t *);
    uint32_t (*wr)(n2g_i2c_if_t *);
} at_callout_fvt_t;

/*
 * Define the AT24C0x device object structure
 */
typedef struct dev_at_object_t {
    dev_object_t	base;
    at_callin_fvt_t	*callin_fvt;
    at_callout_fvt_t	*callout_fvt;
    n2g_i2c_if_t	*i2c_p;		/* I2C API interace pointer */
    dev_at24c0n_desc_t	*init_p;	/* Descriptor for show */
    char		*dev_name;	/* Device function name */
    at_t		param;		/* Data pattern for content init */
    uint8_t		dev_type;	/* Refer to at24c0n_dev_type enum */
} dev_at_object_t;

/*
 * EEPROM type
 */
typedef enum {
    AT24C_01 = 0,	/* AT24C01 1K (128 * 8) */
    AT24C_02,		/* AT24C02 2K (256 * 8) */
    AT24C_04,		/* AT24C04 4K (256 * 8) */
    AT24C_INVALID,	/* Invalid type */
} at24c0n_dev_type;

/*
 * AT24C01/02/04 Defines
 */
#define AT24C01_MAX	0x7F	/* AT24C01 - 128 bytes */
#define AT24C02_MAX	0xFF	/* AT24C02 - 256 bytes */
#define AT24C04_MAX	0x1FF	/* AT24C04 - 512 bytes */
#define AT24C0X_T_WR	6	/* tWR = 5 ms. With 20% margin */

/* Functions prototype */
extern void dev_at24c0n_create(dev_object_t *, dev_error_report_t);


#endif /* __DEV_AT24C0N_H__ */

/*------------------------------------------------------------------
$Log: dev_at24c0n.h,v $
Revision 1.3  2013/11/26 08:40:31  hroni
fix compiler warning

Revision 1.2  2012/03/28 00:38:06  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:01  ptong
Initial archive of ng_diag module


$Endlog$
*/
