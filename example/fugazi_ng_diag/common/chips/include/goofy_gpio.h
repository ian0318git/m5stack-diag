/* $Id: goofy_gpio.h,v 1.2 2012/03/28 00:38:09 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/include/goofy_gpio.h,v $
 *------------------------------------------------------------------
 * structures and definitions for goofy's GPIO registers
 *
 * May 2006, Bao Buu
 *
 * Copyright (c) 2008-2012 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

#ifndef GOOFY_GPIO_H
#define GOOFY_GPIO_H

#include <dev_object.h>

#define GOOFY_TOP_LVL_GPIO_INTR_PIN  20 /* Has top level intr status bit */

/*
 * GPIO In Out Config register values:
 * input, output, and open drain.
 */
typedef enum
{
    GPIO_IN = 0,
    GPIO_OUT,
    GPIO_OD_OUT,
} goofy_gpio_inout_type_enum;

/*
 * GPIO pin polarity value
 */
typedef enum
{
    GPIO_NOT_INVERTED = 0,
    GPIO_INVERTED
} goofy_gpio_polar_enum;

/*
 * GPIO latched or unlatched interrupt value
 */
typedef enum
{
    GPIO_UNLATCHED_INTR = 0,
    GPIO_LATCHED_INTR,
} goofy_gpio_latched_intr_enum;

/*
 * GPIO pin interrupt type field values
 */
typedef enum
{
    GPIO_NET_INTR,
    GPIO_MAN_INTR,
    GPIO_ERR_INTR,
    GPIO_DISABLE_INTR,
} goofy_gpio_intr_type_enum;

/*
 * This enum is the register grouping of the GPIO regsiter
 * map. Each group consist 1 to several GPIO registers which
 * are used to define a characteristic of a GPIO pin.
 */
typedef enum {
    GPIO_IO_CONF = 0,
    GPIO_POL_CNTRL,
    GPIO_DEBOUNCE_CNTRL,
    GPIO_DEBOUNCE_TIME,
    GPIO_REG_IN_VAL,
    GPIO_REG_OUT_VAL,
    GPIO_INTR_TYPE,
    GPIO_LAT_UNLAT_INTR,
    GPIO_ERR_INTR_STAT,
    GPIO_MAN_INTR_STAT,
    GPIO_NET_INTR_STAT, // 10
    GPIO_ERR_INTR_EN,
    GPIO_MAN_INTR_EN,
    GPIO_NET_INTR_EN,
    GPIO_INTR_TEST,
    SGPIO_OUTPUT,
    SGPIO_INPUT,
    SGPIO_SHIFT_CONF,
    SGPIO_DEBOUNCE_CNTRL,
    SGPIO_DEBOUNCE_TIME,
    SGPIO_FUNC_TYPE,      // 20
    SGPIO_INTR_TEST,
    SGPIO_FAN_TACH,
    SGPIO_ETH_LED_CONF,
    SGPIO_ETH_LED_EN,
    SGPIO_INTR_TYPE,
    SGPIO_ERR_INTR_EVENT,
    SGPIO_MAN_INTR_EVENT,
    SGPIO_NET_INTR_EVENT,
    SGPIO_ERR_INTR_EN,
    SGPIO_MAN_INTR_EN,    // 30
    SGPIO_NET_INTR_EN,
    SGPIO_OUT_POL_CNTRL,
    SGPIO_IN_POL_CNTRL,
} gpio_reg_group_t;

/*
 * This data structure allows the GPIO register to
 * be represented in goofy_gpio_tbl[] as register
 * groups defined in the gpio_reg_group_t above;
 */
typedef struct goofy_gpio_reginfo_
{
    uint32_t reg0_offset;
    uint32_t num_bit_per_pin;
    uint32_t num_pin_per_reg;
    uint32_t pin_mask;
    uint32_t default_val;
} goofy_gpio_reginfo_t;

/*
 * GPIO pin info. This data struct contains
 * the gpio pin characteristics.
 */
typedef struct goofy_gpio_pin_info
{
    uint8_t inout;
    uint8_t polar;
    uint8_t debounce_en;
    uint8_t intr_type;
    uint8_t latch;
    uint8_t intr_en;
} goofy_gpio_pin_info_t;

/*
 * SGPIO pin info. This data struct contains
 * the sgpio pin characteristics.
 */
typedef struct goofy_sgpio_pin_info
{
  uint8_t inout;
  uint8_t polar;
  uint8_t debounce_en;
  uint8_t func_type;
  uint8_t intr_type;
  uint8_t intr_en;
} goofy_sgpio_pin_info_t;

/*
 * Data structure to save Goofy GPIO register settings
 */
typedef struct goofy_gpio_regsave
{
    uint32_t in_out_cfg[3];
    uint32_t pol_ctrl[2];
    uint32_t debounce_ctrl[2];
    uint32_t debounce_time[6];
    uint32_t reg_output_value[2];
    uint32_t intr_type[3];
    uint32_t lat_unlat_intr[2];
    uint32_t err_intr_en[2];
    uint32_t man_intr_en[2];
    uint32_t net_intr_en[2];
    uint32_t intr_test[2];
} goofy_gpio_regsave_t;

/*
 * Data structure to save Goofy SGPIO register settings
 */
typedef struct goofy_sgpio_regsave
{
    uint32_t output[2];
    uint32_t shift_reg_cfg;
    uint32_t debounce_ctrl[2];
    uint32_t debounce_time[8];
    uint32_t function_type;
    uint32_t intr_test[2];
    uint32_t fan_tach;
    uint32_t eth_led_cfg;
    uint32_t eth_led_en;
    uint32_t intr_type[4];
    uint32_t err_intr_en[2];
    uint32_t man_intr_en[2];
    uint32_t net_intr_en[2];
    uint32_t output_pol_ctrl[2];
    uint32_t input_pol_ctrl[2];
} goofy_sgpio_regsave_t;

typedef struct goofy_gpio_
{
    uint32_t reserved0;
    volatile uint32_t gpio_in_out_cfg2;                 /* 0x0004 */
    volatile uint32_t gpio_in_out_cfg1;                 /* 0x0008 */
    volatile uint32_t gpio_in_out_cfg0;
    volatile uint32_t gpio_pol_ctrl1;                   /* 0x0010 */
    volatile uint32_t gpio_pol_ctrl0;
    volatile uint32_t gpio_debounce_ctrl1;
    volatile uint32_t gpio_debounce_ctrl0;
    volatile uint32_t gpio_debounce_time5;              /* 0x0020 */
    volatile uint32_t gpio_debounce_time4;
    volatile uint32_t gpio_debounce_time3;
    volatile uint32_t gpio_debounce_time2;
    volatile uint32_t gpio_debounce_time1;              /* 0x0030 */
    volatile uint32_t gpio_debounce_time0;
    volatile uint32_t gpio_reg_input_value1;
    volatile uint32_t gpio_reg_input_value0;
    volatile uint32_t gpio_reg_output_value1;           /* 0x0040 */
    volatile uint32_t gpio_reg_output_value0;
    uint32_t reserved2;
    volatile uint32_t gpio_intr_type2;                  /* 0x004C */
    volatile uint32_t gpio_intr_type1;                  /* 0x0050 */
    volatile uint32_t gpio_intr_type0;
    volatile uint32_t gpio_lat_unlat_intr1;
    volatile uint32_t gpio_lat_unlat_intr0;
    volatile uint32_t gpio_err_intr_status1;            /* 0x0060 */
    volatile uint32_t gpio_err_intr_status0;
    volatile uint32_t gpio_man_intr_status1;
    volatile uint32_t gpio_man_intr_status0;
    volatile uint32_t gpio_net_intr_status1;            /* 0x0070 */
    volatile uint32_t gpio_net_intr_status0;
    volatile uint32_t gpio_err_intr_en1;
    volatile uint32_t gpio_err_intr_en0;
    volatile uint32_t gpio_man_intr_en1;               /* 0x0080 */
    volatile uint32_t gpio_man_intr_en0;
    volatile uint32_t gpio_net_intr_en1;
    volatile uint32_t gpio_net_intr_en0;
    volatile uint32_t gpio_intr_test1;                  /* 0x0090 */
    volatile uint32_t gpio_intr_test0;
    uint32_t reserved3[2];
    volatile uint32_t sgpio_output1;                    /* 0x00A0 */
    volatile uint32_t sgpio_output0;
    volatile uint32_t sgpio_input1;
    volatile uint32_t sgpio_input0;
    volatile uint32_t sgpio_shift_reg_cfg;              /* 0x00B0 */
    uint32_t reserved4;
    volatile uint32_t sgpio_debounce_ctrl1;
    volatile uint32_t sgpio_debounce_ctrl0;
    volatile uint32_t sgpio_debounce_time7;             /* 0x00C0 */
    volatile uint32_t sgpio_debounce_time6;
    volatile uint32_t sgpio_debounce_time5;
    volatile uint32_t sgpio_debounce_time4;
    volatile uint32_t sgpio_debounce_time3;             /* 0x00D0 */
    volatile uint32_t sgpio_debounce_time2;
    volatile uint32_t sgpio_debounce_time1;
    volatile uint32_t sgpio_debounce_time0;
    volatile uint32_t sgpio_function_type;              /* 0x00E0 */
    uint32_t reserved45;
    volatile uint32_t sgpio_intr_test1;
    volatile uint32_t sgpio_intr_test0;
    volatile uint32_t sgpio_fan_tach;                   /* 0x00F0 */
    uint32_t reserved5;
    volatile uint32_t sgpio_eth_led_cfg;
    volatile uint32_t sgpio_eth_led_en;
    volatile uint32_t sgpio_intr_type3;                 /* 0x0100 */
    volatile uint32_t sgpio_intr_type2;
    volatile uint32_t sgpio_intr_type1;
    volatile uint32_t sgpio_intr_type0;
    volatile uint32_t sgpio_err_intr_event1;            /* 0x0110 */
    volatile uint32_t sgpio_err_intr_event0;
    volatile uint32_t sgpio_man_intr_event1;
    volatile uint32_t sgpio_man_intr_event0;
    volatile uint32_t sgpio_net_intr_event1;            /* 0x0120 */
    volatile uint32_t sgpio_net_intr_event0;
    volatile uint32_t sgpio_err_intr_en1;
    volatile uint32_t sgpio_err_intr_en0;
    volatile uint32_t sgpio_man_intr_en1;              /* 0x0130 */
    volatile uint32_t sgpio_man_intr_en0;
    volatile uint32_t sgpio_net_intr_en1;
    volatile uint32_t sgpio_net_intr_en0;
    volatile uint32_t sgpio_output_pol_ctrl1;           /* 0x0140 */
    volatile uint32_t sgpio_output_pol_ctrl0;
    volatile uint32_t sgpio_input_pol_ctrl1;
    volatile uint32_t sgpio_input_pol_ctrl0;            /* 0x014C */
} goofy_gpio_t;   /* struct goofy_gpio_ */

/*
 * The enum assigns sequence number to the 48 GPIO pins
 */
enum {
    GOOFY_GPIO_PIN0 = 0,
    GOOFY_GPIO_PIN1,
    GOOFY_GPIO_PIN2,
    GOOFY_GPIO_PIN3,
    GOOFY_GPIO_PIN4,
    GOOFY_GPIO_PIN5,
    GOOFY_GPIO_PIN6,
    GOOFY_GPIO_PIN7,
    GOOFY_GPIO_PIN8,
    GOOFY_GPIO_PIN9,
    GOOFY_GPIO_PIN10,
    GOOFY_GPIO_PIN11,
    GOOFY_GPIO_PIN12,
    GOOFY_GPIO_PIN13,
    GOOFY_GPIO_PIN14,
    GOOFY_GPIO_PIN15,
    GOOFY_GPIO_PIN16,
    GOOFY_GPIO_PIN17,
    GOOFY_GPIO_PIN18,
    GOOFY_GPIO_PIN19,
    GOOFY_GPIO_PIN20,
    GOOFY_GPIO_PIN21,
    GOOFY_GPIO_PIN22,
    GOOFY_GPIO_PIN23,
    GOOFY_GPIO_PIN24,
    GOOFY_GPIO_PIN25,
    GOOFY_GPIO_PIN26,
    GOOFY_GPIO_PIN27,
    GOOFY_GPIO_PIN28,
    GOOFY_GPIO_PIN29,
    GOOFY_GPIO_PIN30,
    GOOFY_GPIO_PIN31,
    GOOFY_GPIO_PIN32,
    GOOFY_GPIO_PIN33,
    GOOFY_GPIO_PIN34,
    GOOFY_GPIO_PIN35,
    GOOFY_GPIO_PIN36,
    GOOFY_GPIO_PIN37,
    GOOFY_GPIO_PIN38,
    GOOFY_GPIO_PIN39,
    GOOFY_GPIO_PIN40,
    GOOFY_GPIO_PIN41,
    GOOFY_GPIO_PIN42,
    GOOFY_GPIO_PIN43,
    GOOFY_GPIO_PIN44,
    GOOFY_GPIO_PIN45,
    GOOFY_GPIO_PIN46,
    GOOFY_GPIO_PIN47,
};

/*
 * The enum mapps the external 8 bit shift register pin number to
 * the 64 Goofy SGPIO pin number
 */
enum {
    GOOFY_SGPIO_A0 = 0,
    GOOFY_SGPIO_A1,
    GOOFY_SGPIO_A2,
    GOOFY_SGPIO_A3,
    GOOFY_SGPIO_A4,
    GOOFY_SGPIO_A5,
    GOOFY_SGPIO_A6,
    GOOFY_SGPIO_A7,
    GOOFY_SGPIO_B0, //8
    GOOFY_SGPIO_B1,
    GOOFY_SGPIO_B2,
    GOOFY_SGPIO_B3,
    GOOFY_SGPIO_B4,
    GOOFY_SGPIO_B5,
    GOOFY_SGPIO_B6,
    GOOFY_SGPIO_B7,
    GOOFY_SGPIO_C0, //16
    GOOFY_SGPIO_C1,
    GOOFY_SGPIO_C2,
    GOOFY_SGPIO_C3,
    GOOFY_SGPIO_C4,
    GOOFY_SGPIO_C5,
    GOOFY_SGPIO_C6,
    GOOFY_SGPIO_C7,
    GOOFY_SGPIO_D0, //24
    GOOFY_SGPIO_D1,
    GOOFY_SGPIO_D2,
    GOOFY_SGPIO_D3,
    GOOFY_SGPIO_D4,
    GOOFY_SGPIO_D5,
    GOOFY_SGPIO_D6,
    GOOFY_SGPIO_D7,
    GOOFY_SGPIO_E0, //32
    GOOFY_SGPIO_E1,
    GOOFY_SGPIO_E2,
    GOOFY_SGPIO_E3,
    GOOFY_SGPIO_E4,
    GOOFY_SGPIO_E5,
    GOOFY_SGPIO_E6,
    GOOFY_SGPIO_E7,
    GOOFY_SGPIO_F0, //40
    GOOFY_SGPIO_F1,
    GOOFY_SGPIO_F2,
    GOOFY_SGPIO_F3,
    GOOFY_SGPIO_F4,
    GOOFY_SGPIO_F5,
    GOOFY_SGPIO_F6,
    GOOFY_SGPIO_F7,
    GOOFY_SGPIO_G0, //48
    GOOFY_SGPIO_G1,
    GOOFY_SGPIO_G2,
    GOOFY_SGPIO_G3,
    GOOFY_SGPIO_G4,
    GOOFY_SGPIO_G5,
    GOOFY_SGPIO_G6,
    GOOFY_SGPIO_G7,
    GOOFY_SGPIO_H0, //56
    GOOFY_SGPIO_H1,
    GOOFY_SGPIO_H2,
    GOOFY_SGPIO_H3,
    GOOFY_SGPIO_H4,
    GOOFY_SGPIO_H5,
    GOOFY_SGPIO_H6,
    GOOFY_SGPIO_H7,
};

/*
 * Goofy Ethernet LED number
 */
enum {
    GOOFY_SGPIO_ETH_LED_A0 = 0,
    GOOFY_SGPIO_ETH_LED_A1,
    GOOFY_SGPIO_ETH_LED_A2,
    GOOFY_SGPIO_ETH_LED_A3,
    GOOFY_SGPIO_ETH_LED_A4,
    GOOFY_SGPIO_ETH_LED_A5,
    GOOFY_SGPIO_ETH_LED_A6,
    GOOFY_SGPIO_ETH_LED_A7,
    GOOFY_SGPIO_ETH_LED_MAX,
};

/*
 * GPIO register memory map
 */
#define GPIO_RESERVED0                    0x00000
#define GPIO_INOUT_CONFIG_REG2            0x00004
#define GPIO_INOUT_CONFIG_REG1            0x00008
#define GPIO_INOUT_CONFIG_REG0            0x0000C

#define GPIO_POLARITY_CTRL_REG1           0x00010
#define GPIO_POLARITY_CTRL_REG0           0x00014

#define GPIO_DEBOUNCE_CTRL_REG1           0x00018
#define GPIO_DEBOUNCE_CTRL_REG0           0x0001C
#define GPIO_DEBOUNCE_TIMER_REG5          0x00020
#define GPIO_DEBOUNCE_TIMER_REG4          0x00024
#define GPIO_DEBOUNCE_TIMER_REG3          0x00028
#define GPIO_DEBOUNCE_TIMER_REG2          0x0002C
#define GPIO_DEBOUNCE_TIMER_REG1          0x00030
#define GPIO_DEBOUNCE_TIMER_REG0          0x00034

#define GPIO_REGISTERED_INPUT_VAL_REG1    0x00038
#define GPIO_REGISTERED_INPUT_VAL_REG0    0x0003C
#define GPIO_REGISTERED_OUTPUT_VAL_REG1   0x00040
#define GPIO_REGISTERED_OUTPUT_VAL_REG0   0x00044
#define GPIO_RESERVED1                    0x00048
#define GPIO_INTR_TYPE_REG2               0x0004C
#define GPIO_INTR_TYPE_REG1               0x00050
#define GPIO_INTR_TYPE_REG0               0x00054

#define GPIO_LATCHED_INTR_REG1            0x00058
#define GPIO_LATCHED_INTR_REG0            0x0005C

#define GPIO_ERR_INTR_STATUS_REG1         0x00060
#define GPIO_ERR_INTR_STATUS_REG0         0x00064

#define GPIO_MAN_INTR_STATUS_REG1         0x00068
#define GPIO_MAN_INTR_STATUS_REG0         0x0006C

#define GPIO_NET_INTR_STATUS_REG1         0x00070
#define GPIO_NET_INTR_STATUS_REG0         0x00074

#define GPIO_ERR_INTR_ENABLE_REG1         0x00078
#define GPIO_ERR_INTR_ENABLE_REG0         0x0007C

#define GPIO_MAN_INTR_ENABLE_REG1         0x00080
#define GPIO_MAN_INTR_ENABLE_REG0         0x00084

#define GPIO_NET_INTR_ENABLE_REG1         0x00088
#define GPIO_NET_INTR_ENABLE_REG0         0x0008C

#define GPIO_INTR_TEST_REG1               0x00090
#define GPIO_INTR_TEST_REG0               0x00094
#define GPIO_RESERVED2                    0x00098 //0X98 TO 0X9C
#define SGPIO_OUTPUT_REG1                 0x000A0
#define SGPIO_OUTPUT_REG0                 0x000A4
#define SGPIO_INPUT_REG1                  0x000A8
#define SGPIO_INPUT_REG0                  0x000AC
#define SGPIO_SHIFT_REG_CONFIG_REG        0x000B0
#define GPIO_RESERVED3                    0x000B4
#define SGPIO_DEBOUNCE_CTRL_REG1          0x000B8
#define SGPIO_DEBOUNCE_CTRL_REG0          0x000BC

#define SGPIO_DEBOUNCE_TIME_REG7          0x000C0
#define SGPIO_DEBOUNCE_TIME_REG6          0x000C4
#define SGPIO_DEBOUNCE_TIME_REG5          0x000C8
#define SGPIO_DEBOUNCE_TIME_REG4          0x000CC
#define SGPIO_DEBOUNCE_TIME_REG3          0x000D0
#define SGPIO_DEBOUNCE_TIME_REG2          0x000D4
#define SGPIO_DEBOUNCE_TIME_REG1          0x000D8
#define SGPIO_DEBOUNCE_TIME_REG0          0x000DC

#define SGPIO_FUNCTION_TYPE_REG           0x000E0
#define GPIO_RESERVED4                    0x000E4

#define SGPIO_INTR_TEST_REG1              0x000E8
#define SGPIO_INTR_TEST_REG0              0x000EC

#define SGPIO_FAN_TACH_REG                0x000F0
#define GPIO_RESERVED5                    0x000F4

#define SGPIO_ETHERNET_LED_CONFIG_REG     0x000F8
#define SGPIO_ETHERNET_LED_ENABLE_REG     0x000FC
#define SGPIO_INTR_TYPE_REG3              0x00100
#define SGPIO_INTR_TYPE_REG2              0x00104
#define SGPIO_INTR_TYPE_REG1              0x00108
#define SGPIO_INTR_TYPE_REG0              0x0010C

#define SGPIO_ERR_INTR_EVENT_REG1         0x00110
#define SGPIO_ERR_INTR_EVENT_REG0         0x00114
#define SGPIO_MAN_INTR_EVENT_REG1         0x00118
#define SGPIO_MAN_INTR_EVENT_REG0         0x0011C
#define SGPIO_NET_INTR_EVENT_REG1         0x00120
#define SGPIO_NET_INTR_EVENT_REG0         0x00124

#define SGPIO_ERR_INTR_ENABLE_REG1        0x00128
#define SGPIO_ERR_INTR_ENABLE_REG0        0x0012C
#define SGPIO_MAN_INTR_ENABLE_REG1        0x00130
#define SGPIO_MAN_INTR_ENABLE_REG0        0x00134
#define SGPIO_NET_INTR_ENABLE_REG1        0x00138
#define SGPIO_NET_INTR_ENABLE_REG0        0x0013C

#define SGPIO_OUTPUT_POLARITY_CTRL_REG1   0x00140
#define SGPIO_OUTPUT_POLARITY_CTRL_REG0   0x00144
#define SGPIO_INPUT_POLARITY_CTRL_REG1    0x00148
#define SGPIO_INPUT_POLARITY_CTRL_REG0    0x0014C
#define GPIO_RESERVED6                    0x00150 // 0X150 TO 0X1FF

/*
 * Pin position within the corresonding 32 bit register
 */
#define OFFSET_GPIO_PIN0                      0
#define OFFSET_GPIO_PIN1                      1
#define OFFSET_GPIO_PIN2                      2
#define OFFSET_GPIO_PIN3                      3
#define OFFSET_GPIO_PIN4                      4
#define OFFSET_GPIO_PIN5                      5
#define OFFSET_GPIO_PIN6                      6
#define OFFSET_GPIO_PIN7                      7
#define OFFSET_GPIO_PIN8                      8
#define OFFSET_GPIO_PIN9                      9
#define OFFSET_GPIO_PIN10                     10
#define OFFSET_GPIO_PIN11                     11
#define OFFSET_GPIO_PIN12                     12
#define OFFSET_GPIO_PIN13                     13
#define OFFSET_GPIO_PIN14                     14
#define OFFSET_GPIO_PIN15                     15
#define OFFSET_GPIO_PIN16                     16
#define OFFSET_GPIO_PIN17                     17
#define OFFSET_GPIO_PIN18                     18
#define OFFSET_GPIO_PIN19                     19
#define OFFSET_GPIO_PIN20                     20
#define OFFSET_GPIO_PIN21                     21
#define OFFSET_GPIO_PIN22                     22
#define OFFSET_GPIO_PIN23                     23
#define OFFSET_GPIO_PIN24                     24
#define OFFSET_GPIO_PIN25                     25
#define OFFSET_GPIO_PIN26                     26
#define OFFSET_GPIO_PIN27                     27
#define OFFSET_GPIO_PIN28                     28
#define OFFSET_GPIO_PIN29                     29
#define OFFSET_GPIO_PIN30                     30
#define OFFSET_GPIO_PIN31                     31

/*
 * Pin position within the corresonding 32 bit register
 */
#define OFFSET_GPIO_PIN32                     0
#define OFFSET_GPIO_PIN33                     1
#define OFFSET_GPIO_PIN34                     2
#define OFFSET_GPIO_PIN35                     3
#define OFFSET_GPIO_PIN36                     4
#define OFFSET_GPIO_PIN37                     5
#define OFFSET_GPIO_PIN38                     6
#define OFFSET_GPIO_PIN39                     7
#define OFFSET_GPIO_PIN40                     8
#define OFFSET_GPIO_PIN41                     9
#define OFFSET_GPIO_PIN42                     10
#define OFFSET_GPIO_PIN43                     11
#define OFFSET_GPIO_PIN44                     12
#define OFFSET_GPIO_PIN45                     13
#define OFFSET_GPIO_PIN46                     14
#define OFFSET_GPIO_PIN47                     15

/*
 * Pin position within the corresonding 32 bit register
 */
