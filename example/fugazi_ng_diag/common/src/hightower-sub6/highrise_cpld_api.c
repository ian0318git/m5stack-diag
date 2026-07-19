/* $Id: highrise_cpld_api.c,v 1.2 2021/06/02 02:56:22 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/hightower-sub6/highrise_cpld_api.c,v $
 *******************************************************************************
 * File Name: highrise_cpld_api.c
 *
 * Description: Highrise CPLD api source file
 *
 * Author: Mingchun Ding
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *******************************************************************************
 */
#include <stdio.h>
#include <endian.h>
#include "types.h"
#include "common.h"
#include "highrise_cpld_lib.h"
#include "hr_commn_util.h"

/**********************************************************************
 *
 * Function: hr_cpld_reg_bit_set  
 *
 * This function modify the register with bit operating. 
 *
 * Input : reg - target register
 *         mask - bit mask 
 *         val - new val 
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static long hr_cpld_reg_bit_set(unsigned long reg, unsigned long mask, unsigned long val)
{
    unsigned long data;

    if (PASSED != hr_cpld_reg_read_32(reg, &data)) {
        cterr('f', 0, "CPLD Register read %#06x Failed", reg);
        return FAILED;
    }
    data = be32toh(data);
    data &=~mask;
    data |=(val & mask);
    data = htobe32(data);

    if (PASSED != hr_cpld_reg_write_32(reg, data)) {
        cterr('f', 0, "CPLD Register write [%#06x] = %010x Failed", reg, data);
        return FAILED;
    }

    return PASSED;
}

/**********************************************************************
 *
 * Function: hr_cplda_is_power_int
 *
 * This function check interrupt assert.
 *
 * Input : power rail select
 *               HR_CPLD_PWR_POE_GOOD
 *               HR_CPLD_PWR_POE_DET
 *               HR_CPLD_PWR_INT_3300S
 *               HR_CPLD_PWR_INT_1800S
 *               HR_CPLD_PWR_INT_3300
 *               HR_CPLD_PWR_INT_2500
 *               HR_CPLD_PWR_INT_1800
 *               HR_CPLD_PWR_INT_1500
 *               HR_CPLD_PWR_INT_1200
 *               HR_CPLD_PWR_INT_0900
 *               HR_CPLD_PWR_INT_0800
 *               HR_CPLD_PWR_INT_GASP
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long hr_cpld_is_power_int(unsigned long pwr)
{
    unsigned long data;

    if (PASSED == hr_cpld_reg_read_32(HR_CPLD_PWR_STATUS, &data)) {
        return !(data & pwr);
    }

    return pwr;
}

/**********************************************************************
 *
 * Function: hr_cplda_trigger_power_int
 *
 * This function trigger interrupt for test
 *
 * Input : power rail select
 *               HR_CPLD_PWR_TST_CPU             0x00000400
 *               HR_CPLD_PWR_TST_3300S           0x00000200
 *               HR_CPLD_PWR_TST_1800S           0x00000100
 *               HR_CPLD_PWR_TST_3300            0x00000080
 *               HR_CPLD_PWR_TST_2500            0x00000040
 *               HR_CPLD_PWR_TST_1800            0x00000020
 *               HR_CPLD_PWR_TST_1500            0x00000010
 *               HR_CPLD_PWR_TST_1200            0x00000008
 *               HR_CPLD_PWR_TST_0900            0x00000004
 *               HR_CPLD_PWR_TST_0800            0x00000002
 *               HR_CPLD_PWR_TST_GASP            0x00000001
 *         enable - 1 to assert interrup, 0 to dessert interrupt
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long hr_cpld_trigger_power_int(unsigned long pwr, int enable)
{
    /* logic high to enable test */
    if (enable)
        return hr_cpld_reg_bit_set(HR_CPLD_ERROR_TEST, pwr, pwr);
    else
        return hr_cpld_reg_bit_set(HR_CPLD_ERROR_TEST, pwr, 0);
}


