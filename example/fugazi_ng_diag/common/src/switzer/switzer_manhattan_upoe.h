/*--------------------------------------------------------------------------
 * switzer_manhattan_poe.h - This file contains defintions for
 *                      Switzer 2.5G POE Controller
 * Xuanyu Shi - Jan 2020
 *------------------------------------------------------------------------------------
 */

#ifndef  __SWITZER_MANHATTAN_UPOE_H__
#define  __SWITZER_MANHATTAN_UPOE_H__

#define UPOE_PORTS                     2
#define INTR_MASKED_ALL                0x80
#define PWR_ENABLE                     0xff
#define PWR_STAUS_CHG_INTR             0x02
#define SINGLE_SIGNATURE_PD            0x01
#define DUAL_SIGNATURE_PD              0x02
#define UNKNOWN_PD                     0x00
#define AUTO_CLASS_MASK                0xf0
#define SIGNATURE_PD_MASK              0x0f

#define I2C_CONFIGURATION_A_MASK       0x0
#define I2C_CONFIGURATION_B_MASK       0x20
#define INTEN_MASK                     0x80
#define MFR_ID_SHIFT                   3
#define IC_ID_MASK                     0x07
#define MFR_TI_ID                      0x0A
#define IC_PALPATINE_II                0x05

#define UPOE_DATA_SHIFT                4
#define UPOE_DATA_MASK                 0x0f
#define TWO_BITS_PER_CHNL              2
#define ONE_BIT_PER_CHNL               1
#define PD_PORT_ON                     0xff
#define PD_PORT_OFF                    0x00
#define SET_FOUR_PAIR_MODE             0x8
#define SET_ENABLE_BIT                 0x01
#define UPOE_PORT_1                    0
#define UPOE_PORT_2                    1
#define UPOE_PORT_3                    2
#define UPOE_PORT_4                    3
#define PSE_CHANNEL_1                  0
#define PSE_CHANNEL_2                  1
#define PSE_CHANNEL_3                  2
#define PSE_CHANNEL_4                  3
#define UPOE_I2C_0                     0
#define UPOE_I2C_1                     1

/* FAULT VALUES */
#define SUPPLY_EVENT_FAULT             0x80
#define INRUSH_FAULT                   0x40
#define IFAULT_EVENT                   0x20
#define CLASSIFICATION_CYCYLE          0x10
#define DISCONNECTION_EVENT            0x04
#define DETECTION_CYCYLE               0x08
#define MASKED_4P_VALUE                0x03
#define SHIFT_4P_PORT                  0x02
#define THERMAL_SHUTDOWN               0x80
#define VDD_UVLO                       0x40
#define VDD_UNDERVOLTAGE_WARNING       0x20
#define VPWR_VOLTAGE_OCCUR             0x10
#define OSS_EVENT_OCCUR                0x02
#define SRAM_FAULT_OCCUR               0x01

#define REGISTER_OPERATION_LEFT_OR(portNum, value, bits)  \
    ((value << (port_mapping[portNum].phy_chn1 * bits)) |\
     (value << (port_mapping[portNum].phy_chn2 * bits)))

#define REGISTER_OPERATION_RIGHT_AND(portNum, value, bits) \
    ((value >> (port_mapping[portNum].phy_chn1 * bits)) & \
     (value >> (port_mapping[portNum].phy_chn2 * bits)))


/*portmap struct*/
typedef struct {
    uint8_t portNum;
    uint8_t i2c_num;
    uint8_t phy_chn1;
    uint8_t phy_chn2;
}UpoePortMap_t;

typedef struct {
    uint32      status;
    uint32      config;
    uint16      detectResult;
    uint16      classResult;
    uint32      fault;
    uint8       connResult;
} pwrStatus_t;

typedef enum {
    CLASS_UNKNOWN                  = 0x0,
    CLASS_1                        = 0x01,
    CLASS_2                        = 0x02,
    CLASS_3                        = 0x03,
    CLASS_4                        = 0x04,
    CLASS_0                        = 0x06,
    CLASS_OVERCURRENT              = 0x07,
    CLASS_5                        = 0x08, // 4-pair signature single signature
    CLASS_6                        = 0x09, // 4-pair signature single signature
    CLASS_7                        = 0x0A, // 4-pair signature single signature
    CLASS_8                        = 0x0B, // 4-pair signature single signature
    CLASS_4_PLUS                   = 0x0C, // class type 4 plus type 1 limited
    CLASS_5_DUAL                   = 0x0D, // Class 5 4-pair dual signature
    CLASS_MISMATCH                 = 0x0F   // CLASS MISMATCH
}classStatus_t;


