/* $Id: diag_bcm57412_utils.h,v 1.2 2021/06/02 08:22:34 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/diag_bcm57412_utils.h,v $
 * diag_bcm57412_utils.h - BNXT definitions for BCM57412 NIC
 *
 * Sep 2019,  Ian Chang
 *
 * Copyright (c) 2019-2021 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */
#ifndef __DIAG_BCM57412_UTILS_H__ 
#define __DIAG_BCM57412_UTILS_H__

#include <linux/netlink.h>
#include <linux/genetlink.h>
#include "types.h"

#ifndef __bitwise
#define __bitwise
#endif

/* Reference: BCM5730X/BCM5731X/BCM5740X/BCM5741X/BCM5745X Family 
 * Programmer's Guide (573XX-574XX-PG101)
 */
#define HWRM_PORT_PHY_QCFG              0x27UL
#define HWRM_PORT_PHY_I2C_WRITE         0x2bUL
#define HWRM_PORT_PHY_I2C_READ          0x2cUL
#define HWRM_PORT_SFP_SIDEBAND_CFG      0xd6UL
#define HWRM_PORT_SFP_SIDEBAND_QCFG     0xd7UL
#define HWRM_SELFTEST_QLIST             0x200UL
#define HWRM_SELFTEST_EXEC              0x201UL
#define HWRM_PORT_PHY_MDIO_BUS_ACQUIRE  0xb7UL
#define HWRM_PORT_PHY_MDIO_BUS_RELEASE  0xb8UL


#define I2C_DEV_ADDR_A0	                0xa0
#define I2C_DEV_ADDR_A2             	0xa2
#define SFP_EEPROM_SFF_8472_COMP_ADDR	0x5e
#define SFP_EEPROM_SFF_8472_COMP_SIZE   1

#define DEV_IFINDEX_PATH_SIZE           8192
#define SYS_IFINDEX_PATH "/sys/class/net/%s/ifindex"

enum {
	BNXT_CMD_UNSPEC,
	BNXT_CMD_HWRM,
	BNXT_NUM_CMDS
};
#define BNXT_CMD_MAX (BNXT_NUM_CMDS - 1)
#define BNXT_MAX_PHY_I2C_RESP_SIZE		64
#define BNXT_MAX_PHY_I2C_REQ_SIZE		64

/* attributes */
enum {
	BNXT_ATTR_UNSPEC,
	BNXT_ATTR_PID,
	BNXT_ATTR_IF_INDEX,
	BNXT_ATTR_REQUEST,
	BNXT_ATTR_RESPONSE,
	BNXT_NUM_ATTRS
};
#define BNXT_ATTR_MAX (BNXT_NUM_ATTRS - 1)

#define BNXT_NL_NAME "bnxt_netlink"
#define BNXT_NL_VER  1
#define BNXT_BUF_MAX 256


typedef __u16 __bitwise __le16;
typedef __u16 __bitwise __be16;
typedef __u32 __bitwise __le32;
typedef __u32 __bitwise __be32;
typedef __u64 __bitwise __le64;
typedef __u64 __bitwise __be64;
typedef uint8_t u8;

typedef __u16 __bitwise __sum16;
typedef __u32 __bitwise __wsum;

typedef struct bcm_nl_request_msg_ {
    struct nlmsghdr n;
    struct genlmsghdr g;
    char buf[BNXT_BUF_MAX];
} bcm_nl_request_msg_t;

/* hwrm_port_phy_i2c_read_input (size:320b/40B) */
typedef struct hwrm_port_phy_i2c_read_input_ {
	__le16	req_type;
	__le16	cmpl_ring;
	__le16	seq_id;
	__le16	target_id;
	__le64	resp_addr;
	__le32	flags;
	__le32	enables;
	#define PORT_PHY_I2C_READ_REQ_ENABLES_PAGE_OFFSET     0x1UL
	__le16	port_id;
	unsigned char i2c_slave_addr;
	unsigned char unused_0;
	__le16	page_number;
	__le16	page_offset;
	unsigned char data_length;
	unsigned char unused_1[7];
} hwrm_port_phy_i2c_read_input_t;

/* hwrm_port_phy_i2c_read_output (size:640b/80B) */
typedef struct hwrm_port_phy_i2c_read_output_ {
	__le16	error_code;
	__le16	req_type;
	__le16	seq_id;
	__le16	resp_len;
	__le32	data[16];
	unsigned char unused_0[7];
	unsigned char valid;
} hwrm_port_phy_i2c_read_output_t;

/* hwrm_port_phy_i2c_write_input (size:832b/104B) */
typedef struct hwrm_port_phy_i2c_write_input_ {
	__le16	req_type;
	__le16	cmpl_ring;
	__le16	seq_id;
	__le16	target_id;
	__le64	resp_addr;
	__le32	flags;
	__le32	enables;
	#define PORT_PHY_I2C_WRITE_REQ_ENABLES_PAGE_OFFSET     0x1UL
	__le16	port_id;
	uint8_t	i2c_slave_addr;
	uint8_t	unused_0;
	__le16	page_number;
	__le16	page_offset;
	uint8_t	data_length;
	uint8_t	unused_1[7];
	__le32	data[16];
} hwrm_port_phy_i2c_write_input_t;

/* hwrm_port_phy_i2c_write_output (size:128b/16B) */
typedef struct hwrm_port_phy_i2c_write_output_ {
	__le16	error_code;
	__le16	req_type;
	__le16	seq_id;
	__le16	resp_len;
	uint8_t	unused_0[7];
	uint8_t	valid;
} hwrm_port_phy_i2c_write_output_t;

/* hwrm_port_phy_qcfg_input (size:192b/24B) */
typedef struct hwrm_port_phy_qcfg_input_ {
	__le16	req_type;
	__le16	cmpl_ring;
	__le16	seq_id;
	__le16	target_id;
	__le64	resp_addr;
	__le16	port_id;
	u8	unused_0[6];
} hwrm_port_phy_qcfg_input_t;

