/* $Id: code_sign_dsp.c,v 1.2 2016/10/07 17:52:12 srane Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/sp27dsp/image/code_sign_dsp.c,v $
 *------------------------------------------------------------------
 * code_sign_dsp.c
 *
 * Sep 2016, Smita Rane
 *
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

/***************************************************************************
 *
 *  Copyright (c) 2012, 2016 by Cisco Systems, Inc.
 *  All rights reserved.
 *
 *  This module is an original, unpublished work and is the intellectual 
 *  property of Cisco Inc., and may not be divulged, copied or used 
 *  in any form whatsoever without the express written permission of 
 *  Cisco Systems Inc.
 *
 *  File:
 *  -----
 *  code_sign_dsp.c
 *  A wrapper application to sign the DSP firmware (.rbf files).
 *  This warpper application will invoke code_sign_add_signature_exec
 *  tool (which is copied from IOS build) and in turn will invoke 
 *  abraxas-client tool (also copied from IOS build).
 *
 *  The signed DSP firmware will have the following format:
 *  
 *  ----------------------------
 *  | MAGIC NUMBER   | 4-bytes |
 *  --------------------------
 *  | orig. firmware | 4-bytes |
 *  | length         |         |
 *  ----------------------------
 *  | orig. firmware | vary    |
 *  ----------------------------
 *  | Signature      | vary    |
 *  | Envelop        |         |
 *  ----------------------------
 *
 *  magic number and rbf length are in network order.
 *  
 **************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* magic number to indicate this is a signed dsp firmware.
 * This number must be different than existing 5510 or 26xx or 27xx
 * firmware so that older IOS that doesn't check for signature
 * will flag it as invalid firmware.
 */
#define DSP_CS_MAGIC_NUMBER  0xDAADCAFE

#define CMD_BUFF_LEN         512

/* product name (PVDM) is same for signing all DSP firmwares.
 * product market name can be anything. So will use product market name 
 * for PVDM2 or PVDM3 or PVDM4 or whatever.
 */
static char *productMarketName = NULL;

/* ticket file to be used for authentication. */
static char * ticketFile = NULL;

/* hardcoded arguments used for DSP when invoking the 
 * code_sign_add_signature_exec util.
 */
static char *defaultArgs = "-p PVDM -S PKCSV15 -n -I PVDM4-256 -A Ticket";
static char *nomagic_defaultArgs = "-p PVDM -S PKCSV15 -I PVDM4-256 -A Ticket";
static char *keyVersion = NULL;
static char *execCmd = "/router/bin/code_sign tlv-sign";
static char *keyType = NULL;
static char *imageIn = NULL;
static char *imageOut = NULL;
static int nomagic = 0;
static int envsize = 0;

static void usage (void)
{
    printf("\nUsage: code_sign_dsp [-arguments]\n");
    printf("\n       -k (key type DEV or REL or REV)");
    printf("\n       -f <image file>");
    printf("\n       -o <output signed image file>");
    printf("\n       -v <key-version to be used during image signing>"); 
	printf("\n       -P < product market name, i.e., PVDM2> ");
	printf("\n       -T <Ticket file for authentication>");
    printf("\n       -h <this help text>\n");
}


// ../cs_tools/code_sign_add_signature_exec -s DIGI_SIGN_A  -p PVDM  -P PVDM2  -k DEV -v A -S PKCSV15 -e ../cs_tools/abraxas-client -f test1 -n

