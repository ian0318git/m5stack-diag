/* $Id: bcm_gesw_init.c,v 1.20 2018/05/18 09:24:50 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/bcm_gesw_init.c,v $
 *------------------------------------------------------------------
 *
 * bcm_gesw_init.c - BCM56321 GESW initializations
 *
 * Oct 2011, Paul Tong ported from BCM SDK 5.9.2.
 *
 * Copyright (c) 2013-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/file.h>

#include "sal/core/libc.h"
#include "sal/core/boot.h"
#include "sal/appl/pci.h"
#include "sal/appl/config.h"
#include "sal/appl/sal.h"

#include "soc/mem.h"
#include "soc/devids.h"
#include "soc/debug.h"
#include "soc/drv.h"
#include "soc/l2x.h"
#include "soc/mcm/driver.h"
#include "soc/cmext.h"

#include "bcm/error.h"
#include "bcm/init.h"
#include "bcm/stat.h"
#include "bcm/link.h"
#include "bcm/port.h"
#include "bcm/vlan.h"
#include "bcm/stack.h"

#include "appl/diag/sysconf.h"
#include "appl/diag/system.h"
#include "appl/diag/dport.h"
#include "appl/diag/diag.h"
#include "appl/diag/test.h"
#include "appl/test/loopback.h"
#include "appl/test/loopback2.h"

#include "bde/linux/include/linux-bde.h"

#include "error.h"
#include "bcm_gesw.h"
#include "plat_defs.h"

extern int is_overlord(void);
extern int is_juno(void);
extern int is_utah(void);
extern int is_usd_machines(void);
extern int is_neptune(void);
extern int is_triton(void);
extern int is_proteus(void);
extern int is_neso(void);
extern int is_ntpn_machines(void);
extern int is_vg450(void);

/* The bus properties are (currently) the only system specific
 * settings required. 
 * These must be defined beforehand 
 */

#ifndef SYS_BE_PIO
#error "SYS_BE_PIO must be defined for the target platform"
#endif
#ifndef SYS_BE_PACKET
#error "SYS_BE_PACKET must be defined for the target platform"
#endif
#ifndef SYS_BE_OTHER
#error "SYS_BE_OTHER must be defined for the target platform"
#endif

#if !defined(SYS_BE_PIO) || !defined(SYS_BE_PACKET) || !defined(SYS_BE_OTHER)
#error "platform bus properties not defined."
#endif

/*
 * Broadcom code has define for PASS and FAIL.
 * undef them here and re-define them accoding
 * to the define in common.h to be consistent with
 * our diag code.
 */
#undef PASS
#undef FAIL
#define PASS 0
#define FAIL 1

ibde_t *bde;
int bcm_uid = 0;
char *gesw_init_state = "/gesw_init_state.txt";

/*
 * Function: is_gesw_initialized
 * This function checks if the GESW has already been
 * initialized by an existing diag process.
 *
 * Input: void
 *
 * Return: TRUE/FALSE
 */
int is_gesw_initialized(void)
{
    FILE *fp;
    int pid_x, pid_y;
    int rv = FALSE;
    char buf[128];
    char *fname = "save_ps.txt";
    char *o2_diag="o2x86_lnx";
    char *usd_diag="utah_lnx";
    char *nep_diag="nepx86_diag";
    char *diag_name;


    /* Read the pid saved in the gesw_init_state.txt file
     */
    fp = fopen(gesw_init_state, "r");
    if (fp != NULL) {
        fscanf(fp, "%d", &pid_x);
	fclose(fp);
    }

    /* Check if the saved pid exist in ps command.
     * Remove the old save_ps.txt file first if it exist.
     */
    fp = fopen(fname, "r");
    if (fp != NULL) {
        fclose(fp);
	sprintf(buf, "rm -f %s", fname);
	system(buf);
    }

    /* Capture the ps list and check the saved pid value
     */
    diag_name = nep_diag;
    if (is_overlord() || is_juno()) {
      diag_name = o2_diag;
    }
    else if (is_usd_machines()) {
      diag_name = usd_diag;
    }
    sprintf(buf, "ps | grep %s > %s", diag_name, fname);
    system(buf);

    fp = fopen(fname, "r");
    while (!feof(fp)) {
        fscanf(fp, "%d", &pid_y);
	fgets(buf, sizeof(buf), fp); // get the rest of the line

	if (pid_y == pid_x) {
	    rv = TRUE;
	    break;
	}
    }

    fclose(fp);
    sprintf(buf, "rm -f %s", fname);
    system(buf);

    return rv;
}

/*
 * Function: set_gesw_init_state
 * Write the current process pid into the file gesw_init_state.txt to
 * mark that the BCM chip is init'ed by this process so that other
 * process will not init it again.
 *
 * Input: void
 *
 * Return: void
 */
void set_gesw_init_state(void)
{
    FILE *fp;
    int pid;

    fp = fopen(gesw_init_state, "w");
    pid = getpid();
    fprintf(fp, "%d", pid);
    fclose(fp);
    //    printf("%s pid= %d\n",__FUNCTION__,pid);
}

/*
 * Function: detach_bcm_driver
 * Detach the BCM driver module from the diag.
 *
 * Input: void
 *
 * Return: void
 */
void detach_bcm_driver(void)
{
    if (bcm_attach_check(bcm_uid) == BCM_E_NONE) {
        bcm_detach(bcm_uid);
    }
}

/*
 * Function: gesw_load_init_script
 * Load the rc.soc init script
 *
 * Input: void
 *
 * Return: 0 for success. -1 for error.
 */
