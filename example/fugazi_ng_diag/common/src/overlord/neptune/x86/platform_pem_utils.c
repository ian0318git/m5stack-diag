/* $Id: platform_pem_utils.c,v 1.2 2018/05/18 09:25:00 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/x86/platform_pem_utils.c,v $
 *------------------------------------------------------------------
 *
 * platform_pem_utils.c: 
 *              Routines for accessing the Power Supply eeprom
 *              and monitors (Output Voltage, Output Current,
 *              Input Voltage, Outlet Airflow Temp, and Usage Counter).
 *
 * May 2013 - porting the code from Nightster
 *
 * Copyright (c) 2013-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 * Original Author: Jay McCloy
 */
#include <stdio.h>
#include <string.h>
#include "platform_pem_utils.h"
#include "platform_idprom_utils.h"
#include "types.h"
#include "proto.h"
#include "common.h"
#include "i2c_api.h"
#include "platform_psu.h"
#include "queryflags.h"


rp1ruve_pem_register_t rp1ruve_pem_reg[] = {
    { PAGE_COMMAND          , PMBUS_BYTE_TRANSACTION,    0xDEAD, 0xDEAD, "PAGE_COMMAND" },
    { CLEAR_FAULTS          , PMBUS_NO_DATA_TRANSACTION, 0xDEAD, 0xDEAD, "CLEAR_FAULTS" },
    { FAN_CONFIG_1_2        , PMBUS_BYTE_TRANSACTION,    0xDEAD, 0xDEAD, "FAN_CONFIG_1_2" }, 
    { FAN_COMMAND_1         , PMBUS_WORD_TRANSACTION,    0xDEAD, 0xDEAD, "FAN_COMMAND_1" },  
    { IOUT_OC_FAULT_LIMIT   , PMBUS_WORD_TRANSACTION,    0xDEAD, 0xDEAD, "IOUT_OC_FAULT_LIMIT" },  
    { IOUT_OC_WARN_LIMIT    , PMBUS_WORD_TRANSACTION,    0xDEAD, 0xDEAD, "IOUT_OC_WARN_LIMIT" },  
    { OT_FAULT_LIMIT        , PMBUS_WORD_TRANSACTION,    0xDEAD, 0xDEAD, "OT_FAULT_LIMIT" },  
    { OT_WARN_LIMIT         , PMBUS_WORD_TRANSACTION,    0xDEAD, 0xDEAD, "OT_WARN_LIMIT" },  
    { VIN_UV_WARN_LIMIT     , PMBUS_WORD_TRANSACTION,    0xDEAD, 0xDEAD, "VIN_UV_WARN_LIMIT" },  
    { VIN_UV_FALT_LIMIT     , PMBUS_WORD_TRANSACTION,    0xDEAD, 0xDEAD, "VIN_UV_FALT_LIMIT" },  
    { IIN_OC_WARN_LIMIT     , PMBUS_WORD_TRANSACTION,    0xDEAD, 0xDEAD, "IIN_OC_WARN_LIMIT" },  
    { POWER_GOOD_ON	    , PMBUS_WORD_TRANSACTION,    0xDEAD, 0xDEAD, "POWER_GOOD_ON" },  
    { POWER_GOOD_OFF        , PMBUS_WORD_TRANSACTION,    0xDEAD, 0xDEAD, "POWER_GOOD_OFF" },  
    { POUT_OP_FAULT_LIMIT   , PMBUS_WORD_TRANSACTION,    0xDEAD, 0xDEAD, "POUT_OP_FAULT_LIMIT" },  
    { POUT_OP_WARN_LIMIT    , PMBUS_WORD_TRANSACTION,    0xDEAD, 0xDEAD, "POUT_OP_WARN_LIMIT" },  
    { PIN_OP_WARN_LIMIT     , PMBUS_WORD_TRANSACTION,    0xDEAD, 0xDEAD, "PIN_OP_WARN_LIMIT" },  
    { STATUS_WORD           , PMBUS_WORD_TRANSACTION,    0xDEAD, 0xDEAD, "STATUS_WORD" },  
    { STATUS_VOUT           , PMBUS_BYTE_TRANSACTION,    0xDEAD, 0xDEAD, "STATUS_VOUT" }, 
    { STATUS_IOUT           , PMBUS_BYTE_TRANSACTION,    0xDEAD, 0xDEAD, "STATUS_IOUT" }, 
    { STATUS_INPUT          , PMBUS_BYTE_TRANSACTION,    0xDEAD, 0xDEAD, "STATUS_INPUT" }, 
    { STATUS_TEMPERATURE    , PMBUS_BYTE_TRANSACTION,    0xDEAD, 0xDEAD, "STATUS_TEMPERATURE" }, 
    { STATUS_CML            , PMBUS_BYTE_TRANSACTION,    0xDEAD, 0xDEAD, "STATUS_CML" }, 
    { STATUS_OTHER          , PMBUS_BYTE_TRANSACTION,    0xDEAD, 0xDEAD, "STATUS_OTHER" }, 
    { STATUS_MFG_SPECIFIC   , PMBUS_BYTE_TRANSACTION,    0xDEAD, 0xDEAD, "STATUS_MFG_SPECIFIC" }, 
    { STATUS_FAN_1_2        , PMBUS_BYTE_TRANSACTION,    0xDEAD, 0xDEAD, "STATUS_FAN_1_2" }, 
    { READ_VIN              , PMBUS_WORD_TRANSACTION,    0xDEAD, 0xDEAD, "READ_VIN" },  
    { READ_IIN              , PMBUS_WORD_TRANSACTION,    0xDEAD, 0xDEAD, "READ_IIN" },  
    { READ_VOUT             , PMBUS_WORD_TRANSACTION,    0xDEAD, 0xDEAD, "READ_VOUT" },  
    { READ_IOUT             , PMBUS_WORD_TRANSACTION,    0xDEAD, 0xDEAD, "READ_IOUT" },  
    { READ_TEMPERATURE1     , PMBUS_WORD_TRANSACTION,    0xDEAD, 0xDEAD, "READ_TEMPERATURE1" },  
    { READ_TEMPERATURE2     , PMBUS_WORD_TRANSACTION,    0xDEAD, 0xDEAD, "READ_TEMPERATURE2" },  
    { READ_TEMPERATURE3     , PMBUS_WORD_TRANSACTION,    0xDEAD, 0xDEAD, "READ_TEMPERATURE3" },  
    { READ_FAN_SPEED_1      , PMBUS_WORD_TRANSACTION,    0xDEAD, 0xDEAD, "READ_FAN_SPEED_1" },  
    { READ_POUT             , PMBUS_WORD_TRANSACTION,    0xDEAD, 0xDEAD, "READ_POUT" },  
    { READ_PIN              , PMBUS_WORD_TRANSACTION,    0xDEAD, 0xDEAD, "READ_PIN" },  
    { PMBUS_REVISION        , PMBUS_BYTE_TRANSACTION,    0xDEAD, 0xDEAD, "PMBUS_REVISION" }, 
    { READ_VSB_12V          , PMBUS_WORD_TRANSACTION,    0xDEAD, 0xDEAD, "READ_VSB_12V" },  
    { FIRMWARE_REVISION     , PMBUS_WORD_TRANSACTION,    0xDEAD, 0xDEAD, "FIRMWARE_REVISION" },
    { 0, 0, 0, 0 }};

