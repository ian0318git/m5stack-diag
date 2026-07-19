/* $Id: dash_get_addr.c,v 1.13 2019/08/06 06:56:09 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/dash_get_addr.c,v $
 *------------------------------------------------------------------
 *
 * Filename: dash_get_addr.c
 * mcharon
 * Description: functions to return addresses for various fpga components
 * Copyright (c) 2012-2019 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <endian.h>
#include <stdint.h>
#include "defs.h"
#include "dash_fpga.h"
#include "i2c_address.h"  /* need this because of device address */

/*-------------------------------------------------------------------
 *
 * Function: get_fpga_addr
 * Description: get fpga address
 *
 * Input: NONE
 *
 * Output: address
 *
 *-------------------------------------------------------------------
 */
unsigned long
get_fpga_addr (void)
{
    unsigned long addr;
    assert(dash_fpga);
    addr = (unsigned long)dash_fpga;
                
    return addr;
   
}

/*-------------------------------------------------------------------
 *
 * Function: get_cpld_addr
 * Description: get cpld address
 *
 * Input: NONE
 *
 * Output: address
 *
 *-------------------------------------------------------------------
 */
unsigned long
get_cpld_addr (void)
{
    unsigned long addr;
    assert(dash_cpld);
    addr = (unsigned long)dash_cpld;
    return addr;
   
}

/*-------------------------------------------------------------------
 *
 * Function: get_platform_i2c_addr
 * Description: get i2c address
 *
 * Input: NONE
 *
 * Output: address
 *
 *-------------------------------------------------------------------
 */
unsigned long
get_platform_i2c_addr (int ctrl)
{
    unsigned long addr;
    assert(dash_fpga);
    
    addr = ((unsigned long)dash_fpga) + FPGA_I2C_BASE +
        (ctrl * FPGA_I2C_OFFSET);
                
    return addr;
   
}

/*-------------------------------------------------------------------
 *
 * Function: get_platform_mbx_addr 
 * Description: get mailbox address
 *
 * Input: NONE
 *
 * Output: address
 *
 *-------------------------------------------------------------------
 */
unsigned long
get_platform_mbx_addr (void)
{
    return (dash_fpga + MBX_OFFSET);
}

/*-------------------------------------------------------------------
 *
 * Function: get_platform_uart_addr
 * Description: get uart adddress
 *
 * Input: NONE
 *
 * Output: address
 *
 *-------------------------------------------------------------------
 */
unsigned long
get_platform_uart_addr (int i)
{
    return (dash_fpga + (FPGA_UART_BASE) + (i * FPGA_UART_OFFSET));
}

/*-------------------------------------------------------------------
 *
 * Function: get_platform_intr_ctrl_addr
 * Description: get intr ctrl address
 *
 * Input: NONE
 *
 * Output: address
 *
 *-------------------------------------------------------------------
 */
unsigned long
get_platform_intr_ctrl_addr (int plane)
{
    unsigned long addr = 0;
    unsigned long offset;
    assert(dash_fpga);
    switch (plane) {
    case FP:
        offset = FPGA_FP_INTR_CTRL_REG_OFFSET;
        break;
    case CP:
        offset = FPGA_CP_INTR_CTRL_REG_OFFSET;
        break;
    case NIOS:
        offset = FPGA_NIOS_INTR_CTRL_REG_OFFSET;
        break;
    default:
        assert(!"invalid backplane type");
    }
    addr = ((unsigned long)dash_fpga) + offset;

    return addr;
   
}

/*-------------------------------------------------------------------
 *
 * Function: get_led_ctrl_base
 * Description: get led address
 * 0x400 LED control register
 * Input: NONE
 *
 * Output: address
 *
 *-------------------------------------------------------------------
 */
unsigned long
get_platform_led_ctrl_base (void)
{
    unsigned long addr = 0;
    assert(dash_fpga);
    addr = ((unsigned long)dash_fpga) + LED_CONTROL_OFFSET;
    return addr;
}

/*-------------------------------------------------------------------
 *
 * Function: get_sys_low_level_base
 * Description: get system low level address
 * 0x000 system low level register
 * Input: NONE
 *
 * Output: address
 *
 *-------------------------------------------------------------------
 */
