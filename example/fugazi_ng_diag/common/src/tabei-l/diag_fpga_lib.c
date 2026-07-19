 /* $Id: diag_fpga_lib.c,v 1.3 2020/10/07 08:20:48 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/diag_fpga_lib.c,v $
 *-----------------------------------------------------------------------------
 *
 * Filename   : diag_fpga_lib.c
 * Description: FPGA Library.
 *
 * Copyright (c) 2018-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/io.h>
#include <time.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "defs.h"
#include "error.h"
#include "common.h"
#include "types.h"
#include "nvsysvars.h"
#include <unistd.h>
#include <strings.h>
#include <assert.h>
#include "queryflags.h"
#include "common_utils.h"
#include "queryflags.h"
#include "menu.h"
#include "proto.h"
#include "diag_fpga_lib.h"
#include "diag_fpga.h"


/*******************************************************************************
 *                          Function Declaration
 *******************************************************************************
 */
int fpga_read_reg(uint, uint *);
int fpga_write_reg(uint, uint);
int fpga_reset_api(uint, uint, uint, uint);
int open_ioperm(void);
int close_ioperm(void);
int fpga_register_operation(uint, uint, uint);
int has_ge1_sku(void);
int uio_open(void);
void tabei_fpga_base_addr_init(void);
static int uio_find_name(const char *, char *);

/*******************************************************************************
 *                          Global Variables
 *******************************************************************************
 */
unsigned long dash_fpga;
static volatile void *regs = 0;
static char name[32];
static int uiofd;

