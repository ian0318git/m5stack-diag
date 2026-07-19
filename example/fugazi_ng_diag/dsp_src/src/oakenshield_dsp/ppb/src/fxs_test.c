/* $Id: fxs_test.c,v 1.5 2021/04/15 00:53:07 achiu2 Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/ppb/src/fxs_test.c,v $ 
 *------------------------------------------------------------------
 * fxs_test.c
 * Tests for FXS
 *
 * Oct 2016 - Owen Lin
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

#include <stdint.h>
#include "stdio.h"
#include "string.h"
#include "types.h"
#include "menu.h"
#include "diag_ppb.h"
#include "common.h"
#include "error.h"
#include "debug_console.h"
#include "uart.h"
#include "lsi_sp27xx_reg.h"
#include "libgeneric.h"
#include "libuart.h"
#include "fxs_test.h"
#include "diag_fpga.h"
#include "si3xxx_utils.h"
#include "libgpio.h"
#include "tstcodec_si3050.h"

/*===================================================================*
 *                    Function Prototypes                            *
 *===================================================================*/
extern void si32261_reset(boolean reset); /* Control by the FPGA */
extern char get_oakenshield_id(void); /* Get info from the FPGA */
static uint codec_part_number = 0xFF;
int oak_fxs_calibration(int);
int codec_si32261_rd_reg_conti(int);
int codec_si32261_rd_ram_conti(int);
extern int si3226x_init_with_options(int);
extern int si32261_load_patch(int);
extern int si32261_load_patch_cal(int);
int do_calibration_flag = FALSE;
void reset_fxs_codec(void);
int get_fxs_which_codec(uchar, int);

/*===================================================================*
 *                             Globals                               *
 *===================================================================*/

/* Silab part number */
#define SI32261_ID                      0xC0
#define SI32261_PART_NUMBER_MASK        0xF8
#define FXS_MAX_CALIBRATION_PORTS       144
#define SI3226x_PRAM_DATA               (335 + 0x400)

/*===================================================================*
 *                             Function                              *
 *===================================================================*/

/**********************************************************************
 *
 * Function: get_tdm_bus_num
 *
 * Description: This function will show tdm bus which is between fpga and silab chip
 *              when fxs/fxo loopback test fail. And the tdm bus mapping is defined in 
 *              Oakenshield_fpga_hw_function_spec(EDCS-11500650).
 *
 * Input : port - fxs/fxo port
 *         type - type 1 is fxs loopback
 *                type 2 is fxo loopback
 * Returns: tdm_bus - tdm bus number
 *
 *
 **********************************************************************
 */
int get_tdm_bus_num (int port , int type)
{ 
    uchar board_id = 0xFF;
    int tdm_bus;
    board_id = get_oak_id();
    if (type == FXS_TYPE) {
        switch (board_id) {
        case BOARD_16FXS_2FXO:
            if (port >= FXS_PORT0 && port <= FXS_PORT7) {
                tdm_bus = 0;
            } else {
                tdm_bus = 2;
            }
            break;
        case BOARD_24FXS_4FXO:
            if (port >= FXS_PORT0 && port <= FXS_PORT7) {
                tdm_bus = 0;
            } else {
                tdm_bus = 2;
            }
            break;
        case BOARD_8FXS_12FXO:
            if (port >= FXS_PORT0 && port <= FXS_PORT7) {
                tdm_bus = 0;
            }
            break;
        case VG400_2FXS_2FXO:
            if (port >= FXS_PORT0 && port <= FXS_PORT7) {
                tdm_bus = 0;
            } else {
                tdm_bus = 2;
            }
            break;
        case VG400_4FXS_4FXO:
            if (port >= FXS_PORT0 && port <= FXS_PORT7) {
                tdm_bus = 0;
            } else {
                tdm_bus = 2;
            }
            break;
        case VG400_6FXS_6FXO:
            if (port >= FXS_PORT0 && port <= FXS_PORT7) {
                tdm_bus = 0;
            } else {
                tdm_bus = 2;
            }
            break;
        case VG400_8FXS:
            if (port >= FXS_PORT0 && port <= FXS_PORT7) {
                tdm_bus = 0;
            }
            break;
        case BOARD_72FXS:
            if (port >= FXS_PORT0 && port <= FXS_PORT15) {
                tdm_bus = 0;
            } else if (port >= FXS_PORT16 && port <= FXS_PORT31) {
                tdm_bus = 1;
            } else if (port >= FXS_PORT32 && port <= FXS_PORT33) {
                tdm_bus = 2;
            } else if (port >= FXS_PORT34 && port <= FXS_PORT49) {
                tdm_bus = 3;
            } else if (port >= FXS_PORT50 && port <= FXS_PORT65) {
                tdm_bus = 4;
            } else if (port >= FXS_PORT66 && port <= FXS_PORT71) {
                tdm_bus = 5;
            }
            break;
        case PHOENIX_144FXS:
            if (port < FXS_PORT12) {
                tdm_bus = 0;
            } else if (port < FXS_PORT24) {
                tdm_bus = 1;
            } else if (port < FXS_PORT36) {
                tdm_bus = 2;
            } else if (port < FXS_PORT48) {
                tdm_bus = 3;
            } else if (port < FXS_PORT64) {
                tdm_bus = 4;
            } else if (port < FXS_PORT80) {
                tdm_bus = 5;
            } else if (port < FXS_PORT96) {
                tdm_bus = 6;
            } else if (port < FXS_PORT112) {
                tdm_bus = 7;
            } else if (port < FXS_PORT128) {
                tdm_bus = 8;
            } else {
                tdm_bus = 9;
            }
            break;
        case PHOENIX_132FXS_6FXO:
        case PHOENIX_84FXS_6FXO:
            if (port < FXS_PORT12) {
                tdm_bus = 0;
            } else if (port < FXS_PORT24) {
                tdm_bus = 1;
            } else if (port < FXS_PORT36) {
                tdm_bus = 2;
            } else if (port < FXS_PORT52) {
                tdm_bus = 4;
            } else if (port < FXS_PORT68) {
                tdm_bus = 5;
            } else if (port < FXS_PORT84) {
                tdm_bus = 6;
            } else if (port < FXS_PORT100) {
                tdm_bus = 7;
            } else if (port < FXS_PORT116) {
                tdm_bus = 8;
            } else {
                tdm_bus = 9;
            }
            break;
        default:
        tdm_bus = 0xFF;
        printf("Not support this bord id.\n");
        break;
        }
    } else if (type == FXO_TYPE) {   /* type 0 is fxo lpbk */
        switch (board_id) {
        case BOARD_16FXS_2FXO:
        if (port >= FXO_PORT0 && port <= FXO_PORT1) {
            tdm_bus = 1;
        }
        break;
        case BOARD_24FXS_4FXO:
        if (port >= FXO_PORT0 && port <= FXO_PORT3) {
            tdm_bus = 1;
        }
        break;
        case BOARD_8FXS_12FXO:
        if (port >= FXO_PORT0 && port <= FXO_PORT3) {
            tdm_bus = 1;
        } else {
            tdm_bus = 3;
        }
        break;
        case VG400_2FXS_2FXO:
        if (port >= FXO_PORT0 && port <= FXO_PORT1) {
            tdm_bus = 3;
        }
        break;
        case VG400_4FXS_4FXO:
        if (port >= FXO_PORT0 && port <= FXO_PORT3) {
            tdm_bus = 1;
        } else {
            tdm_bus = 3;
        }
        break;
        case VG400_6FXS_6FXO:
        if (port >= FXO_PORT0 && port <= FXO_PORT3) {
            tdm_bus = 1;
        } else {
            tdm_bus = 3;
        }
        break;
        case PHOENIX_132FXS_6FXO:
        case PHOENIX_84FXS_6FXO:
            if (port < FXO_PORT6) {
                tdm_bus = 3;
            } 
            break;
        default:
        tdm_bus = 0xFF;
        printf("Not support this bord id.\n");
        break;
        }

    } else {
        printf("Wrong type of tdm bus,the type should be FXS or FXO.");
    }
    return (tdm_bus);
}



/**********************************************************************
 *
 * Function: load_patch_debug
 *
 * This function will load patch
 *
 * Input : None
 *
 * Returns: PASSED
 *
 *
 **********************************************************************
 */
