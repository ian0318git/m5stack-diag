/* $Id: siu_io.c,v 1.2 2017/07/28 07:58:49 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/ppb/lib/siu_io.c,v $
 *------------------------------------------------------------------
 * siu_io.c
 *
 * Mar 2012, Smita Rane
 *
 * Copyright (c) 2012-2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

/*
 * siu_io.c
 *
 *  Created on: Sep 18, 2009
 *      Author: dokim
 */
#include "lsi_sp27xx_reg.h"
#include "siu_io.h"
#include "libuart.h"

typedef volatile unsigned long	reg32;

#define SUCCESS 0

#define ON						(1)
#define OFF						(0)

#define MAX_NUM_PORT				(6)
#define MAX_CH						(256)

int __siu_reset ( int port )
{
	int i;

	if (port > MAX_NUM_PORT-1)
	{
		return ERROR;
	}

	REG32_SET_BITS(LSI_SP27XX_TDM_SCON1_RA(port), LSI_SP27XX_TDM_SCON1_IRESET_BM);
	REG32_SET_BITS(LSI_SP27XX_TDM_SCON2_RA(port), LSI_SP27XX_TDM_SCON2_ORESET_BM);
	REG32_SET_BITS(LSI_SP27XX_TDM_SCON4_RA(port), LSI_SP27XX_TDM_SCON4_AGRESET_BM);

	for(i=0; i<8; i++)
	{
		/* Disable input & ouptput channels */
		REG32_WRITE(LSI_SP27XX_TDM_SCIEN_RA(port, i), 0);
		REG32_WRITE(LSI_SP27XX_TDM_SCOEN_RA(port, i), 0);
	}

	/* Clear SCON0 */
	REG32_WRITE(LSI_SP27XX_TDM_SCON0_RA(port), 0);

	return SUCCESS;
}

int __siu_start(int port)
{
#ifdef TDM_DEBUG
    uint32_t reg;
#endif
	if (port > MAX_NUM_PORT-1)
	{
		return ERROR;
	}
#ifdef TDM_DEBUG
sp_SerialPutS("\r\n In __siu_start()");
#endif

	REG32_WRITE(LSI_SP27XX_TDM_STAT_RA(port), LSI_SP27XX_TDM_STAT_RM);
#ifdef TDM_DEBUG
sp_SerialPutS("\r\n In __siu_start() after LSI_SP27XX_TDM_STAT_RA register write ");
#endif
	REG32_RESET_BITS(LSI_SP27XX_TDM_SCON1_RA(port), LSI_SP27XX_TDM_SCON1_IRESET_BM);
#ifdef TDM_DEBUG
sp_SerialPutS("\r\n In __siu_start() after LSI_SP27XX_TDM_SCON1_RA register write ");
sp_SerialPutS("\r\n In __siu_start() LSI_SP27XX_TDM_SCON2_RA = ");
sp_SerialPutLong(port, 'd');
reg = LSI_SP27XX_TDM_BASE+(LSI_SP27XX_TDM_IVL*port);
sp_SerialPutS("\r\n In __siu_start() reg = ");
sp_SerialPutLong(reg, 'h');
#endif
	REG32_RESET_BITS(LSI_SP27XX_TDM_SCON2_RA(port), LSI_SP27XX_TDM_SCON2_ORESET_BM);
#ifdef TDM_DEBUG
sp_SerialPutS("\r\n In __siu_start() after register write ");
#endif

	return SUCCESS;
}

int __siu_stop( int port )
{
	if (port > MAX_NUM_PORT-1)
	{
		return ERROR;
	}

	REG32_SET_BITS(LSI_SP27XX_TDM_SCON1_RA(port), LSI_SP27XX_TDM_SCON1_IRESET_BM);
	REG32_SET_BITS(LSI_SP27XX_TDM_SCON2_RA(port), LSI_SP27XX_TDM_SCON2_ORESET_BM);

	return SUCCESS;
}

