/* $Id: ipmi_sprom.h,v 1.2 2016/04/20 08:41:35 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/fru/ipmi_sprom.h,v $
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 */
#ifndef _IPMI_SPROM_DEFS_H_
#define _IPMI_SPROM_DEFS_H_


#ifndef PACKED
#define PACKED __attribute__((packed))
#endif

#define IPMI_SPROM_TOC_VERSION			0x0001

/*
** *******************************************************************
** IPMI DEFINITIONS 
** *******************************************************************
*/
#define M8_SIZE(sz)     (((sz) >> 3) + (((sz) & 0x07)? 1 : 0))

// recommended sizes for 2K EEPROM
#define IPMI_COMMON_HEADER_SIZE			8
#define IPMI_INTERNAL_USE_SIZE			8	
#define IPMI_CHASSIS_INFO_SIZE			48
#define IPMI_BOARD_INFO_SIZE			128
#define IPMI_PRODUCT_INFO_SIZE			128

#define IPMI_SPROM_COMMON_HEADER_SIGNATURE	0x0000
#define IPMI_SPROM_COMMON_HEADER_V(x)		((unsigned char)x)
#define IPMI_SPROM_COMMON_HEADER_VERSION IPMI_SPROM_COMMON_HEADER_V(1)

#define IPMI_SPROM_INTERNAL_USE_SIGNATURE	0x0000
#define IPMI_SPROM_INTERNAL_USE_VERSION		0x0001

#define IPMI_SPROM_CHASSIS_SIGNATURE		0x0000
#define IPMI_SPROM_CHASSIS_VERSION		0x0001

#define IPMI_SPROM_BOARD_SIGNATURE		0x0000
#define IPMI_SPROM_BOARD_VERSION		0x0001

// type/length, type = bits[7:6], length = bits[5:0]
#define IPMI_SPROM_TYPE_CODE_OFFSET	6
#define IPMI_SPROM_TYPE_CODE_BINARY	0
#define IPMI_SPROM_TYPE_CODE_BCD_PLUS	1
#define IPMI_SPROM_TYPE_CODE_ASCII	2
#define IPMI_SPROM_TYPE_CODE_LC		3

#define IPMI_SPROM_TYPE_CODE_BINARY_MSK		0x00
#define IPMI_SPROM_TYPE_CODE_BCD_PLUS_MSK	0x40
#define IPMI_SPROM_TYPE_CODE_ASCII_MSK		0x80	
#define IPMI_SPROM_TYPE_CODE_LC_MSK		0xC0	
#define IPMI_SPROM_NO_MORE_TYPE_LENGTH		0xC1


// Chassis type definitions
#define IPMI_SPROM_CHASSIS_TYPE_OTHER			0x01
#define IPMI_SPROM_CHASSIS_TYPE_UNKNOWN			0x02
#define IPMI_SPROM_CHASSIS_TYPE_DESKTOP			0x03
#define IPMI_SPROM_CHASSIS_TYPE_LOW_PROFILE_DESKTOP	0x04
#define IPMI_SPROM_CHASSIS_TYPE_PIZZA_BOX		0x05
#define IPMI_SPROM_CHASSIS_TYPE_MINI_TOWER		0x06
#define IPMI_SPROM_CHASSIS_TYPE_TOWER			0x07
#define IPMI_SPROM_CHASSIS_TYPE_PORTABLE		0x08
#define IPMI_SPROM_CHASSIS_TYPE_LAPTOP			0x09
#define IPMI_SPROM_CHASSIS_TYPE_NOTEBOOK		0x0A
#define IPMI_SPROM_CHASSIS_TYPE_HAND_HELD		0x0B
#define IPMI_SPROM_CHASSIS_TYPE_DOCKING_STATION		0x0C
#define IPMI_SPROM_CHASSIS_TYPE_ALL_IN_ONE		0x0D
#define IPMI_SPROM_CHASSIS_TYPE_SUB_NOTEBOOK		0x0E
#define IPMI_SPROM_CHASSIS_TYPE_SPACE_SAVING		0x0F
#define IPMI_SPROM_CHASSIS_TYPE_LUNCH_BOX		0x10
#define IPMI_SPROM_CHASSIS_TYPE_MAIN_SERVER_CHASSIS	0x11
#define IPMI_SPROM_CHASSIS_TYPE_EXPANSION_CHASSIS	0x12
#define IPMI_SPROM_CHASSIS_TYPE_SUB_CHASSIS		0x13
#define IPMI_SPROM_CHASSIS_TYPE_BUS_EXPANSION_CHASSIS	0x14
#define IPMI_SPROM_CHASSIS_TYPE_PERIPHERAL_CHASSIS	0x15
#define IPMI_SPROM_CHASSIS_TYPE_RAID_CHASSIS		0x16
#define IPMI_SPROM_CHASSIS_TYPE_RACK_MOUNT_CHASSIS	0x17

// Language Codes
#define IPMI_LC_ENGLISH_0				0x00
#define IPMI_LC_ENGLISH					0x19

/*
** *******************************************************************
** IPMI COMMON HEADER
** *******************************************************************
*/
typedef struct _ipmi_sprom_common_header_s {
	uint8_t	version;
	uint8_t	internal_use;
	uint8_t	chassis_info;
	uint8_t board_info;
	uint8_t product_info;
	uint8_t	multi_record;
	uint8_t pad;
	uint8_t header_checksum;
}PACKED ipmi_sprom_common_header_t;

/*
** *******************************************************************
** IPMI INTERNAL USE 
** *******************************************************************
*/

// Card Types - these values are EDCS controlled. If a new card type needs to be allocated,
// please check with Arvind (arvikris) before updating this list
#define IPMI_IU_CARD_TYPE_UNKNOWN	0x00
#define IPMI_IU_CARD_TYPE_IOM		0x01
#define IPMI_IU_CARD_TYPE_OPLIN		0x02
#define IPMI_IU_CARD_TYPE_PALO		0x03
#define IPMI_IU_CARD_TYPE_MENLO		0x04
#define IPMI_IU_CARD_TYPE_IBMC		0x05
#define IPMI_IU_CARD_TYPE_MENLO_E	0x06
#define IPMI_IU_CARD_TYPE_SC_FAN	0x07
#define IPMI_IU_CARD_TYPE_SC_PSU	0x08
#define IPMI_IU_CARD_TYPE_SC_BP		0x09
#define IPMI_IU_CARD_TYPE_SCHULTZ	0x0A
#define IPMI_IU_CARD_TYPE_TIGERSHARK	0x0B
#define IPMI_IU_CARD_TYPE_NIANTIC	0x0C
#define IPMI_IU_CARD_TYPE_EVEREST	0x0D
#define IPMI_IU_CARD_TYPE_NETEFFECT	0x0E
#define IPMI_IU_CARD_TYPE_MONTEREYPARK  0x0F
#define IPMI_IU_CARD_TYPE_BBU		0x10
#define IPMI_IU_CARD_TYPE_VASONA	0x11
#define IPMI_IU_CARD_TYPE_DUBLIN	0x12
#define IPMI_IU_CARD_TYPE_FREMONT	0x13
#define IPMI_IU_CARD_TYPE_LIVERMORE	0x14
#define IPMI_IU_CARD_TYPE_NIANTIC_CR	0x15
#define IPMI_IU_CARD_TYPE_MEMFRONT	0x16
#define IPMI_IU_CARD_TYPE_MEMBACK	0x17
#define IPMI_IU_CARD_TYPE_IOM2		0x18
#define IPMI_IU_CARD_TYPE_TURLOCK	0x19
#define IPMI_IU_CARD_TYPE_COTATI	0x1a

// Sub Types
#define IPMI_IU_SUB_TYPE_MAC_ADDR	1
#define IPMI_IU_SUB_TYPE_IOM_TEMP_1	2
#define IPMI_IU_SUB_TYPE_IOM_TEMP_2	3
#define IPMI_IU_SUB_TYPE_RW_TEMP_1	4
#define IPMI_IU_SUB_TYPE_RW_TEMP_2	5
#define IPMI_IU_SUB_TYPE_FAN_RPM	6
#define IPMI_IU_SUB_TYPE_FAN_TEMP	7
#define IPMI_IU_SUB_TYPE_FAN_VOLTAGE	8
#define IPMI_IU_SUB_TYPE_PSU_TEMP	9
#define IPMI_IU_SUB_TYPE_PSU_IN_VOLT   	10
#define IPMI_IU_SUB_TYPE_PSU_OUT_VOLT1	11
#define IPMI_IU_SUB_TYPE_PSU_OUT_VOLT2	12
#define IPMI_IU_SUB_TYPE_PSU_IN_CURRENT	13
#define IPMI_IU_SUB_TYPE_PSU_OUT_CURRENT 14
#define IPMI_IU_SUB_TYPE_PSU_RPM	15
#define IPMI_IU_SUB_TYPE_BBU		16

// Mac Record.
#define IPMI_SPROM_IU_MAC_SIZE	8
#define IPMI_SPROM_IU_TYPE_EOL	0
#define IPMI_SPROM_IU_TYPE_MAC	1

typedef struct ipmi_iu_rec_s {
	uint16_t sub_type;
	uint16_t sub_type_len;
}PACKED ipmi_iu_rec_t;

