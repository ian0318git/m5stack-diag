/* $Id: AQ_RegMaps.h,v 1.2 2019/06/14 05:24:53 mikech2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/katar/x86/AQR4xx/include/registerMap/AQ_RegMaps.h,v $
 *------------------------------------------------------------------
 * 
 * Filename   : 
 * Description: .
 *
 * Copyright (c) 2017 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */
/*AQ_RegMaps.h*/

/************************************************************************************
*
* Copyright (c) 2017 Aquantia Corp.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy 
* of this software and associated documentation files (the "Software"), to deal 
* in the Software without restriction, including without limitation the rights 
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell 
* copies of the Software, and to permit persons to whom the Software is 
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all 
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
* SOFTWARE.
*
************************************************************************************/

/************************************************************************************
*                     Copyright Aquantia Corp.
*                             Freeware
*
* Description:
*
*   This file contains includes all appropriate Aquantia PHY device-specific 
*   register map headers.
*
************************************************************************************/

/*! \file
*   This file contains includes all appropriate Aquantia PHY device-specific 
*   register map headers.
 */

#ifndef AQ_REGISTERMAPS_HEADER
#define AQ_REGISTERMAPS_HEADER

#include <AQ_User.h>
#include "AQ_RegGroupMaxSizes.h"


#ifndef AQ_REVERSED_BITFIELD_ORDERING
/* 
 * Include non-reversed header files (bitfields ordered from LSbit to MSbit) 
 */

/* APPIA */
#include "AQ_APPIA_Autonegotiation_registers.h"
#include "AQ_APPIA_GbE_registers.h"
#include "AQ_APPIA_Global_registers.h"
#include "AQ_APPIA_PCS_registers.h"
#include "AQ_APPIA_PHY_XS_registers.h"
#include "AQ_APPIA_PMA_registers.h"

/* HHD */
#include "AQ_HHD_Autonegotiation_registers.h"
#include "AQ_HHD_GbE_registers.h"
#include "AQ_HHD_Global_registers.h"
#include "AQ_HHD_PCS_registers.h"
#include "AQ_HHD_PHY_XS_registers.h"
#include "AQ_HHD_PMA_registers.h"

/* EUR */
#include "AQ_EUR_Autonegotiation_registers.h"
#include "AQ_EUR_GbE_registers.h"
#include "AQ_EUR_Global_registers.h"
#include "AQ_EUR_PCS_registers.h"
#include "AQ_EUR_PHY_XS_registers.h"
#include "AQ_EUR_PMA_registers.h"

/* CAL */
#include "AQ_CAL_Autonegotiation_registers.h"
#include "AQ_CAL_GbE_registers.h"
#include "AQ_CAL_Global_registers.h"
#include "AQ_CAL_PCS_registers.h"
#include "AQ_CAL_PHY_XS_registers.h"
#include "AQ_CAL_PMA_registers.h"

#else
/* 
 * Include reversed header files (bitfields ordered from MSbit to LSbit) 
 */

/* APPIA */
#include "AQ_APPIA_Autonegotiation_registers_reversed.h"
#include "AQ_APPIA_GbE_registers_reversed.h"
#include "AQ_APPIA_Global_registers_reversed.h"
#include "AQ_APPIA_PCS_registers_reversed.h"
#include "AQ_APPIA_PHY_XS_registers_reversed.h"
#include "AQ_APPIA_PMA_registers_reversed.h"

/* HHD */
#include "AQ_HHD_Autonegotiation_registers_reversed.h"
#include "AQ_HHD_GbE_registers_reversed.h"
#include "AQ_HHD_Global_registers_reversed.h"
#include "AQ_HHD_PCS_registers_reversed.h"
#include "AQ_HHD_PHY_XS_registers_reversed.h"
#include "AQ_HHD_PMA_registers_reversed.h"

/* EUR */
#include "AQ_EUR_Autonegotiation_registers_reversed.h"
#include "AQ_EUR_GbE_registers_reversed.h"
#include "AQ_EUR_Global_registers_reversed.h"
#include "AQ_EUR_PCS_registers_reversed.h"
#include "AQ_EUR_PHY_XS_registers_reversed.h"
#include "AQ_EUR_PMA_registers_reversed.h"

/* CAL */
#include "AQ_CAL_Autonegotiation_registers_reversed.h"
#include "AQ_CAL_GbE_registers_reversed.h"
#include "AQ_CAL_Global_registers_reversed.h"
#include "AQ_CAL_PCS_registers_reversed.h"
#include "AQ_CAL_PHY_XS_registers_reversed.h"
#include "AQ_CAL_PMA_registers_reversed.h"

#endif

/* APPIA */
#include "AQ_APPIA_Autonegotiation_registers_Defines.h"
#include "AQ_APPIA_GbE_registers_Defines.h"
#include "AQ_APPIA_Global_registers_Defines.h"
#include "AQ_APPIA_PCS_registers_Defines.h"
#include "AQ_APPIA_PHY_XS_registers_Defines.h"
#include "AQ_APPIA_PMA_registers_Defines.h"

/* HHD */
#include "AQ_HHD_Autonegotiation_registers_Defines.h"
#include "AQ_HHD_GbE_registers_Defines.h"
#include "AQ_HHD_Global_registers_Defines.h"
#include "AQ_HHD_PCS_registers_Defines.h"
#include "AQ_HHD_PHY_XS_registers_Defines.h"
#include "AQ_HHD_PMA_registers_Defines.h"

/* EUR */
#include "AQ_EUR_Autonegotiation_registers_Defines.h"
#include "AQ_EUR_GbE_registers_Defines.h"
#include "AQ_EUR_Global_registers_Defines.h"
#include "AQ_EUR_PCS_registers_Defines.h"
#include "AQ_EUR_PHY_XS_registers_Defines.h"
#include "AQ_EUR_PMA_registers_Defines.h"

/* CAL */
#include "AQ_CAL_Autonegotiation_registers_Defines.h"
#include "AQ_CAL_GbE_registers_Defines.h"
#include "AQ_CAL_Global_registers_Defines.h"
#include "AQ_CAL_PCS_registers_Defines.h"
#include "AQ_CAL_PHY_XS_registers_Defines.h"
#include "AQ_CAL_PMA_registers_Defines.h"

#endif
/*
 *------------------------------------------------------------------
 * $Log: AQ_RegMaps.h,v $
 * Revision 1.2  2019/06/14 05:24:53  mikech2
 * Collapse katar-branch00 to Main Trunk
 *
 * Revision 1.1.2.1  2018/10/22 08:02:37  mikech2
 * Move project folder to common/src/katar/x86
 *
 * Revision 1.1.2.1  2018/07/10 09:52:14  benlu
 * AQR4XX API
 *
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */

