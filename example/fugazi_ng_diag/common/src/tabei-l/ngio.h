 /* $Id: ngio.h,v 1.3 2020/01/09 01:02:42 jiajliu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/ngio.h,v $
 *------------------------------------------------------------------
 * Filename:    ngio.h
 *
 *
 * Copyright (c) 2018-2019 by cisco Systems, Inc.
 * All rights reserved.
 *
 * Tabei only have WIC ngio, create this dummy file for
 * common source file cookie_4_core.c needs to include it.
 *
 *------------------------------------------------------------------
 */
#ifndef __NGIO_H__
#define __NGIO_H__


#define ENABLE_DELAY            (600)
#define FIRST_SLOT              (1)
#define I2C_UNRESET_DELAY       (50)

#define MAX_WIC                 (1)
#define MAX_DC                  (1)

#define NGIO_BASE  0x32000


/* Dummy declaration */
#define MAX_SM 4 
#define MAX_VM 0
#define SM_I2C_ADDR_IO_PORT 0

#define GPIO_EXP_GE0_10GKR_BIT     0x1
#define GPIO_EXP_GE1_10GKR_BIT     0x2

enum {
    NIM1 = 1,
};

enum {
    NGIO_FAIL_PWR_ENABLE = -1,
    NGIO_FAIL_I2C = -2,
    NGIO_FAIL_END = 2,
};

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
extern uint pcie_config_read(uint32_t, uint32_t, uint16_t, uint, uint);
extern void pcie_config_write(uint32_t, uint32_t, uint16_t, uint, uint, uint32_t);

/* although there is no vm and sm on tabei
 * we leave these prototypes for tam_act2_utils.c 
 * for bypass compile warning 
 */
extern int ngiosm_i2c_unreset(void *);
extern int ngiosm_i2c_reset(void *);
extern int ngiovm_i2c_unreset(void *);
extern int ngiovm_i2c_reset(void *);
extern int ngiowic_i2c_unreset(void *);
extern int ngiowic_i2c_reset(void *);

extern int ngiovm_present (void *);
extern int ngiovm_enable (void *);
extern void ngiovm_disable (void *);
extern int ngiovm_reset (void *);
extern int ngiovm_unreset (void *);
extern int ngiosm_present (void *);
extern int ngiosm_enable (void *);
extern void ngiosm_disable (void *);
extern int ngiosm_reset (void *);
extern int ngiosm_unreset (void *);

extern int ngiosm_enable_uart (void *);
extern int ngiosm_disable_uart (void *);
extern int ngiovm_enable_uart (void *);
extern int ngiovm_disable_uart (void *);
extern void ngiosm_disable_intr(int dev, int);

extern void ngiosm_pci_rdy (void *, int );
extern int cfg_host_10gkr_port(uint , uint , int , int );
extern int get_ngio_pcie_dev_bus_num (uint, uint);

typedef enum {
    NGIO_ETH_SPEED_10M  = 1,
    NGIO_ETH_SPEED_100M = 2,
    NGIO_ETH_SPEED_1G   = 3,
    NGIO_ETH_SPEED_10G  = 4,
    NGIO_ETH_SPEED_UNKNOWN = 0xFF,
} ngio_eth_speed_t;

extern int ngio_cfg_eth_port_speed(uint mod_type, uint slot,
                        const ngio_eth_speed_t *new_speed, ngio_eth_speed_t *old_speed);

#endif /*__NGIO_H__*/

/*-------------------------------------------------
 * $Log: ngio.h,v $
 * Revision 1.3  2020/01/09 01:02:42  jiajliu
 * Merge Curie 2RU to main trunk
 *
 * Revision 1.2  2019/10/17 02:16:25  kehuang2
 * Collapse Tabei-L into main trunk
 *
 * Revision 1.1.2.8  2019/05/22 07:52:43  olin2
 * Support Prince on Tabei-L
 *
 * Revision 1.1.2.7  2018/11/16 05:42:12  olin2
 * Clean up code
 *
 * Revision 1.1.2.6  2018/11/02 07:44:20  olin2
 * Update PCIE setting
 *
 * Revision 1.1.2.5  2018/11/02 02:39:03  kodko
 * Support cookie read for NIM and PIM modules.
 *
 * Revision 1.1.2.4  2018/10/16 11:33:14  olin2
 * Update NIM test
 *
 * Revision 1.1.2.3  2018/10/15 11:48:29  olin2
 * Update for using common slot.c
 *
 * Revision 1.1.2.2  2018/10/09 09:22:05  olin2
 * Initial commit for NIM test
 *
 * Revision 1.1.2.1  2018/10/02 01:50:03  harrchan
 * Initial commit for Tabei-L P1A bring up.
 *
 * $Endlog$
 *-------------------------------------------------
 */
