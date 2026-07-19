/* $Id: highrise_cpld_lib.h,v 1.1 2020/08/19 09:49:35 markzha Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/highrise/highrise_cpld_lib.h,v $
 *******************************************************************************
 * File Name: highrise_cpld_lib.h
 *
 * Description: Highrise CPLD header file
 *
 * Author: Mingchun Ding
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *******************************************************************************
 */

#ifndef VM_HIGHRISE_CPLD_LIB_H
#define VM_HIGHRISE_CPLD_LIB_H

#define HIGHRISE_CPLD_I2C_ADDR       HR_I2C0_ADDR_22

/* Define CPLD register offset */
typedef enum {
    CPLD_NGIO_EXPANDER_INPUT = 0,
    CPLD_NGIO_EXPANDER_OUTPUT,
    CPLD_RESERVED,
    CPLD_NGIO_EXPANDER_DIR,
    CPLD_ZL30363_CONT_STS,
    CPLD_POW_SEQ_STS,
    CPLD_JTAG_CTL,
    CPLD_VERSION,
    CPLD_GPIO_DIRECTION,
    CPLD_SPEED_UP,
    CPLD_TEN,
    CPLD_ELEVEN,
    CPLD_TWELVE,
    CPLD_THIRTEEN,
    CPLD_FOURTEEN,
    CPLD_FIFTEEN,
} cpld_offset_t;

typedef enum {
    HR_CPLD_VERSION             = 0x00,
    HR_CPLD_VERSION_DATE        = 0x04,
    HR_CPLD_BOARD_ID            = 0x08,
#define HR_CPLD_BOARD_ID_MSK            0x1f
#define HR_CPLD_BOARD_NONE              0
#define HR_CPLD_BOARD_HIGHTOWER         1
#define HR_CPLD_BOARD_HIGHRISE_5G       2
#define HR_CPLD_BOARD_HIGHRISE_CAT18    3
    HR_CPLD_PWR_STATUS          = 0x0c,
#define HR_CPLD_PWR_PD_CLASS            0x00007000
#define HR_CPLD_PWR_POE_GOOD            0x00000800
#define HR_CPLD_PWR_POE_DET             0x00000400
#define HR_CPLD_PWR_INT_3300S           0x00000200
#define HR_CPLD_PWR_INT_1800S           0x00000100
#define HR_CPLD_PWR_INT_3300            0x00000080
#define HR_CPLD_PWR_INT_2500            0x00000040
#define HR_CPLD_PWR_INT_1800            0x00000020
#define HR_CPLD_PWR_INT_1500            0x00000010
#define HR_CPLD_PWR_INT_1200            0x00000008
#define HR_CPLD_PWR_INT_0900            0x00000004
#define HR_CPLD_PWR_INT_0800            0x00000002
#define HR_CPLD_PWR_INT_GASP            0x00000001
    HR_CPLD_MODEM_STATUS        = 0x10,
#define HR_CPLD_MODEM_STA_WAKE_ON_WLAN  0x00000200
#define HR_CPLD_MODEM_STA_TGPIO         0x00000100
#define HR_CPLD_MODEM_STA_VREG_PWR_ON   0x00000080
#define HR_CPLD_MODEM_STA_PRESET        0x00000040
#define HR_CPLD_MODEM_STA_WWLAN         0x00000010
#define HR_CPLD_MODEM_STA_ACTIVE        0x00000008
#define HR_CPLD_MODEM_STA_PWR_ON        0x00000004
#define HR_CPLD_MODEM_STA_PWR_PROT_ENA  0x00000002
#define HR_CPLD_MODEM_STA_PWR_PROT_DIS  0x00000001
    HR_CPLD_ERROR_TEST          = 0x14,
#define HR_CPLD_PWR_TST_CPU_DECC        0x00000400
#define HR_CPLD_PWR_TST_3300S           0x00000200
#define HR_CPLD_PWR_TST_1800S           0x00000100
#define HR_CPLD_PWR_TST_3300            0x00000080
#define HR_CPLD_PWR_TST_2500            0x00000040
#define HR_CPLD_PWR_TST_1800            0x00000020
#define HR_CPLD_PWR_TST_1500            0x00000010
#define HR_CPLD_PWR_TST_1200            0x00000008
#define HR_CPLD_PWR_TST_0900            0x00000004
#define HR_CPLD_PWR_TST_0800            0x00000002
#define HR_CPLD_PWR_TST_GASP            0x00000001
    HR_CPLD_RESET_CTRL          = 0x18,
#define HR_CPLD_BTN_RST_CNTR            0x0000ff00
#define HR_CPLD_UNRESET_WDOG            0x00000080
#define HR_CPLD_UNRESET_MODEM_PCIE      0x00000040
#define HR_CPLD_UNRESET_MODEM           0x00000020
#define HR_CPLD_UNRESET_ACT2            0x00000010
#define HR_CPLD_UNRESET_EMMC            0x00000008
#define HR_CPLD_UNRESET_DDR4            0x00000004
#define HR_CPLD_BTN_RST_CNTR_CLR        0x00000002
#define HR_CPLD_CPU_REST_ENB            0x00000001
    HR_CPLD_RESET_PROTECT       = 0x1c,
#define HR_CPLD_RESET_LOCK_MODEM        0x00000020
#define HR_CPLD_RESET_LOCK_ACT2         0x00000010
#define HR_CPLD_RESET_LOCK_EMMC         0x00000008
#define HR_CPLD_RESET_LOCK_DDR4         0x00000004
#define HR_CPLD_RESET_LOCK_CPU          0x00000001
    HR_CPLD_CPU_RESET_TRIGGER   = 0x20,
#define HR_CPLD_CPU_RESET_TRIGGER_MSK   0x00000001
    HR_CPLD_CPU_BOOT_STATUS     = 0x24,
#define HR_CPLD_CPU_BOOT_STA_WLAN       0x00000008
#define HR_CPLD_CPU_BOOT_STA_GE         0x00000004
#define HR_CPLD_CPU_BOOT                0x00000002
#define HR_CPLD_CPU_BOOT_SECURE         0x00000001
    HR_CPLD_MODEM_CTRL          = 0x28,
#define HR_CPLD_MODEM_CONFG             0x00000078
#define HR_CPLD_MODEM_CTRL_DPR          0x00000004
#define HR_CPLD_MODEM_CTRL_GNSS         0x00000002
#define HR_CPLD_MODEM_CTRL_RADIO        0x00000001
    HR_CPLD_LED_CTRL            = 0x2c,
#define HR_CPLD_LED_R_4G                0x00001000
#define HR_CPLD_LED_G_4G                0x00000400
#define HR_CPLD_LED_B_4G                0x00000100
#define HR_CPLD_LED_R_5G                0x00000040
#define HR_CPLD_LED_G_5G                0x00000010
#define HR_CPLD_LED_B_5G                0x00000004
#define HR_CPLD_LED_WWAN                0x00000002
#define HR_CPLD_LED_SW_EN               0x00000001
#define HR_CPLD_LED_MODE_OFF            0x00000000
#define HR_CPLD_LED_MODE_CLR            0x40000000
#define HR_CPLD_LED_MODE_BLINK          0x80000000
#define HR_CPLD_LED_MODE_RED            HR_CPLD_LED_R_5G
#define HR_CPLD_LED_MODE_GREEN          HR_CPLD_LED_G_5G
#define HR_CPLD_LED_MODE_BLUE           HR_CPLD_LED_B_5G
#define HR_CPLD_LED_MODE_YELLOW         (HR_CPLD_LED_R_5G|HR_CPLD_LED_G_5G)
#define HR_CPLD_LED_MODE_PINK           (HR_CPLD_LED_R_5G|HR_CPLD_LED_B_5G)
#define HR_CPLD_LED_MODE_CYAN           (HR_CPLD_LED_G_5G|HR_CPLD_LED_B_5G)
#define HR_CPLD_LED_MODE_WHITE          (HR_CPLD_LED_R_5G|HR_CPLD_LED_G_5G|HR_CPLD_LED_B_5G)
#define HR_CPLD_LED_COLOR_MASK          HR_CPLD_LED_MODE_WHITE
#define HR_CPLD_LED_TYPE_NULL           0x0000
#define HR_CPLD_LED_TYPE_5G             (1 << 0)
#define HR_CPLD_LED_TYPE_4G             (1 << 1)
    HR_CPLD_INT_ENABLE          = 0x40,
#define HR_CPLD_INT_EN_WOL              0x00000004
#define HR_CPLD_INT_EN_PWR_ERR          0x00000002
#define HR_CPLD_INT_EN_CPU_ERR          0x00000001
    HR_CPLD_INT_STATUS          = 0x44,
#define HR_CPLD_INT_STA_WOL             0x00000004
#define HR_CPLD_INT_STA_PWR_ERR         0x00000002
#define HR_CPLD_INT_STA_CPU_ERR         0x00000001
    HR_CPLD_DEBUG               = 0x48,
    HR_CPLD_SCRATCHPAD          = 0x4c,
    HR_CPLD_PHY_STATUS          = 0x50,
#define HR_CPLD_PHY_STA_WOL_GPIO        0x000002C0
#define HR_CPLD_PHY_STA_WOL_INT_EN      0x00000020
#define HR_CPLD_PHY_STA_PORT_INT_EN     0x00000010
#define HR_CPLD_PHY_STA_WOL_EN          0x00000008
#define HR_CPLD_PHY_STA_WOL             0x00000004
#define HR_CPLD_PHY_STA_INT             0x00000002
#define HR_CPLD_PHY_STA_WOL_CLEAR       0x00000001
    HR_CPLD_MAC0_ADDR           = 0x54,
    HR_CPLD_MAC1_ADDR           = 0x58
} highrise_cpld_regs_addr;

