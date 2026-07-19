 /* $Id: diag_emmc_util.c,v 1.2 2019/12/11 10:10:28 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/diag_emmc_util.c,v $
 *------------------------------------------------------------------
 *
 * diag_emmc_util.c - eMMC utility wraps.
 *
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <asm/ioctl.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <inttypes.h>
#include <string.h>
#include <asm/byteorder.h>
#include <linux/mmc/ioctl.h>
#include <linux/fs.h>
#include <unistd.h>
#include "endians.h"
#include "defs.h"
#include "types.h"
#include "nvsysvars.h"
#include "error.h"
#include "pcmap.h"
#include "strings.h"
#include "menu.h"
#include "common.h"
#include "cross_platform.h"
#include "mb_tests.h"
#include "plat_defs.h"
#include "setjmps.h"
#include "proto.h"
#include "dash_fpga.h"
#include "platform_fru.h"
#include "platform_cookie.h"
#include "plat_defs.h"
#include "diag_emmc_test.h"
#include "diag_storage_lib.h"
#include "diag_emmc_util.h"


/*******************************************************************************
 *                          Function Declaration
 *******************************************************************************
 */

/* Local functions */

int read_extcsd(int , __u8 *);
int write_extcsd_value(int , __u8 , __u8 );
int emmc_send_status(int , __u32 *);
int emmc_pslc_fully_enable (int opt);
int show_emmc_info (void);
static uchar get_ecsd_byte_val(uchar *, int);
static int   save_pattern(char *, unsigned int *, int);
static int   load_pattern(char *, unsigned int *, int *);
static int   mmc_fill_cmd_flag(uint32_t, uint32_t, uint *);
static int   smart_num_conversion(char *, long long int *);
static int   parse_and_issue_cmd(int, char *, char *);
static int   get_emmc_cid_info(emmc_info_t *);


/*******************************************************************************
 *                                Globals
 *******************************************************************************
 */
struct mmc_ioc_cmd mmc_local_cmd = {0};
__u32  buffer_ptr[MMC_IOC_MAX_BYTES];


/*******************************************************************************
 *
 * Function    : read_extcsd
 * Description : Function to read extcsd register 
 * Inputs      : fd - file descriptor
 *               ext_csd - register
 * Outputs     : ret - return value from ioctl
 *
 *******************************************************************************
 */
int read_extcsd(int fd, __u8 *ext_csd)
{
    int ret = 0;
    struct mmc_ioc_cmd idata;
    memset(&idata, 0, sizeof(idata));
    memset(ext_csd, 0, sizeof(__u8) * 512);
    idata.write_flag = 0;
    idata.opcode = MMC_SEND_EXT_CSD;
    idata.arg = 0;
    idata.flags = MMC_RSP_SPI_R1 | MMC_RSP_R1 | MMC_CMD_ADTC;
    idata.blksz = EXT_CSD_ARRAY_SIZE;
    idata.blocks = 1;
    mmc_ioc_cmd_set_data(idata, ext_csd);

    ret = ioctl(fd, MMC_IOC_CMD, &idata);
    if (ret) {
        perror("ioctl");
    }

    return (ret);
}

/*******************************************************************************
 *
 * Function    : write_extcsd_value
 * Description : Function to write extcsd register 
 * Inputs      : fd - file descriptor
 *               index - register number
 *               value -value want to write
 * Outputs     : ret - return value from ioctl
 *
 *******************************************************************************
 */
int write_extcsd_value (int fd, __u8 index, __u8 value)
{

    int ret = 0;
    struct mmc_ioc_cmd idata;

    memset(&idata, 0, sizeof(idata));
    idata.write_flag = 1;
    idata.opcode = MMC_SWITCH;
    idata.arg = (MMC_SWITCH_MODE_WRITE_BYTE << 24) |
        (index << 16) |
        (value << 8) |
        EXT_CSD_CMD_SET_NORMAL;
    idata.flags = MMC_RSP_SPI_R1B | MMC_RSP_R1B | MMC_CMD_AC;

    ret = ioctl(fd, MMC_IOC_CMD, &idata);
    if (ret){
        perror("ioctl");
    }

    return (ret);
}