/*******************************************************************************
 *
 * Function    : uio_find_name 
 * Description : Function to find FPGA uio directory name (ie, "uio0")
 * Inputs      : *drv_name - Directory name
 *               *uio_name - uio name
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
static int uio_find_name (const char *drv_name, char *uio_name)
{
    FILE *fp;
    DIR *dir;
    struct dirent *dp;
    char *sys = "/sys/class/uio";

    if (strlen(name)) {
        sprintf(uio_name, name);
        return(1);
    }

    if ((dir = opendir(sys)) == NULL) {
        perror("cannot open directory /sys/class/uio");
        return -1;
    }
    while ((dp = readdir(dir)) != NULL) {
        if (dp->d_name[0] == '.')
            continue;
        sprintf(name, "%s/%s/name", sys, dp->d_name);
        if ((fp = fopen(name, "r")) == NULL) {
            continue;
        }

        fgets(name, sizeof(name), fp);
        if (strstr(name, DRV_NAME)) {
            sprintf(name, dp->d_name);
            sprintf(uio_name, name);
            fclose(fp);
            closedir(dir);
            return 1;
        } else {

        }
        fclose(fp);
    }

    printf("uio driver %s not found in %s directory \n", drv_name,
           sys);
    name[0] = '\0';
    closedir(dir);
    return -1;
}

/*******************************************************************************
 *
 * Function    : uio_open 
 * Description : Function to open FPGA uio driver and do the memory map address
 * Inputs      : None
 *               
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int uio_open(void)
{
    int resourcefd;
    size_t size = UIO_SIZE_16MB; /* should get this from config space */
    char resource[80];
    char config[80];
    char uio_name[32];

    char str[80];

    if (uio_find_name(DRV_NAME, uio_name) >= 0) {
        sprintf(config, "/sys/class/uio/%s/device/config", uio_name);
        sprintf(resource, "/sys/class/uio/%s/device/resource0", uio_name);
    } else {
        printf("unable to open /sys/class/uio/uioX directory.\n");
        exit(0);
    }

    sprintf(str, "/dev/%s", uio_name);
    uiofd = open(str, O_RDONLY);
    if (uiofd < 0) {
        perror("uio open:");
        return (FAILED);
    }

    resourcefd = open(resource, O_RDWR);
    if (resourcefd < 0) {
        perror("config space resource0: open failed:");
        return (FAILED);
    }

    regs = mmap(0, size, PROT_READ|PROT_WRITE, MAP_SHARED, resourcefd, 0);
    if (MAP_FAILED == regs) {
        perror("mmap failed");
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : tabei_fpga_base_addr_init 
 * Description : Function point to FPGA base address initialization.
 * Inputs      : None 
 * Outputs     : None
 *
 *******************************************************************************
 */
void tabei_fpga_base_addr_init (void)
{
    dash_fpga = (unsigned long) regs;
    assert(dash_fpga);
}

/*******************************************************************************
 *
 * Function    : fpga_read_reg
 * Description : Function to read FPGA register.
 * Inputs      : reg_offset - register offset
 *               *buf       - buffer to put read back register value
 * Outputs     : PASSED
 *
 *******************************************************************************
 */
int fpga_read_reg (uint reg_offset, uint32_t *buf)
{
    assert(dash_fpga);
    *buf = *(unsigned int *)(dash_fpga + reg_offset);

    return (PASSED);
}


/*******************************************************************************
 *
 * Function    : fpga_write_reg
 * Description : Function performs FPGA register write.
 * Inputs      : reg_offset - register offset
 *               wr_data    - data for write
 * Outputs     : PASSED
 *
 *******************************************************************************
 */
int fpga_write_reg (uint reg_offset, uint wr_data)
{
    assert(dash_fpga);
    *(unsigned int *)(dash_fpga + reg_offset) = wr_data;

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : fpga_register_operation
 * Description: Function to control FPGA register
 * Inputs     : reg_offset - FPGA register address
 *              reg_mask   - FPGA mask
 *              set_val    - set the value to FPGA
 * Outputs    : PASSED /FAILED
 *
 *******************************************************************************
 */
int fpga_register_operation (uint reg_offset, uint reg_mask, uint set_val)
{
    uint32_t reg_val = 0;
    
    /* Access FPGA Register */
    if (fpga_read_reg(reg_offset, &reg_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }

    /* Logic operation */
    reg_val &= reg_mask;
    reg_val |= set_val; 
    if (fpga_write_reg(reg_offset, reg_val) != PASSED) {
        printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }
    msleep(DELAY_FOR_OPERATION);

    return(PASSED);

}

/*******************************************************************************
 *
 * Function    : fpga_reset_api
 * Description : Function of FPGA to reset/unreset interface.
 * Inputs      : r_offset  - register offset
 *               r_bit     - reset bit of register
 *               r_opt     - reset(TRUE)/un-reset(FALSE)
 *               r_time_ms - the reset time interval(millisecond)
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int fpga_reset_api (uint r_offset, uint r_bit, uint r_opt, uint r_time_ms)
{
    uint reg_val = 0;

    /* Read FPGA interface reset register. */
    if (fpga_read_reg(r_offset, &reg_val) != PASSED) {
        printf("%s: Failed to read FPGA reg.(0x%04X).\n",
               __FUNCTION__, r_offset);
        return (FAILED);
    }

    if (r_opt == TRUE) {
        /* Set the Reset bit. */
        reg_val |= r_bit;
    } else if (r_opt == FALSE) {
        /* Clear the reset bit. */
        reg_val &= (uint)(~r_bit);
    } else {
        printf("%s: Invalid Reset option(%#x).\n", __FUNCTION__, r_opt);
        return (FAILED);
    }

    /* Write the reset/un-reset into the corresponding register bit. */
    if (fpga_write_reg(r_offset, reg_val) != PASSED) {
        printf("%s: Failed to write FPGA reg.(0x%04X).\n",
               __FUNCTION__, r_offset);
        return (FAILED);
    }

    /* Delay milliseconds after reset/un-reset */
    msleep(r_time_ms);

    /* Confirm the change to FPGA interface reset register. */
    reg_val = 0;
    if (fpga_read_reg(r_offset, &reg_val) != PASSED) {
        printf("%s: Failed to read FPGA reg.(0x%04X).\n",
               __FUNCTION__, r_offset);
        return (FAILED);
    }

    if (((r_opt == TRUE) && ((reg_val & r_bit) != r_bit)) ||
        ((r_opt == FALSE) && ((reg_val & r_bit) != 0))) {
        printf("%s: Failed to %s reset bit in FPGA reg.(0x%04X).\n",
               __FUNCTION__, (r_opt == TRUE) ? "set" : "clear", r_offset);
        return (FAILED);
    }

     return (PASSED);
}

/*-------------------------------------------------------------------
 *
 * Function: get_platform_uart_addr
 * Description: get uart adddress
 *
 * Input: NONE
 *
 * Output: address
 *
 *-------------------------------------------------------------------
 */
unsigned long get_platform_uart_addr (int i)
{
    return (dash_fpga + (FPGA_UART_BASE) + (i * FPGA_UART_OFFSET));
}

/*-------------------------------------------------------------------
 *
 * Function: dash_uart_reset
 * Description: Reset UART
 *
 * Input: NONE
 *
 * Output: address
 *
 *-------------------------------------------------------------------
 */
void dash_uart_reset (int port)
{
    uart_t *uart;
    uart = (uart_t *)get_platform_uart_addr(port); /* uart 8 */
    uart->fcr = 0xC6;   /* tx rx reset */
    uart->mcr &= ~0x10; /* turn off loopback mode */
    return;
}


/*-------------------------------------------------------------------
 *
 * Function : dash_uart_tx
 * Description: write a string to a given uart port
 * INPUT:  port         - uart port
 *         test_str     - test data that will be transmitted
 *         test sz      - size of test data
 *         baud    - baud rate
 *  
 *         is_int_lpbk  - enable/disable UART FPGA internal loopback setting
 *                        TRUE : enable internal loopback
 *                        FALSE: disable internal loopback
 * RETURN:
 *         PASSED/FAILED
 * -------------------------------------------------------------------
*/
int dash_uart_tx (int port, int baud, char* tx_str, int tx_sz, int is_int_lpbk)
{
    unsigned int idx;
    uart_t *uart;
    unsigned int quot;
    char dll, dlm; // division latch least significant and most significant

    uart = (uart_t *)get_platform_uart_addr(port);

    quot = 50000000 / baud;
    dll = quot & 0xFF;
    dlm = (quot & 0xFF00) >> 8;

    uart->fcr = 0xC6;   /* tx rx reset */

    /* setup baud rate */
    uart->lcr = 0x83;   /* 0xc */
    uart->dll = dll;
    uart->dlm = dlm;

    uart->lcr = 3;
    uart->fcr = 0x1; /*enable FIFO and 1 byte trigger level */
    if (!tx_sz)
        return(PASSED);

    if (is_int_lpbk) {
        uart->mcr = 0x10;   /* turn on looopback mode */
    } else {
        uart->mcr &= ~0x10;     /* turn off looopback mode */

    }
    for (idx = 0; idx < tx_sz; idx++) {
        uart->dll = (tx_str[idx] & 0xFF);
        usleep(1000);
    }

    return(PASSED);
}

/*-------------------------------------------------------------------
 *
 * Function : uart_rx
 * Description: try to retreive data at the uart port
 * INPUT:  port         - uart port
 * OUTPUT:
 *          rx_str   -   data received
 *          rx_sz    -   size of data received
 * RETURN:
 *         PASSED/FAILED
 * -------------------------------------------------------------------
*/
int dash_uart_rx (int port, int *rx_sz, char* rx_str)
{
    uart_t *uart;
    int cnt = 0;
    char* c;

    uart = (uart_t *)get_platform_uart_addr(port); /* uart 8 */
    cnt = 0;
    c = rx_str;
    while (uart->lsr & 1) {
        c[cnt] = uart->dll;
        cnt++;
        if (*rx_sz > 0) {
            if (cnt >= *rx_sz)
                return(PASSED);
        }
        usleep(2000); /*delay is important: works for baud 9600 */
    }
    *rx_sz = cnt;
    return(PASSED);
}


/*-------------------------------------------------------------------
 *
 * Function : uart_lpbk_txrx
 * Description: write a string to a given uart port and try to retreive data
 * INPUT:  port         - uart port
 *         test_str     - test data that will be transmitted
 *         test sz      - size of test data
 *         baud    - baud rate
 *  
 *         is_int_lpbk  - enable/disable UART FPGA internal loopback setting
 *                        TRUE : enable internal loopback
 *                        FALSE: disable internal loopback
 * OUTPUT:
 *          rx_str   -   data received
 *          rx_sz    -   size of data received
 * RETURN:
 *         PASSED/FAILED
 * -------------------------------------------------------------------
*/
int uart_lpbk_txrx (int port, char* test_str, int test_sz, char* rx_str,
                int *rx_sz, int baud, int is_int_lpbk)
{   

    dash_uart_reset(port);

    dash_uart_tx(port, baud, test_str, test_sz, is_int_lpbk);
    dash_uart_rx(port, rx_sz, rx_str);

    dash_uart_reset(port);

    return(PASSED);
}


/*-------------------------------------------------------------------
 *
 * Function: get_platform_env_fan_base
 * Description: get Environmental Fan Control Register 0x32200
 *              ( Utah, Sword and Dagger only)
 * Input: NONE
 *
 * Output: address
 *
 *-------------------------------------------------------------------
 */
unsigned long get_platform_env_fan_base (void)
{
    unsigned long addr = 0;
    assert(dash_fpga);

    addr = ((unsigned long)dash_fpga) + FPGA_ENV_FAN_OFFSET;

    return addr;
}

/*-------------------------------------------------------------------
 *
 * Function: set_nios_mode
 *  to set nios mode to normal mode, disable mode, or diagnostic mode
 * 
 *
 * Input: mode: NIOS mode, NIOS_DISABLE_MODE (0), 
 *              NIOS_NORMAL_MODE (0x1), NIOS_DIAG_MODE (0x3)
 *
 * Output: PASSED/FAILED
 *
 *-------------------------------------------------------------------
 */
int set_nios_mode (int mode)
{
    volatile uint16_t *msg;
    volatile uint16_t *msg_status;
    volatile uint16_t *version;
    int count = 0;

    unsigned long addr = get_platform_env_fan_base();
    env_fan_t *env_fan = (env_fan_t *)addr;
    env_fan->ctrl = SMART_FAN_ENABLE;


    msg = (volatile uint16_t *)(dash_fpga + NIOS_MODE_REG);
    msg_status = (volatile uint16_t *)(dash_fpga + NIOS_STATUS_REG);
    version = (volatile uint16_t *)(dash_fpga + NIOS_VERSION_REG);

   /* HW suggest to simpify NIOS setup algorithm,
    *  10 times and 300000 us for each polling */
    for (count = 0; count < NIOS_MAX_RETRY; count++) {
        if (mode == NIOS_DISABLE_MODE) {
            if (*msg_status != NIOS_NORMAL_CHECK) {
                break;
            }
        } else { /* normal mode */
            if (*msg_status == NIOS_NORMAL_CHECK) {
                break;
            }
        }
        usleep(NIOS_POLLING_DELAY);
        *msg = mode;
    }

    if (count == NIOS_MAX_RETRY) {
        printf("Failed to setup NIOS mode @0x34010 = %d\n", mode);
        printf("NIOS status register @0x34000 = 0x%x\n", *msg_status);
        return (FAILED);
    } else {
        return (PASSED);
    }
}

/*-------------------------------------------------------------------
 *
 * Function: get_platform_aikido_addr
 * Description: get aikido address
 * 0x31A00 FPGA configuration SPI PROM programming register
 * Input: NONE
 *
 * Output: address
 *
 *-------------------------------------------------------------------
 */
unsigned long get_platform_aikido_addr (void)
{
    assert(dash_fpga);
    return ((unsigned long)dash_fpga + FPGA_AIKIDO_SPI_MASTER_OFFSET);
}

/*********************************************************************
 *
 * Function:    smartfan_is_busy
 *
 * Description: FPGA check smart fan is busy
 *
 * Inputs:      NONE
 *
 * Output:      NONE
 *
 *********************************************************************
 */
boolean smartfan_is_busy (void) {
    unsigned long addr = get_platform_env_fan_base();
    fan_envmnt_reg_t *env_fan = (fan_envmnt_reg_t *)addr;
    if ((env_fan->env_fan_smartfan_status & FAN_SMARTFAN_STAT_BUSY)
         == FAN_SMARTFAN_STAT_BUSY) {
        return (TRUE);
    } else {
        return (FALSE);
    }
}

/*********************************************************************
 *
 * Function:    smartfan_fifo_empty
 *
 * Description: FPGA check smart fan fifo empty
 *
 * Inputs:      NONE
 *
 * Output:      TRUE/FALSE
 *
 *********************************************************************
 */
boolean smartfan_fifo_empty (void) {

    unsigned long addr = get_platform_env_fan_base();
    fan_envmnt_reg_t *env_fan = (fan_envmnt_reg_t *)addr;
    if ((env_fan->env_fan_smartfan_status & FAN_SMARTFAN_STAT_FIFO_EMPTY)
         == FAN_SMARTFAN_STAT_FIFO_EMPTY) {
        return (TRUE);
    } else {
        return (FALSE);
    }
}

/*********************************************************************
 *
 * Function:    smartfan_fifo_rd
 *
 * Description: FPGA smart fan fifo read
 *
 * Inputs:      NONE
 *
 * Output:      Data
 *
 *********************************************************************
 */
uchar smartfan_fifo_rd (void) {

    unsigned long addr = get_platform_env_fan_base();
    fan_envmnt_reg_t *env_fan = (fan_envmnt_reg_t *)addr;
    return ((unsigned char)(env_fan->env_fan_smartfan_fifo));
}

/*********************************************************************
 *
 * Function:    smartfan_start
 *
 * Description: smart fan start
 *
 * Inputs:      Fan number
 *
 * Output:      NONE
 *
 *********************************************************************
 */
void smartfan_start (uchar fan_num) {

    unsigned long addr = get_platform_env_fan_base();
    fan_envmnt_reg_t *env_fan = (fan_envmnt_reg_t *)addr;
    uint data = ((fan_num << FAN_SMARTFAN_CTRL_FAN_OFFSET)
                 | FAN_SMARTFAN_CTRL_START);
    env_fan->env_fan_smartfan_control = data;
}

/*******************************************************************************
 *
 * Function   : is_promethium
 * Description: Return TRUE if platform is Promethium
 *              This function returns FALSE by default.
 * Inputs     : None 
 * Outputs    : TRUE / FALSE
 *
 *******************************************************************************
 */
int is_promethium (void)
{
    
    uint reg_addr = (uint)FPGA_MASTER_REV_REG;
    uint btype = 0;

    if (fpga_read_reg(reg_addr, &btype) != PASSED) {
        printf("%s: Failed to read FPGA Borad Type Reg(0x%04X).\n",
               __FUNCTION__, reg_addr);
        return (FAILED);
    }

    btype = ((btype & FPGA_BTYPE_SKU_BIT)>>FPGA_BTYPE_SKU_BIT_SHIFT);
    
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s: FPGA board type %08X.\n", __FUNCTION__, btype);
    }

    if ((btype == FPGA_BTYPE_PROMETHIUM ) || (btype == FPGA_BTYPE_PROMETHIUM_L)) {
        return (TRUE);
    }
    return (FALSE);
}
/*******************************************************************************
 *
 * Function   : is_not_promethium
 * Description: Return FALSE if platform is Promethium
 *              This function returns TRUE by default.
 * Inputs     : None 
 * Outputs    : TRUE / FALSE
 *
 *******************************************************************************
 */

int is_not_promethium (void)
{   
    if (is_promethium() == TRUE){
        return (FALSE);
    }
    return (TRUE);
}

/*******************************************************************************
 *
 * Function   : is_promethium_l
 * Description: Return TRUE if platform is Proemthium-L
 *              This function returns FALSE by default.
 * Inputs     : None 
 * Outputs    : TRUE / FALSE
 *
 *******************************************************************************
 */

int is_promethium_l (void)
{ 
    uint reg_addr = (uint)FPGA_MASTER_REV_REG;
    uint btype = 0;

    if (fpga_read_reg(reg_addr, &btype) != PASSED) {
        printf("%s: Failed to read FPGA Borad Type Reg(0x%04X).\n",
               __FUNCTION__, reg_addr);
        return (FAILED);
    }

    btype = ((btype & FPGA_BTYPE_SKU_BIT)>>FPGA_BTYPE_SKU_BIT_SHIFT);
    
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s: FPGA board type %08X.\n", __FUNCTION__, btype);
    }

    if (btype == FPGA_BTYPE_PROMETHIUM_L) {
        return (TRUE);
    }

    return (FALSE);
}

