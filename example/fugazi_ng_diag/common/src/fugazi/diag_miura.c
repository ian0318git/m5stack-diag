/* $Id: diag_miura.c,v 1.2 2021/06/02 08:22:34 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/diag_miura.c,v $
 *------------------------------------------------------------------
 *
 * diag_miura.c - Fugazi broadcom miura interfaces.
 *
 * Sep. 2018, Nocken Zou <yozou@cisco.com>
 *
 * Copyright (c) 2019-2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <string.h>
#include "nvsysvars.h"
#include "diag_common.h"
#include "diag_miura.h"
#include "diag_bcm_lib.h"
#include "common.h"

static unsigned int fugazi_miura_ids[MAX_NR_FUGAZI_MIURA_ID /
                                     (sizeof(int) * BITS_PER_BYTE)];

static int fugazi_miura_alloc_id(void)
{
    int idx, bit, size = ARRAY_SIZE(fugazi_miura_ids);

    for (idx = 0; idx < size; idx++) {
        if (!(bit = ffs(~fugazi_miura_ids[idx])))
            continue;
        bit -= 1;
        fugazi_miura_ids[idx] |= (1 << bit);
        return idx * sizeof(int) * BITS_PER_BYTE + bit;
    }
    return -1;
}

static void fugazi_miura_free_id(int id)
{
    int idx, bit;

    if ((unsigned int)id >= MAX_NR_FUGAZI_MIURA_ID)
        return;
    idx = id / (sizeof(int) * BITS_PER_BYTE);
    bit = id % (sizeof(int) * BITS_PER_BYTE);
    fugazi_miura_ids[idx] &= ~(1 << bit);
}

int fugazi_miura_init(struct fugazi_miura *miura,
                        struct fugazi_miura_settings *settings)
{
    bcm_plp_access_t *info;
    int rc;

    memset(miura, 0, sizeof(*miura));
    if ((miura->id = fugazi_miura_alloc_id()) < 0) {
        log_err("no enough id to allocate!\n");
        return -1;
    }

    memcpy(&miura->settings, settings, sizeof(*settings));
    miura->type = settings->type;
    info = &miura->info;
    info->platform_ctxt = miura;
    info->phy_addr = settings->miura_id;

    info->lane_map = FUGAZI_MIURA_LANE_MAP;
    info->if_side = FUGAZI_MIURA_LINE_SIDE;

    memcpy(&miura->mac_info.phy_info, &miura->info, sizeof(miura->info));

    if ((rc = fugazi_miura_fw_download(miura))) {
        /* CS7829107: Jinlin - One BCM82757 chip randomly init fail after firmware download completely */
        log_info("firmware download retry once\n");
        if ((rc = fugazi_miura_fw_download(miura))) {
            goto err;
        }
    }
    return 0;
err:
    fugazi_miura_free_id(miura->id);
    return rc;
}

void fugazi_miura_exit(struct fugazi_miura *miura)
{
    fugazi_miura_reset(miura);
    fugazi_miura_free_id(miura->id);
}

void fugazi_miura_reset(struct fugazi_miura *miura)
{
    if (miura->downloaded) {
        fugazi_miura_macsec_exit(miura);
        bcm_plp_mac_cleanup(miura->type, miura->mac_info);
        bcm_plp_cleanup(miura->type, miura->info);
    }
    miura->downloaded = DISABLE;
}

static int fugazi_miura_mdio_read(void* user_acc, unsigned int core_addr,
                                    unsigned int reg_addr, unsigned int* val)
{
    struct fugazi_miura *miura = user_acc;
    struct fugazi_miura_settings *settings = &miura->settings;
    uint8_t devad = ((reg_addr >> 16) & 0x1f);
    uint16_t addr = (reg_addr & 0xffff), data;
    int rc;

    if (!(rc = settings->read(settings->ctx, settings->phy_id, devad, addr, 
                              &data))) {
        *val = data;
    }
    return rc;
}

static int fugazi_miura_mdio_write(void* user_acc, unsigned int core_addr,
                                     unsigned int reg_addr, unsigned int val)
{
    struct fugazi_miura *miura = user_acc;
    struct fugazi_miura_settings *settings = &miura->settings;
    uint8_t devad = ((reg_addr >> 16) & 0x1f);
    uint16_t addr = (reg_addr & 0xffff), data = val;

    return settings->write(settings->ctx, settings->phy_id, devad, addr, data);
}

