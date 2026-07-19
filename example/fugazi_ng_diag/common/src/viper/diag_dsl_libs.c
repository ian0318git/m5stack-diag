 /* $Id: diag_dsl_libs.c,v 1.3 2018/09/21 02:48:54 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/diag_dsl_libs.c,v $
 *------------------------------------------------------------------
 * diag_dsl_libs.c 
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
#include "diag_common.h"


static char * vdslLineStateName[]=
{
    "BCM_xDSL_LINK_UP",
    "BCM_xDSL_LINK_DOWN",
    "BCM_xDSL_TRAINING_G992_MESSAGE_EXCHANGE",
    "BCM_xDSL_TRAINING_G992_CHANNEL_ANALYSIS",
    "BCM_xDSL_TRAINING_G992_STARTED",
    "BCM_xDSL_TRAINING_G993_MESSAGE_EXCHANGE",
    "BCM_xDSL_TRAINING_G993_CHANNEL_ANALYSIS",
    "BCM_xDSL_TRAINING_G993_STARTED",        
    "BCM_xDSL_TRAINING_G994",
    "BCM_xDSL_G994_NONSTDINFO_RECEIVED",
    "BCM_xDSL_BERT_COMPLETE",
    "BCM_xDSL_ATM_IDLE",
    "BCM_xDSL_EVENT",
    "BCM_xDSL_G997_FRAME_RECEIVED",
    "BCM_xDSL_G997_FRAME_SENT"
};


static char *bcm_adsl_line_state_str[]=
{
    "bcm_adsl_link_up",
    "bcm_adsl_link_down",
    "bcm_adsl_training_g992_exange",
    "bcm_adsl_training_g992_channel_analysis",
    "bcm_adsl_training_g992_started",
    "bcm_adsl_training_g993_exange",
    "bcm_adsl_training_g993_channel_analysis",
    "bcm_adsl_training_g993_started",
    "bcm_adsl_training_g994",
    "bcm_adsl_g994_nonstdinfo_received",
    "bcm_adsl_bert_complete",
    "bcm_adsl_atm_idle",
    "bcm_adsl_event",
    "bcm_adsl_g997_frame_received",
    "bcm_adsl_g997_frame_sent"
};

static bcm_id_string_t             bcm_mode_str[] = {
    {ANSI_T1_413_ID     ,(unsigned char *)"ANSI_T1_413"},    /* ANSI T1.413 1998 */
    {ITU_G_992_1A_ID    ,(unsigned char *)"ITU_G_992_1A"},   /* G.992.1, Annex A */
    {ITU_G_992_3A_ID    ,(unsigned char *)"ITU_G_992_3A"},   /* G.992.3, Annex A */
    {ITU_G_992_5A_ID    ,(unsigned char *)"ITU_G_992_5A"},   /* G.992.5, Annex A */
    {ITU_G_992_3L_ID    ,(unsigned char *)"ITU_G_992_3L"},   /* G.992.3, Annex L */
    {ITU_G_992_3M_ID    ,(unsigned char *)"ITU_G_992_3M"},   /* G.992.3, Annex M */
    {ITU_G_992_5M_ID    ,(unsigned char *)"ITU_G_992_5M"},   /* G.992.5, Annex M */
    {ITU_G_992_1B_ID    ,(unsigned char *)"ITU_G_992_1B"},   /* G.992.1, Annex B */
    {ITU_G_992_3B_ID    ,(unsigned char *)"ITU_G_992_3B"},   /* G.992.3, Annex B */
    {ITU_G_992_5B_ID    ,(unsigned char *)"ITU_G_992_5B"},   /* G.992.5, Annex B */
    {ITU_G_993_1_ID     ,(unsigned char *)"ITU_G_993_1 (VDSL1)"}, /* VDSL1 */
    {ITU_G_993_2_ID     ,(unsigned char *)"ITU_G_993_2 (VDSL2)"}, /* VDSL2 */
    {ITU_G_992_3J_ID    ,(unsigned char *)"ITU_G_992_3J"},   /* G.992.3, Annex J */
    {ITU_G_992_5J_ID    ,(unsigned char *)"ITU_G_992_5J"},   /* G.992.5, Annex J */
    {INVALID_MODE       ,(unsigned char *)"UNKNOWN"}
};

static ushort bcm_line_state_get(void);
static ushort bcm_bonding_state_get(void);
static int bcm_line_config_get(void);
static int bcm63168_self_disconnect(void);
static int bcm63168_sel_op_mode(void);
static int bcm63168_set_op_mode(void);
static int xdsl_relay_drv_pin_set (void);

boolean bcm_do_show_time_auto = TRUE;
ushort bcm_vdsl_op_mode = ITU_G_993_2_ID;
uchar bcm_showtime_duration = 2;  /* 2 seconds */
uchar *bcm_op_string = (uchar *)"UNKNOWN";
uint bcm_op_mode = 12;
uint bcm_idle_listen_params = 0;
uint bcm_channel_bonding = 0;
uint bcm_line_id = BCM_DSL_LINE_0;
uint bcm_qln_monitor_time = IDLE_LISTEN_TIME;
uint bcm_qln_monitor_freq = IDLE_LISTEN_FREQ;


/*******************************************************************************
 *
 * Function: clear
 *
 * Description: Waiting for user to put enter 
 *
 * Input : None
 *
 * Output: None
 *
 *******************************************************************************
 */
void clear (void)
{    
    while ( getchar() != '\n' );
}

/*******************************************************************************
 *
 * Function: bcm63168_check_sku_type
 *
 * Description: Check if SKU ID in xDSL module and in ACT2 are identical.
 *
 * Input : None
 *
 * Output: line state
 *
 *******************************************************************************
 */
