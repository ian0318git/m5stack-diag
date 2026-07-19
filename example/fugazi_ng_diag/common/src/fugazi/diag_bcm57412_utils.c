/* $Id: diag_bcm57412_utils.c,v 1.2 2021/06/02 08:22:34 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/diag_bcm57412_utils.c,v $
 * diag_bcm57412_utils.c - Interfacing functions to BCM57412 via BNXT driver
 *
 * Sep 2019,  Ian Chang
 *
 * Copyright (c) 2019-2021 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <features.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/ethtool.h>
#include <linux/sockios.h>
#include <assert.h>
#include <pthread.h>
#include <sys/time.h>
#include <poll.h>
#include "diag_bcm57412_utils.h"
#include "types.h"

#ifndef __bitwise
#define __bitwise
#endif

#include <stdint.h>
#include "common.h"
#include "common_utils.h"
#include "nvmonvars.h"
#include "defs.h"
#include "proto.h"
#include "menu.h"
#include "error.h"
#include "diag_bcm57412_test.h"
#include "diag_bcm_lib.h"
#include "diag_bcm54194_api.h"


typedef struct bnxt_ipc_mng_ {
    int      socket_fd;
    int      cpr_kernel_session;
    boolean  is_init_was_done;
} bnxt_ipc_mng_t;

static bnxt_ipc_mng_t g_mng;

static boolean validate_next_buf(int remaining, int next_len)
{
    return (remaining >= (NLA_HDRLEN + next_len));
}

/******************************************************************************
 *
 * Function: nla_next
 *
 * Description: next netlink attribute in attribute stream
 *
 * Inputs      : nla       - netlink attribute
 *             : remaining - number of bytes remaining in attribute stream
 * Outputs     : Returns the next netlink attribute in the attribute stream and
 *               decrements remaining by the size of the current attribute.
 *****************************************************************************/
static inline struct nlattr *nla_next (struct nlattr *nla, int *remaining)
{
	unsigned int totlen = NLA_ALIGN(nla->nla_len);

	*remaining -= totlen;
	return (struct nlattr *) ((char *) nla + totlen);
}

/******************************************************************************
 *
 * Function: get_genlmsg_data
 *
 * Description: get message from netlink attribute stream
 *
 * Inputs      : msg: message attribute stream
 *            
 * Outputs     : Data address offset.
 *               
 *****************************************************************************/
static inline uint8_t *get_genlmsg_data(bcm_nl_request_msg_t *msg) {
    uint8_t *t =(uint8_t *)NLMSG_DATA(msg);
    return (t + GENL_HDRLEN);
}

/******************************************************************************
 *
 * Function: get_gelmnsg_nla_data
 *
 * Description: get message from nal data
 *
 * Inputs      : na: netlink attribute
 *            
 * Outputs     : Data address offset.
 *               
 *****************************************************************************/
static inline void *get_gelmnsg_nla_data(struct nlattr *na)
{
    return ((void *)((char*)(na) + NLA_HDRLEN));
}

/******************************************************************************
 *
 * Function: send_gnl_msg
 *
 * Description: This function checks sfp+ cookie byte 0 and byte 1 to make  
 *              sure the i2c bus between BCM57412 and SFP module is good.
 *
 * Inputs      : sd   - Socket ID
 *               *msg - command message
 *               *ans - response message
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int send_gnl_msg(int sd,  bcm_nl_request_msg_t *msg,
                        bcm_nl_request_msg_t *ans)
{
    struct sockaddr_nl nladdr;
    memset(&nladdr, 0, sizeof(nladdr));
    nladdr.nl_family = AF_NETLINK;

    char *buf = (char *)msg;
    int left_to_send = msg->n.nlmsg_len;
    int resp_len = 0;

    while (left_to_send) {
        int sent_len = sendto(sd, buf, left_to_send, 0,
                             (struct sockaddr *) &nladdr,
                              sizeof(nladdr));
        if (sent_len > left_to_send) {
            printf("\nFUGAZI_NL: send_gnl_msg, sent_len > left_to_send");
            return (FAILED);
        }

        if (sent_len <= 0) {
            if (errno == EAGAIN) {
                continue;
            } else {
                printf("FUGAZI_NL: sendto returned error");
                return (FAILED);
            }
        }

        buf += (sent_len);
        left_to_send -= (sent_len);
    }

    resp_len = recv(sd, ans, sizeof(bcm_nl_request_msg_t), 0);
    if (resp_len < 0){
        printf("\nFUGAZI_NL: recv failed");
        return (FAILED);
    }
 
     /* Validate response message */
     if (!NLMSG_OK((&ans->n), (uint32_t)resp_len)){
        printf("\nFUGAZI_NL: invalid reply message\n");
        return (FAILED);
     }
 
     if (ans->n.nlmsg_type == NLMSG_ERROR) { /* error */
        printf("\nFUGAZI_NL: received error\n");
        return (FAILED);
     }
 
     return (PASSED);
}
/*
 * Probe the controller in genetlink to find the family id
 * for the CONTROL_EXMPL family
 */
/******************************************************************************
 *
 * Function: get_family_id
 *
 * Description: This function get the netlink family id
 *
 * Inputs      : sd - Socket ID
 * Outputs     : Family ID
 *
 *****************************************************************************/
static int get_family_id (int sd)
{
    bcm_nl_request_msg_t family_req;
    bcm_nl_request_msg_t ans;
    memset(&ans,0 ,sizeof(bcm_nl_request_msg_t));
    int fam_id = -1;

    /* Get family name */
    family_req.n.nlmsg_type  = GENL_ID_CTRL;
    family_req.n.nlmsg_flags = NLM_F_REQUEST;
    family_req.n.nlmsg_seq   = 0;
    family_req.n.nlmsg_pid   = getpid();
    family_req.n.nlmsg_len   = NLMSG_LENGTH(GENL_HDRLEN);
    family_req.g.cmd         = CTRL_CMD_GETFAMILY;
    family_req.g.version     = 0x1;

    struct nlattr *na = (struct nlattr *) get_genlmsg_data(&family_req);
    na->nla_type = CTRL_ATTR_FAMILY_NAME;

    na->nla_len = strlen(BNXT_NL_NAME) + 1 + NLA_HDRLEN;

    strcpy((char *)get_gelmnsg_nla_data(na), BNXT_NL_NAME);

    family_req.n.nlmsg_len += NLMSG_ALIGN(na->nla_len);

    if (send_gnl_msg(sd, &family_req, &ans) < 0) {
        return (FAILED);
    }

    na = (struct nlattr *) get_genlmsg_data(&ans);
    na = (struct nlattr *) ((char *) na + NLA_ALIGN(na->nla_len));
    if (na->nla_type == CTRL_ATTR_FAMILY_ID) {
        fam_id = *(__u16 *) get_gelmnsg_nla_data(na);
    }
    return fam_id;
}
/******************************************************************************
 *
 * Function: construct_hdrs
 *
 * Description: This function construct the netlink.
 *
 * Inputs      : req       - Send command structure
 *               *remaining - remaining parameter
 *               *naddr     - na pointer to starting of next header
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int construct_hdrs (bcm_nl_request_msg_t* req,
                           uint32_t ifindex, int *remaining,
                           struct nlattr **naddr)
{
    uint32_t data = 0;
    int next_len;
    struct nlattr *na;

    /* Send command needed */
    req->n.nlmsg_type   = g_mng.cpr_kernel_session;
    req->n.nlmsg_flags  = NLM_F_REQUEST;
    req->n.nlmsg_seq    = 0;
    req->n.nlmsg_pid    = getpid();
    req->n.nlmsg_len    = NLMSG_LENGTH(GENL_HDRLEN);
    req->g.cmd          = BNXT_CMD_HWRM;

    /* compose message */
    /* Add PID for get to the name-space */
    na = (struct nlattr *) get_genlmsg_data(req);
    na->nla_type = BNXT_ATTR_PID;
    na->nla_len = NLA_HDRLEN + sizeof(data);
    data = getpid();
    memcpy((char *)get_gelmnsg_nla_data(na), &data, sizeof(data));
    req->n.nlmsg_len += NLMSG_ALIGN(na->nla_len);

    /* Add IF_INDEX of the interface */
    na = nla_next(na, remaining);
    next_len = sizeof(data) + NLA_HDRLEN;
    if (validate_next_buf(*remaining, next_len)) {
        na->nla_type = BNXT_ATTR_IF_INDEX;
        na->nla_len = next_len; /* Message length */
        data = ifindex;
        memcpy((char *)get_gelmnsg_nla_data(na), &data, sizeof(data));
        req->n.nlmsg_len += NLMSG_ALIGN(na->nla_len);
    } else {
        printf("\nFUGAZI_NL: construct_hdrs: Failed to insert IF_INDEX");
        return (FAILED);
    }

    /* Set the na pointer to starting of next header */
    na = nla_next(na, remaining);
    *naddr = na;
    return (PASSED);
}
/******************************************************************************
 *
 * Function: create_nl_socket
 *
 * Description: This function create a netlink socket   
 *
 * Inputs      : protocol - netlonk protocol
 * Inputs      : groups   - netlonk group
 * Outputs     : Socket id
 *
 *****************************************************************************/