int load_patch_debug (void)
{
    int port, max_ports, start_port;
    max_ports = get_fxs_port_num();
    uchar board_id = 0xFF;
    reset_fxs_codec();
    board_id = get_oak_id();

    if (board_id == VG400_2FXS_2FXO) {  
         start_port = VG_2FXS_STR_PORT;
    } else {
        start_port = 0;
    }
    for (port = start_port; port < max_ports; port++) {
        if (board_id == PHOENIX_144FXS) {
            if ((port < FXS_PORT24) && phoenix_only_test_dbx_flag) {
                port = FXS_PORT23; /* escape MB FXS port */
                continue;
            } else if ((FXS_PORT24 <= port) && (port < FXS_PORT48) && 
                       (!phoenix_has_dbx(BOARD_DB1_TEST))) {
                port = FXS_PORT47;
                continue;
            } else if ((FXS_PORT48 <= port) && (port < FXS_PORT96) && 
                       (!phoenix_has_dbx(BOARD_DB2_TEST))) {
                port = FXS_PORT95;
                continue;
            } else if ((FXS_PORT96 <= port) && (!phoenix_has_dbx(BOARD_DB3_TEST))) {
                port = max_ports;
                continue;
            }
        } else { /* 132FXS and 84 FXS case */
            if ((port < FXS_PORT24) && phoenix_only_test_dbx_flag) {
                port = FXS_PORT23;
                continue;
            } else if ((FXS_PORT24 <= port) && (port < FXS_PORT36) && 
                       (!phoenix_has_dbx(BOARD_DB1_TEST))) {
                port = FXS_PORT35;
                continue;
            } else if ((FXS_PORT36 <= port) && (port < FXS_PORT84) && 
                       (!phoenix_has_dbx(BOARD_DB2_TEST))) {
                port = FXS_PORT83;
                continue;
            } else if ((FXS_PORT84 <= port) && (!phoenix_has_dbx(BOARD_DB3_TEST))) {
                port = max_ports;
                continue;
            }
        }
        
        bsp_debug_printf("\n\r Port: %d\n", port);

        si32261_protected_mode(port);

        if (port % 2 == 0) {
            si32261_load_patch(port);
        }    
    
    }
    return (PASSED);
}



/**********************************************************************
 *
 * Function: fxs_fxo_led_utility
 *
 * This function will turn on/off LED
 *
 * Input : None
 *
 * Returns: PASSED
 *
 *
 **********************************************************************
 */
