/* $Id: srec2bin.c,v 1.2 2014/06/03 19:08:14 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/srec2bin.c,v $
 * Copyright (c) 2014 by Cisco Systems, Inc.
 * All rights reserved.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SREC_NEWLINE    (0) // Start of line
#define SREC_RECORDTYPE (1) // Sn
#define SREC_BYTECOUNT  (2) // Byte Count
#define SREC_ADDRESS    (3) // Address
#define SREC_DATA       (4) // Data
#define SREC_CHKSUM     (5) // Checksum
#define SREC_ENDOFLINE  (6) // End of a Line

#define LO (0)
#define HI (1)

#define FALSE (0)
#define TRUE  (1)

void print_help(void)
{
    printf("\n");
    printf("srec2bin: SREC file to BIN file conversion Utility v1.1\n");
    printf("\n");
    printf("Convert Motorola SREC (s19, s28, s37) files to a binary image file.\n");
    printf("Multiple SREC files can be overlayed onto a single binary image.\n");
    printf("\n");
    printf("Usage:  srec2bin binfile -{B|K|M|G} size [-d bv] [-s srec1 srec2 ... srecN]\n");
    printf("\n");
    printf("   binfile       (binary file name w/extension)\n");
    printf("   -B rom_size   (in Bytes)\n");
    printf("   -K rom_size   (in KB w/ 1KB = 1024B)\n");
    printf("   -M rom_size   (in MB w/ 1MB = 1024KB)\n");
    printf("   -G rom_size   (in GB w/ 1GB = 1024MB)\n");
    printf("   -d bv         (blank value for addresses not in SREC files)\n");
    printf("   -v level      (verbosity level)\n");
    printf("   -s srec(1..N) (N srec filenames w/extention\n");
    printf("\n");
    printf("NOTES:\n");
    printf("\n");
    printf("   Existing binary files are overwritten.\n");
    printf("   Command line switches are case sensitive.\n");
    printf("   The -s switch must be last, followed only by SREC filenames.\n");
    printf("   One of {B,K,M,G} must be used and 'size' must be an integer.\n");
    printf("   Later SREC files in list take precedence.\n");
    printf("   Attempts to write to values beyond specified ROM size will be ignored.\n");
    printf("   The default blank value is 0x00.\n");
    printf("   The default verbosity is level 0 (no output), max level is 1.\n");
    printf("\n");
    printf("EXAMPLES: To create a:\n");
    printf("  256KB binary file named 'image.bin' filled with all 0's.\n");
    printf("    srec2bin image.bin -K 256 -d 0\n");
    printf("\n");
    printf("  2MB binary file, 'fred.rom' from f1.s19, f2.mot, and f3.s37 w/blanks = 0xFF\n");
    printf("    srec2bin fred.bin -M 2 -s f1.s19 f2.mot f3.s37\n");
    printf("\n");
    printf("  128KB binary file, 'fred.rom' from f1.s19 with no screen output\n");
    printf("    srec2bin fred.bin -K 128 -v 0 -s f1.s19\n");
    printf("\n");
}

void terminate(int verbosity)
{
    if (verbosity)
	{
            printf("Hit RETURN to EXIT: ");
            getc(stdin);
	}
    exit(EXIT_SUCCESS);
}

int hexvalue(int c)
{
    if (('0' <= c)&&(c <= '9')) return c - '0';
    if (('A' <= c)&&(c <= 'F')) return c - 'A' + 10;
    if (('a' <= c)&&(c <= 'f')) return c - 'f' + 10;
    return -1;
}
#ifdef SREC2BIN
int srec2bin_main(int argc, char *argv[], int *addr_list,
                  int *byte_cnt_list)
#else
    int main(int argc, char *argv[])
#endif
{
    FILE *fp_bin, *fp_srec;
    char *fname_bin, *fname_srec;
    int state;
    int processing;
    int error;
    int type;
    int c;
    int nibble;
    int bytes;
    int records;
    int checksum;
    int byte_value;
    int nibble_value;
    int byte_count;
    int address;
    int value;
    int addr_bytes;
    int data_bytes;
    int rom_size, min_rom_size;
    int i;
    int verbosity;
    int s_position;
    int srec;

    unsigned char byte, blank;

    verbosity = 1;

    // Print help screen if no arguments given
    if (2>argc) // No command line arguments
	{
            print_help();
            terminate(verbosity);
	}

    // Scan command line for parameters
    verbosity = 1;
    blank = 0xFF;
    rom_size = 256*1024;
    min_rom_size = 0;

    for (i = 1; i < argc-1; i++)
	{
            if (!strcmp("-d", argv[i])) {
                blank = atoi(argv[i+1]);
            }
                
            if (!strcmp("-v", argv[i])) {
                verbosity = atoi(argv[i+1]);
            }
            if (!strcmp("-B", argv[i]))
                rom_size = atoi(argv[i+1]);
            if (!strcmp("-K", argv[i]))
                rom_size = 1024*atoi(argv[i+1]);
            if (!strcmp("-M", argv[i]))
                rom_size = 1024*1024*atoi(argv[i+1]);
            if (!strcmp("-G", argv[i]))
                rom_size = 1024*1024*1024*atoi(argv[i+1]);
            if (!strcmp("-s", argv[i])) {
                s_position = i+1;
            }
	}

    // Open output (binary) file for writing
    fname_bin = argv[1];
    fp_bin = fopen(fname_bin, "wb");
    if (NULL == fp_bin)
	{
            printf("Failed to open target binary file: <%s>.\n", fname_bin);
            terminate(verbosity);
	}

    // Create blank ROM image.
    byte = blank;
    for (i = 0; i < rom_size; i++)
        fwrite(&byte, 1, 1, fp_bin);

    if (verbosity)
	{
            printf("BIN file:..... %s\n", fname_bin);
            printf("ROM size:..... %i\n", rom_size);
            printf("Blank Value:.. 0x%2X\n", blank);
            printf("Verbosity:.... %i\n", verbosity);
            printf("SREC Files:... %i\n", argc-s_position);
	}

    // Process SREC files
    for (srec = s_position; srec < argc; srec++)
	{
            processing = TRUE;

            fname_srec = argv[srec];
            if (verbosity)
                printf("  <%s>... ", argv[srec]);
            fp_srec = fopen(fname_srec, "rt");
            if (NULL == fp_srec)
		{
                    if (verbosity)
                        printf("Failed to open %s", fname_srec);
                    processing = FALSE;
		}

            state = SREC_NEWLINE;
            nibble = HI;
            bytes = 0;
            records = 0;
            error = 0;

            while( (processing) && (EOF != (c = getc(fp_srec))) )
		{
                    switch (state)
			{
                        case SREC_NEWLINE:    // Start of line
                            switch (c)
                                {
                                case '\n':
                                case '\r':
                                case ' ':
                                case '\t':
                                    break;
                                case 's':
                                case 'S':
                                    // Next State Set-up
                                    state = SREC_RECORDTYPE;
                                    break;
                                default:
                                    processing = FALSE;
                                    error = TRUE;
                                }
                            break;

                        case SREC_RECORDTYPE: // Sn
                            switch (c)
                                {
                                case '0': 
                                case '1': 
                                case '2':
                                case '3':
                                case '5':
                                case '7':
                                case '8':
                                case '9':
                                    type = c - '0';
                                    if (verbosity == 5)
                                        printf("S%1i: ", type);
                                    // Next State Set-up
                                    state = SREC_BYTECOUNT;
                                    nibble = HI;
                                    bytes = 1;
                                    value = 0;
                                    checksum = 0;
                                    break;
                                default:
                                    processing = FALSE;
                                    error = TRUE;
                                }
                            break;

                        case SREC_BYTECOUNT:  // Byte Count
                            if (verbosity == 5)
                                printf("%c",c);
                            nibble_value = hexvalue(c);
                            if (nibble_value >= 0)
                                {
                                    switch (nibble)
                                        {
                                        case HI:
                                            nibble = LO;
                                            byte_value = nibble_value;
                                            break;
                                        case LO:
                                            nibble = HI;
                                            byte_value *= 16;
                                            byte_value += nibble_value;
                                            checksum += byte_value;
                                            bytes--;
                                            if (bytes)
                                                {
                                                    value *= 256;
                                                    value += byte_value;
                                                }
                                            else
                                                {
                                                    byte_count = byte_value;
                                                    if (verbosity == 2)
                                                        printf(" (%3x)\n", byte_count);
                                                    // Next State Set-up
                                                    switch (type)
                                                        {
                                                        case 0: addr_bytes = 0; break;
                                                        case 1: addr_bytes = 2; break;
                                                        case 2: addr_bytes = 3; break;
                                                        case 3: addr_bytes = 4; break;
                                                        case 5: addr_bytes = 0; break;
                                                        case 7: addr_bytes = 4; break;
                                                        case 8: addr_bytes = 3; break;
                                                        case 9: addr_bytes = 2; break;
                                                        default:
                                                            processing = FALSE;
                                                            error = TRUE;
                                                        }
                                                    data_bytes = byte_count - addr_bytes - 1;
                                                    value = 0;
                                                    if (addr_bytes)
                                                        {
                                                            state = SREC_ADDRESS;
                                                            bytes = addr_bytes;
                                                        }
                                                    else
                                                        {
                                                            state = SREC_DATA;
                                                            bytes = data_bytes;
                                                        }
                                                }
                                            break;
                                        default:
                                            processing = FALSE;
                                            error = TRUE;
                                        }
                                }
                            else
                                {
                                    processing = FALSE;
                                    error = TRUE;
                                }
                            break;

                        case SREC_ADDRESS:    // Address
                            if (verbosity == 5)
                                printf("%c",c);
                            nibble_value = hexvalue(c);
                            if (nibble_value >= 0)
                                {
                                    switch (nibble)
                                        {
                                        case HI:
                                            nibble = LO;
                                            byte_value = nibble_value;
                                            break;
                                        case LO:
                                            nibble = HI;
                                            byte_value *= 16;
                                            byte_value += nibble_value;
                                            checksum += byte_value;
                                            value *= 256;
                                            value += byte_value;
                                            nibble = HI;
                                            bytes--;
                                            if (0 == bytes)
                                                {
                                                    address = value;
                                                    if (verbosity == 2)
                                                        printf(" (%10x) ", address);
#ifdef SREC2BIN
                                                    *addr_list++ = address;
#endif
                                                    // Next State Set-up
                                                    state = SREC_DATA;
                                                    value = 0;
                                                    if (data_bytes)
                                                        {
                                                            state = SREC_DATA;
                                                            bytes = data_bytes;
                                                        }
                                                    else
                                                        {
                                                            state = SREC_CHKSUM;
                                                            checksum = (~checksum)&0xFF;
                                                            bytes = 1;
                                                        }
#ifdef SREC2BIN
                                                    *byte_cnt_list = bytes;
                                                    byte_cnt_list++;
#endif
                                                }
                                            break;
                                        default:
                                            processing = FALSE;
                                            error = TRUE;
                                        }
                                }
                            else
                                {
                                    processing = FALSE;
                                    error = TRUE;
                                }
                            break;

                        case SREC_DATA:       // Data
                            if (verbosity == 6)
                                printf("%c",c);
                            nibble_value = hexvalue(c);
                            if (nibble_value >= 0)
                                {
                                    switch (nibble)
                                        {
                                        case HI:
                                            nibble = LO;
                                            byte_value = nibble_value;
                                            break;
                                        case LO:
                                            nibble = HI;
                                            byte_value *= 16;
                                            byte_value += nibble_value;
                                            checksum += byte_value;
                                            if (verbosity == 5)
                                                printf(" (%3i) ", byte_value);
                                            // --------------------------
                                            // Write data to BIN file
                                            switch (type)
                                                {
                                                case 1:
                                                case 2:
                                                case 3:
                                                    if (address < rom_size)
                                                        {
                                                            fseek(fp_bin, address, SEEK_SET);
                                                            byte = byte_value;
                                                            fwrite(&byte, 1, 1, fp_bin);
                                                        }
                                                    address++;
                                                    if (min_rom_size < address)
                                                        min_rom_size = address;
                                                    break;
                                                }
                                            // --------------------------
                                            bytes--;
                                            if (0 == bytes)
                                                {
                                                    state = SREC_CHKSUM;
                                                    checksum = (~checksum)&0xFF;
                                                    bytes = 1;
                                                }
                                            break;
                                        default:
                                            processing = FALSE;
                                            error = TRUE;
                                        }
                                }
                            else
                                {
                                    processing = FALSE;
                                    error = TRUE;
                                }
                            break;

                        case SREC_CHKSUM:     // Checksum
                            if (verbosity == 5)
                                printf("%c",c);
                            nibble_value = hexvalue(c);
                            if (nibble_value >= 0)
                                {
                                    switch (nibble)
                                        {
                                        case HI:
                                            nibble = LO;
                                            byte_value = nibble_value;
                                            break;
                                        case LO:
                                            nibble = HI;
                                            byte_value *= 16;
                                            byte_value += nibble_value;
                                            value *= 256;
                                            value += byte_value;
                                            if (verbosity == 5)
                                                printf(" [%3i] ", byte_value);
                                            if (verbosity == 5)
                                                if (checksum != byte_value)
                                                    printf(" !Checksum Failed! ");
                                            bytes--;
                                            if (0 == bytes)
                                                {
                                                    state = SREC_ENDOFLINE;
                                                    bytes = 1;
                                                }
                                            break;
                                        default:
                                            processing = FALSE;
                                            error = TRUE;
                                        }
                                }
                            else
                                {
                                    processing = FALSE;
                                    error = TRUE;
                                }
                            break;

                        case SREC_ENDOFLINE:  // Checksum
                            switch (c)
                                {
                                case '\n':
                                    if (verbosity == 5)
                                        printf("[%2X]\n", checksum);
                                    state = SREC_NEWLINE;
                                    records++;
                                    break;
                                }
                            break;

                        default: // Unknown State
                            if (verbosity == 5)
                                printf("FSM has entered undefined state.\n");
                            exit(EXIT_FAILURE);
                            break;
			}
		}
            if (verbosity)
                printf("(%i records)\n", records);
            fclose(fp_srec);
	}
    fclose(fp_bin);

    // Housecleaning report
    if (verbosity)
	{
            printf("\n");
            printf("======================================\n");
            printf("Exit State: %i\n", state);
            printf("Error State: %i\n", error);
            printf("Minimum ROM size: %i\n", min_rom_size);
            printf("======================================\n");
	}
#ifndef SREC2BIN
    terminate(verbosity);
#endif
    return(0);
}
/*------------------------------------------------------------------
 $Log: srec2bin.c,v $
 Revision 1.2  2014/06/03 19:08:14  mcharon
 support odd size pwer seq s record

 Revision 1.1  2014/05/29 00:40:25  mcharon
 add srec2bin.c


 $Endlog$
*/
