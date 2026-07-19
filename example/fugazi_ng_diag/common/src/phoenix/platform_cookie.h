/* $Id: platform_cookie.h,v 1.2 2021/04/15 00:52:27 achiu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/phoenix/platform_cookie.h,v $
 *------------------------------------------------------------------
 *
 * platform_cookie.h - Platform specific cookie defines from Xformers.
 *
 * Copyright (c) 2013-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef _PLATFORM_COOKIE_H_ 
#define _PLATFORM_COOKIE_H_ 

#define HEX_VAL_OF_F_CHAR                0x66
#define CHECK_EEPROM_STATUS              "eeprog -qxf /dev/i2c-0 0x54 -16 -t 10 -r 0x0:0x30"
#define GET_HEX_EEPROM_CONTENT           "eeprog -qxf /dev/i2c-0 0x54 -16 -t 10 -r 0x%x:0x%x"
#define SHOW_DMIDECODE_STRING            "dmidecode -q -t 11"
#define INIT_EEPROM                      "eeprog -q -f -16 -i init.bin -w 0x0 -t 10 /dev/i2c-0 0x54"
#define GET_UUID_STRING                  "eeprog -qxf /dev/i2c-0 0x54 -16 -t 10 -r 0x1d:0x2c" 
#define GET_SN_STRING                    "eeprog -qf /dev/i2c-0 0x54 -16 -t 10 -r 0x12:0x1c" 
#define WRITE_DIAG_FILE                  "eeprog -q -f -16 -i diag.bin -w 0x%x -t 10 /dev/i2c-0 0x54"
#define WRITE_CLEAR_FILE                 "eeprog -q -f -16 -i clear.bin -w 0x%x -t 10 /dev/i2c-0 0x54"
#define WRITE_FILE_NAME                  "diag.bin"
#define RM_WRITE_FILE_NAME               "rm diag.bin"
#define INIT_FILE_NAME                   "init.bin"
#define RM_INIT_FILE                     "rm init.bin"
#define HEX_FILE_NAME                    "hex.bin"
#define RM_HEX_FILE                      "rm hex.bin"
#define CLEAR_FILE_NAME                  "clear.bin"
#define RM_CLEAR_FILE_NAME               "rm clear.bin"
#define ALTER_OFFSET_FILE                "alter.bin"
#define RM_ALTER_OFFSET_FILE             "rm alter.bin"
#define ERASE_EEPROM_CMD                 "eeprog -q -f -16 -i erase.bin -w 0x00 -t 10 /dev/i2c-0 0x54"
#define CREATE_ERASE_FILE                "dd if=/dev/zero of=erase.bin bs=1 count=8192"
#define RM_ERASE_FILE                    "rm erase.bin"

#define ASCII_END_OF_STRING              0xa

#define PLATFORM_BUFF_SIZE               259
#define CONTROL_TYPE_LEN                 20
#define PRODUCT_NAME_LEN                 256
#define PRODUCT_SERIAL_LEN               20
#define MAC_ADDRESS_LEN                  14
#define VID_LEN                          20
#define MAC_ADDR_BLK_LEN                 4
#define PROCESSOR_TYPE_LEN               4
#define MFG_TEST_DATA_HEADER_LEN         25
#define MFG_TEST_DATA_LEN                23
#define PART_NUMBER_LEN                  12
#define CLEI_CODE_HEADER_LEN             11
#define CLEI_CODE_LEN                    14
#define VERID_LEN                        4
#define MAC_BLK_LEN                      4
#define QUACK_RETRY                      8
#define ACT2_RESET_UNRESET_DELAY        (500)
#define ACT2_UNRESET_DELAY              (5000)

#define MFG_TEST_DATA_TYPE               0xC4
#define MAC_ADDRESS_BLOCK_SIZE_TYPE      0x43
#define PROCESSOR_TYPE                   0x40
#define CLEI_CODE_TYPE                   0xC6
#define VID_TYPE                         0x89
#define PART_NUMBER_TYPE                 0xE2

#define MAX_COMMAND_LENGTH               2048
#define EEPORG_DATA_START                7
#define WAIT_EEPROG                      5000
/* EEPROM is 64Kbits size */
#define EEPROM_SIZE                      (64 * (1024 / 8))
#define PHOENIX_INIT_EEPROM_SIZE           0x170

