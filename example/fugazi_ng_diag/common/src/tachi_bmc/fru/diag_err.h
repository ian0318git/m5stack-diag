/* $Id: diag_err.h,v 1.2 2016/04/20 08:41:35 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/fru/diag_err.h,v $
 *
 *      File:   diag_err.h
 *      Name:   Sudharshan Kadari
 *
 *      Description:
 *
 *
 * Copyright (c) 1985-2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *
 *********************************************************************/

/////////////////////////////////////////////////////////////////
//                                                             //
// File Name   : diag_err.h                                    //
//                                                             //
// Description : This file includes all the common defines     //
//               used by offline diagnostics.                  //
//                                                             //
/////////////////////////////////////////////////////////////////


#ifndef _DIAG_ERR_H
#define _DIAG_ERR_H

typedef enum _diagerr_
{
	DIAG_SUCCESS = 0,
	DIAG_SOFTWARE_ERROR = 1,
	DIAG_SYSTEM_ERROR = 2,
	DIAG_MALLOC_ERROR = 3,
	DIAG_SYNTAX_ERROR = 4,

	DIAG_REG_ACC_ERROR,
	DIAG_BIST_ERROR,
	DIAG_PORT_LINK_ERROR,
	DIAG_EEPROM_ERROR,
	DIAG_SFP_ERROR,

	// Redwood errors.
	DIAG_RED_ACCESS_ERROR,
	DIAG_RED_REG_TEST_FAILED,
	DIAG_RED_PRBS_TEST_FAILED,
	DIAG_RED_MBIST_TEST_FAILED,
	DIAG_RED_MEMIF_TEST_FAILED,
	DIAG_RED_SERDES_ACC_TEST_FAILED,
	DIAG_RED_SERDES_RXRDY_TEST_FAILED,
	DIAG_RED_FREE_LIST_TEST_FAILED,
	DIAG_RED_PORT_LINK_TEST_FAILED,

	// Packet Errors
	DIAG_PKT_TX_ERROR,
	DIAG_PKT_RX_ERROR,

	// PSU Errors
	DIAG_PSU_PRESENT_FAILED,
	DIAG_PSU_FAILED,
	DIAG_PSU_FAN_FAILED,
	DIAG_PSU_EEPROM_FAILED,
	DIAG_BRD_EEPROM_FAILED,

	// FLASH Errors
	DIAG_FLASH_ERASE_ERROR,
	DIAG_FLASH_OPEN_ERROR,
	DIAG_FLASH_READ_ERROR,
	DIAG_FLASH_WRITE_ERROR,
	DIAG_FLASH_DATA_ERROR,

	// FRU Errors
	DIAG_FRU_CHECK_ERROR,
	DIAG_RTC_ERROR,

	DIAG_FAN_SPEED_ERROR,
	DIAG_PSU_DC_OK_ERROR,
	DIAG_PSU_AC_OK_ERROR,
	DIAG_RANGE_VIOLATE_ERROR,

        //Palo Errors
        DIAG_PALO_PCI_ERROR,
        DIAG_PHY_EEPROM_ERROR,
        DIAG_PHY_PRBS_ERROR,

	// I2C Errors
	DIAG_I2CDEV_OPEN_ERROR,
	DIAG_DATA_MISMATCH_ERROR,

    // PECI Errors
    DIAG_PECI_ERROR,

    // LPC Errors
    DIAG_LPC_ERROR,

        // WOODSIDE Errors
        DIAG_WOO_REG_ACCESS_ERROR,
        DIAG_WOO_REG_TEST_FAILED,
        DIAG_WOO_MBIST_TEST_FAILED,
        DIAG_WOO_SBUS_TEST_FAILED,
        DIAG_WOO_PCS_PRBS_TEST_FAILED,
        DIAG_WOO_XE_CNT_CODE_VIOLATION_ERROR,
        DIAG_WOO_XE_RX_ERROR_SYMBOL_ERROR,
        DIAG_WOO_POLL_FAIL_ERROR,
        DIAG_WOO_POLL_WARN_ERROR,
        DIAG_PHY_POLL_FAIL_ERROR,
        DIAG_PHY_POLL_WARN_ERROR,


        DIAG_SANITY_ERROR,
        DIAG_VMARG_ERROR,

	// Blades
	DIAG_DIMM_SPD_ERROR,
	DIAG_DIMM_TS_ERROR,
       
        //Poe 
        DIAG_POE_DET_CLASS_ERROR, 

        /* PHY LOOPBACK Errors */
        DIAG_PHY_IF_UP_ERROR,
        DIAG_PHY_CONFIG_ERROR,
        DIAG_PHY_INT_LOOPBACK_ERROR,
        DIAG_PHY_EXT_LOOPBACK_ERROR,

	// Exit error.
	DIAG_EXIT_ERROR,
} diagerr_t;

#endif  // _DIAG_ERR_H
