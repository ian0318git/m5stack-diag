/* $Id: tam_act2_api_drv_support.c,v 1.2 2019/06/14 05:24:52 mikech2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/katar/x86/tam_act2_api_drv_support.c,v $
 *------------------------------------------------------------------
 *
 * tam_act2_api_drv_support_katar.c :
 *
 * This file provides the functions to support the API developed for ACT2 chip.
 *
 * DEC 2014 - Ian Chang
 * Ported by Kody Ko
 *
 * Copyright (c) 2014-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 */


#include "platform_aikido.h"


static int katar_i2c_adapter_fd;
static boolean katar_lib_initialized = FALSE;
static boolean bEnableDebug = FALSE;
static boolean bSMBus = FALSE;

/***********************************************************************
 *  External Functions Declaration
 ************************************************************************/
extern void *tam_act2_get_n2g_i2c_if(void);
extern int get_i2c_fd(int); 
extern int read_i2c_reg_aikido (uint8_t *, uint16_t);
extern int write_i2c_reg_aikido (uint8_t *, uint32_t);
extern void msleep (int t);
extern int read_i2c_reg_aikido_1byte (uint8_t *);
extern unsigned long get_aikido_reg_base(void);
extern int aikido_read_32_reg (uint32_t, uint32_t *);
extern int aikido_write_32_reg (uint32_t, uint32_t);
extern uint32_t katar_n2g_i2c_write(n2g_i2c_if_t *);
extern uint32_t katar_n2g_i2c_read(n2g_i2c_if_t *);

/***********************************************************************
 *  Local Functions Declaration
 ************************************************************************/
int katar_diagact2_lib_initialize(char *, int);
void reset_tam_aikido_dev(void);
static int katar_set_i2c_slave_addr(int);
int is_tam_aikido_mbox_on(void);
int is_tam_aikido_on(void); 

extern int act2_i2c_debug;
extern boolean aikido_mailbox_flag;
//static int i2c_adapter_fd;
static int rsleepms = 0; 

typedef struct AHBL_LPC_reg_map_t_ {
  unsigned int AHBL_addr;  /* AHBL addr. */ 
  unsigned int LPC_addr;   /* LPC addr. */ 
} AHBL_LPC_reg_map_t;

static AHBL_LPC_reg_map_t AHBL_LPC_reg_map[] = {
    {0xE000, 0x700},  /* 2KB TAM mailbox buffer */
    {0xC000, 0x300},  /* TAM Mailbox */
    {0x3100, 0x400},  /* Secure JTAG Returned Data Storage */
    {0x3000, 0x330},  /* Secure JTAG Registers */
    {0x2000, 0x200},  /* TAM Resource Controller */
    {0x1000, 0x100},  /* Host Interrupt Controller */
    {0x0100, 0x500},  /* 512Byte Secure Boot TLA trace buffer */
    {0x0000, 0x000},  /* Secure Boot registers */
};




/*-------------------------------------------------------------------
 *
 * Function: reset_plat_dev
 * reset device on control plan
 *
 * Input: bit mask representing device to be reset
 * Output: NONE
 *
 *-------------------------------------------------------------------
 */
void reset_plat_dev(unsigned int mask)
{
    if (diagflag_xram & D_DEBUG_OPTIONS) {
        printf("===== %s %d =====\n",__FUNCTION__,__LINE__);
    }
}



/*-------------------------------------------------------------------
 *
 * Function: unreset_plat_dev
 * unreset device on control plan
 *
 * Input: bit mask representing device to be reset
 * Output: NONE
 *
 *-------------------------------------------------------------------
 */
void unreset_plat_dev(unsigned int mask)
{
    if (diagflag_xram & D_DEBUG_OPTIONS) {
        printf("===== %s %d =====\n",__FUNCTION__,__LINE__);
    }
}



/*
 * Function: katar_diagact2_lib_initialize()
 *
 * This function initializes the library by
 * setting the I2C bus number that the device is on
 * and which is used for subsequent library calls.
 *
 * Input: i2c_adapter - /dev/i2c-<bus number>
 *
 * Output: PASSED if initialized successfully, FAILED otherwise
 */
