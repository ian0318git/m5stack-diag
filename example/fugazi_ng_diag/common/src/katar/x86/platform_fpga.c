/* $Id: platform_fpga.c,v 1.2 2019/06/14 05:24:50 mikech2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/katar/x86/platform_fpga.c,v $
 *------------------------------------------------------------------
 *
 * Copyright (c) 2014-2019 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <endian.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "common.h"
#include "defs.h"
#include "proto.h"
#include "queryflags.h"
#include "goofy_i2c.h"
#include "platform_fpga.h"
#include "i2c_address.h"
#include "linux_api.h" /* print_offset_val */
#include "uio_utils.h"
#include "ethernet.h" /* for SFP definition */

extern unsigned long dash_aikido;
extern unsigned long dash_fpgai2c;
extern unsigned long dash_io_reg;

/*-------------------------------------------------------------------
 *
 * Function: get_platform_reg_base
 * Description: get Base Control Register
 * 
 * Input: NONE
 *
 * Output: address
 *
 *-------------------------------------------------------------------
 */
unsigned long
get_platform_reg_base (void)
{
    unsigned long addr = 0;

    assert(dash_fpga);

	addr = ((unsigned long)dash_fpga) + FPGA_BASE_REG_OFFSET;

    return addr;
}

/*-------------------------------------------------------------------
 *
 * Function: get_aikido_reg_base
 * Description: get Aikido LPC Base Register Address
 * 
 * Input: NONE
 *
 * Output: address
 *
 *-------------------------------------------------------------------
 */
unsigned long
get_aikido_reg_base (void)
{
    unsigned long addr = 0;

    assert(dash_aikido);

        addr = ((unsigned long)dash_aikido) + FPGA_BASE_REG_OFFSET;

    return addr;
}

/*-------------------------------------------------------------------
 *
 * Function: get_fpgai2c_reg_base
 * Description: get FPGA I2C Base Register Address
 * 
 * Input: NONE
 *
 * Output: address
 *
 *-------------------------------------------------------------------
 */
unsigned long
get_fpgai2c_reg_base (void)
{
    unsigned long addr = 0;

    assert(dash_fpgai2c);

        addr = ((unsigned long)dash_fpgai2c) + FPGA_BASE_REG_OFFSET;

    return addr;
}

/*-------------------------------------------------------------------
 *
 * Function: get_platform_nio_reg_base
 * Description: get Non-IO Base Control Register
 *
 * Input: NONE
 *
 * Output: address
 *
 *-------------------------------------------------------------------
 */
unsigned long
get_platform_nio_reg_base (void)
{
    unsigned long addr = 0;

    assert(dash_cpld);

    addr = ((unsigned long)dash_cpld);

    return addr;
}

/*-------------------------------------------------------------------
 *
 * Function: get_platform_io_reg_base
 * Description: get IO Base Control Register
 *
 * Input: NONE
 *
 * Output: address
 *
 *-------------------------------------------------------------------
 */
unsigned long
get_platform_io_reg_base (void)
{
    unsigned long addr = 0;

    assert(dash_io_reg);

    addr = ((unsigned long)dash_io_reg);

    return addr;
}

/*-------------------------------------------------------------------
 *
 * Function: get_platform_env_fan_base
 * Description: get Base Control Register
 * 
 * Input: NONE
 *
 * Output: address
 *
 *-------------------------------------------------------------------
 */
unsigned long
get_platform_prom_reg_base (void)
{
    unsigned long addr = 0;

    assert(dash_fpga);

	addr = ((unsigned long)dash_fpga) + FPGA_PROM_REG_OFFSET;

    return addr;
}

/*-------------------------------------------------------------------
 *
 * Function: get_scratchpad_reg_addr
 * Description: get scratchpad Register
 *
 * Input: NONE
 *
 * Output: address
 *
 *-------------------------------------------------------------------
 */
unsigned long
get_scratchpad_reg_addr (void)
{
    unsigned long addr = get_platform_reg_base();

	addr += FPGA_LPC_SCRATCHPAD_REG;

    return addr;
}

int clear_fpga_status(void)
{
	unsigned long offset=get_scratchpad_reg_addr();
	uint32_t value = 0;

	register_write(offset, 0, BW_32BITS);
	value = *((unsigned int *)(offset));
	if(value!=0)
		return FAILED;
	else
		return PASSED;
}

int read_fpga_reg (int verbose)
{
    unsigned long addr = get_platform_reg_base();
    uint32_t value = 0, choice = 0;
    unsigned long offset=0;
	char name[20];

	printf("Select register base:\n");
	printf("  0. Usrlogic registers\n");
	printf("  1. Non I/O registers\n");
        printf("  2. I/O registers\n");
	printf("  3. Aikido registers\n");
	printf("  4. Usrlogic I2C registers\n");
	printf("  5. Quit\n");	
	choice = gethex_answer("Enter selection:", 5, 0, 0xF);

	switch(choice)
	{
		case 0:
			addr = get_platform_reg_base();
                        sprintf(name, "Usrlogic");
		break;
		case 1:
			addr = get_platform_nio_reg_base();
			sprintf(name, "Non I/O");
		break;
                case 2:
                        addr = get_platform_io_reg_base();
                        sprintf(name, "I/O");
                break;
		case 3:
			addr = get_aikido_reg_base();
			sprintf(name, "Aikido");
		break;
		case 4:
			addr = get_fpgai2c_reg_base();
			sprintf(name, "Usrlogic I2C");
		break;
		default:
			return TRUE;
		break;
	}	
    offset = gethex_answer("Enter reg offset", 0, 0, 0xFFFF);

    value = *((unsigned int *)(addr +  offset));
    printf("%s register: 0x%04lx=%#x\n",name, offset, value);
	
	return TRUE;
}

int write_fpga_reg (int verbose)
{
    unsigned long addr = get_platform_reg_base();
    uint32_t value = 0, setvalue = 0, choice = 0;
    unsigned long offset=0;
	uint8	bReadback = TRUE;
	char name[20];

	if (getc_answer("\nRead reg value back? (y/n)", "yn", 'n')== 'y') 
		bReadback = TRUE;
	else
		bReadback = FALSE;

    printf("Select register base:\n");
    printf("  0. Usrlogic registers\n");
    printf("  1. Non I/O registers\n");
    printf("  2. I/O registers\n");
    printf("  3. Aikido registers\n");
    printf("  4. Usrlogic I2C registers\n");
    printf("  5. Quit\n");
    choice = gethex_answer("Enter selection:", 5, 0, 0xF);

    switch(choice)
    {
	case 0:
	    addr = get_platform_reg_base();
            sprintf(name, "Usrlogic");
	break;
        case 1:
            addr = get_platform_nio_reg_base();
	    sprintf(name, "Non I/O");
        break;
	case 2:
            addr = get_platform_io_reg_base();
            sprintf(name, "I/O");
        break;
        case 3:
            addr = get_aikido_reg_base();
            sprintf(name, "Aikido");
        break;
        case 4:
            addr = get_fpgai2c_reg_base();
            sprintf(name, "Usrlogic I2C");
        break;
        default:
	    return TRUE;
        break;
    }
    offset = gethex_answer("Enter reg offset", 0, 0, 0xFFFF);
	
	if(bReadback)
		value = *((unsigned int *)(addr +  offset));
	else
		value = 0;
    setvalue = gethex_answer("Enter reg value", value, 0, 0xFFFFFFFF);

	register_write((addr +  offset), setvalue, BW_32BITS);
	if(bReadback)
	{
		value = *((unsigned int *)(addr +  offset));
		printf("write %s register: 0x%04lx=%#x (%#x)\n", name, offset, value ,setvalue);
	}else
		printf("write %s register: 0x%04lx value:%#x\n", name, offset, setvalue);

	return TRUE;
}

void katar_disable_boot_timer(void)
{
	unsigned long addr = get_platform_reg_base();
	katar_sys_lvl_t *sysreg;

	if(katar_get_usrlogic_ver()<=FPGA_SPEC_1_7_VER)
	{
		printf("Usrlogic FPGA ver too old,please update\n");
		return;
	}

	if(addr != 0)
	{
		sysreg = (katar_sys_lvl_t *)addr;
		sysreg->boot_timer = 0xCA000000;
	}
	return;
}

/*********************************************************************
 *
 * Function:    katar_get_fan_speed
 *
 * Description: read fan status register (FPAG_FAN_STAT_REG 0x114).
 *
 * Inputs:       fan number
 *
 * Output:      register calue
 *
 *********************************************************************
 */
int katar_get_fan_speed (int fan_num) {

    unsigned long addr = get_platform_reg_base();
    katar_sys_lvl_t *sysreg;
    uint32_t value = 0;


	if(addr != 0)
	{
		sysreg = (katar_sys_lvl_t *)addr;
		value = sysreg->fan_stat;
	}

	switch(fan_num)
	{
		case 0:
			value = (value>>OFFSET_FAN_SPEED_0) & 0xFFFF;
			break;
		case 1:
			value = (value>>OFFSET_FAN_SPEED_1) & 0xFFFF;
			break;
		default:
			value = 0xFFFF;
			break;
	}

    return (value);
}

/*********************************************************************
 *
 * Function:    katar_get_fan_pwm_setting
 *
 * Description: read fan control register (FPGA_FAN_CTRL_REG	 0x110).
 *
 * Inputs:       none
 *
 * Output:      register calue
 *
 *********************************************************************
 */
int katar_get_fan_pwm_setting (void) {

    unsigned long addr = get_platform_reg_base();
    katar_sys_lvl_t *sysreg;
    uint32_t value = 0;

	if(addr != 0)
	{
		sysreg = (katar_sys_lvl_t *)addr;
		value = sysreg->fan_ctrl;
		value = ((value&MASK_FAN_PWM_SET)>>OFFSET_FAN_PWM_SET);
	}
    return (value);
}

