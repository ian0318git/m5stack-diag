/* $Id: smart_cookie.h,v 1.6 2016/06/06 18:30:30 huanngo Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/smart_cookie.h,v $
 *------------------------------------------------------------------
 * smart_cookie.h  - definitions and structures for quack.
 *                   Port from IOS.
 *
 * Copyright (c) 2007-2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 * Author: Huan Ngo
 *------------------------------------------------------------------
 */

#ifndef __SMART_COOKIE_H_
#define __SMART_COOKIE_H_

#include "cli_cmd.h"

#define SMART_ON_HW_SPI_BUS	0x1	/* HW generates clk and data */

#define REQUEST_SCC_ID          		0x01
#define REQUEST_COOKIE_DATA              	0x02
#define REQUEST_SN_SCMFG_PUBKEY_SIGN_1          0x03
#define REQUEST_VL_DEV_PUBKEY_SIGN_2            0x04
#define REQUEST_SIGN_MESSAGE                    0x05
#define REQUEST_SIGN_MESSAGE_32B                0x06   
#define REQUEST_CNTMFG_PUBKEY_SIGN_3            0x07
#define REQUEST_CK_SIGN_4                       0x08
#define REQUEST_SER_NUM                         0x09
#define REQUEST_PUBKEY_N_CERT                   0x0A   
#define REQUEST_SIGNATURE                       0x0B
#define REQUEST_RANDOM_NUMBER                   0x0C
#define REQUEST_SIGN_MSG_DIGEST                 0x0D
#define REQUEST_LOT_INFO                        0x0E

#define RESPONSE_SCC_ID                        	0x11
#define RESPONSE_COOKIE_DATA                   	0x12
#define RESPONSE_SN_SCMFG_PUBKEY_SIGN_1         0x13
#define RESPONSE_VL_DEV_PUBKEY_SIGN_2           0x14
#define RESPONSE_SIGN_MESSAGE                   0x15
#define RESPONSE_SIGN_MESSAGE_32B               0x16    
#define RESPONSE_CNTMFG_PUBKEY_SIGN_3           0x17
#define RESPONSE_CK_SIGN_4                      0x18
#define RESPONSE_SER_NUM                        0x19
#define RESPONSE_PUBKEY_N_CERT                  0x1A
#define RESPONSE_SIGNATURE                      0x1B
#define RESPONSE_RANDOM_NUMBER                  0x1C
#define RESPONSE_SIGN_MSG_DIGEST                0x1D
#define RESPONSE_LOT_INFO                       0x1E

#define EEPROM_PRGM_KEYS_N_SIGNATURE		0x21
#define EEPROM_PRGM_COOKIE_N_SIGNATURE		0x22
#define SMART_EEPROM_READ			0x23
#define EEPROM_READ_RESPONSE			0x24
#define SMART_EEPROM_WRITE			0x25
#define COOKIE_WRITE                            0x26
#define EEPROM_PAGE_LOCK_DOWN                   0x27
     
#define COMMAND_ACK				0x31
#define COMMAND_NACK_CRC_ERR			0x32
#define COMMAND_NACK_INV_MSG_TYPE		0x33
#define COMMAND_NACK_INV_MSG_LEN	        0x34
#define COMMAND_NACK_INV_EEPROM_ACCESS		0x35
#define COMMAND_NACK_MSG_LEN_EXCEED             0x36
#define COMMAND_NACK_INV_CHIP_TYPE              0x37
#define COMMAND_NACK_INV_PARAMETER              0x38
#define COMMAND_NACK_INV_OVERWRT_SPARE          0x39
#define COMMAND_NACK_SIGN_NOT_ALLOW             0x3A
#define COMMAND_NACK_PIN_SET_INCOMPLETE         0x3B

#define FLOW_CONTROL_SET			0x41
#define WRITE_SPARE                             0x42
#define SET_SIGN_PIN_N_LIMIT_COUNT              0x43
#define SET_PIN                                 0x44
#define CHANGE_SIGN_LIMIT_COUNT                 0x45
#define GET_SIGN_LIMIT_COUNT                    0x46
#define REFRESH_SIGN_LIMIT_COUNT                0x47
#define CHANGE_PIN                              0x48
#define RETURN_SIGN_LIMIT_COUNT                 0x49
#define RETURN_SIGN_LIMIT_COUNT_SIZE              13