/* hwrm_port_phy_qcfg_output (size:768b/96B) */
typedef struct hwrm_port_phy_qcfg_output_ {
	__le16	error_code;
	__le16	req_type;
	__le16	seq_id;
	__le16	resp_len;
	u8	link;
	#define PORT_PHY_QCFG_RESP_LINK_NO_LINK 0x0UL
	#define PORT_PHY_QCFG_RESP_LINK_SIGNAL  0x1UL
	#define PORT_PHY_QCFG_RESP_LINK_LINK    0x2UL
	#define PORT_PHY_QCFG_RESP_LINK_LAST   PORT_PHY_QCFG_RESP_LINK_LINK
	u8	unused_0;
	__le16	link_speed;
	#define PORT_PHY_QCFG_RESP_LINK_SPEED_100MB 0x1UL
	#define PORT_PHY_QCFG_RESP_LINK_SPEED_1GB   0xaUL
	#define PORT_PHY_QCFG_RESP_LINK_SPEED_2GB   0x14UL
	#define PORT_PHY_QCFG_RESP_LINK_SPEED_2_5GB 0x19UL
	#define PORT_PHY_QCFG_RESP_LINK_SPEED_10GB  0x64UL
	#define PORT_PHY_QCFG_RESP_LINK_SPEED_20GB  0xc8UL
	#define PORT_PHY_QCFG_RESP_LINK_SPEED_25GB  0xfaUL
	#define PORT_PHY_QCFG_RESP_LINK_SPEED_40GB  0x190UL
	#define PORT_PHY_QCFG_RESP_LINK_SPEED_50GB  0x1f4UL
	#define PORT_PHY_QCFG_RESP_LINK_SPEED_100GB 0x3e8UL
	#define PORT_PHY_QCFG_RESP_LINK_SPEED_10MB  0xffffUL
	#define PORT_PHY_QCFG_RESP_LINK_SPEED_LAST \
                                    PORT_PHY_QCFG_RESP_LINK_SPEED_10MB
	u8	duplex_cfg;
	#define PORT_PHY_QCFG_RESP_DUPLEX_CFG_HALF 0x0UL
	#define PORT_PHY_QCFG_RESP_DUPLEX_CFG_FULL 0x1UL
	#define PORT_PHY_QCFG_RESP_DUPLEX_CFG_LAST \
                                    PORT_PHY_QCFG_RESP_DUPLEX_CFG_FULL
	u8	pause;
	#define PORT_PHY_QCFG_RESP_PAUSE_TX     0x1UL
	#define PORT_PHY_QCFG_RESP_PAUSE_RX     0x2UL
	__le16	support_speeds;
	#define PORT_PHY_QCFG_RESP_SUPPORT_SPEEDS_100MBHD     0x1UL
	#define PORT_PHY_QCFG_RESP_SUPPORT_SPEEDS_100MB       0x2UL
	#define PORT_PHY_QCFG_RESP_SUPPORT_SPEEDS_1GBHD       0x4UL
	#define PORT_PHY_QCFG_RESP_SUPPORT_SPEEDS_1GB         0x8UL
	#define PORT_PHY_QCFG_RESP_SUPPORT_SPEEDS_2GB         0x10UL
	#define PORT_PHY_QCFG_RESP_SUPPORT_SPEEDS_2_5GB       0x20UL
	#define PORT_PHY_QCFG_RESP_SUPPORT_SPEEDS_10GB        0x40UL
	#define PORT_PHY_QCFG_RESP_SUPPORT_SPEEDS_20GB        0x80UL
	#define PORT_PHY_QCFG_RESP_SUPPORT_SPEEDS_25GB        0x100UL
	#define PORT_PHY_QCFG_RESP_SUPPORT_SPEEDS_40GB        0x200UL
	#define PORT_PHY_QCFG_RESP_SUPPORT_SPEEDS_50GB        0x400UL
	#define PORT_PHY_QCFG_RESP_SUPPORT_SPEEDS_100GB       0x800UL
	#define PORT_PHY_QCFG_RESP_SUPPORT_SPEEDS_10MBHD      0x1000UL
	#define PORT_PHY_QCFG_RESP_SUPPORT_SPEEDS_10MB        0x2000UL
	__le16	force_link_speed;
	#define PORT_PHY_QCFG_RESP_FORCE_LINK_SPEED_100MB 0x1UL
	#define PORT_PHY_QCFG_RESP_FORCE_LINK_SPEED_1GB   0xaUL
	#define PORT_PHY_QCFG_RESP_FORCE_LINK_SPEED_2GB   0x14UL
	#define PORT_PHY_QCFG_RESP_FORCE_LINK_SPEED_2_5GB 0x19UL
	#define PORT_PHY_QCFG_RESP_FORCE_LINK_SPEED_10GB  0x64UL
	#define PORT_PHY_QCFG_RESP_FORCE_LINK_SPEED_20GB  0xc8UL
	#define PORT_PHY_QCFG_RESP_FORCE_LINK_SPEED_25GB  0xfaUL
	#define PORT_PHY_QCFG_RESP_FORCE_LINK_SPEED_40GB  0x190UL
	#define PORT_PHY_QCFG_RESP_FORCE_LINK_SPEED_50GB  0x1f4UL
	#define PORT_PHY_QCFG_RESP_FORCE_LINK_SPEED_100GB 0x3e8UL
	#define PORT_PHY_QCFG_RESP_FORCE_LINK_SPEED_10MB  0xffffUL
	#define PORT_PHY_QCFG_RESP_FORCE_LINK_SPEED_LAST \
                                  PORT_PHY_QCFG_RESP_FORCE_LINK_SPEED_10MB
	u8	auto_mode;
	#define PORT_PHY_QCFG_RESP_AUTO_MODE_NONE         0x0UL
	#define PORT_PHY_QCFG_RESP_AUTO_MODE_ALL_SPEEDS   0x1UL
	#define PORT_PHY_QCFG_RESP_AUTO_MODE_ONE_SPEED    0x2UL
	#define PORT_PHY_QCFG_RESP_AUTO_MODE_ONE_OR_BELOW 0x3UL
	#define PORT_PHY_QCFG_RESP_AUTO_MODE_SPEED_MASK   0x4UL
	#define PORT_PHY_QCFG_RESP_AUTO_MODE_LAST       \
                                    PORT_PHY_QCFG_RESP_AUTO_MODE_SPEED_MASK
	u8	auto_pause;
	#define PORT_PHY_QCFG_RESP_AUTO_PAUSE_TX                0x1UL
	#define PORT_PHY_QCFG_RESP_AUTO_PAUSE_RX                0x2UL
	#define PORT_PHY_QCFG_RESP_AUTO_PAUSE_AUTONEG_PAUSE     0x4UL
	__le16	auto_link_speed;
	#define PORT_PHY_QCFG_RESP_AUTO_LINK_SPEED_100MB 0x1UL
	#define PORT_PHY_QCFG_RESP_AUTO_LINK_SPEED_1GB   0xaUL
	#define PORT_PHY_QCFG_RESP_AUTO_LINK_SPEED_2GB   0x14UL
	#define PORT_PHY_QCFG_RESP_AUTO_LINK_SPEED_2_5GB 0x19UL
	#define PORT_PHY_QCFG_RESP_AUTO_LINK_SPEED_10GB  0x64UL
	#define PORT_PHY_QCFG_RESP_AUTO_LINK_SPEED_20GB  0xc8UL
	#define PORT_PHY_QCFG_RESP_AUTO_LINK_SPEED_25GB  0xfaUL
	#define PORT_PHY_QCFG_RESP_AUTO_LINK_SPEED_40GB  0x190UL
	#define PORT_PHY_QCFG_RESP_AUTO_LINK_SPEED_50GB  0x1f4UL
	#define PORT_PHY_QCFG_RESP_AUTO_LINK_SPEED_100GB 0x3e8UL
	#define PORT_PHY_QCFG_RESP_AUTO_LINK_SPEED_10MB  0xffffUL
	#define PORT_PHY_QCFG_RESP_AUTO_LINK_SPEED_LAST \
                                    PORT_PHY_QCFG_RESP_AUTO_LINK_SPEED_10MB
	__le16	auto_link_speed_mask;
	#define PORT_PHY_QCFG_RESP_AUTO_LINK_SPEED_MASK_100MBHD     0x1UL
	#define PORT_PHY_QCFG_RESP_AUTO_LINK_SPEED_MASK_100MB       0x2UL
	#define PORT_PHY_QCFG_RESP_AUTO_LINK_SPEED_MASK_1GBHD       0x4UL
	#define PORT_PHY_QCFG_RESP_AUTO_LINK_SPEED_MASK_1GB         0x8UL
	#define PORT_PHY_QCFG_RESP_AUTO_LINK_SPEED_MASK_2GB         0x10UL
	#define PORT_PHY_QCFG_RESP_AUTO_LINK_SPEED_MASK_2_5GB       0x20UL
	#define PORT_PHY_QCFG_RESP_AUTO_LINK_SPEED_MASK_10GB        0x40UL
	#define PORT_PHY_QCFG_RESP_AUTO_LINK_SPEED_MASK_20GB        0x80UL
	#define PORT_PHY_QCFG_RESP_AUTO_LINK_SPEED_MASK_25GB        0x100UL
	#define PORT_PHY_QCFG_RESP_AUTO_LINK_SPEED_MASK_40GB        0x200UL
	#define PORT_PHY_QCFG_RESP_AUTO_LINK_SPEED_MASK_50GB        0x400UL
	#define PORT_PHY_QCFG_RESP_AUTO_LINK_SPEED_MASK_100GB       0x800UL
	#define PORT_PHY_QCFG_RESP_AUTO_LINK_SPEED_MASK_10MBHD      0x1000UL
	#define PORT_PHY_QCFG_RESP_AUTO_LINK_SPEED_MASK_10MB        0x2000UL
	u8	wirespeed;
	#define PORT_PHY_QCFG_RESP_WIRESPEED_OFF 0x0UL
	#define PORT_PHY_QCFG_RESP_WIRESPEED_ON  0x1UL
	#define PORT_PHY_QCFG_RESP_WIRESPEED_LAST PORT_PHY_QCFG_RESP_WIRESPEED_ON
	u8	lpbk;
	#define PORT_PHY_QCFG_RESP_LPBK_NONE     0x0UL
	#define PORT_PHY_QCFG_RESP_LPBK_LOCAL    0x1UL
	#define PORT_PHY_QCFG_RESP_LPBK_REMOTE   0x2UL
	#define PORT_PHY_QCFG_RESP_LPBK_EXTERNAL 0x3UL
	#define PORT_PHY_QCFG_RESP_LPBK_LAST    PORT_PHY_QCFG_RESP_LPBK_EXTERNAL
	u8	force_pause;
	#define PORT_PHY_QCFG_RESP_FORCE_PAUSE_TX     0x1UL
	#define PORT_PHY_QCFG_RESP_FORCE_PAUSE_RX     0x2UL
	u8	module_status;
	#define PORT_PHY_QCFG_RESP_MODULE_STATUS_NONE          0x0UL
	#define PORT_PHY_QCFG_RESP_MODULE_STATUS_DISABLETX     0x1UL
	#define PORT_PHY_QCFG_RESP_MODULE_STATUS_WARNINGMSG    0x2UL
	#define PORT_PHY_QCFG_RESP_MODULE_STATUS_PWRDOWN       0x3UL
	#define PORT_PHY_QCFG_RESP_MODULE_STATUS_NOTINSERTED   0x4UL
	#define PORT_PHY_QCFG_RESP_MODULE_STATUS_NOTAPPLICABLE 0xffUL
	#define PORT_PHY_QCFG_RESP_MODULE_STATUS_LAST      \
                           PORT_PHY_QCFG_RESP_MODULE_STATUS_NOTAPPLICABLE
	__le32	preemphasis;
	u8	phy_maj;
	u8	phy_min;
	u8	phy_bld;
	u8	phy_type;
	#define PORT_PHY_QCFG_RESP_PHY_TYPE_UNKNOWN          0x0UL
	#define PORT_PHY_QCFG_RESP_PHY_TYPE_BASECR           0x1UL
	#define PORT_PHY_QCFG_RESP_PHY_TYPE_BASEKR4          0x2UL
	#define PORT_PHY_QCFG_RESP_PHY_TYPE_BASELR           0x3UL
	#define PORT_PHY_QCFG_RESP_PHY_TYPE_BASESR           0x4UL
	#define PORT_PHY_QCFG_RESP_PHY_TYPE_BASEKR2          0x5UL
	#define PORT_PHY_QCFG_RESP_PHY_TYPE_BASEKX           0x6UL
	#define PORT_PHY_QCFG_RESP_PHY_TYPE_BASEKR           0x7UL
	#define PORT_PHY_QCFG_RESP_PHY_TYPE_BASET            0x8UL
	#define PORT_PHY_QCFG_RESP_PHY_TYPE_BASETE           0x9UL
	#define PORT_PHY_QCFG_RESP_PHY_TYPE_SGMIIEXTPHY      0xaUL
	#define PORT_PHY_QCFG_RESP_PHY_TYPE_25G_BASECR_CA_L  0xbUL
	#define PORT_PHY_QCFG_RESP_PHY_TYPE_25G_BASECR_CA_S  0xcUL
	#define PORT_PHY_QCFG_RESP_PHY_TYPE_25G_BASECR_CA_N  0xdUL
	#define PORT_PHY_QCFG_RESP_PHY_TYPE_25G_BASESR       0xeUL
	#define PORT_PHY_QCFG_RESP_PHY_TYPE_100G_BASECR4     0xfUL
	#define PORT_PHY_QCFG_RESP_PHY_TYPE_100G_BASESR4     0x10UL
	#define PORT_PHY_QCFG_RESP_PHY_TYPE_100G_BASELR4     0x11UL
	#define PORT_PHY_QCFG_RESP_PHY_TYPE_100G_BASEER4     0x12UL
	#define PORT_PHY_QCFG_RESP_PHY_TYPE_100G_BASESR10    0x13UL
	#define PORT_PHY_QCFG_RESP_PHY_TYPE_40G_BASECR4      0x14UL
	#define PORT_PHY_QCFG_RESP_PHY_TYPE_40G_BASESR4      0x15UL
	#define PORT_PHY_QCFG_RESP_PHY_TYPE_40G_BASELR4      0x16UL
	#define PORT_PHY_QCFG_RESP_PHY_TYPE_40G_BASEER4      0x17UL
	#define PORT_PHY_QCFG_RESP_PHY_TYPE_40G_ACTIVE_CABLE 0x18UL
	#define PORT_PHY_QCFG_RESP_PHY_TYPE_1G_BASET         0x19UL
	#define PORT_PHY_QCFG_RESP_PHY_TYPE_1G_BASESX        0x1aUL
	#define PORT_PHY_QCFG_RESP_PHY_TYPE_1G_BASECX        0x1bUL
	#define PORT_PHY_QCFG_RESP_PHY_TYPE_LAST           \
                                    PORT_PHY_QCFG_RESP_PHY_TYPE_1G_BASECX
	u8	media_type;
	#define PORT_PHY_QCFG_RESP_MEDIA_TYPE_UNKNOWN 0x0UL
	#define PORT_PHY_QCFG_RESP_MEDIA_TYPE_TP      0x1UL
	#define PORT_PHY_QCFG_RESP_MEDIA_TYPE_DAC     0x2UL
	#define PORT_PHY_QCFG_RESP_MEDIA_TYPE_FIBRE   0x3UL
	#define PORT_PHY_QCFG_RESP_MEDIA_TYPE_LAST  \
                                      PORT_PHY_QCFG_RESP_MEDIA_TYPE_FIBRE
	u8	xcvr_pkg_type;
	#define PORT_PHY_QCFG_RESP_XCVR_PKG_TYPE_XCVR_INTERNAL 0x1UL
	#define PORT_PHY_QCFG_RESP_XCVR_PKG_TYPE_XCVR_EXTERNAL 0x2UL
	#define PORT_PHY_QCFG_RESP_XCVR_PKG_TYPE_LAST         \
                            PORT_PHY_QCFG_RESP_XCVR_PKG_TYPE_XCVR_EXTERNAL
	u8	eee_config_phy_addr;
	#define PORT_PHY_QCFG_RESP_PHY_ADDR_MASK              0x1fUL
	#define PORT_PHY_QCFG_RESP_PHY_ADDR_SFT               0
	#define PORT_PHY_QCFG_RESP_EEE_CONFIG_MASK            0xe0UL
	#define PORT_PHY_QCFG_RESP_EEE_CONFIG_SFT             5
	#define PORT_PHY_QCFG_RESP_EEE_CONFIG_EEE_ENABLED      0x20UL
	#define PORT_PHY_QCFG_RESP_EEE_CONFIG_EEE_ACTIVE       0x40UL
	#define PORT_PHY_QCFG_RESP_EEE_CONFIG_EEE_TX_LPI       0x80UL
	u8	parallel_detect;
	#define PORT_PHY_QCFG_RESP_PARALLEL_DETECT     0x1UL
	__le16	link_partner_adv_speeds;
	#define PORT_PHY_QCFG_RESP_LINK_PARTNER_ADV_SPEEDS_100MBHD     0x1UL
	#define PORT_PHY_QCFG_RESP_LINK_PARTNER_ADV_SPEEDS_100MB       0x2UL
	#define PORT_PHY_QCFG_RESP_LINK_PARTNER_ADV_SPEEDS_1GBHD       0x4UL
	#define PORT_PHY_QCFG_RESP_LINK_PARTNER_ADV_SPEEDS_1GB         0x8UL
	#define PORT_PHY_QCFG_RESP_LINK_PARTNER_ADV_SPEEDS_2GB         0x10UL
	#define PORT_PHY_QCFG_RESP_LINK_PARTNER_ADV_SPEEDS_2_5GB       0x20UL
	#define PORT_PHY_QCFG_RESP_LINK_PARTNER_ADV_SPEEDS_10GB        0x40UL
	#define PORT_PHY_QCFG_RESP_LINK_PARTNER_ADV_SPEEDS_20GB        0x80UL
	#define PORT_PHY_QCFG_RESP_LINK_PARTNER_ADV_SPEEDS_25GB        0x100UL
	#define PORT_PHY_QCFG_RESP_LINK_PARTNER_ADV_SPEEDS_40GB        0x200UL
	#define PORT_PHY_QCFG_RESP_LINK_PARTNER_ADV_SPEEDS_50GB        0x400UL
	#define PORT_PHY_QCFG_RESP_LINK_PARTNER_ADV_SPEEDS_100GB       0x800UL
	#define PORT_PHY_QCFG_RESP_LINK_PARTNER_ADV_SPEEDS_10MBHD      0x1000UL
	#define PORT_PHY_QCFG_RESP_LINK_PARTNER_ADV_SPEEDS_10MB        0x2000UL
	u8	link_partner_adv_auto_mode;
	#define PORT_PHY_QCFG_RESP_LINK_PARTNER_ADV_AUTO_MODE_NONE         0x0UL
	#define PORT_PHY_QCFG_RESP_LINK_PARTNER_ADV_AUTO_MODE_ALL_SPEEDS   0x1UL
	#define PORT_PHY_QCFG_RESP_LINK_PARTNER_ADV_AUTO_MODE_ONE_SPEED    0x2UL
	#define PORT_PHY_QCFG_RESP_LINK_PARTNER_ADV_AUTO_MODE_ONE_OR_BELOW 0x3UL
	#define PORT_PHY_QCFG_RESP_LINK_PARTNER_ADV_AUTO_MODE_SPEED_MASK   0x4UL
	#define PORT_PHY_QCFG_RESP_LINK_PARTNER_ADV_AUTO_MODE_LAST       \
                     PORT_PHY_QCFG_RESP_LINK_PARTNER_ADV_AUTO_MODE_SPEED_MASK
	u8	link_partner_adv_pause;
	#define PORT_PHY_QCFG_RESP_LINK_PARTNER_ADV_PAUSE_TX     0x1UL
	#define PORT_PHY_QCFG_RESP_LINK_PARTNER_ADV_PAUSE_RX     0x2UL
	__le16	adv_eee_link_speed_mask;
	#define PORT_PHY_QCFG_RESP_ADV_EEE_LINK_SPEED_MASK_RSVD1     0x1UL
	#define PORT_PHY_QCFG_RESP_ADV_EEE_LINK_SPEED_MASK_100MB     0x2UL
	#define PORT_PHY_QCFG_RESP_ADV_EEE_LINK_SPEED_MASK_RSVD2     0x4UL
	#define PORT_PHY_QCFG_RESP_ADV_EEE_LINK_SPEED_MASK_1GB       0x8UL
	#define PORT_PHY_QCFG_RESP_ADV_EEE_LINK_SPEED_MASK_RSVD3     0x10UL
	#define PORT_PHY_QCFG_RESP_ADV_EEE_LINK_SPEED_MASK_RSVD4     0x20UL
	#define PORT_PHY_QCFG_RESP_ADV_EEE_LINK_SPEED_MASK_10GB      0x40UL
	__le16	link_partner_adv_eee_link_speed_mask;
	#define PORT_PHY_QCFG_RESP_LINK_PARTNER_ADV_EEE_LINK_SPEED_MASK_RSVD1 0x1UL
	#define PORT_PHY_QCFG_RESP_LINK_PARTNER_ADV_EEE_LINK_SPEED_MASK_100MB 0x2UL
	#define PORT_PHY_QCFG_RESP_LINK_PARTNER_ADV_EEE_LINK_SPEED_MASK_RSVD2 0x4UL
	#define PORT_PHY_QCFG_RESP_LINK_PARTNER_ADV_EEE_LINK_SPEED_MASK_1GB   0x8UL
	#define PORT_PHY_QCFG_RESP_LINK_PARTNER_ADV_EEE_LINK_SPEED_MASK_RSVD3 0x10UL
	#define PORT_PHY_QCFG_RESP_LINK_PARTNER_ADV_EEE_LINK_SPEED_MASK_RSVD4 0x20UL
	#define PORT_PHY_QCFG_RESP_LINK_PARTNER_ADV_EEE_LINK_SPEED_MASK_10GB  0x40UL
	__le32	xcvr_identifier_type_tx_lpi_timer;
	#define PORT_PHY_QCFG_RESP_TX_LPI_TIMER_MASK            0xffffffUL
	#define PORT_PHY_QCFG_RESP_TX_LPI_TIMER_SFT             0
	#define PORT_PHY_QCFG_RESP_XCVR_IDENTIFIER_TYPE_MASK    0xff000000UL
	#define PORT_PHY_QCFG_RESP_XCVR_IDENTIFIER_TYPE_SFT     24
	#define PORT_PHY_QCFG_RESP_XCVR_IDENTIFIER_TYPE_UNKNOWN   (0x0UL << 24)
	#define PORT_PHY_QCFG_RESP_XCVR_IDENTIFIER_TYPE_SFP       (0x3UL << 24)
	#define PORT_PHY_QCFG_RESP_XCVR_IDENTIFIER_TYPE_QSFP      (0xcUL << 24)
	#define PORT_PHY_QCFG_RESP_XCVR_IDENTIFIER_TYPE_QSFPPLUS  (0xdUL << 24)
	#define PORT_PHY_QCFG_RESP_XCVR_IDENTIFIER_TYPE_QSFP28    (0x11UL << 24)
	#define PORT_PHY_QCFG_RESP_XCVR_IDENTIFIER_TYPE_LAST     \
                    PORT_PHY_QCFG_RESP_XCVR_IDENTIFIER_TYPE_QSFP28
	__le16	fec_cfg;
	#define PORT_PHY_QCFG_RESP_FEC_CFG_FEC_NONE_SUPPORTED         0x1UL
	#define PORT_PHY_QCFG_RESP_FEC_CFG_FEC_AUTONEG_SUPPORTED      0x2UL
	#define PORT_PHY_QCFG_RESP_FEC_CFG_FEC_AUTONEG_ENABLED        0x4UL
	#define PORT_PHY_QCFG_RESP_FEC_CFG_FEC_CLAUSE74_SUPPORTED     0x8UL
	#define PORT_PHY_QCFG_RESP_FEC_CFG_FEC_CLAUSE74_ENABLED       0x10UL
	#define PORT_PHY_QCFG_RESP_FEC_CFG_FEC_CLAUSE91_SUPPORTED     0x20UL
	#define PORT_PHY_QCFG_RESP_FEC_CFG_FEC_CLAUSE91_ENABLED       0x40UL
	u8	duplex_state;
	#define PORT_PHY_QCFG_RESP_DUPLEX_STATE_HALF 0x0UL
	#define PORT_PHY_QCFG_RESP_DUPLEX_STATE_FULL 0x1UL
	#define PORT_PHY_QCFG_RESP_DUPLEX_STATE_LAST  \
                                    PORT_PHY_QCFG_RESP_DUPLEX_STATE_FULL
	u8	option_flags;
	#define PORT_PHY_QCFG_RESP_OPTION_FLAGS_MEDIA_AUTO_DETECT     0x1UL
	char	phy_vendor_name[16];
	char	phy_vendor_partnumber[16];
	u8	unused_2[7];
	u8	valid;
} hwrm_port_phy_qcfg_output_t;