int bcm63168_check_sku_type (void)
{
    int sku_id = 0;
    char tx_str[VIPER_NC_MAX_STR_SIZE];
    char rx_str[VIPER_NC_MAX_STR_SIZE];

    viper_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    viper_nc_dispatch_comm(DIAG_BCM63268_GET_SKU_TYPE, tx_str);

    
    if (viper_nc_dispatch_comm_is_ok() != PASSED) {
        printf("%s fail at nc dispatch command\n", __FUNCTION__);
        return (FAILED);
    }

    if (viper_nc_get_parms(1, rx_str) != PASSED) {
    	printf("Catch return parameter failed\n");
        return (FAILED);
    }

    sku_id = atoi(rx_str);

    if (viper_dsl_sku != (sku_id & DSL_SKU_MASK)) {
        printf("Check SKU type failed. Expect sku = 0x%x, Read BCM sku = 0x%x",
               viper_dsl_sku, (sku_id & 0x3));
        return (FAILED);
    }

    printf("Check SKU type. Expect sku = 0x%x, Read BCM sku = 0x%x",
           viper_dsl_sku, (sku_id & DSL_SKU_MASK));

    return (PASSED);
}

/*******************************************************************************
 *
 * Function: bcm_line_state_get
 *
 * Description: This function gets the line state from Broadcom 63168
 *
 * Input : none
 *
 * Output: line state
 *
 *******************************************************************************
 */
static ushort bcm_line_state_get(void)
{
    ushort line_state = 0;
    uint line_id = bcm_line_id;
    char tx_str[VIPER_NC_MAX_STR_SIZE];
    char rx_str[VIPER_NC_MAX_STR_SIZE];

    /* Due to line1 link status is meaningless when training single line.
     * Only check line0 link status for line0 and line1 that in use.
     */
    if (!bcm_channel_bonding) {
        line_id = BCM_DSL_LINE_0;
    }

    viper_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    viper_nc_dispatch_comm(DIAG_BCM63268_GET_CONN_INFO, tx_str);


    if (viper_nc_dispatch_comm_is_ok() != PASSED) {
        printf("%s fail at nc dispatch command\n", __FUNCTION__);
        return (FAILED);
    }

    if (viper_nc_get_parms(PARAMETER_NUM_1, rx_str) != PASSED) {
    	printf("Catch return parameter failed\n");
        return (FAILED);
    }

    while (bcm_adsl_line_state_str[line_state] != NULL) {
        if (strstr(rx_str, bcm_adsl_line_state_str[line_state])) {
            break;
        }
        line_state++;
    }

    printf("\n Line %d link state: %s \n",
    		bcm_line_id, vdslLineStateName[line_state]);
    return (line_state);
}

/*******************************************************************************
 *
 * Function: bcm_bonding_state_get
 *
 * Description: This function gets bonding state from Broadcom 63168
 *
 * Input : None.
 *
 * Output: line state
 *
 *******************************************************************************
 */
