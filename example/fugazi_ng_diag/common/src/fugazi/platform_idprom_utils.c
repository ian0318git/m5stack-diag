/* $Id: platform_idprom_utils.c,v 1.2 2021/06/02 08:22:35 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/platform_idprom_utils.c,v $
 *------------------------------------------------------------------
 *
 * platform_idprom_utils.c: 
 *               This file contains various routines used to
 *               interface to the Serial EEPROM via TLVs.
 *
 * May 2013 - porting the code from Nightster
 *
 * Copyright (c) 2019-2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 * Original Author: Dave DeSimone
 */
#include <string.h> /* memcpy */
#include <stdio.h> 
#include "types.h"  /* e.g. uchar  */
#include "common.h" /* TRUE/FALSE */ 

#include "platform_idprom.h"
#include "platform_idprom_utils.h"


/*
 * Basic access routines.
 */
/*
 * getshort_inline
 *
 * read short from misaligned memory
 */
static __attribute__((unused)) inline
unsigned short getshort_inline(register void const *ptr)
{
    /*
     * The generated code turns out to be no worse than the
     * in-line assembly code. Besides, the in-line assembly
     * code causes some problems when further included by
     * getlong_inline(). So decided to use C code.
     */
    return ( ((uchar *)ptr)[0] << 8 | ((uchar *)ptr)[1] );
}

/*
 * putshort_inline
 *
 * write short to misaligned memory
 */
static __attribute__((unused)) inline
void putshort_inline(register void *ptr, ushort value)
{
    /*
     * The generated code turns out to be no worse than the
     * in-line assembly code. Besides, the in-line assembly
     * code causes some problems when further included by
     * putlong_inline(). So decided to use C code.
     */
    ((uchar *)ptr)[1] = (uchar)value;
    ((uchar *)ptr)[0] = (uchar)(value >> 8);
}

/*
 * getlong_inline
 *
 * write long to misaligned memory
 */
static __attribute__((unused)) inline
unsigned int getlong_inline(register void const *ptr)
{
    return( getshort_inline(ptr) << 16 | getshort_inline((ushort *)ptr + 1) );
}

/*
 * putlong_inline
 *
 * write long to misaligned memory
 */
static __attribute__((unused)) inline
void putlong_inline(register void *ptr, register unsigned long value)
{
    putshort_inline(ptr, (ushort)(value >> 16));
    putshort_inline((ushort *)ptr+1, (ushort)value);
}


#define GETSHORT(ptr)		getshort_inline(ptr)
#define PUTSHORT(ptr, val)	putshort_inline((ptr), (val))
#define GETLONG(ptr)		getlong_inline(ptr)
#define PUTLONG(ptr, val)	putlong_inline((ptr), (val))


/**********************************************************************
 *
 * Function: idprom_get_field_data()
 *
 * Description: get specified TLV data.
 *
 * Inputs:  data - idprom data buf
 *          data_length - data length
 *          search_type - the data type
 *          return_buffer - the read data buf
 *          maxlength - max data length 
 *          exact - exact match the length
 *  
 * Outputs: return TRUE if data found, FALSE otherwise.
 *  
 **********************************************************************
 */ 
int idprom_get_field_data (uchar *data, int data_length, int searched_type,
                void *return_buffer, int maxlength, boolean exact)
{
    idprom_tlv_t *tlv;
    uchar *the_end;
    int size = 0;
    int type;
    
    the_end = data + data_length;

    tlv = (idprom_tlv_t *) data;

    while ((tlv->tlv_type != T_IDPROM_EOD) && (data < the_end)) {

        if ((data = pem_idprom_get_entry(data,the_end,&type,&size,NULL)) == NULL) {
            break;
	}

        if (type == searched_type) {

            /*
             * Safeguard: field length must not exceed return length.
             * Should eventually display error message.
             */
	    if (exact && (maxlength != size)) {
                return IDPROM_RET_BAD_SIZE;
            }
            if (!exact && (maxlength < size)) {
                return IDPROM_RET_BAD_SIZE;
            }
            memcpy(return_buffer, data, size);

	    return (size);
	}

        data += size;
        tlv = (void *)data;
    }

    /*
     * Return 0x0 -- meaning didn't find what you were
     * looking for.
     */
    return (0x0);
}


/*
 * Function : idprom_update_field_data()
 *
 * Description Update TLV field.
 * Return TRUE if successful, FALSE otherwise.
 * If successful, set f_start and f_length to indicate which EEPROM data
 * needs to be written back.
 *
 * Inputs:  data - idprom data buf
 *          data_length - data length
 *          permit_new - permit update nore file
 *          field_type - the data type
 *          field_length - the data length
 *          field_data - write data 
 * NOTE: the field_length parameter is a complete one defined in 
 *	 "Cisco Generic ID PROM Specification" page 7, i.e. including
 *	 display field (bit 6 & 7) and length (bit 0-5).
 */
boolean idprom_update_field_data (uchar *data, int data_length,
        boolean permit_new, int field_type, int field_length,
        void *field_data, ushort *f_start, ushort *f_length)
{
    uchar *start, *the_end;
    idprom_tlv_t *tlv;
    int f_type, size, hdr_length, disp_type;

    start = data;
    the_end = data + data_length;
    disp_type = field_length & 0xc0;		/* abstract display bits from lenth field */
    field_length &= ~0xc0; 			/* clear these two bits to get real length */

    while (data < the_end) {
        tlv = (idprom_tlv_t *)data;
        if (tlv->tlv_type == T_IDPROM_EOD) {
            /*
             * End of data reached.
             * If field addition is permitted, create new data field.
             * Otherwise, return FALSE.
             */
            if (permit_new) {
                /*
                 * Type is encoded as 0xXXYY, where XX is the extension
                 * and YY is the field type. The TLV header will consume
                 * YY bytes for the extension (one byte for each increment)
                 * plus the size of the actual field.
                 */
                hdr_length = (field_type >> 8) + sizeof(tlv->tlv_type);
                switch (field_type & T_IDPROM_TYPE_MASK) {
                case T_IDPROM_BYTE:
                    size = sizeof(uchar);
                    break;
                case T_IDPROM_SHORT:
                    size = sizeof (ushort);
                    break;
                case T_IDPROM_LONG:
                    size = sizeof (ulong);
                    break;
                default:
                    size = field_length;
                    hdr_length += sizeof(tlv->tlv_length);
                    break;
                }
                if (data + hdr_length + field_length >= the_end
                                        || size != field_length) {
                    /*
                     * Not enough space or wrong/invalid data field size.
                     * Note: Need to have space for new EOD as well.
                     */
                    return FALSE;
                }
                *f_start = data - (uchar *)start;
                *f_length = hdr_length + size + sizeof(tlv->tlv_type);
 
                /*
                 * First, write field extension values and field type.
                 */
                while (field_type >= T_IDPROM_EXT_OFFSET) {
                    field_type -= T_IDPROM_EXT_OFFSET;
                    tlv->tlv_type = T_IDPROM_EXTENSION;
                    data += sizeof(tlv->tlv_type);
                    tlv = (void *)data;
                }
                tlv->tlv_type = (uchar)field_type;
                /*
                 * Now copy field data.
                 * Note that data needs to point to the first byte of
                 * the next TLV field. Update accordingly.
                 */
                switch (field_type & T_IDPROM_TYPE_MASK) {
                case T_IDPROM_BYTE:
                    tlv->tlv_byte = *(uchar *)field_data;
                    break;
                case T_IDPROM_SHORT:
                    PUTSHORT(&tlv->tlv_short, *(ushort *)field_data);
                    break;
                case T_IDPROM_LONG:
                    PUTLONG(&tlv->tlv_long, *(ulong *)field_data);
                    break;
                default:
                    tlv->tlv_length = size | disp_type;
                    data += sizeof(tlv->tlv_length);
                    memcpy(tlv->tlv_data, field_data, size);
                    break;
                }
                data += size + sizeof(tlv->tlv_type);
                tlv = (idprom_tlv_t *)data;
                tlv->tlv_type = T_IDPROM_EOD;
                return TRUE;
            }
            return FALSE;
        }
        if ((data = pem_idprom_get_entry(data, the_end, &f_type, &size, NULL))
                                                                == NULL) {
            /*
             * Field does not exist.
             * Also, there is a problem with the data format,
             * since otherwise EOD would be returned.
             * Therefore, return FALSE even if it is permitted to add
             * the field.
             */
            return FALSE;
        }
        if (f_type == field_type) {
            /*
             * Found data field.
             * Update field contents.
             * Note: Field size changes not supported.
             */

            if (f_type == T_IDPROM_PROD_NUM){
                if (size < field_length) { /* LC pid length is not fixed */
                    return FALSE;
                }
            }else{
                if (size != field_length) { 
                    return FALSE;
                }
            }

            memcpy(data, field_data, field_length);
            *f_start = data - (uchar *)start;
            *f_length = (uchar)field_length;
            return TRUE;
        }
        data += size;
    }
    return FALSE;
}

void print_hex (uchar *ep, int len)
{
    int i;
 
    for (i=0; i<len; i++) {
        if (i && !(i&7)) {
            printf("\n\t%27s", "");
        }
        printf("%02x ", ep[i]);
    }
}


void print_ascii (uchar *ep, int len)
{
    int i;
    for (i=0; i<len && ep[i]; i++) {
        printf("<%02x>", ep[i]);
    }
#if 0  /* zzzz */
    for (i=0; i<len && ep[i]; i++) {
        if (isprint(ep[i]))
            printf("%c", ep[i]);
        else
            printf("<%02x>", ep[i]);
    }
#endif 
}

/* This routine is use to extract the MAC address from the contents
 * of the EEPROM and put it into a string as an Ethernet style address.
 * (e.g. 1111.2222.3333).
 */
void getMacString (uchar *ep, uchar *mac) 
{
    char hexnums[] = "0123456789ABCDEF";
 
    mac[0] = hexnums[((ep[0]>>4) & 0x0f)];
    mac[1] = hexnums[(ep[0] & 0x0f)];
    mac[2] = hexnums[((ep[1]>>4) & 0x0f)];
    mac[3] = hexnums[(ep[1] & 0x0f)];
    mac[4] = '.';
    mac[5] = hexnums[((ep[2]>>4) & 0x0f)];
    mac[6] = hexnums[(ep[2] & 0x0f)];
    mac[7] = hexnums[((ep[3]>>4) & 0x0f)];
    mac[8] = hexnums[(ep[3] & 0x0f)];
    mac[9] = '.';
    mac[10]= hexnums[((ep[4]>>4) & 0x0f)];
    mac[11]= hexnums[(ep[4] & 0x0f)];
    mac[12]= hexnums[((ep[5]>>4) & 0x0f)];
    mac[13]= hexnums[(ep[5] & 0x0f)];
 
}

/*
 * Function : pem_idprom_get_entry()
 *
 * Description : Starting from pointer to type field, 
 * return pointer to next data field.
 * Also return size and type of field.
 * Input : data = idprom buf
 *         the_end - data end pointer 
 *         type - return data type 
 *         size - return data size 
 *         disp_type - display type
 * Output : return TLV data(PASSED)/NULL(FAILED)
 */
void *pem_idprom_get_entry (uchar *data, uchar *the_end, int *type, int *size,
                int *disp_type)
{
    ushort *p16;
    idprom_tlv_t *tlv;
 
    *type = *size = 0;
    /*
     * First, decode extension fields to extract real type.
     */
    tlv = (idprom_tlv_t *) data;

    while ((tlv->tlv_type == T_IDPROM_EXTENSION) && (data < the_end)) {
        *type += T_IDPROM_EXT_OFFSET;
        data += sizeof(tlv->tlv_type);
        tlv = (idprom_tlv_t *) data;
    }

    if (data >= the_end) {
        return NULL;
    }

    *type |= tlv->tlv_type;
    /*
     * now extract size, basic display type code, and start of data area.
     */
    if (disp_type) {
        *disp_type = S_IDPROM_TYPE_HEX;
    }

    switch (*type & T_IDPROM_TYPE_MASK) {
    case T_IDPROM_BYTE:
        *size = sizeof(uchar);
        data += sizeof(tlv->tlv_type);
        break;
    case T_IDPROM_SHORT:
        *size = sizeof (ushort);
        data += sizeof(tlv->tlv_type);
        break;
    case T_IDPROM_LONG:
        *size = sizeof (ulong);
        data += sizeof(tlv->tlv_type);
        break;
    default:
        if ((*type != T_IDPROM_SPA_FDIAGS) && (*type != T_IDPROM_SPA_ENV))
        {
           if ((tlv->tlv_type == T_IDPROM_SPA_DSIGN0) 
                && (tlv->tlv_length == T_IDPROM_SPA_DSIGN1) 
                && (tlv->tlv_data[0] == T_IDPROM_SPA_DSIGN2))
           {
              *size = sizeof (ushort);
              data += sizeof(tlv->tlv_type);
           }
           else
           {
              *size = tlv->tlv_length & S_IDPROM_SIZE_MASK;
              if (disp_type)
                  *disp_type = tlv->tlv_length & S_IDPROM_TYPE_MASK;
              data = &tlv->tlv_data[0];
           }
        }
        else
        {
           p16 = (ushort*)(&tlv->tlv_length); 
           *size = *p16;
           data += sizeof(idprom_tlv_t)-2;
        }
        break;
    }


    /*
     * Check for minimum size as well as for size exceeding buffer size.
     * Return NULL if size is invalid.
     */
    if (!*size || (data + *size > the_end))
    {
        return NULL;
    }
    return data;
}

/*
 * Function : idprom_print_field()
 * Description : Print single IDPROM field.
 * Input: ep - data buf
 *        type - data type
 *        disp_type - display type 
 *        size - data size
 * Output : None.
 */
void idprom_print_field (uchar *ep, int type, int disp_type, int size)
{
    int i;
    uchar macaddr[] = "1234.5678.9ABC";
 
    switch(type) {
    case T_IDPROM_NUM_SLOTS:
        printf("\n\tNumber of Slots          : %d", ep[0]);
        break;
    case T_IDPROM_FAB_VERSION:
        printf("\n\tFab Version              : %02d", ep[0]);
        break;
    case T_IDPROM_RMA_FAILCODE:
        printf("\n\tRMA Test History         : %02x", ep[0]);
        break;
    case T_IDPROM_RMA_HISTORY:
        printf("\n\tRMA History              : %02x", ep[0]);
        break;
    case T_IDPROM_CONNECTOR_TYPE:
        printf("\n\tConnector Type           : %02x", ep[0]);
        break;
    case T_IDPROM_EHSA_PREF_MSTR:
        printf("\n\tEHSA Preferred Master    : %02x", ep[0]);
        break;
    case T_IDPROM_PWR_SUPPLY:
        printf("\n\tPower Supply Type        : %s", ep[0] ? "DC" : "AC");
        break;
    case T_IDPROM_SPA_FORMAT_REV:
        printf("\n\tSPA IDPROM Format Rev    : %02x", ep[0]);
        break;
    case T_IDPROM_HW_TYPE:
        printf("\n\tController Type          : %d", GETSHORT(&ep[0]));
        break;
    case T_IDPROM_HW_VERSION:
        printf("\n\tHardware Revision        : %d.%d", ep[0], ep[1]);
        break;
    case T_IDPROM_PCB_REVISION:
        printf("\n\tBoard Revision           : ");
        print_ascii(ep, size);
        break;
    case T_IDPROM_MAC_BLKSIZE:
        printf("\n\tMAC Address block size   : %d", GETSHORT(&ep[0]));
        break;
    case T_IDPROM_BOOT_TIMEOUT:
        printf("\n\tBoot Timeout             : %d", GETSHORT(&ep[0]));
        break;
    case T_IDPROM_DEVIATION:
        printf("\n\tDeviation Number         : %d-%d",
               GETSHORT(&ep[0]), GETSHORT(&ep[2]));
        break;
    case T_IDPROM_RMA_NUMBER:
        printf("\n\tRMA Number               : %d-%d-%d-%d",
                                        ep[0], ep[1], ep[2], ep[3]);
        break;
    case T_IDPROM_PCB_PARTNBR_4:
        /*
         * Format is 4 bytes, output xx(1)-yyyy(2)-zz(1)
         */
        printf("\n\t73 level Part Number     : %02d-%04d-%02d",
               ep[0], GETSHORT(&ep[1]), ep[3]);
        break;
    case T_IDPROM_PCB_68_PARTNBR:
        /*
         * Format is 4 bytes, output xx(1)-yyyy(2)-zz(1)
         */
        printf("\n\tTop Assy. Part Number    : %02d-%04d-%02d",
               ep[0], GETSHORT(&ep[1]), ep[3]);
        break;
    case T_IDPROM_PCB_PARTNBR_6:
        /*
         * Format is 6 bytes, output xxx(2)-yyyyy(3)-zz(1)
         */
        printf("\n\t800 level Part Number    : %03d-%05d-%02d",
               GETSHORT(&ep[0]), GETLONG(&ep[1]) & 0x00ffffff, ep[5]);
        break;
    case T_IDPROM_PCB_SERIAL:
        printf("\n\tPCB Serial Number        : ");
        print_ascii(ep, size);
        break;
    case T_IDPROM_CHASSIS_SERIAL:
        printf("\n\tChassis Serial Number    : ");
        print_ascii(ep, size);
        break;
    case T_IDPROM_MACADDR:
        getMacString(ep, macaddr);
        printf("\n\tChassis MAC Address      : ");
        print_ascii(macaddr, 14);
        /* printf("\n\tChassis MAC Address      : %e", ep); */
        break;
    case T_IDPROM_MFG_TEST:
        printf("\n\tManufacturing Test Data  : ");
        print_hex(ep, size);
        break;
    case T_IDPROM_FIELD_DIAGS:
        printf("\n\tField Diagnostics Data   : ");
        print_hex(ep, size);
        break;
    case T_IDPROM_CLEI:
        printf("\n\tCLEI Code                : ");
        print_ascii(ep, size);
        break;
    case T_IDPROM_ENVMON:
        /* Environmental Monitor Data. Do not display   */
        break;
    case T_IDPROM_CALIBRATION:
        printf("\n\tCalibration Data         : ");
        /*
         * Note: Typecast is necessary since values can be negative.
         */
        printf("Minimum: %d dBmV, Maximum: %d dBmV",
                                                (char)ep[0], (char)ep[1]);
        printf("\n\tCalibration values       : ");
        for (i=0; i<ep[2]; i++) {
            printf("0x%04x ", GETSHORT(&ep[3+i*2]));
        }
        break;
    case T_IDPROM_PROD_NUM:
        printf("\n\tProduct Identifier       : ");
        print_ascii(ep, size);
        break;
 
    case T_IDPROM_VERS_ID:
        printf("\n\tVersion Identifier       : ");
        print_ascii(ep, size);
        break;
    case T_IDPROM_UDI_DESCR:
        printf("\n\tUDI Product Description  : ");
        print_hex(ep, size);
        break;
    case T_IDPROM_UDI_NAME:
        printf("\n\tUDI Product Name         : ");
        print_ascii(ep, size);
        break;
    case T_IDPROM_NEW_DEVIATION_NUM:
        printf("\n\tNew Deviation Number     : %d",
               GETLONG(&ep[0]));
        break;
 
    case T_IDPROM_73_REVISION:
        printf("\n\t73 level Revision        : ");
        print_ascii(ep, size);
        break;

    case T_IDPROM_68_REVISION:
        printf("\n\t68 level Revision        : ");
        print_ascii(ep, size);
        break;

    case T_IDPROM_DEV_SPECIFIC:
        printf("\n\tDevice Specific          : ");
        print_hex(ep, size);
        break;

    case T_IDPROM_CSCO_ASSET_MIB:
        printf("\n\tCisco Entity Asset MIB   : ");
        print_hex(ep, size);
        break;

    case T_IDPROM_BASE_MAC_ADDR:
        printf("\n\tBase MAC Address         : ");
        print_hex(ep, size);
        break;

    case T_IDPROM_CLK_FREQ:
        printf("\n\tClock Frequency          : ");
        print_hex(ep, size);
        break;

    case T_IDPROM_PWR_CONSUMPTION:
	printf("\n\tPower Consumption	 : ");
	print_ascii(ep, size);
        break;

    case T_IDPROM_ASST_ALIAS:
        printf("\n\tAsset Alias         	 : ");
        print_hex(ep, size);
        break;
	
    case T_IDPROM_PRCS_LABEL:
        printf("\n\tProcess Label         	 : ");
        print_hex(ep, size);
        break;
	

    case T_IDPROM_DIG_SIGNATURE:
        printf("\n\tDigital Signature        : ");
        print_hex(ep, size);
        break;
	
    case T_IDPROM_SPA_ENV:
	printf("\n\tVoltage Subtype for ENVMON Extended#%d",(char)ep[0]);
	for(i=0;i<(((int)ep[1])/4);i++)
	{
		printf("\n\tVoltage Value #%d ID volt in mv = %d",i+1,*((short*)(ep+2+(i*4))));
		printf("\n\tVoltage Value #%d Shutdown %%  x 10 = %d",i+1,(char)ep[4+(i*4)]);
		printf("\n\tVoltage Value #%d Warning  %%  x 10 = %d",i+1,(char)ep[5+(i*4)]);
	}
        break;

	case T_IDPROM_SPA_FDIAGS:
        printf("\n\tField Diags        	 : ");
        print_hex(ep, size);
        break; 

    default:
        printf("\n\tUnknown Field (type %04x): ", type);
        switch (disp_type) {
        case S_IDPROM_TYPE_HEX:
        default:
            print_hex(ep, size);
            break;
        case S_IDPROM_TYPE_ASCII:
            print_ascii(ep, size);
            break;
        case S_IDPROM_TYPE_DECIMAL:
            /*
             * Either display value as perceived from size or,
             * if size is not associated with a variable type,
             * per byte data with '.' between each value.
             */
            switch(size) {
                case sizeof(uchar):
                    printf("%d", ep[0]);
                    break;
                case sizeof(ushort):
                    printf("%d", GETSHORT(&ep[0]));
                    break;
                case sizeof(ulong):
                    printf("%d", GETLONG(&ep[0]));
                    break;
                default:
                    for (i=0; i<size; i++) {
                        if (i)
                            printf(".");
                        printf("%d", ep[i]);
                    }
            }
            break;
        }
        break;
    }
}