/* from mcp_utils.h */
#define RP1RUVE_PEM_0_EEPROM_ADDR      0x53
#define RP1RUVE_PEM_0_ADDR             0x5B
#define RP1RUVE_PEM_1_EEPROM_ADDR      0x53
#define RP1RUVE_PEM_1_ADDR             0x5B
#define RP1RUVE_PEM_0_EEPROM_ADDR_LOG  0x53 
#define RP1RUVE_PEM_0_ADDR_LOG         0x5B 
#define RP1RUVE_PEM_1_EEPROM_ADDR_LOG  0xD3 
#define RP1RUVE_PEM_1_ADDR_LOG         0xDB 

#define I2C_SMBUS_BYTE		    1
#define I2C_SMBUS_BYTE_DATA	    2 
#define I2C_SMBUS_WORD_DATA	    3

#define MCP_IDPROM_SIZE   256

extern uint32_t get_psu_i2c_struct (n2g_i2c_if_t *i2c_if, uint32_t psu_type);

/**********************************************************************
 *
 * Function: rp1ruve_pem_check_offset
 *
 * Description: check whether the command is available on PEM reigster 
 *              table. 
 *
 * Inputs: offset - command offset 
 *         transaction type - return the transaction type for current 
 *                            command.
 *
 * Outputs: PASSED - offset hit the command 
 *          FAILED - offset is mismatch 
 *
 **********************************************************************
 */