static int create_nl_socket(int protocol, int groups)
{
    int fd;
    struct sockaddr_nl local;

    fd = socket(AF_NETLINK, SOCK_RAW, protocol);
    if (fd < 0){
        printf("\nFUGAZI_NL: socket create error");
        return (-1);
    }

    memset(&local, 0, sizeof(local));
    local.nl_family = AF_NETLINK;
    local.nl_groups = groups;
    if (bind(fd, (struct sockaddr *) &local, sizeof(local)) < 0) {
        close(fd);
        printf("\nFUGAZI_NL: socket bind error");
        return (-1);
    }
    return fd;
}
/******************************************************************************
 *
 * Function: bnxt_impl_deinit_netlink
 *
 * Description: This function deinit the netlink   
 *
 * Inputs      : None
 * Outputs     : None
 *
 *****************************************************************************/

void bnxt_impl_deinit_netlink (void)
{
    if (!g_mng.is_init_was_done) {
        return ;
    }
    if (g_mng.socket_fd > 0) {
        close(g_mng.socket_fd);
        g_mng.socket_fd = 0;
    }
    g_mng.is_init_was_done = FALSE;
}

/******************************************************************************
 *
 * Function: bnxt_impl_init_netlink
 *
 * Description: This function initial the netlink   
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
boolean bnxt_impl_init_netlink (void)
{
    if (g_mng.is_init_was_done) {
        return (PASSED);
    }

    g_mng.socket_fd = create_nl_socket(NETLINK_GENERIC, 0);
    if( g_mng.socket_fd  < 0){
        printf("\nFUGAZI_NL: create_nl_socket failed");
        return (FAILED);
    }

    g_mng.cpr_kernel_session = get_family_id(g_mng.socket_fd);

    if( g_mng.cpr_kernel_session < 0){
        printf("\nFUGAZI_NL: get_family_id failed");
        return(FAILED);
    }
    g_mng.is_init_was_done = TRUE;
    return (PASSED);
}
/******************************************************************************
 *
 * Function: fugazi_eth_get_ifindex
 *
 * Description: This function get the network index from /sys/class/net/%s/ifindex
 *
 * Inputs      : port - ethernet port number
 * Outputs     : ifindex
 *
 *****************************************************************************/
