/* $Id: tdm.c,v 1.3 2021/04/15 00:53:07 achiu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/ppb/src/tdm.c,v $
 *---------------------------------------------------------------------- 
 * tdm.c
 * 
 * TDM driver for Oakenshield modules.
 *
 * Copyright (c) 2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 * Owen Lin - 2016
 *----------------------------------------------------------------------
 */
#include <stdint.h>
#include "stdio.h"
#include "string.h"
#include "types.h"
#include "menu.h"
#include "diag_ppb.h"
#include "common.h"
#include "error.h"
#include "debug_console.h"
#include "uart.h"
#include "lsi_sp27xx_reg.h"
#include "libgeneric.h"
#include "libuart.h"
#include "diag_fpga.h"
#include "ssp.h"
#include "tdmsw16_fpga.h"


oak_tdm_info_t g_tdm_info;
tdm_ds0_dump_info_t g_ds0_dump_info;
tdm_status_e oak_tdmsw16_setup_connection (uint16_t, uint16_t, uint16_t, uint16_t, boolean, boolean);

const oak_tdmsw16_porttype_e 
oakenshield_tdmsw16_ports[OAKENSHIELD_TDMSW16_MAX_STREAMS] = {
    /* codec IOM2 port 0 and 1 on MB */
    TDMSW16_PORT_CODEC, TDMSW16_PORT_CODEC,
    /* codec IOM2 port 2 on MB, 3 on DC */
    TDMSW16_PORT_CODEC, TDMSW16_PORT_CODEC,
    /* CPU port stream 4(connected to SP270x) */
    TDMSW16_PORT_CPU, 
    /* Not Used stream 5 */
    TDMSW16_PORT_NC, 
    /* codec IOM2 port 4 and 5 on DC */
    TDMSW16_PORT_CODEC, TDMSW16_PORT_CODEC,
    /* CPU port (connected to SP270x) */
    TDMSW16_PORT_CPU, 
    /* the rest are not used 9~11*/
    TDMSW16_PORT_NC, TDMSW16_PORT_NC, TDMSW16_PORT_NC,
    /* Stream 13 for DS0 dump */
    TDMSW16_PORT_DS0_DUMP,
    TDMSW16_PORT_NC, TDMSW16_PORT_NC,
};

const oak_tdmsw16_porttype_e
phoenix_tdmsw16_ports[OAKENSHIELD_TDMSW16_MAX_STREAMS] = {
    /* codec stream 0 and 1 on MB */
    TDMSW16_PORT_CODEC, TDMSW16_PORT_CODEC,
    /* codec stream 2 and 3 on DB1 */
    TDMSW16_PORT_CODEC, TDMSW16_PORT_CODEC,
    /* CPU port stream 4(connected to SP270x) */
    TDMSW16_PORT_CPU,
    /* Not Used stream 5 */
    TDMSW16_PORT_NC,
    /* codec stream 6 and 7 on DB2 */
    TDMSW16_PORT_CODEC, TDMSW16_PORT_CODEC,
    /* CPU port stream 8 (connected to SP270x) */
    TDMSW16_PORT_CPU,
    /* the rest are not used 9 */
    TDMSW16_PORT_NC,
    /* codec stream 10 on DB2 */
    TDMSW16_PORT_CODEC,
    /* codec stream 11 and 12 on BD3 */
    TDMSW16_PORT_CODEC, TDMSW16_PORT_CODEC,
    /* the rest are not used 13 */
    TDMSW16_PORT_DS0_DUMP,
    /* codec stream 14 on DB3 */
    TDMSW16_PORT_CODEC,
};


#define OAK_SKU_LAST 0xff
static const uint8_t oak_MB_max_ports_local[OAK_SKU_LAST] = {
    10, 12, 12,       // ananlog voice ports
};
static const uint8_t oak_DC_max_ports_local[OAK_SKU_LAST] = {
    8, 8, 16,       // ananlog voice ports
};
/* list Phoenix each voice port on different TDM port with stream 0 ~ 15 */
static const uint8_t phoenix_144fxs_max_ports_local[OAK_SKU_LAST] = {
    12, 12, 12, 12, 0, 0, 16, 16, 0, 0, 16, 16, 16, 0, 16, 0,
};
/* Combine all Phoenix voice port 12x4+16x6= 132*/
#define PHOENIX_ALL_PORT_NUM_FOR_DS0_DUMP 132

#define TDMSW_ERROR  bsp_debug_printf




/* 
 * tdmsw_indirect_read 
 * 
 * Description:
 *     Read tdmsw register via indirect register.
 *
 * Parameters:
 *     offset - register offset.
 *     val    - point to output value.
 *
 * Returns:
 *     Success or timeout
 */
tdm_status_e tdmsw_indirect_read (uint16_t offset, uint32_t *val)
{
    /* Refer diag_fpgc.c read fpga function */
    fpga_spi_indirect_read(offset, 4, val);

    return(TDMSW64_SUCCESS);

}


/* 
 * tdmsw_indirect_write
 * 
 * Description:
 *     write tdmsw register via indirect register.
 *
 * Parameters:
 *     offset - register offset.
 *     val    - value to write.
 *
 * Returns:
 *     Success or timeout
 */
tdm_status_e oak_tdmsw_indirect_write (uint16_t offset, uint32_t val)
{
    /* Refer diag_fpga.c fpga write function */
    fpga_spi_indirect_write(offset, 4, val);    
    return(TDMSW64_SUCCESS);
}


/*
 * tdmsw_status_string
 *
 * Util function to convert a tdmsw status into string.
 */
