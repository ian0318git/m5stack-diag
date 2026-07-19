/* $Id: prince_scc_struct.h,v 1.1 2013/04/19 07:17:52 xiaoyizh Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/prince/prince_scc_struct.h,v $
 *------------------------------------------------------------------
 * prince_scc_struct.h 
 *      prince scc structures and extern prototypes.
 *
 * Dec 2012, Xiaoying Zhang
 *
 * Copyright (c) 2012-2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#ifndef __PRINCE_SCC_STRUCT__
#define __PRINCE_SCC_STRUCT__

/*
 *  DMA Buffer Descriptors
 */
typedef struct prince_dma_bd_t_ {
    volatile ushort byte_count;
    volatile ushort command_status;
    volatile ulong  buff_ptr;
} __attribute__ ((packed)) prince_dma_bd_t;

/*
 *  Serial Data Structure.
 */
typedef struct prince_serial_ds_t_ {
    prince_proto_regs_t   *proto_regs_addr;      
    prince_dma_regs_t     *dma_tx_regs_addr; 
    prince_dma_regs_t     *dma_rx_regs_addr; 
    prince_ppp_regs_t     *ppp_tx_regs_addr;
    prince_ppp_regs_t     *ppp_rx_regs_addr;  
    prince_bcc_regs_t     *bcc_tx_regs_addr;
    prince_bcc_regs_t     *bcc_rx_regs_addr;  
    prince_serial_itf_t   *serial_itf_addr; 
    prince_cntrl_1_regs_t *cntrl_1_regs_addr;   
    prince_cntrl_2_regs_t *cntrl_2_regs_addr;  
    ushort   port_num;               /* port number                 */
    ushort   org_port_num;           /* original port number        */
    ushort   brg_num;                /* BRG Rate Number             */
    uint     divider;                /* divider                     */  
    uchar    clk_src;                /* Brg clock source            */
    uchar    speed_idx;              /* speed index                 */
    uchar    lpbk_mode;              /* Loopback Mode               */
    ushort    protocol;               /* Protocol                    */
    ulong    ctrl_id;                /* Device vendor-id            */
    ushort   num_buff;               /* Number of buffers           */
    ushort   buff_size;              /* Size of Buffers             */
    ulong    last_rx_desc_addr;      /* Address of last Rx desc     */
    ulong    tx_desc_addr_virt;      /* Virtual Address of Tx Ring  */
    ulong    rx_desc_addr_virt;      /* Virtual Address of Rx Ring  */
    ulong    tx_buf_addr_virt;       /* Virtual Address of Tx Buffer */
    ulong    rx_buf_addr_virt;       /* Virtual Address of Rx Buffer */
    ulong    tx_desc_addr_phys;      /* Physical Address of Tx Ring  */
    ulong    rx_desc_addr_phys;      /* Physical Address of Rx Ring  */
    ulong    tx_buf_addr_phys;       /* Physical Address of Tx Buffer */
    ulong    rx_buf_addr_phys;       /* Physical Address of Rx Buffer */
    uint     baudrate;               /* baud rate                   */
    uchar    crc_rx_buf_opt;         /* need 2 crc bytes in rx buf  */ 
    uchar    crc_enb;                /* crc default enb             */
    uchar    run_mode;               /* run mode                    */
    ulong    base_addr;
} prince_serial_ds_t;

typedef struct prince_baud_t_ {
    uint baudrate_code;         /* one of the sync/async enums */
    uint desired_baud;        
    uint baudrate;              /* the actual baudrate */
    uint divider;               /* clock divider */
    ushort clk_src;             /* clock source */
} prince_baud_t;

typedef struct prince_pll_t_ {
    uint baudrate;              /* the actual baudrate */
    uint pre_divider;           /* clock pre-divider */
    uint post_divider;          /* clock post-divider */
} prince_pll_t;

/*===================================================================*
 *                    Function Prototypes                            *
 *===================================================================*/
extern int get_scc_channel_num();

extern int prince_chan_lpbk_test (uchar run_mode, ulong scc_base_addr,
                                ushort speed, uchar clk_src, 
                                uchar lpbk_mode, uchar protocol,
                                prince_serial_ds_t *s_ds, int priority);
extern void prince_config_serial_ds (uchar run_mode, ulong scc_base_addr, ushort speed_idx,
                                uchar clk_src, uchar lpbk_mode, uchar protocol,
                                prince_serial_ds_t *s_ds, int priority,
                                ulong *des_baud, ushort *clk_src_bits);
extern void prince_init_serial_ds(prince_serial_ds_t *s_ds);
extern void prince_cleanup_mode_bits(prince_serial_ds_t *s_ds);

extern int prince_init_tx_dma_ring(uchar run_mode, ulong scc_base_addr,
                                 prince_serial_ds_t *s_ds);
extern int prince_init_rx_dma_ring(uchar run_mode, ulong scc_base_addr,
                                 prince_serial_ds_t *s_ds);
extern int prince_lpbk_test(ulong scc_base_addr, 
                             prince_serial_ds_t *s_ds);
extern void prince_init_tx_dma_buffers(prince_serial_ds_t *s_ds, uchar data,
                                     uchar pattern);
extern void prince_init_rx_dma_buffers(prince_serial_ds_t *s_ds);
extern ushort prince_start_dma_tx(prince_serial_ds_t *s_ds, int priority);
extern void prince_setup_dma_reg(prince_serial_ds_t *s_ds);
extern ushort prince_check_dma_received_data (prince_serial_ds_t *s_ds);
extern int prince_form_name_buf(char *tmp_name_buf, prince_serial_ds_t *s_ds, int priority);
extern void prince_trap (prince_serial_ds_t *s_ds, int priority);
extern void prince_cleanup_dma_ring(ulong scc_base_addr, 
                                  prince_serial_ds_t *s_ds);
extern void prince_cleanup_serial(prince_serial_ds_t *s_ds, int priority);

#endif /* end __PRINCE_SCC_STRUCT__ */


/******** History ********
$Log: prince_scc_struct.h,v $
Revision 1.1  2013/04/19 07:17:52  xiaoyizh


Initial check in for Prince NIM.


$Endlog$
*/
