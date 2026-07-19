/* $Id: cookie_4_core.c,v 1.23 2020/08/19 09:49:17 markzha Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/cookie_4_core.c,v $
 *------------------------------------------------------------------
 * cookie_4_core.c
 *
 * Jan. 2007, 
 *
 * Copyright (c) 2008-2019 by cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

#include <stdlib.h>
#include "common.h"
#include "types.h"
#include "defs.h"
#include "proto.h"
#include "setjmps.h"
#include "monitor.h"
#include "nvmonvars.h"
#include "mon_plat_defs.h"
#include "error.h"
#include "queryflags.h"
#include "nmc93c46.h"
#include "pcmap.h"
#include "cross_platform.h"
#include "cookie_plat.h"
#ifdef AHSU
#include "cpu.h" 
#include "getnum.h" 
#endif
#include "menu.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "cli_cmd.h"
#include "cookie_4.h" 
#include "smart_cookie.h"
#include "goofy_i2c.h"
#include "dash_fpga.h"
#include "slot.h"
#include "ngio.h"
#include "linux_api.h"


static char *n_hex = "n * 'HH' (ie, '12 34 56')";
static char *n_dec = "n * 'DD'";
static char *n_alp = "n * 'AA'";
#define MAC_ADDR_BYTES 6

#define MAX_U8_VAL         0xFF
static uchar mac_address[MAC_ADDR_BYTES]  = {0};
static uint32_t mac_blk_size = 1; 



extern int cookie_4_processor_x (uchar *contents, int board_type,
                               int cookie_type, int cookie_size, 
                               cli_cookie_cmd *);
extern void write_sys_eeprom_x (pas_management_t *,
                       struct cookie_plat *, int );
extern void read_sys_eeprom_x (pas_management_t *,
                       struct cookie_plat *, int );
extern void init_cookie_4_default_x (int , int ,
                         uchar *, int );
extern void swap_eeprom_x(uchar *, uchar * , int);
extern boolean pas_pa_present (uint slot);
extern void alt_pas_eeprom(void);
extern int slot_start_with(void);


controller_type_t wic_controller_type_info[] = {
    {0x071E,     "HWIC-1DSU-56K"},        /* dsu-56k, next gen */
    {0xffff,     "Reserved"},
    {0x0,        (char *)NULL}
};

controller_type_t vic_controller_type_info[] = {
    {0x04f7,     "HWIC-CABLE-E/J"},   /* Snowshoe 1-port cable modem E/J */
    {0xffff,     "Reserved"},
    {0x0,        (char *)NULL},
};

controller_type_t daughtercard_controller_type_info[] = {
    {0x0567,     "HWIC-ADSLM-DC"},    /* Borghetti Annex M daughter card */
    {0xffff,     "Reserved"},
    {0x0,        (char *)NULL},
};
 
int daughtercard_tbl_size = sizeof(daughtercard_controller_type_info)/
sizeof(controller_type_t);

controller_type_t simm_controller_type_info[] = {
    {0x05dd,     "PVDM3-192"},
    {0x05de,     "PVDM3-256"},
    {0xffff,     "Reserved"},
    {0x0,        (char *)NULL},
};
 
int simm_tbl_size = sizeof(simm_controller_type_info)/
sizeof(controller_type_t);


controller_type_t controller_type_info[] = {
    {0x06DB,    "SM-PA Adapter"}, 
    {0xffff,	"Reserved"},
    {0x0,	(char *)NULL},
};


cookie_4_table cookie_4_info[] = {
    {"Number of Slots         : ", "NUMOFSLOTS",      1,   0x01,  0,    "DDD", 0}, 
    {"Fab Version             : ", "FABREV",          1,   0x02,  0,    "DDD", 0},
    {"RMA Test History        : ", "RMATST",          1,   0x03,  0,    "HH",  0},
    {"RMA History             : ", "RMAHIS",          1,   0x04,  0,    "HH",  0},
    {"Connector Type          : ", "CONTYP",          1,   0x05,  0,    "DDD", 0},
    {"EHSA Preferred Master   : ", "EHSA"  ,          1,   0x06,  0,    "HH",  0},
    {"Vendor ID               : ", "VENDID",          1,   0x07,  0,    "HH",  0},
    {"Processor Type          : ", "PROTYP",          1,   0x09,  0,    "HH",  0},
    {"TDM                     : ", "TDM",             1,   0x0a,  0,    "HH",  0},
    {"Power Supply Type       : ", "PWRSPLYTYP",      1,   0x0b,  0,    "AA",  0},
    {"Compatibility byte      : ", "COMPABYTE",       1,   0x3e,  0,    "HH",  0},
    {"EEPROM format version   : ", "EEPROMFORMATVER", 1,   0x3f,  0,    "HH",  0},
    {"Controller Type         : ", "CNTL",            2,   0x40,  0,    "HH HH",   0},
    {"Hardware Revision       : ", "HWREV",           2,   0x41,  0,    "DDD.DDD", 0},
    {"PCB Revision            : ", "PCBREV",          2,   0x42,  0,    "AA",      0},
    {"MAC Address block size  : ", "MACADDRBLKSIZ",   2,   0x43,  0,    "DDDDD",   0},
    {"Capability Code         : ", "CAPCODEL",        2,   0x44,  0,    "HHHH",    0},
    {"Self Test Result        : ", "SLFTSTRES",       2,   0x45,  0,    "HH HH",   0},
    {"Boot Time Out           : ", "BOOTTIMEOUT",     2,   0x46,  0,    "DDDDD",   0},
    {"Motherboard Channel ID  : ", "MBCHID",          2,   0x47,  0,    "HH HH",   0},
    {"Group Type              : ", "GROUPTYP",        2,   0x48,  0,    "HH HH",   0},
    {"CLI Write Enable        : ", "CLIWRTEN",        2,   0x49,  0,    "HH HH",   0},
    {"Radio Country Code      : ", "RADIOCOUNTRYCODE",2,   0x4a,  0,    "HH HH",   0},
    {"Deviation Number        : ", "DEVINUM",         4,   0x80,  0,    "DDDDD-DDDDD",    0},
    {"RMA Number              : ", "RMANUM",          4,   0x81,  0,    "DDD-DDD-DDD-DDD",0},
    {"Part Number      (73)   : ", "PARTNUM",         4,   0x82,  0,    "DD-DDDDD-DD",    0},
    {"Hardware Date Code      : ", "HWDATECODE",      4,   0x83,  0,    "DDDDDDDD",       0},
    {"Manufacturing Engineer  : ", "MFGENGINEER",     4,   0x84,  0,    "HH HH HH HH",    0},
    {"Fab Part Number  (28)   : ", "28NUM",           4,   0x85,  0,    "28-DDDDD-DD",    0},
    {"Calibration data        : ", "CALIBDATA",       4,   0x86,  0,    "HH HH HH HH",    0},
    {"Top Assy. Part Num (68) : ", "TOPASSYPRTNUM",   4,   0x87,  0,    "DD-DDDDD-DD",    0},
    {"New Deviation Number    : ", "NEWDEVNO",        4,   0x88,  0,    "10 * 'D'",       0},
    {"Version Identifier (VID): ", "VID",             4,   0x89,  0,    "AAAA",           0},
    {"PCB Revision            : ", "PCBREV2",         4,   0x8A,  0,    "AAAA",           0},
    {"TAN Revision            : ", "TANREV",          4,   0x8D,  0,    "AAAA",           0},
    {"Part Number             : ", "C0TYPNUM",        6,   0xc0,  0x46, "DDD-DDDDDD-DD",  0},
    {"PCB Serial Number       : ", "PCBSER",         11,   0xc1,  0x8b, "AAAAAAAAAAA",    0},
    {"Chassis Serial Number   : ", "CHASSN",         11,   0xc2,  0x8b, "AAAAAAAAAAA",    0},
    {"Chassis MAC Address     : ", "CHMAC",           6,   0xc3,  0x06, "HHHH.HHHH.HHHH", 0},
    {"Manufacturing Test Data : ", "MFGTSTDAT",       8,   0xc4,  0x08, "n * 'HH '",      0},
    {"Field Diagnostics Data  : ", "FDIAGDATA",       8,   0xc5,  0x08, "n * 'HH '",      0},
    {"CLEI Code               : ", "CLEI",           10,   0xc6,  0x8a, "AAAAAAAAAA",     0},
    {"ENVMON information      : ", "ENVMON",        128,   0xc7,  0xa0, "n * 'HH '",      DLEN|DFMT},  /* bug fix based on CSCvo89304-2 */
    {"IBR-MC Calibration Data : ", "IBR-MC",          9,   0xc8,  0x09, "n * 'HH '",      0},
    {"Device Values           : ", "DEVVALUES",     128,   0xc9,  0xbf, "n * 'HH '",      DLEN|DFMT},
    {"HardDisk(Cyl-Trk-Blk-Hd): ", "HDINFO",          8,   0xca,  0x48, "DDDD-DDDD-DDDD-DDDD", 0},
    {"Product Num/Id (PID)    : ", "PRODID",        128,   0xcb,  0xbf, "n * 'A'",        DLEN},
    {"Asset ID                : ", "ASSETID",       128,   0xcc,  0xa0, "n * 'A'",        DLEN},
    {"IBR-MC Calibration Data : ", "IBR-MC2",         9,   0xcd,  0x09, "n * 'HH '",      DLEN},
    {"Motherboard PCB Serial #: ", "PCBSERNUM",      11,   0xce,  0x8b, "11 * 'A'",       0},
    {"Base MAC Address        : ", "BAMAC",           6,   0xcf,  0x06, "HHHH.HHHH.HHHH", 0},
    {"Card Name (legacy only) : ", "CARDNAME",      128, 0xd0,  0xbf, "n * 'HH '",      DLEN|DFMT},
    {"File Name (legacy only) : ", "FILENAME",      128, 0xd1,  0xbf, "n * 'HH '",      DLEN|DFMT},
    {"Encryption Digest       : ", "ENCRPYT",       128, 0xd2,  0xbf, "n * 'HH '",      DLEN|DFMT},
    {"Longitudinal Calibration: ", "LONGICALIB",    128, 0xd3,  0xbf, "n * 'HH '",      DLEN|DFMT},
    {"Asset Alias             : ", "ASSETALIAS",    128, 0xd4,  0xa0, "n * 'HH '",      DLEN|DFMT},
    {"Processor Label         : ", "PROLEVEL",      128, 0xd5,  0xbf, "n * 'AA'",       DLEN|DFMT},
    {"System Clock Frequency  : ", "SYSCLKFREQ",      4, 0xd6,  0x84, "HH HH HH HH",   0},
    {"Digital Signature List  : ", "DIGSIG",          0,   0xd9,  0x3f, "n * 'HH '",      DLEN},
    {"Part Number     (341)   : ", "341",	      5, 0xdf,  0x45, "341-DDDD-DD",   0},
    {"Part Number      (73)   : ", "PARTNUM",         6, 0xe2,  0x46,  "DDD-DDDDDD-DD",   0},
    {"Fab Part Number  (28)   : ", "28NUM",           6, 0xe3,  0x46,  "DDD-DDDDDD-DD",   0},
    {"RFID - PCA              : ", "RFIDPCA",       128, 0xe5,  0xbf, "n * 'HH '",      DLEN|DFMT},
    {"MC520 Upstream Cal Data : ", "MC520",         128, 0xf0,  0xbf, "n * 'HH '",      DLEN|DFMT},
    {"Laser information       : ", "LASERINFO",     128, 0xf2,  0xbf, "n * 'HH '",      DLEN|DFMT},
    {"ENVMON Extended info    : ", "ENVMONEXTINFO", 128, 0xf3,  0xbf, "n * 'HH '",      DLEN|DFMT},
    {NULL,                NULL,      0,   0,     0,    "",               0}
};

cookie_4_table  *search_type_4_table (uchar);
COOKIE_4 	*buffer_search (int);
COOKIE_4 	*buffer_search_x (boolean, int);
COOKIE_4 	*search_type_4_buf_pool (uchar);
void            show_cookie_4 (int);
void            show_cookie_4_x(boolean, char *, int);
void            change_cookie_4 (int);
void            add_cookie_4 (void);
void            move_cookie_4 (void);
void            remove_cookie_4 (void);
void            update_cookie_4 (int, int, int, uchar *);
void 		read_cookie_content_x (uchar *, int, int, int, int);
int 		read_cookie_4_content (uchar *, int, int, int, int);
uchar 		*search_type_ret_addr_of_first_data (uchar *, uchar,
						     uchar *, int);
int		init_add_mb_cookie_content (void);
void            dump_old_cookie_x (uchar *, int);
int             cookie_sanity_check (int, int, int);
int             check_cookie_valid_x (int , int, int, int);
void            put_cookie_4_x (cookie_plat_t *, int , int, int);
pas_management_t  *get_pas_cookie_4_ptr (int, int);
void     	get_cookie_4_plat_x (int, int, int *, uchar *, int );
int             update_selected_tlv_pack (uchar *, uchar , uint ,
                                          uchar *, int , int );
void            update_cookie_4_x (int condition_flag, int board_type,
                             int reserved, uchar *original_cookie_ptr,
                             int cookie_size);
int		toss_cookie_4_x (int , int , int , cli_cookie_cmd *);

int             get_dec_user_input (char *s);
int             process_raw_cookie_all_x (uchar *cookie_contents, int cookie_size);



static int      display_char (uchar);
static void     get_dec (unsigned long *);
static void     prompt_for_display_format(COOKIE_4 *buf);


int             field_addr_idx;		/* index to the next avalable buffer */
int            last_selected_index;    /* counter for the last buffer */
COOKIE_4        field_addr[sizeof(cookie_4_info) / sizeof(cookie_4_table)];
COOKIE_4        *cookie_root;	       /* addr to the buffer link list entry */
uchar   	tmp_buf[256], *p_tmp;  /* temp storage array and its pointer */

extern pas_management_t *pas_management_list[];
extern pas_management_t *pas_cookie_ptr;

/**************************************
 function: set_mac_blk_size
   description: store size of mac block
   the routine is called when user displays or modifies 0x43
   input: size  - size of mac blk
   outout: none

****************************************/
static void
set_mac_blk_size (uint32_t size)
{
    mac_blk_size = size;
}

/**************************************
function get_mac_blk_size
descrition: returns the size of mac block
input: non
output: size of mac blk
**************************************/
uint32_t
get_mac_blk_size ()
{
    return mac_blk_size;
}

/**************************************
function set_chassic_mac
descrition: store the mac address into static variable.
the routine is called when user displays or modifies 0xC3
filed
input: pointer to mac address to be saved/stored
output: none
**************************************/
void
set_chassis_mac (char *buf)
{
    /*
    printf("\n\t\t\t %#x:%#x:%#x:%#x:%#x:#x\n",
           buf, *(buf+1), *(buf+2), *(buf+3), *(buf+4), *(buf+5));
    */
    memcpy(mac_address, buf, MAC_ADDR_BYTES);
}

/*
   function: incr_ip_mac_addr :
   description: helper function to inrement a mac address.
   Two indexes are maintained: one is called increment index which is
   incremented each time to get the next mac. There is also a parent
   index which is incremented once the increment index reaches maximum
   value in a byte i.e 0xFF. The increment index represents the LSB and
   the parent index represents the MSB adjacent to LSB.
   input: paddr   -- current mac address
          i       -- index (see description
          p       -- parent index (see description)
          max_val -- max value before increasing parent index (usually 0xFF)
 */
static void
incr_mac_addr(unsigned char *paddr, unsigned int *i,
                             unsigned int *p, unsigned int max_val)
{
    unsigned int incr_index, parent_index ;
    incr_index = *i ;
    parent_index = *p ;
    if ( paddr[incr_index] == max_val ){
        if ( paddr[parent_index] == max_val) {
            paddr[parent_index] = 0 ;
            parent_index--;
        }
        paddr[parent_index] +=1 ;
        paddr[incr_index] = 0 ;
    } else{
        paddr[incr_index]++ ;
    }
    *i = incr_index ;
    *p = parent_index ;
}

/*
   get_mac_address_from_block. the block mac assigned starts at
   chasis mac address (0xc3) and ends at mac blks size (0x43).
   for example, if we want to get the last mac address from block, we pass in
   offset value of mac_blk_size - 1, and new_mac will return the last
   mac address
   input  - offset from chasis mac address
   output - new_mac : the new mac address

*/
int
get_mac_from_block (uint32_t offset, uchar *new_mac)
{
    unsigned int i;
    unsigned int mac_incr_idx = MAC_ADDR_BYTES - 1, mac_parent_idx = MAC_ADDR_BYTES - 2 ;
    unsigned char mac_addr[MAC_ADDR_BYTES];
 
    memcpy( &mac_addr[0], &mac_address, sizeof(unsigned char)*MAC_ADDR_BYTES) ;
    for(i=0; i<offset; i++) {
        incr_mac_addr( mac_addr, &mac_incr_idx, &mac_parent_idx, MAX_U8_VAL) ;
    }
    /*
    printf("new mac %#x %#x %#x %#x %#x %#x\n",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3],
           mac_addr[4], mac_addr[5], mac_addr[6]);
    */
    memcpy(new_mac, mac_addr, sizeof(unsigned char)*MAC_ADDR_BYTES);
    return 0;
}


/* 
   verify_length_field()

   Check to see if the length field comforms to the spec.
   The length field may contain information for display format.
   
   Input: cp        : cookie_pointer in the cookie_4_info table
          contents  : pointer to most significant byte of the length field
   Output: PASSED, if length field looks valid; FAILED otherwise.
 */
static int verify_length_field(cookie_4_table *cp, uchar *contents)
{
    int retval = PASSED;

    /* check to see if length field contains information on display format */
    if (cp->variable_type & DFMT) {
	switch (*contents & TYPE_SIZE_MASK) {
	case HEX_FORMAT:
	case DEC_FORMAT:
	case ALP_FORMAT:
	    cp->val_length &= ~TYPE_SIZE_MASK;
	    cp->val_length |= *contents & TYPE_SIZE_MASK;
	    break;
	default:
	    retval = FAILED;
	}
    } else {
	if ((*contents > cp->len_fd) || (*contents <= cp->val_length)) 
	    retval = FAILED;

    }
    if (retval) {
	printf("Warning: length field does not follow the spec!!\n");
	printf("type=%#x, len_fd=%#x, val_length=%#x, content=%#x\n",
	       cp->type, cp->len_fd, cp->val_length, *contents);
	return FAILED;
    }
    return PASSED;
}


/*
  prompt_for_display_format

  Ask user for display format. This routine works only with variable len
  data.

  Input: buf   : pointer to COOKIE_4
  Output: None.
 */

static void prompt_for_display_format(COOKIE_4 *buf)
{

    int max_len, display_format;

    if (!(buf->p_info->variable_type & DFMT))
	return ;
    printf("\nYou will need to specify the display format.\n");
    printf("Please enter '%d' to select Alphanumeric format, or\n", 0);
    printf("       enter '%d' to select Decimal format, or\n", 1);
    printf("       enter '%d' to select Hex format.\n", 2);
    display_format =  getdec_answer("Please enter your selection: ", 0, 0, 2);

    max_len = buf->p_info->len_fd - buf->p_info->val_length;
    switch (display_format) {
    case 0:
	buf->p_info->val_length = ALP_FORMAT;
        buf->p_info->input_form = n_alp;
        //	process_input_form(buf->p_info->input_form, 'A');
        break;
    case 1:
	buf->p_info->val_length = DEC_FORMAT;
        buf->p_info->input_form = n_dec;
        //	process_input_form(buf->p_info->input_form, 'D');
        break;
    case 2:
	buf->p_info->val_length = HEX_FORMAT;
        buf->p_info->input_form = n_hex;
        //	process_input_form(buf->p_info->input_form, 'H');
	break;
    }

    buf->p_info->len_fd = buf->p_info->val_length + max_len;

#ifdef DEBUG
    printf("val_length is %#x ; len_fd is %#x\n",
	   buf->p_info->val_length,
	   buf->p_info->len_fd);
#endif
    return ;
}


/* 
   cookie_4_enque ():

   If cookie_root is NULL then forward and backward pointer point to itself.
   else put the new buffer at forward side of the post buffer

   Input: pb: post buffer, the base of current operation 
   nb: new buffer, the buffer will be added to the link list
   Output: None.
 */
void
cookie_4_enque (COOKIE_4 *pb, COOKIE_4 *nb)
{

    if (cookie_root == (COOKIE_4 *)NULL) {
        nb->b = nb->f = cookie_root = nb;
    }
    else {
	nb->b = pb;
	nb->f = pb->f;
	pb->f->b = nb;
	pb->f = nb;
    }
}

/*
   cookie_4_deque ():
   grep address of backward buffer to the backward pointer of forward side of 
   buffer. so is next one except in another direction.

   Input:  dl: delete buffer, remove the buffer from link list.
   Output: None.
 */
void
cookie_4_deque (COOKIE_4 *dl)
{
    dl->f->b = dl->b;
    dl->b->f = dl->f;
}


/*
  get_new_buf ():
  get a new buffer from data section and init it.
  The backward side of the cookie_root buffer contains used spare buffer.
  Get recycle buffer first before using other fresh buffers.

  Input:  None.
  Output: a clean buffer.
  */
COOKIE_4 *
get_new_buf (void)
{
    COOKIE_4 *buf;

    /* prevent to access undefined pointer */
    if ((cookie_root != (COOKIE_4 *)NULL) 
	&& (cookie_root->b != (COOKIE_4 *)NULL)
	&& (cookie_root->b->spare_flag == SPARE)) {
        buf = cookie_root->b;
        cookie_4_deque(buf);
    }
    else {
        if (field_addr_idx >= (int)
	    (sizeof(cookie_4_info) / sizeof(cookie_4_table))) {
            printf("Check any duplicate TYPE field in the cookie!!\n");
            return((COOKIE_4 *)NULL);
        }
        buf = &field_addr[field_addr_idx++];
    }

    /* clean the buffer */
    buf->f = buf->b = (COOKIE_4 *)NULL;
    buf->p_val_byte = (uchar *)NULL;
    buf->p_info = (cookie_4_table *)NULL;
    buf->type = buf->spare_flag = 0;
    buf->length = 0;
    return(buf);
}


/* 
   parsing_n_setup_q_x ():
   Parsing the cookie contents, and set up the link list according to 
   the "TYPE" field.

   Input:  contents, pointer for the cookie array.
           cookie_size
   Output: NEED_NEW_COOKIE_FORMAT,
   PARSING_N_SETUP_FAILED,
   PARSING_N_SETUP_PASSED.
 */
int
parsing_n_setup_q_x (uchar *contents, int cookie_size)
{
    int val1, val2;           /* temp storage for length used for 0xFx type */
    COOKIE_4 *buf;            /* working buffer */
    cookie_4_table *cp;       /* cookie_4_table pointer */
    uchar temp_type;          /* temp type saved field */
    uchar *cookie_begin_addr = contents;

    if (*contents != CURRENT_FORMAT_VERSION) {
        printf("\nNot a format 4 cookie; Format version = 0x%x!!\n", *contents);
        return(NEED_NEW_COOKIE_FORMAT);
    }

    field_addr_idx = 0;	/* point to the beginning of the buffer resource */
    cookie_root = (COOKIE_4 *)NULL;

    /* handle the first format_version byte */
    if ((buf = get_new_buf()) == (COOKIE_4 *)NULL)
	return(PARSING_N_SETUP_FAILED);
    buf->misc = *contents++;  /* unique temp storage there */
    buf->type = (uchar)FORMAT_VERSION_TYPE;
    if ((cp = search_type_4_table(buf->type)) == (cookie_4_table *)NULL)
        return(PARSING_N_SETUP_FAILED);
    buf->p_info = cp;         /* pointer in cookie_4_table */
    cookie_4_enque(cookie_root, buf);

    /* handle the second compatibility byte */
    if ((buf = get_new_buf()) == (COOKIE_4 *)NULL)
        return(PARSING_N_SETUP_FAILED);
    buf->misc = *contents++;  /* unique temp storage there */
    buf->type = (uchar)COMPATIBILITY_TYPE; 
    if ((cp = search_type_4_table(buf->type)) == (cookie_4_table *)NULL)
        return(PARSING_N_SETUP_FAILED);
    buf->p_info = cp;       /* pointer in cookie_4_table */
    cookie_4_enque(cookie_root, buf);

    /* search from 3rd byte till last "TYPE" byte */
    while (*contents != LAST_BYTE) {
	if (contents - cookie_begin_addr >= cookie_size)
	    return(PARSING_N_SETUP_PASSED);

        if (*contents == EXTENSION_BYTE) {
	    contents++;
	    continue;
	}

        if ((cp = search_type_4_table(*contents)) == (cookie_4_table *)NULL)
            return(NEED_NEW_COOKIE_FORMAT);

        /* get next chunk of memory */
        if ((buf = get_new_buf()) == (COOKIE_4 *)NULL)
            return(PARSING_N_SETUP_FAILED);
        buf->p_info = cp;       /* pointer in cookie_4_table */
        buf->type = temp_type = *contents;
        contents++;		/* skip to next */

        /* deal with no length field */
        if ((temp_type & TYPE_SIZE_MASK) != TYPE_LENGTH_FOLLOW) {
            /* deal with data */
            buf->p_val_byte = contents;
	    buf->length = buf->p_info->val_length;
        }
        /* deal length field with dynamic length */
        else if ((cp->variable_type & DLEN)) {
	    /* test to see if length field looks valid */
	    if (verify_length_field(cp, contents)) {
		return(NEED_NEW_COOKIE_FORMAT);
	    }
	    /* length field looks ok, so lets find out number of bytes */
	    if (cp->type >= F_TYPE) {
		/* handle the case where length field is 2 bytes */
		val1 = *contents++ & TYPE_LENGTH_MASK; /* disregard 2 MSBs. */
		val2 = *contents;
		buf->length = (val1 << 8 | val2);
	    } else {
		buf->length = (*contents & TYPE_LENGTH_MASK);
	    }
	    buf->p_val_byte = ++contents;
	    
	}
	/* deal with fix length field */
	else if (cp->len_fd != *contents ) {
            printf("Warning length field error, after TYPE 0x%2x\n"
                  "exp_val = 0x%2x, obs_val = 0x%2x\n", cp->type,
		   cp->len_fd, *contents );
	    return(NEED_NEW_COOKIE_FORMAT);
	}
	else {
	    /* skip len_fd to deal with data */
	    buf->p_val_byte = ++contents;
	    buf->length = cp->val_length;
        }

        contents += buf->length;
        /* point to next type now */

        cookie_4_enque(cookie_root, buf);
        /* pack up the current buffer, NEXT !!! */
    }

    return(PARSING_N_SETUP_PASSED);
}


