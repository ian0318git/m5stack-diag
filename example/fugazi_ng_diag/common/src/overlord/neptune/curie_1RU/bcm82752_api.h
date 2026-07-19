/* $Id: bcm82752_api.h,v 1.2 2019/08/06 06:56:11 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_1RU/bcm82752_api.h,v $
 *-----------------------------------------------------------------------------
 * bcm82752_api.h - Header for BCM 10G PHY bcm82752 API.
 *          Leverage from KP
 *
 * Feb 2019, Leschen
 *
 * Copyright (c) 2016 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */
#include <unistd.h>
#include <stdint.h>
#include <errno.h>

#define SYS_SIDE 1
#define LINE_SIDE 0

#define FW_SUCCESS                  0x0     /* success */
#define FW_ERR_PARAM                0x1     /* bad parameters */
#define FW_ERR_SW_INIT              0x2     /* fw_ops is invalid */
#define FW_ERR_HW_ACC               0x3     /* hw acc error */
#define FW_ERR_ACC_TIMEOUT          0x4     /* hw acc timeout */
#define FW_ERR_NOT_SUPPORT          0x5     /* not support operation */
#define FW_ERR_LINE_NOT_LOCKED      0x6     /* port line side not locked */
#define FW_ERR_HOST_NOT_LOCKED      0x7     /* port host side not locked */
#define FW_ERR_LPBK_UNSUPPORTED     0x8     /* loopback unsupported */
#define FW_ERR_NO_EDC_CONNECTED     0x9     /* no edc connected for the fp */
#define FW_ERR_INTF_UNSUPPORTED     0xa     /* interface unsupported */
#define FW_ERR_NOT_INITIALIZED      0xb     /* port not initialized */
#define FW_ERR_LINE_NOT_CONVERGED   0xc     /* port line side not converged  */
#define FW_ERR_HOST_NOT_CONVERGED   0xd     /* port host side not converged  */
#define FW_ERR_NOT_IMPLEMENTED      999     /* functionality not implemented */

#define CURIE_IF_SIDE_TO_QUADRA28(if_side)               \
    ((if_side == CURIE_IF_SIDE_SYS) ?                    \
     CURIE_QUADRA28_SYS_SIDE : CURIE_QUADRA28_LINE_SIDE)
#define CURIE_BCM82752_PORT_SPEED_1G  1000
#define CURIE_BCM82752_PORT_SPEED_10G 10000
#define BCM82752_PORT1 0x0
#define BCM82752_PORT2 0x1

typedef enum bcm82752_intf
{
    BCM82752_XFI_INTF,
    BCM82752_SFI_INTF,
    BCM82752_MAX_INTF
} bcm82752_intf_t;

typedef enum bcm82752_loopback {
    BCM82752_LOOPBACK_UNDEFINED = 0,
    BCM82752_LOOPBACK_XFI,
    BCM82752_LOOPBACK_XFI_LINE,
    BCM82752_LOOPBACK_PCS,
    BCM82752_LOOPBACK_PCS_LINE,
    BCM82752_LOOPBACK_NONE,
    BCM82752_LOOPBACK_XFI_PRBS,
    BCM82752_LOOPBACK_PCS_PRBS,
} bcm82752_loopback_t;

typedef enum phy_lpbk
{
    LPBK_START,
    LPBK_NONE,
    LPBK_PMD,
    LPBK_PMA,
    LPBK_PCS,
    LPBK_HOST_FAR,
    LPBK_LINE = LPBK_HOST_FAR,
    LPBK_HOST_NEAR,
    LPBK_LINE_FAR,
    LPBK_HOST = LPBK_LINE_FAR,
    LPBK_LINE_NEAR,
    LPBK_DEFAULT,
    LPBK_PD_HOST_FAR,
    LPBK_PD_HOST_NEAR,
    LPBK_PD_LINE_FAR,
    LPBK_PD_LINE_NEAR,
    LPBK_BCM_XFI,
    LPBK_BCM_XFI_LINE,
    LPBK_BCM_PCS,
    LPBK_BCM_PCS_LINE,
    LPBK_BCM_PCS_PRBS,
    LPBK_BCM_XFI_PRBS,
    LPBK_END
} phy_lpbk_t;

typedef enum phy_speed
{
    SPEED_START,
    SPEED_1G,
    SPEED_2G,
    SPEED_4G,
    SPEED_8G,
    SPEED_10G,
    SPEED_16G,
    SPEED_40G,
    SPEED_100G,
    SPEED_DEFAULT,
    SPEED_END
} phy_speed_t;

