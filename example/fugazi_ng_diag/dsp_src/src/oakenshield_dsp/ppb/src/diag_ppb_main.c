/* $Id: diag_ppb_main.c,v 1.5 2021/04/15 00:53:07 achiu2 Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/ppb/src/diag_ppb_main.c,v $
 *------------------------------------------------------------------
 * diag_ppb_main.c
 *      ARM diagnostic entry point
 *
 * Mar 2012, Smita Rane
 *
 * Copyright (c) 2012-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include "stdint.h"
#include "string.h"
#include "stdio.h"
#include "types.h"
#if defined(USE_AG_MG_REGS)
#include "ag_mg.h"
#include "ag_mg_regs.h"
#include "ag_mg_dev.h"
#elif defined(USE_LSI_SP27XX_REGS)
#include "lsi_sp27xx_reg.h"
#endif
#include "libeth.h"
#include "libgeneric.h"
#include "libuart.h"
#include "libgpio.h"
#include "common.h"
#include "diag_common.h"
#include "diag_ppb.h"
#include "irom_timer.h"
#include "uart.h"
#include "ssp.h"
#include "debug_console.h"
#include "error.h"
#include "diag_dss.h"
#include "section_table.h"
#include "arm_common.h"
#include "libtdm.h"
#include "libavsc.h"
#include "tdm_utils.h"
#include "diag_fpga.h"
#include "fxs_test.h"
#include "tstcodec_si3050.h"



/* Extern Function declarations */
extern int  diag_menu(void);
extern int  ethernet_test(uint32_t, int, int);
extern int  tdm_lpbk(int);
extern int  tdm_ext_lpbk(int);
extern int  tdm_int_lpbk(void);
extern void _start(void);
extern void spi_init(void);
extern void spi_init_fpga(void);
extern uint32_t test_mem(void);
extern uchar get_oak_id(void);
extern int read_fpga_dir_reg (void); 
extern int write_fpga_dir_reg (void); 
extern int read_fpga_indir_reg (void); 
extern int write_fpga_indir_reg (void);
extern int codec_si32261_set_stop_ring(int, int);
int dsp_test_intf_sync_sig(int, int);
int dsp_break_whileloop(void);
int arm11_cpu1_boot_test(void);
int dss_core_sanity(int dss_core);
static int sp_setdac_high(int);
static int sp_setdac_low(int);
static int sp_setdac_norm(int);
static int sp_show1DOT5dac(void);
static int sp_show3DOT3dac(void);
static void cmd_parse(const char *cmd, uint32_t len);
static int disp_ver(int host_cmd);
int get_info(int host_cmd);
void timer_delay (uint32_t onems); 
uchar sku_id;
//static uint32_t uart_test(void);

/* mem display related globals */
static uint32_t LastArg1 ;
static uint32_t LastArg2 ;
static uint32_t LastArg3 ;

static char verstring[] =  "version "VER"."BUILD", built on "__DATE__":"__TIME__;

/* Global variables */
uint8_t src_macaddr[6];
uint8_t dest_macaddr[6];
int dsp_tests_use_enet;   /* Use ethernet command/response for tests */

/* Ethernet debug */
extern TXD_BQUE TxBuffDes1[BuffTxElements];
extern RX_BQUE RxBuffDes1[BuffRxElements]; 
extern uint8_t out_frame[3][2048];
extern uint8_t in_frame[2][1024*2];
extern unsigned long
gethex_answer(char *msgstr, unsigned long currentval, unsigned long min,
          unsigned long max);


/* identify device */
dsp_type_t dsp_device;
dsp_type_t *dsp_device_p = &dsp_device;
/* identify DSP */
dspid_t	dspid;
dspid_t *dspid_p = &dspid;
/* host interface packet */
dspif_ether_t hostif_msg;
dspif_ether_t *hostif_msg_p = &hostif_msg;

avscCLparam_t avscParam;

/* for booloader */
arm_common_t dss_mgr_checksum_table __attribute__ ((section (".chksumtab")));
volatile uint32_t app_resetMsgMem[255] SECTION("RESET_MSG");

arm_common_t dss_mgr_checksum_table = {
    {CHECKSUM_MAGIC_INVALID,	/* checksum magic */
    {{0, 0, 0, 0}},},		/* section_table_entry */
    {0, 0, 0, 0, 0, 0},         /* mac_da */
    {0, 0, 0, 0, 0, 0},         /* mac_sa */
    0,                          /* vlan_tag */
    0                           /* device_id */
};

/* use the location just for magic number - cores are ready */
volatile uint32_t   *dss0_if = (uint32_t *)0xC02FE000;
volatile uint32_t   *dss1_if = (uint32_t *)0xC02FE200;
volatile uint32_t   *dss2_if = (uint32_t *)0xC02FE400;
volatile uint32_t   *dss3_if = (uint32_t *)0xC02FE600;
volatile uint32_t   *cpu1_if = (uint32_t *)0xC02FD600;

volatile uint32_t *dss0_ppb_if;
volatile uint32_t *dss1_ppb_if;
volatile uint32_t *dss2_ppb_if;
volatile uint32_t *dss3_ppb_if;
volatile uint32_t *arm_cpu1_if;

/* use the location for command interface between PPB & Cores */
volatile dspif_info_t *ppb_dss0_if = (volatile dspif_info_t *)0xC05FF280;
volatile dspif_info_t *ppb_dss1_if = (volatile dspif_info_t *)0xC05FF480;
volatile dspif_info_t *ppb_dss2_if = (volatile dspif_info_t *)0xC05FF680;
volatile dspif_info_t *ppb_dss3_if = (volatile dspif_info_t *)0xC05FF880;

volatile bootup_msg_t *ppb_bootmsg_addr = (volatile bootup_msg_t *)0xC0580000;
volatile host_comm_status_t *cmd_status_addr = (volatile host_comm_status_t *)0xC0500000;

/* To share uart between DSS and PPB */
volatile uint32_t *uart_mem = (uint32_t *)0xc02FF800;
volatile uint32_t *uart_getlock = (uint32_t *)0xc02FF840;

/* PPB & Cores interface */
volatile dspif_info_t hhd_if;
volatile dspif_info_t *hd_if;
/* core id */
uint8_t core_id;
volatile float dsp_freq_value = 300;

unsigned char pid[128];
int dc_slot = 0;

#define DEBUG_CONSOLE_INPUT 1
#define DEBUG 1
/* define register locations */
#if defined(USE_AG_MG_REGS)
volatile ag_mg_regs_car_reg_s *car_regs_ptr =
    (volatile ag_mg_regs_car_reg_s *)(AG_MG_REGS_CAR_BASE);
#else /*  defined(USE_LSI_SP27XX_REGS)  */
volatile lsi_sp27xx_car_reg_s *car_regs_ptr =
    (volatile lsi_sp27xx_car_reg_s *)(LSI_SP27XX_CAR_BASE);
#endif

/* extern */
extern unsigned char __BSS_START;
extern unsigned char __BSS_END;


/**********************************************************************
 *
 * Function: read_ppb_gpio_utility
 *
 * Read PPB GPIO data
 *
 * Input : None
 *
 * Returns: None
 *
 *
 **********************************************************************
 */
void read_ppb_gpio_utility (void)
{
    int gpio_data;

    gpio_data = sp_GetGPIOData(0xFF);

    bsp_debug_printf("\n\r PPB GPIO: %x \n", gpio_data);

}

/**********************************************************************
 *
 * Function: msleep
 *
 * delay in number of ms
 *
 * Input : delay millisecond time
 *
 * Returns:
 *
 *
 **********************************************************************
 */
void msleep(uint32 sleep_time)
{
    lsi_mg_delay(sleep_time * 1000);
}

/**********************************************************************
 *
 * Function: usleep
 *
 * delay in number of us
 *
 * Input : delay microsecond time
 *
 * Returns:
 *
 *
 **********************************************************************
 */
void usleep(uint32 sleep_time)
{
    lsi_mg_delay(sleep_time);
}

/***********************************************************************
 *
 * Function: timer_delay
 *
 * Description:  delay in number of ms
 *
 * Input : timer out value in number of 1ms
 *
 * Returns: none
 *
 **********************************************************************
 */
void timer_delay (uint32_t onems) 
{
	uint32_t timer = 0;
	uint32_t timer_value = 0;

	/* calculate timer value:
	 * PPB SYSCLK = DSPCLK/2
	 * To get 1ms = (1/SYSCLK) * timer_value
	 */
	timer_value = (dsp_freq_value / 2) * 1000;

	/* Init the timer first */
	AG_MG_IROM_TIMER_REG(AG_MG_IROM_TIMER0, AG_MG_IROM_TIMERCONTROL) = 
            AG_MG_IROM_TIMERCONTROL_DFLT;

	/* 
	 * Load the timeout value to the timer value register - 
	 * @50MHz generate 500ms timeout 
	 * (1/50MHz * (25000 * 1000))  = 500ms
	 */
	timer = onems * timer_value;
	AG_MG_IROM_TIMER_REG(AG_MG_IROM_TIMER0, AG_MG_IROM_TIMERLOAD) = timer;
	/* Enable the timer */
	AG_MG_IROM_TIMER_REG(AG_MG_IROM_TIMER0, AG_MG_IROM_TIMERCONTROL) |= 
            AG_MG_IROM_TIMERCONTROL_ENABLE;
 
	/* 
	** Wait for the timeout
	*/
	while (timer != 0)
	{
        timer = AG_MG_IROM_TIMER_REG(AG_MG_IROM_TIMER0, AG_MG_IROM_TIMERVAL); 
        uart_puts(".");
	}
}

/***********************************************************************
 *
 * Function: init_id
 *
 * Description: identify the chip 
 *
 * Input : none
 *
 * Returns: none 
 *
 **********************************************************************
 */
void init_id(void)
{
    /* store in DSP type struct */
#ifdef BOOT_DEBUG
    uart_puts("\r\n	START init_id ");
    uart_puts("\r\n	car_regs_ptr (in hex) = ");
    uart_put_long((uint32_t)car_regs_ptr, 16);
#endif
    dsp_device_p->device_type = sp_check27xxDevId();
    dsp_device_p->device_freq = DSP_750MHZ;

    /* set the freq */
    dsp_freq_value = 750;
    
    /* For Oakenshield the chip_id, module_id logic is not correct, there is only
       one SP2704 on the Oakenshield and mostly MAC address is used for 
       identification but leaving the code as is if needed for future */

    /* now, identify which DSP - this matters the MAC, the TDM */
    /*  DSP CHIP_ID[0] : 0 chip 0, 1 chip 1 */
    dsp_device_p->chip_id = sp_check27xxChipId();

    /* init core id to PFUSE123 (SP2702/SP2702) */
    dsp_device_p->core_id = sp_check27xxPFUSE123();

    /* now set up dspid as src id in the message */
    /* DSP CHIP_ID[2:1] has slot ID: 0, 1, 2, 3 */
    dspid_p->slot_id = ((car_regs_ptr->chipid.fields.chipid & (0x6)) >> 1);
    /*
     * The PLD Module ID Register (HCNTL = 0x08) bit 4 to bit 0 are connected
     * to DSP's GPIO[6:2]. Together with SLOT_ID[2:0], these bits can be used 
     * to identify NGSM in a next generation Network Module.
     */
    dspid_p->module_id = 0xff; /* future module use */
    /* For each PVDM-NG, there are up to two DSPs. The CHIP_ID[0] pin 
     * on the SP260x identifies between these two.
     */
    dspid_p->dsp_id = dsp_device_p->chip_id;
    /*
     * One of the DSS Control Registers, IDCODE (DSS Identification Register)
     * provides DSS core ID. Bits10-7 (DSSID) of IDCODE are 
     * hardwired physical ID for DSS core:
     * DSS0: DSSID[10-7] = 0000
     * DSS1: DSSID[10-7] = 0001
     * DSS2: DSSID[10-7] = 0010
     */
    dspid_p->core_id = 0xff; /* fill in later to identify core that run test */

    dss0_ppb_if = (uint32_t *)0xC02FE800;
    dss1_ppb_if = (uint32_t *)0xC02FEC00;
    dss2_ppb_if = (uint32_t *)0xC02FF000;
    dss3_ppb_if = (uint32_t *)0xC02FF400;
    arm_cpu1_if = (uint32_t *)0xC02FD600;

    *dss0_ppb_if = 0;
    *dss1_ppb_if = 0;
    *dss2_ppb_if = 0;
    *dss3_ppb_if = 0;
    *arm_cpu1_if = 0;
    hd_if = (dspif_info_t *)&hhd_if;
}

