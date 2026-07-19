/* $Id: platform_emmc.c,v 1.2 2021/06/02 02:56:24 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/hightower-sub6/platform_emmc.c,v $
 *-----------------------------------------------------------------------------
 * 
 * Filename   : platform_emmc.c
 * Description: Highrise eMMC Library.
 *
 * Copyright (c) 2017 ~ 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *-----------------------------------------------------------------------------
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
#include "plat_defs.h"
#include "platform_emmc.h"
#include "hr_commn_util.h"

/*******************************************************************************
 *                          External Function Declaration
 *******************************************************************************
 */
extern void force_user_do_power_cycle(char *prompts[]);

/*******************************************************************************
 *                          Function Declaration
 *******************************************************************************
 */
int          emmc_pslc_fully_enable(int);
int          show_emmc_info(void);
static uchar get_ecsd_byte_val(uchar *, int);
static int   save_pattern(char *, unsigned int *, int);
static int   load_pattern(char *, unsigned int *, int *);
static int   mmc_fill_cmd_flag(uint32_t, uint32_t, uint *);
static int   smart_num_conversion(char *, long long int *);
static int   parse_and_issue_cmd(int, char *, char *);
static int   get_emmc_cid_info(highrise_emmc_info_t *);

/*******************************************************************************
 *                                Globals
 *******************************************************************************
 */
struct mmc_ioc_cmd mmc_local_cmd = {0};
__u32  buffer_ptr[MMC_IOC_MAX_BYTES];

