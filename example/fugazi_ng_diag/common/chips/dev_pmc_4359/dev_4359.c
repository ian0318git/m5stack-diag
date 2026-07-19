/* $Id: dev_4359.c,v 1.4 2012/06/11 21:54:41 ywen Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_pmc_4359/dev_4359.c,v $
 *------------------------------------------------------------------
 *
 * pm4359.c - PMC Sierra Comet Tetra FRAMER device driver functions.
 * 
 * Copyright (c) 2007-2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 * Author: Hoang Nguyen - Ported from PM4354.c by Bao Buu
 *------------------------------------------------------------------
 */

#include "endians.h"
#include "types.h"
#include "defs.h"
#include "common.h"
#include "common_utils.h"
#include "dev_print.h"
#include "dev_object.h"
#include "dev_4359.h"

#undef IDEBUG
#undef XDEBUG

dev_object_fvt_t       pmc4359_fvt;
dev_4359_callin_fvt_t  pmc4359_callin;
dev_4359_callout_fvt_t pmc4359_callout;

/*===================================================================*
 *                    Function Prototypes                            *
 *===================================================================*/
static uint32 pmc4359_dev_attach (dev_object_t *dev);
static uint32 pmc4359_dev_init (dev_object_t *dev);
static void pmc4359_destroy(dev_object_t **dev);
static int pmc4359_reg_test(dev_object_t *dev, int num_ports);
static int pm4359_set_operation(dev_object_t *dev);
static int pmc4359_init_xpsc(dev_object_t *dev, uchar frm_num, ushort reg);
static int pmc4359_dump_xpsc(dev_object_t *, ulong, ushort);
static int pmc4359_read_rlps(dev_object_t *, ulong);
static int pm4359_linetx_analog_conf(dev_object_t *dev);
static void pm4359_set_tx_mode(dev_object_t *dev, ulong frm_bar, uchar op_mode,
			       uchar frm_mode);
static void pm4359_set_rx_mode(dev_object_t *dev, ulong frm_bar, uchar op_mode,
			       uchar frm_mode);
static void pmc4359_set_loopback(dev_object_t *dev, uchar frm_num, 
				 uchar loop_mode);
static void pmc4359_dev_reset(dev_object_t *dev);
static void pmc4359_set_cfg_info(dev_object_t *dev, uchar port, uchar mode);
static void pm4359_t1_tx_conf(dev_object_t *dev);
static void pm4359_t1_rx_conf(dev_object_t *dev);
static void pm4359_e1_tx_conf(dev_object_t *dev);
static void pm4359_e1_rx_conf(dev_object_t *dev);
static void pm4359_rx_jat_conf(dev_object_t *dev);
static void pm4359_tx_jat_conf(dev_object_t *dev);
static void pm4359_rx_clk_conf(dev_object_t *dev);
static void pm4359_bp_clk_conf(dev_object_t *dev);
static void pm4359_bp_fp_conf(dev_object_t *dev);
static void pm4359_clk_sync_conf(dev_object_t *dev);
static void pm4359_rx_line_conf(dev_object_t *dev);
static void pm4359_conf_rlps_ram(dev_object_t *dev, ulong *ram);
static void pm4359_program_xlpg(dev_object_t *dev, uchar waveform[24][5]);
static void pm4359_program_fuses(dev_object_t *dev);
static void pm4359_rlps_opti(dev_object_t *dev);
static void pm4359_rlps_volt(dev_object_t *dev);
static uint32 pmc4359_dev_show(dev_object_t *dev, print_fn_t dev_print, 
			       dev_show_cmd_e cmd);
static void pmc4359_ycable_enab(dev_object_t *dev, uchar frm_num, 
				ulong enable);
int pmc4359_wr_ind_reg(dev_object_t *, ulong, int, uchar);
int pmc4359_rd_ind_reg(dev_object_t *, ulong, int, uchar *);

/* Buffer for error or information log */
uchar log_buffer[384];

uchar pm4359_xlpg_wvfmtbls[CMQ_TX_LBO_E1_120OHM + 1][CMQ_XLPG_MAX_SAMPLES][CMQ_XLPG_MAX_UNITS] =
{
    /* CMQ_TX_LBO_T1_LONG_HAUL_0DB */
        {{  0x00, 0x44, 0x00, 0x00, 0x00 },
          { 0x00, 0x43, 0x00, 0x00, 0x00 },
          { 0x0F, 0x42, 0x00, 0x00, 0x00 },
          { 0x27, 0x41, 0x00, 0x00, 0x00 },
          { 0x34, 0x00, 0x00, 0x00, 0x00 },
          { 0x34, 0x00, 0x00, 0x00, 0x00 },
          { 0x34, 0x00, 0x00, 0x00, 0x00 },
          { 0x34, 0x00, 0x00, 0x00, 0x00 },
          { 0x34, 0x00, 0x00, 0x00, 0x00 },
          { 0x34, 0x00, 0x00, 0x00, 0x00 },
          { 0x34, 0x00, 0x00, 0x00, 0x00 },
          { 0x34, 0x00, 0x00, 0x00, 0x00 },
          { 0x34, 0x00, 0x00, 0x00, 0x00 },
          { 0x33, 0x00, 0x00, 0x00, 0x00 },
          { 0x2B, 0x00, 0x00, 0x00, 0x00 },
          { 0x21, 0x00, 0x00, 0x00, 0x00 },
          { 0x4A, 0x00, 0x00, 0x00, 0x00 },
          { 0x4B, 0x00, 0x00, 0x00, 0x00 },
          { 0x4B, 0x00, 0x00, 0x00, 0x00 },
          { 0x4B, 0x00, 0x00, 0x00, 0x00 },
          { 0x4A, 0x00, 0x00, 0x00, 0x00 },
          { 0x46, 0x00, 0x00, 0x00, 0x00 },
          { 0x45, 0x00, 0x00, 0x00, 0x00 },
          { 0x44, 0x00, 0x00, 0x00, 0x00  }}, /*, 23, */

    /* CMQ_TX_LBO_T1_LONG_HAUL_7_5DB */
        {{  0x04, 0x16, 0x00, 0x00, 0x00 },
          { 0x0A, 0x13, 0x00, 0x00, 0x00 },
          { 0x11, 0x12, 0x00, 0x00, 0x00 },
          { 0x1A, 0x10, 0x00, 0x00, 0x00 },
          { 0x21, 0x0F, 0x00, 0x00, 0x00 },
          { 0x26, 0x0E, 0x00, 0x00, 0x00 },
          { 0x2A, 0x0D, 0x00, 0x00, 0x00 },
          { 0x2F, 0x0C, 0x00, 0x00, 0x00 },
          { 0x35, 0x0B, 0x00, 0x00, 0x00 },
          { 0x38, 0x0A, 0x00, 0x00, 0x00 },
          { 0x3A, 0x09, 0x00, 0x00, 0x00 },
          { 0x3A, 0x08, 0x00, 0x00, 0x00 },
          { 0x3A, 0x07, 0x00, 0x00, 0x00 },
          { 0x3A, 0x06, 0x00, 0x00, 0x00 },
          { 0x39, 0x05, 0x00, 0x00, 0x00 },
          { 0x35, 0x04, 0x00, 0x00, 0x00 },
          { 0x32, 0x03, 0x00, 0x00, 0x00 },
          { 0x25, 0x03, 0x00, 0x00, 0x00 },
          { 0x22, 0x02, 0x00, 0x00, 0x00 },
          { 0x20, 0x02, 0x00, 0x00, 0x00 },
          { 0x1E, 0x01, 0x00, 0x00, 0x00 },
          { 0x1D, 0x01, 0x00, 0x00, 0x00 },
          { 0x1A, 0x00, 0x00, 0x00, 0x00 },
          { 0x19, 0x00, 0x00, 0x00, 0x00  }}, /*, 23, */

    /* CMQ_TX_LBO_T1_LONG_HAUL_15DB */
        {{  0x03, 0x2A, 0x12, 0x02, 0x00 },
          { 0x08, 0x29, 0x11, 0x01, 0x00 },
          { 0x0D, 0x28, 0x10, 0x00, 0x00 },
          { 0x12, 0x27, 0x0F, 0x00, 0x00 },
          { 0x18, 0x26, 0x0F, 0x00, 0x00 },
          { 0x1B, 0x25, 0x0E, 0x00, 0x00 },
          { 0x20, 0x24, 0x0D, 0x00, 0x00 },
          { 0x24, 0x23, 0x0D, 0x00, 0x00 },
          { 0x28, 0x22, 0x0C, 0x00, 0x00 },
          { 0x2B, 0x21, 0x0B, 0x00, 0x00 },
          { 0x2E, 0x20, 0x0B, 0x00, 0x00 },
          { 0x31, 0x1F, 0x0A, 0x00, 0x00 },
          { 0x34, 0x1E, 0x09, 0x00, 0x00 },
          { 0x36, 0x1D, 0x09, 0x00, 0x00 },
          { 0x36, 0x1C, 0x08, 0x00, 0x00 },
          { 0x36, 0x1B, 0x07, 0x00, 0x00 },
          { 0x34, 0x1A, 0x07, 0x00, 0x00 },
          { 0x32, 0x19, 0x06, 0x00, 0x00 },
          { 0x30, 0x18, 0x05, 0x00, 0x00 },
          { 0x2F, 0x17, 0x05, 0x00, 0x00 },
          { 0x2E, 0x16, 0x04, 0x00, 0x00 },
          { 0x2D, 0x15, 0x03, 0x00, 0x00 },
          { 0x2C, 0x14, 0x03, 0x00, 0x00 },
          { 0x2B, 0x13, 0x02, 0x00, 0x00  }}, /* 23 */

    /* CMQ_TX_LBO_T1_LONG_HAUL_22_5DB */
        {{  0x03, 0x20, 0x10, 0x04, 0x01 },
          { 0x05, 0x1F, 0x0F, 0x04, 0x01 },
          { 0x07, 0x1F, 0x0F, 0x03, 0x01 },
          { 0x08, 0x1E, 0x0E, 0x03, 0x01 },
          { 0x09, 0x1D, 0x0D, 0x02, 0x01 },
          { 0x0A, 0x1D, 0x0D, 0x02, 0x01 },
          { 0x0D, 0x1C, 0x0C, 0x01, 0x01 },
          { 0x11, 0x1B, 0x0B, 0x01, 0x01 },
          { 0x13, 0x1B, 0x0B, 0x01, 0x01 },
          { 0x16, 0x1A, 0x0A, 0x01, 0x01 },
          { 0x19, 0x19, 0x09, 0x01, 0x01 },
          { 0x1B, 0x19, 0x09, 0x01, 0x01 },
          { 0x1D, 0x18, 0x08, 0x01, 0x01 },
          { 0x1E, 0x17, 0x07, 0x01, 0x01 },
          { 0x21, 0x17, 0x07, 0x01, 0x01 },
          { 0x22, 0x16, 0x07, 0x01, 0x01 },
          { 0x23, 0x15, 0x06, 0x01, 0x01 },
          { 0x24, 0x15, 0x06, 0x01, 0x01 },
          { 0x25, 0x14, 0x06, 0x01, 0x01 },
          { 0x25, 0x13, 0x05, 0x01, 0x01 },
          { 0x25, 0x13, 0x05, 0x01, 0x01 },
          { 0x23, 0x12, 0x05, 0x01, 0x01 },
          { 0x22, 0x11, 0x04, 0x01, 0x01 },
          { 0x21, 0x11, 0x04, 0x01, 0x01  }}, /* 23 */

    /* CMQ_TX_LBO_T1_SHORT_HAUL_110FT */
        {{  0x00, 0x44, 0x00, 0x00, 0x00 },
          { 0x00, 0x43, 0x00, 0x00, 0x00 },
          { 0x0F, 0x42, 0x00, 0x00, 0x00 },
          { 0x27, 0x42, 0x00, 0x00, 0x00 },
          { 0x34, 0x41, 0x00, 0x00, 0x00 },
          { 0x34, 0x00, 0x00, 0x00, 0x00 },
          { 0x34, 0x00, 0x00, 0x00, 0x00 },
          { 0x34, 0x00, 0x00, 0x00, 0x00 },
          { 0x34, 0x00, 0x00, 0x00, 0x00 },
          { 0x34, 0x00, 0x00, 0x00, 0x00 },
          { 0x34, 0x00, 0x00, 0x00, 0x00 },
          { 0x34, 0x00, 0x00, 0x00, 0x00 },
          { 0x34, 0x00, 0x00, 0x00, 0x00 },
          { 0x33, 0x00, 0x00, 0x00, 0x00 },
          { 0x2B, 0x00, 0x00, 0x00, 0x00 },
          { 0x21, 0x00, 0x00, 0x00, 0x00 },
          { 0x4A, 0x00, 0x00, 0x00, 0x00 },
          { 0x4B, 0x00, 0x00, 0x00, 0x00 },
          { 0x4B, 0x00, 0x00, 0x00, 0x00 },
          { 0x4B, 0x00, 0x00, 0x00, 0x00 },
          { 0x4A, 0x00, 0x00, 0x00, 0x00 },
          { 0x46, 0x00, 0x00, 0x00, 0x00 },
          { 0x45, 0x00, 0x00, 0x00, 0x00 },
          { 0x44, 0x00, 0x00, 0x00, 0x00  }}, /* 23 */

    /* CMQ_TX_LBO_T1_SHORT_HAUL_220FT */
        {{  0x00, 0x45, 0x00, 0x00, 0x00 },
          { 0x00, 0x44, 0x00, 0x00, 0x00 },
          { 0x0F, 0x43, 0x00, 0x00, 0x00 },
          { 0x27, 0x43, 0x00, 0x00, 0x00 },
          { 0x39, 0x42, 0x00, 0x00, 0x00 },
          { 0x39, 0x00, 0x00, 0x00, 0x00 },
          { 0x37, 0x00, 0x00, 0x00, 0x00 },
          { 0x35, 0x00, 0x00, 0x00, 0x00 },
          { 0x34, 0x00, 0x00, 0x00, 0x00 },
          { 0x34, 0x00, 0x00, 0x00, 0x00 },
          { 0x34, 0x00, 0x00, 0x00, 0x00 },
          { 0x33, 0x00, 0x00, 0x00, 0x00 },
          { 0x33, 0x00, 0x00, 0x00, 0x00 },
          { 0x33, 0x00, 0x00, 0x00, 0x00 },
          { 0x2B, 0x00, 0x00, 0x00, 0x00 },
          { 0x21, 0x00, 0x00, 0x00, 0x00 },
          { 0x4C, 0x00, 0x00, 0x00, 0x00 },
          { 0x4E, 0x00, 0x00, 0x00, 0x00 },
          { 0x4E, 0x00, 0x00, 0x00, 0x00 },
          { 0x4E, 0x00, 0x00, 0x00, 0x00 },
          { 0x4B, 0x00, 0x00, 0x00, 0x00 },
          { 0x49, 0x00, 0x00, 0x00, 0x00 },
          { 0x48, 0x00, 0x00, 0x00, 0x00 },
          { 0x47, 0x00, 0x00, 0x00, 0x00  }}, /* 23 */

    /* CMQ_TX_LBO_T1_SHORT_HAUL_330FT */
        {{  0x00, 0x44, 0x00, 0x00, 0x00 },
          { 0x00, 0x43, 0x00, 0x00, 0x00 },
          { 0x0F, 0x42, 0x00, 0x00, 0x00 },
          { 0x27, 0x42, 0x00, 0x00, 0x00 },
          { 0x3D, 0x41, 0x00, 0x00, 0x00 },
          { 0x3D, 0x41, 0x00, 0x00, 0x00 },
          { 0x3B, 0x00, 0x00, 0x00, 0x00 },
          { 0x37, 0x00, 0x00, 0x00, 0x00 },
          { 0x34, 0x00, 0x00, 0x00, 0x00 },
          { 0x34, 0x00, 0x00, 0x00, 0x00 },
          { 0x34, 0x00, 0x00, 0x00, 0x00 },
          { 0x34, 0x00, 0x00, 0x00, 0x00 },
          { 0x34, 0x00, 0x00, 0x00, 0x00 },
          { 0x33, 0x00, 0x00, 0x00, 0x00 },
          { 0x33, 0x00, 0x00, 0x00, 0x00 },
          { 0x30, 0x00, 0x00, 0x00, 0x00 },
          { 0x5A, 0x00, 0x00, 0x00, 0x00 },
          { 0x58, 0x00, 0x00, 0x00, 0x00 },
          { 0x4E, 0x00, 0x00, 0x00, 0x00 },
          { 0x4E, 0x00, 0x00, 0x00, 0x00 },
          { 0x4B, 0x00, 0x00, 0x00, 0x00 },
          { 0x49, 0x00, 0x00, 0x00, 0x00 },
          { 0x47, 0x00, 0x00, 0x00, 0x00 },
          { 0x44, 0x00, 0x00, 0x00, 0x00  }}, /* 23 */

    /* CMQ_TX_LBO_T1_SHORT_HAUL_440FT */
        {{  0x00, 0x44, 0x00, 0x00, 0x00 },
          { 0x00, 0x43, 0x00, 0x00, 0x00 },
          { 0x0F, 0x42, 0x00, 0x00, 0x00 },
          { 0x27, 0x42, 0x00, 0x00, 0x00 },
          { 0x3C, 0x41, 0x00, 0x00, 0x00 },
          { 0x3C, 0x41, 0x00, 0x00, 0x00 },
          { 0x3A, 0x00, 0x00, 0x00, 0x00 },
          { 0x35, 0x00, 0x00, 0x00, 0x00 },
          { 0x34, 0x00, 0x00, 0x00, 0x00 },
          { 0x34, 0x00, 0x00, 0x00, 0x00 },
          { 0x34, 0x00, 0x00, 0x00, 0x00 },
          { 0x34, 0x00, 0x00, 0x00, 0x00 },
          { 0x33, 0x00, 0x00, 0x00, 0x00 },
          { 0x32, 0x00, 0x00, 0x00, 0x00 },
          { 0x32, 0x00, 0x00, 0x00, 0x00 },
          { 0x30, 0x00, 0x00, 0x00, 0x00 },
          { 0x5A, 0x00, 0x00, 0x00, 0x00 },
          { 0x58, 0x00, 0x00, 0x00, 0x00 },
          { 0x4E, 0x00, 0x00, 0x00, 0x00 },
          { 0x4E, 0x00, 0x00, 0x00, 0x00 },
          { 0x4B, 0x00, 0x00, 0x00, 0x00 },
          { 0x49, 0x00, 0x00, 0x00, 0x00 },
          { 0x47, 0x00, 0x00, 0x00, 0x00 },
          { 0x44, 0x00, 0x00, 0x00, 0x00  }}, /* 23 */

    /* CMQ_TX_LBO_T1_SHORT_HAUL_550FT */
        {{  0x00, 0x44, 0x00, 0x00, 0x00 },
          { 0x00, 0x43, 0x00, 0x00, 0x00 },
          { 0x0F, 0x42, 0x00, 0x00, 0x00 },
          { 0x27, 0x42, 0x00, 0x00, 0x00 },
          { 0x3D, 0x41, 0x00, 0x00, 0x00 },
          { 0x3D, 0x41, 0x00, 0x00, 0x00 },
          { 0x35, 0x00, 0x00, 0x00, 0x00 },
          { 0x32, 0x00, 0x00, 0x00, 0x00 },
          { 0x31, 0x00, 0x00, 0x00, 0x00 },
          { 0x31, 0x00, 0x00, 0x00, 0x00 },
          { 0x2E, 0x00, 0x00, 0x00, 0x00 },
          { 0x2E, 0x00, 0x00, 0x00, 0x00 },
          { 0x2E, 0x00, 0x00, 0x00, 0x00 },
          { 0x2C, 0x00, 0x00, 0x00, 0x00 },
          { 0x2A, 0x00, 0x00, 0x00, 0x00 },
          { 0x28, 0x00, 0x00, 0x00, 0x00 },
          { 0x6B, 0x00, 0x00, 0x00, 0x00 },
          { 0x6B, 0x00, 0x00, 0x00, 0x00 },
          { 0x4D, 0x00, 0x00, 0x00, 0x00 },
          { 0x4D, 0x00, 0x00, 0x00, 0x00 },
          { 0x4B, 0x00, 0x00, 0x00, 0x00 },
          { 0x49, 0x00, 0x00, 0x00, 0x00 },
          { 0x47, 0x00, 0x00, 0x00, 0x00 },
          { 0x44, 0x00, 0x00, 0x00, 0x00  }}, /* 23 */

    /* CMQ_TX_LBO_T1_SHORT_HAUL_660FT */
        {{  0x00, 0x42, 0x00, 0x00, 0x00 },
          { 0x10, 0x42, 0x00, 0x00, 0x00 },
          { 0x25, 0x41, 0x00, 0x00, 0x00 },
          { 0x3B, 0x41, 0x00, 0x00, 0x00 },
          { 0x3B, 0x41, 0x00, 0x00, 0x00 },
          { 0x33, 0x01, 0x00, 0x00, 0x00 },
          { 0x2C, 0x00, 0x00, 0x00, 0x00 },
          { 0x2B, 0x00, 0x00, 0x00, 0x00 },
          { 0x2B, 0x00, 0x00, 0x00, 0x00 },
          { 0x2A, 0x00, 0x00, 0x00, 0x00 },
          { 0x29, 0x00, 0x00, 0x00, 0x00 },
          { 0x29, 0x00, 0x00, 0x00, 0x00 },
          { 0x27, 0x00, 0x00, 0x00, 0x00 },
          { 0x27, 0x00, 0x00, 0x00, 0x00 },
          { 0x27, 0x00, 0x00, 0x00, 0x00 },
          { 0x55, 0x00, 0x00, 0x00, 0x00 },
          { 0x5B, 0x00, 0x00, 0x00, 0x00 },
          { 0x5B, 0x00, 0x00, 0x00, 0x00 },
          { 0x5D, 0x00, 0x00, 0x00, 0x00 },
          { 0x49, 0x00, 0x00, 0x00, 0x00 },
          { 0x48, 0x00, 0x00, 0x00, 0x00 },
          { 0x45, 0x00, 0x00, 0x00, 0x00 },
          { 0x44, 0x00, 0x00, 0x00, 0x00 },
          { 0x44, 0x00, 0x00, 0x00, 0x00  }}, /* 23 */

    /* CMQ_TX_LBO_E1_75OHM */
        {{  0x00, 0x00, 0x00, 0x00, 0x00 },
          { 0x00, 0x00, 0x00, 0x00, 0x00 },
          { 0x10, 0x00, 0x00, 0x00, 0x00 },
          { 0x35, 0x00, 0x00, 0x00, 0x00 },
          { 0x35, 0x00, 0x00, 0x00, 0x00 },
          { 0x35, 0x00, 0x00, 0x00, 0x00 },
          { 0x35, 0x00, 0x00, 0x00, 0x00 },
          { 0x34, 0x00, 0x00, 0x00, 0x00 },
          { 0x34, 0x00, 0x00, 0x00, 0x00 },
          { 0x34, 0x00, 0x00, 0x00, 0x00 },
          { 0x34, 0x00, 0x00, 0x00, 0x00 },
          { 0x34, 0x00, 0x00, 0x00, 0x00 },
          { 0x34, 0x00, 0x00, 0x00, 0x00 },
          { 0x34, 0x00, 0x00, 0x00, 0x00 },
          { 0x34, 0x00, 0x00, 0x00, 0x00 },
          { 0x05, 0x00, 0x00, 0x00, 0x00 },
          { 0x41, 0x00, 0x00, 0x00, 0x00 },
          { 0x00, 0x00, 0x00, 0x00, 0x00 },
          { 0x00, 0x00, 0x00, 0x00, 0x00 },
          { 0x00, 0x00, 0x00, 0x00, 0x00 },
          { 0x00, 0x00, 0x00, 0x00, 0x00 },
          { 0x00, 0x00, 0x00, 0x00, 0x00 },
          { 0x00, 0x00, 0x00, 0x00, 0x00 },
          { 0x00, 0x00, 0x00, 0x00, 0x00 }}, /* 23 */

    /* CMQ_TX_LBO_E1_120OHM */
        {{  0x00, 0x00, 0x00, 0x00, 0x00 },
          { 0x00, 0x00, 0x00, 0x00, 0x00 },
          { 0x10, 0x00, 0x00, 0x00, 0x00 },
          { 0x35, 0x00, 0x00, 0x00, 0x00 },
          { 0x35, 0x00, 0x00, 0x00, 0x00 },
          { 0x35, 0x00, 0x00, 0x00, 0x00 },
          { 0x35, 0x00, 0x00, 0x00, 0x00 },
          { 0x34, 0x00, 0x00, 0x00, 0x00 },
          { 0x34, 0x00, 0x00, 0x00, 0x00 },
          { 0x34, 0x00, 0x00, 0x00, 0x00 },
          { 0x34, 0x00, 0x00, 0x00, 0x00 },
          { 0x34, 0x00, 0x00, 0x00, 0x00 },
          { 0x34, 0x00, 0x00, 0x00, 0x00 },
          { 0x34, 0x00, 0x00, 0x00, 0x00 },
          { 0x34, 0x00, 0x00, 0x00, 0x00 },
          { 0x05, 0x00, 0x00, 0x00, 0x00 },
          { 0x41, 0x00, 0x00, 0x00, 0x00 },
          { 0x00, 0x00, 0x00, 0x00, 0x00 },
          { 0x00, 0x00, 0x00, 0x00, 0x00 },
          { 0x00, 0x00, 0x00, 0x00, 0x00 },
          { 0x00, 0x00, 0x00, 0x00, 0x00 },
          { 0x00, 0x00, 0x00, 0x00, 0x00 },
          { 0x00, 0x00, 0x00, 0x00, 0x00 },
          { 0x00, 0x00, 0x00, 0x00, 0x00  }}, /* 23 */
};