/***********************************************************************
 *
 * Function: setup_dc_info
 *
 * Description:  Extract DC info from the packet
 *
 * Input : packet received from Host
 *  
 * Returns: PASSED
 *
 **********************************************************************
 */
uint32_t setup_dc_info (dspif_ether_t *pkt_ptr)
{
#ifdef DEBUG
    char mem_disp[40];
#endif
    uint i;

#ifdef DEBUG
    sprintf(mem_disp, "0x%x 1510", (unsigned int)pkt_ptr);    
    bsp_debug_printf("\r\n mem_disp = %s ", mem_disp);
    do_mem_md(mem_disp);
    bsp_debug_printf("\r\n 0x%x bufmsg addr  : contents %d", 
        (unsigned int)&(pkt_ptr->dspif_info.bufmsg[0]), 
        pkt_ptr->dspif_info.bufmsg[0]);
#endif
    for (i=0; i<50; i++) {
        pid[i] = pkt_ptr->dspif_info.bufmsg[i];
#ifdef DEBUG
    bsp_debug_printf(" %d. pid = %c, bufmsg = %c ", i, pid[i], 
        pkt_ptr->dspif_info.bufmsg[i]);
#endif
    }
    bsp_debug_printf("\r\n PID of Oakenshield is %s", pid);
    dc_slot = 0;

    bsp_debug_printf("\r\n Oakenshield is SM_MODULE \r\n");
    bsp_debug_printf("\r\n Oakenshield dest_id = 0x%x\n",(int)pkt_ptr->dspif_hdr.dest_id);
    dc_slot = (int)SWAP32(pkt_ptr->dspif_hdr.dest_id);
        
    bsp_debug_printf("\r\n Oakenshield platform ngsm_num = %d\n", dc_slot);
    dsp_tests_use_enet = (int)pkt_ptr->dspif_info.errmsg[0];
    menu_display = (int)pkt_ptr->dspif_info.errmsg[1];
    bsp_debug_printf("\r\n Use ethernet interface %d\r\n", dsp_tests_use_enet);

    return (PASSED);
}

/***********************************************************************
 *
 * Function: ppb_con
 *
 * Description: ppb ARM console commands 
 *
 * Input : none 
 *
 * Returns: PASSED
 *
 **********************************************************************
 */
/* Access to NGSM PPB console for memory display */
uint32_t ppb_con (void)
{
    char     cbuf[CBUF_SIZE], ch;
    uint32_t num_char, count;

    num_char = 0;
    count = 0x30;

    uart_puts("\r\nppb_diags> ");
    while (1) {
        if ((ch = uart_getch())) {

             if (ch == EXIT_CHAR) {
                return 0;
            }
            while (uart_putch(ch) == 0)
                ;
            if ((ch != 0x7f) && (ch != 0x8)) {   /* DEL */
                cbuf[num_char] = ch;
                if (ch == '\r') {
                    cbuf[num_char] = '\0';
                    cmd_parse(cbuf, num_char);
                    (void)uart_puts("\r\nppb_diags> ");
                    memset(cbuf, 0, num_char);
                    num_char = 0;
                } else if (num_char < (CBUF_SIZE - 1)) {
                    num_char++;
                } else {
                    memset(cbuf, 0, num_char);
                    num_char = 0;
                    (void)uart_puts("\r\nnppb_diags> ");
                }
            } else if (num_char > 0) {
                num_char--;
                cbuf[num_char] = '\0';
                while (uart_putch(0x8) == 0)
                    ;
                while (uart_putch(' ') == 0)
                    ;
                while (uart_putch(0x8) == 0)
                    ;
            }
        }
    }
}

int dsp_test_debug (void)
{
    int count; 
    /*
    1. find which test to debug or whichmemory/size from the packet
    2. Find how many 1400 byte pkts will be needed to send the register/memory info
    3. loop send debug info pkt - no of pkts
    4. wait for ist pkt req
    5. send pkt (host willcheck if last pkt)
    6. sent last pkt return to loop
    */
    count = 0x500;
    while (count--) {
        lsi_mg_delay(WAIT_TIME); /* need to adjust number later */
    } /* wait for debugging or host reset */
    uart_puts("\r\n NOW SENDING MEM DUMP\n");
    if (send_host_memmsg()) {
        uart_puts("\r\n Failed to send MEM message\n");
        sprintf((char *)&(hd_if->errmsg), "Failed to send MEM message");
        uart_puts((char *)hd_if->errmsg);
    } else {
        uart_puts("\r\n Sent host mem msg \n");
        uart_puts("\r\n Now Waiting to receive command packet from Host\n");
    }
    return (PASSED);
}

/***********************************************************************
 *
 * Function: proc_host_msg
 *
 * Description: Process message from the host 
 *
 * Input : host interface packet
 *
 * Returns: PASSED/FAILED
 *
 * NOTE: it is important of accessing to the interface space 
 * since core is constantly reading the same location 
 * and will execute the test and come back with result
 * quickly!
 **********************************************************************
 */