static char *tdmsw_status_string (tdm_status_e code)
{
    switch (code) {
    case TDMSW64_SUCCESS:
        return "SUCCESS";

    case TDMSW64_WR_TIMEOUT:
        return "WR_TIMEOUT";

    case TDMSW64_RD_TIMEOUT:
        return "RD_TIMEOUT";

    case TDMSW64_CONNECT_CONNECT:
        return "CONNECT_CONNECT";

    case TDMSW64_DISCONNECT_DISCONNECT:
        return "DISCONNECT_DISCONNECT";

    case TDMSW64_INVALID_STREAM_NUM:
        return "INVALID_STREAM_NUM";

    case TDMSW64_INVALID_TIMESLOT:
        return "INVALID_TIMESLOT";

    case TDMSW64_INVALID_RATE:
        return "INVALID_RATE";

    case TDMSW64_INVALID_MODE:
        return "INVALID_MODE";

    case TDMSW64_VERIFICATION_FAILED:
        return "VERIFICATION_FAILED";

    default:
        return "unknown";
    }


}



/*
 * tdm_ds0_dump_connect_notify
 *
 * Description:
 *     When a new cross connect is made, this function will be invoked.
 *     It will check if the new connection might modify the current
 *     ds0-dump source and destination connections.
 *     If so, reconnect the ds0 dump to capture the new destination.
 *     This most likely will happen when a voice call transition from
 *     DSP call into TDM hair-pin call.
 *
 * Parameters:
 *     src_str - source stream
 *     src_ts  - source timeslot.
 *     dst_str - destination stream.
 *     dst_ts - destination timeslot.
 *
 * Returns: None.
 */
static void tdm_ds0_dump_connect_notify (uint16_t src_str, uint16_t src_ts, 
                                         uint16_t dst_str, uint16_t dst_ts)
{
    uint16_t other_str, other_ts;
    boolean conn_modified = 0;

    if (!g_ds0_dump_info.active) {
        return;
    }

    /* check if ds0-dump connection will be affected.
     * by design, if connection changed, we will keep the one
     * in g_ds0_dump_info.src_stream and change the other side. 
     */
    if (g_ds0_dump_info.rx_stream == src_str &&
        g_ds0_dump_info.rx_timeslot == src_ts) {
        other_str = dst_str;
        other_ts = dst_ts;
        conn_modified = 1;
    } else if (g_ds0_dump_info.rx_stream == dst_str &&
               g_ds0_dump_info.rx_timeslot == dst_ts) {
        other_str = src_str;
        other_ts = src_ts;
        conn_modified = 1;
    }

    /* if connection has changed, set the new tx stream/slot */
    if (conn_modified) {
        g_ds0_dump_info.tx_stream = other_str;
        g_ds0_dump_info.tx_timeslot = other_ts;
        oak_tdmsw16_setup_connection(other_str, other_ts, 
                                        g_tdm_info.ds0_dump_stream_num,
                                        1, /* tx use timeslot 1 */
                                        0, /* one way */
                                        1 /* forced */);
    }

    return;
}



/*
 * tdm_get_cid_address_offset
 *
 * Description:
 *     compute the cid address based on stream and timeslot.
 *
 * Parameters:
 *     rate - stream rate in oak_tdmsw16_str_rate_e
 *     stream - stream number.
 *     timeslot - timeslot number.
 *
 * Returns:
 *     cid (connection memory) address offset.
 */
static uint32_t tdm_get_cid_address_offset (oak_tdmsw16_str_rate_e rate,
                                            uint16_t stream, uint16_t timeslot)
{
    uint32_t offset = 0;

    switch (rate) {
    case TDMSW16_2MBPS:
        offset = ((stream * 128) + (timeslot * 4)) * 4;
        break;

    case TDMSW16_8MBPS:
        offset = ((stream * 128) + timeslot) * 4;
        break;

    case TDMSW16_16MBPS:
        offset = ((stream * 128) + ((timeslot % 2) * 128) + (timeslot>>1)) * 4;
        break;

    case TDMSW16_32MBPS:
        offset = ((stream * 128) + ((timeslot % 4) * 128) + (timeslot>>2)) * 4;
        break;

    default:
        TDMSW_ERROR("Error in %s: Invalid stream rate %d for stream %d\n",
                    __FUNCTION__, rate, stream);
        break;
    }

    return(offset);
}


/* 
 * oak_tdmsw16_is_ds0_connected 
 *
 * Description:
 *     Util function to check if a timeslot is connected to something.
 *
 * Parameters:
 *     str - stream number
 *     chan - timeslot.
 *     cid - optionally return the CID if this pointer is not NULL.
 *
 * Returns: 1 if connected, 0 if not connected.
 *
 */
static boolean oak_tdmsw16_is_ds0_connected (uint16_t str, uint16_t chan, 
                                                uint32_t *cid)
{
    tdmsw_stream_info_t *str_info;
    uint32_t my_cid;
    uint16_t my_cid_offset;
    tdm_status_e status;

    str_info = &g_tdm_info.streams[str];

    /* read the cid from fpga */
    my_cid_offset = tdm_get_cid_address_offset(str_info->rate, str, chan);
    status = FAILED;
    status = tdmsw_indirect_read(my_cid_offset, &my_cid);
    if (status != TDMSW64_SUCCESS) {
        TDMSW_ERROR("%s: Error in reading tdmsw cid %d, status=%d(%s)\n",
                    __FUNCTION__, my_cid_offset, status,
                    tdmsw_status_string(status));
        return(0);
    }

    if (cid) {
        *cid = my_cid;
    }

    if ((my_cid & CM_ODRV) == 0) {
        return(0);  // not connected
    } else {
        return(1);   // connected
    }

    /* should never reach this point. */
    return(0);
}




/*
 * oak_tdmsw_gen_cid
 *
 * Description:
 *     generate the cid content based on given src stream and timeslot.
 *
 * Parameters:
 *     rate - the stream rate.
 *     stream - source stream.
 *     timeslot - source timeslot.
 *
 * Returns:
 *     cid.
 */
static uint32_t oak_tdmsw_gen_cid (oak_tdmsw16_str_rate_e rate,
                                      uint16_t stream, uint16_t timeslot)
{
    uint32_t cid = 0;

    switch (rate) {
    case TDMSW16_2MBPS:
        cid = (stream * 128) + (timeslot * 4) + 3;
        break;

    case TDMSW16_8MBPS:
        cid = (stream * 128) + timeslot;
        break;

    case TDMSW16_16MBPS:
        cid = (stream * 128) + ((timeslot % 2) * 128) + (timeslot>>1);
        break;

    case TDMSW16_32MBPS:
        cid = (stream * 128) + ((timeslot % 4) * 128) + (timeslot>>2);
        break;

    default:
        TDMSW_ERROR("Error in %s: Invalid stream rate %d for stream %d\n",
                    __FUNCTION__, rate, stream);
        break;
    }

    return(cid);

}



/*
 * Name: oak_tdmsw16_conn_config
 *
 * Description:
 *      Configure connection store memory for tdm switch in Oakenshield FPGA.
 *      This is the actual worker routine that will be called for 
 *      setting and disconnecting connections.
 *
 * Parameters:
 *       rx_str         - source highway number (input stream)
 *       rx_chan        - source timeslot number (input DS0)
 *       tx_str         - destination highway number (output stream)
 *       tx_chan        - destination timeslot number (output DS0)
 *       data_mode      - operation/mode for the channel
 *       host_data      - force when mode is TDMSW64_HDSM
 */
static tdm_status_e oak_tdmsw16_tdm_conn_config (uint16_t rx_str, 
                                                    uint16_t rx_chan,
                                                    uint16_t tx_str,
                                                    uint16_t tx_chan,
                                                    tdmsw16_data_mode_e data_mode, 
                                                    uchar host_data)
{
    uint16_t cid_offset; /* location of connection memory for tx_str */
    uint32_t cid_value;
    oak_tdmsw16_str_rate_e rate;

    rate = g_tdm_info.streams[tx_str].rate;

    cid_offset = tdm_get_cid_address_offset(rate, tx_str, tx_chan);

    /* form cid value for write to connection memory */
    switch (data_mode) {
    case TDMSW16_HI_Z: 
        cid_value = CM_PASSWORD;  /* disabled */
        break;

    case TDMSW16_NORMAL:
        rate = g_tdm_info.streams[rx_str].rate;
        cid_value = oak_tdmsw_gen_cid(rate, rx_str, rx_chan);
        /* OR with password and output enable */
        cid_value |= (CM_PASSWORD | CM_ODRV);
        break;

    case TDMSW16_HDSM:
        cid_value = (CM_PASSWORD | CM_ODRV | host_data);
        break;

    default:
        TDMSW_ERROR("%s: Unknown mode %d\n",
                    __FUNCTION__, data_mode);
        return(TDMSW64_INVALID_MODE);
    }

    bsp_debug_printf("CID_offset: %x, CID_value: %x\n", cid_offset, cid_value);

    oak_tdmsw_indirect_write(cid_offset, cid_value);

    uint32_t rd_data;
    tdmsw_indirect_read(cid_offset, &rd_data);
    bsp_debug_printf("TDMSW connection memory %x: %x\n\r", cid_offset, rd_data);

    return(PASSED);
} 



/*
 * Name: oak_tdmsw16_disconnect_connection
 *
 * Description:
 *      Configure connection store memory for OAKENSHIELD TDM FPGA.
 *      Call oak_tdmsw16_conn_config by passing (0,0) as the source 
 *      stream/channel and TDMSW16_HI_Z to indicate this is disconnect
 *      connection.
 */
tdm_status_e oak_tdmsw16_disconnect_connection (uint16_t rx_str, 
                                                   uint16_t rx_chan, 
                                                   uint16_t tx_str, 
                                                   uint16_t tx_chan, 
                                                   boolean bidirection,
                                                   boolean forced)
{
    tdm_status_e status;
    tdmsw_stream_info_t* tx_str_info;
    tdmsw_stream_info_t* rx_str_info;

    /* check if stream number looks right */
    if (rx_str >= g_tdm_info.max_streams ||
        tx_str >= g_tdm_info.max_streams) {
        return(TDMSW64_INVALID_STREAM_NUM);
    }

    /* check if timeslot looks right */
    if (rx_chan >= g_tdm_info.streams[rx_str].max_timeslots ||
        tx_chan >= g_tdm_info.streams[tx_str].max_timeslots) {
        return(TDMSW64_INVALID_TIMESLOT);
    }

    /* check if timeslot already dis-connected. */
    if (!forced && !oak_tdmsw16_is_ds0_connected(tx_str, tx_chan, 0)) {
        g_tdm_info.dis_dis_conn_cnt++;
        return(TDMSW64_DISCONNECT_DISCONNECT);
    }

    /* if bi-directional, also check src_str */
    if (bidirection) {
        if (!forced && !oak_tdmsw16_is_ds0_connected(rx_str, rx_chan, 0)) {
            g_tdm_info.dis_dis_conn_cnt++;
            return(TDMSW64_DISCONNECT_DISCONNECT);
        }
    }

    /* all good. dis-connect */
    tx_str_info = &g_tdm_info.streams[tx_str];
    rx_str_info = &g_tdm_info.streams[rx_str];

    /* dis-connect rx_str to tx_str */
    status = oak_tdmsw16_tdm_conn_config(rx_str, rx_chan, 
                                            tx_str, tx_chan,
                                            TDMSW16_HI_Z, 0);
    if (status != TDMSW64_SUCCESS) {
        return(status);
    }

    /* dis-connect other direction if needed */
    if (bidirection) {
        /* disconnect tx_str to rx_str */
        status = oak_tdmsw16_tdm_conn_config(tx_str, tx_chan, 
                                                rx_str, rx_chan,
                                                TDMSW16_HI_Z, 0);
        if (status != TDMSW64_SUCCESS) {
            return(status);
        }
    }

    return(TDMSW64_SUCCESS);

}



