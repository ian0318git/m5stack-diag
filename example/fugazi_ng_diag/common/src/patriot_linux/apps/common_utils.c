/* $Id: common_utils.c,v 1.1 2014/03/25 02:12:32 huanngo Exp $
 * $Source: 
 *------------------------------------------------------------------
 *
 * Filename: common_utils.c
 *
 *
 * Copyright (c) 2007-2014 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *
 *------------------------------------------------------------------
 */

/* Includes. */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "endians.h"
#include "defs.h"
#include "patriot_main.h"
#include "router_if.h"
#include "common_utils.h"

boolean disp_mem = FALSE;
boolean chain1 = FALSE;
boolean chain2 = FALSE;
boolean chain3 = FALSE;
extern fe_packet_t *tx_packet_p;
extern uchar err_msg[];
extern uchar err_msg1[];
extern uchar err_msg2[];
extern uchar err_msg3[];
extern uchar dismem_msg[];

extern uint32_t p1021_i2c_read_fpga_byte(uint32_t, volatile uchar *);
extern uint32_t p1021_i2c_write_fpga_byte(uint32_t, uchar);
extern int ds3170_read(uchar *, uint);
extern int ds3170_write(uchar , uint);

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



/*
** Get a line from the standard input device.
** Return or linefeed terminates the line.
** Note that there must be room in the buffer for a terminator.
*/
int
get_line(char *buffer, int bufsiz)
     /* buffer : character buffer pointer */
     /* bufsiz : character buffer size */
{
  char c, *bptr;
  int count;

  bufsiz -= 1;  /* make room for the terminator */
  bptr = buffer;
  count = 0;

  while(1) {
    c = getchar();
    switch(c) {
    case '\r':
    case '\n':
      goto terminate;
    case '\b':  /* backspace character */
    case '\177':  /* delete key */
      if(count) {  /* are there characters in the buffer? */
	puts("\b \b");  /* erase one */
	--count;
	--bptr;
      } else putchar('\007');  /* ring the bell */
      break;
    case '\003':  /* control C */
      bptr = buffer;
      count = 0;
      goto terminate;  /* effectively kills the line */
    default:
      if(count < bufsiz) {  /* there is room in the buffer */
	++count;
	if(c != '\033') putchar(c);  /* don't echo <ESC> */
	*bptr++ = c;
	break;
      } else {
	puts("\007\n*** line too large ***\n");
	goto terminate;
      }
    }
  }
terminate:
  *bptr = '\0';  /* terminate the line */
  putchar('\n');
  return(count);
}


unsigned long
gethex_answer(char *msgstr, unsigned long currentval, unsigned long min,
	      unsigned long max)
{
  char buffer[32];
  ulong newval;

  int len=0;

  while(1) {
    printf("%s [0x%lx]:  ", msgstr, currentval);
    fgets((((char *)buffer)), sizeof(buffer), (stdin));

    if ((len = strlen(buffer)) > 1) {
        if (buffer[len-1] == '\r' || buffer[len-1] == '\n') {
            buffer[len-1] = '\0';
        }
    }
    if(buffer[0] == '\0' || buffer[0] == '\r' || buffer[0] == '\n')
      return(currentval);
    if((getnum(take_0x_addr(buffer), 16, &newval)) <= 0 ||
       (newval < min) || (newval > max)) {
      printf("valid entry 0x%x to 0x%x...try again\n", min, max);
      continue;
    } else {
        return((ulong)newval);
    }
  }
}

int
getc_answer(char * msg, char *cmpstr, char curval )
{
    char buffer[4];
    char *test;
    int abc;

    while(1) {
	printf("%s  [%c]:  ", msg, curval);
	abc = get_line(buffer, sizeof(buffer));
	if(buffer[0] == '\0' || buffer[0] == '\r' || buffer[0] == '\n')
	    return(curval);
	if(strchr(cmpstr, buffer[0])) return(buffer[0]);
    }
}

