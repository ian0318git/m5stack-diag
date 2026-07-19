/* $Id: diag_led_util.c,v 1.7 2020/02/04 08:49:42 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nutella/diag_led_util.c,v $
 *------------------------------------------------------------------
 *
 * diag_led_util.c - This file is for LED utility 
 *
 *
 * Copyright (c) 2008-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/io.h>
#include <linux/types.h>
#include <linux/sockios.h>
#include <sys/ioctl.h>
#include <linux/mii.h>
#include <linux/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <linux/if_ether.h>
#include "endians.h"
#include "defs.h"
#include "types.h"
#include "nvsysvars.h"
#include "error.h"
#include "pcmap.h"
#include "strings.h"
#include "menu.h"
#include "common.h"
#include "cross_platform.h"
#include "mb_tests.h"
#include "plat_defs.h"
#include "setjmps.h"
#include "proto.h"
#include "diag_fpga.h"
#include "diag_fpga_lib.h"
#include "platform_fru.h"
#include "platform_cookie.h"
#include "plat_defs.h"
#include "dnv_eth_lib.h"
#include "diag_gephy_lib.h"
#include "diag_led_test.h"
#include "diag_i350_test.h"

/* Local functions */
int diag_sys_led_util(void);
int diag_vpn_led_util(void);
int diag_lte_rssi_led_util(void);
int diag_lte_sim_led_util(void);
int diag_gephy_led_util(int);
int diag_i350_led_util(void);
int diag_i350_led_on_off(int, boolean);
int diag_port80_led_util(void);
int diag_all_led_on_util(int);

/******************************************************************************
 *
 * Function: diag_sys_led_util
 *
 * Description: System LED utility
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_sys_led_util (void)
{
    uint32_t test_mode = 0;
    int rc = FAILED, on = 1, off = 0;

    while (1) {
        printf("System LED Supported Mode:\n");
        printf("[0] Turn Green on.\n");
        printf("[1] Turn Amber on.\n");
        printf("[2] Turn off.\n");
        printf("[f] Leave the utility.\n");
        test_mode = gethex_answer("Enter LED mode: ", 0, 0, 0xf);

        if (fpga_version_is_more_than_2p0() == FALSE) {
            if (test_mode == 0) {
                /* Turn on system led with green */
                rc = fpga_register_operation(FPGA_LED_REG, SYS_OK_LED_GREEN, on);

            } else if (test_mode == 1) {
                /* Turn on system led with amber */
                rc = fpga_register_operation(FPGA_LED_REG, SYS_OK_LED_AMBER, on);

            } else if (test_mode == 2) {
                /* Turn off all led */
                rc = fpga_register_operation(FPGA_LED_REG, LED_OFF, off);

            } else if (test_mode == 0xf) {
                break;
            } else {
                printf("Wrong test mode!\n");
            }
        } else {
            if (test_mode == 0) {
                /* Turn on system led with green */
                rc = fpga_register_operation(CEDGE_LPC_STATUS_LED_REG,
                                             CEDGE_SYS_OK_LED_GREEN, on);

            } else if (test_mode == 1) {
                /* Turn on system led with amber */
                rc = fpga_register_operation(CEDGE_LPC_STATUS_LED_REG,
                                             CEDGE_SYS_OK_LED_AMBER, on);

            } else if (test_mode == 2) {
                /* Turn off all led */
                rc = fpga_register_operation(CEDGE_LPC_STATUS_LED_REG, LED_OFF, off);

            } else if (test_mode == 0xf) {
                break;
            } else {
                printf("Wrong test mode!\n");
            }

        }
    }
    return (rc);
}