/* Command message for diagnostic */
#define ECHO_REQUEST				0x51
#define ECHO_REPLY				0x52
#define ECHO_INCOMPLETE_REQ                     0x53 /* Not sent to SCC */

    
/* Size of messages (including CRC) */
#define RESPONSE_SCC_ID_SIZE                       8
#define RESPONSE_COOKIE_DATA_SIZE                132
#define RESPONSE_SN_SCMFG_PUBKEY_SIGN_1_SIZE     134
#define RESPONSE_VL_DEV_PUBKEY_SIGN_2_SIZE       101
#define RESPONSE_CNTMFG_PUBKEY_SIGN_3_SIZE       102
#define RESPONSE_CK_SIGN_4_SIZE                   52
#define RESPONSE_SIGN_MESSAGE_SIZE                53
#define RESPONSE_SIGN_MESSAGE_32B_SIZE            87       
#define RESPONSE_COOKIE_DATA_SIZE_64B             68
#define RESPONSE_PUBKEY_N_CERT_SIZE              105
#define RESPONSE_SIGNATURE_SIZE                   52
#define RESPONSE_LOT_INFO_SIZE                    68
#define COMMAND_ACK_NACK_SIZE                      5

/* One byte command (shorter command message takes less time to transmit) */
#define GET_SCC_ID                              0x61
#define GET_COOKIE_DATA_64B                     0x62 /* for 64 bytes cookie data */
#define GET_COOKIE_DATA_128B                    0x63 /* for 128 bytes cookie data */
#define FLOW_CONTROL_GET_NEXT                   0x64

/* Size defines */
#define MAX_DATA_SIZE                           256
#define PUB_KEY_SIZE                             48    /* 192 bit curve */
#define PVT_KEY_SIZE                             24    /* 192 bit curve */   
#define SER_NUM_SIZE                             32
#define SIGN_SIZE                                48
#define RAND_NUM_SIZE                           128
#define RAND_NUM_SIZE_32B                        32
#define COOKIE_DATA_SIZE                        128 
#define READ_WRITE_MAX_SIZE                     127
#define CRE_MAX_SIZE                           2048
#define AUTO_TEST_MAX_SIZE                     1024
#define DIGEST_LEN                               20 
#define WDC_SIZE                                958

#define MOTHERBOARD_ID                           0
#define INVALID_SLOT_ID                         -1

#define LOT_INFO_SIZE                           64

/*
 * The command/ response structure of the messages sent to/from the smart
 * chip controller 
 */
typedef struct sm_message_ {
     uchar type;
     uchar version;
     uchar length;
     uchar data[MAX_DATA_SIZE]; 
     uchar cksum;
} sm_message_t;

#define HEADER_SIZE                    4 /* includes the CRC too */ 
#define MAX_MESSAGE_SIZE               MAX_DATA_SIZE + HEADER_SIZE
typedef struct tlv_lot_info_ {
    uchar type;
    uchar size;
    uchar data[LOT_INFO_SIZE];
} tlv_lot_info_t;
   
#define TLV_SCC_MFG_DATE                        0x01
#define TLV_SCC_PO                              0x02
#define TLV_SCC_PO_RANGE_START_CHIP_ID          0x03
#define TLV_SCC_PO_RANGE_END_CHIP_ID            0x04
#define TLV_SCC_CHIP_ID                         0x05

#define TLV_CM_DATE_START_RANGE                 0x01
#define TLV_CM_DATE_END_RANGE                   0x02
#define TLV_CM_PO_RANGE_START                   0x05
#define TLV_CM_PO_RANGE_END                     0x06
#define TLV_CM_PO_RANGE_START_CHIP_ID           0x07
#define TLV_CM_PO_RANGE_END_CHIP_ID             0x08
#define TLV_CM_81_82_LIST_START                 0x10
#define TLV_CM_EPS_KEY_SRC                      0x10
#define TLV_EXACT_MATCH		                0x81
#define TLV_RANGE_MATCH                         0x82
#define TLV_INFO_ONLY                           0x83

