/* $Id: diag_reg.c,v 1.2 2016/04/20 08:41:35 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/fru/diag_reg.c,v $
 *
 *      File:   diag_reg.c
 *      Name:   Sudharshan Kadari
 *
 *      Description:
 *       Diag infra structure 
 *
 *
 * Copyright (c) 1985-2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *
 *********************************************************************/

#include <stdio.h>
#include <string.h>
#include "diag_reg.h"
#include "regex.h"


reg_desc_t *find_reg_desc_by_exp(reg_desc_t *p_reg, regex_t *myregex)
{ 
	int  	   rc = 0, nmatch = 0;
	regmatch_t	pmatch[100];

	if (!p_reg) return (NULL);

	while(p_reg->desc_type != TYP_NONE)
	{ 
		if (p_reg->desc_type == TYP_REG) {
			rc = regexec(myregex, p_reg->name, nmatch, pmatch, 0);
			if (!rc) {
				return(p_reg);
			}
		}
		p_reg++;
	}
	return NULL;
}

reg_desc_t *find_reg_desc_by_typ(reg_desc_t *p_reg, unsigned char *typ )
{ 
	if (!p_reg) return (NULL);

	while(p_reg->desc_type != TYP_NONE)
	{ 
		if (p_reg->desc_type == TYP_REG) {
			if (!strncasecmp(p_reg->typ, typ, strlen(typ)))
				return p_reg;
		}
		p_reg++;
	}
	return NULL;
}

reg_desc_t *find_reg_desc_by_addr(reg_desc_t *p_reg, unsigned int reg_addr )
{ 
	if (!p_reg) return (NULL);

	while(p_reg->desc_type != TYP_NONE)
	{ 
		if (p_reg->desc_type == TYP_REG) {
			if (p_reg->addr == reg_addr)
				return p_reg;
		}
		p_reg++;
	}
	return NULL;
}

char *find_reg_name_by_addr(reg_desc_t *p_reg, unsigned int reg_addr )
{ 
	if (!p_reg) return (NULL);

	while(p_reg->desc_type != TYP_NONE)
	{ 
		if (p_reg->desc_type == TYP_REG) {
			if (p_reg->addr == reg_addr)
				return p_reg->name;
		}
		p_reg++;
	}
	return NULL;
}


reg_desc_t *find_reg_desc_by_blk_and_addr(reg_desc_t *p_reg, 
                                          char *block,
                                          unsigned int reg_addr)
{ 
	if (!p_reg) return (NULL);

	while(p_reg->desc_type != TYP_NONE)
	{ 
		if (p_reg->desc_type == TYP_REG) {
			if ((p_reg->addr == reg_addr) &&
			    (strncasecmp(block, p_reg->blk, 
                                         strlen(block)) == 0)) {
				return p_reg;
                        }
		}
		p_reg++;
	}
	return NULL;
}

reg_desc_t *find_next_reg(reg_desc_t *p_reg)
{ 
	if (!p_reg) return (NULL);

	while(p_reg->desc_type != TYP_NONE)
	{ 
		p_reg++;
		if (p_reg->desc_type == TYP_REG) {
			return p_reg;
		}
	}
	return NULL;
}

reg_desc_t *find_reg_desc(reg_desc_t *p_reg, char *reg_name )
{ 
	if (!p_reg) return (NULL);

	while(p_reg->desc_type != TYP_NONE)
	{ 
		if (p_reg->desc_type == TYP_REG) {
			if (strncasecmp(reg_name, p_reg->name, strlen(reg_name)) == 0)
				return p_reg;
		}
		p_reg++;
	}
	return NULL;
}

static char fmt_buf[3000];
static char line[3000];
void copy_to_fmt_buf( char *f, char *l )
{
	int len, pos, i;
	char *cont_line = "\n                  : ";

	len = strlen(l);
	pos = 0;

	while (pos < len)
	{
		for (i = 0; (i < 55) && (pos < len); i++, pos++)
		{
			if (*l == '\\' && *(l+1) == 'n')
			{
				l += 2; pos += 2; *f++ = ' '; continue;
			}
			if (*l == '\n')
			{
				l += 1; *f++ = ' '; continue;
			}
			*f++ = *l++;
		}
		if (pos < len) {
			sprintf( f, "%s", cont_line );
			f += strlen(cont_line);
		}
	}
	*f = 0; // terminate it
}


int decode_reg(reg_desc_t *p_reg,  unsigned int reg_addr, unsigned int reg_val )
{
	reg_desc_t *p_fld;

	if (!p_reg) return (0);

	memset(fmt_buf, 0, sizeof(fmt_buf));

	while(p_reg->desc_type != TYP_NONE)
	{
		if ((p_reg->addr == reg_addr) && (p_reg->desc_type == TYP_REG))
		{

			/* sprintf the whole thing into fmt_buf  and format to fit 80 char wide scree */
			sprintf( fmt_buf, "\n%-24s  (0x%08x) : [%08x]   ", p_reg->name, p_reg->addr, reg_val );
			if (*(p_reg->desc) != '\0')
			{
				sprintf( line, "desc: %s", p_reg->desc );
				copy_to_fmt_buf( &fmt_buf[ strlen(fmt_buf) ], line );
			}
			printf("%s\n", fmt_buf );

			p_fld = p_reg+1;
	      		// while (((char*)p_fld < ((char*)&reg_desc[0] + sizeof(reg_desc))) && 
			while((p_fld->desc_type == TYP_FLD) && (p_fld->addr == reg_addr))
			{
				unsigned int mask;
				int bit, lo, hi;

				fmt_buf[0] = 0;

				/* generate the fld mask */
				lo = p_fld->rst_val;
				hi = p_fld->mask;
				mask = 0;
				for (bit = lo; bit <= hi; bit++)
					mask |= (1 << bit);

				/* re-use line */
				printf( "   %-24s [%2d:%2d] : [0x%X]   ", p_fld->name, hi, lo, (mask & reg_val) >> lo);

       
				if (*(p_fld->desc) != '\0')
				{
					sprintf( line, "  %s", p_fld->desc );
					//strcat( fmt_buf, line );  
					copy_to_fmt_buf( &fmt_buf[ strlen(fmt_buf)-1 ], line );

					printf("%s", fmt_buf );
					fmt_buf[0] = 0;
				}
				if (*(p_fld->typ) != '\0')
				{
					sprintf( line, "(cause: %s)", p_fld->typ);
					//strcat( fmt_buf, line );
					copy_to_fmt_buf( &fmt_buf[ strlen(fmt_buf)-1 ], line );
				}
				printf("%s\n", fmt_buf );

				p_fld++;
			}
			return 1;
		}
		p_reg++;
	}
	return 0; // not found
}


void reg_list_dump (reg_desc_t *p_reg )
{
	if (!p_reg) return;
	while(p_reg->desc_type != TYP_NONE) {
		if (p_reg->desc_type == TYP_REG) {
      			printf ("%-24s : [0x%08x]  \t%s\n", p_reg->name, p_reg->addr, p_reg->desc);
		}
		p_reg++;
	}
}