unsigned long
get_sys_low_level_base (void)
{
    unsigned long addr = 0;
    assert(dash_fpga);
    addr = ((unsigned long)dash_fpga) + SYS_LOW_LEVEL_OFFSET;
    return addr;

}

/*-------------------------------------------------------------------
 *
 * Function: get_platform_netclk_ptpconfig_base
 * Description: Network Clock and PTP Config address
 * 0x10100 base for above registers
 * Input: None
 *
 * Output: address
 *
 *-------------------------------------------------------------------
 */
unsigned long
get_platform_net_clk_ptp_conf_base (void)
{
    unsigned long addr = 0;
    assert(dash_fpga);
    addr = ((unsigned long)dash_fpga) + NET_CLK_PTP_CONF_REG_OFF;
    return addr;

}

/*-------------------------------------------------------------------
 *
 * Function: get_platform_uart_mux_addr
 * Description: get uart mux address
 * 0x900 UART multiplexer control register 
 * Input: plane, not used
 *
 * Output: address
 *
 *-------------------------------------------------------------------
 */
unsigned long
get_platform_uart_mux_addr (int plane)
{   
    assert(dash_fpga);
    return ((unsigned long)dash_fpga + UART_MUX_CONTROL_OFFSET);
}

/*-------------------------------------------------------------------
 *
 * Function: get_platform_sfp_stat_ctrl_addr 
 * Description: get sfp stat ctrl addr
 * 0x10000 SFP status and control register 
 * Input: NONE
 *
 * Output: address
 *
 *-------------------------------------------------------------------
 */
unsigned long
get_platform_sfp_stat_ctrl_addr (int plane)
{   
    assert(dash_fpga);
    return ((unsigned long)dash_fpga + SFP_STATUS_CONTROL_OFFSET);
}

/*-------------------------------------------------------------------
 *
 * Function: get_platform_prom_addr
 * Description: get prom address
 * 0x31800 FPGA configuration SPI PROM programming register
 * Input: NONE
 *
 * Output: address
 *
 *-------------------------------------------------------------------
 */
unsigned long
get_platform_prom_addr (void)
{
    assert(dash_fpga);
    //    printf("get_platform_prom_base: %lx\n", dash_fpga + FPGA_CP_SPI_PROM_OFFSET);
    return ((unsigned long)dash_fpga + FPGA_CP_SPI_PROM_OFFSET);
}

/*-------------------------------------------------------------------
 *
 * Function: get_platform_aikido_addr
 * Description: get aikido address
 * 0x31A00 FPGA configuration SPI PROM programming register
 * Input: NONE
 *
 * Output: address
 *
 *-------------------------------------------------------------------
 */
unsigned long
get_platform_aikido_addr (void)
{
    assert(dash_fpga);
    return ((unsigned long)dash_fpga + FPGA_AIKIDO_SPI_MASTER_OFFSET); 
}

/*-------------------------------------------------------------------
 *
 * Function: get_platform_nios_prom_addr
 * Description: get nios prom address
 *
 * Input: NONE
 *
 * Output: address
 *
 *-------------------------------------------------------------------
 */
unsigned long
get_platform_nios_prom_addr (void)
{
    assert(dash_fpga);
    return ((unsigned long)dash_fpga + FPGA_NIOS_SPI_PROM_OFFSET);
}


/*-------------------------------------------------------------------
 *
 * Function: get_platform_ps_env_base
 * Description: get power supply environment addre
 * 0x32100 Power supply and Environmental Register
 *
 * Input: NONE
 *
 * Output: address
 *
 *-------------------------------------------------------------------
 */
unsigned long
get_platform_ps_env_base (void)
{
    unsigned long addr = 0;
    assert(dash_fpga);

    addr = ((unsigned long)dash_fpga) + FPGA_PS_ENV_OFFSET;
#if 0
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("get_platform_ps_env_base: base %p + %#x\n",
               (void *)addr, FPGA_PS_ENV_OFFSET);
    }
#endif

    return addr;
}

