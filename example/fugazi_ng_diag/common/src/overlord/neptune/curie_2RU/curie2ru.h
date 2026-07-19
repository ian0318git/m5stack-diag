/* $Id: curie2ru.h,v 1.2 2021/01/11 11:06:06 xiaolaya Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_2RU/curie2ru.h,v $
 *------------------------------------------------------------------
 *
 * curie2ru.h - Curie2ru definitions.
 *
 * Dec. 2018, Nocken Zou <yozou@cisco.com>
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __CURIE2RU_H__
#define __CURIE2RU_H__

#define _XOPEN_SOURCE 500

#include <stdint.h>
#include <unistd.h>
#include <linux/netlink.h>
#include <linux/genetlink.h>
#include "types.h"

#ifndef PLP_MIURA_SUPPORT
#define PLP_MIURA_SUPPORT
#endif
#ifndef PLP_MACSEC_SUPPORT
#define PLP_MACSEC_SUPPORT
#endif
#ifndef PLP_QUADRA28_SUPPORT
#define PLP_QUADRA28_SUPPORT
#endif

#define BCM_PLP_BASE_T_PHY  1

#include "linux_pci.h"
#include "curie2ru_common.h"
#include "curie2ru_bnxt.h"
#include "curie2ru_miura.h"
#include "curie2ru_quadra28.h"

#define BUF_SIZE    4096
#define CURIE2RU_PORT_SPEED_1G  1000
#define CURIE2RU_PORT_SPEED_10G 10000

typedef enum {
    CURIE2RU_IF_SIDE_LINE,
    CURIE2RU_IF_SIDE_SYS,

    MAX_NR_CURIE2RU_IF_SIDE
} curie2ru_if_side_t;

typedef enum {
    CURIE2RU_LANE_0,
    CURIE2RU_LANE_1,

    MAX_NR_CURIE2RU_LANE
} curie2ru_lane_t;

typedef enum {
    CURIE2RU_LED_OFF,
    CURIE2RU_LED_GREEN,
    CURIE2RU_LED_RED,
    CURIE2RU_LED_AMBER,
    CURIE2RU_LED_YELLOW,
    CURIE2RU_LED_BLUE,
    MAX_NR_CURIE2RU_LED
} curie2ru_led_t;

typedef enum {
    CURIE2RU_VMARG_NORMAL,
    CURIE2RU_VMARG_HIGH,
    CURIE2RU_VMARG_LOW,
    MAX_NR_CURIE2RU_VMARG
} curie2ru_vmarg_t;

typedef enum {
    CURIE2RU_PRBS_7,
    CURIE2RU_PRBS_9,
    CURIE2RU_PRBS_11,
    CURIE2RU_PRBS_15,
    CURIE2RU_PRBS_23,
    CURIE2RU_PRBS_31,
    MAX_NR_CURIE2RU_PRBS
} curie2ru_prbs_t;

#define NR_CURIE2RU_BNXT    2

struct curie2ru {
    struct curie2ru_bnxt bnxt[NR_CURIE2RU_BNXT];
    struct curie2ru_miura miura;
    struct curie2ru_quadra28 quadra28[NR_CURIE2RU_BNXT];
};

enum {
        BNXT_CMD_UNSPEC,
        BNXT_CMD_HWRM,
        BNXT_NUM_CMDS
};

#define HWRM_PORT_SFP_SIDEBAND_CFG      0xd6UL
/* attributes */
enum {
        BNXT_ATTR_UNSPEC,
        BNXT_ATTR_PID,
        BNXT_ATTR_IF_INDEX,
        BNXT_ATTR_REQUEST,
        BNXT_ATTR_RESPONSE,
        BNXT_NUM_ATTRS
};

typedef uint8_t u8;

/* BCM82752 band side regiter */
#define OPT_CONF_STAT_REG 0xc8e4
#define OPT_CONF_CTRL_REG 0xc800
#define OPT_CONF_CTRL_VAL 0x383f
#define RX_LOS_STATUS_MASK           0x40
#define TX_FLT_STATUS_MASK           0x20
#define MOD_ABS_STATUS_MASK          0x8

/* BCM82757 band side regiter */
#define PORT_0_OPT_RX_LOS_REG         0x8a5f
#define PORT_0_OPT_TX_FAULT_REG       0x8a67
#define PORT_0_OPT_MOD_ABS_REG        0x8a6f
#define PORT_1_OPT_RX_LOS_REG         0x8a61
#define PORT_1_OPT_TX_FAULT_REG       0x8a69
#define PORT_1_OPT_MOD_ABS_REG        0x8a71
#define RX_LOS_MASK           0x4
#define TX_FLT_MASK           0x4
#define MOD_ABS_MASK          0x4

