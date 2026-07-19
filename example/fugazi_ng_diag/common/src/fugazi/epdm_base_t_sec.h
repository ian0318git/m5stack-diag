/*
 *
 * $Id: epdm_base_t_sec.h $
 *
 * $Copyright: Copyright 2016 Broadcom Limited.
 * This program is the proprietary software of Broadcom Limited
 * and/or its licensors, and may only be used, duplicated, modified
 * or distributed pursuant to the terms and conditions of a separate,
 * written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized
 * License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software
 * and all intellectual property rights therein.  IF YOU HAVE
 * NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE
 * IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE
 * ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of
 * Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS
 * PROVIDED "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY,
 * OR OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL,
 * INCIDENTAL, SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER
 * ARISING OUT OF OR IN ANY WAY RELATING TO YOUR USE OF OR INABILITY
 * TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF
 * THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR USD 1.00,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING
 * ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.$
 *
 *
 */

#ifndef EPDM_BASE_T_SEC_H
#define EPDM_BASE_T_SEC_H

#include "epdm_bcm_base_t_common_sec_defines.h"
#include "epdm_bcm_common_defines.h"
#include "epdm_custom_config.h"
#define __PLP_NOT_USED ((void*)0)

/* SECY */
typedef bcm_plp_base_t_secy_status_t (*__bcm_plp_base_t_secy_device_init_f)(bcm_plp_base_t_sec_phy_access_t *pa,
                                 const bcm_plp_base_t_secy_settings_t * const settings_p);

typedef bcm_plp_base_t_secy_status_t (*__bcm_plp_base_t_secy_device_uninit_f)(bcm_plp_base_t_sec_phy_access_t *pa);

typedef bcm_plp_base_t_secy_status_t (*__bcm_plp_base_t_secy_device_limits_f)(bcm_plp_base_t_sec_phy_access_t *pa,
                                 unsigned int * const max_vport_count_p,
                                 unsigned int * const max_rule_count_p,
                                 unsigned int * const max_sa_count_p,
                                 unsigned int * const max_sc_count_p);

typedef bcm_plp_base_t_secy_status_t (*__bcm_plp_base_t_secy_vport_add_f)(bcm_plp_base_t_sec_phy_access_t *pa,
                                 bcm_plp_base_t_secy_vport_handle_t *vport_handle_p);


typedef unsigned char (*__bcm_plp_base_t_secy_vport_handle_issame_f)(bcm_plp_base_t_sec_phy_access_t *pa,
                                 const bcm_plp_base_t_secy_vport_handle_t * const handle1_p,
                                 const bcm_plp_base_t_secy_vport_handle_t * const handle2_p);

typedef bcm_plp_base_t_secy_status_t (*__bcm_plp_base_t_secy_vport_remove_f)(bcm_plp_base_t_sec_phy_access_t *pa,
                                 const bcm_plp_base_t_secy_vport_handle_t vport_handle);

typedef bcm_plp_base_t_secy_status_t (*__bcm_plp_base_t_secy_sa_add_f)(bcm_plp_base_t_sec_phy_access_t *pa,
                                 const bcm_plp_base_t_secy_vport_handle_t vport_handle,
                                 bcm_plp_base_t_secy_sa_handle_t * const sa_handle_p,
                                 const bcm_plp_base_t_secy_sa_t * const sa_p);

typedef bcm_plp_base_t_secy_status_t (*__bcm_plp_base_t_secy_sa_update_f)(bcm_plp_base_t_sec_phy_access_t *pa,
                                 const bcm_plp_base_t_secy_sa_handle_t sa_handle,
                                 const bcm_plp_base_t_secy_sa_t * const sa_p);

typedef bcm_plp_base_t_secy_status_t (*__bcm_plp_base_t_secy_sa_read_f)(bcm_plp_base_t_sec_phy_access_t *pa,
                                 const bcm_plp_base_t_secy_sa_handle_t sa_handle,
                                 const unsigned int word_offset,
                                 const unsigned int word_count,
                                 unsigned int * transform_p);

typedef unsigned char (*__bcm_plp_base_t_secy_sa_handle_issame_f)(bcm_plp_base_t_sec_phy_access_t *pa,
                                 const bcm_plp_base_t_secy_sa_handle_t * const handle1_p,
                                 const bcm_plp_base_t_secy_sa_handle_t * const handle2_p);

typedef unsigned char (*__bcm_plp_base_t_secy_sa_handle_sa_index_issame_f)(bcm_plp_base_t_sec_phy_access_t *pa,
                                 const bcm_plp_base_t_secy_sa_handle_t sa_handle,
                                 const unsigned int sa_index);


typedef bcm_plp_base_t_secy_status_t (*__bcm_plp_base_t_secy_sa_remove_f)(bcm_plp_base_t_sec_phy_access_t *pa,
                                 const bcm_plp_base_t_secy_sa_handle_t sa_handle);

typedef bcm_plp_base_t_secy_status_t (*__bcm_plp_base_t_secy_sa_nextpn_update_f)(bcm_plp_base_t_sec_phy_access_t *pa,
                                 const bcm_plp_base_t_secy_sa_handle_t sa_handle,
                                 const unsigned int nextpn_lo,
                                 const unsigned int nextpn_hi,
                                 unsigned char * const fnextpn_written_p);

typedef bcm_plp_base_t_secy_status_t (*__bcm_plp_base_t_secy_sa_chain_f)(bcm_plp_base_t_sec_phy_access_t *pa,
                                 const bcm_plp_base_t_secy_sa_handle_t active_sa_handle,
                                 bcm_plp_base_t_secy_sa_handle_t * const new_sa_handle_p,
                                 const bcm_plp_base_t_secy_sa_t * const new_sa_p);


