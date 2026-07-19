/* $Id: ata_dev_dvr.h,v 1.1 2013/05/09 05:42:38 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/ata_dev_dvr.h,v $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/ata_dev_dvr.h,v $
 *------------------------------------------------------------------
 * ata_dev_dvr.h - Header file for generic ATA device driver code
 *
 * Nov 2000, Min Xu
 *
 * Ported from rommon code by Khalid Sabzwari
 *
 * Copyright (c) 2013 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

#ifndef __ATA_DEV_LES_H__
#define __ATA_DEV_LES_H__

#define         ATA_BYTES_PER_SECTOR  512
#define         ATA_WORDS_PER_SECTOR  (ATA_BYTES_PER_SECTOR/2)

#define 	ADRSPC_CF0_TASK_REG	0x1F0
#define 	ADRSPC_CF1_TASK_REG	0x170
#define 	CF0_DEV_CTL_REG         0x3F6
#define 	CF1_DEV_CTL_REG         0x376
#define		CS6_MB_IOFPGA_CF0_REGS  0x10501000
#define         CS6_MB_IOFPGA_CF1_REGS            0x10502000

#define         ATA_MAX_RW_SECTORS      256
#define         ATA_LEFS_FORMAT_DATA    0xffff
#define         ATA_SECTOR_RD_CODE_SIZE 0x100

#define ATA_STATUS_RETRY	0xfffff

/*
   ATA STATUS REGISTER BIT Defs
   */
#define         STAT_ERROR      0x01
#define         STAT_INDEX      0x02
#define         STAT_CORR       0x04
#define         STAT_DRQ        0x08
#define         STAT_SEEKCOMPLETE       0x10
#define         STAT_WRITEFAULT 0x20
#define         STAT_READY      0x40
#define         STAT_BUSY       0x80

#define         STAT_READY_FOR_CMD_MASK  (STAT_READY | STAT_SEEKCOMPLETE)

/*  Card status request  codes */

#define         READY_FOR_CMD   1  /* wait for card ready status */
#define         READY_FOR_XFER  2  /* wait for data transfer ready */
#define         WAIT_NOT_BSY    3  /* wait for busy bit to clear */
#define         SOFT_RESET      4  /* software reset for drivecontrol */

/* ------
   ATA Command OpCodes , also defined by the standard
   ------
   */
#define CMD_DRIVEID             0xEC
#define CMD_READSECTOR          0x20
#define CMD_WRITESECTOR         0x30
#define CMD_FORMAT_TRACK        0x50
#define CMD_REQUEST_SENSE       0x03
#define TF_CMD_REQ_SENSE CMD_REQUEST_SENSE
#define CMD_RECALIBRATE         0x10
#define CMD_DRIVEDIAGS          0x90


/* stuff for parsing CIS structure - note this does not inlcude
 * Mfg. ID right now
 */
#define         MAX_CIS_RETRIES      0x05
#define         CISTPL_NULL          0x00            /* Null tuple - ignore */
#define         CISTPL_DEVICEGEO_A   0x1F            /* Device Geometry information for*/
#define         CISTPL_MANFID        0x20            /* Manufacturer & Card ID Tuple*/
#define         CISTPL_FUNCID        0x21            /* Function ID Tuple*/
#define         CISTPL_FUNCE         0x22            /* Function Extension Tuple*/
#define         CISTPL_SWIL          0x23            /* Software Interleaving*/
#define         CISTPL_VERS_2        0x40            /* Level-2 version tuple*/
#define         CISTPL_FORMAT        0x41            /* Format tuple*/
#define         CISTPL_GEOMETRY      0x42            /* Geometry tuple*/
#define         CISTPL_BYTEORDER     0x43            /* Byte order tuple*/
#define         CISTPL_DATE          0x44            /* Card initialization date tuple*/
#define         CISTPL_BATTERY       0x45            /* Battery replacement date tuple  */
#define         CISTPL_ORG           0x46            /* Organization tuple*/
#define         CISTPL_FORMAT_A      0x47            /* Data recording format for*/

#define         CISTPL_SPCL          0x90            /* Special purpose tuple*/

#define CISTPL_DEVICE_OFFSET     0x00
#define CISTPL_DEVICE            0x01
#define CISTPL_JEDEC_ID          0x18
#define CISTPL_VERS_1            0x15
#define CISTPL_LIST_END          0xFF

#define CIS_ATTRIBUTEMEMORYSIZE 512

#define FLASH_FHDR_MAGIC       0xbad00b1e /* file header magic  */
#define FLASH_FHDR_UNUSED      0xffffffff