unsigned int
getdec_answer(char *msgstr, unsigned int currentval, unsigned int min, 
	      unsigned int max)
{
  char buffer[32];
  unsigned int newval;

  int len=0;

  while(1) {
    printf("%s [%d]:  ", msgstr, currentval);
    fgets((((char *)buffer)), sizeof(buffer), (stdin));
    if ((len = strlen(buffer)) > 1) {
        if (buffer[len-1] == '\r' || buffer[len-1] == '\n') {
            buffer[len-1] = '\0';
        }
    }

    if(buffer[0] == '\0' || buffer[0] == '\r' || buffer[0] == '\n')  
      return(currentval); /* null line returns current value */
    if((getnum(buffer,10, &newval)) <= 0 || (newval < min) 
       || (newval > max)) {
	printf("valid entry %d to %d ... try again\n", min, max);
	continue;
    } else return(newval);
  }
}

/**********************************************************************
 *
 * Function: new_register_read
 *
 * This function reads and returns the value from a specified address.
 *
 * Input :  reg_info_t * - points to the register test info struct.
 *          base_addr - points to the tested register.
 *          buf * - read data buffer pointer
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
uint32_t new_register_read(reg_info_t *reg_ptr, ulong reg_addr, 
                           volatile uchar *buf, int access_type)
{
    if (access_type == I2C_BUS) {
        return (p1021_i2c_read_fpga_byte(reg_addr, buf));
    } else {
        return (ds3170_read((uchar *)buf, reg_addr));
    }
    return (PASSED);
}

/**********************************************************************
 *
 * Function: new_register_write
 *
 * This function writes a value to a specified address.
 *
 * Input : reg_info_t * - points to the register test info struct.
 *         base_addr    - points to the tested based register.
 *         value        - data to be written.
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static uint
new_register_write(reg_info_t *reg_ptr, ulong reg_addr, uchar value,
                    int access_type)
{
    if (access_type == I2C_BUS) {
        return (p1021_i2c_write_fpga_byte(reg_addr, value));
    } else {
        return  (ds3170_write(value, reg_addr));
    }
    return (PASSED);
}

/**********************************************************************
 *
 * Function: register_display
 *
 * For each register from reg_ptr, this function just displays the 
 * current value
 *
 * Input : *reg_ptr    - Address of the module addr register
 *         access_type - SPI or I2C bus
 *
 * Output: Always return PASSED
 *
 **********************************************************************
 */
int
register_display(reg_info_t *reg_ptr, int access_type)
{
    uchar readval;
    ulong reg_addr;
    int rc;

    while (reg_ptr->size.size != 0) {
	reg_addr = reg_ptr->offset;
	/*
	 * display current value
	 */
	printf("\n %25s",reg_ptr->name);
	rc = new_register_read(reg_ptr, reg_addr, &readval, access_type);
	if (rc != PASSED) {
	    printf("\nRegister display failed when reading %s"
		   "Register at %#x.\n",  reg_ptr->name, reg_addr);
	    return(FAILED);
	}
	printf(" , 0x%08x = 0x%.08x, (mask:0x%.08x, size:%d)",
	       reg_addr, readval, reg_ptr->mask, reg_ptr->size.size);
	
	reg_ptr++;
    }
    return(PASSED);
}

/**********************************************************************
 *
 * Function: register_alter
 *
 * For each register from reg_ptr, this function checks for accessibility
 * and displays the current value, and allows a modification if possible.
 *
 * Input :  *reg_ptr    - Address of the module addr register
 *          access_type - SPI or I2C bus
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
register_alter(reg_info_t *reg_ptr, int access_type)
{
    int tmp;
    uchar readval;
    uint read_ival;
    ulong reg_addr;
    char buffer[48], *c_ptr;

    printf("\n in register_alter");

    while (reg_ptr->size.size != 0) {
        reg_addr = reg_ptr->offset;

	/*
	 * display current value
	 */
	printf("\n %25s",reg_ptr->name);
	new_register_read(reg_ptr, reg_addr, &readval, access_type);
	
	printf(" , %#.8x = %#.8x, (mask:%#.8x, size:%d)",
	       reg_addr, readval, reg_ptr->mask, reg_ptr->size.size);
	
	printf("\n");
	printf(" Command : \n");
	printf("\'x\', \'q\' : exit / quit.\n");
	printf("\',\', \'p\' : go to previous register.\n");
	printf("\'r\', \'.\' : stay in the current location.\n");
	printf(" Press Enter to go to the next register.\n ");
	printf(" Type new register value or command :");

	c_ptr = buffer;
	get_line(c_ptr,sizeof(buffer));
	switch(*c_ptr) {
	case 'x':
	case 'q':
	    return(0);	/* quit */
	case ',':
	case 'p':	/* prev location */
	    reg_ptr--;
	    /* fall through */
	case 'r':
	case '.':	/* same location */
	    continue;
	case 0:
	    break;	/* next location */
	default:
	    tmp = getnnum(c_ptr,16, &read_ival,0);
		readval = (uchar)read_ival;
	    if(tmp == 0) {
		printf("bad value \"%s\"\n",c_ptr);
		continue;	/* same location again */
	    } else {
		if (reg_ptr->type & READ_ONLY) {
		    printf(" READ ONLY");
		    continue;	/* same location again */
		} else {
		    new_register_write(reg_ptr, reg_addr, readval, access_type);
		    reg_ptr--;	/* redisplay this value, to verify the write */
		    c_ptr += tmp;
		    if (*c_ptr == '.')
			continue;	/* same location */
		}
		break;	/* next location */
	    }
	}
	
	reg_ptr++;
    }
    return (PASSED);
}


