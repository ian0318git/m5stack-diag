/* $Id: diag_lewis_gesw_test.c,v 1.4 2017/03/30 08:30:53 hondwang Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_lewis_gesw_test.c,v $ 
 *------------------------------------------------------------------
 *
 * diag_lewis_gesw_test.c - Marvell lewis_gesw (98DX4235) Switch tests
 * 
 * November 2015, Josh Skow
 *
 * Copyright (c) 2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ioctl.h> 
#include <sys/wait.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include "common.h"
#include "types.h"
#include "error.h"
#include "queryflags.h"
#include "slot.h"
#include "menu.h"
#include "ngio.h"
#include "defs.h"
#include "nvmonvars.h"
#include "diag_lewis_gesw_test.h"
#include "intel_tests.h"
#include "diag_nc_common.h"
#include "platform_fru.h"
#include "diag_i2c_test.h"
#include <fcntl.h>
#include "endians.h"
#include "types.h"
#include "nvsysvars.h"
#include "pcmap.h"
#include "strings.h"
#include "cross_platform.h"
#include "mb_tests.h"

/* Marvell Switch FLAG define */
#define MVLF_1    (MF_CONTINUOUS | MF_DOGRP)
#define MVLF_2    (MVLF_1 | MF_DOALL)
#define MVLF_3    (MVLF_2 | MF_SHOW_ERRCOUNT)
#define MVLF_4    (MVLF_1 | MF_SHOW_ERRCOUNT)

/* Show time stamps */
int VERBOSE_MODE=0;
time_t start;
time_t end;
long test_time;

/* Child process PID for receiving info from GESW*/
pid_t childpid; 
int status;
int lpbk_diag_flag=1;

int check_nc_up(void);
int diag_lewis_gesw_test(int);
int lewis_gesw_pass_file (void);
int run_lewis_gesw_test (char *test_comm, int tachi_flag);

extern int do_all_menu_items(struct menuinfo *);

int diag_lewis_gesw_test(int);
int lewis_gesw_pass_file (void);
int run_lewis_gesw_test (char *test_comm, int tachi_flag);

static int diag_lewis_gesw_mem_test(void);
static int diag_lewis_gesw_reg_test(void);
static int diag_lewis_gesw_nand_test(void);
static int diag_lewis_gesw_gpio_test(void);
static int diag_lewis_gesw_poe_test(void);
static int diag_lewis_gesw_poe_reg_test(void);
static int diag_lewis_gesw_phy_reg_test(void);
static int diag_lewis_gesw_phy_irq_test(void);
static int diag_lewis_gesw_phy_int_loopback_test(void);

static int diag_lewis_gesw_phy_ext_loopback_test(void);
static int diag_lewis_gesw_port_lpbk (void);
// static int diag_lewis_gesw_bist (void);

static int display_lewis_gesw_mem_test_reg(void);
static int display_lewis_gesw_nand_test_reg(void);
static int display_lewis_gesw_gpio_test_reg(void);
static int display_lewis_gesw_phy_reg_test_reg(void);
static int display_lewis_gesw_phy_irq_test_reg(void);
static int display_lewis_gesw_phy_int_loopback_reg(void);
static int display_lewis_gesw_phy_ext_loopback_reg(void);
static int display_lewis_gesw_reg_test_reg(void);
static int display_lewis_gesw_port_lpbk_reg(void);
static int display_lewis_gesw_poe_test_reg(void);

/* Sub Menu used for lewis_gesw tests.
 */
