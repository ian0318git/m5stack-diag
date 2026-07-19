/* $Id: platform_idprom.h,v 1.2 2019/08/06 06:56:13 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_1RU/platform_idprom.h,v $
 *------------------------------------------------------------------
 *
 * platform_idprom.h: This is the header file for the Serial EEPROM
 *
 * May 2013 - porting the code from Nightster
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 * Original Author: Dave DeSimone
 */
#ifndef _IDPROM_H_
#define _IDPROM_H_

#define PEM_EEPROM_SIZE 512
#define PEM_EEPROM_SIZE_OLD 256
#include "types.h"
typedef  enum scby_c2w_access_ {
    SCBY_I2C_ACCESS = 0,
    SCBY_HT_ACCESS,
} scby_cw2_access_t;


typedef struct mcp_idprom_t_ {
    uchar eeprom_version;
    uchar model;
    uchar hardware_version_high;
    uchar hardware_version_low;
    uchar serial_number[4];
    uchar part_number[4];
    uchar mac_base[6];
    uchar mac_block_size[2];
    uchar test_history;
    uchar RMA_number[3];
    uchar mfg_date;
    uchar reserved[7];
    uchar undefined[224];
    uchar undefined_2[256];
#if ( defined(MCP_FP160G) || defined(MCP_1RUVE) )
    uchar undefined_3[512];
#endif /* MCP_FP160G || MCP_1RUVE */
} mcp_idprom_t;

#define EEPROM_BYTES sizeof(mcp_idprom_t)


#define PACKED(item) item /* TODO fix-me: item __attribute__ ((packed)) */
#ifndef MCP_DIAG_SIM
#define _U      01
#define _L      02
#define _N      04
#define _S      010
#define _P      020
#define _C      040
#define _X      0100
#define _B      0200
#endif

typedef enum {
    SAME_ADDRESS   = 0,
    PREV_ADDRESS   = 1,
    NEXT_ADDRESS   = 2,
    END_OF_TEST    = -1
} prom_write_state_t;



/*
 * definition of IDPROM TLV field.
 */
typedef struct {
    uchar PACKED(tlv_type);     /* object type          */
    union {
        uchar PACKED(t_byte);   /* byte size fields     */
        ushort PACKED(t_short); /* short size fields    */
        ulong PACKED(t_long);   /* long size fields     */
        struct {                /* dynamic fields       */
            uchar PACKED(t_length);/* object length     */
            uchar PACKED(t_data[0]);/* object data      */
        } PACKED(t_dyn);
    } tlv_u;
} idprom_tlv_t;



/*
 * shortcuts
 */
#define tlv_byte        tlv_u.t_byte
#define tlv_short       tlv_u.t_short
#define tlv_long        tlv_u.t_long
#define tlv_length      tlv_u.t_dyn.t_length
#define tlv_data        tlv_u.t_dyn.t_data
 
/*
 * ID PROM Type fields.
 * Type field groups are defined as follows:
 *      0x00 - 0x3f - no length field, 1 byte data field
 *      0x40 - 0x7f - no length field, 2 byte data field
 *                      (1st byte d15-8, 2nd byte d7-0)
 *      0x80 - 0xbf - no length field, 4 byte data field
 *                      (1st byte d31-24, ...)
 *      0xc0 - 0xfe - next byte is length field
 *      0xff - extends to next byte for type
 *             (currently end-of-data marker)
 */
 
/*
 * individual type definitions
 * To add a new type,
 *      - Define T_IDPROM_xxxx.
 *      - For dynamic size fields, define S_IDPROM_xxxx.
 *      - Add display information to idprom.c:idprom_print_field().
 *
 * Extended types can be defined by adding (multiples of)
 * T_IDPROM_EXT_OFFSET (see idprom_private.h) to the type definition.
 * For example, the first extended type would have a value of 0x101.
 * Representation for this type in IDPROM would be [0x00 0x01].
 * Extended type 0x201 would be represented in ID PROM by [0x00 0x00 0x01].
 */
