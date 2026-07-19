/* $Id: bcm_gesw_utils.c,v 1.6 2018/05/18 09:24:51 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/bcm_gesw_utils.c,v $
 *------------------------------------------------------------------
 *
 * bcm_gesw_utils.c - BCM56321 diagnostics utility functions
 *
 * Oct 2011, Paul Tong
 *
 * Copyright (c) 2013-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include "sys/types.h"

#include "sal/core/libc.h"
#include "sal/appl/pci.h"
#include "sal/appl/config.h"
#include "sal/core/boot.h"

#include "soc/mem.h"
#include "soc/devids.h"
#include "soc/debug.h"
#include "soc/drv.h"
#include "soc/l2x.h"
#include "soc/mcm/driver.h"

#include "bcm/error.h"
#include "bcm/vlan.h"
#include "bcm/init.h"
#include "bcm/stat.h"
#include "bcm/link.h"
#include "bcm/port.h"
#include "bcm/mcast.h"
#include "bcm/field.h"

#include "appl/diag/sysconf.h"
#include "appl/diag/system.h"
#include "appl/diag/dport.h"
#include "appl/diag/diag.h"
#include "appl/diag/test.h"
#include "appl/test/loopback.h"
#include "appl/test/loopback2.h"
#include "../src/appl/test/testlist.h"

#include "bcm_gesw.h"
#include "queryflags.h"

extern cmd_result_t sh_process_command(int u, char *c);

#undef DEBUG
//#define DEBUG 1

/* This function is for experimenting the API for loopback control
*/
void pfix_loopback_util(void)
{
    int unit, port, rv;
    uint value, on_off;
    int yn;

    unit = 0;
    port = getdec_answer("Enter greyhound port number (2 - 22)", 2, 2, 22);
    printf("port is %d\n", port);

    printf("BCM_PORT_PHY_CONTROL_LOOPBACK_REMOTE(%d): default\n", BCM_PORT_PHY_CONTROL_LOOPBACK_REMOTE);
    yn = getdec_answer("Enter 0 or 1", 0, 0, 1);
    if (yn) {
      rv = bcm_port_phy_control_get(unit, port, BCM_PORT_PHY_CONTROL_LOOPBACK_REMOTE, &value);
      if (rv) {
	printf("get failed, rv= %d\n", rv);
      }
      printf("get port= %d value= %d\n", port, value);
      on_off = 1;
      printf("on_off= %d\n", on_off);
      rv = bcm_port_phy_control_set(unit, port, BCM_PORT_PHY_CONTROL_LOOPBACK_REMOTE, on_off);
      if (rv) {
	printf("set failed, rv= %d\n", rv);
      }
      rv = bcm_port_phy_control_get(unit, port, BCM_PORT_PHY_CONTROL_LOOPBACK_REMOTE, &value);
      if (rv) {
	printf("get failed, rv= %d\n", rv);
      }
      printf("get port= %d value= %d\n", port, value);
    }


    printf("\nBCM_PORT_PHY_CONTROL_LOOPBACK_EXTERNAL(%d): default\n", BCM_PORT_PHY_CONTROL_LOOPBACK_EXTERNAL);
    yn = getdec_answer("Enter 0 or 1", 0, 0, 1);
    if (yn) {
      rv = bcm_port_phy_control_get(unit, port, BCM_PORT_PHY_CONTROL_LOOPBACK_EXTERNAL, &value);
      if (rv) {
	printf("get failed, rv= %d\n", rv);
      }
      printf("get port= %d value= %d\n", port, value);
      on_off = 1;
      printf("on_off= %d\n", on_off);
      rv = bcm_port_phy_control_set(unit, port, BCM_PORT_PHY_CONTROL_LOOPBACK_EXTERNAL, on_off);
      if (rv) {
	printf("set failed, rv= %d\n", rv);
      }
      rv = bcm_port_phy_control_get(unit, port, BCM_PORT_PHY_CONTROL_LOOPBACK_EXTERNAL, &value);
      if (rv) {
	printf("get failed, rv= %d\n", rv);
      }
      printf("get port= %d value= %d\n", port, value);
    }

    printf("\nBCM_PORT_PHY_CONTROL_LOOPBACK_INTERNAL(%d): default\n", BCM_PORT_PHY_CONTROL_LOOPBACK_INTERNAL);
    yn = getdec_answer("Enter 0 or 1", 0, 0, 1);
    if (yn) {
      rv = bcm_port_phy_control_get(unit, port, BCM_PORT_PHY_CONTROL_LOOPBACK_INTERNAL, &value);
      if (rv) {
	printf("get failed, rv= %d\n", rv);
      }
      printf("get port= %d value= %d\n", port, value);
      on_off = 1;
      printf("on_off= %d\n", on_off);
      rv = bcm_port_phy_control_set(unit, port, BCM_PORT_PHY_CONTROL_LOOPBACK_INTERNAL, on_off);
      if (rv) {
	printf("set failed, rv= %d\n", rv);
      }
      rv = bcm_port_phy_control_get(unit, port, BCM_PORT_PHY_CONTROL_LOOPBACK_INTERNAL, &value);
      if (rv) {
	printf("get failed, rv= %d\n", rv);
      }
      printf("get port= %d value= %d\n", port, value);
    }

    printf("\nBCM_PORT_PHY_CONTROL_LOOPBACK_REMOTE_PCS_BYPASS(%d): default\n", BCM_PORT_PHY_CONTROL_LOOPBACK_REMOTE_PCS_BYPASS);
    yn = getdec_answer("Enter 0 or 1", 0, 0, 1);
    if (yn) {
      rv = bcm_port_phy_control_get(unit, port, BCM_PORT_PHY_CONTROL_LOOPBACK_REMOTE_PCS_BYPASS, &value);
      if (rv) {
	printf("get failed, rv= %d\n", rv);
      }
      printf("get port= %d value= %d\n", port, value);
      on_off = 1;
      printf("on_off= %d\n", on_off);
      rv = bcm_port_phy_control_set(unit, port, BCM_PORT_PHY_CONTROL_LOOPBACK_REMOTE_PCS_BYPASS, on_off);
      if (rv) {
	printf("set failed, rv= %d\n", rv);
      }
      rv = bcm_port_phy_control_get(unit, port, BCM_PORT_PHY_CONTROL_LOOPBACK_REMOTE_PCS_BYPASS, &value);
      if (rv) {
	printf("get failed, rv= %d\n", rv);
      }
      printf("get port= %d value= %d\n", port, value);
    }

    printf("\nBCM_PORT_PHY_CONTROL_LOOPBACK_PMD(%d): default\n", BCM_PORT_PHY_CONTROL_LOOPBACK_PMD);
    yn = getdec_answer("Enter 0 or 1", 0, 0, 1);
    if (yn) {
      rv = bcm_port_phy_control_get(unit, port, BCM_PORT_PHY_CONTROL_LOOPBACK_PMD, &value);
      if (rv) {
	printf("get failed, rv= %d\n", rv);
      }
      printf("get port= %d value= %d\n", port, value);
      on_off = 1;
      printf("on_off= %d\n", on_off);
      rv = bcm_port_phy_control_set(unit, port, BCM_PORT_PHY_CONTROL_LOOPBACK_PMD, on_off);
      if (rv) {
	printf("set failed, rv= %d\n", rv);
      }
      rv = bcm_port_phy_control_get(unit, port, BCM_PORT_PHY_CONTROL_LOOPBACK_PMD, &value);
      if (rv) {
	printf("get failed, rv= %d\n", rv);
      }
      printf("get port= %d value= %d\n", port, value);
    }
    return;
}