static int rp1ruve_pem_check_offset (int offset, uint32_t *transaction_type) {

    int i = 0;

    while ( i < (sizeof(rp1ruve_pem_reg)/sizeof(rp1ruve_pem_register_t)) ) {
	if ( rp1ruve_pem_reg[i].command_code == offset ) {
	    *transaction_type = rp1ruve_pem_reg[i].transaction_type;
	    return PASSED;
	}

	i ++;
    }
    return FAILED;
}

#ifdef ASR1000
static int rp1ruve_i2c_read( char dev, uint32_t addr, int alen, char *buffer, int len, int usec ) {
    int rv;
    unsigned long save = disable_msr_interrupts();
    rv = i2c_rd(dev, addr, alen, buffer, len, usec);
    enable_msr_interrupts(save);
    return rv;
}

static int rp1ruve_i2c_write( uchar dev, uint32_t addr, int alen, char *buffer, uint32_t len ) {
    int rv;
    unsigned long save = disable_msr_interrupts();
    rv = i2c_write(dev, addr, alen, buffer, len);
    enable_msr_interrupts(save);
    return rv;
}
#endif  /* ASR1000 */

/**********************************************************************
 *
 * Function: rp1ruve_pem_read
 *
 * Description: read PEM register via I2C
 *
 * Inputs: ps - PEM number.
 *         command_code - command code.
 *         data - buffer to store the read data. 
 *
 * Outputs: PASSED/FAILED
 *
 **********************************************************************
 */
int rp1ruve_pem_read (int ps, uint32_t command_code, uint16_t* data) {

    uint32_t transaction_type;
    n2g_i2c_if_t i2c_if;
    uint32_t rc = FAILED;

    switch (ps){
    case 1:
        rc = get_psu_i2c_struct(&i2c_if, PEM0_MCNTRL);
        break;
    case 2:
        rc = get_psu_i2c_struct(&i2c_if, PEM1_MCNTRL);
        break;
    default:
        printf("*** %s:%d Got Unknown PSU no.(%d)\n",
               __FUNCTION__, __LINE__, ps);
        break;
    }

    if (rc == FAILED)
        return rc;
    
    /* it is necessary to get the size from transaction_type */
    if ( rp1ruve_pem_check_offset( command_code, &transaction_type )) {
	cterr('f', 0,"PEM invalid offset 0x%x\n", command_code);
	return FAILED;
    }
 
    /*
    if(rp1ruve_i2c_read( dev_addr, command_code, 1, (char *)data, transaction_type, TIME_OUT) ) {
        cterr('f', 0,"I2C read failed");    
	return TEST_FAILED;
    }
    */

    i2c_if.buf = (char *)data;
    i2c_if.size = (uint16_t)transaction_type;
    i2c_if.offset = command_code;

    rc = n2g_i2c_read(&i2c_if);
    if (rc != PASSED) {
        printf("*** %s: Failed to read PSU (rc = %#x).", __FUNCTION__, rc);
	return rc;
    }

    return PASSED;
}