static int
display_char (uchar c)
{
    if( (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z')
        || (c >= 'a' && c <= 'z') || (c == ' ') ||
	(c == '/')) {
        putchar (c);
        return(PASSED);
    }
    return(FAILED);
}


/*
  display_cookie_4_element ():
  display each individual element which associated with "TYPE", 
  and unique display format.

  note: by passing compiler restriction in the run time,
  the code looks awkward.

  Input:  index: counter for display.
  buf: current buffer.
  board_type.
  Output: None.
 */
void
display_cookie_4_element (uint index, COOKIE_4 *buf, int board_type, boolean mode,
                          char *target)
{
    unsigned int i, val, max;
    uint  temp_uint;
    uchar temp_uchar;
    volatile uchar *p_data;
    controller_type_t *ctrl_info;
    uchar format_type;

    /* If in CLI mode */
    if(mode == CLI_MODE) {
    	  printf("\n%s%s:", target, buf->p_info->p_sn);
    }else { /* If in Menu mode */
        printf("\n%2d %s", index, buf->p_info->p_fs);
    }
    p_data = buf->p_val_byte;

    switch(buf->p_info->type) {
    case 0x40:
	val = *p_data++;
	val = val << 8 | *p_data;

	switch (board_type) {
	case VIC_MODULE:
	    ctrl_info = vic_controller_type_info;
	    break;
	case WIC_MODULE:
	    ctrl_info = wic_controller_type_info;
	    break;
	case DAUGHTER_CARD:
	    ctrl_info = daughtercard_controller_type_info;
	    break;
	case SPMM_MODULE:
	case VM_MODULE:
	    ctrl_info = simm_controller_type_info;
	    break;
	default:
	    ctrl_info = controller_type_info;
	    break;
	}
	while (ctrl_info->ctrl_name != (char *)NULL) {
	    if (ctrl_info->ctrl_type == (uint32_t)val)
		break;
	    ctrl_info++;
	}

	/*
	 * for mother board, this value is random due to
	 * ROMMON needs to calculate password in priv mode
	 * so can not print out any warning message, just
	 * print out the value
	 */
	if (ctrl_info->ctrl_name == (char *)NULL) {
	    printf("0x%04x", val);
	    return;
	}
	printf("%s", ctrl_info->ctrl_name);
	break;
    case 0x41:
	printf("%d.", *p_data++);
	printf("%d",  *p_data);
	break;
    case 0x43:	/* MAC Address Block Size */
    case 0x46:
    case 0x83:	/* Hardware Date Code */
	temp_uint = 0;
	val = buf->p_info->val_length;
	for (i = 0; i < val; i++)
	    temp_uint = temp_uint * 256 + *p_data++;

        if (buf->p_info->type == 0x43) {
            set_mac_blk_size(temp_uint);
        }
        printf("%d", temp_uint);
	break;
    case 0x88:
	temp_uint = 0;
	val = buf->p_info->val_length;
	for (i = 0; i < val; i++)
	    temp_uint = temp_uint * 256 + *p_data++;
	printf("%u", temp_uint);
	break;
    case 0x01:
    case 0x02:
    case 0x05:
    case 0x81:
	val = buf->p_info->val_length;
	for (i = 0; i < val; i++) {
	    printf("%d", *p_data++);
	    if ((i + 1) != val)
		printf("-");
	}
	break;
    case 0x03:
    case 0x04:
    case 0x06:
    case 0x07:
    case 0x09:
    case 0x0a:
    case 0x44:
    case 0x45:
    case 0x47: 
    case 0x48: 
    case 0x49:
    case 0x4a:	/* Radio Country Code */
    case 0x84:
    case 0x86:
    case 0xc4:
    case 0xc5:
	val = buf->p_info->val_length;
	for (i = 0; i < val; i++)
	    printf("%02x ", *p_data++);
	break;
    case 0xd6:
        val = *p_data++ << 8;
        val |= *p_data++;
        printf("Processor:0x%04x ", val);
        val = *p_data++ << 8;
        val |= *p_data++;
        printf("Bus:0x%04x", val);
        break;
    case 0xc8:
    case 0xcd:
	temp_uchar = *p_data++;

	/* if negative expand to uint */
	temp_uint = (temp_uchar & 0x80) ? 
	    (0xffffff00 | temp_uchar) : temp_uchar;

	printf("Minimum: %d dBmV, Maximum: %d dBmV\n",
	       temp_uint, *p_data++);
	p_data++;   /* skip to next field */
	printf("       Calibration values  : ");
	val = *p_data << 8;
	p_data++;
	printf("0x%04x ", val |= *p_data);
	p_data++;
	val = *p_data << 8;
	p_data++;
	printf("0x%04x ", val |= *p_data);
	p_data++;
	val = *p_data << 8;
	p_data++;
	printf("0x%04x", val |= *p_data);
	break;
    case 0xc0:
    case 0xdf:
    case 0xe2:
    case 0xe3:
	if (buf->p_info->type == 0xdf) {
	    max = 2;
	} else {
	    max = 3;
	}
	val = *p_data << 8;
	p_data++;
        if (buf->p_info->type == 0xdf) {
            printf("%03d-", val |= *p_data);
        } else {
            printf("%02d-", val |= *p_data);
        }
	p_data++;
	for (val = 0, i = 0; i < max; i++)
	    val = val * 256 + *p_data++;
	if (buf->p_info->type == 0xdf) {
	    printf("%04d-", val);
	} else {
	    printf("%05d-", val);
	}
	printf("%02d", *p_data);
	break;
    case 0xca:
	val = (*p_data << 8);
	*p_data++;
	printf("\n       HD Cylinders        : %d\n", val |= *p_data);
	*p_data++;
	val = (*p_data << 8);
	*p_data++;
	printf("       HD Block/Track      : %d\n", val |= *p_data);
	p_data++;
	val = (*p_data << 8);
	*p_data++;
	printf("       HD Bytes/Block      : %d\n", val |= *p_data);
	p_data++;
	val = (*p_data << 8);
	*p_data++;
	printf("       HD Heads            : %d\n", val |= *p_data);
	break;
    case 0x0b:
	val = *p_data;
	if (val == 0x00)
	    printf("AC");
	else if (val == 0x01)
	    printf("DC");
	else
	    printf("AC or DC ?");
	break;
    case 0x42:
    case 0x89:
    case 0x8a:
    case 0x8d:
    case 0xc1:
    case 0xc2:
    case 0xc6:
    case 0xce:
	val = buf->p_info->val_length;
	for (i = 0; i < val; i++)
	    if (display_char (*p_data++))
		return;
	break;
    case 0xc3:
        set_chassis_mac((char *)p_data);
    case 0xcf:
        //        printf("hello dafdsf ");
	val = *p_data << 8;
	p_data++;
	printf("%04x.", val |= *p_data);
	p_data++;
	val = *p_data << 8;
	p_data++;
	printf("%04x.", val |= *p_data);
	p_data++;
	val = *p_data << 8;
	p_data++;
	printf("%04x", val |= *p_data);
	break;
    case 0x80:
	val = *p_data << 8;
	p_data++;
	printf("%d-", val |= *p_data);
	p_data++;
	val = *p_data << 8;
	p_data++;
	printf("%d", val |= *p_data);
	break;
    case 0x82:
    case 0x85:
    case 0x87:
	printf("%02d-", *p_data);
	p_data++;
	val = *p_data << 8;
	p_data++;
        val |= *p_data;
        if (val >= 10000) {
	    printf("%05d-", val);
        } else {
	    printf("%04d-", val);
	}
	p_data++;
	printf("%02d",  *p_data);
	break;
    case 0xc7:  /* bug fix based on CSCvo89304-2 */
    case 0xc9:
    case 0xcb:
    case 0xcc:
    case 0xd0:
    case 0xd1:
    case 0xd2:
    case 0xd3:
    case 0xd4:
    case 0xd5:
    case 0xd9:
    case 0xe5:/* Add for Star Wifi PCBA */
    case 0xf0:
    case 0xf2:
    case 0xf3:
	format_type = buf->p_info->val_length & TYPE_SIZE_MASK;
	if (!(format_type == HEX_FORMAT)) {
	    for (i = 0; i < buf->length; i++)
		printf("%c", *p_data++);
	} else {
	    val = buf->length;
	    for (i = 0; i < val; i++)
		printf("%02x ", *p_data++);
            /* new request to display C9 subfields for NGIO (EDCS-1020001) */
            if (buf->p_info->type == 0xc9) {
                p_data = buf->p_val_byte;
                val = *p_data++;
                /* only display subfields of known requested versions */
                if (val == 1) { /* attribute version 1 */
                    printf("\n%7s%-28s: %02x", "", "Attribute Version", val);
                    val = *p_data++;
                    printf("\n%7s%-28s: %02x", "", "Type of Module", val);
                    val = *p_data++;
                    printf("\n%7s%-28s: %02x", "", "Module Size", val);
                    val = *p_data++;
                    printf("\n%7s%-28s: %02x", "", "Power Rating", val);
                    val = *p_data++;
                    printf("\n%7s%-28s: %02x", "", 
                                            "Number of Control Endpoints", val);
                    /* when "Number of Control Endpoints" = 0xff,
                     * this means the control interface is not defined
                     * then the control interface information doesn't need
                     * to be displayed 
                     */
                    if (val != 0xff) {
                        for (i = 1; i <= val; i++) {
                            const int maxlen = DEVICE_VALUES_SIZE;
                            char strbuf[maxlen];
                            snprintf(strbuf, maxlen-1, 
                                                "Control (%d) Interface Type", i);
                            printf("\n%7s%-28s: %02x", "", strbuf, *p_data++);
                            snprintf(strbuf, maxlen-1, 
                                                "Control (%d) Address Offset", i);
                            printf("\n%7s%-28s: %02x", "", strbuf, *p_data++);
                        }
                    }
                    val = *p_data++;
                    printf("\n%7s%-28s: %02x", "", 
                                            "Daughter Cards Capable", val);
                } else if (val == 2) { /* attribute version 2 */
                    printf("\n%7s%-28s: %02x", "", "Attribute Version", val);
                    val = *p_data++;
                    printf("\n%7s%-28s: %02x", "", "Type of Module", val);
                    val = *p_data++;
                    printf("\n%7s%-28s: %02x", "", "Module Size", val);
                    val = *p_data++;
                    printf("\n%7s%-28s: %02x", "", "Power Rating", val);
                    val = *p_data++;
                    printf("\n%7s%-28s: %02x", "", 
                                            "Daughter Cards Capable", val);
                    val = *p_data++;
                    printf("\n%7s%-28s: %02x", "", "KR Support", val);
                    val = *p_data++;
                    printf("\n%7s%-28s: %02x", "", "16-bit GPIO", val);
                    val = *p_data++;
                    printf("\n%7s%-28s: %02x", "", 
                                            "Alien Sub module Reset", val);
                    val = *p_data++;
                    printf("\n%7s%-28s: %02x", "", 
                                            "Number of Control Endpoints", val);
                    /* when "Number of Control Endpoints" = 0xff,
                     * this means the control interface is not defined
                     * then the control interface information doesn't need
                     * to be displayed 
                     */
                    if (val != 0xff) {
                        for (i = 1; i <= val; i++) {
                            const int maxlen = DEVICE_VALUES_SIZE;
                            char strbuf[maxlen];
                            snprintf(strbuf, maxlen-1, 
                                                "Control (%d) Interface Type", i);
                            printf("\n%7s%-28s: %02x", "", strbuf, *p_data++);
                            snprintf(strbuf, maxlen-1, 
                                                "Control (%d) Address Offset", i);
                            printf("\n%7s%-28s: %02x", "", strbuf, *p_data++);
                        }
                    }
                    /* Because oakenshield has this column start from 0x6D Attribute Version 
                     * and Thermal Profile address is 0x78 so buffer length need bigger than 0xB */ 
                    if (buf->length > 0xB) {      
                        val = *p_data++;
                        printf("\n%7s%-28s: %02x", "", 
                                            "Module Thermal Profile", val);
                    }
                }
            }
	}
	break;
    default:
	printf("New / unsupport type = 0x%2x!!!\n", buf->type);
	break;
    }
}


/* 
   show_cookie_4 ():
   (The wrapper)
   show all the cookie contents in the buffer link list.
   last_selected_index display the current counter. 

   Input:  board_type.
   Output: None.
 */ 
void
show_cookie_4 (int board_type)
{   
	  show_cookie_4_x(MENU_MODE, NULL, board_type);
}

/* 
   show_cookie_4_x ():
   show all the cookie contents in the buffer link list.
   last_selected_index display the current counter. 

   Input:  cli_mode or menu mode , target string, board_type,
   Output: None.
 */ 
 
void show_cookie_4_x(boolean mode, char *target, int board_type)
{   
	  COOKIE_4 *buf;

    last_selected_index = 1;
    
    buf = cookie_root;
    
    if(mode == CLI_MODE) { /* in CLI mode */
        printf("\n%s%s:%2x", target, buf->p_info->p_sn, buf->misc);
    }else {
        /* format version */
        printf("\n%2d %s%2x", last_selected_index++, buf->p_info->p_fs, buf->misc);
    }
    buf = buf->b;

    while (buf->spare_flag == SPARE)
        buf = buf->b;
    
    /* if in Menu mode */
    if(mode == MENU_MODE) { 
        /* compatibility type */
        printf("\n%2d %s%2x", last_selected_index++, buf->p_info->p_fs, buf->misc);
    }/* otherwise skip compatibility type */
    buf = buf->b;

    while (buf != cookie_root) {
        display_cookie_4_element(last_selected_index++, buf, board_type, mode, target);
        buf = buf->b;
    }
}

void
dump_old_cookie_x (uchar *byte_ptr, int cookie_size)
{
    int i, j;

    printf("\nOLD cookie contents: \n");
    for(j=0; j < cookie_size; ) {
        for (i = 0; i < 16;  i++, j++)
            printf("%2.2x ", *byte_ptr++);
        putchar('\n');
    }
    return;
}
 
int
cookie_sanity_check (int board_type, int cookie_type, int cookie_size)
{
    int  verbose = 1;
    char ch;

    /*  First check cookie valid or not */
    if (check_cookie_valid_x (board_type, cookie_type, 
		verbose, cookie_size)) {
	printf("\nContinue? (y/n) [n]: ");
	ch = getchar();
	printf("%c\n", ch);
	if ((ch == 'y') || (ch == 'Y'))
	    ;
	else
	    return (FAILED);
    }
    return(PASSED);
}


/*
 * read_cookie_content_x
 *
 * Input:
 *         content_ptr = ptr for eeprom data.
 *         num_byte   = num byte want to read.
 *         board_type = MOTHER_BOARD, cookie_type = MB, BP, TDM. FE, AIM ...
 *         board_type = NETWK_MODULE, cookie_type = slot number.
 *
 * Output: None.
 *
 */
void
read_cookie_content_x (uchar *content_ptr, int num_byte, int board_type, 
		     int cookie_type, int cookie_size)
{
    int status;

    /*  First read contents of cookie */
    get_cookie_4_plat_x(board_type, cookie_type, &status, content_ptr,
			cookie_size);

#ifdef DEBUG
    printf("\nIn read_cookie_content_x\n");
    dismem((uchar *)content_ptr, num_byte, (uint)content_ptr, 16);
    printf("\n");
#endif
}

/* 
 * read_cookie_4_content
 *
 * Input:  
 *         content_ptr = ptr for eeprom data.
 *         num_byte   = num byte want to read.
 *         board_type = MOTHER_BOARD, cookie_type = MB, BP, TDM. FE, AIM ...
 *         board_type = NETWK_MODULE, cookie_type = slot number.
 *        
 * Output: PASSED or FAILED.
 *
 */
int 
read_cookie_4_content (uchar *content_ptr, int num_byte, 
		       int board_type, int cookie_type,
                       int cookie_size)
{
    int status;

    if (cookie_sanity_check(board_type, cookie_type, cookie_size)) {
        printf("cookie sanity check failed\n");
	return(FAILED);
    }

    /*  First read contents of cookie */
    get_cookie_4_plat_x(board_type, cookie_type, &status, 
				content_ptr, cookie_size);

#ifdef DEBUG
    printf("\nIn read_cookie_content_x\n");
    dismem((uchar *)content_ptr, num_byte, (uint)content_ptr, 16);
    printf("\n");
#endif

    return(status);
}


/*---------------------------------------------------------------------------
 * toss_nm_cookie  
 *
 * DESCRIPTION:
 *     This function is the entry to cookie utilities for NM
 * 
 * PARAMETERS:
 *     N/A
 *
 * RETURNS:
 *     N/A
 *--------------------------------------------------------------------------*/


void
toss_nm_cookie (void)
{
    int slot, slot_start;
    uchar cookie[COOKIE_SIZE_256];
    struct ngio_intf_t *ngio;
    
    sc_context *con, cont;
    con = &cont;
    
#ifdef VERBOSE
    printf("\ntoss_nm_cookie()\n");
#endif
    
    slot_start = slot_start_with();
    slot = gethex_answer("\nEnter Slot Number: ", 
                         slot_start, slot_start, get_max_sm_slots() );
    
    ngio = (struct ngio_intf_t *)slot_get_ngiosm(slot);
    assert(slot);
    assert(ngio);
    if (!ngio->is_present(ngio)) {
	printf("\nNo Port Module in slot %d.\n", slot);
	return;
    }
    con->type = NETWK_MODULE;
    con->slot = slot;
    con->cookie_contents = cookie;

    plat_init_smart_eeprom_context (con, NETWK_MODULE, 
		slot, cookie);

    smart_cookie_read_write_eeprom(con, NULL);

}


/* 
   toss_cookie_4_x ():
   The main entry of the cookie_4 module.
   Read contents, process contents or save contents if been modified.

   Input:  board_type = MOTHER_BOARD, cookie_type = MB, BP, TDM. FE, AIM ...
   board_type = NETWK_MODULE, cookie_type = slot number.
   cookie_size
   Output: PASSED or FAILED.
   */
int 
toss_cookie_4_x (int board_type, int cookie_type, int cookie_size, cli_cookie_cmd *cli_cmd)
{
    uchar contents[cookie_size];

    printf("here cookei_4_core.c %d\n", __LINE__);
    if (read_cookie_4_content(contents, cookie_size,
			      board_type, cookie_type, cookie_size))
	return(FAILED);

    printf("here cookei_4_core.c %d\n", __LINE__);
    if (cookie_4_processor_x(contents, board_type, 
			     cookie_type, cookie_size, cli_cmd)) { /* 5th params is MENU mode */
	put_cookie_4_x((struct cookie_plat *)contents,
  		     board_type, cookie_type, cookie_size);
    }
    
    return(PASSED);
}


/*
  cookie_4_processor_x ():
  The core of the cookie_4 module.
  If the Format version byte not match 4 will load default format for the
  the new cookie.
  then loop in the selection until quit.

  Input:  contents : pointer to 128 bytes of tlv formated cookie contents.
  board_type = MOTHER_BOARD, cookie_type = MB, BP, TDM. FE, AIM ...
  board_type = NETWK_MODULE, cookie_type = slot number.
  cookie_size = 128, 256, 512, 1024 ... bytes.
  cli_cookie_cmd = cli cmd structure
  Output: None.
  */
int
cookie_4_processor_x (uchar *contents, int board_type, 
		    int cookie_type, int cookie_size, cli_cookie_cmd *cli_cmd)
{
    int   ret_val;
    unsigned long   rval;
    int   modified = 0;
    uchar *cookie_begin_addr = contents;
    char  fname[32];
    size_t size = 0;

    ret_val = parsing_n_setup_q_x(contents, cookie_size);

    if (ret_val == PARSING_N_SETUP_FAILED)
	return(modified);
    else if (ret_val == NEED_NEW_COOKIE_FORMAT) {
        dump_old_cookie_x(contents, cookie_size);
	printf("\n******************** WARNING ************************");
	printf("\ncookie content is either not programmed or corrupted.");
	init_cookie_4_default_x(board_type, cookie_type, 
				contents, cookie_size);
	parsing_n_setup_q_x (contents, cookie_size);
	printf("\n!!!!! User needs to SAVE board type is %d!!!!!\n", 
		board_type);
    }

    /* init p_tmp for new data storage space */
    p_tmp = tmp_buf;

    /* if cli_cmd != NULL then it run in cli mode */
    /* prevent to access undefined pointer */
    if (cli_cmd != NULL) {
        /* in CLI mode */
        if((cli_cmd->cli_mode == CLI_DISCOVERY) || 
           (cli_cmd->cli_mode == CLI_COOKIE)) {
            /* Modified 0 if no need to change , 1 if need to change */
            switch(cli_cmd->type) {
                case CLI_COOKIE_FILL :
                    /* assign the cli contents  */
                    memcpy(contents, cli_cmd->contents, cli_cmd->size);
                    modified = 1;
                    return (modified);
                case CLI_COOKIE_CHANGE :
                    if (cli_change_cookie(cli_cmd->field, cli_cmd->str, cli_cmd)) {
                        printf("Change cookie field %x = %s failed.\n", 
                        cli_cmd->field, cli_cmd->str);
                        return (modified);
                    }
                    update_cookie_4_x(WRITE_EEPROM, board_type, 0, 
			              cookie_begin_addr, cookie_size);
                    modified = 1;
                    return (modified);
                case CLI_COOKIE_DISPLAY_M2M :  /* display discovery style */
                    show_cookie_4_x(CLI_MODE, cli_cmd->buf, board_type);
                    return (modified);
                case CLI_COOKIE_DISPLAY_FMT :  /* display readable cookie */
                    printf("\nINDEX");
                    show_cookie_4_x(MENU_MODE, NULL, board_type);
                    printf("\n\n");
                    return (modified); 
                case CLI_COOKIE_DISPLAY_RAW : /* display raw cookie data */
                    update_cookie_4_x(SHOW_COOKIE_INGREDIENT,
			              board_type, 0, (uchar *)0, cookie_size);
	                  return (modified);
                default :
                    return (modified);
            
            }
        }
        return (modified);
    } /* CLI mode finished in here */

    /* Menu mode */
    while (1) {
        do {
	    rval = 0;
            printf("\nINDEX");
            show_cookie_4 (board_type);
            printf("\n\nOPTION\n");
#ifdef AHSU /* temp mask */
            printf("0. Raw cookie access.\n");
#endif
            printf("1. CHANGE cookie data field.\n");
            printf("2. ADD a new cookie data field.\n");
            printf("3. REMOVE a cookie data field.\n");
            printf("4. MOVE a cookie data field before another data field.\n");
            printf("5. DISPLAY raw cookie content.\n");
	    printf("6. LOAD default.\n");
            printf("7. SAVE then EXIT.\n");
            printf("8. EXIT only.\n");
            printf("                Select an option > ");


            get_dec (&rval);
#ifdef AHSU  /* temp mask */
	    if ((rval > 8) || (rval < 0))
		printf("\n\n*** Illgal option selected, "
		       "please try again...\n");
        } while ((rval > 8) || (rval < 0));
#endif
	    if ((rval > 8) || (rval < 1))
		printf("\n\n*** Illgal option selected, "
		       "please try again...\n");
        } while ((rval > 8) || (rval < 1));

        switch(rval) {
#ifdef AHSU  /* temp mask */
	case 0:
/*
	    raw_cookie_access ();
*/
            if (process_raw_cookie_all_x (contents,
                                          cookie_size ) == TRUE) {
                modified = 1;
                printf("\n!!!!! User needs to SAVE board type is %d!!!!!\n",
                board_type);
            }
            parsing_n_setup_q_x (contents, cookie_size);
            p_tmp = tmp_buf;
            break;
#endif
	case 1:

	    change_cookie_4 (board_type);
	    break;
	case 2:
	    add_cookie_4();
	    break;
	case 3:
	    remove_cookie_4();
	    break;
	case 4:
	    move_cookie_4();
	    break;
	case 5:
	    update_cookie_4_x(SHOW_COOKIE_INGREDIENT,
			    board_type, 0, (uchar *)0, cookie_size);
	    break;
	case 6:
            sprintf(fname, "/cookie.txt");
            if (file_exist(fname, &size)) {
                printf("/cookie.txt exist. \n");
                update_cookie_4_x(WRITE_COOKIE_FROM_FILE, board_type, 0,
                            cookie_begin_addr, cookie_size);
            } else {
                init_cookie_4_default_x(board_type, cookie_type, 
				  contents, cookie_size);
            }

            parsing_n_setup_q_x (contents, cookie_size);
            p_tmp = tmp_buf;
	    break;
        case 7:
            update_cookie_4_x(WRITE_EEPROM, board_type, 0,
                            cookie_begin_addr, cookie_size);
            modified = 1;
            return(modified);
            break;
	case 8:
            printf("\n\n**** Warning: Cookie content "
		   "is not saved. ****\n");
	    return(modified);
	    break;
	default:
	    cterr('w',0,"Illegal option %d selected by user.");
	    return(modified);
        }
    }
}