static ushort bcm_bonding_state_get(void)
{
    char tx_str[VIPER_NC_MAX_STR_SIZE];

    viper_nc_init_parms_file();
    
    /* to build up nc command 
     * first  bcm_op_mode
     * second bcm_line_id
     * thrid  bcm idle listen time
     * forth  bcm idle listen frquence*/

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    viper_nc_dispatch_comm(DIAG_BCM63268_GET_BONDING_STATE, tx_str);

    if (viper_nc_dispatch_comm_is_ok() != PASSED) {
        printf("%s fail at nc dispatch command\n", __FUNCTION__);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function: bcm_line_config_get
 *
 * Description: This function gets the configuration from Broadcom 63168
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int bcm_line_config_get(void)
{
    char tx_str[VIPER_NC_MAX_STR_SIZE];

    viper_nc_init_parms_file();

    /* to build up nc command 
     * first  bcm_op_mode
     * second bcm_line_id
     * thrid  bcm idle listen time
     * forth  bcm idle listen frquence*/

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    viper_nc_dispatch_comm(DIAG_BCM63268_GET_CONFIG_INFO, tx_str);

    if (viper_nc_dispatch_comm_is_ok() != PASSED) {
        printf("%s fail at nc dispatch command\n", __FUNCTION__);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function: bcm63168_self_disconnect
 *
 * Description: This function stops the showtime connection.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int bcm63168_self_disconnect(void)
{
    char ch;
    int ix;
    ushort line_state = BCM_ADSL_LINK_DOWN;
    ushort line_state0 = BCM_ADSL_LINK_DOWN;
    ushort line_state1 = BCM_ADSL_LINK_DOWN;

    if (bcm_do_show_time_auto == TRUE) {
        printf("Will self disconnect in %d seconds\n", bcm_showtime_duration);
        if (bcm63168_connection_stop()) {
            cterr('f', 0, "Failed to send connection stop command");
            return (FAILED);
        }
    
        for (ix = 0; ix < DSL_SELF_DISCONN_RETRY_TIMES; ix++) {
            msleep(WAIT_DSL_SELF_DISCONN);
            if (bcm_channel_bonding) {
                bcm_line_id = BCM_DSL_LINE_0;
                line_state0 = bcm_line_state_get();
                printf("\nLine%d state deactivate = 0x%08x\n",
                       bcm_line_id, line_state);

                bcm_line_id = BCM_DSL_LINE_1;
                line_state1 = bcm_line_state_get();
                printf("\nLine%d state deactivate = 0x%08x\n", 
                       bcm_line_id, line_state);
            } else {
                line_state = bcm_line_state_get();
                printf("\nLine state deactivate = 0x%08x\n", line_state);
            }

            if (bcm_channel_bonding) {
                if ((line_state0 == BCM_ADSL_LINK_DOWN) &&
                    (line_state1 == BCM_ADSL_LINK_DOWN)) {
                    break;
                }
            } else {
                if (line_state == BCM_ADSL_LINK_DOWN) {
                    break;
                }
            }
        }

        if (bcm_channel_bonding) {
            if ((line_state0 != BCM_ADSL_LINK_DOWN) ||
                (line_state1 != BCM_ADSL_LINK_DOWN)) {
                cterr('f', 0, "Fail to deactivate the line, line state0 = %d, line state1 = %d",
                              line_state0, line_state1);
                return (FAILED);
            }
        } else {
            if (line_state != BCM_ADSL_LINK_DOWN) {
                cterr('f', 0, "Fail to deactivate the line, line state = %d",
                              line_state);
                return (FAILED);
            }
        }
        return (PASSED);
       
    } else {

        if (bcm_channel_bonding) {
            bcm_line_id = BCM_DSL_LINE_0;
            if (bcm63168_showtime_no_retrain()) {
                return (FAILED);
            }

            bcm_line_id = BCM_DSL_LINE_1;
            if (bcm63168_showtime_no_retrain()) {
                return (FAILED);
            }
        } else {
            if (bcm63168_showtime_no_retrain()) {
                return (FAILED);
            }
        }

        for (ix = 0; ix < DSL_SELF_DISCONN_TIMEOUT; ix++) {
            msleep(DSL_WAIT_10MS);

            if ((ix % GET_LINE_DELAY) == 0) {
                if (bcm_channel_bonding) {
                    bcm_line_id = BCM_DSL_LINE_0;
                    line_state0 = bcm_line_state_get();
                    printf("\nLine state = 0x%08x\n", line_state0);

                    bcm_line_id = BCM_DSL_LINE_1;
                    line_state0 = bcm_line_state_get();
                    printf("\nLine state = 0x%08x\n", line_state1);
                } else {
                    line_state = bcm_line_state_get();
                    printf("\nLine state = 0x%08x\n", line_state);
                }
            }
            /* After 2 minutes if the state is not 0 then timeout,
              otherwise quit as soon as it goes showtime */
            if (bcm_channel_bonding) {
                if ((line_state0 == BCM_ADSL_LINK_UP) && 
                    (line_state1 == BCM_ADSL_LINK_UP)) {
                    break;
                }
            } else {
                if (line_state == BCM_ADSL_LINK_UP) {
                    break;
                }
            }
        }

        if (bcm_channel_bonding) {
            if ((line_state0 != BCM_ADSL_LINK_UP) || 
                (line_state1 != BCM_ADSL_LINK_UP)) {
                cterr('f', 0, "Fail to connect, line state0 = 0x%08x\n, "
                              "line state1 = 0x%08x\n",
                              line_state0, line_state1);
                return (FAILED);
            }
        } else {
            if (line_state != BCM_ADSL_LINK_UP) {
                cterr('f', 0, "Fail to connect, line%d state = 0x%08x\n",
                              bcm_line_id, line_state);
                return (FAILED);
            }
        }
    
        while (1) {
            printf("\nHit 'd' to deactivate the line or\n");
            printf("any other key for configurations\n");
            ch = getchar();
            clear();
            if (ch == 'd') {
                if (bcm63168_connection_stop()) {
                    cterr('f', 0, "Failed to send connection stop command");
                    return (FAILED);
                }
                return (PASSED);
            } else {
                if (bcm_channel_bonding) {
                    bcm_line_id = BCM_DSL_LINE_0;
                    if (bcm_line_config_get()) {
                        cterr('f', 0, "Failed to get line config");
                        return (FAILED);
                    }

                    bcm_line_id = BCM_DSL_LINE_1;
                    if (bcm_line_config_get()) {
                        cterr('f', 0, "Failed to get line config");
                        return (FAILED);
                    }
                } else {
                    if (bcm_line_config_get()) {
                        cterr('f', 0, "Failed to get line config");
                        return (FAILED);
                    }
                }
            }
        }
    }
}

/*******************************************************************************
 *
 * Function   : xdsl_relay_drv_pin_set
 *
 * Description: Pull xDSL_RELAY_DRV high or low.
 *              Control relay pin (gpio5) to allow either AFE pair (line 0 or 1) to
 *              be routed to the center pins of the RJ-45.
 *              Low:  Line 0 (Annex A)
 *              High: Line 1 (Annex M)
 * Inputs     : None
 *
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int xdsl_relay_drv_pin_set (void)
{
    char tx_str[VIPER_NC_MAX_STR_SIZE];
    char comm[VIPER_NC_MAX_STR_SIZE];

    viper_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    if (bcm_line_id) {
    	sprintf(comm, DIAG_BCM63268_RELAY_PIN_HIGH);
	    printf("Pull xDSL Relay Pin High\n");
    } else {
    	sprintf(comm, DIAG_BCM63268_RELAY_PIN_LOW);
        printf("Pull xDSL Relay Pin Low\n");
    }

    viper_nc_dispatch_comm(comm, tx_str);

    if (viper_nc_dispatch_comm_is_ok() != PASSED) {
        printf("%s fail at nc dispatch commandn", __FUNCTION__);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function: bcm63168_do_showtime
 *
 * Description: This function runs the showtime test
 *
 * Input : None.
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
int bcm63168_do_showtime(void)
{
    int retry, retval = PASSED;

    retry = SHOWTIME_RETRY_TIMES;
    while (retry) {
        retval = bcm63168_connection_start();
        if (retval) {
            /* Do retry when connection fails */
            msleep(WAIT_DSL_CONN_START);
            retry--;
        } else {
            retry = 0;
        }
    }

    /* if either line 0/1 does not link up, then return fail */
    if (retval) {
        return (FAILED);
    }
    /* check the bonding state */
    if (bcm_channel_bonding) {
        retval = bcm_bonding_state_get();
    }

    /* display the info */
    if (bcm_channel_bonding) {
        bcm63168_get_xtm_bonding_info();
    } else {
        bcm63168_get_adslmib_info();
    }

    return (retval);
}

/*******************************************************************************
 *
 * Function: bcm63168_showtime_test
 *
 * Description: This function runs the showtime test
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
int bcm63168_showtime_test(void)
{
    int retval = FAILED;

    /***** connection start *****/
    retval = bcm63168_do_showtime();
    if (retval != PASSED) {
        cterr('f', 0, "bcm63168 do showtime fail\n");
        return (retval);
    }

    /***** connection stop *****/
    retval = bcm63168_self_disconnect();
    if (retval != PASSED) {
        cterr('f', 0, "bcm63168 self disconnect fail\n");
        return (retval);
    }

    printf("BCM63168 do showtime completed\n");
    return (retval);
}