typedef struct ipmi_iu_tlv_s {
	uint8_t 	type;
	uint8_t 	len;
	uint16_t 	value;
}PACKED ipmi_iu_tlv_t;

typedef struct ipmi_sprom_iu_mac_s {
	ipmi_iu_rec_t	srec;
	ipmi_iu_tlv_t	cnt;

	uint8_t 	type;
	uint8_t 	len;
	uint8_t 	mac[6];
}PACKED ipmi_iu_rec_mac_t;

// SDR Record
#define IPMI_IU_ENTRY_TYPE_CNT	1
#define IPMI_IU_ENTRY_TYPE_MAC	2

#define SDR_MULTI_FACTOR	1
#define SDR_BASE_OFFSET		2
#define SDR_K1			3
#define SDR_K2			4
#define SDR_NOMINAL_READ	5
#define SDR_NORMAL_MAX		6
#define SDR_NORMAL_MIN		7
#define SDR_SENSOR_MAX		8
#define SDR_SENSOR_MIN		9
#define SDR_UP_NR_THRES		10
#define SDR_UP_CRIT		11
#define SDR_UP_NON_CRIT		12
#define SDR_LO_NR_THRES		13
#define SDR_LO_CRIT		14
#define SDR_LO_NON_CRIT		15
#define SDR_POS_HYST		16
#define SDR_NEG_HYST		17

typedef struct ipmi_sdr_rec_s {
	uint8_t	 type;
	uint8_t  len;
	int16_t value;
}PACKED ipmi_sdr_rec_t;

typedef struct ipmi_iu_sdr_s {
	ipmi_iu_rec_t	srec;
	ipmi_sdr_rec_t	multi_factor;
	ipmi_sdr_rec_t	base_offset;
	ipmi_sdr_rec_t	k1;
	ipmi_sdr_rec_t	k2;
	ipmi_sdr_rec_t	nominal_reading;
	ipmi_sdr_rec_t	normal_max;
	ipmi_sdr_rec_t	normal_min;
	ipmi_sdr_rec_t	sensor_max;
	ipmi_sdr_rec_t	sensor_min;
	ipmi_sdr_rec_t	upper_nr_thres;
	ipmi_sdr_rec_t	upper_crit_thres;
	ipmi_sdr_rec_t	upper_non_crit_thres;
	ipmi_sdr_rec_t	lower_nr_thres;
	ipmi_sdr_rec_t	lower_crit_thres;
	ipmi_sdr_rec_t	lower_non_crit_thres;
	ipmi_sdr_rec_t	pos_hyst;
	ipmi_sdr_rec_t	neg_hyst;
}PACKED ipmi_iu_sdr_t;

// Board Specific Internal Use Areas.
#define IU_IOM_PAD_SIZE 	3
typedef struct _ipmi_iom_internal_use_s {
	uint8_t			version;
	uint8_t			length;
	uint16_t		card_type;
	ipmi_iu_rec_mac_t	mac;
	ipmi_iu_sdr_t		brd_temp1;
	ipmi_iu_sdr_t		brd_temp2;
	ipmi_iu_sdr_t		rw_temp1;
	ipmi_iu_sdr_t		rw_temp2;
	uint8_t			pad[IU_IOM_PAD_SIZE];
	uint8_t			checksum;
} PACKED ipmi_iom_internal_use_t;

// FAN Specific Internal Use Areas.
#define IU_FAN_PAD_SIZE         3
typedef struct _ipmi_fan_internal_use_s {
	uint8_t			version;
	uint8_t			length;
	uint16_t		card_type;
	ipmi_iu_sdr_t		fan_rpm;
	ipmi_iu_sdr_t		fan_temp;
	ipmi_iu_sdr_t		fan_volt;
	uint8_t			pad[IU_FAN_PAD_SIZE];
	uint8_t			checksum;
} PACKED ipmi_fan_internal_use_t;

// PSU Specific Internal Use Area.
#define IU_PSU_PAD_SIZE         3
typedef struct _ipmi_psu_internal_use_s {
	uint8_t			version;
	uint8_t			length;
	uint16_t		card_type;
	ipmi_iu_sdr_t		psu_temp;
	ipmi_iu_sdr_t		psu_in_volt;
	ipmi_iu_sdr_t		psu_out_volt1;
	ipmi_iu_sdr_t		psu_out_volt2;
	ipmi_iu_sdr_t		psu_in_current;
	ipmi_iu_sdr_t		psu_out_current;
	ipmi_iu_sdr_t		psu_rpm;
	uint8_t			pad[IU_PSU_PAD_SIZE];
	uint8_t			checksum;
} PACKED ipmi_psu_internal_use_t, ipmi_sprom_psu_internal_use_t;