/*
  get_str_len

  get the lenght of the str and returns the number of
  bytes inside the buffer
   
  Input:  buf - points to our string
          cp  - cookie structure pointer
  Output: cnt - number of bytes inside the buffer
*/
static int 
get_str_len(char *buf, cookie_4_table *cp)
{
    char *tmp;
    int cnt, i;
    tmp = buf;

    if (cp->variable_type & DFMT) {
	if ((cp->val_length & TYPE_SIZE_MASK) == HEX_FORMAT) {     
	    for (i=0, cnt=1; i < (int)strlen((char *)buf); i++) {
		if (*tmp++ == ' ' ) 
		    cnt++;
	    }
	} else {
	    return strlen((char *)buf);
	}
    } else {
	if (strchr(cp->input_form, 'H') != NULL) {
	    for (i=0, cnt=1; i < (int)strlen((char *)buf); i++) {
		if (*tmp++ == ' ' ) 
		    cnt++;
	    } 
	} else {
	    return strlen((char *)buf);
	}
    }

    return cnt;
}


/*
  fetch_user_input_data ():
   
  Input:  tok_ptr, pointer to user input char stream.
  buf, current working buffer
  Output: None.
 */

void
fetch_user_input_data (char *tok_ptr, COOKIE_4 *buf)
{
    int i, val, numchar;
    utype_t ret_val;
    uchar ch, ch1;
    uchar format_type;

    if (!(buf->p_info->variable_type & DLEN))
	buf->length = buf->p_info->val_length;
    else {
        buf->length = get_str_len(tok_ptr, buf->p_info);
    }

    buf->p_val_byte = p_tmp;

    switch(buf->type) {
    case 0x41:
	val = buf->p_info->val_length;
	for (i = 0; i < val; i ++) {
	    numchar = getnnum(tok_ptr, 10, (utype_t *)&ret_val, 3);
	    *p_tmp++ = (uchar) ret_val;
	    tok_ptr += (numchar + 1);      /* skip dot */
	}
	break;
    case 0x43:
    case 0x46:
	/* upto 64k */
	numchar = getnnum(tok_ptr, 10, (utype_t *)&ret_val, 5);
	*p_tmp++ = (uchar)(ret_val / 256);
	*p_tmp++ = (uchar)(ret_val % 256);
        if (buf->type == 43) {
            set_mac_blk_size(ret_val);
        }
	break;
    case 0x83:
	numchar = getnnum(tok_ptr, 10, &ret_val, 8);
	for (i = 3; i >= 0; i--)
	    *p_tmp++ = (uchar)(ret_val >> (i * 8));
	break;
    case 0x88:
	numchar = getnnum(tok_ptr, 10, &ret_val, 10);
	for (i = 3; i >= 0; i--) {
	    *p_tmp++ = (uchar)(ret_val >> (i * 8));
	}
	break;
    case 0x01:
    case 0x02:
    case 0x05:
    case 0x81:
	val = buf->p_info->val_length;
	for (i = 0; i < val; i++) {
	    numchar = getnnum(tok_ptr, 10, &ret_val, 3);
	    *p_tmp++ = (numchar) ? (uchar)ret_val : 0x00;
	    tok_ptr += (numchar +1);      /* skip hyphen */
	}
	break;
    case 0x03:
    case 0x04:
    case 0x06:
    case 0x07:
    case 0x09:
    case 0x0a:
    case 0x40:
    case 0x44:
    case 0x45:
    case 0x47:
    case 0x48:
    case 0x49:
    case 0x4a:	/* Radio Country Code */
    case 0x84:
    case 0x86:
    case 0xc4:
    case 0xc5:
    case 0xc8:
    case 0xcd:
    case 0xd6:
	val = buf->p_info->val_length;
	for (i = 0; i < val; i++) {
	    numchar = getnnum(tok_ptr, 16, &ret_val, 2);
	    *p_tmp++ = (numchar) ? (uchar)ret_val : 0x00;
	    tok_ptr += (numchar + 1);      /* skip blank */
	}

	break;
    case 0xc0:
    case 0xdf:
    case 0xe2:
    case 0xe3:
	numchar = getnnum(tok_ptr, 10, &ret_val, 3);
	*p_tmp++ = (uchar)(ret_val / 256);
	*p_tmp++ = (uchar)(ret_val % 256);
	tok_ptr += (numchar +1);
	if (buf->type == 0xdf) {
	    numchar = getnnum(tok_ptr, 10, &ret_val, 4);
	    *p_tmp++ = (uchar)((ret_val >> 8) & 0xff);
	    *p_tmp++ = (uchar)(ret_val & 0xff);
	} else {
	    numchar = getnnum(tok_ptr, 10, &ret_val, 6);
	    *p_tmp++ = (uchar)((ret_val >> 16) & 0xff);
	    *p_tmp++ = (uchar)((ret_val >> 8) & 0xff);
	    *p_tmp++ = (uchar)(ret_val & 0xff);
	}
	tok_ptr += (numchar +1);
	numchar = getnnum(tok_ptr, 10, &ret_val, 2);
	*p_tmp++ = (uchar)(ret_val & 0xff);
	tok_ptr += (numchar +1);
	break;
    case 0xca:
	val = buf->p_info->val_length;
	for (i = 0; i < val; i++) {
	    numchar = getnnum(tok_ptr, 10, &ret_val, 4);
	    *p_tmp++ = (uchar)((ret_val >> 8) & 0xff);
	    *p_tmp++ = (uchar)(ret_val & 0xff);
	    tok_ptr += (numchar +1);
	}
	break;
    case 0x0b:
	ch = *tok_ptr++;
	ch1 = *tok_ptr;
	if ((ch1 == 'C') || (ch1 == 'c')) {
	    if ((ch == 'A') || (ch == 'a'))
		*p_tmp++ = 0x00;
	    else if ((ch == 'D') || (ch == 'd'))
		*p_tmp++ = 0x01;
	    else
		*p_tmp++ = 0x3f;   /* '?' */
	} else {
	    *p_tmp++ = 0x3f;
	}
	break;
    case 0x42:
    case 0x89:
    case 0x8a:
    case 0x8d:
    case 0xc1:
    case 0xc2:
    case 0xc6:
    case 0xce:
	val = buf->p_info->val_length;
	for (i = 0; i < val; i++)
	    *p_tmp++ = *tok_ptr++;
	break;
    case 0xc3:
        if (buf->type == 0xc3) {
            set_chassis_mac(tok_ptr);
        }
    case 0xcf:
	for (i = 0; i < 3; i++) {
	    numchar = getnnum(tok_ptr, 16, &ret_val, 4);
	    *p_tmp++ = (uchar)(ret_val / 256);
	    *p_tmp++ = (uchar)(ret_val % 256);
	    tok_ptr += (numchar + 1);      /* skip dot */
	}

	break;
    case 0x80:
	for (i = 0; i < 2; i++) {
	    numchar = getnnum(tok_ptr, 10, &ret_val, 5);
	    *p_tmp++ = (uchar)(ret_val / 256);
	    *p_tmp++ = (uchar)(ret_val % 256);
	    tok_ptr += (numchar + 1);
	}
	break;
    case 0x82:
    case 0x85:
    case 0x87:
	numchar = getnnum(tok_ptr, 10, &ret_val, 2);
	*p_tmp++ = (uchar)(ret_val & 0xff);
	tok_ptr += (numchar + 1);
	numchar = getnnum(tok_ptr, 10, &ret_val, 5);
	*p_tmp++ = (uchar)(ret_val / 256);
	*p_tmp++ = (uchar)(ret_val % 256);
	tok_ptr += (numchar + 1);
	numchar = getnnum(tok_ptr, 10, &ret_val, 2);
	*p_tmp++ = (uchar)(ret_val & 0xff);
	tok_ptr += (numchar + 1);
	break;
    case 0xc7:  /* bug fix based on CSCvo89304-2 */
    case 0xc9:
    case 0xcb:
    case 0xcc:
    case 0xd0:
    case 0xd1:
    case 0xd2:
    case 0xd3:
    case 0xd4:
    case 0xd5:
    case 0xd9:
    case 0xe5: /* Add for Star Wifi PCBA */
    case 0xf0:
    case 0xf2:
    case 0xf3:
	format_type = buf->p_info->val_length & TYPE_SIZE_MASK;
	if (!(format_type == HEX_FORMAT)) {
	    i = 0;
	    while (*tok_ptr != '\0') {
		*p_tmp++ = *tok_ptr++;
		i++;
		if (buf->p_info->val_length + i >
		    buf->p_info->len_fd)
		    break;
	    }
	    buf->length = i;
	} else {
	    val = buf->length;
	    for (i = 0; i < val; i++) {
		numchar = getnnum(tok_ptr, 16, &ret_val, 2);
		*p_tmp++ = (numchar) ? (uchar)ret_val : 0x00;
		tok_ptr += (numchar + 1);      /* skip blank */
	    }
	}
	break;
    default:
	printf("New / unsupport type = 0x%2x!!!\n", buf->type);
	break;
    }
}