/*-------------------------------------------------------------------
 *
 * Function: get_platform_env_fan_base
 * Description: get Environmental Fan Control Register 0x32200
 *              ( Utah, Sword and Dagger only)
 * Input: NONE
 *
 * Output: address
 *
 *-------------------------------------------------------------------
 */
unsigned long
get_platform_env_fan_base (void)
{
    unsigned long addr = 0;
    assert(dash_fpga);

    addr = ((unsigned long)dash_fpga) + FPGA_ENV_FAN_OFFSET;
#if 0
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("get_platform_env_fan_base: base %p + %#x\n",
               (void *)addr, FPGA_ENV_FAN_OFFSET);
    }
#endif

    return addr;
}

/*-------------------------------------------------------------------
 *
 * Function: get_platform_env_mcu_base
 * Description: get platform env mcu addr
 * 0x33000 Environmental MCU download control register
 * Input: NONE
 *
 * Output: address
 *
 *-------------------------------------------------------------------
 */
unsigned long
get_platform_env_mcu_base (int plane)
{
    unsigned long addr = 0;
    assert(dash_fpga);
    addr = ((unsigned long)dash_fpga) + FPGA_ENV_MCU_OFFSET;
    /*
    printf("get_platform_env_mcu_base: base %p + %#x\n",
           (void *)addr, FPGA_ENV_MCU_OFFSET);
    */
    return addr;

}

/*-------------------------------------------------------------------
 *
 * Function: get_platform_vm_base
 * Description: get platform vm addr
 *
 * Input: plane, not used
 *
 * Output: address
 *
 *-------------------------------------------------------------------
 */
unsigned long
get_platform_vm_base (int plane)
{
    unsigned long addr = 0;
    assert(dash_fpga);
    addr = ((unsigned long)dash_fpga) + FPGA_VOL_MON_OFFSET;

    return addr;

}

/*-------------------------------------------------------------------
 *
 * Function: get_platform_multiboot_base
 * Description: get mutipboot address (0x2_2000)
 *
 * Input: NONE
 *
 * Output: address
 *
 *-------------------------------------------------------------------
 */
unsigned long
get_platform_multiboot_base (void)
{
    unsigned long addr = 0;
    assert(dash_fpga);
    addr = ((unsigned long)dash_fpga) + FPGA_HEADER_OFFSET;

    return addr;

}

/*------------------------------------------------------------------
$Log: dash_get_addr.c,v $
Revision 1.13  2019/08/06 06:56:09  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.12.2.4  2018/10/16 09:03:01  meho
Pluggable re-structured.

Revision 1.12.2.3  2018/09/03 08:58:26  alpeng
able to get aikido spi regsiter on fpga

Revision 1.12.2.2  2018/08/24 19:22:54  meho
Added dash fpga r/w function

Revision 1.12.2.1  2018/08/02 09:12:04  meho
Added pluggable portion in DASH FPGA

Revision 1.12  2018/05/18 09:24:51  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.11.64.1  2016/10/20 01:51:33  leschen
Modify to use scratchpad reg to do FPGA register test

Revision 1.11  2013/08/05 10:13:56  alpeng
support fan utility on Utah

Revision 1.10  2012/12/24 10:04:13  srane
Add finction to get Network Clock and PTP Config address.

Revision 1.9  2012/09/18 19:19:55  mcharon
support poll slot-fix io intr test; support fpga upgrade; support serial irq intr tst

Revision 1.8  2012/06/04 10:35:08  palin2
Clean up compiler warnings.

Revision 1.7  2012/05/23 09:20:08  alpeng
support is_sfp_present function for SFP test

Revision 1.6  2012/05/02 02:05:28  mcharon
add config header support

Revision 1.5  2012/04/17 23:39:39  ptong
Remove #include nvmonvars.h

Revision 1.4  2012/04/17 14:13:58  palin2
Add 12V PoE PSU cookie utility support.

Revision 1.3  2012/04/16 15:29:18  palin2
Update 12V PoE PSU tests and utilities based on HW team's request:
1) Add "Registers test" support.
2) Add "PoE PSU" info into bootlog message.
3) Add utility to verified FPGA related PoE PSU detect function.

Revision 1.2  2012/03/28 00:38:17  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:06  ptong
Initial archive of ng_diag module


$Endlog$
*/