/**********************************************************************
 *
 * Function: rp1ruve_pem_write
 *
 * Description: write data to PEM buffer.
 *
 * Inputs: ps - PEM number.
 *         command_code - command code.
 *         data - data buffer for PEM reigster writing.
 *
 * Outputs: PASSED/FAILED
 *
 **********************************************************************
 */
int rp1ruve_pem_write (int ps, uint32_t command_code, uint16_t data ) {

    uint8_t buf[2];
    uint32_t transaction_type;
    n2g_i2c_if_t i2c_if;
    uint32_t rc = TEST_FAILED; 

    /*Setup I2C API parameter struct*/
    switch (ps){
    case 1:
        rc = get_psu_i2c_struct(&i2c_if, PEM0_MCNTRL);
        break;
    case 2:
        rc = get_psu_i2c_struct(&i2c_if, PEM1_MCNTRL);
        break;
    default:
        printf("*** %s:%d Got Unknown PSU no.(%d)\n",
               __FUNCTION__, __LINE__, ps);
        return rc;
    }

    
    if ( rp1ruve_pem_check_offset( command_code, &transaction_type )) {
	cterr('f', 0,"PEM invalid offset 0x%x\n", command_code);
	return TEST_FAILED;
    }

    buf[0] = data & 0xFF;
    buf[1] = (data & 0xFF00) >> 8;

    /* If NO_DATA_TRANSACTION (0), need to call send byte I2C command */ 
    if ( transaction_type == PMBUS_NO_DATA_TRANSACTION ) {
	transaction_type = PMBUS_BYTE_TRANSACTION;	
    }

    i2c_if.buf = (char *)&buf[0];
    i2c_if.size = (uint16_t)transaction_type;
    i2c_if.offset = command_code;

    rc = n2g_i2c_write(&i2c_if);
    if (rc != PASSED) {
        cterr('f', 0, "PSU write %#x @ %#x failed. rc = %#x",
		data, command_code, rc);
        return(FAILED);
    }

    return TEST_PASSED;
}

#ifdef ASR1000
/*
 * pem_write_eerom()
 *
 * Write PEM's EEPROM via ( ICH9 SMBus => PCA9548 I2C mux ).
 * PEM0 => PT3 of PCA9548
 * PEM1 => PT4 of PCA9548
 * of local buffer.  Note ps parameter = 0 or 1 for Power Supply #.
 *
 * Return: 0 - Write OK
 *         1 - Write Failed 
 */
static int rp1ruve_pem_write_eeprom_old (uint8_t *idprom, int size, uint32_t unused, int ps)
{
    int rc;
    uint offset = 0;
    int alen = 1;
    unsigned long save;
  
    save = disable_msr_interrupts();

    /* Write all 256 bytes of EEPROM */
    for(offset=0;offset < size; ++offset){
	rc = i2c_write( pem_eeprom_addr(ps), offset, alen, &idprom[offset], 1);
	delay_ms(11);
    }

    enable_msr_interrupts(save);

    return (rc);
}
#endif  /* ASR1000 */