uint32_t proc_host_msg(dspif_ether_t * hostif_msg_p)
{
    uint32_t    op_type, lnerrors = 0;
    uint16_t	result;

    op_type = hostif_msg_p->dspif_hdr.op_type;

    // SR ?? add later StarProPPB_UTILS_memcpy8((uint8_t *)hd_if->bufmsg,
    // SR ?? add later 				 (const uint8_t *)&(hostif_msg_p->dspif_info.bufmsg[0]),
    // SR ?? add later 		 128);
    // SR ?? add later StarProPPB_UTILS_memcpy8((uint8_t *)hd_if->errmsg,
    // SR ?? add later 		 (const uint8_t *)&(hostif_msg_p->dspif_info.errmsg[0]),
    // SR ?? add later 		 128);
    memcpy((uint8_t*)hd_if, &(hostif_msg_p->dspif_info), sizeof(dspif_info_t));
    /* 
     * if doing ge loopback, can be done in ARM 
     * once reganized it's ge loopback, wait for
     * test data, copy to sending msg buf and
     * send it back, until see test done message
     */
    uart_puts("\r\n In op_type() op_type code =  ");
    uart_put_long(op_type, 10);
    uart_puts("\r\n select field = ");
    uart_put_long((int)hd_if->select, 10);
    cmd_status_addr->prev_command_recv = cmd_status_addr->cur_command_recv;
    cmd_status_addr->prev_status = cmd_status_addr->cur_status;
    if ((op_type == OP_TEST_REQUEST) && ((hd_if->select == SELECT_GE0_LPBK) |
        (hd_if->select == SELECT_GE1_LPBK))) {
        cmd_status_addr->cur_command_recv = hd_if->select;
        if (hd_if->select == SELECT_GE0_LPBK)  {
            uart_puts("\r\n in GE0 External Loopback Test\n");
            result = wait_lpbk_msg(EMAC0);
        } else {
            uart_puts("\r\n in GE1 External Loopback Test\n");
            ethernet_test(0,0, EMAC1);
            result = wait_lpbk_msg(EMAC1);
            ethernet_test(0,0, EMAC0);
            uart_puts("\r\n in GE1 loopback test enter character to send result ");
            uart_puts("\r\n Need some delay before sending packets");
            msleep(100);
            //uart_getch();
            lsi_mg_delay(WAIT_TIME); /* need to adjust number later */
        }
        /* loopback will continue, return only if requested to stop or failed */
        if (result == FAILED) {
            uart_puts("\r\n Failed to send lpbk message");
            sprintf((char *)&(hd_if->errmsg), 
                "Failed to send lpbk message, err = %x", 
                (unsigned int)hd_if->faults);
            sprintf((char *)&(cmd_status_addr->cur_msg), (char *)&(hd_if->errmsg)); 
            cmd_status_addr->cur_status = RESULT_FAILED;
            uart_puts((char *)hd_if->errmsg);
        } else if (result == PASSED) {
            hd_if->result = RESULT_SUCCESSFUL;
            cmd_status_addr->cur_status = RESULT_SUCCESSFUL;
            return (PASSED);
        }
    }
    if (op_type == OP_TEST_REQUEST) {
        cmd_status_addr->cur_command_recv = hd_if->select;
        if (hd_if->select == SELECT_DSP_CONSOLE) {
            uart_puts("\r\n PPB Console");
            uart_puts("\r\n Type <quit> to exit PPB Console prompt");
            uart_puts("\r\n Type <ctrl-a> <ctrl-x> to return to host Console ");
            lnerrors = ppb_con();
        } else if (hd_if->select == SELECT_DAC_1DOT5SM_HIGH) {
            uart_puts("\r\n in Set 1.5 Voltage Margin High");
            lnerrors = sp_setdac_high(DAC_1DOT5);
        } else if (hd_if->select == SELECT_DAC_1DOT5SM_LOW) {
            uart_puts("\r\n in Set 1.5 Voltage Margin Low ");
            lnerrors = sp_setdac_low(DAC_1DOT5);
        } else if (hd_if->select == SELECT_DAC_NO_1DOT5SM) {
            uart_puts("\r\n in No 1.5 Voltage Margin ");
            lnerrors = sp_setdac_norm(DAC_1DOT5);
            uart_puts("\r\n after No 1.5 Voltage Margin ");
        } else if (hd_if->select == SELECT_DAC_3DOT3SM_HIGH) {
            uart_puts("\r\n in Set .93 Voltage Margin High");
            lnerrors = sp_setdac_high(DAC_3DOT3);
        } else if (hd_if->select == SELECT_DAC_3DOT3SM_LOW) {
            uart_puts("\r\n in Set 3.3 Voltage Margin Low ");
            lnerrors = sp_setdac_low(DAC_3DOT3);
        } else if (hd_if->select == SELECT_DAC_NO_3DOT3SM) {
            uart_puts("\r\n in No 3.3 Voltage Margin ");
            lnerrors = sp_setdac_norm(DAC_3DOT3);
        } else if (hd_if->select == SELECT_DAC_1DOT5_SHOW) {
            uart_puts("\r\n in Show Voltage Margin ");
            lnerrors = sp_show1DOT5dac();
        } else if (hd_if->select == SELECT_DAC_3DOT3_SHOW) {
            uart_puts("\r\n in Show Voltage Margin ");
            lnerrors = sp_show3DOT3dac();
        } else if (hd_if->select == SELECT_READ_FPGA_DIR_REG) {
            uart_puts("\r\n in Read FPGA direct Register ");
            lnerrors = read_fpga_dir_reg();
        } else if (hd_if->select == SELECT_WRITE_FPGA_DIR_REG) {
            uart_puts("\r\n in Write FPGA direct Register ");
            lnerrors = write_fpga_dir_reg();
        } else if (hd_if->select == SELECT_READ_FPGA_INDIR_REG) {
            uart_puts("\r\n in Read FPGA direct Register ");
            lnerrors = read_fpga_indir_reg();
        } else if (hd_if->select == SELECT_WRITE_FPGA_INDIR_REG) {
            uart_puts("\r\n in Read FPGA direct Register ");
            lnerrors = write_fpga_indir_reg();
        } else if (hd_if->select == SELECT_ECC_MEM) {
            uart_puts("\r\n in ECC Memory test");
            lnerrors = ecc_mem_test();
        } else if (hd_if->select == SELECT_GE1_LPBK_PT) {
            uart_puts("\r\n in GE1 Internal Lpbk test ");
            lnerrors = ethernet_test(0, INTERNAL, 1);
        } else if (hd_if->select == SELECT_DSP_SDRAM) {
            uart_puts("\r\n in DDR test ");
            lnerrors = test_mem();
        } else if (hd_if->select == SELECT_UART_TEST) {
            uart_puts("\r\n in UART test ");
            /* Notify host the dsp ready to uart loopback test */
            hd_if->result = RESULT_SUCCESSFUL;
            cmd_status_addr->cur_status = RESULT_SUCCESSFUL;
            if (send_host_testmsg()) {
                uart_puts("\r\n Failed to send test message");
                sprintf((char *)&(hd_if->errmsg), 
                        "Failed to send test message, status = %x", 
                        (unsigned int)ppb_dss0_if->faults);
                uart_puts((char *)hd_if->errmsg);
            } else {
                uart_puts("\r\n Sent host msg command result\n");
                uart_puts("\r\n Now Waiting to host lpbk data\n");
            }
            lnerrors = ppb_con();
            if (lnerrors) {
                cmd_status_addr->cur_status = RESULT_FAILED;
            } else {
                cmd_status_addr->cur_status = RESULT_SUCCESSFUL;
            }
            return (NO_REPLY);
        } else if (hd_if->select == SELECT_READY) {
            uart_puts("\r\n in READY test ");
            lnerrors = 0;
        } else if (hd_if->select == SELECT_TDM_INTLPBK) {
            uart_puts("\r\n in TDM INT lpbk  test ");
            lnerrors = tdm_lpbk(INTERNAL);
        } else if (hd_if->select == SELECT_ARM11CPU1_BOOT) {
            uart_puts("\r\n in ARM11 CPU1 Boot test ");
            lnerrors = arm11_cpu1_boot_test();
        } else if (hd_if->select == SELECT_DSS0_SANITY) {
            uart_puts("\r\n in DSS CORE 0 test ");
/* SR check hd_if pointer value changed in core_sanity test but not reverted back */
            lnerrors = dss_core_sanity(DSS_CORE0);
            hd_if->select = SELECT_DSS0_SANITY;
        } else if (hd_if->select == SELECT_DSS1_SANITY) {
            uart_puts("\r\n in DSS CORE 1 test ");
            lnerrors = dss_core_sanity(DSS_CORE1);
            hd_if->select = SELECT_DSS1_SANITY;
        } else if (hd_if->select == SELECT_DSS2_SANITY) {
            uart_puts("\r\n in DSS CORE 2 test ");
            lnerrors = dss_core_sanity(DSS_CORE2);
            hd_if->select = SELECT_DSS2_SANITY;
        } else if (hd_if->select == SELECT_DSS3_SANITY) {
            uart_puts("\r\n in DSS CORE 3 test ");
            lnerrors = dss_core_sanity(DSS_CORE3);
            hd_if->select = SELECT_DSS3_SANITY;
        } else if (hd_if->select == SELECT_READY) {
            uart_puts("\r\n in READY test ");
            lnerrors = 0;
        } else if (hd_if->select == SELECT_FPGA_REG_TEST) {
            uart_puts("\r\n in FPGA Register test ");
            lnerrors = fpga_reg_test();
        } else if (hd_if->select == SELECT_FPGA_MEM_TEST) {
            uart_puts("\r\n in FPGA Memory test ");
            lnerrors = fpga_mem_test();
        } else if (hd_if->select == SELECT_FPGA_INT_TEST) {
            uart_puts("\r\n in FPGA Interrupt test ");
            lnerrors = fpga_intr_test();
        } else if (hd_if->select == SELECT_FPGA_TDMSW_FORCE_BYTE_TEST) {
            uart_puts("\r\n in FPGA TDMSW force byte test ");
            lnerrors = tdmsw_force_byte_test();
        } else if (hd_if->select == SELECT_TDM_EXTLPBK) {
            uart_puts("\r\n in TDM External Loopback test ");
            lnerrors = tdm_ext_lpbk(EXTERNAL);
        } else if (hd_if->select == SELECT_MEM_DISP) {
            uart_puts("\r\n in DSP Mem Display ");
            lnerrors = dsp_test_debug();
            if (lnerrors)
                cmd_status_addr->cur_status = RESULT_FAILED;
            else
                cmd_status_addr->cur_status = RESULT_SUCCESSFUL;
            return (NO_REPLY);
        } else if (hd_if->select == SELECT_INTF_SYNC) {
            uart_puts("\r\n in DSP Interface SYNC Signal test");
            lnerrors = dsp_test_intf_sync_sig(hd_if->param1, hd_if->param2);
        } else if (hd_if->select == SELECT_FW_VER_DISP) {
            uart_puts("\r\n in Display DSP FW version ");
            lnerrors = disp_ver(1);
        } else if (hd_if->select == SELECT_CODEC_SI32261_DIGITAL_LOOPBACK) {
            uart_puts("\r\n FXS SI32261 LOOPBACK ");
            lnerrors = silab_fxs_lpbk_test();
        } else if (hd_if->select == SELECT_CODEC_SI3050_DIGITAL_LOOPBACK) {
            uart_puts("\r\n FXS SI3050 LOOPBACK ");
            lnerrors = silab_fxo_lpbk_test();
        } else if (hd_if->select == SELECT_CODEC_SI32261_CALIBRATION) {
            uart_puts("\r\n FXS SI32261 Calibration ");
            lnerrors = si32261_common_mode_calibration_wo_result(1);
        } else if (hd_if->select == SELECT_CODEC_SI32261_CALIBRATE_RESULT) {
            uart_puts("\r\n FXS SI32261 Get Calibration Result");
            lnerrors = si32261_collect_cal_result();
        } else if (hd_if->select == SELECT_CODEC_SI32261_CALIBRATE_SAVE) {
            uart_puts("\r\n FXS SI32261 Get Calibration save");
            lnerrors = si32261_save_cal_data();
        } else if (hd_if->select == SELECT_CODEC_SET_FAIL_OVER_PORT) {
            uart_puts("\r\n Codec Set Fail Over Port");
            lnerrors = set_fail_over_port();
        } else if (hd_if->select == SELECT_FXS_FXO_LED) {
            uart_puts("\r\n FXS/FXO LED");
            lnerrors = fxs_fxo_led_utility();
        } else if (hd_if->select == SELECT_CODEC_SI32261_RING) {
            uart_puts("\r\n FXS SI32261 Ring");
            lnerrors = codec_si32261_set_stop_ring(hd_if->param1, TRUE);
        } else if (hd_if->select == SELECT_TDM_CODEC_RST) {
            uart_puts("\r\n TDM Codec Reset test");
            lnerrors = tdm_codec_reset_test();
        } else if (hd_if->select == SET_TOGGLE_SEP_MB_DBX_TEST_FLAG) {
            uart_puts("\r\n toggle flag : toggle_sep_test_dbx_flag");
            toggle_sep_test_dbx_flag(hd_if->param1);
        } else if (hd_if->select == SELECT_HW_BRD_TYPE_FLAG) {
            uart_puts("\r\n Set phoenix_host_hw_brd_type_flag");
            set_host_hw_brd_type_flag(hd_if->param1);
        /* For Get more debug information */
        } else if (hd_if->select == SELECT_GET_INFO) {
            uart_puts("\r\n Got Information ");
            lnerrors = get_info(1);
        } else {
            uart_puts("\r\n Test requested does not match supported tests");
            sprintf((char *)&(hd_if->errmsg), "\nTest requested %d does not\
                            match supported tests\n",  (int)hd_if->select);
            lnerrors = FAILED;
        }
        hd_if->faults |= hd_if->select;
        hd_if->errorcount += lnerrors;
        if (hd_if->errorcount == 0) {
            hd_if->result = RESULT_SUCCESSFUL;
            cmd_status_addr->cur_status = RESULT_SUCCESSFUL;
            return (PASSED);
        } else {
            cmd_status_addr->cur_status = RESULT_FAILED;
            sprintf((char *)&(cmd_status_addr->cur_msg), (char *)&(hd_if->errmsg)); 
            hd_if->result = RESULT_FAILED;
            bsp_debug_printf("\r\n now returning from proc lnerrors = %d\n", lnerrors);
            return (FAILED);
        }
 
    }

    return (PASSED); /* got something back */
}

/***********************************************************************
 *  
 * Function: simple_strtoul
 *
 * Description: Memory display 
 *
 * Input : val 
 *  
 * Returns: none
 *
 **********************************************************************
 */
unsigned long simple_strtoul (const char *cp,char **endp,unsigned int base)
{
    unsigned long result = 0,value;

    if (*cp == '0') {
        cp++;
        if ((*cp == 'x') && diagisxdigit(cp[1])) {
            base = 16;
            cp++;
        }
        if (!base) {
            base = 8;
        }
    }
    if (!base) {    
        base = 10;
    }
    while (diagisxdigit(*cp) && 
          (value = diagisdigit(*cp) ? *cp-'0' : (diagislower(*cp)
          ? diagtoupper(*cp) : *cp)-'A'+10) < base) {
        result = result*base + value;
        cp++;
    }
    if (endp) {
        *endp = (char *)cp;
    }
    return result;
}

/***********************************************************************
 *  
 * Function: put_8x
 *
 * Description: Octal display of the specified value
 *
 * Input : val 
 *  
 * Returns: none
 *
 **********************************************************************
 */
static void put_8x (uint32_t val) 
{
    int shift = 32;
    int digit;
    char *digits[] = {
    "0", "1", "2", "3", "4", "5", "6", "7",
    "8", "9", "A", "B", "C", "D", "E", "F"
    };

    do {
        shift -= 4;
        digit = (val >> shift) & 0xF;
        uart_puts(digits[digit]);
    } while (shift);
}

/***********************************************************************
 *  
 * Function: print_buffer
 *
 * Description: display the address specified in the format indicated 
 *
 * Input : addr, data format, count (number of bytes to display)
 *  
 * Returns: 0 (success)/ -1 (failure)
 *
 **********************************************************************
 */
static int print_buffer (uint32_t off, uint32_t addr, void* data, int32_t count)
{
    unsigned short linebuf[64];
    uint32_t *uip = (void*)linebuf;
    uint32_t orig;
    unsigned char *ucp = (void*)linebuf;
    int i;
    uint8_t linelen = 4;

    if (addr%4) return(-1) ;

    orig = addr;
    while (count > 0) {
        if (off == 1)
            put_8x(addr-orig); 
        else
            put_8x(addr); 

        /* check for overflow condition */
        if (count < linelen)
            linelen = count;
            /* Copy from memory into linebuf and print hex values */
            for (i = 0; i < linelen; i++) {
                uip[i] = *(volatile uint32_t *)data;
                uart_puts(" "); uart_put_long(uip[i], 16);
                data += 4;
            }

            /* Print data in ASCII characters */
            uart_puts("    ");
            for (i = 0; i < linelen*4; i++) {
            if (ucp[i]>=0x20 && ucp[i]<=0x7e) {
                char tbuf[2];

                tbuf[0] = ucp[i];
                tbuf[1] = '\0';
                uart_puts(tbuf) ;
            } else {
                uart_puts(".") ;
            }
        }
        uart_puts ("\r\n") ;
        /* update references */
        addr += linelen*4;
        count -= linelen*4;
    }

    return 0;
}

/***********************************************************************
 *  
 * Function: reset_dss_core
 *
 * Description: display memory 
 *
 * Input : cmdargs
 *  
 * Returns: none
 *
 **********************************************************************
 */
void reset_dss_core (char *cmdargs)
{
    static uint32_t core;
    char *end;

    core = simple_strtoul(cmdargs, &end, 16);
    if ((core >= CORE_DSS0) && (core <= CORE_DSS3)) {
        bsp_debug_printf("\r\nResetting DSS core %d\r\n", core);
        sp_ResetDSS(core);
    } else 
        bsp_debug_printf("\r\nPlease specify valid parameters");
}

/***********************************************************************
 *  
 * Function: unreset_dss_core
 *
 * Description: display memory 
 *
 * Input : cmdargs
 *  
 * Returns: none
 *
 **********************************************************************
 */
void unreset_dss_core (char *cmdargs)
{   
    static uint32_t core;
    char *end;
    
    core = simple_strtoul(cmdargs, &end, 16);
    if ((core >= CORE_DSS0) && (core <= CORE_DSS3)) {
        bsp_debug_printf("\r\nUnreset DSS core %d\r\n", core);
        sp_ReleaseDSS(core);
    } else {
        bsp_debug_printf("\r\nPlease specify valid parameters");
    }
}   

/***********************************************************************
 *  
 * Function: modify_mem
 *
 * Description: Modify memory at address.  Return when user enters space 
 *
 * Input : address 
 *         incrflag - goto next memory location ?
 *  
 * Returns: PASSED/FAILED
 *
 **********************************************************************
 */