/* BIOS EEPROM content length */
#define SN_STRING_LEN                    12
#define UUID_STRING_LEN                  50
#define UUID_EEPROG_STRING_LEN           60
#define UUID_START_POS                   8
#define EEPROM_PRODUCT_NAME_LEN          18
#define EEPROM_PROCESSOR_HEADER_LEN      16
#define EEPROM_PROCESSOR_TYPE_LEN        6
#define EEPROM_NAME_HEADER_LEN           6 
#define EEPROM_NAME_LEN                  32
#define EEPROM_DESR_HEADER_LEN           6
#define EEPROM_DESR_LEN                  33
#define EEPROM_PCBSN_LEN                 11
#define EEPROM_MACADDR_HEADER_LEN        24
#define EEPROM_MACADDR_LEN               6
#define EEPROM_VID_HEADER_LEN            5
#define EEPROM_VID_LEN                   3
#define EEPROM_NUM_STRINGS               1
#define EEPROM_MACADDR_BLK_LEN           4
#define EEPROM_ASCII_MACADDR_LEN         12
#define EEPROM_MACADDR_C_HEADER_LEN      21 
#define EEPROM_MACADDR_AND_COMMON_LEN    14
#define EEPROM_UUID_MACADDR_LEN          14
#define EEPROM_PART_NUMBER_LEN           12
#define EEPROM_PART_HEADER_LEN           13
#define UUID_MAGIC_NO_6                  6
#define UUID_MAGIC_NO_7                  7
#define UUID_MAGIC_NO_8                  8
#define UUID_MAGIC_NO                    0x29
#define UUID_COMMON_POS_4                4
#define UUID_COMMON_POS_9                9
#define COMMON_ACSII_VAL                 0x2e
#define NUM_OF_STRING_OFFSET             0x2D
#define MAC_ADDRESS_HEADER_BASE_OFFSET   0xEE
#define MAC_ADDRESS_BASE_OFFSET          0x103
#define PART_NUMBER_HEADER_OFFSET        0x2E
#define PART_NUMBER_BASE_OFFSET          0x3B
#define SN_BASE_OFFSET                   0x12
#define PCB_SN_BASE_HEADER_OFFSET        0xCE
#define PCB_SN_BASE_OFFSET               0xE1
#define MAC_ADDR_BLK_SIZE_HEADER_OFFSET  0x7C
#define MAC_ADDR_BLK_N0X_SIZE_OFFSET     0x94
#define MAC_ADDR_BLK_SIZE_OFFSET         0x96
#define MFG_TEST_DATA_HEADER_OFFSET      0x9C
#define MFG_TEST_DATA_OFFSET             0xB5
#define PROCESSOR_TYPE_HEADER_OFFSET     0x64
#define PROCESSOR_TYPE_N0X_OFFSET        0x74
#define PROCESSOR_TYPE_OFFSET            0x76
#define CLEI_CODE_HEADER_OFFSET          0x49
#define CLEI_CODE_OFFSET                 0x54 
#define VID_HEADER_OFFSET                0x113
#define VID_OFFSET                       0x118
#define NAME_HEADER_OFFSET               0x11D
#define NAME_OFFSET                      0x123
#define DESR_HEADER_OFFSET               0x145
#define DESR_OFFSET                      0x14B
#define SN_HEADER_LEN                    19 
#define SN_LEN                           11
#define PID_BASE_OFFSET                  0x0
#define UUID_BASE_OFFSET                 0x1D
#define UUID_LEN                         16
#define MAX_STRING_LEN                   8192
#define PHOENIX_EEPROM_INIT_SIZE           0x180

enum {
    BIOS_EEPROM_NAME = 1,
    BIOS_EEPROM_DESR,
    BIOS_EEPROM_NUM_OF_STRING,
    BIOS_EEPROM_EXIT,
    BIOS_EEPROM_CLEAR0,
};

enum {
    DISCRETE_ACT2 = 0,
};

enum {
    READ_BIOS_EEPROM = 0,
    WRITE_BIOS_EEPROM,
    INIT_BIOS_EEPROM,
};

extern boolean pcb_for_sudi;

extern unsigned short get_cookie_id(int, int, uchar*, uint16_t *,
                                    char *);
extern int get_pcb_serial(uchar *, char *);
extern int get_tlv_serial (uchar *, char *, uchar);
extern ushort get_mb_id(void);
extern int alter_mb_cookie(void);
extern int alter_nim_cookie(long);
extern int smartchip(int);
extern int platform_get_pid(char *);
extern int get_cookie_pid (int, int, unsigned char *, char *);
extern int alter_bios_eeprom(int);
extern int get_ngio_mac_addr(int, int, uchar *);
extern void get_mb_pid(char *);
#endif /* _PLATFORM_COOKIE_H_ */

