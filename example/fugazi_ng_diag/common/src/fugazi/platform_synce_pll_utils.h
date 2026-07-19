/* $Id: platform_synce_pll_utils.h,v 1.2 2021/06/02 08:22:36 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/platform_synce_pll_utils.h,v $
 *------------------------------------------------------------------
 * Filename:	platform_synce_pll_utils.h
 *
 * Description: SyncE PLL, IDT8A3xxxx utilities.
 *
 * Copyright (c) 2019 - 2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __PLATFORM_SYNCE_PLL_UTILS_H__
#define __PLATFORM_SYNCE_PLL_UTILS_H__

#define IDT8A3_EEPROM_DATA_PATH "/fugazi-diag/idt8a3_eeprom_data.bin"

#define IDT_BLOCK_HW_REVISION                   0
#define IDT_BLOCK_RESET_CTRL                    1
#define IDT_BLOCK_GENERAL_STATUS                2
#define IDT_BLOCK_STATUS                        3
#define IDT_BLOCK_GPIO_USER_CONTROL             4
#define IDT_BLOCK_STICKY_STATUS_CLEAR           5
#define IDT_BLOCK_GPIO_TOD_NOTIFICATION_CLEAR   6
#define IDT_BLOCK_ALERT_CFG                     7
#define IDT_BLOCK_SYS_DPLL_XO                   8
#define IDT_BLOCK_SYS_APLL                      9
#define IDT_BLOCK_INPUT_0                      10
#define IDT_BLOCK_REF_MON_0                    11
#define IDT_BLOCK_DPLL_0                       12
#define IDT_BLOCK_SYS_DPLL                     13
#define IDT_BLOCK_DPLL_CTRL_0                  14
#define IDT_BLOCK_SYS_DPLL_CTRL                15
#define IDT_BLOCK_DPLL_PHASE_0                 16
#define IDT_BLOCK_DPLL_FREQ_0                  17
#define IDT_BLOCK_DPLL_PHASR_PULL_IN_0         18
#define IDT_BLOCK_GPIO_CFG                     19
#define IDT_BLOCK_GPIO_0                       20
#define IDT_BLOCK_OUT_DIV_MUX                  21
#define IDT_BLOCK_OUTPUT_0                     22
#define IDT_BLOCK_SERIAL                       23
#define IDT_BLOCK_PWM_ENCODER_0                24
#define IDT_BLOCK_PWM_DECODER_0                25
#define IDT_BLOCK_PWM_USER_DATA                26
#define IDT_BLOCK_TOD_0                        27
#define IDT_BLOCK_TOD_WRITE_0                  28
#define IDT_BLOCK_TOD_READ_PRIMARY_0           29
#define IDT_BLOCK_TOD_READ_SECONDARY_0         30

#define IDT_BLOCK_OUTPUT_TDC_CFG               31
#define IDT_BLOCK_OUTPUT_TDC_0                 32
#define IDT_BLOCK_INPUT_TDC                    33

#define IDT_BLOCK_SCRATCH                      34
#define IDT_BLOCK_EEPROM                       35
#define IDT_BLOCK_OTP                          36
#define IDT_BLOCK_BYTE                         37
#define IDT_BLOCK_MAX                          38

#define IDT_EEPROM_PROGRAM                     0x1
#define IDT_EEPROM_VERIFY                      0x2

#define IDT_BLOCK_RESET_CTRL_SOFT_RESET        0x5a

#define IDT_EEPROM_PROG_SUPPORT 1

/* for IDT8A3, 1B MODE */
#define IDT_PAGE_OFFSET                     0xFC
#define IDT_1B_MODE                         0x1020


/* 2 devices, 64KB each, total 128KB.
 */
#if defined IDT_EEPROM_PROG_SUPPORT
#define IDT_EEPROM_MAX_SIZE               0x20000
extern unsigned char idt8a3_eeprom_data[IDT_EEPROM_MAX_SIZE];
#endif


