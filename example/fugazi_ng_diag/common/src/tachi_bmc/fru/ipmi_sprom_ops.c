/* $Id: ipmi_sprom_ops.c,v 1.2 2016/04/20 08:41:36 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/fru/ipmi_sprom_ops.c,v $
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

#ifndef __USE_ISOC99
#define __USE_ISOC99
#endif
#include <ctype.h>

#include "ipmi_sprom_ops.h"

#define _in_strm stdin
static char   _set_buf[256];
static uint32_t _in_line    = 0;
static char   _in_fname[128];

void sprom_mem_fill(char *myptr, char *mystr, uint32_t sz)
{			
	uint32_t len = strlen((const char*)mystr);
	if (len > sz) len = sz;
	memset(myptr, (int)NULL, sz);
	memcpy(myptr, mystr, len);
}

#define isblank(c)    __isctype((c), _ISblank)

#define PRINT_STR(strname, pstr, len) \
	if (pstr) \
	{ memcpy(_set_buf, pstr, len); \
	  _set_buf[len] = '\0';		 \
	  printf("%-14s : %s\n", strname, _set_buf); \
	}

#define PRINT_STR_TL(strname, pstr, len) \
	if (pstr) \
	{ memcpy(_set_buf, pstr, len); \
	  _set_buf[len] = '\0';		 \
	  printf("%-14s : %s\n", strname, _set_buf); \
	}

#ifndef TRUE
#define TRUE	1
#endif

#ifndef FALSE
#define FALSE	0
#endif

#include "ipmi_sprom.h"

#undef sprom_prinf
#define sprom_printf	printf

#undef sprom_err
#define sprom_err	printf

#define INDENT_PER_LEVEL (2)

#define PINDENT(format , args...)  \
    do {					\
	    _print_indent(INDENT_PER_LEVEL);		  \
	    sprom_printf ( format , ## args );	\
    } while (0)


static inline int
_isxdigit (char c)
{
    return (((c >= '0') && (c <= '9')) ||
	    ((c >= 'a') && (c <= 'f')) ||
	    ((c >= 'A') && (c <= 'F')));
}

static int
_ishexstr (char *s)
{
  char *p;

  if (strncmp(s, "0x", 2) == 0
      || strncmp(s, "0X", 2) == 0)
    s+=2;
  for (p=s ; *p; p++) {
    if (!_isxdigit(*p))
      return FALSE;
  }
  return TRUE;
}

int
_parseDateTime (char *str, uint8_t addr[], int errDisp)
{
  int i, slen;
  uint32_t temp;
  char *ptr;
  uint8_t non_canonical = FALSE, canonical = FALSE;

  slen = strlen(str);
  if (slen< 5 || slen>8)
    goto InvalidDate;

  for (i=0; i<slen; i++) {
    if (str[i]=='/')
       canonical = TRUE;
    else if (str[i]==':')
       non_canonical = TRUE;
    else if (!_isxdigit(str[i]))
       goto InvalidDate;
  }

  if (canonical == TRUE && non_canonical == TRUE)
     goto InvalidDate;

  for (i = 0; i< 3; i++ ) {
    ptr = strtok(str,":/");
    str = 0;
    if (ptr && _ishexstr(ptr)) {
      char *end;

      temp = strtol(ptr, &end, 10);
      if ((temp == 0) && (ptr == end)) {
        goto InvalidDate;
      }
      if (temp > 255)
	goto InvalidDate;
      addr[i] = temp;
    } else
      goto InvalidDate;
  }

  return 0;

InvalidDate:
  if (errDisp)
    sprom_printf ("Invalid Date format.\n");
  return -1;
}

static int _readval( void)
{
    uint32_t  i;
    uint32_t  is_blank;

    do {
	if ( fgets( _set_buf, sizeof(_set_buf), _in_strm)) {
	    _in_line++;
	    // zap trailing newline
	    _set_buf[strlen(_set_buf)-1] = '\0';
	    if ( _in_strm != stdin) {
		// reading from a file - more processing
		// skip lines of all white space
		for ( i = 0, is_blank=1; i < strlen(_set_buf); i++) {
		    if ( _set_buf[i] && !isblank(_set_buf[i])) {
			is_blank = 0;
			break;
		    }
		}
		if ( is_blank) {
		    continue;
		}
		if ( i != 0) {
		    // trim leading white space
		    memmove( _set_buf, &_set_buf[i], strlen(&_set_buf[i])+1);
		}
		// skip lines that begin "#"
		if ( !strncmp( _set_buf, "#", 1)) {
		    continue;
		}
	    }
	    return 1;
	}
	else {
	    // error
	    if ( _in_strm != stdin) {
		sprom_err ( "Problems reading data from '%s'\n", _in_fname);
	    }
	    else {
		sprom_err ( "Problems reading data from stdin.\n");
	    }
	    if ( feof( _in_strm)) {
		sprom_err ( "Reason: End of File reached.\n");
	    }
	    else if ( ferror( _in_strm)) {
		sprom_err ( "Reason: %s.\n", strerror(errno));
	    }
	    exit(-1);
	}
    } while (1);

    return 0;
}

#define INPUT_FAILURE(fld_name, fmt , args... )		     \
    {				     \
	sprom_err ( "\nProblems parsing input file.\n");	 \
	sprom_err ( "File name	    : %s\n", _in_fname);	 \
	sprom_err ( "Line number    : %u\n", _in_line);		 \
	sprom_err ( "Input Field    : %s\n", fld_name);		 \
	sprom_err ( "Offending Input: %s\n", _set_buf);		 \
	sprom_err ( "Reason	    : "fmt".\n" , ## args);	 \
	sprom_err ( "Exiting ...\n\n");				 \
	exit( -EIO );						 \
    }

#define FILL_STR(str,field,len)					\
    do {							\
	int _mylen = (int)strlen((const char*)field);		\
	if(_mylen > len) {_mylen = len;}			\
	memset(_set_buf, '\0', sizeof(_set_buf));		\
	sprom_mem_fill((char*)_set_buf, (char*)field, _mylen);	\
	_set_buf[_mylen] = '\0';				\
	PINDENT("  Enter %s [%s]:  ", str, _set_buf);		\
	if ( _readval()) {					\
	    if ((_mylen=strlen(_set_buf))) {			\
		sprom_mem_fill( (char*)field, (char*)_set_buf,  \
			(len > _mylen) ? _mylen+1 : len);	\
	    }							\
	    break;						\
	}							\
    } while (1)

#define FILL_INT(str,pfmt,sfmt,field)					\
    do {								\
	PINDENT("  Enter %s ["#pfmt"]:	", str, field);			\
	if ( _readval()) {						\
	    if ( !strlen(_set_buf)) break;				\
	    if ( _isintstr(_set_buf) ) {				\
		sscanf( _set_buf, #sfmt, &(field));			\
		break;							\
	    }								\
	    else {							\
		INPUT_FAILURE(str, "Not an integer string");		\
	    }								\
	}								\
    } while (1)

#define FILL_SDR(field, unit)			\
{						\
	int _mdata_ = field.value;		\
	FILL_INT(#unit, %d, %d, _mdata_);	\
	field.type  = unit;			\
	field.len   = 2;			\
	field.value = htons((uint16_t)_mdata_); \
}

#if 0
#define FILL_HEX(str,pfmt,sfmt,field)					\
    do {								\
	PINDENT("  Enter %s ["#pfmt"]:	", str, field);		   \
	if ( _readval()) {						\
	    if ( !strlen(_set_buf)) break;				\
	    if ( _ishexstr(_set_buf) ) {				\
		sscanf( _set_buf, #sfmt, &(field));		   \
		break;							\
	    }								\
	    else {							\
		INPUT_FAILURE(str, "Not a hex string");			\
	    }								\
	}								\
    } while (1)
#else
#define FILL_HEX(str,pfmt,sfmt,field)					\
	PINDENT("  Enter %s ["#pfmt"]:	", str, field);		   	\
	if ( _readval()) {						\
	    if (strlen(_set_buf)) 					\
 	    	sscanf( _set_buf, #sfmt, &(field));		   	\
	}								\

#endif
#define FILL_TL_STR(str, field, len)				\
	do {								\
		field##_tl = IPMI_SPROM_TYPE_CODE_LC_MSK | len;		\
		FILL_STR(str, field, len);				\
	} while(0)


// For gathering MAC address
#define FILL_MAC(str,mac)					      \
    do {								\
	PINDENT("  Enter %s [%02X-%02X-%02X-%02X-%02X-%02X]:  ",	\
		 str, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]); \
	if ( _readval()) {						\
	    if ( !strlen(_set_buf)) break;				\
	    if ( _parseMacAddrNoSwap( _set_buf, mac, 1) == 0) {  \
		break;							\
	    }								\
	    else {							\
		INPUT_FAILURE(str, "Unable to parse MAC Address");	\
	    }								\
	}								\
    } while (1)

// For gathering WWN address
#define FILL_WWN(str,wwn)					      \
    do {								\
	PINDENT("  Enter %s [%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X]:	",\
	 str, wwn[0], wwn[1], wwn[2], wwn[3], wwn[4], wwn[5], wwn[6], wwn[7]);\
	if ( _readval()) {						\
	    if ( !strlen(_set_buf)) break;				\
	    if ( _parseWWNAddrNoSwap( _set_buf, wwn, 1) == 0) {  \
		break;							\
	    }								\
	    else {							\
		INPUT_FAILURE(str, "Unable to parse WWN Address");	\
	    }								\
	}								\
    } while (1)

// For gathering DATE 
#define FILL_DATE(str,field)							\
    do {									\
	PINDENT("  Enter %s [%d/%d/%d]:  ", str, field[0], field[1], field[2]);\
	if ( _readval()) {				      \
	    if ( !strlen(_set_buf)) break;		      \
	    if ( _parseDateTime( _set_buf, field, 1) == 0) {  \
		break;					      \
	    }						      \
	    else {					      \
		INPUT_FAILURE(str, "Unable to parse Date");   \
	    }						      \
	}						      \
    } while (1)


static void _print_indent( uint32_t indent)
{
    uint32_t i;
    char   buf[64];

    buf[0] = '\0';
    for ( i = 0; i < indent; i++)
	strcat( buf, " ");

    sprom_printf ( "%s", buf );
}

static inline int
_isintdigit (char c)
{
    return (c >= '0') && (c <= '9');
}

int
_isintstr (char *s)
{
  char *p;

  if (strncmp(s, "+", 1) == 0
      || strncmp(s, "-", 1) == 0)
    s+=1;
  for (p=s ; *p; p++) {
    if (!_isintdigit(*p))
      return FALSE;
  }
  return TRUE;
}

int
_parsePortAddrNoSwap (char *str, uint8_t *addr, int errDisp, int addr_size)
{
  int i, slen;
  uint32_t temp;
  uint64_t temp64;
  char *ptr, *ptemp;
  uint8_t non_canonical = FALSE, canonical = FALSE;

  slen = strlen(str);
  if (slen< ((addr_size*2)-1) || slen>((addr_size*3)-1))
    goto InvalidMac;

  for (i=0; i<slen; i++) {
    if (str[i]=='-')
       canonical = TRUE;
    else if (str[i]==':')
       non_canonical = TRUE;
    else if (!_isxdigit(str[i]))
       goto InvalidMac;
  }

  if (canonical == TRUE && non_canonical == TRUE)
     goto InvalidMac;

  if (canonical == FALSE && non_canonical == FALSE) {
	temp64 = strtoull(str, (char**)NULL, 16);
	ptemp = (char*)&temp64;
	ptemp+=2;
	memcpy(addr, ptemp, 6);
  } else {
    for (i = 0; i< addr_size; i++ ) {
      ptr = strtok(str,":-");
      str = 0;
      if (ptr && _ishexstr(ptr)) {
        char *end;

	temp = strtol(ptr, &end, 16);
	if ((temp == 0) && (ptr == end)) {
	    goto InvalidMac;
	}
	if (temp > 255)
	  goto InvalidMac;
	addr[i] = temp;
      } else
	goto InvalidMac;
    }
  }

  return 0;

InvalidMac:
  if (errDisp)
    sprom_printf ("Invalid MAC/WWN address format.\n");
  return -1;
}

int
_parseMacAddrNoSwap (char *str, uint8_t *addr, int errDisp)
{
	return (_parsePortAddrNoSwap (str, addr, errDisp, 6));
}

int
_parseWWNAddrNoSwap (char *str, uint8_t *addr, int errDisp)
{
	return (_parsePortAddrNoSwap (str, addr, errDisp, 8));
}

// dump raw bytes
void ipmi_raw_dump (uint8_t *buf, int buflen)
{
	int cnt;

	for (cnt = 0; cnt < buflen; cnt++) {
		if (!(cnt & 0x0F))
			printf("\n 0x%04X : ", cnt);
		printf("%02X ", buf[cnt]);	
	}
	printf("\n");
}

// return 0 if check sum is good, otherwise return 
// the xor of all the bytes
int ipmi_zero_checksum_verify (uint8_t *buf, int buflen)
{
	int cnt;
	uint8_t cs = 0;
	for(cnt = 0; cnt < buflen; cnt++) {
		cs += buf[cnt];
	}
	return (cs);
}

int ipmi_header_checksum_verify (uint8_t *buf)
{
	ipmi_sprom_common_header_t *phdr = 
		(ipmi_sprom_common_header_t*)buf;

	if ((phdr->version == 0) ||
	    (phdr->header_checksum == 0))
		return (-1);
	return (ipmi_zero_checksum_verify(buf, 
			sizeof(ipmi_sprom_common_header_t)));
}


int ipmi_fru_verify (uint8_t *buf, uint32_t size)
{
	ipmi_sprom_common_header_t *phdr = (ipmi_sprom_common_header_t*)buf;	
	int rc = 0, len, err = 0;
	uint8_t *pdata;
	// Verify the header check sum.
	rc = ipmi_header_checksum_verify(buf);
	if (rc) {
		printf("  Header check sum failed\n");
		ipmi_raw_dump ((uint8_t*)phdr, (int)sizeof(ipmi_sprom_common_header_t));
		return (rc);
	}

	if (phdr->board_info) {
		printf("  Verify board info\n");
		pdata = buf + (phdr->board_info << 3);
		len = pdata[1] << 3;
		rc = ipmi_zero_checksum_verify(pdata, len);
		if (rc) {
			printf("  board_info info failed\n");
			ipmi_raw_dump (pdata, len);
			err = rc;
		}
		
	}

	if (phdr->internal_use) {
		printf("  Verify internal use\n");
		pdata = buf + (phdr->internal_use << 3);
		len = pdata[1] << 3;
		rc = ipmi_zero_checksum_verify(pdata, len);
		if (rc) {
			printf("  internal_use info failed\n");
			ipmi_raw_dump (pdata, len);
			err = rc;
		}
	}
	if (phdr->chassis_info) {
		printf("  Verify Chassis Info\n");
		pdata = buf + (phdr->chassis_info << 3);
		len = pdata[1] << 3;
		rc = ipmi_zero_checksum_verify(pdata, len);
		if (rc) {
			printf("  Chassis info failed\n");
			ipmi_raw_dump (pdata, len);
			err = rc;
		}
	}
	if (phdr->product_info) {
		printf("  Verify Product Info\n");
		pdata = buf + (phdr->product_info << 3);
		len = pdata[1] << 3;
		rc = ipmi_zero_checksum_verify(pdata, len);
		if (rc) {
			printf("  product_info failed\n");
			ipmi_raw_dump (pdata, len);
			err = rc;
		}
	}

	if (phdr->multi_record) {
		ipmi_sprom_multi_record_header_t *pMrHdr;
		int mr_cs, mr_cnt=4;	// Currently supports max 4 records

		printf("  Verify Multi Record\n");
		pdata = buf + (phdr->multi_record << 3);
		do {
			pMrHdr = (ipmi_sprom_multi_record_header_t*)pdata;
			rc = ipmi_zero_checksum_verify(pdata, sizeof(*pMrHdr));
			if (rc) {
				printf("  MultiRecord Header failed\n");
				ipmi_raw_dump (pdata, sizeof(*pMrHdr));
				err = rc;
			}

			pdata += sizeof(*pMrHdr);
			mr_cs = ipmi_zero_checksum_get(pdata, pMrHdr->length);

			if (mr_cs != pMrHdr->record_checksum) {
				printf("  MultiRecord CheckSum Err [Calc:0x%X Prog:0x%X]\n",
					mr_cs, pMrHdr->record_checksum);
				ipmi_raw_dump ((uint8_t*)pMrHdr, sizeof(*pMrHdr)+pMrHdr->length);
				err = -1;
			}
			if (pMrHdr->version & IPMI_SPROM_MULTI_RECORD_EOL) break;

			pdata += pMrHdr->length;
		} while (--mr_cnt);
	}
	return (err? err : rc);
}
int ipmi_sprom_product_info_checksum_verify (uint8_t *buf, size_t len)
{
	ipmi_sprom_product_info_t *pi = 
		(ipmi_sprom_product_info_t *)buf;

	if (pi->version == 0)
		return (-1);

	return (ipmi_zero_checksum_verify(buf, len));
}

int ipmi_sprom_board_info_checksum_verify (uint8_t *buf, size_t len)
{
	ipmi_sprom_board_info_t *bi = (ipmi_sprom_board_info_t *)buf;

	if (bi->version == 0)
		return (-1);

	return (ipmi_zero_checksum_verify(buf, len));
}

int ipmi_sprom_chassis_info_checksum_verify (uint8_t *buf, size_t len)
{
	ipmi_sprom_chassis_info_t *ci = 
		(ipmi_sprom_chassis_info_t *)buf;

	if (ci->version == 0)
		return (-1);

	return (ipmi_zero_checksum_verify(buf, len));
}

// Compute check sum 
uint8_t ipmi_zero_checksum_get (uint8_t *buf, int buflen)
{
	int cnt;
	uint8_t cs = 0;

	for(cnt = 0; cnt < buflen; cnt++) {
		cs += buf[cnt];
	}

	return (0x100 - cs);
}

void ipmi_zero_checksum_create (uint8_t *buf, int buflen)
{
	int cnt;
	uint8_t cs = 0;

	for(cnt = 0; cnt < buflen-1; cnt++) {
		cs += buf[cnt];
	}

	buf[buflen-1] = 0x100 - cs;
}

void ipmi_multi_record_checksum_create(ipmi_sprom_multi_record_header_t *pmr)
{
	pmr->record_checksum =
	    ipmi_zero_checksum_get((uint8_t *)(pmr + 1), pmr->length);
	ipmi_zero_checksum_create((uint8_t *)pmr, sizeof (*pmr));
}

int ipmi_multi_record_checksum_verify(ipmi_sprom_multi_record_header_t *pmr)
{
	uint8_t cs;

	cs = ipmi_zero_checksum_get((uint8_t *)(pmr + 1), pmr->length);
	if (cs != pmr->record_checksum) {
		return (1);
	}
	return ipmi_zero_checksum_verify((uint8_t *)pmr, sizeof (*pmr));
}

int ipmi_sprom_common_header_create(
		ipmi_sprom_common_header_t *phdr,
		uint8_t internal_use_sz,
		uint8_t chassis_info_sz,
		uint8_t  board_info_sz,
		uint8_t product_info_sz,
		uint8_t multi_record_sz)
{
	uint32_t	offset = M8_SIZE(sizeof(ipmi_sprom_common_header_t));

	phdr->version	   = IPMI_SPROM_COMMON_HEADER_VERSION;

	if (chassis_info_sz) {
		phdr->chassis_info = offset;
		offset += chassis_info_sz; 
	} else {
		phdr->chassis_info = 0x00; 
	}

	if (board_info_sz) {
		phdr->board_info   = offset;
		offset += board_info_sz; 
	} else {
		phdr->board_info = 0x00;
	}

	if (product_info_sz) {
		phdr->product_info = offset; 
		offset += product_info_sz; 
	} else {
		phdr->product_info = 0x00; 
	}

	if (internal_use_sz) {
		phdr->internal_use = offset; 
		offset += internal_use_sz;
	} else {
		phdr->internal_use = 0x00; 
	}

	if (multi_record_sz) {
		phdr->multi_record = offset; 
	} else {
		phdr->multi_record = 0x00; 
	}
	phdr->pad	   = 0x00;
	ipmi_zero_checksum_create((uint8_t*)phdr, sizeof(*phdr));
	return (0);
}


void ipmi_sprom_common_header_dump(ipmi_sprom_common_header_t *phdr)
{
	if (!phdr) return;

	printf(" Common Header Area\n");
	printf("\tVERSION	: 0x%X\n", phdr->version);
	printf("\tINTERNAL USE	: 0x%X\n", phdr->internal_use);
	printf("\tCHASSIS INFO	: 0x%X\n", phdr->chassis_info);
	printf("\tBOARD INFO	: 0x%X\n", phdr->board_info);
	printf("\tPRODUCT INFO	: 0x%X\n", phdr->product_info);
	printf("\tMULTI RECORD	: 0x%X\n", phdr->multi_record);
	printf("\tPADDING	: 0x%X\n", phdr->pad);
	printf("\tCHECKSUM	: 0x%X\n", phdr->header_checksum);
}

// Internal Use Area
int ipmi_sprom_iu_mac_create (ipmi_iu_rec_mac_t *pmac, uint32_t mac_cnt)
{
	uint32_t data;
	char	 buf[16];

	pmac->srec.sub_type = htons(IPMI_IU_SUB_TYPE_MAC_ADDR);
	pmac->srec.sub_type_len = htons(sizeof(ipmi_iu_rec_mac_t) - 
				  sizeof(ipmi_iu_rec_t));

	pmac->cnt.type = IPMI_IU_ENTRY_TYPE_CNT;
	pmac->cnt.len  = 2;
//	pmac->cnt.value = htons(mac_cnt);

	data = (uint32_t)htons(pmac->cnt.value);
	FILL_INT("NUMBER OF MAC",  %02d, %d, data);
	pmac->cnt.value = (uint16_t) htons(data);

	pmac->type = IPMI_IU_ENTRY_TYPE_MAC;
	pmac->len  = 6;
	sprintf(buf, "MAC ADDRESS");
	FILL_MAC(buf, pmac->mac);
	return (0);
}

int ipmi_sprom_iu_tpm_create (uint8_t *tpm_enable)
{
    uint32_t data;

    data = (uint32_t)*tpm_enable;
    do {
        FILL_INT("TPM ENABLE", %02d, %d, data);
        if (data > 1) {
            printf("\nInvalid data (0 for Disable, 1 for Enable\n");
        } else {
            break;
        }
    } while (1);

    *tpm_enable = (uint8_t)data;
    return (0);
}

int ipmi_sprom_iu_window_active_create (uint8_t *window_active)
{
    uint32_t data;

    data = (uint32_t)*window_active;
    FILL_INT("WINDOW ACTIVE", %02d, %d, data);

    *window_active = (uint8_t)data;
    return (0);
}

int ipmi_sprom_iu_tpm_dump (uint8_t tpm_enable)
{
    printf("\tTPM             : %s\n", tpm_enable == 1?"ENABLE":"DISABLE");
    return (0);
}

int ipmi_sprom_iu_window_active_dump (uint8_t wind_act)
{
    printf("\tWINDOW ACTIVE   : %02X\n", wind_act);
    return (0);
}

void uuid_get_random_bytes (uint8_t *rb, int count)
{
    int rfd;
    int rcnt;

    rfd = open("/dev/urandom", O_RDONLY);
    if (rfd < 0) {
        printf("Open device failed (%s)\n", strerror(errno));
        return;
    }

    rcnt = read(rfd, rb, count);
    if (rcnt != count) {
        printf("read device failed with rcnt = %d\n", rcnt);
    }

    close(rfd);
}


int ipmi_sprom_iu_uuid_create (ipmi_ibmc_internal_use_t *iu)
{
    /* FILL_STR("UUID", iu->uuid, IU_UUID_SIZE); */

    memset(iu->uuid, 0, IU_UUID_SIZE);

    /* Copy MAC Address to lower 8 bytes */
    memcpy(iu->uuid, iu->mac.mac, 6);

    /* Fill random bytes at upper 8 bytes */
    uuid_get_random_bytes(&(iu->uuid[8]), 8);

    return (0);
}