/*******************************************************************************
 *
 * Function    : parse_and_issue_cmd
 * Description : Function to parse and issue MMC command.
 * Inputs      : fd      - file descriptor of eMMC driver
 *               *record - buffer to put command string
 *               *delim  - delimiter
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
static int parse_and_issue_cmd (int fd, char *record, char *delim)
{
    int           fld = 0, ctr = 0;
    char          *bash_cmd;
    long long int conv_num = 0;
    int           tran_size = 0, read_size = 0;
    uint          cmd_flag = 0;
    char          arr[MAXFLDS][MAXFLDSIZE]={{0x0}};

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("parse() --> cmd string = %s\n", record);
    }

    if (!strncmp(record, "/!", 2)) {
        bash_cmd = malloc(strlen(record) + 16);
        if (!bash_cmd) {
            printf("\tMalloc failed.\n");
            return (FAILED);
        }
        memset(bash_cmd, 0, strlen(record) + 16);
        system(strcpy(bash_cmd, record+2));

        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("parse() --> bash_cmd = %s\n", bash_cmd);
        }
        free(bash_cmd);
    } else if (strncmp(record, "//", strlen("//"))) { /*First 2 char not equal to "//" */
        char *c_ptr = strtok(record, delim);

        while (c_ptr != NULL) {
            strcpy(arr[fld], c_ptr);
            fld++;
            c_ptr = strtok(NULL, delim);
        }
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        for (ctr = 0; ctr < fld; ctr++) {
            /*print each field*/
            printf("\tField number %3d=%s\n", ctr, arr[ctr]);
        }
    }

    /* Fill the ioctl controll structure */
    /* Fill command OP code */
    if (smart_num_conversion(arr[0], &conv_num) != PASSED) {
        printf("%s(%d) Failed to conversion out OP code(%s).\n",
               __func__, __LINE__, arr[0]);
        return (FAILED);
    }
    mmc_local_cmd.opcode = conv_num;

    /* Fill command argument */
    conv_num = 0;
    if (smart_num_conversion(arr[1], &conv_num) != PASSED) {
        printf("%s(%d) Failed to conversion out argument(%s).\n",
               __func__, __LINE__, arr[1]);
        return (FAILED);
    }
    mmc_local_cmd.arg = conv_num;

    /* calculate flags if > 0 otherwise keep the passed value */
    conv_num = 0;
    if (smart_num_conversion(arr[2], &conv_num) != PASSED) {
        printf("%s(%d) Failed to conversion out %s.\n",
               __func__, __LINE__, arr[2]);
        return (FAILED);
    }

    if (conv_num < 0) {
        if (mmc_fill_cmd_flag(mmc_local_cmd.opcode,
                              mmc_local_cmd.arg,
                              &cmd_flag) != PASSED) {
            printf("%s(%d) Failed to get MMC command(OP: %d) flag.\n",
                   __func__, __LINE__, mmc_local_cmd.opcode);
            return (FAILED);
        }
        mmc_local_cmd.flags = cmd_flag;
    } else {
        /* Fill command flags */
        conv_num = 0;
        if (smart_num_conversion(arr[2], &conv_num) != PASSED) {
            printf("%s(%d) Failed to conversion out flags(%s).\n",
                   __func__, __LINE__, arr[2]);
            return (FAILED);
        }
        mmc_local_cmd.flags = (int)conv_num;
    }

    /* Fill command write flag */
    conv_num = 0;
    if (smart_num_conversion(arr[3], &conv_num) != PASSED) {
        printf("%s(%d) Failed to conversion out write flag(%s).\n",
               __func__, __LINE__, arr[2]);
        return (FAILED);
    }
    mmc_local_cmd.write_flag = (int)conv_num;

    /* Fill command block size(blksz) */
    conv_num = 0;
    if (smart_num_conversion(arr[4], &conv_num) != PASSED) {
        printf("%s(%d) Failed to conversion out block size(%s).\n",
               __func__, __LINE__, arr[2]);
        return (FAILED);
    }
    mmc_local_cmd.blksz = (int)conv_num;

    /* Fill command blocks */
    conv_num = 0;
    if (smart_num_conversion(arr[5], &conv_num) != PASSED) {
        printf("%s(%d) Failed to conversion out blocks(%s).\n",
               __func__, __LINE__, arr[5]);
        return (FAILED);
    }
    mmc_local_cmd.blocks = (int)conv_num;

    /* Fill command of check if it's acmd */
    conv_num = 0;
    if (smart_num_conversion(arr[7], &conv_num) != PASSED) {
        printf("%s(%d) Failed to conversion out is_acmd(%s).\n",
               __func__, __LINE__, arr[7]);
        return (FAILED);
    }
    mmc_local_cmd.is_acmd = (int)conv_num;

    /* Fill command timeout(ns) */
    conv_num = 0;
    if (smart_num_conversion(arr[8], &conv_num) != PASSED) {
        printf("%s(%d) Failed to conversion out timeout value in ns(%s).\n",
               __func__, __LINE__, arr[8]);
        return (FAILED);
    }
    mmc_local_cmd.data_timeout_ns = (int)conv_num;

    /* Fill command timeout(ms) */
    conv_num = 0;
    if (smart_num_conversion(arr[9], &conv_num) != PASSED) {
        printf("%s(%d) Failed to conversion out timeout value in ms(%s).\n",
               __func__, __LINE__, arr[9]);
        return (FAILED);
    }
    mmc_local_cmd.cmd_timeout_ms = (int)conv_num;

    /* Fill command postsleep min.(us) */
    conv_num = 0;
    if (smart_num_conversion(arr[10], &conv_num) != PASSED) {
        printf("%s(%d) Failed to conversion out postsleep min. in us(%s).\n",
               __func__, __LINE__, arr[10]);
        return (FAILED);
    }
    mmc_local_cmd.postsleep_min_us = (int)conv_num;

    /* Fill command postsleep max.(us) */
    conv_num = 0;
    if (smart_num_conversion(arr[11], &conv_num) != PASSED) {
        printf("%s(%d) Failed to conversion out postsleep max. in us(%s).\n",
               __func__, __LINE__, arr[11]);
        return (FAILED);
    }
    mmc_local_cmd.postsleep_max_us = (int)conv_num;

    /* load pattern from file 1.2.3 */
    tran_size = (mmc_local_cmd.blksz * mmc_local_cmd.blocks);
    if ((strncmp(arr[6], "0x0", strlen(arr[6]))) &&
        (mmc_local_cmd.write_flag == 1) &&
        (mmc_local_cmd.blocks > 0)) {

        if (load_pattern(arr[6], buffer_ptr, &read_size) != PASSED) {
            printf("%s(%d) Failed to load pattern from file.\n",
                   __func__, __LINE__);
            return (FAILED);
        }

        if (read_size != tran_size) {
            printf("%s(%d) Failed! read back size(%d) not match"
                   " transfer size(%d).\n",
                   __func__, __LINE__, read_size, tran_size);
            return (FAILED);
        }
    }

    /* Sending command down to kernel space */
    if (ioctl(fd, MMC_IOC_CMD, &mmc_local_cmd) != 0) {
        printf("%s(%d) Failed to sending command: %s(%d)\n",
               __func__, __LINE__, strerror(errno), errno);
        return (FAILED);
    }

    if ((mmc_local_cmd.write_flag == 0) && (mmc_local_cmd.blocks > 0)) {
        /* save pattern */
        if (strncmp(arr[6], "0x0", strlen(arr[6]))) {
            if (save_pattern(arr[6], buffer_ptr, tran_size) != PASSED) {
                printf("%s: Failed to save pattern to file.\n", __func__);
                return (FAILED);
            }
        }
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : save_pattern
 * Description : Function to save pattern buffer to file.
 * Inputs      : *filename - Filename that want save to
 *               *buffer   - Content to save
 *               filesize  - Size to save  
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
static int save_pattern (char *filename, unsigned int *buffer, int filesize)
{
    FILE *file_ptr = fopen(filename, "w");

    if (file_ptr == NULL) {
        printf("%s(%d) Failed to open file(%s): %s(%d)\n",
               __func__, __LINE__, filename, strerror(errno), errno);
        return (FAILED);
    }
 
    fwrite(buffer, 1, filesize, file_ptr);
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s(%d): Saving pattern to %s, pattern size = %dB.\n",
               __func__, __LINE__, filename, filesize);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : load_pattern
 * Description : Function to load pattern from file to buffer.
 * Inputs      : *filename - Filename that want save to
 *               *buffer   - Content to save
 *               *readsize - Size of read back content
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
static int load_pattern (char *filename, unsigned int *buffer, int *readsize)
{
    int  filesize = 0;
    FILE *file_ptr = fopen(filename, "r");

    if (file_ptr == NULL) {
        printf("%s(%d) Failed to open file(%s): %s(%d)\n",
               __func__, __LINE__, filename, strerror(errno), errno);
        return (FAILED);
    }

    filesize = fread(buffer, 1, MMC_IOC_MAX_BYTES, file_ptr);
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s(%d): Loading pattern from %s, pattern size = %dB.\n",
               __func__, __LINE__, filename, filesize);
    }
    if (!filesize) {
        printf("%s(%d): Failed to read from file(error = %d, EOF = %d)\n",
               __func__, __LINE__, ferror(file_ptr), feof(file_ptr));
        return (FAILED);
    }
    *readsize = filesize;

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : mmc_fill_cmd_flag
 * Description : Function to fill command flag base on its OP code. 
 * Inputs      : opcode  - MMC command OP code
 *               arg     - Arguement of command
 *               *flag_p - buffer to put the command flag
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
static int mmc_fill_cmd_flag (uint32_t opcode, uint32_t arg, uint *flag_p)
{
    unsigned int flag = 0;
    char         *wval = "0x0"; /* Optional file pointer to write_flag */
    char         *blkszval = "0x200"; /* Optional file pointer to blksz */
    char         *blocksval = "0x0"; /* Optional file pointer to blocks */

    switch (opcode) {
    case  MMC_GO_IDLE_STATE:  /* bc */
        if ((arg == 0xF0F0F0F0) || (arg == 0)) {
            flag = MMC_RSP_SPI_R1 | MMC_CMD_BC |MMC_RSP_NONE;
        }
        break;
    case MMC_SEND_OP_COND:
        flag = MMC_RSP_SPI_R1 | MMC_CMD_BCR |MMC_RSP_R3;
        break;
    case MMC_ALL_SEND_CID:
        flag = MMC_CMD_BCR |MMC_RSP_R2;
        break;
    case MMC_SET_RELATIVE_ADDR:
        flag = MMC_CMD_AC |MMC_RSP_R1;
        break;
    case MMC_SET_DSR:
        flag = MMC_CMD_BC |MMC_RSP_NONE;
        break;  
    case MMC_SLEEP_AWAKE:
        /* only for select one specify device */
        flag = MMC_RSP_R1 | MMC_CMD_AC;
        break;
    case MMC_SWITCH: /* ac [31:0] See below R1b */
        flag = MMC_CMD_AC |MMC_RSP_R1B;
        break;
    case MMC_SELECT_CARD:
        /* only for select one specify device */
        flag = MMC_RSP_R1B | MMC_CMD_AC;
        break;
    case MMC_SEND_EXT_CSD:
        flag = MMC_RSP_SPI_R1 | MMC_RSP_R1 | MMC_CMD_ADTC;
        blocksval = "0x1";
        blkszval = "0x200";
        break;
    case MMC_SEND_CSD:
        flag = MMC_RSP_R2 | MMC_CMD_AC;
        break;
    case MMC_SEND_CID:
        flag = MMC_RSP_R2 | MMC_CMD_AC;
        break;
    case MMC_READ_DAT_UNTIL_STOP:
        /* need to double check this one, never used */
        flag = MMC_RSP_R1 | MMC_CMD_ADTC;
        break;
    case MMC_STOP_TRANSMISSION:
        flag = MMC_RSP_SPI_R1B | MMC_RSP_R1B | MMC_CMD_AC;
        break;
    case MMC_SEND_STATUS:
        flag = MMC_RSP_SPI_R2 | MMC_RSP_R1 | MMC_CMD_AC;
        break;
    case MMC_GO_INACTIVE_STATE:
        /* need to double check this one */
        flag = MMC_CMD_AC;
        break;
    case MMC_SPI_READ_OCR:
        flag = MMC_RSP_SPI_R3;
        break;
    case MMC_SPI_CRC_ON_OFF:
        flag = MMC_RSP_SPI_R1;
        break;
    case MMC_SET_BLOCKLEN:
        flag = MMC_RSP_SPI_R1 | MMC_RSP_R1 | MMC_CMD_AC;
        break;
    case MMC_READ_SINGLE_BLOCK:
        flag = MMC_RSP_R1 | MMC_CMD_ADTC;
        blocksval = "0x1";
        blkszval = "0x200";
        break;
    case MMC_READ_MULTIPLE_BLOCK:
        flag = MMC_RSP_R1 | MMC_CMD_ADTC;
        break;
    case MMC_BUS_TEST_R:
        flag = MMC_RSP_R1 | MMC_CMD_ADTC;
        break;
    case MMC_WRITE_DAT_UNTIL_STOP:
        /* need to double check this one, never used */
        flag = MMC_RSP_R1 | MMC_CMD_ADTC;
        wval = "1";
        break;
    case MMC_SET_BLOCK_COUNT:
        /* need to double check this one */
        flag = MMC_RSP_R1 | MMC_CMD_AC;
        break;
    case MMC_WRITE_BLOCK:
        flag = MMC_RSP_R1 | MMC_CMD_ADTC;
        wval = "1";
        break;
    case MMC_WRITE_MULTIPLE_BLOCK:
        flag =  MMC_RSP_R1 | MMC_CMD_ADTC;
        wval = "1";
        break;
    case MMC_BUS_TEST_W:
        flag = MMC_RSP_R1 | MMC_CMD_ADTC;
        wval = "1";
        break;
    case MMC_PROGRAM_CID:
        /* need to double check this one */
        flag = MMC_RSP_R1 | MMC_CMD_ADTC;
        break;
    case MMC_PROGRAM_CSD:
        flag = MMC_RSP_R1 | MMC_CMD_ADTC;
        break;
    case MMC_SET_WRITE_PROT:
        /* need to double check this one */
        flag = MMC_RSP_R1B | MMC_CMD_AC;
        break;
    case MMC_CLR_WRITE_PROT:
        /* need to double check this one */
        flag = MMC_RSP_R1B | MMC_CMD_AC;
        break;
    case MMC_SEND_WRITE_PROT:
        flag = MMC_RSP_R1 | MMC_CMD_ADTC;
        break;
    case MMC_ERASE_GROUP_START:
        flag = MMC_RSP_R1 | MMC_CMD_AC;
        break;
    case MMC_ERASE_GROUP_END:
        flag = MMC_RSP_R1 | MMC_CMD_AC;
        break;
    case MMC_ERASE:
        flag = MMC_RSP_R1B | MMC_CMD_AC;
        break;
    case MMC_FAST_IO:
        flag = MMC_RSP_R4 | MMC_CMD_AC;
        break;
    case MMC_GO_IRQ_STATE:
        flag = MMC_RSP_R5 | MMC_CMD_BCR;
        break;
    case MMC_LOCK_UNLOCK:
        flag = MMC_RSP_R1B | MMC_CMD_ADTC;
        break;
    case MMC_APP_CMD:
        flag = MMC_RSP_R1 | MMC_CMD_AC;
        break;
    case MMC_GEN_CMD:
        flag = MMC_RSP_R1 | MMC_CMD_ADTC;
        blocksval = "0x1";
        break;
    case MMC_VEN_CMD60:
        flag = MMC_RSP_R1 | MMC_CMD_ADTC;
        wval = "1";
        break;
    case MMC_VEN_CMD61:
        flag = MMC_RSP_R1 | MMC_CMD_ADTC;
        break;
    case MMC_VEN_CMD62:
        flag = MMC_RSP_SPI_R1 | MMC_CMD_BC | MMC_RSP_NONE;
        break;
    case MMC_VEN_CMD63:
        flag = MMC_RSP_SPI_R1 | MMC_CMD_BC | MMC_RSP_NONE;
        break;
    default:
        printf("%s(%d) Unknown OP code(%d).\n", __func__, __LINE__, opcode);
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("flag = %d,\n", flag);
    }

    *flag_p = flag;
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : smart_num_conversion
 * Description : Function to convert character string to integer.
 * Inputs      : *str      - content of character string to convert
 *               *conv_num - buffer to put converted number 
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
static int smart_num_conversion (char *str, long long int *conv_num)
{
    char *endp;

    if (strlen(str) == 0) {
        /* Empty string */
        *conv_num = 0;
        return (PASSED);
    }

    *conv_num = strtoll(str, &endp, 0);
    if ((str != endp) && (*endp == '\0')) {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("%s(%d) Conversion %s to %lld\n",
                   __func__, __LINE__, str, *conv_num);
        }
        return (PASSED);
    }

    printf("%s(%d) Failed to conversion %s.\n",
           __func__, __LINE__, str);
    return (FAILED);
}

