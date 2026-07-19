/* $Id: conv26x.c,v 1.4 2016/10/07 17:53:21 srane Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/sp27dsp/image/conv26x.c,v $
 *------------------------------------------------------------------
 * conv26x.c
 *              Generate image record sections file for download
 *              Port from DSP.
 *
 * Mar 2012, Smita Rane
 *
 * Copyright (c) 2012-2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

/******************************************************************************
 *
 *  Copyright (c) 2007-2014 by Cisco Systems, Inc.
 *  All rights reserved.
 *
 *  This module is an original, unpublished work and is the intellectual property of Cisco Inc., and
 *  may not be divulged, copied or used in any form whatsoever without the express written
 *  permission of Cisco Systems Inc.
 *
 *  File:
 *  -----
 *  conv26x.c
 *
 *  Modification History
 *  --------------------
 *
 *    jmuir     First version.  Got it going modelled on conv54x.c
 *    wwatson   1.01 added all the version info as per conv54x.c
 *    wwatson   1.02 minor modification to version info
 *    pbecerra  1.03 enhancements:
 *                   option -V to print section info as elf file is processed.
 *                   option -x to do selective exclusion of sections.
 *                   allow two dsp_image files for combining into single load.
 *                   NOTE: when combining dsp_images, the -d options MUST come
 *                   before the "-a arm_image" option.  This is required
 *                   because a section in the ARM code is filled with info
 *                   extracted from the dsp_image files.  If this info is
 *                   not available when the arm_image is being processed,
 *                   the ARM section will be filled with default values
 *                   that can make the DSS unbootable.
 *    pbecerra  1.04 added code to work with sp27xx
 *         
 ******************************************************************************/
#define CVERSION     1.04                  // Update this every time you 
                                          // change this program.
#define VDATE       __DATE__ " " __TIME__ // "00-Oct-05"

#ifndef OLD_BFD
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "bfd.h"

typedef uint32_t uInt32;

#include "section_table.h"
#include "env.h"
#include "dsp_key_buffer.h"
#include "eeprom_map.h"

#ifdef __linux__
extern char *cuserid(char *string);
#endif

extern char *optarg;

/*
 * The following spi_* stuff is for creating the SP2704 boot loader images
 *
 * There are 2 images: The golden image which resides at the beginning of
 * the EEPROM and contains the jump record.  This image should never be touched
 * by the boot loader.  This image is loaded by the DSP when GPIO[0:5] is 0 at startup
 *
 * The second image is the upgrade image.  This includes only a
 * control record and data records and resides at offset 0x20000 of the EEPROM.
 * This image is loaded by the DSP when GPIO[0:5] is 1.  The boot loader
 * can upgrade this image.  If the upgrade image ever gets corrupted,
 * the host can set GPIO[0:5] to 0 to boot the golden image.
 */
enum {
    spi_upgrade,
    spi_golden,
    spi_complete
} spi_mode;
int spi_sdb = 0;

#define SPI_DATA_MAGIC          0xAAEC
#define SPI_TERMINATION_MAGIC   0xCC55
#define SPI_CONTROL_MAGIC       0xBB1A

#define SPI_PLL1SELECT          1
#define SPI_PLL_F               0x1c
#define SPI_SSPDIV              0x2


static FILE *out_fd;
static FILE *rbf_fd;
static FILE *rbf_c_fd;
static FILE *spi_fd;

static unsigned int rbf_c_len = 0;
static unsigned int spi_addr = 0;

static int verbose = 0;
int do_ee_prom;

#define MAGIC_NUMBER           0x20080001
#define MAGIC_NUMBER_SP2704    0x2704FACE
#define MAGIC_NUMBER_ERROR     0xDEADDEAD

typedef struct  {  
    uint32_t            magic_number;
    uint32_t            header_size;
    uint32_t            DSPmajorversion;
    uint32_t            DSPminorversion;
    uint32_t            DSPbuild;
    char                releasestring[32];
    char                buildstring[64];
    char                compatibilitystring1[32];
    char                compatibilitystring2[32];
    char                compatibilitystring3[32];
    char                compatibilitystring4[32];
    char                logical_user[32];
    char                actual_user[32];
} header_stuff_t;

header_stuff_t          version_header; 

char                    current_date_string[50];
char                    current_year_string[5];

int                     embVer = -1,
                        embRev = -1,
                        embBuild = -1;

char                    actual_user[32] = "";
char                    logical_user[32] = "";
char                    release_string[32] = "255.255.255";
char                    build_string[64] = "255.255.255";
char                    s1[32] = "";
char                    s2[32] = "";
char                    s3[32] = "";
char                    s4[32] = "";
char                    gmt_str[80];
int                     save_sections_size;

typedef enum {
    SP26XX,
    SP27XX,
    DM814X,
    TILEGX,
} DEVICE;
static DEVICE dev = SP26XX;
static int asr1k = 0;

static char copyright_header[] =
    "/*\n"
" *------------------------------------------------------------------\n"
" * %s - Generated using conv26x.exe \n"
" *                        (%2.2f %s)\n"
" * By %s (%s) on %s GMT\n" 
" * \n"
" * Firmware version %d.%d.%d\n"
" * \n"
" * External Release Name %s\n"
" * Info string 1 %s\n"
" * Info string 2 %s\n"
" * Info string 3 %s\n"
" * Info string 4 %s\n"
" *\n"
" * Copyright (c) 2007-%s by Cisco Systems, Inc.\n"
" * All rights reserved.\n"
" *------------------------------------------------------------------\n"
" */\n"
;

static char file_header[] =
"/*\n"
" *------------------------------------------------------------------\n"
" * %s - Generated using conv26x.exe \n"
" *                        (%2.2f %s)\n"
" * By %s (%s) on %s GMT\n" 
" * \n"
" * Firmware version %d.%d.%d\n"
" * \n"
" * External Release Name %s\n"
" * Info string 1 %s\n"
" * Info string 2 %s\n"
" * Info string 3 %s\n"
" * Info string 4 %s\n"
" *\n"
" * Copyright (c) 2007-%s by Cisco Systems, Inc.\n"
" * All rights reserved.\n"
" *------------------------------------------------------------------\n"
" */\n"
"#if !defined(_DSP_%s_FMW_H_)\n"
"#define _DSP_%s_FMW_H_\n"
"\n"
"#if !defined(__linux__)\n"
"#include <master.h>\n"
"#include COMP_INC(posix, inttypes.h)\n"
"#else\n"
"#ifdef TARGET_CISCO\n"
"#include COMP_INC(posix, inttypes.h)\n"
"#endif\n"
"#include <stdint.h>\n"
"#endif\n"
"\n"
"typedef struct  {\n"
"    uint32_t            magic_number;\n"
"    uint32_t            header_size;\n"
"    uint32_t            DSPmajorversion;\n"
"    uint32_t            DSPminorversion;\n"
"    uint32_t            DSPbuild;\n"
"    char                releasestring[32];\n"
"    char                buildstring[64];\n"
"    char                compatibilitystring1[32];\n"
"    char                compatibilitystring2[32];\n"
"    char                compatibilitystring3[32];\n"
"    char                compatibilitystring4[32];\n"
"    char                logical_user[32];\n"
"    char                actual_user[32];\n"
"} header_stuff_t;\n"
"\n"
"typedef struct dsp_download_records_ {\n"
"    uint32_t length;\n"
"    uint32_t address;\n"
"    const uint8_t *data;\n"
"} dsp_download_records_t;\n"
"\n"
"#endif  /* !defined(_DSP_%s_FMW_H_) */\n"
;

static char version_header_begin[] = 
"\nstatic const uint8_t dsp_version_header[0x%x] = {";

static char version_header_end[] =
"};\n\n"; 
 
static char sec_prefix[160] = "";