int ipmi_sprom_iu_uuid_dump (ipmi_ibmc_internal_use_t *iu)
{
    int ix;

    printf("\tUUID            : ");

    for (ix = 0; ix < IU_UUID_SIZE; ix++) {
        printf("%02x", iu->uuid[ix]);
    }
    printf("\n");
    return (0);
}


int ipmi_sprom_iu_mac_dump (ipmi_iu_rec_mac_t *pmac)
{
//	printf("\tSUB TYPE	: %d\n", ntohs(pmac->srec.sub_type));
//	printf("\tSUB TYPE LEN	: %d\n", ntohs(pmac->srec.sub_type_len));

//	printf("\tCNT TYPE	: %d\n", pmac->cnt.type);
//	printf("\tCNT LEN	: %d\n", pmac->cnt.len);
	printf("\tNUMBER OF MACS  : %d\n", ntohs(pmac->cnt.value));

//	printf("\tMAC TYPE	: %d\n", pmac->type);
//	printf("\tMAC LEN	: %d\n", pmac->len);
	printf("\tMAC             : %02X:%02X:%02X:%02X:%02X:%02X\n", 
				pmac->mac[0], pmac->mac[1], 
				pmac->mac[2], pmac->mac[3], 
				pmac->mac[4], pmac->mac[5]);
	return (0);
}