/* hwrm_port_sfp_sideband_cfg_input (size:256b/32B) */
typedef struct hwrm_port_sfp_sideband_cfg_input_ {
	__le16	req_type;
	__le16	cmpl_ring;
	__le16	seq_id;
	__le16	target_id;
	__le64	resp_addr;
	__le16	port_id;
	u8	unused_0[6];
	__le32	enables;
	#define PORT_SFP_SIDEBAND_CFG_REQ_ENABLES_RS0         0x1UL
	#define PORT_SFP_SIDEBAND_CFG_REQ_ENABLES_RS1         0x2UL
	#define PORT_SFP_SIDEBAND_CFG_REQ_ENABLES_TX_DIS      0x4UL
	#define PORT_SFP_SIDEBAND_CFG_REQ_ENABLES_MOD_SEL     0x8UL
	#define PORT_SFP_SIDEBAND_CFG_REQ_ENABLES_RESET_L     0x10UL
	#define PORT_SFP_SIDEBAND_CFG_REQ_ENABLES_LP_MODE     0x20UL
	__le32	flags;
	#define PORT_SFP_SIDEBAND_CFG_REQ_FLAGS_RS0         0x1UL
	#define PORT_SFP_SIDEBAND_CFG_REQ_FLAGS_RS1         0x2UL
	#define PORT_SFP_SIDEBAND_CFG_REQ_FLAGS_TX_DIS      0x4UL
	#define PORT_SFP_SIDEBAND_CFG_REQ_FLAGS_MOD_SEL     0x8UL
	#define PORT_SFP_SIDEBAND_CFG_REQ_FLAGS_RESET_L     0x10UL
	#define PORT_SFP_SIDEBAND_CFG_REQ_FLAGS_LP_MODE     0x20UL
} hwrm_port_sfp_sideband_cfg_input_t;