/*
 * Name: oak_tdmsw16_setup_connection
 *
 * Description:
 *      Configure connection store memory for OAKENSHIELD TDM FPGA.
 *      Call oak_tdmsw16_conn_config by passing TDMSW16_NORMAL to
 *      Indicate this is a setup connection.
 *
 * Parameters:
 *     rx_str - source stream
 *     rx_chan - source channel.
 *     tx_str - destination stream.
 *     tx_chan - destination channel.
 *     bidirection - 1 if to connect both direction.
 *     forced - 1 to force connection.
 *
 * Returns: 
 *     SUCCESS or one of the TDM error code.
 *
 */
tdm_status_e oak_tdmsw16_setup_connection (uint16_t rx_str, 
                                              uint16_t rx_chan, 
                                              uint16_t tx_str, 
                                              uint16_t tx_chan, 
                                              boolean bidirection,
                                              boolean forced)
{
    tdm_status_e status;
    tdmsw_stream_info_t* tx_str_info;
    tdmsw_stream_info_t* rx_str_info;

    /* check if stream number looks right */
    if (rx_str >= g_tdm_info.max_streams ||
        tx_str >= g_tdm_info.max_streams) {
        return(TDMSW64_INVALID_STREAM_NUM);
    }

    /* check if timeslot looks right */
    if (rx_chan >= g_tdm_info.streams[rx_str].max_timeslots ||
        tx_chan >= g_tdm_info.streams[tx_str].max_timeslots) {
        return(TDMSW64_INVALID_TIMESLOT);
    }

    /* check if timeslot already connected. */
    if (!forced && oak_tdmsw16_is_ds0_connected(tx_str, tx_chan, 0)) {
        g_tdm_info.conn_conn_cnt++;
        return(TDMSW64_CONNECT_CONNECT);
    }

    /* if bi-directional, also check src_str */
    if (bidirection) {
        if (!forced && oak_tdmsw16_is_ds0_connected(rx_str, rx_chan, 0)) {
            g_tdm_info.conn_conn_cnt++;
            return(TDMSW64_CONNECT_CONNECT);
        }
    }
    /* notify the ds0-dump about the connection. */
    tdm_ds0_dump_connect_notify(rx_str, rx_chan, tx_str, tx_chan);

    /* all good. make connection */
    tx_str_info = &g_tdm_info.streams[tx_str];
    rx_str_info = &g_tdm_info.streams[rx_str];

    status = FAILED;

    /* make connection rx_str to tx_str */
    status = oak_tdmsw16_tdm_conn_config(rx_str, rx_chan, 
                                            tx_str, tx_chan,
                                            TDMSW16_NORMAL, 0);
    if (status != TDMSW64_SUCCESS) {
        return(status);
    }

    /* connect other direction if needed */
    if (bidirection) {
        /* make connection tx_str to rx_str */
        status = oak_tdmsw16_tdm_conn_config(tx_str, tx_chan, 
                                                rx_str, rx_chan,
                                                TDMSW16_NORMAL, 0);
        if (status != TDMSW64_SUCCESS) {
            return(status);
        }
    }

    return(TDMSW64_SUCCESS);

}


void test_tdmsw_xconnect (tdmsw_xconnect_cmd_t *cmd)
{
    int i;
    tdmsw_stream_info_t *stream_info;
    tdm_status_e status;

    /* check for parameter errors */
    if (cmd->src_str < 0 || cmd->src_str >= g_tdm_info.max_streams) {
        bsp_debug_printf("source stream %d not present for module PID = %s\n",
               cmd->src_str);
        return;
    }

    if (cmd->dst_str < 0 || cmd->dst_str >= g_tdm_info.max_streams) {
        bsp_debug_printf("destination stream %d not present for module PID = %s\n",
               cmd->dst_str);
        return;
    }

    stream_info = &g_tdm_info.streams[cmd->src_str];
    if (cmd->src_ts < 0 ||
        (cmd->src_ts + cmd->num_ts) > stream_info->max_timeslots) {
        bsp_debug_printf("source timeslots %d-%d is not valid for stream %d\n",
               cmd->src_ts, (cmd->src_ts + cmd->num_ts - 1), cmd->src_str);
        return;
    }

    stream_info = &g_tdm_info.streams[cmd->dst_str];
    if (cmd->dst_ts < 0 ||
        (cmd->dst_ts + cmd->num_ts) > stream_info->max_timeslots) {
        bsp_debug_printf("destination timeslots %d-%d is not valid for stream %d\n",
               cmd->src_ts, (cmd->dst_ts + cmd->num_ts - 1), cmd->dst_str);
        return;
    }

    for (i = 0; i < cmd->num_ts; i++) {
        if (cmd->connect) {
            status = oak_tdmsw16_setup_connection(cmd->src_str,
                                                         cmd->src_ts + i,
                                                         cmd->dst_str,
                                                         cmd->dst_ts + i,
                                                         0, /* one way */
                                                         1 /* forced */);
        } else {
            status = oak_tdmsw16_disconnect_connection(cmd->src_str,
                                                              cmd->src_ts + i,
                                                              cmd->dst_str,
                                                              cmd->dst_ts + i,
                                                              0, 1);
        }

        if (status != TDMSW64_SUCCESS) {
            bsp_debug_printf("Failed to %s for stream %d, chan %d to " \
                   "stream %d, chan %d\n", 
                   (cmd->connect) ? "connect" : "disconnect",
                   cmd->src_str, cmd->src_ts + i,
                   cmd->dst_str, cmd->dst_ts + i);
            return;
        }
    }

#if 0
    bsp_debug_printf(" TDMSW %s completed for stream %d, chan %d-%d to "\
           "stream %d, chan %d-%d\n", 
           (cmd->connect) ? "connect" : "disconnect",
           cmd->src_str, cmd->src_ts, cmd->src_ts + cmd->num_ts -1,
           cmd->dst_str, cmd->dst_ts, cmd->dst_ts + cmd->num_ts -1);
#endif

    return;
}


