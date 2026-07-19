# $Id: conv26x.mk,v 1.2 2017/07/28 07:58:36 harrchan Exp $
# $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/image/conv26x.mk,v $
#------------------------------------------------------------------
# conv26x.mk  - Makefile for SP26xx convert utility
#
# Mar, 2012 srane
#
# Copyright (c) 2012-2017 by Cisco Systems, Inc.
# All rights reserved.
#------------------------------------------------------------------
#------------------------------------------------------------------
# Makefile - Makefile for SP26xx convert utility
#
# Copyright (c) 2007-2009 by cisco Systems, Inc.
# All rights reserved.
#------------------------------------------------------------------
BUILD_OS := $(shell uname)
LSI_DIR ?= /auto/aegir-ios/tools/dsp/lsi

ifeq ($(BUILD_OS),SunOS)
BFDPATH=$(LSI_DIR)/arm/cgtools/binutils-2.17-build
TARGET=conv26x-SunOS
endif
ifeq ($(BUILD_OS),Linux)
ifneq (,$(findstring 2.6.9, $(BUILD_OS_VER)))
BFD_VER := -DOLD_BFD
BFDPATH=$(LSI_DIR)/arm/cgtools/binutils-2.17-linux-build
else
ifneq (,$(findstring 2.6.18, $(BUILD_OS_VER)))
BFDPATH=$(LSI_DIR)/../ti/binutils-2.24_for_2.6.18
else
BFDPATH=$(LSI_DIR)/../ti/binutils-2.24
endif
endif
TARGET=conv26x-Linux
endif

CC := $(wildcard /sw/packages/gcc/c2.95.3-p8/bin/gcc)
ifeq ($(strip $(CC)),) 
CC := gcc
endif
CFLAGS = -I$(BFDPATH)/bfd -I$(BFDPATH)/include -g -W -Wall -Werror -Wstrict-prototypes
C_SRCS = conv26x.c env.c

C_OBJS := $(C_SRCS:.c=.o)

%.o : %.c
	$(CC) -c $(CFLAGS) -o $@ $<

$(TARGET):$(C_OBJS) 
	$(CC) $(CFLAGS) -o $(TARGET) $(C_OBJS) -L$(BFDPATH)/bfd -L$(BFDPATH)/libiberty -lbfd -liberty -lz

clean:
	rm -f $(TARGET)

# $Log: conv26x.mk,v $
# Revision 1.2  2017/07/28 07:58:36  harrchan
# Collapse Oakenshield-branch to Main Trunk.
#
# Revision 1.1.2.1  2017/06/29 08:14:31  harrchan
# Initial commit code for Oakenshield
#
# Revision 1.2.86.1  2016/12/14 04:49:35  olin2
# Initial commit code for Oakenshield
#
# Revision 1.3  2016/10/07 17:54:33  srane
# CSCvb61570 - Move to SWIMS server for code signing
#
# Revision 1.2  2012/06/28 13:33:09  srane
# New boot loader requirements - environment variables, unique mgaic
# number for SP2704 (will boot only 2704), SSP support.
#
# Revision 1.1  2012/04/18 18:15:17  srane
# Initial checkin
#
#
#------------------------------------------------------------------
# $Endlog$
#