/* 
   change_cookie_4 ():

   Input:  board_type.
   Output: None.
 */
void
change_cookie_4 (int board_type)
{
    uint sel_val;
    utype_t ret_val = 0;
    COOKIE_4 *buf;

    char inbuf[384];
    char *tok_ptr;

    sel_val = get_dec_user_input("Select an index number to change");
    if (!sel_val)
	return;

    if ((buf = buffer_search(sel_val)) == (COOKIE_4 *)NULL)
        return;

    if (sel_val > 2)
        display_cookie_4_element(sel_val, buf, board_type, FALSE, NULL);
    else
        printf("%2d %s%x", sel_val, buf->p_info->p_fs, buf->misc);

    if (sel_val == 1)
	printf("\n\n**** Warning, EEPROM format version other than 4 ****\n"
	       "**** the default cookie format will be restored. ****\n"); 

    prompt_for_display_format(buf);
    
    printf("\nUser input data field (type [length] not included, "
	   "Format as: %s) >", buf->p_info->input_form);
    if (!get_line(inbuf, sizeof(inbuf)))
	return;
    if ((*inbuf == 0) || (*inbuf == ESCAPE_CHAR))
        return;

    tok_ptr = inbuf;
    while (*tok_ptr == ' ')
        tok_ptr++;

    if (sel_val > 2)            /* handle general case */
        fetch_user_input_data(tok_ptr, buf);
    else {                      /* handle first or second element */
        getnnum(tok_ptr, 16, (utype_t *)&ret_val, 2);
        buf->misc = (uchar)ret_val;
    }
}


/* 
   add_cookie_4 ():
  
   Input:  None.
   Output: None.
 */
