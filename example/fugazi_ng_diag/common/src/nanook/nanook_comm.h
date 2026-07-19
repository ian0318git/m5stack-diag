 /* $Id: nanook_comm.h,v 1.2 2019/12/11 10:10:33 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/nanook_comm.h,v $
 *------------------------------------------------------------------
 * 
 * nanook_comm.h
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#ifndef _NANOOK_COMM_H_
#define _NANOOK_COMM_H_

#define INFO_LEN                (10000)
#define RETRY_MAX             (3)

enum ether_interface {
    ETHER_INTERFACE_NIM = 0,
    ETHER_INTERFACE_AC3
};



extern int  nanook_mem_read32(uint, uint *);
extern int  nanook_mem_write32(uint, uint);
extern int exec_cmd (char *, char *, int);
extern uint enable_ether_interface(int);
extern int nanook_mem_write(unsigned long int, uint);

#endif /* _NANOOK_COMM_H_ */

/*-------------------------------------------------
 * $Log: nanook_comm.h,v $
 * Revision 1.2  2019/12/11 10:10:33  lucywang
 * Merged Nanook to main trunk
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