/*
 * Util function to initializes g_tdm_info structure.
 */
static void 
oak_tdmsw_fill_stream_info (uint16_t str_no, tdmsw_stream_info_t* str_info)
{
    if (is_phoenix()) {
        str_info->porttype = phoenix_tdmsw16_ports[str_no];
    } else {
        str_info->porttype = oakenshield_tdmsw16_ports[str_no];
    }
    /* based on stream assignment, populate other info. */
    switch (str_info->porttype) {
    case TDMSW16_PORT_NC:
        break;
    case TDMSW16_PORT_CODEC:
        str_info->rate = TDMSW16_2MBPS;
        str_info->max_timeslots = OAKENSHIELD_TDMSW16_MAX_CHANNELS_2M;
        break;

    case TDMSW16_PORT_CPU:
        str_info->rate = TDMSW16_16MBPS;
        str_info->max_timeslots = OAKENSHIELD_TDMSW16_MAX_CHANNELS_16M;
        break;
    case TDMSW16_PORT_DS0_DUMP:
        str_info->rate = TDMSW16_8MBPS;
        str_info->max_timeslots = OAKENSHIELD_TDMSW16_MAX_CHANNELS_8M;
        break;

    default:
        TDMSW_ERROR("%s: Error: unknown tdm stream port type %d\n",
                    __FUNCTION__, str_info->porttype);
        break;
    }

    return;
}


static void oak_tdmsw_init_module_info (void)
{
    uint16_t str_no;
    int *point_null = NULL, ix = 0;

    g_tdm_info.tdmsw_base_addr = point_null; /* Oakenshield doesn't use memory mapping */
    g_tdm_info.conn_msg_seqno = 0xFFFF; // first seqno from host should 
                                        // start from 0. 
    g_tdm_info.out_of_seq_cnt = 0;

    g_tdm_info.max_streams = OAKENSHIELD_TDMSW16_MAX_STREAMS;
    g_tdm_info.ds0_dump_stream_num = TDMSW16_DS0_DUMP_STREAM;
    if (is_phoenix()) {
        for (ix = 0; ix < OAKENSHIELD_TDMSW16_MAX_STREAMS; ix++) {
            g_tdm_info.streams[ix].max_num_ports = phoenix_144fxs_max_ports_local[ix];
        }
        g_tdm_info.streams[TDMSW16_DSP0_STREAM].max_num_ports = 0xff;
        g_tdm_info.streams[TDMSW16_DSP1_STREAM].max_num_ports = 0xff;
        g_tdm_info.streams[TDMSW16_DS0_DUMP_STREAM].max_num_ports = PHOENIX_ALL_PORT_NUM_FOR_DS0_DUMP;
     } else {
        g_tdm_info.streams[TDMSW16_MB_CODEC_STREAM].max_num_ports = oak_MB_max_ports_local[sku_id];
        g_tdm_info.streams[TDMSW16_DC_CODEC_STREAM].max_num_ports = oak_DC_max_ports_local[sku_id];
        g_tdm_info.streams[TDMSW16_DSP0_STREAM].max_num_ports = 0xff;
        g_tdm_info.streams[TDMSW16_DS0_DUMP_STREAM].max_num_ports =
        oak_MB_max_ports_local[sku_id] + oak_DC_max_ports_local[sku_id];
    }
    /* fill in all stream information. */
    for (str_no = 0; str_no < OAKENSHIELD_TDMSW16_MAX_STREAMS; str_no++) {
        oak_tdmsw_fill_stream_info(str_no, &g_tdm_info.streams[str_no]);
    }
}



void oak_codec_chip_reset(int reset)
{
    oak_diag_codec_reset(reset);

}




/*
 * Set the TDMSW reset bit. 
 *
 * Parameters:
 *     set - 1 to set the bit to '1' (held in reset)
 *           0 to set the bit to '0' (out of reset)
 * 
 */