static submenu_xtable_t lewis_gesw_tests_submenu_table[] = {
    {"Lewis GESW utils", (PFT)diag_lewis_gesw_util,	0,		
	0, (type_t(*)())0, 0,		(PFT)0,	0},
/* Hidden this test, due to Marvell FAE is still investigating 
    {"Lewis I2c test", (type_t(*)())diag_lewis_i2c_scan_test,   0,
	MVLF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
*/
    {"Switch Registers Test", (type_t(*)())diag_lewis_gesw_reg_test,   0,
	MVLF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Internal Port Loopback Test", (type_t(*)())diag_lewis_gesw_port_lpbk,   0,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"DDR3 Memory Test", (type_t(*)())diag_lewis_gesw_mem_test,   0,
	MVLF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"NAND Flash Test", (type_t(*)())diag_lewis_gesw_nand_test,   0,
	MVLF_4, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"GPIO Test", (type_t(*)())diag_lewis_gesw_gpio_test,   0,
	MVLF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Phy Register Test", (type_t(*)())diag_lewis_gesw_phy_reg_test,   0,
	MVLF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Phy IRQ Test", (type_t(*)())diag_lewis_gesw_phy_irq_test,   0,
	MVLF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Phy External Loopback Test", (type_t(*)())diag_lewis_gesw_phy_ext_loopback_test,   0,
	MVLF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Phy Internal Loopback Test", (type_t(*)())diag_lewis_gesw_phy_int_loopback_test,   0,
	MVLF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"PoE Test", (type_t(*)())diag_lewis_gesw_poe_test,   0,
	MVLF_4, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"PoE Register Test", (type_t(*)())diag_lewis_gesw_poe_reg_test,   0,
	MVLF_4, (type_t(*)())0, 0, (type_t(*)())0,   0},
/*     {"Switch BIST", (type_t(*)())diag_lewis_gesw_bist,   0,
	MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0,   0}, */
};


#define LEWIS_GESW_TESTS_SUBMENU_TABLE_SIZE (sizeof(lewis_gesw_tests_submenu_table) / \
				       sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t lewis_gesw_tests_primary_items[LEWIS_GESW_TESTS_SUBMENU_TABLE_SIZE +
				       MAX_BASE_ITEMS];
static mitem_t lewis_gesw_tests_secondary_items[LEWIS_GESW_TESTS_SUBMENU_TABLE_SIZE +
					 MAX_BASE_ITEMS];

menuinfo_t lewis_gesw_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    lewis_gesw_tests_primary_items,
};
menuinfo_t *lewis_gesw_submenup = &lewis_gesw_subtest_menu;

int diag_lewis_gesw_test (int run_all_tests)
{

    build_primary_submenu(lewis_gesw_tests_submenu_table,
			              LEWIS_GESW_TESTS_SUBMENU_TABLE_SIZE,
                          "Lewis GESW", &lewis_gesw_submenup);
    build_secondary_submenu(lewis_gesw_tests_submenu_table,
                            LEWIS_GESW_TESTS_SUBMENU_TABLE_SIZE,
                            lewis_gesw_tests_secondary_items);    

    /* check intel & lewis is up */
    if (check_intel_linux_ready()) {
        printf("Could not auto check Intel linux ready");
        return (FAILED);
    }	
	if (check_menu_flag(D_VERBOSE)) {
		printf("\nVerbose flag is set \n");
		VERBOSE_MODE=1;
	}
	gesw_image_info();
    if (run_all_tests) {
        do_all_menu_items(lewis_gesw_submenup);
    } else {
        menu(lewis_gesw_submenup, lewis_gesw_tests_secondary_items, '\0');
    }
    return (PASSED);
}
/* Generic enhanced error message function.  This provides further debug information
  when encountering an error with the GESW.
*/
static void add_gesw_98DX_err_report (void)
{
	fru_table_offset = GESW_98DX;
	platform_fru_table[GESW_98DX].pid_string = gesw_98DX;
    platform_fru_table[GESW_98DX].location_string = gesw_98DX_loc;

}

/*This function is the main Lewis GESW test entry point.  Take in the command, and send
  to Lewis GESW to run, then read and print the results.
 */