void __siu_int_clk_fs_on(int port)
{
	REG32_RESET_BITS(LSI_SP27XX_TDM_SCON4_RA(port), LSI_SP27XX_TDM_SCON4_AGRESET_BM);
}

void __siu_int_clk_fs_off(int port)
{
	REG32_SET_BITS(LSI_SP27XX_TDM_SCON4_RA(port), LSI_SP27XX_TDM_SCON4_AGRESET_BM);
}

int __siu_nbits_set(int port, int input, int output)
{
	if (port > MAX_NUM_PORT-1)
	{
		return ERROR;
	}

	REG32_RESET_BITS(LSI_SP27XX_TDM_SCON0_RA(port), LSI_SP27XX_TDM_SCON0_OSIZE_BM | LSI_SP27XX_TDM_SCON0_ISIZE_BM);
	REG32_SET_BITS(LSI_SP27XX_TDM_SCON0_RA(port), ((input<<LSI_SP27XX_TDM_SCON0_OSIZE_BO)&LSI_SP27XX_TDM_SCON0_OSIZE_BO)\
													|((output<<LSI_SP27XX_TDM_SCON0_ISIZE_BO)&LSI_SP27XX_TDM_SCON0_ISIZE_BM));

	return SUCCESS;
}

int __siu_shift_order (int port, int in_shift, int out_shift)
{
    if (port > MAX_NUM_PORT-1)
    {
        return ERROR;
    }

    REG32_SET_BITS(LSI_SP27XX_TDM_SCON0_RA(port), (in_shift)?LSI_SP27XX_TDM_SCON0_IMSB_BM:0);
    REG32_SET_BITS(LSI_SP27XX_TDM_SCON0_RA(port), (out_shift)?LSI_SP27XX_TDM_SCON0_OMSB_BM:0);

    return SUCCESS;

}

int __siu_clk_ctrl (int port, int enable)
{
    if (port > MAX_NUM_PORT-1)
    {
        return ERROR;
    }

    REG32_SET_BITS(LSI_SP27XX_TDM_SCON0_RA(port), (enable)?LSI_SP27XX_TDM_SCON0_SIUCLKEN_BM:0);

    return SUCCESS;
}

int __siu_in_clk_set(int port, int active, int internally_gen)
{
	if (port > MAX_NUM_PORT-1)
	{
		return ERROR;
	}

	REG32_SET_BITS(LSI_SP27XX_TDM_SCON0_RA(port), (active)?LSI_SP27XX_TDM_SCON0_ICKA_BM:0);
	REG32_SET_BITS(LSI_SP27XX_TDM_SCON1_RA(port), (internally_gen)?LSI_SP27XX_TDM_SCON1_ICKE_BM:0);

	return SUCCESS;
}

int __siu_out_clk_set(int port, int active, int internally_gen)
{
	if (port > MAX_NUM_PORT-1)
	{
		return ERROR;
	}

	REG32_SET_BITS(LSI_SP27XX_TDM_SCON0_RA(port), (active)?LSI_SP27XX_TDM_SCON0_OCKA_BM:0);
	REG32_SET_BITS(LSI_SP27XX_TDM_SCON2_RA(port), (internally_gen)?LSI_SP27XX_TDM_SCON2_OCKE_BM:0);

	return SUCCESS;
}

int __siu_in_fsync_set(int port, int active, int internally_gen)
{
	if (port > MAX_NUM_PORT-1)
	{
		return ERROR;
	}

	REG32_RESET_BITS(LSI_SP27XX_TDM_SCON0_RA(port), LSI_SP27XX_TDM_SCON0_IFSA_BM);
	REG32_SET_BITS(LSI_SP27XX_TDM_SCON0_RA(port), (active)?LSI_SP27XX_TDM_SCON0_IFSA_BM:0);

	REG32_RESET_BITS(LSI_SP27XX_TDM_SCON1_RA(port), LSI_SP27XX_TDM_SCON1_IFSE_BM);
	REG32_SET_BITS(LSI_SP27XX_TDM_SCON1_RA(port), (internally_gen)?LSI_SP27XX_TDM_SCON1_IFSE_BM:0);

	return SUCCESS;
}

