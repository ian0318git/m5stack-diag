/* $Id: diag_pem_lib.h,v 1.2 2016/04/20 11:25:31 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_pem_lib.h,v $
 *------------------------------------------------------------------
 *
 * diag_pem_util.h: porting from Utah, based on rp1ruve_pem_utils.c
 *
 * May 2013 - porting the code from Nightster
 *
 * Copyright (c) 2014-2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 * Original Author: antam
 */
#ifndef _DIAG_PEM_UTILS_H_
#define _DIAG_PEM_UTILS_H_

#include "diag_idprom.h"
#include "diag_idprom_util.h"
#include "types.h"
#include "cli_cmd.h"

#define PSU_POE_SKU_STR   "PWR-4430-POE" 

#define MCP_IDPROM_SIZE   256
/* PSU related device */
typedef enum {
    PEM0_EEPROM = 0,         /* PEM0 EEPROM */
    PEM0_MCNTRL,             /* PEM0 Management */
    PEM1_EEPROM,             /* PEM1 EEPROM */
    PEM1_MCNTRL,             /* PEM1 Management */
} TACHI_PEM_NUM; 

#define PSU_ONE                  (1)
#define PSU_TWO                  (2)

#define RP1RUVE_PEM_TYPE_UNKNOWN 0
#define RP1RUVE_PEM_TYPE_ACDC    1
#define RP1RUVE_PEM_TYPE_DCDC    2

#define RP1RUVE_NUM_PEM          2
#define RP1RUVE_NUM_PEM_REGS   256 /* 2 bytes per reg */ 
#define RP1RUVE_PEM_REG_BYTES  512 /* 256 * 2 +> 512 bytes */
#define RP1RUVE_PEM_REG_WORDS  256 

#define PEM_EEPROM_SIZE_OLD     256   /* 2048bits = 256 Bytes */
#define PEM_EEPROM_SIZE         512   /* 32kbit part, will have 512 bytes info in IDPROM */  

#define PEM_RD_PTR_WR_CMD       0x01  /* I2C command to set Read-Pointer */
#define PEM_DATA_PKT_SIZE       14

#define TEST_PASSED         0
#define TEST_FAILED         1

/* pem_data_read reads 16 bytes, 2 more than needed. */
/* the read buffer needs to accomodate this.  so for */
/* defining the read buffer size, use the definition */
/* below (PEM_DATA_PKT_BUF_SIZE)                     */

/*  PEM PMbus registers:

03	CLEAR_FAULTS			W	N/A								use SendByte command
3A	FAN_CONFIG_1_2			R	BYTE
3B	FAN_COMMAND_1 			R/W	WORD	Percent		N=0		(100 = 100%)		data commanded in duty cycle (%)
46	IOUT_OC_FAULT_LIMIT		R	WORD	Amps		N=-3		(114 = 14.25A)
4A	IOUT_OC_WARN_LIMIT		R	WORD	Amps		N=-3		(102 = 12.75A)
4F	OT_FAULT_LIMIT			R	WORD	°C		N=0		(105 = 105 °C)
51	OT_WARN_LIMIT			R	WORD	°C		N=0		( 95 =  95 °C)
58	VIN_UV_WARN_LIMIT		R	WORD	Volts		N=-1		(205 = 102.5V)
59	VIN_UV_FALT_LIMIT		R	WORD	Volts		N=-1		(221 = 110.5V)
5D	IIN_OC_WARN_LIMIT		R	WORD	Amps		N=-3 OR -6/7	( 98 = 12.25A)
5E	POWER_GOOD_ON			R	WORD	Volts		N=0		(120 = 12.0V) 		direct with one decimal place
5F	POWER_GOOD_OFF			R	WORD	Volts		N=0		(118 = 11.8V) 		direct with one decimal place
68	POUT_OP_FAULT_LIMIT		R	WORD	Watts		N=2		(100 = 400W)
6A	POUT_OP_WARN_LIMIT		R	WORD	Watts		N=2		(116 = 464W)
6B	PIN_OP_WARN_LIMIT		R	WORD	Watts		N=2		( 65 = 260W)
79	STATUS_WORD			R	WORD
7A	STATUS_VOUT			R	BYTE
7B	STATUS_IOUT			R	BYTE
7C	STATUS_INPUT			R	BYTE
7D	STATUS_TEMPERATURE		R	BYTE
7E	STATUS_CML			R	BYTE
7F	STATUS_OTHER			R	BYTE
80	STATUS_MFG_SPECIFIC		R	BYTE
81	STATUS_FAN_1_2			R	BYTE
88	READ_VIN			R	WORD	Volts		N=-1		(481 = 240.5V)
89	READ_IIN			R	WORD	Amps		N=-3 OR -6/7	(180 = 22.50A)
8B	READ_VOUT			R	WORD	Volts		N=0		(145 = 14.5V)		direct with one decimal place
8C	READ_IOUT			R	WORD	Amps		N=0		(385 = 38.5A)		direct with one decimal place
8D	READ_TEMPERATURE1 		R	WORD	°C		N=0		(100 = 100 °C)		inlet temperature
8E	READ_TEMPERATURE2 		R	WORD	°C		N=0		(105 = 105 °C)		output heatsink temperature
8F	READ_TEMPERATURE3 		R	WORD	°C		N=0		( 85 =  85 °C)		hot spot temperature
90	READ_FAN_SPEED_1		R	WORD	RPM		N=5		(225 = 7200 RPM)
96	READ_POUT			R	WORD	Watts		N=2 OR 0	(100 = 400W)
97	READ_PIN			R	WORD	Watts		N=2 OR 0	( 65 = 260W)
98	PMBUS_REVISION			R	BYTE
D0	READ_VSB_12V			R	WORD	Volts		N=0		(119 = 11.9V)		direct with one decimal place
EB	FIRMWARE REVISION		R	WORD								010Ch = Revision 1.12	
*/