typedef bcm_plp_base_t_secy_status_t (*__bcm_plp_base_t_secy_sa_switch_f)(bcm_plp_base_t_sec_phy_access_t *pa,
                                 const bcm_plp_base_t_secy_sa_handle_t active_sa_handle,
                                 const bcm_plp_base_t_secy_sa_handle_t new_sa_handle,
                                 const bcm_plp_base_t_secy_sa_t * const new_sa_p);


typedef bcm_plp_base_t_secy_status_t (*__bcm_plp_base_t_secy_rule_add_f)(bcm_plp_base_t_sec_phy_access_t *pa,
                                 const bcm_plp_base_t_secy_vport_handle_t vport_handle,
                                 bcm_plp_base_t_secy_rule_handle_t *rule_handle_p,
                                 const bcm_plp_base_t_secy_rule_t *rule_p);

typedef bcm_plp_base_t_secy_status_t (*__bcm_plp_base_t_secy_rule_remove_f)(bcm_plp_base_t_sec_phy_access_t *pa,
                                 const bcm_plp_base_t_secy_rule_handle_t rule_handle);

typedef bcm_plp_base_t_secy_status_t (*__bcm_plp_base_t_secy_rule_update_f)(bcm_plp_base_t_sec_phy_access_t *pa,
                                 const bcm_plp_base_t_secy_rule_handle_t rule_handle,
                                 const bcm_plp_base_t_secy_rule_t *rule_p);

typedef bcm_plp_base_t_secy_status_t (*__bcm_plp_base_t_secy_rule_enable_f)(bcm_plp_base_t_sec_phy_access_t *pa,
                                 const bcm_plp_base_t_secy_rule_handle_t rule_handle,
                                 const unsigned char fsync);

typedef unsigned char (*__bcm_plp_base_t_secy_rule_handle_issame_f)(bcm_plp_base_t_sec_phy_access_t *pa,
                                 const bcm_plp_base_t_secy_rule_handle_t * const handle1_p,
                                 const bcm_plp_base_t_secy_rule_handle_t * const handle2_p);

typedef bcm_plp_base_t_secy_status_t (*__bcm_plp_base_t_secy_rule_disable_f)(bcm_plp_base_t_sec_phy_access_t *pa,
                                 const bcm_plp_base_t_secy_rule_handle_t rule_handle,
                                 const unsigned char fsync);

typedef bcm_plp_base_t_secy_status_t (*__bcm_plp_base_t_secy_rule_enable_disable_f)(bcm_plp_base_t_sec_phy_access_t *pa,
                                 const bcm_plp_base_t_secy_rule_handle_t rule_handle_disable,
                                 const bcm_plp_base_t_secy_rule_handle_t rule_handle_enable,
                                 const unsigned char enable_all,
                                 const unsigned char disable_all,
                                 const unsigned char fsync);

typedef bcm_plp_base_t_secy_status_t (*__bcm_plp_base_t_secy_sa_index_get_f)(bcm_plp_base_t_sec_phy_access_t *pa,
                                 const bcm_plp_base_t_secy_sa_handle_t sa_handle,
                                 unsigned int * const sa_index_p,
                                 unsigned int * const sc_index_p);

typedef bcm_plp_base_t_secy_status_t (*__bcm_plp_base_t_secy_rule_index_get_f)(bcm_plp_base_t_sec_phy_access_t *pa,
                                 const bcm_plp_base_t_secy_rule_handle_t rule_handle,
                                 unsigned int * const rule_index_p);

typedef bcm_plp_base_t_secy_status_t (*__bcm_plp_base_t_secy_vport_index_get_f)(bcm_plp_base_t_sec_phy_access_t *pa,
                                 const bcm_plp_base_t_secy_vport_handle_t vport_handle,
                                 unsigned int * const vport_index_p);

typedef bcm_plp_base_t_secy_status_t (*__bcm_plp_base_t_secy_sa_handle_get_f)(bcm_plp_base_t_sec_phy_access_t *pa,
                                 const unsigned int sa_index,
                                 bcm_plp_base_t_secy_sa_handle_t * const sa_handle_p);

typedef bcm_plp_base_t_secy_status_t (*__bcm_plp_base_t_secy_rule_handle_get_f)(bcm_plp_base_t_sec_phy_access_t *pa,
                                 const unsigned int rule_index,
                                 bcm_plp_base_t_secy_rule_handle_t * const rule_handle_p);

typedef bcm_plp_base_t_secy_status_t (*__bcm_plp_base_t_secy_vport_handle_get_f)(bcm_plp_base_t_sec_phy_access_t *pa,
                                 const unsigned int vport_index,
                                 bcm_plp_base_t_secy_vport_handle_t * const vport_handle_p);

typedef bcm_plp_base_t_secy_status_t (*__bcm_plp_base_t_secy_device_update_f)(bcm_plp_base_t_sec_phy_access_t *pa,
                                                              const bcm_plp_base_t_secy_device_params_t  * const device_p );

typedef bcm_plp_base_t_secy_status_t (*__bcm_plp_base_t_secy_crypt_auth_bypass_len_update_f)(bcm_plp_base_t_sec_phy_access_t *pa,
                                                                                              const unsigned int bypass_length);
typedef bcm_plp_base_t_secy_status_t (*__bcm_plp_base_t_secy_rules_mtu_check_update_f)(bcm_plp_base_t_sec_phy_access_t *pa,
                                                                    const unsigned int sci_index,
                                                                    const bcm_plp_base_t_secy_sc_rule_mtu_check_t * const mtucheck_rule_p);

typedef bcm_plp_base_t_secy_status_t (*__bcm_plp_base_t_secy_tcam_statistics_get_f)(bcm_plp_base_t_sec_phy_access_t *pa,
                                                                                const bcm_plp_base_t_secy_rule_handle_t rule_handle,
                                                                                bcm_plp_base_t_secy_tcam_stat_t * const stat_p,
                                                                                const unsigned char fsync);