int __siu_out_fsync_set(int port, int active, int internally_gen)
{
	if (port > MAX_NUM_PORT-1)
	{
		return ERROR;
	}
	REG32_RESET_BITS(LSI_SP27XX_TDM_SCON0_RA(port), LSI_SP27XX_TDM_SCON0_OFSA_BM);
	REG32_SET_BITS(LSI_SP27XX_TDM_SCON0_RA(port), (active)?LSI_SP27XX_TDM_SCON0_OFSA_BM:0);

	REG32_RESET_BITS(LSI_SP27XX_TDM_SCON2_RA(port), LSI_SP27XX_TDM_SCON2_OFSE_BM);
	REG32_SET_BITS(LSI_SP27XX_TDM_SCON2_RA(port), (internally_gen)?LSI_SP27XX_TDM_SCON2_OFSE_BM:0);

	return SUCCESS;
}

int __siu_clk_ctl (int port)
{
        if (port > MAX_NUM_PORT-1)
        {
                return ERROR;
        }
        REG32_SET_BITS(LSI_SP27XX_TDM_TDMCLKCTL_RA, 1);
    return SUCCESS;

}

int __siu_agsync (int port)
{
        if (port > MAX_NUM_PORT-1)
        {
                return ERROR;
        }
        REG32_SET_BITS(LSI_SP27XX_TDM_SCON4_RA(port), LSI_SP27XX_TDM_SCON4_AGSYNC_BM);
    return SUCCESS;

}

int __siu_ratio_set(int port, int agcklim, int agfslim)
{
	if (port > MAX_NUM_PORT-1)
	{
		return ERROR;
	}

	REG32_RESET_BITS(LSI_SP27XX_TDM_SCON3_RA(port), LSI_SP27XX_TDM_SCON3_AGCKLIM_BM);
	REG32_SET_BITS(LSI_SP27XX_TDM_SCON3_RA(port), agcklim&LSI_SP27XX_TDM_SCON3_AGCKLIM_BM);

	REG32_RESET_BITS(LSI_SP27XX_TDM_SCON4_RA(port), LSI_SP27XX_TDM_SCON4_AGFSLIM_BM);
	REG32_SET_BITS(LSI_SP27XX_TDM_SCON4_RA(port), agfslim&LSI_SP27XX_TDM_SCON4_AGFSLIM_BM);

	return SUCCESS;
}

int __siu_reset_loopback_for_test(int port)
{
        if (port > MAX_NUM_PORT-1)
        {
                return ERROR;
        }

        REG32_RESET_BITS(LSI_SP27XX_TDM_SCON0_RA(port), LSI_SP27XX_TDM_SCON0_SIOLB_BM); 
        return SUCCESS;

}

int __siu_set_loopback_for_test(int port)
{
	if (port > MAX_NUM_PORT-1)
	{
		return ERROR;
	}

	REG32_SET_BITS(LSI_SP27XX_TDM_SCON0_RA(port), LSI_SP27XX_TDM_SCON0_SIOLB_BM);
	return SUCCESS;
}

int __siu_mode_set(int port, int mode)
{
	if (port > MAX_NUM_PORT-1)
	{
		return ERROR;
	}

	REG32_SET_BITS(LSI_SP27XX_TDM_SCON2_RA(port), (mode<<LSI_SP27XX_TDM_SCON2_OFRAME_BO));

	return SUCCESS;
}