int lewis_gesw_pass_file (void)
{
	/*Linux dependent system calls */
	int ret = PASSED;
	FILE *gesw_log;
	char blank[18] = " ";
	char buf[80];
	int len;
	int count=0;
	int log_open=1;
	if (system("cat marvell_command | nc 192.123.123.11 12345 &> diag_gesw_log "))
	{
		ret = FAILED;
	}
	
	gesw_log = fopen("diag_gesw_log", "r+");
	if (!(gesw_log == NULL))
	{
		if (ret != FAILED)
		{	
			/*Remove last line of log */
			if (fseek(gesw_log, -18, SEEK_END))
			{
				printf("fseek failed\n");
			}
			len = fwrite(blank, 1, 18, gesw_log);
			/*Find line where Lua CLI shell ready */
			fseek(gesw_log, 0, SEEK_SET);
			printf("\n");
			while(fgets(buf, sizeof(buf), gesw_log))
			{
				/*Only print lines relevant to testing*/
				if (count > 0) 
				{
					if (count > 4) 
					{
						printf("%s", buf);
					}
					count++;
				}
				if (strstr(buf, "LUA CLI shell ready") && count < 1)
				{
					/*Start printing after next 3 lines are read*/
					count = 1;
				}
				/*Check log for error*/
				if (strstr(buf, "error") || strstr(buf, "ERROR"))
				{
					ret = FAILED;
				}
			}
			fclose(gesw_log);
			log_open=0;
		} else {
			printf("\n");
			system("more diag_gesw_log");
		}
		if (log_open > 0)
		{
			fclose(gesw_log);
		}
/* 		if (!system("(grep 'error' diag_gesw_log >& /dev/null)") || (!system("(grep 'ERROR' diag_gesw_log >& /dev/null)"))) 
		{
			ret = FAILED;
		} */
	} else {
		printf("\nERROR: No log file.  The test did not run, check if BMC-Switch connection is up.");
		printf("\nTo check GESW connection, ping 192.123.123.11 on BMC console.");
		fclose(gesw_log);
	}
	system("rm marvell_command");
 	system("rm diag_gesw_log");  
	return ret;
}
/* Run a background process to see if Lewis sends current test information back to BMC.
   If information is sent back to BMC, read the information to the screen.
 */
int lewis_test_rcv_info (void) 
{
	int ret;
	/*Open BMC result file port to transmit info mid-test if needed */
	check_nc_up();
	childpid = fork();
	if (childpid == 0)
	{
		while (1) {
			if (system("ls /tmp/nc_result >& /dev/null") == 0)
			{
				ret = nc_check_test_status();
				check_nc_up();
			}
			sleep(1);
		}
	}
	else if (childpid > 0)
	{
		/* printf("This is not the child process"); */
	} 
	else
	{
		printf("Could not start GESW mid-test info receiver");
	}
	return PASSED;
}

/*Check system processes to see if nc connection is already up */
int check_nc_up (void) 
{
	int ret=PASSED;
	if (system("ps | grep '[n]c -l -p 1888' >& /dev/null") == 0) 
	{

	} else
	{
		nc_init_result_file();
	}
	return (ret);
}
/*Clean up test listening child process */
int kill_lewis_rcv_info (void)
{
	if (childpid > 0) 
	{
	kill(childpid, SIGKILL);
	system("pkill -f 'nc -l -p 1888'");
	wait(&status);
	}
	return PASSED;
}
/*Tachi_flag set to TACHI_SPECIFIC if tachi-l menu needed,
* otherwise set the flag to GENERIC_TEST
* test_comm is the GESW luaCli string that runs the test/utility
* e.g. "nand test device 0 bmc_conn 1\r"
 */
int run_lewis_gesw_test (char *test_comm, int tachi_flag)
{
	int ret = PASSED;

	char tachil_str[] = NC_MVL_TACHI_L;
	int chr_lng = strlen(tachil_str) + strlen(test_comm);
	char comm[chr_lng];
	FILE *test_file;
	
	sprintf(comm, "%s%s", tachil_str, test_comm);
	
	/* Write the Switch test command to a file, used to run test on Switch */
	test_file = fopen("marvell_command", "a+");
	if (tachi_flag == 1) {
		fwrite(comm, 1, sizeof(comm), test_file);
	} else {
		fwrite(test_comm, 1, sizeof(test_comm), test_file);
	}
	fclose(test_file);
	
	/* Run the test */
	if (lewis_gesw_pass_file()) {
		ret = FAILED;
	}

	return (ret);
}

