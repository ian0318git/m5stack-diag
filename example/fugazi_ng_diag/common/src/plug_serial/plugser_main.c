/* $Id: plugser_main.c,v 1.6 2018/11/23 09:28:46 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/plug_serial/plugser_main.c,v $
 *------------------------------------------------------------------
 *
 * plugser_main.c - Pluggable Serial main entry.
 *
 * Copyright (c) 2016-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include "nvmonvars.h"
#include "common.h"
#include "types.h"
#include "pcmap.h"
#include "prince_reg.h"
#include "prince_ge_mac.h"
#include "dev_phy_88e1512.h"

#define SGMII_AMPLITUDE_MASK    (0x7)  /* bit 2:0 */
#define SGMII_AMPLITUDE_VALUE   (5)    /* 504mV */
#define GPIO_38                 (38)
#define GPIO_39                 (39)
#define MAX_BUF                 (64)

extern char *banner_string;
extern void diag_menu (int argc, const char *argv[]);
extern int diag_do_all(void);

extern int zynq_qspi_init(void);
extern void zynq_qspi_exit(void);
extern int zynq_i2c_init(void);
extern void zynq_i2c_exit(void);
extern int serdes_type_gpio_test(void);
extern void display_sys_info(int);

int fd_prc = -1;
static int fd_vtop = -1;

static ulong sys_csr_base = 0;
static ulong scc_base = 0;
static ulong ps_intr_ctrl_base = 0;
static ulong ps_ddr_ctrl_base = 0;
ulong fpga_ver = 0;
int SERDES_TYPE_0 = GPIO_38;
int SERDES_TYPE_1 = GPIO_39;
typedef struct plugser_dma_addr_ {
    ulong scc_rx_buf_phys;
    ulong scc_rx_buf_virt;
    ulong scc_tx_buf_phys;
    ulong scc_tx_buf_virt;
    ulong scc_rxbd_phys;
    ulong scc_rxbd_virt;
    ulong scc_txbd_phys;
    ulong scc_txbd_virt;

    ulong ge_rx_buf_phys;
    ulong ge_rx_buf_virt;
    ulong ge_tx_buf_phys;
    ulong ge_tx_buf_virt;
    ulong ge_rxbd_phys;
    ulong ge_rxbd_virt;
    ulong ge_txbd_phys;
    ulong ge_txbd_virt;
} plugser_dma_addr;

static plugser_dma_addr dma_addr;

/*****************************************************************************
 * Function   : get_fpga_base
 * 
 * Description: Get the fpga_base address
 * 
 * Inputs     : None              
 * Outputs    : fpga_base address
 *****************************************************************************/
ulong get_fpga_base (void)
{
    return (sys_csr_base);
}

/*****************************************************************************
 * Function   : get_ge_mac_base
 *
 * Description: Get the ge_mac_base address
 *
 * Inputs     : None              
 * Outputs    : ge_mac_base address
 *****************************************************************************/
ulong get_ge_mac_base (void)
{
    return (get_fpga_base() + ZYNC_GE_MAC_CSR_OFFSET);
}

/*****************************************************************************
 * Function   : get_ge_dma_base
 *
 * Description: Get the ge_dma_base address
 *
 * Inputs     : None              
 * Outputs    : ge_dma_base address
 *****************************************************************************/
ulong get_ge_dma_base (void)
{
    return (get_fpga_base() + ZYNC_GE_DMA_CSR_OFFSET);
}

/*****************************************************************************
 * Function   : get_scc_base
 *
 * Description: Get the scc_base address
 *
 * Inputs     : None              
 * Outputs    : scc_base address
 *****************************************************************************/
ulong get_scc_base (void)
{
    return (scc_base);
}

/*****************************************************************************
 * Function   : get_ps_intr_ctrl_base
 *
 * Description: Get the ps_intr_ctrl_base address
 *
 * Inputs     : None              
 * Outputs    : ps_intr_ctrl_base address
 *****************************************************************************/
ulong get_ps_intr_ctrl_base (void)
{
    return (ps_intr_ctrl_base);
}