typedef struct idt8a3_block_st {
    uint8_t   block_id;
    uint32_t  base;
    char      *name;
} idt8a3_block_t;


typedef struct idt8a3_reg_st {
    uint32_t  offset;
    char      *name;
} idt8a3_reg_t;

/* Used in Recovered clock test */
typedef struct idt8a3_recovered_clk_st {
    uint8_t   input_no;    /* reg. INPUT_#, idt8a3 reg INPUT_n relative to PHY no */
    long long in_freq;     /* reg, IN_FREQ, input frequency to idt8a3 from PHY in Hz */
    uint16_t  in_div;      /* reg. IN_DIV, input divider value */
    uint8_t   in_sync;     /* reg. IN_SYNC, */
    uint8_t   in_mode;     /* reg. IN_MODE, */
    uint32_t  clk_status_mask; /* bit mask to read reg. STATUS.IN#_MON_STATUS relative to PHY no */
} idt8a3_recovered_clk_t;

typedef struct _rcvd_clock_test {
        int clock_en;
        int clock_locked;
} rcvd_clock_test_t;

#define CONFIG_DATA_VER_4             4

#define FUGAZI_MAX_PHY_PORT_CLK_TEST  8
#define FUGAZI_LAST_1G_PHY_PORT       3   /* start from 0,1,2,3 */
#define FUGAZI_1ST_10G_PHY_PORT       4   /* start from 4,5,6,7 */
#define IDT8A3_CHECK_CLOCK_VALID      0
#define IDT8A3_CHECK_CLOCK_INVALID    1
#define IDT8A3_INPUT_CLK_FFO_MAX      10  /* max input clock FFO to display */

#define IDT8A3_CHECK_VALID_POLL_TIME        60  /* check valid clock polling time in sec */
#define IDT8A3_CHECK_VALID_DURATION_TIME    10  /* check valid clock duration time in sec */
#define IDT8A3_CHECK_INVALID_POLL_TIME      20  /* check invalid clock polling time in sec */
#define IDT8A3_CHECK_INVALID_DURATION_TIME  2   /* check invalid clock duration time in sec */


/* SyncE PLL frequency margin (in Hz) */
#define SYNCE_FREQUCY_500MHZ   500000000
#define SYNCE_FREQUCY_625MHZ   625000000
#define SYNCE_FREQUCY_PPM      1000000   /* Parts Per Million */
#define SYNCE_FREQUCY_100PPM   100       /* only allow +-100ppm of margin high or low */

/* SyncE PLL INPUT frequency (in Hz) */
#define SYNCE_FREQUCY_25MHZ         25000000
#define SYNCE_FREQUCY_156p25MHZ     156250000

/* SyncE PLL frequency margin status */
#define FREQ_MARGIN_NORMAL         0
#define FREQ_MARGIN_HIGH           1
#define FREQ_MARGIN_LOW            2

/* DPLL_CTRL_# register index, Fugazi only use DPLL_CTRL_0, ..., DPLL_CTRL_3 */
typedef enum {
    FUGAZI_SYNCE_DPLL_CTRL_0,   /* output frequency is SYNCE_FREQUCY_500MHZ */
    FUGAZI_SYNCE_DPLL_CTRL_1,   /* output frequency is SYNCE_FREQUCY_500MHZ */
    FUGAZI_SYNCE_DPLL_CTRL_2,   /* output frequency is SYNCE_FREQUCY_625MHZ */
    FUGAZI_SYNCE_DPLL_CTRL_3,   /* output frequency is SYNCE_FREQUCY_500MHZ */
    MAX_FUGAZI_SYNCE_DPLL_CTRL
} fugazi_synce_dpll_ctrl_t;


extern int fugazi_idt8a3_set_page( uint8_t * );
extern int fugazi_idt8a3_read( uint32_t, uint8_t *, uint32_t * );
extern int fugazi_idt8a3_write( uint32_t, uint8_t *, uint32_t * );
extern int fugazi_idt8a3_eeprom_read( uint8_t, uint32_t, uint8_t *, uint32_t );
extern int fugazi_idt8a3_eeprom_write( uint8_t, uint32_t, uint8_t *, uint32_t );
extern int fugazi_get_idt8a3_eeprom_data( uint8_t * );