/*********************************************************************
 *
 * Function:    katar_get_fan_pwm_current
 *
 * Description: read fan control register (FPGA_FAN_CTRL_REG	 0x110).
 *
 * Inputs:       none
 *
 * Output:      register calue
 *
 *********************************************************************
 */
int katar_get_fan_pwm_current (void) {

    unsigned long addr = get_platform_reg_base();
    katar_sys_lvl_t *sysreg;
    uint32_t value = 0;

	if(addr != 0)
	{
		sysreg = (katar_sys_lvl_t *)addr;
        value = sysreg->fan_ctrl;
		value = ((value&MASK_FAN_PWM_CUR)>>OFFSET_FAN_PWM_CUR);
	}
    return (value);
}

/*********************************************************************
 *
 * Function:    katar_get_fan_rpm_threshold
 *
 * Description: read fan control register (FPGA_FAN_CTRL_REG	 0x110).
 *
 * Inputs:       none
 *
 * Output:      register calue
 *
 *********************************************************************
 */
int katar_get_fan_rpm_threshold (void) {

    unsigned long addr = get_platform_reg_base();
    katar_sys_lvl_t *sysreg;
    uint32_t value = 0;

	if(addr != 0)
	{
		sysreg = (katar_sys_lvl_t *)addr;
        value = sysreg->fan_ctrl;
		value = ((value&MASK_FAN_RPM_TH)>>OFFSET_FAN_RPM_TH);
	}
    return (value);
}

/*********************************************************************
 *
 * Function:    katar_set_fan_pwm
 *
 * Description: set fan control register (FPGA_FAN_CTRL_REG 0x110).
 *
 * Inputs:       pwm value
 *
 * Output:      none
 *
 *********************************************************************
 */
void katar_set_fan_pwm (int pwm_setting) {

    unsigned long addr = get_platform_reg_base();
    katar_sys_lvl_t *sysreg;
    uint32_t value = 0;

	if(addr!=0)
	{
		sysreg = (katar_sys_lvl_t *)addr;
		value = sysreg->fan_ctrl;
	}
	value &= ~MASK_FAN_PWM_SET;
	value |= ((pwm_setting<<OFFSET_FAN_PWM_SET)& MASK_FAN_PWM_SET);

	if(addr!=0)
		sysreg->fan_ctrl = value;
	else
		printf("%s set reg value:0x%x\n",__FUNCTION__,value);
	
    return;
}

/*********************************************************************
 *
 * Function:    katar_set_fan_threshold
 *
 * Description: set fan control register (FPGA_FAN_CTRL_REG 0x110).
 *
 * Inputs:       pwm threshold
 *
 * Output:      none
 *
 *********************************************************************
 */
void katar_set_fan_threshold (int pwm_th) {

    unsigned long addr = get_platform_reg_base();
    katar_sys_lvl_t *sysreg;
    uint32_t value = 0;

	if(addr!=0)
	{
		sysreg = (katar_sys_lvl_t *)addr;
		value = sysreg->fan_ctrl;
	}
	value &= ~MASK_FAN_RPM_TH;
	value |= ((pwm_th<<OFFSET_FAN_RPM_TH)& MASK_FAN_RPM_TH);

	if(addr!=0)
		sysreg->fan_ctrl = value;
	else
		printf("%s set reg value:0x%x\n",__FUNCTION__,value);

    return;
}

void katar_get_led_control(void)
{
    unsigned long addr = get_platform_reg_base();
	katar_sys_lvl_t *sysreg;
    uint32_t value = 0;
	
	if(addr!=0)
	{
		sysreg = (katar_sys_lvl_t *)addr;
		value = sysreg->stat_led;
	}
	//setting this field to '11' will tell the FPGA to drive the LEDs according to the LED Management Control Register(0x48)
	value |= 0x3;

	if(addr!=0)
		sysreg->stat_led = value;
	else
		printf("%s set reg value:0x%x\n",__FUNCTION__,value);

    return;
}

/*********************************************************************
 *
 * Function:    katar_set_led_ctrl
 *
 * Description: set led control register (FPGA_LED_CTRL_REG	0x048).
 *
 * Inputs:       led_port    target led port
 *                  blink         blink setting , set to -1 if you don't want to chnage
 *                  color         color setting , set to -1 if you don't want to chnage
 *
 * Output:      none
 *
 *********************************************************************
 */
void katar_set_led_ctrl (uint led_port, int blink, int color) {

    unsigned long addr = get_platform_reg_base();
    katar_sys_lvl_t *sysreg;
	uint32_t offset_blink = 0 ,mask_blink = 0 ,offset_color = 0 ,mask_color = 0;
    uint32_t value = 0;

	switch(led_port)
	{
		case LED_SYS:
			offset_blink = OFFSET_BLINK_SYSTEM;
			mask_blink = MASK_BLINK_SYSTEM;
			offset_color = OFFSET_LED_SYSTEM;
			mask_color = MASK_LED_SYSTEM;
			break;

		case LED_HA:
			offset_blink = OFFSET_BLINK_HA;
			mask_blink = MASK_BLINK_HA;
			offset_color = OFFSET_LED_HA;
			mask_color = MASK_LED_HA;
			break;

		case LED_ALARM:
			offset_blink = OFFSET_BLINK_ALARM;
			mask_blink = MASK_BLINK_ALARM;
			offset_color = OFFSET_LED_ALARM;
			mask_color = MASK_LED_ALARM;
			break;

		default:
			return;
			break;
	}

	if(addr!=0)
	{
		sysreg = (katar_sys_lvl_t *)addr;
		value = sysreg->led_ctrl;
	}
	if(blink >= 0)
	{
		value &= ~mask_blink;
		value |= ((blink<<offset_blink)& mask_blink);
	}
	if(color >= 0)
	{
		value &= ~mask_color;
		value |= ((color<<offset_color)& mask_color);
	}

	if(addr!=0)
		sysreg->led_ctrl = value;
	else
		printf("%s set reg value:0x%x\n",__FUNCTION__,value);

    return;
}


/*********************************************************************
 *
 * Function:    katar_get_led_ctrl_reg
 *
 * Description: get led control register (FPGA_LED_CTRL_REG	0x048).
 *
 * Inputs:      none
 *
 * Output:      reg value
 *
 *********************************************************************
 */
int katar_get_led_ctrl_reg (void) {

    unsigned long addr = get_platform_reg_base();
    katar_sys_lvl_t *sysreg;
    uint32_t value = 0;

	if(addr != 0)
	{
		sysreg = (katar_sys_lvl_t *)addr;
		value = sysreg->led_ctrl;
	}
    return value;
}

/*********************************************************************
 *
 * Function:    katar_get_led_color
 *
 * Description: get led control register (FPGA_LED_CTRL_REG	0x048).
 *
 * Inputs:       led_port    target led port
 *
 * Output:      target led color
 *
 *********************************************************************
 */
int katar_get_led_color (uint led_port) {

    unsigned long addr = get_platform_reg_base();
    katar_sys_lvl_t *sysreg;
	uint32_t offset = 0 ,mask = 0;
    uint32_t value = 0;

	switch(led_port)
	{
		case LED_SYS:
			offset = OFFSET_LED_SYSTEM;
			mask = MASK_LED_SYSTEM;
			break;

		case LED_HA:
			offset = OFFSET_LED_HA;
			mask = MASK_LED_HA;
			break;

		case LED_ALARM:
			offset = OFFSET_LED_ALARM;
			mask = MASK_LED_ALARM;
			break;
		
		default:
			return value;
			break;
	}
	if(addr != 0)
	{
		sysreg = (katar_sys_lvl_t *)addr;
		value = ((sysreg->led_ctrl&mask)>>offset);
	}
    return value;
}

/*********************************************************************
 *
 * Function:    katar_get_led_blink
 *
 * Description: get led control register (FPGA_LED_CTRL_REG	0x048).
 *
 * Inputs:       led_port    target led port
 *
 * Output:      target led blink
 *
 *********************************************************************
 */
int katar_get_led_blink (uint led_port) {

    unsigned long addr = get_platform_reg_base();
    katar_sys_lvl_t *sysreg;
	uint32_t offset = 0 ,mask = 0;
    uint32_t value = 0;

	switch(led_port)
	{
		case LED_SYS:
			offset = OFFSET_BLINK_SYSTEM;
			mask = MASK_BLINK_SYSTEM;
			break;

		case LED_HA:
			offset = OFFSET_BLINK_HA;
			mask = MASK_BLINK_HA;
			break;

		case LED_ALARM:
			offset = OFFSET_BLINK_ALARM;
			mask = MASK_BLINK_ALARM;
			break;
			
		default:
			return 0;
			break;
	}
	if(addr != 0)
	{
		sysreg = (katar_sys_lvl_t *)addr;
		value = ((sysreg->led_ctrl&mask)>>offset);
	}
    return value;
}

/*********************************************************************
 *
 * Function:    katar_set_poe_led_color
 *
 * Description: set led control register (FPGA_NIO_LED_CTRL_REG	0x000).
 *
 * Inputs:       led_port    target led port
 *                  color         color setting
 *
 * Output:      none
 *
 *********************************************************************
 */
void katar_set_poe_led_color (uint led_port, int color) {

    unsigned long addr = get_platform_nio_reg_base();
    katar_nio_lvl_t *nioreg;
	uint32_t offset_color = 0 ,mask_color = 0;
    uint32_t value = 0;
	uint32_t bInvert = FALSE;

	switch(led_port)
	{
		//POE P1 & P1 set 0 to power on LED
		case LED_POE_P0:
			offset_color = OFFSET_LED_POE_P0;
			mask_color = MASK_LED_POE_P0;
			bInvert = TRUE;
			break;

		case LED_POE_P1:
			offset_color = OFFSET_LED_POE_P1;
			mask_color = MASK_LED_POE_P1;
			bInvert = TRUE;
			break;

		case LED_POE_SYS:
			offset_color = OFFSET_LED_POE_SYS;
			mask_color = MASK_LED_POE_SYS;
			bInvert = FALSE;
			break;
		default:
			return;
			break;
	}

	if(addr!=0)
	{
		nioreg = (katar_nio_lvl_t *)addr;
		value = nioreg->led_ctrl;
	}
	
	if(bInvert)
		color = !color;

	value &= ~mask_color;
	value |= ((color<<offset_color)& mask_color);

	if(addr!=0)
		nioreg->led_ctrl = value;
	else
		printf("%s set reg value:0x%x\n",__FUNCTION__,value);

    return;
}

