/* $Id: eeprom_map.h,v 1.2 2017/07/28 07:58:36 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/image/eeprom_map.h,v $
 *------------------------------------------------------------------
 * eeprom_map.h
 *
 * Oct 2016, Smita Rane
 *
 * Copyright (c) 2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

/*------------------------------------------------------------------
 * eeprom_map.h - sp27xx EEPROM map usage
 *
 * Aug. 2012 Da Lin
 *
 * Copyright (c) 2012 by cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

#ifndef _EEPROM_MAP_H
#define _EEPROM_MAP_H

#define SPI_GOLDEN_OFFSET       0x104
#define SPI_JUMP_END            0xC4
#define SPI_VERSION_OFFSET      0xE0    /* spi mapping revision */

#define SPI_PRI_KEY_OFFSET      0x20000  /* primary key */
#define SPI_CONFIG_OFFSET       0x30000  /* environment config */
#define SPI_BACKUP_KEY_OFFSET   0x40000  /* primary key */
#define SPI_UPGRADE_OFFSET      0x50000  /* upgrade bootloader */

#define SPI_BLOCK_SIZE          0x10000 /* block size of eeprom */

/* SDB only has 2Mbits of eeprom. Need to remap it */
#define SDB_CFG_REMAP_OFFSET    0x10000

/* upgrade bldr shares same block with public key*/
#define SDB_UPGRADE_OFFSET      0x22000

#define SDB_UPGRADE_KEY_DIFF    (SDB_UPGRADE_OFFSET - SPI_PRI_KEY_OFFSET)

#endif
/*
 * $Log: eeprom_map.h,v $
 * Revision 1.2  2017/07/28 07:58:36  harrchan
 * Collapse Oakenshield-branch to Main Trunk.
 *
 * Revision 1.1.2.1  2017/06/29 08:14:31  harrchan
 * Initial commit code for Oakenshield
 *
 * Revision 1.1.4.2  2016/12/14 04:49:36  olin2
 * Initial commit code for Oakenshield
 *
 * Revision 1.1  2016/10/07 21:51:50  srane
 * CSCvb61570 - Move to SWIMS server for code signing
 *
 *------------------------------------------------------------------
 * $Endlog$
 */