/******************************************************************************
 *
 * Function: diag_lte_rssi_led_util
 *
 * Description: Utility of LTE RSSI LED 
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_lte_rssi_led_util (void)
{
    uint32_t test_mode = 0;
    int rc = FAILED, on = 1;

    while (1) {
        printf("LTE RSSI LED Supported Mode:\n");
        printf("[0] Turn LED with high rssi \t- Green steady on.\n");
        printf("[1] Turn LED with medium rssi \t- Green yellow blink.\n");
        printf("[2] Turn LED with low rssi \t- Yellow steady on.\n");
        printf("[3] Turn LED with weak rssi \t- Yellow blink.\n");
        printf("[4] Turn LED with no rssi \t- LED off.\n");
        printf("[f] Leave the utility.\n");
        test_mode = gethex_answer("Enter LED mode: ", 0, 0, 0xf);

        if (test_mode == 0) {
            /* Turn on LTE Modem RSSI with high RSSI */
            rc = fpga_register_operation(FPGA_LTE_RSSI_LED, LTE_MOD_HIGH_RSSI, on);

        } else if (test_mode == 1) {
            /* Turn on LTE Modem RSSI with medium RSSI */
            rc = fpga_register_operation(FPGA_LTE_RSSI_LED, LTE_MOD_MEDIUM_RSSI, on);

        } else if (test_mode == 2) {
            /* Turn on LTE Modem RSSI with low RSSI */
            rc = fpga_register_operation(FPGA_LTE_RSSI_LED, LTE_MOD_LOW_RSSI, on);

        } else if (test_mode == 3) {
            /* Turn on LTE Modem RSSI with weak RSSI */
            rc = fpga_register_operation(FPGA_LTE_RSSI_LED, LTE_MOD_RSSI, on);

        } else if (test_mode == 4) {
            /* Turn on LTE Modem RSSI with no RSSI */
            rc = fpga_register_operation(FPGA_LTE_RSSI_LED, LTE_MOD_NO_RSSI, on);

        } else if (test_mode == 0xf) {
            break;
        } else {
            printf("Wrong test mode!\n");
        }
    }
    return (rc);
}

/******************************************************************************
 *
 * Function: diag_lte_sim_led_util
 *
 * Description:  Utility of LTE SIM card LED
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_lte_sim_led_util (void)
{
    uint32_t test_mode = 0;
    int rc = FAILED, on = 1, off = 0;

    while (1) {
        printf("LTE SIM LED Supported Mode:\n");
        printf("[0] Turn Green on.\n");
        printf("[1] Turn off.\n");
        printf("[f] Leave the utility.\n");
        test_mode = gethex_answer("Enter LED mode: ", 0, 0, 0xf);

        if (test_mode == 0) {
            /* Turn on LTE SIM card LED*/
            rc = fpga_register_operation(FPGA_LED_REG, LTE_SIM_ACT_LED, on);

        } else if (test_mode == 1) {
            /* Turn off all led */
            rc = fpga_register_operation(FPGA_LED_REG, LED_OFF, off);

        } else if (test_mode == 0xf) {
            break;
        } else {
            printf("Wrong test mode!\n");
        }
    }
    return (rc);
}

/******************************************************************************
 *
 * Function: diag_vpn_led_util
 *
 * Description: Utility of VPN LED
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_vpn_led_util (void)
{
    uint32_t test_mode = 0;
    int rc = FAILED, on = 1, off = 0;

    while (1) {
        printf("VPN LED Supported Mode:\n");
        printf("[0] Turn Green on.\n");
        printf("[1] Turn Yellow on.\n");
        printf("[2] Turn off.\n");
        printf("[f] Leave the utility.\n");
        test_mode = gethex_answer("Enter LED mode: ", 0, 0, 0xf);

        if (fpga_version_is_more_than_2p0() == FALSE) {
            if (test_mode == 0) {
                /* Turn on vpn led with green*/
                rc = fpga_register_operation(FPGA_LED_REG, VPN_OK_LED_GREEN, on);

            } else if (test_mode == 1) {
                /* Turn on vpn led with amber*/
                rc = fpga_register_operation(FPGA_LED_REG, VPN_OK_LED_YELLOW, on);

            } else if (test_mode == 2) {
                /* Turn off all led*/
                rc = fpga_register_operation(FPGA_LED_REG, LED_OFF, off);
            
            } else if (test_mode == 0xf) {
                break;
            } else {
                printf("Wrong test mode!\n");
            }
        } else {
            if (test_mode == 0) {
                /* Turn on vpn led with green*/
                rc = fpga_register_operation(CEDGE_LPC_STATUS_LED_REG, VPN_OK_LED_GREEN, on);

            } else if (test_mode == 1) {
                /* Turn on vpn led with amber*/
                rc = fpga_register_operation(CEDGE_LPC_STATUS_LED_REG, VPN_OK_LED_YELLOW, on);

            } else if (test_mode == 2) {
                /* Turn off all led*/
                rc = fpga_register_operation(CEDGE_LPC_STATUS_LED_REG, LED_OFF, off);
            
            } else if (test_mode == 0xf) {
                break;
            } else {
                printf("Wrong test mode!\n");
            }

        }
    }
    return (rc);
}