/* XLPG waveform scale factors */
uchar pm4359_xlpg_wvfmscale[] = {
     0x10, /* CMQ_TX_LBO_T1_LONG_HAUL_0DB */
     0x06, /* CMQ_TX_LBO_T1_LONG_HAUL_7_5DB */
     0x03, /* CMQ_TX_LBO_T1_LONG_HAUL_15DB */
     0x02, /* CMQ_TX_LBO_T1_LONG_HAUL_22_5DB */
     0x10, /* CMQ_TX_LBO_T1_SHORT_HAUL_110FT */
     0x11, /* CMQ_TX_LBO_T1_SHORT_HAUL_220FT */
     0x12, /* CMQ_TX_LBO_T1_SHORT_HAUL_330FT */
     0x13, /* CMQ_TX_LBO_T1_SHORT_HAUL_440FT */
     0x16, /* CMQ_TX_LBO_T1_SHORT_HAUL_550FT */
     0x1A, /* CMQ_TX_LBO_T1_SHORT_HAUL_660FT */
     0x17, /* CMQ_TX_LBO_E1_75OHM */
     0x17, /* CMQ_TX_LBO_E1_120OHM */
     0x0C, /* USER DEFINED */
};

long
pm4359_rlps_ram[CMQ_RX_LINE_EQ_RAM_E1 + 1][CMQ_RLPS_EQUALIZER_RAM_SIZE] =
{
    {   0x03061C3F, 0x03061C3D, 0x03061C3A, 0x03062C3D,
        0x03062C3B, 0x03062C38, 0x030E2C3F, 0x030E2C3C,
        0x030E2C38, 0x03162C3F, 0x03162C3D, 0x03162C3A,
        0x03163C3F, 0x03163C38, 0x0316283B, 0x0316383B,
        0x03163CBB, 0x031E3CBF, 0x031E3CBD, 0x031E3CBA,
        0x031E3CB8, 0x03263CBC, 0x032628BA, 0x032638BB,
        0x0B263D3F, 0x0B263D3E, 0x0B263D3D, 0x0B263D3C,
        0x0B26293A, 0x0B26393F, 0x13262DB8, 0x132E2DBF,
        0x132E2DBF, 0x132E2DBE, 0x132E2DBD, 0x132E2DBC,
        0x132E2DBB, 0x132E19B8, 0x132E29BF, 0x1B2E1E38,
        0x1B361E3F, 0x1B361E3C, 0x1B361E3B, 0x1B360A3B,
        0x1B361A3B, 0x23361EBF, 0x23361EB8, 0x23361EBF,
        0x233E1EBD, 0x2B3E1EBB, 0x2B3E1EB8, 0x2B461EBF,
        0x33461EBD, 0x33461EBA, 0x33461EB8, 0x334E1EBA,
        0x32461EBC, 0x3A4E1EBF, 0x3A4E1EBC, 0x3A4E0ABA,
        0x3A4E1AB8, 0x424E1F3F, 0x424E0F3E, 0x424E0F3D,
        0x424E0F3C, 0x424E0F3A, 0x424E0F3B, 0x424E1F3A,
        0x4A561F3F, 0x4A561F3B, 0x4A560B38, 0x4A561B3B,
        0x52561FBF, 0x52561FBC, 0x52561FB8, 0x525E1FBF,
        0x5A5E1FBE, 0x5A5E1FBC, 0x5A5E1FBB, 0x5A5E1FB9,
        0x5A5C1FBB, 0x62641FBF, 0x62641FBE, 0x62641FBD,
        0x62641FBD, 0x62641FBC, 0x62661FBC, 0x62661BBB,
        0x62661BBA, 0x62661BB9, 0x62661BB8, 0x62662BBD,
        0x6A662BBC, 0x6A662BBA, 0x6A663BBD, 0x6A663BBB,
        0x6A663BB8, 0x726E3BBF, 0x726E3BBC, 0x726E3BB8,
        0x72763BBF, 0x7A763BBD, 0x7A763BBA, 0x7A763BB8,
        0x7A7E3BBB, 0x827E37BF, 0x827E37BE, 0x827E37BD,
        0x827E37BB, 0x827E37BA, 0x8A7E47BF, 0x8A7E47BF,
        0x8A7E47BF, 0x8A7E47BF, 0x897647B8, 0x898647BB,
        0x898657BF, 0x898657BE, 0x898657BD, 0x898657BC,
        0x898657BB, 0x898657BA, 0x918E57B9, 0x918E57B8,
        0x918E57BF, 0x918E57BF, 0x908E57BA, 0x908E47B9,
        0x909647BB, 0x909647BA, 0x909647B9, 0x909647B8,
        0x909647BF, 0x909E47BE, 0x909E47BD, 0x909E47BC,
        0x909E47BA, 0x989E57BB, 0x98A657BF, 0x98A657BE,
        0x98A657BD, 0x98A657BB, 0x98A657BA, 0xA0A667BB,
        0xA0AE67BA, 0xA0AE67BF, 0xA8AE67BD, 0xA8AE67BB,
        0xA8AE67BA, 0xA8AE77BC, 0xA8AC77BF, 0xA8AC77BD,
        0xB0AC77BA, 0xB0AC77B8, 0xB0B477BF, 0xB0B477BC,
        0xB0B477BA, 0xB8BC77B8, 0xB8BC77BF, 0xC0BC77BC,
        0xC0BC77BA, 0xC0BC87BB, 0xC0BA87BE, 0xC0BA87BE,
        0xC0BA87BD, 0xC0BA87BC, 0xC0BA87BB, 0xC0BA87BA,
        0xC8B887BB, 0xC8B897BA, 0xC8B897BD, 0xC8B897BB,
        0xD0B897BA, 0xD0B8A7BD, 0xD0B8A7BC, 0xD0B8A7BB,
        0xD0BAB7BA, 0xD0B8B7BD, 0xD0B8B7B8, 0xD8B8B7B7,
        0xD8B8B7B6, 0xD8B8B7B5, 0xD8BAC7B4, 0xD8B8C7B6,
        0xE0B8C7B5, 0xE0BAD7B4, 0xE0B8D7B6, 0xE0B8D7B5,
        0xE0BAE7B4, 0xE0B8E7B6, 0xE0B8E7B4, 0xE8BAF7B3,
        0xE8B8F7B6, 0xE8B8F7B5, 0xE8BB07B4, 0xE8B907B6,
        0xE8B907B5, 0xE8BB17B4, 0xE8B917B6, 0xE8B917B5,
        0xE8BB27B4, 0xF0B927B6, 0xF0B927B4, 0xF0BB37B3,
        0xF0B937B6, 0xF8B937B3, 0xF8B937A8, 0xF8B937AF,
        0xF8B937AC, 0xF8BB47AB, 0xF8B947AE, 0xF8B947AD,
        0xF8BB57AC, 0xF8B957AE, 0xF8B957AE, 0xF8BB67AD,
        0xF8B967AF, 0xF8B967AF, 0xF8BB77AA, 0xF8B977AC,
        0xF8B977AB, 0xF8BB87AB, 0xF8B987AD, 0xF8B987AB,
        0xF8BB97AB, 0xF8B997AD, 0xF8B997AB, 0xF8BBA7AB,
        0xF8B9A7AD, 0xF8B9A7A8, 0xF8B9A7A7, 0xF8B9A7A5,
        0xF8B9A7A3, 0xF8BBB7A3, 0xF8B9B7A6, 0xF8B9B7A6,
        0xF8B9B7A5, 0xF8BBB7A3, 0xF8BDC7A3, 0xF8BBC7A6,
        0xF8BBC7A4, 0xF8BBC7A2, 0xF8B9C7A5, 0xF8B9C7A3,
        0xF8B9C7A0, 0xF8B9C79F, 0xF8B9C79E, 0xF8B9C79D,
        0xF8B9C79C, 0xF8B9C79B, 0xF8B9C79A, 0xF8B9C79A,
        0xF8B9C799, 0xF8B9C799, 0xF8B9C798, 0xF8B9C798},

    {   0x03062C3E, 0x03062C3C, 0x03062C3A, 0x03062C38,
        0x030E2C3F, 0x030E2C38, 0x03162C3F, 0x03162C3B,
        0x03162C38, 0x03163C3F, 0x03163C38, 0x031E3C3F,
        0x031E3C3C, 0x031E3C3A, 0x031E3C39, 0x031E3C38,
        0x031E4C3F, 0x031E4C3C, 0x031E4C3A, 0x031E4C38,
        0x03264C3F, 0x03264C3B, 0x03264C38, 0x032E4C3F,
        0x032E4C3B, 0x032E4C39, 0x032E4C38, 0x032E5C3F,
        0x032E5C3D, 0x032E5C3B, 0x032E5C38, 0x032E6C3F,
        0x032E6C38, 0x03366C3F, 0x03366C3C, 0x03366C3A,
        0x03366C38, 0x03367C3F, 0x03367C3D, 0x03367C3C,
        0x03367C3A, 0x03367C39, 0x03367C38, 0x0B3E7C3F,
        0x0B3E683F, 0x0B3E683B, 0x0B3E683A, 0x0B3E6838,
        0x0B3E6CBF, 0x133E6CBD, 0x133E6CBC, 0x133E6CBA,
        0x133E58B8, 0x133E68BD, 0x1B3E6D3F, 0x1B3E6D3D,
        0x1B3E6D3B, 0x1B3E6D3A, 0x1B3E6D38, 0x1B466D3F,
        0x1B466D3D, 0x23466D3C, 0x23466D3A, 0x23465938,
        0x23466938, 0x23466DBF, 0x23466DBC, 0x2B466DBA,
        0x2B466DB8, 0x2B4E6DBF, 0x2A465DB8, 0x2A466DBD,
        0x2A466DBB, 0x324E6DBD, 0x324E6DBB, 0x3A4E59B8,
        0x3A4E69B8, 0x3A4E6E3F, 0x3A4E6E3C, 0x3A4E6E3A,
        0x3A4E6E38, 0x3A566E3E, 0x3A566E38, 0x3A5E6E3F,
        0x425E6E3B, 0x425E6E38, 0x425E6A3E, 0x425E6A3D,
        0x425E6EBF, 0x4A5E6EBE, 0x4A5E6EBD, 0x4A5E6EBC,
        0x4A5E6EBB, 0x4A5E6EBA, 0x4A5E6EB9, 0x4A665EB9,
        0x4A665EB8, 0x526E5EBF, 0x526E5EBB, 0x526E5EB8,
        0x52765EBF, 0x52765EBE, 0x52765EBD, 0x52765EBC,
        0x5A765EBB, 0x5A765EBA, 0x5A765EB9, 0x5A765ABC,
        0x5A745ABF, 0x62764F38, 0x62765F38, 0x62745F3E,
        0x62745F3D, 0x62745F3C, 0x6A745F3B, 0x6A745F3A,
        0x6A745F38, 0x6A744B38, 0x6A745B3F, 0x6A6C5FB8,
        0x6A745FBF, 0x72745FBE, 0x72745FBD, 0x72745FBB,
        0x72745FBA, 0x72745FB9, 0x727E5FBF, 0x71765FB8,
        0x79865FBB, 0x798E5FBD, 0x798E5FBA, 0x79965FBE,
        0x78965FBE, 0x809E5FBF, 0x809E5FBB, 0x80A65FBD,
        0x88A65FBA, 0x88A66FBE, 0x88A66FBA, 0x90A67FBE,
        0x90A67FBB, 0x90A67BBE, 0x90A67BBD, 0x90A67BBB,
        0x98A67BBA, 0x98A67BB8, 0x98A68BBB, 0x98A687BD,
        0x98A687BB, 0xA0A687BA, 0xA0A687B8, 0xA0AE87BE,
        0xA0AE87BD, 0xA0AE87BC, 0xA0AE87BA, 0xA0AE87B9,
        0xA8AE87B8, 0xA8B687BE, 0xA8B687BD, 0xA8B687BB,
        0xA8B687BA, 0xA8B687B8, 0xB0BE87BE, 0xB0BE87BD,
        0xB0BE87BC, 0xB0BE87BA, 0xB0BE87B9, 0xB8BE87B8,
        0xB8BC87BB, 0xB8BC97BE, 0xB8BC97BB, 0xB8BC97BA,
        0xC0BC97B8, 0xC0BCA7BB, 0xC0BAA7BE, 0xC0BAA7BC,
        0xC0BAA7BA, 0xC8BAA7B9, 0xC8BAA7B8, 0xC8B8A7BC,
        0xC8BAB7BA, 0xC8B8B7BF, 0xC8B8B7BE, 0xC8B8B7BD,
        0xD0B8B7B7, 0xD0B8B7B6, 0xD0B8B7B5, 0xD8B8B7B5,
        0xD8BAC7B4, 0xD8B8C7B7, 0xD8B8C7B6, 0xD8BAD7B5,
        0xD8B8D7B7, 0xD8B8D7B7, 0xD8B8D7B6, 0xE0BAD7AD,
        0xE0BCE7AC, 0xE8BAE7AF, 0xE8BAE7AE, 0xE8BAE7AD,
        0xE8BCF7AB, 0xE8BAF7AE, 0xF0B8F7AE, 0xF0B8F7AD,
        0xF0BB07AB, 0xF0B907AE, 0xF0BB17AC, 0xF0B917AE,
        0xF8BB27AC, 0xF8BB37AD, 0xF8B937AE, 0xF8BB47AC,
        0xF8B947AD, 0xF8BB57AB, 0xF8B957AD, 0xF8B957AC,
        0xF8B957AB, 0xF8B967AD, 0xF8BB77AB, 0xF8B977AE,
        0xF8B977AD, 0xF8BB87AD, 0xF8B987AE, 0xF8B987AD,
        0xF8BB97AD, 0xF8B997AE, 0xF8B997AC, 0xF8BBA7AC,
        0xF8B9A7AE, 0xF8BBB7AE, 0xF8B9B7AF, 0xF8B9B7AE,
        0xF8B9B7AD, 0xF8BBC7AD, 0xF8B9C7AE, 0xF8B9C7AE,
        0xF8B9C7AD, 0xF8B9C7AD, 0xF8B9C7AC, 0xF8B9C7AC,
        0xF8B9C7AB, 0xF8B9C7AB, 0xF8B9C7AB, 0xF8B9C7AA,
        0xF8B9C7AA, 0xF8B9C7AA, 0xF8B9C7AA, 0xF8B9C7A9,
        0xF8B9C7A9, 0xF8B9C7A9, 0xF8B9C7A9, 0xF8B9C7A8,
        0xF8B9C7A8, 0xF8B9C7A8, 0xF8B9C7A8, 0xF8B9C7A8}
};

static reg_info_t pm4359_reg_tbl[] = {
    {"Quad Global Conf",         0x0000, READ_ONLY,   {1}, 0xFF, 0x00},
    {"Quad Rx Options",          0x0002, READ_WRITE,  {1}, 0xFF, 0x80},
    {"Quad Tx FRM Options",      0x0005, READ_WRITE,  {1}, 0xFF, 0x00},
    {"Quad CDCRC Conf",          0x0010, READ_WRITE,  {1}, 0xE6, 0x00},
    {"Quad RJAT N2 Control",     0x0016, READ_WRITE,  {1}, 0xFF, 0x00},
    {"Quad RJAT Conf",           0x0017, READ_WRITE,  {1}, 0x1F, 0x00},
    {"Quad TX-ELST Conf",        0x0020, READ_WRITE,  {1}, 0x03, 0x03},
    {"Quad RXCE Rx Dlink",       0x0028, READ_WRITE,  {1}, 0xFF, 0x20},
    {"Quad RXCE Rx Bit Sel",     0x0029, READ_WRITE,  {1}, 0xFF, 0x00},
    {"Quad BRIF FP Conf",        0x0031, READ_WRITE,  {1}, 0xFF, 0x20},
    {"Quad BTIF Tx Bit Offset",  0x0044, READ_WRITE,  {1}, 0x0F, 0x00},
    {"Quad T1-FRMR Conf",        0x0048, READ_WRITE,  {1}, 0xFE, 0x00},
    {"Quad IBCD Conf",           0x004C, READ_WRITE,  {1}, 0x0F, 0x00},
    {"Quad IBCD Act Code",       0x004E, READ_WRITE,  {1}, 0xFF, 0x00},
    {"Quad T1 XBAS Conf",        0x0054, READ_WRITE,  {1}, 0xFF, 0x00},
    {"Quad T1 XBOC Control",     0x0066, READ_WRITE,  {1}, 0x4F, 0x00},
    {"Quad TPSC Data Buffer",    0x006F, READ_WRITE,  {1}, 0xFF, 0x00},
    {"Quad RPSC Data Buffer",    0x0073, READ_WRITE,  {1}, 0xFF, 0x00},
    {"Quad T1 APRM Octet 2",     0x007B, READ_WRITE,  {1}, 0xFF, 0x1C},
    {"Quad E1-FRMR CRC Count",   0x009A, READ_ONLY,   {1}, 0xF8, 0x00},
    {"Quad E1-TRAN Int Enable",  0x0084, READ_ONLY,   {1}, 0x1F, 0x00},
    {"Quad E1-FRMR Alarm",       0x0093, READ_ONLY,   {1}, 0xEF, 0x00},
    {"Quad TDPR Hi Tx Thresh",   0x00A9, READ_WRITE,  {1}, 0x7F, 0x40},
    {"Quad TDPR Tx Data",        0x00AD, READ_WRITE,  {1}, 0xFF, 0x00},
    {"Quad RX-ELST Idle Code",   0x00B2, READ_WRITE,  {1}, 0xFF, 0x00},
    {"Quad RDLC Sec Addr Match", 0x00C5, READ_WRITE,  {1}, 0xFF, 0x00},
    {"Quad RLPS Eq Volt Ref",    0x00DC, READ_WRITE,  {1}, 0x3F, 0x00},
    {"Quad PRBS Generator",      0x00E0, READ_WRITE,  {1}, 0x2F, 0x00},
    {"Quad XLPG Conf",           0x00F0, READ_WRITE,  {1}, 0x9F, 0x00},
    {"Quad RLPS Eq Conf",        0x00FF, READ_WRITE,  {1}, 0xCF, 0x03},
    {"end",                      0x0000, 0, {0}, 0x0, 0x0},
};

cmq_framer_conf_t pmc4359_default_conf_info[] = {
    {0,                                   /* framer_num */
     CMQ_MODE_T1,                         /* op_mode */
     CMQ_FRM_MODE_T1_ESF,                 /* frm_mode */
     CMQ_T1_ZSUP_NONE,                    /* zcode_suppr_format */
     DISABLE,                             /* sf_align_en */
     CMQ_T1_OOF_2OF4,                     /* out_of_frame */
#ifdef ALW
     CMQ_T1_ESF_FRAME_ALGO_ONE_CANDIDATE, /* frm_esf_algo */
#else
     CMQ_T1_ESF_FRAME_ALGO_CRC_6,         /* frm_esf_algo */
#endif
     DISABLE,                             /* ccofa_en */
     INVALID,                             /* ts16Signaling */
     INVALID,                             /* nat_bit_en */
     INVALID,                             /* xtra_bit_en */
     INVALID,                             /* febee_en */
     INVALID,                             /* cas_align_en */
     INVALID,                             /* crc2ncrc_en */
     INVALID,                             /* no_refrm_err_en */
     INVALID,                             /* refrm_xs_crc_en */
     INVALID,                             /* los_bit_2crit_en */
     INVALID,                             /* mfrm_los_alig_crit */
     INVALID,                             /* ais_criteria */
     INVALID,                             /* rai_criteria */
     INVALID,                             /* mult_faseo_en */
     INVALID,                             /* nfas_err_en */
     ENABLE,                              /* rjat_byp_en */
#ifdef ALW	/* config bp intf, 0x30, 0x31, 0x40, 0x41 */
     ENABLE,                              /* rx_elst_byp */
#else
     DISABLE,                             /* rx_elst_byp */
#endif
     ENABLE,                              /* tjat_byp_en */
#ifdef ALW
     CMQ_TJAT_DIV_N1_CTL_1544_1544,      /* tjat_ref_div */
     CMQ_TJAT_DIV_N2_CTL_1544_1544,      /* tjat_output_div */
#else
     CMQ_TJAT_DIV_N1_CTL_G1544_1544,     /* tjat_ref_div */
     CMQ_TJAT_DIV_N2_CTL_G1544_1544,     /* tjat_output_div */
#endif
     DISABLE,                            /* tjat_limit_ov_under */
     ENABLE,                             /* tjat_fifo_cent */
     CMQ_RJAT_DIV_NX_CTL_T1,             /* rjat_ref_div   */
     CMQ_RJAT_DIV_NX_CTL_T1,             /* rjat_output_div */
     DISABLE,                            /* rjat_limit_ov_under */
     ENABLE,                             /* rjat_fifo_cent */
     CMQ_TJAT_OUTPUT_CLK_INTERN_JAT,     /* out_clock */
#ifdef ALW
     CMQ_TJAT_PLL_REF_CLK_CTCLK,         /* pll_ref_clock */
#else
     CMQ_TJAT_PLL_REF_CLK_BACKPLANE,     /* pll_ref_clock */
#endif
     ENABLE,                             /* tx_elst_byp */
     0,                                  /* recover_clk_sel */
     CMQ_CDRC_CFG_LOS_T1_PCM15,          /* los_thres */
#ifdef ALW
     ENABLE,                             /* brif_mas_mode */
#else
     DISABLE,                            /* brif_mas_mode */
#endif
     CMQ_BACKPLANE_FULL_FRAME_MODE,      /* brif_data_mode */
     DISABLE,                            /* brif_clkx2 */
#ifdef ALW
     CMQ_BACKPLANE_CLK_RATE_1544,        /* brif_data_rate */
     DISABLE,                            /* brif_de_hi */
#else
     CMQ_BACKPLANE_CLK_RATE_2048,        /* brif_data_rate */
     ENABLE,                             /* brif_de_hi */
#endif
     DISABLE,                            /* brif_fe_hi */
     CMQ_XCLK_2048_TXCLK_1544,           /* synth_tx_freq */
#ifdef ALW
     ENABLE,                             /* btif_mas_mode */
#else
     DISABLE,                            /* btif_mas_mode */
#endif
     CMQ_BACKPLANE_FULL_FRAME_MODE,      /* btif_data_mode */
     DISABLE,                            /* btif_clkx2 */
#ifdef ALW
     CMQ_BACKPLANE_CLK_RATE_1544,        /* btif_data_rate */
     ENABLE,                             /* btif_de_hi */
#else
     CMQ_BACKPLANE_CLK_RATE_2048,        /* btif_data_rate */
     DISABLE,                            /* btif_de_hi */
#endif
     DISABLE,                            /* btif_fe_hi */
#ifdef ALW
     ENABLE,                             /* r_fp_mas_mode */
#else
     DISABLE,                            /* r_fp_mas_mode */
#endif
     CMQ_BACKPLANE_RX_FP_T1E1_HIGH_EVERY_FRAME, /* r_fp_mode */
     DISABLE,                            /* r_fp_inv_en */
     DISABLE,                            /* r_alt_fdl_en */
#ifdef ALW
     CMQ_BACKPLANE_TIMESLOT_MAP_3_OF_4,  /* r_tslot_map_format */
#else
     CMQ_BACKPLANE_TIMESLOT_MAP_24_OF_32,/* r_tslot_map_format */
#endif
     DISABLE,                            /* r_par_ins_en */
     DISABLE,                            /* r_odd_par */
     DISABLE,                            /* r_ext_par_en */
     DISABLE,                            /* r_fbit_fix */
     DISABLE,                            /* r_fbit_pol */
     0,                                  /* r_fp_frm_offset */
     DISABLE,                            /* r_fp_bit_offset_en */
     0,                                  /* r_fpBitOffset */
#ifdef ALW
     ENABLE,                             /* t_fp_mas_mode */
#else
     DISABLE,                            /* t_fp_mas_mode */
#endif
     DISABLE,                            /* t_fp_inv_en */
     DISABLE,                            /* t_t1_esf_align */
#ifdef ALW
     CMQ_BACKPLANE_TIMESLOT_MAP_3_OF_4,  /* t_tslot_map_format */
#else
     CMQ_BACKPLANE_TIMESLOT_MAP_24_OF_32,/* t_tslot_map_format */
#endif
     DISABLE,                            /* t_odd_par */
     DISABLE,                            /* t_ext_par_en */
     0,                                  /* t_fp_frm_offset */
     DISABLE,                            /* t_fp_bitoffset_en */
     0,                                  /* t_fp_bitoffset */
     DISABLE,                            /* squelch_en */
     CMQ_RX_ALOS_9DB_THRESH,             /* alos_thres */
     1,                                  /* alos_det_period */
     1,                                  /* alos_clr_period */
     DISABLE,                            /* xlpg_highz_en */
     CMQ_TX_LBO_T1_SHORT_HAUL_110FT,     /* xlpg_line_drv_val */
    },
    {0,                                   /* framer_num */
     CMQ_MODE_E1,                         /* op_mode */
     CMQ_FRM_MODE_E1,                     /* frm_mode */
     INVALID,                             /* zcode_suppr_format */
     DISABLE,                             /* sf_align_en */
     INVALID,                             /* out_of_frame */
     INVALID,                             /* frm_esf_algo */
     INVALID,                             /* ccofa_en */
     CMQ_E1_SIG_INS_NONE,                 /* ts16Signaling */
     DISABLE,                             /* nat_bit_en */
     DISABLE,                             /* xtra_bit_en */
     DISABLE,                             /* febee_en */
     DISABLE,                             /* cas_align_en */
     DISABLE,                             /* crc2ncrc_en */
     DISABLE,                             /* no_refrm_err_en */
     DISABLE,                             /* refrm_xs_crc_en */
     DISABLE,                             /* los_bit_2crit_en */
     DISABLE,                             /* mfrm_los_alig_crit */
     DISABLE,                             /* ais_criteria */
     DISABLE,                             /* rai_criteria */
     DISABLE,                             /* mult_faseo_en */
     DISABLE,                             /* nfas_err_en */
     ENABLE,                              /* rjat_byp_en */
#ifdef ALW	/* config bp intf, 0x30, 0x31, 0x40, 0x41 */
     ENABLE,                              /* rx_elst_byp */
#else
     DISABLE,                             /* rx_elst_byp */
#endif
     ENABLE,                              /* tjat_byp_en */
     CMQ_TJAT_DIV_N1_CTL_2048_2048,      /* tjat_ref_div */
     CMQ_TJAT_DIV_N2_CTL_2048_2048,      /* tjat_output_div */
     DISABLE,                            /* tjat_limit_ov_under */
     ENABLE,                             /* tjat_fifo_cent */
     CMQ_RJAT_DIV_NX_CTL_E1,             /* rjat_ref_div   */
     CMQ_RJAT_DIV_NX_CTL_E1,             /* rjat_output_div */
     DISABLE,                            /* rjat_limit_ov_under */
     ENABLE,                             /* rjat_fifo_cent */
     CMQ_TJAT_OUTPUT_CLK_INTERN_JAT,     /* out_clock */
#ifdef ALW
     CMQ_TJAT_PLL_REF_CLK_CTCLK,         /* pll_ref_clock */
#else
     CMQ_TJAT_PLL_REF_CLK_BACKPLANE,     /* pll_ref_clock */
#endif
     ENABLE,                             /* tx_elst_byp */
     0,                                  /* recover_clk_sel */
     CMQ_LOS_THRESH_PCM_10_HDB3,         /* los_thres */
#ifdef ALW
     ENABLE,                             /* brif_mas_mode */
#else
     DISABLE,                            /* brif_mas_mode */
#endif
     CMQ_BACKPLANE_FULL_FRAME_MODE,      /* brif_data_mode */
     DISABLE,                            /* brif_clkx2 */
     CMQ_BACKPLANE_CLK_RATE_2048,        /* brif_data_rate */
#ifdef ALW
     DISABLE,                            /* brif_de_hi */
#else
     ENABLE,                             /* brif_de_hi */
#endif
     DISABLE,                            /* brif_fe_hi */
     CMQ_XCLK_2048_TXCLK_2048,           /* synth_tx_freq */
#ifdef ALW
     ENABLE,                             /* btif_mas_mode */
#else
     DISABLE,                            /* btif_mas_mode */
#endif
     CMQ_BACKPLANE_FULL_FRAME_MODE,      /* btif_data_mode */
     DISABLE,                            /* btif_clkx2 */
     CMQ_BACKPLANE_CLK_RATE_2048,        /* btif_data_rate */
#ifdef ALW
     ENABLE,                             /* btif_de_hi */
#else
     DISABLE,                            /* btif_de_hi */
#endif
     DISABLE,                            /* btif_fe_hi */
#ifdef ALW
     ENABLE,                             /* r_fp_mas_mode */
#else
     DISABLE,                            /* r_fp_mas_mode */
#endif
     CMQ_BACKPLANE_RX_FP_T1E1_HIGH_EVERY_FRAME, /* r_fp_mode */
     DISABLE,                            /* r_fp_inv_en */
     DISABLE,                            /* r_alt_fdl_en */
     INVALID,                            /* r_tslot_map_format */
     DISABLE,                            /* r_par_ins_en */
     DISABLE,                            /* r_odd_par */
     DISABLE,                            /* r_ext_par_en */
     DISABLE,                            /* r_fbit_fix */
     DISABLE,                            /* r_fbit_pol */
     0,                                  /* r_fp_frm_offset */
     DISABLE,                            /* r_fp_bit_offset_en */
     0,                                  /* r_fpBitOffset */
#ifdef ALW
     ENABLE,                             /* t_fp_mas_mode */
#else
     DISABLE,                            /* t_fp_mas_mode */
#endif
     DISABLE,                            /* t_fp_inv_en */
     INVALID,                            /* t_t1_esf_align */
     INVALID,                            /* t_tslot_map_format */
     DISABLE,                            /* t_odd_par */
     DISABLE,                            /* t_ext_par_en */
     0,                                  /* t_fp_frm_offset */
     DISABLE,                            /* t_fp_bitoffset_en */
     0,                                  /* t_fp_bitoffset */
     DISABLE,                            /* squelch_en */
     CMQ_RX_ALOS_9DB_THRESH,             /* alos_thres */
     1,                                  /* alos_det_period */
     1,                                  /* alos_clr_period */
     DISABLE,                            /* xlpg_highz_en */
     CMQ_TX_LBO_E1_75OHM,                /* xlpg_line_drv_val */
    },
};