void
add_cookie_4 (void)
{
    char inbuf[384];
    char *tok_ptr;
    uchar sel_type;
    int i, numchar;
    utype_t ret_val;
    cookie_4_table *cp;
    COOKIE_4 *buf;

    /* display all the "type" */
    printf("DIAG TYPE\n");
    cp = cookie_4_info;
    i = 0;
    while (cp->p_fs != NULL) {
        printf("0x%02x %s", cp->type, cp->p_fs);
        if (i++ % 2)
            printf("\n");   /* one line for two items */
        else
            printf("    ");
        cp++;
    }

    printf("\nUser input type field (two HEX number) > ");

    for (i = 0; i < (int)sizeof(inbuf); i++)
        inbuf[i] = 0x00;
    if (!get_line(inbuf, sizeof(inbuf)))
	return;

    if (*inbuf == ESCAPE_CHAR)
	return;

    tok_ptr = inbuf;
    while (*tok_ptr == ' ')   /* skip space */
        tok_ptr++;

    numchar = getnnum(tok_ptr, 16, (utype_t *)&ret_val, 2);
    if (!numchar)
        return;

    sel_type = (uchar)ret_val;

#ifdef DEBUG
    printf("sel_type - 0x%2x\n", sel_type);
#endif

    /* make sure the type been supported */
    if ((cp = search_type_4_table(sel_type)) == (cookie_4_table *)NULL)
        return;

    /* check to prevent duplicate type entry in the current ring */
    if((buf = search_type_4_buf_pool(sel_type)) != (COOKIE_4 *)NULL)
        return;
    
    /* accept new input type afterward */
    if ((buf = get_new_buf()) == (COOKIE_4 *)NULL)
        return;
    buf->p_info = cp;       /* pointer in cookie_4_table */
    buf->type = sel_type;

    prompt_for_display_format(buf);
    
    printf("\nUser input data field (type [length] not included, "
	   "Format as:%s) >", buf->p_info->input_form);
    if (!get_line(inbuf, sizeof(inbuf)))
	return;
    if ((*inbuf == 0) || (*inbuf == ESCAPE_CHAR))
        return;

    tok_ptr = inbuf;
    while (*tok_ptr == ' ')
        tok_ptr++;

    fetch_user_input_data(tok_ptr, buf);

    cookie_4_enque(cookie_root, buf);
    return;
}


/*
  search_type_4_buf_pool ():
  using the selected type to get the associated buffer in the link list.

  Input:  target_type, the type user looking for. 
  Output: wanted buffer or NONE.
  */

COOKIE_4 *
search_type_4_buf_pool (uchar target_type)
{
    COOKIE_4 *buf;

    buf = cookie_root->f;
    while (buf != cookie_root) {
        if (buf->spare_flag == NON_SPARE) {
            if (buf->type == target_type) {
                printf("\nDuplicate TYPE in the same cookie is not allowed!!");
                return(buf);
            }
        }
        buf = buf->f;
    }
    return((COOKIE_4 *)NULL);
}


/* 
   search_type_4_table ():
   using the selected type to get the associated cookie_pointer 
   in the data structure.

   Input:  target_type, the type user looking for.
   Output: cookie_pointer in the cookie_4_info table.
 */
cookie_4_table *
search_type_4_table (uchar target_type)
{
    cookie_4_table *cp;

    /* found right entry in cookie_4_table */
    cp = cookie_4_info;
    while (cp->p_fs != NULL) {
        if (target_type != cp->type) {
            cp++;
        }
        else
            return(cp);
    }

    printf("\n***** Probably new or unsupport type = 0x%02x!!\n", target_type);

    return((cookie_4_table *)NULL);
}


static void
get_dec(unsigned long *val)
{
    char inbuf[30];
    char *ptr;
    int i;

    for (i = 0; i < (int)sizeof(inbuf); i++)
	inbuf[i] = 0x00;

    get_line(inbuf, sizeof(inbuf));

    if (*inbuf == ESCAPE_CHAR)
	return;
    ptr = inbuf;
    while (*ptr == ' ')
	ptr++;
    getnnum(ptr, 10, val, 2);

#ifdef DEBUG
    printf("get_dec() val = 0x%x \n", *val);
#endif
}

int
get_dec_user_input (char *s)
{
    utype_t ret_val = 0;

    while (1) {
        printf("%s > ", s);
        get_dec(&ret_val);
        if ((ret_val < last_selected_index) && (ret_val >= 0))
            break;
        printf("\nUndefine input, try again!!\n");
    }
    return(ret_val);
}


/* 
   buffer_search ():
   (the wrapper)
   using index to search the correspond buffer in the link list.

   Input:  target_index,
   Output: None or a buffer in the link list.
   */
COOKIE_4 *
buffer_search (int target_index)
{
    COOKIE_4 *buf;

    buf = buffer_search_x (FALSE, target_index);
    if(buf == (COOKIE_4 *)NULL)
         return((COOKIE_4 *)NULL);
    else
    	   return (buf);
        
}

/* 
   buffer_search_x ():
   using index to search the correspond buffer in the link list.

   Input:  target_index, cli_mode or menu mode
   Output: None or a buffer in the link list.
   */
COOKIE_4 *
buffer_search_x (boolean mode, int target_index)
{
    COOKIE_4 *buf;
    int buf_cnt = 0;
    
    buf = cookie_root;
    buf = buf->b;
    if (mode == FALSE) { /* In Menu mode */
        buf_cnt = 1;

        if (target_index == 1)
            return(cookie_root);
        buf_cnt++;
    }
    while (buf->spare_flag == SPARE)
        buf = buf->b;
    while (buf != cookie_root) {
    	  if (mode == FALSE) { /* In Menu mode */
            if (buf_cnt++ == target_index)
                return(buf);
        }else {
        	  if (buf->type == target_index){
                return (buf);
            }
        }
        buf = buf->b;
    }
    printf("Index out of COOKIE_4 buffer pool!!\n");
    return((COOKIE_4 *)NULL);
}

/* 
   remove_cookie_4 ():
   remove a buffer from link list and label "SPARE" for recycle purpose.

   Input:  None.
   Output: None.
   */
void
remove_cookie_4 (void)
{
    int  ret_val = 0; 
    COOKIE_4 *buf;

    ret_val = get_dec_user_input("\nSelect an index number to remove");
    if (!ret_val)
        return;

    if (ret_val < 3) {
        printf("**** Warning: Format version(1) and Compatibility(2) "
	       "field are reserved!! ****\n");
        return;
    }
    buf = buffer_search(ret_val);

    cookie_4_deque (buf);
    buf->spare_flag = SPARE;

    /* save to cookie_root backward side */
    cookie_4_enque(cookie_root->b, buf);
    return;
}


/*
  move_cookie_4 ():
  moving a buffer to front of the other buffer.

  Input:  None.
  Output: None.
  */
void
move_cookie_4 (void)
{
    int ret_val;
    COOKIE_4 *buf_from, *buf_to;

    ret_val = get_dec_user_input("Select an index number to move from");
    if (!ret_val)
        return;

    /* compatibility and format version field are reserved */
    if (ret_val < 3) {
        printf("Format version(1) and Compatibility(2) field are reserved.\n"
               "Please select other data field to move from !!\n");
        return;
    }
    buf_from =  buffer_search(ret_val);

    ret_val = get_dec_user_input("Select an index number to fill before");
    if (!ret_val)
        return;

    /* 
     * between compatibility and format version field is reserved 
     * for spare buf only 
     */
    if (ret_val == 2) {
        printf("Please select other data field to fill, "
	       "Compatibility(2) field is reserved!!");
        return;
    }
    buf_to = buffer_search(ret_val);

    if (buf_from == buf_to)
        return; /* do nothing */

    /* remove the buf_from buf from link list */
    cookie_4_deque (buf_from);

    /* add buf_from buffer to buf_to buffer forward side */
    cookie_4_enque(buf_to, buf_from);
    return;
}


/*
  update_cookie_4_x ():
  display raw cookie contents or write to EEPROM.

  Input:  condition_flag: to choose only display or write to cookie. 
  condition_flag:
  board_type: mother_board or network_module.
  reserved:
  original_cookie_ptr: feed in cookie contents addr.
  cookie_size:
  Output: None.
 */
void
update_cookie_4_x (int condition_flag, int board_type, 
		   int reserved, uchar *original_cookie_ptr, 
		   int cookie_size)
{
    uchar out_prom[cookie_size*2], *out_p, *cptr;
    char buf_tmp[cookie_size*2], *buf_p;
    char  fname[32];
    int   i, data_cnt;
    ushort   val1;
    COOKIE_4 *buf;
    FILE *fp;

    for (i = 0; i < cookie_size; i++)
        out_prom[i] = 0xff;

    out_p = out_prom;
    buf = cookie_root;
    /* format 4 */
    *out_p++ = buf->misc;
    buf = buf->b;

    while (buf->spare_flag == SPARE)
        buf = buf->b;

    /* compatibility byte */
    *out_p++ = buf->misc;
    buf = buf->b;

    while (buf != cookie_root) {
        *out_p++ = buf->type;
	if (buf->p_info->len_fd) {
	    if (buf->p_info->variable_type & DLEN) {
		if (buf->p_info->type >= F_TYPE) {
		    /* handle the case where length field is 2 bytes */
		    val1 = buf->length | (buf->p_info->val_length << 8);
		    *out_p++ = (uchar)((val1 & 0xFF00) >> 8);
		    *out_p++ = (uchar)(val1 & 0x00FF);
		} else
		    *out_p++ = buf->length + buf->p_info->val_length ;
	    }
	    else 
                *out_p++ = buf->p_info->len_fd;
	}
        data_cnt = buf->length;
        cptr = buf->p_val_byte;
        for (i = 0; i < data_cnt; i++) {
            *out_p++ = *cptr++;
        }
        buf = buf->b;
    }

    if (out_p - out_prom > cookie_size) {
        printf("Format 4 cookie only has %d byte size!!\n, ", 
			cookie_size);
	printf("Delete unnecessary field then try again.\n");
        return;
    }

    switch (condition_flag) {
    case WRITE_EEPROM:
	/* save back to original cookie */
	movbyte(out_prom, original_cookie_ptr, cookie_size);
	break;
    case SHOW_COOKIE_INGREDIENT:
        sprintf(fname, "/cookie.txt");
        fp = fopen(fname, "w");
        printf("EEPROM contents (hex):");
        out_p = out_prom;
        for (i = 0; i < cookie_size; i++) {
            if (!(i % 16)) {
                printf("\n  0x%02x:", i);
                fprintf(fp, "\n");
            }
            fprintf(fp, " %02x", *out_p);
            printf(" %02x", *out_p++);
        }
        printf("\n\n");
        fclose(fp);
        break;
    case WRITE_COOKIE_FROM_FILE:
        
        memset(buf_tmp, 0, (cookie_size*2));

        printf("Write EEPROM contents from /cookie.txt \n");
        fp = fopen("/cookie.txt", "r");
        if (!fp) {
            printf("file /cookie.txt is not exist\n");
            printf("please using dump cookie to file first\n");
            return; 
        }

        buf_p = buf_tmp;

        for (i = 0; i < cookie_size; i++) {
            fscanf(fp, "%s", buf_p);
            out_prom[i] = strtol(buf_p, NULL, 16);
            
        }
        fclose(fp);

        out_p = out_prom;
        for (i = 0; i < cookie_size; i++) {
            if (!(i % 16))
                printf("\n  0x%02x:", i);
            printf(" %02x", *out_p++);
        } 

        if (getc_answer("\n\nWriting above cookie?", "yn", 'n') == 'y') {
            /* save back to original cookie */
            movbyte(out_prom, original_cookie_ptr, cookie_size);
        } else { 
            printf("Abort..\n ");
        }
        break;
    default:
	break;
    }
    return;
}

/*
  search_type_ret_addr_of_first_data ():
  using "type" to search through cookie_4_info table, and 
  return the beginning address
  of data contents and how many bytes in the data field. 

  Input:  begin_of_cookie, beginning address of cookie prom.
  wanted_type, 
  ret_num_of_bytes, number of bytes in the data field.

  Output: the beginning address of data field following the "type [length]".
  NULL, if not found.
  */
uchar * 
search_type_ret_addr_of_first_data (uchar *begin_of_cookie, uchar wanted_type,
				    uchar *ret_num_of_bytes, int verbose)
{
    int val1, val2, i;
    uchar *contents = begin_of_cookie;
    cookie_4_table *cp;       /* cookie_4_table pointer */
    uchar temp_type;
    uchar dynamic_length = 0;

    if (*contents != CURRENT_FORMAT_VERSION) {
	if (verbose)
            cterr('f',0,"Not a TLV cookie format version;\nactual: %#x "
		  "expected: 0x4; user search type %#x.",
		  *contents, wanted_type);
	return((uchar *)NULL);
    }

    /* check the type in the cookie_4_info table or not */
    if ((cp = search_type_4_table(wanted_type)) == (cookie_4_table *)NULL)
	return((uchar *)NULL);
 
    /* skip to the third byte , where the first "type" existed */
    contents += 2;

    while (*contents != LAST_BYTE) {
	if (contents - begin_of_cookie >= COOKIE_SIZE_256)
	    return((uchar*)NULL);

        if (*contents == EXTENSION_BYTE) {
            contents++;
            continue;
        }

        if ((cp = search_type_4_table(*contents)) == (cookie_4_table *)NULL) {
	    /* Print whole cookie content from memory */
	    for (i=0; i < COOKIE_SIZE_256; i++) {
		if (!( i % 16)) printf("\n%04x: ", i);
		printf("%02x ", *(begin_of_cookie + i));
	    }
	    if (verbose)
	        cterr('f',0,"Probably not a standard TLV format cookie, "
		      "type = 0x%02x", *contents);
	    return((uchar *)NULL);
	}

        temp_type = *contents++;
	/* skip length field */ 
        if ((temp_type & TYPE_SIZE_MASK) == TYPE_LENGTH_FOLLOW) { 
	    if (cp->variable_type & DLEN) {
		if (cp->type >= F_TYPE) {
		    /* field 0xf0 and higher use 2 bytes for length field */
		    /* ignore 2 MSBytes */
		    val1 = *contents++ & TYPE_LENGTH_MASK; 
		    val2 = *contents;
		    dynamic_length = ((val1 << 8) | val2) ;
		} else {
		    /*ignore 2 MSBs */
		    dynamic_length = *contents & TYPE_LENGTH_MASK; 
		}
	    }
            contents++;
        }

        if (temp_type != wanted_type) {
	    if (dynamic_length) {
		contents += dynamic_length;
		dynamic_length = 0;
	    } else {
	        contents += cp->val_length;
	    }
	    /* point to next type now */
	}
	else {
	    *ret_num_of_bytes = (dynamic_length) ? 
		dynamic_length  : cp->val_length;
            /* return the beginning addr of data field */
	    return(contents);
	}
    }
    return((uchar *)NULL);
}

