/* $Id: diag_emmc_lib.c,v 1.2 2021/09/24 01:21:05 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/elixir/diag_emmc_lib.c,v $
 *------------------------------------------------------------------
 *
 * diag_emmc_lib.c
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
#include "diag_emmc_test.h"
#include "linux_main.h"


/*******************************************************************************
 *                          Function Declaration
 *******************************************************************************
 */
uchar get_ecsd_byte_val(uchar *, int);
static int   save_pattern(char *, unsigned int *, int);
static int   load_pattern(char *, unsigned int *, int *);
static int   mmc_fill_cmd_flag(uint32_t, uint32_t, uint *);
static int   smart_num_conversion(char *, long long int *);
int diag_emmc_parse_and_issue_cmd(int, char *, char *);
int get_emmc_cid_info(plat_emmc_info_t *);
int emmc_slot_tests(void);


/*******************************************************************************
 *                                Globals
 *******************************************************************************
 */
struct mmc_ioc_cmd mmc_local_cmd = {0};
__u32  buffer_ptr[MMC_IOC_MAX_BYTES];

/*******************************************************************************
 *
 * Function    : diag_emmc_parse_and_issue_cmd
 * Description : Function to parse and issue MMC command.
 * Inputs      : fd      - file descriptor of eMMC driver
 *               *record - buffer to put command string
 *               *delim  - delimiter
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int diag_emmc_parse_and_issue_cmd (int fd, char *record, char *delim)
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
        bash_cmd = malloc(sizeof(record));
        system(strcpy(bash_cmd, record+2));

        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("parse() --> bash_cmd = %s\n", bash_cmd);
        }
        free(bash_cmd);
    } else if (strncmp(record, "//", strlen("//"))) {
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

    fclose(file_ptr);
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
        fclose(file_ptr);
        return (FAILED);
    }
    *readsize = filesize;

    fclose(file_ptr);
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
uchar get_ecsd_byte_val (uchar *data, int byte_num)
{
    return (data[byte_num]);
}

/*******************************************************************************
 *
 * Function    : get_emmc_cid_info
 * Description : Function to get current eMMC CID info 
 * Inputs      : *emmc_info - buffer to put eMMC info
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int get_emmc_cid_info (plat_emmc_info_t *emmc_info)
{
    FILE  *fd = NULL;
    char  filename[PLAT_EMMC_BUF_SIZE];
    char  buf[PLAT_EMMC_BUF_SIZE];

    /* Get Manufacturer ID */ 
    memset(buf, 0, sizeof(buf));
    memset(filename, 0, sizeof(filename));
    sprintf(filename, PLAT_EMMC_SYSFS_MANFID);
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
    } else if (strstr(buf, EMMC_WD_MANFID_STR) != NULL) {
        sprintf(emmc_info->manf_name, "WD(0x45)");
    } else {
        printf("%s(%d) Unknown Manf ID(%s).\n", __func__, __LINE__, buf);
        fclose(fd);
        return (FAILED);
    }
    fclose(fd);

    /* Get eMMC product name */ 
    memset(buf, 0, sizeof(buf));
    memset(filename, 0, sizeof(filename));
    sprintf(filename, PLAT_EMMC_SYSFS_NAME);
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
 * Function   :    get_emmc_size
 * Description:    get emmc size
 * Inputs     :    buffer to return size, buffer size & device name 
 * Outputs    :    PASSED/FAILED
 *
 *******************************************************************************
 */
int get_emmc_size (char* sysfilesize, int bufsize, char* dev_name)
{
    char cmd[MAX_COMMAND_LENGTH]={0};

    sprintf(cmd, "fdisk -l 2>/dev/null | grep Disk | grep -i -w %s | awk '{print $5}'", dev_name);
    
    if ( (ExecuteCmdbyPopen (cmd, sysfilesize, bufsize)) == 0 ) {
        return FAILED;
    }

    return PASSED;
}

/*-------------------------------------------------
 * $Log: diag_emmc_lib.c,v $
 * Revision 1.2  2021/09/24 01:21:05  harrchan
 * Collapse Elixir-branch to Main Trunk.
 *
 * Revision 1.1.2.2  2021/07/06 03:27:31  harrchan
 * Add emmc Manufacturer ID to support vendor Western Digital
 *
 * Revision 1.1.2.1  2020/09/09 09:08:06  illiu
 * First version which has been ported with Dreamliner and Marvel CPSS
 *
 * Revision 1.2  2019/01/10 06:36:22  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