int fugazi_miura_fw_download(struct fugazi_miura *miura)
{
    bcm_plp_access_t *info = &miura->info;
    bcm_plp_firmware_load_type_t firmware_load_type;
    int rc;
    
    info->lane_map = FUGAZI_MIURA_LANE_MAP;
    info->if_side = FUGAZI_MIURA_LINE_SIDE;

    /* Initialize chip and download Firmware (using Unicast) */
    memset(&firmware_load_type, 0, sizeof(bcm_plp_firmware_load_type_t));
    firmware_load_type.firmware_load_method = bcmpmFirmwareLoadMethodInternal;
    firmware_load_type.force_load_method = bcmpmFirmwareLoadForce;

    rc = bcm_plp_init_fw_bcast(miura->type, miura->info,
                               fugazi_miura_mdio_read,
                               fugazi_miura_mdio_write,
                               &firmware_load_type, bcmpmFirmwareBroadcastNone);
    if(rc) {
        log_err("bcm_plp_init_fw_bcast API for bcmpmFirmwareBroadcastNone "
                "option failed for PHY-ID[%d], LANE_MAP [0x%x] "
                "with return code [%d]\n", info->phy_addr, info->lane_map, rc);
        /* BRCM FAE advice adding bcm_plp_mac_cleanup() and bcm_plp_cleanup() 
           before 2nd time firmware download */
        bcm_plp_mac_cleanup(miura->type, miura->mac_info); 
        bcm_plp_cleanup(miura->type, miura->info);
        return rc;
    }
    log_dbg("bcm_plp_init_fw_bcast API for bcmpmFirmwareBroadcastNone "
            "option Success for PHY-ID[%d], LANE_MAP [0x%x] "
            "with return code [%d]\n", info->phy_addr, info->lane_map, rc);

    miura->downloaded = ENABLE;
    return (PASSED);
}

static int __fugazi_miura_macsec_init(struct fugazi_miura *miura,
                                        int macsec_side, int bypass)
{
    bcm_plp_sec_phy_access_t sec_info;
    int secy_rc, cfye_rc; /* Return code */
    bcm_plp_cfye_init_t init_settings;
    bcm_plp_secy_settings_t settings;

    memset(&sec_info, 0, sizeof(sec_info));
    memcpy(&sec_info.phy_info, &miura->info, sizeof(sec_info.phy_info));
    sec_info.macsec_side = macsec_side;

    memset(&init_settings, 0, sizeof(bcm_plp_cfye_init_t));

    /* If bypass flag is enable, set Cfye to bypass mode */
    if (bypass)
        init_settings.flow_latency_bypass = 1;

    cfye_rc = bcm_plp_cfye_device_init(miura->type, &sec_info, &init_settings);

    if (cfye_rc != BCM_PLP_CFYE_STATUS_OK){
        log_err("bcm_plp_cfye_device_init API failed for PHY-ID[%d], "
                "macsec_side [%d], return code [%d] \n",
                sec_info.phy_info.phy_addr, sec_info.macsec_side, cfye_rc);
        return cfye_rc;
    } else {
        log_dbg("bcm_plp_cfye_device_init API success for PHY-ID[%d], "
                "macsec_side [%d], return code [%d] \n",
                sec_info.phy_info.phy_addr, sec_info.macsec_side, cfye_rc);
    }
    miura->cfye_inited[macsec_side] = 1;

    memset(&settings, 0, sizeof(bcm_plp_secy_settings_t));

    if (bypass) {
        settings.drop_bypass.fbypass = 1;
    } else {
        settings.drop_bypass.drop_type = BCM_PLP_SECY_SA_DROP_INTERNAL;
    }

    /*Initializes a SecY device_id instance identified by IntefaceId parameter.*/
    secy_rc = bcm_plp_secy_device_init(miura->type, &sec_info, &settings);

    if (secy_rc != BCM_PLP_SECY_STATUS_OK) {
        log_err("bcm_plp_secy_device_init API failed for PHY-ID[%d], "
                "macsec_side [%d], return code [%d] \n",
                sec_info.phy_info.phy_addr, sec_info.macsec_side, secy_rc);
        return secy_rc;
    } else {
        log_dbg("bcm_plp_secy_device_init API success for PHY-ID[%d], "
                "macsec_side [%d], return code [%d] \n",
                sec_info.phy_info.phy_addr, sec_info.macsec_side, secy_rc);
    }
    miura->secy_inited[macsec_side] = 1;

    return (secy_rc || cfye_rc);
}