/*------------------------------------------------------------------------------
 *
 * Function: diag_lewis_gesw_mem_test
 *
 * Description: This function executes the lewis_gesw DDR3 mem test
 *
 * Input:  VOID
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */
static int diag_lewis_gesw_mem_test (void)
{
    int ret = PASSED;
    char test_comm[] = NC_MVL_MEM_TEST;
	
    if (get_enhance_err_flag()) {
        add_gesw_98DX_err_report();
        cterr_add_component("GESW-98DX", "GESW-98DX DDR3 SDRAM");
        cterr_add_reg_dump((PFV)display_lewis_gesw_mem_test_reg);
        cterr_add_env_dump((PFV)display_env);
        cterr_add_debug("Check DDR3 memory connection.",
	    "Try different DDR3 memory card.",
	    "Check data path trace from DDR3 to GESW 98DX.");
    }
	
    if (VERBOSE_MODE == 1) {
        start = time(NULL);
    }
    /*Fork process to listen for mid-test info*/
    lewis_test_rcv_info();
    testname("DDR3 Memory");
    prpass(testpass, "DDR3 Memory, ");

    if (run_lewis_gesw_test(test_comm, TACHI_SPECIFIC)) {
        if (!check_menu_flag(D_STOPONERR)) {
            kill_lewis_rcv_info();
    }
        cterr('f', 0, "DDR3 Memory Test Failed");
        ret = FAILED;
    }
    /* After test, kill mid-test info listening child process */
    kill_lewis_rcv_info();
	
    if (VERBOSE_MODE == 1) {
        end = time(NULL);
        test_time = end-start;
        printf("\nThe test took %ld seconds\n", test_time);
    }
    prcomplete(testpass, errcount, 0);

    return (ret);
}

static int display_lewis_gesw_mem_test_reg (void)
{
    cterr_db_print("Test by Lewis /dev/mem driver, no register dump");
    return (PASSED);
}
/*------------------------------------------------------------------------------
 *
 * Function: diag_lewis_gesw_nand_test
 *
 * Description: This function executes the NAND Flash test
 *
 * Input:  VOID
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */
static int diag_lewis_gesw_nand_test (void)
{
	int ret = PASSED;
	
	if (get_enhance_err_flag()) {
        add_gesw_98DX_err_report();
		cterr_add_component("GESW-98DX", "GESW-98DX NAND Flash");
		cterr_add_reg_dump((PFV)display_lewis_gesw_nand_test_reg);
		cterr_add_env_dump((PFV)display_env);
		cterr_add_debug("Check NAND flash physical connection.",
						"Try different NAND flash component.");
    }
	if (VERBOSE_MODE == 1) {
		start = time(NULL);
	}
    testname("NAND Flash");
    prpass(testpass, "NAND Flash, ");
	char test_comm[] = NC_MVL_NAND_TEST;
	
	if (run_lewis_gesw_test(test_comm, TACHI_SPECIFIC)) 
	{
		cterr('f', 0, "NAND Flash Test Failed");
        ret = FAILED;
	}
	if (VERBOSE_MODE == 1) {
		end = time(NULL);
		test_time = end-start;
		printf("\nThe test took %ld seconds\n", test_time);
	}
    prcomplete(testpass, errcount, 0);
    return (ret);
}

static int display_lewis_gesw_nand_test_reg (void)
{
    cterr_db_print("Test by Lewis /dev/mtd3 driver, no register dump");
    return (PASSED);
}

/*------------------------------------------------------------------------------
 *
 * Function: diag_lewis_gesw_gpio_test
 *
 * Description: This function executes the lewis_gesw GPIO test
 *
 * Input:  VOID
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */
static int diag_lewis_gesw_gpio_test (void)
{
	int ret = PASSED;
		
	if (get_enhance_err_flag()) {
        add_gesw_98DX_err_report();
		cterr_add_component("GESW-98DX", "GESW-98DX LEDs", "GESW-PoE Daughter Card",
							"FPGA");
		cterr_add_reg_dump((PFV)display_lewis_gesw_gpio_test_reg);
		cterr_add_env_dump((PFV)display_env);
		cterr_add_debug("Check LEDs traces.",
						"Check PoE daughter card connection.",
						"Check LEDs to see if burned out.");
    }
	if (VERBOSE_MODE == 1) {
		start = time(NULL);
	}
    testname("GPIO/LED");
    prpass(testpass, "GPIO/LED, ");
    char test_comm[] = NC_MVL_GPIO_TEST;

	if (run_lewis_gesw_test(test_comm, TACHI_SPECIFIC)) 
	{
		cterr('f', 0, "GPIO Test Failed");
        ret = FAILED;
    }
	if (VERBOSE_MODE == 1) {
		end = time(NULL);
		test_time = end-start;
		printf("\nThe test took %ld seconds\n", test_time);
	}
    prcomplete(testpass, errcount, 0);
    return (ret);
}