/*****************************************************************************
 * Function   : get_ps_ddr_ctrl_base
 *
 * Description: Get the ps_ddr_ctrl_base address
 *
 * Inputs     : None              
 * Outputs    : ps_ddr_ctrl_base address
 *****************************************************************************/
ulong get_ps_ddr_ctrl_base (void)
{
    return (ps_ddr_ctrl_base);
}

/*****************************************************************************
 * Function   : get_scc_dma_rx_phys
 *
 * Description: Get the scc_rx_buf_phys for DMAs
 *
 * Inputs     : None              
 * Outputs    : scc_rx_buf_phys value
 *****************************************************************************/
ulong get_scc_dma_rx_phys (void)
{
    return (dma_addr.scc_rx_buf_phys);
}

/*****************************************************************************
 * Function   : get_scc_dma_rx_virt
 *
 * Description: Get the scc_rx_buf_virt for DMAs
 *
 * Inputs     : None              
 * Outputs    : scc_rx_buf_virt value
 *****************************************************************************/
ulong get_scc_dma_rx_virt (void)
{
    return (dma_addr.scc_rx_buf_virt);
}

/*****************************************************************************
 * Function   : get_scc_dma_tx_phys
 *
 * Description: Get the scc_tx_buf_phys for DMAs
 *
 * Inputs     : None              
 * Outputs    : scc_tx_buf_phys value
 *****************************************************************************/
ulong get_scc_dma_tx_phys (void)
{
    return (dma_addr.scc_tx_buf_phys);
}

/*****************************************************************************
 * Function   : get_scc_dma_tx_virt
 *
 * Description: Get the scc_tx_buf_virt for DMAs
 *
 * Inputs     : None              
 * Outputs    : scc_tx_buf_virt value
 *****************************************************************************/
ulong get_scc_dma_tx_virt (void)
{
    return (dma_addr.scc_tx_buf_virt);
}

/*****************************************************************************
 * Function   : get_scc_dma_rxbd_phys
 *
 * Description: Get the scc_rxbd_phys for DMAs
 *
 * Inputs     : None              
 * Outputs    : scc_rxbd_phys value
 *****************************************************************************/
ulong get_scc_dma_rxbd_phys (void)
{
    return (dma_addr.scc_rxbd_phys);
}

/*****************************************************************************
 * Function   : get_scc_dma_rxbd_virt
 *
 * Description: Get the scc_rxbd_virt for DMAs
 *
 * Inputs     : None              
 * Outputs    : scc_rxbd_virt value
 *****************************************************************************/
ulong get_scc_dma_rxbd_virt (void)
{
    return (dma_addr.scc_rxbd_virt);
}

/*****************************************************************************
 * Function   : get_scc_dma_txbd_phys
 *
 * Description: Get the scc_txbd_phys for DMAs
 *
 * Inputs     : None              
 * Outputs    : scc_txbd_phys value
 *****************************************************************************/
ulong get_scc_dma_txbd_phys (void)
{
    return (dma_addr.scc_txbd_phys);
}

/*****************************************************************************
 * Function   : get_scc_dma_txbd_virt
 *
 * Description: Get the scc_txbd_virt for DMAs
 *
 * Inputs     : None              
 * Outputs    : scc_txbd_virt value
 *****************************************************************************/
ulong get_scc_dma_txbd_virt (void)
{
    return (dma_addr.scc_txbd_virt);
}

/*****************************************************************************
 * Function   : get_ge_dma_rx_phys
 *
 * Description: Get the ge_rx_buf_phys for DMAs
 *
 * Inputs     : None              
 * Outputs    : ge_rx_buf_phys value
 *****************************************************************************/
ulong get_ge_dma_rx_phys (void)
{
    return (dma_addr.ge_rx_buf_phys);
}

/*****************************************************************************
 * Function   : get_ge_dma_rx_virt
 *
 * Description: Get the ge_rx_buf_virt for DMAs
 *
 * Inputs     : None              
 * Outputs    : ge_rx_buf_virt value
 *****************************************************************************/
