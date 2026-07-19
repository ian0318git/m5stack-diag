/* $Id: smart_cookie.c,v 1.23 2017/07/14 02:51:38 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/smart_cookie.c,v $
 *------------------------------------------------------------------
 * smart_cookie.c
 *
 * This file contains utilities to read/write the Renesa smart chip.
 * It also contain functions to program the digital signature on 
 * the smart chip. Originally, it was ported from IOS
 *
 * November 2003
 * Copyright (c) 2009-2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 * Author: Huan Ngo
 *------------------------------------------------------------------
 */
#include "types.h"
#include "common.h"
#include "menu.h"
#include "error.h"
#include "signals.h"
#include "mon_plat_defs.h"
#include "nvsysvars.h"
#include "queryflags.h"
#include "nmc93c46.h"
#include "pm_utils.h"
#include "cross_platform.h"
#include "cookie_plat.h"
#include "cli_cmd.h"
#include "smart_cookie.h"
#include "defs.h"
#include "smart_cookie_auth.h"
#include "cookie_4.h"
#include "slot.h"
#include "dev_print.h"
#include "proto.h"
#include "crypto_credential.h"
#include "strings.h"
#include "platform_cookie.h"
#include "ngio.h"
#include "act2_utils.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#undef NEGATIVE_TEST

/* AHSU: temp fix */
extern int act2_is_simple_mode(void *);
extern int act2_drv_write (void *module, unsigned char *send_buf, unsigned int length);
extern int act2_drv_read (void *module, unsigned char *receive_buf, unsigned int length);
extern boolean pas_pa_present(uint);
extern int plat_chassisnum_to_realnum(int);
extern int get_max_num_vm(void);
extern int mbrd_pvdm_chip_select(uint, boolean);
extern int pvdm_present(int);
extern int sm_slot_start_with (void);
extern boolean is_sm_present (int slot);
extern boolean is_ism_present (int slot);
extern int touppoer(int i);
extern int  act2_install_wdc(int);
extern int cookie_4_processor_x (uchar *contents, int board_type,
                               int cookie_type, int cookie_size, 
                               cli_cookie_cmd *);
extern char atoh(char c);
#ifdef SUDI
extern int verify_sudi(uchar *key_data_ptr, uchar *root_cert_ptr,
               uchar *subca_cert_ptr, uchar *dev_cert_ptr,
               uchar *serial_num, uchar *product_id);
#endif

#define MAX_SC_NM_IDS (sizeof(nm_sc_info_tbl) / sizeof(struct nm_sc_info))
/* define in smart_cookie_auth.c */

uchar sc_response_msg[MAX_MESSAGE_SIZE];
char sc_err_msg[MAX_MESSAGE_SIZE];
static uchar command_msg[MAX_MESSAGE_SIZE];

static uchar cm_mfg_pub_key[PUB_KEY_SIZE];
static uchar serial_number[SER_NUM_SIZE];
static uchar signature[SIGN_SIZE];
static uchar scc_lot_info[LOT_INFO_SIZE];
static uchar ctrl_ID[3];   
static uchar req_cookie_data[] = {0x00, 0x00, 0x80};
static uchar eeprom_data_save[COOKIE_DATA_SIZE];
static int imc_polling_timeout = IMC_POLLING_TIMEOUT;
static sc_context *con_gl;
static inline void assert_spi_chip_select(pas_management_t *pa);
static inline void deassert_spi_chip_select(pas_management_t *pa);
static void imc_send_raw_cmd(sc_context *con, ushort imc_cmd);
static void display_scc_return_status(scc_return_status_t status,
				       sc_context *con);
static void scc_send_2bytes(sc_context *con, ushort word);
static ushort format_double_bit(uchar byte);
static ushort unformat_double_bit(ushort rx_word);
static void update_nak_ack_resp_msg_len(uchar rx_char, ushort *resp_length);
static void scc_imc_send_data(sc_context *con, ushort word);
static void scc_send_cmd_to_imc(sc_context *con, void *send_buf,
				 ushort length);
static void scc_send_req_status_msg(sc_context *con);
ushort scc_nm_aim_read_2bytes(sc_context *con_p);
static ushort scc_read_2bytes(sc_context *con);
static void scc_send_1byte_cmd(sc_context *con, char send_char);
static void scc_flow_control_get_next(sc_context *con);
static boolean scc_req_resp_msg_from_imc(sc_context *con,
				      uchar *status_buf, ushort length);
static type_t test_get_imc_id(sc_context *con);
static scc_return_status_t scc_process_cmd(sc_context *con, void *cmd, 
				    ushort cmd_length, uchar *resp_buffer,
				    uint resp_length);
static void copy_cookie_from_rsp_msg(uchar *cookie, uchar *res_msg,
				     int length);
static type_t smart_cookie_access_test(sc_context *con);
static void print_sm_cookie_fields(sc_context *con, uchar *dig_list,
				   uchar num_byte);
static void print_sm_dig_sig_list(sc_context *con);
static int get_sm_dig_sign_list(sc_context *con, uchar *dig_list,
				 uchar *num_byte);
static int sm_write_cm_sign_cmd(sc_context *con, uchar *cm_signature);
static int sm_write_cm_pub_key_cmd(sc_context *con);
static int sm_write_cookie_sign_cmd(sc_context *con, uchar *cookie_signature);
static int sm_write_ep_tlv_info_cmd(sc_context *con, uchar *ep_tlv_info,
				    int len);
static type_t smart_cookie_write_dig_sig(sc_context *con);
static int display_sm_mfg_lot_TLV(sc_context *con);
static int display_cookie_signature(sc_context *con);
static int display_cm_pb_key_signature(sc_context *con);
static int smart_cookie_write_mfg_lockdown_area(sc_context *con);
static int smart_cookie_lockdown(sc_context *con);
static type_t smart_cookie_program_dig_sign_submenu(sc_context *con);
int smart_cookie_program_dig_sign_submenu_x(sc_context *con, boolean mode);
static int progauth_pabay(char *pa_bay);
static int progauth_pabay_slot(char *pa_bay, char *slot);
static ushort get_ctrl_id_from_cookie_contents(uchar *cookie_contents);
static boolean cookie_in_smart_access_test = FALSE;
static ushort flow_control_window_size = DF_FLOW_CONTROL_WINDOW_SIZE;
void scc_reset_nm_imc(sc_context *con);
static type_t show_scc_version(sc_context *con);
static scc_return_status_t i2c_scc_process_cmd(sc_context *con,
                                                void *cmd_buffer,
                                                ushort cmd_length,
                                                uchar *resp_buffer,
                                                uint resp_length);
scc_return_status_t send_i2c_cmd (sc_context *con, void *cmd_buffer,
                                uint cmd_length, void *resp_buffer,
                                uint resp_length);
void scc_delay_for_cmd_processing (uchar cmd_type);
static int i2c_scc_read_bytes(sc_context *con, uchar *read_buffer);
static int i2c_scc_write_bytes(sc_context *con_p, uchar *i2c_cmd, int msg_size);
static scc_return_status_t i2c_scc_process_cmd_simple(sc_context *con, void *, ushort,
                                                      uchar *, uint);
static scc_return_status_t send_i2c_cmd_simple(sc_context *, void *, uint,
                                               void *, uint);

static struct fru_platform_info fru_platform_info_tbl[] = { 
    /* add entry here if WDC should use chasis serial number
     instead of PCB serial number. */
    {"CISCO3945",        0x0613},
};

static smartchip_submenu_t smartchip_submenu_table[] = {
   {"program & authenticate digital signature", 
    smart_cookie_program_dig_sign_submenu, (type_t *)&con_gl, 
       MF_CONTINUOUS,                 (type_t(*)())0, 0, (type_t(*)())0, 0},
   {"authenticate smart chip Screen", smart_cookie_authenticate_retest,
       (type_t *)&con_gl,
       MF_CONTINUOUS | MF_DOALL,      (type_t(*)())0, 0, (type_t(*)())0, 0},
   {"echo test",                    smart_cookie_echo_test, (type_t *)&con_gl,
       MF_CONTINUOUS | MF_DOALL,      (type_t(*)())0, 0, (type_t(*)())0, 0},
   {"read/write cookie test",       smart_cookie_access_test, (type_t *)&con_gl,
       0,                             (type_t(*)())0, 0, (type_t(*)())0, 0},
   {"reset IMC",                    reset_imc, (type_t *)&con_gl,
       0,                             (type_t(*)())0, 0, (type_t(*)())0, 0},
   {"show IMC HW Rev, SW Ver",      test_get_imc_id, (type_t *)&con_gl,
       0,                             (type_t(*)())0, 0, (type_t(*)())0, 0},
   {"authenticate smart chip",      smart_cookie_authenticate, (type_t *)&con_gl,
       MF_CONTINUOUS | MF_DOALL,      (type_t(*)())0, 0, (type_t(*)())0, 0},
   {"show SCC Version & Type",      show_scc_version, (type_t *)&con_gl,
       0,                             (type_t(*)())0, 0, (type_t(*)())0, 0},
#ifdef NEGATIVE_TEST
   {"authenticate force error",     smart_cookie_auth_force, (type_t *)&con_gl,
       MF_CONTINUOUS | MF_DOALL,      (type_t(*)())0, 0, (type_t(*)())0, 0},
#endif
}; 
#define SMART_COOKIE_SUBMENU_SIZE (sizeof(smartchip_submenu_table) / \
                                  sizeof(submenu_xtable_t))

/* 
 * Primary smart cookie submenu items (filled in from xtable)
 */
static mitem_t smartchip_submenu_primary_items[SMART_COOKIE_SUBMENU_SIZE +
    MAX_BASE_ITEMS];
menuinfo_t smart_cookie_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    smartchip_submenu_primary_items,
};
menuinfo_t *smart_cookie_submenup = &smart_cookie_subtest_menu;

#ifdef DYNO
boolean is_sm_present (int slot)
{
    return FALSE;
}
int    slot_start_with(void)
{

}
#endif

/*-------------------------------------------------------------------
 *
 * Function: show_smart_cookie_submenu()
 *
 * First build the submenu for the smart cookie diags based on the
 * smartchip_submenu_t.  Because add_menu_item() ignores the menu
 * existence function and parameter, copy those out of the
 * corresponding smartchip_submenu_t element.  Then display the menu to
 * the user for interaction.
 */
int
show_smart_cookie_submenu (void)
{
    unsigned int i;
    mitem_t *pi;
    menuinfo_t *pm = smart_cookie_submenup;
    smartchip_submenu_t *px = smartchip_submenu_table;
    init_base_submenu(&smart_cookie_submenup, (type_t)"Smart Cookie");
    pi = pm->miptr + pm->msize;  /* initially at next item after base items */
    for (i = 0; i < SMART_COOKIE_SUBMENU_SIZE; i++, px++, pi++) {
        add_menu_item(smart_cookie_submenup, px->x_title, px->x_pfunc,
                      px->x_pparam, px->x_flags);
        pi->mixfunc = px->x_xfunc;
        pi->mixparam = px->x_xparam;
    }
    menu(smart_cookie_submenup, '\0', '\0' );
    return(PASSED);
}
    
struct mitem smartchip_debug_subdiag[] = {
    {"reset IMC",                 0, 0, 0,(type_t *) 0, 0,(type_t(*)())0, 0},
    {"show IMC HW Rev, SW Ver",   0, 0, 0,(type_t *) 0, 0,(type_t(*)())0, 0}
};
static struct mitem smartchip_debug_maindiag[] = {
    {"reset IMC",                 0, 0, reset_imc,
    (type_t *)&con_gl, 0,(type_t(*)())0, 0}, 
    {"show IMC HW Rev, SW Ver",   0, 0, test_get_imc_id,
    (type_t *)&con_gl, 0,(type_t(*)())0, 0}
}; 
struct menuinfo smartchipdebugmaindiag = {
    "Smart Cookie Debug Menu",                     /* title */
    (type_t)0 ,                              	   /* title param */
    (PFT)menu_show_dflags,                         /* show diag flags */
    0,                                             /* generic prompt */
    sizeof(smartchip_debug_maindiag)/sizeof(struct mitem), /* size of menu */
    smartchip_debug_maindiag,
};
/*------------------------------------------------------------------------    
 * assert_spi_chip_select
 *
 * Description:
 *   This function is used to assert the chip select
 *
 * Parameters:
 *    pa - pointer to pas_management_t
 *
 * Returns:
 *   None
 *--------------------------------------------------------------------------*/ 
static inline void
assert_spi_chip_select(pas_management_t *pa)
{
    pas_eeprom_select(pa, TRUE);           /* Assert chip select */
}
/*------------------------------------------------------------------------    
 * deassert_spi_chip_select
 *
 * Description:
 *   This function is used to deassert the chip select
 *
 * Parameters:
 *    pa - pointer to pas_management_t
 *
 * Returns:
 *   None
 *--------------------------------------------------------------------------*/ 
   
static inline void
deassert_spi_chip_select (pas_management_t *pa)
{    
    assert(!"NOT Supported\n");
}
/*------------------------------------------------------------------------    
 * imc_nm_aim_send_raw_cmd
 *
 * Description:
 *   This function is used to send a raw command to IMC
 *
 * Parameters:
 *    con - pointer to sc_context
 *    imc_cmd - Command to IMC   
 *
 * Returns:
 *   None
 *--------------------------------------------------------------------------*/ 
static void
imc_send_raw_cmd (sc_context *con, ushort imc_cmd)
{
    union spi_cmd_u spi_cmd;
    /*
     * Send the Command word
     */
    spi_cmd.spi_cmd_str.spi_msg_id  = imc_cmd;
    spi_cmd.spi_cmd_str.msg_cntl = 1;
    spi_cmd.spi_cmd_str.data = 0;
    scc_imc_send_data(con, spi_cmd.spi_cmd_word);
}
/*------------------------------------------------------------------------
 * imc_send_command
 *
 * Description:
 *   This function ia a main routine to send a command to IMC and
 *   read back the response message.
 *
 * Parameters:
 *   con         - Pointer to sc_context
 *   command     - Command to be sent to IMC
 *   resp_buffer - Pointer to the response message buffer
 *   resp_length - Expected length the response message *
 *
 * Returns:
 *   True - if IOS is successfully retrieved the response message;
 *          otherwise, return FALSE
 *--------------------------------------------------------------------------*/
void
imc_send_command(sc_context *con, message_to_imc command,
		 void *resp_buffer, uint resp_length)
{
    uchar *rx_buff_ptr = resp_buffer;
    ushort rx_word;
    switch (command) {
    case IMC_SCC_RESET:
	imc_send_raw_cmd(con, IMC_SPI_CMD_SCC_RESET);
	break;
    case IMC_GET_IMC_HW_VER:
	imc_send_raw_cmd(con, IMC_SPI_CMD_HW_REV);
	break;
    case IMC_GET_IMC_SW_VER:
	imc_send_raw_cmd(con, IMC_SPI_CMD_SW_REV);
	break;
    case IMC_RESYNC:
        /*
         * We tread the Re-Sync Comamnd special
         */
      	scc_imc_send_data(con, IMC_SPI_RESYNC);
	return;
    case IMC_POWER_ON:
    case IMC_POWER_OFF:
        /*
         * Mar platforms don't support Power On/Off commands
         */
	return;
    default:
	return;
    }
    /*
     * Read the Response Message
     */
    while (resp_length > 0) {
	rx_word = scc_read_2bytes(con);
	rx_word = unformat_double_bit(rx_word);  
        *rx_buff_ptr = rx_word;
	rx_buff_ptr++;
        resp_length--;
	smart_cookie_delay(DELAY_BETWEEN_READ_CYCLE);
    }
    return;
}
/*------------------------------------------------------------------------
 * reset_IMC
 *
 * Description:
 *   This is function to reset IMC
 *   To reset IMC, there must be a delay of 100usec after chipselect
 *   and 200usec after deselect
 * 
 * Parameters:
 *   con - pointer to sc_context
 *
 * Returns:
 *   None
 *----------------------------------------------------------------------*/
type_t
reset_imc(sc_context *con)
{    
    con->quack_reset(con); 
    msleep(SCC_RESET_DELAY);
    return PASSED;
}
/*------------------------------------------------------------------------
 * Function: quack_verion
 *
 * Description:
 *   This function returns the quack version on the SCC
 *
 * Parameters:
 *   con - pointer to sc_context
 *
 * Returns:
 *   The quack version on SCC
 *----------------------------------------------------------------------*/
int
quack_version(sc_context *con)
{
    int version;
    /* 
     * before calling quack_version(), codes should call
     * is_smart_eeprom() first to make sure the module has quack.
     */
    if (send_command_to_smart_cookie(con, REQUEST_SCC_ID,
				     NULL, 0)) {
	return SCC_ILL_VERSION;
    }	
    version = sc_response_msg[5];

    return version;
}
/*------------------------------------------------------------------------
 * Function: quack_type
 *
 * Description:
 *   This function returns the quack type on the SCC
 *   1st nibble is vendor ID (for Renesas = 0, Atmel = 1)
 *   2nd nibble is device type (for example, 1 is AT128RCT36)
 *   To learn more, contact Frank Fang (fcfang)
 *
 * Parameters:
 *   con - pointer to sc_context
 *
 * Returns:
 *   The quack type of SCC
 *----------------------------------------------------------------------*/
int
quack_type(sc_context *con)
{
    int type;
    /* 
     * before calling quack_type(), codes should call
     * is_smart_eeprom() first to make sure the module has quack.
     */
    if (send_command_to_smart_cookie(con, REQUEST_SCC_ID,
				     NULL, 0)) {
	return SCC_ILL_VERSION;
    }	
    type = sc_response_msg[4];
    return type;
}
/*------------------------------------------------------------------------
 * Function: check_quack_version
 *
 * Description:
 *   This function determines the module has the new (3 or above) or
 * old (2) or illegal quack version. If is has the old version, it'll 
 * check whether this module is allowed to support the old quack version.
 *
 * Parameters:
 *   con - pointer to sc_context
 *
 * Returns:
 *   PASSED: if the quack version on this module is supported
 *   FAILED: if the quack version on this module is not supported
 *----------------------------------------------------------------------*/
int
check_quack_version(sc_context *con)
{
    int version, type;
    /* 
     * before calling check_quack_version(), is_smart_eeprom()
     * should be called first to make sure the module has quack
     */
    version = quack_version(con);
    switch (version) {
    case SCC_ILL_VERSION:
	cterr('f',0,"%s Can't read Smart Chip version", con->info_string);
	return FAILED;
    case 0:
    case 1:
    case SCC_1_VERSION:
	type = quack_type(con);
	if (type == 0x01) { /* Renesa, AT128RCT36 */
	    cterr('f',0,"%s has unsupported Smart Chip. "
		  "Please replace Smart Chip.", con->info_string);
	    return FAILED;
	}
	break;
    default:
	break;
    }
    return PASSED;    
}
/*------------------------------------------------------------------------
 * Function: show_scc_version
 *
 * Description:
 *   This function shows the quack version on the SCC
 *
 * Parameters:
 *   con - pointer to sc_context
 *
 * Returns:
 *   None
 *----------------------------------------------------------------------*/
static type_t
show_scc_version(sc_context *con)
{
    int version, type;
    /* 
     * before calling check_quack_version(), is_smart_eeprom()
     * should be called first to make sure the module has quack
     */
    version = quack_version(con);
    printf("\n SCC Version:  0x%02x", version);
    type = quack_type(con);
    printf("\n SCC Type   :  0x%02x", type);
    return PASSED;
}    
/*-----------------------------------------------------------------------------
 *  NAME: calculate_cksum
 *
 *  DESCRIPTION:
 *   Return checksum for the specified chunk of data
 *
 *  PARAMETERS:
 *    p - pointer to data
 *    count - total number of bytes
 *
 *  RETURNS:
 *      Checksum calculated
 *---------------------------------------------------------------------------*/
uchar
calculate_cksum(uchar *p, int count)
{
    uchar cksum, *data;
    ushort i;
    data = p;
    cksum = 0;
    for (i = 0; i < count; i++)
        cksum += *data++;
    return (~cksum);
}
/*------------------------------------------------------------------------
 * display_scc_return_status
 *
 * Description:
 *   This function displays the status
 *  
 *
 * Parameters:
 *   status      - scc_return_status
 *   con         - pointer to sc_context
 *
 * Returns:
 *   None
 *----------------------------------------------------------------------*/
static void
display_scc_return_status(scc_return_status_t status,
			  sc_context *con)
{
    switch (status){
    case SCC_OK:
        printf("\n    PASS:SCC_OK");
	break;
    case SCC_NAK:
        printf("\n***    FAIL:SCC_NAK");
	break;
    case SCC_INVALID_SLOT_ID:
        printf("\n***    FAIL:SCC_INVALID_SLOT_ID, type = %d, slot = %d\n",
	       con->type, con->slot);
	break;
    case SCC_CARD_NOT_PRESENT:
        printf("\n***    FAIL:SCC_CARD_NOT_PRESENT, type = %d, slot = %d\n",
	       con->type, con->slot);
	break;
    case SCC_CARD_POWER_DOWN:
        printf("\n***    FAIL:SCC_CARD_POWER_DOWN, type = %d, slot = %d\n",
	       con->type, con->slot);
	break;
    case SCC_TIMEOUT:
        printf("\n***    FAIL:SCC_TIMEOUT");
	break;
    default:
        printf("\n***    FAIL:Invalid Return Status,type = %d, slot = %d\n",
	       con->type, con->slot);
	break;
    }
}
/*------------------------------------------------------------------------
 * scc_reset_nm_imc
 *
 * Description:
 *   This function implementes a reset to IMC by generating a
 *   reset via CS line.
 *   A empty CS but sending no data to the IMC would reset the
 *   IMC
 *
 * Parameters:
 *   con    - pointer to sc_context
 *
 * Returns:
 *   M/A 
 *--------------------------------------------------------------------------*/
void
scc_reset_nm_imc(sc_context *con)
{
    assert(!"\nnot supported\n");
}

/*------------------------------------------------------------------------
 * scc_nm_aim_send_2bytes
 *
 * Description:
 *   Send 16 Bits to the Bit-Bang interface - MSB first
 *
 * Parameters:
 *   con_p - Pointer to sc_context
 *   word - data to be sent to the the Bit-Bing interface
 *
 * Returns:
 *   None
 *--------------------------------------------------------------------------*/
void
scc_nm_aim_send_2bytes(sc_context *con_p, ushort *word)
{
    assert(!"\nnot supported.\n");
}

/*------------------------------------------------------------------------
 * scc_send_2bytes
 *
 * Description:
 *   Send 2 bytes of data to smart cookie.
 *
 * Parameters:
 *   con  - context pointer
 *   word - data to be sent to the the Bit-Bing interface
 *   For send 2 bytes and read 2 bytes, there must be 100usec delay
 *   after select the chip, 50usec after read or write 2 bytes and
 *   250usec after deselect the chip
 *
 * Returns:
 *   None
 *--------------------------------------------------------------------------*/
static void
scc_send_2bytes(sc_context *con_p, ushort word)
{
    con_p->quack_write_2bytes(con_p, (char *)&word, sizeof(ushort)); 
}
/*------------------------------------------------------------------------
 * format_double_bit
 *
 * Description:
 *   This funtion is used to format Double-Bit feature as
 *   a requirement of IMC. The input is a 8-bits and
 *   the output is 16-bits. For each bit of the input data
 *   it will be double for the output data.
 *   i.e.  Input: 0x24     or b0010 0100
 *         Output:0x0C30   or b0000 1100 0011 0000
 *
 * Parameters:
 *   byte - a 8-bit input data requires to be formatted
 * Returns:
 *   a 16-bit data
 *--------------------------------------------------------------------------*/
static ushort
format_double_bit(uchar byte)
{
    ushort word = 0;
    uchar bit, i;
    
    for (i = 0; i < 8; i++) {
	bit = (byte >> (7 - i)) & 0x1;
	word = (word << 1) | bit;
	word = (word << 1) | bit;
    }
    return word;
}  
/*------------------------------------------------------------------------
 * unformat_double_bit
 *
 * Description:
 *   This function extracts a byte from double bit format word
 *   i.e   Input: 0x0C30   or b0000 1100 0011 0000
 *   i.e.  Output:0x0024   or b0000 0000 0010 0100
 *
 * Parameters:
 *    ushort rx_word - input double bit format word
 *
 * Returns:
 *    unpacked byte in lower byte of short
 *--------------------------------------------------------------------------*/
static ushort
unformat_double_bit(ushort rx_word)
{ 
    ushort unpacked = 0;
    int i;
    char bit;
 
    for (i = 0; i < 8; i++) {
	bit = (rx_word >> (i<<1)) & 1;
	bit = bit << i;
	unpacked |= bit;
    }
    return (unpacked & 0x00FF);
}
/*------------------------------------------------------------------------
 * update_nak_ack_resp_msg_len
 *    
 * Description:
 *   Adjust the Respose Message Length if required
 *
 * Parameters:
 *   rx_char     - Received Char
 *   resp_length - Pointer to the Response Message Length
 *
 * Returns:
 *   None
 *--------------------------------------------------------------------------*/
