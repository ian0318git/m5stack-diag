/* $Id: diag_nc_common.h,v 1.6 2017/03/30 08:30:53 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_nc_common.h,v $
 *------------------------------------------------------------------
 *
 * diag_nc_common.h
 * CSX-Tachi nc command header file  
 *
 * Nov 2015, Alan Peng
 *
 * Copyright (c) 2016-2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#ifndef __NC_COMMON_H__
#define __NC_COMMON_H__

#define DIAG_COMMAND_UNAME_DISPLAY      "diag_cmdline_uname"
#define DIAG_COMMAND_IS_NIM_PRESENT     "diag_nc_is_nim_present"
#define DIAG_COMMAND_GET_NIM_CTYPE      "diag_nc_get_nim_ctype"
#define DIAG_COMMAND_READ_FPGA_REG      "diag_nc_read_fpga_reg"
#define DIAG_COMMAND_SET_FPGA_REG       "diag_nc_set_fpga_reg"
#define DIAG_COMMAND_RESET_DEVICE       "diag_nc_reset_device"
#define DIAG_COMMAND_GET_BOARD_TYPE     "diag_nc_get_board_type"
#define DIAG_INTEL_MB_TEST        "diag_intel_mb_test"
#define DIAG_INTEL_CPU_TEST       "diag_intel_cpu_test"
#define DIAG_INTEL_MEM_TEST       "diag_intel_mem_test"
#define DIAG_INTEL_HDD_TEST       "diag_intel_hdd_test"
#define DIAG_INTEL_USB_TEST       "diag_intel_usb_test"
#define DIAG_INTEL_USB20_TEST     "diag_intel_usb20_test"
#define DIAG_INTEL_USB30_TEST     "diag_intel_usb30_test"
#define DIAG_INTEL_SSD_TEST       "diag_intel_ssd_test"
#define DIAG_INTEL_EMMC20_TEST      "diag_intel_emmc20_test"
#define DIAG_INTEL_EMMC30_TEST      "diag_intel_emmc30_test"
#define DIAG_INTEL_BMCUSB0_TEST       "diag_intel_bmcusb0_test"
#define DIAG_INTEL_BMCUSB1_TEST       "diag_intel_bmcusb1_test"
#define DIAG_BTB_TEST        "diag_btb_test"
#define DIAG_I350_TEST       "diag_i350_test"
#define DIAG_X710_TEST       "diag_x710_test"
#define DIAG_I210_TEST       "diag_i210_test"
#define DIAG_GET_I350_MODE         "diag_get_i350_mode"
#define DIAG_REFLASH_I350_MODE     "diag_reflash_i350_mode"
#define DIAG_FORCE_I350_LINK       "diag_force_i350_link"
#define DIAG_DISABLE_I350_RX       "diag_nc_intel_disable_i350_rx"  
#define DIAG_INTEL_SHUTDOWN       "diag_intel_shutdown"
#define DIAG_SHOW_HDD_SIZE       "diag_show_hdd_size"
#define DIAG_SHOW_DIMM_SIZE       "diag_show_dimm_size"
#define DIAG_SHOW_TETH_INTERFACE        "diag_show_teth_interfaces"
#define DIAG_SHOW_TSD_DEVICES        "diag_show_tsd_devices"
#define DIAG_INTEL_CPU_CORE_TEST     "diag_intel_cpu_core_test"
#define DIAG_INTEL_PCI_IF_TEST       "diag_intel_pci_if_test"
#define DIAG_INTEL_TPM20_SPI_TEST      "diag_nc_intel_tpm20_spi_test"
#define DIAG_INTEL_FW_VERSION        "diag_intel_fw_version"
#define DIAG_NIM_FW_VERSION          "diag_nim_fw_version"
#define DIAG_F2W_I350_TEST           "diag_f2w_i350_test"
#define DIAG_F2W_I350_INTF_UP        "diag_f2w_i350_intf_up"
#define DIAG_F2W_I350_RELAY_TEST     "diag_f2w_i350_relay_test"
#define DIAG_F2W_I350_LED_TEST     "diag_f2w_i350_led_test"
#define DIAG_COMMAND_MAX_ITEM           "NULL"

#define INTEL_DF_PCI_BUS                "1"
#define INTEL_ISP_TEST_PCI_BUS 		"2"
#define INTEL_ISP_RAID_PCI_BUS 		"3"
#define INTEL_ISP_CRYPTO_PCI_BUS 	"4"

#define INTEL_NC_STRING_PARAMETER   "0"

#define INTEL_CPU_CORE_6 '1'
#define INTEL_CPU_CORE_8 '2'
#define INTEL_CPU_CORE_12 '3'

/* I350 Fiber */
#define FPGA_SFP_0_CONFIG_REG 				0x10004
#define FPGA_SFP_1_CONFIG_REG 				0x1000C
#define FPGA_SFP_PRESENT_OUTPUT_ENABLE 		0x10200
#define FPGA_SFP_PRESENT_OUTPUT_DISABLE 	0x10000
#define FPGA_SFP_PRESENT_MASK 				0x10000
#define BMC_I350_SFP_I2C_ADDRESS 			    0xA0
#define BMC_I350_SFP_I2C_CONTROL_REG 			0x00