int iu_sdr_create (ipmi_iu_sdr_t *psdr, uint8_t srec_type, 
				char* name)
{
	psdr->srec.sub_type = htons(srec_type);
	psdr->srec.sub_type_len = htons(sizeof(ipmi_iu_sdr_t) -  
				  sizeof(ipmi_iu_rec_t));

	printf("\tCreate SDR for %s\n", name);
	FILL_SDR(psdr->multi_factor, SDR_MULTI_FACTOR);
	FILL_SDR(psdr->base_offset, SDR_BASE_OFFSET);
	FILL_SDR(psdr->k1, SDR_K1);
	FILL_SDR(psdr->k2, SDR_K2);
	FILL_SDR(psdr->nominal_reading, SDR_NOMINAL_READ);
	FILL_SDR(psdr->normal_max, SDR_NORMAL_MAX);
	FILL_SDR(psdr->normal_min, SDR_NORMAL_MIN);
	FILL_SDR(psdr->sensor_max, SDR_SENSOR_MAX);
	FILL_SDR(psdr->sensor_min, SDR_SENSOR_MIN);
	FILL_SDR(psdr->upper_nr_thres, SDR_UP_NR_THRES);
	FILL_SDR(psdr->upper_crit_thres, SDR_UP_CRIT);
	FILL_SDR(psdr->upper_non_crit_thres, SDR_UP_NON_CRIT);
	FILL_SDR(psdr->lower_nr_thres, SDR_LO_NR_THRES);
	FILL_SDR(psdr->lower_crit_thres, SDR_LO_CRIT);
	FILL_SDR(psdr->lower_non_crit_thres, SDR_LO_NON_CRIT);
	FILL_SDR(psdr->pos_hyst, SDR_POS_HYST);
	FILL_SDR(psdr->neg_hyst, SDR_NEG_HYST);

	return (0);
}