/*******************************************************************************
 *
 * Function: bcm63168_set_tone
 *
 * Description: Issues the set tone command to the bcm63168 processor
 *
 * Input : None.
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
int bcm63168_set_tone (void)
{
    char tx_str[VIPER_NC_MAX_STR_SIZE];

    viper_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    viper_nc_dispatch_comm(DIAG_BCM63268_SET_TONES, tx_str);

    if (viper_nc_dispatch_comm_is_ok() != PASSED) {
        printf("%s fail at nc dispatch command\n", __FUNCTION__);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function: bcm63168_set_test_mode
 *
 * Description: This function sets test mode for VDSL
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
int bcm63168_set_test_mode (void)
{
    char tx_str[VIPER_NC_MAX_STR_SIZE];

    viper_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    viper_nc_dispatch_comm(DIAG_BCM63268_SET_TEST_MODE, tx_str);

    if (viper_nc_dispatch_comm_is_ok() != PASSED) {
        printf("%s fail at nc dispatch command\n", __FUNCTION__);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function: bcm63168_send_all_tone
 *
 * Description: This function lets it transmit power spectrum so 
 *              parametric can do PSD test (should be all upstream 
 *              bandwidth)
 *
 * Input : None.
 *
 * Output: PASSED/FAILED.
 *
 *******************************************************************************
 */
int bcm63168_send_all_tone (void)
{
    char tx_str[VIPER_NC_MAX_STR_SIZE];

    /* Need to set the xDSL relay ping for annexA/M SKU. */
    if (viper_dsl_sku != DSL_SKU_ANNEX_B) {
        if (xdsl_relay_drv_pin_set() == FAILED) {
            printf("%s fail at nc dispatch command\n", __FUNCTION__);
            return (FAILED);
        }
    }

    viper_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    viper_nc_dispatch_comm(DIAG_BCM63268_SEND_ALL_TONE, tx_str);

    if (viper_nc_dispatch_comm_is_ok() != PASSED) {
        printf("%s fail at nc dispatch command\n", __FUNCTION__);
        return (FAILED);
    }

    printf("\n\n\n\n\n            Tone generation in progress\n"); 
    printf("            Use a scope to observe the output\n\n\n");
    printf("\nAllow 5 seconds delays\n");

    msleep(SEND_ALL_TONE_DELAY);

    while (1) {
        printf("Then hit 'e' for quit\n");
        if ('e' == getchar()) {
            if (bcm63168_set_test_mode() == FAILED) {
                return (FAILED);
            }
            break;
        }
        clear();
    }
    
    if (bcm63168_get_version() == FAILED) {
    	return (FAILED);
    }
    
    if (bcm63168_get_configure() == FAILED) {
        return (FAILED);
    }

    return (PASSED);
} 

/*******************************************************************************
 *
 * Function: bcm63168_get_idle_listen_result
 *
 * Description: This function sets the bcm63168 into idle or quiet mode
 *
 * Input : None.
 *
 * Output: PASSED/FAILED.
 *
 *******************************************************************************
 */
int bcm63168_get_idle_listen_result (void)
{
    char tx_str[VIPER_NC_MAX_STR_SIZE];

    viper_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    viper_nc_dispatch_comm(DIAG_BCM63268_PRINT_IDLE_LISTEN, tx_str);

    if (viper_nc_dispatch_comm_is_ok() != PASSED) {
        printf("%s fail at nc dispatch command\n", __FUNCTION__);
        return (FAILED);
    }
    
    return (PASSED);
}   

/*******************************************************************************
 *
 * Function: bcm63168_idle_listen
 *
 * Description: This function sets the bcm63168 into idle or quiet mode
 *
 * Input : None.
 *
 * Output: PASSED/FAILED.
 *
 *******************************************************************************
 */
