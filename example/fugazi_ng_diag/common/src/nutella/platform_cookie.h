/* $Id: platform_cookie.h,v 1.4 2019/07/11 12:31:32 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nutella/platform_cookie.h,v $
 *------------------------------------------------------------------
 *
 * platform_cookie.h - Platform specific cookie defines from Xformers.
 *
 * Sept. 2008, Shih-Nan Huang
 *
 * Copyright (c) 2013-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef _PLATFORM_COOKIE_H_ 
#define _PLATFORM_COOKIE_H_ 

#define DISCRETE_I2C_ACT2                0
#define AIKIDO_ACT2                      1 
#define AIKIDO_I2C_ACT2                  2

#define HEX_VAL_OF_F_CHAR                0x66 
#define CHECK_EEPROM_STATUS              "eeprog -qxf /dev/i2c-0 0x54 -16 -t 10 -r 0x0:0x30"
#define GET_NO_STRING                    "eeprog -qxf /dev/i2c-0 0x54 -16 -t 10 -r 0x0:0x1"
#define INIT_EEPROM                      "eeprog -q -f -16 -i init.bin -w 0x0 -t 10 /dev/i2c-0 0x54"
#define GET_READABLE_STRING              "eeprog -qf /dev/i2c-0 0x54 -16 -t 10 -r 0x%x:0x%x"
#define GET_HEX_EEPROM_CONTENT           "eeprog -qxf /dev/i2c-0 0x54 -16 -t 10 -r 0x%x:0x%x"
#define WRITE_DIAG_FILE                  "eeprog -q -f -16 -i diag.bin -w 0x%x -t 10 /dev/i2c-0 0x54"
#define WRITE_CLEAR_FILE                 "eeprog -q -f -16 -i clear.bin -w 0x%x -t 10 /dev/i2c-0 0x54"
#define WRITE_FILE_NAME                  "diag.bin"
#define INIT_FILE_NAME                   "init.bin"
#define CLEAR_FILE_NAME                  "clear.bin"
#define RM_CLEAR_FILE_NAME               "rm clear.bin"
#define RM_INIT_FILE                     "rm init.bin"
#define RM_WRITE_FILE_NAME               "rm diag.bin"
#define ERASE_EEPROM_CMD                 "eeprog -q -f -16 -i erase.bin -w 0x00 -t 10 /dev/i2c-0 0x54"
#define CREATE_ERASE_FILE                "awk 'BEGIN { while (1) printf(\"%c\", 255) }' | dd of=erase.bin bs=1 count=8192"
#define RM_ERASE_FILE                    "rm erase.bin"
#define END_OF_STRING                    0xa
#define SECOND_FIELD                     30
#define NO_OF_STRING_HEX_VAL_BYTE0       9
#define NO_OF_STRING_HEX_VAL_BYTE1       10
#define READ_STRING_START_ADDR           0x0
#define READ_STRING_END_ADDR             0x180

#define QUACK_RETRY                      8
#define PLATFORM_BUFF_SIZE               259
#define CONTROL_TYPE_LEN                 20
#define PRODUCT_NAME_LEN                 256
#define PRODUCT_SERIAL_LEN               20
#define MAC_ADDRESS_LEN                  14
#define VID_LEN                          20
#define MAC_ADDR_BLK_LEN                 4
#define PROCESSOR_TYPE_LEN               4
#define MFG_TEST_DATA_LEN                23
#define PART_NUMBER_LEN                  12
#define CLEI_CODE_LEN                    14
#define VERID_LEN                        4
#define MAC_BLK_LEN                      4
#define HW_VERSION_LEN                   4
#define VSN_LEN                          16
#define PCA_LEN                          12
#define TITLE_LEN                        5
#define HW_DOT_LEN                       1
#define HW_ZERO_LEN                      1

#define LAW_HEX_VAL_OF_ALPHABET          0x40
#define HIGH_HEX_VAL_OF_ALPHABET         0x5B
#define UPPER_TO_LOWER_CASE              0x20
#define MFG_TEST_DATA_TYPE               0xC4
#define MAC_ADDRESS_BLOCK_SIZE_TYPE      0x43
#define PROCESSOR_TYPE                   0x40
#define CLEI_CODE_TYPE                   0xC6
#define VID_TYPE                         0x89
#define PART_NUMBER_TYPE                 0xE2
#define HW_VERSION_TYPE                  0x41
#define VSN_TYPE                         0xC9
#define PCA_TYPE                         0xC1

#define PROCESSOR_TYPE_OFFSET            0x74

#define MAX_COMMAND_LENGTH               2048
#define EEPORG_DATA_START                7
#define WAIT_EEPROG                      100
/* EEPROM is 64Kbits size */
#define EEPROM_SIZE                      (64 * (1024 / 8))
#define NUTELLA_EEPROM_INIT_SIZE         0x180