typedef enum {
    UNKNOWN_DETECTION              = 0x0,
    SHORT_CIRCUIT_DETECTION        = 0x01,
    TOO_LOW_DETECTION              = 0x03,
    VALID_DETECTION                = 0x04,
    TOO_HIGH_DETECTION             = 0x05,
    OPEN_CIRCUIT_DETECTION         = 0x06,
    MOSFET_FAULT_DETECTION         = 0x0e
}detStatus_t;

typedef enum {
     OFF_MODE                      = 0x0,
     MANUAL_MODE                   = 0x01,
     SEMI_AUTO_MODE                = 0x02,
     AUTO_MODE                     = 0x03
}operating_mode_t;

typedef enum {
    NO_FAULT                       = 0x0,
    INVALID_DETECTION              = 0x1,
    CLASSIFICATION_ERROR           = 0x2,
    INSUFFICIENT_POWER             = 0x3
}power_on_fault_t;

typedef enum {
    _4P_15W                        = 0x00,
    _4P_30W                        = 0x03,
    _4P_45W                        = 0x04,
    _4P_60W                        = 0x05,
    _4P_75W                        = 0x06,
    _4P_90W                        = 0x07
}pwr_allocation_t;

int switzer_pse_register_read(struct switzer_manhattan *mod, uint8_t port_num, uint8_t reg);
int switzer_pse_register_write(struct switzer_manhattan *mod, uint8_t port_num, uint8_t reg, uint8_t data);
int switzer_upoe_set_general_mask(struct switzer_manhattan *mod, uint8_t port_num, uint8_t value);
int switzer_upoe_get_general_mask(struct switzer_manhattan *mod, uint8_t port_num, uint8_t *value);
int switzer_upoe_pd_connection_check(void);
int switzer_display_port_mapping(void);
int switzer_upoe_set_operating_mode(struct switzer_manhattan *mod, uint8_t port_num, operating_mode_t mode);
int switzer_upoe_get_classification_detection_status(struct switzer_manhattan *mod, uint8_t port_num,
                                                         classStatus_t *classStatus, detStatus_t *detectStatus);
int switzer_upoe_get_port_pwr_good_status(struct switzer_manhattan *mod, uint8_t port_num);
int switzer_upoe_get_port_pwr_enable_status(struct switzer_manhattan *mod, uint8_t port_num);
int switzer_upoe_set_detection_enable (struct switzer_manhattan *mod, uint8_t port_num);
int switzer_upoe_set_classification_enable (struct switzer_manhattan *mod, uint8_t port_num);
int switzer_upoe_set_port_Pcut_disable(struct switzer_manhattan *mod, uint8_t port_num);
int switzer_upoe_set_timing_configuration();
int switzer_upoe_restart_detection (struct switzer_manhattan *mod, uint8_t port_num);
int  switzer_upoe_restart_classification (struct switzer_manhattan *mod, uint8_t port_num);
int switzer_upoe_set_port_one_bit_oss(struct switzer_manhattan *mod, uint8_t port_num);
int switzer_upoe_set_detection_classification_enable(struct switzer_manhattan *mod, uint8_t port_num);
int switzer_upoe_set_port_dc_disconnect_enable(struct switzer_manhattan *mod, uint8_t port_num, int disconnect_enable);
int switzer_upoe_set_port_power (struct switzer_manhattan *mod, uint8_t port_num, int on_off);
int switzer_upoe_set_port_reset (struct switzer_manhattan *mod, uint8_t port_num);
int switzer_upoe_set_legacy_detection(struct switzer_manhattan *mod, uint8_t port_num, int enable);
int switzer_upoe_set_interrupt_mask(struct switzer_manhattan *mod, uint8_t port_num, uint8_t masked);
int switzer_upoe_get_interrupt_mask(struct switzer_manhattan *mod, uint8_t port_num, uint8_t* masked);
int switzer_upoe_get_interrupt_events(struct switzer_manhattan *mod, uint8_t port_num, uint8_t *pwr_enable_event,
                                          uint8_t *pwr_good_status_event, uint8_t *disconnect_event, uint8_t *pwr_cut_event,
                                          uint8_t *detect_event, uint8_t *cls_event, uint8_t *inrush_event,
                                          uint8_t *ilim_event, uint8_t *supply_event);
int switzer_upoe_get_poweron_fault(struct switzer_manhattan *mod, uint8_t port_num, power_on_fault_t *power_onfault);
int switzer_upoe_set_4P_power_allocation (struct switzer_manhattan *mod, uint8_t port_num, pwr_allocation_t power_allocation);
int switzer_upoe_get_4P_power_allocation(struct switzer_manhattan *mod, uint8_t port_num, int *power_allocation);
int switzer_upoe_set_4P_policing ();
int switzer_upoe_get_temperature (struct switzer_manhattan *mod, uint8_t port_num, unsigned long *temp_value);
int switzer_upoe_get_pse_mfrid (struct switzer_manhattan *mod, uint8_t *mfr_id, uint8_t *ic_id);
int switzer_upoe_get_firmware_version(struct switzer_manhattan *mod, uint8_t *firmwarerev);
int switzer_upoe_get_input_voltage(struct switzer_manhattan *mod, uint8_t port_num, unsigned long *voltage);
int switzer_upoe_get_port_volt_current_measurements(struct switzer_manhattan *mod, uint8_t port_num,
                                                        unsigned long *voltage, unsigned long *current);
