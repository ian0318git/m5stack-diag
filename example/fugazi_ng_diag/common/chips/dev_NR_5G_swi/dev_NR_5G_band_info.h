/* $Id: dev_NR_5G_band_info.h,v 1.4 2021/06/30 20:04:55 tshanmug Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_NR_5G_swi/dev_NR_5G_band_info.h,v $
 *------------------------------------------------------------------
 * Filename:    dev_NR_band_info.h
 *
 * Description: Header File of SWI driver
 *
 * Copyright (c) 2020 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DEV_NR_BAND_INFO_H__
#define __DEV_NR_BAND_INFO_H__

#define MAIN_AUX_SUPPORTED    (MAIN_RSSI|AUX_RSSI)
#define MIMO1_MIMO2_SUPPORTED (MIMO1_RSSI|MIMO2_RSSI)
#define ALL_ANTENNA_SUPPORTED (MAIN_RSSI|AUX_RSSI|MIMO1_RSSI|MIMO2_RSSI)

nr_sub6_band_struct nr_sub6_band_legacy_tbl[]= \
{
    { BAND_N1,  20000, 390000, 428000, "2140",     "1950",     ALL_ANTENNA_SUPPORTED},
    { BAND_N2,  20000, 376000, 392000, "1960",     "1880",     ALL_ANTENNA_SUPPORTED},
    { BAND_N3,  20000, 349500, 368500, "1842.5",   "1747.5",   ALL_ANTENNA_SUPPORTED},
    { BAND_N5,  20000, 167300, 176300, "881.5",    "836.5",    MAIN_AUX_SUPPORTED},
    { BAND_N28, 20000, 145100, 156100, "780.5",    "725.5",    MAIN_AUX_SUPPORTED},
    { BAND_N41, 20000, 518598, 518598, "2593",     "2593",     MIMO1_MIMO2_SUPPORTED},
    { BAND_N66, 20000, 349000, 431000, "2155",     "1745",     ALL_ANTENNA_SUPPORTED},
    { BAND_N71, 20000, 136100, 126900, "634.5",    "680.5",    MAIN_AUX_SUPPORTED},
    { BAND_N77, 50000, 650000, 650000, "3750",     "3750",     ALL_ANTENNA_SUPPORTED},
    { BAND_N78, 50000, 636666, 636666, "3549.99",  "3549.99",  ALL_ANTENNA_SUPPORTED},
    { BAND_N79, 50000, 713333, 713333, "4699.995", "4699.995", ALL_ANTENNA_SUPPORTED},
};
nr_sub6_band_struct nr_sub6_band_tbl[]= \
{
    { BAND_N1,  5, 390000, 428000, "2140",     "1950",     ALL_ANTENNA_SUPPORTED},
    { BAND_N2,  5, 376000, 392000, "1960",     "1880",     ALL_ANTENNA_SUPPORTED},
    { BAND_N3,  5, 349500, 368500, "1842.5",   "1747.5",   ALL_ANTENNA_SUPPORTED},
    { BAND_N5,  5, 167300, 176300, "881.5",    "836.5",    MAIN_AUX_SUPPORTED},
    { BAND_N28, 5, 145100, 156100, "780.5",    "725.5",    MAIN_AUX_SUPPORTED},
    { BAND_N41, 5, 518598, 518598, "2593",     "2593",     MIMO1_MIMO2_SUPPORTED},
    { BAND_N66, 5, 349000, 431000, "2155",     "1745",     ALL_ANTENNA_SUPPORTED},
    { BAND_N71, 5, 136100, 126900, "634.5",    "680.5",    MAIN_AUX_SUPPORTED},
    { BAND_N77, 9, 650000, 650000, "3750",     "3750",     ALL_ANTENNA_SUPPORTED},
    { BAND_N78, 9, 636666, 636666, "3549.99",  "3549.99",  ALL_ANTENNA_SUPPORTED},
    { BAND_N79, 9, 713333, 713333, "4699.995", "4699.995", ALL_ANTENNA_SUPPORTED},
};

int band_tbl_size = sizeof (nr_sub6_band_tbl)/sizeof(nr_sub6_band_struct);
int band_legacy_tbl_size = sizeof (nr_sub6_band_legacy_tbl)/sizeof(nr_sub6_band_struct);
#define BAND_TBL_SIZE  band_tbl_size


nr_mmwave_band_struct nr_mmwave_band_tbl[]= \
{  /* band_num ,  bw_idx, bw    , tx_ch,   rx_ch  , if_fr_mhz ,  */
    { BAND_N257,  13    , 13, 2079167, 2079167, "8646.48" },
    { BAND_N258,  13    , 13, 2043749 , 2043749 , "8122.2"  },
    { BAND_N260,  13    , 13, 2279165, 2279165, "9180.6"  },
    { BAND_N261,  13    , 13, 2077949, 2077949, "8573.4"  },
};

int band_mmwave_tbl_size = sizeof (nr_mmwave_band_tbl)/sizeof(nr_mmwave_band_struct);
#define BAND_MMWAVE_TBL_SIZE  band_mmwave_tbl_size

/* externs */
extern int sub6_ant_test_band_config(nr_sub6_band_struct *, int, int );

#endif //__DEV_NR_BAND_INFO_H__
/*********************************************************************
 * $Log: dev_NR_5G_band_info.h,v $
 * Revision 1.4  2021/06/30 20:04:55  tshanmug
 * Chrysler Sub6 OTA and SWI common layer changes, Dual SIM test support
 *
 * Revision 1.3  2021/06/02 02:56:20  alpeng
 * merge sears into trunk
 *
 * Revision 1.1.4.5  2021/03/17 09:34:10  alpeng
 * sync trunk to this branch
 *
 * Revision 1.1.4.4  2020/12/22 22:49:28  tshanmug
 * Empire prrq review comment fix
 *
 * $Endlog$
 */