int fxs_fxo_led_utility (void)
{
    uint32_t choice = 0, choice_led = 0, reg_data;
    boolean exit_flag = FALSE;
    uchar board_id = 0xFF; 

    board_id = get_oak_id();

    if (!is_vg400() && !is_phoenix()) {
        bsp_debug_printf("\n\r LED test \n");

        fpga_spi_direct_read(FPGA_GENERAL_FXS_FXO_LED, 1, &reg_data);
        fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 1, 1);
        msleep(100);              /* delay for flash led */  
        fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 1, 0);
        msleep(100);              /* delay for flash led */  
        fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 1, 1);
        msleep(100);              /* delay for flash led */ 
        fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 1, 0);
        msleep(100);              /* delay for flash led */ 
        fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 1, reg_data);

        return (PASSED);
    } else {
        if (is_phoenix()) {
            while (!exit_flag) {
                bsp_debug_printf("\r FXS/FXO Port LED Utility \r\n");
                bsp_debug_printf("\r 0. Enable LED \r\n");
                bsp_debug_printf("\r 1. Disable LED \r\n");
                bsp_debug_printf("\r 2. Exit \r\n");

                choice = gethex_answer("Enter selection:", 0, 0, 7);

                switch(choice) {
                case 0:
                    fpga_spi_direct_write(PHOENIX_FPGA_GENERAL_LED, 8, PHOENIX_ALL_LED_EN);
                    break;
                case 1:
                    fpga_spi_direct_write(PHOENIX_FPGA_GENERAL_LED, 8, PHOENIX_ALL_LED_DIS);
                    break;
                case 2:
                    exit_flag = TRUE;
                    break;
                default:
                    bsp_debug_printf("\r Not support this item. \r\n");
                    break;
                }
            }
        } else if (board_id == VG400_8FXS) {
            while (!exit_flag) {
                bsp_debug_printf("\r FXS/FXO Port LED Utility \r\n");
                bsp_debug_printf("\r 0. Enable All Green LED \r\n");
                bsp_debug_printf("\r 1. Enable All RED LED \r\n");
                bsp_debug_printf("\r 2. Disable All LED \r\n");
                bsp_debug_printf("\r 3. Enable FXS Green LED \r\n");
                bsp_debug_printf("\r 4. Enable FXS Red LED \r\n");
                bsp_debug_printf("\r 5. Exit \r\n");

                choice = gethex_answer("Enter selection:", 0, 0, 5);
                switch(choice) {
                case 0:
                    fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, ALL_LEDGRN_EN);
                    break;
                case 1: 
                    fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, ALL_REDLED_EN);
                    break;
                case 2:
                    fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, ALL_LED_DIS);
                    break;
                case 3:
                    while (!exit_flag) {
                        bsp_debug_printf("\r 0. Enable FXS port 0 Green LED \r\n");
                        bsp_debug_printf("\r 1. Enable FXS port 1 Green LED \r\n");
                        bsp_debug_printf("\r 2. Enable FXS port 2 Green LED \r\n");
                        bsp_debug_printf("\r 3. Enable FXS port 3 Green LED \r\n");
                        bsp_debug_printf("\r 4. Enable FXS port 4 Green LED \r\n");
                        bsp_debug_printf("\r 5. Enable FXS port 5 Green LED \r\n");
                        bsp_debug_printf("\r 6. Enable FXS port 6 Green LED \r\n");
                        bsp_debug_printf("\r 7. Enable FXS port 7 Green LED \r\n");
                        bsp_debug_printf("\r 8. Exit \r\n");

                    choice_led = gethex_answer("Enter selection:", 0, 0, 8);
                    switch(choice_led) {
                    case 0:
                        fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, FXSP0_LEDGRN);
                        break;
                    case 1:
                        fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, FXSP1_LEDGRN);
                        break;
                    case 2:
                        fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, FXSP2_LEDGRN);
                        break;
                    case 3:
                        fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, FXSP3_LEDGRN);
                        break;
                    case 4:
                        fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, FXSP4_LEDGRN);
                        break;
                    case 5:
                        fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, FXSP5_LEDGRN);
                        break;
                    case 6:
                        fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, FXSP6_LEDGRN);
                        break;
                    case 7:
                        fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, FXSP7_LEDGRN);
                        break;
                    case 8:
                        exit_flag = TRUE;
                        break;
                        }
                    }

                case 4:
                    while (!exit_flag) {
                        bsp_debug_printf("\r 0. Enable FXS port 0 Red LED \r\n");
                        bsp_debug_printf("\r 1. Enable FXS port 1 Red LED \r\n");
                        bsp_debug_printf("\r 2. Enable FXS port 2 Red LED \r\n");
                        bsp_debug_printf("\r 3. Enable FXS port 3 Red LED \r\n");
                        bsp_debug_printf("\r 4. Enable FXS port 4 Red LED \r\n");
                        bsp_debug_printf("\r 5. Enable FXS port 5 Red LED \r\n");
                        bsp_debug_printf("\r 6. Enable FXS port 6 Red LED \r\n");
                        bsp_debug_printf("\r 7. Enable FXS port 7 Red LED \r\n");
                        bsp_debug_printf("\r 8. Exit \r\n");

                        choice_led = gethex_answer("Enter selection:", 0, 0, 8);
                        switch(choice_led) {
                        case 0:
                            fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, FXSP0_LEDRED);
                            break;
                        case 1:
                            fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, FXSP1_LEDRED);
                            break;
                        case 2:
                            fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, FXSP2_LEDRED);
                            break;
                        case 3:
                            fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, FXSP3_LEDRED);
                            break;
                        case 4:
                            fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, FXSP4_LEDRED);
                            break;
                        case 5:
                            fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, FXSP5_LEDRED);
                            break;
                        case 6:
                            fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, FXSP6_LEDRED);
                            break;
                        case 7:
                            fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, FXSP7_LEDRED);
                            break;
                        case 8:
                             exit_flag = TRUE;
                            break;
                        }
                    }

                case 5:
                    exit_flag = TRUE;
                    break;
                default:
                    bsp_debug_printf("\r Not support this item. \r\n");
                    break;
                }  
            }
        } else {
            while (!exit_flag) {
                bsp_debug_printf("\r FXS/FXO Port LED Utility \r\n");
                bsp_debug_printf("\r 0. Enable All Green LED \r\n");
                bsp_debug_printf("\r 1. Enable All RED LED \r\n");
                bsp_debug_printf("\r 2. Disable All LED \r\n");
                bsp_debug_printf("\r 3. Enable FXS Green LED \r\n");
                bsp_debug_printf("\r 4. Enable FXS Red LED \r\n");
                bsp_debug_printf("\r 5. Enable FXO Green LED \r\n");
                bsp_debug_printf("\r 6. Enable FXO Red LED \r\n");
                bsp_debug_printf("\r 7. Exit \r\n");

                choice = gethex_answer("Enter selection:", 0, 0, 7);

                switch(choice) {
                case 0:
                    fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, ALL_LEDGRN_EN);
                    break;
                case 1: 
                    fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, ALL_REDLED_EN);
                    break;
                case 2:
                    fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, ALL_LED_DIS);
                    break;
                case 3:
                    if (board_id == VG400_6FXS_6FXO) {
                        while (!exit_flag) {
                            bsp_debug_printf("\r 0. Enable FXS port 0 Green LED \r\n");
                            bsp_debug_printf("\r 1. Enable FXS port 1 Green LED \r\n");
                            bsp_debug_printf("\r 2. Enable FXS port 2 Green LED \r\n");
                            bsp_debug_printf("\r 3. Enable FXS port 3 Green LED \r\n");
                            bsp_debug_printf("\r 4. Enable FXS port 4 Green LED \r\n");
                            bsp_debug_printf("\r 5. Enable FXS port 5 Green LED \r\n");
                            bsp_debug_printf("\r 6. Exit \r\n");

                            choice_led = gethex_answer("Enter selection:", 0, 0, 6);
                            switch(choice_led) {
                            case 0:
                                fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, FXSP0_LEDGRN);
                                break;
                            case 1:
                                fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, FXSP1_LEDGRN);
                                break;
                            case 2:
                                fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, FXSP2_LEDGRN);
                                break;
                            case 3:
                                fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, FXSP3_LEDGRN);
                                break;
                            case 4:
                                fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, FXSP4_LEDGRN);
                                break;
                            case 5:
                                fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, FXSP5_LEDGRN);
                                break;
                            case 6:
                                exit_flag = TRUE;
                                break;
                            }
                        }
                    } else if (board_id == VG400_4FXS_4FXO) {
                        while (!exit_flag) {
                            bsp_debug_printf("\r 0. Enable FXS port 0 Green LED \r\n");
                            bsp_debug_printf("\r 1. Enable FXS port 1 Green LED \r\n");
                            bsp_debug_printf("\r 2. Enable FXS port 2 Green LED \r\n");
                            bsp_debug_printf("\r 3. Enable FXS port 3 Green LED \r\n");
                            bsp_debug_printf("\r 4. Exit \r\n");

                            choice_led = gethex_answer("Enter selection:", 0, 0, 4);
                            switch(choice_led) {
                            case 0:
                                fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, FXSP0_LEDGRN);
                                break;
                            case 1:
                                fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, FXSP1_LEDGRN);
                                break;
                            case 2:
                                fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, FXSP2_LEDGRN);
                                break;
                            case 3:
                                fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, FXSP3_LEDGRN);
                                break;
                            case 4:
                                exit_flag = TRUE;
                                break;
                            }
                         }
                    } else if (board_id == VG400_2FXS_2FXO) {
                        while (!exit_flag) {
                            bsp_debug_printf("\r 0. Enable FXS port 0 Green LED \r\n");
                            bsp_debug_printf("\r 1. Enable FXS port 1 Green LED \r\n");
                            bsp_debug_printf("\r 2. Exit \r\n");

                            choice_led = gethex_answer("Enter selection:", 0, 0, 2);
                            switch(choice_led) {
                            case 0:
                                fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, FXSP4_LEDGRN);
                                break;
                            case 1:
                                fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, FXSP5_LEDGRN);
                                break;
                            case 2:
                                exit_flag = TRUE;
                                break;
                            }
                        }
                    }
                case 4:
                    if (board_id == VG400_6FXS_6FXO) {
                        while (!exit_flag) {
                            bsp_debug_printf("\r 0. Enable FXS port 0 Red LED \r\n");
                            bsp_debug_printf("\r 1. Enable FXS port 1 Red LED \r\n");
                            bsp_debug_printf("\r 2. Enable FXS port 2 Red LED \r\n");
                            bsp_debug_printf("\r 3. Enable FXS port 3 Red LED \r\n");
                            bsp_debug_printf("\r 4. Enable FXS port 4 Red LED \r\n");
                            bsp_debug_printf("\r 5. Enable FXS port 5 Red LED \r\n");
                            bsp_debug_printf("\r 6. Exit \r\n");

                            choice_led = gethex_answer("Enter selection:", 0, 0, 6);
                            switch(choice_led) {
                            case 0:
                                fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, FXSP0_LEDRED);
                                break;
                            case 1:
                                fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, FXSP1_LEDRED);
                                break;
                            case 2:
                                fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, FXSP2_LEDRED);
                                break;
                            case 3:
                                fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, FXSP3_LEDRED);
                                break;
                            case 4:
                                fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, FXSP4_LEDRED);
                                break;
                            case 5:
                                fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, FXSP5_LEDRED);
                                break;
                            case 6:
                                exit_flag = TRUE;
                                break;
                            }
                        }
                    } else if (board_id == VG400_4FXS_4FXO) {
                        while (!exit_flag) {
                            bsp_debug_printf("\r 0. Enable FXS port 0 Red LED \r\n");
                            bsp_debug_printf("\r 1. Enable FXS port 1 Red LED \r\n");
                            bsp_debug_printf("\r 2. Enable FXS port 2 Red LED \r\n");
                            bsp_debug_printf("\r 3. Enable FXS port 3 Red LED \r\n");
                            bsp_debug_printf("\r 4. Exit \r\n");

                            choice_led = gethex_answer("Enter selection:", 0, 0, 4);
                            switch(choice_led) {
                            case 0:
                                fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, FXSP0_LEDRED);
                                break;
                            case 1:
                                fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, FXSP1_LEDRED);
                                break;
                            case 2:
                                fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, FXSP2_LEDRED);
                                break;
                            case 3:
                                fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, FXSP3_LEDRED);
                                break;
                            case 4:
                                exit_flag = TRUE;
                                break;
                            }
                        }
                    } else if (board_id == VG400_2FXS_2FXO) {
                        while (!exit_flag) {
                            bsp_debug_printf("\r 0. Enable FXS port 0 Red LED \r\n");
                            bsp_debug_printf("\r 1. Enable FXS port 1 Red LED \r\n");
                            bsp_debug_printf("\r 2. Exit \r\n");

                            choice_led = gethex_answer("Enter selection:", 0, 0, 2);
                            switch(choice_led) {
                            case 0:
                                fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, FXSP4_LEDRED);
                                break;
                            case 1:
                                fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, FXSP5_LEDRED);
                                break;
                            case 2:
                                exit_flag = TRUE;
                                break;
                            }
                        }
                    }

                case 5:
                    if (board_id == VG400_6FXS_6FXO) {
                        while (!exit_flag) {
                            bsp_debug_printf("\r 0. Enable FXO port 0 Green LED \r\n");
                            bsp_debug_printf("\r 1. Enable FXO port 1 Green LED \r\n");
                            bsp_debug_printf("\r 2. Enable FXO port 2 Green LED \r\n");
                            bsp_debug_printf("\r 3. Enable FXO port 3 Green LED \r\n");
                            bsp_debug_printf("\r 4. Enable FXO port 4 Green LED \r\n");
                            bsp_debug_printf("\r 5. Enable FXO port 5 Green LED \r\n");
                            bsp_debug_printf("\r 6. Exit \r\n");

                            choice_led = gethex_answer("Enter selection:", 0, 0, 6);
                            switch(choice_led) {
                            case 0:
                                fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, FXOP0_LEDGRN);
                                break;
                            case 1:
                                fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, FXOP1_LEDGRN);
                                break;
                            case 2:
                                fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, FXOP2_LEDGRN);
                                break;
                            case 3:
                                fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, FXOP3_LEDGRN);
                                break;
                            case 4:
                                fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, FXOP4_LEDGRN);
                                break;
                            case 5:
                                fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, FXOP5_LEDGRN);
                                break;
                            case 6:
                                exit_flag = TRUE;
                                break;
                            }
                        }
                    } else if (board_id == VG400_4FXS_4FXO) {
                        while (!exit_flag) {
                            bsp_debug_printf("\r 0. Enable FXO port 0 Green LED \r\n");
                            bsp_debug_printf("\r 1. Enable FXO port 1 Green LED \r\n");
                            bsp_debug_printf("\r 2. Enable FXO port 2 Green LED \r\n");
                            bsp_debug_printf("\r 3. Enable FXO port 3 Green LED \r\n");
                            bsp_debug_printf("\r 4. Exit \r\n");

                            choice_led = gethex_answer("Enter selection:", 0, 0, 4);
                            switch(choice_led) {
                            case 0:
                                fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, FXOP0_LEDGRN);
                                break;
                            case 1:
                                fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, FXOP1_LEDGRN);
                                break;
                            case 2:
                                fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, FXOP2_LEDGRN);
                                break;
                            case 3:
                                fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, FXOP3_LEDGRN);
                                break;
                            case 4:
                                exit_flag = TRUE;
                                break;
                            }
                        }
                    } else if (board_id == VG400_2FXS_2FXO) {
                        while (!exit_flag) {
                            bsp_debug_printf("\r 0. Enable FXO port 0 Green LED \r\n");
                            bsp_debug_printf("\r 1. Enable FXO port 1 Green LED \r\n");
                            bsp_debug_printf("\r 2. Exit \r\n");

                            choice_led = gethex_answer("Enter selection:", 0, 0, 2);
                            switch(choice_led) {
                            case 0:
                                fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, FXOP4_LEDGRN);
                                break;
                            case 1:
                                fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, FXOP5_LEDGRN);
                                break;
                            case 2:
                                exit_flag = TRUE;
                                break;
                            }
                        }
                    }
                case 6:
                    if (board_id == VG400_6FXS_6FXO) {
                        while (!exit_flag) {
                            bsp_debug_printf("\r 0. Enable FXO port 0 Red LED \r\n");
                            bsp_debug_printf("\r 1. Enable FXO port 1 Red LED \r\n");
                            bsp_debug_printf("\r 2. Enable FXO port 2 Red LED \r\n");
                            bsp_debug_printf("\r 3. Enable FXO port 3 Red LED \r\n");
                            bsp_debug_printf("\r 4. Enable FXO port 4 Red LED \r\n");
                            bsp_debug_printf("\r 5. Enable FXO port 5 Red LED \r\n");
                            bsp_debug_printf("\r 6. Exit \r\n");

                            choice_led = gethex_answer("Enter selection:", 0, 0, 6);
                            switch(choice_led) {
                            case 0:
                                fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, FXOP0_LEDRED);
                                break;
                            case 1:
                                fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, FXOP1_LEDRED);
                                break;
                            case 2:
                                fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, FXOP2_LEDRED);
                                break;
                            case 3:
                                fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, FXOP3_LEDRED);
                                break;
                            case 4:
                                fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, FXOP4_LEDRED);
                                break;
                            case 5:
                                fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, FXOP5_LEDRED);
                                break;
                            case 6:
                                exit_flag = TRUE;
                                break;
                            }
                        }
                    } else if (board_id == VG400_4FXS_4FXO) {
                        while (!exit_flag) {
                            bsp_debug_printf("\r 0. Enable FXO port 0 Red LED \r\n");
                            bsp_debug_printf("\r 1. Enable FXO port 1 Red LED \r\n");
                            bsp_debug_printf("\r 2. Enable FXO port 2 Red LED \r\n");
                            bsp_debug_printf("\r 3. Enable FXO port 3 Red LED \r\n");
                            bsp_debug_printf("\r 4. Exit \r\n");

                            choice_led = gethex_answer("Enter selection:", 0, 0, 4);
                            switch(choice_led) {
                            case 0:
                                fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, FXOP0_LEDRED);
                                break;
                            case 1:
                                fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, FXOP1_LEDRED);
                                break;
                            case 2:
                                fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, FXOP2_LEDRED);
                                break;
                            case 3:
                                fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, FXOP3_LEDRED);
                                break;
                            case 4:
                                exit_flag = TRUE;
                                break;
                            }
                        }
                    } else if (board_id == VG400_2FXS_2FXO) {
                        while (!exit_flag) {
                            bsp_debug_printf("\r 0. Enable FXO port 0 Red LED \r\n");
                            bsp_debug_printf("\r 1. Enable FXO port 1 Red LED \r\n");
                            bsp_debug_printf("\r 2. Exit \r\n");

                            choice_led = gethex_answer("Enter selection:", 0, 0, 2);
                            switch(choice_led) {
                            case 0:
                                fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, FXOP4_LEDRED);
                                break;
                            case 1:
                                fpga_spi_direct_write(FPGA_GENERAL_FXS_FXO_LED, 8, FXOP5_LEDRED);
                                break;
                            case 2:
                                exit_flag = TRUE;
                                break;
                            }
                        }
                    }
                case 7:
                    exit_flag = TRUE;
                    break;
                default:
                    bsp_debug_printf("\r Not support this item. \r\n");
                    break;
                }
            }
        }
    }
    return (PASSED);
}