static int display_lewis_gesw_gpio_test_reg (void)
{
    cterr_db_print("Test by Lewis /dev/mem driver, no register dump");
    return (PASSED);
}
/*------------------------------------------------------------------------------
 *
 * Function: diag_lewis_gesw_phy_reg_test
 *
 * Description: This function executes the lewis_gesw phy register test
 *
 * Input:  VOID
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */
static int diag_lewis_gesw_phy_reg_test (void)
{
    int ret = PASSED;
	
    if (get_enhance_err_flag()) {
        add_gesw_98DX_err_report();
        cterr_add_component("GESW-98DX", "GESW-PHY 88E1680L");
        cterr_add_reg_dump((PFV)display_lewis_gesw_phy_reg_test_reg);
        cterr_add_env_dump((PFV)display_env);
        cterr_add_debug("Check MDC/MDIO trace between 88E1680L MDC/MDIO and Lewis", 
            "Check the 88E1680L chip is functional.",
            "Try new 88E1680L, chip may be bad.");
    }
    if (VERBOSE_MODE == 1) {
        start = time(NULL);
    }
    testname("Phy Register");
    prpass(testpass, "Phy Register, ");
	char test_comm[] = NC_MVL_PHY_REG_TEST;

	if (run_lewis_gesw_test(test_comm, TACHI_SPECIFIC)) 
	{
		cterr('f', 0, "Phy Register Test Failed");
        ret = FAILED;
	}
	if (VERBOSE_MODE == 1) {
		end = time(NULL);
		test_time = end-start;
		printf("\nThe test took %ld seconds\n", test_time);
	}
    prcomplete(testpass, errcount, 0);
    return (ret);
}

static int display_lewis_gesw_phy_reg_test_reg (void)
{
    cterr_db_print("Relative testing register dumped in log");
    return (PASSED);
}

/*------------------------------------------------------------------------------
 *
 * Function: diag_lewis_gesw_phy_irq_test
 *
 * Description: This function executes the lewis_gesw DDR3 mem test
 *
 * Input:  VOID
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */
static int diag_lewis_gesw_phy_irq_test (void)
{
    int ret = PASSED;

    if (get_enhance_err_flag()) {
        add_gesw_98DX_err_report();
        cterr_add_component("GESW-98DX", "GESW-PHY 88E1680L");
        cterr_add_reg_dump((PFV)display_lewis_gesw_phy_irq_test_reg);
        cterr_add_env_dump((PFV)display_env);
	cterr_add_debug("Check traces betewwn 88E1680L and Lewis GPIO pin",
            "Check 88E1680L is functional.",
            "Try new 88E1680L, chip may be bad.");
    }
    if (VERBOSE_MODE == 1) {
        start = time(NULL);
    }
    testname("PHY IRQ");
    prpass(testpass, "PHY IRQ, ");
    char test_comm[] = NC_MVL_PHY_IRQ_TEST;

	if (run_lewis_gesw_test(test_comm, TACHI_SPECIFIC)) 
	{
		cterr('f', 0, "PHY IRQ Test Failed");
        ret = FAILED;
	}
	if (VERBOSE_MODE == 1) {
		end = time(NULL);
		test_time = end-start;
		printf("\nThe test took %ld seconds\n", test_time);
	}
    prcomplete(testpass, errcount, 0);
    return (ret);
}

static int display_lewis_gesw_phy_irq_test_reg (void)
{
    cterr_db_print("Relative testing register dumped in log");
    return (PASSED);
}

