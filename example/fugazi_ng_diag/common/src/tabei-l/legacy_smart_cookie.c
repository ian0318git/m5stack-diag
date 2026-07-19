/* $Id: legacy_smart_cookie.c,v 1.3 2019/12/30 06:03:45 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/legacy_smart_cookie.c,v $
 *------------------------------------------------------------------
 *
 * legacy_smart_cookie.c - Legacy Smart Cookie Read Function 
 *
 * Copyright (c) 2015-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include "nvmonvars.h"
#include "common.h"
#include "types.h"
#include "tam_act2_api_drv_support.h"
#include "legacy_smart_cookie.h"
#include "cross_platform.h"

/*******************************************************************************
 *                              Global Variables                               *
 *******************************************************************************
 */
static uchar command_msg[MAX_MESSAGE_SIZE];
static uchar sc_response_msg[MAX_MESSAGE_SIZE];
static int imc_polling_timeout = IMC_POLLING_TIMEOUT;
static char sc_err_msg[MAX_MESSAGE_SIZE];
static ushort flow_control_window_size = DF_FLOW_CONTROL_WINDOW_SIZE;
static uchar req_cookie_data[] = {0x00, 0x00, 0x80};

/*******************************************************************************
 *                             Function Prototypes                             *
 *******************************************************************************
 */
int cookie_is_act2(sc_context *);
int send_command_to_smart_cookie(sc_context *, char, uchar *, uint);
int quack_version (sc_context *con);
static uchar calculate_cksum(uchar *, int);
static scc_return_status_t i2c_scc_process_cmd(sc_context *, void *,
                                               ushort, uchar *, uint);
static void display_scc_return_status(scc_return_status_t, sc_context *);
static scc_return_status_t i2c_scc_process_cmd_simple(sc_context *, void *, ushort,
                                                      uchar *, uint);
static scc_return_status_t send_i2c_cmd_simple(sc_context *, void *, uint,
                                               void *, uint);
static void scc_delay_for_cmd_processing(uchar);
static int i2c_scc_read_bytes(sc_context *, uchar *);
static void update_nak_ack_resp_msg_len(uchar, ushort *);
static int i2c_scc_write_bytes(sc_context *, unsigned char *, int);
int smart_cookie_read(sc_context *);
static void copy_cookie_from_rsp_msg(uchar *, uchar *, int);

extern int act2_is_simple_mode(void *);

/**************************************************************************
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
 *************************************************************************/
static void copy_cookie_from_rsp_msg (uchar *cookie, uchar *res_msg,
                                      int length)
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

/**************************************************************************
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
 *************************************************************************/
int smart_cookie_read (sc_context *con)
{
    if (act2_is_simple_mode(con)) {
        cterr('f', 0, "device is in simple mode");
        return FAILED;
    }
    
    if (send_command_to_smart_cookie(con, REQUEST_COOKIE_DATA,
                                     req_cookie_data, sizeof(req_cookie_data))) {
        return (FAILED);
    }

    copy_cookie_from_rsp_msg(con->cookie_contents, &sc_response_msg[3],
                             con->dev_if_p->cookie_size);
    
    return (PASSED);
}

/**************************************************************************
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
 *************************************************************************/
static int i2c_scc_write_bytes (sc_context *con_p, unsigned char *i2c_cmd,
                                int msg_size)
{
    if (con_p->quack_write_2bytes(con_p, i2c_cmd, msg_size)) {
        return (FAILED);
    }
    return (PASSED);
}

/**************************************************************************
 * send_i2c_cmd_simple
 *
 * Description:
 *   Send command throught I2C
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
 *************************************************************************/
static scc_return_status_t send_i2c_cmd_simple (sc_context *con, void *cmd_buffer,
                                                uint cmd_length, void *resp_buffer,
                                                uint resp_length)
{
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf(" simple mode:  bytes-left: %d", cmd_length);
    }

    if (!act2_drv_write (con, cmd_buffer , cmd_length)) {
        cterr('f', 0, "Failed to write into IMC2");
        return (SCC_TIMEOUT);
    }

    return (SCC_OK);
}

/**************************************************************************
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
 *************************************************************************/
static void update_nak_ack_resp_msg_len (uchar rx_char, ushort *resp_length)
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

/**************************************************************************
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
 *************************************************************************/
static int i2c_scc_read_bytes (sc_context *con, uchar *read_buffer)
{
    if (con->quack_read_2bytes(con, read_buffer)) {
        return (FAILED);
    }
    return (PASSED);
}

/**************************************************************************
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
 *************************************************************************/
static void scc_delay_for_cmd_processing (uchar cmd_type)
{
    switch (cmd_type) {
    case REQUEST_SIGN_MESSAGE:
    case REQUEST_SIGN_MESSAGE_32B:
    case REQUEST_SIGN_MSG_DIGEST:
         usleep(WAIT_FOR_RANDOM_NUMBER_SIGNING * 2);
         break;
    case EEPROM_PAGE_LOCK_DOWN:
         usleep(WAIT_FOR_EEPROM_LOCKING);
         break;
    default:
         usleep(WAIT_DEFAULT_TIME);
         break;
    }
    return;
}