struct rp1ruve_pem_register_t {
    unsigned int command_code;
    unsigned int transaction_type;
    unsigned int data1;
    unsigned int data2;
    char *msg;
};

#define CLEAR_FAULTS          0x03
#define FAN_CONFIG_1_2        0x3A
#define FAN_COMMAND_1         0x3B
#define IOUT_OC_FAULT_LIMIT   0x46
#define IOUT_OC_WARN_LIMIT    0x4A
#define OT_FAULT_LIMIT        0x4F
#define OT_WARN_LIMIT         0x51
#define VIN_UV_WARN_LIMIT     0x58
#define VIN_UV_FALT_LIMIT     0x59
#define IIN_OC_WARN_LIMIT     0x5D
#define POWER_GOOD_ON	      0x5E
#define POWER_GOOD_OFF        0x5F 
#define POUT_OP_FAULT_LIMIT   0x68
#define POUT_OP_WARN_LIMIT    0x6A
#define PIN_OP_WARN_LIMIT     0x6B
#define STATUS_WORD           0x79
#define STATUS_VOUT           0x7A
#define STATUS_IOUT           0x7B
#define STATUS_INPUT          0x7C
#define STATUS_TEMPERATURE    0x7D
#define STATUS_CML            0x7E
#define STATUS_OTHER          0x7F
#define STATUS_MFG_SPECIFIC   0x80
#define STATUS_FAN_1_2        0x81
#define READ_VIN              0x88
#define READ_IIN              0x89
#define READ_VOUT             0x8B
#define READ_IOUT             0x8C
#define READ_TEMPERATURE1     0x8D
#define READ_TEMPERATURE2     0x8E
#define READ_TEMPERATURE3     0x8F
#define READ_FAN_SPEED_1      0x90 
#define READ_POUT             0x96
#define READ_PIN              0x97
#define PMBUS_REVISION        0x98
#define READ_VSB_12V          0xD0
#define FIRMWARE_REVISION     0xEB

typedef enum {
    PMBUS_NO_DATA_TRANSACTION,
    PMBUS_BYTE_TRANSACTION,
    PMBUS_WORD_TRANSACTION,
    PMBUS_BLOCK_TRANSACTION,
    PMBUS_MFR_TRANSACTION
} rp1ruve_pem_command_length;

#define PEM_T1_OFFSET       0
#define PEM_T2_OFFSET       1
#define PEM_FAN_RPM        32
#define PEM_FAN_SPEED      36
#define PEM_VOUT1_V        40
#define PEM_VIN_V          50
#define PEM_VOUT1_I        51
#define PEM_VIN_I          61
#define PEM_MODEL_ID       65
#define PEM_SHUTDOWN_E     94
#define PEM_THERMAL_E      95
#define PEM_OUTPUT_I_E     96
#define PEM_INPUT_E        97
#define PEM_STATUS_RESET   98
#define PEM_CONTROL        99


int  rp1ruve_pem_write_eeprom(uchar *idprom, int size, uint32_t unused, int ps);
int  rp1ruve_pem_read_eeprom(uchar *idprom, int size, uint32_t addr, int unused, int ps);
void rp1ruve_pem_display_eeprom(uchar *idprom, int size, int ps);


int  rp1ruve_pem_read( int ps, uint32_t command_code, unsigned short int* data);
int  rp1ruve_pem_write( int ps, uint32_t command_code, unsigned short int data );
int  rp1ruve_pem_data_read_all(int);
void rp1ruve_pem_display(int ps);
extern int pem_show_cookie_x(boolean, cli_cookie_cmd *);


int pem_get_hw_ver( int ps_num, uint32_t *buf1, uint32_t *buf2, uint32_t *buf3 );
int pem_force_error_check_ver( int ps_num );
int pem_create_error( int ps_num, uint32_t force_error );
int pem_get_sw_ver( int ps_num, uchar *major, uchar *minor );
int read_pem_eeprom_core (int ps_num, int size, uint32_t addr, uchar *rdBuf);
int pem_get_type( int ps_num, uchar *type );
uchar pem_dev_addr(int);
int rp1ruve_pem_check_present(int, int *);
int rp1ruve_pem_fan_set(int, unsigned char);
int rp1ruve_pem_fan_get(int, unsigned char *);
void build_pem_menu(unsigned int);
unsigned int check_psu_present(unsigned int);
int pem_check(void);
int pem_clear(void);
unsigned int check_poe_psu_present(int, int);
int pem_rd_cookie_pid(uchar *);


#endif /*_DIAG_PEM_UTILS_H_*/

/*
 *------------------------------------------------------------------
 * $Log: diag_pem_lib.h,v $
 * Revision 1.2  2016/04/20 11:25:31  benchen2
 * add tachi fru portion
 *
 * Revision 1.1.2.3  2016/03/16 09:00:34  alpeng
 * update sku for check poe and non-poe
 *
 * Revision 1.1.2.2  2015/12/29 12:31:06  alpeng
 * support get_mb_pid for check MB sku
 *
 * Revision 1.1.2.1  2015/12/23 11:16:13  alpeng
 * support PEM(PSU) utility and its fan utils
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
