/* $Id: pty.h,v 1.2 2019/08/06 06:56:09 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nightwatch/pty.h,v $
 *------------------------------------------------------------------
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 * @file   pty.h
 * @brief  pseudo-terminal related
 *
 * Initial: Jan 2018, Frank Wu <fenwu2@cisco.com>
 * -----------------------------------------------------------------------------
 */

#ifndef __UTILS_PTY_H_INCLUDED__
#define __UTILS_PTY_H_INCLUDED__

int pty_exec(const char *cmd, int *child_pid,  int *ptm_fd);

#endif

/*-------------------------------------------------
$Log: pty.h,v $
Revision 1.2  2019/08/06 06:56:09  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.1.2.1  2019/05/30 05:33:34  mingding
CSCvk64124-29: Support PCIe-based Nightwatch Server Module

*/
