/* $Id: full_load.h,v 1.2 2018/02/09 09:56:56 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/full_load.h,v $
 *------------------------------------------------------------------
 *
 * full_load.h - Full loading utility
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __FULL_LOAD_H__
#define __FULL_LOAD_H__

#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <sys/types.h>  
#include <sys/socket.h>  
#include <sys/ioctl.h>  
#include <linux/sockios.h>  
#include <linux/types.h>
#include <sys/mman.h>
#include <unistd.h>

#define BUF_SIZE 512

extern void fload_start(void);

#endif /* __FULL_LOAD_H__ */

/*------------------------------------------------------------------
$Log: full_load.h,v $
Revision 1.2  2018/02/09 09:56:56  hondwang
Merge Star branch star-branch-c9xx to main trunk

Revision 1.1.4.2  2018/01/20 05:57:49  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.1.2.1  2017/09/20 08:18:09  lucywang
added full load utility


$Endlog$
*/