/*******************************************************************************
 *
 * Function   : has_4core
 * Description: Reture ture when the sku has 4 core CPU
 *              This function returns FALSE by default.
 * Inputs     : None 
 * Outputs    : TRUE / FALSE
 *
 *******************************************************************************
 */

int has_4core (void)
{   
    if (is_promethium_l() == TRUE){
        return (TRUE);
    }
    return (FALSE);
}

/*******************************************************************************
 *
 * Function   : is_fortnite
 * Description: Return TRUE if platform is Fortnite
 *              This function returns FALSE by default.
 * Inputs     : None 
 * Outputs    : TRUE / FALSE
 *
 *******************************************************************************
 */

int is_fortnite (void)
{ 
    uint reg_addr = (uint)FPGA_MASTER_REV_REG;
    uint btype = 0;

    if (fpga_read_reg(reg_addr, &btype) != PASSED) {
        printf("%s: Failed to read FPGA Borad Type Reg(0x%04X).\n",
               __FUNCTION__, reg_addr);
        return (FAILED);
    }

    btype = ((btype & FPGA_BTYPE_SKU_BIT)>>FPGA_BTYPE_SKU_BIT_SHIFT);
    
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s: FPGA board type %08X.\n", __FUNCTION__, btype);
    }

    if (btype == FPGA_BTYPE_FORTNITE) {
        return (TRUE);
    }

    return (FALSE);
}

