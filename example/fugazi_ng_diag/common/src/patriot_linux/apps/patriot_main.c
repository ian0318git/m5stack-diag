/* $Id: patriot_main.c,v 1.1 2014/03/25 02:12:34 huanngo Exp $
 * $Source: 
 *******************************************************************************
 * File Name: patriot_main.c
 *
 * Description: Patriot module side main file
 *
 *      
 *
 * Copyright (c)2011 - 2014 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *******************************************************************************
 */

/* Includes. */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "patriot_main.h"
#include "router_if.h"
#include "sgmii_defs.h"
#include "p1021_etsec.h"
#include "p1021_immap.h"
#include "patriot_intr.h"
#include <sys/select.h>

//#define DEBUG 1

extern fe_packet_t *tx_packet_p;
extern tsec_info_struct_t *etsec_init_tsec (int etsec_num);
extern int patriot_receive_frames(int etsec_num, tsec_info_struct_t *tsec_p,
				  int mode);

signed int p1021_fd = -1;
static signed int mem_fd = -1;
volatile void *patriot_ccsr_base = NULL;
volatile void *patriot_ddr_base = NULL;
unsigned char param_arr[6];
unsigned char param_cmd_menu;

long int ADRSPC_PQUICC_IMEMB;
long int VIR_ADRSPC_RAM;
int param;
unsigned char to_host_param[10];

mac_addr_t module_mac_addr;
mac_addr_t host_mac_addr;

boolean n2g_flag;

typedef int (*PFI)();

typedef struct patriot_cmd_table {
    ushort cmd;
    PFI routine;
    int param;
} patriot_cmd_table_t;

uchar err_msg[1024];
uchar err_msg1[1024];
uchar err_msg2[1024];
uchar err_msg3[1024];
uchar dismem_msg[1024];

/*
 *  Table of commands received from host platform and the routines to
 *  be executed for the command.
 */
