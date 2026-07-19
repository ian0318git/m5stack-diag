/* $Id: diag_dsl_lib.c,v 1.3 2019/05/21 07:44:19 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/diag_dsl_lib.c,v $
 *------------------------------------------------------------------
 * 
 * diag_dsl_lib.c
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
#include "nvmonvars.h"
#include "error.h"
#include "menu.h"
#include "defs.h"
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "common.h"
#include "types.h"
#include "queryflags.h"
#include "strings.h"
#include "common_utils.h"
#include "diag_moka_fpga_lib.h"
#include "diag_dsl_lib.h"
#include "diag_uart_lib.h"
#include "diag_dsl_test.h"

static void plat_transmit_nc_request(int);
static void plat_get_module_ip_addr(char *);

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
static void diag_dsl_clear_char(void);

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

extern unsigned int plat_turbo_sku;
extern int getdec_answer(char *,uint ,uint ,uint);

/*******************************************************************************
 * Function: diag_dsl_clear_char
 *
 * Description: To ensure the clear operation is done.
 * Input : None
 * Output: None
 *******************************************************************************
 */
static void diag_dsl_clear_char (void)
{    
    while ( getchar() != '\n' );
}

/*******************************************************************************
 * Function: bcm63168_check_sku_type
 *
 * Description: Check if SKU ID in xDSL module and in ACT2 are identical.
 * Input : None
 * Output: PASSED/FAILED
 *******************************************************************************
 */
