/* $Id: cmd_rom_ugd.c,v 1.1 2017/10/19 14:04:29 palin2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/cmd_rom_ugd.c,v $ 
 *------------------------------------------------------------------
 * 
 * Filename   : cmd_rom_ugd.c
 * Description: Main file of TSN CLI command, rom-ugd.
 *              This command is for TSN to upgrade ROMMON.
 *
 *              This Tool has developer version.
 *              In developer version, user can upgrade any version
 *              of ROMMON; Otherwise, tool/code will lock one
 *              specific ROMMON to upgrade. That is to avoid MFG
 *              program the wrong ROMMON.
 *              To enable developer version, please add definition,
 *              DEVELOPER_VER, in Makefile.
 *
 * Copyright (c) 2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <mtd/mtd-user.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "common.h"
#include "types.h"
#include "common_utils.h"
#include "menu.h"
#include "nvmonvars.h"
#include "proto.h"
#include "queryflags.h"
#include "cmd_rom_ugd.h"


/*******************************************************************************
 *                             Functions Declaration                           *
 *******************************************************************************
 */
static int get_img_content(char *, int, uchar *);
int tsn_spi_bootflash_rd(int, int, int, uchar *);
int tsn_spi_bootflash_wr(int, int, int, uchar *);
int tsn_spi_bootflash_erase(int, int, int);
void rom_ugd_usage(void);
int  rom_ugd_util(char *);

/*******************************************************************************
 *                               Global Variable                               *
 *******************************************************************************
 */

/*******************************************************************************
 *                                    Functions                                *
 *******************************************************************************
 */
/*******************************************************************************
 *
 * Function    : show_rom_ugd_usage
 * Description : To show usage of TSN ROMMON upgrade function.
 * Inputs      : None
 * Outputs     : None
 *
 *******************************************************************************
 */
void rom_ugd_usage (void)
{
    printf("usage:\n");
#ifdef DEVELOPER_VER
    printf("rom-ugd [ugd_file]\n");
#else
    printf("rom-ugd\n");
#endif /* DEVELOPER_VER */
}

