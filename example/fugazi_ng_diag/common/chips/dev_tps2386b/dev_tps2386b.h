/* $Id: dev_tps2386b.h,v 1.2 2019/01/10 06:30:25 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_tps2386b/dev_tps2386b.h,v $
 *------------------------------------------------------------------
 * Filename   : dev_tps2386b.h
 *
 * Copyright (c) 2018 - 2019 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------------------
 */

#include "types.h"
#include "i2c_api.h"
#include "common_utils.h"
#include "dev_object.h"

/* structure of port status */
typedef struct tps2386b_port_status {
    int    port_number;       /* port number, 0~3 */
    int    port_op_mode;      /* port operation mode */
    int    port_detection;    /* port detection */
    int    port_class;        /* port class */
    int    port_power_good;   /* port power good/not good */
    int    port_onoff;        /* port on/off */
    double port_current;      /* port current (I) */
    double port_voltage;      /* port voltage (V) */
    double port_power;        /* port power (P=I*V)*/
}tps2386b_port_status_t;


typedef enum {
    TPS2386B_DEV_STATE = 0,
    TPS2386B_ATTACH,
    TPS2386B_REG_TEST,
    TPS2386B_I2C_RD,
    TPS2386B_I2C_WR,
} tps2386b_report_code_t;

typedef struct tps2386b_reg_info {
    const char  *reg_name;    /* name of register */
    const ulong reg_addr;     /* offset of register */
    const int   rw_type;      /* read or write type */
}tps2386b_reg_info_t;

/*==========[Call-in function]========== */
typedef struct tps2386b_callin_fvt_t_ {
    int (*register_test)(dev_object_t *);                  /* Register Test */
    int (*interrupt_test)(dev_object_t *);                 /* Interrupt Test */
    int (*util_read_reg)(dev_object_t *);                  /* register read */
    int (*util_write_reg)(dev_object_t *);                 /* register write */
    int (*util_dump_register)(dev_object_t *);             /* dump register */
    int (*util_show_pwr_stat)(dev_object_t *, int, int);   /* show pwr status */
    int (*util_detect_pwr)(dev_object_t *, int, int);     /* detect port pwr */
} tps2386b_callin_fvt_t;

/*==========[Call-out function]========== */
/* Function Vectors set by the upper level (eg., platform).*/
typedef struct tps2386b_callout_fvt_t_ {
    uint32_t (*open)(n2g_i2c_if_t *);   /* Open I2C device */
    uint32_t (*close)(n2g_i2c_if_t *);  /* Close I2C device */
    uint32_t (*rd)(n2g_i2c_if_t *);     /* I2C read function */
    uint32_t (*wr)(n2g_i2c_if_t *);     /* I2C write function */
    int (*chk_intr_assert)(void);       /* Check interrupt pin is asserted */
    int (*chk_intr_deassert)(void);     /* Check interrupt pin is de-asserted */
} tps2386b_callout_fvt_t;

/*==========[TPS2386B device object structure]========== */
typedef struct tps2386b_object_t {
    dev_object_t             base;
    tps2386b_callin_fvt_t    *callin_fvt;
    tps2386b_callout_fvt_t   *callout_fvt;
    n2g_i2c_if_t             *i2c_p; /* I2C API interace pointer */
    reg_info_t               *reg_p; /* Register table pointer */
} dev_tps2386b_object_t;

/* Externs */
extern int tps2386b_dev_create(dev_object_t *, dev_error_report_t);

