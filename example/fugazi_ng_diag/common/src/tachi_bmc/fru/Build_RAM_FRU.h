/* $Id: Build_RAM_FRU.h,v 1.2 2016/04/20 08:41:35 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/fru/Build_RAM_FRU.h,v $
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 */
#ifndef BUILD_RAM_FRU_H
#define BUILD_RAM_FRU_H


#define TRUE                                (1)
#define FALSE                               (0)
#define NVRAM_FRU_FILENAME                  "/mnt/jffs2/FRU.DAT"
#define RAM_FS_FRU_FILENAME                 "/var/nuova/FRU0_RAM.DAT"
#define ASSET_TAG_FILENAME                  "/mnt/jffs2/AssetTag.DAT"
#define CHASSIS_INFO_FILENAME               "/var/nuova/ChassisInfo.DAT"

#define FRU_XFR_BUF_SIZE                    (0x10)
#define MAX_FRU_SIZE                        (1024)

#define INTERNAL_USE_INDEX                  (0x01)
#define CHASSIS_INFO_INDEX                  (0x02)
#define BOARD_AREA_INDEX                    (0x03)
#define PRODUCT_AREA_INDEX                  (0x04)
#define MULTI_RECORD_INDEX                  (0x05)
#define NUM_TOC_ELEMENTS                    (0x05)

#define INTERNAL_USE_AREA_SIZE              (72)
#define MAX_ASSET_TAG_LEN                   (32)
#define INITIAL_DST_FRU_OFFSET              (8)

#define G_STATUS_SUCCESS                     (0)
#define G_STATUS_CAN_NOT_ALLOC_MEM           (-100)
#define G_STATUS_CAN_NOT_LOAD_NVRAM_FRU      (-200)
#define G_STATUS_CAN_NOT_CREATE_RAM_FRU      (-300)
#define G_STATUS_CAN_NOT_WRITE_RAM_FRU       (-400)

#define TransferByteFromSrcToDst(S, SOffset, D, DOffset, SaveVal, TLen)     {SaveVal = *(D + DOffset) = *(S + SOffset); DOffset++; SOffset++; TLen++;}
#define FRU_Offset(x)                       ((x) << 3)

#define RECONSTRUCT_RAMFRU_CMD \
				"/usr/local/bin/Build_RAM_FRU > /dev/null 2> /dev/null"

/*****************************************************************************/
typedef struct assettagstruct
{
  unsigned char Len;
  char          Buf[MAX_ASSET_TAG_LEN+1];
}AssetTagStruct;


#endif   /* BUILD_RAM_FRU_H */