#define OFFSET_GPIO_IN_OUT_CFG0_PIN0                        0
#define OFFSET_GPIO_IN_OUT_CFG0_PIN1                        2
#define OFFSET_GPIO_IN_OUT_CFG0_PIN2                        4
#define OFFSET_GPIO_IN_OUT_CFG0_PIN3                        6
#define OFFSET_GPIO_IN_OUT_CFG0_PIN4                        8
#define OFFSET_GPIO_IN_OUT_CFG0_PIN5                        10
#define OFFSET_GPIO_IN_OUT_CFG0_PIN6                        12
#define OFFSET_GPIO_IN_OUT_CFG0_PIN7                        14
#define OFFSET_GPIO_IN_OUT_CFG0_PIN8                        16
#define OFFSET_GPIO_IN_OUT_CFG0_PIN9                        18
#define OFFSET_GPIO_IN_OUT_CFG0_PIN10                       20
#define OFFSET_GPIO_IN_OUT_CFG0_PIN11                       22
#define OFFSET_GPIO_IN_OUT_CFG0_PIN12                       24
#define OFFSET_GPIO_IN_OUT_CFG0_PIN13                       26
#define OFFSET_GPIO_IN_OUT_CFG0_PIN14                       28
#define OFFSET_GPIO_IN_OUT_CFG0_PIN15                       30

/*
 * Pin position within the corresonding 32 bit register
 */
#define OFFSET_GPIO_IN_OUT_CFG1_PIN16                       0
#define OFFSET_GPIO_IN_OUT_CFG1_PIN17                       2
#define OFFSET_GPIO_IN_OUT_CFG1_PIN18                       4
#define OFFSET_GPIO_IN_OUT_CFG1_PIN19                       6
#define OFFSET_GPIO_IN_OUT_CFG1_PIN20                       8
#define OFFSET_GPIO_IN_OUT_CFG1_PIN21                       10
#define OFFSET_GPIO_IN_OUT_CFG1_PIN22                       12
#define OFFSET_GPIO_IN_OUT_CFG1_PIN23                       14
#define OFFSET_GPIO_IN_OUT_CFG1_PIN24                       16
#define OFFSET_GPIO_IN_OUT_CFG1_PIN25                       18
#define OFFSET_GPIO_IN_OUT_CFG1_PIN26                       20
#define OFFSET_GPIO_IN_OUT_CFG1_PIN27                       22
#define OFFSET_GPIO_IN_OUT_CFG1_PIN28                       24
#define OFFSET_GPIO_IN_OUT_CFG1_PIN29                       26
#define OFFSET_GPIO_IN_OUT_CFG1_PIN30                       28
#define OFFSET_GPIO_IN_OUT_CFG1_PIN31                       30

/*
 * Pin position within the corresonding 32 bit register
 */
#define OFFSET_GPIO_IN_OUT_CFG2_PIN32                       0
#define OFFSET_GPIO_IN_OUT_CFG2_PIN33                       2
#define OFFSET_GPIO_IN_OUT_CFG2_PIN34                       4
#define OFFSET_GPIO_IN_OUT_CFG2_PIN35                       6
#define OFFSET_GPIO_IN_OUT_CFG2_PIN36                       8
#define OFFSET_GPIO_IN_OUT_CFG2_PIN37                       10
#define OFFSET_GPIO_IN_OUT_CFG2_PIN38                       12
#define OFFSET_GPIO_IN_OUT_CFG2_PIN39                       14
#define OFFSET_GPIO_IN_OUT_CFG2_PIN40                       16
#define OFFSET_GPIO_IN_OUT_CFG2_PIN41                       18
#define OFFSET_GPIO_IN_OUT_CFG2_PIN42                       20
#define OFFSET_GPIO_IN_OUT_CFG2_PIN43                       22
#define OFFSET_GPIO_IN_OUT_CFG2_PIN44                       24
#define OFFSET_GPIO_IN_OUT_CFG2_PIN45                       26
#define OFFSET_GPIO_IN_OUT_CFG2_PIN46                       28
#define OFFSET_GPIO_IN_OUT_CFG2_PIN47                       30

/*
 * Pin position within the corresonding 32 bit register
 */
#define OFFSET_GPIO_POL_CTRL0_PIN0                      0
#define OFFSET_GPIO_POL_CTRL0_PIN1                      1
#define OFFSET_GPIO_POL_CTRL0_PIN2                      2
#define OFFSET_GPIO_POL_CTRL0_PIN3                      3
#define OFFSET_GPIO_POL_CTRL0_PIN4                      4
#define OFFSET_GPIO_POL_CTRL0_PIN5                      5
#define OFFSET_GPIO_POL_CTRL0_PIN6                      6
#define OFFSET_GPIO_POL_CTRL0_PIN7                      7
#define OFFSET_GPIO_POL_CTRL0_PIN8                      8
#define OFFSET_GPIO_POL_CTRL0_PIN9                      9
#define OFFSET_GPIO_POL_CTRL0_PIN10                     10
#define OFFSET_GPIO_POL_CTRL0_PIN11                     11
#define OFFSET_GPIO_POL_CTRL0_PIN12                     12
#define OFFSET_GPIO_POL_CTRL0_PIN13                     13
#define OFFSET_GPIO_POL_CTRL0_PIN14                     14
#define OFFSET_GPIO_POL_CTRL0_PIN15                     15
#define OFFSET_GPIO_POL_CTRL0_PIN16                     16
#define OFFSET_GPIO_POL_CTRL0_PIN17                     17
#define OFFSET_GPIO_POL_CTRL0_PIN18                     18
#define OFFSET_GPIO_POL_CTRL0_PIN19                     19
#define OFFSET_GPIO_POL_CTRL0_PIN20                     20
#define OFFSET_GPIO_POL_CTRL0_PIN21                     21
#define OFFSET_GPIO_POL_CTRL0_PIN22                     22
#define OFFSET_GPIO_POL_CTRL0_PIN23                     23
#define OFFSET_GPIO_POL_CTRL0_PIN24                     24
#define OFFSET_GPIO_POL_CTRL0_PIN25                     25
#define OFFSET_GPIO_POL_CTRL0_PIN26                     26
#define OFFSET_GPIO_POL_CTRL0_PIN27                     27
#define OFFSET_GPIO_POL_CTRL0_PIN28                     28
#define OFFSET_GPIO_POL_CTRL0_PIN29                     29
#define OFFSET_GPIO_POL_CTRL0_PIN30                     30
#define OFFSET_GPIO_POL_CTRL0_PIN31                     31

/*
 * Pin position within the corresonding 32 bit register
 */
#define OFFSET_GPIO_POL_CTRL1_PIN32                     0
#define OFFSET_GPIO_POL_CTRL1_PIN33                     1
#define OFFSET_GPIO_POL_CTRL1_PIN34                     2
#define OFFSET_GPIO_POL_CTRL1_PIN35                     3
#define OFFSET_GPIO_POL_CTRL1_PIN36                     4
#define OFFSET_GPIO_POL_CTRL1_PIN37                     5
#define OFFSET_GPIO_POL_CTRL1_PIN38                     6
#define OFFSET_GPIO_POL_CTRL1_PIN39                     7
#define OFFSET_GPIO_POL_CTRL1_PIN40                     8
#define OFFSET_GPIO_POL_CTRL1_PIN41                     9
#define OFFSET_GPIO_POL_CTRL1_PIN42                     10
#define OFFSET_GPIO_POL_CTRL1_PIN43                     11
#define OFFSET_GPIO_POL_CTRL1_PIN44                     12
#define OFFSET_GPIO_POL_CTRL1_PIN45                     13
#define OFFSET_GPIO_POL_CTRL1_PIN46                     14
#define OFFSET_GPIO_POL_CTRL1_PIN47                     15

/*
 * Pin position within the corresonding 32 bit register
 */
#define OFFSET_GPIO_DEBOUNCE_CTRL0_PIN0                      0
#define OFFSET_GPIO_DEBOUNCE_CTRL0_PIN1                      1
#define OFFSET_GPIO_DEBOUNCE_CTRL0_PIN2                      2
#define OFFSET_GPIO_DEBOUNCE_CTRL0_PIN3                      3
#define OFFSET_GPIO_DEBOUNCE_CTRL0_PIN4                      4
#define OFFSET_GPIO_DEBOUNCE_CTRL0_PIN5                      5
#define OFFSET_GPIO_DEBOUNCE_CTRL0_PIN6                      6
#define OFFSET_GPIO_DEBOUNCE_CTRL0_PIN7                      7
#define OFFSET_GPIO_DEBOUNCE_CTRL0_PIN8                      8
#define OFFSET_GPIO_DEBOUNCE_CTRL0_PIN9                      9
#define OFFSET_GPIO_DEBOUNCE_CTRL0_PIN10                     10
#define OFFSET_GPIO_DEBOUNCE_CTRL0_PIN11                     11
#define OFFSET_GPIO_DEBOUNCE_CTRL0_PIN12                     12
#define OFFSET_GPIO_DEBOUNCE_CTRL0_PIN13                     13
#define OFFSET_GPIO_DEBOUNCE_CTRL0_PIN14                     14
#define OFFSET_GPIO_DEBOUNCE_CTRL0_PIN15                     15
#define OFFSET_GPIO_DEBOUNCE_CTRL0_PIN16                     16
#define OFFSET_GPIO_DEBOUNCE_CTRL0_PIN17                     17
#define OFFSET_GPIO_DEBOUNCE_CTRL0_PIN18                     18
#define OFFSET_GPIO_DEBOUNCE_CTRL0_PIN19                     19
#define OFFSET_GPIO_DEBOUNCE_CTRL0_PIN20                     20
#define OFFSET_GPIO_DEBOUNCE_CTRL0_PIN21                     21
#define OFFSET_GPIO_DEBOUNCE_CTRL0_PIN22                     22
#define OFFSET_GPIO_DEBOUNCE_CTRL0_PIN23                     23
#define OFFSET_GPIO_DEBOUNCE_CTRL0_PIN24                     24
#define OFFSET_GPIO_DEBOUNCE_CTRL0_PIN25                     25
#define OFFSET_GPIO_DEBOUNCE_CTRL0_PIN26                     26
#define OFFSET_GPIO_DEBOUNCE_CTRL0_PIN27                     27
#define OFFSET_GPIO_DEBOUNCE_CTRL0_PIN28                     28
#define OFFSET_GPIO_DEBOUNCE_CTRL0_PIN29                     29
#define OFFSET_GPIO_DEBOUNCE_CTRL0_PIN30                     30
#define OFFSET_GPIO_DEBOUNCE_CTRL0_PIN31                     31

/*
 * Pin position within the corresonding 32 bit register
 */
#define OFFSET_GPIO_DEBOUNCE_CTRL1_PIN32                     0
#define OFFSET_GPIO_DEBOUNCE_CTRL1_PIN33                     1
#define OFFSET_GPIO_DEBOUNCE_CTRL1_PIN34                     2
#define OFFSET_GPIO_DEBOUNCE_CTRL1_PIN35                     3
#define OFFSET_GPIO_DEBOUNCE_CTRL1_PIN36                     4
#define OFFSET_GPIO_DEBOUNCE_CTRL1_PIN37                     5
#define OFFSET_GPIO_DEBOUNCE_CTRL1_PIN38                     6
#define OFFSET_GPIO_DEBOUNCE_CTRL1_PIN39                     7
#define OFFSET_GPIO_DEBOUNCE_CTRL1_PIN40                     8
#define OFFSET_GPIO_DEBOUNCE_CTRL1_PIN41                     9
#define OFFSET_GPIO_DEBOUNCE_CTRL1_PIN42                     10
#define OFFSET_GPIO_DEBOUNCE_CTRL1_PIN43                     11
#define OFFSET_GPIO_DEBOUNCE_CTRL1_PIN44                     12
#define OFFSET_GPIO_DEBOUNCE_CTRL1_PIN45                     13
#define OFFSET_GPIO_DEBOUNCE_CTRL1_PIN46                     14
#define OFFSET_GPIO_DEBOUNCE_CTRL1_PIN47                     15

/*
 * Pin position within the corresonding 32 bit register
 */
#define OFFSET_GPIO_INTR_TYPE0_PIN0                         0
#define OFFSET_GPIO_INTR_TYPE0_PIN1                         2
#define OFFSET_GPIO_INTR_TYPE0_PIN2                         4
#define OFFSET_GPIO_INTR_TYPE0_PIN3                         6
#define OFFSET_GPIO_INTR_TYPE0_PIN4                         8
#define OFFSET_GPIO_INTR_TYPE0_PIN5                         10
#define OFFSET_GPIO_INTR_TYPE0_PIN6                         12
#define OFFSET_GPIO_INTR_TYPE0_PIN7                         14
#define OFFSET_GPIO_INTR_TYPE0_PIN8                         16
#define OFFSET_GPIO_INTR_TYPE0_PIN9                         18
#define OFFSET_GPIO_INTR_TYPE0_PIN10                        20
#define OFFSET_GPIO_INTR_TYPE0_PIN11                        22
#define OFFSET_GPIO_INTR_TYPE0_PIN12                        24
#define OFFSET_GPIO_INTR_TYPE0_PIN13                        26
#define OFFSET_GPIO_INTR_TYPE0_PIN14                        28
#define OFFSET_GPIO_INTR_TYPE0_PIN15                        30

/*
 * Pin position within the corresonding 32 bit register
 */
#define OFFSET_GPIO_INTR_TYPE1_PIN16                        0
#define OFFSET_GPIO_INTR_TYPE1_PIN17                        2
#define OFFSET_GPIO_INTR_TYPE1_PIN18                        4
#define OFFSET_GPIO_INTR_TYPE1_PIN19                        6
#define OFFSET_GPIO_INTR_TYPE1_PIN20                        8
#define OFFSET_GPIO_INTR_TYPE1_PIN21                        10
#define OFFSET_GPIO_INTR_TYPE1_PIN22                        12
#define OFFSET_GPIO_INTR_TYPE1_PIN23                        14
#define OFFSET_GPIO_INTR_TYPE1_PIN24                        16
#define OFFSET_GPIO_INTR_TYPE1_PIN25                        18
#define OFFSET_GPIO_INTR_TYPE1_PIN26                        20
#define OFFSET_GPIO_INTR_TYPE1_PIN27                        22
#define OFFSET_GPIO_INTR_TYPE1_PIN28                        24
#define OFFSET_GPIO_INTR_TYPE1_PIN29                        26
#define OFFSET_GPIO_INTR_TYPE1_PIN30                        28
#define OFFSET_GPIO_INTR_TYPE1_PIN31                        30

/*
 * Pin position within the corresonding 32 bit register
 */
#define OFFSET_GPIO_INTR_TYPE2_PIN32                        0
#define OFFSET_GPIO_INTR_TYPE2_PIN33                        2
#define OFFSET_GPIO_INTR_TYPE2_PIN34                        4
#define OFFSET_GPIO_INTR_TYPE2_PIN35                        6
#define OFFSET_GPIO_INTR_TYPE2_PIN36                        8
#define OFFSET_GPIO_INTR_TYPE2_PIN37                        10
#define OFFSET_GPIO_INTR_TYPE2_PIN38                        12
#define OFFSET_GPIO_INTR_TYPE2_PIN39                        14
#define OFFSET_GPIO_INTR_TYPE2_PIN40                        16
#define OFFSET_GPIO_INTR_TYPE2_PIN41                        18
#define OFFSET_GPIO_INTR_TYPE2_PIN42                        20
#define OFFSET_GPIO_INTR_TYPE2_PIN43                        22
#define OFFSET_GPIO_INTR_TYPE2_PIN44                        24
#define OFFSET_GPIO_INTR_TYPE2_PIN45                        26
#define OFFSET_GPIO_INTR_TYPE2_PIN46                        28
#define OFFSET_GPIO_INTR_TYPE2_PIN47                        30

/*
 * Pin position within the corresonding 32 bit register
 */
#define OFFSET_SGPIO_OUTPUT0_A0                             0
#define OFFSET_SGPIO_OUTPUT0_A1                             1
#define OFFSET_SGPIO_OUTPUT0_A2                             2
#define OFFSET_SGPIO_OUTPUT0_A3                             3
#define OFFSET_SGPIO_OUTPUT0_A4                             4
#define OFFSET_SGPIO_OUTPUT0_A5                             5
#define OFFSET_SGPIO_OUTPUT0_A6                             6
#define OFFSET_SGPIO_OUTPUT0_A7                             7
#define OFFSET_SGPIO_OUTPUT0_B0                             8
#define OFFSET_SGPIO_OUTPUT0_B1                             9
#define OFFSET_SGPIO_OUTPUT0_B2                             10
#define OFFSET_SGPIO_OUTPUT0_B3                             11
#define OFFSET_SGPIO_OUTPUT0_B4                             12
#define OFFSET_SGPIO_OUTPUT0_B5                             13
#define OFFSET_SGPIO_OUTPUT0_B6                             14
#define OFFSET_SGPIO_OUTPUT0_B7                             15
#define OFFSET_SGPIO_OUTPUT0_C0                             16
#define OFFSET_SGPIO_OUTPUT0_C1                             17
#define OFFSET_SGPIO_OUTPUT0_C2                             18
#define OFFSET_SGPIO_OUTPUT0_C3                             19
#define OFFSET_SGPIO_OUTPUT0_C4                             20
#define OFFSET_SGPIO_OUTPUT0_C5                             21
#define OFFSET_SGPIO_OUTPUT0_C6                             22
#define OFFSET_SGPIO_OUTPUT0_C7                             23
#define OFFSET_SGPIO_OUTPUT0_D0                             24
#define OFFSET_SGPIO_OUTPUT0_D1                             25
#define OFFSET_SGPIO_OUTPUT0_D2                             26
#define OFFSET_SGPIO_OUTPUT0_D3                             27
#define OFFSET_SGPIO_OUTPUT0_D4                             28
#define OFFSET_SGPIO_OUTPUT0_D5                             29
#define OFFSET_SGPIO_OUTPUT0_D6                             30
#define OFFSET_SGPIO_OUTPUT0_D7                             31

/*
 * Pin position within the corresonding 32 bit register
 */
#define OFFSET_SGPIO_OUTPUT1_E0                             0
#define OFFSET_SGPIO_OUTPUT1_E1                             1
#define OFFSET_SGPIO_OUTPUT1_E2                             2
#define OFFSET_SGPIO_OUTPUT1_E3                             3
#define OFFSET_SGPIO_OUTPUT1_E4                             4
#define OFFSET_SGPIO_OUTPUT1_E5                             5
#define OFFSET_SGPIO_OUTPUT1_E6                             6
#define OFFSET_SGPIO_OUTPUT1_E7                             7
#define OFFSET_SGPIO_OUTPUT1_F0                             8
#define OFFSET_SGPIO_OUTPUT1_F1                             9
#define OFFSET_SGPIO_OUTPUT1_F2                             10
#define OFFSET_SGPIO_OUTPUT1_F3                             11
#define OFFSET_SGPIO_OUTPUT1_F4                             12
#define OFFSET_SGPIO_OUTPUT1_F5                             13
#define OFFSET_SGPIO_OUTPUT1_F6                             14
#define OFFSET_SGPIO_OUTPUT1_F7                             15
#define OFFSET_SGPIO_OUTPUT1_G0                             16
#define OFFSET_SGPIO_OUTPUT1_G1                             17
#define OFFSET_SGPIO_OUTPUT1_G2                             18
#define OFFSET_SGPIO_OUTPUT1_G3                             19
#define OFFSET_SGPIO_OUTPUT1_G4                             20
#define OFFSET_SGPIO_OUTPUT1_G5                             21
#define OFFSET_SGPIO_OUTPUT1_G6                             22
#define OFFSET_SGPIO_OUTPUT1_G7                             23
#define OFFSET_SGPIO_OUTPUT1_H0                             24
#define OFFSET_SGPIO_OUTPUT1_H1                             25
#define OFFSET_SGPIO_OUTPUT1_H2                             26
#define OFFSET_SGPIO_OUTPUT1_H3                             27
#define OFFSET_SGPIO_OUTPUT1_H4                             28
#define OFFSET_SGPIO_OUTPUT1_H5                             29
#define OFFSET_SGPIO_OUTPUT1_H6                             30
#define OFFSET_SGPIO_OUTPUT1_H7                             31

/*
 * Pin position within the corresonding 32 bit register
 */
#define OFFSET_SGPIO_INPUT0_A0                              0
#define OFFSET_SGPIO_INPUT0_A1                              1
#define OFFSET_SGPIO_INPUT0_A2                              2
#define OFFSET_SGPIO_INPUT0_A3                              3
#define OFFSET_SGPIO_INPUT0_A4                              4
#define OFFSET_SGPIO_INPUT0_A5                              5
#define OFFSET_SGPIO_INPUT0_A6                              6
#define OFFSET_SGPIO_INPUT0_A7                              7
#define OFFSET_SGPIO_INPUT0_B0                              8
#define OFFSET_SGPIO_INPUT0_B1                              9
#define OFFSET_SGPIO_INPUT0_B2                              10
#define OFFSET_SGPIO_INPUT0_B3                              11
#define OFFSET_SGPIO_INPUT0_B4                              12
#define OFFSET_SGPIO_INPUT0_B5                              13
#define OFFSET_SGPIO_INPUT0_B6                              14
#define OFFSET_SGPIO_INPUT0_B7                              15
#define OFFSET_SGPIO_INPUT0_C0                              16
#define OFFSET_SGPIO_INPUT0_C1                              17
#define OFFSET_SGPIO_INPUT0_C2                              18
#define OFFSET_SGPIO_INPUT0_C3                              19
#define OFFSET_SGPIO_INPUT0_C4                              20
#define OFFSET_SGPIO_INPUT0_C5                              21
#define OFFSET_SGPIO_INPUT0_C6                              22
#define OFFSET_SGPIO_INPUT0_C7                              23
#define OFFSET_SGPIO_INPUT0_D0                              24
#define OFFSET_SGPIO_INPUT0_D1                              25
#define OFFSET_SGPIO_INPUT0_D2                              26
#define OFFSET_SGPIO_INPUT0_D3                              27
#define OFFSET_SGPIO_INPUT0_D4                              28
#define OFFSET_SGPIO_INPUT0_D5                              29
#define OFFSET_SGPIO_INPUT0_D6                              30
#define OFFSET_SGPIO_INPUT0_D7                              31

/*
 * Pin position within the corresonding 32 bit register
 */
#define OFFSET_SGPIO_INPUT1_E0                              0
#define OFFSET_SGPIO_INPUT1_E1                              1
#define OFFSET_SGPIO_INPUT1_E2                              2
#define OFFSET_SGPIO_INPUT1_E3                              3
#define OFFSET_SGPIO_INPUT1_E4                              4
#define OFFSET_SGPIO_INPUT1_E5                              5
#define OFFSET_SGPIO_INPUT1_E6                              6
#define OFFSET_SGPIO_INPUT1_E7                              7
#define OFFSET_SGPIO_INPUT1_F0                              8
#define OFFSET_SGPIO_INPUT1_F1                              9
#define OFFSET_SGPIO_INPUT1_F2                              10
#define OFFSET_SGPIO_INPUT1_F3                              11
#define OFFSET_SGPIO_INPUT1_F4                              12
#define OFFSET_SGPIO_INPUT1_F5                              13
#define OFFSET_SGPIO_INPUT1_F6                              14
#define OFFSET_SGPIO_INPUT1_F7                              15
#define OFFSET_SGPIO_INPUT1_G0                              16
#define OFFSET_SGPIO_INPUT1_G1                              17
#define OFFSET_SGPIO_INPUT1_G2                              18
#define OFFSET_SGPIO_INPUT1_G3                              19
#define OFFSET_SGPIO_INPUT1_G4                              20
#define OFFSET_SGPIO_INPUT1_G5                              21
#define OFFSET_SGPIO_INPUT1_G6                              22
#define OFFSET_SGPIO_INPUT1_G7                              23
#define OFFSET_SGPIO_INPUT1_H0                              24
#define OFFSET_SGPIO_INPUT1_H1                              25
#define OFFSET_SGPIO_INPUT1_H2                              26
#define OFFSET_SGPIO_INPUT1_H3                              27
#define OFFSET_SGPIO_INPUT1_H4                              28
#define OFFSET_SGPIO_INPUT1_H5                              29
#define OFFSET_SGPIO_INPUT1_H6                              30
#define OFFSET_SGPIO_INPUT1_H7                              31

/*
 * Pin position within the corresonding 32 bit register
 */
#define OFFSET_SGPIO_SHIFT_REG_CFG_SPEED                    0
#define OFFSET_SGPIO_SHIFT_REG_CFG_SIZE                     8
#define OFFSET_SGPIO_SHIFT_REG_CFG_VALID                    15

/*
 * Pin position within the corresonding 32 bit register
 */