int fugazi_eth_get_ifindex (uint16_t port)
{
    char path[DEV_IFINDEX_PATH_SIZE];
    char dev_name[IF_NAMESIZE];
    int ifindex; 
    FILE *fp;

    sprintf(dev_name, "eth%d", port);
    snprintf(path, DEV_IFINDEX_PATH_SIZE, SYS_IFINDEX_PATH, dev_name);
    fp = fopen(path, "r");
    fscanf(fp, "%d", &ifindex);
    fclose(fp);
    return ifindex;
} 
/******************************************************************************
 *
 * Function: bnxt_netlink_i2c_read
 *
 * Description: The firmware processes the HWRM command HWRM_PORT_PHY_I2C_READ. 
 *              The firmware write the data into I2C bus.
 *
 * Inputs      : i2c_addr : 8-bit I2C slave address
 *               ifindex  : network index
 *               page_num : The page number that is being accessed over I2C
 *               port_id  : Port ID of port
 *               offset   : data offset 
 *               data_len : Length of data to read
 *               buff     : read data buffer
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int bnxt_netlink_i2c_read (uint8_t i2c_addr, uint32_t ifindex, uint16_t page_num,
                           uint16_t port_id, uint16_t offset, uint8_t data_len,
                           uint8_t *buff)
{
    bcm_nl_request_msg_t req, ans;
    hwrm_port_phy_i2c_read_input_t i2c_read_req;
    hwrm_port_phy_i2c_read_output_t *i2c_read_resp;
    struct nlattr *na = NULL;
    int next_len = 0, remaining = BNXT_BUF_MAX;
    char *byte_data = NULL;

    if (!buff || !g_mng.is_init_was_done) {
        printf("\nFUGAZI_NL %d: i2c_read: buff is NULL or init not done", 
                port_id);
        return (FAILED);
    }

    if (data_len > BNXT_MAX_PHY_I2C_RESP_SIZE) {
        /* Not seen any case for data read > 64 bytes in one call.
         * Hence ignore append logic */
        printf("\nFUGAZI_NL %d: i2c_read: Read datalen > permissible length",
                port_id);
        return (FAILED);
    }

    memset(&i2c_read_req, 0, sizeof(i2c_read_req));
    memset(&req, 0, sizeof(req));
    memset(&ans, 0, sizeof(ans));

    if (construct_hdrs(&req, ifindex, &remaining, &na)) {
        printf("\nFUGAZI_NL %d: construct_hdrs failed", port_id);
        return (FAILED);
    }

    /* Add actual request */
    next_len = sizeof(i2c_read_req) + NLA_HDRLEN;
    if (validate_next_buf(remaining, next_len)) {
        na->nla_type = BNXT_ATTR_REQUEST;
        na->nla_len = next_len; /*Message length */
        i2c_read_req.req_type = HWRM_PORT_PHY_I2C_READ;
        i2c_read_req.i2c_slave_addr = i2c_addr;
        i2c_read_req.page_number = page_num;
        i2c_read_req.port_id = port_id;
        i2c_read_req.page_offset = offset;
        i2c_read_req.data_length = data_len;
        i2c_read_req.enables = offset ?
                               PORT_PHY_I2C_READ_REQ_ENABLES_PAGE_OFFSET : 0;
        memcpy((char *)get_gelmnsg_nla_data(na),
               &i2c_read_req, sizeof(i2c_read_req));
        req.n.nlmsg_len += NLA_ALIGN(na->nla_len);
    } else {
        printf("\nFUGAZI_NL %d: i2c_read: IF_INDEX. Failed to insert REQUEST",
                port_id);
        return (FAILED);
    }

    if (send_gnl_msg(g_mng.socket_fd, &req, &ans) < 0) {
        printf("\nFUGAZI_NL: send_gnl_msg returned error\n");
        return (FAILED);
    }

    /* Parse the reply message */
    na = (struct nlattr *) get_genlmsg_data(&ans);
    i2c_read_resp = (hwrm_port_phy_i2c_read_output_t *)get_gelmnsg_nla_data(na);

    if (i2c_read_resp->error_code) {
        printf("\nFUGAZI_NL %d: i2c_read: Read error_code 0x%x",
                port_id, i2c_read_resp->error_code);
        return (FAILED);
    }

    byte_data = (char *)i2c_read_resp->data;
    memcpy(buff, byte_data, data_len);
    return (PASSED);
}
/******************************************************************************
 *
 * Function: bnxt_netlink_i2c_write
 *
 * Description: The firmware processes the HWRM command HWRM_PORT_PHY_I2C_WRITE. 
 *              The firmware write the data into I2C bus. 
 *
 * Inputs      : i2c_addr : 8-bit I2C slave address
 *               ifindex  : network index
 *               page_num : The page number that is being accessed over I2C
 *               port_id  : Port ID of port
 *               offset   : data offset 
 *               data_len : Length of data to write
 *               buff     : write data buffer
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int bnxt_netlink_i2c_write (uint8_t i2c_addr, uint32_t ifindex, uint16_t page_num,
                            uint16_t port_id, uint16_t offset, uint8_t data_len,
                            uint8_t *buff)
{
    bcm_nl_request_msg_t req, ans;
    hwrm_port_phy_i2c_write_input_t i2c_write_req;
    hwrm_port_phy_i2c_write_output_t *i2c_write_resp;
    struct nlattr *na = NULL;
    int next_len = 0, remaining = BNXT_BUF_MAX;


    if (!buff || !g_mng.is_init_was_done) {
        printf("FUGAZI_NL %d: i2c_write: buff is NULL or init not done", 
                port_id);
        return (FAILED);
    }

    if (data_len > BNXT_MAX_PHY_I2C_REQ_SIZE) {
        /* Not seen any case for data write > 64 bytes in one call.
         * Hence ignore multiple call logic */
        printf("FUGAZI_NL %d: i2c_write: Write datalen > permissible length",
                port_id);
        return (FAILED);
    }

    memset(&i2c_write_req, 0, sizeof(i2c_write_req));
    memset(&req, 0, sizeof(req));
    memset(&ans, 0, sizeof(ans));

    if (construct_hdrs(&req, ifindex, &remaining, &na)) {
        printf("\nFUGAZI_NL %d: construct_hdrs failed", port_id);
        return (FAILED);
    }

    /* Add actual request */
    next_len = sizeof(i2c_write_req) + NLA_HDRLEN;
    if (validate_next_buf(remaining, next_len)) {
        na->nla_type = BNXT_ATTR_REQUEST;
        na->nla_len = next_len; /* Message length */
        i2c_write_req.req_type = HWRM_PORT_PHY_I2C_WRITE;
        i2c_write_req.i2c_slave_addr = i2c_addr;
        i2c_write_req.page_number = page_num;
        i2c_write_req.port_id = port_id;
        i2c_write_req.page_offset = offset;
        i2c_write_req.data_length = data_len;
        i2c_write_req.enables = offset ?
                                PORT_PHY_I2C_READ_REQ_ENABLES_PAGE_OFFSET : 0;
        memcpy(i2c_write_req.data, buff, data_len);
        memcpy((char *)get_gelmnsg_nla_data(na), &i2c_write_req,
               sizeof(i2c_write_req));
        req.n.nlmsg_len += NLA_ALIGN(na->nla_len);
    } else {
        printf("\nFUGAZI_NL %d: i2c_write: IF_INDEX. Failed to insert REQUEST",
                port_id);
        return (FAILED);
    }


    if (send_gnl_msg(g_mng.socket_fd, &req, &ans) < 0) {
        printf("\nFUGAZI_NL: i2c_write: send_gnl_msg returned error\n");
        return (FAILED);
    }

    /* Parse the reply message */
    na = (struct nlattr *) get_genlmsg_data(&ans);
    i2c_write_resp = (hwrm_port_phy_i2c_write_output_t *)get_gelmnsg_nla_data(na);

    if (i2c_write_resp->error_code) {
        printf("\nFUGAZI_NL %d: i2c_write: Resp. error_code 0x%x",
                port_id, i2c_write_resp->error_code);
        return (FAILED);

    }
    return (PASSED);
}

/******************************************************************************
 *
 * Function: bnxt_sfp_detect
 *
 * Description: The firmware processes the HWRM command and payload contained 
 *              in the message. The firmware reads the GPIO pin 
 *              (SFP_ABS output from SFP to BCM57412 MAC) to populate 
 *              the value in the response.
 *
 * Inputs      : port_id - Port ID
 *             : ifindex - network index
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/

int bnxt_sfp_detect (uint16_t port_id, uint32_t ifindex)
{
    bcm_nl_request_msg_t req, ans;
    hwrm_port_phy_qcfg_input_t read_req;
    hwrm_port_phy_qcfg_output_t *read_resp;
    struct nlattr *na = NULL;
    int next_len = 0, remaining = BNXT_BUF_MAX;

    memset(&read_req, 0, sizeof(read_req));
    memset(&req, 0, sizeof(req));
    memset(&ans, 0, sizeof(ans));

    if (construct_hdrs(&req, ifindex, &remaining, &na)) {
        printf("\nFUGAZI_NL %d: construct_hdrs failed", port_id);
        return (FAILED);
    }

    /* Add actual request */
    next_len = sizeof(read_req) + NLA_HDRLEN;
    if (validate_next_buf(remaining, next_len)) {
        na->nla_type = BNXT_ATTR_REQUEST;
        na->nla_len = next_len; /* Message length */
        read_req.req_type = HWRM_PORT_PHY_QCFG;
        read_req.port_id = port_id;
        memcpy((char *)get_gelmnsg_nla_data(na), &read_req, sizeof(read_req));
        req.n.nlmsg_len += NLA_ALIGN(na->nla_len);
    } else {
        printf("\nFUGAZI_NL %d: xcvr_detect: Failed to insert REQUEST", port_id);
        return (FAILED);
    }

    if (send_gnl_msg(g_mng.socket_fd, &req, &ans) < 0) {
        printf("\nFUGAZI_NL %d: xcvr_detect: send_gnl_msg returned error", port_id);
        return (FAILED);
    }

    /* Parse the reply message */
    na = (struct nlattr *) get_genlmsg_data(&ans);
    read_resp = (hwrm_port_phy_qcfg_output_t *)get_gelmnsg_nla_data(na);

    if (read_resp->error_code) {
        printf("\nFUGAZI_NL %d: xcvr_detect: Read error_code 0x%x",
                port_id, read_resp->error_code);
        return (FAILED);
    }

    if (read_resp->module_status == PORT_PHY_QCFG_RESP_MODULE_STATUS_NONE) {
        /* SFP inserted */
        return (PASSED);
    }
    return (FAILED);
}

