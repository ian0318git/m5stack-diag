/* $Id: diag_raid_lib.c,v 1.2 2016/04/20 11:25:32 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_raid_lib.c,v $
 *------------------------------------------------------------------
 *
 * diag_raid_lib.c
 * 
 * Oct. 2015, iyc
 *
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include "nvmonvars.h"
#include "diag_raid_lib.h"
#include <fcntl.h>
#include <sys/mman.h>
#include "i2c_api.h" 
#include "common.h"
#include "platform_i2c.h"
#include "nvmonvars.h"
#include "diag_i2c_api.h"
#include "linux_api.h"
#include "proto.h"
#include "common_utils.h"
#include "types.h"
#include "error.h"
#include "defs.h"
#include "diag_raid_test.h"
#include "diag_i2c_lib.h"
#include <pthread.h>
#include <termios.h>

long platform_pca9557_init (void);
char * parse_cpld_data(void);
long platform_pca9557_program_cpld(unsigned char *);
int get_raid_i2c_struct (n2g_i2c_if_t *, int);
int raid_switch_ctrl(int);

static unsigned int shift(unsigned int, unsigned char, unsigned char);
static unsigned char pca9557_jtag_io (char, char);
static void reset_2_run(void);
static void reset_jtag(void);
static void run_2_ir(void);
static void ir_2_run(void);
static void run_test_delay(int);
static void dr_2_run(void);
static void ir_2_dr(void);
static void run_2_dr(void);

static reg_info_t_ext pca9557_reg_ext = {1, diag_pca9557_read_fn,
                                         diag_pca9557_write_fn, 0};

static reg_info_t_ext cpld_reg_ext = {1, platform_cpld_read_fn,
                                      platform_cpld_write_fn, 0};


static reg_info_t pca9557_reg_tbl[]=
{
    {"Input port", PCA9557_NGIO_EXPANDER_INPUT,
     (READ_ONLY), {(unsigned long)&pca9557_reg_ext}, 0x00, 0x00},
    {"Output port", PCA9557_NGIO_EXPANDER_OUTPUT,
     (PCA9557_RW), {(unsigned long)&pca9557_reg_ext}, 0xFF, 0x00},
    {"Polarity Inversion port", PCA9557_POLARITY_INVERSION,
     (PCA9557_RW), {(unsigned long)&pca9557_reg_ext}, 0xFF, 0xF0},
    {"Configuration port", PCA9557_CONFIGURATION,
     (READ_ONLY), {(unsigned long)&pca9557_reg_ext}, 0xFF, 0xFF},
    {0, 0, 0, {0}, 0, 0},
};

static reg_info_t cpld_5m570_reg_tbl[] = {
    {"NGIO Expander Input Register", CPLD_NGIO_EXPANDER_INPUT,
     CPLD_RONLY, {(unsigned long)&cpld_reg_ext}, 0x20, 0x23},
    {"NGIO Expander Output Register", CPLD_NGIO_EXPANDER_OUTPUT,
     CPLD_RONLY, {(unsigned long)&cpld_reg_ext}, 0x20, 0x23},
    {"NGIO Expander Dir Register", CPLD_NGIO_EXPANDER_DIR,
     CPLD_RONLY, {(unsigned long)&cpld_reg_ext}, 0x20, 0xff},
    {"ZL30363 and Control Status Register", CPLD_ZL30363_CONT_STS,
     CPLD_RONLY, {(unsigned long)&cpld_reg_ext}, 0xff, 0x00},
    {"Power Sequence Status Register", CPLD_POW_SEQ_STS,
     CPLD_RONLY, {(unsigned long)&cpld_reg_ext}, 0x8f, 0x80},
    {"Jtag Control Register", CPLD_JTAG_CTL,
     CPLD_RW, {(unsigned long)&cpld_reg_ext}, 0xe, 0x0},
    {"CPLD Version Register", CPLD_VERSION,
     CPLD_RONLY, {(unsigned long)&cpld_reg_ext}, 0xf, 0x0},
    {"CPLD GPIO Direction Register", CPLD_GPIO_DIRECTION,
     CPLD_RONLY, {(unsigned long)&cpld_reg_ext}, 0xff, 0x0},
    {"CPLD Speed Up Register", CPLD_SPEED_UP,
     CPLD_RONLY, {(unsigned long)&cpld_reg_ext}, 0xff, 0x0},
    {"CPLD Ten Register", CPLD_TEN,
     CPLD_RONLY, {(unsigned long)&cpld_reg_ext}, 0xff, 0x0},
    {"CPLD Eleven Register", CPLD_ELEVEN,
     CPLD_RONLY, {(unsigned long)&cpld_reg_ext}, 0xff, 0x0},
    {"CPLD Twelve Register", CPLD_TWELVE,
     CPLD_RONLY, {(unsigned long)&cpld_reg_ext}, 0xff, 0x0},
    {"CPLD Thirteen Register", CPLD_THIRTEEN,
     CPLD_RONLY, {(unsigned long)&cpld_reg_ext}, 0xff, 0x0},
    {"CPLD Fourteen Register", CPLD_FOURTEEN,
     CPLD_RONLY, {(unsigned long)&cpld_reg_ext}, 0xff, 0x0},
    {"CPLD Fifteen Register", CPLD_FIFTEEN,
     CPLD_RONLY, {(unsigned long)&cpld_reg_ext}, 0xff, 0x0},
    {"END", 0x0 ,0 ,{0}, 0x0, 0x0},
};

/**********************************************************************
 *
 * Function: platform_cpld_read_fn
 *
 * Read platform CPLD Register.
 *
 * Input : addr - Register offset
 *         size - Register size
 *         *buf - pointer to read buffer
 *         *param - pointer to param
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int platform_cpld_read_fn (unsigned long addr, int size,
                           unsigned long *buf, void *param)
{
    int rc = PASSED;
    if  (platform_5m570_i2c_r(addr, (char*)buf) == FAILED) {
        rc = (FAILED);
    }
    return (rc);
}

/**********************************************************************
 *
 * Function: platform_cpld_write_fn
 *
 * Write platform CPLD Register.
 *
 * Input : addr - Register offset
 *         size - Register size
 *         data - data for write
 *         *param - pointer to param
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int platform_cpld_write_fn (unsigned long addr, int size,
                            unsigned long data, void *param)
{
    int rc = PASSED;
    if  (platform_5m570_i2c_w(addr, data) == FAILED) {
        rc = (FAILED);
    }
    return (rc);
}

int platform_pca9557_i2c_r (uint32_t offset, uchar *data_in)
{
    int rc;
    n2g_i2c_if_t i2c_if;

    rc = get_raid_i2c_struct(&i2c_if, MB_I2C_ADDR_PCA9557);

    if (rc != PASSED) {
        return (FAILED);
    }

    i2c_if.buf = (char *)data_in;
    i2c_if.size = sizeof(uint8_t);
    i2c_if.offset = offset;

    rc = n2g_i2c_read(&i2c_if);

    if (rc != RC_I2C_OP_OK) {
        printf("%s: Unable to read. rc=0x%08x\n", __FUNCTION__, rc);
        return (FAILED);
    }

    msleep(REN_I2C_PROC_TIME);

    return (PASSED);
}

int platform_pca9557_i2c_w (uint32_t offset, uchar data_out)
{
    int rc;
    n2g_i2c_if_t i2c_if;
   
    rc = get_raid_i2c_struct(&i2c_if, MB_I2C_ADDR_PCA9557);

    if (rc != PASSED) {
        return (FAILED);
    }

    i2c_if.buf = (char *)&data_out;
    i2c_if.size = sizeof(uint8_t);
    i2c_if.offset = offset;
    rc = n2g_i2c_write(&i2c_if);

    if (rc != RC_I2C_OP_OK) {
        msleep(REN_I2C_PROC_TIME);
        return (FAILED);
    }

    /* I2C cycle time */
    msleep(REN_I2C_PROC_TIME);

    return (PASSED);
}