#define OFFSET_SGPIO_DEBOUNCE_CTRL0_A0                      0
#define OFFSET_SGPIO_DEBOUNCE_CTRL0_A1                      1
#define OFFSET_SGPIO_DEBOUNCE_CTRL0_A2                      2
#define OFFSET_SGPIO_DEBOUNCE_CTRL0_A3                      3
#define OFFSET_SGPIO_DEBOUNCE_CTRL0_A4                      4
#define OFFSET_SGPIO_DEBOUNCE_CTRL0_A5                      5
#define OFFSET_SGPIO_DEBOUNCE_CTRL0_A6                      6
#define OFFSET_SGPIO_DEBOUNCE_CTRL0_A7                      7
#define OFFSET_SGPIO_DEBOUNCE_CTRL0_B0                      8
#define OFFSET_SGPIO_DEBOUNCE_CTRL0_B1                      9
#define OFFSET_SGPIO_DEBOUNCE_CTRL0_B2                      10
#define OFFSET_SGPIO_DEBOUNCE_CTRL0_B3                      11
#define OFFSET_SGPIO_DEBOUNCE_CTRL0_B4                      12
#define OFFSET_SGPIO_DEBOUNCE_CTRL0_B5                      13
#define OFFSET_SGPIO_DEBOUNCE_CTRL0_B6                      14
#define OFFSET_SGPIO_DEBOUNCE_CTRL0_B7                      15
#define OFFSET_SGPIO_DEBOUNCE_CTRL0_C0                      16
#define OFFSET_SGPIO_DEBOUNCE_CTRL0_C1                      17
#define OFFSET_SGPIO_DEBOUNCE_CTRL0_C2                      18
#define OFFSET_SGPIO_DEBOUNCE_CTRL0_C3                      19
#define OFFSET_SGPIO_DEBOUNCE_CTRL0_C4                      20
#define OFFSET_SGPIO_DEBOUNCE_CTRL0_C5                      21
#define OFFSET_SGPIO_DEBOUNCE_CTRL0_C6                      22
#define OFFSET_SGPIO_DEBOUNCE_CTRL0_C7                      23
#define OFFSET_SGPIO_DEBOUNCE_CTRL0_D0                      24
#define OFFSET_SGPIO_DEBOUNCE_CTRL0_D1                      25
#define OFFSET_SGPIO_DEBOUNCE_CTRL0_D2                      26
#define OFFSET_SGPIO_DEBOUNCE_CTRL0_D3                      27
#define OFFSET_SGPIO_DEBOUNCE_CTRL0_D4                      28
#define OFFSET_SGPIO_DEBOUNCE_CTRL0_D5                      29
#define OFFSET_SGPIO_DEBOUNCE_CTRL0_D6                      30
#define OFFSET_SGPIO_DEBOUNCE_CTRL0_D7                      31

/*
 * Pin position within the corresonding 32 bit register
 */
#define OFFSET_SGPIO_DEBOUNCE_CTRL1_E0                      0
#define OFFSET_SGPIO_DEBOUNCE_CTRL1_E1                      1
#define OFFSET_SGPIO_DEBOUNCE_CTRL1_E2                      2
#define OFFSET_SGPIO_DEBOUNCE_CTRL1_E3                      3
#define OFFSET_SGPIO_DEBOUNCE_CTRL1_E4                      4
#define OFFSET_SGPIO_DEBOUNCE_CTRL1_E5                      5
#define OFFSET_SGPIO_DEBOUNCE_CTRL1_E6                      6
#define OFFSET_SGPIO_DEBOUNCE_CTRL1_E7                      7
#define OFFSET_SGPIO_DEBOUNCE_CTRL1_F0                      8
#define OFFSET_SGPIO_DEBOUNCE_CTRL1_F1                      9
#define OFFSET_SGPIO_DEBOUNCE_CTRL1_F2                      10
#define OFFSET_SGPIO_DEBOUNCE_CTRL1_F3                      11
#define OFFSET_SGPIO_DEBOUNCE_CTRL1_F4                      12
#define OFFSET_SGPIO_DEBOUNCE_CTRL1_F5                      13
#define OFFSET_SGPIO_DEBOUNCE_CTRL1_F6                      14
#define OFFSET_SGPIO_DEBOUNCE_CTRL1_F7                      15
#define OFFSET_SGPIO_DEBOUNCE_CTRL1_G0                      16
#define OFFSET_SGPIO_DEBOUNCE_CTRL1_G1                      17
#define OFFSET_SGPIO_DEBOUNCE_CTRL1_G2                      18
#define OFFSET_SGPIO_DEBOUNCE_CTRL1_G3                      19
#define OFFSET_SGPIO_DEBOUNCE_CTRL1_G4                      20
#define OFFSET_SGPIO_DEBOUNCE_CTRL1_G5                      21
#define OFFSET_SGPIO_DEBOUNCE_CTRL1_G6                      22
#define OFFSET_SGPIO_DEBOUNCE_CTRL1_G7                      23
#define OFFSET_SGPIO_DEBOUNCE_CTRL1_H0                      24
#define OFFSET_SGPIO_DEBOUNCE_CTRL1_H1                      25
#define OFFSET_SGPIO_DEBOUNCE_CTRL1_H2                      26
#define OFFSET_SGPIO_DEBOUNCE_CTRL1_H3                      27
#define OFFSET_SGPIO_DEBOUNCE_CTRL1_H4                      28
#define OFFSET_SGPIO_DEBOUNCE_CTRL1_H5                      29
#define OFFSET_SGPIO_DEBOUNCE_CTRL1_H6                      30
#define OFFSET_SGPIO_DEBOUNCE_CTRL1_H7                      31

/*
 * Pin position within the corresonding 32 bit register
 */
#define OFFSET_SGPIO_FUNCTION_TYPE_FAN_A0                   0
#define OFFSET_SGPIO_FUNCTION_TYPE_FAN_A1                   1
#define OFFSET_SGPIO_FUNCTION_TYPE_FAN_A2                   2
#define OFFSET_SGPIO_FUNCTION_TYPE_FAN_A3                   3

/*
 * Pin position within the corresonding 32 bit register
 */
#define OFFSET_SGPIO_INTR_TEST0_A0                          0
#define OFFSET_SGPIO_INTR_TEST0_A1                          1
#define OFFSET_SGPIO_INTR_TEST0_A2                          2
#define OFFSET_SGPIO_INTR_TEST0_A3                          3
#define OFFSET_SGPIO_INTR_TEST0_A4                          4
#define OFFSET_SGPIO_INTR_TEST0_A5                          5
#define OFFSET_SGPIO_INTR_TEST0_A6                          6
#define OFFSET_SGPIO_INTR_TEST0_A7                          7
#define OFFSET_SGPIO_INTR_TEST0_B0                          8
#define OFFSET_SGPIO_INTR_TEST0_B1                          9
#define OFFSET_SGPIO_INTR_TEST0_B2                          10
#define OFFSET_SGPIO_INTR_TEST0_B3                          11
#define OFFSET_SGPIO_INTR_TEST0_B4                          12
#define OFFSET_SGPIO_INTR_TEST0_B5                          13
#define OFFSET_SGPIO_INTR_TEST0_B6                          14
#define OFFSET_SGPIO_INTR_TEST0_B7                          15
#define OFFSET_SGPIO_INTR_TEST0_C0                          16
#define OFFSET_SGPIO_INTR_TEST0_C1                          17
#define OFFSET_SGPIO_INTR_TEST0_C2                          18
#define OFFSET_SGPIO_INTR_TEST0_C3                          19
#define OFFSET_SGPIO_INTR_TEST0_C4                          20
#define OFFSET_SGPIO_INTR_TEST0_C5                          21
#define OFFSET_SGPIO_INTR_TEST0_C6                          22
#define OFFSET_SGPIO_INTR_TEST0_C7                          23
#define OFFSET_SGPIO_INTR_TEST0_D0                          24
#define OFFSET_SGPIO_INTR_TEST0_D1                          25
#define OFFSET_SGPIO_INTR_TEST0_D2                          26
#define OFFSET_SGPIO_INTR_TEST0_D3                          27
#define OFFSET_SGPIO_INTR_TEST0_D4                          28
#define OFFSET_SGPIO_INTR_TEST0_D5                          29
#define OFFSET_SGPIO_INTR_TEST0_D6                          30
#define OFFSET_SGPIO_INTR_TEST0_D7                          31

/*
 * Pin position within the corresonding 32 bit register
 */
#define OFFSET_SGPIO_INTR_TEST1_E0                          0
#define OFFSET_SGPIO_INTR_TEST1_E1                          1
#define OFFSET_SGPIO_INTR_TEST1_E2                          2
#define OFFSET_SGPIO_INTR_TEST1_E3                          3
#define OFFSET_SGPIO_INTR_TEST1_E4                          4
#define OFFSET_SGPIO_INTR_TEST1_E5                          5
#define OFFSET_SGPIO_INTR_TEST1_E6                          6
#define OFFSET_SGPIO_INTR_TEST1_E7                          7
#define OFFSET_SGPIO_INTR_TEST1_F0                          8
#define OFFSET_SGPIO_INTR_TEST1_F1                          9
#define OFFSET_SGPIO_INTR_TEST1_F2                          10
#define OFFSET_SGPIO_INTR_TEST1_F3                          11
#define OFFSET_SGPIO_INTR_TEST1_F4                          12
#define OFFSET_SGPIO_INTR_TEST1_F5                          13
#define OFFSET_SGPIO_INTR_TEST1_F6                          14
#define OFFSET_SGPIO_INTR_TEST1_F7                          15
#define OFFSET_SGPIO_INTR_TEST1_G0                          16
#define OFFSET_SGPIO_INTR_TEST1_G1                          17
#define OFFSET_SGPIO_INTR_TEST1_G2                          18
#define OFFSET_SGPIO_INTR_TEST1_G3                          19
#define OFFSET_SGPIO_INTR_TEST1_G4                          20
#define OFFSET_SGPIO_INTR_TEST1_G5                          21
#define OFFSET_SGPIO_INTR_TEST1_G6                          22
#define OFFSET_SGPIO_INTR_TEST1_G7                          23
#define OFFSET_SGPIO_INTR_TEST1_H0                          24
#define OFFSET_SGPIO_INTR_TEST1_H1                          25
#define OFFSET_SGPIO_INTR_TEST1_H2                          26
#define OFFSET_SGPIO_INTR_TEST1_H3                          27
#define OFFSET_SGPIO_INTR_TEST1_H4                          28
#define OFFSET_SGPIO_INTR_TEST1_H5                          29
#define OFFSET_SGPIO_INTR_TEST1_H6                          30
#define OFFSET_SGPIO_INTR_TEST1_H7                          31

/*
 * Pin position within the corresonding 32 bit register
 */
#define OFFSET_SGPIO_FAN_TACH_MAX_LIMIT_1                   0
#define OFFSET_SGPIO_FAN_TACH_MAX_LIMIT_2                   8
#define OFFSET_SGPIO_FAN_TACH_MAX_LIMIT_3                   16
#define OFFSET_SGPIO_FAN_TACH_MAX_LIMIT_4                   24

#define OFFSET_SGPIO_ETH_LED_CFG_PAUSE                      0
#define OFFSET_SGPIO_ETH_LED_CFG_OFF                        8
#define OFFSET_SGPIO_ETH_LED_CFG_ON                         12

#define OFFSET_SGPIO_ETH_LED_EN_A0                          0
#define OFFSET_SGPIO_ETH_LED_EN_A1                          2
#define OFFSET_SGPIO_ETH_LED_EN_A2                          4
#define OFFSET_SGPIO_ETH_LED_EN_A3                          6
#define OFFSET_SGPIO_ETH_LED_EN_A4                          8
#define OFFSET_SGPIO_ETH_LED_EN_A5                          10
#define OFFSET_SGPIO_ETH_LED_EN_A6                          12
#define OFFSET_SGPIO_ETH_LED_EN_A7                          14

/*
 * Pin position within the corresonding 32 bit register
 */
#define OFFSET_SGPIO_INTR_TYPE0_A0                          0
#define OFFSET_SGPIO_INTR_TYPE0_A1                          2
#define OFFSET_SGPIO_INTR_TYPE0_A2                          4
#define OFFSET_SGPIO_INTR_TYPE0_A3                          6
#define OFFSET_SGPIO_INTR_TYPE0_A4                          8
#define OFFSET_SGPIO_INTR_TYPE0_A5                          10
#define OFFSET_SGPIO_INTR_TYPE0_A6                          12
#define OFFSET_SGPIO_INTR_TYPE0_A7                          14
#define OFFSET_SGPIO_INTR_TYPE0_B0                          16
#define OFFSET_SGPIO_INTR_TYPE0_B1                          18
#define OFFSET_SGPIO_INTR_TYPE0_B2                          20
#define OFFSET_SGPIO_INTR_TYPE0_B3                          22
#define OFFSET_SGPIO_INTR_TYPE0_B4                          24
#define OFFSET_SGPIO_INTR_TYPE0_B5                          26
#define OFFSET_SGPIO_INTR_TYPE0_B6                          28
#define OFFSET_SGPIO_INTR_TYPE0_B7                          30

/*
 * Pin position within the corresonding 32 bit register
 */
#define OFFSET_SGPIO_INTR_TYPE1_C0                          0
#define OFFSET_SGPIO_INTR_TYPE1_C1                          2
#define OFFSET_SGPIO_INTR_TYPE1_C2                          4
#define OFFSET_SGPIO_INTR_TYPE1_C3                          6
#define OFFSET_SGPIO_INTR_TYPE1_C4                          8
#define OFFSET_SGPIO_INTR_TYPE1_C5                          10
#define OFFSET_SGPIO_INTR_TYPE1_C6                          12
#define OFFSET_SGPIO_INTR_TYPE1_C7                          14
#define OFFSET_SGPIO_INTR_TYPE1_D0                          16
#define OFFSET_SGPIO_INTR_TYPE1_D1                          18
#define OFFSET_SGPIO_INTR_TYPE1_D2                          20
#define OFFSET_SGPIO_INTR_TYPE1_D3                          22
#define OFFSET_SGPIO_INTR_TYPE1_D4                          24
#define OFFSET_SGPIO_INTR_TYPE1_D5                          26
#define OFFSET_SGPIO_INTR_TYPE1_D6                          28
#define OFFSET_SGPIO_INTR_TYPE1_D7                          30

/*
 * Pin position within the corresonding 32 bit register
 */
#define OFFSET_SGPIO_INTR_TYPE2_E0                          0
#define OFFSET_SGPIO_INTR_TYPE2_E1                          2
#define OFFSET_SGPIO_INTR_TYPE2_E2                          4
#define OFFSET_SGPIO_INTR_TYPE2_E3                          6
#define OFFSET_SGPIO_INTR_TYPE2_E4                          8
#define OFFSET_SGPIO_INTR_TYPE2_E5                          10
#define OFFSET_SGPIO_INTR_TYPE2_E6                          12
#define OFFSET_SGPIO_INTR_TYPE2_E7                          14
#define OFFSET_SGPIO_INTR_TYPE2_F0                          16
#define OFFSET_SGPIO_INTR_TYPE2_F1                          18
#define OFFSET_SGPIO_INTR_TYPE2_F2                          20
#define OFFSET_SGPIO_INTR_TYPE2_F3                          22
#define OFFSET_SGPIO_INTR_TYPE2_F4                          24
#define OFFSET_SGPIO_INTR_TYPE2_F5                          26
#define OFFSET_SGPIO_INTR_TYPE2_F6                          28
#define OFFSET_SGPIO_INTR_TYPE2_F7                          30

/*
 * Pin position within the corresonding 32 bit register
 */
#define OFFSET_SGPIO_INTR_TYPE3_G0                          0
#define OFFSET_SGPIO_INTR_TYPE3_G1                          2
#define OFFSET_SGPIO_INTR_TYPE3_G2                          4
#define OFFSET_SGPIO_INTR_TYPE3_G3                          6
#define OFFSET_SGPIO_INTR_TYPE3_G4                          8
#define OFFSET_SGPIO_INTR_TYPE3_G5                          10
#define OFFSET_SGPIO_INTR_TYPE3_G6                          12
#define OFFSET_SGPIO_INTR_TYPE3_G7                          14
#define OFFSET_SGPIO_INTR_TYPE3_H0                          16
#define OFFSET_SGPIO_INTR_TYPE3_H1                          18
#define OFFSET_SGPIO_INTR_TYPE3_H2                          20
#define OFFSET_SGPIO_INTR_TYPE3_H3                          22
#define OFFSET_SGPIO_INTR_TYPE3_H4                          24
#define OFFSET_SGPIO_INTR_TYPE3_H5                          26
#define OFFSET_SGPIO_INTR_TYPE3_H6                          28
#define OFFSET_SGPIO_INTR_TYPE3_H7                          30

/*
 * Pin position within the corresonding 32 bit register
 */
#define OFFSET_SGPIO_ERR_INTR_EVENT0_A0                     0
#define OFFSET_SGPIO_ERR_INTR_EVENT0_A1                     1
#define OFFSET_SGPIO_ERR_INTR_EVENT0_A2                     2
#define OFFSET_SGPIO_ERR_INTR_EVENT0_A3                     3
#define OFFSET_SGPIO_ERR_INTR_EVENT0_A4                     4
#define OFFSET_SGPIO_ERR_INTR_EVENT0_A5                     5
#define OFFSET_SGPIO_ERR_INTR_EVENT0_A6                     6
#define OFFSET_SGPIO_ERR_INTR_EVENT0_A7                     7
#define OFFSET_SGPIO_ERR_INTR_EVENT0_B0                     8
#define OFFSET_SGPIO_ERR_INTR_EVENT0_B1                     9
#define OFFSET_SGPIO_ERR_INTR_EVENT0_B2                     10
#define OFFSET_SGPIO_ERR_INTR_EVENT0_B3                     11
#define OFFSET_SGPIO_ERR_INTR_EVENT0_B4                     12
#define OFFSET_SGPIO_ERR_INTR_EVENT0_B5                     13
#define OFFSET_SGPIO_ERR_INTR_EVENT0_B6                     14
#define OFFSET_SGPIO_ERR_INTR_EVENT0_B7                     15
#define OFFSET_SGPIO_ERR_INTR_EVENT0_C0                     16
#define OFFSET_SGPIO_ERR_INTR_EVENT0_C1                     17
#define OFFSET_SGPIO_ERR_INTR_EVENT0_C2                     18
#define OFFSET_SGPIO_ERR_INTR_EVENT0_C3                     19
#define OFFSET_SGPIO_ERR_INTR_EVENT0_C4                     20
#define OFFSET_SGPIO_ERR_INTR_EVENT0_C5                     21
#define OFFSET_SGPIO_ERR_INTR_EVENT0_C6                     22
#define OFFSET_SGPIO_ERR_INTR_EVENT0_C7                     23
#define OFFSET_SGPIO_ERR_INTR_EVENT0_D0                     24
#define OFFSET_SGPIO_ERR_INTR_EVENT0_D1                     25
#define OFFSET_SGPIO_ERR_INTR_EVENT0_D2                     26
#define OFFSET_SGPIO_ERR_INTR_EVENT0_D3                     27
#define OFFSET_SGPIO_ERR_INTR_EVENT0_D4                     28
#define OFFSET_SGPIO_ERR_INTR_EVENT0_D5                     29
#define OFFSET_SGPIO_ERR_INTR_EVENT0_D6                     30
#define OFFSET_SGPIO_ERR_INTR_EVENT0_D7                     31

/*
 * Pin position within the corresonding 32 bit register
 */
#define OFFSET_SGPIO_ERR_INTR_EVENT1_E0                     0
#define OFFSET_SGPIO_ERR_INTR_EVENT1_E1                     1
#define OFFSET_SGPIO_ERR_INTR_EVENT1_E2                     2
#define OFFSET_SGPIO_ERR_INTR_EVENT1_E3                     3
#define OFFSET_SGPIO_ERR_INTR_EVENT1_E4                     4
#define OFFSET_SGPIO_ERR_INTR_EVENT1_E5                     5
#define OFFSET_SGPIO_ERR_INTR_EVENT1_E6                     6
#define OFFSET_SGPIO_ERR_INTR_EVENT1_E7                     7
#define OFFSET_SGPIO_ERR_INTR_EVENT1_F0                     8
#define OFFSET_SGPIO_ERR_INTR_EVENT1_F1                     9
#define OFFSET_SGPIO_ERR_INTR_EVENT1_F2                     10
#define OFFSET_SGPIO_ERR_INTR_EVENT1_F3                     11
#define OFFSET_SGPIO_ERR_INTR_EVENT1_F4                     12
#define OFFSET_SGPIO_ERR_INTR_EVENT1_F5                     13
#define OFFSET_SGPIO_ERR_INTR_EVENT1_F6                     14
#define OFFSET_SGPIO_ERR_INTR_EVENT1_F7                     15
#define OFFSET_SGPIO_ERR_INTR_EVENT1_G0                     16
#define OFFSET_SGPIO_ERR_INTR_EVENT1_G1                     17
#define OFFSET_SGPIO_ERR_INTR_EVENT1_G2                     18
#define OFFSET_SGPIO_ERR_INTR_EVENT1_G3                     19
#define OFFSET_SGPIO_ERR_INTR_EVENT1_G4                     20
#define OFFSET_SGPIO_ERR_INTR_EVENT1_G5                     21
#define OFFSET_SGPIO_ERR_INTR_EVENT1_G6                     22
#define OFFSET_SGPIO_ERR_INTR_EVENT1_G7                     23
#define OFFSET_SGPIO_ERR_INTR_EVENT1_H0                     24
#define OFFSET_SGPIO_ERR_INTR_EVENT1_H1                     25
#define OFFSET_SGPIO_ERR_INTR_EVENT1_H2                     26
#define OFFSET_SGPIO_ERR_INTR_EVENT1_H3                     27
#define OFFSET_SGPIO_ERR_INTR_EVENT1_H4                     28
#define OFFSET_SGPIO_ERR_INTR_EVENT1_H5                     29
#define OFFSET_SGPIO_ERR_INTR_EVENT1_H6                     30
#define OFFSET_SGPIO_ERR_INTR_EVENT1_H7                     31

/*
 * Pin position within the corresonding 32 bit register
 */
#define OFFSET_SGPIO_MAN_INTR_EVENT0_A0                    0
#define OFFSET_SGPIO_MAN_INTR_EVENT0_A1                    1
#define OFFSET_SGPIO_MAN_INTR_EVENT0_A2                    2
#define OFFSET_SGPIO_MAN_INTR_EVENT0_A3                    3
#define OFFSET_SGPIO_MAN_INTR_EVENT0_A4                    4
#define OFFSET_SGPIO_MAN_INTR_EVENT0_A5                    5
#define OFFSET_SGPIO_MAN_INTR_EVENT0_A6                    6
#define OFFSET_SGPIO_MAN_INTR_EVENT0_A7                    7
#define OFFSET_SGPIO_MAN_INTR_EVENT0_B0                    8
#define OFFSET_SGPIO_MAN_INTR_EVENT0_B1                    9
#define OFFSET_SGPIO_MAN_INTR_EVENT0_B2                    10
#define OFFSET_SGPIO_MAN_INTR_EVENT0_B3                    11
#define OFFSET_SGPIO_MAN_INTR_EVENT0_B4                    12
#define OFFSET_SGPIO_MAN_INTR_EVENT0_B5                    13
#define OFFSET_SGPIO_MAN_INTR_EVENT0_B6                    14
#define OFFSET_SGPIO_MAN_INTR_EVENT0_B7                    15
#define OFFSET_SGPIO_MAN_INTR_EVENT0_C0                    16
#define OFFSET_SGPIO_MAN_INTR_EVENT0_C1                    17
#define OFFSET_SGPIO_MAN_INTR_EVENT0_C2                    18
#define OFFSET_SGPIO_MAN_INTR_EVENT0_C3                    19
#define OFFSET_SGPIO_MAN_INTR_EVENT0_C4                    20
#define OFFSET_SGPIO_MAN_INTR_EVENT0_C5                    21
#define OFFSET_SGPIO_MAN_INTR_EVENT0_C6                    22
#define OFFSET_SGPIO_MAN_INTR_EVENT0_C7                    23
#define OFFSET_SGPIO_MAN_INTR_EVENT0_D0                    24
#define OFFSET_SGPIO_MAN_INTR_EVENT0_D1                    25
#define OFFSET_SGPIO_MAN_INTR_EVENT0_D2                    26
#define OFFSET_SGPIO_MAN_INTR_EVENT0_D3                    27
#define OFFSET_SGPIO_MAN_INTR_EVENT0_D4                    28
#define OFFSET_SGPIO_MAN_INTR_EVENT0_D5                    29
#define OFFSET_SGPIO_MAN_INTR_EVENT0_D6                    30
#define OFFSET_SGPIO_MAN_INTR_EVENT0_D7                    31

/*
 * Pin position within the corresonding 32 bit register
 */
#define OFFSET_SGPIO_MAN_INTR_EVENT1_E0                    0
#define OFFSET_SGPIO_MAN_INTR_EVENT1_E1                    1
#define OFFSET_SGPIO_MAN_INTR_EVENT1_E2                    2
#define OFFSET_SGPIO_MAN_INTR_EVENT1_E3                    3
#define OFFSET_SGPIO_MAN_INTR_EVENT1_E4                    4
#define OFFSET_SGPIO_MAN_INTR_EVENT1_E5                    5
#define OFFSET_SGPIO_MAN_INTR_EVENT1_E6                    6
#define OFFSET_SGPIO_MAN_INTR_EVENT1_E7                    7
#define OFFSET_SGPIO_MAN_INTR_EVENT1_F0                    8
#define OFFSET_SGPIO_MAN_INTR_EVENT1_F1                    9
#define OFFSET_SGPIO_MAN_INTR_EVENT1_F2                    10
#define OFFSET_SGPIO_MAN_INTR_EVENT1_F3                    11
#define OFFSET_SGPIO_MAN_INTR_EVENT1_F4                    12
#define OFFSET_SGPIO_MAN_INTR_EVENT1_F5                    13
#define OFFSET_SGPIO_MAN_INTR_EVENT1_F6                    14
#define OFFSET_SGPIO_MAN_INTR_EVENT1_F7                    15
#define OFFSET_SGPIO_MAN_INTR_EVENT1_G0                    16
#define OFFSET_SGPIO_MAN_INTR_EVENT1_G1                    17
#define OFFSET_SGPIO_MAN_INTR_EVENT1_G2                    18
#define OFFSET_SGPIO_MAN_INTR_EVENT1_G3                    19
#define OFFSET_SGPIO_MAN_INTR_EVENT1_G4                    20
#define OFFSET_SGPIO_MAN_INTR_EVENT1_G5                    21
#define OFFSET_SGPIO_MAN_INTR_EVENT1_G6                    22
#define OFFSET_SGPIO_MAN_INTR_EVENT1_G7                    23
#define OFFSET_SGPIO_MAN_INTR_EVENT1_H0                    24
#define OFFSET_SGPIO_MAN_INTR_EVENT1_H1                    25
#define OFFSET_SGPIO_MAN_INTR_EVENT1_H2                    26
#define OFFSET_SGPIO_MAN_INTR_EVENT1_H3                    27
#define OFFSET_SGPIO_MAN_INTR_EVENT1_H4                    28
#define OFFSET_SGPIO_MAN_INTR_EVENT1_H5                    29
#define OFFSET_SGPIO_MAN_INTR_EVENT1_H6                    30
#define OFFSET_SGPIO_MAN_INTR_EVENT1_H7                    31