/*******************************************************************************
 *
 * Function    : get_ecsd_byte_val 
 * Description : Function to get specific byte value of eMMC ext_csd register.
 * Inputs      : data - content of ext_csd register
 *               byte_num - number of ext_csd register
 * Outputs     : The byte value of specific ext_csd register
 *
 *******************************************************************************
 */
static uchar get_ecsd_byte_val (uchar *data, int byte_num)
{
    return (data[byte_num]);
}

/*******************************************************************************
 *
 * Function   : emmc_pslc_fully_enable
 * Description: Function to enable HIGHRISE eMMC fully pSLC mode.
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int emmc_pslc_fully_enable (int opt)
{
    char     *devname = HIGHRISE_EMMC_DEVNAME;
    uint32_t cmd_arg = 0;
    uchar    byte_val = 0;
    char     ch = 0;
    char     record[1024];
    int      fd = -1;

    printf("Start to enable eMMC pSLC mode:\n");
    /* To manage eMMC access via ioctl() */
    mmc_ioc_cmd_set_data(mmc_local_cmd, buffer_ptr);

    /* Show current eMMC info */
    if (show_emmc_info() != PASSED) {
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
    if (parse_and_issue_cmd(fd, record, ",") != PASSED) {
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

    {
        /* remove all partitions, give a sanitized emmc to rcS to re-partition*/
        char *part_rmv[] = {
            "#!/bin/bash",
            "flag=1",
            "while [ $flag -eq 1 ];",
            "do",
            "   flag=0",
            "   p=$(fdisk -l /dev/mmcblk0 |grep 'mmcblk0p' |sed -n '1 p' |awk '{print $1}');",
            "   if [ x$p != 'x' ];",
            "   then",
            "       echo \"delete $p\"",
            "       n=$(fdisk -l /dev/mmcblk0 |grep 'mmcblk0p' |wc -l)",
            "       if [ $n -eq 1 ]",
            "       then",
            "           i=''",
            "       else",
            "           i=${p/*mmcblk0p/}",
            "       fi",
            "       echo -e \"d\\n$i\\nw\\n\" |fdisk /dev/mmcblk0",
            "       flag=1",
            "   fi",
            "done",
            NULL,
        };
        int   i = 0;
        FILE *fp = NULL;
        char  buf[128] = {[0 ... sizeof(buf) - 1] = 0};
        const char *fname = "./.rmv_all_emmc_part.sh";

        fp = fopen(fname, "w");
        if (fp) {
            for(i = 0; part_rmv[i]; i++) {
                fprintf(fp, "%s\n", part_rmv[i]);
            }
            fsync(fileno(fp));
            fclose(fp);

            snprintf(buf, sizeof(buf) - 1, "/bin/chmod +x %s && %s", fname, fname);
            system(buf);
        }
    }

    /* 2. Set capacity mode to HIGH. */
    printf("Set capacity mode to High... ");
    memset(record, 0, sizeof(record));
    sprintf(record, "6,0x03AF0100,-0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0");
    if (parse_and_issue_cmd(fd, record, ",") != PASSED) {
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
    if (parse_and_issue_cmd(fd, record, ",") != PASSED) {
        printf("%s(%d) Failed to parse and issue command %s.\n",
               __func__, __LINE__, record);
        close(fd);
        return (FAILED);
    }

    /* 3.2 CMD6 arg(0x03890000) */
    memset(record, 0, sizeof(record));
    sprintf(record, "6,0x03890000,-0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0");
    if (parse_and_issue_cmd(fd, record, ",") != PASSED) {
        printf("%s(%d) Failed to parse and issue command %s.\n",
               __func__, __LINE__, record);
        close(fd);
        return (FAILED);
    }

    /* 3.3 CMD6 arg(0x038A0000) */
    memset(record, 0, sizeof(record));
    sprintf(record, "6,0x038A0000,-0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0");
    if (parse_and_issue_cmd(fd, record, ",") != PASSED) {
        printf("%s(%d) Failed to parse and issue command %s.\n",
               __func__, __LINE__, record);
        close(fd);
        return (FAILED);
    }

    /* 3.4 CMD6 arg(0x038B0000) */
    memset(record, 0, sizeof(record));
    sprintf(record, "6,0x038B0000,-0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0");
    if (parse_and_issue_cmd(fd, record, ",") != PASSED) {
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
    if (parse_and_issue_cmd(fd, record, ",") != PASSED) {
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
    if (parse_and_issue_cmd(fd, record, ",") != PASSED) {
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
    if (parse_and_issue_cmd(fd, record, ",") != PASSED) {
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
    if (parse_and_issue_cmd(fd, record, ",") != PASSED) {
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
    if (parse_and_issue_cmd(fd, record, ",") != PASSED) {
        printf("%s(%d) Failed to parse and issue command %s.\n",
               __func__, __LINE__, record);
        close(fd);
        return (FAILED);
    }

    /* 6. Power cycle(or RST_N). */
    printf("\nSoftware configuration process of enable eMMC pSLC mode is DONE.\n");
    printf("Please power cycle your unit to make it works!\n");

    {
        char *prmpts[] = {
            "All data has lost on eMMC after pSLC mode is enabled.",
            NULL
        };
        force_user_do_power_cycle(prmpts);
    }

    close(fd);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : show_emmc_info
 * Description : Function to show current HIGHRISE eMMC info 
 * Inputs      : None
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int show_emmc_info (void)
{
    ulong           numblocks = 0;
    char            *devname = HIGHRISE_EMMC_DEVNAME;
    uchar           byte_val = 0;
    char            record[1024];
    int             fd = -1;
    highrise_emmc_info_t emmc_info;

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
    if (parse_and_issue_cmd(fd, record, ",") != PASSED) {
        printf("%s(%d) Failed to parse and issue command %s.\n",
               __func__, __LINE__, record);
        close(fd);
        return (FAILED);
    }

    /* Check ECSD[156] to see if pSLC mode is enabled. */
    byte_val = get_ecsd_byte_val((__u8 *)buffer_ptr, 156);

    /* 3. Show eMMC info */
    printf("\n"
           "%-16s: %s\n"
           "%-16s  %-6s : %s\n"
           "%-16s  %-6s : %s\n"
           "%-16s  %-6s : %.3f GB(%lu Bytes)\n"
           "%-16s  %s %s\n",
           "eMMC Info", devname,
           " ", "Manuf", emmc_info.manf_name,
           " ", "Prod", emmc_info.prod_name,
           " ", "Size", (double)((numblocks * mmc_local_cmd.blksz) / ONE_GB), (numblocks * mmc_local_cmd.blksz),
           " ", "pSLC mode", ((byte_val & EMMC_PSLC_ENABLE) == EMMC_PSLC_ENABLE) ? "Enabled" : "Disabled");

    close(fd);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : get_emmc_cid_info
 * Description : Function to get current HIGHRISE eMMC CID info 
 * Inputs      : *emmc_info - buffer to put eMMC info
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
static int get_emmc_cid_info (highrise_emmc_info_t *emmc_info)
{
    FILE  *fd = NULL;
    char  filename[64];
    char  buf[64];

    /* Get Manufacturer ID */ 
    memset(buf, 0, sizeof(buf));
    memset(filename, 0, sizeof(filename));
    sprintf(filename, HIGHRISE_EMMC_SYSFS_MANFID);
    fd = fopen(filename, "r");
    if (fd == NULL) {
        printf("%s(%d) Failed to fopen %s: %s(%d)\n",
               __func__, __LINE__, filename, strerror(errno), errno);
        return (FAILED);
    }

    fscanf(fd, "%[^\n]", buf);

    if (strstr(buf, EMMC_MICRON_MANFID_STR) != NULL) {
        sprintf(emmc_info->manf_name, "Micron(0x13)");
    } else if (strstr(buf, EMMC_TOSHIBA_MANFID_STR) != NULL) {
        sprintf(emmc_info->manf_name, "Toshiba(0x11)");
    } else if (strstr(buf, EMMC_HYNIX_MANFID_STR) != NULL) {
        sprintf(emmc_info->manf_name, "SK Hynix(0x90)");
    } else if (strstr(buf, EMMC_SAMSUNG_MANFID_STR) != NULL) {
        sprintf(emmc_info->manf_name, "Samsung(0x15)");
    } else if (strstr(buf, EMMC_WESTERN_DIGITAL_MANFID_STR) != NULL) { 
        sprintf(emmc_info->manf_name, "Western Digital(0x45)");
    } else {
        printf("%s(%d) Unknown Manf ID(%s).\n", __func__, __LINE__, buf);
        fclose(fd);
        return (FAILED);
    }
    fclose(fd);

    /* Get eMMC product name */ 
    memset(buf, 0, sizeof(buf));
    memset(filename, 0, sizeof(filename));
    sprintf(filename, HIGHRISE_EMMC_SYSFS_NAME);
    fd = fopen(filename, "r");
    if (fd == NULL) {
        printf("%s(%d) Failed to fopen %s: %s(%d)\n",
               __func__, __LINE__, filename, strerror(errno), errno);
        return (FAILED);
    }

    fscanf(fd, "%[^\n]", buf);
    sprintf(emmc_info->prod_name, buf);
    fclose(fd);

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : emmc_hwrst_enb_dis
 * Description : Function to set/get current hwrst enbale/disable status.
 * Inputs      :
 *               new        - new status:
 *                            0 :get current status
 *                            1 :for enable
 *                            2 :for disable
 *                       others :undefined
 *               post       - the final status after operation.
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int emmc_hwrst_enb_dis(int new, int *post)
{
    char            *devname = HIGHRISE_EMMC_DEVNAME;
    uchar           byte_val = 0;
    char            record[1024];
    int             fd = -1;
    uint32_t        val= 0;

    /* To manage eMMC access via ioctl() */
    mmc_ioc_cmd_set_data(mmc_local_cmd, buffer_ptr);

    fd = open(devname, O_RDWR);
    if (fd < 0) {
        printf("%s(%d) Failed to open device(%s): %s(%d)\n",
               __func__, __LINE__, devname, strerror(errno), errno);
        return (FAILED);
    }

    /* Read ECSD(Ext_CSD). */
    memset(buffer_ptr, 0, sizeof(buffer_ptr));
    memset(record, 0, sizeof(record));

    sprintf(record, "8,0x00000,-0x1,0x0,0x200,0x1,0x0,0x0,0x0,0x0,0x0,0x0");
    if (parse_and_issue_cmd(fd, record, ",") != PASSED) {
        printf("%s(%d) Failed to parse and issue command %s.\n",
               __func__, __LINE__, record);
        close(fd);
        return (FAILED);
    }

    /* Check ECSD[162] to see if hwrst has already enabled/disabled */
    byte_val = get_ecsd_byte_val((__u8 *)buffer_ptr, 162);

    if (post)
        *post = byte_val & 0x3;

    switch (byte_val & 0x3) {
    case 0:
        if (new == 0) {
            close(fd);
            return (PASSED);
        }
        break;
    case 1:
        close(fd);
        return new == 1 ? (PASSED) : (FAILED);
    case 2:
        close(fd);
        return new == 2 ? (PASSED) : (FAILED);
    default:
        close(fd);
        return (FAILED);
    }

    if (new == 1 || new == 2) {
        memset(record, 0, sizeof(record));

        val = (0x3 << 24) | (162U << 16) | (new << 8);
        sprintf(record, "6, 0x%08x,-0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0", val);
        if (parse_and_issue_cmd(fd, record, ",") != PASSED) {
            printf("%s(%d) Failed to parse and issue command %s.\n",
                   __func__, __LINE__, record);
            close(fd);
            return (FAILED);
        }
        if (post)
            *post = new;
        return (PASSED);
    }

    close(fd);
    return (FAILED);
}


/*-------------------------------------------------
 * $Log: platform_emmc.c,v $
 * Revision 1.2  2021/06/02 02:56:24  alpeng
 * merge sears into trunk
 *
 * Revision 1.1.4.1  2020/12/09 01:52:01  alpeng
 * use C comment
 *
 * Revision 1.1  2020/08/19 09:50:53  markzha
 * *** empty log message ***
 *
 * Revision 1.3  2019/01/18 05:54:46  yungchen
 * Merge Supernova branch to the main trunk (CSCvn79871)
 *
 * Revision 1.2  2017/09/04 16:09:41  palin2
 * Added utilities to enable fully eMMC pSLC mode and show eMMC info.(CSCvf82437)
 *
 * $Endlog$
 *-------------------------------------------------
 */