/******************************************************************************
 *
 * Function: fugazi_sfp_present
 *
 * Description: The function via netlink to detect the SFP present message. 
 *
 * Inputs      : port - Port number
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int fugazi_sfp_present (uint16_t port)
{
    uint32_t ifindex;
    int retval = PASSED;
    
    bnxt_impl_init_netlink();
    ifindex = fugazi_eth_get_ifindex(port);
    if (bnxt_sfp_detect(port, ifindex) == FAILED) {
        printf("\nPort %d: SFP is not present", port);
        retval = FAILED;
    }
    bnxt_impl_deinit_netlink();
    return (retval);
}
/******************************************************************************
 *
 * Function: bnxt_netlink_sideband_read
 *
 * Description: The firmware processes the HWRM command and payload contained 
 *              in the message. The firmware reads the GPIO pin 
 *              (SFP_ABS, TX_FAULT, RX_LOS output from SFP to BCM57412 MAC) 
 *              to populate the value in the response.
 *
 * Inputs      : port_id - Port ID
 *             : ifindex - network index
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int bnxt_netlink_sideband_read (uint16_t port_id, uint32_t ifindex)
{
    bcm_nl_request_msg_t req, ans;
    hwrm_port_sfp_sideband_qcfg_input_t sideband_read_req;
    hwrm_port_sfp_sideband_qcfg_output_t *sideband_read_resp;
    struct nlattr *na = NULL;
    int next_len = 0, remaining = BNXT_BUF_MAX;

    memset(&sideband_read_req, 0, sizeof(sideband_read_req));
    memset(&req, 0, sizeof(req));
    memset(&ans, 0, sizeof(ans));

    if (construct_hdrs(&req, ifindex, &remaining, &na)) {
        printf("\nFUGAZI_NL %d: construct_hdrs failed", port_id);
        return (FAILED);
    }

    /* Add actual request */
    next_len = sizeof(sideband_read_req) + NLA_HDRLEN;
    if (validate_next_buf(remaining, next_len)) {
        na->nla_type = BNXT_ATTR_REQUEST;
        na->nla_len = next_len; /* Message length */
        sideband_read_req.req_type = HWRM_PORT_SFP_SIDEBAND_QCFG;
        sideband_read_req.port_id = port_id;
        sideband_read_req.seq_id = 0;
        sideband_read_req.cmpl_ring = 0;
        sideband_read_req.target_id = 0;
        sideband_read_req.resp_addr = 0;
        memcpy((char *)get_gelmnsg_nla_data(na), &sideband_read_req, 
               sizeof(sideband_read_req));
        req.n.nlmsg_len += NLA_ALIGN(na->nla_len);
    } else {
        printf("\nFUGAZI_NL %d: xcvr_detect: Failed to insert REQUEST", 
                port_id);
        return (FAILED);
    }

    if (send_gnl_msg(g_mng.socket_fd, &req, &ans) < 0) {
        printf("\nFUGAZI_NL %d: xcvr_detect: send_gnl_msg returned error", 
                port_id);
        return (FAILED);
    }

    /* Parse the reply message */
    na = (struct nlattr *) get_genlmsg_data(&ans);
    sideband_read_resp = (hwrm_port_sfp_sideband_qcfg_output_t *)
                          get_gelmnsg_nla_data(na);

    if (sideband_read_resp->error_code) {
        printf("\nFUGAZI_NL %d: Read error_code 0x%x",
                port_id, sideband_read_resp->error_code);
        return (FAILED);
    }
    if (sideband_read_resp->valid) {
        printf("\nFUGAZI_NL %d: Read sideband_read_resp not valid %x",
                port_id, sideband_read_resp->valid);
        return (FAILED);
    }
    printf("\n FUGAZI Port %d: Sideband signals %x", port_id, 
            sideband_read_resp->sideband_signals);

    return (PASSED);
}
/******************************************************************************
 *
 * Function: bnxt_netlink_sideband_tx_dis
 *
 * Description: The firmware processes the HWRM command and payload contained 
 *              in the message. Enable / Disable tx_dis sideband.
 *
 * Inputs      : port_id - Port ID
 *             : ifindex - network index
 *             : enable - enable / disable flag
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int bnxt_netlink_sideband_tx_dis (uint16_t port_id, uint32_t ifindex, 
                                  uint16_t enable)
{
    bcm_nl_request_msg_t req, ans;
    hwrm_port_sfp_sideband_cfg_input_t sideband_write_req;
    hwrm_port_sfp_sideband_cfg_output_t *sideband_write_resp;
    struct nlattr *na = NULL;
    int next_len = 0, remaining = BNXT_BUF_MAX;

    memset(&sideband_write_req, 0, sizeof(sideband_write_req));
    memset(&req, 0, sizeof(req));
    memset(&ans, 0, sizeof(ans));

    if (construct_hdrs(&req, ifindex, &remaining, &na)) {
        printf("\nFUGAZI_NL %d: construct_hdrs failed", port_id);
        return (FAILED);
    }

    /* Add actual request */
    next_len = sizeof(sideband_write_req) + NLA_HDRLEN;
    if (validate_next_buf(remaining, next_len)) {
        na->nla_type = BNXT_ATTR_REQUEST;
        na->nla_len = next_len; /* Message length */
        sideband_write_req.req_type = HWRM_PORT_SFP_SIDEBAND_CFG;
        sideband_write_req.port_id = port_id;
        sideband_write_req.cmpl_ring = 0;
        sideband_write_req.seq_id = 0;
        sideband_write_req.target_id = 0;
        sideband_write_req.resp_addr = 0;
        sideband_write_req.enables |= PORT_SFP_SIDEBAND_CFG_REQ_ENABLES_TX_DIS;
        if (enable) {
            sideband_write_req.flags |= PORT_SFP_SIDEBAND_CFG_REQ_FLAGS_TX_DIS;
        } else {
            sideband_write_req.flags &= PORT_SFP_SIDEBAND_CFG_REQ_FLAGS_TX_DIS;
        }
        memcpy((char *)get_gelmnsg_nla_data(na), &sideband_write_req, 
               sizeof(sideband_write_req));
        req.n.nlmsg_len += NLA_ALIGN(na->nla_len);
    } else {
        printf("\nFUGAZI_NL %d: xcvr_detect: Failed to insert REQUEST", 
                port_id);
        return (FAILED);
    }

    if (send_gnl_msg(g_mng.socket_fd, &req, &ans) < 0) {
        printf("\nFUGAZI_NL %d: xcvr_detect: send_gnl_msg returned error", 
                port_id);
        return (FAILED);
    }

    /* Parse the reply message */
    na = (struct nlattr *) get_genlmsg_data(&ans);
    sideband_write_resp = (hwrm_port_sfp_sideband_cfg_output_t *)
                           get_gelmnsg_nla_data(na);

    if (sideband_write_resp->error_code) {
        printf("\nFUGAZI_NL %d: write error_code 0x%x",
                port_id, sideband_write_resp->error_code);
        return (FAILED);
    }
    if (diagflag_xram & D_DEBUG_OPTIONS) {
        printf("\n FUGAZI Port %d: tx_dis %s", port_id, 
                enable ? "enable" : "disable" );
    }
    return (PASSED);
}
/******************************************************************************
 *
 * Function: bnxt_netlink_reg_test
 *
 * Description: The firmware processes the HWRM command and payload contained 
 *              in the message. This function is called by driver to request 
 *              which self tests to be run.
 *
 * Inputs      : port_id - Port ID
 *             : ifindex - network index
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int bnxt_netlink_reg_test (uint16_t port_id, uint32_t ifindex)
{
    bcm_nl_request_msg_t req, ans;
    hwrm_selftest_exec_input_t selftest_exec_req;
    hwrm_selftest_exec_output_t *selftest_exec_resp;
    struct nlattr *na = NULL;
    int next_len = 0, remaining = BNXT_BUF_MAX;

    memset(&selftest_exec_req, 0, sizeof(selftest_exec_req));
    memset(&req, 0, sizeof(req));
    memset(&ans, 0, sizeof(ans));

    if (construct_hdrs(&req, ifindex, &remaining, &na)) {
        printf("\nFUGAZI_NL %d: construct_hdrs failed", port_id);
        return (FAILED);
    }

    /* Add actual request */
    next_len = sizeof(selftest_exec_req) + NLA_HDRLEN;
    if (validate_next_buf(remaining, next_len)) {
        na->nla_type = BNXT_ATTR_REQUEST;
        na->nla_len = next_len; /* Message length */
        selftest_exec_req.req_type = HWRM_SELFTEST_EXEC;
        selftest_exec_req.flags |= SELFTEST_EXEC_REQ_FLAGS_REGISTER_TEST;
        memcpy((char *)get_gelmnsg_nla_data(na), &selftest_exec_req, 
               sizeof(selftest_exec_req));
        req.n.nlmsg_len += NLA_ALIGN(na->nla_len);
    } else {
        printf("\nFUGAZI_NL %d: xcvr_detect: Failed to insert REQUEST", 
                port_id);
        return (FAILED);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\n FUGAZI Port %d: hwrm_selftest_exec_input  %x", port_id, 
                 selftest_exec_req.flags);
    }
    if (send_gnl_msg(g_mng.socket_fd, &req, &ans) < 0) {
        printf("\nFUGAZI_NL %d: xcvr_detect: send_gnl_msg returned error",
                port_id);
        return (FAILED);
    }

    /* Parse the reply message */
    na = (struct nlattr *) get_genlmsg_data(&ans);
    selftest_exec_resp = (hwrm_selftest_exec_output_t *)get_gelmnsg_nla_data(na);

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\n FUGAZI Port %d: Requested tests  %x", 
                port_id, selftest_exec_resp->requested_tests);
        printf("\n FUGAZI Port %d: Test success  %x", port_id, 
                selftest_exec_resp->test_success);
    }

    if (selftest_exec_resp->error_code) {
        printf("\nFUGAZI_NL %d: write error_code 0x%x",
                port_id, selftest_exec_resp->error_code);
        return (FAILED);
    }
    if (selftest_exec_resp->test_success != 
        SELFTEST_EXEC_RESP_TEST_SUCCESS_REGISTER_TEST) {
        printf("\nFUGAZI_NL %d: Register test failed 0x%x",
                port_id, selftest_exec_resp->requested_tests);
        return (FAILED);
    }
    return (PASSED);
}

