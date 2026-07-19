/* $Id: bcm63268_lib.c,v 1.4 2017/07/14 02:51:38 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/goldschlager/bcm63268_lib.c,v $
 *------------------------------------------------------------------------------
 *
 * bcm63268_lib.c: NC communication with bcm63268
 *
 * Oct. 2013 - James Lin
 *
 * Copyright (c) 2013-2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------------------
 */
#include <stdio.h>
#include <string.h>
#include "common.h"
#include "defs.h"
#include "types.h"
#include "menu.h"
#include "error.h"
#include "proto.h"
#include "nvsysvars.h"
#include "setjmps.h"
#include "types.h"
#include "queryflags.h"
#include "console.h"
#include "strings.h"
#include "slot.h"
#include "bcm63268_adslmib_def.h"
#include "bcm63268_lib.h"
#include "cpu.h"
#include "common_utils.h"
#include "ngwic_goldschlager_comm.h"

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

static bcm_id_string_t             bcm_mode_str[] = {
    {ANSI_T1_413_ID     ,(unsigned char *)"ANSI_T1_413"},    /* ANSI T1.413 1998 */
    {ITU_G_992_1A_ID    ,(unsigned char *)"ITU_G_992_1A"},   /* G992.1 Annex A */
    {ITU_G_992_3A_ID    ,(unsigned char *)"ITU_G_992_3A"},   /* G.992.3, Annex A */
    {ITU_G_992_5A_ID    ,(unsigned char *)"ITU_G_992_5A"},   /* G.992.5, Annex A */
    {ITU_G_992_3L_ID    ,(unsigned char *)"ITU_G_992_3L"},   /* G.992.3, Annex L */
    {ITU_G_992_3M_ID    ,(unsigned char *)"ITU_G_992_3M"},   /* G.992.3, Annex M */
    {ITU_G_992_5M_ID    ,(unsigned char *)"ITU_G_992_5M"},   /* G.992.5, Annex M */
    {ITU_G_992_1B_ID    ,(unsigned char *)"ITU_G_992_1B"},   /* G992.1 Annex B */
    {ITU_G_992_3B_ID    ,(unsigned char *)"ITU_G_992_3B"},   /* G.992.3, Annex B */
    {ITU_G_992_5B_ID    ,(unsigned char *)"ITU_G_992_5B"},   /* G.992.5, Annex B */
    {ITU_G_993_1_ID     ,(unsigned char *)"ITU_G_993_1 (VDSL1)"}, /* VDSL1 */
    {ITU_G_993_2_ID     ,(unsigned char *)"ITU_G_993_2 (VDSL2)"}, /* VDSL2 */
    {ITU_G_992_3J_ID    ,(unsigned char *)"ITU_G_992_3J"},   /* G.992.3, Annex J */
    {ITU_G_992_5J_ID    ,(unsigned char *)"ITU_G_992_5J"},   /* G.992.5, Annex J */
    {INVALID_MODE       ,(unsigned char *)"UNKNOWN"}
};

static ushort bcm_line_state_get(ngio_if *);
static ushort bcm_bonding_state_get(ngio_if *);
static int bcm_line_config_get(ngio_if *);
static int bcm63268_self_disconnect(ngio_if *);
static int bcm63268_sel_op_mode(void);
static int bcm63268_set_op_mode(ngio_if *);

boolean bcm_do_show_time_auto = TRUE;
ushort bcm_vdsl_op_mode = ITU_G_993_2_ID;
uchar bcm_showtime_duration = 2;  /* 2 seconds */
uchar *bcm_op_string = (uchar *)"UNKNOWN";
uint bcm_op_mode = 12;
uint bcm_idle_listen_params = 0;
uint bcm_channel_bonding = 0;
uint bcm_line_id = BCM_DSL_LINE_0;
extern unsigned short goldschlager_sku;

void clear (void)
{    
    while ( getchar() != '\n' );
}

/*******************************************************************************
 *
 * Function: bcm63268_check_sku_type
 *
 * Description: Check if SKU ID in GS HW and in ACT2 are identical.
 *
 * Input : iface - Points to the HWIC interface struct.
 *
 * Output: line state
 *
 *******************************************************************************
 */
int bcm63268_check_sku_type(ngio_if *iface)
{
    int gs_sku = 0;
    gs_sku = goldschlager_nc_dispatch_return_value(FRM_HOST_GET_SKU_TYPE,
                                                         bcm_op_mode,
                                                         bcm_idle_listen_params,
                                                         bcm_line_id);

    if (goldschlager_sku != gs_sku) {
        printf("Check SKU type failed"
            "Expect sku = 0x%x, Read BCM sku = 0x%x", 
            goldschlager_sku, gs_sku);
        return (FAILED);
    }

    return (PASSED);    
}

/*******************************************************************************
 *
 * Function: bcm_line_state_get
 *
 * Description: This function gets the line state from Broadcom 63268
 *
 * Input : iface - Points to the HWIC interface struct.
 *
 * Output: line state
 *
 *******************************************************************************
 */
static ushort bcm_line_state_get(ngio_if *iface)
{
    ushort line_state;
    uint line_id = bcm_line_id;

    if (!bcm_channel_bonding) {
        line_id = 0;
    }

    line_state = goldschlager_nc_dispatch_return_value(FRM_HOST_GET_CONN_INFO,
                                                         bcm_op_mode,
                                                         bcm_idle_listen_params,
                                                         line_id);

    printf("\n Line %d link state: %s \n", bcm_line_id, vdslLineStateName[line_state]);        
    return (line_state);    
}

/*******************************************************************************
 *
 * Function: bcm_bonding_state_get
 *
 * Description: This function gets bonding state from Broadcom 63268
 *
 * Input : iface - Points to the HWIC interface struct.
 *
 * Output: line state
 *
 *******************************************************************************
 */
static ushort bcm_bonding_state_get(ngio_if *iface)
{
    int rc = PASSED;

    rc = goldschlager_nc_dispatch_comm(FRM_HOST_GET_BONDING_STATE,
                                                         bcm_op_mode,
                                                         bcm_idle_listen_params,
                                                         bcm_line_id);
    if (rc != PASSED) {
        printf("\nBonding failed\n");
        return (FAILED);
    }

    return (rc);    
}
/*******************************************************************************
 *
 * Function: bcm_line_config_get
 *
 * Description: This function get the configuration from Broadcom 63268 
 *
 * Input : iface - Points to the HWIC interface struct.
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int bcm_line_config_get(ngio_if *iface)
{
    int rc = FAILED;

    rc = goldschlager_nc_dispatch_comm(FRM_HOST_GET_CONFIG_INFO,
                                       bcm_op_mode,
                                       bcm_idle_listen_params, 
                                       bcm_line_id);
    
    if (rc != PASSED) {
        printf("nc dispatch comm fail \n");
        return (rc);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function: bcm63268_self_disconnect
 *
 * Description: This function runs the showtime test
 *
 * Input : iface - Points to the HWIC interface struct.
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int bcm63268_self_disconnect(ngio_if *iface)
{
        
    char ch;
    int ix, disc_retry = 3, two_minutes_timeout = 12000;
    ushort line_state = BCM_ADSL_LINK_DOWN;
    ushort line_state0 = BCM_ADSL_LINK_DOWN;
    ushort line_state1 = BCM_ADSL_LINK_DOWN;

    if (bcm_do_show_time_auto == TRUE) {
        printf("Will self disconnect in %d seconds\n", bcm_showtime_duration);
        if (bcm63268_connection_stop(iface)) {
            cterr('f', 0, "Failed to send connection stop command");
            return (FAILED);
        }
    
        for (ix = 0; ix < disc_retry; ix++) {
            msleep(1000);
			
            if (bcm_channel_bonding) {
                bcm_line_id = BCM_DSL_LINE_0;
                line_state0 = bcm_line_state_get(iface);
                printf("\nLine%d state deactivate = 0x%08x\n",
                       bcm_line_id, line_state);

                bcm_line_id = BCM_DSL_LINE_1;
                line_state1 = bcm_line_state_get(iface);
                printf("\nLine%d state deactivate = 0x%08x\n", 
                       bcm_line_id, line_state);
            } else {
                line_state = bcm_line_state_get(iface);
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
            if (bcm63268_showtime_no_retrain(iface)) {
                return(FAILED);
            }

            bcm_line_id = BCM_DSL_LINE_1;
            if (bcm63268_showtime_no_retrain(iface)) {
                return(FAILED);
            }
        } else {
            if (bcm63268_showtime_no_retrain(iface)) {
                return(FAILED);
            }
        }

        for (ix = 0; ix < two_minutes_timeout; ix++) {
            msleep(10); /* give some time */

            if ((ix%500) == 0) {
                if (bcm_channel_bonding) {
                    bcm_line_id = BCM_DSL_LINE_0;
                    line_state0 = bcm_line_state_get(iface);
                    printf("\nLine state = 0x%08x\n", line_state0);

                    bcm_line_id = BCM_DSL_LINE_1;
                    line_state0 = bcm_line_state_get(iface);
                    printf("\nLine state = 0x%08x\n", line_state1);
                } else {
                    line_state = bcm_line_state_get(iface);
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
                if (bcm63268_connection_stop(iface)) {
                    cterr('f', 0, "Failed to send connection stop command");
                    return (FAILED);
                }
                return (PASSED);
            } else {

               if (bcm_channel_bonding) {
                    bcm_line_id = BCM_DSL_LINE_0;
                    if (bcm_line_config_get(iface)) {
                        cterr('f', 0, "Failed to get line config");
                        return (FAILED);
                    }

                    bcm_line_id = BCM_DSL_LINE_1;
                    if (bcm_line_config_get(iface)) {
                        cterr('f', 0, "Failed to get line config");
                        return (FAILED);
                    }
               } else {
                    if (bcm_line_config_get(iface)) {
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
 * Function: bcm63268_do_showtime
 *
 * Description: This function runs the showtime test
 *
 * Input : iface - Points to the HWIC interface struct.
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
int bcm63268_do_showtime(ngio_if *iface)
{
    int retry, retval = PASSED;

    retry = 1;
    while (retry) {
        retval = bcm63268_connection_start(iface);
        if (retval) {
            /* Do retry when connection fails */
            msleep(2000);
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
        bcm_line_id = BCM_DSL_LINE_0;
        bcm_line_state_get(iface);
        bcm_line_id = BCM_DSL_LINE_1;
        bcm_line_state_get(iface);

        retval = bcm_bonding_state_get(iface);
    } else {
        bcm_line_state_get(iface);
    }

    /* display the info */
    if (bcm_channel_bonding) {
        bcm63268_get_xtm_bonding_info(iface);
    } else {
        bcm63268_get_adslmib_info(iface);
    }

    return (retval);
}

/*******************************************************************************
 *
 * Function: bcm63268_showtime
 *
 * Description: This function runs the showtime test
 *
 * Input : iface - Points to the HWIC interface struct.
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
int bcm63268_showtime(ngio_if *iface)
{
    int retval = PASSED;
    
    if (iface->menu_display) {
        testname("BCM63268 Showtime Test, ");
    } else {
        prpass(testpass,"BCM63268 Showtime Test, ");
    }

    if (bcm_channel_bonding) {
        
        /***** connection start *****/
        retval = bcm63268_do_showtime(iface);
        if (retval != PASSED) {
            cterr('f', 0, "bcm63268 do showtime fail\n");
            return(retval);
        }

        /***** connection stop *****/
        retval = bcm63268_self_disconnect(iface);
        if (retval != PASSED) {
            cterr('f', 0, "bcm63268 self disconnect fail\n");
            return(retval);
        }

    } else {
        retval = bcm63268_do_showtime(iface);
        if (retval != PASSED) {
            cterr('f', 0, "bcm63268 do showtime(line id %d) fail\n", bcm_line_id);
            return(retval);
        }

        retval = bcm63268_self_disconnect(iface);
        if (retval != PASSED) {
            cterr('f', 0, "bcm63268 self disconnect(line id %d) fail\n", bcm_line_id);
            return(retval);
        }    
    }

    printf("BCM63168 do showtime completed\n");
    return (retval);
}

/*******************************************************************************
 *
 * Function: bcm63268_set_tone
 *
 * Description: Issues the set tone command to the BCM63268 processor
 *
 * Input : iface - Points to the WIC interface struct.
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
int bcm63268_set_tone (ngio_if *iface)
{
    int rc = FAILED;    

    if (iface->menu_display) {
        testname("Set BCM63268 tones");
    } else {
        prpass(testpass,"Set BCM63268 tones");
    }    

    rc = goldschlager_nc_dispatch_comm(FRM_HOST_SET_TONES,bcm_op_mode,
                                       bcm_idle_listen_params, bcm_line_id);

    if (rc != PASSED) {
        printf("nc dispatch comm fail \n");
        return (rc);
    }

    prcomplete(testpass, errcount, (char *)0);        
    
    return (rc);
}

/*******************************************************************************
 *
 * Function: bcm63268_set_test_mode
 *
 * Description: This function sets test mode for VDSL
 *
 * Input : iface - Points to the HWIC interface struct.
 *         testmode - bcm63268 vdsl phy test mode
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
int bcm63268_set_test_mode (ngio_if *iface, ushort testmode)
{
    int rc = FAILED;

    rc = goldschlager_nc_dispatch_comm(FRM_HOST_SET_TEST_MODE,bcm_op_mode,
                                       bcm_idle_listen_params, bcm_line_id);

    if (rc != PASSED) {
        printf("nc dispatch comm fail \n");
        return (rc);
    }
    
    return(PASSED);
}

/*******************************************************************************
 *
 * Function: bcm63268_send_all_tone
 *
 * Description: This function lets it transmit power spectrum so 
 *              parametric can do PSD test (should be all upstream 
 *              bandwidth)
 *
 * Input : iface - Points to the HWIC interface struct.
 *
 * Output: PASSED/FAILED.
 *
 *******************************************************************************
 */
int bcm63268_send_all_tone (ngio_if *iface)
{
    int rc = FAILED;

    if (iface->menu_display) {
        testname("BCM63268 Send All Tone, ");
    } else {
        prpass(testpass,"BCM63268 Send All Tone, ");
    }
    
    rc = goldschlager_nc_dispatch_comm(FRM_HOST_ALL_TONE,bcm_op_mode,
                                       bcm_idle_listen_params, bcm_line_id);

    if (rc != PASSED) {
        printf("bcm63268 send all tone fail \n");
        return (rc);
    }

    printf("\n\n\n\n\n            Tone generation in progress\n"); 
    printf("            Use a scope to observe the output\n\n\n");
    printf("\nAllow 5 seconds delays\n");

    msleep(SEND_ALL_TONE_DELAY);

    while (1) {
        printf("Then hit 'e' for quit\n");
        if ('e' == getchar()) {
            if (bcm63268_set_test_mode (iface, ADSL_TEST_NORMAL)) {
                return (FAILED);
            }
            break;
        }
        clear();
    }
    
    bcm63268_get_version(iface);
    
    bcm63268_get_configure(iface);

    return (PASSED);
} 

/*******************************************************************************
 *
 * Function: bcm63268_get_idle_listen_result
 *
 * Description: This function set the chip set into idle or quiet mode
 *
 * Input : iface - Points to the HWIC interface struct.
 *
 * Output: PASSED/FAILED.
 *
 *******************************************************************************
 */
int bcm63268_get_idle_listen_result (ngio_if *iface)
{
    int rc = FAILED;

    rc = goldschlager_nc_dispatch_comm(FRM_HOST_PRINT_IDLE_LISTEN,bcm_op_mode,
                                       bcm_idle_listen_params, bcm_line_id);

    if (rc != PASSED) {
        printf("nc dispatch comm fail \n");
        return (rc);
    }
    
    return (PASSED);
}   

/*******************************************************************************
 *
 * Function: bcm63268_idle_listen
 *
 * Description: This function set the chip set into idle or quiet mode
 *
 * Input : iface - Points to the HWIC interface struct.
 *
 * Output: PASSED/FAILED.
 *
 *******************************************************************************
 */
int bcm63268_idle_listen (ngio_if *iface)
{
    int i, rc = FAILED;

    if (iface->menu_display) {
        testname("BCM63268 Idle Listen, ");
    } else {
        prpass(testpass,"BCM63268 Idle Listen, ");
    }
    
    rc = goldschlager_nc_dispatch_comm(FRM_HOST_SET_IDLE_LISTEN,bcm_op_mode,
                                       bcm_idle_listen_params, bcm_line_id);

    if (rc != PASSED) {
        printf("bcm63268 idle listen fail \n");
        return (rc);
    }
    
    printf("Please generate the signal at Tip & Ring\n");
    
    do {
        printf("Then hit any key to start listening\n");
        getchar();
        clear();
        printf("Please allow 25 seconds delay.\n");
        for(i = 0; i < 25; i++) {
            printf(" .");
            msleep(1000);
        }    
        printf("\n");

        msleep(1000);

        /* get idle listen result from bcm and print it */
        if (bcm63268_get_idle_listen_result(iface)) {
            return (FAILED);
        }
        if (bcm63268_set_test_mode (iface, ADSL_TEST_NORMAL)) {
            return (FAILED);
        }
    } while (0);
    
    return (PASSED);
}    

/*******************************************************************************
 *
 * Function: bcm63268_reset_statcounters
 *
 * Description: This function reset vdsl2 counters
 *
 * Input : iface - Points to the HWIC interface struct.
 *
 * Output: PASSED/FAILED.
 *
 *******************************************************************************
 */
int bcm63268_reset_statcounters (ngio_if *iface)
{
    int rc = FAILED;
    
    rc = goldschlager_nc_dispatch_comm(FRM_HOST_RESET_STATCOUNTERS,bcm_op_mode,
                                       bcm_idle_listen_params, bcm_line_id);

    if (rc != PASSED) {
        printf("nc dispatch comm fail \n");
        return (rc);
    }
    
    return (PASSED);    
}

/*******************************************************************************
 *
 * Function: bcm63268_get_xdsl_info
 *
 * Description: This function get xdsl mib object
 *
 * Input : iface - Points to the HWIC interface struct.
 *
 * Output: PASSED/FAILED.
 *
 *******************************************************************************
 */
int bcm63268_get_xdsl_info (ngio_if *iface)
{
    uint op_mode, ori_op_mode;
    int rc = FAILED;
    
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

    rc = goldschlager_nc_dispatch_comm(FRM_HOST_GET_XDSL_INFO,bcm_op_mode,
                                       bcm_idle_listen_params, bcm_line_id);

    if (rc != PASSED) {
        printf("nc dispatch comm fail \n");
        return (rc);
    }

    bcm_op_mode = ori_op_mode;

    return (PASSED);    
}

/*******************************************************************************
 *
 * Function: bcm63268_get_adslmib_info
 *
 * Description: This function prints xdsl mib object info
 *
 * Input : iface - Points to the NGWIC interface struct.
 *
 * Output: PASSED/FAILED.
 *
 *******************************************************************************
 */
int bcm63268_get_adslmib_info (ngio_if *iface)
{
    int rc = FAILED;
     
    rc = goldschlager_nc_dispatch_comm(FRM_HOST_GET_ADSLMIB_INFO,bcm_op_mode,
                                       bcm_idle_listen_params, bcm_line_id);

    if (rc != PASSED) {
        printf("nc dispatch comm fail \n");
        return (rc);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function: bcm63268_get_xtm_bonding_info
 *
 * Description: This function get xtm bonnding info
 *
 * Input : iface - Points to the NGWIC interface struct.
 *
 * Output: PASSED/FAILED.
 *
 *******************************************************************************
 */
int bcm63268_get_xtm_bonding_info (ngio_if *iface)
{
    int rc = FAILED;
     
    rc = goldschlager_nc_dispatch_comm(FRM_HOST_GET_XTM_BONDING_INFO,bcm_op_mode,
                                       bcm_idle_listen_params, bcm_line_id);

    if (rc != PASSED) {
        printf("nc dispatch comm fail \n");
        return (rc);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function: bcm63268_sel_op_mode
 *
 * Description: This function select BCM63268 xDSL mode
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int bcm63268_sel_op_mode(void)
{
    uint i, op_mode;

    printf("\n1. ANSI_T1_413_ID (ANSI T1.413 1998)");
    if ((goldschlager_sku == GS_NIM_VAB_A) ||
        (goldschlager_sku == GS_NIM_VA_B) ||
        (goldschlager_sku == GS_NIM_VAB_M)) {
        printf("\n2. ITU_G_992_1A_ID (G.992.1 Annex A)");
        printf("\n3. ITU_G_992_3A_ID (G.992.3, Annex A)");
        printf("\n4. ITU_G_992_5A_ID (G.992.5, Annex A)");
        printf("\n5. ITU_G_992_3L_ID (G.992.3, Annex L)");
        printf("\n6. ITU_G_992_3M_ID (G.992.3, Annex M)");
        printf("\n7. ITU_G_992_5M_ID (G.992.5, Annex M)");
        printf("\n8. ITU_G_992_1B_ID (G992.1 Annex B)");
        printf("\n9. ITU_G_992_3B_ID (G.992.3, Annex B)");
        printf("\n10. ITU_G_992_5B_ID (G.992.5, Annex B)");
    }

    printf("\n11. ITU_G_993_1 (VDSL1)");
    printf("\n12. ITU_G_993_2 (VDSL2)");
    if (goldschlager_sku == GS_NIM_VA_B) {
        printf("\n13. ITU_G_992_3J_ID (G.992.3, Annex J)");
        printf("\n14. ITU_G_992_5J_ID (G.992.5, Annex J)");
        printf("\n15. ITU_G_992_3J_ID for Alcatel DSLAM (G.992.3, Annex J)");
        printf("\n16. ITU_G_992_5J_ID for Alcatel DSLAM (G.992.5, Annex J)");
    }
  
    op_mode = getdec_answer("\nSelect Mode", 12, 1, 16);
    printf("\n");

    bcm_op_mode = op_mode;
    
    switch (op_mode) {
    case 1:
        bcm_vdsl_op_mode = ANSI_T1_413_ID;
        break;
    case 2:
        bcm_vdsl_op_mode = ITU_G_992_1A_ID;
        break;
    case 3:
        bcm_vdsl_op_mode = ITU_G_992_3A_ID;
        break;
    case 4:
        bcm_vdsl_op_mode = ITU_G_992_5A_ID;
        break;
    case 5:
        bcm_vdsl_op_mode = ITU_G_992_3L_ID;
        break;
    case 6:
        bcm_vdsl_op_mode = ITU_G_992_3M_ID;
        break;
    case 7:
        bcm_vdsl_op_mode = ITU_G_992_5M_ID;
        break;   
    case 8:
        bcm_vdsl_op_mode = ITU_G_992_1B_ID;
        break;   
    case 9:
        bcm_vdsl_op_mode = ITU_G_992_3B_ID;
        break;
    case 10:
        bcm_vdsl_op_mode = ITU_G_992_5B_ID;
        break;
    case 11:
        bcm_vdsl_op_mode = ITU_G_993_1_ID;
        break;    
    case 12:
        bcm_vdsl_op_mode = ITU_G_993_2_ID;
        break;
    case 13:
        bcm_vdsl_op_mode = ITU_G_992_3J_ID;
        break;
    case 14:
        bcm_vdsl_op_mode = ITU_G_992_5J_ID;
        break;    
    case 15:
        bcm_vdsl_op_mode = ITU_G_992_3J_ID;
        break;
    case 16:
        bcm_vdsl_op_mode = ITU_G_992_5J_ID;
        break;    
    default:  /* Default mode : VDSL2 */
        bcm_vdsl_op_mode = ITU_G_993_2_ID;
        bcm_op_mode = 12;
        break;
    }

    for (i = 0; i < (sizeof(bcm_mode_str)/sizeof(bcm_id_string_t)); i++) {
        if (bcm_vdsl_op_mode == bcm_mode_str[i].code) {
            break;
        }
    }

    bcm_op_string = bcm_mode_str[i].name;
    
    return (PASSED);
}

/*******************************************************************************
 *
 * Function: bcm63268_set_op_mode
 *
 * Description: This function configures and shows status of BCM63268 chip
 *
 * Input : iface - Points to the HWIC interface struct.
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int bcm63268_set_op_mode(ngio_if *iface)
{
    uint rc = PASSED;

    if ((goldschlager_sku == GS_NIM_VAB_A) ||
        (goldschlager_sku == GS_NIM_VA_B) ||
        (goldschlager_sku == GS_NIM_VAB_M)) {
        
        rc = goldschlager_nc_dispatch_comm(FRM_HOST_SET_LINE_MODE,bcm_op_mode,
                                           bcm_idle_listen_params, bcm_line_id);

        if (rc != PASSED) {
            printf("bcm63268 set test mode fail\n");
            return (FAILED);
        }

    } else {
        printf("\nThe SKU(0x%x) is not supported.", goldschlager_sku);
        return (FAILED);
    }

    if (bcm63268_get_line_mode(iface)) {
        return (FAILED);
    }
    
    return (PASSED);
}

/*******************************************************************************
 *
 * Function: bcm63268_initialize
 *
 * Description: Issues the Initialize command to the BCM63268 processor
 *
 * Input : iface - Points to the HWIC interface struct.
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
int bcm63268_initialize (ngio_if *iface)
{
    int rc = FAILED;

    if (iface->menu_display) {
        testname("BCM63268 Initialize test, ");
    } else {
        prpass(testpass,"BCM63268 Initialize test, ");
    }   

    rc = goldschlager_nc_dispatch_comm(FRM_HOST_INIT_BCM63268,bcm_op_mode,
                                                      bcm_idle_listen_params, bcm_line_id);

    if (rc != PASSED) {
        printf("nc dispatch comm fail \n");
        return (rc);
    }

     return(PASSED);
}

/*******************************************************************************
 *
 * Function: bcm63268_uninitialize
 *
 * Description: Issues the Un-Initialize command to the BCM63268 processor
 *
 * Input : iface - Points to the HWIC interface struct.
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
int bcm63268_uninitialize (ngio_if *iface)
{
    int rc = FAILED;

    if (iface->menu_display) {
        testname("BCM63268 uninitialize test");
    } else {
        prpass(testpass,"BCM63268 uninitialize test");
    }
    
    rc = goldschlager_nc_dispatch_comm(FRM_HOST_UNINIT_BCM63268,bcm_op_mode,
                                                      bcm_idle_listen_params, bcm_line_id);

    if (rc != PASSED) {
        printf("nc dispatch comm fail \n");
        return (rc);
    }

     return(PASSED);
}

/*******************************************************************************
 *
 * Function: bcm63268_configure
 *
 * Description: Issues the Configure command to the BCM63268 processor
 *
 * Input : iface - Points to the HWIC interface struct.
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
int bcm63268_configure (ngio_if *iface)
{
    int rc = FAILED;
    
    if (iface->menu_display) {
        testname("BCM63268 Confiure test");
    } else {
        prpass(testpass,"BCM63268 Confiure test");
    }  
    
    rc = goldschlager_nc_dispatch_comm(FRM_HOST_CONFIG_BCM63268,bcm_op_mode,
                                       bcm_idle_listen_params, bcm_line_id);

    if (rc != PASSED) {
        printf("nc dispatch comm fail \n");
        return (rc);
    }

    return(PASSED);
}

/*******************************************************************************
 *
 * Function: bcm63268_get_configure
 *
 * Description: Issues the GetConfiguration command to the BCM63268 processor
 *
 * Input : iface - Points to the HWIC interface struct.
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
int bcm63268_get_configure (ngio_if *iface)
{
    int rc = FAILED;

    if (iface->menu_display) {
        testname("BCM63268 Get Configuration, ");
    } else {
        prpass(testpass,"BCM63268 Get Configuration, ");
    } 
    
    rc = goldschlager_nc_dispatch_comm(FRM_HOST_GET_CONFIG_INFO,bcm_op_mode,
                                       bcm_idle_listen_params, bcm_line_id);
    
    if (rc != PASSED) {
        printf("nc dispatch comm fail \n");
        return (rc);
    }

    return(PASSED);
}

/*******************************************************************************
 *
 * Function: bcm63268_get_version
 *
 * Description: Issues the GetVersion command to the BCM63268 processor
 *
 * Input : iface - Points to the HWIC interface struct.
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
int bcm63268_get_version (ngio_if *iface)
{
    int rc = FAILED;
    
    if (iface->menu_display) {
        testname("BCM63268 Get Version, ");
    } else {
        prpass(testpass,"BCM63268 Get Version, ");
    }     
    
    rc = goldschlager_nc_dispatch_comm(FRM_HOST_GET_VERSION,bcm_op_mode,
                                       bcm_idle_listen_params, bcm_line_id);
    
    if (rc != PASSED) {
        printf("nc dispatch comm fail \n");
        return (rc);
    }

    return(PASSED);
}

/*******************************************************************************
 *
 * Function: bcm63268_show_profile
 *
 * Description: Issues the show profile command to the BCM63268 processor
 *
 * Input : iface - Points to the HWIC interface struct.
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
int bcm63268_show_profile (ngio_if *iface)
{
    int rc = FAILED;
    
    if (iface->menu_display) {
        testname("BCM63268 Show Profile, ");
    } else {
        prpass(testpass,"BCM63268 Show Profile, ");
    }     
    
    rc = goldschlager_nc_dispatch_comm(DIAG_COMMAND_BCM63268_SHOW_PROFILE,bcm_op_mode,
                                       bcm_idle_listen_params, bcm_line_id);
    
    if (rc != PASSED) {
        printf("nc dispatch comm fail \n");
        return (rc);
    }

    return(PASSED);
}

/*******************************************************************************
 *
 * Function: bcm63268_show_spi_flash_reg
 *
 * Description: Issues the show spi flash reg command to the BCM63268 processor
 *
 * Input : iface - Points to the HWIC interface struct.
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
int bcm63268_show_spi_flash_reg (ngio_if *iface)
{
    int rc = FAILED;

    if (iface->menu_display) {
        testname("BCM63268 Show SPI Flash Registers, ");
    } else {
        prpass(testpass,"BCM63268 Show SPI Flash Registers, ");
    }

    rc = goldschlager_nc_dispatch_comm(DIAG_COMMAND_BCM63268_SHOW_SPI_FLASH_REG,
    		                           bcm_op_mode, bcm_idle_listen_params,
    		                           bcm_line_id);

    if (rc != PASSED) {
        printf("nc dispatch comm fail \n");
        return (rc);
    }

    return(PASSED);
}

/*******************************************************************************
 *
 * Function: bcm63268_connection_start
 *
 * This function sets the connection to the DSLAM
 *
 * Input : iface - Points to the HWIC interface struct.
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
int bcm63268_connection_start (ngio_if *iface)
{
    int i, rc = FAILED;
    ushort line_state = BCM_ADSL_LINK_DOWN;
    ushort line_state0 = BCM_ADSL_LINK_DOWN;
    ushort line_state1 = BCM_ADSL_LINK_DOWN;	
    
    if (iface->menu_display) {
        testname("BCM63268 VDSL Connection Start, ");
    } else {
        prpass(testpass,"BCM63268 VDSL Connection Start, ");
    }
    
    if (bcm_channel_bonding) {
        bcm_line_id = BCM_DSL_LINE_BONDING;
    }

    rc = goldschlager_nc_dispatch_comm(FRM_HOST_CONN_START,bcm_op_mode,
                                       bcm_idle_listen_params, bcm_line_id);

    if (rc != PASSED) {
        printf("nc dispatch comm fail \n");
        return (rc);
    }

    for (i = 0; i <= 18000; i++) {
        msleep(10); /* give some time */

        /* get the link status in every 5 sec */
        if ((i%500) == 0) {
            if (bcm_channel_bonding) {
                bcm_line_id = BCM_DSL_LINE_0;
                line_state0 = bcm_line_state_get(iface);
                bcm_line_id = BCM_DSL_LINE_1;
                line_state1 = bcm_line_state_get(iface);
            } else {
                line_state = bcm_line_state_get(iface);
            }
        }
        /* After 1 or 2 minutes if the state is link down then stop, otherwise
           give it 1 more minute, up to 3 minutes maximum */
        if (bcm_channel_bonding) {
            if ((i >= 12000) && (line_state0 == BCM_ADSL_LINK_UP)
                                 && (line_state1 == BCM_ADSL_LINK_UP)) {
                break;
            }
        } else {
            if ((i >= 6000) && (line_state == BCM_ADSL_LINK_UP)) {
                break;
            }
        }
    }

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
 * Function: bcm63268_connection_stop
 *
 * Description: Issues Connection Stop command to the BCM63268 processor 
 *
 * Input : iface - Points to the HWIC interface struct.
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
int bcm63268_connection_stop (ngio_if *iface)
{
    int rc = FAILED;
    int line_state = 0;
    if (iface->menu_display) {
        testname("BCM63268 Connection Stop, ");
    } else {
        prpass(testpass,"BCM63268 Connection Stop, ");
    }

    rc = goldschlager_nc_dispatch_comm(FRM_HOST_CONN_STOP,bcm_op_mode,
                                       bcm_idle_listen_params, bcm_line_id);
    
    if (rc != PASSED) {
        printf("nc dispatch comm fail \n");
        return (rc);
    }

    line_state = bcm_line_state_get (iface);

    if (line_state != BCM_ADSL_LINK_DOWN) {
        return (FAILED);
    }

    return(PASSED);
}

/*******************************************************************************
 *
 * Function: bcm63268_showtime_no_retrain
 *
 * Description: Issues no retrain command to the BCM63268 processor 
 *
 * Input : iface - Points to the HWIC interface struct.
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
int bcm63268_showtime_no_retrain(ngio_if *iface)
{
    int rc = FAILED;
    
    rc = goldschlager_nc_dispatch_comm(FRM_HOST_SHOWTIME_CONT,bcm_op_mode,
                                       bcm_idle_listen_params, bcm_line_id);
    
    if (rc != PASSED) {
        printf("nc dispatch comm fail \n");
        return (rc);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function: bcm63268_get_conn_info
 *
 * Description: Issues the Get Connection Info command to the BCM63268
 *
 * Input : iface  - Points to the HWIC interface struct.
 * 
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
int bcm63268_get_conn_info (ngio_if *iface)
{
    if (iface->menu_display) {
        testname("Get BCM63268 Connection Info, ");
    } else {
        prpass(testpass,"Get BCM63268 Connection Info, ");
    }   
    
    goldschlager_nc_dispatch_return_value(FRM_HOST_GET_CONN_INFO,
                                          bcm_op_mode,
                                          bcm_idle_listen_params,
                                          bcm_line_id);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function: bcm63268_volt_normal
 *
 * Description: Issues set voltage normal command to the BCM63268
 *
 * Input : iface  - Points to the HWIC interface struct.
 * 
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
int bcm63268_volt_normal (ngio_if *iface)
{
    int rc = FAILED;
    
    rc = goldschlager_nc_dispatch_comm(DIAG_COMMAND_BCM63268_VOLT_NORMAL,
                                       bcm_op_mode,
                                       bcm_idle_listen_params, 
                                       bcm_line_id);
    
    if (rc != PASSED) {
        printf("nc dispatch comm fail \n");
        return (rc);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function: bcm63268_volt_high
 *
 * Description: Issues set voltage high command to the BCM63268
 *
 * Input : iface  - Points to the HWIC interface struct.
 * 
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
int bcm63268_volt_high (ngio_if *iface)
{
    int rc = FAILED;
    
    rc = goldschlager_nc_dispatch_comm(DIAG_COMMAND_BCM63268_VOLT_HIGH,
                                       bcm_op_mode,
                                       bcm_idle_listen_params, 
                                       bcm_line_id);
    
    if (rc != PASSED) {
        printf("nc dispatch comm fail \n");
        return (rc);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function: bcm63268_volt_low
 *
 * Description: Issues set voltage low command to the BCM63268
 *
 * Input : iface  - Points to the HWIC interface struct.
 * 
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
int bcm63268_volt_low (ngio_if *iface)
{
    int rc = FAILED;
    
    rc = goldschlager_nc_dispatch_comm(DIAG_COMMAND_BCM63268_VOLT_LOW,
                                       bcm_op_mode,
                                       bcm_idle_listen_params, 
                                       bcm_line_id);
    
    if (rc != PASSED) {
        printf("nc dispatch comm fail \n");
        return (rc);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function: bcm63268_led_test
 *
 * Description: Issues LED test command to the BCM63268 processor
 *
 * Input : iface - Points to the HWIC interface struct.
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
int bcm63268_led_test (ngio_if *iface)
{
    int rc = FAILED;
    
    if (iface->menu_display) {
        testname("BCM63268 Led Test, ");
    } else {
        prpass(testpass,"BCM63268 Led Test, ");
    }     
    
    rc = goldschlager_nc_dispatch_comm(DIAG_COMMAND_BCM63268_LED_TEST,
                                       bcm_op_mode,
                                       bcm_idle_listen_params, 
                                       bcm_line_id);
    
    if (rc != PASSED) {
        printf("nc dispatch comm fail \n");
        return (rc);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function: bcm63268_get_line_mode
 *
 * Description: Issues the command to the BCM63268 processor to get
 *              the line state
 *
 * Input : iface - Points to the HWIC interface struct.
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
int bcm63268_get_line_mode (ngio_if *iface)
{
    if (iface->menu_display) {
        testname("BCM 63268 get line mode");
    } else {
        prpass(testpass,"BCM 63268 get line mode");
    }
    
    goldschlager_nc_dispatch_comm(FRM_HOST_GET_LINE_MODE,
                                  bcm_op_mode,
                                  bcm_idle_listen_params, 
                                  bcm_line_id);

    return (PASSED);
}

/*******************************************************************************
 *
 * Function: bcm63268_vdsl_test_option_select
 *
 * Description: This function allows to select different test options
 *
 * Input : iface - Points to the HWIC interface struct.
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
int bcm63268_vdsl_test_option_select (ngio_if *iface)
{
    uchar ch;
    uchar showtime_auto;
    ushort line_id, channel_bonding,idle_params;

    while (1) {
        printf("\nBroadcom 63268 Test Configuration Menu\n");

        showtime_auto = (bcm_do_show_time_auto ? 'y':'n');
        idle_params = (bcm_idle_listen_params ? 'y':'n');
        channel_bonding = (bcm_channel_bonding ? 'y':'n');
        line_id = (bcm_line_id ? 1:0);
        
        printf("\n1. Perform auto disconnect in show time?[y/n]: %c", showtime_auto);
        printf("\n2. Select xDSL mode: %s", bcm_op_string);
        printf("\n3. Set parameters for Idle Listen?[y/n]: %c", idle_params);
        printf("\n4. Set channel blnding?[y/n]: %c", channel_bonding);
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
            if (bcm63268_sel_op_mode() == FAILED) {
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
            if (bcm63268_set_op_mode(iface) == FAILED) {
                return (FAILED);
            }
            msleep(100); /* For the chip to reconfigure */
            return (PASSED);
        default:
            printf("\nInvalid input\n");
            break;
        }
        clear();
    }
    return (PASSED);

}

/******** History ********/
/*------------------------------------------------------------------------------
 * $Log: bcm63268_lib.c,v $
 * Revision 1.4  2017/07/14 02:51:38  alpeng
 * fixed compiler warning, due to cross-compiler version was updated.
 *
 * Revision 1.3  2015/02/13 12:26:49  meho
 * Added the utility to read Boardcom SPI flash registers
 *
 * Revision 1.2  2014/09/17 03:32:16  jamlin
 * Add support for Goldschlager NIM.
 *
 * Revision 1.1.6.2  2014/08/08 02:43:57  jamlin
 * goladschlager-branch3 initail commit.
 *
 * Revision 1.1.4.7  2014/04/11 03:50:04  jamlin
 * GS annexB PID changes from NIM-VAB-B tp NIM-VA-B
 *
 * Revision 1.1.4.6  2014/04/08 13:12:31  jamlin
 * Checkin enhanced error message.
 *
 * Revision 1.1.4.5  2014/02/10 04:17:03  jamlin
 * added get_xdsl_profile function
 *
 * Revision 1.1.4.4  2014/02/10 03:59:20  jamlin
 * rename nc_dispatch_linkstatus to nc_dispatch_return_value function and added check_sku_type function
 *
 * Revision 1.1.4.3  2014/02/10 03:32:21  jamlin
 * added bcm_bonding_state_get function and fixed showtime bonding issue
 *
 * Revision 1.1.4.2  2014/01/07 01:54:52  jamlin
 * Goldschlager new branch goldschlager-branch2
 *
 * Revision 1.1.2.5  2014/01/03 07:39:46  jamlin
 * Increase wait time to 25 sec in do idle listen util.
 *
 * Revision 1.1.2.4  2013/12/04 01:38:52  jamlin
 * Support Bonding channels showtime status display.
 *
 * Revision 1.1.2.3  2013/11/22 14:09:37  jamlin
 * optimize test options in parametric test.
 *
 * Revision 1.1.2.2  2013/11/22 12:50:21  jamlin
 * Add PID check to differentiate GS SKUs.
 *
 * Revision 1.1.2.1  2013/11/02 13:39:51  jamlin
 * Initial commit for bringup.
 *
 *
 *------------------------------------------------------------------------------
 * $Endlog$
 *------------------------------------------------------------------------------
 */