/* FIXME, only for pass compile */
/* Define PCA9557 register offset */
typedef enum {
    PCA9557_NGIO_EXPANDER_INPUT = 0,
    PCA9557_NGIO_EXPANDER_OUTPUT,
    PCA9557_POLARITY_INVERSION,
    PCA9557_CONFIGURATION,
} pca9557_offset_t;
/* FIXME END */

/* JTAG Control (Offset 6) */
#define CPLD_JTAG_ON                (0x1 << 4)
#define CPLD_JTAG_TCK_ST            (0x1 << 3)
#define CPLD_JTAG_TMS_ST            (0x1 << 2)
#define CPLD_TDI_ST                 (0x1 << 1)
#define CPLD_TDO_ST                 (0x1 << 0)

/* ZL30363 and Control Status (Offset 4) */
#define CPLD_GPIO_0                 (0x1)
#define CPLD_GPIO_1                 (0x1 << 1)
#define CPLD_GPIO_2                 (0x1 << 2)
#define CPLD_GPIO_3                 (0x1 << 3)
#define CPLD_GPIO_4                 (0x1 << 4)
#define CPLD_GPIO_5                 (0x1 << 5)
#define CPLD_GPIO_6                 (0x1 << 6)

/* ZL30363 Expander Output (Offset 1) */
#define ZL30363_RESET_ACTIVE_HIGH   (0x1 << 5)

#define IOCPLD_VER_LEN              (8)
#define MAX_NOTE_LEN                (257)

/* CPLD Program */
#define CPLD_ERASE_COMMAND          0x013d
#define CPLD_SAMPLE_PRELOAD         0x0280
#define CPLD_ISP_ENABLE             0x0266
#define CPLD_ADDRESS_SHIFT          0x0301
#define CPLD_ISP_READ               0x0281
#define CPLD_ISC_PROG               0x00bd
#define CPLD_ISC_PROG_DONE_1        0xFFDF
#define CPLD_ISC_PROG_DONE_2        0xFFFF
#define CPLD_ISP_DISABLE            0x019a
#define CPLD_BYPASS                 0x03ff

#define PCA9557_TDI_GPIO2_OUTPUT            (0x1 << 2)
#define PCA9557_TDI_GPIO4_OUTPUT            (0x1 << 4)
#define PCA9557_TDO_GPIO5_INPUT             (0x1 << 5)
#define PCA9557_TCK_GPIO6_OUTPUT            (0x1 << 6)
#define PCA9557_TMS_GPIO7_OUTPUT            (0x1 << 7)

#define JBI_RETURN_TYPE int