/*********************************************************************
 *
 * Function:    katar_get_poe_led_color
 *
 * Description: get led control register (FPGA_NIO_LED_CTRL_REG	0x000).
 *
 * Inputs:       led_port    target led port
 *
 * Output:      target led color
 *
 *********************************************************************
 */
int katar_get_poe_led_color (uint led_port) {

    unsigned long addr = get_platform_nio_reg_base();
    katar_nio_lvl_t *nioreg;
	uint32_t offset = 0 ,mask = 0;
    uint32_t value = 0;
	uint32_t bInvert = FALSE;

	switch(led_port)
	{
		case LED_POE_P0:
			offset = OFFSET_LED_POE_P0;
			mask = MASK_LED_POE_P0;
			bInvert = TRUE;
			break;

		case LED_POE_P1:
			offset = OFFSET_LED_POE_P1;
			mask = MASK_LED_POE_P1;
			bInvert = TRUE;
			break;

		case LED_POE_SYS:
			offset = OFFSET_LED_POE_SYS;
			mask = MASK_LED_POE_SYS;
			bInvert = FALSE;
			break;
	}
	if(addr != 0)
	{
		nioreg = (katar_nio_lvl_t *)addr;
		value = ((nioreg->led_ctrl&mask)>>offset);
	}
	if(bInvert)
		value = !value;

    return value;
}

static int get_intr_offset_mask(uint intr_typ,uint32_t *offset, uint32_t *mask, uint32_t *reg_typ)
{
	switch(intr_typ)
	{
		case INTR_USB_COM:
            *offset = OFFSET_INTR_USB_COM;
            *mask = MASK_INTR_USB_COM;
			*reg_typ = INTR_MISC_REG;
            break;

		case INTR_DIMM_OVERHEAT:
			*offset = OFFSET_INTR_DIMM;
			*mask = MASK_INTR_DIMM;
			*reg_typ = INTR_IRQ10_REG;
			break;

		case INTR_RESET_BTN:
			*offset = OFFSET_INTR_RESET_BTN;
			*mask = MASK_INTR_RESET_BTN;
			*reg_typ = INTR_RST_BTN_REG;
			break;

		case INTR_FAN_TACH_LOW:
			*offset = OFFSET_INTR_FAN_TACH;
			*mask = MASK_INTR_FAN_TACH;
			*reg_typ = INTR_IRQ10_REG;
			break;

		case INTR_POE:
            *offset = OFFSET_INTR_POE;
            *mask = MASK_INTR_POE;
			*reg_typ = INTR_POE_REG;
            break;

        case INTR_GE_SW:
            *offset = OFFSET_INTR_GE_SW;
            *mask = MASK_INTR_GE_SW;
			*reg_typ = INTR_SFP_REG;
            break;

        case INTR_SFP_P1_PRESENT:
            *offset = OFFSET_INTR_SFP1_PRE;
            *mask = MASK_INTR_SFP1_PRE;
			*reg_typ = INTR_SFP_REG;
            break;

        case INTR_SFP_P1_LOS:
            *offset = OFFSET_INTR_SFP1_LOS;
            *mask = MASK_INTR_SFP1_LOS;
			*reg_typ = INTR_SFP_REG;
            break;

        case INTR_SFP_P1_FAULT:
            *offset = OFFSET_INTR_SFP1_FAU;
            *mask = MASK_INTR_SFP1_FAU;
			*reg_typ = INTR_SFP_REG;
            break;

        case INTR_SFP_P0_PRESENT:
            *offset = OFFSET_INTR_SFP0_PRE;
            *mask = MASK_INTR_SFP0_PRE;
			*reg_typ = INTR_SFP_REG;
            break;

        case INTR_SFP_P0_LOS:
            *offset = OFFSET_INTR_SFP0_LOS;
            *mask = MASK_INTR_SFP0_LOS;
			*reg_typ = INTR_SFP_REG;
            break;

        case INTR_SFP_P0_FAULT:
            *offset = OFFSET_INTR_SFP0_FAU;
            *mask = MASK_INTR_SFP0_FAU;
			*reg_typ = INTR_SFP_REG;
            break;

        case INTR_CCCP_READY:
            *offset = OFFSET_INTR_CCCP_READY;
            *mask = MASK_INTR_CCCP_READY;
            *reg_typ = INTR_IRQ11_REG;
            break;

        case INTR_FPCP_READY:
            *offset = OFFSET_INTR_FPCP_READY;
            *mask = MASK_INTR_FPCP_READY;
            *reg_typ = INTR_IRQ11_REG;
            break;

        case INTR_PKT_READY:
            *offset = OFFSET_INTR_PKT_READY;
            *mask = MASK_INTR_PKT_READY;
            *reg_typ = INTR_IRQ11_REG;
            break;

		case INTR_ILL_ACC:
            *offset = OFFSET_INTR_ILL_ACC;
            *mask = MASK_INTR_ILL_ACC;
            *reg_typ = INTR_IRQ09_REG;
            break;
		
		case INTR_ALL:
			*offset = 0;
            *mask = 0xFFFFFFFF; 
			*reg_typ = INTR_ALL_REG;
            break;

		default:
			*offset = 0;
			*mask = 0;
			return FALSE;
			break;
	}
	return TRUE;
}

/*********************************************************************
 *
 * Function:    katar_check_interupt
 *
 * Description: read interupt stat register (FPGA_INTR_STAT_REG  0x200).
 *
 * Inputs:       intr_typ : interupt type
 *                  bClear : clear after check 
 *
 * Output:      FPGA_IRQ_NONE/ALL/0/1
 *
 *********************************************************************
 */
int katar_check_interupt (uint intr_typ,int bClear) {

	unsigned long addr = get_platform_reg_base();
    katar_nio_lvl_t *nioreg;
	katar_io_lvl_t *ioreg;
	katar_sys_lvl_t *sysreg;
	uint32_t offset = 0 ,mask = 0,reg_typ = 0;
    uint32_t value = 0;

	if(get_intr_offset_mask(intr_typ,&offset,&mask,&reg_typ)==FALSE)
		return FPGA_IRQ_NONE;

	addr = get_platform_reg_base();
	sysreg = (katar_sys_lvl_t *)addr;
	addr = get_platform_nio_reg_base();
	nioreg = (katar_nio_lvl_t *)addr;
	addr = get_platform_io_reg_base();
	ioreg = (katar_io_lvl_t *)addr;

	switch(reg_typ)
	{
		case INTR_IRQ09_REG:
			if(sysreg == NULL)
				return FPGA_IRQ_NONE;
			value = ((sysreg->irq09_stat&mask)>>offset);
			break;
		case INTR_IRQ10_REG:
			if(sysreg == NULL)
				return FPGA_IRQ_NONE;
			value = ((sysreg->irq10_stat&mask)>>offset);
			break;
		case INTR_IRQ11_REG:
			if(sysreg == NULL)
				return FPGA_IRQ_NONE;
			value = ((sysreg->irq11_stat&mask)>>offset);
			break;
		case INTR_RST_BTN_REG:
			if(sysreg == NULL)
                return FPGA_IRQ_NONE;
            value = ((sysreg->rst_button&mask)>>offset);
            break;
		case INTR_MISC_REG:
			if(sysreg == NULL)
				return FPGA_IRQ_NONE;
			value = ((sysreg->intr_stat&mask)>>offset);	
			break;
		case INTR_POE_REG:
			if(nioreg == NULL)
				return FPGA_IRQ_NONE;
			value = ((nioreg->poe_intr_stat&mask)>>offset);
			break;
		case INTR_SFP_REG:
            if(ioreg == NULL)
				return FPGA_IRQ_NONE;
			value = ((ioreg->sfp_intr_stat&mask)>>offset);
            break;
		case INTR_ALL_REG:
			if((sysreg == NULL)|(nioreg == NULL)|(ioreg == NULL))
				return FPGA_IRQ_NONE;
			value = (sysreg->rst_button&MASK_INTR_RESET_BTN)|(sysreg->intr_stat)|(nioreg->poe_intr_stat)|(ioreg->sfp_intr_stat)|
					(sysreg->irq11_stat&MASK_INTR_IRQ11_ALL)|(sysreg->irq09_stat&MASK_INTR_IRQ09_ALL)|(sysreg->irq10_stat&MASK_INTR_IRQ10_ALL);
			break;
		default:
			return FPGA_IRQ_NONE;
			break;
	}

	if(value)
	{
		if(bClear)
			if(katar_clear_interupt(intr_typ)==FAILED)
				printf("Clear %d FAILED\n",intr_typ);

		switch(reg_typ)
		{
			case INTR_MISC_REG:
				return 7;
				break;
			case INTR_IRQ09_REG:
				return 9;
				break;
			case INTR_IRQ10_REG:
				return 10;
				break;
			case INTR_IRQ11_REG:
                return 11;
                break;
			case INTR_RST_BTN_REG:
				return 12;
				break;
			case INTR_POE_REG:
				return 14;
				break;
			case INTR_SFP_REG:
				return 15;
				break;
			default:
				return FPGA_IRQ_ALL;
                break;
		}
	}else
		return FPGA_IRQ_NONE;
}

/*********************************************************************
 *
 * Function:    katar_clear_interupt
 *
 * Description: clear interupt stat register (FPGA_INTR_STAT_REG  0x200).
 *
 * Inputs:       intr_typ : interupt type
 *
 * Output:      PASSED/FAILED
 *
 *********************************************************************
 */
