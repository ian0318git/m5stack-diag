/* $Id: query.c,v 1.7 2020/06/23 05:41:21 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/query.c,v $
 *------------------------------------------------------------------
 *
 * Copyright (c) 2011-2020 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

/*
** Mechanism to query the user for test parameters.
** Query the user for the test parameters dictated by the queryflag bits.
** There must be an argument of size int for each bit set in queryflag.
** The current value of the argument may be displayed as the default and
** will be used if the user simply enters a <ret>.
**
** Note the case of QU_R_WR (read or write; case 4).  If this case is
** specified 'r' (for read) and the QU_VALUE (specify the value to be
** written) bit is set the routine will automatically skip over the
** query and the pointer arg supplied for it.  Additionally, if the user
** chooses "write" and the QU_OPSIZ bit is set, the value to be written
** is tested to see if it "fits" into the specified operation size.
*/
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

#include "endians.h"
#include "types.h"
#include "queryflags.h"
#include "testmem.h"
#include "bitdef.h"
#include "proto.h"
#include "menu.h"
#include "common.h"

static
char *querystrings[] = {  /* strings to match the queryflag bits */
    "source address",
    "destination address",
    "start address",
    "test size or length in bytes",
    "read or write the location (r/w)",
    "pattern to be written",
    "number of passes",
    "operation size 'd'ouble-long, 'l'ong, 'w'ord, or 'b'yte",
    "incrementing pattern",
    "scope trigger address",
    "abbreviated test?",
};
static char opsizstr[] = "bwlldddd";  /* must be in this order with 4 d's */


/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * Function: take_0x_addr
 * Given addr_p, a pointer to an input buffer containing a hex address,
 * shift the pointer past the optional prefix 0x and return the new ptr;
 * otherwise (no prefix), simply return the input pointer.
 */
char *
take_0x_addr (char *addr_p)
{
    char c;

    for (c = *addr_p; isspace(c); c = *(++addr_p)) {
        /* scan to first non-whitespace char */
    }
    if ((c == '0') && (*(addr_p + 1) == 'x')) {
        addr_p += 2;  /* pass "0x" prefix */
    }
    return(addr_p);
}

void    
query_user_ex(QUERYFLAG queryflag, unsigned long *tmp0, unsigned long *tmp1, 
	      unsigned long *tmp2, unsigned long *tmp3)
{
    unsigned int i;
    unsigned long temp, mask, wrpat, *temp_ptr;
    char rd_wr;
    char buffer[80];
    unsigned long *arg_list[4];
    int cnt;

    if (sizeof(arg_list)/sizeof(long) > 4) {
#if defined(TACHI) || defined(TABEIM)
	printf("sizeof(arg_list) = %d; sizeof(long) = %d; num_of_arg= %d\n",
	       sizeof(arg_list), sizeof(long), sizeof(arg_list)/sizeof(long));
#else 
	printf("sizeof(arg_list) = %ld; sizeof(long) = %ld; num_of_arg= %ld\n",
	       sizeof(arg_list), sizeof(long), sizeof(arg_list)/sizeof(long));
#endif
	printf("file: %s function:%s -- incorrect number of arguments\n", 
	       __FILE__, __FUNCTION__);
        exit (0);
    } else {
        arg_list[0] = tmp0;
        arg_list[1] = tmp1;
        arg_list[2] = tmp2;
        arg_list[3] = tmp3;
    }
    
    wrpat = mask = rd_wr = 0;

    cnt = 0;
    for(i=0; i < (sizeof(QUERYFLAG)*8); i++) {
	if((queryflag >> i) & 1) {                      /* bit is set */
            temp_ptr = (unsigned long *)arg_list[cnt++];/* get next argument */
	    temp = *temp_ptr;
	    switch(i) {
	    case QU_VALUE_BIT:
		/*
		** If user wishes to read, there is no
		** need to query for write value.  Also
		** protect against operation size checking
		** on "wrpat" by turning off the QU_VALUE
		** bit.
		*/
		if(rd_wr == 'r') {
		    queryflag &= ~QU_VALUE;
		    break;
		}
		/* fall through */
	    case QU_SOURCE_BIT: case QU_DEST_BIT:
	    case QU_START_BIT: case QU_SIZE_BIT:
	    case QU_TRIGGER_BIT: case QU_PASSES_BIT:
		sprintf(buffer, "Enter in hex the %s", querystrings[i]);
		temp = gethex_answer(buffer, temp, 0, (unsigned long)0xFFFFFFFFFFFFFFFFULL);
		if( i == QU_VALUE_BIT )
                    wrpat = temp;
		break;
	    case QU_R_WR_BIT:
		sprintf(buffer, "Do you wish to %s?",
			querystrings[QU_R_WR_BIT]);
		rd_wr = temp = getc_answer(buffer, "rw", temp);
		break;
	    case QU_ABBREV_BIT:
		sprintf(buffer,querystrings[QU_ABBREV_BIT]);
		if( getc_answer(buffer,"yn",'n') == 'y' )
                    temp = 1;
		else
                    temp = 0;
		break;
	    case QU_OPSIZ_BIT:
                if (temp < 1)
                    temp = 'b';
                else
                    temp = opsizstr[temp-1];  /* convert to ascii char */
		if(!strchr(opsizstr, temp)) temp = 'b';  /* validate it */
      		sprintf(buffer, "Enter the %s",
			querystrings[QU_OPSIZ_BIT]);
		temp = getc_answer(buffer, opsizstr, temp);
		switch(temp) {  /* convert back to a number */
		case 'd': /* double long (64 bit) */
		    temp = 8;
		    mask = 0;
		    break;
		case 'l':
		    temp = 4;
		    mask = (unsigned long)0xffffffff00000000ull;
		    break;
		case 'w':
		    temp = 2;
		    mask = (unsigned long)0xffffffffffff0000ull;
		    break;
		case 'b':
		    temp = 1;
		    mask = (unsigned long)0xffffffffffffff00ull;
		    break;
		}
		if((queryflag & QU_VALUE) && (wrpat & mask)) {
		    sprintf(buffer, "Value (0x%lx) is larger than the op. "
			    "size - respecify? y/n", wrpat);
		    if(getc_answer(buffer, "yn", 'y') == 'y') {
			i--;  /* do this (QU_OPSIZ) over again */
                        cnt--;  
			continue;
		    }
		}
		break;
	    default:
		continue;  /* in case of unsupported bit */
	    }
	    *temp_ptr = temp;  /* set new value */
	}
    }
}