/*
 * Pin position within the corresonding 32 bit register
 */
#define OFFSET_SGPIO_NETWORK_INTR_EVENT0_A0                 0
#define OFFSET_SGPIO_NETWORK_INTR_EVENT0_A1                 1
#define OFFSET_SGPIO_NETWORK_INTR_EVENT0_A2                 2
#define OFFSET_SGPIO_NETWORK_INTR_EVENT0_A3                 3
#define OFFSET_SGPIO_NETWORK_INTR_EVENT0_A4                 4
#define OFFSET_SGPIO_NETWORK_INTR_EVENT0_A5                 5
#define OFFSET_SGPIO_NETWORK_INTR_EVENT0_A6                 6
#define OFFSET_SGPIO_NETWORK_INTR_EVENT0_A7                 7
#define OFFSET_SGPIO_NETWORK_INTR_EVENT0_B0                 8
#define OFFSET_SGPIO_NETWORK_INTR_EVENT0_B1                 9
#define OFFSET_SGPIO_NETWORK_INTR_EVENT0_B2                 10
#define OFFSET_SGPIO_NETWORK_INTR_EVENT0_B3                 11
#define OFFSET_SGPIO_NETWORK_INTR_EVENT0_B4                 12
#define OFFSET_SGPIO_NETWORK_INTR_EVENT0_B5                 13
#define OFFSET_SGPIO_NETWORK_INTR_EVENT0_B6                 14
#define OFFSET_SGPIO_NETWORK_INTR_EVENT0_B7                 15
#define OFFSET_SGPIO_NETWORK_INTR_EVENT0_C0                 16
#define OFFSET_SGPIO_NETWORK_INTR_EVENT0_C1                 17
#define OFFSET_SGPIO_NETWORK_INTR_EVENT0_C2                 18
#define OFFSET_SGPIO_NETWORK_INTR_EVENT0_C3                 19
#define OFFSET_SGPIO_NETWORK_INTR_EVENT0_C4                 20
#define OFFSET_SGPIO_NETWORK_INTR_EVENT0_C5                 21
#define OFFSET_SGPIO_NETWORK_INTR_EVENT0_C6                 22
#define OFFSET_SGPIO_NETWORK_INTR_EVENT0_C7                 23
#define OFFSET_SGPIO_NETWORK_INTR_EVENT0_D0                 24
#define OFFSET_SGPIO_NETWORK_INTR_EVENT0_D1                 25
#define OFFSET_SGPIO_NETWORK_INTR_EVENT0_D2                 26
#define OFFSET_SGPIO_NETWORK_INTR_EVENT0_D3                 27
#define OFFSET_SGPIO_NETWORK_INTR_EVENT0_D4                 28
#define OFFSET_SGPIO_NETWORK_INTR_EVENT0_D5                 29
#define OFFSET_SGPIO_NETWORK_INTR_EVENT0_D6                 30
#define OFFSET_SGPIO_NETWORK_INTR_EVENT0_D7                 31

/*
 * Pin position within the corresonding 32 bit register
 */
#define OFFSET_SGPIO_NETWORK_INTR_EVENT1_E0                 0
#define OFFSET_SGPIO_NETWORK_INTR_EVENT1_E1                 1
#define OFFSET_SGPIO_NETWORK_INTR_EVENT1_E2                 2
#define OFFSET_SGPIO_NETWORK_INTR_EVENT1_E3                 3
#define OFFSET_SGPIO_NETWORK_INTR_EVENT1_E4                 4
#define OFFSET_SGPIO_NETWORK_INTR_EVENT1_E5                 5
#define OFFSET_SGPIO_NETWORK_INTR_EVENT1_E6                 6
#define OFFSET_SGPIO_NETWORK_INTR_EVENT1_E7                 7
#define OFFSET_SGPIO_NETWORK_INTR_EVENT1_F0                 8
#define OFFSET_SGPIO_NETWORK_INTR_EVENT1_F1                 9
#define OFFSET_SGPIO_NETWORK_INTR_EVENT1_F2                 10
#define OFFSET_SGPIO_NETWORK_INTR_EVENT1_F3                 11
#define OFFSET_SGPIO_NETWORK_INTR_EVENT1_F4                 12
#define OFFSET_SGPIO_NETWORK_INTR_EVENT1_F5                 13
#define OFFSET_SGPIO_NETWORK_INTR_EVENT1_F6                 14
#define OFFSET_SGPIO_NETWORK_INTR_EVENT1_F7                 15
#define OFFSET_SGPIO_NETWORK_INTR_EVENT1_G0                 16
#define OFFSET_SGPIO_NETWORK_INTR_EVENT1_G1                 17
#define OFFSET_SGPIO_NETWORK_INTR_EVENT1_G2                 18
#define OFFSET_SGPIO_NETWORK_INTR_EVENT1_G3                 19
#define OFFSET_SGPIO_NETWORK_INTR_EVENT1_G4                 20
#define OFFSET_SGPIO_NETWORK_INTR_EVENT1_G5                 21
#define OFFSET_SGPIO_NETWORK_INTR_EVENT1_G6                 22
#define OFFSET_SGPIO_NETWORK_INTR_EVENT1_G7                 23
#define OFFSET_SGPIO_NETWORK_INTR_EVENT1_H0                 24
#define OFFSET_SGPIO_NETWORK_INTR_EVENT1_H1                 25
#define OFFSET_SGPIO_NETWORK_INTR_EVENT1_H2                 26
#define OFFSET_SGPIO_NETWORK_INTR_EVENT1_H3                 27
#define OFFSET_SGPIO_NETWORK_INTR_EVENT1_H4                 28
#define OFFSET_SGPIO_NETWORK_INTR_EVENT1_H5                 29
#define OFFSET_SGPIO_NETWORK_INTR_EVENT1_H6                 30
#define OFFSET_SGPIO_NETWORK_INTR_EVENT1_H7                 31

/*
 * Pin position within the corresonding 32 bit register
 */
#define OFFSET_SGPIO_ERR_INTR_EN0_A0                        0
#define OFFSET_SGPIO_ERR_INTR_EN0_A1                        1
#define OFFSET_SGPIO_ERR_INTR_EN0_A2                        2
#define OFFSET_SGPIO_ERR_INTR_EN0_A3                        3
#define OFFSET_SGPIO_ERR_INTR_EN0_A4                        4
#define OFFSET_SGPIO_ERR_INTR_EN0_A5                        5
#define OFFSET_SGPIO_ERR_INTR_EN0_A6                        6
#define OFFSET_SGPIO_ERR_INTR_EN0_A7                        7
#define OFFSET_SGPIO_ERR_INTR_EN0_B0                        8
#define OFFSET_SGPIO_ERR_INTR_EN0_B1                        9
#define OFFSET_SGPIO_ERR_INTR_EN0_B2                        10
#define OFFSET_SGPIO_ERR_INTR_EN0_B3                        11
#define OFFSET_SGPIO_ERR_INTR_EN0_B4                        12
#define OFFSET_SGPIO_ERR_INTR_EN0_B5                        13
#define OFFSET_SGPIO_ERR_INTR_EN0_B6                        14
#define OFFSET_SGPIO_ERR_INTR_EN0_B7                        15
#define OFFSET_SGPIO_ERR_INTR_EN0_C0                        16
#define OFFSET_SGPIO_ERR_INTR_EN0_C1                        17
#define OFFSET_SGPIO_ERR_INTR_EN0_C2                        18
#define OFFSET_SGPIO_ERR_INTR_EN0_C3                        19
#define OFFSET_SGPIO_ERR_INTR_EN0_C4                        20
#define OFFSET_SGPIO_ERR_INTR_EN0_C5                        21
#define OFFSET_SGPIO_ERR_INTR_EN0_C6                        22
#define OFFSET_SGPIO_ERR_INTR_EN0_C7                        23
#define OFFSET_SGPIO_ERR_INTR_EN0_D0                        24
#define OFFSET_SGPIO_ERR_INTR_EN0_D1                        25
#define OFFSET_SGPIO_ERR_INTR_EN0_D2                        26
#define OFFSET_SGPIO_ERR_INTR_EN0_D3                        27
#define OFFSET_SGPIO_ERR_INTR_EN0_D4                        28
#define OFFSET_SGPIO_ERR_INTR_EN0_D5                        29
#define OFFSET_SGPIO_ERR_INTR_EN0_D6                        30
#define OFFSET_SGPIO_ERR_INTR_EN0_D7                        31

/*
 * Pin position within the corresonding 32 bit register
 */
#define OFFSET_SGPIO_ERR_INTR_EN1_E0                        0
#define OFFSET_SGPIO_ERR_INTR_EN1_E1                        1
#define OFFSET_SGPIO_ERR_INTR_EN1_E2                        2
#define OFFSET_SGPIO_ERR_INTR_EN1_E3                        3
#define OFFSET_SGPIO_ERR_INTR_EN1_E4                        4
#define OFFSET_SGPIO_ERR_INTR_EN1_E5                        5
#define OFFSET_SGPIO_ERR_INTR_EN1_E6                        6
#define OFFSET_SGPIO_ERR_INTR_EN1_E7                        7
#define OFFSET_SGPIO_ERR_INTR_EN1_F0                        8
#define OFFSET_SGPIO_ERR_INTR_EN1_F1                        9
#define OFFSET_SGPIO_ERR_INTR_EN1_F2                        10
#define OFFSET_SGPIO_ERR_INTR_EN1_F3                        11
#define OFFSET_SGPIO_ERR_INTR_EN1_F4                        12
#define OFFSET_SGPIO_ERR_INTR_EN1_F5                        13
#define OFFSET_SGPIO_ERR_INTR_EN1_F6                        14
#define OFFSET_SGPIO_ERR_INTR_EN1_F7                        15
#define OFFSET_SGPIO_ERR_INTR_EN1_G0                        16
#define OFFSET_SGPIO_ERR_INTR_EN1_G1                        17
#define OFFSET_SGPIO_ERR_INTR_EN1_G2                        18
#define OFFSET_SGPIO_ERR_INTR_EN1_G3                        19
#define OFFSET_SGPIO_ERR_INTR_EN1_G4                        20
#define OFFSET_SGPIO_ERR_INTR_EN1_G5                        21
#define OFFSET_SGPIO_ERR_INTR_EN1_G6                        22
#define OFFSET_SGPIO_ERR_INTR_EN1_G7                        23
#define OFFSET_SGPIO_ERR_INTR_EN1_H0                        24
#define OFFSET_SGPIO_ERR_INTR_EN1_H1                        25
#define OFFSET_SGPIO_ERR_INTR_EN1_H2                        26
#define OFFSET_SGPIO_ERR_INTR_EN1_H3                        27
#define OFFSET_SGPIO_ERR_INTR_EN1_H4                        28
#define OFFSET_SGPIO_ERR_INTR_EN1_H5                        29
#define OFFSET_SGPIO_ERR_INTR_EN1_H6                        30
#define OFFSET_SGPIO_ERR_INTR_EN1_H7                        31

/*
 * Pin position within the corresonding 32 bit register
 */
#define OFFSET_SGPIO_MAN_INTR_EN0_A0                       0
#define OFFSET_SGPIO_MAN_INTR_EN0_A1                       1
#define OFFSET_SGPIO_MAN_INTR_EN0_A2                       2
#define OFFSET_SGPIO_MAN_INTR_EN0_A3                       3
#define OFFSET_SGPIO_MAN_INTR_EN0_A4                       4
#define OFFSET_SGPIO_MAN_INTR_EN0_A5                       5
#define OFFSET_SGPIO_MAN_INTR_EN0_A6                       6
#define OFFSET_SGPIO_MAN_INTR_EN0_A7                       7
#define OFFSET_SGPIO_MAN_INTR_EN0_B0                       8
#define OFFSET_SGPIO_MAN_INTR_EN0_B1                       9
#define OFFSET_SGPIO_MAN_INTR_EN0_B2                       10
#define OFFSET_SGPIO_MAN_INTR_EN0_B3                       11
#define OFFSET_SGPIO_MAN_INTR_EN0_B4                       12
#define OFFSET_SGPIO_MAN_INTR_EN0_B5                       13
#define OFFSET_SGPIO_MAN_INTR_EN0_B6                       14
#define OFFSET_SGPIO_MAN_INTR_EN0_B7                       15
#define OFFSET_SGPIO_MAN_INTR_EN0_C0                       16
#define OFFSET_SGPIO_MAN_INTR_EN0_C1                       17
#define OFFSET_SGPIO_MAN_INTR_EN0_C2                       18
#define OFFSET_SGPIO_MAN_INTR_EN0_C3                       19
#define OFFSET_SGPIO_MAN_INTR_EN0_C4                       20
#define OFFSET_SGPIO_MAN_INTR_EN0_C5                       21
#define OFFSET_SGPIO_MAN_INTR_EN0_C6                       22
#define OFFSET_SGPIO_MAN_INTR_EN0_C7                       23
#define OFFSET_SGPIO_MAN_INTR_EN0_D0                       24
#define OFFSET_SGPIO_MAN_INTR_EN0_D1                       25
#define OFFSET_SGPIO_MAN_INTR_EN0_D2                       26
#define OFFSET_SGPIO_MAN_INTR_EN0_D3                       27
#define OFFSET_SGPIO_MAN_INTR_EN0_D4                       28
#define OFFSET_SGPIO_MAN_INTR_EN0_D5                       29
#define OFFSET_SGPIO_MAN_INTR_EN0_D6                       30
#define OFFSET_SGPIO_MAN_INTR_EN0_D7                       31

/*
 * Pin position within the corresonding 32 bit register
 */
#define OFFSET_SGPIO_MAN_INTR_EN1_E0                       0
#define OFFSET_SGPIO_MAN_INTR_EN1_E1                       1
#define OFFSET_SGPIO_MAN_INTR_EN1_E2                       2
#define OFFSET_SGPIO_MAN_INTR_EN1_E3                       3
#define OFFSET_SGPIO_MAN_INTR_EN1_E4                       4
#define OFFSET_SGPIO_MAN_INTR_EN1_E5                       5
#define OFFSET_SGPIO_MAN_INTR_EN1_E6                       6
#define OFFSET_SGPIO_MAN_INTR_EN1_E7                       7
#define OFFSET_SGPIO_MAN_INTR_EN1_F0                       8
#define OFFSET_SGPIO_MAN_INTR_EN1_F1                       9
#define OFFSET_SGPIO_MAN_INTR_EN1_F2                       10
#define OFFSET_SGPIO_MAN_INTR_EN1_F3                       11
#define OFFSET_SGPIO_MAN_INTR_EN1_F4                       12
#define OFFSET_SGPIO_MAN_INTR_EN1_F5                       13
#define OFFSET_SGPIO_MAN_INTR_EN1_F6                       14
#define OFFSET_SGPIO_MAN_INTR_EN1_F7                       15
#define OFFSET_SGPIO_MAN_INTR_EN1_G0                       16
#define OFFSET_SGPIO_MAN_INTR_EN1_G1                       17
#define OFFSET_SGPIO_MAN_INTR_EN1_G2                       18
#define OFFSET_SGPIO_MAN_INTR_EN1_G3                       19
#define OFFSET_SGPIO_MAN_INTR_EN1_G4                       20
#define OFFSET_SGPIO_MAN_INTR_EN1_G5                       21
#define OFFSET_SGPIO_MAN_INTR_EN1_G6                       22
#define OFFSET_SGPIO_MAN_INTR_EN1_G7                       23
#define OFFSET_SGPIO_MAN_INTR_EN1_H0                       24
#define OFFSET_SGPIO_MAN_INTR_EN1_H1                       25
#define OFFSET_SGPIO_MAN_INTR_EN1_H2                       26
#define OFFSET_SGPIO_MAN_INTR_EN1_H3                       27
#define OFFSET_SGPIO_MAN_INTR_EN1_H4                       28
#define OFFSET_SGPIO_MAN_INTR_EN1_H5                       29
#define OFFSET_SGPIO_MAN_INTR_EN1_H6                       30
#define OFFSET_SGPIO_MAN_INTR_EN1_H7                       31

/*
 * Pin position within the corresonding 32 bit register
 */
#define OFFSET_SGPIO_NETWORK_INTR_EN0_A0                    0
#define OFFSET_SGPIO_NETWORK_INTR_EN0_A1                    1
#define OFFSET_SGPIO_NETWORK_INTR_EN0_A2                    2
#define OFFSET_SGPIO_NETWORK_INTR_EN0_A3                    3
#define OFFSET_SGPIO_NETWORK_INTR_EN0_A4                    4
#define OFFSET_SGPIO_NETWORK_INTR_EN0_A5                    5
#define OFFSET_SGPIO_NETWORK_INTR_EN0_A6                    6
#define OFFSET_SGPIO_NETWORK_INTR_EN0_A7                    7
#define OFFSET_SGPIO_NETWORK_INTR_EN0_B0                    8
#define OFFSET_SGPIO_NETWORK_INTR_EN0_B1                    9
#define OFFSET_SGPIO_NETWORK_INTR_EN0_B2                    10
#define OFFSET_SGPIO_NETWORK_INTR_EN0_B3                    11
#define OFFSET_SGPIO_NETWORK_INTR_EN0_B4                    12
#define OFFSET_SGPIO_NETWORK_INTR_EN0_B5                    13
#define OFFSET_SGPIO_NETWORK_INTR_EN0_B6                    14
#define OFFSET_SGPIO_NETWORK_INTR_EN0_B7                    15
#define OFFSET_SGPIO_NETWORK_INTR_EN0_C0                    16
#define OFFSET_SGPIO_NETWORK_INTR_EN0_C1                    17
#define OFFSET_SGPIO_NETWORK_INTR_EN0_C2                    18
#define OFFSET_SGPIO_NETWORK_INTR_EN0_C3                    19
#define OFFSET_SGPIO_NETWORK_INTR_EN0_C4                    20
#define OFFSET_SGPIO_NETWORK_INTR_EN0_C5                    21
#define OFFSET_SGPIO_NETWORK_INTR_EN0_C6                    22
#define OFFSET_SGPIO_NETWORK_INTR_EN0_C7                    23
#define OFFSET_SGPIO_NETWORK_INTR_EN0_D0                    24
#define OFFSET_SGPIO_NETWORK_INTR_EN0_D1                    25
#define OFFSET_SGPIO_NETWORK_INTR_EN0_D2                    26
#define OFFSET_SGPIO_NETWORK_INTR_EN0_D3                    27
#define OFFSET_SGPIO_NETWORK_INTR_EN0_D4                    28
#define OFFSET_SGPIO_NETWORK_INTR_EN0_D5                    29
#define OFFSET_SGPIO_NETWORK_INTR_EN0_D6                    30
#define OFFSET_SGPIO_NETWORK_INTR_EN0_D7                    31

/*
 * Pin position within the corresonding 32 bit register
 */
#define OFFSET_SGPIO_NETWORK_INTR_EN1_E0                    0
#define OFFSET_SGPIO_NETWORK_INTR_EN1_E1                    1
#define OFFSET_SGPIO_NETWORK_INTR_EN1_E2                    2
#define OFFSET_SGPIO_NETWORK_INTR_EN1_E3                    3
#define OFFSET_SGPIO_NETWORK_INTR_EN1_E4                    4
#define OFFSET_SGPIO_NETWORK_INTR_EN1_E5                    5
#define OFFSET_SGPIO_NETWORK_INTR_EN1_E6                    6
#define OFFSET_SGPIO_NETWORK_INTR_EN1_E7                    7
#define OFFSET_SGPIO_NETWORK_INTR_EN1_F0                    8
#define OFFSET_SGPIO_NETWORK_INTR_EN1_F1                    9
#define OFFSET_SGPIO_NETWORK_INTR_EN1_F2                    10
#define OFFSET_SGPIO_NETWORK_INTR_EN1_F3                    11
#define OFFSET_SGPIO_NETWORK_INTR_EN1_F4                    12
#define OFFSET_SGPIO_NETWORK_INTR_EN1_F5                    13
#define OFFSET_SGPIO_NETWORK_INTR_EN1_F6                    14
#define OFFSET_SGPIO_NETWORK_INTR_EN1_F7                    15
#define OFFSET_SGPIO_NETWORK_INTR_EN1_G0                    16
#define OFFSET_SGPIO_NETWORK_INTR_EN1_G1                    17
#define OFFSET_SGPIO_NETWORK_INTR_EN1_G2                    18
#define OFFSET_SGPIO_NETWORK_INTR_EN1_G3                    19
#define OFFSET_SGPIO_NETWORK_INTR_EN1_G4                    20
#define OFFSET_SGPIO_NETWORK_INTR_EN1_G5                    21
#define OFFSET_SGPIO_NETWORK_INTR_EN1_G6                    22
#define OFFSET_SGPIO_NETWORK_INTR_EN1_G7                    23
#define OFFSET_SGPIO_NETWORK_INTR_EN1_H0                    24
#define OFFSET_SGPIO_NETWORK_INTR_EN1_H1                    25
#define OFFSET_SGPIO_NETWORK_INTR_EN1_H2                    26
#define OFFSET_SGPIO_NETWORK_INTR_EN1_H3                    27
#define OFFSET_SGPIO_NETWORK_INTR_EN1_H4                    28
#define OFFSET_SGPIO_NETWORK_INTR_EN1_H5                    29
#define OFFSET_SGPIO_NETWORK_INTR_EN1_H6                    30
#define OFFSET_SGPIO_NETWORK_INTR_EN1_H7                    31

/*
 * Pin position within the corresonding 32 bit register
 */
#define OFFSET_SGPIO_OUTPUT_POLARITY_CTRL0_A0               0
#define OFFSET_SGPIO_OUTPUT_POLARITY_CTRL0_A1               1
#define OFFSET_SGPIO_OUTPUT_POLARITY_CTRL0_A2               2
#define OFFSET_SGPIO_OUTPUT_POLARITY_CTRL0_A3               3
#define OFFSET_SGPIO_OUTPUT_POLARITY_CTRL0_A4               4
#define OFFSET_SGPIO_OUTPUT_POLARITY_CTRL0_A5               5
#define OFFSET_SGPIO_OUTPUT_POLARITY_CTRL0_A6               6
#define OFFSET_SGPIO_OUTPUT_POLARITY_CTRL0_A7               7
#define OFFSET_SGPIO_OUTPUT_POLARITY_CTRL0_B0               8
#define OFFSET_SGPIO_OUTPUT_POLARITY_CTRL0_B1               9
#define OFFSET_SGPIO_OUTPUT_POLARITY_CTRL0_B2               10
#define OFFSET_SGPIO_OUTPUT_POLARITY_CTRL0_B3               11
#define OFFSET_SGPIO_OUTPUT_POLARITY_CTRL0_B4               12
#define OFFSET_SGPIO_OUTPUT_POLARITY_CTRL0_B5               13
#define OFFSET_SGPIO_OUTPUT_POLARITY_CTRL0_B6               14
#define OFFSET_SGPIO_OUTPUT_POLARITY_CTRL0_B7               15
#define OFFSET_SGPIO_OUTPUT_POLARITY_CTRL0_C0               16
#define OFFSET_SGPIO_OUTPUT_POLARITY_CTRL0_C1               17
#define OFFSET_SGPIO_OUTPUT_POLARITY_CTRL0_C2               18
#define OFFSET_SGPIO_OUTPUT_POLARITY_CTRL0_C3               19
#define OFFSET_SGPIO_OUTPUT_POLARITY_CTRL0_C4               20
#define OFFSET_SGPIO_OUTPUT_POLARITY_CTRL0_C5               21
#define OFFSET_SGPIO_OUTPUT_POLARITY_CTRL0_C6               22
#define OFFSET_SGPIO_OUTPUT_POLARITY_CTRL0_C7               23
#define OFFSET_SGPIO_OUTPUT_POLARITY_CTRL0_D0               24
#define OFFSET_SGPIO_OUTPUT_POLARITY_CTRL0_D1               25
#define OFFSET_SGPIO_OUTPUT_POLARITY_CTRL0_D2               26
#define OFFSET_SGPIO_OUTPUT_POLARITY_CTRL0_D3               27
#define OFFSET_SGPIO_OUTPUT_POLARITY_CTRL0_D4               28
#define OFFSET_SGPIO_OUTPUT_POLARITY_CTRL0_D5               29
#define OFFSET_SGPIO_OUTPUT_POLARITY_CTRL0_D6               30
#define OFFSET_SGPIO_OUTPUT_POLARITY_CTRL0_D7               31

/*
 * Pin position within the corresonding 32 bit register
 */
