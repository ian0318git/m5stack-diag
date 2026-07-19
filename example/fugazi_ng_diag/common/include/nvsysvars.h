/* $Id: nvsysvars.h,v 1.6 2014/02/18 09:11:12 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/nvsysvars.h,v $
 *------------------------------------------------------------------
 *
 * Copyright (c) 2010-2014 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __NVSYSVARS_H__
#define __NVSYSVARS_H__

#ifdef FORTITUDE
#include "../src/fortitude/nvmonvars.h"
#elif PRINCE
#include "../src/prince/nvmonvars.h"
#elif OTHER
/* TBD : the other platform*/
#else /* overlord */

#ifdef CVMX_DP /* overlord/cavium */
#include "../src/overlord/cavium/nvmonvars.h"
#else /* overlord/x86 */
#include "../src/overlord/nvmonvars.h"
#endif /* CVMX_DP */

#endif 

#endif /* __NVSYSVARS_H__ */

/* end of module */

/******** History ******** 
$Log: nvsysvars.h,v $
Revision 1.6  2014/02/18 09:11:12  alpeng
CSCul88171-3: remove useless files: mon_boot.h, c82576_ethmap.h, eth_frames.c, stack.h, sys_regs.h

Revision 1.5  2013/05/09 07:40:59  alpeng
modify include path for nvmonvars.h

Revision 1.4  2013/04/23 07:28:36  xiaoyizh
Add Prince support.

Revision 1.3  2012/07/18 22:59:29  ptong
Fix a problem so that (NVRAM)->diagflag is used correctly on Cavium data plane menu

Revision 1.2  2012/03/28 00:38:11  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:04  ptong
Initial archive of ng_diag module


$Endlog$
*/
