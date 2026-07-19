/*------------------------------------------------------------------
 *
 * pericom_pcie_switch.h - P17C9X2G808PR PCIe Packet Switch
 *
 * Jun. 2019, Jacob Rast <jrast@cisco.com>
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#define     ADR_DATA_REG_ADR(input) \
    ((input << ADR_DATA_REG_ADR_OFFSET) & 0x0000FFFF)

#define     ADR_DATA_REG_DATA(input) \
    ((input << ADR_DATA_REG_DATA_OFFSET) & 0xFFFF0000)

#define     EEPROM_CONTROL_REG      0x87C
#define     EEPROM_ADR_DATA_REG     0x880

#define     ADR_DATA_REG_ADR_OFFSET  0
#define     ADR_DATA_REG_DATA_OFFSET 16

#define     ADR_DATA_REG_START_OFFSET 0
#define     ADR_DATA_REG_STATUS_OFFSET 8
#define     ADR_DATA_REG_COMMAND_OFFSET 16

#define     EEPROM_COMMAND_START        1

#define     EEPROM_COMMAND_WRITE_STATUS  0x01
#define     EEPROM_COMMAND_WRITE        0x02
#define     EEPROM_COMMAND_READ         0x03
#define     EEPROM_COMMAND_DISABLE_WRITE    0x04
#define     EEPROM_COMMAND_READ_STATUS  0x05
#define     EEPROM_COMMAND_ENABLE_WRITE 0x06
#define     EEPROM_COMMAND_ERASE_EEPROM 0xC7
#define     EEPROM_COMMAND_DISABLE_AUTOLOAD 0x06
#define     EEPROM_COMMAND_ENABLE_AUTOLOAD 0x00

#define     ADR_DATA_REG_AUTOLOAD_OFFSET 4

#define     EEPROM_CONTROL_ERASE        (EEPROM_COMMAND_ERASE_EEPROM << ADR_DATA_REG_COMMAND_OFFSET)
#define     EEPROM_CONTROL_ENABLE_WRITE (EEPROM_COMMAND_ENABLE_WRITE << ADR_DATA_REG_COMMAND_OFFSET)
#define     EEPROM_CONTROL_WRITE        (EEPROM_COMMAND_WRITE << ADR_DATA_REG_COMMAND_OFFSET)
#define     EEPROM_CONTROL_READ         (EEPROM_COMMAND_READ << ADR_DATA_REG_COMMAND_OFFSET)
#define     EEPROM_CONTROL_START        (EEPROM_COMMAND_START << ADR_DATA_REG_START_OFFSET)
#define     EEPROM_CONTROL_DISABLE_AUTOLOAD (EEPROM_COMMAND_DISABLE_AUTOLOAD << ADR_DATA_REG_AUTOLOAD_OFFSET)