/******************************************************************************
 *
 * Function: bnxt_bus_release
 *
 * Description: The firmware processes the HWRM command and payload contained 
 *              in the message. This function released MDIO BUS to BCM57412 
 *              firmware in order detect LASI. 
 *
 * Inputs      : port_id - Port ID
 *             : ifindex - network index
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/

int bnxt_bus_release (uint16_t port_id, uint32_t ifindex)
{
    bcm_nl_request_msg_t req, ans;
    hwrm_port_phy_mdio_bus_release_input_t bus_release_req;
    hwrm_port_phy_mdio_bus_release_output_t *bus_release_resp;
    struct nlattr *na = NULL;
    int next_len = 0, remaining = BNXT_BUF_MAX;
    int      *unused_1;

    memset(&bus_release_req, 0, sizeof(bus_release_req));
    memset(&req, 0, sizeof(req));
    memset(&ans, 0, sizeof(ans));

    if (construct_hdrs(&req, ifindex, &remaining, &na)) {
        printf("\nFUGAZI_NL %d: construct_hdrs failed", port_id);
        return (FAILED);
    }

    /* Add actual request */
    next_len = sizeof(bus_release_req) + NLA_HDRLEN;
    if (validate_next_buf(remaining, next_len)) {
        na->nla_type = BNXT_ATTR_REQUEST;
        na->nla_len = next_len; /* Message length */
        bus_release_req.req_type = HWRM_PORT_PHY_MDIO_BUS_RELEASE;
        bus_release_req.port_id = port_id;
        bus_release_req.client_id = 0x10;
        memcpy((char *)get_gelmnsg_nla_data(na), &bus_release_req, 
                sizeof(bus_release_req));
        req.n.nlmsg_len += NLA_ALIGN(na->nla_len);
        if (diagflag_xram & D_DEBUG_OPTIONS) {
            printf("bus_release_req: \n");
            printf("bus_release_req.req_type=0x%x \n", (int)bus_release_req.req_type);
            printf("bus_release_req.cmpl_ring=0x%x\n", (int)bus_release_req.cmpl_ring);
            printf("bus_release_req.seq_id=0x%x \n",   (int)bus_release_req.seq_id);
            printf("bus_release_req.target_id=0x%x\n", (int)bus_release_req.target_id);
            printf("bus_release_req.resp_addr=0x%x\n", (int)bus_release_req.resp_addr);
            printf("bus_release_req.port_id=0x%x\n",   (int)bus_release_req.port_id);
            printf("bus_release_req.client_id=0x%x\n", (int)bus_release_req.client_id);
            unused_1 = (int *)&bus_release_req.unused_0[0];
            printf("bus_release_req.unused_0=0x%x\n", *unused_1);
        }
    } else {
        printf("\nFUGAZI_NL %d: xcvr_detect: Failed to insert REQUEST", port_id);
        return (FAILED);
    }

    if (send_gnl_msg(g_mng.socket_fd, &req, &ans) < 0) {
        printf("\nFUGAZI_NL %d: xcvr_detect: send_gnl_msg returned error", port_id);
        return (FAILED);
    }

    /* Parse the reply message */
    na = (struct nlattr *) get_genlmsg_data(&ans);
    bus_release_resp  = (hwrm_port_phy_mdio_bus_release_output_t *)get_gelmnsg_nla_data(na);

    if (diagflag_xram & D_DEBUG_OPTIONS) {
        printf("\nbus_release_resp: \n");
        printf("bus_release_resp.error_code=0x%x \n", (int)bus_release_resp->error_code);
        printf("bus_release_resp.req_type=0x%x\n", (int)bus_release_resp->req_type);
        printf("bus_release_resp.seq_id=0x%x \n",   (int)bus_release_resp->seq_id);
        printf("bus_release_resp.resp_len=0x%x\n", (int)bus_release_resp->resp_len);
        printf("bus_release_resp.unused_0=0x%x\n", (int)bus_release_resp->unused_0);
        printf("bus_release_resp.clients_id=0x%x\n", (int)bus_release_resp->clients_id);
        unused_1 = (int *)bus_release_resp->unused_1;
        printf("bus_release_resp.unused_1=0x%x\n", *unused_1);
        printf("bus_release_resp.valid=0x%x\n", (int)bus_release_resp->valid);
    }
    if (bus_release_resp->error_code) {
        printf("\nFUGAZI_NL %d: xcvr_detect: Read error_code 0x%x",
                port_id, bus_release_resp->error_code);
        return (FAILED);
    }
    return (PASSED);
}
/******************************************************************************
 *
 * Function: bcm57412_mdio_bus_release 
 *
 * Description: This utility release MDIO bus 
 *
 * Inputs      : none
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int bcm57412_mdio_bus_release (int mode)
{
    int retval = PASSED;
    int ix = 0;
    int ifindex, start_port, end_port;
    char *tname = "BCM57412 MDIO BUS release ";
    int port;

    if (mode == FALSE) {
        testname("%s", tname);
        port = gethex_answer("\nPlease enter port number (0~11), (0xff for all)"
                             " : ", 0xff, 0, 0xff);
    } else {
        port = 0xff;
    }

    bnxt_impl_init_netlink();
    if (port == 0xff) {
        start_port = 0;
        end_port = 12;
    } else {
        start_port = port ;
        end_port = port + 1;
    }
    for (ix = start_port; ix < end_port; ix++) {
        ifindex = fugazi_eth_get_ifindex(ix);
        if (diagflag_xram & D_DEBUG_OPTIONS) {
            printf("\nifindex = %x, \n", ifindex);
        }
        if (bnxt_bus_release(ix, ifindex)) {
            printf("Port %d: MDIO BUS release not responding", ix);
            retval = FAILED;
            continue;
        }
    }
    bnxt_impl_deinit_netlink();
    return (retval);
}
/******************************************************************************
 *
 * Function: bnxt_bus_acquire
 *
 * Description: The firmware processes the HWRM command and payload contained 
 *              in the message. This function acquired MDIO BUS from BCM57412 
 *              firmware 
 *
 * Inputs      : port_id - Port ID
 *             : ifindex - network index
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/

int bnxt_bus_acquire (uint16_t port_id, uint32_t ifindex)
{
    bcm_nl_request_msg_t req, ans;
    hwrm_port_phy_mdio_bus_acquire_input_t bus_acquire_req;
    hwrm_port_phy_mdio_bus_acquire_output_t *bus_acquire_resp;
    struct nlattr *na = NULL;
    int next_len = 0, remaining = BNXT_BUF_MAX;
    uint16_t *unused;
    int      *unused_1;

    memset(&bus_acquire_req, 0, sizeof(bus_acquire_req));
    memset(&req, 0, sizeof(req));
    memset(&ans, 0, sizeof(ans));

    if (construct_hdrs(&req, ifindex, &remaining, &na)) {
        printf("\nFUGAZI_NL %d: construct_hdrs failed", port_id);
        return (FAILED);
    }

    /* Add actual request */
    next_len = sizeof(bus_acquire_req) + NLA_HDRLEN;
    if (validate_next_buf(remaining, next_len)) {
        na->nla_type = BNXT_ATTR_REQUEST;
        na->nla_len = next_len; /* Message length */
        bus_acquire_req.req_type = HWRM_PORT_PHY_MDIO_BUS_ACQUIRE;
        bus_acquire_req.port_id = port_id;
        bus_acquire_req.client_id = 0x10;
        bus_acquire_req.mdio_bus_timeout = 0xFFFF;
        memcpy((char *)get_gelmnsg_nla_data(na), &bus_acquire_req, 
                sizeof(bus_acquire_req));
        req.n.nlmsg_len += NLA_ALIGN(na->nla_len);
        if (diagflag_xram & D_DEBUG_OPTIONS) {
            printf("bus_acquire_req: \n");
            printf("bus_acquire_req.req_type=0x%x \n", (int)bus_acquire_req.req_type);
            printf("bus_acquire_req.cmpl_ring=0x%x\n", (int)bus_acquire_req.cmpl_ring);
            printf("bus_acquire_req.seq_id=0x%x \n",   (int)bus_acquire_req.seq_id);
            printf("bus_acquire_req.target_id=0x%x\n", (int)bus_acquire_req.target_id);
            printf("bus_acquire_req.resp_addr=0x%x\n", (int)bus_acquire_req.resp_addr);
            printf("bus_acquire_req.port_id=0x%x\n",   (int)bus_acquire_req.port_id);
            printf("bus_acquire_req.client_id=0x%x\n", (int)bus_acquire_req.client_id);
            printf("bus_acquire_req.mdio_bus_timeout=0x%x\n", 
                    (int)bus_acquire_req.mdio_bus_timeout);
            unused = (uint16_t *)&bus_acquire_req.unused_0[0];
            printf("bus_acquire_req.unused_0=0x%x\n", *unused);
        }
    } else {
        printf("\nFUGAZI_NL %d: xcvr_detect: Failed to insert REQUEST", port_id);
        return (FAILED);
    }

    if (send_gnl_msg(g_mng.socket_fd, &req, &ans) < 0) {
        printf("\nFUGAZI_NL %d: xcvr_detect: send_gnl_msg returned error", port_id);
        return (FAILED);
    }

    /* Parse the reply message */
    na = (struct nlattr *) get_genlmsg_data(&ans);
    bus_acquire_resp  = (hwrm_port_phy_mdio_bus_acquire_output_t *)get_gelmnsg_nla_data(na);

    if (diagflag_xram & D_DEBUG_OPTIONS) {
        printf("\nbus_acquire_resp: \n");
        printf("bus_acquire_resp.error_code=0x%x \n", (int)bus_acquire_resp->error_code);
        printf("bus_acquire_resp.req_type=0x%x\n", (int)bus_acquire_resp->req_type);
        printf("bus_acquire_resp.seq_id=0x%x \n",   (int)bus_acquire_resp->seq_id);
        printf("bus_acquire_resp.resp_len=0x%x\n", (int)bus_acquire_resp->resp_len);
        printf("bus_acquire_resp.unused_0=0x%x\n", (int)bus_acquire_resp->unused_0);
        printf("bus_acquire_resp.client_id=0x%x\n", (int)bus_acquire_resp->client_id);
        unused_1 = (int *)bus_acquire_resp->unused_1;
        printf("bus_acquire_resp.unused_1=0x%x\n", *unused_1);
        printf("bus_acquire_resp.valid=0x%x\n", (int)bus_acquire_resp->valid);
    }
    /* CSCvw09463 : MDIO Bus arbitration "valid" not set. Instead, check error_code */
    if (bus_acquire_resp->error_code) {
        printf("\nFUGAZI_NL %d: xcvr_detect: Read error_code 0x%x",
                port_id, bus_acquire_resp->error_code);
        return (FAILED);
    }
    return (PASSED);
}
/******************************************************************************
 *
 * Function: bcm57412_mdio_bus_acquire 
 *
 * Description: This utility acquire the mido bus 
 *
 * Inputs      : none
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int bcm57412_mdio_bus_acquire (int mode)
{
    int retval = PASSED;
    int ix = 0;
    int ifindex, start_port, end_port;
    char *tname = "BCM57412 MDIO BUS acquire ";
    int port;
    if (mode == FALSE) {
        testname("%s", tname);
        port = gethex_answer("\nPlease enter port number (0~11), (0xff for all)"
                             " : ", 0xff, 0, 0xff);
    } else {
        port = 0xff;
    }
    bnxt_impl_init_netlink();
    if (port == 0xff) {
        start_port = 0;
        end_port = 12;
    } else {
        start_port = port ;
        end_port = port + 1;
    }
    for (ix = start_port; ix < end_port; ix++) {
        ifindex = fugazi_eth_get_ifindex(ix);
        if (diagflag_xram & D_DEBUG_OPTIONS) {
            printf("\nifindex = %x, \n", ifindex);
        }
        if (bnxt_bus_acquire(ix, ifindex)) {
            printf("Port %d: MDIO BUS acquire not responding", ix);
            retval = FAILED;
            continue;
        }
    }
    bnxt_impl_deinit_netlink();
    return (retval);
}


/******************************************************************************
 *
 * Function: sfp_display_eeprom
 *
 * Description: Print SFP EEPROM value
 *
 * Inputs      : *buf  - point to date buff to be print.
 *               len - number of byte to be print.
 * Outputs     : none
 *
 *****************************************************************************/