#define OFFSET_SGPIO_OUTPUT_POLARITY_CTRL1_E0               0
#define OFFSET_SGPIO_OUTPUT_POLARITY_CTRL1_E1               1
#define OFFSET_SGPIO_OUTPUT_POLARITY_CTRL1_E2               2
#define OFFSET_SGPIO_OUTPUT_POLARITY_CTRL1_E3               3
#define OFFSET_SGPIO_OUTPUT_POLARITY_CTRL1_E4               4
#define OFFSET_SGPIO_OUTPUT_POLARITY_CTRL1_E5               5
#define OFFSET_SGPIO_OUTPUT_POLARITY_CTRL1_E6               6
#define OFFSET_SGPIO_OUTPUT_POLARITY_CTRL1_E7               7
#define OFFSET_SGPIO_OUTPUT_POLARITY_CTRL1_F0               8
#define OFFSET_SGPIO_OUTPUT_POLARITY_CTRL1_F1               9
#define OFFSET_SGPIO_OUTPUT_POLARITY_CTRL1_F2               10
#define OFFSET_SGPIO_OUTPUT_POLARITY_CTRL1_F3               11
#define OFFSET_SGPIO_OUTPUT_POLARITY_CTRL1_F4               12
#define OFFSET_SGPIO_OUTPUT_POLARITY_CTRL1_F5               13
#define OFFSET_SGPIO_OUTPUT_POLARITY_CTRL1_F6               14
#define OFFSET_SGPIO_OUTPUT_POLARITY_CTRL1_F7               15
#define OFFSET_SGPIO_OUTPUT_POLARITY_CTRL1_G0               16
#define OFFSET_SGPIO_OUTPUT_POLARITY_CTRL1_G1               17
#define OFFSET_SGPIO_OUTPUT_POLARITY_CTRL1_G2               18
#define OFFSET_SGPIO_OUTPUT_POLARITY_CTRL1_G3               19
#define OFFSET_SGPIO_OUTPUT_POLARITY_CTRL1_G4               20
#define OFFSET_SGPIO_OUTPUT_POLARITY_CTRL1_G5               21
#define OFFSET_SGPIO_OUTPUT_POLARITY_CTRL1_G6               22
#define OFFSET_SGPIO_OUTPUT_POLARITY_CTRL1_G7               23
#define OFFSET_SGPIO_OUTPUT_POLARITY_CTRL1_H0               24
#define OFFSET_SGPIO_OUTPUT_POLARITY_CTRL1_H1               25
#define OFFSET_SGPIO_OUTPUT_POLARITY_CTRL1_H2               26
#define OFFSET_SGPIO_OUTPUT_POLARITY_CTRL1_H3               27
#define OFFSET_SGPIO_OUTPUT_POLARITY_CTRL1_H4               28
#define OFFSET_SGPIO_OUTPUT_POLARITY_CTRL1_H5               29
#define OFFSET_SGPIO_OUTPUT_POLARITY_CTRL1_H6               30
#define OFFSET_SGPIO_OUTPUT_POLARITY_CTRL1_H7               31

/*
 * Pin position within the corresonding 32 bit register
 */
#define OFFSET_SGPIO_INPUT_POLARITY_CTRL0_A0                0
#define OFFSET_SGPIO_INPUT_POLARITY_CTRL0_A1                1
#define OFFSET_SGPIO_INPUT_POLARITY_CTRL0_A2                2
#define OFFSET_SGPIO_INPUT_POLARITY_CTRL0_A3                3
#define OFFSET_SGPIO_INPUT_POLARITY_CTRL0_A4                4
#define OFFSET_SGPIO_INPUT_POLARITY_CTRL0_A5                5
#define OFFSET_SGPIO_INPUT_POLARITY_CTRL0_A6                6
#define OFFSET_SGPIO_INPUT_POLARITY_CTRL0_A7                7
#define OFFSET_SGPIO_INPUT_POLARITY_CTRL0_B0                8
#define OFFSET_SGPIO_INPUT_POLARITY_CTRL0_B1                9
#define OFFSET_SGPIO_INPUT_POLARITY_CTRL0_B2                10
#define OFFSET_SGPIO_INPUT_POLARITY_CTRL0_B3                11
#define OFFSET_SGPIO_INPUT_POLARITY_CTRL0_B4                12
#define OFFSET_SGPIO_INPUT_POLARITY_CTRL0_B5                13
#define OFFSET_SGPIO_INPUT_POLARITY_CTRL0_B6                14
#define OFFSET_SGPIO_INPUT_POLARITY_CTRL0_B7                15
#define OFFSET_SGPIO_INPUT_POLARITY_CTRL0_C0                16
#define OFFSET_SGPIO_INPUT_POLARITY_CTRL0_C1                17
#define OFFSET_SGPIO_INPUT_POLARITY_CTRL0_C2                18
#define OFFSET_SGPIO_INPUT_POLARITY_CTRL0_C3                19
#define OFFSET_SGPIO_INPUT_POLARITY_CTRL0_C4                20
#define OFFSET_SGPIO_INPUT_POLARITY_CTRL0_C5                21
#define OFFSET_SGPIO_INPUT_POLARITY_CTRL0_C6                22
#define OFFSET_SGPIO_INPUT_POLARITY_CTRL0_C7                23
#define OFFSET_SGPIO_INPUT_POLARITY_CTRL0_D0                24
#define OFFSET_SGPIO_INPUT_POLARITY_CTRL0_D1                25
#define OFFSET_SGPIO_INPUT_POLARITY_CTRL0_D2                26
#define OFFSET_SGPIO_INPUT_POLARITY_CTRL0_D3                27
#define OFFSET_SGPIO_INPUT_POLARITY_CTRL0_D4                28
#define OFFSET_SGPIO_INPUT_POLARITY_CTRL0_D5                29
#define OFFSET_SGPIO_INPUT_POLARITY_CTRL0_D6                30
#define OFFSET_SGPIO_INPUT_POLARITY_CTRL0_D7                31

/*
 * Pin position within the corresonding 32 bit register
 */
#define OFFSET_SGPIO_INPUT_POLARITY_CTRL1_E0                0
#define OFFSET_SGPIO_INPUT_POLARITY_CTRL1_E1                1
#define OFFSET_SGPIO_INPUT_POLARITY_CTRL1_E2                2
#define OFFSET_SGPIO_INPUT_POLARITY_CTRL1_E3                3
#define OFFSET_SGPIO_INPUT_POLARITY_CTRL1_E4                4
#define OFFSET_SGPIO_INPUT_POLARITY_CTRL1_E5                5
#define OFFSET_SGPIO_INPUT_POLARITY_CTRL1_E6                6
#define OFFSET_SGPIO_INPUT_POLARITY_CTRL1_E7                7
#define OFFSET_SGPIO_INPUT_POLARITY_CTRL1_F0                8
#define OFFSET_SGPIO_INPUT_POLARITY_CTRL1_F1                9
#define OFFSET_SGPIO_INPUT_POLARITY_CTRL1_F2                10
#define OFFSET_SGPIO_INPUT_POLARITY_CTRL1_F3                11
#define OFFSET_SGPIO_INPUT_POLARITY_CTRL1_F4                12
#define OFFSET_SGPIO_INPUT_POLARITY_CTRL1_F5                13
#define OFFSET_SGPIO_INPUT_POLARITY_CTRL1_F6                14
#define OFFSET_SGPIO_INPUT_POLARITY_CTRL1_F7                15
#define OFFSET_SGPIO_INPUT_POLARITY_CTRL1_G0                16
#define OFFSET_SGPIO_INPUT_POLARITY_CTRL1_G1                17
#define OFFSET_SGPIO_INPUT_POLARITY_CTRL1_G2                18
#define OFFSET_SGPIO_INPUT_POLARITY_CTRL1_G3                19
#define OFFSET_SGPIO_INPUT_POLARITY_CTRL1_G4                20
#define OFFSET_SGPIO_INPUT_POLARITY_CTRL1_G5                21
#define OFFSET_SGPIO_INPUT_POLARITY_CTRL1_G6                22
#define OFFSET_SGPIO_INPUT_POLARITY_CTRL1_G7                23
#define OFFSET_SGPIO_INPUT_POLARITY_CTRL1_H0                24
#define OFFSET_SGPIO_INPUT_POLARITY_CTRL1_H1                25
#define OFFSET_SGPIO_INPUT_POLARITY_CTRL1_H2                26
#define OFFSET_SGPIO_INPUT_POLARITY_CTRL1_H3                27
#define OFFSET_SGPIO_INPUT_POLARITY_CTRL1_H4                28
#define OFFSET_SGPIO_INPUT_POLARITY_CTRL1_H5                29
#define OFFSET_SGPIO_INPUT_POLARITY_CTRL1_H6                30
#define OFFSET_SGPIO_INPUT_POLARITY_CTRL1_H7                31

/*********  field masks **********************/
#define MASK_GPIO_PIN0                        0x00000001
#define MASK_GPIO_PIN1                        0x00000002
#define MASK_GPIO_PIN2                        0x00000004
#define MASK_GPIO_PIN3                        0x00000008
#define MASK_GPIO_PIN4                        0x00000010
#define MASK_GPIO_PIN5                        0x00000020
#define MASK_GPIO_PIN6                        0x00000040
#define MASK_GPIO_PIN7                        0x00000080
#define MASK_GPIO_PIN8                        0x00000100
#define MASK_GPIO_PIN9                        0x00000200
#define MASK_GPIO_PIN10                       0x00000400
#define MASK_GPIO_PIN11                       0x00000800
#define MASK_GPIO_PIN12                       0x00001000
#define MASK_GPIO_PIN13                       0x00002000
#define MASK_GPIO_PIN14                       0x00004000
#define MASK_GPIO_PIN15                       0x00008000
#define MASK_GPIO_PIN16                       0x00010000
#define MASK_GPIO_PIN17                       0x00020000
#define MASK_GPIO_PIN18                       0x00040000
#define MASK_GPIO_PIN19                       0x00080000
#define MASK_GPIO_PIN20                       0x00100000
#define MASK_GPIO_PIN21                       0x00200000
#define MASK_GPIO_PIN22                       0x00400000
#define MASK_GPIO_PIN23                       0x00800000
#define MASK_GPIO_PIN24                       0x01000000
#define MASK_GPIO_PIN25                       0x02000000
#define MASK_GPIO_PIN26                       0x04000000
#define MASK_GPIO_PIN27                       0x08000000
#define MASK_GPIO_PIN28                       0x10000000
#define MASK_GPIO_PIN29                       0x20000000
#define MASK_GPIO_PIN30                       0x40000000
#define MASK_GPIO_PIN31                       0x80000000

#define MASK_GPIO_PIN32                       0x00000001
#define MASK_GPIO_PIN33                       0x00000002
#define MASK_GPIO_PIN34                       0x00000004
#define MASK_GPIO_PIN35                       0x00000008
#define MASK_GPIO_PIN36                       0x00000010
#define MASK_GPIO_PIN37                       0x00000020
#define MASK_GPIO_PIN38                       0x00000040
#define MASK_GPIO_PIN39                       0x00000080
#define MASK_GPIO_PIN40                       0x00000100
#define MASK_GPIO_PIN41                       0x00000200
#define MASK_GPIO_PIN42                       0x00000400
#define MASK_GPIO_PIN43                       0x00000800
#define MASK_GPIO_PIN44                       0x00001000
#define MASK_GPIO_PIN45                       0x00002000
#define MASK_GPIO_PIN46                       0x00004000
#define MASK_GPIO_PIN47                       0x00008000

#define GPIO_IN_OUT_PIN_PER_REG                             16
#define GPIO_IN_OUT_BIT_PER_PIN                             2
#define MASK_GPIO_IN_OUT_CFG0_PIN0                          0x00000003
#define MASK_GPIO_IN_OUT_CFG0_PIN1                          0x0000000c
#define MASK_GPIO_IN_OUT_CFG0_PIN2                          0x00000030
#define MASK_GPIO_IN_OUT_CFG0_PIN3                          0x000000c0
#define MASK_GPIO_IN_OUT_CFG0_PIN4                          0x00000300
#define MASK_GPIO_IN_OUT_CFG0_PIN5                          0x00000c00
#define MASK_GPIO_IN_OUT_CFG0_PIN6                          0x00003000
#define MASK_GPIO_IN_OUT_CFG0_PIN7                          0x0000c000
#define MASK_GPIO_IN_OUT_CFG0_PIN8                          0x00030000
#define MASK_GPIO_IN_OUT_CFG0_PIN9                          0x000c0000
#define MASK_GPIO_IN_OUT_CFG0_PIN10                         0x00300000
#define MASK_GPIO_IN_OUT_CFG0_PIN11                         0x00c00000
#define MASK_GPIO_IN_OUT_CFG0_PIN12                         0x03000000
#define MASK_GPIO_IN_OUT_CFG0_PIN13                         0x0c000000
#define MASK_GPIO_IN_OUT_CFG0_PIN14                         0x30000000
#define MASK_GPIO_IN_OUT_CFG0_PIN15                         0xc0000000

#define MASK_GPIO_IN_OUT_CFG1_PIN16                         0x00000003
#define MASK_GPIO_IN_OUT_CFG1_PIN17                         0x0000000c
#define MASK_GPIO_IN_OUT_CFG1_PIN18                         0x00000030
#define MASK_GPIO_IN_OUT_CFG1_PIN19                         0x000000c0
#define MASK_GPIO_IN_OUT_CFG1_PIN20                         0x00000300
#define MASK_GPIO_IN_OUT_CFG1_PIN21                         0x00000c00
#define MASK_GPIO_IN_OUT_CFG1_PIN22                         0x00003000
#define MASK_GPIO_IN_OUT_CFG1_PIN23                         0x0000c000
#define MASK_GPIO_IN_OUT_CFG1_PIN24                         0x00030000
#define MASK_GPIO_IN_OUT_CFG1_PIN25                         0x000c0000
#define MASK_GPIO_IN_OUT_CFG1_PIN26                         0x00300000
#define MASK_GPIO_IN_OUT_CFG1_PIN27                         0x00c00000
#define MASK_GPIO_IN_OUT_CFG1_PIN28                         0x03000000
#define MASK_GPIO_IN_OUT_CFG1_PIN29                         0x0c000000
#define MASK_GPIO_IN_OUT_CFG1_PIN30                         0x30000000
#define MASK_GPIO_IN_OUT_CFG1_PIN31                         0xc0000000

#define MASK_GPIO_IN_OUT_CFG2_PIN32                         0x00000003
#define MASK_GPIO_IN_OUT_CFG2_PIN33                         0x0000000c
#define MASK_GPIO_IN_OUT_CFG2_PIN34                         0x00000030
#define MASK_GPIO_IN_OUT_CFG2_PIN35                         0x000000c0
#define MASK_GPIO_IN_OUT_CFG2_PIN36                         0x00000300
#define MASK_GPIO_IN_OUT_CFG2_PIN37                         0x00000c00
#define MASK_GPIO_IN_OUT_CFG2_PIN38                         0x00003000
#define MASK_GPIO_IN_OUT_CFG2_PIN39                         0x0000c000
#define MASK_GPIO_IN_OUT_CFG2_PIN40                         0x00030000
#define MASK_GPIO_IN_OUT_CFG2_PIN41                         0x000c0000
#define MASK_GPIO_IN_OUT_CFG2_PIN42                         0x00300000
#define MASK_GPIO_IN_OUT_CFG2_PIN43                         0x00c00000
#define MASK_GPIO_IN_OUT_CFG2_PIN44                         0x03000000
#define MASK_GPIO_IN_OUT_CFG2_PIN45                         0x0c000000
#define MASK_GPIO_IN_OUT_CFG2_PIN46                         0x30000000
#define MASK_GPIO_IN_OUT_CFG2_PIN47                         0xc0000000

#define GPIO_POL_PIN_PER_REG                            32
#define GPIO_POL_BIT_PER_PIN                            1
#define MASK_GPIO_POL_CTRL0_PIN0                        0x00000001
#define MASK_GPIO_POL_CTRL0_PIN1                        0x00000002
#define MASK_GPIO_POL_CTRL0_PIN2                        0x00000004
#define MASK_GPIO_POL_CTRL0_PIN3                        0x00000008
#define MASK_GPIO_POL_CTRL0_PIN4                        0x00000010
#define MASK_GPIO_POL_CTRL0_PIN5                        0x00000020
#define MASK_GPIO_POL_CTRL0_PIN6                        0x00000040
#define MASK_GPIO_POL_CTRL0_PIN7                        0x00000080
#define MASK_GPIO_POL_CTRL0_PIN8                        0x00000100
#define MASK_GPIO_POL_CTRL0_PIN9                        0x00000200
#define MASK_GPIO_POL_CTRL0_PIN10                       0x00000400
#define MASK_GPIO_POL_CTRL0_PIN11                       0x00000800
#define MASK_GPIO_POL_CTRL0_PIN12                       0x00001000
#define MASK_GPIO_POL_CTRL0_PIN13                       0x00002000
#define MASK_GPIO_POL_CTRL0_PIN14                       0x00004000
#define MASK_GPIO_POL_CTRL0_PIN15                       0x00008000
#define MASK_GPIO_POL_CTRL0_PIN16                       0x00010000
#define MASK_GPIO_POL_CTRL0_PIN17                       0x00020000
#define MASK_GPIO_POL_CTRL0_PIN18                       0x00040000
#define MASK_GPIO_POL_CTRL0_PIN19                       0x00080000
#define MASK_GPIO_POL_CTRL0_PIN20                       0x00100000
#define MASK_GPIO_POL_CTRL0_PIN21                       0x00200000
#define MASK_GPIO_POL_CTRL0_PIN22                       0x00400000
#define MASK_GPIO_POL_CTRL0_PIN23                       0x00800000
#define MASK_GPIO_POL_CTRL0_PIN24                       0x01000000
#define MASK_GPIO_POL_CTRL0_PIN25                       0x02000000
#define MASK_GPIO_POL_CTRL0_PIN26                       0x04000000
#define MASK_GPIO_POL_CTRL0_PIN27                       0x08000000
#define MASK_GPIO_POL_CTRL0_PIN28                       0x10000000
#define MASK_GPIO_POL_CTRL0_PIN29                       0x20000000
#define MASK_GPIO_POL_CTRL0_PIN30                       0x40000000
#define MASK_GPIO_POL_CTRL0_PIN31                       0x80000000

#define MASK_GPIO_POL_CTRL1_PIN32                       0x00000001
#define MASK_GPIO_POL_CTRL1_PIN33                       0x00000002
#define MASK_GPIO_POL_CTRL1_PIN34                       0x00000004
#define MASK_GPIO_POL_CTRL1_PIN35                       0x00000008
#define MASK_GPIO_POL_CTRL1_PIN36                       0x00000010
#define MASK_GPIO_POL_CTRL1_PIN37                       0x00000020
#define MASK_GPIO_POL_CTRL1_PIN38                       0x00000040
#define MASK_GPIO_POL_CTRL1_PIN39                       0x00000080
#define MASK_GPIO_POL_CTRL1_PIN40                       0x00000100
#define MASK_GPIO_POL_CTRL1_PIN41                       0x00000200
#define MASK_GPIO_POL_CTRL1_PIN42                       0x00000400
#define MASK_GPIO_POL_CTRL1_PIN43                       0x00000800
#define MASK_GPIO_POL_CTRL1_PIN44                       0x00001000
#define MASK_GPIO_POL_CTRL1_PIN45                       0x00002000
#define MASK_GPIO_POL_CTRL1_PIN46                       0x00004000
#define MASK_GPIO_POL_CTRL1_PIN47                       0x00008000

#define GPIO_DEBOUNCE_PIN_PER_REG                            32
#define GPIO_DEBOUNCE_BIT_PER_PIN                            1
#define MASK_GPIO_DEBOUNCE_CTRL0_PIN0                        0x00000001
#define MASK_GPIO_DEBOUNCE_CTRL0_PIN1                        0x00000002
#define MASK_GPIO_DEBOUNCE_CTRL0_PIN2                        0x00000004
#define MASK_GPIO_DEBOUNCE_CTRL0_PIN3                        0x00000008
#define MASK_GPIO_DEBOUNCE_CTRL0_PIN4                        0x00000010
#define MASK_GPIO_DEBOUNCE_CTRL0_PIN5                        0x00000020
#define MASK_GPIO_DEBOUNCE_CTRL0_PIN6                        0x00000040
#define MASK_GPIO_DEBOUNCE_CTRL0_PIN7                        0x00000080
#define MASK_GPIO_DEBOUNCE_CTRL0_PIN8                        0x00000100
#define MASK_GPIO_DEBOUNCE_CTRL0_PIN9                        0x00000200
#define MASK_GPIO_DEBOUNCE_CTRL0_PIN10                       0x00000400
#define MASK_GPIO_DEBOUNCE_CTRL0_PIN11                       0x00000800
#define MASK_GPIO_DEBOUNCE_CTRL0_PIN12                       0x00001000
#define MASK_GPIO_DEBOUNCE_CTRL0_PIN13                       0x00002000
#define MASK_GPIO_DEBOUNCE_CTRL0_PIN14                       0x00004000
#define MASK_GPIO_DEBOUNCE_CTRL0_PIN15                       0x00008000
#define MASK_GPIO_DEBOUNCE_CTRL0_PIN16                       0x00010000
#define MASK_GPIO_DEBOUNCE_CTRL0_PIN17                       0x00020000
#define MASK_GPIO_DEBOUNCE_CTRL0_PIN18                       0x00040000
#define MASK_GPIO_DEBOUNCE_CTRL0_PIN19                       0x00080000
#define MASK_GPIO_DEBOUNCE_CTRL0_PIN20                       0x00100000
#define MASK_GPIO_DEBOUNCE_CTRL0_PIN21                       0x00200000
#define MASK_GPIO_DEBOUNCE_CTRL0_PIN22                       0x00400000
#define MASK_GPIO_DEBOUNCE_CTRL0_PIN23                       0x00800000
#define MASK_GPIO_DEBOUNCE_CTRL0_PIN24                       0x01000000
#define MASK_GPIO_DEBOUNCE_CTRL0_PIN25                       0x02000000
#define MASK_GPIO_DEBOUNCE_CTRL0_PIN26                       0x04000000
#define MASK_GPIO_DEBOUNCE_CTRL0_PIN27                       0x08000000
#define MASK_GPIO_DEBOUNCE_CTRL0_PIN28                       0x10000000
#define MASK_GPIO_DEBOUNCE_CTRL0_PIN29                       0x20000000
#define MASK_GPIO_DEBOUNCE_CTRL0_PIN30                       0x40000000
#define MASK_GPIO_DEBOUNCE_CTRL0_PIN31                       0x80000000

#define MASK_GPIO_DEBOUNCE_CTRL1_PIN32                       0x00000001
#define MASK_GPIO_DEBOUNCE_CTRL1_PIN33                       0x00000002
#define MASK_GPIO_DEBOUNCE_CTRL1_PIN34                       0x00000004
#define MASK_GPIO_DEBOUNCE_CTRL1_PIN35                       0x00000008
#define MASK_GPIO_DEBOUNCE_CTRL1_PIN36                       0x00000010
#define MASK_GPIO_DEBOUNCE_CTRL1_PIN37                       0x00000020
#define MASK_GPIO_DEBOUNCE_CTRL1_PIN38                       0x00000040
#define MASK_GPIO_DEBOUNCE_CTRL1_PIN39                       0x00000080
#define MASK_GPIO_DEBOUNCE_CTRL1_PIN40                       0x00000100
#define MASK_GPIO_DEBOUNCE_CTRL1_PIN41                       0x00000200
#define MASK_GPIO_DEBOUNCE_CTRL1_PIN42                       0x00000400
#define MASK_GPIO_DEBOUNCE_CTRL1_PIN43                       0x00000800
#define MASK_GPIO_DEBOUNCE_CTRL1_PIN44                       0x00001000
#define MASK_GPIO_DEBOUNCE_CTRL1_PIN45                       0x00002000
#define MASK_GPIO_DEBOUNCE_CTRL1_PIN46                       0x00004000
#define MASK_GPIO_DEBOUNCE_CTRL1_PIN47                       0x00008000

#define MASK_GPIO_DEBOUNCE_TIME_PIN0                        0x0000000f
#define GPIO_DB_TIME_PIN_PER_REG                            8
#define GPIO_DB_TIME_BIT_PER_PIN                            4

#define GPIO_INT_TYPE_PIN_PER_REG                         16
#define GPIO_INT_TYPE_BIT_PER_PIN                         2
#define MASK_GPIO_INTR_TYPE0_PIN0                           0x00000003
#define MASK_GPIO_INTR_TYPE0_PIN1                           0x0000000c
#define MASK_GPIO_INTR_TYPE0_PIN2                           0x00000030
#define MASK_GPIO_INTR_TYPE0_PIN3                           0x000000c0
#define MASK_GPIO_INTR_TYPE0_PIN4                           0x00000300
#define MASK_GPIO_INTR_TYPE0_PIN5                           0x00000c00
#define MASK_GPIO_INTR_TYPE0_PIN6                           0x00003000
#define MASK_GPIO_INTR_TYPE0_PIN7                           0x0000c000
#define MASK_GPIO_INTR_TYPE0_PIN8                           0x00030000
#define MASK_GPIO_INTR_TYPE0_PIN9                           0x000c0000
#define MASK_GPIO_INTR_TYPE0_PIN10                          0x00300000
#define MASK_GPIO_INTR_TYPE0_PIN11                          0x00c00000
#define MASK_GPIO_INTR_TYPE0_PIN12                          0x03000000
#define MASK_GPIO_INTR_TYPE0_PIN13                          0x0c000000
#define MASK_GPIO_INTR_TYPE0_PIN14                          0x30000000
#define MASK_GPIO_INTR_TYPE0_PIN15                          0xc0000000