int platform_5m570_i2c_r (uint32_t offset, uchar *data_in)
{
    int rc;
    n2g_i2c_if_t i2c_if;

    rc = get_raid_i2c_struct(&i2c_if, MB_I2C_ADDR_CPLD_5M570);

    if (rc != PASSED) {
        return (FAILED);
    }

    i2c_if.buf = (char *)data_in;
    i2c_if.size = sizeof(uint8_t);
    i2c_if.offset = offset;

    rc = n2g_i2c_read(&i2c_if);

    if (rc != RC_I2C_OP_OK) {
        printf("%s: Unable to read. rc=0x%08x\n", __FUNCTION__, rc);
        return (FAILED);
    }

    msleep(REN_I2C_PROC_TIME);

    return (PASSED);
}

int platform_5m570_i2c_w (uint32_t offset, uchar data_out)
{
    int rc;
    n2g_i2c_if_t i2c_if;
   
    rc = get_raid_i2c_struct(&i2c_if, MB_I2C_ADDR_CPLD_5M570);

    if (rc != PASSED) {
        return (FAILED);
    }

    i2c_if.buf = (char *)&data_out;
    i2c_if.size = sizeof(uint8_t);
    i2c_if.offset = offset;

    rc = n2g_i2c_write(&i2c_if);

    if (rc != RC_I2C_OP_OK) {
        msleep(REN_I2C_PROC_TIME);
        return (FAILED);
    }

    /* I2C cycle time */
    msleep(REN_I2C_PROC_TIME);

    return (PASSED);
}

long platform_pca9557_init (void)
{
    char cpld_write_buf, test_buf;
    
    cpld_write_buf = 0;

    if (platform_pca9557_i2c_w(PCA9557_POLARITY_INVERSION, cpld_write_buf) == FAILED) {
        return (FAILED);
    }

    if (platform_pca9557_i2c_r(PCA9557_POLARITY_INVERSION, &cpld_write_buf) == FAILED) {
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("PCA9557_POLARITY_INVERSION value 0x%x\n", cpld_write_buf);
    }
     /* Configure the GPIO 5 as INPUT, others as OUTPUT.
     * PCA9557_TDO_GPIO5_INPUT : 0x1 << 5
     * PCA9557_TDI_GPIO2_OUTPUT: 0x1 << 2
     * cpld_write_buf : 0x24 ()
     * */

    cpld_write_buf = (PCA9557_TDO_GPIO5_INPUT | PCA9557_TDI_GPIO2_OUTPUT);
    if (platform_pca9557_i2c_w(PCA9557_CONFIGURATION, cpld_write_buf) == FAILED) {
        return (FAILED);
    }

    if (platform_pca9557_i2c_r(PCA9557_CONFIGURATION, &test_buf)
        == FAILED) {
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("PCA9557_CONFIGURATION value 0x%x\n", test_buf);
    }
    return (PASSED);
}

char * parse_cpld_data(void)
{
    FILE * stream, * stream1;
    unsigned int at, bt, ct, dt, et, ft, gt, ht;
    unsigned char *hex, *hex1, *hex2;
    int k = 0, temp = 0;
    int data, l;
    char buff[BUFSIZ];

    stream = fopen(EPM570_FILE_LOCATION,"r");
    stream1 = fopen(EPM570_FILE_LOCATION,"r");

     /* Pre-calculate the size of cpld data */
    if (stream == NULL ) {
       printf("Please upload the cpld c file to SD card!!\n");
    } else {
        while(feof(stream) == 0) {
            fgets( buff, sizeof buff, stream );
            k+=sscanf( buff, "%x %x %x %x %x %x %x %x", &at, &bt, &ct, &dt, &et, &ft, &gt, &ht );
        }
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
           printf("k %d\n",k);
    }

    /* Allocate memory to store the cpld data */
    if ( (hex = (unsigned char *)malloc(k)) == NULL ) {
         printf("system is out of memory");
    }


    if (stream1 == NULL ) {
       printf("Please upload the cpld c file to SD card!!\n");
    } else {
      while(feof(stream1) == 0) {
         fscanf(stream1, "%x", &data);

           /* hex1 is used to record the first data of cpld c file */
           if ( temp == 0) {
             hex1 = hex;
           }
           *hex = data;
         hex++;
         temp++;
      }
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
            for (l = 0; l < 10; l++) {
               printf("hex[%d] %x\n",l,*(hex1++));
            }
            hex2 = hex-1;
            for (l = k; l >= k-10; l--) {
               printf("hex[%d] %x\n",l,*(hex2--));
            }
    }

    fclose(stream);
    fclose(stream1);
    return hex1;
}