typedef bcm_plp_base_t_secy_status_t (*__bcm_plp_base_t_secy_rxcam_statistics_get_f)(bcm_plp_base_t_sec_phy_access_t *pa,
                                                                                 const unsigned int scindex,
                                                                                 bcm_plp_base_t_secy_rxcam_stat_t* const stats_p,
                                                                                 const unsigned char fsync);
typedef bcm_plp_base_t_secy_status_t (*__bcm_plp_base_t_secy_secy_statistics_i_get_f)(bcm_plp_base_t_sec_phy_access_t *pa,
                                                                                 bcm_plp_base_t_secy_vport_handle_t vport_handle,
                                                                                 bcm_plp_base_t_secy_secy_stat_i_t * const stats_p,
                                                                                 const unsigned char fsync);

typedef bcm_plp_base_t_secy_status_t (*__bcm_plp_base_t_secy_secy_statistics_e_get_f)(bcm_plp_base_t_sec_phy_access_t *pa,
                                                                                 bcm_plp_base_t_secy_vport_handle_t vport_handle,
                                                                                 bcm_plp_base_t_secy_secy_stat_e_t * const stats_p,
                                                                                 const unsigned char fsync);
typedef bcm_plp_base_t_secy_status_t (*__bcm_plp_base_t_secy_sa_statistics_i_get_f)(bcm_plp_base_t_sec_phy_access_t *pa,
                                                                           const bcm_plp_base_t_secy_sa_handle_t sa_handle,
                                                                           bcm_plp_base_t_secy_sa_stat_i_t * const stats_p,
                                                                           const unsigned char fsync);
typedef bcm_plp_base_t_secy_status_t (*__bcm_plp_base_t_secy_sa_statistics_e_get_f)(bcm_plp_base_t_sec_phy_access_t *pa,
                                                                            const bcm_plp_base_t_secy_sa_handle_t sa_handle,
                                                                            bcm_plp_base_t_secy_sa_stat_e_t * const stats_p,
                                                                            const unsigned char fsync);
typedef bcm_plp_base_t_secy_status_t (*__bcm_plp_base_t_secy_global_statistics_get_f)(bcm_plp_base_t_sec_phy_access_t *pa,
                                                                                 bcm_plp_base_t_secy_global_stat_t * const stat_p,
                                                                                 const unsigned char fsync);
typedef bcm_plp_base_t_secy_status_t (*__bcm_plp_base_t_secy_ifc_statistics_e_get_f)(bcm_plp_base_t_sec_phy_access_t *pa,
                                                                    bcm_plp_base_t_secy_vport_handle_t vport_handle,
                                                                    bcm_plp_base_t_secy_ifc_stat_e_t * const stats_p,
                                                                    const unsigned char fsync);
typedef bcm_plp_base_t_secy_status_t (*__bcm_plp_base_t_secy_ifc_statistics_i_get_f)(bcm_plp_base_t_sec_phy_access_t *pa,
                                                                        bcm_plp_base_t_secy_vport_handle_t vport_handle,
                                                                        bcm_plp_base_t_secy_ifc_stat_i_t * const stats_p,
                                                                        const unsigned char fsync);
typedef bcm_plp_base_t_secy_status_t (*__bcm_plp_base_t_secy_device_count_summary_pifc_checkandclear_f)(bcm_plp_base_t_sec_phy_access_t *pa,
                                                                                                           bcm_plp_base_t_secy_vport_handle_t ** const vport_handle_pp,
                                                                                                           unsigned int * const num_ifc_indexes_p);

typedef bcm_plp_base_t_secy_status_t (*__bcm_plp_base_t_secy_device_count_summary_prxcam_checkandclear_f)(bcm_plp_base_t_sec_phy_access_t *pa,
                                                                                                         unsigned int ** const rx_cam_indexes_pp,
                                                                                                         unsigned int * const num_rx_cam_indexes_p);
typedef bcm_plp_base_t_secy_status_t (*__bcm_plp_base_t_secy_device_count_summary_psa_checkandclear_f)(bcm_plp_base_t_sec_phy_access_t *pa,
                                                                                           bcm_plp_base_t_secy_sa_handle_t ** const sa_handle_pp,
                                                                                           unsigned int * const num_sa_indexes_p);
typedef bcm_plp_base_t_secy_status_t (*__bcm_plp_base_t_secy_device_count_summary_psecy_checkandclear_f)(bcm_plp_base_t_sec_phy_access_t *pa,
                                                                                                                bcm_plp_base_t_secy_vport_handle_t ** const vport_handle_pp,
                                                                                                                unsigned int * const nm_secy_indexes_p);
typedef bcm_plp_base_t_secy_status_t (*__bcm_plp_base_t_secy_device_count_summary_ptcam_checkandclear_f)(bcm_plp_base_t_sec_phy_access_t *pa,
                                                                                 bcm_plp_base_t_secy_rule_handle_t ** const rule_handle_pp,
                                                                                 unsigned int * const num_tcam_indexes_p);
typedef bcm_plp_base_t_secy_status_t (*__bcm_plp_base_t_secy_device_count_summary_sa_checkandclear_f)(bcm_plp_base_t_sec_phy_access_t *pa,
                                                                                    bcm_plp_base_t_secy_sa_handle_t sa_handle,
                                                                                    unsigned int * const count_summary_sa_p);
typedef bcm_plp_base_t_secy_status_t (*__bcm_plp_base_t_secy_device_count_summary_secy_checkandclear_f)(bcm_plp_base_t_sec_phy_access_t *pa,
                                                                                                 bcm_plp_base_t_secy_vport_handle_t vport_handle,
                                                                                                 unsigned int * const count_summary_secy_p);