#define MASK_GPIO_INTR_TYPE1_PIN16                          0x00000003
#define MASK_GPIO_INTR_TYPE1_PIN17                          0x0000000c
#define MASK_GPIO_INTR_TYPE1_PIN18                          0x00000030
#define MASK_GPIO_INTR_TYPE1_PIN19                          0x000000c0
#define MASK_GPIO_INTR_TYPE1_PIN20                          0x00000300
#define MASK_GPIO_INTR_TYPE1_PIN21                          0x00000c00
#define MASK_GPIO_INTR_TYPE1_PIN22                          0x00003000
#define MASK_GPIO_INTR_TYPE1_PIN23                          0x0000c000
#define MASK_GPIO_INTR_TYPE1_PIN24                          0x00030000
#define MASK_GPIO_INTR_TYPE1_PIN25                          0x000c0000
#define MASK_GPIO_INTR_TYPE1_PIN26                          0x00300000
#define MASK_GPIO_INTR_TYPE1_PIN27                          0x00c00000
#define MASK_GPIO_INTR_TYPE1_PIN28                          0x03000000
#define MASK_GPIO_INTR_TYPE1_PIN29                          0x0c000000
#define MASK_GPIO_INTR_TYPE1_PIN30                          0x30000000
#define MASK_GPIO_INTR_TYPE1_PIN31                          0xc0000000

#define MASK_GPIO_INTR_TYPE2_PIN32                          0x00000003
#define MASK_GPIO_INTR_TYPE2_PIN33                          0x0000000c
#define MASK_GPIO_INTR_TYPE2_PIN34                          0x00000030
#define MASK_GPIO_INTR_TYPE2_PIN35                          0x000000c0
#define MASK_GPIO_INTR_TYPE2_PIN36                          0x00000300
#define MASK_GPIO_INTR_TYPE2_PIN37                          0x00000c00
#define MASK_GPIO_INTR_TYPE2_PIN38                          0x00003000
#define MASK_GPIO_INTR_TYPE2_PIN39                          0x0000c000
#define MASK_GPIO_INTR_TYPE2_PIN40                          0x00030000
#define MASK_GPIO_INTR_TYPE2_PIN41                          0x000c0000
#define MASK_GPIO_INTR_TYPE2_PIN42                          0x00300000
#define MASK_GPIO_INTR_TYPE2_PIN43                          0x00c00000
#define MASK_GPIO_INTR_TYPE2_PIN44                          0x03000000
#define MASK_GPIO_INTR_TYPE2_PIN45                          0x0c000000
#define MASK_GPIO_INTR_TYPE2_PIN46                          0x30000000
#define MASK_GPIO_INTR_TYPE2_PIN47                          0xc0000000

#define MASK_SGPIO_OUTPUT0_A0                               0x00000001
#define MASK_SGPIO_OUTPUT0_A1                               0x00000002
#define MASK_SGPIO_OUTPUT0_A2                               0x00000004
#define MASK_SGPIO_OUTPUT0_A3                               0x00000008
#define MASK_SGPIO_OUTPUT0_A4                               0x00000010
#define MASK_SGPIO_OUTPUT0_A5                               0x00000020
#define MASK_SGPIO_OUTPUT0_A6                               0x00000040
#define MASK_SGPIO_OUTPUT0_A7                               0x00000080
#define MASK_SGPIO_OUTPUT0_B0                               0x00000100
#define MASK_SGPIO_OUTPUT0_B1                               0x00000200
#define MASK_SGPIO_OUTPUT0_B2                               0x00000400
#define MASK_SGPIO_OUTPUT0_B3                               0x00000800
#define MASK_SGPIO_OUTPUT0_B4                               0x00001000
#define MASK_SGPIO_OUTPUT0_B5                               0x00002000
#define MASK_SGPIO_OUTPUT0_B6                               0x00004000
#define MASK_SGPIO_OUTPUT0_B7                               0x00008000
#define MASK_SGPIO_OUTPUT0_C0                               0x00010000
#define MASK_SGPIO_OUTPUT0_C1                               0x00020000
#define MASK_SGPIO_OUTPUT0_C2                               0x00040000
#define MASK_SGPIO_OUTPUT0_C3                               0x00080000
#define MASK_SGPIO_OUTPUT0_C4                               0x00100000
#define MASK_SGPIO_OUTPUT0_C5                               0x00200000
#define MASK_SGPIO_OUTPUT0_C6                               0x00400000
#define MASK_SGPIO_OUTPUT0_C7                               0x00800000
#define MASK_SGPIO_OUTPUT0_D0                               0x01000000
#define MASK_SGPIO_OUTPUT0_D1                               0x02000000
#define MASK_SGPIO_OUTPUT0_D2                               0x04000000
#define MASK_SGPIO_OUTPUT0_D3                               0x08000000
#define MASK_SGPIO_OUTPUT0_D4                               0x10000000
#define MASK_SGPIO_OUTPUT0_D5                               0x20000000
#define MASK_SGPIO_OUTPUT0_D6                               0x40000000
#define MASK_SGPIO_OUTPUT0_D7                               0x80000000

#define MASK_SGPIO_OUTPUT1_E0                               0x00000001
#define MASK_SGPIO_OUTPUT1_E1                               0x00000002
#define MASK_SGPIO_OUTPUT1_E2                               0x00000004
#define MASK_SGPIO_OUTPUT1_E3                               0x00000008
#define MASK_SGPIO_OUTPUT1_E4                               0x00000010
#define MASK_SGPIO_OUTPUT1_E5                               0x00000020
#define MASK_SGPIO_OUTPUT1_E6                               0x00000040
#define MASK_SGPIO_OUTPUT1_E7                               0x00000080
#define MASK_SGPIO_OUTPUT1_F0                               0x00000100
#define MASK_SGPIO_OUTPUT1_F1                               0x00000200
#define MASK_SGPIO_OUTPUT1_F2                               0x00000400
#define MASK_SGPIO_OUTPUT1_F3                               0x00000800
#define MASK_SGPIO_OUTPUT1_F4                               0x00001000
#define MASK_SGPIO_OUTPUT1_F5                               0x00002000
#define MASK_SGPIO_OUTPUT1_F6                               0x00004000
#define MASK_SGPIO_OUTPUT1_F7                               0x00008000
#define MASK_SGPIO_OUTPUT1_G0                               0x00010000
#define MASK_SGPIO_OUTPUT1_G1                               0x00020000
#define MASK_SGPIO_OUTPUT1_G2                               0x00040000
#define MASK_SGPIO_OUTPUT1_G3                               0x00080000
#define MASK_SGPIO_OUTPUT1_G4                               0x00100000
#define MASK_SGPIO_OUTPUT1_G5                               0x00200000
#define MASK_SGPIO_OUTPUT1_G6                               0x00400000
#define MASK_SGPIO_OUTPUT1_G7                               0x00800000
#define MASK_SGPIO_OUTPUT1_H0                               0x01000000
#define MASK_SGPIO_OUTPUT1_H1                               0x02000000
#define MASK_SGPIO_OUTPUT1_H2                               0x04000000
#define MASK_SGPIO_OUTPUT1_H3                               0x08000000
#define MASK_SGPIO_OUTPUT1_H4                               0x10000000
#define MASK_SGPIO_OUTPUT1_H5                               0x20000000
#define MASK_SGPIO_OUTPUT1_H6                               0x40000000
#define MASK_SGPIO_OUTPUT1_H7                               0x80000000

#define MASK_SGPIO_INPUT1_E0                                0x00000001
#define MASK_SGPIO_INPUT1_E1                                0x00000002
#define MASK_SGPIO_INPUT1_E2                                0x00000004
#define MASK_SGPIO_INPUT1_E3                                0x00000008
#define MASK_SGPIO_INPUT1_E4                                0x00000010
#define MASK_SGPIO_INPUT1_E5                                0x00000020
#define MASK_SGPIO_INPUT1_E6                                0x00000040
#define MASK_SGPIO_INPUT1_E7                                0x00000080
#define MASK_SGPIO_INPUT1_F0                                0x00000100
#define MASK_SGPIO_INPUT1_F1                                0x00000200
#define MASK_SGPIO_INPUT1_F2                                0x00000400
#define MASK_SGPIO_INPUT1_F3                                0x00000800
#define MASK_SGPIO_INPUT1_F4                                0x00001000
#define MASK_SGPIO_INPUT1_F5                                0x00002000
#define MASK_SGPIO_INPUT1_F6                                0x00004000
#define MASK_SGPIO_INPUT1_F7                                0x00008000
#define MASK_SGPIO_INPUT1_G0                                0x00010000
#define MASK_SGPIO_INPUT1_G1                                0x00020000
#define MASK_SGPIO_INPUT1_G2                                0x00040000
#define MASK_SGPIO_INPUT1_G3                                0x00080000
#define MASK_SGPIO_INPUT1_G4                                0x00100000
#define MASK_SGPIO_INPUT1_G5                                0x00200000
#define MASK_SGPIO_INPUT1_G6                                0x00400000
#define MASK_SGPIO_INPUT1_G7                                0x00800000
#define MASK_SGPIO_INPUT1_H0                                0x01000000
#define MASK_SGPIO_INPUT1_H1                                0x02000000
#define MASK_SGPIO_INPUT1_H2                                0x04000000
#define MASK_SGPIO_INPUT1_H3                                0x08000000
#define MASK_SGPIO_INPUT1_H4                                0x10000000
#define MASK_SGPIO_INPUT1_H5                                0x20000000
#define MASK_SGPIO_INPUT1_H6                                0x40000000
#define MASK_SGPIO_INPUT1_H7                                0x80000000

#define MASK_SGPIO_INPUT0_A0                                0x00000001
#define MASK_SGPIO_INPUT0_A1                                0x00000002
#define MASK_SGPIO_INPUT0_A2                                0x00000004
#define MASK_SGPIO_INPUT0_A3                                0x00000008
#define MASK_SGPIO_INPUT0_A4                                0x00000010
#define MASK_SGPIO_INPUT0_A5                                0x00000020
#define MASK_SGPIO_INPUT0_A6                                0x00000040
#define MASK_SGPIO_INPUT0_A7                                0x00000080
#define MASK_SGPIO_INPUT0_B0                                0x00000100
#define MASK_SGPIO_INPUT0_B1                                0x00000200
#define MASK_SGPIO_INPUT0_B2                                0x00000400
#define MASK_SGPIO_INPUT0_B3                                0x00000800
#define MASK_SGPIO_INPUT0_B4                                0x00001000
#define MASK_SGPIO_INPUT0_B5                                0x00002000
#define MASK_SGPIO_INPUT0_B6                                0x00004000
#define MASK_SGPIO_INPUT0_B7                                0x00008000
#define MASK_SGPIO_INPUT0_C0                                0x00010000
#define MASK_SGPIO_INPUT0_C1                                0x00020000
#define MASK_SGPIO_INPUT0_C2                                0x00040000
#define MASK_SGPIO_INPUT0_C3                                0x00080000
#define MASK_SGPIO_INPUT0_C4                                0x00100000
#define MASK_SGPIO_INPUT0_C5                                0x00200000
#define MASK_SGPIO_INPUT0_C6                                0x00400000
#define MASK_SGPIO_INPUT0_C7                                0x00800000
#define MASK_SGPIO_INPUT0_D0                                0x01000000
#define MASK_SGPIO_INPUT0_D1                                0x02000000
#define MASK_SGPIO_INPUT0_D2                                0x04000000
#define MASK_SGPIO_INPUT0_D3                                0x08000000
#define MASK_SGPIO_INPUT0_D4                                0x10000000
#define MASK_SGPIO_INPUT0_D5                                0x20000000
#define MASK_SGPIO_INPUT0_D6                                0x40000000
#define MASK_SGPIO_INPUT0_D7                                0x80000000

#define MASK_SGPIO_SHIFT_REG_CFG_SPEED                      0x000000ff
#define MASK_SGPIO_SHIFT_REG_CFG_SIZE                       0x00000100
#define MASK_SGPIO_SHIFT_REG_CFG_VALID                      0x00008000
#define SGPIO_SHIFT_REG_CFG_VALID                           0x00008000
#define SGPIO_SHIFT_REG_CFG_SIZE_64                         0x00000100
#define SGPIO_SHIFT_REG_CFG_SIZE_32                         0x00000000
#define SGPIO_SHIFT_REG_SPEED_DEFAULT                       0x00000028

#define MASK_SGPIO_DEBOUNCE_CTRL0_A0                        0x00000001
#define MASK_SGPIO_DEBOUNCE_CTRL0_A1                        0x00000002
#define MASK_SGPIO_DEBOUNCE_CTRL0_A2                        0x00000004
#define MASK_SGPIO_DEBOUNCE_CTRL0_A3                        0x00000008
#define MASK_SGPIO_DEBOUNCE_CTRL0_A4                        0x00000010
#define MASK_SGPIO_DEBOUNCE_CTRL0_A5                        0x00000020
#define MASK_SGPIO_DEBOUNCE_CTRL0_A6                        0x00000040
#define MASK_SGPIO_DEBOUNCE_CTRL0_A7                        0x00000080
#define MASK_SGPIO_DEBOUNCE_CTRL0_B0                        0x00000100
#define MASK_SGPIO_DEBOUNCE_CTRL0_B1                        0x00000200
#define MASK_SGPIO_DEBOUNCE_CTRL0_B2                        0x00000400
#define MASK_SGPIO_DEBOUNCE_CTRL0_B3                        0x00000800
#define MASK_SGPIO_DEBOUNCE_CTRL0_B4                        0x00001000
#define MASK_SGPIO_DEBOUNCE_CTRL0_B5                        0x00002000
#define MASK_SGPIO_DEBOUNCE_CTRL0_B6                        0x00004000
#define MASK_SGPIO_DEBOUNCE_CTRL0_B7                        0x00008000
#define MASK_SGPIO_DEBOUNCE_CTRL0_C0                        0x00010000
#define MASK_SGPIO_DEBOUNCE_CTRL0_C1                        0x00020000
#define MASK_SGPIO_DEBOUNCE_CTRL0_C2                        0x00040000
#define MASK_SGPIO_DEBOUNCE_CTRL0_C3                        0x00080000
#define MASK_SGPIO_DEBOUNCE_CTRL0_C4                        0x00100000
#define MASK_SGPIO_DEBOUNCE_CTRL0_C5                        0x00200000
#define MASK_SGPIO_DEBOUNCE_CTRL0_C6                        0x00400000
#define MASK_SGPIO_DEBOUNCE_CTRL0_C7                        0x00800000
#define MASK_SGPIO_DEBOUNCE_CTRL0_D0                        0x01000000
#define MASK_SGPIO_DEBOUNCE_CTRL0_D1                        0x02000000
#define MASK_SGPIO_DEBOUNCE_CTRL0_D2                        0x04000000
#define MASK_SGPIO_DEBOUNCE_CTRL0_D3                        0x08000000
#define MASK_SGPIO_DEBOUNCE_CTRL0_D4                        0x10000000
#define MASK_SGPIO_DEBOUNCE_CTRL0_D5                        0x20000000
#define MASK_SGPIO_DEBOUNCE_CTRL0_D6                        0x40000000
#define MASK_SGPIO_DEBOUNCE_CTRL0_D7                        0x80000000

#define MASK_SGPIO_DEBOUNCE_CTRL1_E0                        0x00000001
#define MASK_SGPIO_DEBOUNCE_CTRL1_E1                        0x00000002
#define MASK_SGPIO_DEBOUNCE_CTRL1_E2                        0x00000004
#define MASK_SGPIO_DEBOUNCE_CTRL1_E3                        0x00000008
#define MASK_SGPIO_DEBOUNCE_CTRL1_E4                        0x00000010
#define MASK_SGPIO_DEBOUNCE_CTRL1_E5                        0x00000020
#define MASK_SGPIO_DEBOUNCE_CTRL1_E6                        0x00000040
#define MASK_SGPIO_DEBOUNCE_CTRL1_E7                        0x00000080
#define MASK_SGPIO_DEBOUNCE_CTRL1_F0                        0x00000100
#define MASK_SGPIO_DEBOUNCE_CTRL1_F1                        0x00000200
#define MASK_SGPIO_DEBOUNCE_CTRL1_F2                        0x00000400
#define MASK_SGPIO_DEBOUNCE_CTRL1_F3                        0x00000800
#define MASK_SGPIO_DEBOUNCE_CTRL1_F4                        0x00001000
#define MASK_SGPIO_DEBOUNCE_CTRL1_F5                        0x00002000
#define MASK_SGPIO_DEBOUNCE_CTRL1_F6                        0x00004000
#define MASK_SGPIO_DEBOUNCE_CTRL1_F7                        0x00008000
#define MASK_SGPIO_DEBOUNCE_CTRL1_G0                        0x00010000
#define MASK_SGPIO_DEBOUNCE_CTRL1_G1                        0x00020000
#define MASK_SGPIO_DEBOUNCE_CTRL1_G2                        0x00040000
#define MASK_SGPIO_DEBOUNCE_CTRL1_G3                        0x00080000
#define MASK_SGPIO_DEBOUNCE_CTRL1_G4                        0x00100000
#define MASK_SGPIO_DEBOUNCE_CTRL1_G5                        0x00200000
#define MASK_SGPIO_DEBOUNCE_CTRL1_G6                        0x00400000
#define MASK_SGPIO_DEBOUNCE_CTRL1_G7                        0x00800000
#define MASK_SGPIO_DEBOUNCE_CTRL1_H0                        0x01000000
#define MASK_SGPIO_DEBOUNCE_CTRL1_H1                        0x02000000
#define MASK_SGPIO_DEBOUNCE_CTRL1_H2                        0x04000000
#define MASK_SGPIO_DEBOUNCE_CTRL1_H3                        0x08000000
#define MASK_SGPIO_DEBOUNCE_CTRL1_H4                        0x10000000
#define MASK_SGPIO_DEBOUNCE_CTRL1_H5                        0x20000000
#define MASK_SGPIO_DEBOUNCE_CTRL1_H6                        0x40000000
#define MASK_SGPIO_DEBOUNCE_CTRL1_H7                        0x80000000

#define MASK_SGPIO_FUNCTION_TYPE_FAN_A0                     0x00000001
#define MASK_SGPIO_FUNCTION_TYPE_FAN_A1                     0x00000002
#define MASK_SGPIO_FUNCTION_TYPE_FAN_A2                     0x00000004
#define MASK_SGPIO_FUNCTION_TYPE_FAN_A3                     0x00000008

#define MASK_SGPIO_INTR_TEST0_A0                            0x00000001
#define MASK_SGPIO_INTR_TEST0_A1                            0x00000002
#define MASK_SGPIO_INTR_TEST0_A2                            0x00000004
#define MASK_SGPIO_INTR_TEST0_A3                            0x00000008
#define MASK_SGPIO_INTR_TEST0_A4                            0x00000010
#define MASK_SGPIO_INTR_TEST0_A5                            0x00000020
#define MASK_SGPIO_INTR_TEST0_A6                            0x00000040
#define MASK_SGPIO_INTR_TEST0_A7                            0x00000080
#define MASK_SGPIO_INTR_TEST0_B0                            0x00000100
#define MASK_SGPIO_INTR_TEST0_B1                            0x00000200
#define MASK_SGPIO_INTR_TEST0_B2                            0x00000400
#define MASK_SGPIO_INTR_TEST0_B3                            0x00000800
#define MASK_SGPIO_INTR_TEST0_B4                            0x00001000
#define MASK_SGPIO_INTR_TEST0_B5                            0x00002000
#define MASK_SGPIO_INTR_TEST0_B6                            0x00004000
#define MASK_SGPIO_INTR_TEST0_B7                            0x00008000
#define MASK_SGPIO_INTR_TEST0_C0                            0x00010000
#define MASK_SGPIO_INTR_TEST0_C1                            0x00020000
#define MASK_SGPIO_INTR_TEST0_C2                            0x00040000
#define MASK_SGPIO_INTR_TEST0_C3                            0x00080000
#define MASK_SGPIO_INTR_TEST0_C4                            0x00100000
#define MASK_SGPIO_INTR_TEST0_C5                            0x00200000
#define MASK_SGPIO_INTR_TEST0_C6                            0x00400000
#define MASK_SGPIO_INTR_TEST0_C7                            0x00800000
#define MASK_SGPIO_INTR_TEST0_D0                            0x01000000
#define MASK_SGPIO_INTR_TEST0_D1                            0x02000000
#define MASK_SGPIO_INTR_TEST0_D2                            0x04000000
#define MASK_SGPIO_INTR_TEST0_D3                            0x08000000
#define MASK_SGPIO_INTR_TEST0_D4                            0x10000000
#define MASK_SGPIO_INTR_TEST0_D5                            0x20000000
#define MASK_SGPIO_INTR_TEST0_D6                            0x40000000
#define MASK_SGPIO_INTR_TEST0_D7                            0x80000000

#define MASK_SGPIO_INTR_TEST1_E0                            0x00000001
#define MASK_SGPIO_INTR_TEST1_E1                            0x00000002
#define MASK_SGPIO_INTR_TEST1_E2                            0x00000004
#define MASK_SGPIO_INTR_TEST1_E3                            0x00000008
#define MASK_SGPIO_INTR_TEST1_E4                            0x00000010
#define MASK_SGPIO_INTR_TEST1_E5                            0x00000020
#define MASK_SGPIO_INTR_TEST1_E6                            0x00000040
#define MASK_SGPIO_INTR_TEST1_E7                            0x00000080
#define MASK_SGPIO_INTR_TEST1_F0                            0x00000100
#define MASK_SGPIO_INTR_TEST1_F1                            0x00000200
#define MASK_SGPIO_INTR_TEST1_F2                            0x00000400
#define MASK_SGPIO_INTR_TEST1_F3                            0x00000800
#define MASK_SGPIO_INTR_TEST1_F4                            0x00001000
#define MASK_SGPIO_INTR_TEST1_F5                            0x00002000
#define MASK_SGPIO_INTR_TEST1_F6                            0x00004000
#define MASK_SGPIO_INTR_TEST1_F7                            0x00008000
#define MASK_SGPIO_INTR_TEST1_G0                            0x00010000
#define MASK_SGPIO_INTR_TEST1_G1                            0x00020000
#define MASK_SGPIO_INTR_TEST1_G2                            0x00040000
#define MASK_SGPIO_INTR_TEST1_G3                            0x00080000
#define MASK_SGPIO_INTR_TEST1_G4                            0x00100000
#define MASK_SGPIO_INTR_TEST1_G5                            0x00200000
#define MASK_SGPIO_INTR_TEST1_G6                            0x00400000
#define MASK_SGPIO_INTR_TEST1_G7                            0x00800000
#define MASK_SGPIO_INTR_TEST1_H0                            0x01000000
#define MASK_SGPIO_INTR_TEST1_H1                            0x02000000
#define MASK_SGPIO_INTR_TEST1_H2                            0x04000000
#define MASK_SGPIO_INTR_TEST1_H3                            0x08000000
#define MASK_SGPIO_INTR_TEST1_H4                            0x10000000
#define MASK_SGPIO_INTR_TEST1_H5                            0x20000000
#define MASK_SGPIO_INTR_TEST1_H6                            0x40000000
#define MASK_SGPIO_INTR_TEST1_H7                            0x80000000

#define MASK_SGPIO_FAN_TACH_MAX_LIMIT_1                     0x0000003f
#define MASK_SGPIO_FAN_TACH_MAX_LIMIT_2                     0x00003f00
#define MASK_SGPIO_FAN_TACH_MAX_LIMIT_3                     0x003f0000
#define MASK_SGPIO_FAN_TACH_MAX_LIMIT_4                     0x3f000000
#define SGPIO_FAN_TACH_DEFAULT                              0x08080808

#define MASK_SGPIO_ETH_LED_CFG_PAUSE                        0x0000000f
#define MASK_SGPIO_ETH_LED_CFG_OFF                          0x00000f00
#define MASK_SGPIO_ETH_LED_CFG_ON                           0x0000f000
#define SGPIO_ETH_LED_CFG_DEFAULT                           0x00003307

#define MASK_SGPIO_ETH_LED_EN_A0                            0x00000003
#define MASK_SGPIO_ETH_LED_EN_A1                            0x0000000c
#define MASK_SGPIO_ETH_LED_EN_A2                            0x00000030
#define MASK_SGPIO_ETH_LED_EN_A3                            0x000000c0
#define MASK_SGPIO_ETH_LED_EN_A4                            0x00000300
#define MASK_SGPIO_ETH_LED_EN_A5                            0x00000c00
#define MASK_SGPIO_ETH_LED_EN_A6                            0x00003000
#define MASK_SGPIO_ETH_LED_EN_A7                            0x0000c000
#define SGPIO_ETH_LED_1BLINK                                0x00000001
#define SGPIO_ETH_LED_2BLINK                                0x00000002
#define SGPIO_ETH_LED_3BLINK                                0x00000003
#define SGPIO_ETH_LED_BLINK_CTRL_BIT_WIDTH                  2