/*******************************************************************************
 *
 * Function    : get_file_content
 * Description : Function to get content of image for ROMMON upgrade.
 *               This function returns buffer pointer that store content.
 * Inputs      : *img - image name and location
 *               img_sz - image size
 *               *buf - buffer to store read back content
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
static int get_img_content (char *img, int img_sz, uchar *buf)
{
    FILE *img_fp;

    printf("Reading image... ");
    if (fflush(stdout) != 0) {
        printf("%s(%d) Failed to do fflush(stdout): %s\n",
               __func__, __LINE__, strerror(errno));
    }

    /* Get image content */
    img_fp = fopen(img, "rb");
    if (img_fp == NULL) {
        printf("%s(%d) Failed to open %s: %s.\n",
               __func__, __LINE__, img, strerror(errno));
        return (FAILED);
    }

    if (fread(buf, 1, img_sz, img_fp) != img_sz) {
        printf("%s(%d) Failed to get %s content.\n", __func__, __LINE__, img);
        fclose(img_fp);
        return (FAILED);
    }

    printf("DONE.\n");
    if (fflush(stdout) != 0) {
        printf("%s(%d) Failed to do fflush(stdout): %s\n",
               __func__, __LINE__, strerror(errno));
    }

    fclose(img_fp);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : tsn_spi_bootflash_rd
 * Description : Function to read TSN SPI bootflash content.
 * Inputs      : fd - file descriptor
 *               offset - start location to read
 *               size - read size
 *               *buf - buffer to store read back content
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int tsn_spi_bootflash_rd (int fd, int offset, int size, uchar *buf)
{
    int rd_sz = -1;

    /* reposition read file offset */
    if (lseek(fd, offset, SEEK_SET) == -1) {
        printf("%s(%d) Failed to reposition file offset.\n", __func__, __LINE__);
        return (FAILED);
    }

    /* Start to read data back */
    rd_sz = read(fd, buf, size);
    if (rd_sz == -1) {
        printf("%s(%d) Filed to read data out: %s\n",
               __func__, __LINE__, strerror(errno));
        return (FAILED);
    } else if (rd_sz != size) {
        printf("%s(%d) Error, read back %d bytes is not expected(%d bytes)\n",
               __func__, __LINE__, rd_sz, size);
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : tsn_spi_bootflash_wr
 * Description : Function to write TSN SPI bootflash content.
 * Inputs      : fd - file descriptor
 *               offset - start location to write
 *               size - write size
 *               *buf - buffer to store write back content
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int tsn_spi_bootflash_wr (int fd, int offset, int size, uchar *buf)
{
    int wr_sz = -1;

    /* reposition write file offset */
    if (lseek(fd, offset, SEEK_SET) == -1) {
        printf("%s(%d) Failed to reposition file offset.\n", __func__, __LINE__);
        return (FAILED);
    }

    /* Start to write data back */
    wr_sz = write(fd, buf, size);
    if (wr_sz == -1) {
        printf("%s(%d) Filed to write data out: %s\n",
               __func__, __LINE__, strerror(errno));
        return (FAILED);
    } else if (wr_sz != size) {
        printf("%s(%d) Error, write back %d bytes is not expected(%d bytes)\n",
               __func__, __LINE__, wr_sz, size);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : tsn_spi_bootflash_erase
 * Description : Function to erase TSN SPI bootflash sector(s).
 * Inputs      : fd - file descriptor
 *               offset - start location to read
 *               size - size that want to erase
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int tsn_spi_bootflash_erase (int fd, int offset, int size)
{
    erase_info_t ei;

    /* Set erase size */
    ei.length = size;

    for (ei.start = offset; ei.start < (offset + size); ei.start += ei.length) {
        if (ioctl(fd, MEMERASE, &ei) < 0) {
            printf("%s(%d) Failed to erase bootflash.\n", __func__, __LINE__);
            return (FAILED);
        }
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : rom_ugd_util
 * Description : Utility to upgrade TSN ROMMON.
 * Inputs      : *img - Image name and location for ROMMON upgrade
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int rom_ugd_util (char *img)
{
    int ret_val = PASSED;
    struct stat img_st;
    int img_size = 0;
    uchar *img_cont_p;
    int accum_len = 0;
    char usr_ch = 0;
    mtd_info_t mtd_info;
    int mtd_fd = -1;
    int ctr = 0;
    char mtd_name[MTD_NAME_SZ];
    int mtd_size = 0, mtd_blk_sz = 0; /* byte(s) */
    uchar *mtd_blk_cont;
    int chk_len = 0;

    /* Sanity check */
    if (img == NULL) {
        printf("%s(%d) Image pointer is NULL.\n", __func__, __LINE__);
        return (FAILED);
    }

    /* Check image exist and get image size if it exists */
    if (stat(img, &img_st) == 0) {
        img_size = img_st.st_size;
    } else {
        printf("%s(%d) Cannont determine size of %s: %s\n",
               __func__, __LINE__, img, strerror(errno));
        return (FAILED);
    }
    printf("Image %s exists(size: %d bytes).\n", img, img_size);

    if (img_size != IMG_SZ_8MB) {
        printf("Warning!! Input ROMMON binary is %.2f MB(should be 8MB).\n",
               (img_size / ONE_MB));
        printf("Program anyway?('y'/'Y' for yes; or any other key to quit)\n");
        usr_ch = getchar();
        if ((usr_ch != 'y') && (usr_ch != 'Y')) {
            printf("Stop by user request.\n");
            return (PASSED);
        }
    }

    /* Get image content */
    img_cont_p = (uchar *)malloc(img_size * sizeof(uchar));
    if (img_cont_p == NULL) {
        printf("%s(%d) Failed to malloc %d bytes to store image content.\n",
               __func__, __LINE__, img_size);
        return (FAILED);
    }

    if (get_img_content(img, img_size, img_cont_p) != PASSED) {
        printf("%s(%d) Failed to get %s content.\n", __func__, __LINE__, img);
        return (FAILED);
    }

    /* Start to upgrade ROMMON */
    printf("Upgrading ROMMON:");
    if (fflush(stdout) != 0) {
        printf("%s(%d) Failed to do fflush(stdout): %s\n",
               __func__, __LINE__, strerror(errno));
    }

    do {
        memset(mtd_name, 0, sizeof(mtd_name));
        sprintf(mtd_name, "/dev/mtd%d", ctr);

        mtd_fd = open(mtd_name, O_RDWR);
        if (mtd_fd == -1) {
            printf("%s(%d) Failed to open %s: %s\n",
                   __func__, __LINE__, mtd_name, strerror(errno));
            free(img_cont_p);
            return (FAILED);
        }

        /* Get SPI NOR flash info(size, sector size, ...) */
        ioctl(mtd_fd, MEMGETINFO, &mtd_info);

        mtd_size = mtd_info.size;
        mtd_blk_sz = mtd_info.erasesize;

        mtd_blk_cont = (uchar *)malloc(mtd_blk_sz * sizeof(uchar));
        if (mtd_blk_cont == NULL) {
            printf("%s(%d) Failed to malloc %d bytes to store MTD content.\n",
                   __func__, __LINE__, mtd_blk_sz);
            free(img_cont_p);
            if (close(mtd_fd) == -1) {
                printf("%s(%d) Failed to close %s: %s\n",
                       __func__, __LINE__, mtd_name, strerror(errno));
            }
            return (FAILED);
        }

        /* Update bootflash(SPI NOR flash) content */
        for (chk_len = 0; chk_len < mtd_size; chk_len += mtd_blk_sz) {
            /* Normalize buffer */
            memset(mtd_blk_cont, 0, mtd_blk_sz);

            /* Read content of SPI NOR flash sector */
            if (tsn_spi_bootflash_rd(mtd_fd,
                                     chk_len,
                                     mtd_blk_sz,
                                     mtd_blk_cont) != PASSED) {
                printf("%s(%d) Failed to read %s sector %d.\n",
                       __func__, __LINE__, mtd_name, (int)(chk_len / mtd_blk_sz));
                ret_val = FAILED;
                if (close(mtd_fd) == -1) {
                    printf("%s(%d) Failed to close %s: %s\n",
                           __func__, __LINE__, mtd_name, strerror(errno));
                }
                goto ROM_UGD_END;
            }

            /* Compare content with upgraded image */
            if (memcmp(&img_cont_p[accum_len],
                       mtd_blk_cont,
                       mtd_blk_sz) == 0) {
               accum_len += mtd_blk_sz;

               if (accum_len >= img_size) {
                   /* Upgrade done */
                   break;
               } else {
	           printf(".");
	           if (fflush(stdout) != 0) {
		       printf("%s(%d) Failed to do fflush(stdout): %s\n",
                              __func__, __LINE__, strerror(errno));
                   }
                   continue;
	       }
            }

            /* If update SPI NOR flash content is needed */
            /* Erase SPI NOR flash sector */
            if (tsn_spi_bootflash_erase(mtd_fd, chk_len, mtd_blk_sz) != PASSED) {
                printf("%s(%d) Failed to erase %s sector %d.\n",
                       __func__, __LINE__, mtd_name, (int)(chk_len / mtd_blk_sz));
                ret_val = FAILED;
                if (close(mtd_fd) == -1) {
                    printf("%s(%d) Failed to close %s: %s\n",
                           __func__, __LINE__, mtd_name, strerror(errno));
                }
                goto ROM_UGD_END;
            }

            /* Program upgraded image content to sector of SPI NOR flash */
            if (tsn_spi_bootflash_wr(mtd_fd,
                                     chk_len,
                                     mtd_blk_sz,
                                     &img_cont_p[accum_len]) != PASSED) {
                printf("%s(%d) Failed to write %s sector %d.\n",
                       __func__, __LINE__, mtd_name, (int)(chk_len / mtd_blk_sz));
                ret_val = FAILED;
                if (close(mtd_fd) == -1) {
                    printf("%s(%d) Failed to close %s: %s\n",
                           __func__, __LINE__, mtd_name, strerror(errno));
                }
                goto ROM_UGD_END;
            }

            /* Read content of SPI NOR flash sector back for confirm */
            /* Normalize buffer */
            memset(mtd_blk_cont, 0, mtd_blk_sz);

            if (tsn_spi_bootflash_rd(mtd_fd,
                                     chk_len,
                                     mtd_blk_sz,
                                     mtd_blk_cont) != PASSED) {
                printf("%s(%d) Failed to read %s sector %d.\n",
                       __func__, __LINE__, mtd_name, (int)(chk_len / mtd_blk_sz));
                ret_val = FAILED;
                if (close(mtd_fd) == -1) {
                    printf("%s(%d) Failed to close %s: %s\n",
                           __func__, __LINE__, mtd_name, strerror(errno));
                }
                goto ROM_UGD_END;
            }

            /* Compare content with upgraded image */
            if (memcmp(&img_cont_p[accum_len],
                       mtd_blk_cont,
                       mtd_blk_sz) != 0) {
                printf("%s(%d) %s sector%d is not programmed correctly: "
                       "Data mismatch.\n", __func__, __LINE__, mtd_name,
                       (int)(chk_len / mtd_blk_sz));
                ret_val = FAILED;
                if (close(mtd_fd) == -1) {
                    printf("%s(%d) Failed to close %s: %s\n",
                           __func__, __LINE__, mtd_name, strerror(errno));
                }
                goto ROM_UGD_END;
            }
            accum_len += mtd_blk_sz;
            printf("U");
            if (fflush(stdout) != 0) {
                printf("%s(%d) Failed to do fflush(stdout): %s\n",
                       __func__, __LINE__, strerror(errno));
            }
        }

        if (close(mtd_fd) == -1) {
            printf("%s(%d) Failed to close %s: %s\n",
                   __func__, __LINE__, mtd_name, strerror(errno));
            ret_val = FAILED;
            goto ROM_UGD_END;
        }
        ctr++;
    } while (accum_len < img_size);

ROM_UGD_END:
    free(img_cont_p);

    if (ret_val == PASSED) {
        printf("\nUpgraded ROMMON successfully.\n");
        printf("New ROMMON will take effect from UNIT next power ON.\n");
    }
    return (ret_val);
}


/*-------------------------------------------------
 * $Log: cmd_rom_ugd.c,v $
 * Revision 1.1  2017/10/19 14:04:29  palin2
 * Added support to upgrade ROMMON to TSN Oct-2017 Pilot version, 16.6(1r).
 *
 * $Endlog$
 *-------------------------------------------------
 */