/**************************************************************************
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
 *************************************************************************/
static scc_return_status_t send_i2c_cmd (sc_context *con, void *cmd_buffer,
                                         uint cmd_length, void *resp_buffer,
                                         uint resp_length)
{
    ushort send_bytes = cmd_length;
    unsigned char   i2c_cmd[MAX_N2G_QCK_MSG_SIZE];
    uchar imc_msg_id = IMC_SPI_CMD_START;
    uchar *tmp_cmd_buffer = (uchar *)cmd_buffer;
    int i, tx_size;

    if (act2_is_simple_mode(con)) {
        return (send_i2c_cmd_simple(con, cmd_buffer, cmd_length,
                                    resp_buffer, resp_length));
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
        usleep(WAIT_FOR_N2G_SCC_PROCESS_CMD);
    }
     /*
     * Notify to the N2G-SCC that this is the end of the SCC command sequence
     */
    i2c_cmd[0] = IMC_SPI_CMD_END;
    if (i2c_scc_write_bytes(con, i2c_cmd, 1)) {
        return (SCC_TIMEOUT);
    }
    usleep(WAIT_FOR_N2G_SCC_PROCESS_CMD);
    return (SCC_OK);
}

/**************************************************************************
 * i2c_scc_process_cmd_simple
 *
 * Description:
 *   This function process the command
 *
 *
 * Parameters:
 *   con         - pointer to sc_context 
 *   cmd_buffer  - command buffer
 *   cmd_length  - command length
 *   resp_buffer - response buffer
 *   resp_length - response length 
 *
 * Returns:
 *   None
 *
 *************************************************************************/
static scc_return_status_t i2c_scc_process_cmd_simple (sc_context *con, void *cmd_buffer,
                                                       ushort cmd_length,
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
        return (status);
    }
    /*
     * Based on command type, we wait for various length of time
     */

    if (!act2_drv_read (con, (char *)resp_buffer,resp_length)) {
        return (SCC_TIMEOUT);
    }

    return (SCC_OK);
}

/**************************************************************************
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
 *
 *************************************************************************/
static void display_scc_return_status (scc_return_status_t status,
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

/**************************************************************************
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
 *
 *************************************************************************/
static scc_return_status_t i2c_scc_process_cmd (sc_context *con, void *cmd_buffer,
                                                ushort cmd_length, uchar *resp_buffer,
                                                uint resp_length)
{
    scc_return_status_t status;
    int ix;
    uchar tmp_buffer[MAX_N2G_QCK_MSG_SIZE];
    uchar *rx_buff_ptr = resp_buffer;
    ushort rx_length;
    uint timeout_ctr, rx_ctr;

    if (act2_is_simple_mode(con)) {
        return (i2c_scc_process_cmd_simple(con, cmd_buffer, cmd_length,
                                           resp_buffer, resp_length));
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
        usleep(WAIT_FOR_N2G_SCC_PROCESS_CMD);
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
            for (ix = 1; ix <= tmp_buffer[0]; ix++) {
                *rx_buff_ptr = tmp_buffer[ix];
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
                 usleep(WAIT_FOR_GET_NEXT_CMD);
            }
        } else if (tmp_buffer[0] == 0) {
            usleep(WAITING_FOR_REPLY_MSG);
        } else {
            printf("Invalid Message Size: %0x\n", tmp_buffer[0]);
            usleep(WAITING_FOR_REPLY_MSG);
        }
    }
    if (timeout_ctr > 0) {
        return (SCC_OK);
    } else {
        return (SCC_TIMEOUT);
    }
}


/**************************************************************************
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
 *
 *************************************************************************/
static scc_return_status_t scc_process_cmd (sc_context *con, void *cmd,
                                            ushort cmd_length, uchar *resp_buffer,
                                            uint resp_length)
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
    {
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
    usleep(WAITING_FOR_REPLY_MSG);
    /*
     * Retrieve the Response Messsage from SCC
     */
    switch (con->dev_if_p->interface) {
        case SCC_I2C_IF:
            return status;
            break;
        default:
            printf("Invalid SCC interface. You forgot to initialize SCC IF\n");
            break;
    }
    return (SCC_TIMEOUT);
}

/**************************************************************************
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
 *************************************************************************/
static uchar calculate_cksum (uchar *p, int count)
{
    uchar cksum, *data;
    ushort ix;
    data = p;
    cksum = 0;

    for (ix = 0; ix < count; ix++) {
        cksum += *data++;
    }
    return (~cksum);
}

/**************************************************************************
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
 *
 *************************************************************************/
