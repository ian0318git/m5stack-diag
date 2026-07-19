/* $Id: legacy_smart_cookie.h,v 1.3 2019/12/30 06:03:45 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/legacy_smart_cookie.h,v $
 *------------------------------------------------------------------
 *
 * legacy_smart_cookie.h - Legacy Smart Cookie header files
 *
 * Copyright (c) 2015-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

/* Command message for diagnostic */
#define ECHO_REQUEST                            0x51
#define ECHO_REPLY                              0x52
#define ECHO_INCOMPLETE_REQ                     0x53 /* Not sent to SCC */

#define ACT2_VERSION_NUMBER_START               0x17

#define IMC_POLLING_TIMEOUT                     900
/* By experiments, it only need 1 polling to detect the SCC ID,
    * to be safe, take the margin of 10 polling */
#define SCC_ID_CMD_POLLING_TIMEOUT              10

#define WAITING_FOR_REPLY_MSG                   2000

#define MAX_N2G_QCK_MSG_SIZE                    4

#define DF_FLOW_CONTROL_WINDOW_SIZE             8

#define N2G_QCK_MAX_POLLING_TIMEOUT_CTR         200
#define WAIT_FOR_N2G_SCC_PROCESS_CMD            100
#define WAIT_FOR_GET_NEXT_CMD                   1000
#define WAIT_FOR_CMD_COMPLETE                   (10 * 1.5)
#define WAIT_FOR_RANDOM_NUMBER_SIGNING          300000
#define WAIT_FOR_EEPROM_LOCKING                 50000
#define WAIT_DEFAULT_TIME                       20000

#define SCC_IMC_IF                              0x0001
#define SCC_DUART_IF                            0x0002
#define SCC_FPGA_IF                             0x0004
#define SCC_I2C_IF                              0x0008
#define SCC_ILL_VERSION                         -1

#define MAX_DATA_SIZE                           256
#define HEADER_SIZE                             4 /* includes the CRC too */
#define MAX_MESSAGE_SIZE                        MAX_DATA_SIZE + HEADER_SIZE

/* One byte command (shorter command message takes less time to transmit) */
#define GET_SCC_ID                              0x61
#define GET_COOKIE_DATA_64B                     0x62 /* for 64 bytes cookie data */
#define GET_COOKIE_DATA_128B                    0x63 /* for 128 bytes cookie data */
#define FLOW_CONTROL_GET_NEXT                   0x64

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

#define REQUEST_SCC_ID                          0x01
#define REQUEST_COOKIE_DATA                     0x02
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

#define RESPONSE_SCC_ID                         0x11
#define RESPONSE_COOKIE_DATA                    0x12
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

#define EEPROM_PRGM_KEYS_N_SIGNATURE            0x21
#define EEPROM_PRGM_COOKIE_N_SIGNATURE          0x22
#define SMART_EEPROM_READ                       0x23
#define EEPROM_READ_RESPONSE                    0x24
#define SMART_EEPROM_WRITE                      0x25
#define COOKIE_WRITE                            0x26
#define EEPROM_PAGE_LOCK_DOWN                   0x27

#define COMMAND_ACK                             0x31
#define COMMAND_NACK_CRC_ERR                    0x32
#define COMMAND_NACK_INV_MSG_TYPE               0x33
#define COMMAND_NACK_INV_MSG_LEN                0x34
#define COMMAND_NACK_INV_EEPROM_ACCESS          0x35
#define COMMAND_NACK_MSG_LEN_EXCEED             0x36
#define COMMAND_NACK_INV_CHIP_TYPE              0x37
#define COMMAND_NACK_INV_PARAMETER              0x38
#define COMMAND_NACK_INV_OVERWRT_SPARE          0x39
#define COMMAND_NACK_SIGN_NOT_ALLOW             0x3A
#define COMMAND_NACK_PIN_SET_INCOMPLETE         0x3B

