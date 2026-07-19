/* $Id: libppbuart.c,v 1.2 2017/07/28 07:58:48 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/ppb/lib/libppbuart.c,v $
 *------------------------------------------------------------------
 * libppbuart.c
 *
 * Mar 2012, Smita Rane
 *
 * Copyright (c) 2012-2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

/******************************************************************************
 *
 * Copyright (c) 2010 LSI Inc.  All Rights Reserved
 *
 * Filename:	libuart.c
 * Author:		DW Kim
 * Content:		UART API
 *
 * History:		June 12th 2009 : First release
 *
 ******************************************************************************/

/* standard header files */
#include "libuart.h"
#include <stdlib.h>
#include "lsi_sp27xx_reg.h"
#include "uart.h"

#define NULL_PTR	-1
#define HEX	16
#define DEC 10


void sp_InitSerial(uint32_t brd_i, uint32_t brd_f, uint32_t lbe)
{
	/* set-up UART control register */
	/* 1. disable UART */
	REG32_RESET_BITS(LSI_SP27XX_UARTCR_RA, LSI_SP27XX_UARTCR_UARTEN_BM);

	/* 2. wait for the end of transmission of reception of the current character */

	/* 3. Flush the transmit FIFO by disabling bit 4(FEN) in the line control register */
	REG32_RESET_BITS(LSI_SP27XX_UARTLCR_H_RA, LSI_SP27XX_UARTLCR_H_FEN_BM);
	REG32_SET_BITS(LSI_SP27XX_UARTLCR_H_RA , LSI_SP27XX_UARTLCR_H_FEN_BM);

	/* 3.5 program integer and fractional information of baud rate divider */
	/* baud rate = UART_CLK/(16 * (brd_i + brd_f/64)) */

	REG32_WRITE(LSI_SP27XX_UARTIBRD_RA, brd_i);
	REG32_WRITE(LSI_SP27XX_UARTFBRD_RA, brd_f);

	/* 4. Reprogram the control register */
	REG32_SET_BITS(LSI_SP27XX_UARTCR_RA, ((lbe<<LSI_SP27XX_UARTCR_LBE_BO)
						| LSI_SP27XX_UARTCR_TXE_BM  | LSI_SP27XX_UARTCR_RXE_BM));

	REG32_SET_BITS(LSI_SP27XX_UARTLCR_H_RA, (0x3<<LSI_SP27XX_UARTLCR_H_WLEN_BO)); /* 8bit transmit */

	REG32_SET_BITS(LSI_SP27XX_UARTCR_RA, LSI_SP27XX_UARTCR_UARTEN_BM);
}

void sp_SerialPutC(char tx_data)
{
	while(CHK_REG_MASK(LSI_SP27XX_UARTFR_RA,LSI_SP27XX_UARTFR_BUSY_BM)) ;

	REG32_WRITE(LSI_SP27XX_UARTDR_RA, tx_data);

	if(tx_data=='\r')
	{
		while(CHK_REG_MASK(LSI_SP27XX_UARTFR_RA,LSI_SP27XX_UARTFR_BUSY_BM)) ;
			tx_data = '\n';

			REG32_WRITE(LSI_SP27XX_UARTDR_RA, tx_data);
	};

}

char sp_SerialGetC()
{
	/* wait until FIFO is released */
	while(CHK_REG_MASK(LSI_SP27XX_UARTFR_RA, LSI_SP27XX_UARTFR_RXFE_BM)) ;

	return ((*(volatile int *)LSI_SP27XX_UARTDR_RA)&0xff);
//	return (MSK_REG_MASK(LSI_SP27XX_UARTDR_RA, 0xFF));
}

char* ultostr(uint32_t value, char* ptr, uint32_t base, uint32_t fixed_length);