/******************************************************************************
 *
 * Function: diag_gephy_led_util
 *
 * Description: Utility of gephy LED
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_gephy_led_util (int dummy)
{
    dev_88e1543_object_t gephy_obj;
    dev_88e1543_object_t *gephy_obj_p = &gephy_obj;
    int rc;
    uint32_t test_mode = 0, test_port = 0;

    /* Create device for MRV1543 GE PHY */
    rc = diag_gephy_dev_create(gephy_obj_p);
    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed", __FUNCTION__);
        return (FAILED);
    }

    while (1) {
        printf("GE PHY LED Supported Mode:\n");
        printf("[0] Turn Green on.\n");
        printf("[1] Turn off.\n");
        printf("[f] Leave the utility.\n");
        test_mode = gethex_answer("Enter LED mode: ", 0, 0, 0xf);
        
        if (test_mode == 0) {
            test_port = gethex_answer("Enter GE PHY Port Number: ", 0, 0, 0x3);
            rc = gephy_obj_p->callin_fvt->
                 gephy_set_led_on((dev_object_t *)gephy_obj_p, test_port);
            if ( rc == FAILED) {
                goto _exit;
            } 

        } else if (test_mode == 1) {
            test_port = gethex_answer("Enter GE PHY Port Number: ", 0, 0, 0x3);
            rc = gephy_obj_p->callin_fvt->
                 gephy_set_led_off((dev_object_t *)gephy_obj_p, test_port);
            if (rc == FAILED) {
                goto _exit;
            } 

        } else if (test_mode == 0xf) {
            for (test_port = GEPHYP0; test_port <= GEPHYP3; test_port++){
                rc = gephy_obj_p->callin_fvt->
                     gephy_set_led_default((dev_object_t *)gephy_obj_p, test_port);
                if (rc == FAILED) {
                    goto _exit;
                }
            }
            return (PASSED);

        } else {
            printf("Wrong test mode!\n");
        }
    }

_exit:
    gephy_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&gephy_obj_p);
    
    return (rc);
}

/******************************************************************************
 *
 * Function: diag_i350_led_util
 *
 * Description:  Utility of I350 SFP LED
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_i350_led_util (void)
{
    uint32_t test_mode = 0, test_port = 0;

    while (1) {
        printf("Warning please check you have lpbk cable on SFP module\n");
        printf("I350 SFP LED Supported Mode:\n");
        printf("[0] Turn Green on.\n");
        printf("[1] Turn off.\n");
        printf("[f] Leave the utility.\n");
        printf("\n");
        test_mode = gethex_answer("Enter LED mode: ", 0, 0, 0xf);

        if (test_mode == 0) {
            test_port = gethex_answer("Enter I350 Port number: ", 0, 0, 0x1);
            test_port += DNV_I350_PORT1; 
            /*Turn on led by link up SFP*/
            diag_i350_led_on_off(test_port, TRUE);
        } else if (test_mode == 1) {
            test_port = gethex_answer("Enter I350 Port number: ", 0, 0, 0x1);
            test_port += DNV_I350_PORT1; 
            /*Turn off led by link up SFP*/
            diag_i350_led_on_off(test_port, FALSE);
        } else if (test_mode == 0xf) {
            break;
        } else {
            printf("Wrong test mode!\n");
        }
    }
    return (PASSED);
}

/******************************************************************************
 *
 * Function: diag_I350_led_on_off
 *
 * Description:  Turn on/off I350 LED
 *
 * Inputs      : port - I350 port num
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_i350_led_on_off (int portnum, boolean command)
{
    struct mii_ioctl_data *miip;
    int sk;
    struct ifreq ethreq;
    char interface_name[20];
    char iface_name_up[128];
    char iface_name_down[128];
    uint buf;

    /* Create socket for ioctl calls
     */
    sk = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP));
    if (sk < 0) {
        printf("%s() Error Creating RX Socket", __FUNCTION__);
        return (FAILED);
    }

    dnv_get_correct_iface_name(portnum, interface_name);

    sprintf(ethreq.ifr_name, interface_name);

    miip = (struct mii_ioctl_data *)&ethreq.ifr_ifru;

    /* detect phy media type from SFP module.
     * In addition to using the standardized calls,
     * each interface can define its own ioctl commands.
     * The plip interface, for example,
     * allows the interface to modify its internal timeout values via ioctl.
     * The ioctl implementation for sockets recognizes 16 commands
     * as private to the interface: SIOCDEVPRIVATE through SIOCDEVPRIVATE+15. */
    if (ioctl(sk, I350_IOCTL_COMMAND3, &ethreq) < 0) {
        printf("%s() Error do IOCTL %d", __FUNCTION__, I350_IOCTL_COMMAND3);
        close(sk);
        return (FAILED);
    } 
    buf = miip->val_out;
    /* Set up SFP module FCLF-8521-3 register to link up */
    if ((buf == e1000_media_type_copper) && (command == TRUE)) {
        if (ioctl(sk, I350_IOCTL_COMMAND2, &ethreq) < 0) {
            printf("%s() Error do IOCTL %d", __FUNCTION__, I350_IOCTL_COMMAND2);
            close(sk);
            return (FAILED);
        } 
    }
    close(sk);

    /* When link up led will turn on*/
    if (command == TRUE) {
        sprintf(iface_name_up, "ifconfig %s up", interface_name);
        system(iface_name_up);
    } else {
        sprintf(iface_name_down, "ifconfig %s down", interface_name);
        system(iface_name_down);
    }

    return (PASSED);
}

