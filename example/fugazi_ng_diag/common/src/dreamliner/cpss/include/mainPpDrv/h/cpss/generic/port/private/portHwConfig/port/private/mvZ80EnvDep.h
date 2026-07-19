/* $Id: mvZ80EnvDep.h,v 1.1 2015/02/13 11:35:00 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/cpss/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/private/mvZ80EnvDep.h,v $
 *------------------------------------------------------------------
 *
 * Ian Chang - Nov. 2014
 *
 * Copyright (c) 2014-2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 */
/*******************************************************************************
*              Copyright 2001, GALILEO TECHNOLOGY, LTD.
*
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL. NO RIGHTS ARE GRANTED
* HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT OF MARVELL OR ANY THIRD
* PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE DISCRETION TO REQUEST THAT THIS
* CODE BE IMMEDIATELY RETURNED TO MARVELL. THIS CODE IS PROVIDED "AS IS".
* MARVELL MAKES NO WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS
* ACCURACY, COMPLETENESS OR PERFORMANCE. MARVELL COMPRISES MARVELL TECHNOLOGY
* GROUP LTD. (MTGL) AND ITS SUBSIDIARIES, MARVELL INTERNATIONAL LTD. (MIL),
* MARVELL TECHNOLOGY, INC. (MTI), MARVELL SEMICONDUCTOR, INC. (MSI), MARVELL
* ASIA PTE LTD. (MAPL), MARVELL JAPAN K.K. (MJKK), GALILEO TECHNOLOGY LTD. (GTL)
* AND GALILEO TECHNOLOGY, INC. (GTI).
********************************************************************************
* gtEnvDep.h
*
* DESCRIPTION:    Hardware environment depended types definition
*
* DEPENDENCIES:   Operating System.
*
* FILE REVISION NUMBER:
*       $Revision: 1.1 $
*******************************************************************************/

#ifndef __mvZ80EnvDeph
#define __mvZ80EnvDeph



#define AP_Ports_Num (12)

#define  STATIC
/* Return the mask including "numOfBits" bits.          */
#define U16BIT_MASK(numOfBits) (~(0xFFFFFFFF << (numOfBits)))

/* Calculate the field mask for a given offset & length */
/* e.g.: BIT_MASK(8,2) = 0xFFFFFCFF                     */
#define U16FIELD_MASK_NOT(offset,len)                      \
        (~(U16BIT_MASK((len)) << (offset)))

/* Calculate the field mask for a given offset & length */
/* e.g.: BIT_MASK(8,2) = 0x00000300                     */
#define U16FIELD_MASK(offset,len)                      \
        ( (U16BIT_MASK((len)) << (offset)) )
/* Returns the info located at the specified offset & length in data.   */
#define U16_GET_FIELD(data,offset,length)           \
        (((data) >> (offset)) & ((1 << (length)) - 1))


/* Sets the field located at the specified offset & length in data.     */
#define U16_SET_FIELD(data,offset,length,val)           \
   (data) = (((data) & U16FIELD_MASK_NOT((offset),(length))) | ((val) <<(offset)))


#endif   /* __mvZ80EnvDeph */



/*
 *------------------------------------------------------------------
 * $Log: mvZ80EnvDep.h,v $
 * Revision 1.1  2015/02/13 11:35:00  iachang
 * Check in Marvell GE switch driver header file.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