ulong get_ge_dma_rx_virt (void)
{
    return (dma_addr.ge_rx_buf_virt);
}

/*****************************************************************************
 * Function   : get_ge_dma_tx_phys
 *
 * Description: Get the ge_tx_buf_phys for DMAs
 *
 * Inputs     : None              
 * Outputs    : ge_tx_buf_phys value
 *****************************************************************************/
ulong get_ge_dma_tx_phys (void)
{
    return (dma_addr.ge_tx_buf_phys);
}

/*****************************************************************************
 * Function   : get_ge_dma_tx_virt
 *
 * Description: Get the ge_tx_buf_virt for DMAs
 *
 * Inputs     : None              
 * Outputs    : ge_tx_buf_virt value
 *****************************************************************************/
ulong get_ge_dma_tx_virt (void)
{
    return (dma_addr.ge_tx_buf_virt);
}

/*****************************************************************************
 * Function   : get_ge_rxbd_phys
 *
 * Description: Get the ge_rxbd_phys for DMAs
 *
 * Inputs     : None              
 * Outputs    : ge_rxbd_phys value
 *****************************************************************************/
ulong get_ge_rxbd_phys (void)
{
    return (dma_addr.ge_rxbd_phys);
}

/*****************************************************************************
 * Function   : get_ge_rxbd_virt
 *
 * Description: Get the ge_rxbd_virt for DMAs
 *
 * Inputs     : None              
 * Outputs    : ge_rxbd_virt value
 *****************************************************************************/
ulong get_ge_rxbd_virt (void)
{
    return (dma_addr.ge_rxbd_virt);
}

/*****************************************************************************
 * Function   : get_ge_txbd_phys
 *
 * Description: Get the ge_txbd_phys for DMAs
 *
 * Inputs     : None              
 * Outputs    : ge_txbd_phys value
 *****************************************************************************/
ulong get_ge_txbd_phys (void)
{
    return (dma_addr.ge_txbd_phys);
}

/*****************************************************************************
 * Function   : get_ge_txbd_virt
 *
 * Description: Get the ge_txbd_virt for DMAs
 *
 * Inputs     : None              
 * Outputs    : ge_txbd_virt value
 *****************************************************************************/
ulong get_ge_txbd_virt (void)
{
    return (dma_addr.ge_txbd_virt);
}

/*****************************************************************************
 * Function   : get_dma_addr
 *
 * Description: Get the Physical & Virtual address for DMAs
 *
 * Inputs     : file handle              
 * Outputs    : PASS/FAIL
 *****************************************************************************/
