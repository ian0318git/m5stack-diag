/* $Id: bcm82752_test.h,v 1.2 2018/05/18 09:24:56 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/cavium/bcm82752_test.h,v $
 *-----------------------------------------------------------------------------
 * bcm82752_api.h - Header for BCM 10G PHY bcm82752 Test.
 *
 *
 * June 2016, Mecca Ho
 *
 * Copyright (c) 2016 - 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */


/* BCM82752 register map */
/* XFI PMA/PMD (devive 1) Register table */
#define BCM82752_XFI_PMD_CTRL_REG            (BCM82752_PMD_CONTROL_REG | \
                                             (BCM82752_DEV_PMA << 16))
#define BCM82752_XFI_PMD_STAT_REG            (BCM82752_PMD_STATUS_REG | \
                                             (BCM82752_DEV_PMA << 16))
#define BCM82752_XFI_PMD_ID_0_REG            (BCM82752_PMD_ID_MSB_REG | \
                                             (BCM82752_DEV_PMA << 16))
#define BCM82752_XFI_PMD_ID_1_REG            (BCM82752_PMD_ID_LSB_REG | \
                                             (BCM82752_DEV_PMA << 16))
#define BCM82752_XFI_PMD_SPEED_ABIL_REG      (BCM82752_PMD_SPEED_ABILITY_REG | \
                                             (BCM82752_DEV_PMA << 16))
#define BCM82752_XFI_PMD_DEVICE_IN_PAK_1_REG (BCM82752_DEVICES_IN_PACKAGE_1_REG | \
                                             (BCM82752_DEV_PMA << 16))
#define BCM82752_XFI_PMD_DEVICE_IN_PAK_2_REG (BCM82752_DEVICES_IN_PACKAGE_2_REG | \
                                             (BCM82752_DEV_PMA << 16))
#define BCM82752_XFI_PMD_STAT_2_REG          (BCM82752_PMD_STATUS_2_REG | \
                                             (BCM82752_DEV_PMA << 16))
#define BCM82752_XFI_PMD_TRANSMIT_DIS_REG    (BCM82752_PMD_TRANSMIT_DISABLE_REG | \
                                             (BCM82752_DEV_PMA << 16))
#define BCM82752_XFI_PMD_RX_SIG_DETECT_REG   (BCM82752_PMD_RECEIVE_SIGNAL_DETECT_REG | \
                                             (BCM82752_DEV_PMA << 16))
#define BCM82752_XFI_PMD_EXT_ABIL_REG        (BCM82752_PMD_EXTENDED_ABILITY_REG | \
                                             (BCM82752_DEV_PMA << 16))
#define BCM82752_XFI_PMD_ORG_UNI_ID_0_REG    (BCM82752_PMD_ORGANIZATIONALLY_UNIQUE_ID_MSB_REG | \
                                             (BCM82752_DEV_PMA << 16))
#define BCM82752_XFI_PMD_ORG_UNI_ID_1_REG    (BCM82752_PMD_ORGANIZATIONALLY_UNIQUE_ID_LSB_REG | \
                                             (BCM82752_DEV_PMA << 16))

/* XFI PCS (devive 3) Register table */
#define BCM82752_XFI_PCS_CTRL_1_REG          (BCM82752_PCS_CONTROL_1_REG | \
                                             (BCM82752_DEV_PCS << 16))
#define BCM82752_XFI_PCS_STAT_1_REG          (BCM82752_PCS_STATUS_1_REG | \
                                             (BCM82752_DEV_PCS << 16))
#define BCM82752_XFI_PCS_ID_0_REG            (BCM82752_PCS_ID_MSB_REG | \
                                             (BCM82752_DEV_PCS << 16))
#define BCM82752_XFI_PCS_ID_1_REG            (BCM82752_PCS_ID_LSB_REG | \
                                             (BCM82752_DEV_PCS << 16))
#define BCM82752_XFI_PCS_SPEED_ABIL_REG      (BCM82752_SPEED_ABILITY_REG | \
                                             (BCM82752_DEV_PCS << 16))