/* hwrm_port_sfp_sideband_cfg_output (size:128b/16B) */
typedef struct hwrm_port_sfp_sideband_cfg_output_ {
	__le16	error_code;
	__le16	req_type;
	__le16	seq_id;
	__le16	resp_len;
	u8	unused[7];
	u8	valid;
} hwrm_port_sfp_sideband_cfg_output_t;

/* hwrm_port_sfp_sideband_qcfg_input (size:192b/24B) */
typedef struct hwrm_port_sfp_sideband_qcfg_input_ {
	__le16	req_type;
	__le16	cmpl_ring;
	__le16	seq_id;
	__le16	target_id;
	__le64	resp_addr;
	__le16	port_id;
	u8	unused_0[6];
} hwrm_port_sfp_sideband_qcfg_input_t;

/* hwrm_port_sfp_sideband_qcfg_output (size:192b/24B) */
typedef struct hwrm_port_sfp_sideband_qcfg_output_ {
	__le16	error_code;
	__le16	req_type;
	__le16	seq_id;
	__le16	resp_len;
	__le32	supported_mask;
	__le32	sideband_signals;
	#define PORT_SFP_SIDEBAND_QCFG_RESP_SIDEBAND_SIGNALS_MOD_ABS      0x1UL
	#define PORT_SFP_SIDEBAND_QCFG_RESP_SIDEBAND_SIGNALS_RX_LOS       0x2UL
	#define PORT_SFP_SIDEBAND_QCFG_RESP_SIDEBAND_SIGNALS_RS0          0x4UL
	#define PORT_SFP_SIDEBAND_QCFG_RESP_SIDEBAND_SIGNALS_RS1          0x8UL
	#define PORT_SFP_SIDEBAND_QCFG_RESP_SIDEBAND_SIGNALS_TX_DIS       0x10UL
	#define PORT_SFP_SIDEBAND_QCFG_RESP_SIDEBAND_SIGNALS_TX_FAULT     0x20UL
	#define PORT_SFP_SIDEBAND_QCFG_RESP_SIDEBAND_SIGNALS_MOD_SEL      0x40UL
	#define PORT_SFP_SIDEBAND_QCFG_RESP_SIDEBAND_SIGNALS_RESET_L      0x80UL
	#define PORT_SFP_SIDEBAND_QCFG_RESP_SIDEBAND_SIGNALS_LP_MODE      0x100UL
	u8	unused[7];
	u8	valid;
} hwrm_port_sfp_sideband_qcfg_output_t;