#ifndef LE_16
#define LE_16(x) ((((x) & 0xFF) << 8) | (((x) >> 8) & 0xFF))
#endif
#ifndef LE_32
#define LE_32(x) \
    ((uint32_t)((((uint32_t)(x) & 0x000000ffU) << 24)   |  \
                (((uint32_t)(x) & 0x0000ff00U) <<  8)   |  \
                (((uint32_t)(x) & 0x00ff0000U) >>  8)   |  \
                (((uint32_t)(x) & 0xff000000U) >> 24)))

#endif
/*
 * Definitions for core_id_flags
 */
#define CORE_ID_FLAG_DSS_0 (1 << 0)
#define CORE_ID_FLAG_DSS_1 (1 << 1)
#define CORE_ID_FLAG_DSS_2 (1 << 2)
#define CORE_ID_FLAG_DSS_3 (1 << 3)
#define CORE_ID_FLAG_ALL_DSS (CORE_ID_FLAG_DSS_0 | \
                              CORE_ID_FLAG_DSS_1 | \
                              CORE_ID_FLAG_DSS_2 | \
                              CORE_ID_FLAG_DSS_3)
#define CORE_ID_FLAG_DSS_IMG_1 ((uint32_t)(1 << 30))
#define CORE_ID_FLAG_PPB   ((uint32_t)(1 << 31))
                                   
typedef struct _image_info {
    struct _image_info *next;
    char *image_name;
    char image_prefix[80];
    uint32_t core_id_flags;
    uint32_t option_flags;
    char included_sections[2048];
    char excluded_sections[2048];
} image_info;

//#define IS_DSS_SYSTEM_MEMORY(x) ((x) <= 0x002fffff)
static int IS_DSS_SYSTEM_MEMORY(uint32_t x)
{
    uint32_t max;
    if (dev == SP26XX) {
        max = 0x002fffff;
    } else {
        max = 0x005fffff;
    }
    return ((x) <= max);
}
//#define IS_DSS_LOCAL_MEMORY(x) (((x) >= 0x40000000) && ((x) <= 0x4000ffff))
static int IS_DSS_LOCAL_MEMORY(uint32_t x)
{
    uint32_t max;
    if (dev == SP26XX) {
        max = 0x4000ffff;
    } else {
        max = 0x4003ffff;
    }
    return (((x) >= 0x40000000) && ((x) <= max));
}
//#define IS_DSS_DDR2_MEMORY(x) (((x) >= 0x10000000) && ((x) <= 0x2fffffff))
static int IS_DSS_DDR2_MEMORY(uint32_t x)
{
    uint32_t min, max;
    if (dev == SP26XX) {
        min = 0x10000000;
        max = 0x2fffffff;
    } else {
        min = 0x20000000;
        max = 0x3fffffff;
    }
    return (((x) >= min) && ((x) <= max));
}

/*
 * Definitions for option_flags
 */
#define OPTION_FLAG_RETAIN (1 << 0)

typedef struct dsp_download_records_ {
    struct dsp_download_records_ *next;
    uint32_t length;
    uint32_t address;
    uint32_t core_id_flags;
    uint32_t option_flags;
    uint32_t section_attributes;
    uint32_t sum;
    char *name;
} dsp_download_records_t;

static dsp_download_records_t *dlrec_head = NULL;
static dsp_download_records_t *dlrec_cur = NULL;

typedef struct {
    uint32_t ovltab;
    uint32_t ovlcnt;
} dss_overlay_t;
static dss_overlay_t dss_overlay[2];
static unsigned dss_ovltab_cnt;
#define DSS_OVERLAY_LENGTH (2*2*sizeof(uint32_t))
#define DSS_OVLTAB_CNT_MAX (sizeof(dss_overlay) / sizeof(dss_overlay_t))

static void
usage (void) {
    fprintf(stderr, "Usage: conv26x <options>\n");
    fprintf(stderr, "Options are:\n");
    fprintf(stderr, "  -a <arm_image>\n");
    fprintf(stderr, "  -d <dsp_image>\n");
    fprintf(stderr, "  -0 <dss0_image>\n");
    fprintf(stderr, "  -1 <dss1_image>\n");
    fprintf(stderr, "  -2 <dss2_image>\n");
    fprintf(stderr, "  -o <output_file>\n");
    fprintf(stderr, "  -r <rbf_file>\n");
    fprintf(stderr, "  -c <rbf_c_file>\n");
    fprintf(stderr, "  -e create output files for eprom burner\n");
    fprintf(stderr, "  -gspi <boot loader golden spi_file>\n");
    fprintf(stderr, "  -uspi <boot loader upgrade spi_file>\n");
    fprintf(stderr, "  -spi <boot loader complete spi_file>\n");
    fprintf(stderr, "  -img <signed upgrader boot loader>\n");
    fprintf(stderr, "  -s <include section name>\n");
    fprintf(stderr, "  -x <exclude section name>\n");
    fprintf(stderr, "  -veA.B.C embed version info\n");
    fprintf(stderr, "  -u<user> username of builder\n");
    fprintf(stderr, "  -cs1<string> Compatibility string 1\n");
    fprintf(stderr, "  -cs2<string> Compatibility string 2\n");
    fprintf(stderr, "  -cs3<string> Compatibility string 3\n");
    fprintf(stderr, "  -cs4<string> Compatibility string 4\n");
    fprintf(stderr, "  -sp27xx Build for sp27xx device.  Default is sp26xx\n");
    fprintf(stderr, "  -dm814x Build for dm814x device.  Default is sp26xx\n");
    fprintf(stderr, "  -tilegx Build for tilegx device.  Default is sp26xx\n");
    fprintf(stderr, "  -asr1k  Build for use by asr1k\n");
    fprintf(stderr, "  -vr<string> Version release string\n");
    fprintf(stderr, "  -V Verbose output\n");
    exit(1);
}

static void
nonfatal (const char *msg)
{
    fprintf(stderr, "Bfd error in %s\n", msg);
}


static void
fix_name (char *name) {
    int i;

    i = 0;
    while (name[i]) {
        if (name[i] == '.') {
            name[i] = '_';
        }
        i++;
    }
}

static void
fill_in_version_header(void) {
    time_t now;
    char *nl;

    now = time(NULL);
    sprintf(gmt_str, "%s", asctime(gmtime(&now)));
    nl = strchr(gmt_str, '\n');
    if (nl) {
        *nl = '\0';
    }
    sprintf(build_string, "Built on %s", gmt_str);

    if (dev == SP26XX) {
        version_header.magic_number = htonl(MAGIC_NUMBER);
    } else if (dev == SP27XX) {
        version_header.magic_number = htonl(MAGIC_NUMBER_SP2704);
    } else {
        version_header.magic_number = htonl(MAGIC_NUMBER_ERROR);
    }  
    version_header.header_size = htonl(sizeof(version_header));
    version_header.DSPmajorversion = htonl(embVer);
    version_header.DSPminorversion = htonl(embRev);
    version_header.DSPbuild = htonl(embBuild);
    strncpy(version_header.releasestring, release_string, sizeof(version_header.releasestring));
    strncpy(version_header.buildstring, build_string, sizeof(version_header.buildstring));
    strncpy(version_header.compatibilitystring1, s1, sizeof(version_header.compatibilitystring1));
    strncpy(version_header.compatibilitystring2, s2, sizeof(version_header.compatibilitystring2));
    strncpy(version_header.compatibilitystring3, s3, sizeof(version_header.compatibilitystring3));
    strncpy(version_header.compatibilitystring4, s4, sizeof(version_header.compatibilitystring4));
    strncpy(version_header.logical_user, logical_user, sizeof(version_header.logical_user));
    strncpy(version_header.actual_user, actual_user, sizeof(version_header.actual_user));
}