int katar_clear_interupt (uint intr_typ) {

	unsigned long addr = get_platform_reg_base();
    katar_nio_lvl_t *nioreg;
	katar_io_lvl_t *ioreg;
	katar_sys_lvl_t *sysreg;
	uint32_t offset = 0 ,mask = 0,reg_typ = 0;
    uint32_t value = 0;
	int count = 1000;

	if(get_intr_offset_mask(intr_typ,&offset,&mask,&reg_typ)==FALSE)
		return FPGA_IRQ_NONE;

	addr = get_platform_reg_base();
	sysreg = (katar_sys_lvl_t *)addr;
	addr = get_platform_nio_reg_base();
	nioreg = (katar_nio_lvl_t *)addr;
	addr = get_platform_io_reg_base();
	ioreg = (katar_io_lvl_t *)addr;

	do {
		switch(reg_typ)
		{
			case INTR_IRQ09_REG:
				if(sysreg == NULL)
					return FAILED;
				sysreg->irq09_stat = (1<<offset);
				count--;
				msleep(1);
				value = ((sysreg->irq09_stat&mask)>>offset);
				break;
			case INTR_IRQ10_REG:
				if(sysreg == NULL)
					return FAILED;
				sysreg->irq10_stat = (1<<offset);
				//INTR_FAN_TACH_LOW need to clear fan_intr_stat reg too
				if(intr_typ == INTR_FAN_TACH_LOW)
					sysreg->fan_intr_stat = sysreg->fan_intr_stat;
				count--;
				msleep(1);
				value = ((sysreg->irq10_stat&mask)>>offset);
				break;
			case INTR_IRQ11_REG:
				if(sysreg == NULL)
					return FAILED;
				sysreg->irq11_stat = (1<<offset);
				count--;
				msleep(1);
				value = ((sysreg->irq11_stat&mask)>>offset);
				break;
			case INTR_RST_BTN_REG:
				if(sysreg == NULL)
                    return FAILED;
                sysreg->rst_button = (1<<offset);
                count--;
                msleep(1);
                value = ((sysreg->rst_button&mask)>>offset);
                break;
			case INTR_MISC_REG:
				if(sysreg == NULL)
					return FAILED;
				sysreg->intr_stat = (1<<offset);
				count--;
				msleep(1);
				value = ((sysreg->intr_stat&mask)>>offset); 
				break;
			case INTR_POE_REG:
				if(nioreg == NULL)
					return FAILED;
				nioreg->poe_intr_stat = (1<<offset);
				count--;
				msleep(1);
				value = ((nioreg->poe_intr_stat&mask)>>offset);
				break;
			case INTR_SFP_REG:
				if(ioreg == NULL)
					return FAILED;
				ioreg->sfp_intr_stat = (1<<offset);
				count--;
				msleep(1);
				value = ((ioreg->sfp_intr_stat&mask)>>offset);
				break;
			case INTR_ALL_REG:
				if((sysreg == NULL)|(nioreg == NULL)|(ioreg == NULL))
					return FAILED;
				sysreg->irq10_stat = sysreg->irq10_stat;
				sysreg->irq11_stat &= (0x6);
				sysreg->irq09_stat &= (0x4);
				sysreg->intr_stat = sysreg->intr_stat;
				nioreg->poe_intr_stat = nioreg->poe_intr_stat;
				ioreg->sfp_intr_stat = ioreg->sfp_intr_stat;
				count--;
				msleep(1);
				value = (sysreg->irq10_stat)|(sysreg->intr_stat)|(nioreg->poe_intr_stat)|(ioreg->sfp_intr_stat)|
                    (sysreg->irq11_stat&0x6)|(sysreg->irq09_stat&0x4);
				break;
			default:
				return FPGA_IRQ_NONE;
				break;
		}
	} while (value && (count > 0));
	if(value)
		return FAILED;
	else
		return PASSED;
}

/*********************************************************************
 *
 * Function:    katar_check_interupt_mask
 *
 * Description: read interupt mask register (FPGA_INTR_MASK_REG 0x204).
 *
 * Inputs:       intr_typ : interupt type
 *
 * Output:      TRUE/FALSE
 *
 *********************************************************************
 */
int katar_check_interupt_mask (uint intr_typ) {

	unsigned long addr = get_platform_reg_base();
    katar_nio_lvl_t *nioreg;
	katar_io_lvl_t *ioreg;
	katar_sys_lvl_t *sysreg;
	uint32_t offset = 0 ,mask = 0,reg_typ = 0;
    uint32_t value = 0;

	if(get_intr_offset_mask(intr_typ,&offset,&mask,&reg_typ)==FALSE)
		return FALSE;

	addr = get_platform_reg_base();
	sysreg = (katar_sys_lvl_t *)addr;
	addr = get_platform_nio_reg_base();
	nioreg = (katar_nio_lvl_t *)addr;
	addr = get_platform_io_reg_base();
	ioreg = (katar_io_lvl_t *)addr;

	switch(reg_typ)
	{
		case INTR_IRQ09_REG:
			if(sysreg == NULL)
				return FALSE;
			value = ((sysreg->irq09_mask&mask)>>offset);
			break;
		case INTR_IRQ10_REG:
			if(sysreg == NULL)
				return FALSE;
			value = ((sysreg->irq10_mask&mask)>>offset);
			if(intr_typ == INTR_FAN_TACH_LOW)
				value |= (sysreg->fan_intr_mask&0x3);
			break;
		case INTR_IRQ11_REG:
			if(sysreg == NULL)
				return FALSE;
			value = ((sysreg->irq11_mask&mask)>>offset);
			break;
		case INTR_RST_BTN_REG:
			if(sysreg == NULL)
                return FALSE;
            value = ((sysreg->rst_button_mask&mask)>>offset);
            break;
		case INTR_MISC_REG:
			if(sysreg == NULL)
				return FALSE;
			value = ((sysreg->intr_mask&mask)>>offset);	
			break;
		case INTR_POE_REG:
			if(nioreg == NULL)
				return FALSE;
			value = ((nioreg->poe_intr_mask&mask)>>offset);
			break;
		case INTR_SFP_REG:
            if(ioreg == NULL)
				return FALSE;
			value = ((ioreg->sfp_intr_mask&mask)>>offset);
            break;
		default:
			return FALSE;
			break;
	}
	if(value)
		return TRUE;
	else
		return FALSE;
}

/*********************************************************************
 *
 * Function:    katar_interupt_mask_control
 *
 * Description: set interupt mask register (FPGA_INTR_MASK_REG 	0x204).
 *
 * Inputs:       intr_typ : interupt type
 *                  bSet : 1 for interrupted masked off , 0 for interrupt not masked off 
 *
 * Output:      none
 *
 *********************************************************************
 */
void katar_interupt_mask_control (uint intr_typ ,int bSet) {

	unsigned long addr = get_platform_reg_base();
    katar_nio_lvl_t *nioreg;
	katar_io_lvl_t *ioreg;
	katar_sys_lvl_t *sysreg;
	uint32_t offset = 0 ,mask = 0,reg_typ = 0;

	if(get_intr_offset_mask(intr_typ,&offset,&mask,&reg_typ)==FALSE)
		return;

	addr = get_platform_reg_base();
	sysreg = (katar_sys_lvl_t *)addr;
	addr = get_platform_nio_reg_base();
	nioreg = (katar_nio_lvl_t *)addr;
	addr = get_platform_io_reg_base();
	ioreg = (katar_io_lvl_t *)addr;

	switch(reg_typ)
	{
		case INTR_IRQ09_REG:
			if(sysreg == NULL)
				return;
			if(bSet)
				sysreg->irq09_mask |= (1<<offset);
			else
				sysreg->irq09_mask &= ~(mask);
			break;
		case INTR_IRQ10_REG:
			if(sysreg == NULL)
				return;
			if(bSet)
			{
                sysreg->irq10_mask |= (1<<offset);
				if(intr_typ == INTR_FAN_TACH_LOW)
					sysreg->fan_intr_mask = 0x3;
            }else
			{
                sysreg->irq10_mask &= ~(mask);
				if(intr_typ == INTR_FAN_TACH_LOW)
                    sysreg->fan_intr_mask = 0x0;
			}
			break;
		case INTR_IRQ11_REG:
			if(sysreg == NULL)
				return;
			if(bSet)
				sysreg->irq11_mask |= (1<<offset);
			else
				sysreg->irq11_mask &= ~(mask);
			break;
		case INTR_RST_BTN_REG:
            if(sysreg == NULL)
                return;
            if(bSet)
                sysreg->rst_button_mask |= (1<<offset);
            else
                sysreg->rst_button_mask &= ~(mask);
            break;
		case INTR_MISC_REG:
			if(sysreg == NULL)
				return;
			if(bSet)
				sysreg->intr_mask |= (1<<offset);
			else
				sysreg->intr_mask &= ~(mask);
			break;
		case INTR_POE_REG:
			if(nioreg == NULL)
				return;
			if(bSet)
				nioreg->poe_intr_mask |= (1<<offset);
			else
				nioreg->poe_intr_mask &= ~(mask);
			break;
		case INTR_SFP_REG:
            if(ioreg == NULL)
				return;
			if(bSet)
				ioreg->sfp_intr_mask |= (1<<offset);
			else
				ioreg->sfp_intr_mask &= ~(mask);
            break;
		default:
			return;
			break;
	}
	return;
}