int gesw_load_init_script(void)
{
    int rv;

    rv = sh_rcload_file(bcm_uid, NULL, SOC_INIT_RC, FALSE);
    if (rv != CMD_OK) {
        printf("ERROR loading rc script on unit %d\n", bcm_uid);
	return(-1);
    }
    return (0);
}

/*
 * Function: bcm_soc_init
 * This function reset the BCM chip
 *
 * Input: unit - unit number. It is always 0 in overlord
 *
 * Return: PASS/FAIL
 */
int bcm_soc_init(int unit, int reset)
{
    /* Prevent hardware access during chip reset */
    system_shutdown(unit, 0);

    if (reset) {
        if (soc_reset_init(unit) < 0) {
	    return(FAIL);
	}
    } else {
        if (soc_init(unit) < 0) {
	    return(FAIL);
	}
    }

    return(PASS);
}

/*
 * Function: bcm_misc_init
 * Initialize miscellaneous chip registers
 *
 * Input: unit - unit number. It is always 0 in overlord
 *
 * Return: PASS/FAIL
 */
int bcm_misc_init(int unit)
{
    int rv;

    if ((rv = soc_misc_init(unit)) < 0) {
        printf("%s Unable to initialize misc for device %d: %s\n",
	       __FUNCTION__, unit, soc_errmsg((cmd_result_t)rv));
	return(FAIL);
    }
    return(PASS);
}

/*
 * Function: bcm_mmu_init
 * Initialize MMU registers
 *
 * Input: unit - unit number. It is always 0 in overlord
 *
 * Return: PASS/FAIL
 */
int bcm_mmu_init(int unit)
{
    int	bcm_rv;

    if ((bcm_rv = soc_mmu_init(unit)) < 0) {
        printf("%s Unable to initialize MMU for device: %s\n",
	       __FUNCTION__, bcm_errmsg(bcm_rv));
	return(FAIL);
    }
    return(PASS);
}

/*
 * Function: bcm_bcm_init
 * Init the bcm software layer
 * 
*/
int bcm_bcm_init(int unit)
{
    int	bcm_rv;

    if ((bcm_rv = bcm_init(unit)) < 0) {
        printf("%s Unable to initialize BCM driver on unit %d: %s\n",
	       __FUNCTION__, unit, bcm_errmsg(bcm_rv));
	return(FAIL);
    }

    return(PASS);
}

/*
 * Function: ovld_bcm_port_init
 * Init portsfor overlord
 *
 * Input: void
 *
 * Return: PASS
 */
int ovld_bcm_port_init(void)
{
    int unit = bcm_uid;
    int port;

    bcm_port_init(unit);

    for(port=2; port <= 4; port++) {
      bcm_port_enable_set(unit, port, TRUE);
      bcm_port_learn_set(unit, port, (BCM_PORT_LEARN_ARL | BCM_PORT_LEARN_FWD));
      bcm_port_linkscan_set(unit, port, BCM_LINKSCAN_MODE_SW);
      bcm_port_speed_set(unit, port, 1000);
      bcm_port_duplex_set(unit, port, BCM_PORT_DUPLEX_FULL);
      bcm_port_autoneg_set(unit, port, TRUE);
    }
    sleep(3);

    return (PASS);
}

/*
 * Function: ovld_bcm_check_port_init
 * Check the ports are initialized corrected after power up.
 *
 * Input: void
 *
 * Return: PASS/FAIL
 */