/*
  search_type_ret_addr_of_first_data_x ():
  using "type" to search through cookie_4_info table, and
  return the beginning address
  of data contents and how many bytes in the data field.

  Input:  begin_of_cookie, beginning address of cookie prom.
  wanted_type,
  ret_num_of_bytes, number of bytes in the data field.

  Output: the beginning address of data field following the "type [length]".
  NULL, if not found.
  */
uchar *
search_type_ret_addr_of_first_data_x (uchar *begin_of_cookie, uchar wanted_type,
                        uchar *ret_num_of_bytes, int verbose, int cookie_size)
{
    int val1, val2, i;
    uchar *contents = begin_of_cookie;
    cookie_4_table *cp;       /* cookie_4_table pointer */
    uchar temp_type;
    uchar dynamic_length = 0;

    if (*contents != CURRENT_FORMAT_VERSION) {
        if (verbose)
            cterr('f',0,"Not a TLV cookie format version;\nactual: %#x "
                  "expected: 0x4; user search type %#x.",
                  *contents, wanted_type);
        return((uchar *)NULL);
    }

    /* check the type in the cookie_4_info table or not */
    if ((cp = search_type_4_table(wanted_type)) == (cookie_4_table *)NULL)
        return((uchar *)NULL);

    /* skip to the third byte , where the first "type" existed */
    contents += 2;

    while (*contents != LAST_BYTE) {
        if (contents - begin_of_cookie >= cookie_size)
            return((uchar*)NULL);

        if (*contents == EXTENSION_BYTE) {
            contents++;
            continue;
        }

        if ((cp = search_type_4_table(*contents)) == (cookie_4_table *)NULL) {
	    /* Print whole cookie content from memory */
	    for (i=0; i < cookie_size; i++) {
		if (!( i % 16)) printf("\n%04x: ", i);
		printf("%02x ", *(begin_of_cookie + i));
	    }
            if (verbose)
                cterr('f',0,"Probably not a standard TLV format cookie, "
                      "type = 0x%02x", *contents);
            return((uchar *)NULL);
        }

        temp_type = *contents++;
        /* skip length field */
        if ((temp_type & TYPE_SIZE_MASK) == TYPE_LENGTH_FOLLOW) {
            if (cp->variable_type & DLEN) {
                if (cp->type >= F_TYPE) {
                    /* field 0xf0 and higher use 2 bytes for length field */
                    /* ignore 2 MSBytes */
                    val1 = *contents++ & TYPE_LENGTH_MASK;
                    val2 = *contents;
                    dynamic_length = ((val1 << 8) | val2) ;
                } else {
                    /*ignore 2 MSBs */
                    dynamic_length = *contents & TYPE_LENGTH_MASK;
                }
            }
            contents++;
        }

        if (temp_type != wanted_type) {
            if (dynamic_length) {
                contents += dynamic_length;
                dynamic_length = 0;
            } else {
                contents += cp->val_length;
            }
            /* point to next type now */
        }
        else {
            *ret_num_of_bytes = (dynamic_length) ?
                dynamic_length  : cp->val_length;
            /* return the beginning addr of data field */
            return(contents);
        }
    }
    return((uchar *)NULL);
}


/*
  get_cookie_4_controller_type ():
  search control_type in the cookie then return the ID for the 
  module.

  Input:  board_type, either mother_board or network_module.
  cookie_type, number of port module or tdm, aim cards.

  Output: controller_type - the id number for the module. 
  */
ushort
get_cookie_4_controller_type (int board_type, int cookie_type)
{
    int cookie_size = COOKIE_SIZE_256;
    uchar contents[cookie_size];
    uchar num_byte, *data_ptr;
    ushort controller_type;
    int i, status;
    
    get_cookie_4_plat_x(board_type, cookie_type, &status, 
			contents, cookie_size);
    controller_type = 0;

    if ((data_ptr = (uchar *)search_type_ret_addr_of_first_data 
	 (contents, (uchar)CONTROLLER_TYPE, 
	  &num_byte, FALSE)) == (uchar *)NULL){
	/*Search CONTROLLER_TYPE failed. */ 
	controller_type = 0xffff; /* illegal code */
    }
    else {
	for (i = num_byte; i > 0;){
	    controller_type |= *data_ptr << (--i*8);
	    data_ptr++;  
	}
    }
    return (controller_type);
}


/*
 * check_cookie_valid_x ()
 * 
 * Check the current cookie is valid or not.
 *
 * input:  board_type: mother_board or network_module. 
 *	   cookie_type: port module or TDM, AIM cards.
 *	   verbose :
 *
 * output: PASSED 0
 *	   FAILED 1
 */
int
check_cookie_valid_x (int board_type, int cookie_type, int verbose,
                    int cookie_size)
{
    int i;
    int status;
    uchar *cptr;
    uchar contents[cookie_size];
 
    /* intent to share with external user : call from port modules */
 
    get_cookie_4_plat_x(board_type, cookie_type, &status, 
				contents, cookie_size);
    if (status == FAILED) {
        if (verbose)
            cterr('f',0,"Fail to read cookie contents.");
        return(FAILED);
    }

    cptr = contents;

    for (i = 0; i < cookie_size; i++, cptr++)
	/* cptr is incremented for each for loop */
        if ((*cptr != 0xff) && (*cptr != 0x0))
            return (PASSED);
 
    if (verbose) {
        printf("\nDump Cookie data:");
        dismem((uchar*)cptr, cookie_size, *(unsigned*)cptr, 1);
	cterr('f',0,"Cookie content is either blank or "
	      "module is not installed.");
    }
    return(FAILED);
}


/*
** Read cookie contents 
*/
void
get_cookie_4_plat_x (int board_type, int cookie_type,
		   int *status, uchar *contents, int cookie_size)
{
    unsigned int rc;
    rc = read_eeprom_block(0, cookie_size, contents);
    if (rc != RC_I2C_OP_OK) {
        printf("problem reading eeprom \n");
    }
    return;
}


void
put_cookie_4_x (cookie_plat_t *cptr, int board_type, 
		int cookie_type, int cookie_size)
{
#ifdef LINUX_APP
    printf("FIX ME need to suport put_cookie_4_x()\n");
    assert(0);
#else /* Diagmon */
    pas_management_t *pas_cookie_ptr;

    pas_cookie_ptr = (pas_management_t *)
           get_pas_cookie_4_ptr(board_type, cookie_type);
    write_sys_eeprom_x(pas_cookie_ptr, cptr, cookie_size);
#endif /* LINUX_APP */
}


#ifdef AHSU
/*---------------------------------------------------------------------------
 * print_sm_cookie_field_by_field()
 *
 * DESCRIPTION:
 *     This function reads and displays all fields in the digital signature
 * list field by field
 * 
 * PARAMETERS:
 *     con - context structure pointer 
 *     dig_list - pointer to the array of the digital signature list
 *     mum_byte - number of elements in the digital signature list
 *
 * RETURNS:
 *     N/A
 *--------------------------------------------------------------------------*/

void
print_sm_cookie_field_by_field (sc_context *con, uchar *dig_list,
				uchar num_byte)
{

    uchar i, j, bytes, *data_ptr;
    char *pcb, *pid;
    uchar controller_type[4];
    char pcb_serial_val_init[12] = " NO PCB NUM";
    char pm_serial_num_ascii[12];
    char product_id_init[15] = " NO PRODUCT ID";
    uchar product_id[128];
    cookie_4_table *cp;

    for (j = 0; j < num_byte; j++) {
        cp = search_type_4_table (dig_list[j]);
	printf("\n%s", cp->p_fs);
	
	switch (dig_list[j]) {
	case CONTROLLER_TYPE:
	    if (( data_ptr = (uchar *)search_type_ret_addr_of_first_data 
		   (con->cookie_contents, (uchar)CONTROLLER_TYPE, 
		    &bytes, FALSE)) == (uchar *)NULL){
	       /*Search CONTROLLER_TYPE failed. */ 
	         for (i = 0; i < bytes + 2; i++)
		     controller_type[i] = 0xff;
	    } else {
	        for (i = 0; i < bytes; i++) {
		     controller_type[i] = *data_ptr++;
		     printf("%02x ", controller_type[i]);
		}
	    }
	    break;
	case PCB_SERIAL_NUM:
      	    if ((data_ptr = (uchar *)search_type_ret_addr_of_first_data
		 (con->cookie_contents, PCB_SERIAL_NUM, 
		  &bytes, FALSE)) == NULL) {
	        pcb = pcb_serial_val_init;
		for ( i = 0; i < 13; i++)
		    pm_serial_num_ascii[i] = *pcb++;
	    } else {
	        for (i = 0; i < bytes; i++) {
		    pm_serial_num_ascii[i] = *data_ptr++;
		    printf("%02x ", pm_serial_num_ascii[i]);    
		}
	    }
	    break;
	/* Adding product ID (0xCB) so the 0xD9 list can include it. The
	   request is from Bryce project */
	case PRODUCT_ID:
      	    if ((data_ptr = (uchar *)search_type_ret_addr_of_first_data
		 (con->cookie_contents, PRODUCT_ID, 
		  &bytes, FALSE)) == NULL) {
	        pid = product_id_init;
		for ( i = 0; i < 15; i++)
		    product_id[i] = *pid++;
	    } else {
	        for (i = 0; i < bytes; i++) {
		    product_id[i] = *data_ptr++;
		    printf("%02x ", product_id[i]);    
		}
	    }
	    break;
	default:
	    break;
	}
    }
}
#endif

/*-----------------------------------------------------------------------------
 *
 * Function: display_eeprom
 *
 * This function prints ata that's passed in.
 *
 * Input: print_array  - array to be printed out
 * 
 * Output: none.
 *
 *-------------------------------------------------------------------------
 */
void display_eeprom(uchar *print_array)
{
    int i, j;
    
    printf("\n");
    for( j = 0; j < 8; j++ ) {
        for ( i = 0; i < 16;  i++ )
           printf("%2.2x ", *print_array++ );
        putchar('\n');
    }

}

/*---------------------------------------------------------------------------
 * format_eeprom_data
 * DESCRIPTION:
 *     ask user for location within eeprom to modify.
 * 
 * PARAMETERS:
 *     eeprom_data_ptr - points to eeprom data
 *
 * RETURNS:
 *     PASSED
 *--------------------------------------------------------------------------*/

ushort format_eeprom_data(uchar *eeprom_data_ptr )
{
  int addr;
  register char *c_ptr;
  char inbuf[16];
  uint val;
  char buffer[120];
  int tmp;  

    printf("\nPress x or q to quit");
    printf("\nEnter goes to next location. \n\n");

    sprintf(buffer, "Enter address offset within Cookie you want to change:");

    addr = (int)gethex_answer(buffer, 0, 0, 256);

    while( 1 ) {

          printf("%.2x > %.2x ", addr, eeprom_data_ptr[addr]  );
          c_ptr = inbuf;
          get_line(c_ptr,sizeof(inbuf));

          switch( *c_ptr ) {
             case 'x': /* quit */ 
             case 'q': /* quit */
               goto exit;
	       break;

             case 0:   /* next location */
             break;

             default:
               tmp = getnum(c_ptr,16,&val);
               if( tmp == 0 ) {
                  printf("bad value \"%s\"\n",c_ptr);
                  continue; /* same location again */
               } else {
                  eeprom_data_ptr[addr] = (uchar)val;
	       } /* else */
               break; /* next location */
          }
          addr = addr + 1;
    }  /* while */
exit:

    display_eeprom(eeprom_data_ptr);

    return( PASSED );
}