#define FLOW_CONTROL_SET                        0x41
#define WRITE_SPARE                             0x42
#define SET_SIGN_PIN_N_LIMIT_COUNT              0x43
#define SET_PIN                                 0x44
#define CHANGE_SIGN_LIMIT_COUNT                 0x45
#define GET_SIGN_LIMIT_COUNT                    0x46
#define REFRESH_SIGN_LIMIT_COUNT                0x47
#define CHANGE_PIN                              0x48
#define RETURN_SIGN_LIMIT_COUNT                 0x49
#define RETURN_SIGN_LIMIT_COUNT_SIZE              13

/*
 * Following define are from IOS to IMC
 */
#define IMC_SPI_CMD_START                       0x01
#define IMC_SPI_CMD_CONTD                       0x02
#define IMC_SPI_CMD_END                         0x03
#define IMC_SPI_CMD_DREQ                        0x05 /* Req Data from IMC */
#define IMC_SPI_CMD_SCC_RESET                   0x07 /* Reset the SCC */
#define IMC_SPI_CMD_STATUS                      0x09 /* Req IMC status */
#define IMC_SPI_RESYNC                          0xDDDD
#define IMC_SPI_CMD_HW_REV                      0x0E /* Get IMC Hardware Revision */
#define IMC_SPI_CMD_SW_REV                      0x0C /* Get IMC SOftware Revision */

#define N2G_SCC_TX_MSG_SIZE                     MAX_N2G_QCK_MSG_SIZE

#define SC_COOKIE_ADDR_MB    0x03
#define SC_COOKIE_ADDR_LB    0x80
#define COOKIE_DATA_SIZE     128
#define READ_WRITE_MAX_SIZE  127


typedef struct dev_if_info_ {
    uint16_t interface;
    uint8_t parm1;      /* i2c_bus_no */
    uint8_t parm2;      /* i2c_dev */
    uint8_t parm3;      /* i2c_mux */
    uint8_t parm4;      /* i2c_ctrl */
    uint16_t cookie_size;
    uint16_t offset;
} dev_if_info_t;

/*
 * PA management typedef
 */
typedef struct pas_management_t_ {

    uint pa_bay;                        /* Port Adaptor bay */

    /*
     * PA ID eeprom clock register
     */
    ulong clk_reg_width;
    ulong clk_mask;
    volatile void *clk_reg;

    /*
     * PA ID eeprom select register
     */
    ulong select_reg_width;
    ulong select_mask;
    ulong enable_select_bits;
    ulong disable_select_bits;
    volatile void *select_reg;

    /*
     * PA ID eeprom datain register
     */
    ulong datain_reg_width;
    ulong datain_mask;
    volatile void *datain_reg;

    /*
     * PA ID eeprom dataout register
     */
    ulong dataout_reg_width;
    ulong dataout_mask;
    volatile void *dataout_reg;

} pas_management_t;

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

typedef enum scc_return_status_ {
     SCC_OK,
     SCC_NAK,
     SCC_INVALID_SLOT_ID,
     SCC_CARD_NOT_PRESENT,
     SCC_CARD_POWER_DOWN,
     SCC_TIMEOUT,
} scc_return_status_t;

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

extern int smart_cookie_read(sc_context *);
extern int cookie_is_act2(sc_context *);
extern void i2c_act2_reset(sc_context *);
extern int i2c_act2_write_bytes(sc_context *, char *, int);
extern int i2c_act2_read_bytes(sc_context *, char *);
extern int act2_is_simple_mode(void *);
extern int smart_cookie_read_x(sc_context *con_p, ushort size);
extern int smart_cookie_write_x(sc_context *con_p, uchar *cookie_buf, ushort size);
extern int smart_cookie_write(sc_context *con, uchar *cookie_buf);



/*---------------------------------------------------------------
$Log: legacy_smart_cookie.h,v $
Revision 1.3  2019/12/30 06:03:45  kehuang2
CSCvs55860: Support Alter Quack cookie

Revision 1.2  2019/10/17 02:16:24  kehuang2
Collapse Tabei-L into main trunk

Revision 1.1.2.2  2019/07/31 03:45:08  kehuang2
Merge platform_tam_cookie into platform_cookie

Revision 1.1.2.1  2018/11/02 02:39:03  kodko
Support cookie read for NIM and PIM modules.

$Endlog$
*/