/**********************************************************************
 *
 * Function: register_tests
 *
 * For each register from reg_ptr, this function checks for accessibility
 * and does a ripple 1 and a ripple 0 test if applicable (not all registers
 * are W/R register).
 *
 * Input : Address of the base register, info for all registers
 *
 * Output: PASSED/FAILED
 **********************************************************************
 */
int 
register_tests(reg_info_t *reg_ptr, int access_type)
{
    int  i, rc;
    ulong reg_addr;
    uint size;
    uchar readval, original_data, temp, data;

    while (reg_ptr->size.size != 0) {
	printf("\nTesting Register: %s\n", reg_ptr->name);fflush(0);
	reg_addr = reg_ptr->offset;
        /*
         * Test a register if it's a R/W register
         */
        if ((reg_ptr->type & (READ_ONLY | WRITE_ONLY)) == READ_WRITE) {
	    if (reg_ptr->type & SAVE_RESTORE) {
		/* Save and restore the original data */
		rc = new_register_read(reg_ptr, reg_addr, &original_data,
				       access_type);
		if (rc != PASSED) {
		    sprintf(err_msg,
			    "\n%s, [#%d]:%s Register first read failed at offset %#x\n",
			    __FUNCTION__, __LINE__,
			    reg_ptr->name, reg_ptr->offset);
		    print_err(FALSE, err_msg, LVL_1);
		    return(FAILED);
		}
		/* If the mask bits is set (read/writeable), the unmasked bits
		 * (read only/write only, or reserved) must be preserved.
		 */
		/* Some devices have read only bit that may change from time
		 * to time. These read only bits cannot be trusted and tested.
		 * (CSCso39166)
		 */
	    }
	    size = reg_ptr->size.size;
	    
	    /*
	     * ripple 1 test
	     */
	    printf("\nRipple 1 test");fflush(0);
	    for (i = 0; i < (size * 8); i++) {
		temp = (1 << i) & reg_ptr->mask;
		if (!temp)
		    continue;
		
		rc = new_register_write(reg_ptr, reg_addr, temp, access_type);
		if (rc != PASSED) {
		    sprintf(err_msg,
			    "\n%s, [#%d]:Ripple one test failed when writing %s "
			   "register at offset %#x with %#x\n", __FUNCTION__, __LINE__,
			   reg_ptr->name, reg_ptr->offset, temp);
		    print_err(FALSE, err_msg, LVL_1);
		    return(FAILED);
		}
		
		rc = new_register_read(reg_ptr, reg_addr, &readval, access_type);
		if (rc != PASSED) {
		    sprintf(err_msg,
			   "\n%s, [#%d]:Ripple one test failed when reading %s "
			   "register at offset %#x with %#x\n", __FUNCTION__, __LINE__,
			   reg_ptr->name, reg_ptr->offset, temp);
		    print_err(FALSE, err_msg, LVL_1);
		    return(FAILED);
		}
		
		if ((readval&reg_ptr->mask) != temp) {
		    sprintf(err_msg,
			    "\n%s, [#%d]:Ripple one test failed when accessing %s "
			   "Register at %#x. Expect: %#x, Read: %#x.\n", __FUNCTION__,
			   __LINE__, reg_ptr->name, reg_addr, temp, readval);
		    print_err(FALSE, err_msg, LVL_1);
#ifdef REG_TEST_DEBUG
			temp = 0xBB;
			break;
#endif
			return(FAILED);
		    }
	    }
	    
#ifdef REG_TEST_DEBUG
		if (temp == 0xBB) {
		    reg_ptr++;
		    continue;
		}
#endif
		
		/*
		 * ripple 0 test
		 */
		printf("\nRipple 0 test");fflush(0);
		for (i = 0; i < (size * 8); i++) {
		    temp = (1 << i) & reg_ptr->mask;
		    if (!temp)
			continue;
		    temp = (~(1 << i)) & reg_ptr->mask;
		    
		    rc = new_register_write(reg_ptr, reg_addr, temp, access_type);
		    if (rc != PASSED) {
			sprintf(err_msg,
				"\n%s, [#%d]:Ripple zero test failed when writing %s"
			       "Register at %#x.\n", __FUNCTION__, __LINE__,
			       reg_ptr->name, reg_addr);
			print_err(FALSE, err_msg, LVL_1);
			return(FAILED);
		    }
		    
		    rc = new_register_read(reg_ptr, reg_addr, &readval,
					   access_type);
		    if (rc != PASSED) {
			sprintf(err_msg,
				"\n%s, [#%d]:Ripple zero test failed when reading %s"
			       "Register at %#x.\n", __FUNCTION__, __LINE__,
			       reg_ptr->name, reg_addr);
			print_err(FALSE, err_msg, LVL_1);
			return(FAILED);
		    }
		    
		    if ((readval&reg_ptr->mask) != temp) {
			sprintf(err_msg,
				"\n%s, [#%d]:Ripple zero test failed when accessing "
			       "%s Register at %#x. Expect: %#x, Read: "
			       "%#x.\n", __FUNCTION__, __LINE__,
			       reg_ptr->name, reg_addr, temp, readval);
			print_err(FALSE, err_msg, LVL_1);
			return(FAILED);
		    }
		}
		
		/*
		 * pattern test
		 */
		printf("\nPattern test");fflush(0);
		data = (uchar)PATTERN;
		for (i=0;i<2;i++){
		    temp = data &reg_ptr->mask;
		    
		    if (!temp) {
			continue;
		    }
		    
		    rc = new_register_write(reg_ptr, reg_addr, temp, access_type);
		    if (rc != PASSED) {
			sprintf(err_msg,
				"\n%s, [#%d]:Pattern test failed when writing %s "
			       "Register at %#x.\n", __FUNCTION__, __LINE__,
			       reg_ptr->name, reg_addr);
			print_err(FALSE, err_msg, LVL_1);
			return(FAILED);
		    }
		    
		    rc = new_register_read(reg_ptr, reg_addr, &readval,
					   access_type);
		    if (rc != PASSED) {
			sprintf(err_msg,
				"\n%s, [#%d]:Pattern test failed when reading %s "
			       "Register at %#x.\n", __FUNCTION__, __LINE__,
			       reg_ptr->name, reg_addr);
			print_err(FALSE, err_msg, LVL_1);
			return(FAILED);
		    }
		    
		    if ((readval&reg_ptr->mask) != temp) {
			sprintf(err_msg,
				"\n%s, [#%d]:Pattern test failed when accessing %s "
			       "Register at %#x. Expect: %#x, Read: %#x.\n",
			       __FUNCTION__, __LINE__,reg_ptr->name, reg_addr,
			       temp, readval);
			print_err(FALSE, err_msg, LVL_1);
			return(FAILED);
		    }
		    
		    data = (uchar)~PATTERN; /* complement data pattern */
		}
		
		/*
		 * restore reset value
		 */
		if (reg_ptr->type & SAVE_RESTORE) {
		    /* Restore the save value */
		    rc = new_register_write(reg_ptr, reg_addr, original_data,
					    access_type);
		} else {
		    /* Restore reset value */
		    rc = new_register_write(reg_ptr, reg_addr, reg_ptr->reset_val,
					    access_type);
		}
		
		if (rc != PASSED) {
		    sprintf(err_msg,
			    "\n%s, [#%d]:Write failed when %s %s Register at %#x",
			   (reg_ptr->type & SAVE_RESTORE) ? "restoring\n" :
			   "resetting\n", __FUNCTION__, __LINE__, reg_ptr->name, reg_addr);
		    print_err(FALSE, err_msg, LVL_1);
		    return(FAILED);
		}
	}
	reg_ptr++;
    }
    return(PASSED);
}