// Mezzanine/Blade Internal Use Area.
typedef struct {
        uint8_t                 version;
        uint8_t                 length;
        uint16_t                card_type;
	ipmi_iu_rec_mac_t	mac;
	uint8_t			pad[3];
        uint8_t                 checksum;
} PACKED ipmi_mezz_internal_use_t, ipmi_ibmc_internal_use_v1_t;

#define IU_UUID_SIZE            16
// Mezzanine/Blade Internal Use Area.
typedef struct {
        uint8_t                 version;
        uint8_t                 length;
        uint16_t                card_type;
	ipmi_iu_rec_mac_t	mac;
	uint8_t			blade_class;	
	/* Added TPM Enable/Disable and UUID */
    uint8_t         uuid[IU_UUID_SIZE];
    uint8_t         tpm;
	uint8_t			window_active;
        uint8_t                 checksum;
} PACKED ipmi_ibmc_internal_use_t;

// Mezzanine/Blade Internal Use Area.
typedef struct {
        uint8_t                 version;
        uint8_t                 length;
        uint16_t                card_type;
	uint8_t			pad[1];
        uint8_t                 checksum;
} PACKED ipmi_mem_internal_use_t;

typedef struct {
	uint8_t			version;
	uint8_t			length;
	uint16_t 		card_type;
	uint8_t			pad[3];
	uint8_t			checksum;
} PACKED ipmi_hddbp_internal_use_t;

#define IPMI_SPROM_IBMC_IU_BLADE_CLASS_UNKNOWN (0)
#define IPMI_SPROM_IBMC_IU_BLADE_CLASS_GOODING (1)
#define IPMI_SPROM_IBMC_IU_BLADE_CLASS_VENTURA (2)
// IPMI_SPROM_IBMC_IU_BLADE_CLASS_NONE is for platforms that are not "blade systems"
#define IPMI_SPROM_IBMC_IU_BLADE_CLASS_NONE    (3)

/*
** *******************************************************************
** IPMI CHASSIS INFO 
** *******************************************************************
*/
#define IPMI_SPROM_CHASSIS_PART_NUMBER_SIZE	12
#define IPMI_SPROM_CHASSIS_SERIAL_NUMBER_SIZE	11
#define IPMI_SPROM_CHASSIS_PAD_SIZE		2
#define IPMI_SPROM_CHASSIS_MFG_INFO_SIZE	17

typedef struct _ipmi_sprom_chassis_info_s {
	uint8_t	version;
	uint8_t	length;			// This is in multiples of 8.
	uint8_t	type;			// Enumeration.
	uint8_t part_num_tl;		// Part number type/length.
	uint8_t part_num[IPMI_SPROM_CHASSIS_PART_NUMBER_SIZE];
	uint8_t	serial_num_tl;		// Serial number type/length.
	uint8_t serial_num[IPMI_SPROM_CHASSIS_SERIAL_NUMBER_SIZE];
	uint8_t mfg_info_tl;
	uint8_t mfg_info[IPMI_SPROM_CHASSIS_MFG_INFO_SIZE];
	uint8_t	no_more_tl;		// Indicates no more type lengths
	uint8_t chassis_checksum;
}PACKED ipmi_sprom_chassis_info_t;

typedef struct _ipmi_sprom_chassis_info_old_s {
	uint8_t	version;
	uint8_t	length;			// This is in multiples of 8.
	uint8_t	type;			// Enumeration.
	uint8_t part_num_tl;		// Part number type/length.
	uint8_t part_num[IPMI_SPROM_CHASSIS_PART_NUMBER_SIZE];
	uint8_t	serial_num_tl;		// Serial number type/length.
	uint8_t serial_num[IPMI_SPROM_CHASSIS_SERIAL_NUMBER_SIZE];
	uint8_t	no_more_tl;		// Indicates no more type lengths
	uint8_t pad[IPMI_SPROM_CHASSIS_PAD_SIZE];
	uint8_t chassis_checksum;
}PACKED ipmi_sprom_chassis_info_old_t;
/*
** *******************************************************************
** IPMI BOARD INFO 
** *******************************************************************
*/

#define IPMI_SPROM_BOARD_MFG_INFO_SIZE		17	
#define IPMI_SPROM_BOARD_PRODUCT_NAME_SIZE	18
#define IPMI_SPROM_BOARD_SERIAL_NUMBER_SIZE	11
#define IPMI_SPROM_BOARD_PART_NUMBER_SIZE	12
#define IPMI_SPROM_BOARD_PAD_SIZE		4
#define IPMI_SPROM_BOARD_NIM_PAD_SIZE		6
#define IPMI_SPROM_BOARD_MFG_DATE_SIZE		3
#define IPMI_SPROM_BOARD_FRU_FILE_ID_SIZE	5
#define IPMI_SPROM_BOARD_CUSTOM_ID_SIZE_OLD    	5	
#define IPMI_SPROM_BOARD_CUSTOM_ID_SIZE     	6	
#define IPMI_SPROM_BOARD_CLEI_SIZE_OLD		15	
#define IPMI_SPROM_BOARD_CLEI_SIZE		14	