int ovld_bcm_check_port_init(void)
{
    int unit = bcm_uid;
    int port, rc = PASS;
    int port_en, learn_mode, linkscan, speed, full_duplex, autoneg;
    gesw_port_asgn_t *mptr, *mptr_sav;
    
#ifdef UTAH
    /* Check ge0-ge18 which are connected to NGIO interfaces
     * These port are forced to 1G, no autoneg, and full duplex.
     * They are connected to NGIO which are powered off at board init.
     * Check all these settings.
     */
    mptr = mptr_sav = plat_gesw_port_map_get();
    while (mptr->slot_type >= 0) {
        if (mptr->bcm_port_type == BCM_PTYPE_GE) {
	    port = mptr->bcm_port_num;
	    bcm_port_enable_get(unit, port, &port_en);
	    bcm_port_learn_get(unit, port, (uint *) &learn_mode);
	    bcm_port_linkscan_get(unit, port, &linkscan);
	    bcm_port_speed_get(unit, port, &speed);
	    bcm_port_duplex_get(unit, port, &full_duplex);
	    bcm_port_autoneg_get(unit, port, &autoneg);

	    if ((port_en != TRUE) || 
		(learn_mode != (BCM_PORT_LEARN_ARL | BCM_PORT_LEARN_FWD)) ||
		(linkscan != BCM_LINKSCAN_MODE_SW) ||
		(speed != 1000) ||
		(full_duplex != TRUE) ||
		(autoneg != FALSE)) {

	        printf("port %d init error:\n"
		       "  enable= %d expected= %d\n"
		       "  learn_mode= %d expected=%d\n"
		       "  linkscan= %d expected= %d\n"
		       "  speed= %d expexted= %d\n"
		       "  full_duplex= %d expected %d\n"
		       "  autoneg= %d expected= %d\n",
		       port, port_en, TRUE,
		       learn_mode, (BCM_PORT_LEARN_ARL |BCM_PORT_LEARN_FWD),
		       linkscan, BCM_LINKSCAN_MODE_SW,
		       speed, 1000,
		       full_duplex, BCM_PORT_DUPLEX_FULL,
		       autoneg, FALSE);
		rc = FAIL;
		break;
	    }
	}
	mptr++;
    }


    /* Check XAUI port 0 which is connected to FPGA XAUI interface
     * in HIGIG2 mode
     */
    mptr = mptr_sav;
    while (mptr->slot_type >= 0) {
        if ((mptr->bcm_port_type == BCM_PTYPE_XE) && (mptr->slot_type == TGT_DEV_DP)) {
	    port = mptr->bcm_port_num;
	    bcm_port_enable_get(unit, port, &port_en);
	    bcm_port_learn_get(unit, port, (uint *) &learn_mode);
	    bcm_port_linkscan_get(unit, port, &linkscan);
	    bcm_port_autoneg_get(unit, port, &autoneg);
	    bcm_port_speed_get(unit, port, &speed);

	    if ((port_en != TRUE) || 
		(learn_mode != (BCM_PORT_LEARN_ARL | BCM_PORT_LEARN_FWD)) ||
		(linkscan != BCM_LINKSCAN_MODE_SW) ||
		(autoneg != TRUE)) {

	        printf("port %d init error:\n"
		       "  enable= %d expected= %d\n"
		       "  learn_mode= %d expected=%d\n"
		       "  linkscan= %d expected= %d\n"
		       "  speed= %d expexted= %d\n"
		       "  autoneg= %d expected= %d\n",
		       port, port_en, TRUE,
		       learn_mode, (BCM_PORT_LEARN_ARL |BCM_PORT_LEARN_FWD),
		       linkscan, BCM_LINKSCAN_MODE_SW,
		       speed, 10000,
		       autoneg, FALSE);
		rc = FAIL;
	    }
	}
	mptr++;
    }

    /* Check XAUI port 1,2 (xe0, xe1 in the "ps" command) which are
     * connected to NGIO interfaces.
     * These port are forced to 10G, no autoneg.
     */
    mptr = mptr_sav;
    while (mptr->slot_type >= 0) {
        if ((mptr->bcm_port_type == BCM_PTYPE_XE) && (mptr->slot_type == TGT_DEV_NGSM)) {
	    port = mptr->bcm_port_num;
	    bcm_port_enable_get(unit, port, &port_en);
	    bcm_port_learn_get(unit, port, (uint *)&learn_mode);
	    bcm_port_linkscan_get(unit, port, &linkscan);
	    bcm_port_speed_get(unit, port, &speed);
	    bcm_port_autoneg_get(unit, port, &autoneg);

	    if ((port_en != TRUE) || 
		(learn_mode != (BCM_PORT_LEARN_ARL | BCM_PORT_LEARN_FWD)) ||
		(linkscan != BCM_LINKSCAN_MODE_SW) ||
		(speed != 10000) ||
		(autoneg != FALSE)) {

	        printf("port %d init error:\n"
		       "  enable= %d expected= %d\n"
		       "  learn_mode= %d expected=%d\n"
		       "  linkscan= %d expected= %d\n"
		       "  speed= %d expexted= %d\n"
		       "  autoneg= %d expected= %d\n",
		       port, port_en, TRUE,
		       learn_mode, (BCM_PORT_LEARN_ARL |BCM_PORT_LEARN_FWD),
		       linkscan, BCM_LINKSCAN_MODE_SW,
		       speed, 10000,
		       autoneg, FALSE);
		rc = FAIL;
		break;
	    }
	}
	mptr++;
    }
#else // Overlord or Juno
    /* Check ge0-ge2 which are connected to cavecreek sgmii 1-3
     * These ports are set to 1G and autoneg. Only need to check these
     * setting and no need to check full duplex.
     */
    mptr = mptr_sav = plat_gesw_port_map_get();
    while (mptr->slot_type >= 0) {
        if ((mptr->bcm_port_type == BCM_PTYPE_GE) && (mptr->slot_type == TGT_DEV_CPU)) {
	    port = mptr->bcm_port_num;
	    bcm_port_enable_get(unit, port, &port_en);
	    bcm_port_learn_get(unit, port, (uint *)&learn_mode);
	    bcm_port_linkscan_get(unit, port, &linkscan);
	    bcm_port_speed_get(unit, port, &speed);
	    bcm_port_autoneg_get(unit, port, &autoneg);

	    if ((port_en != TRUE) || 
		(learn_mode != (BCM_PORT_LEARN_ARL | BCM_PORT_LEARN_FWD)) ||
		(linkscan != BCM_LINKSCAN_MODE_SW) ||
		(speed != 1000) ||
		(autoneg != TRUE)) {

	        printf("port %d init error:\n"
		       "  enable= %d expected= %d\n"
		       "  learn_mode= %d expected=%d\n"
		       "  linkscan= %d expected= %d\n"
		       "  speed= %d expexted= %d\n"
		       "  autoneg= %d expected= %d\n",
		       port, port_en, TRUE,
		       learn_mode, (BCM_PORT_LEARN_ARL |BCM_PORT_LEARN_FWD),
		       linkscan, BCM_LINKSCAN_MODE_SW,
		       speed, 1000,
		       autoneg, TRUE);
		rc = FAIL;
		break;
	    }
	}
	mptr++;
    }

    /* Check ge3-ge18 which are connected to NGIO interfaces
     * These port are forced to 1G, no autoneg, and full duplex.
     * They are connected to NGIO which are powered off at board init.
     * Check all these settings.
     */
    mptr = mptr_sav;
    while (mptr->slot_type >= 0) {
        if ((mptr->bcm_port_type == BCM_PTYPE_GE) && (mptr->slot_type != TGT_DEV_CPU)) {
	    port = mptr->bcm_port_num;
	    bcm_port_enable_get(unit, port, &port_en);
	    bcm_port_learn_get(unit, port, (uint *)&learn_mode);
	    bcm_port_linkscan_get(unit, port, &linkscan);
	    bcm_port_speed_get(unit, port, &speed);
	    bcm_port_duplex_get(unit, port, &full_duplex);
	    bcm_port_autoneg_get(unit, port, &autoneg);

	    if ((port_en != TRUE) || 
		(learn_mode != (BCM_PORT_LEARN_ARL | BCM_PORT_LEARN_FWD)) ||
		(linkscan != BCM_LINKSCAN_MODE_SW) ||
		(speed != 1000) ||
		(full_duplex != TRUE) ||
		(autoneg != FALSE)) {

	        printf("port %d init error:\n"
		       "  enable= %d expected= %d\n"
		       "  learn_mode= %d expected=%d\n"
		       "  linkscan= %d expected= %d\n"
		       "  speed= %d expexted= %d\n"
		       "  full_duplex= %d expected %d\n"
		       "  autoneg= %d expected= %d\n",
		       port, port_en, TRUE,
		       learn_mode, (BCM_PORT_LEARN_ARL |BCM_PORT_LEARN_FWD),
		       linkscan, BCM_LINKSCAN_MODE_SW,
		       speed, 1000,
		       full_duplex, BCM_PORT_DUPLEX_FULL,
		       autoneg, FALSE);
		rc = FAIL;
		break;
	    }
	}
	mptr++;
    }

    /* Check xe0 which are connected to Cavium, and
     * xe1-xe2 which are connected toNGIO interfaces.
     * These port are forced to 10G, no autoneg.
     */
    mptr = mptr_sav;
    while (mptr->slot_type >= 0) {
        if ((mptr->bcm_port_type == BCM_PTYPE_XE) && 
	    ((mptr->slot_type == TGT_DEV_DP) || (mptr->slot_type == TGT_DEV_NGSM))) {
	    port = mptr->bcm_port_num;
	    bcm_port_enable_get(unit, port, &port_en);
	    bcm_port_learn_get(unit, port, (uint *)&learn_mode);
	    bcm_port_linkscan_get(unit, port, &linkscan);
	    bcm_port_speed_get(unit, port, &speed);
	    bcm_port_autoneg_get(unit, port, &autoneg);

	    if ((port_en != TRUE) || 
		(learn_mode != (BCM_PORT_LEARN_ARL | BCM_PORT_LEARN_FWD)) ||
		(linkscan != BCM_LINKSCAN_MODE_SW) ||
		(speed != 10000) ||
		(autoneg != FALSE)) {

	      printf("port %d init error:\n"
		     "  enable= %d expected= %d\n"
		     "  learn_mode= %d expected=%d\n"
		     "  linkscan= %d expected= %d\n"
		     "  speed= %d expexted= %d\n"
		     "  autoneg= %d expected= %d\n",
		     port, port_en, TRUE,
		     learn_mode, (BCM_PORT_LEARN_ARL |BCM_PORT_LEARN_FWD),
		     linkscan, BCM_LINKSCAN_MODE_SW,
		     speed, 10000,
		     autoneg, FALSE);
	      rc = FAIL;
	      break;
	    }
	}
	mptr++;
    }
#endif /* UTAH */

    return (rc);
}

