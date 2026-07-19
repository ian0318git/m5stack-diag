/* $Id: cmd_rom_ugd.h,v 1.1 2017/10/19 14:04:29 palin2 Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/cmd_rom_ugd.h,v $ 
 *-------------------------------------------------------------------------
 * 
 * Filename   : cmd_rom_ugd.h
 * Description: Header file of TSN CLI command, rom-ugd, to upgrade ROMMON.
 *
 * Copyright (c) 2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *--------------------------------------------------------------------------
 */

#ifndef __CMD_ROM_UGD_H__
#define __CMD_ROM_UGD_H__

/* Common */
#define TSN_BF_MTD_NUM   3
#define MTD_NAME_SZ      16 
#define IMG_SZ_8MB       (8 * 1024 * 1024)   /* 8MB */
#define ONE_MB           (1024.0 * 1024.0)

/* Externs */
extern void rom_ugd_usage(void);
extern int  rom_ugd_util(char *);

#endif   /* __CMD_ROM_UGD_H__ */


/*-------------------------------------------------
 * $Log: cmd_rom_ugd.h,v $
 * Revision 1.1  2017/10/19 14:04:29  palin2
 * Added support to upgrade ROMMON to TSN Oct-2017 Pilot version, 16.6(1r).
 *
 * $Endlog$
 *-------------------------------------------------
 */
