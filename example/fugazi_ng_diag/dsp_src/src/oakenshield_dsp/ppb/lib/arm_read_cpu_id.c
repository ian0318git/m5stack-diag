/* $Id: arm_read_cpu_id.c,v 1.2 2017/07/28 07:58:39 harrchan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/ppb/lib/arm_read_cpu_id.c,v $
 *------------------------------------------------------------------
 * arm_read_cpu_id.c
 *
 * Mar 2012, Smita Rane
 *
 * Copyright (c) 2012-2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include "libgeneric.h"
#include "lsi_sp27xx_reg.h"

inline int sp_readMPcpuid(void)
{
	int cpuid;
	__asm__ volatile("MRC p15, 0, %[result], c0, c0, 5" : [result] "=r" (cpuid));
	return (0x1&cpuid);
}

void
sp_ARM1release(void)
{
	/* configure ARM1 to boot from location 0 */
	REG32_RESET_BITS(LSI_SP27XX_ARMCTL_VINITHI_RA, LSI_SP27XX_VINITHI_ARM1_VINITHI_BM);

	/* release ARM1 from reset */
	REG32_RESET_BITS(LSI_SP27XX_CAR_ARMRSTCTL_RA , LSI_SP27XX_ARMRSTCTL_ARM1SWRST_BM);
}

void
sp_ARM1reset(void)
{
	/* ARM1 reset */
	REG32_SET_BITS(LSI_SP27XX_CAR_ARMRSTCTL_RA , LSI_SP27XX_ARMRSTCTL_ARM1SWRST_BM);
}
/******** History ********
$Log: arm_read_cpu_id.c,v $
Revision 1.2  2017/07/28 07:58:39  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.1.2.1  2017/06/29 08:14:34  harrchan
Initial commit code for Oakenshield

Revision 1.3  2012/09/10 06:32:49  srane
Add routine to reset ARM11 CPU1.

Revision 1.2  2012/05/10 22:48:10  srane
clean up and modify files for exception handling and TDM tests.

Revision 1.1  2012/04/18 09:47:30  srane
Initial checkin


$Endlog$
*/

