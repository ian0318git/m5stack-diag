# $Id: code_sign_dsp.mk,v 1.1 2016/09/28 06:06:25 srane Exp $
# $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/sp27dsp/image/code_sign_dsp.mk,v $
#------------------------------------------------------------------
# Makefile - 
#
# Sep, 2016 srane
#
# Copyright (c) 2016 by Cisco Systems, Inc.
# All rights reserved.
#------------------------------------------------------------------
#

#------------------------------------------------------------------
# Makefile - Makefile for DSP code sign utility
#
# Copyright (c) 2012, 2016 by cisco Systems, Inc.
# All rights reserved.
#------------------------------------------------------------------
BUILD_OS := $(shell uname)
TARGET = code_sign_dsp

CC := $(wildcard /sw/packages/gcc/c2.95.3-p8/bin/gcc)
ifeq ($(strip $(CC)),) 
CC := gcc
endif

#CFLAGS =  $(addprefix -I, $(INCLUDE_DIR)) -g -W -Wall -Werror \
#          -Wstrict-prototypes $(addprefix -D, $(CC_DEFINES)) 

CFLAGS =  -g -W -Wall -Wstrict-prototypes $(addprefix -D, $(CC_DEFINES))

VPATH := ../cs_tools/src:

C_SRCS = code_sign_dsp.c

C_OBJS := $(C_SRCS:.c=.o)

%.o : %.c
	$(CC) -c $(CFLAGS) -o $@ $<

$(TARGET): $(C_OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(C_OBJS)

clean:
	rm -f *.o $(TARGET)

# $Log: code_sign_dsp.mk,v $
# Revision 1.1  2016/09/28 06:06:25  srane
# Makefile for the code_sign_dsp
#
#
#------------------------------------------------------------------
# $Endlog$
#
