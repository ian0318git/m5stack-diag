/* $Id: ngio.h,v 1.4 2020/01/09 01:02:38 jiajliu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/ngio.h,v $
 *------------------------------------------------------------------
 *
 * ngio.h - Header file for NGIO
 *
 * June 2015, Times Huang ported from Overlord
 *
 * Copyright (c) 2015-2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#ifndef __NGIO_H__
#define __NGIO_H__

#define ENABLE_DELAY            (600)
#define FIRST_SLOT              (1)
#define I2C_UNRESET_DELAY       (50)

#define MAX_SM                  (0)
#define MAX_WIC                 (3)
#define MAX_WIC_TACHI_L         (1)
#define MAX_WIC_TACHI_H         (3)
#define MAX_VM                  (0)
#define MAX_DC                  (1)

/* tachi-L: nim1 only; tachi-H: nim1-nim3 */
enum {
    NIM1 = 1,
    NIM2 = 2,
    NIM3 = 3,
};

enum {
    NGIOSM1 = 1,
    NGIOSM2 = 2,
};

enum {
    NGIOWIC1 = 1,
    NGIOWIC2 = 2,
    NGIOWIC3 = 3,
};

extern int ngiowic_present(void *);
extern int is_ngiowic_i2c_unreset(void *);
extern int ngiowic_reset(void *);
extern int ngiowic_unreset(void *);
extern int ngiowic_i2c_reset(void *); 
extern int ngiowic_i2c_unreset(void *);
extern int ngioisp_i2c_reset(void *); 
extern int ngioisp_i2c_unreset(void *);
extern int ngiowic_enable(void *);
extern void ngiowic_disable(void *);
extern void ngiowic_enable_intr (int, int);
extern void ngiowic_disable_intr(int, int);
extern int ngiowic_enable_uart(void *);
extern int ngiowic_disable_uart(void *);
extern int ngiodc_present(void *);
extern int ngiodc_reset(void *);
extern int ngiodc_unreset(void *);
extern int ngiodc_i2c_reset(void *);
extern int ngiodc_i2c_unreset(void *);
extern int ngiodc_enable(void *);
extern void ngiodc_disable(void *);
extern void ngiodc_enable_intr (int, int);
extern void ngiodc_disable_intr(int, int);
extern int ngiodc_enable_uart(void *);
extern int ngiodc_disable_uart(void *);
extern void oir_wic1_intr_hndlr(int, void *);
extern void clr_all_oir_intr(void);
extern void ngiowic_pci_rdy(void *, int);
extern int ngio_sync_out_enable(void *, int);
extern int ngio_sync_out_disable(void *, int);

/* although there is no vm and sm on tachi 
 * we leave these prototypes for tam_act2_utils.c 
 * for bypass compile warning 
 */
extern int ngiovm_unreset(void *);
extern int ngiovm_reset(void *);
extern int ngiovm_i2c_unreset(void *);
extern int ngiovm_i2c_reset(void *);
extern int ngiosm_reset(void *);
extern int ngiosm_unreset(void *);
extern int ngiosm_i2c_reset(void *);
extern int ngiosm_i2c_unreset(void *);

typedef enum {
    NGIO_ETH_SPEED_10M  = 1,
    NGIO_ETH_SPEED_100M = 2,
    NGIO_ETH_SPEED_1G   = 3,
    NGIO_ETH_SPEED_10G  = 4,
    NGIO_ETH_SPEED_UNKNOWN = 0xFF,
} ngio_eth_speed_t;

extern int ngio_cfg_eth_port_speed(uint mod_type, uint slot,
                        const ngio_eth_speed_t *new_speed, ngio_eth_speed_t *old_speed);

#endif /* __NGIO_H__ */

/*---------------------------------------------------------------
$Log: ngio.h,v $
Revision 1.4  2020/01/09 01:02:38  jiajliu
Merge Curie 2RU to main trunk

Revision 1.3  2017/03/30 08:34:08  hondwang
Tachi-L brach merge

Revision 1.2.14.1  2017/01/03 00:56:33  haohsu
Add NIM to Tachi-l

Revision 1.2  2016/04/20 11:25:28  benchen2
add tachi fru portion

Revision 1.1.2.6  2016/04/18 07:00:47  benchen2
according to prrq fix isp define

Revision 1.1.2.5  2015/12/22 17:26:03  huanngo
Support Tachi-High tests and menus CSCux15587

Revision 1.1.2.4  2015/07/28 09:32:38  alpeng
adding entry for ngio test on platform_slot.c

Revision 1.1.2.3  2015/07/26 06:02:22  tirawan
Add FPGA I2C read/write function and ACT2 cookie/smartchip programming

Revision 1.1.2.2  2015/07/24 06:59:58  alpeng
Add ngio.c to support NIM test

Revision 1.1.2.1  2015/06/11 02:01:10  tirawan
Add files for Tachi BMC project


$Endlog$
*/