static patriot_cmd_table_t patriot_cmd_table[] = {
    {FROM_HOST_CPU_ALIVE_TEST, patriot_cpu_alive_test, 0},
    {FROM_HOST_READ_FPGA_VERSION, patriot_read_fpga_version, 0},
    {FROM_HOST_FPGA_DOWNLOAD_TO_FPGA, patriot_fpga_download_to_fpga, 0},
    {FROM_HOST_FPGA_REG_TEST, patriot_fpga_reg_test, 0},
    {FROM_HOST_MEMORY_TEST, patriot_memory_test, 0},
    {FROM_HOST_SPI_PROM_TEST, patriot_spi_prom_test, 0},
    {FROM_HOST_DS3170_REG_TEST, patriot_ds3170_reg_test, 0},
    {FROM_HOST_E3_AIS_TEST, patriot_clear_e3_ais_test, 0},
    {FROM_HOST_CLR_T3_INTR_TEST, patriot_clear_t3_intr_test, 0},
    {FROM_HOST_CLR_T3_BERT_TEST, patriot_clear_t3_bert_test, 0},
    {FROM_HOST_FREESCALE_LPBK_TEST, patriot_fs_lpbk_test, 0},
    {FROM_HOST_CLR_T3_LPBK_TEST, patriot_clear_t3_int_lpbk_test, 0},
    {FROM_HOST_CLR_T3_EX_LPBK_TEST, patriot_clear_t3_ext_lpbk_test, 0},
    {FROM_HOST_SUB_T3_LPBK_TEST, patriot_subrate_t3_int_lpbk_test, 0},
    {FROM_HOST_SUB_T3_EX_LPBK_TEST, patriot_subrate_t3_ext_lpbk_test, 0},
    {FROM_HOST_CLR_E3_LPBK_TEST, patriot_clear_e3_int_lpbk_test, 0},
    {FROM_HOST_CLR_E3_EX_LPBK_TEST, patriot_clear_e3_ext_lpbk_test, 0},
    {FROM_HOST_SUB_E3_LPBK_TEST, patriot_subrate_e3_int_lpbk_test, 0},
    {FROM_HOST_SUB_E3_EX_LPBK_TEST, patriot_subrate_e3_ext_lpbk_test, 0},
    {FROM_HOST_FREESCALE_MAC_LPBK_TEST, patriot_mac_lpbk_test, 0},
    {FROM_HOST_FPGA_LPBK_TEST, patriot_fpga_lpbk_test, 0},
    {FROM_HOST_FPGA_RESET, patriot_fpga_reset, 0},
    {FROM_HOST_DS3170_RESET, patriot_ds3170_reset, 0},
    {FROM_HOST_LED_DISPLAY, patriot_display_led, 0},
    {FROM_HOST_SWITCH_CONSOLE, patriot_switch_console, 0},
    {FROM_HOST_FREESCALE_UCC_LPBK_TEST, patriot_fs_ucc_lpbk_test, 0},
    {FROM_HOST_FPGA_GPIO_FRAMER_GPIO_TEST, patriot_test_fpga_gpio_framer, 0},
    {FROM_HOST_WRITE_MAC_ADDR, patriot_write_mac_addr, 0},
    {FROM_HOST_CPU_GPIO_TEST_IO1_W1, patriot_host_to_module_gpio1_wr1_test, 0},
    {FROM_HOST_CPU_GPIO_TEST_IO1_W0, patriot_host_to_module_gpio1_wr0_test, 0},
    {FROM_HOST_CPU_GPIO_TEST_IO4_W1, patriot_host_to_module_gpio4_wr1_test, 0},
    {FROM_HOST_CPU_GPIO_TEST_IO4_W0, patriot_host_to_module_gpio4_wr0_test, 0},
    {FROM_HOST_CPU_GPIO_TEST_IO3_R1, patriot_module_to_host_gpio3_rd1_test, 0},
    {FROM_HOST_CPU_GPIO_TEST_IO3_R0, patriot_module_to_host_gpio3_rd0_test, 0},
    {FROM_HOST_FPGA_INTR_TEST, patriot_fpga_intr_test, 0},
    {FROM_HOST_SUB_T3_IND_LPBK_TEST, patriot_subrate_t3_individual_int_lpbk_test,0},
    {FROM_HOST_SUB_T3_IND_EX_LPBK_TEST, patriot_subrate_t3_individual_ext_lpbk_test,0},
    {FROM_HOST_SUB_E3_IND_LPBK_TEST, patriot_subrate_e3_individual_int_lpbk_test,0},
    {FROM_HOST_SUB_E3_IND_EX_LPBK_TEST, patriot_subrate_e3_individual_ext_lpbk_test,0},
    {FROM_HOST_ECC_TEST, patriot_ddr_ecc_single_bit_err_test,0},
    {FROM_HOST_UART_TEST, patriot_uart_test,0},
    {FROM_HOST_GE0_LPBK_TEST, patriot_ge0_loopback_test, 0},
    {FROM_HOST_POWER_ALTER_NO_MARGIN, patriot_power_no_margin, 0},
    {FROM_HOST_POWER_ALTER_LOW_MARGIN, patriot_power_margin_low, 0},
    {FROM_HOST_POWER_ALTER_HIGH_MARGIN, patriot_power_margin_high, 0},
    {FROM_HOST_UPGRADE_FPGA_DOWNLOAD_TO_SPI_PROM,
     patriot_upgrade_fpga_download_to_spi_prom, 0},
    {FROM_HOST_GOLDEN_FPGA_DOWNLOAD_TO_SPI_PROM,
     patriot_golden_fpga_download_to_spi_prom, 0},
    {FROM_HOST_FPGA_READ_INFO, patriot_dump_fpga_info_to_host, 0},
    
};
#define NUM_CMDS_FRM_HOST sizeof(patriot_cmd_table)/sizeof(patriot_cmd_table_t)


/***********************************************************************
 *
 * Function:	patriot_setup_eth_dev()
 *
 * Description: Setup the etsec on the Freescale processor to allow
 *		the generation/transmission/reception of ethernet
 *		packets.
 *
 * Input:	etsec_num - ETSEC number
 *
 * Output:	PASSED if there are no initialization errors
 *		FAILED if there are initialization errors
 *
 ************************************************************************
 */
int
patriot_setup_eth_dev (int etsec_num, int lpbk_opt)
{

    /*
     * initialize the etsec for Ethernet operation
     * default preamble, full duplex, append CRC, 1000Mbps, flow control
     */
    if (etsec_init(etsec_num, lpbk_opt, POLL_MODE, TRUE) == FAILED) {
        return (FAILED);
    }
    /* start the etsec Tx/Rx DMA */
    etsec_start(etsec_num, TRUE);

    return (PASSED);
}