typedef bcm_plp_base_t_secy_status_t (*__bcm_plp_base_t_secy_device_count_summary_tcam_global_checkandclear_f)(bcm_plp_base_t_sec_phy_access_t *pa,
                                                                                                          unsigned int * const count_summary_tcam_global_p);
typedef bcm_plp_base_t_secy_status_t (*__bcm_plp_base_t_secy_device_count_summary_ifc_checkandclear_f)(bcm_plp_base_t_sec_phy_access_t *pa,
                                                                                           bcm_plp_base_t_secy_vport_handle_t vport_handle,
                                                                                           unsigned int * const count_summary_ifc_p);
typedef bcm_plp_base_t_secy_status_t (*__bcm_plp_base_t_secy_sa_expired_summary_checkandclear_f)(bcm_plp_base_t_sec_phy_access_t *pa,
                                                                                      bcm_plp_base_t_secy_sa_handle_t ** const sa_handle_pp,
                                                                                      unsigned int * const num_sa_indexes_p);
typedef bcm_plp_base_t_secy_status_t (*__bcm_plp_base_t_secy_sa_pnthr_summary_checkandclear_f)(bcm_plp_base_t_sec_phy_access_t *pa,
                                                                                       bcm_plp_base_t_secy_sa_handle_t ** const sa_handle_pp,
                                                                                       unsigned int * const num_sa_indexes_p);
typedef bcm_plp_base_t_secy_status_t (*__bcm_plp_base_t_secy_intr_enable_set_f)(bcm_plp_base_t_sec_phy_access_t *pa, const bcm_plp_base_t_secy_intr_t *secy_intr_p);
typedef bcm_plp_base_t_secy_status_t (*__bcm_plp_base_t_secy_intr_enable_get_f)(bcm_plp_base_t_sec_phy_access_t *pa, bcm_plp_base_t_secy_intr_t *secy_intr_p);
typedef bcm_plp_base_t_secy_status_t (*__bcm_plp_base_t_secy_intr_status_get_f)(bcm_plp_base_t_sec_phy_access_t *pa, bcm_plp_base_t_secy_intr_t *secy_intr_p);
typedef bcm_plp_base_t_secy_status_t (*__bcm_plp_base_t_secy_intr_status_clear_f)(bcm_plp_base_t_sec_phy_access_t *pa, const bcm_plp_base_t_secy_intr_t *secy_intr_p);
typedef bcm_plp_base_t_secy_status_t (*__bcm_plp_base_t_secy_event_status_get_f)(bcm_plp_base_t_sec_phy_access_t *pa, bcm_plp_base_t_secy_intr_t *secy_intr_p);
typedef bcm_plp_base_t_secy_status_t (*__bcm_plp_base_t_secy_intr_set_f)(bcm_plp_base_t_sec_phy_access_t *pa, const bcm_plp_base_t_secy_intr_t *secy_intr, const unsigned int enable);
typedef bcm_plp_base_t_secy_status_t (*__bcm_plp_base_t_secy_intr_get_f)(bcm_plp_base_t_sec_phy_access_t *pa, bcm_plp_base_t_secy_intr_t *secy_intr, unsigned int *enable);
typedef bcm_plp_base_t_sa_builder_status_t (*__bcm_plp_base_t_secy_build_transform_record_f)(bcm_plp_base_t_sec_phy_access_t *pa, bcm_plp_base_t_sa_builder_params_t *params, unsigned int *sa_buffer_p);

typedef bcm_plp_base_t_secy_status_t (*__bcm_plp_base_t_secy_sa_active_e_get_f)(bcm_plp_base_t_sec_phy_access_t *pa,
                                  const unsigned int vport,
                                  bcm_plp_base_t_secy_sa_handle_t * const active_sa_handle_p);
typedef bcm_plp_base_t_secy_status_t (*__bcm_plp_base_t_secy_sa_active_i_get_f)(bcm_plp_base_t_sec_phy_access_t *pa,
                                  const unsigned int vport,
                                  const unsigned char * const sci_p,
                                  bcm_plp_base_t_secy_sa_handle_t * const active_sa_handle_p);
typedef bcm_plp_base_t_secy_status_t (*__bcm_plp_base_t_secy_sa_window_size_update_f)(bcm_plp_base_t_sec_phy_access_t *pa,
                                                                                      const bcm_plp_base_t_secy_sa_handle_t sa_handle,
                                                                                      const unsigned int window_size);

typedef int (*__bcm_plp_base_t_imacsec_version_get_f)(bcm_plp_base_t_sec_phy_access_t *pa, bcm_plp_base_t_version_t* version_info);

typedef bcm_plp_base_t_secy_status_t (*__bcm_base_t_secy_bypass_set_f)(bcm_plp_base_t_sec_phy_access_t *pa, const unsigned char fbypass);

typedef bcm_plp_base_t_secy_status_t (*__bcm_base_t_secy_bypass_get_f)(bcm_plp_base_t_sec_phy_access_t *pa, unsigned char *fbypass);

typedef int (*__bcm_base_t_mac_reset_f)(bcm_plp_mac_access_t mac_info, int reset);

typedef int (*__bcm_base_t_mac_pause_set_f)(bcm_plp_mac_access_t mac_info, int pause);

typedef int (*__bcm_base_t_mac_datapath_flush_f)(bcm_plp_mac_access_t mac_info, int enable_disable);

typedef int (*__bcm_base_t_mac_port_enable_f)(bcm_plp_mac_access_t mac_info, int tx_rx, int enable_disable);
/* MAC */
/* For future use */

