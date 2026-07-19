 /* $Id: diag_dsl_util.c,v 1.2 2018/08/06 02:31:50 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/diag_dsl_util.c,v $
 *------------------------------------------------------------------
 * diag_dsl_util.c 
 * 
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "common.h"
#include "types.h"
#include "queryflags.h"
#include "strings.h"
#include "diag_dsl_mib_defs.h"
#include "diag_dsl_libs.h"
#include "common_utils.h"
#include "diag_xdsl_test.h"
#include "diag_fpga.h"
#include "diag_nc_lib.h"
#include "diag_dsl_util.h"


/*******************************************************************************
 *                              Global Variables                               *
 *******************************************************************************
 */
extern uint bcm_op_mode;
extern uint bcm_line_id;
extern uint bcm_qln_monitor_time;
extern uint bcm_qln_monitor_freq;

int bcm63168_led_utils(int opt);
int xdsl_bcm63168_led_utils(int opt);
int marvell_1512_reset(void);


/*******************************************************************************
 *
 * Function: bcm63168_led_utils
 *
 * Description: Issues LED utils command to the bcm63168 processor
 *
 * Input : option for LED OFF/ LED CD ON / LED Data ON 
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
int bcm63168_led_utils (int opt)
{
    char tx_str[VIPER_NC_MAX_STR_SIZE];



    viper_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);
 
    switch (opt) {
        case DSL_LED_OFF :
            viper_nc_dispatch_comm(DIAG_BCM63268_LED_OFF, tx_str);
            break;
        case DSL_LED_CD_ON :
            viper_nc_dispatch_comm(DIAG_BCM63268_LED_CD_ON, tx_str);
            break;
        case DSL_LED_DATA_ON :
            viper_nc_dispatch_comm(DIAG_BCM63268_LED_DATA_ON, tx_str);
            break;
        default:
            printf("No Option Available\n");
            return (FAILED); 
    }


    if (viper_nc_dispatch_comm_is_ok() != PASSED) {
        printf("%s fail\n", __FUNCTION__);
        return (FAILED);
    }

    return (PASSED);
}


/******************************************************************************* *
 * Function   : xdsl_bcm63168_led_utils
 *
 * Description: The function to perform Link LEDs utils 
 *
 * Inputs     : option: dummy
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int xdsl_bcm63168_led_utils (int opt)
{
    int retval = FAILED;
    char test_mode;


    printf("xDSL LED Utility\n");
    
    /* Check xdsl is ready */
    if (wrap_pri_intf_rdy_chk() != PASSED) {
        printf ("\nxdsl module is not ready."
                "Please boot up xdsl module first.\n");
        return (PASSED);
    }

    while (1) {
        printf("xDSL LED Supported Mode:\n");
        printf("[0] Turn off.\n");
        printf("[1] Turn LED CD on.\n");
        printf("[2] Turn LED DATA on.\n");
        printf("[f] Leave the utility.\n");
        test_mode = gethex_answer("Enter LED mode: ", 0, 0, 0xf);

        if (test_mode == 0xF) {
            return (PASSED);
        }
    
        if ((retval = bcm63168_led_utils(test_mode)) != PASSED) {
            printf("%s failed.", __FUNCTION__);
            return (retval);
        }   
    }
    
    return (retval);
}

/*******************************************************************************
 *
 * Function    : marvell_1512_reset
 * Description : This utility performs a reset to the 1512
 * Input       : NONE
 * Output      : PASSED/FAILED
 *
 *******************************************************************************
 */
int marvell_1512_reset (void)
{   
    /* DSL SKU does the dsl module initialization reset sequence */
    /* FGPA 0x804 bit 24 */
    if (fpga_reset_api(FPGA_EXTER_DEV_RST_REG, FPGA_1512_RESET, TRUE,
                          WAITTIME_20_MS)
                          == FAILED) {
        return (FAILED);
    }
    /* DSL SKU un-reset the dsl module */
    if (fpga_reset_api(FPGA_EXTER_DEV_RST_REG, FPGA_1512_RESET, FALSE,
                          WAITTIME_20_MS)
                          == FAILED) {
        return (FAILED);
    }

    return (PASSED);
}



/******** History ********
$Log: diag_dsl_util.c,v $
Revision 1.2  2018/08/06 02:31:50  harrchan
Merge viper E2E to the main trunk (CSCvk28469)

Revision 1.1.2.2  2018/06/25 07:02:35  olin2
Remove Viper-Intel P0 DSL support

Revision 1.1.2.1  2018/05/21 08:42:35  olin2
Support DSL LED on/off utility






$Endlog$
*/