int bcm63168_check_sku_type(void)
{
    int sku_id = 0;
    char tx_str[PLAT_NC_MAX_STR_SIZE];
    char rx_str[PLAT_NC_MAX_STR_SIZE];

    plat_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    plat_nc_dispatch_comm(DIAG_BCM63268_GET_SKU_TYPE, tx_str);

    if (plat_nc_dispatch_comm_is_ok() != PASSED) {
        printf("%s fail\n", __FUNCTION__);
        return (FAILED);
    }

    if (plat_nc_get_parms(1, rx_str) != PASSED) {
    	printf("Catch return parameter failed\n");
        return (FAILED);
    }

    sku_id = atoi(rx_str);

    if (plat_turbo_sku != sku_id) {
        printf("Check SKU type failed. Expect sku = 0x%x, Read BCM sku = 0x%x\n",
               plat_turbo_sku, sku_id);
        return (FAILED);
    }

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
    char tx_str[PLAT_NC_MAX_STR_SIZE];
    char rx_str[PLAT_NC_MAX_STR_SIZE];

    /* Due to line1 link status is meaningless when training single line.
     * Only check line0 link status for line0 and line1 that in use.
     */
    if (!bcm_channel_bonding) {
        line_id = BCM_DSL_LINE_0;
    }

    plat_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    plat_nc_dispatch_comm(DIAG_BCM63268_GET_CONN_INFO, tx_str);


    if (plat_nc_dispatch_comm_is_ok() != PASSED) {
        printf("%s fail\n", __FUNCTION__);
        return (FAILED);
    }

    if (plat_nc_get_parms(1, rx_str) != PASSED) {
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
    char tx_str[PLAT_NC_MAX_STR_SIZE];

    plat_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    plat_nc_dispatch_comm(DIAG_BCM63268_GET_BONDING_STATE, tx_str);

    if (plat_nc_dispatch_comm_is_ok() != PASSED) {
        printf("%s fail\n", __FUNCTION__);
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
    char tx_str[PLAT_NC_MAX_STR_SIZE];

    plat_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    plat_nc_dispatch_comm(DIAG_BCM63268_GET_CONFIG_INFO, tx_str);

    if (plat_nc_dispatch_comm_is_ok() != PASSED) {
        printf("%s fail\n", __FUNCTION__);
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
            printf("%s:%d:Failed to send connection stop command\n", 
                   __FUNCTION__, __LINE__);
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
                printf("%s:%d:Fail to deactivate the line, line state0 = %d, line state1 = %d\n", 
                       __FUNCTION__, __LINE__, line_state0, line_state1);
                return (FAILED);
            }
        } else {
            if (line_state != BCM_ADSL_LINK_DOWN) {
                printf("%s:%d:Fail to deactivate the line, line state = %d\n", 
                       __FUNCTION__, __LINE__, line_state);
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
            msleep(10); /* give some time */

            if ((ix%500) == 0) {
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
                printf("%s:%d:Fail to connect, line state0 = 0x%08x\n, " 
                       "line state1 = 0x%08x\n\n", 
                        __FUNCTION__, __LINE__, line_state0, line_state1);
                return (FAILED);
            }
        } else {
            if (line_state != BCM_ADSL_LINK_UP) {
                printf("%s:%d:Fail to connect, line%d state = 0x%08x\n\n", 
                       __FUNCTION__, __LINE__, bcm_line_id, line_state);
                return (FAILED);
            }
        }
    
        while (1) {
            printf("\nHit 'd' to deactivate the line or\n");
            printf("any other key for configurations\n");
            ch = getchar();
            diag_dsl_clear_char();
            if (ch == 'd') {
                if (bcm63168_connection_stop()) {
                    printf("%s:%d:Failed to send connection stop command\n", 
                           __FUNCTION__, __LINE__);
                    return (FAILED);
                }
                return (PASSED);
            } else {
               if (bcm_channel_bonding) {
                    bcm_line_id = BCM_DSL_LINE_0;
                    if (bcm_line_config_get()) {
                        printf("%s:%d:Failed to get line config\n", 
                               __FUNCTION__, __LINE__);
                        return (FAILED);
                    }

                    bcm_line_id = BCM_DSL_LINE_1;
                    if (bcm_line_config_get()) {
                        printf("%s:%d:Failed to get line config\n", 
                               __FUNCTION__, __LINE__);
                        return (FAILED);
                    }
               } else {
                    if (bcm_line_config_get()) {
                        printf("%s:%d:Failed to get line config\n", 
                               __FUNCTION__, __LINE__);
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
    char tx_str[PLAT_NC_MAX_STR_SIZE];
    char comm[PLAT_NC_MAX_STR_SIZE];

    plat_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    if (bcm_line_id) {
    	sprintf(comm, DIAG_BCM63268_RELAY_PIN_HIGH);
	    printf("Pull xDSL Relay Pin High\n");
    } else {
    	sprintf(comm, DIAG_BCM63268_RELAY_PIN_LOW);
        printf("Pull xDSL Relay Pin Low\n");
    }

    plat_nc_dispatch_comm(comm, tx_str);

    if (plat_nc_dispatch_comm_is_ok() != PASSED) {
        printf("%s fail\n", __FUNCTION__);
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
        printf("%s:%d:bcm63168 do showtime fail\n", __FUNCTION__, __LINE__);
        return (retval);
    }

    /***** connection stop *****/
    retval = bcm63168_self_disconnect();
    if (retval != PASSED) {
        printf("%s:%d:bcm63168 self disconnect fail\n", __FUNCTION__, __LINE__);
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
    char tx_str[PLAT_NC_MAX_STR_SIZE];

    plat_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    plat_nc_dispatch_comm(DIAG_BCM63268_SET_TONES, tx_str);

    if (plat_nc_dispatch_comm_is_ok() != PASSED) {
        printf("%s fail\n", __FUNCTION__);
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
    char tx_str[PLAT_NC_MAX_STR_SIZE];

    plat_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    plat_nc_dispatch_comm(DIAG_BCM63268_SET_TEST_MODE, tx_str);

    if (plat_nc_dispatch_comm_is_ok() != PASSED) {
        printf("%s fail\n", __FUNCTION__);
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
    char tx_str[PLAT_NC_MAX_STR_SIZE];

    /* Need to set the xDSL relay ping for annexA/M SKU. */
    if ((plat_turbo_sku != DSL_SKU_ANNEX_B) && 
        (!(plat_turbo_sku & DSL138_SKU_GFAST))) {
        if (xdsl_relay_drv_pin_set() == FAILED) {
            printf("%s fail\n", __FUNCTION__);
            return (FAILED);
        }
    }

    plat_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    plat_nc_dispatch_comm(DIAG_BCM63268_SEND_ALL_TONE, tx_str);

    if (plat_nc_dispatch_comm_is_ok() != PASSED) {
        printf("%s fail\n", __FUNCTION__);
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
        diag_dsl_clear_char();
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
    char tx_str[PLAT_NC_MAX_STR_SIZE];

    plat_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    plat_nc_dispatch_comm(DIAG_BCM63268_PRINT_IDLE_LISTEN, tx_str);

    if (plat_nc_dispatch_comm_is_ok() != PASSED) {
        printf("%s fail\n", __FUNCTION__);
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
    char tx_str[PLAT_NC_MAX_STR_SIZE];

    /* Need to set the xDSL relay ping for annexA/M SKU. */
    if ((plat_turbo_sku != DSL_SKU_ANNEX_B) && 
        (!(plat_turbo_sku & DSL138_SKU_GFAST))) {
        if (xdsl_relay_drv_pin_set() == FAILED) {
            printf("%s fail\n", __FUNCTION__);
            return (FAILED);
        }
    }

    if (bcm_idle_listen_params) {
    	bcm_qln_monitor_time = (long)getdec_answer("Time (s)", IDLE_LISTEN_TIME, 0, 65535);
    	bcm_qln_monitor_freq = (long)getdec_answer("Frequency (ms)", IDLE_LISTEN_FREQ, 0, 65535);
    }

    plat_nc_init_parms_file();
    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);
    plat_nc_dispatch_comm(DIAG_BCM63268_SET_IDLE_LISTEN, tx_str);
    if (plat_nc_dispatch_comm_is_ok() != PASSED) {
        printf("%s fail\n", __FUNCTION__);
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
        diag_dsl_clear_char();

        printf("Please wait QLN monitor time delay (%d seconds).\n",
                bcm_qln_monitor_time);
        for(ix = 0; ix < (bcm_qln_monitor_time); ix++) {
            printf(" .");
            fflush(stdout);
            msleep(1000);
        }    
        printf("\n");
        msleep(1000);

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
    char tx_str[PLAT_NC_MAX_STR_SIZE];

    plat_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    plat_nc_dispatch_comm(DIAG_BCM63268_RESET_STATCOUNTERS, tx_str);

    if (plat_nc_dispatch_comm_is_ok() != PASSED) {
        printf("%s fail\n", __FUNCTION__);
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
    char tx_str[PLAT_NC_MAX_STR_SIZE];
    
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

    plat_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    plat_nc_dispatch_comm(DIAG_BCM63268_GET_XDSL_INFO, tx_str);

    if (plat_nc_dispatch_comm_is_ok() != PASSED) {
        printf("%s fail\n", __FUNCTION__);
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
    char tx_str[PLAT_NC_MAX_STR_SIZE];

    plat_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    plat_nc_dispatch_comm(DIAG_BCM63268_GET_ADSLMIB_INFO, tx_str);

    if (plat_nc_dispatch_comm_is_ok() != PASSED) {
        printf("%s fail\n", __FUNCTION__);
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
    char tx_str[PLAT_NC_MAX_STR_SIZE];

    plat_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    plat_nc_dispatch_comm(DIAG_BCM63268_GET_XTM_BONDING_INFO, tx_str);

    if (plat_nc_dispatch_comm_is_ok() != PASSED) {
        printf("%s fail\n", __FUNCTION__);
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
    uint plat_annex_a = 0;
    uint plat_annex_m = 0;
    uint plat_annex_bj = 0;
    
    printf("\n1. ANSI_T1_413_ID (ANSI T1.413 1998)");
    plat_annex_a = DSL_SKU_ANNEX_A;
    plat_annex_m = DSL_SKU_ANNEX_M;
    plat_annex_bj = DSL_SKU_ANNEX_B;

    if ((plat_annex_a == plat_turbo_sku) || 
        (plat_annex_m == plat_turbo_sku)) {
        printf("\n2. ITU_G_992_1A_ID (G.992.1 Annex A)");
        printf("\n3. ITU_G_992_3A_ID (G.992.3, Annex A)");
        printf("\n4. ITU_G_992_5A_ID (G.992.5, Annex A)");
        printf("\n6. ITU_G_992_3M_ID (G.992.3, Annex M)");
        printf("\n7. ITU_G_992_5M_ID (G.992.5, Annex M)");
    } else
    if (plat_annex_bj == plat_turbo_sku) { 
        printf("\n8. ITU_G_992_1B_ID (G.992.1 Annex B)");
        printf("\n9. ITU_G_992_3B_ID (G.992.3, Annex B)");
        printf("\n10. ITU_G_992_5B_ID (G.992.5, Annex B)");
    }

    printf("\n11. ITU_G_993_1 (VDSL1)");
    printf("\n12. ITU_G_993_2 (VDSL2)");
    if (plat_annex_bj == plat_turbo_sku) { 
        printf("\n13. ITU_G_992_3J_ID (G.992.3, Annex J)");
        printf("\n14. ITU_G_992_5J_ID (G.992.5, Annex J)");
    }
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
    case ITU_G_992_3J:
        bcm_vdsl_op_mode = ITU_G_992_3J_ID;
        break;
    case ITU_G_992_5J:
        bcm_vdsl_op_mode = ITU_G_992_5J_ID;
        break;    
    default:  /* Default mode : VDSL2 */
        bcm_vdsl_op_mode = ITU_G_993_2_ID;
        bcm_op_mode = ITU_G_993_2;
        break;
    }

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
    char tx_str[PLAT_NC_MAX_STR_SIZE];
    uint plat_annex_a = 0;
    uint plat_annex_m = 0;
    uint plat_annex_bj = 0;
    plat_annex_a = DSL_SKU_ANNEX_A;
    plat_annex_m = DSL_SKU_ANNEX_M;
    plat_annex_bj = DSL_SKU_ANNEX_B;

    plat_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    if ((plat_turbo_sku == plat_annex_a) ||
        (plat_turbo_sku == plat_annex_m) ||
        (plat_turbo_sku == plat_annex_bj)) { 
        plat_nc_dispatch_comm(DIAG_BCM63268_SET_LINE_MODE, tx_str);

        if (plat_nc_dispatch_comm_is_ok() != PASSED) {
            printf("%s fail\n", __FUNCTION__);
            return (FAILED);
        }
    } else {
        printf("\nThe SKU(0x%x) is not supported.", plat_turbo_sku);
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
    char tx_str[PLAT_NC_MAX_STR_SIZE];

    plat_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    plat_nc_dispatch_comm(DIAG_INIT_BCM63268, tx_str);

    if (plat_nc_dispatch_comm_is_ok() != PASSED) {
        printf("%s fail\n", __FUNCTION__);
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
    char tx_str[PLAT_NC_MAX_STR_SIZE];

    plat_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    plat_nc_dispatch_comm(DIAG_UNINIT_BCM63268, tx_str);

    if (plat_nc_dispatch_comm_is_ok() != PASSED) {
        printf("%s fail\n", __FUNCTION__);
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
    char tx_str[PLAT_NC_MAX_STR_SIZE];

    plat_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    plat_nc_dispatch_comm(DIAG_CONFIG_BCM63268, tx_str);

    if (plat_nc_dispatch_comm_is_ok() != PASSED) {
        printf("%s fail\n", __FUNCTION__);
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
    char tx_str[PLAT_NC_MAX_STR_SIZE];

    plat_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    plat_nc_dispatch_comm(DIAG_BCM63268_GET_CONFIG_INFO, tx_str);

    if (plat_nc_dispatch_comm_is_ok() != PASSED) {
        printf("%s fail\n", __FUNCTION__);
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
    char tx_str[PLAT_NC_MAX_STR_SIZE];

    plat_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    plat_nc_dispatch_comm(DIAG_BCM63268_GET_VERSION, tx_str);

    if (plat_nc_dispatch_comm_is_ok() != PASSED) {
        printf("%s fail\n", __FUNCTION__);
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
    char tx_str[PLAT_NC_MAX_STR_SIZE];

    plat_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    plat_nc_dispatch_comm(DIAG_BCM63268_SHOW_PROFILE, tx_str);

    if (plat_nc_dispatch_comm_is_ok() != PASSED) {
        printf("%s fail\n", __FUNCTION__);
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
    char tx_str[PLAT_NC_MAX_STR_SIZE];

    plat_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    plat_nc_dispatch_comm(DIAG_BCM63268_SHOW_SPI_FLASH_REG, tx_str);

    if (plat_nc_dispatch_comm_is_ok() != PASSED) {
        printf("%s fail\n", __FUNCTION__);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function: bcm63168_chk_spi_flash_protect
 *
 * Description: Checking SPI flash protection
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
int bcm63168_chk_spi_flash_protect (void)
{
    char tx_str[PLAT_NC_MAX_STR_SIZE];

    plat_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    plat_nc_dispatch_comm(DIAG_BCM63268_CHK_SPI_FLASH_PROTECT, tx_str);

    if (plat_nc_dispatch_comm_is_ok() != PASSED) {
        printf("%s failed to check the xDSL SPI Flash protection\n", __FUNCTION__);
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
    char tx_str[PLAT_NC_MAX_STR_SIZE];

    /* Need to set the xDSL relay ping for annexA/M SKU. */
    if ((plat_turbo_sku != DSL_SKU_ANNEX_B) && 
        (!(plat_turbo_sku & DSL138_SKU_GFAST))) {
        if (xdsl_relay_drv_pin_set() == FAILED) {
            printf("%s fail\n", __FUNCTION__);
            return (FAILED);
        }
    }

    temp_line_id = bcm_line_id;

    if (bcm_channel_bonding) {
        bcm_line_id = BCM_DSL_LINE_BONDING;
    }

    plat_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    plat_nc_dispatch_comm(DIAG_BCM63268_CONN_START, tx_str);

    if (plat_nc_dispatch_comm_is_ok() != PASSED) {
        printf("%s fail\n", __FUNCTION__);
        return (FAILED);
    }

    for (ix = 0; ix <= DSL_CONN_TIMEOUT; ix++) {
        msleep(10); /* give some time */

        /* get the link status in every 5 sec */
        if ((ix%GET_LINE_DELAY) == 0) {
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
    char tx_str[PLAT_NC_MAX_STR_SIZE];

    /* Need to set the xDSL relay ping for annexA/M SKU. */
    if ((plat_turbo_sku != DSL_SKU_ANNEX_B) && 
        (!(plat_turbo_sku & DSL138_SKU_GFAST))) {
        if (xdsl_relay_drv_pin_set() == FAILED) {
            printf("%s fail\n", __FUNCTION__);
            return (FAILED);
        }
    }

    plat_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    plat_nc_dispatch_comm(DIAG_BCM63268_CONN_STOP, tx_str);

    if (plat_nc_dispatch_comm_is_ok() != PASSED) {
        printf("%s fail\n", __FUNCTION__);
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
    char tx_str[PLAT_NC_MAX_STR_SIZE];

    plat_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    plat_nc_dispatch_comm(DIAG_BCM63268_SHOWTIME_CONT, tx_str);

    if (plat_nc_dispatch_comm_is_ok() != PASSED) {
        printf("%s fail\n", __FUNCTION__);
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
    char tx_str[PLAT_NC_MAX_STR_SIZE];

    plat_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    plat_nc_dispatch_comm(DIAG_BCM63268_GET_CONN_INFO, tx_str);

    if (plat_nc_dispatch_comm_is_ok() != PASSED) {
        printf("%s fail\n", __FUNCTION__);
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
    char tx_str[PLAT_NC_MAX_STR_SIZE];

    plat_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);
    
    plat_nc_dispatch_comm(DIAG_BCM63268_LED_TEST, tx_str);

    if (plat_nc_dispatch_comm_is_ok() != PASSED) {
        printf("%s fail\n", __FUNCTION__);
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
    char tx_str[PLAT_NC_MAX_STR_SIZE];

    plat_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    plat_nc_dispatch_comm(DIAG_BCM63268_GET_LINE_MODE, tx_str);

    if (plat_nc_dispatch_comm_is_ok() != PASSED) {
        printf("%s fail\n", __FUNCTION__);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function: bcm63138_vdsl35b
 *
 * Description: Issues the vdsl35b profile command to the bcm63138 processor
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
int bcm63138_vdsl35b (void)
{
    char tx_str[PLAT_NC_MAX_STR_SIZE];

    plat_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    plat_nc_dispatch_comm(DIAG_SET_PROFILE_35B, tx_str);

    if (plat_nc_dispatch_comm_is_ok() != PASSED) {
        printf("%s fail\n", __FUNCTION__);
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
        diag_dsl_clear_char();

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
            diag_dsl_clear_char();
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
    char tx_str[PLAT_NC_MAX_STR_SIZE];

    plat_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    plat_nc_dispatch_comm(DIAG_BCM63268_SPI_PROTECT, tx_str);

    if (plat_nc_dispatch_comm_is_ok() != PASSED) {
        printf("%s fail\n", __FUNCTION__);
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
    char tx_str[PLAT_NC_MAX_STR_SIZE];

    plat_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);

    plat_nc_dispatch_comm(DIAG_BCM63268_SPI_UNPROTECT, tx_str);

    if (plat_nc_dispatch_comm_is_ok() != PASSED) {
        printf("%s fail\n", __FUNCTION__);
        return (FAILED);
    }

    return (PASSED);
}

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
    char tx_str[PLAT_NC_MAX_STR_SIZE];

    plat_nc_init_parms_file();

    sprintf(tx_str, "%d,%d,%d,%d",
            bcm_op_mode, bcm_line_id, bcm_qln_monitor_time, bcm_qln_monitor_freq);
   
    switch (opt) {
        case LED_OFF :
            plat_nc_dispatch_comm(DIAG_BCM63268_LED_OFF, tx_str);
            break;
        case LED_CD_ON :
            plat_nc_dispatch_comm(DIAG_BCM63268_LED_CD_ON, tx_str);
            break;
        case LED_DATA_ON :
            plat_nc_dispatch_comm(DIAG_BCM63268_LED_DATA_ON, tx_str);
            break;
        default:
            printf("No Option Available\n");
            return (FAILED); 
    }


    if (plat_nc_dispatch_comm_is_ok() != PASSED) {
        printf("%s fail\n", __FUNCTION__);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : xdsl_console_switch
 *
 * Description: A utility to Console Switch to xDSL BRCM console.
 *
 * Inputs     : NONE 
 *             
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int xdsl_console_switch (void)
{
    struct uart_parm picocom;
    picocom.tty_dev = PLAT_DSL_UART_DEV_STR;
    picocom.baudrate = 9600;    
    picocom.databit = 8;
    picocom.parity = "1";
    picocom.flow = "n";

    plat_console_switch(&picocom);

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : pri_intf_rdy_chk
 *
 * Description: Check if DSL_FPGA_EXP_PRI_RDY is asserted.
 *
 * Inputs     : None
 *
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int pri_intf_rdy_chk (void)
{
    unsigned int data;

    if (fpga_read_32_reg(FPGA_DSL_STATUS_CTL_REG, &data) == FAILED) {
        printf("%s:%d:Failed to read CPLD register.\n", __FUNCTION__, __LINE__);
    	return (FAILED);
    }

    /* Check EXP_PRI_RDY is high or low */
    if (data & DSL_FPGA_EXP_PRI_RDY) {
        return (PASSED);
    } else {
        return (FAILED);
    }
}

/*******************************************************************************
 *
 * Function: xdsl_confirm_cfe_parms_set
 *
 * Description: This function set cfe parameters.
 *
 * Input : boolean os_param = diag or ios
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
int xdsl_confirm_cfe_parms_set (boolean os_param)
{
    int ix;
    const int maxlen = PLAT_DSL_MAX_LENGTH;
    char tty[maxlen];
    char linux_file_str[maxlen];
    char linux_ip_str[maxlen];
    int result = PASSED;
    int tty_desc;
    char *dsl_img_filename;

    if (os_param == IOS_CFE_PARAM) { 
        prpass(testpass, "Confirming CFE ios Parameters,");
    } else {
        prpass(testpass, "Confirming CFE diag Parameters,");
    }
    printf("\n");

    printf("Start CFE UART...\n");
    fflush(stdout);
    fflush(stderr);

    snprintf(tty, maxlen-1, PLAT_DSL_UART_DEV_STR);
    tty_desc = open(tty, O_RDWR | O_NOCTTY);
    if (tty_desc < 0) {
        perror("open tty failed");
        fflush(stdout);
        return (FAILED);
    }

    /* Step 0 : Catch "Press any key" */
    result = plat_uart_rx_polling(tty_desc, PLAT_DSL_PRESS_KEY_STRING,
    		                      DSL_CFE_PRESS_KEY_TIMEOUT);
    if (result == TRUE) {
        printf("Found : %s\n", PLAT_DSL_PRESS_KEY_STRING);
        fflush(stdout);
    } else {
        printf("[%s] Not Found.\n", PLAT_DSL_PRESS_KEY_STRING);
        fflush(stdout);
        goto exit_cfe_parms_set_failed;
    } 

    /* Step 1 : Find "CFE>" */
    for (ix = 0; ix < DSL_CFE_RETRY_TIMES; ix++) {
        if (plat_uart_tx(tty_desc, PLAT_DSL_CR_STRING) == FAILED) {
            goto exit_cfe_parms_set_failed;
        }
        result = plat_uart_rx_polling(tty_desc, PLAT_DSL_CFE_STRING,
        		                      DSL_CFE_TIMEOUT);
        if (result == TRUE) {
            printf("Found : %s\n", PLAT_DSL_CFE_STRING);
            fflush(stdout);
            break;
        }
    }

    if (ix == DSL_CFE_RETRY_TIMES) {
        printf("[%s] Not Found.\n", PLAT_DSL_CFE_STRING);
        fflush(stdout);
        goto exit_cfe_parms_set_failed;
    }
    msleep(WAIT_DSL_CFE_PROMPT);

    /* Step 2 : Check Set board parameters   */
    if (plat_uart_tx(tty_desc, PLAT_DSL_PRINT_CFE_PARMS_STRING) == FAILED) {
        goto exit_cfe_parms_set_failed;
    }    
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Send bdpm [%s]\n", PLAT_DSL_PRINT_CFE_PARMS_STRING);
        fflush(stdout);
    }

    if (plat_uart_tx(tty_desc, PLAT_DSL_CR_STRING) == FAILED) {
        goto exit_cfe_parms_set_failed;
    }
    if (os_param == IOS_CFE_PARAM) { 
        sprintf(linux_ip_str, "%s.%d:ffffff00", PLAT_IOS_DSL_SUBNET_STR,
                PLAT_IOS_MODULE_IP_ADDR);
    } else {    
        sprintf(linux_ip_str, "%s.%d", PLAT_DIAG_DSL_SUBNET_STR,
                PLAT_DIAG_MODULE_IP_ADDR);
    }
    /* Step 3: Check Board IP */
    for (ix = 0; ix < DSL_CFE_RETRY_TIMES; ix++) {
        result = plat_uart_rx_polling(tty_desc, linux_ip_str,
        		                      DSL_CFE_TIMEOUT);
        if (result == TRUE) {
            printf("Found : %s\n", linux_ip_str);
            fflush(stdout);
            break;
        }
    }

    if (ix == DSL_CFE_RETRY_TIMES) {
        printf("[%s] Not Found.\n", linux_ip_str);
        fflush(stdout);
        goto exit_cfe_parms_set_failed;
    }

    if (os_param == IOS_CFE_PARAM) { 
        sprintf(linux_ip_str, "%s.%d", PLAT_IOS_DSL_SUBNET_STR,
                PLAT_IOS_HOST_IP_ADDR);
    } else {
        sprintf(linux_ip_str, "%s.%d", PLAT_DIAG_DSL_SUBNET_STR,
                PLAT_DIAG_HOST_IP_ADDR);
    }
    
    /* Step 4: Check Host IP */
    for (ix = 0; ix < DSL_CFE_RETRY_TIMES; ix++) {
        result = plat_uart_rx_polling(tty_desc, linux_ip_str,
        		                      DSL_CFE_TIMEOUT);
        if (result == TRUE) {
            printf("Found : %s\n", linux_ip_str);
            fflush(stdout);
            break;
        }
    }

    if (ix == DSL_CFE_RETRY_TIMES) {
        printf("[%s] Not Found.\n", linux_ip_str);
        fflush(stdout);
        goto exit_cfe_parms_set_failed;
    }

    if (os_param == IOS_CFE_PARAM) { 
        sprintf(linux_ip_str, "%s.%d", PLAT_IOS_DSL_SUBNET_STR,
                PLAT_IOS_GATEWAY_IP_ADDR);
    } else {
        sprintf(linux_ip_str, "%s.%d", PLAT_DIAG_DSL_SUBNET_STR,
                PLAT_DIAG_GATEWAY_IP_ADDR);
    }
    /* Step 5: Check Gateway IP */
    for (ix = 0; ix < DSL_CFE_RETRY_TIMES; ix++) {
        result = plat_uart_rx_polling(tty_desc, linux_ip_str,
        		                      DSL_CFE_TIMEOUT);
        if (result == TRUE) {
            printf("Found : %s\n", linux_ip_str);
            fflush(stdout);
            break;
        }
    }

    if (ix == DSL_CFE_RETRY_TIMES) {
        printf("[%s] Not Found.\n", linux_ip_str);
        fflush(stdout);
        goto exit_cfe_parms_set_failed;
    }
    

    /* Step 6: Check default linux file name */
    if (os_param == IOS_CFE_PARAM) { 
        dsl_img_filename = getenv(IOS_IMAGE_NAME);
    } else {
        dsl_img_filename = getenv(DSL_IMAGE_NAME);
    }

    if (dsl_img_filename == NULL || !strcmp("none", dsl_img_filename)) {
        if (os_param == IOS_CFE_PARAM) { 
            printf("%s: Env variable %s is not set\n", __func__, IOS_IMAGE_NAME);
        } else {
            printf("%s: Env variable %s is not set\n", __func__, DSL_IMAGE_NAME);
        } 
        goto exit_cfe_parms_set_failed;
    } else {
        if (os_param == IOS_CFE_PARAM) { 
            printf("IOS Image Filename: %s\n", dsl_img_filename);
            sprintf(linux_file_str, "%s%s",
                PLAT_IOS_HOST_FIRMWARE_FOLDER_STRING, dsl_img_filename);
        } else {
            printf("DIAG Image Filename: %s\n", dsl_img_filename);
            sprintf(linux_file_str, "%s", dsl_img_filename);
        }
    }
    
    for (ix = 0; ix < DSL_CFE_RETRY_TIMES; ix++) {
        result = plat_uart_rx_polling(tty_desc, linux_file_str,
        		                      DSL_CFE_TIMEOUT);
        if (result == TRUE) {
            printf("Found : %s\n", linux_file_str);
            fflush(stdout);
            break;
        }
    }

    if (ix == DSL_CFE_RETRY_TIMES) {
        printf("[%s] Not Found.\n", linux_file_str);
        fflush(stdout);
        goto exit_cfe_parms_set_failed;
    }

    /* Step 8 : Find "command status" */
    for (ix = 0; ix < DSL_CFE_RETRY_TIMES; ix++) {
        if (plat_uart_tx(tty_desc, PLAT_DSL_CR_STRING) == FAILED) {
            goto exit_cfe_parms_set_failed;
        }
        result = plat_uart_rx_polling(tty_desc, PLAT_DSL_COMMAND_STATUS_STRING,
        		                      DSL_CFE_COMMAND_STATUS_TIMEOUT);
        if (result == TRUE) {
            printf("Found : %s\n", PLAT_DSL_COMMAND_STATUS_STRING);
            fflush(stdout);
            break;
        }
        msleep(WAIT_DSL_CFE_COMMAND_STATUS);
    }

    if (ix == DSL_CFE_RETRY_TIMES) {
        printf("[%s] Not Found.\n", PLAT_DSL_COMMAND_STATUS_STRING);
        fflush(stdout);
        goto exit_cfe_parms_set_failed;
    }
    msleep(WAIT_DSL_CFE_COMMAND_STATUS);

    /* Step 9 : Find "CFE>" */
    for (ix = 0; ix < DSL_CFE_RETRY_TIMES; ix++) {
        if (plat_uart_tx(tty_desc, PLAT_DSL_CR_STRING) == FAILED) {
            goto exit_cfe_parms_set_failed;
        }
        result = plat_uart_rx_polling(tty_desc, PLAT_DSL_CFE_STRING,
        		                      DSL_CFE_TIMEOUT);
        if (result == TRUE) {
            printf("Found : %s\n", PLAT_DSL_CFE_STRING);
            fflush(stdout);
            break;
        } 
    }

    if (ix == DSL_CFE_RETRY_TIMES) {
        printf("[%s] Not Found.\n", PLAT_DSL_CFE_STRING);
        fflush(stdout);
        goto exit_cfe_parms_set_failed;
    }

    msleep(WAIT_FOR_FLASH_WRITE);

    close(tty_desc);
    return (PASSED);
    
exit_cfe_parms_set_failed:
    close(tty_desc);
    return (FAILED);
}

/*******************************************************************************
 *
 * Function: xdsl_cfe_parms_set
 *
 * Description: This function set cfe parameters for kernel bootup.
 *
 * Input : os_param = IOS or Diag params
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
int xdsl_cfe_parms_set (boolean os_param)
{
    int ix;
    const int maxlen = PLAT_DSL_MAX_LENGTH;
    char tty[maxlen];
    char linux_file_str[maxlen];
    char linux_ip_str[maxlen];
    int result = PASSED;
    int tty_desc;
    char *dsl_img_filename;

    if (os_param == IOS_CFE_PARAM) { 
        prpass(testpass, "Set xdsl CFE IOS Parameters,");
    } else {
        prpass(testpass, "Set xdsl CFE DIAG Parameters,");
    }
    printf("\n");

    printf("Start CFE UART...\n");
    fflush(stdout);
    fflush(stderr);

    snprintf(tty, maxlen-1, PLAT_DSL_UART_DEV_STR);
    tty_desc = open(tty, O_RDWR | O_NOCTTY);
    if (tty_desc < 0) {
        perror("open tty failed");
        fflush(stdout);
        return (FAILED);
    }

    /* Step 0 : Catch "Press any key" */
    result = plat_uart_rx_polling(tty_desc, PLAT_DSL_PRESS_KEY_STRING,
    		                      DSL_CFE_PRESS_KEY_TIMEOUT);
    if (result != FALSE) {
        printf("Found : %s\n", PLAT_DSL_PRESS_KEY_STRING);
        fflush(stdout);
    } else {
        printf("[%s] Not Found.\n", PLAT_DSL_PRESS_KEY_STRING);
        fflush(stdout);
        goto exit_cfe_parms_set_failed;
    } 

    /* Step 1 : Find "CFE>" */
    for (ix = 0; ix < DSL_CFE_RETRY_TIMES; ix++) {
        if (plat_uart_tx(tty_desc, PLAT_DSL_CR_STRING) == FAILED) {
            goto exit_cfe_parms_set_failed;
        }
        result = plat_uart_rx_polling(tty_desc, PLAT_DSL_CFE_STRING,
        		                      DSL_CFE_TIMEOUT);
        if (result != FALSE) {
            printf("Found : %s\n", PLAT_DSL_CFE_STRING);
            fflush(stdout);
            break;
        }
    }

    if (ix == DSL_CFE_RETRY_TIMES) {
        printf("[%s] Not Found.\n", PLAT_DSL_CFE_STRING);
        fflush(stdout);
        goto exit_cfe_parms_set_failed;
    }
    msleep(WAIT_DSL_CFE_PROMPT);

    /* Step 2 : Set board parameters   */
    if (plat_uart_tx(tty_desc, PLAT_DSL_CHANGE_CFE_PARMS_STRING) == FAILED) {
        goto exit_cfe_parms_set_failed;
    }    
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Send bdpm [%s]\n", PLAT_DSL_CHANGE_CFE_PARMS_STRING);
        fflush(stdout);
    }
    if (plat_uart_tx(tty_desc, PLAT_DSL_CR_STRING) == FAILED) {
        goto exit_cfe_parms_set_failed;
    }
    msleep(WAIT_DSL_UART_TX);

    if (os_param == IOS_CFE_PARAM) { 
        sprintf(linux_ip_str, "%s.%d:ffffff00", PLAT_IOS_DSL_SUBNET_STR,
                PLAT_IOS_MODULE_IP_ADDR);
    } else {
        sprintf(linux_ip_str, "%s.%d", PLAT_DIAG_DSL_SUBNET_STR,
                PLAT_DIAG_MODULE_IP_ADDR);
    }
    /* Step 3: Set Board IP */
    if (plat_uart_tx(tty_desc, linux_ip_str) == FAILED) {
        goto exit_cfe_parms_set_failed;
    } 
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Send Board IP[%s]\n", linux_ip_str);
        fflush(stdout);
    }
    if (plat_uart_tx(tty_desc, PLAT_DSL_CR_STRING) == FAILED) {
        goto exit_cfe_parms_set_failed;
    }
    msleep(WAIT_DSL_UART_TX);

    if (os_param == IOS_CFE_PARAM) { 
        sprintf(linux_ip_str, "%s.%d", PLAT_IOS_DSL_SUBNET_STR,
                PLAT_IOS_HOST_IP_ADDR);
    } else {
        sprintf(linux_ip_str, "%s.%d", PLAT_DIAG_DSL_SUBNET_STR,
                PLAT_DIAG_HOST_IP_ADDR);
    }
    /* Step 4: Set Host IP */
    if (plat_uart_tx(tty_desc, linux_ip_str) == FAILED) {
        goto exit_cfe_parms_set_failed;
    } 
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Send Host IP[%s]\r", linux_ip_str);
        fflush(stdout);
    }
    if (plat_uart_tx(tty_desc, PLAT_DSL_CR_STRING) == FAILED) {
        goto exit_cfe_parms_set_failed;
    }
    msleep(WAIT_DSL_UART_TX);

    if (os_param == IOS_CFE_PARAM) { 
        sprintf(linux_ip_str, "%s.%d", PLAT_IOS_DSL_SUBNET_STR,
                PLAT_IOS_GATEWAY_IP_ADDR);
    } else {
        sprintf(linux_ip_str, "%s.%d", PLAT_DIAG_DSL_SUBNET_STR,
                PLAT_DIAG_GATEWAY_IP_ADDR);
    }
    /* Step 5: Set Gateway IP */
    if (plat_uart_tx(tty_desc, linux_ip_str) == FAILED) {
        goto exit_cfe_parms_set_failed;
    } 
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Send GW IP[%s]\n", linux_ip_str);
        fflush(stdout);
    }
    if (plat_uart_tx(tty_desc, PLAT_DSL_CR_STRING) == FAILED) {
        goto exit_cfe_parms_set_failed;
    }
    msleep(WAIT_DSL_UART_TX);
    
    if (FLAG_CFE_BOOT_TWO_IMG == TRUE) { 
        /* Step 6: Set where image to run from */
        if (plat_uart_tx(tty_desc, PLAT_DSL_RUN_IMAGE_LOCATION) == FAILED) {
            goto exit_cfe_parms_set_failed;
        }    
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("Send run image location[%s]\n", PLAT_DSL_RUN_IMAGE_LOCATION);
            fflush(stdout);
        }
        if (plat_uart_tx(tty_desc, PLAT_DSL_CR_STRING) == FAILED) {
            goto exit_cfe_parms_set_failed;
        }
        msleep(WAIT_DSL_UART_TX);

        /* Step 7: Set default linux file name */
        sprintf(linux_file_str, "%s", GFAST_HOST_RUN_FILE);
        if (plat_uart_tx(tty_desc, linux_file_str) == FAILED) {
            goto exit_cfe_parms_set_failed;
        }
        printf("host run image [%s]\n", linux_file_str);
        fflush(stdout);
        if (plat_uart_tx(tty_desc, PLAT_DSL_CR_STRING) == FAILED) {
            goto exit_cfe_parms_set_failed;
        }
        msleep(WAIT_DSL_UART_TX);
        /* host flash file name */ 
        if (plat_uart_tx(tty_desc, PLAT_DSL_CR_STRING) == FAILED) {
            goto exit_cfe_parms_set_failed;
        }
        msleep(WAIT_DSL_UART_TX);
        /* boot delay */
        if (plat_uart_tx(tty_desc, PLAT_DSL_CR_STRING) == FAILED) {
            goto exit_cfe_parms_set_failed;
        }
        msleep(WAIT_DSL_UART_TX);
        
        /* Step 8: Set ramdisk store address */
        sprintf(linux_file_str, "%s", GFAST_HOST_RAMDISK_FILE);
        if (plat_uart_tx(tty_desc, linux_file_str) == FAILED) {
            goto exit_cfe_parms_set_failed;
        }
        printf("host ramdisk image [%s]\n", linux_file_str);
        fflush(stdout);
        if (plat_uart_tx(tty_desc, PLAT_DSL_CR_STRING) == FAILED) {
            goto exit_cfe_parms_set_failed;
        }
        msleep(WAIT_DSL_UART_TX);

        /* Step 8: Set ramdisk store address */
        if (plat_uart_tx(tty_desc, GFAST_RAMDISK_STORE_ADDR) == FAILED) {
            goto exit_cfe_parms_set_failed;
        }    
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("Send run image location[%s]\n", GFAST_RAMDISK_STORE_ADDR);
            fflush(stdout);
        }
        if (plat_uart_tx(tty_desc, PLAT_DSL_CR_STRING) == FAILED) {
            goto exit_cfe_parms_set_failed;
        }
        msleep(WAIT_DSL_UART_TX);

    } else { /* CFE BOOT SINGLE IMG */
        /* Step 6: Set where image to run from */
        if (plat_uart_tx(tty_desc, PLAT_DSL_RUN_IMAGE_LOCATION_H) == FAILED) {
            goto exit_cfe_parms_set_failed;
        }    
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("Send run image location[%s]\n", PLAT_DSL_RUN_IMAGE_LOCATION);
            fflush(stdout);
        }

        /* Step 6: one more carrier return */
        if (plat_uart_tx(tty_desc, PLAT_DSL_CR_STRING) == FAILED) {
            goto exit_cfe_parms_set_failed;
        } 
        msleep(WAIT_DSL_UART_TX);

        /* Step 7: Set default linux file name */
        /* Check environment parameter, if not use default filename*/ 
        if (os_param == IOS_CFE_PARAM) { 
            dsl_img_filename = getenv(IOS_IMAGE_NAME);
        } else {
            dsl_img_filename = getenv(DSL_IMAGE_NAME);
        }

        if (dsl_img_filename == NULL || !strcmp("none", dsl_img_filename)) {
            if (os_param == IOS_CFE_PARAM) { 
                printf("%s: Env variable %s is not set\n", __func__, IOS_IMAGE_NAME);
            } else {
                printf("%s: Env variable %s is not set\n", __func__, DSL_IMAGE_NAME);
            }
            goto exit_cfe_parms_set_failed;
        } else {
            if (os_param == IOS_CFE_PARAM) { 
                printf("IOS Image Filename: %s\n", dsl_img_filename);
                sprintf(linux_file_str, "%s%s", 
                        PLAT_IOS_HOST_FIRMWARE_FOLDER_STRING, dsl_img_filename);
            } else {
                printf("DSL Image Filename: %s\n", dsl_img_filename);
                sprintf(linux_file_str, "%s", dsl_img_filename);
            }
        }
        
        if (plat_uart_tx(tty_desc, linux_file_str) == FAILED) {
            goto exit_cfe_parms_set_failed;
        }
        printf("Diag image [%s]\n", linux_file_str);
        fflush(stdout);
        if (plat_uart_tx(tty_desc, PLAT_DSL_CR_STRING) == FAILED) {
            goto exit_cfe_parms_set_failed;
        }
        msleep(WAIT_DSL_UART_TX);
    }  /* End of setup boot CFE with one or two images */

    if (os_param == IOS_CFE_PARAM) { 
        /* Set host flash file */
        if (plat_uart_tx(tty_desc, BCM963XX_FS_KERNEL) == FAILED) {
            goto exit_cfe_parms_set_failed;
        }    
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("Send DSL host flash image [%s]\n", BCM963XX_FS_KERNEL);
            fflush(stdout);
        }
        if (plat_uart_tx(tty_desc, PLAT_DSL_CR_STRING) == FAILED) {
            goto exit_cfe_parms_set_failed;
        } 
        msleep(WAIT_DSL_UART_TX);

        if (plat_uart_tx(tty_desc, PLAT_DSL_CR_STRING) == FAILED) {
            goto exit_cfe_parms_set_failed;
        } 
        msleep(WAIT_DSL_UART_TX);

        /* Clear ramdisk file after booting up by diag*/
        if (plat_uart_tx(tty_desc, CLEAR_PARAM) == FAILED) {
            goto exit_cfe_parms_set_failed;
        }    
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("Remove ramdisk image [%s]\n", CLEAR_PARAM);
            fflush(stdout);
        }
        if (plat_uart_tx(tty_desc, PLAT_DSL_CR_STRING) == FAILED) {
            goto exit_cfe_parms_set_failed;
        } 
        msleep(WAIT_DSL_UART_TX);
       
        if (plat_uart_tx(tty_desc, CLEAR_PARAM) == FAILED) {
            goto exit_cfe_parms_set_failed;
        }    
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("Remove ramdisk store addr [%s]\n", CLEAR_PARAM);
            fflush(stdout);
        }

        if (plat_uart_tx(tty_desc, PLAT_DSL_CR_STRING) == FAILED) {
            goto exit_cfe_parms_set_failed;
        } 
        msleep(WAIT_DSL_UART_TX);
    }

    /* Step 8 : Find "command status" */
    for (ix = 0; ix < DSL_CFE_RETRY_TIMES; ix++) {
        if (plat_uart_tx(tty_desc, PLAT_DSL_CR_STRING) == FAILED) {
            goto exit_cfe_parms_set_failed;
        }
        result = plat_uart_rx_polling(tty_desc, PLAT_DSL_COMMAND_STATUS_STRING,
        		                      DSL_CFE_COMMAND_STATUS_TIMEOUT);
        if (result != FALSE) {
            printf("Found : %s\n", PLAT_DSL_COMMAND_STATUS_STRING);
            fflush(stdout);
            break;
        }
        msleep(WAIT_DSL_CFE_COMMAND_STATUS);
    }

    if (ix == DSL_CFE_RETRY_TIMES) {
        printf("[%s] Not Found.\n", PLAT_DSL_COMMAND_STATUS_STRING);
        fflush(stdout);
        goto exit_cfe_parms_set_failed;
    }
    msleep(WAIT_DSL_CFE_COMMAND_STATUS);

    /* Step 9 : Find "CFE>" */
    for (ix = 0; ix < DSL_CFE_RETRY_TIMES; ix++) {
        if (plat_uart_tx(tty_desc, PLAT_DSL_CR_STRING) == FAILED) {
            goto exit_cfe_parms_set_failed;
        }
        result = plat_uart_rx_polling(tty_desc, PLAT_DSL_CFE_STRING,
        		                      DSL_CFE_TIMEOUT);
        if (result != FALSE) {
            printf("Found : %s\n", PLAT_DSL_CFE_STRING);
            fflush(stdout);
            break;
        } 
    }

    if (ix == DSL_CFE_RETRY_TIMES) {
        printf("[%s] Not Found.\n", PLAT_DSL_CFE_STRING);
        fflush(stdout);
        goto exit_cfe_parms_set_failed;
    }

    msleep(WAIT_FOR_FLASH_WRITE);

    close(tty_desc);
    return (PASSED);
    
exit_cfe_parms_set_failed:
    close(tty_desc);
    return (FAILED);
}

/*****************************************************************
 *
 * Function: plat_nc_dispatch_comm
 *
 * Description: This function transmits nc client request to module.
 *
 * Input:  None
 *
 * Output: None
 *
 *****************************************************************
 */
void plat_nc_dispatch_comm (char *comm, char *parms_str)
{
    char cmd[128];

    /* Sanity check */
    if (comm == NULL) {
        printf("%s: NULL Pointer\n", __FUNCTION__);
        return;
    }

    /* Prepare command and listen for module to grab */
    sprintf(cmd, "echo %s,%s, > %s",
            comm, parms_str, DIAG_PLAT_NC_COMMAND_DISPATCH_FILE);
    system(cmd);

    sprintf(cmd, "nc -l -p %d < %s &",
            DIAG_PLAT_NC_EXECUTE_COMMAND_TRANSFER_PORT_BASE,
            DIAG_PLAT_NC_COMMAND_DISPATCH_FILE);
    system(cmd);

    plat_transmit_nc_request(DIAG_PLAT_NC_EXECUTE_COMMAND_PORT_BASE);
}

/*******************************************************************************
 *
 * Function: plat_nc_dispatch_comm_is_ok
 *
 * Description: This function checks the NC response is good or not.
 *
 * Input:  None
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
int plat_nc_dispatch_comm_is_ok(void)
{
    char buff[64];

    /* First token is to indicate the result of the opcode */
    if (plat_nc_get_parms(0, buff) == FAILED) {
        return (FAILED);
    }

    if (strcmp(buff, DIAG_PLAT_NC_RTN_PASS_STR)) {
        return (FAILED);
    }

    return (PASSED);
}

/*****************************************************************
 *
 * Function: plat_transmit_nc_request
 *
 * Description: This function transmits nc client request to module
 *              on provided port number
 *
 * Input:  None
 *
 * Output: None
 *
 *****************************************************************
 */
static void plat_transmit_nc_request (int port)
{
    char cmd[32];
    char module_ipaddr[32];

    plat_get_module_ip_addr(module_ipaddr);

    sprintf(cmd, "nc %s %d", module_ipaddr, port);
    system(cmd);
}

/*****************************************************************
 *
 * Function: plat_get_module_ip_addr
 *
 * Description: This function returns IP Address of module.
 *
 * Input:  ip_addr - Buffer to put ip address
 *
 * Output: None
 *
 *****************************************************************
 */
static void plat_get_module_ip_addr (char *ip_addr)
{
    char module_ip[PLAT_NC_MAX_STR_SIZE];

    /* Sanity check */
    if (ip_addr == NULL) {
        printf("%s: NULL Pointer\n", __FUNCTION__);
        return;
    }

    sprintf(module_ip, "%s.%d", PLAT_DIAG_DSL_SUBNET_STR,
            PLAT_DIAG_MODULE_IP_ADDR);

    sprintf(ip_addr, "%s", module_ip);
}

/*-------------------------------------------------
 * $Log: diag_dsl_lib.c,v $
 * Revision 1.3  2019/05/21 07:44:19  wilbhuan
 * Add a new xDSL utility to check the SPI Flash protection.
 *
 * Revision 1.2  2019/01/10 06:36:22  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