/* TPS2386B Register map */
#define TPS2386B_INTR_REG                0x00
#define TPS2386B_INTR_MASK_REG           0x01
#define TPS2386B_PWREVENT_REG            0x02
#define TPS2386B_PWREVENT_COR_REG        0x03
#define TPS2386B_DET_REG                 0x04
#define TPS2386B_DET_COR_REG             0x05
#define TPS2386B_FAULTEVENT_REG          0x06
#define TPS2386B_FAULTEVENT_COR_REG      0x07
#define TPS2386B_STARTEVENT_REG          0x08
#define TPS2386B_STARTEVENT_COR_REG      0x09
#define TPS2386B_SUPPLYEVENT_REG         0x0A
#define TPS2386B_SUPPLYEVENT_COR_REG     0x0B
#define TPS2386B_P1_STATUS_REG           0x0C
#define TPS2386B_P2_STATUS_REG           0x0D
#define TPS2386B_P3_STATUS_REG           0x0E
#define TPS2386B_P4_STATUS_REG           0x0F
#define TPS2386B_PWR_STAT_REG            0x10
#define TPS2386B_I2C_SLVADDR_REG         0x11
#define TPS2386B_OP_MODE_REG             0x12
#define TPS2386B_DISCONN_EN_REG          0x13
#define TPS2386B_DETCLA_EN_REG           0x14
#define TPS2386B_PORT_PWR_DIS_REG        0x15
#define TPS2386B_TIMING_CONF_REG         0x16
#define TPS2386B_GENERAL_MASK_REG        0x17
#define TPS2386B_DETCLA_RESTART_REG      0x18
#define TPS2386B_PWR_EN_REG              0x19
#define TPS2386B_RESET_REG               0x1A
#define TPS2386B_ID_REG                  0x1B
#define TPS2386B_POLICE21_CONFIG_REG     0x1E
#define TPS2386B_POLICE43_CONFIG_REG     0x1F
#define TPS2386B_IEEE_PWR_ENA_REG        0x23
#define TPS2386B_PWRON_FAULT_REG         0x24
#define TPS2386B_PWRON_FAULT_COR_REG     0x25
#define TPS2386B_TEMP_REG                0x2C
#define TPS2386B_IN_VOLT_LSB_REG         0x2E
#define TPS2386B_IN_VOLT_MSB_REG         0x2F
#define TPS2386B_P1_CURR_LSB_REG         0x30
#define TPS2386B_P1_CURR_MSB_REG         0x31
#define TPS2386B_P1_VOLT_LSB_REG         0x32
#define TPS2386B_P1_VOLT_MSB_REG         0x33
#define TPS2386B_P2_CURR_LSB_REG         0x34
#define TPS2386B_P2_CURR_MSB_REG         0x35
#define TPS2386B_P2_VOLT_LSB_REG         0x36
#define TPS2386B_P2_VOLT_MSB_REG         0x37
#define TPS2386B_P3_CURR_LSB_REG         0x38
#define TPS2386B_P3_CURR_MSB_REG         0x39
#define TPS2386B_P3_VOLT_LSB_REG         0x3A
#define TPS2386B_P3_VOLT_MSB_REG         0x3B
#define TPS2386B_P4_CURR_LSB_REG         0x3C
#define TPS2386B_P4_CURR_MSB_REG         0x3D
#define TPS2386B_P4_VOLT_LSB_REG         0x3E
#define TPS2386B_P4_VOLT_MSB_REG         0x3F
#define TPS2386B_POEPLUS_REG             0x40
#define TPS2386B_FW_REV_REG              0x41
#define TPS2386B_I2C_WD_REG              0x42
#define TPS2386B_DEV_ID_REG              0x43
#define TPS2386B_COOL_DOWN_REG           0x45

#define TPS2386B_STAT_REG_RST_VALUE      0x0   /* after reset, status register
                                                * (0x0C/0x0D/0x0E/0x0F) should
                                                * be 0x0 */
#define TPS2386B_ALL_PORT_RST            0x10  /* set RESET register(0x1A) 
                                                * RESAL field bit[4] as 1
                                                * to reset all port */

#define TPS2386B_START_REG               TPS2386B_INTR_REG       /* 0x0  */
#define TPS2386B_END_REG                 TPS2386B_COOL_DOWN_REG  /* 0x45 */

#define TPS2386B_ENA_SUPEN_INTR           0x80 /* Interrupt Enable,
                                                  Register(0x1) bit[8] = 1 */
#define TPS2386B_DIS_SUPEN_INTR           0x0  /* Interrupt Disable,
                                                  Register(0x1) bit[8] = 0 */

#define DEFAULT_OP_MODE   0
#define DEFAULT_DETECTION 0
#define DEFAULT_CLASS     0
#define PORT_PWR_OFF      0
#define PORT_PWR_ON       1
#define PORT_PORT_OFF     0
#define PORT_PORT_ON      1
#define NO_CURRENT        0.0
#define NO_VOLTAGE        0.0
#define NO_POWER          0.0

#define TPS2386B_ALL_4PORT 4
#define TPS2386B_PORT1 1
#define TPS2386B_PORT2 2
#define TPS2386B_PORT3 3
#define TPS2386B_PORT4 4
/* TRUE if input port = 2 or 1 */
#define TPS2386B_PORT21(PORT) ((PORT == TPS2386B_PORT2) || \
                               (PORT == TPS2386B_PORT1))
/* TRUE if input port = 4 or 3 */
#define TPS2386B_PORT43(PORT) ((PORT == TPS2386B_PORT4) || \
                               (PORT == TPS2386B_PORT3))
/* TRUE if input port = 3 or 1 */
#define TPS2386B_PORT31(PORT) ((PORT == TPS2386B_PORT3) || \
                               (PORT == TPS2386B_PORT1))

#define REG_MASK_FOR_OP_MODE   0x03 /* operation mode register(0x12) mask */
#define REG_MASK_FOR_DETECTION 0x0f /* P1~P4 status register 
                                     * (0x0C/0x0D/0x0E/0x0F) detection field 
                                     * mask */