void katar_interupt_mask_clear_for_mb_test(void)
{
    unsigned long addr = get_platform_reg_base();
    katar_nio_lvl_t *nioreg;
    katar_io_lvl_t *ioreg;
    katar_sys_lvl_t *sysreg;
	
	addr = get_platform_reg_base();
    sysreg = (katar_sys_lvl_t *)addr;
    addr = get_platform_nio_reg_base();
    nioreg = (katar_nio_lvl_t *)addr;
    addr = get_platform_io_reg_base();
    ioreg = (katar_io_lvl_t *)addr;

	sysreg->irq09_mask = 0;
	sysreg->irq10_mask = 0;
	sysreg->irq11_mask = 0;
	sysreg->fan_intr_mask = 0;
	sysreg->rst_button_mask = 0;
	sysreg->intr_mask = 0;
	nioreg->poe_intr_mask = 0;
	ioreg->sfp_intr_mask = 0x3600;
	return;	
}

/*********************************************************************
 *
 * Function:    katar_force_interupt
 *
 * Description: set force interupt register (FPGA_INTR_MASK_REG 	0x204).
 *
 * Inputs:       intr_typ : interupt type
 *
 * Output:      none
 *
 *********************************************************************
 */
int katar_force_interupt (uint intr_typ) {

	unsigned long addr = get_platform_reg_base();
    katar_nio_lvl_t *nioreg;
	katar_io_lvl_t *ioreg;
	katar_sys_lvl_t *sysreg;
	uint32_t offset = 0 ,mask = 0,reg_typ = 0;
	uint32_t value = 0;

	if(get_intr_offset_mask(intr_typ,&offset,&mask,&reg_typ)==FALSE)
		return FAILED;;

	addr = get_platform_reg_base();
	sysreg = (katar_sys_lvl_t *)addr;
	addr = get_platform_nio_reg_base();
	nioreg = (katar_nio_lvl_t *)addr;
	addr = get_platform_io_reg_base();
	ioreg = (katar_io_lvl_t *)addr;

	switch(reg_typ)
	{
		case INTR_IRQ09_REG:
			addr = get_platform_reg_base();
			if(addr == 0)
				return FAILED;
			switch(intr_typ)
            {
                default:
                    return FAILED;
                    break;
				case INTR_ILL_ACC:
					register_write((addr +  0xF00), 1, BW_32BITS);
					break; 
			}
			break;
		case INTR_IRQ10_REG:
		case INTR_RST_BTN_REG:
			return FAILED;
			break;
		case INTR_IRQ11_REG:
			if(sysreg == NULL)
                return FAILED;
			switch(intr_typ)
			{
				default:
					return FAILED;
					break;
				case INTR_CCCP_READY:
					sysreg->cc_status = sysreg->cc_status ^ 1;
					break;
                case INTR_FPCP_READY:
					sysreg->fp_status = sysreg->fp_status ^ 1;
                    break;
                case INTR_PKT_READY:
					sysreg->fp_status_2 = sysreg->fp_status_2 ^ 1;
                    break;
			}
			break;
		case INTR_MISC_REG:
			if(sysreg == NULL)
				return FAILED;
			value = sysreg->intr_force;
			value &= ~(mask);
			value |= (1<<offset);
			sysreg->intr_force = value;
			break;
		case INTR_POE_REG:
			if(nioreg == NULL)
				return FAILED;
			value = nioreg->poe_intr_force;
			value &= ~(mask);
			value |= (1<<offset);
			nioreg->poe_intr_force = value;
			break;
		case INTR_SFP_REG:
            if(ioreg == NULL)
				return FAILED;
			value = ioreg->sfp_intr_force;
			value &= ~(mask);
			value |= (1<<offset);
			ioreg->sfp_intr_force = value;
            break;
		default:
			return FAILED;
			break;
	}
	return PASSED;
}

int katar_get_poe_54V_present (void) {

    unsigned long addr = get_platform_nio_reg_base();
    katar_nio_lvl_t *nioreg;
    uint32_t value = 0;

	if(katar_get_usrlogic_ver()<=FPGA_SPEC_1_7_VER)
    {
        printf("Usrlogic FPGA ver too old,please update\n");
        return (value);
    }

    if(addr != 0)
    {
        nioreg = (katar_nio_lvl_t *)addr;
        value = nioreg->poe_pow;
        value &= 0x1;
    }
    return (value);
}

/*********************************************************************
 *
 * Function:    katar_get_usb_com_stat
 *
 * Description: read usb com control register (FPGA_USB_CONSOLE_REG 0x148).
 *
 * Inputs:       none
 *
 * Output:      Console USB Detect value
 *
 *********************************************************************
 */
int katar_get_usb_com_stat (void) {

    unsigned long addr = get_platform_reg_base();
    katar_sys_lvl_t *sysreg;
    uint32_t value = 0;

	if(addr != 0)
	{
		sysreg = (katar_sys_lvl_t *)addr;
        value = sysreg->usb_com;
		value = ((value&MASK_USB_COM_DETECT)>>OFFSET_USB_COM_DETECT);
	}
    return (value);
}

/*********************************************************************
 *
 * Function:    katar_get_usb_com_manual
 *
 * Description: read usb com control register (FPGA_USB_CONSOLE_REG 0x148).
 *
 * Inputs:       none
 *
 * Output:      Console USB Manual value
 *
 *********************************************************************
 */
int katar_get_usb_com_manual (void) {

    unsigned long addr = get_platform_reg_base();
    katar_sys_lvl_t *sysreg;
    uint32_t value = 0;

	if(addr != 0)
	{
		sysreg = (katar_sys_lvl_t *)addr;
        value = sysreg->usb_com;
		value = ((value&MASK_USB_COM_MANUAL)>>OFFSET_USB_COM_MANUAL);
	}
    return (value);
}

/*********************************************************************
 *
 * Function:    katar_set_usb_com_manual
 *
 * Description: set usb com control register (FPGA_USB_CONSOLE_REG 0x148).
 *
 * Inputs:       bEnable : set to 1 to enable usb manual control;
 *					 set to 0 for automatically select
 *
 * Output:      none
 *
 *********************************************************************
 */
void katar_set_usb_com_manual (int bEnable) {

    unsigned long addr = get_platform_reg_base();
    katar_sys_lvl_t *sysreg;
    uint32_t value = 0;

	if(addr!=0)
	{
		sysreg = (katar_sys_lvl_t *)addr;
		value = sysreg->usb_com;
	}

	value &= ~MASK_USB_COM_MANUAL;
	if(bEnable)
		value |= ((1<<OFFSET_USB_COM_MANUAL) & MASK_USB_COM_MANUAL);

	if(addr!=0)
		sysreg->usb_com = value;
	else
		printf("%s set reg value:0x%x\n",__FUNCTION__,value);

	msleep(100);
    return;
}

/*********************************************************************
 *
 * Function:    katar_set_usb_com_control
 *
 * Description: set usb com control register (FPGA_USB_CONSOLE_REG 0x148).
 *
 * Inputs:       bEnable : set to 1 to enable usb console;set to 0 for disable
 *
 * Output:      none
 *
 *********************************************************************
 */
void katar_set_usb_com_control (int bEnable) {

    unsigned long addr = get_platform_reg_base();
    katar_sys_lvl_t *sysreg;
    uint32_t value = 0;

	if(addr!=0)
	{
		sysreg = (katar_sys_lvl_t *)addr;
		value = sysreg->usb_com;
	}

	//Do nothing if USB Manual Mux Select bit is 0 (Automatically select)
	if((value&OFFSET_USB_COM_MANUAL)==FALSE)
		return;

	value &= ~MASK_USB_COM_SELECT;
	if(bEnable)
		value |= ((1<<OFFSET_USB_COM_SELECT) & MASK_USB_COM_SELECT);

	if(addr!=0)
	{
		if(bEnable)
			printf("Switch to USB console start\n");
		else
			printf("Switch to normal console start\n");

		sysreg->usb_com = value;
		msleep(100);

		if(bEnable)
            printf("Switch to USB console done\n");
        else
            printf("Switch to normal console done\n");
	}
	else
		printf("%s set reg value:0x%x\n",__FUNCTION__,value);

    return;
}

/*********************************************************************
 *
 * Function:    katar_get_rst_btn_info
 *
 * Description: get reset button status register (FPGA_RESET_BUTTON_REG 0x218).
 *
 * Inputs:       btn_stat : pointer to get Press Push-Button Switch Status
 *			btn_dur : pointer to get Press Push-Button Switch Duration
 *			bClear : set 1 to clear after read Press Push-Button Switch Duration
 *
 * Output:      PASSED/FAILED
 *
 *********************************************************************
 */
int katar_get_rst_btn_info (int *btn_stat,int *btn_dur,int bClear)
{
    unsigned long addr = get_platform_reg_base();
	katar_sys_lvl_t *sysreg;
    uint32_t value = 0;

	if(katar_get_usrlogic_ver()<=FPGA_SPEC_1_7_VER)
    {
        printf("Usrlogic FPGA ver too old,please update\n");
        return FAILED;
    }

	if(addr == 0)
		return FAILED;

	sysreg = (katar_sys_lvl_t *)addr;
	value = sysreg->rst_button;
	*btn_stat = ((value&MASK_RST_BTN_STAT)>>OFFSET_RST_BTN_STAT);
	*btn_dur = ((value&MASK_RST_BTN_DUR)>>OFFSET_RST_BTN_DUR);
	
	if(bClear)
	{
		int count = 1000;

	    do {
		/*W1C to clear interupt*/
		sysreg->rst_button |= (1<<OFFSET_RST_BTN_DUR);
        count--;
        msleep(1);
		value = ((sysreg->rst_button&MASK_RST_BTN_DUR)>>OFFSET_RST_BTN_DUR);
	    } while (value && (count > 0));
		
		if(value)
		    return FAILED;
		else
			return PASSED;
	}else
		return PASSED;
}

/*********************************************************************
 *
 * Function:    katar_reset_device
 *
 * Description: set device reset register (FPGA_LPC_EXT_DEV_RST_REG     0x01C).
 *
 * Inputs:       dev_typ : device type
 *                  bReset : 1 for reset device ; 0 for unreset device
 *
 * Output:      none
 *
 *********************************************************************
 */
