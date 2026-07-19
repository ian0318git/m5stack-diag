/* $Id: ngio.h,v 1.22 2020/01/09 01:02:19 jiajliu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/ngio.h,v $
 *------------------------------------------------------------------
 * ngio.h
 *
 * Copyright (c) 2008-2019 by Cisco Systems, Inc.
 * All rights reserved.
 * 
 *------------------------------------------------------------------
 */

/***********************************************
NGSM1 Slot offset +0x10
NGSM1 Slot offset +0x20 
NGWIC1 Slot offset +0x50 
NGWIC2 Slot offset +0x60 
NGWIC3 Slot offset +0x70.
************************************************/

#ifndef __NGIO_H__
#define __NGIO_H__

#define NGIO_BASE  0x32000
#define FIRST_SLOT 1
enum {
    NGIO_FAIL_PWR_ENABLE = -1,
    NGIO_FAIL_I2C = -2,
    NGIO_FAIL_END = 2,
};

enum {
    NGIO_I2C_ACT2,
    NGIO_I2C_OIR,
    NGIO_I2C_MAX,
};

enum {
    NGIOSM1 = 1,
    NGIOSM2 = 2,
};

#define MAX_SM   4
#define MAX_SM_O2 2
#define MAX_SM_JUNO 0
#define MAX_SM_UTAH  2
#define MAX_SM_SWORD  1
#define MAX_SM_DAGGER  0
#define MAX_SM_GOLDBEACH 0
#define MAX_SM_NEPTUNE 4
#define MAX_SM_VG450 4
#define MAX_SM_TRITON  2
#define MAX_SM_PROTEUS 2
#define MAX_SM_NESO    0
#define MAX_SM_URANIUM   2 /* All curie 2ru has 2 SM */
#define MAX_SM_RADIUM    1 /* All curie 1ru has 1 SM */

enum {
    NGIOWIC1 = 1,
    NGIOWIC2 = 2,
    NGIOWIC3 = 3,
};
enum {
    SATA_ONE = 1,
    SATA_TWO = 2,
};

typedef enum {
    NGIO_ETH_SPEED_10M  = 1,
    NGIO_ETH_SPEED_100M = 2,
    NGIO_ETH_SPEED_1G   = 3,
    NGIO_ETH_SPEED_10G  = 4,
    NGIO_ETH_SPEED_UNKNOWN = 0xFF,
} ngio_eth_speed_t;

#define MAX_WIC  3
#define MAX_WIC_JUNO 3
#define MAX_WIC_UTAH  3
#define MAX_WIC_SWORD  2
#define MAX_WIC_DAGGER 2
#define MAX_WIC_GOLDBEACH 2
#define MAX_WIC_NEPTUNE 3 
#define MAX_WIC_VG450 3 
#define MAX_WIC_TRITON  3
#define MAX_WIC_PROTEUS 3
#define MAX_WIC_NESO    3
#define MAX_WIC_URANIUM   2 /* All curie 2ru has 2 NIM */
#define MAX_WIC_RADIUM    1 /* radium and thallium has 1 NIM */
#define MAX_WIC_POLONIUM  2

#define MAX_DC   1

#define MAX_VM   1
#define MAX_VM_NEPTUNE 0 
#define MAX_VM_URANIUM 0 /* All curie 2ru has no VM */
#define MAX_VM_RADIUM  0 /* All curie 1ru has no VM */
#define MAX_VM_GOLDBEACH 0

typedef struct ng_t_ {
#define NGIO_HDLS_MODE   0x40000
#define NGIO_PWR_FLT_OVR 0x20000
#define NGIO_PWR_OK      0x10000
#define NGIO_FLT_INTR    0x400
#define NGIO_INS_INTR    0x200
#define NGIO_RMV_INTR    0x100
#define NGIO_PRSNT       0x80
#define NGIO_I2C_OK      0x40
#define NGIO_UART_TX     0x20
#define NGIO_PWR_EN      0x10
#define NGIO_SRC_SEL     8
#define NGIO_PCI_RDY     4
#define NGIO_RESET       2
#define NGIO_I2C_RESET   1    
    volatile unsigned int ctrl;

#define NGIO_FLT_INTR    0x400
#define NGIO_INS_INTR    0x200
#define NGIO_REM_INTR    0x100 
    volatile unsigned int intr;

    volatile unsigned int debounce; 
    volatile unsigned int pad;
} ng_t;