int modify_mem(uint32_t addr, int incrflag)
{   
    ulong   i;
    int     nbytes;
    char UartLine[90];
    char* endp ;
    
    bsp_debug_printf("\r\n Please enter <space> to exit \r\n");
    if (addr%4) return(-1) ;
    
    /* Print the address, followed by value.  Then accept input for
     * the next value.  A non-converted value exits.
     */
    do {
        uart_put_long(addr, 16);

        bsp_debug_printf(" [0x%x]: ", *((uint   *)addr));
        nbytes = uart_rx_str(UartLine) ;
        if (UartLine[0] == 0x20) {
            nbytes = 0 ;
        } else if (UartLine[0] == '\0') {
            if (incrflag)
                addr += 4;
        } else {
            i = simple_strtoul(UartLine, &endp, 16) ;
            if (nbytes) {

                *((uint   *)addr) = i;
                if (incrflag)
                    addr += 4;
            }
        }
        bsp_debug_printf("\r\n");
    } while (nbytes);

    return 0;
}

/***********************************************************************
 *  
 * Function: sp_configureDACGPIO
 *
 * Description: initialize the GPIO pins to simulate I2C signals 
 *              SCL and SDA
 *
 * Input : None 
 *  
 * Returns: None
 *
 **********************************************************************
 */

/* ret: 0 => success, -1 => not configured */
void sp_configureDACGPIO (void)           
{       
    sp_GPIOasI2C_Init(SCL, SDA, 0);
}       

/***********************************************************************
 *  
 * Function: sp_setdac_high
 *
 * Description: Set high volatge margin
 *
 * Input : which_volt - 3.3 or 1.5
 *  
 * Returns: PASSED
 *
 **********************************************************************
 */
static int sp_setdac_high (int which_volt)
{   
    int dac_volt_high;
    uint16_t fpga_addr; 
    uint32_t fpga_data, reg_data;

    if (which_volt == DAC_1DOT5) {
        dac_volt_high = DAC_1DOT5_VOLT_HIGH;
    } else if (which_volt == DAC_3DOT3) {
        dac_volt_high = DAC_3DOT3_VOLT_HIGH;
    } else {
        uart_puts("\r\n unknown volt specified\n");
        return (FAILED);
    }

    fpga_addr = 0x9310;

    fpga_spi_direct_read(fpga_addr, 1, &fpga_data); 
    /* set 1.5 or 3.3 voltage margin bit */
    fpga_data |= dac_volt_high; 

    fpga_spi_direct_write(fpga_addr, 1, fpga_data);

    fpga_spi_direct_read(fpga_addr, 1, &reg_data);
    bsp_debug_printf("\r\n Voltage Margin reg: %x - %x\n", fpga_addr, reg_data);

    if ((reg_data & dac_volt_high) != dac_volt_high) {
        sprintf((char *)&(hd_if->errmsg), "\nSet margin high failed.\n");
        return (FAILED);
    }

    return (PASSED);
}
        
/***********************************************************************
 *  
 * Function: sp_setdac_low
 *
 * Description: Set Low volatge margin
 *
 * Input : which_volt - 3.3 ot 1.5 
 *  
 * Returns: PASSED
 *  
 **********************************************************************
 */
static int sp_setdac_low (int which_volt)
{   
    int dac_volt_low;
    uint16_t fpga_addr;
    uint32_t fpga_data, reg_data;

    if (which_volt == DAC_1DOT5) {
        dac_volt_low = DAC_1DOT5_VOLT_LOW;
    } else if (which_volt == DAC_3DOT3) {
        dac_volt_low = DAC_3DOT3_VOLT_LOW;
    } else {
        uart_puts("\r\n unknown volt specified\n");
        return (FAILED);
    }

    fpga_addr = 0x9310;

    fpga_spi_direct_read(fpga_addr, 1, &fpga_data);
    /* set 1.5 or 3.3 voltage margin bit */
    fpga_data |= dac_volt_low;

    fpga_spi_direct_write(fpga_addr, 1, fpga_data);

    fpga_spi_direct_read(fpga_addr, 1, &reg_data);
    bsp_debug_printf("\r\n Voltage Margin reg: %x - %x\n", fpga_addr, reg_data);
    if ((reg_data & dac_volt_low) != dac_volt_low) {
        sprintf((char *)&(hd_if->errmsg), "\nSet margin low failed.\n");
        return (FAILED);
    }
            
    return (PASSED);
}       
        

/***********************************************************************
 *  
 * Function: sp_setdac_norm
 *
 * Description: Set volatge margin to normal
 *
 * Input : which_volt - 3.3 ot 1.5
 *  
 * Returns: PASSED
 *  
 **********************************************************************
 */
static int sp_setdac_norm (int which_volt)
{
    int volt_reg;
    uint16_t fpga_addr;
    uint32_t reg_data;
    
    if (which_volt == DAC_1DOT5)
        volt_reg = DAC_1DOT5_MASK;
    else if (which_volt == DAC_3DOT3)
        volt_reg = DAC_3DOT3_MASK;
    else {
        uart_puts("\r\n unknown volt specified\n");
        return (FAILED);
    }

    fpga_addr = 0x9310;

    fpga_spi_direct_read(fpga_addr, 1, &reg_data);
    /* clear 1.5 or 3.3 voltage margin bit */
    reg_data &= ~(volt_reg);  
    fpga_spi_direct_write(fpga_addr, 1, reg_data);

    fpga_spi_direct_read(fpga_addr, 1, &reg_data);
    bsp_debug_printf("\r\n Voltage Margin reg: %x - %x\n", fpga_addr, reg_data);
    if ((reg_data & volt_reg) != 0) {
        sprintf((char *)&(hd_if->errmsg), "\nSet margin normal failed.\n");
        return (FAILED);
    }

    return (PASSED);
            
}       

/***********************************************************************
 *  
 * Function: sp_show1DOT5dac
 *
 * Description: Show voltage margin
 *
 *  
 * Returns: PASSED
 *  
 **********************************************************************
 */
static int sp_show1DOT5dac (void)
{
    uint32_t dac_value;   

    int which_voltage = 0;

    which_voltage = DAC_1DOT5;

    dac_value = sp_readdac (which_voltage);

    bsp_debug_printf("dac value: %lx",dac_value);

    if (dac_value == 0x8 ) {

        sprintf((char *)&(hd_if->bufmsg), "The 1.5 voltage Margin status is High");
        
    } else if (dac_value == 0x4) {

        sprintf((char *)&(hd_if->bufmsg), "The 1.5 voltage Margin status is Low");

    } else if (dac_value == 0x0) {

        sprintf((char *)&(hd_if->bufmsg), "The 1.5 voltage Margin status is Normal");

    } else {

        sprintf((char *)&(hd_if->bufmsg), "Please set 1.5 voltage margin to Normal");
    }


    return (PASSED);

}


/***********************************************************************
 *  
 * Function: sp_show3DOT3dac
 *
 * Description: Show voltage margin
 *  
 * Returns: PASSED
 *  
 **********************************************************************
 */
static int sp_show3DOT3dac (void)
{
    uint32_t dac_value;   

    int which_voltage = 0;

    which_voltage = DAC_3DOT3;

    dac_value = sp_readdac (which_voltage);

    bsp_debug_printf("dac value: %lx",dac_value);

    if (dac_value == 0x20 ) {

        sprintf((char *)&(hd_if->bufmsg), "The 3.3 voltage Margin status is High");
        
    } else if (dac_value == 0x10) {

        sprintf((char *)&(hd_if->bufmsg), "The 3.3 voltage Margin status is Low");

    } else if (dac_value == 0x0) {

        sprintf((char *)&(hd_if->bufmsg), "The 3.3 voltage Margin status is Normal");

    } else {

        sprintf((char *)&(hd_if->bufmsg), "Please set 3.3 voltage margin to Normal");
    }


    return (PASSED);

}       

/***********************************************************************
 *  
 * Function: sp_readdac
 *
 * Description: Display the DAC registers.
 *      
 * Input : None
 *  
 * Returns: value
 *  
 **********************************************************************
 */
uint16_t sp_readdac (int which_volt)
{
    int volt_reg;
    uint16_t fpga_addr;
    uint32_t reg_data;

    if (which_volt == DAC_1DOT5)
        volt_reg = DAC_1DOT5_MASK;
    else if (which_volt == DAC_3DOT3)
        volt_reg = DAC_3DOT3_MASK;
    else {
        uart_puts("\r\n unknown volt specified\n");
        return (FAILED);
    }

    fpga_addr = 0x9310;

    fpga_spi_direct_read(fpga_addr, 1, &reg_data);


    return (reg_data&volt_reg);
}

/***********************************************************************
 *  
 * Function: eprom_write
 *
 * Description: Write to SPI EEPROM at address 0x60000 
 *
 * Input : cmdargs - string enterred by user
 *         arr - internal storage for read data
 *         size - 20bytes of data will be return 
 *  
 * Returns: none
 *
 **********************************************************************
 */ 
void eprom_write (char *cmdargs, uint8_t *arr, int size)
{
    //static uint32_t addr, addrto ;
    uint32_t addr, addrto ;
    char            *arg, *end, mem_disp[60] ;

    if ((arg=strsep(&cmdargs, " ")) != NULL) {
        addr = simple_strtoul(arg, &end, 16) ;
    }
    if ((arg=strsep(&cmdargs, " ")) != NULL) {
        addrto = simple_strtoul(arg, &end, 16) ;
    }
    if ((arg=strsep(&cmdargs, " ")) != NULL) {
        size = simple_strtoul(arg, &end, 16) ;
    }
    if (size > 0x20)
        size = 0x20;
    if (addrto < 0x5ffff)
        addrto = 0x60000;
    if (addrto > 0x7ffff)
        addrto = 0x60000;
    bsp_debug_printf("\r\n copy %d bytes from 0x%x to 0x%x\n", size, addr, addrto);
    bsp_debug_printf("\r\n Display 0x%x contents -", addrto);
    eeprom_read((unsigned) addrto, arr, size);
    sprintf(mem_disp, "0x%x %d", (unsigned int)arr, (uint)(size/4));
    do_mem_md_off(mem_disp);
    bsp_debug_printf("\r\n Display 0x%x contents -", addr);
    eeprom_read((unsigned) addr, arr, size);
    sprintf(mem_disp, "0x%x %d", (unsigned int)arr, (uint)(size/4));
    do_mem_md_off(mem_disp);
    eeprom_write((unsigned) addrto, arr, size);

}

/***********************************************************************
 *  
 * Function: eprom_disp
 *
 * Description: display x0100 bytes of SPI EEPROM From address specified 
 *
 * Input : cmdargs
 *  
 * Returns: none
 *
 **********************************************************************
 */ 
void eprom_disp (char *cmdargs, uint8_t *arr, int size)
{
    static uint32_t addr ;
    char            *arg, *end, mem_disp[60] ;

    if ((arg=strsep(&cmdargs, " ")) != NULL) {
        addr = simple_strtoul(arg, &end, 16) ;
    }
    bsp_debug_printf("\r\n In eprom_disp  0x%x\n ", addr);
    eeprom_read((unsigned) addr, arr, size);
    sprintf(mem_disp, "0x%x %d", (unsigned int)arr, (uint)(size/4));
    do_mem_md_off(mem_disp);
 
}

/***********************************************************************
 *  
 * Function: do_mem_md
 *
 * Description: display memory 
 *
 * Input : cmdargs
 *  
 * Returns: none
 *
 **********************************************************************
 */
void do_mem_md (char* cmdargs)
{
    static uint32_t addr ;
    static uint32_t length=0 ;
    char            *arg, *end ;

    bsp_debug_printf("\r\n");
    if ((arg=strsep(&cmdargs, " ")) != NULL) {
        addr = simple_strtoul(arg, &end, 16) ;
        if ((arg=strsep(&cmdargs, " ")) != NULL) {
            length = simple_strtoul(arg, &end, 16) ;
        }
    }
    if (addr%4) return;
    /* Print the lines. */
    if (length > 1024) length = 1024;
    print_buffer(0, addr, (void*)addr, (int32_t)length);
    addr += length ; //4*length;

    return;
}

/***********************************************************************
 *  
 * Function: do_mem_md
 *
 * Description: display memory 
 *
 * Input : cmdargs
 *  
 * Returns: none
 *
 **********************************************************************
 */
void do_mem_md_off (char* cmdargs)
{
    static uint32_t addr ;
    static uint32_t length=0 ;
    char            *arg, *end ;

    bsp_debug_printf("\r\n");
    if ((arg=strsep(&cmdargs, " ")) != NULL) {
        addr = simple_strtoul(arg, &end, 16) ;
        if ((arg=strsep(&cmdargs, " ")) != NULL) {
            length = simple_strtoul(arg, &end, 16) ;
        }
    }
    if (addr%4) return;
    /* Print the lines. */ 
    if (length > 1024) length = 1024;
    print_buffer(1, addr, (void*)addr, (int32_t)length);
    addr += length ; //4*length;

    return;
}

