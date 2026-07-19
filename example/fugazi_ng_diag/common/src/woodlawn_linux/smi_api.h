/* $Id: smi_api.h,v 1.2 2013/10/08 08:48:32 tirawan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn_linux/smi_api.h,v $
 *------------------------------------------------------------------
 * Filename: smi_api.h
 *
 * Description: SMI/MDIO API header file.
 *
 * Copyright (c) 2006-2013 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __SMI_API_H__
#define __SMI_API_H__

#ifdef LINUX_APP
#include <unistd.h>
#else /* Diagmon */
#ifndef __N2GI2C_H__
typedef uint32_t	pid_t;	/* usually in <unistd.h> */
#endif /* __N2GI2C_H__ */
#endif /* LINUX_APP */
typedef uint16_t	smi_t;	/* SMI data type */

#ifdef OVERLORD
typedef __pid_t pid_t;
#endif
/* Common defines */

/* SMI API interface struct */
typedef union {
    pid_t	lock_pid;	/* Locked PID */
} smi_rt_if_t;

typedef struct smi_iface {
    smi_rt_if_t ret;		/* Return info struct */
    smi_t	*buf;		/* Read/write buffer pointer */
    uint8_t	offset;		/* SMI device register or memory offset */
    uint8_t	smi_dev;	/* MB_SMI_DEVICE device ID */
    uint8_t	smi_speed;	/* SMI bus speed used during init */
} smi_if_t;

/* Motherboard SMI states */
typedef struct smi_states {
    pid_t pid;			/* Process ID using the device */
    uint8_t	ctrl;		/* SMI controller enum */
    uint8_t	state;		/* SMI state */
    uint8_t	smi_addr;	/* SMI Physical address */
} smi_states_t;

/* SMI Controller States defines */
#define SMI_IDLE		0	/* Idle state */
#define SMI_LOCKED		1	/* Locked state */

#if 0 /* INFORM_MONT */
/* SMI bus speed */
typedef enum {
    SMI_2P5MHZ = 0,		/* 2.5 MHz */
    SMI_3P125MHZ,		/* 3.125 MHz */
    SMI_4P16MHZ,		/* 4.1667 MHz */
    SMI_6P25MHZ,		/* 6.25 MHz */
    SMI_8MHZ,			/* 8.33 MHz */
    SMI_12P5MHZ,		/* 12.5 MHz */
} SMI_SPEED;
#else
/* SMI bus speed */
typedef enum {
    SMI_2P5MHZ = 0,	/* 2.5 MHz */
    SMI_3P125MHZ,	/* 3.125 MHz */
    SMI_4P16MHZ,	/* 4.1667 MHz */
    SMI_6P25MHZ,	/* 6.25 MHz */
    SMI_8MHZ,		/* 8.33 MHz */
    SMI_12P5MHZ,	/* 12.5 MHz */
    SMI_2P35MHZ,	/* 2.357 MHz */
    SMI_2P75MHZ,	/* 2.75 MHz */
    SMI_3P3MHZ,		/* 3.3 MHz */
    SMI_4P125MHZ,	/* 4.125 MHz */
    SMI_5P5MHZ,		/* 5.5 MHz */
    SMI_8P25MHZ,	/* 8.25 MHz */
    SMI_16P5MHZ,	/* 16.5 MHz */
} SMI_SPEED;
#endif

/* SMI operation */
#define	SMI_WRITE		0	/* SMI write command */
#define SMI_READ		1	/* SMI read command */

/* Delays used for SMI */
/*	8 MHz ie 125ns per bit, 32 bits preambles, 2 bits start of frame,
 *	2 bits opcode, 5 bits PHY address, 5 bits offset, 2 bits turn-around,
 *	16 bits data, *	totals 64 bits. 64 * 125 = 8000 ns, or 8 microseconds.
 *	Add 1 extra microseconds for margin (10%).
 */
/* need to increase to 30 microseconds for slower CPU device */
#define SMI_TIMEOUT		30
#define SMI_RESET_DELAY		10	/* SMI controller reset. It can be
					 * the same as SMI_TIMEOUT */