int katar_diagact2_lib_initialize(char *i2c_adapter, int addr)
{
    if (katar_lib_initialized) {
        return (PASSED);  // avoid lib init twice
    }

    /*
     * Open /dev/i2c-%d adapter device 
     */
    katar_i2c_adapter_fd = get_i2c_fd(0);

    if (katar_i2c_adapter_fd < 0) {
        printf("%s:open(%s) failed\n", __FUNCTION__, i2c_adapter);
        katar_lib_initialized = FALSE;
        return (FAILED);
    }

    printf("\nI2C addr: 0x%x ; %s\n",addr, __FUNCTION__);

    /*
     * Set the Slave I2C address for the accesses 
     */
    if (katar_set_i2c_slave_addr(addr) != SUCCEED) {
        printf("\n ERROR: Cannot set slave address.");
        close(katar_i2c_adapter_fd);
        katar_lib_initialized = FALSE;
        return (FAILED);
    }

    katar_lib_initialized = TRUE;
    return (PASSED);
}



/*
 * Function: katar_set_i2c_slave_addr()
 * Set ACT2 slave address
 *
 * Input: addr - I2C slave address
 *
 * Output: SUCCEED if set I2C slave address successfully
 */
static int katar_set_i2c_slave_addr(int addr)
{
    if (ioctl(katar_i2c_adapter_fd, I2C_SLAVE, addr) < 0) {
        printf("%s:ioctl(I2C_SLAVE, 0x%x) failed", __FUNCTION__, addr);
        return (FAILED);
    }

    return (SUCCEED);
}


/*
 * Function: act2_drv_write
 *
 * This function does the write to the ACT2 device. The FPGA is the master.
 *
 * Inputs: ptr to module -
 *         ptr to send_buf -
 *         length - number of bytes to write.
 *
 * Output: PASSED/FAILED
 */
int act2_drv_write (void *module, char *send_buf, unsigned int length)
{
    n2g_i2c_if_t i2c_if;
    n2g_i2c_if_t *n2g_i2c_act;
    int ret_val = 0;

    if (!length) {
        cterr('f',0," Error in act2_drv_write. length = %d", length);
        return(FAILED);
    }

    n2g_i2c_act = (n2g_i2c_if_t *)tam_act2_get_n2g_i2c_if();

    assert(n2g_i2c_act);
    
    memcpy(&i2c_if, n2g_i2c_act, sizeof(i2c_if));
    i2c_if.size = length;
    i2c_if.buf = send_buf;

    if (diagflag_xram & D_DEBUG_OPTIONS) {
        printf("offset = %u\n", i2c_if.offset);
        printf("i2c_bus_type = %u\n", i2c_if.i2c_bus_type);
        printf("i2c_dev = %u\n", i2c_if.i2c_dev);
        printf("i2c_ctrl = %u\n", i2c_if.i2c_ctrl);
        printf("sub_addr_len = %u\n", i2c_if.sub_addr_len);
        printf("size = %u\n", i2c_if.size);
        printf("rd_hd_size = %u\n", i2c_if.rd_hd_size);
        printf("wr_hd_size = %u\n", i2c_if.wr_hd_size);
        printf("mux = %u\n", i2c_if.mux);
        printf("err_no = %u\n", i2c_if.err_no);
        printf("i2c_speed = %u\n", i2c_if.i2c_speed);
        printf("i2c_base = %lu\n", i2c_if.i2c_base);
        printf("dev_name = %s\n", i2c_if.dev_name);
        printf("katar_i2c_adapter_fd = %d\n", katar_i2c_adapter_fd);
    }

    if (diagflag_xram & D_DEBUG_OPTIONS) {
       uint ix;
       printf(" act2_api_drv_support.c write :i2c_ctrl %d; addr %#x; mux %d; len %d\n",
           i2c_if.i2c_ctrl, i2c_if.i2c_dev, i2c_if.mux, i2c_if.size);

        printf("\n");
        printf("\n ----- write buf within act2_drv_write --------\n");
        for (ix =0; ix < length; ix++) {
            printf(" %02x ", send_buf[ix]);
            if (ix && (ix % SEGMENT_I2C_READ == 0))
                printf("\n"); 
        }
        printf("\n ----------------------------------------------\n");
    }
    ret_val = katar_n2g_i2c_write(&i2c_if);
    if (ret_val == RC_I2C_SLV_NACK) {
        if (diagflag_xram & D_DEBUG_OPTIONS) {
            printf("Warning! fpga_i2c_normal_op() returns RC_I2C_SLV_NACK!\n");
        }
        return(0);
    } else if (ret_val != RC_I2C_OP_OK) {
    //if (ret_val != RC_I2C_OP_OK) {
        cterr('f',0," ERROR in act2_drv_write");
        return(0);
    } else {
        return(length); // No error detected 
    }
  
}

/*
 * Function: act2_drv_read
 *
 * This function does the read from the ACT2 device. The FPGA is the master.
 *
 * Inputs: ptr to module -
 *         ptr to status_buf -
 *         length - number of bytes to read.
 *
 * Output: PASSED/FAILED
 */
int act2_drv_read (void *module, char *receive_buf, unsigned int length)
{
    n2g_i2c_if_t i2c_if;
    n2g_i2c_if_t *n2g_i2c_act;
    int ret_val = 0;
    int size;
    uint ix;

    memset(receive_buf,0xCC,length);
    
    n2g_i2c_act = (n2g_i2c_if_t *)tam_act2_get_n2g_i2c_if();

    assert(n2g_i2c_act);

    memcpy(&i2c_if, n2g_i2c_act, sizeof(i2c_if));
    
    i2c_if.buf = receive_buf;
    size = (int)length;

    //if (act2_i2c_debug) {
    if (diagflag_xram & D_DEBUG_OPTIONS) {
        printf(" act2_api_drv_support.c read :i2c_ctrl %d; addr %#x; mux %d; len %d\n",
               i2c_if.i2c_ctrl, i2c_if.i2c_dev, i2c_if.mux, length);
    }

    while (size) {
        if (size > SEGMENT_I2C_READ) {
            i2c_if.size = SEGMENT_I2C_READ;
        } else {
            i2c_if.size = size;
        }
        size -= i2c_if.size;
        
        for (ix = 0; ix < ACT_RETRY; ix++) {
            ret_val = katar_n2g_i2c_read(&i2c_if);
            if (ret_val != RC_I2C_OP_OK) {
                msleep(ACT_DELAY);
                continue;
            } else {
                break;
            }
        }
        if (ret_val != RC_I2C_OP_OK) {
            cterr('f',0,"READ ERROR in act2_drv_read: error %d", ret_val);
            return(0);
        }
        i2c_if.buf += SEGMENT_I2C_READ;
    } 

    if (diagflag_xram & D_DEBUG_OPTIONS) {
        printf("\n");
        printf("\n ----- receive buf within act2_drv_read --------\n");
        for (ix =0; ix < length; ix++) {
            printf(" %02x ", receive_buf[ix] & 0xFF);
            if (ix && (ix % SEGMENT_I2C_READ == 0))
                printf("\n"); 
        }
        printf("\n ----------------------------------------------\n");
    }

    return(length);
}

void tam_lib_platform_debug(void *platform_opaque_handle,
                       boolean bSetting)
{
        bEnableDebug = bSetting;
        return;
}

void tam_lib_platform_add_delay(void *platform_opaque_handle,
                       int delayms)
{
        rsleepms = delayms;
        return;
}

void tam_lib_platform_smbus(void *platform_opaque_handle,
                       boolean SMBus_Or_FPGAI2C)
{
        bSMBus = SMBus_Or_FPGAI2C;
        return;
}

/*
 * Function: tam_lib_platform_write 
 *
 * This function does the write to the ACT2 device. The FPGA is the master.
 * Calls are made to this function from within the act2 tam libary located at.
 * /auto/sp-engops/diags/pld/act2lite/x86/WNBU/Katar_WLC/Aikido/tam_library_w_standalone_katar.a
 *
 * Inputs: ptr to platform_opaque_handle(module) -
 *         ptr to send_buf -
 *         length - number of bytes to write.
 *
 * Output: number of bytes writen
 *         -1 error
 */

int32_t tam_lib_platform_write(void *platform_opaque_handle,
                       uint8_t * send_buffer, uint32_t length)
{
    if (bEnableDebug) {
        int ix;
        printf("Writing %d bytes:\n", length);
        for (ix = 0; ix < length; ix++) {
            printf(" %02x", (unsigned char)send_buffer[ix]);
        }
        printf("\n");
    }

    uint8_t count = 0;
    int res;

    if (bSMBus) {
        if (diagflag_xram & D_DEBUG_OPTIONS) {
            printf("Use SMBus interface\n");  // Use SMBus
        }
retry1:
        if ( write_i2c_reg_aikido(send_buffer, length) != PASSED ) {
            if (count > ACT_RETRY) {
                printf("\n *** ERROR: unable to write from ACT2 device, error no is %d\n", errno);
                printf("Something went wrong with write()! Error description is: %s\n", strerror(errno));
                fflush(stdout);
                return (-1);
            } else {
                count++;
                msleep(100);
                goto retry1;
            }
        } else {
            return (length);
        }
    } else {
        if (diagflag_xram & D_DEBUG_OPTIONS) {
            printf("Use FPGA-I2C interface\n");  // Use FPGA-I2C
        }
retry2:
        res = act2_drv_write(platform_opaque_handle, (char *)send_buffer, length);
        //if (res == -1) {
        if (res <= 0) {
            if (count > ACT_RETRY) {
                printf("\n *** ERROR: unable to write from ACT2 device, error no is %d\n", errno);
                printf("Something went wrong with write()! Error description is: %s\n", strerror(errno));
                fflush(stdout);
                return (-1);
            } else {
                if (diagflag_xram & D_DEBUG_OPTIONS) {
                    printf("count=%d | res=%d\n", count, res);
                }
                count++;
                msleep(100);
                goto retry2;
            }
        } else {
            return (length);
        }
    } 






/*
    int res;
    int ix;
    n2g_i2c_if_t i2c_if;
    n2g_i2c_if_t *n2g_i2c_act;


    n2g_i2c_act = (n2g_i2c_if_t *)tam_act2_get_n2g_i2c_if();
    if (n2g_i2c_act == NULL) {
        printf("%s: n2g_i2c_act returns NULL\n", __func__);
        return (FAILED);
    }
    memcpy(&i2c_if, n2g_i2c_act, sizeof(i2c_if));

    //
    // Print one space to solve the corrupt printf buffer, if this is not
    // added, then there will be some weird chars print out.
    //
    printf(" ");
    fflush(stdout);

    //if (act2_i2c_debug) {
    if (diagflag_xram & D_DEBUG_OPTIONS) {
        printf("\n Inside tam_lib_platform_write. length = 0x%x \n",
               length);
        for (ix = 0; ix < length; ix++) {
            printf(" %02x", send_buffer[ix]);
        }
        printf("\n");
        fflush(stdout);
    }

    usleep(ACT_RW_DELAY);

    if (diagflag_xram & D_DEBUG_OPTIONS) {
        printf("offset = %u\n", i2c_if.offset);
        printf("i2c_bus_type = %u\n", i2c_if.i2c_bus_type);
        printf("i2c_dev = %u\n", i2c_if.i2c_dev);
        printf("i2c_ctrl = %u\n", i2c_if.i2c_ctrl);
        printf("sub_addr_len = %u\n", i2c_if.sub_addr_len);
        printf("size = %u\n", i2c_if.size);
        printf("rd_hd_size = %u\n", i2c_if.rd_hd_size);
        printf("wr_hd_size = %u\n", i2c_if.wr_hd_size);
        printf("mux = %u\n", i2c_if.mux);
        printf("err_no = %u\n", i2c_if.err_no);
        printf("i2c_speed = %u\n", i2c_if.i2c_speed);
        printf("i2c_base = %lu\n", i2c_if.i2c_base);
        printf("dev_name = %s\n", i2c_if.dev_name);
        printf("katar_i2c_adapter_fd = %d\n", katar_i2c_adapter_fd);
    }

    n2g_i2c_dev_t i2c_dev;
    i2c_dev.bus_no = i2c_if.i2c_bus_type;
    i2c_dev.dev_addr = i2c_if.i2c_dev;
    i2c_dev.rd_hd_size = i2c_if.rd_hd_size;
    i2c_dev.wr_hd_size = i2c_if.wr_hd_size;
    i2c_dev.fp = katar_i2c_adapter_fd;
    i2c_if.size = length;
    i2c_if.buf = (char *)send_buffer;

    if (diagflag_xram & D_DEBUG_OPTIONS) {
        printf("offset = %u\n", i2c_dev.bus_no);
        printf("dev_addr = %u\n", i2c_dev.dev_addr);
        printf("rd_hd_size = %u\n", i2c_dev.rd_hd_size);
        printf("wr_hd_size = %u\n", i2c_dev.wr_hd_size);
        printf("fp = %d\n", i2c_dev.fp);
        printf("offset = %u\n", i2c_if.offset);
        printf("size = %u\n", i2c_if.size);
    } 


    for (ix = 0; ix < ACT_RETRY; ix++) {

        if (i2c_if.i2c_bus_type == IOFPGA_I2C) {
            res = act2_drv_write(platform_opaque_handle, (char *)send_buffer, length);
        } else {
            //res = write(katar_i2c_adapter_fd, send_buffer, length);
            //res = api_mb_i2c_write(&i2c_dev, i2c_if.offset, i2c_if.size, (char *)i2c_if.buf);
            res = api_mb_i2c_write(&i2c_dev, i2c_if.offset, length, (char *)send_buffer);
        }
        
        if (res == -1) {
            msleep(100);
            continue;
        } else {
            break;
        }
    }
    if (ix >= ACT_RETRY) {
        printf("\n *** ERROR: unable to write from ACT2 device, error no is %d\n", 
              errno);
        printf("Something went wrong with write()! Error description is: %s\n", 
              strerror(errno));
        fflush(stdout);
        return (-1);
    }
    usleep(ACT_RW_DELAY);

    return (length);
*/
}


/*
 * Function: tam_lib_platform_read 
 *
 * This function does the read from the ACT2 device. The FPGA is the master.
 * Calls are made to this function from within the act2 tam libary located at.
 * /auto/sp-engops/diags/pld/act2lite/x86/WNBU/Katar_WLC/Aikido/tam_library_w_standalone_katar.a
 *
 * Inputs: ptr to module(platform_opaque_handle) -
 *         min_time - min time to wait (for future use)
 *         max_time - max time to wait (for future use)
 *         ptr to status_buf(receive_buf) -
 *         length - number of bytes to read.
 *         bytes_actually_read - number of bytes actually read.
 *
 * Output: TAM_RC_OK successfully
 *         -1   failed
 */
tam_lib_status_t tam_lib_platform_read(void *platform_opaque_handle,
                      uint32_t min_time,
                      uint32_t max_time,
                      uint8_t * read_buffer,
                      uint16_t bytes_to_read,
                      uint16_t * bytes_actually_read)
{
    int i;
    uint8_t count = 0;
    int res;

    if (rsleepms) {
        msleep(rsleepms);
        rsleepms = 0;
    } else {
        msleep(4);
    }

    if(bEnableDebug) {
            printf("Reading %d bytes:\n", bytes_to_read);
    }

    if (bSMBus) {
        if (diagflag_xram & D_DEBUG_OPTIONS) {
            printf("Use SMBus interface\n");  // Use SMBus
        }

        for (i = 0; i < bytes_to_read ; i++) {
retry1:
            if ( read_i2c_reg_aikido_1byte(&read_buffer[i]) != PASSED ) {
                if (count > ACT_RETRY) {
                    printf("\n *** ERROR: unable to read from ACT2 device, errno no is : %d\n", errno);
                    printf("Something went wrong with read()! Error description is: %s\n", strerror(errno));
                    fflush(stdout);
                    return (-1);
                } else {
                    count++;
                    msleep(100);
                    goto retry1;
                }
            } else if(bEnableDebug) {
                printf("0x%02x ", read_buffer[i]);
            }
            msleep(4);
        }
    }
    else {
        if (diagflag_xram & D_DEBUG_OPTIONS) {
            printf("Use FPGA-I2C interface\n");  // Use FPGA-I2C
        }
retry2:
        res = act2_drv_read(platform_opaque_handle, (char *)read_buffer, bytes_to_read);
        if (res == -1) {
            if (count > ACT_RETRY) {
                printf("\n *** ERROR: unable to read from ACT2 device, errno no is : %d\n", errno);
                printf("Something went wrong with read()! Error description is: %s\n", strerror(errno));
                fflush(stdout);
                return (-1);
            } else {
                count++;
                msleep(100);
                goto retry2;
            }
        } else if(bEnableDebug) {
            for (i = 0; i < bytes_to_read ; i++) {
                printf("0x%02x ", read_buffer[i]);
            }
        }
        msleep(4);
    } 

    if (bEnableDebug) {
        printf("\n");  // only needed when reading bytes are printed at read_i2c_reg_aikido_1byte
    }
    *bytes_actually_read = bytes_to_read;
    return (PASSED);

/*
    int res, ix;
    uint8_t count = 0;
    n2g_i2c_if_t i2c_if;
    n2g_i2c_if_t *n2g_i2c_act;


    n2g_i2c_act = (n2g_i2c_if_t *)tam_act2_get_n2g_i2c_if();
    if (n2g_i2c_act == NULL) {
        printf("%s: n2g_i2c_act returns NULL\n", __func__);
        return (FAILED);
    }
    memcpy(&i2c_if, n2g_i2c_act, sizeof(i2c_if));

    usleep(ACT_RW_DELAY);

retry:

    if (i2c_if.i2c_bus_type == IOFPGA_I2C) {
        res = act2_drv_read(platform_opaque_handle, (char *)read_buffer, bytes_to_read);
    } else {
    res = read(katar_i2c_adapter_fd, read_buffer, bytes_to_read);     // was bytes_to_read
    }

   if (res == -1) {
        if (count > ACT_RETRY) {
            printf("\n *** ERROR: unable to read from ACT2 device, errno no is : %d\n", 
                   errno);
            printf("Something went wrong with read()! Error description is: %s\n", 
                   strerror(errno));
            fflush(stdout);
            return (-1);
        } else {
            count++;
            msleep(100);
            goto retry;
        }
    }

    //if (act2_i2c_debug) {
    if (diagflag_xram & D_DEBUG_OPTIONS) {
        printf("\n Inside tam_lib_platform_read. length = 0x%x \n",
               bytes_to_read);
        for (ix = 0; ix < bytes_to_read; ix++) {
            printf(" %02x", read_buffer[ix]);
        }
        printf("\n");
        fflush(stdout);
    }

    usleep(ACT_RW_DELAY);
    *bytes_actually_read = bytes_to_read;

    return (PASSED);
*/
}

/*
 * Function: katar_tam_lib_platform_mbx_write 
 *
 * This function does the write to the ACT2 device. The FPGA is the master.
 * Calls are made to this function from within the act2 tam libary located at.
 * /auto/sp-engops/diags/pld/act2lite/ARM/TSN/tam_library_w_standalone_tsn_diag.a
 *
 * Inputs: ptr to platform_opaque_handle(module) -
 *         ptr to send_buf -
 *         length - number of bytes to write.
 *
 * Output: number of bytes written
 *         -1 error
 */
int32_t tam_lib_platform_mbx_write(void *platform_opaque_handle,
                           uint16_t bytes_to_send,
                           uint8_t *send_buffer)
{
    int ix = 0;; 
    uint16_t byte_count = 0;
    uint tx_reg_offset = 0;
    uint tx_data = 0;
    unsigned int AHBL_addr, LPC_addr;

    if (diagflag_xram & D_DEBUG_OPTIONS) {
        printf("\n\n Inside tam_lib_platform_mbx_write. length = 0x%x \n",
               bytes_to_send);
        for (ix = 0; ix < bytes_to_send; ix++) {
            printf(" %02x", send_buffer[ix]);
        }   
        printf("\n");
        fflush(stdout);
    }   

    usleep(ACT_RW_DELAY);

    AHBL_addr = (send_buffer[2] << 8) + send_buffer[1] + send_buffer[3];
    for (ix = 0; ix < sizeof(AHBL_LPC_reg_map); ix++) {
        if (AHBL_addr >= AHBL_LPC_reg_map[ix].AHBL_addr) {
            LPC_addr = AHBL_LPC_reg_map[ix].LPC_addr + (AHBL_addr - AHBL_LPC_reg_map[ix].AHBL_addr);
            break;
        }
    }
    if (diagflag_xram & D_DEBUG_OPTIONS) {
        printf("ix=%d | AHBL_addr=0x%x | LPC_addr=0x%x | AHBL_LPC_reg_map={0x%x 0x%x)\n", ix, AHBL_addr, LPC_addr, AHBL_LPC_reg_map[ix].AHBL_addr, AHBL_LPC_reg_map[ix].LPC_addr);
    }

    tx_reg_offset  = LPC_addr;
    byte_count = bytes_to_send - 4;
    for (ix = 0; byte_count > 0; ix++) {
        fflush(stdout);
        tx_data = (send_buffer[4 + (ix * 4) ] | 
                   send_buffer[4 + (ix * 4) + 1] << 8 | 
                   send_buffer[4 + (ix * 4) + 2] << 16 |
                   send_buffer[4 + (ix * 4) + 3] << 24);
        if (aikido_write_32_reg(tx_reg_offset + (ix * 4), tx_data) != PASSED) {
            printf("%s: Failed to write FPGA Reg0x%#X.\n", __FUNCTION__, tx_reg_offset + ix);
            return (-1);
        }   
        byte_count -= 4;
    }   
    return bytes_to_send;
}

/*
 * Function: tam_lib_platform_mbx_read 
 *
 * This function does the read from the ACT2 device. The FPGA is the master.
 * Calls are made to this function from within the act2 tam libary located at.
 * /auto/sp-engops/diags/pld/act2lite/ARM/TSN/tam_library_w_standalone_tsn_diag.a
 *
 * Inputs: ptr to module(platform_opaque_handle) -
 *         min_time - min time to wait (for future use)
 *         max_time - max time to wait (for future use)
 *         ptr to status_buf(receive_buf) -
 *         length - number of bytes to read.
 *         bytes_actually_read - number of bytes actually read.
 *
 * Output: TAM_RC_OK successfully
 *         -1   failed
 */
tam_lib_status_t tam_lib_platform_mbx_read(void *platform_opaque_handle,
                          uint16_t bytes_to_send,
                          uint8_t *send_buffer,
                          uint16_t bytes_to_read,
                          uint8_t *read_buffer,
                          uint16_t *bytes_actually_read)
{
    int ix = 0, jx = 0;
    uint rx_buffer = 0;
    uchar rx_buffer_tmp[3000];
    uint16_t tx_reg_offset = 0;
    uint16_t byte_count = 0, byte_shift = 0;
    unsigned int AHBL_addr, LPC_addr;
   
    if (diagflag_xram & D_DEBUG_OPTIONS) {
        printf("\n\n Inside tam_lib_platform_mbx_read. length = 0x%x \n",
               bytes_to_read);
        for (ix = 0; ix < bytes_to_send; ix++) {
            printf(" %02x", send_buffer[ix]);
        }
        fflush(stdout);
    }

    AHBL_addr = (send_buffer[2] << 8) + send_buffer[1] + send_buffer[3];
    for (ix = 0; ix < sizeof(AHBL_LPC_reg_map); ix++) {
        if (AHBL_addr >= AHBL_LPC_reg_map[ix].AHBL_addr) {
            LPC_addr = AHBL_LPC_reg_map[ix].LPC_addr + (AHBL_addr - AHBL_LPC_reg_map[ix].AHBL_addr);
            break;
        }
    }
    if (diagflag_xram & D_DEBUG_OPTIONS) {
        printf("ix=%d | AHBL_addr=0x%x | LPC_addr=0x%x | AHBL_LPC_reg_map={0x%x 0x%x)\n", ix, AHBL_addr, LPC_addr, AHBL_LPC_reg_map[ix].AHBL_addr, AHBL_LPC_reg_map[ix].LPC_addr);
    }

    tx_reg_offset  = LPC_addr;
    /* Read buffer header FF FF FF FF*/
    for (jx = 0; jx < 4; jx++) {
        read_buffer[jx] = 0xFF;
    }
    byte_shift = tx_reg_offset % 4;
    tx_reg_offset -= (tx_reg_offset % 4);

    byte_count = bytes_to_read - 4 + byte_shift;
    for (ix = 0; byte_count > 0; ix++) {
        if (aikido_read_32_reg(tx_reg_offset + (ix * 4), &rx_buffer) != PASSED) {
            printf("%s: Failed to write FPGA Reg0x%#X.\n", __FUNCTION__, tx_reg_offset + ix);
            return (-1);
        }
        for (jx = 0; jx < 4; jx++) {
            rx_buffer_tmp[(ix * 4) + jx] = (rx_buffer >> (8 * jx)) & 0xff;
        }
        if (byte_count >= 4) {
            byte_count -= 4;
        } else {
            byte_count = 0;
        }
    }
    for (ix = 0; ix < bytes_to_read - 4; ix++) {
        read_buffer[4 + ix] = rx_buffer_tmp[byte_shift + ix] & 0xff;
    }

    if (diagflag_xram & D_DEBUG_OPTIONS) {
        printf("\n Actually read length = 0x%x : ",
               bytes_to_read);
        for (ix = 0; ix < bytes_to_read; ix++) {
            printf(" %02x", read_buffer[ix]);
        }
        printf("\n");
        fflush(stdout);
    }
    *bytes_actually_read = bytes_to_read;
    return TAM_RC_OK;
}


