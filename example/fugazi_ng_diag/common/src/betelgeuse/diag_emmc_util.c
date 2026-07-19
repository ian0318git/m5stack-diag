/* $Id: diag_emmc_util.c,v 1.2 2019/01/10 06:36:22 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/diag_emmc_util.c,v $
 *------------------------------------------------------------------
 *
 * diag_emmc_util.c
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------------------
 */

#include "error.h"
#include "common.h"
#include "types.h"
#include "menu.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <asm/ioctl.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <inttypes.h>
#include <string.h>
#include <asm/byteorder.h>
#include <linux/mmc/ioctl.h>
#include <linux/fs.h>
#include <unistd.h>
#include "common_utils.h"
#include "nvmonvars.h"
#include "proto.h"
#include "diag_moka_fpga_lib.h"
#include "diag_emmc_lib.h"
#include "diag_emmc_util.h"


/*******************************************************************************
 *                          Function Declaration
 *******************************************************************************
 */
void emmc_util_entry(void);

/*******************************************************************************
 *                                Globals
 *******************************************************************************
 */
__u32  buffer_ptr[MMC_IOC_MAX_BYTES];

/******************************************************************************
 *                                 Menus
 ******************************************************************************/
/*
 * eMMC Utility menu
 */
static submenu_xtable_t emmc_util_tbl[] = {
    {"Enable eMMC pSLC mode",   (PFT)diag_emmc_pslc_fully_en_util,   0,
     0, 
     (type_t(*)())0,            0,
     (type_t(*)())0,            0},
    {"Show eMMC info",          (PFT)diag_show_emmc_info_util,       0,
     0, 
     (type_t(*)())0,            0,
     (type_t(*)())0,            0},
};

#define EMMC_UTIL_TBL_SIZE (sizeof(emmc_util_tbl) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t emmc_util_menu_pri_items[EMMC_UTIL_TBL_SIZE + MAX_BASE_ITEMS];
static mitem_t emmc_util_menu_sec_items[EMMC_UTIL_TBL_SIZE + MAX_BASE_ITEMS];

static menuinfo_t emmc_util_menu = {
    "%s Utility Menu",
    0,                          /* mtparam added by init_empty_menu */
    (PFT) show_endnote,         /* notes missing WICs in combos */
    0,                          /* use generic prompt */
    0,                          /* size (bumped by add_menu_item() */
    emmc_util_menu_pri_items,
};

menuinfo_t *emmc_util_menu_p = &emmc_util_menu;


/*******************************************************************************
 *
 * Function   : emmc_util_entry
 * Description: Entry functon of eMMC utility.
 * Inputs     : None
 * Outputs    : None
 *
 *******************************************************************************
 */
void emmc_util_entry (void)
{
    build_primary_submenu(emmc_util_tbl, EMMC_UTIL_TBL_SIZE,
                          "eMMC", &emmc_util_menu_p);
    build_secondary_submenu(emmc_util_tbl, EMMC_UTIL_TBL_SIZE,
                            emmc_util_menu_sec_items);

    menu(emmc_util_menu_p, emmc_util_menu_sec_items, 0);
}