static int
section_included (image_info *imginfo, const char *secname) {
    char search_str[80];

    if (strlen(imginfo->included_sections) == 0) {
        return 1;
    }
    sprintf(search_str, "{%s}", secname);
    if (strstr(imginfo->included_sections, search_str) == NULL) {
        return 0;
    }
    return 1;
}

static int
section_excluded (image_info *imginfo, const char *secname) {
    char search_str[80];

    if (strlen(imginfo->excluded_sections) == 0) {
        return 0;
    }
    sprintf(search_str, "{%s}", secname);
    if (strstr(imginfo->excluded_sections, search_str) != NULL) {
        return 1;
    }
    return 0;
}

/*
 * Compute a Fletcher's checksum (RFC 1146)
 */
static uint32_t fletcher( uint16_t *data, size_t len )
{
    uint32_t sum1 = 0xffff, sum2 = 0xffff;
    uint32_t ret;
#ifdef _BIG_ENDIAN
    uint16_t *sumptr;
#endif
 
    while (len) {
        unsigned tlen = len > 360 ? 360 : len;
        len -= tlen;
        do {
            sum1 += *data++;
            sum2 += sum1;
        } while (--tlen);
        sum1 = (sum1 & 0xffff) + (sum1 >> 16);
        sum2 = (sum2 & 0xffff) + (sum2 >> 16);
    }
    /* Second reduction step to reduce sums to 16 bits */
    sum1 = (sum1 & 0xffff) + (sum1 >> 16);
    sum2 = (sum2 & 0xffff) + (sum2 >> 16);
    ret = sum2 << 16 | sum1;
#ifdef _BIG_ENDIAN
    sumptr = (uint16_t *)(&ret);
    sumptr[0] = LE_16(sumptr[0]);
    sumptr[1] = LE_16(sumptr[1]);
#endif
    return ret;
}

static void
get_ovltab_info (dsp_download_records_t *dlrec, unsigned char *data) {
    unsigned char *ovlcnt_p;
    if (dss_ovltab_cnt < DSS_OVLTAB_CNT_MAX) {
#ifdef _BIG_ENDIAN
        dss_overlay[dss_ovltab_cnt].ovltab = LE_32(dlrec->address);
#else
        dss_overlay[dss_ovltab_cnt].ovltab = dlrec->address;
#endif
        ovlcnt_p = data + dlrec->length - sizeof(uint32_t);
        memcpy(&dss_overlay[dss_ovltab_cnt].ovlcnt, ovlcnt_p, sizeof(uint32_t));
        ++dss_ovltab_cnt;
    }
}
static void
put_ovltab_info (dsp_download_records_t *dlrec, unsigned char *data) {
    if (dlrec->length >= DSS_OVERLAY_LENGTH) {
        memcpy(&data[0*sizeof(uint32_t)], &dss_overlay[0].ovltab, sizeof(uint32_t));
        memcpy(&data[1*sizeof(uint32_t)], &dss_overlay[0].ovlcnt, sizeof(uint32_t));
        memcpy(&data[2*sizeof(uint32_t)], &dss_overlay[1].ovltab, sizeof(uint32_t));
        memcpy(&data[3*sizeof(uint32_t)], &dss_overlay[1].ovlcnt, sizeof(uint32_t));
    } else if (verbose) {
        fprintf(stdout,
                "length of .dss_overlay < expected (%d < %d)\n",
                (int)(dlrec->length), (int)DSS_OVERLAY_LENGTH);
    }
}

/*
 * spi_put
 *
 * Add data to the spi file and return the byte checksum of that data
 */
uint16_t spi_put (unsigned int length, void *data) {
    unsigned char *cp = data;
    unsigned int i;
    uint16_t csum = 0;

    for (i = 0; i < length; ++i) {
        fwrite(cp + i, 1, 1, spi_fd);
        csum += *(cp + i);
    }
    return csum;
}

static void
write_section (dsp_download_records_t *dlrec, unsigned char *data) {
    if (out_fd) {
        unsigned int count;

        fprintf(out_fd, "/*\n * %s @ 0x%.8x\n */\n", dlrec->name,
                dlrec->address);
        fprintf(out_fd, "static const uint8_t %s[0x%x] = {", dlrec->name,
                dlrec->length);
        count = 0;
        while (count < dlrec->length) {
            if (count % 16 == 0) {
                fprintf(out_fd, "\n  ");
            }
            fprintf(out_fd, "0x%.2x%s", data[count],
                    dlrec->length - count == 1 ? "" : ",");
            ++count;
        }
        fprintf(out_fd, "};\n");
    }

    if (rbf_fd) {
        unsigned int tmp;

        tmp = htonl(dlrec->length);
        fwrite(&tmp, sizeof(tmp), 1, rbf_fd);
        tmp = htonl(dlrec->address);
        fwrite(&tmp, sizeof(tmp), 1, rbf_fd);
        fwrite(data, 1, dlrec->length, rbf_fd);
    }

    if (rbf_c_fd) {
        unsigned int count;
        unsigned int tmp;
        unsigned char *cp;

        tmp = htonl(dlrec->length);
        cp = (unsigned char *)&(tmp);
        fprintf(rbf_c_fd, "  0x%02x,  0x%02x,  0x%02x,  0x%02x,   /* Section length */\n",
                cp[0], cp[1], cp[2], cp[3]);
        tmp = htonl(dlrec->address);
        fprintf(rbf_c_fd, "  0x%02x,  0x%02x,  0x%02x,  0x%02x,   /* Section address */\n",
                cp[0], cp[1], cp[2], cp[3]);
        count = 0;
        while (count < dlrec->length) {
            fprintf(rbf_c_fd, "  0x%02x,", data[count]);
            ++count;
            if ((count < dlrec->length) && (count % 10 == 0)) {
                fprintf(rbf_c_fd, "\n");
            }
        }
        fprintf(rbf_c_fd, "\n");
        rbf_c_len += dlrec->length + 8;
    }

    if (spi_fd) {
        uint16_t tmp_16;
        uint32_t tmp_32;
        uint16_t csum = 0;

        /* 
         * output a SPI data record containing the section
         */
        if (verbose) {
            printf("%s at offset 0x%x\n", dlrec->name, spi_addr);
        }

        /* output the data record magic number */
        tmp_16 = SPI_DATA_MAGIC;
        csum += spi_put(sizeof(tmp_16), &tmp_16);
        spi_addr += sizeof(tmp_16);
        /* output the data record length */
        if (dlrec->length > 0xFFFF) {
            fprintf(stderr, "Section %s exceeds maximum length\n", dlrec->name);
            exit(2);
        }
        tmp_16 = dlrec->length + 15;
        csum += spi_put(sizeof(tmp_16), &tmp_16);
        spi_addr += sizeof(tmp_16);
        /* output the next record offset */
        tmp_32 = spi_addr + dlrec->length + 11;
        csum += spi_put(3, &tmp_32);
        spi_addr += 3;
        /* output the load address */
        tmp_32 = dlrec->address;
        csum += spi_put(sizeof(tmp_32), &tmp_32);
        spi_addr += 4;
        /* output the data */
        csum += spi_put(dlrec->length, data);
        spi_addr += dlrec->length;
        /* output the checksum */
        spi_put(sizeof(csum), &csum);
        spi_addr += 2;
        /* output one's complement of csum */
        csum = ~csum;
        spi_put(sizeof(csum), &csum);
        spi_addr += 2;
    }
}

