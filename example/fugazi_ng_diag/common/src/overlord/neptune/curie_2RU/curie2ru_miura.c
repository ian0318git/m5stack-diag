/* $Id: curie2ru_miura.c,v 1.2 2020/03/11 17:46:59 jiajliu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_2RU/curie2ru_miura.c,v $
 *------------------------------------------------------------------
 *
 * curie2ru_miura.c - Curie2ru broadcom miura interfaces.
 *
 * Sep. 2018, Nocken Zou <yozou@cisco.com>
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <string.h>

#include "curie2ru_common.h"
#include "curie2ru_miura.h"
#include <bcm_epdm_id.h>
#include <dash_fpga.h>
#include "time_util.h"

int curie2ru_miura_init(struct curie2ru_miura *miura,
                        struct curie2ru_miura_settings *settings)
{
    bcm_plp_access_t *info;
    int rc;

    memset(miura, 0, sizeof(*miura));
    if ((miura->id = bcm_epdm_alloc_id()) < 0) {
        log_err("no enough id to allocate!\n");
        return -1;
    }

    memcpy(&miura->settings, settings, sizeof(*settings));
    miura->type = settings->type;
    info = &miura->info;
    info->platform_ctxt = miura;
    info->phy_addr = miura->id;

    info->lane_map = CURIE2RU_MIURA_LANE_MAP;
    info->if_side = CURIE2RU_MIURA_LINE_SIDE;

    memcpy(&miura->mac_info.phy_info, &miura->info, sizeof(miura->info));

    if ((rc = curie2ru_miura_fw_download(miura))) {
        log_info("firmware download retry once\n");
        bcm_plp_cleanup(miura->type, miura->info);
        if ((rc = curie2ru_miura_fw_download(miura))) {
            goto err;
        }
    }

    return 0;
err:
    return rc;
}

void curie2ru_miura_exit(struct curie2ru_miura *miura)
{
    curie2ru_miura_reset(miura);
    bcm_epdm_free_id(miura->id);
}

void curie2ru_miura_reset(struct curie2ru_miura *miura)
{
    if (miura->downloaded) {
        curie2ru_miura_macsec_exit(miura);
        bcm_plp_mac_cleanup(miura->type, miura->mac_info);
        bcm_plp_cleanup(miura->type, miura->info);
    }
    miura->downloaded = 0;
}

static int curie2ru_miura_mdio_read(void *user_acc, unsigned int core_addr,
                                    unsigned int reg_addr, unsigned int* val)
{
    struct curie2ru_miura *miura = user_acc;
    struct curie2ru_miura_settings *settings = &miura->settings;
    uint8_t prtad = settings->prtad;
    uint8_t devad = ((reg_addr >> 16) & 0x1f);
    uint16_t addr = (reg_addr & 0xffff), data;
    int rc;

    if (!(rc = settings->read(settings->ctx, prtad, devad, addr, &data)))
        *val = data;
    return rc;
}

static int curie2ru_miura_mdio_write(void *user_acc, unsigned int core_addr,
                                     unsigned int reg_addr, unsigned int val)
{
    struct curie2ru_miura *miura = user_acc;
    struct curie2ru_miura_settings *settings = &miura->settings;
    uint8_t prtad = settings->prtad;
    uint8_t devad = ((reg_addr >> 16) & 0x1f);
    uint16_t addr = (reg_addr & 0xffff), data = val;

    return settings->write(settings->ctx, prtad, devad, addr, data);
}

int curie2ru_miura_fw_download(struct curie2ru_miura *miura)
{
    bcm_plp_access_t *info = &miura->info;
    bcm_plp_firmware_load_type_t firmware_load_type;
    int rc;
    struct res_usage base, now;

    info->lane_map = CURIE2RU_MIURA_LANE_MAP;
    info->if_side = CURIE2RU_MIURA_LINE_SIDE;

    /* Initialize chip and download Firmware (using Unicast) */
    memset(&firmware_load_type, 0, sizeof(bcm_plp_firmware_load_type_t));
    firmware_load_type.firmware_load_method = bcmpmFirmwareLoadMethodInternal;
    firmware_load_type.force_load_method = bcmpmFirmwareLoadForce;

    get_res_usage(&base);
    rc = bcm_plp_init_fw_bcast(miura->type, miura->info,
                               curie2ru_miura_mdio_read,
                               curie2ru_miura_mdio_write,
                               &firmware_load_type, bcmpmFirmwareBroadcastNone);
    get_res_usage(&now);
    print_time_elapsed(&base, &now, "FW download time: ");
    if(rc) {
        log_info("bcm_plp_init_fw_bcast API for bcmpmFirmwareBroadcastNone "
                "option failed for PHY-ID[%d], LANE_MAP [0x%x] "
                "with return code [%d]\n", info->phy_addr, info->lane_map, rc);
        return rc;
    }
    log_dbg("bcm_plp_init_fw_bcast API for bcmpmFirmwareBroadcastNone "
            "option Success for PHY-ID[%d], LANE_MAP [0x%x] "
            "with return code [%d]\n", info->phy_addr, info->lane_map, rc);

    miura->downloaded = 1;
    return 0;
}

static int __curie2ru_miura_macsec_init(struct curie2ru_miura *miura,
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

int curie2ru_miura_macsec_init(struct curie2ru_miura *miura, int bypass)
{
    int rc = 0, macsec_side;

    for(macsec_side = CURIE2RU_MIURA_EGRESS;
        macsec_side < CURIE2RU_MIURA_MAX_MACSEC_SIDE_ALLOWED && !rc;
        macsec_side++)
        rc = __curie2ru_miura_macsec_init(miura, macsec_side, bypass);
    return rc;
}

static int __curie2ru_miura_macsec_exit(struct curie2ru_miura *miura,
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

void curie2ru_miura_macsec_exit(struct curie2ru_miura *miura)
{
    int macsec_side;

    for(macsec_side = CURIE2RU_MIURA_EGRESS;
        macsec_side < CURIE2RU_MIURA_MAX_MACSEC_SIDE_ALLOWED; macsec_side++)
        __curie2ru_miura_macsec_exit(miura, macsec_side);
}

/*
 *-----------------------------------------------------------------------------
$Log: curie2ru_miura.c,v $
Revision 1.2  2020/03/11 17:46:59  jiajliu
Refine code for bcm utlity and test

Revision 1.1  2020/01/09 01:01:57  jiajliu
Merge Curie 2RU to main trunk

$Endlog$
 *-----------------------------------------------------------------------------
 */