#define SUPPORT_DISCRETE_AIKIDO_ACT2     1


/* EEPROM String offset */
#define EEPROM_NUM_OF_STRING_OFFSET      0x0
#define EEPROM_SM_OFFSET                 0x1
#define EEPROM_SP_OFFSET                 0x20
#define EEPROM_SV_OFFSET                 0x40
#define EEPROM_SS_OFFSET                 0x50
#define EEPROM_SK_OFFSET                 0x70
#define EEPROM_SF_OFFSET                 0x90
#define EEPROM_BM_OFFSET                 0xA0
#define EEPROM_BP_OFFSET                 0xC0
#define EEPROM_BV_OFFSET                 0xE0
#define EEPROM_BS_OFFSET                 0xF0
#define EEPROM_CM_OFFSET                 0x110
#define EEPROM_CV_OFFSET                 0x130
#define EEPROM_CS_OFFSET                 0x140
#define EEPROM_CA_OFFSET                 0x150
#define EEPROM_CSK_OFFSET                0x160

/* EEPROM String length */
#define EEPROM_NUM_STRINGS_LEN           1
#define EEPROM_SM_LEN                    31
#define EEPROM_SP_LEN                    32
#define EEPROM_SV_LEN                    16
#define EEPROM_SS_LEN                    32
#define EEPROM_SK_LEN                    32
#define EEPROM_SF_LEN                    16
#define EEPROM_BM_LEN                    32
#define EEPROM_BP_LEN                    32
#define EEPROM_BV_LEN                    16
#define EEPROM_BS_LEN                    32
#define EEPROM_CM_LEN                    32
#define EEPROM_CV_LEN                    16
#define EEPROM_CS_LEN                    16 
#define EEPROM_CA_LEN                    16
#define EEPROM_CSK_LEN                   32
#define MAX_STRING_LEN                   8192

enum {
    DMI_EEPROM_NUM_OF_STRING = 1,
    DMI_EEPROM_SM,
    DMI_EEPROM_SP,
    DMI_EEPROM_SV,
    DMI_EEPROM_SS,
    DMI_EEPROM_SK,
    DMI_EEPROM_SF,
    DMI_EEPROM_BM,
    DMI_EEPROM_BP,
    DMI_EEPROM_BV,
    DMI_EEPROM_BS,
    DMI_EEPROM_CM,
    DMI_EEPROM_CV,
    DMI_EEPROM_CS,
    DMI_EEPROM_CA,
    DMI_EEPROM_CSK,
    DMI_EEPROM_EXIT,
    DMI_EEPROM_ERASE,
};

enum {
    READ_DMI_EEPROM = 0,
    WRITE_DMI_EEPROM,
};

extern boolean pcb_for_sudi;

extern unsigned short get_cookie_id(int, int, uchar*, uint16_t *, char *);
extern int get_pcb_serial(uchar *, char *);
extern int get_tlv_serial (uchar *, char *, uchar);
extern ushort get_mb_id(void);
extern int alter_mb_cookie(void);
extern int smartchip(int);
extern int platform_get_pid(char *);
extern int alter_dmi_eeprom(int);

#endif /* _PLATFORM_COOKIE_H_ */

/*-------------------------------------------------
$Log: platform_cookie.h,v $
Revision 1.4  2019/07/11 12:31:32  alicehua
Collapse Nutella codes into main trunk.

$Endlog$
*/