void katar_reset_device (uint dev_typ, uint bReset) {

    unsigned long addr = 0;
    katar_sys_lvl_t *sysreg;
	katar_nio_lvl_t *nioreg;
    katar_io_lvl_t *ioreg;
	uint32_t mask = 0;
	uint8 reg_type = FPGA_RSTDEV_REG;

	switch(dev_typ)
	{
		case RSTDEV_USB30:
			mask = MASK_RSTDEV_USB30;
			reg_type = FPGA_RSTDEV_REG;
			break;
		case RSTDEV_USB_HUB:
			mask = MASK_RSTDEV_USB_HUB;
			reg_type = FPGA_RSTDEV_REG;
			break;			
		case RSTDEV_GE_PHY_0:
			mask = MASK_RSTDEV_GE_PHY_0;
			reg_type = FPGA_RSTDEV_REG;
			break;
		case RSTDEV_GE_PHY_1:
			mask = MASK_RSTDEV_GE_PHY_1;
			reg_type = FPGA_RSTDEV_REG;
			break;
		case RSTDEV_EMMC:
			mask = MASK_RSTDEV_EMMC;
			reg_type = FPGA_RSTDEV_REG;
			break;	
		case RSTDEV_POE:
			mask = MASK_RSTDEV_POE;
			reg_type = NIO_RSTDEV_REG;
			break;
		case RSTDEV_25G_PHY:
            mask = MASK_RSTDEV_25G_PHY;
            reg_type = IO_RSTDEV_REG;
			break;
        case RSTDEV_SFP_MUX:
            mask = MASK_RSTDEV_SFP_MUX;
            reg_type = IO_RSTDEV_REG;
            break;
        case RSTDEV_10G_PHY_B:
            mask = MASK_RSTDEV_10G_PHY_B;
            reg_type = IO_RSTDEV_REG;
            break;
        case RSTDEV_10G_PHY_A:
            mask = MASK_RSTDEV_10G_PHY_A;
            reg_type = IO_RSTDEV_REG;
            break;
		default:
			return;
			break;
	}
	switch(reg_type)
	{
		case FPGA_RSTDEV_REG:
			addr = get_platform_reg_base();
			if(addr==0)
				return;
			sysreg = (katar_sys_lvl_t *)addr;
            if(bReset)
                sysreg->dev_rst |= mask;
            else
                sysreg->dev_rst &= ~mask;
			break;
		case NIO_RSTDEV_REG:
			addr = get_platform_nio_reg_base();
			if(addr==0)
                return;
			nioreg = (katar_nio_lvl_t *)addr;
            if(bReset)
                nioreg->poe_reset |= mask;
            else
                nioreg->poe_reset &= ~mask;
			break;
		case IO_RSTDEV_REG:
			addr = get_platform_io_reg_base();
			if(addr==0)
                return;
			ioreg = (katar_io_lvl_t *)addr;
            if(bReset)
                ioreg->dev_rst |= mask;
            else
                ioreg->dev_rst &= ~mask;
			break;
	}
    return;
}

int katar_get_usrlogic_ver (void)
{
	unsigned long addr = get_platform_reg_base();
    katar_sys_lvl_t *sysreg;
	uint32_t ver = 0;

	if(addr!=0)
	{
		sysreg = (katar_sys_lvl_t *)addr;
		ver = sysreg->ver;
	}
	return ver;
}

/*-------------------------------------------------------------------
 *
 * Function: katar_get_platform_ver
 * 
 *
 * Input: verbose: if flag set to true then print version
 * Output: cpld_ver: cpld version
 *         cpld_Brd: cpld board revision
 *         fpga_brad: fpga brd rev
 *         always returns PASSED
 *
 *-------------------------------------------------------------------
 */
int
katar_get_platform_ver (unsigned int verbose, unsigned int *cpld_ver,
                      unsigned int *fpga_ver, unsigned int *cpld_brd,
                      unsigned int *fpga_brd)
{

    unsigned long addr = get_platform_reg_base();
    katar_sys_lvl_t *sysreg;
    uint32_t brd = 0;
	uint32_t ver = 0,sec_ver = 0,sec_id = 0;

	if(addr!=0)
	{
		sysreg = (katar_sys_lvl_t *)addr;
		brd = sysreg->dbg_ctrl;
		ver = sysreg->ver;
		sec_ver = sysreg->sec_ver_date;
		sec_id = sysreg->sec_ver_id;
	}
	*fpga_ver = ver;
	*fpga_brd = (brd&0x600)>>9;
	*cpld_ver = sec_ver;
	*cpld_brd = sec_id;

    if (verbose) {
        printf("UsrLogic FPGA Version: %08x\nAikido FPGA Version: %08x , Id: 0x%x\n",*fpga_ver,*cpld_ver,*cpld_brd);
    }
    return PASSED;
}

/*-------------------------------------------------------------------
 *
 * Function: katar_boot_spi_select_control
 * 
 * Description: set spi boot select register (FPGA_LPC_SPI_CTRL_REG     0x058).
 *
 * Input: bClear: if flag set to true then SPI boot chip via hardware decision
 *           boot_typ : force to boot from select chip , not work if bClear is set
 *         always returns PASSED
 *
 *-------------------------------------------------------------------
 */
int katar_boot_spi_select_control(int bClear, unsigned int boot_typ)
{
    unsigned long addr = get_platform_reg_base();
    katar_sys_lvl_t *sysreg;
    uint32_t value = 0;

	if(bClear==FALSE)
		value = (1<<OFFSET_SPICTL_BOOT_OVRD)|((boot_typ&1)<<OFFSET_SPICTL_BOOT_SEL);

	if(addr!=0)
	{
		sysreg = (katar_sys_lvl_t *)addr;
		sysreg->spi_ctrl = value;
	}
	return PASSED;
}


void katar_set_prom_opcode (int opcode,int address) {

    unsigned long addr = get_platform_prom_reg_base();
    katar_spi_lvl_t *promreg;
    uint32_t value = 0;

	value = ((opcode & 0xFF)<<OFFSET_PROM_OP_CODE)|((address & MASK_PROM_OP_ADDR)<<OFFSET_PROM_OP_ADDR);
	
	if(addr!=0)
	{
		promreg = (katar_spi_lvl_t *)addr;
		promreg->op_code = value;
	}

//	printf("--%s(%d) value:%x\n",__FUNCTION__,__LINE__,value);
    return;
}

void katar_set_prom_read_length(int legth) {

    unsigned long addr = get_platform_prom_reg_base();
    katar_spi_lvl_t *promreg;
    uint32_t value = 0;

	//Set 0 for read 1 byte, set 0xFF for read 256 Byte
	value = ((legth-1) & MASK_PROM_READ_SIZE);

	if(addr!=0)
	{
		promreg = (katar_spi_lvl_t *)addr;
		promreg->read_size = value;
	}
//	printf("--%s(%d) value:%x\n",__FUNCTION__,__LINE__,value);
    return;
}

void katar_set_prom_write_data(uint8_t data) {
	
    unsigned long addr = get_platform_prom_reg_base();
    katar_spi_lvl_t *promreg;
    uint32_t value = 0;

	value = (data & MASK_PROM_RW_DATA);
	
	if(addr!=0)
	{
		promreg = (katar_spi_lvl_t *)addr;
		promreg->data = value;
	}
//	printf("--%s(%d) value:%x\n",__FUNCTION__,__LINE__,value);
    return;	
}

uint8_t katar_get_prom_read_data(void) {
	
    unsigned long addr = get_platform_prom_reg_base();
    katar_spi_lvl_t *promreg;
    uint32_t value = 0;
	uint8_t result = 0;

	if(addr!=0)
	{
		promreg = (katar_spi_lvl_t *)addr;
		value = promreg->data;
	}
	result = (uint8_t) (value & MASK_PROM_RW_DATA);
//	printf("--%s(%d) value:%x\n",__FUNCTION__,__LINE__,value);
    return result;
}

int katar_check_prom_FIFO_Empty(boolean bCheckRead) {
	
    unsigned long addr = get_platform_prom_reg_base();
    katar_spi_lvl_t *promreg;
    uint32_t value = 0;
	uint32_t offset = 0;

	if(addr!=0)
	{
		promreg = (katar_spi_lvl_t *)addr;
		value = promreg->stat;
	}
	
	if(bCheckRead)
		offset = OFFSET_PROM_STAT_R_EMPTY;
	else
		offset = OFFSET_PROM_STAT_W_EMPTY;

	value = (value & (1<<offset)) >> offset;
//	printf("--%s(%d) value:%x\n",__FUNCTION__,__LINE__,value);
	return value;	
}

int katar_clear_prom_op_done(void) {

    unsigned long addr = get_platform_prom_reg_base();
    katar_spi_lvl_t *promreg;
    uint32_t value = 0;
	int count = 1000;

	if(addr==0)
	{
		return FAILED;
	}
	
	promreg = (katar_spi_lvl_t *)addr;
	value = ((promreg->stat&(1<<OFFSET_PROM_STAT_DONE))>>OFFSET_PROM_STAT_DONE);
//	printf("--%s(%d) value:%x\n",__FUNCTION__,__LINE__,value);
	while (value && (count > 0)) 
	{
		/*W1C to clear done bit*/
		promreg->stat |= (1<<OFFSET_PROM_STAT_DONE);
		count--;
		msleep(1);
		value = ((promreg->stat&(1<<OFFSET_PROM_STAT_DONE))>>OFFSET_PROM_STAT_DONE);
	}

	if(count > 0)
		return PASSED;
	else
		return FAILED;
}