void 
msleep (int n)
{

    usleep(n * 1000);
    
}


/**********************************************************************
 *
 * Function: wastetime
 *
 * Description: delay for 1 usec
 *
 * Input : usec - number of usec for delay
 *
 * Output: N/A
 *
 **********************************************************************
 */
void wastetime (long usec)
{
    unsigned long counts, i;
    unsigned long time_low;

    /* Multiply by factor to make usec accurate
       for CPU speed 400 MHz, multiply by 27
       for CPU speed 667 MHz, multiply by 45 */
    counts = usec * 45;
    for (i = 0; i < counts; i++) {
	asm volatile ("ori 0,0,0");
    }
}    


int
alt_cpu_regs(unsigned long start_addr, int opsiz)
{
    union location {
	unsigned char byte;
	unsigned short word;
	unsigned lword;
    };
    register union location *addr;
    int tmp;
    register char *c_ptr;
    char inbuf[16];
    unsigned long val;
    
    /* Translate to the mapping address */
    addr = (union location *)(start_addr + ADRSPC_PQUICC_IMEMB);
    
    while(1) {
	printf("%.6lx = ", (unsigned long)addr);
	switch(opsiz) {
	case 2:
	    printf("%.4x",addr->word);
	    break;
	case 4:
	    printf("%.8x",addr->lword);
	    break;
	case 1:
	default:
	    opsiz = 1;
	    printf("%.2x",addr->byte);
	    break;
	}
	puts(" > ");
	c_ptr = inbuf;
	fgets((((char *)inbuf)), sizeof(inbuf), (stdin));
	
	switch(*c_ptr) {
	case 'x':
	case 'q': return(0);  /* quit */
	case ',':
	case 'p': /* prev location */
	    addr = (union location *)((unsigned long)addr - opsiz);
	    /* fall through */
	case 'r':
	case '.': /* same location */
	    continue;
	case 0: break; /* next location */
	default:
	    c_ptr = take_0x_addr(c_ptr);
	    tmp = getnum(c_ptr,16,&val);
	    if(tmp == 0) {
		printf("bad value \"%s\"\n",c_ptr);
		continue; /* same location again */
	    } else {
		switch(opsiz) {
		case 1:
		    addr->byte = val;
		    break;
		case 2:
		    addr->word = val;
		    break;
		case 4:
		    addr->lword = val;
		    break;
		}
		c_ptr += tmp;
		if(*c_ptr == '.')
		    continue;  /* same location */
	    }
	    break; /* next location */
	}
	addr = (union location *)((unsigned long)addr + opsiz);
	/* bump address */
    }
}



