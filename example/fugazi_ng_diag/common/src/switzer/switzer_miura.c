/* $Id: switzer_miura.c,v 1.3 2021/10/20 06:10:18 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/switzer/switzer_miura.c,v $
 *------------------------------------------------------------------
 *
 * switzer_miura.h - Switzer broadcom miura interfaces.
 *
 * Sep. 2018, Nocken Zou <yozou@cisco.com>
 *
 * Copyright (c) 2018-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <string.h>

#include "common.h"
#include "switzer_common.h"
#include "switzer_miura.h"

#define BITS_PER_BYTE    8
#define MAX_NR_SWITZER_MIURA_ID 1024

static unsigned int switzer_miura_ids[MAX_NR_SWITZER_MIURA_ID /
                                      (sizeof(int) * BITS_PER_BYTE)];

int __attribute__((weak)) bcm_epdm_alloc_id(void)
{
    int idx, bit, size = ARRAY_SIZE(switzer_miura_ids);

    for (idx = 0; idx < size; idx++) {
        if (!(bit = ffs(~switzer_miura_ids[idx])))
            continue;
        bit -= 1;
        switzer_miura_ids[idx] |= (1 << bit);
        return idx * sizeof(int) * BITS_PER_BYTE + bit;
    }
    return -1;
}

void __attribute__((weak)) bcm_epdm_free_id(int id)
{
    int idx, bit;

    if ((unsigned int)id >= MAX_NR_SWITZER_MIURA_ID)
        return;
    idx = id / (sizeof(int) * BITS_PER_BYTE);
    bit = id % (sizeof(int) * BITS_PER_BYTE);
    switzer_miura_ids[idx] &= ~(1 << bit);
}

static int switzer_miura_alloc_id(void)
{
     return bcm_epdm_alloc_id();
}

static void switzer_miura_free_id(int id)
{
    bcm_epdm_free_id(id);
}

int switzer_miura_init(struct switzer_miura *miura,
                       struct switzer_miura_settings *settings)
{
    bcm_plp_access_t *info;
    int rc;

    memset(miura, 0, sizeof(*miura));
    if ((miura->id = switzer_miura_alloc_id()) < 0) {
        log_err("no enough id to allocate!\n");
        return -1;
    }
    memcpy(&miura->settings, settings, sizeof(*settings));
    miura->type = settings->type;
    info = &miura->info;
    info->platform_ctxt = miura;
    info->phy_addr = miura->id;

    info->lane_map = SWITZER_MIURA_LANE_MAP;
    info->if_side = SWITZER_MIURA_LINE_SIDE;

    memcpy(&miura->mac_info.phy_info, &miura->info, sizeof(miura->info));

    if ((rc = switzer_miura_fw_download(miura)))
        goto err;

    return 0;
err:
    switzer_miura_free_id(miura->id);
    return rc;
}

void switzer_miura_exit(struct switzer_miura *miura)
{
    switzer_miura_reset(miura);
    switzer_miura_free_id(miura->id);
}

void switzer_miura_reset(struct switzer_miura *miura)
{
    if (miura->downloaded) {
        switzer_miura_macsec_exit(miura);
        bcm_plp_mac_cleanup(miura->type, miura->mac_info);
        bcm_plp_cleanup(miura->type, miura->info);
    }
    miura->downloaded = 0;
}

static int switzer_miura_mdio_read(void* user_acc, unsigned int core_addr,
                                   unsigned int reg_addr, unsigned int* val)
{
    struct switzer_miura *miura = user_acc;
    struct switzer_miura_settings *settings = &miura->settings;
    uint8_t devad = ((reg_addr >> 16) & 0x1f);
    uint16_t addr = (reg_addr & 0xffff), data;
    int rc;

    if (!(rc = settings->read(settings->ctx, devad, addr, &data)))
        *val = data;
    return rc;
}

static int switzer_miura_mdio_write(void* user_acc, unsigned int core_addr,
                                    unsigned int reg_addr, unsigned int val)
{
    struct switzer_miura *miura = user_acc;
    struct switzer_miura_settings *settings = &miura->settings;
    uint8_t devad = ((reg_addr >> 16) & 0x1f);
    uint16_t addr = (reg_addr & 0xffff), data = val;

    return settings->write(settings->ctx, devad, addr, data);
}

int switzer_miura_fw_download(struct switzer_miura *miura)
{
    bcm_plp_access_t *info = &miura->info;
    bcm_plp_firmware_load_type_t firmware_load_type;
    int rc;

    info->lane_map = SWITZER_MIURA_LANE_MAP;
    info->if_side = SWITZER_MIURA_LINE_SIDE;

    /* Initialize chip and download Firmware (using Unicast) */
    memset(&firmware_load_type, 0, sizeof(bcm_plp_firmware_load_type_t));
    firmware_load_type.firmware_load_method = bcmpmFirmwareLoadMethodInternal;
    firmware_load_type.force_load_method = bcmpmFirmwareLoadForce;

    rc = bcm_plp_init_fw_bcast(miura->type, miura->info,
                               switzer_miura_mdio_read,
                               switzer_miura_mdio_write,
                               &firmware_load_type, bcmpmFirmwareBroadcastNone);
    if(rc) {
        /* As FAE's suggestion, invoke download api after failed */
        bcm_plp_mac_cleanup(miura->type, miura->mac_info);
        bcm_plp_cleanup(miura->type, miura->info);
        rc = bcm_plp_init_fw_bcast(miura->type, miura->info,
                                   switzer_miura_mdio_read,
                                   switzer_miura_mdio_write,
                                   &firmware_load_type, bcmpmFirmwareBroadcastNone);
        if (rc) {
            log_err("bcm_plp_init_fw_bcast API for bcmpmFirmwareBroadcastNone "
                    "option failed for PHY-ID[%d], LANE_MAP [0x%x] "
                    "with return code [%d]\n", info->phy_addr, info->lane_map, rc);
            return rc;
        }
    }
    log_dbg("bcm_plp_init_fw_bcast API for bcmpmFirmwareBroadcastNone "
            "option Success for PHY-ID[%d], LANE_MAP [0x%x] "
            "with return code [%d]\n", info->phy_addr, info->lane_map, rc);

    miura->downloaded = 1;
    return 0;
}

static int __switzer_miura_macsec_init(struct switzer_miura *miura,
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

int switzer_miura_macsec_init(struct switzer_miura *miura, int bypass)
{
    int rc = 0, macsec_side;

    for(macsec_side = SWITZER_MIURA_EGRESS;
        macsec_side < SWITZER_MIURA_MAX_MACSEC_SIDE_ALLOWED && !rc;
        macsec_side++)
        rc = __switzer_miura_macsec_init(miura, macsec_side, bypass);
    return rc;
}

static int __switzer_miura_macsec_exit(struct switzer_miura *miura,
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

void switzer_miura_macsec_exit(struct switzer_miura *miura)
{
    int macsec_side;

    for(macsec_side = SWITZER_MIURA_EGRESS;
        macsec_side < SWITZER_MIURA_MAX_MACSEC_SIDE_ALLOWED; macsec_side++)
        __switzer_miura_macsec_exit(miura, macsec_side);
}