/* hwrm_selftest_qlist_input (size:128b/16B) */
typedef struct hwrm_selftest_qlist_input_ {
	__le16	req_type;
	__le16	cmpl_ring;
	__le16	seq_id;
	__le16	target_id;
	__le64	resp_addr;
} hwrm_selftest_qlist_input_t;

/* hwrm_selftest_qlist_output (size:2240b/280B) */
typedef struct hwrm_selftest_qlist_output_ {
	__le16	error_code;
	__le16	req_type;
	__le16	seq_id;
	__le16	resp_len;
	u8	num_tests;
	u8	available_tests;
	#define SELFTEST_QLIST_RESP_AVAILABLE_TESTS_NVM_TEST                 0x1UL
	#define SELFTEST_QLIST_RESP_AVAILABLE_TESTS_LINK_TEST                0x2UL
	#define SELFTEST_QLIST_RESP_AVAILABLE_TESTS_REGISTER_TEST            0x4UL
	#define SELFTEST_QLIST_RESP_AVAILABLE_TESTS_MEMORY_TEST              0x8UL
	#define SELFTEST_QLIST_RESP_AVAILABLE_TESTS_PCIE_SERDES_TEST         0x10UL
	#define SELFTEST_QLIST_RESP_AVAILABLE_TESTS_ETHERNET_SERDES_TEST     0x20UL
	u8	offline_tests;
	#define SELFTEST_QLIST_RESP_OFFLINE_TESTS_NVM_TEST                 0x1UL
	#define SELFTEST_QLIST_RESP_OFFLINE_TESTS_LINK_TEST                0x2UL
	#define SELFTEST_QLIST_RESP_OFFLINE_TESTS_REGISTER_TEST            0x4UL
	#define SELFTEST_QLIST_RESP_OFFLINE_TESTS_MEMORY_TEST              0x8UL
	#define SELFTEST_QLIST_RESP_OFFLINE_TESTS_PCIE_SERDES_TEST         0x10UL
	#define SELFTEST_QLIST_RESP_OFFLINE_TESTS_ETHERNET_SERDES_TEST     0x20UL
	u8	unused_0;
	__le16	test_timeout;
	u8	unused_1[2];
	char	test0_name[32];
	char	test1_name[32];
	char	test2_name[32];
	char	test3_name[32];
	char	test4_name[32];
	char	test5_name[32];
	char	test6_name[32];
	char	test7_name[32];
	u8	unused_2[7];
	u8	valid;
} hwrm_selftest_qlist_output_t;