int bcm63168_idle_listen (void)
{
    int ix, ch;
    char tx_str[VIPER_NC_MAX_STR_SIZE];

    /* Need to set the xDSL relay ping for annexA/M SKU. */
    if (viper_dsl_sku != DSL_SKU_ANNEX_B) {
        if (xdsl_relay_drv_pin_set() == FAILED) {
            printf("%s fail at nc dispatch command\n", __FUNCTION__);
            return (FAILED);
        }
    }

    if (bcm_idle_listen_params) {
    	bcm_qln_monitor_time = (long)getdec_answer("Time (s)", IDLE_LISTEN_TIME, 0, 65535);
    	bcm_qln_monitor_freq = (long)getdec_answer("Frequency (ms)", IDLE_LISTEN_FREQ, 0, 65535);
    }

    viper_nc_init_parms_file();
    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);
    viper_nc_dispatch_comm(DIAG_BCM63268_SET_IDLE_LISTEN, tx_str);
    if (viper_nc_dispatch_comm_is_ok() != PASSED) {
        printf("%s fail at nc dispatch command\n", __FUNCTION__);
        return (FAILED);
    }

    printf("Please generate the signal at Tip & Ring\n");
    
    do {
        printf("Then hit any key to start listening\n");
        ch = getchar();
        if (ch == -1) {
        	printf("get char failed.\n");
        	return (FAILED);
        }
        clear();

        printf("Please wait QLN monitor time delay (%d seconds).\n",
                bcm_qln_monitor_time);
        for(ix = 0; ix < (bcm_qln_monitor_time); ix++) {
            printf(" .");
            fflush(stdout);
            msleep(SLEEP_1000);
        }    
        printf("\n");
        msleep(SLEEP_1000);

        /* get idle listen result from bcm and print it */
        if (bcm63168_get_idle_listen_result()) {
            return (FAILED);
        }
        if (bcm63168_set_test_mode()) {
            return (FAILED);
        }
    } while (0);
    
    return (PASSED);
}    

/*******************************************************************************
 *
 * Function: bcm63168_reset_statcounters
 *
 * Description: This function reset vdsl2 counters
 *
 * Input : None
 *
 * Output: PASSED/FAILED.
 *
 *******************************************************************************
 */
int bcm63168_reset_statcounters (void)
{
    char tx_str[VIPER_NC_MAX_STR_SIZE];

    viper_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    viper_nc_dispatch_comm(DIAG_BCM63268_RESET_STATCOUNTERS, tx_str);

    if (viper_nc_dispatch_comm_is_ok() != PASSED) {
        printf("%s fail at nc dispatch command\n", __FUNCTION__);
        return (FAILED);
    }

    return (PASSED);    
}

/*******************************************************************************
 *
 * Function: bcm63168_get_xdsl_info
 *
 * Description: This function gets xdsl info
 *
 * Input : None
 *
 * Output: PASSED/FAILED.
 *
 *******************************************************************************
 */
int bcm63168_get_xdsl_info (void)
{
    uint op_mode, ori_op_mode;
    char tx_str[VIPER_NC_MAX_STR_SIZE];
    
    printf("\nSelect the info you want to get:");
    printf("\n1. --state");
    printf("\n2. --stats");    
    printf("\n3. --SNR");    
    printf("\n4. --QLN");
    printf("\n5. --Hlog");    
    printf("\n6. --linediag1");    
    printf("\n7. --pbParam");    
    printf("\n8. --Hlin");    
    printf("\n9. --Bits");    

    ori_op_mode = bcm_op_mode;

    op_mode = getdec_answer("", 1, 1, 9);
    
    bcm_op_mode = op_mode;

    viper_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    viper_nc_dispatch_comm(DIAG_BCM63268_GET_XDSL_INFO, tx_str);

    if (viper_nc_dispatch_comm_is_ok() != PASSED) {
        printf("%s fail at nc dispatch command\n", __FUNCTION__);
        return (FAILED);
    }

    bcm_op_mode = ori_op_mode;

    return (PASSED);    
}

/*******************************************************************************
 *
 * Function: bcm63168_get_adslmib_info
 *
 * Description: This function prints xdsl mib object info
 *
 * Input : None
 *
 * Output: PASSED/FAILED.
 *
 *******************************************************************************
 */