static void
update_nak_ack_resp_msg_len (uchar rx_char, ushort *resp_length)
{
    if ((rx_char == COMMAND_ACK) ||
        (rx_char == COMMAND_NACK_CRC_ERR) ||
        (rx_char == COMMAND_NACK_INV_MSG_TYPE) ||
        (rx_char == COMMAND_NACK_INV_MSG_LEN) ||
        (rx_char == COMMAND_NACK_INV_EEPROM_ACCESS) ||   
        (rx_char == COMMAND_NACK_MSG_LEN_EXCEED) ||
        (rx_char == COMMAND_NACK_INV_CHIP_TYPE) ||
        (rx_char == COMMAND_NACK_INV_OVERWRT_SPARE) ||
        (rx_char == COMMAND_NACK_SIGN_NOT_ALLOW) ||
        (rx_char == COMMAND_NACK_PIN_SET_INCOMPLETE)) {
        *resp_length = COMMAND_ACK_NACK_SIZE;
    }
}
/*------------------------------------------------------------------------
 * scc_imc_send_data
 *
 * Description:
 *   Format 16-Bits word into Double-Bit format and send it
 *   to the Bit-Bing interface.
 *
 * Parameters:
 *   con  - pointer to sc_context
 *   word - data to be sent to the the Bit-Bing interface
 *
 * Returns:
 *   None
 *--------------------------------------------------------------------------*/
static void
scc_imc_send_data(sc_context *con, ushort word)
{
    /*
     * Send Low Byte
     */
    scc_send_2bytes(con, format_double_bit(word & 0xff));
    smart_cookie_delay(DELAY_AFTER_2BYTES_CMD);
    /*
     * Send High Byte
     */
    scc_send_2bytes(con, format_double_bit(word >> 8));
}
/*------------------------------------------------------------------------
 * scc_send_cmd_to_imc
 *
 * Description:
 *   This function takes a SCC's command string, formats the SCC's 
 *   command string into a set of 16-Bits word per requirements of IMC's
 *   protocol messages, and then send them to the Bit-Bang interface
 *
 * Parameters:
 *   con      - pointer to sc_context
 *   send_buf - Pointer to the sending buffer
 *   length   - Length of the sending data
 *
 * Returns:
 *   None
 *--------------------------------------------------------------------------*/
static void
scc_send_cmd_to_imc(sc_context *con, void *send_buf, ushort length)
{
    int i;
    uchar *buf = (uchar *)send_buf;
    union spi_cmd_u spi_cmd;
    /*
     * Send the 1st word
     */
    spi_cmd.spi_cmd_str.spi_msg_id  = IMC_SPI_CMD_START;
    spi_cmd.spi_cmd_str.msg_cntl = length;
    spi_cmd.spi_cmd_str.data = *buf++;
    //    printf("smart cookie  %d; %d\n", length, __LINE__); /*DEBUG_ZZZ*/
    scc_imc_send_data(con, spi_cmd.spi_cmd_word);
    smart_cookie_delay(DELAY_AFTER_1_SCC_CMD);
    /*
     * Send the nth word
     */
    for (i = 2; i <= length; i++) {
        if (i == length) {
	    spi_cmd.spi_cmd_str.spi_msg_id = IMC_SPI_CMD_END;
        } else {
	    spi_cmd.spi_cmd_str.spi_msg_id = IMC_SPI_CMD_CONTD;
        }
        spi_cmd.spi_cmd_str.msg_cntl = i;
        spi_cmd.spi_cmd_str.data = *buf++;
        scc_imc_send_data(con, spi_cmd.spi_cmd_word);
        smart_cookie_delay(DELAY_AFTER_1_SCC_CMD);
    }
}   
/*------------------------------------------------------------------------
 * scc_send_req_status_msg
 * 
 * Description:
 *   This function is used to send a "Req-Data" command to the IMC
 *
 * Parameters:
 *   con - pointer to sc_context
 *         
 * Returns:
 *    None
 *--------------------------------------------------------------------------*/
static void
scc_send_req_status_msg(sc_context *con)
{
    union spi_cmd_u spi_cmd;
    /* 
     * Send the Command word
     */
    spi_cmd.spi_cmd_str.spi_msg_id  = IMC_SPI_CMD_DREQ;
    spi_cmd.spi_cmd_str.msg_cntl = 1;
    spi_cmd.spi_cmd_str.data = 0x0;
    scc_imc_send_data(con, spi_cmd.spi_cmd_word);
    smart_cookie_delay(DELAY_AFTER_1_IMC_CMD);
}
/*------------------------------------------------------------------------
 * scc_nm_aim_read_2bytes
 *
 * Description:
 *   Read 16-Bits from the Bit-Bang interface
 *
 * Parameters:
 *   con_p   - Pointer to sc_context
 *
 * Returns:
 *   16-Bits data which read from the Bit-Bang interface
 *--------------------------------------------------------------------------*/
ushort
scc_nm_aim_read_2bytes(sc_context *con_p)
{
    return 0;
}

/*------------------------------------------------------------------------
 * scc_read_2bytes
 *
 * Description:
 *   Read 16-Bits from spi interface.
 *
 * Parameters:
 *   con - sc_context pointer
 *   For send 2 bytes and read 2 bytes, there must be 100usec delay
 *   after select the chip, 50usec after read or write 2 bytes and
 *   250usec after deselect the chip
 *
 * Returns:
 *   16-Bits data read from spi interface.
 *--------------------------------------------------------------------------*/
static ushort
scc_read_2bytes(sc_context *con)
{
    ushort rx_word  = 0;
    rx_word = con->quack_read_2bytes(con);
#ifdef DEBUG_INF_COOKIE
    printf("\n scc_read_2bytes(): %d", rx_word);
#endif
    return rx_word;
}

/*------------------------------------------------------------------------
 * scc_send_1byte_cmd
 *
 * Description:
 *  This function is used to send "1-Byte" command to SCC
 *
 * Parameters:
 *    con       = pointer to sc_context
 *    send_char - Character to be send to the IMC
 *
 * Returns:
 *    None
 *--------------------------------------------------------------------------*/
static void
scc_send_1byte_cmd(sc_context *con, char send_char)
{
    union spi_cmd_u spi_cmd;
    /* 
     * Send the Command word
     */
    spi_cmd.spi_cmd_str.spi_msg_id  = IMC_SPI_CMD_START;
    spi_cmd.spi_cmd_str.msg_cntl = 1;
    spi_cmd.spi_cmd_str.data = send_char;
    scc_imc_send_data(con, spi_cmd.spi_cmd_word);
}
    
/*------------------------------------------------------------------------
 * scc_flow_control_get_next
 *
 * Description:
 *   This function is used to send a "Get-Next" command to SCC
 *
 * Parameters:
 *    con - pointer to sc_context
 *
 * Returns:
 *    None
 *--------------------------------------------------------------------------*/
static void
scc_flow_control_get_next(sc_context *con)
{
    scc_send_1byte_cmd(con, SCC_FLOW_CONTROL_GET_NEXT);
    smart_cookie_delay(DELAY_AFTER_SCC_1_BYTE_CMD);
}
/*------------------------------------------------------------------------
 *scc_req_resp_msg_from_imc 
 *
 * Description:
 *   This function is used to read response message from the SCC via IMC.
 *   IOS sends out a "Req-Data" message to IMC.  If IMC has data, it
 *   will response with "01" and follow by the actual data.  Otherwise,
 *   IMC will put "0xff" to notify the IOS there is no data available.  
 *
 * Parameters:
 *   con        - pointer to sc_context
 *   status_buf - Pointer to the response message buffer
 *   length     - Expected length the response message
 *
 * Returns:
 *   Pass - if successfully retrieved the response message;
 *          otherwise, return Failed
 *--------------------------------------------------------------------------*/
static int
scc_req_resp_msg_from_imc(sc_context *con, uchar *status_buf, ushort length)
{
    ushort rx_word, rx_word1;
    uchar *rx_buff_ptr = (uchar *)status_buf;
    ushort resp_length = length;
    int rx_ctr = 0;
    int timeout_ctr = 0;
    do {
	scc_send_req_status_msg(con);
	rx_word = scc_read_2bytes(con);
	smart_cookie_delay(DELAY_BETWEEN_READ_CYCLE);
	rx_word = unformat_double_bit(rx_word >> 8);
	if (rx_word == IMC_SPI_DATA_VALID) {
	    /* Read Actual Data */
	    /*
	     * Read double packed upper nibble
	     */
	    rx_word = scc_read_2bytes(con);
	    smart_cookie_delay(DELAY_BETWEEN_READ_CYCLE); 
	    /*
	     * Read double packed lower nibble
	     */
	    rx_word1 = scc_read_2bytes(con);
	    smart_cookie_delay(DELAY_BETWEEN_READ_CYCLE);
	    /*
	     * Extract the word
	     */
	    rx_word =  rx_word  & 0xFF00;  /* Get high Nibble */
	    rx_word |= (rx_word1 & 0xFF00) >> 8;  /* Get low nibble */
	    rx_word = unformat_double_bit(rx_word);
	    *rx_buff_ptr = (uchar)(rx_word);
	    /*
	     * Check for NAK Messages.
	     * If NAK Message is received, then the Response Message
	     * length is 5
	     */
	    if (rx_ctr == 0) {
                update_nak_ack_resp_msg_len(*rx_buff_ptr, &resp_length);
	    }
	    rx_buff_ptr++;
	    rx_ctr++;
	    if (((rx_ctr % flow_control_window_size) == 0)
		&& (rx_ctr != resp_length)){
		scc_flow_control_get_next(con);
	    }
	    /*
	     * Reset the timeout counter
	     */
	    timeout_ctr = 0;
	}
	timeout_ctr++;
    } while ((rx_ctr < resp_length) && (timeout_ctr < imc_polling_timeout));
    if (timeout_ctr >= imc_polling_timeout) {
        msleep(10); // Allow device to settle after failed access
        return FAILED;
    }
    return PASSED;
}
/*------------------------------------------------------------------------
 * scc_process_cmd
 *
 * Description:
 *   This is a main function to send SCC command to the SCC via IMC
 *   for PA/NM and AIM interface
 *
 * Parameters:
 *   cmd         - Pointer to the command bufer to be sent to SCC
 *   cmd_length  - Length of the command
 *   resp_buffer - Pointer to the response message buffer
 *   resp_length - Expected length the response message
 *
 * Returns:
 *   Sending status of the message
 *--------------------------------------------------------------------------*/
static scc_return_status_t
scc_process_cmd(sc_context *con, void *cmd,
                ushort cmd_length, uchar *resp_buffer, uint resp_length)
{
    uchar *scc_cmd = (uchar *)cmd;
    scc_return_status_t status = 0;
    if (*scc_cmd == REQUEST_SCC_ID) {
        imc_polling_timeout = SCC_ID_CMD_POLLING_TIMEOUT;
    } else {
        imc_polling_timeout = IMC_POLLING_TIMEOUT;
    }
    /*
     * Send the command to SCC
     */
#ifdef COOKIE_DEBUG
    { // KFS
        int32_t i=0,len=cmd_length;
        printf ("\ncmd_length=%d", len);
        printf ("\nCommand=");
        while (len--) {
            printf ("%02x ", scc_cmd[i++]);
        }
        printf ("\n");
    }
#endif
#ifdef COOKIE_DEBUG
    printf("scc_process_cmd() con->dev_if_p->interface = %d\n",
                                                con->dev_if_p->interface);
#endif
    switch (con->dev_if_p->interface) {
        case SCC_IMC_IF:
            scc_send_cmd_to_imc(con, cmd, cmd_length);
            break;
        case SCC_DUART_IF:
            con->quack_write_2bytes(con, cmd, cmd_length);
            break;
        case SCC_I2C_IF:
            status = i2c_scc_process_cmd(con, cmd, cmd_length, resp_buffer,
                                         resp_length);
            if (status != SCC_OK) {
                return status;
            }
            break;
        default:
            printf("Invalid SCC interface. You forgot to initialize SCC IF\n");
            break;
    }
    /*
     * Wait for SCC to process the command
     */
    smart_cookie_delay(WAITING_FOR_REPLY_MSG);
    /*
     * Retrieve the Response Messsage from SCC
     */
    switch (con->dev_if_p->interface) {
        case SCC_IMC_IF:
            if (!scc_req_resp_msg_from_imc(con, resp_buffer, resp_length)) {
                return SCC_OK;
            }
            break;
        case SCC_DUART_IF:
            if (!con->quack_read_2bytes(con,resp_buffer, resp_length)) {
                return SCC_OK;
            }
            break;
        case SCC_I2C_IF:
            return status;
            break;
        default:
            printf("Invalid SCC interface. You forgot to initialize SCC IF\n");
            break;
    }
    return SCC_TIMEOUT;
}
  
/*-----------------------------------------------------------------------  
 * test_get_imc_id
 *
 * Description:
 *   get hardware revision and software revision of imc.
 *
 *   useful to determine if we can access the imc when there
 *   is a problem with smart cookie access (but this must
 *   be called outside of the smart cookie submenu.  The
 *   way the code is currently structured, if we are unable
 *   to access the smart cookie, then this function is not
 *   reachable since the smart cookie submenu is not displayed.).
 *
 * Parameters:
 *   con - context pointer
 *
 * Returns:
 *   imc id
 *-----------------------------------------------------------------------*/
type_t
test_get_imc_id(sc_context *con)
{
    uint32_t dev_id;
    assert(con);
    (void)imc_send_command(con,	IMC_GET_IMC_HW_VER, (uchar *)&dev_id, 1);
    dev_id = (dev_id & 0x0f000) >> 12 ;
    printf("\n IMC Hardware Revision: 0x%02x", dev_id);

    (void)imc_send_command(con, IMC_GET_IMC_SW_VER, (uchar *)&dev_id, 1);

    dev_id = (dev_id & 0x0f000) >> 12;
    printf("\n IMC Software Version:  0x%02x", dev_id);
    return dev_id;
}
/*---------------------------------------------------------------------------
 * NAME: send_command_to_smart_cookie
 *
 * DESCRIPTION:
 *  This function will send a command to a smart cookie 
 * 
 * PARAMETERS:
 *     con - sc_context pointer
 *     type  - Message ID
 *     data - pointer to data
 *     data_length - length of data to be sent
 *
 * RETURNS:
 *     PASSED - if successful
 *     FAILED - if unsuccessful
 *--------------------------------------------------------------------------*/
int
send_command_to_smart_cookie(sc_context *con, char type, uchar *data, 
			     uint data_length)
{
    uchar *ptr = command_msg;
    uint resp_len;
    uchar resp_type;
    boolean single_byte_cmd = FALSE;
    unsigned int i;
    scc_return_status_t send_status;
    switch (type) {
    case GET_SCC_ID:
        single_byte_cmd = TRUE;
        /* fall through */
    case REQUEST_SCC_ID:
        resp_len = RESPONSE_SCC_ID_SIZE;
        resp_type = RESPONSE_SCC_ID;
        break;
    case GET_COOKIE_DATA_64B:
        single_byte_cmd = TRUE;
        resp_len = RESPONSE_COOKIE_DATA_SIZE_64B;
        resp_type = RESPONSE_COOKIE_DATA;
        break;
    case GET_COOKIE_DATA_128B:
        single_byte_cmd = TRUE;
        /* fall through */
    case REQUEST_COOKIE_DATA:
        resp_len = RESPONSE_COOKIE_DATA_SIZE;
        resp_type = RESPONSE_COOKIE_DATA;
        break;
    case REQUEST_SN_SCMFG_PUBKEY_SIGN_1:
        resp_len = RESPONSE_SN_SCMFG_PUBKEY_SIGN_1_SIZE;
        resp_type = RESPONSE_SN_SCMFG_PUBKEY_SIGN_1;
        break;
    case REQUEST_VL_DEV_PUBKEY_SIGN_2:
        resp_len = RESPONSE_VL_DEV_PUBKEY_SIGN_2_SIZE;
        resp_type = RESPONSE_VL_DEV_PUBKEY_SIGN_2;
        break;
    case REQUEST_CNTMFG_PUBKEY_SIGN_3:
        resp_len = RESPONSE_CNTMFG_PUBKEY_SIGN_3_SIZE;
        resp_type = RESPONSE_CNTMFG_PUBKEY_SIGN_3;
        break;
    case REQUEST_CK_SIGN_4:
        resp_len = RESPONSE_CK_SIGN_4_SIZE;
        resp_type = RESPONSE_CK_SIGN_4;
        break;
    case REQUEST_SIGN_MESSAGE:
        resp_len = RESPONSE_SIGN_MESSAGE_SIZE;
        resp_type = RESPONSE_SIGN_MESSAGE;
        break;
    case REQUEST_SIGN_MSG_DIGEST:
        resp_len = RESPONSE_SIGN_MESSAGE_SIZE;
        resp_type = RESPONSE_SIGN_MSG_DIGEST;
        break;
    case REQUEST_SIGN_MESSAGE_32B:
        resp_len = RESPONSE_SIGN_MESSAGE_32B_SIZE;
        resp_type = RESPONSE_SIGN_MESSAGE_32B;
        break;
    case REQUEST_PUBKEY_N_CERT:
        resp_len = RESPONSE_PUBKEY_N_CERT_SIZE;
        resp_type = RESPONSE_PUBKEY_N_CERT;
        break; 
    case REQUEST_SIGNATURE:
        resp_len = RESPONSE_SIGNATURE_SIZE;
        resp_type  = RESPONSE_SIGNATURE;
        break;
    case REQUEST_LOT_INFO:
        resp_len = RESPONSE_LOT_INFO_SIZE;
        resp_type  = RESPONSE_LOT_INFO;
        break;
    case ECHO_REQUEST:
        resp_len = data_length + HEADER_SIZE;
        resp_type = ECHO_REPLY;
	break;
    case SET_SIGN_PIN_N_LIMIT_COUNT:
    case SET_PIN:
    case CHANGE_SIGN_LIMIT_COUNT:
    case REFRESH_SIGN_LIMIT_COUNT:
    case CHANGE_PIN:
    case WRITE_SPARE:
    case SMART_EEPROM_WRITE:
        resp_len = COMMAND_ACK_NACK_SIZE;
        resp_type = COMMAND_ACK;
        break;
    case SMART_EEPROM_READ:
        if (act2_is_simple_mode(con)) {
            resp_len = con->dev_if_p->cookie_size;
        } else {
            resp_len = data_length + HEADER_SIZE;
        }
        resp_type = EEPROM_READ_RESPONSE;
        break;
    case GET_SIGN_LIMIT_COUNT:
        resp_len = RETURN_SIGN_LIMIT_COUNT_SIZE;
        resp_type = RETURN_SIGN_LIMIT_COUNT;
        break;
    case EEPROM_PAGE_LOCK_DOWN:
        resp_len = COMMAND_ACK_NACK_SIZE;  
        resp_type = COMMAND_ACK;
        break;    
    default:
        resp_len = HEADER_SIZE;
	resp_type = 0;
        break;
    }
    for (i = 0; i < MAX_MESSAGE_SIZE; i++) {
	sc_response_msg[i] = 0x00;
    }
     
    if (single_byte_cmd) {
	command_msg[0] = type;
    } else {
	*ptr++ = type;
	*ptr++ = 0;
	*ptr++ = data_length;
	for (i = 0; i < data_length; i++) 
	    *ptr++ = *data++;
	command_msg[data_length+3] = 0;
	*ptr = calculate_cksum(command_msg, data_length + 3);
    }
    
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\nCommand\n");
	for (i = 0; i<(uint)(((sm_message_t *)command_msg)->length+HEADER_SIZE);i++) {
	    if ((i % 16) == 0)
	        printf("\n");
	    printf("%02X ", command_msg[i]);
	}
    }
    send_status = scc_process_cmd(con, command_msg, 
			       single_byte_cmd ? 1 : data_length + HEADER_SIZE,
			       sc_response_msg, resp_len);

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\nResponse\n");
	for (i = 0; i<(uint)(((sm_message_t *)sc_response_msg)->length+HEADER_SIZE);i++) {
	    if ((i % 16) == 0)
	        printf("\n");
	    printf("%02X ", sc_response_msg[i]);
	}
	printf("\n");
    }
    if (send_status != SCC_OK) {
	 if ((NVRAM)->diagflag & D_VERBOSE) {
	     display_scc_return_status(send_status, con);
	 }
	 sprintf(sc_err_msg, "SCC_OK failed");
	 return FAILED;
    }
    /* check the checksum of the response */

    if (calculate_cksum(sc_response_msg,
			((sm_message_t *)sc_response_msg)->length + 
			HEADER_SIZE)) {
        printf("smart_cookie.c calculate check sum failed @%d\n", __LINE__);
	sprintf(sc_err_msg, "Calculate checksum failed");
	return FAILED;
    }
    if (((sm_message_t *)sc_response_msg)->type != resp_type) {
        printf("smart_cookie @%d\n", __LINE__);
        sprintf(sc_err_msg, "Error in response Expected = %02x, Received = "
		"%02x\n", resp_type, ((sm_message_t *)sc_response_msg)->type);
	return FAILED;  
    }
     
    return PASSED;
}
/*---------------------------------------------------------------------------
 * copy_cookie_from_sps_msg
 *
 * DESCRIPTION:
 *  This function copies the cookie data into cookie array 
 * 
 * PARAMETERS:
 *     cookie  - array to hold the cookie data
 *     res_msg - response message from the SCC
 *     data_length - length of data to be copied
 *
 * RETURNS:
 *     None
 *--------------------------------------------------------------------------*/
static void
copy_cookie_from_rsp_msg(uchar *cookie, uchar *res_msg, int length)
{
    int i;
    for (i = 0; i < length; i++) {
        cookie[i] = res_msg[i];
    }
#ifdef DEBUG
    for (i = 0; i < length; i++) {
        if (!(i % 16)) {
            printf("\n");
        }
        printf("%2x ", cookie[i]);
    }
#endif
}
/*---------------------------------------------------------------------------
 * sm_write_cookie_cmd
 *
 * DESCRIPTION:
 *  This function write cookie data into the smart chip 
 * 
 * PARAMETERS:
 *     con - sc_context pointer
 *     cookie_buf - cookie data to be written
 *
 * RETURNS:
 *     None
 *--------------------------------------------------------------------------*/
int
sm_write_cookie_cmd(sc_context *con, uchar *cookie_buf)
{
    int i;
    uchar data[MAX_DATA_SIZE];
    data[0] = SC_COOKIE_ADDR_MB; /* offset MSB */
    data[1] = SC_COOKIE_ADDR_LB; /* offset LSB */
    data[2] = COOKIE_DATA_SIZE;
    for (i = 0; i < COOKIE_DATA_SIZE; i++)
        data[3 + i] = cookie_buf[i];
    
    if (send_command_to_smart_cookie(con, SMART_EEPROM_WRITE, data,
				     data[2] + 3)) {
	cterr('f', 0, "Fail to write cookie content %s", sc_err_msg);
        return FAILED;
    }
    return PASSED;
}

/*------------------------------------------------------------------------
 * smart_cookie_read_x
 *
 * Description:
 *   This function reads x bytes in a smart cookie
 *
 * Parameters:
 *   con_p - Smart Cookie Context Structure
 *
 * Returns:
 *   Passed/Failed
 *--------------------------------------------------------------------------*/
int
smart_cookie_read_x(sc_context *con_p, ushort size)
{
    int i;
    uchar addr_mb, addr_lb;
    ushort base_addr, index_addr, temp, index = 0;
    uchar cmd[3];
    uchar cookie_buf[size];

    addr_mb = SC_COOKIE_ADDR_MB; /* offset MSB */
    addr_lb = SC_COOKIE_ADDR_LB; /* offset LSB */
    base_addr = (addr_mb << 8) | addr_lb;
    index_addr = base_addr;
    temp = size;
    while (temp > 0) {
        cmd[0] = addr_mb;
        cmd[1] = addr_lb;
        if (temp >  READ_WRITE_MAX_SIZE) {
            temp = temp - READ_WRITE_MAX_SIZE;
            cmd[2] = READ_WRITE_MAX_SIZE;
        } else {
            cmd[2] = temp;
            temp = 0;
        }

        index_addr = index_addr + cmd[2];

        /*
         * add prpass here so that if fail during read of cookie we do not
         * print the prpass of the previous test in the TestFailed field
         * (save_testprogress) when cterr is called.
         */
        if (send_command_to_smart_cookie(con_p, SMART_EEPROM_READ, cmd,
                                         sizeof(cmd))) {
            prpass(testpass, "read %s cookie", con_p->info_string);
            cterr('f',0,"Read cookie  failed type = %d,"
                  "slot = %d %s", con_p->type, con_p->slot, sc_err_msg);
            return FAILED;
        }

        for(i = 0; i < cmd[2]; i++) {
            cookie_buf[i+index] = ((sm_message_t *)sc_response_msg)->data[i];
        }
        index = index + cmd[2];
        addr_mb = (uchar)(index_addr >> 8);
        addr_lb = (uchar)(index_addr & 0xff);
    }
    copy_cookie_from_rsp_msg(con_p->cookie_contents, &cookie_buf[0],
                             size);
    return PASSED;
}

/*------------------------------------------------------------------------
 * sm_write_cookie_x_cmd
 *
 * Description:
 *   This function writes x bytes in a smart cookie
 *
 * Parameters:
 *   con_p - Smart Cookie Context Structure
 *   cookie_buf - Data to be stored
 *
 * Returns:
 *   Passed/Failed
 *--------------------------------------------------------------------------*/