static void
do_section (bfd *bfdfile, asection *section, void *dummy) {
    dsp_download_records_t *dlrec;
    unsigned int section_size;
    unsigned char *data;
    image_info *imginfo = dummy;
    int is_dss_overlay, is_ovltab;

    if (verbose > 1) {
        fprintf(stdout, "  %s: flags=%X, vma=%08X, lma=%08x, size=%d",
                section->name,
                section->flags,
                (uint32_t)section->vma,
                (uint32_t)section->lma,
                (uint32_t)section->size
            );
    }

    if (section_excluded(imginfo, section->name)) {
        if (verbose > 1) {
            fprintf(stdout," - excluded\n");
        }
        return;
    }

    if (!section_included(imginfo, section->name)) {
        if (verbose > 1) {
            fprintf(stdout," - !inc\n");
        }
        return;
    }

    /* SEC_DATA, SEC_CODE */
    if (((section->flags & SEC_HAS_CONTENTS) == 0) ||
        (((section->flags & SEC_DATA) == 0) &&
         ((section->flags & SEC_CODE) == 0))) {
        if (verbose > 1) {
            fprintf(stdout," - !(data || code) || empty\n");
        }
        return;
    }
    if (section->size == 0) {
        if (verbose > 1) {
            fprintf(stdout,"\n");
        }
        return;
    }

    dlrec = (dsp_download_records_t *)malloc(sizeof(dsp_download_records_t));
    if (dlrec == NULL) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }

    is_dss_overlay = (strcmp(".dss_overlay", section->name) == 0);
    is_ovltab = (strcmp(".ovltab", section->name) == 0);

    dlrec->sum = 0;
    dlrec->next = NULL;
    dlrec->section_attributes = 0;
    if (section->flags & SEC_CODE) {
        dlrec->section_attributes |= SECTION_CODE;
        if (verbose > 1) {
            fprintf(stdout, " CODE");
        }
    }
    if (IS_DSS_LOCAL_MEMORY(section->lma)) {
        dlrec->section_attributes |= SECTION_SAVE;
        if (verbose > 1) {
            fprintf(stdout, " SAVE");
        }
        save_sections_size += section->size;
    }
    if (section->flags & SEC_READONLY) {
        dlrec->section_attributes |= SECTION_READONLY;
        if (verbose > 1) {
            fprintf(stdout, " READONLY");
        }
    }
    /*
     * If the section is ".odo_vectors_rw", make it look like a writeable section
     * This section gets modified when interrupt handlers are installed on SP2704
     */
    if (strcmp(".odo_vectors_rw", section->name) == 0) {
        dlrec->section_attributes &= ~SECTION_READONLY;
    }
    dlrec->length = section->size;
    if (imginfo->core_id_flags & CORE_ID_FLAG_ALL_DSS) {
        /*
         * Need to map DSS memory address to PPB addresses
         */
        if (IS_DSS_SYSTEM_MEMORY(section->lma)) {
            dlrec->address = section->lma + 0xC0000000;
        } else if (IS_DSS_LOCAL_MEMORY(section->vma)) {
            dlrec->address = section->lma + 0x40000000;
        } else if (IS_DSS_DDR2_MEMORY(section->vma)) {
            dlrec->address = section->lma + 0xC0000000;
        } else {
            dlrec->address = section->lma;
        }
    } else {
        dlrec->address = section->lma;
    }
    dlrec->name = malloc(strlen(section->name) + strlen(sec_prefix)
                         + strlen(imginfo->image_prefix) + 3);
    sprintf(dlrec->name, "%s_%s%s", sec_prefix, imginfo->image_prefix,
            section->name);
    fix_name(dlrec->name);

    if (dlrec_cur) {
        dlrec_cur->next = dlrec;
    } else {
        dlrec_head = dlrec;
    }
    dlrec_cur = dlrec;

    section_size = bfd_section_size(bfdfile, section);
    data = (unsigned char *)malloc(section_size);
    if (data == NULL) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    
    bfd_get_section_contents(bfdfile, section, data, 0, section_size); 
    if (is_ovltab) {
        get_ovltab_info(dlrec, data);
    } else if (is_dss_overlay) {
        put_ovltab_info(dlrec, data);
    }
    dlrec->sum = fletcher((uint16_t *)data, section_size / 2);

    write_section(dlrec, data);

    if (do_ee_prom) {
        FILE *eef = fopen(dlrec->name, "w");
        unsigned int i;

        for (i = 0; i < section_size; ++i) {
            fprintf(eef, "%.2X\n", data[i]);
        }
        fclose(eef);
    }
    free(data);
    if (verbose) {
        if (verbose > 1) {
            fprintf(stdout," - Record created\n");
        } else {
            fprintf(stdout, "  %s: flags=%X, vma=%08X, lma=%08X, size=%d\n",
                    section->name,
                    section->flags,
                    (uint32_t)section->vma,
                    (uint32_t)section->lma,
                    (uint32_t)section->size
                );
        }
    }
}

static void
add_image (image_info *imginfo) {
    bfd *bfdfile;
    char **matching;

    bfdfile = bfd_openr(imginfo->image_name, NULL);
    if (bfdfile == NULL) {
        nonfatal(imginfo->image_name);
        exit(1);
    }

    if (bfd_check_format_matches (bfdfile, bfd_object, &matching)) {
        bfd_map_over_sections(bfdfile, do_section, imginfo);
    }

    bfd_close(bfdfile);
}

#define RM_NAME "reset_message"

static void
add_rm (char *rm_file, uint32_t rm_addr) {
    dsp_download_records_t *dlrec;
    int rm_fd;
    struct stat fstats;
    unsigned char *data;

    rm_fd = open(rm_file, O_RDONLY);
    if (rm_fd == -1) {
        fprintf(stderr, "Can not open reset message file %s\n", rm_file);
        exit(1);
    }
    if (fstat(rm_fd, &fstats) == -1) {
        fprintf(stderr, "Can not stat %s\n", rm_file);
        exit(1);
    }
    dlrec = (dsp_download_records_t *)malloc(sizeof(dsp_download_records_t));
    if (dlrec == NULL) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }

    dlrec->sum = 0;
    dlrec->next = NULL;
    dlrec->section_attributes = SECTION_READONLY;
    dlrec->length = fstats.st_size;
    dlrec->address = rm_addr;
    dlrec->name = malloc(strlen(sec_prefix) + strlen(RM_NAME) + 3);
    sprintf(dlrec->name, "%s_%s", sec_prefix, RM_NAME);

    if (dlrec_cur) {
        dlrec_cur->next = dlrec;
    } else {
        dlrec_head = dlrec;
    }
    dlrec_cur = dlrec;
    
    data = (unsigned char *)malloc(dlrec->length);
    if (data == NULL) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    read(rm_fd, data, dlrec->length);
    dlrec->sum = fletcher((uint16_t *)data, dlrec->length / 2);
    write_section(dlrec, data);
    free(data);
}