/*
 * Function: cfg_10gkr_port
 * Configure dual interface ports (GE/10G-KR) on the bcm53403/53404 (greyhound) chip
 *
 * Input: port - bcm internal physical port number (starts from number 2)
 *        en_10gkr - 1: 10G, 0: 1G
 * Return: PASS
 */
int cfg_10gkr_port(int port, int en_10gkr)
{
    int unit = bcm_uid;
    int rv;
    char *gesw_port_name=get_gesw_pname(port);
    bcm_port_abil_t ability_mask;

    /* Per broadcom FAE's advice, disable AN mode before disbling port
     */
    rv = bcm_port_autoneg_set(unit, port, FALSE);
    rv += bcm_port_enable_set(unit, port, FALSE);

    /* fix NTC unstable issue on regression, 
     * need to setup 10G advertisement before changing speed on GE switch */
    /* BCM_GESW_10G_ADV = 0x220, value comes from bcm_port_advert_get() */
    /* BCM_GESW_DEFAULT_ADV = 0xe20, value get from bcm_port_advert_get() */
    /* Set to 10G-KR or GE interface */
    if (en_10gkr) {
        /* BCM_GESW_10G_ADV = 0x220, value comes from bcm_port_advert_get() */
        ability_mask = BCM_GESW_10G_ADV; 
        rv += bcm_port_advert_set(unit, port, ability_mask);
	rv += bcm_port_interface_set(unit, port, BCM_PORT_IF_KR);
	rv += bcm_port_speed_set(unit, port, 10000);
	rv += bcm_port_phy_control_set(unit, port, BCM_PORT_PHY_CONTROL_CL72, TRUE);

#define use_an_mode  1 /* Victory platforms use automge on 10G-KR ports */
#if use_an_mode
	/* In AN mode, FEC is setup in the config.bcm file.
	 * In Force Speed mode, FEC is setup with API
	 */
	//	printf("pfix-0 %s: Setup GESW 10gkr port %d (%s) to 10G AN mode\n", __FUNCTION__, port, gesw_port_name);
	rv += bcm_port_autoneg_set(unit, port, TRUE);
#else
	//	printf("pfix-1 %s: Setup GESW 10gkr port %d (%s) to 10G FS mode\n", __FUNCTION__, port, gesw_port_name);
	rv += bcm_port_phy_control_set(unit, port, BCM_PORT_PHY_CONTROL_FORWARD_ERROR_CORRECTION, TRUE);
#endif
    }
    else {
        // printf("pfix-2 %s: Setup GESW 10gkr port %d (%s) to 1G mode\n", __FUNCTION__, port, gesw_port_name);
        /* 1G speed */
        /* BCM_GESW_DEFAULT_ADV = 0xe20, value get from bcm_port_advert_get() */
        ability_mask = BCM_GESW_DEFAULT_ADV;
        rv += bcm_port_advert_set(unit, port, ability_mask);
        rv += bcm_port_interface_set(unit, port, BCM_PORT_IF_GMII);
	rv += bcm_port_speed_set(unit, port, 1000);
	/* CL72 is link training used in KR mode
	 */
	rv += bcm_port_phy_control_set(unit, port, BCM_PORT_PHY_CONTROL_FORWARD_ERROR_CORRECTION, FALSE);
	rv += bcm_port_phy_control_set(unit, port, BCM_PORT_PHY_CONTROL_CL72, FALSE);
    }

    if (rv == 0) {
        bcm_port_enable_set(unit, port, TRUE);
	return (PASS);
    }

    printf("%s: Setup GESW 10gkr port %d (%s) to %dG mode....failed.\n",
	   __FUNCTION__, port, gesw_port_name, (en_10gkr ? 10 : 1));
    return(FAIL);
}