///////////1111111111111111111111111111
int write_pem_eeprom_util(void) {

    uchar  data[4], size, ps, buf_util[512];
    n2g_i2c_if_t i2c_if;
    uint32_t rc = FAILED, offset;

    memset((uchar *)buf_util, 0, 512);
    memset((uchar *)data, 0, 4);

    ps = getdec_answer("Select PEM PSU ", 0, 0, 1);

    switch (ps){
    case 1:
        rc = get_psu_i2c_struct(&i2c_if, PEM0_EEPROM);
        break;
    case 2:
        rc = get_psu_i2c_struct(&i2c_if, PEM1_EEPROM);
        break;
    default:
        printf("*** %s:%d Got Unknown PSU no.(%d)\n",
               __FUNCTION__, __LINE__, ps);
        return rc;
    }

    printf(" Get PEM PSU%d i2c information \n", ps);

    printf(" i2c_dev = 0x%x, i2c_ctrl = %d, mux = %d\n", 
        i2c_if.i2c_dev, i2c_if.i2c_ctrl, i2c_if.mux);

    if (getc_answer("offset equal to -1 ?", "yn", 'n') == 'y' ) {
        i2c_if.offset = -1;
    } else  {
        offset = gethex_answer("Write offset ", 0, 0, 0xff);
    }
    
    size = getdec_answer("Write szie ", 1, 1, 4);

    if (getc_answer("Fill all(4) data buffers?", "yn", 'n') == 'y' ) {
        data[0] = gethex_answer("Write data[0] ", 0, 0, 0xff);
        data[1] = gethex_answer("Write data[1] ", 0, 0, 0xff);
        data[2] = gethex_answer("Write data[2] ", 0, 0, 0xff);
        data[3] = gethex_answer("Write data[3] ", 0, 0, 0xff);
    } else {
        data[0] = gethex_answer("Write data[0] ", 0, 0, 0xff);
    }
           

    /* Max total wrtie data is 512 */
    i2c_if.buf = (char *)&data[0];
    i2c_if.size = size;
    i2c_if.offset = offset;

    rc = n2g_i2c_write(&i2c_if);
    if (rc != PASSED) {
        cterr('f', 0, "PSU eeprom write failed");
        return(FAILED);
    }

    msleep(100);

    if (getc_answer("read back immediately?", "yn", 'n') == 'y' ) {
        printf("reading back....\n");
        i2c_if.offset = 0;
        i2c_if.buf = (char *)&buf_util[0];
        i2c_if.size = 512;

        rc = n2g_i2c_read(&i2c_if);
        if (rc != PASSED) {
            printf("*** %s: Failed to read PSU eeprom (rc = %#x).", __FUNCTION__, rc);
            return rc;
        }

        mcp_print_buffer_data_with_ascii( &buf_util[0], 512, 16, 1 );
    } 

    return (PASSED);
}


/**********************************************************************
 *
 * Function: rp1ruve_pem_write_eeprom_new
 *
 * Description: write data to PEM eeprom 
 *
 * Inputs: idprom - dumpped array from PEM eeprom
 *         tot_wr_size - total write byte to PEM eeprom
 *         unused - unused. 
 *         ps - PEM num. 
 *
 * Outputs: PASSED/FAILED
 *
 **********************************************************************
 */
static int rp1ruve_pem_write_eeprom_new (uint8_t *idprom, int tot_wr_size, uint32_t unused, int ps) {

    uint32_t ia;
    uint8_t  addr_hold[3];
    n2g_i2c_if_t i2c_if;
    uint32_t rc = FAILED;

    /*Setup I2C API parameter struct*/
    switch (ps){
    case 1:
        rc = get_psu_i2c_struct(&i2c_if, PEM0_EEPROM);
        break;
    case 2:
        rc = get_psu_i2c_struct(&i2c_if, PEM1_EEPROM);
        break;
    default:
        printf("*** %s:%d Got Unknown PSU no.(%d)\n",
               __FUNCTION__, __LINE__, ps);
        return rc;
    }

    /* Max total wrtie data is 512 */
    for ( ia = 0; ia < tot_wr_size; ia++) {
        addr_hold[0] = ((ia & 0x0FF00)>> 8);  /* addr high */
        addr_hold[1] = (ia & 0xFF);          /* addr low */
        addr_hold[2] = idprom[ia];        /* data */

        i2c_if.offset = -1;   /* we passs the offset(addr) from data pkt */
        i2c_if.buf = (char *)&addr_hold[0];
        i2c_if.size = sizeof(addr_hold);
        rc = n2g_i2c_write(&i2c_if);
        if (rc != PASSED) {
            cterr('f', 0, "PSU eeprom write %#x @ page%d %#x failed. rc = %#x",
                    addr_hold[2], addr_hold[0], addr_hold[1], rc);
            return(FAILED);
        }
        msleep(10);  /* to avoid writing too frequently */

    }
    return (PASSED);

}

/**********************************************************************
 *
 * Function: rp1ruve_pem_write_eeprom
 *
 * Description: wrapper for writing data to PEM eeprom
 *
 * Inputs: idprom - dumpped array from PEM eeprom
 *         size - total write byte to PEM eeprom
 *         unused - unused.
 *         ps - PEM num.
 *
 * Outputs: PASSED/FAILED
 * NOTE: rp1ruve_pem_write_eeprom_old is for eeprom size = 256 bytes.
 *
 **********************************************************************
 */
int rp1ruve_pem_write_eeprom (uint8_t *idprom, int size, uint32_t unused, int ps)
{
    int rc = FAILED;
#ifdef ASR1000
    /*
    unsigned long save;

    save = disable_msr_interrupts();

   
    if ( PEM_EEPROM_SIZE_OLD < size )
	rc = rp1ruve_pem_write_eeprom_new ( idprom, size, unused, ps);
    else
	rc = rp1ruve_pem_write_eeprom_old ( idprom, size, unused, ps);
    
    enable_msr_interrupts(save);*/
#endif /* ASR1000 */

    rc = rp1ruve_pem_write_eeprom_new ( idprom, size, unused, ps);

    return (rc);
}

#ifdef ASR1000
/*
 * pem_read_eeprom()
 *
 * Read PEM's EEPROM via ( ICH9 SMBus => PCA9548 I2C mux ).
 * PEM0 => PT3 of PCA9548
 * PEM1 => PT4 of PCA9548
 * of local buffer.  Note ps parameter = 0 or 1 for Power Supply #.
 *
 * Return: PASSED = 0 : Read OK
 *         FAILED = 1 : Read Failed 
 */
static int rp1ruve_pem_read_eeprom_old (uint8_t *idprom, int size, uint32_t addr, int unused, int ps)
{
    int rc = TEST_PASSED;
    int alen = 1;
    int offset;

    /* Read 'size' bytes of EEPROM starting at 'addr' */
    for(offset=addr;offset < (addr+size); ++offset){
	rc = i2c_read( pem_eeprom_addr(ps), offset, alen, &idprom[offset], 1);

	if (rc != PASSED) {
	    cterr('f', 0,"i2c_read() FAILED");
	    return TEST_FAILED;
	}
    }

    return (rc);

} /* pem_read_eeprom() */
#endif  /* ASR1000 */

/**********************************************************************
 *
 * Function: rp1ruve_pem_read_eeprom_new
 *
 * Description: wrapper for reading data to PEM eeprom
 *
 * Inputs: idprom - dumpped array from PEM eeprom
 *         size - total write byte to PEM eeprom
 *         unused - unused.
 *         ps - PEM num.
 *
 * Outputs: PASSED/FAILED
 * NOTE: rp1ruve_pem_write_eeprom_old is for eeprom size = 256 bytes.
 *
 **********************************************************************
 */
static int rp1ruve_pem_read_eeprom_new (uint8_t *idprom, int numBytes, uint32_t addr_input, int unused, int ps) {

    n2g_i2c_if_t i2c_if;
    uint32_t rc = TEST_FAILED;
    uchar data[2];

    memset((uchar *)data, 0, 2);

    /*Setup I2C API parameter struct*/
    switch (ps){
    case 1:
        rc = get_psu_i2c_struct(&i2c_if, PEM0_EEPROM);
        break;
    case 2:
        rc = get_psu_i2c_struct(&i2c_if, PEM1_EEPROM);
        break;
    default:
        printf("*** %s:%d Got Unknown PSU no.(%d)\n",
               __FUNCTION__, __LINE__, ps);
        return rc;
    }

    i2c_if.offset = -1;

    /* we must tell PSU the start address first, write 2 bytes data stand for start addr */
    /* if address > 255, addr offset move to next page */
    if (addr_input > 0xFF)  {
        data[0] = 1;
        data[1] = (addr_input - 0xFF);
    } else {
        data[0] = 0; 
        data[1] = addr_input;
    }

    i2c_if.size = 2;
    i2c_if.buf = (char *)&data[0];
    rc = n2g_i2c_write(&i2c_if);
    if (rc != PASSED) {
        printf("*** %s: Failed to write PSU eeprom (rc = %#x).", __FUNCTION__, rc);
        return rc;
    }

    msleep(10);


    i2c_if.buf = (char *)&idprom[0];
    i2c_if.size = numBytes;
 
    rc = n2g_i2c_read(&i2c_if);
    if (rc != PASSED) {
        printf("*** %s: Failed to read PSU eeprom (rc = %#x).", __FUNCTION__, rc);
        return rc;
    }

    return (0);
}

/**********************************************************************
 *
 * Function: rp1ruve_pem_read_eeprom
 *
 * Description: wrapper for reading data from PEM eeprom
 *
 * Inputs: idprom - dumpped array from PEM eeprom
 *         size - total write byte to PEM eeprom
 *         addr - start addr
 *         unused - unused
 *         ps - PEM num.
 *
 * Outputs: PASSED/FAILED
**********************************************************************
*/
int rp1ruve_pem_read_eeprom (uint8_t *idprom, int size, uint32_t addr, int unused, int ps)
{
    int rc = TEST_PASSED;

    rc = rp1ruve_pem_read_eeprom_new ( idprom, size, addr, unused, ps );

    return (rc);

} /* pem_read_eeprom() */


/**********************************************************************
 *
 * Function: rp1ruve_pem_display_eeprom
 *
 * Description: wrapper for reading data from PEM eeprom
 *
 * Inputs: idprom - dumpped array from PEM eeprom
 *         size - eeprom size
 *         ps - PEM num.
 *
 * Outputs: PASSED/FAILED
**********************************************************************
*/
void rp1ruve_pem_display_eeprom (uint8_t *idprom, int size, int ps)
{

    /* Note: Display only 256 bytes of TLV Formated ID PROM */
    printf("\nPS%d EEPROM:", ps);
    idprom_print_all_fields(idprom + IDPROM_TLV_OFFSET,
			    MCP_IDPROM_SIZE-IDPROM_TLV_OFFSET);

    /* # bytes display in raw hex data is determined by 'size' parameter */
    printf("\n\nPS%d EEPROM format version %d", ps, idprom[0]);
    printf("\nPS%d EEPROM contents (hex):", ps);

    mcp_print_buffer_data_with_ascii( &idprom[0], size, 16, 1 );

} /* pem_display_eeprom() */

/**********************************************************************
 *
 * Function: rp1ruve_pem_data_read_all
 *
 * Description: read all the PEM commands data
 *
 * Inputs: ps - PEM num.
 *
 * Outputs: PASSED/FAILED
**********************************************************************
*/
int rp1ruve_pem_data_read_all (int ps) {

    int i = 0;
    int rc = 0;
    uint16_t data;
    
    while ( rp1ruve_pem_reg[i].command_code ) {
	if ( rp1ruve_pem_reg[i].transaction_type != PMBUS_NO_DATA_TRANSACTION ) {
	    if ( ps == 0 )
		rp1ruve_pem_reg[i].data1 = 0xDEAD;
	    else
		rp1ruve_pem_reg[i].data2 = 0xDEAD;
	}
	i ++;
    }


    i = 0;
    while ( i < (sizeof(rp1ruve_pem_reg)/sizeof(rp1ruve_pem_register_t)) ) {
	if ( rp1ruve_pem_reg[i].transaction_type != PMBUS_NO_DATA_TRANSACTION ) {

	    rc |= rp1ruve_pem_read( ps, rp1ruve_pem_reg[i].command_code, &data );

	    if ( ps == 0 )
		rp1ruve_pem_reg[i].data1 = data;
	    else
		rp1ruve_pem_reg[i].data2 = data;
	}
	i ++;
    }

    return rc;
}