typedef struct __plp__base__t__sec__dispatch__s__ {
    __bcm_plp_base_t_secy_device_init_f __secy_device_init;
    __bcm_plp_base_t_secy_device_uninit_f __secy_device_uninit;
    __bcm_plp_base_t_secy_device_limits_f __secy_device_limits;
    __bcm_plp_base_t_secy_vport_add_f __secy_vport_add;
    __bcm_plp_base_t_secy_vport_handle_issame_f __secy_vport_handle_issame;
    __bcm_plp_base_t_secy_vport_remove_f __secy_vport_remove;
    __bcm_plp_base_t_secy_sa_add_f __secy_sa_add;
    __bcm_plp_base_t_secy_sa_update_f __secy_sa_update;
    __bcm_plp_base_t_secy_sa_read_f __secy_sa_read;
    __bcm_plp_base_t_secy_sa_handle_issame_f __secy_sa_handle_issame;
    __bcm_plp_base_t_secy_sa_handle_sa_index_issame_f __secy_sa_handle_sa_index_issame;
    __bcm_plp_base_t_secy_sa_remove_f __secy_sa_remove;
    __bcm_plp_base_t_secy_sa_nextpn_update_f __secy_sa_nextpn_update;
    __bcm_plp_base_t_secy_sa_chain_f __secy_sa_chain;
    __bcm_plp_base_t_secy_sa_switch_f __secy_sa_switch;
    __bcm_plp_base_t_secy_rule_add_f __secy_rule_add;
    __bcm_plp_base_t_secy_rule_update_f __secy_rule_update;
    __bcm_plp_base_t_secy_rule_enable_f __secy_rule_enable;
    __bcm_plp_base_t_secy_rule_enable_disable_f __secy_rule_enable_disable;
    __bcm_plp_base_t_secy_rule_handle_issame_f __secy_rule_handle_issame;
    __bcm_plp_base_t_secy_rule_disable_f __secy_rule_disable;
    __bcm_plp_base_t_secy_rule_remove_f __secy_rule_remove;
    __bcm_plp_base_t_secy_sa_index_get_f __secy_sa_index_get;
    __bcm_plp_base_t_secy_rule_index_get_f __secy_rule_index_get;
    __bcm_plp_base_t_secy_vport_index_get_f __secy_vport_index_get;
    __bcm_plp_base_t_secy_sa_handle_get_f __secy_sa_handle_get;
    __bcm_plp_base_t_secy_rule_handle_get_f __secy_rule_handle_get;
    __bcm_plp_base_t_secy_vport_handle_get_f __secy_vport_handle_get;
    __bcm_plp_base_t_secy_device_update_f __secy_device_update;
    __bcm_plp_base_t_secy_crypt_auth_bypass_len_update_f __secy_crypt_auth_bypass_len_update;
    __bcm_plp_base_t_secy_rules_mtu_check_update_f __secy_rules_mtu_check_update;
    __bcm_plp_base_t_secy_tcam_statistics_get_f __secy_tcam_statistics_get;
    __bcm_plp_base_t_secy_rxcam_statistics_get_f __secy_rxcam_statistics_get;
    __bcm_plp_base_t_secy_secy_statistics_i_get_f __secy_secy_statistics_i_get;
    __bcm_plp_base_t_secy_secy_statistics_e_get_f __secy_secy_statistics_e_get;
    __bcm_plp_base_t_secy_sa_statistics_i_get_f __secy_sa_statistics_i_get;
    __bcm_plp_base_t_secy_sa_statistics_e_get_f __secy_sa_statistics_e_get;
    __bcm_plp_base_t_secy_global_statistics_get_f __secy_global_statistics_get;
    __bcm_plp_base_t_secy_ifc_statistics_e_get_f __secy_ifc_statistics_e_get;
    __bcm_plp_base_t_secy_ifc_statistics_i_get_f __secy_ifc_statistics_i_get;
    __bcm_plp_base_t_secy_device_count_summary_pifc_checkandclear_f __secy_device_count_summary_pifc_checkandclear;
    __bcm_plp_base_t_secy_device_count_summary_prxcam_checkandclear_f __secy_device_count_summary_prxcam_checkandclear;
    __bcm_plp_base_t_secy_device_count_summary_psa_checkandclear_f __secy_device_count_summary_psa_checkandclear;
    __bcm_plp_base_t_secy_device_count_summary_psecy_checkandclear_f __secy_device_count_summary_psecy_checkandclear;
    __bcm_plp_base_t_secy_device_count_summary_ptcam_checkandclear_f __secy_device_count_summary_ptcam_checkandclear;
    __bcm_plp_base_t_secy_device_count_summary_sa_checkandclear_f __secy_device_count_summary_sa_checkandclear;
    __bcm_plp_base_t_secy_device_count_summary_secy_checkandclear_f __secy_device_count_summary_secy_checkandclear;
    __bcm_plp_base_t_secy_device_count_summary_tcam_global_checkandclear_f __secy_device_count_summary_tcam_global_checkandclear;
    __bcm_plp_base_t_secy_device_count_summary_ifc_checkandclear_f __secy_device_count_summary_ifc_checkandclear;
    __bcm_plp_base_t_secy_sa_expired_summary_checkandclear_f __secy_sa_expired_summary_checkandclear;
    __bcm_plp_base_t_secy_sa_pnthr_summary_checkandclear_f __secy_sa_pnthr_summary_checkandclear;
    __bcm_plp_base_t_secy_intr_enable_set_f __secy_intr_enable_set;
    __bcm_plp_base_t_secy_intr_enable_get_f __secy_intr_enable_get;
    __bcm_plp_base_t_secy_intr_status_get_f __secy_intr_status_get;
    __bcm_plp_base_t_secy_intr_status_clear_f __secy_intr_status_clear;
    __bcm_plp_base_t_secy_event_status_get_f __secy_event_status_get;
    __bcm_plp_base_t_secy_intr_set_f             __secy_intr_set;
    __bcm_plp_base_t_secy_intr_get_f             __secy_intr_get;
    __bcm_plp_base_t_secy_build_transform_record_f __secy_build_transform_record;
    __bcm_plp_base_t_secy_sa_active_e_get_f __secy_sa_active_e_get;
    __bcm_plp_base_t_secy_sa_active_i_get_f __secy_sa_active_i_get;
    __bcm_plp_base_t_imacsec_version_get_f __imacsec_version_get;
    __bcm_plp_base_t_secy_sa_window_size_update_f __secy_sa_window_size_update;
    __bcm_base_t_secy_bypass_set_f __secy_bypass_set;
    __bcm_base_t_secy_bypass_get_f __secy_bypass_get;
    void* reserved16;
    void* reserved17;
    void* reserved18;
    void* reserved19;
    void* reserved20;
    __bcm_base_t_mac_reset_f __mac_reset;
    __bcm_base_t_mac_pause_set_f __mac_pause_set;
    __bcm_base_t_mac_datapath_flush_f __mac_datapath_flush;
    __bcm_base_t_mac_port_enable_f __mac_port_enable;
} __plp__base__t__sec__dispatch__t__;

