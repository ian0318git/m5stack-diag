/* $Id: intel_tests.h,v 1.2 2016/04/20 11:25:27 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/intel_tests.h,v $
 *------------------------------------------------------------------
 *
 * intel_tests.h
 *
 * Jan 2015, Honda Wang adapted from Tachi-L.
 *
 * Copyright (c) 2016 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef _INTEL_TESTS_H_
#define _INTEL_TESTS_H_

#define WAIT_SYSTEM_CHECK_TIME	3
#define WAIT_DEUBG_BIOS_BOOT_TIME  210
#define WAIT_INTEL_LINUX_BOOT_TIME  (70 + WAIT_DEUBG_BIOS_BOOT_TIME)
#define PRE_PING_CHECK_LOOP	3
#define PING_CHECK_LOOP 30
#define BMC_STORE_PATH	"/mnt/datastore/"
#define TFTP_CMD "tftp -g -r"
#define TFTP_CMD_L "-l"
#define LS_L_CMD "ls -l"
#define GREP_E_CMD "grep -e"
#define GREP_N_CHECK_CMD "grep -n '[^0-9]0[^0-9]'"
#define SYS_OR_CMD "|"
#define LEWIS_IMAGE_PATH "/mnt/intel_emmc/"
#define LEWIS_IMAGE "diag-remote-image.bin"
#define LEWIS_CHECK "/tmp/lewis_done"
#define UNMNT_EMMC "umount"
#define CONNECT_EMMC_TO_INTEL "fx3sraidconf connect-single-emmc"

typedef void (*sighandler_t)(int);

enum lewis_state_en {
    CHECK_EMMC=0,
    EMMC_NOT_CONNECT,
    DOWNLOAD_IMAGE
};


/*-----------------------------------------------------------------------
 *  Externs                                                             *
 *----------------------------------------------------------------------*/
extern menuinfo_t intel_subtest_menu;
extern menuinfo_t *intel_submenup;
extern title_buf_t intel_subtest_header;
extern title_buf_t intel_subtest_title[];

extern int map_mainmem_test(int);
extern int test_memory_ecc();
extern int ethernet_tests(int);
extern int build_ge_phy_menu(int);
extern int check_intel_linux_ready(void);
extern int intel_linux_alive(void);
extern int intel_lewis_alive(void);

#endif /* INTEL_TESTS_H__ */
/*---------------------------------------------------------------
 $Log: intel_tests.h,v $
 Revision 1.2  2016/04/20 11:25:27  benchen2
 add tachi fru portion

 Revision 1.1.2.8  2016/03/29 08:54:36  hondwang
 Add ping test loop to 3 and remove recover function

 Revision 1.1.2.6  2016/02/20 16:20:18  hondwang
 Add CPU and PCI bus testing

 Revision 1.1.2.5  2016/01/20 07:13:56  hondwang
 Modify for INTEL linux and lewis check utility and INTEL NC flag

 Revision 1.1.2.4  2016/01/12 00:29:02  uid259484
 modify to add INTEL NC utility show HDD, DIMM and linux version.
 And add RAID card and BTB testing to daughter card item.

 Revision 1.1.2.3  2016/01/11 10:28:22  jimmyya
 Add lewis check functions

 Revision 1.1.2.2  2015/12/30 06:27:21  hondwang
 Add downlaod server path env require for image download

 Revision 1.1.2.1  2015/12/28 06:12:30  hondwang
 Add and modify files for INTEL NC command support


 $Endlog$
 */