/*********************************************************************
 *
 * Function: decode_cmd
 * Description: Decodes the command from the host executes 
 *              the appropriate test.
 * Inputs: None
 * Outputs: PASSED/FAILED
 *********************************************************************
 */
static int
decode_cmd (uchar host_cmd)
{
    uint i, cmd_not_found;
    patriot_cmd_table_t *patriot_cmd_table_p;
    int ret_val = FAILED;

#ifdef DEBUG
	printf("\n ENTERED DECODE_CMD, host_cmd = 0x%08x \n", host_cmd);
#endif
    /*
     *  Init for command search and initialize timer for use in delays.
     */
    patriot_cmd_table_p = patriot_cmd_table;
    cmd_not_found = TRUE;
    
    /*
     *  Search command table for the command that was received.
     */
    i = 0;
#ifdef DEBUG
    printf("NUM_CMDS_FRM_HOST = %#x\n", NUM_CMDS_FRM_HOST);
#endif
    while (( i++ < NUM_CMDS_FRM_HOST) && cmd_not_found ) {
#ifdef DEBUG
	printf("patriot_cmd_table_p = %#x\n", (unsigned int)patriot_cmd_table_p);
#endif
	if ( patriot_cmd_table_p->cmd == host_cmd ) {
	    ret_val = (patriot_cmd_table_p->routine)();
	    cmd_not_found = FALSE;
	    break;	  
	}
	patriot_cmd_table_p++;
    }

    /*
     *  If the command was not found, return msg to host that command
     *  could not be decoded.
     */
    if (cmd_not_found == TRUE) {
	ret_val = CMD_NOT_FOUND;
    }

    return ret_val;
}

/*******************************************************************************
 *
 * Function    : config_pcie
 * Description : Configures PCIe interface for RC 
 * Inputs      : None
 * Outputs     : PASSED/FAILED 
 *
 *******************************************************************************
 */
static int config_pcie (void)
{

    /*
     * Set PCIe ATMU Inbound registers
     * Inbound window 1
     * needs to set registers: pexiwbear, pexiwbar, pexitar, and pexiwar.
     */
    printf("\nConfig PCIE windows\n");
    
    /*
     * Inbound Window 1:
     *
     * For CPU CCSRBAR accesses 
     * Set this window for access to CPU CCSRBAR to work 
     * We should not be setting a window for it, it should work,
     * if PEXCCSRBAR (offset 0x10 in the RC config space) is set 
     * to CCSRBAR base. However, setting PEXCCSBAR is not working.
     * FSL is investigating. The work around for it to set up a
     * inbound window.
     * 
     * DEIN is not set
     */

    REGB->im_pcie[0].pexiwbar1 = (PCIE_MAP_CCSRBAR_ADDR >>
				  MPC8500_PIWBAR_BA_SHIFT);
    REGB->im_pcie[0].pexitar1 = (ADRSPC_PQUICC_IMEMB >>
				 MPC8500_PITAR_TA_SHIFT);
    REGB->im_pcie[0].pexiwar1 = (MPC8500_PIWAR_TGI_LM |
				 MPC8500_PIWAR_RTT_RDSN |
				 MPC8500_PIWAR_WTT_WRSN |
				 MPC8500_PIWAR_IWS_4M);

    /*
     * Enable Inbound windows
     * needs to set the enable addr translation bit in 
     * PCIe Inbound Window Attributes Registers (PIWAR).
     */
    REGB->im_pcie[0].pexiwar1 |= MPC8500_PIWAR_EN;

}


/*********************************************************************
 * Function: patriot_init_eth_intr_count
 * Description: This function initializes the interrupt count number
 * Inputs: None
 * Outputs: None
 *********************************************************************
 */
void patriot_init_eth_intr_count(void)
{

    sm_patriot_eth_intr_iface_t intr;

    memset(&intr, 0, sizeof(intr));

    if (write(p1021_fd, &intr, sizeof(intr))<0) {
        perror("Unanble to init interrupt count\n");
        fflush(0);
        exit(0);
    }
    return;
}


/*********************************************************************
 * Function: patriot_get_eth_intr_count
 * Description: This function get the interrupt count number
 * Inputs: None
 * Outputs: None
 *********************************************************************
 */
