/* $Id: diag_dsl_util.c,v 1.3 2019/05/21 07:44:19 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/diag_dsl_util.c,v $
 *------------------------------------------------------------------
 * 
 * diag_dsl_util.c
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include "common.h"
#include "types.h"
#include "menu.h"
#include "defs.h"
#include "error.h"
#include "strings.h"
#include "nvmonvars.h"
#include "common_utils.h"
#include "queryflags.h"
#include "linux_ntwk.h" /* tftp_get */
#include "diag_moka_fpga_lib.h"
#include "diag_uart_lib.h"
#include "diag_enhance_err_msg_lib.h"
#include "platform_cookie.h"
#include "diag_temp_sensor_util.h"
#include "diag_dsl_lib.h"
#include "diag_dsl_test.h"
#include "diag_dsl_util.h"

uint bcm_op_mode;
uint bcm_line_id;
uint bcm_qln_monitor_time;
uint bcm_qln_monitor_freq;
/*******************************************************************************
 *
 * Function    : xdsl_util_bcm63168_reset
 * Description : This utility performs a reset to the BCM63168 on the xDSL module
 * Input       : NONE
 * Output      : PASSED/FAILED
 *
 *******************************************************************************
 */
int xdsl_util_bcm63168_reset (void)
{
    /* DSL SKU does the dsl module initialization reset sequence */
    if (fpga_reset_32_api(FPGA_EXTER_DEV_RST_REG, EXT_DSL_CHIP_RESET, TRUE,
                          WAITTIME_20_MS)
        == FAILED) {
    	return (FAILED);
    }
    /* DSL SKU un-reset the dsl module */
    if (fpga_reset_32_api(FPGA_EXTER_DEV_RST_REG, EXT_DSL_CHIP_RESET, FALSE,
                          WAITTIME_20_MS)
        == FAILED) {
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : xdsl_util_init_bcm63168
 *
 * Description: The functions to init bcm63168.
 *
 * Inputs     : NONE 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int xdsl_util_init_bcm63168 (void)
{
    int retval = FAILED;
    
    if ((retval = bcm63168_initialize()) != PASSED) {
        printf("%s fail\n", __FUNCTION__);
        return (retval);
    }
    
    return (retval);
}

/*******************************************************************************
 *
 * Function   : xdsl_util_config_bcm63168
 *
 * Description: The function to configure DSL profile
 *
 * Inputs     : NONE 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int xdsl_util_config_bcm63168 (void)
{
    int retval = FAILED;
    printf("Fix later.\n");    
    if ((retval = bcm63168_configure()) != PASSED) {
        printf("%s fail\n", __FUNCTION__);
        return (retval);
    }
    
    return (retval);
}

/*******************************************************************************
 *
 * Function   : xdsl_util_get_bcm63168_config
 *
 * Description: The functions to get DSL profile.
 *
 * Inputs     : NONE 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int xdsl_util_get_bcm63168_config (void)
{
    int retval = FAILED;
    
    if ((retval = bcm63168_get_configure()) != PASSED) {
        printf("%s fail\n", __FUNCTION__);
        return (retval);
    }
    
    return (retval);
}

/*******************************************************************************
 *
 * Function   : xdsl_util_get_bcm63168_version
 *
 * Description: The functions to get DSL driver version.
 *
 * Inputs     : NONE 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int xdsl_util_get_bcm63168_version (void)
{
    int retval = FAILED;
    
    if ((retval = bcm63168_get_version()) != PASSED) {
        printf("%s fail\n", __FUNCTION__);
        return (retval);
    }
    
    return (retval);
}

/*******************************************************************************
 *
 * Function   : xdsl_util_connection_start
 *
 * Description: The functions to start to train with DSLAM.
 *
 * Inputs     : NONE 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int xdsl_util_connection_start (void)
{
    int retval = FAILED;
    
    if ((retval = bcm63168_do_showtime()) != PASSED) {
        printf("%s fail\n", __FUNCTION__);
        return (retval);
    }
    
    return (retval);
}

/*******************************************************************************
 *
 * Function   : xdsl_util_connection_stop
 *
 * Description: The functions to stop DSL from showtime.
 *
 * Inputs     : NONE 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int xdsl_util_connection_stop (void)
{
    int retval = FAILED;

    if ((retval = bcm63168_connection_stop()) != PASSED) {
        printf("%s fail\n", __FUNCTION__);
        return (retval);
    }
    
    return (retval);
}

/*******************************************************************************
 *
 * Function   : xdsl_util_get_xdsl_mib_info
 *
 * Description: The functions to get DSL inforamtion.
 *
 * Inputs     : NONE 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int xdsl_util_get_xdsl_mib_info (void)
{
    int retval = FAILED;
    
    if ((retval = bcm63168_get_adslmib_info()) != PASSED) {
        printf("%s fail\n", __FUNCTION__);
        return (retval);
    }
    
    return (retval);
}

/*******************************************************************************
 *
 * Function   : xdsl_util_get_xtm_bonding_info
 *
 * Description: The functions to get DSL bonding status.
 *
 * Inputs     : NONE 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int xdsl_util_get_xtm_bonding_info (void)
{
    int retval = FAILED;
    printf("platform doesn't have bonding.\n");    
    return (retval);
    if ((retval = bcm63168_get_xtm_bonding_info()) != PASSED) {
        printf("%s fail\n", __FUNCTION__);
        return (retval);
    }
    
    return (retval);
}

/*******************************************************************************
 *
 * Function   : xdsl_util_get_xdsl_info
 *
 * Description: The functions to get DSL inforamtion.
 *
 * Inputs     : NONE 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int xdsl_util_get_xdsl_info (void)
{
    int retval = FAILED;
    
    if ((retval = bcm63168_get_xdsl_info()) != PASSED) {
        printf("%s fail\n", __FUNCTION__);
        return (retval);
    }
    
    return (retval);
}

/*******************************************************************************
 *
 * Function   : xdsl_util_get_connection_info
 *
 * Description: The function to get connection information.
 *
 * Inputs     : NONE 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int xdsl_util_get_connection_info (void)
{
    int retval = FAILED;
    
    if ((retval = bcm63168_get_conn_info()) != PASSED) {
        printf("%s fail\n", __FUNCTION__);
        return (retval);
    }
    
    return (retval);
}

/*******************************************************************************
 *
 * Function   : xdsl_util_bcm63168_led_test
 *
 * Description: The function to perform Link LEDs test
 *
 * Inputs     : NONE 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int xdsl_util_bcm63168_led_test (void)
{
    uchar mb_get_loc[FRU_SIZE] = {0};
    uchar mb_get_pid[FRU_SIZE] = {0};
#ifdef ENHANCE_ERROR_MSG_RDY
    /*
     * 1. Subtests of the test function will reuse all variables
     * 2. All variables will be cleared automatically when
     *    entering and leaving each menu item.
     */
    /* Segment 1: PID | Unique_string : slot_info */
    fru_table_offset = MB;
    /* fru_table_offset should be set, otherwise, it will not */
    /* go to enhanced error message format in cterr() */
    /* set fru_table_offset to get the predefine value */
    /* or change mb_pid & mb_loc below */
    platform_get_pid((char *)mb_get_pid);
    strcpy((char *)mb_get_loc, "MB");
    platform_fru_table[fru_table_offset].pid_string = mb_get_pid;
    platform_fru_table[fru_table_offset].location_string = mb_get_loc;

    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("Marvell Armada 7040", "BCM63xxx", "LED");

    /* Segment 5: register and memory dump */
    //cterr_add_reg_dump((PFV)bcm63168_get_xdsl_profile);

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)show_plat_curr_temps);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Check the LED GPIO to see if its' implementation "
                    "is identical to BCM63xxx specification."
                    "and the failed I2C devices.",
                    "If step a is OK, check the LED interface "
                    "between BCM.");