#define REG_MASK_FOR_CLASS     0xf0 /* P1~P4 status register mask
                                     * (0x0C/0x0D/0x0E/0x0F) class field mask */
#define REG_MASK_FOR_PG_PE     0x11 /* power status register(0x12)
                                     * power good and on/off mask */
#define REG_MASK_FOR_PG        0x01 /* power status register(0x12) 
                                     * power good mask */
#define REG_MASK_FOR_PE        0x01 /* power status register(0x12) 
                                     * port On/Off mask */

/* port range:1~4, outcome range:0x0C/0x0D/0x0E/0x0F */
#define REG_SHIFT_STATUS_REG(PORT)      (TPS2386B_P1_STATUS_REG + PORT - 1)

/* port range:1~4, outcome range:0x30/0x34/0x38/0x3C */
#define REG_SHIFT_CURRENT_REG_LSB(PORT) (TPS2386B_P1_CURR_LSB_REG + \
                                         4 * (PORT - 1))
/* port range:1~4, outcome range:0x31/0x35/0x39/0x3D */
#define REG_SHIFT_CURRENT_REG_MSB(PORT) REG_SHIFT_CURRENT_REG_LSB(PORT) + 1

/* port range:1~4, outcome range:0x32/0x36/0x3A/0x3E */
#define REG_SHIFT_VOLTAGE_REG_LSB(PORT) (TPS2386B_P1_VOLT_LSB_REG + \
                                         4 * (PORT - 1))
/* port range:1~4, outcome range:0x33/0x37/0x3B/0x3F */
#define REG_SHIFT_VOLTAGE_REG_MSB(PORT) REG_SHIFT_VOLTAGE_REG_LSB(PORT) + 1

#define PORT_PLUS_TPON          0x1
#define ENA_PORT_PLUS_BASE     0x10 /* only bit 4 is 1 */
/* port range:1~4, outcome range:0x10/0x20/0x40/0x80 */
#define REG_SHIFT_PORT_ENA_PLUS(x) (ENA_PORT_PLUS_BASE << (x - 1))
#define REG_SHIFT_PORT_DIS_PLUS(x) (~REG_SHIFT_PORT_ENA_PLUS(x))

#define REG_SHIFT_CLASS         4          /* bit shift for class field */
#define REG_SHIFT_PG(PORT)      (PORT + 3) /* port range:1~4, shift range:4~7 */
#define REG_SHIFT_PE(PORT)      (PORT - 1) /* port range:1~4, shift range:0~3 */
#define REG_SHIFT_MSB_CURRENT   8          
#define REG_SHIFT_MSB_VOLTAGE   8          

#define TPS2386B_MAX_POLLING            50 /* polling 50 rounds */
#define TPS2386B_POLLING_INTERVAL      200 /* 200ms */
#define TPS2386B_WAIT_PD_READY_MAX       5 /* polling 5 rounds */
#define TPS2386B_WAIT_PD_READY_INTVAL 1000 /* 1000ms */
#define TPS2386B_RST_WAIT_TIME          10 /* 10ms */

#define CURRENT_RESOLUTION          61.035 /* current resolution */
#define VOLTAGE_RESOLUTION          3.662  /* voltage resolution */

#define CURRENT_UNI_TRANS           1000   /* uni translation from uA to mA */
#define VOLTAGE_UNI_TRANS           1000   /* uni translation from mV to V */
#define POWER_UNI_TRANS             1000   /* uni translation from mW to W */

/* Operation mode define */
#define CFG_ALL_PORT_SEMI_AUTO_MODE 0xAA
#define OP_MODE_OFF       0x0
#define OP_MODE_MANUAL    0x1
#define OP_MODE_SEMIAUTO1 0x2
#define OP_MODE_SEMITUAO2 0x3
#define CHECK_OP_MODE(MODE) ((MODE == OP_MODE_OFF) ||  \
                             (MODE == OP_MODE_MANUAL) || \
                             (MODE == OP_MODE_SEMIAUTO1) || \
                             (MODE == OP_MODE_SEMITUAO2))

/* Port ON/OFF define */
#define CHECK_PORT_ONOFF(X)  ((X == PORT_PORT_ON) || (X == PORT_PORT_OFF))

/* Detection and Class define */
#define ENA_ALL_PORT_DETECTION_CLASS 0xFF
/* Detection define */
#define DETECTION_UNKNOWN       0x0
#define DETECTION_SHORT_CIRCUIT 0x1
#define DETECTION_RSV1          0x2
#define DETECTION_TOO_LOW       0x3
#define DETECTION_VALID         0x4
#define DETECTION_TOO_HIGH      0x5
#define DETECTION_OPEN_CIRCUIT  0x6
#define DETECTION_RSV2          0x7
#define DETECTION_MOS_FAULT     0xE
#define CHECK_DETECTION(DET)   ((DET == DETECTION_UNKNOWN) || \
                                (DET == DETECTION_SHORT_CIRCUIT) || \
                                (DET == DETECTION_RSV1) || \
                                (DET == DETECTION_TOO_LOW) || \
                                (DET == DETECTION_VALID) || \
                                (DET == DETECTION_TOO_HIGH) || \
                                (DET == DETECTION_OPEN_CIRCUIT) || \
                                (DET == DETECTION_RSV2) || \
                                (DET == DETECTION_MOS_FAULT))