int
sm_write_cookie_x_cmd(sc_context *con_p, uchar *cookie_buf, ushort size)
{
    int i;
    uchar addr_mb, addr_lb;
    ushort base_addr, index_addr, temp, index = 0;
    uchar data[MAX_DATA_SIZE];

    addr_mb = SC_COOKIE_ADDR_MB; /* offset MSB */
    addr_lb = SC_COOKIE_ADDR_LB; /* offset LSB */

    base_addr = (addr_mb << 8) | addr_lb;
    index_addr = base_addr;
    temp = size;

    while (temp > 0) {
        data[0] = addr_mb;
        data[1] = addr_lb;
        if (temp > READ_WRITE_MAX_SIZE) {
            temp = temp - READ_WRITE_MAX_SIZE;
            data[2] = READ_WRITE_MAX_SIZE;
        } else {
            data[2] = temp;
            temp = 0;
        }
        for (i = 0; i < data[2]; i++) {
            data[3 + i] = cookie_buf[i + index];
        }
        index = index + i;
        index_addr = index_addr + i;

        if (send_command_to_smart_cookie(con_p, SMART_EEPROM_WRITE, data,
                                         data[2] + 3)) {
            cterr('f', 0, "Failed to write cookie %s", sc_err_msg);
            return FAILED;
        }
        addr_mb = (uchar)(index_addr >> 8);
        addr_lb = (uchar)(index_addr & 0xff);
    }

    return PASSED;

}

/*---------------------------------------------------------------------------
 * smart_cookie_echo_test
 *
 * DESCRIPTION:
 *  This function send bytes of data to the smart chip and receive them back 
 * 
 * PARAMETERS:
 *     con - pointer to sc_context 
 *
 * RETURNS:
 *     PASSED - if successful
 *     FAILED - if unsuccessful
 *--------------------------------------------------------------------------*/
type_t
smart_cookie_echo_test(sc_context *con)
{
    int i,j, msg_length;
    uchar data[MAX_DATA_SIZE];
    int status;
    
    testname ("%s", "Smart Cookie echo");
    status = PASSED;
    msg_length = COOKIE_DATA_SIZE; /* only 128 bytes */
    for (i = 0; i < MAX_DATA_SIZE; i++) {
	data[i] = i;   /* fill data */
    }    
    for (j = 0; j < 2; j++) {
	if(send_command_to_smart_cookie(con, ECHO_REQUEST,
					&data[j*COOKIE_DATA_SIZE],
					msg_length)){
	    cterr('f', 0, "Failed to run echo request %s", sc_err_msg);
	    status = FAILED;
	}
	/* 
	 * Verify if echo message received is OK 
	 */
	for (i = 0 ; i < COOKIE_DATA_SIZE; i++) {
	    if (data[j*COOKIE_DATA_SIZE+i] != (uchar)sc_response_msg[i + 3]) {
		cterr('f',0,"\nEcho test failed Sent[%d] = %d, "
		      "Received[%d] = %d",
		      i, data[i], i, sc_response_msg[i + 3]);
		status = FAILED; 
	    }
	}
    }
    prcomplete(testpass, errcount, 0);
    return status;
}
/*---------------------------------------------------------------------------
 * smart_cookie_read
 *
 * DESCRIPTION:
 *  main entry for reading smart chip eeprom
 * 
 * PARAMETERS:
 *     con  - context pointer
 *
 * RETURNS:
 *     PASSED - if successful
 *     FAILED - if unsuccessful
 *--------------------------------------------------------------------------*/
int
smart_cookie_read(sc_context *con) 
{
    if (act2_is_simple_mode(con)) {
        cterr('f', 0, "device is in simple mode");
        return FAILED;
    }
    
    if (send_command_to_smart_cookie(con, REQUEST_COOKIE_DATA, 
				req_cookie_data, sizeof(req_cookie_data))) {
	return FAILED;
    }
    copy_cookie_from_rsp_msg(con->cookie_contents, &sc_response_msg[3], 
	 		     con->dev_if_p->cookie_size);
    return PASSED;
}
/*---------------------------------------------------------------------------
 * smart_cookie_read_write_aim_eeprom
 *
 * DESCRIPTION:
 *  This function allows reading and writing into EEPROM on Smart chip 
 * 
 * PARAMETERS:
 *     con - pointer to sc_context
 *
 * RETURNS:
 *     PASSED - if successful
 *     FAILED - if unsuccessful
 *--------------------------------------------------------------------------*/
int
smart_cookie_read_write_aim_eeprom(sc_context *con) {
  
    if(smart_cookie_read(con))
	return FAILED;
    if (cookie_4_processor_x(con->cookie_contents, MOTHER_BOARD, 
				con->slot, con->dev_if_p->cookie_size, NULL)) {/* 5th params is MENU mode */
	if(sm_write_cookie_cmd(con, con->cookie_contents))
	   return FAILED;
    }			
    return PASSED;
}
/*---------------------------------------------------------------------------
 * smart_cookie_read_write_eeprom
 *
 * DESCRIPTION:
 *  This function allows reading and writing into EEPROM on Smart chip 
 * 
 * PARAMETERS:
 *     con - pointer to sc_context
 *     cli_cmd - cli structure  
 *
 * RETURNS:
 *     PASSED - if successful
 *     FAILED - if unsuccessful
 *--------------------------------------------------------------------------*/
int
smart_cookie_read_write_eeprom (sc_context *con, cli_cookie_cmd *cli_cmd)
{

    if (act2_is_simple_mode(con)) {
        cterr('f', 0, "device is in simple mode. can't access Act1 space.");
        return FAILED;
    }
  
    if (con->dev_if_p->cookie_size != COOKIE_SIZE_128) {
        if(smart_cookie_read_x(con, con->dev_if_p->cookie_size)) {
            printf("failed smart_cookie.c @%d\n", __LINE__);
            return FAILED;
        }
        if (cookie_4_processor_x(con->cookie_contents, con->type, 
				con->slot, con->dev_if_p->cookie_size, cli_cmd)) {
            if(sm_write_cookie_x_cmd(con, con->cookie_contents,
                                 con->dev_if_p->cookie_size))
                 return FAILED;
        }
     } else {   /* size 128; should not come here for new new platforms (ovd, or newer ) */
	if(smart_cookie_read(con)) {
            return FAILED;
        }
        if (cookie_4_processor_x(con->cookie_contents, con->type,
                                con->slot, con->dev_if_p->cookie_size, cli_cmd)) {
	    if(sm_write_cookie_cmd(con, con->cookie_contents))
          	return FAILED;
        }   
    }			
    return PASSED;
}

/*---------------------------------------------------------------------------
 * smart_cookie_access_test
 *
 * DESCRIPTION:
 *  This function write a pattern into the EEPROM and read back, make sure
 *  they are the same
 * 
 * PARAMETERS:
 *     con - pointer to sc_context
 *
 * RETURNS:
 *     PASSED - if successful
 *     FAILED - if unsuccessful
 *--------------------------------------------------------------------------*/
type_t
smart_cookie_access_test(sc_context *con)
{
    int i, status;
    uchar eeprom_data_test[COOKIE_DATA_SIZE];
    testname ("%s", "Smart Cookie Read/Write");
    printf("\n\nDo not send break or turn off power during this test."
	   "\nThe EEPROM contents may not be recovered.\n\n");
    /* Flag to keep track if the cookie resides in smart or not */
    cookie_in_smart_access_test = TRUE;
    
    status = PASSED;
    if(smart_cookie_read(con))
	status = FAILED;
    /* Copy cookie contents to a safe place, clean up cookie_contents */
    for (i = 0; i < COOKIE_DATA_SIZE; i++) {
	eeprom_data_save[i] = con->cookie_contents[i];
	con->cookie_contents[i] = 0;
    }
#ifdef COOKIE_DEBUG    
    putchar('\n');
    for( i = 0; i < COOKIE_DATA_SIZE; i++) {
	if ((i != 0) && (i%16 == 0))
	    putchar('\n');
	linepos += printf("%.*x ",2,eeprom_data_save[i]);
    }
#endif    
    for( i = 0; i < PROTECTED_BYTES; i++) {/* do not test first 16 bytes */
	eeprom_data_test[i] = eeprom_data_save[i];
    }
    for( i = PROTECTED_BYTES; i < COOKIE_DATA_SIZE; i++) {
	eeprom_data_test[i] = i;
    }
    if(sm_write_cookie_cmd(con, eeprom_data_test))
	status = FAILED;
    if(smart_cookie_read(con))
	status = FAILED;
    for (i = 0; i < PROTECTED_BYTES; i++) {
	if(con->cookie_contents[i] != eeprom_data_test[i]) {
	    cterr('f',0,"mismatch on test readback protected bytes, "
		  "wrote %#x read %#x at %2x\n", eeprom_data_test[i],
		  con->cookie_contents[i], i);
	    status = FAILED;
	}
    }
    for( i = PROTECTED_BYTES; i < COOKIE_DATA_SIZE; i++) {
	if(con->cookie_contents[i] != eeprom_data_test[i]) {
	    cterr('f',0,"mismatch on test readback testing bytes, "
		  "wrote %#x read %#x at %2x\n", eeprom_data_test[i],
		  con->cookie_contents[i], i);
	    status = FAILED;
	}    
    }
#ifdef COOKIE_DEBUG
    putchar('\n');
    for( i = 0; i < COOKIE_DATA_SIZE; i++) {
	if ((i != 0) && (i%16 == 0))
	    putchar('\n');
	linepos += printf("%.*x ",2,con->cookie_contents[i]);
    }
#endif
    if(sm_write_cookie_cmd(con, eeprom_data_save))
	status = FAILED;
    if(smart_cookie_read(con))
	status = FAILED;
    
    for( i = 0; i < COOKIE_DATA_SIZE; i++) {
	if(con->cookie_contents[i] != eeprom_data_save[i]) {
	    cterr('f',0,"mismatch on test restoring, wrote %#x "
		  "read %#x at %2x\n", eeprom_data_save[i],
		  con->cookie_contents[i], i);
	    status = FAILED;
	}    
    }
#ifdef COOKIE_DEBUG
    putchar('\n');
    for( i = 0; i < COOKIE_DATA_SIZE; i++) {
	if ((i != 0) && (i%16 == 0))
	    putchar('\n');
	linepos += printf("%.*x ",2,con->cookie_contents[i]);
    }
#endif
    cookie_in_smart_access_test = FALSE;
    prcomplete(testpass, errcount, 0);
    return status;
}
/*---------------------------------------------------------------------------
 * get_ctrl_id_from_cookie_contents
 *
 * DESCRIPTION:
 *     This function extracts the controller ID from the cookie contents
 * 
 * PARAMETERS:
 *     uchar *: pointer to the cookie contents 
 *
 * RETURNS:
 *     cookie controller ID
 *     
 *--------------------------------------------------------------------------*/
static ushort
get_ctrl_id_from_cookie_contents(uchar *cookie_contents)
{
    uchar num_byte, *data_ptr;
    ushort field;
    int i;
    field = 0;
    if (( data_ptr = (uchar *)search_type_ret_addr_of_first_data 
	  (cookie_contents, (uchar)CONTROLLER_TYPE, 
	   &num_byte, FALSE)) == (uchar *)NULL){
	/*Search CONTROLLER_TYPE failed. */ 
	field = 0xffff; /* illegal code */
    } else {
	for (i = num_byte; i > 0;){
	    field |= *data_ptr << (--i*8);
	    data_ptr++;  
	}
    }
    return (field);
}
/*---------------------------------------------------------------------------
 * get_smart_cookie_controller_type  
 *
 * DESCRIPTION:
 *     This function return the controller type
 * 
 * 
 * PARAMETERS:
 *     con - sc_context structure 
 *
 * RETURNS:
 *     cookie controller type
 *     
 *--------------------------------------------------------------------------*/
ushort
get_smart_cookie_controller_type(sc_context *con)
{
    ushort controller_id = 0;
    if(smart_cookie_read(con))
	return ILLEGAL_ID;
    
    /* Cookie must be read into con->cookie_contents first */
    controller_id = get_ctrl_id_from_cookie_contents(con->cookie_contents);
    return (controller_id);
}

int
get_pid (uchar *cookie_contents, char *pid)
{
    uchar num_byte, *data_ptr;
    int i;
    
    if ((data_ptr = (uchar *) search_type_ret_addr_of_first_data
         (cookie_contents, (uchar) PRODUCT_ID,
          &num_byte, FALSE)) == (uchar *) NULL) {
        /*Search PRODUCT_ID failed. */
        pid[0] = 0;                /* illegal code */
	return(FAILED);
    } else {
	for (i = 0; i < num_byte; i++) {
            pid[i] = *data_ptr++;
        }
    }

    return(PASSED);
}

/*---------------------------------------------------------------------------
 * print_sm_cookie_fields  
 *
 * DESCRIPTION:
 *     This function reads and displays all fields in the digital signature
 * list
 * 
 * PARAMETERS:
 *     con - context structure pointer 
 *     dig_list - pointer to the array of the digital signature list
 *     mum_byte - number of elements in the digital signature list
 *
 * RETURNS:
 *     N/A
 *--------------------------------------------------------------------------*/