static int parseArg (int argc, char **argv)
{
    int arg = 1;

    while (arg < argc) {
        if (strcmp(argv[arg], "-k") == 0) {
            /* -k : key type, DEV or REL */
            ++arg;
            if (arg >= argc) {
                usage();
                return (1);
            }
            keyType = argv[arg];
        } else if (strcmp(argv[arg], "-f") == 0) {
            /* input dsp firmware */
            ++arg;
            if (arg >= argc) {
                usage();
                return (1);
            }
            imageIn = argv[arg];
        } else if (strcmp(argv[arg], "-o") == 0) {
            /* output signed dsp firmware */
            ++arg;
            if (arg >= argc) {
                usage();
                return (1);
            }
            imageOut = argv[arg];
        } else if (strcmp(argv[arg], "-v") == 0) {
            /* key version */
            ++arg;
            if (arg >= argc) {
                usage();
                return (1);
            }
            keyVersion = argv[arg];
        } else if (strcmp(argv[arg], "-P") == 0) {
            /* product marketing name */
            ++arg;
            if (arg >= argc) {
                usage();
                return (1);
            }
            productMarketName = argv[arg];
        } else if (strcmp(argv[arg], "-T") == 0) {
            /* authentication ticket file */
            ++arg;
            if (arg >= argc) {
                usage();
                return (1);
            }
            ticketFile = argv[arg];
        } else if (strcmp(argv[arg], "-nomagic") == 0) {
            nomagic = 1;
        } else if (strcmp(argv[arg], "-envsize") == 0) {
            envsize = 1;
        } else if (strcmp(argv[arg], "-h") == 0) {
            usage();
        } else {
            printf("\nUnknown option %s\n", argv[arg]);
            usage();
            return (1);
        }
        arg++;
    }

    return (0);
}
int main (int argc, char **argv) 
{
    char cmd[CMD_BUFF_LEN] = {0};
    unsigned int fw_len;
    FILE *in_fw, *out_fw;
    unsigned int i;
    int result;

    /* parse input arguments */
    if (parseArg(argc, argv)) {
        /* something wrong, exit */
        exit(1);
    }

    /* check if all required arguments are present */
    if (!productMarketName || !keyVersion || !keyType || 
        !imageIn || !imageOut || !ticketFile) {
        usage();
        exit(1);
    }

    /* open input dsp firmware */
    in_fw = fopen(imageIn, "rb");
    if (in_fw == NULL) {
        printf("\nUnable to open file %s for read\n", imageIn);
        exit(1);
    }

    /* open output file for write */
    out_fw = fopen(imageOut, "wb");
    if (out_fw == NULL) {
        printf("\nUnable to open file %s for write\n", imageOut);
        fclose(in_fw);
        exit(1);
    }

    /* figure out the input firmware length */
    fseek(in_fw, 0 , SEEK_END);
    fw_len = ftell(in_fw);
    printf("\nfilesize of %s: %d bytes\n", imageIn, fw_len);

    if (nomagic == 0) {
        /* write the magic number to output file */
        fputc((DSP_CS_MAGIC_NUMBER >> 24) & 0xFF, out_fw);
        fputc((DSP_CS_MAGIC_NUMBER >> 16) & 0xFF, out_fw);
        fputc((DSP_CS_MAGIC_NUMBER >> 8) & 0xFF, out_fw);
        fputc(DSP_CS_MAGIC_NUMBER & 0xFF, out_fw);
        /* length to output file */
        fputc((fw_len >> 24) & 0xFF, out_fw);
        fputc((fw_len >> 16) & 0xFF, out_fw);
        fputc((fw_len >> 8) & 0xFF, out_fw);
        fputc(fw_len & 0xFF, out_fw);
    }

    /* copy the input file into output file */
    fseek(in_fw, 0 , SEEK_SET);
    for (i = 0; i < fw_len; i++) {
        fputc(fgetc(in_fw), out_fw);
    }

    fclose(in_fw);
    fclose(out_fw); /* close the files, since the util will open it
                     * again for write.
                     */

    /* now, call the util to signed the output file. Signature will
     * be appended to the end of the output file.
     */
	//Enter Hex value of key version for swims code_sign. Example: Pass 0x41 for "A" key version.
	sprintf(cmd, "%s %s -P %s -k %s -v 0x%02X -f %s -T %s", 
            execCmd, nomagic ? nomagic_defaultArgs: defaultArgs, productMarketName, keyType,
            keyVersion[0], imageOut, ticketFile);
    
    if (envsize) {
        sprintf(cmd, "%s -i -o %s", cmd, imageOut);
    }
    printf("\nExecuting %s\n", cmd);
    result = system(cmd);

    /* when the code sign command failed, it might returns error code 256,
     * which doesn't treated as error in our make file.
     * So check if non-zero then abort with error code of 1 so that
     * makefile recognizes it is an error.
     */
    if (result != 0) {
        printf("ERROR in code signing with error code %d\n", result);
        exit(1);
    }

    return result;
}
/*
 * $Log: code_sign_dsp.c,v $
 * Revision 1.2  2016/10/07 17:52:12  srane
 * CSCvb61570 - Move to SWIMS server for code signing
 *
 * Revision 1.1  2016/09/28 06:02:53  srane
 * Code sign dsp exec failed (/auto/ directory). Port from IOS.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
*/