/* SMI API prototypes */
extern uint smi_setup(void);
extern uint32_t smi_reset(smi_if_t *smi_p);
extern uint32_t smi_init(smi_if_t *smi_p);
extern uint32_t smi_open(smi_if_t *smi_p);
extern uint32_t smi_close(smi_if_t *smi_p);
extern uint32_t smi_read(smi_if_t *smi_p);
extern uint32_t smi_write(smi_if_t *smi_p);

extern uint32_t api_mb_smi_reset(smi_states_t *state_p);
extern uint32_t api_mb_smi_init(smi_states_t *state_p, uint8_t phase);
extern uint32_t api_mb_smi_read(smi_states_t *state_p, uint8_t dev_addr,
		uint8_t offset, smi_t *buf);
extern uint32_t api_mb_smi_write(smi_states_t *state_p, uint8_t dev_addr,
		uint8_t offset, smi_t data);

#endif /* __SMI_API_H__ */

/*------------------------------------------------------------------
 * $Log: smi_api.h,v $
 * Revision 1.2  2013/10/08 08:48:32  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.2.1  2013/08/20 10:59:11  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.1  2013/04/24 10:58:02  tirawan
 * First Woodlawn linux integration
 *
 * Revision 1.1  2013/03/13 06:42:43  kuangik
 * Add for the first time
 *
 * Revision 1.4  2012/08/03 10:16:53  evanli
 * Mapping to latest O2 source code on 20120726
 *
 * Revision 1.1.1.1  2012/02/10 05:59:50  kody
 * Initial imports Woodlawn project code base.
 *
 * Revision 1.1.1.1.30.2  2011/04/25 22:00:12  mcharon
 * changes for overlord
 *
 * Revision 1.1.1.1.30.1  2011/03/11 21:39:02  mcharon
 * initial support informers linux
 *
 * Revision 1.1.1.1  2009/10/17 02:05:38  huyhoang
 * Initial archive of diaglinux module
 *
 * Revision 1.8.8.1.2.1  2009/10/16 16:22:05  huyhoang
 * + Fix compilation error on DiagLinux
 *
 * Revision 1.8.8.1  2009/06/04 09:33:20  sctsai
 * Sync with informers2-tag-060209 repository.
 *
 * Revision 1.7.10.3  2009/05/12 22:18:20  shihs
 * Support montalvo SMI interface and SMI test cases.
 *
 * Revision 1.7.10.2  2009/03/16 22:51:39  sctsai
 * Sync with informers-tag-031309-sync repository.
 *
 * Revision 1.7.2.2  2009/03/13 22:37:28  ptong
 * Sync with ngd-informers-031209 repository.
 *
 * Revision 1.7.2.1  2008/11/18 00:29:20  siyen
 * GE MAC/PHY fixes to support NIC card.
 *
 * Revision 1.8  2009/03/10 02:47:11  ksabzwar
 * Sync xformers-tag-030909 to the main ngd diag repository
 *
 * Revision 1.7  2008/05/09 22:11:03  srane
 * sync of xformers-tag-050908 into ngd-diags-rep
 *
 * Revision 1.5.2.3  2009/03/06 00:50:52  shihs
 * Support Montalvo PVDM3 and ethernet APIs.
 *
 * Revision 1.5.2.2  2008/04/29 21:25:24  siyen
 * Fixed Ramjet GE PHY registers test failure (CSCsq01335).
 *
 * Revision 1.5.2.1  2008/04/10 20:39:55  siyen
 * Added workaround for Marvell GE switch errata (CSCso31013). Also bumped up the SMI speed of the switch to 4.167 MHz.
 *
 * Revision 1.5  2008/02/29 02:25:02  siyen
 * Added SFP loopback supports.
 *
 * Revision 1.4  2007/12/22 00:59:39  siyen
 * Changed buf pointer in SMI API struct to *uint16_t, and offset to uint8_t.
 * cVS: ----------------------------------------------------------------------
 *
 * Revision 1.3  2007/11/29 01:25:11  siyen
 * SMI state table controller enum instead of the base address.
 *
 * Revision 1.2  2007/10/12 00:06:17  siyen
 * Removed N2G references.
 *
 * Revision 1.1  2007/10/10 17:58:02  siyen
 * Initial checkin.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 *
 *------------------------------------------------------------------
 */
