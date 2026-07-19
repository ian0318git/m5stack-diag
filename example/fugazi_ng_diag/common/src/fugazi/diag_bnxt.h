/* $Id: diag_bnxt.h,v 1.2 2021/06/02 08:22:34 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/diag_bnxt.h,v $
 *------------------------------------------------------------------
 *
 * diag_bnxt.h - Fugazi broadcom bnxt interfaces header file
 *
 * Dec. 2018, Nocken Zou <yozou@cisco.com>
 *
 * Copyright (c) 2019-2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __FUGAZI_BNXT_H__
#define __FUGAZI_BNXT_H__

#define IF_SYS_DIR "/sys/bus/pci/devices/%04x:%02x:%02x.%01d/net"
#define IF_SYS_PATH_MAX 128


struct fugazi_bnxt_settings {
    uint16_t pci_domain;
    uint8_t pci_bus;
    uint8_t pci_dev;
    uint8_t pci_func;
};

struct fugazi_bnxt {
    struct fugazi_bnxt_settings settings;
#define BNXT_IFNAME_SIZE    32
    char ifname[BNXT_IFNAME_SIZE];
    int skfd;
};

extern int fugazi_bnxt_init(struct fugazi_bnxt *bnxt,
                            struct fugazi_bnxt_settings *settings);
extern void fugazi_bnxt_exit(struct fugazi_bnxt *bnxt);
extern int fugazi_bnxt_mdio_read(struct fugazi_bnxt *bnxt,
                                 uint8_t prtad, uint16_t devad,
                                 uint16_t addr, uint16_t *data);
extern int fugazi_bnxt_mdio_write(struct fugazi_bnxt *bnxt,
                                  uint8_t prtad, uint16_t devad,
                                  uint16_t addr, uint16_t data);

#endif /* __FUGAZI_BNXT_H__ */

/*-------------------------------------------------
 * $Log: diag_bnxt.h,v $
 * Revision 1.2  2021/06/02 08:22:34  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.8.2  2020/08/26 02:37:48  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.6.4  2020/07/31 09:52:08  iachang
 * Code clean up.
 *
 * Revision 1.1.6.3  2019/04/06 01:07:50  iachang
 * BCM82757 10G PHY pass clause 45 parameter into bnxt_en driver. This change also need driver support
 *
 * Revision 1.1.6.2  2019/03/14 03:48:25  letsai
 * Initial check in.
 *
 *
 *
 *
 * $Endlog$
 * */