static void sfp_display_eeprom (uchar *buf, int len)
{
    int ix, jx=0;
    uchar *p_buf_data = (uchar *)buf;
    char str_buf[32] = "NULL";
    uchar ch;

    /* display 16 bytes per row */
    for ( ix=0; ix<len; ix++, p_buf_data++ ) {
       if ( (ix % 16) == 0 ) {
           memcpy(str_buf, p_buf_data, 16);
           printf("\n");
           printf("%04X: ",ix);
       }
       /* print hex value */
       printf("%02X ", *p_buf_data);

       /* conver to string */
       ch = buf[ix];
       if (ch < ' ' || ch >= 0x7F) {
           ch = '.';
       } else {
           ch = buf[ix] & 0x7F;
       }
       str_buf[jx++] = ch;

       /* print string */
       if ( ((ix+1) % 16) == 0 ) {
           printf("    %s", str_buf);
           memset(str_buf, 0, sizeof(str_buf));
           jx = 0;
       }
    }
    printf("\n");
}

/******************************************************************************
 *
 * Function: sfp_read_eeprom
 *
 * Description: Read SFP EEPROM value. read 16 bytes a time
 *              by total 256 bytes via netlink.
 *
 * Inputs      : *buf  - point to date buff to be print.
 *               len - number of byte to be print.
 * Outputs     : none
 *
 *****************************************************************************/
