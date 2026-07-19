/* $Id: reva_sm_def.h,v 1.2 2017/03/16 05:20:25 umlin Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/reva/sm/reva_sm_def.h,v $
 *------------------------------------------------------------------
 * reva_def.h 
 *      Reva projects - SM definitions and prototypes.
 *
 * Copyright (c) 2016-2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include "prince_def.h"
 
#ifndef __REVA_DEFS__
#define __REVA_DEFS__

#define REVA_MAX_CH_NUM      64

/* enum for async baud rate generator */
enum {
    REVA_AS_BPS_NONE = 0,
    REVA_AS_BPS_300,
    REVA_AS_BPS_600,
    REVA_AS_BPS_1200,
    REVA_AS_BPS_2400,
    REVA_AS_BPS_4800,
    REVA_AS_BPS_9600,
    REVA_AS_BPS_14400,
    REVA_AS_BPS_19200,
    REVA_AS_BPS_32K,
    REVA_AS_BPS_38400,
    REVA_AS_BPS_48K,
    REVA_AS_BPS_56K,
    REVA_AS_BPS_57600,
    REVA_AS_BPS_64K,
    REVA_AS_BPS_72K,
    REVA_AS_BPS_115200,
    REVA_AS_BPS_128K,
    REVA_AS_BPS_230400,
    REVA_AS_BPS_256K,
};

/*
 * External Functions
 */
extern int async_serial_channel_test (int);
extern int ngwic_reva_chan_lpbk_test(int);

extern char as_pid[20];

#endif /* end __REVA_DEFS__ */


/******** History ******** 
$Log: reva_sm_def.h,v $
Revision 1.2  2017/03/16 05:20:25  umlin
Reva-SM: Commit Reva-SM module side diag codes to main trunk

Revision 1.1.2.1  2016/10/18 22:05:19  umlin
Reva-SM: SM-Module side diag, refer to FPGA 24~63 ports memory mapping to add those async loopback test. Removed PPP loopback function because of FPGA PPP logic is removed due to FPGA resource constraint.



$Endlog$
*/
