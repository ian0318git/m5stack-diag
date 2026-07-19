# $Id: Makefile,v 1.10 2012/10/04 23:37:13 srane Exp $
# $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/sp27dsp/ppb/obj/Makefile,v $
#------------------------------------------------------------------
# Makefile
#
# Mar 2012, Smita Rane
#
# Copyright (c) 2012 by Cisco Systems, Inc.
# All rights reserved.
#
#------------------------------------------------------------------


#------------------------------------------------------------------
# Makefile to create sp27xx-PPB image
#
# September 2010, pbecerra
#
# Copyright (c) 2010-2012 by cisco Systems, Inc.
# All rights reserved.
#------------------------------------------------------------------
ELF_APP := arm_sp2700_sw.elf
# Default image
$(ELF_APP):

system := $(shell uname -s)
ifneq ($(system), Linux)
$(error Only a Linux build is supported)
endif

# Locate the base directory for LSI DSP tools
LSI_DIR ?= /auto/aegir-ios/tools/dsp/lsi

include version

CGEN_VERSION    := 4.1.0
OSECK_VERSION   := 4.0.3
DEVICES_VERSION := 2.9.3

ARCH      := arm11_le
KRNARCH   := arm11
BOARD     := evb2704
HOSTARCH  := linux
OSECKROOT := $(LSI_DIR)/ose/OSEck$(OSECK_VERSION)
BSPROOT   := $(OSECKROOT)/$(ARCH)/bsp-$(BOARD)

USE_LSI_SP27XX_REGS = yes
USE_FUSION_IDE_ETH = yes

INCPPB           := ../inc
ifeq ($(USE_FUSION_IDE_ETH), yes)
ETH       := ../fide
else
ETH       := ../bootloader
endif

SCLLC_TOOLS_HOME := $(LSI_DIR)/arm/cgtools/$(CGEN_VERSION)-linux

#setup paths to cgtools for build environment
ARM_CGEN_DIR := $(LSI_DIR)/arm/cgtools/$(CGEN_VERSION)-$(HOSTARCH)
SC_DEVICES := $(LSI_DIR)/devices/$(DEVICES_VERSION)/sp27xx

export PATH := $(ARM_CGEN_DIR)/bin:$(PATH)

CC  = arm-elf-gcc
CPP = arm-elf-cpp
LD  = arm-elf-ld
AS  = arm-elf-gcc
RM  = rm -f
MKCONFIG = $(OSECKROOT)/$(HOSTARCH)/bin/mkconfig_$(KRNARCH)

# Build options
ifeq ($(MAKELEVEL),0)
USE_LSI_SP2700_DRV :=no
USE_USERINT        :=no
USE_VLAN           :=no
DEBUG_CONSOLE      :=yes
endif

VPATH := ../src:
VPATH += $(BSPROOT)/src:
VPATH += ../src/diag_menu
VPATH += ../lib

INCLUDE_DIR := $(INCPPB) 
ifeq ($(USE_LSI_SP27XX_REGS), yes)
INCLUDE_DIR += $(SC_DEVICES)/ppb/include
else
INCLUDE_DIR += $(SP2700DRV)/regs
endif
INCLUDE_DIR += ../lib
INCLUDE_DIR += $(BSPROOT)/src
INCLUDE_DIR += $(ARM_CGEN_DIR)/arm-elf/include

LIB_DIRS := $(BSPROOT)/lib \
	$(OSECKROOT)/$(ARCH)/lib \
	$(OSECKROOT)/$(ARCH)/krn-$(KRNARCH)/lib \
	$(ARM_CGEN_DIR)/arm-elf/lib \
	$(ARM_CGEN_DIR)/lib/gcc/arm-elf/$(CGEN_VERSION) \
	$(ETH)/lib

LIBS := linx_l 
LIBS += c 
LIBS += gcc 

ifeq ($(USE_LSI_SP2700_DRV), yes)
CC_DEFINES += USE_LSI_SP2700_DRV USE_NIL SP_PPB_PKT_DRV_INC_UDL_SUPPORT
endif

ifeq ($(USE_LSI_SP27XX_REGS), yes)
CC_DEFINES += USE_LSI_SP27XX_REGS
else
CC_DEFINES += USE_AG_MG_REGS
endif
CC_DEFINES += ARM 
CC_DEFINES += VER=\"$(VER)\" BUILD=\"$(BUILD)\" 
CC_DEFINES += MAJOR=\"$(MAJOR)\" MINOR=\"$(MINOR)\"
CC_DEFINES += _SP2603_ 
CC_DEFINES += _SP27XX_
CC_DEFINES += _LITTLE_ENDIAN
CC_DEFINES += DSP_MAX_NUM_PORTS=1
CC_DEFINES += USE_PFM_CONFIG_H
CC_DEFINES += USE_BSP_CONFIG_H
CC_DEFINES += _SP27XX_FIXME_

#ARM11MP_SINGLE_CORE 
ifeq ($(USE_USERINT), yes)
CC_DEFINES += USE_USERINT
endif
ifeq ($(DEBUG_CONSOLE), yes)
CC_DEFINES += DEBUG_CONSOLE
endif