static void
add_checksum_table (uint32_t checksum_table_addr) {
    dsp_download_records_t *dlrec;
    dsp_download_records_t *dlp;
    int ndlrecs = 0;
    section_table_t section_table;
    int i;

    dlrec = dlrec_head;
    while (dlrec) {
        ++ndlrecs;
        dlrec = dlrec->next;
    }

    dlrec = (dsp_download_records_t *)malloc(sizeof(dsp_download_records_t));
    if (dlrec == NULL) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }

    dlrec->name = "dsp_sp2600_ARM_checksum_table";
    dlrec->sum = 0;
    dlrec->section_attributes = SECTION_READONLY;
    dlrec->next = NULL;
    dlrec->address = checksum_table_addr;
    dlrec->length = sizeof(section_table);

    memset(&section_table, 0, sizeof(section_table));
    section_table.checksum_magic = htonl(CHECKSUM_MAGIC);

    dlp = dlrec_head;
    i = 0;
    while (dlp) {
        if (i >= (MAX_SECTIONS-1)) {
            /*
             * The last section entry must be zero, so the actual 
             * max allowed is -1
             */
            fprintf(stderr, "ERROR: Number of sections exceeds %d\n",
                    MAX_SECTIONS-1);
            exit(1);
        }
        section_table.section_table_entry[i].section_length = htonl(dlp->length);
        section_table.section_table_entry[i].section_start = htonl(dlp->address);
        section_table.section_table_entry[i].section_attributes = htonl(dlp->section_attributes);
        section_table.section_table_entry[i].section_sum = htonl(dlp->sum);
        dlp = dlp->next;
        ++i;
    }
    if (verbose) {
        fprintf(stderr, "Number of sections=%d, max=%d-1\n",
                i-1, MAX_SECTIONS);
    }
    if (out_fd) {
        uint8_t *data;
        uint32_t count;

        fprintf(out_fd, "/*\n * %s @ 0x%.8x\n */\n", dlrec->name,
                dlrec->address);
        fprintf(out_fd, "static const uint8_t %s[0x%x] = {\n", dlrec->name,
                dlrec->length);
        /*
         * Output the section table
         */
        count = 0;
        data = (uint8_t *)&section_table;
        while (count < sizeof(section_table)) {
            if (count % 16 == 0) {
                fprintf(out_fd, "\n  ");
            }
            fprintf(out_fd, "0x%.2x%s", data[count],
                    sizeof(section_table) - count == 1 ? "" : ",");
            ++count;
        }
        fprintf(out_fd, "};\n");
    }

    if (rbf_fd) {
        unsigned int tmp;

        tmp = htonl(dlrec->length);
        fwrite(&tmp, sizeof(tmp), 1, rbf_fd);
        tmp = htonl(dlrec->address);
        fwrite(&tmp, sizeof(tmp), 1, rbf_fd);
        fwrite(&section_table, 1, sizeof(section_table), rbf_fd);
    }

    if (rbf_c_fd) {
        unsigned int count;
        unsigned int tmp;
        unsigned char *cp;

        tmp = htonl(dlrec->length);
        cp = (unsigned char *)&tmp;
        fprintf(rbf_c_fd, "  0x%02x,  0x%02x,  0x%02x,  0x%02x,   /* Section length */\n",
                cp[0], cp[1], cp[2], cp[3]);
        tmp =  htonl(dlrec->address);
        fprintf(rbf_c_fd, "  0x%02x,  0x%02x,  0x%02x,  0x%02x,   /* Section address */\n",
                cp[0], cp[1], cp[2], cp[3]);
        count = 0;
        cp = (unsigned char *)&section_table;
        while (count < dlrec->length) {
            fprintf(rbf_c_fd, "  0x%02x,", cp[count]);
            ++count;
            if ((count < dlrec->length) && (count % 10 == 0)) {
                fprintf(rbf_c_fd, "\n");
            }
        }
        fprintf(rbf_c_fd, "\n");
        rbf_c_len += dlrec->length + 8;
    }

    if (dlrec_cur) {
        dlrec_cur->next = dlrec;
    } else {
        dlrec_head = dlrec;
    }
    dlrec_cur = dlrec;
}    
    
static void
include_section (image_info *imginfo, char *section_name) {
    if (imginfo == NULL)
        return;
    strcat(imginfo->included_sections, "{");
    strcat(imginfo->included_sections, section_name);
    strcat(imginfo->included_sections, "}");
}

static void
exclude_section (image_info *imginfo, char *section_name) {
    if (imginfo == NULL)
        return;
    strcat(imginfo->excluded_sections, "{");
    strcat(imginfo->excluded_sections, section_name);
    strcat(imginfo->excluded_sections, "}");
}

/*
 * find_checksum_table
 *
 * Search the symbol table to find the address of the checksum table
 * The symbol is assumed to be "dss_mgr_checksum_table"
 */
static uint32_t
find_checksum_table (image_info *imginfo) {
    bfd *bfdfile;
    long storage_needed;
    asymbol **symbol_table;
    char **matching;
    long number_of_symbols;
    long i;

    if (imginfo == NULL) {
        return 0;
    }
    bfdfile = bfd_openr(imginfo->image_name, NULL);
    if (bfdfile == NULL) {
        nonfatal(imginfo->image_name);
        return 0;
    }
    if (!bfd_check_format_matches (bfdfile, bfd_object, &matching)) {
        return 0;
    }

    storage_needed = bfd_get_symtab_upper_bound(bfdfile);
    if (storage_needed <= 0) {
        return 0;
    }
    symbol_table = (asymbol **)malloc(storage_needed);
    if (symbol_table == NULL) {
        return 0;
    }
    number_of_symbols = bfd_canonicalize_symtab(bfdfile, symbol_table);
    if (number_of_symbols <= 0) {
        return 0;
    }
    for (i = 0; i < number_of_symbols; ++i) {
        if (strcmp("dss_mgr_checksum_table", symbol_table[i]->name) == 0) {
            return (uint32_t)bfd_asymbol_value(symbol_table[i]);
        }
    }
    bfd_close(bfdfile);
    return 0;
}

void output_version_header(FILE *fd) {
    unsigned long n;

    if (fd == NULL) {
        return;
    }
    fprintf(fd, version_header_begin, sizeof(version_header));  
    for (n = 0; n < sizeof(version_header); n++) {
        if (n > 0) {
            fprintf(fd, ",");
        }
        if ((n % 16) == 0) {
            fprintf(fd, "\n  ");
        }
        fprintf(fd, "0x%02x", ((unsigned char *)&version_header)[n]);
    } 
    fprintf(fd, version_header_end);  
}

FILE *output_c_header(char *filename) {
    FILE *tmp_fd;

    if (filename) {
        const char *dev_str;
        time_t  t;

        time(&t);
        strcpy(current_date_string, ctime(&t));
        strncpy(current_year_string,
                &current_date_string[strlen(current_date_string) - 5], 4);
        current_year_string[4] = '\0';

        printf("Sending C output to %s\n", filename);
        tmp_fd = fopen(filename, "w");
        if (tmp_fd == NULL) {
            fprintf(stderr, "Can't open %s\n", filename);
            exit(2);
        }
        strncpy(actual_user, cuserid(NULL), sizeof(actual_user)); 
        switch (dev) {
        case SP26XX:
            dev_str = "SP2600";
            break;
        case SP27XX:
            dev_str = "SP2700";
            break;
        case DM814X:
            dev_str = "ANALOGBRI";
            break;
        case TILEGX:
            dev_str = "TILEGX";
            break;
        default:
            dev_str = "UNK";
            break;
        }
        if (asr1k) {
            fprintf(tmp_fd, copyright_header, filename, CVERSION, VDATE, 
                    logical_user, actual_user, gmt_str,
                    embVer, embRev, embBuild,
                    release_string, s1, s2, s3, s4,
                    current_year_string);
            fprintf(tmp_fd, "#include \"spa_dsp_sp26_fw.h\"\n");
        } else {
            fprintf(tmp_fd, file_header, filename, CVERSION, VDATE, 
                    logical_user, actual_user, gmt_str,
                    embVer, embRev, embBuild,
                    release_string, s1, s2, s3, s4,
                    current_year_string, dev_str, dev_str, dev_str);
        }
    } else {
        tmp_fd = NULL;
    }
    return tmp_fd;
}


void output_spi_ctrl (void) {
    uint16_t tmp_16;
    uint16_t csum;
    uint8_t tmp_8;
    uint32_t tmp_32;

    if (verbose) {
        printf("Control Record at 0x%x\n", spi_addr);
    }

    tmp_16 = SPI_CONTROL_MAGIC;
    csum = spi_put(sizeof(tmp_16), &tmp_16);
    spi_addr += sizeof(tmp_16);

    tmp_32 = spi_addr + 15;
    csum += spi_put(3, &tmp_32);
    spi_addr += 3;

    tmp_8 = SPI_PLL1SELECT;         /* pll1select */
    csum +=spi_put(1, &tmp_8);
    ++spi_addr;

    tmp_8 = SPI_PLL_F;              /* pll_f */
    csum +=spi_put(1, &tmp_8);
    ++spi_addr;

    tmp_8 = SPI_SSPDIV;             /* sspdiv */
    csum +=spi_put(1, &tmp_8);
    ++spi_addr;

    tmp_8 = 0x0;                /* reserved */
    csum +=spi_put(1, &tmp_8);
    ++spi_addr;

    tmp_8 = ~SPI_PLL1SELECT;         /* one's complement of pll1select */
    spi_put(1, &tmp_8);
    ++spi_addr;

    tmp_8 = ~SPI_PLL_F;              /* one's complement of pll_f */
    spi_put(1, &tmp_8);
    ++spi_addr;

    tmp_8 = ~SPI_SSPDIV;             /* one's complement of sspdiv */
    spi_put(1, &tmp_8);
    ++spi_addr;

    tmp_8 = ~0x0;                /* one's complement of reserved */
    spi_put(1, &tmp_8);
    ++spi_addr;

    spi_put(2, &csum);
    spi_addr += 2;

    csum = ~csum;
    spi_put(2, &csum);
    spi_addr += 2;
}
    
