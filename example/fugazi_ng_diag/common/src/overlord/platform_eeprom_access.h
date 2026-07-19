/* $Id: platform_eeprom_access.h,v 1.4 2020/01/09 01:02:19 jiajliu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/platform_eeprom_access.h,v $
 *------------------------------------------------------------------
 * platform_eeprom_access.h : header file for platform_eeprom_access.c
 *
 * Jan. 2007, 
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */
#ifndef __PLATFORM_EEPROM_ACCESS_H__
#define __PLATFORM_EEPROM_ACCESS_H__


#define INTEL_EEUPDATE_DIR   "/diag_utils/intel-eeupdate-tool"
#define PROGRAM_X86_MAC   "eeupdate_mac_program"
#define BCM57412_DIR "/curie-1RU-diag/"
#define BCM57412_DIR_2RU "/curie-2RU-diag/"
#define BCM57412_SCRIPT "./load.sh -m"
#define BCM57412_HOST_SCRIPT "./load.sh -dev 1-2 -m"
#define BCM57412_NIM_SCRIPT "./load.sh -dev 3-4 -m"
#define RM_BCM57412_DRIVER "rmmod bnxt_en"
#define INSERT_BCM57412_DRIVER "modprobe bnxt_en"

#define I350_PORTS 1
#define BCM57412_PORTS 2

#if 0
#define ETH0 0
#define ETH1 1 
#define ETH2 2 
#define ETH3 3 
#define ETH4 4 
#define ETH5 5 
#define ETH6 6 
#define ETH7 7 
#define ETH8 8 
#define ETH9 9 
#endif

#define NIC1 1 
#define NIC2 2 
#define NIC3 3 
#define NIC4 4 
#define NIC5 5 
#define NIC6 6 
#define NIC7 7 
#define NIC8 8 

extern int program_mac_eeprom(int port, unsigned char *mac_addr);
extern int display_mac_eeprom(int, int);

#endif

/*
$Log: platform_eeprom_access.h,v $
Revision 1.4  2020/01/09 01:02:19  jiajliu
Merge Curie 2RU to main trunk

Revision 1.3  2019/08/06 06:56:10  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.2.2.2  2019/07/24 08:32:35  alpeng
merge trunk to branch

Revision 1.2.2.1  2018/08/23 22:56:37  leschen
Add macro to support Curie MAC programming

Revision 1.2  2018/05/18 09:24:51  alpeng
Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.1.66.3  2017/02/28 23:32:48  ptong
Fix program_x86_mac to setup management and 10G port MAC addresses

Revision 1.1.66.2  2017/01/25 03:46:56  meho
Changed program x86 mac script.

Revision 1.1.66.1  2017/01/20 08:05:38  meho
Updated program GE/10G MAC menu.

Revision 1.1  2013/05/09 05:42:39  alpeng
moving overlord common code from x86

Revision 1.2  2012/03/28 00:38:23  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:09  ptong
Initial archive of ng_diag module


$Endlog$
*/
