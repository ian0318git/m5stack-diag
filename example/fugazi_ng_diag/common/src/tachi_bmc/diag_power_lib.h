/* $Id: diag_power_lib.h,v 1.2 2016/04/20 11:25:32 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_power_lib.h,v $
 *------------------------------------------------------------------
 *
 * diag_power_lib.h - Header file for Power Library
 *
 * June 2015, Times Huang ported from mfgdiag
 *
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#ifndef __DIAG_POWER_LIB__
#define __DIAG_POWER_LIB__

#define PLT2_PWR_IOC_MAGIC              (0xB8)
#define PLT2_PWR_IOCGSTATE              _IOR( PLT2_PWR_IOC_MAGIC, 4, int )

#define INTEL_POWER_ON                  (1)
#define INTEL_POWER_OFF                 (2)
#define HOST_POWER_FILE                 "/dev/host_power"

extern void diag_intel_power_on(int);
extern int diag_intel_power_status(void);
extern int diag_intel_power_ctl(void);

#endif /* __DIAG_POWER_LIB__ */

/*---------------------------------------------------------------
$Log: diag_power_lib.h,v $
Revision 1.2  2016/04/20 11:25:32  benchen2
add tachi fru portion

Revision 1.1.2.2  2015/10/01 08:38:21  tirawan
Update Temperature sensor description and add Intel power on/off utility

Revision 1.1.2.1  2015/06/11 02:01:09  tirawan
Add files for Tachi BMC project


$Endlog$
*/