uint8_t default_sp2704_mac[] = {
    0x00, 0xFA, 0xCE, 0x27, 0x04, 0x01
};

void spi_fill_upto (unsigned int upto)
{
    while (spi_addr < upto) {
        fputc(0x0, spi_fd);
        ++spi_addr;
    }
}

FILE *output_spi_header (char *filename) {
    uint32_t tmp_32;

    if (filename == NULL) {
        return NULL;
    }
    spi_fd = fopen(filename, "w");
    if (spi_fd == NULL) {
        fprintf(stderr, "Can't open %s\n", filename);
        exit(2);
    }
    if ((spi_mode == spi_golden) ||
        (spi_mode == spi_complete)) {
        /* Put out SPI image start word 0xFACE2704 */
        tmp_32 = 0xFACE2704;
        spi_put(4, &tmp_32);
        spi_addr = 4;

        /* Now output the jump record */

        /* First offset is for the Golden image  at 0x104 */
        tmp_32 = SPI_GOLDEN_OFFSET;
        spi_put(3, &tmp_32);
        spi_addr += 3;    

        /* Second offset is for the upgradeable image  at 0x20000 */
        tmp_32 = SPI_GOLDEN_OFFSET; // SPI_UPGRADE_OFFSET;
        spi_put(3, &tmp_32);
        spi_addr += 3;    

        /* Fill in more jump table entries to allow for more upgrade images */
        tmp_32 = SPI_GOLDEN_OFFSET; // SPI_UPGRADE_OFFSET;
        spi_put(3, &tmp_32);
        spi_addr += 3;    

        tmp_32 = SPI_GOLDEN_OFFSET; // SPI_UPGRADE_OFFSET;
        spi_put(3, &tmp_32);
        spi_addr += 3;    

        /* Fill the rest of the jump table with the Golden image offset */
        while (spi_addr < SPI_JUMP_END) {
            tmp_32 = SPI_GOLDEN_OFFSET;
            spi_put(3, &tmp_32);
            spi_addr += 3;    
        }

        /* Write out a MAC address at 0xC4.  This will be patched by manufacturing */
        /* no longer used, replace by boot config at 0x10000 */
        spi_put(6, default_sp2704_mac);
        spi_addr += 6;

        /* Fill up to 0xE0 with zeros */
        while (spi_addr < SPI_VERSION_OFFSET) {
            fputc(0x0, spi_fd);
            ++spi_addr;
        }
        fputc(0x1, spi_fd);  /*  version 1 */
        ++spi_addr;
        if (spi_sdb) {
            fputc(0x1, spi_fd);  /* type (0=NGVM, 1=SDB) */
            ++spi_addr;
        }

        /* Fill up to 0x104 with zeros */
        while (spi_addr < SPI_GOLDEN_OFFSET) {
            fputc(0x0, spi_fd);
            ++spi_addr;
        }
    } else {
        spi_addr = SPI_UPGRADE_OFFSET;
    }

    /* output a control record */

    output_spi_ctrl();

    return spi_fd;
}

#define ETH_ADDR_L 6
#define MAX_PID_LEN (64 - ETH_ADDR_L)
#define MAX_TFTPFILE_LEN 64

#define SP27_DDR3_ECC 0x80
typedef enum {
    SP27_DDR3_16BIT = 0,
    SP27_DDR3_32BIT = 1
} sp27_platform_t;

/* write the public key to spi file. Note that primary and back
 * keys are the same for now. Just saved into different section
 * of the eeprom.
 */
void output_spi_key_record(void) 
{
    uint32_t size;
    uint32_t i;

    size = sizeof(primary);
    printf("\nWriting public key to 0x%x, size=%d\n", spi_addr, size);
    for (i = 0; i < size; i++) {
        fputc(primary[i], spi_fd);
        ++spi_addr;
    }
}

void output_spi_config(void) {
    uint32_t size;
    void *bc;

    env_init();
    env_set_string("MAC0", "00:11:22:33:44:77");
    if (spi_sdb) {
        env_set_int("DDR3_CONFIG", SP27_DDR3_ECC + SP27_DDR3_32BIT, 16);
        env_set_int("DSPSOK", 1, 16);
        env_set_string("PID", "SP2704 SDB");
        env_set_string("IP_ADDRESS", "192.168.0.10");
        env_set_string("TFTP_SERVER", "192.168.0.100");
        env_set_string("TFTP_FILE", "dsp_sp2700_fw.img");
    } else {
        env_set_string("PID", "NGVM");
        env_set_string("IP_ADDRESS", "0.0.0.0");
        env_set_string("TFTP_SERVER", "0.0.0.0");
        env_set_int("DDR3_CONFIG", SP27_DDR3_ECC + SP27_DDR3_32BIT, 16);
        env_set_string("TFTP_FILE", "firmware/dsp_sp2700_fw.img");
    }
    env_sync(0);
    bc = env_get_raw(&size);
    printf("config going to 0x%x\n", spi_addr);
    spi_put(size, bc);
    spi_addr += size;

}

void output_termination_record (void) {
    uint16_t tmp;
    uint32_t tmp_32;
    uint16_t csum;
    
    if (verbose) {
        printf("Termination Record at 0x%x\n", spi_addr);
    }
    tmp = SPI_TERMINATION_MAGIC;
    csum = spi_put(sizeof(tmp), &tmp);
    spi_addr += sizeof(tmp);

    tmp_32 = 0;
    csum += spi_put(sizeof(tmp_32), &tmp_32);
    spi_addr += sizeof(tmp_32);
    
    spi_put(sizeof(csum), &csum);
    spi_addr += sizeof(csum);
    csum = ~csum;
    spi_put(sizeof(csum), &csum);
    spi_addr += sizeof(csum);
}

void output_signed_upgrade_bldr_to_spi (char *filename)
{
    FILE *in_fw;
    unsigned int fw_len;
    unsigned int i;

    printf("\nProcessing %s", filename);
    /* open input dsp firmware */
    in_fw = fopen(filename, "rb");
    if (in_fw == NULL) {
        printf("\nUnable to open file %s for read\n", filename);
        exit(1);
    }

    /* figure out the input firmware length */
    fseek(in_fw, 0 , SEEK_END);
    fw_len = ftell(in_fw);
    printf("\nUpgrade bootloader size: %d bytes\n", fw_len);

    fseek(in_fw, 0 , SEEK_SET);
    for (i = 0; i < fw_len; i++) {
        fputc(fgetc(in_fw), spi_fd);
        ++spi_addr;
    }

    fclose(in_fw);

}