/**********************************************************************
 *
 * Function: get_fxs_port_num
 *
 * This function will get fxs port number
 *
 * Input : None
 *
 * Returns: Number of FXS port
 *
 *
 **********************************************************************
 */
int get_fxs_port_num(void)
{
    uchar board_id = 0xFF;
    int num_port;
    board_id = get_oak_id();

    switch (board_id) {
    case BOARD_16FXS_2FXO:
        num_port = 16;
        break;
    case BOARD_24FXS_4FXO:
        num_port = 24;
        break;
    case BOARD_8FXS_12FXO:
        num_port = 8;
        break;
    case BOARD_72FXS:
        num_port = 72;
        break;
    case VG400_2FXS_2FXO:
        num_port = 6;
        break;
    case VG400_4FXS_4FXO:
        num_port = 4;
        break;
    case VG400_6FXS_6FXO:
        num_port = 6;
        break;
    case VG400_8FXS:
        num_port = 8;
        break;
    case PHOENIX_144FXS:
        num_port = 144;
        break;
    case PHOENIX_132FXS_6FXO:
        num_port = 132;
        break;
    case PHOENIX_84FXS_6FXO:
        num_port = 84;
        break;
    default:
        num_port = 0xFF;
        break;
    }

    return (num_port);

}



/**********************************************************************
 *  Function:  reset_fxs_codec
 *
 *  Description: Reset FXS Codec
 *
 *  Input: None
 *
 *  Returns: None
 *
 * ********************************************************************
 */
void reset_fxs_codec(void)
{
    /* FPGA to reset the Codec */
    si32261_reset(TRUE);
    /* Reset pulse 33/PCLK per data sheet. 2 MHz PCLK = 16.5 us */
    lsi_mg_delay(20);     
    si32261_reset(FALSE);
    lsi_mg_delay(100); /* Init time from 1 us to 100 us */
}


/**********************************************************************
 *
 * Function: is_si32261
 *
 * This routine checks if the codec is SI32261.
 *
 * Input : none
 * Output: TRUE/FALSE
 *
 **********************************************************************
 */