static void
print_sm_cookie_fields(sc_context *con, uchar *dig_list, uchar num_byte)
{
    uchar i,j, bytes, *data_ptr;
    uchar controller_type[4];
    char *pcb, *pid, *chassis;
    char pcb_serial_val_init[14] = " NO PCB NUM  ";
    uchar pm_serial_num_ascii[14];
    char product_id_init[15] = " NO PRODUCT ID";
    char chassis_serial_val_init[14] = "NO CHASSI NUM";
    uchar chassis_serial_num_ascii[14];
    uchar product_id[128];

    for (j = 0; j < num_byte; j++) {
        switch (dig_list[j]) {
	case CONTROLLER_TYPE:
	    if (( data_ptr = (uchar *)search_type_ret_addr_of_first_data 
		   (con->cookie_contents, (uchar)CONTROLLER_TYPE, 
		    &bytes, FALSE)) == (uchar *)NULL){
	       /*Search CONTROLLER_TYPE failed. */ 
	         for (i = 0; i < bytes + 2; i++)
		     controller_type[i] = 0xff;
	    } else {
	        data_ptr--;
	        for (i = 0; i < bytes + 1; i++) {
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
	        data_ptr -= 2;
	        for (i = 0; i < bytes + 2; i++) {
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
		data_ptr -= 2;
	        for (i = 0; i < bytes + 2; i++) {
		    product_id[i] = *data_ptr++;
		    printf("%02x ", product_id[i]);    
		}
	    }
	    break;
        case CHASSIS_SERIAL_NUM:
            if ((data_ptr = (uchar *)search_type_ret_addr_of_first_data
                 (con->cookie_contents, CHASSIS_SERIAL_NUM,
                  &bytes, FALSE)) == NULL) {
                chassis = chassis_serial_val_init;
                for ( i = 0; i < 13; i++)
                    chassis_serial_num_ascii[i] = *chassis++;
            } else {
                data_ptr -= 2;
                for (i = 0; i < bytes + 2; i++) {
                    chassis_serial_num_ascii[i] = *data_ptr++;
                    printf("%02x ", chassis_serial_num_ascii[i]);
                }
            }
            break;
	default:
	    break;
	}
    }
}
/*---------------------------------------------------------------------------
 * print_sm_dig_sig_list 
 *
 * DESCRIPTION:
 *     This function reads and displays the digital signature list
 * 
 * 
 * PARAMETERS:
 *     con - sc_context structure 
 *
 * RETURNS:
 *     NONE
 *--------------------------------------------------------------------------*/
static void
print_sm_dig_sig_list(sc_context *con)
{
    uchar i, bytes, *data_ptr;
    uchar dig_sig_list[100];
    if ((data_ptr = (uchar *)search_type_ret_addr_of_first_data
	 (con->cookie_contents, DIG_SIG_LIST, 
	  &bytes, FALSE)) == NULL) {
        for ( i = 0; i < 100; i++)
	    dig_sig_list[i] = 0xff;
    } else {
        data_ptr -= 2;
        for (i = 0; i < bytes + 2; i++) {
	    dig_sig_list[i] = *data_ptr++;
	    printf("%02x ", dig_sig_list[i]);    
	}
    }
}
/*---------------------------------------------------------------------------
 * get_sm_dig_sign_list
 *
 * DESCRIPTION:
 *     This function reads the digital signature list into an array
 * 
 * 
 * PARAMETERS:
 *     con - sc_context structure 
 *
 * RETURNS:
 *     PASSED - if successful
 *     FAILED - if unsuccessful
 *--------------------------------------------------------------------------*/
static int
get_sm_dig_sign_list(sc_context *con, uchar *dig_list, uchar *num_byte)
{
    uchar *data_ptr, j;
    sc_context *con_p;
    con_p = con;
    if ((data_ptr = (uchar *)search_type_ret_addr_of_first_data
        (con_p->cookie_contents, DIG_SIG_LIST, num_byte, FALSE)) == NULL) {
        cterr('f', 0, "Can't read digital signature list");
	return FAILED;
    } else {
        for (j = 0; j < *num_byte; j++) { 
	    dig_list[j] = *data_ptr++;
	}
    }
    return PASSED;
}
 
/*---------------------------------------------------------------------------
 * sm_write_cm_sign_cmd
 *
 * DESCRIPTION:
 *     This function writes the comtract manufacturing signature (certificate)
 * 
 * 
 * PARAMETERS:
 *     con - sc_context structure 
 *     cm_signature - contract manufacturing signature
 *    
 * RETURNS:
 *     PASSED - if successful
 *     FAILED - if unsuccessful
 *--------------------------------------------------------------------------*/
static int 
sm_write_cm_sign_cmd(sc_context *con, uchar *cm_signature)
{
    int i;
    uchar data[MAX_DATA_SIZE];
    data[0] = SC_CM_PUB_KEY_SIGN_ADDR_MB; /* offset MSB */
    data[1] = SC_CM_PUB_KEY_SIGN_ADDR_LB; /* offset LSB */
    data[2] = SIGN_SIZE + 2;  
    
    for (i = 0; i < SIGN_SIZE + 2; i++)
        data[3 + i] = cm_signature[i];
    
    if (send_command_to_smart_cookie(con, SMART_EEPROM_WRITE, data, 
				     data[2] + 3)) {
        cterr('f', 0, "Failed to write CM Signature %s", sc_err_msg);
	return FAILED;
    }
    /* Verify the Signature */
    if (send_command_to_smart_cookie(con, REQUEST_CNTMFG_PUBKEY_SIGN_3, 
				     NULL, 0)) {
        cterr('f', 0, "Failed to read CM Signature %s", sc_err_msg);
	return FAILED;
    }
    printf("\n");
    for (i = 0; i < SIGN_SIZE + 2; i++) {
	if ((NVRAM)->diagflag & D_VERBOSE) {
	    printf("%02x ",
		   ((sm_message_t *)sc_response_msg)->data[i+PUB_KEY_SIZE]);
	}
        
	if (((sm_message_t *)sc_response_msg)->data[i+PUB_KEY_SIZE] != 
	    cm_signature[i]) {
	    cterr('f', 0, "Verify CM digital signature failed, expect %#.2x, "
		  "read %#.2x\n", cm_signature[i], 
		  ((sm_message_t *)sc_response_msg)->data[i+PUB_KEY_SIZE]); 
             return FAILED;
 	}
	
    }
    return PASSED;
}
/*---------------------------------------------------------------------------
 * sm_write_cm_pub_key_cmd
 *
 * DESCRIPTION:
 *     This function writes the contract manufacturing public key
 * 
 * 
 * PARAMETERS:
 *     con - sc_context structure 
 *
 * RETURNS:
 *     PASSED - if successful
 *     FAILED - if unsuccessful
 *--------------------------------------------------------------------------*/
static int 
sm_write_cm_pub_key_cmd(sc_context *con)
{
    unsigned int i;
    uchar data[MAX_DATA_SIZE];
    data[0] = SC_CM_PUB_KEY_ADDR_MB; /* offset MSB */
    data[1] = SC_CM_PUB_KEY_ADDR_LB; /* offset LSB */
    data[2] = sizeof(cm_mfg_pub_key);
    for (i = 0; i < sizeof(cm_mfg_pub_key); i++)
        data[3 + i] = cm_mfg_pub_key[i];
    if (send_command_to_smart_cookie(con, SMART_EEPROM_WRITE, data, 
				     data[2] + 3)) {
        cterr('f', 0, "Failed to write CM Pub Key %s", sc_err_msg);
	return FAILED;
    }
    /* Verify the Signature */
    if (send_command_to_smart_cookie(con, REQUEST_CNTMFG_PUBKEY_SIGN_3, 
				     NULL, 0)) {
        cterr('f', 0, "Failed to read CM Pub Key %s", sc_err_msg);
	return FAILED;
    }
    printf("\n");
    for (i = 0; i < SIGN_SIZE; i++) {
	if ((NVRAM)->diagflag & D_VERBOSE) {
	    printf("%02X ", ((sm_message_t *)sc_response_msg)->data[i]);
	}
        if (((sm_message_t *)sc_response_msg)->data[i] != cm_mfg_pub_key[i]) {
	    cterr('f', 0, "Verify cookie digital signature failed, "
		  "expect %#.2x, read %#.2x\n",
		  cm_mfg_pub_key[i], ((sm_message_t *)sc_response_msg)->data[i]); 
            return FAILED;
	}
    }
    return PASSED;
}
/*---------------------------------------------------------------------------
 * sm_write_cookie_sign_cmd
 *
 * DESCRIPTION:
 *     This function writes the cookie signature
 * 
 * 
 * PARAMETERS:
 *     con - sc_context structure 
 *     cookie_signature - cookie signature
 *
 * RETURNS:
 *     PASSED - if successful
 *     FAILED - if unsuccessful
 *--------------------------------------------------------------------------*/
static int 
sm_write_cookie_sign_cmd(sc_context *con, uchar *cookie_signature)
{
    int i;
    uchar data[MAX_DATA_SIZE];
    data[0] = SC_COOKIE_SIGN_ADDR_MB; /* offset MSB */
    data[1] = SC_COOKIE_SIGN_ADDR_LB; /* offset LSB */
    data[2] = SIGN_SIZE;
    for (i = 0; i < SIGN_SIZE; i++) {
        data[3 + i] = cookie_signature[i];
    }	
    if (send_command_to_smart_cookie(con, SMART_EEPROM_WRITE, data, 
				     data[2] + 3)) {
        cterr('f', 0, "Failed to write Cookie Signature %s", sc_err_msg);
	return FAILED;
    }
    if (send_command_to_smart_cookie(con, REQUEST_CK_SIGN_4, NULL, 0)) {
        cterr('f', 0, "Cookie signature failed type = %d, slot = %d %s",
              con->type, con->slot, sc_err_msg);
        return FAILED;
    }
    printf("\n");
    for (i = 0; i < SIGN_SIZE; i++) { 
	if ((NVRAM)->diagflag & D_VERBOSE) {
	    printf("%02X ", ((sm_message_t *)sc_response_msg)->data[i]);
	}
        if (((sm_message_t *)sc_response_msg)->data[i] != cookie_signature[i]) {
	    cterr('f', 0, "Verify CM digital signature failed," 
		  "expect %#.2x, read %#.2x\n", cookie_signature[i],
		  ((sm_message_t *)sc_response_msg)->data[i]); 
            return FAILED;
	}
    }
    return PASSED;
}
/*---------------------------------------------------------------------------
 * sm_write_ep_tlv_info_cmd 
 *
 * DESCRIPTION:
 *     This function writes the Epsilon (contract manufacturing) TLV 
 * 
 * 
 * PARAMETERS:
 *     con - sc_context structure
 *     ep_tlv_info - pointer to the array of Epsilon TLV Info
 *     len - length of the TLV Info 
 *
 * RETURNS:
 *     PASSED - if successful
 *     FAILED - if unsuccessful
 *--------------------------------------------------------------------------*/
static int 
sm_write_ep_tlv_info_cmd(sc_context *con, uchar *ep_tlv_info, int len)
{
    int i;
    uchar data[MAX_DATA_SIZE];
    uchar cmd[2];
    data[0] = SC_CM_LOT_TLV_ADDR_MB; /* offset MSB */
    data[1] = SC_CM_LOT_TLV_ADDR_LB; /* offset LSB */
    data[2] = LOT_INFO_SIZE;
    for (i = 0; i < LOT_INFO_SIZE; i++) {
        data[3 + i] = ep_tlv_info[i];
    }
   
    if (send_command_to_smart_cookie(con, SMART_EEPROM_WRITE, data, 
				     data[2] + 3)) {
        cterr('f', 0, "Failed to write TLV info %s", sc_err_msg);
	return FAILED;
    }
        
    cmd[0] = 2;
    if (send_command_to_smart_cookie(con, REQUEST_LOT_INFO, cmd, 1)) {
        cterr('f',0,"RESPONSE_LOT_INFO failed type = %d, slot = %d %s",
              con->type, con->slot, sc_err_msg);
        return FAILED;
    }
    printf("\n");
    for (i = 0; i < (len+1)/3; i++) {
	if ((NVRAM)->diagflag & D_VERBOSE) {
	    printf("%02X ", ((sm_message_t *)sc_response_msg)->data[i]);
	}
        if (((sm_message_t *)sc_response_msg)->data[i] != ep_tlv_info[i]) {
	    cterr('f', 0, "Verify CM digital signature failed, "
		  "expect %#.2x, read %#.2x\n",
		 ep_tlv_info[i], ((sm_message_t *)sc_response_msg)->data[i]); 
            return FAILED; 
	} 
    }
    return PASSED;
}
/*---------------------------------------------------------------------------
 * smart_cookie_write_dig_sig
 *
 * DESCRIPTION:
 *  This function receive the data from the console and write into the 
 *  smart chip
 * 
 * PARAMETERS:
 *     con - context structure 
 *
 * RETURNS:
 *     PASSED - if successful
 *     FAILED - if unsuccessful
 *--------------------------------------------------------------------------*/
static type_t
smart_cookie_write_dig_sig(sc_context *con)
{
    int i, len;
    char sig_buf[SIGN_SIZE * 3]; /* two for a 2 digit hex and 1 for space */
    char cm_sig_buf[(SIGN_SIZE + 2) * 3];
    uchar cm_signature[SIGN_SIZE + 2], cookie_signature[SIGN_SIZE];
    uchar ep_tlv_info[LOT_INFO_SIZE];
    char ep_tlv_buf[LOT_INFO_SIZE * 3]; /* two for a 2 digit hex
					    and 1 for space */
    
    printf("\nEnter Cookie Digital Signature >");
    get_line(sig_buf, sizeof(sig_buf)); 
    len = strlen(sig_buf);
    
    if (len != (SIGN_SIZE * 3 - 1)) {
        cterr('f', 0, "\nThe Cookie Digital Signature must be 48 bytes");
	return FAILED;
    }
    
    for (i = 0; i < len; i++) {
        sig_buf[i] = atoh(sig_buf[i]);
    }
    for (i = 0; i < SIGN_SIZE; i++) {
        cookie_signature[i] = ((sig_buf[3*i] << 4) & 0xf0) | 
	                       ((sig_buf[3*i + 1]) & 0x0f);
    }
    do {
        printf("\nEnter CM Public Key >");
        get_line(sig_buf, sizeof(sig_buf)); 
        len = strlen(sig_buf);
        if (len != (SIGN_SIZE * 3 - 1)) {
            printf("The CM Public Key must be 48 bytes\n");
        }
    } while (len != (SIGN_SIZE * 3 - 1));
    
    for (i = 0; i < len; i++) {
        sig_buf[i] = atoh(sig_buf[i]);
    }
    for (i = 0; i < SIGN_SIZE; i++) {
        cm_mfg_pub_key[i] = ((sig_buf[3*i] << 4) & 0xf0) | 
	                      ((sig_buf[3*i + 1]) & 0x0f);
    }
    do {
        printf("\nEnter CM Certificate >");
        get_line(cm_sig_buf, sizeof(cm_sig_buf));
        len = strlen(cm_sig_buf);
        if (len != ((SIGN_SIZE + 2) * 3 - 1)) {
            printf("\nThe CM Certificate must be 50 bytes\n");
        }
    } while (len != (SIGN_SIZE + 2) *3 - 1);

    for (i = 0; i < len; i++) {
        cm_sig_buf[i] = atoh(cm_sig_buf[i]);
    }
    for (i = 0; i < SIGN_SIZE + 2; i++) {
        cm_signature[i] = ((cm_sig_buf[3*i] << 4) & 0xf0) | 
	                   ((cm_sig_buf[3*i + 1]) & 0x0f);
    }

    do {
        printf("\nEnter Epsilon TLV >");
        get_line(ep_tlv_buf, sizeof(ep_tlv_buf)); 
        len = strlen(ep_tlv_buf);
    
        if (len > LOT_INFO_SIZE * 3) {
            printf("\nThe Epsilon TLV exceeds 64 bytes\n");
        }
    } while ((!len) || (len >  LOT_INFO_SIZE * 3));
    
    for (i = 0; i < len; i++) {
        ep_tlv_buf[i] = atoh(ep_tlv_buf[i]);
    }
    for (i = 0; i < (len+1)/3; i++) {
        ep_tlv_info[i] = ((ep_tlv_buf[3*i] << 4) & 0xf0) | 
	                  ((ep_tlv_buf[3*i + 1]) & 0x0f);
    }
    if(sm_write_cookie_sign_cmd(con, cookie_signature)) 
        return FAILED; 
    
    if(sm_write_cm_pub_key_cmd(con)) 
        return FAILED;   
    
    if(sm_write_cm_sign_cmd(con, cm_signature)) 
        return FAILED; 
    
    if(sm_write_ep_tlv_info_cmd(con, ep_tlv_info, len)) 
        return FAILED; 
    
    return PASSED;
}
/*---------------------------------------------------------------------------
 * display_sm_mfg_lot_TLV 
 *
 * DESCRIPTION:
 *     This function reads and displays the SC manufacturing lot TLV
 * 
 * 
 * PARAMETERS:
 *     con - context structure 
 *
 * RETURNS:
 *     PASSED - if successful
 *     FAILED - if unsuccessful
 *--------------------------------------------------------------------------*/
static int 
display_sm_mfg_lot_TLV(sc_context *con)
{
    uchar i;
    uchar cmd[1];
    cmd[0] = 1;
    if (send_command_to_smart_cookie(con, REQUEST_LOT_INFO, cmd, 1)) {
        cterr('f',0,"RESPONSE_LOT_INFO failed type = %d, slot = %d %s",
              con->type, con->slot, sc_err_msg);
        return FAILED;
     }
     printf("\nTLV Info : ");
     for (i = 0; i < LOT_INFO_SIZE; i++) { 
	 scc_lot_info[i] = ((sm_message_t *)sc_response_msg)->data[i];
	 printf("%02x ",  scc_lot_info[i]); 
     }
     printf("\n");
     return PASSED;
}
/*---------------------------------------------------------------------------
 * display_cookie_signature
 *
 * DESCRIPTION:
 *     This function reads and displays the cookie digital signature
 * 
 * 
 * PARAMETERS:
 *     con - context structure 
 *
 * RETURNS:
 *     PASSED - if successful
 *     FAILED - if unsuccessful
 *--------------------------------------------------------------------------*/
static int
display_cookie_signature(sc_context *con)
{
    int i;
    if (send_command_to_smart_cookie(con, REQUEST_CK_SIGN_4, NULL, 0)) {
        cterr('f', 0, "Cookie signature failed type = %d, slot = %d %s",
              con->type, con->slot, sc_err_msg);
        return FAILED;
    }
    printf("\nCookie Digital Signature : ");    
    for (i = 0; i < SIGN_SIZE; i++) {
        signature[i] =  ((sm_message_t *)sc_response_msg)->data[i];
	printf("%02x ", signature[i]);
    }
    return PASSED;
}
/*---------------------------------------------------------------------------
 * display_cm_pb_key_signature 
 *
 * DESCRIPTION:
 *     This function reads and displays the contract manufacturing public
 * key and signature
 * 
 * PARAMETERS:
 *     con - sc_context structure 
 *
 * RETURNS:
 *     PASSED - if successful
 *     FAILED - if unsuccessful
 *--------------------------------------------------------------------------*/
static int
display_cm_pb_key_signature(sc_context *con)
{
    int i;
    uchar cm_signature[SIGN_SIZE + 2];
    uchar cmd[1];
    if (send_command_to_smart_cookie(con, REQUEST_CNTMFG_PUBKEY_SIGN_3, 
				     NULL, 0)) {
        cterr('f',0, "\nCNTMFG_PUBKEY_SIGN failed type = %d, slot = %d %s",
	      con->type, con->slot, sc_err_msg);
        return FAILED;
    }
    printf("\nCM Public Key: ");
    for (i = 0; i < PUB_KEY_SIZE; i++) {
        cm_mfg_pub_key[i] = ((sm_message_t *)sc_response_msg)->data[i];
        printf("%02x ", cm_mfg_pub_key[i]);
    }
    printf("\nCM Certificate: ");
    for (i = 0; i < SIGN_SIZE + 2; i++) {
        cm_signature[i] = ((sm_message_t *)sc_response_msg)->data[i+PUB_KEY_SIZE];
        printf("%02x ", cm_signature[i]);
    }
    cmd[0] = 2;
    if (send_command_to_smart_cookie(con, REQUEST_LOT_INFO, cmd, 1)) {
        cterr('f',0,"RESPONSE_LOT_INFO failed type = %d, slot = %d %s",
              con->type, con->slot, sc_err_msg);
        return FAILED;
    }
    printf("\nCM LOT TLV: ");
    for (i = 0; i < LOT_INFO_SIZE; i++) {
       printf("%02x ", ((sm_message_t *)sc_response_msg)->data[i]);
    }
    return PASSED;
}
/*---------------------------------------------------------------------------
 * smart_cookie_write_mfg_lockdown_area
 *
 * DESCRIPTION:
 *     This function writes the controller ID into 0x800 address
 * 
 * 
 * PARAMETERS:
 *     con - sc_context structure 
 *
 * RETURNS:
 *     PASSED - if successful
 *     FAILED - if unsuccessful
 *--------------------------------------------------------------------------*/
static int
smart_cookie_write_mfg_lockdown_area(sc_context *con)
{
    uchar i, bytes, *data_ptr;
    uchar data[MAX_DATA_SIZE];
    uchar cmd[3];
    if (( data_ptr = (uchar *)search_type_ret_addr_of_first_data 
	  (con->cookie_contents, (uchar)CONTROLLER_TYPE, 
	   &bytes, FALSE)) == (uchar *)NULL){
        /*Search CONTROLLER_TYPE failed. */ 
        for (i = 0; i < bytes + 2; i++)
	    ctrl_ID[i] = 0xff;
    } else {
        data_ptr--;
	for (i = 0; i < 3; i++) {
	    ctrl_ID[i] = *data_ptr++;
        }
    }
    if (((ctrl_ID[1] == 0x00) && (ctrl_ID[2] == 0x00)) ||
	((ctrl_ID[1] == 0xff) && (ctrl_ID[2] == 0xff))) {
	cterr('f', 0, "Controller ID invalid = %#.02x %#.02x",
	      ctrl_ID[1], ctrl_ID[2]);
	return FAILED;
    }
    
    data[0] = SC_LOCK_DOWN_ADDR_MB; /* offset MSB */
    data[1] = SC_LOCK_DOWN_ADDR_LB; /* offset LSB */
    data[2] = sizeof(ctrl_ID);
    for (i = 0; i < sizeof(ctrl_ID); i++) {
        data[3 + i] = ctrl_ID[i];
    }
   
    if (send_command_to_smart_cookie(con, SMART_EEPROM_WRITE, data, 
				     data[2] + 3)) {
        cterr('f', 0, "Failed to write to Mfg Lockdown area %s", sc_err_msg);
	return FAILED;
    }
    cmd[0] = SC_LOCK_DOWN_ADDR_MB;
    cmd[1] = SC_LOCK_DOWN_ADDR_LB;
    cmd[2] = 0x03;
    if (send_command_to_smart_cookie(con, SMART_EEPROM_READ, cmd, 3)) {
        cterr('f',0,"READ MFG LOCKDOWN AREA failed type = %d,"
	      "slot = %d %s", con->type, con->slot, sc_err_msg);
        return FAILED;
    }
    for(i = 0; i < sizeof(ctrl_ID); i++) {
      /* printf("%02x ", ((sm_message_t *)sc_response_msg)->data[i]); */
	if (ctrl_ID[i] != ((sm_message_t *)sc_response_msg)->data[i]) {
	    cterr('f',0,"\n Wrong Controller ID Expect = %02x, Read = %02x",
		  ctrl_ID[i], ((sm_message_t *)sc_response_msg)->data[i]);
	    return FAILED; 
	}
    }
    return PASSED;
}
/*---------------------------------------------------------------------------
 * smart_cookie_lockdown
 *
 * DESCRIPTION:
 *     This function locks down 64 bytes (1 page) starting at 0x800 address.
 * After looking down, it tries to write again to test the lock down.
 * 
 * PARAMETERS:
 *     con - sc_context structure 
 *
 * RETURNS:
 *     PASSED - if successful
 *     FAILED - if unsuccessful
 *--------------------------------------------------------------------------*/
static int 
smart_cookie_lockdown(sc_context *con)
{
    unsigned int i;
    uchar cmd[9], test_ID[3];
    uchar data[MAX_DATA_SIZE];
    cmd[0] = EEPROM_PAGE_LOCK_DOWN;
    cmd[1] = 0x00;  
    cmd[2] = 0x20;  /* address (0x800) / size of page (0x40) */
    cmd[3] = 0x00;  
    cmd[4] = 0x20;
    cmd[5] = 0x00;    
    cmd[6] = 0x01;  /* lock only 1 page */
    cmd[7] = 0x00;  
    cmd[8] = 0x01;
    if (send_command_to_smart_cookie(con, EEPROM_PAGE_LOCK_DOWN, cmd, 9)) {
        cterr('f',0,"\nLOCKDOWN failed type = %d, slot = %d %s",
              con->type, con->slot, sc_err_msg);
        return FAILED;
    }
    /* Testing the lockdown area */
    
    for (i = 0; i < sizeof(test_ID); i++)
        test_ID[i] = 0xab;    /* random number to test */ 
    data[0] = SC_LOCK_DOWN_ADDR_MB; /* offset MSB */
    data[1] = SC_LOCK_DOWN_ADDR_LB; /* offset LSB */
    data[2] = sizeof(test_ID);
    for (i = 0; i < sizeof(test_ID); i++) {
        data[3 + i] = test_ID[i];
    }
   
    if (send_command_to_smart_cookie(con, SMART_EEPROM_WRITE, data,
				     data[2] + 3)) {
	cterr('f', 0, "Failed to write smart eeprom %s", sc_err_msg);
	return FAILED;
    }
    cmd[0] = SC_LOCK_DOWN_ADDR_MB;
    cmd[1] = SC_LOCK_DOWN_ADDR_LB;
    cmd[2] = 0x03;
    if (send_command_to_smart_cookie(con, SMART_EEPROM_READ, cmd, 3)) {
        cterr('f',0,"READ MFG LOCKDOWN AREA failed type = %d, slot = %d %s",
              con->type, con->slot, sc_err_msg);
        return FAILED;
    }
    for(i = 0; i < sizeof(ctrl_ID); i++) {
      /* printf("%02x ", ((sm_message_t *)sc_response_msg)->data[i]); */
	if (ctrl_ID[i] != ((sm_message_t *)sc_response_msg)->data[i]) {
	    cterr('f',0,"\n Wrong Controller ID Expect = %02x, Read = %02x",
		 ctrl_ID[i], ((sm_message_t *)sc_response_msg)->data[i]);
	    return FAILED; 
	}
    }
    return PASSED;
}
/*---------------------------------------------------------------------------
 * smart_cookie_program_dig_sign_mainmenu
 *
 * DESCRIPTION:
 *     This function program write the controller ID into 0x800 address,  
 * lock this area, and program the digital signature 
 * 
 * PARAMETERS:
 *     con - sc_context structure 
 *
 * RETURNS:
 *     PASSED - if successful
 *     FAILED - if unsuccessful
 *--------------------------------------------------------------------------*/
type_t
smart_cookie_program_dig_sign_mainmenu(sc_context *con)
{
    int i;
    uchar num_byte;
    uchar dev_signature[SIGN_SIZE];
    uchar dig_list[COOKIE_SIZE_128]; /* Max number of fields in cookie */
    
    testname ("Smart Cookie");
    prpass(testpass, "Program Signature");
    
    if (smart_cookie_read(con))
	return FAILED;
    if (check_quack_version(con))
	return FAILED;
    
    /* write controller ID and lock down */
    if (smart_cookie_write_mfg_lockdown_area(con))
	return FAILED;
    if (smart_cookie_lockdown(con))
	return FAILED;
    /* Get the digital signature list */
    if (get_sm_dig_sign_list(con, dig_list, &num_byte))
	return FAILED;
    for (i = 0; i < SER_NUM_SIZE; i++) {
         serial_number[i] = 0;
    }
    if (send_command_to_smart_cookie(con, REQUEST_SN_SCMFG_PUBKEY_SIGN_1, 
				     NULL, 0)) {
        cterr('f',0,"SN_SCMFG_PUB_KEY_SIGN failed type = %d, "
	      "slot = %d %s", con->type, con->slot, sc_err_msg);
        return FAILED;
     }
    for (i = 0; i < SER_NUM_SIZE; i++) 
	 serial_number[i] = ((sm_message_t *)sc_response_msg)->data[i];
    if (send_command_to_smart_cookie(con, REQUEST_VL_DEV_PUBKEY_SIGN_2, 
				     NULL, 0)) {
       cterr('f', 0,"VL_DEV_PUBKEY_SIGN failed type = %d, slot = %d %s",
	     con->type, con->slot, sc_err_msg);
       return FAILED;
    }
    for (i = 0; i < SIGN_SIZE; i++) {
        dev_signature[i] = 
	     ((sm_message_t *)sc_response_msg)->data[i+PUB_KEY_SIZE+1];
    }
    if(display_sm_mfg_lot_TLV (con))
	return FAILED;
    printf("All cookie data fields : ");	    
    for (i = 0; i < SER_NUM_SIZE; i++) {
	printf("%02x ", serial_number[i]);
    }
    print_sm_cookie_fields (con, dig_list, num_byte); 
    print_sm_dig_sig_list (con);
    for(i = 0; i < SIGN_SIZE; i++)
        printf("%02x ", dev_signature[i]);
    
    printf("\nSC Serial Number        : ");
    for (i = 0; i < SER_NUM_SIZE; i++) {
	printf("%02x ", serial_number[i]);
    }
    print_sm_cookie_field_by_field (con, dig_list, num_byte);
    printf("\nDigital Signature List  : ");
    print_sm_dig_sig_list (con);
    printf("\nDev Pub Key Signature   : ");
    for(i = 0; i < SIGN_SIZE; i++)
	printf("%02x ", dev_signature[i]);
    
    if (smart_cookie_write_dig_sig(con))
	return FAILED;
    
    return PASSED;
}

/*
    smart_cookie_program_dig_sign_submenu
    
    DESCRIPTION:
        This function is the wrapper for CLI and Menu mode
        
    PARAMETERS:
        con - sc_context structure
        mode - FALSE (Menu Mode)
               TRUE  (CLI Mode)
    
    RETURNS:
       PASSED - if successful
       FAILED - if unsuccessful
*/
static type_t
smart_cookie_program_dig_sign_submenu(sc_context *con)
{   
    int rc = PASSED;
	  
    if (smart_cookie_program_dig_sign_submenu_x(con, FALSE))
    {
        printf("smart_cookie_program_dig_sign_submenu failed; line %d\n", __LINE__);
        rc = FAILED;
    }
    
    return rc;
}

/*---------------------------------------------------------------------------
 * smart_cookie_program_dig_sign_submenu
 *
 * DESCRIPTION:
 *     This function program write the controller ID into 0x800 address,  
 * lock this area, and program the digital signature 
 * 
 * PARAMETERS:
 *     con - sc_context structure 
 *
 * RETURNS:
 *     PASSED - if successful
 *     FAILED - if unsuccessful
 *--------------------------------------------------------------------------*/
int
smart_cookie_program_dig_sign_submenu_x(sc_context *con, boolean mode)
{
    int i, stop;
    uchar num_byte, ch;
    uchar dev_signature[SIGN_SIZE];
    uchar dig_list[128]; /* Max number of fields in cookie */
    
    testname ("%s", "Smart Cookie");
    prpass(testpass, "Program Signature");
     stop = FALSE;
     if(smart_cookie_read(con))
         return FAILED;
     /* write controller ID and lock down */
     if(smart_cookie_write_mfg_lockdown_area(con))
         return FAILED;
     if(smart_cookie_lockdown(con))
         return FAILED;
     /* Get the digital signature list */
     if(get_sm_dig_sign_list(con, dig_list, &num_byte))
         return FAILED;
     for (i = 0; i < SER_NUM_SIZE; i++) {
         serial_number[i] = 0;
     }
     if (send_command_to_smart_cookie(con, REQUEST_SN_SCMFG_PUBKEY_SIGN_1, 
                                      NULL, 0)) {
         cterr('f',0,"SN_SCMFG_PUB_KEY_SIGN failed type = %d, "
               "slot = %d %s", con->type, con->slot, sc_err_msg);
         return FAILED;
      }
     for (i = 0; i < SER_NUM_SIZE; i++) 
          serial_number[i] = ((sm_message_t *)sc_response_msg)->data[i];
     if (send_command_to_smart_cookie(con, REQUEST_VL_DEV_PUBKEY_SIGN_2, 
                                      NULL, 0)) {
        cterr('f', 0,"VL_DEV_PUBKEY_SIGN failed type = %d, slot = %d %s",
              con->type, con->slot, sc_err_msg);
        return FAILED;
     }
     for (i = 0; i < SIGN_SIZE; i++) {
         dev_signature[i] = 
              ((sm_message_t *)sc_response_msg)->data[i+PUB_KEY_SIZE+1];
     }
     /* CLI mode */
     if (mode) {
         if (smart_cookie_program_dig_sign_mainmenu(con))
         {
             return FAILED;
         }
         smart_cookie_authenticate(con);
         return PASSED;
     }
     else {
     while (1) {
         printf("\n\nCOMMAND\n");
         printf("1. Program & Authenticate Digital Signature.\n");
         printf("2. Display SC Mfg Lot TLV.\n");
         printf("3. Display data to sign.\n");
         printf("4. Display detailed data.\n");
         printf("5. Exit\n");
         ch = getc_answer("Select an option", "1234567", '5');
         switch(ch) {
         case '1':
             if (smart_cookie_program_dig_sign_mainmenu(con))
                 break;
             smart_cookie_authenticate(con);
             break;
         case '2':
             display_sm_mfg_lot_TLV (con);
             break;
         case '3':
             printf("All cookie data fields : ");	    
             for (i = 0; i < SER_NUM_SIZE; i++) {
                 printf("%02x ", serial_number[i]);
             }
             print_sm_cookie_fields (con, dig_list, num_byte); 
             print_sm_dig_sig_list (con);
             for(i = 0; i < SIGN_SIZE; i++)
                  printf("%02x ", dev_signature[i]);
             printf("\n");
             break;
         case '4':    
             printf("\nSC Serial Number        : ");
             for (i = 0; i < SER_NUM_SIZE; i++) {
                 printf("%02x ", serial_number[i]);
             }
             print_sm_cookie_field_by_field (con, dig_list, num_byte);
             printf("\nDigital Signature List  : ");
             print_sm_dig_sig_list (con);
             printf("\nDev Pub Key Signature   : ");
             for(i = 0; i < SIGN_SIZE; i++)
                  printf("%02x ", dev_signature[i]);
             break;
         case '5':
             printf("Exit\n");
             stop = TRUE;
             break;
         case '6':
             display_cookie_signature (con);
             break;
         case '7':
             display_cm_pb_key_signature (con);
             break;
         default:
             stop = TRUE;
             break;
         }
         if (stop == TRUE)
             break;
     }
   }
     return PASSED;

 }

 int
 query_user_for_device(char *choice)
 {
     int i;
     printf("\nEnter (M)otherboard, (S)M Module, "
                 " (W)ic & Vic, (P)VDM, (Q)UIT >");

     i = get_line(choice, sizeof(choice));

     choice[0] = toupper((int)choice[0]);
     if (choice[0] == 'Q')
         return -1;

     if (i == 1) { /* The user doesn't enter the slot number */
         choice[1] = 'f'; /* dummy slot */
         printf("no slot selected  smart_cookie %d\n", __LINE__);

     }

     switch(choice[0]) {
     case 'M':
         return MOTHER_BOARD;
     case 'S':
         return SM_MODULE;
     case 'W':
         return WIC_MODULE;
         break;
     case 'V':
         return VM_MODULE;
     default:
         assert(!"user enter Invalid device choice for device type");
     }
     return -1;
 }

/* check if this is Act1/QUACK, or ACT2 */
int is_act2(void)
{
     int version = act2_version(0);

     if (version < 0x17) {
         printf("is_act2: ACT1 version = %#x\n", version);
         return 0;
     }
     printf("is_act2: ACT2 version = %#x\n", version);
     return 1;
}

 /*------------------------------------------------------------------------
  * smartchip
  *
  * DESCRIPTION:
  *  menu to allow user to select various test for smart chip.
  * 
  * PARAMETERS:
  *     op - type of operation to be performed.
  *
  * RETURNS:
  *     PASSED - if successful
  *     FAILED - if unsuccessful
  *--------------------------------------------------------------------------*/
 int
 smartchip (int submenu_flag)
 {
     char choice[5], type;
     int i, slot, rc = PASSED;
     uchar cookie[COOKIE_SIZE_512];
     sc_context *con, cont;
     dev_if_info_t dev_if;
     struct ngio_intf_t *ngio;
     
     con = &cont;
     con->dev_if_p = &dev_if;
     con->dev_if_p->cookie_size = COOKIE_SIZE_512;
     con_gl = con;

     testname ("Smart Cookie");
     if (submenu_flag == 0)
         prpass(testpass, "Main Menu");  
     else
         prpass(testpass, "Sub Menu");
     if (get_max_sm_slots() == 0) {
         printf("\nEnter (M)otherboard, "
                "(W)ic, (V)M, Wic (D)aughter card, I(L)P (Q)UIT  >");
     } else {
         printf("\nEnter (M)otherboard, (S)M Module, "
                "(W)ic, (V)M, Wic (D)aughter card, I(L)P, "
                "SM daughter(C)ard, (Q)UIT  >");
     }
     i = get_line(choice, sizeof(choice) + 1);
     if ( i == 1) { /* The user doesn't enter the slot number */
         choice[1] = 'f'; /* dummy slot */
     }
     slot = atoh(choice[1]);

     switch(choice[0]) {
     case 'M':
     case 'm':
         /*ACT2 library*/
         if (plat_init_smart_eeprom_context(con, MOTHER_BOARD,
                                            0, cookie) == FAILED) {
             return FAILED;
         }
        act2_init_cont((void*)con);  /* must be called after plat_init */
        
        if (choice[0] == 'm' ) {
            return (act2_prog(0));
        }
        
        if (submenu_flag == 0) {
            if (smart_cookie_program_dig_sign_mainmenu(con)) {
                rc = FAILED;
            } else {
                if (smart_cookie_authenticate(con)) {
                    rc = FAILED;
                }
            }
        } else {
            show_smart_cookie_submenu();
        }
        break;

     case 'S':
     case 's':
     case 'C':
     case 'c':
        if (get_max_sm_slots() == 0) {
            goto Default;
        }

        if((slot < FIRST_SLOT) || (slot > MAX_SM+FIRST_SLOT)) {
            slot = gethex_answer("\nEnter SM Slot Number: ",
                                 FIRST_SLOT, FIRST_SLOT, MAX_SM+FIRST_SLOT);
        }

        ngio = (struct ngio_intf_t *)slot_get_ngiosm(slot);
        
        if (!ngiosm_present((void *)ngio)) {
	    cterr('f', 0, "No Service Module in slot %d.", slot);
            return FAILED;
        }
        if ((ngiosm_enable((void *)ngio)) < 0) {
            cterr('f', 0, "Unable to power SM module slot %d", slot);
            return FAILED;
        }
        if ((ngiosm_i2c_unreset((void *)ngio)) < 0) {
            cterr('f', 0, "Unable to unreset SM module slot %d", slot);
            return FAILED;
        }
        if (choice[0] == 'C' || choice[0] == 'c') {
            //slot = 1;
            type = SM_DAUGHTER_CARD;
        } else 
            type = SM_MODULE;

        if (plat_init_smart_eeprom_context(con, type, slot, cookie) == FAILED) {
            return FAILED;
        }
        
        act2_init_cont((void*)con);  /* must be called after plat_init */

        if (choice[0] == 's' || choice[0] == 'c' ) {
            return (act2_prog(0));
        } else {
            
            if (submenu_flag == 0) {
                if (smart_cookie_program_dig_sign_mainmenu(con)) {
                    rc = FAILED;
                } else {
                    if (smart_cookie_authenticate(con)) {
                        rc = FAILED;
                    }
                }
            } else {
                show_smart_cookie_submenu();
            }
        } /* else act2 */
        break;
    case 'V':
    case 'v':
        slot = FIRST_SLOT; /* only one pvdm slot */
        ngio = (struct ngio_intf_t *)slot_get_ngiovm(slot);
        
        if (!ngiovm_present(ngio)) {
	    cterr('f', 0, "No VM in slot.");
            return FAILED;
        }

        if ((ngiovm_i2c_unreset(ngio)) < 0) {
            cterr('f', 0, "Unable to unreset VM module.\n");
            return FAILED;
        }
        
        if (plat_init_smart_eeprom_context(con, VM_MODULE,
                                           slot, cookie) == FAILED) {
            return FAILED;
        }
        act2_init_cont((void*)con);  /* must be called after plat_init */

        if (choice[0] == 'v' ) {
            return (act2_prog(0));
        } else {
            if (submenu_flag == 0) {
                if (smart_cookie_program_dig_sign_mainmenu(con)) {
                    rc = FAILED;
                } else {
                    if (smart_cookie_authenticate(con)) {
                        rc = FAILED;
                    }
                }
            } else {
                show_smart_cookie_submenu();
            }
        } /* else is_act2 */
        
        break;
     case 'W':
     case 'w':
     case 'D':
     case 'd':
        if((slot < FIRST_SLOT) || (slot > MAX_WIC+FIRST_SLOT)) {
            slot = gethex_answer("\nEnter WIC Slot Number: ",
                                 FIRST_SLOT, FIRST_SLOT, MAX_WIC+FIRST_SLOT);
        }
        ngio = (struct ngio_intf_t *)slot_get_ngiowic(slot);
        
        if (!ngiowic_present(ngio)) {
	    printf("\nNo WIC in slot %d.\n", con->slot);
            return FAILED;
        }
        if ((ngiowic_enable(ngio)) < 0) {
            printf("Unable to power WIC module slot %d\n", slot);
            return FAILED;
        }
        if ((ngiowic_i2c_unreset(ngio)) < 0) {
            printf("Unable to unreset WIC  module slot %d\n", slot);
            return FAILED;
        }

        if (choice[0] == 'D' || choice[0] == 'd') {
            //slot = 1;
            type = WIC_DAUGHTER_CARD;
        } else 
            type = WIC_MODULE;
        
        if (plat_init_smart_eeprom_context(con,type, slot, cookie) == FAILED) {
            return FAILED;
        }
        act2_init_cont((void*)con);  /* must be called after plat_init */

        if (choice[0] == 'w' || choice[0] == 'd') {
            return (act2_prog(0));
        } else {
            if (submenu_flag == 0) {
                if (smart_cookie_program_dig_sign_mainmenu(con)) {
                    rc = FAILED;
                } else {
                    if (smart_cookie_authenticate(con)) {
                        rc = FAILED;
                    }
                }
            } else {
                show_smart_cookie_submenu();
            }
        } /* else is_act2 */
        
        break;

     case 'E':
     case 'e':
        printf("This function is not available\n");
        return FAILED;
	break;
     case 'l':
     case 'L':
         if (plat_init_smart_eeprom_context(con, DAUGHTER_CARD,
                                            0, cookie) == FAILED) {
             return FAILED;
         }
         act2_init_cont((void*)con);  /* must be called after plat_init */
         if (choice[0] == 'l') {
             return (act2_prog(0));
         } else {
             if (submenu_flag == 0) {
                 if (smart_cookie_program_dig_sign_mainmenu(con)) {
                     rc = FAILED;
                 } else {
                     if (smart_cookie_authenticate(con)) {
                         rc = FAILED;
                     }
                 }
             } else {
                 show_smart_cookie_submenu();
             }
         }
         break;
     case 'a':
     case 'A':
         if((slot < FIRST_SLOT) || (slot > MAX_WIC+FIRST_SLOT)) {
            slot = gethex_answer("\nEnter WIC Slot Number: ",
                                 FIRST_SLOT, FIRST_SLOT, MAX_WIC+FIRST_SLOT);
        }
        ngio = (struct ngio_intf_t *)slot_get_ngiowic(slot);
        
        if (!ngiowic_present(ngio)) {
	    printf("\nNo WIC in slot %d.\n", con->slot);
            return FAILED;
        }
        if ((ngiowic_enable(ngio)) < 0) {
            printf("Unable to power WIC module slot %d\n", slot);
            return FAILED;
        }
        if ((ngiowic_i2c_unreset(ngio)) < 0) {
            printf("Unable to unreset WIC  module slot %d\n", slot);
            return FAILED;
        }

        if (choice[0] == 'D' || choice[0] == 'd') {
            //slot = 1;
            type = WIC_DAUGHTER_CARD;
        } else 
            type = WIC_MODULE;
        
        if (plat_init_smart_eeprom_context(con,type, slot, cookie) == FAILED) {
            return FAILED;
        }
        act2_init_cont((void*)con);  /* must be called after plat_init */

        if (choice[0] == 'w' || choice[0] == 'd') {
            return (act2_prog(0));
        } else {
            if (submenu_flag == 0) {
                if (smart_cookie_program_dig_sign_mainmenu(con)) {
                    rc = FAILED;
                } else {
                    if (smart_cookie_authenticate(con)) {
                        rc = FAILED;
                    }
                }
            } else {
                show_smart_cookie_submenu();
            }
        } /* else is_act2 */
        
        break;

     case 'Q':
	break;
  Default:
    default:
	printf("Invalid input %s\n", choice);    
        break;
    }

    return(rc);

}

#ifdef AUTHENTICATION_TEST_Y /* For EDVT with retry */
/*------------------------------------------------------------------------
 * smartchip_authenticate
 *
 * DESCRIPTION:
 *  menu to allow user to select various test for smart chip.
 * 
 * PARAMETERS:
 *     type - type of module
 *     slot - slot number
 *
 * RETURNS:
 *     PASSED - if successful
 *     FAILED - if unsuccessful
 *--------------------------------------------------------------------------*/
int
smartchip_authenticate(uchar type, uchar slot)
{
    int rc = PASSED;
    uchar cookie[COOKIE_SIZE_512];
    sc_context *con, cont;

    con = &cont;
    con_gl = con;
    if (plat_init_smart_eeprom_context(con, type, slot, cookie) == FAILED) {
	return FAILED;
    }
    if (smart_cookie_authenticate(con)) {
        rc = FAILED;
    }
    return(rc);

}
#endif 
/*------------------------------------------------------------------------
 * smartchip_authenticate_retest
 *
 * DESCRIPTION:
 *  menu to allow user to select various test for smart chip.
 * 
 * PARAMETERS:
 *     type - type of module
 *     slot - slot number
 *
 * RETURNS:
 *     PASSED - if successful
 *     FAILED - if unsuccessful
 *--------------------------------------------------------------------------*/
type_t
smartchip_authenticate_retest(uchar type, uchar slot)
{
    int rc = PASSED;
    uchar cookie[COOKIE_SIZE_512];
    sc_context *con, cont;
    dev_if_info_t dev_if;
 
    con = &cont;
    con->dev_if_p = &dev_if;
    con->dev_if_p->cookie_size = COOKIE_SIZE_512;
    con_gl = con;
    if (plat_init_smart_eeprom_context(con, type, slot, cookie) == FAILED) {
	return FAILED;
    }

    if (smart_cookie_authenticate_retest(con)) {
	    rc = FAILED;
    }
    return(rc);

}
/*------------------------------------------------------------------------
 * proauth
 *
 * DESCRIPTION: This function allows user to type in a CLI command at the
 *              diagmon> prompt to program and authenticate smart chip
 * 
 * PARAMETERS: argc = number of parameter
 *             argv - array to hold the command
 *
 * RETURNS:
 *     PASSED - if successful
 *     FAILED - if unsuccessful
 *--------------------------------------------------------------------------*/
int
progauth (int argc,char *argv[])
{
    int i;
    register char *cptr, c;
    char pa_bay[2], slot[2], sub_slot[2];
    /* Check if the user type in correct number of parameters */
    if ((argc > 4) || (argc < 2)){ 
	printf("Too few or too many parameters. Must be from 1 to 3 "
	       "parameters\n");
	progauth_help();
	return FAILED;
    } 
    for (i = 1; i<argc; i++) {
	cptr = argv[i];
	c = *cptr;
	c = toupper(c);
	
	switch (c) {
	case 'M':
	    /* For motherboard, don't need to type slot number */
	    if (strlen(argv[i]) != 1) {
		printf("For motherboard, the commnad is: progauth m\n");
		progauth_help();
		return FAILED;
	    }
	case 'N':
	case 'W':
	case 'A':
	case 'E':
	case 'P':
	    break;
	case 'D':  /* Daughter card on SM */
	    if (i < 2) {
		printf("Daughtercard is not supported on motherboard, %s\n",
		       argv[i]);
		progauth_help();
		return FAILED;
	    } else {
		break;
	    }
	case '?': /* For help */
	    progauth_help();
	    return FAILED;
	default:
	    printf("Invalid input, %s, parameter %d.\n", argv[i], i);
	    progauth_help();
	    return FAILED;
	}
	/* Check if every parameter has a right size */
	if ((strlen(argv[i]) != 2)) {
	    if ((c != 'M') && (c != '?')) {
		printf("Parameter sizes must be 2, you type in %s\n", argv[i]);
		progauth_help();
		return FAILED;
	    }		    
	} 
    	
    }	
    /* Call different functions for different numbers of parameters */
    switch (argc) {
    case 2:	
	strcpy(pa_bay, argv[1]);
	if(progauth_pabay(pa_bay))
	    return FAILED;
	break;
    case 3:
	strcpy(pa_bay, argv[1]);
	strcpy(slot, argv[2]);
	if(progauth_pabay_slot(pa_bay, slot))
	    return FAILED;
	break;
    case 4:
	strcpy(pa_bay, argv[1]);
	strcpy(slot, argv[2]);
	strcpy(sub_slot, argv[3]);
	printf("This configuration is not supported yet\n");
	break;
    default:
	break;
    }	
    return PASSED;
    
}    
/*------------------------------------------------------------------------
 * proauth_pabay
 *
 * DESCRIPTION: This function allows user to program and authenticate
 *              the smart chip on modules plugging to motherboard
 * 
 * PARAMETERS: pa_bay = the array holding the type and slot number
 *
 * RETURNS:
 *     PASSED - if successful
 *     FAILED - if unsuccessful
 *--------------------------------------------------------------------------*/
static int
progauth_pabay(char *pa_bay)
{
    int slot_start, max_pvdm_slot, rc = PASSED;
    int pa_num = 0;
    uchar cookie[COOKIE_SIZE_512];
    sc_context *con, cont;
    struct ngio_intf_t *ngio;
    dev_if_info_t dev_if;
    testname ("Smart Cookie");
    con = &cont;
    con->dev_if_p = &dev_if;
    con_gl = con;
    pa_bay[0] = toupper(pa_bay[0]);
    
    if (isdigit(pa_bay[1])) {
	pa_num = atoh(pa_bay[1]);
    } else {
	if ((pa_bay[0] != 'M') && (pa_bay[0] != '?')) {
	    printf("Slot number %c is not a digit\n", pa_bay[1]);
	    progauth_help();
	    return FAILED;
	}    
    }
	
    switch(pa_bay[0]) {
    case 'M':
	if (plat_init_smart_eeprom_context(con, MOTHER_BOARD,
                                           0, cookie) == FAILED) {
            return FAILED;
        }
	if (smart_cookie_program_dig_sign_mainmenu(con)) {
	    rc = FAILED;
	} else {
	    if (smart_cookie_authenticate(con)) {
		rc = FAILED;
	    }
	}
       	break;
    case 'N':
        slot_start = FIRST_SLOT;
        if ((pa_num < slot_start) || (pa_num > get_max_sm_slots())) {	    
	    cterr('f', 0, "Invalid NM slot number = %d, the valid range"
		  " is from %d to %d", pa_num, slot_start, get_max_sm_slots());
	    return FAILED;
	}

        ngio = (struct ngio_intf_t *)slot_get_ngiosm(pa_num);
        
	if (plat_init_smart_eeprom_context(con, NETWK_MODULE,
                                           pa_num, cookie) == FAILED) {
            return FAILED;
        }

        if (!ngiosm_present(ngio)) {
	    cterr('f', 0, "progauth_pabay: No Service Module in slot %d.", pa_num);
            return FAILED;
        }
        if ((ngiosm_enable(ngio)) < 0) {
            cterr('f', 0, "progauth_pabay: Unable to power SM module slot %d", pa_num);
            return FAILED;
        }
        if ((ngiosm_i2c_unreset(ngio)) < 0) {
            cterr('f', 0, "progauth_pabay: Unable to unreset SM module slot %d", pa_num);
            return FAILED;
        }

	if (smart_cookie_program_dig_sign_mainmenu(con))
	    return FAILED;
	if (smart_cookie_authenticate(con))
	    return FAILED;
	break;
    case 'W':
	if((pa_num < 0) || (pa_num > MAX_WIC)) {
	    cterr('f', 0, "Invalid H/V/WIC slot number = %d, the valid range"
		  " is from 0 to %d",
		  pa_num, MAX_WIC);
	    return FAILED; 
	}	
        if (plat_init_smart_eeprom_context(con, WIC_MODULE,
					   pa_num, cookie) == FAILED) {
            return FAILED;
        }
	if (smart_cookie_program_dig_sign_mainmenu(con))
	    return FAILED;
	if (smart_cookie_authenticate(con))
	    return FAILED;
	break;
    case 'I':
    case 'E':
        cterr('f', 0, "This function is not available");
	return FAILED;
    case 'P':
        max_pvdm_slot = get_max_num_vm();
        if ((pa_num < FIRST_SLOT) || (pa_num > (MAX_VM))) {
	    cterr('f', 0, "Invalid PVDM slot number = %d, the valid range"
		  " is from 0 to %d", pa_num, max_pvdm_slot - 1);
	    return FAILED;
	}

        if (plat_init_smart_eeprom_context(con, VM_MODULE,
                                           pa_num, cookie) == FAILED) {
            return FAILED;
        }
	if (smart_cookie_program_dig_sign_mainmenu(con))
	    return FAILED;
	if (smart_cookie_authenticate(con))
	    return FAILED;
	break;
    default:
        break;
    }

    return (rc);

}

/*------------------------------------------------------------------------
 * progauth_pabay_slot
 *
 * DESCRIPTION: This function allows user to program and authenticate
 *              the smart chip on WIC, PVDM, and daughtercard  plugging
 *              to network module
 * 
 * PARAMETERS: pa_bay = the array holding type and slot number of the NM
 *             slot - the array holding type and slot number of the WIC,
 *             PVDM or daughtercard
 *
 * RETURNS:
 *     PASSED - if successful
 *     FAILED - if unsuccessful
 *--------------------------------------------------------------------------*/
static int
progauth_pabay_slot(char *pa_bay, char *slot)
{
    int pa_num, pa_start;
    int slot_num;
    struct nm_sc_info sc_slot_info_tbl[MAX_SM + 1];
    struct nm_sc_info *sc_slotp;
    sc_context *con, cont;
    struct ngio_intf_t *ngio;
    
    testname ("Smart Cookie");
    con = &cont;
    con_gl = con;
    
    pa_bay[0] = toupper(pa_bay[0]);
    pa_num = atoh(pa_bay[1]);
    
    slot[0] = toupper(slot[0]);
    if (isdigit(slot[1])) {
	slot_num = atoh(slot[1]);
    } else {
	printf("Slot number %c is not a digit\n", slot[1]);
	progauth_help();
	return FAILED;
    }
    
    switch(pa_bay[0]) {
    case 'N':
	pa_start = FIRST_SLOT;
        if ((pa_num < pa_start) || (pa_num > get_max_sm_slots())) {	    
	    cterr('f', 0, "Invalid NM slot number = %d, the valid range"
		  " is from %d to %d", pa_num, pa_start, get_max_sm_slots());
	    return FAILED;
	}

        ngio = (struct ngio_intf_t *)slot_get_ngiosm(pa_num);
        
        if (!ngiosm_present(ngio)) {
	    cterr('f', 0, "No Service Module in slot %d.", pa_num);
            return FAILED;
        }
        if ((ngiosm_enable(ngio)) < 0) {
            cterr('f', 0, "Unable to power SM module slot %d", pa_num);
            return FAILED;
        }
        if ((ngiosm_i2c_unreset(ngio)) < 0) {
            cterr('f', 0, "Unable to unreset SM module slot %d", pa_num);
            return FAILED;
        }

	switch(slot[0]) {
	case 'W': /* H/V/WIC */
	    sc_slotp = &sc_slot_info_tbl[pa_num];
	    (*sc_slotp->wic_sc_progauth)(pa_num, slot_num, TRUE);
	    break;
	case 'P': /* PVDM */
	    sc_slotp = &sc_slot_info_tbl[pa_num];
	    (*sc_slotp->pvdm_sc_progauth)(pa_num, slot_num, TRUE);
	    break;
	case 'D': /* Daughtercard */
	    sc_slotp = &sc_slot_info_tbl[pa_num];
	    (*sc_slotp->daughter_sc_progauth)(pa_num, slot_num, TRUE);
	    break;    
	default:
	    break;
	}    
	break;
    default:
	cterr('f', 0, "This configuration is currently not supported.");
        break;
    }
    return PASSED;

}
/*------------------------------------------------------------------------
 * progauth_error_msg
 *
 * DESCRIPTION: This function print out the error message for unsupported
 *              configuration
 * 
 * PARAMETERS: none
 *
 * RETURNS: FAILED
 *--------------------------------------------------------------------------*/
int
progauth_error_msg(void)
{
    cterr('f', 0, "This configuration is not supported."); 
    return FAILED;
}    
/*------------------------------------------------------------------------
 * progauth_help
 *
 * DESCRIPTION: This function print out the help for the progauth command
 * 
 * PARAMETERS: none
 *
 * RETURNS: None
 *--------------------------------------------------------------------------*/
void
progauth_help(void)
{
    printf("\n");
    printf("usage: progauth TypeSlot [TypeSlot] [TypeSlot]\n"); 
    printf("       Type=a,d,e,m,n,p,w.\n");
    printf("       a = Aim, d = Daughtercard, e = ETM, m = motherboard\n");
    printf("       n = network module, p = PVDM, w = WIC\n");
    printf("       Slot=0...max\n");
    printf("       Exception: for motherboard, you only need Type\n");
    printf("\n");
    printf("Examples:\n");
    printf("diagmon 7 > progauth m        -----> to program"
	   " and authenticate motherboard\n");
    printf("diagmon 8 > progauth p0       -----> to program"
	   " and authenticate PVDM0 on motherboard\n");
    printf("diagmon 9 > progauth w1       -----> to program"
	   " and authenticate WIC0 on motherboard\n");
    printf("diagmon 10 > progauth n1 w0   -----> to program"
	   " and authenticate WIC0 on NM slot 1\n");
    printf("diagmon 11 > progauth n2 p2   -----> to program"
	   " and authenticate PVDM2 on NM slot 2 (for Soprano)\n");
    printf("diagmon 12 > progauth n2 d0   -----> to program"
	   " and authenticate daughtercard 0 on NM slot 2 (for Venom)\n");
}    

/*-----------------------------------------------------------------------
 * imc_format_test_cmd
 * 
 * Description:
 *  This function format the Atmel's Read command
 *  
 * Parameters:
 *  command      - Pointer to the command buffer
 *  test_pattern - a test value required for  Atmel's Read command
 *  
 * Returns:
 *   None. 
 *-----------------------------------------------------------------------*/
void
imc_format_test_cmd(uchar *command, uchar test_pattern)
{
    command[0] = ATMEL_START_BIT;
    command[1] = ATMEL_EEPROM_READ_CMD;
    command[2] = test_pattern;
    command[3] = 0;
}

/********************************************************************
 *  alt_sm_dc_cookie
 *
 *  This function alters SM's daughter card cookie
 *
 *  Inputs:  slot, daughter card , mode (CLI / MENU) , cli structure
 *
 *  Outputs: pass if program sucessfully; else fail
 *
 ********************************************************************
 */
int
alt_sm_dc_cookie (int slot, int em_slot, boolean mode, cli_cookie_cmd *cmd)
{
    uchar cookie_contents[COOKIE_SIZE_512];
    sc_context *con, cont;
    dev_if_info_t dev_if;
    int len = 0;
    struct ngio_intf_t *ngio;

    
    if(mode == CLI_MODE) {
        if((cmd != NULL) && (cmd->cli_mode == CLI_DISCOVERY)) {
            strcpy(cmd->buf, "SM");
            len = strlen(cmd->buf);
            sprintf(&cmd->buf[len], "%d:DC%d:", slot, em_slot);
        }
    }

    ngio = (struct ngio_intf_t *)slot_get_ngiosm(slot);	
    ngiosm_unreset(ngio);

    con = &cont;
    con->dev_if_p = &dev_if;
    con->dev_if_p->cookie_size = COOKIE_SIZE_512;

    if ( plat_init_smart_eeprom_context (con, SM_DAUGHTER_CARD,
                                    slot, (uchar *)cookie_contents)) {
        return(FAILED);
    }
    msleep(10);

    if (smart_cookie_read_write_eeprom(con, cmd)) {
         return(FAILED);
    }
    return (PASSED);
}

/********************************************************************
 *  authen_nm_em_cookie
 *
 *  This function alters NM's daughter card cookie
 *
 *  Inputs:  slot, daughter card
 *
 *  Outputs: pass if program sucessfully; else fail
 *
 ********************************************************************
 */
int
authen_nm_em_cookie (int slot, int em_slot, boolean submenu_flag)
{
    int   slot_start;
    uchar  cookie[COOKIE_SIZE_512];
    sc_context *con, cont;
    dev_if_info_t dev_if;
    struct ngio_intf_t *ngio;


    testname ("Daughter Card Smart Cookie");
    slot_start = FIRST_SLOT;

    if ((slot < slot_start) || (slot > get_max_sm_slots())) {	    
        slot = gethex_answer("\nEnter Slot Number: ", 
    			     slot_start, slot_start, (get_max_sm_slots()));
    }

    con = &cont;
    con_gl = con;
    con->dev_if_p = &dev_if;
    con->dev_if_p->cookie_size = COOKIE_SIZE_512;

    if (plat_init_smart_eeprom_context(con, NETWK_MODULE,
                                       slot, cookie) == FAILED) {
        return (FAILED);
    }

    ngio = (struct ngio_intf_t *)slot_get_ngiosm(slot);
    
    if (!ngiosm_present(ngio)) {
        cterr('f', 0, "No Service Module in slot %d.", con->slot);
        return FAILED;
    }
    if ((ngiosm_enable(ngio)) < 0) {
        cterr('f', 0, "Unable to power SM module slot %d", slot);
        return FAILED;
    }
    if ((ngiosm_i2c_unreset(ngio)) < 0) {
        cterr('f', 0, "Unable to unreset SM module slot %d", slot);
        return FAILED;
    }

    con->type = DAUGHTER_CARD;
    sprintf(con->info_string, "NM %d EM", slot);

    if (submenu_flag == 0) {
        if (smart_cookie_program_dig_sign_mainmenu(con)) {
    	return (FAILED);
        }
        if (smart_cookie_authenticate(con)) {
            return (FAILED);
        }
    } else {
        show_smart_cookie_submenu();
    }

    return(PASSED);

}

/********************************************************************
 *  authen_sm_dc_cookie
 *
 *  This function alters SM's daughter card cookie
 *
 *  Inputs:  slot, daughter card
 *
 *  Outputs: pass if program sucessfully; else fail
 *
 ********************************************************************
 */
int
authen_sm_dc_cookie (int slot, int em_slot, boolean submenu_flag)
{
    int   slot_start;
    uchar  cookie[COOKIE_SIZE_512];
    sc_context *con, cont;
    dev_if_info_t dev_if;
    struct ngio_intf_t *ngio;
 
    testname ("Daughter Card Smart Cookie");
    slot_start = FIRST_SLOT;

    if ((slot < slot_start) || (slot > get_max_sm_slots())) {	    
        slot = gethex_answer("\nEnter Slot Number: ", 
    			     slot_start, slot_start, get_max_sm_slots() );
    }

    con = &cont;
    con_gl = con;
    con->dev_if_p = &dev_if;
    con->dev_if_p->cookie_size = COOKIE_SIZE_512;

    if (plat_init_smart_eeprom_context(con, SM_DAUGHTER_CARD,
                                       slot, cookie) == FAILED) {
        return (FAILED);
    }

    ngio = (struct ngio_intf_t *)slot_get_ngiosm(slot);
    
    if (!ngiosm_present(ngio)) {
        cterr('f', 0, "progauth_pabay: No Service Module in slot %d.", slot);
        return FAILED;
    }
    if ((ngiosm_enable(ngio)) < 0) {
        cterr('f', 0, "progauth_pabay: Unable to power SM module slot %d", slot);
        return FAILED;
    }
    if ((ngiosm_i2c_unreset(ngio)) < 0) {
        cterr('f', 0, "progauth_pabay: Unable to unreset SM module slot %d", slot);
        return FAILED;
    }


    if (submenu_flag == 0) {
        if (smart_cookie_program_dig_sign_mainmenu(con)) {
            return FAILED;
	 } else {
            if (smart_cookie_authenticate(con)) {
                return FAILED;
            }
        }
    } else {
        show_smart_cookie_submenu();
    }

    return(PASSED);
}


/*------------------------------------------------------------------------
 * i2c_scc_process_cmd
 *
 * Description:
 *   This is a main function to send SCC command to the I2C interface to
 * the SCC
 *
 * Parameters:
 *   cmd         - Pointer to the command bufer to be sent to SCC
 *   cmd_length  - Length of the command
 *   resp_buffer - Pointer to the response message buffer
 *   resp_length - Expected length the response message
 *
 * Returns:
 *   Sending status of the message
 *--------------------------------------------------------------------------*/
static scc_return_status_t
i2c_scc_process_cmd (sc_context *con, void *cmd_buffer, ushort cmd_length,
                    uchar *resp_buffer, uint resp_length)
{
    scc_return_status_t status;
    int i;
    uchar tmp_buffer[MAX_N2G_QCK_MSG_SIZE];
    uchar *rx_buff_ptr = resp_buffer;
    ushort rx_length;
    uint timeout_ctr, rx_ctr;

    if (act2_is_simple_mode(con)) {
        return i2c_scc_process_cmd_simple(con, cmd_buffer, cmd_length,
                            resp_buffer, resp_length);
    }
    /*
     * Send the Command to SCC
     */
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\nSend new request to SCC respons len %d", resp_length);
    }
    status = send_i2c_cmd(con, cmd_buffer, cmd_length, resp_buffer,
                          resp_length);
    if (status != SCC_OK) {
        return status;
    }
    /*
     * Based on command type, we wait for various length of time
     */
    scc_delay_for_cmd_processing(*(uchar *)cmd_buffer);
    /*
     * Poll SCC for answer
     */
    rx_length = (ushort)resp_length;
    timeout_ctr = N2G_QCK_MAX_POLLING_TIMEOUT_CTR;
    rx_ctr = 0;
    while ((rx_length > 0) && (timeout_ctr-- > 0)) {
        /*
         * Tell the Device Driver Dependent layer to read 4 bytes
         */
        if (i2c_scc_read_bytes(con, tmp_buffer)) {
            return SCC_TIMEOUT;
        }
        smart_cookie_delay(WAIT_FOR_N2G_SCC_PROCESS_CMD);
        if ((NVRAM)->diagflag & D_VERBOSE) {
            if (tmp_buffer[0] > 0) {
                printf("\n rx-Char:%02x %02x %02x %02x: Rx-length:%d",
                       tmp_buffer[0], tmp_buffer[1], tmp_buffer[2],
                       tmp_buffer[3], rx_length);
            }
        }
        if ((tmp_buffer[0] > 0) && (tmp_buffer[0] < MAX_N2G_QCK_MSG_SIZE)) {
            /*
             * If we get good data bytes, we do following:
             * 1) put data to data buffer for storage
             * 2) do counter adjustment.
             * 3) Check flow-control window
             */
            for (i = 1; i <= tmp_buffer[0]; i++) {
                *rx_buff_ptr = tmp_buffer[i];
                rx_buff_ptr++;
                rx_length--;
                /*
                 * Check for NAK Messages.
                 * If NAK Message is received, then it requires to
                 * adjust the length of the Response Message
                 */
                if (rx_ctr == 0) {
                    update_nak_ack_resp_msg_len((uchar)*rx_buff_ptr,
                                                &rx_length);
                }
                rx_ctr++;
                timeout_ctr = N2G_QCK_MAX_POLLING_TIMEOUT_CTR;
            }
            /*
             * Based on number of bytes received, we embed flow control
             * message to the communication
             */
            if (((rx_ctr % flow_control_window_size) == 0) &&
                 (rx_ctr != resp_length)){
                 tmp_buffer[0] = FLOW_CONTROL_GET_NEXT;
                 if(send_i2c_cmd(con, tmp_buffer, 1, NULL, 0)) {
                     return SCC_TIMEOUT;
                 }
                 /* delay to make sure SCC ready for next transaction */
                 smart_cookie_delay(WAIT_FOR_GET_NEXT_CMD);
            }
        } else if (tmp_buffer[0] == 0) {
            smart_cookie_delay(WAITING_FOR_REPLY_MSG);
        } else {
            printf("Invalid Message Size: %0x\n", tmp_buffer[0]);
            smart_cookie_delay(WAITING_FOR_REPLY_MSG);
        }
    }
    if (timeout_ctr > 0) {
        return SCC_OK;
    } else {
         return SCC_TIMEOUT;
    }
}

static scc_return_status_t
i2c_scc_process_cmd_simple(sc_context *con, void *cmd_buffer, ushort cmd_length,
                    uchar *resp_buffer, uint resp_length)
{
    scc_return_status_t status;

    /*
     * Send the Command to SCC
     */
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\nSend new request to SCC simple");
    }

    status = send_i2c_cmd_simple(con, cmd_buffer, cmd_length, resp_buffer,
                          resp_length);
    if (status != SCC_OK) {
        return status;
    }
    /*
     * Based on command type, we wait for various length of time
     */

    if (!act2_drv_read (con, resp_buffer,resp_length)) {
        return SCC_TIMEOUT;
    }
    
    return SCC_OK;

}