int
main (int argc, char **argv) {
    image_info *images = NULL;
    image_info *curimage = NULL;

    char *c_output_file = NULL;
    char *h_output_file = NULL;
    char *h2_output_file = NULL;
    char *rbf_file = NULL;
    char *rbf_c_file = NULL;
    char *spi_file = NULL;
    char *signed_bldr = NULL;
    int arg, c;
    dsp_download_records_t *dlrec;
    uint32_t checksum_table_addr;
    char *rm_file = NULL;
    uInt32 rm_addr = 0;
    int dss_image_count = 0;

    bfd_init();
    arg = 1;
    while (arg < argc) {
        if (strcmp(argv[arg], "-sp27xx") == 0) {
            dev = SP27XX;
        } else if (strcmp(argv[arg], "-dm814x") == 0) {
            dev = DM814X;
        } else if (strcmp(argv[arg], "-tilegx") == 0) {
            dev = TILEGX;
        } else if (strcmp(argv[arg], "-asr1k") == 0) {
            asr1k = 1;
        } else if (strcmp(argv[arg], "-e") == 0) {
            /* -e : Create output files for eeprom burner */
            do_ee_prom = 1;
        } else if (strcmp(argv[arg], "-V") == 0) {
            ++verbose;
        } else if (strcmp(argv[arg], "-s") == 0) {
            /* -s : Include a section by name */
            ++arg;
            if (arg >= argc) {
                usage();
            }
            include_section(curimage, argv[arg]);
        } else if (strcmp(argv[arg], "-x") == 0) {
            /* -x : Exclude a section by name */
            ++arg;
            if (arg >= argc) {
                usage();
            }
            exclude_section(curimage, argv[arg]);
        } else if (strcmp(argv[arg], "-o") == 0) {
            /* -o : Specify name of .c output file */
            ++arg;
            if (arg >= argc) {
                usage();
            }
            c_output_file = argv[arg];
        } else if (strcmp(argv[arg], "-r") == 0) {
            /* -r : Specify name of .rbf output file */
            ++arg;
            if (arg >= argc) {
                usage();
            }
            rbf_file = argv[arg];
        } else if (strcmp(argv[arg], "-c") == 0) {
            /* -c : Specify name of rbf .c output file */
            ++arg;
            if (arg >= argc) {
                usage();
            }
            rbf_c_file = argv[arg];
        } else if (strcmp(argv[arg], "-gspi") == 0) {
            /* -spi: Generate spi (EEPROM) image for boot loader */
            ++arg;
            if (arg >= argc) {
                usage();
            }
            if (spi_file) {
                fprintf(stderr, "Can't do -uspi and -spi at the same time\n");
                exit(2);
            }
            spi_mode = spi_golden;
            spi_file = argv[arg];
        } else if (strcmp(argv[arg], "-uspi") == 0) {
            /* -uspi: Generate spi (EEPROM) upgreade image for boot loader */
            ++arg;
            if (arg >= argc) {
                usage();
            }
            if (spi_file) {
                fprintf(stderr, "Can't do -uspi and -spi at the same time\n");
                exit(2);
            }
            spi_mode = spi_upgrade;
            spi_file = argv[arg];
        } else if (strcmp(argv[arg], "-spi") == 0) {
            /* -spi: Generate complete spi (EEPROM) image for boot loader */
            ++arg;
            if (arg >= argc) {
                usage();
            }
            if (spi_file) {
                fprintf(stderr, "Can't do -uspi and -spi at the same time\n");
                exit(2);
            }
            spi_mode = spi_complete;
            spi_file = argv[arg];
        } else if (strcmp(argv[arg], "-sdb") == 0) {
            spi_sdb = 1;
        } else if (strcmp(argv[arg], "-img") == 0) {
            ++arg;
            if (arg >= argc) {
                usage();
            }
            signed_bldr = argv[arg];
        } else if (strcmp(argv[arg], "-u") == 0) {
            /* -u : Print the usage message */
            ++arg;
            if (arg >= argc) {
            usage();
            }
            sscanf(argv[arg], "%s", logical_user);
        } else if (strcmp(argv[arg], "-ve") == 0) {
            /* -ve : Specify embeded extended version info */
            ++arg;
            if (arg >= argc) {
                usage();
            }
            sscanf(argv[arg], "%d.%d.%d", &embVer, &embRev, &embBuild);
            fprintf(stderr,
                    "conv26x: Embedding Extended Version Information %d.%d.%d\n",
                    embVer, embRev, embBuild);
        } else if (strcmp(argv[arg], "-cs1") == 0) {
            /* -cs1 : Specify compatibility string 1 */
            ++arg;
            if (arg >= argc) {
                usage();
            }
            strncpy(s1, argv[arg], sizeof(s1));
        } else if (strcmp(argv[arg], "-cs2") == 0) {
            /* -cs2 : Specify compatibility string 2 */
            ++arg;
            if (arg >= argc) {
                usage();
            }
            strncpy(s2, argv[arg], sizeof(s2));
        } else if (strcmp(argv[arg], "-cs3") == 0) {
            /* -cs3 : Specify compatibility string 3 */
            ++arg;
            if (arg >= argc) {
                usage();
            }
            strncpy(s3, argv[arg], sizeof(s3));
        } else if (strcmp(argv[arg], "-cs4") == 0) {
            /* -cs4 : Specify compatibility string 4 */
            ++arg;
            if (arg >= argc) {
                usage();
            }
            strncpy(s4, argv[arg], sizeof(s4));
        } else if (strcmp(argv[arg], "-vr") == 0) {
            /* -vr : Specify version release string */
            ++arg;
            if (arg >= argc) {
                usage();
            }
            sscanf(argv[arg], "%s", release_string);
        } else if (strcmp(argv[arg], "-name") == 0) {
            /* -name : Specify section name prefix */
            ++arg;
            if (arg >= argc) {
                usage();
            }
            strcpy(sec_prefix, argv[arg]);
        } else if (strcmp(argv[arg], "-rm") == 0) {
            /* -rm : Specify reset message file name */
            ++arg;
            if (arg >= argc) {
                usage();
            }
            rm_file = argv[arg];
        } else if (strcmp(argv[arg], "-ra") == 0) {
            /* -rm : Specify reset message address */
            ++arg;
            if (arg >= argc) {
                usage();
            }
            sscanf(argv[arg], "%x", &rm_addr);
        } else if (strcmp(argv[arg], "-h") == 0) {
            ++arg;
            if (arg >= argc) {
                usage();
            }
            h_output_file = argv[arg];
        } else if (strcmp(argv[arg], "-h2") == 0) {
            ++arg;
            if (arg >= argc) {
                usage();
            }
            h2_output_file = argv[arg];
        } else {
            if (argv[arg][0] == '-') {
                /* specify linker output file by type */
                image_info *newimage;
                newimage = (image_info *)malloc(sizeof(image_info));
                if (newimage == NULL) {
                    fprintf(stderr, "Out of memory\n");
                    exit(1);
                }
                newimage->next = NULL;
                newimage->image_name = NULL;
                newimage->image_prefix[0] = '\0';
                newimage->core_id_flags = 0;
                newimage->option_flags = 0;
                newimage->included_sections[0] = '\0';
                if (curimage) {
                    curimage->next = newimage;
                } else {
                    images = newimage;
                }
                curimage = newimage;
                c = 1;
                while (argv[arg][c]) {
                    switch(argv[arg][c]) {
                    case 'a':
                        curimage->core_id_flags = CORE_ID_FLAG_PPB;
                        break;
                    case '0':
                        curimage->core_id_flags |= CORE_ID_FLAG_DSS_0;
                        break;
                    case '1':
                        curimage->core_id_flags |= CORE_ID_FLAG_DSS_1;
                        break;
                    case '2':
                        curimage->core_id_flags |= CORE_ID_FLAG_DSS_2;
                        break;
                    case 'd':
                        curimage->core_id_flags = CORE_ID_FLAG_ALL_DSS;
                        if (dss_image_count == 1) {
                            curimage->core_id_flags |= CORE_ID_FLAG_DSS_IMG_1;
                        }
                        dss_image_count++;
                        break;
                    default:
                        usage();
                        break;
                    }
                    ++c;
                }
                /* Test for various dis-allowed combinations */
                if ((curimage->core_id_flags & CORE_ID_FLAG_ALL_DSS) &&
                    (curimage->core_id_flags & CORE_ID_FLAG_PPB)) {
                    usage();
                }
                if (curimage->core_id_flags == CORE_ID_FLAG_PPB) {
                    strcpy(curimage->image_prefix, "ARM_");
                } else if (curimage->core_id_flags == CORE_ID_FLAG_ALL_DSS) {
                    strcpy(curimage->image_prefix, "DSS_");
                } else if (curimage->core_id_flags & CORE_ID_FLAG_DSS_IMG_1) {
                    strcpy(curimage->image_prefix, "DSS_F1_");
                } else {
                    char dss_string[80];

                    dss_string[0] = '\0';
                    if (curimage->core_id_flags & CORE_ID_FLAG_DSS_0) {
                        strcat(dss_string, "0");
                    }
                    if (curimage->core_id_flags & CORE_ID_FLAG_DSS_1) {
                        strcat(dss_string, "1");
                    }
                    if (curimage->core_id_flags & CORE_ID_FLAG_DSS_2) {
                        strcat(dss_string, "2");
                    }
                    sprintf(curimage->image_prefix, "DSS%s_", dss_string);
                }
            } else {
                if (curimage->image_name) {
                    usage();
                }
                curimage->image_name = argv[arg];
            }
        }
        ++arg;
    }
    if (strlen(sec_prefix) == 0) {
        if (dev == SP26XX) {
            strncpy(sec_prefix, "dsp_sp2600", sizeof(sec_prefix));
        } else if (dev == SP27XX) {
            strncpy(sec_prefix, "dsp_sp2700", sizeof(sec_prefix));
        } else if (dev == DM814X) {
            strncpy(sec_prefix, "dsp_analogbri", sizeof(sec_prefix));
        } else if (dev == TILEGX) {
            strncpy(sec_prefix, "dsp_tilegx", sizeof(sec_prefix));
        }
    }
    fill_in_version_header();
    out_fd = output_c_header(c_output_file);
    if (h_output_file) {
        FILE *h_fd = output_c_header(h_output_file);
        if (h_fd) {
            fclose(h_fd);
        }
    }
    if (h2_output_file) {
        FILE *h2_fd = output_c_header(h2_output_file);
        if (h2_fd) {
            output_version_header(h2_fd);
            fclose(h2_fd);
        }
    }
    output_version_header(out_fd);
    rbf_c_fd = output_c_header(rbf_c_file);
    if (rbf_file) {
        printf("Sending RBF output to %s\n", rbf_file);
        rbf_fd = fopen(rbf_file, "w");
        if (rbf_fd == NULL) {
            fprintf(stderr, "Can't open %s\n", c_output_file);
            exit(2);
        }
        fwrite(&version_header, sizeof(version_header), 1, rbf_fd);
    } else {
        rbf_fd = NULL;
    }

    if (rbf_c_fd) {
        unsigned int n;
        fprintf(rbf_c_fd, "const unsigned char %s_fmw_start[] = {", sec_prefix);
        for (n = 0; n < sizeof(version_header); n++) {
            if ((n % 10) == 0) {
                fprintf(rbf_c_fd, "\n  ");
            }
            fprintf(rbf_c_fd, "0x%02x,  ", ((unsigned char *)&version_header)[n]);
        }
        fprintf(rbf_c_fd, "\n");
        rbf_c_len += sizeof(version_header);
    }
    memset(&dss_overlay, 0, sizeof(dss_overlay));
    dss_ovltab_cnt = 0;

    if (spi_file) {
        spi_fd = output_spi_header(spi_file);
    }

    curimage = images;
    while (curimage) {
        if (verbose) {
            fprintf(stdout, "Processing %s image\n", curimage->image_name);
        }
        add_image(curimage);
        curimage = curimage->next;
    }

    /*
     * Add the reset message if specified
     */
    if (rm_file) {
        add_rm(rm_file, rm_addr);
    }

    /*
     * Find the ARM image
     */
    curimage = images;
    while (curimage) {
        if (curimage->core_id_flags & CORE_ID_FLAG_PPB) {
            checksum_table_addr = find_checksum_table(curimage);
            if (checksum_table_addr == 0) {
                fprintf(stderr, "dss_mgr_checksum_table not found in %s\n",
                        curimage->image_name);
            } else {
                break;
            }
        }
        curimage = curimage->next;
    }
    if (checksum_table_addr != 0) {
        add_checksum_table(checksum_table_addr);
    }
    if (out_fd) {
        fprintf(out_fd, "static dsp_download_records_t %s_fmw[] = {\n",
                sec_prefix);
        dlrec = dlrec_head;
        while (dlrec) {
            fprintf(out_fd, "  {0x%.8x, 0x%.8x, %s},\n", dlrec->length,
                    dlrec->address, dlrec->name);
            dlrec = dlrec->next;
        }
        fprintf(out_fd, "  {0, 0, (void *)0}\n");
        fprintf(out_fd, "};\n");
        fprintf(out_fd,
                "const dsp_download_records_t * %s_fmw_get (void)\n"
                "{\n"
                "    return (%s_fmw);\n"
                "}\n", asr1k?"btl_sp2600":sec_prefix, sec_prefix);

        fclose(out_fd);
    }

    if (rbf_fd) {
        uint32_t zero = 0;
        fwrite(&zero, sizeof(zero), 1, rbf_fd);
        fwrite(&zero, sizeof(zero), 1, rbf_fd);
        fclose(rbf_fd);
    }

    if (rbf_c_fd) {
        fprintf(rbf_c_fd, "  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00};\n");
        fprintf(rbf_c_fd, "const uint32_t      %s_fmw_size = 0x%08x;\n", sec_prefix,
                rbf_c_len + 8);
        fclose(rbf_c_fd);
    }

    if (spi_fd) {
        output_termination_record();
        if (spi_mode != spi_complete) {
            fclose(spi_fd);
        } else {
            if (spi_sdb) {
                /* put the config into 0x10000 for SDB */
                spi_fill_upto(SDB_CFG_REMAP_OFFSET);
                output_spi_config();
            }

            /* put the primary key into spi 0x20000*/
            spi_fill_upto(SPI_PRI_KEY_OFFSET);
            output_spi_key_record();

            /* for SDB, the upgrade bootloader is located at 0x22000 */
            if (spi_sdb) {
                spi_fill_upto(SDB_UPGRADE_OFFSET);
                if (signed_bldr) {
                    output_signed_upgrade_bldr_to_spi(signed_bldr);
                }
            } else { /* NGVM */
                /* environment variables to 0x30000*/
                spi_fill_upto(SPI_CONFIG_OFFSET);
                output_spi_config();

                /* backup key record to 0x40000*/
                spi_fill_upto(SPI_BACKUP_KEY_OFFSET);
                output_spi_key_record();

                /* signed upgrade bootloader to 0x50000 */
                spi_fill_upto(SPI_UPGRADE_OFFSET);
                if (signed_bldr) {
                    output_signed_upgrade_bldr_to_spi(signed_bldr);
                }
            }
        }
    }

    printf("Saved sections size is %d (0x%x)\n", save_sections_size, save_sections_size);
    return 0;
}

/*
 * $Log: conv26x.c,v $
 * Revision 1.4  2016/10/07 17:53:21  srane
 * CSCvb61570 - Move to SWIMS server for code signing
 *
 * Revision 1.3  2012/06/28 21:19:52  srane
 * Boot loader changes application firmware name.
 *
 * Revision 1.2  2012/06/28 13:33:09  srane
 * New boot loader requirements - environment variables, unique mgaic
 * number for SP2704 (will boot only 2704), SSP support.
 *
 * Revision 1.1  2012/04/18 18:15:17  srane
 * Initial checkin
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
*/