#define T_IDPROM_NUM_SLOTS      0x01    /* Number of slots in chassis   */
#define T_IDPROM_FAB_VERSION    0x02    /* Fab Version                  */
#define T_IDPROM_RMA_FAILCODE   0x03    /* RMA test history/failure code*/
#define T_IDPROM_RMA_HISTORY    0x04    /* RMA history                  */
#define T_IDPROM_CONNECTOR_TYPE 0x05    /* Card connector type          */
#define T_IDPROM_EHSA_PREF_MSTR 0x06    /* EHSA Preferred Master        */
#define T_IDPROM_PWR_SUPPLY     0x0B    /* Power Supply Type 0=AC, 1=DC */
#define T_IDPROM_SPA_FORMAT_REV 0x0D    /* SPA IDPROM Format Revision   */
 
#define T_IDPROM_HW_TYPE        0x40    /* HW (PA) type                 */
#define T_IDPROM_HW_VERSION     0x41    /* HW (PA) version              */
#define T_IDPROM_PCB_REVISION   0x42    /* PCB Revision number          */
#define T_IDPROM_MAC_BLKSIZE    0x43    /* MAC address block size       */
#define T_IDPROM_BOOT_TIMEOUT   0x46    /* BOOT TIMEOUT         */
 
#define T_IDPROM_DEVIATION      0x80    /* Deviation Number             */
#define T_IDPROM_RMA_NUMBER     0x81    /* RMA number                   */
#define T_IDPROM_PCB_PARTNBR_4  0x82    /* PCB part number (4-byte) Level 73 */
#define T_IDPROM_PCB_68_PARTNBR 0x87    /* PCB part number (4-byte) Level-68 */
#define T_IDPROM_NEW_DEVIATION_NUM 0x88	/* New Deviation Number		*/
#define T_IDPROM_VERS_ID	0x89	/* Version Identifier		*/
#define T_IDPROM_73_REVISION    0x8A    /* 73 Level Revision            */
#define T_IDPROM_68_REVISION    0x8D    /* 68 Level Revision            */
#define T_IDPROM_UDI_DESCR	0xDA	/* UDI Product Description	*/
#define T_IDPROM_UDI_NAME	0xDB	/* UDI Product Name  		*/
 
#define T_IDPROM_PCB_PARTNBR_6  0xC0    /* PCB part number (6-byte) Level-800 */
#define T_IDPROM_PCB_SERIAL     0xC1    /* PCB serial number            */
#define T_IDPROM_CHASSIS_SERIAL 0xC2    /* Chassis serial number        */
#define T_IDPROM_MACADDR        0xC3    /* Chassis base MAC address     */
#define T_IDPROM_MFG_TEST       0xC4    /* MFG Test Engineering field   */
#define T_IDPROM_FIELD_DIAGS    0xC5    /* Field Diagnostics results    */
#define T_IDPROM_CLEI           0xC6    /* CLEI Code                    */
#define T_IDPROM_ENVMON         0xC7    /* Environmental Monitor data   */
#define T_IDPROM_CALIBRATION    0xC8    /* Calibration data             */
#define T_IDPROM_DEV_SPECIFIC   0xC9    /* Device Specific Values       */
#define T_IDPROM_PROD_NUM	0xCB	/* Product Number		*/
#define T_IDPROM_CSCO_ASSET_MIB	0xCC	/* Cisco Entity Asset MIB	*/
#define T_IDPROM_BASE_MAC_ADDR	0xCF	/* Base MAC Address	*/
#define T_IDPROM_ASST_ALIAS 	0xD4  /*Asset Alias */  
#define T_IDPROM_PRCS_LABEL 	0xD5  /*Process Label */
#define T_IDPROM_CLK_FREQ       0xD6    /* Clock Frequency      */