int bcm63168_get_adslmib_info (void)
{
    char tx_str[VIPER_NC_MAX_STR_SIZE];

    viper_nc_init_parms_file();

    /* to build up nc command 
     * first  bcm_op_mode
     * second bcm_line_id
     * thrid  bcm idle listen time
     * forth  bcm idle listen frquence*/

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    viper_nc_dispatch_comm(DIAG_BCM63268_GET_ADSLMIB_INFO, tx_str);

    if (viper_nc_dispatch_comm_is_ok() != PASSED) {
        printf("%s fail at nc dispatch command\n", __FUNCTION__);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function: bcm63168_get_xtm_bonding_info
 *
 * Description: This function get xtm bonding info
 *
 * Input : None.
 *
 * Output: PASSED/FAILED.
 *
 *******************************************************************************
 */
int bcm63168_get_xtm_bonding_info (void)
{
    char tx_str[VIPER_NC_MAX_STR_SIZE];

    viper_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    viper_nc_dispatch_comm(DIAG_BCM63268_GET_XTM_BONDING_INFO, tx_str);

    if (viper_nc_dispatch_comm_is_ok() != PASSED) {
        printf("%s fail at nc dispatch command\n", __FUNCTION__);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function: bcm63168_sel_op_mode
 *
 * Description: This function select bcm63168 xDSL mode
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int bcm63168_sel_op_mode(void)
{
    uint ix, op_mode;

    printf("\n1. ANSI_T1_413_ID (ANSI T1.413 1998)");
    if ((viper_dsl_sku == DSL_SKU_ANNEX_M) ||
        (viper_dsl_sku == DSL_SKU_ANNEX_A)) {
        printf("\n2. ITU_G_992_1A_ID (G.992.1 Annex A)");
        printf("\n3. ITU_G_992_3A_ID (G.992.3, Annex A)");
        printf("\n4. ITU_G_992_5A_ID (G.992.5, Annex A)");
        printf("\n6. ITU_G_992_3M_ID (G.992.3, Annex M)");
        printf("\n7. ITU_G_992_5M_ID (G.992.5, Annex M)");
    } else if (viper_dsl_sku == DSL_SKU_ANNEX_B) { 
        printf("\n8. ITU_G_992_1B_ID (G.992.1 Annex B)");
        printf("\n9. ITU_G_992_3B_ID (G.992.3, Annex B)");
        printf("\n10. ITU_G_992_5B_ID (G.992.5, Annex B)");
    }

    printf("\n11. ITU_G_993_1 (VDSL1)");
    printf("\n12. ITU_G_993_2 (VDSL2)");

    printf("\nSelect Mode");
    fflush(stdout);

    op_mode = getdec_answer("", ITU_G_993_2, ANSI_T1_413, ITU_G_993_2);

    bcm_op_mode = op_mode;
    
    switch (op_mode) {
    case ANSI_T1_413:
        bcm_vdsl_op_mode = ANSI_T1_413_ID;
        break;
    case ITU_G_992_1A:
        bcm_vdsl_op_mode = ITU_G_992_1A_ID;
        break;
    case ITU_G_992_3A:
        bcm_vdsl_op_mode = ITU_G_992_3A_ID;
        break;
    case ITU_G_992_5A:
        bcm_vdsl_op_mode = ITU_G_992_5A_ID;
        break;
    case ITU_G_992_3M:
        bcm_vdsl_op_mode = ITU_G_992_3M_ID;
        break;
    case ITU_G_992_5M:
        bcm_vdsl_op_mode = ITU_G_992_5M_ID;
        break;   
    case ITU_G_992_1B:
        bcm_vdsl_op_mode = ITU_G_992_1B_ID;
        break;   
    case ITU_G_992_3B:
        bcm_vdsl_op_mode = ITU_G_992_3B_ID;
        break;
    case ITU_G_992_5B:
        bcm_vdsl_op_mode = ITU_G_992_5B_ID;
        break;
    case ITU_G_993_1:
        bcm_vdsl_op_mode = ITU_G_993_1_ID;
        break;    
    case ITU_G_993_2:
        bcm_vdsl_op_mode = ITU_G_993_2_ID;
        break;
    default:  /* Default mode : VDSL2 */
        bcm_vdsl_op_mode = ITU_G_993_2_ID;
        bcm_op_mode = ITU_G_993_2;
        break;
    }
    /* Find op_mode index number  */
    for (ix = 0; ix < (sizeof(bcm_mode_str)/sizeof(bcm_id_string_t)); ix++) {
        if (bcm_vdsl_op_mode == bcm_mode_str[ix].code) {
            break;
        }
    }
    bcm_op_string = bcm_mode_str[ix].name;
    
    return (PASSED);
}

/*******************************************************************************
 *
 * Function: bcm63168_set_op_mode
 *
 * Description: This function configures and shows status of bcm63168 chip
 *
 * Input : None.
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int bcm63168_set_op_mode(void)
{
    char tx_str[VIPER_NC_MAX_STR_SIZE];

    viper_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    if ((viper_dsl_sku == DSL_SKU_ANNEX_A) ||
        (viper_dsl_sku == DSL_SKU_ANNEX_B) ||
        (viper_dsl_sku == DSL_SKU_ANNEX_M)) {
        viper_nc_dispatch_comm(DIAG_BCM63268_SET_LINE_MODE, tx_str);

        if (viper_nc_dispatch_comm_is_ok() != PASSED) {
            printf("%s fail at nc dispatch command\n", __FUNCTION__);
            return (FAILED);
        }
    } else {
        printf("\nThe SKU(0x%x) is not supported.", viper_dsl_sku);
        return (FAILED);
    }

    if (bcm63168_get_line_mode() == FAILED) {
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function: bcm63168_initialize
 *
 * Description: Issues the Initialize command to the bcm63168 processor
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
int bcm63168_initialize (void)
{
    char tx_str[VIPER_NC_MAX_STR_SIZE];

    viper_nc_init_parms_file();

    /* to build up nc command 
     * first  bcm_op_mode
     * second bcm_line_id
     * thrid  bcm idle listen time
     * forth  bcm idle listen frquence*/

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    viper_nc_dispatch_comm(DIAG_INIT_BCM63268, tx_str);

    if (viper_nc_dispatch_comm_is_ok() != PASSED) {
        printf("%s fail at nc dispatch command\n", __FUNCTION__);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function: bcm63168_uninitialize
 *
 * Description: Issues the Un-Initialize command to the bcm63168 processor
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
int bcm63168_uninitialize (void)
{
    char tx_str[VIPER_NC_MAX_STR_SIZE];

    viper_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    viper_nc_dispatch_comm(DIAG_UNINIT_BCM63268, tx_str);

    if (viper_nc_dispatch_comm_is_ok() != PASSED) {
        printf("%s fail at nc dispatch command\n", __FUNCTION__);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function: bcm63168_configure
 *
 * Description: Issues the Configure command to the bcm63168 processor
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
int bcm63168_configure (void)
{
    char tx_str[VIPER_NC_MAX_STR_SIZE];

    viper_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    viper_nc_dispatch_comm(DIAG_CONFIG_BCM63268, tx_str);

    if (viper_nc_dispatch_comm_is_ok() != PASSED) {
        printf("%s fail at nc dispatch command\n", __FUNCTION__);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function: bcm63168_get_configure
 *
 * Description: Issues the GetConfiguration command to the bcm63168 processor
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
int bcm63168_get_configure (void)
{
    char tx_str[VIPER_NC_MAX_STR_SIZE];

    viper_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    viper_nc_dispatch_comm(DIAG_BCM63268_GET_CONFIG_INFO, tx_str);

    if (viper_nc_dispatch_comm_is_ok() != PASSED) {
        printf("%s fail at nc dispatch command\n", __FUNCTION__);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function: bcm63168_get_version
 *
 * Description: Issues the GetVersion command to the bcm63168 processor
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
int bcm63168_get_version (void)
{
    char tx_str[VIPER_NC_MAX_STR_SIZE];

    viper_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    viper_nc_dispatch_comm(DIAG_BCM63268_GET_VERSION, tx_str);

    if (viper_nc_dispatch_comm_is_ok() != PASSED) {
        printf("%s fail at nc dispatch command\n", __FUNCTION__);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function: bcm63168_show_profile
 *
 * Description: Issues the show profile command to the bcm63168 processor
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
int bcm63168_show_profile (void)
{
    char tx_str[VIPER_NC_MAX_STR_SIZE];

    viper_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    viper_nc_dispatch_comm(DIAG_BCM63268_SHOW_PROFILE, tx_str);

    if (viper_nc_dispatch_comm_is_ok() != PASSED) {
        printf("%s fail at nc dispatch command\n", __FUNCTION__);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function: bcm63168_show_spi_flash_reg
 *
 * Description: Issues the show spi flash reg command to the bcm63168 processor
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
int bcm63168_show_spi_flash_reg (void)
{
    char tx_str[VIPER_NC_MAX_STR_SIZE];

    viper_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    viper_nc_dispatch_comm(DIAG_BCM63268_SHOW_SPI_FLASH_REG, tx_str);

    if (viper_nc_dispatch_comm_is_ok() != PASSED) {
        printf("%s fail at nc dispatch command\n", __FUNCTION__);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function: bcm63168_connection_start
 *
 * This function sets the connection to the DSLAM
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
int bcm63168_connection_start (void)
{
    int ix;
    ushort line_state = BCM_ADSL_LINK_DOWN;
    ushort line_state0 = BCM_ADSL_LINK_DOWN;
    ushort line_state1 = BCM_ADSL_LINK_DOWN;	
    uint temp_line_id;
    char tx_str[VIPER_NC_MAX_STR_SIZE];

    /* Need to set the xDSL relay ping for annexA/M SKU. */
    if (viper_dsl_sku != DSL_SKU_ANNEX_B) {
        if (xdsl_relay_drv_pin_set() == FAILED) {
            printf("%s fail\n", __FUNCTION__);
            return (FAILED);
        }
    }

    temp_line_id = bcm_line_id;

    if (bcm_channel_bonding) {
        bcm_line_id = BCM_DSL_LINE_BONDING;
    }

    viper_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    viper_nc_dispatch_comm(DIAG_BCM63268_CONN_START, tx_str);

    if (viper_nc_dispatch_comm_is_ok() != PASSED) {
        printf("%s fail at nc dispatch command\n", __FUNCTION__);
        return (FAILED);
    }

    for (ix = 0; ix <= DSL_CONN_TIMEOUT; ix++) {
        msleep(DSL_WAIT_10MS); 

        /* get the link status in every 5 sec */
        if ((ix % GET_LINE_DELAY) == 0) {
            if (bcm_channel_bonding) {
                bcm_line_id = BCM_DSL_LINE_0;
                line_state0 = bcm_line_state_get();
                bcm_line_id = BCM_DSL_LINE_1;
                line_state1 = bcm_line_state_get();
            } else {
                line_state = bcm_line_state_get();
            }
        }
        /* After 1 or 2 minutes if the state is link down then stop, otherwise
           give it 1 more minute, up to 3 minutes maximum */
        if (bcm_channel_bonding) {
            if ((ix >= WAIT_BONDING_SHOWTIME) && (line_state0 == BCM_ADSL_LINK_UP)
                             && (line_state1 == BCM_ADSL_LINK_UP)) {
                break;
            }
        } else {
            if ((ix >= WAIT_SINGLE_SHOWTIME) && (line_state == BCM_ADSL_LINK_UP)) {
                break;
            }
        }
    }

    bcm_line_id = temp_line_id;

    if (bcm_channel_bonding) {
        if ((line_state0 != BCM_ADSL_LINK_UP) ||
            (line_state1 != BCM_ADSL_LINK_UP)) {
            return (FAILED);
        } else {
            printf("Broadcom showtime: Bonding connection established\n");
        }
    } else {
        if (line_state != BCM_ADSL_LINK_UP) {
            return (FAILED);
        } else {
            printf("Broadcom showtime: Connection established\n");
        }
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function: bcm63168_connection_stop
 *
 * Description: Issues Connection Stop command to the bcm63168 processor
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
int bcm63168_connection_stop (void)
{
    int line_state = 0;
    char tx_str[VIPER_NC_MAX_STR_SIZE];

    /* Need to set the xDSL relay ping for annexA/M SKU. */
    if (viper_dsl_sku != DSL_SKU_ANNEX_B) {
        if (xdsl_relay_drv_pin_set() == FAILED) {
            printf("%s fail\n", __FUNCTION__);
            return (FAILED);
        }
    }

    viper_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    viper_nc_dispatch_comm(DIAG_BCM63268_CONN_STOP, tx_str);

    if (viper_nc_dispatch_comm_is_ok() != PASSED) {
        printf("%s fail at nc dispatch command\n", __FUNCTION__);
        return (FAILED);
    }

    line_state = bcm_line_state_get();

    if (line_state != BCM_ADSL_LINK_DOWN) {
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function: bcm63168_showtime_no_retrain
 *
 * Description: Issues no retrain command to the bcm63168 processor
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
int bcm63168_showtime_no_retrain(void)
{
    char tx_str[VIPER_NC_MAX_STR_SIZE];

    viper_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    viper_nc_dispatch_comm(DIAG_BCM63268_SHOWTIME_CONT, tx_str);

    if (viper_nc_dispatch_comm_is_ok() != PASSED) {
        printf("%s fail at nc dispatch command\n", __FUNCTION__);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function: bcm63168_get_conn_info
 *
 * Description: Issues the Get Connection Info command to the bcm63168
 *
 * Input : None
 * 
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
int bcm63168_get_conn_info (void)
{
    char tx_str[VIPER_NC_MAX_STR_SIZE];

    viper_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    viper_nc_dispatch_comm(DIAG_BCM63268_GET_CONN_INFO, tx_str);

    if (viper_nc_dispatch_comm_is_ok() != PASSED) {
        printf("%s fail at nc dispatch command\n", __FUNCTION__);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function: bcm63168_led_test
 *
 * Description: Issues LED test command to the bcm63168 processor
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
int bcm63168_led_test (void)
{
    char tx_str[VIPER_NC_MAX_STR_SIZE];

    viper_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    viper_nc_dispatch_comm(DIAG_BCM63268_LED_TEST, tx_str);

    if (viper_nc_dispatch_comm_is_ok() != PASSED) {
        printf("%s fail at nc dispatch command\n", __FUNCTION__);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function: bcm63168_get_line_mode
 *
 * Description: Issues the command to the bcm63168 processor to get
 *              the line state
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
int bcm63168_get_line_mode (void)
{
    char tx_str[VIPER_NC_MAX_STR_SIZE];

    viper_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    viper_nc_dispatch_comm(DIAG_BCM63268_GET_LINE_MODE, tx_str);

    if (viper_nc_dispatch_comm_is_ok() != PASSED) {
        printf("%s fail at nc dispatch command\n", __FUNCTION__);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function: bcm63168_vdsl_test_option_select
 *
 * Description: This function allows to select different test options
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
int bcm63168_vdsl_test_option_select (void)
{
    uchar ch;
    uchar showtime_auto;
    ushort line_id, channel_bonding,idle_params;

    while (1) {
        printf("\nBroadcom 63268 Test Configuration Menu\n");

        showtime_auto = (bcm_do_show_time_auto ? 'y':'n');
        idle_params = (bcm_idle_listen_params ? 'y':'n');
        channel_bonding = (bcm_channel_bonding ? 'y':'n');
        line_id = (bcm_line_id ? BCM_DSL_LINE_1:BCM_DSL_LINE_0);
        
        printf("\n1. Perform auto disconnect in show time?[y/n]: %c", showtime_auto);
        printf("\n2. Select xDSL mode: %s", bcm_op_string);
        printf("\n3. Set parameters for Idle Listen?[y/n]: %c", idle_params);
        printf("\n4. Set channel bonding?[y/n]: %c", channel_bonding);
        printf("\n5. Select Line ID?[0/1]: %d", line_id);
        printf("\nq. Quit\n");

        ch = getchar();
        clear();

        switch (ch) {
        case '1':
            printf ("\nEnter value [y/n]: ");
            bcm_do_show_time_auto = (('y' == getchar()) ? TRUE : FALSE);
            break;
        case '2':
            if (bcm63168_sel_op_mode() == FAILED) {
                return (FAILED);
            }
            break;
        case '3':
            printf ("\nEnter value [y/n]: ");
            bcm_idle_listen_params = (('y' == getchar()) ? 1 : 0);
            break;
        case '4':
            printf ("\nEnter value [y/n]: ");
            bcm_channel_bonding = (('y' == getchar()) ? 1 : 0);
            msleep(100); /* For the chip to reconfigure */
            break;
        case '5':
            printf ("\nEnter Line ID [0/1]: ");
            bcm_line_id = (('1' == getchar()) ? 1 : 0);
            msleep(100); /* For the chip to reconfigure */
            break;
        case 'q':
            if (bcm63168_set_op_mode() == FAILED) {
                return (FAILED);
            }
            msleep(WAIT_BCM_RECONFIG);
            return (PASSED);
        default:
            printf("\nInvalid input\n");
            break;
        }

        if (ch != '2') {
            clear();
        }
    }
    
    return (PASSED);
}

/*******************************************************************************
 *
 * Function: bcm63168_en_wp_spi_flash_reg
 *
 * Description: Issues the enable spi flash reg command to the bcm63168 processor
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
int bcm63168_en_wp_spi_flash_reg (void)
{
    char tx_str[VIPER_NC_MAX_STR_SIZE];

    viper_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    viper_nc_dispatch_comm(DIAG_BCM63268_SPI_PROTECT, tx_str);

    if (viper_nc_dispatch_comm_is_ok() != PASSED) {
        printf("%s fail at nc dispatch command\n", __FUNCTION__);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function: bcm63168_dis_wp_spi_flash_reg
 *
 * Description: Issues the disable spi flash reg command to the bcm63168 processor
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
int bcm63168_dis_wp_spi_flash_reg (void)
{
    char tx_str[VIPER_NC_MAX_STR_SIZE];

    viper_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    viper_nc_dispatch_comm(DIAG_BCM63268_SPI_UNPROTECT, tx_str);

    if (viper_nc_dispatch_comm_is_ok() != PASSED) {
        printf("%s fail at nc dispatch command\n", __FUNCTION__);
        return (FAILED);
    }

    return (PASSED);
}
/******** History ********
$Log: diag_dsl_libs.c,v $
Revision 1.3  2018/09/21 02:48:54  harrchan
Merge viper DSL to the main trunk (CSCvm57542)

Revision 1.2  2018/08/06 02:31:50  harrchan
Merge viper E2E to the main trunk (CSCvk28469)

Revision 1.1.2.3  2018/06/27 06:27:48  harrchan
Follow the coding rule to clean up code

Revision 1.1.2.2  2018/04/16 08:41:43  olin2
Support DSL test

Revision 1.1.2.1  2018/02/27 08:06:33  harrchan
Initial viper application code base





$Endlog$
*/