/* hwrm_selftest_exec_input (size:192b/24B) */
typedef struct hwrm_selftest_exec_input_ {
	__le16	req_type;
	__le16	cmpl_ring;
	__le16	seq_id;
	__le16	target_id;
	__le64	resp_addr;
	u8	flags;
	#define SELFTEST_EXEC_REQ_FLAGS_NVM_TEST                 0x1UL
	#define SELFTEST_EXEC_REQ_FLAGS_LINK_TEST                0x2UL
	#define SELFTEST_EXEC_REQ_FLAGS_REGISTER_TEST            0x4UL
	#define SELFTEST_EXEC_REQ_FLAGS_MEMORY_TEST              0x8UL
	#define SELFTEST_EXEC_REQ_FLAGS_PCIE_SERDES_TEST         0x10UL
	#define SELFTEST_EXEC_REQ_FLAGS_ETHERNET_SERDES_TEST     0x20UL
	u8	unused_0[7];
} hwrm_selftest_exec_input_t;

/* hwrm_selftest_exec_output (size:128b/16B) */
typedef struct hwrm_selftest_exec_output_ {
	__le16	error_code;
	__le16	req_type;
	__le16	seq_id;
	__le16	resp_len;
	u8	requested_tests;
	#define SELFTEST_EXEC_RESP_REQUESTED_TESTS_NVM_TEST                 0x1UL
	#define SELFTEST_EXEC_RESP_REQUESTED_TESTS_LINK_TEST                0x2UL
	#define SELFTEST_EXEC_RESP_REQUESTED_TESTS_REGISTER_TEST            0x4UL
	#define SELFTEST_EXEC_RESP_REQUESTED_TESTS_MEMORY_TEST              0x8UL
	#define SELFTEST_EXEC_RESP_REQUESTED_TESTS_PCIE_SERDES_TEST         0x10UL
	#define SELFTEST_EXEC_RESP_REQUESTED_TESTS_ETHERNET_SERDES_TEST     0x20UL
	u8	test_success;
	#define SELFTEST_EXEC_RESP_TEST_SUCCESS_NVM_TEST                 0x1UL
	#define SELFTEST_EXEC_RESP_TEST_SUCCESS_LINK_TEST                0x2UL
	#define SELFTEST_EXEC_RESP_TEST_SUCCESS_REGISTER_TEST            0x4UL
	#define SELFTEST_EXEC_RESP_TEST_SUCCESS_MEMORY_TEST              0x8UL
	#define SELFTEST_EXEC_RESP_TEST_SUCCESS_PCIE_SERDES_TEST         0x10UL
	#define SELFTEST_EXEC_RESP_TEST_SUCCESS_ETHERNET_SERDES_TEST     0x20UL
	u8	unused_0[5];
	u8	valid;
} hwrm_selftest_exec_output_t;