/*
 * Function: init_dual_speed_ports
 * Init the 10gkr ports to 1G or 10G.
 *
 * Input: en_10gkr - 1: 10G, 0: 1G
 *
 * Return: PASS or FAIL
 */
static int init_dual_speed_ports(int en_10gkr)
{
    int rv, port;
    int dual_sp_port_start = 0;
    int dual_sp_port_end = -1;

    if (is_bcm(BCM53403_DID)) { /* Utah and neptune */
	if (is_utah()) {
	    /* Utah uses port 16-20 as 1G or 10G
	     */
	    dual_sp_port_start = 16;
	    dual_sp_port_end = 20;
	}
	else if (is_neptune() || is_vg450()) {
	    /* Neptune uses port 16-19 as dual speed and 20 as 10G only
	     */
	    dual_sp_port_start = 16;
	    dual_sp_port_end = 19;
	}
	else if (is_triton() || is_proteus()) {
	    /* Triton and Proteus use port 17-19 as dual speed and 20 as 10G only
	     */
	    dual_sp_port_start = 17;
	    dual_sp_port_end = 19;
	}
	else {
	    /* Neso uses port 19 as dual speed and 20 as 10G only
	     */
	    dual_sp_port_start = 19;
	    dual_sp_port_end = 19;
	}
    }
    else if (is_bcm(BCM53404_DID)) { /* Sword, Dagger */
        dual_sp_port_start = 11;
	dual_sp_port_end = 13;
    }
    for (port=dual_sp_port_start; port <= dual_sp_port_end; port++) {
        rv = cfg_10gkr_port(port, en_10gkr);
	if (rv != PASS) {
	    printf("%s: Config BCM port %d (%s) to 1G mode failed. %s\n",
		   __FUNCTION__, port, get_gesw_pname(port), bcm_errmsg(rv));
	    return(FAIL);
	}
    }
    return(PASS);
}

/*
 * Function: host_cfg_bcm_ports
 * Host platforms configure the GESW ports for the platform.
 *
 * Input: void
 *
 * Return: PASS or FAIL
 */
int host_cfg_bcm_ports(void)
{
    int rv, unit, bcm_hg_port, fpga_xaui_modid;

    unit = bcm_uid;

    if (is_usd_machines()) {
        if (is_plat_10gkr_capable()) {
	    bcm_hg_port = 2;

	    /* Set greyhound chip port-2 xe0 (interface with FPGA XAUI) to
	     * XGMII
	     */
	    rv = bcm_port_enable_set(unit, bcm_hg_port, FALSE);
	    rv += bcm_port_interface_set(unit, bcm_hg_port, BCM_PORT_IF_XGMII);
	    rv += bcm_port_autoneg_set(unit, bcm_hg_port, TRUE);
	    rv += bcm_port_speed_set(unit, bcm_hg_port, 10000);
	    rv += bcm_port_enable_set(bcm_uid, bcm_hg_port, TRUE);
	    if (rv < 0) {
	        printf("%s: Fail to set BCM xe0 to XAUI mode. %s\n",
		       __FUNCTION__, bcm_errmsg(rv));
		return(FAIL);
	    }

	    /* Init dual speed ports to 1G. The rc.soc script init all dual
	     * speed ports to 10G. We init these ports that will be
	     * changeable to 1G at beginning of diag.
	     */
	    if (init_dual_speed_ports(0) != PASS) {
	        return(FAIL);
	    }
	}
	else {
	  //	    printf("USD plat is not 10gkr capable\n");
	    bcm_hg_port = 26;
	}

	/* Set the modid for the device
	 */
	fpga_xaui_modid = 1;
	rv = bcm_stk_modport_set(unit, fpga_xaui_modid, bcm_hg_port);
	if (rv < 0) {
	    printf("%s: Fail to set BCM HG2 modport modid %d on port %d. %s\n",
		   __FUNCTION__, fpga_xaui_modid, bcm_hg_port, bcm_errmsg(rv));
	    return(FAIL);
	}
    }
    else if (is_ntpn_machines() || is_vg450()) {
        /* Init dual speed ports to 1G. The rc.soc script init all dual
	 * speed ports to 10G. We init these ports that will be
	 * changeable to 1G at beginning of diag.
	 */
        if (init_dual_speed_ports(0) != PASS) {
	    return(FAIL);
	}
    }
    return(PASS);
}

