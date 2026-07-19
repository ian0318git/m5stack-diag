/* $Id: curie2ru_quadra28.c,v 1.1 2020/01/09 01:01:57 jiajliu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_2RU/curie2ru_quadra28.c,v $
 *------------------------------------------------------------------
 *
 * curie2ru_quadra28.c - Curie2ru broadcom quadra28 interfaces.
 *
 * Feb. 2019, Jiajia Liu <jiajliu@cisco.com>
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <string.h>

#include "curie2ru_common.h"
#include "curie2ru_quadra28.h"
#include "time_util.h"

int curie2ru_quadra28_init(struct curie2ru_quadra28 *q28,
                           struct curie2ru_quadra28_settings *settings)
{
    bcm_plp_access_t *info;
    int i;

    memset(q28, 0, sizeof(*q28) * 2);

    q28[0].id = 0x10;
    q28[1].id = 0x11;

    for (i = 0; i < 2; i++) {
        memcpy(&q28[i].settings, &settings[i], sizeof(*settings));
        q28[i].type = settings[i].type;
    }

    for (i = 0; i < 2; i++) {
        info = &q28[i].info;
        info->platform_ctxt = &q28[i];
        info->phy_addr = q28[i].id;

        info->lane_map = CURIE2RU_QUADRA28_LANE_MAP;
        info->if_side = CURIE2RU_QUADRA28_LINE_SIDE;
    }

    return curie2ru_quadra28_fw_download(q28);
}

void curie2ru_quadra28_exit(struct curie2ru_quadra28 *q28)
{
    curie2ru_quadra28_reset(q28);
}

void curie2ru_quadra28_reset(struct curie2ru_quadra28 *q28)
{
    int i;

    for (i = 0; i < 2; i++) {
        if (q28[i].downloaded) {
            bcm_plp_cleanup(q28[i].type, q28[i].info);
            q28[i].downloaded = 0;
        }
    }
}

static int curie2ru_quadra28_mdio_read(void* user_acc, unsigned int core_addr,
                                    unsigned int reg_addr, unsigned int* val)
{
    struct curie2ru_quadra28 *q28 = user_acc;
    struct curie2ru_quadra28_settings *settings = &q28->settings;
    uint8_t devad = ((reg_addr >> 16) & 0x1f);
    uint16_t addr = (reg_addr & 0xffff), data;
    int rc;

    if (!(rc = settings->read(settings->ctx, core_addr, devad, addr, &data)))
        *val = data;
    return rc;
}

static int curie2ru_quadra28_mdio_write(void* user_acc, unsigned int core_addr,
                                     unsigned int reg_addr, unsigned int val)
{
    struct curie2ru_quadra28 *q28 = user_acc;
    struct curie2ru_quadra28_settings *settings = &q28->settings;
    uint8_t devad = ((reg_addr >> 16) & 0x1f);
    uint16_t addr = (reg_addr & 0xffff), data = val;

    return settings->write(settings->ctx, core_addr, devad, addr, data);
}

int curie2ru_quadra28_fw_download(struct curie2ru_quadra28 *q28)
{
    bcm_plp_access_t *info;
    plp_static_config_t stat;
    plp_static_config_t stat_s;
    bcm_plp_firmware_load_type_t fw_load_type;
    int rv = 0;
    int i;
    struct res_usage base, now;

    memset(&stat,0,sizeof(plp_static_config_t));
    memset(&stat_s,0,sizeof(plp_static_config_t));
    fw_load_type.firmware_load_method = bcmpmFirmwareLoadMethodInternal;
    fw_load_type.force_load_method = bcmpmFirmwareLoadForce;
    stat.ull_dp = 0;

    for (i = 0; i < 2; i++) {
        info = &q28[i].info;
        info->flags = 0;    /* Coldboot */
        info->phy_addr = q28[i].id;

        rv = bcm_plp_static_config_set(q28[i].type, *info, (void*)&stat);
        if (rv != 0)
            printf("\nbcm_plp_static_config_set failed rv %d\n", rv);

        rv = bcm_plp_static_config_get(q28[i].type, *info, (void*)&stat_s);
        if(rv != 0)
            printf("\nbcm_plp_static_config_get failed rv %d\n", rv);
        else
            printf("\nstat structure = static.ull_dp=%d\n", stat_s.ull_dp);

        fw_load_type.firmware_load_method = bcmpmFirmwareLoadMethodInternal;
        rv = bcm_plp_init_fw_bcast(q28[i].type, *info,
                                   curie2ru_quadra28_mdio_read,
                                   curie2ru_quadra28_mdio_write, &fw_load_type,
                                   bcmpmFirmwareBroadcastEnable);
        if (rv != 0) {
            printf("Bcast enable init failed for phy_id %d rv %d\n",
                   info->phy_addr, rv);
            return rv;
        }
    }

    get_res_usage(&base);
    info = &q28[0].info;
    fw_load_type.firmware_load_method = bcmpmFirmwareLoadMethodInternal;
    rv = bcm_plp_init_fw_bcast(q28[0].type, *info,
                               curie2ru_quadra28_mdio_read,
                               curie2ru_quadra28_mdio_write,
                               &fw_load_type,
                               bcmpmFirmwareBroadcastFirmwareExecute);
    get_res_usage(&now);
    print_time_elapsed(&base, &now, "FW download time: ");
    if (rv != 0) {
        printf("Bcast execute FW bcst init failed for phy_id %d rv %d\n", 0, rv);
        return rv;
     }

    rv = bcm_plp_init_fw_bcast(q28[0].type, *info,
                               curie2ru_quadra28_mdio_read,
                               curie2ru_quadra28_mdio_write, &fw_load_type,
                               bcmpmFirmwareBroadcastFirmwareVerify);
    if (rv != 0) {
        printf("Bcast Verify init failed for phy_id %d rv %d\n",
               info->phy_addr, rv);
        return rv;
    }

    rv =  bcm_plp_init_fw_bcast(q28[0].type, *info,
                                curie2ru_quadra28_mdio_read,
                                curie2ru_quadra28_mdio_write, &fw_load_type,
                                bcmpmFirmwareBroadcastEnd);
    if (rv != 0) {
        printf("Bcast End init failed for phy_id %d rv %d\n",
               info->phy_addr, rv);
        return rv;
    }

    q28[0].downloaded = 1;
    q28[1].downloaded = 1;

    return 0;
}