/*******************************************************************************
 *
 * Function   : is_not_fortnite
 * Description: Return FALSE if platform is Fortnite
 *              This function returns TRUE by default.
 * Inputs     : None 
 * Outputs    : TRUE / FALSE
 *
 *******************************************************************************
 */

int is_not_fortnite (void)
{   
    if (is_fortnite() == TRUE){
        return (FALSE);
    }
    return (TRUE);
}

/*-------------------------------------------------------------------
 *
 * Function : is_tabeil
 * Description: Return TRUE if platform is Tabei-L
 *              This function returns FALSE by default. If platform
 *              is Tabei-L, declare this function in platform code
 *              
 * INPUT:  dummy -- not used.
 * OUTPUT: return TRUE or FALSE
 * -------------------------------------------------------------------
 */
int is_tabeil (void)
{
    uint reg_addr = (uint)FPGA_MASTER_REV_REG;
    uint btype = 0;

    if (fpga_read_reg(reg_addr, &btype) != PASSED) {
        printf("%s: Failed to read FPGA Borad Type Reg(0x%04X).\n",
               __FUNCTION__, reg_addr);
        return (FAILED);
    }

    btype = ((btype & FPGA_BTYPE_SKU_BIT)>>FPGA_BTYPE_SKU_BIT_SHIFT);
    
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s: FPGA board type %08X.\n", __FUNCTION__, btype);
    }

    if (btype == FPGA_BTYPE_TABEI_L) {
        return (TRUE);
    }
    return (FALSE);

}

/*******************************************************************************
 *
 * Function   : is_not_tabeil
 * Description: Return FALSE if platform is Tabei-L
 *              This function returns TRUE by default.
 * Inputs     : None 
 * Outputs    : TRUE / FALSE
 *
 *******************************************************************************
 */

int is_not_tabeil (void)
{   
    if (is_tabeil() == TRUE){
        return (FALSE);
    }
    return (TRUE);
}

/*******************************************************************************
 *
 * Function   : has_nim
 * Description: Return TRUE if platform has NIM
 *              This function returns FALSE by default.
 * Inputs     : None 
 * Outputs    : TRUE / FALSE
 *
 *******************************************************************************
 */

int has_nim (void)
{   
    if ((is_tabeil() | is_promethium()) == TRUE){
        return (TRUE);
    }
    return (FALSE);
}

/*******************************************************************************
 *
 * Function   : has_pim
 * Description: Return TRUE if platform has PIM
 *              This function returns FALSE by default.
 * Inputs     : None 
 * Outputs    : TRUE / FALSE
 *
 *******************************************************************************
 */

int has_pim (void)
{   
    if ((is_tabeil() | is_promethium()) == TRUE){
        return (TRUE);
    }
    return (FALSE);
}

/*******************************************************************************
 *
 * function   : has_dimm_slot
 * description: return true if platform has nim
 *              this function returns false by default.
 * inputs     : none 
 * outputs    : true / false
 *
 *******************************************************************************
 */

