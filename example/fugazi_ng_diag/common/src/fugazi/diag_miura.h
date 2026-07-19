/* $Id: diag_miura.h,v 1.2 2021/06/02 08:22:34 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/diag_miura.h,v $
 *------------------------------------------------------------------
 *
 * diag_miura.h - Curie2ru broadcom miura interfaces.
 *
 * Sep. 2018, Nocken Zou <yozou@cisco.com>
 *
 * Copyright (c) 2019-2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __FUGAZI_MIURA_H__
#define __FUGAZI_MIURA_H__

#ifndef PLP_MIURA_SUPPORT
#define PLP_MIURA_SUPPORT
#endif
#ifndef PLP_MACSEC_SUPPORT
#define PLP_MACSEC_SUPPORT
#endif

#include <stdint.h>
#include <epdm.h>
#include <epdm_sec.h>

#define FUGAZI_MIURA_LANE_MAP   0xF
#define FUGAZI_MIURA_LANE_MAX   4
#define FUGAZI_MIURA_SYS_SIDE   1
#define FUGAZI_MIURA_LINE_SIDE  0
#define FUGAZI_MIURA_EGRESS     0
#define FUGAZI_MIURA_INGRESS    1
#define BITS_PER_BYTE           8
#define MAX_NR_FUGAZI_MIURA_ID  1024
#define FUGAZI_MIURA_MAX_MACSEC_SIDE_ALLOWED 2

struct fugazi_miura_settings {
    const char *type;
    void *ctx;
    int phy_id;
    int (*read)(void* ctx, uint16_t phy_id, uint16_t devad, uint16_t addr, 
                uint16_t *data);
    int (*write)(void* ctx, uint16_t phy_id, uint16_t devad, uint16_t addr, 
                 uint16_t data);
    int miura_id;
};

struct fugazi_miura {
    const char *type;
    bcm_plp_access_t info;
    bcm_plp_mac_access_t mac_info;
    struct fugazi_miura_settings settings;
    int id;
    int downloaded;
    int cfye_inited[FUGAZI_MIURA_MAX_MACSEC_SIDE_ALLOWED];
    int secy_inited[FUGAZI_MIURA_MAX_MACSEC_SIDE_ALLOWED];
};

int fugazi_miura_init(struct fugazi_miura *miura,
                        struct fugazi_miura_settings *settings);
void fugazi_miura_exit(struct fugazi_miura *miura);
void fugazi_miura_reset(struct fugazi_miura *miura);

int fugazi_miura_fw_download(struct fugazi_miura *miura);
int fugazi_miura_macsec_init(struct fugazi_miura *miura, int bypass);
void fugazi_miura_macsec_exit(struct fugazi_miura *miura);

#endif /* __FUGAZI_MIURA_H__ */

/*-------------------------------------------------
 * $Log: diag_miura.h,v $
 * Revision 1.2  2021/06/02 08:22:34  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.8.2  2020/08/26 02:37:49  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.6.6  2020/08/04 08:37:06  iachang
 * Update Copyright to 2020
 *
 * Revision 1.1.6.5  2020/01/15 07:30:08  iachang
 * Skip BCM82757 fw download with Diag initial. It can save Diag menu boot up time, and help debug.
 *
 * Revision 1.1.6.4  2019/04/17 22:44:16  iachang
 * Modify BCM82757 PHY ID for P1A2 board.
 *
 * Revision 1.1.6.3  2019/04/06 01:07:50  iachang
 * BCM82757 10G PHY pass clause 45 parameter into bnxt_en driver. This change also need driver support
 *
 * Revision 1.1.6.2  2019/03/14 03:48:26  letsai
 * Initial check in.
 *
 *
 *
 *
 * $Endlog$
 * */