/* receive string data via UART device */
int32_t sp_SerialGetS(char* rx_data)
{
	char* offset;
	char temp;
	uint32_t num_char = 0;

	offset = rx_data;
	*offset = 0x0;

	if(rx_data == NULL)
	{
		return NULL_PTR;
	}

	while(1)
	{
		temp = sp_SerialGetC();
		sp_SerialPutC(temp);

		*offset = temp;

		num_char++;

		if(*offset == '\r') {
			break;
		}
		offset++;
		if (num_char >= UART_MAX_CHARS) {
			break;
		}
	}
	return num_char;
}

/* print out string data via UART device */
int32_t sp_SerialPutS(char* tx_data)
{
	char *offset;
	uint32_t num_char = 0;

	if (tx_data == NULL)
	{
		return NULL_PTR;
	}

	offset = tx_data;

	while(1)
	{
		sp_SerialPutC(*offset);

		if(*offset == '\0')
		{
			break;
		}

		num_char++;
		offset++;
	}
	return num_char;
}

/* print out long data via UART device */
int32_t sp_SerialPutLong(uint32_t data, char format)
{
	char temp[11];
	uint32_t offset = 0;

	if(format=='h')
	{
		sp_SerialPutS("0x");
		ultostr(data, &temp[0], HEX, 8);
		return (sp_SerialPutS(&temp[0]));
	}
	else if(format=='d')
	{
		ultostr(data, &temp[0], DEC, 10);
		while(temp[offset]=='0')
		{
			if(offset>9) /* reaches to the last element of the char array */
			{
				break;
			}
			offset++;
		}

		if(offset == 10) /* if the character is '0', then display 0 */
		{
			return (sp_SerialPutS("0"));
		}

		else
		{
			return (sp_SerialPutS(&temp[offset]));
		}
	}
	return -1;
}

/* data converter */
char* ultostr(uint32_t value, char* ptr, uint32_t base, uint32_t fixed_length)
{
  uint32_t t = 0, res = 0;
  uint32_t count = 0;
  uint32_t temp = value;
  uint32_t i = 0;

  if (NULL == ptr)
  {
    return NULL;
  }

  if (temp == 0)
  {
	  count++;
  }

  while(temp>0)
  {
	  temp=temp/base;
	  count++;
  }

  if(fixed_length != 0) /* variable */
    if(fixed_length>(count-1))
		  count = fixed_length;

  ptr += count;
  *ptr = '\0';

  do
  {
    res = value - base * (t = value / base);
    if (res < 10)
    {
      * -- ptr = '0' + res;
    }
    else if ((res >= 10) && (res < 16))
    {
        * --ptr = 'A' - 10 + res;
    }
    i++;
  } while ((value = t) != 0);

  while(count-i)
  {
	  * -- ptr = '0';
	  i++;
  }
  return(ptr);
}

uint32_t uart_rx_str(char* rx_data)
{
        char* offset;
        char temp;
        uint32_t num_char = 0;

        offset = rx_data;
        *offset = 0x0;

        if(rx_data == NULL) {
                return NULL_PTR;
        }

        while(1)
        {
                temp = uart_getch();
                uart_putch(temp);

                if (temp != 0x7f) {     /* DEL */
                        *offset = temp;
                        num_char++;
                        if(*offset == '\r') {
                                /* Remove the carriage return....it is not
                                 * needed anymore
                                 */
                                *offset = '\0' ;
                                break;
                        }
                        offset++;
                } else {
                        if (num_char>0){
                                offset-- ;
                                num_char-- ;
                        }
                }
        }
        return num_char;
}

/******** History ********
$Log: libppbuart.c,v $
Revision 1.2  2017/07/28 07:58:48  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.1.2.1  2017/06/29 08:14:36  harrchan
Initial commit code for Oakenshield

Revision 1.3  2012/06/28 21:25:04  srane
add uart rx routine.

Revision 1.2  2012/05/10 22:48:11  srane
clean up and modify files for exception handling and TDM tests.

Revision 1.1  2012/04/18 09:47:31  srane
Initial checkin


$Endlog$
*/