boolean is_si32261 (void)
{
    /* Read Codec ID */
    if (codec_part_number == 0xFF) {
        si_read_reg_32261(0, 0, &codec_part_number); /* port 0  and reg 0 is chip ID*/
    }

    bsp_debug_printf("\n\r codec_part_number: %x \n", codec_part_number);

    if ((codec_part_number & SI32261_PART_NUMBER_MASK) == SI32261_ID) {
        return (TRUE);
    }

    return (FALSE);
}

/**********************************************************************
 *  Function: silab_fxs_lpbk_test
 *
 *  Description: Loopback test from DSP->FPGA->FXS codec and back.
 *           This function is wrapper for the test, which tests
 *           DSP-TDM loopbacks.  The DSP will send a packet to
 *           the fxs through the tdm switch. The codec on the FXS is
 *           programmed in loopback mode. The packet is looped back
 *           from the codec through the TDM switch back to DSP.
 *           Example: TDM connection for DSP :
 *           DSP ->TDM in stream 23->out stream 4->in stream 4->
 *           out stream 23.
 *
 *  Input: none
 *
 *  Returns:  PASSED or FAILED
 * ******************************************************************** 
 */
int silab_fxs_lpbk_test (void)
{
    int port, tdm_bus, start_port, num_port = 0;
    char errstr[128];
    uchar board_id = 0xFF;
    num_port = get_fxs_port_num();

    if (num_port == 0xff) {
        cterr('f', 0,"Unknown Oakenshield FXS present");
        sprintf((char *)&(hd_if->errmsg), "\nUnknown Oakenshield FXS present\n");
        return(FAILED);
    }

    PRINT_STR("\r Oakenshield FXS CODEC loopback Test\r\r");

    /*
     * FXS has no power on control, but there is register CRR to reset the chip
     */
    reset_fxs_codec();
    board_id = get_oak_id();

    if (board_id ==  VG400_2FXS_2FXO) {
        start_port = VG_2FXS_STR_PORT;
    } else {
        start_port = 0;
    }
    for (port = start_port; port < num_port; port++) {
        /* Phoenix need separate DBx and except no exist which_port */
        if (is_phoenix()) {
            if (board_id == PHOENIX_144FXS) {
                if ((port < FXS_PORT24) && phoenix_only_test_dbx_flag) {
                    port = FXS_PORT23; /* escape MB FXS port */
                    continue;
                } else if ((FXS_PORT24 <= port) && (port < FXS_PORT48) && 
                           (!phoenix_has_dbx(BOARD_DB1_TEST))) {
                    port = FXS_PORT47;
                    continue;
                } else if ((FXS_PORT48 <= port) && (port < FXS_PORT96) && 
                           (!phoenix_has_dbx(BOARD_DB2_TEST))) {
                    port = FXS_PORT95;
                    continue;
                } else if ((FXS_PORT96 <= port) && (!phoenix_has_dbx(BOARD_DB3_TEST))) {
                    port = num_port;
                    continue;
                }
            } else { /* 132FXS and 84 FXS case */
                if ((port < FXS_PORT24) && phoenix_only_test_dbx_flag) {
                    port = FXS_PORT23;
                    continue;
                } else if ((FXS_PORT24 <= port) && (port < FXS_PORT36) && 
                           (!phoenix_has_dbx(BOARD_DB1_TEST))) {
                    port = FXS_PORT35;
                    continue;
                } else if ((FXS_PORT36 <= port) && (port < FXS_PORT84) && 
                           (!phoenix_has_dbx(BOARD_DB2_TEST))) {
                    port = FXS_PORT83;
                    continue;
                } else if ((FXS_PORT84 <= port) && (!phoenix_has_dbx(BOARD_DB3_TEST))) {
                    port = num_port;
                    continue;
                }
            }
        }
        
        PRINT_STR("\r\r");
        PRINT_STR("FXS Codec Digital lpbk. port: ");
        PRINT_DEC(port);
                          
        if (si32261_codec_digital_loopback(port) == FAILED) {
            tdm_bus = get_tdm_bus_num(port, FXS_TYPE);
            sprintf(errstr, "\n Fail lpbk port%d (TDM bus%d)\n", port, tdm_bus);
            strcat((char *)&(hd_if->errmsg), errstr);
            cterr('f', 0,"SI32261 Codec Loopback test Failed\n");
            return (FAILED);
        }
    }
    

    sprintf((char *)&(hd_if->bufmsg), "FXS Loopback test done!\n"); 

    return (PASSED);
}

/**********************************************************************
 *
 * Function: codec_si32261_rd_reg_conti
 *
 * Description: This send command to SI32261 to read register.
 *
 * Input : port
 *
 * Output:  PASSED/FAILED
 *
 *********************************************************************
*/
int codec_si32261_rd_reg_conti (int port)
{
    int ret_code, ix;
    uint reg_num_first, reg_num_last, data;

    reg_num_first = gethex_answer("\nEnter first reg# to read",0,0,127);

    reg_num_last = gethex_answer("\nEnter last reg# to read",0,0,127);

    for (ix = reg_num_first; ix <= reg_num_last; ix++) {
        data = 0; 
        ret_code = si_read_reg_32261(port, ix, &data);

        bsp_debug_printf("\n\r Reg %d read is 0x%x \r", ix, data&0xff);
    }

    return (ret_code);
}


/**********************************************************************
 *
 * Function: codec_si32261_read_write_reg
 *
 * Description: This send command to SI32261 to read or write register.
 *
 * Input : port, read/write flag
 *
 * Output:  PASSED/FAILED
 *
 *********************************************************************
*/
int codec_si32261_read_write_reg (int port, int rd_wt_flg)
{
    int ret_code;
    uint reg_num, data;
    uchar board_id = 0xFF;
    
    board_id = get_oak_id();
    reg_num = gethex_answer("\nEnter reg# to read/write",0,0,127);
    if (rd_wt_flg == TRUE) {
        if (board_id == VG400_2FXS_2FXO) {
            ret_code = si_read_reg_32261(port + VG_2FXS_STR_PORT, reg_num, &data);
        } else {
            ret_code = si_read_reg_32261(port, reg_num, &data);
        }
    } else {
        data = gethex_answer("\nEnter data to write",0,0,0xff);
        if (board_id == VG400_2FXS_2FXO) {
            ret_code = si_write_reg_32261(port + VG_2FXS_STR_PORT, reg_num, data);
        } else {
            ret_code = si_write_reg_32261(port, reg_num, data);
        }
    }

    if (ret_code != PASSED) {
        cterr('f',0,"Codec port %d - %s reg %x failed", port,
              rd_wt_flg ? "read" : "write", reg_num);
    } else {
        if (rd_wt_flg == 1) {
            PRINT_STR("\nreg read is : ");
            PRINT_HEX(data & 0xff);
        }
    }

    return (ret_code);
}

/**********************************************************************
 *
 * Function: codec_si32261_set_stop_onoff_hook_map
 *
 * Description: This send command to SI32261 to set or stop on-off hook mapping.
 *
 * Input : port, set/stop mapping
 *
 * Output:  PASSED/FAILED
 *
 *********************************************************************
*/
int codec_si32261_set_stop_onoff_hook_map (int port, int set_stop_flg)
{
    int ret_code, which_codec, max_fxs_port;
    char errstr[128];
    uchar board_id = 0xFF;

    board_id = get_oak_id();

    max_fxs_port = get_fxs_port_num();
    if (set_stop_flg == 1) {
        bsp_debug_printf("\n\r Will set one codec once \r");

        which_codec = get_fxs_which_codec(board_id, port);

        bsp_debug_printf("\n\r Set Codec %d to ON-OFF hook mapping \r", which_codec);
        ret_code = si32261_codec_set_onoff_hook_map(which_codec);
    } else {
        ret_code = si32261_codec_stop_ring(port);
    }

    if (ret_code != PASSED) {
        cterr('f',0,"Codec port %d %s on-off hook mapping failed.", port,
                     set_stop_flg ? "set" : "stop");
        sprintf(errstr, "\nport %d %s on-off hook mapping fail\n", port,
                     set_stop_flg ? "set" : "stop");
        strcat((char *)&(hd_if->errmsg), errstr);
    }

    sprintf((char *)&(hd_if->bufmsg), "Codec port %d %s on-off hook mapping success", port,\
            set_stop_flg ? "set" : "stop");

    return (ret_code);
}