/*
  update_selected_tlv_pack ():
  using "type" to search through cookie_4_info table.
     the update length was restrict to the same length, otherwise
     the operation will overwrite the next TLV pack; update to the
     end of the cookie list does not have the same restriction.

  Input:  begin_of_cookie, beginning address of cookie eeprom.
  wanted_type,
  update_data_size - number of bytes will be updated in the data field.
  update_data_ptr -  begin pointer of input data\[array].
  display_type - hex, ASCII, decimal ...
  verbose

  Output: PASSED/FAILED
  */
int
update_selected_tlv_pack (uchar *begin_of_cookie, uchar wanted_type,
                          unsigned int update_data_size, uchar *update_data_ptr,
                          int display_type, int verbose)
{
    unsigned int val1, val2, i, length_byte;
    uchar *contents = begin_of_cookie;
    cookie_4_table *cp;       /* cookie_4_table pointer */
    uchar temp_type;
    uchar dynamic_length = 0;

    if (*contents != CURRENT_FORMAT_VERSION) {
        if (verbose)
            cterr('f',0,"Not a TLV cookie format version;\nactual: %#x "
                  "expected: 0x4; user search type %#x.",
                  *contents, wanted_type);
        return(FAILED);
    }

    /* check the type in the cookie_4_info table or not */
    if ((cp = search_type_4_table(wanted_type)) == (cookie_4_table *)NULL)
        return(FAILED);

    /* skip to the third byte , where the first "type" existed */
    contents += 2;

    while (*contents != LAST_BYTE) {
        if (contents - begin_of_cookie >= COOKIE_SIZE_256)
            return(FAILED);

        if (*contents == EXTENSION_BYTE) {
            contents++;
            continue;
        }

        if ((cp = search_type_4_table(*contents)) == (cookie_4_table *)NULL) {
	    /* Print whole cookie content from memory */
	    for (i=0; i < COOKIE_SIZE_256; i++) {
		if (!( i % 16)) printf("\n%04x: ", i);
		printf("%02x ", *(begin_of_cookie + i));
	    }
            if (verbose)
                cterr('f',0,"Probably not a standard TLV format cookie, "
                      "type = 0x%02x", *contents);
            return(FAILED);
        }

        temp_type = *contents++;
        /* skip length field */
        if ((temp_type & TYPE_SIZE_MASK) == TYPE_LENGTH_FOLLOW) {
            if (cp->variable_type & DLEN) {
                if (cp->type >= F_TYPE) {
                    /* field 0xf0 and higher use 2 bytes for length field */
                    /* ignore 2 MSBits */
                    val1 = *contents++ & TYPE_LENGTH_MASK;
                    val2 = *contents;
                    dynamic_length = ((val1 << 8) | val2) ;
                } else {
                    /*ignore 2 MSBs */
                    dynamic_length = *contents & TYPE_LENGTH_MASK;
                }
            }
            contents++;
            /* point to first value data */
        }

        if (temp_type != wanted_type) {
            if (dynamic_length) {
                contents += dynamic_length;
                dynamic_length = 0;
            } else {
                contents += cp->val_length;
            }
            /* point to next type now */
        }
        else {  /* update the cookie contents follow the current exist type */
            if (dynamic_length) {
                if (dynamic_length != update_data_size) {
                    cterr('f',0,"update_data_size is not same as the existed one.\n");
                    return(FAILED);
                }
            } else {
                if (cp->val_length != update_data_size) {
                    cterr('f',0,"update_data_size is not same as the existed one.\n");
                    return(FAILED);
                }
            }
            for (i = 0; i < update_data_size; i++)
                *contents++ = *update_data_ptr++;

            return(PASSED);
        }
    }

    /* add to the end of cookie list */
    if (wanted_type >= F_TYPE)
        length_byte = 2;
    else if (wanted_type >= C_TYPE)
        length_byte = 1;
    else
        length_byte = 0;
    if ((contents + length_byte + update_data_size) - begin_of_cookie
                                >= COOKIE_SIZE_256)
        return(FAILED);

    *contents++ = wanted_type;

    if (wanted_type >= F_TYPE) {            /* two bytes length category */
        *contents++ = display_type | (update_data_size / 0xff);
        *contents++ = update_data_size % 0xff;
     } else if (wanted_type >= C_TYPE)      /* one byte length category */
        *contents++ = display_type | update_data_size;

    for (i = 0; i < update_data_size; i++)
        *contents++ = *update_data_ptr++;

    return(PASSED);
}

/******************************************************************
 * Function: process_raw_cookie_all_x
 *
 * User enters all cookie data on SINGLE command line, then
 * store the cookie in the flash. Note that on Giove, cookie is in
 * the nvram and nvram is stored in flash.
 * Format for the cookie enter: "04 ff c3 ... 12", then 'ENTER' to
 * finish.
 *
 * Input - cookie_contents: array to store the cookie data
 *         in_buf: buffer to store the cookie string that user inputs.
 *                 This should be equal to length*3 since each cookie
 *                 byte contains 2 digits(ASCII) and 1 space(ASCII)
 *                 as mentioned in the format above.
 *         cookie_size:
 *
 * Output: TRUE - cookie data was changed, need to save later
 *         FALSE - cookie data was not changed, no need to save
 *
 ******************************************************************
 */
int
process_raw_cookie_all_x (uchar *cookie_contents, int cookie_size)
{
    int digit_entered, ix, iy;
#ifdef AHSU
    int digit_entered, ix, iy, temp_num;
    long temp_long;
#endif
    char *b_ptr;
    int in_buf_length = cookie_size *3;
    char in_buf[in_buf_length];
  
    /* Each cookie byte contains 2 digits and 1 space as "04 ff ..." */
    in_buf_length = cookie_size * 3;

    printf("\n");
    printf("!!! CAUTION !!!\n");
    printf("This operation will erase ALL cookie contents\n");
    printf("Just hit 'ENTER' at the prompt to quit, OR\n");
    printf("Please input the cookie contents as: "
           "04 ff c3 ... 12, then 'ENTER' > \n");
    b_ptr    = in_buf;
    digit_entered = get_line(b_ptr, in_buf_length);

    printf("digit_entered = %d\n", digit_entered);

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("in_buf string = %s\n", in_buf);
        printf("Number of digit entered = %d\n", digit_entered);
        printf("Contents of input buffer:\n");
        for (ix=0; ix<in_buf_length; ix++) {
            if (ix%8 == 0) {
                printf("\n");
                printf("0x%08x: ", ix);
            }
            printf("0x%02x ", in_buf[ix]);
        }
    }

    /* Some error checking... */
    if ((digit_entered == 0 ) || (*in_buf == ESCAPE_CHAR)) {
        printf("Enter nothing!!!\n");
        return(FALSE);          /* nothing entered? */
    }
    /*
     * Input correct? Should be "04 ff c3 .... 12('ENTER' to finish)"
     * +1 is to take care of the string terminator, ie. '\0'
     * Note that each cookie byte has 2 digits and 1 space.
     */
    if (((digit_entered) % 3) != 0) {
        printf("\n*** Should be in the form of \"04 ff c3 ... 12 ff\"\n");
        return(FALSE);
    }

    /* Convert the inputs in ASCII to hex number */
    for (ix=0, b_ptr=in_buf, iy=0;
         ix < digit_entered;
         ix+=3, b_ptr+=3, iy++) {
        /*
         * Check 1st and 2nd digits are hex, followed by space or
         * string terminator, ie. '\0'
         */
#ifdef AHSU
        if ((isxdigit(*b_ptr)) && (isxdigit(*(b_ptr+1))) && 
            ((*(b_ptr+2) == ' ') || (*(b_ptr+2) == '\0') ||
            (*(b_ptr+2) == 0xa)) || (*(b_ptr+2) == 0x0)) {
            if (temp_num = getnnum((char *)b_ptr, 16, 
				(long *)&temp_long, 2) != 2) {
                printf("*** Should process 2 ASCII digits "
                       "to get a hex number\n");
                return(FALSE);
            } else {
                cookie_contents[iy] = (uchar)temp_long;
                printf("%2x ", (uchar)temp_long);
            }
        }
        else {
            printf("ix = %d, *(b_ptr+2) = 0x%2x\n", ix, *(b_ptr+2));
            printf("\n*** Cookie input format, ex: \"04 ff c3 ... 12 ff\"\n");
            return(FALSE);
        }
#endif
    }

    /* blank out rest of cookie space */
    for (iy=digit_entered/3; iy < cookie_size; iy++) {
        cookie_contents[iy] = 0xff;
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\n");
        printf("Contents of cookie after process:\n");
        for (ix=0; ix < cookie_size; ix++) {
            if (ix%8 == 0) {
                printf("\n");
                printf("0x%08x: ", ix);
            }
            printf("0x%02x ", cookie_contents[ix]);
        }
    }

    return(TRUE);
}


/* provide the wrapper for the cookie_4.c users 
 * the default cookie size in cookie_4.c is 128 bytes. 
 */
int
cookie_4_processor (uchar *contents, int board_type,
                    int cookie_type, cli_cookie_cmd *cli_cmd) {
    return(cookie_4_processor_x(contents, board_type, 
			 cookie_type, COOKIE_SIZE_256, cli_cmd)); /* 5th params is MENU mode */
}

int
toss_cookie_4 (int board_type, int cookie_type)
{
    return(toss_cookie_4_x(board_type, cookie_type, 
			   COOKIE_SIZE_256, NULL));
}

/* end of module */

/******** History ********
$Log: cookie_4_core.c,v $
Revision 1.23  2020/08/19 09:49:17  markzha
*** empty log message ***

Revision 1.22  2019/06/14 03:58:52  mikech2
Collapse katar-branch00 to Main Trunk

Revision 1.21.22.2  2019/04/11 03:29:21  peteteng
Based on code review PRRQ#CSCvo89304-2; Remove common/src/overlord/katar folder

Revision 1.21.22.1  2018/10/31 03:55:25  peteteng
Fix Cookie type 0xC7 issue

Revision 1.21  2018/02/09 13:29:52  hondwang
Add E5 comment for Star Wifi PCBA

Revision 1.20  2018/02/09 09:11:18  hondwang
Merge Star branch star-branch-c9xx to main trunk

Revision 1.19.22.1  2018/01/20 06:29:55  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.19.6.3  2017/10/13 22:39:53  hondwang
Add E5 for Wifi PCBA info

Revision 1.19.6.2  2017/09/28 08:08:26  harrchan
Enable 0x8d cookie field

Revision 1.19.6.1  2017/09/19 02:41:22  harrchan
CSCvf92272: Add cookie type 0x8d

Revision 1.19  2017/07/28 07:49:40  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.18  2017/07/14 02:51:38  alpeng
fixed compiler warning, due to cross-compiler version was updated.

Revision 1.17  2016/04/20 07:03:32  benchen2
merge tachi_branch to maintrunk

Revision 1.16.8.1  2015/10/26 12:42:16  tirawan
Enable 0xe2/0xe3 cookie field

Revision 1.16  2015/01/08 02:50:28  erwu2
CSCur53675: Support type 0xE2/E3 cookie format, need to be verified by MFG

Revision 1.15  2014/11/12 02:59:08  erwu2
CSCur53675: Support type 0xC0 2-6-2 cookie format

Revision 1.14  2014/08/21 09:38:08  danchung
fix the bug in cookie display of control interface fields

Revision 1.13  2013/12/18 06:32:46  hroni
use toolchain in router/bin to do make with TOOLS_VER=c4.5.3-p1, TOOLS_ARCH=x86_64

Revision 1.12  2013/12/18 00:24:40  mcharon
file_exist now returns size of file

Revision 1.11  2013/11/27 10:36:30  alpeng
support /cookie.txt to store/write cookie

Revision 1.10  2013/11/26 08:40:33  hroni
fix compiler warning

Revision 1.9  2013/11/11 21:18:39  mcharon
pass string instead of number in first argum of host_send_packet ; add xaui support

Revision 1.8  2012/12/07 02:02:56  mcharon
chang default mac_blk_size from 0 to 1

Revision 1.7  2012/06/06 09:48:03  aarwang
- Clean up compiler warnings.

Revision 1.6  2012/05/17 23:20:33  shhuang
Minor change from review feedback.

Revision 1.5  2012/05/04 20:01:45  mcharon
use void* instead of int as argument to to func ptrs in ngio_intf

Revision 1.4  2012/04/20 01:10:43  shhuang
New request to display C9 subfields for NGIO attributes version 2.

Revision 1.3  2012/04/03 18:15:59  mcharon
change n_alp string DD to AA

Revision 1.2  2012/03/28 00:38:13  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:05  ptong
Initial archive of ng_diag module


$Endlog$
*/
