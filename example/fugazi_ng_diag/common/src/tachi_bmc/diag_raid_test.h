/* $Id: diag_raid_test.h,v 1.2 2016/04/20 11:25:32 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_raid_test.h,v $
 *------------------------------------------------------------------
 *
 * diag_raid_test.h
 *
 * Copyright (c) 2015-2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#ifndef DIAG_RAID_TEST_H_
#define DIAG_RAID_TEST_H_

extern int diag_raidcard_build_test(int);
extern int diag_raid_io_test(void);

#define CH200 1
#endif /* DIAG_RAID_TEST_H_ */
/*---------------------------------------------------------------
$Log: diag_raid_test.h,v $
Revision 1.2  2016/04/20 11:25:32  benchen2
add tachi fru portion

Revision 1.1.2.3  2016/03/26 05:27:53  benchen2
add raid io interface test

Revision 1.1.2.2  2016/03/02 08:35:41  benchen2
add sbr vdd eeprom ping test

Revision 1.1.2.1  2015/11/13 07:19:23  benchen2
Add raid card entrance menu

$Endlog$
*/

