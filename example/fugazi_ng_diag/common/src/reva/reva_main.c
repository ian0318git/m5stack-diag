/* $Id: reva_main.c,v 1.2 2016/05/06 03:43:53 umlin Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/reva/reva_main.c,v $
 *------------------------------------------------------------------
 *
 * reva_main.c - Reva main entry.
 *
 * Copyright (c) 2015-2016 by Cisco Systems, Inc.
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

extern char *banner_string;

extern void diag_menu (int argc, const char *argv[]);
extern int diag_do_all(void);
extern int ge_mac_init(void);
extern int ge_phy_init(uint);
extern int dev_88e1512_set_lpbk (int, int, int);

extern int zynq_qspi_init(void);
extern void zynq_qspi_exit(void);
extern int zynq_i2c_init(void);
extern void zynq_i2c_exit(void);

extern void display_sys_info(int);

int fd_prc = -1;
static int fd_vtop = -1;

static ulong sys_csr_base = 0;
static ulong scc_base = 0;
static ulong ps_intr_ctrl_base = 0;
static ulong ps_ddr_ctrl_base = 0;

typedef struct prince_dma_addr_ {
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
} prince_dma_addr;

static prince_dma_addr dma_addr;

/*****************************************************************************
 * Function   : get_fpga_base
 * 
 * Description: Get the fpga_base address
 * 
 * Inputs     : None              
 * Outputs    : fpga_base address
 *****************************************************************************/
ulong get_fpga_base()
{
    return sys_csr_base;
}

/*****************************************************************************
 * Function   : get_ge_mac_base
 *
 * Description: Get the ge_mac_base address
 *
 * Inputs     : None              
 * Outputs    : ge_mac_base address
 *****************************************************************************/
ulong get_ge_mac_base()
{
    return get_fpga_base() + ZYNC_GE_MAC_CSR_OFFSET;
}

/*****************************************************************************
 * Function   : get_ge_dma_base
 *
 * Description: Get the ge_dma_base address
 *
 * Inputs     : None              
 * Outputs    : ge_dma_base address
 *****************************************************************************/
ulong get_ge_dma_base()
{
    return get_fpga_base() + ZYNC_GE_DMA_CSR_OFFSET;
}

/*****************************************************************************
 * Function   : get_scc_base
 *
 * Description: Get the scc_base address
 *
 * Inputs     : None              
 * Outputs    : scc_base address
 *****************************************************************************/
ulong get_scc_base()
{
    return scc_base;
}

/*****************************************************************************
 * Function   : get_ps_intr_ctrl_base
 *
 * Description: Get the ps_intr_ctrl_base address
 *
 * Inputs     : None              
 * Outputs    : ps_intr_ctrl_base address
 *****************************************************************************/
ulong get_ps_intr_ctrl_base()
{
    return ps_intr_ctrl_base;
}

/*****************************************************************************
 * Function   : get_ps_ddr_ctrl_base
 *
 * Description: Get the ps_ddr_ctrl_base address
 *
 * Inputs     : None              
 * Outputs    : ps_ddr_ctrl_base address
 *****************************************************************************/
ulong get_ps_ddr_ctrl_base()
{
    return ps_ddr_ctrl_base;
}

/*****************************************************************************
 * Function   : get_scc_dma_rx_phys
 *
 * Description: Get the scc_rx_buf_phys for DMAs
 *
 * Inputs     : None              
 * Outputs    : scc_rx_buf_phys value
 *****************************************************************************/
ulong get_scc_dma_rx_phys()
{
    return dma_addr.scc_rx_buf_phys;
}

/*****************************************************************************
 * Function   : get_scc_dma_rx_virt
 *
 * Description: Get the scc_rx_buf_virt for DMAs
 *
 * Inputs     : None              
 * Outputs    : scc_rx_buf_virt value
 *****************************************************************************/
ulong get_scc_dma_rx_virt()
{
    return dma_addr.scc_rx_buf_virt;
}

/*****************************************************************************
 * Function   : get_scc_dma_tx_phys
 *
 * Description: Get the scc_tx_buf_phys for DMAs
 *
 * Inputs     : None              
 * Outputs    : scc_tx_buf_phys value
 *****************************************************************************/