/*****************************************************************
 *
 * Name: pmc4359_dev_create()
 *
 * Description: Create object with various device function
 * point to "do nothing"
 *
 * Input: dev_object_t pointer to the PMC Framer device
 *
 * Returns: none
 *
 *****************************************************************/
void
pmc4359_dev_create (dev_object_t *dev, dev_error_report_t error_report_fn)
{
    dev_4359_object_t *pmc4359 = (dev_4359_object_t *)dev;
    
    pmc4359->base.dev_state = DEV_STATE_CREATE;

    /* Init the device object structure to default "do nothing" */
    init_default_dev_object(dev, &pmc4359_fvt);
    pmc4359->base.dev_object_fvt->dev_attach = pmc4359_dev_attach;
    pmc4359->base.dev_object_fvt->dev_init = pmc4359_dev_init;    
    pmc4359->base.dev_object_fvt->dev_error_report = error_report_fn;
    pmc4359->base.dev_object_fvt->dev_show = pmc4359_dev_show;
    pmc4359->base.dev_object_fvt->dev_destroy = pmc4359_destroy;
}

/*****************************************************************
 *
 * Name: pmc4359_dev_attach()
 *
 * Description: Attach the PMC Comet Tetra 4359 device for use. This
 *   function will initialize and setup all necessary pointers
 *   and bring the chip to operation.
 *
 * Input: Pointer to the PMC 4359 device object
 *
 * Returns: PASSED
 *
 *****************************************************************/
static uint32
pmc4359_dev_attach (dev_object_t *dev)
{
    dev_4359_object_t *pmc4359 = (dev_4359_object_t *)dev;

    /* Init configuration info default to T1 */
    pmc4359_set_cfg_info(dev, 0, MODE_T1);

    /* 
     * initialize the call in functions. These are the only mean the
     * outside world can communicate with this chip.
     */
    pmc4359->callin_fvt = &pmc4359_callin;
    pmc4359->callin_fvt->register_test = pmc4359_reg_test;
    pmc4359->callin_fvt->dev_reset     = pmc4359_dev_reset;
    pmc4359->callin_fvt->set_cfg_info  = pmc4359_set_cfg_info;
    pmc4359->callin_fvt->set_loopback  = pmc4359_set_loopback;
    pmc4359->callin_fvt->rd_ind_reg    = pmc4359_rd_ind_reg;
    pmc4359->callin_fvt->wr_ind_reg    = pmc4359_wr_ind_reg;
    pmc4359->callin_fvt->init_xpsc     = pmc4359_init_xpsc;
    pmc4359->callin_fvt->dump_xpsc     = pmc4359_dump_xpsc;
    pmc4359->callin_fvt->read_rlps     = pmc4359_read_rlps;
    pmc4359->callin_fvt->ycable_enab   = pmc4359_ycable_enab;
    
    /* init the call out function, continue from the caller */
    pmc4359->callout_fvt = &pmc4359_callout;

    pmc4359->base.dev_state = DEV_STATE_ATTACH;

    return (PASSED);
}

/*****************************************************************
 * Name: pmc4359_destroy
 *
 * Description:	Destroy the dev_object structure and free all the resources.
 *
 * Input: dev_object_t pointer to the PMC 4359 device object
 *
 * Returns: none
 *
 * Assumptions: The dev_attch() function has been called and successfully
 *
 *****************************************************************/
static void
pmc4359_destroy(dev_object_t **dev)
{
    uint32_t rc;
    dev_4359_object_t *pmc4359 = (dev_4359_object_t *)*dev;
    char err_buf[80];

    if (dev == NULL) {
	return;
    }

    if (*dev == NULL) {
	return;
    }

    if (pmc4359->callout_fvt) {
	free(pmc4359->callout_fvt);	/* Free callout struct */
	pmc4359->callout_fvt = NULL;
    }

    if (pmc4359->callin_fvt) {
	free(pmc4359->callin_fvt);	/* Free callin struct */
	pmc4359->callin_fvt = NULL;
    }

    if (pmc4359->cfg_info_p) {
	free(pmc4359->cfg_info_p);	/* Free cfg_info_p struct */
	pmc4359->cfg_info_p = NULL;
    }

    if (pmc4359->base.dev_object_fvt) {
	free(pmc4359->base.dev_object_fvt);	/* Free dev_object_t */
	pmc4359->base.dev_object_fvt = NULL;
    }
}

/*****************************************************************
 *
 * Name: pmc4359_dev_init()
 *
 * Description: Initializes the framer chip with information specified
 *              in pmc4359_default_conf_info.
 *    
 * Input: dev_object_t pointer to the PMC 4359 device 
 *
 * Returns: PASSED/FAILED
 *
 * Note: Make sure base.dev_addr has been initialized to chip_base_addr
 *       before calling this function.
 *
 *****************************************************************/
static uint32
pmc4359_dev_init (dev_object_t *dev)
{
    ulong frm_bar;
    uchar op_mode, val, ram_type, frm_num;
    cmq_framer_conf_t *conf_info_p;
    dev_4359_object_t *pmc4359 = (dev_4359_object_t *)dev;
    dev_4359_callout_fvt_t *callout_p = pmc4359->callout_fvt;

    pmc4359->base.dev_state = DEV_STATE_INIT;

    conf_info_p = pmc4359->cfg_info_p;
    op_mode = conf_info_p->op_mode;
    frm_num = conf_info_p->quad_num;
    ram_type = (op_mode == CMQ_MODE_T1) ? CMQ_RX_LINE_EQ_RAM_T1 :
                                           CMQ_RX_LINE_EQ_RAM_E1;
    frm_bar = (ulong)pmc4359->base.dev_addr + (frm_num * CMQ_FRM_OFFSET * 
					       pmc4359->bus_width);

    /* only init the framer TPSC and RPSC indirect registers if framer 0 */
    if (frm_num == 0) {
        pmc4359_init_xpsc(dev, frm_num, CMQ_TPSC_CFG);
        pmc4359_init_xpsc(dev, frm_num, CMQ_RPSC_CFG);
    }

    /* Write to framer CMQ_TPSC_CFG and CMQ_RPSC_CFG register */
    callout_p->wr_frm_reg(frm_bar, CMQ_TPSC_CFG, CMQ_TPSC_CFG_IND_ACCESS, 
			  pmc4359->bus_width);
    callout_p->wr_frm_reg(frm_bar, CMQ_RPSC_CFG, CMQ_RPSC_CFG_IND_ACCESS, 
			  pmc4359->bus_width);

    /*
     * set Operation Mode / elastic
     */
    if (pm4359_set_operation(dev))
	return(FAILED);

    /*
     * configure clock sync unit,  0xd6
     */
    pm4359_clk_sync_conf(dev);

    /*
     * configure line decoder  0x10
     */
    pm4359_rx_clk_conf(dev);

    /*
     * configure framer   0x02, 0x05, 0x80, 0x90, 0x91, 0x50, 0x60, 0x48, 0x54
     */
    if (op_mode == CMQ_MODE_T1) {
        pm4359_t1_tx_conf(dev);
        pm4359_t1_rx_conf(dev);

        val = callout_p->rd_frm_reg(frm_bar, CMQ_T1_FRMR_CFG, pmc4359->bus_width);
#ifdef DELETE_FOR_NGD
        printf("\nCMQ_T1_FRMR_CFG @offset %#x = %#x\n", CMQ_T1_FRMR_CFG, val);
#endif

	/* Clear IR and OR bits in register 0x0n74 for T1 operation */
	val = callout_p->rd_frm_reg(frm_bar, CMQ_DS0_DDLB_ELST_CFG,
				    pmc4359->bus_width) & 
	    ~(DS0_DDLB_ELST_IR | DS0_DDLB_ELST_OR);
	callout_p->wr_frm_reg(frm_bar, CMQ_DS0_DDLB_ELST_CFG, val,
			      pmc4359->bus_width);

	/* Set term cntrl bits in register 0x0nBE for T1 operation */
	callout_p->wr_frm_reg(frm_bar, CMQ_TERM_CNTRL, CMQ_TERM_CNTRL_RXTERM_EN,
			      pmc4359->bus_width);
    } else {
        pm4359_e1_tx_conf(dev);
        pm4359_e1_rx_conf(dev);

	/* Set IR and OR bits in register 0x0n74 for E1 operation */
	val = callout_p->rd_frm_reg(frm_bar, CMQ_DS0_DDLB_ELST_CFG,
				    pmc4359->bus_width) | 
	    (DS0_DDLB_ELST_IR | DS0_DDLB_ELST_OR);
	callout_p->wr_frm_reg(frm_bar, CMQ_DS0_DDLB_ELST_CFG, val,
			      pmc4359->bus_width);

	/* Set term cntrl bits in register 0x0nBE for E1 operation */
	callout_p->wr_frm_reg(frm_bar, CMQ_TERM_CNTRL,
			      CMQ_TERM_CNTRL_RXTERM_EN |
			      CMQ_TERM_CNTRL_RXTERM_E1 |
			      CMQ_TERM_CNTRL_RXTERM_E1_75,
			      pmc4359->bus_width);
    }

    /*
     * configure jitter attenuators tx timing options
     */
    pm4359_tx_jat_conf(dev);
    pm4359_rx_jat_conf(dev);

    /*
     * configure backplane interface, 0x30 0x40
     */
    pm4359_bp_clk_conf(dev);
    pm4359_bp_fp_conf(dev);
    pm4359_rlps_volt(dev);

    /*
     * configure rx equalizer RAM
     */
    pm4359_conf_rlps_ram(dev, pm4359_rlps_ram[ram_type]);
    
    /*
     * configure receive line interface 0xF8 ... 0xFF
     */
    pm4359_rx_line_conf(dev);

    /*
     * configure tx pulse template 0xF0
     */
    callout_p->wr_frm_reg(frm_bar, CMQ_XLPG_LINE_DRV_CFG, 
			  CMQ_XLPG_LINE_DRV_HIGHZ_EN, pmc4359->bus_width);
    val = callout_p->rd_frm_reg(frm_bar, CMQ_XLPG_LINE_DRV_CFG, 
				pmc4359->bus_width) | 
	pm4359_xlpg_wvfmscale[conf_info_p->xlpg_line_drv_val];
    callout_p->wr_frm_reg(frm_bar, CMQ_XLPG_LINE_DRV_CFG, val, 
			  pmc4359->bus_width);

#ifdef XDEBUG
    printf("\nxlpg line drive value %#.2x\n", 
	   pm4359_xlpg_wvfmscale[conf_info_p->xlpg_line_drv_val]);
#endif
    /*
     * set amplitude of pulse template and enable transmission
     */
    pm4359_linetx_analog_conf(dev);
    pm4359_rlps_opti(dev);

    if (!conf_info_p->xlpg_highz_en) {
	val = callout_p->rd_frm_reg(frm_bar, CMQ_XLPG_LINE_DRV_CFG, 
				    pmc4359->bus_width) & 
	    ~CMQ_XLPG_LINE_DRV_HIGHZ_EN;
	callout_p->wr_frm_reg(frm_bar, CMQ_XLPG_LINE_DRV_CFG, val, 
			      pmc4359->bus_width);
    }
    return(PASS);
}



/**********************************************************************
 *
 * Function: pmc4359_set_loopback()
 *
 * This function clears/sets a loopback of type line, payload, digital,
 *                or per channel loopback.
 *                Only one loop type setting is valid.
 *
 * Input : dev       - Pointer to PMC framer device object.
 *         frm_num   - Port Number
 *         loop_mode - loopback type
 *                     No Loopback      - CMQ_MST_DIAG_LP_NONE
 *                     Digital Loopback - CMQ_MST_DIAG_DIG_LPBCK
 *                     Line Loopback    - CMQ_MST_DIAG_LINE_LPBCK
 *                     Payload Loopback - CMQ_MST_DIAG_DIG_LPBCK
 *                     
 *         
 * Output: NONE
 *
 **********************************************************************
 */
static void 
pmc4359_set_loopback (dev_object_t *dev, uchar frm_num, uchar loop_mode)
{
    uchar val;
    ulong chip_bar, frm_bar;
    dev_4359_object_t *pmc4359 = (dev_4359_object_t *)dev;
    dev_4359_callout_fvt_t *callout_p = pmc4359->callout_fvt;

    chip_bar = (ulong)pmc4359->base.dev_addr;
    frm_bar = chip_bar + (frm_num * CMQ_FRM_OFFSET * pmc4359->bus_width);
    val = callout_p->rd_frm_reg(frm_bar, CMQ_MST_DIAG, pmc4359->bus_width);
    val = (val & ~(CMQ_MST_DIAG_PAYLD_LPBCK | CMQ_MST_DIAG_LINE_LPBCK |
		   CMQ_MST_DIAG_DIG_LPBCK)) | loop_mode;
    callout_p->wr_frm_reg(frm_bar, CMQ_MST_DIAG, val, pmc4359->bus_width);

    if (loop_mode == CMQ_MST_DIAG_LINE_LPBCK) {
        /* 
         * setup jitter attenuator clock dividers, offset: 0x19, 0x1A
         */
	callout_p->wr_frm_reg(chip_bar, CMQ_TJAT_DIV_N1_CTL, 0x2F, 
			      pmc4359->bus_width);
	callout_p->wr_frm_reg(chip_bar, CMQ_TJAT_DIV_N2_CTL, 0x2F, 
			      pmc4359->bus_width);

        /*
         * clear the transmit timing options register to give us jitter
         * attenuated loop timing with the TX-Elastic store enabled
         */
	callout_p->wr_frm_reg(chip_bar, CMQ_TX_TIMING_OPTS, 0, 
			      pmc4359->bus_width);
    }
    if (loop_mode == CMQ_MST_DIAG_ALOOP) {
	/* for y-cable testing */
	callout_p->wr_frm_reg(frm_bar, CMQ_TERM_CNTRL, 0x0, 
			      pmc4359->bus_width);
	callout_p->wr_frm_reg(frm_bar, CMQ_XLPG_LINE_DRV_CFG, 0x40, 
			      pmc4359->bus_width);
    }
}

/**********************************************************************
 *
 * Function: pmc4359_ycable_enab()
 *
 * This function enable or disable Y Cable feature on the PMC Framer.
 * This is on a per port basis.
 *
 * Input : dev       - Pointer to PMC framer device object.
 *         frm_num   - Port Number
 *         enable    - TRUE to enable Y Cable, FALSE otherwise
 *         
 * Output: NONE
 *
 **********************************************************************
 */
static void 
pmc4359_ycable_enab (dev_object_t *dev, uchar frm_num, ulong enable)
{
    uchar val;
    ulong frm_bar;
    dev_4359_object_t *pmc4359 = (dev_4359_object_t *)dev;
    dev_4359_callout_fvt_t *callout_p = pmc4359->callout_fvt;

    frm_bar = (ulong)pmc4359->base.dev_addr + (frm_num * CMQ_FRM_OFFSET * 
					       pmc4359->bus_width);
    val = callout_p->rd_frm_reg(frm_bar, CMQ_MST_DIAG, pmc4359->bus_width);

    /* Set bit to enable Y Cable */
    if (enable)
	val |= CMQ_MST_DIAG_ALOOP;
    else
	val &= ~CMQ_MST_DIAG_ALOOP;

    callout_p->wr_frm_reg(frm_bar, CMQ_MST_DIAG, val, pmc4359->bus_width);
}


/**********************************************************************
 *
 * Function: pmc4359_reg_test()
 *
 * This function: tests Comet Tetra Framer internal registers
 *
 * If an error occurs, check bit 3 (CSU lock) of the CSU config
 * register to see if the bit is set.  If it is set, then the
 * CSU unit has achieved phase and frequency lock to XCLK.
 * This bit was taken out by PMC since it is unstable and would
 * sometimes return CSU_LOCK = 0 even when the CSU was locked.
 * The read of this register was put in just to see if the CSU
 * may be unlocked when a register r/w error occurs.  However,
 * since this bit may be unstable, the value of this bit at the
 * time of the r/w error may not prove anything.
 *
 * Input : dev - Pointer to the framer device object
 *         num_ports: 1, 2, 4
 *
 * Output: PASSED or offset of failing register
 *
 **********************************************************************
 */
static int 
pmc4359_reg_test (dev_object_t *dev, int num_ports)
{
    ulong i, data, base_addr, count, offset;
    uchar temp, readval, rdval;
    reg_info_t *reg_ptr;
    dev_4359_object_t *pmc4359 = (dev_4359_object_t *)dev;
    dev_4359_callout_fvt_t *callout_p = pmc4359->callout_fvt;

    offset = 0;
    base_addr = (ulong)pmc4359->base.dev_addr;
    for (count = 0; count < num_ports; count++) {
	printf("\nFramer register test for port = %d, base_addr = %#x, "
	       "offset = %#x", count, base_addr, offset);
	reg_ptr = &pm4359_reg_tbl[0];
	while (reg_ptr->size.size != 0) {
        /*
         * Test a register if it's a R/W register
         */
        if (reg_ptr->type == READ_WRITE) {
            /* 
             * ripple 1 test
             */
            for (i = 0; i < (reg_ptr->size.size * 8); i++) {
                temp = (1 << i) & reg_ptr->mask;
                if (!temp)
                    continue;

		/* Write to register under test */
		callout_p->wr_frm_reg(base_addr, reg_ptr->offset + offset, temp,
				      pmc4359->bus_width);
		/* Read back */
		readval = callout_p->rd_frm_reg(base_addr,
						reg_ptr->offset + offset,
						pmc4359->bus_width);
                if ((readval & reg_ptr->mask) != temp) {
                    rdval = callout_p->rd_frm_reg(base_addr, CMQ_CSU_CFG,
                                                  pmc4359->bus_width);
                    sprintf(log_buffer, "Ripple one test failed when accessing "
			"%s Register offset %#x\n"
			"base_addr %#x, Expect %#x, Read %#x, CSU config %#x ",
			reg_ptr->name, reg_ptr->offset + offset, base_addr,
			temp, readval, rdval);
		    (*pmc4359->base.dev_object_fvt->dev_error_report)
			(&pmc4359->base, log_buffer, FATAL);
                    return(reg_ptr->offset + offset);
                }
            }

            /* 
             * ripple 0 test
             */
            for (i = 0; i < (reg_ptr->size.size * 8); i++) {
                temp = (1 << i) & reg_ptr->mask;
                if (!temp)
                    continue;
                temp = (~(1 << i)) & reg_ptr->mask;
		/* Write to register under test */
		callout_p->wr_frm_reg(base_addr, reg_ptr->offset + offset, temp,
				      pmc4359->bus_width);
		/* Read back */
		readval = callout_p->rd_frm_reg(base_addr,
						reg_ptr->offset + offset,
						pmc4359->bus_width);

                if ((readval & reg_ptr->mask) != temp) {
                    rdval = callout_p->rd_frm_reg(base_addr, CMQ_CSU_CFG,
                                                  pmc4359->bus_width);
                    sprintf(log_buffer, "Ripple zero test failed when accessing"
			" %s Register offset %#x\n"
			"base_addr %#x, Expect %#x, Read %#x, CSU config %#x",
			reg_ptr->name, reg_ptr->offset + offset, base_addr,
			temp, readval, rdval);
		    (*pmc4359->base.dev_object_fvt->dev_error_report)
			(&pmc4359->base, log_buffer, FATAL);
		    return(reg_ptr->offset + offset);
                }
             }

	    /*
	     * pattern test
	     */
	    data = PATTERN;
	    for (i = 0; i < 2; i++){
		temp = data &reg_ptr->mask;
		/* Write to register under test */
		callout_p->wr_frm_reg(base_addr, reg_ptr->offset + offset, temp,
				      pmc4359->bus_width);
		/* Read back */
		readval = callout_p->rd_frm_reg(base_addr,
						reg_ptr->offset + offset,
						pmc4359->bus_width);
		if ((readval & reg_ptr->mask) != temp) {
                    rdval = callout_p->rd_frm_reg(base_addr, CMQ_CSU_CFG,
                                                  pmc4359->bus_width);
		    sprintf(log_buffer, "Pattern test failed when accessing %s "
			"Register offset %#x\n"
			"base_addr %#x, Expect %#x, Read %#x, CSU config %#x ",
			reg_ptr->name, reg_ptr->offset + offset, base_addr,
			temp, readval, rdval);
		    (*pmc4359->base.dev_object_fvt->dev_error_report)
			(&pmc4359->base, log_buffer, FATAL);
		    return(reg_ptr->offset + offset);
		}

		data = ~PATTERN; /* complement data pattern */
	    }
	    
	     /*
	      * restore reset value
	      */
	    callout_p->wr_frm_reg(base_addr, reg_ptr->offset + offset, 
				  reg_ptr->reset_val, pmc4359->bus_width);
         }
         reg_ptr++;
      }
      if (num_ports == 2) {
        offset = 0x300;
      } else {
        offset += 0x100;
      }
    }

    /*
     * reset the framer
     */
    pmc4359_dev_reset(dev);

    return(PASSED);
}