#define PIN_ACERT_VALUE             0x4
#define SIDEBAND_ASSERT_TIME    500
#define SIDEBAND_TIMEOUT        30
#define BCM57412_SFP_PORT               12
#define DEV_IFINDEX_PATH_SIZE           64
#define BNXT_NL_NAME "bnxt_netlink"
#define BNXT_BUF_MAX 256
#define SYS_IFINDEX_PATH "/sys/class/net/%s/ifindex"
/* hwrm_port_sfp_sideband_cfg_input (size:256b/32B) */
typedef struct hwrm_port_sfp_sideband_cfg_input_ {
        __le16  req_type;
        __le16  cmpl_ring;
        __le16  seq_id;
        __le16  target_id;
        __le64  resp_addr;
        __le16  port_id;
        u8      unused_0[6];
        __le32  enables;
        #define PORT_SFP_SIDEBAND_CFG_REQ_ENABLES_RS0         0x1UL
        #define PORT_SFP_SIDEBAND_CFG_REQ_ENABLES_RS1         0x2UL
        #define PORT_SFP_SIDEBAND_CFG_REQ_ENABLES_TX_DIS      0x4UL
        #define PORT_SFP_SIDEBAND_CFG_REQ_ENABLES_MOD_SEL     0x8UL
        #define PORT_SFP_SIDEBAND_CFG_REQ_ENABLES_RESET_L     0x10UL
        #define PORT_SFP_SIDEBAND_CFG_REQ_ENABLES_LP_MODE     0x20UL
        __le32  flags;
        #define PORT_SFP_SIDEBAND_CFG_REQ_FLAGS_RS0         0x1UL
        #define PORT_SFP_SIDEBAND_CFG_REQ_FLAGS_RS1         0x2UL
        #define PORT_SFP_SIDEBAND_CFG_REQ_FLAGS_TX_DIS      0x4UL
        #define PORT_SFP_SIDEBAND_CFG_REQ_FLAGS_MOD_SEL     0x8UL
        #define PORT_SFP_SIDEBAND_CFG_REQ_FLAGS_RESET_L     0x10UL
        #define PORT_SFP_SIDEBAND_CFG_REQ_FLAGS_LP_MODE     0x20UL
} hwrm_port_sfp_sideband_cfg_input_t;

/* hwrm_port_sfp_sideband_cfg_output (size:128b/16B) */
typedef struct hwrm_port_sfp_sideband_cfg_output_ {
        __le16  error_code;
        __le16  req_type;
        __le16  seq_id;
        __le16  resp_len;
        u8      unused[7];
        u8      valid;
} hwrm_port_sfp_sideband_cfg_output_t;

typedef struct bcm_nl_request_msg_ {
    struct nlmsghdr n;
    struct genlmsghdr g;
    char buf[BNXT_BUF_MAX];
} bcm_nl_request_msg_t;

int curie2ru_init(struct curie2ru *curie);
void curie2ru_exit(struct curie2ru *curie);

int curie2ru_bcm82757_read(struct curie2ru *curie,
                           curie2ru_lane_t lane, curie2ru_if_side_t if_side,
                           uint32_t devaddr, uint32_t regaddr, uint32_t *data);
int curie2ru_bcm82757_write(struct curie2ru *curie,
                            curie2ru_lane_t lane, curie2ru_if_side_t if_side,
                            uint32_t devaddr, uint32_t regaddr, uint32_t data);
int curie2ru_bcm82757_tx_lane_control_set(struct curie2ru *curie,
                            curie2ru_lane_t lane, int tx_ctrl);
int curie2ru_bcm82757_rx_lane_control_set(struct curie2ru *curie,
                            curie2ru_lane_t lane, int rx_ctrl);
int curie2ru_bcm82757_regs_dump(struct curie2ru *curie, curie2ru_lane_t lane);
int curie2ru_bcm82757_dump(struct curie2ru *curie,
                           curie2ru_lane_t lane, curie2ru_if_side_t if_side);
int curie2ru_bcm82757_mac_dump(struct curie2ru *curie,
                               curie2ru_lane_t lane, curie2ru_if_side_t if_side);
int curie2ru_bcm82757_link_status(struct curie2ru *curie,
                                  curie2ru_lane_t lane, curie2ru_if_side_t if_side,
                                  unsigned int *link_status);
int curie2ru_bcm82757_display_eye_scan(struct curie2ru *curie,
                                       curie2ru_lane_t lane, curie2ru_if_side_t if_side);
int curie2ru_bcm82757_rx_get(struct curie2ru *curie,
                             curie2ru_lane_t lane, bcm_plp_rx_t* rx, curie2ru_if_side_t if_side);
int curie2ru_bcm82757_rx_set(struct curie2ru *curie,
                             curie2ru_lane_t lane, bcm_plp_rx_t rx, curie2ru_if_side_t if_side);

int curie2ru_bcm82757_loopback_set(struct curie2ru *curie,
                                   curie2ru_lane_t lane, curie2ru_if_side_t if_side,
                                   unsigned int lb_mode, unsigned int enable);

int curie2ru_bcm82757_prbs_set(struct curie2ru *curie,
                               curie2ru_lane_t lane, curie2ru_if_side_t if_side,
                               curie2ru_prbs_t prbs, unsigned int enable);