void patriot_get_eth_intr_count(void)
{
    sm_patriot_eth_intr_iface_t intr;

    memset(&intr, 0, sizeof(intr));
    if (read(p1021_fd, &intr, sizeof(intr))<0) {
        perror("Unable to get interrupt count numbers");
        fflush(0);
        exit(0);
    }

    etsec_recv_nframes[ETSEC2 - 1] = intr.eth_rx_intr_cnt;
	
    return;
}

/* Convert a string of MAC address "xx:xx:xx:xx:xx:xx" to
 * 6 byte uchar numbers
 */
int macstr2macaddr(char *macstr, mac_addr_t *mac_buf)
{
    char tmp_mac[6];
    char *cptr, tmpstr[4];
    int ii, tmp_hex, count;

    ii = 0;
    count = 0;
    cptr = macstr;
    do {
      memset(tmpstr, 0, sizeof(tmpstr));
      memcpy(tmpstr, cptr, 2);
      count += sscanf((char *)tmpstr, "%x", &tmp_hex);
      tmp_mac[ii] = (uchar)tmp_hex;
      ii++;
      cptr += 3; /* point to next mac byte */
    } while(ii < 6);

    if (count == 6) {
        memcpy(mac_buf, tmp_mac, sizeof(mac_addr_t));
	return(0);
    }
    else {
        return(-1);
    }
}



/*
 * Function: system_eth_mac_addr_get
 *   Get the cavecreek ethernet port mac address from the linux file.
 *
 * Input:
 *   port - cavecreek sgmii port number
 *   mac_buf - pointer to buffer to store the mac address
 *
 * Return: void
 */
static void system_eth_mac_addr_get(int port, mac_addr_t *mac_buf)
{
    char file_name[] = "/sys/class/net/eth1/address";
    FILE *stream_p;
    uchar macstr[] ="00:01:02:03:04:05";

    sprintf(file_name, "/sys/class/net/eth%d/address", port);

    stream_p = fopen(file_name, "r");
    fscanf(stream_p, "%s", macstr);
    fclose(stream_p);

    macstr2macaddr(macstr, mac_buf);
}

/*
 * Function: diag_firmware_version_get
 *   Get the diag firmware from the file version
 *
 * Input:
 *   ver_buf - pointer to buffer to store the version
 *
 * Return: void
 */
static void diag_firmware_version_get(unsigned char *ver_buf)
{
    char file_name[] = "/lib/modules/2.6.32.36.cge/kernel/drivers/cisco/userspace/version";
    FILE *stream_p;

    stream_p = fopen(file_name, "r");
    fscanf(stream_p, "%s", ver_buf);
    fclose(stream_p);

}



/*********************************************************************
 * Function: main
 * Description: This is the main entry to user app.
 * Note: Efforts have been made to run the ethernet send & receive
 * in INTR_MODE but there is a problem of synchronization between
 * the host and the module. If the host send a packet while the module
 * has not finished a previous test then the next test is not excecuted
 * and stay in infinite loop.
 * If we pass parameter INTR_MODE in the function call
 * patriot_receive_frames() in main() and patriot_setup_eth_dev() and
 * test the CPU Alive Test, it will work fine.
 * For the patriot_fs_lpbk_test() test to work, the
 * host must wait for 1000ms before sending each packets
 * 
 * Inputs: None
 * Outputs: PASSED/FAILED
 *********************************************************************
 */