typedef struct _ipmi_sprom_board_info_old_s_ {
	uint8_t	version;
	uint8_t length;	
	uint8_t	language_code;
	uint8_t mfg_date_time[3];
	uint8_t mfg_info_tl;
	uint8_t mfg_info[IPMI_SPROM_BOARD_MFG_INFO_SIZE];
	uint8_t	product_name_tl;
	uint8_t product_name[IPMI_SPROM_BOARD_PRODUCT_NAME_SIZE];
	uint8_t	serial_num_tl;		// Serial number type/length.
	uint8_t serial_num[IPMI_SPROM_BOARD_SERIAL_NUMBER_SIZE];
	uint8_t part_num_tl;		// Part number type/length.
	uint8_t part_num[IPMI_SPROM_BOARD_PART_NUMBER_SIZE];
	uint8_t	fru_file_id_tl;
	uint8_t	fru_file_id[IPMI_SPROM_BOARD_FRU_FILE_ID_SIZE];
	uint8_t	custom_id_tl;
	uint8_t bom_rev;
	uint8_t hw_rev;
	uint8_t pid_rev[3];
	uint8_t	clei_tl;
	uint8_t	clei[IPMI_SPROM_BOARD_CLEI_SIZE];
	uint8_t	no_more_tl;	// Indicates no more type lengths
//	uint8_t	pad[IPMI_SPROM_BOARD_PAD_SIZE];	
	uint8_t board_checksum;
}PACKED ipmi_sprom_board_info_old_t;

typedef struct _ipmi_sprom_board_info_s_ {
	uint8_t	version;
	uint8_t length;	
	uint8_t	language_code;
	uint8_t mfg_date_time[3];
	uint8_t mfg_info_tl;
	uint8_t mfg_info[IPMI_SPROM_BOARD_MFG_INFO_SIZE];
	uint8_t	product_name_tl;
	uint8_t product_name[IPMI_SPROM_BOARD_PRODUCT_NAME_SIZE];
	uint8_t	serial_num_tl;		// Serial number type/length.
	uint8_t serial_num[IPMI_SPROM_BOARD_SERIAL_NUMBER_SIZE];
	uint8_t part_num_tl;		// Part number type/length.
	uint8_t part_num[IPMI_SPROM_BOARD_PART_NUMBER_SIZE];
	uint8_t	fru_file_id_tl;
	uint8_t	fru_file_id[IPMI_SPROM_BOARD_FRU_FILE_ID_SIZE];
	uint8_t	custom_id_tl;
	uint8_t bom_rev[1];
	uint8_t hw_rev;
	uint8_t pid_rev[3];
	uint8_t	clei_tl;
	uint8_t	clei[IPMI_SPROM_BOARD_CLEI_SIZE];
	uint8_t pad_tl;
	uint8_t	pad[IPMI_SPROM_BOARD_NIM_PAD_SIZE];
	uint8_t	no_more_tl;	// Indicates no more type lengths
	uint8_t board_checksum;
}PACKED ipmi_sprom_board_info_t;

#define IPMI_SPROM_PRODUCT_MFG_NAME_SIZE       17
#define IPMI_SPROM_PRODUCT_NAME_SIZE           18
#define IPMI_SPROM_PRODUCT_PART_MODEL_SIZE     11
#define IPMI_SPROM_PRODUCT_VERSION_SIZE        2
#define IPMI_SPROM_PRODUCT_SERIAL_NUMBER_SIZE  11
#define IPMI_SPROM_PRODUCT_ASSET_TAG_SIZE      0
#define IPMI_SPROM_PRODUCT_FRU_FILE_ID_SIZE    0
#define IPMI_SPROM_PRODUCT_PAD_SIZE            2
#define IPMI_SPROM_PRODUCT_CUSTOM_ID_SIZE      5