long platform_pca9557_program_cpld(unsigned char *pof_start_address)
{
    unsigned short silicon_id[5];
    unsigned char *file_pt;
    unsigned int ix;
    unsigned int r_data;
    unsigned short w_data;
    int start_address;
    int status=1;
    int cpld_size;

    /* reset */
    reset_jtag();

    /* reset to ir */
    reset_2_run();

    /***************************** Sample Preload ******************************/
    printf("Sample Preload.\n");

    /* run to ir */
    run_2_ir();
    
    /* ISC_DISABLE */
    shift(0x5,10,0);

    /* run */
    ir_2_run();

    /* Delay 4 clock cycle which refers from LA waves of website sample code. */
    run_test_delay(4);

    /* LOAD ISC_READ */
    run_2_ir();
    shift(0x0, 1, 1);
    dr_2_run();

    /* LOAD ISC_READ */
    run_2_ir();
    shift(0x0, 1, 1);
    dr_2_run();

    /* Delay 2 clock cycle which refers from LA waves of website sample code. */
    run_test_delay(2);

    /***************************** RT ISP Enable ******************************/
    printf("RT ISP Enable.\n");

    /* run to ir */
    run_2_ir();

    /* ISC_DISABLE */
    shift(0x199,10,0);

    /* run */
    ir_2_run();

    /* Delay 4 clock cycle which refers from LA waves of website sample code. */
    run_test_delay(4);
    /**************************** read  silicon_id ****************************/
    /* run to ir */
    run_2_ir();

    /* ISC_ADDRESS_SHIFT */
    shift(0x203, 10, 0);

    /* LOAD Address in DR */
    ir_2_dr();

    printf("5M570 silicon ID location at 0x2220\n ");
    shift(0x2220, 14, 1);

    /* run */
    ir_2_run();

    /* LOAD ISC_READ */
    run_2_ir();
    shift(0x205, 10, 0);

    /* run */
    ir_2_run();

    /* Delay 4 clock cycle which refers from LA waves of website sample code. */
    run_test_delay(4);

    for(ix = 0; ix < 5; ix++) {
        run_2_dr();
        r_data = shift(0xFFFF, 16, 1);
        silicon_id[ix] = r_data;
        dr_2_run();
    }


    printf("\nsilicon_id is: ");
    for(ix = 0; ix < 5; ix++) {
        printf("%#x, ", silicon_id[ix]);
    }
    printf("\n");

     /* silicon_id = 0x4c41,0x4554,0x4152,0x3031,0x0304 for 5M240Z*/
    sleep(5);
    /******************************** Erase 0 *********************************/
    printf("Erase 0.\n");

    /* run to ir */
    run_2_ir();

    /* ISC_ADDRESS_SHIFT */
    shift(0x203,10,0);

    /* LOAD Address in DR */
    ir_2_dr();


    printf("0x0 shift 14\n");
    shift(0x0,14,1);

    /* run */
    ir_2_run();

    /* LOAD ISC_ERASE */
    run_2_ir();
    shift(0x2F2,10,0);

    /* run */
    ir_2_run();

    /* Delay 7 clock cycle which refers from LA waves of website sample code. */
    run_test_delay(7);

    sleep(5);
    /******************************** Erase 1 *********************************/
    printf("Erase 1.\n");
    /* run to ir */
    run_2_ir();

    /* ISC_ADDRESS_SHIFT */
    shift(0x203,10,0);

    /* LOAD Address in DR */
    ir_2_dr();

    printf("0x2000 shift 14\n");
    shift(0x2000,14,1);

    /* run */
    ir_2_run();

    /* LOAD ISC_ERASE */
    run_2_ir();
    shift(0x2F2,10,0);

    /* run */
    ir_2_run();

    /* Delay 7 clock cycle which refers from LA waves of website sample code. */
    run_test_delay(7);
    sleep(5);

    /******************************** Erase 2 *********************************/
    printf("Erase 2.\n");
    /* run to ir */
    run_2_ir();

    /* ISC_ADDRESS_SHIFT */
    shift(0x203,10,0);

    /* LOAD Address in DR */
    ir_2_dr();


    printf("0x2100 shift 14\n");
    shift(0x2100,14,1);

    /* run */
    ir_2_run();

    /* LOAD ISC_ERASE */
    run_2_ir();
    shift(0x2F2,10,0);

    /* run */
    ir_2_run();

    /* Delay 7 clock cycle which refers from LA waves of website sample code. */
    run_test_delay(7);
    sleep(5);
    /******************************** Program *********************************/
    printf("Program.\n");
    /* run to ir */
    run_2_ir();

    /* ISC_ADDRESS_SHIFT */
    shift(0x203,10,0);

    /* LOAD Address in DR */
    ir_2_dr();

    shift(0x0,14,1);

    /* run */
    ir_2_run();

    /* LOAD ISC_PROG */
    run_2_ir();
    shift(0x2F4,10,0);

    /* run */
    ir_2_run();

    /*use the logic analyer to confirm the first data
     * cpld_size is defined by the datasheet
     * */

    start_address = 159;
    cpld_size = 6912;
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("start_address is %d\n",start_address);
        printf("cpld_size is %d\n",cpld_size);
    }

    file_pt = pof_start_address + start_address;
    /* 6912 */
    for(ix = 0; ix < cpld_size; ix++) {
        w_data = 0;
        w_data = *file_pt;
        file_pt++;
        w_data = w_data | ((*file_pt) << 8);

        if(ix<10){
            printf("w_data is %x\n",w_data);
        }

        file_pt++;

        /* write */
        run_2_dr();

        r_data = shift(w_data, 16, 1);
        printf(".");
        dr_2_run();

        /* Delay 3 clock cycle which refers from LA waves of website sample code. */
        run_test_delay(3);

    }
    /******************************** Verify *********************************/
    printf("\nVerify.\n");
    /* run to ir */
    run_2_ir();

    /* ISC_ADDRESS_SHIFT */
    shift(0x203,10,0);
    /* LOAD Address in DR */
    ir_2_dr();

    shift(0x0,14,1);

    /* run */
    ir_2_run();

    /* LOAD ISC_READ */
    run_2_ir();
    shift(0x205,10,0);

    /* run */
    ir_2_run();

    /* Start to verify from firmware offset 171 which refers from LA waves of
     * website sample code. */
    printf("start_address is %d\n",start_address);
    file_pt = pof_start_address +start_address ;
    for(ix = 0; ix < cpld_size; ix++) {
        w_data = 0;
        w_data = *file_pt;
        file_pt++;
        w_data = w_data | ((*file_pt) << 8);
        file_pt++;

        /* w_data=0;
         * w_data=((*file_pt)<<8);
         * file_pt++;
         * w_data=w_data|(*file_pt);
         * file_pt++;
         */

        /* write */
        run_2_dr();
        r_data = shift(0xFFFF, 16, 1);

        if(ix<10)
            printf("r_data is %x\n",r_data);

        dr_2_run();
        printf(".");

        if(r_data != w_data) {
            pca9557_jtag_io(0, 0);
            printf("Verify address %d fail r_data is %#x w_data is %#x\n",
                   ix, r_data, w_data);
        } else {
             status=0;
        }
    }

    /* Delay 7 clock cycle which refers from LA waves of website sample code. */
   run_test_delay(7);
   /***************************** Program Done ******************************/
    /* run to ir */
    run_2_ir();

    /* ISC_ADDRESS_SHIFT */
    shift(0x203,10,0);

    /* LOAD Address in DR */
    ir_2_dr();

    shift(0x0,14,1);
    /* run */
    dr_2_run();

    /* LOAD ISC_PROG */
    run_2_ir();
    shift(0x2F4,10,0);

    /* run */
    ir_2_run();

    run_2_dr();
    w_data = 0xFFDF;
    printf("w_data is %x\n",w_data);
    r_data = shift(w_data, 16, 1);            //0xFBFF,0x7BFF
    dr_2_run();
    
    usleep(100);

    /* set add 0 */
    /* run to ir */
    run_2_ir();

    /* ISC_ADDRESS_SHIFT */
    shift(0x203,10,0);

    /* LOAD Address in DR */
    ir_2_dr();

    shift(0x0,14,1);

    /* run */
    dr_2_run();

    /* LOAD ISC_READ */
    run_2_ir();
    shift(0x205,10,0);

    /* run */
    ir_2_run();

    run_2_dr();
    r_data = shift(0xFFFF,16,1);

    printf("r_data is %x\n",r_data);

    dr_2_run();
    /**************************** RT ISP Disable *****************************/
    printf("RT ISP Disable.\n");
    /* run to ir */
    run_2_ir();

    /* ISC_DISABLE */
    shift(0x166,10,0);
    printf("ENABLE 0x166\n");

    /* run */
    ir_2_run();

    /* Delay 7 clock cycle which refers from LA waves of website sample code. */
    run_test_delay(7);

    /* run to ir */
    run_2_ir();

    /* BYPASS */
    shift(0x3FF,10,0);

    /* run */
    ir_2_run();

    run_test_delay(7);
    reset_jtag();

    if(status == 0){
        printf("Program Done.\n");
        return (PASSED);
    } else {
        printf("Program Fail.\n");
        return (FAILED);
    }
    return  (PASSED);
}


int get_raid_i2c_struct (n2g_i2c_if_t *raid_i2c, int dev_addr)
{
    n2g_i2c_if_t *tmp;

    /* init i2c_if for I2C */
    tmp = (n2g_i2c_if_t *)platform_fpga_get_n2g_i2c_if(I2C_CTRL_FIFTEEN, I2C_MUX_ZERO,
                                                       dev_addr);

    if (tmp == NULL) {
        printf("%s: Failed to get Altitude Sensor I2C interface structure.\n",
               __FUNCTION__);
        return (FAILED);
    }

    memcpy(raid_i2c, tmp, sizeof(n2g_i2c_if_t));

    return (PASSED);
}
 