#define MIN_CM_TLV_COUNT                        0x02
#define EPS_CISCO_LAB                           0x00

/*
 * TLV structure returned on parsing the data
 */
typedef struct tlv_id_ {
    int tlv_type;
    int size;
    int display;
    uchar *data;
} tlv_id_t;

/*
 * Messages sent from IOS to IMC
 */
typedef enum message_to_imc_ { 
     IMC_RESET,
     IMC_SCC_RESET,
     IMC_GET_IMC_HW_VER,
     IMC_GET_IMC_SW_VER,
     IMC_RESYNC,
     IMC_POWER_ON,
     IMC_POWER_OFF
} message_to_imc;
     

typedef enum scc_return_status_ {
     SCC_OK,
     SCC_NAK,
     SCC_INVALID_SLOT_ID,
     SCC_CARD_NOT_PRESENT,
     SCC_CARD_POWER_DOWN,
     SCC_TIMEOUT,
} scc_return_status_t;

/*
 * Commands from IOS to IMC
 *
 *  FORMAT::
 *    --------------------------------------------------
 *    | IMC MSG-ID |  Control INFO  |  TX-Data         |
 *    --------------------------------------------------
 *   15          12 11             8 7               0
 */
typedef struct {
   uchar spi_msg_id        : 4;        /* bit 12 .. 15 */
   uchar msg_cntl          : 4;        /* bit 8  .. 11 */
   uchar data              : 8;        /* bit 7  .. 0  */
} spi_cmd_str_t;

union spi_cmd_u {
   ushort spi_cmd_word;
   spi_cmd_str_t spi_cmd_str;
};

/* 
 * Following define are from IOS to IMC 
 */
#define IMC_SPI_CMD_START     0x01
#define IMC_SPI_CMD_CONTD     0x02
#define IMC_SPI_CMD_END       0x03
#define IMC_SPI_CMD_DREQ      0x05 /* Req Data from IMC */
#define IMC_SPI_CMD_SCC_RESET 0x07 /* Reset the SCC */
#define IMC_SPI_CMD_STATUS    0x09 /* Req IMC status */
#define IMC_SPI_RESYNC        0xDDDD
#define IMC_SPI_CMD_HW_REV    0x0E /* Get IMC Hardware Revision */
#define IMC_SPI_CMD_SW_REV    0x0C /* Get IMC SOftware Revision */
    
#define IMC_CMD_SIZE          0x2
/* 
 * Following define are from IMC to IOS 
 */
#define IMC_SPI_DATA_VALID    0x01
#define IMC_SPI_DATA_INVALID  0xFF

/*
 * SCC Check Sum
 */
#define SCC_FLOW_CONTROL_GET_NEXT           0x64

#define DF_FLOW_CONTROL_WINDOW_SIZE            8
#define IMC_POLLING_TIMEOUT                  900
/* By experiments, it only need 1 polling to detect the SCC ID,
 * to be safe, take the margin of 10 polling */
#define SCC_ID_CMD_POLLING_TIMEOUT            10

#define HIGH 1
#define LOW  0

#define PROTECTED_BYTES 16 

#define SCC_IMC_IF      0x0001
#define SCC_DUART_IF    0x0002
#define SCC_FPGA_IF     0x0004
#define SCC_I2C_IF      0x0008

typedef struct dev_if_info_ {
    uint16_t interface;
    uint8_t parm1;      /* i2c_bus_no */
    uint8_t parm2;      /* i2c_dev */
    uint8_t parm3;      /* i2c_mux */
    uint8_t parm4;      /* i2c_ctrl */
    uint16_t cookie_size;
    uint16_t offset;
} dev_if_info_t;

typedef struct sc_context_ {
    uchar *cookie_contents;
    pas_management_t *pa;
    PFT quack_read_2bytes;
    PFT quack_write_2bytes;
    PFT quack_write_read_2words;
    PFT quack_reset;
    char *info_string;
    uchar type;
    uchar slot;
    dev_if_info_t *dev_if_p;
} sc_context;
    