/*
 * Function: set_bcm_port_mode
 * Set the port mode of the GESW ports for the platform.
 * This function follows the platform BCM SDK rc.soc file.
 *
 * Input: void
 *
 * Return: void
 */
void set_bcm_port_mode(void)
{
    int unit = bcm_uid;
    char *cmd;
    int print_cmd = FALSE;

    if (is_usd_machines()) {
        cmd = "port ge linkscan=on autoneg=off speed=0 fullduplex=true framemax=12000 txpause=false rxpause=false";
	exec_bcm_shell_cmd(unit, cmd, print_cmd);

	cmd = "port hg0 encap=HIGIG2 linkscan=on autoneg=on speed=0 fullduplex=true framemax=12000 txpause=false rxpause=false";
	exec_bcm_shell_cmd(unit, cmd, print_cmd);

	cmd = "port xe linkscan=on autoneg=off speed=0 fullduplex=true framemax=12000 txpause=false rxpause=false";
	exec_bcm_shell_cmd(unit, cmd, print_cmd);
    }
    else if (is_overlord() || is_juno()) {
        cmd = "port ge0-ge2 linkscan=on autoneg=on speed=0 fullduplex=true framemax=12000 txpause=false rxpause=false";
        exec_bcm_shell_cmd(unit, cmd, print_cmd);

        cmd = "port ge3-ge23 linkscan=on autoneg=off speed=0 fullduplex=true framemax=12000 txpause=false rxpause=false";
	exec_bcm_shell_cmd(unit, cmd, print_cmd);

	cmd = "port xe linkscan=on autoneg=off speed=0 fullduplex=true framemax=12000 txpause=false rxpause=false";
	exec_bcm_shell_cmd(unit, cmd, print_cmd);
    }
    else { /* Neptune, triton, proteus, neso */
        /* set ge0 and xe5 to an=on. They are connected to x86 CPU 
	 */
        cmd = "port ge ls=on an=off sp=0 fd=true fm=12000 tpau=false rpau=false";
        exec_bcm_shell_cmd(unit, cmd, print_cmd);
        cmd = "port ge0 ls=on an=on sp=0 fd=true fm=12000 tpau=false rpau=false";
        exec_bcm_shell_cmd(unit, cmd, print_cmd);

	cmd = "port xe ls=on an=off sp=0 fd=true fm=12000 tpau=false rpau=false";
	exec_bcm_shell_cmd(unit, cmd, print_cmd);

	/* Neptune xe5 connects to x86 in 10G-KR mode with autoneg=on
	 *  with FEC turned on in the config.bcm file
	 * On Triton, Proteus, and Neso, this port's ID is xe4.
	 */
	if (is_neptune() || is_vg450()) {
	    cmd = "port xe5 ls=on an=on if=kr adv=10g speed=0 fd=true fm=12000 tpau=false rpau=false";
	}
	else {
	    cmd = "port xe4 ls=on an=on if=kr adv=10g speed=0 fd=true fm=12000 tpau=false rpau=false";
	}
	exec_bcm_shell_cmd(unit, cmd, print_cmd);
    }
    // printf("BCM port modes initialized\n");
}

/*
 * Function: ovld_gesw_init
 * Init the GESW for overlord appliaction. This function follows the
 * steps provide in the BCM SDK rc.soc file.
 *
 * Input: void
 *
 * Return: PASS or FAIL
 */
int ovld_gesw_init(void)
{
    int unit = bcm_uid;

    diag_rc_load(unit);
    set_bcm_port_mode();

    if (host_cfg_bcm_ports() != PASS) {
        return(FAIL);
    }
    return(PASS);
}
   
/*
 * Function: check_bcm_scripts
 * This function checks if the correct rc.soc, and config.bcm
 * are in the current directory.
 *
 * Input: void
 *
 * Return: PASS/FAIL
 */
int check_bcm_scripts(void)
{
    FILE *fp;
    char *fname_hlx[2] = {"rc.soc", "config.bcm"};
    char *fname_gh403[2] = {"rc.soc403", "config.bcm403"};
    char *fname_gh404[2] = {"rc.soc404", "config.bcm404"};
    char **fn_ptr;
    int len, ii;
    char *config_file, *config_temp;

    if (is_bcm(BCM53403_DID)) { /* Utah, Neptune, Triton, Proteus, Neso */
        /* fname_gh403 is the scripts for the BCM53403 chip
	 */
        fn_ptr = fname_gh403;
    }
    else if (is_bcm(BCM53404_DID)) { /* Sword, Dagger */
        /* fname_gh404 is the scripts for the BCM53404 chip
	 */
        fn_ptr = fname_gh404;
    }
    else {
        /* fname_hlx is the scripts for the BCM56321 chip
	 */
        fn_ptr = fname_hlx;      
    }

    /* Check if rc.soc and config.bcm exist in the current directory
     */
    for (ii=0; ii < 2; ii++) {
        if ((fp = fopen(fn_ptr[ii], "r")) == NULL) {
	    printf("Missing script %s in current direcory for GESW init\n", fn_ptr[ii]);
	    return(FAIL);
	}
	fclose(fp);
    }

    /* Set the rc.soc and config.bcm files for the platform
     * This set is leveraged from BCM SDK.
     */
    diag_rc_set(bcm_uid, fn_ptr[0]);
    config_file = fn_ptr[1];

    len = sal_strlen(config_file);
    if ((config_temp = sal_alloc(len+5, NULL)) != NULL) {
        sal_strcpy(config_temp, config_file);
	sal_strcpy(&config_temp[len], ".tmp");
	sal_config_file_set(config_file, config_temp);
	sal_free(config_temp);
    }
    return(PASS);
}