/**********************************************************************
 *
 * Function: codec_si32261_set_stop_ring
 *
 * Description: This send command to SI32261 to set or stop ring.
 *
 * Input : port, set/stop ring flag
 *
 * Output:  PASSED/FAILED
 *
 *********************************************************************
*/
int codec_si32261_set_stop_ring (int port, int set_stop_flg)
{
    int ret_code, which_codec, max_fxs_port;
    char errstr[128]; 
    uchar board_id = 0xFF;

    board_id = get_oak_id();

    max_fxs_port = get_fxs_port_num();
    if (set_stop_flg == 1) {
        bsp_debug_printf("\n\r Will set one codec once \r");

        which_codec = get_fxs_which_codec(board_id, port);

        bsp_debug_printf("\n\r Set Codec %d to Ring \r", which_codec);
        ret_code = si32261_codec_set_ring(which_codec);
    } else {
        ret_code = si32261_codec_stop_ring(port);
    }

    if (ret_code != PASSED) {
        cterr('f',0,"Codec port %d %s ring failed.", port,
                     set_stop_flg ? "set" : "stop");
        sprintf(errstr, "\nport %d %s ring fail\n", port, 
                     set_stop_flg ? "set" : "stop");
        strcat((char *)&(hd_if->errmsg), errstr);
    } 

    sprintf((char *)&(hd_if->bufmsg), "Codec port %d %s ring success", port,\
            set_stop_flg ? "set" : "stop");

    return (ret_code);
}


/**********************************************************************
 *
 * Function: codec_si32261_ring_si3050_test 
 *
 * Description: This send command to SI32261 to set ring to 
 *              test Si3050 ring ports.
 *
 * Input : port
 *
 * Output:  PASSED/FAILED
 *
 *********************************************************************
*/
int codec_si32261_ring_si3050_test (int port)
{
    int  ret_code, which_codec, max_fxs_port;
    int  data,  wait_time;
    uchar board_id;
    char errstr[128]; 

    max_fxs_port = get_fxs_port_num();
    board_id = get_oak_id();
    bsp_debug_printf("\n\r Please use swapped wire for this test\r"); 
    bsp_debug_printf("\n\r Will set one codec once \r");
    
    if (is_phoenix()) {
        which_codec = get_fxs_which_codec(board_id, port);
    } else {
        which_codec = port/2;
    }
    bsp_debug_printf("\n\r Set Codec %d to Ring \r", which_codec); 
    ret_code = si32261_codec_set_ring(which_codec);

    if (ret_code != PASSED) {
        cterr('f',0,"Codec port %d ring failed.", port);
        sprintf(errstr, "\n port %d ring fail\n", port);
        strcat((char *)&(hd_if->errmsg), errstr);  
    }

    sprintf((char *)&(hd_if->bufmsg), "Codec port %d ring success", port);

    reset_si3050();     /* Reset si3050 chip*/

    /* Set Reg 24 to 0x0, only use positive ring for detection */
    ret_code = si3050_reg_write(port, REG_RING_VAL, 0x0);
    ret_code = si3050_reg_read(port, REG_RING_VAL, &data);

    bsp_debug_printf("\n\r Please plug in wire on FXO port : %d \r", port);
    bsp_debug_printf("\n\r Ring testing on FXO port : %d\r ", port);

    wait_time = TIME_DETECT_RING;
    do {
        ret_code = si3050_reg_read(port, REG_DAA_CONTROL, &data);
        if(data == POS_RING_DETECT) {
            bsp_debug_printf("\n\r FXO port : %d ring test passed\r ", port);
            break;
        }
        if (wait_time == 0 ) {
            ret_code = si3050_reg_read(port, REG_DAA_CONTROL, &data);
            bsp_debug_printf("\n\r port %d reg5 value is : 0x%2x\r",port, data & 0xff);
            cterr('f', 0, "FXO %d port ring test fail", port);
        }
    } while (wait_time--);

    return (ret_code); 
}


/**********************************************************************
 *
 * Function: si32261_codec_rd_ram_conti
 *
 * Description: This send command to SI32261 to read Ram continue
 *
 * Input : port
 *
 * Output:  PASSED/FAILED
 *
 *********************************************************************
*/
int codec_si32261_rd_ram_conti (int port)
{
    int reg_num_first, reg_num_last, ret_val, ram_data;
    uint16_t data, data_lo;
    int ix;

    reg_num_first = gethex_answer("\nEnter first RAM addr to read",0,0,0x07ff);
    reg_num_last = gethex_answer("\nEnter last RAM addr to read",0,0,0x07ff);


    for (ix = reg_num_first; ix <= reg_num_last; ix++) {
        ret_val = si_read_ram_32261(port, ix, &data, &data_lo);

        ram_data = (int)data;

        bsp_debug_printf("\n\r RAM %d read is  0x%x \r", ix, (ram_data << 16) | data_lo);
    }

    return (ret_val);
}

/**********************************************************************
 *
 * Function: si32261_codec_rd_wr_ram
 *
 * Description: This send command to SI32261 to read or write RAM.
 *
 * Input : port, rd_wr_flg
 *
 * Output:  PASSED/FAILED
 *
 *********************************************************************
*/
int codec_si32261_rd_wt_ram (int port, int rd_wt_flg)
{
    int reg_num, ret_val, ram_data;
    uint16_t data_hi, data_lo;
    int read_time, ix, pram_data;

    reg_num = gethex_answer("\nEnter addr to read/write",0,0,0x07ff);


    if ((reg_num == (SI3226x_PRAM_DATA)) && (rd_wt_flg == TRUE)) {
        read_time = gethex_answer("\n read how many data back",0,0,0x400);
        for (ix = 0; ix <= read_time; ix++) {
            ret_val = si_read_ram_32261(port, reg_num, &data_hi, &data_lo);
            ram_data = (int)data_hi;
            pram_data = ((ram_data << 16) | data_lo);
            bsp_debug_printf("\n\r RAM read is %d \r", pram_data >> 9);
        }
        return (ret_val);
    }

    if (rd_wt_flg == TRUE) {
        ret_val = si_read_ram_32261(port, reg_num, &data_hi, &data_lo);
    } else {
        data_hi = gethex_answer("\nEnter high data to write",0,0,0x1fff);
        data_lo = gethex_answer("\nEnter low data to write",0,0,0xffff);

        ret_val = si_write_ram_32261(port, reg_num, data_hi, data_lo);
    }

    ram_data = (int)data_hi;

    if (ret_val != PASSED) {
        cterr('f',0,"Codec port %d %s RAM @ %x failed.", port,
                     rd_wt_flg ? "read" : "write", reg_num);
    } else {
        if (rd_wt_flg == 1) {
            bsp_debug_printf("\r\n RAM read is  0x%x ", (ram_data << 16) | data_lo);
        }
    }
    return (ret_val);
}

/**********************************************************************
 *
 * Function: codec_read_rev
 *
 * Description: This send command to 32261 to get its rev.
 *
 * Input : port
 *
 * Output:  PASSED/FAILED
 *
 *********************************************************************
*/
int codec_read_rev (int port)
{
    int ret_code;
    uint rev_code;

    ret_code = si_read_reg_32261(port, 0, &rev_code); /* chip ID register address is 0*/

    if (ret_code != PASSED) {
        cterr('f',0,"port %d SI3241 Read Codec Rev command failed.", port);
    } else {
       if (((rev_code & 0xff) & SI32261_PART_NUMBER_MASK) == SI32261_ID) {
           PRINT_STR("\rSI32261 Rev is ");
           PRINT_HEX(rev_code & 0x7);
       } else {
           cterr('f',0,"Wrong chip the ID value is %x", rev_code);
           ret_code = FAILED;
	   }
    }

    return (ret_code);
}

/**********************************************************************
 *
 * Function: codec_si32261_protected
 *
 * Description: This to enable protected mode for higher ram RD
 *
 * Input : port
 *
 * Output:. PASSED/FAILED
 *
 *********************************************************************
*/
int codec_si32261_protected (int port)
{
    return (si32261_protected_mode(port));
}


