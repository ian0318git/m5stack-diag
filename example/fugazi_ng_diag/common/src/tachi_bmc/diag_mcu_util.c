/* $Id: diag_mcu_util.c,v 1.4 2016/09/02 06:41:53 jimmyya Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_mcu_util.c,v $
 *------------------------------------------------------------------
 *
 * diag_mcu_util.c - MCU Utility Functions
 * 
 * June 2015, Times Huang
 *
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ioctl.h> 
#include "common.h"
#include "types.h"
#include "error.h"
#include "menu.h"
#include "defs.h"
#include "common_utils.h"
#include "diag_mcu_util.h"
#include "nvmonvars.h"
#include "diag_fpga_util.h"
#include "diag_fpga_lib.h"
#include "diag_mcu_lib.h"
#include "assert.h"
#include "proto.h"
#include "extern.h"
#include "uio_utils.h"
#include "linux_api.h"


int diag_mcu_util(void);
int diag_mcu_show_ver(void);

static int diag_mcu_fw_upgrade(void);
static int diag_mcu_reg_alter(void);
static int diag_mcu_reg_display(void);
static int diag_mcu_reg_dump(void);

static volatile uint32_t intr_cnt ;

extern const unsigned char eprom[];
extern const unsigned long eprom_start;
extern const unsigned long eprom_length;
extern const unsigned char overlord_env_mcu_revision[];

#ifdef TEMPORARY_PASS
extern const unsigned char vm_eprom[];
extern const unsigned long vm_eprom_start;
extern const unsigned long vm_eprom_length;
extern const unsigned char overlord_vmon_mcu_revision[];
#endif

#define MAX_REGION 2
#define FW_BIN_SZ 0x10000
#define BIN_FW "pseq.bin"
#define PWR_SEQ_FW_DEFAULT_PATH "/tmp/pseq.s19"
#define SREC_DATA_SZ 32
#define TO_EEPROM
static uint16_t start_region[MAX_REGION]   = {0x8000, 0x1000};
static uint16_t end_region[MAX_REGION]     = {0xFC00, 0x1400};

extern int srec2bin_main(int, char *argv[], int*, int*);

/* Sub Menu used for MCU utility.
 */