/*------------------------------------------------------------------------
 * send_i2c_cmd
 *
 * Description:
 *   Write the command to the SCC via I2C interface
 *
 * Parameters:
 *   con         - sc_context pointer
 *   cmd         - Pointer to the command bufer to be sent to SCC
 *   cmd_length  - Length of the command
 *   resp_buffer - Pointer to the response message buffer
 *   resp_length - Expected length the response message
 *
 * Returns:
 *   scc_return_status_t - command status return
 *--------------------------------------------------------------------------*/
scc_return_status_t
send_i2c_cmd (sc_context *con, void *cmd_buffer, uint cmd_length,
              void *resp_buffer, uint resp_length)
{
    ushort send_bytes = cmd_length;
    unsigned char   i2c_cmd[MAX_N2G_QCK_MSG_SIZE];
    uchar imc_msg_id = IMC_SPI_CMD_START;
    uchar *tmp_cmd_buffer = (uchar *)cmd_buffer;
    int i, tx_size;

    if (act2_is_simple_mode(con)) {
        return send_i2c_cmd_simple(con, cmd_buffer, cmd_length,
                            resp_buffer, resp_length);
    }
    while (send_bytes > 0) {
        /*
        * Formatting the transport command message
        */
       i2c_cmd[0] = imc_msg_id;
       if (send_bytes > (N2G_SCC_TX_MSG_SIZE - 1)) {
           tx_size = N2G_SCC_TX_MSG_SIZE - 1;
       } else {
           tx_size = send_bytes;
       }
       if ((NVRAM)->diagflag & D_VERBOSE) {
           printf("\nSend-data: %02x: ", i2c_cmd[0]);
       }
        for (i = 1; i <= tx_size; i++) {
            i2c_cmd[i] = *tmp_cmd_buffer++;
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("%02x ", i2c_cmd[i]);
            }
       }
        /*
        * Forward the Sending command to the Device Dependent Layer
        */
        if (i2c_scc_write_bytes(con, &i2c_cmd[0], N2G_SCC_TX_MSG_SIZE)) {
            return SCC_TIMEOUT;
        }
        send_bytes -= tx_size;
        imc_msg_id = IMC_SPI_CMD_CONTD;
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("  bytes-left: %d", send_bytes);
        }
        /*
         * Wait for the N2G-SCC to process the command
         */
        smart_cookie_delay(WAIT_FOR_N2G_SCC_PROCESS_CMD);
    }
     /*
     * Notify to the N2G-SCC that this is the end of the SCC command sequence
     */
    i2c_cmd[0] = IMC_SPI_CMD_END;
    if (i2c_scc_write_bytes(con, i2c_cmd, 1)) {
        return SCC_TIMEOUT;
    }
    smart_cookie_delay(WAIT_FOR_N2G_SCC_PROCESS_CMD);
    return SCC_OK;
}

scc_return_status_t
send_i2c_cmd_simple (sc_context *con, void *cmd_buffer, uint cmd_length,
              void *resp_buffer, uint resp_length)
{

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf(" simple mode:  bytes-left: %d", cmd_length);
    }
    
    if (!act2_drv_write (con, cmd_buffer , cmd_length)) {
        cterr('f', 0, "Failed to write into IMC2");
        return SCC_TIMEOUT;
    }

    return SCC_OK;
}


/*
 * scc_delay_for_cmd_processing:
 *
 * Description:
 *    Based on SCC CMD type, put various delay length. During this wait time
 *    SCC supposes to finish preparing answers for host's next enquiry.
 *
 * Parameters:
 *    cmd_type - Command type
 *
 * Returns:
 *    None
 */
void scc_delay_for_cmd_processing (uchar cmd_type)
{
    switch (cmd_type) {
    case REQUEST_SIGN_MESSAGE:
    case REQUEST_SIGN_MESSAGE_32B:
    case REQUEST_SIGN_MSG_DIGEST:
         smart_cookie_delay(WAIT_FOR_RANDOM_NUMBER_SIGNING * 2);
         break;
    case EEPROM_PAGE_LOCK_DOWN:
         smart_cookie_delay(WAIT_FOR_EEPROM_LOCKING);
         break;
    default:
         smart_cookie_delay(WAIT_DEFAULT_TIME);
         break;
    }
    return;
}
/*------------------------------------------------------------------------
 * i2c_scc_read_bytes
 *
 * Description:
 *   Read bytes from the SCC via I2C interface
 *
 * Parameters:
 *   con - sc_context pointer
 *   read_buffer - buffer to hold the data
 *
 * Returns:
 *   PASSED/FAILED
 *--------------------------------------------------------------------------*/
static int
i2c_scc_read_bytes(sc_context *con, uchar *read_buffer)
{
    if (con->quack_read_2bytes(con, read_buffer)) {
        return (FAILED);
    }
    return (PASSED);
}
/*------------------------------------------------------------------------
 * i2c_scc_write_bytes
 *
 * Description:
 *   Write bytes to the SCC via I2C interface
 *
 * Parameters:
 *   con  - context pointer
 *   i2c_cmd - pointer to the command to be sent
 *   msg_size - size of the command
 *
 * Returns:
 *   PASSED/FAILED
 *--------------------------------------------------------------------------*/
static int
i2c_scc_write_bytes(sc_context *con_p, unsigned char *i2c_cmd, int msg_size)
{
    if (con_p->quack_write_2bytes(con_p, i2c_cmd, msg_size)) {
        return (FAILED);
    }
    return (PASSED);
}
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
    char *pcb, *pid, *chassis;
    uchar controller_type[4];
    char pcb_serial_val_init[12] = " NO PCB NUM";
    char pm_serial_num_ascii[12];
    char product_id_init[15] = " NO PRODUCT ID";
    uchar product_id[128];
    char chassis_serial_val_init[14] = "NO CHASSI NUM";
    uchar chassis_serial_num_ascii[14];
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
        case CHASSIS_SERIAL_NUM:
            if ((data_ptr = (uchar *)search_type_ret_addr_of_first_data
                 (con->cookie_contents, CHASSIS_SERIAL_NUM,
                  &bytes, FALSE)) == NULL) {
                chassis = chassis_serial_val_init;
                for ( i = 0; i < 13; i++)
                    chassis_serial_num_ascii[i] = *chassis++;
            } else {
                for (i = 0; i < bytes; i++) {
                    chassis_serial_num_ascii[i] = *data_ptr++;
                    printf("%02x ", chassis_serial_num_ascii[i]);
                }
            }
            break;
        default:
            break;
        }
    }
}

/*---------------------------------------------------------------------------
 * print_cookie_udi  
 *
 * DESCRIPTION:
 *     This function reads and displays UDI fields for credential
 * programming
 * 
 * PARAMETERS:
 *     con - context structure pointer 
 *     pcb_serial_num - array to hold PCB Serial number
 *     product_id - array to hold product ID
 *     version_id - array to hold version ID
 *
 * RETURNS:
 *     PASSED/FAILED
 *--------------------------------------------------------------------------*/

static int
print_cookie_udi(sc_context *con, char *pcb_serial_num,
		 char *chassis_serial_num, char *product_id,
		 char *version_id, boolean pcb_flag)
{
    uchar i, bytes, *data_ptr;
    char *pcb, *chassis, *pid, *vid;
    char pcb_serial_val_init[14] = " NO PCB NUM  ";
    char chassis_serial_val_init[14] = "NO CHASSI NUM";
    char product_id_init[15] = " NO PRODUCT ID";
    char version_id_init[15] = " NO VERSION ID";

    printf("\nUDI");
    
    /* Print PID */
    printf("\nPID:");
    if ((data_ptr = (uchar *)search_type_ret_addr_of_first_data
	 (con->cookie_contents, PRODUCT_ID, 
	  &bytes, FALSE)) == NULL) {
	pid = product_id_init;
	for ( i = 0; i < 15; i++) {
	    product_id[i] = *pid++;
	    printf("%c", product_id[i]);
	}
	return (FAILED);
    } else {
	for (i = 0; i < bytes; i++) {
	    product_id[i] = *data_ptr++;
	    if (product_id[i] != ' ') {
		printf("%c", product_id[i]);
	    }
	}
    }

    /* Print VID */
    printf("\nVID:");
    if ((data_ptr = (uchar *)search_type_ret_addr_of_first_data
	 (con->cookie_contents, VERSION_ID, 
	  &bytes, FALSE)) == NULL) {
	vid = version_id_init;
	for ( i = 0; i < 15; i++) {
	    version_id[i] = *vid++;
	    
	}
	return (FAILED);
    } else {
	for (i = 0; i < bytes; i++) {
	    version_id[i] = *data_ptr++;
	    if (version_id[i] != ' ') {
		printf("%c", version_id[i]);
	    }
	}
    }
    
    if (pcb_flag == TRUE) {
	/* Print PCB */
	printf("\nSN:");
	if ((data_ptr = (uchar *)search_type_ret_addr_of_first_data
	     (con->cookie_contents, PCB_SERIAL_NUM, 
	      &bytes, FALSE)) == NULL) {
	    pcb = pcb_serial_val_init;
	    for ( i = 0; i < 13; i++) {
		pcb_serial_num[i] = *pcb++;
		printf("%c", pcb_serial_num[i]);
	    }
	    return (FAILED);
	} else {
	    for (i = 0; i < bytes; i++) {
		pcb_serial_num[i] = *data_ptr++;
		if (pcb_serial_num[i] != ' ') {
		    printf("%c", pcb_serial_num[i]);
		}
	    }
	}
    } else {
	/* Print Chassis */
	printf("\nSN:");
	if ((data_ptr = (uchar *)search_type_ret_addr_of_first_data
	     (con->cookie_contents, CHASSIS_SERIAL_NUM, 
	      &bytes, FALSE)) == NULL) {
	    chassis = chassis_serial_val_init;
	    for ( i = 0; i < 13; i++) {
		chassis_serial_num[i] = *chassis++;
		printf("%c", chassis_serial_num[i]);
	    }
	    return (FAILED);
	} else {
	    for (i = 0; i < bytes; i++) {
		chassis_serial_num[i] = *data_ptr++;
		if (chassis_serial_num[i] != ' ') {
		    printf("%c", chassis_serial_num[i]);
		}
	    }
	}
    }
    return (PASSED);
}

/*---------------------------------------------------------------------------
 * progwdc_pabay
 *
 * DESCRIPTION:
 *     This function write the credential to the smart cookie EEPROM
 *
 * PARAMETERS:
 *      pa_bay = the array holding the type and slot number
 *
 * RETURNS:
 *     PASSED - if successful
 *     FAILED - if unsuccessful
 *--------------------------------------------------------------------------*/
static int
progwdc_pabay(char *pa_bay)
{
    int i, pa_num = 0;
    uchar cookie[COOKIE_SIZE_512];
    char chassis_serial_num[PCB_SERIAL_NUM_SIZE];
    char pcb_serial_num[PCB_SERIAL_NUM_SIZE];
    char product_id[PRODUCT_ID_SIZE];
    char version_id[VERSION_ID_SIZE];
    uchar  mask[5];
    ushort controller_id = 0;
    sc_context con, *con_p;
    struct ngio_intf_t *ngio;
    
    boolean pcb_flag = TRUE;
    dev_if_info_t dev_if;
    con_p = &con;
    testname ("Program Credential");

    /* clean up mask before using */
    memset(mask, 0, sizeof(mask));

    con_p->dev_if_p = &dev_if;
    con_p->dev_if_p->cookie_size = COOKIE_SIZE_512;
    /* In case the verification fails, mask out the WDC */
    mask[0] = SC_CREDENTIAL_ADDR_MB; /* offset MSB */
    mask[1] = SC_CREDENTIAL_ADDR_LB; /* offset LSB */
    mask[2] = 2;
    mask[3] = 0xFF;
    mask[4] = 0xFF;
    pa_bay[0] = toupper(pa_bay[0]);
    if (isdigit(pa_bay[1])) {
        pa_num = atoh(pa_bay[1]);
    } else {
        if ((pa_bay[0] != 'M') && (pa_bay[0] != '?')) {
            printf("Slot number %c is not a digit\n", pa_bay[1]);
            progauth_help();
            return FAILED;
        }
    }
    for (i = 0; i < PCB_SERIAL_NUM_SIZE; i++) {
        pcb_serial_num[i] = '\0';
    }
    for (i = 0; i < PCB_SERIAL_NUM_SIZE; i++) {
	chassis_serial_num[i] = '\0';
    }
    for (i = 0; i < PRODUCT_ID_SIZE; i++) {
        product_id[i] = '\0';
    }
    for (i = 0; i < VERSION_ID_SIZE; i++) {
        version_id[i] = '\0';
    }
    switch(pa_bay[0]) {
    case 'M':
        if (plat_init_smart_eeprom_context(con_p, MOTHER_BOARD,
                                           0, cookie) == FAILED) {
            return (FAILED);
        }
        break;
    case 'N':    /* For Bryce 2nd Quack */
        ngio = slot_get_ngiosm(pa_num);
        if (plat_init_smart_eeprom_context(con_p, NETWK_MODULE,
                                           pa_num, cookie) == FAILED) {
            return FAILED;
        }
        if (!ngiosm_present(ngio)) {
	    cterr('f', 0, "No Service Module in slot %d.", pa_num);
            return FAILED;
        }
        if ((ngiosm_enable(ngio)) < 0) {
            cterr('f', 0, "Unable to power SM module slot %d", pa_num);
            return FAILED;
        }
        if ((ngiosm_i2c_unreset(ngio)) < 0) {
            cterr('f', 0, "Unable to unreset SM module slot %d", pa_num);
            return FAILED;
        }

        break;
    default:
        printf("\nThis configuration does not support WDC yet\n");
        break;
    }
    
    if (check_quack_version(con_p)) {
        return (FAILED);
    }
    
    if (smart_cookie_read(con_p)) {
        return (FAILED);
    }

    if (pa_bay[0] == 'M') {
	controller_id = get_ctrl_id_from_cookie_contents(con_p->cookie_contents);
	
	for (i = 0; i < FRU_PLAT_MAX_IDS; i++) {
	    if (fru_platform_info_tbl[i].id == controller_id) {
		pcb_flag = FALSE;
		break;
	    }
	}
    }

    /* If PCB S/N is not used for SUDI, it will not be used for WDC
       so set the pcb_flag to FALSE here */
    if (pcb_for_sudi == FALSE) {
	pcb_flag = FALSE;
    }

    if (print_cookie_udi(con_p, pcb_serial_num, chassis_serial_num, product_id,
			 version_id, pcb_flag)) {
	printf("\nFailed to print UDI\n");
	return (FAILED);
    }
    return PASSED;
}

/*------------------------------------------------------------------------
 * progwdc_help
 *
 * DESCRIPTION: This function print out the help for the progwdc command
 *
 * PARAMETERS: none
 *
 * RETURNS: None
 *--------------------------------------------------------------------------*/
void
progwdc_help(void)
{
    printf("\n");
    printf("usage: progwdc TypeSlot [TypeSlot] [TypeSlot]\n");
    printf("       Type=a,d,e,m,n,p,w.\n");
    printf("       a = Aim, d = Daughtercard, e = ETM, m = motherboard\n");
    printf("       n = network module, p = PVDM, w = WIC\n");
    printf("       Slot=0...max\n");
    printf("       Exception: for motherboard, you only need Type\n");
    printf("\n");
    printf("Examples:\n");
    printf("diagmon 7 > progwdc m        -----> to program"
           " and verify motherboard WDC\n");
    printf("diagmon 8 > progwdc p0       -----> to program"
           " and verify PVDM0 WDC on motherboard\n");
    printf("diagmon 9 > progwdc w1       -----> to program"
           " and verify WIC0 WDC on motherboard\n");
    printf("diagmon 10 > progwdc n1 w0   -----> to program"
           " and verify WIC0 on NM slot 1\n");
    printf("So far only motherboard WDC is implemented.\n");
}


/*------------------------------------------------------------------------
 * progwdc
 *
 * DESCRIPTION: This function allows user to type in a CLI command at the
 *              diagmon> prompt to program and verify the WDC (Watchtower
 *              Device Certificate)
 *
 * PARAMETERS: argc = number of parameter
 *             argv - array to hold the command
 *
 * RETURNS:
 *     PASSED - if successful
 *     FAILED - if unsuccessful
 *--------------------------------------------------------------------------*/
int
progwdc (int argc,char *argv[])
{
   return act2_install_wdc(0);
}

/*------------------------------------------------------------------------
 * print_cookie
 *
 * DESCRIPTION:  print cookie info to be captured by MFG script. this is
 *               used for WDC.
 *             
 *
 * PARAMETERS: argc = number of parameter
 *             argv - array to hold the command
 *
 * RETURNS:
 *     PASSED - if successful
 *     FAILED - if unsuccessful
 *--------------------------------------------------------------------------*/
int
print_cookie (int argc,char *argv[])
{
    int i;
    register char *cptr, c;
    char pa_bay[2];
    /* Check if the user type in correct number of parameters */
    if ((argc > 4) || (argc < 2)){
        printf("Too few or too many parameters. Must be from 1 to 3 "
               "parameters\n");
        progwdc_help();
        return (FAILED);
    }
    for (i = 1; i<argc; i++) {
        cptr = argv[i];
        c = *cptr;
        c = toupper(c);
        switch (c) {
        case 'M':
            /* For motherboard, don't need to type slot number */
            if (strlen(argv[i]) != 1) {
                printf("For motherboard, the commnad is: progwdc m\n");
                progwdc_help();
                return (FAILED);
            }
            break;
        case 'N':
            break;
        case 'W':
        case 'A':
        case 'E':
        case 'P':
        case 'D':  /* Daughter card on NM */
            printf("This configuration is not supported yet\n");
            return (FAILED);
        case '?': /* For help */
            progwdc_help();
            return (FAILED);
        default:
            printf("Invalid input, %s, parameter %d.\n", argv[i], i);
            progwdc_help();
            return (FAILED);
        }
        /* Check if every parameter has a right size */
        if ((strlen(argv[i]) != 2)) {
            if ((c != 'M') && (c != '?')) {
                printf("Parameter sizes must be 2, you type in %s\n",
                       argv[i]);
                progwdc_help();
                return (FAILED);
            }
        }
 
    }
    /* Call different functions for different numbers of parameters */
    switch (argc) {
    case 2:
        strcpy(pa_bay, argv[1]);
        if(progwdc_pabay(pa_bay)) {
            return (FAILED);
        }
        break;
    case 3:
    case 4:
        printf("This configuration is not supported yet\n");
        break;
    default:
        break;
    }
    return PASSED;
}
/*------------------------------------------------------------------------
 * rmadelete
 *
 * DESCRIPTION: This function allows user to type in a CLI command to
 *              reset the licence in the NVRAM area. The function will
 *              send the request to the server. When it get approval,
 *              it'll go and reset the licence by writing 0xFF into
 *              the licence area.
 *
 *
 * PARAMETERS: None
 *
 * RETURNS:
 *     PASSED - if successful
 *     FAILED - if unsuccessful
 *--------------------------------------------------------------------------*/