/**********************************************************************
 *
 * Function: si32261_codec_utilies
 *
 * Description: This test .ll the utilities
 *
 * Input : None
 *
 * Output:  PASSED/FAILED
 *
 *********************************************************************
*/
int si32261_codec_utilities(void)
{
    int tst_item, do_utility = TRUE;
    int data, port, max_ports;
    int disp_menu = TRUE;

    max_ports = get_fxs_port_num();
    if (max_ports == 0xff) {
        cterr('f', 0,"Unknown Oakenshield FXS present");
        return(FAILED);
    }

    while (do_utility == TRUE) {

        if (disp_menu == TRUE) {
            PRINT_STR("\r\r");
            PRINT_STR("1. SI32261 FXS Codec Reg Read\r");
            PRINT_STR("2. SI32261 FXS Codec Reg Write\r");
            PRINT_STR("3. SI32261 Set Codec Ring\r");
            PRINT_STR("4. SI32261 Stop Codec Ring\r");
            PRINT_STR("5. SI32261 RAM read\r");
            PRINT_STR("6. SI32261 RAM write\r");
            PRINT_STR("7. SI32261 Rev\r");
            PRINT_STR("8. protected mode\r");
            PRINT_STR("9. Do LB Calibration\r");
            PRINT_STR("a. Do FXS initialization\r");
            PRINT_STR("b. Do FXS load patch\r");
            PRINT_STR("c. Reset FXS Codec\r");
            PRINT_STR("d. SI32261 Reg read conti\r");
            PRINT_STR("e. SI32261 RAM read conti\r");
            PRINT_STR("f. SI32261 Ring test SI3050 Ring port\r");
            PRINT_STR("10. SI32261 Set Codec ON-OFF Hook mapping\r");
            PRINT_STR("11. SI32261 Stop Codec ON-OFF Hook mapping\r");
            PRINT_STR("ff. Quit only\r");
        }

        disp_menu = FALSE;

        tst_item = gethex_answer("                Select an option > ", 0, 0, 0xff);

        if (tst_item == 0xff) {
            break;
        }

        data = 0;

        if (tst_item != 0) {
            port = gethex_answer("\nEnter voice port. Staring from 0",
                                    0, 0, max_ports - 1);
        }

        /* Get the TDM port for mapping to connection memory */

        switch (tst_item) {
        case 1:  /* reg read */
            codec_si32261_read_write_reg (port, TRUE);
            break;
        case 2: /* reg write */
            codec_si32261_read_write_reg (port, FALSE);
            break;
        case 3: /* set ring */
            return (codec_si32261_set_stop_ring (port, TRUE));
            break;
        case 4: /* stop ring */
            return (codec_si32261_set_stop_ring (port, FALSE));
            break;
        case 5: /* read ram */
            codec_si32261_rd_wt_ram (port, TRUE);
            break;
        case 6: /* write ram */
            codec_si32261_rd_wt_ram (port, FALSE);
            break;
        case 7: /* read rev */
            codec_read_rev (port);
            break;
        case 8: /* protected */
            codec_si32261_protected (port);
            break;
        case 9: /* LB calibration */
            return (oak_fxs_calibration(port));
            break;
        case 0xa: 
            si3226x_init_with_options(port);
            break;
        case 0xb: 
            si32261_load_patch (port);
            break;
        case 0xc: 
            reset_fxs_codec();
            break;
        case 0xd: /* read reg */
            codec_si32261_rd_reg_conti (port);
            break;
        case 0xe: /* read ram */
            codec_si32261_rd_ram_conti (port);
            break;
        case 0xf: /* set FXS ring to test FXO ring ports */
            return (codec_si32261_ring_si3050_test (port));
            break;
        case 0x10: /* set on-off hook mapping */
            return (codec_si32261_set_stop_onoff_hook_map (port, TRUE));
            break;
        case 0x11: /* stop on-off hook mapping */
            return (codec_si32261_set_stop_onoff_hook_map (port, FALSE));
            break;
        default:
            disp_menu = TRUE;
            break;
        }
     }

     return (PASSED);
}

/**********************************************************************
 *
 * Function: oak_fxs_calibration
 *
 * Description: The routine sends command to SI32261 to request to
 *              perform longitude calibtation.
 *
 * Input : start port_num
 *
 * Output: PASSED/FAILED
 *
 *********************************************************************
*/
int oak_fxs_calibration (int port_num)
{
    int max_fxs_ports, go_calibration, port;
    int ret_val = PASSED;
    boolean write_default_value = FALSE;
    uint ret_data[FXS_MAX_CALIBRATION_PORTS];
    uint calib_data[FXS_MAX_CALIBRATION_PORTS];
    /* fw_setenv calib string */
    char calib_str[80], calib_data_str[20];
    char cal_num[10], errstr[128];
    uchar board_id;
    
    do_calibration_flag = TRUE;
    bsp_debug_printf ("\n\r Oakenshield FXS Codec Calibration \n");

    memset(calib_data_str, 0 , sizeof(calib_data_str));
    memset(calib_str, 0 , sizeof(calib_str));

    max_fxs_ports = get_fxs_port_num();
    if (max_fxs_ports == 0xff) {
        cterr('f', 0,"Unknown FXS present");
        return(FAILED);
    }

    /* Sanity Check */
    if (max_fxs_ports > FXS_MAX_CALIBRATION_PORTS) {
        cterr('f', 0, "Max Calibration Ports is %d\n",
               FXS_MAX_CALIBRATION_PORTS);
        return(FAILED);
    }

    bsp_debug_printf("\n\r For the calibration, user need remove all"
           " the attached connector!!\n");

    go_calibration = gethex_answer("Use default calibration value. 1: Yes, 0: No", 0, 0, 1);
    if (go_calibration == 1) {
        write_default_value = TRUE;
    } else {
        write_default_value = FALSE;
    }
    reset_fxs_codec();
    if (write_default_value == TRUE) {
        for (port = 0; port < max_fxs_ports; port++) {
	        calib_data[port] = 0x10108080;
        }
    } else {
        board_id = get_oak_id();
        if (board_id == VG400_2FXS_2FXO) { 
            port_num = VG_2FXS_STR_PORT;
        } 

        /* si3226x_load_patch will load max ports once */
        bsp_debug_printf("\r\nDo SI32261 Load Patch\r\n");
        for (port = port_num; port < max_fxs_ports; port++) {
            /* Phoenix need separate DBx and except no exist which_port */
            if (is_phoenix()) {
                if (board_id == PHOENIX_144FXS) {
                    if ((port < FXS_PORT24) && phoenix_only_test_dbx_flag) {
                        port = FXS_PORT23; /* escape MB FXS port */
                        continue;
                    } else if ((FXS_PORT24 <= port) && (port < FXS_PORT48) && 
                               (!phoenix_has_dbx(BOARD_DB1_TEST))) {
                        port = FXS_PORT47;
                        continue;
                    } else if ((FXS_PORT48 <= port) && (port < FXS_PORT96) && 
                               (!phoenix_has_dbx(BOARD_DB2_TEST))) {
                        port = FXS_PORT95;
                        continue;
                    } else if ((FXS_PORT96 <= port) && (!phoenix_has_dbx(BOARD_DB3_TEST))) {
                        port = max_fxs_ports;
                        continue;
                    }
                } else { /* 132FXS and 84 FXS case */
                    if ((port < FXS_PORT24) && phoenix_only_test_dbx_flag) {
                        port = FXS_PORT23;
                        continue;
                    } else if ((FXS_PORT24 <= port) && (port < FXS_PORT36) && 
                               (!phoenix_has_dbx(BOARD_DB1_TEST))) {
                        port = FXS_PORT35;
                        continue;
                    } else if ((FXS_PORT36 <= port) && (port < FXS_PORT84) && 
                               (!phoenix_has_dbx(BOARD_DB2_TEST))) {
                        port = FXS_PORT83;
                        continue;
                    } else if ((FXS_PORT84 <= port) && (!phoenix_has_dbx(BOARD_DB3_TEST))) {
                        port = max_fxs_ports;
                        continue;
                    }
                }
            }
            if (si32261_protected_mode(port) == FAILED) {
                cterr('f', 0,  "Load patch set protect fail\n");
                return (FAILED);
            }
            msleep(10);
        }

        bsp_debug_printf("Load patch\r\n");

        /* load patch */
        if (si32261_load_patch_cal(max_fxs_ports) == FAILED) {
            cterr('f', 0, "load_patch fail\n");
            sprintf(errstr, "\nload patch fail\n");
            strcat((char *)&(hd_if->errmsg), errstr);
            return (FAILED);
        }
        for (port = port_num; port < max_fxs_ports; port++) {
            /* Phoenix need separate DBx and except no exist which_port */
            if (is_phoenix()) {
                if (board_id == PHOENIX_144FXS) {
                    if ((port < FXS_PORT24) && phoenix_only_test_dbx_flag) {
                        port = FXS_PORT23; /* escape MB FXS port */
                        continue;
                    } else if ((FXS_PORT24 <= port) && (port < FXS_PORT48) && 
                               (!phoenix_has_dbx(BOARD_DB1_TEST))) {
                        port = FXS_PORT47;
                        continue;
                    } else if ((FXS_PORT48 <= port) && (port < FXS_PORT96) && 
                               (!phoenix_has_dbx(BOARD_DB2_TEST))) {
                        port = FXS_PORT95;
                        continue;
                    } else if ((FXS_PORT96 <= port) && (!phoenix_has_dbx(BOARD_DB3_TEST))) {
                        port = max_fxs_ports;
                        continue;
                    }
                } else { /* 132FXS and 84 FXS case */
                    if ((port < FXS_PORT24) && phoenix_only_test_dbx_flag) {
                        port = FXS_PORT23;
                        continue;
                    } else if ((FXS_PORT24 <= port) && (port < FXS_PORT36) && 
                               (!phoenix_has_dbx(BOARD_DB1_TEST))) {
                        port = FXS_PORT35;
                        continue;
                    } else if ((FXS_PORT36 <= port) && (port < FXS_PORT84) && 
                               (!phoenix_has_dbx(BOARD_DB2_TEST))) {
                        port = FXS_PORT83;
                        continue;
                    } else if ((FXS_PORT84 <= port) && (!phoenix_has_dbx(BOARD_DB3_TEST))) {
                        port = max_fxs_ports;
                        continue;
                    }
                }
            }
            if (si32261_common_mode_calibration(port, &ret_data[port]) == FAILED) {
                cterr('f', 0, "%s Unable to get calibration data for port %d",
                       __FUNCTION__, port);
                ret_val = FAILED;
                continue;
            }        
            calib_data[port] =  0x10100000 | (ret_data[port] & 0xFFFF);
        }
    
    }

    for (port = port_num; port < max_fxs_ports; port++) {
	    sprintf(&calib_data_str[0], " %x", (int)calib_data[port]);
        strcat(&calib_str[0], &calib_data_str[0]);

        if ((port % 8) == 7) {    /* There is 8 ports in 1 set and start from 0*/
            sprintf(&cal_num[0], "calib_%d", ((port + 1)/8));
            bsp_debug_printf("\r\n %s= %s\n", cal_num, calib_str);
        } else if ((port % max_fxs_ports) == (max_fxs_ports -1)) {
            sprintf(&cal_num[0], "calib_%d", (((port + 1)/8)+1));
            bsp_debug_printf("\r\n %s= %s\n", cal_num, calib_str);
        }
            
        sp_SetGPIODataHigh(0x40);
        env_get();
        env_set_string(cal_num, calib_str);
        env_sync(0);
        sp_SetGPIODataLow(0x40);

        memset(calib_data_str, 0 , sizeof(calib_data_str));
        memset(calib_str, 0 , sizeof(calib_str));

    }

    do_calibration_flag = FALSE;



    return (ret_val);

}