typedef struct {
	uint8_t	version;
	uint8_t length;	
	uint8_t	language_code;
	uint8_t mfg_name_tl;
	uint8_t mfg_name[IPMI_SPROM_PRODUCT_MFG_NAME_SIZE]; 
	uint8_t product_name_tl;
	uint8_t product_name[IPMI_SPROM_PRODUCT_NAME_SIZE]; 
	uint8_t part_model_tl;
	uint8_t part_model[IPMI_SPROM_PRODUCT_PART_MODEL_SIZE];
	uint8_t prd_version_tl;
	uint8_t prd_version[IPMI_SPROM_PRODUCT_VERSION_SIZE];
        uint8_t serial_num_tl;  // Serial number type/length.
        uint8_t serial_num[IPMI_SPROM_PRODUCT_SERIAL_NUMBER_SIZE];
        uint8_t asset_tag_tl;
        uint8_t fru_file_id_tl;
        uint8_t custom_id_tl;
        uint8_t pid_rev[3];
        uint8_t bom_rev;
        uint8_t hw_rev;
        uint8_t pad_tl;
        uint8_t pad[IPMI_SPROM_PRODUCT_PAD_SIZE];
        uint8_t no_more_tl;     // Indicates no more type lengths
        uint8_t product_checksum;
}PACKED ipmi_sprom_product_info_t;


#define IPMI_SPROM_BMC_PRODUCT_MFG_NAME_SIZE		17
#define IPMI_SPROM_BMC_PRODUCT_NAME_SIZE		18
#define IPMI_SPROM_BMC_PRODUCT_PART_MODEL_SIZE 		12
#define IPMI_SPROM_BMC_PRODUCT_SERIAL_NUMBER_SIZE	11
#define IPMI_SPROM_BMC_PRODUCT_VERSION_SIZE		1
#define IPMI_SPROM_BMC_PRODUCT_ASSET_TAG_SIZE		28	
#define IPMI_SPROM_BMC_PRODUCT_FRU_FILE_ID_SIZE		8
#define IPMI_SPROM_BMC_PRODUCT_PAD_SIZE			8	
#define IPMI_SPROM_BMC_PRODUCT_NIM_PAD_SIZE		6
#define IPMI_SPROM_BMC_PRODUCT_CUSTOM_ID_SIZE       	5

typedef struct {
	uint8_t	version;
	uint8_t length;	
	uint8_t	language_code;
	uint8_t mfg_name_tl;
	uint8_t mfg_name[IPMI_SPROM_BMC_PRODUCT_MFG_NAME_SIZE]; 
	uint8_t product_name_tl;
	uint8_t product_name[IPMI_SPROM_BMC_PRODUCT_NAME_SIZE]; 
	uint8_t part_model_tl;
	uint8_t part_model[IPMI_SPROM_BMC_PRODUCT_PART_MODEL_SIZE];
	uint8_t prd_version_tl;
	uint8_t prd_version;
	uint8_t	serial_num_tl;	// Serial number type/length.
	uint8_t serial_num[IPMI_SPROM_BMC_PRODUCT_SERIAL_NUMBER_SIZE];
	uint8_t asset_tag_tl;
	uint8_t asset_tag[IPMI_SPROM_BMC_PRODUCT_ASSET_TAG_SIZE];
	uint8_t fru_file_id_tl;
	uint8_t fru_file_id[IPMI_SPROM_BMC_PRODUCT_FRU_FILE_ID_SIZE];
        uint8_t custom_id_tl;
        uint8_t bom_rev[2];
        uint8_t pid_rev[3];
	uint8_t pad_tl;
	uint8_t pad[IPMI_SPROM_BMC_PRODUCT_NIM_PAD_SIZE];
	uint8_t	no_more_tl;	// Indicates no more type lengths
	uint8_t product_checksum;
}PACKED ipmi_sprom_bmc_product_info_t;

/*
** *******************************************************************
** IPMI MULTI RECORD AREA 
** *******************************************************************
*/

#define IPMI_SPROM_MULTI_RECORD_VERSION			0x02
#define IPMI_SPROM_MULTI_RECORD_EOL			0x80

#define IPMI_SPROM_MULTI_RECORD_POWER_SUPPLY		0x00
#define IPMI_SPROM_MULTI_RECORD_DC_OUTPUT		0x01
#define IPMI_SPROM_MULTI_RECORD_DC_LOAD			0x02
#define IPMI_SPROM_MULTI_RECORD_MGMT_ACCESS		0x03
#define IPMI_SPROM_MULTI_RECORD_BASE_COMPATIBILITY	0x04
#define IPMI_SPROM_MULTI_RECORD_EXT_COMPATIBILITY	0x05
#define IPMI_SPROM_MULTI_RECORD_OEM_NCSI	        0xC0

typedef struct _ipmi_sprom_multi_record_header_s_ {
	uint8_t	record_type;
	uint8_t version;
	uint8_t length;
	uint8_t record_checksum;
	uint8_t header_checksum;
}PACKED ipmi_sprom_multi_record_header_t;

/*
 * NC-SI Multi Record used in Monterey Park.
 */