#define BCM82752_XFI_PCS_DEVICE_IN_PAK_1_REG (BCM82752_DEVICES_IN_PACKAGE_1_REG | \
                                             (BCM82752_DEV_PCS << 16))
#define BCM82752_XFI_PCS_DEVICE_IN_PAK_2_REG (BCM82752_DEVICES_IN_PACKAGE_2_REG | \
                                             (BCM82752_DEV_PCS << 16))
#define BCM82752_XFI_PCS_CTRL_2_REG          (BCM82752_PCS_CONTROL_2_REG | \
                                             (BCM82752_DEV_PCS << 16))
#define BCM82752_XFI_PCS_STAT_2_REG          (BCM82752_PCS_STATUS_2_REG | \
                                             (BCM82752_DEV_PCS << 16))
#define BCM82752_XFI_PCS_ORG_UNI_ID_0_REG    (BCM82752_ORGANIZATIONALLY_UNIQUE_ID_MSB_REG | \
                                             (BCM82752_DEV_PCS << 16))
#define BCM82752_XFI_PCS_ORG_UNI_ID_1_REG    (BCM82752_ORGANIZATIONALLY_UNIQUE_ID_LSB_REG | \
                                             (BCM82752_DEV_PCS << 16))
#define BCM82752_XFI_PCS_10G_X_STAT_REG      (BCM82752_10GBASE_X_PCS_STATUS_REG | \
                                             (BCM82752_DEV_PCS << 16))
#define BCM82752_XFI_PCS_10G_R_STAT_REG      (BCM82752_10GBASE_R_PCS_STATUS_REG | \
                                             (BCM82752_DEV_PCS << 16))
#define BCM82752_XFI_PCS_10G_R_STAT_2_REG    (BCM82752_10GBASE_R_PCS_STATUS_2_REG | \
                                             (BCM82752_DEV_PCS << 16))
#define BCM82752_XFI_PCS_10G_R_JIT_TEST_SEED_A0_REG (BCM82752_10GBASE_R_PCS_JITTER_TEST_SEED_A0_REG | \
                                                    (BCM82752_DEV_PCS << 16))
#define BCM82752_XFI_PCS_10G_R_JIT_TEST_SEED_A1_REG (BCM82752_10GBASE_R_PCS_JITTER_TEST_SEED_A1_REG | \
                                                    (BCM82752_DEV_PCS << 16))
#define BCM82752_XFI_PCS_10G_R_JIT_TEST_SEED_A2_REG (BCM82752_10GBASE_R_PCS_JITTER_TEST_SEED_A2_REG | \
                                                    (BCM82752_DEV_PCS << 16))
#define BCM82752_XFI_PCS_10G_R_JIT_TEST_SEED_A3_REG (BCM82752_10GBASE_R_PCS_JITTER_TEST_SEED_A3_REG | \
                                                    (BCM82752_DEV_PCS << 16))
#define BCM82752_XFI_PCS_10G_R_JIT_TEST_SEED_B0_REG (BCM82752_10GBASE_R_PCS_JITTER_TEST_SEED_B0_REG | \
                                                    (BCM82752_DEV_PCS << 16))
#define BCM82752_XFI_PCS_10G_R_JIT_TEST_SEED_B1_REG (BCM82752_10GBASE_R_PCS_JITTER_TEST_SEED_B1_REG | \
                                                    (BCM82752_DEV_PCS << 16))
#define BCM82752_XFI_PCS_10G_R_JIT_TEST_SEED_B2_REG (BCM82752_10GBASE_R_PCS_JITTER_TEST_SEED_B2_REG | \
                                                    (BCM82752_DEV_PCS << 16))
#define BCM82752_XFI_PCS_10G_R_JIT_TEST_SEED_B3_REG (BCM82752_10GBASE_R_PCS_JITTER_TEST_SEED_B3_REG | \
                                                    (BCM82752_DEV_PCS << 16))
#define BCM82752_XFI_PCS_10G_R_JIT_TEST_CTRL_REG    (BCM82752_10GBASE_R_PCS_JITTER_TEST_CONTROL_REG | \
                                                    (BCM82752_DEV_PCS << 16))