/* hwrm_port_phy_mdio_bus_release_input (size:192b/24B) */
typedef struct hwrm_port_phy_mdio_bus_release_input_ {
    __le16  req_type;
    __le16  cmpl_ring;
    __le16  seq_id;
    __le16  target_id;
    __le64  resp_addr;
    __le16  port_id;
    __le16  client_id;
    u8  unused_0[4];
} hwrm_port_phy_mdio_bus_release_input_t;

/* hwrm_port_phy_mdio_bus_release_output (size:128b/16B) */
typedef struct hwrm_port_phy_mdio_bus_release_output_ {
    __le16  error_code;
    __le16  req_type;
    __le16  seq_id;
    __le16  resp_len;
    __le16  unused_0;
    __le16  clients_id;
    u8  unused_1[3];
    u8  valid;
} hwrm_port_phy_mdio_bus_release_output_t;

/* hwrm_port_phy_mdio_bus_acquire_input (size:192b/24B) */
typedef struct hwrm_port_phy_mdio_bus_acquire_input_ {
    __le16  req_type;
    __le16  cmpl_ring;
    __le16  seq_id;
    __le16  target_id;
    __le64  resp_addr;
    __le16  port_id;
    __le16  client_id;
    __le16  mdio_bus_timeout;
    u8  unused_0[2];
} hwrm_port_phy_mdio_bus_acquire_input_t;