#endif

    int retval = FAILED;
    char *tname = "xDSL LED";

    testname(tname);
    prpass(testpass, "%s, ", tname);

    /* Check xdsl is ready */
    if (pri_intf_rdy_chk() != PASSED) {
        printf ("\nxdsl module is not ready."
                "Please boot up xdsl module first.\n");
        return (PASSED);
    }
    
    if ((retval = bcm63168_led_test()) != PASSED) {
        cterr('f', 0, "%s failed.", __FUNCTION__);
        return (retval);
    }
    
    prcomplete(testpass, errcount, (char *)0);
    return (retval);
}

/*******************************************************************************
 *
 * Function   : xdsl_util_bcm63168_show_profile
 *
 * Description: The function to show DSL profile
 *
 * Inputs     : NONE 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int xdsl_util_bcm63168_show_profile (void)
{
    int retval = FAILED;
    
    if ((retval = bcm63168_show_profile()) != PASSED) {
        printf("%s fail\n", __FUNCTION__);
        return (retval);
    }
    
    return (retval);
}

/*******************************************************************************
 *
 * Function   : xdsl_util_ping
 *
 * Description: The function is going to ping DSL module.
 *
 * Inputs     : NONE
 *
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int xdsl_util_ping (void)
{
    char tx_str[PLAT_NC_MAX_STR_SIZE];

    plat_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    plat_nc_dispatch_comm(DIAG_PING_BCM63268, tx_str);

    if (plat_nc_dispatch_comm_is_ok() != PASSED) {
        printf("%s fail\n", __FUNCTION__);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function: xdsl_util_bcm63168_show_spi_flash_reg
 *
 * Description: This function displays xdsl SPI flash registers.
 *
 * Input : None
 *
 * Output: PASSED/FAILED.
 *
 *******************************************************************************
 */