#define JBIC_SUCCESS            0
#define JBIC_OUT_OF_MEMORY      1
#define JBIC_IO_ERROR           2
/* #define JAMC_SYNTAX_ERROR       3 */
#define JBIC_UNEXPECTED_END     4
#define JBIC_UNDEFINED_SYMBOL   5
/* #define JAMC_REDEFINED_SYMBOL   6 */
#define JBIC_INTEGER_OVERFLOW   7
#define JBIC_DIVIDE_BY_ZERO     8
#define JBIC_CRC_ERROR          9
#define JBIC_INTERNAL_ERROR    10
#define JBIC_BOUNDS_ERROR      11
/* #define JAMC_TYPE_MISMATCH     12 */
/* #define JAMC_ASSIGN_TO_CONST   13 */
/* #define JAMC_NEXT_UNEXPECTED   14 */
/* #define JAMC_POP_UNEXPECTED    15 */
/* #define JAMC_RETURN_UNEXPECTED 16 */
/* #define JAMC_ILLEGAL_SYMBOL    17 */
#define JBIC_VECTOR_MAP_FAILED 18
#define JBIC_USER_ABORT        19
#define JBIC_STACK_OVERFLOW    20
#define JBIC_ILLEGAL_OPCODE    21
/* #define JAMC_PHASE_ERROR       22 */
/* #define JAMC_SCOPE_ERROR       23 */
#define JBIC_ACTION_NOT_FOUND  24
typedef struct JBI_PROCINFO_STRUCT
{
	char *name;
	unsigned char attributes;
	struct JBI_PROCINFO_STRUCT *next;
} JBI_PROCINFO;

#define PROGRAM_PTR unsigned char *
JBI_RETURN_TYPE jbi_get_file_info
(
	PROGRAM_PTR program,
	int program_size,
	int *format_version,
	int *action_count,
	int *procedure_count
);

JBI_RETURN_TYPE jbi_get_action_info
(
	PROGRAM_PTR program,
	int program_size,
	int index,
	char **name,
	char **description,
	JBI_PROCINFO **procedure_list
);
JBI_RETURN_TYPE jbi_execute
(
	PROGRAM_PTR program,
	int program_size,
	char *workspace,
	int workspace_size,
	char *action,
	char **init_list,
	int reset_jtag,
	int *error_address,
	int *exit_code,
	int *format_version
);

JBI_RETURN_TYPE jbi_get_note
(
	PROGRAM_PTR program,
	int program_size,
	int *offset,
	char *key,
	char *value,
	int length
);

extern long util_oir_cpld_reg_read(void);
extern long util_oir_cpld_reg_write(void);
extern long hr_cpld_reg_test_lib(void);
extern long hr_cpld_reg_read_lib(uchar, uchar *);
extern long hr_cpld_reg_write_lib(uchar, uchar);
extern long hr_cpld_reg_read_32(unsigned long, unsigned long *);
extern long hr_cpld_reg_write_32(unsigned long, unsigned long);
extern long hr_cpld_jtag_ctrl(boolean);
extern long display_embedded_cpld_fw_version(uchar *);
extern long cpld_jtag_io(int, int, int);
extern long max2_cpld_program(void);
extern long hr_simply_program_cpld(unsigned char *);
extern long hr_cpld_set_wol_mac_addr(uchar *, int);
extern long hr_cpld_get_wol_mac_addr(uchar *, int);
extern long hr_cpld_set_led(unsigned long, unsigned long);
extern int hr_cpld_get_boardid(uint8_t *id, char *name);
extern int hr_cpld_get_version(uint16_t *ver_numb, uint16_t *ver_date);
extern int hr_cpld_get_sys_info(char *buf, int size);
extern long hr_cpld_intr_enable(unsigned long enb_msk);
extern long hr_cpld_intr_status(unsigned long *status, unsigned long *enb_msk);


#endif /* VM_HIGHRISE_CPLD_LIB_H */

/******** History ********/
/*------------------------------------------------------------------------------
 * $Log: highrise_cpld_lib.h,v $
 * Revision 1.1  2020/08/19 09:49:35  markzha
 * *** empty log message ***
 *
 *
 *------------------------------------------------------------------------------
 * $Endlog $
 *------------------------------------------------------------------------------
 */