/**********************************************************************
 *
 * Function: hr_cpld_intr_status 
 *
 * This function check interrupt status and return to caller. 
 *
 * Input : status - return status register 
 *         enb_msk - return enable register 
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long hr_cpld_intr_status(unsigned long *status, unsigned long *enb_msk)
{
    if (status)
        ERR_RET_COND(PASSED != hr_cpld_reg_read_32(HR_CPLD_INT_STATUS, status),
            FAILED, "Failed to read HR_CPLD_INT_STATUS\n");
    if (enb_msk)
        ERR_RET_COND(PASSED != hr_cpld_reg_read_32(HR_CPLD_INT_ENABLE, enb_msk),
            FAILED, "Failed to read HR_CPLD_INT_ENABLE\n");

    return PASSED;
}

/**********************************************************************
 *
 * Function: hr_cpld_intr_enable 
 *
 * This function setup interrupt to interrupt enable register. 
 *
 * Input : enb_msk - the value user want to enable intr. 
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long hr_cpld_intr_enable(unsigned long enb_msk)
{
    ERR_RET_COND(PASSED != hr_cpld_reg_write_32(HR_CPLD_INT_ENABLE, enb_msk),
        FAILED, "Failed to read HR_CPLD_INT_ENABLE\n");
    return 0;
}

/**********************************************************************
 *
 * Function: hr_cplda_reset
 *
 * This function reset specified devices
 *
 * Input : dev select devices to reset
 *               HR_CPLD_UNRESET_MODEM_PCIE      0x00000040
 *               HR_CPLD_UNRESET_MODEM           0x00000020
 *               HR_CPLD_UNRESET_ACT2            0x00000010
 *               HR_CPLD_UNRESET_EMMC            0x00000008
 *               HR_CPLD_UNRESET_DDR4            0x00000004
 *         enable - 1 to assert interrup, 0 to dessert interrupt
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long hr_cpld_reset(unsigned long dev, int enable)
{
    /* logic low to reset */
    if (enable)
        return hr_cpld_reg_bit_set(HR_CPLD_RESET_CTRL, dev, 0);
    else
        return hr_cpld_reg_bit_set(HR_CPLD_RESET_CTRL, dev, dev);
}

/**********************************************************************
 *
 * Function: hr_cplda_reset_protect
 *
 * This function protect/unprotect specified devices be reset
 *
 * Input : dev select devices to reset
 *             HR_CPLD_RESET_LOCK_MODEM        0x00000020
 *             HR_CPLD_RESET_LOCK_ACT2         0x00000010
 *             HR_CPLD_RESET_LOCK_EMMC         0x00000008
 *             HR_CPLD_RESET_LOCK_DDR4         0x00000004
 *             HR_CPLD_RESET_LOCK_CPU          0x00000001
 *         enable - 1 to assert interrup, 0 to dessert interrupt
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long hr_cpld_reset_protect(unsigned long dev, int enable)
{
    /* logic high to enable reset protect */
    if (enable)
        return hr_cpld_reg_bit_set(HR_CPLD_RESET_CTRL, dev, dev);
    else
        return hr_cpld_reg_bit_set(HR_CPLD_RESET_CTRL, dev, 0);
}

