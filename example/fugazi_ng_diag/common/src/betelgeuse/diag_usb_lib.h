/* $Id: diag_usb_lib.h,v 1.2 2019/01/10 06:36:28 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/diag_usb_lib.h,v $
 *------------------------------------------------------------------
 * 
 * diag_usb_lib.h
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */


#define MAX_FILENAME_LENGTH     255
#define MAX_COMMAND_LENGTH      2048

#define USB_SLOT0 0

extern int ExecuteCmdbyPopen(char *cmd, char *retBuf, int sizeOfBuf);
extern int quiet_launch;

extern int usb_get_info(void);
extern int usb_get_speed(int);
extern int usb_get_info(void);
extern int diag_usb_util(int);
extern int usb_slot_tests(int);

/*-------------------------------------------------
 * $Log: diag_usb_lib.h,v $
 * Revision 1.2  2019/01/10 06:36:28  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