int katar_clear_prom_FIFO_status(boolean bClrRead) {

    unsigned long addr = get_platform_prom_reg_base();
    katar_spi_lvl_t *promreg;
    uint32_t value = 0;
	int count = 1000;

	if(addr==0)
	{
		return FAILED;
	}
	
	promreg = (katar_spi_lvl_t *)addr;
//	printf("--%s(%d) value:%x\n",__FUNCTION__,__LINE__,value);
	if(bClrRead)
	{
		int count = 1000;

		while( (katar_check_prom_FIFO_Empty(TRUE) != TRUE) && (count > 0))
		{
			katar_get_prom_read_data();
			count--;
			msleep(1);
		}
		
		if(count == 0)
			return FAILED;
	}else
	{
		count = 1000;
		value = ((promreg->stat&(1<<OFFSET_PROM_STAT_W_ERR))>>OFFSET_PROM_STAT_W_ERR);
		while (value && (count > 0)) 
		{
			/*W1C to clear write overrun bit*/
			promreg->stat |= (1<<OFFSET_PROM_STAT_W_ERR);
			count--;
			msleep(1);
			value = ((promreg->stat&(1<<OFFSET_PROM_STAT_W_ERR))>>OFFSET_PROM_STAT_W_ERR);
		}
		
		if(count == 0)
			return FAILED;
	}

	return PASSED;
}

int katar_check_prom_op_done(boolean bClear) {
	
    unsigned long addr = get_platform_prom_reg_base();
    katar_spi_lvl_t *promreg;
    uint32_t value = 0;
	int result = 0;

	if(addr == 0)
		return result;
	
	promreg = (katar_spi_lvl_t *)addr;
	value = promreg->stat;
	result = ((value &(1<<OFFSET_PROM_STAT_DONE))>>OFFSET_PROM_STAT_DONE);

//	printf("--%s(%d) value:%x\n",__FUNCTION__,__LINE__,value);

	if(bClear)
	{
		int count = 1000;

		value = result;
		while (value && (count > 0)) 
		{
			/*W1C to clear done bit*/
			promreg->stat |= (1<<OFFSET_PROM_STAT_DONE);
			count--;
			msleep(1);
			value = ((promreg->stat&(1<<OFFSET_PROM_STAT_DONE))>>OFFSET_PROM_STAT_DONE);
		}
	}	
    return result;

}

void katar_set_prom_control(boolean bWrite, boolean bUseAddr, boolean bUseDummy, boolean bSwapByte) {	
    unsigned long addr = get_platform_prom_reg_base();
    katar_spi_lvl_t *promreg;
    uint32_t value = 0;

	//set default Baud Rate Divisor [10:4] , SPI Frequency = 50 MHz /( (Divisor + 1) * 2).  Default frequency is 12.5MHz.
	value = 0x00000010;
	value |= (bWrite<<OFFSET_PROM_CTRL_DIR)|(bUseAddr<<OFFSET_PROM_CTRL_USEADDR)|
			(bUseDummy<<OFFSET_PROM_CTRL_USEDUMMY)|(bSwapByte<<OFFSET_PROM_CTRL_SWAPBYTE);
	
	if(addr!=0)
	{
		promreg = (katar_spi_lvl_t *)addr;
		promreg->ctrl = value;
	}

//	printf("--%s(%d) value:%x\n",__FUNCTION__,__LINE__,value);

    return;
}


//Only used name,offset,size for display_fpga_regs
static reg_info_t katar_fpga_regs[] = {
//    {"LPC reserve",       		0x0,  						READ_ONLY,  {4}, MASK_32, 0},
    {"LPC Reset Reason",  		FPGA_LPC_RESET_REASON_REG,  READ_ONLY,  {4}, MASK_32, 0},
    {"LPC Scratchpad",    		FPGA_LPC_SCRATCHPAD_REG,  	READ_WRITE, {4}, MASK_32, 0},
    {"LPC Status",        		FPGA_LPC_STATUS_REG,  		READ_ONLY,  {4}, MASK_32, 0},
    {"LPC CPRSTCNTRL",          FPGA_LPC_CPRSTCNTRL_REG,    READ_WRITE, {4}, MASK_32, 0},
    {"LPC Status LED Control",	FPGA_LPC_STAT_LED_CTRL_REG, READ_WRITE, {4}, MASK_32, 0},
    {"LPC Debug Control",     	FPGA_LPC_DEBUG_CTRL_REG, 	READ_WRITE, {4}, MASK_32, 0},
    {"LPC Dev Reset Control",   FPGA_LPC_EXT_DEV_RST_REG, 	READ_WRITE, {4}, MASK_32, 0},
    {"LPC RP LED Control",   	FPGA_LPC_RP_LED_CTRL_REG,   READ_WRITE, {4}, MASK_32, 0},
    {"LPC Intr IRQ09 Status",   FPGA_LPC_IRQ09_STAT_REG, 	READ_ONLY,  {4}, MASK_32, 0},
    {"LPC Intr IRQ09 Mask",   	FPGA_LPC_IRQ09_MASK_REG, 	READ_ONLY,  {4}, MASK_32, 0},
    {"LPC Intr IRQ10 Status",   FPGA_LPC_IRQ10_STAT_REG,    READ_ONLY,  {4}, MASK_32, 0},
    {"LPC Intr IRQ10 Mask",     FPGA_LPC_IRQ10_MASK_REG,    READ_ONLY,  {4}, MASK_32, 0},
    {"LPC Intr IRQ11 Status",   FPGA_LPC_IRQ11_STAT_REG,    READ_ONLY,  {4}, MASK_32, 0},
    {"LPC Intr IRQ11 Mask",     FPGA_LPC_IRQ11_MASK_REG,    READ_ONLY,  {4}, MASK_32, 0},
    {"LPC Scratchpad1",         FPGA_LPC_SCRATCHPAD1_REG,   READ_WRITE, {4}, MASK_32, 0},
    {"LED Management Control",	FPGA_LED_CTRL_REG, 			READ_WRITE, {4}, MASK_32, 0},
    {"LPC Chassis Test",     	FPGA_IRQ_TEST_REG, 			READ_WRITE, {4}, MASK_32, 0},
    {"LPC SPI Control",   		FPGA_LPC_SPI_CTRL_REG, 		READ_WRITE, {4}, MASK_32, 0},
    {"LPC Boot Timer",          FPGA_LPC_BOOT_TIMER_REG,    READ_WRITE, {4}, MASK_32, 0},
    {"LPC Board Type",  		FPGA_LPC_BOARDTYPE_REG, 	READ_ONLY,  {4}, MASK_32, 0},
    {"LPC Version",  			FPGA_LPC_VERSION_REG, 		READ_ONLY, 	{4}, MASK_32, 0},
    {"LPC Secure Boot Status",  FPGA_SEC_BOOT_STAT_REG, 	READ_ONLY,  {4}, MASK_32, 0},
    {"LPC Secure Boot Signing", FPGA_SEC_BOOT_SIGN_REG, 	READ_ONLY,  {4}, MASK_32, 0},
    {"SEC FP Status 2",   		FPGA_SEC_FP_STATUS_2_REG,   READ_ONLY,  {4}, MASK_32, 0},
    {"SEC FPGA Version Date",   FPGA_SEC_VERSION_DATE_REG,  READ_ONLY,  {4}, MASK_32, 0},
    {"SEC FPGA Version Id",     FPGA_SEC_VERSION_ID_REG,    READ_ONLY,  {4}, MASK_32, 0},
    {"LPC Conf Header Control", FPGA_CONF_HEADER_CTRL_REG,  READ_ONLY,  {4}, MASK_32, 0},
    {"LPC Conf Header Debug",   FPGA_CONF_HEADER_DBG_REG,   READ_ONLY,  {4}, MASK_32, 0},
    {"LPC SKU Feature",   		FPGA_LPC_SKU_FEATURE_REG,   READ_ONLY,  {4}, MASK_32, 0},
    {"LPC Fan Intr Status",     FPGA_FAN_INTR_STAT_REG,     READ_ONLY,  {4}, MASK_32, 0},
    {"LPC Fan Intr Mask",       FPGA_FAN_INTR_MASK_REG,     READ_ONLY,  {4}, MASK_32, 0},
	{"LPC Fan Status",   		FPAG_FAN_STAT_REG, 			READ_ONLY,  {4}, MASK_32, 0},
    {"LPC Fan Control",    		FPGA_FAN_CTRL_REG, 			READ_WRITE, {4}, MASK_32, 0},
    {"IOS Watchdog Strobe",   	FPGA_IOS_WATCHDOG_REG, 	  	READ_ONLY,  {4}, MASK_32, 0},
    {"LPC Ext Intr Pending",   	FPGA_EXT_INTR_PEND_REG,   	READ_ONLY,  {4}, MASK_32, 0},
    {"LPC Ext Intr Mask",   	FPGA_EXT_INTR_MASK_REG,   	READ_ONLY,  {4}, MASK_32, 0},
    {"LPC Force Ext Intr",   	FPGA_EXT_INTR_FORCE_REG,   	READ_ONLY,  {4}, MASK_32, 0},
	{"LPC USB Console", 	  	FPGA_USB_CONSOLE_REG,		READ_WRITE,	{4}, MASK_32, 0},
	{"LPC Interrupt Status", 	FPGA_INTR_STAT_REG,			READ_ONLY,	{4}, MASK_32, 0},
	{"LPC Interrupt Mask", 	  	FPGA_INTR_MASK_REG,			READ_WRITE,	{4}, MASK_32, 0},
	{"LPC Force Interrupt", 	FPGA_INTR_FORCE_REG,		READ_WRITE,	{4}, MASK_32, 0},
    {"LPC Reset Button",     	FPGA_RST_BUTTON_REG,        READ_ONLY, 	{4}, MASK_32, 0},
    {"LPC Reset Button Mask",   FPGA_RST_BUTTON_MASK_REG,   READ_ONLY,  {4}, MASK_32, 0},
    {"SEC CC Status",           FPGA_SEC_CC_STATUS_REG,     READ_ONLY,  {4}, MASK_32, 0},
    {"SEC FP Status",           FPGA_SEC_FP_STATUS_REG,     READ_ONLY,  {4}, MASK_32, 0},
    {"END",               		0x000,       				0,   		{0}, MASK_32, 0},
};