int main(int argc, char *argv[])
{

    unsigned char ret_val, ch = '\0', ver_buf[] = "00.00.00";
    int i, etsec_num;
    volatile unsigned char type, cmd, temp;
    unsigned char buf[64], buf1[64], read_buf[1600];
    unsigned long ptr, ptr1;
    tsec_info_struct_t *tsec_p;
    unsigned char *rd_ptr;
    tsec_bd_t  *tx_bd, *rx_bd, *rx_bd_vir_addr;
    fe_packet_t *rx_pak;
    unsigned long vir_ptr, phy_ptr;
    patriot_msg_t msg;

    diag_firmware_version_get(ver_buf);

    printf("\nDiagnostic Firmware Version: %s\n", ver_buf);fflush(0);
    
    sprintf(buf, "/dev/p1021");
    if(p1021_fd >= 0) {
	close(p1021_fd);
    }

    p1021_fd = open(buf, O_RDWR);
    if (p1021_fd < 0) {
	perror("DEVICE p1021: ERROR - device open failed.\n");
	return (-1);
    }

    patriot_ccsr_base = mmap(NULL, 0x100000, PROT_READ | PROT_WRITE, 
			     MAP_SHARED, p1021_fd, 0xFFE00000 );
    
    if (patriot_ccsr_base == MAP_FAILED) {
	close(p1021_fd);
	perror("Error mapping the memory chunk");
	return (-1);
    }

    ADRSPC_PQUICC_IMEMB = (unsigned long)patriot_ccsr_base;
#ifdef DEBUG
    printf("\npatriot_ccsr_base = 0x%08x", (unsigned long)patriot_ccsr_base);

    ptr1 = ((unsigned long)patriot_ccsr_base + 0xB0028);
    printf("\nptr1 = 0x%08x\n", (unsigned long)ptr1);
    printf("\n*ptr1 = 0x%08x\n", *(unsigned int*)ptr1);
    *(unsigned int*)ptr1 = 0xABCD;
    printf("\nptr1 = 0x%08x\n", (unsigned long)ptr1);
    printf("\n*ptr1 = 0x%08x\n", *(unsigned int*)ptr1);
#endif

    sprintf(buf, "/dev/mem");
    if(mem_fd >= 0) {
	close(mem_fd);
    }

    mem_fd = open(buf, O_RDWR);
    if (mem_fd < 0) {
	perror("DEVICE /dev/mem: ERROR - device open failed.\n");
	return (-1);
    }
    
    patriot_ddr_base = mmap(NULL, 0x10000000, PROT_READ | PROT_WRITE, 
			    MAP_SHARED, mem_fd, 0x00000000 );

    VIR_ADRSPC_RAM = (unsigned long)patriot_ddr_base;
    
#ifdef DEBUG
    ptr1 = ((unsigned long)patriot_ddr_base + 0x300);
    printf("\nptr1 = 0x%08x\n", (unsigned long)ptr1);
    printf("\n*ptr1 = 0x%08x\n", *(unsigned int*)ptr1);    
    *(unsigned int*)ptr1 = 0xABCD;
    printf("\n*ptr1 = 0x%08x\n", *(unsigned int*)ptr1);
#endif
    
#ifdef DEBUG
    printf("\nREGB->im_gur.pmuxcr = 0x%08x\n", REGB->im_gur.pmuxcr);
#endif    
    REGB->im_gur.pmuxcr &= ~0x60000000;
    REGB->im_gur.pmuxcr |= 0x0000A040;
    patriot_spi_prom_init();
    ds3170_init_espi();
    config_pcie();
    
    /* Set pin 3 of PCA9557 to 1 to let the host know kernel is up */
    /* Rev 2C or earlier uses pin PA24 */
    /* PA24 as output and GPIO */
    /* clear direction bits for PA24 */
    REGB->im_gur.cpdir2a &= ~(MPC8500_CPDIR2_INOUT(24));
    /* PA24 as outtput */
    REGB->im_gur.cpdir2a |=  MPC8500_CPDIR2_OUT(24);
    /* clear function bits for PA24 and set it as GPIO */
    REGB->im_gur.cppar2a &= ~(MPC8500_CPPAR2(24, 0x3));
    /* Set the pin to 1 */
    REGB->im_gur.cpddata |= 0x00000080;

    printf("\nREGB->im_gur.cpddata = 0x%08x\n",  REGB->im_gur.cpddata);fflush(0);
    if (REGB->im_gur.cpddata & 0x0000200) {
	n2g_flag = TRUE;
    } else {
	n2g_flag = FALSE;
    }
    
    /* Rev 2D or later uses pin PB4 */
    /* PB4 as output and GPIO */
    /* clear direction bits for PB4 */
    REGB->im_gur.cpdir1b &= ~(MPC8500_CPDIR1_INOUT(4));
    /* PB4 as outtput */
    REGB->im_gur.cpdir1b |=  MPC8500_CPDIR1_OUT(4);
    /* clear function bits for PB4 and set it as GPIO */
    REGB->im_gur.cppar1b &= ~(MPC8500_CPPAR1(4, 0x3));
    /* Set the pin to 1 */
    REGB->im_gur.cpddatb |= 0x08000000;


#ifdef DEBUG    
    printf("\nPA24 REGB->im_gur.cpdir2a = 0x%08x", REGB->im_gur.cpdir2a);
    printf("\nPA24 REGB->im_gur.cppar2a = 0x%08x", REGB->im_gur.cppar2a);
    printf("\nPA24 REGB->im_gur.cpddata = 0x%08x\n", REGB->im_gur.cpddata);
#endif
    
    /* If submenu */
    if ((argc > 1) && (strcmp(argv[1], "menu") == 0)) {
	param_cmd_menu = PATRIOT_MENU;
	patriot_submenu();
	return (PASSED);
    }
    if (n2g_flag) {
	etsec_num = ETSEC2;
    } else {
	etsec_num = ETSEC3;
    }
    printf("etsec_num = %d\n", etsec_num);fflush(0);

    system_eth_mac_addr_get(0, &module_mac_addr);
#ifdef DEBUG
    printf("Module MAC address %02x:%02x:%02x:%02x:%02x:%02x",
	   module_mac_addr[0],
	   module_mac_addr[1],
	   module_mac_addr[2],
	   module_mac_addr[3],
	   module_mac_addr[4],
	   module_mac_addr[5]);
    fflush(0); 
#endif	       
    
    if (patriot_setup_eth_dev(etsec_num, SGMII_LPBK_NONE)) {
	return (FAILED);
    }

#ifdef DEBUG_SEND_A_PACKET  
    tsec_p = (tsec_info_struct_t *)etsec_get_info_ptr(etsec_num);
    while (1) {
	if (patriot_receive_frames(etsec_num, tsec_p, POLL_MODE) == PASSED) {
	    printf("\nRX frame\n");
	    break;
	}
    }
    for (i = 0; i < 10; i++) {
	printf("\nSend a packet\n");
	getchar();
	while (1) {
	    patriot_send_a_packet();
	}
    }
#endif    

    tsec_p = (tsec_info_struct_t *)etsec_get_info_ptr(etsec_num);

    while (1) {
	if (patriot_receive_frames(etsec_num, tsec_p, POLL_MODE) == PASSED) {
	    rx_bd = (tsec_bd_t *)etsec_get_rxbd(tsec_p);
#ifdef DEBUG	    
	    printf("\n%s %d: rx_bd = 0x%08x", __FUNCTION__, __LINE__, rx_bd);
#endif	    
	    rx_bd_vir_addr = (tsec_bd_t *)vir_addr(rx_bd);
	    rd_ptr = (unsigned char *)vir_addr(rx_bd_vir_addr->buf_ptr);

	    memset((uchar *)read_buf, 0, 1600);

	    for (i = 0; i < rx_bd_vir_addr->length; i++) {
		read_buf[i] = *rd_ptr;
		rd_ptr++;
	    }
	    memset((uchar *)rd_ptr, 0, MAX_RX_BUF);
	    
	    /* mark frame as processed */
	    rx_bd_vir_addr->status &= ~PQUICC_BDSTAT_RX_RO1;
	    rx_bd_vir_addr->status |= PQUICC_BDSTAT_RX_EMPTY;
	    /*
	     * if wrap occurs, we must re-initialize the tx and rx
	     * buffer descriptors so that we can Tx/Rx more frames
	     */
	    ret_val = check_for_bd_wrap(etsec_num);
	    /* If fail to reinitialize the tx & rx BD's, quit */
	    if (ret_val ==  FAILED) {
		memset((uchar *)tx_packet_p, 0, sizeof(fe_packet_t));
		tx_packet_p->data[0] = TO_HOST_REINIT_TX_RX_FAIL;
		if (patriot_send_frames(etsec_num, tsec_p, POLL_MODE)) {
		    printf("\nFailed to send frames to host\n");
		    break;
		}
		break;
	    }
	   
	    rx_pak = (fe_packet_t *)&read_buf[0];
	    type = rx_pak->data[0];
	    /* If it's a command, send ACK first */
	    if (type != PATRIOT_CMD) {
		printf("\nNot a valid command type = %d\n", type);
		continue;
	    }

	    cmd = rx_pak->data[1];
	    if ((cmd == FROM_HOST_SUB_T3_IND_LPBK_TEST) ||
		(cmd == FROM_HOST_SUB_T3_IND_EX_LPBK_TEST) ||
		(cmd == FROM_HOST_SUB_E3_IND_LPBK_TEST) ||
		(cmd == FROM_HOST_SUB_E3_IND_EX_LPBK_TEST)) {
	    	printf("\n Subrate Individual test (module)\n");
		for (i = 0; i < 4; i++) {
		    param_arr[i] = rx_pak->data[i];
		    printf("\nparam_arr[%d]=%x", i, param_arr[i]);
		}
		/* PATRIOT_CMD */
		param_cmd_menu = param_arr[0];
	    } else {
		if (cmd == FROM_HOST_WRITE_MAC_ADDR) {
		    for (i = 0; i < 6; i++) {
			param_arr[i] = rx_pak->data[i + 2];
		    }
		} else {
		    param = (rx_pak->data[2] << 24) | (rx_pak->data[3] << 16) |
			(rx_pak->data[4] << 8 ) | (rx_pak->data[5]);
		}
	    }
#ifdef DEBUG	    
	    printf("\ncmd = 0x%02x", cmd);
	    printf("\nparam = 0x%08x", param);
#endif	    
	    /* Clean up the tx packet */
	    memset((uchar *)tx_packet_p, 0, sizeof(fe_packet_t));
	    tx_packet_p->data[0] = PATRIOT_ACK;
	    tx_packet_p->data[1] = cmd + TEST_ACK;
#ifdef DEBUG	    
	    printf("\ntx_packet_p->data[0] = 0x%02x", tx_packet_p->data[0]);
	    printf("\ntx_packet_p->data[1] = 0x%02x\n", tx_packet_p->data[1]);
#endif
	    if (patriot_send_frames(etsec_num, tsec_p, POLL_MODE)) {
		printf("\nFailed to send frames to host\n");
		continue;
	    }

	    if (cmd == FROM_HOST_CPU_ALIVE_TEST) {
		continue;
	    }
	    
	    if (cmd == FROM_HOST_SWITCH_CONSOLE) {
		break;
	    }
	    
	    ret_val = decode_cmd(cmd);
	    
#ifdef DEBUG	    
	    printf("\nret_val = 0x%02x", ret_val);
#endif	    
	    tx_packet_p->data[0] = PATRIOT_RESULT;
	    tx_packet_p->data[1] = ret_val;
	    tx_packet_p->data[2] = to_host_param[0];
	    if (patriot_send_frames(etsec_num, tsec_p, POLL_MODE)) {
		printf("\nFailed to send frames to host\n");
		continue;
	    }
	    
	}
	
    }

    cleanup_tsec (etsec_num);

    return (PASSED);
}