#define T_IDPROM_PWR_CONSUMPTION 0xD7  /*Power Consumption */  
#define T_IDPROM_DIG_SIGNATURE 	0xD9  /*Digital signature */  
 
/*
 * Size fields - only useful and defined for variable length fields.
 * Other field types have implicit length.
 * For variable length fields, set value to zero.
 */
#define S_IDPROM_PCB_PARTNBR_6  6       /* Part Number                  */
#define S_IDPROM_PCB_SERIAL     11      /* PCB serial number            */
#define S_IDPROM_CHASSIS_SERIAL 11      /* Chassis serial number        */
#define S_IDPROM_MACADDR        6       /* Chassis base MAC address     */
#define S_IDPROM_MFG_TEST       8       /* MFG Test Engineering field   */
#define S_IDPROM_FIELD_DIAGS    8       /* Field Diagnostics results    */
#define S_IDPROM_CLEI           10      /* CLEI data                    */
#define S_IDPROM_ENVMON         32      /* Environmental Monitor data   */
#define S_IDPROM_PROD_NUM	8	/* Product Number		*/
#define S_IDPROM_CALIBRATION    0       /* Calibration data             */
 
 
/*
 * ID PROM Type fields.
 * Type field groups are defined as follows:
 *      0x00 - 0x3f - no length field, 1 byte data field
 *      0x40 - 0x7f - no length field, 2 byte data field
 *                      (1st byte d15-8, 2nd byte d7-0)
 *      0x80 - 0xbf - no length field, 4 byte data field
 *                      (1st byte d31-24, ...)
 *      0xc0 - 0xfe - next byte is length field
 *      0xff - extends to next byte for type
 *             (currently end-of-data marker)
 */
 
#define T_IDPROM_TYPE_MASK      0xc0    /* mask to get basic field type */
 
#define T_IDPROM_BYTE           0x00    /* data is single byte          */
#define T_IDPROM_SHORT          0x40    /* data is two bytes            */
#define T_IDPROM_LONG           0x80    /* data is four bytes           */
#define T_IDPROM_VAR            0xc0    /* variable length field        */
#define T_IDPROM_SPA_FDIAGS     0xF4    /* variable length field        */
#define T_IDPROM_SPA_ENV        0xF3    /* variable length field        */
 
#define T_IDPROM_SPA_DSIGN0 0xc1 
#define T_IDPROM_SPA_DSIGN1 0x40
#define T_IDPROM_SPA_DSIGN2 0xcb

#define T_IDPROM_EXTENSION      0x00    /* extension type marker        */
#define T_IDPROM_EOD            0xff    /* end-of-data marker           */
 
#define T_IDPROM_EXT_OFFSET     0x100   /* offset to add to type field  */
                                        /* for extended types           */
 
/*
 * display basic size field definition
 * Type values are used to define the raw display type.
 * Type values are stored in length field bit 6&7.
 */
#define S_IDPROM_SIZE_MASK      0x3f
#define S_IDPROM_TYPE_MASK      0xc0
#define S_IDPROM_TYPE_HEX       0x00
#define S_IDPROM_TYPE_DECIMAL   0x40
#define S_IDPROM_TYPE_ASCII     0x80
#define S_IDPROM_TYPE_RESERVED  0xc0
 
/*
 * ID PROM version number.
 * ID PROM must have this version number to support
 * TLV fields.
 */
#define TLV_IDPROM_VERSION      0x04
 
/*
 * Version number and TLV data offset for compatible TLV data
 */
#define IDPROM_VERSION_OFFSET   0
#define IDPROM_TLV_OFFSET       2
 
/*
 * Secondary version number for 'old' adapters.
 * Some PAs are programmed with a version number field of 0x00.
 */
#define IDPROM_VERSION_0        0
 
/*
 * Environmental data fields
 */