/**********************************************************************
 *
 * Function: pmc4359_dev_reset()
 *
 * This function: resets the Comet Tetra Framer chip
 *
 * Input : dev - Pointer to the framer device object
 *         
 * Output: NONE
 *
 **********************************************************************
 */
static void 
pmc4359_dev_reset (dev_object_t *dev)
{
    uchar val;
    ulong chip_bar;
    dev_4359_object_t *pmc4359 = (dev_4359_object_t *)dev;
    dev_4359_callout_fvt_t *callout_p = pmc4359->callout_fvt;

    chip_bar = (ulong)pmc4359->base.dev_addr;

    /*
     * set then unset the Master RESET bit
     */
    callout_p->wr_frm_reg(chip_bar, CMQ_RESET, CMQ_RESET_CHIP, 
			  pmc4359->bus_width);
    callout_p->wr_frm_reg(chip_bar, CMQ_RESET, ~CMQ_RESET_CHIP, 
			  pmc4359->bus_width);

    /*
     * program the ouptut PIN enable
     */
    val = callout_p->rd_frm_reg(chip_bar, CMQ_GLB_CFG, pmc4359->bus_width);
    callout_p->wr_frm_reg(chip_bar, CMQ_GLB_CFG, val | CMQ_GLBL_PGM_IO_EN,
			  pmc4359->bus_width);
}


/**********************************************************************
 *
 * Function: pmc4359_set_cfg_info()
 *
 * This function set the configuration info for the framer to the
 * given port and operating mode. All subsequent operation will
 * be operated on this port and at this mode.
 *
 * Input : dev  - Pointer to the framer device object
 *         mode - MODE_T1 for T1 mode or MODE_E1 for E1
 *         port - port number to set to
 *         
 * Output: NONE
 *
 **********************************************************************
 */
static void 
pmc4359_set_cfg_info (dev_object_t *dev, uchar port, uchar mode)
{
    volatile uchar rdval;
    dev_4359_object_t *pmc4359 = (dev_4359_object_t *)dev;

    /* Boundary check */
    mode = (mode < CMQ_MODE_E1) ? mode : CMQ_MODE_E1;

    pmc4359->cfg_info_p = &pmc4359_default_conf_info[mode];
    pmc4359->cfg_info_p->quad_num = port;
#ifndef DELETE_FOR_NGD
    rdval = pmc4359->cfg_info_p->quad_num;
#endif

}


/**********************************************************************
 *
 * Function: pmc4359_dump_xpsc
 *
 * This function: displays the specified indirect registers
 *
 * Input : dev - Pointer to the Framer device object
 *         frm_num        - framer number
 *         reg            - start of indrect registers
 *         
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int 
pmc4359_dump_xpsc (dev_object_t *dev, ulong conf_reg_addr, ushort offset)
{
    dev_4359_object_t *pmc4359 = (dev_4359_object_t *)dev;
    ulong i, lower_byte, max_reg;
    uchar val;

    /*
     * Sanity check to make sure the indirect regions requested are
     * TPSC, RPSC or SIGX
     */
    lower_byte = conf_reg_addr & 0xFF;
    if ((lower_byte != CMQ_TPSC_CFG) && (lower_byte != CMQ_RPSC_CFG) &&
        (lower_byte != CMQ_SIGX_CFG_CHG_SIG_STATE)) {
        sprintf(log_buffer, "conf_reg_addr at 0x%x is not a valid region\n",
                conf_reg_addr);
        (*pmc4359->base.dev_object_fvt->dev_error_report)(&pmc4359->base,
                                                          log_buffer, FATAL);
        return(FAILED);
    }

#ifdef IDEBUG
    printf("\npmc4359_dump_xpsc, conf_reg addr: %#x, offset %#x\n",
	conf_reg_addr, offset);
#endif
    if (lower_byte == CMQ_SIGX_CFG_CHG_SIG_STATE) {
        max_reg = PM4359_SIGX_NUM_IND_REG;
    } else {
        max_reg = PM4359_NUM_IND_REG;
    }

    /* Start dumping */
    printf("\nIndirect Reg %#.2x\noffset %#.2x  ",
	conf_reg_addr & 0x3ff, offset);
    for (i = 0; i < max_reg; i++) {
        if (pmc4359_rd_ind_reg(dev, conf_reg_addr, offset + i, &val)) {
            return(FAILED);
	}
        printf("%#.2x ", val);
	if ((((offset + i) & 7) == 7) && (i < max_reg - 1))
	    printf("\noffset %#.2x  ", offset + i + 1);
    }

    return(PASSED);
}


/**********************************************************************
 *
 * Function: pmc4359_init_xpsc
 *
 * This function: initializes a set of indirect registers.
 *	The set of indirect registers possible are:
 *	CMQ_TPSC_CFG, CMQ_RPSC_CFG, and CMQ_SIGX_CFG_CHG_SIG_STATE
 *
 * Input : dev - Pointer to the Framer device object
 *         frm_num        - framer number
 *         reg            - start of indrect registers
 *         
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int 
pmc4359_init_xpsc (dev_object_t *dev, uchar frm_num, ushort reg)
{
    dev_4359_object_t *pmc4359 = (dev_4359_object_t *)dev;
    ulong frm_base_addr;
    ulong i;

    frm_base_addr = (ulong)pmc4359->base.dev_addr + 
	(CMQ_FRM_OFFSET * pmc4359->bus_width * frm_num);
    /*
     * set all to 0
     */
    for (i = 0; i < PM4359_NUM_IND_REG; i++) {
        if (pmc4359_wr_ind_reg(dev, (frm_base_addr + reg),
                                   CMQ_TPSC_IND_DATA_CTL + i, 0))
                return(FAILED);
    }

    return(PASSED);
}


/**********************************************************************
 *
 * Function: pmc4359_wr_ind_reg()
 *
 * This function writes a specified value to an indirect register.
 * There are 3 blocks of indirect registers that can be acessed
 * using this function. They are TPSC, RPSC and SIGX. These are
 * per channel registers so the conf_reg_addr should contain the
 * channel information as well. For example, 0x50 is the SIGX_CFG 
 * register for port 0, 0x150 is for port 1, 0x250 for port 2 and 
 * 0x350 for port 3.
 *
 * Input : dev      - Pointer to the PMC device driver object
 *         conf_reg - indrect register configuration reg
 *         offset   - offset of an indirect reg
 *         value    - value to be written to an indirect reg
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int 
pmc4359_wr_ind_reg (dev_object_t *dev, ulong conf_reg_addr, int offset, 
		    uchar value)
{
    dev_4359_object_t *pmc4359 = (dev_4359_object_t *)dev;
    dev_4359_callout_fvt_t *callout_p = pmc4359->callout_fvt;
    ulong status_reg, addr_reg, data_reg, frm_bar, wait_time;
    uchar port_num, lower_byte, val;

    /* 
     * Sanity check to make sure the indirect regions requested are
     * TPSC, RPSC or SIGX
     */
    lower_byte = (uchar) conf_reg_addr & 0xFF;
    if ((lower_byte != CMQ_TPSC_CFG) && (lower_byte != CMQ_RPSC_CFG) &&
	(lower_byte != CMQ_SIGX_CFG_CHG_SIG_STATE)) {
	sprintf(log_buffer, "conf_reg_addr = 0x%x is not a valid region\n", 
		conf_reg_addr);
	(*pmc4359->base.dev_object_fvt->dev_error_report)(&pmc4359->base, 
							  log_buffer, FATAL);
	return(FAILED);
    }

#ifdef IDEBUG
    printf("pmc4359_wr_ind_reg,"
           " conf_reg addr: %#x, offset %#x, value: %#x\n",
           conf_reg_addr, offset, value);
#endif

    /* Port number is the upper byte of conf_reg_addr */
    port_num = (uchar)((conf_reg_addr & 0xFF00) >> 8);
    frm_bar = (ulong)pmc4359->base.dev_addr + (port_num * CMQ_FRM_OFFSET * 
					       pmc4359->bus_width);
    /*
     * Make sure the IND Bit is set.
     */
    val = callout_p->rd_frm_reg(frm_bar, lower_byte, pmc4359->bus_width);
    if (!(val & CMQ_CFG_IND_ACCESS)) {
        val |= CMQ_CFG_IND_ACCESS;
	callout_p->wr_frm_reg(frm_bar, lower_byte, val, pmc4359->bus_width);
    }

    /* Calculate address for status, address and data register */
    status_reg = lower_byte + 1;
    addr_reg   = lower_byte + 2;
    data_reg   = lower_byte + 3;

    /* Wait for BUSY signal to clear - typically within 640 ns */
    wait_time = 0;
    do {
        msleep(BUSY_WAIT_TIME);
        val = callout_p->rd_frm_reg(frm_bar, status_reg, pmc4359->bus_width);
        wait_time += BUSY_WAIT_TIME;
    } while(((val & CMQ_MICRO_ACCESS_STAT_BUSY) ==
             CMQ_MICRO_ACCESS_STAT_BUSY) && (wait_time < BUSY_WAIT_MAX));

    if (wait_time >= BUSY_WAIT_MAX) {
        sprintf(log_buffer, "Busy timeout prior to write of indirect register"
                " 0x%x, offset 0x%x\n", conf_reg_addr, offset);
        (*pmc4359->base.dev_object_fvt->dev_error_report)(&pmc4359->base,
                                                          log_buffer, FATAL);
        return(FAILED);
    }

    /* Put data to be written into data register */
    callout_p->wr_frm_reg(frm_bar, data_reg, value, pmc4359->bus_width);
       
    /*
     * Setup the address and RW bit (1 for read, 0 for write)
     */
    val = (offset & CMQ_CHN_IND_ADDR_MASK);
    callout_p->wr_frm_reg(frm_bar, addr_reg, val, pmc4359->bus_width);

    /* Wait for BUSY signal to clear - typically within 640 ns */
    wait_time = 0;
    do {
	/* Sleep for 1ms */
	msleep(BUSY_WAIT_TIME);
	val = callout_p->rd_frm_reg(frm_bar, status_reg, pmc4359->bus_width);
	wait_time += BUSY_WAIT_TIME;
    } while(((val & CMQ_MICRO_ACCESS_STAT_BUSY) == 
	     CMQ_MICRO_ACCESS_STAT_BUSY) && (wait_time < BUSY_WAIT_MAX));
	
    if (wait_time >= BUSY_WAIT_MAX) {
	sprintf(log_buffer, "Busy timeout while writing to indirect register"
		" 0x%x, offset 0x%x\n", conf_reg_addr, offset);
	(*pmc4359->base.dev_object_fvt->dev_error_report)(&pmc4359->base, 
							  log_buffer, FATAL);
        return(FAILED);
    }

    return(PASSED);
}


/**********************************************************************
 *
 * Function: pmc4359_rd_ind_reg
 *
 * This function reads an indirect register and stores it in value_p
 * There are 3 blocks of indirect registers that can be read
 * using this function. They are TPSC, RPSC and SIGX. These are
 * per channel registers so the conf_reg_addr should contain the
 * channel information as well. For example, 0x50 is the SIGX_CFG 
 * register for port 0, 0x150 is for port 1, 0x250 for port 2 and 
 * 0x350 for port 3.
 *
 * Input : dev      - Pointer to the PMC device driver object
 *         conf_reg - indrect register configuration reg
 *         offset   - offset of an indirect reg
 *         value_p  - a variable to store value read from an indirect reg
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int 
pmc4359_rd_ind_reg (dev_object_t *dev, ulong conf_reg_addr, int offset, 
		    uchar *value_p)
{
    dev_4359_object_t *pmc4359 = (dev_4359_object_t *)dev;
    dev_4359_callout_fvt_t *callout_p = pmc4359->callout_fvt;
    ulong status_reg, addr_reg, data_reg, frm_bar, wait_time, lower_byte;
    uchar port_num, val;

    /* 
     * Sanity check to make sure the indirect regions requested are
     * TPSC, RPSC or SIGX
     */
    lower_byte = conf_reg_addr & 0xFF;
    if ((lower_byte != CMQ_TPSC_CFG) && (lower_byte != CMQ_RPSC_CFG) &&
	(lower_byte != CMQ_SIGX_CFG_CHG_SIG_STATE)) {
	sprintf(log_buffer, "conf_reg_addr at 0x%x is not a valid region\n", 
		conf_reg_addr);
	(*pmc4359->base.dev_object_fvt->dev_error_report)(&pmc4359->base, 
							  log_buffer, FATAL);
	return(FAILED);
    }

    /* Port number is the upper byte of conf_reg_addr */
    port_num = (uchar)((conf_reg_addr & 0xFF00) >> 8);
    frm_bar = (ulong)pmc4359->base.dev_addr + (port_num * CMQ_FRM_OFFSET * 
					       pmc4359->bus_width);
    /*
     * Make sure the IND Bit is set.
     */
    val = callout_p->rd_frm_reg(frm_bar, lower_byte, pmc4359->bus_width);
    if (!(val & CMQ_CFG_IND_ACCESS)) {
        val |= CMQ_CFG_IND_ACCESS;
	callout_p->wr_frm_reg(frm_bar, lower_byte, val, pmc4359->bus_width);
    }

    /* Calculate address for status, address and data register */
    status_reg = (lower_byte + 1);
    addr_reg   = (lower_byte + 2);
    data_reg   = (lower_byte + 3);

#ifdef IDEBUG
    printf("pmc4359_rd_ind_reg,"
           " conf_reg addr: %#x, offset %#x, value addr %#x, status_reg %#x\n",
           conf_reg_addr, offset, value_p, status_reg);
#endif

    /* Wait for BUSY signal to clear - typically within 640 ns */
    wait_time = 0;
    do {
        msleep(BUSY_WAIT_TIME);
        val = callout_p->rd_frm_reg(frm_bar, status_reg, pmc4359->bus_width);
        wait_time += BUSY_WAIT_TIME;
    } while(((val & CMQ_MICRO_ACCESS_STAT_BUSY) ==
             CMQ_MICRO_ACCESS_STAT_BUSY) && (wait_time < BUSY_WAIT_MAX));

    if (wait_time >= BUSY_WAIT_MAX) {
        sprintf(log_buffer, "Busy timeout prior to read of indirect register"
                " offset 0x%x, status @%#x = %#x\n",
		offset, status_reg, val);
        (*pmc4359->base.dev_object_fvt->dev_error_report)(&pmc4359->base,
                                                          log_buffer, FATAL);
        return(FAILED);
    }

    /*
     * Setup the address and RW bit (1 for read, 0 for write)
     */
    val = (offset & CMQ_CHN_IND_ADDR_MASK) | CMQ_CHN_IND_ADDR_CTL_RD;
    callout_p->wr_frm_reg(frm_bar, addr_reg, val, pmc4359->bus_width);

    /* Wait for BUSY signal to clear - typically within 640 ns */
    wait_time = 0;
    do {
	msleep(BUSY_WAIT_TIME);
	val = callout_p->rd_frm_reg(frm_bar, status_reg, pmc4359->bus_width);
	wait_time += BUSY_WAIT_TIME;
    } while(((val & CMQ_MICRO_ACCESS_STAT_BUSY) == 
	     CMQ_MICRO_ACCESS_STAT_BUSY) && (wait_time < BUSY_WAIT_MAX));
	
    if (wait_time >= BUSY_WAIT_MAX) {
	sprintf(log_buffer, "Busy timeout during read of indirect register"
		" offset 0x%x, status @%#x = %#x\n",
		offset, lower_byte + status_reg, val);
	(*pmc4359->base.dev_object_fvt->dev_error_report)(&pmc4359->base, 
							  log_buffer, FATAL);
        return(FAILED);
    }

    *value_p = callout_p->rd_frm_reg(frm_bar, data_reg, pmc4359->bus_width);
    return(PASSED);
}


/**********************************************************************
 *
 * Function: pm4359_set_operation
 *
 * This function: Configure global data format for the device.
 *                Registers: 0x00, 0x1C, 0x20, 0xB0, 0xB4, 0x28, 0x38
 *
 * Input : dev - Pointer to the framer device object
 *         
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int 
pm4359_set_operation (dev_object_t *dev)
{
    uchar val, framer, op_mode;
    ulong framer_bar, chip_bar;
    dev_4359_object_t *pmc4359 = (dev_4359_object_t *)dev;
    dev_4359_callout_fvt_t *callout_p = pmc4359->callout_fvt;

    chip_bar = (ulong)pmc4359->base.dev_addr;
    op_mode = pmc4359->cfg_info_p->op_mode;
    val   = CMQ_GLBL_PGM_IO_EN;    /* PIO as output */

    switch (op_mode) {
    case CMQ_MODE_E1:
        val |= CMQ_GLBL_E1_MODE;
        break;
    case CMQ_MODE_T1:
        val &= ~CMQ_GLBL_E1_MODE;
        break;
    default:
	/* Invalid operation mode, expected 0 or 1 */
        return(FAILED);
    }

    callout_p->wr_frm_reg(chip_bar, CMQ_GLB_CFG, val, pmc4359->bus_width);

    /*
     * configure all blocks that need to be configured as per the
     * the operational mode
     */
    for (framer = 0; framer < CMQ_MAX_FRAMERS; framer++) {
        framer_bar = chip_bar+ (framer * CMQ_FRM_OFFSET * pmc4359->bus_width);
        if (op_mode == CMQ_MODE_E1) {
            /*
             * rx elastic config: reg 0x1c
             */
	    callout_p->wr_frm_reg(framer_bar, CMQ_RX_ELST_CFG, 
				  CMQ_RX_ELST_CFG_E1, pmc4359->bus_width);
            /*
             * configure rx ccs elastic store: reg 0xb0
             */
	    callout_p->wr_frm_reg(framer_bar, CMQ_RX_ELST_CCS_CFG,
				  CMQ_RX_ELST_CCS_CFG_E1, pmc4359->bus_width);

            /*
             * tx elastic config: reg 0x20
             */
	    callout_p->wr_frm_reg(framer_bar, CMQ_TX_ELST_CFG,
				  CMQ_TX_ELST_CFG_E1, pmc4359->bus_width);

            /*
             * configure tx ccs elastic store: reg 0xb4
             */
	    callout_p->wr_frm_reg(framer_bar, CMQ_TX_ELST_CCS_CFG,
				  CMQ_TX_ELST_CCS_CFG_E1, pmc4359->bus_width);

            /*
             * for E1, the T1 data link bit in the receive data link control
             * register must be off.  0x28
             */
            val = callout_p->rd_frm_reg(framer_bar, CMQ_RX_DLNK_CTL,
					pmc4359->bus_width) & 
		~(CMQ_RX_DLNK_CTL_EN_TERM | CMQ_RX_DLNK_CTL_EXTRACT_EVEN_FRM |
		  CMQ_RX_DLNK_CTL_EXTRACT_ODD_FRM);
	    callout_p->wr_frm_reg(framer_bar, CMQ_RX_DLNK_CTL, val,
				  pmc4359->bus_width);

            /*
             * for E1, the T1 data link bit in the transmit data link control
             *  register must be off and the odd and even bits must be off as
             * well.  0x38
             */
            val = callout_p->rd_frm_reg(framer_bar, CMQ_TX_DLNK_CTL,
					pmc4359->bus_width) & 
		~(CMQ_TX_DLNK_CTL_EN_INS | CMQ_TX_DLNK_CTL_INS_EVEN_FRM |
		  CMQ_TX_DLNK_CTL_INS_ODD_FRM);
	    callout_p->wr_frm_reg(framer_bar, CMQ_TX_DLNK_CTL, val, 
				  pmc4359->bus_width);

        } else {            /* T1 mode */
            /*
             * Rx elastic config: reg 0x1c
             */
	    callout_p->wr_frm_reg(framer_bar, CMQ_RX_ELST_CFG, 
				  CMQ_RX_ELST_CFG_T1, pmc4359->bus_width);
            /*
             * configure rx ccs elastic store: reg 0xb0
             */
	    callout_p->wr_frm_reg(framer_bar, CMQ_RX_ELST_CCS_CFG, 
				  CMQ_RX_ELST_CCS_CFG_T1, pmc4359->bus_width);

            /*
             * Tx elastic config: reg 0x20
             */
	    callout_p->wr_frm_reg(framer_bar, CMQ_TX_ELST_CFG, 
				  CMQ_TX_ELST_CFG_T1, pmc4359->bus_width);

            /*
             * configure tx ccs elastic store: reg 0xb4
             */
	    callout_p->wr_frm_reg(framer_bar, CMQ_TX_ELST_CCS_CFG,
				  CMQ_TX_ELST_CCS_CFG_T1, pmc4359->bus_width);
         }
    }

    return(PASSED);
}


/**********************************************************************
 *
 * Function: pm4359_set_tx_mode()
 *
 * This function: sets transmitter's mode
 *
 * Input : dev       - Pointer to the PMC device object
 *         frm_bar   - Base address of the framer
 *         op_mode   - operation mode, T1 or E1
 *         frm_mode  - framing mode
 *         
 * Output: NONE
 *
 **********************************************************************
 */
static void 
pm4359_set_tx_mode (dev_object_t *dev, ulong frm_bar, uchar op_mode, 
		    uchar frm_mode)
{
    uchar val;
    dev_4359_object_t *pmc4359 = (dev_4359_object_t *)dev;
    dev_4359_callout_fvt_t *callout_p = pmc4359->callout_fvt;

    if (op_mode == CMQ_MODE_E1) {
        val = callout_p->rd_frm_reg(frm_bar, CMQ_E1_TRAN_CFG, 
				    pmc4359->bus_width) & 
	    ~(CMQ_E1_TRN_CFG_EN_GEN_CRC_MFRM | CMQ_E1_TRN_CFG_FRM_DIS);

        switch (frm_mode) {
        case CMQ_FRM_MODE_E1_CRC_MFRM:
            val |= CMQ_E1_TRN_CFG_EN_GEN_CRC_MFRM;
            break;
        case CMQ_FRM_MODE_E1_UNFRAMED:
            val |= CMQ_E1_TRN_CFG_FRM_DIS;
            break;
        default:
            break;
        }
	callout_p->wr_frm_reg(frm_bar, CMQ_E1_TRAN_CFG, val, 
			      pmc4359->bus_width);

    } else {
        val = callout_p->rd_frm_reg(frm_bar, CMQ_T1_XBAS_CFG, 
				    pmc4359->bus_width) & 
	    ~(CMQ_T1_XBAS_CFG_JPN_EN | CMQ_T1_XBAS_CFG_MODE_MASK);

        switch (frm_mode) {
        case CMQ_FRM_MODE_T1_SF:
            val |= CMQ_T1_XBAS_CFG_SF_MODE;
            break;
        case CMQ_FRM_MODE_T1_DM:
            val |= CMQ_T1_XBAS_CFG_T1DM_MODE;
            break;
        case CMQ_FRM_MODE_T1_SLC96:
            val |= CMQ_T1_XBAS_CFG_SLC96_MODE;
            break;
        case CMQ_FRM_MODE_T1_DM_FDL:
            val |= CMQ_T1_XBAS_CFG_T1DM_MODE2;
            break;
        case CMQ_FRM_MODE_T1_ESF:
            val |= CMQ_T1_XBAS_CFG_ESF_FDL_MODE | CMQ_T1_XBAS_CFG_B8ZS_EN;
            break;
        case CMQ_FRM_MODE_T1_SF_JPN_ALARM:
            val |= CMQ_T1_XBAS_CFG_SF_MODE | CMQ_T1_XBAS_CFG_JPN_EN;
            break;
        case CMQ_FRM_MODE_T1_DM_JPN_ALARM:
            val |= CMQ_T1_XBAS_CFG_T1DM_MODE | CMQ_T1_XBAS_CFG_JPN_EN;
            break;
        case CMQ_FRM_MODE_T1_SLC96_JPN_ALARM:
            val |= CMQ_T1_XBAS_CFG_SLC96_MODE | CMQ_T1_XBAS_CFG_JPN_EN;
            break;
        case CMQ_FRM_MODE_T1_DM_FDL_JPN_ALARM:
            val |= CMQ_T1_XBAS_CFG_T1DM_MODE2 | CMQ_T1_XBAS_CFG_JPN_EN;
            break;
        case CMQ_FRM_MODE_T1_JT_G704:
            val |= CMQ_T1_XBAS_CFG_ESF_FDL_MODE | CMQ_T1_XBAS_CFG_JPN_EN;
            break;
        default:
            break;
        }
	callout_p->wr_frm_reg(frm_bar, CMQ_T1_XBAS_CFG, val, 
			      pmc4359->bus_width);
    }

    /*
     * configure framing in the transmit framing bypass options register
     * reg 0x05
     */
    val = callout_p->rd_frm_reg(frm_bar, CMQ_TX_FRM_BYPASS_OPTS,
				pmc4359->bus_width);
    if ((frm_mode == CMQ_FRM_MODE_E1_UNFRAMED) ||
        (frm_mode == CMQ_FRM_MODE_T1_UNFRAMED))
        val |= CMQ_TX_FRM_OPT_FRM_DIS;
    else
        val &= ~CMQ_TX_FRM_OPT_FRM_DIS; 

    callout_p->wr_frm_reg(frm_bar, CMQ_TX_FRM_BYPASS_OPTS, val, 
			  pmc4359->bus_width);
}