#define BCM82752_XFI_PCS_10G_R_JIT_TEST_ERR_CNT_REG (BCM82752_10GBASE_R_PCS_JITTER_TEST_ERR_CNT_REG | \
                                                    (BCM82752_DEV_PCS << 16))

/* SFI PMA/PMD (devive 1) Register table */
#define BCM82752_SFI_PMD_CTRL_REG            (BCM82752_PMD_CONTROL_REG | \
                                             (BCM82752_DEV_PMA << 16))
#define BCM82752_SFI_PMD_STAT_REG            (BCM82752_PMD_STATUS_REG | \
                                             (BCM82752_DEV_PMA << 16))
#define BCM82752_SFI_PMD_ID_0_REG            (BCM82752_PMD_ID_MSB_REG | \
                                             (BCM82752_DEV_PMA << 16))
#define BCM82752_SFI_PMD_ID_1_REG            (BCM82752_PMD_ID_LSB_REG | \
                                             (BCM82752_DEV_PMA << 16))
#define BCM82752_SFI_PMD_SPEED_ABIL_REG      (BCM82752_PMD_SPEED_ABILITY_REG | \
                                             (BCM82752_DEV_PMA << 16))
#define BCM82752_SFI_PMD_DEVICE_IN_PAK_1_REG (BCM82752_DEVICES_IN_PACKAGE_1_REG | \
                                             (BCM82752_DEV_PMA << 16))
#define BCM82752_SFI_PMD_DEVICE_IN_PAK_2_REG (BCM82752_DEVICES_IN_PACKAGE_2_REG | \
                                             (BCM82752_DEV_PMA << 16))
#define BCM82752_SFI_PMD_CTRL_2_REG          (BCM82752_PMD_CONTROL_2_REG | \
                                             (BCM82752_DEV_PMA << 16))
#define BCM82752_SFI_PMD_STAT_2_REG          (BCM82752_PMD_STATUS_2_REG | \
                                             (BCM82752_DEV_PMA << 16))
#define BCM82752_SFI_PMD_TRANSMIT_DIS_REG    (BCM82752_PMD_TRANSMIT_DISABLE_REG | \
                                             (BCM82752_DEV_PMA << 16))
#define BCM82752_SFI_PMD_RX_SIG_DETECT_REG   (BCM82752_PMD_RECEIVE_SIGNAL_DETECT_REG | \
                                             (BCM82752_DEV_PMA << 16))
#define BCM82752_SFI_PMD_EXT_ABIL_REG        (BCM82752_PMD_EXTENDED_ABILITY_REG | \
                                             (BCM82752_DEV_PMA << 16))
#define BCM82752_SFI_PMD_ORG_UNI_ID_0_REG    (BCM82752_PMD_ORGANIZATIONALLY_UNIQUE_ID_MSB_REG | \
                                             (BCM82752_DEV_PMA << 16))
#define BCM82752_SFI_PMD_ORG_UNI_ID_1_REG    (BCM82752_PMD_ORGANIZATIONALLY_UNIQUE_ID_LSB_REG | \
                                             (BCM82752_DEV_PMA << 16))

/* SFI PCS (devive 3) Register table */
#define BCM82752_SFI_PCS_CTRL_1_REG          (BCM82752_PCS_CONTROL_1_REG | \
                                             (BCM82752_DEV_PCS << 16))
#define BCM82752_SFI_PCS_STAT_1_REG          (BCM82752_PCS_STATUS_1_REG | \
                                             (BCM82752_DEV_PCS << 16))
#define BCM82752_SFI_PCS_ID_0_REG            (BCM82752_PCS_ID_MSB_REG | \
                                             (BCM82752_DEV_PCS << 16))
#define BCM82752_SFI_PCS_ID_1_REG            (BCM82752_PCS_ID_LSB_REG | \
                                             (BCM82752_DEV_PCS << 16))
#define BCM82752_SFI_PCS_SPEED_ABIL_REG      (BCM82752_SPEED_ABILITY_REG | \
                                             (BCM82752_DEV_PCS << 16))
#define BCM82752_SFI_PCS_DEVICE_IN_PAK_1_REG (BCM82752_DEVICES_IN_PACKAGE_1_REG | \
                                             (BCM82752_DEV_PCS << 16))
#define BCM82752_SFI_PCS_DEVICE_IN_PAK_2_REG (BCM82752_DEVICES_IN_PACKAGE_2_REG | \
                                             (BCM82752_DEV_PCS << 16))
#define BCM82752_SFI_PCS_CTRL_2_REG          (BCM82752_PCS_CONTROL_2_REG | \
                                             (BCM82752_DEV_PCS << 16))
#define BCM82752_SFI_PCS_STAT_2_REG          (BCM82752_PCS_STATUS_2_REG | \
                                             (BCM82752_DEV_PCS << 16))
#define BCM82752_SFI_PCS_10G_R_EEE_CAP_REG   (BCM82752_10GBASE_R_PCS_EEE_CAPABILITY_REG | \
                                             (BCM82752_DEV_PCS << 16))
#define BCM82752_SFI_PCS_WAKE_ERR_CNT_REG    (BCM82752_10GBASE_R_PCS_WAKE_ERROR_COUNTER_REG | \
                                             (BCM82752_DEV_PCS << 16))
#define BCM82752_SFI_PCS_10G_R_STAT_REG      (BCM82752_10GBASE_R_PCS_STATUS_REG | \
                                             (BCM82752_DEV_PCS << 16))
#define BCM82752_SFI_PCS_10G_R_STAT_2_REG    (BCM82752_10GBASE_R_PCS_STATUS_2_REG | \
                                             (BCM82752_DEV_PCS << 16))
#define BCM82752_SFI_PCS_10G_R_JIT_TEST_SEED_A0_REG (BCM82752_10GBASE_R_PCS_JITTER_TEST_SEED_A0_REG | \
                                                    (BCM82752_DEV_PCS << 16))
#define BCM82752_SFI_PCS_10G_R_JIT_TEST_SEED_A1_REG (BCM82752_10GBASE_R_PCS_JITTER_TEST_SEED_A1_REG | \
                                                    (BCM82752_DEV_PCS << 16))
#define BCM82752_SFI_PCS_10G_R_JIT_TEST_SEED_A2_REG (BCM82752_10GBASE_R_PCS_JITTER_TEST_SEED_A2_REG | \
                                                    (BCM82752_DEV_PCS << 16))
#define BCM82752_SFI_PCS_10G_R_JIT_TEST_SEED_A3_REG (BCM82752_10GBASE_R_PCS_JITTER_TEST_SEED_A3_REG | \
                                                    (BCM82752_DEV_PCS << 16))
#define BCM82752_SFI_PCS_10G_R_JIT_TEST_SEED_B0_REG (BCM82752_10GBASE_R_PCS_JITTER_TEST_SEED_B0_REG | \
                                                    (BCM82752_DEV_PCS << 16))
#define BCM82752_SFI_PCS_10G_R_JIT_TEST_SEED_B1_REG (BCM82752_10GBASE_R_PCS_JITTER_TEST_SEED_B1_REG | \
                                                    (BCM82752_DEV_PCS << 16))
#define BCM82752_SFI_PCS_10G_R_JIT_TEST_SEED_B2_REG (BCM82752_10GBASE_R_PCS_JITTER_TEST_SEED_B2_REG | \
                                                    (BCM82752_DEV_PCS << 16))
#define BCM82752_SFI_PCS_10G_R_JIT_TEST_SEED_B3_REG (BCM82752_10GBASE_R_PCS_JITTER_TEST_SEED_B3_REG | \
                                                    (BCM82752_DEV_PCS << 16))
#define BCM82752_SFI_PCS_10G_R_JIT_TEST_CTRL_REG    (BCM82752_10GBASE_R_PCS_JITTER_TEST_CONTROL_REG | \
                                                    (BCM82752_DEV_PCS << 16))
