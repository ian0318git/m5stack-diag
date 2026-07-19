/*
 * $Id: dev_NR_5G_band_info.h,v 1.2 2021/06/02 02:56:19 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_NR_5G_telit/dev_NR_5G_band_info.h,v $
 *------------------------------------------------------------------
 * Filename:    dev_NR_band_info.h
 *
 * Description: Header File of Telit driver
 *
 * Copyright (c) 2021 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DEV_NR_BAND_INFO_H__
#define __DEV_NR_BAND_INFO_H__

/*
Antenna mapping with Band
	PRX	DRX	M1	M2	TX0	TX1
N1	0	1	2	3	0	2
N2	0	1	2	3	0	2
N3	0	1	2	3	0	2
N5	0	1	X	X	0	1
N7	0	1	2	3	0	2
N8	0	1	X	X	0	X
N12	0	1	X	X	0	X
N20	0	1	X	X	0	X
N25	2	3	X	X	2	X
N28	0	1	X	X	0	X
N38	0	1	2	3	0	X
N40	0	1	2	3	0	X
N41	0	1	2	3	0	X
N48	1	0	2	3	1	X
N66	0	1	2	3	0	2
N71	0	1	X	X	0	X
N77	1	0	2	3	0	2
N78	1	0	2	3	0	2
N79	1	0	2	3	0	2


*/

#define PRX   NR_5G_READ_MAIN_RSSI_PWR
#define DRX   NR_5G_READ_DIV_RSSI_PWR
#define M1    NR_5G_READ_MIMO1_RSSI_PWR
#define M2    NR_5G_READ_MIMO2_RSSI_PWR

nr_sub6_band_struct nr_sub6_band_tbl[]= \
{
    /*BAND   |  FREQ    |ANT0 |ANT1 |ANT2 |ANT3 |SUPPORTED ANTENNA    */
    /*=======|==========|=====|=====|=====|=====|=====================*/
    { BAND_N1,  2140000, PRX,  DRX,  M1,   M2,   (ANT0 | ANT1 | ANT2 | ANT3)},
    { BAND_N2,  1960000, PRX,  DRX,  M1,   M2,   (ANT0 | ANT1 | ANT2 | ANT3)},
    { BAND_N3,  1842500, PRX,  DRX,  M1,   M2,   (ANT0 | ANT1 | ANT2 | ANT3)},
    { BAND_N7,  2655000, PRX,  DRX,  M1,   M2,   (ANT0 | ANT1 | ANT2 | ANT3)},
    { BAND_N38, 2595000, PRX,  DRX,  M1,   M2,   (ANT0 | ANT1 | ANT2 | ANT3)},
    { BAND_N40, 2350000, PRX,  DRX,  M1,   M2,   (ANT0 | ANT1 | ANT2 | ANT3)},
    { BAND_N41, 2593000, PRX,  DRX,  M1,   M2,   (ANT0 | ANT1 | ANT2 | ANT3)},
    { BAND_N48, 3625000, DRX,  PRX,  M1,   M2,   (ANT0 | ANT1 | ANT2 | ANT3)},
    { BAND_N66, 2155000, PRX,  DRX,  M1,   M2,   (ANT0 | ANT1 | ANT2 | ANT3)},
    { BAND_N77, 3750000, DRX,  PRX,  M1,   M2,   (ANT0 | ANT1 | ANT2 | ANT3)},
    { BAND_N78, 3550000, DRX,  PRX,  M1,   M2,   (ANT0 | ANT1 | ANT2 | ANT3)},
    { BAND_N79, 4700000, DRX,  PRX,  M1,   M2,   (ANT0 | ANT1 | ANT2 | ANT3)},
//  { BAND_N5,  881500,  PRX,  DRX,  M1,   M2,   (ANT0 | ANT1 )},
//  { BAND_N8,  942500,  PRX,  DRX,  M1,   M2,   (ANT0 | ANT1 )},
//  { BAND_N12, 737500,  PRX,  DRX,  M1,   M2,   (ANT0 | ANT1 )},
//  { BAND_N20, 806000,  PRX,  DRX,  M1,   M2,   (ANT0 | ANT1 )},
//  { BAND_N25, 1962500, PRX,  DRX,  M1,   M2,   (ANT0 | ANT1 )},
//  { BAND_N28, 780500,  PRX,  DRX,  M1,   M2,   (ANT0 | ANT1 )},
//  { BAND_N71, 634500,  PRX,  DRX,  M1,   M2,   (ANT0 | ANT1 )},
};

int band_tbl_size = sizeof (nr_sub6_band_tbl)/sizeof(nr_sub6_band_struct);
#define BAND_TBL_SIZE  band_tbl_size



/* externs */
extern int ant_test_band_config(nr_sub6_band_struct *, int);
#endif //__DEV_NR_BAND_INFO_H__
/*********************************************************************
 * $Log: dev_NR_5G_band_info.h,v $
 * Revision 1.2  2021/06/02 02:56:19  alpeng
 * merge sears into trunk
 *
 * Revision 1.1.2.2  2021/02/27 00:43:07  tshanmug
 * Sears code cleanup
 *
 * Revision 1.1.2.1  2021/02/12 01:12:12  tshanmug
 * Sears RSSI multiband test supported file
 *
 */