int fugazi_miura_macsec_init(struct fugazi_miura *miura, int bypass)
{
    int rc = 0, macsec_side;

    for(macsec_side = FUGAZI_MIURA_EGRESS;
        macsec_side < FUGAZI_MIURA_MAX_MACSEC_SIDE_ALLOWED && !rc;
        macsec_side++)
        rc = __fugazi_miura_macsec_init(miura, macsec_side, bypass);
    return rc;
}

static int __fugazi_miura_macsec_exit(struct fugazi_miura *miura,
                                        int macsec_side)
{
    bcm_plp_sec_phy_access_t sec_info;
    int secy_rc, cfye_rc; /* Return code */

    memset(&sec_info, 0, sizeof(sec_info));
    memcpy(&sec_info.phy_info, &miura->info, sizeof(sec_info.phy_info));
    sec_info.macsec_side = macsec_side;

    if (miura->secy_inited[macsec_side]) {
        secy_rc = bcm_plp_secy_device_uninit(miura->type, &sec_info);

        if (secy_rc != BCM_PLP_SECY_STATUS_OK) {
            log_err("bcm_plp_secy_device_init API failed for PHY-ID[%d], "
                    "macsec_side [%d], return code [%d] \n",
                    sec_info.phy_info.phy_addr, sec_info.macsec_side, secy_rc);
            return secy_rc;
        } else {
            log_dbg("bcm_plp_secy_device_init API success for PHY-ID[%d], "
                    "macsec_side [%d], return code [%d] \n",
                    sec_info.phy_info.phy_addr, sec_info.macsec_side, secy_rc);
        }
        miura->secy_inited[macsec_side] = 0;
    }

    if (miura->cfye_inited[macsec_side]) {
        cfye_rc = bcm_plp_cfye_device_uninit(miura->type, &sec_info);

        if (cfye_rc != BCM_PLP_CFYE_STATUS_OK){
            log_err("bcm_plp_cfye_device_init API failed for PHY-ID[%d], "
                    "macsec_side [%d], return code [%d] \n",
                    sec_info.phy_info.phy_addr, sec_info.macsec_side, cfye_rc);
            return cfye_rc;
        } else {
            log_dbg("bcm_plp_cfye_device_init API success for PHY-ID[%d], "
                    "macsec_side [%d], return code [%d] \n",
                    sec_info.phy_info.phy_addr, sec_info.macsec_side, cfye_rc);
        }
        miura->cfye_inited[macsec_side] = 0;
    }

    return (secy_rc || cfye_rc);
}

void fugazi_miura_macsec_exit(struct fugazi_miura *miura)
{
    int macsec_side;

    for(macsec_side = FUGAZI_MIURA_EGRESS;
        macsec_side < FUGAZI_MIURA_MAX_MACSEC_SIDE_ALLOWED; macsec_side++)
        __fugazi_miura_macsec_exit(miura, macsec_side);
}


/*-------------------------------------------------
 * $Log: diag_miura.c,v $
 * Revision 1.2  2021/06/02 08:22:34  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.8.2  2020/08/26 02:37:49  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.6.11  2020/07/31 09:52:09  iachang
 * Code clean up.
 *
 * Revision 1.1.6.10  2020/07/02 02:20:08  iachang
 * emove duplicate functions and add bcm_plp_mac_cleanup() into fugazi_miura_fw_download
 *
 * Revision 1.1.6.9  2020/06/29 03:31:43  iachang
 * BRCM FAE advice adding bcm_plp_mac_cleanup() and bcm_plp_cleanup() before 2nd time firmware download
 *
 * Revision 1.1.6.8  2020/01/15 07:30:08  iachang
 * Skip BCM82757 fw download with Diag initial. It can save Diag menu boot up time, and help debug.
 *
 * Revision 1.1.6.7  2019/10/22 02:05:53  iachang
 * CS7829107: One BCM82757 chip randomly init fail after firmware download completely
 *
 * Revision 1.1.6.6  2019/08/02 03:32:38  iachang
 * Add BCM82757 Regs dump utility
 * Add packet count check when BCM82757 loopback test failed.
 *
 * Revision 1.1.6.5  2019/04/17 22:44:16  iachang
 * Modify BCM82757 PHY ID for P1A2 board.
 *
 * Revision 1.1.6.4  2019/04/06 01:07:50  iachang
 * BCM82757 10G PHY pass clause 45 parameter into bnxt_en driver. This change also need driver support
 *
 * Revision 1.1.6.3  2019/03/14 21:46:47  iachang
 * Bring up BCM82757 first PHY.
 *
 * Revision 1.1.6.2  2019/03/14 03:48:35  letsai
 * Initial check in.
 *
 *
 *
 *
 * $Endlog$
 * */