static void tdmsw_set_reset_bit (boolean set)
{
    uint32_t val;
    tdm_status_e status;

    status = fpga_spi_indirect_read(TDMSW64_CTL, 1, &val);
    if (status != TDMSW64_SUCCESS) {
        TDMSW_ERROR("%s: Error: TDMSW64_CTL READ failed, error=%d(%s)\n",
                    __FUNCTION__, status, tdmsw_status_string(status));
        return;
    }

    if (set) {
        val |= TDMSW_RST;
    } else {
        val &= ~TDMSW_RST;
    }

    status = fpga_spi_indirect_write(TDMSW64_CTL, 1, val);
    if (status != TDMSW64_SUCCESS) {
        TDMSW_ERROR("%s: Error: TDMSW64_CTL Write failed, error=%d(%s)\n",
                    __FUNCTION__, status, tdmsw_status_string(status));
        return;
    }

    return;
}


/*
 * Held the TDMSW in reset
 */
void tdmsw_reset (void)
{
    tdmsw_set_reset_bit(1);
}

/*
 * Pull TDMSW out of reset.
 */
void tdmsw_unreset (void)
{
    tdmsw_set_reset_bit(0);
}


/*
 * oak_tdmsw16_clock_reset
 *
 * Description:
 *     Set PLL to free run, pull PLL out of reset, and unreset TDMSW.
 *
 */
#define TDMPLL_TIMEOUT 1000
static void oak_tdmsw16_clock_reset (void)
{
    uint32_t reg_data;
    int i;

    /* put TDMPLL out of reset */
    fpga_spi_direct_read(FPGA_GENERAL_MISC_CONTROL, 1, &reg_data);
    fpga_spi_direct_write(FPGA_GENERAL_MISC_CONTROL, 1, reg_data | TDM_PLL_RST);
    fpga_spi_direct_read(FPGA_GENERAL_MISC_CONTROL, 1, &reg_data);
    bsp_debug_printf("\n\r TDMPLL is held in reset! (0x%08X 0x%02x)\n", 
           FPGA_GENERAL_MISC_CONTROL, reg_data);
    msleep(1);

    fpga_spi_direct_read(FPGA_GENERAL_MISC_CONTROL, 1, &reg_data);
    fpga_spi_direct_write(FPGA_GENERAL_MISC_CONTROL, 1, reg_data & ~TDM_PLL_RST);
    fpga_spi_direct_read(FPGA_GENERAL_MISC_CONTROL, 1, &reg_data);
    bsp_debug_printf("\n\r TDMPLL is out of reset! (0x%08X 0x%02x)\n", 
           FPGA_GENERAL_MISC_CONTROL, reg_data);
    fpga_spi_direct_read(FPGA_GENERAL_TDM_PLL_CTRL_STAT_1C, 2, &reg_data);
    bsp_debug_printf("\n\r TDMPLL status: (0x%08X 0x%02x)\n", 
           FPGA_GENERAL_TDM_PLL_CTRL_STAT_1C, reg_data);

    /* config TDMPLL */
    fpga_spi_direct_read(FPGA_GENERAL_TDM_PLL_CTRL_STAT_1C, 1, &reg_data);
    reg_data |= (TDMPLL_PRI_ENA | TDMPLL_PRI_SEL);
    fpga_spi_direct_write(FPGA_GENERAL_TDM_PLL_CTRL_STAT_1C, 1, reg_data);

    fpga_spi_direct_read(FPGA_GENERAL_MISC_CONTROL, 1, &reg_data);
    bsp_debug_printf("\n\r misc_control is: (0x%08X 0x%02x)\n", 
           FPGA_GENERAL_MISC_CONTROL, reg_data);

    fpga_spi_direct_read(FPGA_GENERAL_TDM_PLL_CTRL_STAT_1C, 1, &reg_data);
    bsp_debug_printf("\n\r TDMPLL is configured: (0x%08X 0x%02x)\n", 
           FPGA_GENERAL_TDM_PLL_CTRL_STAT_1C, reg_data);

    fpga_spi_direct_read(FPGA_GENERAL_TDM_PLL_CTRL_STAT_1D, 1, &reg_data);
    bsp_debug_printf("\n\r TDMPLL status: (0x%08X 0x%02x)\n", 
           FPGA_GENERAL_TDM_PLL_CTRL_STAT_1D, reg_data);

    /* make sure TDMPLL is locked */
    /* read back bit 8 (TDMPLL_LOCK) and make sure it's 1 */
    fpga_spi_direct_read(FPGA_GENERAL_TDM_PLL_CTRL_STAT_1D, 1, &reg_data);
    for (i = 0; i < TDMPLL_TIMEOUT; i++) {
        fpga_spi_direct_read(FPGA_GENERAL_TDM_PLL_CTRL_STAT_1D, 1, &reg_data);
        if (reg_data & TDMPLL_LOCK) {
            break;
        }
        msleep(10);
    }
    if (i == TDMPLL_TIMEOUT) {
        TDMSW_ERROR("Warning: TDMPLL not locked - 0x%02x!\n", reg_data);
    } else {
        bsp_debug_printf("\n\r TDMPLL is locked - 0x%02x!\n", reg_data);
    }

    tdmsw_reset(); /* Assert TDMSW reset */
    /* pull TDMSW out of reset */
    tdmsw_unreset();

    bsp_debug_printf("\n\r TDMSW is out of RESET now!\n");
}


/*
 * oakenshield_tdmsw_set_enable_reg_bit
 * 
 * Description:
 *     Helper function to set the bits in enable register. 
 */
static inline void oak_tdmsw_set_enable_reg_bit (uint32_t *reg, uint16_t str_no, 
                                 oak_tdmsw16_on_off_e onoff)
{
    uint32_t tmp_reg = *reg;
    uint16_t index = str_no % TDMSW16_STREAMS_PER_ENABLE_REG;

    if (TDMSW16_DISABLE == onoff) {
        tmp_reg &= ~((uint32_t) 1 << index); 
    } else {
        tmp_reg |= (uint32_t) 1 << index; 
    }

    *reg = tmp_reg;
    return;
}