/***********************************************************************
 *  
 * Function: do_mem_mw
 *
 * Description: memory write
 *
 * Input : cmdargs
 *  
 * Returns: none
 *
 **********************************************************************
 */
int do_mem_mw (char* cmdargs)
{
    uint32_t        *curaddr, addr ;
    uint32_t        length=0, pattern=0 ;
    char            *arg, *end, mem_disp[60] ;

    if ((arg=strsep(&cmdargs, " ")) != NULL) {
        addr = simple_strtoul(arg, &end, 16) ;
        if ((arg=strsep(&cmdargs, " ")) != NULL) {
            length = simple_strtoul(arg, &end, 16) ;
            if ((arg=strsep(&cmdargs, " ")) != NULL) {
                pattern = simple_strtoul(arg,&end,16) ;
            }
        }
    } else {
        addr = LastArg1 ;
        length = LastArg2 ;
        pattern = LastArg3 ;
    }
    if (addr%4) {
        return(-1) ;
    }
    curaddr = (uint32_t*)addr ;
    while (curaddr < (uint32_t*)(addr+(length*4))) {
        *curaddr = pattern ;
        curaddr ++ ;
    }

    LastArg1 = addr ;
    LastArg2 = length ;
    LastArg3 = pattern ;

    bsp_debug_printf("\r\n");
    sprintf(mem_disp, "0x%x %d", (unsigned int)addr, (uint)(length*4));
    do_mem_md(mem_disp);

    return 0 ;

}

/***********************************************************************
 *  
 * Function: do_mem_mm
 *
 * Description: incremental memory write
 *
 * Input : cmdargs
 *  
 * Returns: none
 *
 **********************************************************************
 */
int do_mem_mm (char* cmdargs)
{
    uint32_t        addr ;
    char            *arg, *end ;

    if ((arg=strsep(&cmdargs, " ")) != NULL) {
        addr = simple_strtoul(arg, &end, 16) ;
    } else {
        return -1 ;
    }
    modify_mem(addr, 1);
    addr += 4;
    return (0) ;
}

/***********************************************************************
 *  
 * Function: do_mem_nm
 *
 * Description: non incremental memory write
 *
 * Input : cmdargs
 *  
 * Returns: none
 *
 **********************************************************************
 */
int do_mem_nm (char* cmdargs)
{
    uint32_t        addr ;
    char            *arg, *end ;

    if ((arg=strsep(&cmdargs, " ")) != NULL) {
        addr = simple_strtoul(arg, &end, 16) ;
    } else {
        return -1 ;
    }
    modify_mem(addr, 0) ;
    return(0) ;
}

/***********************************************************************
 *      
 * Function: disp_eth_buff
 *      
 * Description: Display Ethernet 1 Buff 
 *
 * Input : None
 *  
 * Returns: none
 *      
 **********************************************************************
 */     
void disp_eth_buff (void)
{
    char mem_disp[50];

    bsp_debug_printf("\r\n EMAC1 TX Buffer =0x%x \r\n", out_frame[1]);
    sprintf(mem_disp, "0x%x %d", (unsigned int)out_frame[1], 256);
    do_mem_md(mem_disp);
    bsp_debug_printf("\r\n EMAC1 RX Buffer =0x%x \r\n", in_frame[0]);
    sprintf(mem_disp, "0x%x %d", (unsigned int)in_frame[0], 256);
    do_mem_md(mem_disp);
}

/***********************************************************************
 *  
 * Function: disp_eth_bd
 *
 * Description: Display Ethernet 1 BD 
 *
 * Input : None
 *  
 * Returns: none
 *
 **********************************************************************
 */
void disp_eth_bd (void)
{
    uint size;
    char mem_disp[50];

    bsp_debug_printf("\r\n EMAC1 TX BD =0x%x, #of elements %d", 
                     TxBuffDes1, BuffTxElements);
    bsp_debug_printf("\r\n 9 Queues, using queue 1 at offset 0x30");
    bsp_debug_printf("\r\n status = len:14,resv:14:eof,sof,own,wrap:1 \r\n");
    size = BuffTxElements;
    sprintf(mem_disp, "0x%x %d", (unsigned int)TxBuffDes1, size);
    do_mem_md(mem_disp);
    bsp_debug_printf("\r\n EMAC1 RX BD =0x%x, #of elements %d\r\n", 
                     RxBuffDes1, BuffRxElements);
    bsp_debug_printf("\r\n status = len:14,mac_err,chksum_srr,DLT_fail:1,L2 format:2,unrecog_L2,");
    bsp_debug_printf("\r\n unrecog_L2L3,badip,type,svt_fail,cor,eof,sof,own,wrap:1\r\r");
    size = BuffRxElements;
    sprintf(mem_disp, "0x%x %d", (unsigned int)RxBuffDes1, size);
    do_mem_md(mem_disp);
}

/***********************************************************************
 *  
 * Function: disp_ddr3_reg
 *
 * Description: Display DDr3 controller registers 
 *
 * Input : None
 *  
 * Returns: none
 *
 **********************************************************************
 */
void disp_ddr3_reg (void)
{
    uint addr, size;
    char mem_disp[50];

    addr = 0x9C000000;
    size = 0x2bc;
    bsp_debug_printf("\r\n DDR3 Controller Registers:  \r\n");
    sprintf(mem_disp, "0x%x %d", (unsigned int)addr, size);
    do_mem_md(mem_disp);

}

/***********************************************************************
 *  
 * Function: disp_tdm_reg
 *
 * Description: Display registers related to TDM port specified
 *
 * Input : cmdargs
 *  
 * Returns: none
 *
 **********************************************************************
 */
void disp_tdm_reg (char *cmdargs)
{
    static uint32_t port;
    uint addr, size;
    char mem_disp[50], *end;

    port = simple_strtoul(cmdargs, &end, 16);
    if (port < 6) {
        bsp_debug_printf("\r\nDisplay TDM port %d registers \r\n", port);
    } else {
        bsp_debug_printf("\r\nPlease specify valid port between 0 and 6\n");
        return;
    }
    bsp_debug_printf("\r\n TDM SIU Registers for Port:  %d\r\n", port);
    addr = 0x98010000 + (0x100 * port);
    size = 0x100;
    sprintf(mem_disp, "0x%x %d", (unsigned int)addr, size);
    do_mem_md(mem_disp);
    bsp_debug_printf("\r\n TDM SWTU Status and Control Registers for Port:  %d\r\n", port);
    addr = 0x98010800 + (0x100 * port);
    size = 0x100;
    sprintf(mem_disp, "0x%x %d", (unsigned int)addr, size);
    do_mem_md(mem_disp);
    bsp_debug_printf("\r\n TDM SWTU Source Channel Registers for Port:  %d\r\n", port);
    addr = 0x98020000 + (0x2000 * port);
    size = 0x100;
    sprintf(mem_disp, "0x%x %d", (unsigned int)addr, size);
    do_mem_md(mem_disp);
    bsp_debug_printf("\r\n TDM SWTU Destination Channel Registers for Port: %d\r\n ", port);
    addr = 0x98021000 + (0x2000 * port);
    size = 0x100;
    sprintf(mem_disp, "0x%x %d", (unsigned int)addr, size);
    do_mem_md(mem_disp);
    bsp_debug_printf("\r\n TDM Interrupt Control Registers \r\n");
    addr = 0x98011060;
    size = 0x40;
    sprintf(mem_disp, "0x%x %d", (unsigned int)addr, size);
    do_mem_md(mem_disp);
    bsp_debug_printf("\r\n TDM Universal Counters Registers\r\n");
    addr = 0x98011100;
    size = 0x200;
    sprintf(mem_disp, "0x%x %d", (unsigned int)addr, size);
    do_mem_md(mem_disp);
    bsp_debug_printf("\r\n TDM src buffer for port %d\r\n", port);
    addr = (uint)&(SWTU0_SOURCEBUFFER[port][0][0]);
    size = 0x100;
    sprintf(mem_disp, "0x%x %d", (unsigned int)addr, size);
    do_mem_md(mem_disp);
    bsp_debug_printf("\r\n TDM dest buffer for port %d\r\n", port);
    addr = (uint)&(SWTU0_DESTINATIONBUFFER[port][0][0]);
    size = 0x100;
    sprintf(mem_disp, "0x%x %d", (unsigned int)addr, size);
    do_mem_md(mem_disp);
}

/***********************************************************************
 *  
 * Function: disp_ver
 *
 * Description: Display Diags firmware version
 *
 * Input : void
 *  
 * Returns: none
 *
 **********************************************************************
 */
int get_info(int host_cmd)
{
    int number_of_channel = 10;
    unsigned int i = 1;
    char *c = (char*)&i;
    if (*c)    
        PRINT_STR("Little endian");
    else
        PRINT_STR("Big endian");

    PRINT_STR("\r");
    PRINT_STR("PRINT_STR");
    PRINT_STR("NUMBER_OF_CHANNEL");
    PRINT_DEC(number_of_channel);
    bsp_debug_printf("\r\nbsp_debug_printf ");
    uart_puts("\r\n uart_puts \n");

    PRINT_STR("Done");

    PRINT_STR("Param 2 : ");
    PRINT_DEC(hd_if->param1);
    PRINT_STR("param 3 : ");
    PRINT_DEC(hd_if->param2);

    if (hd_if->param1 == 0xaa) {
        msleep(1000);
        hd_if->param3 = 0xdd;
    }
    if (hd_if->param2 == 0x55) {
        hd_if->param4 = 0xcc;
    }
    sprintf((char *)&(hd_if->errmsg), "get info failure\n");
    return PASSED;

}

/***********************************************************************
 *  
 * Function: disp_ver
 *
 * Description: Display Diags firmware version
 *
 * Input : void
 *  
 * Returns: none
 *
 **********************************************************************
 */
static int disp_ver (int host_cmd)
{
    uart_puts("\r\nCisco NGSM \r\n");
    uart_puts(verstring);
    uart_puts("\r\n");
    if (host_cmd == 1)
        sprintf((char *)&(hd_if->bufmsg), "%s", verstring);
    return (PASSED);
}

/***********************************************************************
 *  
 * Function: do_show_help
 *
 * Description: show commands supported 
 *
 * Input : cmdargs
 *  
 * Returns: none
 *
 **********************************************************************
 */
static void do_show_help (char *cmdargs) 
{
    uart_puts("\r\nmh 1.5                     DAC 1.5V margin high");
    uart_puts("\r\nml 1.5                     DAC 1.5V margin low");
    uart_puts("\r\nnm 1.5                     Turn off 1.5V margin");
    uart_puts("\r\ndacset 1.5 <value in hex>  Set DAC 1.5V with value");
    uart_puts("\r\ndac 1.5                    display 1.5V DAC regsiter");
    uart_puts("\r\nmh .93                     DAC .93V margin high");
    uart_puts("\r\nml .93                     DAC .93V margin low");
    uart_puts("\r\nnm .93                     Turn off .93V margin");
    uart_puts("\r\ndacset .93 <value in hex>  Set DAC .93V with value");
    uart_puts("\r\ndac .93                    display .93V DAC regsiter");
    uart_puts("\r\nmw <addr> <len> <pattern>  memory write");
    uart_puts("\r\nmm <addr>                  memory modify");
    uart_puts("\r\nmd <start> <len>           display memory");
    uart_puts("\r\nmw <addr> <len> <pattern>  memory write");
    uart_puts("\r\nmm <addr>                  memory modify");
    uart_puts("\r\nnm <addr>                  memory modify (non-incremental)");
    uart_puts("\r\nreload                     reload the ARM");
    uart_puts("\r\nenet bd                    display Buffer Descriptor");
    uart_puts("\r\nenet buffer                display Buffer");
    uart_puts("\r\nreset <1-4>                reset a DSS core");
    uart_puts("\r\nunreset <1-4>              unreset a DSS core");
    uart_puts("\r\ntdm <0-5>                  display TDM registers");
    uart_puts("\r\nddr3                       display DDR3 registers");
    uart_puts("\r\neread <addr>               display eeprom contents ");
    uart_puts("\r\newrite <from> <to> <size>  copy eeprom from - to ");
    uart_puts("\r\nver                        Diags firmware version");
}