typedef struct ngio_t_ {
    volatile unsigned int general;  /* 0x00 */
#define NGSM4_LEGACY_MODE        0x8000
#define NGSM3_LEGACY_MODE        0x2000
#define NGSM2_LEGACY_MODE        0x800
#define NGSM1_LEGACY_MODE        0x200
    volatile unsigned int pad[3];   /* 0x04-0x0C */
    ng_t sm[MAX_SM];                /* 0x10-0x40*/
    ng_t wic[MAX_WIC];              /* 0x50-0x70 */
    volatile unsigned int pad2[4];  /* 0x80-0x8C */

#define NGIO_MOD_RESET           0x02

    volatile unsigned int ngvm;  /* 0x90 */
    volatile unsigned int pad3[3];   /* 0x94-0x9C */

#define NGIO_SATA2_INS            0x800
#define NGIO_SATA2_RMV            0x400
#define NGIO_SATA1_INS            0x200
#define NGIO_SATA1_RMV            0x100
#define NGIO_SATA_TWO_PRSNT       0x20
#define NGIO_SATA_ONE_PRSNT       0x10

    volatile unsigned int sata_ctrl; /* 0xA0 */
    volatile unsigned int sata_intr;  /* 0xA4*/

} ngio_t;

#define GPIO_EXP_GE0_10GKR_BIT     0x1
#define GPIO_EXP_GE1_10GKR_BIT     0x2

extern int ngiosm_present(void *);
extern int ngiosm_reset(void *);
extern int ngiosm_unreset(void *);
extern int is_ngiosm_i2c_unreset(void *);
extern int ngiosm_i2c_reset(void *);
extern int ngiosm_i2c_unreset(void *);
extern int ngiosm_enable(void *);
extern void ngiosm_disable(void *);
extern void ngiosm_enable_intr(int dev, int);
extern void ngiosm_disable_intr(int dev, int);
extern int ngiosm_enable_uart(void *);
extern int ngiosm_disable_uart(void *);
extern void ngiosm_pci_rdy(void *, int);
extern void ngiowic_pci_rdy(void *, int);
extern int ngiowic_enable(void *);
extern void ngiowic_disable(void *);
extern int ngiowic_present(void *);
extern int ngiowic_reset(void *);
extern int ngiowic_unreset(void *);
extern int ngiowic_i2c_reset(void *);
extern int ngiowic_i2c_unreset(void *);
extern int is_ngiowic_i2c_unreset(void *);
extern void ngiowic_disable(void *);
extern void ngiowic_enable_intr(int dev, int);
extern void ngiowic_disable_intr(int dev, int);
extern int ngiosm_enable_uart(void *);
extern int ngiosm_disable_uart(void *);
extern int ngiowic_enable_uart(void *);
extern int ngiowic_disable_uart(void *);
extern void ngiosata_enable_intr(int, int);
extern void ngiosata_disable_intr(int, int);
extern void ngiopim_enable_intr(int, int);
extern void ngiopim_disable_intr(int, int);

extern int ngiodc_present(void *);
extern int ngiodc_reset(void *);
extern int ngiodc_unreset(void *);
extern int ngiodc_i2c_reset(void *);
extern int ngiodc_i2c_unreset(void *);
extern int ngiodc_enable(void *);
extern void ngiodc_disable(void *);
extern void ngiodc_enable_intr(int dev, int);
extern void ngiodc_disable_intr(int dev, int);
extern int ngiodc_enable_uart(void *);
extern int ngiodc_disable_uart(void *);


extern int ngio_get_dev(int dev, unsigned char * func);
extern unsigned char ngiowic_get_i2c_ctrl(int dev);
extern unsigned char ngiosm_get_i2c_ctrl(int dev);
extern int ngio_get_i2c_offset(int type);
extern int ngiovm_present(void *);
extern int ngiovm_unreset(void *);
extern int ngiovm_reset(void *);
extern int ngiovm_i2c_unreset(void *);
extern int ngiovm_i2c_reset(void *);
extern int ngiovm_enable(void *);
extern void ngiovm_disable(void *);
extern int ngiovm_enable_uart(void *);
extern int ngiovm_disable_uart(void *);
extern unsigned char ngio_get_i2c_addr(int type);
extern void *get_i2c(int dev, int, int addr);
extern const unsigned char * ngio_err_str(int err);
extern void release_reset_of_sm(int);
extern void oir_sm1_intr_hndlr(int, void*);
extern void oir_sm2_intr_hndlr(int, void*);
extern void oir_sm3_intr_hndlr(int, void*);
extern void oir_sm4_intr_hndlr(int, void*);
extern void oir_wic1_intr_hndlr(int, void*);
extern void oir_wic2_intr_hndlr(int, void*);
extern void oir_wic3_intr_hndlr(int, void*);
extern void oir_sata_intr_hndlr(int, void*);
extern void clr_all_oir_intr(void);
extern void ngio_pci_rdy(void *, int);
extern void ngio_plx_intr_mask(void *, int);
extern int is_ngio_use_pcie(void *);
extern int get_ngio_pcie_dev_bus_num (uint, uint);
extern uint host_ngio_10gkr_capability (uint, uint);
extern int cfg_host_10gkr_port(uint mod_type, uint slot, int ngio_port, int en_10gkr);
extern int ngio_cfg_eth_port_speed(uint mod_type, uint slot,
                        const ngio_eth_speed_t *new_speed, ngio_eth_speed_t *old_speed);