/*
 * oak_tdmsw_enable_stream
 * 
 * Description:
 *     Helper function to set the bits in enable register. 
 */
static inline void oak_tdmsw_enable_stream (uint16_t str_no, oak_tdmsw16_on_off_e onoff)
{
    uint32_t reg_val;
    uint16_t reg;
    tdm_status_e status;

    if (str_no < TDMSW16_STREAMS_PER_ENABLE_REG) {
        reg = TDMSW64_ENBL_31_00;
    } else {
        reg = TDMSW64_ENBL_63_32;
    }

    /* read the reg value from fpga */
    status = fpga_spi_indirect_read(reg, 4, &reg_val);
    if (status != TDMSW64_SUCCESS) {
        TDMSW_ERROR("%s: Error in reading tdmsw register %d, status=%d(%s)\n",
                    __FUNCTION__, reg, status, tdmsw_status_string(status));
    }

    /* set the corresponding bit */
    oak_tdmsw_set_enable_reg_bit(&reg_val, str_no, onoff);

    /* write back to tdmsw */
    status = fpga_spi_indirect_write(reg, 4, reg_val);
    if (status != TDMSW64_SUCCESS) {
        TDMSW_ERROR("%s: Error in writing tdmsw register %d, status=%d(%s)\n",
                    __FUNCTION__, reg, status, tdmsw_status_string(status));
    }

    return;
}


/*
 * oak_tdmsw_get_rate_reg
 * 
 * Description:
 *     Helper function to find out which rate register for a given
 *     stream number. 
 *
 * Parameters:
 *    str_no - stream number, 0 to 63
 *
 * Returns:
 *    one of the rate register (TDMSW64_RATE_63_48 to TDMSW64_RATE_15_00)
 */
static inline uint16_t oak_tdmsw_get_rate_reg (uint16_t str_no)
{
    uint16_t index = (uint16_t) (str_no / TDMSW16_STREAMS_PER_STREAM_RATE_REG);

    if (index == 0) {
        return(TDMSW64_RATE_15_00);
    } else {
        TDMSW_ERROR("%s: Error: Invalid stream number %d\n", __FUNCTION__,
                    str_no);
    }

    return(0);
}



/*
 * oakenshiled_tdmsw_set_rate_reg_bit
 * 
 * Description:
 *     Helper function to set the bits in rate register. 
 */
static inline void oak_tdmsw_set_rate_reg_bit (uint32_t *reg, uint16_t str_no, 
                               oak_tdmsw16_str_rate_e str_rate)
{
    uint32_t tmp_reg = *reg;
    uint16_t index = str_no % TDMSW16_STREAMS_PER_STREAM_RATE_REG;
    uint32_t mask;
    uint16_t shift;

    shift = index * TDMSW16_STREAM_RATE_BITS_PER_STREAM;
    mask = (uint32_t) TDMSW16_STREAM_RATE_MASK << shift;

    /* clear the bits first */
    tmp_reg &= ~mask;

    switch (str_rate) {
    case TDMSW16_2MBPS:
        tmp_reg |= (uint32_t) TDMSW16_STREAM_RATE_2MBPS << shift;
        break;

    case TDMSW16_8MBPS:
        tmp_reg |= (uint32_t) TDMSW16_STREAM_RATE_8MBPS << shift;
        break;

    case TDMSW16_16MBPS:
        tmp_reg |= (uint32_t) TDMSW16_STREAM_RATE_16MBPS << shift;
        break;

    default:
        TDMSW_ERROR("%s: Error: unsupported stream rate %d\n", __FUNCTION__,
                    str_rate);
        break;
    }

    *reg = tmp_reg;

    return;

}



/*
 * Name: oak_tdmsw16_set_str_rate
 *
 * Description:
 *      Sets the stream rate 
 */
static tdm_status_e oak_tdmsw16_set_str_rate (uint16_t str_no,
                                                 oak_tdmsw16_str_rate_e str_rate)
{
    uint16_t rate_reg;
    uint32_t reg_val;
    tdm_status_e status;

    /* find the correspond register */
    rate_reg = oak_tdmsw_get_rate_reg(str_no);
    if (!rate_reg) {
        return(TDMSW64_INVALID_STREAM_NUM);
    }

    /* read the current reg value from tdmsw */
    status = fpga_spi_indirect_read(rate_reg, 4, &reg_val);
    if (status != TDMSW64_SUCCESS) {
        return(status);
    }

    /* set the reg field */
    oak_tdmsw_set_rate_reg_bit(&reg_val, str_no, str_rate);

    /* write back to tdmsw */
    status = fpga_spi_indirect_write(rate_reg, 4, reg_val);

    return(status);
}



static tdm_status_e  oak_tdmsw16_reset_stream (uint16_t str_no)
{
    tdmsw_stream_info_t* str_info = &g_tdm_info.streams[str_no];
    tdm_status_e status;
    uint16_t timeslot;
    uint32_t cm_offset;

    if (str_info->porttype == TDMSW16_PORT_NC) {
        return(TDMSW64_SUCCESS);
    }

    /* set to correct rate */
    status = oak_tdmsw16_set_str_rate(str_no, str_info->rate);
    if (status != TDMSW64_SUCCESS) {
        TDMSW_ERROR("%s: Error in seting rate for stream %d, status=%d(%s)\n",
                    __FUNCTION__, str_no, status,
                    tdmsw_status_string(status));
        return(status);
    }
    /* clear all connection memory content */
    for (timeslot = 0; timeslot < str_info->max_timeslots; timeslot++) {
        cm_offset = tdm_get_cid_address_offset(str_info->rate, str_no, 
                                               timeslot);
        status = fpga_spi_indirect_write(cm_offset, 4, CM_PASSWORD);
        if (status != TDMSW64_SUCCESS) {
            TDMSW_ERROR("%s: Error in clearing cm, str=%d, ts=%d, " \
                        "status=%d(%s)\n",
                        __FUNCTION__, str_no, timeslot, status,
                        tdmsw_status_string(status));
            return(status);
        }
    }
    /* enable the stream */
    oak_tdmsw_enable_stream(str_no, TDMSW16_ENABLE);

    return(TDMSW64_SUCCESS);
}



