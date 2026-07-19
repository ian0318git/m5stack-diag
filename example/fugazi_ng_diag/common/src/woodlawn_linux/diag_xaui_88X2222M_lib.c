/* $Id: diag_xaui_88X2222M_lib.c,v 1.7 2015/02/14 12:48:41 kodko Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn_linux/diag_xaui_88X2222M_lib.c,v $
 *-----------------------------------------------------------------------------
 * diag_xaui_88X2222M_lib.c - Utility Menu and Functions for Woodlawn PHY 88X2222M
 *
 * February 2012, Kody Ko
 * Copyright (c) 2015 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */
#include "error.h"
#include "common.h"
#include "types.h"
#include "menu.h"
#include "cvmx.h"
#include "platform_smi.h"
#include "platform_smi_lib.h"
#include "diag_fpga_lib.h"
#include "diag_xaui_88X2222M_lib.h"
#include "cvmx-mdio.h"

/*****************************************************************************
 *  Functions Declaration
 *****************************************************************************/

extern void msleep(unsigned long);

int mrvl_88X2222M_en_ext_lpbk(void);
int mrvl_88X2222M_sel_port2(void);
int mrvl_88X2222M_is_sfp_plus_present(void);
int turn_off_mrvl_88X2222M_i2c(void);
int mrvl_88X2222M_read_i2c(int, int, char *);
int mrvl_88X2222M_disp_sfp_eeprom(void);
int switch_sfp_plus_led(int);
int enable_mrvl2222m_macsec_power(void);

/******************************************************************************
 *
 * Function: mrvl_88X2222M_disp_sfp_eeprom
 *
 * Description: This API function displays SFP+ EEPROM
 *
 * Inputs      : void
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
int mrvl_88X2222M_disp_sfp_eeprom (void)
{
    int rv, sku_id;
    int reg_addr;
    unsigned char buf;

    sku_id = get_sku_id();

    if (sku_id == WOODLAWN_6GE) {
        printf("This SKU doesn't support SFP+\n");
        return (PASSED);
    }

    /* Check if SFP+ is present */
    if (mrvl_88X2222M_is_sfp_plus_present() == FALSE) {
        printf("SFP+ is not detected\n");
        return (PASSED);
    }

    /* Now, display SFP+ eeprom */
    printf("SFP+ EEPROM contents:\n\n");

    printf("0x00: ");
    for (reg_addr = 0; reg_addr < SFP_EEPROM_SIZE; reg_addr++) {
        if (reg_addr && ((reg_addr % 16) == 0)) {
            printf("\n");
            printf("0x%02x: ", reg_addr);
        }

        rv = mrvl_88X2222M_read_i2c(SFP_EEPROM_ADDR, reg_addr, (char *)&buf);
        if (rv != PASSED) {
            printf("Read reg fail at offset 0x%.8x\n", reg_addr);
            return (FAILED);
        } else {
            printf("%02x ", buf);
        }
    }
    printf("\n");

    return (PASSED);
}

/******************************************************************************
 *
 * Function: mrvl_88X2222M_read_i2c
 *
 * Description: This API function reads I2C data through 88X2222 chip
 *
 * Inputs      : addr - Slave I2C address
 *               reg_addr - Register address
 *               buf - data buffer
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
int mrvl_88X2222M_read_i2c (int addr, int reg_addr, char *buf)
{
    uint reg_val, twsi_stats;
    ten_g_phy_t phy_88X2222M = {MRV88X2222M_REG_DEVICE_1,
                                MRVL_88X2222M_SMI2_PORT0_ADDR};
    int timeout = EEPROM_OP_TIMEOUT;

    /* Make sure that TWSI status is idle */
    do {
        if ((read_ten_g_phy_reg(EEPROM_READ_DATA_REGISTER, MRVL_88X2222M_PHY_REG_LEN,
                                &reg_val, &phy_88X2222M)) == FAILED) {
            return (FAILED);
        }

        twsi_stats = (reg_val >> EEPROM_TWSI_STATUS_SHIFT) & 0x7;

        if (twsi_stats != EEPROM_TWSI_STATUS_IN_PROG) {
            break;
        }
        msleep(10);
    } while (timeout--);

    if (timeout <= 0) {
        printf("%s: TWSI Status is not ready (%d)\n", __FUNCTION__, twsi_stats);
        return (FAILED);
    }

    /* Slave Address */
    reg_val = (addr) << EEPROM_SLAVE_ADDRESS_SHIFT;
    reg_val |= reg_addr;
    reg_val |= EEPROM_SLAVE_COMMAND_READ;

    /* Write to EEPROM address register */
    if ((write_ten_g_phy_reg(EEPROM_ADDRESS_REGISTER, MRVL_88X2222M_PHY_REG_LEN,
                             reg_val, &phy_88X2222M)) == FAILED) {
        return (FAILED);
    }

    /* Make sure that TWSI status is idle */
    do {
        if ((read_ten_g_phy_reg(EEPROM_READ_DATA_REGISTER, MRVL_88X2222M_PHY_REG_LEN,
                                &reg_val, &phy_88X2222M)) == FAILED) {
            return (FAILED);
        }

        twsi_stats = (reg_val >> EEPROM_TWSI_STATUS_SHIFT) & 0x7;

        if (twsi_stats != EEPROM_TWSI_STATUS_IN_PROG) {
            if (twsi_stats == EEPROM_TWSI_STATUS_DONE) {
                /* Command is executed properly, collect the data */
                *buf = reg_val & 0xff;
                return (PASSED);
            } else {
                printf("%s: Command is not executed properly (%d)\n", __FUNCTION__,
                       twsi_stats);
                return (FAILED);
            }
        }
        msleep(10);
    } while (timeout--);

    if (timeout <= 0) {
        printf("%s: TWSI Status is not ready after the command (%d)\n",
                __FUNCTION__, twsi_stats);
    }

    return (FAILED);
}

/******************************************************************************
 *
 * Function: mrvl_88X2222M_is_sfp_plus_present
 *
 * Description: This API function detects the presence of SFP plus
 *
 * Inputs      : None
 * Outputs     : TRUE/FALSE
 *
 *****************************************************************************/
int mrvl_88X2222M_is_sfp_plus_present (void)
{
    uint reg_val;
    ten_g_phy_t phy_88X2222M = {MRV88X2222M_REG_DEVICE_31,
                                MRVL_88X2222M_SMI2_PORT0_ADDR};

    /* Is SFP+ module present - Device 31, Register F012*/
    if ((read_ten_g_phy_reg(GPIO_DATA, MRVL_88X2222M_PHY_REG_LEN,
                            &reg_val, &phy_88X2222M)) == FAILED) {
        return (FAILED);
    }

     /* If SFP+ module present - Bit 0 = 0 */
    if ((reg_val &= ~DETECT_SFP_PLUS_MODULE) == 1) {
        return (FALSE);
    } else {
        return (TRUE);
    }
}

/******************************************************************************
 *
 * Function: turn_off_mrvl_88X2222M_i2c
 *
 * Description: This API function turn off 88X2222M I2C bus when read sfp+ eeprom
 *
 * Inputs      : None
 * Outputs     : TRUE/FALSE
 *
 *****************************************************************************/