/***********************************************************************
 *  
 * Function: cmd_parse
 *
 * Description: PPB console commands 
 *
 * Input : which cmd  
 *  
 * Returns: none
 *
 **********************************************************************
 */
static void cmd_parse (const char *cmd, uint32_t len)
{
    uint32_t data;
    uint16_t val;
    uint8_t eprom_data[0x200];

    if (strncmp(cmd, "reload", 6) == 0) {
        uart_puts("\r\nResetting ");
        REG32_READ(LSI_SP27XX_CAR_CHIP_RESET_RA, data);
        REG32_SET_BITS(LSI_SP27XX_CAR_CHIP_RESET_RA, 1);
    } else if (strncmp(cmd, "memtest", 7) == 0) {
        test_mem();
        uart_puts("\r\n");
    } else if (strncmp(cmd, "mh 1.5", 6) == 0) {
        sp_setdac_high(DAC_1DOT5);
    } else if (strncmp(cmd, "ml 1.5", 6) == 0) {
        sp_setdac_low(DAC_1DOT5);
    } else if (strncmp(cmd, "nm 1.5", 6) == 0) {
        sp_setdac_norm(DAC_1DOT5);
    } else if (strncmp(cmd, "dac 1.5", 7) == 0) {
        val = sp_readdac(DAC_1DOT5);
        bsp_debug_printf("\r\n DAC Reg at 0x9310 = 0x%x", val);
    } else if (strncmp(cmd, "mh 3.3", 6) == 0) {
        sp_setdac_high(DAC_3DOT3);
    } else if (strncmp(cmd, "ml 3.3", 6) == 0) {
        sp_setdac_low(DAC_3DOT3);
    } else if (strncmp(cmd, "nm 3.3", 6) == 0) {
        sp_setdac_norm(DAC_3DOT3);
    } else if (strncmp(cmd, "dac 3.3", 7) == 0) {
        val = sp_readdac(DAC_3DOT3);
        bsp_debug_printf("\r\n DAC Reg at 0x9310 = 0x%x", val);
    } else if (strncmp(cmd, "eread", 5) == 0) {
        spi_init();
        eprom_disp((char *)cmd+6, (void *)eprom_data, 0x100);
    } else if (strncmp(cmd, "ewrite", 6) == 0) {
        eprom_write((char *)cmd+7, (void *)eprom_data, 0x20);
    } else if (strncmp(cmd, "md", 2) == 0) {
        do_mem_md((char *)cmd+3);
    } else if (strncmp(cmd, "mw", 2) == 0) {
        do_mem_mw((char *)cmd+3);
    } else if (strncmp(cmd, "mm", 2) == 0) {
        do_mem_mm((char *)cmd+3);
    } else if (strncmp(cmd, "nm", 2) == 0) {
        do_mem_nm((char *)cmd+3);
    } else if (strncmp(cmd, "help", 4) == 0) {
        do_show_help((char *)cmd + 5);
    } else if (strncmp(cmd, "reset", 5) == 0) {
        reset_dss_core((char *)cmd+6);
    } else if (strncmp(cmd, "unreset", 7) == 0) {
        unreset_dss_core((char *)cmd+8);
    } else if (strncmp(cmd, "tdm", 3) == 0) {
        disp_tdm_reg((char *)cmd+4);
    } else if (strncmp(cmd, "ver", 3) == 0) {
        disp_ver(0);
    } else if (strncmp(cmd, "ddr3", 4) == 0) {
        disp_ddr3_reg();
    } else if (strncmp(cmd, "enet bd", 7) == 0) { 
        disp_eth_bd();
    } else if (strncmp(cmd, "enet buffer", 7) == 0) {
        disp_eth_buff();
    } else {
#ifdef DEBUG_CONSOLE_INPUT
        uart_puts("\n");
        uart_put_long(*(uint32_t *)cmd, 16);
        uart_puts(" ");
        uart_put_long(*(uint32_t *)(cmd + 4), 16);
        uart_puts(" ");
#endif
    }
}

/***********************************************************************
 *
 * Function:  boot_hpi_failure
 *
 * Description: wrong port
 *
 * Input : timer out value
 *
 * Returns: none
 *
 **********************************************************************
 */
static void boot_hpi_failure(uint32_t fail_code) {
    while (1) ;
}

/***********************************************************************
 *
 * Function: mac_config
 *
 * Description: get mac address
 *
 * Input : which DSP mac
 *
 * Returns: none
 *
 **********************************************************************
 */
static void mac_config (uint32_t which_mac)
{
    /* mac addrs are in checksum table by bootloader */
    if (which_mac == AG_MG_PPB_MAC0_DEVICE) {
        memcpy(&src_macaddr, (dss_mgr_checksum_table.mac_sa), 6);
        memcpy(&dest_macaddr, (dss_mgr_checksum_table.mac_da), 6);
    } else {
        boot_hpi_failure(0);
    }
    bsp_debug_printf("\r\n mac_config() after dss_mgr_checksum_table mac sa, memcpy");
    bsp_debug_printf("\r\n src_macaddr = 0x%x:0x%x:0x%x:0x%x:0x%x:0x%x",
                     src_macaddr[0], src_macaddr[1], src_macaddr[2],
                     src_macaddr[3], src_macaddr[4], src_macaddr[5]);
    bsp_debug_printf("\r\n dest_macaddr = 0x%x:0x%x:0x%x:0x%x:0x%x:0x%x",
                     dest_macaddr[0], dest_macaddr[1], dest_macaddr[2],
                     dest_macaddr[3], dest_macaddr[4], dest_macaddr[5]);
}

/***********************************************************************
 *
 * Function: init_ppb
 *
 * Description: Initialize DSP here for the portion that has not
 *              been done by bootloader.
 *
 * Input : dsp_device_p 
 *
 * Returns: none 
 *
 * Note:
 * init_ppb: need to know how much bootloader has done 
 * according to jim: bootloader init DDR3, ppb init TDM
 * and whatever bootloader does not do.
 * All current init in DSS will move to PPB
 **********************************************************************
 */
static void init_ppb(dsp_type_t * dsp_device_p)
{
    /* DSP code to find configuration parameters */
    /* SR ?? */
    //AG_MG_PPB_BOOT_DefaultInit();
    /* scan parameters to be used in driver init */
    //AG_MG_PPB_BOOT_ParameterScan(&list_end_address);
    /* assuming that bootloader has already set up the mac */
    mac_config(AG_MG_PPB_MAC0_DEVICE);
}

/***********************************************************************
 *
 * Function: _diag_ppb_start
 * Description: Entry point for diagnostics running on ARM/PPB       
 *		Major task for this diagnostics is to do all
 *		necessary initialziation here and interface to
 *		host platform for DSS cores.
 *
 * Input : none
 *
 * Returns: none 
 *
 **********************************************************************
 */