ulong get_scc_dma_tx_phys()
{
    return dma_addr.scc_tx_buf_phys;
}

/*****************************************************************************
 * Function   : get_scc_dma_tx_virt
 *
 * Description: Get the scc_tx_buf_virt for DMAs
 *
 * Inputs     : None              
 * Outputs    : scc_tx_buf_virt value
 *****************************************************************************/
ulong get_scc_dma_tx_virt()
{
    return dma_addr.scc_tx_buf_virt;
}

/*****************************************************************************
 * Function   : get_scc_dma_rxbd_phys
 *
 * Description: Get the scc_rxbd_phys for DMAs
 *
 * Inputs     : None              
 * Outputs    : scc_rxbd_phys value
 *****************************************************************************/
ulong get_scc_dma_rxbd_phys()
{
    return dma_addr.scc_rxbd_phys;
}

/*****************************************************************************
 * Function   : get_scc_dma_rxbd_virt
 *
 * Description: Get the scc_rxbd_virt for DMAs
 *
 * Inputs     : None              
 * Outputs    : scc_rxbd_virt value
 *****************************************************************************/
ulong get_scc_dma_rxbd_virt()
{
    return dma_addr.scc_rxbd_virt;
}

/*****************************************************************************
 * Function   : get_scc_dma_txbd_phys
 *
 * Description: Get the scc_txbd_phys for DMAs
 *
 * Inputs     : None              
 * Outputs    : scc_txbd_phys value
 *****************************************************************************/
ulong get_scc_dma_txbd_phys()
{
    return dma_addr.scc_txbd_phys;
}

/*****************************************************************************
 * Function   : get_scc_dma_txbd_virt
 *
 * Description: Get the scc_txbd_virt for DMAs
 *
 * Inputs     : None              
 * Outputs    : scc_txbd_virt value
 *****************************************************************************/
ulong get_scc_dma_txbd_virt()
{
    return dma_addr.scc_txbd_virt;
}

/*****************************************************************************
 * Function   : get_ge_dma_rx_phys
 *
 * Description: Get the ge_rx_buf_phys for DMAs
 *
 * Inputs     : None              
 * Outputs    : ge_rx_buf_phys value
 *****************************************************************************/
ulong get_ge_dma_rx_phys()
{
    return dma_addr.ge_rx_buf_phys;
}

/*****************************************************************************
 * Function   : get_ge_dma_rx_virt
 *
 * Description: Get the ge_rx_buf_virt for DMAs
 *
 * Inputs     : None              
 * Outputs    : ge_rx_buf_virt value
 *****************************************************************************/
ulong get_ge_dma_rx_virt()
{
    return dma_addr.ge_rx_buf_virt;
}

/*****************************************************************************
 * Function   : get_ge_dma_tx_phys
 *
 * Description: Get the ge_tx_buf_phys for DMAs
 *
 * Inputs     : None              
 * Outputs    : ge_tx_buf_phys value
 *****************************************************************************/
ulong get_ge_dma_tx_phys()
{
    return dma_addr.ge_tx_buf_phys;
}

/*****************************************************************************
 * Function   : get_ge_dma_tx_virt
 *
 * Description: Get the ge_tx_buf_virt for DMAs
 *
 * Inputs     : None              
 * Outputs    : ge_tx_buf_virt value
 *****************************************************************************/
ulong get_ge_dma_tx_virt()
{
    return dma_addr.ge_tx_buf_virt;
}

/*****************************************************************************
 * Function   : get_ge_rxbd_phys
 *
 * Description: Get the ge_rxbd_phys for DMAs
 *
 * Inputs     : None              
 * Outputs    : ge_rxbd_phys value
 *****************************************************************************/
ulong get_ge_rxbd_phys()
{
    return dma_addr.ge_rxbd_phys;
}

/*****************************************************************************
 * Function   : get_ge_rxbd_virt
 *
 * Description: Get the ge_rxbd_virt for DMAs
 *
 * Inputs     : None              
 * Outputs    : ge_rxbd_virt value
 *****************************************************************************/