/*
 * This structure is used to map the NM id to the name and
 * authentication test routine.
 */
struct nm_sc_info {
    char   *name;                /* module name */
    ushort id;                   /* the port module id */
    PFI    wic_sc_progauth;      /* the wic initializaion */
    PFI    pvdm_sc_progauth;     /* the pvdm initialization */
    PFI    daughter_sc_progauth; /* the daughtercard initialization */
};

typedef struct smartchip_submenu_t_ {
    char *x_title;        /* title of the item */
    type_t  (*x_pfunc)();    /* primary diag function */
    type_t  *x_pparam;        /* primary function parameter */
    int  x_flags;         /* menu item flags */
    type_t  (*x_xfunc)();    /* item boolean existence function */
    type_t  x_xparam;        /* existence function parameter */
    type_t  (*x_sfunc)();    /* secondary diag function (if any) */
    type_t  x_sparam;        /* secondary function parameter (not int*) */
} smartchip_submenu_t;

#define SC_COOKIE_ADDR_MB                   0x03
#define SC_COOKIE_ADDR_LB                   0x80
#define SC_CM_PUB_KEY_SIGN_ADDR_MB          0x03
#define SC_CM_PUB_KEY_SIGN_ADDR_LB          0x40
#define SC_CM_PUB_KEY_ADDR_MB               0x03
#define SC_CM_PUB_KEY_ADDR_LB               0x00
#define SC_COOKIE_SIGN_ADDR_MB              0x05
#define SC_COOKIE_SIGN_ADDR_LB              0x80
#define SC_CM_LOT_TLV_ADDR_MB               0x02
#define SC_CM_LOT_TLV_ADDR_LB               0xC0
#define SC_LOCK_DOWN_ADDR_MB                0x08
#define SC_LOCK_DOWN_ADDR_LB                0x00
#define SC_CREDENTIAL_ADDR_MB               0x18
#define SC_CREDENTIAL_ADDR_LB               0x00

#define WAITING_FOR_REPLY_MSG               2000
#define DELAY_AFTER_1_SCC_CMD               1000
#define DELAY_AFTER_2BYTES_CMD               260
#define DELAY_AFTER_1_IMC_CMD                260
#define DELAY_AFTER_SCC_1_BYTE_CMD          1400
#define DELAY_BETWEEN_READ_CYCLE             260
#define IMC_CMD_DELAY_TIME                   200
#define IMC_CS_DELAY                         100
#define BIT_BANG_INTRF_CLK_INTERVAL            2
#define IMC_CLK_TO_CS_DELAY                   50
#define DELAY_DSCC4                          500
#define QUACK_INIT_TIME			      10 /* Quack init time in ms */
#define SCC_RESET_DELAY                       1000 /* 1sec delay after scc reset */

#define SCC_ILL_VERSION                       -1
#define ILLEGAL_ID                        0xffff 
#define SCC_1_VERSION                          2 /* Quack 1 has version 2 */

/*
 * Test patterns used for IMC detection using Atmel driver.
 */
#define IMC_TST_PAT1                        0xFF 
#define IMC_TST_PAT2                        0x00
#define IMC_TST_PAT3                        0xF0
#define ATMEL_EEPROM_READ_CMD               (0x02 << 6)
#define ATMEL_START_BIT                     1   
#define ATMEL_CMD_SIZE                      2

/* For RMA Deletion */
#define DEL_REQUEST_SIZE                   64
#define DEL_APPROVAL_SIZE                 420

/* I2C */
#define MAX_IMC2_SIZE                       256
#define MAX_POLLING_TIMEOUT_CTR             500
#define I2C_TX_MSG_SIZE                       4
#define QUACK_I2C_ADDR                     0xE2
#define I2C_RD                                1
#define I2C_WR                                0
#define WAIT_FOR_I2C_TRANSACTION    (150 * 1.5)
#define MAX_N2G_QCK_MSG_SIZE                  4
#define N2G_SCC_RX_MSG_SIZE                 MAX_N2G_QCK_MSG_SIZE
#define WAIT_FOR_CMD_COMPLETE        (10 * 1.5)
#define WAIT_FOR_RANDOM_NUMBER_SIGNING   300000
#define WAIT_FOR_EEPROM_LOCKING           50000
#define WAIT_DEFAULT_TIME                 20000
#define N2G_SCC_TX_MSG_SIZE                 MAX_N2G_QCK_MSG_SIZE
#define WAIT_FOR_N2G_SCC_PROCESS_CMD        100
#define N2G_QCK_MAX_POLLING_TIMEOUT_CTR     200
#define WAIT_FOR_N2G_SCC_PROCESS_CMD        100
#define WAIT_FOR_GET_NEXT_CMD              1000

/* Sudi Data Address */
#define KEY_HEADER_ADDR      0x900
#define KEY_CONTENT_ADDR     0x908
#define CERT_HEADER_ADDR     0x1108
#define CERT_CONTENT_ADDR1   0x1110  /* from 0x1110 - 0x17ff */
#define CERT_CONTENT_ADDR2   0x1C00  /* from 0x1c00 - 0x1fff */
#define CERT_CONTENT_ADDR3   0x2200  /* from 0x2200 - 0x330f */
#define CERT_DATA1_SIZE      0x6F0
#define CERT_DATA2_SIZE      0x400

/* Sudi data Size */
#define KEY_HEADER_SIZE   8
#define KEY_CONTENT_SIZE  2048    /* 2k */
#define CERT_HEADER_SIZE  8
#define CERT_CONTENT_SIZE 7168    /* 7k */

/* Sudi Header define */
#define TYPE_KEY          1
#define TYPE_CERT         2
#define MAGIC_NUM         0xDEADBEEF

#define MAX_DATA_PER_LINE 100

/* define Sudi header structure */
typedef struct sudi_header_t_ {
    ushort type;
    ushort size;
    uint   magic;
} sudi_header_t;

/* 
 * Extern definitions 
 */
extern uchar response_msg[MAX_MESSAGE_SIZE];
extern uchar err_msg[MAX_MESSAGE_SIZE];
extern menuinfo_t smart_cookie_subtest_menu;
extern boolean pcb_for_sudi;

extern int show_smart_cookie_submenu(void);
extern boolean is_smart_eeprom(sc_context *con);
extern uchar calculate_cksum(uchar *p, int count);
extern boolean is_smart_eeprom(sc_context *con);
extern int cookie_read_write_eeprom(sc_context *con, int size,PFI eeprom_erase);
extern int smart_cookie_read_write_eeprom(sc_context *con, cli_cookie_cmd *cli_cmd);
extern type_t smart_cookie_authenticate(sc_context *con);
extern int smart_cookie_read(sc_context *con);
extern int smart_cookie_read_write_aim_eeprom(sc_context *con);
extern ushort get_smart_cookie_controller_type(sc_context *con);
extern void scc_reset_nm_imc(sc_context *con);
extern int smartchip(int op);
extern int is_act2(void);
extern int sm_write_cookie_cmd(sc_context *con, uchar *cookie_buf);
extern int plat_init_smart_eeprom_context(sc_context *con_p, uchar type, 
					   uchar slot, uchar *cookie_p);
extern int send_command_to_smart_cookie(sc_context *con, char type,
			 uchar *data, uint data_length);
extern type_t smart_cookie_echo_test(sc_context *con);
extern void smart_cookie_delay(long usec);
extern void clean_smart_eeprom_context (sc_context *con_p);
extern int resync_mb_wic_smart_chip(sc_context *con_p);

extern ushort vwic_read_bytes(sc_context *con_p);
extern ushort scc_nm_aim_read_2bytes(sc_context *con_p);
extern void resync_pvdm_smart_chip(sc_context *con_p);
extern void soprano_resync_pvdm_smart_chip(sc_context *con_p);
extern ushort soprano_pvdm_scc_read_2bytes(sc_context *con_p);
extern int soprano_pvdm_scc_write_read_2words(sc_context *con_p, ushort *cmd,
					       ushort *reply);
extern int soprano_pvdm_smartchip_auth(int slot, int pvdm,
				       uchar progauth);