/*------------------------------------------------------------------------------
 *
 * Function: diag_lewis_gesw_phy_int_loopback_test
 *
 * Description: This function executes the lewis_gesw phy internal loopback test
 *
 * Input:  VOID
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */
static int diag_lewis_gesw_phy_int_loopback_test (void)
{
    int ret = PASSED;
	
    if (get_enhance_err_flag()) {
        add_gesw_98DX_err_report();
        cterr_add_component("GESW-98DX", "GESW-PHY 88E1680L");
        cterr_add_reg_dump((PFV)display_lewis_gesw_phy_int_loopback_reg);
        cterr_add_env_dump((PFV)display_env);
        cterr_add_debug("Check QSGMII interface between 88E1680L and Lewis",
            "Check 88E1680L is functional.",
            "Try new 88E1680L, chip may be bad.");
    }
	if (VERBOSE_MODE == 1) {
		start = time(NULL);
	}
	lewis_test_rcv_info();
    testname("Phy Internal Loopback");
    prpass(testpass, "Phy Internal Loopback, ");
	char test_comm[] = NC_MVL_PHY_INLB_TEST;

	if (lpbk_diag_flag)
	{
		if (run_lewis_gesw_test(test_comm, TACHI_SPECIFIC)) 
		{
				if (!check_menu_flag(D_STOPONERR)) 
				{
					kill_lewis_rcv_info();
				}
				cterr('f', 0, "Phy Internal Loopback Test Failed");
				ret = FAILED;
		}
	} else
	{
		printf("\nExternal Phy loopback test run, skipping this test");
		lpbk_diag_flag=1;
	}
	kill_lewis_rcv_info();

	if (VERBOSE_MODE == 1) {
		end = time(NULL);
		test_time = end-start;
		printf("\nThe test took %ld seconds\n", test_time);
	}
    prcomplete(testpass, errcount, 0);
	if (ret == FAILED) 
	{
		printf("\n PHY Internal Loopback test FAILED!\n");
		printf("\n Running GESW Internal Loopback test to diagnose problem...\n");
		diag_lewis_gesw_port_lpbk();
	}
    return (ret);
}

static int display_lewis_gesw_phy_int_loopback_reg (void)
{
    cterr_db_print("Test by Lewis CPSS tool, no register dump");
    return (PASSED);
}

/*------------------------------------------------------------------------------
 *
 * Function: diag_lewis_gesw_phy_ext_loopback_test
 *
 * Description: This function executes the lewis_gesw phy external loopback test
 *
 * Input:  VOID
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */
static int diag_lewis_gesw_phy_ext_loopback_test (void)
{
    int ret = PASSED;
	
    if (get_enhance_err_flag()) {
        add_gesw_98DX_err_report();
        cterr_add_component("GESW-98DX", "GESW-PHY 88E1680L","GESW-External Ports", 
	    "GESW-External loopback connector");
        cterr_add_reg_dump((PFV)display_lewis_gesw_phy_ext_loopback_reg);
        cterr_add_env_dump((PFV)display_env);
        cterr_add_debug("Check results of Internal PHY loopback test.",
            "If no error with Internal phy lpbk test, probe external port traces.",
            "If no external port problems, try different loopback connector.");
    }
	if (VERBOSE_MODE == 1) {
		start = time(NULL);
	}
	lewis_test_rcv_info();
    testname("PHY External Loopback");
    prpass(testpass, "PHY External Loopback, ");
	
	if (check_menu_flag(D_EXT_LOOPBACK)) {
		printf("\nExternal loopback flag not set, skipping this test ");
		kill_lewis_rcv_info();
		prcomplete(testpass, errcount, 0);
    	return (ret);
	} else {
		char test_comm[] = NC_MVL_PHY_EXLB_TEST;
	
		if (run_lewis_gesw_test(test_comm, TACHI_SPECIFIC)) 
		{
			if (!check_menu_flag(D_STOPONERR)) 
			{
			kill_lewis_rcv_info();
			}
			cterr('f', 0, "PHY External Loopback Test Failed");
			ret = FAILED;
		}
		kill_lewis_rcv_info();
		if (VERBOSE_MODE == 1) {
			end = time(NULL);
			test_time = end-start;
			printf("\nThe test took %ld seconds\n", test_time);
		}
		prcomplete(testpass, errcount, 0);
		if (ret == FAILED) 
		{
			printf("\n PHY External Loopback test FAILED!\n");
			printf("\n Running PHY Internal Loopback test to diagnose problem...\n");
			diag_lewis_gesw_phy_int_loopback_test();
			lpbk_diag_flag=0;
		} else
		{
			lpbk_diag_flag=0;
		}
		return (ret);
	}
}

static int display_lewis_gesw_phy_ext_loopback_reg (void)
{
    cterr_db_print("Test by Lewis CPSS tool, no register dump");
    return (PASSED);
}