void    
query_user(QUERYFLAG queryflag, ...)
{
    unsigned int i;
    va_list argptr;
    unsigned temp, mask, wrpat, *temp_ptr;
    char rd_wr;
    char buffer[80];
    int last_arg_residual = 0;

    temp_ptr = NULL;
    wrpat = mask = rd_wr = 0;
    temp_ptr = (unsigned *)&temp;
    va_start(argptr, queryflag);
    for(i=0; i < (sizeof(QUERYFLAG)*8); i++) {
	if((queryflag >> i) & 1) {                      /* bit is set */
            if(!last_arg_residual) {
                temp_ptr = va_arg(argptr, unsigned *);      /* get next argument */
            }
	    temp = *temp_ptr;
	    switch(i) {
	    case QU_VALUE_BIT:
		/*
		** If user wishes to read, there is no
		** need to query for write value.  Also
		** protect against operation size checking
		** on "wrpat" by turning off the QU_VALUE
		** bit.
		*/
		if(rd_wr == 'r') {
		    queryflag &= ~QU_VALUE;
		    break;
		}
		/* fall through */
                case QU_SOURCE_BIT:
                case QU_DEST_BIT:
                case QU_START_BIT:
                case QU_SIZE_BIT:
                case QU_TRIGGER_BIT:
                case QU_PASSES_BIT:
		sprintf(buffer, "Enter in hex the %s", querystrings[i]);
		temp = gethex_answer(buffer, temp, 0, 0xFFFFFFFF);
		if( i == QU_VALUE_BIT ) wrpat = temp;
		break;
	    case QU_R_WR_BIT:
		sprintf(buffer, "Do you wish to %s?",
			querystrings[QU_R_WR_BIT]);
		rd_wr = temp = getc_answer(buffer, "rw", temp);
		break;
	    case QU_ABBREV_BIT:
		sprintf(buffer,querystrings[QU_ABBREV_BIT]);
		if( getc_answer(buffer,"yn",'n') == 'y' ) temp = 1;
		else temp = 0;
		break;
	    case QU_OPSIZ_BIT:
		temp = opsizstr[temp-1];  /* convert to ascii char */
		if(!strchr(opsizstr, temp)) temp = 'b';  /* validate it */
      		sprintf(buffer, "Enter the %s",
			querystrings[QU_OPSIZ_BIT]);
		temp = getc_answer(buffer, opsizstr, temp);
		switch(temp) {  /* convert back to a number */
		case 'l':
		    temp = 4;
		    mask = 0;
		    break;
		case 'w':
		    temp = 2;
		    mask = 0xffff0000;
		    break;
		case 'b':
		    temp = 1;
		    mask = 0xffffff00;
		    break;
		}
		if((queryflag & QU_VALUE) && (wrpat & mask)) {
		    sprintf(buffer,
 "Value (0x%x) is larger than the op. size - respecify? y/n",
			    wrpat);
		    if(getc_answer(buffer, "yn", 'y') == 'y') {
			i--;  /* do this (QU_OPSIZ) over again */
                        last_arg_residual = 1;
			continue;
		    }
		}
		break;
	    default:
                last_arg_residual = 0;
		continue;  /* in case of unsupported bit */
	    }
            last_arg_residual = 0;
	    *temp_ptr = temp;  /* set new value */
	}
    }
    va_end(argptr);
}

