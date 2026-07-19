/* $Id: fortitude_framer.c,v 1.8 2013/04/19 18:33:37 ywen Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fortitude/fortitude_framer.c,v $
 *------------------------------------------------------------------
 *
 * fortitude_framer.c - This file contains functions for PMC4358 framer.
 *
 * Christine Wen -- Oct. 2011
 *
 * Copyright (c) 2011-2013 by Cisco Systems, Inc.
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
#include "common.h"
#include "types.h" 
#include "defs.h"
#include "error.h"
#include "nvmonvars.h"
#include "pcmap.h"
#include "dev_object.h"
#include "dev_4359.h"
#include "fortitude.h"

#undef DEBUG 

static ulong saved_rbase = 0;
static ulong saved_wbase = 0;

dev_4359_object_t dev_4359_object;

extern uint32 err_report (dev_object_t *dev, char *err_msg,
			  uint32 err_type);
extern void fpga_set_framer_txhiz(int ena);
extern void fpga_reset_framer();
extern void fpga_unreset_framer();

static int set_pmc_clk_mode (int port_num, int op_mode, frmr_clk_mode clk_mode);

/*********************************************************************
 *
 * Function: framer_read_reg()
 *
 * Description: This function performs a read to NPU memory-mapped
 *              framer registers.  
 *
 * Inputs:  base_addr - Base address of the chip.
 *          offset    - Offset of the register to be read.
 *	    bus_width - BW_8BITS, BW_16BITS, BW_32BITS
 *
 * Outputs: Register value.
 *
 *********************************************************************
 */
static uchar
framer_read_reg (ulong base_addr, ulong offset, uchar bus_width)
{
    uchar rdval;
    ulong mem_offset;
    volatile uchar *mem_addr;

    mem_offset = offset * bus_width;
    mem_addr = (volatile uchar *)(base_addr + mem_offset);
    rdval = *mem_addr;

#ifdef DEBUG
    printf("framer_read_reg(%#x, %#x, %#x) mem_addr @%#x = %#.2x\n",
	   base_addr, offset, bus_width, (ulong)mem_addr, rdval);
#endif

    return(rdval);
}

/*********************************************************************
 *
 * Function: framer_write_reg()
 *
 * Description: This function performs a write to NPU memory-mapped
 *              framer registers.  
 *
 * Inputs:  base_addr - Base address of the chip.
 *          offset    - Offset of the register to write.
 *          wr_val    - Value to be written
 *          bus_width - size
 *
 * Outputs: None
 *
 *********************************************************************
 */
static void
framer_write_reg (ulong base_addr, ulong offset, uchar wr_val, uchar bus_width)
{
    volatile uchar rdval;
    ulong mem_offset;
    volatile uchar *mem_addr;

    mem_offset = offset * bus_width;
    mem_addr = (volatile uchar *)(base_addr + mem_offset);
    *mem_addr = wr_val;

    /* dummy read to insure posted write completes */
    rdval = *mem_addr;

#ifdef DEBUG
    printf("framer_write_reg(%#.8x, %#.2x, %#.2x, %#x) write to %#.8x\n",
	   base_addr, offset, wr_val, bus_width, (ulong)mem_addr);
#endif
}


/*****************************************************************
 *
 * Function: framer_attach()
 *
 * This function creates the PMC4359 device object and initializes
 * it with necessary platform specific information ready for use.
 *
 * Input: None
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************/
int 
framer_attach()
{
    dev_4359_object_t *pmc4359 = &dev_4359_object;
    dev_object_t *dev = (dev_object_t *)pmc4359;
    int rc;

    /* After power on, the Framer is in reset. Take it out of reset first. */
    fpga_unreset_framer();

    /* Create common device object */
    pmc4359_dev_create(dev, (dev_error_report_t) err_report);
    
    /* Attach the device object */
    rc = pmc4359->base.dev_object_fvt->dev_attach(dev);

    if (rc != PASSED) {
	pmc4359->base.dev_object_fvt->dev_destroy(&dev);
	cterr('f', 0, "Framer Device attach failed");
	return (FAILED);
    }

    /* Initialize framer base address */
    pmc4359->base.dev_addr = (void *)get_framer_base();
#ifdef DEBUG
    printf("pmc4359->base.dev_addr = %#x\n", (ulong)pmc4359->base.dev_addr);
#endif
    /* Setup call-out function vectors */
    pmc4359->callout_fvt->rd_frm_reg = framer_read_reg;
    pmc4359->callout_fvt->wr_frm_reg = framer_write_reg;

    /*
     * NPU accesses Framer on 8 bit boundary through the local bus. 
     * Set bus_width accordingly so the addresses can be adjusted correctly.
     */
    pmc4359->bus_width = BW_8BITS;

    return (PASSED);
}

/*****************************************************************
 *
 * Function: framer_detach()
 *
 * This function will detach PMC4359 device object.
 *
 * Input: None
 *
 * Output: None
 *
 *****************************************************************/
void framer_detach()
{
    dev_4359_object_t *pmc4359 = &dev_4359_object;

    /* free all the memory from malloc in device_attach() */
    pmc4359->base.dev_object_fvt->dev_destroy((dev_object_t **)&pmc4359);
}


/**********************************************************************
 *
 * Function: framer_reg_test()
 *
 * This function will perform the framer register test.
 * (calls dev_4359.c:pmc4359_reg_test())
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
framer_reg_test ()
{
    int retval, framer, num_ports;
    dev_4359_object_t *pmc4359_p = &dev_4359_object;

    prpass(testpass ,"framer registers test");

    fpga_reset_framer();
    usleep(1000);
    fpga_unreset_framer();
    usleep(1000);

    /* perform framer register test */
    num_ports = get_num_ports();

    retval = pmc4359_p->callin_fvt->register_test((dev_object_t *)pmc4359_p,
						  num_ports);

    if (retval != PASSED) {
	/* added for debugging */
	framer = (retval & 0x700) >> 8;
	cterr('f', 0, "Framer%d register test failure", framer);
    }

    return(retval);
}

/**********************************************************************
 *
 * Function: framer_ycable_util()
 *
 * This function will enable/disable y-cable mode on the PMC framer.
 * This test requires user interaction.
 *
 * Input : None
 *
 * Output: Always return PASSED to avoid compilation warning
 *
 **********************************************************************
 */
int
framer_ycable_util ()
{
    ulong op_mode;
    uchar port_num, port_max, frm_port;
    volatile uchar *mem_addr, rdval;
    dev_4359_object_t *pmc4359_p;
    int i;

    printf("\n Y-cable utility\n");
    port_max = get_num_ports();
    if (port_max > 1) {
        port_num = gethex_answer("\nEnter port number:", 0, 0, port_max - 1);
    } else {
        port_num = 0;
    }
    if ((port_max == 2) & (port_num == 1)) {
	frm_port = 3;
    } else {
	frm_port = port_num;
    }

    op_mode = gethex_answer("\nDisable (0) or Enable (1) Y-cable mode ?:",
			    0, 0, 1);

    pmc4359_p = (dev_4359_object_t *)&dev_4359_object;
 
    pmc4359_p->callin_fvt->ycable_enab((dev_object_t *)pmc4359_p,
                                       frm_port, op_mode);

    mem_addr = (volatile uchar *)(get_framer_base() +
	       (frm_port * CMQ_FRM_OFFSET) + CMQ_MST_DIAG);

    for (i = 0; i < 4; i++) {
	rdval = *mem_addr;
	printf("\nframer Master Diagnostics register @%#x is set to %#x\n",
	       mem_addr, rdval);
	mem_addr++;
    }

    return(PASSED);
}


/**********************************************************************
 *
 * Function: framer_display_regs()
 *
 * This function will interact with the user to find out the neccessary
 * inputs. It then will display framer registers based on those inputs.
 * 
 * The framer registers are of size byte, and also are accessed as
 * 8 bit quantities.
 *
 * Input : None
 *
 * Output: Always return PASSED to avoid compilation warning
 *
 **********************************************************************
 */
int
framer_display_regs ()
{
    int offset, port_num, port_max, num_ports, length;
    int mode, op_mode, reg_avail, count, frmr_port;
    uchar str_buf[80];
    dev_4359_object_t *pmc4359_p;

    port_max = get_num_ports();

    if (port_max > 1) {
        sprintf(str_buf, "\nEnter framer port number[0-%d] (%d for all ports):",
	        (port_max-1), port_max);
        port_num = gethex_answer(str_buf, 0, 0, port_max);
    } else {
	port_num = 0;
    }

    mode = gethex_answer("all registers (1) or status registers only (0)",
	1, 0, 1);

    if (port_num == port_max) {
	num_ports = port_max;
	frmr_port = 0;
    } else {
	num_ports = 1;
	if ((port_max == 2) & (port_num == 1)) {
	    frmr_port = 3;
	} else {
	    frmr_port = port_num;
	}
    }

    pmc4359_p = (dev_4359_object_t *)&dev_4359_object;
    op_mode = CMQ_MODE_T1;
    for (count = 0; count < num_ports; count++) {
	pmc4359_p->callin_fvt->set_cfg_info(
		(dev_object_t *)pmc4359_p, frmr_port, op_mode);
	if (mode) {
	    pmc4359_p->base.dev_object_fvt->dev_show(
		(dev_object_t *)pmc4359_p,
		(print_fn_t)printf, DEV_SHOW_ALL);
	} else {
	    pmc4359_p->base.dev_object_fvt->dev_show(
		(dev_object_t *)pmc4359_p,
		(print_fn_t)printf, DEV_SHOW_STATUS);
	}
	if (num_ports == 2)
	    frmr_port = 3;
	else
	    frmr_port++;
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: framer_read_rlps_ram()
 *
 * This function will read the contents of the framer rlps ram and
 * either dump it or verify it against the previously written value
 *
 * Input : flag: 0 = verify, 1 = dump
 *
 * Output: none
 *
 **********************************************************************
 */
static void
framer_read_rlps_ram (int flag)
{
    int port_num, port_max, num_ports;
    int op_mode, count, frmr_port;
    uchar str_buf[80];
    dev_4359_object_t *pmc4359_p;
    
    port_max = get_num_ports();
    if (port_max > 1) {
        sprintf(str_buf, "\nEnter port number[0-%d] (%d for all ports):",
	        (port_max-1), port_max);
        port_num = gethex_answer(str_buf, 0, 0, port_max);
    } else {
	port_num = 0;
    }

    if (port_num == port_max) {
	num_ports = port_max;
	frmr_port = 0;
    } else {
	num_ports = 1;
	if ((port_max == 2) & (port_num == 1)) {
	    frmr_port = 3;
	} else {
	    frmr_port = port_num;
	}
    }

    pmc4359_p = (dev_4359_object_t *)&dev_4359_object;
    op_mode = CMQ_MODE_T1;
    for (count = 0; count < num_ports; count++) {
        pmc4359_p->callin_fvt->set_cfg_info((dev_object_t *)pmc4359_p,
					    frmr_port, op_mode);
        pmc4359_p->callin_fvt->read_rlps((dev_object_t *)pmc4359_p, flag);
	if (num_ports == 2)
	    frmr_port = 3;
	else
	    frmr_port++;
    }
}

/**********************************************************************
 *
 * Function: framer_dump_rlps_ram()
 *
 * This function will display the contents of the framer rlps ram
 *
 * Input : None
 *
 * Output: Always return PASSED to avoid compilation warning
 *
 **********************************************************************
 */
int
framer_dump_rlps_ram ()
{
    framer_read_rlps_ram(1);
    return(PASSED);
}

/**********************************************************************
 *
 * Function: framer_verify_rlps_ram()
 *
 * This function will verify the contents of the framer rlps ram
 *
 * Input : None
 *
 * Output: Always return PASSED to avoid compilation warning
 *
 **********************************************************************
 */
int
framer_verify_rlps_ram ()
{
    framer_read_rlps_ram(0);
    return(PASSED);
}

/**********************************************************************
 *
 * Function: framer_dump_indirect()
 *
 * This function will display the framer indirect registers,
 * TPSC, RPSC, and SIGX
 *
 * Input : None
 *
 * Output: Always return PASSED to avoid compilation warning
 *
 **********************************************************************
 */
int
framer_dump_indirect ()
{
    int port_num, port_max, num_ports;
    int op_mode, count, frmr_port, reg_type, reg_start;
    ulong frm_base_addr;
    uchar str_buf[80];
    dev_4359_object_t *pmc4359_p;
    
    port_max = get_num_ports();
    if (port_max > 1) {
        sprintf(str_buf, "\nEnter port number[0-%d] (%d for all ports):",
	        (port_max-1), port_max);
        port_num = gethex_answer(str_buf, 0, 0, port_max);
    } else {
	port_num = 0;
    }

    if (port_num == port_max) {
	num_ports = port_max;
	frmr_port = 0;
    } else {
	num_ports = 1;
	if ((port_max == 2) & (port_num == 1)) {
	    frmr_port = 3;
	} else {
	    frmr_port = port_num;
	}
    }

    reg_type = gethex_answer("Indirect register type, 0=TPSC, 1=RPSC, 2=SIGX",
        0, 0, 2);
    if (reg_type == 1) {
        reg_type = CMQ_RPSC_CFG;
	reg_start = CMQ_RPSC_IND_DATA_CTL;
        sprintf(str_buf, "RPSC");
    } else if (reg_type == 2) {
        reg_type = CMQ_SIGX_CFG_CHG_SIG_STATE;
	reg_start = CMQ_SIGX_IND_CUR_SIG_OFFSET_BASEADDR;
        sprintf(str_buf, "SIGX");
    } else {
        reg_type = CMQ_TPSC_CFG;
	reg_start = CMQ_TPSC_IND_DATA_CTL;
        sprintf(str_buf, "TPSC");
    }

    pmc4359_p = (dev_4359_object_t *)&dev_4359_object;
    op_mode = CMQ_MODE_T1;
    for (count = 0; count < num_ports; count++) {
        frm_base_addr = get_framer_base() + (frmr_port * CMQ_FRM_OFFSET);
        pmc4359_p->callin_fvt->set_cfg_info((dev_object_t *)pmc4359_p,
			frmr_port, op_mode);
        pmc4359_p->callin_fvt->dump_xpsc((dev_object_t *)pmc4359_p,
			frm_base_addr + reg_type, reg_start);
	if (num_ports == 2)
	    frmr_port = 3;
	else
	    frmr_port++;
    }

    return(PASSED);
}

/**********************************************************************
 *
 * Function: framer_rd_indirect()
 *
 * This function will display the requested framer indirect register,
 * of type TPSC, RPSC, and SIGX
 *
 * Input : None
 *
 * Output: Always return PASSED to avoid compilation warning
 *
 **********************************************************************
 */
int
framer_rd_indirect ()
{
    int offset, port_num, port_max, num_ports, frmr_port;
    int op_mode, count, st_reg, reg_type, num_reg, cnt, max_reg;
    ulong frm_base_addr;
    uchar val;
    uchar str_buf[80];
    dev_4359_object_t *pmc4359_p = (dev_4359_object_t *)&dev_4359_object;;
    
    port_max = get_num_ports();
    if (port_max > 1) {
        sprintf(str_buf, "\nEnter port number[0-%d] (%d for all ports):",
	        port_max, port_max);
        port_num = gethex_answer(str_buf, 0, 0, port_max);
    } else {
	port_num = 0;
    }

    if (port_num == port_max) {
	num_ports = port_max;
	frmr_port = 0;
    } else {
	num_ports = 1;
	if ((port_max == 2) & (port_num == 1)) {
	    frmr_port = 3;
	} else {
	    frmr_port = port_num;
	}
    }

    reg_type = gethex_answer("Indirect register type, 0=TPSC, 1=RPSC, 2=SIGX",
	0, 0, 2);
    if (reg_type == 1) {
	reg_type = CMQ_RPSC_CFG;
	st_reg = CMQ_RPSC_IND_OFFSET;
	max_reg = PM4359_NUM_IND_REG;
	sprintf(str_buf, "RPSC");
    } else if (reg_type == 2) {
	reg_type = CMQ_SIGX_CFG_CHG_SIG_STATE;
	st_reg = CMQ_SIGX_IND_CUR_SIG_OFFSET_BASEADDR;
	max_reg = PM4359_SIGX_NUM_IND_REG;
	sprintf(str_buf, "SIGX");
    } else {
	reg_type = CMQ_TPSC_CFG;
	st_reg = CMQ_TPSC_IND_OFFSET;
	max_reg = PM4359_NUM_IND_REG;
	sprintf(str_buf, "TPSC");
    }

    offset = gethex_answer("Enter indirect register offset",
			   st_reg, st_reg, st_reg+max_reg);

    num_reg = gethex_answer("Enter number of registers to read",
			    1, 1, max_reg);

    op_mode = CMQ_MODE_T1;

    for (count = 0; count < num_ports; count++) {
	frm_base_addr = get_framer_base() + (frmr_port * CMQ_FRM_OFFSET);
	printf("\n%s indirect register @%#x\noffset %#.2x  ",
	       str_buf, frm_base_addr + reg_type, offset);
        pmc4359_p->callin_fvt->set_cfg_info( (dev_object_t *)pmc4359_p,
		    frmr_port, op_mode);
	for (cnt = 0; cnt < num_reg; cnt++) {
            pmc4359_p->callin_fvt->rd_ind_reg( (dev_object_t *)pmc4359_p,
		    frm_base_addr + reg_type, offset + cnt, &val);
	    printf("%#.2x ", val);
	    if ((((offset + cnt) & 7) == 7) && (cnt < num_reg - 1))
		printf("\noffset %#.2x ", offset + cnt + 1);
	}
	if (num_ports == 2)
	    frmr_port = 3;
	else
	    frmr_port++;
    }

    return(PASSED);
}

/**********************************************************************
 *
 * Function: framer_wr_indirect()
 *
 * This function will allow the user to modify the framer indirect
 * registers: TPSC, RPSC, and SIGX
 *
 * Input : None
 *
 * Output: Always return PASSED to avoid compilation warning
 *
 **********************************************************************
 */
int
framer_wr_indirect ()
{
    int port_num, port_max, num_ports, reg_type, st_reg;
    int op_mode, reg_val, count, frmr_port, num_reg, cnt, max_reg;
    ulong frm_base_addr;
    uchar wrval, val;
    uchar str_buf[80];
    dev_4359_object_t *pmc4359_p;
    
    port_max = get_num_ports();
    if (port_max > 1) {
        sprintf(str_buf, "\nEnter framer port number[0-%d] (%d for all ports):",
	        port_max, port_max);
        port_num = gethex_answer(str_buf, 0, 0, port_max);
    } else {
	port_num = 0;
    }

    if (port_num == port_max) {
	num_ports = port_max;
	frmr_port = 0;
    } else {
	num_ports = 1;
	if ((port_max == 2) & (port_num == 1)) {
	    frmr_port = 3;
	} else {
	    frmr_port = port_num;
	}
    }
    
    reg_type = gethex_answer("Indirect register type, 0=TPSC, 1=RPSC, 2=SIGX",
	0, 0, 2);

    if (reg_type == 1) {
	reg_type = CMQ_RPSC_CFG;
	st_reg = CMQ_RPSC_IND_OFFSET;
	max_reg = PM4359_NUM_IND_REG;
	sprintf(str_buf, "RPSC");
    } else if (reg_type == 2) {
	reg_type = CMQ_SIGX_CFG_CHG_SIG_STATE;
	st_reg = CMQ_SIGX_IND_CUR_SIG_OFFSET_BASEADDR;
	max_reg = PM4359_SIGX_NUM_IND_REG;
	sprintf(str_buf, "SIGX");
    } else {
	reg_type = CMQ_TPSC_CFG;
	st_reg = CMQ_TPSC_IND_OFFSET;
	max_reg = PM4359_NUM_IND_REG;
	sprintf(str_buf, "TPSC");
    }

    reg_val = gethex_answer("Enter indirect register offset: ",
	st_reg, st_reg, st_reg+max_reg);
    num_reg = gethex_answer("Enter number of registers to write",
	1, 1, max_reg);
    wrval = gethex_answer("Enter value to write: ", 0, 0, 0xff);

    pmc4359_p = (dev_4359_object_t *)&dev_4359_object;
    op_mode = CMQ_MODE_T1;
    for (count = 0; count < num_ports; count++) {
	frm_base_addr = get_framer_base() + (frmr_port * CMQ_FRM_OFFSET);
        pmc4359_p->callin_fvt->set_cfg_info((dev_object_t *)pmc4359_p,
					    frmr_port, op_mode);
	for (cnt = 0; cnt < num_reg; cnt++) {
            pmc4359_p->callin_fvt->wr_ind_reg((dev_object_t *)pmc4359_p,
		    frm_base_addr + reg_type, reg_val + cnt, wrval);
            pmc4359_p->callin_fvt->rd_ind_reg((dev_object_t *)pmc4359_p,
		    frm_base_addr + reg_type, reg_val + cnt, &val);
	    if (wrval != val) {
	        cterr('f', 0, "Framer%d %s reg offset %#x, wrote %#x, read %#x",
		    frmr_port, str_buf, reg_val + cnt, wrval, val);
		count = num_ports;
		break;
	    }
	}
	if (num_ports == 2)
	    frmr_port = 3;
	else
	    frmr_port++;
    }

    return(PASSED);
}

/**********************************************************************
 *
 * Function: framer_rd_pw()
 *
 * This function will display pulse waveform configuration.
 *
 * Input : None
 *
 * Output: Always return PASSED to avoid compilation warning
 *
 **********************************************************************
 */
int
framer_rd_pw ()
{
    int offset, port_num, port_max, num_ports, frmr_port;
    int op_mode, count, i, j;
    ulong frm_base_addr;
    uchar val;
    uchar str_buf[80];
    dev_4359_object_t *pmc4359_p = (dev_4359_object_t *)&dev_4359_object;;
    
    port_max = get_num_ports();
    if (port_max > 1) {
        sprintf(str_buf, "\nEnter port number[0-%d] (%d for all ports):",
	        port_max, port_max);
        port_num = gethex_answer(str_buf, 0, 0, port_max);
    } else {
	port_num = 0;
    }

    if (port_num == port_max) {
	num_ports = port_max;
	frmr_port = 0;
    } else {
	num_ports = 1;
	if ((port_max == 2) & (port_num == 1)) {
	    frmr_port = 3;
	} else {
	    frmr_port = port_num;
	}
    }

    op_mode = CMQ_MODE_T1;

    for (count = 0; count < num_ports; count++) {
	frm_base_addr = get_framer_base() + (frmr_port * CMQ_FRM_OFFSET);
	printf("base_addr %x\n", frm_base_addr);
        pmc4359_p->callin_fvt->set_cfg_info( (dev_object_t *)pmc4359_p,
		    frmr_port, op_mode);
	pmc4359_p->callout_fvt->wr_frm_reg(frm_base_addr, 0xF0, 0x50,
					   pmc4359_p->bus_width);
	for (i = 0; i < 24; i++) {
	    for (j = 0; j < 5; j++) {
		pmc4359_p->callout_fvt->wr_frm_reg(frm_base_addr, 0xF2, 
					 ((i<<3) | j), pmc4359_p->bus_width);
		val = pmc4359_p->callout_fvt->rd_frm_reg(frm_base_addr, 0xF3,
						       pmc4359_p->bus_width);

		printf("%#.2x ", val & 0x7f);
	    }
	    printf("\n");
	}
	if (num_ports == 2)
	    frmr_port = 3;
	else
	    frmr_port++;
    }

    return(PASSED);
}


/*****************************************************************
 *
 * Function: framer_peek_reg()
 *
 * Description: This function performs a read to NPU memory-mapped
 *              framer registers.  
 *
 * Input: None
 *
 * Output: always return PASSED
 *
 *****************************************************************/
int 
framer_peek_reg ()
{
   uchar val;
   ushort offset;
   ulong mem_addr, base_addr;
   dev_4359_object_t *pmc4359 = &dev_4359_object;

   offset = gethex_answer("\nEnter Framer register offset[0x0 to 0x7FFF]:",
			  0, 0, 0x7fff);

   base_addr = (ulong)pmc4359->base.dev_addr;

   mem_addr = base_addr + (offset * pmc4359->bus_width);

   val = pmc4359->callout_fvt->rd_frm_reg(base_addr, offset,
					  pmc4359->bus_width);
                                          
   printf("\n register value @%#x = %#x ", mem_addr, val);
   return PASSED;
}

/*****************************************************************
 *
 * Function: framer_poke_reg()
 *
 * Description: This function performs a write to NPU memory-mapped
 *              framer registers.  
 *
 * Input: None
 *
 * Output: always return PASSED
 *
 *****************************************************************/
int 
framer_poke_reg ()
{
   uchar wr_val;
   ushort offset;
   ulong base_addr;
   dev_4359_object_t *pmc4359 = &dev_4359_object;

   offset = gethex_answer("\nEnter Framer register offset[0x0 to 0x7FFF]:",
			  0, 0, 0x7fff);

   wr_val = gethex_answer("\nEnter write value[0x0 to 0xFF]:", 0, 0, 0xff);

   base_addr = (ulong)pmc4359->base.dev_addr;

   pmc4359->callout_fvt->wr_frm_reg(base_addr, offset, wr_val,
				    pmc4359->bus_width);
                                          
   return PASSED;
}


/**********************************************************************
 *
 * Function: init_framer_for_lpbk()
 *
 * The NPU will initialize the framer with the specified op_mode and lpbk_mode
 *
 * Input : port number
 *	   op_mode - CMQ_MODE_T1, CMQ_MODE_E1
 *	   lpbk_mode - FRMR_DIG_LPBK_SLAVE, FRMR_DIG_LPBK_MASTER,
 *                     FRMR_EXT_LPBK_SLAVE, FRMR_EXT_LPBK_MASTER, 
 *                     FRMR_ALOOP_LPBK_MASTER, FRMR_ALOOP
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
init_framer_for_lpbk (int port_num, int op_mode, frmr_lpbk_mode lpbk_mode)
{
    int retval, loop_optn;
    dev_4359_object_t *pmc4359_p;
    unsigned long frmr_bar;
    uchar rdval = 0;

    fpga_reset_framer();
    usleep(5000);
    fpga_unreset_framer();
    usleep(5000);

    /* initialize framer for requested loopback mode */
    pmc4359_p = (dev_4359_object_t *)&dev_4359_object;

    pmc4359_p->callin_fvt->set_cfg_info((dev_object_t *)pmc4359_p,
                                        port_num, op_mode);

    retval = pmc4359_p->base.dev_object_fvt->dev_init((dev_object_t *)pmc4359_p);
    if (retval == FAILED) {
        cterr('f', 0, "Framer initialization failed");
    } else {
	if ((lpbk_mode == FRMR_DIG_LPBK_SLAVE) 
	    || (lpbk_mode == FRMR_DIG_LPBK_MASTER))
	    loop_optn = CMQ_MST_DIAG_DIG_LPBCK;	/* framer digital loopback */
        else if (lpbk_mode == FRMR_ALOOP)
            loop_optn = CMQ_MST_DIAG_ALOOP;     /* Y-cable testing */
	else
	    loop_optn = CMQ_MST_DIAG_LP_NONE;	/* external loopback */

	if (lpbk_mode == FRMR_ALOOP_LPBK_MASTER) {
	    frmr_bar = get_framer_base() + (port_num * CMQ_FRM_OFFSET);
	    pmc4359_p->callout_fvt->wr_frm_reg(frmr_bar, 0x0a, 0x40,
					 pmc4359_p->bus_width);
	} else {
	    pmc4359_p->callin_fvt->set_loopback((dev_object_t *)pmc4359_p,
						port_num, loop_optn);
	}

	/*
	 * if digital lpbk mode, tri-state the liu transceiver
	 * otherwise, take framer transceivers out of high impedence state
	 */
	if ((lpbk_mode == FRMR_DIG_LPBK_SLAVE) 
	    || (lpbk_mode == FRMR_DIG_LPBK_MASTER))
	    fpga_set_framer_txhiz(TRUE);
	else 
	    fpga_set_framer_txhiz(FALSE);
	
	if ((lpbk_mode == FRMR_EXT_LPBK_MASTER) 
	    || (lpbk_mode == FRMR_DIG_LPBK_MASTER)
	    || (lpbk_mode == FRMR_ALOOP_LPBK_MASTER)
	    || (lpbk_mode == FRMR_ALOOP)) {
	    set_pmc_clk_mode(port_num, op_mode, CLK_MASTER);
	} else {
	    /* for all the other loopback tests, configure the framer
	       in clock slave mode. */
	    set_pmc_clk_mode(port_num, op_mode, CLK_SLAVE);
	}

	/* Allow framer to get into in-frame status
	 * must wait here since only now is the loop complete for the framer
	 * to send/rcv (after setup of loopback mode and transceivers)
	 */ 
	usleep(10000);
#ifdef DEBUG
	frmr_bar = get_framer_base() + (port_num * CMQ_FRM_OFFSET);
	rdval = pmc4359_p->callout_fvt->rd_frm_reg(frmr_bar, 0x0, 
					   pmc4359_p->bus_width);
	printf("\nframer register @ %#x = %#x\n", frmr_bar+0x00, rdval);
	rdval = pmc4359_p->callout_fvt->rd_frm_reg(frmr_bar, 0x02, 
					   pmc4359_p->bus_width);
	printf("framer register @ %#x = %#x\n", frmr_bar+0x02, rdval);
	rdval = pmc4359_p->callout_fvt->rd_frm_reg(frmr_bar, 0x30, 
					   pmc4359_p->bus_width);
	printf("framer register @ %#x = %#x\n", frmr_bar+0x30, rdval);
	rdval = pmc4359_p->callout_fvt->rd_frm_reg(frmr_bar, 0x31, 
					   pmc4359_p->bus_width);
	printf("framer register @ %#x = %#x\n", frmr_bar+0x31, rdval);
	rdval = pmc4359_p->callout_fvt->rd_frm_reg(frmr_bar, 0x40, 
					   pmc4359_p->bus_width);
	printf("framer register @ %#x = %#x\n", frmr_bar+0x40, rdval);
	rdval = pmc4359_p->callout_fvt->rd_frm_reg(frmr_bar, 0x41, 
					   pmc4359_p->bus_width);
	printf("framer register @ %#x = %#x\n", frmr_bar+0x41, rdval);
	rdval = pmc4359_p->callout_fvt->rd_frm_reg(frmr_bar, 0x34, 
					   pmc4359_p->bus_width);
	printf("framer register @ %#x = %#x\n", frmr_bar+0x34, rdval);
	rdval = pmc4359_p->callout_fvt->rd_frm_reg(frmr_bar, 0x44, 
					   pmc4359_p->bus_width);
	printf("framer register @ %#x = %#x\n", frmr_bar+0x44, rdval);
	rdval = pmc4359_p->callout_fvt->rd_frm_reg(frmr_bar, 0xbb, 
					   pmc4359_p->bus_width);
	printf("framer register @ %#x = %#x\n", frmr_bar+0xbb, rdval);
	rdval = pmc4359_p->callout_fvt->rd_frm_reg(frmr_bar, 0xd6, 
					   pmc4359_p->bus_width);
	printf("framer register @ %#x = %#x\n", frmr_bar+0xd6, rdval);
#endif
    }
    return(retval);
}

/**********************************************************************
 *
 * Function: check_framer_alignment()
 *
 * The function will check the framer in frame alignment or not.
 *
 * Input : port number
 *	   op_mode - MODE_T1, MODE_E1
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int 
check_framer_alignment (int port_num, int op_mode)
{
    volatile int rdval;
    volatile uint *mem_addr;
    int cnt;

    mem_addr = (volatile uint *)(get_framer_base() + 
				 (port_num * CMQ_FRM_OFFSET));

    /* wait for framer to get into frame alignment */
    if (op_mode == CMQ_MODE_E1) {
	mem_addr += CMQ_E1_FRMR_STAT;
	/* if bit 6 = 0, then in frame, if not wait for a while */
	if (*mem_addr & CMQ_E1_FRM_STAT_OOF_STATE) {
	    for (cnt = 100; cnt > 0; cnt--) {
		msleep(1);
		if (!(*mem_addr & CMQ_E1_FRM_STAT_OOF_STATE)) {
		    break;
		}
	    }
	    if (!cnt) {
		cterr('f',0,"\n Framer%d lost frame alignment, "
		      "E1-FRM status reg @%#x = %#x\n",
		      port_num, mem_addr, *mem_addr);
		return (FAILED);
	    }   
	}
    } else {
	mem_addr += CMQ_T1_FRMR_STAT_INT_IND;	
	/* if bit 0 = 1, then in frame, if not wait for a while */
	if (!(*mem_addr & CMQ_T1_FRMR_STAT_IN_FRM_STATE)) {
	    for (cnt = 100; cnt > 0; cnt--) {
		msleep(5);
		if (*mem_addr & CMQ_T1_FRMR_STAT_IN_FRM_STATE) {
		    break;
		}
	    }
	    if (!cnt) {
		cterr('f',0,"Framer%d out of frame event, "
		      "T1-FRM status reg @%#x = %#x\n",
		      port_num, mem_addr, *mem_addr);
		return (FAILED);
	    }   
	}		
    }
    /*
     * clear PMON counters (0x58 - 0x5f) by writing to frame
     * register 0xnD with any value
     */
    mem_addr = (volatile uint *)(get_framer_base() + 
				 (port_num * CMQ_FRM_OFFSET));
    mem_addr += 0xD;
    *mem_addr = 0x10;
    /* dummy read */
    rdval = *mem_addr;

    return PASSED;
}


/**********************************************************************
 *
 * Function: set_pmc_clk_mode()
 *
 * The function will configure the framer in clock master or slave mode.
 *
 * Input : port number
 *	   op_mode - CMQ_MODE_T1, CMQ_MODE_E1
 *         clk_mode - CLK_MASTER, CLK_SLAVE
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int 
set_pmc_clk_mode (int port_num, int op_mode, frmr_clk_mode clk_mode)
{
    unsigned long frmr_bar, chip_bar;
    uchar wr_val, rdval;
    dev_4359_object_t *pmc4359 = &dev_4359_object;

    chip_bar = get_framer_base();
    frmr_bar = get_framer_base() + (port_num * CMQ_FRM_OFFSET);

    if (op_mode == CMQ_MODE_E1) {
	pmc4359->callout_fvt->wr_frm_reg(frmr_bar, 0x1a, 0xff,
					 pmc4359->bus_width);
	pmc4359->callout_fvt->wr_frm_reg(frmr_bar, 0xbe, 0x05,
					 pmc4359->bus_width);
	pmc4359->callout_fvt->wr_frm_reg(frmr_bar, 0xfa, 0x2d,
					 pmc4359->bus_width);
	pmc4359->callout_fvt->wr_frm_reg(frmr_bar, 0xfb, 0x2d,
					 pmc4359->bus_width);
	pmc4359->callout_fvt->wr_frm_reg(chip_bar, 0xd6, 0x0,
					 pmc4359->bus_width);
    } else {
	pmc4359->callout_fvt->wr_frm_reg(frmr_bar, 0x34, 0x08,
					 pmc4359->bus_width);
	pmc4359->callout_fvt->wr_frm_reg(frmr_bar, 0x44, 0x08,
					 pmc4359->bus_width);

	pmc4359->callout_fvt->wr_frm_reg(frmr_bar, 0x1a, 0xc0,
					 pmc4359->bus_width);
	pmc4359->callout_fvt->wr_frm_reg(frmr_bar, 0xfa, 0x20,
					 pmc4359->bus_width);
	pmc4359->callout_fvt->wr_frm_reg(frmr_bar, 0xfb, 0x20,
					 pmc4359->bus_width);
    }
    pmc4359->callout_fvt->wr_frm_reg(frmr_bar, 0x10, 0x0,
				     pmc4359->bus_width);
    pmc4359->callout_fvt->wr_frm_reg(frmr_bar, 0xf9, 0xcc,
				     pmc4359->bus_width);

    if (clk_mode == CLK_MASTER) {
	/* if the framer is configured in clock-master mode(where 
	   the framer drives the clock and frame sync), the data path
	   is from the NPU to the framer. TDMSW64 will be bypassed. */
	pmc4359->callout_fvt->wr_frm_reg(frmr_bar, 0x06, 0x0d,
					 pmc4359->bus_width);
	pmc4359->callout_fvt->wr_frm_reg(frmr_bar, 0x19, 0xff,
					 pmc4359->bus_width);

	if (op_mode == CMQ_MODE_E1) {
	    pmc4359->callout_fvt->wr_frm_reg(frmr_bar, 0x02, 0x30,
					     pmc4359->bus_width);
	    pmc4359->callout_fvt->wr_frm_reg(frmr_bar, 0x34, 0x8,
					     pmc4359->bus_width);
	    pmc4359->callout_fvt->wr_frm_reg(frmr_bar, 0x44, 0x8,
					     pmc4359->bus_width);
	    pmc4359->callout_fvt->wr_frm_reg(frmr_bar, 0x80, 0x10,
					     pmc4359->bus_width);
	    pmc4359->callout_fvt->wr_frm_reg(frmr_bar, 0x90, 0xc2,
					     pmc4359->bus_width);
	}
	wr_val = CMQ_BRIF_CFG_CLK_EDGE_HI_FRM;
	if (op_mode == CMQ_MODE_E1) {
	    wr_val |= CMQ_BRIF_CFG_CLK_RATE_SEL_2048;
	}
 	pmc4359->callout_fvt->wr_frm_reg(frmr_bar, CMQ_BRIF_CFG, wr_val,
					 pmc4359->bus_width);

	wr_val = CMQ_BRIF_FRM_PULSE_EN_ALT_FDL;
	pmc4359->callout_fvt->wr_frm_reg(frmr_bar, CMQ_BRIF_FRM_PULSE_CFG, 
					 wr_val, pmc4359->bus_width);

	wr_val = CMQ_BTIF_CFG_CLK_EDGE_HI_FRM;
	if (op_mode == CMQ_MODE_E1) {
	    wr_val |= CMQ_BTIF_CFG_CLK_RATE_SEL_2048;
	}
	pmc4359->callout_fvt->wr_frm_reg(frmr_bar, CMQ_BTIF_CFG, 
					 wr_val, pmc4359->bus_width);

	wr_val = CMQ_BTIF_FRM_PULSE_CFG_24_TSLOTS;
	pmc4359->callout_fvt->wr_frm_reg(frmr_bar, CMQ_BTIF_FRM_PULSE_CFG, 
					 wr_val, pmc4359->bus_width);
    } else {
	/* In framer clock slave mode(where the FPGA drives the clock 
	   and framer sync), the data path is from the NPU to the FPGA
	   to the framer. */
	pmc4359->callout_fvt->wr_frm_reg(chip_bar, 0x0, 0x91,
					 pmc4359->bus_width);
	pmc4359->callout_fvt->wr_frm_reg(frmr_bar, 0x02, 0x08,
					 pmc4359->bus_width);
	pmc4359->callout_fvt->wr_frm_reg(frmr_bar, 0x03, 0x84,
					 pmc4359->bus_width);
	pmc4359->callout_fvt->wr_frm_reg(frmr_bar, 0x06, 0x0c,
					 pmc4359->bus_width);
	pmc4359->callout_fvt->wr_frm_reg(frmr_bar, 0x19, 0x0,
					 pmc4359->bus_width);
	pmc4359->callout_fvt->wr_frm_reg(frmr_bar, 0x34, 0x0,
					 pmc4359->bus_width);
	pmc4359->callout_fvt->wr_frm_reg(frmr_bar, 0x44, 0x0,
					 pmc4359->bus_width);
	pmc4359->callout_fvt->wr_frm_reg(frmr_bar, 0x80, 0x10,
					 pmc4359->bus_width);
	pmc4359->callout_fvt->wr_frm_reg(frmr_bar, 0x90, 0xe6,
					 pmc4359->bus_width);

	wr_val = CMQ_BRIF_CFG_CLK_SLAVE | CMQ_BRIF_CFG_CLK_EDGE_HI_DATA;
	wr_val |= CMQ_BRIF_CFG_CLK_RATE_SEL_2048;
	pmc4359->callout_fvt->wr_frm_reg(frmr_bar, CMQ_BRIF_CFG, wr_val,
					 pmc4359->bus_width);

	wr_val = CMQ_BRIF_FRM_PULSE_SLAVE;
	pmc4359->callout_fvt->wr_frm_reg(frmr_bar, CMQ_BRIF_FRM_PULSE_CFG, 
					 wr_val, pmc4359->bus_width);

	wr_val = CMQ_BTIF_CFG_CLK_SLAVE | CMQ_BTIF_CFG_CLK_RATE_SEL_2048;
	pmc4359->callout_fvt->wr_frm_reg(frmr_bar, CMQ_BTIF_CFG, 
					 wr_val, pmc4359->bus_width);

	wr_val = CMQ_BTIF_FRM_PULSE_SLAVE;
	pmc4359->callout_fvt->wr_frm_reg(frmr_bar, CMQ_BTIF_FRM_PULSE_CFG, 
				 wr_val, pmc4359->bus_width);
    }	
}

/******** History ********
$Log: fortitude_framer.c,v $
Revision 1.8  2013/04/19 18:33:37  ywen
update NPU and framer setting to work with the timing change in the new FPGA.

Revision 1.7  2012/08/29 22:45:45  ywen
Add framer analog loopback test utility.

Revision 1.6  2012/08/22 18:10:01  ywen
Add utility to display pulse waveform for debug.

Revision 1.5  2012/06/13 17:54:34  ywen
Add support for TDMSW16 and 2 port SKU.

Revision 1.4  2012/05/14 23:21:10  ywen
Code cleanup and add debug information if test fails.

Revision 1.3  2012/04/12 23:22:38  ywen
code update to make framer digital loopback and external loopback work in clock slave mode.

Revision 1.2  2012/03/28 00:38:17  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:06  ptong
Initial archive of ng_diag module


$Endlog$
*/