/*******************************************************************************
 *
 * Function    : emmc_send_status
 * Description : Function to check emmc status
 * Inputs      : fd - file descriptor
 *               response - status response
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int emmc_send_status(int fd, __u32 *response)
{
    int ret = 0;
    struct mmc_ioc_cmd idata;

    memset(&idata, 0, sizeof(idata));
    idata.opcode = MMC_SEND_STATUS;
    idata.arg = (1 << 16);
    idata.flags = MMC_RSP_R1 | MMC_CMD_AC;

    ret = ioctl(fd, MMC_IOC_CMD, &idata);
    if (ret){
        perror("ioctl");
    }

    *response = idata.response[0];

    return ret;
}


/*******************************************************************************
 *
 * Function   : emmc_pslc_fully_enable
 * Description: Function to enable eMMC fully pSLC mode and to set
 *              GPP partition.
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int emmc_pslc_fully_enable (int opt)
{
    char *devname = EMMC_DEVNAME;
    __u8 value;
    __u8 ext_csd[512];
    int fd, ret;
    unsigned int enh_multiplier;
    char     ch = 0;
    uchar    byte_val = 0;
    __u32 response;

    /* Show emmc information first */
    show_emmc_info();

    byte_val = get_ecsd_byte_val((__u8 *)buffer_ptr, PARTITION_ATTRIBUTE);
    if ((byte_val & 0x1) == 0x1) {
        printf("\nCurrent eMMC fully pSLC mode is enabled.\n");
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

    fd = open(devname, O_RDWR);
    if (fd < 0) {
        printf("%s(%d) Failed to open device(%s): %s(%d)\n",
               __func__, __LINE__, devname, strerror(errno), errno);
        return (FAILED);
    }

    ret = read_extcsd(fd, ext_csd);
    if (ret) {
        printf("%s(%d) Failed to read ext CSD: %s(%d)\n",
               __func__, __LINE__, strerror(errno), errno);
	     close(fd);
        return (FAILED);
    }

    /* assert not PARTITION_SETTING_COMPLETED */
    if (ext_csd[EXT_CSD_PARTITION_SETTING_COMPLETED]) {
        printf("Current eMMC fully pSLC mode is enabled.\n");
        close(fd);
        return (PASSED);
    }

    /* set EXT_CSD_ERASE_GROUP_DEF bit 0 */
    ret = write_extcsd_value(fd, EXT_CSD_ERASE_GROUP_DEF, 0x1);
    if (ret) {
        printf("Could not write 0x1 to EXT_CSD[%d] in %s\n", EXT_CSD_ERASE_GROUP_DEF, devname);
        close(fd);
        return (FAILED);
    }

    /* Configure Enhanced User Data Area Start Address. Entire UDA is Enhanced */
    value = 0;
    ret = write_extcsd_value(fd, EXT_CSD_ENH_START_ADDR_3, value);
    if (ret) {
        printf("Could not write 0x%02x to EXT_CSD[%d] in %s\n", value, EXT_CSD_ENH_START_ADDR_3, devname);
        close(fd);
        return (FAILED);
    }

    value = 0;
    ret = write_extcsd_value(fd, EXT_CSD_ENH_START_ADDR_2, value);
    if (ret) {
        printf("Could not write 0x%02x to EXT_CSD[%d] in %s\n", value, EXT_CSD_ENH_START_ADDR_2, devname);
        close(fd);
        return (FAILED);
    }

    value = 0;
    ret = write_extcsd_value(fd, EXT_CSD_ENH_START_ADDR_1, value);
    if (ret) {
        printf("Could not write 0x%02x to EXT_CSD[%d] in %s\n", value, EXT_CSD_ENH_START_ADDR_1, devname);
        close(fd);
        return (FAILED);
    }

    value = 0;
    ret = write_extcsd_value(fd, EXT_CSD_ENH_START_ADDR_0, value);
    if (ret) {
        printf("Could not write 0x%02x to EXT_CSD[%d] in %s\n", value, EXT_CSD_ENH_START_ADDR_0, devname);
        close(fd);
        return (FAILED);
    }

    /* MaxEnhSizeMul[2] ext_csd[159], MaxEnhSizeMul[1] ext_csd[158], MaxEnhSizeMul[0] ext_csd[157] */
    /* Set the Enhanced User Data Area Size. Entire UDA is enhanced */
    enh_multiplier = (ext_csd[EXT_CSD_MAX_ENH_SIZE_MULT_2] << 16) |
                     (ext_csd[EXT_CSD_MAX_ENH_SIZE_MULT_1] << 8) |
                     (ext_csd[EXT_CSD_MAX_ENH_SIZE_MULT_0]);

    value = (enh_multiplier >> 16) & ENH_SIZE_MULT_MASK;
    ret = write_extcsd_value(fd, EXT_CSD_ENH_SIZE_MULT_2, value);
    if (ret) {
        printf("Could not write 0x%02x to EXT_CSD[%d] in %s\n", value, EXT_CSD_ENH_SIZE_MULT_2, devname);
        close(fd);
        return (FAILED);
    }

    value = (enh_multiplier >> 8) & ENH_SIZE_MULT_MASK;
    ret = write_extcsd_value(fd, EXT_CSD_ENH_SIZE_MULT_1, value);
    if (ret) {
        printf("Could not write 0x%02x to EXT_CSD[%d] in %s\n", value, EXT_CSD_ENH_SIZE_MULT_1, devname);
        close(fd);
        return (FAILED);
    }

    value = enh_multiplier & ENH_SIZE_MULT_MASK;
    ret = write_extcsd_value(fd, EXT_CSD_ENH_SIZE_MULT_0, value);
    if (ret) {
        printf("Could not write 0x%02x to EXT_CSD[%d] in %s\n", value, EXT_CSD_ENH_SIZE_MULT_0, devname);
        close(fd);
        return (FAILED);
    }

    /* Set Enhanced attribute for UDA */
    value = ext_csd[EXT_CSD_PARTITIONS_ATTRIBUTE];

    value |= 0x1; /* Set Enhanced for UDA */
    ret = write_extcsd_value(fd, EXT_CSD_PARTITIONS_ATTRIBUTE, value);
    if (ret) {
        printf("Could not write 0x%02x to EXT_CSD[%d] in %s\n", value, EXT_CSD_PARTITIONS_ATTRIBUTE, devname);
        close(fd);
        return (FAILED);
    }

    /*
     **********************************************************************
     * Activate below code only after everything seems fine!
     **********************************************************************
     */

    /* Set Partition Complete */
    ret = write_extcsd_value(fd, EXT_CSD_PARTITION_SETTING_COMPLETED, 0x1);
    if (ret) {
        printf("Could not write 0x1 to "
            "EXT_CSD[%d] in %s\n",
            EXT_CSD_PARTITION_SETTING_COMPLETED, devname);
        close(fd);
        return (FAILED);
    }

    /* Check if everything went fine! */
    ret = emmc_send_status(fd, &response);
    if (ret) {
        printf("Could not get response to SEND_STATUS "
            "from %s\n", devname);
        close(fd);
        return (FAILED);
    }

    if (response & R1_SWITCH_ERROR) {
	printf("Setting OTP PARTITION_SETTING_COMPLETED "
		"failed on %s\n", devname);
	    close(fd);
        return (FAILED);
    }

    printf("\nSoftware configuration process of enable eMMC pSLC mode is DONE.\n");
    printf("Please power cycle your unit to make it works!\n");

    close(fd);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : show_emmc_info
 * Description : Function to show current eMMC info 
 * Inputs      : None
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int show_emmc_info (void)
{
    ulong           numblocks = 0;
    char            *devname = EMMC_DEVNAME;
    uchar           byte_val = 0;
    char            record[1024];
    int             fd = -1;
    emmc_info_t emmc_info;
    int ix;


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

    if ((NVRAM)->diagflag & D_VERBOSE) {
        for (ix = 0; ix < EXT_CSD_ARRAY_SIZE; ix++) {
            if(ix % 16 == 0) {
                printf("%3d\t", ix);
            }
            byte_val = get_ecsd_byte_val((__u8 *)buffer_ptr, ix);
            printf("%02x ", byte_val);
		
            if(ix % 16 == 15) {
                printf("\n");
            }
        }
    }

    /* Check ECSD[156] to see if pSLC mode is enabled. */
    byte_val = get_ecsd_byte_val((__u8 *)buffer_ptr, PARTITION_ATTRIBUTE);

    /* 3. Show eMMC info */
    printf("\nCurrent eMMC(%s) info\n", devname);
    printf("Manuf Name    : %s\n", emmc_info.manf_name);
    printf("Product Name  : %s\n", emmc_info.prod_name);
    printf("Size          : %.3f GB(%lu Bytes).\n",
           (double)((numblocks * mmc_local_cmd.blksz) / ONE_GB),
           (numblocks * mmc_local_cmd.blksz));
    printf("pSLC mode     : %s.\n",
           ((byte_val & EMMC_PSLC_ENABLE) == EMMC_PSLC_ENABLE) ?
           "Enabled" : "NOT enabled");

    close(fd);

    return (PASSED);
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
    case MMC_SWITCH: 
        /* ac [31:0] See below R1b */
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
        bash_cmd = malloc(sizeof(record));
        system(strcpy(bash_cmd, record+2));

        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("parse() --> bash_cmd = %s\n", bash_cmd);
        }
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
 * Function    : get_emmc_cid_info
 * Description : Function to get current eMMC CID info 
 * Inputs      : *emmc_info - buffer to put eMMC info
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
static int get_emmc_cid_info (emmc_info_t *emmc_info)
{
    FILE  *fd = NULL;
    char  filename[EMMC_BUF_SIZE];
    char  buf[EMMC_BUF_SIZE];

    /* Get Manufacturer ID */ 
    memset(buf, 0, sizeof(buf));
    memset(filename, 0, sizeof(filename));
    sprintf(filename, EMMC_SYSFS_MANFID);
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
    } else if (strstr(buf, EMMC_KINGSTON_MANFID_STR) != NULL) {
        sprintf(emmc_info->manf_name, "Kingston(0x70)");
    } else if (strstr(buf, EMMC_SAMSUNG_MANFID_STR) != NULL) {
        sprintf(emmc_info->manf_name, "Samsung(0x15)");
    } else {
        printf("%s(%d) Unknown Manf ID(%s).\n", __func__, __LINE__, buf);
        fclose(fd);
        return (FAILED);
    }
    fclose(fd);

    /* Get eMMC product name */ 
    memset(buf, 0, sizeof(buf));
    memset(filename, 0, sizeof(filename));
    sprintf(filename, EMMC_SYSFS_NAME);
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

/*------------------------------------------------------------------
$Log: diag_emmc_util.c,v $
Revision 1.2  2019/12/11 10:10:28  lucywang
Merged Nanook to main trunk


$Endlog$
*/