static int get_dma_addr (int fd)
{
    void *virt_addr;

    if (ioctl(fd, GET_SCC_DMA_RX_BUF_PHYS, &dma_addr.scc_rx_buf_phys)) {
        perror("Failed to get SCC DMA Rx buffer Physical address");
        return (FAILED);
    }

    virt_addr = (void *)mmap(NULL, 
        PRINCE_SCC_BUF_SIZE * PRINCE_SCC_BUF_NUM, 
        (PROT_READ | PROT_WRITE), 
        MAP_SHARED, fd, (ulong)dma_addr.scc_rx_buf_phys);
    if (virt_addr == MAP_FAILED) {
        close(fd);
        perror("Error mmapping SCC DMA Rx buffer");
        return (FAILED);
    }
    dma_addr.scc_rx_buf_virt = (ulong)virt_addr;

    if (ioctl(fd, GET_SCC_DMA_TX_BUF_PHYS, &dma_addr.scc_tx_buf_phys)) {
        perror("Failed to get SCC DMA Tx buffer Physical address");
        return (FAILED);
    }

    virt_addr = (void *)mmap(NULL, 
        PRINCE_SCC_BUF_SIZE * PRINCE_SCC_BUF_NUM, 
        (PROT_READ | PROT_WRITE), 
        MAP_SHARED, fd, (ulong)dma_addr.scc_tx_buf_phys);
    if (virt_addr == MAP_FAILED) {
        close(fd);
        perror("Error mmapping SCC DMA Rx buffer");
        return (FAILED);
    }
    dma_addr.scc_tx_buf_virt = (ulong)virt_addr;

    if (ioctl(fd, GET_SCC_DMA_RXBD_PHYS, &dma_addr.scc_rxbd_phys)) {
        perror("Failed to get SCC DMA RXBD Physical address");
        return (FAILED);
    }

    virt_addr = (void *)mmap(NULL, 
        PRINCE_SCC_BUF_SIZE * PRINCE_SCC_BUF_NUM, 
        (PROT_READ | PROT_WRITE), 
        MAP_SHARED, fd, (ulong)dma_addr.scc_rxbd_phys);
    if (virt_addr == MAP_FAILED) {
        close(fd);
        perror("Error mmapping SCC DMA RXBD");
        return (FAILED);
    }
    dma_addr.scc_rxbd_virt = (ulong)virt_addr;

    if (ioctl(fd, GET_SCC_DMA_TXBD_PHYS, &dma_addr.scc_txbd_phys)) {
        perror("Failed to get SCC DMA TXBD Physical address");
        return (FAILED);
    }

    virt_addr = (void *)mmap(NULL, 
        PRINCE_SCC_BUF_SIZE * PRINCE_SCC_BUF_NUM, 
        (PROT_READ | PROT_WRITE), 
        MAP_SHARED, fd, (ulong)dma_addr.scc_txbd_phys);
    if (virt_addr == MAP_FAILED) {
        close(fd);
        perror("Error mmapping SCC DMA TXBD");
        return (FAILED);
    }
    dma_addr.scc_txbd_virt = (ulong)virt_addr;

    if (ioctl(fd, GET_GE_DMA_RX_BUF_PHYS, &dma_addr.ge_rx_buf_phys)) {
        perror("Failed to get GE DMA RX buffer Physical address");
        return (FAILED);
    }

    virt_addr = (void *)mmap(NULL, 
        PRINCE_GE_DMA_RXBD_BUF_SIZE * PRINCE_GE_DMA_RXBD_NUM, 
        (PROT_READ | PROT_WRITE), 
        MAP_SHARED, fd, (ulong)dma_addr.ge_rx_buf_phys);

    if (virt_addr == MAP_FAILED) {
        close(fd);
        perror("Error mmapping GE DMA RX buffer");
        return (FAILED);
    }
    dma_addr.ge_rx_buf_virt = (ulong)virt_addr;

    if (ioctl(fd, GET_GE_DMA_TX_BUF_PHYS, &dma_addr.ge_tx_buf_phys)) {
        perror("Failed to get GE DMA TX buffer Physical address");
        return (FAILED);
    }

    virt_addr = (void *)mmap(NULL, 
        PRINCE_GE_DMA_TXBD_BUF_MAX, 
        (PROT_READ | PROT_WRITE), 
        MAP_SHARED, fd, (ulong)dma_addr.ge_tx_buf_phys);
    if (virt_addr == MAP_FAILED) {
        close(fd);
        perror("Error mmapping GE DMA TX buffer");
        return (FAILED);
    }
    dma_addr.ge_tx_buf_virt = (ulong)virt_addr;

    if (ioctl(fd, GET_GE_DMA_RXBD_PHYS, &dma_addr.ge_rxbd_phys)) {
        perror("Failed to get GE DMA RXBD Physical address");
        return (FAILED);
    }

    virt_addr = (void *)mmap(NULL, 
        PRINCE_GE_DMA_RXBD_NUM * BYTES_PER_BD, 
        (PROT_READ | PROT_WRITE), 
        MAP_SHARED, fd, (ulong)dma_addr.ge_rxbd_phys);
    if (virt_addr == MAP_FAILED) {
        close(fd);
        perror("Error mmapping GE DMA RXBD");
        return (FAILED);
    }
    dma_addr.ge_rxbd_virt = (ulong)virt_addr;

    if (ioctl(fd, GET_GE_DMA_TXBD_PHYS, &dma_addr.ge_txbd_phys)) {
        perror("Failed to get GE DMA TXBD Physical address");
        return (FAILED);
    }

    virt_addr = (void *)mmap(NULL, 
        PRINCE_GE_DMA_TXBD_NUM * BYTES_PER_BD * PRINCE_GE_DMA_TXBD_TYPE, 
        (PROT_READ | PROT_WRITE), 
        MAP_SHARED, fd, (ulong)dma_addr.ge_txbd_phys);
    if (virt_addr == MAP_FAILED) {
        close(fd);
        perror("Error mmapping GE DMA TXBD");
        return (FAILED);
    }
    dma_addr.ge_txbd_virt = (ulong)virt_addr;

    return (PASSED);
}

/*****************************************************************************
 * Function   : get_gic_spi_status1
 *
 * Description: Get SPI status
 *
 * Inputs     : None              
 * Outputs    : current status
 *****************************************************************************/
ulong get_gic_spi_status1 (void)
{
    ulong base_addr = get_ps_intr_ctrl_base();
    ulong *reg_p = (ulong *)(base_addr + ZYNC_PS_SPI_STS1_OFFSET);
    printf("ZYNC_PS_SPI_STS1 = %#x\n", *reg_p);
    return (*reg_p);
}

/*****************************************************************************
 * Function   : get_virt_base_addr
 *
 * Description: Get the virtual memory base addresses
 *
 * Inputs     : None              
 * Outputs    : PASS/FAIL
 *****************************************************************************/
int get_virt_base_addr (void)
{
    void *csr_base_ptr;
    void *scc_base_ptr;
    void *ps_intr_base_ptr;
    void *ps_ddrc_base_ptr;

    /* Memory map for system csr, GE DMA and GE MAC */
    csr_base_ptr = (void *)mmap(NULL, ZYNC_SYSTEM_CSR_LENGTH, (PROT_READ | PROT_WRITE), 
                MAP_SHARED, fd_prc, ZYNC_SYSTEM_CSR_BASE);
    if (csr_base_ptr == MAP_FAILED) {
        close(fd_prc);
        cterr('f', 0, "Error mmapping for System CSR");
        return (FAILED);
    }
    sys_csr_base = (ulong)csr_base_ptr;

    /* Memory map for SCC */
    scc_base_ptr = (void *)mmap(NULL, ZYNC_SCC_CSR_LENGTH, (PROT_READ | PROT_WRITE), 
                MAP_SHARED, fd_prc, ZYNC_SCC_CSR_BASE);
    if (scc_base_ptr == MAP_FAILED) {
        close(fd_prc);
        cterr('f', 0, "Error mmapping for SCC");
        return (FAILED);
    }
    scc_base = (ulong)scc_base_ptr;

    /* Memory map for CPU Interrupt Control Distributor */
    ps_intr_base_ptr = (void *)mmap(NULL, ZYNC_PS_CPU_INTR_CTRL_LENGTH, (PROT_READ | PROT_WRITE), 
                MAP_SHARED, fd_prc, ZYNC_PS_CPU_INTR_CTRL_BASE);
    if (ps_intr_base_ptr == MAP_FAILED) {
        close(fd_prc);
        cterr('f', 0, "Error mmapping for CPU Interrupt Control Distributor");
        return (FAILED);
    }
    ps_intr_ctrl_base = (ulong)ps_intr_base_ptr;

    /* Memory map for CPU DDR Controller */
    ps_ddrc_base_ptr = (void *)mmap(NULL, ZYNC_PS_DDRC_LENGTH, (PROT_READ | PROT_WRITE), 
                MAP_SHARED, fd_prc, ZYNC_PS_DDRC_BASE);
    if (ps_ddrc_base_ptr == MAP_FAILED) {
        close(fd_prc);
        cterr('f', 0, "Error mmapping for CPU DDR Controller");
        return (FAILED);
    }
    ps_ddr_ctrl_base = (ulong)ps_ddrc_base_ptr;

    return (PASSED);
}

/**************************************************************************
 *
 * Function: plugser_help
 *
 * Display help
 *
 * Input: None
 *
 * Return: None
 *
 * *************************************************************************
 */
static void plugser_help (void)
{
    printf("Usage: plugser [-a] [-h]\n\n");

    printf("Options:\n");
    printf("-a Do all the tests\n");
    printf("-h Display this help\n");
    printf("\n");
}

/*****************************************************************************
 * Function   : set_ready_bit
 *
 * Description: Set primary interface ready bit high
 *
 * Inputs     : None              
 * Outputs    : None
 *****************************************************************************/
static void set_ready_bit (void)
{
    ulong base_addr = get_fpga_base();
    sys_csr_reg_t *sys_csr = (sys_csr_reg_t *)base_addr;

    /* Set primary interface ready bit high */
    sys_csr->leds_sts |= PRI_INTF_READY;
}
/*****************************************************************************
 * Function   : set_serdes_type_gpio
 * CSCvm45577 : Pluggable Serial - SerDes Type test Failed	        
 *
 * Description: Config Serdes Type GPIO pin
 * example :
 * echo 38 > /sys/class/gpio/export
 * echo 39 > /sys/class/gpio/export
 * echo in > /sys/class/gpio/gpio38/direction
 * echo in > /sys/class/gpio/gpio39/direction
 *
 * Inputs     : None              
 * Outputs    : None
 *****************************************************************************/
static void set_serdes_type_gpio (void)
{
    char cmd[MAX_BUF];

    /* Step1 : export GPIO pin */
    sprintf(cmd, "echo %d > /sys/class/gpio/export", SERDES_TYPE_0);
    system(cmd);
    sprintf(cmd, "echo %d > /sys/class/gpio/export", SERDES_TYPE_1);
    system(cmd);
    /* Step2 : Set GPIO pin direction */
    sprintf(cmd, "echo in > /sys/class/gpio/gpio%d/direction", SERDES_TYPE_0);
    system(cmd);
    sprintf(cmd, "echo in > /sys/class/gpio/gpio%d/direction", SERDES_TYPE_1);
    system(cmd);
}

/*******************************************************************************
 * Function: dev_88e1512_set_out_amp
 *
 * Description: This function sets the output amptitule for Marvell GE PHY
 *
 * Input: phy_addr - phy address
 * Output: PASSED/FAILED
 *******************************************************************************
 */
static int dev_88e1512_set_out_amp (uint phy_addr)
{
    int retval = PASSED;
    ushort data;

    retval = dev_phy_read_reg(MRV88E1512_REG_PAGE_1, 
                              MRV88E1512_SPECIFIC_CONTROL2_REG, &data);
    if (retval != PASSED) {
        printf("%s(): phy smi read failed\n", __FUNCTION__);
        return (retval);
    }
    data &= ~(SGMII_AMPLITUDE_MASK);
    data |= SGMII_AMPLITUDE_VALUE;
    retval = dev_phy_write_reg(MRV88E1512_REG_PAGE_1, 
                               MRV88E1512_SPECIFIC_CONTROL2_REG, &data);
    if (retval != PASSED) {
        printf("%s(): Set SGMII Output Amplitude failed.\n", __FUNCTION__);
        return (retval);
    }
    return (retval);
}
/*****************************************************************************
 * Function   : main
 *              
 * Description: entry point of Pluggable Serial
 *              
 * Inputs     : argc, number of argument
 *              argv, command line arguments
 * Outputs    : exit status
 *****************************************************************************/
int main (int argc, const char *argv[])
{
    char arg;
    char cmd[32];
    int is_do_all          = FALSE;
    int is_menu_mode       = FALSE;
    int is_serdes_mode     = FALSE;
    int opt;
    ulong base_addr;

    if (argc > 1) {
        for (;;) {
            opt = getopt(argc, argv, "ahs");
            if (opt == EOF) {
                break;
            }

            switch (opt) {
            case 'h':  /* Help */
                plugser_help();
                exit(1);
                break;
            case 'a':  /* Do all */
                is_do_all = TRUE;
                break;
            case 's':  /* SerDes Test */
                is_serdes_mode = TRUE;
                break;
            default:
                plugser_help();
                exit(1);
                break;
            }
        }
    } else {
        /* Menu mode if no option is provided */
        is_menu_mode   = TRUE;
    }

    fflush(stdin);
    /* Serdes type test */
    if (is_serdes_mode == TRUE) {
        serdes_type_gpio_test();
        is_serdes_mode = FALSE;        
        return (PASSED);
    }
    /* Open modules */
    fd_prc = open("/dev/prc", O_RDWR);
    if (fd_prc <= 0) {
        cterr('f', 0, "/dev/prc not found: fd_prc = %d\n", fd_prc);
        return (FAILED);
    }

    fd_vtop = open("/dev/addr_vtop", O_RDWR);
    if (fd_vtop <= 0) {
        cterr('f', 0, "/dev/addr_vtop not found: fd_vtop = %d\n", fd_vtop);
        return (FAILED);
    }

    /* Get the virtual memory base addresses */
    if (get_virt_base_addr()) {
         perror("Failed to get the virtual base addresses");
         return (FAILED);
    }

    /* Get FPGA version */
    base_addr = get_fpga_base();
    sys_csr_reg_t *sys_csr = (sys_csr_reg_t *)base_addr;
    fpga_ver = ((sys_csr->fpga_revision) >> 8) & 0xffff;
    printf("FPGA revision: %x\n", fpga_ver);

    /* Get the Physical & Virtual address for DMAs */
    get_dma_addr(fd_prc);

    /* Initialize QSPI and I2c. */
    if (zynq_qspi_init()) {
        cterr('f', 0, "QSPI init failed");
    }
    if (zynq_i2c_init()) {
        cterr('f', 0, "I2C init failed");
    }

    /* CSCvh67800: Set output amplitude per HW request */
    printf("Set output amplitude.\n");
    dev_88e1512_set_out_amp(PRINCE_PHY_ADDR);

    /* Assign IP address */
    system("ifconfig eth0 192.168.2.200 netmask 255.255.255.0 up");

    /* Set Ready Pin */
    set_ready_bit();

    /* Config Serdes Type GPIO pin */
    set_serdes_type_gpio();

    display_sys_info(0);
    printf("%s", banner_string);

    /* Turn off over commit to avoid oom during memory test. */
    system("echo 2 > /proc/sys/vm/overcommit_memory");

    if (is_menu_mode == TRUE) {
        diag_menu(1, argv); /* goto menu directly; */
    } else {
        if (is_do_all == TRUE) {
            /* Do all tests from here */
            diag_do_all();
        }
    }

    zynq_qspi_exit();
    zynq_i2c_exit();

    return (PASSED);
}

/******** History ********
$Log: plugser_main.c,v $
Revision 1.6  2018/11/23 09:28:46  hondwang
Re-instruct pluggable common code with CDETs CSCvn17216

Revision 1.5  2018/09/21 03:01:16  iachang
CSCvm45577: Fixed SerDes Type GPIO test issue

Revision 1.4.10.1  2018/11/21 09:37:22  iachang
Sync up with main trunk.

Revision 1.4  2018/08/02 09:35:01  iachang
Merge Pluggable Serial from branch star-branch-c9xx to main trunk

Revision 1.3  2018/02/09 09:17:34  hondwang
Merge Star branch star-branch-c9xx to main trunk

Revision 1.2.2.3  2018/01/24 08:47:15  iachang
CSCvh67800: Set output amplitude per HW request

Revision 1.2.2.2  2018/01/20 06:54:58  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.2  2018/01/20 04:58:56  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.1.4.5  2017/10/24 11:50:08  iachang
Supported SerDes Type GPIO Test.

Revision 1.1.4.4  2017/09/13 16:54:29  iachang
Support Pluggable Serial test via NC command

Revision 1.1.4.3  2017/08/25 10:27:16  lucywang
modified for ASYNC test and FPGA update

Revision 1.1.4.2  2017/08/08 07:43:38  hondwang
add pluggable serial for star-branch-c9xx

Revision 1.1.2.1  2017/07/31 10:49:58  lucywang
add pluggable serial code of host and module


$Endlog$
*/