/**********************************************************************
 *
 * Function: pac9557_jtag_io
 *
 * This function provide the jtag io function via pca9557
 *
 * Input : tms
 *         tdi
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static unsigned char pca9557_jtag_io (char tms,char tdi)
{
    unsigned char in_data;
    unsigned char out_data=0;
    
    if(tms != 0) {
        /* tms */
        out_data |= PCA9557_TMS_GPIO7_OUTPUT;
    }

    if(tdi != 0) {
        /* tdi */
        out_data |= PCA9557_TDI_GPIO4_OUTPUT;
    }
   
    /* tck = 0 */
    if (platform_pca9557_i2c_w(PCA9557_NGIO_EXPANDER_OUTPUT, out_data)
        == FAILED) {
        return (FAILED);
    }
   
    /* read tdo */
    if (platform_pca9557_i2c_r(PCA9557_NGIO_EXPANDER_INPUT, &in_data)
        == FAILED) {
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("PCA9557_NGIO_EXPANDER_INPUT 0x%x\n", in_data);
    }

    /* tdo = 1? write 1 to uart */
    if((in_data & PCA9557_TDO_GPIO5_INPUT) >= 1) {
        in_data = 0x01;
    } else {
        in_data = 0x00;
    }

    /* tck = 1 */
    out_data |= PCA9557_TCK_GPIO6_OUTPUT;
    if (platform_pca9557_i2c_w(PCA9557_NGIO_EXPANDER_OUTPUT, out_data) == FAILED) {
        return (FAILED);
    }

    /* tck = 0 */
    out_data &= ~PCA9557_TCK_GPIO6_OUTPUT;
    if (platform_pca9557_i2c_w(PCA9557_NGIO_EXPANDER_OUTPUT, out_data) == FAILED) {
        return (FAILED);
    }
    return (in_data);
}


/**********************************************************************
 *
 * Function: reset_2_run
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
static void reset_2_run(void)
{
    pca9557_jtag_io(0, 0);
}

/**********************************************************************
 *
 * Function: reset_jtag
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
static void reset_jtag(void)
{
    pca9557_jtag_io(1, 0);
    pca9557_jtag_io(1, 0);
    pca9557_jtag_io(1, 0);
    pca9557_jtag_io(1, 0);
    pca9557_jtag_io(1, 0);
}

/**********************************************************************
 *
 * Function: run_2_ir
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
static void run_2_ir(void)
{
    pca9557_jtag_io(1, 0);
    pca9557_jtag_io(1, 0);
    pca9557_jtag_io(0, 0);
    pca9557_jtag_io(0, 0);
}

/**********************************************************************
 *
 * Function: ir_2_run
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
static void ir_2_run(void)
{
    /* Enter Pause-IR from Exit1-IR */
    pca9557_jtag_io(0, 0);
    /* Enter Exit2-IR from Pause-IR */
    pca9557_jtag_io(1, 0);
    /* Enter Update-IR from Exit2-IR */
    pca9557_jtag_io(1, 0);
    /* Enter Run from Update-IR */
    pca9557_jtag_io(0, 0);
}


/**********************************************************************
 *
 * Function: run_test_delay
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
static void run_test_delay(int delay_times)
{
    int ix;

    for (ix = 0; ix < delay_times; ix++) {
        pca9557_jtag_io(0, 0);
    }
}
/**********************************************************************
 *
 * Function: shift
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
unsigned int shift(unsigned int data, unsigned char cnt, unsigned char msb)
{
    unsigned char i;
    unsigned char tms;
    unsigned char tdi;
    unsigned char tdo;
    unsigned int rddata;


    rddata = 0;
    for(i = 0; i < cnt; i++) {
        if(i == (cnt-1)) {
            tms = 1;
        } else {
            tms = 0;
        }

        if(msb == 1) {
            tdi = (data & (1 << (cnt - i - 1))) > 0 ? 1:0;
        } else {
            tdi = (data & (1 << i))> 0 ? 1:0;
        }

        /* write tdi,tms */
        tdo = pca9557_jtag_io(tms, tdi);

        /* read tdo */
        if(msb == 1) {
            rddata = (tdo != 0) ? (rddata | (1 << (cnt - i - 1))) : rddata;
        } else {
            rddata = (tdo != 0) ? (rddata | ( 1 << i)) : rddata;
        }
    }

    return (rddata);

}

/**********************************************************************
 *
 * Function: dr_2_run
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
static void dr_2_run(void)
{
    /* Enter Pause-IR from Exit1-IR */
    pca9557_jtag_io(0, 0);
    /* Enter Exit2-IR from Pause-IR */
    pca9557_jtag_io(1, 0);
    pca9557_jtag_io(1, 0);
    pca9557_jtag_io(0, 0);
}

/**********************************************************************
 *
 * Function: ir_2_dr
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
static void ir_2_dr(void)
{
    /* Enter Pause-IR from Exit1-IR */
    pca9557_jtag_io(0, 0);
    /* Enter Exit2-IR from Pause-IR */
    pca9557_jtag_io(1, 0);
    pca9557_jtag_io(1, 0);
    pca9557_jtag_io(1, 0);
    pca9557_jtag_io(0, 0);
    pca9557_jtag_io(0, 0);
}