typedef enum phy_port_mode
{
    PORT_MODE_START,
    PORT_MODE_1G_SR,
    PORT_MODE_10G_SR,
    PORT_MODE_10G_LR,
    PORT_MODE_10G_LRM,
    PORT_MODE_10G_CX1,
    PORT_MODE_10G_ACX1,
    PORT_MODE_10G_ER,
    PORT_MODE_10G_USR,
    PORT_MODE_10G_LRM_SM,
    PORT_MODE_10G_FET,
    PORT_MODE_40G_CR4,
    PORT_MODE_40G_SR4,
    PORT_MODE_10G_ZR,
    PORT_MODE_10G_DWDM,
    PORT_MODE_100G_SR10,
    PORT_MODE_100G_CR10,
    PORT_MODE_100G_LR4,
    PORT_MODE_100G_ER4,
    PORT_MODE_10G_COPPER_ACTIVE,
    PORT_MODE_10G_COPPER_PASSIVE,
    PORT_MODE_40G_COPPER_ACTIVE,
    PORT_MODE_40G_COPPER_PASSIVE,
    PORT_MODE_40G_BKOUT_COPPER_ACTIVE,
    PORT_MODE_40G_BKOUT_COPPER_PASSIVE,
    PORT_MODE_100G_LPBK_PASSIVE,
    PORT_MODE_100G_LPBK_ACTIVE,
    PORT_MODE_40G_CR4_FORCE_CX1,
    PORT_MODE_40G_CR4_LBX, /*electrical loopback QSFP */
    PORT_MODE_DEFAULT,
    PORT_MODE_END
} phy_port_mode_t;

typedef struct phy_reg_tbl
{
    const char * reg_name;
    uint32_t     reg_addr;
    uint8_t      mmd;
} phy_reg_tbl_t;

#define PRBS_TEST_DELAY 1
#define BCMI_QUADRA28_TX_CTRL_5r (0x0001d0a5 | PHYMOD_REG_ACC_RAW)
#define BCMI_QUADRA28_TXFIR_CONTROL1r (0x0001d110 | PHYMOD_REG_ACC_RAW)
#define BCMI_QUADRA28_TXFIR_CONTROL2r (0x0001d111 | PHYMOD_REG_ACC_RAW)
#define PHYMOD_REG_ACC_RAW    0x000000
#define C800_MAGIC  0x38ff

typedef enum {
    CURIE_PRBS_7,
    CURIE_PRBS_9,
    CURIE_PRBS_11,
    CURIE_PRBS_15,
    CURIE_PRBS_23,
    CURIE_PRBS_31,
    MAX_NR_CURIE_PRBS
} curie_prbs_t;

#define BIT32(x)    (1<<x)

extern int bcm82752_reg_rd(int port, int dev, int reg);
extern int bcm82752_reg_wr(int port, int dev, int reg, int val);
extern int bcm82752_verify_mcode(int port);
extern int bcm82752_set_port_mode(int port, phy_port_mode_t mode);
extern int bcm82752_set_sfi_serdes(int port, phy_speed_t speed);
extern int bcm82752_xfi_sfi_access(int port, bcm82752_intf_t intf);
extern int bcm82752_soft_reset(int port, int dev_id);
extern int bcm82752_cfg_setting(int port, int speed, int duplex, int auto_neg, bcm82752_intf_t intf);
extern int bcm82752_is_link_up(int port, unsigned int *link_up);
extern int bcm82752_config_loopback(int port, bcm82752_loopback_t loopback_mode);
extern int bcm82752_set_port_speed(int port, phy_speed_t speed);
extern int enable_bcm82752_ptp_engine (int);
extern int en_bcm82752_ptp_per_port (int, int);
extern int bcm82752_is_sfp_module_present(int);
extern uint32_t bcm82752_twsi_mii_reg_rw(uint8_t, uint32_t, uint32_t, uint8_t *, uint32_t, uint8_t);
extern int quadra28_eye_diagram(void);
extern int miura_eye_diagram(void);
extern int is_bcm82752(void);
extern int not_bcm82752(void);
extern int bcm82752_emphasis_setting (void);
/*-------------------------------------------------
$Log: bcm82752_api.h,v $
Revision 1.2  2019/08/06 06:56:11  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.1.2.2  2019/06/04 06:04:17  leschen
Support PHY PRBS feature

Revision 1.1.2.1  2019/03/12 07:41:51  leschen
Initial check in to support BCM82752


$Endlog$
*/
