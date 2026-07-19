/* $Id: p1021_espi.h,v 1.1 2014/03/25 02:12:33 huanngo Exp $
 * $Source: 
 *------------------------------------------------------------------
 *
 * p1021_espi.h:  P1021 eSPI defines
 *
 * May 2011 - Steja
 *
 * Copyright (c) 2011 - 2014 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 *
 *------------------------------------------------------------------
 */

#ifndef __P1021_ESPI_H__
#define __P1021_ESPI_H__

#define ESPI_XFER_TIMEOUT       0x4000 
#define RESET_VALUE             0xFFFFFFFF

/*******************************
 * eSPI Mode Register
 *******************************/
#define ESPI_MODE_EN            0x80000000
#define ESPI_LOOP_MODE          0x40000000
#define ESPI_OD_MODE            0x20000000

/*******************************
 * eSPI ModeX Register
 *******************************/
#define ESPI_MODEX_CI1          0x80000000
#define ESPI_MODEX_CP1          0x40000000
#define ESPI_MODEX_REV1         0x20000000
#define ESPI_MODEX_DIV16        0x10000000
#define ESPI_MODEX_PM(y)        (((y) & 0xf) << 24)
#define ESPI_MODEX_ODD1         0x00800000
#define ESPI_MODEX_POL1         0x00100000
#define ESPI_MODEX_LEN(y)       (((y) & 0xf) << 16)
#define ESPI_MODEX_CSBEF(y)     (((y) & 0xf) << 12)
#define ESPI_MODEX_CSAFT(y)     (((y) & 0xf) << 8)
#define ESPI_MODEX_CSCG(y)      (((y) & 0xf) << 3)


/*******************************
 * eSPI Event Register
 *******************************/
#define ESPI_EV_TXE             0x00008000 /* Tx fifo is empty */
#define ESPI_EV_DON             0x00004000 /* Last character was transmitted */
#define ESPI_EV_RXF             0x00001000 /* Rx fifo is full */
#define ESPI_EV_RNE             0x00000200 /* Rx fifo is not empty */
#define ESPI_EV_TNF             0x00000100 /* Tx fifo not full */

/*******************************
 * eSPI Command Register
 *******************************/
#define ESPI_COM_CS0            0x00000000
#define ESPI_COM_CS1            0x40000000
#define ESPI_COM_CS2            0x80000000
#define ESPI_COM_CS3            0xC0000000
#define ESPI_COM_RX_DELAY       0x20000000
#define ESPI_COM_DO             0x10000000
#define ESPI_COM_TO             0x08000000
#define ESPI_COM_HLD            0x04000000
#define ESPI_COM_RX_SKIP(y)      (((y) & 0xff) << 16)

typedef enum {
    ESPI_CS0 = 0, /* SPI CS0 */
    ESPI_CS1,     /* SPI CS1 */         
    ESPI_CS2,     /* SPI CS2 */         
    ESPI_CS3,     /* SPI CS3 */
} espi_cs;

typedef enum {
    BLOCK_SIZE1 = 1,
    BLOCK_SIZE2,
    BLOCK_SIZE3,
    BLOCK_SIZE4,
    BLOCK_SIZE5,
    BLOCK_SIZE6,
    BLOCK_SIZE7,
    BLOCK_SIZE8,
} block_size;

extern int espi_init(int, int);
extern void espi_activate(int, int, int, int);
extern void espi_deactivate(int);
extern int espi_xfr(int, uchar *, uchar *, int, int, int);

extern void display_espi_registers(void);

#endif /* __P1021_ESPI_H__ */


/*------------------------------------------------------------------------------
 * $Log: p1021_espi.h,v $
 * Revision 1.1  2014/03/25 02:12:33  huanngo
 * Adding patriot_linux directory to ng_diag code tree
 *
 * Revision 1.4  2012/07/31 00:12:11  huanngo
 * Fix the bug in the module submenu utility to modify SPI PROM data
 *
 * Revision 1.3  2012/07/18 23:47:26  huanngo
 * Support read and write multiple bytes to SPI PROM
 *
 * Revision 1.2  2012/05/08 23:52:55  huanngo
 * Support SM Patriot on ngd main code tree
 *
 * Revision 1.1.4.3  2011/08/26 14:44:56  steja
 * Update p1021 code to display SPI registers
 *
 * Revision 1.1.4.2  2011/08/18 19:43:23  huanngo
 * Update code to patriot2-branch
 *
 * Revision 1.1.2.6  2011/08/16 17:57:59  huanngo
 * Fix bugs for SPI EEPROM
 *
 * Revision 1.1.2.5  2011/08/06 00:17:39  huanngo
 * Update code for Patriot
 *
 * Revision 1.1.2.4  2011/07/14 14:38:27  steja
 * Update Patriot Project Module side code
 *
 * Revision 1.1.2.3  2011/05/26 00:38:11  huanngo
 * Update with SPI PROM access and FPGA, DS3170 reset functions
 * Change the SPI read/write to uchar access
 *
 * Revision 1.1.2.2  2011/05/25 16:05:04  steja
 * Update the DS3170 testing function based on specs
 *
 *------------------------------------------------------------------------------
 * $Endlog$
 *
 *------------------------------------------------------------------------------
 */