/**********************************************************************
 *
 * Function: pm4359_set_rx_mode
 *
 * This function: sets receiver's mode
 *
 * Input : dev       - Pointer to the PMC device object
 *         frm_bar   - Base address of the framer
 *         op_mode   - operation mode, T1 or E1
 *         frm_mode  - framing mode
 *         
 * Output: NONE
 *
 **********************************************************************
 */
static void 
pm4359_set_rx_mode (dev_object_t *dev, ulong frm_bar, uchar op_mode, 
		    uchar frm_mode)
{
    uchar val;
    dev_4359_object_t *pmc4359 = (dev_4359_object_t *)dev;
    dev_4359_callout_fvt_t *callout_p = pmc4359->callout_fvt;

    if (op_mode == CMQ_MODE_E1) {
        /*
         * E1 Frame alignment options, reg 0x90
         */
        val = callout_p->rd_frm_reg(frm_bar, CMQ_E1_FRMR_FRM_ALIGN_OPT,
				    pmc4359->bus_width) & 
	    ~CMQ_E1_FRM_OPT_RESERVE1;

        switch (frm_mode) {
        case CMQ_FRM_MODE_E1:
            val &= ~CMQ_E1_FRM_OPT_CRC_EN;
            break;
        case CMQ_FRM_MODE_E1_CRC_MFRM:
            val |= CMQ_E1_FRM_OPT_CRC_EN;
            break;
        default:
            break;
        }

	/* Write back */
	callout_p->wr_frm_reg(frm_bar, CMQ_E1_FRMR_FRM_ALIGN_OPT, val,
			      pmc4359->bus_width);
    } else {
        /*
         * T1 framer configration, reg 0x48
         */
        val = callout_p->rd_frm_reg(frm_bar, CMQ_T1_FRMR_CFG,
				    pmc4359->bus_width) & 
	    ~(CMQ_T1_FRMR_CFG_MODE_MASK | CMQ_T1_FRMR_CFG_JPN_EN);

        switch (frm_mode) {
        case CMQ_FRM_MODE_T1_SF:
            val |= CMQ_T1_FRMR_CFG_SF_MODE;
            break;
        case CMQ_FRM_MODE_T1_DM:
            val |= CMQ_T1_FRMR_CFG_T1DM_MODE;
            break;
        case CMQ_FRM_MODE_T1_SLC96:
            val |= CMQ_T1_FRMR_CFG_SLC96_MODE;
            break;
        case CMQ_FRM_MODE_T1_DM_FDL:
            val |= CMQ_T1_FRMR_CFG_T1DM_MODE2;
            break;
        case CMQ_FRM_MODE_T1_ESF:
            val |= CMQ_T1_FRMR_CFG_ESF_FDL_MODE | CMQ_T1_FRMR_CFG_ESF_CRC_ALGO;
            break;
        case CMQ_FRM_MODE_T1_SF_JPN_ALARM:
            val |= CMQ_T1_FRMR_CFG_SF_MODE | CMQ_T1_FRMR_CFG_JPN_EN;
            break;
        case CMQ_FRM_MODE_T1_DM_JPN_ALARM:
            val |= CMQ_T1_FRMR_CFG_T1DM_MODE | CMQ_T1_FRMR_CFG_JPN_EN;
            break;
        case CMQ_FRM_MODE_T1_SLC96_JPN_ALARM:
            val |= CMQ_T1_FRMR_CFG_SLC96_MODE | CMQ_T1_FRMR_CFG_JPN_EN;
            break;
        case CMQ_FRM_MODE_T1_DM_FDL_JPN_ALARM:
            val |= CMQ_T1_FRMR_CFG_T1DM_MODE2 | CMQ_T1_FRMR_CFG_JPN_EN;
            break;
        case CMQ_FRM_MODE_T1_JT_G704:
            val |= CMQ_T1_FRMR_CFG_ESF_FDL_MODE | CMQ_T1_FRMR_CFG_JPN_EN;
            break;
        default:
            break;
        }
	/* Write back */
	callout_p->wr_frm_reg(frm_bar, CMQ_T1_FRMR_CFG, val, 
			      pmc4359->bus_width);
    }

    /*
     * configure receive options, reg 0x02
     */
    val = callout_p->rd_frm_reg(frm_bar, CMQ_RX_OPTS, pmc4359->bus_width);
    if ((frm_mode == CMQ_FRM_MODE_E1_UNFRAMED) ||
        (frm_mode == CMQ_FRM_MODE_T1_UNFRAMED))
        val |= CMQ_RX_OPT_UNFRM;
    else
        val &= ~CMQ_RX_OPT_UNFRM; 

    callout_p->wr_frm_reg(frm_bar, CMQ_RX_OPTS, val, pmc4359->bus_width);
}


/**********************************************************************
 *
 * Function: pm4359_t1_tx_conf()
 *
 * This function: configures T1 transmit interface.
 *
 * Input : dev - Pointer to the framer device object
 *         
 * Output: NONE
 *
 **********************************************************************
 */
static void 
pm4359_t1_tx_conf (dev_object_t *dev)
{
    uchar val, frm_num;
    ulong frm_bar;
    cmq_framer_conf_t *conf_info_p;
    dev_4359_object_t *pmc4359 = (dev_4359_object_t *)dev;
    dev_4359_callout_fvt_t *callout_p = pmc4359->callout_fvt;

    conf_info_p = pmc4359->cfg_info_p;
    frm_num = conf_info_p->quad_num;
    frm_bar = (ulong)pmc4359->base.dev_addr + (frm_num * CMQ_FRM_OFFSET * 
					       pmc4359->bus_width);
    pm4359_set_tx_mode(dev, frm_bar, CMQ_MODE_T1, conf_info_p->frm_mode);

    /*
     * configure T1 zero code suppression formats, reg 0x54
     */
    val = callout_p->rd_frm_reg(frm_bar, CMQ_T1_XBAS_CFG, pmc4359->bus_width) 
	& ~CMQ_T1_XBAS_CFG_ZSUP_FMT_MASK;

    switch(conf_info_p->zcode_suppr_format) {
    case CMQ_T1_ZSUP_NONE:
        val |= CMQ_T1_XBAS_CFG_ZSUP_FMT_NONE;
        break;
    case CMQ_T1_ZSUP_GTE:
        val |= CMQ_T1_XBAS_CFG_ZSUP_FMT_GTE;
        break;
    case CMQ_T1_ZSUP_DDS:
        val |= CMQ_T1_XBAS_CFG_ZSUP_FMT_DDS;
        break;
    case CMQ_T1_ZSUP_BELL:
        val |= CMQ_T1_XBAS_CFG_ZSUP_FMT_BELL;
        break;
    default:
        break;
    }
    callout_p->wr_frm_reg(frm_bar, CMQ_T1_XBAS_CFG, val, pmc4359->bus_width);

    /*
     * configure superframe alignment of signaling bits, reg 0x05
     */
    val = callout_p->rd_frm_reg(frm_bar, CMQ_TX_FRM_BYPASS_OPTS, 
				pmc4359->bus_width) & 
	~CMQ_TX_FRM_OPT_SIG_ALIGN_EN;

    if (conf_info_p->sf_align_en)
        val |= CMQ_TX_FRM_OPT_SIG_ALIGN_EN;

    callout_p->wr_frm_reg(frm_bar, CMQ_TX_FRM_BYPASS_OPTS, val,
			  pmc4359->bus_width);
}
    


/**********************************************************************
 *
 * Function: pm4359_t1_rx_conf
 *
 * This function: configures T1 receive interface.
 *
 * Input : dev - Pointer to the framer device object
 *         
 * Output: NONE
 *
 **********************************************************************
 */
static void 
pm4359_t1_rx_conf (dev_object_t *dev)
{
    uchar val, frm_num;
    ulong frm_bar;
    cmq_framer_conf_t *conf_info_p;
    dev_4359_object_t *pmc4359 = (dev_4359_object_t *)dev;
    dev_4359_callout_fvt_t *callout_p = pmc4359->callout_fvt;

    conf_info_p = pmc4359->cfg_info_p;
    frm_num = conf_info_p->quad_num;
    frm_bar = (ulong)pmc4359->base.dev_addr + (frm_num * CMQ_FRM_OFFSET * 
					       pmc4359->bus_width);

    /* Set RX mode */
    pm4359_set_rx_mode(dev, frm_bar, CMQ_MODE_T1, conf_info_p->frm_mode);

    if (conf_info_p->frm_mode != CMQ_FRM_MODE_T1_UNFRAMED) {
        /*
         * PREP 6133:
         * we have a COMET Tetra device in framed T1 => RJAT must be enabled
         * by turning off the bypass bit (RJATBYP), reg 0x02
         */
	val = callout_p->rd_frm_reg(frm_bar, CMQ_RX_OPTS, pmc4359->bus_width);
	callout_p->wr_frm_reg(frm_bar, CMQ_RX_OPTS, 
			      (val & ~CMQ_RX_OPT_JAT_BYPASS), 
			      pmc4359->bus_width);

        /*
         * configure remaining options, 0x48
         */
        val = callout_p->rd_frm_reg(frm_bar, CMQ_T1_FRMR_CFG,
				    pmc4359->bus_width) & 
	    ~(CMQ_T1_FRMR_CFG_ERR_DET_MASK | CMQ_T1_FRMR_CFG_ESF_CRC_ALGO);

        switch (conf_info_p->out_of_frame) {
        case CMQ_T1_OOF_2OF4:
            val |= CMQ_T1_FRMR_CFG_ERR_DET_2OF4;
            break;
        case CMQ_T1_OOF_2OF5:
            val |= CMQ_T1_FRMR_CFG_ERR_DET_2OF5;
            break;
        case CMQ_T1_OOF_2OF6:
            val |= CMQ_T1_FRMR_CFG_ERR_DET_2OF6;
            break;
        default:
            break;
        }

        switch (conf_info_p->frm_esf_algo) {
        case CMQ_T1_ESF_FRAME_ALGO_CRC_6:
            val |= CMQ_T1_FRMR_CFG_ESF_CRC_ALGO;
            break;
        case CMQ_T1_ESF_FRAME_ALGO_ONE_CANDIDATE:
        default:
            break;
        }
	/* Write val */
	callout_p->wr_frm_reg(frm_bar, CMQ_T1_FRMR_CFG, val, 
			      pmc4359->bus_width);

        val = callout_p->rd_frm_reg(frm_bar, CMQ_RX_OPTS, pmc4359->bus_width);
        if (conf_info_p->ccofa_en)
            val |= CMQ_RX_OPT_CNT_COFA;
        else
            val &= ~CMQ_RX_OPT_CNT_COFA;
	callout_p->wr_frm_reg(frm_bar, CMQ_RX_OPTS, val, pmc4359->bus_width);
    }

    /*
     * configure all blocks in the receive data path for T1 mode, reg 0x60
     */
    val = callout_p->rd_frm_reg(frm_bar, CMQ_T1_ALMI_CFG, pmc4359->bus_width) 
	& ~CMQ_T1_ALMI_CFG_MODE_MASK;

    switch (conf_info_p->frm_mode) {
    case CMQ_FRM_MODE_T1_SF:
    case CMQ_FRM_MODE_T1_SF_JPN_ALARM:
        val |= CMQ_T1_ALMI_CFG_SF_MODE;
        break;
    case CMQ_FRM_MODE_T1_DM:
    case CMQ_FRM_MODE_T1_DM_JPN_ALARM:
        val |= CMQ_T1_ALMI_CFG_T1DM_MODE;
        break;
    case CMQ_FRM_MODE_T1_SLC96:
    case CMQ_FRM_MODE_T1_SLC96_JPN_ALARM:
        val |= CMQ_T1_ALMI_CFG_SLC96_MODE;
        break;
    case CMQ_FRM_MODE_T1_DM_FDL:
    case CMQ_FRM_MODE_T1_DM_FDL_JPN_ALARM:
        val |= CMQ_T1_ALMI_CFG_T1DM_MODE2;
        break;
    case CMQ_FRM_MODE_T1_ESF:
    case CMQ_FRM_MODE_T1_JT_G704:
        val |= CMQ_T1_ALMI_CFG_ESF_FDL_MODE;
        break;
    default:
        break;
    }
    callout_p->wr_frm_reg(frm_bar, CMQ_T1_ALMI_CFG, val, pmc4359->bus_width);

    /*
     * configure the signalling extractor to support ESF, reg 0x50
     */
    val = callout_p->rd_frm_reg(frm_bar, CMQ_SIGX_CFG_CHG_SIG_STATE, 
				pmc4359->bus_width);

    switch (conf_info_p->frm_mode) {
    case CMQ_FRM_MODE_T1_SF:
    case CMQ_FRM_MODE_T1_SF_JPN_ALARM:
    case CMQ_FRM_MODE_T1_DM:
    case CMQ_FRM_MODE_T1_DM_JPN_ALARM:
    case CMQ_FRM_MODE_T1_SLC96:
    case CMQ_FRM_MODE_T1_SLC96_JPN_ALARM:
    case CMQ_FRM_MODE_T1_DM_FDL:
    case CMQ_FRM_MODE_T1_DM_FDL_JPN_ALARM:
        val &= ~CMQ_SIGX_CFG_ESF_MODE;
        break;
    case CMQ_FRM_MODE_T1_ESF:
    case CMQ_FRM_MODE_T1_JT_G704:
        val |= CMQ_SIGX_CFG_ESF_MODE;
        break;
    default:
    break;
    }
    callout_p->wr_frm_reg(frm_bar, CMQ_SIGX_CFG_CHG_SIG_STATE, val,
			  pmc4359->bus_width);
}


/**********************************************************************
 *
 * Function: pm4359_e1_tx_conf
 *
 * This function: configures E1 transmit interface
 *
 * Input : dev - Pointer to the framer device object
 *         
 * Output: None
 *
 **********************************************************************
 */
static void
pm4359_e1_tx_conf (dev_object_t *dev)
{
    uchar val;
    cmq_framer_conf_t *conf_info_p;
    ulong frm_bar;
    uchar frm_num;
    dev_4359_object_t *pmc4359 = (dev_4359_object_t *)dev;
    dev_4359_callout_fvt_t *callout_p = pmc4359->callout_fvt;

    conf_info_p = pmc4359->cfg_info_p;
    frm_num = conf_info_p->quad_num;
    frm_bar = (ulong)pmc4359->base.dev_addr + (frm_num * CMQ_FRM_OFFSET * 
					       pmc4359->bus_width);
    pm4359_set_tx_mode(dev, frm_bar, CMQ_MODE_E1, conf_info_p->frm_mode);

    /*
     * configure E1 TRANS, reg 0x80
     */
    val = callout_p->rd_frm_reg(frm_bar, CMQ_E1_TRAN_CFG, pmc4359->bus_width)
	& ~(CMQ_E1_TRN_CFG_SIG_CCS_EN | CMQ_E1_TRN_CFG_SIG_CAS_EN |
	  CMQ_E1_TRN_CFG_INS_NAT_INTER_DIS | CMQ_E1_TRN_CFG_INS_EXTRA_BIT_DIS
	  | CMQ_E1_TRN_CFG_FEBE_DIS);

    switch(conf_info_p->ts16Signaling) {
    case CMQ_E1_SIG_INS_HDLC_CCS:
        val |= CMQ_E1_TRN_CFG_SIG_CCS_EN;
        break;
    case CMQ_E1_SIG_INS_CAS:
        val |= CMQ_E1_TRN_CFG_SIG_CAS_EN;
        break;
    case CMQ_E1_SIG_INS_NONE:
    default:
        break;
    }
    if (!conf_info_p->nat_bit_en)
        val |= CMQ_E1_TRN_CFG_INS_NAT_INTER_DIS;
    if (!conf_info_p->xtra_bit_en)
        val |= CMQ_E1_TRN_CFG_INS_EXTRA_BIT_DIS;
    if (!conf_info_p->febee_en)
        val |= CMQ_E1_TRN_CFG_FEBE_DIS;
    callout_p->wr_frm_reg(frm_bar, CMQ_E1_TRAN_CFG, val, pmc4359->bus_width);
}


/**********************************************************************
 *
 * Function: pm4359_e1_rx_conf
 *
 * This function: configures E1 receive interface
 *
 * Input : dev - Pointer to the framer device object
 *         
 * Output: NONE
 *
 **********************************************************************
 */
static void 
pm4359_e1_rx_conf (dev_object_t *dev)
{
    uchar val, frm_num;
    ulong frm_bar;
    cmq_framer_conf_t *conf_info_p;
    dev_4359_object_t *pmc4359 = (dev_4359_object_t *)dev;
    dev_4359_callout_fvt_t *callout_p = pmc4359->callout_fvt;

    conf_info_p = pmc4359->cfg_info_p;
    frm_num = conf_info_p->quad_num;
    frm_bar = (ulong)pmc4359->base.dev_addr + (frm_num * CMQ_FRM_OFFSET * 
					       pmc4359->bus_width);

    /* Set RX mode */
    pm4359_set_rx_mode(dev, frm_bar, CMQ_MODE_E1, conf_info_p->frm_mode);

    if (conf_info_p->frm_mode != CMQ_FRM_MODE_T1_UNFRAMED) {
        /*
         * E1 framer alignment options, reg 0x90
         */
        val = callout_p->rd_frm_reg(frm_bar, CMQ_E1_FRMR_FRM_ALIGN_OPT,
				    pmc4359->bus_width) & 
	    ~(CMQ_E1_FRM_OPT_CAS_DIS | CMQ_E1_FRM_OPT_CRC_2NO_CRC |
	      CMQ_E1_FRM_OPT_CRC_THRESH_RESYNC | CMQ_E1_FRM_OPT_REFRM_DIS |
	      CMQ_E1_FRM_OPT_RESERVE1);

        if (!conf_info_p->cas_align_en)
            val |= CMQ_E1_FRM_OPT_CAS_DIS;
        if (conf_info_p->crc2ncrc_en)
            val |= CMQ_E1_FRM_OPT_CRC_2NO_CRC;
        if (conf_info_p->no_refrm_err_en)
            val |= CMQ_E1_FRM_OPT_REFRM_DIS;
        if (conf_info_p->refrm_xs_crc_en)
            val |= CMQ_E1_FRM_OPT_CRC_THRESH_RESYNC;
	callout_p->wr_frm_reg(frm_bar, CMQ_E1_FRMR_FRM_ALIGN_OPT, val,
			      pmc4359->bus_width);

        /*
         * E1 framer maintenance mode options, reg 0x91
         */
        val = callout_p->rd_frm_reg(frm_bar, CMQ_E1_FRMR_MAINT_MODE_OPT,
				    pmc4359->bus_width) & 
	    ~(CMQ_E1_FRM_MAINT_OPT_NFAS_OOF_CRITERIA |
	      CMQ_E1_FRM_MAINT_OPT_OOMF_CRITERIA |
	      CMQ_E1_FRM_MAINT_OPT_OOMF_TSLOT16_CRITERIA |
	      CMQ_E1_FRM_MAINT_OPT_RAI_CRITERIA |
	      CMQ_E1_FRM_MAINT_OPT_AIS_CRITERIA);

        if (conf_info_p->los_bit_2crit_en)
            val |= CMQ_E1_FRM_MAINT_OPT_NFAS_OOF_CRITERIA;

        switch (conf_info_p->mfrm_los_alig_crit) {
        case CMQ_E1_LOSS_MFRM_ALIGN_TS16_CRIT_ZERO_1_MFRM:
            val |= CMQ_E1_FRM_MAINT_OPT_OOMF_CRITERIA;
            break;
        case CMQ_E1_LOSS_MFRM_ALIGN_TS16_CRIT_ZERO_2_MFRM:
            val |= CMQ_E1_FRM_MAINT_OPT_OOMF_CRITERIA;
            val |= CMQ_E1_FRM_MAINT_OPT_OOMF_TSLOT16_CRITERIA;
            break;
        case CMQ_E1_LOSS_MFRM_ALIGN_TS16_CRIT_NONE:
        default:
            break;
        }

        switch (conf_info_p->ais_criteria) {
        case CMQ_E1_RAI_CRIT_4_CONSEC_A_1:
            val |= CMQ_E1_FRM_MAINT_OPT_RAI_CRITERIA;
            break;
        case CMQ_E1_RAI_CRIT_ALL_A_1:
        default:
            break;
        }
	callout_p->wr_frm_reg(frm_bar, CMQ_E1_FRMR_MAINT_MODE_OPT, val,
			      pmc4359->bus_width);

        /*
         * receive options, reg 0x02
         */
        val = callout_p->rd_frm_reg(frm_bar, CMQ_RX_OPTS, pmc4359->bus_width)
	    & ~(CMQ_RX_OPT_WRD_ERR | CMQ_RX_OPT_CNT_NFAS);
        if (conf_info_p->mult_faseo_en)
            val |= CMQ_RX_OPT_WRD_ERR;
        if (conf_info_p->nfas_err_en)
            val |= CMQ_RX_OPT_CNT_NFAS;

	callout_p->wr_frm_reg(frm_bar, CMQ_RX_OPTS, val, pmc4359->bus_width);
    }
}


/**********************************************************************
 *
 * Function: pm4359_rx_jat_conf
 *
 * This function: configures the receive JAT
 *
 * Input : dev - Pointer to the framer device object
 *         
 * Output: NONE
 *
 **********************************************************************
 */
static void 
pm4359_rx_jat_conf (dev_object_t *dev)
{
    uchar val, e1_mode, frm_num;
    ulong frm_bar;
    cmq_framer_conf_t *conf_info_p;
    dev_4359_object_t *pmc4359 = (dev_4359_object_t *)dev;
    dev_4359_callout_fvt_t *callout_p = pmc4359->callout_fvt;

    conf_info_p = pmc4359->cfg_info_p;
    frm_num = conf_info_p->quad_num;
    frm_bar = (ulong)pmc4359->base.dev_addr + (frm_num * CMQ_FRM_OFFSET * 
					       pmc4359->bus_width);
    e1_mode = callout_p->rd_frm_reg(frm_bar, CMQ_GLB_CFG, pmc4359->bus_width)
	& CMQ_GLBL_E1_MODE;

    /*
     * Rx options, reg 0x02
     */
    val = callout_p->rd_frm_reg(frm_bar, CMQ_RX_OPTS, pmc4359->bus_width) & 
	~(CMQ_RX_OPT_JAT_BYPASS | CMQ_RX_OPT_ELST_BYPASS);

    if (!conf_info_p->rjat_byp_en) {
        if ((!e1_mode) && (!(val & CMQ_RX_OPT_UNFRM))) {
	    /*            cterr('f',0,"RJAT cannot be disabled in framed T1");*/
            return;
        }
        val |= CMQ_RX_OPT_JAT_BYPASS;
    }
    if (conf_info_p->rx_elst_byp)
        val |= CMQ_RX_OPT_ELST_BYPASS;

    callout_p->wr_frm_reg(frm_bar, CMQ_RX_OPTS, val, pmc4359->bus_width);

    /*
     * RJAT reference clock disivor, reg 0x15
     */
    callout_p->wr_frm_reg(frm_bar, CMQ_RJAT_DIV_N1_CTL, 
			  conf_info_p->rjat_ref_div, pmc4359->bus_width);
    /*
     * RJAT output clock disivor, reg 0x16
     */
    callout_p->wr_frm_reg(frm_bar, CMQ_RJAT_DIV_N2_CTL, 
			  conf_info_p->rjat_output_div, pmc4359->bus_width);
    msleep(20);
    
    /*
     * RJAT configuration, 0x17
     */
    val = callout_p->rd_frm_reg(frm_bar, CMQ_RJAT_CFG, pmc4359->bus_width) & 
	~(CMQ_RJAT_CFG_FIFO_LIMIT | CMQ_RJAT_CFG_CNTR_FIFO_RD_PTR);

    if (conf_info_p->rjat_limit_ov_under)
        val |= CMQ_RJAT_CFG_FIFO_LIMIT;
    if (conf_info_p->rjat_fifo_cent)
        val |= CMQ_RJAT_CFG_CNTR_FIFO_RD_PTR;
    callout_p->wr_frm_reg(frm_bar, CMQ_RJAT_CFG, val, pmc4359->bus_width);
}


/**********************************************************************
 *
 * Function: pm4359_tx_jat_conf
 *
 * This function: configures the transmit JAT
 *
 * Input : dev - Pointer to the framer device object
 *         
 * Output: NONE
 *
 **********************************************************************
 */
