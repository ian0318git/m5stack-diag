/* $Id: i2c_api.h,v 1.17 2021/06/02 07:42:56 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/i2c_api.h,v $
 *------------------------------------------------------------------
 * Filename: i2c_api.h
 *
 * Description: Shinkansen I2C header file.
 *
 * Copyright (c) 2006-2019 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __N2GI2C_H__
#define __N2GI2C_H__

#ifndef __SMI_API_H__

#include <unistd.h>

#endif /* __SMI_API_H__ */

typedef __pid_t pid_t;

/* Common defines */

/* I2C API interface struct */
typedef union {
    pid_t	lock_pid;	/* Locked PID */
} ret_info_t;


/* I2C device characteristics table */
typedef struct n2g_i2c_dev{
    uint8_t bus_no;		/* I2C bus enum */
    uint16_t dev_addr;		/* I2C slave physical address */
    uint8_t rd_hd_size;		/* I2C read header (offset) size */
    uint8_t wr_hd_size;		/* I2C write header (offset) size */
    int         fp;             /* File pointer */
} n2g_i2c_dev_t;

/* Definition of I2C interface characteristics */
typedef struct n2g_i2c_iface {
    uint32_t      offset;         /* I2C device register or memory offset...
                                   * if < 0, no offset will be sent on the bus.
                                   */
    uint8_t       i2c_bus_type;   /* I2C bus number */
    uint8_t       i2c_dev;        /* MB_I2C_DEVICE device ID */
    uint8_t       i2c_ctrl;       /* MB_I2C_DEVICE device ID */
    uint8_t       sub_addr_len;   /* I2C Sub address lenth */
    uint16_t      size;           /* Number of read/write bytes */
    uint8_t       rd_hd_size;     /* I2C read header (offset) size */
    uint8_t       wr_hd_size;     /* I2C write header (offset) size */
    uint8_t       mux;            /* Mux number that I2C device connected to */
    uint32_t      err_no;
    uint32_t      i2c_speed;
    char *buf;           /* Read/write buffer pointer */
    unsigned long i2c_base;       /* points to register of i2c controller */
    char *dev_name;      /* I2C device name */
} n2g_i2c_if_t;

/* I2C devices states */
typedef struct n2g_i2c_states {
    pid_t	pid;		/* Locked process ID */
    n2g_i2c_dev_t *i2c_dev;	/* Pointer to I2C characteristics struct */
    int		state;		/* I2C state */
} n2g_i2c_states_t;

/* Linux specific */
/* I2C host adapter */
typedef struct i2c_host {
    unsigned char        *dev_name;      /* /dev/i2c... */
    int         fp;             /* File pointer (user mode) */
} i2c_host_t;

/* I2C Controller States defines */
#define N2G_I2C_IDLE		0	/* Idle state */
#define N2G_I2C_LOCKED		1	/* Locked Busy state */
#define N2G_I2C_ADDR_WR_ERR	2	/* Unable to write I2c */
#define N2G_I2C_DATA_WR_ERR	3	/* Unable to write I2c */
#define N2G_I2C_DATA_RD_ERR	3	/* Unable to write I2c */
#define N2G_I2C_O_ERR		4	/* Unable to open device */
/*
 * Note: Changes for Informers --
 * Informers use the (n2g_i2c_states.pid != 0) as the locked
 * condition by a process. When (pid == 0), the slave device is
 * not locked by any process. So N2G_I2C_LOCKED is not used in
 * Informers i2c_api.c. Informers use N2G_I2C_SLV_BUSY to mark if the
 * slave device is busy transferring data in the n2g_i2c_states.state
 * field during write and read operations.
 */
#define N2G_I2C_SLV_BUSY  N2G_I2C_LOCKED /* Device is transferring data */

/* I2C bus speed */
typedef enum {
    N2G_I2C_100KHZ,			/* 100 KHz */
    N2G_I2C_400KHZ,			/* 400 KHz */
} N2G_I2C_SPEED;

/* Goofy I2C master DMA mode bus speed */
typedef enum {
    N2G_I2C_DMA_400KHZ,			/* 400 KHz */
    N2G_I2C_DMA_HI_SPEED,		/* 1.5 MHz */
} N2G_I2C_DMA_SPEED;

/* I2C operation */
#define	N2G_I2C_WRITE		0	/* I2C write command */
#define N2G_I2C_READ		1	/* I2C read command */

/* Miscellaneous defines */
#define N2G_I2C_RETRY1          1       /* Number of retry */
#define N2G_I2C_RETRY2          2       /* Number of retry */
#define N2G_I2C_RETRY3          3       /* Number of retry */
#define I2C_MAX_XFER_SIZE	0xFF	/* size field is uint8_t */
#define N2G_I2C_OPEN_TIMEOUT    100     /* Open Busy timeout */
#define N2G_I2C_BIT_DELAY	11	/* 100 KHz - 10 microseconds with one
					 * extra microsecond for granularity */
