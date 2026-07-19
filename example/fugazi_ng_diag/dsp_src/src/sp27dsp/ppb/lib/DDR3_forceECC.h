/* $Id: DDR3_forceECC.h,v 1.1 2012/06/07 22:34:34 srane Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/sp27dsp/ppb/lib/DDR3_forceECC.h,v $
 *------------------------------------------------------------------
 * DDR3_forceECC.h 
 * Description: type definitions, macros, etc. to be used for DDR3
 * ECC error-forcing functions
 *
 * June 2012, Smita Rane
 *
 * Copyright (c) 2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

/******************************************************************************

 *                             NOTIFICATION

 *

 * Copyright (c) 2012 LSI Inc.  All Rights Reserved

 *

 * This is unpublished proprietary information of LSI Inc.  This

 * copyright notice does not evidence publication.

 *

 * The use of the software, documentation, methodologies, and other information

 * contained herein is governed solely by the associated license agreements.

 * Any inconsistent use shall be deemed to be a misappropriation of the

 * intellectual property of LSI Inc. and treated accordingly.

 *----------------------------------------------------------------------------

 *

 * DDR3_forceECC.h - type definitions, macros, etc. to be used for DDR3

 * ECC error-forcing functions

 *

 *  Created on: Apr 24, 2012

 *      Author: vlos

 */



#ifndef DDR3_FORCEECC_H_

#define DDR3_FORCEECC_H_



/*----------------------------------*/

/*   	TYPE DEFINITIONS    		*/

/*----------------------------------*/



/* Which 32-bit word (from the 128-bit user word) to use for error generation */

typedef enum {

	WORD0 = 0,

	WORD1,

	WORD2,

	WORD3

} user_word_num_t;



/* Which bit to use as the incorrect bit when forcing an ECC error */

typedef enum {

	NO_ERROR = 0,

	MULTIBIT_ERROR = 0x7F,

	CHECK0 = 0x1,

	CHECK1 = 0x2,

	CHECK2 = 0x4,

	CHECK3 = 0x8,

	CHECK4 = 0x10,

	CHECK5 = 0x20,

	CHECK6 = 0x40,

	DATA0 = 0x75,

	DATA1 = 0x70,

	DATA2 = 0x6D,

	DATA3 = 0x6B,

	DATA4 = 0x68,

	DATA5 = 0x67,

	DATA6 = 0x64,

	DATA7 = 0x62,

	DATA8 = 0x5E,

	DATA9 = 0x5B,

	DATA10 = 0x58,

	DATA11 = 0x57,

	DATA12 = 0x54,

	DATA13 = 0x52,

	DATA14 = 0x4F,

	DATA15 = 0x4A,

	DATA16 = 0x34,

	DATA17 = 0x31,

	DATA18 = 0x2C,

	DATA19 = 0x2A,

	DATA20 = 0x29,

	DATA21 = 0x26,

	DATA22 = 0x25,

	DATA23 = 0x23,

	DATA24 = 0x1C,

	DATA25 = 0x1A,

	DATA26 = 0x19,

	DATA27 = 0x16,

	DATA28 = 0x15,

	DATA29 = 0x13,

	DATA30 = 0x0E,

	DATA31 = 0x0B

} bit_pos_t;



#endif /* DDR3_FORCEECC_H_ */

/******** History ********
$Log: DDR3_forceECC.h,v $
Revision 1.1  2012/06/07 22:34:34  srane
Initial checkin for ECC memory test.
 

$Endlog$
*/