/*------------------------------------------------------------------------------
 *
 * Function: diag_lewis_gesw_reg_test
 *
 * Description: This function executes the lewis_gesw DDR3 mem test
 *
 * Input:  VOID
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */
static int diag_lewis_gesw_reg_test (void)
{
    int ret = PASSED;

    if (get_enhance_err_flag()) {
        add_gesw_98DX_err_report();
        cterr_add_component("GESW-98DX");
        cterr_add_reg_dump((PFV)display_lewis_gesw_reg_test_reg);
        cterr_add_env_dump((PFV)display_env);
        cterr_add_debug("Check physical connections of 98DX.",
            "Check 98DX physical traces.",
            "Try different 98DX.");
    }
    if (VERBOSE_MODE == 1) {
        start = time(NULL);
    }
    testname("Switch Registers");
    prpass(testpass, "Switch Registers, ");
    char test_comm[] = NC_MVL_SW_REG_TEST;
	
    if (run_lewis_gesw_test(test_comm, TACHI_SPECIFIC)) {
        cterr('f', 0, "Switch Registers Test");
        ret = FAILED;
    }
    if (VERBOSE_MODE == 1) {
        end = time(NULL);
        test_time = end-start;
        printf("\nThe test took %ld seconds\n", test_time);
    }
    prcomplete(testpass, errcount, 0);
    return (ret);
}

static int display_lewis_gesw_reg_test_reg (void)
{
    cterr_db_print("Relative testing register dumped in log");
    return (PASSED);
}

/*------------------------------------------------------------------------------
 *
 * Function: diag_lewis_gesw_bist
 *
 * Description: This function executes the lewis_gesw BIST to test internal memory
 *
 * Input:  VOID
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */
/* static int diag_lewis_gesw_bist (void)
{
	int ret = PASSED;
    testname("Switch Built in Self Test ");
    prpass(testpass, "Switch BIST");
	char test_comm[] = NC_MVL_SW_BIST_TEST;
	char test_comm2[] = "end\r reload\r";
	
	if (run_lewis_gesw_test(test_comm, 1)) 
	{
		cterr('f', 0, "Switch Registers Test");
        ret = FAILED;
	}
	sleep(1);
	if (run_lewis_gesw_test(test_comm2, 0)) 
	{
		cterr('f', 0, "Switch Registers Test");
        ret = FAILED;
	}
	
    prcomplete(testpass, errcount, 0);
    return (ret);
} */

/*------------------------------------------------------------------------------
 *
 * Function: diag_lewis_gesw_port_lpbk
 *
 * Description: This function executes the internal port loopback test.
 *
 * Input:  VOID
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */
static int diag_lewis_gesw_port_lpbk (void)
{
    int ret = PASSED;
	
    if (get_enhance_err_flag()) {
        add_gesw_98DX_err_report();
        cterr_add_component("GESW-98DX");
        cterr_add_reg_dump((PFV)display_lewis_gesw_port_lpbk_reg);
        cterr_add_env_dump((PFV)display_env);
        cterr_add_debug("Check physical connections of 98DX.",
	    "Check 98DX physical traces.",
	    "Try different 98DX.");
    }
    if (VERBOSE_MODE == 1) {
        start = time(NULL);
    }
    lewis_test_rcv_info();
    testname("Internal port loopback");
    prpass(testpass, "Internal port loopback, ");
	char test_comm[] = NC_MVL_SW_LB_TEST;
	
	if (run_lewis_gesw_test(test_comm, TACHI_SPECIFIC)) 
	{
		if (!check_menu_flag(D_STOPONERR)) 
		{
		kill_lewis_rcv_info();
		}
		cterr('f', 0, "Internal port loopback failed");
        ret = FAILED;
	}
	kill_lewis_rcv_info();
	if (VERBOSE_MODE == 1) {
		end = time(NULL);
		test_time = end-start;
		printf("\nThe test took %ld seconds\n", test_time);
	}
    prcomplete(testpass, errcount, 0);

    return (ret);
}

static int display_lewis_gesw_port_lpbk_reg (void)
{
    cterr_db_print("Test by Lewis CPSS tool, no register dump");
    return (PASSED);
}