/******************************************************************************
 *
 * Function: diag_port80_led_util
 *
 * Description : Utiliy of Port 80 LED
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_port80_led_util(void)
{
    uint32_t reg_val = 0, default_val = 0, test_mode = 0, test_port = 0;

    while (1) {
        printf("Port80 LED Supported Mode:\n");
        printf("[0] Turn Green on.\n");
        printf("[1] Turn off.\n");
        printf("[f] Leave the utility.\n");
        printf("\n");
        test_mode = gethex_answer("Enter LED mode: ", 0, 0, 0xf);

        /* Set read/write permission start from Port80 on, only one port*/
        if (ioperm(PORT80_ADDR, ACCESS_PORT_NUM, PORT80_ACCESS_ON) < 0) {
            printf("Failed to set access of Port80 on.");
            return (FAILED);
        }

        default_val = inb(PORT80_ADDR);

        if (test_mode == 0) {
            test_port = gethex_answer("Enter LED Port number: ", 0, 0, 0x7);
            /* Turn on Port80 led with green*/
            reg_val = inb(PORT80_ADDR);
            outb(reg_val | (PORT80_LED_ON << test_port), PORT80_ADDR);

        } else if (test_mode == 1) {
            test_port = gethex_answer("Enter LED Port number: ", 0, 0, 0x7);
            /* Turn off Port80 led */
            reg_val = inb(PORT80_ADDR);
            outb(reg_val & (~(PORT80_LED_OFF << test_port)), PORT80_ADDR);

        } else if (test_mode == 0xf) {
            outb(default_val & PORT80_LED_ALL_OFF, PORT80_ADDR);
            
            /* Set read/write permission start from Port80 off, only one port*/
            if (ioperm(PORT80_ADDR, ACCESS_PORT_NUM, PORT80_ACCESS_OFF) < 0) {
                printf("Failed to set access of Port80 off");
                return (FAILED);
            }

            break;
        } else {
            printf("Wrong test mode!\n");
        }
    }

    return (PASSED);
}

/******************************************************************************
 *
 * Function: diag_all_led_on_util
 *
 * Description: Utility to turn on all LED
 *
 * Inputs      : option - LED color option
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_all_led_on_util (int option)
{
    dev_88e1543_object_t gephy_obj;
    dev_88e1543_object_t *gephy_obj_p = &gephy_obj;
    int rc = FAILED, on = 1;
    uint32_t ix;
    printf("Warning please check you have lpbk cable on SFP module\n");

    /* Create device for MRV1543 GE PHY */
    rc = diag_gephy_dev_create(gephy_obj_p);
    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed", __FUNCTION__);
        return (FAILED);
    }
	/* Turn on all led */
    if (fpga_version_is_more_than_2p0() == FALSE) {
        if (option == GREEN) {
            /* Turn on system led with green */
            rc = fpga_register_operation(FPGA_LED_REG, VPN_SYS_LED_GREEN, on);
            if ( rc == FAILED) {
                goto _exit;
            }
        } else {
            /* Turn on system led with yellow */
            rc = fpga_register_operation(FPGA_LED_REG, VPN_SYS_LED_YELLOW, on);
            if ( rc == FAILED) {
                goto _exit;
            } 
        }
    } else {
        if (option == GREEN) {
            /* Turn on system led with green */
            rc =fpga_register_operation(CEDGE_LPC_STATUS_LED_REG,
                                        CEDGE_VPN_SYS_LED_GREEN, on);
            if ( rc == FAILED) {
                goto _exit;
            } 
        } else {
            /* Turn on system led with yellow */
            rc = fpga_register_operation(CEDGE_LPC_STATUS_LED_REG,
                                         CEDGE_VPN_SYS_LED_YELLOW, on);
            if ( rc == FAILED) {
                goto _exit;
            } 

        }
    }
        
	if (has_lte_sku() == TRUE) {
        if (option == GREEN) {
            /* Turn on LTE Modem RSSI with green color*/
            rc =fpga_register_operation(FPGA_LTE_RSSI_LED, LTE_MOD_HIGH_RSSI, on);
            if ( rc == FAILED) {
                goto _exit;
            } 
        } else {
            /* Turn on LTE Modem RSSI with yellow */
            rc = fpga_register_operation(FPGA_LTE_RSSI_LED,LTE_MOD_LOW_RSSI, on);
            if ( rc == FAILED) {
                goto _exit;
            } 
        }

        /* Turn on LTE SIM card LED with green color*/
        rc =fpga_register_operation(FPGA_LED_REG, LTE_SIM_ACT_LED, on);
        if ( rc == FAILED) {
            goto _exit;
        } 
	}

    /* Turn on all GEPHY ports LED with green color */
    for (ix= GEPHYP0; ix <= GEPHYP3; ix++) {
        rc = gephy_obj_p->callin_fvt->gephy_set_led_on((dev_object_t *)gephy_obj_p, ix);
       if ( rc == FAILED) {
            goto _exit;
        } 
	}

    /* Turn on all i350 ports LED with green color */
	if (has_sfp_sku() == TRUE) {
        /*Turn on led by link up SFP*/
        rc = diag_i350_led_on_off(DNV_I350_PORT1, TRUE);
        if ( rc == FAILED) {
            goto _exit;
        } 
        rc = diag_i350_led_on_off(DNV_I350_PORT2, TRUE);
        if ( rc == FAILED) {
            goto _exit;
        } 
	}    