/**********************************************************************
 *
 * Function: hr_cplda_set_wol_mac_addr
 *
 * This function to set mac address for WOL
 *
 * Input : mac      - point to mac address data in big endian
 *         reversed - reversed byte order
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long hr_cpld_set_wol_mac_addr(uchar *mac, int reversed)
{
    unsigned long data = 0;
    uchar *p = (uchar*)&data;
    int j;

    if (reversed) {
        for (j = 2; j < 4; j++)
            p[j] = *mac++;
        if (PASSED != hr_cpld_reg_write_32(HR_CPLD_MAC1_ADDR, data)) {
            cterr('f', 0, "CPLD Register write HR_CPLD_MAC1_ADDR Failed");
            return (FAILED);
        }

        for (j = 0; j < 4; j++)
            p[j] = *mac++;
        if (PASSED != hr_cpld_reg_write_32(HR_CPLD_MAC0_ADDR, data)) {
            cterr('f', 0, "CPLD Register write HR_CPLD_MAC0_ADDR Failed");
            return (FAILED);
        }
    } else {
        mac += 5;
        for (j = 2; j < 4; j++)
            p[j] = *mac--;
        if (PASSED != hr_cpld_reg_write_32(HR_CPLD_MAC1_ADDR, data)) {
            cterr('f', 0, "CPLD Register write HR_CPLD_MAC1_ADDR Failed");
            return (FAILED);
        }

        for (j = 0; j < 4; j++)
            p[j] = *mac--;
        if (PASSED != hr_cpld_reg_write_32(HR_CPLD_MAC0_ADDR, data)) {
            cterr('f', 0, "CPLD Register write HR_CPLD_MAC0_ADDR Failed");
            return (FAILED);
        }
    }

    return PASSED;
}

/**********************************************************************
 *
 * Function: hr_cplda_get_wol_mac_addr
 *
 * This function to get mac address for WOL
 *
 * Input : mac      - point to mac address data in big endian
 *         reversed - reversed byte order
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long hr_cpld_get_wol_mac_addr(uchar *mac, int reversed)
{
    unsigned long data;
    uchar *p = (uchar*)&data;
    int j;

    if (reversed) {
        if (PASSED != hr_cpld_reg_read_32(HR_CPLD_MAC1_ADDR, &data)) {
            cterr('f', 0, "CPLD Register read HR_CPLD_MAC1_ADDR Failed");
            return (FAILED);
        }
        for (j = 2; j < 4; j++)
            *mac++ = p[j];

        if (PASSED != hr_cpld_reg_read_32(HR_CPLD_MAC0_ADDR, &data)) {
            cterr('f', 0, "CPLD Register read HR_CPLD_MAC0_ADDR Failed");
            return (FAILED);
        }
        for (j = 0; j < 4; j++)
            *mac++ = p[j];
    } else {
        mac += 5;
        if (PASSED != hr_cpld_reg_read_32(HR_CPLD_MAC1_ADDR, &data)) {
            cterr('f', 0, "CPLD Register read HR_CPLD_MAC1_ADDR Failed");
            return (FAILED);
        }
        for (j = 2; j < 4; j++)
            *mac-- = p[j];

        if (PASSED != hr_cpld_reg_read_32(HR_CPLD_MAC0_ADDR, &data)) {
            cterr('f', 0, "CPLD Register read HR_CPLD_MAC0_ADDR Failed");
            return (FAILED);
        }
        for (j = 0; j < 4; j++)
            *mac-- = p[j];
    }

    return PASSED;
}

/**********************************************************************
 *
 * Function: hr_cplda_set_led
 *
 * This function to set led mode off/blink/color
 *
 * Input : type    - bit map to select led: 4G/5G/WWAN
 *                      HR_CPLD_LED_TYPE_NULL   0x0000   // off S/W control, keeping color settings
 *                      HR_CPLD_LED_TYPE_5G     0x0001
 *                      HR_CPLD_LED_TYPE_4G     0x0002
 *         mode    - led mode: blink/red/green/blue/yellow/ping/cyan/white/off
 *                      HR_CPLD_LED_MODE_OFF
 *                      HR_CPLD_LED_MODE_CLR
 *                      HR_CPLD_LED_MODE_BLINK
 *                      HR_CPLD_LED_MODE_RED
 *                      HR_CPLD_LED_MODE_GREEN
 *                      HR_CPLD_LED_MODE_BLUE
 *                      HR_CPLD_LED_MODE_YELLOW
 *                      HR_CPLD_LED_MODE_PINK
 *                      HR_CPLD_LED_MODE_CYAN
 *                      HR_CPLD_LED_MODE_WHITE
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long hr_cpld_set_led(unsigned long type, unsigned long mode)
{
    unsigned long data = 0;
    unsigned long mask = 0;
    unsigned long color = 0;
#define HR_CPLD_LED_SHIFT 6

    if (PASSED != hr_cpld_reg_read_32(HR_CPLD_LED_CTRL, &data)) {
        cterr('f', 0, "CPLD Register read HR_CPLD_LED_CTRL Failed");
        return (FAILED);
    }

    /* set color */
    color = mode & HR_CPLD_LED_COLOR_MASK;

    if (HR_CPLD_LED_MODE_BLINK & mode) {
        if (color) {
            /* blink specified color */
            color |= (color << 1);
        } else {
            /* blink all color */
            color  = (HR_CPLD_LED_MODE_WHITE<<1);
        }
    } else if (color) {
        mask = (color << 1);
    }

    /* set pure color */
    if (HR_CPLD_LED_MODE_CLR & mode) {
        mask = HR_CPLD_LED_COLOR_MASK|(HR_CPLD_LED_COLOR_MASK<<1);
    }

    if (HR_CPLD_LED_TYPE_NULL == type) {
        /* turn off led S/W controlled */
        data &=~HR_CPLD_LED_SW_EN;
    } else {
        if (HR_CPLD_LED_TYPE_5G & type) {
            data |= HR_CPLD_LED_SW_EN;
            data &=~mask;
            data |= color;
        }
        if (HR_CPLD_LED_TYPE_4G & type) {
            data |= HR_CPLD_LED_SW_EN;
            data &=~(mask << HR_CPLD_LED_SHIFT);
            data |= (color << HR_CPLD_LED_SHIFT);
        }
    }

    if (PASSED != hr_cpld_reg_write_32(HR_CPLD_LED_CTRL, data)) {
        cterr('f', 0, "CPLD Register write HR_CPLD_LED_CTRL Failed");
        return (FAILED);
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: hr_cplda_set_off
 *
 * This function to turn off led
 *
 * Input : type    - bit map to select led: 4G/5G/WWAN
 *                      HR_CPLD_LED_TYPE_5G     0x0001
 *                      HR_CPLD_LED_TYPE_4G     0x0002
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long hr_cpld_led_off(unsigned long type)
{
    return hr_cpld_set_led(type, HR_CPLD_LED_MODE_CLR);
}

/**********************************************************************
 *
 * Function: hr_cplda_disable_led_sw_mode
 *
 * This function to disable led S/W control
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long hr_cpld_disable_led_sw_mode(void)
{
    return hr_cpld_set_led(HR_CPLD_LED_TYPE_NULL, HR_CPLD_LED_MODE_OFF);
}

/**********************************************************************
 *
 * Function: hr_cpld_default_value
 *
 * This function to save/restore cpld configures
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int hr_cpld_init_default(int flag)
{
    hr_cpld_reg_write_32(HR_CPLD_ERROR_TEST, 0);

    hr_cpld_reg_write_32(HR_CPLD_RESET_CTRL,
        HR_CPLD_UNRESET_MODEM_PCIE |
        HR_CPLD_UNRESET_ACT2       |
        HR_CPLD_UNRESET_EMMC       |
        HR_CPLD_UNRESET_DDR4);

    hr_cpld_reg_write_32(HR_CPLD_RESET_PROTECT,
        HR_CPLD_RESET_LOCK_MODEM |
        HR_CPLD_RESET_LOCK_ACT2  |
        HR_CPLD_RESET_LOCK_EMMC  |
        HR_CPLD_RESET_LOCK_DDR4  |
        HR_CPLD_RESET_LOCK_CPU);

    hr_cpld_reg_write_32(HR_CPLD_LED_CTRL, 0);

    hr_cpld_reg_write_32(HR_CPLD_INT_ENABLE,
        HR_CPLD_INT_EN_WOL     |
        HR_CPLD_INT_EN_PWR_ERR |
        HR_CPLD_INT_EN_CPU_ERR);

    return 0;
}

/**********************************************************************
 *
 * Function: hr_cpld_reset_act2
 *
 * This function to reset ACT2 
 *
 * Input : none
 *
 * Output: PASSED
 *
 **********************************************************************
 */
long hr_cpld_reset_act2(void)
{
    hr_cpld_reset(HR_CPLD_UNRESET_ACT2, 1);
    return (PASSED);
}

/**********************************************************************
 *
 * Function: hr_cpld_unreset_act2
 *
 * This function to unreset ACT2
 *
 * Input : none
 *
 * Output: PASSED
 *
 **********************************************************************
 */
long hr_cpld_unreset_act2()
{
    hr_cpld_reset(HR_CPLD_UNRESET_ACT2, 0);
    return (PASSED);
}

/**********************************************************************
 *
 * Function: hr_cpld_get_version
 *
 * This function returns cpld version and date to caller.
 *
 * Input : ver_numb - cpld version
 *         ver_date - cpld date.
 *
 * Output: PASSED
 *
 **********************************************************************
 */
int hr_cpld_get_version(uint16_t *ver_numb, uint16_t *ver_date)
{
    unsigned long val = 0;

    ERR_RET_COND(PASSED != hr_cpld_reg_read_32(HR_CPLD_VERSION, &val), FAILED, "Read cpld failed.\n");
    if (ver_numb)
        *ver_numb = val & 0xffff;
    ERR_RET_COND(PASSED != hr_cpld_reg_read_32(HR_CPLD_VERSION_DATE, &val), FAILED, "Read cpld failed.\n");
    if (ver_date)
        *ver_date = val & 0xffff;
    return 0;
}

/**********************************************************************
 *
 * Function: hr_cpld_get_boardid
 *
 * This function returns board id and board naming to caller. 
 *
 * Input : id - board id from cpld
 *         name - board naming, based on HFS. 
 *
 * Output: PASSED
 *
 **********************************************************************
 */
int hr_cpld_get_boardid(uint8_t *id, char *name)
{
    unsigned long val = 0;
    ERR_RET_COND(PASSED != hr_cpld_reg_read_32(HR_CPLD_BOARD_ID, &val), FAILED, "Read cpld failed.\n");

    if (id)
        *id = val & HR_CPLD_BOARD_ID_MSK;

    if (name)
        strcpy(name,
            (val & HR_CPLD_BOARD_ID_MSK) == HR_CPLD_BOARD_NONE           ? "Unknown"           : (
            (val & HR_CPLD_BOARD_ID_MSK) == HR_CPLD_BOARD_HIGHTOWER_MMWV ? "Hightower MMWV"    : (
            (val & HR_CPLD_BOARD_ID_MSK) == HR_CPLD_BOARD_HIGHTOWER_SUB6 ? "Hightower sub6"    : (
            (val & HR_CPLD_BOARD_ID_MSK) == HR_CPLD_BOARD_HIGHRISE_CAT18 ? "Hightower CAT18"    : (
            "Unknown")))));

    return 0;
}

/**********************************************************************
 *
 * Function: hr_cpld_get_sys_info 
 *
 * This function dumpes various board info to users.  (based on CPLD);
 *
 * Input : buf - info buffer
 *         size - buffer size. 
 *
 * Output: PASSED
 *
 **********************************************************************
 */
int hr_cpld_get_sys_info(char *buf, int size)
{
    unsigned long val = 0;
    unsigned long vam = 0;
    int len = 0;
    int ret = 0;

    ERR_RET_COND(!buf || size <= 0, FAILED, "Invalid argument\n");

    memset(buf, 0, size);

    #define _PRINT_INFO(fmt, ...) do { \
        ret = snprintf(buf + len, size - len - 1, fmt, ##__VA_ARGS__); \
        ERR_RET_COND(ret < 0, FAILED, "%d:snprintf() failed, ret:%d.\n", __LINE__, ret); \
        ERR_RET_COND(ret >= size - len - 1, FAILED, "%d:Not enough buffer.\n", __LINE__); \
        len += ret; \
    }while(0)

    ERR_RET_COND(PASSED != hr_cpld_reg_read_32(HR_CPLD_BOARD_ID, &val), FAILED, "Read cpld failed.\n");
    _PRINT_INFO("%-16s: 0x%02lx(%s)\n\n",
        "Board ID",
        val & HR_CPLD_BOARD_ID_MSK,
        (val & HR_CPLD_BOARD_ID_MSK) == HR_CPLD_BOARD_NONE           ? "Unknown"           : (
        (val & HR_CPLD_BOARD_ID_MSK) == HR_CPLD_BOARD_HIGHTOWER_MMWV ? "Hightower MMWV"    : (
        (val & HR_CPLD_BOARD_ID_MSK) == HR_CPLD_BOARD_HIGHTOWER_SUB6 ? "Hightower sub6"    : (
        (val & HR_CPLD_BOARD_ID_MSK) == HR_CPLD_BOARD_HIGHRISE_CAT18 ? "Hightower CAT18"    : (
        "Unknown")))));

    ERR_RET_COND(PASSED != hr_cpld_reg_read_32(HR_CPLD_VERSION, &val), FAILED, "Read cpld failed.\n");
    ERR_RET_COND(PASSED != hr_cpld_reg_read_32(HR_CPLD_VERSION_DATE, &vam), FAILED, "Read cpld failed.\n");
    _PRINT_INFO("%-16s: 0x%04lx 0x%04lx\n\n", "CPLD Version", val & 0xffff, vam & 0xffff);

    ERR_RET_COND(PASSED != hr_cpld_reg_read_32(HR_CPLD_PWR_STATUS, &val), FAILED, "Read cpld failed.\n");
    _PRINT_INFO("%-16s: 0x%08lx\n", "Power Status", val);
    if (val & HR_CPLD_PWR_POE_DET)
    _PRINT_INFO("%-18sPoE_PD_CLS   - %lx\n"
                "%-18sPoE_Pwr      - %s\n",
                " ", (val & HR_CPLD_PWR_PD_CLASS ) >> BIT_START(HR_CPLD_PWR_PD_CLASS, 0),
                " ", (val & HR_CPLD_PWR_POE_GOOD ) ? "Good"  : "Fault");
    _PRINT_INFO("%-18sPoE_Detected - %s\n"
                "%-18s3.3V_Standby - %s\n"
                "%-18s1.8V_Standby - %s\n"
                "%-18s3.3V         - %s\n"
                "%-18s2.5V         - %s\n"
                "%-18s1.8V         - %s\n"
                "%-18s1.5V         - %s\n"
                "%-18s1.2V         - %s\n"
                "%-18s0.9V         - %s\n"
                "%-18s0.8V         - %s\n"
                "%-18sDyingGasp    - %s\n\n",
                " ", (val & HR_CPLD_PWR_POE_DET  ) ? "Yes"   : "No",
                " ", (val & HR_CPLD_PWR_INT_3300S) ? "Fault" : "Good",
                " ", (val & HR_CPLD_PWR_INT_1800S) ? "Fault" : "Good",
                " ", (val & HR_CPLD_PWR_INT_3300 ) ? "Fault" : "Good",
                " ", (val & HR_CPLD_PWR_INT_2500 ) ? "Fault" : "Good",
                " ", (val & HR_CPLD_PWR_INT_1800 ) ? "Fault" : "Good",
                " ", (val & HR_CPLD_PWR_INT_1500 ) ? "Fault" : "Good",
                " ", (val & HR_CPLD_PWR_INT_1200 ) ? "Fault" : "Good",
                " ", (val & HR_CPLD_PWR_INT_0900 ) ? "Fault" : "Good",
                " ", (val & HR_CPLD_PWR_INT_0800 ) ? "Fault" : "Good",
                " ", (val & HR_CPLD_PWR_INT_GASP ) ? "Yes"   : "No");


    ERR_RET_COND(PASSED != hr_cpld_reg_read_32(HR_CPLD_MODEM_STATUS, &val), FAILED, "Read cpld failed.\n");
    _PRINT_INFO("%-16s: 0x%08lx\n"
                "%-18s%s %s %s\n\n",
                "Modem Info", val,
                " ", val & HR_CPLD_MODEM_STA_VREG_PWR_ON ? "1.8V_VREG, On" : "",
                     val & HR_CPLD_MODEM_STA_PRESET      ? "CAT18 Modem"   : "",
                     val & HR_CPLD_MODEM_STA_ACTIVE      ? "Active & On"   : "");

    ERR_RET_COND(PASSED != hr_cpld_reg_read_32(HR_CPLD_RESET_PROTECT, &val), FAILED, "Read cpld failed.\n");
    _PRINT_INFO("%-16s: 0x%08lx\n"
                "%-18s%s%s\n"
                "%-18s%s%s\n"
                "%-18s%s%s\n"
                "%-18s%s%s\n"
                "%-18s%s%s\n\n",
                "Reset Protect", val,
                " ", "Modem Reset Protection ", val & HR_CPLD_RESET_LOCK_MODEM ? "enabled" : "disabled",
                " ", "Act2  Reset Protection ", val & HR_CPLD_RESET_LOCK_ACT2  ? "enabled" : "disabled",
                " ", "Emmc  Reset Protection ", val & HR_CPLD_RESET_LOCK_EMMC  ? "enabled" : "disabled",
                " ", "DDR   Reset Protection ", val & HR_CPLD_RESET_LOCK_DDR4  ? "enabled" : "disabled",
                " ", "CPU   Reset Protection ", val & HR_CPLD_RESET_LOCK_CPU   ? "enabled" : "disabled");

    return PASSED;
}

/**********************************************************************
 *
 * Function: hr_cpld_show_sys_info
 *
 * This function display various board info to users.  (based on CPLD);
 *
 * Input : none
 * Output: PASSED
 *
 **********************************************************************
 */
int hr_cpld_show_sys_info(void)
{
    char buf[4096] = {[0 ... sizeof(buf)-1] = 0};
    char *L = NULL;
    char *t = NULL;

    ERR_RET_COND(PASSED != hr_cpld_get_sys_info(buf, sizeof(buf)), FAILED, "Failed to get sys info from cpld.\n");

    printf("\n");
    forline(L, &buf[0], t) {
        printf("%s\n", L);
    }

    return PASSED;
}

/**********************************************************************
 *
 * Function: hr_cpld_show_poe_info
 *
 * This function dumpe poe info, based on cpld. 
 *
 * Input : none
 * Output: PASSED
 *
 **********************************************************************
 */
int ht_cpld_show_poe_info (void) 
{
    unsigned long val = 0;
    ERR_RET_COND(PASSED != hr_cpld_reg_read_32(HR_CPLD_PWR_STATUS, &val), FAILED, "Read cpld failed.\n");
    printf("\n%-16s: 0x%08lx\n"
           "%-18sPower is-%s Power from PoE-%s PoE_PD_CLS-%lx\n",
           "Power Status", val,
                " ", val & HR_CPLD_PWR_POE_GOOD  ? "Good" : "Bad", 
                     val & HR_CPLD_PWR_POE_DET   ? "Yes" : "No",
                    (val & HR_CPLD_PWR_PD_CLASS) >> BIT_START(HR_CPLD_PWR_PD_CLASS, 0));

    /* based on HW description, HR_CPLD_PWR_POE_GOOD is reflect to power good. 
     * it will be 0 ~120ms during power up machine. 
     */
    return PASSED; 
}

/******** History ********/
/*------------------------------------------------------------------------------
 * $Log: highrise_cpld_api.c,v $
 * Revision 1.2  2021/06/02 02:56:22  alpeng
 * merge sears into trunk
 *
 * Revision 1.1.4.3  2020/12/09 07:29:50  alpeng
 * add function prologue; remove redundant header; adding ifdef for header files;
 *
 * Revision 1.1.4.2  2020/12/09 01:52:02  alpeng
 * use C comment
 *
 * Revision 1.1.4.1  2020/09/10 06:03:16  alpeng
 * add board id check before launch diag
 *
 * Revision 1.1  2020/08/19 09:50:52  markzha
 * *** empty log message ***
 *
 *
 *------------------------------------------------------------------------------
 * $Endlog $
 *------------------------------------------------------------------------------
 */

