/* $Id: prince_extern.h,v 1.1 2013/04/19 07:17:51 xiaoyizh Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/prince/prince_extern.h,v $ 
 *------------------------------------------------------------------
 * prince_def.h 
 *      Prince projects - NIM 1T/2T/4T extern declarations..
 *
 * Xiaoying Zhang -- Nov. 2012
 *
 * Copyright (c) 2012-2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#ifndef __PRINCE_EXTERN__
#define __PRINCE_EXTERN__

//extern int simpsons_fpga_download(ulong);
extern int hwic_native_reg_test(void *);

/* The following are menu items */
//extern int ngwic_prince_chan_lpbk_test(prince_chan_params_t * prince_iface);
extern int ngwic_prince_chan_lpbk_test(int channel);
extern int prince_reg_test(ulong scc_base_addr);
extern int prince_util_lpbk_test(ulong scc_base_addr);
extern int prince_pll_test(ulong scc_base_addr);
extern int prince_tdm_test(ulong scc_base_addr);
extern int prince_interrupt_test(ulong scc_base_addr);
extern int prince_led_test(ulong scc_base_addr);
extern int prince_cable_id_test(ulong scc_base_addr);
extern int prince_reset_test(ulong scc_base_addr);
extern int prince_disp_reg_test(ulong scc_base_addr);
extern int prince_alter_reg_test(ulong scc_base_addr);
extern int prince_disp_mem_test(ulong scc_base_addr);
extern int prince_alter_mem_test(ulong scc_base_addr);
extern int prince_mem_test(ulong scc_base_addr);
extern int prince_ddr_test(ulong scc_base_addr);
extern int prince_all_chan_test(ulong scc_base_addr);
extern int prince_all_pll_test(ulong scc_base_addr);

/* The follwing are support functions and data */ 
extern prince_serial_ds_t prince_serial_ds[/*NUM_HWIC_SLOTS*/4][NUM_HWIC_CH]; 
extern prince_pll_t prince_pll_64k[]; 
extern prince_pll_t prince_pll_56k[]; 
extern prince_pll_t prince_pll_8as_4as[]; 
extern uint prince_freq_intr_cnt[/*NUM_HWIC_SLOTS*/4] ; 
extern uchar plat_margin; 
extern int prince_chan_lpbk_test (uchar run_mode, ulong scc_base_addr,
                                ushort speed, uchar clk_src, 
                                uchar lpbk_mode, uchar protocol,
                                prince_serial_ds_t *s_ds);
extern int get_prince_chan_num(ulong scc_base_addr);
extern int get_prince_adj_chan_num(ulong scc_base_addr, int chan_num);
extern int prince_cable_id_check(ulong scc_base_addr);
extern int prince_freq_intr_setup(ulong scc_base_addr, int chan, 
                                ushort type);
extern int prince_freq_intr_cleanup(ulong scc_base_addr, int chan,
                                  ushort type);
extern void prince_cleanup_dma_ring(ulong scc_base_addr, 
                                  prince_serial_ds_t *s_ds);
extern void prince_cleanup_serial(prince_serial_ds_t *s_ds);
extern int prince_check_freq_count(prince_serial_ds_t *s_ds, uint baudrate,
                                 ulong scc_base_addr, uint freq_count,
                                 uint loop_count_this,
                                 uint loop_count_last, uint retry_count);
extern void prince_init_serial_ds( prince_serial_ds_t *s_ds);
extern void prince_display_dma_regs(ulong scc_base_addr, uchar option,
                                 uchar disp_opt, uchar chan);
extern void prince_display_native_regs(ulong scc_base_addr); 
extern void prince_display_cntrl_1_regs(ulong scc_base_addr);
extern void prince_display_cntrl_2_regs(ulong scc_base_addr);
extern void prince_display_serial_itf_regs(ulong scc_base_addr, uchar
                                         disp_opt, uchar chan);
extern void prince_display_proto_regs(ulong scc_base_addr, 
                                    uchar disp_opt, uchar chan);
extern void prince_cleanup_mode_bits(prince_serial_ds_t *s_ds);
extern int prince_read_freq_margin(void);
extern void dprince_dump_proto_regs(void);
extern void dprince_dump_serial_itf_regs(void);
extern void dprince_dump_native_regs(void);
extern void dprince_dump_cntrl_1_regs(void);
extern void dprince_dump_cntrl_2_regs(void);

/* prince_download */
/* external */
extern unsigned char prince_firmware[];
extern unsigned long prince_firmware_size;
extern unsigned char prince_firmware_sync[];
extern unsigned long prince_firmware_size_sync;

#endif /* end __PRINCE_EXTERN__ */


/******** History ******** 
$Log: prince_extern.h,v $
Revision 1.1  2013/04/19 07:17:51  xiaoyizh


Initial check in for Prince NIM.


$Endlog$
*/