int _diag_ppb_start (void)
{
    uint32_t count, dsp0, dsp1, dsp2, dsp3;
    uint32_t ret, sp2700, dss_cores, char_count;
    uint8_t  *recv_p;
    uint8_t board_id;

#ifdef BOOT_DEBUG
    bsp_debug_printf("\r\n car_regs_ptr = 0x%x", (uint32_t )car_regs_ptr);
#endif

    /* Lock the UART resource for ARM Core (uart shared between DSS cores) */
    *uart_mem = CORE_ARM;
    *uart_getlock = FREE;

    /* Init the UART for the specified baud rate */
    SP_INIT_SERIAL(375, 9600); /* SYSCLK = 375MHz and baud rate = 9600 */

    memset(&__BSS_START, 0, &__BSS_END - &__BSS_START);
    memset((char *)ppb_bootmsg_addr, 0, sizeof(bootup_msg_t));
    memset((char *)cmd_status_addr, 0, sizeof(host_comm_status_t));

    uart_puts("\r\nCisco NGSM \r\n");
    uart_puts(verstring);

    /* Boot progress 1. Write the verstring to DDR3 memory */
    char_count = sprintf((char *)&(ppb_bootmsg_addr->msg[0]), verstring);

    /* identify chipset */
    init_id();

    /* Read PFUSE123 to find if SP2702 or SP2704 */
    sp2700 = sp_check27xxPFUSE123();

    /* Reset all DSS Cores */
    if (sp2700 == PFUSE123_SP2702) {
        uart_puts("\r\n PFUSE123 SP2702");
        dss_cores = SP_ALL_2702_DSS_CORES_BM;
        /* Boot progress 2. Write Device id, chip id */
        char_count += sprintf((char *)&(ppb_bootmsg_addr->msg[char_count]),
                              "DEVID = 0x%x, CHIPID = 0x%x, PFUSE123 SP2702",
                              dsp_device_p->device_type, dsp_device_p->chip_id);
    } else {
        uart_puts("\r\n PFUSE123 SP2704");
        dss_cores = SP_ALL_DSS_CORES_BM;
        /* Boot progress 2. Write Device id, chip id */
        char_count += sprintf((char *)&(ppb_bootmsg_addr->msg[char_count]),
                              "DEVID = 0x%x, CHIPID = 0x%x, PFUSE123 SP2704",
                              dsp_device_p->device_type, dsp_device_p->chip_id);
    }

    sp_ResetDSS(dss_cores);
    lsi_mg_delay(100);

    /* Now Bringup the DSS Cores */
    sp_ReleaseDSS(dss_cores);

    /* Delay a while for all DSS cores to boot up */
    lsi_mg_delay(10000);

    /* Bootloader needs to setup the uart and MAC for dhcp please do not
       redo any init here */
    init_ppb(dsp_device_p);

    /* 
     * Check magic location for MAGIC number (43)
     * SR make sure the locations are cleared before test
     */
    count = 0; dsp0 = 0; dsp1 = 0; dsp2 = 0; dsp3 = 0;
    while (*dss0_if != MAGIC) {
        lsi_mg_delay(WAIT_TIME); /* need to adjust number later */
        if (++count > 100) {
            dsp0 = 1;
            bsp_debug_printf("\r\n DSS0 Not Up");
            /* Boot progress 3. DSS 0 status */
            char_count += sprintf((char *)&(ppb_bootmsg_addr->msg[char_count]), "DSS0 Not Up, ");
            break;
        } 
    }
    if (dsp0 == 0) {
            /* Boot progress 3. DSS 0 status */
            bsp_debug_printf("\r\n DSS0 Up");
            char_count += sprintf((char *)&(ppb_bootmsg_addr->msg[char_count]), "DSS0 Up, ");
    }
    count = 0;
    while (*dss1_if != MAGIC) {
        lsi_mg_delay(WAIT_TIME); /* need to adjust number later */
        if (++count > 100) {
            dsp1 = 1;
            bsp_debug_printf("\r\n DSS1 Not Up");
            /* Boot progress 3. DSS 0 status */
            char_count += sprintf((char *)&(ppb_bootmsg_addr->msg[char_count]), "DSS1 Not Up, ");
            break;
        }
    }
    if (dsp1 == 0) {
            /* Boot progress 3. DSS 0 status */
            bsp_debug_printf("\r\n DSS1 Up");
            char_count += sprintf((char *)&(ppb_bootmsg_addr->msg[char_count]), "DSS1 Up, ");
    }
    if (sp2700 == PFUSE123_SP2704) {
        count = 0;
        while (*dss2_if != MAGIC) {
            lsi_mg_delay(WAIT_TIME); /* need to adjust number later */
            if (++count > 100) {
                dsp2 = 1;
                /* Boot progress 3. DSS 0 status */
                bsp_debug_printf("\r\n DSS2 Not Up");
                char_count += sprintf((char *)&(ppb_bootmsg_addr->msg[char_count]), "DSS2 Not Up, ");
                break;
            }
        }
        if (dsp2 == 0) {
            /* Boot progress 3. DSS 0 status */
            bsp_debug_printf("\r\n DSS2 Up");
            char_count += sprintf((char *)&(ppb_bootmsg_addr->msg[char_count]), "DSS2 Up, ");
        }
        count = 0;
        while (*dss3_if != MAGIC) {
            lsi_mg_delay(WAIT_TIME); /* need to adjust number later */
            if (++count > 100) {
                dsp3 = 1;
                /* Boot progress 3. DSS 0 status */
                bsp_debug_printf("\r\n DSS3 Not Up");
                char_count += sprintf((char *)&(ppb_bootmsg_addr->msg[char_count]), "DSS3 Not Up, ");
                break;
            }
        }
        if (dsp3 == 0) {
            /* Boot progress 3. DSS 0 status */
            char_count += sprintf((char *)&(ppb_bootmsg_addr->msg[char_count]), "DSS3 Up, ");
            bsp_debug_printf("\r\n DSS3 Up");
        }
    }
    ethernet_test(0, 0, 0); 
    char_count += sprintf((char *)&(ppb_bootmsg_addr->msg[char_count]), "Setup Ethernet ");

    volatile uint32_t *addr, data;
    /* wait request */
    sp_InitGPIO();
    uart_puts(" GPIO dir = ");
    addr = (uint32_t *)(0x30046400);
    data = *addr;
    uart_put_long((uint32_t)data, 16);
    sp_SetGPIODirectionInput(0x7);   /* GPIO 0 and GPIO 1 and GPIO 2 */
    sp_SetGPIODirectionOutput(0x50);  /* GPIO 4 and GPIO 6 */
    uart_puts(" GPIO dir = ");
    addr = (uint32_t *)(0x30046400);
    data = *addr;
    uart_put_long((uint32_t)data, 16);
    sp_SetGPIODataLow(0x40);  /* GPIO 6 Low for read FPGA Register */
    char_count += sprintf((char *)&(ppb_bootmsg_addr->msg[char_count]), 
                  "Driven GPIO expander bit 4 to high DSP Ready");
    count = 0xFFFFF;
    
    bsp_debug_printf("Setup SPI interface\n");
    spi_init_fpga();

    bsp_debug_printf("\n\r Set up FPGA\n");
    
    board_id = get_oak_id();
    bsp_debug_printf("\n\r Borad ID: %x\n", board_id);
    switch (board_id & FPGA_MB_BOARD_ID_MASK) {
    case BOARD_16FXS_2FXO:
        sku_id = OAKENSHIELD_SKU_16FXS_2FXO;
        break;
    case BOARD_24FXS_4FXO:
        sku_id = OAKENSHIELD_SKU_24FXS_4FXO;
        break;
    case BOARD_8FXS_12FXO:
        sku_id = OAKENSHIELD_SKU_8FXS_12FXO;
        break;
    case BOARD_72FXS:
        sku_id = OAKENSHIELD_SKU_72FXS;
        break;
    case VG400_2FXS_2FXO:
        sku_id = VG400_SKU_2FXS_2FXO;
        break;
    case VG400_4FXS_4FXO:
        sku_id = VG400_SKU_4FXS_4FXO;
        break;
    case VG400_6FXS_6FXO:
        sku_id = VG400_SKU_6FXS_6FXO;
        break;
    case VG400_8FXS:
        sku_id = VG400_SKU_8FXS;
        break;
    case PHOENIX_144FXS:
        sku_id = PHOENIX_SKU_144FXS;
        break;
    case PHOENIX_132FXS_6FXO:
        sku_id = PHOENIX_SKU_132FXS_6FXO;
        break;
    case PHOENIX_84FXS_6FXO:
        sku_id = PHOENIX_SKU_84FXS_6FXO;
        break;
    default:
        bsp_debug_printf("Unknown FPGA board id: 0x%0x\n", board_id);
        return 0;
    }


    /* set up FPGA */
    /* take TDMSW out of reset */
    fpga_unreset_tdmsw();

    /* take TDM PLL out of reset */
    fpga_unreset_tdm_pll();
    msleep(500);

    /* configure TDMPLL to use primary input clock. */
    fpga_config_tdm_pll();
    msleep(800);

    /* check TDMPLL lock status. */
    fpga_check_tdm_pll();

    /* Setup FPGA env */
    fpga_setup(); 


    sp_SetGPIODataHigh(0x10); /* GPIO 4 Hight for ready */
    uart_puts("\r\n Driven GPIO 4 to high DSP Ready ");

    while (count) {
        recv_p = wait_host_msg(SELECT_READY);
        memcpy((uint8_t *)hostif_msg_p, recv_p, sizeof(dspif_ether_t));
        if (recv_p != NULL) {
            setup_dc_info((dspif_ether_t *)recv_p);
            uart_puts("\r\n Received NGSM info in Host READY message \n");
            /* Boot progress 6. SELECT_READY message received */
            char_count += sprintf((char *)&(ppb_bootmsg_addr->msg[char_count]), 
                          "Received NGSM info in Host READY message");
            cmd_status_addr->cur_command_recv = SELECT_READY;
            cmd_status_addr->cur_status = RESULT_SUCCESSFUL;
            break;
        }
        count--;
    }
    if (recv_p == 0) {
        uart_puts("\r\n Did not receive READY message from Host.\n");
        uart_puts("\r\n Using UART interface to run tests. \n");
        uart_puts("\r\n Please reset and try again\n");
        /* Boot progress 6. No SELECT_READY message received */
        char_count += sprintf((char *)&(ppb_bootmsg_addr->msg[char_count]),
                        "Did not receive READY message from Host.\
                        Using UART interface to run tests. Please reset and try again.");
        cmd_status_addr->cur_command_recv = SELECT_READY;
        sprintf((char *)&(cmd_status_addr->cur_msg), "Did not receive READY message from Host.\
                        Using UART interface to run tests. Please reset and try again."); 
        cmd_status_addr->cur_status = RESULT_FAILED;
        dsp_tests_use_enet = 0;
        menu_display = 1;
        
    } else {
        /* send ready */
        /* SR after error in send please select the ppb_diag mode for host to get memory info */
        /* SR also after ready pkt send ready resp with slot info and check subsequent pkts for the correct dsp */
        /* SR check host side code if did not receive ready response from DSP does it time out */
        if (send_host_readymsg()) {
            /* can't send, just save it so debugger can take a look */
            sprintf((char *)&(hd_if->errmsg), "Failed to send ready message, status = %x",
                            (unsigned int)ppb_dss0_if->faults);
            uart_puts((char *)hd_if->errmsg);
            /* Boot progress 7. Cannot send Ready message to Host */
            char_count += sprintf((char *)&(ppb_bootmsg_addr->msg[char_count]),
                            "Cannot send Ready message to Host.");
            dsp_tests_use_enet = 0;
            menu_display = 1;
        } else {
            sprintf((char *)&(hd_if->errmsg), "Done send ready message, status = %x",
                            (unsigned int)ppb_dss0_if->faults);
            /* Boot progress 7. Sent Ready message to Host */
            char_count += sprintf((char *)&(ppb_bootmsg_addr->msg[char_count]), "Sent Ready message to Host.");
            uart_puts((char *)hd_if->errmsg);
        }
    }
    
    /* Set GPIO pins for volatage margin */
    //sp_configureDACGPIO();
    /* Enable the AVS in closed loop mode, check if not in volatage margin */
    avscParam.ckiFreqInMHz = 25;
    avscParam.mcmSlaveFailmode = defaultToVDDmax;
    avscParam.csmStopPoint = 7; /* <= 4 <= 0x28 */
    avscParam.csmUpdateRate = 20; /* should be <= 1320 */

    if ((ret = enable_avs(avscParam))) {
        bsp_debug_printf("\r\n ***Error %d Could not enable AVSC \n", ret);
        char_count += sprintf((char *)&(ppb_bootmsg_addr->msg[char_count]),
                        "No .93V margin. Could not enable AVSC error ");
    } else {
        bsp_debug_printf("\r\n Enable AVSC in closed loop mode\n");
        char_count += sprintf((char *)&(ppb_bootmsg_addr->msg[char_count]), "No .93V margin. AVSC enabled.");
    }


    if (dsp_tests_use_enet == 0) {
        /* delay before menu display */
        count = 0;
        while (count < 300) {
            lsi_mg_delay(WAIT_TIME);
            count++;
        }

        diag_menu();
    } else {
        uart_puts("\r\n Now Waiting to receive command packet from Host\n");
        char_count += sprintf((char *)&(ppb_bootmsg_addr->msg[char_count]), 
                              "Now Waiting to receive command packet from Host.");
        while (1) {
            recv_p = wait_host_msg(0);
            /* if wait_host_msg did not get a msg, return NULL */
            /* SR handle error when recv_p is error because of */
            /* pkt mismatch, send back error to pkt to source */
            if (recv_p) {
                memcpy((uint8_t *)hostif_msg_p, recv_p, sizeof(dspif_ether_t));
                memcpy((uint8_t*)hd_if, &(hostif_msg_p->dspif_info), sizeof(dspif_info_t));
                if (hd_if -> select == DSP_STOP_WHILE_LOOP) {
                    break;
                }
                uart_puts("\r\n Now Process the received message\n");
                /* process request, wait for result can be FAILED/PASSED/NO_REPLY */
                ret = proc_host_msg(hostif_msg_p);
                if ((ret == FAILED) || (ret == PASSED)) {
                    bsp_debug_printf("\r\n ret from proc = %d\n",ret);
                    dspid_p->core_id = core_id;
                    /* send result */
                    if (send_host_testmsg()) {
                        uart_puts("\r\n Failed to send test message");
                        sprintf((char *)&(hd_if->errmsg), 
                                 "Failed to send test message, status = %x", 
                                (unsigned int)ppb_dss0_if->faults);
                        uart_puts((char *)hd_if->errmsg);
                    } else {
                        uart_puts("\r\n Sent host msg command result\n");
                        uart_puts("\r\n Now Waiting to receive command packet from Host\n");
                    }
                } else if (ret == NO_REPLY) {
                    ;
                } 
            } else {
                sprintf((char *)&(hd_if->bufmsg), "Waiting for message...");
            }
        }
        dsp_break_whileloop(); /* stop wait host message and host can console switch to dsp */
    }
    return(PASSED);

}


/***********************************************************************
 *
 * Function: dsp_break_whileloop 
 *
 * Description: DSP stop wait host message and host can console switch  to dsp side.
 *
 * Input : None 
 *
 * Returns: None
 *
 **********************************************************************
 */
int dsp_break_whileloop (void)
{
    int retvl = PASSED;
    sprintf((char *)&(hd_if->bufmsg), "DSP stop wait host message and please console switch.");
    hd_if->result = RESULT_SUCCESSFUL;
    if (send_host_testmsg()) {
        uart_puts("\r\n Failed to send test message, sta");
        sprintf((char *)&(hd_if->errmsg), 
                 "Failed to send test message, status = %x", 
                 (unsigned int)ppb_dss0_if->faults);
        uart_puts((char *)hd_if->errmsg);
        retvl = FAILED;
    } else {
        uart_puts("\r\n Sent host msg command result\n");
    }
    diag_menu();
    return(retvl);
}

/***********************************************************************
 *
 * Function: arm11_cpu1_boot_test 
 *
 * Description: ARM11 CPU1 Boot test
 *
 * Input : None 
 *
 * Returns: PASSED/FAILED
 *
 **********************************************************************
 */
int arm11_cpu1_boot_test (void)
{
    uint32_t arm1, value, count;

    /* Test ARM CPU1 */
    /* first unreset the ARM CPU1 */
    arm1 = 0;
    if (sp_readMPcpuid() == 0) {
        uart_puts("\r\n In CPU0 about to reset CPU1\n");
        sp_ARM1release();
        count = 0;
        uart_puts("\r\n After CPU0 reset CPU1\n");
        value = *cpu1_if;
        bsp_debug_printf("\r\n * 0x%x = 0x%x\n", cpu1_if, value);
        while (*cpu1_if != MAGIC) {
            lsi_mg_delay(WAIT_TIME); /* need to adjust number later */
            if (++count > 50) {
                arm1 = 1;
                bsp_debug_printf("\r\n ***** ARM1 Not Up *****");
                break;
            }
        }
        sp_ARM1reset();
        if (arm1 == 1)
            return (FAILED);
    } else  {
        bsp_debug_printf("\r\n *** Not ARM11 CPU0, cannot run the CPU1 test\n");
        return (FAILED);
    }
    *cpu1_if = 0;
    return (PASSED);

}

