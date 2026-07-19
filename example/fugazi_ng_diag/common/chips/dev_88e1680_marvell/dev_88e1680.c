/* $Id: dev_88e1680.c,v 1.2 2019/12/11 10:10:22 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_88e1680_marvell/dev_88e1680.c,v $
 *------------------------------------------------------------------
 * Filename:	dev_88e1680.c
 *
 * Description: Marvell 88e1680 PHY Device Driver
 * Copyright (c) 2019 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */


#include <sys/time.h>
#include "defs.h"
#include "types.h"
#include "queryflags.h"
#include "common.h"
#include "dev_88e1680.h"
#include "dev_print.h"
#include "free.h"
#include "proto.h"
#include "common_utils.h"
#include "types.h"
#include "error.h"
#include "nvmonvars.h"
#ifdef LINUX_APP
#include <assert.h>
#endif


static uint32 dev_88e1680_attach(dev_object_t *);
static uint32 dev_88e1680_detach(dev_object_t *);
static uint32 dev_88e1680_restart(dev_object_t *);
static void dev_88e1680_destroy(dev_object_t **);
static int dev_88e1680_start_mad_driver(dev_object_t *, MAD_DEV *, int);
static int dev_88e1680_phy_1680_init(dev_object_t *, MAD_DEV *, int);
static int dev_88e1680_phy_init (dev_object_t *, MAD_DEV *, int);
static int dev_88e1680_phy_config(dev_object_t *, MAD_DEV *, int);
static int dev_88e1680_print_phy_counter(dev_object_t *, MAD_DEV *, uint);
static int dev_88e1680_clear_phy_counter (dev_object_t *, MAD_DEV *, uint);
static int dev_88e1680_start_mac_lpbk (dev_object_t *, MAD_DEV *, int, MAD_SPEED_MODE);
static int dev_88e1680_start_ext_lpbk (dev_object_t *, MAD_DEV *, int, MAD_SPEED_MODE);
static int dev_88e1680_phy_force_speed (dev_object_t *, MAD_DEV *, int, MAD_SPEED_MODE);
static int dev_88e1680_phy_register_tests (dev_object_t *, MAD_DEV *, MAD_LPORT, MAD_U16, const reg_info_t *);
static int dev_88e1680_phy_reg_test_single (dev_object_t *, MAD_DEV *, MAD_LPORT);
static int dev_88e1680_phy_reg_test (dev_object_t *, MAD_DEV *, uint);
static int dev_88e1680_phy_detect_phone (dev_object_t *, MAD_DEV *, int);
static int dev_88e1680_dump_phy_reg (dev_object_t *, MAD_DEV *, int);
static int dev_88e1680_reset_phy(dev_object_t *, MAD_DEV *);
//static int dev_88e1680_read_phy_reg_util (dev_object_t *, MAD_DEV *, int);
static int dev_88e1680_read_phy_reg_util (dev_object_t *, MAD_DEV *, uint, uint, uint, uint *);
static int dev_88e1680_write_phy_reg_util (dev_object_t * , MAD_DEV *, uint, uint, uint, uint);
static int dev_88e1680_phy_intr_test (dev_object_t *, MAD_DEV *, int);
static int dev_88e1680_led_on (dev_object_t *, MAD_DEV *, uint);
static int dev_88e1680_led_off (dev_object_t *, MAD_DEV *, uint);
static int dev_88e1680_led_default (dev_object_t *, MAD_DEV *, uint);
static int dev_88e1680_enable_force_interrupt (dev_object_t *, MAD_DEV *, uint, uint);
static int dev_88e1680_gen_int(dev_object_t *, MAD_DEV *, uint);
static int dev_88e1680_clear_int(dev_object_t *, MAD_DEV *, uint);
static int dev_88e1680_set_test_mode (dev_object_t *, MAD_DEV *, uint, uint);

/*===================================================================*
 *                    Polling function                               *
 *===================================================================*/



/*===================================================================*
 *                    Global variables                               *
 *===================================================================*/


static char mrv_88e1680_err_buf[MRV88E1680_ERR_BUF_SIZE];

