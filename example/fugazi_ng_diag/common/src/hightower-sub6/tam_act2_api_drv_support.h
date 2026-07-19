/* $Id: tam_act2_api_drv_support.h,v 1.1 2020/08/19 09:50:54 markzha Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/hightower-sub6/tam_act2_api_drv_support.h,v $
 *------------------------------------------------------------------
 * 
 * tam_act2_api_drv_support.h
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef TAM_ACT2_API_DRV_SUPPORT_H_
#define TAM_ACT2_API_DRV_SUPPORT_H_

#define SUCCEED             0
#define DIAG_MAX_ERRMSG_LEN 1024
#define I2C_SLAVE           0x0703
#define I2C_ACT2_TIMEOUT    0x0709
#define SEGMENT_I2C_READ    511
#define ACT_RETRY           100
#define ACT_DELAY           200

extern int is_tam_aikido_mbox_on(void);
extern int is_tam_aikido_on(void);
#endif /* TAM_ACT2_API_DRV_SUPPORT_H_ */

/*-------------------------------------------------
 * $Log: tam_act2_api_drv_support.h,v $
 * Revision 1.1  2020/08/19 09:50:54  markzha
 * *** empty log message ***
 *
 * Revision 1.2  2019/01/10 06:36:29  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