int
rmadelete()
{
    char rma_buf[ DEL_APPROVAL_SIZE* 3];
    uchar del_approval[DEL_APPROVAL_SIZE];
    char rma_buf1[AUTO_TEST_MAX_SIZE *3], rma_buf2[AUTO_TEST_MAX_SIZE *3];
    char pcb_serial_num[PCB_SERIAL_NUM_SIZE];
    char chassis_serial_num[PCB_SERIAL_NUM_SIZE];
    char product_id[PRODUCT_ID_SIZE];
    char version_id[VERSION_ID_SIZE];
    uchar del_request[DEL_REQUEST_SIZE];
    uchar cookie[COOKIE_SIZE_512];
    ushort controller_id = 0;
    boolean pcb_flag = FALSE;
    int i, size, len, len_mod, del_request_sz, del_approval_sz;
    sc_context con, *con_p;
    con_p = &con;
    dev_if_info_t dev_if;
    testname ("RMA Deletion");

    con_p->dev_if_p = &dev_if;
    con_p->dev_if_p->cookie_size = COOKIE_SIZE_512;

    for (i = 0; i < PCB_SERIAL_NUM_SIZE; i++) {
        pcb_serial_num[i] = '\0';
    }
    for (i = 0; i < PCB_SERIAL_NUM_SIZE; i++) {
	chassis_serial_num[i] = '\0';
    }
    for (i = 0; i < PRODUCT_ID_SIZE; i++) {
        product_id[i] = '\0';
    }
    for (i = 0; i < VERSION_ID_SIZE; i++) {
        version_id[i] = '\0';
    }
    if (plat_init_smart_eeprom_context(con_p, MOTHER_BOARD,
                                           0, cookie) == FAILED) {
        return (FAILED);
    }
 
    if (check_quack_version(con_p)) {
        return (FAILED);
    }
    if (smart_cookie_read(con_p)) {
        return (FAILED);
    }

    controller_id = get_ctrl_id_from_cookie_contents(con_p->cookie_contents);
	
    for (i = 0; i < FRU_PLAT_MAX_IDS; i++) {
	if (fru_platform_info_tbl[i].id == controller_id) {
	    pcb_flag = TRUE;
	    break;
	}
    }
    if (print_cookie_udi(con_p, pcb_serial_num, chassis_serial_num, product_id,
			 version_id, pcb_flag)) {
	printf("\nFailed to print UDI\n");
	return (FAILED);
    }

    printf("\nDeletion request : ");
    for (i = 0; i < DEL_REQUEST_SIZE; i++) {
        del_request[i] = rand();
        printf("%02x ", del_request[i]);
    }
    printf("\n");
    size = DEL_APPROVAL_SIZE;
    if (size > AUTO_TEST_MAX_SIZE) {
        printf("\nEnter %d bytes: ", AUTO_TEST_MAX_SIZE);
        get_line(rma_buf1, sizeof(rma_buf1));
        for (i = 0; i < AUTO_TEST_MAX_SIZE; i++) {
            rma_buf[i] = rma_buf1[i];
        }
        printf("\nEnter %d bytes: ", size - AUTO_TEST_MAX_SIZE);
        get_line(rma_buf2, sizeof(rma_buf2));
        for (i = AUTO_TEST_MAX_SIZE; i < size; i++) {
            rma_buf[i] = rma_buf2[i - AUTO_TEST_MAX_SIZE];
        }
    } else {
        printf("\nEnter %d bytes: ", size);
        get_line(rma_buf, sizeof(rma_buf));
    }
    len = strlen(rma_buf);
    /* Make len to be a multiple of 3 */
    len_mod = len%3;
    if (len_mod != 0) {
        len += (3 - len_mod);
    }
    if (len != (size * 3)) {
        cterr ('f', 0, "The Deletion Approval has %d bytes, must be %d bytes",
               len/3, size);
        return (FAILED);
    }
    for (i = 0; i < len; i++) {
        rma_buf[i] = atoh(rma_buf[i]);
    }
    for (i = 0; i < size; i++) {
        del_approval[i] = ((rma_buf[3*i] << 4) & 0xf0) |
            ((rma_buf[3*i + 1]) & 0x0f);
    }
    del_request_sz = sizeof(del_request);
    del_approval_sz = sizeof(del_approval);

    if (pcb_flag == TRUE) {
	if (verify_rma_deletion(pcb_serial_num, product_id, del_request,
				del_request_sz, del_approval, del_approval_sz)) {
	return (FAILED);
	}
    } else {
	if (verify_rma_deletion(chassis_serial_num, product_id, del_request,
				del_request_sz, del_approval, del_approval_sz)) {
	return (FAILED);
	}
    }
#if 0
    if (nvflash_rma_delete()) {
        cterr('f',0, "Failed rma delete");
        return (FAILED);
    }
#endif
printf("nvflash rma delete currently not suported: FIX ME::\n\n");

    return (PASSED);
}
/*---------------------------------------------------------------------------
 * testwdc_bay
 *
 * DESCRIPTION:
 *     This function reads and tests the WDC from the smart cookie EEPROM
 *
 * PARAMETERS:
 *      pa_bay = the array holding the type and slot number
 *
 * RETURNS:
 *     PASSED - if successful
 *     FAILED - if unsuccessful
 *--------------------------------------------------------------------------*/
static int
testwdc_pabay(char *pa_bay)
{

    int i, slot_start, size, temp, pa_num = 0, index = 0;
    int status = 0;
    uchar cre_read[CRE_MAX_SIZE];
    uchar cookie[COOKIE_SIZE_512];
    char pcb_serial_num[PCB_SERIAL_NUM_SIZE];
    char chassis_serial_num[PCB_SERIAL_NUM_SIZE];
    char product_id[PRODUCT_ID_SIZE];
    char version_id[VERSION_ID_SIZE];
    uchar cmd[3], mask[5];
    uchar addr_mb, addr_lb;
    ushort base_addr, index_addr, controller_id = 0;
    sc_context con, *con_p;
    boolean pcb_flag = FALSE;
    con_p = &con;
    dev_if_info_t dev_if;

    /* clean up mask before using */
    memset(mask, 0, sizeof(mask));
    
    con_p->dev_if_p = &dev_if;
    con_p->dev_if_p->cookie_size = COOKIE_SIZE_512;
    testname ("Test WDC");
    /* In case the verification fails, mask out the WDC */
    mask[0] = SC_CREDENTIAL_ADDR_MB; /* offset MSB */
    mask[1] = SC_CREDENTIAL_ADDR_LB; /* offset LSB */
    mask[2] = 2;
    mask[3] = 0xFF;
    mask[4] = 0xFF;
    pa_bay[0] = toupper(pa_bay[0]);
    if (isdigit(pa_bay[1])) {
        pa_num = atoh(pa_bay[1]);
    } else {
        if ((pa_bay[0] != 'M') && (pa_bay[0] != '?')) {
            printf("Slot number %c is not a digit\n", pa_bay[1]);
            progauth_help();
            return FAILED;
        }
    }
    for (i = 0; i < PCB_SERIAL_NUM_SIZE; i++) {
        pcb_serial_num[i] = '\0';
    }
    for (i = 0; i < PCB_SERIAL_NUM_SIZE; i++) {
	chassis_serial_num[i] = '\0';
    }
    for (i = 0; i < PRODUCT_ID_SIZE; i++) {
        product_id[i] = '\0';
    }
    for (i = 0; i < VERSION_ID_SIZE; i++) {
        version_id[i] = '\0';
    }
    switch(pa_bay[0]) {
    case 'M':
        if (plat_init_smart_eeprom_context(con_p, MOTHER_BOARD,
                                           0, cookie) == FAILED) {
            return (FAILED);
        }
        break;
    case 'N':    /* For Bryce 2nd Quack */
        slot_start = slot_start_with();
        if (slot_start == -1) {
            cterr('f', 0, "Network Module is not supported on this platform");
            return FAILED;
        }
        if ((pa_num < slot_start) || (pa_num > get_max_sm_slots())) {
            cterr('f', 0, "Invalid NM slot number = %d, the valid range"
                  " is from %d to %d", pa_num, slot_start, get_max_sm_slots());
            return FAILED;
        }
        if (plat_init_smart_eeprom_context(con_p, NETWK_MODULE,
                                           pa_num, cookie) == FAILED) {
            return FAILED;
        }

        break;
    default:
        printf("\nThis configuration does not support WDC yet\n");
        break;
    }
    if (check_quack_version(con_p)) {
        return (FAILED);
    }
    if (smart_cookie_read(con_p)) {
        return (FAILED);
    }
    
    if (pa_bay[0] == 'M') {
	controller_id = get_ctrl_id_from_cookie_contents(con_p->cookie_contents);
	
	for (i = 0; i < FRU_PLAT_MAX_IDS; i++) {
	    if (fru_platform_info_tbl[i].id == controller_id) {
		pcb_flag = TRUE;
		break;
	    }
	}
    }

    if (print_cookie_udi(con_p, pcb_serial_num, chassis_serial_num, product_id,
			 version_id, pcb_flag)) {
	printf("\nFailed to print UDI\n");
	return (FAILED);
    }

    size = WDC_SIZE; /* Hardcoded size for reading */
    index = 0;
    /* Write credential */
    addr_mb = SC_CREDENTIAL_ADDR_MB; /* offset MSB */
    addr_lb = SC_CREDENTIAL_ADDR_LB; /* offset LSB */
    base_addr = (addr_mb << 8) | addr_lb;
    index_addr = base_addr;
    temp = size + 2; /* 2 bytes for the size of WDC */
    while (temp > 0) {
        cmd[0] = addr_mb;
        cmd[1] = addr_lb;
        if (temp >  READ_WRITE_MAX_SIZE) {
            temp = temp - READ_WRITE_MAX_SIZE;
            cmd[2] = READ_WRITE_MAX_SIZE;
        } else {
            cmd[2] = temp;
            temp = 0;
        }
        index_addr = index_addr + cmd[2];
        if (send_command_to_smart_cookie(con_p, SMART_EEPROM_READ, cmd,
                                         sizeof(cmd))) {
            cterr('f',0,"Read credential failed type = %d,"
                  "slot = %d %s", con_p->type, con_p->slot, sc_err_msg);
            return FAILED;
        }
        for(i = 0; i < cmd[2]; i++) {
            cre_read[i+index] = ((sm_message_t *)sc_response_msg)->data[i];
        }
        index = index + cmd[2];
        addr_mb = (uchar)(index_addr >> 8);
        addr_lb = (uchar)(index_addr & 0xff);
    }
    printf("\n");

    if (pcb_flag == TRUE) {
	status = verify_credential(pcb_serial_num, product_id, cre_read, size);
    } else {
	status = verify_credential(chassis_serial_num, product_id, cre_read, size);
    }

    if (status) {
        if (send_command_to_smart_cookie(con_p, SMART_EEPROM_WRITE, mask,
                                         5)) {
            cterr('f', 0, "Failed to write the mask length %s", sc_err_msg);
            return (FAILED);
        }
        cterr('f', 0, "Failed to verify the WDC, error = %#.x", status);
        return (FAILED);
    }
    return (PASSED);

}
/*------------------------------------------------------------------------
 * testwdc_help
 *
 * DESCRIPTION: This function print out the help for the testwdc command
 *
 * PARAMETERS: none
 *
 * RETURNS: None
 *--------------------------------------------------------------------------*/
void
testwdc_help(void)
{
    printf("\n");
    printf("usage: testwdc TypeSlot [TypeSlot] [TypeSlot]\n");
    printf("       Type=a,d,e,m,n,p,w.\n");
    printf("       a = Aim, d = Daughtercard, e = ETM, m = motherboard\n");
    printf("       n = network module, p = PVDM, w = WIC\n");
    printf("       Slot=0...max\n");
    printf("       Exception: for motherboard, you only need Type\n");
    printf("\n");
    printf("Examples:\n");
    printf("diagmon 7 > testwdc m        -----> to verify motherboard WDC\n");
    printf("diagmon 8 > testwdc p0       -----> to verify PVDM0 WDC"
           " on motherboard\n");
    printf("diagmon 9 > testwdc w1       -----> to verify WIC0 WDC"
           " on motherboard\n");
    printf("diagmon 10 > testwdc n1 w0   -----> to verify WIC0"
           " on NM slot 1\n");
    printf("So far only motherboard WDC is implemented.\n");
}
/*------------------------------------------------------------------------
 * testwdc
 *
 * DESCRIPTION: This function allows user to type in a CLI command at the
 *              diagmon> prompt to verify the WDC (Watchtower
 *              Device Certificate)
 *
 * PARAMETERS: argc = number of parameter
 *             argv - array to hold the command
 *
 * RETURNS:
 *     PASSED - if successful
 *     FAILED - if unsuccessful
 *--------------------------------------------------------------------------*/
int
testwdc(int argc,char *argv[])
{
    int i;
    register char *cptr, c;
    char pa_bay[2];
    /* Check if the user type in correct number of parameters */
    if ((argc > 4) || (argc < 2)){
        printf("Too few or too many parameters. Must be from 1 to 3 "
               "parameters\n");
        testwdc_help();
        return (FAILED);
    }
    for (i = 1; i<argc; i++) {
        cptr = argv[i];
        c = *cptr;
        c = toupper(c);
        switch (c) {
        case 'M':
            /* For motherboard, don't need to type slot number */
            if (strlen(argv[i]) != 1) {
                printf("For motherboard, the commnad is: testwdc m\n");
                testwdc_help();
                return (FAILED);
            }
            break;
        case 'N':
            break;
        case 'W':
        case 'A':
        case 'E':
        case 'P':
        case 'D':  /* Daughter card on NM */
            printf("This configuration is not supported yet\n");
            return (FAILED);
        case '?': /* For help */
            testwdc_help();
            return (FAILED);
        default:
            printf("Invalid input, %s, parameter %d.\n", argv[i], i);
            testwdc_help();
            return (FAILED);
        }
        /* Check if every parameter has a right size */
        if ((strlen(argv[i]) != 2)) {
            if ((c != 'M') && (c != '?')) {
                printf("Parameter sizes must be 2, you type in %s\n", argv[i]);
                testwdc_help();
                return (FAILED);
            }
        }
 
    }
    /* Call different functions for different numbers of parameters */
    switch (argc) {
    case 2:
        strcpy(pa_bay, argv[1]);
        if(testwdc_pabay(pa_bay)) {
            return (FAILED);
        }
        break;
    case 3:
    case 4:
        printf("This configuration is not supported yet\n");
        break;
    default:
        break;
    }
    return PASSED;
}
/*------------------------------------------------------------------------------
 *
 * Function: generic_nm_quack_test().
 *
 * This function implements a generic quack test for NM. Its function is similar
 * to case M of smartchip() above. However, it removes the interactive aspects
 * of smartchip() and allows the flexibility of initializing context for
 * additional devices on NM test bus such as a 2nd QUACK or daughter card QUACK.
 *
 * Input:  slot           - slot number returned by get_real_slot.
 *         submenu_flag   - TRUE/FALSE for enabling menu interface.
 *         con_init       - optional input for additional context init.
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */
int
generic_nm_quack_test (int slot, int submenu_flag,
                      void (*con_init) (sc_context * con))
{
#ifdef LINUX_APP
    printf("Fix me inside generic_nm_quack_test()\n");
    exit(0);
#else /* Diagmon */
    uchar cookie[COOKIE_SIZE_512];
    sc_context *con, cont;
    con = &cont;
    con_gl = con;
    /* no checking on input since, slot is a programmer input */
    if (plat_init_smart_eeprom_context(con, NETWK_MODULE,
                                       slot, cookie) == FAILED) {
        return (FAILED);
    }
    if (con_init) {
        con_init(con);
    }
    if (!pas_pa_present(con->slot)) {
        printf("\nNo Port Module in slot %d.\n", con->slot);
        return (FAILED);
    }

    if (submenu_flag == FALSE) {
        if (smart_cookie_program_dig_sign_mainmenu(con)) {
            return (FAILED);
        }
        if (smart_cookie_authenticate(con)) {
            return (FAILED);
        }
    } else {
        show_smart_cookie_submenu();
    }
    return (PASSED);
#endif /* LINUX_APP */
}

#ifdef SUDI
/*---------------------------------------------------------------------------
 * write_smart_cookie
 *
 * DESCRIPTION:
 *  This function writes data into the smart chip
 *
 * PARAMETERS:
 *     con_p - sc_context pointer
 *     data_ptr - data to be written
 *     base_addr - base address in the smart chip to write data to
 *     data_size - the size of data
 *
 * RETURNS:
 *     PASSED/FAILED
 *--------------------------------------------------------------------------*/
static int
write_smart_cookie(sc_context *con_p, uchar *data_ptr,
           ushort base_addr, ushort data_size)
{
    uchar data[READ_WRITE_MAX_SIZE + 3];
    uint i;
    ushort index_addr, temp, index = 0;

    index_addr = base_addr;
    temp = data_size;

    while (temp > 0) {
    data[0] = (uchar)(index_addr >> 8);
    data[1] = (uchar)(index_addr & 0xff);
        if (temp > READ_WRITE_MAX_SIZE) {
            temp = temp - READ_WRITE_MAX_SIZE;
            data[2] = READ_WRITE_MAX_SIZE;
        } else {
            data[2] = temp;
            temp = 0;
        }
        for (i = 0; i < data[2]; i++) {
            data[3 + i] = data_ptr[i + index];
        }
        index = index + data[2];
        index_addr = index_addr + data[2];

        if (send_command_to_smart_cookie(con_p, SMART_EEPROM_WRITE, data,
                                         data[2] + 3)) {
            cterr('f', 0, "write smart cookie failed");
            return (FAILED);
        }
    }

    return (PASSED);
}

/*---------------------------------------------------------------------------
 * read_smart_cookie
 *
 * DESCRIPTION:
 *  This function reads data from the smart chip
 *
 * PARAMETERS:
 *     con_p - sc_context pointer
 *     data_ptr - data pointer to save the read data
 *     base_addr - base address in the smart chip to read data from
 *     data_size - the size of data
 *
 * RETURNS:
 *     PASSED/FAILED
 *--------------------------------------------------------------------------*/
static int
read_smart_cookie(sc_context *con_p, uchar *data_ptr,
           ushort base_addr, ushort data_size)
{
    uchar cmd[3];
    uint i;
    ushort index_addr, temp, index = 0;

    index_addr = base_addr;
    temp = data_size;

    while (temp > 0) {
    cmd[0] = (uchar)(index_addr >> 8);
    cmd[1] = (uchar)(index_addr & 0xff);
        if (temp > READ_WRITE_MAX_SIZE) {
            temp = temp - READ_WRITE_MAX_SIZE;
            cmd[2] = READ_WRITE_MAX_SIZE;
        } else {
            cmd[2] = temp;
            temp = 0;
        }
        if (send_command_to_smart_cookie(con_p, SMART_EEPROM_READ, cmd,
                                         sizeof(cmd))) {
            printf("read smart cookie failed %s\n", sc_err_msg);
            cterr('f', 0, "read smart cookie failed");
            return (FAILED);
    }

        for(i = 0; i < cmd[2]; i++) {
        data_ptr[i + index] = ((sm_message_t *)sc_response_msg)->data[i];
    }

    index = index + cmd[2];
    index_addr = index_addr + cmd[2];
    }
    return (PASSED);
}


/*---------------------------------------------------------------------------
 * receive_sudi_data
 *
 * DESCRIPTION:
 *  This function receives SUDI data from autotest server.
 *
 * PARAMETERS:
 *     sudi_data_ptr - SUDI data pointer
 *     len - the length of the data to be received
 *
 * RETURNS:
 *     None
 *--------------------------------------------------------------------------*/
static void
receive_sudi_data(uchar *sudi_data_ptr, int len)
{
    uchar data_line[MAX_DATA_PER_LINE];
    int size, line_size;

    printf("\nEnter %d bytes: ", len);
    line_size = get_line(data_line, MAX_DATA_PER_LINE);
    data_line[line_size++] = '\n';
    data_line[line_size] = '\0';
    strcpy(sudi_data_ptr, data_line);
    size = len - line_size;

    while (size > 0) {
    line_size = get_line(data_line, MAX_DATA_PER_LINE);
    data_line[line_size++] = '\n';
    data_line[line_size] = '\0';
    strcat(sudi_data_ptr, data_line);
    size -= line_size;
    }
#ifdef SUDI_DEBUG
    printf("\nThe real data size is: %d\n", strlen(sudi_data_ptr));
    printf("%s", sudi_data_ptr);
#endif
}

/*---------------------------------------------------------------------------
 * write_read_sudi
 *
 * DESCRIPTION:
 *  This function writes SUDI data to the smart chip, then read back to verify.
 *
 * PARAMETERS:
 *     con_p - sc_context pointer
 *     sudi_data_ptr - SUDI data pointer
 *     sudi_data_ptr1 - SUDI data pointer1 for TYPE_CERT
 *     sudi_data_ptr2 - SUDI data pointer2 for TYPE_CERT
 *     sudi_hd_ptr - pointer to SUDI header
 *     type - the type of SUDI data, TYPE_KEY/TYPE_CERT
 *
 * RETURNS:
 *     PASSED/FAILED
 *--------------------------------------------------------------------------*/