/*
 * oakenshield_tdmsw16_cold_reset_init 
 *
 * Description:
 *
 *    Reset the Fortitude TDMSW and then initialize the connection memory.
 */
static tdm_status_e oak_tdmsw16_cold_reset_init (void)
{
    uint16_t str_no;
    tdm_status_e status;

    /* take PLL and TDMSW out of reset */
    oak_tdmsw16_clock_reset();

    /* clear enable registers (disable all streams) */
    status = fpga_spi_indirect_write(TDMSW64_ENBL_31_00, 1, 0);
    if (status != TDMSW64_SUCCESS) {
        TDMSW_ERROR("%s: Error in clearing TDMSW64_ENBL_31_00, code=%d(%s)",
                    __FUNCTION__, status, tdmsw_status_string(status));
        return(status);
    }

    /* clear loopback registers */
    status = fpga_spi_indirect_write(TDMSW64_LPBK_31_00, 1, 0);
    if (status != TDMSW64_SUCCESS) {
        TDMSW_ERROR("%s: Error in clearing TDMSW64_LPBK_31_00, code=%d(%s)",
                    __FUNCTION__, status, tdmsw_status_string(status));
        return(status);
    }

    /* clear rate registers. This puts all stream into 2M. Will set
     * to correct rate later.
     */
    status = fpga_spi_indirect_write(TDMSW64_RATE_15_00, 1, 0);
    if (status != TDMSW64_SUCCESS) {
        TDMSW_ERROR("%s: Error in clearing TDMSW64_RATE_15_00, code=%d(%s)",
                    __FUNCTION__, status, tdmsw_status_string(status));
        return(status);
    }

    /* reset all streams */
    for (str_no = 0; str_no < g_tdm_info.max_streams; str_no++) {
        status = oak_tdmsw16_reset_stream(str_no);
        if (status != TDMSW64_SUCCESS) {
            TDMSW_ERROR("%s: Error in reseting stream %d, error=%d(%s)\n",
                        __FUNCTION__, str_no, status, 
                        tdmsw_status_string(status));
            return(status);
        }
    }

    return(TDMSW64_SUCCESS);
}



/*
 * dymamo_tdm_init_hardware
 *
 * Description:
 *     Initialize the TDM hardware in the FPGA
 *
 */
static void oak_tdm_init_hardware (void)
{
    tdm_status_e status;

    /* put codec chips in reset */
    oak_codec_chip_reset(1);

    /* cold reset tdm switch and enable streams that needs to be enable */
    status = oak_tdmsw16_cold_reset_init();
    if (status != TDMSW64_SUCCESS) {
        TDMSW_ERROR("%s: Error in reseting tdmsw, error=%d(%s)\n",
                    __FUNCTION__, status, tdmsw_status_string(status));
        return;
    }

    /* take codec chips out of reset */
    oak_codec_chip_reset(0);
}




/* 
 * oak_module_tdm_init
 * 
 * Description:
 *    This is called from oak main program on the module. It does the 
 *    following -
 *              1) Analysis hardware (TDMSW16 vs TDMSW64).
 *              2) Set up structure oak_tdm_info_t for this SPA 
 *                 and fill in info like tdmsw_base_addr, TDMSW64/16 specific 
 *                 stream info, etc.
 *              3) Call oakenshield_tdm_init_hardware to cold reset/init 
 *                 TDMSW64/16 and enable all TDM streams to proper rate.
 *              4) Call initialize_tdm_app to init the function table for the 
 *                 handers of TDM related control message 
 *              5) Call tdm_register_app_handler(opcode, func) for all 
 *                 opcodes for TDM related control messages. 
 * Parameters:
 *    None
 */
void oak_module_tdm_init (void)
{

    /* Set up structure oakenshield_tdm_info_t */
    oak_tdmsw_init_module_info();

    oak_tdm_init_hardware();

    g_ds0_dump_info.active = 0;
    g_ds0_dump_info.local_file_rx = NULL;
    g_ds0_dump_info.local_file_tx = NULL;
    g_ds0_dump_info.rx_buff = NULL;
    g_ds0_dump_info.tx_buff = NULL;
}


/******** History ********
$Log: tdm.c,v $
Revision 1.3  2021/04/15 00:53:07  achiu2
[PRRQ:CSCvx56970-2]Phoenix code review for ER

Revision 1.2  2017/07/28 07:58:51  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.1.2.1  2017/06/29 08:14:39  harrchan
Initial commit code for Oakenshield

Revision 1.1.2.4  2017/01/17 05:07:06  olin2
Clean up debug code

Revision 1.1.2.3  2017/01/05 06:06:34  olin2
Support FXS Ring and Calibration

Revision 1.1.2.2  2016/12/23 06:56:04  olin2
Support FXS/FXO loopback test

Revision 1.1.2.1  2016/12/14 04:57:38  olin2
Initial commit code for Oakenshield




$Endlog$
*/
