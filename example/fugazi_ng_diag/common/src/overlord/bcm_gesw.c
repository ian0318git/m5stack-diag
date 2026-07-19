/* $Id: bcm_gesw.c,v 1.5 2014/10/17 22:04:35 ptong Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/bcm_gesw.c,v $
 *------------------------------------------------------------------
 *
 * bcm_gesw.c - These are function to setup the gesw for operation
 *
 * Oct 2011, Paul Tong
 *
 * Copyright (c) 2013-2014 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <signal.h>
#include <setjmp.h>

/* Include files from the BCM SDK
 */
#include "shared/bsl.h"
#include "shared/shr_bprof.h"

#include "sal/core/libc.h"
#include "sal/appl/pci.h"
#include "sal/appl/config.h"
#include "sal/core/boot.h"

#include "soc/mem.h"
#include "soc/memtune.h"
#include "soc/devids.h"
#include "soc/debug.h"
#include "soc/drv.h"
#include "soc/l2x.h"

#include "bcm/error.h"
#include "bcm/init.h"
#include "bcm/stat.h"
#include "bcm/link.h"
#include "bcm/port.h"
#include "bcm/rx.h"

#include "appl/diag/sysconf.h"
#include "appl/diag/bslmgmt.h"
#include "appl/diag/system.h"
#include "appl/diag/diag.h"
#include "appl/diag/dport.h"

/* End BCM SDK include files */

#include "queryflags.h"
#include "bcm_gesw.h"
#include "plat_defs.h"

#undef DEBUG
#define DEBUG  0
#define DEBUG_FORWARDING   0

/*
 * Function: gesw_build_pbmp_from_mask
 * construct a bcm port bit map from bit mask
 *
 * Input: bit_mask - the bit mask to convert to pbmp format
 *
 * Return: bpmp value
 */
bcm_pbmp_t
gesw_build_pbmp_from_mask(uint32 bit_mask)
{
    int i;
    bcm_pbmp_t pbmp;

    BCM_PBMP_CLEAR(pbmp);
    for (i=0; i < (sizeof(uint32)*8); i++) {
        if (bit_mask & 0x1) {
	  BCM_PBMP_PORT_ADD(pbmp, i);
	}
	bit_mask = bit_mask >> 1;
    }
    return (pbmp);
}

/*
 * Function: invoke_bcm_shell
 * This code is leveraged from the BCM SDK diag shell program. It
 * starts the BCM diag shell from the Overlord diagnostics program.
 *
 * Input: void
 *
 * Return: PASS
 */
int invoke_bcm_shell(void)
{
    sh_process(-1, "BCM", TRUE);
    return (PASS);
}

/*
 * Function: host_init_bcm_shell
 * This code is ported from the BCM SDK diag_shell in
 * /src/appl/diag/system.c. It init the GESW without launching
 * the diag shell. The shell BCM> can be launched with the
 * invoke_bcm_shell function.
 *
 * Input: void
 *
 * Return: PASS
 */
void host_init_bcm_shell(void)
{
    /* NOTE: This function is ported from diag_shell() of
     * the SDK system.c file. 
     * Please do no change it without knowing the effect of 
     * initializing the BCM shell
     */

    uint32  flags;
    char    *script;
    int     no_rc_warning = 1; 
    int i; 
    int         rv = BCM_E_NONE;

    /* This bcm mdebug_init defined in SDK /src/appl/diag/system.c
     */
    bslmgmt_init();
    diag_init();
    sysconf_init(); 

   /*
     * At boot time, probe for devices and attach the first one.
     * In PLISIM, this is not done; the probe and attach commands
     * must be given explicitly.
     */
    flags = sal_boot_flags_get(); /* flags is alwyas 0 on Utah */

    if (!(flags & BOOT_F_NO_PROBE)) {
        /* On Cisco platforms, we always probe
	 */
        SHR_BPROF_STATS_TIME(SHR_BPROF_SOC_PROBE) {
	    /* sysconf_probe prints:
	     * DMA pool size: 8388608
	     *  PCI unit 0: Dev 0x8403, Rev 0x01, Chip BCM53403_A0, Driver BCM53400_A0
	     */
            if ((sysconf_probe()) < 0) {
                cli_out("ERROR: PCI SOC device probe failed\n");
            }
        }

	/* soc_ndev is 1 since there is only 1 BCM chip on platform
	 */
        var_set_integer("units", soc_ndev, FALSE, FALSE);

        if (!(flags & BOOT_F_NO_ATTACH)) {
            for (i = 0; i < soc_all_ndev; i++) {
                SHR_BPROF_STATS_TIME(SHR_BPROF_SOC_ATTACH) {
		    /* sysconf_attach prints:
		     * SOC unit 0 attached to PCI device BCM53403_A0
		     */
		    rv = sysconf_attach(i); 
                }
                if (rv < 0) {
                    cli_out("ERROR: SOC unit %d attach failed\n", i);
                }
            } /* for */
        } else {
	    cli_out("Boot flags: Attach NOT performed\n");
	}
    } else {
        cli_out("Boot flags: Probe NOT performed\n");
    }

    /* Add backdoor for mem tuner to update system configuration */
    soc_mem_config_set = sal_config_set;

    /*
     * If a startup script is given in the boot parameters, attempt to
     * load it.  This script is for general system configurations such
     * as host table additions and NFS mounts.
     */
    if ((script = sal_boot_script()) != NULL) {
        sal_printf("n pfix-100-%s..................\n", __FUNCTION__);
        if (sh_rcload_file(-1, NULL, script, FALSE) != CMD_OK) {
	    cli_out("ERROR loading boot init script: %s\n", script);
        }
        no_rc_warning = 0;
     }
    
    /*
     * If a default init file is given, attempt to load it.
     */
    if (!(flags & BOOT_F_NO_RC)) {
        for (i = 0; i < soc_ndev; i++) {
	    if (soc_attached(i)) {
                sh_swap_unit_vars(i);

		/* diag_rc_load loads the rc.soc and prints:
		 * rc: unit 0 device BCM53403_A0
		 */
                if (diag_rc_load(i) != CMD_OK) {
                    cli_out("ERROR loading rc script on unit %d\n", i);
                }
            }
        }
    } else if (no_rc_warning) {
        cli_out("Boot flags: initialization scripts NOT loaded\n");
    }

    if (soc_ndev <= 0) {
        cli_out("No attached units.\n");
    }
}

/******** History ******** 
$Log: bcm_gesw.c,v $
Revision 1.5  2014/10/17 22:04:35  ptong
Update to use BCM sdk-xgs-robo-6.4.2

Revision 1.4  2014/06/19 22:00:51  ptong
Incorporated BCM sdk-6.4.1-EA2 for 10G-KR interface. New code works on O2, June, and non Greyhound USD regression

Revision 1.3  2014/03/13 18:32:31  ptong
Remove gesw_info_init and unused code

Revision 1.2  2013/05/28 18:05:04  ptong
Remove bcm/filter.h

Revision 1.1  2013/05/09 05:42:35  alpeng
moving overlord common code from x86

Revision 1.7  2013/01/08 01:18:14  ptong
Added function headers

Revision 1.6  2012/09/07 22:50:00  ptong
Code clean-up

Revision 1.5  2012/06/06 07:34:05  palin2
Clean up compiler warnings.

Revision 1.4  2012/06/05 11:44:36  palin2
Clean up compiler warnings.

Revision 1.3  2012/05/16 07:20:51  ptong
Add GESW port TX util

Revision 1.2  2012/03/28 00:38:20  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:07  ptong
Initial archive of ng_diag module


$Endlog$
*/