int turn_off_mrvl_88X2222M_i2c (void)
{
    int port_num, reg_addr, dev_addr, bus_addr, phy_addr;
    uint reg_val;

    port_num = MRVL_88X2222M_PORTS;
    bus_addr = (MRVL_88X2222M_SMI2_ADDR << 4);
    dev_addr = MRV88X2222M_REG_DEVICE_31;
    reg_addr = GPIO_TRISTATE_CTRL;
    phy_addr = MRVL_88X2222M_PORT_0_ADDR;
    
    ten_g_phy_t alter_phy_88X2222M = {dev_addr, (bus_addr | (phy_addr))};

    /* Get the current register vlaue */
    if ((read_ten_g_phy_reg(reg_addr, MRVL_88X2222M_PHY_REG_LEN,
                            &reg_val, &alter_phy_88X2222M)) == FAILED) {
        cterr('f', 0, "Can't read register value at phy_addr=%d, "
              "dev_addr=%04x, reg_addr=%04x", phy_addr, dev_addr, reg_addr);
        return (FAILED);
    }

    reg_val &= ~(MRVL2222M_SCL_OUTPUT_MASK | MRVL2222M_SDA_OUTPUT_MASK);

    if ((write_ten_g_phy_reg(reg_addr, MRVL_88X2222M_PHY_REG_LEN,
                             reg_val, &alter_phy_88X2222M))== FAILED) {
        cterr('f', 0, "Can't alter register v alue at phy_addr=%d, "
                "dev_addr=%04x, reg_addr=%04x", phy_addr, dev_addr, reg_addr);
        return (FAILED);
    }

    reg_addr = GPIO_INTERRUPT_TYPE;

    /* Get the current register vlaue */
    if ((read_ten_g_phy_reg(reg_addr, MRVL_88X2222M_PHY_REG_LEN,
                            &reg_val, &alter_phy_88X2222M)) == FAILED) {
        cterr('f', 0, "Can't read register value at phy_addr=%d, "
              "dev_addr=%04x, reg_addr=%04x", phy_addr, dev_addr, reg_addr);
        return (FAILED);
    }

    reg_val |= (MRVL2222M_SDA_FUNC_MASK | MRVL2222M_SCL_FUNC_MASK);

    if ((write_ten_g_phy_reg(reg_addr, MRVL_88X2222M_PHY_REG_LEN,
                             reg_val, &alter_phy_88X2222M))== FAILED) {
        cterr('f', 0, "Can't alter register v alue at phy_addr=%d, "
                "dev_addr=%04x, reg_addr=%04x", phy_addr, dev_addr, reg_addr);
        return (FAILED);
    }
    return (PASSED);
}

/******************************************************************************
 *
 * Function: mrvl_88X2222M_en_ext_lpbk
 *
 * Description: This API function performs the register setting of Phy 88X2222M
 *              which pass the traffic in order to do the external loopback.
 *
 *              Select 88X2222M port 0 (SFI)
 *              Write 31.f400.11:8=0001 SFI port 2 transmit idles
 *              Write 31.f400.3:0=1000 SFI port 0 transmit from XFI port 0
 *              Write 31.f401.3:0=1000 XFI port 0 transmit from SFI port 0
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int mrvl_88X2222M_en_ext_lpbk (void)
{
    uint reg_val;

    ten_g_phy_t phy_88X2222M = {MRV88X2222M_REG_DEVICE_31,
                                MRVL_88X2222M_SMI2_PORT0_ADDR};
    
    /* Is SFP+ module present - Device 31, Register F012*/
    if ((read_ten_g_phy_reg(GPIO_DATA, MRVL_88X2222M_PHY_REG_LEN,
                            &reg_val, &phy_88X2222M)) == FAILED) {
        return (FAILED);
    } 

     /* If SFP+ module present - Bit 0 = 0 */
    if ((reg_val &= ~DETECT_SFP_PLUS_MODULE) == 1) {
        cterr('f', 0, "88X2222M SFP+ module is not present");
        return (FAILED);
    } else {
        prpass(testpass, "Has detect SFP+ module");
    }

    /*
     * Set up the external loopback mode in 88X2222M chip:
     * Cavium CPU -> Phy 88X2222M -> SFP+
     */

    /* Get the current device address 31 register 0xF400 value */
    if ((read_ten_g_phy_reg(TRANS_SOUR_N_REG, MRVL_88X2222M_PHY_REG_LEN,
                            &reg_val, &phy_88X2222M)) == FAILED) {
        return (FAILED);
    }
    /* Clear the Bits 11:8 and 3:0 */
    reg_val &= 0xF0F0;
    /* Bits 11:8 SFI line port 2 are in output idles status */
    reg_val |= (LINE_PORT_IDLE << 8);
    /* Bits 3:0 Select XFI port 0 to attach to SFI line port 0 */
    reg_val |= (SELECT_XFI_PORT_0);
    /* Write register value */
    if ((write_ten_g_phy_reg(TRANS_SOUR_N_REG, MRVL_88X2222M_PHY_REG_LEN,
                             reg_val, &phy_88X2222M)) == FAILED) {
        return (FAILED);
    }

    /* Get the current device address 31 register 0xF401 value */
    if ((read_ten_g_phy_reg(TRANS_SOUR_M_REG, MRVL_88X2222M_PHY_REG_LEN,
                            &reg_val, &phy_88X2222M)) == FAILED) {
        return (FAILED);
    }
    /* Clear the Bits 3:0 */
    reg_val &= 0xFFF0;
    /* Bits 3:0 Select SFI port 0 to attach to XFI line port 0 */
    reg_val |= (SELECT_SFI_PORT_0);
    /* Write register value */
    if ((write_ten_g_phy_reg(TRANS_SOUR_M_REG, MRVL_88X2222M_PHY_REG_LEN,
                             reg_val, &phy_88X2222M))== FAILED) {
        return (FAILED);
    }

    return (PASSED);
}

/******************************************************************************
 *
 * Function: mrvl_88X2222M_sel_port2
 *
 * Description: This API function performs the register setting of Phy 88X2222M
 *              which selects port 2 (10GBase - T)
 *
 *              Select 88X2222M port 2 (10GBase-T)
 *              Write 31.f400.11:8=1000 SFI port 2 transmit from XFI port 0
 *              Write 31.f400.3:0=0001 SFI port 0 transmit idles
 *              Write 31.f401.3:0=1010 XFI port 0 transmit from SFI port 2
 *
 *              Increase SFI transmit amplitude
 *              Write 1e.0xb116 = 0x8015
 *              Write 1e.0xb117 = 0x0014
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int mrvl_88X2222M_sel_port2 (void)
{
    uint reg_val;

    ten_g_phy_t phy_88X2222M = {MRV88X2222M_REG_DEVICE_31,
                                MRVL_88X2222M_SMI2_PORT0_ADDR};

    /* Get the current device address 31 register 0xF400 value */
    if ((read_ten_g_phy_reg(TRANS_SOUR_N_REG, MRVL_88X2222M_PHY_REG_LEN,
                            &reg_val, &phy_88X2222M)) == FAILED) {
        return (FAILED);
    }
    /* Clear the Bits 11:8 and 3:0 */
    reg_val &= 0xF0F0;
    /* Bits 11:8 SFI port 2 transmit from XFI port 0 */
    reg_val |= (SELECT_XFI_PORT_0 << 8);
    /* Bits 3:0 SFI port 0 transmit idles */
    reg_val |= (LINE_PORT_IDLE);
    /* Write register value */
    if ((write_ten_g_phy_reg(TRANS_SOUR_N_REG, MRVL_88X2222M_PHY_REG_LEN,
                             reg_val, &phy_88X2222M)) == FAILED) {
        return (FAILED);
    }

    /* Get the current device address 31 register 0xF401 value */
    if ((read_ten_g_phy_reg(TRANS_SOUR_M_REG, MRVL_88X2222M_PHY_REG_LEN,
                            &reg_val, &phy_88X2222M)) == FAILED) {
        return (FAILED);
    }
    /* Clear the Bits 11:8 and 3:0 */
    reg_val &= 0xF0F0;
    /* Bits 11:8 XFI port 2 transmit idles */
    reg_val |= (LINE_PORT_IDLE << 8);
    /* Bits 3:0 XFI port 0 transmit from SFI port 2 */
    reg_val |= (SELECT_SFI_PORT_2);
    /* Write register value */
    if ((write_ten_g_phy_reg(TRANS_SOUR_M_REG, MRVL_88X2222M_PHY_REG_LEN,
                             reg_val, &phy_88X2222M))== FAILED) {
        return (FAILED);
    }

    msleep(1000);

    phy_88X2222M.device_id = MRV88X2222M_REG_DEVICE_30;

    /* Write SMI device id 30 port 2 register offset 0xb116 value 0x8015 */
    if ((write_ten_g_phy_reg(0xb116, MRVL_88X2222M_PHY_REG_LEN,
                             0x8015, &phy_88X2222M)) == FAILED) {
        return (FAILED);
    }

    /* Write SMI device id 30 port 2 register offset 0xb117 value 0x0014 */
    if ((write_ten_g_phy_reg(0xb117, MRVL_88X2222M_PHY_REG_LEN,
                             0x0014, &phy_88X2222M)) == FAILED) {
        return (FAILED);
    }

    return (PASSED);
}