static int
write_read_sudi(sc_context *con_p, uchar *sudi_data_ptr, uchar *sudi_data_ptr1,
        uchar *sudi_data_ptr2, sudi_header_t *sudi_hd_ptr, int type)
{
    uchar *sudi_read_ptr, *sudi_data_ptr0;
    sudi_header_t read_header;
    uint max_sudi_size, sudi_hd_size, sudi_data_size;
    uint sudi_data_size0, sudi_data_size1, sudi_data_size2;
    uint i, len;
    ushort hd_base_addr, data_base_addr;
    uchar sudi_name[20], sudi_string[50];
    uint sudi_data2_size;

    switch (type) {
    case TYPE_KEY:
    max_sudi_size = KEY_CONTENT_SIZE;
    sudi_hd_size = KEY_HEADER_SIZE;
    hd_base_addr = KEY_HEADER_ADDR;
    data_base_addr = KEY_CONTENT_ADDR;
    sprintf((char *)sudi_name, "private key");
    break;
    case TYPE_CERT:
    max_sudi_size = CERT_CONTENT_SIZE;
    sudi_hd_size = CERT_HEADER_SIZE;
    hd_base_addr = CERT_HEADER_ADDR;
    data_base_addr = CERT_CONTENT_ADDR1;
    sprintf((char *)sudi_name, "certificate");
    break;
    default:
    cterr('f',0,"Unsupported SUDI type: %d", type);
    return (FAILED);
    }

    sudi_data_ptr0 = (uchar *)malloc_nm(CERT_CONTENT_SIZE);
    if (sudi_data_ptr0 == NULL) {
    cterr('f',0,"Failed to malloc memory for sudi_data_ptr0");
    return (FAILED);
    }

    /* receive sudi data from autotest server */
    if (type == TYPE_KEY) {
    sprintf((char *)sudi_string, "\nEnter %s size:", sudi_name);
    sudi_data_size = getdec_answer(sudi_string, 0, 0, max_sudi_size);
    receive_sudi_data(sudi_data_ptr, sudi_data_size);
    } else {
    sprintf((char *)sudi_string, "\nEnter root cert size:");
    sudi_data_size0 = getdec_answer(sudi_string, 0, 0, max_sudi_size);
    receive_sudi_data(sudi_data_ptr0, sudi_data_size0);

    sprintf((char *)sudi_string, "\nEnter sub CA cert size:");
    sudi_data_size1 = getdec_answer(sudi_string, 0, 0, max_sudi_size);
    receive_sudi_data(sudi_data_ptr1, sudi_data_size1);

    sprintf((char *)sudi_string, "\nEnter dev cert size:");
    sudi_data_size2 = getdec_answer(sudi_string, 0, 0, max_sudi_size);
    receive_sudi_data(sudi_data_ptr2, sudi_data_size2);

    strcpy(sudi_data_ptr, sudi_data_ptr0);
    strcat(sudi_data_ptr, sudi_data_ptr1);
    strcat(sudi_data_ptr, sudi_data_ptr2);

    sudi_data_size = sudi_data_size0 + sudi_data_size1 + sudi_data_size2;
    }

    len = strlen(sudi_data_ptr);

    if (sudi_data_size > max_sudi_size) {
    free(sudi_data_ptr0);
    cterr('f', 0, "Received data size is bigger than the max data size."
          "Received data size = %d, Max data size = %d",
          sudi_data_size, max_sudi_size);
    return (FAILED);
    }

    if (len != sudi_data_size) {
    free(sudi_data_ptr0);
        cterr ('f', 0, "The %s data has %d bytes, must be %d bytes",
           sudi_name, len, sudi_data_size);
        return (FAILED);
    }

    /* setup sudi header */
    sudi_hd_ptr->type = type;
    sudi_hd_ptr->size = sudi_data_size;
    sudi_hd_ptr->magic = MAGIC_NUM;

    /* write sudi header to smart cookie */
    if (write_smart_cookie(con_p, (uchar *)sudi_hd_ptr, hd_base_addr,
               sudi_hd_size) == FAILED) {
    free(sudi_data_ptr0);
    cterr('f', 0, "Failed to write %s header to smart cookie: %s",
          sudi_name, sc_err_msg);
    return (FAILED);
    }

    /* write sudi data to smart cookie */
    if (type == TYPE_KEY) {
    if (write_smart_cookie(con_p, sudi_data_ptr, data_base_addr,
                   sudi_data_size) == FAILED) {
        free(sudi_data_ptr0);
        cterr('f', 0, "Failed to write %s data to smart cookie: %s",
          sudi_name, sc_err_msg);
        return (FAILED);
    }
    } else {
    /*
     * Because of the bug in Quack 2(CSCte72759), 0x2000 - 0x21FF is
     * the reserved area which can not be used to store user data.
     * Also 0x1800 - 0x1BBF is used to store WDC data.
     * So certificate data needs to be fragmented to three parts:
     * 0x1110 - 0x17FF, 0x1C00 - 0x1FFF and 0x2200 - 0x330F.
     */
    if (write_smart_cookie(con_p, sudi_data_ptr, data_base_addr,
                   CERT_DATA1_SIZE) == FAILED) {
        free(sudi_data_ptr0);
        cterr('f', 0, "Failed to write %s data to smart cookie: %s",
          sudi_name, sc_err_msg);
        return (FAILED);
    }

    sudi_data2_size = sudi_data_size - CERT_DATA1_SIZE;

    if (sudi_data2_size > CERT_DATA2_SIZE) {
        if (write_smart_cookie(con_p, &sudi_data_ptr[CERT_DATA1_SIZE],
             CERT_CONTENT_ADDR2, CERT_DATA2_SIZE) == FAILED) {
        free(sudi_data_ptr0);
        cterr('f', 0, "Failed to write %s data to smart cookie: %s",
              sudi_name, sc_err_msg);
        return (FAILED);
        }

        if (write_smart_cookie(con_p,
              &sudi_data_ptr[CERT_DATA1_SIZE + CERT_DATA2_SIZE],
              CERT_CONTENT_ADDR3, sudi_data2_size - CERT_DATA2_SIZE)
        == FAILED) {
        free(sudi_data_ptr0);
        cterr('f', 0, "Failed to write %s data to smart cookie: %s",
              sudi_name, sc_err_msg);
        return (FAILED);
        }
    } else {
        if (write_smart_cookie(con_p, &sudi_data_ptr[CERT_DATA1_SIZE],
             CERT_CONTENT_ADDR2, sudi_data2_size) == FAILED) {
        free(sudi_data_ptr0);
        cterr('f', 0, "Failed to write %s data to smart cookie: %s",
              sudi_name, sc_err_msg);
        return (FAILED);
        }
    }
    }

    sudi_read_ptr = (uchar *)malloc_nm(max_sudi_size);
    if (sudi_read_ptr == NULL) {
    free(sudi_data_ptr0);
    cterr('f',0,"Failed to malloc memory for sudi_read_ptr");
    return (FAILED);
    }

    /* read sudi header from smart cookie */
    if (read_smart_cookie(con_p, (uchar *)&read_header, hd_base_addr,
              sudi_hd_size) == FAILED) {
    free(sudi_data_ptr0);
    free(sudi_read_ptr);
    cterr('f', 0, "Failed to read %s header from smart cookie: %s",
          sudi_name, sc_err_msg);
    return (FAILED);
    }

    /* compare the sudi header */
    if (read_header.type != sudi_hd_ptr->type) {
    free(sudi_data_ptr0);
    free(sudi_read_ptr);
    cterr('f', 0, "Wrong type in %s header. Expect = %d, Read = %d",
          sudi_name, sudi_hd_ptr->type, read_header.type);
    return (FAILED);
    }

    if (read_header.size != sudi_hd_ptr->size) {
    free(sudi_data_ptr0);
    free(sudi_read_ptr);
    cterr('f', 0, "Wrong size in %s header. Expect = %d, Read = %d",
          sudi_name, sudi_hd_ptr->size, read_header.size);
    return (FAILED);
    }

    if (read_header.magic != sudi_hd_ptr->magic) {
    free(sudi_data_ptr0);
    free(sudi_read_ptr);
    cterr('f', 0, "Wrong magic data in %s header. Expect = %d, Read = %d",
          sudi_name, sudi_hd_ptr->magic, read_header.magic);
    return (FAILED);
    }

    /* read sudi data from smart cookie */
    if (type == TYPE_KEY) {
    if (read_smart_cookie(con_p, sudi_read_ptr, data_base_addr,
                  sudi_data_size) == FAILED) {
        free(sudi_data_ptr0);
        free(sudi_read_ptr);
        cterr('f', 0, "Failed to read %s data from smart cookie: %s",
          sudi_name, sc_err_msg);
        return (FAILED);
    }
    } else {
    if (read_smart_cookie(con_p, sudi_read_ptr, data_base_addr,
                  CERT_DATA1_SIZE) == FAILED) {
        free(sudi_data_ptr0);
        free(sudi_read_ptr);
        cterr('f', 0, "Failed to read %s data from smart cookie: %s",
          sudi_name, sc_err_msg);
        return (FAILED);
    }

    sudi_data2_size = sudi_data_size - CERT_DATA1_SIZE;
    if (sudi_data2_size > CERT_DATA2_SIZE) {
        if (read_smart_cookie(con_p, &sudi_read_ptr[CERT_DATA1_SIZE],
               CERT_CONTENT_ADDR2, CERT_DATA2_SIZE) == FAILED) {
        free(sudi_data_ptr0);
        free(sudi_read_ptr);
        cterr('f', 0, "Failed to read %s data from smart cookie: %s",
              sudi_name, sc_err_msg);
        return (FAILED);
        }

        if (read_smart_cookie(con_p,
                      &sudi_read_ptr[CERT_DATA1_SIZE + CERT_DATA2_SIZE],
              CERT_CONTENT_ADDR3, sudi_data2_size - CERT_DATA2_SIZE)
        == FAILED) {
        free(sudi_data_ptr0);
        free(sudi_read_ptr);
        cterr('f', 0, "Failed to read %s data from smart cookie: %s",
              sudi_name, sc_err_msg);
        return (FAILED);
        }
    } else {
        if (read_smart_cookie(con_p, &sudi_read_ptr[CERT_DATA1_SIZE],
               CERT_CONTENT_ADDR2, sudi_data2_size) == FAILED) {
        free(sudi_data_ptr0);
        free(sudi_read_ptr);
        cterr('f', 0, "Failed to read %s data from smart cookie: %s",
              sudi_name, sc_err_msg);
        return (FAILED);
        }
    }
    }

#ifdef SUDI_DEBUG
    printf("%s", sudi_read_ptr);
#endif

    /* compare the sudi data */
    for(i = 0; i < sudi_data_size; i++) {
    if (sudi_data_ptr[i] != sudi_read_ptr[i]) {
        free(sudi_data_ptr0);
        free(sudi_read_ptr);
            cterr('f', 0, "\n Wrong SUDI data, at offset: %d, "
          "Expect = %c, Read = %c", i,
                  sudi_data_ptr[i], sudi_read_ptr[i]);
            return (FAILED);
        }
    }

    if (type == TYPE_CERT) {
    strcpy(sudi_data_ptr, sudi_data_ptr0);
    }

    free(sudi_data_ptr0);
    free(sudi_read_ptr);

    return (PASSED);
}


/*---------------------------------------------------------------------------
 * read_sudi_data
 *
 * DESCRIPTION:
 *  This function reads SUDI data from Quack.
 *
 * PARAMETERS:
 *     con_p - sc_context pointer
 *     key_data_ptr - pointer to SUDI key data
 *     root_cert_ptr - pointer to SUDI root cert data
 *     subca_cert_ptr - pointer to SUDI subca cert data
 *     dev_cert_ptr - pointer to SUDI dev cert data
 *
 * RETURNS:
 *     PASSED/FAILED
 *--------------------------------------------------------------------------*/
static int
read_sudi_data(sc_context *con_p, uchar *key_data_ptr, uchar *root_cert_ptr,
           uchar *subca_cert_ptr, uchar *dev_cert_ptr)
{
    sudi_header_t key_header, cert_header;
    uchar *sudi_read_ptr, *data1_ptr, *data2_ptr;
    uchar cert_end_delimiter[] = "-----END CERTIFICATE-----";
    int delimiter_len, total_data_len, len, data1_len, data2_len, i;
    int sudi_data_size;

    sudi_read_ptr = (uchar *)malloc_nm(CERT_CONTENT_SIZE);
    if (sudi_read_ptr == NULL) {
    cterr('f',0,"Failed to malloc memory for sudi_read_ptr");
    return (FAILED);
    }

    /* read sudi key header from smart cookie */
    if (read_smart_cookie(con_p, (uchar *)&key_header, KEY_HEADER_ADDR,
              KEY_HEADER_SIZE) == FAILED) {
    free(sudi_read_ptr);
    cterr('f', 0, "Failed to read key header from smart cookie: %s",
          sc_err_msg);
    return (FAILED);
    }

    /* compare the sudi key header */
    if (key_header.type != TYPE_KEY) {
    free(sudi_read_ptr);
    cterr('f', 0, "Wrong type in key header. Expect = %d, Read = %d",
          TYPE_KEY, key_header.type);
    return (FAILED);
    }

    if (key_header.magic != MAGIC_NUM) {
    free(sudi_read_ptr);
    cterr('f', 0, "Wrong magic data in key header. Expect = %d, Read = %d",
          MAGIC_NUM, key_header.magic);
    return (FAILED);
    }

    /* read sudi key data from smart cookie */
    if (read_smart_cookie(con_p, key_data_ptr, KEY_CONTENT_ADDR,
              key_header.size) == FAILED) {
    free(sudi_read_ptr);
    cterr('f', 0, "Failed to read key data from smart cookie: %s", sc_err_msg);
    return (FAILED);
    }

    /* read sudi cert header from smart cookie */
    if (read_smart_cookie(con_p, (uchar *)&cert_header, CERT_HEADER_ADDR,
              CERT_HEADER_SIZE) == FAILED) {
    free(sudi_read_ptr);
    cterr('f', 0, "Failed to read cert header from smart cookie: %s",
          sc_err_msg);
    return (FAILED);
    }

    /* compare the sudi cert header */
    if (cert_header.type != TYPE_CERT) {
    free(sudi_read_ptr);
    cterr('f', 0, "Wrong type in cert header. Expect = %d, Read = %d",
          TYPE_KEY, cert_header.type);
    return (FAILED);
    }

    if (cert_header.magic != MAGIC_NUM) {
    free(sudi_read_ptr);
    cterr('f', 0, "Wrong magic data in cert header. Expect = %d, Read = %d",
          MAGIC_NUM, cert_header.magic);
    return (FAILED);
    }

    /* read sudi cert data from smart cookie */
    if (read_smart_cookie(con_p, sudi_read_ptr, CERT_CONTENT_ADDR1,
              CERT_DATA1_SIZE) == FAILED) {
    free(sudi_read_ptr);
    cterr('f', 0, "Failed to read cert data from smart cookie: %s", sc_err_msg);
    return (FAILED);
    }

    sudi_data_size = cert_header.size - CERT_DATA1_SIZE;

    if (sudi_data_size > CERT_DATA2_SIZE) {
    if (read_smart_cookie(con_p, &sudi_read_ptr[CERT_DATA1_SIZE],
                  CERT_CONTENT_ADDR2, CERT_DATA2_SIZE) == FAILED) {
        free(sudi_read_ptr);
        cterr('f', 0, "Failed to read cert data from smart cookie: %s",
          sc_err_msg);
        return (FAILED);
    }

    if (read_smart_cookie(con_p,
             &sudi_read_ptr[CERT_DATA1_SIZE + CERT_DATA2_SIZE],
                 CERT_CONTENT_ADDR3, sudi_data_size - CERT_DATA2_SIZE)
                 == FAILED) {
        free(sudi_read_ptr);
        cterr('f', 0, "Failed to read cert data from smart cookie: %s",
          sc_err_msg);
        return (FAILED);
    }
    } else {
    if (read_smart_cookie(con_p, &sudi_read_ptr[CERT_DATA1_SIZE],
                  CERT_CONTENT_ADDR2, sudi_data_size) == FAILED) {
        free(sudi_read_ptr);
        cterr('f', 0, "Failed to read cert data from smart cookie: %s",
          sc_err_msg);
        return (FAILED);
    }
    }

    total_data_len = strlen(sudi_read_ptr);
    delimiter_len = strlen(cert_end_delimiter);
    data1_ptr = strstr(sudi_read_ptr, cert_end_delimiter);
    if (data1_ptr != NULL) {
    len = strlen(data1_ptr);
    data1_len = total_data_len - len + delimiter_len + 1;
    for (i = 0; i < data1_len; i++) {
        root_cert_ptr[i] = sudi_read_ptr[i];
    }
    }
#ifdef SUDI_DEBUG
    printf("root_cert_len = %d\n root_cert_data = %s\n", data1_len, root_cert_pt
r);
#endif
    data1_ptr += delimiter_len;
    data2_ptr = strstr(data1_ptr, cert_end_delimiter);
    if (data2_ptr != NULL) {
    len = strlen(data2_ptr);
    data2_len = total_data_len - len + delimiter_len + 1 - data1_len;
    for (i = 0; i < data2_len; i++) {
        subca_cert_ptr[i] = sudi_read_ptr[i + data1_len];
    }
    }
#ifdef SUDI_DEBUG
    printf("subca_cert_len = %d\n subca_cert_data = %s\n", data2_len,
       subca_cert_ptr);
#endif
    for (i = 0; i < total_data_len - data1_len - data2_len; i++) {
    dev_cert_ptr[i] = sudi_read_ptr[i + data1_len + data2_len];
    }
#ifdef SUDI_DEBUG
    printf("dev_cert_len = %d\n dev_cert_data = %s\n",
       total_data_len - data1_len - data2_len, dev_cert_ptr);
#endif
    free(sudi_read_ptr);
    return (PASSED);
}


/*---------------------------------------------------------------------------
 * progsudi
 *
 * DESCRIPTION:
 *  This function is the main entry to program SUDI to the smart chip and
 *  validate the SUDI data.
 *
 * PARAMETERS: None
 *
 * RETURNS: PASSED/FAILED
 *--------------------------------------------------------------------------*/
int
progsudi()
{
    sudi_header_t key_header, cert_header;
    uchar *key_data_ptr, *root_cert_ptr, *subca_cert_ptr, *dev_cert_ptr;
    uchar cookie[COOKIE_SIZE_512];
    uchar chassis_serial_num[PCB_SERIAL_NUM_SIZE];
    uchar pcb_serial_num[PCB_SERIAL_NUM_SIZE];
    uchar product_id[PRODUCT_ID_SIZE];
    uchar version_id[VERSION_ID_SIZE];
    sc_context con, *con_p;
    dev_if_info_t dev_if;
    uint i;

    con_p = &con;
    testname ("Program SUDI");

    con_p->dev_if_p = &dev_if;
    con_p->dev_if_p->cookie_size = COOKIE_SIZE_512;

    for (i = 0; i < PCB_SERIAL_NUM_SIZE; i++) {
        pcb_serial_num[i] = '\0';
    }
    for (i = 0; i < PCB_SERIAL_NUM_SIZE; i++) {
    chassis_serial_num[i] = '\0';
    }
    for (i = 0; i < PRODUCT_ID_SIZE; i++) {
        product_id[i] = '\0';
    }
    for (i = 0; i < VERSION_ID_SIZE; i++) {
        version_id[i] = '\0';
    }

    if (plat_init_smart_eeprom_context(con_p, MOTHER_BOARD,
                       0, cookie) == FAILED) {
    return (FAILED);
    }

    if (check_quack_version(con_p)) {
        return (FAILED);
    }

    if (smart_cookie_read(con_p)) {
        return (FAILED);
    }

    key_data_ptr = (uchar *)malloc_nm(KEY_CONTENT_SIZE);
    if (key_data_ptr == NULL) {
        cterr('f',0,"Failed to malloc memory for key_data_ptr");
        return (FAILED);
    }

    root_cert_ptr = (uchar *)malloc_nm(CERT_CONTENT_SIZE);
    if (root_cert_ptr == NULL) {
        
        free(key_data_ptr);
        cterr('f',0,"Failed to malloc memory for root_cert_ptr");
        return (FAILED);
    }


    subca_cert_ptr = (uchar *)malloc_nm(CERT_CONTENT_SIZE);
    if (subca_cert_ptr == NULL) {

        free(key_data_ptr);
        free(root_cert_ptr);
        cterr('f',0,"Failed to malloc memory for subca_cert_ptr");
        return (FAILED);
    }

    dev_cert_ptr = (uchar *)malloc_nm(CERT_CONTENT_SIZE);
    if (dev_cert_ptr == NULL) {

        free(key_data_ptr);
        free(root_cert_ptr);
        free(subca_cert_ptr);
        cterr('f',0,"Failed to malloc memory for dev_cert_ptr");
        return (FAILED);
    }

    /* use chassis_serial_number here instead of pcb_serial_number */
    if (print_cookie_udi(con_p, pcb_serial_num, chassis_serial_num,
             product_id, version_id, FALSE)) {
        printf("\nFailed to print UDI\n");

        free(key_data_ptr);
        free(root_cert_ptr);
        free(subca_cert_ptr);
        free(dev_cert_ptr);
        return (FAILED);
    }

    if (write_read_sudi(con_p, key_data_ptr, NULL, NULL, &key_header,
            TYPE_KEY) == FAILED) {
        free(key_data_ptr);
        free(root_cert_ptr);
        free(subca_cert_ptr);
        free(dev_cert_ptr);
        return (FAILED);
    }

    if (write_read_sudi(con_p, root_cert_ptr, subca_cert_ptr, dev_cert_ptr,
            &cert_header, TYPE_CERT) == FAILED) {

        free(key_data_ptr);
        free(root_cert_ptr);
        free(subca_cert_ptr);
        free(dev_cert_ptr);
        return (FAILED);
    }

#ifdef SUDI
    /* verify private key and certificates */
    if (verify_sudi(key_data_ptr, root_cert_ptr, subca_cert_ptr, dev_cert_ptr,
            chassis_serial_num, product_id) == FAILED) {
        free(key_data_ptr);
        free(root_cert_ptr);
        free(subca_cert_ptr);
        free(dev_cert_ptr);
        cterr('f', 0, "Failed to verify SUDI data");
        return (FAILED);
    }
#endif

    free(key_data_ptr);
    free(root_cert_ptr);
    free(subca_cert_ptr);
    free(dev_cert_ptr);

    return (PASSED);
}


/*---------------------------------------------------------------------------
 * testsudi
 *
 * DESCRIPTION:
 *  This function read the SUDI data from Quack and validate the SUDI data.
 *
 * PARAMETERS: None
 *
 * RETURNS: PASSED/FAILED
 *--------------------------------------------------------------------------*/
int
testsudi()
{
    uchar *key_data_ptr, *root_cert_ptr, *subca_cert_ptr, *dev_cert_ptr;
    uchar cookie[COOKIE_SIZE_512];
    uchar chassis_serial_num[PCB_SERIAL_NUM_SIZE];
    uchar pcb_serial_num[PCB_SERIAL_NUM_SIZE];
    uchar product_id[PRODUCT_ID_SIZE];
    uchar version_id[VERSION_ID_SIZE];
    sc_context con, *con_p;
    dev_if_info_t dev_if;
    uint i;

    con_p = &con;
    testname ("Test SUDI");

    con_p->dev_if_p = &dev_if;
    con_p->dev_if_p->cookie_size = COOKIE_SIZE_512;

    for (i = 0; i < PCB_SERIAL_NUM_SIZE; i++) {
        pcb_serial_num[i] = '\0';
    }
    for (i = 0; i < PCB_SERIAL_NUM_SIZE; i++) {
    chassis_serial_num[i] = '\0';
    }
    for (i = 0; i < PRODUCT_ID_SIZE; i++) {
        product_id[i] = '\0';
    }
    for (i = 0; i < VERSION_ID_SIZE; i++) {
        version_id[i] = '\0';
    }

    if (plat_init_smart_eeprom_context(con_p, MOTHER_BOARD,
                       0, cookie) == FAILED) {
    return (FAILED);
    }


    if (check_quack_version(con_p)) {
        return (FAILED);
    }

    if (smart_cookie_read(con_p)) {
        return (FAILED);
    }

    key_data_ptr = (uchar *)malloc_nm(KEY_CONTENT_SIZE);
    if (key_data_ptr == NULL) {
        cterr('f',0,"Failed to malloc memory for key_data_ptr");
        return (FAILED);
    }

    root_cert_ptr = (uchar *)malloc_nm(CERT_CONTENT_SIZE);
    if (root_cert_ptr == NULL) {

        free(key_data_ptr);
        cterr('f',0,"Failed to malloc memory for root_cert_ptr");
        return (FAILED);
    }

    subca_cert_ptr = (uchar *)malloc_nm(CERT_CONTENT_SIZE);
    if (subca_cert_ptr == NULL) {

        free(key_data_ptr);
        free(root_cert_ptr);
        cterr('f',0,"Failed to malloc memory for subca_cert_ptr");
        return (FAILED);
    }

    dev_cert_ptr = (uchar *)malloc_nm(CERT_CONTENT_SIZE);
    if (dev_cert_ptr == NULL) {

        free(key_data_ptr);
        free(root_cert_ptr);
        free(subca_cert_ptr);
        cterr('f',0,"Failed to malloc memory for dev_cert_ptr");
        return (FAILED);
    }

    /* use chassis_serial_number here instead of pcb_serial_number */
    if (print_cookie_udi(con_p, pcb_serial_num, chassis_serial_num,
             product_id, version_id, FALSE)) {
    printf("\nFailed to print UDI\n");

    free(key_data_ptr);
    free(root_cert_ptr);
    free(subca_cert_ptr);
    free(dev_cert_ptr);
    return (FAILED);
    }

    if (read_sudi_data(con_p, key_data_ptr, root_cert_ptr, subca_cert_ptr,
               dev_cert_ptr) == FAILED) {
        free(key_data_ptr);
        free(root_cert_ptr);
        free(subca_cert_ptr);
        free(dev_cert_ptr);
        return (FAILED);
    }

#ifdef SUDI
    /* verify private key and certificates */
    if (verify_sudi(key_data_ptr, root_cert_ptr, subca_cert_ptr, dev_cert_ptr,
            chassis_serial_num, product_id) == FAILED) {
        
        free(key_data_ptr);
        free(root_cert_ptr);
        free(subca_cert_ptr);
        free(dev_cert_ptr);
        cterr('f', 0, "Failed to verify SUDI data");
        return (FAILED);
    }
#endif

    free(key_data_ptr);
    free(root_cert_ptr);
    free(subca_cert_ptr);
    free(dev_cert_ptr);

    return (PASSED);
}

#endif
/* end of file */
/******** History ******** 
$Log: smart_cookie.c,v $
Revision 1.23  2017/07/14 02:51:38  alpeng
fixed compiler warning, due to cross-compiler version was updated.

Revision 1.22  2016/06/06 18:30:39  huanngo
Add code to support programming SUDI/WDC with Chassis S/N

Revision 1.21  2014/05/02 20:36:14  mcharon
support CLIIP/SUDI optoin for module installed on SM

Revision 1.20  2014/02/13 19:03:11  mcharon
support act2 authentication on sword

Revision 1.19  2013/11/26 08:40:34  hroni
fix compiler warning

Revision 1.18  2013/08/27 21:39:59  mcharon
user lower case to execute act2 menu. remove is_act2

Revision 1.17  2013/07/18 22:33:16  mcharon
remove debug printf

Revision 1.16  2013/07/18 20:12:11  mcharon
support MB POE quack

Revision 1.15  2013/05/31 12:51:11  danchung
Add checking board type for Juno.

Revision 1.14  2013/03/11 03:33:16  alpeng
supporting CLI for NGIO-DC

Revision 1.13  2013/03/08 19:05:08  mcharon
add function to read pid

Revision 1.12  2013/02/28 00:38:44  srane
Add smart cookie tests for NGWIC Daughter card.

Revision 1.11  2013/01/11 19:41:38  mcharon
use PCB serial num instead of Chasis ser no for WDC

Revision 1.10  2012/11/06 20:39:49  mcharon
add headers/cleanup/remove unneeded functions/files

Revision 1.9  2012/10/25 18:56:18  mcharon
improve error reporting

Revision 1.8  2012/09/18 19:19:55  mcharon
support poll slot-fix io intr test; support fpga upgrade; support serial irq intr tst

Revision 1.7  2012/08/11 00:14:47  mcharon
flag as error if we try to access Act1 space while still in simple mode

Revision 1.6  2012/06/20 20:29:58  mcharon
support wdc

Revision 1.5  2012/06/06 09:48:03  aarwang
- Clean up compiler warnings.

Revision 1.4  2012/05/08 06:12:57  alpeng
change the CLI cmd from PVDM to VM

Revision 1.3  2012/05/04 20:01:46  mcharon
use void* instead of int as argument to to func ptrs in ngio_intf

Revision 1.2  2012/03/28 00:38:15  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:05  ptong
Initial archive of ng_diag module


$Endlog$
*/