/*******************************************************************************
 *
 * Function   : diag_emmc_pslc_fully_en_util
 * Description: Function to enable eMMC fully pSLC mode.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int diag_emmc_pslc_fully_en_util (void)
{
    char     *devname = PLAT_EMMC_DEVNAME;
    uint32_t cmd_arg = 0;
    uchar    byte_val = 0;
    char     ch = 0;
    char     record[1024];
    int      fd = -1;

    /* To manage eMMC access via ioctl() */
    mmc_ioc_cmd_set_data(mmc_local_cmd, buffer_ptr);

    /* Show current eMMC info */
    if (diag_show_emmc_info_util() != PASSED) {
        printf("%s(%d) Failed to show eMMC info.\n", __func__, __LINE__);
        return (FAILED);
    }

    fd = open(devname, O_RDWR);
    if (fd < 0) {
        printf("%s(%d) Failed to open device(%s): %s(%d)\n",
               __func__, __LINE__, devname, strerror(errno), errno);
        return (FAILED);
    }

    memset(buffer_ptr, 0, sizeof(buffer_ptr));

    /* 1. Read ECSD(Ext_CSD). */
    memset(record, 0, sizeof(record));
    sprintf(record, "8,0x00000,-0x1,0x0,0x200,0x1,0x0,0x0,0x0,0x0,0x0,0x0");
    if (diag_emmc_parse_and_issue_cmd(fd, record, ",") != PASSED) {
        printf("%s(%d) Failed to parse and issue command %s.\n",
               __func__, __LINE__, record);
        close(fd);
        return (FAILED);
    }

    byte_val = get_ecsd_byte_val((__u8 *)buffer_ptr, 156);
    if ((byte_val & 0x1) == 0x1) {
        printf("Current eMMC fully pSLC mode is enabled.\n");
        close(fd);
        return (PASSED);
    }

    printf("\nNote:\n"
           "1. Once eMMC pSLC mode is enabled, all current data or partitions"
           " in eMMC will be cleared out.\n"
           "2. Once eMMC pSLC mode is enabled, it can't be recovered.\n"
           "Still want to Enable pSLC mode?\n");
    printf("('y' for yes; or any other key to quit)\n");
    ch = getchar();
    if (ch != 'y') {
        printf("Stop by user request.\n");
        return (PASSED);
    }

    printf("Start to enable eMMC pSLC mode:\n");
    /* 1.1 Read ECSD[160] bit0 and bit1(both should be 1) to 
     *     verify the device supports enhanced mode.
     */
    printf("Check if current eMMC supports pSLC mode... ");
    byte_val = 0;
    byte_val = get_ecsd_byte_val((__u8 *)buffer_ptr, 160);

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("byte[160]= %#x.\n", byte_val);
    }

    if ((byte_val & 0x3) == 0x3) {
        printf("Supported.\n");
    } else {
        printf("This eMMC device doesn't support pSLC mode.\n");
        close(fd);
        return (FAILED);
    }

    /* 2. Set capacity mode to HIGH. */
    printf("Set capacity mode to High... ");
    memset(record, 0, sizeof(record));
    sprintf(record, "6,0x03AF0100,-0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0");
    if (diag_emmc_parse_and_issue_cmd(fd, record, ",") != PASSED) {
        printf("%s(%d) Failed to parse and issue command %s.\n",
               __func__, __LINE__, record);
        close(fd);
        return (FAILED);
    }
    printf("Done.\n");

    /* 3. Set enhanced user data area. */
    /* 3.1 CMD6 arg(0x03880000) */
    printf("Set pSLC area... ");
    memset(record, 0, sizeof(record));
    sprintf(record, "6,0x03880000,-0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0");
    if (diag_emmc_parse_and_issue_cmd(fd, record, ",") != PASSED) {
        printf("%s(%d) Failed to parse and issue command %s.\n",
               __func__, __LINE__, record);
        close(fd);
        return (FAILED);
    }

    /* 3.2 CMD6 arg(0x03890000) */
    memset(record, 0, sizeof(record));
    sprintf(record, "6,0x03890000,-0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0");
    if (diag_emmc_parse_and_issue_cmd(fd, record, ",") != PASSED) {
        printf("%s(%d) Failed to parse and issue command %s.\n",
               __func__, __LINE__, record);
        close(fd);
        return (FAILED);
    }

    /* 3.3 CMD6 arg(0x038A0000) */
    memset(record, 0, sizeof(record));
    sprintf(record, "6,0x038A0000,-0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0");
    if (diag_emmc_parse_and_issue_cmd(fd, record, ",") != PASSED) {
        printf("%s(%d) Failed to parse and issue command %s.\n",
               __func__, __LINE__, record);
        close(fd);
        return (FAILED);
    }

    /* 3.4 CMD6 arg(0x038B0000) */
    memset(record, 0, sizeof(record));
    sprintf(record, "6,0x038B0000,-0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0");
    if (diag_emmc_parse_and_issue_cmd(fd, record, ",") != PASSED) {
        printf("%s(%d) Failed to parse and issue command %s.\n",
               __func__, __LINE__, record);
        close(fd);
        return (FAILED);
    }

    /* 3.5 Set ECSD[140] ENH_SIZE_MULT_0 = ECSD[157].
     *     => CMD6 arg(0x038C ECSD[157])
     */
    memset(record, 0, sizeof(record));
    cmd_arg = (uint32_t)((0x3 << 24) |
                         (0x8C << 16) |
                         ((get_ecsd_byte_val((__u8 *)buffer_ptr, 157)) << 8));
    sprintf(record, "6,0x%08X,-0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0", cmd_arg);
    if (diag_emmc_parse_and_issue_cmd(fd, record, ",") != PASSED) {
        printf("%s(%d) Failed to parse and issue command %s.\n",
               __func__, __LINE__, record);
        close(fd);
        return (FAILED);
    }

    /* 3.6 Set ECSD[141] ENH_SIZE_MULT_0 = ECSD[158].
     *     => CMD6 arg(0x038D ECSD[158])
     */
    memset(record, 0, sizeof(record));
    cmd_arg = 0;
    cmd_arg = (uint32_t)((0x3 << 24) |
                         (0x8D << 16) |
                         ((get_ecsd_byte_val((__u8 *)buffer_ptr, 158)) << 8));
    sprintf(record, "6,0x%08X,-0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0", cmd_arg);
    if (diag_emmc_parse_and_issue_cmd(fd, record, ",") != PASSED) {
        printf("%s(%d) Failed to parse and issue command %s.\n",
               __func__, __LINE__, record);
        close(fd);
        return (FAILED);
    }

    /* 3.7 Set ECSD[142] ENH_SIZE_MULT_0 = ECSD[159].
     *     => CMD6 arg(0x038E ECSD[159])
     */
    memset(record, 0, sizeof(record));
    cmd_arg = 0;
    cmd_arg = (uint32_t)((0x3 << 24) |
                         (0x8E << 16) |
                         ((get_ecsd_byte_val((__u8 *)buffer_ptr, 159)) << 8));
    sprintf(record, "6,0x%08X,-0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0", cmd_arg);
    if (diag_emmc_parse_and_issue_cmd(fd, record, ",") != PASSED) {
        printf("%s(%d) Failed to parse and issue command %s.\n",
               __func__, __LINE__, record);
        close(fd);
        return (FAILED);
    }
    printf("Done.\n");

    /* 4. Set enhanced attribute. */
    /* 4.1 Set ECSD[156] ENH_USR = 0x01.
     *     => CMD6 arg(0x039C0100)
     */
    printf("Set pSLC attribute... ");
    memset(record, 0, sizeof(record));
    sprintf(record, "6,0x039C0100,-0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0");
    if (diag_emmc_parse_and_issue_cmd(fd, record, ",") != PASSED) {
        printf("%s(%d) Failed to parse and issue command %s.\n",
               __func__, __LINE__, record);
        close(fd);
        return (FAILED);
    }
    printf("Done.\n");

    /* 5. Complete configuration. */
    /* 5.1 Set ECSD[155] PARTITION_SETTING_COMPLETED = 0x01.
     *     => CMD6 arg(0x039B0100)
     */
    memset(record, 0, sizeof(record));
    sprintf(record, "6,0x039B0100,-0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0");
    if (diag_emmc_parse_and_issue_cmd(fd, record, ",") != PASSED) {
        printf("%s(%d) Failed to parse and issue command %s.\n",
               __func__, __LINE__, record);
        close(fd);
        return (FAILED);
    }

    /* 6. Power cycle(or RST_N). */
    printf("\nSoftware configuration process of enable eMMC pSLC mode is DONE.\n");
    printf("Please power cycle your unit to make it works!\n");

    close(fd);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : diag_show_emmc_info_util
 * Description : Function to show current eMMC info 
 * Inputs      : None
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int diag_show_emmc_info_util (void)
{
    ulong           numblocks = 0;
    char            *devname = PLAT_EMMC_DEVNAME;
    uchar           byte_val = 0;
    char            record[1024];
    int             fd = -1;
    plat_emmc_info_t emmc_info;

    /* To manage eMMC access via ioctl() */
    mmc_ioc_cmd_set_data(mmc_local_cmd, buffer_ptr);

    if (get_emmc_cid_info(&emmc_info) != PASSED) {
        printf("%s(%d) Failed to get eMMC info.\n", __func__, __LINE__);
        return (FAILED);
    }

    fd = open(devname, O_RDWR);
    if (fd < 0) {
        printf("%s(%d) Failed to open device(%s): %s(%d)\n",
               __func__, __LINE__, devname, strerror(errno), errno);
        return (FAILED);
    }

    /* Get number of eMMC blocks */
    ioctl(fd, BLKGETSIZE, &numblocks);

    /* Read ECSD(Ext_CSD). */
    memset(buffer_ptr, 0, sizeof(buffer_ptr));
    memset(record, 0, sizeof(record));

    sprintf(record, "8,0x00000,-0x1,0x0,0x200,0x1,0x0,0x0,0x0,0x0,0x0,0x0");
    if (diag_emmc_parse_and_issue_cmd(fd, record, ",") != PASSED) {
        printf("%s(%d) Failed to parse and issue command %s.\n",
               __func__, __LINE__, record);
        close(fd);
        return (FAILED);
    }

    /* Check ECSD[156] to see if pSLC mode is enabled. */
    byte_val = get_ecsd_byte_val((__u8 *)buffer_ptr, 156);

    /* 3. Show eMMC info */
    printf("\nCurrent eMMC(%s) info\n", devname);
    printf("Manuf Name  : %s\n", emmc_info.manf_name);
    printf("Product Name: %s\n", emmc_info.prod_name);
    printf("Size        : %.3f GB(%lu Bytes).\n",
           (double)((numblocks * mmc_local_cmd.blksz) / ONE_GB),
           (numblocks * mmc_local_cmd.blksz));
    printf("pSLC mode   : %s.\n",
           ((byte_val & EMMC_PSLC_ENABLE) == EMMC_PSLC_ENABLE) ?
           "Enabled" : "NOT enabled");

    close(fd);
    return (PASSED);
}

/*-------------------------------------------------
 * $Log: diag_emmc_util.c,v $
 * Revision 1.2  2019/01/10 06:36:22  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