/**********************************************************************
 *
 * Function: rp1ruve_pem_display
 *
 * Description: dump the PEM register.
 *
 * Inputs: ps_mask - PEM num.
 *
 * Outputs: PASSED/FAILED
**********************************************************************
*/
void rp1ruve_pem_display (int ps_mask) {

    int i = 0;
    int display0=0, display1=0;

    if ( ps_mask & 0x1 ) 
	display0 = 1;

    if ( ps_mask & 0x2 )
	display1 = 1;

    printf("       PEM0            PEM1 \n\r" );

    /* Upper function should call read_all before calling this */
    while ( i < (sizeof(rp1ruve_pem_reg)/sizeof(rp1ruve_pem_register_t)) ) {
	if ( rp1ruve_pem_reg[i].transaction_type != PMBUS_NO_DATA_TRANSACTION ) {

	    printf("[0x%02X] ", rp1ruve_pem_reg[i].command_code );

	    if ( display0 ) 
		printf("0x%04X (%5d)  ", rp1ruve_pem_reg[i].data1, rp1ruve_pem_reg[i].data1 );
	    else
		printf("n/a             ");

	    if ( display1 ) 
		printf("0x%04X (%5d) ", rp1ruve_pem_reg[i].data2, rp1ruve_pem_reg[i].data2 );
	    else
		printf("n/a ");

	    printf("- %s \n\r", rp1ruve_pem_reg[i].msg );
	}
	i ++;
    }

}

/**********************************************************************
 *
 * Function: rp1ruve_pem_fan_set
 *
 * Description: set fan speed.
 *
 * Inputs: ps_mask - PEM num.
 *         speed - speed 
 *
 * Outputs: PASSED/FAILED
**********************************************************************
*/
int rp1ruve_pem_fan_set (int ps_num, uint8_t speed ) {

    return rp1ruve_pem_write( ps_num, FAN_COMMAND_1, (uint16_t)speed );
}

/**********************************************************************
 *
 * Function: rp1ruve_pem_fan_get
 *  
 * Description: get fan speed.
 *  
 * Inputs: ps_mask - PEM num.
 *         speed - speed 
 *
 * Outputs: PASSED/FAILED
**********************************************************************
*/
int rp1ruve_pem_fan_get (int ps_num, uint8_t *speed ) {

    uint16_t data;

    if ( rp1ruve_pem_read( ps_num, FAN_COMMAND_1, &data ))
	return TEST_FAILED;

    *speed = (uint8_t)data;

    return TEST_PASSED;
}

/*
 *------------------------------------------------------------------
 * $Log: platform_pem_utils.c,v $
 * Revision 1.2  2018/05/18 09:25:00  alpeng
 *  Neptune merge to trunk with tag <neptune-branch-0518>
 *
 * Revision 1.1.2.1  2016/06/02 22:04:02  jskow
 * Move Overlord/x86 specific files to Neptune/x86.
 *
 * Revision 1.9  2014/07/01 10:32:30  danchung
 * Support Juno AC+IP supply registers read utility for different page
 *
 * Revision 1.8  2013/12/18 06:32:59  hroni
 * use toolchain in router/bin to do make with TOOLS_VER=c4.5.3-p1, TOOLS_ARCH=x86_64
 *
 * Revision 1.7  2013/11/26 08:40:38  hroni
 * fix compiler warning
 *
 * Revision 1.6  2013/08/22 07:43:30  alpeng
 * support both MENU and CLI PSU cookie on Juno
 *
 * Revision 1.5  2013/08/14 07:25:09  alpeng
 * fixed compile error
 *
 * Revision 1.4  2013/08/14 06:01:35  alpeng
 * support PSU ucontroller write
 *
 * Revision 1.3  2013/08/13 07:19:30  alpeng
 * support i2c scan on PEM ucontroller, update the code for new PSU eeprom w/r and ucontroller read
 *
 * Revision 1.2  2013/07/16 08:02:09  alpeng
 * support i2c read and read cookie for juno psu
 *
 * Revision 1.1  2013/05/31 12:43:15  danchung
 * Porting PSU source code from Nightster for Juno.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