void
modify_cpu_regs(void)
{

    unsigned long addr;
    char ch;
    int op;

    addr = gethex_answer("Enter in hex the start address", 0, 0,
			 0xFFFFF);

    ch = getc_answer("Enter the operation size 'l'ong, 'w'ord or 'b'yte",
		     "lwb", 'b'); 
    switch(ch) {  /* convert back to a number */
    case 'l':
	op = 4;
	break;
    case 'w':
	op = 2;
	break;
    case 'b':
	op = 1;
	break;
    }

    alt_cpu_regs(addr, op);
    return;
}

/**********************************************************************
 *
 * Function: clean_err_msg_buf
 *
 * Description: Utility to set memory to 0
 *
 * Input : None
 *
 * Output: N/A
 *
 **********************************************************************
 */
void
clean_err_msg_buf(void)
{

    memset((uchar *)err_msg, 0, sizeof(1024));
    memset((uchar *)err_msg1, 0, sizeof(1024));
    memset((uchar *)err_msg2, 0, sizeof(1024));
    memset((uchar *)err_msg3, 0, sizeof(1024));
    memset((uchar *)dismem_msg, 0, sizeof(1024));

}

/**********************************************************************
 *
 * Function: print_err
 *
 * Description: Utility to print error message to the host
 *
 * Input : host_display - flag for prepare buf packet send to the host
 *         str   -  error message concatenate to the main buffer
 *         level -  to avoid overwritten by each error message we define each
 *                  level when it calling from the begin.
 *
 * Output: N/A
 *
 **********************************************************************
 */