/*------------------------------------------------------------------------------
 * $Log: patriot_main.c,v $
 * Revision 1.1  2014/03/25 02:12:34  huanngo
 * Adding patriot_linux directory to ng_diag code tree
 *
 * Revision 1.12  2012/12/04 21:00:42  huanngo
 * Cosmetic change
 *
 * Revision 1.11  2012/12/03 12:35:17  steja
 * 1. Add Error message utility
 * 2. Fix Framer interrupt Diagnostic loopback
 *
 * Revision 1.10  2012/10/29 22:02:21  huanngo
 * Adding the function to read and display version for the diag Patriot firmware
 *
 * Revision 1.9  2012/10/25 07:24:10  steja
 * Remove "Clear" from the subrate test
 *
 * Revision 1.8  2012/10/16 07:42:40  steja
 * Improve the GPIO test
 *
 * Revision 1.7  2012/10/15 21:17:48  huanngo
 * Reading module MAC address from /sys/class/net/eth1/address and assign correct ETSEC number
 *
 * Revision 1.6  2012/09/14 23:41:56  huanngo
 * Adding the utility to display FPGA secure boot registers and multiboot info table
 *
 * Revision 1.5  2012/07/18 23:51:53  huanngo
 * Adding functions to support programming FPGA to SPI PROM
 *
 * Revision 1.4  2012/06/11 07:43:28  steja
 * Add Power Margin Utilities
 *
 * Revision 1.3  2012/06/08 23:35:39  huanngo
 * Adding constant definitions for ECC memory,UART and GE 0 loopback tests
 *
 * Revision 1.2  2012/05/08 23:52:55  huanngo
 * Support SM Patriot on ngd main code tree
 *
 * Revision 1.1.4.14  2012/04/30 18:37:25  huanngo
 * Download FPGA when necesary, not right after Linux boot up
 *
 * Revision 1.1.4.13  2012/04/12 18:37:02  huanngo
 * Clean up and cosmetic changes
 *
 * Revision 1.1.4.12  2012/03/16 12:05:52  steja
 * Update the code to support Subrate individual test loopback
 *
 * Revision 1.1.4.11  2012/02/28 02:21:42  huanngo
 * When running "./patriot menu" the submenu will comes up
 *
 * Revision 1.1.4.10  2012/01/09 23:06:18  huanngo
 * Support on xformers mips and informers and clean up
 *
 * Revision 1.1.4.9  2011/12/21 23:46:32  huanngo
 * Adding tests for FPGA interrupt and fix bug in FPGA i2c intermittent access failure
 *
 * Revision 1.1.4.8  2011/12/08 15:07:11  steja
 * Update IO Test function
 *
 * Revision 1.1.4.7  2011/12/01 18:51:05  huanngo
 * Support new command to write MAC address to EEPROM and fix bugs
 *
 * Revision 1.1.4.6  2011/11/24 09:33:34  steja
 * Update Patriot code
 *
 * Revision 1.1.4.5  2011/11/24 00:40:01  huanngo
 * Update code for Patriot to fix bugs and support new tests
 *
 * Revision 1.1.4.4  2011/10/27 09:35:08  steja
 * Update DS3170 BERT test
 *
 * Revision 1.1.4.3  2011/10/07 01:11:45  huanngo
 * Update code to support HDLC, SPI EEPROM and FPGA
 *
 * Revision 1.1.4.2  2011/08/18 19:43:25  huanngo
 * Update code to patriot2-branch
 *
 * Revision 1.1.2.17  2011/08/06 00:17:40  huanngo
 * Update code for Patriot
 *
 * Revision 1.1.2.16  2011/07/19 06:11:34  huanngo
 * Update code per code review comments
 *
 * Revision 1.1.2.15  2011/07/08 00:08:48  huanngo
 * Clean up code
 *
 * Revision 1.1.2.14  2011/07/04 09:54:35  steja
 * Update DS3170 code :
 * 1. Add {FROM_HOST_CLR_T3_EX_LPBK_TEST}
 * 2. FROM_HOST_CLR_T3_EX_LPBK_TEST
 *     FROM_HOST_CLR_T3_SUB_EX_LPBK_TEST
 *     FROM_HOST_CLR_E3_EX_LPBK_TEST
 *     FROM_HOST_CLR_E3_SUB_EX_LPBK_TEST
 * 3. Update return (TO_HOST_CLR_E3_EX_LPBK_TEST_FAIL) and
 *     return(TO_HOST_CLR_E3_EX_LPBK_TEST_OK)
 *
 * Revision 1.1.2.13  2011/07/03 19:04:14  huanngo
 * Remove subrate individual loopbacks from host
 *
 * Revision 1.1.2.12  2011/07/01 22:13:02  huanngo
 * Clean up and update code for Patriot
 *
 * Revision 1.1.2.11  2011/07/01 15:39:07  steja
 * 1. Update DS3170 utility test code
 * 2. Update Internal and External loopback test for DS3170
 *
 * Revision 1.1.2.10  2011/06/28 06:27:56  huanngo
 * Update code to support Patriot SM
 *
 * Revision 1.1.2.9  2011/06/22 02:37:18  steja
 * Update DS3170 code Interrupt Handler function
 *
 * Revision 1.1.2.8  2011/06/17 07:03:54  steja
 * 1. Move Patriot_fpga_test to patriot_main.c
 * 2. Remove fpga loopback test item
 *
 * Revision 1.1.2.7  2011/06/14 10:13:41  steja
 * Update DS3170 code and FPGA Register test
 *
 * Revision 1.1.2.6  2011/06/13 06:48:56  steja
 * Update code for DS3170 and FPGA
 *
 * Revision 1.1.2.5  2011/06/09 01:28:10  huanngo
 * Update code to support Patriot SM
 *
 * Revision 1.1.2.4  2011/05/25 16:05:06  steja
 * Update the DS3170 testing function based on specs
 *
 * Revision 1.1.2.3  2011/05/21 01:01:29  huanngo
 * Support memory test, I2C interface
 *
 * Revision 1.1.2.2  2011/05/09 15:38:37  steja
 * Initial Check in Maxim DS3170 Framer
 *
 * Revision 1.1.2.1  2011/05/02 23:33:22  huanngo
 * Update code to support Patriot module side
 *
 *------------------------------------------------------------------------------
 * $Endlog$
 *
 *------------------------------------------------------------------------------
 */

