/* $Id: patriot_framer_spi.h,v 1.1 2014/03/25 02:12:43 huanngo Exp $
 * $Source: 
 *******************************************************************************
 * File Name: patriot_framer_spi.h
 *
 * Description: FRAMER SPI DRIVER
 *
 *
 * Author: Sofian Teja, port from IOS
 * Copyright (c)2011 - 2014 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *******************************************************************************
 */

#ifndef __PATRIOT_FRAMER_SPI_H__
#define __PATRIOT_FRAMER_SPI_H__
/*
 * Patriot Framer SPI
 */

/* Functions */
int patriot_framer_spi_init(void);
void patriot_framer_spi_exit(void);
int patriot_framer_write(unsigned short wraddr, unsigned char wrdata);
int patriot_framer_read(unsigned short rxaddr, unsigned char *rxdata);
int patriot_framer_interrupt_handler(void);
int patriot_framer_interrupt_disabled(void);
int patriot_framer_id(void);

/* Variables */
extern int patriot_framer_spi_debug;
extern void patriot_framer_count(void);

#endif /* __PATRIOT_FRAMER_SPI_H__ */

/******** History ********/
/*------------------------------------------------------------------------------
 * $Log: patriot_framer_spi.h,v $
 * Revision 1.1  2014/03/25 02:12:43  huanngo
 * Adding patriot_linux directory to ng_diag code tree
 *
 * Revision 1.3  2014/03/06 01:56:52  steja
 * 1. added cli command for margining patriot voltage
 * 2. enhance framer interrupt and ecc memory test timing
 *
 * Revision 1.2  2012/05/08 23:52:56  huanngo
 * Support SM Patriot on ngd main code tree
 *
 * Revision 1.1.2.1  2012/03/13 13:31:33  steja
 * Support Framer Interrupt
 *
 *
 *------------------------------------------------------------------------------
 * $Endlog$
 *
 *------------------------------------------------------------------------------
 */