/*
 * Function : idprom_print_field()
 * Description: Print all IDPROM field.
 * Input : data - data buf
 *         length - data length
 * output: None.
 */
void idprom_print_all_fields (uchar *data, int length)
{
    uchar *start, *the_end, *newdata;
    idprom_tlv_t *tlv;
    int type, size, disp_type, index;
 
    start = data;
    the_end = data + length;
 
    tlv = (idprom_tlv_t *)data;
    while ((tlv->tlv_type!=T_IDPROM_EOD) && (data < the_end)) {
        if ((newdata = pem_idprom_get_entry(data, the_end, &type, 
					&size, &disp_type)) == NULL) {
            index = data - start;
            printf("\n\tIDPROM FIELD FORMAT ERROR, index 0x%x", index);
            break;
        }
        data = newdata;
        idprom_print_field(data, type, disp_type, size);
        data += size;
        tlv = (idprom_tlv_t *)data;
    }
}

/*
 * Function : mcp_print_buffer_data_with_ascii
 * Description: Print all IDPROM field with acsii mode.
 * Input : buffer - data buf
 *         size - data buf size
 *         char_per_line - the characters on each line.
 *         extra_space - print extra space
 * output: PASSED.
 */

int mcp_print_buffer_data_with_ascii (uchar *buffer, uint32 size, int char_per_line, int extra_space ) {

    int buffer_size, print_size, line_index, i;

    line_index  = 0;
    buffer_size = size;

    while ( buffer_size > 0 ) {

        print_size   = ( buffer_size>char_per_line ) ? char_per_line: buffer_size;
        buffer_size -= ( buffer_size>char_per_line ) ? char_per_line: buffer_size;

        printf("\n\r0x%04X: ", line_index );

        for ( i=0; i<print_size; i++ ) {
            if ( extra_space )
                printf("%02X ", buffer[line_index+i]);
            else
                printf("%02X", buffer[line_index+i]);
        }

        if ( char_per_line < size ) {
            for ( i=0; i<(char_per_line-print_size); i++ ) {
                if ( extra_space )
                    printf("   ");
                else
                    printf("  ");
            }
        }

        if ( extra_space )
            printf("| ");
        else
            printf(" | ");

        for ( i=0; i<print_size; i++ ) {
            if (( 0x20 < buffer[line_index+i] ) && ( buffer[line_index+i] < 0x7F ))
                printf("%c", buffer[line_index+i] );
            else
                printf(" ");
        }

        line_index += print_size;
    }

    printf("\n\r\n\r");

    return PASSED;
}


/*-------------------------------------------------
 * $Log: platform_idprom_utils.c,v $
 * Revision 1.2  2021/06/02 08:22:35  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.8.2  2020/08/26 02:37:51  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.6.3  2020/08/04 08:37:06  iachang
 * Update Copyright to 2020
 *
 * Revision 1.1.6.2  2019/03/14 03:48:36  letsai
 * Initial check in.
 *
 *
 *
 *
 * $Endlog$
 */
