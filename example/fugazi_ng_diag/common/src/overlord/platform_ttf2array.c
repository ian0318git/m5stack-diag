/* $Id: platform_ttf2array.c,v 1.1 2013/05/09 05:42:37 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/platform_ttf2array.c,v $
 *------------------------------------------------------------------
 * platform_ttf2array.c  this program open ttf file, read its contents,
 * swap bits within a byte. for example, 0xF7 will become EF
 * (need to do this when we get dash fpga (ttf file) from hardware. )
 *
 * by: mcharon
 *
 * Copyright (c) 2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>

/*************************************************************
 * Function: swapbyte
 * Description:  swap bits of a byte. for example, 0xF7 will become EF.
 * Input: c , byte to swap
 * Output: return byte that has been swapped.
 *************************************************************
 */
unsigned char
swapbyte (unsigned char c)
{
    int i ;
        unsigned char result=0;
        
        for(i=0;i<8;++i) {
                result=result<<1;
                result|=(c&1);
                c=c>>1;
        }
        return result;
}


/*************************************************************
 * Function: ttf2array
 * Description:  open ttf file, read its contents, swap bits,
 * and stored in array. 
 * Input: size - number of bytes to read,
 *        file - file name
 *        fpga - array to store file content.
 * Output: return number of bytes found from ttf file
 *************************************************************
 */
int ttf2array (int size, const char *file, unsigned char *fpga) {

    FILE *fp_s;
    unsigned int c, i, line;
    unsigned int val;
    char *tmp = fpga;
    
    fp_s = fopen(file, "r");

    if (!fp_s) {
        printf("\n\ncan't open %s\n\n", file);
        exit(0);
    }

    i = 1;
    line = 1;
    while (!feof(fp_s)) {
        if (fscanf(fp_s, "%d", &val) == EOF) {
            printf("problem scanning number.\n");
            goto out;
        }

        *tmp++ = swapbyte(val);

        if ((c = fgetc(fp_s)) == EOF) {
            printf("end of file. no more characters. %d bytes.  %d lines\n", i, line);
            goto out;

        } else {
            if (c == ',') {
                
            } else {
                printf("File read successfully. %d bytes found.\n", i);
                goto out;
            }
        }
        i++;
    }
    
 out:
    fclose(fp_s);
    
    return i;
}

/*-------------------------------------------------
$Log: platform_ttf2array.c,v $
Revision 1.1  2013/05/09 05:42:37  alpeng
moving overlord common code from x86

Revision 1.2  2012/11/06 20:39:51  mcharon
add headers/cleanup/remove unneeded functions/files

Revision 1.1  2012/09/27 22:06:14  mcharon
support converting ttf file 2 arrayd


$Endlog$
*/