/* This function is for experimenting the API for loopback control
*/
void pfix_loopback_util_1(void)
{
    int unit, port, rv;
    int lb_mode, yn;

    unit = 0;
    port = getdec_answer("Enter greyhound port number (2 - 22)", 2, 2, 22);
    printf("port is %d\n", port);

    printf("\nbcm_port_loopback_get: default\n");
    rv = bcm_port_loopback_get(unit, port, &lb_mode);
    if (rv) {
      printf("get failed, rv= %d\n", rv);
    }
    printf("get port= %d lb_mode= %d\n", port, lb_mode);


    printf("set NONE loopback\n");
    rv = bcm_port_loopback_set(unit, port, BCM_PORT_LOOPBACK_NONE);
    if (rv) {
      printf("set failed, rv= %d\n", rv);
    }
    rv = bcm_port_loopback_get(unit, port, &lb_mode);
    if (rv) {
      printf("get failed, rv= %d\n", rv);
    }
    printf("get port= %d lb_mode= %d\n", port, lb_mode);


    printf("set MAC loopback\n");
    yn = getdec_answer("Enter 0 or 1", 0, 0, 1);
    if (yn) {
      rv = bcm_port_loopback_set(unit, port, BCM_PORT_LOOPBACK_MAC);
      if (rv) {
	printf("set failed, rv= %d\n", rv);
      }
      rv = bcm_port_loopback_get(unit, port, &lb_mode);
      if (rv) {
	printf("get failed, rv= %d\n", rv);
      }
      printf("get port= %d lb_mode= %d\n", port, lb_mode);
    }

    printf("set PHY loopback\n");
    yn = getdec_answer("Enter 0 or 1", 0, 0, 1);
    if (yn) {
      rv = bcm_port_loopback_set(unit, port, BCM_PORT_LOOPBACK_PHY);
      if (rv) {
	printf("set failed, rv= %d\n", rv);
      }
      rv = bcm_port_loopback_get(unit, port, &lb_mode);
      if (rv) {
	printf("get failed, rv= %d\n", rv);
      }
      printf("get port= %d lb_mode= %d\n", port, lb_mode);
    }

    printf("set PHY_REMOTE loopback\n");
    yn = getdec_answer("Enter 0 or 1", 0, 0, 1);
    if (yn) {
      rv = bcm_port_loopback_set(unit, port, BCM_PORT_LOOPBACK_PHY_REMOTE);
      if (rv) {
	printf("set failed, rv= %d\n", rv);
      }
      rv = bcm_port_loopback_get(unit, port, &lb_mode);
      if (rv) {
	printf("get failed, rv= %d\n", rv);
      }
      printf("get port= %d lb_mode= %d\n", port, lb_mode);
    }
    return;
}