extern int venom_daughter_smartchip_auth(ushort em_id, int slot, 
					int em_slot, uchar progauth);
extern int copland_wic_smartchip_auth(int slot, int wic_port,
                                      uchar progauth);
extern int soprano_vic_smartchip_auth(int slot, int wic_port,
                                      uchar progauth);
extern int guido_vic_smartchip_auth(int slot, int wic_port,
                                    uchar progauth);
extern int progauth_error_msg(void);
extern void progauth_help(void);
extern type_t smart_cookie_authenticate_retest(sc_context *con);
extern int smart_cookie_auth_force(sc_context *con);
extern int progauth(int argc,char *argv[]);
extern boolean is_atmel_eeprom(sc_context *con_p);
extern void imc_format_test_cmd(uchar *command, uchar test_pattern);
extern void scc_nm_aim_write_read_2words(sc_context *con_p, ushort *cmd,
					ushort *reply);
extern int scc_vwic_write_read_2words(sc_context *con_p, ushort *cmd,
				      ushort *reply);
extern struct menuinfo *diagflagp;
extern uchar sc_response_msg[MAX_MESSAGE_SIZE];
extern char sc_err_msg[MAX_MESSAGE_SIZE];
extern int fecpm_wic_smartchip_auth_retest(int slot, int wic_port,
					   uchar prog_auth);
extern int combo_wic_smartchip_auth_retest(int slot, int wic_port,
					   uchar prog_auth);
extern int venom_daughter_smartchip_auth_retest(ushort em_id, int slot, 
                                                int em_slot, uchar progauth);
extern int copland_wic_smartchip_auth_retest(int slot, int wic_port,
                                             uchar progauth);
extern int soprano_vic_smartchip_auth_retest(int slot, int wic_port,
                                             uchar progauth);
extern int guido_vic_smartchip_auth_retest(int slot, int wic_port,
                                           uchar progauth);
extern int alt_nm_em_cookie (int, int, boolean, cli_cookie_cmd *);
extern int authen_nm_em_cookie (int slot, int em_slot, boolean submenu_flag);

extern int i2c_read_bytes(sc_context *, char *, int);
extern int i2c_write_bytes(sc_context *, char *, int);
extern int i2c_quack_read_bytes(sc_context *, char *);
extern int i2c_quack_write_bytes(sc_context *, char *, int tx_size);
extern void i2c_quack_reset(sc_context *con);
extern type_t smart_cookie_program_dig_sign_mainmenu(sc_context *);
extern void print_sm_cookie_field_by_field (sc_context *, uchar *, uchar );
extern type_t smartchip_authenticate_retest(uchar, uchar);
extern int smartchip_authenticate(uchar, uchar);
extern ushort i2c_spi_read_2_bytes(sc_context *con_p);
extern void i2c_spi_reset_nm_imc(sc_context *con_p);
extern void i2c_spi_write_read_2_words(sc_context *con_p, ushort *cmd, ushort *reply);
extern type_t reset_imc(sc_context *con);
extern int check_quack_version(sc_context *con);

extern void i2c_spi_send_2_bytes(sc_context *con_p, ushort word);
extern void scc_nm_aim_send_2bytes(sc_context *con_p, ushort *word);
extern int vwic_send_bytes(sc_context *con_p, ushort word);
extern void soprano_pvdm_scc_send_2bytes (sc_context *con_p, ushort cmd);
extern int query_user_for_device(char *choice);
extern int get_pid(uchar *cookie_contents, char *pid);
#endif /* __SMART_COOKIE_H_ */

/******** History ******** 
$Log: smart_cookie.h,v $
Revision 1.6  2016/06/06 18:30:30  huanngo
Add code to support programming SUDI/WDC with Chassis S/N

Revision 1.5  2013/11/26 08:40:33  hroni
fix compiler warning

Revision 1.4  2013/03/11 03:33:15  alpeng
supporting CLI for NGIO-DC

Revision 1.3  2013/03/08 19:05:08  mcharon
add function to read pid

Revision 1.2  2012/03/28 00:38:12  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:04  ptong
Initial archive of ng_diag module


$Endlog$
*/