/**********************************************************************
 *
 * Function: get_fxs_which_codec
 *
 * Description: return which codec based on board ID and FXS port.
 *
 * Input : board_id
 *         port
 *
 * Output: which_codec
 *
 *********************************************************************
*/
int get_fxs_which_codec(uchar board_id, int port)
{
    int which_codec;

    switch (board_id) {
        case PHOENIX_144FXS:
            if (port < FXS_PORT12) {
                which_codec = port/2;
            } else if ((FXS_PORT12 <= port) && (port < FXS_PORT24)) {
                which_codec = port/2 + 2;
            } else if ((FXS_PORT24 <= port) && (port < FXS_PORT36)) {
                which_codec = port/2 + 4;
            } else if ((FXS_PORT36 <= port) && (port < FXS_PORT48)) {
                which_codec = port/2 + 6;
            } else if ((FXS_PORT36 <= port) && (port < FXS_PORT48)) {
                which_codec = port/2 + 6;
            } else {
                which_codec = port/2 + 8;
            }
            break;

        case PHOENIX_132FXS_6FXO:
        case PHOENIX_84FXS_6FXO:
            if (port < FXS_PORT12) {
                which_codec = port/2;
            } else if ((FXS_PORT12 <= port) && (port < FXS_PORT24)) {
                which_codec = port/2 + 2;
            } else if ((FXS_PORT24 <= port) && (port < FXS_PORT36)) {
                which_codec = port/2 + 4;
            } else {
                which_codec = port/2 + 14;
            }
            break;

        case VG400_2FXS_2FXO:
            which_codec = (port + VG_2FXS_STR_PORT)/2;
            break;

        default:
            which_codec = port/2;
    }

    return (which_codec);
}


/******** History ********
$Log: fxs_test.c,v $
Revision 1.5  2021/04/15 00:53:07  achiu2
[PRRQ:CSCvx56970-2]Phoenix code review for ER

Revision 1.4.64.3  2020/12/17 09:17:02  hondwang

CSCvw82265:
    Fix console switch to DSP side calibration issue
Fixed by:
   1. Fix allocate space not enought issue
   2. Fix Phoenix 132 and 84 FXS port sku miss last 4 data issue
Server: sjc-ads-9168

Revision 1.4.64.2  2020/07/28 08:56:47  hondwang
Phoenix DSP LED utility fix

Revision 1.4  2018/08/30 06:39:42  haohsu
Collapse Vg400-branch to Main Trunk

Revision 1.3.20.8  2018/05/25 16:27:19  haohsu
Code change for FXO Ring Detection

Revision 1.3.20.7  2018/05/23 17:01:16  haohsu
Add FXO Ring detection

Revision 1.3.20.6  2018/05/09 18:33:45  haohsu
Fixed 2FXS/2FXO TDM mapping issue

Revision 1.3.20.5  2018/05/08 23:03:49  haohsu
Suport FXO Ring test

Revision 1.3.20.4  2018/04/13 03:18:01  haohsu
Code Change for Vg400 (return calibration value, skip FXO loopback in FXO SKus)

Revision 1.3.20.3  2018/03/22 06:05:00  haohsu
Fixed 6FXS for writing cal value to SPI ROM

Revision 1.3.20.2  2018/02/06 09:26:17  haohsu
Code change for VG400

Revision 1.3.20.1  2018/01/26 09:42:04  haohsu
*** empty log message ***

Revision 1.3  2017/08/09 08:12:25  harrchan
Display TDM bus number when FXS/FXO loopback fail

Revision 1.2  2017/07/28 07:58:51  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.1.2.1  2017/06/29 08:14:39  harrchan
Initial commit code for Oakenshield

Revision 1.1.2.9  2017/04/26 01:58:29  harrchan
Optimize oakenshield  FXS calibration

Revision 1.1.2.8  2017/04/17 06:08:46  olin2
Remove Enable PLL function

Revision 1.1.2.7  2017/03/30 10:25:50  harrchan
Add fpga upgrade utility

Revision 1.1.2.6  2017/03/09 07:23:34  harrchan
Support oakenshield double wide case

Revision 1.1.2.5  2017/02/09 06:41:05  olin2
Support voltage margin and fail over port utility

Revision 1.1.2.4  2017/01/17 05:07:06  olin2
Clean up debug code

Revision 1.1.2.3  2017/01/05 06:06:33  olin2
Support FXS Ring and Calibration

Revision 1.1.2.2  2016/12/23 06:56:04  olin2
Support FXS/FXO loopback test

Revision 1.1.2.1  2016/12/14 04:57:38  olin2
Initial commit code for Oakenshield




$Endlog$
*/