int switzer_upoe_set_foldback_curve (struct switzer_manhattan *mod, uint8_t port_num, int set_foldback);
int switzer_upoe_get_foldback_curve (struct switzer_manhattan *mod, uint8_t port_num, int switzer_upoe_set_foldback);
int switzer_upoe_get_port_detect_capacitance(struct switzer_manhattan *mod, uint8_t port_num, unsigned long *detectcapacitance);
int switzer_upoe_get_port_detect_resistance(struct switzer_manhattan *mod, uint8_t port_num, unsigned long *detectresistance);
int switzer_upoe_get_port_assigned_class(struct switzer_manhattan *mod, uint8_t port_num, int *preclass, int *assignclass);
int switzer_load_sram_and_parity_code (struct switzer_manhattan *mod);
int safe_mode_load_code (struct switzer_manhattan *mod);
int switzer_upoe_pse_init(struct switzer_manhattan *mod, operating_mode_t mode, pwr_allocation_t pwr);

/*****************TPS23881 *******************/
/*TPS23881 is an 8-channel PSE, which can provide up to 90w power with 4p mode*/
#define POE_REG_INTERRUPT                          0x00    /* Interrupt */
#define POE_REG_INTR_MASK                          0x01    /* Int Mask */
#define POE_REG_POWER_EVENT                        0x02    /* Power Event */
#define POE_REG_POWER_EVENT_COR                    0x03    /* Power Event CoR */
#define POE_REG_DETECT_EVENT                       0x04    /* Detect Event */
#define POE_REG_DETECT_EVENT_COR                   0x05    /* Detect Event CoR */
#define POE_REG_FAULT_EVENT                        0x06    /* Fault Event */
#define POE_REG_FAULT_EVENT_COR                    0x07    /* Fault Event CoR */
#define POE_REG_TSTART_ILIM_EVENT                  0x08    /* tSTART Event */
#define POE_REG_TSTART_EVENT_COR                   0x09    /* tSTART Event CoR */
#define POE_REG_SUPPLY_EVENT                       0x0A    /* Supply Event */
#define POE_REG_SUPPLY_EVENT_COR                   0x0B    /* Supply Event CoR */
#define POE_REG_CH1_DISCOVERY                      0x0C    /* Ch1 Status */
#define POE_REG_CH2_DISCOVERY                      0x0D    /* CH2 Status */
#define POE_REG_CH3_DISCOVERY                      0x0E    /* CH3 Status */
#define POE_REG_CH4_DISCOVERY                      0x0F    /* CH4 Status */
#define POE_REG_POWER_STATUS                       0x10    /* Power Status */
#define POE_REG_PIN_STATUS                         0x11    /* Pin Status */
#define POE_REG_OPERATING_MODE                     0x12    /* Operating Mode */
#define POE_REG_DISCONNECT_ENABLE                  0x13    /* Disconnect Enable */
#define POE_REG_DETECT_CLASS_ENABLE                0x14    /* Detect/Class Enable */
#define POE_REG_PWRPR_ICUT_DISABLE                 0x15    /* power priority/ICUT disable */
#define POE_REG_TIMING_CONFIG                      0x16    /* Timing Config */
#define POE_REG_GENERAL_MASK                       0x17    /* general mask Config */
#define POE_REG_DET_CLASS_RESTART                  0x18    /* Det/Class Restart  */
#define POE_REG_PWRON                              0x19    /* Power ON */
#define POE_REG_RESET                              0x1A    /* RESET */
#define POE_REG_ID                                 0x1B    /* ID */
#define POE_REG_AC                                 0x1C    /* Auto Class */
#define POE_REG_TEST_ENABLE                        0x1C    /* test enable*/
#define POE_REG_2P_POLICE_CH1                      0x1E    /* police Channel 1 config */
#define POE_REG_2P_POLICE_CH2                      0x1F    /* police Channel 2 config */
#define POE_REG_2P_POLICE_CH3                      0x20    /* police Channel 3 config */
#define POE_REG_2P_POLICE_CH4                      0x21    /* police Channel 4 config */
#define POE_REG_LEGACY_DETECT                      0x22    /* legacy detect */
#define POE_REG_PWR_ON_FAULT                       0x24    /* power-on fault */
#define POE_REG_PORTS_REMAPPING                    0x26    /* ports remapping */
#define POE_REG_PWR_PORIORITY_P2_P1                0x27    /* multi-bit power priority */
#define POE_REG_PWR_PORIORITY_P4_P3                0x28    /* multi-bit power priority */
#define POE_REG_4P_PWR_ALLOCATION                  0x29    /* 4p power allocation*/
#define POE_REG_4P_POLICE_CONFIG_CH1_CH2           0x2A    /* 4p police config for channel 1,2*/
#define POE_REG_4P_POLICE_CONFIG_CH3_CH4           0x2B    /* 4p police config for channel 3, 4*/
#define POE_REG_TEMP                               0x2C    /* Temperature */
#define POE_REG_4P_OPERATION                       0x2D    /* 4p operation, Ilin, Pcut, Dc connect*/
#define POE_REG_INPUT_VOLTAGE                      0x2E    /* input voltage*/
#define POE_REG_CURRENT_CH1                        0x30    /* Current measurement for CH1 */
#define POE_REG_CURRENT_CH2                        0x34    /* Current measurement for CH2 */
#define POE_REG_CURRENT_CH3                        0x38    /* Current measurement for CH3 */
#define POE_REG_CURRENT_CH4                        0x3c    /* Current measurement for CH4 */
#define POE_REG_VOLTAGE_CH1                        0x32    /* Voltage measurement for CH4 */
#define POE_REG_VOLTAGE_CH2                        0x36    /* Voltage measurement for CH4 */
#define POE_REG_VOLTAGE_CH3                        0x3a    /* Voltage measurement for CH4 */
#define POE_REG_VOLTAGE_CH4                        0x34    /* Voltage measurement for CH4 */
#define POE_REG_FOLDBACK_AND_INRUSH                0x40    /* For Powering misbehaving*/
#define POE_REG_FIRMWARE_REV                       0x41    /* For firmware revision*/
#define POE_REG_WATCHDOG                           0x42    /* Watch Dog */
#define POE_REG_DEVICE_ID                          0x43    /* Device ID */
#define POE_REG_DETECT_RESISTANCE_CH1              0x44    /* P1 Detection signature resistance */
#define POE_REG_DETECT_RESISTANCE_CH2              0x45    /* P2 Detection signature resistance */
#define POE_REG_DETECT_RESISTANCE_CH3              0x46    /* P3 Detection signature resistance */
#define POE_REG_DETECT_RESISTANCE_CH4              0x47    /* P4 Detection signature resistance */
#define POE_REG_DETECT_CAPACITANCE_CH1             0x48    /* P1 Detection signature capacitance*/
#define POE_REG_DETECT_CAPACITANCE_CH2             0x49    /* P2 Detection signature capacitance */
#define POE_REG_DETECT_CAPACITANCE_CH3             0x4A    /* P3 Detection signature capacitance */
#define POE_REG_DETECT_CAPACITANCE_CH4             0x4B    /* P4 Detection signature capacitance */
#define POE_REG_ASSIGNED_CLASS_CH1                 0x4C    /*Channel 1 hold assigned class */
#define POE_REG_ASSIGNED_CLASS_CH2                 0x4D    /*Channel 2 hold assigned class */
#define POE_REG_ASSIGNED_CLASS_CH3                 0x4E    /*Channel 3 hold assigned class */
#define POE_REG_ASSIGNED_CLASS_CH4                 0x4F    /* Channel 4 hold assigned class */
#define POE_REG_AUTO_CLASS_CONTROL                 0x50    /* auto class control and power */
#define POE_REG_AUTOCLASS_PWR_REG_P1               0x51    /*Port 1 auto class control and power */
#define POE_REG_AUTOCLASS_PWR_REG_P2               0x52    /*Port 2 auto class control and power */
#define POE_REG_AUTOCLASS_PWR_REG_P3               0x53    /*Port 3 auto class control and power */
#define POE_REG_AUTOCLASS_PWR_REG_P4               0x54    /*Port 4 auto class control and power */
#define POE_REG_PCUT_OFFSET_P1_P2                  0x55    /* Cut power offset for port 1,2*/
#define POE_REG_PCUT_OFFSET_P3_P4                  0x56    /* Cut power offset for port 3,4*/
#define POE_REG_SRAM_CTRL_REG                      0x60    /* SRAM Control register*/
#define POE_REG_SRAM_DATA_REG                      0x61    /* SRAM Data register*/
#define POE_REG_SRAM_PROGRAM_START_ADDR_LSB        0x62    /* start programming at a specific address LSB*/
#define POE_REG_SRAM_PROGRAM_START_ADDR_MSB        0x63    /* start programming at a specific address MSB */


#endif /* __SWITZER_MANHATTAN_UPOE_H__*/