/*-------------------------------------------------------------*/
/* port loopback set and get functions ------------------------*/

/*
 * Function:
 *   bcm_gesw_ge_line_lpbk_set
 *
 * Description:
 *   Set or clear the BCM GESW GE port line loopback.
 *   This function is leveraged from BCM SDK.
 *   The unit and port number follows the scheme in broadcom SDK
 *
 * Input:
 *   unit - BCM chip unit id on the system
 *   port - GE port number
 *   on_off - flag to turn the loopback on or off
 *
 * Output:
 *   SOC_E_XXX
 */
static int bcm_gesw_ge_line_lpbk_set(int unit, int port, int on_off)
{
    uint cmdcfg;
    int rv = 0;

    /* Greyhound GESW 53403, and 53404 support remotel line
     * loopback at the PCS layer. The external packet did
     * loopback to the sender, but it also entered and flooded
     * the switch.
     */
    if (is_bcm_greyhound()) {
        /* for PCS remote loopback */
        rv = bcm_port_phy_control_set(unit, port, BCM_PORT_PHY_CONTROL_LOOPBACK_REMOTE, on_off);
	if (rv) {
	    printf("%s failed, port= %d on_off= %d\n", __FUNCTION__, port, on_off);
	}
    }
    else {
        /* Victory platforms (O2, Juno, USD) use Helix GESW which allow
	 * line loopback at the port.
	 */
        SOC_IF_ERROR_RETURN(READ_COMMAND_CONFIGr(unit, port, &cmdcfg));

	soc_reg_field_get(unit, COMMAND_CONFIGr, cmdcfg, LINE_LOOPBACKf);

	soc_reg_field_set(unit, COMMAND_CONFIGr, &cmdcfg, LINE_LOOPBACKf, on_off);
	SOC_IF_ERROR_RETURN(WRITE_COMMAND_CONFIGr(unit, port, cmdcfg));

	/* pfix-debug */
	//    READ_COMMAND_CONFIGr(unit, port, &cmdcfg);
	//    printf("pfix, %s-2 port= %d read cmdcfg= %#.8x\n", __FUNCTION__,port,cmdcfg);
    }
    return(SOC_E_NONE);
}

/*
 * Function:
 *   bcm_gesw_ge_line_lpbk_get
 *
 * Description:
 *   Get the BCM GESW GE port line loopback state.
 *   The unit and port number follows the scheme in broadcom SDK
 *
 * Input:
 *   unit - BCM chip unit id on the system
 *   port - GE port number
 *   state - ptr to the state vaule holder
 *
 * Output:
 *   SOC_E_XXX
 */
static int bcm_gesw_ge_line_lpbk_get(int unit, int port, int *state)
{
    uint cmdcfg;
    int rv = 0;

    if (is_bcm_greyhound()) {
        /* for PCS remote loopback */
        rv = bcm_port_phy_control_get(unit, port, BCM_PORT_PHY_CONTROL_LOOPBACK_REMOTE, (uint *)state);
	if (rv) {
	    printf("%s failed, port= %d\n", __FUNCTION__, port);
	}
	printf("\nNote: For BCM53403 and 53404 the line loopback status always read back 0.\n");
    }
    else {
        /* Victory platforms (O2, Juno, USD) use Helix GESW which allow
	 * line loopback at the port.
	 */
        SOC_IF_ERROR_RETURN(READ_COMMAND_CONFIGr(unit, port, &cmdcfg));
	//    printf("pfix, %s-0 port= %d read cmdcfg= %#.8x\n", __FUNCTION__, port, cmdcfg);

	*state = soc_reg_field_get(unit, COMMAND_CONFIGr, cmdcfg,
				  LINE_LOOPBACKf);
	//    printf("pfix, %s-1 port= %d LINE_LOOPBACKf= %#.8x\n", __FUNCTION__, port, *state);
    }
    return SOC_E_NONE;
}

/*
 * Function:
 *   bcm_gesw_ge_lpbk_set
 *
 * Description:
 *   Set the BCM GESW GE port to MAC, line, or no loopback.
 *   This function is leveraged from BCM SDK.
 *   The unit and port number follows the scheme in broadcom SDK
 *
 * Input:
 *   unit - BCM chip unit id on the system
 *   port - GE port number
 *   lb - flag for MAC, line, or no loopback
 *
 * Output:
 *   SOC_E_XXX
 */
int bcm_gesw_ge_lpbk_set(int unit, int port, int lb)
{
    int rc = SOC_E_NONE;

    if (lb == GESW_LOOPBACK_NONE) {
	rc |= bcm_gesw_ge_line_lpbk_set(unit, port, 0);
	rc |= bcm_port_loopback_set(unit, port, BCM_PORT_LOOPBACK_NONE);
    }
    else if (lb == GESW_MAC_LOOPBACK) {
	rc |= bcm_gesw_ge_line_lpbk_set(unit, port, 0);
	rc |= bcm_port_loopback_set(unit, port, BCM_PORT_LOOPBACK_MAC);
    }
    else if (lb == GESW_LINE_LOOPBACK) {
	rc |= bcm_port_loopback_set(unit, port, BCM_PORT_LOOPBACK_NONE);
	rc |= bcm_gesw_ge_line_lpbk_set(unit, port, 1);
    }

    return rc;
}

/*
 * Function:
 *   bcm_gesw_ge_lpbk_get
 *
 * Description:
 *   Get the state of the BCM GESW GE port loopback setting.
 *   This function is leveraged from BCM SDK.
 *
 * Input:
 *   unit - BCM chip unit id
 *   port - GE port number
 *   lb - flag for MAC or line loopback
 *   state - The state of the MAC or line loopback bit (1 or 0)
 *
 * Output:
 *   SOC_E_XXX
 */