#define BCM82752_SFI_PCS_10G_R_JIT_TEST_ERR_CNT_REG (BCM82752_10GBASE_R_PCS_JITTER_TEST_ERR_CNT_REG | \
                                                    (BCM82752_DEV_PCS << 16))
/* BCM82757 register map */
/* No need to separate XFI/SFI interface on BCM82757 register. */
#define BCM82757_MAIN_CLK_RST_CTRL 0x8200
#define BCM82757_GEN_CTRL_REG1 0x8201
#define BCM82757_GEN_CTRL_REG2 0x8202
#define BCM82757_GEN_CTRL_REG3 0x8203

#define BCM82757_MDIO_POR_REG 0x82FE
#define BCM82757_MICRO_BOOT_REG 0x82FF

#define BCM82757_CHIP_ID_REG 0x8B00
#define BCM82757_CHIP_REV_REG 0x8B01
#define BCM82757_CHIP_CONFIG_REG 0x8B02

#define BCM82757_MODULE_MAIN_CTRL_REG 0x8700
#define BCM82757_MODULE_STATUS_REG 0x8701

#define BCM82757_PAD_MDIO1_CTRL_REG 0x8A30
#define BCM82757_PAD_MDIO2_CTRL_REG 0x8A32
#define BCM82757_PAD_ADR0_CTRL_REG 0x8A34
#define BCM82757_PAD_ADR1_CTRL_REG 0x8A36
#define BCM82757_PAD_ADR2_CTRL_REG 0x8A38

#define BCM82757_MST_CODE_RAM_MEM_ADDR 0x8400
#define BCM82757_MST_DATA_RAM_MEM_ADDR 0x8401
#define BCM82757_MST_VECTOR_TBL_ADDR   0x8402

#define BCM82757_LMI_RESET_CTRL_REG 0x9000
#define BCM82757_LMI_CMD_REG 0x9001
#define BCM82757_LMI_ADDR_REG 0x9002
#define BCM82757_LMI_DATA_REG 0x9003

#define BCM82757_SCRATCH_REG0 0x826C
#define BCM82757_SCRATCH_REG1 0x826D
#define BCM82757_SCRATCH_REG2 0x826E
#define BCM82757_SCRATCH_REG3 0x826F

#define BCM82757_SFI_MAIN_CLK_RST_CTRL    (BCM82757_MAIN_CLK_RST_CTRL | (BCM82752_DEV_PMA << 16))
#define BCM82757_SFI_GEN_CTRL_REG1        (BCM82757_GEN_CTRL_REG1 | (BCM82752_DEV_PMA << 16))
#define BCM82757_SFI_GEN_CTRL_REG2        (BCM82757_GEN_CTRL_REG2 | (BCM82752_DEV_PMA << 16))
#define BCM82757_SFI_GEN_CTRL_REG3        (BCM82757_GEN_CTRL_REG3 | (BCM82752_DEV_PMA << 16))

#define BCM82757_SFI_MDIO_POR_REG        (BCM82757_MDIO_POR_REG | (BCM82752_DEV_PMA << 16))
#define BCM82757_SFI_MICRO_BOOT_REG          (BCM82757_MICRO_BOOT_REG | (BCM82752_DEV_PMA << 16))

#define BCM82757_SFI_CHIP_ID_REG         (BCM82757_CHIP_ID_REG | (BCM82752_DEV_PMA << 16))
#define BCM82757_SFI_CHIP_REV_REG        (BCM82757_CHIP_REV_REG | (BCM82752_DEV_PMA << 16))
#define BCM82757_SFI_CHIP_CONFIG_REG     (BCM82757_CHIP_CONFIG_REG | (BCM82752_DEV_PMA << 16))

#define BCM82757_SFI_MODULE_MAIN_CTRL_REG    (BCM82757_MODULE_MAIN_CTRL_REG | (BCM82752_DEV_PMA << 16))
#define BCM82757_SFI_MODULE_STATUS_REG       (BCM82757_MODULE_STATUS_REG | (BCM82752_DEV_PMA << 16))