static int sfp_read_eeprom (int ix, int addr, uchar *data_buf)
{
    int   len, remaining, sfp_eeprom_buf_ix;
    int   page_num, offset, ifindex;
    uchar sfp_buf[BNXT_BUF_MAX] = "NULL";

    remaining = SFP_EEPROM_SIZE;
    len = SFP_EEPROM_READ_LEN;
    sfp_eeprom_buf_ix = 0;
    offset = 0x0;
    page_num = 0x0;

    while (remaining > 0) {
        memset(sfp_buf, 0, sizeof(sfp_buf));
        /* Read SFP EEPROM value from netlink 16bytes a time */
        ifindex = fugazi_eth_get_ifindex(ix);
        if (bnxt_netlink_i2c_read(addr, ifindex, page_num,
                                  ix, offset, len, (uint8_t *)sfp_buf)) {
            cterr('f',0,"Port %d: eeprom read: Target not responding", ix);
            bnxt_impl_deinit_netlink();
            return (FAILED);
        }
        memcpy(&data_buf[sfp_eeprom_buf_ix], sfp_buf, len);
        sfp_eeprom_buf_ix += len;
        offset += len;
        remaining -= len;
    } /* while (remaining > 0) { */

    return (PASSED);
}


/******************************************************************************
 *
 * Function: sfp_display_cookie
 *
 * Description: This utility dump the SFP cookie via netlink
 *
 * Inputs      : port - port number
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int sfp_display_cookie (int port)
{
    int   rc = PASSED;
    int   ix, jx, ifindex,  start_port, end_port;
    uchar sfp_eeprom_buf[SFP_EEPROM_SIZE] = "NULL";
    int   sfp_absent, sfp_read_fail=0;
    int   addr;

    if (port == 0xff) {
        start_port = 0;
        end_port = BCM57412_SFP_PORT;
    } else {
        start_port = port ;
        end_port = port + 1;
    }

    bnxt_impl_init_netlink();

    for (ix = start_port; ix < end_port; ix++) {
        memset(sfp_eeprom_buf, 0, sizeof(sfp_eeprom_buf));

        /* check if SFP PRESENT */
        ifindex = fugazi_eth_get_ifindex(ix);
        sfp_absent = bnxt_sfp_detect(ix, ifindex);

        /* SFP present */
        if (!sfp_absent) {
            for (jx=0; jx<2; jx++ ) {
                if (jx == 0) {
                    /* read SFP EEPROM at address 0xA0 */
                    addr = I2C_DEV_ADDR_A0;
                } else {
                    /* read SFP EEPROM at address 0xA0 */
                    addr = I2C_DEV_ADDR_A2;
                }
                printf("\nPort %d Module - 0x%02X:\n", ix, addr);

                /* Read SFP EEPROM */
                sfp_read_fail =  sfp_read_eeprom(ix, addr, sfp_eeprom_buf);
                if (sfp_read_fail) {
                    printf("SFP READ(%X)ERR!\n", addr);
                    rc = FAILED;
                    continue;
                }
                /* display SFP cookie value */
                sfp_display_eeprom(sfp_eeprom_buf, SFP_EEPROM_SIZE);

            } /* for (jx=0; jx<2; jx++ ) {*/
        } else {
            /* SFP NOT present */
            printf("\nPort %d Module:\n", ix);
            printf("SFP ABSENT\n");
        } /* if (!sfp_absent) { */

    }  /* for (ix = 0; ix < BCM57412_SFP_PORT; ix++) { */

    bnxt_impl_deinit_netlink();

    return (rc);
}