int bcm_gesw_ge_lpbk_get(int unit, int port, int lb, int *state)
{
    int rc = SOC_E_NONE;
    int lb_st;

    if (lb == GESW_MAC_LOOPBACK) {
	rc |= bcm_port_loopback_get(unit, port, &lb_st);
	*state = (lb_st == BCM_PORT_LOOPBACK_MAC) ? 1 : 0;
    }
    else if (lb == GESW_LINE_LOOPBACK) {
	rc |= bcm_gesw_ge_line_lpbk_get(unit, port, state);
    }

    return rc;
}

/*
 * Function:
 *   bcm_gesw_ge_link_status_get
 *
 * Description:
 *   Get the state of the BCM GESW GE port link status.
 *   This function is leveraged from BCM SDK.
 *
 * Input:
 *   unit - BCM chip unit id
 *   port - GE port number
 *   status - 1 stand for link up, 0 stand for link down
 *
 * Output:
 *   SOC_E_XXX
 */
int bcm_gesw_ge_link_status_get(int unit, int port, int *status)
{
    int rc = SOC_E_NONE;

    rc |= bcm_port_link_status_get(unit, port, status);

    return rc;
}

/*
 * Function:
 *   bcm_gesw_xaui_lpbk_set
 *
 * Description:
 *   Set the BCM GESW XAUI to MAC, line, or no loopback.
 *   This function is leveraged from BCM SDK.
 *
 * Input:
 *   unit - BCM chip unit id
 *   port - XAUI port number
 *   lb - flag for MAC, line, or no loopback
 *
 * Output:
 *   SOC_E_XXX
 */
int bcm_gesw_xaui_lpbk_set(int unit, int port, int lb)
{
    uint64 ctrl, octrl;

    if (is_bcm_greyhound()) {
        printf("%s xaui loopback not supported in greyhound chip\n",__FUNCTION__);
	return SOC_E_FAIL;
    }
    else {
        /* Victory platforms (O2, Juno, USD) use Helix GESW which allow
	 * line loopback at the port.
	 */
        SOC_IF_ERROR_RETURN(READ_MAC_CTRLr(unit, port, &ctrl));

	octrl = ctrl;

	if (lb == GESW_LOOPBACK_NONE) {
	    soc_reg64_field32_set(unit, MAC_CTRLr, &ctrl, LCLLOOPf, 0);
	    soc_reg64_field32_set(unit, MAC_CTRLr, &ctrl, RMTLOOPf, 0);
	}
	else if (lb == GESW_MAC_LOOPBACK) {
	    soc_reg64_field32_set(unit, MAC_CTRLr, &ctrl, RMTLOOPf, 0);
	    soc_reg64_field32_set(unit, MAC_CTRLr, &ctrl, LCLLOOPf, 1);
	}
	else if (lb == GESW_LINE_LOOPBACK) {
	    soc_reg64_field32_set(unit, MAC_CTRLr, &ctrl, LCLLOOPf, 0);
	    soc_reg64_field32_set(unit, MAC_CTRLr, &ctrl, RMTLOOPf, 1);
	}

	if (COMPILER_64_NE(ctrl, octrl)) {
	    SOC_IF_ERROR_RETURN(WRITE_MAC_CTRLr(unit, port, ctrl));
	}

	/* pfix debug */
	/*
           READ_MAC_CTRLr(unit, port, &ctrl);

           remote_lpbk = soc_reg64_field32_get(unit, MAC_CTRLr, ctrl, RMTLOOPf);
           local_lpbk = soc_reg64_field32_get(unit, MAC_CTRLr, ctrl, LCLLOOPf);
           printf("pfix GESW xaui port %d mac loop back= %d remote loop back %d\n",
	           port, local_lpbk, remote_lpbk);
	*/

	return SOC_E_NONE;
    }
}

/*
 * Function:
 *   bcm_gesw_xaui_lpbk_get
 *
 * Description:
 *   Get the state of the BCM GESW XAUI loopback setting.
 *   This function is leveraged from BCM SDK.
 *
 * Input:
 *   unit - BCM chip unit id
 *   port - XAUI port number
 *   lb - flag for MAC or line loopback
 *   state - The state of the MAC or line loopback bit (1 or 0)
 *
 * Output:
 *   SOC_E_XXX
 */
