/* $Id: smart_cookie_auth.h,v 1.2 2012/03/28 00:38:12 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/smart_cookie_auth.h,v $
 *------------------------------------------------------------------
 * Definitions imported during port of IOS authentication code.
 *
 * October 2003, David Turner
 *
 * Copyright (c) 2009-2012 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

 /* This file contains the header for smart_cookie.c. It was ported 
  * from IOS.
  */ 

#define PAGE_SIZE                                 64
#define CONT_ID_TLV_SIZE                           3

#define AUTH_MAX_ERR                            64
#define AUTH_MAX_LOOP_RETRY                      1
#define AUTH_MAX_LOOP_RETEST                    10
#define AUTH_MAX_RETRY                           1

#define	T_IDPROM_HW_TYPE	0x40	/* HW (PA) type			*/
#define	IDPROM_VERSION_OFFSET	   0
#define	IDPROM_TLV_OFFSET	   2
#define	TLV_IDPROM_VERSION	0x04
#define	IDPROM_VERSION_0           0
#define	IDPROM_VERSION             1
#define	IDPROM_VERSION_2           2

#define	T_IDPROM_TYPE_MASK	0xc0	/* mask to get basic field type	*/

#define	T_IDPROM_BYTE		0x00	/* data is single byte		*/
#define	T_IDPROM_SHORT		0x40	/* data is two bytes		*/
#define	T_IDPROM_LONG		0x80	/* data is four bytes		*/
#define	T_IDPROM_VAR		0xc0	/* variable length field	*/
#define	T_IDPROM_VAR2		0xf0	/* variable length field with   */

#define	T_IDPROM_EXTENSION	0x00	/* extension type marker	*/
#define	T_IDPROM_EOD		0xff	/* end-of-data marker		*/

#define	T_IDPROM_EXT_OFFSET	0x100	/* offset to add to type field	*/
					/* for extended types		*/

/*
 * display basic size field definition
 * Type values are used to define the raw display type.
 * Type values are stored in length field bit 6&7.
 */
#define	S_IDPROM_SIZE_MASK	0x3f
#define	S_IDPROM_SIZE_MASK2	0x3fff
#define	S_IDPROM_TYPE_MASK	0xc0
#define	S_IDPROM_TYPE_MASK2	0xc000
#define	S_IDPROM_TYPE_HEX	0x00
#define	S_IDPROM_TYPE_DECIMAL	0x40
#define	S_IDPROM_TYPE_ASCII	0x80
#define	S_IDPROM_TYPE_RESERVED	0xc0

#define BSAFE_CONST             0x04

/*
 * shortcuts
 */
#define	tlv_byte	tlv_u.t_byte
#define	tlv_short	tlv_u.t_short
#define	tlv_int	        tlv_u.t_int
#define	tlv_length	tlv_u.t_dyn.t_dyn_1.t_length
#define	tlv_data	tlv_u.t_dyn.t_dyn_1.t_data
#define	tlv_length2 	tlv_u.t_dyn.t_dyn_2.t_length
#define	tlv_data2   	tlv_u.t_dyn.t_dyn_2.t_data

/* 
 * Smart chip specific information
 */
typedef struct scc_info_ {
    uchar type;
    uchar version[2];
    uchar post_result;
} scc_info_t;


#define PACKED(item) item

/*
 * definition of IDPROM TLV field.
 */
typedef struct {
    unsigned char PACKED(tlv_type);	/* object type		*/
    union {
	uchar PACKED(t_byte);	/* byte size fields	*/
	ushort PACKED(t_short);	/* short size fields	*/
	uint PACKED(t_int);	/* long size fields	*/
	union {
	    struct { /* 1-byte length dynamic fields    */
		uchar PACKED(t_length);/* object length	*/
		uchar PACKED(t_data[0]);/* object data	*/
	    } PACKED(t_dyn_1);
	    struct { /* 2-byte length dynamic fields	*/
		ushort PACKED(t_length);/* object length*/
		uchar PACKED(t_data[0]);/* object data	*/
	    } PACKED(t_dyn_2);
	} PACKED(t_dyn);

    } tlv_u;
} __attribute__ ((packed)) idprom_tlv_t;

extern void *idprom_get_entry(uchar *, uchar *, int *, int *, int *);
extern tlv_id_t *get_tlv_from_cookie(uchar *, int, int);
extern type_t smart_cookie_authenticate(sc_context *con);
extern void random_fill(uchar *random_number, int size, int flag);

/******** History ******** 
$Log: smart_cookie_auth.h,v $
Revision 1.2  2012/03/28 00:38:12  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:04  ptong
Initial archive of ng_diag module


$Endlog$
*/
