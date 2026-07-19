/* $Id: platform_stub.h,v 1.2 2016/04/20 11:25:31 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/platform_stub.h,v $
 *------------------------------------------------------------------
 *
 * platform_stub.h - The header file for creating the dummy functions
 *                   compiler issue.
 *
 * Feb. 2015, Kody Ko
 *
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef _PLATFORM_STUB_H_
#define _PLATFORM_STUB_H_

#define SCC_I2C_IF      0x0008

extern uint host_ngio_10gkr_capability(uint, uint);

extern int ngiovm_present(void *p);
extern int ngiosm_present(void *p);
extern int ngiosm_enable(void *p);

extern type_t smartchip_authenticate_retest(uchar, uchar);
extern int smartchip_authenticate(uchar, uchar);
extern int is_10g_gesw(void);


#endif                          /* _PLATFORM_STUB_H_ */

/******** History ********
$Log: platform_stub.h,v $
Revision 1.2  2016/04/20 11:25:31  benchen2
add tachi fru portion

Revision 1.1.2.4  2015/08/31 06:42:08  tirawan
Ported legacy smart cookie to support Quack chip read as TAM library cookie read function doesn't work on Quack chip

Revision 1.1.2.3  2015/08/21 06:46:29  alpeng
support ge/xaui test for testcard; clean up repo;

Revision 1.1.2.2  2015/07/28 09:32:38  alpeng
adding entry for ngio test on platform_slot.c

Revision 1.1.2.1  2015/07/24 03:39:36  tirawan
Add FPGA I2C read/write function, ACT2 cookie read/write function

Revision 1.1.2.3  2015/03/05 03:35:00  umlin
ESPN: [All components] Follow Cisco's programming rules.

Revision 1.1.2.2  2015/02/25 07:46:16  kodko
Fixed ACT2 cont.dev_if_p is not initialized and show weird chars issues during programming.

Revision 1.1.2.1  2015/02/06 10:49:45  kodko
Sync latest ACT2 CLIP/SUDI and enable 0xe2 field in cookie_4_core.c.

$Endlog $
*/