#define BCM82757_SFI_PAD_MDIO1_CTRL_REG (BCM82757_PAD_MDIO1_CTRL_REG | (BCM82752_DEV_PMA << 16))
#define BCM82757_SFI_PAD_MDIO2_CTRL_REG (BCM82757_PAD_MDIO2_CTRL_REG | (BCM82752_DEV_PMA << 16))
#define BCM82757_SFI_PAD_ADR0_CTRL_REG  (BCM82757_PAD_ADR0_CTRL_REG | (BCM82752_DEV_PMA << 16))
#define BCM82757_SFI_PAD_ADR1_CTRL_REG  (BCM82757_PAD_ADR1_CTRL_REG | (BCM82752_DEV_PMA << 16))
#define BCM82757_SFI_PAD_ADR2_CTRL_REG  (BCM82757_PAD_ADR2_CTRL_REG | (BCM82752_DEV_PMA << 16))

#define BCM82757_SFI_MST_CODE_RAM_MEM_ADDR (BCM82757_MST_CODE_RAM_MEM_ADDR | (BCM82752_DEV_PMA << 16))
#define BCM82757_SFI_MST_DATA_RAM_MEM_ADDR (BCM82757_MST_DATA_RAM_MEM_ADDR | (BCM82752_DEV_PMA << 16))
#define BCM82757_SFI_MST_VECTOR_TBL_ADDR   (BCM82757_MST_VECTOR_TBL_ADDR | (BCM82752_DEV_PMA << 16))

#define BCM82757_SFI_LMI_RESET_CTRL_REG (BCM82757_LMI_RESET_CTRL_REG | (BCM82752_DEV_PMA << 16))
#define BCM82757_SFI_LMI_CMD_REG        (BCM82757_LMI_CMD_REG | (BCM82752_DEV_PMA << 16))
#define BCM82757_SFI_LMI_ADDR_REG       (BCM82757_LMI_ADDR_REG | (BCM82752_DEV_PMA << 16))
#define BCM82757_SFI_LMI_DATA_REG       (BCM82757_LMI_DATA_REG | (BCM82752_DEV_PMA << 16))

#define BCM82757_XFI_SCRATCH_PAD_REG0       (BCM82757_SCRATCH_REG0 | (BCM82752_DEV_PMA << 16))
#define BCM82757_XFI_SCRATCH_PAD_REG1       (BCM82757_SCRATCH_REG1 | (BCM82752_DEV_PMA << 16))
#define BCM82757_XFI_SCRATCH_PAD_REG2       (BCM82757_SCRATCH_REG2 | (BCM82752_DEV_PMA << 16))
#define BCM82757_XFI_SCRATCH_PAD_REG3       (BCM82757_SCRATCH_REG3 | (BCM82752_DEV_PMA << 16))

extern int ten_g_bcm8275x_test (int);
/*-------------------------------------------------
$Log: bcm82752_test.h,v $
Revision 1.2  2018/05/18 09:24:56  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.1.2.7  2017/09/07 06:46:46  meho
1. Fixed dump BCM82752 register bug.
2. Added dump BCM82757 register utility.
3. Added BCM82757 indirect register r/w utility.

Revision 1.1.2.6  2017/04/10 05:27:24  meho
Integrated BCM82752/82757 API.

Revision 1.1.2.5  2017/03/30 05:02:24  meho
Fixed BCM82752 register test bug.

Revision 1.1.2.4  2016/11/29 06:27:52  meho
Changed submenu name and code clean up.

Revision 1.1.2.3  2016/07/25 11:28:30  meho
Added register dump utility for BCM82752.

Revision 1.1.2.2  2016/07/12 08:40:58  meho
1. Added BCM54194/BCM82752 register tests.
2. Added BCM54194 internal/external-copper loopback configuration.

Revision 1.1.2.1  2016/07/07 09:04:30  meho
1. Added BCM54194 RDB register r/w utility.
2. Added GE PHY internal/external loopback skeleton.
3. Added 10GE PHY internal/external loopback skeleton.



$Endlog$
*/