int __siu_chnum_set(int port, int num_ch)
{
	if (port > MAX_NUM_PORT-1)
	{
		return ERROR;
	}

	if (num_ch > MAX_CH)
	{
		num_ch = MAX_CH;
	}

	/* in */
	REG32_WRITE(LSI_SP27XX_TDM_SCON1_RA(port), LSI_SP27XX_TDM_SCON1_IRESET_BM);

	REG32_RESET_BITS(LSI_SP27XX_TDM_SCON1_RA(port), LSI_SP27XX_TDM_SCON1_IFLIM_BM);
	REG32_SET_BITS(LSI_SP27XX_TDM_SCON1_RA(port), num_ch-1);

	/* out */
	REG32_WRITE(LSI_SP27XX_TDM_SCON2_RA(port), LSI_SP27XX_TDM_SCON2_ORESET_BM);

	REG32_RESET_BITS(LSI_SP27XX_TDM_SCON2_RA(port), LSI_SP27XX_TDM_SCON2_OFLIM_BM);
	REG32_SET_BITS(LSI_SP27XX_TDM_SCON2_RA(port), num_ch-1);

	return SUCCESS;
}

/* enable/disable channels */

int __siu_onoff_in_chs(int port, int group, int ch_bmask, int onoff)
{
	if (port > MAX_NUM_PORT-1)
	{
		return ERROR;
	}

	if (group>7)
	{
		return ERROR;
	}

	if(onoff == OFF)
	{
		REG32_RESET_BITS(LSI_SP27XX_TDM_SCIEN_RA(port, group), ch_bmask);
	}
	else /* onoff == ON */
	{
		REG32_SET_BITS(LSI_SP27XX_TDM_SCIEN_RA(port, group), ch_bmask);
	}

	return SUCCESS;
}

int __siu_onoff_out_chs(int port, int group, int ch_bmask, int onoff)
{
	if (port > MAX_NUM_PORT-1)
	{
		return ERROR;
	}

	if (group>7)
	{
		return ERROR;
	}

	if(onoff == OFF)
	{
		REG32_RESET_BITS(LSI_SP27XX_TDM_SCOEN_RA(port, group), ch_bmask);
	}
	else /* onoff == ON */
	{
		REG32_SET_BITS(LSI_SP27XX_TDM_SCOEN_RA(port, group), ch_bmask);
	}

	return SUCCESS;
}

/* masking/unmasking channel(s) */

int __siu_msk_out_chs(int port, int group, int ch_bmask, int onoff)
{
	if (port > MAX_NUM_PORT-1)
	{
		return ERROR;
	}

	if (group>7)
	{
		return ERROR;
	}

	if(onoff == OFF) /* unmasked */
	{
		REG32_SET_BITS(LSI_SP27XX_TDM_SCOMSK_RA(port, group), ch_bmask);
	}
	else /* onoff == on, masked */
	{
		REG32_RESET_BITS(LSI_SP27XX_TDM_SCOMSK_RA(port, group), ch_bmask);
	}

	return SUCCESS;
}

/*****/

SIU_STATUS __siu_chk_status(int port)
{
	int temp;
	SIU_STATUS status;

	if (port > MAX_NUM_PORT-1)
	{
		status.reg = ERROR;
		return status;
	}

	REG32_READ(LSI_SP27XX_TDM_STAT_RA(port), temp);

	status.reg = temp;

	return status;
}

/* clear status */
int __siu_clr_status(int port)
{
	if (port > MAX_NUM_PORT-1)
	{
		return ERROR;
	}

	REG32_WRITE(LSI_SP27XX_TDM_STAT_RA(port), LSI_SP27XX_TDM_STAT_RM);

	return SUCCESS;
}
/******** History ********
$Log: siu_io.c,v $
Revision 1.2  2017/07/28 07:58:49  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.1.2.1  2017/06/29 08:14:37  harrchan
Initial commit code for Oakenshield

Revision 1.3  2012/06/07 22:50:24  srane
Support TDM external loopback test.

Revision 1.2  2012/05/10 22:48:11  srane
clean up and modify files for exception handling and TDM tests.

Revision 1.1  2012/04/18 09:47:31  srane
Initial checkin


$Endlog$
*/