/*------------------------------------------------------------------------------
 *
 * Function: diag_lewis_gesw_poe_test
 *
 * Description: This function executes the lewis_gesw poe test
 *
 * Input:  VOID
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */
static int diag_lewis_gesw_poe_test (void)
{
    int ret = PASSED;
	
    if (get_enhance_err_flag()) {
        add_gesw_98DX_err_report();
        cterr_add_component("GESW-98DX", "GESW-98DX PoE Card");
        cterr_add_reg_dump((PFV)display_lewis_gesw_poe_test_reg);
        cterr_add_env_dump((PFV)display_env);
        cterr_add_debug("Check PoE card connector.",
            "Check SPI traces between Lewis and PoE card.",
	    "Check the PoE card traces.");
    }
    if (VERBOSE_MODE == 1) {
        start = time(NULL);
    }
    testname("PoE Test");
    prpass(testpass, "PoE Test, ");
    char test_comm[] = NC_MVL_POE_TEST;

    if (run_lewis_gesw_test(test_comm, TACHI_SPECIFIC)) {
		cterr('f', 0, "PoE test Failed");
        ret = FAILED;
	}
	if (VERBOSE_MODE == 1) {
		end = time(NULL);
		test_time = end-start;
		printf("\nThe test took %ld seconds\n", test_time);
	}
	
    prcomplete(testpass, errcount, 0);
    return (ret);
}

/*------------------------------------------------------------------------------
 *
 * Function: diag_lewis_gesw_poe_reg_test
 *
 * Description: This function executes the lewis_gesw poe SPI register test
 *
 * Input:  VOID
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */
static int diag_lewis_gesw_poe_reg_test (void)
{
    int ret = PASSED;
    char test_comm[] = NC_MVL_POE_REG_TEST;

    if (get_enhance_err_flag()) {
        add_gesw_98DX_err_report();
        cterr_add_component("GESW-98DX", "GESW-98DX PoE Card");
        cterr_add_reg_dump((PFV)display_lewis_gesw_poe_test_reg);
        cterr_add_env_dump((PFV)display_env);
        cterr_add_debug("Check PoE card connector.",
            "Check SPI traces between Lewis and PoE card.",
	    "Check the PoE card traces.");
    }
    if (VERBOSE_MODE == 1) {
        start = time(NULL);
    }
    testname("PoE Reg Test");
    prpass(testpass, "PoE Reg Test, ");

    if (run_lewis_gesw_test(test_comm, TACHI_SPECIFIC)) {
        cterr('f', 0, "PoE register test Failed");
        ret = FAILED;
	}
	if (VERBOSE_MODE == 1) {
		end = time(NULL);
		test_time = end-start;
		printf("\nThe test took %ld seconds\n", test_time);
	}
	
    prcomplete(testpass, errcount, 0);
    return (ret);
}

static int display_lewis_gesw_poe_test_reg (void)
{
    cterr_db_print("Test by Lewis /dev/dragonite driver, no register dump");
    return (PASSED);
}

/*----------------------------------------------------------------
$Log: diag_lewis_gesw_test.c,v $
Revision 1.4  2017/03/30 08:30:53  hondwang
Tachi-L brach merge

Revision 1.3.10.3  2017/02/08 03:42:55  hondwang
Add POE register test

Revision 1.3.10.2  2016/11/30 13:32:28  hondwang
Fix build image issue with enhance error message

Revision 1.3  2016/08/12 08:03:42  jimmyya
change Nand flash as utility

Revision 1.2  2016/04/20 11:25:27  benchen2
add tachi fru portion

Revision 1.1.2.15  2016/03/03 09:48:09  jimmyya
Add Lewis I2C test

Revision 1.1.2.14  2016/02/16 23:41:15  jskow
Add enhanced error messaging to Lewis GESW

Revision 1.1.2.13  2016/02/04 20:56:56  jskow
Update GESW show_version utility

Revision 1.1.2.10  2016/01/20 01:54:55  jskow
Fix GESW nc listening process to be killed on error so that stop on error flag works

Revision 1.1.2.9  2016/01/19 20:35:37  jskow
Update GESW mid-test info to be self contained


Revision 1.1.2.8  2016/01/15 05:54:43  jimmyya
Remove the child process for nc listening

Revision 1.1.2.7  2016/01/14 02:27:51  jskow
Fix missing file close in lewis_gesw_pass_file

Revision 1.1.2.5  2016/01/13 09:07:20  jimmyya
use do_all_meun to execute do all test

Revision 1.1.2.4  2016/01/12 07:36:22  jimmyya
move BTB test to diag_testcard_test.c

$Endlog$
*/
