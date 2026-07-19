/* $Id: dev_dimm.h,v 1.2 2012/03/28 00:38:07 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_dimm/dev_dimm.h,v $
 *------------------------------------------------------------------
 * Filename: dev_dimm.h
 *
 * Description: DIMM Serial Presence-Detect Matrix. This file is based on
 *		Micron DDR2 Datasheet.
 *
 * Copyright (c) 2009 ~ 2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DEV_DIMM_H__
#define __DEV_DIMM_H__

#include "types.h"
#include "i2c_api.h"
#include "common_utils.h"
#include "dev_object.h"

/*
 * dev_error_report message codes
 */
typedef enum {
    DIMM_DEV_STATE = 0,
    DIMM_ATTACH,
    DIMM_SHOW,
    DIMM_GET_MEMSIZE,
}dimm_report_code_t;

typedef struct dimm_i2c {
    uchar size;		/* 0 - Number of SPD bytes used */
    uchar dev_size;	/* Total number of bytes in SPD device */
    uchar type;		/* Fundamental memory type */
    uchar row_addr;	/* Number of row address on SDRAM */
    uchar column_addr;	/* Number of column address on SDRAM */
    uchar height;	/* DIMM height and module ranks */
    uchar data_width;	/* Module data width */
    uchar res0;		/* Reserved */
    uchar v_if;		/* 8 - Module voltage interface levels */
    uchar tck_l4;	/* SDRAM cycle time tCK CAS latency of 4.0 */
    uchar tac_l4;	/* SDRAM access time tAC CAS latency of 4.0 */
    uchar conf_type;	/* Module configuration type */
    uchar ref_rate;	/* Refresh rate/type */
    uchar dev_width;	/* SDRAM device width */
    uchar err_data_w;	/* Error-checking SDRAM data width */
    uchar res1;		/* Reserved */
    uchar burst_len;	/* 0x10 - Burst lengths supported */
    uchar banks;	/* Number of banks on SDRAM device */
    uchar cas_lat;	/* CAS latencies supported */
    uchar mod_thick;	/* Module thickness */
    uchar ddr2_type;	/* DDR2 DIMM type */
    uchar mod_att;	/* SDRAM module attributes */
    uchar dev_att;	/* SDRAM device attributes */
    uchar tck_l3;	/* SDRAM cycle time tCK CAS latency of 3.0 */
    uchar tac_l3;	/* 0x18 - SDRAM access time tAC CAS latency of 3.0 */
    uchar tck_l2;	/* SDRAM cycle time tCK CAS latency of 2.0 */
    uchar tac_l2;	/* SDRAM access time tAC CAS latency of 2.0 */
    uchar trp;		/* MIN row precharge time, tRP */
    uchar trrd;		/* MIN row active to row active, tRRD */
    uchar trcd;		/* MIN RAS# to CAS# delay, tRCD */
    uchar tras;		/* MIN RAS# pulse width, tRAS */
    uchar rank_den;	/* Module rank density */
    uchar tisb;		/* 0x20 - Address and command setup time, tISb */
    uchar tihb;		/* Address and command hold time, tIHb */
    uchar tdsb;		/* Data/data mask input setup time, tDSb */
    uchar tdhb;		/* Data/data mask input hold time, tDHb */
    uchar twr;		/* Write recovery time, tWR */
    uchar twtr;		/* WRITE-to-READ command delay, tWTR */
    uchar trtp;		/* READ-to_PRECHARGE command delay, tRTP */
    uchar an_probe;	/* Memory analysis probe */
    uchar x_trc_trfc;	/* 0x28 - Extension for tRC and tRFC */
    uchar trc;		/* MIN active auto refresh time, tRC */
    uchar trfc;		/* MIN AUTO-REFRESH to ACTIVE command period, tRFC */
    uchar tckmax;	/* SDRAM device MAX cycle time, tCKMAX */
    uchar tdqsq;	/* SDRAM device MAX DQS-DQ skew time, tDQSA */
    uchar tqhs;		/* SDRAM device MAX read data hold skew factor, tQHS */
    uchar pll_relock;	/* PLL relock time */
    uchar res2[15];	/* 0x2F - 3D - Optional features, reserved */
    uchar spd_rev;	/* SPD revision */
    uchar chksum;	/* Checksum for bytes 0 - 62 */
    uchar jedec_id[8];	/* 0x40 - Manufacturer's JEDEC ID code */
    uchar mfg_loc;	/* 0x48 - Manufacturing location */
    uchar part_no[18];	/* Module part number */
    uchar pcb_id[2];	/* PCB identification code */
    uchar mfg_yr;	/* Year of manufacture */
    uchar mfg_wk;	/* Week of manufacture */
    uchar serial_no[4];	/* Module serial number */
    uchar mfg_sp[29];	/* Manufacturer-specific data */
/*    uchar res3[128];	 Reserved for customer use */
} dimm_i2c_t;

/* DIMM offsets defines */
#define DIMM_ROW_BITS		3	/* Number of Row Address Bits */
#define DIMM_COL_BITS		4	/* Number of Column Address Bits */
#define DIMM_RANKS		5	/* DIMM height and Module Ranks */
#define DIMM_SDRAM_WIDTH	13	/* SDRAM Width */
#define DIMM_BANKS		17	/* Number of SDRAM banks */

#define DIMM_RANKS_MASK		0x7	/* Number of ranks mask */
#define DIMM_MIN		20	/* Minimum of 1MB DIMM */

/*
 * device callin function - service provided and defined by the device
 */
typedef struct dimm_callin_fvt_t_ {
    int (*get_memsize)(dev_object_t *);
} dimm_callin_fvt_t;

/*
 * device callout function - service needed by the device and defined by
 *			     platform
 */
typedef struct dimm_callout_fvt_t_ {
    /*
     * Vectors set by the upper level (eg., platform).
     */
    uint32_t (*open)(n2g_i2c_if_t *);
    uint32_t (*close)(n2g_i2c_if_t *);
    uint32_t (*rd)(n2g_i2c_if_t *);
    uint32_t (*wr)(n2g_i2c_if_t *);
} dimm_callout_fvt_t;

/*
** Define the SPD DIMM device object structure.
*/
typedef struct dev_dimm_object_t_ {
    dev_object_t	base;
    dimm_callin_fvt_t	*callin_fvt;
    dimm_callout_fvt_t	*callout_fvt;
    n2g_i2c_if_t	*i2c_p;		/* I2C API interface pointer */
    uint32_t		*memsize;	/* points to memory size buffer */
    char		*dev_name;	/* points to text of DIMM name */
}dev_dimm_object_t;

/* Functions prototype */
extern void spd_dimm_dev_create(dev_object_t *, dev_error_report_t,
                                dev_object_fvt_t *);

#endif /* __DEV_DIMM_H__ */

/*
 *------------------------------------------------------------------
$Log: dev_dimm.h,v $
Revision 1.2  2012/03/28 00:38:07  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:01  ptong
Initial archive of ng_diag module


$Endlog$
*/