#
# Add path to dump_sections.h file
#
ifneq ($(DIR_DUMP_SECTIONS_H),)
INCLUDE_DIR += $(DIR_DUMP_SECTIONS_H)
else
#INCLUDE_DIR += ../obj-27-ppb
endif

ifeq ($(USE_FUSION_IDE_ETH), yes)
ETH_SRC = PPB27_EMACloop.c  emac27_hal.c libeth.c libserdes.c lsi_mg_report.c \
	libppbuart.c
else
ETH_SRC       := ../bootloader
endif

TDM_SRC = TDM_tests.c libtdm.c tdm_intr_and_unicnt_cntl.c swtu_io.c siu_io.c \
	lsi_mg_pll.c
 
#ifeq ($(RELEASE), 0) # if make RELEASE=0, make it debugable
#CC_FLAGS = -g -O0
#else
#CC_FLAGS = -g -O2
#endif
CC_FLAGS = -g -O0
CC_FLAGS += -Wall -Werror -Wstrict-prototypes -march=armv6 \
	-msoft-float -mlittle-endian -mlong-calls -ffunction-sections \
	$(addprefix -D, $(CC_DEFINES)) $(addprefix -I, $(INCLUDE_DIR)) 

AS_FLAGS = -c -g -Wa,-mlittle-endian -Wa,-mno-fpu -Wa,-march=armv6 -DLONG_CALLS

LD_FLAGS = -T linkscript.lcf -u odo_exception_handler \
	-u boot_hpi_jump_entry \
	-u dss_mgr_checksum_table \
	-u app_resetMsgMem -no-warn-mismatch --gc-sections

C_SRCS = diag_ppb_main.c diag_ppb_comm.c uart.c test_mem.c menu.c ppb_test.c \
	debug_console.c diag_ppb_utils.c lsi_mg_report.c libppbuart.c irqrtn.c \
	fiqrtn.c mpcore_dist_intrc_io.c libppbint.c arm_read_cpu_id.c errrtn.c \
	diag_dss_release.c PPB27_EMACloop.c emac27_hal.c libserdes.c libeth.c \
	error.c libppbgpio.c $(TDM_SRC) PPB27_DDR3_Force_ECC_Errors.c libdma.c \
	dma_basic.c ddr3_init.c ssp.c gpio_I2C.c libavsc.c 

S_SRCS = init.S 

C_DEPS := $(C_SRCS:.c=.d)
C_OBJS := $(C_SRCS:.c=.o)
A_OBJS := $(S_SRCS:.S=.o)
OBJS := $(C_OBJS) $(A_OBJS)

ifeq ($(MAKELEVEL),0)
objects: dump_sections.h $(OBJS)
	$(RM) dump_sections.h 

dump_sections.h:
	echo "typedef struct {" > $@
	echo "    unsigned int coreid;" >> $@
	echo "    char name[80];" >> $@
	echo "    unsigned int startaddr;" >> $@
	echo "    unsigned int len;" >> $@
	echo "} section_def;" >> $@
	echo "section_def dump_sections[] = {" >> $@
	echo "    {0, \"\", 0, 0}" >> $@
	echo "};" >> $@
	echo "section_def dump_sections_1[] = {" >> $@
	echo "    {0, \"\", 0, 0}" >> $@
	echo "};" >> $@
endif

%.o : %.c
	$(CC) -c $(CC_FLAGS) -MD -MP -MF $(basename $@).d -o $@ $<

%.o : %.S
	$(AS) $(AS_FLAGS) -o $@ $<

arm_sp2700_sw.elf: $(OBJS) linkscript.lcf
	$(LD) $(LD_FLAGS) sysmem.lcf $(OBJS) \
	--start-group \
	$(addprefix -L, $(LIB_DIRS)) \
	$(addprefix -l, $(LIBS)) \
	--end-group \
	-Map $@.map -o $@

install: $(REL_APP)
	scp $< home-lnx:/tftpboot/$<

clean:
	$(RM) *.o *.d *.elf *.map

ifneq ($(MAKECMDGOALS),clean)
-include $(C_DEPS)
endif

# Include the static analysis makefile support
# only if running static analysis.
# If not running static analysis no impact on regular builds
ifneq ($(SES),)
#Include the SES static analysis makefile if running static analysis
include /auto/ses/repository/static_analysis/sesmake_include
endif # End of ifneq ($(SES),)

# /******** History ********
# $Log: Makefile,v $
# Revision 1.10  2012/10/04 23:37:13  srane
# Version control.
#
# Revision 1.9  2012/10/02 23:18:51  srane
# Add version file.
#
# Revision 1.8  2012/09/10 06:34:25  srane
# Add AVS file.
#
# Revision 1.7  2012/08/15 14:14:10  srane
# Add linker file for sysmem.
#
# Revision 1.6  2012/07/17 20:45:30  srane
# Add GPIO I2C support.
#
# Revision 1.5  2012/06/28 13:33:49  srane
# Add SSP support
#
# Revision 1.4  2012/06/07 22:50:41  srane
# TDM external loopback, ECC memory test
#
# Revision 1.3  2012/05/24 23:23:23  srane
# Add gpio file.
#
# Revision 1.2  2012/05/10 22:57:50  srane
# Add TDM support. Adjust the linker sections.
#
# Revision 1.1  2012/04/18 09:51:06  srane
# Initial checkin
#
# 
# $Endlog$