int bcmphy_quadra28_reg_read(struct curie2ru_quadra28 *q28,
							 int if_side, unsigned int phy_id,
                             unsigned int lane, unsigned int dev_id,
                             unsigned int *reg_addr, unsigned int *val, int n)
{
    int i, rv;
    int lane_index;
    unsigned int v;
    bcm_plp_access_t *info = &q28->info;

    info->if_side = if_side;
    /* Slelect the side  */
    if (if_side == 1) {
        rv = bcm_plp_reg_value_get(q28->type, *info, dev_id, 0xffff, &v);
        v |= 1;
        rv = bcm_plp_reg_value_set(q28->type, *info, dev_id, 0xffff, v);
    } else {
        rv = bcm_plp_reg_value_get(q28->type, *info, dev_id, 0xffff, &v);
        v &= ~(1);
        rv = bcm_plp_reg_value_set(q28->type, *info, dev_id, 0xffff, v);
    }

    /* Slelect the channel based on lane */
    if (lane == 0xf) {
        /*Set the address extension register*/
        rv = bcm_plp_reg_value_get(q28->type, *info, dev_id, 0xc702, &v);
        v &= ~(0xf);
        rv = bcm_plp_reg_value_set(q28->type, *info, dev_id, 0xc702, v);
        /*Enable the broadcast */
        rv = bcm_plp_reg_value_get(q28->type, *info, dev_id, 0xc712, &v);
        v |= 1;
        rv = bcm_plp_reg_value_set(q28->type, *info, dev_id, 0xc712, v);
    } else {     /*Select the channel based on lane number */
        for (lane_index = 0; lane_index < 4; lane_index++) {
            if (lane & (1<<lane_index)) {
                rv = bcm_plp_reg_value_get(q28->type, *info, dev_id, 0xc702, &v);
                v |= lane_index;
                rv = bcm_plp_reg_value_set(q28->type, *info, dev_id, 0xc702, v);
            }
        }
    }
    /* Get the value  */
    for (i = 0; i < n; i++) {
        rv = bcm_plp_reg_value_get(q28->type, *info, dev_id, reg_addr[i], &val[i]);
    }
    /* reset the broadcast and line side interface   */
    rv = bcm_plp_reg_value_get(q28->type, *info, dev_id, 0xffff, &v);
    v &= ~(1);
    rv = bcm_plp_reg_value_set(q28->type, *info, dev_id, 0xffff, v);
    rv = bcm_plp_reg_value_get(q28->type, *info, dev_id, 0xc712, &v);
    v &=~(1);
    rv = bcm_plp_reg_value_set(q28->type, *info, dev_id, 0xc712, v);

    return rv;
}

int bcmphy_quadra28_reg_write(struct curie2ru_quadra28 *q28,
                              int if_side, unsigned int phy_id,
                              unsigned int lane, unsigned int dev_id,
                              unsigned int *reg_addr, unsigned int *val, int n)
{
    int i,rv;
    int lane_index;
    unsigned int v;
    bcm_plp_access_t *info = &q28->info;

    info->if_side = if_side;
    /* Slelect the side  */
    if (if_side == 1) {
        rv = bcm_plp_reg_value_get(q28->type, *info, dev_id, 0xffff, &v);
        v |= 1;
        rv= bcm_plp_reg_value_set(q28->type, *info, dev_id, 0xffff, v);
    } else {
        rv = bcm_plp_reg_value_get(q28->type, *info, dev_id, 0xffff, &v);
        v &= ~(1);
        rv = bcm_plp_reg_value_set(q28->type, *info, dev_id, 0xffff, v);
    }
    /* Slelect the channel based on lane */
    if (lane == 0xf) {
        /*Set the address extension register*/
        rv = bcm_plp_reg_value_get(q28->type, *info, dev_id, 0xc702, &v);
        v &= ~(0xf);
        rv = bcm_plp_reg_value_set(q28->type, *info, dev_id, 0xc702, v);
        /*Enable the broadcast */
        rv = bcm_plp_reg_value_get(q28->type, *info, dev_id, 0xc712, &v);
        v |= 1;
        rv = bcm_plp_reg_value_set(q28->type, *info, dev_id, 0xc712, v);
    } else {     /*Select the channel based on lane number */
        for (lane_index = 0; lane_index < 4; lane_index++) {
            if (lane & (1<<lane_index)) {
                rv = bcm_plp_reg_value_get(q28->type, *info, dev_id, 0xc702, &v);
                v |= lane_index;
                rv = bcm_plp_reg_value_set(q28->type, *info, dev_id, 0xc702, v);
            }
        }
    }
    /* Set the value  */
    for (i = 0; i < n; i++) {
        rv = bcm_plp_reg_value_set(q28->type, *info, dev_id, reg_addr[i], val[i]);
    }
    /* reset the broadcast and line side interface   */
    rv = bcm_plp_reg_value_get(q28->type, *info, dev_id, 0xffff, &v);
    v &= ~(1);
    rv = bcm_plp_reg_value_set(q28->type, *info, dev_id, 0xffff, v);
    rv = bcm_plp_reg_value_get(q28->type, *info, dev_id, 0xc712, &v);
    v &=~(1);
    rv = bcm_plp_reg_value_set(q28->type, *info, dev_id, 0xc712, v);

    return rv;
}

/*
 *-----------------------------------------------------------------------------
$Log: curie2ru_quadra28.c,v $
Revision 1.1  2020/01/09 01:01:57  jiajliu
Merge Curie 2RU to main trunk

$Endlog$
 *-----------------------------------------------------------------------------
 */