void marvell_2222p_a0_init (void)
{
    unsigned int mii_value;
    unsigned int smi2 = 0x2;
    unsigned int port0 = 0x4;
    unsigned int port1 = 0x5;
    unsigned int port2 = 0x6;
    unsigned int port3 = 0x7;

    /* Chip HW Reset */
    /* xrw smi_u1 d31 p0 rF404 h4000 */
    cvmx_mdio_45_write(smi2, port0, 0x1F, 0xf404, 0x4000);
    msleep(100); 

    /*10GR-10Gx4*/
    /* xrw smi_u1 d31 p0 rf002 h7173 */
    cvmx_mdio_45_write(smi2, port0, 0x1F, 0xf002, 0x7173);

    /* xrw smi_u1 d31 p2 rf002 h7173 */
    cvmx_mdio_45_write(smi2, port2, 0x1F, 0xf002, 0x7173);

    /*couple write to all lanes*/
    /* xrw smi_u1 d30 p0 rb841 he000 */
    cvmx_mdio_45_write(smi2, port0, 0x1e, 0xb841, 0xe0000);

    /* SFI workarounds */
    /* xrw smi_u1 d30 p0 rb1e6 bxx10000000010100 */
    mii_value = cvmx_mdio_45_read(smi2, port0, 0x1e, 0xb1e6);
    mii_value |= 0x2014;
    mii_value &= 0xe014;
    cvmx_mdio_45_write(smi2, port0, 0x1e, 0xb1e6, mii_value);

    /* xrw smi_u1 d30 p0 rb1e6 bxx10000100010100 */
    mii_value = cvmx_mdio_45_read(smi2, port0, 0x1e, 0xb1e6);
    mii_value |= 0x2114;
    mii_value &= 0xe114;
    cvmx_mdio_45_write(smi2, port0, 0x1e, 0xb1e6, mii_value);

    /* xrw smi_u1 d30 p0 rb1e6 bxx10001000010100 */
    mii_value = cvmx_mdio_45_read(smi2, port0, 0x1e, 0xb1e6);
    mii_value |= 0x2214;
    mii_value &= 0xe214;
    cvmx_mdio_45_write(smi2, port0, 0x1e, 0xb1e6, mii_value);

    /* xrw smi_u1 d30 p0 rb1e6 bxx10001100010100 */
    mii_value = cvmx_mdio_45_read(smi2, port0, 0x1e, 0xb1e6);
    mii_value |= 0x2314;
    mii_value &= 0xe314;
    cvmx_mdio_45_write(smi2, port0, 0x1e, 0xb1e6, mii_value);

    /* xrw smi_u1 d30 p0 rb1e6 bxx10010000010100 */
    mii_value = cvmx_mdio_45_read(smi2, port0, 0x1e, 0xb1e6);
    mii_value |= 0x2414;
    mii_value &= 0xe414;
    cvmx_mdio_45_write(smi2, port0, 0x1e, 0xb1e6, mii_value);

    /* xrw smi_u1 d30 p0 rb1e6 bxx10010100010100 */
    mii_value = cvmx_mdio_45_read(smi2, port0, 0x1e, 0xb1e6);
    mii_value |= 0x2514;
    mii_value &= 0xe514;
    cvmx_mdio_45_write(smi2, port0, 0x1e, 0xb1e6, mii_value);

    /* xrw smi_u1 d30 p0 rb1e6 bxx10011000010100 */
    mii_value = cvmx_mdio_45_read(smi2, port0, 0x1e, 0xb1e6);
    mii_value |= 0x2614;
    mii_value &= 0xe614;
    cvmx_mdio_45_write(smi2, port0, 0x1e, 0xb1e6, mii_value);

    /* xrw smi_u1 d30 p0 rb1e6 bxx10011100010100 */
    mii_value = cvmx_mdio_45_read(smi2, port0, 0x1e, 0xb1e6);
    mii_value |= 0x2714;
    mii_value &= 0xe714;
    cvmx_mdio_45_write(smi2, port0, 0x1e, 0xb1e6, mii_value);

    /* xrw smi_u1 d30 p0 rb1e6 bxx10100000100110 */
    mii_value = cvmx_mdio_45_read(smi2, port0, 0x1e, 0xb1e6);
    mii_value |= 0x2826;
    mii_value &= 0xe826;
    cvmx_mdio_45_write(smi2, port0, 0x1e, 0xb1e6, mii_value);

    /* xrw smi_u1 d30 p0 rb1e6 bxx10100100111000 */
    mii_value = cvmx_mdio_45_read(smi2, port0, 0x1e, 0xb1e6);
    mii_value |= 0x2938;
    mii_value &= 0xe938;
    cvmx_mdio_45_write(smi2, port0, 0x1e, 0xb1e6, mii_value);

    /* xrw smi_u1 d30 p0 rb1e6 bxx10101001001010 */
    mii_value = cvmx_mdio_45_read(smi2, port0, 0x1e, 0xb1e6);
    mii_value |= 0x2a4a;
    mii_value &= 0xea4a;
    cvmx_mdio_45_write(smi2, port0, 0x1e, 0xb1e6, mii_value);

    /* xrw smi_u1 d30 p0 rb1e6 bxx10101101011100 */
    mii_value = cvmx_mdio_45_read(smi2, port0, 0x1e, 0xb1e6);
    mii_value |= 0x2b5c;
    mii_value &= 0xeb5c;
    cvmx_mdio_45_write(smi2, port0, 0x1e, 0xb1e6, mii_value);

    /* xrw smi_u1 d30 p0 rb1e6 bxx10110001101101 */
    mii_value = cvmx_mdio_45_read(smi2, port0, 0x1e, 0xb1e6);
    mii_value |= 0x2c6d;
    mii_value &= 0xec6d;
    cvmx_mdio_45_write(smi2, port0, 0x1e, 0xb1e6, mii_value);

    /* xrw smi_u1 d30 p0 rb1e6 bxx10110101111110 */
    mii_value = cvmx_mdio_45_read(smi2, port0, 0x1e, 0xb1e6);
    mii_value |= 0x2d7e;
    mii_value &= 0xed7e;
    cvmx_mdio_45_write(smi2, port0, 0x1e, 0xb1e6, mii_value);

    /* xrw smi_u1 d30 p0 rb1e6 bxx10111010001111 */
    mii_value = cvmx_mdio_45_read(smi2, port0, 0x1e, 0xb1e6);
    mii_value |= 0x2e8f;
    mii_value &= 0xee8f;
    cvmx_mdio_45_write(smi2, port0, 0x1e, 0xb1e6, mii_value);

    /* xrw smi_u1 d30 p0 rb1e6 bxx10111110011111 */
    mii_value = cvmx_mdio_45_read(smi2, port0, 0x1e, 0xb1e6);
    mii_value |= 0x2f9f;
    mii_value &= 0xef9f;
    cvmx_mdio_45_write(smi2, port0, 0x1e, 0xb1e6, mii_value);

    /* xrw smi_u1 d30 p0 rb1e6 bxx11000000010100 */
    mii_value = cvmx_mdio_45_read(smi2, port0, 0x1e, 0xb1e6);
    mii_value |= 0x3014;
    mii_value &= 0xf014;
    cvmx_mdio_45_write(smi2, port0, 0x1e, 0xb1e6, mii_value);

    /* xrw smi_u1 d30 p0 rb1e6 bxx11000100100110 */
    mii_value = cvmx_mdio_45_read(smi2, port0, 0x1e, 0xb1e6);
    mii_value |= 0x3126;
    mii_value &= 0xf126;
    cvmx_mdio_45_write(smi2, port0, 0x1e, 0xb1e6, mii_value);

    /* xrw smi_u1 d30 p0 rb1e6 bxx11001000111000 */
    mii_value = cvmx_mdio_45_read(smi2, port0, 0x1e, 0xb1e6);
    mii_value |= 0x3238;
    mii_value &= 0xf238;
    cvmx_mdio_45_write(smi2, port0, 0x1e, 0xb1e6, mii_value);

    /* xrw smi_u1 d30 p0 rb1e6 bxx11001101001010 */
    mii_value = cvmx_mdio_45_read(smi2, port0, 0x1e, 0xb1e6);
    mii_value |= 0x334a;
    mii_value &= 0xf34a;
    cvmx_mdio_45_write(smi2, port0, 0x1e, 0xb1e6, mii_value);

    /* xrw smi_u1 d30 p0 rb1e6 bxx11010001011100 */
    mii_value = cvmx_mdio_45_read(smi2, port0, 0x1e, 0xb1e6);
    mii_value |= 0x345b;
    mii_value &= 0xf45b;
    cvmx_mdio_45_write(smi2, port0, 0x1e, 0xb1e6, mii_value);

    /* xrw smi_u1 d30 p0 rb1e6 bxx11010101101101 */
    mii_value = cvmx_mdio_45_read(smi2, port0, 0x1e, 0xb1e6);
    mii_value |= 0x356d;
    mii_value &= 0xf56d;
    cvmx_mdio_45_write(smi2, port0, 0x1e, 0xb1e6, mii_value);

    /* xrw smi_u1 d30 p0 rb1e6 bxx11011001111110 */
    mii_value = cvmx_mdio_45_read(smi2, port0, 0x1e, 0xb1e6);
    mii_value |= 0x367e;
    mii_value &= 0xf67e;
    cvmx_mdio_45_write(smi2, port0, 0x1e, 0xb1e6, mii_value);

    /* xrw smi_u1 d30 p0 rb1e6 bxx11011110001111 */
    mii_value = cvmx_mdio_45_read(smi2, port0, 0x1e, 0xb1e6);
    mii_value |= 0x378f;
    mii_value &= 0xf78f;
    cvmx_mdio_45_write(smi2, port0, 0x1e, 0xb1e6, mii_value);

    /* xrw smi_u1 d30 p0 rb1e6 bxx11100010011111 */
    mii_value = cvmx_mdio_45_read(smi2, port0, 0x1e, 0xb1e6);
    mii_value |= 0x389f;
    mii_value &= 0xf89f;
    cvmx_mdio_45_write(smi2, port0, 0x1e, 0xb1e6, mii_value);

    /* xrw smi_u1 d30 p0 rb1e6 bxx11100110011111 */
    mii_value = cvmx_mdio_45_read(smi2, port0, 0x1e, 0xb1e6);
    mii_value |= 0x399f;
    mii_value &= 0xf99f;
    cvmx_mdio_45_write(smi2, port0, 0x1e, 0xb1e6, mii_value);

    /* xrw smi_u1 d30 p0 rb1e6 bxx11101010011111 */
    mii_value = cvmx_mdio_45_read(smi2, port0, 0x1e, 0xb1e6);
    mii_value |= 0x3a9f;
    mii_value &= 0xfa9f;
    cvmx_mdio_45_write(smi2, port0, 0x1e, 0xb1e6, mii_value);

    /* xrw smi_u1 d30 p0 rb1e6 bxx11101110011111 */
    mii_value = cvmx_mdio_45_read(smi2, port0, 0x1e, 0xb1e6);
    mii_value |= 0x3b9f;
    mii_value &= 0xfb9f;
    cvmx_mdio_45_write(smi2, port0, 0x1e, 0xb1e6, mii_value);

    /* xrw smi_u1 d30 p0 rb1e6 bxx11110010011111 */
    mii_value = cvmx_mdio_45_read(smi2, port0, 0x1e, 0xb1e6);
    mii_value |= 0x3c9f;
    mii_value &= 0xfc9f;
    cvmx_mdio_45_write(smi2, port0, 0x1e, 0xb1e6, mii_value);

    /* xrw smi_u1 d30 p0 rb1e6 bxx11110110011111 */
    mii_value = cvmx_mdio_45_read(smi2, port0, 0x1e, 0xb1e6);
    mii_value |= 0x3d9f;
    mii_value &= 0xfd9f;
    cvmx_mdio_45_write(smi2, port0, 0x1e, 0xb1e6, mii_value);

    /* xrw smi_u1 d30 p0 rb1e6 bxx11111010011111 */
    mii_value = cvmx_mdio_45_read(smi2, port0, 0x1e, 0xb1e6);
    mii_value |= 0x3e9f;
    mii_value &= 0xfe9f;
    cvmx_mdio_45_write(smi2, port0, 0x1e, 0xb1e6, mii_value);

    /* xrw smi_u1 d30 p0 rb1e6 bxx11111110011111 */
    mii_value = cvmx_mdio_45_read(smi2, port0, 0x1e, 0xb1e6);
    mii_value |= 0x3f9f;
    mii_value &= 0xff9f;
    cvmx_mdio_45_write(smi2, port0, 0x1e, 0xb1e6, mii_value);

    /* xrw smi_u1 d30 p0 rb1e7 b11xxxxxxxxxxxxxx */
    mii_value = cvmx_mdio_45_read(smi2, port0, 0x1e, 0xb1e7);
    mii_value |= 0xc000;
    cvmx_mdio_45_write(smi2, port0, 0x1e, 0xb1e7, mii_value);

    /* sleep 200 */
    msleep(200);

    /*PCS Reset*/
    /* xrw smi_u1 d31 p0-3 rf003 h8080 */
    cvmx_mdio_45_write(smi2, port0, 0x1F, 0xf003, 0x8080);
    cvmx_mdio_45_write(smi2, port1, 0x1F, 0xf003, 0x8080);
    cvmx_mdio_45_write(smi2, port2, 0x1F, 0xf003, 0x8080);
    cvmx_mdio_45_write(smi2, port3, 0x1F, 0xf003, 0x8080);

    mii_value = cvmx_mdio_45_read(smi2, port0, 0x1f, 0xf404);
    mii_value |= 0x100;
    cvmx_mdio_45_write(smi2, port0, 0x1f, 0xf404, mii_value);
}

void marvell_2222p_init (void)
{
    unsigned int mii_value;
    unsigned int smi2 = 0x2;
    unsigned int port0 = 0x4;
    unsigned int port1 = 0x5;
    unsigned int port2 = 0x6;
    unsigned int port3 = 0x7;

    /* Chip HW Reset */
    /* xrw smi_u1 d31 p0 rF404 h4000 */
    cvmx_mdio_45_write(smi2, port0, 0x1F, 0xf404, 0x4000);
    msleep(100); 

    /*10GR-10Gx4*/
    /* xrw smi_u1 d31 p0 rf002 h7173 */
    cvmx_mdio_45_write(smi2, port0, 0x1F, 0xf002, 0x7173);

    /* xrw smi_u1 d31 p2 rf002 h7173 */
    cvmx_mdio_45_write(smi2, port2, 0x1F, 0xf002, 0x7173);

    /*Disable KR training for LRM side*/
    /* xrw smi_u1 d3 p0 rf07c h8143 */
    cvmx_mdio_45_write(smi2, port0, 0x3, 0xf07c, 0x8143);
    cvmx_mdio_45_write(smi2, port1, 0x3, 0xf07c, 0x8143);
    cvmx_mdio_45_write(smi2, port2, 0x3, 0xf07c, 0x8143);
    cvmx_mdio_45_write(smi2, port3, 0x3, 0xf07c, 0x8143);

    /*PCS Reset*/
    /* xrw smi_u1 d31 p0 rf003 h8080 */
    cvmx_mdio_45_write(smi2, port0, 0x1F, 0xf003, 0x8080);
    /* power down unused ports */
    cvmx_mdio_45_write(smi2, port1, 0x1F, 0xf003, 0x4040);
    cvmx_mdio_45_write(smi2, port2, 0x1F, 0xf003, 0x4040);
    cvmx_mdio_45_write(smi2, port3, 0x1F, 0xf003, 0x4040);
    msleep(10);

    /*KR: couple DSP lane writing */
    /* xrw smi_u1 d30 p0 r9041 h0001 */
    cvmx_mdio_45_write(smi2, port0, 0x1e, 0x9041, 0x0001);

    /* KR: PLL setting */
    /* xrw smi_u1 d30 p0 r8108 b1111100100001xxx */
    mii_value = cvmx_mdio_45_read(smi2, port0, 0x1e, 0x8108);
    mii_value |= 0xf908;
    mii_value &= 0xf90f;
    cvmx_mdio_45_write(smi2, port0, 0x1e, 0x8108, mii_value);

    /* xrw smi_u1 d30 p0 r8102 b1xxxx000101xxxxx */
    mii_value = cvmx_mdio_45_read(smi2, port0, 0x1e, 0x8102);
    mii_value |= 0x80a0;
    mii_value &= 0xf8bf;
    cvmx_mdio_45_write(smi2, port0, 0x1e, 0x8102, mii_value);

    /*LRM: couple DSP lane writing */
    /* xrw smi_u1 d30 p0 rb841 he000 */
    cvmx_mdio_45_write(smi2, port0, 0x1e, 0xb841, 0xe000); 

    /* PLL setting */
    /* xrw smi_u1 d30 p0 rb108 b1xxxxxx011010xxx */
    mii_value = cvmx_mdio_45_read(smi2, port0, 0x1e, 0xb108);
    mii_value |= 0x80d0;
    mii_value &= 0xfed7;
    cvmx_mdio_45_write(smi2, port0, 0x1e, 0xb108, mii_value);

    /* xrw smi_u1 d30 p0 rb121 b1101xxxxxxxxxxxx */
    mii_value = cvmx_mdio_45_read(smi2, port0, 0x1e, 0xb121);
    mii_value |= 0xd000;
    mii_value &= 0xdfff;
    cvmx_mdio_45_write(smi2, port0, 0x1e, 0xb121, mii_value);

    /*software reset KR8*/
    /* xrw smi_u1 d30 p0 r8000 b1xxxxxxxxxxxxxxx */
    mii_value = cvmx_mdio_45_read(smi2, port0, 0x1e, 0x8000);
    mii_value |= 0x8000;
    cvmx_mdio_45_write(smi2, port0, 0x1e, 0x8000, mii_value);

    /*software reset LRM*/
    /* xrw smi_u1 d30 p0 rb000 b1xxxxxxxxxxxxxxx */
    mii_value = cvmx_mdio_45_read(smi2, port0, 0x1e, 0xb000);
    mii_value |= 0x8000;
    cvmx_mdio_45_write(smi2, port0, 0x1e, 0xb000, mii_value); 
}

/* Init the 88X2222M revision A0 PHY according to Marvell's device release note
 */
void marvell_2222m_init_a0(void)
{
    unsigned int mii_value;
    unsigned int smi2 = 0x2;
    unsigned int port0 = 0x4;
    unsigned int port1 = 0x5;
    unsigned int port2 = 0x6;
    unsigned int port3 = 0x7;

    /* xrw smi_u1 d31 p0 rF404 h4000
     * Write SMI device id 31 port 0 register offset 0xf404 value 0x4000
     * chip hw reset
     */
    cvmx_mdio_45_write(smi2, port0, 0x1F, 0xf404, 0x4000);
    msleep(100);

    /* xrw smi_u1 d31 p0 rf002 h7173 */
    cvmx_mdio_45_write(smi2, port0, 0x1F, 0xf002, 0x7173);

    /* xrw smi_u1 d31 p2 rf002 h7173 */
    cvmx_mdio_45_write(smi2, port2, 0x1F, 0xf002, 0x7173);

    /* Extends p0~p4 PCS waiting time */
    /* xrw smi_u1 d4 p0 rf074 bx1xxxxxxxxxxxxxx */
    mii_value = cvmx_mdio_45_read(smi2, port0, 0x4, 0xf074);
    mii_value |= 0x4000;
    cvmx_mdio_45_write(smi2, port0, 0x4, 0xf074, mii_value);

    /* xrw smi_u1 d4 p1 rf074 bx1xxxxxxxxxxxxxx */
    mii_value = cvmx_mdio_45_read(smi2, port1, 0x4, 0xf074);
    mii_value |= 0x4000;
    cvmx_mdio_45_write(smi2, port1, 0x4, 0xf074, mii_value);

    /* xrw smi_u1 d4 p2 rf074 bx1xxxxxxxxxxxxxx */
    mii_value = cvmx_mdio_45_read(smi2, port2, 0x4, 0xf074);
    mii_value |= 0x4000;
    cvmx_mdio_45_write(smi2, port2, 0x4, 0xf074, mii_value);

    /* xrw smi_u1 d4 p3 rf074 bx1xxxxxxxxxxxxxx */
    mii_value = cvmx_mdio_45_read(smi2, port3, 0x4, 0xf074);
    mii_value |= 0x4000;
    cvmx_mdio_45_write(smi2, port3, 0x4, 0xf074, mii_value);

    /* pcs reset - xrw smi_u1 d31 p0 rf003 h8080 */
    cvmx_mdio_45_write(smi2, port0, 0x1F, 0xf003, 0x8080);

    /* unused ports can be powered down by changing 0x8080 to 0x4040 */
    cvmx_mdio_45_write(smi2, port1, 0x1F, 0xf003, 0x4040);
    cvmx_mdio_45_write(smi2, port2, 0x1F, 0xf003, 0x4040);
    cvmx_mdio_45_write(smi2, port3, 0x1F, 0xf003, 0x4040);
    msleep(10);

    /* XFI couple write to all  */
    /* xrw smi_u1 d30 p0 r9041 h03fe */
    cvmx_mdio_45_write(smi2, port0, 0x1e, 0x9041, 0x03fe);

    /* xrw smi_u1 d30 p0 r8042 h0800 */
    cvmx_mdio_45_write(smi2, port0, 0x1e, 0x8042, 0x0800);
    msleep(400);

    /* TxPLL setting */ 
    /* xrw smi_u1 d30 p0 r8143 h9082 */
    cvmx_mdio_45_write(smi2, port0, 0x1e, 0x8143, 0x9082);

    /* RxPLL setting */
    /* xrw smi_u1 d30 p0 r8134 h9082 */
    cvmx_mdio_45_write(smi2, port0, 0x1e, 0x8134, 0x9082);

    /* dsp work around finish */
    /* xrw smi_u1 d30 p0 r8000 b1xxxxxxxxxxxxxxx */
    mii_value = cvmx_mdio_45_read(smi2, port0, 0x1e, 0x8000);
    mii_value |= 0x8000;
    cvmx_mdio_45_write(smi2, port0, 0x1e, 0x8000, mii_value);
    msleep(100);

    /* xrw smi_u1 d30 p0 rb800 b1xxxxxxxxxxxxxxx */
    mii_value = cvmx_mdio_45_read(smi2, port0, 0x1e, 0xb800);
    mii_value |= 0x8000;
    cvmx_mdio_45_write(smi2, port0, 0x1e, 0xb800, mii_value);
}

/* Init the 88X2222M PHY according to Marvell's device release note
 */
void marvell_2222m_init_z1(void)
{
    unsigned int phy_identifier_1, mii_value;
    unsigned int smi2 = 0x2;
    unsigned int port0 = 0x4;
    unsigned int port2 = 0x6;

    /* Get the PMA/PMD Device Identifier 1
     * Device 1, Register 0x0002
     */
    phy_identifier_1 = cvmx_mdio_45_read(0x2, 0x4, 0x1, 0x2);

    if (phy_identifier_1 == 0x0141) {   /* Is it a Marvell PHY? */
        /* xrw smi_u1 d31 p0 rF404 h4000
         * Write SMI device id 31 port 0 register offset 0xf404 value 0x4000
         * chip hw reset
         */
        cvmx_mdio_45_write(smi2, port0, 0x1F, 0xf404, 0x4000);

        msleep(100);

        /* xrw smi_u1 d31 p0 rf002 h7173
         * Write SMI device id 31 port 0 register offset 0xf002 value 0x7173
         * 10GR-10Gx4
         */
        cvmx_mdio_45_write(smi2, port0, 0x1F, 0xf002, 0x7173);

        /* xrw smi_u1 d31 p2 rf002 h7173
         * Write SMI device id 31 port 2 register offset 0xf002 value 0x7173
         * 10GR-10Gx4
         */
        cvmx_mdio_45_write(smi2, port2, 0x1F, 0xf002, 0x7173);

        /* xrw smi_u1 d4 p0 rf074 h4250
         * Write SMI device id 4 port 0 register offset 0xf074 value 0x4250
         */
        cvmx_mdio_45_write(smi2, port0, 0x4, 0xf074, 0x4250);

        /* xrw smi_u1 d4 p2 rf074 h4250
         * Write SMI device id 4 port 2 register offset 0xf074 value 0x4250
         */
        cvmx_mdio_45_write(smi2, port2, 0x4, 0xf074, 0x4250);

        /* xrw smi_u1 d3 p0 rf074 h0250
         * Write SMI device id 3 port 0 register offset 0xf074 value 0x0250
         */
        cvmx_mdio_45_write(smi2, port0, 0x3, 0xf074, 0x0250);

        /* xrw smi_u1 d3 p2 rf074 h0250
         * Write SMI device id 3 port 2 register offset 0xf074 value 0x0250
         */
        cvmx_mdio_45_write(smi2, port2, 0x3, 0xf074, 0x0250);

        /* xrw smi_u1 d31 p0 rf003 h8080
         * Write SMI device id 31 port 0 register offset 0xf003 value 0x8080
         */
        cvmx_mdio_45_write(smi2, port0, 0x1f, 0xf003, 0x8080);

        /* xrw smi_u1 d31 p2 rf003 h8080
         * Write SMI device id 31 port 2 register offset 0xf003 value 0x8080
         */
        cvmx_mdio_45_write(smi2, port2, 0x1f, 0xf003, 0x8080);

        msleep(10);

        /* xrw smi_u1 d30 p0 rb841 he000
         * Write SMI device id 30 port 0 register offset 0xb841 value 0xe000
         */
        cvmx_mdio_45_write(smi2, port0, 0x1e, 0xb841, 0xe000);

        /* xrw smi_u1 d30 p0 r9041 h03fe
         * Write SMI device id 30 port 0 register offset 0x9041 value 0x03fe
         */
        cvmx_mdio_45_write(smi2, port0, 0x1e, 0x9041, 0x03fe);

        /*********************  xfi work around  **************************/

        /* xrw smi_u1 d30 p0 r8090 hfff9
         * Write SMI device id 30 port 0 register offset 0x8090 value 0xfff9
         */
        cvmx_mdio_45_write(smi2, port0, 0x1e, 0x8090, 0xfff9);

        /* xrw smi_u1 d30 p0 r80b9 bxxxxxxxx00xxxxxx
         * Write SMI device id 30 port 0 register offset 0x80b9 value bxxxxxxxx00xxxxxx
         */
        mii_value = cvmx_mdio_45_read(smi2, port0, 0x1e, 0x80b9);
        mii_value &= 0xFF3F;
        cvmx_mdio_45_write(smi2, port0, 0x1e, 0x80b9, mii_value);

        /* xrw smi_u1 d30 p0 r809e h0000
         * Write SMI device id 30 port 0 register offset 0x809e value 0x0000
         */
        cvmx_mdio_45_write(smi2, port0, 0x1e, 0x809e, 0x0000);

        /* xrw smi_u1 d30 p0 r80ea h7200
         * Write SMI device id 30 port 0 register offset 0x80ea value 0x7200
         */
        cvmx_mdio_45_write(smi2, port0, 0x1e, 0x80ea, 0x7200);

        /* xrw smi_u1 d30 p0 r80a1 h4342
         * Write SMI device id 30 port 0 register offset 0x80a1 value 0x4342
         */
        cvmx_mdio_45_write(smi2, port0, 0x1e, 0x80a1, 0x4342);

        /* xrw smi_u1 d30 p0 r80b0 h0200
         * Write SMI device id 30 port 0 register offset 0x80b0 value 0x0200
         */
        cvmx_mdio_45_write(smi2, port0, 0x1e, 0x80b0, 0x0200);

        /* xrw smi_u1 d30 p0 r80b1 h4242
         * Write SMI device id 30 port 0 register offset 0x80b1 value 0x4242
         */
        cvmx_mdio_45_write(smi2, port0, 0x1e, 0x80b1, 0x4242);

        /* xrw smi_u1 d30 p0 r8074 h9ddd
         * Write SMI device id 30 port 0 register offset 0x8074 value 0x9ddd
         */
        cvmx_mdio_45_write(smi2, port0, 0x1e, 0x8074, 0x9ddd);

        /* xrw smi_u1 d30 p0 r8075 hbddd
         * Write SMI device id 30 port 0 register offset 0x8075 value 0xbddd
         */
        cvmx_mdio_45_write(smi2, port0, 0x1e, 0x8075, 0xbddd);

        /* xrw smi_u1 d30 p0 r8076 heddd
         * Write SMI device id 30 port 0 register offset 0x8076 value 0xeddd
         */
        cvmx_mdio_45_write(smi2, port0, 0x1e, 0x8076, 0xeddd);

        /* xrw smi_u1 d30 p0 r80b4 bx100xxxxx011x100
         * Write SMI device id 30 port 0 register offset 0x80b4 value bx100xxxxx011x100
         */
        mii_value = cvmx_mdio_45_read(smi2, port0, 0x1e, 0x80b4);
        mii_value |= 0x4034;
        mii_value &= 0xcfbc;
        cvmx_mdio_45_write(smi2, port0, 0x1e, 0x80b4, mii_value);

        /* xrw smi_u1 d30 p0 r80b5 bx011x011x010x011
         * Write SMI device id 30 port 0 register offset 0x80b5 value bx011x011x010x011
         */
        mii_value = cvmx_mdio_45_read(smi2, port0, 0x1e, 0x80b5);
        mii_value |= 0x3323;
        mii_value &= 0xbbab;
        cvmx_mdio_45_write(smi2, port0, 0x1e, 0x80b5, mii_value);

        /* xrw smi_u1 d30 p0 r80ba bxx0xxxxxxxxxxxxx
         * Write SMI device id 30 port 0 register offset 0x80ba value bxx0xxxxxxxxxxxxx
         */
        mii_value = cvmx_mdio_45_read(smi2, port0, 0x1e, 0x80ba);
        mii_value &= 0xdfff;
        cvmx_mdio_45_write(smi2, port0, 0x1e, 0x80ba, mii_value);

        /*********************  lrm work around  **************************/
        /* xrw smi_u1 d30 p0 rB116 h800b
         * Write SMI device id 30 port 0 register offset 0xB116 value 0x800b
         */
        cvmx_mdio_45_write(smi2, port0, 0x1e, 0xB116, 0x800b);

        /* xrw smi_u1 d30 p0 rB117 h021e
         * Write SMI device id 30 port 0 register offset 0xB117 value 0x021e
         */
        cvmx_mdio_45_write(smi2, port0, 0x1e, 0xB117, 0x021e);

        /* xrw smi_u1 d30 p0 rB060 h4a37
         * Write SMI device id 30 port 0 register offset 0xB060 value 0x4a37
         */
        cvmx_mdio_45_write(smi2, port0, 0x1e, 0xB060, 0x4a37);

        /* xrw smi_u1 d30 p0 rB064 h0003
         * Write SMI device id 30 port 0 register offset 0xB064 value 0x0003
         */
        cvmx_mdio_45_write(smi2, port0, 0x1e, 0xB064, 0x0003);

        /* xrw smi_u1 d30 p0 rB181 h2220
         * Write SMI device id 30 port 0 register offset 0xB181 value 0x2220
         */
        cvmx_mdio_45_write(smi2, port0, 0x1e, 0xB181, 0x2220);

        /* xrw smi_u1 d30 p0 rB182 h2220
         * Write SMI device id 30 port 0 register offset 0xB182 value 0x2220
         */
        cvmx_mdio_45_write(smi2, port0, 0x1e, 0xB182, 0x2220);

        /* xrw smi_u1 d30 p0 rB19c h0050
         * Write SMI device id 30 port 0 register offset 0xB19c value 0x0050
         */
        cvmx_mdio_45_write(smi2, port0, 0x1e, 0xB19c, 0x0050);

        /* xrw smi_u1 d30 p0 rB1b6 h8000
         * Write SMI device id 30 port 0 register offset 0xB1b6 value 0x8000
         */
        cvmx_mdio_45_write(smi2, port0, 0x1e, 0xB1b6, 0x8000);

        /* xrw smi_u1 d30 p0 rB1b7 h7750
         * Write SMI device id 30 port 0 register offset 0xB1b7 value 0x7750
         */
        cvmx_mdio_45_write(smi2, port0, 0x1e, 0xB1b7, 0x7750);

        /* xrw smi_u1 d30 p0 rB1ba h0000
         * Write SMI device id 30 port 0 register offset 0xB1ba value 0x0000
         */
        cvmx_mdio_45_write(smi2, port0, 0x1e, 0xB1ba, 0x0000);

        /* xrw smi_u1 d30 p0 rB1c0 h4020
         * Write SMI device id 30 port 0 register offset 0xB1c0 value 0x4020
         */
        cvmx_mdio_45_write(smi2, port0, 0x1e, 0xB1c0, 0x4020);

        /* xrw smi_u1 d30 p0 rB1e7 h8003
         * Write SMI device id 30 port 0 register offset 0xB1e7 value 0x8003
         */
        cvmx_mdio_45_write(smi2, port0, 0x1e, 0xB1e7, 0x8003);

        /* xrw smi_u1 d30 p0 rB1ec h1d1a
         * Write SMI device id 30 port 0 register offset 0xB1ec value 0x1d1a
         */
        cvmx_mdio_45_write(smi2, port0, 0x1e, 0xB1ec, 0x1d1a);

        /* xrw smi_u1 d30 p0 rB1c1 h8000
         * Write SMI device id 30 port 0 register offset 0xB1c1 value 0x8000
         */
        cvmx_mdio_45_write(smi2, port0, 0x1e, 0xB1c1, 0x8000);

        /* xrw smi_u1 d30 p0 rB1c3 h0048
         * Write SMI device id 30 port 0 register offset 0xB1c3 value 0x0048
         */
        cvmx_mdio_45_write(smi2, port0, 0x1e, 0xB1c3, 0x0048);

        /* xrw smi_u1 d30 p0 rB1bd h0007
         * Write SMI device id 30 port 0 register offset 0xB1bd value 0x0007
         */
        cvmx_mdio_45_write(smi2, port0, 0x1e, 0xB1bd, 0x0007);

        /* xrw smi_u1 d30 p0 rB1DD h6660
         * Write SMI device id 30 port 0 register offset 0xB1DD value 0x6660
         */
        cvmx_mdio_45_write(smi2, port0, 0x1e, 0xB1DD, 0x6660);

        /* xrw smi_u1 d30 p0 rB170 h6000
         * Write SMI device id 30 port 0 register offset 0xB170 value 0x6000
         */
        cvmx_mdio_45_write(smi2, port0, 0x1e, 0xB170, 0x6000);

        /* xrw smi_u1 d30 p0 rb001 h0001
         * Write SMI device id 30 port 0 register offset 0xb001 value 0x0001
         */
        cvmx_mdio_45_write(smi2, port0, 0x1e, 0xb001, 0x0001);

        /* xrw smi_u1 d30 p0 rB1b1 h6968
         * Write SMI device id 30 port 0 register offset 0xb1b1 value 0x6968
         */
        cvmx_mdio_45_write(smi2, port0, 0x1e, 0xb1b1, 0x6968);

        /* xrw smi_u1 d30 p0 rB1b2 h0
         * Write SMI device id 30 port 0 register offset 0xb1b2 value 0x0000
         */
        cvmx_mdio_45_write(smi2, port0, 0x1e, 0xb1b2, 0x0000);

        /* xrw smi_u1 d30 p0 r8000 b1xxxxxxxxxxxxxxx
         * Write SMI device id 30 port 0 register offset 0x8000 value b1xxxxxxxxxxxxxxx
         */
        mii_value = cvmx_mdio_45_read(smi2, port0, 0x1e, 0x8000);
        mii_value |= 0x8000;
        cvmx_mdio_45_write(smi2, port0, 0x1e, 0x8000, mii_value);

        /* xrw smi_u1 d30 p0 rb800 b1xxxxxxxxxxxxxxx
         * Write SMI device id 30 port 0 register offset 0xb800 value b1xxxxxxxxxxxxxxx
         */
        mii_value = cvmx_mdio_45_read(smi2, port0, 0x1e, 0xb800);
        mii_value |= 0x8000;
        cvmx_mdio_45_write(smi2, port0, 0x1e, 0xb800, mii_value);

        /*  Write SMI device id 31 port 0 register offset 0xf012 value bxxxxxxx0x1xxxxxx
         */
        mii_value = cvmx_mdio_45_read(smi2, port0, 0x1f, 0xf012);
        mii_value |= 0x0040;
        mii_value &= 0xfeff;
        cvmx_mdio_45_write(smi2, port0, 0x1f, 0xf012, mii_value);
    }
}

void marvell_2222m_init (void) 
{
    unsigned int mii_value;
    unsigned int smi2 = 0x2;
    unsigned int port0 = 0x4;

    /* identify the 88x2222 silicon revision */
    mii_value = cvmx_mdio_45_read(smi2, port0, 0x1, 0x0003);
    if ((mii_value ==  MRVL2222M_A0_REV1) || (mii_value ==  MRVL2222M_A0_REV2)) {
        /* revA0 */
        printf("*****silicon revision A0*****\n");
        marvell_2222m_init_a0();
    } else if (mii_value ==  MRVL2222M_Z1) {
        /* rev Z1 */
        printf("*****silicon revision Z1*****\n");
        marvell_2222m_init_z1();
    } else if (mii_value ==  MRVL2222P) {
        /* rev P */
        printf("*****silicon revision P*****\n");
        marvell_2222p_init();
    } else if (mii_value == MRVL2222P_A0) {
        /* rev 2222P-A0 */
        printf("*****silicon revision 2222P-A0*****\n");
        marvell_2222p_a0_init();
    } else {
        printf("*****Doesn't support this chip id\n");
    } 
}

void marvell_2222m_sfi_compliance_testing (void)
{
    printf("SFI Compliance Testing\n");
    unsigned int phy_identifier_1, mii_value;
    unsigned int smi2 = MRVL_88X2222M_SMI2_ADDR;
    unsigned int port0 = MRVL_88X2222M_PORT_0_ADDR;
    unsigned int port2 = MRVL_88X2222M_PORT_2_ADDR;

    /* Get the PMA/PMD Device Identifier 1
     * Device 1, Register 0x0002
     */
    phy_identifier_1 = cvmx_mdio_45_read(smi2, port0, MRV88X2222M_REG_DEVICE_1, 0x2);

    if (phy_identifier_1 == 0x0141) {
        
        /* chip hw reset */
        cvmx_mdio_45_write(smi2, port0, 0x1F, 0xf404, 0x4000);

        /* disable PCS reseting at N side */
        cvmx_mdio_45_write(smi2, port0, 0x3, 0xf074, 0x0250);
        cvmx_mdio_45_write(smi2, port2, 0x3, 0xf074, 0x0250);

        /* disable PCS reseting at M side */
        cvmx_mdio_45_write(smi2, port0, 0x4, 0xf074, 0x0250);
        cvmx_mdio_45_write(smi2, port2, 0x4, 0xf074, 0x0250);

        /* dsp workaroung begin */

        /* couple write to all */
        cvmx_mdio_45_write(smi2, port0, 0x1E, 0xb841, 0xe000);

        /* adjust Tx output amplitude to pass eye template at compliance point B */
        cvmx_mdio_45_write(smi2, port0, 0x1E, 0xb116, 0x8013);
        cvmx_mdio_45_write(smi2, port0, 0x1E, 0xb117, 0x0215);

        cvmx_mdio_45_write(smi2, port0, 0x1E, 0xb060, 0x4937);

        cvmx_mdio_45_write(smi2, port0, 0x1E, 0xb064, 0x0003);

        cvmx_mdio_45_write(smi2, port0, 0x1E, 0xb170, 0x8000);

        cvmx_mdio_45_write(smi2, port0, 0x1E, 0xb001, 0x0001);

        /* software reset */
        mii_value = cvmx_mdio_45_read(smi2, port0, 0x1E, 0xb800);
        mii_value |= 0x8000;
        cvmx_mdio_45_write(smi2, port0, 0x1E, 0xb800, mii_value);

        msleep(2000);

        /* PRBS31 at DSP side */
        cvmx_mdio_45_write(smi2, port0, 0x1E, 0xb003, 0x1800);

        msleep(500);

        cvmx_mdio_45_write(smi2, port0, 0x1E, 0xb0a8, 0x1000);
        cvmx_mdio_45_write(smi2, port0, 0x1E, 0xb0a9, 0x0004);

        cvmx_mdio_45_write(smi2, port0, 0x1E, 0xb0a9, 0x0000);

        cvmx_mdio_45_write(smi2, port0, 0x1E, 0xb0a9, 0x0004);
        
    }
     
}

int switch_sfp_plus_led (int on)
{
    int reg_addr, dev_addr, bus_addr, phy_addr;
    uint reg_val;  

    bus_addr = (MRVL_88X2222M_SMI2_ADDR << 4);
    dev_addr = MRV88X2222M_REG_DEVICE_31;
    reg_addr = LED1_CONTROL_REG;
    phy_addr = MRVL_88X2222M_PORT_0_ADDR;

    ten_g_phy_t alter_phy_88X2222M = {dev_addr, (bus_addr | (phy_addr))};

    if (on == TRUE) {
        reg_val = SFP_PLUS_SPEED_LED_ON;
        if ((write_ten_g_phy_reg(reg_addr, MRVL_88X2222M_PHY_REG_LEN,
                             reg_val, &alter_phy_88X2222M))== FAILED) {
            cterr('f', 0, "Can't alter register value at phy_addr=%d, "
                "dev_addr=%04x, reg_addr=%04x", phy_addr, dev_addr, reg_addr);
            return (FAILED);
        }
    } else {
        reg_val = SFP_PLUS_SPEED_LED_OFF;
        if ((write_ten_g_phy_reg(reg_addr, MRVL_88X2222M_PHY_REG_LEN,
                             reg_val, &alter_phy_88X2222M))== FAILED) {
            cterr('f', 0, "Can't alter register value at phy_addr=%d, "
                "dev_addr=%04x, reg_addr=%04x", phy_addr, dev_addr, reg_addr);
            return (FAILED);
        }
    }

    return (PASSED);
}

/******************************************************************************
 *
 * Function: enable_mrvl2222m_macsec_power
 *
 * Description: This function makes the MACSec power states be active
 *
 * Inputs      : void
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
int enable_mrvl2222m_macsec_power (void)
{
    uint reg_val;
    ulong reg_addr = MRV88X2222M_F2R_CFG_REG;
    ten_g_phy_t write_phy_88X2222M = {MRV88X2222M_REG_DEVICE_31,
                                      MRVL_88X2222M_SMI2_PORT0_ADDR};

    /* Enable MACSec before accessing MACSec/PTP registers */
    if ((read_ten_g_phy_reg(reg_addr, MRVL_88X2222M_PHY_REG_LEN,
                            &reg_val, &write_phy_88X2222M))== FAILED) {
        printf("Can't alter register value at reg_addr=%lx", reg_addr);
        fflush(stdout);
        return (FAILED);
    }

    reg_val |= MRV88X2222M_F2R_MAC_PWR_STS;
    
    if ((write_ten_g_phy_reg(reg_addr, MRVL_88X2222M_PHY_REG_LEN,
                             reg_val, &write_phy_88X2222M))== FAILED) {
        printf("Can't alter register value at reg_addr=%lx", reg_addr);
        fflush(stdout);
        return (FAILED);
    }

    return (PASSED);
}

/*-------------------------------------------------
 * $Log: diag_xaui_88X2222M_lib.c,v $
 * Revision 1.7  2015/02/14 12:48:41  kodko
 * Collapse timing card branch code into main trunk.
 *
 * Revision 1.6  2015/02/04 07:23:12  leschen
 * Fix for sfp+ speed led control.
 *
 * Revision 1.5  2014/03/18 02:43:42  leschen
 * Update 2222p A0 init script.
 *
 * Revision 1.4  2014/03/17 07:02:20  leschen
 * Add 2222P A0 init script.
 *
 * Revision 1.3.2.1  2014/04/30 13:47:22  kodko
 * Support 1548P/2222P clock/trigger in/out verification.
 *
 * Revision 1.3  2013/12/12 09:18:18  leschen
 * Add Marvell 2222p init script
 *
 * Revision 1.2  2013/10/08 08:48:29  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.4.2  2013/08/20 10:58:54  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.4  2013/07/02 07:11:25  leschen
 * Check in A0 final script utility
 *
 * Revision 1.1.2.3  2013/06/17 11:13:50  leschen
 * Modify 88X2222 A0 init script utility
 *
 * Revision 1.1.2.2  2013/06/10 13:27:52  leschen
 * Add A0 init script to init utility
 *
 * Revision 1.1.2.1  2013/04/24 10:37:19  tirawan
 * Initial check-in for woodlawn linux code
 *
 * Revision 1.4  2013/04/10 11:17:25  leslie
 * Add lib to turn off 88X2222M I2C bus
 *
 * Revision 1.3  2013/03/27 04:49:36  kuangik
 * Code cleanup after adding -Wall
 *
 * Revision 1.2  2013/03/20 10:34:55  kuangik
 * Implement I2C Read Utility and SFP+ eeprom display utility
 *
 * Revision 1.9  2012/12/11 00:57:42  leslie
 * Add detect SFP+ module mechanism.
 *
 * Revision 1.8  2012/11/19 02:36:03  leslie
 * Add 2222M sfi compliance testing utility.
 *
 * Revision 1.7  2012/10/24 10:39:51  leslie
 * Fix and clean up code.
 *
 * Revision 1.6  2012/10/03 06:03:14  kody
 * Adjust the X2222M amplitude to link up with X3120
 *
 * Revision 1.5  2012/09/21 11:46:15  kody
 * Add 88X2222m initialization script and port mapping API.
 *
 * Revision 1.4  2012/08/03 10:16:55  evanli
 * Mapping to latest O2 source code on 20120726
 *
 * Revision 1.2  2012/05/18 10:20:47  kody
 * Add 88X2222M external loopback api.
 *
 * Revision 1.1  2012/04/16 02:39:42  kody
 * Add Marvell XAUI 88X2222M test.
 *
 * $Endlog$
 *-------------------------------------------------
 */