#define SGPIO_INTR_TYPE_DEFAULT                             0xffffffff //disable 
#define MASK_SGPIO_INTR_TYPE0_A0                            0x00000003
#define MASK_SGPIO_INTR_TYPE0_A1                            0x0000000c
#define MASK_SGPIO_INTR_TYPE0_A2                            0x00000030
#define MASK_SGPIO_INTR_TYPE0_A3                            0x000000c0
#define MASK_SGPIO_INTR_TYPE0_A4                            0x00000300
#define MASK_SGPIO_INTR_TYPE0_A5                            0x00000c00
#define MASK_SGPIO_INTR_TYPE0_A6                            0x00003000
#define MASK_SGPIO_INTR_TYPE0_A7                            0x0000c000
#define MASK_SGPIO_INTR_TYPE0_B0                            0x00030000
#define MASK_SGPIO_INTR_TYPE0_B1                            0x000c0000
#define MASK_SGPIO_INTR_TYPE0_B2                            0x00300000
#define MASK_SGPIO_INTR_TYPE0_B3                            0x00c00000
#define MASK_SGPIO_INTR_TYPE0_B4                            0x03000000
#define MASK_SGPIO_INTR_TYPE0_B5                            0x0c000000
#define MASK_SGPIO_INTR_TYPE0_B6                            0x30000000
#define MASK_SGPIO_INTR_TYPE0_B7                            0xc0000000

#define MASK_SGPIO_INTR_TYPE1_C0                            0x00000003
#define MASK_SGPIO_INTR_TYPE1_C1                            0x0000000c
#define MASK_SGPIO_INTR_TYPE1_C2                            0x00000030
#define MASK_SGPIO_INTR_TYPE1_C3                            0x000000c0
#define MASK_SGPIO_INTR_TYPE1_C4                            0x00000300
#define MASK_SGPIO_INTR_TYPE1_C5                            0x00000c00
#define MASK_SGPIO_INTR_TYPE1_C6                            0x00003000
#define MASK_SGPIO_INTR_TYPE1_C7                            0x0000c000
#define MASK_SGPIO_INTR_TYPE1_D0                            0x00030000
#define MASK_SGPIO_INTR_TYPE1_D1                            0x000c0000
#define MASK_SGPIO_INTR_TYPE1_D2                            0x00300000
#define MASK_SGPIO_INTR_TYPE1_D3                            0x00c00000
#define MASK_SGPIO_INTR_TYPE1_D4                            0x03000000
#define MASK_SGPIO_INTR_TYPE1_D5                            0x0c000000
#define MASK_SGPIO_INTR_TYPE1_D6                            0x30000000
#define MASK_SGPIO_INTR_TYPE1_D7                            0xc0000000

#define MASK_SGPIO_INTR_TYPE2_E0                            0x00000003
#define MASK_SGPIO_INTR_TYPE2_E1                            0x0000000c
#define MASK_SGPIO_INTR_TYPE2_E2                            0x00000030
#define MASK_SGPIO_INTR_TYPE2_E3                            0x000000c0
#define MASK_SGPIO_INTR_TYPE2_E4                            0x00000300
#define MASK_SGPIO_INTR_TYPE2_E5                            0x00000c00
#define MASK_SGPIO_INTR_TYPE2_E6                            0x00003000
#define MASK_SGPIO_INTR_TYPE2_E7                            0x0000c000
#define MASK_SGPIO_INTR_TYPE2_F0                            0x00030000
#define MASK_SGPIO_INTR_TYPE2_F1                            0x000c0000
#define MASK_SGPIO_INTR_TYPE2_F2                            0x00300000
#define MASK_SGPIO_INTR_TYPE2_F3                            0x00c00000
#define MASK_SGPIO_INTR_TYPE2_F4                            0x03000000
#define MASK_SGPIO_INTR_TYPE2_F5                            0x0c000000
#define MASK_SGPIO_INTR_TYPE2_F6                            0x30000000
#define MASK_SGPIO_INTR_TYPE2_F7                            0xc0000000

#define MASK_SGPIO_INTR_TYPE3_G0                            0x00000003
#define MASK_SGPIO_INTR_TYPE3_G1                            0x0000000c
#define MASK_SGPIO_INTR_TYPE3_G2                            0x00000030
#define MASK_SGPIO_INTR_TYPE3_G3                            0x000000c0
#define MASK_SGPIO_INTR_TYPE3_G4                            0x00000300
#define MASK_SGPIO_INTR_TYPE3_G5                            0x00000c00
#define MASK_SGPIO_INTR_TYPE3_G6                            0x00003000
#define MASK_SGPIO_INTR_TYPE3_G7                            0x0000c000
#define MASK_SGPIO_INTR_TYPE3_H0                            0x00030000
#define MASK_SGPIO_INTR_TYPE3_H1                            0x000c0000
#define MASK_SGPIO_INTR_TYPE3_H2                            0x00300000
#define MASK_SGPIO_INTR_TYPE3_H3                            0x00c00000
#define MASK_SGPIO_INTR_TYPE3_H4                            0x03000000
#define MASK_SGPIO_INTR_TYPE3_H5                            0x0c000000
#define MASK_SGPIO_INTR_TYPE3_H6                            0x30000000
#define MASK_SGPIO_INTR_TYPE3_H7                            0xc0000000

#define MASK_SGPIO_ERR_INTR_EVENT0_A0                       0x00000001
#define MASK_SGPIO_ERR_INTR_EVENT0_A1                       0x00000002
#define MASK_SGPIO_ERR_INTR_EVENT0_A2                       0x00000004
#define MASK_SGPIO_ERR_INTR_EVENT0_A3                       0x00000008
#define MASK_SGPIO_ERR_INTR_EVENT0_A4                       0x00000010
#define MASK_SGPIO_ERR_INTR_EVENT0_A5                       0x00000020
#define MASK_SGPIO_ERR_INTR_EVENT0_A6                       0x00000040
#define MASK_SGPIO_ERR_INTR_EVENT0_A7                       0x00000080
#define MASK_SGPIO_ERR_INTR_EVENT0_B0                       0x00000100
#define MASK_SGPIO_ERR_INTR_EVENT0_B1                       0x00000200
#define MASK_SGPIO_ERR_INTR_EVENT0_B2                       0x00000400
#define MASK_SGPIO_ERR_INTR_EVENT0_B3                       0x00000800
#define MASK_SGPIO_ERR_INTR_EVENT0_B4                       0x00001000
#define MASK_SGPIO_ERR_INTR_EVENT0_B5                       0x00002000
#define MASK_SGPIO_ERR_INTR_EVENT0_B6                       0x00004000
#define MASK_SGPIO_ERR_INTR_EVENT0_B7                       0x00008000
#define MASK_SGPIO_ERR_INTR_EVENT0_C0                       0x00010000
#define MASK_SGPIO_ERR_INTR_EVENT0_C1                       0x00020000
#define MASK_SGPIO_ERR_INTR_EVENT0_C2                       0x00040000
#define MASK_SGPIO_ERR_INTR_EVENT0_C3                       0x00080000
#define MASK_SGPIO_ERR_INTR_EVENT0_C4                       0x00100000
#define MASK_SGPIO_ERR_INTR_EVENT0_C5                       0x00200000
#define MASK_SGPIO_ERR_INTR_EVENT0_C6                       0x00400000
#define MASK_SGPIO_ERR_INTR_EVENT0_C7                       0x00800000
#define MASK_SGPIO_ERR_INTR_EVENT0_D0                       0x01000000
#define MASK_SGPIO_ERR_INTR_EVENT0_D1                       0x02000000
#define MASK_SGPIO_ERR_INTR_EVENT0_D2                       0x04000000
#define MASK_SGPIO_ERR_INTR_EVENT0_D3                       0x08000000
#define MASK_SGPIO_ERR_INTR_EVENT0_D4                       0x10000000
#define MASK_SGPIO_ERR_INTR_EVENT0_D5                       0x20000000
#define MASK_SGPIO_ERR_INTR_EVENT0_D6                       0x40000000
#define MASK_SGPIO_ERR_INTR_EVENT0_D7                       0x80000000

#define MASK_SGPIO_ERR_INTR_EVENT1_E0                       0x00000001
#define MASK_SGPIO_ERR_INTR_EVENT1_E1                       0x00000002
#define MASK_SGPIO_ERR_INTR_EVENT1_E2                       0x00000004
#define MASK_SGPIO_ERR_INTR_EVENT1_E3                       0x00000008
#define MASK_SGPIO_ERR_INTR_EVENT1_E4                       0x00000010
#define MASK_SGPIO_ERR_INTR_EVENT1_E5                       0x00000020
#define MASK_SGPIO_ERR_INTR_EVENT1_E6                       0x00000040
#define MASK_SGPIO_ERR_INTR_EVENT1_E7                       0x00000080
#define MASK_SGPIO_ERR_INTR_EVENT1_F0                       0x00000100
#define MASK_SGPIO_ERR_INTR_EVENT1_F1                       0x00000200
#define MASK_SGPIO_ERR_INTR_EVENT1_F2                       0x00000400
#define MASK_SGPIO_ERR_INTR_EVENT1_F3                       0x00000800
#define MASK_SGPIO_ERR_INTR_EVENT1_F4                       0x00001000
#define MASK_SGPIO_ERR_INTR_EVENT1_F5                       0x00002000
#define MASK_SGPIO_ERR_INTR_EVENT1_F6                       0x00004000
#define MASK_SGPIO_ERR_INTR_EVENT1_F7                       0x00008000
#define MASK_SGPIO_ERR_INTR_EVENT1_G0                       0x00010000
#define MASK_SGPIO_ERR_INTR_EVENT1_G1                       0x00020000
#define MASK_SGPIO_ERR_INTR_EVENT1_G2                       0x00040000
#define MASK_SGPIO_ERR_INTR_EVENT1_G3                       0x00080000
#define MASK_SGPIO_ERR_INTR_EVENT1_G4                       0x00100000
#define MASK_SGPIO_ERR_INTR_EVENT1_G5                       0x00200000
#define MASK_SGPIO_ERR_INTR_EVENT1_G6                       0x00400000
#define MASK_SGPIO_ERR_INTR_EVENT1_G7                       0x00800000
#define MASK_SGPIO_ERR_INTR_EVENT1_H0                       0x01000000
#define MASK_SGPIO_ERR_INTR_EVENT1_H1                       0x02000000
#define MASK_SGPIO_ERR_INTR_EVENT1_H2                       0x04000000
#define MASK_SGPIO_ERR_INTR_EVENT1_H3                       0x08000000
#define MASK_SGPIO_ERR_INTR_EVENT1_H4                       0x10000000
#define MASK_SGPIO_ERR_INTR_EVENT1_H5                       0x20000000
#define MASK_SGPIO_ERR_INTR_EVENT1_H6                       0x40000000
#define MASK_SGPIO_ERR_INTR_EVENT1_H7                       0x80000000

#define MASK_SGPIO_MAN_INTR_EVENT0_A0                      0x00000001
#define MASK_SGPIO_MAN_INTR_EVENT0_A1                      0x00000002
#define MASK_SGPIO_MAN_INTR_EVENT0_A2                      0x00000004
#define MASK_SGPIO_MAN_INTR_EVENT0_A3                      0x00000008
#define MASK_SGPIO_MAN_INTR_EVENT0_A4                      0x00000010
#define MASK_SGPIO_MAN_INTR_EVENT0_A5                      0x00000020
#define MASK_SGPIO_MAN_INTR_EVENT0_A6                      0x00000040
#define MASK_SGPIO_MAN_INTR_EVENT0_A7                      0x00000080
#define MASK_SGPIO_MAN_INTR_EVENT0_B0                      0x00000100
#define MASK_SGPIO_MAN_INTR_EVENT0_B1                      0x00000200
#define MASK_SGPIO_MAN_INTR_EVENT0_B2                      0x00000400
#define MASK_SGPIO_MAN_INTR_EVENT0_B3                      0x00000800
#define MASK_SGPIO_MAN_INTR_EVENT0_B4                      0x00001000
#define MASK_SGPIO_MAN_INTR_EVENT0_B5                      0x00002000
#define MASK_SGPIO_MAN_INTR_EVENT0_B6                      0x00004000
#define MASK_SGPIO_MAN_INTR_EVENT0_B7                      0x00008000
#define MASK_SGPIO_MAN_INTR_EVENT0_C0                      0x00010000
#define MASK_SGPIO_MAN_INTR_EVENT0_C1                      0x00020000
#define MASK_SGPIO_MAN_INTR_EVENT0_C2                      0x00040000
#define MASK_SGPIO_MAN_INTR_EVENT0_C3                      0x00080000
#define MASK_SGPIO_MAN_INTR_EVENT0_C4                      0x00100000
#define MASK_SGPIO_MAN_INTR_EVENT0_C5                      0x00200000
#define MASK_SGPIO_MAN_INTR_EVENT0_C6                      0x00400000
#define MASK_SGPIO_MAN_INTR_EVENT0_C7                      0x00800000
#define MASK_SGPIO_MAN_INTR_EVENT0_D0                      0x01000000
#define MASK_SGPIO_MAN_INTR_EVENT0_D1                      0x02000000
#define MASK_SGPIO_MAN_INTR_EVENT0_D2                      0x04000000
#define MASK_SGPIO_MAN_INTR_EVENT0_D3                      0x08000000
#define MASK_SGPIO_MAN_INTR_EVENT0_D4                      0x10000000
#define MASK_SGPIO_MAN_INTR_EVENT0_D5                      0x20000000
#define MASK_SGPIO_MAN_INTR_EVENT0_D6                      0x40000000
#define MASK_SGPIO_MAN_INTR_EVENT0_D7                      0x80000000

#define MASK_SGPIO_MAN_INTR_EVENT1_E0                      0x00000001
#define MASK_SGPIO_MAN_INTR_EVENT1_E1                      0x00000002
#define MASK_SGPIO_MAN_INTR_EVENT1_E2                      0x00000004
#define MASK_SGPIO_MAN_INTR_EVENT1_E3                      0x00000008
#define MASK_SGPIO_MAN_INTR_EVENT1_E4                      0x00000010
#define MASK_SGPIO_MAN_INTR_EVENT1_E5                      0x00000020
#define MASK_SGPIO_MAN_INTR_EVENT1_E6                      0x00000040
#define MASK_SGPIO_MAN_INTR_EVENT1_E7                      0x00000080
#define MASK_SGPIO_MAN_INTR_EVENT1_F0                      0x00000100
#define MASK_SGPIO_MAN_INTR_EVENT1_F1                      0x00000200
#define MASK_SGPIO_MAN_INTR_EVENT1_F2                      0x00000400
#define MASK_SGPIO_MAN_INTR_EVENT1_F3                      0x00000800
#define MASK_SGPIO_MAN_INTR_EVENT1_F4                      0x00001000
#define MASK_SGPIO_MAN_INTR_EVENT1_F5                      0x00002000
#define MASK_SGPIO_MAN_INTR_EVENT1_F6                      0x00004000
#define MASK_SGPIO_MAN_INTR_EVENT1_F7                      0x00008000
#define MASK_SGPIO_MAN_INTR_EVENT1_G0                      0x00010000
#define MASK_SGPIO_MAN_INTR_EVENT1_G1                      0x00020000
#define MASK_SGPIO_MAN_INTR_EVENT1_G2                      0x00040000
#define MASK_SGPIO_MAN_INTR_EVENT1_G3                      0x00080000
#define MASK_SGPIO_MAN_INTR_EVENT1_G4                      0x00100000
#define MASK_SGPIO_MAN_INTR_EVENT1_G5                      0x00200000
#define MASK_SGPIO_MAN_INTR_EVENT1_G6                      0x00400000
#define MASK_SGPIO_MAN_INTR_EVENT1_G7                      0x00800000
#define MASK_SGPIO_MAN_INTR_EVENT1_H0                      0x01000000
#define MASK_SGPIO_MAN_INTR_EVENT1_H1                      0x02000000
#define MASK_SGPIO_MAN_INTR_EVENT1_H2                      0x04000000
#define MASK_SGPIO_MAN_INTR_EVENT1_H3                      0x08000000
#define MASK_SGPIO_MAN_INTR_EVENT1_H4                      0x10000000
#define MASK_SGPIO_MAN_INTR_EVENT1_H5                      0x20000000
#define MASK_SGPIO_MAN_INTR_EVENT1_H6                      0x40000000
#define MASK_SGPIO_MAN_INTR_EVENT1_H7                      0x80000000

#define MASK_SGPIO_NETWORK_INTR_EVENT0_A0                   0x00000001
#define MASK_SGPIO_NETWORK_INTR_EVENT0_A1                   0x00000002
#define MASK_SGPIO_NETWORK_INTR_EVENT0_A2                   0x00000004
#define MASK_SGPIO_NETWORK_INTR_EVENT0_A3                   0x00000008
#define MASK_SGPIO_NETWORK_INTR_EVENT0_A4                   0x00000010
#define MASK_SGPIO_NETWORK_INTR_EVENT0_A5                   0x00000020
#define MASK_SGPIO_NETWORK_INTR_EVENT0_A6                   0x00000040
#define MASK_SGPIO_NETWORK_INTR_EVENT0_A7                   0x00000080
#define MASK_SGPIO_NETWORK_INTR_EVENT0_B0                   0x00000100
#define MASK_SGPIO_NETWORK_INTR_EVENT0_B1                   0x00000200
#define MASK_SGPIO_NETWORK_INTR_EVENT0_B2                   0x00000400
#define MASK_SGPIO_NETWORK_INTR_EVENT0_B3                   0x00000800
#define MASK_SGPIO_NETWORK_INTR_EVENT0_B4                   0x00001000
#define MASK_SGPIO_NETWORK_INTR_EVENT0_B5                   0x00002000
#define MASK_SGPIO_NETWORK_INTR_EVENT0_B6                   0x00004000
#define MASK_SGPIO_NETWORK_INTR_EVENT0_B7                   0x00008000
#define MASK_SGPIO_NETWORK_INTR_EVENT0_C0                   0x00010000
#define MASK_SGPIO_NETWORK_INTR_EVENT0_C1                   0x00020000
#define MASK_SGPIO_NETWORK_INTR_EVENT0_C2                   0x00040000
#define MASK_SGPIO_NETWORK_INTR_EVENT0_C3                   0x00080000
#define MASK_SGPIO_NETWORK_INTR_EVENT0_C4                   0x00100000
#define MASK_SGPIO_NETWORK_INTR_EVENT0_C5                   0x00200000
#define MASK_SGPIO_NETWORK_INTR_EVENT0_C6                   0x00400000
#define MASK_SGPIO_NETWORK_INTR_EVENT0_C7                   0x00800000
#define MASK_SGPIO_NETWORK_INTR_EVENT0_D0                   0x01000000
#define MASK_SGPIO_NETWORK_INTR_EVENT0_D1                   0x02000000
#define MASK_SGPIO_NETWORK_INTR_EVENT0_D2                   0x04000000
#define MASK_SGPIO_NETWORK_INTR_EVENT0_D3                   0x08000000
#define MASK_SGPIO_NETWORK_INTR_EVENT0_D4                   0x10000000
#define MASK_SGPIO_NETWORK_INTR_EVENT0_D5                   0x20000000
#define MASK_SGPIO_NETWORK_INTR_EVENT0_D6                   0x40000000
#define MASK_SGPIO_NETWORK_INTR_EVENT0_D7                   0x80000000

#define MASK_SGPIO_NETWORK_INTR_EVENT1_E0                   0x00000001
#define MASK_SGPIO_NETWORK_INTR_EVENT1_E1                   0x00000002
#define MASK_SGPIO_NETWORK_INTR_EVENT1_E2                   0x00000004
#define MASK_SGPIO_NETWORK_INTR_EVENT1_E3                   0x00000008
#define MASK_SGPIO_NETWORK_INTR_EVENT1_E4                   0x00000010
#define MASK_SGPIO_NETWORK_INTR_EVENT1_E5                   0x00000020
#define MASK_SGPIO_NETWORK_INTR_EVENT1_E6                   0x00000040
#define MASK_SGPIO_NETWORK_INTR_EVENT1_E7                   0x00000080
#define MASK_SGPIO_NETWORK_INTR_EVENT1_F0                   0x00000100
#define MASK_SGPIO_NETWORK_INTR_EVENT1_F1                   0x00000200
#define MASK_SGPIO_NETWORK_INTR_EVENT1_F2                   0x00000400
#define MASK_SGPIO_NETWORK_INTR_EVENT1_F3                   0x00000800
#define MASK_SGPIO_NETWORK_INTR_EVENT1_F4                   0x00001000
#define MASK_SGPIO_NETWORK_INTR_EVENT1_F5                   0x00002000
#define MASK_SGPIO_NETWORK_INTR_EVENT1_F6                   0x00004000
#define MASK_SGPIO_NETWORK_INTR_EVENT1_F7                   0x00008000
#define MASK_SGPIO_NETWORK_INTR_EVENT1_G0                   0x00010000
#define MASK_SGPIO_NETWORK_INTR_EVENT1_G1                   0x00020000
#define MASK_SGPIO_NETWORK_INTR_EVENT1_G2                   0x00040000
#define MASK_SGPIO_NETWORK_INTR_EVENT1_G3                   0x00080000
#define MASK_SGPIO_NETWORK_INTR_EVENT1_G4                   0x00100000
#define MASK_SGPIO_NETWORK_INTR_EVENT1_G5                   0x00200000
#define MASK_SGPIO_NETWORK_INTR_EVENT1_G6                   0x00400000
#define MASK_SGPIO_NETWORK_INTR_EVENT1_G7                   0x00800000
#define MASK_SGPIO_NETWORK_INTR_EVENT1_H0                   0x01000000
#define MASK_SGPIO_NETWORK_INTR_EVENT1_H1                   0x02000000
#define MASK_SGPIO_NETWORK_INTR_EVENT1_H2                   0x04000000
#define MASK_SGPIO_NETWORK_INTR_EVENT1_H3                   0x08000000
#define MASK_SGPIO_NETWORK_INTR_EVENT1_H4                   0x10000000
#define MASK_SGPIO_NETWORK_INTR_EVENT1_H5                   0x20000000
#define MASK_SGPIO_NETWORK_INTR_EVENT1_H6                   0x40000000
#define MASK_SGPIO_NETWORK_INTR_EVENT1_H7                   0x80000000

#define MASK_SGPIO_ERR_INTR_EN0_A0                          0x00000001
#define MASK_SGPIO_ERR_INTR_EN0_A1                          0x00000002
#define MASK_SGPIO_ERR_INTR_EN0_A2                          0x00000004
#define MASK_SGPIO_ERR_INTR_EN0_A3                          0x00000008
#define MASK_SGPIO_ERR_INTR_EN0_A4                          0x00000010
#define MASK_SGPIO_ERR_INTR_EN0_A5                          0x00000020
#define MASK_SGPIO_ERR_INTR_EN0_A6                          0x00000040
#define MASK_SGPIO_ERR_INTR_EN0_A7                          0x00000080
#define MASK_SGPIO_ERR_INTR_EN0_B0                          0x00000100
#define MASK_SGPIO_ERR_INTR_EN0_B1                          0x00000200
#define MASK_SGPIO_ERR_INTR_EN0_B2                          0x00000400
#define MASK_SGPIO_ERR_INTR_EN0_B3                          0x00000800
#define MASK_SGPIO_ERR_INTR_EN0_B4                          0x00001000
#define MASK_SGPIO_ERR_INTR_EN0_B5                          0x00002000
#define MASK_SGPIO_ERR_INTR_EN0_B6                          0x00004000
#define MASK_SGPIO_ERR_INTR_EN0_B7                          0x00008000
#define MASK_SGPIO_ERR_INTR_EN0_C0                          0x00010000
#define MASK_SGPIO_ERR_INTR_EN0_C1                          0x00020000
#define MASK_SGPIO_ERR_INTR_EN0_C2                          0x00040000
#define MASK_SGPIO_ERR_INTR_EN0_C3                          0x00080000
#define MASK_SGPIO_ERR_INTR_EN0_C4                          0x00100000
#define MASK_SGPIO_ERR_INTR_EN0_C5                          0x00200000
#define MASK_SGPIO_ERR_INTR_EN0_C6                          0x00400000
#define MASK_SGPIO_ERR_INTR_EN0_C7                          0x00800000
#define MASK_SGPIO_ERR_INTR_EN0_D0                          0x01000000
#define MASK_SGPIO_ERR_INTR_EN0_D1                          0x02000000
#define MASK_SGPIO_ERR_INTR_EN0_D2                          0x04000000
#define MASK_SGPIO_ERR_INTR_EN0_D3                          0x08000000
#define MASK_SGPIO_ERR_INTR_EN0_D4                          0x10000000
#define MASK_SGPIO_ERR_INTR_EN0_D5                          0x20000000
#define MASK_SGPIO_ERR_INTR_EN0_D6                          0x40000000
#define MASK_SGPIO_ERR_INTR_EN0_D7                          0x80000000