int bcm_gesw_xaui_lpbk_get(int unit, int port, int lb, int *state)
{
    uint64 ctrl;

    if (is_bcm_greyhound()) {
        printf("%s xaui loopback not supported in greyhound chip\n",__FUNCTION__);
	return SOC_E_FAIL;
    }
    else {
        /* Victory platforms (O2, Juno, USD) use Helix GESW which allow
	 * line loopback at the port.
	 */
        SOC_IF_ERROR_RETURN(READ_MAC_CTRLr(unit, port, &ctrl));
	//    printf("pfix, %s-0 read ctrl= %#.8lx\n", __FUNCTION__,ctrl);

	if (lb == GESW_MAC_LOOPBACK) {
	    *state = soc_reg64_field32_get(unit, MAC_CTRLr, ctrl, LCLLOOPf);
	}
	else if (lb == GESW_LINE_LOOPBACK) {
	    *state = soc_reg64_field32_get(unit, MAC_CTRLr, ctrl, RMTLOOPf);
	}

	//    printf("pfix, %s-2 lb %d state is %d\n", __FUNCTION__, lb, *state);

	return SOC_E_NONE;
    }
}

/*-------------------------------------------------------------*/

/*
 * Function: get_bcm_shell_test_result
 * Get the test result of a BCM diag shell test
 *
 * Input: 
 * testnum - BCM diag test number
 * runcnt - test run count
 * passcnt - test pass count
 *
 * Return: -1 for error return, 0 for normal return
 */
int get_bcm_shell_test_result(int testnum, int *runcnt, int *passcnt)
{
    int i;

    for (i = 0; i < test_cnt; i++) {
        if (test_list[i].t_test == testnum) {
	    *runcnt = test_list[i].t_runs;
	    *passcnt = test_list[i].t_success;
#ifdef DEBUG
	    printf("test %d found. runs=%d, pass= %d, fail= %d\n",
		   test_list[i].t_test,
		   test_list[i].t_runs,
		   test_list[i].t_success,
		   test_list[i].t_fail);
#endif
	    return(0);
	}
    }
    return (-1);
}

/*
 * Function: exec_bcm_shell_cmd
 * Execute a command at the BCM shell from the Overlord diag code.
 *
 * Input: 
 * unit - BCM chip unit number. It is always 0 on Overlord.
 * cmd - the BCM shell command
 * print_cmd - flag to determine if the command is printed on screen
 *
 * Return: PASS/FAIL
 */
int exec_bcm_shell_cmd (int unit, char *cmd, int print_cmd)
{
    if (print_cmd) {
        printf("BCM cmd: %s\n", cmd);
    }
    if (sh_process_command(unit, cmd) == CMD_OK) {
        return(PASS);
    }
    else {
        return(FAIL);
    }
}

/******** History ******** 
$Log: bcm_gesw_utils.c,v $
Revision 1.6  2018/05/18 09:24:51  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.5.32.2  2017/02/22 00:58:04  ptong
Fixed the BCM TX util

Revision 1.5.32.1  2016/06/22 21:25:35  ptong
Add temporary util for neptune bringup

Revision 1.5  2014/08/24 05:57:41  ptong
Added information printing

Revision 1.4  2014/07/28 03:43:46  alpeng
check gesw link status before sending packet

Revision 1.3  2014/06/19 22:00:51  ptong
Incorporated BCM sdk-6.4.1-EA2 for 10G-KR interface. New code works on O2, June, and non Greyhound USD regression

Revision 1.2  2013/11/26 08:40:35  hroni
fix compiler warning

Revision 1.1  2013/05/09 05:42:35  alpeng
moving overlord common code from x86

Revision 1.6  2013/01/08 01:18:14  ptong
Added function headers

Revision 1.5  2012/09/07 22:50:00  ptong
Code clean-up

Revision 1.4  2012/06/05 11:44:36  palin2
Clean up compiler warnings.

Revision 1.3  2012/04/03 01:46:18  ptong
Replace loading rc.soc with ovld_gesw_init() to avoid re-init the bcm drivers

Revision 1.2  2012/03/28 00:38:20  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:08  ptong
Initial archive of ng_diag module


$Endlog$
*/