/* PCA 9543 MUX*/
#define BMC_PCA9543_MUX_PORT0_MASK        (0x01)
#define BMC_PCA9543_MUX_PORT1_MASK        (0x02)
#define BMC_PCA9543_MUX_BUS 					(7)
#define BMC_PCA9543_MUX_ADDRESS 			    (0xE0)

/* Fix me when got correct PID */
#define TACHI_SKU1_PID "ENCS5406/K9"
#define TACHI_SKU2_PID "ENCS5408/K9"
#define TACHI_SKU3_PID "ENCS5412/K9"

#define DIAG_INTEL_I350_FIBER_MODE           0
#define DIAG_INTEL_I350_COPPER_MODE           1

#define NC_RECHECK_WAIT_TIME  5000000
#define NC_RECHECK_WAIT_LOOP  20

struct nc_command {
    char *cmd_str;
    int (*func)();
};

struct nc_args {
    char arg[64]; 
    struct nc_args *next;
};


#define BMC_IPADDR       "192.123.123.1"
#define LEWIS_IPADDR     "192.123.123.3"
#define INTEL_IPADDR     "192.123.123.2"

#define BMC_SUB          (0)
#define INTEL_SUB        (1)
#define LEWIS_SUB        (2)
#define ALL_SUB          (9)

#define CLI_REQ_RECV     (0)
#define CLI_REQ_SEND     (1)
#define CLI_REQ_TRIG     (2)
#define SVR_LSTN_RECV      (3)
#define SVR_LSTN_SEND      (4)
#define SVR_LSTN_EXEC      (5)

#define DUMMY1      "dummy1"
#define DUMMY2      "dummy2"
#define DUMMY3      "dummy3"
#define DUMMY4      "dummy4"

typedef enum
{
    BOARD_P1B = 0,
    BOARD_P2A,
    BOARD_P2B,
    BOARD_P2C,
    BOARD_UNKNOW,
} TACHI_L;

#define FX3SRAIDCONF_DISCONN           "fx3sraidconf disconnect"
#define FX3SRAIDCONF_SING_CONN         "fx3sraidconf connect-single-emmc"
#define FX3SRAIDCONF_SING_CONN_USB2    "fx3sraidconf connect-single-emmc 2"
#define FX3SRAIDCONF_SING_CONN_USB3    "fx3sraidconf connect-single-emmc 3"
#define FX3SRAIDCONF_SING_CONN_10SEC   (10000)
#define FX3SRAIDCONF_SING_CONN_2SEC    (2000)

/* nc_command.c */
extern int general_nc_wrapper(unsigned int, unsigned int, char *, char *, char *);

/* nc_client.c */
extern void nc_init_result_file(void);
extern int nc_host_dispatch_comm(unsigned int, struct nc_args *);
extern int nc_check_test_status(void);
extern int diag_nc_nim_testcard_test(void);
extern int diag_nc_client_entry(unsigned int, char *, char *, char *, char *,
        char *);
extern int diag_nc_client_utility_entry(unsigned int, char *, char *, char *,
        char *, char *);
/* intel test item */
extern int diag_nc_intel_mb_test(void);
extern int diag_nc_intel_cpu_test(void);
extern int diag_nc_intel_mem_test(void);
extern int diag_nc_intel_hdd_test(void);
extern int diag_nc_intel_usb_test(void);
extern int diag_nc_intel_usb20_test(void);
extern int diag_nc_intel_usb30_test(void);
extern int diag_nc_intel_ssd_test(void);
extern int diag_nc_intel_emmc_test(void);
extern int diag_nc_intel_bmcusb0_test(void);
extern int diag_nc_intel_bmcusb1_test(void);
extern int diag_nc_intel_btb_test(void);
extern int diag_nc_intel_i350_test(void);
extern int diag_intel_i350_fiber_i2c_test(void);
extern int diag_nc_intel_x710_test(void);
extern int diag_nc_intel_i210_test(void);
extern int diag_nc_intel_get_i350_mode(void);
extern int diag_nc_intel_reflash_i350_mode(void);
extern int diag_nc_intel_force_i350_link(void);
extern int diag_nc_intel_disable_i350_rx(void);
extern int diag_nc_show_hdd_size(void);
extern int diag_nc_show_dimm_size(void);
extern int diag_show_teth_interfaces(void);
extern int diag_show_tsd_devices(void);
extern int diag_intel_cpu_core_test(void);
extern int diag_intel_pci_if_test(void);
extern int diag_nc_intel_tpm20_spi_test(void);
extern int diag_intel_isp_test_pci_if_test(void);
extern int diag_intel_isp_raid_pci_if_test(void);
extern int diag_intel_isp_crypto_pci_if_test(void);
extern int diag_nc_intel_fw_version(void);
extern int diag_nc_nim_fw_version(void);
extern int diag_nc_intel_shutdown(void);

extern int diag_nc_nim_dl_test(void);
extern int diag_nc_nim_dl_lpbk(int, int, int);

extern int diag_nc_f2w_i350_test(char*);
extern int diag_nc_f2w_i350_relay_test(char*);
extern int diag_nc_f2w_i350_led_test(char*);
extern int diag_nc_f2w_i350_intf_up(void);


/* nc_server.c */
void nc_init_listen_port(void);
int diag_nc_get_dispatch_comm(unsigned int);
int diag_nc_server_dispatch_comm(void);
struct nc_args *diag_get_parms_frm_host(void);
void diag_return_parms_to_host(unsigned int, char *, char *);

#endif /* __NC_COMMON_H__ */

/*---------------------------------------------------------------
$Log: diag_nc_common.h,v $
Revision 1.6  2017/03/30 08:30:53  hondwang
Tachi-L brach merge

Revision 1.5  2017/01/25 01:13:13  kodko
Get the Fab Version and PCB Revision cookie field values to distinguish the board type and do the USB3.0/USB2.0 or USB3.0 only test.

Revision 1.4.2.1  2016/11/30 13:32:29  hondwang
Fix build image issue with enhance error message

Revision 1.4  2016/10/19 02:54:42  hondwang
Add I350 I2C and LED test

Revision 1.3  2016/06/04 09:22:20  alpeng
initial check in for f2w

Revision 1.2  2016/04/20 11:25:29  benchen2
add tachi fru portion

Revision 1.1.2.20  2016/04/12 02:46:50  hondwang
fix TPM20 NC

Revision 1.1.2.19  2016/04/11 14:18:33  hondwang
Add TPM20 testing function

Revision 1.1.2.18  2016/03/28 04:16:45  benchen2
modify i350 lpbk test

Revision 1.1.2.17  2016/03/23 05:52:33  hondwang
Add 6C/8C/12C correct PID

Revision 1.1.2.16  2016/03/11 09:45:38  hondwang
Fix NC parameter type issue

Revision 1.1.2.15  2016/03/03 07:55:36  hondwang
Add I350 Fiber I2C testing

Revision 1.1.2.14  2016/03/03 04:55:41  alpeng
 add nim version into bmc util

Revision 1.1.2.13  2016/02/26 09:00:22  hondwang
add intel enhance error message, pci bus scan

Revision 1.1.2.12  2016/02/20 16:20:18  hondwang
Add CPU and PCI bus testing

Revision 1.1.2.11  2016/02/03 03:18:08  hondwang
to support intel core, pci bus, Tethx and Tsdx check

Revision 1.1.2.10  2016/01/12 00:29:01  uid259484
modify to add INTEL NC utility show HDD, DIMM and linux version.
And add RAID card and BTB testing to daughter card item.

Revision 1.1.2.9  2016/01/06 03:02:00  hondwang
Add NC retry to 10 Sec

Revision 1.1.2.8  2016/01/06 01:36:34  jimmyya
Add intel x710 test

Revision 1.1.2.7  2016/01/06 00:39:29  jimmyya
Add intel x710 test

Revision 1.1.2.6  2015/12/28 06:12:30  hondwang
Add and modify files for INTEL NC command support

Revision 1.1.2.5  2015/12/21 01:55:28  alpeng
update nc lib

Revision 1.1.2.4  2015/12/09 10:35:57  alpeng
update code to support lpbk test on bmc for dreamliner

Revision 1.1.2.3  2015/12/01 02:04:36  alpeng
update nc infra structures and support testcard pcie test with nc

Revision 1.1.2.2  2015/11/25 06:12:12  benchen2
add bmc nc comm portion

Revision 1.1.2.1  2015/11/24 12:14:30  alpeng
add nc infrastructure


$Endlog$
*/