int curie2ru_bcm82757_prbs_clear_rx_stat(struct curie2ru *curie, curie2ru_lane_t lane, curie2ru_if_side_t if_side);
int curie2ru_bcm82757_prbs_check(struct curie2ru *curie,
                                 curie2ru_lane_t lane, curie2ru_if_side_t if_side);

int curie2ru_bcm82757_firmware_lane_set(struct curie2ru *curie,
                                        curie2ru_lane_t lane, curie2ru_if_side_t if_side,
                                        bcm_plp_pm_firmware_lane_config_t *firmware_lane_config);
int curie2ru_bcm82757_firmware_lane_get(struct curie2ru *curie,
                                        curie2ru_lane_t lane, curie2ru_if_side_t if_side,
                                        bcm_plp_pm_firmware_lane_config_t *firmware_lane_config);

int curie2ru_bcm82757_cl73_set(struct curie2ru *curie,
                               curie2ru_lane_t lane, curie2ru_if_side_t if_side,
                               unsigned int enable);
int curie2ru_bcm82757_cl37_set(struct curie2ru *curie,
                               curie2ru_lane_t lane, curie2ru_if_side_t if_side,
                               unsigned int enable);

void curie2ru_bcm82757_config_macsec_cleanup(struct curie2ru *curie);
void curie2ru_bcm82757_line_side_interface_set(char *type);
void force_line_side_intf_lrm(int force);
int curie2ru_bcm82757_line_side_interface_lrm(void);
int curie2ru_bcm82757_macsec_init(struct curie2ru *curie, int bypass);
void curie2ru_bcm82757_macsec_exit(struct curie2ru *curie);
int curie2ru_bcm82757_set_macsec_bypass_mode(struct curie2ru *curie, curie2ru_lane_t lane, int speed);
int curie2ru_bcm82757_config_macsec_bypass(struct curie2ru *curie, curie2ru_lane_t lane, int speed);


int curie2ru_bcm82752_1g_config(struct curie2ru *curie, int port, int recovered);
int curie2ru_bcm82752_10g_config(struct curie2ru *curie, int port);
int curie2ru_bcm82752_mode_config(struct curie2ru *curie, int port, int speed);
int curie2ru_bcm82752_read(struct curie2ru *curie,
                           int port, curie2ru_if_side_t if_side,
                           uint32_t devaddr, uint32_t regaddr, uint32_t *data);
int curie2ru_bcm82752_write(struct curie2ru *curie,
                            int port, curie2ru_if_side_t if_side,
                            uint32_t devaddr, uint32_t regaddr, uint32_t data);
int curie2ru_bcm82752_regs_dump(struct curie2ru *curie, int port);
int curie2ru_bcm82752_dump(struct curie2ru *curie,
                           int port, curie2ru_if_side_t if_side);
int curie2ru_bcm82752_phy_dump(struct curie2ru *curie,
                               int port, curie2ru_if_side_t if_side);
int curie2ru_bcm82752_link_status(struct curie2ru *curie,
                                  int port, curie2ru_if_side_t if_side,
                                  unsigned int *link_status);
int curie2ru_bcm82752_display_eye_scan(struct curie2ru *curie,
                               int port, curie2ru_if_side_t if_side);
int curie2ru_bcm82752_loopback_set(struct curie2ru *curie,
                                   int port, curie2ru_if_side_t if_side,
                                   unsigned int lb_mode, unsigned int enable);
int curie2ru_bcm82752_prbs_set(struct curie2ru *curie,
                               int port, curie2ru_if_side_t if_side,
                               curie2ru_prbs_t prbs, unsigned int enable);
int curie2ru_bcm82752_prbs_clear_rx_stat(struct curie2ru *curie, int port, curie2ru_if_side_t if_side);
int curie2ru_bcm82752_prbs_check(struct curie2ru *curie,
                                 int port, curie2ru_if_side_t if_side);
int curie2ru_bcm82752_set_cl37_an(struct curie2ru *curie, int port, int enable);
int curie2ru_bcm82752_check_cl37_an(struct curie2ru *curie, int port, int *an, int *link, int *done);
int curie2ru_bcm82752_firmware_lane_set(struct curie2ru *curie,
                        int port, curie2ru_if_side_t if_side,
                        bcm_plp_pm_firmware_lane_config_t *firmware_lane_config);
int curie2ru_bcm82752_firmware_lane_get(struct curie2ru *curie,
                        int port, curie2ru_if_side_t if_side,
                        bcm_plp_pm_firmware_lane_config_t *firmware_lane_config);
int curie2ru_bcm82752_cl73_set(struct curie2ru *curie,
                               int port, curie2ru_if_side_t if_side,
                               unsigned int enable);
int bcm57412_port_sideband_tx_dis(int, int);


#endif /* __CURIE2RU_H__ */

/*
 *-----------------------------------------------------------------------------
$Log: curie2ru.h,v $
Revision 1.2  2021/01/11 11:06:06  xiaolaya
*** empty log message ***

Revision 1.1  2020/01/09 01:01:56  jiajliu
Merge Curie 2RU to main trunk

$Endlog$
 *-----------------------------------------------------------------------------
 */