int has_dimm_slot (void)
{   
    if ((is_tabeil() | is_promethium()) == TRUE){
        return (TRUE);
    }
    return (FALSE);
}

/*******************************************************************************
 *
 * Function   : has_emmc
 * Description: Return TRUE if platform has eMMC
 *              This function returns FALSE by default.
 * Inputs     : None 
 * Outputs    : TRUE / FALSE
 *
 *******************************************************************************
 */

int has_emmc (void)
{   
    if ((is_promethium() | is_fortnite()) == TRUE){
        return (TRUE);
    }
    return (FALSE);
}

/*******************************************************************************
 *
 * Function   : has_hdd
 * Description: Return TRUE if platform has HDD
 *              This function returns FALSE by default.
 * Inputs     : None 
 * Outputs    : TRUE / FALSE
 *
 *******************************************************************************
 */

int has_hdd (void)
{   
    if (is_tabeil() == TRUE){
        return (TRUE);
    }
    return (FALSE);
}

/*******************************************************************************
 *
 * Function   : has_tpm
 * Description: Return TRUE if platform has TPM
 *              This function returns FALSE by default.
 * Inputs     : None 
 * Outputs    : TRUE / FALSE
 *
 *******************************************************************************
 */

int has_tpm (void)
{   
    if (is_tabeil() == TRUE){
        return (TRUE);
    }
    return (FALSE);
}

/*******************************************************************************
 *
 * Function   : has_phy1514
 * Description: Return TRUE if platform has GE PHY 1514
 *              This function returns FALSE by default.
 * Inputs     : None 
 * Outputs    : TRUE / FALSE
 *
 *******************************************************************************
 */

int has_phy1514 (void)
{   
    if (is_tabeil() == TRUE){
        return (TRUE);
    }
    return (FALSE);
}

/*******************************************************************************
 *
 * Function   : has_phy1543
 * Description: Return TRUE if platform has GE PHY 1543
 *              This function returns FALSE by default.
 * Inputs     : None 
 * Outputs    : TRUE / FALSE
 *
 *******************************************************************************
 */

int has_phy1543 (void)
{   
    if (is_fortnite() == TRUE){
        return (TRUE);
    }
    return (FALSE);
}

/*******************************************************************************
 *
 * Function   : has_esw6390
 * Description: Return TRUE if platform has GE Switch 6390
 *              This function returns FALSE by default.
 * Inputs     : None 
 * Outputs    : TRUE / FALSE
 *
 *******************************************************************************
 */

int has_esw6390 (void)
{   
    if (is_fortnite() == TRUE){
        return (TRUE);
    }
    return (FALSE);
}

/*******************************************************************************
 *
 * Function   : has_i350
 * Description: Return TRUE if platform has I350
 *              This function returns FALSE by default.
 * Inputs     : None 
 * Outputs    : TRUE / FALSE
 *
 *******************************************************************************
 */

int has_i350 (void)
{   
    if ((is_tabeil() | is_promethium()) == TRUE){
        return (TRUE);
    }
    return (FALSE);
}

/*******************************************************************************
 *
 * Function   : has_m2sata
 * Description: Return TRUE if platform has M.2 SATA
 *              This function returns FALSE by default.
 * Inputs     : None 
 * Outputs    : TRUE / FALSE
 *
 *******************************************************************************
 */

int has_m2sata (void)
{   
    if (is_fortnite() == TRUE){
        return (TRUE);
    }
    return (FALSE);
}

/*******************************************************************************
 *
 * Function   : has_m2pcie
 * Description: Return TRUE if platform has M.2 PCIe
 *              This function returns FALSE by default.
 * Inputs     : None 
 * Outputs    : TRUE / FALSE
 *
 *******************************************************************************
 */

int has_m2pcie (void)
{   
    if ((is_tabeil() | is_promethium()) == TRUE){
        return (TRUE);
    }
    return (FALSE);
}

/*******************************************************************************
 *
 * Function   : has_bios_eeprom
 * Description: Return TRUE if platform has BIOS EEPROM
 *              This function returns FALSE by default.
 * Inputs     : None 
 * Outputs    : TRUE / FALSE
 *
 *******************************************************************************
 */

int has_bios_eeprom (void)
{   
    if ((is_tabeil() | is_fortnite()) == TRUE){
        return (TRUE);
    }
    return (FALSE);
}

/*******************************************************************************
*
* Function   : has_barometer
* Description: Return TRUE if platform has barometer
*              This function returns FALSE by default.
* Inputs     : None 
* Outputs    : TRUE / FALSE
*
*******************************************************************************
*/

int has_barometer (void)
{   
   if ((is_promethium() | is_fortnite()) == TRUE){
       return (TRUE);
   }
   return (FALSE);
}

/*******************************************************************************
 *
 * Function   : check_poe_psu_present
 * Description: Return if the expected 12V PoE PSU is installed.
 * Inputs     : psu_no - No. of PoE PSU that will be checked
 *              option - test mode (QUICK_MODE & FULL_MODE)
 * Outputs    : TRUE/FALSE
 *
 *******************************************************************************
 */
uint32_t check_poe_psu_present (uint32_t psu_no, uint32_t option)
{
    if (fpga_poe_detect() == FAILED) {
        return (FALSE);
    }

    return (TRUE);
}


/*-------------------------------------------------
 * $Log: diag_fpga_lib.c,v $
 * Revision 1.3  2020/10/07 08:20:48  kehuang2
 * CSCvv99413: Collapse Promethium-L into main trunk
 *
 * Revision 1.2  2019/10/17 02:16:21  kehuang2
 * Collapse Tabei-L into main trunk
 *
 * Revision 1.1.2.28  2019/10/01 10:00:31  olin2
 * Support check POE present
 *
 * Revision 1.1.2.27  2019/10/01 08:55:35  olin2
 * Code clean up
 *
 * Revision 1.1.2.26  2019/09/27 07:57:23  kehuang2
 * Clean up code
 *
 * Revision 1.1.2.25  2019/09/02 08:38:02  olin2
 * support display smart fan info util
 *
 * Revision 1.1.2.24  2019/08/26 07:55:00  kehuang2
 * Clean up code by the comment of code review
 *
 * Revision 1.1.2.23  2019/08/06 07:20:28  kehuang2
 * Update present function base on the comment of code review
 *
 * Revision 1.1.2.22  2019/07/09 06:11:31  kehuang2
 * Update I2C bus change and enhence I2C scan coverage
 *
 * Revision 1.1.2.21  2019/07/04 03:23:36  kehuang2
 * Combine Tabei-L sereies image together(Fortnite, Tabei-L, Promethium)
 *
 * Revision 1.1.2.20  2019/06/20 06:21:13  kehuang2
 *
 * 1. Support linux_block_test function
 * 2. Update Diag menu item base on currently project information
 *
 * Revision 1.1.2.19  2019/06/06 09:29:48  kehuang2
 * Update menu item display function
 *
 * Revision 1.1.2.18  2019/05/29 03:16:17  kehuang2
 *
 * 1.Merge image according to official board type.
 * 2.Reform the structure of diag menu
 *
 * Revision 1.1.2.17  2019/03/26 06:09:16  olin2
 * Support Dreamliner on Tabei-L
 *
 * Revision 1.1.2.16  2019/03/19 10:46:17  kehuang2
 * Clean up code
 *
 * Revision 1.1.2.15  2019/03/19 09:26:26  kehuang2
 * Merge Sku1 and Sku2 into same image
 *
 * Revision 1.1.2.14  2019/01/21 10:42:09  harrchan
 * Update for sku1 future use
 *
 * Revision 1.1.2.13  2019/01/18 02:31:46  harrchan
 * Update code after code review
 *
 * Revision 1.1.2.12  2019/01/03 03:16:48  harrchan
 * Add distinguish sku function
 *
 * Revision 1.1.2.11  2018/12/25 07:24:38  olin2
 * Clean up code
 *
 * Revision 1.1.2.10  2018/12/05 06:39:20  olin2
 * Update Fan control for NIOS
 *
 * Revision 1.1.2.9  2018/11/28 07:37:27  olin2
 * Update fan util
 *
 * Revision 1.1.2.8  2018/11/16 05:42:09  olin2
 * Clean up code
 *
 * Revision 1.1.2.7  2018/11/15 06:56:06  olin2
 * initial commit for Fan utils
 *
 * Revision 1.1.2.6  2018/10/29 01:29:55  olin2
 * Remove unused function
 *
 * Revision 1.1.2.5  2018/10/18 11:06:52  olin2
 * Support NIM testcard UART test
 *
 * Revision 1.1.2.4  2018/10/15 12:30:12  kodko
 * Add CPLD register read/write function.
 *
 * Revision 1.1.2.3  2018/10/15 09:31:32  kodko
 * Porting FPGA UIO driver read/write function.
 *
 * Revision 1.1.2.2  2018/10/08 06:34:21  harrchan
 * Increase FPGA read write function
 *
 * Revision 1.1.2.1  2018/10/02 01:49:58  harrchan
 * Initial commit for Tabei-L P1A bring up.
 *
 * $Endlog$
 *-------------------------------------------------
 */