int send_command_to_smart_cookie (sc_context *con, char type, uchar *data,
                                  uint data_length)
{
    uchar *ptr = command_msg;
    uint resp_len;
    uchar resp_type;
    boolean single_byte_cmd = FALSE;
    unsigned int ix;
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
    for (ix = 0; ix < MAX_MESSAGE_SIZE; ix++) {
        sc_response_msg[ix] = 0x00;
    }

    if (single_byte_cmd) {
        command_msg[0] = type;
    } else {
        *ptr++ = type;
        *ptr++ = 0;
        *ptr++ = data_length;

        for (ix = 0; ix < data_length; ix++) {
            *ptr++ = *data++;
        }
        command_msg[data_length+3] = 0;
        *ptr = calculate_cksum(command_msg, data_length + 3);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\nCommand\n");
        for (ix = 0; ix < (uint)(((sm_message_t *)command_msg)->length+HEADER_SIZE);ix++) {
            if ((ix % 16) == 0) {
                printf("\n");
            }
            printf("%02X ", command_msg[ix]);
        }
    }
    send_status = scc_process_cmd(con, command_msg,
                   single_byte_cmd ? 1 : data_length + HEADER_SIZE,
                   sc_response_msg, resp_len);

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\nResponse\n");
        for (ix = 0; ix < (uint)(((sm_message_t *)sc_response_msg)->length+HEADER_SIZE); ix++) {
            if ((ix % 16) == 0) {
                printf("\n");
            }
            printf("%02X ", sc_response_msg[ix]);
        }
        printf("\n");
    }
    if (send_status != SCC_OK) {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            display_scc_return_status(send_status, con);
        }
        sprintf(sc_err_msg, "SCC_OK failed");
        return (FAILED);
    }
    /* check the checksum of the response */

    if (calculate_cksum(sc_response_msg,
            ((sm_message_t *)sc_response_msg)->length +
            HEADER_SIZE)) {
        printf("smart_cookie.c calculate check sum failed @%d\n", __LINE__);
        sprintf(sc_err_msg, "Calculate checksum failed");
        return (FAILED);
    }
    if (((sm_message_t *)sc_response_msg)->type != resp_type) {
        printf("smart_cookie @%d\n", __LINE__);
        sprintf(sc_err_msg, "Error in response Expected = %02x, Received = "
        "%02x\n", resp_type, ((sm_message_t *)sc_response_msg)->type);
        return (FAILED);
    }

    return (PASSED);
}


/**************************************************************************
 *
 * is_act2
 *
 * DESCRIPTION:
 *     Return TRUE if it is ACT2
 *
 * PARAMETERS:
 *     con  - context pointer
 *
 * RETURNS:
 *     TRUE - ACT2
 *     FALSE - Quack
 *
 *************************************************************************/
int cookie_is_act2 (sc_context *con)
{
    int version;

    if (is_tam_aikido_mbox_on() == TRUE) {
        return (TRUE);
    } /* if mbox is on, it must be aikido, which is act2 */

    if (send_command_to_smart_cookie(con, REQUEST_SCC_ID,
                                     NULL, 0)){
        printf("%s: Sending command to smart cookie failed\n", __func__);
        return (FALSE);
    }

    version = sc_response_msg[5];

    printf("Chip version is %02x\n", version);

    if (version < ACT2_VERSION_NUMBER_START) {
        return (FALSE);
    }

    return (TRUE);
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
int smart_cookie_read_x(sc_context *con_p, ushort size)
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

        if (send_command_to_smart_cookie(con_p, SMART_EEPROM_READ, cmd,
                                         sizeof(cmd))) {
            cterr('f',0,"Read cookie  failed type = %d,"
                  "slot = %d %s", con_p->type, con_p->slot, sc_err_msg);
            return (FAILED);
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
    return (PASSED);
}

/*------------------------------------------------------------------------
 * smart_cookie_write_x
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

int smart_cookie_write_x(sc_context *con_p, uchar *cookie_buf, ushort size)
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
            return (FAILED);
        }
        addr_mb = (uchar)(index_addr >> 8);
        addr_lb = (uchar)(index_addr & 0xff);
    }

    return (PASSED);
}

/*---------------------------------------------------------------------------
 * smart_cookie_write
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
int smart_cookie_write(sc_context *con, uchar *cookie_buf)
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
        return (FAILED);
    }

    return (PASSED);
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
int quack_version (sc_context *con)
{
    int version;

    /* this function is used for quack,
     * if platfrom is used for ACT2 or aikido */
    if (is_tam_aikido_on()) {
        printf("%s: is used for quack, skipping for AIKIDO \n", __FUNCTION__);
        return 0;
    }

    /*
     * before calling quack_version(), codes should call
     * is_smart_eeprom() first to make sure the module has quack.
     */
    if (send_command_to_smart_cookie(con, REQUEST_SCC_ID,
                     NULL, 0)) {
        return (SCC_ILL_VERSION);
    }
    version = sc_response_msg[5];

    return (version);
}


/*---------------------------------------------------------------
$Log: legacy_smart_cookie.c,v $
Revision 1.3  2019/12/30 06:03:45  kehuang2
CSCvs55860: Support Alter Quack cookie

Revision 1.2  2019/10/17 02:16:24  kehuang2
Collapse Tabei-L into main trunk

Revision 1.1.2.2  2018/12/05 06:50:35  olin2
initial commit for Aikido

Revision 1.1.2.1  2018/11/02 02:39:03  kodko
Support cookie read for NIM and PIM modules.

$Endlog$
*/