/**********************************************************************
 *
 * Function: run_2_dr
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
static void run_2_dr(void)
{
    pca9557_jtag_io(1, 0);
    pca9557_jtag_io(0, 0);
    pca9557_jtag_io(0, 0);
}


int raid_switch_ctrl (int switch_dev)
{
    char val_buf;
    if (platform_5m570_i2c_r(CPLD_RESERVED, &val_buf) == FAILED) {
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("offset 2 value 0x%x\n", val_buf);
    }

    if (switch_dev) {
        val_buf |= SMB_SAS_ISOLATE;
    } else {
        val_buf &= (~SMB_SAS_ISOLATE);
    }

    if (platform_5m570_i2c_w(CPLD_RESERVED, val_buf) == FAILED) {
        return (FAILED);
    }
    
    return (PASSED);
}


int raid_sbr_ctrl (int en_program)
{
    char val_buf;
    if (platform_5m570_i2c_r(CPLD_RESERVED, &val_buf) == FAILED) {
        return (FAILED);
    }

    printf("CPLD Reg. 2 value 0x%x\n", val_buf);

    if (en_program) {
        val_buf &= (~SBR_CTRL_LSI);
        val_buf &= (~SMB_SAS_ISOLATE);
    } else {
        val_buf |= SBR_CTRL_LSI;
        val_buf &= (~SMB_SAS_ISOLATE);
    }

    if (platform_5m570_i2c_w(CPLD_RESERVED, val_buf) == FAILED) {
        return (FAILED);
    }

    return (PASSED);
}


int diag_raid_sbr_fw_upgrade (ulong arr_size, uchar *arr)
{
    int ix;

    int temp = (int)arr_size;
    printf("arr_size %d\n", temp);
   
    printf("Programming...\n");
    for (ix = 0; ix < arr_size ;ix++) {
        diag_i2c_byte_write(I2C_CTRL_FIVE, DC_I2C_SBR, ix, arr[ix]);
        usleep(500);
    }

    printf("Verifying...\n");
    uchar data;
    for (ix = 0; ix < arr_size ; ix++) {
        diag_i2c_byte_read(I2C_CTRL_FIVE, DC_I2C_SBR, ix, &data);
        usleep(500);
        if (arr[ix] != data) {
            printf("\n0x%x expected 0x%x  actual 0x%x\n", ix, arr[ix], data);
            return (FAILED);
        }
    }
    
    return (PASSED);
}
/**********************************************************************
 *
 * Function: platform_cpld_jtag_ctl
 *
 * Function to turn on jtag
 *
 * Input : jtag_on_off - turn on/off the jtag on
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long platform_cpld_jtag_ctl (boolean jtag_on_off)
{
    uchar cpld_buf;

    /* JTAG Control register JTAG_ON bit as 1 */
    if (platform_5m570_i2c_r(CPLD_JTAG_CTL, &cpld_buf) == FAILED) {
        return (FAILED);
    }

    if (jtag_on_off == TRUE) {
        /* Make JTAG_ON bit as 1 */
        cpld_buf |= (CPLD_JTAG_ON);
    } else {
        /* Make JTAG_ON bit as 0 */
        cpld_buf &= ~(CPLD_JTAG_ON);
    }

    /* Make JTAG_TCK bit as 0 */
    cpld_buf &= ~(CPLD_JTAG_TCK_ST);

    /* Alter the register */
    if (platform_5m570_i2c_w(CPLD_JTAG_CTL, cpld_buf) == FAILED) {
        return (FAILED);
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: platform_cpld_power_ctl
 *
 * Function to control cpld power on/off
 *
 * Input : cpld_power - power on/off the cpld
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long platform_cpld_power_ctl (void)
{
    uchar cpld_buf;

    /* Power Sequence Status register bit 7 as 1 */
    if (platform_5m570_i2c_r(CPLD_POW_SEQ_STS, &cpld_buf) == FAILED) {
        return (FAILED);
    }

    /* Make Power Sequence Status bit 7 as 0 */
    cpld_buf &= ~(CPLD_POWER_RESTART);

    /* Alter the register */
    if (platform_5m570_i2c_w(CPLD_POW_SEQ_STS, cpld_buf) == FAILED) {
        return (FAILED);
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: timingcard_pca9557_power_cycle_cpld
 *
 * This function supports the PCA9557 to power cycle CPLD.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long platform_pca9557_power_cycle_cpld (void)
{
    uchar cpld_write_buf;

    printf("\nPOWER-CYCLE the CPLD to get new CPLD image running...\n");

    /* Step 1: Set output high on IO2 */
    cpld_write_buf = 0x4;
    if (platform_pca9557_i2c_w(PCA9557_NGIO_EXPANDER_OUTPUT, cpld_write_buf) == FAILED) {
        return (FAILED);
    }

    /* Step 2: Set IO2 as output */
    cpld_write_buf = 0xFB;
    if (platform_pca9557_i2c_w(PCA9557_CONFIGURATION, cpld_write_buf) == FAILED) {
        return (FAILED);
    }

    /* Step 3: Power cycle the CPLD. */
    if (platform_cpld_power_ctl() == FAILED) {
        printf("Current CPLD firmware is not responding.\n");
    }

    /* Need to delay at least 10 ms after step 3. */
    msleep(15);

    /* Step 4: Turn off 1.8V CPLD */
    cpld_write_buf = 0x0;
    if (platform_pca9557_i2c_w(PCA9557_NGIO_EXPANDER_OUTPUT, cpld_write_buf) == FAILED) {
        return (FAILED);
    }

    /* Step 5: Turn on 1.8V CPLD */
    cpld_write_buf = 0x4;
    if (platform_pca9557_i2c_w(PCA9557_NGIO_EXPANDER_OUTPUT, cpld_write_buf) == FAILED) {
        return (FAILED);
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: speed_up_cpld_jtag_io
 *
 * JTAG I/O routine
 *
 * Input : reg_offset - cpld register offset
 *         cpld_w_buf - cpld write data value
 *         tdo_read - cpld read data value flag
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static long speed_up_cpld_jtag_io (uchar reg_offset, uchar cpld_w_buf,
                                   boolean tdo_read)
{
    uchar cpld_r_buf = 0, rd_data;

    if (tdo_read == FALSE) {
        /* Write data value */
        if (platform_5m570_i2c_w(reg_offset, cpld_w_buf) == FAILED) {
            return (FAILED);
        }
    }

    /* Read data value */
    if (tdo_read == TRUE) {
        if (platform_5m570_i2c_r(reg_offset, &rd_data) == FAILED) {
            return (FAILED);
        }

        cpld_r_buf = rd_data;
    }

    return (cpld_r_buf);
}

static int count = 0;
/**********************************************************************
 *
 * Function: cpld_jtag_io
 *
 * JTAG I/O routine
 *
 * Input : tms - jtag tms control
 *         tdi - jtag tdi control
 *         read_tdo - jtag read_tdo control
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long cpld_jtag_io (int tms, int tdi, int read_tdo)
{
    int tdo = 0;
    uchar cpld_buf;

    /* Print out the status. */
    count++;
    if ((count % 10) == 0) {
        printf(".");
        count = 0;
    }

    /* Upgrade the firmware from CPLD
     * Write data (TMS, TDI, TCK written low)
     * Read TDO
     * Write data (TCK written high)
     * Write data (TCK written low)
     */
    cpld_buf = CPLD_JTAG_ON;

    /* Prepare: Write data (TMS) */
    if (tms) {
        cpld_buf |= CPLD_JTAG_TMS_ST;
    } else {
        cpld_buf &= ~CPLD_JTAG_TMS_ST;
    }

    /* Prepare: Write data (TDI) */
    if (tdi) {
        cpld_buf |= CPLD_TDI_ST;
    } else {
        cpld_buf &= ~CPLD_TDI_ST;
    }

    /* Prepare: TCK written low */
    cpld_buf &= ~CPLD_JTAG_TCK_ST;

    /* step 1: Write data (TMS, TDI, TCK written low) */
    if (platform_5m570_i2c_w(CPLD_JTAG_CTL, cpld_buf) == FAILED) {
        return (FAILED);
    }

    /* step 2: Read TDO */
    if (read_tdo) {
        if (platform_5m570_i2c_r(CPLD_JTAG_CTL, &cpld_buf) == FAILED) {
            return (FAILED);
        }

        tdo = (cpld_buf & CPLD_TDO_ST) ? 1 : 0;
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("%d ", tdo);
        }
    }

    /* Step 3: Write data (TCK written high) */
    cpld_buf |= CPLD_JTAG_TCK_ST;
    if (platform_5m570_i2c_w(CPLD_JTAG_CTL, cpld_buf) == FAILED) {
        return (FAILED);
    }

    /* Step 4: Write data (TCK written low) */
    cpld_buf &= ~CPLD_JTAG_TCK_ST;
    if (platform_5m570_i2c_w(CPLD_JTAG_CTL, cpld_buf) == FAILED) {
        return (FAILED);
    }

    return (tdo);
}

/**********************************************************************
 *
 * Function: speedup_cpld_reset_jtag
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
static void speedup_cpld_reset_jtag(void)
{
    speed_up_cpld_jtag_io(CPLD_TWELVE, 0xFF, FALSE);
}

/**********************************************************************
 *
 * Function: cpld_reset_2_run
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
static void cpld_reset_2_run(void)
{
    cpld_jtag_io(0, 0, 0);
}

/**********************************************************************
 *
 * Function: speedup_cpld_run_2_ir
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
static void speedup_cpld_run_2_ir(void)
{
    speed_up_cpld_jtag_io(CPLD_TWELVE, 0xC, FALSE);
}

/**********************************************************************
 *
 * Function: speedup_cpld_ir_2_run
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
static void speedup_cpld_ir_2_run(void)
{
    speed_up_cpld_jtag_io(CPLD_TWELVE, 0x80, FALSE);
}

/**********************************************************************
 *
 * Function: cpld_run_test_delay
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
static void cpld_run_test_delay(int delay_cycles)
{
    int ix;

    for (ix = 0; ix < delay_cycles; ix++) {
        cpld_jtag_io(0, 0, 0);
    }
}

/**********************************************************************
 *
 * Function: speedup_cpld_dr_2_run
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
static void speedup_cpld_dr_2_run(void)
{
    speed_up_cpld_jtag_io(CPLD_TWELVE, 0x80, FALSE);
}

/**********************************************************************
 *
 * Function: speedup_cpld_ir_2_dr
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
static void speedup_cpld_ir_2_dr(void)
{
    speed_up_cpld_jtag_io(CPLD_TWELVE, 0x1C, FALSE);
}

/**********************************************************************
 *
 * Function: speedup_cpld_run_2_dr
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
static void speedup_cpld_run_2_dr(void)
{
    speed_up_cpld_jtag_io(CPLD_TWELVE, 0x4, FALSE);
}

/**********************************************************************
 *
 * Function: cpld_shift
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
static unsigned int cpld_shift(unsigned int data, unsigned char cnt,
                               unsigned char msb, boolean read_tdo)
{
    unsigned char ix;
    unsigned char tms;
    unsigned char tdi;
    unsigned char tdo;
    unsigned int rddata;


    rddata = 0;
    for(ix = 0; ix < cnt; ix++) {
        if(ix == (cnt - 1)) {
            tms = 1;
        } else {
            tms = 0;
        }

        if(msb == 1) {
            tdi = (data & (1 << (cnt - ix - 1))) > 0 ? 1:0;
        } else {
            tdi = (data & (1 << ix))> 0 ? 1:0;
        }

        /* Write TMS, TDI and read TDO */
        tdo = cpld_jtag_io(tms, tdi, read_tdo);

        /* read tdo */
        if(msb == 1) {
            rddata = (tdo != 0) ? (rddata | (1 << (cnt - ix - 1))) : rddata;
        } else {
            rddata = (tdo != 0) ? (rddata | ( 1 << ix)) : rddata;
        }
    }

    return (rddata);
}

/**********************************************************************
 *
 * Function: erase_all
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
static void erase_all(int erase_addr)
{
    unsigned short w_data;

    printf("\nErase start address %#x.\n", erase_addr);
    /* run to ir */
    speedup_cpld_run_2_ir();

    /* ISC_ADDRESS_SHIFT */
    /* Speed up write data, MSB = 0, so write from LSB to MSB.
     * And add 0 at LSB to make it as 16 bits value.
     * So write 0x0301 here. */
    w_data = CPLD_ADDRESS_SHIFT;
    speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);

    /* LOAD Address in DR */
    speedup_cpld_ir_2_dr();
    cpld_shift(erase_addr, 13, 1, 0);

    /* run */
    speedup_cpld_ir_2_run();

    /* LOAD ISC_ERASE */
    speedup_cpld_run_2_ir();
    /* Speed up write data, MSB = 0, so write from LSB to MSB.
     * And add 0 at LSB to make it as 16 bits value.
     * So write 0x013d here. */
    w_data = CPLD_ERASE_COMMAND;
    speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);

    /* run */
    speedup_cpld_ir_2_run();

    /* Delay 7 clock cycle which refers from LA waves of website sample code. */
    msleep(600);
}
/**********************************************************************
 *
 * Function: timingcard_simply_program_cpld
 *
 * This function programs the cpld firmware via pca9557
 *
 * Input : *pof_start_address - pointer to the program firmware.
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long platform_simply_program_cpld (unsigned char *pof_start_address)
{
    unsigned short silicon_id[5];
    unsigned char *file_pt;
    unsigned int ix;
    unsigned int r_data;
    unsigned short w_data;

    /* reset */
    speedup_cpld_reset_jtag();

    /* reset to ir */
    cpld_reset_2_run();

    /***************************** Sample Preload ******************************/
    printf("\nSample Preload.\n");

    /* run to ir */
    speedup_cpld_run_2_ir();

    /* Speed up write data, MSB = 0, so write from LSB to MSB.
     * And add 0 at LSB to make it as 16 bits value.
     * So write 0x0280 here. */
    w_data = CPLD_SAMPLE_PRELOAD;
    speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);

    /* run */
    speedup_cpld_ir_2_run();

    /* Delay 4 clock cycle which refers from LA waves of website sample code. */
    cpld_run_test_delay(4);

    /* LOAD ISC_READ */
    speedup_cpld_run_2_ir();
    /* Speed up write data, MSB = 1, so write from MSB to LSB.
     * And add 0 at LSB to make it as 8 bits value.
     * So write 0x0 here. */
    w_data = 0x0;
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);
    speedup_cpld_dr_2_run();

    /* LOAD ISC_READ */
    speedup_cpld_run_2_ir();
    /* Speed up write data, MSB = 1, so write from MSB to LSB.
     * And add 0 at LSB to make it as 8 bits value.
     * So write 0x0 here. */
    w_data = 0x0;
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);
    speedup_cpld_dr_2_run();

    /* Delay 2 clock cycle which refers from LA waves of website sample code. */
    cpld_run_test_delay(2);

    /***************************** RT ISP Enable ******************************/
    printf("\nRT ISP Enable.\n");

    /* run to ir */
    speedup_cpld_run_2_ir();

    /* Speed up write data, MSB = 0, so write from LSB to MSB.
     * And add 0 at LSB to make it as 16 bits value.
     * So write 0x0266 here. */
    w_data = CPLD_ISP_ENABLE;
    speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);

    /* run */
    speedup_cpld_ir_2_run();

    /* Delay 4 clock cycle which refers from LA waves of website sample code. */
    cpld_run_test_delay(4);

    /**************************** read  silicon_id ****************************/
    /* run to ir */
    speedup_cpld_run_2_ir();

    /* ISC_ADDRESS_SHIFT */
    /* Speed up write data, MSB = 0, so write from LSB to MSB.
     * And add 0 at LSB to make it as 16 bits value.
     * So write 0x0301 here. */
    w_data = CPLD_ADDRESS_SHIFT;
    speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);

    /* LOAD Address in DR */
    speedup_cpld_ir_2_dr();

    /* Speed up write data. */
    speed_up_cpld_jtag_io(CPLD_TEN, 0x22, FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, 0x20, FALSE);

    /* run */
    speedup_cpld_ir_2_run();

    /* LOAD ISC_READ */
    speedup_cpld_run_2_ir();
    /* Speed up write data, MSB = 0, so write from LSB to MSB.
     * And add 0 at LSB to make it as 16 bits value.
     * So write 0x0281 here. */
    w_data = CPLD_ISP_READ;
    speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);

    /* run */
    speedup_cpld_ir_2_run();

    /* Delay 4 clock cycle which refers from LA waves of website sample code. */
    cpld_run_test_delay(4);

    for(ix = 0; ix < 5; ix++) {
        speedup_cpld_run_2_dr();
        /* Speed up read data. */
        r_data = speed_up_cpld_jtag_io(CPLD_TEN, 0xFF, TRUE);
        r_data = ((r_data << 8) | speed_up_cpld_jtag_io(CPLD_ELEVEN, 0xFF, TRUE));
        silicon_id[ix] = r_data;
        speedup_cpld_dr_2_run();
    }

    printf("\nsilicon_id is: ");
    for(ix = 0; ix < 5; ix++) {
        printf("%#x, ", silicon_id[ix]);
    }
    printf("\n");

     /* silicon_id = 0x4c41,0x4554,0x4152,0x3031,0x0400 for 5M570Z */
    
    /* Erase 0 */
    erase_all(0x0);
    /* Erase 1 */
    erase_all(0x2000);
    /* Erase 2 */
    erase_all(0x2100);

    /******************************** Program *********************************/
    printf("\nProgram.\n");
    /* run to ir */
    speedup_cpld_run_2_ir();

    /* ISC_ADDRESS_SHIFT */
    /* Speed up write data, MSB = 0, so write from LSB to MSB.
     * And add 0 at LSB to make it as 16 bits value.
     * So write 0x0301 here. */
    w_data = CPLD_ADDRESS_SHIFT;
    speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);

    /* LOAD Address in DR */
    speedup_cpld_ir_2_dr();
    /* Speed up write data, MSB = 1, so write from MSB to LSB.
     * And add 0 at LSB to make it as 16 bits value.
     * So write 0x0 here. */
    w_data = 0x0;
    speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);

    /* run */
    speedup_cpld_ir_2_run();

    /* LOAD ISC_PROG */
    speedup_cpld_run_2_ir();
    /* Speed up write data, MSB = 0, so write from LSB to MSB.
     * And add 0 at LSB to make it as 16 bits value.
     * So write 0x00bd here. */
    w_data = CPLD_ISC_PROG;
    speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);

    /* run */
    speedup_cpld_ir_2_run();

    /* Start to program from firmware offset 159 which refers from LA waves of
     * website sample code. */
    file_pt = pof_start_address + 159;
    /* 6912 */
    for(ix = 0; ix < 6912; ix++) {

        /* Show the status bar */
        if ((ix % 50) == 0) {
            printf(".");
        }

        w_data = 0;
        w_data = *file_pt;
        file_pt++;
        w_data = w_data | ((*file_pt) << 8);
     
        if (ix<10) {
            printf("w_data is 0x%x\n",w_data);
        }

        file_pt++;

        /* write */
        speedup_cpld_run_2_dr();
        
        /* Speed up write data. */
        speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
        speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);
        speedup_cpld_dr_2_run();
        
        /* Delay clock cycle which refers from LA waves of website sample code. */
            speed_up_cpld_jtag_io(CPLD_TEN, 0xFF, FALSE);
    }

    /******************************** Verify *********************************/
    printf("\nVerify.\n");
    /* run to ir */
    speedup_cpld_run_2_ir();

    /* ISC_ADDRESS_SHIFT */
    /* Speed up write data, MSB = 0, so write from LSB to MSB.
     * And add 0 at LSB to make it as 16 bits value.
     * So write 0x0301 here. */
    w_data = CPLD_ADDRESS_SHIFT;
    speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);

    /* LOAD Address in DR */
    speedup_cpld_ir_2_dr();
    /* Speed up write data, MSB = 1, so write from MSB to LSB.
     * And add 0 at LSB to make it as 16 bits value.
     * So write 0x0 here. */
    w_data = 0x0;
    speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);

    /* run */
    speedup_cpld_ir_2_run();

    /* LOAD ISC_READ */
    speedup_cpld_run_2_ir();
    /* Speed up write data, MSB = 0, so write from LSB to MSB.
     * And add 0 at LSB to make it as 16 bits value.
     * So write 0x0281 here. */
    w_data = CPLD_ISP_READ;
    speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);

    /* run */
    speedup_cpld_ir_2_run();

    /* Start to verify from firmware offset 171 which refers from LA waves of
     * website sample code. */
    file_pt = pof_start_address + 159;
    for(ix = 0; ix < 6912; ix++) {

        /* Show the status bar */
        if ((ix % 50) == 0) {
            printf(".");
        }

        w_data = 0;
        w_data = *file_pt;
        file_pt++;
        w_data = w_data | ((*file_pt) << 8);
        file_pt++;

        /* w_data=0;
         * w_data=((*file_pt)<<8);
         * file_pt++;
         * w_data=w_data|(*file_pt);
         * file_pt++;
         */

        /* write */
        speedup_cpld_run_2_dr();;

        /* Speed up read data. */
        r_data = speed_up_cpld_jtag_io(CPLD_TEN, 0xFF, TRUE);
        r_data = ((r_data << 8) | speed_up_cpld_jtag_io(CPLD_ELEVEN, 0xFF, TRUE));
        speedup_cpld_dr_2_run();

        if(r_data != w_data) {
            cpld_jtag_io(0, 0, 0);
            printf("Verify address %d fail r_data is %#x w_data is %#x\n",
                   ix, r_data, w_data);
            return (FAILED);
        }

    }

    /* Delay 7 clock cycle which refers from LA waves of website sample code. */
    cpld_run_test_delay(7);

    /***************************** Program Done ******************************/
    printf("\nSending Program Done.\n");
    /* run to ir */
    speedup_cpld_run_2_ir();

    /* ISC_ADDRESS_SHIFT */
    /* Speed up write data, MSB = 0, so write from LSB to MSB.
     * And add 0 at LSB to make it as 16 bits value.
     * So write 0x0301 here. */
    w_data = CPLD_ADDRESS_SHIFT;
    speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);

    /* LOAD Address in DR */
    speedup_cpld_ir_2_dr();
    /* Speed up write data, MSB = 1, so write from MSB to LSB.
     * And add 0 at LSB to make it as 16 bits value.
     * So write 0x0 here. */
    w_data = 0x0;
    speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);

    /* run */
    speedup_cpld_dr_2_run();

    /* LOAD ISC_PROG */
    speedup_cpld_run_2_ir();
    /* Speed up write data, MSB = 0, so write from LSB to MSB.
     * And add 0 at LSB to make it as 16 bits value.
     * So write 0x00bd here. */
    w_data = CPLD_ISC_PROG;
    speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);

    /* run */
    speedup_cpld_ir_2_run();
    speedup_cpld_run_2_dr();
    /* Speed up write data, MSB = 1, so write from MSB to LSB.
     * And add 0 at LSB to make it as 16 bits value.
     * So write 0xffdf here. */
    w_data = CPLD_ISC_PROG_DONE_1;
    speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);
    speedup_cpld_dr_2_run();

    usleep(100);

    /* set add 0 */
    /* run to ir */
    speedup_cpld_run_2_ir();

    /* ISC_ADDRESS_SHIFT */
    /* Speed up write data, MSB = 0, so write from LSB to MSB.
     * And add 0 at LSB to make it as 16 bits value.
     * So write 0x0301 here. */
    w_data = CPLD_ADDRESS_SHIFT;
    speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);

    /* LOAD Address in DR */
    speedup_cpld_ir_2_dr();
    /* Speed up write data, MSB = 1, so write from MSB to LSB.
     * And add 0 at LSB to make it as 16 bits value.
     * So write 0x0 here. */
    w_data = 0x0;
    speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);

    /* run */
    speedup_cpld_dr_2_run();

    /* LOAD ISC_READ */
    speedup_cpld_run_2_ir();
    /* Speed up write data, MSB = 0, so write from LSB to MSB.
     * And add 0 at LSB to make it as 16 bits value.
     * So write 0x0281 here. */
    w_data = CPLD_ISP_READ;
    speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);

    /* run */
    speedup_cpld_ir_2_run();

    speedup_cpld_run_2_dr();
    /* Speed up write data, MSB = 1, so write from MSB to LSB.
     * And add 0 at LSB to make it as 16 bits value.
     * So write 0xFFFF here. */
    w_data = CPLD_ISC_PROG_DONE_2;
    speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);

    speedup_cpld_dr_2_run();

    /**************************** RT ISP Disable *****************************/
    printf("\nRT ISP Disable.\n");
    /* run to ir */
    speedup_cpld_run_2_ir();

    /* ISC_DISABLE */
    /* Speed up write data, MSB = 0, so write from LSB to MSB.
     * And add 0 at LSB to make it as 16 bits value.
     * So write 0x019a here. */
    w_data = CPLD_ISP_DISABLE;
    speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);

    /* run */
    speedup_cpld_ir_2_run();

    /* Delay 7 clock cycle which refers from LA waves of website sample code. */
    cpld_run_test_delay(7);

    /* run to ir */
    speedup_cpld_run_2_ir();

    /* BYPASS */
    /* Speed up write data, MSB = 0, so write from LSB to MSB.
     * And add 0 at LSB to make it as 16 bits value.
     * So write 0x03ff here. */
    w_data = CPLD_BYPASS;
    speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);

    /* run */
    speedup_cpld_ir_2_run();

    cpld_run_test_delay(7);

    speedup_cpld_reset_jtag();

    printf("\nProgram Done.\n");

    return (PASSED);
}

int platform_cpld_reg_dump(void)
{
    if (register_display(0, &cpld_5m570_reg_tbl[0]) == FAILED) {
        cterr('f', 0, "CPLD Register Display Failed");
        return (FAILED);
    }

    return (PASSED);
}

int diag_pca9557_read_fn (unsigned long addr, int size, unsigned long *buf, void *param)
{
    return (platform_pca9557_i2c_r(addr, (uint8_t *)buf));
}

int diag_pca9557_write_fn (unsigned long addr, int size, unsigned long data, void *param)
{
    return (platform_pca9557_i2c_w(addr, data));
}

int raid_cpld_reg_test_lib(void)
{
    if (register_tests(0, &cpld_5m570_reg_tbl[0]) == FAILED) {
        return (FAILED);
    }
    return (PASSED);
}
int raid_9557_reg_test_lib(void)
{
    if (register_tests(0, &pca9557_reg_tbl[0]) == FAILED) {
        return (FAILED);
    }
    return (PASSED);
}

char last_str[0];

static void
* read_aux (void *u)
{
    int timeout = 120; /*in secs */
    int size = 0; /* when size= 0, read all bytes from uart controller */

    s_uart *uart = (s_uart *)u;

    if (rx_uart(uart->dev, size, (char *)uart->buf, timeout, uart->tst_typ) < 0) {

    }
    pthread_exit(NULL);
}

int raid_uart_intf_test (char *dev, const char *test_str, speed_t test_speed)
{
    struct termios config, ori_conf;
    int uart_fd, ret_val = PASSED, result = 0;
    char tmp_str[100];
    char *pattern = "Sgpio\n";
    pthread_t threads;

    s_uart uart;

    uart.dev =  dev;

    memset(uart.buf, '\0', sizeof(uart.buf));
    uart.tst_typ = DEFAULT_CASE;

    /* using specified pattern */
    if (test_str != NULL) {
        uart.tst_typ = SPECIAL_PAT;
        pattern = (char *)test_str;

        /* get last character for rx_uart() */
        sprintf(tmp_str, pattern);
        last_str[0] = tmp_str[strlen(tmp_str)- 1];
    }

    uart_fd = open(dev, O_WRONLY);
    if (uart_fd < 0) {
        cterr('f', 0, "failed to open %s", dev);
        return FAILED;
    }

    if (tcgetattr(uart_fd, &config) < 0) {
        close(uart_fd);
        cterr('f', 0, "uart_intf_test(): Failed in tcgetattr() %d", __LINE__);
        return (FAILED);
    }

    /* Backup default config for recover after test */
    memcpy(&ori_conf, &config, sizeof(config));

    if ((config.c_cflag & CBAUD) != test_speed) {
        if (cfsetospeed(&config, test_speed) < 0) {
            tcsetattr(uart_fd, TCSAFLUSH, &ori_conf);
            close(uart_fd);
            cterr('f', 0, "uart_intf_test(): Failed to set output speed.");
            return (FAILED);
        }

        if (cfsetispeed(&config, test_speed) < 0) {
            tcsetattr(uart_fd, TCSAFLUSH, &ori_conf);
            close(uart_fd);
            cterr('f', 0, "uart_intf_test(): Failed to set intput speed.");
            return (FAILED);
        }
    }

    config.c_lflag &= ~(ICANON|IEXTEN|ISIG|ECHO);
    config.c_iflag |= IGNCR;
    config.c_oflag &= ~(OPOST);

    if (tcsetattr(uart_fd, TCSAFLUSH, &config) < 0) {
        tcsetattr(uart_fd, TCSAFLUSH, &ori_conf); /*try to reset to orig value */
        close(uart_fd);
        cterr('f', 0, "\nuart_intf_test(): Failed in set new config values tcsetattr()");
        return (FAILED);
    }
    
    if(pthread_create(&threads, NULL, read_aux, (void *)&uart)) {
        perror("pthread_create failed.");  /* softeware bug. should never occur */
        /* Recover to original settings */
        result = tcsetattr(uart_fd, TCSAFLUSH, &ori_conf);
        if (result < 0) {
            cterr('f', 0, "uart_intf_test(): Failed in tcsetattr() %d", __LINE__);
        }
        close(uart_fd);
        return FAILED;
    }

    msleep(500);

    tx_uart(dev, pattern, 1);

    pthread_join(threads, NULL);

    if (!strlen(uart.buf)) {
        /* Recover to original settings */
        cterr('f', 0, "uart_intf_test(): No data received() %d", __LINE__);
        result = tcsetattr(uart_fd, TCSAFLUSH, &ori_conf);
        close(uart_fd);
        if (result < 0) {
            cterr('f', 0, "uart_intf_test(): Failed in tcsetattr() %d", __LINE__);
        }
        return FAILED;
    }

    printf("length %d bytes\n", strlen(uart.buf));
    printf("rx_buf is %s\n", uart.buf);
   
    if(strlen(uart.buf) <= 0){
        return FAILED;
    }

    ret_val = PASSED;
    /* Recover to original settings */
    result = tcsetattr(uart_fd, TCSAFLUSH, &ori_conf);
    if (result < 0) {
        /*software erro should coem here */
        perror("\nuart_intf_test(): data rx ok but Failed in tcsetattr()"); 
        ret_val = FAILED;
    }
    close(uart_fd);

    return (ret_val);

}