static const reg_info_t marvell_88e1680_reg_page0[] = {   /* Page 0*/
    {"Copper Control",      0x00, READ_ONLY,  {2}, 0x3140, 0x1940},
    {"Copper Status",       0x01, READ_ONLY,  {2}, 0x0000, 0x7949},
    {"PHY ID1",             0x02, READ_ONLY,  {2}, 0x0000, 0x0141},
    {"PHY ID2",             0x03, READ_ONLY,  {2}, 0x0000, 0x0eb0},
    {"Copper Auto-Neg",     0x04, READ_ONLY,  {2}, 0xA21F, 0x01e1},
    {"Copper Link-P Abil",  0x05, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Copper Auto-Neg Exp", 0x06, READ_ONLY,  {2}, 0x0000, 0x0004},
    {"Copper Next Page",    0x07, READ_ONLY,  {2}, 0xB7FF, 0x2001},
    {"Copper Link Partner", 0x08, READ_ONLY,  {2}, 0x0000, 0x0000},    
    {"1000BT Control",      0x09, READ_ONLY,  {2}, 0xF200, 0x0f00},
    {"1000BT Status",       0x0A, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"MMD Control",         0x0D, READ_ONLY,  {2}, 0xC000, 0x0000},
    {"MMD Addr/Data",       0x0E, READ_WRITE, {2}, 0xFFFF, 0x0000},
    {"Extended Status",     0x0F, READ_ONLY,  {2}, 0x0000, 0x3000},
    {"Copper Spec Cntl1",   0x10, READ_ONLY,  {2}, 0x0000, 0x3060},
    {"Copper Spec Ststus",  0x11, READ_ONLY,  {2}, 0x0000, 0x8040},
    {"Copper Spec Intr Ena",0x12, READ_ONLY,  {2}, 0xFFFF, 0x0000},
    {"Copper Intr Status",  0x13, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Copper Spec Cntl2",   0x14, READ_ONLY,  {2}, 0x0000, 0x0020},
    {"Copper Spec Rx Err",  0x15, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Page Register",       0x16, READ_WRITE, {2}, 0x0000, 0x0000},
    {"Global Intr Status",  0x17, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Copper Spec Cntl3",   0x1A, READ_ONLY,  {2}, 0x0000, 0x0040},
    {"end",                 0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1680_reg_page2[] = {   /* Page 2*/
    {"MAC Spec Cntl1",      0x10, READ_ONLY,  {2}, 0x0000, 0x4008},
    {"MAC Spec Intr Ena",   0x12, READ_ONLY,  {2}, 0x008C, 0x0000},
    {"MAC Intr Status",     0x13, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"MAC Spec Cntl2",      0x15, READ_ONLY,  {2}, 0x0000, 0x1046},
    {"end",                 0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1680_reg_page3[] = {   /* Page 3*/
    {"LED Func Cntl1",      0x10, READ_WRITE, {2}, 0x00FF, 0x111e},
    {"LED Polarity Cntl",   0x11, READ_WRITE, {2}, 0xFF0F, 0x8800},
    {"LED Timer Cntl",      0x12, READ_WRITE, {2}, 0xF70F, 0x4b05},
    {"LED Func Cntl&Polar", 0x13, READ_WRITE, {2}, 0x0000, 0x0073},
    {"end",                 0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1680_reg_page4[] = {   /* Page 4*/
    {"QSGMII Control",      0x00, READ_ONLY,  {2}, 0x4000, 0x1140},
    {"QSGMII Status",       0x01, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"QSGMII Spec Cntl1",   0x10, READ_ONLY,  {2}, 0xF0C0, 0x6244},
    {"QSGMII Spec Status",  0x11, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"QSGMII Spec Intr Ena",0x12, READ_ONLY,  {2}, 0x7F80, 0x0000},
    {"QSGMII Intr Status",  0x13, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"QSGMII RX_ER Byte",   0x14, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"QSGMII Rx Err Cnt",   0x15, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Page Register",       0x16, READ_WRITE, {2}, 0x0000, 0x0000},
    {"PRBS Control",        0x17, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PRBS Err Cnt LSB",    0x18, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PRBS Err Cnt MSB",    0x19, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"QSGMII Global Cntr1", 0x1A, READ_ONLY,  {2}, 0x3204, 0xC000},
    {"QSGMII Global Cntr2", 0x1B, READ_ONLY,  {2}, 0x4103, 0x3f80},
    {"end",                 0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1680_reg_page5[] = {   /* Page 5*/
    {"Adv VCT TX MDI0",     0x10, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Adv VCT TX MDI1",     0x11, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Adv VCT TX MDI2",     0x12, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Adv VCT TX MDI3",     0x13, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"1000BT Pair Skew",    0x14, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"1000BT Pair Swap",    0x15, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Page Register",       0x16, READ_WRITE, {2}, 0x0000, 0x0000},
    {"Adv VCT Control",     0x17, READ_ONLY,  {2}, 0x3FFF, 0x0603},
    {"Adv VCT Sample point",0x18, READ_ONLY,  {2}, 0x03FF, 0x000},
    {"Adv VCT Cross Pair",  0x19, READ_ONLY,  {2}, 0x7F7F, 0x0104},
    {"Adv VCT Same Pair 01",0x1A, READ_ONLY,  {2}, 0x7F7F, 0x0F12},
    {"Adv VCT Same Pair 23",0x1B, READ_ONLY,  {2}, 0x7F7F, 0x0A0C},
    {"Adv VCT Same Pair 4", 0x1C, READ_ONLY,  {2}, 0x3FFF, 0x0006},
    {"end",                 0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1680_reg_page6[] = {   /* Page 6*/
    {"Packet Generation",   0x10, READ_ONLY,  {2}, 0xFF07, 0x0000},
    {"CRC Counters",        0x11, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Checker Control",     0x12, READ_ONLY,  {2}, 0x0008, 0x0000},
    {"Copper Port IPG Cntl",0x13, READ_ONLY,  {2}, 0x00FF, 0x000B},
    {"General Control",     0x14, READ_ONLY,  {2}, 0x0000, 0x0200},
    {"Page Register",       0x16, READ_WRITE, {2}, 0x0000, 0x0000},
    {"Late Colli Cnt1&2",   0x17, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Late Colli Cnt3&4",   0x18, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Late Colli Window",   0x19, READ_WRITE, {2}, 0x1F00, 0x0000},
    {"Misc Test",           0x1A, READ_WRITE, {2}, 0x9F00, 0x1900},
    {"Temperature Sensor",  0x1B, READ_WRITE, {2}, 0x1F00, 0x0C00},
    {"end",                 0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1680_reg_page7[] = {   /* Page 7*/
    {"PHY Cable Diag 0",    0x10, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PHY Cable Diag 1",    0x11, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PHY Cable Diag 2",    0x12, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PHY Cable Diag 3",    0x13, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PHY Cable Diag Relt", 0x14, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PHY Cable Diag Cntl", 0x15, READ_ONLY,  {2}, 0x0400, 0x4000},
    {"Page Register",       0x16, READ_WRITE, {2}, 0x0000, 0x0000},
    {"Adv VCT Cros Pair",   0x19, READ_WRITE, {2}, 0x7F7F, 0x0104},
    {"Adv VCT Same Pair 01",0x1A, READ_WRITE, {2}, 0x7F7F, 0x0F12},
    {"Adv VCT Same Pair 23",0x1B, READ_WRITE, {2}, 0x7F7F, 0x0A0C},
    {"Adv VCT Same Pair 4", 0x1C, READ_WRITE, {2}, 0x007F, 0x0006},
    {"end",                 0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1680_reg_page8[] = {   /* Page 8*/
    {"PTP Port Cntl 0",     0x00, READ_ONLY,  {2}, 0x0000, 0x1000},
    {"PTP Port Cntl 1",     0x01, READ_ONLY,  {2}, 0x0000, 0x020C},     
    {"PTP Port Cntl 2",     0x02, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Port Cntl 8",     0x08, READ_ONLY,  {2}, 0x0000, 0x0000},  
    {"PTP Arr0 Byte1&0",    0x09, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Arr0 Byte3&2",    0x0A, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Arr0 Sequ ID",    0x0B, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Arr1 Byte1&0",    0x0C, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Arr1 Byte3&2",    0x0D, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Arr1 Sequ ID",    0x0E, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Page Register",       0x16, READ_WRITE, {2}, 0x0000, 0x0000},
    {"end",                 0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1680_reg_page9[] = {   /* Page 9*/
    {"PTP Dep Status",      0x00, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Dep Byte1&0",     0x01, READ_ONLY,  {2}, 0x0000, 0x0000},     
    {"PTP Dep Byte3&2",     0x02, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Dep Sequ ID",     0x03, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Dep Cnt",         0x05, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Page Register",       0x16, READ_WRITE, {2}, 0x0000, 0x0000},
    {"end",                 0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1680_reg_page12[] = {   /* Page 12*/
    {"TAI Global Conf 0",   0x00, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"TAI Global Conf 1",   0x01, READ_ONLY,  {2}, 0x0000, 0x1F40},     
    {"TAI Global Conf 2",   0x02, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"TAI Global Conf 3",   0x03, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"TAI Global Conf 4",   0x04, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"TAI Global Conf 5",   0x05, READ_ONLY,  {2}, 0x0000, 0xF000},
    {"TAI Global Conf 8",   0x08, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"TAI Globle Conf 9",   0x09, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Event Cap Byte1&0",   0x0A, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Event Cap Byte3&2",   0x0B, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"TAI Global Conf 12",  0x0C, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"TAI Global Conf 13",  0x0D, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Global Time1&0",  0x0E, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Global Time3&2",  0x0F, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Page Register",       0x16, READ_WRITE, {2}, 0x0000, 0x0000},
    {"end",                 0x00, 0, {0}, 0, 0},
};

static const reg_info_t marvell_88e1680_reg_page14[] = {   /* Page 14*/
    {"PTP Global Conf 0",   0x00, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Global Conf 1",   0x01, READ_ONLY,  {2}, 0x0000, 0x0000},     
    {"PTP Global Conf 2",   0x02, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PTP Global Conf 3",   0x03, READ_ONLY,  {2}, 0x0000, 0x0001},
    {"PTP Global Status",   0x08, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"Page Register",       0x16, READ_WRITE, {2}, 0x0000, 0x0000},
    {"end",                 0x00, 0, {0}, 0, 0},
};


/******************************************************************************
 *
 * Name:	mrv88e1680_dev_create()
 *
 * Description:	Create object with various device function
 *        		point to "do nothing"
 *
 * Input:	dev_object_t pointer to the 88e1680 device.
 *		    error reporting function pointer.
 *
 * Returns:	none
 *
 *****************************************************************************/
void mrv88e1680_dev_create (dev_object_t *dev, dev_error_report_t error_report_fn)
{
    dev_object_fvt_t	*dev_fvt;
    dev_88e1680_object_t *obj_88e1680= (dev_88e1680_object_t*)dev;

    /* Allocate memory for the device object */
    if ((dev_fvt = (dev_object_fvt_t *)malloc(sizeof(dev_object_fvt_t))) ==
		            NULL) {
        /* Unable to allocate memory */
        error_report_fn(dev, "malloc failure in 88e1680_dev_create()", 0);
        printf("%s: NULL\n", __func__);
	    return;
    }

    /* Init the device object structure to default "do nothing" */
    init_default_dev_object(dev, dev_fvt);

    obj_88e1680->base.dev_object_fvt->dev_attach	= dev_88e1680_attach;
    obj_88e1680->base.dev_object_fvt->dev_detach	= dev_88e1680_detach;
    obj_88e1680->base.dev_object_fvt->dev_restart	= dev_88e1680_restart;
    obj_88e1680->base.dev_object_fvt->dev_error_report	= error_report_fn;
    obj_88e1680->base.dev_object_fvt->dev_destroy	= dev_88e1680_destroy;
    obj_88e1680->base.dev_object_fvt->dev_name	= "Marvell GE PHY 88e1680";

    obj_88e1680->callin_fvt = (dev_88e1680_callin_fvt_t *)
                               malloc(sizeof(dev_88e1680_callin_fvt_t));
    obj_88e1680->callout_fvt = (dev_88e1680_callout_fvt_t *)
                                malloc(sizeof(dev_88e1680_callout_fvt_t));

    obj_88e1680->base.dev_state = DEV_STATE_CREATE;
}


/******************************************************************************
 *
 * Name:	dev_88e1680_attach()
 *
 * Description:	Attach the 88e1680 device for use. This function will
 *		        initialize and setup all necessary pointers and bring the
 *        		chip to operation.
 *
 * Input:	Pointer to the 88e1680 device object
 *
 * Returns:	PASSED/FAILED
 *
 *****************************************************************************/
static uint32 dev_88e1680_attach (dev_object_t *dev)
{
    dev_88e1680_object_t *obj_88e1680 = (dev_88e1680_object_t *) dev;

    if (obj_88e1680->callin_fvt == NULL) {
        DEV_ERROR_REPORT(dev, "dev_88e1680_attach() callin malloc", DEV_88E1680_ATTACH);
        return (FAILED);
    }

    if (obj_88e1680->callout_fvt == NULL) {
        DEV_ERROR_REPORT(dev, "dev_88e1680_attach() callout malloc", DEV_88E1680_ATTACH);
        return (FAILED);
    }

    obj_88e1680->callin_fvt->phy_start_mad_driver = dev_88e1680_start_mad_driver;
    obj_88e1680->callin_fvt->phy_config = dev_88e1680_phy_config;
    obj_88e1680->callin_fvt->start_mac_lpbk = dev_88e1680_start_mac_lpbk;
    obj_88e1680->callin_fvt->start_ext_lpbk = dev_88e1680_start_ext_lpbk;
    obj_88e1680->callin_fvt->phy_force_speed = dev_88e1680_phy_force_speed;
    obj_88e1680->callin_fvt->phy_register_tests = dev_88e1680_phy_register_tests;
    obj_88e1680->callin_fvt->phy_reg_test_single = dev_88e1680_phy_reg_test_single;
    obj_88e1680->callin_fvt->phy_reg_test = dev_88e1680_phy_reg_test;
    obj_88e1680->callin_fvt->phy_detect_phone = dev_88e1680_phy_detect_phone;
    obj_88e1680->callin_fvt->print_phy_counter = dev_88e1680_print_phy_counter;
    obj_88e1680->callin_fvt->clear_phy_counter = dev_88e1680_clear_phy_counter;
    obj_88e1680->callin_fvt->dump_phy_reg = dev_88e1680_dump_phy_reg;
    obj_88e1680->callin_fvt->reset_phy = dev_88e1680_reset_phy;
    obj_88e1680->callin_fvt->read_phy_reg_util = dev_88e1680_read_phy_reg_util;
    obj_88e1680->callin_fvt->write_phy_reg_util = dev_88e1680_write_phy_reg_util;
    obj_88e1680->callin_fvt->phy_intr_test = dev_88e1680_phy_intr_test;
    obj_88e1680->callin_fvt->led_on = dev_88e1680_led_on;
    obj_88e1680->callin_fvt->led_off = dev_88e1680_led_off;
    obj_88e1680->callin_fvt->led_default = dev_88e1680_led_default;
    obj_88e1680->callin_fvt->gen_int = dev_88e1680_gen_int;
    obj_88e1680->callin_fvt->clear_int = dev_88e1680_clear_int;
    obj_88e1680->callin_fvt->set_test_mode = dev_88e1680_set_test_mode;

    return (PASSED);
}


/******************************************************************************
 *
 * Name:	dev_88e1680_detach()
 *
 * Description:	detach the device specific functions from the caller.
 *	        	All of the device specific function are connected to the
 *        		dev_do_nothing() function, except for the dev_attach()
 *        		function. Also, the dev_state must be assigned the value
 *        		of DEV_STATE_DETACH.
 *
 *        		Since, some platforms may want to detach the device, but not
 *        		release the memory resources (via a free () in the
 *        		dev_destroy()), this function can be executed to accomplish
 *        		this task. However, before a detached device can be used again,
 *        		it must be re-attached (via the dev_attach()).
 *
 * Input:	Pointer to the 88e1680 device object
 *
 * Returns:	PASSED/FAILED
 *
 *****************************************************************************/
static uint32 dev_88e1680_detach (dev_object_t *dev)
{
    dev_88e1680_object_t *obj_88e1680 = (dev_88e1680_object_t *) dev;

    /* Init the device object structure to default "do nothing" */
    init_default_dev_object(dev, obj_88e1680->base.dev_object_fvt);

    obj_88e1680->base.dev_state = DEV_STATE_DETACH;

    return (PASSED);

}


/******************************************************************************
 * Name:	dev_88e1680_restart
 *
 * Description:	To restart a previously initialized device without resetting
 *		        the device or changing its configuration.
 *		        For example, during a failover event.
 *
 *		        Change the state of the device from its current state
 *		        to an initial state. Also, dev_state must be assigned the
 *		        value of DEV_STATE_INIT.
 *
 * Input:	dev_object_t pointer to the 88e1680 device
 *
 * Returns:	PASSED/FAILED
 *
 * Assumptions:	The dev_attach() and dev_reconfig_needed() functions has been
 *		        called and successfully executed.
 *
 *****************************************************************************/
static uint32 dev_88e1680_restart (dev_object_t *dev)
{
    dev_88e1680_object_t *obj_88e1680= (dev_88e1680_object_t *) dev;

    obj_88e1680->base.dev_state = DEV_STATE_INIT;
    return (PASSED);
}


/******************************************************************************
 * Name:	dev_88e1680_destroy
 *
 * Description:	Destroy the dev_object structure and free all the resources.
 *
 * Input:	dev_object_t pointer to the 88e1680 device
 *
 * Returns:	none
 *
 * Assumptions:	The dev_attch() function has been called and successfully
 *
 *****************************************************************************/
static void dev_88e1680_destroy (dev_object_t **dev)
{
    dev_88e1680_object_t *obj_88e1680;

    if (dev == NULL) {
        return;
    }

    if (*dev == NULL) {
        return;
    }

    obj_88e1680 = (dev_88e1680_object_t *)*dev;

    if (obj_88e1680->callout_fvt) {
        free(obj_88e1680->callout_fvt);	/* Free callout struct */
    }

    if (obj_88e1680->callin_fvt) {
        free(obj_88e1680->callin_fvt);		/* Free callin struct */
    }

    free(obj_88e1680->base.dev_object_fvt);	/* Free dev_object_t */
}


/**********************************************************************
 *
 * Function: dev_88e1680_start_mad_driver
 *
 * This function: Marvell 88E1680 start mad driver
 *
 * Input : dev - dev_object_t pointer to the 88e1680 device
 *            maddev - MAD_DEV pointer to the 88e1680 device
 *            smi_addr - smi addr base
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int dev_88e1680_start_mad_driver(dev_object_t *dev, MAD_DEV *maddev, int smi_addr)
{
    int rc;
    unsigned char port;
    dev_88e1680_object_t *obj_88e1680 = (dev_88e1680_object_t *) dev;
    dev_88e1680_callout_fvt_t *callout_p = obj_88e1680->callout_fvt;

    rc = (*callout_p->phy_mad_load_driver)(maddev, (int)smi_addr);
    if (rc != PASSED) {
        sprintf(mrv_88e1680_err_buf, "%s: Failed (%#x)", __func__, rc);
        return (FAILED);
    }

    /* disable all the interrupts */
    rc = (*callout_p->phy_mad_disable_int)(maddev);
    if (rc != PASSED) {
        sprintf(mrv_88e1680_err_buf, "%s: Failed (%#x)", __func__, rc);
        return (FAILED);
    }

    /* init all PHYs */
    for(port=0; port<maddev->numOfPorts; port++) {
	if (dev_88e1680_phy_init(dev, maddev, port) == FAILED)
	    return (FAILED);

	if (dev_88e1680_phy_config(dev, maddev, port) == FAILED)
	    return (FAILED);
    }

    /* enable all PHYs */
    for(port=0; port<maddev->numOfPorts; port++) {
        if((*callout_p->phy_mad_set_phy_enable)(maddev, port, MAD_TRUE) != PASSED) {
            cterr('f',0,"Failed to Enable Phy (port %i)\n",port);
            return FAILED;
        }
    }
    return (PASSED);
	
}


/**********************************************************************
 *
 * Function: dev_88e1680_phy_1680_init
 *
 * This function: Marvell 88E1680 phy 1680 init
 *
 * Input : dev - dev_object_t pointer to the 88e1680 device
 *            maddev - MAD_DEV pointer to the 88e1680 device
 *            port - port numer
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int dev_88e1680_phy_1680_init(dev_object_t *dev, MAD_DEV *maddev, int port)
{

    int rc;
    MAD_U32 reg_d;

    dev_88e1680_object_t *obj_88e1680 = (dev_88e1680_object_t *) dev;
    dev_88e1680_callout_fvt_t *callout_p = obj_88e1680->callout_fvt;

    assert(maddev);

    /* workaround implement */
    rc = (*callout_p->phy_read_reg)(maddev, port, 4, 27, &reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }

    /* write 27.5 = 1 */
    reg_d |= 1 << 5;
    rc = (*callout_p->phy_write_reg)(maddev, port, 4, 27, reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }

    /* page 253, for QSGMII TX amplitude change */
    rc = (*callout_p->phy_write_reg)(maddev, port, 0x00FD, 8, 0x0B53);
    if (rc != PASSED) {
	return (FAILED);
    }

    rc = (*callout_p->phy_write_reg)(maddev, port, 0x00FD, 7, 0x200D);
    if (rc != PASSED) {
	return (FAILED);
    }

    /* page 255, for EEE initialization */
    rc = (*callout_p->phy_write_reg)(maddev, port, 0x00FF, 17, 0xB030);
    if (rc != PASSED) {
	return (FAILED);
    }

    rc = (*callout_p->phy_write_reg)(maddev, port, 0x00FF, 16, 0x215C);
    if (rc != PASSED) {
	return (FAILED);
    }

    return (PASSED);
}


/**********************************************************************
 *
 * Function: dev_88e1680_phy_init
 *
 * This function: Marvell 88E1680 phy init
 *
 * Input : dev - dev_object_t pointer to the 88e1680 device
 *            maddev - MAD_DEV pointer to the 88e1680 device
 *            port - port numer
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int dev_88e1680_phy_init (dev_object_t *dev, MAD_DEV *maddev, int port)
{

    int rc;
    MAD_U32 reg_d;

    dev_88e1680_object_t *obj_88e1680 = (dev_88e1680_object_t *) dev;
    dev_88e1680_callout_fvt_t *callout_p = obj_88e1680->callout_fvt;


    /* workaroud implement */
    /* 1680L revision A0 */
    rc = dev_88e1680_phy_1680_init(dev, maddev, port);
    if (rc != PASSED) {
	return (rc);
    }

    /* power down/ power up QSMGII */
    rc= (*callout_p->phy_read_reg)(maddev, port, 4, 0, &reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }
    reg_d |= 1 << 11;
    rc= (*callout_p->phy_write_reg)(maddev, port, 4, 0, reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }
    
    rc= (*callout_p->phy_read_reg)(maddev, port, 4, 0, &reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }
    reg_d &= ~(1 << 11);
    rc= (*callout_p->phy_write_reg)(maddev, port, 4, 0, reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }

    /* disable MacSec */
    rc = (*callout_p->phy_read_reg)(maddev, port, 18, 27, &reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }

    /* write 27_18.13 = 0 */
    reg_d &= ~(1 << 13);
    rc = (*callout_p->phy_write_reg)(maddev, port, 18, 27, reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }

    /* enable QSGMII counter */
    rc = (*callout_p->phy_write_reg)(maddev, port, 18, 18, 0x0006);
    if (rc != PASSED) {
	return (FAILED);
    }
    
    /* enable copper counter */
    rc = (*callout_p->phy_write_reg)(maddev, port, 6, 16, 0x0010);
    if (rc != PASSED) {
	return (FAILED);
    }
    
    return (PASSED);
}


/**********************************************************************
 *
 * Function: dev_88e1680_phy_config
 *
 * This function: Marvell 88E1680 phy config
 *
 * Input : dev - dev_object_t pointer to the 88e1680 device
 *            maddev - MAD_DEV pointer to the 88e1680 device
 *            port - port numer
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int dev_88e1680_phy_config(dev_object_t *dev, MAD_DEV *maddev, int port)
{

    int rc;
    MAD_U32 reg_d;

    dev_88e1680_object_t *obj_88e1680 = (dev_88e1680_object_t *) dev;
    dev_88e1680_callout_fvt_t *callout_p = obj_88e1680->callout_fvt;

    assert(maddev);

    /* Enable 1Gbps advertise (page 0, reg 9)*/
    rc = (*callout_p->phy_read_reg)(maddev, port, 0, 9, &reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }
    reg_d |= PHY_1000BT_ADV;
    rc = (*callout_p->phy_write_reg)(maddev, port, 0, 9, reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }

    /* Enable 10 & 100 Mbps advertise (page 0, reg 4)*/
    rc = (*callout_p->phy_read_reg)(maddev, port, 0, 4, &reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }
    reg_d |= PHY_100BT_ADV;
    reg_d |= PHY_10BT_ADV;
    rc = (*callout_p->phy_write_reg)(maddev, port, 0, 4, reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }

    /* config phy speed 1Gpbs for SGMII (page 2, reg 21) */
    rc = (*callout_p->phy_read_reg)(maddev, port, 2, 21, &reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }
    reg_d &= ~PHY_MAC_SPD_MASK;
    reg_d |= PHY_MAC_SPD_1000M;
    rc = (*callout_p->phy_write_reg)(maddev, port, 2, 21, reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }

    /* clear 1000BT PHY External loopback mode */
    rc = (*callout_p->phy_read_reg)(maddev, port, 6, 18, &reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }
    reg_d &= ~PHY_ENA_STUB_TEST;
    rc = (*callout_p->phy_write_reg)(maddev, port, 6, 18, reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }

    /* set 1Gbps & auto-neg & full-duplex, clear loopback, soft reset (page 0, reg 0) */
    reg_d = PHY_COPPER_FULL_DUPLEX | PHY_SPD_SEL_1000M | PHY_AUTO_NEO_ENA | PHY_COOPER_RST;
    rc = (*callout_p->phy_write_reg)(maddev, port, 0, 0, reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }

    return (PASSED);
}


/**********************************************************************
 *
 * Function: dev_88e1680_print_phy_counter
 *
 * This function: Marvell 88E1680 print PHY counter
 *
 * Input : dev - dev_object_t pointer to the 88e1680 device
 *            maddev - MAD_DEV pointer to the 88e1680 device
 *            port_num - number of ports
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
dev_88e1680_print_phy_counter(dev_object_t *dev, MAD_DEV *maddev, uint port_num)
{

    MAD_U32 reg_d1, reg_d2;

    dev_88e1680_object_t *obj_88e1680 = (dev_88e1680_object_t *) dev;
    dev_88e1680_callout_fvt_t *callout_p = obj_88e1680->callout_fvt;

    (*callout_p->phy_read_reg)(maddev, port_num, 18, 17, &reg_d1);
    (*callout_p->phy_read_reg)(maddev, port_num, 6, 17, &reg_d2);
    printf("counters for PHY port %d: QSGMII = 0x%x, Copper = 0x%x\n", port_num, reg_d1, reg_d2);

    return (PASSED);
}


/**********************************************************************
 *
 * Function: dev_88e1680_clear_phy_counter
 *
 * This function: Marvell 88E1680 clear PHY counter
 *
 * Input : dev - dev_object_t pointer to the 88e1680 device
 *            maddev - MAD_DEV pointer to the 88e1680 device
 *            port_num - number of ports
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int 
dev_88e1680_clear_phy_counter (dev_object_t *dev, MAD_DEV *maddev,  uint port_num)
{

    dev_88e1680_object_t *obj_88e1680 = (dev_88e1680_object_t *) dev;
    dev_88e1680_callout_fvt_t *callout_p = obj_88e1680->callout_fvt;

    (*callout_p->phy_write_reg)(maddev, port_num, 6, 18, 0x0010);
    (*callout_p->phy_write_reg)(maddev, port_num, 18, 18, 0x0016);

    return (PASSED);
}


/**********************************************************************
 *
 * Function: dev_88e1680_start_mac_lpbk
 *
 * This function: Marvell 88E1680 starts mac loopback
 *
 * Input : dev - dev_object_t pointer to the 88e1680 device
 *            maddev - MAD_DEV pointer to the 88e1680 device
 *            port - number of ports
 *            speed - target speed
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */

static int dev_88e1680_start_mac_lpbk (dev_object_t *dev, MAD_DEV *maddev, int port, MAD_SPEED_MODE speed)
{
    MAD_U32 reg_d, target_speed;
    int rc;

    dev_88e1680_object_t *obj_88e1680 = (dev_88e1680_object_t *) dev;
    dev_88e1680_callout_fvt_t *callout_p = obj_88e1680->callout_fvt;

    if (maddev == 0) {
	cterr('f',0,"MAD driver is not initialized.");
        return FAILED;
    }

    /* disable stub test */
    rc = (*callout_p->phy_read_reg)(maddev, port, 6, 18, &reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }
    reg_d &= ~PHY_ENA_STUB_TEST;
    rc = (*callout_p->phy_write_reg)(maddev, port, 6, 18, reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }

    /* set QSGMII speed (page 2, reg 21) */
    if (speed == MAD_SPEED_1000M) {
	 //printf("SPEED 1000 CONFIG.\n");
        reg_d = 0x1046;
    } else if (speed == MAD_SPEED_100M) {
        //printf("SPEED 100 CONFIG.\n");
        reg_d = 0x1045;
    } else {
        //printf("SPEED 10 CONFIG.\n");
        reg_d = 0x1044;
    }
    rc = (*callout_p->phy_write_reg)(maddev, port, 2, 21, reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }

    if (speed == MAD_SPEED_1000M) {
        /* force master (page 0, reg 9) */
        reg_d = 0x1f00;
        rc = (*callout_p->phy_write_reg)(maddev, port, 0, 9, reg_d);
        if (rc != PASSED) {
	    return (FAILED);
        }
    }

    /* set Cooper control (page 0, reg 0) */
    if (speed == MAD_SPEED_1000M) {
        reg_d = 0x9140;
    } else if (speed == MAD_SPEED_100M) {
        reg_d = 0xA100;
    } else {
        reg_d = 0x8100;
    }
    rc = (*callout_p->phy_write_reg)(maddev, port, 0, 0, reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }
    msleep(100);

    if (speed == MAD_SPEED_1000M) {
        /* set page 0xfa */
        reg_d = 0x0418;
        rc = (*callout_p->phy_write_reg)(maddev, port, 0xfa, 1, reg_d);
        if (rc != PASSED) {
	    return (FAILED);
        }
        reg_d = 0x020c;
        rc = (*callout_p->phy_write_reg)(maddev, port, 0xfa, 7, reg_d);
        if (rc != PASSED) {
	    return (FAILED);
        }
    } else {
        /* speed 100/10 */
        rc = (*callout_p->phy_read_reg)(maddev, port, 0, 16, &reg_d);
        if (rc != PASSED) {
	     return (FAILED);
        }
        /* Force link good */
        reg_d = reg_d | 0x400;
        rc = (*callout_p->phy_write_reg)(maddev, port, 0, 16, reg_d);
        if (rc != PASSED) {
	     return (FAILED);
        }
    }
    msleep(200);

    
    /* Check copper link speed (page 0, reg 17)*/
    rc = (*callout_p->phy_read_reg)(maddev, port, 0, 17, &reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
	if ((reg_d & PHY_COPPER_LINK) != PHY_COPPER_LINK) {
	   printf("\nCopper link is not up. register @ (page 0, reg 17) = %#x\n", reg_d);
	} else {
	   printf("\nCopper link is up. register @ (page 0, reg 17) = %#x\n", reg_d);	
	}
    }

    if (speed == MAD_SPEED_1000M) {
        target_speed = PHY_LINK_SPEED_1000;
    } else if (speed == MAD_SPEED_100M) {
        target_speed = PHY_LINK_SPEED_100;
    } else {
        target_speed = PHY_LINK_SPEED_10;
    }

    if ((reg_d & PHY_LINK_SPEED_MASK) != target_speed) {
	if (speed == MAD_SPEED_1000M) {
            cterr('f',0,"Copper side speed is not 1000Mbps.");
        } else if (speed == MAD_SPEED_100M) {
            cterr('f',0,"Copper side speed is not 100Mbps.");
        } else {
            cterr('f',0,"Copper side speed is not 10Mbps.");
        }
	return (FAILED);
    }  
    if ((reg_d & PHY_FULL_DUPLEX) != PHY_FULL_DUPLEX) {
	cterr('f',0,"Copper side is not full duplex.");
	return (FAILED);
    } 

    /* Check MAC Side Link up and speed (page 4, reg 17)*/
    rc = (*callout_p->phy_read_reg)(maddev, port, 4, 17, &reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
	if ((reg_d & PHY_COPPER_LINK) != PHY_COPPER_LINK) {
	    printf("\nQSGMII link is not up. register @ (page 4, reg 17) = %#x\n", reg_d);
	} else {
	    printf("\nQSGMII link is up. register @ (page 4, reg 17) = %#x\n", reg_d);	
	}
    }

    if ((reg_d & PHY_LINK_SPEED_MASK) != target_speed) {
       if (speed == MAD_SPEED_1000M) {
            cterr('f',0,"MAC side speed is not 1000Mbps.");
       } else if (speed == MAD_SPEED_100M) {
            cterr('f',0,"MAC side speed is not 100Mbps.");
       } else {
            cterr('f',0,"MAC side speed is not 10Mbps.");
       }
	return (FAILED);
    }
    if ((reg_d & PHY_FULL_DUPLEX) != PHY_FULL_DUPLEX) {
	cterr('f',0,"MAC side is not full duplex.");
	return (FAILED);
    } 

    /* set loopback */
    rc = (*callout_p->phy_read_reg)(maddev, port, 0, 0, &reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }
    reg_d |= PHY_LPBK_ENA;
    rc = (*callout_p->phy_write_reg)(maddev, port, 0, 0, reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }

    msleep(2000); 

     if (speed == MAD_SPEED_1000M) {
        /* set page 0xfa */
        reg_d = 0x0200;
        rc = (*callout_p->phy_write_reg)(maddev, port, 0xfa, 7, reg_d);
        if (rc != PASSED) {
	    return (FAILED);
        }
        reg_d = 0x0400;
        rc = (*callout_p->phy_write_reg)(maddev, port, 0xfa, 1, reg_d);
        if (rc != PASSED) {
	    return (FAILED);
        }
    } else {

	 rc = (*callout_p->phy_read_reg)(maddev, port, 0, 16, &reg_d);
        if (rc != PASSED) {
	     return (FAILED);
        }
	 /* Force link good */
	 reg_d = reg_d & ~(0x400);
	 rc = (*callout_p->phy_write_reg)(maddev, port, 0, 16, reg_d);
        if (rc != PASSED) {
	     return (FAILED);
        }
        
    }

    msleep(200);

    /* Check MAC Side Link up and Sync (page 4, reg 17)*/
    rc = (*callout_p->phy_read_reg)(maddev, port, 4, 17, &reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
	if ((reg_d & PHY_COPPER_LINK) != PHY_COPPER_LINK) {
	    printf("\nQSGMII link is not up. register @ (page 4, reg 17) = %#x\n", reg_d);
	} else {
	    printf("\nQSGMII link is up. register @ (page 4, reg 17) = %#x\n", reg_d);	
	}
    }
	
    if (!(reg_d & PHY_SYNC)) {
	return (FAILED);
    }
    if ((reg_d & PHY_LINK_SPEED_MASK) != target_speed) {
	if (speed == MAD_SPEED_1000M) {
            cterr('f',0,"MAC side speed is not 1000Mbps..");
       } else if (speed == MAD_SPEED_100M) {
            cterr('f',0,"MAC side speed is not 100Mbps..");
       } else {
            cterr('f',0,"MAC side speed is not 10Mbps..");
       }
	return (FAILED);
    }
    if ((reg_d & PHY_FULL_DUPLEX) != PHY_FULL_DUPLEX) {
	cterr('f',0,"MAC side is not full duplex.");
	return (FAILED);
    } 

    return PASSED;

}


/**********************************************************************
 *
 * Function: dev_88e1680_start_ext_lpbk
 *
 * This function: Marvell 88E1680 starts external loopback
 *
 * Input : dev - dev_object_t pointer to the 88e1680 device
 *            maddev - MAD_DEV pointer to the 88e1680 device
 *            port - number of ports
 *            speed - target speed 
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int dev_88e1680_start_ext_lpbk (dev_object_t *dev, MAD_DEV *maddev, int port, MAD_SPEED_MODE speed)
{
    MAD_U32 reg_d, target_speed;
    int rc;

    dev_88e1680_object_t *obj_88e1680 = (dev_88e1680_object_t *) dev;
    dev_88e1680_callout_fvt_t *callout_p = obj_88e1680->callout_fvt;

    if (maddev == 0) {
        cterr('f',0,"MAD driver is not initialized");
        return FAILED;
    }

    if (speed == MAD_SPEED_1000M) {
        /* enable stub test */
        rc = (*callout_p->phy_read_reg)(maddev, port, 6, 18, &reg_d);
        if (rc != PASSED) {
	    return (FAILED);
        }
        reg_d |= PHY_ENA_STUB_TEST;
        rc = (*callout_p->phy_write_reg)(maddev, port, 6, 18, reg_d);
        if (rc != PASSED) {
	    return (FAILED);
        }
    } else {
        /* disable stub test */
        rc = (*callout_p->phy_read_reg)(maddev, port, 6, 18, &reg_d);
        if (rc != PASSED) {
	    return (FAILED);
        }
        reg_d &= ~(PHY_ENA_STUB_TEST);
        rc = (*callout_p->phy_write_reg)(maddev, port, 6, 18, reg_d);
        if (rc != PASSED) {
	    return (FAILED);
        }
    }

    if (speed == MAD_SPEED_1000M) {
        /* set speed (page 0, reg 9)*/
        rc = (*callout_p->phy_read_reg)(maddev, port, 0, 9, &reg_d);
        if (rc != PASSED) {
            return (FAILED);
        }
        reg_d |= PHY_1000BT_ADV;
        rc = (*callout_p->phy_write_reg)(maddev, port, 0, 9, reg_d);
        if (rc != PASSED) {
            return (FAILED);
        }
    } else {
        rc = (*callout_p->phy_read_reg)(maddev, port, 0, 9, &reg_d);
        if (rc != PASSED) {
            return (FAILED);
        }
        reg_d &= ~(PHY_1000BT_ADV);
        rc = (*callout_p->phy_write_reg)(maddev, port, 0, 9, reg_d);
        if (rc != PASSED) {
            return (FAILED);
        }
        if (speed == MAD_SPEED_100M) {
            /* set speed (page 0, reg 4)*/
            rc = (*callout_p->phy_read_reg)(maddev, port, 0, 4, &reg_d);
            if (rc != PASSED) {
                return (FAILED);
            }
            reg_d |= PHY_100BT_ADV;
            rc = (*callout_p->phy_write_reg)(maddev, port, 0, 4, reg_d);
            if (rc != PASSED) {
                return (FAILED);
            }
        } else {
            /* set speed (page 0, reg 4)*/
            rc = (*callout_p->phy_read_reg)(maddev, port, 0, 4, &reg_d);
            if (rc != PASSED) {
                return (FAILED);
            }
            reg_d &= ~(PHY_100BT_ADV);
	     reg_d |= PHY_10BT_ADV;
            rc = (*callout_p->phy_write_reg)(maddev, port, 0, 4, reg_d);
            if (rc != PASSED) {
                return (FAILED);
            }
        }
    
    }

    /* PHY soft reset */
    /* set Cooper control (page 0, reg 0) */
    if (speed == MAD_SPEED_1000M) {
        reg_d = PHY_COPPER_FULL_DUPLEX | PHY_SPD_SEL_1000M | PHY_AUTO_NEO_ENA | PHY_COOPER_RST;
    } else if (speed == MAD_SPEED_100M) {
        reg_d = PHY_COPPER_FULL_DUPLEX | PHY_SPD_SEL_100M | PHY_COOPER_RST;
    } else {
        reg_d = PHY_COPPER_FULL_DUPLEX | PHY_SPD_SEL_10M | PHY_COOPER_RST;
    }
    //reg_d = PHY_COPPER_FULL_DUPLEX | PHY_SPD_SEL_1000M | PHY_AUTO_NEO_ENA | PHY_COOPER_RST;
    rc = (*callout_p->phy_write_reg)(maddev, port, 0, 0, reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }
    msleep(100);

    /* disable loopback */
    rc = (*callout_p->phy_read_reg)(maddev, port, 0, 0, &reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }

    reg_d &= ~PHY_LPBK_ENA;
    rc = (*callout_p->phy_write_reg)(maddev, port, 0, 0, reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }
    msleep(2000);  /* Wait for link up */

    if (speed == MAD_SPEED_1000M) {
        target_speed = PHY_LINK_SPEED_1000;
    } else if (speed == MAD_SPEED_100M) {
        target_speed = PHY_LINK_SPEED_100;
    } else {
        target_speed = PHY_LINK_SPEED_10;
    }

    /* Check copper link speed (page 0, reg 17)*/
    rc = (*callout_p->phy_read_reg)(maddev, port, 0, 17, &reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
	if ((reg_d & PHY_COPPER_LINK) != PHY_COPPER_LINK) {
	    printf("\nCopper link is not up. register @ (page 0, reg 17) = %#x\n", reg_d);
	} else {
	    printf("\nCopper link is up. register @ (page 0, reg 17) = %#x\n", reg_d);	
	}
    }

    if ((reg_d & PHY_LINK_SPEED_MASK) != target_speed) {
	if (speed == MAD_SPEED_1000M) {
            cterr('f',0,"Copper side speed is not 1000Mbps.");
        } else if (speed == MAD_SPEED_100M) {
            cterr('f',0,"Copper side speed is not 100Mbps.");
        } else {
            cterr('f',0,"Copper side speed is not 10Mbps.");
        }
	return (FAILED);
    } 
    /* Check MAC Side Link up and Sync (page 4, reg 17)*/
    rc = (*callout_p->phy_read_reg)(maddev, port, 4, 17, &reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }
    if (!(reg_d & PHY_SYNC)) {
	return (FAILED);
    }
    if ((reg_d & PHY_LINK_SPEED_MASK) != target_speed) {
	if (speed == MAD_SPEED_1000M) {
            cterr('f',0,"MAC side speed is not 1000Mbps..");
       } else if (speed == MAD_SPEED_100M) {
            cterr('f',0,"MAC side speed is not 100Mbps..");
       } else {
            cterr('f',0,"MAC side speed is not 10Mbps..");
       }
	return (FAILED);
    }

    return PASSED;
}



/**********************************************************************
 *
 * Function: dev_88e1680_phy_force_speed
 *
 * This function: Marvell 88E1680 phy force speed by modify advertise capabilities.
 *
 * Input : dev - dev_object_t pointer to the 88e1680 device
 *            maddev - MAD_DEV pointer to the 88e1680 device
 *            port - number of ports
 *            speed - target speed 
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int dev_88e1680_phy_force_speed (dev_object_t *dev, MAD_DEV *maddev, int port, MAD_SPEED_MODE speed)
{

    MAD_U32 reg_d;
    int rc;

    dev_88e1680_object_t *obj_88e1680 = (dev_88e1680_object_t *) dev;
    dev_88e1680_callout_fvt_t *callout_p = obj_88e1680->callout_fvt;

    if (maddev == 0) {
        cterr('f',0,"MAD driver is not initialized");
        return FAILED;
    }

    /* disable stub test */
    rc = (*callout_p->phy_read_reg)(maddev, port, 6, 18, &reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }
    reg_d &= ~(PHY_ENA_STUB_TEST);
    rc = (*callout_p->phy_write_reg)(maddev, port, 6, 18, reg_d);
    if (rc != PASSED) {
	return (FAILED);
    }

    if (speed == MAD_SPEED_1000M) {
        /* set speed (page 0, reg 9)*/
        rc = (*callout_p->phy_read_reg)(maddev, port, 0, 9, &reg_d);
        if (rc != PASSED) {
            return (FAILED);
        }
        reg_d |= PHY_1000BT_ADV;
        rc = (*callout_p->phy_write_reg)(maddev, port, 0, 9, reg_d);
        if (rc != PASSED) {
            return (FAILED);
        }
    } else {
        rc = (*callout_p->phy_read_reg)(maddev, port, 0, 9, &reg_d);
        if (rc != PASSED) {
            return (FAILED);
        }
        reg_d &= ~(PHY_1000BT_ADV);
        rc = (*callout_p->phy_write_reg)(maddev, port, 0, 9, reg_d);
        if (rc != PASSED) {
            return (FAILED);
        }
        if (speed == MAD_SPEED_100M) {
            /* set speed (page 0, reg 4)*/
            rc = (*callout_p->phy_read_reg)(maddev, port, 0, 4, &reg_d);
            if (rc != PASSED) {
                return (FAILED);
            }
            reg_d |= PHY_100BT_ADV;
            rc = (*callout_p->phy_write_reg)(maddev, port, 0, 4, reg_d);
            if (rc != PASSED) {
                return (FAILED);
            }
        } else {
            /* set speed (page 0, reg 4)*/
            rc = (*callout_p->phy_read_reg)(maddev, port, 0, 4, &reg_d);
            if (rc != PASSED) {
                return (FAILED);
            }
            reg_d &= ~(PHY_100BT_ADV);
	     reg_d |= PHY_10BT_ADV;
            rc = (*callout_p->phy_write_reg)(maddev, port, 0, 4, reg_d);
            if (rc != PASSED) {
                return (FAILED);
            }
        }
    
    }


    /* PHY soft reset */
    /* set Cooper control (page 0, reg 0) */
    reg_d = PHY_COPPER_FULL_DUPLEX | PHY_SPD_SEL_1000M | PHY_AUTO_NEO_ENA |PHY_COOPER_RST;
    rc = (*callout_p->phy_write_reg)(maddev, port, 0, 0, reg_d);
    
    if (rc != PASSED) {
	return (FAILED);
    }
    msleep(100);


    return PASSED;
}


/**********************************************************************
 *
 * Function: dev_88e1680_phy_register_tests
 *
 * This function: Marvell 88E1680 PHY register test
 *
 * Input : dev - dev_object_t pointer to the 88e1680 device
 *            maddev - MAD_DEV pointer to the 88e1680 device
 *            port_num - port number
 *            page_num - page number
 *            reg_ptr - reg_info_t pointer to 88e1680 device
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int 
dev_88e1680_phy_register_tests (dev_object_t *dev, MAD_DEV *maddev, MAD_LPORT port_num, MAD_U16 page_num, const reg_info_t *reg_ptr )
{
    uint32_t ix;
    uint retval, ret_val, save_val, readval;
    uint data, temp, tst_offset;

    dev_88e1680_object_t *obj_88e1680 = (dev_88e1680_object_t *) dev;
    dev_88e1680_callout_fvt_t *callout_p = obj_88e1680->callout_fvt;

    readval = 0;
    retval = PASSED;
    ret_val = PASSED;

    while (reg_ptr->size.size != 0) {
        retval = (*callout_p->phy_read_reg)(maddev, port_num, page_num, reg_ptr->offset, &save_val);
        if (retval == FAILED) {
            cterr('f',0,"%s(): Error reading PHY port %d page %d register %d",
		  __FUNCTION__, port_num, page_num, reg_ptr->offset);
            return (FAILED);
        }

        if (reg_ptr->type == READ_WRITE) {
            tst_offset = reg_ptr->offset;

            /* 
             * ripple 1 test
             */
            for (ix = 0; ix < (reg_ptr->size.size * 8); ix++) {

                temp = (1 << ix) & reg_ptr->mask;
                if (!temp) {
                    continue;
                }

                /* Write to register under test */
                retval = (*callout_p->phy_write_reg)(maddev, port_num, page_num, tst_offset, temp);

                /* Read back */
                if (retval == PASSED) {
                    ret_val = (*callout_p->phy_read_reg)(maddev, port_num, page_num, reg_ptr->offset, &readval);
                }
                if (((readval & reg_ptr->mask) != temp) ||
                    (retval == FAILED) || (ret_val == FAILED)) {
                    cterr('f',0,"%s(): Ripple one test failed when accessing "
			  "PHY port %d page %d register %d. "
			  " Expect %#x, Read %#x", __FUNCTION__,
			  port_num, page_num, tst_offset, temp, readval);
                    return (FAILED);
                }
            }

            /* 
             * ripple 0 test
             */
            for (ix = 0; ix < (reg_ptr->size.size * 8); ix++) {
                temp = (1 << ix) & reg_ptr->mask;
                if (!temp) {
                    continue;
                }
                temp = (~(1 << ix)) & reg_ptr->mask;
                /* Write to register under test */
                retval = (*callout_p->phy_write_reg)(maddev, port_num, page_num, tst_offset, temp);
                if (retval == PASSED) {
                    /* Read back */
                    ret_val = (*callout_p->phy_read_reg)(maddev, port_num, page_num, reg_ptr->offset, &readval);
                }
                if (((readval & reg_ptr->mask) != temp) ||
                    (retval == FAILED) || (ret_val == FAILED)) {
                    cterr('f',0,"%s(): Ripple zero test failed when accessing "
			  "PHY port %d page %d register %d. "
			  " Expect %#x, Read %#x", __FUNCTION__,
			  port_num, page_num, tst_offset, temp, readval);
                    return (FAILED);
                }
            }

            /*
             * pattern test
             */
            data = PATTERN;
            for (ix = 0; ix < 2; ix++) {
                temp = data & reg_ptr->mask;
                /* Write to register under test */
                retval = (*callout_p->phy_write_reg)(maddev, port_num, page_num, tst_offset, temp);
                if (retval == PASSED) {
                    /* Read back */
                    ret_val = (*callout_p->phy_read_reg)(maddev, port_num, page_num, reg_ptr->offset, &readval);
                }
                if (((readval & reg_ptr->mask) != temp) ||
                    (retval == FAILED) || (ret_val == FAILED)) {
                    cterr('f',0,"%s(): Pattern test failed when accessing "
			  "PHY port %d page %d register %d. "
			  " Expect %#x, Read %#x", __FUNCTION__,
			  port_num, page_num, tst_offset, temp, readval);
                    return (FAILED);
                }
    
                data = ~PATTERN; /* complement data pattern */
            }

            /*
             * restore original value
             */	    
	    retval = (*callout_p->phy_write_reg)(maddev, port_num, page_num, tst_offset, save_val);

            if (retval == FAILED) {
		cterr('f',0,"%s(): Error restoring PHY port %d page %d register %d. ",
		      __FUNCTION__, port_num, page_num, tst_offset);
		return (FAILED);
            }
        }
        reg_ptr++;
    }

    return (PASSED);
}


/**********************************************************************
 *
 * Function: dev_88e1680_phy_reg_test_single
 *
 * This function: Marvell 88E1680 PHY register test single
 *
 * Input : dev - dev_object_t pointer to the 88e1680 device
 *            maddev - MAD_DEV pointer to the 88e1680 device
 *            port_num - port number
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int 
dev_88e1680_phy_reg_test_single (dev_object_t *dev, MAD_DEV *maddev, MAD_LPORT port_num)
{
    int ret = PASSED;

    if (dev_88e1680_phy_register_tests(dev, maddev, port_num, 0, &marvell_88e1680_reg_page0[0]) == FAILED) {
        cterr('f',0, "PHY Reg Test Failed");
        return (FAILED);
    }

    if (dev_88e1680_phy_register_tests(dev, maddev, port_num, 2, &marvell_88e1680_reg_page2[0]) == FAILED) {
        cterr('f',0, "PHY Reg Test Failed");
        return (FAILED);
    }

    if (dev_88e1680_phy_register_tests(dev, maddev, port_num, 3, &marvell_88e1680_reg_page3[0]) == FAILED) {
        cterr('f',0, "PHY Reg Test Failed");
        return (FAILED);
    }

    if (dev_88e1680_phy_register_tests(dev, maddev, port_num, 4, &marvell_88e1680_reg_page4[0]) == FAILED) {
        cterr('f',0, "PHY Reg Test Failed");
        return (FAILED);
    }

    if (dev_88e1680_phy_register_tests(dev, maddev, port_num, 5, &marvell_88e1680_reg_page5[0]) == FAILED) {
        cterr('f',0, "PHY Reg Test Failed");
        return (FAILED);
    }       

    if (dev_88e1680_phy_register_tests(dev, maddev, port_num, 6, &marvell_88e1680_reg_page6[0]) == FAILED) {
        cterr('f',0, "PHY Reg Test Failed");
        return (FAILED);
    }

    if (dev_88e1680_phy_register_tests(dev, maddev, port_num, 7, &marvell_88e1680_reg_page7[0]) == FAILED) {
        cterr('f',0, "PHY Reg Test Failed");
        return (FAILED);
    }

    if (dev_88e1680_phy_register_tests(dev, maddev, port_num, 8, &marvell_88e1680_reg_page8[0]) == FAILED) {
        cterr('f',0, "PHY Reg Test Failed");
        return (FAILED);
    }   

    if (dev_88e1680_phy_register_tests(dev, maddev, port_num, 9, &marvell_88e1680_reg_page9[0]) == FAILED) {
        cterr('f',0, "PHY Reg Test Failed");
        return (FAILED);
    }   

    if (dev_88e1680_phy_register_tests(dev, maddev, port_num, 12, &marvell_88e1680_reg_page12[0]) == FAILED) {
        cterr('f',0, "PHY Reg Test Failed");
        return (FAILED);
    }       

    if (dev_88e1680_phy_register_tests(dev, maddev, port_num, 14, &marvell_88e1680_reg_page14[0]) == FAILED) {
        cterr('f',0, "PHY Reg Test Failed");
        return (FAILED);
    }   

    return (ret);
}


/**********************************************************************
 *
 * Function: dev_88e1680_phy_reg_test
 *
 * This function: Marvell 88E1680 PHY register test 
 *
 * Input : dev - dev_object_t pointer to the 88e1680 device
 *            maddev - MAD_DEV pointer to the 88e1680 device
 *            port_num - port number
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int 
dev_88e1680_phy_reg_test (dev_object_t *dev, MAD_DEV *maddev, uint port_num)
{
	
    assert(maddev);

    if (dev_88e1680_phy_reg_test_single(dev, maddev, port_num) == FAILED) {
        return (FAILED);
    }
    return (PASSED);
}


/**********************************************************************
 *
 * Function: dev_88e1680_phy_detect_phone
 *
 * This function: Marvell 88E1680 PHY detcet phone
 *
 * Input : dev - dev_object_t pointer to the 88e1680 device
 *            maddev - MAD_DEV pointer to the 88e1680 device
 *            port_num - port number 
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int 
dev_88e1680_phy_detect_phone (dev_object_t *dev, MAD_DEV *maddev, int port)
{
  
    uint ix, rc, detected;
    MAD_U32 reg_d;

    dev_88e1680_object_t *obj_88e1680 = (dev_88e1680_object_t *) dev;
    dev_88e1680_callout_fvt_t *callout_p = obj_88e1680->callout_fvt;

    assert(maddev);

    /* power up copper interface on PHY device. */
    if((*callout_p->phy_mad_set_phy_enable)(maddev, port, MAD_TRUE) != MAD_OK) {
	cterr('f',0,"Failed to Enable Phy (port %i)\n",port);
	return FAILED;
    }

    /* Disable Auto Negotiation */
    rc = (*callout_p->phy_read_reg)(maddev, port, 0, PHY_CONTROL_REG, &reg_d);
    if (rc != PASSED) {
        cterr('f',0, "phy smi read failed. port = %#x"
	      "page = %#x, reg = %#x, rc = %#x\n",
	      port, 0, PHY_CONTROL_REG, rc);
        return (FAILED);
    }

    if ((reg_d & PHY_AUTO_NEO_ENA) == PHY_AUTO_NEO_ENA) {
        reg_d &= ~PHY_AUTO_NEO_ENA;
        rc = (*callout_p->phy_write_reg)(maddev, port, 0, PHY_CONTROL_REG, reg_d);
        if (rc != PASSED) {
	    cterr('f',0, "phy smi write failed. port = %#x"
		  "page = %#x, reg = %#x, rc = %#x\n",
		  port, 0, PHY_CONTROL_REG, rc);
            return (FAILED);
        }
        
        if ((*callout_p->phy_mad_hw_page_reset)(maddev, port, 0) != MAD_OK) {
	    if (rc != PASSED) {
		cterr('f',0,"PHY soft reset failed.");
		return (FAILED);
	    }
	}
    }

    /* Disable power over Ethernet detection */
    rc = (*callout_p->phy_read_reg)(maddev, port, 0, PHY_SPECIFIC_CONTROL3_REG, &reg_d);
    if (rc != PASSED) {
	cterr('f',0, "phy smi read failed. port = %#x"
	      "page = %#x, reg = %#x, rc = %#x\n",
	      port, 0, PHY_SPECIFIC_CONTROL3_REG, rc);
        return (FAILED);
    }
    
    reg_d &= ~PHY_P0_R26_DTE_DETECT;
    rc = (*callout_p->phy_write_reg)(maddev, port, 0, PHY_SPECIFIC_CONTROL3_REG, reg_d);
    if (rc != PASSED) {
	cterr('f',0, "phy smi write failed. port = %#x"
	      "page = %#x, reg = %#x, rc = %#x\n",
	      port, 0, PHY_SPECIFIC_CONTROL3_REG, rc);
        return (FAILED);
    }

    /* Set DTE power status drop to 5 seconds */
    rc = (*callout_p->phy_read_reg)(maddev, port, 0, PHY_SPECIFIC_CONTROL3_REG, &reg_d);
    if (rc != PASSED) {
	cterr('f',0, "phy smi read failed. port = %#x"
	      "page = %#x, reg = %#x, rc = %#x\n",
	      port, 0, PHY_SPECIFIC_CONTROL3_REG, rc);
        return (FAILED);
    }

    reg_d &= ~PHY_P0_R26_DTE_STATUS_DROP_MSK;
    reg_d |= PHY_P0_R26_DTE_STATUS_DROP_5S;
    rc = (*callout_p->phy_write_reg)(maddev, port, 0, PHY_SPECIFIC_CONTROL3_REG, reg_d);
    if (rc != PASSED) {
	cterr('f',0, "phy smi write failed. port = %#x"
	      "page = %#x, reg = %#x, rc = %#x\n",
	      port, 0, PHY_SPECIFIC_CONTROL3_REG, rc);
        return (FAILED);
    }

    /* Enable power over Ethernet detection bit */
    rc = (*callout_p->phy_read_reg)(maddev, port, 0, PHY_SPECIFIC_CONTROL3_REG, &reg_d);
    if (rc != PASSED) {
	cterr('f',0, "phy smi read failed. port = %#x"
	      "page = %#x, reg = %#x, rc = %#x\n",
	      port, 0, PHY_SPECIFIC_CONTROL3_REG, rc);
        return (FAILED);
    }
    
    reg_d |= PHY_P0_R26_DTE_DETECT;
    rc = (*callout_p->phy_write_reg)(maddev, port, 0, PHY_SPECIFIC_CONTROL3_REG, reg_d);
    if (rc != PASSED) {
	cterr('f',0, "phy smi write failed. port = %#x"
	      "page = %#x, reg = %#x, rc = %#x\n",
	      port, 0, PHY_SPECIFIC_CONTROL3_REG, rc);
        return (FAILED);
    }

    /* enable Auto Negotiation and reset phy */
    rc = (*callout_p->phy_read_reg)(maddev, port, 0, PHY_CONTROL_REG, &reg_d);
    if (rc != PASSED) {
        cterr('f',0, "phy smi read failed. port = %#x"
	      "page = %#x, reg = %#x, rc = %#x\n",
	      port, 0, PHY_CONTROL_REG, rc);
        return (FAILED);
    }

    reg_d |= PHY_AUTO_NEO_ENA;
    rc = (*callout_p->phy_write_reg)(maddev, port, 0, PHY_CONTROL_REG, reg_d);
    if (rc != PASSED) {
	cterr('f',0, "phy smi write failed. port = %#x"
	      "page = %#x, reg = %#x, rc = %#x\n",
	      port, 0, PHY_CONTROL_REG, rc);
	return (FAILED);
    }
        
    if ((*callout_p->phy_mad_hw_page_reset)(maddev, port,0) != MAD_OK) {
        if (rc != PASSED) {
	    cterr('f',0,"PHY soft reset failed.");
            return (FAILED);
        }
    }
    
    for (ix = MRVL_PHONE_DETECT_TIME; ix; ix--) {
        printf("\r%d seconds left", ix);
        msleep(1000);
    }

    /* read detection status register */
    rc = (*callout_p->phy_read_reg)(maddev, port, 0, PHY_SPECIFIC_STATUS1_REG, &detected);
    if (rc != PASSED) {
        cterr('f',0, "phy smi read failed. port = %#x"
	      "page = %#x, reg = %#x, rc = %#x\n",
	      port, 0, PHY_CONTROL_REG, rc);
        return (FAILED);
    }

    /* Disable power over Ethernet detection after detection */
    rc = (*callout_p->phy_read_reg)(maddev, port, 0, PHY_SPECIFIC_CONTROL3_REG, &reg_d);
    if (rc != PASSED) {
	cterr('f',0, "phy smi read failed. port = %#x"
	      "page = %#x, reg = %#x, rc = %#x\n",
	      port, 0, PHY_SPECIFIC_CONTROL3_REG, rc);
        return (FAILED);
    }
    
    reg_d &= ~PHY_P0_R26_DTE_DETECT;
    rc = (*callout_p->phy_write_reg)(maddev, port, 0, PHY_SPECIFIC_CONTROL3_REG, reg_d);
    if (rc != PASSED) {
	cterr('f',0, "phy smi write failed. port = %#x"
	      "page = %#x, reg = %#x, rc = %#x\n",
	      port, 0, PHY_SPECIFIC_CONTROL3_REG, rc);
        return (FAILED);
    }
    
    /* Check detection result */
    if (detected & PHY_P0_R17_DTE_NEED_POWER) {
        return (PASSED);    /* Found Cisco PD */
    } else {
        return (FAILED);    /* PD not detected */
    }
}


/**********************************************************************
 *
 * Function: dev_88e1680_dump_phy_reg
 *
 * This function: Marvell 88E1680 dump PHY register
 *
 * Input : dev - dev_object_t pointer to the 88e1680 device
 *            maddev - MAD_DEV pointer to the 88e1680 device
 *            port - port number  
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
dev_88e1680_dump_phy_reg (dev_object_t *dev, MAD_DEV *maddev, int port)
{

    dev_88e1680_object_t *obj_88e1680 = (dev_88e1680_object_t *) dev;
    dev_88e1680_callout_fvt_t *callout_p = obj_88e1680->callout_fvt;

    (*callout_p->phy_mad_display_reg)(maddev, port, 0);
    (*callout_p->phy_mad_display_reg)(maddev, port, 2);
    (*callout_p->phy_mad_display_reg)(maddev, port, 4);
    (*callout_p->phy_mad_display_reg)(maddev, port, 18);

    return (PASSED);
}


/**********************************************************************
 *
 * Function: dev_88e1680_reset_phy
 *
 * This function: Marvell 88E1680 reset PHY
 *
 * Input : dev - dev_object_t pointer to the 88e1680 device
 *            maddev - MAD_DEV pointer to the 88e1680 device
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int 
dev_88e1680_reset_phy(dev_object_t *dev, MAD_DEV *maddev)
{
    unsigned char port;
    //MAD_DEV *dev = &phy_dev[get_slot_num()-1];
    int rc;

    dev_88e1680_object_t *obj_88e1680 = (dev_88e1680_object_t *) dev;
    dev_88e1680_callout_fvt_t *callout_p = obj_88e1680->callout_fvt;

    /* put PHY in reset */
    rc = (*callout_p->phy_reset_api)(PHY_RESET);
    if (rc != PASSED) {
	return (FAILED);
    }
    msleep(500);

    /* take PHY out of reset */
    rc = (*callout_p->phy_reset_api)(PHY_UNRESET);
    if (rc != PASSED) {
	return (FAILED);
    }
    msleep(500);

    /* disable all the interrupts */
    if (((*callout_p->phy_mad_disable_int)(maddev)) == FAILED) {
	cterr('f',0,"Failed to disable PHY interrupts.");
	return FAILED;
    }

    /* init all PHYs */
    for(port = 0; port < maddev->numOfPorts; port++) {
	if (dev_88e1680_phy_init(dev, maddev, port) == FAILED)
	    return (FAILED);

	if (dev_88e1680_phy_config(dev, maddev, port) == FAILED)
	    return (FAILED);
    }

    /* enable all PHYs */
    for(port=0; port < maddev->numOfPorts; port++) {
        if((*callout_p->phy_mad_set_phy_enable)(maddev, port, MAD_TRUE) != PASSED) {
            cterr('f',0,"Failed to Enable Phy (port %i)\n",port);
            return FAILED;
        }
    }

    return PASSED;
}



/**********************************************************************
 *
 * Function: dev_88e1680_read_phy_reg_util
 *
 * This function: Marvell 88E1680 read PHY register utility
 *
 * Input : dev - dev_object_t pointer to the 88e1680 device
 *            maddev - MAD_DEV pointer to the 88e1680 device
 *            port_num - target port number
 *            page_num - target page number
 *            reg_num - target register number
 *            * data - pointer for read back data
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int 
dev_88e1680_read_phy_reg_util (dev_object_t *dev, MAD_DEV *maddev, uint port_num, uint page_num, uint reg_num, uint * data)
{

    dev_88e1680_object_t *obj_88e1680 = (dev_88e1680_object_t *) dev;
    dev_88e1680_callout_fvt_t *callout_p = obj_88e1680->callout_fvt;

    assert(maddev);
   
    return((*callout_p->phy_read_reg)(maddev, port_num, page_num, reg_num, data));
    
}


#if 0
/**********************************************************************
 *
 * Function: dev_88e1680_read_phy_reg_util
 *
 * This function: Marvell 88E1680 read PHY register utility
 *
 * Input : dev - dev_object_t pointer to the 88e1680 device
 *            maddev - MAD_DEV pointer to the 88e1680 device
 *            total_port - Total of existing ports.
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int 
dev_88e1680_read_phy_reg_util (dev_object_t *dev, MAD_DEV *maddev, int total_port)
{

    //MAD_LPORT port_num;
    //MAD_U16 page_num;
    //MAD_U16 reg_num;
    MAD_U32 data;
    int ret;

    dev_88e1680_object_t *obj_88e1680 = (dev_88e1680_object_t *) dev;
    dev_88e1680_callout_fvt_t *callout_p = obj_88e1680->callout_fvt;

    assert(maddev);
   
    //port_num = getdec_answer("Enter PHY port number: ", 0, 0, total_port);
    //page_num = getdec_answer("Enter PHY page number: ", 0, 0, 18);
    //reg_num = getdec_answer("Enter PHY register number: ", 0, 0, 31);
    ret = (*callout_p->phy_read_reg)(maddev, port_num, page_num, reg_num, &data);

    //if (ret == PASSED)
    printf("PHY register value @ offset %d = %#x\n", reg_num, data&0xffff);
    return (ret);
	
}
#endif

#if 1
/**********************************************************************
 *
 * Function: dev_88e1680_write_phy_reg_util
 *
 * This function: Marvell 88E1680 write PHY register utility
 *
 * Input : dev - dev_object_t pointer to the 88e1680 device
 *            maddev - MAD_DEV pointer to the 88e1680 device
 *            port_num - target port number
 *            page_num - target page number
 *            reg_num - target register number
 *            data - write-in data
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int 
dev_88e1680_write_phy_reg_util (dev_object_t *dev, MAD_DEV *maddev, uint port_num, uint page_num, uint reg_num, uint data)
{

    dev_88e1680_object_t *obj_88e1680 = (dev_88e1680_object_t *) dev;
    dev_88e1680_callout_fvt_t *callout_p = obj_88e1680->callout_fvt;

    assert(maddev);
   
    return ((*callout_p->phy_write_reg)(maddev, port_num, page_num, reg_num, data));
}
#endif

#if 0
/**********************************************************************
 *
 * Function: dev_88e1680_write_phy_reg_util
 *
 * This function: Marvell 88E1680 write PHY register utility
 *
 * Input : dev - dev_object_t pointer to the 88e1680 device
 *            maddev - MAD_DEV pointer to the 88e1680 device
 *            total_port - Total of existing ports.
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int 
dev_88e1680_write_phy_reg_util (dev_object_t *dev, MAD_DEV *maddev, int total_port)
{
    MAD_LPORT port_num;
    MAD_U16 page_num;
    MAD_U16 reg_num;
    MAD_U16 data;

    dev_88e1680_object_t *obj_88e1680 = (dev_88e1680_object_t *) dev;
    dev_88e1680_callout_fvt_t *callout_p = obj_88e1680->callout_fvt;

    assert(maddev);
   
    port_num = getdec_answer("Enter PHY port number: ", 0, 0, total_port);
    page_num = getdec_answer("Enter PHY page number: ", 0, 0, 18);
    reg_num = getdec_answer("Enter PHY register number: ", 0, 0, 31);
    data = gethex_answer("Enter write data: ", 0, 0, 0xffff);
    return ((*callout_p->phy_write_reg)(maddev, port_num, page_num, reg_num, data));
}
#endif

/**********************************************************************
 *
 * Function: dev_88e1680_phy_intr
 *
 * This function: Marvell 88E1680 write PHY register utility
 *
 * Input : dev - dev_object_t pointer to the 88e1680 device
 *            maddev - MAD_DEV pointer to the 88e1680 device
 *            test_port - Target of test port.
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int 
dev_88e1680_phy_intr_test (dev_object_t *dev, MAD_DEV *maddev, int test_port)
{

    uint16_t data; 
    uint32_t rd_data;
    int ret, ix;

    dev_88e1680_object_t *obj_88e1680 = (dev_88e1680_object_t *) dev;
    dev_88e1680_callout_fvt_t *callout_p = obj_88e1680->callout_fvt;

    assert(maddev);

    //To-do call-out function to read FPGA interrupt status register.

    /* read PHY intr status register to clear all the pending bits */
    ret = (*callout_p->phy_read_reg)(maddev, test_port, 4, PHY_QSGMII_INTR_STATUS_REG, &rd_data);

    /* enable PHY QSGMII Link Status Changed interrupt (page 4, register 19, bit 10) */
    data = PHY_QSGMII_LINK_STATUS_CHANGED;
    ret = (*callout_p->phy_write_reg)(maddev, test_port, 4, PHY_QSGMII_INTR_ENA_REG, data);
    if (ret != PASSED) {
	cterr('f',0, "phy smi write failed. port = %#x"
	      "page = %#x, reg = %#x, status = %#x\n",
	      test_port, 4, PHY_QSGMII_INTR_ENA_REG, ret);
        return (FAILED);
    }

    //To-do call-out function to access FPGA interrupt status register and enable interrupt

    /* start and stop PHY MAC loopback will trigger QSGMII Link Status Changed intr */
    if (dev_88e1680_start_mac_lpbk(dev, maddev, test_port, MAD_SPEED_1000M)) {
	cterr('f',0,"Failed to enable PHY loopback for port %d", test_port);
	return (FAILED);
    }

    msleep(1000);

    if (dev_88e1680_phy_config(dev, maddev, test_port) == FAILED){
	 cterr('f',0,"Failed to do PHY config for port %d", test_port);
        return (FAILED);
    }

    msleep(1000);

    /* read PHY global intr status register (page 0, reg 23) */
    ret = (*callout_p->phy_read_reg)(maddev, test_port, 0, PHY_GLOBAL_INTR_STATUS_REG, &rd_data);
    if ((rd_data & (1 << test_port)) != (1 << test_port)) {
	cterr('f',0,"PHY interrupt is not active on port %d, PHY_GLOBAL_INTR_STATUS = %#x", 
	      test_port, rd_data);
	return (FAILED);
    }

    for (ix = 0; ix < PHY_INTR_DELAY; ix++) {

       //To-do call-out function to read FPGA interrupt status register.
	msleep(20);
    }

    /* read status bit to clear it */
    ret = (*callout_p->phy_read_reg)(maddev, test_port, 4, PHY_QSGMII_INTR_STATUS_REG, &rd_data);
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("QSGMII interrupt status register = %#x\n", rd_data&0xffff);
        fflush(0);
    }

    //To-do call-out function to disable FPGA interrupt register.

    /* disable QSGMII interrupts */
    data = 0;
    ret = (*callout_p->phy_write_reg)(maddev, test_port, 4, PHY_QSGMII_INTR_ENA_REG, data);
    if (ret != PASSED) {
	cterr('f',0, "phy smi write failed. port = %#x"
	      "page = %#x, reg = %#x, status = %#x\n",
	      test_port, 4, PHY_QSGMII_INTR_ENA_REG, ret);
        return(FAILED);
    }
   
    return (PASSED);
	
}


/**********************************************************************
 *
 * Function: dev_88e1680_led_on
 *
 * This function: Marvell 88E1680 force led on
 *
 * Input : dev - dev_object_t pointer to the 88e1680 device
 *            maddev - MAD_DEV pointer to the 88e1680 device
 *            port_num - target port number
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int 
dev_88e1680_led_on (dev_object_t *dev, MAD_DEV *maddev, uint port_num)
{

    uint page_num, reg_num; 
    uint16_t data;

    dev_88e1680_object_t *obj_88e1680 = (dev_88e1680_object_t *) dev;
    dev_88e1680_callout_fvt_t *callout_p = obj_88e1680->callout_fvt;

    assert(maddev);

    page_num = E1680_REG_PAGE3;
    reg_num = PHY88E1680_LED_FUNC_CTRL_REG;
    data = (E1680_LCR_LED1_F_ON) | (E1680_LCR_LED0_F_ON);
   
    return ((*callout_p->phy_write_reg)(maddev, port_num, page_num, reg_num, data));
    
}


/**********************************************************************
 *
 * Function: dev_88e1680_led_off
 *
 * This function: Marvell 88E1680 force led off
 *
 * Input : dev - dev_object_t pointer to the 88e1680 device
 *            maddev - MAD_DEV pointer to the 88e1680 device
 *            port_num - target port number
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int 
dev_88e1680_led_off (dev_object_t *dev, MAD_DEV *maddev, uint port_num)
{

    uint page_num, reg_num; 
    uint16_t data;

    dev_88e1680_object_t *obj_88e1680 = (dev_88e1680_object_t *) dev;
    dev_88e1680_callout_fvt_t *callout_p = obj_88e1680->callout_fvt;

    assert(maddev);

    page_num = E1680_REG_PAGE3;
    reg_num = PHY88E1680_LED_FUNC_CTRL_REG;
    data = (E1680_LCR_LED1_F_OFF) | (E1680_LCR_LED0_F_OFF);
   
    return ((*callout_p->phy_write_reg)(maddev, port_num, page_num, reg_num, data));
    
}


/**********************************************************************
 *
 * Function: dev_88e1680_led_default
 *
 * This function: Marvell 88E1680 write led default pattern
 *
 * Input : dev - dev_object_t pointer to the 88e1680 device
 *            maddev - MAD_DEV pointer to the 88e1680 device
 *            port_num - target port number
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int 
dev_88e1680_led_default (dev_object_t *dev, MAD_DEV *maddev, uint port_num)
{

    uint page_num, reg_num; 
    uint16_t data;

    dev_88e1680_object_t *obj_88e1680 = (dev_88e1680_object_t *) dev;
    dev_88e1680_callout_fvt_t *callout_p = obj_88e1680->callout_fvt;

    assert(maddev);

    page_num = E1680_REG_PAGE3;
    reg_num = PHY88E1680_LED_FUNC_CTRL_REG;
    data = (E1680_LCR_LED1_DEFAULT) | (E1680_LCR_LED0_DEFAULT);
   
    return ((*callout_p->phy_write_reg)(maddev, port_num, page_num, reg_num, data));
    
}

/**********************************************************************
 *
 * Function: dev_88e1680_enable_force_interrupt
 *
 * This function: Marvell 88E1680 force interrupt
 *
 * Input : dev - dev_object_t pointer to the 88e1680 device
 *            maddev - MAD_DEV pointer to the 88e1680 device
 *            port_num - target port number
 *            enable - enable / disable
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int 
dev_88e1680_enable_force_interrupt (dev_object_t *dev, MAD_DEV *maddev, uint port_num, uint enable)
{

    uint page_num, reg_num; 
    uint rc; 
    uint data, bit_mask;

    dev_88e1680_object_t *obj_88e1680 = (dev_88e1680_object_t *) dev;
    dev_88e1680_callout_fvt_t *callout_p = obj_88e1680->callout_fvt;

    assert(maddev);

    page_num = MRV88E1680_REG_PAGE_3;
    reg_num = MRV88E1680_TMR_CONTROL_REG;
    rc = (*callout_p->phy_read_reg)(maddev, port_num, page_num, reg_num, &data);

    bit_mask = PHY_TIMER_CNTRL_FORCE_INT;

    if (enable == DEV_88E1680_ENABLE) {
        data |= bit_mask;
    } else {
        data &= ~bit_mask;
    }
   
    return ((*callout_p->phy_write_reg)(maddev, port_num, page_num, reg_num, data));
    
}


/**********************************************************************
 *
 * Function: dev_88e1680_gen_int
 *
 * This function: Marvell 88E1680 generate interrupt
 *
 * Input : dev - dev_object_t pointer to the 88e1680 device
 *            maddev - MAD_DEV pointer to the 88e1680 device
 *            port_num - target port number
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int 
dev_88e1680_gen_int (dev_object_t *dev, MAD_DEV *maddev, uint port_num)
{

    uint rc; 

    rc = dev_88e1680_enable_force_interrupt(dev, maddev, port_num, DEV_88E1680_ENABLE);
    return rc;
    
}


/**********************************************************************
 *
 * Function: dev_88e1680_clear_int
 *
 * This function: Marvell 88E1680 clear interrupt
 *
 * Input : dev - dev_object_t pointer to the 88e1680 device
 *            maddev - MAD_DEV pointer to the 88e1680 device
 *            port_num - target port number
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int 
dev_88e1680_clear_int (dev_object_t *dev, MAD_DEV *maddev, uint port_num)
{

    uint rc; 

    rc = dev_88e1680_enable_force_interrupt(dev, maddev, port_num, DEV_88E1680_DISABLE);
    return rc;
    
}


/**********************************************************************
 *
 * Function: dev_88e1680_set_test_mode
 *
 * This function: Marvell 88E1680 set test mode
 *
 * Input : dev - dev_object_t pointer to the 88e1680 device
 *            maddev - MAD_DEV pointer to the 88e1680 device
 *            port_num - target port number
 *            test_mode - target test mode
 *
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int 
dev_88e1680_set_test_mode (dev_object_t *dev, MAD_DEV *maddev, uint port_num, uint test_mode)
{

    dev_88e1680_object_t *obj_88e1680 = (dev_88e1680_object_t *) dev;
    dev_88e1680_callout_fvt_t *callout_p = obj_88e1680->callout_fvt;

    assert(maddev);
   
    return ((*callout_p->phy_mad_set_test_mode)(maddev, port_num, test_mode));
    
}


/******** History ******** 
 *
 *$Endlog$
*/