#define CHECK_DETECT_VALID(DET) (DET == DETECTION_VALID)

/* Class define */
#define CLASS_UNKNOWN     0x0
#define CLASS_CLASS1      0x1
#define CLASS_CLASS2      0x2
#define CLASS_CLASS3      0x3
#define CLASS_CLASS4      0x4
#define CLASS_RSV         0x5
#define CLASS_CLASS0      0x6
#define CLASS_OVERCURRENT 0x7
#define CHECK_CLASS_VALID(CLASS) (CHECK_CLASS_IS_0TO3(CLASS) || \
                                  CHECK_CLASS_IS_4(CLASS))
#define CHECK_CLASS_IS_4(CLASS)     (CLASS == CLASS_CLASS4)
#define CHECK_CLASS_IS_0TO3(CLASS) ((CLASS == CLASS_CLASS0) || \
                                    (CLASS == CLASS_CLASS1) || \
                                    (CLASS == CLASS_CLASS2) || \
                                    (CLASS == CLASS_CLASS3))
#define CHECK_CLASS(CLASS) ((CHECK_CLASS_VALID(CLASS)) || \
                            (CLASS == CLASS_UNKNOWN) || \
                            (CLASS == CLASS_RSV) || \
                            (CLASS == CLASS_OVERCURRENT))

#define CLR_POL_BASE 0x0f
/* if port = 1 or 3, outcome = 0xf0, clear bit[3:0]; 
 * if port = 2 or 4, outcome = 0x0f, clear bit[7:4]*/
#define REG_SHIFT_CLR_POL(PORT) (CLR_POL_BASE << (((PORT) % 2) * 4))
#define ICUT_POL_320MA 0xf
#define ICUT_POL_640MA 0x8
#define REG_SHIFT_POL_DATA 4

#define ENA_IEEE_PWR_T2_BASE 0x10
#define ENA_IEEE_PWR_T1_BASE 0x01
/* port range:1~4, outcome range:0x10/0x20/0x40/0x80 */
#define ENA_IEEE_PWR_T2(PORT) (ENA_IEEE_PWR_T2_BASE << (PORT -1))
/* port range:1~4, outcome range:0x01/0x02/0x04/0x08 */
#define ENA_IEEE_PWR_T1(PORT) (ENA_IEEE_PWR_T1_BASE << (PORT -1))
/* port range:1~4, outcome range:0x11/0x22/0x44/0x88 */
#define REG_SHIFT_PWR_STAT(PORT) (REG_MASK_FOR_PG_PE << (PORT - 1))

/* for register test */
#define END_VAL            0x0
#define REG_MASK_INTR_MASK 0xFF
#define REG_MASK_OP_MODE   0xFF
#define INI_VAL_INTR_MASK  0x80
#define INI_VAL_OP_MODE    0x0

#define INFO_SIZE_OP_MODE 4
#define INFO_SIZE_ONOFF   2
#define INFO_SIZE_DET     9
#define INFO_SIZE_CLASS   8
#define INFO_TB static const char

#define TPS2386B_POLLING_ROUND    10 /* 10 rounds */
#define TPS2386B_POLLING_PERIOD  500 /* 500ms */
#define STATUS_POLLING_ROUND      10 /* 10 rounds */
#define STATUS_POLLING_PERIOD    100 /* 100ms */
#define COMPARE_AND     0 /* read data & pattern == 0       */
#define COMPARE_EQL     1 /* read data           == pattern */
#define COMPARE_AND_EQL 2 /* read data & pattern == pattern */

#define INTR_POLLING_ROUND  100 /* 100 rounds */
#define INTR_POLLING_PERIOD  10 /* 10ms */

#define TPS2386B_ERR_BUF_SIZE       (80)

#define TPS2386B_REG_ONE_BYTE_ACCESS 1 /* 1 byte */

/* TPS2386B register Read/Write flag */
#define TPS2386B_REG_RO_FLAG   (READ_ONLY | REG_ACCESS)
#define TPS2386B_REG_RW_FLAG   (READ_WRITE| SAVE_RESTORE | REG_ACCESS)

/*-------------------------------------------------
 * $Log: dev_tps2386b.h,v $
 * Revision 1.2  2019/01/10 06:30:25  wilbhuan
 * The beginning of TI TPS2386B PoE PSE Controller device driver.
 *
 *-------------------------------------------------
 */