#define MASK_SGPIO_ERR_INTR_EN1_E0                          0x00000001
#define MASK_SGPIO_ERR_INTR_EN1_E1                          0x00000002
#define MASK_SGPIO_ERR_INTR_EN1_E2                          0x00000004
#define MASK_SGPIO_ERR_INTR_EN1_E3                          0x00000008
#define MASK_SGPIO_ERR_INTR_EN1_E4                          0x00000010
#define MASK_SGPIO_ERR_INTR_EN1_E5                          0x00000020
#define MASK_SGPIO_ERR_INTR_EN1_E6                          0x00000040
#define MASK_SGPIO_ERR_INTR_EN1_E7                          0x00000080
#define MASK_SGPIO_ERR_INTR_EN1_F0                          0x00000100
#define MASK_SGPIO_ERR_INTR_EN1_F1                          0x00000200
#define MASK_SGPIO_ERR_INTR_EN1_F2                          0x00000400
#define MASK_SGPIO_ERR_INTR_EN1_F3                          0x00000800
#define MASK_SGPIO_ERR_INTR_EN1_F4                          0x00001000
#define MASK_SGPIO_ERR_INTR_EN1_F5                          0x00002000
#define MASK_SGPIO_ERR_INTR_EN1_F6                          0x00004000
#define MASK_SGPIO_ERR_INTR_EN1_F7                          0x00008000
#define MASK_SGPIO_ERR_INTR_EN1_G0                          0x00010000
#define MASK_SGPIO_ERR_INTR_EN1_G1                          0x00020000
#define MASK_SGPIO_ERR_INTR_EN1_G2                          0x00040000
#define MASK_SGPIO_ERR_INTR_EN1_G3                          0x00080000
#define MASK_SGPIO_ERR_INTR_EN1_G4                          0x00100000
#define MASK_SGPIO_ERR_INTR_EN1_G5                          0x00200000
#define MASK_SGPIO_ERR_INTR_EN1_G6                          0x00400000
#define MASK_SGPIO_ERR_INTR_EN1_G7                          0x00800000
#define MASK_SGPIO_ERR_INTR_EN1_H0                          0x01000000
#define MASK_SGPIO_ERR_INTR_EN1_H1                          0x02000000
#define MASK_SGPIO_ERR_INTR_EN1_H2                          0x04000000
#define MASK_SGPIO_ERR_INTR_EN1_H3                          0x08000000
#define MASK_SGPIO_ERR_INTR_EN1_H4                          0x10000000
#define MASK_SGPIO_ERR_INTR_EN1_H5                          0x20000000
#define MASK_SGPIO_ERR_INTR_EN1_H6                          0x40000000
#define MASK_SGPIO_ERR_INTR_EN1_H7                          0x80000000

#define MASK_SGPIO_MAN_INTR_EN0_A0                         0x00000001
#define MASK_SGPIO_MAN_INTR_EN0_A1                         0x00000002
#define MASK_SGPIO_MAN_INTR_EN0_A2                         0x00000004
#define MASK_SGPIO_MAN_INTR_EN0_A3                         0x00000008
#define MASK_SGPIO_MAN_INTR_EN0_A4                         0x00000010
#define MASK_SGPIO_MAN_INTR_EN0_A5                        0x00000020
#define MASK_SGPIO_MAN_INTR_EN0_A6                         0x00000040
#define MASK_SGPIO_MAN_INTR_EN0_A7                         0x00000080
#define MASK_SGPIO_MAN_INTR_EN0_B0                         0x00000100
#define MASK_SGPIO_MAN_INTR_EN0_B1                         0x00000200
#define MASK_SGPIO_MAN_INTR_EN0_B2                         0x00000400
#define MASK_SGPIO_MAN_INTR_EN0_B3                         0x00000800
#define MASK_SGPIO_MAN_INTR_EN0_B4                         0x00001000
#define MASK_SGPIO_MAN_INTR_EN0_B5                         0x00002000
#define MASK_SGPIO_MAN_INTR_EN0_B6                         0x00004000
#define MASK_SGPIO_MAN_INTR_EN0_B7                         0x00008000
#define MASK_SGPIO_MAN_INTR_EN0_C0                         0x00010000
#define MASK_SGPIO_MAN_INTR_EN0_C1                         0x00020000
#define MASK_SGPIO_MAN_INTR_EN0_C2                         0x00040000
#define MASK_SGPIO_MAN_INTR_EN0_C3                         0x00080000
#define MASK_SGPIO_MAN_INTR_EN0_C4                         0x00100000
#define MASK_SGPIO_MAN_INTR_EN0_C5                         0x00200000
#define MASK_SGPIO_MAN_INTR_EN0_C6                         0x00400000
#define MASK_SGPIO_MAN_INTR_EN0_C7                         0x00800000
#define MASK_SGPIO_MAN_INTR_EN0_D0                         0x01000000
#define MASK_SGPIO_MAN_INTR_EN0_D1                         0x02000000
#define MASK_SGPIO_MAN_INTR_EN0_D2                         0x04000000
#define MASK_SGPIO_MAN_INTR_EN0_D3                         0x08000000
#define MASK_SGPIO_MAN_INTR_EN0_D4                         0x10000000
#define MASK_SGPIO_MAN_INTR_EN0_D5                         0x20000000
#define MASK_SGPIO_MAN_INTR_EN0_D6                         0x40000000
#define MASK_SGPIO_MAN_INTR_EN0_D7                         0x80000000

#define MASK_SGPIO_MAN_INTR_EN1_E0                         0x00000001
#define MASK_SGPIO_MAN_INTR_EN1_E1                         0x00000002
#define MASK_SGPIO_MAN_INTR_EN1_E2                         0x00000004
#define MASK_SGPIO_MAN_INTR_EN1_E3                         0x00000008
#define MASK_SGPIO_MAN_INTR_EN1_E4                         0x00000010
#define MASK_SGPIO_MAN_INTR_EN1_E5                         0x00000020
#define MASK_SGPIO_MAN_INTR_EN1_E6                         0x00000040
#define MASK_SGPIO_MAN_INTR_EN1_E7                         0x00000080
#define MASK_SGPIO_MAN_INTR_EN1_F0                         0x00000100
#define MASK_SGPIO_MAN_INTR_EN1_F1                         0x00000200
#define MASK_SGPIO_MAN_INTR_EN1_F2                         0x00000400
#define MASK_SGPIO_MAN_INTR_EN1_F3                         0x00000800
#define MASK_SGPIO_MAN_INTR_EN1_F4                         0x00001000
#define MASK_SGPIO_MAN_INTR_EN1_F5                         0x00002000
#define MASK_SGPIO_MAN_INTR_EN1_F6                         0x00004000
#define MASK_SGPIO_MAN_INTR_EN1_F7                         0x00008000
#define MASK_SGPIO_MAN_INTR_EN1_G0                         0x00010000
#define MASK_SGPIO_MAN_INTR_EN1_G1                         0x00020000
#define MASK_SGPIO_MAN_INTR_EN1_G2                         0x00040000
#define MASK_SGPIO_MAN_INTR_EN1_G3                         0x00080000
#define MASK_SGPIO_MAN_INTR_EN1_G4                         0x00100000
#define MASK_SGPIO_MAN_INTR_EN1_G5                         0x00200000
#define MASK_SGPIO_MAN_INTR_EN1_G6                         0x00400000
#define MASK_SGPIO_MAN_INTR_EN1_G7                         0x00800000
#define MASK_SGPIO_MAN_INTR_EN1_H0                         0x01000000
#define MASK_SGPIO_MAN_INTR_EN1_H1                         0x02000000
#define MASK_SGPIO_MAN_INTR_EN1_H2                         0x04000000
#define MASK_SGPIO_MAN_INTR_EN1_H3                         0x08000000
#define MASK_SGPIO_MAN_INTR_EN1_H4                         0x10000000
#define MASK_SGPIO_MAN_INTR_EN1_H5                         0x20000000
#define MASK_SGPIO_MAN_INTR_EN1_H6                         0x40000000
#define MASK_SGPIO_MAN_INTR_EN1_H7                         0x80000000

#define MASK_SGPIO_NETWORK_INTR_EN0_A0                      0x00000001
#define MASK_SGPIO_NETWORK_INTR_EN0_A1                      0x00000002
#define MASK_SGPIO_NETWORK_INTR_EN0_A2                      0x00000004
#define MASK_SGPIO_NETWORK_INTR_EN0_A3                      0x00000008
#define MASK_SGPIO_NETWORK_INTR_EN0_A4                      0x00000010
#define MASK_SGPIO_NETWORK_INTR_EN0_A5                      0x00000020
#define MASK_SGPIO_NETWORK_INTR_EN0_A6                      0x00000040
#define MASK_SGPIO_NETWORK_INTR_EN0_A7                      0x00000080
#define MASK_SGPIO_NETWORK_INTR_EN0_B0                      0x00000100
#define MASK_SGPIO_NETWORK_INTR_EN0_B1                      0x00000200
#define MASK_SGPIO_NETWORK_INTR_EN0_B2                      0x00000400
#define MASK_SGPIO_NETWORK_INTR_EN0_B3                      0x00000800
#define MASK_SGPIO_NETWORK_INTR_EN0_B4                      0x00001000
#define MASK_SGPIO_NETWORK_INTR_EN0_B5                      0x00002000
#define MASK_SGPIO_NETWORK_INTR_EN0_B6                      0x00004000
#define MASK_SGPIO_NETWORK_INTR_EN0_B7                      0x00008000
#define MASK_SGPIO_NETWORK_INTR_EN0_C0                      0x00010000
#define MASK_SGPIO_NETWORK_INTR_EN0_C1                      0x00020000
#define MASK_SGPIO_NETWORK_INTR_EN0_C2                      0x00040000
#define MASK_SGPIO_NETWORK_INTR_EN0_C3                      0x00080000
#define MASK_SGPIO_NETWORK_INTR_EN0_C4                      0x00100000
#define MASK_SGPIO_NETWORK_INTR_EN0_C5                      0x00200000
#define MASK_SGPIO_NETWORK_INTR_EN0_C6                      0x00400000
#define MASK_SGPIO_NETWORK_INTR_EN0_C7                      0x00800000
#define MASK_SGPIO_NETWORK_INTR_EN0_D0                      0x01000000
#define MASK_SGPIO_NETWORK_INTR_EN0_D1                      0x02000000
#define MASK_SGPIO_NETWORK_INTR_EN0_D2                      0x04000000
#define MASK_SGPIO_NETWORK_INTR_EN0_D3                      0x08000000
#define MASK_SGPIO_NETWORK_INTR_EN0_D4                      0x10000000
#define MASK_SGPIO_NETWORK_INTR_EN0_D5                      0x20000000
#define MASK_SGPIO_NETWORK_INTR_EN0_D6                      0x40000000
#define MASK_SGPIO_NETWORK_INTR_EN0_D7                      0x80000000

#define MASK_SGPIO_NETWORK_INTR_EN1_E0                      0x00000001
#define MASK_SGPIO_NETWORK_INTR_EN1_E1                      0x00000002
#define MASK_SGPIO_NETWORK_INTR_EN1_E2                      0x00000004
#define MASK_SGPIO_NETWORK_INTR_EN1_E3                      0x00000008
#define MASK_SGPIO_NETWORK_INTR_EN1_E4                      0x00000010
#define MASK_SGPIO_NETWORK_INTR_EN1_E5                      0x00000020
#define MASK_SGPIO_NETWORK_INTR_EN1_E6                      0x00000040
#define MASK_SGPIO_NETWORK_INTR_EN1_E7                      0x00000080
#define MASK_SGPIO_NETWORK_INTR_EN1_F0                      0x00000100
#define MASK_SGPIO_NETWORK_INTR_EN1_F1                      0x00000200
#define MASK_SGPIO_NETWORK_INTR_EN1_F2                      0x00000400
#define MASK_SGPIO_NETWORK_INTR_EN1_F3                      0x00000800
#define MASK_SGPIO_NETWORK_INTR_EN1_F4                      0x00001000
#define MASK_SGPIO_NETWORK_INTR_EN1_F5                      0x00002000
#define MASK_SGPIO_NETWORK_INTR_EN1_F6                      0x00004000
#define MASK_SGPIO_NETWORK_INTR_EN1_F7                      0x00008000
#define MASK_SGPIO_NETWORK_INTR_EN1_G0                      0x00010000
#define MASK_SGPIO_NETWORK_INTR_EN1_G1                      0x00020000
#define MASK_SGPIO_NETWORK_INTR_EN1_G2                      0x00040000
#define MASK_SGPIO_NETWORK_INTR_EN1_G3                      0x00080000
#define MASK_SGPIO_NETWORK_INTR_EN1_G4                      0x00100000
#define MASK_SGPIO_NETWORK_INTR_EN1_G5                      0x00200000
#define MASK_SGPIO_NETWORK_INTR_EN1_G6                      0x00400000
#define MASK_SGPIO_NETWORK_INTR_EN1_G7                      0x00800000
#define MASK_SGPIO_NETWORK_INTR_EN1_H0                      0x01000000
#define MASK_SGPIO_NETWORK_INTR_EN1_H1                      0x02000000
#define MASK_SGPIO_NETWORK_INTR_EN1_H2                      0x04000000
#define MASK_SGPIO_NETWORK_INTR_EN1_H3                      0x08000000
#define MASK_SGPIO_NETWORK_INTR_EN1_H4                      0x10000000
#define MASK_SGPIO_NETWORK_INTR_EN1_H5                      0x20000000
#define MASK_SGPIO_NETWORK_INTR_EN1_H6                      0x40000000
#define MASK_SGPIO_NETWORK_INTR_EN1_H7                      0x80000000

#define MASK_SGPIO_OUTPUT_POLARITY_CTRL0_A0                 0x00000001
#define MASK_SGPIO_OUTPUT_POLARITY_CTRL0_A1                 0x00000002
#define MASK_SGPIO_OUTPUT_POLARITY_CTRL0_A2                 0x00000004
#define MASK_SGPIO_OUTPUT_POLARITY_CTRL0_A3                 0x00000008
#define MASK_SGPIO_OUTPUT_POLARITY_CTRL0_A4                 0x00000010
#define MASK_SGPIO_OUTPUT_POLARITY_CTRL0_A5                 0x00000020
#define MASK_SGPIO_OUTPUT_POLARITY_CTRL0_A6                 0x00000040
#define MASK_SGPIO_OUTPUT_POLARITY_CTRL0_A7                 0x00000080
#define MASK_SGPIO_OUTPUT_POLARITY_CTRL0_B0                 0x00000100
#define MASK_SGPIO_OUTPUT_POLARITY_CTRL0_B1                 0x00000200
#define MASK_SGPIO_OUTPUT_POLARITY_CTRL0_B2                 0x00000400
#define MASK_SGPIO_OUTPUT_POLARITY_CTRL0_B3                 0x00000800
#define MASK_SGPIO_OUTPUT_POLARITY_CTRL0_B4                 0x00001000
#define MASK_SGPIO_OUTPUT_POLARITY_CTRL0_B5                 0x00002000
#define MASK_SGPIO_OUTPUT_POLARITY_CTRL0_B6                 0x00004000
#define MASK_SGPIO_OUTPUT_POLARITY_CTRL0_B7                 0x00008000
#define MASK_SGPIO_OUTPUT_POLARITY_CTRL0_C0                 0x00010000
#define MASK_SGPIO_OUTPUT_POLARITY_CTRL0_C1                 0x00020000
#define MASK_SGPIO_OUTPUT_POLARITY_CTRL0_C2                 0x00040000
#define MASK_SGPIO_OUTPUT_POLARITY_CTRL0_C3                 0x00080000
#define MASK_SGPIO_OUTPUT_POLARITY_CTRL0_C4                 0x00100000
#define MASK_SGPIO_OUTPUT_POLARITY_CTRL0_C5                 0x00200000
#define MASK_SGPIO_OUTPUT_POLARITY_CTRL0_C6                 0x00400000
#define MASK_SGPIO_OUTPUT_POLARITY_CTRL0_C7                 0x00800000
#define MASK_SGPIO_OUTPUT_POLARITY_CTRL0_D0                 0x01000000
#define MASK_SGPIO_OUTPUT_POLARITY_CTRL0_D1                 0x02000000
#define MASK_SGPIO_OUTPUT_POLARITY_CTRL0_D2                 0x04000000
#define MASK_SGPIO_OUTPUT_POLARITY_CTRL0_D3                 0x08000000
#define MASK_SGPIO_OUTPUT_POLARITY_CTRL0_D4                 0x10000000
#define MASK_SGPIO_OUTPUT_POLARITY_CTRL0_D5                 0x20000000
#define MASK_SGPIO_OUTPUT_POLARITY_CTRL0_D6                 0x40000000
#define MASK_SGPIO_OUTPUT_POLARITY_CTRL0_D7                 0x80000000

#define MASK_SGPIO_OUTPUT_POLARITY_CTRL1_E0                 0x00000001
#define MASK_SGPIO_OUTPUT_POLARITY_CTRL1_E1                 0x00000002
#define MASK_SGPIO_OUTPUT_POLARITY_CTRL1_E2                 0x00000004
#define MASK_SGPIO_OUTPUT_POLARITY_CTRL1_E3                 0x00000008
#define MASK_SGPIO_OUTPUT_POLARITY_CTRL1_E4                 0x00000010
#define MASK_SGPIO_OUTPUT_POLARITY_CTRL1_E5                 0x00000020
#define MASK_SGPIO_OUTPUT_POLARITY_CTRL1_E6                 0x00000040
#define MASK_SGPIO_OUTPUT_POLARITY_CTRL1_E7                 0x00000080
#define MASK_SGPIO_OUTPUT_POLARITY_CTRL1_F0                 0x00000100
#define MASK_SGPIO_OUTPUT_POLARITY_CTRL1_F1                 0x00000200
#define MASK_SGPIO_OUTPUT_POLARITY_CTRL1_F2                 0x00000400
#define MASK_SGPIO_OUTPUT_POLARITY_CTRL1_F3                 0x00000800
#define MASK_SGPIO_OUTPUT_POLARITY_CTRL1_F4                 0x00001000
#define MASK_SGPIO_OUTPUT_POLARITY_CTRL1_F5                 0x00002000
#define MASK_SGPIO_OUTPUT_POLARITY_CTRL1_F6                 0x00004000
#define MASK_SGPIO_OUTPUT_POLARITY_CTRL1_F7                 0x00008000
#define MASK_SGPIO_OUTPUT_POLARITY_CTRL1_G0                 0x00010000
#define MASK_SGPIO_OUTPUT_POLARITY_CTRL1_G1                 0x00020000
#define MASK_SGPIO_OUTPUT_POLARITY_CTRL1_G2                 0x00040000
#define MASK_SGPIO_OUTPUT_POLARITY_CTRL1_G3                 0x00080000
#define MASK_SGPIO_OUTPUT_POLARITY_CTRL1_G4                 0x00100000
#define MASK_SGPIO_OUTPUT_POLARITY_CTRL1_G5                 0x00200000
#define MASK_SGPIO_OUTPUT_POLARITY_CTRL1_G6                 0x00400000
#define MASK_SGPIO_OUTPUT_POLARITY_CTRL1_G7                 0x00800000
#define MASK_SGPIO_OUTPUT_POLARITY_CTRL1_H0                 0x01000000
#define MASK_SGPIO_OUTPUT_POLARITY_CTRL1_H1                 0x02000000
#define MASK_SGPIO_OUTPUT_POLARITY_CTRL1_H2                 0x04000000
#define MASK_SGPIO_OUTPUT_POLARITY_CTRL1_H3                 0x08000000
#define MASK_SGPIO_OUTPUT_POLARITY_CTRL1_H4                 0x10000000
#define MASK_SGPIO_OUTPUT_POLARITY_CTRL1_H5                 0x20000000
#define MASK_SGPIO_OUTPUT_POLARITY_CTRL1_H6                 0x40000000
#define MASK_SGPIO_OUTPUT_POLARITY_CTRL1_H7                 0x80000000

#define MASK_SGPIO_INPUT_POLARITY_CTRL0_A0                  0x00000001
#define MASK_SGPIO_INPUT_POLARITY_CTRL0_A1                  0x00000002
#define MASK_SGPIO_INPUT_POLARITY_CTRL0_A2                  0x00000004
#define MASK_SGPIO_INPUT_POLARITY_CTRL0_A3                  0x00000008
#define MASK_SGPIO_INPUT_POLARITY_CTRL0_A4                  0x00000010
#define MASK_SGPIO_INPUT_POLARITY_CTRL0_A5                  0x00000020
#define MASK_SGPIO_INPUT_POLARITY_CTRL0_A6                  0x00000040
#define MASK_SGPIO_INPUT_POLARITY_CTRL0_A7                  0x00000080
#define MASK_SGPIO_INPUT_POLARITY_CTRL0_B0                  0x00000100
#define MASK_SGPIO_INPUT_POLARITY_CTRL0_B1                  0x00000200
#define MASK_SGPIO_INPUT_POLARITY_CTRL0_B2                  0x00000400
#define MASK_SGPIO_INPUT_POLARITY_CTRL0_B3                  0x00000800
#define MASK_SGPIO_INPUT_POLARITY_CTRL0_B4                  0x00001000
#define MASK_SGPIO_INPUT_POLARITY_CTRL0_B5                  0x00002000
#define MASK_SGPIO_INPUT_POLARITY_CTRL0_B6                  0x00004000
#define MASK_SGPIO_INPUT_POLARITY_CTRL0_B7                  0x00008000
#define MASK_SGPIO_INPUT_POLARITY_CTRL0_C0                  0x00010000
#define MASK_SGPIO_INPUT_POLARITY_CTRL0_C1                  0x00020000
#define MASK_SGPIO_INPUT_POLARITY_CTRL0_C2                  0x00040000
#define MASK_SGPIO_INPUT_POLARITY_CTRL0_C3                  0x00080000
#define MASK_SGPIO_INPUT_POLARITY_CTRL0_C4                  0x00100000
#define MASK_SGPIO_INPUT_POLARITY_CTRL0_C5                  0x00200000
#define MASK_SGPIO_INPUT_POLARITY_CTRL0_C6                  0x00400000
#define MASK_SGPIO_INPUT_POLARITY_CTRL0_C7                  0x00800000
#define MASK_SGPIO_INPUT_POLARITY_CTRL0_D0                  0x01000000
#define MASK_SGPIO_INPUT_POLARITY_CTRL0_D1                  0x02000000
#define MASK_SGPIO_INPUT_POLARITY_CTRL0_D2                  0x04000000
#define MASK_SGPIO_INPUT_POLARITY_CTRL0_D3                  0x08000000
#define MASK_SGPIO_INPUT_POLARITY_CTRL0_D4                  0x10000000
#define MASK_SGPIO_INPUT_POLARITY_CTRL0_D5                  0x20000000
#define MASK_SGPIO_INPUT_POLARITY_CTRL0_D6                  0x40000000
#define MASK_SGPIO_INPUT_POLARITY_CTRL0_D7                  0x80000000

#define MASK_SGPIO_INPUT_POLARITY_CTRL1_E0                  0x00000001
#define MASK_SGPIO_INPUT_POLARITY_CTRL1_E1                  0x00000002
#define MASK_SGPIO_INPUT_POLARITY_CTRL1_E2                  0x00000004
#define MASK_SGPIO_INPUT_POLARITY_CTRL1_E3                  0x00000008
#define MASK_SGPIO_INPUT_POLARITY_CTRL1_E4                  0x00000010
#define MASK_SGPIO_INPUT_POLARITY_CTRL1_E5                  0x00000020
#define MASK_SGPIO_INPUT_POLARITY_CTRL1_E6                  0x00000040
#define MASK_SGPIO_INPUT_POLARITY_CTRL1_E7                  0x00000080
#define MASK_SGPIO_INPUT_POLARITY_CTRL1_F0                  0x00000100
#define MASK_SGPIO_INPUT_POLARITY_CTRL1_F1                  0x00000200
#define MASK_SGPIO_INPUT_POLARITY_CTRL1_F2                  0x00000400
#define MASK_SGPIO_INPUT_POLARITY_CTRL1_F3                  0x00000800
#define MASK_SGPIO_INPUT_POLARITY_CTRL1_F4                  0x00001000
#define MASK_SGPIO_INPUT_POLARITY_CTRL1_F5                  0x00002000
#define MASK_SGPIO_INPUT_POLARITY_CTRL1_F6                  0x00004000
#define MASK_SGPIO_INPUT_POLARITY_CTRL1_F7                  0x00008000
#define MASK_SGPIO_INPUT_POLARITY_CTRL1_G0                  0x00010000
#define MASK_SGPIO_INPUT_POLARITY_CTRL1_G1                  0x00020000
#define MASK_SGPIO_INPUT_POLARITY_CTRL1_G2                  0x00040000
#define MASK_SGPIO_INPUT_POLARITY_CTRL1_G3                  0x00080000
#define MASK_SGPIO_INPUT_POLARITY_CTRL1_G4                  0x00100000
#define MASK_SGPIO_INPUT_POLARITY_CTRL1_G5                  0x00200000
#define MASK_SGPIO_INPUT_POLARITY_CTRL1_G6                  0x00400000
#define MASK_SGPIO_INPUT_POLARITY_CTRL1_G7                  0x00800000
#define MASK_SGPIO_INPUT_POLARITY_CTRL1_H0                  0x01000000
#define MASK_SGPIO_INPUT_POLARITY_CTRL1_H1                  0x02000000
#define MASK_SGPIO_INPUT_POLARITY_CTRL1_H2                  0x04000000
#define MASK_SGPIO_INPUT_POLARITY_CTRL1_H3                  0x08000000
#define MASK_SGPIO_INPUT_POLARITY_CTRL1_H4                  0x10000000
#define MASK_SGPIO_INPUT_POLARITY_CTRL1_H5                  0x20000000
#define MASK_SGPIO_INPUT_POLARITY_CTRL1_H6                  0x40000000
#define MASK_SGPIO_INPUT_POLARITY_CTRL1_H7                  0x80000000

extern void goofy_gpio_reg_display (dev_object_t *dev);
extern void gfy_gpio_attach (dev_object_t *dev);

#endif /* GOOFY_GPIO_H */

/******** History ******** 
$Log: goofy_gpio.h,v $
Revision 1.2  2012/03/28 00:38:09  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:02  ptong
Initial archive of ng_diag module


$Endlog$
*/
