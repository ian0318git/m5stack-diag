/* $Id: cavium_register_access.c,v 1.2 2013/10/08 08:48:27 tirawan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn_linux/cavium_register_access.c,v $
 *-----------------------------------------------------------------------------
 * cavium_register_access.c - r/w cavium registers function for Woodlawn
 *
 * June 2012, Leslie Chen
 * Copyright (c) 2013 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */


#include "proto.h"
#include "menu.h"
#include "types.h"
#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include "nvsysvars.h"
#include "queryflags.h"
#include "error.h"

/* 
 * below header file declare for debug errors
 */
#include "stdint.h"
#include <cvmx.h>
#include <cvmx-pexp-defs.h>
#include <defs.h>
#include <cvmx-gpio.h>

#undef COP0_INDEX
#undef COP0_ENTRYLO0
#undef COP0_ENTRYLO1
#undef COP0_PAGEMASK
#undef COP0_PAGEGRAIN
#undef COP0_BADVADDR
#undef COP0_ENTRYHI
#undef COP0_STATUS
#undef COP0_CAUSE
#undef COP0_EPC
#undef COP0_PRID
#undef COP0_CONFIG
#undef COP0_DESAVE

void print_cavium_status_reg(void);
void print_cavium_cause_reg(void);
void print_cavium_prid_reg(void);
void print_cavium_desave_reg(void);
void write_cavium_cop0_reg(void);
void print_cavium_msi_regs(void);

void cvmx_dump_npei(int);
void cvmx_dump_cfg(int);
void cvmx_dump_pesc(int);

int test_malloc_dev(void);
int test_malloc(void);

int set_cavium_gpio_pin5(void);

void print_64bit_hex(uint64_t);
void print_cavium_cop0_reg(int);

/*
 *for "write_cavium_cop0_reg"
 */
extern uint64_t setcp0_cause(uint64_t);
extern uint64_t setcp0_desave (uint64_t);
extern uint32_t cvmx_pcie_cfgx_read(int, uint32_t);

/*
 * Defines used to read the Coprocessor Register of the Octeon CPU.
 */
#define READ_COP0(dest, R) \
         asm volatile (\
         ".set push \n" \
         ".set mips64 \n" \
         "dmfc0 %[rt]," R "\n" \
         ".set pop \n" \
         : [rt] "=r" (dest))

/* 
 *defines based on cvmx-interrupt.c 
 */
#define COP0_INDEX			"$0,0"
#define COP0_ENTRYLO0			"$2,0"
#define COP0_ENTRYLO1			"$3,0"
#define COP0_PAGEMASK			"$5,0"
#define COP0_PAGEGRAIN			"$5,1"
#define COP0_BADVADDR			"$8,0"
#define COP0_ENTRYHI			"$10,0"
#define COP0_STATUS			"$12,0"
#define COP0_CAUSE			"$13,0"
#define COP0_EPC			"$14,0"
#define COP0_PRID			"$15,0"
#define COP0_CONFIG			"$16,0"
#define COP0_WATCHLO			"$18,0"
#define COP0_WATCHHI			"$19,0"
#define COP0_ECC			"$26,0"
#define COP0_ICACHE_ERR			"$27,0"
#define COP0_DCACHE_ERR			"$27,1"
#define COP0_ITAGLO			"$28,0"
#define COP0_IDATALO			"$28,1"
#define COP0_DTAGLO			"$28,2"
#define COP0_DDATALO			"$28,3"
#define COP0_IDATAHI			"$29,1"
#define COP0_DTAGHI			"$29,2"
#define COP0_DDATAHI			"$29,3"
#define COP0_ERROR_EPC			"$30,0"
#define COP0_DESAVE			"$31,0"

/*
 * SLI registers
 */
#define SLI_MSI_RCV0    0x80011f0000013c10ULL
#define SLI_MSI_RCV1    0x80011f0000013c20ULL
#define SLI_MSI_RCV2    0x80011f0000013c30ULL
#define SLI_MSI_RCV3    0x80011f0000013c40ULL
#define SLI_MSI_WR_MAP 0x80011f0000013c90ULL
#define SLI_MSI_RD_MAP 0x80011f0000013ca0ULL
#define SLI_PCIE_MSI_RCV 0x80011f0000013cb0ULL
#define SLI_MSI_ENB0 0x80011f0000013c50ULL
#define SLI_MSI_ENB1 0x80011f0000013c60ULL
#define SLI_MSI_ENB2 0x80011f0000013c70ULL
#define SLI_MSI_ENB3 0x80011f0000013c80ULL
#define SLI_STATE1 0x80011f0000010620ULL
#define SLI_CTL_PORT0 0x80011f0000010050ULL
#define SLI_CTL_PORT1 0x80011f0000010060ULL

static const int CAVIUM_GPIO_PIN5 = 1<<5; /* GPIO 5 */
static const int CAVIUM_GPIO_PIN6 = 1<<6; /* GPIO 6 */
static const int CAVIUM_GPIO_PIN7 = 1<<7; /* GPIO 7 */

/* 
 *SMI registers
 */
#define SMI_DRV_CTL 0x8001180000001828ULL

#define SMI_0_CMD 0x8001180000003800ULL
#define SMI_0_WR_DAT 0x8001180000003808ULL
#define SMI_0_RD_DAT 0x8001180000003810ULL
#define SMI_0_CLK 0x8001180000003818ULL
#define SMI_0_WR_EN 0x8001180000003820ULL

#define SMI_1_CMD 0x8001180000003880ULL
#define SMI_1_WR_DAT 0x8001180000003888ULL
#define SMI_1_RD_DAT 0x8001180000003890ULL
#define SMI_1_CLK 0x8001180000003898ULL
#define SMI_1_WR_EN 0x80011800000038A0ULL

#define SMI_2_CMD 0x8001180000003900ULL
#define SMI_2_WR_DAT 0x8001180000003908ULL
#define SMI_2_RD_DAT 0x8001180000003910ULL
#define SMI_2_CLK 0x8001180000003918ULL
#define SMI_2_WR_EN 0x8001180000003920ULL

#define SMI_3_CMD 0x8001180000003980ULL
#define SMI_3_WR_DAT 0x8001180000003988ULL
#define SMI_3_RD_DAT 0x8001180000003990ULL
#define SMI_3_CLK 0x8001180000003998ULL
#define SMI_3_WR_EN 0x80011800000039A0ULL

void print_smi_reg (void)
{
    uint64_t smi0,smi1,smi2,smi3;
    uint max_num=3,min_num=0;
    int smi_port_num;
    
    smi_port_num = getdec_answer("Enter SMI port number 0~3", 0, min_num, max_num);
    printf("Display SMI %d registers\n",smi_port_num);
  
    switch (smi_port_num) {
        case 0:
                smi0 = cvmx_read_csr(SMI_DRV_CTL);
                printf("\n SMI_DRV_CTL = ");
                print_64bit_hex(smi0);

                smi0 = cvmx_read_csr(SMI_0_CMD);
                printf("\n SMI_0_CMD = ");
                print_64bit_hex(smi0);

                smi0 = cvmx_read_csr(SMI_0_WR_DAT);
                printf("\n SMI_0_WR_DAT = ");
                print_64bit_hex(smi0);

                smi0 = cvmx_read_csr(SMI_0_RD_DAT);
                printf("\n SMI_0_RD_DAT = ");
                print_64bit_hex(smi0);

                smi0 = cvmx_read_csr(SMI_0_CLK);
                printf("\n SMI_0_CLK = ");
                print_64bit_hex(smi0);

                smi0 = cvmx_read_csr(SMI_0_WR_EN);
                printf("\n SMI_0_WR_EN = ");
                print_64bit_hex(smi0);
                
                cvmx_write_csr(SMI_0_WR_EN, 0x0);
                smi0 = cvmx_read_csr(SMI_0_WR_EN);
                printf("\n SMI_0_WR_EN = ");
                print_64bit_hex(smi0);
                break;
        case 1:
                smi1 = cvmx_read_csr(SMI_DRV_CTL);
                printf("\n SMI_DRV_CTL = ");
                print_64bit_hex(smi1);

                smi1 = cvmx_read_csr(SMI_1_CMD);
                printf("\n SMI_1_CMD = ");
                print_64bit_hex(smi1);

                smi1 = cvmx_read_csr(SMI_1_WR_DAT);
                printf("\n SMI_1_WR_DAT = ");
                print_64bit_hex(smi1);

                smi1 = cvmx_read_csr(SMI_1_RD_DAT);
                printf("\n SMI_1_RD_DAT = ");
                print_64bit_hex(smi1);

                smi1 = cvmx_read_csr(SMI_1_CLK);
                printf("\n SMI_1_CLK = ");
                print_64bit_hex(smi1);

                smi1 = cvmx_read_csr(SMI_1_WR_EN);
                printf("\n SMI_1_WR_EN = ");
                print_64bit_hex(smi1);

                cvmx_write_csr(SMI_1_WR_EN, 0x0);
                smi0 = cvmx_read_csr(SMI_1_WR_EN);
                printf("\n SMI_0_WR_EN = ");
                print_64bit_hex(smi0);
                break;
        case 2:
                smi2 = cvmx_read_csr(SMI_DRV_CTL);
                printf("\n SMI_DRV_CTL = ");
                print_64bit_hex(smi2);

                smi2 = cvmx_read_csr(SMI_2_CMD);
                printf("\n SMI_2_CMD = ");
                print_64bit_hex(smi2);

                smi2 = cvmx_read_csr(SMI_2_WR_DAT);
                printf("\n SMI_2_WR_DAT = ");
                print_64bit_hex(smi2);

                smi2 = cvmx_read_csr(SMI_2_RD_DAT);
                printf("\n SMI_2_RD_DAT = ");
                print_64bit_hex(smi2);

                smi2 = cvmx_read_csr(SMI_2_CLK);
                printf("\n SMI_2_CLK = ");
                print_64bit_hex(smi2);

                smi2 = cvmx_read_csr(SMI_2_WR_EN);
                printf("\n SMI_2_WR_EN = ");
                print_64bit_hex(smi2);

               
                cvmx_write_csr(SMI_2_WR_EN, 0x0);
                smi0 = cvmx_read_csr(SMI_2_WR_EN);
                printf("\n SMI_0_WR_EN = ");
                print_64bit_hex(smi0);
                break;
        case 3:
                smi3 = cvmx_read_csr(SMI_DRV_CTL);
                printf("\n SMI_DRV_CTL = ");
                print_64bit_hex(smi3);

                smi3 = cvmx_read_csr(SMI_3_CMD);
                printf("\n SMI_3_CMD = ");
                print_64bit_hex(smi3);

                smi3 = cvmx_read_csr(SMI_3_WR_DAT);
                printf("\n SMI_3_WR_DAT = ");
                print_64bit_hex(smi3);

                smi3 = cvmx_read_csr(SMI_3_RD_DAT);
                printf("\n SMI_3_RD_DAT = ");
                print_64bit_hex(smi3);

                smi3 = cvmx_read_csr(SMI_3_CLK);
                printf("\n SMI_3_CLK = ");
                print_64bit_hex(smi3);

                smi3 = cvmx_read_csr(SMI_3_WR_EN);
                printf("\n SMI_3_WR_EN = ");
                print_64bit_hex(smi3);

                               
                cvmx_write_csr(SMI_3_WR_EN, 0x0);
                smi0 = cvmx_read_csr(SMI_3_WR_EN);
                printf("\n SMI_0_WR_EN = ");
                print_64bit_hex(smi0);
                break;
        default : 
                 printf("Fail, port number error\n");
    }
}

/*-----------------below functions copy from mon_exceptn.c-------------------*/
/*
 * Function print_cavium_status_reg
 *
 * This function will display the octeon status register
 *
 * Input: none.
 *
 * Output: none.
 */
void 
print_cavium_status_reg (void)
{
    uint64_t status0, status1, status2;

    status0 = cvmx_read_csr(0x8001180080940078ULL);
    printf("\n COP0_MAP0 = ");
    print_64bit_hex(status0);

    status1 = cvmx_read_csr(0x8001180080940008ULL);
    printf("\n COP1_MAP1 = ");
    print_64bit_hex(status1);

    status2 = cvmx_read_csr(0x800118008094fff8ULL);
    printf("\n COP1_MAP2 = ");
    print_64bit_hex(status2);
    
    /*READ_COP0(status, COP0_STATUS);
    printf("\n CPU Status  = ");
    print_64bit_hex(status);
    printf("\n");*/
}

/*
 * Function print_cavium_cause_reg
 *
 * This function will display the octeon cause register.
 *
 * Input: none.
 *
 * Output: none.
 */
void 
print_cavium_cause_reg (void)
{
    uint64_t cause;

    READ_COP0(cause, COP0_CAUSE);
    printf("\n CPU Cause  = ");
    print_64bit_hex(cause);
    printf("\n");

    uint32_t id;
    asm ("mfc0 %0, $15,0;nop" : "=r" (id));
    print_64bit_hex(id);
    printf("\n");
}

/*
 * Function print_cavium_prid_reg
 *
 * This function will display the processor identification register
 *
 * Input: none.
 *
 * Output: none.
 */
void
print_cavium_prid_reg (void)
{
    /*uint64_t status;

    READ_COP0(status, COP0_PRID);
    printf("\n PRID = ");
    print_64bit_hex(status);
    printf("\n");*/

    #ifdef CVMX_BUILD_FOR_LINUX_USER
    extern uint32_t cvmx_app_init_processor_id;
    print_64bit_hex(cvmx_app_init_processor_id);
    printf("\n");
    #else
    uint32_t id;
    asm ("mfc0 %0, $15,0" : "=r" (id));
    print_64bit_hex(id);
    printf("\n");
    #endif

}

/*
 * Function print_cavium_desave_reg
 *
 * This function will display the cop0 desave register
 * This is a scratch register.
 *
 * Input: none.
 *
 * Output: none.
 */
void 
print_cavium_desave_reg (void)
{
    uint64_t rdval;

    READ_COP0(rdval, COP0_DESAVE);
    printf("\n DESAVE register value =");
    print_64bit_hex(rdval);
    printf("\n");
}

/*
 * Function write_cavium_cop0_reg
 *
 * This function will write the octeon cp0 desave and cause register
 * Used to test ability to write to cop0 registers.
 * DSAVE is a scratch register.
 * Verified that set_cp0_cause_reg() does not work but setcp0_cause() does
 *
 * Input: none.
 *
 * Output: none.
 */
void 
write_cavium_cop0_reg (void)
{
    uint64_t status, wrval;
    uint val;

    val = gethex_answer("\nCOP0 register type: 1=DESAVE, 2=CAUSE: ",
			  1, 1, 2);
    wrval = gethex_answer("\nEnter LS 32-bit value to write : ",
			  0, 0, 0xFFFFFFFF);

    if (val == 1) {
        READ_COP0(status, COP0_DESAVE);
        printf("\nprior register value ");
        print_cavium_cop0_reg(val);

        /* write new value and verify */
        /*setcp0_desave(wrval);*/
        printf("\nnew register value ");
        print_cavium_cop0_reg(val);
    } else {
        READ_COP0(status, COP0_CAUSE);
        printf("\nprior register value ");
        print_cavium_cop0_reg(val);

        /* write new value and verify
         * use bits 9:8 to verify write to cause register
         */
        //set_cp0_cause_reg(wrval);
        /*setcp0_cause(wrval);*/
        printf("\nnew register value ");
        print_cavium_cop0_reg(val);
    };
}

/*
 * Function print_cavium_msi_regs
 *
 * This function displays some of the important registers within
 * Octeon that shows PCIe status
 *
 * Input: none.
 *
 * Output: none.
 */
void 
print_cavium_msi_regs(void)
{
    uint64_t msi_wr_map, msi_rd_map, pcie_msi_rcv;
    uint64_t msi_rcv0, msi_rcv1, msi_rcv2, msi_rcv3;
    uint64_t msi_enb0, msi_enb1, msi_enb2, msi_enb3;
    uint64_t sli_state1,sli_ctl_port0,sli_ctl_port1;
 
    /* data sheet page-648 */
    /* Read and display the SLI_MSI_WR_MAP regsiter */
    msi_wr_map = cvmx_read_csr(SLI_MSI_WR_MAP);
    printf("\n SLI_MSI_WR_MAP   = ");
    print_64bit_hex(msi_wr_map);
    
    /* Read and display the SLI_MSI_RD_MAP regsiter */
    msi_rd_map = cvmx_read_csr(SLI_MSI_RD_MAP);
    printf("\n SLI_MSI_RD_MAP   = ");
    print_64bit_hex(msi_rd_map);
    
    /* Read and display the NPEI_PCIE_MSI_RCV regsiter */
    /* WOODLAWN-Read and display the SLI_PCIE_MSI_RCV regsiter */
    pcie_msi_rcv = cvmx_read_csr(SLI_PCIE_MSI_RCV);
    printf("\n SLI_PCIE_MSI_RCV  = ");
    print_64bit_hex(pcie_msi_rcv);

    /* Read and display the SLI_MSI_RCV0 regsiter */
    msi_rcv0 = cvmx_read_csr(SLI_MSI_RCV0);
    printf("\n SLI_MSI_RCV0  = ");
    print_64bit_hex(msi_rcv0);
    
    /* Read and display the SLI_MSI_RCV1 regsiter */
    msi_rcv1 = cvmx_read_csr(SLI_MSI_RCV1);
    printf("\n SLI_MSI_RCV1  = ");
    print_64bit_hex(msi_rcv1);
    /* Read and display the SLI_MSI_RCV2 regsiter */
    msi_rcv2 = cvmx_read_csr(SLI_MSI_RCV2);
    printf("\n SLI_MSI_RCV2  = ");
    print_64bit_hex(msi_rcv2);
    /* Read and display the SLI_MSI_RCV3 regsiter */
    msi_rcv3 = cvmx_read_csr(SLI_MSI_RCV3);
    printf("\n SLI_MSI_RCV3  = ");
    print_64bit_hex(msi_rcv3);

    /* Read and display the SLI_MSI_ENB0 regsiter */
    msi_enb0 = cvmx_read_csr(SLI_MSI_ENB0);
    printf("\n SLI_MSI_ENB0  = ");
    print_64bit_hex(msi_enb0);
    
    /* Read and display the SLI_MSI_ENB1 regsiter */
    msi_enb1 = cvmx_read_csr(SLI_MSI_ENB1);
    printf("\n SLI_MSI_ENB1  = ");
    print_64bit_hex(msi_enb1);
    
    /* Read and display the SLI_MSI_ENB2 regsiter */
    msi_enb2 = cvmx_read_csr(SLI_MSI_ENB2);
    printf("\n SLI_MSI_ENB2  = ");
    print_64bit_hex(msi_enb2);
    
    /* Read and display the SLI_MSI_ENB3 regsiter */
    msi_enb3 = cvmx_read_csr(SLI_MSI_ENB3);
    printf("\n SLI_MSI_ENB3  = ");
    print_64bit_hex(msi_enb3);

    /* Read and display the SLI_STATE1 regsiter */
    sli_state1 = cvmx_read_csr(SLI_STATE1);
    printf("\n SLI_STATE1  = ");
    print_64bit_hex(sli_state1);

    /* Read and display the SLI_CTL_PORT0 regsiter */
    sli_ctl_port0 = cvmx_read_csr(SLI_CTL_PORT0);
    printf("\n SLI_CTL_PORT0  = ");
    print_64bit_hex(sli_ctl_port0);

    /* Read and display the SLI_CTL_PORT1 regsiter */
    sli_ctl_port1 = cvmx_read_csr(SLI_CTL_PORT1);
    printf("\n SLI_CTL_PORT1  = ");
    print_64bit_hex(sli_ctl_port1);

}

/*-----------------------cvmx-cpie.c------------------------------*/
void cvmx_dump_npei(int pcie_port)
{
    uint idx = 0;
    uint64_t start;
    printf("\nNPEI REGISTERS\n");
    printf("=========================================\n");
    for (idx = 0; idx < 32; idx+=4) {
        printf("@");
        print_64bit_hex(CVMX_PEXP_NPEI_BAR1_INDEXX(idx));
        printf("(%2d): %#07lx %#07lx %#07lx %#07lx\n", idx,
               cvmx_read_csr(CVMX_PEXP_NPEI_BAR1_INDEXX(idx+0)),
               cvmx_read_csr(CVMX_PEXP_NPEI_BAR1_INDEXX(idx+1)),
               cvmx_read_csr(CVMX_PEXP_NPEI_BAR1_INDEXX(idx+2)),
               cvmx_read_csr(CVMX_PEXP_NPEI_BAR1_INDEXX(idx+3)));
    }

    for (start = CVMX_PEXP_NPEI_CTL_PORT0; start <=0x80011F00000095F0ull;
         start+=8) {
        printf("@");
        print_64bit_hex(start);
        printf("\t=");
        print_64bit_hex(cvmx_read_csr(start));
        printf("\n");
    }
    for (start = CVMX_PEXP_NPEI_CTL_PORT0; start <=0x80011F00000095F0ull;
         start+=8) {
        printf("@");
        print_64bit_hex(start);
        printf("\t=");
        print_64bit_hex(cvmx_read_csr(start));
        printf("\n");
    }

    for (start = (uint64_t)CVMX_PEXP_NPEI_PKTX_SLIST_BAOFF_DBELL(0x0);
         start <=0x80011F0000009DF0ull;
         start+=8) {
        printf("@");
        print_64bit_hex(start);
        printf("\t=");
        print_64bit_hex(cvmx_read_csr(start));
        printf("\n");
    }

    for (start = (uint64_t)CVMX_PEXP_NPEI_PKTX_CNTS(0x0);
         start <=0x80011F000000A5F0ull;
         start+=8) {
        printf("@");
        print_64bit_hex(start);
        printf("\t=");
        print_64bit_hex(cvmx_read_csr(start));
        printf("\n");
    }

    for (start = (uint64_t)CVMX_PEXP_NPEI_PKTX_INSTR_BADDR(0x0); start <= (31*8);
         start+=8) {
        printf("@");
        print_64bit_hex(start);
        printf("\t=");
        print_64bit_hex(cvmx_read_csr(start));
        printf("\n");
    }

    for (start = (uint64_t)CVMX_PEXP_NPEI_PKTX_INSTR_BAOFF_DBELL(0x0); start <= (31*8);
         start+=8) {
        printf("@");
        print_64bit_hex(start);
        printf("\t=");
        print_64bit_hex(cvmx_read_csr(start));
        printf("\n");
    }

    for (start = (uint64_t)CVMX_PEXP_NPEI_PKTX_INSTR_FIFO_RSIZE(0x0); start <= (31*8);
         start+=8) {
        printf("@");
        print_64bit_hex(start);
        printf("\t=");
        print_64bit_hex(cvmx_read_csr(start));
        printf("\n");
    }

    for (start = (uint64_t)CVMX_PEXP_NPEI_PKTX_INSTR_HEADER(0x0); start <= (31*8);
         start+=8) {
        printf("@");
        print_64bit_hex(start);
        printf("\t=");
        print_64bit_hex(cvmx_read_csr(start));
        printf("\n");
    }

    for (start = (uint64_t)CVMX_PEXP_NPEI_PKTX_IN_BP(0x0); start <= (31*8);
         start+=8) {
        printf("@");
        print_64bit_hex(start);
        printf("\t=");
        print_64bit_hex(cvmx_read_csr(start));
        printf("\n");
    }

    for (start = (uint64_t)CVMX_PEXP_NPEI_CTL_STATUS2; start <= CVMX_PEXP_NPEI_INT_ENB2;
         start+=8) {
        printf("@");
        print_64bit_hex(start);
        printf("\t=");
        print_64bit_hex(cvmx_read_csr(start));
        printf("\n");
    } 

}

/* 
 *CN68XX-in page 906
 *display octeon cfg reg pcie_port 0
 *RC mode PCIe configuration registers
 *
 *display PCIe configuration registers
 */
void cvmx_dump_cfg(int pcie_port)
{
    int idx;
    uint64_t val=0x0000000000000018;
    uint64_t cfg;

    /* RC Mode*/
    /*cvmx_write_csr((0x80011800c0000030ull)+((pcie_port) & 1)*0x1000000ull, val);
    cfg = cvmx_read_csr((0x80011800c0000030ull)+((pcie_port) & 1)*0x1000000ull);
    print_64bit_hex(cfg);*/

    cvmx_write_csr(0x80011800c0000028ull, val);
    cfg = cvmx_read_csr(0x80011800c0000030ull);
    print_64bit_hex(cfg);

    cfg = cvmx_read_csr(0x80011800c0000018ull);
    print_64bit_hex(cfg);
    
    printf("\nPCI_CFG[0..74] on PCIe port %d\n", pcie_port);
    for (idx = 0; idx < 74; idx+=4) {
        printf("idx=%d\n",idx);
        printf("\t0x%02x [CFG%02d]: 0x%08x 0x%08x 0x%08x 0x%08x\n",
               idx*4, idx,
               cvmx_pcie_cfgx_read(pcie_port, CVMX_PCIERCX_CFG000(pcie_port)+idx*4+0x0),
               cvmx_pcie_cfgx_read(pcie_port, CVMX_PCIERCX_CFG000(pcie_port)+idx*4+0x4),
               cvmx_pcie_cfgx_read(pcie_port, CVMX_PCIERCX_CFG000(pcie_port)+idx*4+0x8),
               cvmx_pcie_cfgx_read(pcie_port, CVMX_PCIERCX_CFG000(pcie_port)+idx*4+0xc));
    }

    printf("\nPCI_CFG[448..492] on PCIe port %d\n", pcie_port);
    for (idx = 448; idx < 492; idx+=4) {
        printf("\t0x%02x [CFG%02d]: 0x%08x 0x%08x 0x%08x 0x%08x\n",
               idx*4, idx,
               cvmx_pcie_cfgx_read(pcie_port, CVMX_PCIERCX_CFG000(pcie_port)+idx*4+0x0),
               cvmx_pcie_cfgx_read(pcie_port, CVMX_PCIERCX_CFG000(pcie_port)+idx*4+0x4),
               cvmx_pcie_cfgx_read(pcie_port, CVMX_PCIERCX_CFG000(pcie_port)+idx*4+0x8),
               cvmx_pcie_cfgx_read(pcie_port, CVMX_PCIERCX_CFG000(pcie_port)+idx*4+0xc));
    }

}

/*
 * display octeon pesc pcie_port 0
 */
void cvmx_dump_pesc(int pcie_port)
{
    uint64_t start;
    printf("\nPESC REGISTERS pcie port %d:\n", pcie_port);
    printf("=========================================\n");
    for (start = CVMX_PESCX_CTL_STATUS(pcie_port);
         start <= CVMX_PESCX_BIST_STATUS2(pcie_port);
         start+=8) {
        printf("@");
        print_64bit_hex(start);
        printf("\t=");
        print_64bit_hex(cvmx_read_csr(start));
        printf("\n");
    }

}

/*-----------------------map_mem_test.c------------------------------*/
/*
 * Function: test_malloc_dev
 *
 * This function is test for malloc_dev 
 *
 * Input: None
 *
 * Output: PASSED/FAILED
 */
int
test_malloc_dev (void)
{
    uchar *buff_p;
    uint  *mem_addr;

    buff_p = (uchar *)malloc_dev((unsigned long)1024);
    if (buff_p == NULL) {
        cterr('f', 0, "Unable to allocate memory for data buffer");
        return(FAILED);
    }
    mem_addr = (uint *)buff_p;

    printf("\n mem_addr = %#lx", (unsigned long)mem_addr);
    dismem((uchar *)mem_addr, 64, (ulong)mem_addr, BW_32BITS);
    return (PASSED);
}

/*
 * Function: test_malloc
 *
 * This function is test for malloc 
 *
 * Input: None
 *
 * Output: PASSED/FAILED
 */
int
test_malloc (void)
{
    ulong *buff_p;
    ulong  *mem_addr;
    int   i;

    for (i = 0; i < 60; i++) {
       buff_p = (ulong *)malloc((unsigned long)TWO_MEG);
       if (buff_p == NULL) {
           cterr('f', 0, "Unable to allocate memory for data buffer");
           return(FAILED);
       }
       mem_addr = buff_p;

       printf("\n mem_addr = %#lx", (unsigned long)mem_addr);
       dismem((uchar *)mem_addr, 64, (ulong)mem_addr, BW_32BITS);
    }
    return (PASSED);
}

/*-----------------------platform_utils.c------------------------------*/
/*
 * Function: set_cavium_gpio_pin5
 *
 * This functions is will set the Cavium GPIO Pin5
 *
 * Input : none
 * 
 * Output: none.
 */
int
set_cavium_gpio_pin5 (void)
{
#ifdef LINUX_APP
    cvmx_gpio_set(CAVIUM_GPIO_PIN5);
#endif
    return PASSED;
}

/*-----------------------------------------------------*/
void
print_64bit_hex (uint64_t val)
{
    uint32_t high_val, low_val;
    char buff[40];

    low_val = (uint32) val;
    high_val  = (val >> 32);

    sprintf(buff, " 0x%08X %08X", high_val, low_val);
    puts(buff);
}

/*
 * Function print_cavium_cop0_reg
 *
 * This function will display the requested octeon cp0 register
 *
 * Input: none.
 *
 * Output: none.
 */
void 
print_cavium_cop0_reg (int regval)
{
    uint64_t rdval;
    char buffer[8];

    if (regval == 1) {
        READ_COP0(rdval, COP0_DESAVE);
        sprintf(buffer, "DESAVE");
    } else {
        READ_COP0(rdval, COP0_CAUSE);
        sprintf(buffer, "CAUSE");
    }
    printf("\n %s register value =", buffer);
    print_64bit_hex(rdval);
    printf("\n");
}

/******************************************************************************
 *
 * Function: get_gpio_rx_dat_bits
 *    Get the Cavium GPIO bit value specified by the bit mask
 *
 * Input: bitmask - bit mask for the GPIO bits
 *
 * Return: GPIO value
 *****************************************************************************/
uint32_t get_gpio_rx_dat_bits (uint32_t bitmask)
{
    cvmx_gpio_rx_dat_t gpio_rx_dat;
    uint32_t bitval;

    /* Make sure gmxno is clear for rx */
    gpio_rx_dat.u64 = cvmx_read_csr(CVMX_GPIO_RX_DAT);
    bitval = (uint32_t)gpio_rx_dat.u64 & bitmask;

#if DEBUG
    printf("%s gpio_rx_dat= 0x%lx bitval= %x\n",
       __FUNCTION__, gpio_rx_dat.u64, bitval);
#endif

    return(bitval);
}

/*-------------------------------------------------
 * $Log: cavium_register_access.c,v $
 * Revision 1.2  2013/10/08 08:48:27  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.4.2  2013/08/20 10:58:49  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.1  2013/04/24 10:37:12  tirawan
 * Initial check-in for woodlawn linux code
 *
 * Revision 1.2  2013/03/27 04:49:34  kuangik
 * Code cleanup after adding -Wall
 *
 * Revision 1.8  2012/10/24 06:44:33  leslie
 * Fix print smi reg function.
 *
 * Revision 1.7  2012/08/28 08:29:19  leslie
 * Add function get_gpio_rx_dat_bits.
 *
 * Revision 1.6  2012/08/03 10:16:55  leslie
 * Mapping to latest O2 source code on 20120726
 *
 * Revision 1.2  2012/07/19 06:42:42  leslie
 * Fix error message.
 *
 * Revision 1.1  2012/07/05 06:02:41  leslie
 * Add Woodlawn cavium register r/w test
 *
 * $Endlog $
 *-------------------------------------------------
 */