/* SECY */
#define  bcm_plp_base_t_secy_device_init(_pd, _a, _b) \
    PLP_SEC_CALL((_pd), __secy_device_init, ((_a), (_b)))

#define  bcm_plp_base_t_secy_device_uninit(_pd, _a) \
    PLP_SEC_CALL((_pd), __secy_device_uninit, ((_a)))

#define  bcm_plp_base_t_secy_device_limits(_pd, _a, _b, _c, _d, _e) \
        PLP_SEC_CALL((_pd), __secy_device_limits, ((_a), (_b), (_c), (_d), (_e)))

#define bcm_plp_base_t_secy_vport_add(_pd, _a, _b) \
         PLP_SEC_CALL((_pd), __secy_vport_add, ((_a), (_b)))

#define bcm_plp_base_t_secy_vport_handle_issame(_pd, _a, _b, _c) \
         PLP_SEC_CALL((_pd), __secy_vport_handle_issame, ((_a), (_b), (_c)))
#define bcm_plp_base_t_secy_vport_remove(_pd, _a, _b) \
         PLP_SEC_CALL((_pd), __secy_vport_remove, ((_a), (_b)))

#define  bcm_plp_base_t_secy_sa_add(_pd, _a, _b, _c, _d) \
    PLP_SEC_CALL((_pd), __secy_sa_add, ((_a), (_b), (_c), (_d)))

#define  bcm_plp_base_t_secy_sa_update(_pd, _a, _b, _c) \
    PLP_SEC_CALL((_pd), __secy_sa_update, ((_a), (_b), (_c)))

#define  bcm_plp_base_t_secy_sa_read(_pd, _a, _b, _c, _d, _e) \
    PLP_SEC_CALL((_pd), __secy_sa_read, ((_a), (_b), (_c), (_d), (_e)))
#define  bcm_plp_base_t_secy_sa_handle_issame(_pd, _a, _b, _c) \
            PLP_SEC_CALL((_pd), __secy_sa_handle_issame, ((_a), (_b), (_c)))

#define  bcm_plp_base_t_secy_sa_handle_sa_index_issame(_pd, _a, _b, _c) \
            PLP_SEC_CALL((_pd), __secy_sa_handle_sa_index_issame, ((_a), (_b), (_c)))

#define  bcm_plp_base_t_secy_sa_remove(_pd, _a, _b) \
    PLP_SEC_CALL((_pd),  __secy_sa_remove, ((_a), (_b)))
#define bcm_plp_base_t_secy_sa_nextpn_update(_pd, _a, _b, _c, _d, _e) \
    PLP_SEC_CALL((_pd), __secy_sa_nextpn_update, ((_a), (_b), (_c), (_d), (_e)))

#define  bcm_plp_base_t_secy_sa_chain(_pd, _a, _b, _c, _d) \
        PLP_SEC_CALL((_pd), __secy_sa_chain, ((_a), (_b), (_c), (_d)))

#define  bcm_plp_base_t_secy_sa_switch(_pd, _a, _b, _c, _d) \
        PLP_SEC_CALL((_pd), __secy_sa_switch, ((_a), (_b), (_c), (_d)))

#define bcm_plp_base_t_secy_rule_add(_pd, _a, _b, _c, _d) \
        PLP_SEC_CALL((_pd), __secy_rule_add, ((_a), (_b), (_c), (_d)))

#define bcm_plp_base_t_secy_rule_update(_pd, _a, _b, _c) \
        PLP_SEC_CALL((_pd), __secy_rule_update, ((_a), (_b), (_c)))
#define bcm_plp_base_t_secy_rule_enable(_pd, _a, _b, _c) \
        PLP_SEC_CALL((_pd), __secy_rule_enable, ((_a), (_b), (_c)))
#define bcm_plp_base_t_secy_rule_enable_disable(_pd, _a, _b, _c, _d, _e, _f) \
        PLP_SEC_CALL((_pd), __secy_rule_enable_disable, ((_a), (_b), (_c), (_d), (_e), (_f)))
#define bcm_plp_base_t_secy_rule_handle_issame(_pd, _a, _b, _c) \
        PLP_SEC_CALL((_pd), __secy_rule_handle_issame, ((_a), (_b), (_c)))
#define bcm_plp_base_t_secy_rule_disable(_pd, _a, _b, _c) \
        PLP_SEC_CALL((_pd), __secy_rule_disable, ((_a), (_b), (_c)))