/*-------------------------------------------------------------------
 *      
 * Function : is_tam_aikido_mbox_on
 * Description: Return TRUE if mailbox in Aikido is turned on
 * INPUT:  dummy -- not used.
 * OUTPUT: return TRUE or FALSE
 * -------------------------------------------------------------------
 */ 
int is_tam_aikido_mbox_on (void)
{   
    return (aikido_mailbox_flag);
}  


/*-------------------------------------------------------------------
 *      
 * Function : is_tam_aikido_on
 * Description: Return TRUE if ACT2 chip is Aikido
 * INPUT:  dummy -- not used.
 * OUTPUT: return TRUE or FALSE
 * -------------------------------------------------------------------
 */ 
int is_tam_aikido_on (void)
{   
    return (TRUE);
}  





/*
 *------------------------------------------------------------------
 * $Log: tam_act2_api_drv_support.c,v $
 * Revision 1.2  2019/06/14 05:24:52  mikech2
 * Collapse katar-branch00 to Main Trunk
 *
 * Revision 1.1.2.4  2019/03/14 03:58:52  peteteng
 * Code cleanup
 *
 * Revision 1.1.2.3  2019/03/13 03:34:15  peteteng
 * Code cleanup
 *
 * Revision 1.1.2.2  2019/02/12 08:06:31  mikech2
 * rename katar_*.h files
 *
 * Revision 1.1.2.1  2019/01/29 01:54:22  mikech2
 * rename katar_* files
 *
 * Revision 1.1.2.13  2019/01/09 03:25:22  peteteng
 * Add is_tam_aikido_on()
 *
 * Revision 1.1.2.12  2018/12/27 00:42:24  peteteng
 * Support Aikido thru UserLogic FPGA I2C
 *
 * Revision 1.1.2.11  2018/12/21 07:28:45  peteteng
 * Add ACT2 programming thru LPC
 *
 * Revision 1.1.2.10  2018/12/20 09:10:57  peteteng
 * Add FPGA I2C read/write/scan/reset util
 *
 * Revision 1.1.2.9  2018/12/14 02:06:04  mikech2
 * Fix Akido FPGA SMBus read issue
 *
 * Revision 1.1.2.8  2018/12/13 15:41:07  peteteng
 * Fix cookie util in Aikido FW-v10015
 *
 * Revision 1.1.2.7  2018/12/12 02:03:39  peteteng
 * Add Aikido FW upgrade through LPC
 *
 * Revision 1.1.2.6  2018/12/06 08:32:25  mikech2
 * Fine-tune Aikido I2C r/w and fix Aikido update FW utility
 *
 * Revision 1.1.2.5  2018/12/01 10:39:01  peteteng
 * Speed up Aikido cookie
 *
 * Revision 1.1.2.4  2018/11/29 03:19:58  peteteng
 * Fix Aikido cookie - read one byte
 *
 * Revision 1.1.2.3  2018/11/17 11:09:49  peteteng
 * Fix Aikido cookie issue
 *
 * Revision 1.1.2.2  2018/11/08 02:21:52  peteteng
 * Remove names in comment
 *
 * Revision 1.1.2.1  2018/10/22 08:02:31  mikech2
 * Move project folder to common/src/katar/x86
 *
 * Revision 1.1.2.6  2018/10/22 03:04:32  peteteng
 * use common code tam_act2_utils.c without smart_cookie.c
 *
 * Revision 1.1.2.5  2018/10/04 09:40:27  peteteng
 * Add alter MB CPU cookie on EEPROM Utility
 *
 * Revision 1.1.2.4  2018/10/02 02:32:15  mikech2
 * Modify FPGA register according SPEC 1.7.1
 *
 * Revision 1.1.2.3  2018/07/12 08:02:08  peteteng
 * add tam_lib_platform_write/read
 *
 * Revision 1.1.2.2  2018/07/02 02:40:32  peteteng
 * update reset and unreset functions
 *
 * Revision 1.1.2.1  2018/06/26 06:30:09  peteteng
 * Add Aikido Cookie menu
 *
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */


