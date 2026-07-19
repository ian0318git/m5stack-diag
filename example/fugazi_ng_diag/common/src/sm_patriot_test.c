/* $Id: sm_patriot_test.c,v 1.23 2018/05/18 09:24:48 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/sm_patriot_test.c,v $
 *******************************************************************************
 * File Name: sm_patriot_test.c
 *
 * Description: Patriot test source file
 *
 *      
 * Author: Huan Ngo
 * Copyright (c)2013-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *******************************************************************************
 */
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "endians.h"
#include "types.h"
#include "defs.h"
#include "signals.h"
#include "error.h"
#include "nvmonvars.h"
#include "console.h"
#include "nvsysvars.h"
#include "pm_utils.h"
#include "pci.h"
#include "slot.h"
#include "mon_plat_defs.h"
#include "common.h"
#include "common_utils.h"
#include "cookie_4.h"
#include "proto.h"
#include "router_if.h"
#include "sm_patriot.h"
#include "i2c_api.h"
#include "plat_defs.h"
#include "pca.h"
#include "linux_api.h"
#include "sgmii_defs.h"
#include "bcm_gesw_defs.h"
#include "dash_fpga.h"
extern n2g_i2c_if_t pca_i2c[];
uchar cmd_param[4];

/**********************************************************************
 *
 * Function: patriot_fpga_reg_test
 * This function test FPGA registers
 *
 * Input : iface - Patriot data structure info pointer
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_fpga_reg_test(patriot_ds_t *iface)
{
    int retval = PASSED;
    
    prpass(testpass, "FPGA Register test");

    if (iface->fpga_downloaded[iface->slot] != TRUE) {
	if (patriot_fpga_download(iface)) {
	    return (FAILED);
	}
    }    
    
    if (patriot_setup_ge_env(iface) == FAILED) {
        return (FAILED);
    }

    patriot_clear_rx_buf();
    
    if (patriot_send_cmd(iface, FROM_HOST_FPGA_REG_TEST, 0)) {
	retval = FAILED;
    }

    if (retval == PASSED) {
	if (patriot_rcv_cmd_result_packet(iface, FROM_HOST_FPGA_REG_TEST)) {
	    retval = FAILED;
	}
    }

    if (patriot_cleanup_ge_env(iface) == FAILED) {
	retval = FAILED;
    }
    
    return(retval);
    
}

/**********************************************************************
 *
 * Function: patriot_cpu_alive_test
 * This function test to make sure the CPU is alive and can response
 *
 * Input : iface - Patriot data structure info pointer
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_cpu_alive_test(patriot_ds_t *iface)
{

    int retval = PASSED;
    
    prpass(testpass, "CPU Alive test");

    if (patriot_setup_ge_env(iface) == FAILED) {
	return (FAILED);
    }
	
    patriot_clear_rx_buf();
	
    if (patriot_send_cmd(iface, FROM_HOST_CPU_ALIVE_TEST, 0)) {
	retval = FAILED;
    }

    if (patriot_cleanup_ge_env(iface) == FAILED) {
	retval = FAILED;
    }

    return retval;
    
}



/**********************************************************************
 *
 * Function: patriot_memory_test
 * This function test memory on Patriot SM
 *
 * Input : iface - Patriot data structure info pointer
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_memory_test(patriot_ds_t *iface)
{

    int retval = PASSED;
    prpass(testpass, "Memory test");


    if (patriot_setup_ge_env(iface) == FAILED) {
        return (FAILED);
    }

    patriot_clear_rx_buf();
    
    if (patriot_send_cmd(iface, FROM_HOST_MEMORY_TEST, 0)) {
	retval = FAILED;
    }

    if (retval == PASSED) {
	if (patriot_rcv_cmd_result_packet(iface, FROM_HOST_MEMORY_TEST)) {
	    retval = FAILED;
	}
    }
	
    if (patriot_cleanup_ge_env(iface) == FAILED) {
	retval = FAILED;
    }
    
    return(retval);
    
}

/**********************************************************************
 *
 * Function: patriot_spi_prom_test_wrap
 * This function test SPI PROM on Patriot SM
 *
 * Input : iface - Patriot data structure info pointer
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_spi_prom_test_wrap(patriot_ds_t *iface)
{

    if (patriot_spi_prom_test(iface, 0)) {
	return (FAILED);
    }
    
    return (PASSED);
}
    
/**********************************************************************
 *
 * Function: patriot_spi_prom_test
 * This function test SPI PROM on Patriot SM
 *
 * Input : iface - Patriot data structure info pointer
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_spi_prom_test(patriot_ds_t *iface, int spi_num)
{
    int retval = PASSED;

    prpass(testpass, "SPI PROM test");        
    
    if (patriot_setup_ge_env(iface) == FAILED) {
        return (FAILED);
    }

    patriot_clear_rx_buf();

    if (patriot_send_cmd(iface, FROM_HOST_SPI_PROM_TEST, spi_num)) {
	retval = FAILED;
    }

    if (retval == PASSED) {
	if (patriot_rcv_cmd_result_packet(iface, FROM_HOST_SPI_PROM_TEST)) {
	    return (FAILED);
	}
    }

    if (patriot_cleanup_ge_env(iface) == FAILED) {
	retval = FAILED;
    }
    
    return(retval);
    
}



/**********************************************************************
 *
 * Function: patriot_ds3170_reg_test
 * This function test Maxim DS3170 registers
 *
 * Input : iface - Patriot data structure info pointer
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_ds3170_reg_test(patriot_ds_t *iface)
{
    int retval = PASSED;

    prpass(testpass, "DS3170 Register test");

    if (patriot_setup_ge_env(iface) == FAILED) {
        return (FAILED);
    }

    patriot_clear_rx_buf();        
    
    if (patriot_send_cmd(iface, FROM_HOST_DS3170_REG_TEST, 0)) {
	retval = FAILED;
    }

    if (retval == PASSED) {
	if (patriot_rcv_cmd_result_packet(iface, FROM_HOST_DS3170_REG_TEST)) {
	    retval = FAILED;
	}
    }

    if (patriot_cleanup_ge_env(iface) == FAILED) {
	retval = FAILED;
    }
    
    return(retval);
    
}



/**********************************************************************
 *
 * Function: patriot_clear_e3_ais_test
 *
 * This function tests E3 AIS thru the LIU.
 *
 * Input : iface - Patriot data structure info pointer
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_clear_e3_ais_test(patriot_ds_t *iface)
{
    int retval = PASSED;
    
    prpass(testpass, "Clear E3 AIS test");

    printf("\n!!!!!!THIS TEST REQUIRES EXTERNAL LOOPBACK CABLE!!!!!!\n");

    if (iface->fpga_downloaded[iface->slot] != TRUE) {
	if (patriot_fpga_download(iface)) {
	    return (FAILED);
	}
    }    
    
    if (patriot_setup_ge_env(iface) == FAILED) {
        return (FAILED);
    }

    patriot_clear_rx_buf();        
    
    if (patriot_send_cmd(iface, FROM_HOST_E3_AIS_TEST, 0)) {
	retval = FAILED;
    }

    if (retval == PASSED) {
	if (patriot_rcv_cmd_result_packet(iface, FROM_HOST_E3_AIS_TEST)) {
	    retval = FAILED;
	}
    }

    if (patriot_cleanup_ge_env(iface) == FAILED) {
	retval = FAILED;
    }
    
    return(retval);
    
}

/**********************************************************************
 *
 * Function: patriot_framer_intr_test
 *
 * This function tests Framer interrupt
 *
 * Input : iface - Patriot data structure info pointer
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_framer_intr_test(patriot_ds_t *iface)
{
    int retval = PASSED;

    prpass(testpass, "Framer Interrupt test");

    if (patriot_setup_ge_env(iface) == FAILED) {
        return (FAILED);
    }

    patriot_clear_rx_buf();        
    
    if (patriot_send_cmd(iface, FROM_HOST_CLR_T3_INTR_TEST, 0)) {
	retval = FAILED;
    }

    if (retval == PASSED) {
	if (patriot_rcv_cmd_result_packet(iface, FROM_HOST_CLR_T3_INTR_TEST)) {
	    retval = FAILED;
	}
    }

    if (patriot_cleanup_ge_env(iface) == FAILED) {
	retval = FAILED;
    }
    
    return(retval);
    
}


/**********************************************************************
 *
 * Function: patriot_clear_t3_bert_test
 *
 * This function tests the Clear T3 Bert. The pattern is generated in
 * the DS3112 framer and looped back at the LIU to the framer.
 * THIS TEST REQUIRES A CABLE TO BE ATTACHED TO WORK CORRECTLY. 
 *
 * Input : iface - Patriot data structure info pointer
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_clear_t3_bert_test(patriot_ds_t *iface)
{
    int retval = PASSED;
    
    prpass(testpass, "Clear T3 BERT test");

    printf("\n!!!!!!THIS TEST REQUIRES EXTERNAL LOOPBACK CABLE!!!!!!\n");

    if (iface->fpga_downloaded[iface->slot] != TRUE) {
	if (patriot_fpga_download(iface)) {
	    return (FAILED);
	}
    }
    
    if (patriot_setup_ge_env(iface) == FAILED) {
        return (FAILED);
    }

    patriot_clear_rx_buf();        
    
    if (patriot_send_cmd(iface, FROM_HOST_CLR_T3_BERT_TEST, 0)) {
	retval = FAILED;
    }

    if (retval == PASSED) {
	if (patriot_rcv_cmd_result_packet(iface, FROM_HOST_CLR_T3_BERT_TEST)) {
	    retval = FAILED;
	}
    }

    if (patriot_cleanup_ge_env(iface) == FAILED) {
	retval = FAILED;
    }
    
    return(retval);

}


/**********************************************************************
 *
 * Function: patriot_fs_lpbk_test
 *
 * This function tests the Clear T3 Internal loopbacks with interrupt
 * enabled.
 *
 * Input : iface - Patriot data structure info pointer
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_fs_lpbk_test(patriot_ds_t *iface)
{

    int ret_val = PASSED;
    
    prpass(testpass, "Patriot Freescale Loopback Test");

    if (patriot_setup_ge_env(iface) == FAILED) {
        return (FAILED);
    }

    patriot_clear_rx_buf();
    
    
    if (patriot_send_cmd(iface, FROM_HOST_FREESCALE_LPBK_TEST, 0)) {
	ret_val = FAILED;
    }

    if (ret_val == PASSED) {
	if (patriot_send_test_data_packets(iface, PATRIOT_CPU_LPBK)) {
	    ret_val = FAILED;
	}
    }

    if (ret_val == PASSED) {
	if (patriot_rcv_cmd_result_packet(iface,
					  FROM_HOST_FREESCALE_LPBK_TEST)) {
	    ret_val = FAILED;
	}
    }

    if (patriot_cleanup_ge_env(iface) == FAILED) {
        ret_val = FAILED;
    }
    
    return ret_val;
}


/**********************************************************************
 *
 * Function: patriot_fs_ucc_lpbk_test
 *
 * This function tests UCC internal loopback on the Freescale CPU
 *
 * Input : iface - Patriot data structure info pointer
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_fs_ucc_lpbk_test(patriot_ds_t *iface)
{
    int ret_val = PASSED;

    prpass(testpass, "Patriot Freescale UCC Loopback Test");

    if (patriot_setup_ge_env(iface) == FAILED) {
        return (FAILED);
    }

    patriot_clear_rx_buf();

    
    if (patriot_send_cmd(iface, FROM_HOST_FREESCALE_UCC_LPBK_TEST, 0)) {
	ret_val = FAILED;
    }

    if (ret_val == PASSED) {
	if (patriot_rcv_cmd_result_packet(iface,
					  FROM_HOST_FREESCALE_UCC_LPBK_TEST)) {
	    ret_val = FAILED;
	}
    }
    
    if (patriot_cleanup_ge_env(iface) == FAILED) {
        ret_val = FAILED;
    }
    
    return(ret_val);
}


/**********************************************************************
 *
 * Function: patriot_fpga_lpbk_test
 *
 * This function tests FPGA loopback
 *
 * Input : iface - Patriot data structure info pointer
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_fpga_lpbk_test(patriot_ds_t *iface)
{
    int ret_val = PASSED;

    prpass(testpass, "Patriot FPGA Loopback Test");

    if (iface->fpga_downloaded[iface->slot] != TRUE) {
	if (patriot_fpga_download(iface)) {
	    return (FAILED);
	}
    }
    
    if (patriot_setup_ge_env(iface) == FAILED) {
        return (FAILED);
    }

    patriot_clear_rx_buf();

    
    if (patriot_send_cmd(iface, FROM_HOST_FPGA_LPBK_TEST, 0)) {
	ret_val = FAILED;
    }

    if (ret_val == PASSED) {
	if (patriot_rcv_cmd_result_packet(iface,
					  FROM_HOST_FPGA_LPBK_TEST)) {
	    ret_val = FAILED;
	}
    }
    
    if (patriot_cleanup_ge_env(iface) == FAILED) {
        ret_val = FAILED;
    }
    
    return(ret_val);
}



/**********************************************************************
 *
 * Function: patriot_clear_t3_lpbk_test
 *
 * This function is a wrapper for Clear T3 loopbacks.
 *
 * Input : iface - Patriot data structure info pointer
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_clear_t3_lpbk_test(patriot_ds_t *iface)
{

    int ret_val = PASSED;
    prpass(testpass, "Clear T3 Loopback test");

    if (iface->fpga_downloaded[iface->slot] != TRUE) {
	if (patriot_fpga_download(iface)) {
	    return (FAILED);
	}
    }
    
    if (patriot_setup_ge_env(iface) == FAILED) {
        return (FAILED);
    }

    patriot_clear_rx_buf();
    
    
    if (patriot_send_cmd(iface, FROM_HOST_CLR_T3_LPBK_TEST, 0)) {
	ret_val = FAILED;
    }

    if (ret_val == PASSED) {
	if (patriot_rcv_cmd_result_packet(iface, FROM_HOST_CLR_T3_LPBK_TEST)) {
	    ret_val = FAILED;
	}
    }
	
    if (!((NVRAM)->diagflag & D_EXT_LOOPBACK)) {
	
	if (patriot_send_cmd(iface, FROM_HOST_CLR_T3_EX_LPBK_TEST, 0)) {
	    ret_val = FAILED;
	}
        if (ret_val == PASSED) {
	    if (patriot_rcv_cmd_result_packet(iface,
					      FROM_HOST_CLR_T3_EX_LPBK_TEST)) {
	        ret_val = FAILED;
	    }
	}
    }

    if (patriot_cleanup_ge_env(iface) == FAILED) {
        ret_val = FAILED;
    }
    
    return ret_val;
}


/**********************************************************************
 *
 * Function: patriot_subrate_t3_lpbk_test
 *
 * This function performs T3 subrates loopback.
 *
 * Input : iface - Patriot data structure info pointer
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_subrate_t3_lpbk_test(patriot_ds_t *iface)
{
    int ret_val = PASSED;
    prpass(testpass, "Subrate T3 Loopback test");

    if (iface->fpga_downloaded[iface->slot] != TRUE) {
	if (patriot_fpga_download(iface)) {
	    return (FAILED);
	}
    }
    
    if (patriot_setup_ge_env(iface) == FAILED) {
        return (FAILED);
    }

    patriot_clear_rx_buf();    
    
    if (patriot_send_cmd(iface, FROM_HOST_SUB_T3_LPBK_TEST, 0)) {
    	ret_val = FAILED;
    }

    if (ret_val == PASSED) {
	if (patriot_rcv_cmd_result_packet(iface,
					  FROM_HOST_SUB_T3_LPBK_TEST)) {
	    ret_val = FAILED;
	}
    }
	
    if (!((NVRAM)->diagflag & D_EXT_LOOPBACK)) {
	if (patriot_send_cmd(iface, FROM_HOST_SUB_T3_EX_LPBK_TEST, 0)) {
	    ret_val = FAILED;
	}
	
	if (patriot_rcv_cmd_result_packet(iface,
					  FROM_HOST_SUB_T3_EX_LPBK_TEST)) {
	    ret_val = FAILED;
	}
    }
    
    if (patriot_cleanup_ge_env(iface) == FAILED) {
        ret_val = FAILED;
    }
    
    return ret_val;

}

/**********************************************************************
 *
 * Function: patriot_subrate_t3_ind_lpbk_test
 *
 * This function performs T3 subrates individual loopback.
 *
 * Input : iface - Patriot data structure info pointer
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_subrate_t3_ind_lpbk_test(patriot_ds_t *iface)
{
    int ret_val = PASSED;
    uchar i = 0;
    uchar sub_cmd[2] = {0x00, 0x00};

    prpass(testpass, "Subrate T3 individual Loopback test");
    
    if (iface->fpga_downloaded[iface->slot] != TRUE) {
	if (patriot_fpga_download(iface)) {
	    return (FAILED);
	}
    }
    
    /* Select Vendor */
    printf("\n");
    i = gethex_answer("type, 0-Kentrox, 1-Digital Link, 2-Larscom,"
                          "      3-Adtran, 4-Verilink", 0, 0, 0x4);

    sub_cmd[0] = i;

    /* Select Rate */
    switch (i) {
    case 0x0:
    	printf("\n Kentrox ");
        printf("\n   1-1.5K, 2-2K, ..., 11-9.5K\n"
               "     12-10K, 13-10.5K, 14-11K, ..., 25-19.5K\n"
               "     26-20K, 27-20.5K, 28-21K, ..., 39-29.5K\n"
               "     3A-30K, 3B-30.5K, 3C-31K, ..., 44-35K\n");
        i = gethex_answer("Select T3 rate: ", 0x12, 1, 0x44);
    	break;
    case 0x1:
    	printf("\n Digital Link ");
        printf("\n   0x0 = 300 Bandwidth"
        	   "\n   0x1 = 10000 Bandwidth"
        	   "\n   0x2 = 20000 Bandwidth"
        	   "\n   0x3 = 34010 Bandwidth"
        	   "\n   0x4 = 44210 Bandwidth \n"
               );
        i = gethex_answer("Select T3 for Digital Link rate: ", 0x3, 0, 0x4);
    	break;
    case 0x2:
    	printf("\n Larscom ");
    	printf("\n   0x0 = 3100 Bandwidth"
               "\n   0x1 = 10000 Bandwidth"
               "\n   0x2 = 20000 Bandwidth"
               "\n   0x3 = 34010 Bandwidth"
               "\n   0x4 = 44210 Bandwidth \n"
    	           );
    	    i = gethex_answer("Select T3 for Larscom rate: ", 0x3, 0, 0x4);
    	break;
    case 0x3:
    	printf("\n Adtran ");
        printf("\n   0x0 = 75 Bandwidth"
        	   "\n   0x1 = 10000 Bandwidth"
        	   "\n   0x2 = 20000 Bandwidth"
        	   "\n   0x3 = 34010 Bandwidth"
        	   "\n   0x4 = 44210 Bandwidth \n"
               );
        i = gethex_answer("Select T3 for Adtran rate: ", 0x3, 0, 0x4);
    	break;
    case 0x4:
    	printf("\n Verilink");
        printf("\n   0x0 = 1500 Bandwidth"
        	   "\n   0x1 = 10000 Bandwidth"
        	   "\n   0x2 = 20000 Bandwidth"
        	   "\n   0x3 = 34010 Bandwidth"
        	   "\n   0x4 = 44210 Bandwidth \n"
               );
        i = gethex_answer("Select T3 for Verilink rate: ", 0x3, 0, 0x4);
    	break;
    default:
    	break;
    }

    sub_cmd[1] = i;

    /* combine command */
    cmd_param[0] = PATRIOT_CMD;
    cmd_param[1] = FROM_HOST_SUB_T3_IND_LPBK_TEST;
    cmd_param[2] = sub_cmd[0];  /* Vendor */
    cmd_param[3] = sub_cmd[1];  /* Rate */
#ifdef DEBUG
    for (ix = 0; ix < 4; ix++) {
    	printf("\ncmd_packet_p[%d] = %x", ix, cmd_param[ix]);
    }
#endif
    if (patriot_setup_ge_env(iface) == FAILED) {
        return (FAILED);
    }

    patriot_clear_rx_buf();

    if (patriot_send_cmd(iface, FROM_HOST_SUB_T3_IND_LPBK_TEST, 0)) {
    	ret_val = FAILED;
    }

    if (ret_val == PASSED) {
	if (patriot_rcv_cmd_result_packet(iface,
			FROM_HOST_SUB_T3_IND_LPBK_TEST)) {
	    ret_val = FAILED;
	}
    }

    if (!((NVRAM)->diagflag & D_EXT_LOOPBACK)) {
        /* combine command */
    	cmd_param[0] = PATRIOT_CMD;
    	cmd_param[1] = FROM_HOST_SUB_T3_IND_EX_LPBK_TEST;
    	cmd_param[2] = sub_cmd[0];  /* Vendor */
    	cmd_param[3] = sub_cmd[1];  /* Rate */
#ifdef DEBUG
	for (ix = 0; ix < 4; ix++) {
	    printf("\ncmd_packet_p[%d] = %x", ix, cmd_param[ix]);
	}
#endif
	if (patriot_send_cmd(iface, FROM_HOST_SUB_T3_IND_EX_LPBK_TEST, 0)) {
	    ret_val = FAILED;
	}
	
	if (patriot_rcv_cmd_result_packet(iface,
					  FROM_HOST_SUB_T3_IND_EX_LPBK_TEST)){
	    ret_val = FAILED;
	}
    }

    if (patriot_cleanup_ge_env(iface) == FAILED) {
        ret_val = FAILED;
    }

    return ret_val;

}



/**********************************************************************
 *
 * Function: patriot_clear_e3_lpbk_test
 *
 * This function is a wrapper for Clear E3 loopbacks.
 *
 * Input : iface - Patriot data structure info pointer
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_clear_e3_lpbk_test(patriot_ds_t *iface)
{

    int ret_val = PASSED;
    prpass(testpass, "Clear E3 Loopback test");    

    if (iface->fpga_downloaded[iface->slot] != TRUE) {
	if (patriot_fpga_download(iface)) {
	    return (FAILED);
	}
    }
    
    if (patriot_setup_ge_env(iface) == FAILED) {
        return (FAILED);
    }

    patriot_clear_rx_buf();
    
    if (patriot_send_cmd(iface, FROM_HOST_CLR_E3_LPBK_TEST, 0)) {
	ret_val = FAILED;
    }

    if (ret_val == PASSED) {
	if (patriot_rcv_cmd_result_packet(iface, FROM_HOST_CLR_E3_LPBK_TEST)) {
	    ret_val = FAILED;
	}
    }
	
    if (!((NVRAM)->diagflag & D_EXT_LOOPBACK)) {
	
	if (patriot_send_cmd(iface, FROM_HOST_CLR_E3_EX_LPBK_TEST, 0)) {
	    ret_val = FAILED;
	}
	
	if (ret_val == PASSED) {
	    if (patriot_rcv_cmd_result_packet(iface,
					  FROM_HOST_CLR_E3_EX_LPBK_TEST)) {
                ret_val = FAILED;
	    }
	}
    }

    if (patriot_cleanup_ge_env(iface) == FAILED) {
        ret_val = FAILED;
    }
    
    return ret_val;
}


/**********************************************************************
 *
 * Function: patriot_subrate_e3_lpbk_test
 *
 * This function performs E3 subrates loopback.
 *
 * Input : iface - Patriot data structure info pointer
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_subrate_e3_lpbk_test(patriot_ds_t *iface)
{

    int ret_val = PASSED;

    prpass(testpass, "Subrate E3 Loopback test");

    if (iface->fpga_downloaded[iface->slot] != TRUE) {
	if (patriot_fpga_download(iface)) {
	    return (FAILED);
	}
    }
    
    if (patriot_setup_ge_env(iface) == FAILED) {
        return (FAILED);
    }

    patriot_clear_rx_buf();    
    
    if (patriot_send_cmd(iface, FROM_HOST_SUB_E3_LPBK_TEST, 0)) {
    	ret_val = FAILED;
    }

    if (ret_val == PASSED) {
	if (patriot_rcv_cmd_result_packet(iface,
					  FROM_HOST_SUB_E3_LPBK_TEST)) {
	    ret_val = FAILED;
	}
    }
	
    if (!((NVRAM)->diagflag & D_EXT_LOOPBACK)) {
	if (patriot_send_cmd(iface, FROM_HOST_SUB_E3_EX_LPBK_TEST, 0)) {
	    ret_val = FAILED;
	}
	
	if (patriot_rcv_cmd_result_packet(iface,
					  FROM_HOST_SUB_E3_EX_LPBK_TEST)) {
	    ret_val = FAILED;
	}
    }
    
    if (patriot_cleanup_ge_env(iface) == FAILED) {
        ret_val = FAILED;
    }
    
    return ret_val;

}

/**********************************************************************
 *
 * Function: patriot_subrate_e3_ind_lpbk_test
 *
 * This function performs E3 subrates individual loopback.
 *
 * Input : iface - Patriot data structure info pointer
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_subrate_e3_ind_lpbk_test(patriot_ds_t *iface)
{

    int ret_val = PASSED;
    uchar i = 0;
    uchar sub_cmd[2] = {0x00, 0x00};

    prpass(testpass, "Subrate E3 Individual Loopback test");

    if (iface->fpga_downloaded[iface->slot] != TRUE) {
	if (patriot_fpga_download(iface)) {
	    return (FAILED);
	}
    }    
    
    /* Select Vendor */
    printf("\n");
    i = gethex_answer("type, 0-Kentrox, 1-Digital Link, 2-Unframed", 0,
                           0, 0x2);

    sub_cmd[0] = i;

    /* Select Rate */
    switch (i) {
    case 0x0:
    	printf("\n Kentrox ");
        printf("\n    0-1K, 1-1.5K, 2-2K, ..., 11-9.5K\n"
               "     12-10K, 13-10.5K, 14-11K, ..., 25-19.5K\n"
               "     26-20K, 27-20.5K, 28-21K, ..., 2F-24.5K\n");
        i = gethex_answer("Select T3 rate: ", 0x12, 0, 0x2F);
    	break;
    case 0x1:
    	printf("\n Digital Link ");
        printf("\n   0x0 = 358 Bandwidth"
        	   "\n   0x1 = 10000 Bandwidth"
        	   "\n   0x2 = 20000 Bandwidth"
        	   "\n   0x3 = 34010 Bandwidth \n"
               );
        i = gethex_answer("Select T3 for Digital Link rate: ", 0x3, 0, 0x3);
    	break;
    case 0x2:
    	printf("\n UNFRAME");
        i = 0;
    	break;
    default:
    	break;
    }

    sub_cmd[1] = i;

    /* combine command */
    cmd_param[0] = PATRIOT_CMD;
    cmd_param[1] = FROM_HOST_SUB_E3_IND_LPBK_TEST;
    cmd_param[2] = sub_cmd[0];  /* Vendor */
    cmd_param[3] = sub_cmd[1];  /* Rate */
#ifdef DEBUG
	for (ix = 0; ix < 4; ix++) {
		printf("\ncmd_packet_p[%d] = %x", ix, cmd_param[ix]);
	}
#endif
    if (patriot_setup_ge_env(iface) == FAILED) {
        return (FAILED);
    }

    patriot_clear_rx_buf();

    if (patriot_send_cmd(iface, FROM_HOST_SUB_E3_IND_LPBK_TEST, 0)) {
    	ret_val = FAILED;
    }

    if (ret_val == PASSED) {
	if (patriot_rcv_cmd_result_packet(iface,
			FROM_HOST_SUB_E3_IND_LPBK_TEST)) {
	    ret_val = FAILED;
	}
    }

    if (!((NVRAM)->diagflag & D_EXT_LOOPBACK)) {
        /* combine command */
    	cmd_param[0] = PATRIOT_CMD;
    	cmd_param[1] = FROM_HOST_SUB_E3_IND_EX_LPBK_TEST;
    	cmd_param[2] = sub_cmd[0];  /* Vendor */
    	cmd_param[3] = sub_cmd[1];  /* Rate */
#ifdef DEBUG
	for (ix = 0; ix < 4; ix++) {
	    printf("\ncmd_packet_p[%d] = %x", ix, cmd_param[ix]);
	}
#endif
	if (patriot_send_cmd(iface, FROM_HOST_SUB_E3_IND_EX_LPBK_TEST, 0)) {
	    ret_val = FAILED;
	}

	if (patriot_rcv_cmd_result_packet(iface,
			FROM_HOST_SUB_E3_IND_EX_LPBK_TEST)) {
	    ret_val = FAILED;
	}
    }

    if (patriot_cleanup_ge_env(iface) == FAILED) {
        ret_val = FAILED;
    }

    return ret_val;

}


/**********************************************************************
 *
 * Function: patriot_test_fpga_gpio_framer
 * This function tests FPGA GPIO and Framer GPIO
 * Notes: A. FPGA test GPIO on Framer
 *           1. Walking's 1 test by write 0x01 to FPGA Framer GPIO OE register (0x3)
 *           2. Walking's 1 test by write 0x01 to FPGA Framer GPIO register (0x2)
 *           3. Read back the Framer GPIO read register (0x1c) to verify value.
 *        B. Framer test GPIO on FPGA
 *           1. Walking's 11 to Framer GPIO control register (0x0A ~ 0x0B)
 *           2. Read back the Framer GPIO read register (0x1c) to verify value.
 *           3. Read back the FPGA Framer GPIO register (0x2) to verify value.
 *
 * Input : iface - Patriot data structure info pointer
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_test_fpga_gpio_framer(patriot_ds_t *iface)
{
    int retval = PASSED;

    prpass(testpass, "FPGA GPIO ping pong FRAMER GPIO");

    if (iface->fpga_downloaded[iface->slot] != TRUE) {
	if (patriot_fpga_download(iface)) {
	    return (FAILED);
	}
    }    
    
    if (patriot_setup_ge_env(iface) == FAILED) {
        return (FAILED);
    }

    patriot_clear_rx_buf();

    if (patriot_send_cmd(iface, FROM_HOST_FPGA_GPIO_FRAMER_GPIO_TEST, 0)) {
	retval = FAILED;
    }

    if (retval == PASSED) {
	if (patriot_rcv_cmd_result_packet(iface,
				       FROM_HOST_FPGA_GPIO_FRAMER_GPIO_TEST)) {
	    retval = FAILED;
	}
    }

    if (patriot_cleanup_ge_env(iface) == FAILED) {
	retval = FAILED;
    }

    return(retval);

}

/**********************************************************************
 *
 * Function: patriot_fpga_intr_test
 * This function test FPGA interrupts
 *
 * Input : iface - Patriot data structure info pointer
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_fpga_intr_test(patriot_ds_t *iface)
{
    int retval = PASSED;
    
    prpass(testpass, "FPGA Interrupt test");

    if (iface->fpga_downloaded[iface->slot] != TRUE) {
	if (patriot_fpga_download(iface)) {
	    return (FAILED);
	}
    }    
    
    if (patriot_setup_ge_env(iface) == FAILED) {
        return (FAILED);
    }

    patriot_clear_rx_buf();
    
    if (patriot_send_cmd(iface, FROM_HOST_FPGA_INTR_TEST, 0)) {
	retval = FAILED;
    }

    if (retval == PASSED) {
	if (patriot_rcv_cmd_result_packet(iface, FROM_HOST_FPGA_INTR_TEST)) {
	    retval = FAILED;
	}
    }

    if (patriot_cleanup_ge_env(iface) == FAILED) {
	retval = FAILED;
    }
    
    return(retval);
    
}


/**********************************************************************
 *
 * Function: patriot_host_to_module_gpio_test
 * This function tests IO port from host to module Pin GPIO
 * Through 6 test part.
 * 1. From Host set 1 to IO1 port then in the module check PA23 if turn 1.
 * 2. From Host set 0 to IO1 port then in the module check PA23 if turn 0
 * 3. From Host set 1 to IO4 port then in the module check PB10 if turn 1.
 * 4. From Host set 0 to IO4 port then in the module check PB10 if turn 0.
 * 5. From Module set 1 to PB04 port then in the Host check IO3 if turn 1
 * 6. From Module set 0 to PB04 port then in the Host check IO3 if turn 0
 *
 * Input : iface - Patriot data structure info pointer
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_host_to_module_gpio_test(patriot_ds_t *iface)
{
    int retval = PASSED;
    int debug = FALSE;
    uchar data = 0, io_port_conf = 0;
    uchar restore_data = 0;
    n2g_i2c_if_t *pca1;
    pca1 = &pca_i2c[1];
    
    prpass(testpass, "Host to Module GPIO");

    if (diagflag_xram & D_DEBUG_OPTIONS) {
    	debug = TRUE;
    }

    /* Reinitialize the IO Port controller */
    if (io_port_8bit_i2c_read(pca1, CONFIGURATION_REG,
			      &io_port_conf, TRUE)) {
	return (FAILED);
    }
    
    /* Set IO 1, 4 to output.  Set IO 3 to input */
    io_port_conf |= BIT3;
    io_port_conf &= ~(BIT1 | BIT4);
    
    if (io_port_8bit_i2c_write(pca1, CONFIGURATION_REG,
			       &io_port_conf)) {
	return (FAILED);
    }
    
    /* Restore Default Value */
    if (io_port_8bit_i2c_read(pca1, OUTPUT_PORT_REG,
			       &data, TRUE)) {
	return (FAILED);
    }
    restore_data = data;

    if (debug) {
        printf("\n%s, [#%d]: Save data contents OUTPUT bit = 0x%02x, \n",
              __FUNCTION__, __LINE__, restore_data);
    }
    /* Initial value set IO for 0x00 */
    data |= 0x00;
    if (io_port_8bit_i2c_write(pca1, OUTPUT_PORT_REG,
				&data)) {
	    return (FAILED);
    }
    msleep(1000);

    if (io_port_8bit_i2c_write(pca1, INPUT_PORT_REG,
    				&data)) {
    	return (FAILED);
    }
    msleep(1000);
    
    if (io_port_8bit_i2c_read(pca1, OUTPUT_PORT_REG,
			      &data, TRUE)) {
	return (FAILED);
    }

    if (debug) {
		printf("\n%s, [#%d]: Before testing 1 for IO 1 contents = 0x%02x\n",
				__FUNCTION__, __LINE__, (data & BIT1));
		printf("\n");
	}
    
    /* Set IO 1 to 0x2, then check the results if the IO 1 pin up */
    data |= BIT1;
    if (io_port_8bit_i2c_write(pca1, OUTPUT_PORT_REG,
			       &data)) {
	return (FAILED);
    }
    msleep(1000);

    if (patriot_setup_ge_env(iface) == FAILED) {
        retval = FAILED;
    }
    
    if (retval == FAILED) {
	return (FAILED);
    }
    
    prpass(testpass, "Test IO 1 Write 1 from host to module read 1");
    patriot_clear_rx_buf();
    
    if (patriot_send_cmd(iface, FROM_HOST_CPU_GPIO_TEST_IO1_W1, 0)) {
	retval = FAILED;
    }
    
    if (retval == PASSED) {
	if (patriot_rcv_cmd_result_packet(iface, FROM_HOST_CPU_GPIO_TEST_IO1_W1)) {
	    retval = FAILED;
	}
    }
    
    if (patriot_cleanup_ge_env(iface) == FAILED) {
	retval = FAILED;
    }
    
    if (retval == FAILED) {
	return (FAILED);
    }
    
    if (debug) {
		if (io_port_8bit_i2c_read(pca1, OUTPUT_PORT_REG,
					   &data, TRUE)) {
		    return (FAILED);
		}
		printf("\n%s, [#%d]: After testing 1 for IO 1 contents = 0x%02x",
				__FUNCTION__, __LINE__, (data & BIT1));
    }
    
    /* After module check the IO 1 is set 1 then set to clear 0 */
    if (io_port_8bit_i2c_read(pca1, OUTPUT_PORT_REG,
			      &data, TRUE)) {
	return (FAILED);
    }
    
    if (debug) {
        printf("\n%s, [#%d]: Before testing 0 for IO 1 contents = 0x%02x\n",
    		__FUNCTION__, __LINE__, (data & BIT1));
        printf("\n");
    }
    
    /* Set IO 1 to ~0x2, then check the results if the IO 1 pin down */
    data &= ~BIT1;
    if (io_port_8bit_i2c_write(pca1, OUTPUT_PORT_REG,
			       &data)) {
	return (FAILED);
    }
    msleep(1000);
    
    prpass(testpass, "Test IO 1 Write 0 from host to module read 0");
    if (patriot_setup_ge_env(iface) == FAILED) {
        return (FAILED);
    }
    patriot_clear_rx_buf();
    
    if (patriot_send_cmd(iface, FROM_HOST_CPU_GPIO_TEST_IO1_W0, 0)) {
	retval = FAILED;
    }
    
    if (retval == PASSED) {
	if (patriot_rcv_cmd_result_packet(iface, FROM_HOST_CPU_GPIO_TEST_IO1_W0)) {
	    retval = FAILED;
	}
    }
    
    if (patriot_cleanup_ge_env(iface) == FAILED) {
	retval = FAILED;
    }

    if (retval == FAILED) {
	return (FAILED);
    }   
    msleep(1000);
    
    if (debug) {
	    if (io_port_8bit_i2c_read(pca1, OUTPUT_PORT_REG,
				   &data, TRUE)) {
	        return (FAILED);
	    }
	    printf("\n%s, [#%d]: After testing 0 for IO 1 contents = 0x%02x",
	    		__FUNCTION__, __LINE__, (data & BIT1));
    }
    
    /* Check the IO 4 value set 0x10 from module */
    if (io_port_8bit_i2c_read(pca1, OUTPUT_PORT_REG,
			       &data, TRUE)) {
	return (FAILED);
    }

    if (debug) {
        printf("\n%s, [#%d]: Before testing 1 for IO 4 contents = 0x%02x\n",
    		__FUNCTION__,__LINE__, (data & BIT4));
        printf("\n");
    }

    /* Set IO 4 to 0x10, then check the results if the IO 4 pin up */
    data |= BIT4;
    if (io_port_8bit_i2c_write(pca1, OUTPUT_PORT_REG,
				&data)) {
	return (FAILED);
    }
    msleep(1000);
    
    if (patriot_setup_ge_env(iface) == FAILED) {
        retval = FAILED;
    }
    
    if (retval == FAILED) {
	return (FAILED);
    }
    
    prpass(testpass, "Test IO 4 Write 1 from host to module read 1");
    patriot_clear_rx_buf();

    if (patriot_send_cmd(iface, FROM_HOST_CPU_GPIO_TEST_IO4_W1, 0)) {
	retval = FAILED;
    }
    
    if (retval == PASSED) {
	if (patriot_rcv_cmd_result_packet(iface, FROM_HOST_CPU_GPIO_TEST_IO4_W1)) {
	    retval = FAILED;
	}
    }

    if (patriot_cleanup_ge_env(iface) == FAILED) {
	retval = FAILED;
    }
    
    if (retval == FAILED) {
	return (FAILED);
    }

    if (debug) {
	    if (io_port_8bit_i2c_read(pca1, OUTPUT_PORT_REG,
				   &data, TRUE)) {
	        return (FAILED);
	    }
	    printf("\n%s, [#%d]: After testing 1 for IO 4 contents = 0x%02x",
	    		__FUNCTION__, __LINE__, (data & BIT4));
    }

    /* After module check the IO 4 is set 1 then set to clear 0 */
    if (io_port_8bit_i2c_read(pca1, OUTPUT_PORT_REG,
			       &data, TRUE)) {
	return (FAILED);
    }

    if (debug) {
        printf("\n%s, [#%d]: Before testing 0 for IO 4 contents = 0x%02x\n",
    		__FUNCTION__, __LINE__, (data & BIT4));
        printf("\n");
    }
    
    /* Set IO 4 to ~0x10, then check the results if the IO 4 pin down */
    data &= ~BIT4;
    if (io_port_8bit_i2c_write(pca1, OUTPUT_PORT_REG,
				&data)) {
	return (FAILED);
    }
    msleep(1000);
    
    prpass(testpass, "Test IO 4 Write 0 from host to module read 0");
    if (patriot_setup_ge_env(iface) == FAILED) {
        return (FAILED);
    }
    patriot_clear_rx_buf();

    if (patriot_send_cmd(iface, FROM_HOST_CPU_GPIO_TEST_IO4_W0, 0)) {
	retval = FAILED;
    }

    if (retval == PASSED) {
	if (patriot_rcv_cmd_result_packet(iface, FROM_HOST_CPU_GPIO_TEST_IO4_W0)) {
	    retval = FAILED;
	}
    }

    if (patriot_cleanup_ge_env(iface) == FAILED) {
	retval = FAILED;
    }

    if (retval == FAILED) {
	return (FAILED);
    }
    msleep(1000);

    if (debug) {
    	if (io_port_8bit_i2c_read(pca1, OUTPUT_PORT_REG,
    				   &data, TRUE)) {
    	    return (FAILED);
    	}
    	printf("\n%s, [#%d]: After testing 0 for IO 4 contents = 0x%02x",
    			__FUNCTION__, __LINE__, (data & BIT4));

        if (io_port_8bit_i2c_read(pca1, INPUT_PORT_REG,
    			       &data, TRUE)) {
    	    return FAILED;
        }
        msleep(1000);

        printf("\n%s, [#%d]: Before testing 1 for IO 3 contents = 0x%02x\n",
        		__FUNCTION__, __LINE__, (data & BIT3));
        printf("\n");
    }
    /* Check the IO 3 value set 0x08 from module */
    prpass(testpass, "Test IO 3 Read 1 from module to host read 1");
    if (patriot_setup_ge_env(iface) == FAILED) {
        return (FAILED);
    }
    patriot_clear_rx_buf();
    
    if (patriot_send_cmd(iface, FROM_HOST_CPU_GPIO_TEST_IO3_R1, 0)) {
	retval = FAILED;
    }
    
    if (retval == PASSED) {
	if (patriot_rcv_cmd_result_packet(iface, FROM_HOST_CPU_GPIO_TEST_IO3_R1)) {
	    retval = FAILED;
	}
    }
    
    if (patriot_cleanup_ge_env(iface) == FAILED) {
	retval = FAILED;
    }

    if (retval == FAILED) {
	return (FAILED);
    }
    
    if (io_port_8bit_i2c_read(pca1, INPUT_PORT_REG,
			      &data, TRUE)) {
	return FAILED;
    }
    msleep(1000);

    if (debug) {
        printf("\n%s, [#%d]: After testing 1 for IO 3 contents = 0x%02x\n",
        		__FUNCTION__, __LINE__, (data & BIT3));
        printf("\n");
    }
    
    if((data & BIT3) != BIT3) {
        printf("\n Failed IO 3 value = 0x%02x, expected = 0x%02x",
        (data & BIT3), BIT3);
	       return FAILED;
    }
    msleep(1000);
    
    /* Check the IO 3 value set ~0x08 from module */
    if (debug) {
    	if (io_port_8bit_i2c_read(pca1, INPUT_PORT_REG,
    				   &data, TRUE)) {
    	return FAILED;
    	}
    	msleep(1000);

    	printf("\n%s, [#%d]: Before testing 0 for IO 3 contents = 0x%02x\n",
    			__FUNCTION__, __LINE__, (data & BIT3));
    	printf("\n");
    }
    
    prpass(testpass, "Test IO 3 Read 0 from module to host read 0");
    if (patriot_setup_ge_env(iface) == FAILED) {
        return (FAILED);
    }
    patriot_clear_rx_buf();

    if (patriot_send_cmd(iface, FROM_HOST_CPU_GPIO_TEST_IO3_R0, 0)) {
	retval = FAILED;
    }
    
    if (retval == PASSED) {
	if (patriot_rcv_cmd_result_packet(iface, FROM_HOST_CPU_GPIO_TEST_IO3_R0)) {
	    retval = FAILED;
	}
    }

    if (patriot_cleanup_ge_env(iface) == FAILED) {
	retval = FAILED;
    }
    
    if (io_port_8bit_i2c_read(pca1, INPUT_PORT_REG,
			      &data, TRUE)) {
	retval = FAILED;
    }
    msleep(1000);
    
    if (debug) {
        printf("\n%s, [#%d]: After testing 0 for IO 3 contents = 0x%02x\n",
        		__FUNCTION__, __LINE__, (data & BIT3));
        printf("\n");
    }


    if((data & BIT3) != 0x00) {
	printf("\n Failed IO 3 value = 0x%02x, expected = 0x%02x",
	       (data & BIT3), 0x00);
	retval = FAILED;
    }

    if (io_port_8bit_i2c_read(pca1, INPUT_PORT_REG,
			      &data, TRUE)) {
	return (FAILED);
    }
#ifdef DEBUG    
    printf("\nINPUT_PORT_REG = 0x%02x", data);
    if (io_port_8bit_i2c_read(pca1, OUTPUT_PORT_REG,
			      &data, TRUE)) {
	return (FAILED);
    }
    printf("\nOUTPUT_PORT_REG = 0x%02x", data);
    if (io_port_8bit_i2c_read(pca1, POLARITY_INVERSION_REG,
			      &data, TRUE)) {
	return (FAILED);
    }
    printf("\nPOLARITY_INVERSION_REG = 0x%02x", data);
    if (io_port_8bit_i2c_read(pca1, CONFIGURATION_REG,
			      &data, TRUE)) {
	return (FAILED);
    }
    printf("\nCONFIGURATION_REG = 0x%02x\n", data);    
#endif

    prpass(testpass, "Restore default value after test.");
    /* Restore the default OUTPUT value */
    data = restore_data;
    if (io_port_8bit_i2c_write(pca1, OUTPUT_PORT_REG,
				&data)) {
	return (FAILED);
    }
    msleep(1000);

	if (io_port_8bit_i2c_read(pca1, OUTPUT_PORT_REG,
				   &data, TRUE)) {
	    return (FAILED);
	}

	if((data & restore_data) != restore_data) {
        printf("\n Failed restore output value = 0x%02x, expected = 0x%02x",
               (data & restore_data), restore_data);
        return (FAILED);
	}

    if (debug) {
        printf("\n%s, [#%d]: After restored OUTPUT bits, read back contents = 0x%02x\n",
        		__FUNCTION__, __LINE__, (data & restore_data));
        printf("\n");
    }

    return(retval);

}

/**********************************************************************
 *
 * Function: patriot_memory_ecc_test
 * This function test the Patriot module memory ECC
 *
 * Input : iface - Patriot data structure info pointer
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_memory_ecc_test(patriot_ds_t *iface)
{
    int retval = PASSED;
    
    prpass(testpass, "Patriot memory ECC test");
    
    if (patriot_setup_ge_env(iface) == FAILED) {
        return (FAILED);
    }

    patriot_clear_rx_buf();
    
    if (patriot_send_cmd(iface, FROM_HOST_ECC_TEST, 0)) {
	retval = FAILED;
    }

    if (retval == PASSED) {
	if (patriot_rcv_cmd_result_packet(iface, FROM_HOST_ECC_TEST)) {
	    retval = FAILED;
	}
    }
    
    if (patriot_cleanup_ge_env(iface) == FAILED) {
	retval = FAILED;
    }
    
    return(retval);
    
}

/**********************************************************************
 *
 * Function: patriot_uart_test
 * This function test UART interface. The host will send a string to
 * the module side. Once the module receive the string, it'll compare
 * with the original. If they're the same, it'll print out another
 * string on the UART interface. The host side will read it and compare.
 *
 * Input : iface - Patriot data structure info pointer
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_uart_test(patriot_ds_t *iface)
{

    int retval, i;
    char buf[64] = {0};
    int port, rx_sz;
    char *ptr;
    char *str = "ABCDEFGH\n", *str1 = "STUVWXYZ\n";

    memset(buf, 0, sizeof(buf));
    
    /* Ask the module to wait for a string from host */
    if (patriot_setup_ge_env(iface) == FAILED) {
        return (FAILED);
    }

    patriot_clear_rx_buf();
    
    if (patriot_send_cmd(iface, FROM_HOST_UART_TEST, 0)) {
	retval = FAILED;
    }
    //    msleep(300);
    port = iface->uart; 
    
    dash_uart_reset(port);
    dash_uart_tx(port, 9600, str, strlen(str), 0);

    if (patriot_rcv_cmd_result_packet(iface, FROM_HOST_UART_TEST)) {
        retval = FAILED;
    }

    if (patriot_cleanup_ge_env(iface) == FAILED) {
	retval = FAILED;
    }

    if (retval == FAILED) {
        cterr('f', 0, "GE command to send uart string failed");
	return (FAILED);
    }

    /*a delay between tx/rx here will cause uart to fail so
     after tx, quickly rx as soon as possible */
    rx_sz = 0;
    dash_uart_rx(port, &rx_sz, buf);

    dash_uart_reset(port);

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("!!!!!string received=%s; [ sent %s] %s\n",
               buf, str, str1);
    }
    dash_uart_reset(port);
    ptr = memchr(buf, str1[0], strlen(str) + strlen(str1));
    if (ptr == NULL) {
	cterr('f', 0, "tx/rx strings do not match: expected %s, got %s",
	      str1, buf);
	return (FAILED);
    }
    for (i = 0; i < strlen(str1)-1; i++, ptr++) {
        if (*ptr != str1[i]) {
            cterr('f', 0, "tx/rx strings do not match: expected %s, got %s",
                  str1, buf);
        }
	return(FAILED);
    }
    return(PASSED);
}

/**********************************************************************
 *
 * Function: patriot_ge0_loopback_test
 * This function test the GE 0 interface. The Patriot module will
 * send ethernet packets to the host switch, and loopback there
 *
 * Input : iface - Patriot data structure info pointer
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
patriot_ge0_loopback_test(patriot_ds_t *iface)
{
    int ge_port, retval = PASSED;
    
    prpass(testpass, "GE 0 Loopback test");

    ge_port = ovld_get_ge_sw_port_num(iface->slot, TGT_DEV_NGSM, 1);
    /* Set the loopback at the switch */
    set_gesw_line_loopback(ge_port, 1);
    
    if (patriot_setup_ge_env(iface) == FAILED) {
        return (FAILED);
    }

    patriot_clear_rx_buf();
    
    if (patriot_send_cmd(iface, FROM_HOST_GE0_LPBK_TEST, 0)) {
	retval = FAILED;
    }

    if (retval == PASSED) {
	if (patriot_rcv_cmd_result_packet_for_ge0_lpbk(iface,
						       FROM_HOST_GE0_LPBK_TEST)) {
	    retval = FAILED;
	}
    }

    if (patriot_cleanup_ge_env(iface) == FAILED) {
	retval = FAILED;
    }

    /* Unset the loopback at the switch */
    set_gesw_line_loopback(ge_port, 0);
    
    return(retval);
    
}


/******** History ********/ 
/*--------------------------------------------------------------------
$Log: sm_patriot_test.c,v $
Revision 1.23  2018/05/18 09:24:48  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.22.40.1  2016/12/05 06:36:59  alpeng
fixed the uart ctrl num for ngio; change is approved on prrq

Revision 1.22  2014/06/12 19:16:57  huanngo
Fix the GE0 loopback failure on the new switch from Utah

Revision 1.21  2014/05/19 23:31:33  mcharon
cvs update

Revision 1.20  2014/02/22 05:06:07  mcharon
add uart test that bypassse tty driver

Revision 1.19  2014/02/18 09:11:12  alpeng
CSCul88171-3: remove useless files: mon_boot.h, c82576_ethmap.h, eth_frames.c, stack.h, sys_regs.h

Revision 1.18  2013/01/28 01:52:47  steja
<CSCue19620> Diag Improvement for I/O Expander testing.

Revision 1.17  2012/10/25 08:23:54  steja
Remove "Clear" from the subrate test

Revision 1.16  2012/10/16 12:35:01  steja
Improve the GPIO test

Revision 1.15  2012/10/15 21:24:14  huanngo
Remove the test for upgrade SPI PROM as it's no longer populaced

Revision 1.14  2012/09/24 05:58:25  alpeng
add argument for rx_uart(), for getting last character on rx

Revision 1.13  2012/07/25 00:43:07  mcharon
add size and timeout argument for uart_rx

Revision 1.12  2012/07/25 00:12:15  huanngo
Fix uart test with new linux kernel

Revision 1.11  2012/06/30 00:15:48  huanngo
Adding UART test

Revision 1.10  2012/06/13 23:45:16  huanngo
Add a bug fix for UART test

Revision 1.9  2012/06/12 17:47:05  huanngo
Fix compiling warning by adding the header file "bcm_gesw_defs.h"

Revision 1.8  2012/06/08 19:05:43  huanngo
Move the patriot_memory_ecc_test from sm_patriot_utils.c to sm_patriot_test.c

Revision 1.7  2012/06/07 21:20:17  huanngo
Adding new tests

Revision 1.6  2012/05/17 02:55:09  steja
Remove the IO 7 test for P2 board

Revision 1.5  2012/05/02 17:55:58  huanngo
Clean up and dowonload FPGA when necessary, not right after boot up Linux

Revision 1.4  2012/03/28 23:35:08  huanngo
Support new tests and utilities on Patriot

Revision 1.3  2012/03/28 00:38:15  mcharon
remove forward slash from second line

Revision 1.2  2012/03/25 03:58:22  palin2
Clean up Overlord Intel side (x86) compile warnings.

Revision 1.1.1.1  2012/03/23 23:02:05  ptong
Initial archive of ng_diag module

$Endlog$

*/