ulong get_ge_rxbd_virt()
{
    return dma_addr.ge_rxbd_virt;
}

/*****************************************************************************
 * Function   : get_ge_txbd_phys
 *
 * Description: Get the ge_txbd_phys for DMAs
 *
 * Inputs     : None              
 * Outputs    : ge_txbd_phys value
 *****************************************************************************/
ulong get_ge_txbd_phys()
{
    return dma_addr.ge_txbd_phys;
}

/*****************************************************************************
 * Function   : get_ge_txbd_virt
 *
 * Description: Get the ge_txbd_virt for DMAs
 *
 * Inputs     : None              
 * Outputs    : ge_txbd_virt value
 *****************************************************************************/
ulong get_ge_txbd_virt()
{
    return dma_addr.ge_txbd_virt;
}

/*****************************************************************************
 * Function   : get_dma_addr
 *
 * Description: Get the Physical & Virtual address for DMAs
 *
 * Inputs     : file handle              
 * Outputs    : PASS/FAIL
 *****************************************************************************/
static int get_dma_addr(int fd)
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
ulong get_gic_spi_status1()
{
    ulong base_addr = get_ps_intr_ctrl_base();
    ulong *reg_p = (ulong *)(base_addr + ZYNC_PS_SPI_STS1_OFFSET);
    printf("ZYNC_PS_SPI_STS1 = %#x\n", *reg_p);
    return *reg_p;
}

/*****************************************************************************
 * Function   : get_virt_base_addr
 *
 * Description: Get the virtual memory base addresses
 *
 * Inputs     : None              
 * Outputs    : PASS/FAIL
 *****************************************************************************/
int get_virt_base_addr()
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
    /* ZYNC_SCC_CSR_LENGTH = 0x10000, Reva has 6 blocks CSR, it needs 0x18000. */
    scc_base_ptr = (void *)mmap(NULL, (ZYNC_SCC_CSR_LENGTH * 2), (PROT_READ | PROT_WRITE), 
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
 * Function: reva_help
 *
 * Display help
 *
 * Input: None
 * Return: None
 * **************************************************************************/
static void reva_help (void)
{
    printf("Usage: prince [-a] [-h]\n\n");

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
static void set_ready_bit()
{
    ulong base_addr = get_fpga_base();
    sys_csr_reg_t *sys_csr = (sys_csr_reg_t *)base_addr;

    /* Set primary interface ready bit high */
    sys_csr->leds_sts |= PRI_INTF_READY;
}

/*****************************************************************************
 * Function   : main
 *              
 * Description: entry point of reva
 *              
 * Inputs     : argc, number of argument
 *              argv, command line arguments
 * Outputs    : exit status
 *****************************************************************************/
int main(int argc, const char *argv[])
{
    char arg;
    char cmd[32];
    int is_do_all          = FALSE;
    int is_menu_mode       = FALSE;
    int opt;

    if (argc > 1) {
        for (;;) {
            opt = getopt(argc, argv, "ah");
            if (opt == EOF) {
                break;
            }

            switch (opt) {
            case 'h':  /* Help */
                reva_help();
                exit(1);
                break;
            case 'a':  /* Do all */
                is_do_all = TRUE;
                break;
            default:
                reva_help();
                exit(1);
                break;
            }
        }
    } else {
        /* Menu mode if no option is provided */
        is_menu_mode   = TRUE;
    }

    fflush(stdin);

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

    /* Get the Physical & Virtual address for DMAs */
    get_dma_addr(fd_prc);

    /* Initialize QSPI and I2c. */
    if (zynq_qspi_init()) {
        cterr('f', 0, "QSPI init failed");
    }
    if (zynq_i2c_init()) {
        cterr('f', 0, "I2C init failed");
    }

    /* Assign IP address */
    system("ifconfig eth0 192.123.123.200 netmask 255.255.255.0 up");

    /* Set Ready Pin */
    set_ready_bit();

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

    return(PASSED);
}

/******** History ********
$Log: reva_main.c,v $
Revision 1.2  2016/05/06 03:43:53  umlin
Reva: Commit Reva module side diag codes to main trunk


$Endlog$
*/