static submenu_xtable_t mcu_util_submenu_table[] = {
    {"Firmware Upgrade", (type_t(*)())diag_mcu_fw_upgrade,   0,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Register Alter Utility", (type_t(*)())diag_mcu_reg_alter,   0,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
	{"Register Display Utility", (type_t(*)())diag_mcu_reg_display,   0,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
	{"Register Dump Utility", (type_t(*)())diag_mcu_reg_dump,   0,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
};

#define MCU_UTIL_SUBMENU_TABLE_SIZE (sizeof(mcu_util_submenu_table) / \
				       sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t mcu_util_primary_items[MCU_UTIL_SUBMENU_TABLE_SIZE +
				       MAX_BASE_ITEMS];
static mitem_t mcu_util_secondary_items[MCU_UTIL_SUBMENU_TABLE_SIZE +
					 MAX_BASE_ITEMS];

menuinfo_t mcu_util_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    mcu_util_primary_items,
};
menuinfo_t *mcu_util_submenup = &mcu_util_subtest_menu;

int diag_mcu_util (void)
{
    set_nios_mode(NIOS_DISABLE_MODE);
    build_primary_submenu(mcu_util_submenu_table,
			              MCU_UTIL_SUBMENU_TABLE_SIZE,
                          "MCU", &mcu_util_submenup);
    build_secondary_submenu(mcu_util_submenu_table,
                            MCU_UTIL_SUBMENU_TABLE_SIZE,
                            mcu_util_secondary_items);    
                            
    menu(mcu_util_submenup, mcu_util_secondary_items, '\0');
    set_nios_mode(NIOS_DIAG_MODE);
    return (PASSED);
}

int diag_mcu_show_ver (void)
{
    uint16_t d16;

    set_nios_mode(NIOS_DISABLE_MODE);
    diag_mcu_reg_read(MCU_VER_REG_OFFSET, &d16);

    printf("MCU Version: v%d.%d\n", (d16 >> 8) & 0xFF, d16 & 0xFF);

    set_nios_mode(NIOS_DIAG_MODE);
    return (PASSED);
}

static uint16_t
cnt_record(char *fw_path, uint16_t *line)
{
    FILE *fp;
    uint16_t ret = PASSED;
    char buf[SREC_DATA_SZ* 2 + 32]; /*data + header, etc... */

    if ((fp = fopen(fw_path, "rb")) == NULL) {
        printf("unable to open srec file %s.\n", fw_path);
        perror("");
        return(FAILED);
    }
    *line = 0;
    while ( fgets(buf, sizeof(buf), fp) ) {
        *line = *line+1;
        if (*line == 0xFFFF) {
            printf("too many records in srec file\n");
            ret = FAILED;
            break;
        }
    }
    fclose(fp);
    if (*line <= 1) {
        printf("Too few records in srec file\n");
        ret = FAILED;
    }
    printf("%d records in srec file\n", *line);
    return(ret);
}

static int
_pwr_seq_eeprom_update ()
{
    int rc = PASSED;
    uint16_t data;
    /* set cmd to UPDATE mode */
    data = PWR_SEQ_CMD_UPDATE;
    rc = diag_mcu_reg_write(PWR_SEQ_FW_CMD_REG, data);
    if (rc != PASSED) {
        printf("Set cmd to update mode failed (offset = %X, data = %x)\n", \
               PWR_SEQ_FW_CMD_REG, data);
        goto fun_ret;
    }

fun_ret:

    return rc;
}

/*******************************************************************************
 *
 * Function   : _write_pwr_seq_fw
 * Description: Write a section of power sequencer firmware through I2C
 * Inputs     : fw_buf: pointer to  a buffer to hold the binary data
 *              to_address: the start address (target addr) of the binary data
 *              byte_len: length of the binary data
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int
_write_pwr_seq_fw(unsigned char *fw_buf, uint16_t to_address,
                  uint16_t end_address)
{
    uint16_t wr_data;
    int id, idx;
    int rc = FAILED;

#ifdef TO_EEPROM
    /* set write address to corresponding addr of  the begining block */
    rc = diag_mcu_reg_write(PWR_SEQ_FW_CMD_REG, to_address);
    if (rc != PASSED) {
        printf("Reseting CMD register failed (offset = %X, data = %x)\n",\
        PWR_SEQ_FW_CMD_REG, to_address);
        goto fun_ret;
    }
#endif
    fflush(NULL);

    for (idx = 0, id = to_address; id < end_address; id += 2,
             idx+=2 /*2 bytes per wr */)
    {   
        wr_data = ((fw_buf[idx] & 0xFF) << 8) +
            (fw_buf[idx+1] & 0xFF);
        if ((idx%ONE_K)==0) {
            print_spining_wheel(-1);
        }

        /* begins write data */
#ifdef TO_EEPROM
        rc = diag_mcu_reg_write(PWR_SEQ_FW_DATA_REG, wr_data);
        if (rc != PASSED) {
            printf("FW data register write failed (offset = %X, data = %x)\n",\
                    PWR_SEQ_FW_DATA_REG, wr_data);
            goto fun_ret;
        }
#endif
    }
    rc = PASSED;

fun_ret:
    return rc;
}

static int
_pwr_seq_eeprom_rd (uint16_t to_addr, uint16_t *rd_data)
{
    int rc = FAILED;

    /* set write address to corresponding addr of  the begining block */
    *rd_data = 0;
    rc = diag_mcu_reg_write(PWR_SEQ_FW_CMD_REG, to_addr);
    if (rc != PASSED) {
        printf("Reseting CMD register failed (offset = %X\n",\
               PWR_SEQ_FW_CMD_REG);
        goto fun_ret;
    }

    rc = diag_mcu_reg_read(PWR_SEQ_FW_DATA_REG, rd_data);
    if (rc != PASSED) {
        printf("FW data register write failed (offset = %X", \
               PWR_SEQ_FW_DATA_REG);
        goto fun_ret;
    }
    rc = PASSED;

fun_ret:

    return rc;
}



static int
verify_pwr_seq_fw (void)
{
    uint16_t wr_data, to_addr, end_addr;
    unsigned char *hex;
    int id, region;
    int rc = FAILED;
    int mismatch = 0;

    if ((hex = (unsigned char *)malloc(FW_BIN_SZ)) == NULL) {
        printf("system is out of memory.\n");
        return -1;
    }
    if ((readfile(BIN_FW, hex, FW_BIN_SZ) < 0)) {
        printf("unable to read binary firmware file.\n");
    }

    set_nios_mode(NIOS_DISABLE_MODE);

    /* set write address to corresponding addr of  the begining block */
    for (region = 0; region < MAX_REGION; region++) {
        to_addr = start_region[region];
        end_addr = end_region[region];

        rc = diag_mcu_reg_write(PWR_SEQ_FW_CMD_REG, to_addr);
        if (rc != PASSED) {
            printf("Reseting CMD register failed (offset = %X, data = %x)\n", \
                   PWR_SEQ_FW_CMD_REG, to_addr);
            goto fun_ret;
        }
        printf("verifying [0x%04x-0x%04x]...\n",  to_addr, end_addr);

        /* firwmare has 64K byte */
        for (mismatch = 0, id = to_addr; id < end_addr; id += 2) {
            rc = diag_mcu_reg_read(PWR_SEQ_FW_DATA_REG, &wr_data);
            if (rc != PASSED) {
                printf("FW data register write failed (offset = %X, data = %x)\n", \
                       PWR_SEQ_FW_DATA_REG, wr_data);
                goto fun_ret;
            }
            if ((id%256)==0) {
                print_spining_wheel(-1);
            }
            if  ((((wr_data >> 8) & 0xFF) != hex[id])
                 && (((wr_data & 0xFF) != hex[id+1]))) {
                rc = FAILED;
                printf("data mismatch @0x%06x ", id);
                printf("expect [0x%02x%02x] : found [0x%02x%02x]\n", hex[id],
                       hex[id+1], (wr_data >> 8) & 0xFF, (wr_data & 0xFF));

                if (mismatch++ > 10) {
                    printf("too many mistmatches...aborting comparison\n");
                    break;
                }
            }
        }
    }
 fun_ret:
    if (hex)
        free(hex);
    return rc;
}


static int diag_mcu_fw_upgrade (void)
{
    time_t start_t, stop_t;
    uint16_t data;
    uint32_t rc = FAILED;
    int byte_len, idx, ret_val;
    char fw_path[256] = {0};
    char *tk = NULL;
    char *argv[10];
    char cmd[80];
    unsigned char *fw_buf = NULL; //[SREC_DATA_SZ] = {0}; /* 32 bytes data per line (block)*/
    FILE *fp_bin = NULL;
    uint16_t to_address, blk_size, size_opt;
    int *addr_list, *byte_cnt_list;
    uint16_t line = 0;

    byte_cnt_list = addr_list = NULL;
   
    /* disable NIOS so it won't interfere I2C read result */
    set_nios_mode(NIOS_DISABLE_MODE);

    /* query fw path and filename */
    printf("Please enter srec file [%s] (Enter q to quit): ", \
           PWR_SEQ_FW_DEFAULT_PATH);
    fflush(stdout);
    get_line(fw_path, sizeof(fw_path));
    if (strcmp(fw_path, "q") == 0)
    {   /* quit */
        return PASSED;
    }
    if (strlen(fw_path) <= 0)
    {   
        /* use default path */
        memcpy(fw_path, PWR_SEQ_FW_DEFAULT_PATH, \
                        strlen(PWR_SEQ_FW_DEFAULT_PATH));
    }
    if (cnt_record(fw_path, &line)==FAILED)
        return(FAILED);
    addr_list = (int *)malloc(sizeof(int *) * (line+1));
    byte_cnt_list = (int *)malloc(sizeof(int *) * (line+1));

    //usage: srec2bin output_file -k file_size -v 0 -d 0 -s pseq.s19
    //-v : verbose
    //-d : padd with 0
    //-s : input file..has to be the last option
    size_opt = FW_BIN_SZ / ONE_K;
    sprintf(cmd, "./srec2bin %s -K %d -v 0 -d 0 -s %s", BIN_FW, size_opt, fw_path);
    tk = strtok(cmd, " ");
    idx = 0;
    argv[idx++] = tk;
    while (tk != NULL) {
        tk = strtok(NULL, " ");
        argv[idx++] = tk;
        if (tk)
            printf("%s ", tk);
    }
    idx--;
    printf("\nsrec file is used to create %s [%d bytes].\n", BIN_FW,
           size_opt * ONE_K);
    srec2bin_main(idx, argv, addr_list, byte_cnt_list);
    if ((fp_bin = fopen(BIN_FW, "rb")) == NULL) {
        printf("unable to open binary file for programming.\n");
        return (FAILED);
    }
    /* check size of binary pseq_fw.bin (should be 64K) */
    fseek(fp_bin, 0, SEEK_END);
    byte_len = ftell(fp_bin);
    if (byte_len != FW_BIN_SZ) {
        printf("%s has incorrect size of %d bytes. expect %d bytes.\n", BIN_FW,
               byte_len, FW_BIN_SZ);
        goto fun_ret;
    }
    printf("File transfering from srec to bin is done\n");
    fw_buf = (unsigned char *)malloc(byte_len);
    memset(fw_buf, 0, sizeof(byte_len));

    /* set cmd to UPDATE mode */
    data = PWR_SEQ_CMD_UPDATE;
    rc = diag_mcu_reg_write(PWR_SEQ_FW_CMD_REG, data);
    if (rc != PASSED) {
            printf("Set cmd to update mode failed (offset = %X, data = %x)\n",\
            PWR_SEQ_FW_CMD_REG, data);
        goto fun_ret;
    }
    printf("Using %s file to update.\n", BIN_FW);
    printf("Regions to be updated: [0x%4x-%4x] and [0x%4x-%4x]\n\n",
           start_region[0], end_region[0]-1, start_region[1], end_region[1]-1);
    time(&start_t);
    _pwr_seq_eeprom_update();

    for (idx=0;idx<line;idx++) {
        if ( ((addr_list[idx] >= start_region[0]) && (addr_list[idx] < end_region[0])) ||
             ((addr_list[idx] >= start_region[1]) && (addr_list[idx] < end_region[1])) ) {

            to_address = addr_list[idx];
            blk_size = byte_cnt_list[idx];

            /* seek and read data from file and store to fw_buf */
            fseek(fp_bin, to_address, SEEK_SET);
            byte_len = fread(fw_buf, 1, blk_size,  fp_bin);
            if (byte_len != blk_size) {
                printf("at record %d @0x%4x, found %d bytes; expect %d bytes\n",
                       idx, blk_size, byte_len, to_address);
                goto fun_ret;
            }
            /* handle special case when a record has odd number of data */
            if (blk_size%2) {
                /*read 2 bytes from eeprom; store only upper nibble to the
                 end of buffer to make the size of data even. */
                _pwr_seq_eeprom_rd(to_address+blk_size, &data);
                fw_buf[blk_size] = (data >> 8) & 0xFF;
                /*round up to even number */
                blk_size++;
            }
            _write_pwr_seq_fw(fw_buf, to_address, to_address+blk_size);
        }
    }

    time(&stop_t);
    printf("Update done. Took %.0f secs.\n", difftime(stop_t, start_t));
    ret_val = verify_pwr_seq_fw();
    time(&stop_t);
    printf("total time: %.0f secs.\n", difftime(stop_t, start_t));

    if (ret_val < 0) {
        printf("\n\n****WARNING****\n\n");
        printf("\nFirmware verification failed. Please re-program firmware.\n");
        printf("Do not reboot system until firmware upgrade is successful.\n");
        return FAILED;
    } else {
        printf("MCU updating done.\n");
        printf("To load the updated firmware, please reboot system.\n");
    }

    rc = PASSED;


fun_ret:
    if (fp_bin!=NULL)
        fclose(fp_bin);
    if (fw_buf!=NULL)
        free(fw_buf);
    if (addr_list)
        free(addr_list);
    if (byte_cnt_list)
        free(byte_cnt_list);

    return rc;

}

static int diag_mcu_reg_alter (void)
{
    uint32_t offset, d16;
    int32_t result;
            
    offset = gethex_answer("enter offset ", 0x26, 0, 0x49);
    d16 = gethex_answer("enter 16-bit data (ie, 0x1234)", 0x1234, 0, 0xFFFF);

    result = diag_mcu_reg_write(offset, d16);
    if (result == FAILED) {
        printf("%s: error reading reg\n", __FUNCTION__);
    }
   
    return (result);
}

static int diag_mcu_reg_display (void)
{
    int result = FAILED;
    uint32_t offset;
    uint16_t d16;

    offset = gethex_answer("enter offset ", 0x26, 0, 0xFF);
    result = diag_mcu_reg_read(offset, &d16);
    if (result == FAILED) {
        printf("%s: error reading reg\n", __FUNCTION__);
    }

    printf("@%#x = 0x%04x\n", offset, d16);

    return (result);
}

static int diag_mcu_reg_dump (void)
{
    uint32_t offset;
    uint16_t d16;

    printf("MCU Register Display:\n");

    for (offset = 0; offset < 0xFF; offset++) {
        if (offset % 16 == 0) {
            printf("\n");
            printf("0x%02x: ", offset);
        }
        diag_mcu_reg_read(offset, &d16);

        printf("%04x ", d16);
    }
    printf("\n");

    return (PASSED);
}



/*---------------------------------------------------------------
$Log: diag_mcu_util.c,v $
Revision 1.4  2016/09/02 06:41:53  jimmyya
remove useless menu item

Revision 1.3  2016/08/09 02:44:20  jimmyya
add MCU software updating utility

Revision 1.2  2016/04/20 11:25:28  benchen2
add tachi fru portion

Revision 1.1.2.6  2015/10/15 06:23:21  benchen2
add set_nios_mode

Revision 1.1.2.5  2015/09/25 02:18:24  tirawan
Correct MCU reg read/write (not to byte swap) and display MCU version

Revision 1.1.2.4  2015/09/24 05:31:58  tirawan
Add MCU Register Dump Utility

Revision 1.1.2.3  2015/09/18 02:40:41  tirawan
No support on MCU firmware upgrade for now

Revision 1.1.2.2  2015/07/31 07:39:31  hondwang
mcu r,w, upgrade

Revision 1.1.2.1  2015/06/11 02:01:09  tirawan
Add files for Tachi BMC project


$Endlog$
*/