#define bcm_plp_base_t_secy_rule_remove(_pd, _a, _b) \
        PLP_SEC_CALL((_pd), __secy_rule_remove, ((_a), (_b)))

#define bcm_plp_base_t_secy_sa_index_get(_pd, _a, _b, _c, _d) \
        PLP_SEC_CALL((_pd), __secy_sa_index_get, ((_a), (_b), (_c), (_d)))

#define bcm_plp_base_t_secy_rule_index_get(_pd, _a, _b, _c) \
        PLP_SEC_CALL((_pd), __secy_rule_index_get, ((_a), (_b), (_c)))
#define bcm_plp_base_t_secy_vport_index_get(_pd, _a, _b, _c) \
        PLP_SEC_CALL((_pd), __secy_vport_index_get, ((_a), (_b), (_c)))

#define bcm_plp_base_t_secy_sa_handle_get(_pd, _a, _b, _c) \
        PLP_SEC_CALL((_pd), __secy_sa_handle_get, ((_a), (_b), (_c)))

#define bcm_plp_base_t_secy_rule_handle_get(_pd, _a, _b, _c) \
        PLP_SEC_CALL((_pd), __secy_rule_handle_get, ((_a), (_b), (_c)))
#define bcm_plp_base_t_secy_vport_handle_get(_pd, _a, _b, _c) \
        PLP_SEC_CALL((_pd), __secy_vport_handle_get, ((_a), (_b), (_c)))

#define bcm_plp_base_t_secy_device_update(_pd, _a, _b) \
    PLP_SEC_CALL((_pd), __secy_device_update, ((_a), (_b)))

#define  bcm_plp_base_t_secy_crypt_auth_bypass_len_update(_pd, _a, _b) \
        PLP_SEC_CALL((_pd), __secy_crypt_auth_bypass_len_update, ((_a), (_b)))

#define bcm_plp_base_t_secy_rules_mtu_check_update(_pd, _a, _b, _c) \
        PLP_SEC_CALL((_pd), __secy_rules_mtu_check_update, ((_a), (_b), (_c)))
#define bcm_plp_base_t_secy_tcam_statistics_get(_pd, _a, _b, _c, _d) \
        PLP_SEC_CALL((_pd), __secy_tcam_statistics_get, ((_a), (_b), (_c), (_d)))

#define  bcm_plp_base_t_secy_rxcam_statistics_get(_pd, _a, _b, _c, _d) \
    PLP_SEC_CALL((_pd), __secy_rxcam_statistics_get, ((_a), (_b), (_c), (_d)))

#define  bcm_plp_base_t_secy_secy_statistics_i_get(_pd, _a, _b, _c, _d) \
    PLP_SEC_CALL((_pd), __secy_secy_statistics_i_get, ((_a), (_b), (_c), (_d)))

#define  bcm_plp_base_t_secy_secy_statistics_e_get(_pd, _a, _b, _c, _d) \
        PLP_SEC_CALL((_pd), __secy_secy_statistics_e_get, ((_a), (_b), (_c), (_d)))

#define  bcm_plp_base_t_secy_sa_statistics_i_get(_pd, _a, _b, _c, _d) \
    PLP_SEC_CALL((_pd), __secy_sa_statistics_i_get, ((_a), (_b), (_c), (_d)))

#define  bcm_plp_base_t_secy_sa_statistics_e_get(_pd, _a, _b, _c, _d) \
    PLP_SEC_CALL((_pd), __secy_sa_statistics_e_get, ((_a), (_b), (_c), (_d)))
#define bcm_plp_base_t_secy_global_statistics_get(_pd, _a, _b, _c) \
    PLP_SEC_CALL((_pd), __secy_global_statistics_get, ((_a), (_b), (_c)))

#define  bcm_plp_base_t_secy_ifc_statistics_e_get(_pd, _a, _b, _c, _d) \
    PLP_SEC_CALL((_pd), __secy_ifc_statistics_e_get, ((_a), (_b), (_c), (_d)))

#define  bcm_plp_base_t_secy_ifc_statistics_i_get(_pd, _a, _b, _c, _d) \
    PLP_SEC_CALL((_pd), __secy_ifc_statistics_i_get, ((_a), (_b), (_c), (_d)))


#define  bcm_plp_base_t_secy_device_count_summary_secy_checkandclear(_pd, _a, _b, _c) \
    PLP_SEC_CALL((_pd), __secy_device_count_summary_secy_checkandclear, ((_a), (_b), (_c)))

#define  bcm_plp_base_t_secy_device_count_summary_pifc_checkandclear(_pd, _a, _b, _c) \
    PLP_SEC_CALL((_pd), __secy_device_count_summary_pifc_checkandclear, ((_a), (_b), (_c)))

#define  bcm_plp_base_t_secy_device_count_summary_ifc_checkandclear(_pd, _a, _b, _c) \
    PLP_SEC_CALL((_pd), __secy_device_count_summary_ifc_checkandclear, ((_a), (_b), (_c)))

#define  bcm_plp_base_t_secy_device_count_summary_prxcam_checkandclear(_pd, _a, _b, _c) \
    PLP_SEC_CALL((_pd), __secy_device_count_summary_prxcam_checkandclear, ((_a), (_b), (_c)))

#define bcm_plp_base_t_secy_device_count_summary_ptcam_checkandclear(_pd, _a, _b, _c) \
    PLP_SEC_CALL((_pd), __secy_device_count_summary_ptcam_checkandclear, ((_a), (_b), (_c)))

#define  bcm_plp_base_t_secy_device_count_summary_psa_checkandclear(_pd, _a, _b, _c) \
    PLP_SEC_CALL((_pd), __secy_device_count_summary_psa_checkandclear, ((_a), (_b), (_c)))