/*
 * Function: bcm_gesw_config
 * This function is leveraged from the BCM SDK diag code which sets up
 * the environment for the BCM chip
 *
 * Input: void
 *
 * Return: 0 for success, -1 for error
 */
int
bcm_gesw_config (void)
{
    if (is_gesw_initialized()) {
        printf("Skip GESW config. It has been done.\n");
	return(0);
    }

    printf("\nInitialize GE switch\n");

    if (check_bcm_scripts() == FAIL) {
        return(-1);
    }

    if (sal_core_init() < 0 || sal_appl_init() < 0) {
	printf("SAL Initialization failed\r\n");
	return(-1);
    }

    /* Init the BCM shell
     */
    host_init_bcm_shell();

    if (is_ntpn_machines() || is_vg450()) {
        /* Neptune's rc.soc403 set an=off on all port because BCM shell
	 * port self tests invoke the rc.soc file and these tests do not
	 * work with KX and KR mode with an=on. Neptune need to call
	 * set_bcm_port_mode to set the 2 ports (ge0 and xe5) connected 
	 * to CPU with an=on for normal platform support.
	 */
        set_bcm_port_mode();
    }
    /* pfix-dbg */

    if (host_cfg_bcm_ports() != PASS) {
        return(-1);
    }

    set_gesw_init_state();

    return (0);
}

/*
 * Function: bde_create
 * This function is copied from the BCM SDK. It creates an pci
 * instance of the BCM GE switch chip in the diag application
 * program
 *
 * Input: void
 *
 * Return: 0 for success
 */
int
bde_create(void)
{	
    linux_bde_bus_t bus;

    /* The followin defines are set in the Makefile for the platform
     */
    bus.be_pio = SYS_BE_PIO;
    bus.be_packet = SYS_BE_PACKET;
    bus.be_other = SYS_BE_OTHER;

    return linux_bde_create(&bus, &bde);
}

/*
 * Function: pci_dma_putw
 * This function is copied from the BCM SDK and is required when
 * linking the BCM library.
 */
int pci_dma_putw(pci_dev_t *dev, uint32 addr, uint32 data)
{
    *(uint32 *) ((uint64)addr) = data;
    return 0;
}

/*
 * Function: pci_dma_puth
 * This function is copied from the BCM SDK and is required when
 * linking the BCM library.
 */
int pci_dma_puth(pci_dev_t *dev, uint32 addr, uint16 data)
{
    *(uint16 *) ((uint64)addr) = data;
    return 0;
}

/*
 * Function: pci_dma_putb
 * This function is copied from the BCM SDK and is required when
 * linking the BCM library.
 */
int pci_dma_putb(pci_dev_t *dev, uint32 addr, uint8 data)
{
    *(uint8 *) ((uint64)addr) = data;
    return 0;
}

/*
 * Function: pci_dma_getw
 * This function is copied from the BCM SDK and is required when
 * linking the BCM library.
 */
uint32 pci_dma_getw(pci_dev_t *dev, uint32 addr)
{
    uint32 data;
    data = *(uint32 *) ((uint64)addr);
    return data;
}

/*
 * Function: pci_dma_geth
 * This function is copied from the BCM SDK and is required when
 * linking the BCM library.
 */
uint16 pci_dma_geth(pci_dev_t *dev, uint32 addr)
{
    uint16 data;
    data = *(uint16 *) ((uint64)addr);
    return data;
}

/*
 * Function: pci_dma_getb
 * This function is copied from the BCM SDK and is required when
 * linking the BCM library.
 */
uint8 pci_dma_getb(pci_dev_t *dev, uint32 addr)
{
    uint8 data;
    data = *(uint8 *) ((uint64)addr);
    return data;
}

/*
 * Function: pci_print_all
 * This function is copied from the BCM SDK and is required when
 * linking the BCM library.
 */