void
print_err(boolean host_display, char *str, char level)
{
    unsigned char temp_msg[1024];
    unsigned int len = 0;

    memset((uchar *)temp_msg, 0, sizeof(temp_msg));

    switch (level)
    {
    case LVL_1 :
        sprintf(temp_msg, "\n%s",str);
        len = strlen(temp_msg);
        strncat(err_msg1, temp_msg, len);
        chain1 = TRUE;
        break;
    case LVL_2 :
        sprintf(temp_msg, "\n%s",str);
        len = strlen(temp_msg);
        strncat(err_msg2, temp_msg, len);
        chain2 = TRUE;
        break;
    case LVL_3 :
        sprintf(temp_msg, "\n%s",str);
        len = strlen(temp_msg);
        strncat(err_msg3, temp_msg, len);
        chain3 = TRUE;
        break;
    case LVL_X :
        sprintf(temp_msg, "\n%s",str);
        len = strlen(temp_msg);
        strncat(dismem_msg, temp_msg, len);
        disp_mem = TRUE;
        break;
    default:
        sprintf(temp_msg, "\n%s",str);
        len = strlen(temp_msg);
        strncat(err_msg, temp_msg, len);

        if (chain1) {
            len = strlen(err_msg1);
            strncat(err_msg, err_msg1, len);
            chain1 = FALSE;
        }
        if (chain2) {
            len = strlen(err_msg2);
            strncat(err_msg, err_msg2, len);
            chain2 = FALSE;
        }
        if (chain3) {
            len = strlen(err_msg3);
            strncat(err_msg, err_msg3, len);
            chain3 = FALSE;
        }
        if(disp_mem) {
            len = strlen(dismem_msg);
            strncat(err_msg, dismem_msg, len);
            disp_mem = FALSE;
        }
        break;
    }
    len = strlen(err_msg);
    if (len > 1024) {
        printf("\n**** Warning: err_msg buffer size is out of limit !****\n");
    }
    /* if err flag is Level 0 and host display flag is fully condition then just copy to the tx packet */
    if (host_display && (level == LVL_0)) {
        /* Clean up the tx packet */
        memset((uchar *)tx_packet_p, 0, sizeof(fe_packet_t));
        memcpy((char *)&(tx_packet_p->data[12]), (char *)&(err_msg[0]), 1024);
        /* Clean up the buffer so it can re-use */
        clean_err_msg_buf();
    }
}

/* End of File */
/*------------------------------------------------------------------------------
 * $Log: common_utils.c,v $
 * Revision 1.1  2014/03/25 02:12:32  huanngo
 * Adding patriot_linux directory to ng_diag code tree
 *
 * Revision 1.6  2012/12/03 12:35:16  steja
 * 1. Add Error message utility
 * 2. Fix Framer interrupt Diagnostic loopback
 *
 * Revision 1.5  2012/10/01 18:44:37  huanngo
 * Change the asm function from "msync" to "ori 0, 0, 0"
 *
 * Revision 1.4  2012/08/27 22:32:55  huanngo
 * Adjust the wastetime function when CPU speed goes from 400MHz to 667MHz
 *
 * Revision 1.3  2012/08/21 01:14:41  huanngo
 * Adding more printing when running register tests
 *
 * Revision 1.2  2012/05/08 23:52:54  huanngo
 * Support SM Patriot on ngd main code tree
 *
 * Revision 1.1.4.8  2012/04/12 18:37:02  huanngo
 * Clean up and cosmetic changes
 *
 * Revision 1.1.4.7  2012/02/06 22:29:04  huanngo
 * Update to not compile code using bitbake, use make with local kernel
 *
 * Revision 1.1.4.6  2012/01/09 23:06:17  huanngo
 * Support on xformers mips and informers and clean up
 *
 * Revision 1.1.4.5  2011/11/11 16:05:33  steja
 * Fix the Alter register function to work properly
 *
 * Revision 1.1.4.4  2011/10/27 09:35:08  steja
 * Update DS3170 BERT test
 *
 * Revision 1.1.4.3  2011/10/07 01:11:44  huanngo
 * Update code to support HDLC, SPI EEPROM and FPGA
 *
 * Revision 1.1.4.2  2011/08/18 19:43:21  huanngo
 * Update code to patriot2-branch
 *
 *------------------------------------------------------------------------------
 * $Endlog$
 *
 *------------------------------------------------------------------------------
 */