/*
 * When FSLIB is defined, the ATA device driver is used in ATA
 * monlib.  There is no delay function in ATA monlib, so larger
 * ATA_WAIT_COUNT is necessary.  This fix is needed to boot
 * from DOSFS formatted Sandisk 128MB CompactFlash.
 */
#ifdef FSLIB
#define ATA_WAIT_COUNT          0x800000
#else
//#define ATA_WAIT_COUNT          50000
#define ATA_WAIT_COUNT          500000
#endif

/*
 * ATA file system type
 */
#define ATA_LEFS       1       /* Low End File System      */
#define ATA_DOSFS      2       /* DOS (PCMCIA) File System */
#define ATA_FS_UNKNOWN 3

#define ATA_OK                  0
#define ATA_ERR                 -1
#define DIAG_NO_ERROR   	0x01

#define ODD_ADDR_SHIFT          8
#define TRIDE_MWDMA_MODE_4    0x2
#define PIO_MODE               0x1
#define MWDMA_MODE             0x4
#define CMD_SET_FEATURE         0xEF
#define SET_TRANSFER_MODE      0x3

/*
 * CIS Tuple definitions
 * Two type of CISTPL address mapping platforms uses this file:
 *  1) assumes CISTPL addr to start at 0x8000 offset from device base addr
 *     (piper, giove, etc..)
 *  2) maps CISTPL addr to specific address. (cuisinart)
 *  
 *  If platfroms map CISTPL address space to a specific address different from 
 *  the device base address then they will have to define PLATFORM_ATA_CIS_BASE
 * 
 */
#ifdef PLATFORM_ATA_CIS_BASE
#define GET_ATA_CISTPL_ADDR(dev_addr, offset) (PLATFORM_ATA_CIS_BASE + offset)
#define ATA_CISTPL_MANFID_OFF    (0x40)
#define ATA_CISTPL_VENDERID_OFF  (0x48)
#else
#define GET_ATA_CISTPL_ADDR(dev_addr, offset) (dev_addr + offset)
#define ATA_CISTPL_MANFID_OFF    (0x8040)
#define ATA_CISTPL_VENDERID_OFF  (0x8048)
#endif

/*
 * ATA venders implement format track as writing 0xff
 */
#define ATA_SANDISK    0x0045

/*
 * ATA venders implement format track as DOS format
 */
#define ATA_TOSHIBA    0x0098
#define ATA_HITACHI    0x0007
#define ATA_MICRON     0x002C

#define ATA_DEF_HEAD   0xE0

#define OUTB(_port_, _val_) \
 __asm__ __volatile__ ( "outb %%al,%%dx" : : "a" (_val_), "d"(_port_) );
#define OUTW(_port_, _val_) \
 __asm__ __volatile__ ( "outw %%ax,%%dx" : : "a" (_val_), "d"(_port_) );
#define OUTL(_port_, _val_) \
 __asm__ __volatile__ ( "outl %%eax, %%dx" :: "a" (_val_), "d"(_port_) );
#define INB(_port_) \
({ unsigned char _x;\
 __asm__ __volatile__ ( "xor %%eax,%%eax ;" \
   "inb %%dx, %%al" : "=a" (_x) : "d"(_port_) ); \
  _x; \
})
 
#define INW(_port_) \
({ \
   unsigned short _x; \
    __asm__ __volatile__ ("xor %%eax,%%eax ; " \
  "inw %%dx, %%ax" : "=a" (_x) : "d"(_port_) ); \
 _x; \
})
 
#define INL(_port_) \
({ \
 unsigned long _x; \
 __asm__ __volatile__ ("xor %%eax,%%eax ; " \
  "inl %%dx, %%eax" : "=a" (_x) : "d"(_port_) ); \
 _x; \
})

/*
 * Defines for Identify device command (0xEC)
 */
#define TRIDE_PIO_MODE_6	2
 
typedef struct ata_reg_t_ {
    volatile uchar data_reg;      /* offst 0, 16-bit access R/W data */
    volatile uchar err_fr;        /* offst 1, error/feature register */
    volatile uchar sect_cnt;      /* offst 2, sector count */
    volatile uchar sect_num;      /* offst 3, sector number */
    volatile uchar cyl_low;       /* offst 4, cylinder low */
    volatile uchar cyl_high;      /* offst 5, cylinder high */
    volatile uchar dev_hd;        /* offst 6, Ddevice head */
    volatile uchar dev_status_cmd;  /* offst 7, status/Command */
} ata_reg_t;
typedef struct ata_alt_reg_t_ {
    volatile ushort dev_alt_status;         /* offst 0, 8-bit access */
} ata_alt_reg_t;

#define DATA_REG(x)            (((ata_reg_t *)x)->data_reg)
#define SECTOR_CNT(x)          (((ata_reg_t *)x)->sect_cnt)
#define SECTOR_NUM(x)          (((ata_reg_t *)x)->sect_num)
#define CYLINDER_LOW(x)        (((ata_reg_t *)x)->cyl_low)
#define CYLINDER_HIGH(x)       (((ata_reg_t *)x)->cyl_high)
#define HEAD_DRIVE(x)          (((ata_reg_t *)x)->dev_hd)
#define STATUS_COMMAND(x)      (((ata_reg_t *)x)->dev_status_cmd)
#define ERROR_FEATURE(x)       (((ata_reg_t *)x)->err_fr)
#define DEVICE_CTRL(x)         ((ata_alt_reg_t *)x)


/*
 * ATA drive information structure
 */
typedef struct ata_drive_info_t_ {
    ushort config_data;		/* 0  Ceneral configuration bit-significant info */
    ushort d_cylinders;		/* 1  Default number of cylinders		*/
    ushort res_000;		/* 2  Reserved					*/
    ushort d_heads;		/* 3  Default number of heads			*/
    ushort bytes_per_track;	/* 4  Unformatted bytes per track		*/
    ushort bytes_per_sector;	/* 5  Unformatted bytes per sector		*/
    ushort d_sectors_per_track;	/* 6  Default number of sectors per track	*/
    ulong  d_sectors_per_card;	/* 7-8   Number of sectors per card		*/
    ushort res_001;		/* 9     Reserved				*/
    char   serial_number[20];	/* 10-19 Right justified serial number		*/
    ushort buffer_info[3];	/* 20-22 Buffer type, size and EEC bytes 	*/
    char   firmware_rev[8];	/* 23-26 Firmware version in big endian		*/
    char   model_number[40];	/* 27-46 Model # in big endian left justified 	*/
    ushort info_001[6];		/* 47-52 Information we don't care		*/
    ushort tran_param_valid;    /* 53    Translation param valid	   	*/
    ushort cylinders;           /* 54    Current number of cylinders          	*/
    ushort heads;               /* 55    Current number of heads              	*/
    ushort sectors_per_track;   /* 56    Current number of sectors per track  	*/
    ulong  sectors_per_card;    /* 57-58 Number of sectors per card        	*/
    ushort res1;		/* 59     */
    ulong  lba_num;             /* 60-61  */
    ushort res2;		/* 62	  */
    ushort mwdma_wd;            /* 63     mwdma support or not 			*/
    ushort pio_wd;              /* 64     PIO modes supported or not 		*/
    ushort res[98];	        /* 65-162 Don't care data - at least for now  	*/
    ushort true_ide_cap;        /* True-IDE timing capability and settings 	*/
    ushort padding[93];	        /* Padding to make the size 512 bytes		*/
}  __attribute__ ((packed)) ata_drive_info_t;

/*
 * Cylinder/Head/Sector (CHS) address type
 */
typedef struct chs_t_ {
    ulong byte;
    uchar sector;
    uchar head;
    ushort cyl;
} chs_t;

extern uchar sector_buf[];
extern char ata_sector_rd_code[];
extern int (*cached_ata_sector_rd_fn)(void *ata_reg, uchar *buf, 
                                      int sector_cnt);

typedef void (*cf_led_fn_t) (void);

/*
 * External Function Declaration
 */
extern ulong ata_read(void *, uchar *, uchar *, ulong, cf_led_fn_t, cf_led_fn_t);
extern ulong ata_write(void *, uchar *, uchar *, ulong, cf_led_fn_t, cf_led_fn_t);
extern ulong ata_get_dev_size(void *);
extern ulong ata_erase(void *, uchar *, ulong, ulong, cf_led_fn_t, cf_led_fn_t);
extern boolean ata_erase_all(void *, ulong, cf_led_fn_t, cf_led_fn_t);
extern int ata_sector_rd(void *, uchar *, int);
extern int ata_read_sector(void *, uchar, ushort, uchar, uchar *, uchar);
extern int ata_write_sector(void *, uchar, ushort, uchar, uchar *, uchar);
extern int ata_get_drive_info(void *, ata_drive_info_t *);
extern int ata_file_system_type(void *);
extern int ata_write_sector_lba(void *,  ulong, uchar *, uchar);
extern int ata_read_sector_lba(void *, ulong, uchar *, uchar);
extern void ata_test_read(void);

#endif

/******** History ********
$Log: ata_dev_dvr.h,v $
Revision 1.1  2013/05/09 05:42:38  alpeng
moving overlord common code from x86

Revision 1.2  2012/03/28 00:38:19  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:07  ptong
Initial archive of ng_diag module


$Endlog$
*/