void pci_print_all(void)
{
    int device;

    sal_printf("Scanning function 0 of devices 0-%d\n", bde->num_devices(BDE_SWITCH_DEVICES) - 1);
    sal_printf("device fn venID devID class  rev MBAR0    MBAR1    IPIN ILINE\n");

    for (device = 0; device < bde->num_devices(BDE_SWITCH_DEVICES); device++) {
	uint32		vendorID, deviceID, class, revID;
	uint32		MBAR0, MBAR1, ipin, iline;
	
	vendorID = (bde->pci_conf_read(device, PCI_CONF_VENDOR_ID) & 0x0000ffff);
	
	if (vendorID == 0)
	    continue;
	

#define CONFIG(offset)	bde->pci_conf_read(device, (offset))

	deviceID = (CONFIG(PCI_CONF_VENDOR_ID) & 0xffff0000) >> 16;
	class    = (CONFIG(PCI_CONF_REVISION_ID) & 0xffffff00) >>  8;
	revID    = (CONFIG(PCI_CONF_REVISION_ID) & 0x000000ff) >>  0;
	MBAR0    = (CONFIG(PCI_CONF_BAR0) & 0xffffffff) >>  0;
	MBAR1    = (CONFIG(PCI_CONF_BAR1) & 0xffffffff) >>  0;
	iline    = (CONFIG(PCI_CONF_INTERRUPT_LINE) & 0x000000ff) >>  0;
	ipin     = (CONFIG(PCI_CONF_INTERRUPT_LINE) & 0x0000ff00) >>  8;
	
#undef CONFIG

	sal_printf("%02x  %02x %04x  %04x  "
		   "%06x %02x  %08x %08x %02x   %02x\n",
		   device, 0, vendorID, deviceID, class, revID,
		   MBAR0, MBAR1, ipin, iline);
    }
}

/******** History ******** 
$Log: bcm_gesw_init.c,v $
Revision 1.20  2018/05/18 09:24:50  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.19  2017/02/06 08:36:15  alpeng
fixed NTC 10G-KR link up unstable issue

Revision 1.18.34.8  2017/11/27 06:08:40  leschen
Initial check in to support VG450.

Revision 1.18.34.7  2017/04/05 06:41:58  leschen
Sync with <ng_diag-tag-032917>

Revision 1.18.34.6  2017/03/14 21:30:47  ptong
Fix set_bcm_port_mode() to set xe5 for Neptune but xe4 for MLKs to connect to x86 10G-KR port

Revision 1.18.34.5  2016/12/19 22:17:22  ptong
Added GESW advertisement setting in cfg_10gkr_port()

Revision 1.18.34.4  2016/10/27 21:49:40  ptong
Completed BCM self tests on Neptune

Revision 1.18.34.3  2016/10/17 19:22:57  ptong
Cosmetic change

Revision 1.18.34.2  2016/10/17 00:19:33  ptong
10G-KR modules now is supported on Neptune

Revision 1.18.34.1  2016/06/10 18:25:38  ptong
Enhance GESW support to Neptune

Revision 1.19  2017/02/06 08:36:15  alpeng
fixed NTC 10G-KR link up unstable issue

Revision 1.18  2014/08/15 23:37:27  ptong
Minor fix

Revision 1.17  2014/08/06 21:29:29  ptong
Use BRCM SDK-LGA20140718 for Greyhound. Support autoneg on KR ports

Revision 1.16  2014/07/23 02:04:31  ptong
BCM53404 doesn't have port 14

Revision 1.15  2014/06/24 22:17:50  ptong
Init all greyhound ports to 1G SGMII

Revision 1.14  2014/06/19 22:00:51  ptong
Incorporated BCM sdk-6.4.1-EA2 for 10G-KR interface. New code works on O2, June, and non Greyhound USD regression

Revision 1.13  2014/05/08 23:11:29  ptong
Remove BCM KNET code

Revision 1.12  2014/05/02 21:55:00  ptong
Check for rc.soc and config.bcm

Revision 1.11  2014/04/20 05:39:05  ptong
Comment out cfg_10gkr_port() to fix compile error. It is only for greybound bring-up

Revision 1.10  2014/04/18 00:18:50  ptong
Prepare to support Greyhound 10G-KR bring-up

Revision 1.9  2014/04/02 21:47:51  ptong
Use port mapping between BCM and platform to support the new BCM chip

Revision 1.8  2014/03/13 18:32:31  ptong
Remove gesw_info_init and unused code

Revision 1.7  2013/11/26 08:40:35  hroni
fix compiler warning

Revision 1.6  2013/10/14 06:59:38  hroni
add set modport to 26 in ovld_gesw_init()

Revision 1.5  2013/08/08 23:28:56  ptong
change ge port init for utah

Revision 1.4  2013/07/03 23:46:20  ptong
Modify GE switch config to support Utah

Revision 1.3  2013/06/14 00:10:46  ptong
Added BCM KNET for Utah

Revision 1.2  2013/05/09 23:50:56  ptong
Add framemax to port config

Revision 1.1  2013/05/09 05:42:35  alpeng
moving overlord common code from x86

Revision 1.12  2013/01/08 01:18:14  ptong
Added function headers

Revision 1.11  2012/10/23 22:52:37  ptong
Added function to check bcm port init after system comes up

Revision 1.10  2012/07/20 18:53:00  ptong
Add retry to BCM switch init

Revision 1.9  2012/06/05 11:44:36  palin2
Clean up compiler warnings.

Revision 1.8  2012/06/04 10:35:16  palin2
Clean up compiler warnings.

Revision 1.7  2012/06/02 00:29:50  ptong
Change ge0-ge2 to an=on

Revision 1.6  2012/05/15 17:57:12  ptong
Change diag name string from onet_lnx to o2x86_lnx

Revision 1.5  2012/05/04 23:48:57  ptong
Improve BCM init process and test message printing

Revision 1.4  2012/04/28 00:40:40  ptong
Change port setup to not auto-neg

Revision 1.3  2012/04/03 01:46:18  ptong
Replace loading rc.soc with ovld_gesw_init() to avoid re-init the bcm drivers

Revision 1.2  2012/03/28 00:38:20  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:08  ptong
Initial archive of ng_diag module


$Endlog$
*/