/* hwrm_port_phy_mdio_bus_acquire_output (size:128b/16B) */
typedef struct hwrm_port_phy_mdio_bus_acquire_output_ {
    __le16  error_code;
    __le16  req_type;
    __le16  seq_id;
    __le16  resp_len;
    __le16  unused_0;
    __le16  client_id;
    u8  unused_1[3];
    u8  valid;
} hwrm_port_phy_mdio_bus_acquire_output_t;


/* definition for Read, display SFP EEPROM */
#define SFP_EEPROM_SIZE                   256
#define SFP_EEPROM_READ_LEN               16
#define SFP_SN_SZ                         20

#define SFP_EEPROM_VENDOR_PART_NO_OFFSET  40
#define SFP_VENDOR_PN_SZ                  16

#define SFP_EEPROM_SERIAL_NO_OFFSET       68
#define SFP_EEPROM_SERIAL_NO_LENGTH       16

#define SFP_EEPROM_VENDOR_NAME_OFFSET     20
#define SFP_EEPROM_VENDOR_NAME_LEN        16

#define SFP_EEPROM_A2_PID_OFFSET          192
#define SFP_EEPROM_A2_PID_LEN             20

#define SZ_PN_SN_SH                       124
#define SZ_MSG_ERR                        18

#define FMT_PN       "%-18s"
#define FMT_STATE    "%-10s"
#define FMT_SN       "%-18s"
#define MSG_EE_ERR   "SFP READ(%X)ERR!"



void bnxt_impl_deinit_netlink (void);
boolean bnxt_impl_init_netlink (void);
extern int fugazi_eth_get_ifindex(uint16_t);
extern int bnxt_netlink_i2c_read(uint8_t, uint32_t, uint16_t, uint16_t, 
                          uint16_t, uint8_t, uint8_t *);
extern int bnxt_netlink_i2c_write(uint8_t, uint32_t, uint16_t, uint16_t, 
                           uint16_t, uint8_t, uint8_t *);
extern int bnxt_sfp_detect(uint16_t, uint32_t);
extern int bnxt_netlink_sideband_read(uint16_t, uint32_t);
extern int bnxt_netlink_sideband_tx_dis(uint16_t, uint32_t, uint16_t);
extern int bnxt_netlink_reg_test(uint16_t, uint32_t);
extern int fugazi_sfp_present(uint16_t);
extern int bnxt_bus_release(uint16_t, uint32_t);
extern int bnxt_bus_acquire(uint16_t, uint32_t);
extern int bcm57412_sideband_tx_dis(int);
extern int bcm57412_sideband_tx_dis_setup(int, int);
extern int bcm57412_mdio_bus_release(int);
extern int bcm57412_mdio_bus_acquire(int);
extern int sfp_display_info(void);
extern int sfp_display_cookie(int);
#endif //__DIAG_BCM57412_UTILS_H__
/*
 *------------------------------------------------------------------
 * $Log: diag_bcm57412_utils.h,v $
 * Revision 1.2  2021/06/02 08:22:34  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.4.4  2021/03/19 18:35:09  pdoong
 * Add Dump SFP Module Info utility to display SFP Vendor PN, S/N, Vendor name, PID
 *
 * Revision 1.1.4.3  2020/10/22 02:30:53  iachang
 * Fixed compile re-define issue.
 *
 * Revision 1.1.4.2  2020/08/26 02:37:48  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.2.9  2020/08/05 09:02:41  iachang
 * Update Copyright to 2020
 *
 * Revision 1.1.2.8  2020/07/03 07:34:20  iachang
 * Support bcm57412_mdio_bus_release() and bcm57412_mdio_bus_acquire()
 * Move those funcitons from diag_bcm57412_test.c to diag_bcm57412_utils.c
 *
 * Revision 1.1.2.7  2020/04/22 07:11:04  iachang
 * Add BCM57412 mdio_bus_acquire function
 *
 * Revision 1.1.2.6  2020/03/06 05:53:15  iachang
 * Implement BCM57412 sideband tx_dis setup function.
 *
 * Revision 1.1.2.5  2020/03/04 08:29:18  iachang
 * Correct BCM82757 Side_band register dump
 *
 * Revision 1.1.2.4  2020/01/09 08:16:11  iachang
 * Add bcm57412_mdio_bus_release function, and modify BCM57412 sideband tx_dis utility.
 *
 * Revision 1.1.2.3  2019/11/14 08:29:11  iachang
 * Implement SFP present function.
 *
 * Revision 1.1.2.2  2019/10/16 08:53:06  iachang
 * Port BCM57412 Register test via Netlink to replace Broadcom Cdiag tool
 *
 * Revision 1.1.2.1  2019/09/27 08:03:58  iachang
 * Changed "BCM57412 SFP i2c Test" from Broadcom CDiag to Netlink.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