int ppb_gpio_test (int sig, int status)
{
    volatile uint32_t *addr;
    int i, data;

    if (status & SET_LEVEL) { /* Output signal */
        sp_SetGPIODirectionOutput(sig);   
        for (i=0;i<100;i++)
        data = i;
        if (status == SET_HIGH) {
            uart_puts("\r\n set high and Output direction for sig ");
            uart_put_long((uint32_t)sig, 16);
            sp_SetGPIODataHigh(sig);
        } else {
            uart_puts("\r\n set low and Output direction for sig ");
            uart_put_long((uint32_t)sig, 16);
            sp_SetGPIODataLow(sig);
        }
    } else {
        uart_puts(" GPIO dir = ");
        addr = (uint32_t *)(0x30046400);
        data = *addr;
        uart_put_long((uint32_t)data, 16);
        sp_SetGPIODirectionInput(sig);
        uart_puts("\r\n Set Input direction for sig ");
        uart_put_long((uint32_t)sig, 16);
        uart_puts(" status = ");
        uart_put_long((uint32_t)status, 16);
        uart_puts(" GPIO dir = ");
        addr = (uint32_t *)(0x30046400);
        data = *addr;
        uart_put_long((uint32_t)data, 16);
        for (i=0;i<100;i++)
            data = i;
        addr = (uint32_t *)(0x30046000+0x3fc);
        data = *addr;
        uart_puts("\r\n GPIO data register = ");
        uart_put_long((uint32_t)data, 16);
        data = sp_GetGPIOData(sig);
        uart_puts("\r\n GPIO data register = ");
        uart_put_long((uint32_t)data, 16);
        if (status & 1) /* check high */ {
            if (data & sig) {
                uart_puts("\r\n Result passed read gpio data = ");
                uart_put_long((uint32_t)(data), 16);
                return (PASSED);
            } else {
                uart_puts("\r\n Result failed read gpio data = ");
                uart_put_long((uint32_t)(data), 16);
                return (FAILED);
            }
        } else  { /* check low */
            if (data) {
                uart_puts("\r\n Result failed read gpio data = ");
                uart_put_long((uint32_t)(data), 16);
                return (FAILED);
            } else {
                uart_puts("\r\n Result passed read gpio data = ");
                uart_put_long((uint32_t)(data), 16);
                return (PASSED);
            }
        }
    } 
    return (PASSED);
}

/***********************************************************************
 *
 * Function: dsp_test_intf_sync_sig 
 *
 * Description: Test SYNC signals from the NGSM interface  
 *
 * Input : DSS Core # 
 *
 * Returns: PASSED/FAILED
 *
 **********************************************************************
 */
int dsp_test_intf_sync_sig (int sig, int status)
{
    int gpio_pin, count, timeout, test;

    uart_put_long((uint32_t)sig, 10);

    uart_puts("\r\n In dsp_test_intf_sync_sig for signal ");
    uart_put_long((uint32_t)sig, 16);
    uart_puts(" status ");
    uart_put_long((uint32_t)status, 16);
    switch (sig) {
    case SYNC_TRIG_OUT:
    case SYNC_OUT:
        /* The GPIO signal is PPB GPIO */
    uart_puts("\r\n In ppb_gpio_test \n");
        bsp_debug_printf("\r\n Calling ppb_gpio_test for signal %d, status 0x%x\n", sig, status);
        return (ppb_gpio_test(sig, status));
        break;
    case SYNC_IN: 
        bsp_debug_printf("\r\n Calling DSS 1 for signal %d, status 0x%x\n", sig, status);
        gpio_pin = sig - 0x7fe; hd_if = (dspif_info_t *)0xC02FEC00; 
        break;
    case SYNC_OUT1: 
        bsp_debug_printf("\r\n Calling DSS 2 for signal %d, status 0x%x\n", sig, status);
        gpio_pin = sig - 0x1ffe; hd_if = (dspif_info_t *)0xC02FF000; break;
    case SYNC_TRIG_IN: 
        bsp_debug_printf("\r\n Calling DSS 3 for signal %d, status 0x%x\n", sig, status);
        gpio_pin = sig - 0x7ffe; hd_if = (dspif_info_t *)0xC02FF400; break;
    default: hd_if = (dspif_info_t *)0xC02FE800;break;
    }
        /* Build the command */
    hd_if->result = RESULT_RUNNING;
    hd_if->flags = FLAG_NULL;
    hd_if->select = SELECT_INTF_SYNC;
    test = hd_if->select;
    hd_if->faults = 0;
    hd_if->location = 0;
    hd_if->expected = 0;
    hd_if->actual = 0;
    hd_if->extra = 0;
    hd_if->errorcount = 0;
    hd_if->testcounter = 0;
    hd_if->ReadyOnTest = 0;
    hd_if->TestCtrl = 0;
    hd_if->WhoAmI = 0;
    hd_if->ver_no = 0;
    hd_if->wait_states = 0;
    hd_if->param1 = gpio_pin;
    hd_if->param2 = status;
    hd_if->param3 = 0;
    hd_if->param4 = 0;
    memset((hd_if->bufmsg), 0, 128);
    memset((hd_if->errmsg), 0, 128);

    /* set up everything and then tell core: GO */
    hd_if->command = CMD_RUN;
    count = 0;
    timeout = 100;
    while (hd_if->result == RESULT_RUNNING) {
        sprintf((char *)&(hd_if->errmsg[64]), "wating for result = 0x%x\n", (unsigned int)hd_if->result);
        uart_puts(".");
        if (++count > timeout) {
            lsi_mg_delay(WAIT_TIME);
            break;
        }
        lsi_mg_delay(20);
    }
    if (count >= timeout) {
        sprintf((char *)&(hd_if->errmsg[64]), "DSP SYNC signal test failed for signal %d level %d ", sig, status);
        uart_puts((char *)hd_if->errmsg);
        cterr('f', 0, (char *)hd_if->errmsg);
        return (FAILED);
    }
    hd_if->select = test;
    uart_puts("\r\n DSP SYNCsignal test passed for signal ");
    uart_put_long((uint32_t)sig, 10);
    uart_puts(" level ");
    uart_put_long((uint32_t)status, 10);
    return (PASSED); /* got something back */

}

/***********************************************************************
 *
 * Function: dss_core_sanity 
 *
 * Description: DSS Core Sanity test
 *
 * Input : DSS Core # 
 *
 * Returns: PASSED/FAILED
 *
 **********************************************************************
 */
int dss_core_sanity (int dss_core)
{
    int count, timeout, test, sp2700;
    volatile dspif_info_t *save_p;

    uart_put_long((uint32_t)dss_core, 10);

    uart_puts("\r\n In dss_core_sanity for core ");
    uart_put_long((uint32_t)dss_core, 10);

    save_p = hd_if;

    switch (dss_core) {
    case DSS_CORE0: hd_if = (dspif_info_t *)0xC02FE800; break;
    case DSS_CORE1: hd_if = (dspif_info_t *)0xC02FEC00; break;
    case DSS_CORE2: hd_if = (dspif_info_t *)0xC02FF000; break;
    case DSS_CORE3: hd_if = (dspif_info_t *)0xC02FF400; break;
    default: hd_if = (dspif_info_t *)0xC02FE800;break;
    }

    /* Build the command */
    hd_if->result = RESULT_RUNNING;
    hd_if->flags = FLAG_NULL;
    test = hd_if->select;
    hd_if->select = SELECT_DSP_SANITY;
    hd_if->faults = 0;
    hd_if->location = 0;
    hd_if->expected = 0;
    hd_if->actual = 0;
    hd_if->extra = 0;
    hd_if->errorcount = 0;
    hd_if->testcounter = 0;
    hd_if->ReadyOnTest = 0;
    hd_if->TestCtrl = 0;
    hd_if->WhoAmI = 0;
    hd_if->ver_no = 0;
    hd_if->wait_states = 0;
    hd_if->param1 = 0;
    hd_if->param2 = 0;
    hd_if->param3 = 0;
    hd_if->param4 = 0;
    memset((hd_if->bufmsg), 0, 128);
    memset((hd_if->errmsg), 0, 128);

    /* set up everything and then tell core: GO */
    hd_if->command = CMD_RUN;
    count = 0;
    timeout = 100;
    REG32_READ(0x9801226C, sp2700);
    if ((sp2700 == PFUSE123_SP2702) && (dss_core > 1)) {
        hd_if->select = test;
        uart_puts("\r\n DSP Sanity test Passed for Core ");
        uart_put_long((uint32_t)dss_core, 10);
        hd_if = save_p;
        return (PASSED); /* got something back */
    }
    while (hd_if->result == RESULT_RUNNING) {
        sprintf((char *)&(hd_if->errmsg[64]), "wating for result = 0x%x\n", (unsigned int)hd_if->result);
        uart_puts(".");
        if (++count > timeout) {
            lsi_mg_delay(WAIT_TIME);
            break;
        }
        lsi_mg_delay(20);
    }
    if (count >= timeout) {
        uart_puts("\r\n DSP Sanity test Failed for Core ");
        uart_put_long((uint32_t)dss_core, 10);
        uart_puts((char *)hd_if->errmsg); 
        cterr('f', 0, (char *)hd_if->errmsg);
        hd_if = save_p;
        return (FAILED);
    }
    hd_if->select = test;
    uart_puts("\r\n DSP Sanity test Passed for Core ");
    uart_put_long((uint32_t)dss_core, 10);

    hd_if = save_p;

    return (PASSED); /* got something back */
}


/***********************************************************************
 *
 * Function: boot_hpi_jump_entry
 *
 * Description: ARM code main entry point 
 *
 * Input : none 
 *
 * Returns: none
 *
 **********************************************************************
 */
void
__attribute__ ((section(".bootjump")))
boot_hpi_jump_entry (void)
{
    //_diag_ppb_start();
    _start();
}

/******** History ********
$Log: diag_ppb_main.c,v $
Revision 1.5  2021/04/15 00:53:07  achiu2
[PRRQ:CSCvx56970-2]Phoenix code review for ER

Revision 1.4  2018/08/30 06:39:42  haohsu
Collapse Vg400-branch to Main Trunk

Revision 1.3.16.1  2018/02/06 09:26:17  haohsu
Code change for VG400

Revision 1.3  2017/09/15 06:02:52  harrchan
Add error message for margin voltage (CSCvf79330)

Revision 1.2  2017/07/28 07:58:51  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.1.2.1  2017/06/29 08:14:38  harrchan
Initial commit code for Oakenshield

Revision 1.16.78.6  2017/04/26 01:58:29  harrchan
Optimize oakenshield  FXS calibration

Revision 1.16.78.5  2017/03/09 07:23:34  harrchan
Support oakenshield double wide case

Revision 1.16.78.4  2017/02/20 08:32:03  olin2
Add show Voltage Margin utility

Revision 1.16.78.3  2017/02/09 06:41:05  olin2
Support voltage margin and fail over port utility

Revision 1.16.78.2  2017/01/17 05:07:06  olin2
Clean up debug code

Revision 1.16.78.1  2016/12/14 04:57:38  olin2
Initial commit code for Oakenshield

Revision 1.16  2013/02/28 00:40:19  srane
Add ethernet bd and buffer display utility.

Revision 1.15  2012/12/24 00:10:36  srane
Support the NGVM interface SYNC signal test, firmware version host
command and eeprom read/write utility.

Revision 1.14  2012/10/04 23:37:00  srane
Add support for SP2702. Boot up debug messages.

Revision 1.13  2012/10/02 23:17:00  srane
Add version display for diags firmware. Add "ver" command.

Revision 1.12  2012/09/24 01:16:52  srane
Print error message if AVS closed loop mode setup fails.

Revision 1.11  2012/09/10 06:44:27  srane
Add dsp memory display pkt xfer code, ARM11 CPU1 test, cleanup.

Revision 1.10  2012/08/28 18:20:52  srane
Add DAC support for .93V as well (cannot use AVS).

Revision 1.9  2012/08/15 15:09:41  srane
Add support for EMAC1 looback test, commands for eeprom rd/wr.

Revision 1.8  2012/07/17 20:46:07  srane
Add GPIO I2C support, use ethernet to send/receive command/result to the
host. General cleanup.

Revision 1.7  2012/06/28 21:25:56  srane
fix TDM isr, add delay for ethernet loopback etc

Revision 1.6  2012/06/07 23:15:56  srane
fix compile error.

Revision 1.5  2012/06/07 22:50:59  srane
TDM external loopback, ECC memory test

Revision 1.4  2012/05/31 13:50:01  srane
Add util to display tdm registers abd buffers.

Revision 1.3  2012/05/24 23:25:38  srane
Add GPIO code to set ready bit, uart test, support both
uart mode and ethernet mode, other cleanup

Revision 1.2  2012/05/10 22:57:58  srane
Add TDM support. Adjust the linker sections.

Revision 1.1  2012/04/18 09:44:02  srane
Initial checkin


$Endlog$
*/