#define  bcm_plp_base_t_secy_device_count_summary_sa_checkandclear(_pd, _a, _b, _c) \
    PLP_SEC_CALL((_pd), __secy_device_count_summary_sa_checkandclear, ((_a), (_b), (_c)))

#define  bcm_plp_base_t_secy_sa_pnthr_summary_checkandclear(_pd, _a, _b, _c) \
    PLP_SEC_CALL((_pd), __secy_sa_pnthr_summary_checkandclear, ((_a), (_b), (_c)))

#define  bcm_plp_base_t_secy_sa_expired_summary_checkandclear(_pd, _a, _b, _c) \
    PLP_SEC_CALL((_pd), __secy_sa_expired_summary_checkandclear, ((_a), (_b), (_c)))

#define bcm_plp_base_t_secy_device_count_summary_tcam_global_checkandclear(_pd, _a, _b) \
    PLP_SEC_CALL((_pd), __secy_device_count_summary_tcam_global_checkandclear, ((_a), (_b)))

#define  bcm_plp_base_t_secy_device_count_summary_psecy_checkandclear(_pd, _a, _b, _c) \
    PLP_SEC_CALL((_pd), __secy_device_count_summary_psecy_checkandclear, ((_a), (_b), (_c)))

#define bcm_plp_base_t_secy_intr_enable_set(_pd, _a, _b) \
    PLP_SEC_CALL((_pd), __secy_intr_enable_set, ((_a), (_b)))

#define bcm_plp_base_t_secy_intr_enable_get(_pd, _a, _b) \
    PLP_SEC_CALL((_pd), __secy_intr_enable_get, ((_a), (_b)))

#define bcm_plp_base_t_secy_intr_status_get(_pd, _a, _b) \
    PLP_SEC_CALL((_pd), __secy_intr_status_get, ((_a), (_b)))

#define bcm_plp_base_t_secy_intr_status_clear(_pd, _a, _b) \
    PLP_SEC_CALL((_pd), __secy_intr_status_clear, ((_a), (_b)))

#define bcm_plp_base_t_secy_event_status_get(_pd, _a, _b) \
    PLP_SEC_CALL((_pd), __secy_event_status_get, ((_a), (_b)))

#define  bcm_plp_base_t_secy_intr_set(_pd, _a, _b, _c) \
    PLP_SEC_CALL((_pd), __secy_intr_set, ((_a), (_b), (_c)))
#define  bcm_plp_base_t_secy_intr_get(_pd, _a, _b, _c) \
    PLP_SEC_CALL((_pd), __secy_intr_get, ((_a), (_b), (_c)))

#define bcm_plp_base_t_secy_build_transform_record(_pd, _a, _b, _c) \
    PLP_SEC_CALL((_pd), __secy_build_transform_record, ((_a), (_b), (_c)))
#define bcm_plp_base_t_secy_sa_active_e_get(_pd, _a, _b, _c) \
    PLP_SEC_CALL((_pd), __secy_sa_active_e_get, ((_a), (_b), (_c)))

#define bcm_plp_base_t_secy_sa_active_i_get(_pd, _a, _b, _c, _d) \
    PLP_SEC_CALL((_pd), __secy_sa_active_i_get, ((_a), (_b), (_c), (_d)))
#define bcm_plp_base_t_secy_sa_window_size_update(_pd, _a, _b, _c) \
    PLP_SEC_CALL((_pd), __secy_sa_window_size_update, ((_a), (_b), (_c)))
#define bcm_plp_base_t_imacsec_version_get(_pd, _a, _b) \
    PLP_SEC_CALL((_pd), __imacsec_version_get, ((_a), (_b)))
#define bcm_plp_base_t_secy_bypass_set(_pd, _a, _b) \
    PLP_SEC_CALL((_pd), __secy_bypass_set, ((_a), (_b)))
#define bcm_plp_base_t_secy_bypass_get(_pd, _a, _b) \
    PLP_SEC_CALL((_pd), __secy_bypass_get, ((_a), (_b)))
/* MAC */
#define bcm_plp_base_t_mac_reset(_pd, _a, _b) \
    PLP_SEC_CALL((_pd), __mac_reset, ((_a), (_b)))
#define bcm_plp_base_t_mac_pause_set(_pd, _a, _b) \
    PLP_SEC_CALL((_pd), __mac_pause_set, ((_a), (_b)))
#define bcm_plp_base_t_mac_datapath_flush(_pd, _a, _b) \
    PLP_SEC_CALL((_pd), __mac_datapath_flush, ((_a), (_b)))
#define bcm_plp_base_t_mac_port_enable(_pd, _a, _b, _c) \
    PLP_SEC_CALL((_pd), __mac_port_enable, ((_a), (_b), (_c)))

extern __plp__base__t__sec__dispatch__t__ plp_seahawks_sec_dispatch;

#define _PLP_FALSE (0)

#define PLP_CHECK_PF(__pf, __pa) ((__pf == __PLP_NOT_USED) ? _PLP_API_UNAVAILABLE : (__pf __pa))

#if (defined(PLP_MACSEC_SUPPORT)) && (defined(PLP_SEAHAWKS_SUPPORT))
    #define PLP_SEAHAWKS_SEC_COMPARE(_N, _pf, _pa) (strcmp(_N, "seahawks")) == 0 ? PLP_CHECK_PF(plp_seahawks_sec_dispatch._pf, _pa)
#else
    #define PLP_SEAHAWKS_SEC_COMPARE(_N, _pf, _pa) _PLP_FALSE ? _PLP_FALSE
#endif

#define PLP_SEC_CALL(_N, _pf, _pa) (PLP_SEAHAWKS_SEC_COMPARE(_N, _pf, _pa)  :\
                                    _PLP_API_UNAVAILABLE)

#endif /* EPDM_SEC_H */