typedef struct _ipmi_ncsi_multi_record_s {
	ipmi_sprom_multi_record_header_t header;
	uint8_t oem[3];
	uint8_t version;
	uint32_t subtype;
	uint8_t power_source;
	uint16_t peak_standby_power;
	uint8_t mac_count;
	uint8_t mac[6];
	uint8_t pad;
}PACKED ipmi_ncsi_multi_record_t;
#define IPMI_OEM_NCSI_CISCO		{ 0x09, 0x00, 0x00 }
#define IPMI_OEM_NCSI_VERSION		0x01
#define IPMI_OEM_NCSI_SUBTYPE		0x01

#define IPMI_OEM_NCSI_POWER_SOURCE_PCIE 0x01
#define IPMI_OEM_NCSI_POWER_SOURCE_ALT	0x02

/*
** *******************************************************************
** IPMI FRU INFORMATION LAY OUT 
** *******************************************************************
*/

typedef struct _ipmi_sprom_iom_s_ {
	ipmi_sprom_common_header_t	common_header;
	ipmi_sprom_board_info_t		board_info;
	ipmi_iom_internal_use_t		internal_use;
}PACKED sprom_ipmi_iom_t;

typedef struct _ipmi_sprom_iom_bp_s_ {
	ipmi_sprom_common_header_t	common_header;
	ipmi_sprom_chassis_info_t	chassis_info;
	ipmi_sprom_board_info_t		board_info;
}PACKED sprom_ipmi_iom_bp_t;

extern void ipmi_zero_checksum_create (uint8_t *buf, int buflen);
extern int ipmi_zero_checksum_verify (uint8_t *buf, int buflen);

extern void ipmi_multi_record_checksum_create (ipmi_sprom_multi_record_header_t *mr);
extern int ipmi_multi_record_checksum_verify (ipmi_sprom_multi_record_header_t *mr);

typedef struct _ipmi_sprom_iom_psu_s_ {
	ipmi_sprom_common_header_t	common_header;
	ipmi_sprom_product_info_t	product_info;
	ipmi_psu_internal_use_t		internal_use;
}PACKED sprom_ipmi_iom_psu_t;

#define IPMI_SPROM_SD_PRODUCT_MFG_NAME_SIZE       17 
#define IPMI_SPROM_SD_PRODUCT_NAME_SIZE           18
#define IPMI_SPROM_SD_PRODUCT_PART_MODEL_SIZE     11 
#define IPMI_SPROM_SD_PRODUCT_VERSION_SIZE        2
#define IPMI_SPROM_SD_PRODUCT_SERIAL_NUMBER_SIZE  11
#define IPMI_SPROM_SD_PRODUCT_CUSTOM_ID_SIZE      5

typedef struct {
	uint8_t	version;
	uint8_t length;	
	uint8_t	language_code;
	uint8_t mfg_name_tl;
	uint8_t mfg_name[IPMI_SPROM_SD_PRODUCT_MFG_NAME_SIZE]; 
	uint8_t product_name_tl;
	uint8_t product_name[IPMI_SPROM_SD_PRODUCT_NAME_SIZE]; 
	uint8_t part_model_tl;
	uint8_t part_model[IPMI_SPROM_SD_PRODUCT_PART_MODEL_SIZE];
	uint8_t prd_version_tl;
	uint8_t prd_version[IPMI_SPROM_SD_PRODUCT_VERSION_SIZE];
        uint8_t serial_num_tl;  // Serial number type/length.
        uint8_t serial_num[IPMI_SPROM_SD_PRODUCT_SERIAL_NUMBER_SIZE];
        uint8_t asset_tag_tl;
        uint8_t fru_file_id_tl;
        uint8_t custom_id_tl;
        uint8_t pid_rev[3];
        uint8_t bom_rev[2];
        uint8_t standby_amp_tl;
        uint8_t amp[2];
        uint8_t no_more_tl;     // Indicates no more type lengths
        uint8_t product_checksum;
}PACKED ipmi_sprom_psu_product_info_t;

typedef struct _ipmi_sprom_psu_mr_s {
	ipmi_sprom_multi_record_header_t psu_info_hdr;
	uint8_t oem[5];
	uint8_t version[4];
	uint8_t power[2];
} PACKED ipmi_sprom_psu_mr_t;

typedef struct _ipmi_sprom_iom_psu_sd_s_ {
	ipmi_sprom_common_header_t	common_header;
	ipmi_sprom_psu_product_info_t	product_info;
	ipmi_sprom_psu_internal_use_t	internal_use;
	ipmi_sprom_psu_mr_t		multi_record;
}PACKED sprom_ipmi_sd_psu_t;

typedef struct _ipmi_sprom_iom_psu_p0_s_ {
	ipmi_sprom_common_header_t	common_header;
	ipmi_sprom_product_info_t	product_info;
}PACKED sprom_ipmi_iom_psu_p0_t, sprom_ipmi_psu_t;