#ifdef MCP_2RUVE
#define IDPROM_ENV_FIELD_SIZE         0x2E
#define IDPROM_VOLTAGE1_SCALE_OFFSET  8  
#define IDPROM_DEV1_VP2_OFFSET        0x94
#define IDPROM_DP_VOLT0_CHKSUM_START 0x4F 
#define IDPROM_DP_VOLT0_CHKSUM_END   0x7A 
#define IDPROM_DP_VOLT0_CHKSUM_OFFSET 0x7C 
#define IDPROM_DP_VOLT1_CHKSUM_START 0x7F 
#define IDPROM_DP_VOLT1_CHKSUM_END   0xAA 
#define IDPROM_DP_VOLT1_CHKSUM_OFFSET 0xAC 
#endif

/*
 * Return values for idprom_get_field_data
 */
#define IDPROM_RET_BAD_SIZE (int)(-1)

/*
 *  Structure to hold Environmental Voltage info
 */ 

typedef struct {
  
  uchar       type;
  uchar       len;
  uchar       subtype;
  uchar       sensor_i2c_bus;  
  uchar       sensor_i2c_adr;  
  uchar       unique_id;  
  uchar       sensor_name[4];  
  uchar       vx1_scale;
  uchar       vx1_mv_hi;
  uchar       vx1_mv_lo;
  uchar       vx2_scale;
  uchar       vx2_mv_hi;
  uchar       vx2_mv_lo;
  uchar       vx3_scale;
  uchar       vx3_mv_hi;
  uchar       vx3_mv_lo;
  uchar       vx4_scale;
  uchar       vx4_mv_hi;
  uchar       vx4_mv_lo;
  uchar       vx5_scale;
  uchar       vx5_mv_hi;
  uchar       vx5_mv_lo;
  uchar       vp1_scale;
  uchar       vp1_mv_hi;
  uchar       vp1_mv_lo;
  uchar       vp2_scale;
  uchar       vp2_mv_hi;
  uchar       vp2_mv_lo;
  uchar       vp3_scale;
  uchar       vp3_mv_hi;
  uchar       vp3_mv_lo;
  uchar       vp4_scale;
  uchar       vp4_mv_hi;
  uchar       vp4_mv_lo;
  uchar       vh_scale;
  uchar       vh_mv_hi;
  uchar       vh_mv_lo;
  uchar       aux1_scale;
  uchar       aux1_mv_hi;
  uchar       aux1_mv_lo;
  uchar       aux2_scale;
  uchar       aux2_mv_hi;
  uchar       aux2_mv_lo;
  ushort      chksum;

} idprom_env_voltage_t;

#define IDPROM_ENV_VOLTAGE_INFO_SIZE      0x2E
#define IDPROM_ENV_VOLTAGE_SUBTYPE        0x3
#define IDPROM_ENV_VOLTAGE_SUBTYPE_OFFSET 2 


#endif /* _IDPROM_H_ */

/*
 *------------------------------------------------------------------
 * $Log: platform_idprom.h,v $
 * Revision 1.2  2019/08/06 06:56:13  alpeng
 * merge curie, switzer and nightwatch to trunk
 *
 * Revision 1.1.2.1  2018/06/22 08:05:18  alpeng
 * move curie diag to neptune/curie_1RU directory
 *
 * Revision 1.1.2.1  2018/05/30 02:39:36  alpeng
 * porting neptune x86 to curie
 *
 * Revision 1.2  2018/05/18 09:25:00  alpeng
 *  Neptune merge to trunk with tag <neptune-branch-0518>
 *
 * Revision 1.1.2.1  2016/06/02 22:04:01  jskow
 * Move Overlord/x86 specific files to Neptune/x86.
 *
 * Revision 1.2  2013/11/26 08:40:38  hroni
 * fix compiler warning
 *
 * Revision 1.1  2013/05/31 12:43:14  danchung
 * Porting PSU source code from Nightster for Juno.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
