/* $Id: bcm82752_test.h,v 1.2 2019/08/06 06:56:11 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_1RU/bcm82752_test.h,v $
 *-----------------------------------------------------------------------------
 * bcm82752_api.h - Header for BCM 10G PHY bcm82752 Test.
 *
 *
 * Feb 2019, Leschen
 *
 * Copyright (c) 2016 - 2019 by Cisco Systems, Inc.
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
Revision 1.2  2019/08/06 06:56:11  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.1.2.1  2019/03/12 07:41:51  leschen
Initial check in to support BCM82752


$Endlog$
*/