_exit:
    gephy_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&gephy_obj_p);
	
    return (rc);
}

/******************************************************************************
 *
 * Function: diag_all_led_off_util
 *
 * Description: Utility to turn off all LED
 *
 * Inputs      : none
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_all_led_off_util (void)
{
    dev_88e1543_object_t gephy_obj;
    dev_88e1543_object_t *gephy_obj_p = &gephy_obj;
    int rc = FAILED, off = 0;
	uint32_t ix;
    printf("Warning please check you have lpbk cable on SFP module\n");

    /* Create device for MRV1543 GE PHY */
    rc = diag_gephy_dev_create(gephy_obj_p);
    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed", __FUNCTION__);
        return (FAILED);
    }
	/* Turn all led off */
        if (fpga_version_is_more_than_2p0() == FALSE) {
            /* Turn off system led */
            rc = fpga_register_operation(FPGA_LED_REG, LED_OFF, off);
            if ( rc == FAILED) {
                goto _exit;
            } 
        } else {
            /* Turn off system led */
            rc =fpga_register_operation(CEDGE_LPC_STATUS_LED_REG,
                                        LED_OFF, off);
            if ( rc == FAILED) {
                goto _exit;
            } 
        }
        
	    if (has_lte_sku() == TRUE) {
            /* Turn off LTE Modem RSSI led */
            rc =fpga_register_operation(FPGA_LTE_RSSI_LED, LED_OFF, off);
            if ( rc == FAILED) {
                goto _exit;
            } 
            /* Turn off LTE SIM card led */
            rc =fpga_register_operation(FPGA_LED_REG, LED_OFF, off);
            if ( rc == FAILED) {
                goto _exit;
            } 
	    }

		/* Turn off all GEPHY ports led */
        for (ix= GEPHYP0; ix <= GEPHYP3; ix++) {
            rc = gephy_obj_p->callin_fvt->gephy_set_led_off((dev_object_t *)gephy_obj_p, ix);
            if ( rc == FAILED) {
                goto _exit;
            } 
	    }

		/* Turn off all i350 ports led */
	    if (has_sfp_sku() == TRUE) {
            /*Turn off led by link down SFP*/
            rc = diag_i350_led_on_off(DNV_I350_PORT1, FALSE);
            if ( rc == FAILED) {
                goto _exit;
            } 
            rc = diag_i350_led_on_off(DNV_I350_PORT2, FALSE);
            if ( rc == FAILED) {
                goto _exit;
            } 
	    }

_exit:
    gephy_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&gephy_obj_p);
	
    return (rc);
}

/*-------------------------------------------------
$Log: diag_led_util.c,v $
Revision 1.7  2020/02/04 08:49:42  alicehua
CSCvs68364: Add and modify codes for FPGA Phase2.

Revision 1.6  2019/10/16 23:50:47  alicehua
CSCvr68092: Add LED utility (turn on/off all LED).

Revision 1.5  2019/07/12 09:13:42  alicehua
Modified codes based on code PRRQs.

Revision 1.4  2019/07/11 12:31:28  alicehua
Collapse Nutella codes into main trunk.

$Endlog$
*/