static void 
pm4359_tx_jat_conf (dev_object_t *dev)
{
    uchar val, frm_num;
    ulong frm_bar;
    cmq_framer_conf_t *conf_info_p;
    dev_4359_object_t *pmc4359 = (dev_4359_object_t *)dev;
    dev_4359_callout_fvt_t *callout_p = pmc4359->callout_fvt;

    conf_info_p = pmc4359->cfg_info_p;
    frm_num = conf_info_p->quad_num;
    frm_bar = (ulong)pmc4359->base.dev_addr + (frm_num * CMQ_FRM_OFFSET * 
					       pmc4359->bus_width);

    /*
     * Tx line interface configuration, reg 0x04
     */
    val = callout_p->rd_frm_reg(frm_bar, CMQ_TX_LINE_IF_CFG, 
				pmc4359->bus_width) & 
	~CMQ_TX_LIF_CFG_JAT_BYPASS;

    if (!conf_info_p->tjat_byp_en) {
        val |= CMQ_TX_LIF_CFG_JAT_BYPASS;
    }
    callout_p->wr_frm_reg(frm_bar, CMQ_TX_LINE_IF_CFG, val, 
			  pmc4359->bus_width);

    /*
     * TJAT reference clock divisor, reg 0x19
     */
    callout_p->wr_frm_reg(frm_bar, CMQ_TJAT_DIV_N1_CTL,
			  conf_info_p->tjat_ref_div, pmc4359->bus_width);

    /*
     * TJAT output clock divisor, 0x1A
     */
    callout_p->wr_frm_reg(frm_bar, CMQ_TJAT_DIV_N2_CTL,
			  conf_info_p->tjat_output_div, pmc4359->bus_width);
    msleep(20);

    /*
     * TJAT configuration, reg 0x1B
     */
    val = callout_p->rd_frm_reg(frm_bar, CMQ_TJAT_CFG, pmc4359->bus_width) & 
	~(CMQ_TJAT_CFG_FIFO_LIMIT | CMQ_TJAT_CFG_CNTR_FIFO_RD_PTR);

    if (conf_info_p->tjat_limit_ov_under)
        val |= CMQ_TJAT_CFG_FIFO_LIMIT;
    if (conf_info_p->tjat_fifo_cent)
        val |= CMQ_TJAT_CFG_CNTR_FIFO_RD_PTR;

    callout_p->wr_frm_reg(frm_bar, CMQ_TJAT_CFG, val, pmc4359->bus_width);

    /*
     * Tx timing options, reg 0x06
     */
    val = callout_p->rd_frm_reg(frm_bar, CMQ_TX_TIMING_OPTS, 
				pmc4359->bus_width) & 
	~(CMQ_TX_TIME_OPT_OUTPUT_CLK_SEL | CMQ_TX_TIME_OPT_PLL_REF_CLK |
	  CMQ_TX_TIME_OPT_ELST_BYPASS);

    switch (conf_info_p->pll_ref_clock) {
    case CMQ_TJAT_PLL_REF_CLK_FIFO_INPUT:
        val |= CMQ_TX_TIME_OPT_PLL_REF_CLK_JAT;
        break;
    case CMQ_TJAT_PLL_REF_CLK_BACKPLANE:
        val |= CMQ_TX_TIME_OPT_PLL_REF_CLK_BTIF;
        break;
    case CMQ_TJAT_PLL_REF_CLK_RECOVERED:
        val |= CMQ_TX_TIME_OPT_PLL_REF_CLK_RECOVER;
        break;
    case CMQ_TJAT_PLL_REF_CLK_CTCLK:
        val |= CMQ_TX_TIME_OPT_PLL_REF_CLK_COMMON;
        break;
    default:
        break;
    }
    switch (conf_info_p->out_clock) {
    case CMQ_TJAT_OUTPUT_CLK_INTERN_JAT:
        val |= CMQ_TX_TIME_OPT_OUTPUT_CLK_JAT;
        break;
    case CMQ_TJAT_OUTPUT_CLK_CTCLK:
        val |= CMQ_TX_TIME_OPT_OUTPUT_CLK_INTERN |
               CMQ_TX_TIME_OPT_PLL_REF_CLK_COMMON;
        break;
    case CMQ_TJAT_OUTPUT_CLK_FIFO_INPUT:
        val |= CMQ_TX_TIME_OPT_OUTPUT_CLK_FIFO |
               CMQ_TX_TIME_OPT_PLL_REF_CLK_JAT;
        break;
    default:
        break;
    }

    if (conf_info_p->tx_elst_byp)
        val |= CMQ_TX_TIME_OPT_ELST_BYPASS;

    callout_p->wr_frm_reg(frm_bar, CMQ_TX_TIMING_OPTS, val, 
			  pmc4359->bus_width);
}


/**********************************************************************
 *
 * Function: pm4359_rx_clk_conf
 *
 * This function: configures the receive clock.
 *
 * Input : dev - Pointer to the framer device object
 *         
 * Output: NONE
 *
 **********************************************************************
 */
static void 
pm4359_rx_clk_conf (dev_object_t *dev)
{
    ulong frm_bar;
    uchar val, frm_num, op_mode;
    cmq_framer_conf_t *conf_info_p;
    dev_4359_object_t *pmc4359 = (dev_4359_object_t *)dev;
    dev_4359_callout_fvt_t *callout_p = pmc4359->callout_fvt;

    conf_info_p = pmc4359->cfg_info_p;
    frm_num = conf_info_p->quad_num;
    frm_bar = (ulong)pmc4359->base.dev_addr + (frm_num * CMQ_FRM_OFFSET * 
					       pmc4359->bus_width);
    op_mode = conf_info_p->op_mode;

    /*
     * CDRC configure, reg 0x10
     */
    val   = callout_p->rd_frm_reg(frm_bar, CMQ_CDRC_CFG, pmc4359->bus_width) &
	~(CMQ_CDRC_CFG_LOS_THRESH_MASK | CMQ_CDRC_CFG_PLL_ALG_SEL);

    if (conf_info_p->recover_clk_sel == CMQ_RECOVER_CLK_HIGH_FREQ_JAT) {
	/* Comment from Hoang: should this be |= instead of = ??? */
        val = CMQ_CDRC_CFG_PLL_ALG_SEL;
    }
    switch (conf_info_p->los_thres) {
    case CMQ_LOS_THRESH_PCM_10_HDB3:
        if (op_mode == CMQ_MODE_E1) {
            val |= CMQ_CDRC_CFG_LOS_E1_PCM10;
            val &= ~CMQ_CDRC_CFG_AMI;
        }
        break;

    case CMQ_LOS_THRESH_PCM_15_B8ZS:
        if (op_mode == CMQ_MODE_T1) {
            val |= CMQ_CDRC_CFG_LOS_PCM15;
            val &= ~CMQ_CDRC_CFG_AMI;
        }
        break;

    case CMQ_LOS_THRESH_PCM_15_AMI:
        val |= CMQ_CDRC_CFG_LOS_PCM15 |
               CMQ_CDRC_CFG_AMI;
        break;

    case CMQ_LOS_THRESH_PCM_31:
        val |= CMQ_CDRC_CFG_LOS_PCM31;
        break;

    case CMQ_LOS_THRESH_PCM_63:
	/* Comment from Hoang: should this be |= instead of = ??? */
        val = CMQ_CDRC_CFG_LOS_PCM63;
        break;

    case CMQ_LOS_THRESH_PCM_175:
        val |= CMQ_CDRC_CFG_LOS_PCM175;
        break;

    default:
        break;
    }
    callout_p->wr_frm_reg(frm_bar, CMQ_CDRC_CFG, val, pmc4359->bus_width);
}


/**********************************************************************
 *
 * Function: pm4359_bp_clk_conf()
 *
 * This function: Set the System side backplane receive interface
 *
 * Input : dev - Pointer to the framer device object
 *         
 * Output: NONE
 *
 **********************************************************************
 */
static void 
pm4359_bp_clk_conf (dev_object_t *dev)
{
    uchar val, frm_num;
    ulong frm_bar;
    cmq_framer_conf_t *conf_info_p;
    dev_4359_object_t *pmc4359 = (dev_4359_object_t *)dev;
    dev_4359_callout_fvt_t *callout_p = pmc4359->callout_fvt;

    conf_info_p = pmc4359->cfg_info_p;
    frm_num = conf_info_p->quad_num;
    frm_bar = (ulong)pmc4359->base.dev_addr + (frm_num * CMQ_FRM_OFFSET * 
					       pmc4359->bus_width);

    /*
     * BRIF configuration, reg 0x30
     */
    val = callout_p->rd_frm_reg(frm_bar, CMQ_BRIF_CFG,
				pmc4359->bus_width) & ~CMQ_BRIF_CFG_MASK;
    if (conf_info_p->brif_mas_mode) {
        switch (conf_info_p->brif_data_mode) {
        case CMQ_BACKPLANE_FULL_FRAME_MODE:
            break;
        case CMQ_BACKPLANE_NX56K_MODE:
            val |= CMQ_BRIF_CFG_56K_NXDS0;
            break;
        case CMQ_BACKPLANE_NX64K_MODE:
            val |= CMQ_BRIF_CFG_64K_NXDS0;
            break;
        case CMQ_BACKPLANE_NX64K_E1_MODE:
            val |= CMQ_BRIF_CFG_64K_NXDS0_FBIT;
            break;
        }
    } else {
        val |= CMQ_BRIF_CFG_CLK_SLAVE;
    }

    if (conf_info_p->brif_clkx2) {
        val |= CMQ_BRIF_CFG_CLK_FREQ_TIMES2;
    }
    switch (conf_info_p->brif_data_rate) {
        case CMQ_BACKPLANE_CLK_RATE_1544:
            val |= CMQ_BRIF_CFG_CLK_RATE_SEL_1544;
            break;

        case CMQ_BACKPLANE_CLK_RATE_2048:
            val |= CMQ_BRIF_CFG_CLK_RATE_SEL_2048;
            break;

        case CMQ_BACKPLANE_CLK_RATE_8192:
            val |= CMQ_BRIF_CFG_CLK_RATE_SEL_8192;
            break;
        default:
            break;
    }
    if (conf_info_p->brif_de_hi) {
        val |= CMQ_BRIF_CFG_CLK_EDGE_HI_DATA;
    }
    if (conf_info_p->brif_fe_hi) {
        val |= CMQ_BRIF_CFG_CLK_EDGE_HI_FRM;
    }

    callout_p->wr_frm_reg(frm_bar, CMQ_BRIF_CFG, val, pmc4359->bus_width);

    /*
     * BTIF configuration, reg 0x40
     */
    val = callout_p->rd_frm_reg(frm_bar, CMQ_BTIF_CFG,
				pmc4359->bus_width) & ~CMQ_BTIF_CFG_MASK;
    if (conf_info_p->btif_mas_mode) {
        switch (conf_info_p->btif_data_mode) {
        case CMQ_BACKPLANE_FULL_FRAME_MODE:
            break;
        case CMQ_BACKPLANE_NX56K_MODE:
            val |= CMQ_BTIF_CFG_56K_NXDS0;
            break;
        case CMQ_BACKPLANE_NX64K_MODE:
            val |= CMQ_BTIF_CFG_64K_NXDS0;
            break;
        case CMQ_BACKPLANE_NX64K_E1_MODE:
            val |= CMQ_BTIF_CFG_64K_NXDS0_FBIT;
            break;
        }
    } else {
        val |= CMQ_BTIF_CFG_CLK_SLAVE;
    }

    if (conf_info_p->btif_clkx2) {
        val |= CMQ_BTIF_CFG_CLK_FREQ_TIMES2;
    }
    switch (conf_info_p->btif_data_rate) {
        case CMQ_BACKPLANE_CLK_RATE_1544:
            val |= CMQ_BTIF_CFG_CLK_RATE_SEL_1544;
            break;

        case CMQ_BACKPLANE_CLK_RATE_2048:
            val |= CMQ_BTIF_CFG_CLK_RATE_SEL_2048;
            break;

        case CMQ_BACKPLANE_CLK_RATE_8192:
            val |= CMQ_BTIF_CFG_CLK_RATE_SEL_8192;
            break;
        default:
            break;
    }
    if (conf_info_p->btif_de_hi) {
        val |= CMQ_BTIF_CFG_CLK_EDGE_HI_DATA;
    }
    if (conf_info_p->btif_fe_hi) {
        val |= CMQ_BTIF_CFG_CLK_EDGE_HI_FRM;
    }

    callout_p->wr_frm_reg(frm_bar, CMQ_BTIF_CFG, val, pmc4359->bus_width);
}


/**********************************************************************
 *
 * Function: pm4359_bp_fp_conf
 *
 * This function: Set the System side backplane receive interface
 *
 * Input : dev - Pointer to the framer device object
 *         
 * Output: NONE
 *
 **********************************************************************
 */
static void 
pm4359_bp_fp_conf (dev_object_t *dev)
{
    uchar val, frm_num;
    ulong frm_bar;
    cmq_framer_conf_t *conf_info_p;
    dev_4359_object_t *pmc4359 = (dev_4359_object_t *)dev;
    dev_4359_callout_fvt_t *callout_p = pmc4359->callout_fvt;

    conf_info_p = pmc4359->cfg_info_p;
    frm_num = conf_info_p->quad_num;
    frm_bar = (ulong)pmc4359->base.dev_addr + (frm_num * CMQ_FRM_OFFSET * 
					       pmc4359->bus_width);

    /*
     * BRIF frame pulse configuration, reg 0x31
     */
    val = callout_p->rd_frm_reg(frm_bar, CMQ_BRIF_FRM_PULSE_CFG,
				pmc4359->bus_width) & 
	~(CMQ_BRIF_FRM_PULSE_CFG_MASK | CMQ_BRIF_FRM_PULSE_CFG_MODE);

    if (conf_info_p->r_fp_mas_mode) {
        switch (conf_info_p->r_fp_mode) {
        case CMQ_BACKPLANE_RX_FP_T1_HIGH_ON_SF_ESF:
            val |= CMQ_BRIF_FRM_PULSE_CFG_BRXSMFP;
            break;
        case CMQ_BACKPLANE_RX_FP_E1_HIGH_ON_CRC_MFRM:
            val |= CMQ_BRIF_FRM_PULSE_CFG_BRXCMFP;
            break;
        case CMQ_BACKPLANE_RX_FP_E1_HIGH_ON_SIG_MFRM:
            val |= CMQ_BRIF_FRM_PULSE_CFG_BRXSMFP;
            break;
        case CMQ_BACKPLANE_RX_FP_E1_COMP_MFRM:
            val |= CMQ_BRIF_FRM_PULSE_CFG_BRXSMFP |
                   CMQ_BRIF_FRM_PULSE_CFG_BRXCMFP;
            break;
        case CMQ_BACKPLANE_RX_FP_E1_HIGH_ON_OVERHEAD:
            val |= CMQ_BRIF_FRM_PULSE_CFG_ROHM;
            break;
        case CMQ_BACKPLANE_RX_FP_T1E1_HIGH_EVERY_FRAME:
        default:
            break;
        }
    } else
        val |= CMQ_BRIF_FRM_PULSE_SLAVE;

    if (conf_info_p->r_fp_inv_en)
        val |= CMQ_BRIF_FRM_PULSE_CFG_INV;

    if (conf_info_p->r_alt_fdl_en)
        val |= CMQ_BRIF_FRM_PULSE_EN_ALT_FDL;
    if (conf_info_p->r_tslot_map_format == CMQ_BACKPLANE_TIMESLOT_MAP_24_OF_32)
        val |= CMQ_BRIF_FRM_PULSE_CFG_24_TSLOTS;

    /*
     * if bit RXELSTBP of reg 0x02 is 1, then FPMODE bit must be 0
     */
    if (conf_info_p->rx_elst_byp)
       val &= ~CMQ_BRIF_FRM_PULSE_SLAVE;

    callout_p->wr_frm_reg(frm_bar, CMQ_BRIF_FRM_PULSE_CFG, val,
			  pmc4359->bus_width);

    /*
     * parity/F-bit config: reg 0x32
     */
    val = callout_p->rd_frm_reg(frm_bar, CMQ_BRIF_PAR_FBIT_CFG,
				pmc4359->bus_width) & 
	~(CMQ_BRIF_PAR_FBIT_CFG_PARITY_INS_EN | CMQ_BRIF_PAR_FBIT_CFG_ODD |
	  CMQ_BRIF_PAR_FBIT_CFG_PAR_EXTEND | CMQ_BRIF_PAR_FBIT_CFG_FIXF |
	  CMQ_BRIF_PAR_FBIT_CFG_HIGH);

    if (conf_info_p->r_par_ins_en) {
        val |= CMQ_BRIF_PAR_FBIT_CFG_PARITY_INS_EN;
        if (conf_info_p->r_odd_par)
            val |= CMQ_BRIF_PAR_FBIT_CFG_ODD;
    }
    if (conf_info_p->r_ext_par_en)
        val |= CMQ_BRIF_PAR_FBIT_CFG_PAR_EXTEND;
    if (conf_info_p->r_fbit_fix && !conf_info_p->r_par_ins_en) {
        val |= CMQ_BRIF_PAR_FBIT_CFG_FIXF;
        if (conf_info_p->r_fbit_pol)
            val |= CMQ_BRIF_PAR_FBIT_CFG_HIGH;
    }
 
    callout_p->wr_frm_reg(frm_bar, CMQ_BRIF_PAR_FBIT_CFG, val, 
			  pmc4359->bus_width);

    /*
     * time slot offset: reg 0x33
     * FIX for a known issue with COMET family device (if Tx_data = 3F3F
     * and Rx_data = F9F9, 1-bit shift with 2.048M Slave BRIF in T1 Mode).
     * Description:
     * Following initialization there is a small possibility that the backplane
     * receive interface (BRIF) and the receive elastic store buffer (Rx-ELST)
     * will be ot of sync with each other.  When the BRIF and the Rx-ELST are
     * out of sync, the data from the elastic store buffer is shifted by one
     * bit with respect to the frame pulse.  Resetting the frame pulse alignment
     * of the elastic store buffer forces the data to re-align.
     * Workaround:
     * - write hex value 0x01 to reg 0xn33
     * - write hex value 0x00 to reg 0xn33
     */
    callout_p->wr_frm_reg(frm_bar, CMQ_BRIF_TSLOT_OFFSET, 1,
                          pmc4359->bus_width);
    callout_p->wr_frm_reg(frm_bar, CMQ_BRIF_TSLOT_OFFSET, 
			  (conf_info_p->r_fp_frm_offset & 
			   CMQ_BRIF_TSLOT_BYTE_OFFSET_MASK), 
			  pmc4359->bus_width);

    /*
     * bit offset: reg 0x34
     */
    if (conf_info_p->r_fp_bit_offset_en) {
        val = CMQ_BRIF_FP_BIT_OFFSET_EN | (conf_info_p->r_fpBitOffset & 
					   CMQ_BRIF_FP_BIT_OFFSET_MASK);
	callout_p->wr_frm_reg(frm_bar, CMQ_BRIF_BIT_OFFSET, val, 
			      pmc4359->bus_width);
    }

    /*
     * BTIF frame pulse configuration, 0x41
     */
    val = callout_p->rd_frm_reg(frm_bar, CMQ_BTIF_FRM_PULSE_CFG,
				pmc4359->bus_width) & 
	~(CMQ_BTIF_FRM_PULSE_SLAVE | CMQ_BTIF_FRM_PULSE_CFG_INV |
	  CMQ_BTIF_FRM_PULSE_CFG_24_TSLOTS | CMQ_BTIF_FRM_PULSE_ESF_EN);

    if (!conf_info_p->t_fp_mas_mode) {
        val |= CMQ_BTIF_FRM_PULSE_SLAVE;
        if (conf_info_p->t_fp_inv_en)
            val |= CMQ_BTIF_FRM_PULSE_CFG_INV;
    }
    if (conf_info_p->t_t1_esf_align)
        val |= CMQ_BTIF_FRM_PULSE_ESF_EN;
    if (conf_info_p->t_tslot_map_format == CMQ_BACKPLANE_TIMESLOT_MAP_24_OF_32)
        val |= CMQ_BTIF_FRM_PULSE_CFG_24_TSLOTS;

#ifdef ALW
    /*
     * adding this in make T1 loopback fail using firebird
     * whereas it will pass without this
     */
    val |= CMQ_BTIF_FRM_PULSE_TYPE;
    callout_p->wr_frm_reg(frm_bar, CMQ_BTIF_FRM_PULSE_CFG, val, 
			  pmc4359->bus_width);

    msleep(5);
    val &= ~CMQ_BTIF_FRM_PULSE_TYPE;
#endif
    callout_p->wr_frm_reg(frm_bar, CMQ_BTIF_FRM_PULSE_CFG, val, 
			  pmc4359->bus_width);

    /*
     * parity configuation, reg 0x42
     */
    val = callout_p->rd_frm_reg(frm_bar, CMQ_BTIF_PAR_STAT_CFG, 
				pmc4359->bus_width) & 
	~(CMQ_BTIF_PAR_CFG_ODD | CMQ_BTIF_PAR_CFG_EXT);

    if (conf_info_p->t_odd_par)
        val |= CMQ_BTIF_PAR_CFG_ODD;
    if (conf_info_p->t_ext_par_en)
        val |= CMQ_BTIF_PAR_CFG_EXT;
    callout_p->wr_frm_reg(frm_bar, CMQ_BTIF_PAR_STAT_CFG, val, 
			  pmc4359->bus_width);

    /*
     * time slot offset, reg 0x43
     */
    val = conf_info_p->t_fp_frm_offset & (CMQ_BTIF_TSLOT_BYTE_OFFSET_MASK);
    callout_p->wr_frm_reg(frm_bar, CMQ_BTIF_TSLOT_OFFSET, val, 
			  pmc4359->bus_width);
    
    /*
     * bit offset, reg 0x44
     */
    if (conf_info_p->t_fp_bitoffset_en) {
	val = CMQ_BTIF_FP_BIT_OFFSET_EN | (conf_info_p->t_fp_bitoffset & 
					   CMQ_BTIF_FP_BIT_OFFSET_MASK);
	callout_p->wr_frm_reg(frm_bar, CMQ_BTIF_BIT_OFFSET, val,
			      pmc4359->bus_width);
    }
}


/**********************************************************************
 *
 * Function: pm4359_clk_sync_conf()
 *
 * This function: Configures the line transmit interface clock frequency.
 *
 * Input : dev - Pointer to the framer device object
 *         
 * Output: NONE
 *
 **********************************************************************
 */
static void 
pm4359_clk_sync_conf (dev_object_t *dev)
{
    uchar val;
    dev_4359_object_t *pmc4359 = (dev_4359_object_t *)dev;
    dev_4359_callout_fvt_t *callout_p = pmc4359->callout_fvt;

    /*
     * since only 0xD6 is valid. 0x1d6, 0x2d6, 0x3d6 are invalid, we
     * don't need to calculate the offset.
     */
    /*
     * Clock synthesis unit, reg 0xD6
     */
    val = callout_p->rd_frm_reg((ulong)pmc4359->base.dev_addr, CMQ_CSU_CFG,
				pmc4359->bus_width) &
	~CMQ_CSU_CFG_XCLK_TX_MASK;

    switch (pmc4359->cfg_info_p->synth_tx_freq) {
        case CMQ_XCLK_2048_TXCLK_2048:
            val |= CMQ_CSU_CFG_XCLK2048_TX2048;
            break;
        case CMQ_XCLK_1544_TXCLK_1544:
            val |= CMQ_CSU_CFG_XCLK1544_TX1544;
            break;
        case CMQ_XCLK_2048_TXCLK_1544:
            val |= CMQ_CSU_CFG_XCLK2048_TX1544;
            break;
        default:
            break;
    }
    callout_p->wr_frm_reg((ulong)pmc4359->base.dev_addr, CMQ_CSU_CFG, val,
			  pmc4359->bus_width);

    val = callout_p->rd_frm_reg((ulong)pmc4359->base.dev_addr, CMQ_CSU_CFG,
				pmc4359->bus_width);

    /*
     * wait to let CSU lock (PMC application engineer recommends that
     * the wait time be at least 50msecs)
     */
    msleep(100);
}


/**********************************************************************
 *
 * Function: pm4359_rx_line_conf()
 *
 * This function configures receive line.
 *
 * Input : dev - Pointer to the framer device object
 *         
 * Output: NONE
 *
 **********************************************************************
 */