#endif /*__NGIO_H__*/

/******** History ******** 
$Log: ngio.h,v $
Revision 1.22  2020/01/09 01:02:19  jiajliu
Merge Curie 2RU to main trunk

Revision 1.21  2019/08/06 06:56:10  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.20.2.3  2018/12/28 03:30:23  alpeng
update intr util

Revision 1.20.2.2  2018/10/02 20:12:28  ptong
Set correct SM, NIM, VM slot number macro for Curie 1RU

Revision 1.20.2.1  2018/07/12 09:46:51  alpeng
add mb and FPGA board type for curie 1RU; max sm and nim slots; clean up mb_test

Revision 1.20  2018/05/18 09:24:51  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.19  2017/08/10 10:10:41  iachang
CSCvf44161: Merge Goldbeach into USD platform as one image

Revision 1.18.18.7  2018/05/17 10:50:22  alpeng
 sync with trunk <trunk-051618>

Revision 1.18.18.6  2017/11/28 06:11:33  leschen
Return correct MAX SM/WIC/VM slot for VG450.

Revision 1.18.18.5  2017/04/05 08:24:36  leschen
Sync with <ng_diag-tag-032917>

Revision 1.18.18.4  2017/01/23 10:36:52  alpeng
update ngio slot info for triton, proteus and neso

Revision 1.18.18.3  2017/01/06 07:26:00  alpeng
fix poll slot, neptune has no vm

Revision 1.18.18.2  2016/12/16 07:35:05  alpeng
add max wic num for nep, it is used for poll slot

Revision 1.18.18.1  2016/10/18 18:58:55  alpeng
support sm3 and sm4, update intr table

Revision 1.19  2017/08/10 10:10:41  iachang
CSCvf44161: Merge Goldbeach into USD platform as one image

Revision 1.18  2015/06/05 06:13:11  alpeng
fix poll slot issue

Revision 1.17  2014/06/19 22:00:51  ptong
Incorporated BCM sdk-6.4.1-EA2 for 10G-KR interface. New code works on O2, June, and non Greyhound USD regression

Revision 1.16  2014/04/18 00:18:50  ptong
Prepare to support Greyhound 10G-KR bring-up

Revision 1.15  2014/04/14 06:49:55  alpeng
get_ngio_pcie_dev_bus_num() returns NGIO PCIe bus num

Revision 1.14  2014/04/08 10:43:12  alpeng
support general_get_ngio_pcie_bus_num() to return ngio bus num

Revision 1.13  2014/03/18 21:23:40  ptong
Add ngio_plx_intr_mask() to control the PCIe interrupt on PLX switch

Revision 1.12  2013/08/13 03:01:58  alpeng
fix a typo for Juno WIC slot num

Revision 1.11  2013/07/16 10:06:16  alpeng
put platform_slot.c for general using

Revision 1.10  2013/05/31 12:51:28  danchung
Add checking board type for Juno.

Revision 1.9  2013/03/29 05:50:18  mcharon
add function for setting pci rdy bit

Revision 1.8  2012/10/11 08:24:42  alpeng
unsupported SATA on wic slot 3

Revision 1.7  2012/09/20 00:13:01  mcharon
support oir

Revision 1.6  2012/09/12 09:21:13  alpeng
remove SATA test from mbtest and integrate SATA test into ngwic3 test

Revision 1.5  2012/05/11 08:13:27  alpeng
support HDD present to MB test menu

Revision 1.4  2012/05/05 04:02:21  mcharon
support alter daughter board cookie for wic

Revision 1.3  2012/05/04 20:01:46  mcharon
use void* instead of int as argument to to func ptrs in ngio_intf

Revision 1.2  2012/03/28 00:38:18  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:06  ptong
Initial archive of ng_diag module


$Endlog$
*/