int
getc_answer(char * msg, char *cmpstr, char curval )
{
    char buffer[4];

    while(1) {
	printf("%s  [%c]:  ", msg, curval);
	get_line(buffer,sizeof(buffer));
	if(buffer[0] == '\0' || buffer[0] == '\r' || buffer[0] == '\n')
	    return(curval);
	if(strchr(cmpstr, buffer[0])) return(buffer[0]);
    }
}

unsigned long
gethex_answer(char *msgstr, unsigned long currentval, unsigned long min, 
	      unsigned long max)
{
  char buffer[32];
  utype_t newval;

  while(1) {
    printf("%s [0x%lx]:  ", msgstr, currentval);
    fflush(stdout);
    get_line(buffer,sizeof(buffer));
    if(buffer[0] == '\0' || buffer[0] == '\r' || buffer[0] == '\n')
      return(currentval);
    if((getnum(take_0x_addr(buffer), 16, (uint *)&newval)) <= 0 || 
       (newval < min) || (newval > max)) {
      printf("valid entry 0x%lx to 0x%lx...try again\n", min, max);
      fflush(stdout);
      continue;
    } else {
        return((ulong)newval);
    }
  }
}

int
getdec_answer(char *msgstr, uint currentval, uint min, 
	      uint max)
{
  char buffer[32];
  utype_t newval;

  while(1) {
    printf("%s [%d]:  ", msgstr, currentval);
    fflush(stdout);
    get_line(buffer,sizeof(buffer));
    if(buffer[0] == '\0' || buffer[0] == '\r' || buffer[0] == '\n')  
      return(currentval); /* null line returns current value */
    if((getnum(buffer,10, (uint *)&newval)) <= 0 || (newval < min) 
       || (newval > max)) {
	printf("valid entry %d to %d ... try again\n", min, max);
        fflush(stdout);
	continue;
    } else return(newval);
  }
}

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * Function: getdec_token
 * Given *cp_p, the location of a pointer to an input buffer, scan the
 * line for the next decimal token.  Return 0 if End Of Line is reached,
 * or -1 if a non-decimal character is encountered.  Otherwise, convert
 * the number to hex, and return it.  Advance the pointer to the next
 * character beyond the token unless in error.
 */
int
getdec_token (char **cp_p)
{
    char ch, *cp;
    int numchar;
    utype_t number;

    cp = *cp_p;
    /*
     * Skip blanks
     */
    while (*cp == ' ') {
	cp++;
    }
    if (*cp == '\0') {
	*cp_p = cp;
	return(0);
    }
    numchar = getnnum(cp, 10, (utype_t *)&number, 0);
    cp += numchar;
    *cp_p = cp;
    ch = *cp;
    if ((ch == ' ') || (ch == '\0')) {
	/*
	 * Expect the number token to be terminated by <SP> or EOL.
	 * Any other character found is verboten.
	 */
	return(number);
    } else {
	return(-1);
    }
}

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * Function: get_user_test_options
 *
 * Called only when the diagflag D_SET_OPTIONS is turned on by the user;
 * otherwise, the current setting of bits in the given test_options is
 * used.  Each bit defined for the calling diagnostic represents some
 * local option that may be tested for at various points in its code.
 * 
 * This function interacts with the user to allow changes to the
 * calling diagnostic's test_options, which is usually declared as a
 * global to allow access from any function within the diagnostic.
 *
 * Returns the value of test_options, which may or may not be changed
 * from the input value.
 *
 */
unsigned long
get_user_test_options (unsigned char *diag_id,
		       bitdef_t option_defs[],
		       unsigned long options_mask,
		       unsigned long curr_test_options)
{
    unsigned long input_test_options;
    printf("\n%s test options available:\n%#.8lx", diag_id, 
	   options_mask);
    printf("\nCurrent options set:\n%#.8lx", curr_test_options);

    input_test_options = gethex_answer("\nEnter option setting",
				       curr_test_options, 0, options_mask);
    if (input_test_options != curr_test_options) {
	printf("\nChange in options -- currently set:\n%#.8lx", 
	       input_test_options);
    }
    return(input_test_options);

}
/* End of module */

/******** History ******** 
$Log: query.c,v $
Revision 1.7  2020/06/23 05:41:21  kehuang2
Clear mark

Revision 1.6  2020/06/23 03:51:20  kehuang2
Define for tabei-M

Revision 1.5  2016/04/20 07:03:33  benchen2
merge tachi_branch to maintrunk

Revision 1.4.40.1  2015/06/11 02:01:04  tirawan
Add files for Tachi BMC project

Revision 1.4  2013/10/08 08:48:26  tirawan
Woodlawn collapsed to main trunk

Revision 1.3  2012/06/05 09:33:45  aarwang
- Clean up compiler warnings.

Revision 1.2  2012/03/28 00:38:14  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:05  ptong
Initial archive of ng_diag module


$Endlog$
*/