static void 
pm4359_rx_line_conf (dev_object_t *dev)
{
    ulong frm_bar;
    uchar frm_num, val;
    cmq_framer_conf_t *conf_info_p;
    dev_4359_object_t *pmc4359 = (dev_4359_object_t *)dev;
    dev_4359_callout_fvt_t *callout_p = pmc4359->callout_fvt;

    conf_info_p = pmc4359->cfg_info_p;
    frm_num = conf_info_p->quad_num;
    frm_bar = (ulong)pmc4359->base.dev_addr + (frm_num * CMQ_FRM_OFFSET * 
					       pmc4359->bus_width);

    /*
     * RLPS configuration, reg 0xF8
     */
    val = (callout_p->rd_frm_reg(frm_bar, CMQ_RLPS_CFG_STAT, 
				 pmc4359->bus_width) & 
	   ~CMQ_RLPS_CFG_EN_SQUELCH) | CMQ_RLPS_CFG_RESERVE;

    if (conf_info_p->squelch_en)
        val |=  CMQ_RLPS_CFG_EN_SQUELCH;
    callout_p->wr_frm_reg(frm_bar, CMQ_RLPS_CFG_STAT, val, 
			  pmc4359->bus_width);

    /*
     * RLPS ALOS detection/clearance threshold, reg 0xf9
     */
    val = callout_p->rd_frm_reg(frm_bar, CMQ_RLPS_ALOS_DET_CLRNCE_THRESH,
				pmc4359->bus_width) & 
			       ~(CMQ_RLPS_ALOS_CLRNC_THRESH_MASK |
				 CMQ_RLPS_ALOS_DET_THRESH_MASK);
    switch (conf_info_p->alos_thres) {
    case CMQ_RX_ALOS_9DB_THRESH:
           val |= CMQ_RLPS_ALOS_CLRNC_THRESH_DB09 |
                  CMQ_RLPS_ALOS_DET_THRESH_DB09;
           break;
        case CMQ_RX_ALOS_14_5DB_THRESH:
           val |= CMQ_RLPS_ALOS_CLRNC_THRESH_DB145 |
                  CMQ_RLPS_ALOS_DET_THRESH_DB145;
           break;
        case CMQ_RX_ALOS_20DB_THRESH:
           val |= CMQ_RLPS_ALOS_CLRNC_THRESH_DB20 |
                  CMQ_RLPS_ALOS_DET_THRESH_DB20;
           break;
        case CMQ_RX_ALOS_22DB_THRESH:
           val |= CMQ_RLPS_ALOS_CLRNC_THRESH_DB22 |
                  CMQ_RLPS_ALOS_DET_THRESH_DB22;
           break;
        case CMQ_RX_ALOS_25DB_THRESH:
           val |= CMQ_RLPS_ALOS_CLRNC_THRESH_DB25 |
                  CMQ_RLPS_ALOS_DET_THRESH_DB25;
           break;
        case CMQ_RX_ALOS_30DB_THRESH:
           val |= CMQ_RLPS_ALOS_CLRNC_THRESH_DB30 |
                  CMQ_RLPS_ALOS_DET_THRESH_DB30;
           break;
        case CMQ_RX_ALOS_31DB_THRESH:
           val |= CMQ_RLPS_ALOS_CLRNC_THRESH_DB31 |
                  CMQ_RLPS_ALOS_DET_THRESH_DB31;
           break;
        case CMQ_RX_ALOS_35DB_THRESH:
           val |= CMQ_RLPS_ALOS_CLRNC_THRESH_DB35 |
                  CMQ_RLPS_ALOS_DET_THRESH_DB35;
           break;
    }
    callout_p->wr_frm_reg(frm_bar, CMQ_RLPS_ALOS_DET_CLRNCE_THRESH, val,
			  pmc4359->bus_width);

    /*
     * RLPS ALOS detection period, reg 0xFA
     */
    callout_p->wr_frm_reg(frm_bar, CMQ_RLPS_ALOS_DET_PERIOD, 
			  conf_info_p->alos_det_period, pmc4359->bus_width);

    /*
     * RLPS ALOS clearance period, reg 0xFB 
     */
    callout_p->wr_frm_reg(frm_bar, CMQ_RLPS_ALOS_CLRNCE_PERIOD, 
			  conf_info_p->alos_clr_period, pmc4359->bus_width);
    /*
     * quiesce the RLPS RAM
     */
    callout_p->wr_frm_reg(frm_bar, CMQ_RLPS_EQ_RDWR_SEL, CMQ_RLPS_EQ_RD,
			  pmc4359->bus_width);
    callout_p->wr_frm_reg(frm_bar, CMQ_RLPS_EQ_IND_ADDR, 0, 
			  pmc4359->bus_width);
    callout_p->wr_frm_reg(frm_bar, CMQ_RLPS_EQ_LOOP_STAT_CTL, 0,
			  pmc4359->bus_width);
    callout_p->wr_frm_reg(frm_bar, CMQ_RLPS_EQ_CFG, CMQ_RLPS_EQ_CFG_VALU, 
			  pmc4359->bus_width);
}


/**********************************************************************
 *
 * Function: pm4359_conf_rlps_ram()
 *
 * This function: Configures the rx equalizer ram
 *
 * Input : dev - Pointer to the framer device object
 *         ram - pointer to RAM table
 *         
 * Output: NONE
 *
 **********************************************************************
 */
static void 
pm4359_conf_rlps_ram (dev_object_t *dev, ulong *ram)
{
    ulong val, frm_bar, addr;
    uchar frm_num;
    cmq_framer_conf_t *conf_info_p;
    dev_4359_object_t *pmc4359 = (dev_4359_object_t *)dev;
    dev_4359_callout_fvt_t *callout_p = pmc4359->callout_fvt;

    conf_info_p = pmc4359->cfg_info_p;
    frm_num = conf_info_p->quad_num;
    frm_bar = (ulong)pmc4359->base.dev_addr + (frm_num * CMQ_FRM_OFFSET * 
					       pmc4359->bus_width);

    /*
     * before we proceed programming the RAM, the the equalizer feedback loop
     * enable bit in the RLPS equalizer config register must be set to 1
     */
    val = callout_p->rd_frm_reg(frm_bar, CMQ_RLPS_EQ_CFG, pmc4359->bus_width);
    callout_p->wr_frm_reg(frm_bar, CMQ_RLPS_EQ_CFG, val | CMQ_RLPS_EQ_CFG_VALU,
			  pmc4359->bus_width);

    for (addr = 0; addr < CMQ_RLPS_EQUALIZER_RAM_SIZE; addr++) {
        val = ram[addr];
        /*
         * write the 32 bit value, byte by byte, to the appropriate
         * indirect data register
         */
	callout_p->wr_frm_reg(frm_bar, CMQ_RLPS_IND_DATA_REG1, 
			      (val>>24) & 0x000000FF, pmc4359->bus_width);

	callout_p->wr_frm_reg(frm_bar, CMQ_RLPS_IND_DATA_REG2, 
			      (val>>16) & 0x000000FF, pmc4359->bus_width);

	callout_p->wr_frm_reg(frm_bar, CMQ_RLPS_IND_DATA_REG3, 
			      (val>>8) & 0x000000FF, pmc4359->bus_width);

	callout_p->wr_frm_reg(frm_bar, CMQ_RLPS_IND_DATA_REG4, 
			      val & 0x000000FF, pmc4359->bus_width);

        /*
         * select indirect write
         */
	callout_p->wr_frm_reg(frm_bar, CMQ_RLPS_EQ_RDWR_SEL, CMQ_RLPS_EQ_WR,
			      pmc4359->bus_width);

        /*
         * setup indirect offset to write to
         */
	callout_p->wr_frm_reg(frm_bar, CMQ_RLPS_EQ_IND_ADDR, addr,
			      pmc4359->bus_width);
        msleep(5);
    }

    val = callout_p->rd_frm_reg(frm_bar, CMQ_RLPS_EQ_CFG, pmc4359->bus_width);
    callout_p->wr_frm_reg(frm_bar, CMQ_RLPS_EQ_CFG, val | CMQ_RLPS_EQ_CFG_VALU,
			  pmc4359->bus_width);
}

/**********************************************************************
 *
 * Function: pmc4359_read_rlps()
 *
 * This function: reads the contents of the rlps ram and verifies
 * against the previously written value or dumps it
 *
 * Input : dev - Pointer to the framer device object
 *         flag - 1 = dump, 0 = verify
 *         
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int 
pmc4359_read_rlps (dev_object_t *dev, ulong flag)
{
    ulong rd_val, wr_val, val, val1, val2, val3, val4, addr, frm_bar;
    uchar ram_type, op_mode, frm_num;
    ulong *ram;
    cmq_framer_conf_t *conf_info_p;
    dev_4359_object_t *pmc4359 = (dev_4359_object_t *)dev;
    dev_4359_callout_fvt_t *callout_p = pmc4359->callout_fvt;

    conf_info_p = pmc4359->cfg_info_p;
    frm_num = conf_info_p->quad_num;
    frm_bar = (ulong)pmc4359->base.dev_addr + (frm_num * CMQ_FRM_OFFSET *
                                               pmc4359->bus_width);
    op_mode = conf_info_p->op_mode;
    ram_type = (op_mode == CMQ_MODE_T1) ? CMQ_RX_LINE_EQ_RAM_T1 :
                                          CMQ_RX_LINE_EQ_RAM_E1;
    ram = (ulong *)pm4359_rlps_ram[ram_type];

    /*
     * before we proceed to read the RAM, the the equalizer feedback loop
     * enable bit in the RLPS equalizer config register must be set to 1
     */
    val = callout_p->rd_frm_reg(frm_bar, CMQ_RLPS_EQ_CFG, pmc4359->bus_width);
    callout_p->wr_frm_reg(frm_bar, CMQ_RLPS_EQ_CFG, val | CMQ_RLPS_EQ_CFG_VALU,
			  pmc4359->bus_width);

    for (addr = 0; addr < CMQ_RLPS_EQUALIZER_RAM_SIZE; addr++) {
	if (((addr & 7) == 0) && flag) {
	    printf("\naddr %#.2x ", addr);
	}

        /*
         * select indirect read
         */
	callout_p->wr_frm_reg(frm_bar, CMQ_RLPS_EQ_RDWR_SEL, CMQ_RLPS_EQ_RD,
			      pmc4359->bus_width);

        /*
         * setup indirect offset to read from
         */
	callout_p->wr_frm_reg(frm_bar, CMQ_RLPS_EQ_IND_ADDR, addr,
			      pmc4359->bus_width);

        /*
         * perform the read, byte by byte, from the appropriate
         * indirect data register
         */
        val1 = callout_p->rd_frm_reg(frm_bar, CMQ_RLPS_IND_DATA_REG1,
                              pmc4359->bus_width);

        val2 = callout_p->rd_frm_reg(frm_bar, CMQ_RLPS_IND_DATA_REG2,
                              pmc4359->bus_width);

        val3 = callout_p->rd_frm_reg(frm_bar, CMQ_RLPS_IND_DATA_REG3,
                              pmc4359->bus_width);

        val4 = callout_p->rd_frm_reg(frm_bar, CMQ_RLPS_IND_DATA_REG4,
                              pmc4359->bus_width);

        wr_val = ram[addr];
 	rd_val = (((val1 & 0xff) << 24) | ((val2 & 0xff) << 16) |
		  ((val3 & 0xff) << 8) | val4);
	if (flag) {
	    printf(" %.8x", rd_val);
	} else {
	    if (rd_val != wr_val) {
                sprintf(log_buffer, "%s1 RLPS ram read error at addr %#.2x,"
		    " expect %#.8x, read %#.8x",
                    op_mode == CMQ_MODE_T1 ? "T" : "E", addr, rd_val, wr_val);
                (*pmc4359->base.dev_object_fvt->dev_error_report)
                    (&pmc4359->base, log_buffer, FATAL);
                return(FAIL);
	        break;
	    }
	}
    }
    return(PASSED);
}


/**********************************************************************
 *
 * Function: pmc4359_dev_show()
 *
 * This function: prints pm4359 registers' value
 *
 * Input: dev_object_t pointer to the PMC device
 *        A device print function vector
 *        A dev_show_cmd_e command - DEV_SHOW_ALL to show all 
 *        PMC4359 registers or DEV_SHOW_STATUS to show just the
 *        status register. 
 * Output: Always PASSED but return uint32 for compatibility
 *
 **********************************************************************
 */
static uint32 
pmc4359_dev_show (dev_object_t *dev, print_fn_t dev_print, dev_show_cmd_e cmd)
{
    ulong base_addr;
    uchar frm_num;
    dev_4359_object_t *pmc4359 = (dev_4359_object_t *)dev;
    dev_4359_callout_fvt_t *callout_p = pmc4359->callout_fvt;

    frm_num = pmc4359->cfg_info_p->quad_num;
    base_addr = (ulong)pmc4359->base.dev_addr + (frm_num * CMQ_FRM_OFFSET * 
						 pmc4359->bus_width);

    dev_print("\nFramer Comet Tetra %d Base Address: %#.8x\n", frm_num, 
	   base_addr);

    if (cmd != DEV_SHOW_STATUS) {
	/* 0x00 */
	dev_print("Offset %#.2x, CMQ_GLB_CFG                 = %#.2x\n",
		  CMQ_GLB_CFG, callout_p->rd_frm_reg(base_addr, CMQ_GLB_CFG,
						     pmc4359->bus_width));
    }
    /* 0x01 */
    dev_print("Offset %#.2x, CMQ_CLK_MON                 = %#.2x\n",
	      CMQ_CLK_MON, callout_p->rd_frm_reg(base_addr, CMQ_CLK_MON,
						 pmc4359->bus_width));

    if (cmd != DEV_SHOW_STATUS) {
	/* 0x02 */
	dev_print("       %#.2x, CMQ_RX_OPTS                 = %#.2x\n",
		  CMQ_RX_OPTS, callout_p->rd_frm_reg(base_addr, CMQ_RX_OPTS,
						     pmc4359->bus_width));
	/* 0x03 */
	dev_print("       %#.2x, CMQ_RX_LINE_IF_CFG          = %#.2x\n",
		  CMQ_RX_LINE_IF_CFG, 
		  callout_p->rd_frm_reg(base_addr, CMQ_RX_LINE_IF_CFG,
					pmc4359->bus_width));
	/* 0x04 */
	dev_print("       %#.2x, CMQ_TX_LINE_IF_CFG          = %#.2x\n",
		  CMQ_TX_LINE_IF_CFG,
		  callout_p->rd_frm_reg(base_addr, CMQ_TX_LINE_IF_CFG,
					pmc4359->bus_width));
	/* 0x05 */
	dev_print("       %#.2x, CMQ_TX_FRM_BYPASS_OPTS      = %#.2x\n",
		  CMQ_TX_FRM_BYPASS_OPTS,
		  callout_p->rd_frm_reg(base_addr, CMQ_TX_FRM_BYPASS_OPTS,
					pmc4359->bus_width));
	/* 0x06 */
	dev_print("       %#.2x, CMQ_TX_TIMING_OPTS          = %#.2x\n",
		  CMQ_TX_TIMING_OPTS,
		  callout_p->rd_frm_reg(base_addr, CMQ_TX_TIMING_OPTS,
					pmc4359->bus_width));
	/* 0x0A */
	dev_print("       %#.2x, CMQ_MST_DIAG                = %#.2x\n",
		  CMQ_MST_DIAG,
		  callout_p->rd_frm_reg(base_addr, CMQ_MST_DIAG,
					pmc4359->bus_width));
	/* 0x10 */
	dev_print("       %#.2x, CMQ_CDRC_CFG                = %#.2x\n",
		  CMQ_CDRC_CFG, 
		  callout_p->rd_frm_reg(base_addr, CMQ_CDRC_CFG,
					pmc4359->bus_width));
	/* 0x11 */
	dev_print("       %#.2x, CMQ_CDRC_INT_EN             = %#.2x\n",
		  CMQ_CDRC_INT_EN, 
		  callout_p->rd_frm_reg(base_addr, CMQ_CDRC_INT_EN,
					pmc4359->bus_width));
	/* 0x12 */
	dev_print("       %#.2x, CMQ_CDRC_INT_STAT           = %#.2x\n",
		  CMQ_CDRC_INT_STAT, 
		  callout_p->rd_frm_reg(base_addr, CMQ_CDRC_INT_STAT,
					pmc4359->bus_width));
    }
    /* 0x13 */
    dev_print("       %#.2x, CMQ_CDRC_ALT_LOS            = %#.2x\n",
	      CMQ_CDRC_ALT_LOS, 
	      callout_p->rd_frm_reg(base_addr, CMQ_CDRC_ALT_LOS,
				    pmc4359->bus_width));
    /* 0x14 */
    dev_print("       %#.2x, CMQ_RJAT_INT_STAT           = %#.2x\n",
	      CMQ_RJAT_INT_STAT, 
	      callout_p->rd_frm_reg(base_addr, CMQ_RJAT_INT_STAT,
				    pmc4359->bus_width));

    if (cmd != DEV_SHOW_STATUS) {
	/* 0x15 */
	dev_print("       %#.2x, CMQ_RJAT_DIV_N1_CTL         = %#.2x\n",
		  CMQ_RJAT_DIV_N1_CTL,
		  callout_p->rd_frm_reg(base_addr, CMQ_RJAT_DIV_N1_CTL,
					pmc4359->bus_width));
	/* 0x16 */
	dev_print("       %#.2x, CMQ_RJAT_DIV_N2_CTL         = %#.2x\n",
		  CMQ_RJAT_DIV_N2_CTL,
		  callout_p->rd_frm_reg(base_addr, CMQ_RJAT_DIV_N2_CTL,
					pmc4359->bus_width));
	/* 0x17 */
	dev_print("       %#.2x, CMQ_RJAT_CFG                = %#.2x\n",
		  CMQ_RJAT_CFG, 
		  callout_p->rd_frm_reg(base_addr, CMQ_RJAT_CFG, 
					pmc4359->bus_width));
    }
    /* 0x18 */
    dev_print("       %#.2x, CMQ_TJAT_INT_STAT           = %#.2x\n",
	      CMQ_TJAT_INT_STAT, 
	      callout_p->rd_frm_reg(base_addr, CMQ_TJAT_INT_STAT,
				    pmc4359->bus_width));

    if (cmd != DEV_SHOW_STATUS) {
	/* 0x19 */
	dev_print("       %#.2x, CMQ_TJAT_DIV_N1_CTL         = %#.2x\n",
		  CMQ_TJAT_DIV_N1_CTL,
		  callout_p->rd_frm_reg(base_addr, CMQ_TJAT_DIV_N1_CTL,
					pmc4359->bus_width));
	/* 0x1a */
	dev_print("       %#.2x, CMQ_TJAT_DIV_N2_CTL         = %#.2x\n",
		  CMQ_TJAT_DIV_N2_CTL,
		  callout_p->rd_frm_reg(base_addr, CMQ_TJAT_DIV_N2_CTL,
					pmc4359->bus_width));
	/* 0x1b */
	dev_print("       %#.2x, CMQ_TJAT_CFG                = %#.2x\n",
		  CMQ_TJAT_CFG, 
		  callout_p->rd_frm_reg(base_addr, CMQ_TJAT_CFG,
					pmc4359->bus_width));
	/* 0x1c */
	dev_print("       %#.2x, CMQ_RX_ELST_CFG             = %#.2x\n",
		  CMQ_RX_ELST_CFG, 
		  callout_p->rd_frm_reg(base_addr, CMQ_RX_ELST_CFG,
					pmc4359->bus_width));
    }
    /* 0x1D */
    dev_print("       %#.2x, CMQ_RX_ELST_INT_EN_STAT     = %#.2x\n",
	      CMQ_RX_ELST_INT_EN_STAT,
	      callout_p->rd_frm_reg(base_addr, CMQ_RX_ELST_INT_EN_STAT,
				    pmc4359->bus_width));

    if (cmd != DEV_SHOW_STATUS) {
	/* 0x20 */
	dev_print("       %#.2x, CMQ_TX_ELST_CFG             = %#.2x\n",
		  CMQ_TX_ELST_CFG, 
		  callout_p->rd_frm_reg(base_addr, CMQ_TX_ELST_CFG,
					pmc4359->bus_width));
    }
    /* 0x21 */
    dev_print("       %#.2x, CMQ_TX_ELST_INT_EN_STAT     = %#.2x\n",
	      CMQ_TX_ELST_INT_EN_STAT,
	      callout_p->rd_frm_reg(base_addr, CMQ_TX_ELST_INT_EN_STAT,
				    pmc4359->bus_width));

    if (cmd != DEV_SHOW_STATUS) {
	/* 0x28 */
	dev_print("       %#.2x, CMQ_RX_DLNK_CTL             = %#.2x\n",
		  CMQ_RX_DLNK_CTL, 
		  callout_p->rd_frm_reg(base_addr, CMQ_RX_DLNK_CTL,
					pmc4359->bus_width));
	/* 0x30 */
	dev_print("       %#.2x, CMQ_BRIF_CFG                = %#.2x\n",
		  CMQ_BRIF_CFG, 
		  callout_p->rd_frm_reg(base_addr, CMQ_BRIF_CFG,
					pmc4359->bus_width));
	/* 0x31 */
	dev_print("       %#.2x, CMQ_BRIF_FRM_PULSE_CFG      = %#.2x\n",
		  CMQ_BRIF_FRM_PULSE_CFG,
		  callout_p->rd_frm_reg(base_addr, CMQ_BRIF_FRM_PULSE_CFG,
					pmc4359->bus_width));
	/* 0x32 */
	dev_print("       %#.2x, CMQ_BRIF_PAR_FBIT_CFG       = %#.2x\n",
		  CMQ_BRIF_PAR_FBIT_CFG,
		  callout_p->rd_frm_reg(base_addr, CMQ_BRIF_PAR_FBIT_CFG,
					pmc4359->bus_width));
	/* 0x33 */
	dev_print("       %#.2x, CMQ_BRIF_TSLOT_OFFSET       = %#.2x\n",
		  CMQ_BRIF_TSLOT_OFFSET,
		  callout_p->rd_frm_reg(base_addr, CMQ_BRIF_TSLOT_OFFSET,
					pmc4359->bus_width));
	/* 0x34 */
	dev_print("       %#.2x, CMQ_BRIF_BIT_OFFSET         = %#.2x\n",
		  CMQ_BRIF_BIT_OFFSET,
		  callout_p->rd_frm_reg(base_addr, CMQ_BRIF_BIT_OFFSET,
					pmc4359->bus_width));
	/* 0x38 */
	dev_print("       %#.2x, CMQ_TX_DLNK_CTL,            = %#.2x\n",
		  CMQ_TX_DLNK_CTL, 
		  callout_p->rd_frm_reg(base_addr, CMQ_TX_DLNK_CTL,
					pmc4359->bus_width));
	/* 0x40 */
	dev_print("       %#.2x, CMQ_BTIF_CFG                = %#.2x\n",
		  CMQ_BTIF_CFG, 
		  callout_p->rd_frm_reg(base_addr, CMQ_BTIF_CFG,
					pmc4359->bus_width));
	/* 0x41 */
	dev_print("       %#.2x, CMQ_BTIF_FRM_PULSE_CFG      = %#.2x\n",
		  CMQ_BTIF_FRM_PULSE_CFG,
		  callout_p->rd_frm_reg(base_addr, CMQ_BTIF_FRM_PULSE_CFG,
					pmc4359->bus_width));
    }
    /* 0x42 */
    dev_print("       %#.2x, CMQ_BTIF_PAR_STAT_CFG       = %#.2x\n",
	      CMQ_BTIF_PAR_STAT_CFG,
	      callout_p->rd_frm_reg(base_addr, CMQ_BTIF_PAR_STAT_CFG,
				    pmc4359->bus_width));

    if (cmd != DEV_SHOW_STATUS) {
	/* 0x43 */
	dev_print("       %#.2x, CMQ_BTIF_TSLOT_OFFSET       = %#.2x\n",
		  CMQ_BTIF_TSLOT_OFFSET,
		  callout_p->rd_frm_reg(base_addr, CMQ_BTIF_TSLOT_OFFSET,
					pmc4359->bus_width));
	/* 0x44 */
	dev_print("       %#.2x, CMQ_BTIF_BIT_OFFSET         = %#.2x\n",
		  CMQ_BTIF_BIT_OFFSET,
		  callout_p->rd_frm_reg(base_addr, CMQ_BTIF_BIT_OFFSET,
					pmc4359->bus_width));
	/* 0x48 */
	dev_print("       %#.2x, CMQ_T1_FRMR_CFG             = %#.2x\n",
		  CMQ_T1_FRMR_CFG, 
		  callout_p->rd_frm_reg(base_addr, CMQ_T1_FRMR_CFG,
					pmc4359->bus_width));
    }
    /* 0x4A */
    dev_print("       %#.2x, CMQ_T1_FRMR_STAT_INT_IND    = %#.2x\n",
	      CMQ_T1_FRMR_STAT_INT_IND,
	      callout_p->rd_frm_reg(base_addr, CMQ_T1_FRMR_STAT_INT_IND,
				    pmc4359->bus_width));
    /* 0x4D */
    dev_print("       %#.2x, CMQ_IBCD_INT_EN_STAT        = %#.2x\n",
	      CMQ_IBCD_INT_EN_STAT,
	      callout_p->rd_frm_reg(base_addr, CMQ_IBCD_INT_EN_STAT,
				    pmc4359->bus_width));

    if (cmd != DEV_SHOW_STATUS) {
	/* 0x50 */
	dev_print("       %#.2x, CMQ_SIGX_CFG_CHG_SIG_STATE  = %#.2x\n",
		  CMQ_SIGX_CFG_CHG_SIG_STATE, 
		  callout_p->rd_frm_reg(base_addr, CMQ_SIGX_CFG_CHG_SIG_STATE,
					pmc4359->bus_width));
	/* 0x54 */
	dev_print("       %#.2x, CMQ_T1_XBAS_CFG             = %#.2x\n",
		  CMQ_T1_XBAS_CFG, 
		  callout_p->rd_frm_reg(base_addr, CMQ_T1_XBAS_CFG, 
					pmc4359->bus_width));
    }
    /* 0x58 */
    dev_print("       %#.2x, CMQ_PMON_INT_EN_STAT        = %#.2x\n",
	      CMQ_PMON_INT_EN_STAT,
	      callout_p->rd_frm_reg(base_addr, CMQ_PMON_INT_EN_STAT,
				    pmc4359->bus_width));
    /* 0x59 */
    dev_print("       %#.2x, CMQ_PMON_FRM_BIT_ERR_CNT    = %#.2x\n",
	      CMQ_PMON_FRM_BIT_ERR_CNT,
	      callout_p->rd_frm_reg(base_addr, CMQ_PMON_FRM_BIT_ERR_CNT,
				    pmc4359->bus_width));

    if (cmd != DEV_SHOW_STATUS) {
	/* 0x60 */
	dev_print("       %#.2x, CMQ_T1_ALMI_CFG             = %#.2x\n",
		  CMQ_T1_ALMI_CFG, 
		  callout_p->rd_frm_reg(base_addr, CMQ_T1_ALMI_CFG,
					pmc4359->bus_width));
    }
    /* 0x62 */
    dev_print("       %#.2x, CMQ_T1_ALMI_INT_STAT        = %#.2x\n",
	      CMQ_T1_ALMI_INT_STAT,
	      callout_p->rd_frm_reg(base_addr, CMQ_T1_ALMI_INT_STAT,
				    pmc4359->bus_width));
    /* 0x63 */
    dev_print("       %#.2x, CMQ_T1_ALMI_ALRM_DET_STAT   = %#.2x\n",
	      CMQ_T1_ALMI_ALRM_DET_STAT,
	      callout_p->rd_frm_reg(base_addr, CMQ_T1_ALMI_ALRM_DET_STAT,
				    pmc4359->bus_width));
    /* 0x65 */
    dev_print("       %#.2x, CMQ_T1_PDVD_INT_EN_STAT     = %#.2x\n",
	      CMQ_T1_PDVD_INT_EN_STAT,
	      callout_p->rd_frm_reg(base_addr, CMQ_T1_PDVD_INT_EN_STAT,
				    pmc4359->bus_width));
    /* 0x69 */
    dev_print("       %#.2x, CMQ_T1_XPDE_INT_EN_STAT     = %#.2x\n",
	      CMQ_T1_XPDE_INT_EN_STAT,
	      callout_p->rd_frm_reg(base_addr, CMQ_T1_XPDE_INT_EN_STAT,
				    pmc4359->bus_width));
    /* 0x6B */
    dev_print("       %#.2x, CMQ_T1_RBOC_CODE_STAT       = %#.2x\n",
	      CMQ_T1_RBOC_CODE_STAT,
	      callout_p->rd_frm_reg(base_addr, CMQ_T1_RBOC_CODE_STAT,
				    pmc4359->bus_width));

    if (cmd != DEV_SHOW_STATUS) {
	/* 0x6c */
	dev_print("       %#.2x, CMQ_TPSC_CFG                = %#.2x\n",
		  CMQ_TPSC_CFG, callout_p->rd_frm_reg(base_addr, CMQ_TPSC_CFG,
						      pmc4359->bus_width));
    }
    /* 0x6D */
    dev_print("       %#.2x, CMQ_TPSC_MICRO_ACCESS_STAT  = %#.2x\n",
	      CMQ_TPSC_MICRO_ACCESS_STAT,
	      callout_p->rd_frm_reg(base_addr, CMQ_TPSC_MICRO_ACCESS_STAT,
				    pmc4359->bus_width));

    if (cmd != DEV_SHOW_STATUS) {
	/* 0x70 */
	dev_print("       %#.2x, CMQ_RPSC_CFG                = %#.2x\n",
		  CMQ_RPSC_CFG, callout_p->rd_frm_reg(base_addr, CMQ_RPSC_CFG,
						      pmc4359->bus_width));
    }
    /* 0x71 */
    dev_print("       %#.2x, CMQ_RPSC_MICRO_ACCESS_STAT  = %#.2x\n",
	      CMQ_RPSC_MICRO_ACCESS_STAT,
	      callout_p->rd_frm_reg(base_addr, CMQ_RPSC_MICRO_ACCESS_STAT,
				    pmc4359->bus_width));
    /* 0x7A */
    dev_print("       %#.2x, CMQ_T1_APRM_INT_STAT        = %#.2x\n",
	      CMQ_T1_APRM_INT_STAT,
	      callout_p->rd_frm_reg(base_addr, CMQ_T1_APRM_INT_STAT,
				    pmc4359->bus_width));

    if (cmd != DEV_SHOW_STATUS) {
	/* 0x80 */
	dev_print("       %#.2x, CMQ_E1_TRAN_CFG             = %#.2x\n",
		  CMQ_E1_TRAN_CFG, 
		  callout_p->rd_frm_reg(base_addr, CMQ_E1_TRAN_CFG,
					pmc4359->bus_width));
    }
    /* 0x85 */
    dev_print("       %#.2x, CMQ_E1_TRAN_INT_STAT        = %#.2x\n",
	      CMQ_E1_TRAN_INT_STAT,
	      callout_p->rd_frm_reg(base_addr, CMQ_E1_TRAN_INT_STAT,
				    pmc4359->bus_width));

    if (cmd != DEV_SHOW_STATUS) {
	/* 0x90 */
	dev_print("       %#.2x, CMQ_E1_FRMR_FRM_ALIGN_OPT   = %#.2x\n",
		  CMQ_E1_FRMR_FRM_ALIGN_OPT,
		  callout_p->rd_frm_reg(base_addr, CMQ_E1_FRMR_FRM_ALIGN_OPT,
					pmc4359->bus_width));
	/* 0x91 */
	dev_print("       %#.2x, CMQ_E1_FRMR_MAINT_MODE_OPT  = %#.2x\n",
		  CMQ_E1_FRMR_MAINT_MODE_OPT,
		  callout_p->rd_frm_reg(base_addr, CMQ_E1_FRMR_MAINT_MODE_OPT,
					pmc4359->bus_width));
    }
    /* 0x94 */
    dev_print("       %#.2x, CMQ_E1_FRMR_E1_FRM_STAT_INT_IND = %#.2x\n",
	      CMQ_E1_FRMR_E1_FRM_STAT_INT_IND,
	      callout_p->rd_frm_reg(base_addr, 
				    CMQ_E1_FRMR_E1_FRM_STAT_INT_IND,
				    pmc4359->bus_width));
    /* 0x95 */
    dev_print("       %#.2x, CMQ_E1_FRMR_MAINT_ALRM_STAT_INT_IND = %#.2x\n",
	      CMQ_E1_FRMR_MAINT_ALRM_STAT_INT_IND,
	      callout_p->rd_frm_reg(base_addr, 
				    CMQ_E1_FRMR_MAINT_ALRM_STAT_INT_IND,
				    pmc4359->bus_width));
    /* 0x96 */
    dev_print("       %#.2x, CMQ_E1_FRMR_STATT           = %#.2x\n",
	      CMQ_E1_FRMR_STAT, 
	      callout_p->rd_frm_reg(base_addr, CMQ_E1_FRMR_STAT,
				    pmc4359->bus_width));
    /* 0x97 */
    dev_print("       %#.2x, CMQ_E1_FRMR_MAINT_ALRM_STAT = %#.2x\n",
	      CMQ_E1_FRMR_MAINT_ALRM_STAT,
	      callout_p->rd_frm_reg(base_addr, CMQ_E1_FRMR_MAINT_ALRM_STAT,
				    pmc4359->bus_width));
    /* 0x99 */
    dev_print("       %#.2x, CMQ_E1_FRMR_CRC_ERR_CNT_LSB = %#.2x\n",
	      CMQ_E1_FRMR_CRC_ERR_CNT_LSB,
	      callout_p->rd_frm_reg(base_addr, CMQ_E1_FRMR_CRC_ERR_CNT_LSB,
				    pmc4359->bus_width));
    /* 0x9A */
    dev_print("       %#.2x, CMQ_E1_FRMR_CRC_ERR_CNT_MSB = %#.2x\n",
	      CMQ_E1_FRMR_CRC_ERR_CNT_MSB,
	      callout_p->rd_frm_reg(base_addr, CMQ_E1_FRMR_CRC_ERR_CNT_MSB,
				    pmc4359->bus_width));
    /* 0xAC */
    dev_print("       %#.2x, CMQ_TDPR_INT_STAT_UDR_CLR   = %#.2x\n",
	      CMQ_TDPR_INT_STAT_UDR_CLR,
	      callout_p->rd_frm_reg(base_addr, CMQ_TDPR_INT_STAT_UDR_CLR,
				    pmc4359->bus_width));

    if (cmd != DEV_SHOW_STATUS) {
	/* 0xb0 */
	dev_print("       %#.2x, CMQ_RX_ELST_CCS_CFG         = %#.2x\n",
		  CMQ_RX_ELST_CCS_CFG,
		  callout_p->rd_frm_reg(base_addr, CMQ_RX_ELST_CCS_CFG,
					pmc4359->bus_width));
    }
    /* 0xB1 */
    dev_print("       %#.2x, CMQ_RX_ELST_CCS_INT_EN_STAT = %#.2x\n",
	      CMQ_RX_ELST_CCS_INT_EN_STAT,
	      callout_p->rd_frm_reg(base_addr, CMQ_RX_ELST_CCS_INT_EN_STAT,
				    pmc4359->bus_width));

    if (cmd != DEV_SHOW_STATUS) {
	/* 0xb4 */
	dev_print("       %#.2x, CMQ_TX_ELST_CCS_CFG         = %#.2x\n",
		  CMQ_TX_ELST_CCS_CFG,
		  callout_p->rd_frm_reg(base_addr, CMQ_TX_ELST_CCS_CFG,
					pmc4359->bus_width));
    }
    /* 0xB5 */
    dev_print("       %#.2x, CMQ_TX_ELST_CCS_INT_EN_STAT = %#.2x\n",
	      CMQ_TX_ELST_CCS_INT_EN_STAT,
	      callout_p->rd_frm_reg(base_addr, CMQ_TX_ELST_CCS_INT_EN_STAT,
				    pmc4359->bus_width));
    if (cmd != DEV_SHOW_STATUS) {
        /* 0xbe */
        dev_print("       %#.2x, CMQ_TERM_CNTRL              = %#.2x\n",
                  CMQ_TERM_CNTRL,
                  callout_p->rd_frm_reg(base_addr, CMQ_TERM_CNTRL,
                                        pmc4359->bus_width));
    }
    /* 0xC2 */
    dev_print("       %#.2x, CMQ_RDLC_INT_STAT           = %#.2x\n",
	      CMQ_RDLC_INT_STAT,
	      callout_p->rd_frm_reg(base_addr, CMQ_RDLC_INT_STAT,
				    pmc4359->bus_width));

    if (cmd != DEV_SHOW_STATUS) {
	/* 0xd6 */
	dev_print("       %#.2x, CMQ_CSU_CFG                 = %#.2x\n",
		  CMQ_CSU_CFG, callout_p->rd_frm_reg(base_addr, CMQ_CSU_CFG,
						     pmc4359->bus_width));
	/* 0xdc */
	dev_print("       %#.2x, CMQ_RLPS_EQ_LOOP_VOLT_REF   = %#.2x\n",
		  CMQ_RLPS_EQ_LOOP_VOLT_REF,
		  callout_p->rd_frm_reg(base_addr, CMQ_RLPS_EQ_LOOP_VOLT_REF,
					pmc4359->bus_width));
	/* 0xdd */
	dev_print("       %#.2x, CMQ_RLPS_EQ_LOOP_VOLT_REF2  = %#.2x\n",
		  CMQ_RLPS_EQ_LOOP_VOLT_REF2,
		  callout_p->rd_frm_reg(base_addr, CMQ_RLPS_EQ_LOOP_VOLT_REF2,
					pmc4359->bus_width));
    }
    /* 0xE1 */
    dev_print("       %#.2x, CMQ_PRBS_CHK_INT_EN_STAT    = %#.2x\n",
	      CMQ_PRBS_CHK_INT_EN_STAT,
	      callout_p->rd_frm_reg(base_addr, CMQ_PRBS_CHK_INT_EN_STAT,
				    pmc4359->bus_width));

    if (cmd != DEV_SHOW_STATUS) {
	/* 0xf0 */
	dev_print("       %#.2x, CMQ_XLPG_LINE_DRV_CFG       = %#.2x\n",
		  CMQ_XLPG_LINE_DRV_CFG,
		  callout_p->rd_frm_reg(base_addr, CMQ_XLPG_LINE_DRV_CFG,
					pmc4359->bus_width));
    }
    /* 0xF1 */
    dev_print("       %#.2x, CMQ_XLPG_CTL_STAT           = %#.2x\n",
	      CMQ_XLPG_CTL_STAT, 
	      callout_p->rd_frm_reg(base_addr, CMQ_XLPG_CTL_STAT,
				    pmc4359->bus_width));

    /* 0xf8 */
    dev_print("       %#.2x, CMQ_RLPS_CFG_STAT           = %#.2x\n",
	      CMQ_RLPS_CFG_STAT, 
	      callout_p->rd_frm_reg(base_addr, CMQ_RLPS_CFG_STAT,
				    pmc4359->bus_width));

    if (cmd != DEV_SHOW_STATUS) {
	/* 0xf9 */
	dev_print("       %#.2x, CMQ_RLPS_ALOS_DET_CLRNCE_THRESH = %#.2x\n",
		  CMQ_RLPS_ALOS_DET_CLRNCE_THRESH, 
		  callout_p->rd_frm_reg(base_addr, 
					CMQ_RLPS_ALOS_DET_CLRNCE_THRESH,
					pmc4359->bus_width));
	/* 0xfa */
	dev_print("       %#.2x, CMQ_RLPS_ALOS_DET_PERIOD    = %#.2x\n",
		  CMQ_RLPS_ALOS_DET_PERIOD,
		  callout_p->rd_frm_reg(base_addr, CMQ_RLPS_ALOS_DET_PERIOD,
					pmc4359->bus_width));
	/* 0xfb */
	dev_print("       %#.2x, CMQ_RLPS_ALOS_CLRNCE_PERIOD = %#.2x\n",
		  CMQ_RLPS_ALOS_CLRNCE_PERIOD,
		  callout_p->rd_frm_reg(base_addr, 
					CMQ_RLPS_ALOS_CLRNCE_PERIOD,
					pmc4359->bus_width));
	/* 0xfc */
	dev_print("       %#.2x, CMQ_RLPS_EQ_IND_ADDR        = %#.2x\n",
		  CMQ_RLPS_EQ_IND_ADDR, 
		  callout_p->rd_frm_reg(base_addr, CMQ_RLPS_EQ_IND_ADDR,
					pmc4359->bus_width));
	/* 0xfd */
	dev_print("       %#.2x, CMQ_RLPS_EQ_RDWR_SEL        = %#.2x\n",
		  CMQ_RLPS_EQ_RDWR_SEL,
		  callout_p->rd_frm_reg(base_addr, CMQ_RLPS_EQ_RDWR_SEL,
					pmc4359->bus_width));
	/* 0xfe */
	dev_print("       %#.2x, CMQ_RLPS_EQ_LOOP_STAT_CTL   = %#.2x\n",
		  CMQ_RLPS_EQ_LOOP_STAT_CTL,
		  callout_p->rd_frm_reg(base_addr, CMQ_RLPS_EQ_LOOP_STAT_CTL, 
					pmc4359->bus_width));
	/* 0xff */
	dev_print("       %#.2x, CMQ_RLPS_EQ_CFG             = %#.2x\n",
		  CMQ_RLPS_EQ_CFG, 
		  callout_p->rd_frm_reg(base_addr, CMQ_RLPS_EQ_CFG,
					pmc4359->bus_width));
    }
    return(PASSED);
}


/**********************************************************************
 *
 * Function: pm4359_linetx_analog_conf
 *
 * This function configures the analog transmit line interface:
 *            - configures the waveform scaling and pulse shape
 *            - enables/disables transmitter high impedence
 *            - fuse programming
 *
 * Input : dev - Pointer to the framer device object
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int 
pm4359_linetx_analog_conf (dev_object_t *dev)
{
    uchar wv_form_type;
    dev_4359_object_t *pmc4359 = (dev_4359_object_t *)dev;

    wv_form_type = pmc4359->cfg_info_p->xlpg_line_drv_val;
    /* validate analog waveform type */
    if (wv_form_type > CMQ_TX_LBO_RETAIN_CURRENT) {
#ifdef DDEBUG
	cterr('f',0,"Value is out of table range, %d", wv_form_type);
#endif
        return (FAILED);
    }

    /* configure the XLPG table */
    pm4359_program_xlpg(dev, pm4359_xlpg_wvfmtbls[wv_form_type]);

    /* program the fuses */
    pm4359_program_fuses(dev);

    return (PASSED);
}


/**********************************************************************
 *
 * Function: pm4359_program_xlpg()
 *
 * This function: Configures the XLPG transmit waveform.
 *
 * Input : dev - Pointer to the framer device object
 *         pm4359_xlpg_wvfmtbls  - Waveform table 2-D array
 *
 * Output: NONE
 *
 **********************************************************************
 */
static void 
pm4359_program_xlpg (dev_object_t *dev, uchar (*waveform)[CMQ_XLPG_MAX_UNITS])
{
    volatile uchar data;
    ulong frm_bar, iaddr;
    uchar frm_num, i, j;
    dev_4359_object_t *pmc4359 = (dev_4359_object_t *)dev;
    dev_4359_callout_fvt_t *callout_p = pmc4359->callout_fvt;

    frm_num = pmc4359->cfg_info_p->quad_num;
    frm_bar = (ulong)pmc4359->base.dev_addr + (frm_num * CMQ_FRM_OFFSET * 
					       pmc4359->bus_width);

    /* program the transmit pulse shape */
    for(i = 0; i < CMQ_XLPG_MAX_SAMPLES; i++) {
#ifdef XDEBUG
        printf("\ndata %.2d: ", i); 
#endif
        for(j = 0; j < CMQ_XLPG_MAX_UNITS; j++) {
            /* calculate indirect address */
            iaddr = j | (i << CMQ_XLPG_CTL_WAVEFORM_IND_ADDR_SAMPLE_SHIFT);

            /* write indirect address 0xF2 */
	    callout_p->wr_frm_reg(frm_bar, CMQ_XLPG_PULSE_WFORM_STORE_WR_ADDR,
				  iaddr, pmc4359->bus_width);

            /* write data to indirect address 0xf3 */
            data = waveform[i][j];
            data &= CMQ_XLPG_WAVEFORM_STORE_DATA_MASK;
#ifdef XDEBUG
            printf(" %#.2x,", data);
#endif
	    callout_p->wr_frm_reg(frm_bar, CMQ_XLPG_PULSE_WFORM_STORE_WR_DATA,
				  data, pmc4359->bus_width);
        }
    }
}


/**********************************************************************
 *
 * Function: pm4359_program_fuses()
 *
 * This function: Configures the analog transmit line interface Pulse Wave
 *                form and control data for COMET Tetra.
 *                This function performs the initialization sequence
 *                specified in the COMET Tetra Data Sheet, Release 6.
 *
 * Input : dev - Pointer to the framer device object
 *
 * Output: NONE
 *
 **********************************************************************
 */
static void 
pm4359_program_fuses(dev_object_t *dev)
{
    ulong frm_bar;
    uchar frm_num;
    dev_4359_object_t *pmc4359 = (dev_4359_object_t *)dev;
    dev_4359_callout_fvt_t *callout_p = pmc4359->callout_fvt;

    frm_num = pmc4359->cfg_info_p->quad_num;
    frm_bar = (ulong)pmc4359->base.dev_addr + (frm_num * CMQ_FRM_OFFSET * 
					       pmc4359->bus_width);

    /*
     * this function performs the sequence outlined in the COMET Tetra
     * Data Sheet, Release 6
     */
    /* reg offset 0xF6 */
    callout_p->wr_frm_reg(frm_bar, CMQ_XLPG_INIT, CMQ_XLPG_INIT_DATA_SEL_POS |
			  CMQ_XLPG_INIT_DATA_SEL_NEG, pmc4359->bus_width);

    /*  reg offset 0xF4 */
    callout_p->wr_frm_reg(frm_bar, CMQ_XLPG_CFG1, 0, pmc4359->bus_width);

    /*  reg offset 0xF5 */
    callout_p->wr_frm_reg(frm_bar, CMQ_XLPG_CFG2, 0, pmc4359->bus_width);

    /*  reg offset 0xF6 */
    callout_p->wr_frm_reg(frm_bar, CMQ_XLPG_INIT, 0, pmc4359->bus_width);
}


/**********************************************************************
 *
 * Function: pm4359_rlps_opti()
 *
 * This function: This function implements the RLPS initialization sequence
 *                for COMET Tetra devices given in the COMET Tetra Data Sheet.
 *                This sequence ensures optimal receiver sensitivity.
 *
 * Input : dev - Pointer to the framer device object
 *
 * Output: NONE
 *
 **********************************************************************
 */
static void 
pm4359_rlps_opti(dev_object_t *dev)
{
    ulong framer, chip_bar;
    dev_4359_object_t *pmc4359 = (dev_4359_object_t *)dev;
    dev_4359_callout_fvt_t *callout_p = pmc4359->callout_fvt;

    chip_bar = (ulong)pmc4359->base.dev_addr;

    /* write 0x00 to register 0x4D7 */
    callout_p->wr_frm_reg(chip_bar, 0x4D7, 0, pmc4359->bus_width);

    for (framer = 0; framer < CMQ_MAX_FRAMERS; framer++) {
	/* write 0 to register 0x4F1, 0x5F1, 0x6F1, 0x7F1 */
	callout_p->wr_frm_reg(chip_bar, 0x4F1 + (framer * CMQ_FRM_OFFSET * 
						 pmc4359->bus_width), 0,
			      pmc4359->bus_width);

	/* write 0 to register 0x4F9, 0x5F9, 0x6F9, 0x7F9 */
	callout_p->wr_frm_reg(chip_bar, 0x4F9 + (framer * CMQ_FRM_OFFSET * 
						 pmc4359->bus_width), 0,
			      pmc4359->bus_width);
    }

    for (framer = 0; framer < CMQ_MAX_FRAMERS; framer++) {
	/* write 0x04 to register 0x4F9, 0x5F9, 0x6F9, 0x7F9 */
	callout_p->wr_frm_reg(chip_bar, 0x4F9 + (framer * CMQ_FRM_OFFSET * 
						 pmc4359->bus_width), 
			      0x04, pmc4359->bus_width);

	/* write 0x09 to register 0x4FB, 0x5FB, 0x6FB, 0x7FB */
	callout_p->wr_frm_reg(chip_bar, 0x4FB + (framer * CMQ_FRM_OFFSET * 
						 pmc4359->bus_width), 
			      0x09, pmc4359->bus_width);

	/* write 0x20 to register 0x00B */
	callout_p->wr_frm_reg(chip_bar, 0x00B, 0x20, pmc4359->bus_width);

	/* delay for a millisecond */
	msleep(1);

	/* write 0x00 to register 0x4F9, 0x5F9, 0x6F9, 0x7F9 */
	callout_p->wr_frm_reg(chip_bar, 0x4F9 + (framer * CMQ_FRM_OFFSET * 
						 pmc4359->bus_width), 0,
			      pmc4359->bus_width);

	/* write 0x00 to register 0x00B */
	callout_p->wr_frm_reg(chip_bar, 0x00B, 0, pmc4359->bus_width);
    }
}

/**********************************************************************
 *
 * Function: pm4359_rlps_volt()
 *
 * This function: Configures voltage reference
 *
 * Input : dev - Pointer to the framer device object
 *
 * Output: NONE
 *
 **********************************************************************
 */
static void 
pm4359_rlps_volt(dev_object_t *dev)
{
    uchar val;
    ulong frm_bar;
    dev_4359_object_t *pmc4359 = (dev_4359_object_t *)dev;
    dev_4359_callout_fvt_t *callout_p = pmc4359->callout_fvt;

    frm_bar = (ulong)pmc4359->base.dev_addr +
	(pmc4359->cfg_info_p->quad_num * CMQ_FRM_OFFSET * pmc4359->bus_width);
    
    /*
     * configure RLPS voltage reference1: reg 0xdc
     */
    val = callout_p->rd_frm_reg(frm_bar, CMQ_RLPS_EQ_LOOP_VOLT_REF,
			       pmc4359->bus_width) & ~CMQ_RLPS_EQ_VOLT_REF_MASK;
    val |= CMQ_RLPS_VREF_VALU;
    callout_p->wr_frm_reg(frm_bar, CMQ_RLPS_EQ_LOOP_VOLT_REF, val,
			  pmc4359->bus_width);

    /*
     * configure RLPS voltage reference2: reg 0xdd
     */
    if (pmc4359->cfg_info_p->op_mode == CMQ_MODE_E1) {
        val = CMQ_RLPS_VREF_E1;
    } else {
        val = CMQ_RLPS_VREF_T1;
    }
    callout_p->wr_frm_reg(frm_bar, CMQ_RLPS_EQ_LOOP_VOLT_REF2, val,
			  pmc4359->bus_width);
}

/* end module */


/******** History ******** 
$Log: dev_4359.c,v $
Revision 1.4  2012/06/11 21:54:41  ywen
Fix a bug for 2 port framer.

Revision 1.3  2012/04/02 17:21:04  ywen
Fix a bug in Framer y-cable setting.

Revision 1.2  2012/03/28 00:38:08  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:02  ptong
Initial archive of ng_diag module


$Endlog$
*/