#define I2C_SLAVE_ADDR_SHIFT	1	/* slave address shift count */
#define I2C_RESET_TIME		30	/* Reset and init minimum time */
#define I2C_DATA_BYTE_XMIT_TIME (9 * N2G_I2C_BIT_DELAY)	/* time to xmit byte.
					 * ACK/NACK included */
#define I2C_BUS_FREE_TIME	10	/* Bus free time between a STOP and
					 * START condition - tBUF (I2C spec).
					 * 4.7 us. We use 10 us for the worst
					 */

/* I2C API prototypes */
extern uint i2c_setup(void);
extern uint32_t n2g_i2c_reset(n2g_i2c_if_t *i2c_p);
extern uint32_t n2g_i2c_init(n2g_i2c_if_t *i2c_p);
extern uint32_t n2g_i2c_open(n2g_i2c_if_t *i2c_p);
extern uint32_t n2g_i2c_close(n2g_i2c_if_t *i2c_p);
extern uint32_t n2g_i2c_read(n2g_i2c_if_t *i2c_p);
extern uint32_t n2g_i2c_write(n2g_i2c_if_t *i2c_p);
extern uint32_t fpga_i2c_open(n2g_i2c_if_t *i2c_p);
extern uint32_t fpga_i2c_close(n2g_i2c_if_t *i2c_p);
extern uint32_t fpga_i2c_read(n2g_i2c_if_t *i2c_p);
extern uint32_t fpga_i2c_write(n2g_i2c_if_t *i2c_p);
extern uint32_t i2c_rd(n2g_i2c_if_t *i2c_p);
extern uint32_t i2c_wr(n2g_i2c_if_t *i2c_p);
extern uint32_t goofy_fpga_dma(n2g_i2c_if_t *i2c_p);
extern int32_t  i2c_dev_rd(void *);
extern int32_t  i2c_dev_wr(void *, uint8_t);

extern uint32_t api_mb_i2c_reset(uint8_t i2c_ctl);
#if defined(DYNO) || defined(OVERLORD) || defined(WOODLAWN) || defined(WALLANDER) \
    || defined(TSN) || defined(VIPER) || defined(NUTELLA) || defined(TABEIL) \
    || defined(NANOOK) || defined(HIGHRISE) || defined(PHOENIX) || defined(FUGAZI)
extern uint32_t api_mb_i2c_init(i2c_host_t *i2c_ctl, char i2c_speed);
#else
extern uint32_t api_mb_i2c_init(uint8_t i2c_ctl, char i2c_speed);
#endif /* VINDICATOR */
extern uint32_t api_mb_i2c_read(n2g_i2c_dev_t *dev_p, uint32_t offset,
				uint8_t size, char *buf);
extern uint32_t api_mb_i2c_write(n2g_i2c_dev_t *dev_p, uint32_t offset,
				 uint8_t size, char *buf);
extern int i2c_err_no(uint32_t *);
extern char *i2c_err_str(int num);
#endif /* __N2GI2C_H__ */

/*------------------------------------------------------------------
$Log: i2c_api.h,v $
Revision 1.17  2021/06/02 07:42:56  iachang
CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk

Revision 1.16  2021/04/14 09:10:13  achiu2
[PRRQ:CSCvx56970-2] Phoenix code review for ER

Revision 1.15  2020/08/19 09:48:59  markzha
*** empty log message ***

Revision 1.14  2019/12/11 10:10:22  lucywang
Merged Nanook to main trunk

Revision 1.13  2019/10/17 02:16:14  kehuang2
Collapse Tabei-L into main trunk

Revision 1.12  2019/07/11 12:34:40  alicehua
Collapse Nutella codes into main trunk

Revision 1.11.40.1  2019/01/25 02:11:06  harrchan
Add definition of NUTELLA

Revision 1.11  2018/08/06 02:30:59  harrchan
Merge viper E2E to the main trunk (CSCvk28469)

Revision 1.10  2017/08/02 14:21:28  steja
Support TSN-H/M platform code

Revision 1.9.40.1  2017/07/29 03:40:43  steja
tsn-branch5 synced with Maintrunk repositories

Revision 1.9  2015/02/26 07:27:14  xiaoyizh
Add Wallander support.

Revision 1.8  2013/11/26 08:40:33  hroni
fix compiler warning

Revision 1.7  2013/10/08 08:48:25  tirawan
Woodlawn collapsed to main trunk

Revision 1.6  2013/03/14 18:18:14  mcharon
add argument to i2c_err_no

Revision 1.5  2013/02/06 06:09:34  mcharon
add i2c err_no . when fail use this to get failure reason

Revision 1.4  2012/11/07 19:41:16  mcharon
add extern for i2c_err_str

Revision 1.3  2012/06/05 11:44:24  palin2
Clean up compiler warnings.

Revision 1.2  2012/03/28 00:38:11  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:03  ptong
Initial archive of ng_diag module


$Endlog$
*/