static reg_info_t katar_fpga_nio_regs[] = {
    {"NI/O POE LED Control",    FPGA_NIO_LED_CTRL_REG,      READ_WRITE, {4}, MASK_32, 0},
    {"NI/O POE Power Status",   FPGA_NIO_POE_POW_REG,      	READ_ONLY, 	{4}, MASK_32, 0},
    {"NI/O POE Reset",   		FPGA_NIO_POE_RESET_REG,     READ_WRITE, {4}, MASK_32, 0},
    {"NI/O POE Intr Status",    FPGA_NIO_POE_INTR_STAT_REG, READ_ONLY,  {4}, MASK_32, 0},
    {"NI/O POE Intr Mask",      FPGA_NIO_POE_INTR_MASK_REG, READ_WRITE, {4}, MASK_32, 0},
    {"NI/O Force Interrupt",    FPGA_NIO_POE_INTR_FORCE_REG,READ_WRITE, {4}, MASK_32, 0},
    {"END",                     0x000,                      0,          {0}, MASK_32, 0},
};

static reg_info_t katar_fpga_io_regs[] = {
    {"IO Reset Control",        FPGA_IO_RESET_CTRL_REG,     READ_ONLY,  {4}, MASK_32, 0},
    {"IO SFP Status",   		FPGA_IO_SFP_STATUS_REG,     READ_WRITE, {4}, MASK_32, 0},
    {"IO SFP Intr Status",    	FPGA_IO_SFP_INTR_STAT_REG, 	READ_ONLY,  {4}, MASK_32, 0},
    {"IO SFP Intr Mask",      	FPGA_IO_SFP_INTR_MASK_REG, 	READ_WRITE, {4}, MASK_32, 0},
    {"IO Force Interrupt",    	FPGA_IO_SFP_INTR_FORCE_REG,	READ_WRITE, {4}, MASK_32, 0},
    {"END",                     0x000,                      0,          {0}, MASK_32, 0},
};

/*-------------------------------------------------------------------
 *
 * Function : display_fpga_regs (int dummy)
 * Description: display fpga resiter
 * INPUT:  dummy not used.
 * OUTPUT: PASSED or FAILED
 * -------------------------------------------------------------------
*/
int
display_fpga_regs (int dummy)
{

    FILE *fp;
    unsigned int i, tmp;
    unsigned long fpga,io,nio;
    unsigned long offset;
    reg_info_t *ptr;
    
    fpga = get_platform_reg_base();
	io = get_platform_io_reg_base();
	nio = get_platform_nio_reg_base();

    if ((fp = fopen("fpga_regs.txt", "w"))==NULL) {
        printf("unable to open file");
        exit(0);
    }

	if(fpga != 0)
	{
	    fprintf(fp, "------FPGA Registers:------\n");
		printf("------FPGA Registers:------\n");
	    ptr = (reg_info_t *)&katar_fpga_regs;
	    
	    for (i = 0; ptr->size.size != 0; i+=4, ptr++) {
	        offset = ptr->offset;
	        tmp = *((unsigned int *)(fpga +  offset));
	        //        tmp = byteswap32(tmp);
	        fprintf(fp, "%-25s: @0x%04lx=%#x\n", ptr->name, offset , tmp);
	        printf("%-25s: @0x%04lx=%#x\n", ptr->name, offset, tmp);
	    }
	}

    if(nio != 0)
    {
        fprintf(fp, "------Non I/O Registers:------\n");
		printf("------Non I/O Registers:------\n");
        ptr = (reg_info_t *)&katar_fpga_nio_regs;

        for (i = 0; ptr->size.size != 0; i+=4, ptr++) {
            offset = ptr->offset;
            tmp = *((unsigned int *)(nio +  offset));
            //        tmp = byteswap32(tmp);
            fprintf(fp, "%-25s: @0x%04lx=%#x\n", ptr->name, offset , tmp);
            printf("%-25s: @0x%04lx=%#x\n", ptr->name, offset, tmp);
        }
    }

    if(io != 0)
    {
        fprintf(fp, "------I/O Registers:------\n");
		printf("------I/O Registers:------\n");
        ptr = (reg_info_t *)&katar_fpga_io_regs;

        for (i = 0; ptr->size.size != 0; i+=4, ptr++) {
            offset = ptr->offset;
            tmp = *((unsigned int *)(io +  offset));
            //        tmp = byteswap32(tmp);
            fprintf(fp, "%-25s: @0x%04lx=%#x\n", ptr->name, offset , tmp);
            printf("%-25s: @0x%04lx=%#x\n", ptr->name, offset, tmp);
        }
    }


	fclose(fp);
	printf("output saved to file fpga_regs.txt\n");

	//To clear reg stuats
	clear_fpga_status();  
 
    return(PASSED);
}

/*******************************************************************************
 *
 * Function   : katar_is_sfp_present
 * Description: Function to see if SFP module is present.
 *              FPGA_IO_SFP_STATUS_REG(0x004).
 * Inputs     : portnum -port number
 * Outputs    : TRUE/FALSE ; -1 on error
 *
 *******************************************************************************
 */
int katar_is_sfp_present (int portnum)
{
	unsigned long addr = get_platform_io_reg_base();
    katar_io_lvl_t *ioreg;
	uint32_t offset = 0,mask = 0;
	uint32_t present = -1;

	switch(portnum)
	{
		case 0:
			offset = OFFSET_SFP_P0_PRESENT;
			mask = MASK_SFP_P0_PRESENT;
			break;
		case 1:
			offset = OFFSET_SFP_P1_PRESENT;
			mask = MASK_SFP_P1_PRESENT;
			break;
		default:
			return (present);
			break;
	}
	if(addr!=0)
	{
		ioreg = (katar_io_lvl_t *)addr;
		present = ((ioreg->sfp_stat&mask)>>offset);
	}
    return (present);
}

/*
 *------------------------------------------------------------------
 * $Log: platform_fpga.c,v $
 * Revision 1.2  2019/06/14 05:24:50  mikech2
 * Collapse katar-branch00 to Main Trunk
 *
 * Revision 1.1.2.4  2019/03/05 07:29:37  mikech2
 * Clean up codes
 *
 * Revision 1.1.2.3  2019/02/20 02:54:48  mikech2
 * Add SFP present test in SPF intr test
 *
 * Revision 1.1.2.2  2019/02/12 08:06:29  mikech2
 * rename katar_*.h files
 *
 * Revision 1.1.2.1  2019/01/29 01:54:21  mikech2
 * rename katar_* files
 *
 * Revision 1.1.2.13  2019/01/18 03:42:50  mikech2
 * Add fan_intr_mask in interrupt mask control
 *
 * Revision 1.1.2.12  2019/01/17 07:14:19  mikech2
 * Modify according to Kwok's review comments
 *
 * Revision 1.1.2.11  2018/12/25 02:06:11  mikech2
 * Add read back option for Usrlogic FPGA write function
 *
 * Revision 1.1.2.10  2018/12/20 09:10:57  peteteng
 * Add FPGA I2C read/write/scan/reset util
 *
 * Revision 1.1.2.9  2018/12/12 09:06:16  mikech2
 * Update FPGA utility according to SPEC2.2(FW ver:2018121214)
 *
 * Revision 1.1.2.8  2018/11/22 06:55:08  mikech2
 * Add security FPGA version info
 *
 * Revision 1.1.2.7  2018/11/22 02:50:49  peteteng
 * Add Aikido register read/write utility
 *
 * Revision 1.1.2.6  2018/11/15 07:18:25  mikech2
 * Add R/W non I/O FPGA registers
 *
 * Revision 1.1.2.5  2018/11/01 09:19:42  mikech2
 * Update FPGA poe led control according to SPEC
 *
 * Revision 1.1.2.4  2018/11/01 08:55:02  mikech2
 * Disable boot timer when enter diag
 *
 * Revision 1.1.2.3  2018/11/01 07:24:23  mikech2
 * Change log print from *** to ---
 *
 * Revision 1.1.2.2  2018/10/26 02:39:34  mikech2
 * Fix typo
 *
 * Revision 1.1.2.1  2018/10/22 08:02:24  mikech2
 * Move project folder to common/src/katar/x86
 *
 * Revision 1.1.2.13  2018/10/11 06:49:04  mikech2
 * Update FPGA intr test for SERIRQ
 *
 * Revision 1.1.2.12  2018/10/02 02:32:15  mikech2
 * Modify FPGA register according SPEC 1.7.1
 *
 * Revision 1.1.2.11  2018/09/14 06:11:53  mikech2
 * Add mem info to system info
 *
 * Revision 1.1.2.10  2018/09/12 08:32:48  mikech2
 * Fix userlogic FPGA update & system info version issue
 *
 * Revision 1.1.2.9  2018/09/07 02:16:52  mikech2
 * Fix FPGA util issue
 *
 * Revision 1.1.2.8  2018/07/24 09:54:12  peteteng
 * Add SFP cookie - read
 *
 * Revision 1.1.2.7  2018/07/19 06:32:03  mikech2
 * modify logic FPGA upgrade flow
 *
 * Revision 1.1.2.6  2018/06/29 06:48:11  mikech2
 * Add spi boot control function
 *
 * Revision 1.1.2.5  2018/06/28 03:32:56  mikech2
 * Add interrupt mask control menu
 *
 * Revision 1.1.2.4  2018/06/27 01:26:29  mikech2
 * Add reset/unreset device menu
 *
 * Revision 1.1.2.3  2018/06/25 08:24:53  mikech2
 * Add interupt test menu
 *
 * Revision 1.1.2.2  2018/06/21 08:24:09  mikech2
 * remove unused menu, add scratchpad reg test
 *
 * Revision 1.1.2.1  2018/06/20 07:31:02  mikech2
 * Add fan/led/margin control menu
 *
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