typedef struct _ipmi_sprom_iom_fan_s_ {
	ipmi_sprom_common_header_t	common_header;
	ipmi_sprom_board_info_t		board_info;
	ipmi_fan_internal_use_t		internal_use;
}PACKED sprom_ipmi_iom_fan_t;

typedef struct _ipmi_sprom_mezz_s_ {
	ipmi_sprom_common_header_t	common_header;
	ipmi_sprom_board_info_t		board_info;
	ipmi_mezz_internal_use_t	internal_use;
}PACKED sprom_ipmi_mezz_t;

typedef struct _sprom_ipmi_mpark_s_ {
	ipmi_sprom_common_header_t	common_header;
	ipmi_sprom_board_info_t		board_info;
	ipmi_mezz_internal_use_t	internal_use;
	ipmi_ncsi_multi_record_t	ncsi_multi_record;
}PACKED sprom_ipmi_mpark_t;

typedef struct _ipmi_sprom_bbu_s_ {
	ipmi_sprom_common_header_t	common_header;
	ipmi_sprom_board_info_t		board_info;
	ipmi_sprom_bmc_product_info_t	product_info;
}PACKED sprom_ipmi_bbu_t;

typedef struct _ipmi_sprom_bmc_s_ {
	ipmi_sprom_common_header_t	common_header;
	ipmi_sprom_board_info_t		board_info;
	ipmi_sprom_bmc_product_info_t	product_info;
	ipmi_ibmc_internal_use_t	internal_use;
}PACKED sprom_ipmi_ibmc_t, sprom_ipmi_bmc_t;

typedef struct _ipmi_sprom_hddbp_s_ {
	ipmi_sprom_common_header_t	common_header;
	ipmi_sprom_board_info_t		board_info;
	ipmi_hddbp_internal_use_t	internal_use;
}PACKED sprom_ipmi_hddbp_t;

/////////////////////////////////////
// We may not be using these any more
/////////////////////////////////////

typedef struct _ipmi_sprom_mr_psu_info_s {
	uint16_t	capacity;
	uint16_t	peak_val;
	uint8_t		lsb;
	uint8_t		inrush_amp;
	uint8_t		inrush_interval;
	uint16_t	lo_end_inp_v1;
	uint16_t	hi_end_inp_v1;
	uint16_t	lo_end_inp_v2;
	uint16_t	hi_end_inp_v2;
	uint8_t		hi_end_inp_hz;
	uint8_t		inp_drop_out_tol;
	uint8_t		flags;
	uint8_t		hold_up_time;
	uint16_t	peak_capacity;
	uint16_t	total_watts;
	uint8_t		fail_tacho_thres;
}PACKED ipmi_sprom_mr_psu_info_t;

typedef struct _ipmi_sprom_mr_dc_out_s {
	uint8_t		info;
	uint16_t	nominal_volt;
	uint16_t	max_neg_volt;
	uint16_t	max_pos_volt;
	uint16_t	ripple_noise;
	uint16_t	min_amps;
	uint16_t	max_amps;
}PACKED ipmi_sprom_mr_dc_out_t;

typedef struct _ipmi_sprom_mr_mp_cisco_card_s {
	uint8_t		id[5];
	uint32_t  	format_version;
	uint16_t	power;
}PACKED ipmi_sprom_mr_mp_cisco_card_t;

typedef struct _ipmi_psu_sd_mr_s {
	ipmi_sprom_multi_record_header_t psu_info_hdr;
	ipmi_sprom_mr_psu_info_t	 psu_info; 

	ipmi_sprom_multi_record_header_t dc_out_1_hdr;		//12V
	ipmi_sprom_mr_dc_out_t		 dc_out_1;

	ipmi_sprom_multi_record_header_t dc_out_2_hdr;		// 5V
	ipmi_sprom_mr_dc_out_t		 dc_out_2;

	ipmi_sprom_multi_record_header_t cisco_mr_hdr; 
	ipmi_sprom_mr_mp_cisco_card_t	 cisco_mr;

} PACKED ipmi_psu_sd_mr_t;

typedef struct _ipmi_sprom_iom_psu_sd_old_s_ {
	ipmi_sprom_common_header_t	common_header;
	ipmi_sprom_psu_product_info_t	product_info;
	ipmi_psu_sd_mr_t		multi_record;
}PACKED sprom_ipmi_sd_psu_old_t;
typedef struct _ipmi_sprom_mem_s_ {
	ipmi_sprom_common_header_t	common_header;
	ipmi_sprom_board_info_t		board_info;
        ipmi_mem_internal_use_t            internal_use;
}PACKED sprom_ipmi_mem_t;

#endif // _IPMI_SPROM_DEFS_H_
