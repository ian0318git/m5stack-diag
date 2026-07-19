 /* $Id: platform_cookie.h,v 1.3 2020/04/20 02:28:24 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/platform_cookie.h,v $
 *------------------------------------------------------------------
 *
 * platform_cookie.h - Platform specific cookie defines from Xformers.
 *
 * Sept. 2008, Shih-Nan Huang
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef _PLATFORM_COOKIE_H_ 
#define _PLATFORM_COOKIE_H_ 

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

#define SUPPORT_DISCRETE_AIKIDO_ACT2     1

/* BIOS EEPROM content length */
#define EEPROM_PRODUCT_NAME_LEN          18
#define EEPROM_PROCESSOR_TYPE_LEN        6
#define EEPROM_NAME_LEN                  32
#define EEPROM_DESR_LEN                  33
#define EEPROM_PCBSN_LEN                 11
#define EEPROM_MACADDR_LEN               6
#define EEPROM_VID_LEN                   3
#define EEPROM_MACADDR_BLK_LEN           4
#define EEPROM_ASCII_MACADDR_LEN         12
#define EEPROM_MACADDR_AND_COMMON_LEN    14
#define EEPROM_UUID_MACADDR_LEN          14
#define EEPROM_PART_NUMBER_LEN           12
#define UUID_MAGIC_NO_6                  6
#define UUID_MAGIC_NO_7                  7
#define UUID_MAGIC_NO_8                  8
#define UUID_MAGIC_NO                    0x29
#define UUID_COMMON_POS_4                4
#define UUID_COMMON_POS_9                9
#define COMMON_ACSII_VAL                 0x2e
#define MAC_ADDRESS_BASE_OFFSET          0xFD
#define PART_NUMBER_BASE_OFFSET          0x3B
#define SN_BASE_OFFSET                   0x12
#define PCB_SN_BASE_OFFSET               0xDC
#define MAC_ADDR_BLK_N0X_SIZE_OFFSET     0x91
#define MAC_ADDR_BLK_SIZE_OFFSET         0x93
#define MFG_TEST_DATA_OFFSET             0xB1
#define PROCESSOR_TYPE_N0X_OFFSET        0x72
#define PROCESSOR_TYPE_OFFSET            0x74
#define CLEI_CODE_OFFSET                 0x53 
#define VID_OFFSET                       0x111
#define NAME_OFFSET                      0x11B
#define DESR_OFFSET                      0x142
#define SN_LEN                           11
#define PID_BASE_OFFSET                  0x0
#define UUID_BASE_OFFSET                 0x1D
#define UUID_LEN                         16

enum {
    BIOS_EEPROM_PID = 1,
    BIOS_EEPROM_PART_NUM,
    BIOS_EEPROM_CLEI_CODE,
    BIOS_EEPROM_CTRL_TYPE,
    BIOS_EEPROM_MAC_BLK_SIZE,
    BIOS_EEPROM_MFG_TEST_DATA,
    BIOS_EEPROM_PCB_SN,
    BIOS_EEPROM_CHASSIS_MAC,
    BIOS_EEPROM_VID,
    BIOS_EEPROM_NAME,
    BIOS_EEPROM_DESR,
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
extern int alter_pim_cookie(void);
extern int alter_nim_cookie(void);
extern int smartchip(int);
extern int platform_get_pid(char *);
extern int get_cookie_pid (int, int, unsigned char *, char *);
extern int alter_bios_eeprom(int);
extern int get_ngio_mac_addr(int, int, uchar *);

#endif /* _PLATFORM_COOKIE_H_ */

/*-------------------------------------------------
 * $Log: platform_cookie.h,v $
 * Revision 1.3  2020/04/20 02:28:24  lucywang
 *
 * 1. Fixed unplug/plug NIM module dynamically issue and added NIM cookie
 * 2. Added to support NIM Prince
 * 3. (CSCvn43011) add retry workaround for Deverton issue
 * 4. add debug message and set default value to seneors
 * 5. Reverted Register value of temp/press snsr after test
 * 6. Bumped up version to 1.0.2
 *
 * Revision 1.2  2019/12/11 10:10:33  lucywang
 * Merged Nanook to main trunk
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