extern int idt8a3_read( uint32_t, uint8_t *, uint32_t * );
extern int idt8a3_write( uint32_t, uint8_t *, uint32_t * );
extern int idt8a3_reg_dump( uint8_t, int );
extern int idt8a3_reg_info( void );
extern int idt8a3_status_dump( void );
extern int idt8a3_input_clock_check( uint32_t, int, int, int );
extern int idt8a3_input_clock_rate( int );
extern int idt8a3_set_freq_margin( uint32_t, uint32_t, uint32_t);
extern int idt8a3_show_freq( void );
extern int idt8a3_show_fw_version( void);
extern int idt8a3_sw_reset( void );
extern int idt8a3_gen_int( int enable );
extern int idt8a3_reg_test( void );
extern int idt8a3_pll_intr_test (void);
extern int idt8a3_recovered_clock_test (void);
extern int idt8a3_check_eeprom_config_status( void );
extern int idt8a3_check_dpll_lock_status( void );

extern int idt8a3_eeprom_read( uint32_t, uint32_t);
extern int idt8a3_eeprom_write( uint32_t, uint8_t );
extern int idt8a3_eeprom_prog( uint32_t mode );

#endif /* __PLATFORM_SYNCE_PLL_UTILS_H__ */


/*-------------------------------------------------
 * $Log: platform_synce_pll_utils.h,v $
 * Revision 1.2  2021/06/02 08:22:36  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.8.2  2020/08/26 02:37:52  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.6.15  2020/08/24 00:04:29  pdoong
 * Clean code for ER.
 *
 * Revision 1.1.6.14  2020/07/17 04:49:51  pdoong
 * Code clean.
 *
 * Revision 1.1.6.13  2020/02/04 23:11:31  pdoong
 * Add display ~'input clock FFO' for 'i: DPLL Status dump' in SyncE PLL Utility Menu
 *
 * Revision 1.1.6.12  2020/01/07 00:04:53  pdoong
 * Add Checking syncE system DPLL lock status at begin of PHY initialization.
 *
 * Revision 1.1.6.11  2019/10/29 18:58:59  pdoong
 * check SyncE configuration data version, configure SyncE register Input Clock and Reference Monitor only if lower than version 4.
 *
 * Revision 1.1.6.10  2019/08/20 22:02:09  pdoong
 * fixed CSCvq86209 for SyncE Recovered Clock test works under frequency margin high/low.
 *
 * Revision 1.1.6.9  2019/08/09 19:46:40  pdoong
 * Workaround for CSCvq86209: SyncE Recovered clock test failed on frequency margin high&low
 *
 * Revision 1.1.6.8  2019/06/15 00:03:44  pdoong
 * Add SyncE PLL recovered clock test for BCM82757 10G PHY
 *
 * Revision 1.1.6.7  2019/05/21 23:22:03  pdoong
 * Added SyncE recovered clock test from bcm54194 1G PHY output clock
 *
 * Revision 1.1.6.6  2019/05/10 23:31:37  pdoong
 * added display current PLL clock frequency utility
 *
 * Revision 1.1.6.5  2019/05/03 23:27:36  pdoong
 * added pll frequency margin utility
 *
 * Revision 1.1.6.4  2019/04/24 02:16:56  pdoong
 * added syncE generate interrupt & show firmware version
 *
 * Revision 1.1.6.3  2019/04/22 22:47:41  pdoong
 * Add syncE register test and EEPROM program utility
 *
 * Revision 1.1.4.3  2019/03/28 01:59:34  pdoong
 * SyncE idt8a3 register test bring-up passed
 *
 * Revision 1.1.4.2  2019/03/14 01:28:39  letsai
 * Initial check in.
 *
 *
 *
 * $Endlog$
 */