#define PRINT_SDR_V(psdr, name, sdr_name)	\
{				\
	int16_t value = (psdr->name.value * psdr->multi_factor.value);\
	double  myval;\
	int	k2val = (16 - (psdr->k2.value))&0x0F;\
	int     mypow[] = {1, 10, 100, 1000, 10000, 100000, 1000000, 10000000,\
                           100000000};\
	if (psdr->k2.value && (k2val < (int)(sizeof(mypow)/sizeof(int)))) {\
		myval = value/(1.0 *mypow[k2val]);\
		myval += psdr->base_offset.value;\
		printf("\t%s %-20s : %6.3f\n", \
			sdr_name, #name, myval); \
	} else {	\
		value += psdr->base_offset.value;\
		printf("\t%s %-20s : %d\n", \
		sdr_name, #name, (int16_t)ntohs(value)); \
	}\
}

#define PRINT_SDR(psdr, name, sdr_name)	\
{				\
	printf("\t%s %-20s : %d\n", \
		sdr_name, #name, (int16_t)ntohs(psdr->name.value)); \
}

int ipmi_sprom_iu_sdr_dump (ipmi_iu_sdr_t *psdr, char *sdr_name)
{
//	printf("\tSUB TYPE	: %d\n", ntohs(psdr->srec.sub_type));
//	printf("\tSUB TYPE LEN	: %d\n", ntohs(psdr->srec.sub_type_len));
	PRINT_SDR(psdr, multi_factor, sdr_name);
	PRINT_SDR(psdr, base_offset, sdr_name);
	PRINT_SDR(psdr, k1, sdr_name);
	PRINT_SDR(psdr, k2, sdr_name);
	PRINT_SDR_V(psdr, nominal_reading, sdr_name);
	PRINT_SDR_V(psdr, normal_max, sdr_name);
	PRINT_SDR_V(psdr, normal_min, sdr_name);
	PRINT_SDR_V(psdr, sensor_max, sdr_name);
	PRINT_SDR_V(psdr, sensor_min, sdr_name);
	PRINT_SDR_V(psdr, upper_nr_thres, sdr_name);
	PRINT_SDR_V(psdr, upper_crit_thres, sdr_name);
	PRINT_SDR_V(psdr, upper_non_crit_thres, sdr_name);
	PRINT_SDR_V(psdr, lower_nr_thres, sdr_name);
	PRINT_SDR_V(psdr, lower_crit_thres, sdr_name);
	PRINT_SDR_V(psdr, lower_non_crit_thres, sdr_name);
	PRINT_SDR_V(psdr, pos_hyst, sdr_name);
	PRINT_SDR_V(psdr, neg_hyst, sdr_name);
	return (0);
}

int ipmi_sprom_iu_sdr_print (ipmi_iu_sdr_t *psdr, char *sdr_name)
{
	PRINT_SDR(psdr, multi_factor, sdr_name);
	PRINT_SDR(psdr, base_offset, sdr_name);
	PRINT_SDR(psdr, k1, sdr_name);
	PRINT_SDR(psdr, k2, sdr_name);
	PRINT_SDR(psdr, nominal_reading, sdr_name);
	PRINT_SDR(psdr, normal_max, sdr_name);
	PRINT_SDR(psdr, normal_min, sdr_name);
	PRINT_SDR(psdr, sensor_max, sdr_name);
	PRINT_SDR(psdr, sensor_min, sdr_name);
	PRINT_SDR(psdr, upper_nr_thres, sdr_name);
	PRINT_SDR(psdr, upper_crit_thres, sdr_name);
	PRINT_SDR(psdr, upper_non_crit_thres, sdr_name);
	PRINT_SDR(psdr, lower_nr_thres, sdr_name);
	PRINT_SDR(psdr, lower_crit_thres, sdr_name);
	PRINT_SDR(psdr, lower_non_crit_thres, sdr_name);
	PRINT_SDR(psdr, pos_hyst, sdr_name);
	PRINT_SDR(psdr, neg_hyst, sdr_name);
	return (0);
}

int ipmi_iom_iu_create (ipmi_iom_internal_use_t* piu, uint16_t card_type)
{
	uint32_t data;
	piu->version	= IPMI_SPROM_CHASSIS_VERSION;
	piu->length	= M8_SIZE(sizeof(*piu));
	piu->card_type	= htons(card_type);

	data = (uint32_t)piu->card_type;
	FILL_INT("CARD TYPE",  %02d, %d, data);
	piu->card_type = (uint8_t) htons(data);

	ipmi_sprom_iu_mac_create (&piu->mac, 10);
	return (0);
}

int ipmi_iu_sdr_create(ipmi_iu_sdr_t *psdr, char *name)
{
	uint32_t data;
	char	 sname[32];

	sprintf(sname, "%s SENSOR MAX", name);
	data = (uint32_t)psdr->sensor_max.value;
	FILL_INT(sname,  %02d, %d, data);
	psdr->sensor_max.value = (uint16_t) htons(data);

	sprintf(sname, "%s SENSOR MIN", name);
	data = (uint32_t)psdr->sensor_min.value;
	FILL_INT(sname,  %02d, %d, data);
	psdr->sensor_min.value = (uint16_t) htons(data);

	return (0);
}

int ipmi_mezz_iu_create (ipmi_mezz_internal_use_t* piu, uint16_t card_type)
{
	// mac_cnt is not really used in the function. 
	// Value programmed in the eeprom is used.
	int	mac_cnt = 6;

	piu->version	= IPMI_SPROM_CHASSIS_VERSION;
	piu->length	= M8_SIZE(sizeof(*piu));
	if (card_type != IPMI_IU_CARD_TYPE_UNKNOWN) {
		piu->card_type = htons(card_type);
	}

#if 0 
	switch(card_type) {
		case	IPMI_IU_CARD_TYPE_OPLIN:
		case	IPMI_IU_CARD_TYPE_NETEFFECT:
			mac_cnt = 2;
			break;

		case	IPMI_IU_CARD_TYPE_EVEREST:
		case	IPMI_IU_CARD_TYPE_NIANTIC:
		case	IPMI_IU_CARD_TYPE_SCHULTZ:
		case	IPMI_IU_CARD_TYPE_TIGERSHARK:
		case	IPMI_IU_CARD_TYPE_EVEREST:
			mac_cnt = 4;
			break;

		case	IPMI_IU_CARD_TYPE_MONTEREYPARK:
			mac_cnt = 21;
			break;

		case	IPMI_IU_CARD_TYPE_VASONA:
			mac_cnt = 12;
			break;

		case	IPMI_IU_CARD_TYPE_UNKNOWN:
			{
			uint32_t data = (uint32_t)htons(piu->card_type);
			FILL_INT("CARD TYPE",  %02d, %d, data);
			piu->card_type = ntohs(data);
			mac_cnt = htons(piu->mac.cnt.value);
			}
			break;

		default:
			break;
	}
#endif
	ipmi_sprom_iu_mac_create (&piu->mac, mac_cnt);
	return (0);
}

int ipmi_bmc_iu_create (ipmi_ibmc_internal_use_t* piu, uint16_t card_type)
{
        uint16_t mac_cnt = htons(piu->mac.cnt.value);

        piu->version    = IPMI_SPROM_CHASSIS_VERSION;
        piu->length     = M8_SIZE(sizeof(*piu));
        piu->card_type  = htons(card_type);

        if ((mac_cnt == 0xFFFF) || (mac_cnt == 0x0000))
                mac_cnt = 2;
        switch(card_type) {
                case    IPMI_IU_CARD_TYPE_IBMC:
                        break;

                default:
                        break;
        }
        ipmi_sprom_iu_mac_create (&piu->mac, (uint32_t)mac_cnt);

        ipmi_sprom_iu_tpm_create(&piu->tpm);
        ipmi_sprom_iu_window_active_create(&piu->window_active);
        ipmi_sprom_iu_uuid_create(piu);
	return (0);
}

int ipmi_fan_iu_create (ipmi_fan_internal_use_t* piu, uint16_t card_type)
{
	piu->version	= IPMI_SPROM_CHASSIS_VERSION;
	piu->length	= M8_SIZE(sizeof(*piu));
	piu->card_type	= htons(card_type);

	iu_sdr_dflt_iom(&piu->fan_rpm, IPMI_IU_SUB_TYPE_FAN_RPM); 
	iu_sdr_dflt_iom(&piu->fan_temp, IPMI_IU_SUB_TYPE_FAN_TEMP);
	iu_sdr_dflt_iom(&piu->fan_volt, IPMI_IU_SUB_TYPE_FAN_VOLTAGE);
	ipmi_zero_checksum_create((uint8_t*)piu, sizeof(*piu));
	return (0);
}

int ipmi_psu_iu_create (ipmi_psu_internal_use_t* piu, uint16_t card_type)
{
	piu->version	= IPMI_SPROM_CHASSIS_VERSION;
	piu->length	= M8_SIZE(sizeof(*piu));
	piu->card_type	= htons(card_type);
	return (0);
}

int ipmi_mem_iu_create (ipmi_mem_internal_use_t* piu, uint16_t card_type)
{
        piu->version    = IPMI_SPROM_BOARD_VERSION;
        piu->length     = M8_SIZE(sizeof(*piu));
        piu->card_type  = htons(card_type);
	return (0);
}

int ipmi_hddbp_iu_create (ipmi_hddbp_internal_use_t* piu, uint16_t card_type)
{
        piu->version    = IPMI_SPROM_BOARD_VERSION;
        piu->length     = M8_SIZE(sizeof(*piu));
        piu->card_type  = htons(card_type);
	return (0);
}

int ipmi_sprom_internal_use_create (void *pinternal, uint16_t card_type)
{
	int iu_size = 0;

	switch (card_type) {
		case	IPMI_IU_CARD_TYPE_IOM:
		case	IPMI_IU_CARD_TYPE_IOM2:
			{
			ipmi_iom_internal_use_t *piu = 
				(ipmi_iom_internal_use_t*)pinternal;
			iu_size = sizeof(*piu);
			ipmi_iom_iu_create (piu, card_type);

			ipmi_iu_sdr_create(&piu->brd_temp1, "BRD TEMP1");
			ipmi_iu_sdr_create(&piu->brd_temp2, "BRD TEMP2");
			ipmi_iu_sdr_create(&piu->rw_temp1,  "RW TEMP1");
			ipmi_iu_sdr_create(&piu->rw_temp2,  "RW TEMP2");
			}
			break;

		case	IPMI_IU_CARD_TYPE_IBMC:
			{
			ipmi_ibmc_internal_use_t *piu = 
				(ipmi_ibmc_internal_use_t*)pinternal;
			iu_size = sizeof(*piu);
			ipmi_bmc_iu_create (piu, card_type);
			}
			break;

		case	IPMI_IU_CARD_TYPE_OPLIN:
		case	IPMI_IU_CARD_TYPE_PALO:
		case	IPMI_IU_CARD_TYPE_MENLO:
		case	IPMI_IU_CARD_TYPE_MENLO_E:
		case	IPMI_IU_CARD_TYPE_NIANTIC:
		case	IPMI_IU_CARD_TYPE_NETEFFECT:
		case	IPMI_IU_CARD_TYPE_SCHULTZ:
		case	IPMI_IU_CARD_TYPE_TIGERSHARK:
		case	IPMI_IU_CARD_TYPE_EVEREST:
		case	IPMI_IU_CARD_TYPE_MONTEREYPARK:
		case	IPMI_IU_CARD_TYPE_VASONA:
		case	IPMI_IU_CARD_TYPE_DUBLIN:
		case	IPMI_IU_CARD_TYPE_FREMONT:
		case	IPMI_IU_CARD_TYPE_LIVERMORE:
		case	IPMI_IU_CARD_TYPE_UNKNOWN:
			{
			ipmi_mezz_internal_use_t *piu = 
				(ipmi_mezz_internal_use_t*)pinternal;
			iu_size = sizeof(*piu);
			ipmi_mezz_iu_create (piu, card_type);
			}
			break;

		case	IPMI_IU_CARD_TYPE_SC_FAN:
			{
			ipmi_fan_internal_use_t *piu = 
				(ipmi_fan_internal_use_t*)pinternal;
			iu_size = sizeof(*piu);
			ipmi_fan_iu_create (piu, card_type);
			ipmi_iu_sdr_create(&piu->fan_rpm, "FAN RPM");
			ipmi_iu_sdr_create(&piu->fan_temp, "FAN TEMP");
			ipmi_iu_sdr_create(&piu->fan_volt,  "FAN VOLT");
			}
			break;

		case	IPMI_IU_CARD_TYPE_SC_PSU:
			{
			ipmi_psu_internal_use_t *piu = 
				(ipmi_psu_internal_use_t*)pinternal;
			iu_size = sizeof(*piu);
			ipmi_psu_iu_create (piu, card_type);
			ipmi_iu_sdr_create(&piu->psu_rpm, "PSU RPM");
			ipmi_iu_sdr_create(&piu->psu_temp, "PSU TEMP");
			ipmi_iu_sdr_create(&piu->psu_in_volt,  "PSU IN VOLT");
			ipmi_iu_sdr_create(&piu->psu_out_volt1,  "PSU OUT VOLT1");
			ipmi_iu_sdr_create(&piu->psu_out_volt2,  "PSU OUT VOLT2");
			ipmi_iu_sdr_create(&piu->psu_in_current,  "PSU AMP");
			ipmi_iu_sdr_create(&piu->psu_out_current,  "PSU OUT AMP");
			}
			break;
		case	IPMI_IU_CARD_TYPE_MEMFRONT:
		case	IPMI_IU_CARD_TYPE_MEMBACK:
		{
			ipmi_mem_internal_use_t *piu = 
				(ipmi_mem_internal_use_t*)pinternal;
			iu_size = sizeof(*piu);
			ipmi_mem_iu_create (piu, card_type);
			}
			break;

		case	IPMI_IU_CARD_TYPE_TURLOCK:
			{
				ipmi_hddbp_internal_use_t *piu = 
					(ipmi_hddbp_internal_use_t*)pinternal;
				iu_size = sizeof(*piu);
				ipmi_hddbp_iu_create (piu, card_type);
			}
			break;

		default:
			return (-1);

	}

	ipmi_zero_checksum_create((uint8_t*)pinternal, iu_size); 
	return (0);
}



int ipmi_iom_iu_dump (ipmi_iom_internal_use_t* piu)
{
	printf(" IOM Internal Use Area\n");
	printf("\tCARD_TYPE	: %d\n", ntohs(piu->card_type));

	ipmi_sprom_iu_mac_dump (&piu->mac);
	ipmi_sprom_iu_sdr_dump (&piu->brd_temp1, "BRD TEMP1");
	ipmi_sprom_iu_sdr_dump (&piu->brd_temp2, "BRD TEMP2");
	ipmi_sprom_iu_sdr_dump (&piu->rw_temp1, "RW TEMP1");
	ipmi_sprom_iu_sdr_dump (&piu->rw_temp2, "RW TEMP2");
	return (0);
}

int ipmi_mezz_iu_dump (ipmi_mezz_internal_use_t* piu)
{
	printf(" Mezz Internal Use Area\n");
	printf("\tCARD_TYPE	: %d\n", ntohs(piu->card_type));

	ipmi_sprom_iu_mac_dump (&piu->mac);
	return (0);
}

int ipmi_sprom_ncsi_multi_record_dump(ipmi_ncsi_multi_record_t *pmr)
{
	printf("\n NC-SI Multi Record\n");
	printf("\tPOWER SOURCE       : %u\n", pmr->power_source);
	printf("\tPEAK STANDBY POWER : %u.%uW\n",
	       ntohs(pmr->peak_standby_power) / 10,
	       ntohs(pmr->peak_standby_power) % 10);
	printf("\tNUMBER OF MACS     : %u\n", pmr->mac_count);
	printf("\tMAC                : %02X:%02X:%02X:%02X:%02X:%02X\n",
	       pmr->mac[0], pmr->mac[1], pmr->mac[2],
	       pmr->mac[3], pmr->mac[4], pmr->mac[5]);

	return (0);
}

static char* bmc_blade_class[] = { "Unknown", "Gooding", "Ventura", "Rack_Server"};
int ipmi_bmc_iu_dump (ipmi_ibmc_internal_use_t* piu)
{
	printf(" BMC Internal Use Area (blade_class=%d : %s)\n",
        	piu->blade_class, bmc_blade_class[piu->blade_class & 0x03]);
	printf("\tCARD_TYPE	: %d\n", ntohs(piu->card_type));
	ipmi_sprom_iu_mac_dump (&piu->mac);
	ipmi_sprom_iu_tpm_dump (piu->tpm);
	ipmi_sprom_iu_window_active_dump (piu->window_active);
	ipmi_sprom_iu_uuid_dump (piu);

	return (0);
}

int ipmi_hddbp_iu_dump (ipmi_hddbp_internal_use_t* piu)
{
	printf(" HDDBP Internal Use Area \n");
	printf("\tCARD_TYPE	: %d\n", ntohs(piu->card_type));

	return (0);
}

int ipmi_fan_iu_dump (ipmi_fan_internal_use_t* piu)
{
	printf(" FAN Internal Use Area\n");
	printf("\tCARD_TYPE	: %d\n", ntohs(piu->card_type));

	ipmi_sprom_iu_sdr_dump (&piu->fan_rpm,  "FAN RPM");
	ipmi_sprom_iu_sdr_dump (&piu->fan_temp, "FAN TEMP");
	ipmi_sprom_iu_sdr_dump (&piu->fan_volt, "FAN VOLT");
	return (0);
}

int ipmi_psu_iu_dump (ipmi_psu_internal_use_t* piu)
{
	printf(" PSU Internal Use Area\n");
	printf("\tCARD_TYPE	: %d\n", ntohs(piu->card_type));

	ipmi_raw_dump ((uint8_t *)&piu->psu_temp, sizeof(ipmi_iu_sdr_t));
	ipmi_sprom_iu_sdr_dump (&piu->psu_temp, "PSU TEMP");
	ipmi_sprom_iu_sdr_dump (&piu->psu_in_volt, "PSU IN VOLT");
	ipmi_sprom_iu_sdr_dump (&piu->psu_out_volt1, "PSU OUT VOLT1");
	ipmi_sprom_iu_sdr_dump (&piu->psu_out_volt2, "PSU OUT VOLT2");
	ipmi_sprom_iu_sdr_dump (&piu->psu_in_current, "PSU AMP");
	ipmi_sprom_iu_sdr_dump (&piu->psu_out_current, "PSU OUT AMP");
	ipmi_sprom_iu_sdr_dump (&piu->psu_rpm, "PSU RPM");

	return (0);
}

int ipmi_psu_iu_print (ipmi_psu_internal_use_t* piu)
{
	printf(" PSU Internal Use Area\n");
	printf("\tCARD_TYPE	: %d\n", ntohs(piu->card_type));

	ipmi_sprom_iu_sdr_print (&piu->psu_temp, "PSU TEMP");
	ipmi_sprom_iu_sdr_print (&piu->psu_in_volt, "PSU IN VOLT");
	ipmi_sprom_iu_sdr_print (&piu->psu_out_volt1, "PSU OUT VOLT1");
	ipmi_sprom_iu_sdr_print (&piu->psu_out_volt2, "PSU OUT VOLT2");
	ipmi_sprom_iu_sdr_print (&piu->psu_in_current, "PSU AMP");
	ipmi_sprom_iu_sdr_print (&piu->psu_out_current, "PSU OUT AMP");
	ipmi_sprom_iu_sdr_print (&piu->psu_rpm, "PSU RPM");

	return (0);
}

int ipmi_sprom_internal_use_dump (void *pinternal, uint16_t card_type)
{
	switch (card_type) {
		case	IPMI_IU_CARD_TYPE_IOM:
		case	IPMI_IU_CARD_TYPE_IOM2:
		case	IPMI_IU_CARD_TYPE_SC_BP:
			{
			ipmi_iom_internal_use_t *piu = 
				(ipmi_iom_internal_use_t*)pinternal;
			ipmi_iom_iu_dump (piu);
			}
			break;

		case	IPMI_IU_CARD_TYPE_OPLIN:
		case	IPMI_IU_CARD_TYPE_PALO:
		case	IPMI_IU_CARD_TYPE_MENLO:
		case	IPMI_IU_CARD_TYPE_MENLO_E:
		case	IPMI_IU_CARD_TYPE_NIANTIC:
		case	IPMI_IU_CARD_TYPE_NETEFFECT:
		case	IPMI_IU_CARD_TYPE_SCHULTZ:
		case	IPMI_IU_CARD_TYPE_TIGERSHARK:
		case	IPMI_IU_CARD_TYPE_EVEREST:
		case	IPMI_IU_CARD_TYPE_MONTEREYPARK:
		case	IPMI_IU_CARD_TYPE_VASONA:
		case	IPMI_IU_CARD_TYPE_DUBLIN:
		case	IPMI_IU_CARD_TYPE_FREMONT:
		case	IPMI_IU_CARD_TYPE_LIVERMORE:
		case	IPMI_IU_CARD_TYPE_UNKNOWN:
			{
			ipmi_mezz_internal_use_t *piu = 
				(ipmi_mezz_internal_use_t*)pinternal;
			ipmi_mezz_iu_dump (piu);
			}
			break;

		case	IPMI_IU_CARD_TYPE_IBMC:
			{
			ipmi_ibmc_internal_use_t *piu = 
				(ipmi_ibmc_internal_use_t*)pinternal;
			ipmi_bmc_iu_dump (piu);
			}
			break;

		case	IPMI_IU_CARD_TYPE_SC_FAN:
			{
			ipmi_fan_internal_use_t *piu = 
				(ipmi_fan_internal_use_t*)pinternal;
			ipmi_fan_iu_dump (piu);
			}
			break;

		case	IPMI_IU_CARD_TYPE_SC_PSU:
			{
			ipmi_psu_internal_use_t *piu = 
				(ipmi_psu_internal_use_t*)pinternal;
			ipmi_psu_iu_dump (piu);
			}
			break;

		case	IPMI_IU_CARD_TYPE_TURLOCK:
			{
			ipmi_hddbp_internal_use_t *piu = 
				(ipmi_hddbp_internal_use_t*)pinternal;
			ipmi_hddbp_iu_dump (piu);
			}
			break;

		default:
			return (-1);

	}
	return (0);
}

// Chassis Info.
int ipmi_sprom_chassis_info_create (ipmi_sprom_chassis_info_t *pchassis)
{
	if (!pchassis) return 0;

	pchassis->version	= IPMI_SPROM_CHASSIS_VERSION;
	if (!pchassis->length) {
		pchassis->length	= M8_SIZE(sizeof(ipmi_sprom_chassis_info_t));
		pchassis->no_more_tl	= IPMI_SPROM_NO_MORE_TYPE_LENGTH;
		pchassis->type		= IPMI_SPROM_CHASSIS_TYPE_RACK_MOUNT_CHASSIS;
	}

	FILL_TL_STR("CHASSIS PART NUM", pchassis->part_num,
			IPMI_SPROM_CHASSIS_PART_NUMBER_SIZE);
	FILL_TL_STR("CHASSIS SERIAL NUMBER", pchassis->serial_num,
			IPMI_SPROM_CHASSIS_SERIAL_NUMBER_SIZE);

	if (pchassis->length == M8_SIZE(sizeof(ipmi_sprom_chassis_info_t))) {
		FILL_TL_STR("CHASSIS MFG INFO", pchassis->mfg_info, 
			IPMI_SPROM_CHASSIS_MFG_INFO_SIZE);
	}

	ipmi_zero_checksum_create((uint8_t*)pchassis, (pchassis->length << 3));
	return (0);
}

void ipmi_sprom_chassis_info_dump (ipmi_sprom_chassis_info_t *pchassis)
{
	char *pbuf;
	if (!pchassis) return;
	printf(" Chassis Info Area  (%d/%d) \n", 
			(int)(pchassis->length<<3), (int)sizeof(*pchassis));
	printf("\tCHASSIS TYPE	: 0x%X\n", pchassis->type);
	PRINT_STR("\tCHASSIS PART NUM",   pchassis->part_num,
			IPMI_SPROM_CHASSIS_PART_NUMBER_SIZE);
	PRINT_STR("\tCHASSIS SERIAL NUM",   pchassis->serial_num,
			IPMI_SPROM_CHASSIS_SERIAL_NUMBER_SIZE);
	pbuf = (char *)&pchassis->serial_num_tl;
	pbuf += (*pbuf & 0x3F)+1;

	if ((uint8_t)*pbuf == (uint8_t)IPMI_SPROM_NO_MORE_TYPE_LENGTH) 
		return;
	PRINT_STR("\tCHASSIS MFG INFO",   pchassis->mfg_info,
			IPMI_SPROM_CHASSIS_MFG_INFO_SIZE);
}

// board info
int ipmi_sprom_board_info_create (ipmi_sprom_board_info_t *pboard)
{
	ipmi_sprom_board_info_t *pbrd = 
			(ipmi_sprom_board_info_t*) pboard;
	uint32_t     data = 0x00;
	uint8_t	     data8= 0x00;
	if (!pboard) return 0;

	pboard->version		= IPMI_SPROM_BOARD_VERSION;
	pboard->length		= M8_SIZE(sizeof(ipmi_sprom_board_info_t));
	pboard->language_code	= IPMI_LC_ENGLISH_0;
	pboard->no_more_tl	= IPMI_SPROM_NO_MORE_TYPE_LENGTH;
	pboard->fru_file_id_tl	= IPMI_SPROM_TYPE_CODE_LC_MSK |
				  IPMI_SPROM_BOARD_FRU_FILE_ID_SIZE;
	pboard->mfg_info_tl	= IPMI_SPROM_TYPE_CODE_LC_MSK |
				  IPMI_SPROM_BOARD_MFG_INFO_SIZE;
    pboard->custom_id_tl = 0x35;

	FILL_DATE("MFG DATE", pboard->mfg_date_time);

	// Get MFG Info
	FILL_TL_STR("MFG INFO", pboard->mfg_info, 
		IPMI_SPROM_BOARD_MFG_INFO_SIZE);

	// Get Product Name
	FILL_TL_STR("PRODUCT NAME", pboard->product_name, 
		IPMI_SPROM_BOARD_PRODUCT_NAME_SIZE);

	// Get Serial Number
	FILL_TL_STR("SERIAL NUM", pboard->serial_num, 
		IPMI_SPROM_BOARD_SERIAL_NUMBER_SIZE);

	// Set Part Number
	FILL_TL_STR("PART NUM", pboard->part_num, 
		IPMI_SPROM_BOARD_PART_NUMBER_SIZE);

	FILL_TL_STR("FRUFILE ID", pboard->fru_file_id,
                IPMI_SPROM_BOARD_FRU_FILE_ID_SIZE);

	if ((pboard->custom_id_tl&0x3F) ==  IPMI_SPROM_BOARD_CUSTOM_ID_SIZE) {
		// Get Bom REV
		FILL_STR("PART NUM REVISION", pboard->bom_rev, 2);

		data = (uint32_t)pboard->hw_rev;
		FILL_INT("FAB VERSION",  %02d, %d, data);
		pboard->hw_rev = (uint8_t) data;

		FILL_STR("VID", pboard->pid_rev,3); 
#if defined (DIAG_IOM2)
// Not needed for IOM2
#else
		// Set CLEI 
		FILL_TL_STR("CLEI", pboard->clei, 
			IPMI_SPROM_BOARD_CLEI_SIZE);
#endif
	} else {
		// Get Bom REV
		data8 = (uint8_t)pbrd->bom_rev[0];
		FILL_HEX("PART NUM REVISION",  %02X, %hhx, data8);
		pbrd->bom_rev[0] = (uint8_t) data8;

		data = (uint32_t)pbrd->hw_rev;
		FILL_INT("FAB VERSION",  %02d, %d, data);
		pbrd->hw_rev = (uint8_t) data;

		FILL_STR("VID", pbrd->pid_rev,3); 

		// Set CLEI 
		FILL_TL_STR("CLEI", pbrd->clei, 
			IPMI_SPROM_BOARD_CLEI_SIZE);
		FILL_TL_STR("PAD", pboard->pad,
			IPMI_SPROM_BOARD_NIM_PAD_SIZE);
	}

	ipmi_zero_checksum_create((uint8_t*)pboard, sizeof(*pboard));
	return (0);	
}

int ipmi_sprom_board_info_set_serial (ipmi_sprom_board_info_t *pboard)
{
	uint8_t *pbuf = (uint8_t*)&pboard->mfg_info_tl;

	pbuf += ((*pbuf & 0x3F)+1); // mfg_info
	pbuf += ((*pbuf & 0x3F)+1); // product_name

	// Get Serial Number
	FILL_STR("BRD Serial Num", pbuf, 
		IPMI_SPROM_BOARD_SERIAL_NUMBER_SIZE);
	ipmi_zero_checksum_create((uint8_t*)pboard, sizeof(*pboard));
	return (0);	
}

void ipmi_sprom_board_info_dump (ipmi_sprom_board_info_t *pboard)
{
	uint8_t *pbuf, len;
	if (!pboard) return;


	printf("\n Board Info Area (%d)\n", (int)sizeof(*pboard));
        printf("\tMFG DATE      : %02d/%02d/%02d\n",
                                        pboard->mfg_date_time[0],
                                        pboard->mfg_date_time[1],
                                        pboard->mfg_date_time[2]);

	pbuf = (uint8_t*)&pboard->mfg_info_tl;
	len = *pbuf & 0x3F;
	PRINT_STR_TL("\tMFG INFO",   pbuf+1, len);

	pbuf += (len+1);
	len = *pbuf & 0x3F;
	PRINT_STR_TL("\tPRODUCT NAME",   pbuf+1, len);

	pbuf += (len+1);
	len = *pbuf & 0x3F;
	PRINT_STR_TL("\tSERIAL NUM",   pbuf+1, len);

	pbuf += (len+1);
	len = *pbuf & 0x3F;
	PRINT_STR_TL("\tPART NUM",   pbuf+1, len);

	pbuf += (len+1);
	len = *pbuf & 0x3F;
	PRINT_STR_TL("\tFRUFILE ID", pbuf+1, len);

	pbuf += (len+1);
	len = *pbuf & 0x3F;
//	printf("\tCUSTOM ID TL  : 0x%X\n", *pbuf);

	pbuf++;
	if (len == IPMI_SPROM_BOARD_CUSTOM_ID_SIZE) {
		printf("\tPART NUM REV  : %C%C\n", pbuf[0], pbuf[1]);
		pbuf+=2;
	} else {
		printf("\tPART NUM REV  : %02X\n", *pbuf++);
	}
	printf("\tFAB VERSION   : %02d\n", *pbuf++);
	printf("\tVID           : %C%C%C\n", pbuf[0], pbuf[1], pbuf[2]); 
	pbuf+=3;

	len = *pbuf & 0x3F;
#if defined (DIAG_IOM2)
// Not needed for IOM2
#else
	PRINT_STR_TL("\tCLEI",   pbuf+1, len);
	pbuf += (len+1);
	len = *pbuf & 0x3F;
	PRINT_STR_TL("\tPAD", pbuf+1, len);
#endif
	pbuf += (len+1);
//	printf("\tNO MORE TL    : 0x%X\n", *pbuf);
	pbuf++;
//	printf("\tCHECKSUM      : 0x%X\n", *pbuf);
}

// Product info
int ipmi_sprom_product_info_create (ipmi_sprom_product_info_t *pproduct)
{
	uint32_t data = 0x00;
	if (!pproduct) return 0;

	pproduct->version	= IPMI_SPROM_BOARD_VERSION;
	pproduct->length	= M8_SIZE(sizeof(ipmi_sprom_product_info_t));
	pproduct->language_code = IPMI_LC_ENGLISH_0;
	pproduct->no_more_tl	= IPMI_SPROM_NO_MORE_TYPE_LENGTH;
	pproduct->pad_tl	= IPMI_SPROM_TYPE_CODE_LC_MSK |
				  IPMI_SPROM_PRODUCT_PAD_SIZE;

	// Get MFG Info
	FILL_TL_STR("MFG INFO", pproduct->mfg_name, 
		IPMI_SPROM_PRODUCT_MFG_NAME_SIZE);

	FILL_TL_STR("PRODUCT NAME", pproduct->product_name, 
		IPMI_SPROM_PRODUCT_NAME_SIZE);

	FILL_TL_STR("PROD PART NUM", pproduct->part_model, 
			IPMI_SPROM_PRODUCT_PART_MODEL_SIZE);

	FILL_TL_STR("PROD SERIAL NUM", pproduct->serial_num, 
			IPMI_SPROM_PRODUCT_SERIAL_NUMBER_SIZE);

	FILL_TL_STR("PROD PART NUM REV", pproduct->prd_version, 
			IPMI_SPROM_PRODUCT_VERSION_SIZE);

//	data = (uint32_t)pproduct->bom_rev;
//	FILL_INT("PART NUM REVISION",  %02d, %d, data);
	pproduct->bom_rev = (uint8_t) 0x00;

	data = (uint32_t)pproduct->hw_rev;
	FILL_INT("PROD FAB VERSION",  %02d, %d, data);
	pproduct->hw_rev = (uint8_t) data;

	FILL_STR("PROD VID", pproduct->pid_rev,3); 

	ipmi_zero_checksum_create((uint8_t*)pproduct, sizeof(*pproduct));
	return (0);
}


// Product info
int ipmi_sprom_bmc_product_info_create (ipmi_sprom_bmc_product_info_t *pproduct)
{
	if (!pproduct) return 0;

	pproduct->version	= IPMI_SPROM_BOARD_VERSION;
	pproduct->length	= M8_SIZE(sizeof(ipmi_sprom_bmc_product_info_t));
	pproduct->language_code = IPMI_LC_ENGLISH_0;
	pproduct->no_more_tl	= IPMI_SPROM_NO_MORE_TYPE_LENGTH;

	// Get MFG Info
	FILL_TL_STR("MFG INFO", pproduct->mfg_name, 
		IPMI_SPROM_BMC_PRODUCT_MFG_NAME_SIZE);

	FILL_TL_STR("PRODUCT NAME", pproduct->product_name, 
		IPMI_SPROM_BMC_PRODUCT_NAME_SIZE);

	FILL_TL_STR("PROD PART NUM", pproduct->part_model, 
			IPMI_SPROM_BMC_PRODUCT_PART_MODEL_SIZE);

	FILL_TL_STR("PROD SERIAL NUM", pproduct->serial_num, 
			IPMI_SPROM_BMC_PRODUCT_SERIAL_NUMBER_SIZE);

    FILL_TL_STR("FRUFILE ID", pproduct->fru_file_id,
            IPMI_SPROM_BMC_PRODUCT_FRU_FILE_ID_SIZE);

	FILL_STR("PROD PART NUM REV", pproduct->bom_rev, 2);

	pproduct->prd_version = pproduct->bom_rev[0];

	FILL_STR("PROD VID", pproduct->pid_rev,3); 

	FILL_TL_STR("PAD", pproduct->pad,
			IPMI_SPROM_BMC_PRODUCT_NIM_PAD_SIZE);
	ipmi_zero_checksum_create((uint8_t*)pproduct, sizeof(*pproduct));
	return (0);
}

int ipmi_sprom_ncsi_multi_record_create(ipmi_ncsi_multi_record_t *pmr)
{
	ipmi_iu_rec_mac_t mac;
	uint32_t data;

	data = pmr->power_source;
	FILL_INT("POWER SOURCE", %d, %d, data);
	pmr->power_source = data;

	data = ntohs(pmr->peak_standby_power) / 10;
	FILL_INT("PEAK STANDYBY POWER", %d, %d, data);
	pmr->peak_standby_power = htons(data * 10);

	memcpy(mac.mac, pmr->mac, 6);
	if (ipmi_sprom_iu_mac_create(&mac, pmr->mac_count))
		return (1);
	pmr->mac_count = ntohs(mac.cnt.value);
	memcpy(pmr->mac, mac.mac, 6);

	ipmi_multi_record_checksum_create(&pmr->header);

	return (0);
}

void ipmi_sprom_product_info_dump (ipmi_sprom_product_info_t *pproduct)
{
	uint8_t *pbuf, len;
	if (!pproduct) return;

	printf(" Product Info Area (%d)\n", (int)sizeof(*pproduct));
	printf("\tLANGUAGE CODE : 0x%X\n", pproduct->language_code);

	pbuf = &pproduct->mfg_name_tl;
	len = *pbuf & 0x3F;
	PRINT_STR_TL("\tMFG NAME",   pbuf+1, len);

	pbuf += (len+1);
	len = *pbuf & 0x3F;
	PRINT_STR_TL("\tPRODUCT NAME",   pbuf+1, len);

	pbuf += (len+1);
	len = *pbuf & 0x3F;
	PRINT_STR_TL("\tPRODUCT PARTNUM",   pbuf+1, len);

	pbuf += (len+1);
	len = *pbuf & 0x3F;
	PRINT_STR_TL("\tVERSION",   pbuf+1, len);

	pbuf += (len+1);
	len = *pbuf & 0x3F;
	PRINT_STR_TL("\tSERIAL NUM",   pbuf+1, len);

	pbuf += (len+1);
	len = *pbuf & 0x3F;
//	printf("\tASSET TAG TL	: 0x%X\n", *pbuf);

	pbuf += (len+1);
	len = *pbuf & 0x3F;
//	printf("\tFRUFILE ID TL : 0x%X\n", *pbuf);

	pbuf += (len+1);
	len = *pbuf & 0x3F;
//	printf("\tCUSTOM   ID TL: 0x%X\n", *pbuf);

	pbuf++;
	printf("\tPART NUM REV  : %02X\n", pbuf[3]);
	printf("\tFAB VERSION   : %02d\n", pbuf[4]);
	printf("\tVID           : %C%C%C\n", pbuf[0], pbuf[1], pbuf[2]); 
        printf("\n");
}

void ipmi_sprom_bmc_product_info_dump (ipmi_sprom_bmc_product_info_t *pproduct)
{
	uint8_t *pbuf, len;
	if (!pproduct) return;

	printf(" Product Info Area (%d)\n", (int)sizeof(*pproduct));
	printf("\tLANGUAGE CODE : 0x%X\n", pproduct->language_code);

	pbuf = &pproduct->mfg_name_tl;
	len = *pbuf & 0x3F;
	PRINT_STR_TL("\tMFG NAME",   pbuf+1, len);

	pbuf += (len+1);
	len = *pbuf & 0x3F;
	PRINT_STR_TL("\tPRODUCT NAME",   pbuf+1, len);

	pbuf += (len+1);
	len = *pbuf & 0x3F;
	PRINT_STR_TL("\tPRODUCT PARTNUM",   pbuf+1, len);

	pbuf += (len+1);
	len = *pbuf & 0x3F;
	printf("\tVERSION       : %d\n",   pbuf[1]);

	pbuf += (len+1);
	len = *pbuf & 0x3F;
	PRINT_STR_TL("\tSERIAL NUM",   pbuf+1, len);

	pbuf += (len+1);
	len = *pbuf & 0x3F;
	PRINT_STR_TL("\tASSET  TAG",   pbuf+1, len);

	// Fru FILE ID
	pbuf += (len+1);
	len = *pbuf & 0x3F;
	PRINT_STR_TL("\tFRU FILE ID",   pbuf+1, len);

	// Custom ID
	pbuf += (len+1);
	len = *pbuf & 0x3F;

	pbuf++;
	printf("\tPROD PART NUM REV: %c%c\n", pbuf[0], pbuf[1]);
	pbuf+=2;
	printf("\tVID REVISION	: %C%C%C\n", pbuf[0], pbuf[1], pbuf[2]); 
	pbuf+=3;

	len = *pbuf & 0x3F;
	PRINT_STR_TL("\tPAD",   pbuf+1, len);
	pbuf += (len+1);
	// CHECK SUM
	pbuf = ((u_int8_t*)pproduct) + (pproduct->length << 3) - 1;
}

void ipmi_sprom_iom_chassis_dump (sprom_ipmi_iom_bp_t *sprom)
{
	uint8_t *pbuf = (uint8_t*)sprom;	
	ipmi_sprom_chassis_info_dump((ipmi_sprom_chassis_info_t*) 
			(pbuf + (sprom->common_header.chassis_info << 3)));
	ipmi_sprom_board_info_dump ((ipmi_sprom_board_info_t*)
			(pbuf + (sprom->common_header.board_info << 3)));
}

void ipmi_sprom_iom_board_dump (sprom_ipmi_iom_t *sprom)
{
#if defined (DIAG_IOM2)
	ipmi_sprom_internal_use_dump (&sprom->internal_use,
					IPMI_IU_CARD_TYPE_IOM2);
#else
	ipmi_sprom_internal_use_dump (&sprom->internal_use,
					IPMI_IU_CARD_TYPE_IOM);
#endif
	ipmi_sprom_board_info_dump (&sprom->board_info);
}

int ipmi_sprom_iom_dump(char *pbuf)
{
	int rc = 0;
	sprom_ipmi_iom_t psprom;
#if defined(DIAG_IOM2)
	rc = ipmi_sprom_data_get (IPMI_IU_CARD_TYPE_IOM2, pbuf, 
		sizeof(sprom_ipmi_iom_t),
		&psprom, sizeof(psprom));
	if (!rc) {
		printf("Dumping IOM2 Board Contents\n");
		ipmi_sprom_iom_board_dump (&psprom);
	} else {
		printf(" Error: Sprom iom get data rc = %d\n", rc);
	}
#else
	rc = ipmi_sprom_data_get (IPMI_IU_CARD_TYPE_IOM, pbuf, 
		sizeof(sprom_ipmi_iom_t),
		&psprom, sizeof(psprom));
	if (!rc) {
		printf("Dumping IOM Board Contents\n");
		ipmi_sprom_iom_board_dump (&psprom);
	} else {
		printf(" Error: Sprom iom get data rc = %d\n", rc);
	}
#endif
	return (0);
}


int ipmi_sprom_iom_bp_fill(sprom_ipmi_iom_bp_t *sprom)
{
	uint8_t *pbuf = (uint8_t*)sprom;	

	if (!sprom->common_header.chassis_info ||
	    !sprom->common_header.board_info) {
		printf("  Create Common Header\n");
		ipmi_sprom_common_header_create(&sprom->common_header, 0,
			M8_SIZE(sizeof(ipmi_sprom_chassis_info_t)),
			M8_SIZE(sizeof(ipmi_sprom_board_info_t)),
			0, 0);
	}

	printf("  Create Chassis Info\n");
	ipmi_sprom_chassis_info_create ((ipmi_sprom_chassis_info_t*)
			(pbuf + (sprom->common_header.chassis_info << 3)));

	printf("  Create Board Info\n");
	ipmi_sprom_board_info_create ((ipmi_sprom_board_info_t*)
			(pbuf + (sprom->common_header.board_info << 3)));

	return (0);
}

int ipmi_sprom_iom_fill(sprom_ipmi_iom_t *sprom)
{
	printf("  Create Common Header\n");
	ipmi_sprom_common_header_create(&sprom->common_header, 
		M8_SIZE(sizeof(ipmi_iom_internal_use_t)),
		0,
		M8_SIZE(sizeof(ipmi_sprom_board_info_t)),
		0,
		0);

	printf("  Create Internal Use Info\n");
#if defined (DIAG_IOM2)
	ipmi_sprom_internal_use_create (&sprom->internal_use,
					IPMI_IU_CARD_TYPE_IOM2);
	printf("  Create Board Info\n");
	printf("  Be Sure to program Correctly\n");
//	printf("  P0B - Part:73-11623-02, BOM:0x05\n");
//	printf("  P21 - Part:73-13196-001, BOM:0x0\n");
#else
	ipmi_sprom_internal_use_create (&sprom->internal_use,
					IPMI_IU_CARD_TYPE_IOM);
	printf("  Create Board Info\n");
	printf("  Be Sure to program Correctly\n");
	printf("  P0B - Part:73-11623-02, BOM:0x05\n");
	printf("  P0C - Part:73-11623-02, BOM:0x07\n");
	printf("  P0D - Part:73-11623-02, BOM:0x08\n");
	printf("  P1A - Part:73-11623-03, BOM:0x05\n");
	printf("  P1A - Part:73-11623-03, BOM:0x06\n");
#endif
	ipmi_sprom_board_info_create (&sprom->board_info);

	return (0);
}

void ipmi_sprom_board_fru_file_id_create( uint8_t *pbuf)
{
	ipmi_sprom_board_info_t *pboard = (ipmi_sprom_board_info_t *)pbuf;
	FILL_STR("FRU FILE ID", pboard->fru_file_id, 
			IPMI_SPROM_BOARD_FRU_FILE_ID_SIZE);
	ipmi_zero_checksum_create ((uint8_t*)pboard, pboard->length<<3);
}

void ipmi_sprom_product_fru_file_id_create(uint8_t *pprod)
{
	uint8_t *pbuf = pprod;
	uint8_t len;
	pbuf += 3;			// Version + length + language
	pbuf += (*pbuf & 0x3F) + 1;	// Mfg Name
	pbuf += (*pbuf & 0x3F) + 1;	// Product Name
	pbuf += (*pbuf & 0x3F) + 1;	// Part Model 
	pbuf += (*pbuf & 0x3F) + 1;	// Prd Version 
	pbuf += (*pbuf & 0x3F) + 1;	// Serial #
	pbuf += (*pbuf & 0x3F) + 1;	// Asset Tag 
	len = *pbuf & 0x3F;
	if (len) {
		FILL_STR("PROD FRU FILE ID", pbuf+1, len); // Fru File Id
		ipmi_zero_checksum_create ((uint8_t*)pprod, (*(pprod+1))<<3);
	}
}

void ipmi_sprom_psu_dump (sprom_ipmi_psu_t *sprom)
{
	ipmi_sprom_product_info_dump (&sprom->product_info);
}

void ipmi_sprom_iom_psu_dump (sprom_ipmi_iom_psu_t *sprom)
{
	ipmi_sprom_product_info_dump (&sprom->product_info);
	ipmi_sprom_internal_use_dump (&sprom->internal_use, 
				IPMI_IU_CARD_TYPE_SC_PSU);
}

void ipmi_sprom_iom_psu_p0_dump (sprom_ipmi_iom_psu_p0_t *sprom)
{
	ipmi_sprom_product_info_dump (&sprom->product_info);
}

int ipmi_sprom_iom_psu_fill(sprom_ipmi_iom_psu_t *sprom)
{
	printf("  Create Common Header\n");
	ipmi_sprom_common_header_create(&sprom->common_header, 
	M8_SIZE(sizeof(ipmi_psu_internal_use_t)), 0, 0, 
	M8_SIZE(sizeof(ipmi_sprom_product_info_t)), 0);	

	printf("  Create Product Info\n");
	ipmi_sprom_product_info_create (&sprom->product_info);

	printf("  Create Internal Use\n");
	ipmi_sprom_internal_use_create (&sprom->internal_use,
					IPMI_IU_CARD_TYPE_SC_PSU);
	return (0);
}

int ipmi_sprom_psu_fill(sprom_ipmi_psu_t *sprom)
{
	printf("  Create Common Header\n");
	ipmi_sprom_common_header_create(&sprom->common_header, 0, 0, 0, 
	M8_SIZE(sizeof(ipmi_sprom_product_info_t)), 0);	

	printf("  Create Product Info\n");
	ipmi_sprom_product_info_create (&sprom->product_info);

	return (0);
}

int ipmi_sprom_iom_psu_p0_fill(sprom_ipmi_iom_psu_p0_t *sprom)
{
	printf("  Create Common Header\n");
	ipmi_sprom_common_header_create(&sprom->common_header, 0, 0, 0, 
	M8_SIZE(sizeof(ipmi_sprom_product_info_t)), 0);	

	printf("  Create Product Info\n");
	ipmi_sprom_product_info_create (&sprom->product_info);

	return (0);
}

void ipmi_sprom_iom_fan_dump (sprom_ipmi_iom_fan_t *sprom)
{
	ipmi_sprom_internal_use_dump (&sprom->internal_use,
					IPMI_IU_CARD_TYPE_SC_FAN);
	ipmi_sprom_board_info_dump (&sprom->board_info);
}

int ipmi_sprom_iom_fan_fill(sprom_ipmi_iom_fan_t *sprom)
{
	printf("  Create Common Header\n");
	ipmi_sprom_common_header_create(&sprom->common_header, 
			M8_SIZE(sizeof(ipmi_fan_internal_use_t)),
			0, 
			M8_SIZE(sizeof(ipmi_sprom_board_info_t)),
			0, 0);	
	printf("  Create Internal Use Info\n");
	ipmi_sprom_internal_use_create (&sprom->internal_use,
					IPMI_IU_CARD_TYPE_SC_FAN);

	printf("  Create Board Info\n");
	ipmi_sprom_board_info_create (&sprom->board_info);

	return (0);
}

int ipmi_sprom_mezz_fill(sprom_ipmi_mezz_t *sprom, int card_type)
{
	printf("  Create Common Header\n");
	ipmi_sprom_common_header_create(&sprom->common_header, 
		M8_SIZE(sizeof(ipmi_mezz_internal_use_t)),
		0,
		M8_SIZE(sizeof(ipmi_sprom_board_info_t)),
		0,
		0);

	printf("  Create Internal Use Info\n");
	ipmi_sprom_internal_use_create (&sprom->internal_use, card_type);

	printf("  Create Board Info\n");
	ipmi_sprom_board_info_create (&sprom->board_info);
	return (0);
}

void ipmi_sprom_mezz_dump (sprom_ipmi_mezz_t *sprom, int card_type)
{
	ipmi_sprom_internal_use_dump (&sprom->internal_use, card_type);
	ipmi_sprom_board_info_dump (&sprom->board_info);
}

int ipmi_sprom_mem_fill(sprom_ipmi_mem_t *sprom, int card_type)
{
	printf("  Create Common Header\n");
	ipmi_sprom_common_header_create(&sprom->common_header, 
		M8_SIZE(sizeof(ipmi_mem_internal_use_t)),
		0,
		M8_SIZE(sizeof(ipmi_sprom_board_info_t)),
		0,
		0);

	printf("  Create Internal Use Info\n");
	ipmi_sprom_internal_use_create (&sprom->internal_use, card_type);

	printf("  Create Board Info\n");
	ipmi_sprom_board_info_create (&sprom->board_info);
	return (0);
}

void ipmi_sprom_mem_dump (sprom_ipmi_mem_t *sprom, int card_type)
{
	ipmi_sprom_internal_use_dump (&sprom->internal_use, card_type);
	ipmi_sprom_board_info_dump (&sprom->board_info);
}

int ipmi_sprom_hddbp_fill(sprom_ipmi_hddbp_t *sprom, int card_type)
{
	printf("  Create Common Header\n");
	ipmi_sprom_common_header_create(&sprom->common_header, 
		M8_SIZE(sizeof(ipmi_mem_internal_use_t)),
		0,
		M8_SIZE(sizeof(ipmi_sprom_board_info_t)),
		0,
		0);

	printf("  Create Internal Use Info\n");
	ipmi_sprom_internal_use_create (&sprom->internal_use, card_type);

	printf("  Create Board Info\n");
	ipmi_sprom_board_info_create (&sprom->board_info);
	return (0);
}

void ipmi_sprom_hddbp_dump (sprom_ipmi_hddbp_t *sprom, int card_type)
{
	ipmi_sprom_internal_use_dump (&sprom->internal_use, card_type);
	ipmi_sprom_board_info_dump (&sprom->board_info);
}

int ipmi_sprom_mpark_fill(sprom_ipmi_mpark_t *sprom, int card_type)
{
	u_int64_t mac_num;
	int i;

	printf("  Create Common Header\n");
	ipmi_sprom_common_header_create(&sprom->common_header,
		M8_SIZE(sizeof (ipmi_mezz_internal_use_t)),
		0,
		M8_SIZE(sizeof (ipmi_sprom_board_info_t)),
		0,
		M8_SIZE(sizeof (ipmi_ncsi_multi_record_t)));

	printf("  Create Internal Use Info\n");
	ipmi_sprom_internal_use_create(&sprom->internal_use, card_type);

	printf("  Create Board Info\n");
	ipmi_sprom_board_info_create(&sprom->board_info);

	/*
	 * Prime the MAC address in the NC-SI multi-record with the
	 * next contiguous MAC address after the reservation in the
	 * internal use area.
	 */
	mac_num = 0;
	for (i = 0; i < 6; i++)
		mac_num = (mac_num << 8) | sprom->internal_use.mac.mac[i];
	mac_num += ntohs(sprom->internal_use.mac.cnt.value);
	for (i = 5; i >= 0; i--) {
		sprom->ncsi_multi_record.mac[i] = mac_num & 0xff;
		mac_num >>= 8;
	}

	printf("  Create NC-SI Multi Record\n");
	ipmi_sprom_ncsi_multi_record_create(&sprom->ncsi_multi_record);

	return (0);
}

void ipmi_sprom_mpark_dump (sprom_ipmi_mpark_t *sprom, int card_type)
{
	ipmi_sprom_internal_use_dump (&sprom->internal_use, card_type);
	ipmi_sprom_board_info_dump (&sprom->board_info);
        ipmi_sprom_ncsi_multi_record_dump(&sprom->ncsi_multi_record);
}

int ipmi_sprom_bbu_fill(sprom_ipmi_bbu_t *sprom, int card_type)
{
	ipmi_sprom_board_info_t *pbrd = &sprom->board_info;
	ipmi_sprom_bmc_product_info_t *pprd = &sprom->product_info;

	printf("  Create Common Header\n");
	ipmi_sprom_common_header_create(&sprom->common_header, 
		0,
		0,
		M8_SIZE(sizeof(ipmi_sprom_board_info_t)),
		M8_SIZE(sizeof(ipmi_sprom_bmc_product_info_t)),
		0);

	printf("  Create Board Info\n");
	ipmi_sprom_board_info_create (&sprom->board_info);

	printf("  Create Product Info\n");
	// Copy serial number from board to product.
	memcpy(pprd->serial_num, pbrd->serial_num, IPMI_SPROM_PRODUCT_SERIAL_NUMBER_SIZE);
	ipmi_sprom_bmc_product_info_create (&sprom->product_info);

	return (0);
}

void ipmi_sprom_bbu_dump (sprom_ipmi_bbu_t *sprom)
{
	ipmi_sprom_board_info_dump (&sprom->board_info);
	ipmi_sprom_bmc_product_info_dump (&sprom->product_info);
}

int ipmi_sprom_ibmc_fill(sprom_ipmi_ibmc_t *sprom, int card_type)
{
	ipmi_sprom_board_info_t *pbrd = &sprom->board_info;
	ipmi_sprom_bmc_product_info_t *pprd = &sprom->product_info;

	printf("  Create Common Header\n");
	ipmi_sprom_common_header_create(&sprom->common_header, 
		M8_SIZE(sizeof(ipmi_ibmc_internal_use_t)),
		0,
		M8_SIZE(sizeof(ipmi_sprom_board_info_t)),
		M8_SIZE(sizeof(ipmi_sprom_bmc_product_info_t)),
		0);

	printf("  Create Internal Use Info\n");
	ipmi_sprom_internal_use_create (&sprom->internal_use, card_type);

	printf("  Create Board Info\n");
	ipmi_sprom_board_info_create (&sprom->board_info);

	//Copy serial number from board info to product info
	// Except for LA - LA has different information for 
	// Product info and Board Info.
	if (strncasecmp((const char *)sprom->board_info.product_name, 
		(const char *)"R250-2480805", strlen("R250-2480805"))) {
		memcpy(pprd->serial_num, pbrd->serial_num, 
			IPMI_SPROM_PRODUCT_SERIAL_NUMBER_SIZE);
	}

	ipmi_sprom_bmc_product_info_create (&sprom->product_info);

	// Fix Blade Class if it is incorrect
	if (!strncasecmp((const char *)sprom->board_info.product_name, 
		(const char *)"N20-B6620-2", strlen("N20-B6620-2"))) {
		sprom->internal_use.blade_class =
			IPMI_SPROM_IBMC_IU_BLADE_CLASS_VENTURA;
		sprom_mem_fill((char*)pprd->fru_file_id, "VENT_008", 
			IPMI_SPROM_BMC_PRODUCT_FRU_FILE_ID_SIZE);
	} else if (!strncasecmp((const char *)sprom->board_info.product_name, 
		(const char *)"R250-2480805", strlen("R250-2480805"))) {
		sprom->internal_use.blade_class =
			IPMI_SPROM_IBMC_IU_BLADE_CLASS_NONE;
//		sprom_mem_fill((char*)pprd->fru_file_id, "LA_008", 
//			IPMI_SPROM_BMC_PRODUCT_FRU_FILE_ID_SIZE);
	}
	return (0);
}

void ipmi_sprom_ibmc_dump (sprom_ipmi_ibmc_t *sprom, int card_type)
{
	ipmi_sprom_internal_use_dump (&sprom->internal_use, card_type);
	ipmi_sprom_board_info_dump (&sprom->board_info);
	ipmi_sprom_bmc_product_info_dump (&sprom->product_info);
}


/////////////////////////////////////////////////////////////////////
// SanDiego PSU Routines
/////////////////////////////////////////////////////////////////////

void ipmi_sprom_sd_product_info_dump (ipmi_sprom_psu_product_info_t *pproduct)
{
	uint8_t *pbuf, len;
	if (!pproduct) return;

	printf(" Product Info Area SD (%d)\n", (int)sizeof(*pproduct));
	printf("\tLANGUAGE CODE : 0x%X\n", pproduct->language_code);

	pbuf = &pproduct->mfg_name_tl;
	len = *pbuf & 0x3F;
	PRINT_STR_TL("\tMFG NAME",   pbuf+1, len);

	pbuf += (len+1);
	len = *pbuf & 0x3F;
	PRINT_STR_TL("\tPRODUCT NAME",   pbuf+1, len);

	pbuf += (len+1);
	len = *pbuf & 0x3F;
	PRINT_STR_TL("\tPRODUCT PART NUM",   pbuf+1, len);

	pbuf += (len+1);
	len = *pbuf & 0x3F;
	PRINT_STR_TL("\tVERSION",   pbuf+1, len);

	pbuf += (len+1);
	len = *pbuf & 0x3F;
	PRINT_STR_TL("\tSERIAL NUM",   pbuf+1, len);

	pbuf += (len+1);
	len = *pbuf & 0x3F;
	PRINT_STR_TL("\tASSET TAG", pbuf+1, len);

	pbuf += (len+1);
	len = *pbuf & 0x3F;
	PRINT_STR_TL("\tFRUFILE ID", pbuf+1, len);

	// Custom ID
	pbuf += (len+1);
	len = *pbuf & 0x3F;
	pbuf++;
	printf("\tPID REV       : %C%C%C\n", pbuf[0], pbuf[1], pbuf[2]); 
        printf("\n");
}

void ipmi_sprom_mr_hdr_dump(ipmi_sprom_multi_record_header_t *pMrhdr)
{
	printf("  MR Record Type	: %d\n", pMrhdr->record_type);
	printf("  MR Header Version	: %d\n", pMrhdr->version);
	printf("  MR Record Length	: %d\n", pMrhdr->length);
	printf("  MR Record Checksum	: 0x%X\n", pMrhdr->record_checksum);
	printf("  MR Header Checksum	: 0x%X\n", pMrhdr->header_checksum);
}

void ipmi_sprom_mr_sd_dc_out_dump(ipmi_sprom_mr_dc_out_t *pDcout)
{
	printf("  MR DC Out Info	: %d\n",   pDcout->info);
	printf("  MR DC Out NominalVolt	: 0x%X\n", pDcout->nominal_volt);
	printf("  MR DC Out MaxNegVolt	: 0x%X\n", pDcout->max_neg_volt);
	printf("  MR DC Out MaxPosVolt	: 0x%X\n", pDcout->max_pos_volt);
	printf("  MR DC Out RippleNoise	: 0x%X\n", pDcout->ripple_noise);
	printf("  MR DC Out MinCurrent	: 0x%X\n", pDcout->min_amps);
	printf("  MR DC Out MaxCurrent	: 0x%X\n\n", pDcout->max_amps);
}
void ipmi_sprom_mr_sd_cisco_mr_dump(ipmi_sprom_mr_mp_cisco_card_t *pCiscoMr)
{
	printf("  MR CISCO ID           : %C%C%C%C%C\n",   pCiscoMr->id[0], pCiscoMr->id[1],
					pCiscoMr->id[2], pCiscoMr->id[3], pCiscoMr->id[4]);
	printf("  MR CISCO VERISON      : %d\n",   pCiscoMr->format_version);
	printf("  MR CISCO POWER        : 0x%04X\n",   pCiscoMr->power);

}

void ipmi_sprom_sd_psu_info_dump(ipmi_sprom_mr_psu_info_t *pPsuInfo)
{
	printf("  MR PSU Info Capacity	: 0x%X\n",   pPsuInfo->capacity);
	printf("  MR PSU Info PeakVal	: 0x%X\n",   pPsuInfo->peak_val);
	printf("  MR PSU Info Lsb	: 0x%X\n",   pPsuInfo->lsb);
	printf("  MR PSU Info InRushAmp	: 0x%X\n",   pPsuInfo->inrush_amp);
	printf("  MR PSU Info Interval	: 0x%X\n",   pPsuInfo->inrush_interval);
	printf("  MR PSU Info LoInputV1	: 0x%X\n",   pPsuInfo->lo_end_inp_v1);
	printf("  MR PSU Info HiInputV1	: 0x%X\n",   pPsuInfo->hi_end_inp_v1);
	printf("  MR PSU Info LoInputV2	: 0x%X\n",   pPsuInfo->lo_end_inp_v2);
	printf("  MR PSU Info HiInputV2	: 0x%X\n",   pPsuInfo->hi_end_inp_v2);
	printf("  MR PSU Info HiInputHz	: 0x%X\n",   pPsuInfo->hi_end_inp_hz);
	printf("  MR PSU Info InputTol	: 0x%X\n",   pPsuInfo->inp_drop_out_tol);
	printf("  MR PSU Info Flags	: 0x%X\n",   pPsuInfo->flags);
	printf("  MR PSU Info HoldUp	: 0x%X\n",   pPsuInfo->hold_up_time);
	printf("  MR PSU Info Peak Cap	: 0x%X\n",   pPsuInfo->peak_capacity);
	printf("  MR PSU Info Tot Watts	: 0x%X\n",   pPsuInfo->total_watts);
	printf("  MR PSU Info TachoThr	: 0x%X\n\n",   pPsuInfo->fail_tacho_thres);
}

void ipmi_sprom_sd_mr_dump_old (ipmi_psu_sd_mr_t *pMr)
{
	ipmi_sprom_mr_hdr_dump(&pMr->psu_info_hdr);
	ipmi_sprom_sd_psu_info_dump(&pMr->psu_info);

	ipmi_sprom_mr_hdr_dump(&pMr->dc_out_1_hdr);
	ipmi_sprom_mr_sd_dc_out_dump(&pMr->dc_out_1);

	ipmi_sprom_mr_hdr_dump(&pMr->dc_out_2_hdr);
	ipmi_sprom_mr_sd_dc_out_dump(&pMr->dc_out_2);

	ipmi_sprom_mr_hdr_dump(&pMr->cisco_mr_hdr);
	ipmi_sprom_mr_sd_cisco_mr_dump(&pMr->cisco_mr);
}

void ipmi_sprom_sd_psu_dump_old (sprom_ipmi_sd_psu_old_t *sprom)
{
	ipmi_sprom_sd_product_info_dump (&sprom->product_info);
	ipmi_sprom_sd_mr_dump_old (&sprom->multi_record);
}

void ipmi_sprom_sd_mr_dump(ipmi_sprom_psu_mr_t *pMr)
{
	printf("\n\n Multi Record Area SD (%d)\n", (int)sizeof(*pMr));
	ipmi_sprom_mr_hdr_dump(&pMr->psu_info_hdr);
	printf("  MR PSU OEM            : %c%c%c%c%c\n",   pMr->oem[0],
		pMr->oem[1], pMr->oem[2], pMr->oem[3], pMr->oem[4]);
	printf("  MR PSU Version        : %d%d%d%d\n",   pMr->version[0],
		pMr->version[1], pMr->version[2], pMr->version[3]);
	printf("  MR PSU Power          : 0x%X%X\n",   pMr->power[0],
		pMr->power[1]);
}

void ipmi_sprom_sd_psu_dump(sprom_ipmi_sd_psu_t *sprom)
{
	ipmi_sprom_sd_product_info_dump (&sprom->product_info);
	ipmi_psu_iu_print (&sprom->internal_use);
	ipmi_sprom_sd_mr_dump (&sprom->multi_record);
}


// SD Product info
int ipmi_sprom_sd_product_info_create (ipmi_sprom_psu_product_info_t *pproduct)
{
	if (!pproduct) return 0;

	pproduct->version	= IPMI_SPROM_BOARD_VERSION;
	pproduct->length	= M8_SIZE(sizeof(ipmi_sprom_psu_product_info_t));
	pproduct->language_code = IPMI_LC_ENGLISH_0;
	pproduct->no_more_tl	= IPMI_SPROM_NO_MORE_TYPE_LENGTH;

	// Get MFG Info
	FILL_TL_STR("MFG INFO", pproduct->mfg_name, 
		IPMI_SPROM_SD_PRODUCT_MFG_NAME_SIZE);

	FILL_TL_STR("PRODUCT NAME", pproduct->product_name, 
		IPMI_SPROM_SD_PRODUCT_NAME_SIZE);

	FILL_TL_STR("PROD PART NUM", pproduct->part_model, 
			IPMI_SPROM_SD_PRODUCT_PART_MODEL_SIZE);

	FILL_TL_STR("PROD SERIAL NUM", pproduct->serial_num, 
			IPMI_SPROM_SD_PRODUCT_SERIAL_NUMBER_SIZE);

	FILL_TL_STR("PROD PART NUM REV", pproduct->prd_version, 
			IPMI_SPROM_SD_PRODUCT_VERSION_SIZE);

	FILL_STR("PROD PID REV", pproduct->pid_rev,3); 

	ipmi_zero_checksum_create((uint8_t*)pproduct, sizeof(*pproduct));
	return (0);
}

void ipmi_iu_mr_hdr_create(ipmi_sprom_multi_record_header_t *phdr, uint8_t rec_version,
			uint8_t rec_type, uint8_t rec_len, uint32_t rec_checksum) 
{
	phdr->record_type = rec_type;
	phdr->version     = rec_version;
	phdr->length      = rec_len;
	phdr->record_checksum = rec_checksum;
	phdr->header_checksum = 0x00;
	ipmi_zero_checksum_create((uint8_t*)phdr, sizeof(*phdr));
}

int ipmi_iu_mr_psu_info_create(ipmi_sprom_mr_psu_info_t *pInfo)
{
	uint16_t data;
	uint8_t  data8;

	data = (uint32_t)pInfo->capacity;
	FILL_HEX("PROD PSU CAPACITY",  %04X, %hx, data);
	pInfo->capacity = (uint16_t) data;

	data = (uint32_t)pInfo->peak_val;
	FILL_HEX("PROD PSU PEAKVAL",  %04X, %hx, data);
	pInfo->peak_val = (uint16_t) data;

	data8 = (uint32_t)pInfo->lsb;
	FILL_HEX("PROD PSU LSB",  %02X, %hhx, data8);
	pInfo->lsb = (uint8_t) data8;

	data8 = (uint32_t)pInfo->inrush_amp;
	FILL_HEX("PROD PSU INRUSH AMP",  %02X, %hhx, data8);
	pInfo->inrush_amp = (uint8_t) data8;

	data8 = (uint32_t)pInfo->inrush_interval;
	FILL_HEX("PROD PSU INRUSH INTERVAL",  %02X, %hhx, data8);
	pInfo->inrush_interval = (uint8_t) data8;

	data = (uint32_t)pInfo->lo_end_inp_v1;
	FILL_HEX("PROD PSU LO INPUT V1",  %04X, %hx, data);
	pInfo->lo_end_inp_v1 = (uint16_t) data;

	data = (uint32_t)pInfo->hi_end_inp_v1;
	FILL_HEX("PROD PSU HI INPUT V1",  %04X, %hx, data);
	pInfo->hi_end_inp_v1 = (uint16_t) data;

	data = (uint32_t)pInfo->lo_end_inp_v2;
	FILL_HEX("PROD PSU LO INPUT V2",  %04X, %hx, data);
	pInfo->lo_end_inp_v2 = (uint16_t) data;

	data = (uint32_t)pInfo->hi_end_inp_v2;
	FILL_HEX("PROD PSU HI INPUT V2",  %04X, %hx, data);
	pInfo->hi_end_inp_v2 = (uint16_t) data;

	data8 = (uint32_t)pInfo->hi_end_inp_hz;
	FILL_HEX("PROD PSU HI INPUT HZ",  %02X, %hhx, data8);
	pInfo->hi_end_inp_hz = (uint8_t) data8;

	data8 = (uint32_t)pInfo->inp_drop_out_tol;
	FILL_HEX("PROD PSU HI INPUT TOL",  %02X, %hhx, data8);
	pInfo->inp_drop_out_tol = (uint8_t) data8;

	data8 = (uint32_t)pInfo->flags;
	FILL_HEX("PROD PSU FLAGS",  %02X, %hhx, data8);
	pInfo->flags = (uint8_t) data8;

	data8 = (uint32_t)pInfo->hold_up_time;
	FILL_HEX("PROD PSU HOLD UP TIME",  %02X, %hhx, data8);
	pInfo->hold_up_time = (uint8_t) data8;

	data = (uint32_t)pInfo->peak_capacity;
	FILL_HEX("PROD PSU PEAK CAPACITY",  %04X, %hx, data);
	pInfo->peak_capacity = (uint16_t) data;

	data = (uint32_t)pInfo->total_watts;
	FILL_HEX("PROD PSU TOTAL WATTS",  %04X, %hx, data);
	pInfo->total_watts = (uint16_t) data;

	data8 = (uint32_t)pInfo->fail_tacho_thres;
	FILL_HEX("PROD PSU TACHO THRES",  %04X, %hhx, data8);
	pInfo->fail_tacho_thres = (uint8_t) data8;

	return (0);
}

int ipmi_iu_mr_dc_out_create(ipmi_sprom_mr_dc_out_t *pDcOut)
{
	uint16_t data;
	uint8_t  data8;

	data8 = (uint32_t)pDcOut->info;
	FILL_HEX("PROD DC INFO",  %02X, %hhx, data8);
	pDcOut->info = (uint8_t) data8;

	data = (uint32_t)pDcOut->nominal_volt;
	FILL_HEX("PROD DC NOMINAL VOLT",  %04X, %hx, data);
	pDcOut->nominal_volt = (uint16_t) data;

	data = (uint32_t)pDcOut->max_neg_volt;
	FILL_HEX("PROD DC MAX NEG VOLT",  %04X, %hx, data);
	pDcOut->max_neg_volt = (uint16_t) data;

	data = (uint32_t)pDcOut->max_pos_volt;
	FILL_HEX("PROD DC MAX POS VOLT",  %04X, %hx, data);
	pDcOut->max_pos_volt = (uint16_t) data;

	data = (uint32_t)pDcOut->ripple_noise;
	FILL_HEX("PROD DC RIPPLE NOISE",  %04X, %hx, data);
	pDcOut->ripple_noise = (uint16_t) data;

	data = (uint32_t)pDcOut->min_amps;
	FILL_HEX("PROD DC MIN AMPS",  %04X, %hx, data);
	pDcOut->min_amps = (uint16_t) data;

	data = (uint32_t)pDcOut->max_amps;
	FILL_HEX("PROD DC MAX AMPS",  %04X, %hx, data);
	pDcOut->max_amps = (uint16_t) data;

	return (0);
}

int ipmi_iu_mr_cisco_mr_create(ipmi_sprom_mr_mp_cisco_card_t *pCiscoMr)
{
	uint16_t data16;
	uint32_t data32;

	FILL_STR("PROD ID", pCiscoMr->id,5); 

	data32 = (uint32_t)pCiscoMr->format_version;
	FILL_HEX("PROD FORMAT VERSION",  %08X, %X, data32);
	pCiscoMr->format_version = data32;

	data16 = (uint32_t)pCiscoMr->power;
	FILL_HEX("PROD POWER",  %04X, %hx, data16);
	pCiscoMr->power = data16;

	return (0);
}

int ipmi_sprom_sd_mr_create_old (ipmi_psu_sd_mr_t *pmr,
					uint8_t card_type)
{
	uint8_t cs = 0;

	ipmi_iu_mr_psu_info_create(&pmr->psu_info);
	cs = ipmi_zero_checksum_get ((uint8_t*)&pmr->psu_info, 
			sizeof(ipmi_sprom_mr_psu_info_t));
	ipmi_iu_mr_hdr_create(&pmr->psu_info_hdr, 2,
		IPMI_SPROM_MULTI_RECORD_POWER_SUPPLY, 
		sizeof(ipmi_sprom_mr_psu_info_t), cs);

	ipmi_iu_mr_dc_out_create(&pmr->dc_out_1);
	cs = ipmi_zero_checksum_get ((uint8_t*)&pmr->dc_out_1, 
			sizeof(ipmi_sprom_mr_dc_out_t));
	ipmi_iu_mr_hdr_create(&pmr->dc_out_1_hdr, 2,
		IPMI_SPROM_MULTI_RECORD_DC_OUTPUT, 
		sizeof(ipmi_sprom_mr_dc_out_t), cs);

	ipmi_iu_mr_dc_out_create(&pmr->dc_out_2);
	cs = ipmi_zero_checksum_get ((uint8_t*)&pmr->dc_out_2, 
			sizeof(ipmi_sprom_mr_dc_out_t));
	ipmi_iu_mr_hdr_create(&pmr->dc_out_2_hdr, 0x02,
		IPMI_SPROM_MULTI_RECORD_DC_OUTPUT, 
		sizeof(ipmi_sprom_mr_dc_out_t), cs);

	ipmi_iu_mr_cisco_mr_create(&pmr->cisco_mr);
	cs = ipmi_zero_checksum_get ((uint8_t*)&pmr->cisco_mr, 
			sizeof(ipmi_sprom_mr_mp_cisco_card_t));
	ipmi_iu_mr_hdr_create(&pmr->cisco_mr_hdr, 0x82,
		IPMI_SPROM_MULTI_RECORD_OEM_NCSI, 
		sizeof(ipmi_sprom_mr_mp_cisco_card_t), cs);

	return (0);
}

int ipmi_sprom_sd_mr_create(ipmi_sprom_psu_mr_t *pmr, uint8_t card_type)
{
	uint8_t cs = 0;

	cs = ipmi_zero_checksum_get ((uint8_t*)&pmr->oem[0], 11);
	ipmi_iu_mr_hdr_create(&pmr->psu_info_hdr, 0x82,
		IPMI_SPROM_MULTI_RECORD_OEM_NCSI, 11, cs);
	return (0);
}

int ipmi_sprom_sd_psu_fill(sprom_ipmi_sd_psu_t *sprom)
{
	printf("  Create SD PSU Common Header\n");
	ipmi_sprom_common_header_create(&sprom->common_header, 
	M8_SIZE(sizeof(ipmi_sprom_psu_internal_use_t)), 0, 0,
	M8_SIZE(sizeof(ipmi_sprom_psu_product_info_t)), 
	M8_SIZE(sizeof(ipmi_sprom_psu_mr_t)));	

	printf("  Create Internal Use\n");
	ipmi_sprom_internal_use_create (&sprom->internal_use,
					IPMI_IU_CARD_TYPE_SC_PSU);

	printf("  Create Product Info\n");
	ipmi_sprom_sd_product_info_create (&sprom->product_info);

	printf("  Create Multi Record\n");
	ipmi_sprom_sd_mr_create (&sprom->multi_record,
					IPMI_IU_CARD_TYPE_SC_PSU);

	return (0);
}
