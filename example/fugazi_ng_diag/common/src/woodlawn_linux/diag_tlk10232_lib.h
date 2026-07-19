/* $Id: diag_tlk10232_lib.h,v 1.5 2017/09/27 01:56:30 leschen Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn_linux/diag_tlk10232_lib.h,v $
 *-----------------------------------------------------------------------------
 * diag_TLK10232_lib.h
 *
 * January 2013, Leslie Chen
 * Copyright (c) 2017 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */
 
#ifndef __DIAG_TLK10232_LIB_H__
#define __DIAG_TLK10232_LIB_H__

#define TLK_10232_SMI2_ADDR         (0x2)
#define TLK_10232_REG_LEN       (2)
#define TLK_10232_REG_DEVICE_30       (0x1E)

#define REG_TEST_TLK_10232_SMI2_ADDR   (TLK_10232_PHY_ADDR_CHANNEL_A | \
                                        (TLK_10232_SMI2_ADDR << 4))
                                        
/* TLK 10232 phy address = 0x00000, select channel A by setting LSB of phy address = 0, 
     channel B by setting LSB of phy address = 1 */
#define TLK_10232_PHY_ADDR_CHANNEL_A         (0x00000)
#define TLK_10232_PHY_ADDR_CHANNEL_B         (0x00001)

/* TLK 10232 Device Address */
#define TLK_10232_HS_CH_CTRL_1_DEV            (0x1E)
#define TLK_10232_CHANNEL_CTRL_1_DEV        (0x1E)
#define TLK_10232_HS_SERDES_CTRL_1_DEV    (0x1E)
#define TLK_10232_AN_CTRL                              (0x07)
#define TLK_10232_LT_TRAIN_CTRL                   (0x01)
#define TLK_10232_TI_RESERVED_CTRL             (0x1E)
#define TLK_10232_RESET_CTRL                         (0x1E)
#define TLK_10232_DSR_CTRL_1                         (0x1E)
#define TLK_10232_LPBK_TP_CTRL                      (0x1E)

/* TLK 10232 Register Address */
#define TLK_10232_HS_CH_CTRL_1_REG                     (0x001D)
#define TLK_10232_DSR_CHANNEL_CTRL_1_REG         (0x0019)
#define TLK_10232_HS_SERDES_CTRL_1_REG             (0x0002)
#define TLK_10232_DST_CONTROL_1_REG                   (0x0017)
#define TLK_10232_DST_CONTROL_2_REG                   (0x0018)
#define TLK_10232_DSR_CONTROL_2_REG                   (0x001A)
#define TLK_10232_GLOBAL_CTRL_REG                        (0x0000)
#define TLK_10232_AN_CTRL_REG                                (0x0000)
#define TLK_10232_LT_TRAIN_CTRL_REG                    (0x0096)
#define TLK_10232_TI_RESERVED_CTRL_REG              (0x8021)
#define TLK_10232_RESET_CTRL_REG                          (0x000E)
#define TLK_10232_DSR_CTRL_1_REG                          (0x0019)
#define TLK_10232_LPBK_TP_REG                                (0x000B)

/* Direct set up value */
#define TLK_10232_HS_CH_CTRL_1_VAL                                       (0x2000)
#define TLK_10232_DSR_CHANNEL_CTRL_1_VAL                           (0x0300)
#define TLK_10232_HS_SERDES_CTRL_1_VAL                               (0x8317)
#define TLK_10232_GLOBAL_CTRL_VAL                                          (0x8610)
#define TLK_10232_GLOBAL_RESET                                                (0x8000)
#define TLK_10232_AN_CTRL_VAL                                                  (0x2000)
#define TLK_10232_AN_DISABLE                                                    (0x1000)
#define TLK_10232_LT_TRAIN_CTRL_VAL                                       (0x0000)
#define TLK_10232_LINK_TRAIN_DISABLE                                     (0x0003)
#define TLK_10232_TI_RESERVED_CTRL_VAL                                 (0x003f)
#define TLK_10232_HS_AUTO_STATUS_CHECK_DISABLE               (0x0030)
#define TLK_10232_TI_RESERVED_CTRL_VAL_KX                           (0x001f)
#define TLK_10232_PATH_RESET                                                    (0x0008)
#define TLK_10232_XAUIB_TO_XAUIB_DSR_CTRL_1_VAL               (0x2000)
#define TLK_10232_XAUIB_TO_XAUIA_DSR_CTRL_1_VAL               (0x2a00)
#define TLK_10232_DST_CTRL_2_VAL                                             (0x4C20)
#define TLK_10232_LPBK_TP_VAL                                                    (0x0008)
#define TLK_10232_SET_LPBK                                                    (0x0001)
#define TLK_10232_CLEAR_LPBK                                                  (0x0)

/* mask value */
#define TLK_10232_DSR_CONTROL_2_MASK_VAL_DEFAULT      (0xbfff)
#define TLK_10232_DSR_CONTROL_2_MASK_VAL_1            (0xf000)
#define TLK_10232_DSR_CONTROL_2_MASK_VAL_2            (0xf000)
#define TLK_10232_DSR_CONTROL_3_MASK_VAL_3_CHA    (0x5fff)
#define TLK_10232_DSR_CONTROL_3_MASK_VAL_3_CHB    (0x5fff)
/* Selects condition to trigger any data switch */
#define TLK_10232_DSR_ANY_DATA (0x2000)

/* default value */
#define DSR_CONTROL_2_REG_DEFAULT_VAL    (0x4C20)

enum tlk10232 {
  XAUIB_TO_10GKR = 1,
  XAUIB_TO_XAUIB,
  XAUIB_TO_XAUIA,
  XAUIB_TO_1GKX,
};

#define WAIT_AN_COMPLETE   (30)
#define PATH_RESET_TIME    (1000)
/* Declare Extern Functions */
extern int config_tlk_10232_mode(int);
extern int read_tlk_10232_reg(ulong, int, ulong *, void *);
extern int write_tlk_10232_reg(ulong, int, ulong, void *);
extern int tlk10232_xaui_to_xaui_configuration(void);
extern int set_tlk10232_lpbk_bit(int);
extern int tlk10232_mode_select(void);
extern int tlk10232_global_reset(void);
extern int is_10gkr_capable(void);
extern void run_tlk10232_script(void);
int tlk10232_path_reset(void);
#endif
/*-------------------------------------------------
 * $Log: diag_tlk10232_lib.h,v $
 * Revision 1.5  2017/09/27 01:56:30  leschen
 * CSCvd81389 - No need to execute TLK10232 init script.
 *
 * Revision 1.4  2015/03/31 07:33:32  leschen
 * Fix for KR.
 *
 * Revision 1.3  2014/11/12 06:29:14  leschen
 * Support Greyhound tlk10232 10gkr
 *
 * Revision 1.2  2013/10/08 08:48:29  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.4.2  2013/08/20 10:58:54  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.1  2013/04/24 10:37:19  tirawan
 * Initial check-in for woodlawn linux code
 *
 * Revision 1.3  2013/04/12 04:48:36  leslie
 * Fix and clean up code
 *
 * Revision 1.2  2013/04/09 11:03:11  leslie
 * Add TLK10232 deep remote lpbk macros
 *
 * Revision 1.1  2013/03/13 06:42:54  kuangik
 * Add for the first time
 *
 * Revision 1.3  2013/03/07 12:43:15  leslie
 * Add TLK10232 macros.
 *
 * Revision 1.2  2013/01/16 00:59:45  leslie
 * Add
 *
 * $Endlog$
 *-------------------------------------------------
 */