int xdsl_util_bcm63168_show_spi_flash_reg (void)
{
    int retval = FAILED;

    if ((retval = bcm63168_show_spi_flash_reg()) != PASSED) {
        printf("%s fail\n", __FUNCTION__);
        return (retval);
    }

    return (retval);
}

/*******************************************************************************
 *
 * Function: xdsl_util_bcm63168_chk_spi_flash_protect
 *
 * Description: A utility to check the SPI flash protection
 *
 * Input : None
 *
 * Output: PASSED/FAILED.
 *
 *******************************************************************************
 */
int xdsl_util_bcm63168_chk_spi_flash_protect (void)
{
    int retval = FAILED;

    if ((retval = bcm63168_chk_spi_flash_protect()) != PASSED) {
        printf("%s failed to check xDSL SPI Flash protection\n", __FUNCTION__);
        return (retval);
    }

    return (retval);
}

/*******************************************************************************
 *
 * Function: xdsl_util_bcm63168_en_spi_flash_reg
 *
 * Description: This function enable xdsl SPI flash write protect registers.
 *
 * Input : None
 *
 * Output: PASSED/FAILED.
 *
 *******************************************************************************
 */
int xdsl_util_bcm63168_en_spi_flash_reg (void)
{
    int retval = FAILED;

    if ((retval =bcm63168_en_wp_spi_flash_reg()) != PASSED) {
        printf("%s fail\n", __FUNCTION__);
        return (retval);
    }

    return (retval);
}

/*******************************************************************************
 *
 * Function: xdsl_util_bcm63168_dis_spi_flash_reg
 *
 * Description: This function disable xdsl SPI flash write protect registers.
 *
 * Input : None
 *
 * Output: PASSED/FAILED.
 *
 *******************************************************************************
 */
int xdsl_util_bcm63168_dis_spi_flash_reg (void)
{
    int retval = FAILED;

    if ((retval =bcm63168_dis_wp_spi_flash_reg()) != PASSED) {
        printf("%s fail\n", __FUNCTION__);
        return (retval);
    }

    return (retval);
}

/*******************************************************************************
 *
 * Function: xdsl_util_restore_cfe_param 
 *
 * Description: This function restore cfe ios or diag param.
 *
 * Input : os_param TRUE = IOS
 *         os_param FALSE = DIAG 
 *
 * Output: PASSED/FAILED.
 *
 *******************************************************************************
 */
int xdsl_util_restore_cfe_param (boolean os_param)
{
    int retval = FAILED;
    
    /* Step1: Reset */
    xdsl_util_bcm63168_reset();
    
    if (os_param == IOS_CFE_PARAM) {
        /* Step2: Set CFE IPs and Filename */ 
        if (xdsl_cfe_parms_set(IOS_CFE_PARAM) != PASSED) {
            printf("%s fail\n", __FUNCTION__);
            return (retval);
        }

        /* Step3: Reset */
        xdsl_util_bcm63168_reset();
        if (xdsl_confirm_cfe_parms_set(IOS_CFE_PARAM) == PASSED) {
            printf("\nRestore CFE IOS Param Passed\n");
            retval = PASSED;
        } else  {
            printf("\nRestore CFE IOS Param Failed\n");
        }
    } else {
        /* Step2: Set CFE IPs and Filename */ 
        if (xdsl_cfe_parms_set(DIAG_CFE_PARAM) != PASSED) {
            printf("%s fail\n", __FUNCTION__);
            return (retval);
        }

        /* Step3: Reset */
        xdsl_util_bcm63168_reset();
        if (xdsl_confirm_cfe_parms_set(DIAG_CFE_PARAM) == PASSED) {
            printf("\nRestore CFE DIAG Param Passed\n");
            retval = PASSED;
        } else  {
            printf("\nRestore CFE DIAG Param Failed\n");
        }
    }

    return (retval);
}

/******************************************************************************* *
 * Function   : xdsl_util_bcm63168_led
 *
 * Description: The function to perform Link LEDs utils 
 *
 * Inputs     : option for off/carrier detect on /data on 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int xdsl_util_bcm63168_led (int opt)
{
    int retval = FAILED;
    char *tname = "xDSL LED Utility";

    testname(tname);
    prpass(testpass, "%s, ", tname);

    /* Check xdsl is ready */
    if (pri_intf_rdy_chk() != PASSED) {
        printf ("\nxdsl module is not ready."
                "Please boot up xdsl module first.\n");
        return (PASSED);
    }
    
    if ((retval = bcm63168_led_utils(opt)) != PASSED) {
        printf("%s led utils failed.\n", __FUNCTION__);
        return (retval);
    }
    
    prcomplete(testpass, errcount, (char *)0);
    return (retval);
}

/*-------------------------------------------------
 * $Log: diag_dsl_util.c,v $
 * Revision 1.3  2019/05/21 07:44:19  wilbhuan
 * Add a new xDSL utility to check the SPI Flash protection.
 *
 * Revision 1.2  2019/01/10 06:36:22  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