/******************************************************************************
 *
 * Function: sfp_display_info
 *
 * Description: This utility display SFP part#, SN#; 0: display SFP cookie via netlink
 *
 * Inputs      : none
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int sfp_display_info (void)
{
    int   rc = PASSED;
    int   ix, jx, ifindex;
    uchar sfp_eeprom_buf[SFP_EEPROM_SIZE] = "NULL";
    char  fmt_buff[SZ_PN_SN_SH] = {'\0'};
    char  msg_ee_err[SZ_MSG_ERR] = {'\0'};
    char  sernum_buf[SFP_SN_SZ] = {'\0'};
    int   sernum_offset = SFP_EEPROM_SERIAL_NO_OFFSET;
    int   sernum_len = SFP_EEPROM_SERIAL_NO_LENGTH;
    char  partnum_buf[SFP_VENDOR_PN_SZ+1] = {'\0'};
    int   partnum_offset = SFP_EEPROM_VENDOR_PART_NO_OFFSET;
    char  vendor_name_buf[SFP_EEPROM_VENDOR_NAME_LEN+1] = {'\0'};
    int   vendor_name_offset = SFP_EEPROM_VENDOR_NAME_OFFSET;
    char  pid_buf[SFP_EEPROM_A2_PID_LEN+1] = {'\0'};
    int   pid_offset = SFP_EEPROM_A2_PID_OFFSET;
    int   sfp_absent, sfp_read_fail=0, len;
    int   addr;

    printf("\n SFP      State     Vendor PN         S/N               Vendor name       PID\n");

    bnxt_impl_init_netlink();

    for (ix = 0; ix < BCM57412_SFP_PORT; ix++) {
        if (ix < FUGAZI_1G_eth_4) {
            printf("[TE0/1/%d] ", ix );
        } else {
            printf("[GE0/0/%d] ", (ix - FUGAZI_1G_eth_4) );
        }

        memset(fmt_buff, 0, SZ_PN_SN_SH);
        memset(msg_ee_err, 0, sizeof(msg_ee_err));
        memset(sfp_eeprom_buf, 0, sizeof(sfp_eeprom_buf));

        /* check if SFP PRESENT */
        ifindex = fugazi_eth_get_ifindex(ix);
        sfp_absent = bnxt_sfp_detect(ix, ifindex);

        /* SFP present */
        if (!sfp_absent) {

            for (jx=0; jx<2; jx++ ) {
                if (jx == 0) {
                    /* read SFP EEPROM at address 0xA0 */
                    addr = I2C_DEV_ADDR_A0;
                } else {
                    /* read SFP EEPROM at address 0xA0 */
                    addr = I2C_DEV_ADDR_A2;
                }

                /* Read SFP EEPROM */
                sfp_read_fail =  sfp_read_eeprom(ix, addr, sfp_eeprom_buf);
                if (sfp_read_fail) {
                    len = strlen(msg_ee_err);
                    sprintf(&msg_ee_err[len], MSG_EE_ERR, addr);
                }

                /* Prepare SFP info */
                if ( jx == 0 ) {
                    /* copy SFP info from SFP EEPROM at address 0xA0 */
                    if (!sfp_absent) {
                        /* Copy Vendor Part # to partnum buffer */
                        memset(partnum_buf, 0, sizeof(partnum_buf));
                        memcpy((void *)partnum_buf, &sfp_eeprom_buf[partnum_offset], SFP_VENDOR_PN_SZ);
                        len = strlen(fmt_buff);
                        sprintf(&fmt_buff[len],FMT_PN, sfp_read_fail ? msg_ee_err : (char *)partnum_buf);

                        /* Copy Serial # to sernum buffer */
                        memset(sernum_buf, 0, sizeof(sernum_buf));
                        memcpy((void *)sernum_buf, &sfp_eeprom_buf[sernum_offset], sernum_len);
                        len = strlen(fmt_buff);
                        sprintf(&fmt_buff[len],FMT_SN, sfp_read_fail ? (char *)msg_ee_err : (char *)sernum_buf);

                        /* Copy Vendor name to vendor name buffer */
                        memset(vendor_name_buf, 0, sizeof(vendor_name_buf));
                        memcpy((void *)vendor_name_buf, &sfp_eeprom_buf[vendor_name_offset], SFP_EEPROM_VENDOR_NAME_LEN);
                        len = strlen(fmt_buff);
                        sprintf(&fmt_buff[len],FMT_PN, sfp_read_fail ? (char *)msg_ee_err  : (char *)vendor_name_buf);
                    }
                } else {
                    /* copy SFP info from SFP EEPROM at address 0xA2 */
                    if (!sfp_absent) {
                        /* Copy PID to pid buffer */
                        memset(pid_buf, 0, sizeof(pid_buf));
                        memcpy((void *)pid_buf, &sfp_eeprom_buf[pid_offset], SFP_EEPROM_A2_PID_LEN);
                        len = strlen(fmt_buff);
                        sprintf(&fmt_buff[len],FMT_PN, sfp_read_fail ? (char *)msg_ee_err : (char *)pid_buf);
                    }
                }  /* if (jx == 0) { */
            } /* for (jx=0; jx<2; jx++ ) {*/
        } /* if (!sfp_absent) { */

        /* display SFP info */
        if (!sfp_absent) {
            if (sfp_read_fail) {
                printf(FMT_STATE, "PRESENT");
                rc = FAILED;
            } else {
                printf(FMT_STATE, "GOOD");
            }
        } else {
            printf(FMT_STATE, "ABSENT");
        }
        printf("%s\n", fmt_buff);

    }  /* for (ix = 0; ix < BCM57412_SFP_PORT; ix++) { */

    bnxt_impl_deinit_netlink();

    return (rc);
}


/*
 *------------------------------------------------------------------
 * $Log: diag_bcm57412_utils.c,v $
 * Revision 1.2  2021/06/02 08:22:34  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.4.4  2021/03/19 18:35:01  pdoong
 * Add Dump SFP Module Info utility to display SFP Vendor PN, S/N, Vendor name, PID
 *
 * Revision 1.1.4.3  2020/10/21 03:05:17  iachang
 * CSCvw09463 : MDIO Bus arbitration "valid" not set. Instead, check error_code
 *
 * Revision 1.1.4.2  2020/08/26 02:37:48  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.2.15  2020/08/21 03:39:17  iachang
 * Change cterr() to printf() inside bcm57412_mdio_bus_acquire/release() function ,
 * avoid BCM57412 FW didn't support MIDO bus control and get cterr() message when platform initial
 *
 * Revision 1.1.2.14  2020/08/20 06:36:20  iachang
 * PRRQ CSCvo59196-5 : BCM57412 MAC code review
 *
 * Revision 1.1.2.13  2020/08/04 07:03:39  iachang
 * Code clean up.
 *
 * Revision 1.1.2.12  2020/07/21 09:13:27  iachang
 * Removed checking bus_acquire_resp->valid value
 *
 * Revision 1.1.2.11  2020/07/03 07:34:20  iachang
 * Support bcm57412_mdio_bus_release() and bcm57412_mdio_bus_acquire()
 * Move those funcitons from diag_bcm57412_test.c to diag_bcm57412_utils.c
 *
 * Revision 1.1.2.10  2020/07/02 22:42:42  pdoong
 * Add debug msg for mdio_bus_acquire/release utility
 *
 * Revision 1.1.2.9  2020/06/12 02:35:06  iachang
 * Support bcm57412_mdio_bus_acquire and bcm57412_mdio_bus_release for all GE port
 *
 * Revision 1.1.2.8  2020/04/22 07:11:04  iachang
 * Add BCM57412 mdio_bus_acquire function
 *
 * Revision 1.1.2.7  2020/03/18 06:51:44  iachang
 * Create independent file for LASI test
 *
 * Revision 1.1.2.6  2020/01/09 08:16:11  iachang
 * Add bcm57412_mdio_bus_release function, and modify BCM57412 sideband tx_dis utility.
 *
 * Revision 1.1.2.5  2019/11/14 08:29:11  iachang
 * Implement SFP present function.
 *
 * Revision 1.1.2.4  2019/11/06 02:23:53  iachang
 * Modify BCM57412 utility port mapping.
 *
 * Revision 1.1.2.3  2019/10/22 01:55:19  iachang
 * Implement side_band tx_dis utility
 *
 * Revision 1.1.2.2  2019/10/16 08:53:06  iachang
 * Port BCM57412 Register test via Netlink to replace Broadcom Cdiag tool
 *
 * Revision 1.1.2.1  2019/09/27 08:03:58  iachang
 * Changed "BCM57412 SFP i2c Test" from Broadcom CDiag to Netlink.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
